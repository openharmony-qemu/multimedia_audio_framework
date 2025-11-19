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

DeviceType AudioSessionManagerImpl::GetDefaultOutputDevice()
{
    if (audioSessionMngr_ == nullptr) {
        AUDIO_ERR_LOG("audioSessionMngr_ is nullptr");
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "can not get session");
        return DeviceType::key_t::INVALID;
    }

    OHOS::AudioStandard::DeviceType deviceType = OHOS::AudioStandard::DeviceType::DEVICE_TYPE_INVALID;
    int32_t ret = audioSessionMngr_->GetDefaultOutputDevice(deviceType);
    if (ret != OHOS::AudioStandard::SUCCESS) {
        AUDIO_ERR_LOG("GetDefaultOutputDevice Failed, ret = %{public}d", ret);
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_ILLEGAL_STATE, "get deviceType state error");
        return DeviceType::key_t::INVALID;
    }
    return TaiheAudioEnum::ToTaiheDeviceType(deviceType);
}

void AudioSessionManagerImpl::SetDefaultOutputDeviceSync(DeviceType deviceType)
{
    if (audioSessionMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioSessionMngr_ is nullptr");
        return;
    }

    if (!(TaiheAudioEnum::IsLegalInputArgumentDefaultOutputDeviceType(deviceType.get_value()))) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
            "parameter verification failed: The param of mode must be enum deviceType");
        return;
    }

    OHOS::AudioStandard::DeviceType audioDeviceType =
        static_cast<OHOS::AudioStandard::DeviceType>(deviceType.get_value());
    int32_t ret = audioSessionMngr_->SetDefaultOutputDevice(audioDeviceType);
    if (ret != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM);
        return;
    }
}

void AudioSessionManagerImpl::SetAudioSessionScene(AudioSessionScene scene)
{
    if (audioSessionMngr_ == nullptr) {
        AUDIO_ERR_LOG("audioSessionMngr_ is nullptr");
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "can not get session");
        return;
    }

    if (!(TaiheAudioEnum::IsLegalInputArgumentSessionScene(scene.get_value()))) {
        AUDIO_ERR_LOG("valueType invalid");
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
            "parameter verification failed: The param of scene must be enum AudioSessionScene");
        return;
    }

    int32_t ret =
        audioSessionMngr_->SetAudioSessionScene(static_cast<OHOS::AudioStandard::AudioSessionScene>(scene.get_value()));
    if (ret == OHOS::AudioStandard::ERR_NOT_SUPPORTED) {
        AUDIO_ERR_LOG("SetAudioSessionScene Failed, not supported ret = %{public}d", ret);
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_ILLEGAL_STATE);
    } else if (ret != OHOS::AudioStandard::SUCCESS) {
        AUDIO_ERR_LOG("SetAudioSessionScene Failed, ret = %{public}d", ret);
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM);
    }
    return;
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
    std::shared_ptr<uintptr_t> cacheCallback = nullptr;
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

void AudioSessionManagerImpl::OnAudioSessionStateChanged(
    callback_view<void(AudioSessionStateChangedEvent const&)> callback)
{
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterAudioSessionStateCallback(cacheCallback, this);
}

void AudioSessionManagerImpl::OffAudioSessionStateChanged(
    optional_view<callback<void(AudioSessionStateChangedEvent const&)>> callback)
{
    std::shared_ptr<uintptr_t> cacheCallback = nullptr;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
        UnregisterSessionStateCallbackCarryParam(cacheCallback, this);
    } else {
        UnregisterSessionStateCallback(this);
    }
}

void AudioSessionManagerImpl::RegisterAudioSessionStateCallback(std::shared_ptr<uintptr_t> &callback,
    AudioSessionManagerImpl *taiheSessionManager)
{
    if (callback == nullptr) {
        AUDIO_ERR_LOG("OnAudioSessionStateChangeCallback failed, callback function is nullptr");
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "callback function is nullptr");
        return;
    }

    if ((taiheSessionManager == nullptr) || (taiheSessionManager->audioSessionMngr_ == nullptr)) {
        AUDIO_ERR_LOG("AudioSessionManagerImpl can not get session mgr taihe instance");
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "can not get session mgr taihe instance");
        return;
    }

    std::lock_guard<std::mutex> lock(taiheSessionManager->sessionStateCbMutex_);
    CHECK_AND_RETURN_LOG(GetAudioSessionStateCallback(callback, taiheSessionManager) == nullptr,
        "The callback function already registered.");

    std::shared_ptr<OHOS::AudioStandard::AudioSessionStateChangedCallback> stateChangedCallback =
        std::make_shared<TaiheAudioSessionStateCallback>();
    if (stateChangedCallback == nullptr) {
        AUDIO_ERR_LOG("AudioSessionManagerImpl: Memory Allocation Failed!");
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY, "Memory Allocation Failed!");
        return;
    }

    int32_t ret = taiheSessionManager->audioSessionMngr_->SetAudioSessionStateChangeCallback(stateChangedCallback);
    if (ret != OHOS::AudioStandard::SUCCESS) {
        AUDIO_ERR_LOG("SetAudioSessionStateChangeCallback is failed, ret = %{public}d", ret);
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM);
        return;
    }

    std::shared_ptr<TaiheAudioSessionStateCallback> cb =
        std::static_pointer_cast<TaiheAudioSessionStateCallback>(stateChangedCallback);
    taiheSessionManager->sessionStateCallbackList_.push_back(cb);
    cb->SaveCallbackReference(callback);

    AUDIO_INFO_LOG("OnAudioSessionStateChangeCallback is successful");
}

void AudioSessionManagerImpl::UnregisterSessionStateCallbackCarryParam(std::shared_ptr<uintptr_t> &callback,
    AudioSessionManagerImpl *taiheSessionManager)
{
    AUDIO_INFO_LOG("UnregisterCallback StateChanged.");
    if (callback == nullptr) {
        AUDIO_ERR_LOG("UnregisterSessionStateCallbackCarryParam failed, callback function is nullptr");
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "callback function is nullptr");
        return;
    }

    if ((taiheSessionManager == nullptr) || (taiheSessionManager->audioSessionMngr_ == nullptr)) {
        AUDIO_ERR_LOG("AudioSessionManagerImpl can not get session mgr taihe instance");
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "can not get session mgr taihe instance");
        return;
    }

    std::lock_guard<std::mutex> lock(taiheSessionManager->sessionStateCbMutex_);
    std::shared_ptr<TaiheAudioSessionStateCallback> cb = GetAudioSessionStateCallback(callback, taiheSessionManager);
    CHECK_AND_RETURN_LOG(cb != nullptr, "The callback function not registered.");
    std::shared_ptr<OHOS::AudioStandard::AudioSessionStateChangedCallback> stateChangedCallback =
        std::static_pointer_cast<OHOS::AudioStandard::AudioSessionStateChangedCallback>(cb);

    int32_t ret = taiheSessionManager->audioSessionMngr_->UnsetAudioSessionStateChangeCallback(stateChangedCallback);
    if (ret != OHOS::AudioStandard::SUCCESS) {
        AUDIO_ERR_LOG("UnregisterSessionStateCallbackCarryParam Failed, ret = %{public}d", ret);
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM);
        return;
    }

    taiheSessionManager->sessionStateCallbackList_.remove(cb);
    cb.reset();

    AUDIO_ERR_LOG("UnregisterSessionStateCallbackCarryParam Success");
}

void AudioSessionManagerImpl::UnregisterSessionStateCallback(AudioSessionManagerImpl *taiheSessionManager)
{
    AUDIO_INFO_LOG("UnregisterCallback state");
    if ((taiheSessionManager == nullptr) || (taiheSessionManager->audioSessionMngr_ == nullptr)) {
        AUDIO_ERR_LOG("AudioSessionManagerImpl can not get session mgr taihe instance");
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "can not get session mgr taihe instance");
        return;
    }

    std::lock_guard<std::mutex> lock(taiheSessionManager->sessionStateCbMutex_);
    CHECK_AND_RETURN_LOG(!taiheSessionManager->sessionStateCallbackList_.empty(),
        "Not register callback function, no need unregister.");

    int32_t ret = taiheSessionManager->audioSessionMngr_->UnsetAudioSessionStateChangeCallback();
    if (ret != OHOS::AudioStandard::SUCCESS) {
        AUDIO_ERR_LOG("UnsetAudioSessionStateChangeCallback Failed, ret = %{public}d", ret);
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM);
        return;
    }

    for (auto it = taiheSessionManager->sessionStateCallbackList_.rbegin();
        it != taiheSessionManager->sessionStateCallbackList_.rend(); ++it) {
        std::shared_ptr<TaiheAudioSessionStateCallback> cb =
            std::static_pointer_cast<TaiheAudioSessionStateCallback>(*it);
        cb.reset();
    }
    taiheSessionManager->sessionStateCallbackList_.clear();

    AUDIO_INFO_LOG("UnregisterSessionStateCallback Success");
}

void AudioSessionManagerImpl::OnCurrentOutputDeviceChanged(
    callback_view<void(CurrentOutputDeviceChangedEvent const&)> callback)
{
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterAudioSessionDeviceCallback(cacheCallback, this);
}

void AudioSessionManagerImpl::OffCurrentOutputDeviceChanged(
    optional_view<callback<void(CurrentOutputDeviceChangedEvent const&)>> callback)
{
    std::shared_ptr<uintptr_t> cacheCallback = nullptr;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
        UnregisterSessionDeviceCallbackCarryParam(cacheCallback, this);
    } else {
        UnregisterSessionDeviceCallback(this);
    }
}

void AudioSessionManagerImpl::RegisterAudioSessionDeviceCallback(std::shared_ptr<uintptr_t> &callback,
    AudioSessionManagerImpl *taiheSessionManager)
{
    if (callback == nullptr) {
        AUDIO_ERR_LOG("OnAudioSessionDeviceCallback failed, callback function is nullptr");
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "callback function is nullptr");
        return;
    }

    if ((taiheSessionManager == nullptr) || (taiheSessionManager->audioSessionMngr_ == nullptr)) {
        AUDIO_ERR_LOG("AudioSessionManagerImpl can not get session mgr taihe instance");
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "can not get session mgr taihe instance");
        return;
    }

    CHECK_AND_RETURN_LOG(GetAudioSessionDeviceCallback(callback, taiheSessionManager) == nullptr,
        "The callback function already registered.");

    std::shared_ptr<OHOS::AudioStandard::AudioSessionCurrentDeviceChangedCallback> deviceChangedCallback =
        std::make_shared<TaiheAudioSessionDeviceCallback>();
    if (deviceChangedCallback == nullptr) {
        AUDIO_ERR_LOG("AudioSessionManagerImpl: Memory Allocation Failed!");
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY, "Memory Allocation Failed!");
        return;
    }

    int32_t ret = taiheSessionManager->audioSessionMngr_->SetAudioSessionCurrentDeviceChangeCallback(
        deviceChangedCallback);
    if (ret != OHOS::AudioStandard::SUCCESS) {
        AUDIO_ERR_LOG("RegisterAudioSessionDeviceCallback is failed, ret = %{public}d", ret);
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM);
        return;
    }

    std::shared_ptr<TaiheAudioSessionDeviceCallback> cb =
        std::static_pointer_cast<TaiheAudioSessionDeviceCallback>(deviceChangedCallback);
    taiheSessionManager->sessionDeviceCallbackList_.push_back(cb);
    cb->SaveCallbackReference(callback);

    AUDIO_INFO_LOG("RegisterAudioSessionDeviceCallback is successful");
}

void AudioSessionManagerImpl::UnregisterSessionDeviceCallbackCarryParam(std::shared_ptr<uintptr_t> &callback,
    AudioSessionManagerImpl *taiheSessionManager)
{
    AUDIO_INFO_LOG("UnregisterCallback device changed.");
    if (callback == nullptr) {
        AUDIO_ERR_LOG("UnregisterSessionDeviceCallbackCarryParam failed, callback function is nullptr");
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "callback function is nullptr");
        return;
    }

    if ((taiheSessionManager == nullptr) || (taiheSessionManager->audioSessionMngr_ == nullptr)) {
        AUDIO_ERR_LOG("AudioSessionManagerImpl can not get session mgr taihe instance");
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "can not get session mgr taihe instance");
        return;
    }

    std::lock_guard<std::mutex> lock(taiheSessionManager->sessionDeviceCbMutex_);
    std::shared_ptr<TaiheAudioSessionDeviceCallback> cb = GetAudioSessionDeviceCallback(callback, taiheSessionManager);
    CHECK_AND_RETURN_LOG(cb != nullptr, "The callback function not registered.");
    std::shared_ptr<OHOS::AudioStandard::AudioSessionCurrentDeviceChangedCallback> deviceCallback =
        std::static_pointer_cast<OHOS::AudioStandard::AudioSessionCurrentDeviceChangedCallback>(cb);

    int32_t ret = taiheSessionManager->audioSessionMngr_->UnsetAudioSessionCurrentDeviceChangeCallback(deviceCallback);
    if (ret != OHOS::AudioStandard::SUCCESS) {
        AUDIO_ERR_LOG("UnsetAudioSessionCurrentDeviceChangeCallback is failed, ret = %{public}d", ret);
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM);
        return;
    }

    taiheSessionManager->sessionDeviceCallbackList_.remove(cb);
    cb.reset();

    AUDIO_ERR_LOG("UnregisterSessionDeviceCallbackCarryParam Success");
}

void AudioSessionManagerImpl::UnregisterSessionDeviceCallback(AudioSessionManagerImpl *taiheSessionManager)
{
    AUDIO_INFO_LOG("UnregisterCallback device");
    if ((taiheSessionManager == nullptr) || (taiheSessionManager->audioSessionMngr_ == nullptr)) {
        AUDIO_ERR_LOG("AudioSessionManagerImpl can not get session mgr taihe instance");
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "can not get session mgr taihe instance");
        return;
    }

    std::lock_guard<std::mutex> lock(taiheSessionManager->sessionDeviceCbMutex_);
    CHECK_AND_RETURN_LOG(!taiheSessionManager->sessionDeviceCallbackList_.empty(),
        "Not register callback function, no need unregister.");

    int32_t ret = taiheSessionManager->audioSessionMngr_->UnsetAudioSessionCurrentDeviceChangeCallback();
    if (ret != OHOS::AudioStandard::SUCCESS) {
        AUDIO_ERR_LOG("UnregisterSessionDeviceCallback is failed, ret = %{public}d", ret);
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM);
        return;
    }

    for (auto it = taiheSessionManager->sessionDeviceCallbackList_.rbegin();
        it != taiheSessionManager->sessionDeviceCallbackList_.rend(); ++it) {
        std::shared_ptr<TaiheAudioSessionDeviceCallback> cb =
            std::static_pointer_cast<TaiheAudioSessionDeviceCallback>(*it);
        cb.reset();
    }
    taiheSessionManager->sessionDeviceCallbackList_.clear();

    AUDIO_INFO_LOG("UnregisterSessionDeviceCallback Success");
}

std::shared_ptr<TaiheAudioSessionStateCallback> AudioSessionManagerImpl::GetAudioSessionStateCallback(
    std::shared_ptr<uintptr_t> &callback, AudioSessionManagerImpl *taiheSessionManager)
{
    CHECK_AND_RETURN_RET_LOG(taiheSessionManager != nullptr, nullptr, "taiheSessionManager is nullptr");
    std::shared_ptr<TaiheAudioSessionStateCallback> cb = nullptr;
    for (auto &iter : taiheSessionManager->sessionStateCallbackList_) {
        if (iter == nullptr) {
            continue;
        }

        if (iter->ContainSameJsCallback(callback)) {
            cb = iter;
        }
    }
    return cb;
}

std::shared_ptr<TaiheAudioSessionDeviceCallback> AudioSessionManagerImpl::GetAudioSessionDeviceCallback(
    std::shared_ptr<uintptr_t> &callback, AudioSessionManagerImpl *taiheSessionManager)
{
    CHECK_AND_RETURN_RET_LOG(taiheSessionManager != nullptr, nullptr, "taiheSessionManager is nullptr");
    std::shared_ptr<TaiheAudioSessionDeviceCallback> cb = nullptr;
    for (auto &iter : taiheSessionManager->sessionDeviceCallbackList_) {
        if (iter == nullptr) {
            continue;
        }

        if (iter->ContainSameJsCallback(callback)) {
            cb = iter;
        }
    }
    return cb;
}
} // namespace ANI::Audio