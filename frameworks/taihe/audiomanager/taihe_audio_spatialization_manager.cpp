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
#define LOG_TAG "AudioSpatializationManagerImpl"
#endif

#include "taihe_audio_spatialization_manager.h"
#include "taihe_audio_error.h"
#include "taihe_param_utils.h"
#include "taihe_audio_enum.h"

using namespace ANI::Audio;

namespace ANI::Audio {
AudioSpatializationManagerImpl::AudioSpatializationManagerImpl() : audioSpatializationMngr_(nullptr) {}

AudioSpatializationManagerImpl::AudioSpatializationManagerImpl(std::unique_ptr<AudioSpatializationManagerImpl> obj)
{
    if (obj != nullptr) {
        audioSpatializationMngr_ = obj->audioSpatializationMngr_;
    }
}

AudioSpatializationManagerImpl::~AudioSpatializationManagerImpl() = default;

AudioSpatializationManager AudioSpatializationManagerImpl::CreateSpatializationManagerWrapper()
{
    std::unique_ptr<AudioSpatializationManagerImpl> audioSpatializationManagerImpl =
        std::make_unique<AudioSpatializationManagerImpl>();
    if (audioSpatializationManagerImpl != nullptr) {
        audioSpatializationManagerImpl->audioSpatializationMngr_ =
            OHOS::AudioStandard::AudioSpatializationManager::GetInstance();
        return make_holder<AudioSpatializationManagerImpl, AudioSpatializationManager>(
            std::move(audioSpatializationManagerImpl));
    }
    return make_holder<AudioSpatializationManagerImpl, AudioSpatializationManager>(nullptr);
}

void AudioSpatializationManagerImpl::OnSpatializationEnabledChangeForCurrentDevice(callback_view<void(bool)> callback)
{
    CHECK_AND_RETURN_RET_LOG(audioSpatializationMngr_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERROR_INVALID_PARAM),
        "audioSpatializationMngr_ is nullptr");
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterSpatializationEnabledChangeForCurrentDeviceCallback(cacheCallback, this);
}

void AudioSpatializationManagerImpl::OnSpatializationEnabledChangeForAnyDevice(
    callback_view<void(AudioSpatialEnabledStateForDevice const&)> callback)
{
    CHECK_AND_RETURN_RET_LOG(audioSpatializationMngr_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERROR_INVALID_PARAM),
        "audioSpatializationMngr_ is nullptr");
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterSpatializationEnabledChangeCallback(cacheCallback, this);
}

void AudioSpatializationManagerImpl::OnHeadTrackingEnabledChangeForAnyDevice(
    callback_view<void(AudioSpatialEnabledStateForDevice const&)> callback)
{
    CHECK_AND_RETURN_RET_LOG(audioSpatializationMngr_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERROR_INVALID_PARAM),
        "audioSpatializationMngr_ is nullptr");
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterHeadTrackingEnabledChangeCallback(cacheCallback, this);
}

void AudioSpatializationManagerImpl::OffSpatializationEnabledChangeForCurrentDevice(
    optional_view<callback<void(bool)>> callback)
{
    CHECK_AND_RETURN_RET_LOG(audioSpatializationMngr_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERROR_INVALID_PARAM),
        "audioSpatializationMngr_ is nullptr");
    std::shared_ptr<uintptr_t> cacheCallback;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterSpatializationEnabledChangeForCurrentDeviceCallback(cacheCallback, this);
}

void AudioSpatializationManagerImpl::OffSpatializationEnabledChangeForAnyDevice(
    optional_view<callback<void(AudioSpatialEnabledStateForDevice const&)>> callback)
{
    CHECK_AND_RETURN_RET_LOG(audioSpatializationMngr_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERROR_INVALID_PARAM),
        "audioSpatializationMngr_ is nullptr");
    std::shared_ptr<uintptr_t> cacheCallback;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterSpatializationEnabledChangeCallback(cacheCallback, this);
}

void AudioSpatializationManagerImpl::OffHeadTrackingEnabledChangeForAnyDevice(
    optional_view<callback<void(AudioSpatialEnabledStateForDevice const&)>> callback)
{
    CHECK_AND_RETURN_RET_LOG(audioSpatializationMngr_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERROR_INVALID_PARAM),
        "audioSpatializationMngr_ is nullptr");
    std::shared_ptr<uintptr_t> cacheCallback;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterHeadTrackingEnabledChangeCallback(cacheCallback, this);
}

void AudioSpatializationManagerImpl::RegisterSpatializationEnabledChangeForCurrentDeviceCallback(
    std::shared_ptr<uintptr_t> &callback, AudioSpatializationManagerImpl *taiheSpatializationManager)
{
    if (!taiheSpatializationManager->spatializationEnabledChangeForCurrentDeviceCallback_) {
        taiheSpatializationManager->spatializationEnabledChangeForCurrentDeviceCallback_ =
            std::make_shared<TaiheAudioCurrentSpatializationEnabledChangeCallback>(get_env());
        CHECK_AND_RETURN_LOG(taiheSpatializationManager->spatializationEnabledChangeForCurrentDeviceCallback_ !=
            nullptr, "AudioSpatializationManagerImpl: Memory Allocation Failed !!");

        int32_t ret = taiheSpatializationManager->audioSpatializationMngr_->
            RegisterSpatializationEnabledForCurrentDeviceEventListener(
            taiheSpatializationManager->spatializationEnabledChangeForCurrentDeviceCallback_);
        CHECK_AND_RETURN_LOG(ret == OHOS::AudioStandard::SUCCESS,
            "AudioSpatializationManagerImpl: Registering of Spatialization Enabled Change For Current Device Callback"
            "Failed");
    }

    std::shared_ptr<TaiheAudioCurrentSpatializationEnabledChangeCallback> cb =
        std::static_pointer_cast<TaiheAudioCurrentSpatializationEnabledChangeCallback>
        (taiheSpatializationManager->spatializationEnabledChangeForCurrentDeviceCallback_);
    cb->SaveCurrentSpatializationEnabledChangeCallbackReference(callback);

    AUDIO_INFO_LOG("Register spatialization enabled for current device callback is successful");
}

void AudioSpatializationManagerImpl::RegisterSpatializationEnabledChangeCallback(
    std::shared_ptr<uintptr_t> &callback, AudioSpatializationManagerImpl *taiheSpatializationManager)
{
    if (!taiheSpatializationManager->spatializationEnabledChangeCallback_) {
        taiheSpatializationManager->spatializationEnabledChangeCallback_ =
            std::make_shared<TaiheAudioSpatializationEnabledChangeCallback>(get_env());
        CHECK_AND_RETURN_LOG(taiheSpatializationManager->spatializationEnabledChangeCallback_ != nullptr,
            "AudioSpatializationManagerImpl: Memory Allocation Failed !!");

        int32_t ret = taiheSpatializationManager->audioSpatializationMngr_->
            RegisterSpatializationEnabledEventListener(
            taiheSpatializationManager->spatializationEnabledChangeCallback_);
        CHECK_AND_RETURN_LOG(ret == OHOS::AudioStandard::SUCCESS,
            "AudioSpatializationManagerImpl: Registering of Spatialization Enabled Change Callback Failed");
    }

    std::shared_ptr<TaiheAudioSpatializationEnabledChangeCallback> cb =
        std::static_pointer_cast<TaiheAudioSpatializationEnabledChangeCallback>
        (taiheSpatializationManager->spatializationEnabledChangeCallback_);
    cb->SaveSpatializationEnabledChangeCallbackReference(SPATIALIZATION_ENABLED_CHANGE_FOR_ANY_DEVICES_CALLBACK_NAME,
        callback);

    AUDIO_INFO_LOG("Register spatialization enabled callback is successful");
}

void AudioSpatializationManagerImpl::RegisterHeadTrackingEnabledChangeCallback(
    std::shared_ptr<uintptr_t> &callback, AudioSpatializationManagerImpl *taiheSpatializationManager)
{
    if (!taiheSpatializationManager->headTrackingEnabledChangeCallback_) {
        taiheSpatializationManager->headTrackingEnabledChangeCallback_ =
            std::make_shared<TaiheAudioHeadTrackingEnabledChangeCallback>(get_env());
        CHECK_AND_RETURN_LOG(taiheSpatializationManager->headTrackingEnabledChangeCallback_ != nullptr,
            "AudioSpatializationManagerImpl: Memory Allocation Failed !!");

        int32_t ret = taiheSpatializationManager->audioSpatializationMngr_->
            RegisterHeadTrackingEnabledEventListener(
            taiheSpatializationManager->headTrackingEnabledChangeCallback_);
        CHECK_AND_RETURN_LOG(ret == OHOS::AudioStandard::SUCCESS,
            "AudioSpatializationManagerImpl: Registering of Head Tracking Enabled Change Callback Failed");
    }

    std::shared_ptr<TaiheAudioHeadTrackingEnabledChangeCallback> cb =
        std::static_pointer_cast<TaiheAudioHeadTrackingEnabledChangeCallback>
        (taiheSpatializationManager->headTrackingEnabledChangeCallback_);
    cb->SaveHeadTrackingEnabledChangeCallbackReference(HEAD_TRACKING_ENABLED_CHANGE_FOR_ANY_DEVICES_CALLBACK_NAME,
        callback);

    AUDIO_INFO_LOG("Register head tracking enabled callback is successful");
}

void AudioSpatializationManagerImpl::UnregisterSpatializationEnabledChangeForCurrentDeviceCallback(
    std::shared_ptr<uintptr_t> &callback, AudioSpatializationManagerImpl *taiheSpatializationManager)
{
    if (taiheSpatializationManager->spatializationEnabledChangeForCurrentDeviceCallback_ != nullptr) {
        std::shared_ptr<TaiheAudioCurrentSpatializationEnabledChangeCallback> cb =
            std::static_pointer_cast<TaiheAudioCurrentSpatializationEnabledChangeCallback>(
            taiheSpatializationManager->spatializationEnabledChangeForCurrentDeviceCallback_);
        if (callback != nullptr) {
            cb->RemoveCurrentSpatializationEnabledChangeCallbackReference(callback);
        }
        if (callback == nullptr || cb->GetCurrentSpatializationEnabledChangeCbListSize() == 0) {
            int32_t ret = taiheSpatializationManager->audioSpatializationMngr_->
                UnregisterSpatializationEnabledForCurrentDeviceEventListener();
            CHECK_AND_RETURN_LOG(ret == OHOS::AudioStandard::SUCCESS,
                "UnregisterSpatializationEnabledForCurrentDeviceEventListener Failed");
            taiheSpatializationManager->spatializationEnabledChangeForCurrentDeviceCallback_.reset();
            taiheSpatializationManager->spatializationEnabledChangeForCurrentDeviceCallback_ = nullptr;
            cb->RemoveAllCurrentSpatializationEnabledChangeCallbackReference();
        }
    } else {
        AUDIO_ERR_LOG("UnregisterSpatializationEnabledChangeForCurrentDeviceCallback:"
            "spatializationEnabledChangeForCurrentDeviceCallback_ is null");
    }
}


void AudioSpatializationManagerImpl::UnregisterSpatializationEnabledChangeCallback(
    std::shared_ptr<uintptr_t> &callback, AudioSpatializationManagerImpl *taiheSpatializationManager)
{
    if (taiheSpatializationManager->spatializationEnabledChangeCallback_ != nullptr) {
        std::shared_ptr<TaiheAudioSpatializationEnabledChangeCallback> cb =
            std::static_pointer_cast<TaiheAudioSpatializationEnabledChangeCallback>(
            taiheSpatializationManager->spatializationEnabledChangeCallback_);
        if (callback != nullptr) {
            cb->RemoveSpatializationEnabledChangeCallbackReference(
                SPATIALIZATION_ENABLED_CHANGE_FOR_ANY_DEVICES_CALLBACK_NAME, callback);
        }
        if (callback == nullptr || cb->GetSpatializationEnabledChangeCbListSize(
            SPATIALIZATION_ENABLED_CHANGE_FOR_ANY_DEVICES_CALLBACK_NAME) == 0) {
            int32_t ret = taiheSpatializationManager->audioSpatializationMngr_->
                UnregisterSpatializationEnabledEventListener();
            CHECK_AND_RETURN_LOG(ret == OHOS::AudioStandard::SUCCESS,
                "UnregisterSpatializationEnabledEventListener Failed");
            taiheSpatializationManager->spatializationEnabledChangeCallback_.reset();
            taiheSpatializationManager->spatializationEnabledChangeCallback_ = nullptr;
            cb->RemoveAllSpatializationEnabledChangeCallbackReference(
                SPATIALIZATION_ENABLED_CHANGE_FOR_ANY_DEVICES_CALLBACK_NAME);
        }
    } else {
        AUDIO_ERR_LOG("UnregisterSpatializationEnabledChangeCb: spatializationEnabledChangeCallback_ is null");
    }
}

void AudioSpatializationManagerImpl::UnregisterHeadTrackingEnabledChangeCallback(
    std::shared_ptr<uintptr_t> &callback, AudioSpatializationManagerImpl *taiheSpatializationManager)
{
    if (taiheSpatializationManager->headTrackingEnabledChangeCallback_ != nullptr) {
        std::shared_ptr<TaiheAudioHeadTrackingEnabledChangeCallback> cb =
            std::static_pointer_cast<TaiheAudioHeadTrackingEnabledChangeCallback>(
            taiheSpatializationManager->headTrackingEnabledChangeCallback_);
        if (callback != nullptr) {
            cb->RemoveHeadTrackingEnabledChangeCallbackReference(
                HEAD_TRACKING_ENABLED_CHANGE_FOR_ANY_DEVICES_CALLBACK_NAME, callback);
        }
        if (callback == nullptr || cb->GetHeadTrackingEnabledChangeCbListSize(
            HEAD_TRACKING_ENABLED_CHANGE_FOR_ANY_DEVICES_CALLBACK_NAME) == 0) {
            int32_t ret = taiheSpatializationManager->audioSpatializationMngr_->
                UnregisterHeadTrackingEnabledEventListener();
            CHECK_AND_RETURN_LOG(ret == OHOS::AudioStandard::SUCCESS,
                "UnregisterHeadTrackingEnabledEventListener Failed");
            taiheSpatializationManager->headTrackingEnabledChangeCallback_.reset();
            taiheSpatializationManager->headTrackingEnabledChangeCallback_ = nullptr;
            cb->RemoveAllHeadTrackingEnabledChangeCallbackReference(
                HEAD_TRACKING_ENABLED_CHANGE_FOR_ANY_DEVICES_CALLBACK_NAME);
        }
    } else {
        AUDIO_ERR_LOG("UnregisterHeadTrackingEnabledChangeCb: headTrackingEnabledChangeCallback_ is null");
    }
}

} // namespace ANI::Audio
