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

namespace ANI::Audio {
AudioSpatializationManagerImpl::AudioSpatializationManagerImpl() : audioSpatializationMngr_(nullptr) {}

AudioSpatializationManagerImpl::AudioSpatializationManagerImpl(std::shared_ptr<AudioSpatializationManagerImpl> obj)
    : audioSpatializationMngr_(nullptr)
{
    if (obj != nullptr) {
        audioSpatializationMngr_ = obj->audioSpatializationMngr_;
    }
}

AudioSpatializationManagerImpl::~AudioSpatializationManagerImpl() = default;

AudioSpatializationManager AudioSpatializationManagerImpl::CreateSpatializationManagerWrapper()
{
    std::shared_ptr<AudioSpatializationManagerImpl> audioSpatializationManagerImpl =
        std::make_shared<AudioSpatializationManagerImpl>();
    if (audioSpatializationManagerImpl != nullptr) {
        audioSpatializationManagerImpl->audioSpatializationMngr_ =
            OHOS::AudioStandard::AudioSpatializationManager::GetInstance();
        return make_holder<AudioSpatializationManagerImpl, AudioSpatializationManager>(audioSpatializationManagerImpl);
    }
    TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioSpatializationManagerImpl is nullptr");
    return make_holder<AudioSpatializationManagerImpl, AudioSpatializationManager>(nullptr);
}

bool AudioSpatializationManagerImpl::IsSpatializationSupported()
{
    AUDIO_DEBUG_LOG("in");
    bool isSpatializationSupported = false;
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return isSpatializationSupported;
    }
    if (audioSpatializationMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioSpatializationMngr_ is nullptr");
        return isSpatializationSupported;
    }
    isSpatializationSupported = audioSpatializationMngr_->IsSpatializationSupported();
    return isSpatializationSupported;
}

bool AudioSpatializationManagerImpl::IsSpatializationSupportedForDevice(AudioDeviceDescriptor deviceDescriptor)
{
    AUDIO_DEBUG_LOG("IsSpatializationSupportedForDevice");
    bool isSpatializationSupportedForDevice = false;
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return isSpatializationSupportedForDevice;
    }
    bool argTransFlag = true;
    std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor> selectedAudioDevice =
        std::make_shared<OHOS::AudioStandard::AudioDeviceDescriptor>();
    TaiheParamUtils::GetAudioDeviceDescriptor(selectedAudioDevice, argTransFlag, deviceDescriptor);
    if (argTransFlag != true) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERROR_INVALID_PARAM,
            "parameter verification failed: The param of deviceDescriptor must be interface AudioDeviceDescriptor");
        AUDIO_ERR_LOG("invalid parameter");
        return isSpatializationSupportedForDevice;
    }
    if (audioSpatializationMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioSpatializationMngr_ is nullptr");
        return isSpatializationSupportedForDevice;
    }
    isSpatializationSupportedForDevice = audioSpatializationMngr_->IsSpatializationSupportedForDevice(
        selectedAudioDevice);
    return isSpatializationSupportedForDevice;
}

bool AudioSpatializationManagerImpl::IsHeadTrackingSupported()
{
    AUDIO_DEBUG_LOG("in");
    bool isHeadTrackingSupported = false;
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return isHeadTrackingSupported;
    }
    if (audioSpatializationMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioSpatializationMngr_ is nullptr");
        return isHeadTrackingSupported;
    }
    isHeadTrackingSupported = audioSpatializationMngr_->IsHeadTrackingSupported();
    return isHeadTrackingSupported;
}

bool AudioSpatializationManagerImpl::IsHeadTrackingSupportedForDevice(AudioDeviceDescriptor deviceDescriptor)
{
    bool isHeadTrackingSupportedForDevice = false;
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return isHeadTrackingSupportedForDevice;
    }
    bool argTransFlag = true;
    std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor> selectedAudioDevice =
        std::make_shared<OHOS::AudioStandard::AudioDeviceDescriptor>();
    TaiheParamUtils::GetAudioDeviceDescriptor(selectedAudioDevice, argTransFlag, deviceDescriptor);
    if (argTransFlag != true) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERROR_INVALID_PARAM,
            "parameter verification failed: The param of deviceDescriptor must be interface AudioDeviceDescriptor");
        AUDIO_ERR_LOG("invalid parameter");
        return isHeadTrackingSupportedForDevice;
    }
    if (audioSpatializationMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioSpatializationMngr_ is nullptr");
        return isHeadTrackingSupportedForDevice;
    }
    isHeadTrackingSupportedForDevice = audioSpatializationMngr_->IsHeadTrackingSupportedForDevice(
        selectedAudioDevice);
    return isHeadTrackingSupportedForDevice;
}

void AudioSpatializationManagerImpl::SetSpatializationEnabledSync(AudioDeviceDescriptor deviceDescriptor, bool enabled)
{
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return;
    }
    bool spatializationEnable = enabled;
    std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor> selectedAudioDevice =
        std::make_shared<OHOS::AudioStandard::AudioDeviceDescriptor>();
    bool argTransFlag = true;
    int32_t status = TaiheParamUtils::GetAudioDeviceDescriptor(selectedAudioDevice, argTransFlag, deviceDescriptor);
    if (status != AUDIO_OK) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INPUT_INVALID,
            "incorrect parameter types: The param of deviceDescriptor must be interface AudioDeviceDescriptor");
        return;
    }
    if (audioSpatializationMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioSpatializationMngr_ is nullptr");
        return;
    }
    int32_t intValue = audioSpatializationMngr_->SetSpatializationEnabled(selectedAudioDevice, spatializationEnable);
    if (intValue == OHOS::AudioStandard::ERR_PERMISSION_DENIED) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_PERMISSION);
        return;
    } else if (intValue != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM);
        return;
    }
    return;
}

bool AudioSpatializationManagerImpl::IsSpatializationEnabled(AudioDeviceDescriptor deviceDescriptor)
{
    AUDIO_INFO_LOG("in");
    bool isSpatializationEnabled = false;
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return isSpatializationEnabled;
    }
    bool argTransFlag = true;
    std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor> selectedAudioDevice =
        std::make_shared<OHOS::AudioStandard::AudioDeviceDescriptor>();
    TaiheParamUtils::GetAudioDeviceDescriptor(selectedAudioDevice, argTransFlag, deviceDescriptor);
    if (argTransFlag != true) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
            "parameter verification failed: The param of deviceDescriptor must be interface AudioDeviceDescriptor");
        AUDIO_ERR_LOG("invalid parameter");
        return isSpatializationEnabled;
    }
    if (audioSpatializationMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioSpatializationMngr_ is nullptr");
        return isSpatializationEnabled;
    }
    isSpatializationEnabled = audioSpatializationMngr_->IsSpatializationEnabled(selectedAudioDevice);
    return isSpatializationEnabled;
}

bool AudioSpatializationManagerImpl::IsSpatializationEnabledForCurrentDevice()
{
    AUDIO_INFO_LOG("IsSpatializationEnabledForCurrentDevice in");
    bool isSpatializationEnabledForCurrentDevice = false;
    if (audioSpatializationMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioSpatializationMngr_ is nullptr");
        return isSpatializationEnabledForCurrentDevice;
    }
    isSpatializationEnabledForCurrentDevice = audioSpatializationMngr_->IsSpatializationEnabledForCurrentDevice();
    return isSpatializationEnabledForCurrentDevice;
}

void AudioSpatializationManagerImpl::SetHeadTrackingEnabledSync(AudioDeviceDescriptor deviceDescriptor, bool enabled)
{
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return;
    }
    bool headTrackingEnable = enabled;
    std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor> selectedAudioDevice =
        std::make_shared<OHOS::AudioStandard::AudioDeviceDescriptor>();
    bool argTransFlag = true;
    int32_t status = TaiheParamUtils::GetAudioDeviceDescriptor(selectedAudioDevice, argTransFlag, deviceDescriptor);
    if (status != AUDIO_OK) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INPUT_INVALID,
            "incorrect parameter types: The param of deviceDescriptor must be interface AudioDeviceDescriptor");
        return;
    }
    if (audioSpatializationMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioSpatializationMngr_ is nullptr");
        return;
    }
    int32_t intValue = audioSpatializationMngr_->SetHeadTrackingEnabled(selectedAudioDevice, headTrackingEnable);
    if (intValue == OHOS::AudioStandard::ERR_PERMISSION_DENIED) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_PERMISSION);
        return;
    } else if (intValue != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM);
        return;
    }
    return;
}

bool AudioSpatializationManagerImpl::IsHeadTrackingEnabled(AudioDeviceDescriptor deviceDescriptor)
{
    AUDIO_INFO_LOG("in");
    bool isHeadTrackingEnabled = false;
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return isHeadTrackingEnabled;
    }
    bool argTransFlag = true;
    std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor> selectedAudioDevice =
        std::make_shared<OHOS::AudioStandard::AudioDeviceDescriptor>();
    TaiheParamUtils::GetAudioDeviceDescriptor(selectedAudioDevice, argTransFlag, deviceDescriptor);
    if (argTransFlag != true) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
            "parameter verification failed: The param of deviceDescriptor must be interface AudioDeviceDescriptor");
        AUDIO_ERR_LOG("invalid parameter");
        return isHeadTrackingEnabled;
    }
    if (audioSpatializationMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioSpatializationMngr_ is nullptr");
        return isHeadTrackingEnabled;
    }
    isHeadTrackingEnabled = audioSpatializationMngr_->IsHeadTrackingEnabled(selectedAudioDevice);
    return isHeadTrackingEnabled;
}

void AudioSpatializationManagerImpl::UpdateSpatialDeviceState(AudioSpatialDeviceState spatialDeviceState)
{
    AUDIO_INFO_LOG("UpdateSpatialDeviceState");
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return;
    }
    OHOS::AudioStandard::AudioSpatialDeviceState audioSpatialDeviceState;
    if (TaiheParamUtils::GetSpatialDeviceState(&audioSpatialDeviceState, spatialDeviceState) != AUDIO_OK) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERROR_INVALID_PARAM,
            "parameter verification failed: The param of spatialDeviceState must be interface AudioSpatialDeviceState");
        return;
    }
    if (audioSpatializationMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioSpatializationMngr_ is nullptr");
        return;
    }
    int32_t ret = audioSpatializationMngr_->UpdateSpatialDeviceState(audioSpatialDeviceState);
    if (ret == OHOS::AudioStandard::ERR_PERMISSION_DENIED) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_PERMISSION);
        return;
    }
}

void AudioSpatializationManagerImpl::SetSpatializationSceneType(AudioSpatializationSceneType spatializationSceneType)
{
    AUDIO_INFO_LOG("Start to set spatialization rendering scene type");
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return;
    }
    int32_t sceneType = spatializationSceneType.get_value();
    if (!TaiheAudioEnum::IsLegalInputArgumentSpatializationSceneType(sceneType)) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERROR_INVALID_PARAM,
            "parameter verification failed: The param of spatializationSceneType must be \
                enum AudioSpatializationSceneType");
        AUDIO_ERR_LOG("get sceneType failed");
        return;
    }
    if (audioSpatializationMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioSpatializationMngr_ is nullptr");
        return;
    }
    int32_t ret = audioSpatializationMngr_->SetSpatializationSceneType(
        static_cast<OHOS::AudioStandard::AudioSpatializationSceneType>(sceneType));
    if (ret == OHOS::AudioStandard::ERR_PERMISSION_DENIED) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_PERMISSION, "No system permission");
        return;
    }
    return;
}

AudioSpatializationSceneType AudioSpatializationManagerImpl::GetSpatializationSceneType()
{
    AUDIO_INFO_LOG("Start to get current spatialization rendering scene type");
    OHOS::AudioStandard::AudioSpatializationSceneType sceneType =
        OHOS::AudioStandard::AudioSpatializationSceneType::SPATIALIZATION_SCENE_TYPE_DEFAULT;
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return TaiheAudioEnum::ToTaiheAudioSpatializationSceneType(sceneType);
    }
    if (audioSpatializationMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioSpatializationMngr_ is nullptr");
        return TaiheAudioEnum::ToTaiheAudioSpatializationSceneType(sceneType);
    }
    sceneType = audioSpatializationMngr_->GetSpatializationSceneType();
    return TaiheAudioEnum::ToTaiheAudioSpatializationSceneType(sceneType);
}

void AudioSpatializationManagerImpl::OnSpatializationEnabledChangeForCurrentDevice(callback_view<void(bool)> callback)
{
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterSpatializationEnabledChangeForCurrentDeviceCallback(cacheCallback, this);
}

void AudioSpatializationManagerImpl::OnSpatializationEnabledChangeForAnyDevice(
    callback_view<void(AudioSpatialEnabledStateForDevice const&)> callback)
{
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    CHECK_AND_RETURN_RET_LOG(OHOS::AudioStandard::PermissionUtil::VerifySelfPermission(),
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED), "No system permission");
    RegisterSpatializationEnabledChangeCallback(cacheCallback, this);
}

void AudioSpatializationManagerImpl::OnHeadTrackingEnabledChangeForAnyDevice(
    callback_view<void(AudioSpatialEnabledStateForDevice const&)> callback)
{
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    CHECK_AND_RETURN_RET_LOG(OHOS::AudioStandard::PermissionUtil::VerifySelfPermission(),
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED), "No system permission");
    RegisterHeadTrackingEnabledChangeCallback(cacheCallback, this);
}

void AudioSpatializationManagerImpl::OffSpatializationEnabledChangeForCurrentDevice(
    optional_view<callback<void(bool)>> callback)
{
    std::shared_ptr<uintptr_t> cacheCallback = nullptr;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterSpatializationEnabledChangeForCurrentDeviceCallback(cacheCallback, this);
}

void AudioSpatializationManagerImpl::OffSpatializationEnabledChangeForAnyDevice(
    optional_view<callback<void(AudioSpatialEnabledStateForDevice const&)>> callback)
{
    std::shared_ptr<uintptr_t> cacheCallback = nullptr;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    CHECK_AND_RETURN_RET_LOG(OHOS::AudioStandard::PermissionUtil::VerifySelfPermission(),
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED), "No system permission");
    UnregisterSpatializationEnabledChangeCallback(cacheCallback, this);
}

void AudioSpatializationManagerImpl::OffHeadTrackingEnabledChangeForAnyDevice(
    optional_view<callback<void(AudioSpatialEnabledStateForDevice const&)>> callback)
{
    std::shared_ptr<uintptr_t> cacheCallback = nullptr;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    CHECK_AND_RETURN_RET_LOG(OHOS::AudioStandard::PermissionUtil::VerifySelfPermission(),
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED), "No system permission");
    UnregisterHeadTrackingEnabledChangeCallback(cacheCallback, this);
}

void AudioSpatializationManagerImpl::RegisterSpatializationEnabledChangeForCurrentDeviceCallback(
    std::shared_ptr<uintptr_t> &callback, AudioSpatializationManagerImpl *taiheSpatializationManager)
{
    if ((taiheSpatializationManager == nullptr) ||
        (taiheSpatializationManager->audioSpatializationMngr_ == nullptr)) {
        AUDIO_ERR_LOG("Failed to retrieve audio spatialization manager taihe instance.");
        return;
    }
    std::lock_guard<std::mutex> lock(taiheSpatializationManager->mutex_);
    if (!taiheSpatializationManager->spatializationEnabledChangeForCurrentDeviceCallback_) {
        taiheSpatializationManager->spatializationEnabledChangeForCurrentDeviceCallback_ =
            std::make_shared<TaiheAudioCurrentSpatializationEnabledChangeCallback>();
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
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    cb->SaveCurrentSpatializationEnabledChangeCallbackReference(callback);

    AUDIO_INFO_LOG("Register spatialization enabled for current device callback is successful");
}

void AudioSpatializationManagerImpl::RegisterSpatializationEnabledChangeCallback(
    std::shared_ptr<uintptr_t> &callback, AudioSpatializationManagerImpl *taiheSpatializationManager)
{
    if ((taiheSpatializationManager == nullptr) ||
        (taiheSpatializationManager->audioSpatializationMngr_ == nullptr)) {
        AUDIO_ERR_LOG("Failed to retrieve audio spatialization manager taihe instance.");
        return;
    }
    std::lock_guard<std::mutex> lock(taiheSpatializationManager->mutex_);
    if (!taiheSpatializationManager->spatializationEnabledChangeCallback_) {
        taiheSpatializationManager->spatializationEnabledChangeCallback_ =
            std::make_shared<TaiheAudioSpatializationEnabledChangeCallback>();
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
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    cb->SaveSpatializationEnabledChangeCallbackReference(SPATIALIZATION_ENABLED_CHANGE_FOR_ANY_DEVICES_CALLBACK_NAME,
        callback);
    AUDIO_INFO_LOG("Register spatialization enabled callback is successful");
}

void AudioSpatializationManagerImpl::RegisterHeadTrackingEnabledChangeCallback(
    std::shared_ptr<uintptr_t> &callback, AudioSpatializationManagerImpl *taiheSpatializationManager)
{
    if ((taiheSpatializationManager == nullptr) ||
        (taiheSpatializationManager->audioSpatializationMngr_ == nullptr)) {
        AUDIO_ERR_LOG("Failed to retrieve audio spatialization manager taihe instance.");
        return;
    }
    std::lock_guard<std::mutex> lock(taiheSpatializationManager->mutex_);
    if (!taiheSpatializationManager->headTrackingEnabledChangeCallback_) {
        taiheSpatializationManager->headTrackingEnabledChangeCallback_ =
            std::make_shared<TaiheAudioHeadTrackingEnabledChangeCallback>();
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
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    cb->SaveHeadTrackingEnabledChangeCallbackReference(HEAD_TRACKING_ENABLED_CHANGE_FOR_ANY_DEVICES_CALLBACK_NAME,
        callback);
    AUDIO_INFO_LOG("Register head tracking enabled callback is successful");
}

void AudioSpatializationManagerImpl::UnregisterSpatializationEnabledChangeForCurrentDeviceCallback(
    std::shared_ptr<uintptr_t> &callback, AudioSpatializationManagerImpl *taiheSpatializationManager)
{
    CHECK_AND_RETURN_LOG(taiheSpatializationManager != nullptr, "Failed to retrieve taihe instance.");
    std::lock_guard<std::mutex> lock(taiheSpatializationManager->mutex_);
    CHECK_AND_RETURN_LOG(taiheSpatializationManager->audioSpatializationMngr_ != nullptr,
        "spatialization instance null.");
    if (taiheSpatializationManager->spatializationEnabledChangeForCurrentDeviceCallback_ != nullptr) {
        std::shared_ptr<TaiheAudioCurrentSpatializationEnabledChangeCallback> cb =
            std::static_pointer_cast<TaiheAudioCurrentSpatializationEnabledChangeCallback>(
            taiheSpatializationManager->spatializationEnabledChangeForCurrentDeviceCallback_);
        CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
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
    CHECK_AND_RETURN_LOG(taiheSpatializationManager != nullptr, "Failed to retrieve taihe instance.");
    std::lock_guard<std::mutex> lock(taiheSpatializationManager->mutex_);
    CHECK_AND_RETURN_LOG(taiheSpatializationManager->audioSpatializationMngr_ != nullptr,
        "spatialization instance null.");
    if (taiheSpatializationManager->spatializationEnabledChangeCallback_ != nullptr) {
        std::shared_ptr<TaiheAudioSpatializationEnabledChangeCallback> cb =
            std::static_pointer_cast<TaiheAudioSpatializationEnabledChangeCallback>(
            taiheSpatializationManager->spatializationEnabledChangeCallback_);
        CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
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
    CHECK_AND_RETURN_LOG(taiheSpatializationManager != nullptr, "Failed to retrieve taihe instance.");
    std::lock_guard<std::mutex> lock(taiheSpatializationManager->mutex_);
    CHECK_AND_RETURN_LOG(taiheSpatializationManager->audioSpatializationMngr_ != nullptr,
        "spatialization instance null.");
    if (taiheSpatializationManager->headTrackingEnabledChangeCallback_ != nullptr) {
        std::shared_ptr<TaiheAudioHeadTrackingEnabledChangeCallback> cb =
            std::static_pointer_cast<TaiheAudioHeadTrackingEnabledChangeCallback>(
            taiheSpatializationManager->headTrackingEnabledChangeCallback_);
        CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
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
