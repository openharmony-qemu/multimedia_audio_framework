/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "OHAudioStreamManager"
#endif

#include "OHAudioCommon.h"
#include "OHAudioStreamManager.h"

namespace {
const std::set<OH_AudioStream_SourceType> VALID_OH_SOURCE_TYPES = {
    AUDIOSTREAM_SOURCE_TYPE_MIC,
    AUDIOSTREAM_SOURCE_TYPE_VOICE_RECOGNITION,
    AUDIOSTREAM_SOURCE_TYPE_PLAYBACK_CAPTURE,
    AUDIOSTREAM_SOURCE_TYPE_VOICE_CALL,
    AUDIOSTREAM_SOURCE_TYPE_VOICE_COMMUNICATION,
    AUDIOSTREAM_SOURCE_TYPE_VOICE_MESSAGE,
    AUDIOSTREAM_SOURCE_TYPE_CAMCORDER,
    AUDIOSTREAM_SOURCE_TYPE_UNPROCESSED,
    AUDIOSTREAM_SOURCE_TYPE_LIVE
};

const std::set<OH_AudioStream_Usage> VALID_OH_STREAM_USAGES = {
    AUDIOSTREAM_USAGE_UNKNOWN,
    AUDIOSTREAM_USAGE_MUSIC,
    AUDIOSTREAM_USAGE_VOICE_COMMUNICATION,
    AUDIOSTREAM_USAGE_VOICE_ASSISTANT,
    AUDIOSTREAM_USAGE_ALARM,
    AUDIOSTREAM_USAGE_VOICE_MESSAGE,
    AUDIOSTREAM_USAGE_RINGTONE,
    AUDIOSTREAM_USAGE_NOTIFICATION,
    AUDIOSTREAM_USAGE_ACCESSIBILITY,
    AUDIOSTREAM_USAGE_MOVIE,
    AUDIOSTREAM_USAGE_GAME,
    AUDIOSTREAM_USAGE_AUDIOBOOK,
    AUDIOSTREAM_USAGE_NAVIGATION,
    AUDIOSTREAM_USAGE_VIDEO_COMMUNICATION
};
}

using OHOS::AudioStandard::OHAudioStreamManager;
using OHOS::AudioStandard::AudioStreamManager;
using OHOS::AudioStandard::AudioStreamInfo;
using OHOS::AudioStandard::AudioSamplingRate;
using OHOS::AudioStandard::AudioEncodingType;
using OHOS::AudioStandard::AudioSampleFormat;
using OHOS::AudioStandard::AudioChannel;
using OHOS::AudioStandard::AudioChannelLayout;
using OHOS::AudioStandard::StreamUsage;
using OHOS::AudioStandard::DirectPlaybackMode;
using OHOS::AudioStandard::SourceType;

static OHOS::AudioStandard::OHAudioStreamManager *convertManager(OH_AudioStreamManager *manager)
{
    return (OHAudioStreamManager*) manager;
}

OH_AudioCommon_Result OH_AudioManager_GetAudioStreamManager(OH_AudioStreamManager **streamManager)
{
    CHECK_AND_RETURN_RET_LOG(streamManager != nullptr, AUDIOCOMMON_RESULT_ERROR_SYSTEM,
        "streamManager is nullptr");
    OHAudioStreamManager *ohAudioStreamManager = OHAudioStreamManager::GetInstance();
    *streamManager = reinterpret_cast<OH_AudioStreamManager*>(ohAudioStreamManager);
    return AUDIOCOMMON_RESULT_SUCCESS;
}

static OHOS::AudioStandard::AudioStreamInfo convertStreamInfo(OH_AudioStreamInfo *streamInfo)
{
    return AudioStreamInfo(static_cast<AudioSamplingRate>(streamInfo->samplingRate),
        static_cast<AudioEncodingType>(streamInfo->encodingType),
        static_cast<AudioSampleFormat>(streamInfo->sampleFormat),
        OHOS::AudioStandard::OHAudioCommon::ConvertLayoutToChannel(streamInfo->channelLayout),
        static_cast<AudioChannelLayout>(streamInfo->channelLayout));
}

OH_AudioCommon_Result OH_AudioStreamManager_GetDirectPlaybackSupport(
    OH_AudioStreamManager *audioStreamManager, OH_AudioStreamInfo *streamInfo,
    OH_AudioStream_Usage usage, OH_AudioStream_DirectPlaybackMode *directPlaybackMode)
{
    OHAudioStreamManager *ohAudioStreamManager = convertManager(audioStreamManager);
    CHECK_AND_RETURN_RET_LOG(ohAudioStreamManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "ohAudioStreamManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(streamInfo != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "streamInfo is nullptr");
    CHECK_AND_RETURN_RET_LOG(usage > AUDIOSTREAM_USAGE_UNKNOWN, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM,
        "usage is invalid");
    CHECK_AND_RETURN_RET_LOG(directPlaybackMode != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM,
        "directPlaybackMode is nullptr");
    AudioStreamInfo info = convertStreamInfo(streamInfo);
    *directPlaybackMode = ohAudioStreamManager->GetDirectPlaybackSupport(info, static_cast<StreamUsage>(usage));
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OH_AudioStreamManager_IsAcousticEchoCancelerSupported(OH_AudioStreamManager *audioStreamManager,
    OH_AudioStream_SourceType sourceType, bool *supported)
{
    CHECK_AND_RETURN_RET_LOG(supported != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "supported is nullptr");
    OHAudioStreamManager *ohAudioStreamManager = convertManager(audioStreamManager);
    CHECK_AND_RETURN_RET_LOG(ohAudioStreamManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "ohAudioStreamManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(VALID_OH_SOURCE_TYPES.count(sourceType) != 0,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "sourceType is invalid");
    SourceType type = static_cast<SourceType>(sourceType);
    *supported = ohAudioStreamManager->IsAcousticEchoCancelerSupported(type);
    return AUDIOCOMMON_RESULT_SUCCESS;
}

bool OH_AudioStreamManager_IsFastPlaybackSupported(
    OH_AudioStreamManager *streamManager, OH_AudioStreamInfo *streamInfo, OH_AudioStream_Usage usage)
{
    OHAudioStreamManager* ohAudioStreamManager = convertManager(streamManager);
    if (ohAudioStreamManager == nullptr || streamInfo == nullptr || !VALID_OH_STREAM_USAGES.count(usage)) {
        AUDIO_ERR_LOG("Invalid params!");
        return false;
    }

    StreamUsage streamUsage = static_cast<StreamUsage>(usage);
    AudioStreamInfo audioStreamInfo = {
        (AudioSamplingRate)streamInfo->samplingRate,
        (AudioEncodingType)streamInfo->encodingType,
        (AudioSampleFormat)streamInfo->sampleFormat,
        AudioChannel::STEREO,
        (AudioChannelLayout)streamInfo->channelLayout
    };
    return ohAudioStreamManager->IsFastPlaybackSupported(audioStreamInfo, streamUsage);
}

bool OH_AudioStreamManager_IsFastRecordingSupported(
    OH_AudioStreamManager *streamManager, OH_AudioStreamInfo *streamInfo, OH_AudioStream_SourceType source)
{
    OHAudioStreamManager* ohAudioStreamManager = convertManager(streamManager);
    if (ohAudioStreamManager == nullptr || streamInfo == nullptr || !VALID_OH_SOURCE_TYPES.count(source)) {
        AUDIO_ERR_LOG("Invalid params!");
        return false;
    }

    SourceType sourceType = static_cast<SourceType>(source);
    AudioStreamInfo audioStreamInfo = {
        (AudioSamplingRate)streamInfo->samplingRate,
        (AudioEncodingType)streamInfo->encodingType,
        (AudioSampleFormat)streamInfo->sampleFormat,
        AudioChannel::STEREO,
        (AudioChannelLayout)streamInfo->channelLayout
    };
    return ohAudioStreamManager->IsFastRecordingSupported(audioStreamInfo, sourceType);
}

bool OH_AudioStreamManager_IsIntelligentNoiseReductionEnabledForCurrentDevice(
    OH_AudioStreamManager *streamManager, OH_AudioStream_SourceType source)
{
    OHAudioStreamManager *ohAudioStreamManager = convertManager(streamManager);
    CHECK_AND_RETURN_RET_LOG(ohAudioStreamManager != nullptr,
        false, "ohAudioStreamManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(VALID_OH_SOURCE_TYPES.count(source) != 0,
        false, "sourceType is invalid");
    SourceType sourceType = static_cast<SourceType>(source);
    return ohAudioStreamManager->IsIntelligentNoiseReductionEnabledForCurrentDevice(sourceType);
}

namespace OHOS {
namespace AudioStandard {

OHAudioStreamManager::OHAudioStreamManager()
{
    AUDIO_INFO_LOG("OHAudioStreamManager created!");
}

OHAudioStreamManager::~OHAudioStreamManager()
{
    AUDIO_INFO_LOG("OHAudioStreamManager destroyed!");
}

OH_AudioStream_DirectPlaybackMode OHAudioStreamManager::GetDirectPlaybackSupport(AudioStreamInfo streamInfo,
    StreamUsage usage)
{
    CHECK_AND_RETURN_RET_LOG(audioStreamManager_ != nullptr, AUDIOSTREAM_DIRECT_PLAYBACK_NOT_SUPPORTED,
        "failed, audioStreamManager_ is null");
    DirectPlaybackMode mode = audioStreamManager_->GetDirectPlaybackSupport(streamInfo, usage);
    return static_cast<OH_AudioStream_DirectPlaybackMode>(mode);
}

bool OHAudioStreamManager::IsAcousticEchoCancelerSupported(SourceType sourceType)
{
    CHECK_AND_RETURN_RET_LOG(audioStreamManager_ != nullptr, false, "failed, audioStreamManager_ is null");
    return audioStreamManager_->IsAcousticEchoCancelerSupported(sourceType);
}

bool OHAudioStreamManager::IsFastPlaybackSupported(AudioStreamInfo &streamInfo, StreamUsage usage)
{
    CHECK_AND_RETURN_RET_LOG(audioStreamManager_ != nullptr, false, "failed, audioStreamManager_ is null");
    return audioStreamManager_->IsFastPlaybackSupported(streamInfo, usage);
}

bool OHAudioStreamManager::IsFastRecordingSupported(AudioStreamInfo &streamInfo, SourceType source)
{
    CHECK_AND_RETURN_RET_LOG(audioStreamManager_ != nullptr, false, "failed, audioStreamManager_ is null");
    return audioStreamManager_->IsFastRecordingSupported(streamInfo, source);
}

bool OHAudioStreamManager::IsIntelligentNoiseReductionEnabledForCurrentDevice(SourceType sourceType)
{
    CHECK_AND_RETURN_RET_LOG(audioStreamManager_ != nullptr, false, "failed, audioStreamManager_ is null");
    return audioStreamManager_->IsIntelligentNoiseReductionEnabledForCurrentDevice(sourceType);
}
} // namespace AudioStandard
} // namespace OHOS
