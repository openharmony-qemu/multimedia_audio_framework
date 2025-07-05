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

    temp << "streamType<" << config.streamType << "> ";

    temp << "originalSessionId<" << config.originalSessionId << ">";

    return temp.str();
}
} // namespace AudioStandard
} // namespace OHOS

