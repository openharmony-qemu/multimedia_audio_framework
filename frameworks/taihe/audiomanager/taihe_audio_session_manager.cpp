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
#define LOG_TAG "AudioSessionManagerImpl"
#endif

#include "taihe_audio_session_manager.h"
#include "audio_errors.h"
#include "taihe_audio_error.h"
#include "taihe_param_utils.h"
#include "taihe_audio_enum.h"

namespace ANI::Audio {
using namespace OHOS::HiviewDFX;

AudioSessionManagerImpl::AudioSessionManagerImpl() : audioSessionMngr_(nullptr) {}

AudioSessionManagerImpl::AudioSessionManagerImpl(OHOS::AudioStandard::AudioSessionManager *audioSessionMngr)
    : audioSessionMngr_(nullptr)
{
    if (audioSessionMngr != nullptr) {
        audioSessionMngr_ = audioSessionMngr;
    }
}

AudioSessionManagerImpl::~AudioSessionManagerImpl() = default;

AudioSessionManager AudioSessionManagerImpl::CreateSessionManagerWrapper()
{
    auto *audioSessionMngr = OHOS::AudioStandard::AudioSessionManager::GetInstance();
    if (audioSessionMngr == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "Failed to get AudioSessionManager instance");
        return make_holder<AudioSessionManagerImpl, AudioSessionManager>(nullptr);
    }
    return make_holder<AudioSessionManagerImpl, AudioSessionManager>(audioSessionMngr);
}

void AudioSessionManagerImpl::ActivateAudioSessionSync(AudioSessionStrategy const &strategy)
{
    OHOS::AudioStandard::AudioSessionStrategy audioSessionStrategy;
    int32_t result = TaiheParamUtils::GetAudioSessionStrategy(audioSessionStrategy, strategy);
    if (result != AUDIO_OK) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "GetAudioSessionStrategy failed");
        return;
    }
    if (audioSessionMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioSessionMngr_ is nullptr");
        return;
    }
    result = audioSessionMngr_->ActivateAudioSession(audioSessionStrategy);
    if (result != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "System error. ActivateAudioSession fail.");
        return;
    }
}

void AudioSessionManagerImpl::DeactivateAudioSessionSync()
{
    if (audioSessionMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioSessionMngr_ is nullptr");
        return;
    }
    int32_t result = audioSessionMngr_->DeactivateAudioSession();
    if (result != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "System error. DeactivateAudioSession fail.");
        return;
    }
}

bool AudioSessionManagerImpl::IsAudioSessionActivated()
{
    if (audioSessionMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioSessionMngr_ is nullptr");
        return false;
    }
    return audioSessionMngr_->IsAudioSessionActivated();
}

void AudioSessionManagerImpl::OnAudioSessionDeactivated(
    callback_view<void(AudioSessionDeactivatedEvent const&)> callback)
{
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterAudioSessionCallback(cacheCallback, this);
}

void AudioSessionManagerImpl::OffAudioSessionDeactivated(
    optional_view<callback<void(AudioSessionDeactivatedEvent const&)>> callback)
{
    std::shared_ptr<uintptr_t> cacheCallback;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
        UnregisterCallbackCarryParam(cacheCallback, this);
    } else {
        UnregisterCallback(this);
    }
}

void AudioSessionManagerImpl::RegisterAudioSessionCallback(std::shared_ptr<uintptr_t> &callback,
    AudioSessionManagerImpl *taiheSessionManager)
{
    CHECK_AND_RETURN_LOG((taiheSessionManager != nullptr) &&
        (taiheSessionManager->audioSessionMngr_ != nullptr), "Failed to retrieve session mgr taihe instance.");
    std::lock_guard<std::mutex> lock(taiheSessionManager->mutex_);
    if (!taiheSessionManager->audioSessionCallbackTaihe_) {
        taiheSessionManager->audioSessionCallbackTaihe_ = std::make_shared<TaiheAudioSessionCallback>();
        CHECK_AND_RETURN_LOG(taiheSessionManager->audioSessionCallbackTaihe_ != nullptr,
            "AudioSessionManagerImpl: Memory Allocation Failed !!");

        int32_t ret = taiheSessionManager->audioSessionMngr_->SetAudioSessionCallback(
            taiheSessionManager->audioSessionCallbackTaihe_);
        CHECK_AND_RETURN_LOG(ret == OHOS::AudioStandard::SUCCESS,
            "Registering of AudioSessionDeactiveEvent Callback Failed");
    }

    std::shared_ptr<TaiheAudioSessionCallback> cb =
        std::static_pointer_cast<TaiheAudioSessionCallback>(taiheSessionManager->audioSessionCallbackTaihe_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    cb->SaveCallbackReference(callback);
    AUDIO_INFO_LOG("OnRendererStateChangeCallback is successful");
}

void AudioSessionManagerImpl::UnregisterCallbackCarryParam(std::shared_ptr<uintptr_t> &callback,
    AudioSessionManagerImpl *taiheSessionManager)
{
    AUDIO_INFO_LOG("UnregisterCallback");
    CHECK_AND_RETURN_LOG((taiheSessionManager != nullptr) &&
        (taiheSessionManager->audioSessionMngr_ != nullptr), "Failed to retrieve session mgr taihe instance.");
    std::lock_guard<std::mutex> lock(taiheSessionManager->mutex_);
    if (!taiheSessionManager->audioSessionCallbackTaihe_) {
        taiheSessionManager->audioSessionCallbackTaihe_ = std::make_shared<TaiheAudioSessionCallback>();
        CHECK_AND_RETURN_LOG(taiheSessionManager->audioSessionCallbackTaihe_ != nullptr,
            "Memory Allocation Failed !!");
        int32_t ret = taiheSessionManager->audioSessionMngr_->UnsetAudioSessionCallback(
            taiheSessionManager->audioSessionCallbackTaihe_);
        CHECK_AND_RETURN_LOG(ret == OHOS::AudioStandard::SUCCESS, "Unregister Callback CarryParam Failed");
    }
    std::shared_ptr<TaiheAudioSessionCallback> cb =
        std::static_pointer_cast<TaiheAudioSessionCallback>(taiheSessionManager->audioSessionCallbackTaihe_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    cb->SaveCallbackReference(callback);
    AUDIO_ERR_LOG("Unset AudioSessionCallback Success");
}

void AudioSessionManagerImpl::UnregisterCallback(AudioSessionManagerImpl *taiheSessionManager)
{
    AUDIO_INFO_LOG("UnregisterCallback");
    CHECK_AND_RETURN_LOG((taiheSessionManager != nullptr) &&
        (taiheSessionManager->audioSessionMngr_ != nullptr), "Failed to retrieve session mgr taihe instance.");
    std::lock_guard<std::mutex> lock(taiheSessionManager->mutex_);

    int32_t ret = taiheSessionManager->audioSessionMngr_->UnsetAudioSessionCallback();
    if (ret) {
        AUDIO_ERR_LOG("Unset AudioSessionCallback Failed");
        return;
    }
    if (taiheSessionManager->audioSessionCallbackTaihe_ != nullptr) {
        taiheSessionManager->audioSessionCallbackTaihe_.reset();
        taiheSessionManager->audioSessionCallbackTaihe_ = nullptr;
    }
    AUDIO_ERR_LOG("Unset AudioSessionCallback Success");
}
} // namespace ANI::Audio