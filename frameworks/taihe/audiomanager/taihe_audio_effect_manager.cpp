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
#define LOG_TAG "TaiheAudioEffectMgr"
#endif

#include "taihe_audio_effect_manager.h"
#include "taihe_audio_error.h"
#include "taihe_param_utils.h"
#include "taihe_audio_enum.h"
#include "taihe_audio_effect_manager_callback.h"
#include "audio_errors.h"

namespace ANI::Audio {
using namespace OHOS::HiviewDFX;

AudioEffectManagerImpl::AudioEffectManagerImpl() : audioEffectMngr_(nullptr) {}

AudioEffectManagerImpl::AudioEffectManagerImpl(OHOS::AudioStandard::AudioEffectManager *audioEffectMngr)
    : audioEffectMngr_(nullptr)
{
    cachedClientId_ = getpid();
    if (audioEffectMngr != nullptr) {
        audioEffectMngr_ = audioEffectMngr;
    }
}

AudioEffectManagerImpl::~AudioEffectManagerImpl() = default;

AudioEffectManager AudioEffectManagerImpl::CreateEffectManagerWrapper()
{
    auto *audioEffectMngr = OHOS::AudioStandard::AudioEffectManager::GetInstance();
    if (audioEffectMngr == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "Failed to get AudioEffectManager instance");
        return make_holder<AudioEffectManagerImpl, AudioEffectManager>(nullptr);
    }
    return make_holder<AudioEffectManagerImpl, AudioEffectManager>(audioEffectMngr);
}

array<AudioEffectProperty> AudioEffectManagerImpl::GetSupportedAudioEffectProperty()
{
    std::vector<AudioEffectProperty> emptyResult;
    if (audioEffectMngr_ == nullptr) {
        AUDIO_ERR_LOG("audioEffectMngr_ is nullptr");
        TaiheAudioError::ThrowError(TAIHE_ERR_SYSTEM, "incorrect parameter types: The type of options must be empty");
        return array<AudioEffectProperty>(emptyResult);
    }

    OHOS::AudioStandard::AudioEffectPropertyArray propertyArray = {};
    int32_t result = audioEffectMngr_->GetSupportedAudioEffectProperty(propertyArray);
    if (result != AUDIO_OK) {
        AUDIO_ERR_LOG("get audio enhance property failure! %{public}d", result);
        TaiheAudioError::ThrowError(result, "interface operation failed");
        return array<AudioEffectProperty>(emptyResult);
    }
    return TaiheParamUtils::ToTaiheEffectPropertyArray(propertyArray);
}

void AudioEffectManagerImpl::SetAudioEffectProperty(array_view<AudioEffectProperty> propertyArray)
{
    if (audioEffectMngr_ == nullptr) {
        AUDIO_ERR_LOG("audioEffectMngr_ is nullptr");
        TaiheAudioError::ThrowError(TAIHE_ERR_INPUT_INVALID,
            "parameter verification failed: mandatory parameters are left unspecified");
        return;
    }

    OHOS::AudioStandard::AudioEffectPropertyArray innerPropertyArray = {};
    int32_t result = TaiheParamUtils::GetEffectPropertyArray(innerPropertyArray, propertyArray);
    if (result != AUDIO_OK || innerPropertyArray.property.size() <= 0) {
        AUDIO_ERR_LOG("GetEffectPropertyArray failed or arguments error");
        TaiheAudioError::ThrowError(TAIHE_ERR_INVALID_PARAM,
            "parameter verification failed: mandatory parameters are left unspecified");
        return;
    }

    result = audioEffectMngr_->SetAudioEffectProperty(innerPropertyArray);
    if (result != AUDIO_OK) {
        AUDIO_ERR_LOG("set audio effect property failure! %{public}d", result);
        TaiheAudioError::ThrowError(result, "interface operation failed");
        return;
    }
}

array<AudioEffectProperty> AudioEffectManagerImpl::GetAudioEffectProperty()
{
    std::vector<AudioEffectProperty> emptyResult;
    if (audioEffectMngr_ == nullptr) {
        AUDIO_ERR_LOG("audioEffectMngr_ is nullptr");
        TaiheAudioError::ThrowError(TAIHE_ERR_SYSTEM, "incorrect parameter types: The type of options must be empty");
        return array<AudioEffectProperty>(emptyResult);
    }

    OHOS::AudioStandard::AudioEffectPropertyArray propertyArray = {};
    int32_t result = audioEffectMngr_->GetAudioEffectProperty(propertyArray);
    if (result != AUDIO_OK) {
        AUDIO_ERR_LOG("get audio enhance property failure! %{public}d", result);
        TaiheAudioError::ThrowError(TAIHE_ERR_SYSTEM, "interface operation failed");
        return array<AudioEffectProperty>(emptyResult);
    }
    return TaiheParamUtils::ToTaiheEffectPropertyArray(propertyArray);
}

bool AudioEffectManagerImpl::IsAudioSeparationEffectSupported()
{
    AUDIO_DEBUG_LOG("IsAudioSeparationEffectSupported");
    bool isSupported = false;
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return isSupported;
    }
    if (audioEffectMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioEffectMngr_ is nullptr");
        return isSupported;
    }
    isSupported = audioEffectMngr_->IsAudioSeparationEffectSupported();
    return isSupported;
}

void AudioEffectManagerImpl::SetAudioSeparationEffectEnabled(bool enabled, int32_t uid, optional<int64_t> streamId)
{
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return;
    }
    if (audioEffectMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioEffectMngr_ is nullptr");
        return;
    }
    int32_t result = AUDIO_OK;
    if (streamId.has_value()) {
        result = audioEffectMngr_->SetAudioSeparationEffectEnabled(enabled, uid, streamId.value());
    } else {
        result = audioEffectMngr_->SetAudioSeparationEffectEnabled(enabled, uid);
    }
    if (result == OHOS::AudioStandard::ERR_NOT_SUPPORTED) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NOT_SUPPORTED, "Effect is not supported in this device");
        return;
    } else if (result != AUDIO_OK) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "Audio system error occurs");
        return;
    }
}

void AudioEffectManagerImpl::OnAudioSeparationEffectEnabledChange(callback_view<void(bool)> callback)
{
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterAudioSeparationEffectEnabledChangeCallback(cacheCallback, this);
}

void AudioEffectManagerImpl::OffAudioSeparationEffectEnabledChange(optional_view<callback<void(bool)>> callback)
{
    std::shared_ptr<uintptr_t> cacheCallback = nullptr;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterAudioSeparationEffectEnabledChangeCallback(cacheCallback, this);
}

void AudioEffectManagerImpl::SetAudioSeparationEffectVolume(AudioSeparationVolumeType type, double volume)
{
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return;
    }
    if (volume < 0.0 || volume > 1.0) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
            "parameter verification failed: volume range is [0.0, 1.0]");
        return;
    }
    if (audioEffectMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioEffectMngr_ is nullptr");
        return;
    }
    OHOS::AudioStandard::AudioSeparationVolumeType volumeType =
        static_cast<OHOS::AudioStandard::AudioSeparationVolumeType>(type.get_value());
    int32_t result = audioEffectMngr_->SetAudioSeparationEffectVolume(volumeType, volume);
    if (result == OHOS::AudioStandard::ERR_NOT_SUPPORTED) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NOT_SUPPORTED, "Effect is not supported in this device");
        return;
    } else if (result != AUDIO_OK) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "Audio system error occurs");
        return;
    }
}

void AudioEffectManagerImpl::RegisterAudioSeparationEffectEnabledChangeCallback(
    std::shared_ptr<uintptr_t> &callback, AudioEffectManagerImpl *taiheEffectManager)
{
    if ((taiheEffectManager == nullptr) || (taiheEffectManager->audioEffectMngr_ == nullptr)) {
        AUDIO_ERR_LOG("Failed to retrieve audio effect manager taihe instance.");
        return;
    }
    std::lock_guard<std::mutex> lock(taiheEffectManager->mutex_);
    if (!taiheEffectManager->separationEffectEnabledChangeCallback_) {
        taiheEffectManager->separationEffectEnabledChangeCallback_ =
            std::make_shared<TaiheAudioSeparationEffectEnabledChangeCallback>();
        CHECK_AND_RETURN_LOG(taiheEffectManager->separationEffectEnabledChangeCallback_ != nullptr,
            "AudioEffectManagerImpl: Memory Allocation Failed !!");

        int32_t ret = taiheEffectManager->audioEffectMngr_->RegisterAudioSeparationEffectEnabledChangeEventListener(
            taiheEffectManager->separationEffectEnabledChangeCallback_);
        CHECK_AND_RETURN_LOG(ret == OHOS::AudioStandard::SUCCESS,
            "AudioEffectManagerImpl: Registering of Audio Separation Effect Enabled Change Callback Failed");
    }
    std::shared_ptr<TaiheAudioSeparationEffectEnabledChangeCallback> cb =
        std::static_pointer_cast<TaiheAudioSeparationEffectEnabledChangeCallback>(
            taiheEffectManager->separationEffectEnabledChangeCallback_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    cb->SaveAudioSeparationEffectEnabledChangeCallbackReference(callback);
    AUDIO_INFO_LOG("Register audio separation effect enabled change callback is successful");
}

void AudioEffectManagerImpl::UnregisterAudioSeparationEffectEnabledChangeCallback(
    std::shared_ptr<uintptr_t> &callback, AudioEffectManagerImpl *taiheEffectManager)
{
    CHECK_AND_RETURN_LOG(taiheEffectManager != nullptr, "Failed to retrieve taihe instance.");
    std::lock_guard<std::mutex> lock(taiheEffectManager->mutex_);
    CHECK_AND_RETURN_LOG(taiheEffectManager->audioEffectMngr_ != nullptr, "effect instance null.");
    if (taiheEffectManager->separationEffectEnabledChangeCallback_ != nullptr) {
        std::shared_ptr<TaiheAudioSeparationEffectEnabledChangeCallback> cb =
            std::static_pointer_cast<TaiheAudioSeparationEffectEnabledChangeCallback>(
                taiheEffectManager->separationEffectEnabledChangeCallback_);
        CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
        if (callback != nullptr) {
            cb->RemoveAudioSeparationEffectEnabledChangeCallbackReference(callback);
        }
        if (callback == nullptr || cb->GetAudioSeparationEffectEnabledChangeCbListSize() == 0) {
            int32_t ret = taiheEffectManager->audioEffectMngr_->UnregisterAudioSeparationEffectEnabledChangeEventListener();
            CHECK_AND_RETURN_LOG(ret == OHOS::AudioStandard::SUCCESS,
                "UnregisterAudioSeparationEffectEnabledChangeEventListener Failed");
            taiheEffectManager->separationEffectEnabledChangeCallback_.reset();
            taiheEffectManager->separationEffectEnabledChangeCallback_ = nullptr;
            cb->RemoveAllAudioSeparationEffectEnabledChangeCallbackReference();
        }
    } else {
        AUDIO_ERR_LOG("UnregisterAudioSeparationEffectEnabledChangeCb: separationEffectEnabledChangeCallback_ is null");
    }
}
} // namespace ANI::Audio