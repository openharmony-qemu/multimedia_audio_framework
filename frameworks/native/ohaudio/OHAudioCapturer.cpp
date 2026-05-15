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
#include <memory>
#include <cstdint>
#ifndef LOG_TAG
#define LOG_TAG "OHAudioCapturer"
#endif

#include "OHAudioCapturer.h"
#include "audio_errors.h"
#include "audio_utils.h"
#include "OHAudioDeviceDescriptor.h"
#include "OHAudioCommon.h"
#include "stream_dfx_manager.h"
#include <unistd.h>
static const int32_t START_RESULT_SUCCESS = 0;

using OHOS::AudioStandard::Timestamp;

static const int64_t SECOND_TO_NANOSECOND = 1000000000;

static OHOS::AudioStandard::OHAudioCapturer *convertCapturer(OH_AudioCapturer* capturer)
{
    return (OHOS::AudioStandard::OHAudioCapturer*) capturer;
}

OH_AudioStream_Result OH_AudioCapturer_Release(OH_AudioCapturer* capturer)
{
    AUDIO_INFO_LOG("in");
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM,
        OHOS::AudioStandard::StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            OHOS::AudioStandard::RECORD_RELEASE_ILLEGAL_STATE, "convert capturer failed", true),
        "convert capturer failed");
    if (audioCapturer->Release()) {
        OHOS::AudioStandard::ObjectRefMap<OHOS::AudioStandard::OHAudioCapturer>::DecreaseRef(audioCapturer);
        return AUDIOSTREAM_SUCCESS;
    } else {
        OHOS::AudioStandard::StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            OHOS::AudioStandard::RECORD_RELEASE_ILLEGAL_STATE, "Release failed", true);
        return AUDIOSTREAM_ERROR_ILLEGAL_STATE;
    }
}

OH_AudioStream_Result OH_AudioCapturer_Start(OH_AudioCapturer* capturer)
{
    AUDIO_INFO_LOG("in");
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM,
        OHOS::AudioStandard::StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            OHOS::AudioStandard::RECORD_START_ILLEGAL_STATE, "convert capturer failed", true),
        "convert capturer failed");
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioCapturer->IsModernInnerCapturer() != true, AUDIOSTREAM_ERROR_ILLEGAL_STATE,
        OHOS::AudioStandard::StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            OHOS::AudioStandard::RECORD_START_ILLEGAL_STATE, "Innercapturer can not use this interface", true),
        "Innercapturer can not use this interface");
    if (audioCapturer->Start()) {
        return AUDIOSTREAM_SUCCESS;
    } else {
        OHOS::AudioStandard::StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            OHOS::AudioStandard::RECORD_START_ILLEGAL_STATE, "Start failed", true);
        return AUDIOSTREAM_ERROR_ILLEGAL_STATE;
    }
}

OH_AudioStream_Result OH_AudioCapturer_RequestPlaybackCaptureStart(OH_AudioCapturer* capturer,
    OH_AudioCapturer_OnPlaybackCaptureStartCallback callback, void* userData)
{
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM,
        OHOS::AudioStandard::StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            OHOS::AudioStandard::RECORD_START_ILLEGAL_STATE, "convert capturer failed", true),
        "convert capturer failed");
    int32_t startResult = audioCapturer->StartPlaybackCapture(capturer, callback, userData);
    if (startResult == START_RESULT_SUCCESS) {
        return AUDIOSTREAM_SUCCESS;
    } else {
        OHOS::AudioStandard::StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            OHOS::AudioStandard::RECORD_START_OPERATION_FAILED, "StartPlaybackCapture failed", true);
        return AUDIOSTREAM_ERROR_ILLEGAL_STATE;
    }
}

OH_AudioStream_Result OH_AudioCapturer_Pause(OH_AudioCapturer* capturer)
{
    AUDIO_INFO_LOG("in");
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM,
        OHOS::AudioStandard::StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            OHOS::AudioStandard::RECORD_PAUSE_ILLEGAL_STATE, "convert capturer failed", true),
        "convert capturer failed");

    if (audioCapturer->Pause()) {
        return AUDIOSTREAM_SUCCESS;
    } else {
        OHOS::AudioStandard::StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            OHOS::AudioStandard::RECORD_PAUSE_ILLEGAL_STATE, "Pause failed", true);
        return AUDIOSTREAM_ERROR_ILLEGAL_STATE;
    }
}

OH_AudioStream_Result OH_AudioCapturer_Stop(OH_AudioCapturer* capturer)
{
    AUDIO_INFO_LOG("in");
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM,
        OHOS::AudioStandard::StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            OHOS::AudioStandard::RECORD_STOP_ILLEGAL_STATE, "convert capturer failed", true),
        "convert capturer failed");

    if (audioCapturer->Stop()) {
        return AUDIOSTREAM_SUCCESS;
    } else {
        OHOS::AudioStandard::StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            OHOS::AudioStandard::RECORD_STOP_ILLEGAL_STATE, "Stop failed", true);
        return AUDIOSTREAM_ERROR_ILLEGAL_STATE;
    }
}

OH_AudioStream_Result OH_AudioCapturer_SetInputDevice(
    OH_AudioCapturer* capturer, OH_AudioDevice_Type deviceType)
{
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM,
        OHOS::AudioStandard::StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            OHOS::AudioStandard::ERR_RECORD_DEVICE_SWITCH_INVALID_PARAM, "convert Capturer failed", true),
        "convert Capturer failed");
    int32_t ret = audioCapturer->SetInputDevice((OHOS::AudioStandard::DeviceType)deviceType);
    if (ret == OHOS::AudioStandard::ERR_NOT_SUPPORTED) {
        OHOS::AudioStandard::StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            OHOS::AudioStandard::ERR_RECORD_DEVICE_SWITCH_ILLEGAL_STATE,
            "This audioCapturer can not reset the input device", true);
        AUDIO_ERR_LOG("This audioCapturer can not reset the input device");
        return AUDIOSTREAM_ERROR_ILLEGAL_STATE;
    } else if (ret != AUDIOSTREAM_SUCCESS) {
        OHOS::AudioStandard::StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            OHOS::AudioStandard::ERR_RECORD_DEVICE_SWITCH_OPERATION_FAILED,
            "system error when calling this function", true);
        AUDIO_ERR_LOG("system error when calling this function");
        return AUDIOSTREAM_ERROR_SYSTEM;
    }
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioCapturer_SetMuteHint(OH_AudioCapturer* capturer, bool mute)
{
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM,
        OHOS::AudioStandard::StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            OHOS::AudioStandard::RECORD_CONFIG_INVALID_PARAM, "convert Capturer failed", true),
        "convert Capturer failed");
    int32_t ret = audioCapturer->SetMuteHint(mute);
    if (ret == OHOS::AudioStandard::ERROR_ILLEGAL_STATE || ret == OHOS::AudioStandard::ERR_ILLEGAL_STATE) {
        OHOS::AudioStandard::StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            OHOS::AudioStandard::RECORD_CONFIG_ILLEGAL_STATE, "SetMuteHint illegal state", true);
        return AUDIOSTREAM_ERROR_ILLEGAL_STATE;
    }
    if (ret == OHOS::AudioStandard::ERROR_INVALID_PARAM || ret == OHOS::AudioStandard::ERR_INVALID_PARAM) {
        OHOS::AudioStandard::StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            OHOS::AudioStandard::RECORD_CONFIG_INVALID_PARAM, "SetMuteHint invalid param", true);
        return AUDIOSTREAM_ERROR_INVALID_PARAM;
    }
    return ret == OHOS::AudioStandard::SUCCESS ? AUDIOSTREAM_SUCCESS : AUDIOSTREAM_ERROR_SYSTEM;
}

OH_AudioStream_Result OH_AudioCapturer_GetFastStatus(OH_AudioCapturer *capturer,
    OH_AudioStream_FastStatus *status)
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(capturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM,
        OHOS::AudioStandard::StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            OHOS::AudioStandard::RECORD_QUERY_ILLEGAL_STATE, "capturer is nullptr", true),
        "capturer is nullptr");
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(status != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM,
        OHOS::AudioStandard::StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            OHOS::AudioStandard::RECORD_QUERY_ILLEGAL_STATE, "status is nullptr", true),
        "status is nullptr");
    OHOS::AudioStandard::FastStatus fastStatus = audioCapturer->GetFastStatus();
    if (fastStatus == OHOS::AudioStandard::FastStatus::FASTSTATUS_INVALID) {
        OHOS::AudioStandard::StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            OHOS::AudioStandard::RECORD_QUERY_ILLEGAL_STATE, "This audiocapturer can not get the fast status", true);
        AUDIO_ERR_LOG("This audiocapturer can not get the fast status");
        return AUDIOSTREAM_ERROR_ILLEGAL_STATE;
    }
    *status = (OH_AudioStream_FastStatus)fastStatus;
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioCapturer_Flush(OH_AudioCapturer* capturer)
{
    AUDIO_INFO_LOG("in");
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM,
        OHOS::AudioStandard::StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            OHOS::AudioStandard::RECORD_FLUSH_ILLEGAL_STATE, "convert capturer failed", true),
        "convert capturer failed");

    if (audioCapturer->Flush()) {
        return AUDIOSTREAM_SUCCESS;
    } else {
        OHOS::AudioStandard::StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            OHOS::AudioStandard::RECORD_FLUSH_ILLEGAL_STATE, "Flush failed", true);
        return AUDIOSTREAM_ERROR_ILLEGAL_STATE;
    }
}


OH_AudioStream_Result OH_AudioCapturer_GetCurrentState(OH_AudioCapturer* capturer, OH_AudioStream_State* state)
{
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM,
        OHOS::AudioStandard::StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            OHOS::AudioStandard::RECORD_QUERY_ILLEGAL_STATE, "convert capturer failed", true),
        "convert capturer failed");

    OHOS::AudioStandard::CapturerState capturerState = audioCapturer->GetCurrentState();
    *state = (OH_AudioStream_State)capturerState;
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioCapturer_GetLatencyMode(OH_AudioCapturer* capturer,
    OH_AudioStream_LatencyMode* latencyMode)
{
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM,
        OHOS::AudioStandard::StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            OHOS::AudioStandard::RECORD_QUERY_ILLEGAL_STATE, "convert capturer failed", true),
        "convert capturer failed");
    OHOS::AudioStandard::AudioCapturerInfo capturerInfo;
    audioCapturer->GetCapturerInfo(capturerInfo);
    *latencyMode = (OH_AudioStream_LatencyMode)capturerInfo.capturerFlags;
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioCapturer_GetStreamId(OH_AudioCapturer* capturer, uint32_t* streamId)
{
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM,
        OHOS::AudioStandard::StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            OHOS::AudioStandard::RECORD_QUERY_ILLEGAL_STATE, "convert capturer failed", true),
        "convert capturer failed");
    audioCapturer->GetStreamId(*streamId);
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioCapturer_GetChannelCount(OH_AudioCapturer* capturer, int32_t* channelCount)
{
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM,
        OHOS::AudioStandard::StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            OHOS::AudioStandard::RECORD_QUERY_ILLEGAL_STATE, "convert capturer failed", true),
        "convert capturer failed");
    *channelCount = audioCapturer->GetChannelCount();
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioCapturer_GetSamplingRate(OH_AudioCapturer* capturer, int32_t* rate)
{
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM,
        OHOS::AudioStandard::StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            OHOS::AudioStandard::RECORD_QUERY_ILLEGAL_STATE, "convert capturer failed", true),
        "convert capturer failed");

    *rate = audioCapturer->GetSamplingRate();
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioCapturer_GetSampleFormat(OH_AudioCapturer* capturer,
    OH_AudioStream_SampleFormat* sampleFormat)
{
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM,
        OHOS::AudioStandard::StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            OHOS::AudioStandard::RECORD_QUERY_ILLEGAL_STATE, "convert capturer failed", true),
        "convert capturer failed");
    *sampleFormat = (OH_AudioStream_SampleFormat)audioCapturer->GetSampleFormat();
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioCapturer_GetEncodingType(OH_AudioCapturer* capturer,
    OH_AudioStream_EncodingType* encodingType)
{
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM,
        OHOS::AudioStandard::StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            OHOS::AudioStandard::RECORD_QUERY_ILLEGAL_STATE, "convert capturer failed", true),
        "convert capturer failed");
    *encodingType = (OH_AudioStream_EncodingType)audioCapturer->GetEncodingType();
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioCapturer_GetCapturerInfo(OH_AudioCapturer* capturer,
    OH_AudioStream_SourceType* sourceType)
{
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM,
        OHOS::AudioStandard::StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            OHOS::AudioStandard::RECORD_QUERY_ILLEGAL_STATE, "convert capturer failed", true),
        "convert capturer failed");
    OHOS::AudioStandard::AudioCapturerInfo capturerInfo;
    audioCapturer->GetCapturerInfo(capturerInfo);
    *sourceType = (OH_AudioStream_SourceType)capturerInfo.sourceType;
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioCapturer_GetFrameSizeInCallback(OH_AudioCapturer* capturer, int32_t* frameSize)
{
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM,
        OHOS::AudioStandard::StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            OHOS::AudioStandard::RECORD_QUERY_ILLEGAL_STATE, "convert capturer failed", true),
        "convert capturer failed");

    *frameSize = audioCapturer->GetFrameSizeInCallback();
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioCapturer_GetTimestamp(OH_AudioCapturer* capturer,
    clockid_t clockId, int64_t* framePosition, int64_t* timestamp)
{
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM,
        OHOS::AudioStandard::StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            OHOS::AudioStandard::RECORD_QUERY_ILLEGAL_STATE, "convert capturer failed", true),
        "convert capturer failed");
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(clockId == CLOCK_MONOTONIC, AUDIOSTREAM_ERROR_INVALID_PARAM,
        OHOS::AudioStandard::StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            OHOS::AudioStandard::RECORD_QUERY_ILLEGAL_STATE, "error clockId value", true),
        "error clockId value");

    Timestamp stamp;
    Timestamp::Timestampbase base = Timestamp::Timestampbase::MONOTONIC;
    bool ret = audioCapturer->GetAudioTime(stamp, base);
    if (!ret) {
        OHOS::AudioStandard::StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            OHOS::AudioStandard::RECORD_QUERY_ILLEGAL_STATE, "GetAudioTime error", true);
        AUDIO_ERR_LOG("GetAudioTime error!");
        return AUDIOSTREAM_ERROR_ILLEGAL_STATE;
    }
    *framePosition = stamp.framePosition;
    *timestamp = stamp.time.tv_sec * SECOND_TO_NANOSECOND + stamp.time.tv_nsec;
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioCapturer_GetFramesRead(OH_AudioCapturer* capturer, int64_t* frames)
{
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM,
        OHOS::AudioStandard::StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            OHOS::AudioStandard::RECORD_QUERY_ILLEGAL_STATE, "convert capturer failed", true),
        "convert capturer failed");

    *frames = audioCapturer->GetFramesRead();
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioCapturer_SetIndependentAudioSessionStrategy(
    OH_AudioCapturer* capturer, const OH_AudioSession_Strategy *strategy, uint32_t behavior)
{
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM,
        OHOS::AudioStandard::StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            OHOS::AudioStandard::RECORD_CONFIG_INVALID_PARAM, "convert capturer failed", true),
        "convert capturer failed");
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(strategy != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM,
        OHOS::AudioStandard::StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            OHOS::AudioStandard::RECORD_CONFIG_INVALID_PARAM, "strategy is nullptr", true),
        "strategy is nullptr");
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(
        (behavior & ~static_cast<uint32_t>(OHOS::AudioStandard::AudioSessionBehaviorFlags::MASK_VALID_BEHAVIOR)) == 0,
        AUDIOSTREAM_ERROR_INVALID_PARAM,
        OHOS::AudioStandard::StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            OHOS::AudioStandard::RECORD_CONFIG_INVALID_PARAM, "param behavior is invalid", true),
        "param behavior is invalid");

    OHOS::AudioStandard::AudioSessionStrategy audioStrategy;
    audioStrategy.concurrencyMode =
        static_cast<OHOS::AudioStandard::AudioConcurrencyMode>(strategy->concurrencyMode);
    switch (audioStrategy.concurrencyMode) {
        case OHOS::AudioStandard::AudioConcurrencyMode::DEFAULT:
        case OHOS::AudioStandard::AudioConcurrencyMode::MIX_WITH_OTHERS:
        case OHOS::AudioStandard::AudioConcurrencyMode::DUCK_OTHERS:
        case OHOS::AudioStandard::AudioConcurrencyMode::PAUSE_OTHERS:
            break;
        default:
            OHOS::AudioStandard::StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                OHOS::AudioStandard::RECORD_CONFIG_INVALID_PARAM, "concurrencyMode is invalid", true);
            AUDIO_ERR_LOG("concurrencyMode is invalid");
            return AUDIOSTREAM_ERROR_INVALID_PARAM;
    }
    int32_t ret = audioCapturer->SetIndependentAudioSessionStrategy(audioStrategy, behavior);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == OHOS::AudioStandard::SUCCESS,
        AUDIOSTREAM_ERROR_ILLEGAL_STATE,
        OHOS::AudioStandard::StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            OHOS::AudioStandard::RECORD_CONFIG_ILLEGAL_STATE, "Set independent audio session strategy failed", true),
        "Set independent audio session strategy failed");
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioCapturer_GetOverflowCount(OH_AudioCapturer* capturer, uint32_t* count)
{
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM,
        OHOS::AudioStandard::StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            OHOS::AudioStandard::RECORD_QUERY_ILLEGAL_STATE, "convert capturer failed", true),
        "convert capturer failed");

    *count = audioCapturer->GetOverflowCount();
    return AUDIOSTREAM_SUCCESS;
}

namespace OHOS {
namespace AudioStandard {
OHAudioCapturer::OHAudioCapturer()
{
    AUDIO_INFO_LOG("OHAudioCapturer created!");
}

OHAudioCapturer::~OHAudioCapturer()
{
    AUDIO_INFO_LOG("OHAudioCapturer destroyed!");
}

bool OHAudioCapturer::Initialize(const AudioCapturerOptions& capturerOptions)
{
    audioCapturer_ = AudioCapturer::CreateCapturer(capturerOptions);
    isModernInnerCapturer_ = capturerOptions.playbackCaptureConfig.isModernInnerCapturer;
    return audioCapturer_ != nullptr;
}

bool OHAudioCapturer::Start()
{
    if (audioCapturer_ == nullptr) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_START_ILLEGAL_STATE, "capturer client is nullptr", true);
        AUDIO_ERR_LOG("capturer client is nullptr");
        return false;
    }
    return audioCapturer_->Start();
}

bool OHAudioCapturer::Pause()
{
    if (audioCapturer_ == nullptr) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_PAUSE_ILLEGAL_STATE, "capturer client is nullptr", true);
        AUDIO_ERR_LOG("capturer client is nullptr");
        return false;
    }
    return audioCapturer_->Pause();
}

bool OHAudioCapturer::Stop()
{
    if (audioCapturer_ == nullptr) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_STOP_ILLEGAL_STATE, "capturer client is nullptr", true);
        AUDIO_ERR_LOG("capturer client is nullptr");
        return false;
    }
    return audioCapturer_->Stop();
}

bool OHAudioCapturer::Flush()
{
    if (audioCapturer_ == nullptr) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_FLUSH_ILLEGAL_STATE, "capturer client is nullptr", true);
        AUDIO_ERR_LOG("capturer client is nullptr");
        return false;
    }
    return audioCapturer_->Flush();
}

bool OHAudioCapturer::Release()
{
    if (audioCapturer_ == nullptr) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_RELEASE_ILLEGAL_STATE, "capturer client is nullptr", true);
        AUDIO_ERR_LOG("capturer client is nullptr");
        return false;
    }

    if (!audioCapturer_->Release()) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_RELEASE_ILLEGAL_STATE, "Release failed", true);
        return false;
    }
    audioCapturer_ = nullptr;
    audioCapturerCallback_= nullptr;
    return true;
}

CapturerState OHAudioCapturer::GetCurrentState()
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioCapturer_ != nullptr, CAPTURER_INVALID,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_ILLEGAL_STATE, "capturer client is nullptr", true),
        "capturer client is nullptr");
    return audioCapturer_->GetStatus();
}

void OHAudioCapturer::GetStreamId(uint32_t &streamId)
{
    CHECK_AND_CALL_FUNC_RETURN_LOG(audioCapturer_ != nullptr,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_ILLEGAL_STATE, "capturer client is nullptr", true),
        "capturer client is nullptr");
    audioCapturer_->GetAudioStreamId(streamId);
}

AudioChannel OHAudioCapturer::GetChannelCount()
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioCapturer_ != nullptr, MONO,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_ILLEGAL_STATE, "capturer client is nullptr", true),
        "capturer client is nullptr");
    AudioCapturerParams params;
    audioCapturer_->GetParams(params);
    return params.audioChannel;
}

int32_t OHAudioCapturer::GetSamplingRate()
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioCapturer_ != nullptr, MONO,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_ILLEGAL_STATE, "capturer client is nullptr", true),
        "capturer client is nullptr");
    AudioCapturerParams params;
    audioCapturer_->GetParams(params);
    return params.samplingRate;
}

AudioEncodingType OHAudioCapturer::GetEncodingType()
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioCapturer_ != nullptr, ENCODING_INVALID,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_ILLEGAL_STATE, "capturer client is nullptr", true),
        "capturer client is nullptr");
    AudioCapturerParams params;
    audioCapturer_->GetParams(params);
    return params.audioEncoding;
}

void OHAudioCapturer::GetCapturerInfo(AudioCapturerInfo& capturerInfo)
{
    CHECK_AND_CALL_FUNC_RETURN_LOG(audioCapturer_ != nullptr,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_ILLEGAL_STATE, "capturer client is nullptr", true),
        "capturer client is nullptr");
    audioCapturer_->GetCapturerInfo(capturerInfo);
}

AudioSampleFormat OHAudioCapturer::GetSampleFormat()
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioCapturer_ != nullptr, INVALID_WIDTH,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_ILLEGAL_STATE, "capturer client is nullptr", true),
        "capturer client is nullptr");
    AudioCapturerParams params;
    audioCapturer_->GetParams(params);
    return params.audioSampleFormat;
}

void OHAudioCapturer::SetCapturerCallback(CapturerCallback capturerCallbacks, void *userData)
{
    CHECK_AND_CALL_FUNC_RETURN_LOG(audioCapturer_ != nullptr,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_ILLEGAL_STATE, "capturer client is nullptr", true),
        "capturer client is nullptr");
    audioCapturer_->SetCaptureMode(CAPTURE_MODE_CALLBACK);

    SetInterruptCallback(capturerCallbacks, userData);
    SetErrorCallback(capturerCallbacks, userData);
    SetReadDataCallback(capturerCallbacks, userData);
    SetStreamEventCallback(capturerCallbacks, userData);
}

int64_t OHAudioCapturer::GetFramesRead()
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioCapturer_ != nullptr, ERROR,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_ILLEGAL_STATE, "capturer client is nullptr", true),
        "capturer client is nullptr");
    return audioCapturer_->GetFramesRead();
}

bool OHAudioCapturer::GetAudioTime(Timestamp &timestamp, Timestamp::Timestampbase base)
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioCapturer_ != nullptr, false,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_ILLEGAL_STATE, "capturer client is nullptr", true),
        "capturer client is nullptr");
    return audioCapturer_->GetAudioTime(timestamp, base);
}

int32_t OHAudioCapturer::GetFrameSizeInCallback()
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioCapturer_ != nullptr, ERROR,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_ILLEGAL_STATE, "capturer client is nullptr", true),
        "capturer client is nullptr");
    uint32_t frameSize;
    audioCapturer_->GetFrameCount(frameSize);
    return static_cast<int32_t>(frameSize);
}

int32_t OHAudioCapturer::SetInputDevice(DeviceType deviceType)
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioCapturer_ != nullptr, ERROR,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            OHOS::AudioStandard::ERR_RECORD_DEVICE_SWITCH_NULL_POINTER, "capturer client is nullptr", true),
        "capturer client is nullptr");
    return audioCapturer_->SetInputDevice(deviceType);
}

int32_t OHAudioCapturer::SetMuteHint(bool mute)
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioCapturer_ != nullptr, ERROR,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CONFIG_ILLEGAL_STATE, "capturer client is nullptr", true),
        "capturer client is nullptr");
    return audioCapturer_->SetMuteHint(mute);
}

FastStatus OHAudioCapturer::GetFastStatus()
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioCapturer_ != nullptr, FASTSTATUS_INVALID,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_ILLEGAL_STATE, "capturer client is nullptr", true),
        "capturer client is nullptr");
    return audioCapturer_->GetFastStatus();
}

int32_t OHAudioCapturer::GetBufferDesc(BufferDesc &bufDesc) const
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioCapturer_ != nullptr, ERROR,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_SEND_DATA_ILLEGAL_STATE, "capturer client is nullptr", true),
        "capturer client is nullptr");
    return audioCapturer_->GetBufferDesc(bufDesc);
}

int32_t OHAudioCapturer::Enqueue(const BufferDesc &bufDesc) const
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioCapturer_ != nullptr, ERROR,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_SEND_DATA_ILLEGAL_STATE, "capturer client is nullptr", true),
        "capturer client is nullptr");
    return audioCapturer_->Enqueue(bufDesc);
}

int32_t OHAudioCapturer::SelectInputDevice(OH_AudioDeviceDescriptor *desc) const
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioCapturer_ != nullptr, ERR_INVALID_PARAM,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_DEVICE_SWITCH_ILLEGAL_STATE, "capturer client is nullptr", true),
        "capturer client is nullptr");
    return audioCapturer_->SelectInputDevice(OHAudioCommon::ConvertDevice(desc));
}

uint32_t OHAudioCapturer::GetOverflowCount() const
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioCapturer_ != nullptr, ERROR,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_ILLEGAL_STATE, "capturer client is nullptr", true),
        "capturer client is nullptr");
    return audioCapturer_->GetOverflowCount();
}

OH_AudioStream_Result OHAudioCapturerErrorCallback::GetErrorResult(AudioErrors errorCode) const
{
    switch (errorCode) {
        case ERROR_ILLEGAL_STATE:
            return AUDIOSTREAM_ERROR_ILLEGAL_STATE;
        case ERROR_INVALID_PARAM:
            return AUDIOSTREAM_ERROR_INVALID_PARAM;
        case ERROR_SYSTEM:
            return AUDIOSTREAM_ERROR_SYSTEM;
        default:
            return AUDIOSTREAM_ERROR_SYSTEM;
    }
}

void OHAudioCapturerErrorCallback::OnError(AudioErrors errorCode)
{
    OHAudioCapturer* audioCapturer = (OHAudioCapturer*)ohAudioCapturer_;
    CHECK_AND_CALL_FUNC_RETURN_LOG(audioCapturer != nullptr,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_ILLEGAL_STATE, "capturer client is nullptr", true),
        "capturer client is nullptr");
    if (audioCapturer->GetCapturerErrorCallbackType() == ERROR_CALLBACK_COMBINED &&
        callbacks_.OH_AudioCapturer_OnError != nullptr) {
        OH_AudioStream_Result error = GetErrorResult(errorCode);
        callbacks_.OH_AudioCapturer_OnError(ohAudioCapturer_, userData_, error);
    }

    if (audioCapturer->GetCapturerErrorCallbackType() == ERROR_CALLBACK_SEPERATED &&
        errorCallback_ != nullptr) {
        OH_AudioStream_Result error = GetErrorResult(errorCode);
        errorCallback_(ohAudioCapturer_, userData_, error);
    }
}

void OHCapturerServiceDiedCallback::OnAudioPolicyServiceDied()
{
    CHECK_AND_CALL_FUNC_RETURN_LOG(ohAudioCapturer_ != nullptr,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_ILLEGAL_STATE, "renderer client is nullptr", true),
        "renderer client is nullptr");
    OHAudioCapturer* audioCapturer = (OHAudioCapturer*)ohAudioCapturer_;
    if (audioCapturer->GetCapturerErrorCallbackType() == ERROR_CALLBACK_SEPERATED &&
        errorCallback_ != nullptr) {
        OH_AudioStream_Result error = AUDIOSTREAM_ERROR_SYSTEM;
        errorCallback_(ohAudioCapturer_, userData_, error);
    }
}

void OHAudioCapturerModeCallback::OnReadData(size_t length)
{
    OHAudioCapturer* audioCapturer = (OHAudioCapturer*)ohAudioCapturer_;
    OHOS::AudioStandard::ObjectRefMap objectGuard(audioCapturer);
    CHECK_AND_CALL_FUNC_RETURN_LOG(audioCapturer != nullptr,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_ILLEGAL_STATE, "capturer client is nullptr", true),
        "capturer client is nullptr");
    CHECK_AND_CALL_FUNC_RETURN_LOG((callbacks_.OH_AudioCapturer_OnReadData != nullptr) ||
        onReadDataCallback_ != nullptr,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_INVALID_PARAM, "pointer to the fuction is nullptr", true),
        "pointer to the fuction is nullptr");
    BufferDesc bufDesc;
    int32_t ret = audioCapturer->GetBufferDesc(bufDesc);
    if (ret != SUCCESS) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_SEND_DATA_OPERATION_FAILED, "get bufDesc failed", true);
        AUDIO_ERR_LOG("get bufDesc failed, bufLength=%{public}zu, dataLength=%{public}zu",
            bufDesc.bufLength, bufDesc.dataLength);
        return;
    }

    if (audioCapturer->GetCapturerReadDataCallbackType() == READ_DATA_CALLBACK_WITHOUT_RESULT &&
        callbacks_.OH_AudioCapturer_OnReadData != nullptr) {
        callbacks_.OH_AudioCapturer_OnReadData(ohAudioCapturer_, userData_,
            (void*)bufDesc.buffer, bufDesc.bufLength);
    }
    if (audioCapturer->GetCapturerReadDataCallbackType() == READ_DATA_CALLBACK_WITH_RESULT &&
        onReadDataCallback_ != nullptr) {
        onReadDataCallback_(ohAudioCapturer_, userData_, (void*)bufDesc.buffer, bufDesc.bufLength);
        if (length > bufDesc.bufLength) {
            bufDesc.dataLength = bufDesc.bufLength;
        } else {
            bufDesc.dataLength = length;
        }
    }
    audioCapturer->Enqueue(bufDesc);
}

void OHAudioCapturerDeviceChangeCallback::OnStateChange(const AudioDeviceDescriptor &deviceInfo)
{
    OHAudioCapturer* audioCapturer = (OHAudioCapturer*)ohAudioCapturer_;
    CHECK_AND_CALL_FUNC_RETURN_LOG(audioCapturer != nullptr,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_ILLEGAL_STATE, "capturer client is nullptr", true), "capturer client is nullptr");
    CHECK_AND_CALL_FUNC_RETURN_LOG((callbacks_.OH_AudioCapturer_OnStreamEvent != nullptr) ||
        onDeviceChangeCallback_ != nullptr,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_INVALID_PARAM, "OnStreamEvent is nullptr", true), "OnStreamEvent is nullptr");
    OH_AudioStream_Event event = AUDIOSTREAM_EVENT_ROUTING_CHANGED;
    if (audioCapturer->GetCapturerStreamEventCallbackType() == STREAM_EVENT_CALLBACK_COMBINED &&
        callbacks_.OH_AudioCapturer_OnStreamEvent != nullptr) {
        callbacks_.OH_AudioCapturer_OnStreamEvent(ohAudioCapturer_, userData_, event);
    }
    if (audioCapturer->GetCapturerStreamEventCallbackType() == STREAM_EVENT_CALLBACK_SEPERATED &&
        onDeviceChangeCallback_ != nullptr) {
        std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor =
            std::make_shared<AudioDeviceDescriptor>(deviceInfo);
        OH_AudioDeviceDescriptorArray *audioDeviceDescriptorArray =
            (OH_AudioDeviceDescriptorArray *)malloc(sizeof(OH_AudioDeviceDescriptorArray));
        CHECK_AND_CALL_FUNC_RETURN_LOG(audioDeviceDescriptorArray != nullptr,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                RECORD_CALLBACK_MEMORY_ALLOC_FAILED, "audioDeviceDescriptorArray is nullptr", true),
            "audioDeviceDescriptorArray is nullptr");
        uint32_t arraySize = 1;
        int32_t arrayIndex = 0;
        audioDeviceDescriptorArray->size = arraySize;
        audioDeviceDescriptorArray->descriptors =
            (OH_AudioDeviceDescriptor **)malloc(sizeof(OH_AudioDeviceDescriptor *) * arraySize);
        if (audioDeviceDescriptorArray->descriptors == nullptr) {
            free(audioDeviceDescriptorArray);
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                RECORD_CALLBACK_MEMORY_ALLOC_FAILED,
                "audioDeviceDescriptorArray->descriptors is nullptr", true);
            AUDIO_ERR_LOG("audioDeviceDescriptorArray->descriptors is nullptr");
            return;
        }
        audioDeviceDescriptorArray->descriptors[arrayIndex] =
            (OH_AudioDeviceDescriptor *)(new OHAudioDeviceDescriptor(audioDeviceDescriptor));
        if (audioDeviceDescriptorArray->descriptors[arrayIndex] == nullptr) {
            free(audioDeviceDescriptorArray->descriptors);
            free(audioDeviceDescriptorArray);
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                RECORD_CALLBACK_MEMORY_ALLOC_FAILED, "descriptors[arrayIndex] is nullptr", true);
            AUDIO_ERR_LOG("audioDeviceDescriptorArray->descriptors[%{public}d] is nullptr", arrayIndex);
            return;
        }
        onDeviceChangeCallback_(ohAudioCapturer_, userData_, audioDeviceDescriptorArray);
    }
}

void OHAudioCapturerCallback::OnInterrupt(const InterruptEvent &interruptEvent)
{
    OHAudioCapturer *audioCapturer = (OHAudioCapturer*)ohAudioCapturer_;
    CHECK_AND_CALL_FUNC_RETURN_LOG(ohAudioCapturer_ != nullptr,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_ILLEGAL_STATE, "capturer client is nullptr", true),
        "capturer client is nullptr");
    if (audioCapturer->GetCapturerInterruptEventCallbackType() == INTERRUPT_EVENT_CALLBACK_COMBINED &&
        callbacks_.OH_AudioCapturer_OnInterruptEvent != nullptr) {
        OH_AudioInterrupt_ForceType type = (OH_AudioInterrupt_ForceType)(interruptEvent.forceType);
        OH_AudioInterrupt_Hint hint = OH_AudioInterrupt_Hint(interruptEvent.hintType);
        callbacks_.OH_AudioCapturer_OnInterruptEvent(ohAudioCapturer_, userData_, type, hint);
    } else if (audioCapturer->GetCapturerInterruptEventCallbackType() == INTERRUPT_EVENT_CALLBACK_SEPERATED &&
        onInterruptEventCallback_ != nullptr) {
        OH_AudioInterrupt_ForceType type = (OH_AudioInterrupt_ForceType)(interruptEvent.forceType);
        OH_AudioInterrupt_Hint hint = OH_AudioInterrupt_Hint(interruptEvent.hintType);
        onInterruptEventCallback_(ohAudioCapturer_, userData_, type, hint);
    }
}

void OHAudioCapturerFastStatusChangeCallback::OnFastStatusChange(FastStatus status)
{
    CHECK_AND_CALL_FUNC_RETURN_LOG(ohAudioCapturer_ != nullptr,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_ILLEGAL_STATE, "capturer client is nullptr", true),
        "capturer client is nullptr");
    CHECK_AND_CALL_FUNC_RETURN_LOG(callback_ != nullptr,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_INVALID_PARAM, "pointer to the function is nullptr", true),
        "pointer to the function is nullptr");

    callback_(ohAudioCapturer_, userData_, static_cast<OH_AudioStream_FastStatus>(status));
}

void OHAudioCapturerOnPlaybackCaptureStartCallback::OnPlaybackCaptureStartResult(
    PlaybackCaptureStartState state)
{
    CHECK_AND_CALL_FUNC_RETURN_LOG(ohAudioCapturer_ != nullptr,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_ILLEGAL_STATE, "capturer client is nullptr", true),
        "capturer client is nullptr");
    CHECK_AND_CALL_FUNC_RETURN_LOG(callback_ != nullptr,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_INVALID_PARAM, "pointer to the function is nullptr", true),
        "pointer to the function is nullptr");

    callback_(ohAudioCapturer_, userData_, static_cast<OH_AudioStream_PlaybackCaptureStartState>(state));
    if (state == START_STATE_SUCCESS) {
        OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(ohAudioCapturer_);
        CHECK_AND_CALL_FUNC_RETURN_LOG(audioCapturer != nullptr,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                RECORD_CALLBACK_ILLEGAL_STATE, "convert capturer failed", true),
            "convert capturer failed");
        audioCapturer->Start();
    }
}

void OHAudioCapturer::SetReadDataCallback(CapturerCallback capturerCallbacks, void* userData)
{
    if (readDataCallbackType_ == READ_DATA_CALLBACK_WITH_RESULT &&
        capturerCallbacks.onReadDataCallback != nullptr) {
        std::shared_ptr<AudioCapturerReadCallback> callback = std::make_shared<OHAudioCapturerModeCallback>(
            capturerCallbacks.onReadDataCallback, (OH_AudioCapturer*)this, userData);
        audioCapturer_->SetCapturerReadCallback(callback);
        AUDIO_INFO_LOG("The read callback function with result");
    } else if (readDataCallbackType_ == READ_DATA_CALLBACK_WITHOUT_RESULT &&
        capturerCallbacks.callbacks.OH_AudioCapturer_OnReadData != nullptr) {
        std::shared_ptr<AudioCapturerReadCallback> callback = std::make_shared<OHAudioCapturerModeCallback>(
            capturerCallbacks.callbacks, (OH_AudioCapturer*)this, userData);
        audioCapturer_->SetCapturerReadCallback(callback);
        AUDIO_INFO_LOG("The read callback function without result");
    } else {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CONFIG_INVALID_PARAM, "The read callback function is not set", true);
        AUDIO_WARNING_LOG("The read callback function is not set");
    }
}

void OHAudioCapturer::SetStreamEventCallback(CapturerCallback capturerCallbacks, void* userData)
{
    if (streamEventCallbackType_ == STREAM_EVENT_CALLBACK_SEPERATED &&
        capturerCallbacks.onDeviceChangeCallback != nullptr) {
        std::shared_ptr<AudioCapturerDeviceChangeCallback> callback =
            std::make_shared<OHAudioCapturerDeviceChangeCallback>(capturerCallbacks.onDeviceChangeCallback,
            (OH_AudioCapturer*)this, userData);
        audioCapturer_->SetAudioCapturerDeviceChangeCallback(callback);
        AUDIO_INFO_LOG("The stream event callback function with result");
    } else {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CONFIG_NOT_SUPPORTED, "The stream event callback function only supports seperated set", true);
        AUDIO_WARNING_LOG("The stream event callback function only supports seperated set");
    }
}

void OHAudioCapturer::SetInterruptCallback(CapturerCallback capturerCallbacks, void *userData)
{
    if (interruptCallbackType_ == INTERRUPT_EVENT_CALLBACK_SEPERATED &&
        capturerCallbacks.onInterruptEventCallback != nullptr) {
        audioCapturerCallback_ = std::make_shared<OHAudioCapturerCallback>(
            capturerCallbacks.onInterruptEventCallback, (OH_AudioCapturer*)this, userData);
        audioCapturer_->SetCapturerCallback(audioCapturerCallback_);
        AUDIO_INFO_LOG("The Interrupt callback function is for PCM type with result");
    } else if (interruptCallbackType_ == INTERRUPT_EVENT_CALLBACK_COMBINED &&
        capturerCallbacks.callbacks.OH_AudioCapturer_OnInterruptEvent != nullptr) {
        audioCapturerCallback_ = std::make_shared<OHAudioCapturerCallback>(
            capturerCallbacks.callbacks, (OH_AudioCapturer*)this, userData);
        audioCapturer_->SetCapturerCallback(audioCapturerCallback_);
        AUDIO_INFO_LOG("The Interrupt callback function is for PCM type without result");
    } else {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CONFIG_INVALID_PARAM, "The Interrupt callback function is not set", true);
        AUDIO_WARNING_LOG("The Interrupt callback function is not set");
    }
}

void OHAudioCapturer::SetErrorCallback(CapturerCallback capturerCallbacks, void *userData)
{
    if (errorCallbackType_ == ERROR_CALLBACK_SEPERATED &&
        capturerCallbacks.onErrorCallback != nullptr) {
        std::shared_ptr<AudioCapturerPolicyServiceDiedCallback> callback =
            std::make_shared<OHCapturerServiceDiedCallback>(capturerCallbacks.onErrorCallback,
                (OH_AudioCapturer*)this, userData);
        int32_t clientPid = getpid();
        audioCapturer_->RegisterAudioPolicyServerDiedCb(clientPid, callback);

        std::shared_ptr<AudioCapturerErrorCallback> errorCallback = std::make_shared<OHAudioCapturerErrorCallback>(
            capturerCallbacks.onErrorCallback, (OH_AudioCapturer*)this, userData);
        audioCapturer_->SetAudioCapturerErrorCallback(errorCallback);
    } else {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CONFIG_NOT_SUPPORTED,
            "The audio capturer error callback function only supports seperated set", true);
        AUDIO_WARNING_LOG("The audio capturer error callback function only supports seperated set");
    }
}

void OHAudioCapturer::SetCapturerReadDataCallbackType(ReadDataCallbackType readDataCallbackType)
{
    readDataCallbackType_ = readDataCallbackType;
}

void OHAudioCapturer::SetCapturerStreamEventCallbackType(StreamEventCallbackType streamEventCallbackType)
{
    streamEventCallbackType_ = streamEventCallbackType;
}

void OHAudioCapturer::SetCapturerWillMuteWhenInterrupted(InterruptStrategy strategy)
{
    CHECK_AND_CALL_FUNC_RETURN_LOG(audioCapturer_ != nullptr,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CONFIG_ILLEGAL_STATE, "capturer client is nullptr", true),
        "capturer client is nullptr");
    audioCapturer_->SetInterruptStrategy(strategy);
}

ReadDataCallbackType OHAudioCapturer::GetCapturerReadDataCallbackType()
{
    return readDataCallbackType_;
}

StreamEventCallbackType OHAudioCapturer::GetCapturerStreamEventCallbackType()
{
    return streamEventCallbackType_;
}

void OHAudioCapturer::SetCapturerInterruptEventCallbackType(InterruptEventCallbackType callbackType)
{
    interruptCallbackType_ = callbackType;
    CHECK_AND_CALL_FUNC_RETURN_LOG(audioCapturer_ != nullptr,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_ILLEGAL_STATE, "capturer client is nullptr", true),
        "capturer client is nullptr");
    audioCapturer_->SetInterruptEventCallbackType(callbackType);
}

InterruptEventCallbackType OHAudioCapturer::GetCapturerInterruptEventCallbackType()
{
    return interruptCallbackType_;
}

void OHAudioCapturer::SetCapturerErrorCallbackType(ErrorCallbackType errorCallbackType)
{
    errorCallbackType_ = errorCallbackType;
}

ErrorCallbackType OHAudioCapturer::GetCapturerErrorCallbackType()
{
    return errorCallbackType_;
}

void OHAudioCapturer::SetCapturerFastStatusChangeCallback(OH_AudioCapturer_OnFastStatusChange callback, void *userData)
{
    CHECK_AND_CALL_FUNC_RETURN_LOG(audioCapturer_ != nullptr,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_ILLEGAL_STATE, "capturer client is nullptr", true),
        "capturer client is nullptr");
    CHECK_AND_CALL_FUNC_RETURN_LOG(callback != nullptr,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_INVALID_PARAM, "callback is nullptr", true),
        "callback is nullptr");
    audioCapturerFastStatusChangeCallback_ = std::make_shared<OHAudioCapturerFastStatusChangeCallback> (callback,
        reinterpret_cast<OH_AudioCapturer*>(this), userData);
    audioCapturer_->SetFastStatusChangeCallback(audioCapturerFastStatusChangeCallback_);
}

int32_t OHAudioCapturer::StartPlaybackCapture(OH_AudioCapturer* capturer,
    OH_AudioCapturer_OnPlaybackCaptureStartCallback callback, void* userData)
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioCapturer_ != nullptr, ERROR,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_START_ILLEGAL_STATE, "capturer client is nullptr", true),
        "capturer client is nullptr");
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(callback != nullptr, ERROR,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_START_INVALID_PARAM, "callback is nullptr", true),
        "callback is nullptr");
    capturerOnPlaybackCaptureStartCallback_ = std::make_shared<OHAudioCapturerOnPlaybackCaptureStartCallback> (callback,
        reinterpret_cast<OH_AudioCapturer*>(this), userData);
    audioCapturer_->SetPlaybackCaptureStartStateCallback(capturerOnPlaybackCaptureStartCallback_);

    return audioCapturer_->StartPlaybackCapture();
}

bool OHAudioCapturer::IsModernInnerCapturer()
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioCapturer_ != nullptr, false,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_ILLEGAL_STATE, "capturer client is nullptr", true),
        "capturer client is nullptr");
    AudioCapturerInfo capturerInfo;
    CHECK_AND_CALL_FUNC_RETURN_RET(audioCapturer_->GetCapturerInfo(capturerInfo) == SUCCESS, false,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_ILLEGAL_STATE, "GetCapturerInfo fail", true));
    CHECK_AND_CALL_FUNC_RETURN_RET(capturerInfo.sourceType == SOURCE_TYPE_PLAYBACK_CAPTURE &&
        isModernInnerCapturer_, false,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_ILLEGAL_STATE, "sourceType is not SOURCE_TYPE_PLAYBACK_CAPTURE", true));
    return true;
}

int32_t OHAudioCapturer::SetIndependentAudioSessionStrategy(
    const AudioSessionStrategy &strategy, const uint32_t behavior)
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioCapturer_ != nullptr, ERROR,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CONFIG_ILLEGAL_STATE, "capturer client is nullptr", true),
        "capturer client is nullptr");
    return audioCapturer_->SetIndependentAudioSessionStrategy(strategy, behavior);
}

const AudioCapturer *OHAudioCapturer::GetAudioCapturer() const
{
    return audioCapturer_.get();
}
}  // namespace AudioStandard
}  // namespace OHOS
