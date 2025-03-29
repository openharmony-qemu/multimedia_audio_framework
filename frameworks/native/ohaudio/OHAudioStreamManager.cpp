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

#include "OHAudioStreamManager.h"

using OHOS::AudioStandard::OHAudioStreamManager;
using OHOS::AudioStandard::AudioStreamManager;
using OHOS::AudioStandard::StreamUsage;
using OHOS::AudioStandard::AudioEncodingType;
using OHOS::AudioStandard::AudioSampleFormat;
using OHOS::AudioStandard::DirectPlaybackMode;

static OHOS::AudioStandard::OHAudioStreamManager *convertManager(OH_AudioStreamManager* manager)
{
    return (OHAudioStreamManager*) manager;
}

OH_AudioCommon_Result OH_AudioManager_GetAudioStreamManager(OH_AudioStreamManager **audioStreamManager)
{
    OHAudioStreamManager* ohAudioStreamManager = OHAudioStreamManager::GetInstance();
    *audioStreamManager = reinterpret_cast<OH_AudioStreamManager*>(ohAudioStreamManager);
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OH_AudioStreamManager_GetDirectPlaybackSupport(OH_AudioStreamManager *audioStreamManager,
    OH_AudioStream_EncodingType encodingType, OH_AudioStream_SampleFormat format, int32_t channelCount,
    int32_t samplingRate, OH_AudioStream_Usage usage, OH_AudioStream_DirectPlaybackMode *directPlaybackMode)
{
    OHAudioStreamManager* ohAudioStreamManager = convertManager(audioStreamManager);
    CHECK_AND_RETURN_RET_LOG(ohAudioStreamManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "ohAudioStreamManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(encodingType >= AUDIOSTREAM_ENCODING_TYPE_RAW && format >= AUDIOSTREAM_SAMPLE_U8 &&
        channelCount > 0 && samplingRate > 0 && usage > AUDIOSTREAM_USAGE_UNKNOWN,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "failed, invalid param");
    StreamUsage streamUsage = static_cast<StreamUsage>(usage);
    AudioEncodingType encoding = static_cast<AudioEncodingType>(encodingType);
    AudioSampleFormat sampleFormat = static_cast<AudioSampleFormat>(format);
    DirectPlaybackMode mode = ohAudioStreamManager->GetDirectPlaybackSupport(encoding, sampleFormat, channelCount,
        samplingRate, streamUsage);
    *directPlaybackMode = static_cast<OH_AudioStream_DirectPlaybackMode>(mode);
    return AUDIOCOMMON_RESULT_SUCCESS;
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

DirectPlaybackMode OHAudioStreamManager::GetDirectPlaybackSupport(AudioEncodingType encodingType,
    AudioSampleFormat format, int32_t channelCount, int32_t samplingRate, StreamUsage usage)
{
    CHECK_AND_RETURN_RET_LOG(audioStreamManager_ != nullptr,
        DIRECT_PLAYBACK_NOT_SUPPORTED, "failed, audioStreamManager_ is null");
    AudioStreamInfo streamInfo(static_cast<AudioSamplingRate>(samplingRate), encodingType, format,
        static_cast<AudioChannel>(channelCount));
    DirectPlaybackMode mode = audioStreamManager_->GetDirectPlaybackSupport(streamInfo, usage);
    return mode;
}

} // namespace AudioStandard
} // namespace OHOS
