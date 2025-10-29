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
#define LOG_TAG "AudioLoopbackImpl"
#endif

#include "taihe_audio_loopback.h"

#include "taihe_param_utils.h"
#include "taihe_audio_error.h"
#include "taihe_audio_enum.h"
#include "taihe_audio_loopback_callback.h"
#include "audio_stream_manager.h"
#include "audio_manager_log.h"

namespace ANI::Audio {
static constexpr double MIN_VOLUME_IN_DOUBLE = 0.0;
static constexpr double MAX_VOLUME_IN_DOUBLE = 1.0;
std::mutex AudioLoopbackImpl::createMutex_;
int32_t AudioLoopbackImpl::isConstructSuccess_ = OHOS::AudioStandard::SUCCESS;
OHOS::AudioStandard::AudioLoopbackMode AudioLoopbackImpl::sLoopbackMode_ = OHOS::AudioStandard::LOOPBACK_HARDWARE;

AudioLoopbackImpl::AudioLoopbackImpl() : loopback_(nullptr) {}

AudioLoopbackImpl::AudioLoopbackImpl(std::shared_ptr<AudioLoopbackImpl> obj)
    : loopback_(nullptr)
{
    if (obj != nullptr) {
        loopback_ = obj->loopback_;
        callbackTaihe_ = obj->callbackTaihe_;
    }
}

AudioLoopbackImpl::~AudioLoopbackImpl() = default;

AudioLpback AudioLoopbackImpl::CreateAudioLoopbackWrapper(OHOS::AudioStandard::AudioLoopbackMode loopbackMode)
{
    std::lock_guard<std::mutex> lock(createMutex_);
    AudioLpback result = AudioLpback::make_type_null();
    sLoopbackMode_ = loopbackMode;
    std::shared_ptr<AudioLoopbackImpl> impl = std::make_shared<AudioLoopbackImpl>();
    if (impl == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "No memory");
        return result;
    }
    auto streamManager = OHOS::AudioStandard::AudioStreamManager::GetInstance();
    if (streamManager != nullptr && streamManager->IsAudioLoopbackSupported(loopbackMode)) {
        impl->loopback_ = OHOS::AudioStandard::AudioLoopback::CreateAudioLoopback(loopbackMode);
        if (impl->loopback_  == nullptr) {
            AUDIO_ERR_LOG("AudioLoopback Create failed");
            AudioLoopbackImpl::isConstructSuccess_ = TAIHE_ERR_NO_PERMISSION;
        }
    } else {
        AUDIO_ERR_LOG("AudioLoopback not supported");
        AudioLoopbackImpl::isConstructSuccess_ = TAIHE_ERR_UNSUPPORTED;
    }

    if (impl->loopback_ != nullptr && impl->callbackTaihe_ == nullptr) {
        impl->callbackTaihe_ = std::make_shared<TaiheAudioLoopbackCallback>();
        if (impl->callbackTaihe_ == nullptr) {
            TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "No memory");
            return result;
        }
        int32_t ret = impl->loopback_->SetAudioLoopbackCallback(impl->callbackTaihe_);
        if (ret) {
            TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "Construct SetLoopbackCallback failed");
            return result;
        }
    }
    return AudioLpback::make_type_audioLpback(make_holder<AudioLoopbackImpl, AudioLoopback>(impl));
}

AudioLoopbackStatus AudioLoopbackImpl::GetStatusSync()
{
    if (loopback_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "loopback_ is nullptr");
        return AudioLoopbackStatus::key_t::UNAVAILABLE_DEVICE;
    }
    return TaiheAudioEnum::ToTaiheAudioLoopbackStatus(loopback_->GetStatus());
}

void AudioLoopbackImpl::SetVolumeSync(double volume)
{
    if (loopback_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "loopback_ is nullptr");
        return;
    }

    if (volume < MIN_VOLUME_IN_DOUBLE || volume > MAX_VOLUME_IN_DOUBLE) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM);
        return;
    }

    int32_t ret = loopback_->SetVolume(static_cast<float>(volume));
    if (ret != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM);
        return;
    }
}

bool AudioLoopbackImpl::EnableSync(bool enable)
{
    if (loopback_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "loopback_ is nullptr");
        return false;
    }
    return loopback_->Enable(enable);
}

void AudioLoopbackImpl::OnStatusChange(callback_view<void(AudioLoopbackStatus data)> callback)
{
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterLoopbackCallback(cacheCallback, STATUS_CHANGE_CALLBACK_NAME, this);
}

void AudioLoopbackImpl::RegisterLoopbackCallback(std::shared_ptr<uintptr_t> &callback,
    const std::string &cbName, AudioLoopbackImpl *taiheLoopback)
{
    CHECK_AND_RETURN_RET_LOG(taiheLoopback != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "taiheLoopback is nullptr");
    CHECK_AND_RETURN_RET_LOG(taiheLoopback->loopback_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "loopback_ is nullptr");
    CHECK_AND_RETURN_RET_LOG(taiheLoopback->callbackTaihe_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "callbackTaihe_ is nullptr");

    std::shared_ptr<TaiheAudioLoopbackCallback> cb =
        std::static_pointer_cast<TaiheAudioLoopbackCallback>(taiheLoopback->callbackTaihe_);
    cb->SaveCallbackReference(cbName, callback);
}

void AudioLoopbackImpl::OffStatusChange(optional_view<callback<void(AudioLoopbackStatus data)>> callback)
{
    std::shared_ptr<uintptr_t> cacheCallback = nullptr;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterLoopbackCallback(cacheCallback, STATUS_CHANGE_CALLBACK_NAME, this);
}

template <typename T>
static void UnregisterAudioLoopbackSingletonCallbackTemplate(std::shared_ptr<uintptr_t> &callback,
    const std::string &cbName, std::shared_ptr<T> cb, std::function<int32_t(std::shared_ptr<T> callbackPtr,
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
    CHECK_AND_RETURN_LOG(ret == OHOS::AudioStandard::SUCCESS, "Unset of Loopback info change call failed");
    return;
}

void AudioLoopbackImpl::UnregisterLoopbackCallback(std::shared_ptr<uintptr_t> &callback,
    const std::string &cbName, AudioLoopbackImpl *taiheLoopback)
{
    CHECK_AND_RETURN_RET_LOG(taiheLoopback != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "taiheLoopback is nullptr");
    CHECK_AND_RETURN_RET_LOG(taiheLoopback->loopback_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "loopback_ is nullptr");
    CHECK_AND_RETURN_RET_LOG(taiheLoopback->callbackTaihe_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "callbackTaihe_ is nullptr");

    std::shared_ptr<TaiheAudioLoopbackCallback> cb =
        std::static_pointer_cast<TaiheAudioLoopbackCallback>(taiheLoopback->callbackTaihe_);
    UnregisterAudioLoopbackSingletonCallbackTemplate(callback, cbName, cb);
    AUDIO_DEBUG_LOG("UnregisterLoopbackCallback is successful");
}

AudioLpback CreateAudioLoopbackSync(AudioLoopbackMode mode)
{
    AudioLpback result = AudioLpback::make_type_null();
    int32_t audioLoopbackMode = mode.get_value();
    if ((!TaiheAudioEnum::IsLegalInputArgumentAudioLoopbackMode(audioLoopbackMode))) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "loopback mode invaild");
        return result;
    }
    OHOS::AudioStandard::AudioLoopbackMode loopbackMode =
        static_cast<OHOS::AudioStandard::AudioLoopbackMode>(audioLoopbackMode);
    AudioLpback output = AudioLoopbackImpl::CreateAudioLoopbackWrapper(loopbackMode);
    // IsConstructSuccess_ Used when creating a loopback fails.
    if (AudioLoopbackImpl::isConstructSuccess_ != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(AudioLoopbackImpl::isConstructSuccess_);
        AudioLoopbackImpl::isConstructSuccess_ = OHOS::AudioStandard::SUCCESS;
    }
    return output;
}
} // namespace ANI::Audio

TH_EXPORT_CPP_API_CreateAudioLoopbackSync(ANI::Audio::CreateAudioLoopbackSync);