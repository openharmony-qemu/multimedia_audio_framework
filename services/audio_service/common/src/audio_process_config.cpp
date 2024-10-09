/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef LOG_TAG
#define LOG_TAG "ProcessConfig"
#endif

#include "audio_process_config.h"

#include <map>
#include <sstream>

#include "audio_errors.h"
#include "audio_service_log.h"

namespace OHOS {
namespace AudioStandard {
namespace {
static const uint32_t MAX_VALID_USAGE_SIZE = 30; // 128 for pids
static const uint32_t MAX_VALID_PIDS_SIZE = 128; // 128 for pids
static std::map<StreamUsage, std::string> USAGE_TO_STRING_MAP = {
    {STREAM_USAGE_INVALID, "INVALID"},
    {STREAM_USAGE_UNKNOWN, "UNKNOWN"},
    {STREAM_USAGE_MEDIA, "MEDIA"},
    {STREAM_USAGE_MUSIC, "MUSIC"},
    {STREAM_USAGE_VOICE_COMMUNICATION, "VOICE_COMMUNICATION"},
    {STREAM_USAGE_VOICE_ASSISTANT, "VOICE_ASSISTANT"},
    {STREAM_USAGE_ALARM, "ALARM"},
    {STREAM_USAGE_VOICE_MESSAGE, "VOICE_MESSAGE"},
    {STREAM_USAGE_NOTIFICATION_RINGTONE, "NOTIFICATION_RINGTONE"},
    {STREAM_USAGE_RINGTONE, "RINGTONE"},
    {STREAM_USAGE_NOTIFICATION, "NOTIFICATION"},
    {STREAM_USAGE_ACCESSIBILITY, "ACCESSIBILITY"},
    {STREAM_USAGE_SYSTEM, "SYSTEM"},
    {STREAM_USAGE_MOVIE, "MOVIE"},
    {STREAM_USAGE_GAME, "GAME"},
    {STREAM_USAGE_AUDIOBOOK, "AUDIOBOOK"},
    {STREAM_USAGE_NAVIGATION, "NAVIGATION"},
    {STREAM_USAGE_DTMF, "DTMF"},
    {STREAM_USAGE_ENFORCED_TONE, "ENFORCED_TONE"},
    {STREAM_USAGE_ULTRASONIC, "ULTRASONIC"},
    {STREAM_USAGE_VIDEO_COMMUNICATION, "VIDEO_COMMUNICATION"},
    {STREAM_USAGE_RANGING, "RANGING"},
    {STREAM_USAGE_VOICE_CALL_ASSISTANT, "VOICE_CALL_ASSISTANT"},
    {STREAM_USAGE_VOICE_MODEM_COMMUNICATION, "VOICE_MODEM_COMMUNICATION"}
};
}

int32_t ProcessConfig::WriteInnerCapConfigToParcel(const AudioPlaybackCaptureConfig &config, MessageParcel &parcel)
{
    // filterOptions.usages
    size_t usageSize = config.filterOptions.usages.size();
    CHECK_AND_RETURN_RET_LOG(usageSize < MAX_VALID_USAGE_SIZE, ERR_INVALID_PARAM, "usageSize is too large");
    parcel.WriteUint32(usageSize);
    for (size_t i = 0; i < usageSize; i++) {
        parcel.WriteInt32(static_cast<int32_t>(config.filterOptions.usages[i]));
    }

    // filterOptions.usageFilterMode
    parcel.WriteUint32(config.filterOptions.usageFilterMode);

    // filterOptions.pids
    size_t pidSize = config.filterOptions.pids.size();
    CHECK_AND_RETURN_RET_LOG(pidSize <= MAX_VALID_PIDS_SIZE, ERR_INVALID_PARAM, "pidSize is too large");
    parcel.WriteUint32(pidSize);
    for (size_t i = 0; i < pidSize; i++) {
        parcel.WriteUint32(config.filterOptions.pids[i]);
    }

    // filterOptions.pidFilterMode
    parcel.WriteUint32(config.filterOptions.pidFilterMode);

    // silentCapture
    parcel.WriteBool(config.silentCapture);
    return SUCCESS;
}

int32_t ProcessConfig::ReadInnerCapConfigFromParcel(AudioPlaybackCaptureConfig &config, MessageParcel &parcel)
{
    // filterOptions.usages
    uint32_t usageSize = parcel.ReadUint32();
    if (usageSize > MAX_VALID_USAGE_SIZE) {
        AUDIO_ERR_LOG("Invalid param, usageSize is too large: %{public}u", usageSize);
        return ERR_INVALID_PARAM;
    }
    std::vector<StreamUsage> usages = {};
    for (uint32_t i = 0; i < usageSize; i++) {
        int32_t tmpUsage = parcel.ReadInt32();
        if (std::find(AUDIO_SUPPORTED_STREAM_USAGES.begin(), AUDIO_SUPPORTED_STREAM_USAGES.end(), tmpUsage) ==
            AUDIO_SUPPORTED_STREAM_USAGES.end()) {
            AUDIO_ERR_LOG("Invalid param, usage: %{public}d", tmpUsage);
            return ERR_INVALID_PARAM;
        }
        usages.push_back(static_cast<StreamUsage>(tmpUsage));
    }
    config.filterOptions.usages = usages;

    // filterOptions.usageFilterMode
    uint32_t tempMode = parcel.ReadUint32();
    if (tempMode >= FilterMode::MAX_FILTER_MODE) {
        AUDIO_ERR_LOG("Invalid param, usageFilterMode : %{public}u", tempMode);
        return ERR_INVALID_PARAM;
    }
    config.filterOptions.usageFilterMode = static_cast<FilterMode>(tempMode);

    // filterOptions.pids
    uint32_t pidSize = parcel.ReadUint32();
    if (pidSize > MAX_VALID_PIDS_SIZE) {
        AUDIO_ERR_LOG("Invalid param, pidSize is too large: %{public}u", pidSize);
        return ERR_INVALID_PARAM;
    }
    std::vector<int32_t> pids = {};
    for (uint32_t i = 0; i < pidSize; i++) {
        int32_t tmpPid = parcel.ReadInt32();
        if (tmpPid <= 0) {
            AUDIO_ERR_LOG("Invalid param, pid: %{public}d", tmpPid);
            return ERR_INVALID_PARAM;
        }
        pids.push_back(tmpPid);
    }
    config.filterOptions.pids = pids;

    // filterOptions.pidFilterMode
    tempMode = parcel.ReadUint32();
    if (tempMode >= FilterMode::MAX_FILTER_MODE) {
        AUDIO_ERR_LOG("Invalid param, pidFilterMode : %{public}u", tempMode);
        return ERR_INVALID_PARAM;
    }
    config.filterOptions.pidFilterMode = static_cast<FilterMode>(tempMode);

    // silentCapture
    config.silentCapture = parcel.ReadBool();

    return SUCCESS;
}

// INCLUDE 3 usages { 1 2 4 } && EXCLUDE 1 pids { 1234 }
std::string ProcessConfig::DumpInnerCapConfig(const AudioPlaybackCaptureConfig &config)
{
    std::stringstream temp;

    // filterOptions
    switch (config.filterOptions.usageFilterMode) {
        case FilterMode::INCLUDE:
            temp << "INCLUDE";
            break;
        case FilterMode::EXCLUDE:
            temp << "EXCLUDE";
            break;
        default:
            temp << "INVALID";
            break;
    }
    temp << " " << config.filterOptions.usages.size() << " usages { ";
    for (size_t i = 0; i < config.filterOptions.usages.size(); i++) {
        StreamUsage usage = config.filterOptions.usages[i];
        temp << USAGE_TO_STRING_MAP[usage] << " ";
    }
    temp << "} && ";

    // INCLUDE 3 pids { 1 2 4 }
    switch (config.filterOptions.pidFilterMode) {
        case FilterMode::INCLUDE:
            temp << "INCLUDE";
            break;
        case FilterMode::EXCLUDE:
            temp << "EXCLUDE";
            break;
        default:
            temp << "INVALID";
            break;
    }
    temp << " " << config.filterOptions.pids.size() << " pids { ";
    for (size_t i = 0; i < config.filterOptions.pids.size(); i++) {
        temp << config.filterOptions.pids[i] << " ";
    }
    temp << "}";
    // silentCapture will not be dumped.

    return temp.str();
}

int32_t ProcessConfig::WriteConfigToParcel(const AudioProcessConfig &config, MessageParcel &parcel)
{
    // AppInfo
    parcel.WriteInt32(config.appInfo.appUid);
    parcel.WriteUint32(config.appInfo.appTokenId);
    parcel.WriteInt32(config.appInfo.appPid);
    parcel.WriteUint64(config.appInfo.appFullTokenId);

    // AudioStreamInfo
    parcel.WriteInt32(config.streamInfo.samplingRate);
    parcel.WriteInt32(config.streamInfo.encoding);
    parcel.WriteInt32(config.streamInfo.format);
    parcel.WriteInt32(config.streamInfo.channels);
    parcel.WriteUint64(config.streamInfo.channelLayout);

    // AudioMode
    parcel.WriteInt32(config.audioMode);

    // AudioRendererInfo
    parcel.WriteInt32(config.rendererInfo.contentType);
    parcel.WriteInt32(config.rendererInfo.streamUsage);
    parcel.WriteInt32(config.rendererInfo.rendererFlags);
    parcel.WriteInt32(config.rendererInfo.originalFlag);
    parcel.WriteString(config.rendererInfo.sceneType);
    parcel.WriteBool(config.rendererInfo.spatializationEnabled);
    parcel.WriteBool(config.rendererInfo.headTrackingEnabled);
    parcel.WriteInt32(config.rendererInfo.pipeType);

    //AudioPrivacyType
    parcel.WriteInt32(config.privacyType);

    // AudioCapturerInfo
    parcel.WriteInt32(config.capturerInfo.sourceType);
    parcel.WriteInt32(config.capturerInfo.capturerFlags);
    parcel.WriteInt32(config.capturerInfo.originalFlag);
    parcel.WriteInt32(config.capturerInfo.pipeType);

    // streamType
    parcel.WriteInt32(config.streamType);

    // deviceType
    parcel.WriteInt32(config.deviceType);

    // Recorder only
    parcel.WriteBool(config.isInnerCapturer);
    parcel.WriteBool(config.isWakeupCapturer);

    // Original session id for re-create stream
    parcel.WriteUint32(config.originalSessionId);

    return SUCCESS;
}

int32_t ProcessConfig::ReadConfigFromParcel(AudioProcessConfig &config, MessageParcel &parcel)
{
    // AppInfo
    config.appInfo.appUid = parcel.ReadInt32();
    config.appInfo.appTokenId = parcel.ReadUint32();
    config.appInfo.appPid = parcel.ReadInt32();
    config.appInfo.appFullTokenId = parcel.ReadUint64();

    // AudioStreamInfo
    config.streamInfo.samplingRate = static_cast<AudioSamplingRate>(parcel.ReadInt32());
    config.streamInfo.encoding = static_cast<AudioEncodingType>(parcel.ReadInt32());
    config.streamInfo.format = static_cast<AudioSampleFormat>(parcel.ReadInt32());
    config.streamInfo.channels = static_cast<AudioChannel>(parcel.ReadInt32());
    config.streamInfo.channelLayout = static_cast<AudioChannelLayout>(parcel.ReadUint64());

    // AudioMode
    config.audioMode = static_cast<AudioMode>(parcel.ReadInt32());

    // AudioRendererInfo
    config.rendererInfo.contentType = static_cast<ContentType>(parcel.ReadInt32());
    config.rendererInfo.streamUsage = static_cast<StreamUsage>(parcel.ReadInt32());
    config.rendererInfo.rendererFlags = parcel.ReadInt32();
    config.rendererInfo.originalFlag = parcel.ReadInt32();
    config.rendererInfo.sceneType = parcel.ReadString();
    config.rendererInfo.spatializationEnabled = parcel.ReadBool();
    config.rendererInfo.headTrackingEnabled = parcel.ReadBool();
    config.rendererInfo.pipeType = static_cast<AudioPipeType>(parcel.ReadInt32());

    //AudioPrivacyType
    config.privacyType = static_cast<AudioPrivacyType>(parcel.ReadInt32());

    // AudioCapturerInfo
    config.capturerInfo.sourceType = static_cast<SourceType>(parcel.ReadInt32());
    config.capturerInfo.capturerFlags = parcel.ReadInt32();
    config.capturerInfo.originalFlag = parcel.ReadInt32();
    config.capturerInfo.pipeType = static_cast<AudioPipeType>(parcel.ReadInt32());

    // streamType
    config.streamType = static_cast<AudioStreamType>(parcel.ReadInt32());

    // deviceType
    config.deviceType = static_cast<DeviceType>(parcel.ReadInt32());

    // Recorder only
    config.isInnerCapturer = parcel.ReadBool();
    config.isWakeupCapturer = parcel.ReadBool();

    // Original session id for re-create stream
    config.originalSessionId = parcel.ReadUint32();
    return SUCCESS;
}

std::string ProcessConfig::DumpProcessConfig(const AudioProcessConfig &config)
{
    std::stringstream temp;

    // AppInfo
    temp << "appInfo:pid<" << config.appInfo.appPid << "> uid<" << config.appInfo.appUid << "> tokenId<" <<
        config.appInfo.appTokenId << "> ";

    // streamInfo
    temp << "streamInfo:format(" << config.streamInfo.format << ") encoding(" << config.streamInfo.encoding <<
        ") channels(" << config.streamInfo.channels << ") samplingRate(" << config.streamInfo.samplingRate << ") ";

    // audioMode
    if (config.audioMode == AudioMode::AUDIO_MODE_PLAYBACK) {
        temp << "[rendererInfo]:streamUsage(" << config.rendererInfo.streamUsage << ") contentType(" <<
            config.rendererInfo.contentType << ") flag(" << config.rendererInfo.rendererFlags << ") ";
    } else {
        temp << "[capturerInfo]:sourceType(" << config.capturerInfo.sourceType << ") flag(" <<
            config.capturerInfo.capturerFlags << ") ";
    }

    temp << "streamType<" << config.streamType << ">";

    return temp.str();
}
} // namespace AudioStandard
} // namespace OHOS

