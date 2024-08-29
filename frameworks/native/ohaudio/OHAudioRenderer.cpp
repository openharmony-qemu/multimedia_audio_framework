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
#define LOG_TAG "OHAudioRenderer"
#endif

#include "OHAudioRenderer.h"
#include "audio_errors.h"

using OHOS::AudioStandard::Timestamp;

static const int64_t SECOND_TO_NANOSECOND = 1000000000;

static OHOS::AudioStandard::OHAudioRenderer *convertRenderer(OH_AudioRenderer *renderer)
{
    return (OHOS::AudioStandard::OHAudioRenderer*) renderer;
}

static OH_AudioStream_Result ConvertError(int32_t err)
{
    if (err == OHOS::AudioStandard::SUCCESS) {
        return AUDIOSTREAM_SUCCESS;
    } else if (err == OHOS::AudioStandard::ERR_INVALID_PARAM) {
        return AUDIOSTREAM_ERROR_INVALID_PARAM;
    } else if (err == OHOS::AudioStandard::ERR_ILLEGAL_STATE) {
        return AUDIOSTREAM_ERROR_ILLEGAL_STATE;
    }
    return AUDIOSTREAM_ERROR_SYSTEM;
}

OH_AudioStream_Result OH_AudioRenderer_Start(OH_AudioRenderer *renderer)
{
    OHOS::AudioStandard::OHAudioRenderer *audioRenderer = convertRenderer(renderer);
    CHECK_AND_RETURN_RET_LOG(audioRenderer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert renderer failed");
    if (audioRenderer->Start()) {
        return AUDIOSTREAM_SUCCESS;
    } else {
        return AUDIOSTREAM_ERROR_ILLEGAL_STATE;
    }
}

OH_AudioStream_Result OH_AudioRenderer_Pause(OH_AudioRenderer *renderer)
{
    OHOS::AudioStandard::OHAudioRenderer *audioRenderer = convertRenderer(renderer);
    CHECK_AND_RETURN_RET_LOG(audioRenderer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert renderer failed");

    if (audioRenderer->Pause()) {
        return AUDIOSTREAM_SUCCESS;
    } else {
        return AUDIOSTREAM_ERROR_ILLEGAL_STATE;
    }
}

OH_AudioStream_Result OH_AudioRenderer_Stop(OH_AudioRenderer *renderer)
{
    OHOS::AudioStandard::OHAudioRenderer *audioRenderer = convertRenderer(renderer);
    CHECK_AND_RETURN_RET_LOG(audioRenderer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert renderer failed");

    if (audioRenderer->Stop()) {
        return AUDIOSTREAM_SUCCESS;
    } else {
        return AUDIOSTREAM_ERROR_ILLEGAL_STATE;
    }
}

OH_AudioStream_Result OH_AudioRenderer_Flush(OH_AudioRenderer *renderer)
{
    OHOS::AudioStandard::OHAudioRenderer *audioRenderer = convertRenderer(renderer);
    CHECK_AND_RETURN_RET_LOG(audioRenderer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert renderer failed");

    if (audioRenderer->Flush()) {
        return AUDIOSTREAM_SUCCESS;
    } else {
        return AUDIOSTREAM_ERROR_ILLEGAL_STATE;
    }
}

OH_AudioStream_Result OH_AudioRenderer_Release(OH_AudioRenderer *renderer)
{
    OHOS::AudioStandard::OHAudioRenderer *audioRenderer = convertRenderer(renderer);
    CHECK_AND_RETURN_RET_LOG(audioRenderer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert renderer failed");

    if (audioRenderer->Release()) {
        delete audioRenderer;
        audioRenderer = nullptr;
        return AUDIOSTREAM_SUCCESS;
    } else {
        return AUDIOSTREAM_ERROR_ILLEGAL_STATE;
    }
}

OH_AudioStream_Result OH_AudioRenderer_GetCurrentState(OH_AudioRenderer *renderer, OH_AudioStream_State *state)
{
    OHOS::AudioStandard::OHAudioRenderer *audioRenderer = convertRenderer(renderer);
    CHECK_AND_RETURN_RET_LOG(audioRenderer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert renderer failed");

    OHOS::AudioStandard::RendererState rendererState = audioRenderer->GetCurrentState();
    *state = (OH_AudioStream_State)rendererState;
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioRenderer_GetSamplingRate(OH_AudioRenderer *renderer, int32_t *rate)
{
    OHOS::AudioStandard::OHAudioRenderer *audioRenderer = convertRenderer(renderer);
    CHECK_AND_RETURN_RET_LOG(audioRenderer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert renderer failed");

    *rate = audioRenderer->GetSamplingRate();
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioRenderer_GetStreamId(OH_AudioRenderer *renderer, uint32_t *streamId)
{
    OHOS::AudioStandard::OHAudioRenderer *audioRenderer = convertRenderer(renderer);
    CHECK_AND_RETURN_RET_LOG(audioRenderer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert renderer failed");
    audioRenderer->GetStreamId(*streamId);
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioRenderer_GetChannelCount(OH_AudioRenderer *renderer, int32_t *channelCount)
{
    OHOS::AudioStandard::OHAudioRenderer *audioRenderer = convertRenderer(renderer);
    CHECK_AND_RETURN_RET_LOG(audioRenderer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert renderer failed");
    *channelCount = audioRenderer->GetChannelCount();
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioRenderer_GetSampleFormat(OH_AudioRenderer *renderer,
    OH_AudioStream_SampleFormat *sampleFormat)
{
    OHOS::AudioStandard::OHAudioRenderer *audioRenderer = convertRenderer(renderer);
    CHECK_AND_RETURN_RET_LOG(audioRenderer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert renderer failed");
    *sampleFormat = (OH_AudioStream_SampleFormat)audioRenderer->GetSampleFormat();
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioRenderer_GetLatencyMode(OH_AudioRenderer *renderer,
    OH_AudioStream_LatencyMode *latencyMode)
{
    OHOS::AudioStandard::OHAudioRenderer *audioRenderer = convertRenderer(renderer);
    CHECK_AND_RETURN_RET_LOG(audioRenderer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert renderer failed");
    OHOS::AudioStandard::AudioRendererInfo rendererInfo;
    audioRenderer->GetRendererInfo(rendererInfo);
    *latencyMode = (OH_AudioStream_LatencyMode)rendererInfo.rendererFlags;

    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioRenderer_GetRendererInfo(OH_AudioRenderer *renderer,
    OH_AudioStream_Usage *usage)
{
    OHOS::AudioStandard::OHAudioRenderer *audioRenderer = convertRenderer(renderer);
    CHECK_AND_RETURN_RET_LOG(audioRenderer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert renderer failed");

    OHOS::AudioStandard::AudioRendererInfo rendererInfo;
    audioRenderer->GetRendererInfo(rendererInfo);
    *usage = (OH_AudioStream_Usage)rendererInfo.streamUsage;
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioRenderer_GetRendererPrivacy(OH_AudioRenderer* renderer,
    OH_AudioStream_PrivacyType* privacy)
{
    OHOS::AudioStandard::OHAudioRenderer *audioRenderer = convertRenderer(renderer);
    CHECK_AND_RETURN_RET_LOG(audioRenderer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert renderer failed");

    *privacy = (OH_AudioStream_PrivacyType)audioRenderer->GetRendererPrivacy();
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioRenderer_GetEncodingType(OH_AudioRenderer *renderer,
    OH_AudioStream_EncodingType *encodingType)
{
    OHOS::AudioStandard::OHAudioRenderer *audioRenderer = convertRenderer(renderer);
    CHECK_AND_RETURN_RET_LOG(audioRenderer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert renderer failed");
    *encodingType = (OH_AudioStream_EncodingType)audioRenderer->GetEncodingType();
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioRenderer_GetFramesWritten(OH_AudioRenderer *renderer, int64_t *frames)
{
    OHOS::AudioStandard::OHAudioRenderer *audioRenderer = convertRenderer(renderer);
    CHECK_AND_RETURN_RET_LOG(audioRenderer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert renderer failed");
    *frames = audioRenderer->GetFramesWritten();
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioRenderer_GetTimestamp(OH_AudioRenderer *renderer,
    clockid_t clockId, int64_t *framePosition, int64_t *timestamp)
{
    OHOS::AudioStandard::OHAudioRenderer *audioRenderer = convertRenderer(renderer);
    CHECK_AND_RETURN_RET_LOG(audioRenderer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert renderer failed");
    CHECK_AND_RETURN_RET_LOG(clockId == CLOCK_MONOTONIC, AUDIOSTREAM_ERROR_INVALID_PARAM, "error clockId value");
    Timestamp stamp;
    Timestamp::Timestampbase base = Timestamp::Timestampbase::MONOTONIC;
    bool ret = audioRenderer->GetAudioTime(stamp, base);
    if (!ret) {
        AUDIO_ERR_LOG("GetAudioTime error!");
        return AUDIOSTREAM_ERROR_ILLEGAL_STATE;
    }
    *framePosition = stamp.framePosition;
    *timestamp = stamp.time.tv_sec * SECOND_TO_NANOSECOND + stamp.time.tv_nsec;
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioRenderer_GetFrameSizeInCallback(OH_AudioRenderer *renderer, int32_t *frameSize)
{
    OHOS::AudioStandard::OHAudioRenderer *audioRenderer = convertRenderer(renderer);
    CHECK_AND_RETURN_RET_LOG(audioRenderer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert renderer failed");
    *frameSize = audioRenderer->GetFrameSizeInCallback();
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioRenderer_GetSpeed(OH_AudioRenderer *renderer, float *speed)
{
    OHOS::AudioStandard::OHAudioRenderer *audioRenderer = convertRenderer(renderer);
    CHECK_AND_RETURN_RET_LOG(audioRenderer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert renderer failed");
    *speed = audioRenderer->GetSpeed();
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioRenderer_SetSpeed(OH_AudioRenderer *renderer, float speed)
{
    OHOS::AudioStandard::OHAudioRenderer *audioRenderer = convertRenderer(renderer);
    CHECK_AND_RETURN_RET_LOG(audioRenderer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert renderer failed");
    audioRenderer->SetSpeed(speed);
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioRenderer_SetVolume(OH_AudioRenderer *renderer, float volume)
{
    OHOS::AudioStandard::OHAudioRenderer *audioRenderer = convertRenderer(renderer);
    CHECK_AND_RETURN_RET_LOG(audioRenderer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert renderer failed");
    CHECK_AND_RETURN_RET_LOG(((volume >= 0) && (volume <= 1)), AUDIOSTREAM_ERROR_INVALID_PARAM, "volume set invalid");
    int32_t err = audioRenderer->SetVolume(volume);
    return ConvertError(err);
}

OH_AudioStream_Result OH_AudioRenderer_SetVolumeWithRamp(OH_AudioRenderer *renderer, float volume, int32_t durationMs)
{
    OHOS::AudioStandard::OHAudioRenderer *audioRenderer = convertRenderer(renderer);
    CHECK_AND_RETURN_RET_LOG(audioRenderer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert renderer failed");
    CHECK_AND_RETURN_RET_LOG(((volume >= 0) && (volume <= 1)), AUDIOSTREAM_ERROR_INVALID_PARAM, "volume set invalid");
    int32_t err = audioRenderer->SetVolumeWithRamp(volume, durationMs);
    return ConvertError(err);
}

OH_AudioStream_Result OH_AudioRenderer_GetVolume(OH_AudioRenderer *renderer, float *volume)
{
    OHOS::AudioStandard::OHAudioRenderer *audioRenderer = convertRenderer(renderer);
    CHECK_AND_RETURN_RET_LOG(audioRenderer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert renderer failed");
    CHECK_AND_RETURN_RET_LOG(volume != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "volume is nullptr");
    *volume = audioRenderer->GetVolume();
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioRenderer_SetMarkPosition(OH_AudioRenderer *renderer, uint32_t samplePos,
    OH_AudioRenderer_OnMarkReachedCallback callback, void *userData)
{
    OHOS::AudioStandard::OHAudioRenderer *audioRenderer = convertRenderer(renderer);
    CHECK_AND_RETURN_RET_LOG(audioRenderer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert renderer failed");
    CHECK_AND_RETURN_RET_LOG(samplePos > 0, AUDIOSTREAM_ERROR_INVALID_PARAM, "framePos set invalid");
    int32_t err = audioRenderer->SetRendererPositionCallback(callback, samplePos, userData);
    return ConvertError(err);
}

OH_AudioStream_Result OH_AudioRenderer_CancelMark(OH_AudioRenderer *renderer)
{
    OHOS::AudioStandard::OHAudioRenderer *audioRenderer = convertRenderer(renderer);
    CHECK_AND_RETURN_RET_LOG(audioRenderer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert renderer failed");
    audioRenderer->UnsetRendererPositionCallback();
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioRenderer_GetChannelLayout(OH_AudioRenderer *renderer,
    OH_AudioChannelLayout *channelLayout)
{
    OHOS::AudioStandard::OHAudioRenderer *audioRenderer = convertRenderer(renderer);
    CHECK_AND_RETURN_RET_LOG(audioRenderer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert renderer failed");
    *channelLayout = (OH_AudioChannelLayout)audioRenderer->GetChannelLayout();
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioRenderer_GetEffectMode(OH_AudioRenderer *renderer,
    OH_AudioStream_AudioEffectMode *effectMode)
{
    OHOS::AudioStandard::OHAudioRenderer *audioRenderer = convertRenderer(renderer);
    CHECK_AND_RETURN_RET_LOG(audioRenderer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert renderer failed");
    *effectMode = (OH_AudioStream_AudioEffectMode)audioRenderer->GetEffectMode();
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioRenderer_SetEffectMode(OH_AudioRenderer *renderer,
    OH_AudioStream_AudioEffectMode effectMode)
{
    OHOS::AudioStandard::OHAudioRenderer *audioRenderer = convertRenderer(renderer);
    CHECK_AND_RETURN_RET_LOG(audioRenderer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert renderer failed");
    audioRenderer->SetEffectMode((OHOS::AudioStandard::AudioEffectMode)effectMode);
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioRenderer_GetUnderflowCount(OH_AudioRenderer* renderer, uint32_t* count)
{
    OHOS::AudioStandard::OHAudioRenderer *audioRenderer = convertRenderer(renderer);
    CHECK_AND_RETURN_RET_LOG(audioRenderer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert renderer failed");
    CHECK_AND_RETURN_RET_LOG(count != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "count is nullptr");
    *count = audioRenderer->GetUnderflowCount();
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioRenderer_SetSilentModeAndMixWithOthers(
    OH_AudioRenderer* renderer, bool on)
{
    OHOS::AudioStandard::OHAudioRenderer *audioRenderer = convertRenderer(renderer);
    CHECK_AND_RETURN_RET_LOG(audioRenderer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert renderer failed");
    audioRenderer->SetSilentModeAndMixWithOthers(on);
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioRenderer_GetSilentModeAndMixWithOthers(
    OH_AudioRenderer* renderer, bool* on)
{
    OHOS::AudioStandard::OHAudioRenderer *audioRenderer = convertRenderer(renderer);
    CHECK_AND_RETURN_RET_LOG(audioRenderer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert renderer failed");
    *on = audioRenderer->GetSilentModeAndMixWithOthers();
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioRenderer_SetDefaultOutputDevice(
    OH_AudioRenderer* renderer, OH_AudioDevice_Type deviceType)
{
    OHOS::AudioStandard::OHAudioRenderer *audioRenderer = convertRenderer(renderer);
    CHECK_AND_RETURN_RET_LOG(audioRenderer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert renderer failed");
    bool result = (deviceType == AUDIO_DEVICE_TYPE_EARPIECE || deviceType == AUDIO_DEVICE_TYPE_SPEAKER ||
        deviceType == AUDIO_DEVICE_TYPE_DEFAULT) ? true : false;
    CHECK_AND_RETURN_RET_LOG(result != false, AUDIOSTREAM_ERROR_INVALID_PARAM, "deviceType is not valid");
    int32_t ret = audioRenderer->SetDefaultOutputDevice((OHOS::AudioStandard::DeviceType)deviceType);
    if (ret == OHOS::AudioStandard::ERR_NOT_SUPPORTED) {
        AUDIO_ERR_LOG("This audiorenderer can not reset the output device");
        return AUDIOSTREAM_ERROR_ILLEGAL_STATE;
    } else if (ret != AUDIOSTREAM_SUCCESS) {
        AUDIO_ERR_LOG("system error when calling this function");
        return AUDIOSTREAM_ERROR_SYSTEM;
    }
    return AUDIOSTREAM_SUCCESS;
}

namespace OHOS {
namespace AudioStandard {
OHAudioRenderer::OHAudioRenderer()
{
    AUDIO_INFO_LOG("OHAudioRenderer created!");
}

OHAudioRenderer::~OHAudioRenderer()
{
    AUDIO_INFO_LOG("OHAudioRenderer destroyed!");
}

bool OHAudioRenderer::Initialize(AudioRendererOptions &rendererOptions)
{
    bool offloadAllowed = true;

    // unknown stream use music policy as default
    if (rendererOptions.rendererInfo.streamUsage == STREAM_USAGE_UNKNOWN) {
        rendererOptions.rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
        offloadAllowed = false;
    }
    std::string cacheDir = "/data/storage/el2/base/temp";
    audioRenderer_ = AudioRenderer::Create(cacheDir, rendererOptions);

    // if caller do not set usage, do not allow to use offload output
    if (audioRenderer_ != nullptr) {
        audioRenderer_->SetOffloadAllowed(offloadAllowed);
    }

    return audioRenderer_ != nullptr;
}

bool OHAudioRenderer::Start()
{
    if (audioRenderer_ == nullptr) {
        AUDIO_ERR_LOG("renderer client is nullptr");
        return false;
    }
    return audioRenderer_->Start();
}

bool OHAudioRenderer::Pause()
{
    if (audioRenderer_ == nullptr) {
        AUDIO_ERR_LOG("renderer client is nullptr");
        return false;
    }
    return audioRenderer_->Pause();
}

bool OHAudioRenderer::Stop()
{
    if (audioRenderer_ == nullptr) {
        AUDIO_ERR_LOG("renderer client is nullptr");
        return false;
    }
    return audioRenderer_->Stop();
}

bool OHAudioRenderer::Flush()
{
    if (audioRenderer_ == nullptr) {
        AUDIO_ERR_LOG("renderer client is nullptr");
        return false;
    }
    return audioRenderer_->Flush();
}

bool OHAudioRenderer::Release()
{
    if (audioRenderer_ == nullptr) {
        AUDIO_ERR_LOG("renderer client is nullptr");
        return false;
    }

    if (!audioRenderer_->Release()) {
        return false;
    }
    audioRenderer_ = nullptr;
    audioRendererCallback_ = nullptr;
    return true;
}

RendererState OHAudioRenderer::GetCurrentState()
{
    CHECK_AND_RETURN_RET_LOG(audioRenderer_ != nullptr, RENDERER_INVALID, "renderer client is nullptr");
    return audioRenderer_->GetStatus();
}

void OHAudioRenderer::GetStreamId(uint32_t &streamId)
{
    CHECK_AND_RETURN_LOG(audioRenderer_ != nullptr, "renderer client is nullptr");
    audioRenderer_->GetAudioStreamId(streamId);
}

AudioChannel OHAudioRenderer::GetChannelCount()
{
    CHECK_AND_RETURN_RET_LOG(audioRenderer_ != nullptr, MONO, "renderer client is nullptr");
    AudioRendererParams params;
    audioRenderer_->GetParams(params);
    return params.channelCount;
}

int32_t OHAudioRenderer::GetSamplingRate()
{
    CHECK_AND_RETURN_RET_LOG(audioRenderer_ != nullptr, MONO, "renderer client is nullptr");
    AudioRendererParams params;
    audioRenderer_->GetParams(params);
    return params.sampleRate;
}

AudioSampleFormat OHAudioRenderer::GetSampleFormat()
{
    CHECK_AND_RETURN_RET_LOG(audioRenderer_ != nullptr, INVALID_WIDTH, "renderer client is nullptr");
    AudioRendererParams params;
    audioRenderer_->GetParams(params);
    return params.sampleFormat;
}

void OHAudioRenderer::GetRendererInfo(AudioRendererInfo& rendererInfo)
{
    CHECK_AND_RETURN_LOG(audioRenderer_ != nullptr, "renderer client is nullptr");
    audioRenderer_->GetRendererInfo(rendererInfo);
}

AudioEncodingType OHAudioRenderer::GetEncodingType()
{
    CHECK_AND_RETURN_RET_LOG(audioRenderer_ != nullptr, ENCODING_INVALID, "renderer client is nullptr");
    AudioRendererParams params;
    audioRenderer_->GetParams(params);
    return params.encodingType;
}

int64_t OHAudioRenderer::GetFramesWritten()
{
    CHECK_AND_RETURN_RET_LOG(audioRenderer_ != nullptr, ERROR, "renderer client is nullptr");
    return audioRenderer_->GetFramesWritten();
}

bool OHAudioRenderer::GetAudioTime(Timestamp &timestamp, Timestamp::Timestampbase base)
{
    CHECK_AND_RETURN_RET_LOG(audioRenderer_ != nullptr, false, "renderer client is nullptr");
    return audioRenderer_->GetAudioPosition(timestamp, base);
}

int32_t OHAudioRenderer::GetFrameSizeInCallback()
{
    CHECK_AND_RETURN_RET_LOG(audioRenderer_ != nullptr, ERROR, "renderer client is nullptr");
    uint32_t frameSize;
    audioRenderer_->GetFrameCount(frameSize);
    return static_cast<int32_t>(frameSize);
}

int32_t OHAudioRenderer::GetBufferDesc(BufferDesc &bufDesc) const
{
    CHECK_AND_RETURN_RET_LOG(audioRenderer_ != nullptr, ERROR, "renderer client is nullptr");
    return audioRenderer_->GetBufferDesc(bufDesc);
}

int32_t OHAudioRenderer::Enqueue(const BufferDesc &bufDesc) const
{
    CHECK_AND_RETURN_RET_LOG(audioRenderer_ != nullptr, ERROR, "renderer client is nullptr");
    return audioRenderer_->Enqueue(bufDesc);
}

int32_t OHAudioRenderer::SetSpeed(float speed)
{
    CHECK_AND_RETURN_RET_LOG(audioRenderer_ != nullptr, ERROR, "renderer client is nullptr");
    return audioRenderer_->SetSpeed(speed);
}

float OHAudioRenderer::GetSpeed()
{
    CHECK_AND_RETURN_RET_LOG(audioRenderer_ != nullptr, ERROR, "renderer client is nullptr");
    return audioRenderer_->GetSpeed();
}

int32_t OHAudioRenderer::SetVolume(float volume) const
{
    CHECK_AND_RETURN_RET_LOG(audioRenderer_ != nullptr, ERROR, "renderer client is nullptr");
    return audioRenderer_->SetVolume(volume);
}

float OHAudioRenderer::GetVolume() const
{
    CHECK_AND_RETURN_RET_LOG(audioRenderer_ != nullptr, ERROR, "renderer client is nullptr");
    return audioRenderer_->GetVolume();
}

int32_t OHAudioRenderer::SetVolumeWithRamp(float volume, int32_t duration)
{
    CHECK_AND_RETURN_RET_LOG(audioRenderer_ != nullptr, ERROR, "renderer client is nullptr");
    return audioRenderer_->SetVolumeWithRamp(volume, duration);
}

int32_t OHAudioRenderer::SetRendererPositionCallback(OH_AudioRenderer_OnMarkReachedCallback callback,
    uint32_t markPosition, void *userData)
{
    CHECK_AND_RETURN_RET_LOG(audioRenderer_ != nullptr, ERROR, "renderer client is nullptr");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERROR, "callback is nullptr");
    rendererPositionCallback_ = std::make_shared<OHRendererPositionCallback>(callback,
        reinterpret_cast<OH_AudioRenderer*>(this), userData);
    return audioRenderer_->SetRendererPositionCallback(markPosition, rendererPositionCallback_);
}

void OHAudioRenderer::UnsetRendererPositionCallback()
{
    CHECK_AND_RETURN_LOG(audioRenderer_ != nullptr, "renderer client is nullptr");
    audioRenderer_->UnsetRendererPositionCallback();
}

void OHRendererPositionCallback::OnMarkReached(const int64_t &framePosition)
{
    CHECK_AND_RETURN_LOG(ohAudioRenderer_ != nullptr, "renderer client is nullptr");
    CHECK_AND_RETURN_LOG(callback_ != nullptr, "pointer to the function is nullptr");
    callback_(ohAudioRenderer_, framePosition, userData_);
}

AudioChannelLayout OHAudioRenderer::GetChannelLayout()
{
    CHECK_AND_RETURN_RET_LOG(audioRenderer_ != nullptr, CH_LAYOUT_UNKNOWN, "renderer client is nullptr");
    AudioRendererParams params;
    audioRenderer_->GetParams(params);
    return params.channelLayout;
}

AudioPrivacyType OHAudioRenderer::GetRendererPrivacy()
{
    CHECK_AND_RETURN_RET_LOG(audioRenderer_ != nullptr, PRIVACY_TYPE_PUBLIC, "renderer client is nullptr for privacy");
    return audioRenderer_->GetAudioPrivacyType();
}

AudioEffectMode OHAudioRenderer::GetEffectMode()
{
    CHECK_AND_RETURN_RET_LOG(audioRenderer_ != nullptr, EFFECT_NONE, "renderer client is nullptr");
    return audioRenderer_->GetAudioEffectMode();
}

int32_t OHAudioRenderer::SetEffectMode(AudioEffectMode effectMode)
{
    CHECK_AND_RETURN_RET_LOG(audioRenderer_ != nullptr, ERROR, "renderer client is nullptr");
    return audioRenderer_->SetAudioEffectMode(effectMode);
}

void OHAudioRenderer::SetWriteDataCallback(RendererCallback rendererCallbacks, void *userData,
    void *metadataUserData, AudioEncodingType encodingType)
{
    if (encodingType == ENCODING_AUDIOVIVID && rendererCallbacks.writeDataWithMetadataCallback != nullptr) {
        std::shared_ptr<AudioRendererWriteCallback> callback = std::make_shared<OHAudioRendererModeCallback>(
            rendererCallbacks.writeDataWithMetadataCallback, (OH_AudioRenderer*)this, metadataUserData, encodingType);
        audioRenderer_->SetRendererWriteCallback(callback);
        AUDIO_INFO_LOG("The write callback function is for AudioVivid type");
    } else if (encodingType == ENCODING_PCM) {
        if (writeDataCallbackType_ == WRITE_DATA_CALLBACK_WITH_RESULT &&
            rendererCallbacks.onWriteDataCallback != nullptr) {
            std::shared_ptr<AudioRendererWriteCallback> callback = std::make_shared<OHAudioRendererModeCallback>(
                rendererCallbacks.onWriteDataCallback, (OH_AudioRenderer*)this, userData, encodingType);
            audioRenderer_->SetRendererWriteCallback(callback);
            AUDIO_INFO_LOG("The write callback function is for PCM type with result");
        }

        if (writeDataCallbackType_ == WRITE_DATA_CALLBACK_WITHOUT_RESULT &&
            rendererCallbacks.callbacks.OH_AudioRenderer_OnWriteData != nullptr) {
            std::shared_ptr<AudioRendererWriteCallback> callback = std::make_shared<OHAudioRendererModeCallback>(
                rendererCallbacks.callbacks, (OH_AudioRenderer*)this, userData, encodingType);
            audioRenderer_->SetRendererWriteCallback(callback);
            AUDIO_INFO_LOG("The write callback function is for PCM type without result");
        }
    } else {
        AUDIO_WARNING_LOG("The write callback function is not set");
    }
}

void OHAudioRenderer::SetInterruptCallback(RendererCallback rendererCallbacks, void *userData)
{
    if (rendererCallbacks.callbacks.OH_AudioRenderer_OnInterruptEvent != nullptr) {
        audioRendererCallback_ = std::make_shared<OHAudioRendererCallback>(rendererCallbacks.callbacks,
            (OH_AudioRenderer*)this, userData);
        audioRenderer_->SetRendererCallback(audioRendererCallback_);
    } else {
        AUDIO_WARNING_LOG("The audio renderer interrupt callback function is not set");
    }
}

void OHAudioRenderer::SetErrorCallback(RendererCallback rendererCallbacks, void *userData)
{
    if (rendererCallbacks.callbacks.OH_AudioRenderer_OnError != nullptr) {
        std::shared_ptr<AudioRendererPolicyServiceDiedCallback> callback =
            std::make_shared<OHServiceDiedCallback>(rendererCallbacks.callbacks, (OH_AudioRenderer*)this, userData);
        int32_t clientPid = getpid();
        audioRenderer_->RegisterAudioPolicyServerDiedCb(clientPid, callback);

        std::shared_ptr<AudioRendererErrorCallback> errorCallback = std::make_shared<OHAudioRendererErrorCallback>(
            rendererCallbacks.callbacks, (OH_AudioRenderer*)this, userData);
        audioRenderer_->SetAudioRendererErrorCallback(errorCallback);
    } else {
        AUDIO_WARNING_LOG("The audio renderer error callback function is not set");
    }
}

void OHAudioRenderer::SetRendererCallback(RendererCallback rendererCallbacks, void *userData, void *metadataUserData)
{
    CHECK_AND_RETURN_LOG(audioRenderer_ != nullptr, "renderer client is nullptr");
    audioRenderer_->SetRenderMode(RENDER_MODE_CALLBACK);

    AudioEncodingType encodingType = GetEncodingType();
    SetWriteDataCallback(rendererCallbacks, userData, metadataUserData, encodingType);
    SetInterruptCallback(rendererCallbacks, userData);
    SetErrorCallback(rendererCallbacks, userData);
}

void OHAudioRenderer::SetRendererOutputDeviceChangeCallback(OH_AudioRenderer_OutputDeviceChangeCallback callback,
    void *userData)
{
    CHECK_AND_RETURN_LOG(audioRenderer_ != nullptr, "renderer client is nullptr");
    CHECK_AND_RETURN_LOG(callback != nullptr, "callback is nullptr");
    audioRendererDeviceChangeCallbackWithInfo_ =
        std::make_shared<OHAudioRendererDeviceChangeCallbackWithInfo> (callback,
        reinterpret_cast<OH_AudioRenderer*>(this), userData);
    audioRenderer_->RegisterOutputDeviceChangeWithInfoCallback(audioRendererDeviceChangeCallbackWithInfo_);
}

void OHAudioRenderer::SetPreferredFrameSize(int32_t frameSize)
{
    audioRenderer_->SetPreferredFrameSize(frameSize);
}

bool OHAudioRenderer::IsFastRenderer()
{
    return audioRenderer_->IsFastRenderer();
}

uint32_t OHAudioRenderer::GetUnderflowCount()
{
    CHECK_AND_RETURN_RET_LOG(audioRenderer_ != nullptr, ERROR, "renderer client is nullptr");
    return audioRenderer_->GetUnderflowCount();
}

void OHAudioRendererModeCallback::OnWriteData(size_t length)
{
    OHAudioRenderer *audioRenderer = (OHAudioRenderer*)ohAudioRenderer_;
    CHECK_AND_RETURN_LOG(audioRenderer != nullptr, "renderer client is nullptr");
    CHECK_AND_RETURN_LOG(((encodingType_ == ENCODING_PCM) && (callbacks_.OH_AudioRenderer_OnWriteData != nullptr)) ||
        ((encodingType_ == ENCODING_PCM) && (onWriteDataCallback_ != nullptr)) ||
        ((encodingType_ == ENCODING_AUDIOVIVID) && (writeDataWithMetadataCallback_ != nullptr)),
        "pointer to the function is nullptr");
    BufferDesc bufDesc;
    audioRenderer->GetBufferDesc(bufDesc);
    if (encodingType_ == ENCODING_AUDIOVIVID && writeDataWithMetadataCallback_ != nullptr) {
        writeDataWithMetadataCallback_(ohAudioRenderer_, metadataUserData_, (void*)bufDesc.buffer, bufDesc.bufLength,
            (void*)bufDesc.metaBuffer, bufDesc.metaLength);
    } else {
        if (audioRenderer->GetRendererCallbackType() == WRITE_DATA_CALLBACK_WITHOUT_RESULT &&
            callbacks_.OH_AudioRenderer_OnWriteData != nullptr) {
            callbacks_.OH_AudioRenderer_OnWriteData(ohAudioRenderer_, userData_,
                (void*)bufDesc.buffer, bufDesc.bufLength);
        }
        if (audioRenderer->GetRendererCallbackType() == WRITE_DATA_CALLBACK_WITH_RESULT &&
            onWriteDataCallback_ != nullptr) {
            OH_AudioData_Callback_Result result = onWriteDataCallback_(ohAudioRenderer_, userData_,
                (void*)bufDesc.buffer, bufDesc.bufLength);
            if (result == AUDIO_DATA_CALLBACK_RESULT_INVALID) {
                AUDIO_DEBUG_LOG("Data callback returned invalid, data will not be used.");
                bufDesc.dataLength = 0; // Ensure that the invalid data is not used.
            }
        }
    }
    audioRenderer->Enqueue(bufDesc);
}

void OHAudioRendererDeviceChangeCallback::OnOutputDeviceChange(const DeviceInfo &deviceInfo,
    const AudioStreamDeviceChangeReason reason)
{
    CHECK_AND_RETURN_LOG(ohAudioRenderer_ != nullptr, "renderer client is nullptr");
    CHECK_AND_RETURN_LOG(callbacks_.OH_AudioRenderer_OnStreamEvent != nullptr, "pointer to the function is nullptr");

    OH_AudioStream_Event event =  AUDIOSTREAM_EVENT_ROUTING_CHANGED;
    callbacks_.OH_AudioRenderer_OnStreamEvent(ohAudioRenderer_, userData_, event);
}

void OHAudioRendererCallback::OnInterrupt(const InterruptEvent &interruptEvent)
{
    CHECK_AND_RETURN_LOG(ohAudioRenderer_ != nullptr, "renderer client is nullptr");
    CHECK_AND_RETURN_LOG(callbacks_.OH_AudioRenderer_OnInterruptEvent != nullptr, "pointer to the function is nullptr");
    OH_AudioInterrupt_ForceType type = (OH_AudioInterrupt_ForceType)(interruptEvent.forceType);
    OH_AudioInterrupt_Hint hint = OH_AudioInterrupt_Hint(interruptEvent.hintType);
    callbacks_.OH_AudioRenderer_OnInterruptEvent(ohAudioRenderer_, userData_, type, hint);
}

void OHServiceDiedCallback::OnAudioPolicyServiceDied()
{
    CHECK_AND_RETURN_LOG(ohAudioRenderer_ != nullptr, "renderer client is nullptr");
    CHECK_AND_RETURN_LOG(callbacks_.OH_AudioRenderer_OnError != nullptr, "pointer to the function is nullptr");
    OH_AudioStream_Result error = AUDIOSTREAM_ERROR_SYSTEM;
    callbacks_.OH_AudioRenderer_OnError(ohAudioRenderer_, userData_, error);
}

OH_AudioStream_Result OHAudioRendererErrorCallback::GetErrorResult(AudioErrors errorCode) const
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

void OHAudioRendererErrorCallback::OnError(AudioErrors errorCode)
{
    CHECK_AND_RETURN_LOG(ohAudioRenderer_ != nullptr && callbacks_.OH_AudioRenderer_OnError != nullptr,
        "renderer client or error callback funtion is nullptr");
    OH_AudioStream_Result error = GetErrorResult(errorCode);
    callbacks_.OH_AudioRenderer_OnError(ohAudioRenderer_, userData_, error);
}

void OHAudioRendererDeviceChangeCallbackWithInfo::OnOutputDeviceChange(const DeviceInfo &deviceInfo,
    const AudioStreamDeviceChangeReason reason)
{
    CHECK_AND_RETURN_LOG(ohAudioRenderer_ != nullptr, "renderer client is nullptr");
    CHECK_AND_RETURN_LOG(callback_ != nullptr, "pointer to the function is nullptr");

    callback_(ohAudioRenderer_, userData_, static_cast<OH_AudioStream_DeviceChangeReason>(reason));
}

void OHAudioRenderer::SetInterruptMode(InterruptMode mode)
{
    CHECK_AND_RETURN_LOG(audioRenderer_ != nullptr, "renderer client is nullptr");
    audioRenderer_->SetInterruptMode(mode);
}

void OHAudioRenderer::SetSilentModeAndMixWithOthers(bool on)
{
    CHECK_AND_RETURN_LOG(audioRenderer_ != nullptr, "renderer client is nullptr");
    audioRenderer_->SetSilentModeAndMixWithOthers(on);
}

bool OHAudioRenderer::GetSilentModeAndMixWithOthers()
{
    CHECK_AND_RETURN_RET_LOG(audioRenderer_ != nullptr, false, "renderer client is nullptr");
    return audioRenderer_->GetSilentModeAndMixWithOthers();
}

int32_t OHAudioRenderer::SetDefaultOutputDevice(DeviceType deviceType)
{
    CHECK_AND_RETURN_RET_LOG(audioRenderer_ != nullptr, ERROR, "renderer client is nullptr");
    return audioRenderer_->SetDefaultOutputDevice(deviceType);
}

void OHAudioRenderer::SetRendererCallbackType(WriteDataCallbackType writeDataCallbackType)
{
    writeDataCallbackType_ = writeDataCallbackType;
}

WriteDataCallbackType OHAudioRenderer::GetRendererCallbackType()
{
    return writeDataCallbackType_;
}
}  // namespace AudioStandard
}  // namespace OHOS
