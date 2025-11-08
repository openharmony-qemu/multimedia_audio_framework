/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioRendererImpl"
#endif

#include "taihe_audio_renderer.h"
#if defined(ANDROID_PLATFORM) || defined(IOS_PLATFORM)
#include "errors.h"
#else
#ifdef FEATURE_HIVIEW_ENABLE
#include "xpower_event_js.h"
#endif
#endif
#include "taihe_audio_enum.h"
#include "taihe_audio_error.h"
#include "taihe_param_utils.h"
#include "taihe_audio_renderer_callback.h"
#include "taihe_renderer_position_callback.h"
#include "taihe_renderer_period_position_callback.h"
#include "taihe_audio_renderer_policy_service_died_callback.h"
#include "taihe_audio_renderer_write_data_callback.h"
#include "audio_utils.h"

namespace ANI::Audio {
std::unique_ptr<OHOS::AudioStandard::AudioRendererOptions> AudioRendererImpl::sRendererOptions_ = nullptr;
std::mutex AudioRendererImpl::createMutex_;
int32_t AudioRendererImpl::isConstructSuccess_ = OHOS::AudioStandard::SUCCESS;
constexpr double MIN_VOLUME_IN_DOUBLE = 0.0;
constexpr double MAX_VOLUME_IN_DOUBLE = 1.0;
constexpr uint32_t DEFAULT_ARRAY_SIZE = 0;
constexpr uint64_t SEC_TO_NANOSECOND = 1000000000;
static constexpr double MIN_LOUDNESS_GAIN_IN_DOUBLE = -90.0;
static constexpr double MAX_LOUDNESS_GAIN_IN_DOUBLE = 24.0;

template <typename T>
static void UnregisterAudioRendererSingletonCallbackTemplate(std::shared_ptr<uintptr_t> &callback,
    const std::string &cbName, std::shared_ptr<T> cb,
    std::function<int32_t(std::shared_ptr<T> callbackPtr,
        std::shared_ptr<uintptr_t> callback)> removeFunction = nullptr)
{
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    if (callback != nullptr) {
        CHECK_AND_RETURN_LOG(cb->ContainSameJsCallbackInner(cbName, callback), "callback not exists!");
    }
    cb->RemoveCallbackReference(cbName, callback);

    if (removeFunction == nullptr) {
        return;
    }
    int32_t ret = removeFunction(cb, callback);
    CHECK_AND_RETURN_LOG(ret == OHOS::AudioStandard::SUCCESS, "Unset of Renderer info change call failed");
    return;
}

AudioRendererImpl::AudioRendererImpl()
    : audioRenderer_(nullptr), contentType_(OHOS::AudioStandard::ContentType::CONTENT_TYPE_MUSIC),
    streamUsage_(OHOS::AudioStandard::StreamUsage::STREAM_USAGE_MEDIA) {}

AudioRendererImpl::AudioRendererImpl(std::shared_ptr<AudioRendererImpl> obj)
    : audioRenderer_(nullptr), contentType_(OHOS::AudioStandard::ContentType::CONTENT_TYPE_MUSIC),
    streamUsage_(OHOS::AudioStandard::StreamUsage::STREAM_USAGE_MEDIA)
{
    if (obj != nullptr) {
        audioRenderer_ = obj->audioRenderer_;
        contentType_ = obj->contentType_;
        streamUsage_ = obj->streamUsage_;
        callbackTaihe_ = obj->callbackTaihe_;
    }
}

void AudioRendererImpl::CreateRendererFailed()
{
    AudioRendererImpl::isConstructSuccess_ = TAIHE_ERR_SYSTEM;
    if (OHOS::AudioStandard::AudioRenderer::CheckMaxRendererInstances() == OHOS::ERR_OVERFLOW) {
        AudioRendererImpl::isConstructSuccess_ = TAIHE_ERR_STREAM_LIMIT;
    }
    AUDIO_ERR_LOG("Renderer Create failed %{public}d", AudioRendererImpl::isConstructSuccess_);
}

std::shared_ptr<AudioRendererImpl> AudioRendererImpl::CreateAudioRendererNativeObject()
{
    std::shared_ptr<AudioRendererImpl> audioRendererImpl = std::make_shared<AudioRendererImpl>();
    CHECK_AND_RETURN_RET_LOG(audioRendererImpl != nullptr, nullptr, "No memory");
    CHECK_AND_RETURN_RET_LOG(sRendererOptions_ != nullptr, nullptr, "sRendererOptions_ is nullptr");
    audioRendererImpl->contentType_ = sRendererOptions_->rendererInfo.contentType;
    audioRendererImpl->streamUsage_ = sRendererOptions_->rendererInfo.streamUsage;

    OHOS::AudioStandard::AudioRendererOptions rendererOptions = *sRendererOptions_;
    /* AudioRenderer not support other rendererFlags, only support flag 0 */
    if (rendererOptions.rendererInfo.rendererFlags != 0) {
        rendererOptions.rendererInfo.rendererFlags = 0;
    }
#if !defined(ANDROID_PLATFORM) && !defined(IOS_PLATFORM)
    audioRendererImpl->audioRenderer_ = OHOS::AudioStandard::AudioRenderer::CreateRenderer(rendererOptions);
#else
    std::string cacheDir = "";
    audioRendererImpl->audioRenderer_ = OHOS::AudioStandard::AudioRenderer::Create(cacheDir, rendererOptions);
#endif
    if (audioRendererImpl->audioRenderer_ == nullptr) {
        AUDIO_ERR_LOG("Renderer Create failed");
        CreateRendererFailed();
        return nullptr;
    }

    if (audioRendererImpl->streamUsage_ == OHOS::AudioStandard::STREAM_USAGE_UNKNOWN) {
        audioRendererImpl->audioRenderer_->SetOffloadAllowed(false);
    }

    if (audioRendererImpl->audioRenderer_ != nullptr && audioRendererImpl->callbackTaihe_ == nullptr) {
        audioRendererImpl->callbackTaihe_ = std::make_shared<TaiheAudioRendererCallback>();
        CHECK_AND_RETURN_RET_LOG(audioRendererImpl->callbackTaihe_ != nullptr, audioRendererImpl, "No memory");
        int32_t ret = audioRendererImpl->audioRenderer_->SetRendererCallback(audioRendererImpl->callbackTaihe_);
        CHECK_AND_RETURN_RET_LOG(ret == OHOS::AudioStandard::SUCCESS,
            audioRendererImpl, "Construct SetRendererCallback failed");
    }
    return audioRendererImpl;
}

AudioRenderer AudioRendererImpl::CreateAudioRendererWrapper(OHOS::AudioStandard::AudioRendererOptions rendererOptions)
{
    std::lock_guard<std::mutex> lock(createMutex_);
    sRendererOptions_ = std::make_unique<OHOS::AudioStandard::AudioRendererOptions>();
    if (sRendererOptions_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "sRendererOptions_ create failed");
        return make_holder<AudioRendererImpl, AudioRenderer>(nullptr);
    }
    *sRendererOptions_ = rendererOptions;
    std::shared_ptr<AudioRendererImpl> impl = AudioRendererImpl::CreateAudioRendererNativeObject();
    if (impl == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "failed to CreateAudioRendererNativeObject");
        return make_holder<AudioRendererImpl, AudioRenderer>(nullptr);
    }
    return make_holder<AudioRendererImpl, AudioRenderer>(impl);
}

void AudioRendererImpl::StartSync()
{
    if (audioRenderer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioRenderer_ is nullptr");
        return;
    }
    bool ret = audioRenderer_->Start();
    if (!ret) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM);
        return;
    }
}

int64_t AudioRendererImpl::GetAudioTimeSync()
{
    if (audioRenderer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioRenderer_ is nullptr");
        return 0;
    }

    OHOS::AudioStandard::Timestamp timestamp;
    bool ret = audioRenderer_->GetAudioTime(timestamp, OHOS::AudioStandard::Timestamp::Timestampbase::MONOTONIC);
    if (!ret) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "GetAudioTime failure!");
        return 0;
    }
    uint64_t time = static_cast<uint64_t>(timestamp.time.tv_nsec) +
        static_cast<uint64_t>(timestamp.time.tv_sec) * SEC_TO_NANOSECOND;
    return static_cast<int64_t>(time);
}

AudioTimestampInfo AudioRendererImpl::GetAudioTimestampInfoSync()
{
    AudioTimestampInfo emptyTimestampInfo = {
        .framePos = 0,
        .timestamp = 0
    };
    if (audioRenderer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioRenderer_ is nullptr");
        return emptyTimestampInfo;
    }
    OHOS::AudioStandard::Timestamp timeStamp;
    int32_t ret = audioRenderer_->GetAudioTimestampInfo(timeStamp,
        OHOS::AudioStandard::Timestamp::Timestampbase::MONOTONIC);
    if (ret != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "GetAudioTimeStamp failure!");
        return emptyTimestampInfo;
    }

    AudioTimestampInfo result = TaiheParamUtils::ToTaiheAudioTimestampInfo(timeStamp);
    return result;
}

void AudioRendererImpl::DrainSync()
{
    if (audioRenderer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioRenderer_ is nullptr");
        return;
    }
    bool ret = audioRenderer_->Drain();
    if (!ret) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "Drain failure!");
        return;
    }
}

void AudioRendererImpl::FlushSync()
{
    if (audioRenderer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioRenderer_ is nullptr");
        return;
    }
    bool ret = audioRenderer_->Flush();
    if (!ret) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_ILLEGAL_STATE, "Flush failure!");
        return;
    }
}

void AudioRendererImpl::PauseSync()
{
    if (audioRenderer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioRenderer_ is nullptr");
        return;
    }
    bool ret = audioRenderer_->Pause();
    if (!ret) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "Pause failure!");
        return;
    }
}

void AudioRendererImpl::StopSync()
{
    if (audioRenderer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioRenderer_ is nullptr");
        return;
    }
    bool ret = audioRenderer_->Stop();
    if (!ret) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "Stop failure!");
        return;
    }
}

void AudioRendererImpl::ReleaseSync()
{
    if (audioRenderer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioRenderer_ is nullptr");
        return;
    }
    bool ret = audioRenderer_->Release();
    if (!ret) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "Release failure!");
        return;
    }
}

int64_t AudioRendererImpl::GetBufferSizeSync()
{
    size_t bufferSize = 0;
    if (audioRenderer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioRenderer_ is nullptr");
        return static_cast<int64_t>(bufferSize);
    }
    if (audioRenderer_->GetBufferSize(bufferSize) != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "GetBufferSizeSync failed!");
        return 0;
    }
    return static_cast<int64_t>(bufferSize);
}

int64_t AudioRendererImpl::GetAudioStreamIdSync()
{
    uint32_t audioStreamId = 0;
    if (audioRenderer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioRenderer_ is nullptr");
        return audioStreamId;
    }
    int32_t ret = audioRenderer_->GetAudioStreamId(audioStreamId);
    if (ret != OHOS::AudioStandard::SUCCESS) {
        if (ret == OHOS::AudioStandard::ERR_INVALID_INDEX) {
            TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "GetAudioStreamId failure!");
        } else if (ret == OHOS::AudioStandard::ERR_ILLEGAL_STATE) {
            TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_ILLEGAL_STATE, "GetAudioStreamId failure!");
        } else {
            TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "GetAudioStreamId failure!");
        }
        return audioStreamId;
    }
    return static_cast<int64_t>(audioStreamId);
}

void AudioRendererImpl::SetVolumeSync(double volume)
{
    if (audioRenderer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioRenderer_ is nullptr");
        return;
    }
    if (volume < MIN_VOLUME_IN_DOUBLE || volume > MAX_VOLUME_IN_DOUBLE) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_UNSUPPORTED);
        AUDIO_ERR_LOG("SetVolume volume unsupported");
        return;
    }
    int32_t ret = audioRenderer_->SetVolume(static_cast<float>(volume));
    if (ret != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM);
    }
}

double AudioRendererImpl::GetVolume()
{
    if (audioRenderer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioRenderer_ is nullptr");
        return 0;
    }
    double volLevel = audioRenderer_->GetVolume();
    return volLevel;
}

AudioRendererInfo AudioRendererImpl::GetRendererInfoSync()
{
    AudioRendererInfo emptyRendererInfo = {
        .usage = StreamUsage::key_t::STREAM_USAGE_UNKNOWN,
        .rendererFlags = 0,
        .volumeMode = taihe::optional<AudioVolumeMode>(std::in_place_t{}, AudioVolumeMode::key_t::SYSTEM_GLOBAL),
    };
    if (audioRenderer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioRenderer_ is nullptr");
        return emptyRendererInfo;
    }
    OHOS::AudioStandard::AudioRendererInfo rendererInfo = {};
    int32_t ret = audioRenderer_->GetRendererInfo(rendererInfo);
    if (ret != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "GetRendererInfo failure!");
        return emptyRendererInfo;
    }
    AudioRendererInfo result = TaiheParamUtils::ToTaiheRendererInfo(rendererInfo);
    return result;
}

AudioStreamInfo AudioRendererImpl::GetStreamInfoSync()
{
    AudioStreamInfo emptyStreamInfo {
        .samplingRate = ohos::multimedia::audio::AudioSamplingRate::key_t::SAMPLE_RATE_48000,
        .channels = ohos::multimedia::audio::AudioChannel::key_t::CHANNEL_2,
        .sampleFormat = ohos::multimedia::audio::AudioSampleFormat::key_t::SAMPLE_FORMAT_S16LE,
        .encodingType = ohos::multimedia::audio::AudioEncodingType::key_t::ENCODING_TYPE_RAW,
    };
    if (audioRenderer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioRenderer_ is nullptr");
        return emptyStreamInfo;
    }
    OHOS::AudioStandard::AudioStreamInfo streamInfo;
    int32_t ret = audioRenderer_->GetStreamInfo(streamInfo);
    if (ret != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "GetStreamInfo failure!");
        return emptyStreamInfo;
    }
    std::shared_ptr<OHOS::AudioStandard::AudioStreamInfo> streamInfoPtr =
        std::make_shared<OHOS::AudioStandard::AudioStreamInfo>(streamInfo);
    AudioStreamInfo result = TaiheParamUtils::ToTaiheAudioStreamInfo(streamInfoPtr);
    return result;
}

void AudioRendererImpl::SetInterruptModeSync(InterruptMode mode)
{
    if (audioRenderer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioRenderer_ is nullptr");
        return;
    }
    int32_t interruptMode = mode.get_value();
    if (!TaiheAudioEnum::IsLegalInputArgumentInterruptMode(interruptMode)) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
            "parameter verification failed: The param of mode must be enum InterruptMode");
        return;
    }
    audioRenderer_->SetInterruptMode(TaiheAudioEnum::GetNativeInterruptMode(interruptMode));
}

double AudioRendererImpl::GetMinStreamVolumeSync()
{
    if (audioRenderer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioRenderer_ is nullptr");
        return 0;
    }
    double volLevel = audioRenderer_->GetMinStreamVolume();
    return volLevel;
}

double AudioRendererImpl::GetMaxStreamVolumeSync()
{
    if (audioRenderer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioRenderer_ is nullptr");
        return 0;
    }
    double volLevel = audioRenderer_->GetMaxStreamVolume();
    return volLevel;
}

taihe::array<AudioDeviceDescriptor> AudioRendererImpl::GetCurrentOutputDevicesSync()
{
    if (audioRenderer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioRenderer_ is nullptr");
        return taihe::array<AudioDeviceDescriptor>(nullptr, DEFAULT_ARRAY_SIZE);
    }
    OHOS::AudioStandard::AudioDeviceDescriptor deviceInfo(OHOS::AudioStandard::AudioDeviceDescriptor::DEVICE_INFO);
    int32_t ret = audioRenderer_->GetCurrentOutputDevices(deviceInfo);
    if (ret != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "GetCurrentOutputDevices failure!");
        return taihe::array<AudioDeviceDescriptor>(nullptr, DEFAULT_ARRAY_SIZE);
    }
    return TaiheParamUtils::SetValueDeviceInfo(deviceInfo);
}

int64_t AudioRendererImpl::GetUnderflowCountSync()
{
    uint32_t underflowCount = 0;
    if (audioRenderer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioRenderer_ is nullptr");
        return underflowCount;
    }
    underflowCount = audioRenderer_->GetUnderflowCount();
    return static_cast<int64_t>(underflowCount);
}

AudioEffectMode AudioRendererImpl::GetAudioEffectModeSync()
{
    if (audioRenderer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioRenderer_ is nullptr");
        return AudioEffectMode::key_t::EFFECT_NONE;
    }
    return TaiheAudioEnum::ToTaiheAudioEffectMode(audioRenderer_->GetAudioEffectMode());
}

void AudioRendererImpl::SetAudioEffectModeSync(AudioEffectMode mode)
{
    if (audioRenderer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioRenderer_ is nullptr");
        return;
    }
    int32_t effectMode = mode.get_value();
    if (!TaiheAudioEnum::IsLegalInputArgumentAudioEffectMode(effectMode)) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
            "parameter verification failed: The param of mode must be enum InterruptMode");
        return;
    }
    OHOS::AudioStandard::AudioEffectMode audioEffectMode =
        static_cast<OHOS::AudioStandard::AudioEffectMode>(effectMode);
    int32_t ret = audioRenderer_->SetAudioEffectMode(audioEffectMode);
    if (ret != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM);
        return;
    }
}

void AudioRendererImpl::SetChannelBlendMode(ChannelBlendMode mode)
{
    if (audioRenderer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioRenderer_ is nullptr");
        return;
    }
    int32_t channelBlendMode = mode.get_value();
    if (!TaiheAudioEnum::IsLegalInputArgumentChannelBlendMode(channelBlendMode)) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
            "parameter verification failed: The param of mode must be enum InterruptMode");
        return;
    }
    int32_t ret =
        audioRenderer_->SetChannelBlendMode(static_cast<OHOS::AudioStandard::ChannelBlendMode>(channelBlendMode));
    if (ret == OHOS::AudioStandard::ERR_ILLEGAL_STATE) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_ILLEGAL_STATE, "err illegal state");
        return;
    }
}

void AudioRendererImpl::SetVolumeWithRamp(double volume, int32_t duration)
{
    if (audioRenderer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioRenderer_ is nullptr");
        return;
    }
    if (!((volume >= MIN_VOLUME_IN_DOUBLE) && (volume <= MAX_VOLUME_IN_DOUBLE))) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
            "parameter verification failed: invaild volume index");
        AUDIO_ERR_LOG("invaild volume index");
        return;
    }
    int32_t ret = audioRenderer_->SetVolumeWithRamp(static_cast<float>(volume), duration);
    if (ret == OHOS::AudioStandard::ERR_ILLEGAL_STATE) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_ILLEGAL_STATE, "err illegal state");
        return;
    }
}

void AudioRendererImpl::SetSpeed(double speed)
{
    if (audioRenderer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioRenderer_ is nullptr");
        return;
    }

    if (!((speed >= OHOS::AudioStandard::MIN_STREAM_SPEED_LEVEL) &&
        (speed <= OHOS::AudioStandard::MAX_STREAM_SPEED_LEVEL))) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
            "parameter verification failed: invaild volume index");
        return;
    }
    int32_t ret = audioRenderer_->SetSpeed(static_cast<float>(speed));
    if (ret == OHOS::AudioStandard::ERR_ILLEGAL_STATE) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_ILLEGAL_STATE, "err illegal state");
        return;
    }
}

double AudioRendererImpl::GetSpeed()
{
    if (audioRenderer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioRenderer_ is nullptr");
        return 0;
    }
    double ret = audioRenderer_->GetSpeed();
    return ret;
}

void AudioRendererImpl::SetTargetSync(RenderTarget target)
{
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "Caller is not a system application.");
        return;
    }
    if (audioRenderer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioRenderer_ is nullptr");
        return;
    }
    int32_t renderTarget = target.get_value();
    if (!TaiheAudioEnum::IsLegalRenderTarget(renderTarget)) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "Parameter verification failed.");
        return;
    }
    int32_t ret = audioRenderer_->SetTarget(static_cast<OHOS::AudioStandard::RenderTarget>(renderTarget));
    if (ret == OHOS::AudioStandard::SUCCESS) {
        return;
    }
    if (ret == OHOS::AudioStandard::ERR_PERMISSION_DENIED) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_PERMISSION, "Permission denied.");
    } else if (ret == OHOS::AudioStandard::ERR_SYSTEM_PERMISSION_DENIED) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "Caller is not a system application.");
    } else if (ret == OHOS::AudioStandard::ERR_ILLEGAL_STATE) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_ILLEGAL_STATE,
            "Operation not permit at running and release state.");
    } else if (ret == OHOS::AudioStandard::ERR_NOT_SUPPORTED) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_UNSUPPORTED,
            "Current renderer is not supported to set target.");
    } else {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM,
            "Audio client call audio service error, System error.");
    }
    return;
}

RenderTarget AudioRendererImpl::GetTarget()
{
    if (audioRenderer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioRenderer_ is nullptr");
        return RenderTarget::key_t::NORMAL_PLAYBACK;
    }
    OHOS::AudioStandard::RenderTarget target = audioRenderer_->GetTarget();
    RenderTarget result = TaiheAudioEnum::ToTaiheRenderTarget(target);
    return result;
}

AudioState AudioRendererImpl::GetState()
{
    if (audioRenderer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioRenderer_ is nullptr");
        return AudioState::key_t::STATE_INVALID;
    }
    OHOS::AudioStandard::RendererState state = audioRenderer_->GetStatus();
    return TaiheAudioEnum::ToTaiheAudioState(state);
}

void AudioRendererImpl::SetSilentModeAndMixWithOthers(bool on)
{
    if (audioRenderer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioRenderer_ is nullptr");
        return;
    }
    audioRenderer_->SetSilentModeAndMixWithOthers(on);
}

bool AudioRendererImpl::GetSilentModeAndMixWithOthers()
{
    if (audioRenderer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioRenderer_ is nullptr");
        return false;
    }
    bool on = audioRenderer_->GetSilentModeAndMixWithOthers();
    return on;
}

void AudioRendererImpl::SetDefaultOutputDeviceSync(DeviceType deviceType)
{
    if (audioRenderer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioRenderer_ is nullptr");
        return;
    }

    int32_t deviceTypeValue = deviceType.get_value();
    if (!TaiheAudioEnum::IsLegalInputArgumentDefaultOutputDeviceType(deviceTypeValue)) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
            "parameter verification failed: The param of mode must be enum InterruptMode");
        return;
    }
    OHOS::AudioStandard::DeviceType audioDeviceType = static_cast<OHOS::AudioStandard::DeviceType>(deviceTypeValue);
    int32_t ret = audioRenderer_->SetDefaultOutputDevice(audioDeviceType);
    if (ret != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_ILLEGAL_STATE, "SetDefaultOutputDevice failure!");
        return;
    }
}

void AudioRendererImpl::SetLoudnessGainSync(double loudnessGain)
{
    if (audioRenderer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioRenderer_ is nullptr");
        return;
    }

    OHOS::AudioStandard::AudioRendererInfo rendererInfo = {};
    audioRenderer_->GetRendererInfo(rendererInfo);
    OHOS::AudioStandard::StreamUsage streamUsage = rendererInfo.streamUsage;
    if (streamUsage != OHOS::AudioStandard::STREAM_USAGE_MUSIC &&
        streamUsage != OHOS::AudioStandard::STREAM_USAGE_MOVIE &&
        streamUsage != OHOS::AudioStandard::STREAM_USAGE_AUDIOBOOK) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_UNSUPPORTED);
        return;
    }
    if (loudnessGain < MIN_LOUDNESS_GAIN_IN_DOUBLE ||
        loudnessGain > MAX_LOUDNESS_GAIN_IN_DOUBLE) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERROR_INVALID_PARAM);
        return;
    }
    int32_t ret = audioRenderer_->SetLoudnessGain(static_cast<float>(loudnessGain));
    if (ret != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "SetLoudnessGainSync failure!");
        return;
    }
}

double AudioRendererImpl::GetLoudnessGain()
{
    if (audioRenderer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioRenderer_ is nullptr");
        return 0;
    }
    OHOS::AudioStandard::AudioRendererInfo rendererInfo = {};
    audioRenderer_->GetRendererInfo(rendererInfo);
    OHOS::AudioStandard::StreamUsage streamUsage = rendererInfo.streamUsage;
    if (!(streamUsage == OHOS::AudioStandard::STREAM_USAGE_MUSIC ||
        streamUsage == OHOS::AudioStandard::STREAM_USAGE_MOVIE ||
        streamUsage == OHOS::AudioStandard::STREAM_USAGE_AUDIOBOOK)) {
        double result = static_cast<double>(0.0f);
        return result;
    }
    double loudnessGain = audioRenderer_->GetLoudnessGain();
    return loudnessGain;
}

void AudioRendererImpl::RegisterRendererCallback(std::shared_ptr<uintptr_t> &callback,
    const std::string &cbName, AudioRendererImpl *taiheRenderer)
{
    CHECK_AND_RETURN_RET_LOG(taiheRenderer != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "taiheRenderer is nullptr");
    std::lock_guard<std::mutex> lock(taiheRenderer->mutex_);
    CHECK_AND_RETURN_RET_LOG(taiheRenderer->audioRenderer_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "audioRenderer_ is nullptr");
    CHECK_AND_RETURN_RET_LOG(taiheRenderer->callbackTaihe_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "callbackTaihe_ is nullptr");
    std::shared_ptr<TaiheAudioRendererCallback> cb =
        std::static_pointer_cast<TaiheAudioRendererCallback>(taiheRenderer->callbackTaihe_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    cb->SaveCallbackReference(cbName, callback);
}

void AudioRendererImpl::RegisterPeriodPositionCallback(int64_t frame, std::shared_ptr<uintptr_t> &callback,
    const std::string &cbName, AudioRendererImpl *taiheRenderer)
{
    CHECK_AND_RETURN_RET_LOG(taiheRenderer != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "taiheRenderer is nullptr");
    std::lock_guard<std::mutex> lock(taiheRenderer->mutex_);
    CHECK_AND_RETURN_RET_LOG(taiheRenderer->audioRenderer_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "audioRenderer_ is nullptr");
    if (frame > 0) {
        if (taiheRenderer->periodPositionCbTaihe_ == nullptr) {
            taiheRenderer->periodPositionCbTaihe_ = std::make_shared<TaiheRendererPeriodPositionCallback>();
            CHECK_AND_RETURN_RET_LOG(taiheRenderer->periodPositionCbTaihe_ != nullptr,
                TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY),
                "periodPositionCbTaihe_ is nullptr, No memory");

            int32_t ret = taiheRenderer->audioRenderer_->SetRendererPeriodPositionCallback(frame,
                taiheRenderer->periodPositionCbTaihe_);
            CHECK_AND_RETURN_RET_LOG(ret == OHOS::AudioStandard::SUCCESS,
                TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM),
                "SetRendererPeriodPositionCallback failed");

            std::shared_ptr<TaiheRendererPeriodPositionCallback> cb =
                std::static_pointer_cast<TaiheRendererPeriodPositionCallback>(taiheRenderer->periodPositionCbTaihe_);
            CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
            cb->SaveCallbackReference(cbName, callback);
        } else {
            AUDIO_DEBUG_LOG("periodReach already subscribed.");
        }
    } else {
        AUDIO_ERR_LOG("frame value not supported!!");
    }
}

void AudioRendererImpl::RegisterPositionCallback(int64_t markPosition, std::shared_ptr<uintptr_t> &callback,
    const std::string &cbName, AudioRendererImpl *taiheRenderer)
{
    CHECK_AND_RETURN_RET_LOG(taiheRenderer != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "taiheRenderer is nullptr");
    std::lock_guard<std::mutex> lock(taiheRenderer->mutex_);
    CHECK_AND_RETURN_RET_LOG(taiheRenderer->audioRenderer_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "audioRenderer_ is nullptr");
    CHECK_AND_RETURN_RET_LOG(markPosition > 0, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_INPUT_INVALID, "parameter verification failed: The param of frame is not supported"),
        "Mark Position value not supported!!");
    taiheRenderer->positionCbTaihe_ = std::make_shared<TaiheRendererPositionCallback>();
    CHECK_AND_RETURN_RET_LOG(taiheRenderer->positionCbTaihe_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "positionCbTaihe_ is nullptr");
    int32_t ret = taiheRenderer->audioRenderer_->SetRendererPositionCallback(markPosition,
        taiheRenderer->positionCbTaihe_);
    CHECK_AND_RETURN_RET_LOG(ret == OHOS::AudioStandard::SUCCESS,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM), "SetRendererPositionCallback fail");

    std::shared_ptr<TaiheRendererPositionCallback> cb =
        std::static_pointer_cast<TaiheRendererPositionCallback>(taiheRenderer->positionCbTaihe_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    cb->SaveCallbackReference(cbName, callback);
}

void AudioRendererImpl::RegisterRendererDeviceChangeCallback(std::shared_ptr<uintptr_t> &callback,
    AudioRendererImpl *taiheRenderer)
{
    CHECK_AND_RETURN_RET_LOG(taiheRenderer != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "taiheRenderer is nullptr");
    std::lock_guard<std::mutex> lock(taiheRenderer->mutex_);
    CHECK_AND_RETURN_RET_LOG(taiheRenderer->audioRenderer_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "audioRenderer_ is nullptr");
    if (!taiheRenderer->rendererDeviceChangeCallbackTaihe_) {
        taiheRenderer->rendererDeviceChangeCallbackTaihe_ =
            std::make_shared<TaiheAudioRendererDeviceChangeCallback>();
        CHECK_AND_RETURN_LOG(taiheRenderer->rendererDeviceChangeCallbackTaihe_ != nullptr,
            "rendererDeviceChangeCallbackTaihe_ is nullptr, No memory");

        int32_t retDeviceChange = taiheRenderer->audioRenderer_->RegisterOutputDeviceChangeWithInfoCallback(
            taiheRenderer->rendererDeviceChangeCallbackTaihe_);
        CHECK_AND_RETURN_LOG(retDeviceChange == OHOS::AudioStandard::SUCCESS,
            "Registering of Renderer Device Change Callback Failed");
    }

    if (!taiheRenderer->rendererPolicyServiceDiedCallbackTaihe_) {
        taiheRenderer->rendererPolicyServiceDiedCallbackTaihe_ =
            std::make_shared<TaiheAudioRendererPolicyServiceDiedCallback>(taiheRenderer);
        CHECK_AND_RETURN_LOG(taiheRenderer->rendererPolicyServiceDiedCallbackTaihe_ != nullptr,
            "Registering of Renderer Device Change Callback Failed");

        int32_t retAudioPolicy = taiheRenderer->audioRenderer_->RegisterAudioPolicyServerDiedCb(getpid(),
            taiheRenderer->rendererPolicyServiceDiedCallbackTaihe_);
        CHECK_AND_RETURN_LOG(retAudioPolicy == OHOS::AudioStandard::SUCCESS,
            "Registering of AudioPolicyService Died Change Callback Failed");
    }

    std::shared_ptr<TaiheAudioRendererDeviceChangeCallback> cb =
        std::static_pointer_cast<TaiheAudioRendererDeviceChangeCallback>(
        taiheRenderer->rendererDeviceChangeCallbackTaihe_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    cb->AddCallbackReference(callback);
    AUDIO_INFO_LOG("RegisterRendererStateChangeCallback is successful");
}

void AudioRendererImpl::RegisterRendererOutputDeviceChangeWithInfoCallback(std::shared_ptr<uintptr_t> &callback,
    AudioRendererImpl *taiheRenderer)
{
    CHECK_AND_RETURN_RET_LOG(taiheRenderer != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "taiheRenderer is nullptr");
    std::lock_guard<std::mutex> lock(taiheRenderer->mutex_);
    CHECK_AND_RETURN_RET_LOG(taiheRenderer->audioRenderer_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "audioRenderer_ is nullptr");
    if (!taiheRenderer->rendererOutputDeviceChangeWithInfoCallbackTaihe_) {
        taiheRenderer->rendererOutputDeviceChangeWithInfoCallbackTaihe_ =
            std::make_shared<TaiheAudioRendererOutputDeviceChangeWithInfoCallback>();
        CHECK_AND_RETURN_LOG(taiheRenderer->rendererOutputDeviceChangeWithInfoCallbackTaihe_ != nullptr,
            "rendererOutputDeviceChangeWithInfoCallbackTaihe_ is nullptr, No memory");

        int32_t retOutputDevice = taiheRenderer->audioRenderer_->RegisterOutputDeviceChangeWithInfoCallback(
            taiheRenderer->rendererOutputDeviceChangeWithInfoCallbackTaihe_);
        CHECK_AND_RETURN_LOG(retOutputDevice == OHOS::AudioStandard::SUCCESS,
            "Registering of Renderer Device Change Callback Failed");
    }

    if (!taiheRenderer->rendererPolicyServiceDiedCallbackTaihe_) {
        taiheRenderer->rendererPolicyServiceDiedCallbackTaihe_ =
            std::make_shared<TaiheAudioRendererPolicyServiceDiedCallback>(taiheRenderer);
        CHECK_AND_RETURN_LOG(taiheRenderer->rendererPolicyServiceDiedCallbackTaihe_ != nullptr,
            "Registering of Renderer Device Change Callback Failed");

        int32_t retPolicyService = taiheRenderer->audioRenderer_->RegisterAudioPolicyServerDiedCb(getpid(),
            taiheRenderer->rendererPolicyServiceDiedCallbackTaihe_);
        CHECK_AND_RETURN_LOG(retPolicyService == OHOS::AudioStandard::SUCCESS,
            "Registering of AudioPolicyService Died Change Callback Failed");
    }

    std::shared_ptr<TaiheAudioRendererOutputDeviceChangeWithInfoCallback> cb =
        taiheRenderer->rendererOutputDeviceChangeWithInfoCallbackTaihe_;
    cb->AddCallbackReference(callback);
    AUDIO_INFO_LOG("RegisterRendererStateChangeCallback is successful");
}

void AudioRendererImpl::RegisterRendererWriteDataCallback(std::shared_ptr<uintptr_t> &callback,
    const std::string &cbName, AudioRendererImpl *taiheRenderer)
{
    CHECK_AND_RETURN_RET_LOG(taiheRenderer != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "taiheRenderer is nullptr");
    std::lock_guard<std::mutex> lock(taiheRenderer->mutex_);
    CHECK_AND_RETURN_RET_LOG(taiheRenderer->audioRenderer_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "audioRenderer_ is nullptr");
    if (taiheRenderer->rendererWriteDataCallbackTaihe_ != nullptr) {
        AUDIO_WARNING_LOG("writeData already subscribed. The old writeData function will be overwritten.");
    }

    taiheRenderer->rendererWriteDataCallbackTaihe_ =
        std::make_shared<TaiheRendererWriteDataCallback>(taiheRenderer);
    taiheRenderer->audioRenderer_->SetRenderMode(OHOS::AudioStandard::RENDER_MODE_CALLBACK);
    CHECK_AND_RETURN_LOG(taiheRenderer->rendererWriteDataCallbackTaihe_ != nullptr, "writeDataCbTaihe_ is nullpur");
    int32_t ret = taiheRenderer->audioRenderer_->SetRendererWriteCallback(
        taiheRenderer->rendererWriteDataCallbackTaihe_);
    CHECK_AND_RETURN_LOG(ret == OHOS::AudioStandard::SUCCESS, "SetRendererWriteCallback failed");
    std::shared_ptr<TaiheRendererWriteDataCallback> cb =
        std::static_pointer_cast<TaiheRendererWriteDataCallback>(taiheRenderer->rendererWriteDataCallbackTaihe_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    cb->AddCallbackReference(cbName, callback);
    AUDIO_INFO_LOG("Register Callback is successful");
}

void AudioRendererImpl::UnregisterRendererCallback(std::shared_ptr<uintptr_t> &callback,
    const std::string &cbName, AudioRendererImpl *taiheRenderer)
{
    CHECK_AND_RETURN_RET_LOG(taiheRenderer != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "taiheRenderer is nullptr");
    std::lock_guard<std::mutex> lock(taiheRenderer->mutex_);
    CHECK_AND_RETURN_RET_LOG(taiheRenderer->audioRenderer_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "audioRenderer_ is nullptr");
    CHECK_AND_RETURN_LOG(taiheRenderer->callbackTaihe_ != nullptr, "taiheRendererCallback is nullptr");

    std::shared_ptr<TaiheAudioRendererCallback> cb =
        std::static_pointer_cast<TaiheAudioRendererCallback>(taiheRenderer->callbackTaihe_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    UnregisterAudioRendererSingletonCallbackTemplate(callback, cbName, cb);
    AUDIO_DEBUG_LOG("UnregisterRendererCallback is successful");
}

void AudioRendererImpl::UnregisterRendererDeviceChangeCallback(std::shared_ptr<uintptr_t> &callback,
    AudioRendererImpl *taiheRenderer)
{
    CHECK_AND_RETURN_RET_LOG(taiheRenderer != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "taiheRenderer is nullptr");
    std::lock_guard<std::mutex> lock(taiheRenderer->mutex_);
    CHECK_AND_RETURN_RET_LOG(taiheRenderer->audioRenderer_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "audioRenderer_ is nullptr");
    CHECK_AND_RETURN_LOG(taiheRenderer->rendererDeviceChangeCallbackTaihe_ != nullptr,
        "rendererDeviceChangeCallbackTaihe_ is nullptr, return");

    CHECK_AND_RETURN_LOG(taiheRenderer->rendererPolicyServiceDiedCallbackTaihe_ != nullptr,
        "rendererPolicyServiceDiedCallbackTaihe_ is nullptr, return");

    std::shared_ptr<TaiheAudioRendererDeviceChangeCallback> cb =
        std::static_pointer_cast<TaiheAudioRendererDeviceChangeCallback>(
            taiheRenderer->rendererDeviceChangeCallbackTaihe_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    cb->RemoveCallbackReference(callback);
    if (callback == nullptr || cb->GetCallbackListSize() == 0) {
        int32_t ret = taiheRenderer->audioRenderer_->UnregisterOutputDeviceChangeWithInfoCallback(cb);
        CHECK_AND_RETURN_LOG(ret == OHOS::AudioStandard::SUCCESS, "unregister renderer device change callback failed");
        ret = taiheRenderer->audioRenderer_->UnregisterAudioPolicyServerDiedCb(getpid());
        CHECK_AND_RETURN_LOG(ret == OHOS::AudioStandard::SUCCESS, "unregister AudioPolicyServerDiedCb failed");
        taiheRenderer->DestroyTaiheCallbacks();
    }
    AUDIO_INFO_LOG("UnregisterRendererDeviceChangeCallback success!");
}

void AudioRendererImpl::UnregisterRendererOutputDeviceChangeWithInfoCallback(std::shared_ptr<uintptr_t> &callback,
    AudioRendererImpl *taiheRenderer)
{
    CHECK_AND_RETURN_RET_LOG(taiheRenderer != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "taiheRenderer is nullptr");
    std::lock_guard<std::mutex> lock(taiheRenderer->mutex_);
    CHECK_AND_RETURN_RET_LOG(taiheRenderer->audioRenderer_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "audioRenderer_ is nullptr");
    CHECK_AND_RETURN_LOG(taiheRenderer->rendererOutputDeviceChangeWithInfoCallbackTaihe_ != nullptr,
        "rendererOutputDeviceChangeWithInfoCallbackTaihe_ is nullptr, return");

    CHECK_AND_RETURN_LOG(taiheRenderer->rendererPolicyServiceDiedCallbackTaihe_ != nullptr,
        "rendererPolicyServiceDiedCallbackTaihe_ is nullptr, return");

    std::shared_ptr<TaiheAudioRendererOutputDeviceChangeWithInfoCallback> cb =
        taiheRenderer->rendererOutputDeviceChangeWithInfoCallbackTaihe_;
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    cb->RemoveCallbackReference(callback);
    if (callback == nullptr || cb->GetCallbackListSize() == 0) {
        int32_t ret = taiheRenderer->audioRenderer_->UnregisterOutputDeviceChangeWithInfoCallback(cb);
        CHECK_AND_RETURN_LOG(ret == OHOS::AudioStandard::SUCCESS,
            "UnregisterRendererOutputDeviceChangeWithInfoCallback failed");

        ret = taiheRenderer->audioRenderer_->UnregisterAudioPolicyServerDiedCb(getpid());
        CHECK_AND_RETURN_LOG(ret == OHOS::AudioStandard::SUCCESS, "unregister AudioPolicyServerDiedCb failed");

        taiheRenderer->DestroyTaiheCallbacks();
    }
    AUDIO_INFO_LOG("UnregisterRendererOutputDeviceChangeWithInfoCallback success");
}

void AudioRendererImpl::UnregisterPeriodPositionCallback(std::shared_ptr<uintptr_t> &callback,
    const std::string &cbName, AudioRendererImpl *taiheRenderer)
{
    CHECK_AND_RETURN_RET_LOG(taiheRenderer != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "taiheRenderer is nullptr");
    std::lock_guard<std::mutex> lock(taiheRenderer->mutex_);
    CHECK_AND_RETURN_RET_LOG(taiheRenderer->audioRenderer_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "audioRenderer_ is nullptr");
    CHECK_AND_RETURN_LOG(taiheRenderer->periodPositionCbTaihe_ != nullptr, "periodPositionCbTaihe is nullptr");

    std::shared_ptr<TaiheRendererPeriodPositionCallback> cb =
        std::static_pointer_cast<TaiheRendererPeriodPositionCallback>(taiheRenderer->periodPositionCbTaihe_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    std::function<int32_t(std::shared_ptr<TaiheRendererPeriodPositionCallback> callbackPtr,
        std::shared_ptr<uintptr_t> callbackFunction)> removeFunction =
        [&taiheRenderer] (std::shared_ptr<TaiheRendererPeriodPositionCallback> callbackPtr,
        std::shared_ptr<uintptr_t> callbackFunction) {
            taiheRenderer->audioRenderer_->UnsetRendererPeriodPositionCallback();
            taiheRenderer->periodPositionCbTaihe_ = nullptr;
            return OHOS::AudioStandard::SUCCESS;
        };
    UnregisterAudioRendererSingletonCallbackTemplate(callback, cbName, cb, removeFunction);
    AUDIO_DEBUG_LOG("UnregisterRendererPeriodPositionCallback is successful");
}

void AudioRendererImpl::UnregisterPositionCallback(std::shared_ptr<uintptr_t> &callback,
    const std::string &cbName, AudioRendererImpl *taiheRenderer)
{
    CHECK_AND_RETURN_RET_LOG(taiheRenderer != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "taiheRenderer is nullptr");
    std::lock_guard<std::mutex> lock(taiheRenderer->mutex_);
    CHECK_AND_RETURN_RET_LOG(taiheRenderer->audioRenderer_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "audioRenderer_ is nullptr");
    CHECK_AND_RETURN_LOG(taiheRenderer->positionCbTaihe_ != nullptr, "rendererCallbackTaihe is nullptr");

    std::shared_ptr<TaiheRendererPositionCallback> cb =
        std::static_pointer_cast<TaiheRendererPositionCallback>(taiheRenderer->positionCbTaihe_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    std::function<int32_t(std::shared_ptr<TaiheRendererPositionCallback> callbackPtr,
        std::shared_ptr<uintptr_t> callbackFunction)> removeFunction =
        [&taiheRenderer] (std::shared_ptr<TaiheRendererPositionCallback> callbackPtr,
        std::shared_ptr<uintptr_t> callbackFunction) {
            taiheRenderer->audioRenderer_->UnsetRendererPositionCallback();
            taiheRenderer->positionCbTaihe_ = nullptr;
            return OHOS::AudioStandard::SUCCESS;
        };
    UnregisterAudioRendererSingletonCallbackTemplate(callback, cbName, cb, removeFunction);
    AUDIO_DEBUG_LOG("UnregisterRendererPositionCallback is successful");
}

void AudioRendererImpl::UnregisterRendererWriteDataCallback(std::shared_ptr<uintptr_t> &callback,
    AudioRendererImpl *taiheRenderer)
{
    CHECK_AND_RETURN_RET_LOG(taiheRenderer != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "taiheRenderer is nullptr");
    std::lock_guard<std::mutex> lock(taiheRenderer->mutex_);
    CHECK_AND_RETURN_RET_LOG(taiheRenderer->audioRenderer_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "audioRenderer_ is nullptr");
    CHECK_AND_RETURN_LOG(taiheRenderer->rendererWriteDataCallbackTaihe_ != nullptr,
        "taiheRendererWriteDataCallback is nullptr, return");
    std::shared_ptr<TaiheRendererWriteDataCallback> cb =
        std::static_pointer_cast<TaiheRendererWriteDataCallback>(taiheRenderer->rendererWriteDataCallbackTaihe_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    cb->RemoveCallbackReference(callback);
    AUDIO_INFO_LOG("Unregister Callback is successful");
}

void AudioRendererImpl::OnStateChange(callback_view<void(AudioState)> callback)
{
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterRendererCallback(cacheCallback, STATE_CHANGE_CALLBACK_NAME, this);
}

void AudioRendererImpl::OnAudioInterrupt(callback_view<void(InterruptEvent const&)> callback)
{
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterRendererCallback(cacheCallback, AUDIO_INTERRUPT_CALLBACK_NAME, this);
}

void AudioRendererImpl::OnOutputDeviceChange(callback_view<void(array_view<AudioDeviceDescriptor>)> callback)
{
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterRendererDeviceChangeCallback(cacheCallback, this);
}

void AudioRendererImpl::OnOutputDeviceChangeWithInfo(callback_view<void(AudioStreamDeviceChangeInfo const&)> callback)
{
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterRendererOutputDeviceChangeWithInfoCallback(cacheCallback, this);
}

void AudioRendererImpl::OnPeriodReach(int64_t frame, callback_view<void(int64_t)> callback)
{
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterPeriodPositionCallback(frame, cacheCallback, PERIOD_REACH_CALLBACK_NAME, this);
}

void AudioRendererImpl::OnMarkReach(int64_t frame, callback_view<void(int64_t)> callback)
{
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterPositionCallback(frame, cacheCallback, MARK_REACH_CALLBACK_NAME, this);
}

void AudioRendererImpl::OnWriteData(callback_view<AudioDataCallbackResult(array_view<uint8_t>)> callback)
{
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterRendererWriteDataCallback(cacheCallback, WRITE_DATA_CALLBACK_NAME, this);
}

void AudioRendererImpl::OffAudioInterrupt(optional_view<callback<void(InterruptEvent const&)>> callback)
{
    std::shared_ptr<uintptr_t> cacheCallback = nullptr;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterRendererCallback(cacheCallback, AUDIO_INTERRUPT_CALLBACK_NAME, this);
}

void AudioRendererImpl::OffStateChange(optional_view<callback<void(AudioState)>> callback)
{
    std::shared_ptr<uintptr_t> cacheCallback = nullptr;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterRendererCallback(cacheCallback, STATE_CHANGE_CALLBACK_NAME, this);
}

void AudioRendererImpl::OffOutputDeviceChange(optional_view<callback<void(array_view<AudioDeviceDescriptor>)>> callback)
{
    std::shared_ptr<uintptr_t> cacheCallback = nullptr;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterRendererDeviceChangeCallback(cacheCallback, this);
}

void AudioRendererImpl::OffOutputDeviceChangeWithInfo(
    optional_view<callback<void(AudioStreamDeviceChangeInfo const&)>> callback)
{
    std::shared_ptr<uintptr_t> cacheCallback = nullptr;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterRendererOutputDeviceChangeWithInfoCallback(cacheCallback, this);
}

void AudioRendererImpl::OffPeriodReach(optional_view<callback<void(int64_t)>> callback)
{
    std::shared_ptr<uintptr_t> cacheCallback = nullptr;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterPeriodPositionCallback(cacheCallback, PERIOD_REACH_CALLBACK_NAME, this);
}

void AudioRendererImpl::OffMarkReach(optional_view<callback<void(int64_t)>> callback)
{
    std::shared_ptr<uintptr_t> cacheCallback = nullptr;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterPositionCallback(cacheCallback, MARK_REACH_CALLBACK_NAME, this);
}

void AudioRendererImpl::OffWriteData(optional_view<callback<AudioDataCallbackResult(array_view<uint8_t>)>> callback)
{
    std::shared_ptr<uintptr_t> cacheCallback = nullptr;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterRendererWriteDataCallback(cacheCallback, this);
}

void AudioRendererImpl::DestroyCallbacks()
{
    CHECK_AND_RETURN_LOG(rendererDeviceChangeCallbackTaihe_ != nullptr,
        "rendererDeviceChangeCallbackTaihe_ is nullptr");
    rendererDeviceChangeCallbackTaihe_->RemoveAllCallbacks();
    DestroyTaiheCallbacks();
}

void AudioRendererImpl::DestroyTaiheCallbacks()
{
    if (rendererDeviceChangeCallbackTaihe_ != nullptr) {
        rendererDeviceChangeCallbackTaihe_.reset();
        rendererDeviceChangeCallbackTaihe_ = nullptr;
    }

    if (rendererPolicyServiceDiedCallbackTaihe_ != nullptr) {
        rendererPolicyServiceDiedCallbackTaihe_.reset();
        rendererPolicyServiceDiedCallbackTaihe_ = nullptr;
    }
}

AudioRenderer CreateAudioRendererSync(AudioRendererOptions const &options)
{
    OHOS::AudioStandard::AudioRendererOptions rendererOptions;
    if (TaiheParamUtils::GetRendererOptions(&rendererOptions, options) != AUDIO_OK) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INPUT_INVALID,
            "parameter verification failed: The param of options must be interface AudioRendererOptions");
        AUDIO_ERR_LOG("get rendererOptions failed");
        return make_holder<AudioRendererImpl, AudioRenderer>(nullptr);
    }
    return AudioRendererImpl::CreateAudioRendererWrapper(rendererOptions);
}
} // namespace ANI::Audio

TH_EXPORT_CPP_API_CreateAudioRendererSync(ANI::Audio::CreateAudioRendererSync);
