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
#define LOG_TAG "OHAudioCapturer"
#endif

#include "OHAudioCapturer.h"
#include "audio_errors.h"

using OHOS::AudioStandard::Timestamp;

static const int64_t SECOND_TO_NANOSECOND = 1000000000;

static OHOS::AudioStandard::OHAudioCapturer *convertCapturer(OH_AudioCapturer* capturer)
{
    return (OHOS::AudioStandard::OHAudioCapturer*) capturer;
}

OH_AudioStream_Result OH_AudioCapturer_Release(OH_AudioCapturer* capturer)
{
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert capturer failed");
    if (audioCapturer->Release()) {
        delete audioCapturer;
        audioCapturer = nullptr;
        return AUDIOSTREAM_SUCCESS;
    } else {
        return AUDIOSTREAM_ERROR_ILLEGAL_STATE;
    }
}

OH_AudioStream_Result OH_AudioCapturer_Start(OH_AudioCapturer* capturer)
{
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert capturer failed");
    if (audioCapturer->Start()) {
        return AUDIOSTREAM_SUCCESS;
    } else {
        return AUDIOSTREAM_ERROR_ILLEGAL_STATE;
    }
}

OH_AudioStream_Result OH_AudioCapturer_Pause(OH_AudioCapturer* capturer)
{
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert capturer failed");

    if (audioCapturer->Pause()) {
        return AUDIOSTREAM_SUCCESS;
    } else {
        return AUDIOSTREAM_ERROR_ILLEGAL_STATE;
    }
}

OH_AudioStream_Result OH_AudioCapturer_Stop(OH_AudioCapturer* capturer)
{
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert capturer failed");

    if (audioCapturer->Stop()) {
        return AUDIOSTREAM_SUCCESS;
    } else {
        return AUDIOSTREAM_ERROR_ILLEGAL_STATE;
    }
}

OH_AudioStream_Result OH_AudioCapturer_Flush(OH_AudioCapturer* capturer)
{
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert capturer failed");

    if (audioCapturer->Flush()) {
        return AUDIOSTREAM_SUCCESS;
    } else {
        return AUDIOSTREAM_ERROR_ILLEGAL_STATE;
    }
}


OH_AudioStream_Result OH_AudioCapturer_GetCurrentState(OH_AudioCapturer* capturer, OH_AudioStream_State* state)
{
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert capturer failed");

    OHOS::AudioStandard::CapturerState capturerState = audioCapturer->GetCurrentState();
    *state = (OH_AudioStream_State)capturerState;
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioCapturer_GetLatencyMode(OH_AudioCapturer* capturer,
    OH_AudioStream_LatencyMode* latencyMode)
{
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert capturer failed");
    OHOS::AudioStandard::AudioCapturerInfo capturerInfo;
    audioCapturer->GetCapturerInfo(capturerInfo);
    *latencyMode = (OH_AudioStream_LatencyMode)capturerInfo.capturerFlags;
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioCapturer_GetStreamId(OH_AudioCapturer* capturer, uint32_t* streamId)
{
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert capturer failed");
    audioCapturer->GetStreamId(*streamId);
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioCapturer_GetChannelCount(OH_AudioCapturer* capturer, int32_t* channelCount)
{
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert capturer failed");
    *channelCount = audioCapturer->GetChannelCount();
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioCapturer_GetSamplingRate(OH_AudioCapturer* capturer, int32_t* rate)
{
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert capturer failed");

    *rate = audioCapturer->GetSamplingRate();
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioCapturer_GetSampleFormat(OH_AudioCapturer* capturer,
    OH_AudioStream_SampleFormat* sampleFormat)
{
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert capturer failed");
    *sampleFormat = (OH_AudioStream_SampleFormat)audioCapturer->GetSampleFormat();
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioCapturer_GetEncodingType(OH_AudioCapturer* capturer,
    OH_AudioStream_EncodingType* encodingType)
{
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert capturer failed");
    *encodingType = (OH_AudioStream_EncodingType)audioCapturer->GetEncodingType();
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioCapturer_GetCapturerInfo(OH_AudioCapturer* capturer,
    OH_AudioStream_SourceType* sourceType)
{
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert capturer failed");
    OHOS::AudioStandard::AudioCapturerInfo capturerInfo;
    audioCapturer->GetCapturerInfo(capturerInfo);
    *sourceType = (OH_AudioStream_SourceType)capturerInfo.sourceType;
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioCapturer_GetFrameSizeInCallback(OH_AudioCapturer* capturer, int32_t* frameSize)
{
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert capturer failed");

    *frameSize = audioCapturer->GetFrameSizeInCallback();
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioCapturer_GetTimestamp(OH_AudioCapturer* capturer,
    clockid_t clockId, int64_t* framePosition, int64_t* timestamp)
{
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert capturer failed");
    CHECK_AND_RETURN_RET_LOG(clockId == CLOCK_MONOTONIC, AUDIOSTREAM_ERROR_INVALID_PARAM, "error clockId value");

    Timestamp stamp;
    Timestamp::Timestampbase base = Timestamp::Timestampbase::MONOTONIC;
    bool ret = audioCapturer->GetAudioTime(stamp, base);
    if (!ret) {
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
    CHECK_AND_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert capturer failed");

    *frames = audioCapturer->GetFramesRead();
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioCapturer_GetOverflowCount(OH_AudioCapturer* capturer, uint32_t* count)
{
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert capturer failed");

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
    std::string cacheDir = "/data/storage/el2/base/temp";
    audioCapturer_ = AudioCapturer::Create(capturerOptions, cacheDir);
    return audioCapturer_ != nullptr;
}

bool OHAudioCapturer::Start()
{
    if (audioCapturer_ == nullptr) {
        AUDIO_ERR_LOG("capturer client is nullptr");
        return false;
    }
    return audioCapturer_->Start();
}
    
bool OHAudioCapturer::Pause()
{
    if (audioCapturer_ == nullptr) {
        AUDIO_ERR_LOG("capturer client is nullptr");
        return false;
    }
    return audioCapturer_->Pause();
}

bool OHAudioCapturer::Stop()
{
    if (audioCapturer_ == nullptr) {
        AUDIO_ERR_LOG("capturer client is nullptr");
        return false;
    }
    return audioCapturer_->Stop();
}

bool OHAudioCapturer::Flush()
{
    if (audioCapturer_ == nullptr) {
        AUDIO_ERR_LOG("capturer client is nullptr");
        return false;
    }
    return audioCapturer_->Flush();
}

bool OHAudioCapturer::Release()
{
    if (audioCapturer_ == nullptr) {
        AUDIO_ERR_LOG("capturer client is nullptr");
        return false;
    }

    if (!audioCapturer_->Release()) {
        return false;
    }
    audioCapturer_ = nullptr;
    audioCapturerCallback_= nullptr;
    return true;
}

CapturerState OHAudioCapturer::GetCurrentState()
{
    CHECK_AND_RETURN_RET_LOG(audioCapturer_ != nullptr, CAPTURER_INVALID, "capturer client is nullptr");
    return audioCapturer_->GetStatus();
}

void OHAudioCapturer::GetStreamId(uint32_t &streamId)
{
    CHECK_AND_RETURN_LOG(audioCapturer_ != nullptr, "capturer client is nullptr");
    audioCapturer_->GetAudioStreamId(streamId);
}

AudioChannel OHAudioCapturer::GetChannelCount()
{
    CHECK_AND_RETURN_RET_LOG(audioCapturer_ != nullptr, MONO, "capturer client is nullptr");
    AudioCapturerParams params;
    audioCapturer_->GetParams(params);
    return params.audioChannel;
}

int32_t OHAudioCapturer::GetSamplingRate()
{
    CHECK_AND_RETURN_RET_LOG(audioCapturer_ != nullptr, MONO, "capturer client is nullptr");
    AudioCapturerParams params;
    audioCapturer_->GetParams(params);
    return params.samplingRate;
}

AudioEncodingType OHAudioCapturer::GetEncodingType()
{
    CHECK_AND_RETURN_RET_LOG(audioCapturer_ != nullptr, ENCODING_INVALID, "capturer client is nullptr");
    AudioCapturerParams params;
    audioCapturer_->GetParams(params);
    return params.audioEncoding;
}

void OHAudioCapturer::GetCapturerInfo(AudioCapturerInfo& capturerInfo)
{
    CHECK_AND_RETURN_LOG(audioCapturer_ != nullptr, "capturer client is nullptr");
    audioCapturer_->GetCapturerInfo(capturerInfo);
}

AudioSampleFormat OHAudioCapturer::GetSampleFormat()
{
    CHECK_AND_RETURN_RET_LOG(audioCapturer_ != nullptr, INVALID_WIDTH, "capturer client is nullptr");
    AudioCapturerParams params;
    audioCapturer_->GetParams(params);
    return params.audioSampleFormat;
}

void OHAudioCapturer::SetCapturerCallback(OH_AudioCapturer_Callbacks callbacks, void* userData)
{
    CHECK_AND_RETURN_LOG(audioCapturer_ != nullptr, "capturer client is nullptr");
    audioCapturer_->SetCaptureMode(CAPTURE_MODE_CALLBACK);
    if (callbacks.OH_AudioCapturer_OnReadData != nullptr) {
        std::shared_ptr<AudioCapturerReadCallback> callback = std::make_shared<OHAudioCapturerModeCallback>(callbacks,
            (OH_AudioCapturer*)this, userData);
        audioCapturer_->SetCapturerReadCallback(callback);
    } else {
        AUDIO_WARNING_LOG("The read callback function is not set");
    }

    if (callbacks.OH_AudioCapturer_OnInterruptEvent != nullptr) {
        audioCapturerCallback_ = std::make_shared<OHAudioCapturerCallback>(callbacks,
            (OH_AudioCapturer*)this, userData);
        audioCapturer_->SetCapturerCallback(audioCapturerCallback_);
    } else {
        AUDIO_WARNING_LOG("The capturer interrupt event callback function is not set");
    }
}

int64_t OHAudioCapturer::GetFramesRead()
{
    CHECK_AND_RETURN_RET_LOG(audioCapturer_ != nullptr, ERROR, "capturer client is nullptr");
    return audioCapturer_->GetFramesRead();
}

bool OHAudioCapturer::GetAudioTime(Timestamp &timestamp, Timestamp::Timestampbase base)
{
    CHECK_AND_RETURN_RET_LOG(audioCapturer_ != nullptr, false, "capturer client is nullptr");
    return audioCapturer_->GetAudioTime(timestamp, base);
}

int32_t OHAudioCapturer::GetFrameSizeInCallback()
{
    CHECK_AND_RETURN_RET_LOG(audioCapturer_ != nullptr, ERROR, "capturer client is nullptr");
    uint32_t frameSize;
    audioCapturer_->GetFrameCount(frameSize);
    return static_cast<int32_t>(frameSize);
}

int32_t OHAudioCapturer::GetBufferDesc(BufferDesc &bufDesc) const
{
    CHECK_AND_RETURN_RET_LOG(audioCapturer_ != nullptr, ERROR, "capturer client is nullptr");
    return audioCapturer_->GetBufferDesc(bufDesc);
}

int32_t OHAudioCapturer::Enqueue(const BufferDesc &bufDesc) const
{
    CHECK_AND_RETURN_RET_LOG(audioCapturer_ != nullptr, ERROR, "capturer client is nullptr");
    return audioCapturer_->Enqueue(bufDesc);
}

uint32_t OHAudioCapturer::GetOverflowCount() const
{
    CHECK_AND_RETURN_RET_LOG(audioCapturer_ != nullptr, ERROR, "capturer client is nullptr");
    return audioCapturer_->GetOverflowCount();
}

void OHAudioCapturerModeCallback::OnReadData(size_t length)
{
    OHAudioCapturer* audioCapturer = (OHAudioCapturer*)ohAudioCapturer_;
    CHECK_AND_RETURN_LOG(audioCapturer != nullptr, "capturer client is nullptr");
    CHECK_AND_RETURN_LOG(callbacks_.OH_AudioCapturer_OnReadData != nullptr, "pointer to the fuction is nullptr");
    BufferDesc bufDesc;
    audioCapturer->GetBufferDesc(bufDesc);
    callbacks_.OH_AudioCapturer_OnReadData(ohAudioCapturer_,
        userData_,
        (void*)bufDesc.buffer,
        bufDesc.bufLength);
    audioCapturer->Enqueue(bufDesc);
}

void OHAudioCapturerCallback::OnInterrupt(const InterruptEvent &interruptEvent)
{
    CHECK_AND_RETURN_LOG(ohAudioCapturer_ != nullptr, "capturer client is nullptr");
    CHECK_AND_RETURN_LOG(callbacks_.OH_AudioCapturer_OnInterruptEvent != nullptr, "pointer to the fuction is nullptr");

    OH_AudioInterrupt_ForceType type = (OH_AudioInterrupt_ForceType)(interruptEvent.forceType);
    OH_AudioInterrupt_Hint hint = OH_AudioInterrupt_Hint(interruptEvent.hintType);
    callbacks_.OH_AudioCapturer_OnInterruptEvent(ohAudioCapturer_, userData_, type, hint);
}
}  // namespace AudioStandard
}  // namespace OHOS
