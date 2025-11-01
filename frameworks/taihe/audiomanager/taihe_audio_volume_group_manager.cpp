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
#define LOG_TAG "AudioVolumeGroupManagerImpl"
#endif

#include "taihe_audio_volume_group_manager.h"

#ifdef FEATURE_HIVIEW_ENABLE
#if !defined(ANDROID_PLATFORM) && !defined(IOS_PLATFORM)
#include "xpower_event_js.h"
#endif
#endif
#include "taihe_audio_error.h"
#include "taihe_param_utils.h"
#include "taihe_audio_enum.h"
#include "taihe_audio_ringermode_callback.h"
#include "taihe_audio_micstatechange_callback.h"
#include "audio_errors.h"
#include "audio_manager_log.h"

namespace ANI::Audio {
constexpr double VOLUME_DEFAULT_DOUBLE = 0.0;
constexpr double INPUT_MAX_DEFAULT_DOUBLE = 0.0;
constexpr double OUTPUT_MAX_DEFAULT_DOUBLE = 0.0;

AudioVolumeGroupManagerImpl::AudioVolumeGroupManagerImpl()
{
}

AudioVolumeGroupManagerImpl::AudioVolumeGroupManagerImpl(std::shared_ptr<AudioVolumeGroupManagerImpl> obj)
{
    if (obj != nullptr) {
        audioGroupMngr_ = obj->audioGroupMngr_;
        cachedClientId_ = obj->cachedClientId_;
    }
}

AudioVolumeGroupManager AudioVolumeGroupManagerImpl::CreateAudioVolumeGroupManagerWrapper(int32_t groupId)
{
    auto groupManager = OHOS::AudioStandard::AudioSystemManager::GetInstance()->GetGroupManager(groupId);
    if (groupManager == nullptr) {
        AUDIO_ERR_LOG("Failed to get group manager!");
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERROR_INVALID_PARAM,
            "GetVolumeGroupManagerSync failed: invalid param");
        return make_holder<AudioVolumeGroupManagerImpl, AudioVolumeGroupManager>(nullptr);
    }
    std::shared_ptr<AudioVolumeGroupManagerImpl> audioVolumeGroupManagerImpl =
        std::make_shared<AudioVolumeGroupManagerImpl>();
    audioVolumeGroupManagerImpl->audioGroupMngr_ = groupManager;
    audioVolumeGroupManagerImpl->cachedClientId_ = getpid();
    return make_holder<AudioVolumeGroupManagerImpl, AudioVolumeGroupManager>(audioVolumeGroupManagerImpl);
}

void AudioVolumeGroupManagerImpl::SetVolumeSync(AudioVolumeType volumeType, int32_t volume)
{
    int32_t volType = volumeType.get_value();
    int32_t volLevel = volume;
    if (!TaiheAudioEnum::IsLegalInputArgumentVolType(volType)) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_UNSUPPORTED,
            "parameter verification failed: The param of volumeType must be enum AudioVolumeType");
        return;
    }
    if (audioGroupMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioGroupMngr_ is nullptr");
        return;
    }
    int32_t ret = audioGroupMngr_->SetVolume(TaiheAudioEnum::GetNativeAudioVolumeType(volType), volLevel);
    CHECK_AND_RETURN_RET(ret == OHOS::AudioStandard::SUCCESS, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_SYSTEM, "setvolume failed"));
    return;
}

void AudioVolumeGroupManagerImpl::SetVolumeWithFlagSync(AudioVolumeType volumeType, int32_t volume, int32_t flags)
{
    int32_t volType = volumeType.get_value();
    if (!TaiheAudioEnum::IsLegalInputArgumentVolType(volType)) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM);
        return;
    }
    int32_t volLevel = volume;
    int32_t volFlag = flags;
    if (audioGroupMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioGroupMngr_ is nullptr");
        return;
    }
    int32_t intValue = audioGroupMngr_->SetVolume(TaiheAudioEnum::GetNativeAudioVolumeType(volType), volLevel, volFlag);
    if (intValue != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "setvolumeWithFlag failed");
        return;
    }
    return;
}

AudioVolumeType AudioVolumeGroupManagerImpl::GetActiveVolumeTypeSync(int32_t uid)
{
    int32_t clientUid = uid;
    OHOS::AudioStandard::AudioStreamType volType = OHOS::AudioStandard::AudioStreamType::STREAM_VOICE_CALL;
    if (audioGroupMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioGroupMngr_ is nullptr");
        return TaiheAudioEnum::GetJsAudioVolumeType(volType);
    }
    volType = audioGroupMngr_->GetActiveVolumeType(clientUid);
    return TaiheAudioEnum::GetJsAudioVolumeType(volType);
}

int32_t AudioVolumeGroupManagerImpl::GetVolumeSync(AudioVolumeType volumeType)
{
    int32_t volType = volumeType.get_value();
    int32_t volLevel = 0;
    if (!TaiheAudioEnum::IsLegalInputArgumentVolType(volType)) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
            "parameter verification failed: The param of volumeType must be enum AudioVolumeType");
        return volLevel;
    }
    if (audioGroupMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioGroupMngr_ is nullptr");
        return volLevel;
    }
    volLevel = audioGroupMngr_->GetVolume(TaiheAudioEnum::GetNativeAudioVolumeType(volType));
    return volLevel;
}

int32_t AudioVolumeGroupManagerImpl::GetMinVolumeSync(AudioVolumeType volumeType)
{
    int32_t volType = volumeType.get_value();
    int32_t volLevel = 0;
    if (!TaiheAudioEnum::IsLegalInputArgumentVolType(volType)) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
            "parameter verification failed: The param of volumeType must be enum AudioVolumeType");
        return volLevel;
    }
    if (audioGroupMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioGroupMngr_ is nullptr");
        return volLevel;
    }
    volLevel = audioGroupMngr_->GetMinVolume(TaiheAudioEnum::GetNativeAudioVolumeType(volType));
    return volLevel;
}

int32_t AudioVolumeGroupManagerImpl::GetMaxVolumeSync(AudioVolumeType volumeType)
{
    int32_t volType = volumeType.get_value();
    int32_t volLevel = 0;
    if (!TaiheAudioEnum::IsLegalInputArgumentVolType(volType)) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
            "parameter verification failed: The param of volumeType must be enum AudioVolumeType");
        return volLevel;
    }
    if (audioGroupMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioGroupMngr_ is nullptr");
        return volLevel;
    }
    volLevel = audioGroupMngr_->GetMaxVolume(TaiheAudioEnum::GetNativeAudioVolumeType(volType));
    return volLevel;
}

void AudioVolumeGroupManagerImpl::MuteSync(AudioVolumeType volumeType, bool mute)
{
    int32_t volType = volumeType.get_value();
    if (!TaiheAudioEnum::IsLegalInputArgumentVolType(volType)) {
        TaiheAudioError::ThrowError(TAIHE_ERR_UNSUPPORTED);
        return;
    }
    if (audioGroupMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioGroupMngr_ is nullptr");
        return;
    }
    int32_t ret = audioGroupMngr_->SetMute(TaiheAudioEnum::GetNativeAudioVolumeType(volType), mute);
    CHECK_AND_RETURN_RET(ret == OHOS::AudioStandard::SUCCESS, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_SYSTEM, "setmute failed"));
    return;
}

bool AudioVolumeGroupManagerImpl::IsMuteSync(AudioVolumeType volumeType)
{
    bool isMute = false;
    int32_t volType = volumeType.get_value();
    if (!TaiheAudioEnum::IsLegalInputArgumentVolType(volType)) {
        AUDIO_ERR_LOG("Invalid volumeType: %{public}d", volType);
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERROR_INVALID_PARAM,
            "parameter verification failed: The param of volumeType must be enum AudioVolumeType");
        return false;
    }
    if (audioGroupMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioGroupMngr_ is nullptr");
        return false;
    }
    int32_t ret = audioGroupMngr_->IsStreamMute(TaiheAudioEnum::GetNativeAudioVolumeType(volType), isMute);
    CHECK_AND_RETURN_RET_LOG(ret == OHOS::AudioStandard::SUCCESS, false, "IsStreamMute failure!");
    return isMute;
}

void AudioVolumeGroupManagerImpl::SetRingerModeSync(AudioRingMode mode)
{
    int32_t ringMode = mode.get_value();
    if (!TaiheAudioEnum::IsLegalInputArgumentRingMode(ringMode)) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_UNSUPPORTED);
        return;
    }
    if (audioGroupMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioGroupMngr_ is nullptr");
        return;
    }
    int32_t ret = audioGroupMngr_->SetRingerMode(TaiheAudioEnum::GetNativeAudioRingerMode(ringMode));
    CHECK_AND_RETURN_RET(ret == OHOS::AudioStandard::SUCCESS, TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM,
        "setringermode failed"));
    return;
}

AudioRingMode AudioVolumeGroupManagerImpl::GetRingerModeSync()
{
    OHOS::AudioStandard::AudioRingerMode ringerMode = OHOS::AudioStandard::AudioRingerMode::RINGER_MODE_NORMAL;
    if (audioGroupMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioGroupMngr_ is nullptr");
        return TaiheAudioEnum::ToTaiheAudioRingMode(ringerMode);
    }
    ringerMode = audioGroupMngr_->GetRingerMode();
    return TaiheAudioEnum::ToTaiheAudioRingMode(ringerMode);
}

void AudioVolumeGroupManagerImpl::SetMicMuteSync(bool mute)
{
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return;
    }
    if (audioGroupMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioGroupMngr_ is nullptr");
        return;
    }
    int32_t ret = audioGroupMngr_->SetMicrophoneMute(mute);
    if (ret != OHOS::AudioStandard::SUCCESS) {
        if (ret == OHOS::AudioStandard::ERR_PERMISSION_DENIED) {
            TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_PERMISSION);
            return;
        } else {
            TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM);
            return;
        }
    }
    return;
}

void AudioVolumeGroupManagerImpl::SetMicMutePersistentSync(bool mute, PolicyType type)
{
    int32_t policyType = type.get_value();
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return;
    }
    if (audioGroupMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioGroupMngr_ is nullptr");
        return;
    }
    int32_t ret = audioGroupMngr_->SetMicrophoneMutePersistent(
        mute, static_cast<OHOS::AudioStandard::PolicyType>(policyType));
    if (ret != OHOS::AudioStandard::SUCCESS) {
        if (ret == OHOS::AudioStandard::ERR_PERMISSION_DENIED) {
            TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_PERMISSION);
            return;
        } else {
            TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM);
            return;
        }
    }
    return;
}

bool AudioVolumeGroupManagerImpl::IsPersistentMicMute()
{
    bool isPersistentMicMute = false;
    if (audioGroupMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioGroupMngr_ is nullptr");
        return isPersistentMicMute;
    }
    isPersistentMicMute = audioGroupMngr_->GetPersistentMicMuteState();
    return isPersistentMicMute;
}

bool AudioVolumeGroupManagerImpl::IsMicrophoneMuteSync()
{
    if (audioGroupMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioGroupMngr_ is nullptr");
        return false;
    }
    return audioGroupMngr_->IsMicrophoneMute();
}

void AudioVolumeGroupManagerImpl::AdjustVolumeByStepSync(VolumeAdjustType adjustType)
{
    int32_t adjustTypeInt32 = adjustType.get_value();
    if (!TaiheAudioEnum::IsLegalInputArgumentVolumeAdjustType(adjustType)) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERROR_INVALID_PARAM,
            "parameter verification failed: The param of adjustType must be enum VolumeAdjustType");
        return;
    }
    if (audioGroupMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioGroupMngr_ is nullptr");
        return;
    }
    int32_t ret = audioGroupMngr_->
    AdjustVolumeByStep(static_cast<OHOS::AudioStandard::VolumeAdjustType>(adjustTypeInt32));
    if (ret != OHOS::AudioStandard::SUCCESS) {
        if (ret == OHOS::AudioStandard::ERR_PERMISSION_DENIED) {
            TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_PERMISSION);
            return;
        } else {
            TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "System error. Set app volume fail.");
            return;
        }
    }
    return;
}

void AudioVolumeGroupManagerImpl::OnRingerModeChange(callback_view<void(AudioRingMode)> callback)
{
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterRingModeCallback(cacheCallback, RINGERMODE_CALLBACK_NAME, this);
}

void AudioVolumeGroupManagerImpl::RegisterRingModeCallback(std::shared_ptr<uintptr_t> &callback,
    const std::string &cbName, AudioVolumeGroupManagerImpl *audioVolumeGroupManagerImpl)
{
    CHECK_AND_RETURN_RET_LOG(audioVolumeGroupManagerImpl != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "audioVolumeGroupManagerImpl is nullptr");
    std::lock_guard<std::mutex> lock(audioVolumeGroupManagerImpl->mutex_);
    CHECK_AND_RETURN_RET_LOG(audioVolumeGroupManagerImpl->audioGroupMngr_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "audioGroupMngr_ is nullptr");
    if (audioVolumeGroupManagerImpl->ringerModecallbackTaihe_ == nullptr) {
        audioVolumeGroupManagerImpl->ringerModecallbackTaihe_ = std::make_shared<TaiheAudioRingerModeCallback>();
        int32_t ret = audioVolumeGroupManagerImpl->audioGroupMngr_->SetRingerModeCallback(
            audioVolumeGroupManagerImpl->cachedClientId_, audioVolumeGroupManagerImpl->ringerModecallbackTaihe_);
        CHECK_AND_RETURN_LOG(ret == OHOS::AudioStandard::SUCCESS, "SetRingerModeCallback Failed");
    }
    std::shared_ptr<TaiheAudioRingerModeCallback> cb =
        std::static_pointer_cast<TaiheAudioRingerModeCallback>(audioVolumeGroupManagerImpl->ringerModecallbackTaihe_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    cb->SaveCallbackReference(cbName, callback);
}

void AudioVolumeGroupManagerImpl::OffRingerModeChange(optional_view<callback<void(AudioRingMode)>> callback)
{
    std::shared_ptr<uintptr_t> cacheCallback = nullptr;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterRingerModeCallback(cacheCallback, this);
}

void AudioVolumeGroupManagerImpl::UnregisterRingerModeCallback(std::shared_ptr<uintptr_t> &callback,
    AudioVolumeGroupManagerImpl *audioVolumeGroupManagerImpl)
{
    CHECK_AND_RETURN_RET_LOG(audioVolumeGroupManagerImpl != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "audioVolumeGroupManagerImpl is nullptr");
    std::lock_guard<std::mutex> lock(audioVolumeGroupManagerImpl->mutex_);
    CHECK_AND_RETURN_RET_LOG(audioVolumeGroupManagerImpl->audioGroupMngr_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "audioGroupMngr_ is nullptr");
    CHECK_AND_RETURN_LOG(audioVolumeGroupManagerImpl->ringerModecallbackTaihe_ != nullptr,
        "ringerModecallbackTaihe is null");
    std::shared_ptr<TaiheAudioRingerModeCallback> cb = std::static_pointer_cast<TaiheAudioRingerModeCallback>(
        audioVolumeGroupManagerImpl->ringerModecallbackTaihe_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    if (callback != nullptr) {
        CHECK_AND_RETURN_LOG(cb->IsSameCallback(callback),
            "The callback need to be unregistered is not the same as the registered callback");
    }
    int32_t ret = audioVolumeGroupManagerImpl->audioGroupMngr_->UnsetRingerModeCallback(
        audioVolumeGroupManagerImpl->cachedClientId_, cb);
    CHECK_AND_RETURN_LOG(ret == OHOS::AudioStandard::SUCCESS, "UnsetRingerModeCallback failed");
    cb->RemoveCallbackReference(callback);
    audioVolumeGroupManagerImpl->ringerModecallbackTaihe_ = nullptr;
    AUDIO_INFO_LOG("UnregisterRingerModeCallback success");
}

void AudioVolumeGroupManagerImpl::OnMicStateChange(callback_view<void(MicStateChangeEvent const&)> callback)
{
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterMicStateChangeCallback(cacheCallback, MIC_STATE_CHANGE_CALLBACK_NAME, this);
}

void AudioVolumeGroupManagerImpl::RegisterMicStateChangeCallback(std::shared_ptr<uintptr_t> &callback,
    const std::string &cbName, AudioVolumeGroupManagerImpl *audioVolumeGroupManagerImpl)
{
    CHECK_AND_RETURN_RET_LOG(audioVolumeGroupManagerImpl != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "audioVolumeGroupManagerImpl is nullptr");
    std::lock_guard<std::mutex> lock(audioVolumeGroupManagerImpl->mutex_);
    CHECK_AND_RETURN_RET_LOG(audioVolumeGroupManagerImpl->audioGroupMngr_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "audioGroupMngr_ is nullptr");
    if (!audioVolumeGroupManagerImpl->micStateChangeCallbackTaihe_) {
        audioVolumeGroupManagerImpl->micStateChangeCallbackTaihe_ =
            std::make_shared<TaiheAudioManagerMicStateChangeCallback>();
        if (!audioVolumeGroupManagerImpl->micStateChangeCallbackTaihe_) {
            AUDIO_ERR_LOG("Memory Allocation Failed !!");
        }
        int32_t ret = audioVolumeGroupManagerImpl->audioGroupMngr_->SetMicStateChangeCallback(
            audioVolumeGroupManagerImpl->micStateChangeCallbackTaihe_);
        if (ret) {
            AUDIO_ERR_LOG("Registering Microphone Change Callback Failed");
        }
    }
    std::shared_ptr<TaiheAudioManagerMicStateChangeCallback> cb =
        std::static_pointer_cast<TaiheAudioManagerMicStateChangeCallback>(audioVolumeGroupManagerImpl->
            micStateChangeCallbackTaihe_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    cb->SaveCallbackReference(cbName, callback);
    AUDIO_DEBUG_LOG("On SetMicStateChangeCallback is successful");
}

void AudioVolumeGroupManagerImpl::OffMicStateChange(optional_view<callback<void(MicStateChangeEvent const&)>> callback)
{
    CHECK_AND_RETURN_RET_LOG(audioGroupMngr_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "audioGroupMngr_ is nullptr");
    std::shared_ptr<uintptr_t> cacheCallback = nullptr;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterMicStateChangeCallback(cacheCallback, this);
}

void AudioVolumeGroupManagerImpl::UnregisterMicStateChangeCallback(std::shared_ptr<uintptr_t> &callback,
    AudioVolumeGroupManagerImpl *audioVolumeGroupManagerImpl)
{
    CHECK_AND_RETURN_RET_LOG(audioVolumeGroupManagerImpl != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "audioVolumeGroupManagerImpl is nullptr");
    std::lock_guard<std::mutex> lock(audioVolumeGroupManagerImpl->mutex_);
    CHECK_AND_RETURN_RET_LOG(audioVolumeGroupManagerImpl->audioGroupMngr_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "audioGroupMngr_ is nullptr");
    CHECK_AND_RETURN_LOG(audioVolumeGroupManagerImpl->micStateChangeCallbackTaihe_ != nullptr,
        "micStateChangeCallbackTaihe is null");
    std::shared_ptr<TaiheAudioManagerMicStateChangeCallback> cb =
        std::static_pointer_cast<TaiheAudioManagerMicStateChangeCallback>(
        audioVolumeGroupManagerImpl->micStateChangeCallbackTaihe_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    if (callback != nullptr) {
        CHECK_AND_RETURN_LOG(cb->IsSameCallback(callback),
            "The callback need to be unregistered is not the same as the registered callback");
    }
    int32_t ret = audioVolumeGroupManagerImpl->audioGroupMngr_->UnsetMicStateChangeCallback(
        audioVolumeGroupManagerImpl->micStateChangeCallbackTaihe_);
    CHECK_AND_RETURN_LOG(ret == OHOS::AudioStandard::SUCCESS, "UnregisterMicStateChangeCallback failed");
    cb->RemoveCallbackReference(callback);
    audioVolumeGroupManagerImpl->micStateChangeCallbackTaihe_ = nullptr;
    AUDIO_INFO_LOG("UnregisterMicStateChangeCallback success");
}

bool AudioVolumeGroupManagerImpl::IsVolumeUnadjustable()
{
    AUDIO_INFO_LOG("IsVolumeUnadjustable");
    bool isVolumeUnadjustable = false;
    if (audioGroupMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioGroupMngr_ is nullptr");
        return isVolumeUnadjustable;
    }
    isVolumeUnadjustable = audioGroupMngr_->IsVolumeUnadjustable();
    AUDIO_INFO_LOG("IsVolumeUnadjustable is successful");
    return isVolumeUnadjustable;
}

void AudioVolumeGroupManagerImpl::AdjustSystemVolumeByStepSync(AudioVolumeType volumeType, VolumeAdjustType adjustType)
{
    int32_t volType = volumeType.get_value();
    if (!((TaiheAudioEnum::IsLegalInputArgumentVolType(volType)) && (volType != TaiheAudioEnum::ALL))) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
            "The param of volumeType must be enum AudioVolumeType");
        return;
    }
    int32_t volumeAdjustType = adjustType.get_value();
    if (!TaiheAudioEnum::IsLegalInputArgumentVolumeAdjustType(volumeAdjustType)) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
            "The param of adjustType must be enum VolumeAdjustType");
        return;
    }
    if (audioGroupMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioGroupMngr_ is nullptr");
        return;
    }
    int32_t volumeAdjustStatus = audioGroupMngr_->AdjustSystemVolumeByStep(TaiheAudioEnum::GetNativeAudioVolumeType(
        volType), static_cast<OHOS::AudioStandard::VolumeAdjustType>(volumeAdjustType));
    if (volumeAdjustStatus != OHOS::AudioStandard::SUCCESS) {
        if (volumeAdjustStatus == OHOS::AudioStandard::ERR_PERMISSION_DENIED) {
            TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_PERMISSION);
            return;
        } else {
            TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "System error. Set app volume fail.");
            return;
        }
    }
    return;
}

double AudioVolumeGroupManagerImpl::GetSystemVolumeInDbSync(AudioVolumeType volumeType, int32_t volumeLevel,
    DeviceType device)
{
    double volumeInDb = VOLUME_DEFAULT_DOUBLE;
    int32_t volType = volumeType.get_value();
    if (!TaiheAudioEnum::IsLegalInputArgumentVolType(volType)) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
            "parameter verification failed: The param of volumeType must be enum AudioVolumeType");
        return volumeInDb;
    }
    int32_t volLevel = volumeLevel;
    int32_t deviceType = device.get_value();
    if (!TaiheAudioEnum::IsLegalInputArgumentDeviceType(deviceType)) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
            "parameter verification failed: The param of volumeType must be enum DeviceType");
        return volumeInDb;
    }
    if (audioGroupMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioGroupMngr_ is nullptr");
        return volumeInDb;
    }
    volumeInDb = audioGroupMngr_->GetSystemVolumeInDb(TaiheAudioEnum::GetNativeAudioVolumeType(volType), volLevel,
        static_cast<OHOS::AudioStandard::DeviceType>(deviceType));
    if (OHOS::AudioStandard::FLOAT_COMPARE_EQ(static_cast<float>(volumeInDb),
        static_cast<float>(OHOS::AudioStandard::ERR_INVALID_PARAM))) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "volumeInDb invalid");
        return volumeInDb;
    } else if (volumeInDb < 0) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "System error. Set app volume fail.");
        return volumeInDb;
    }
    return volumeInDb;
}

double AudioVolumeGroupManagerImpl::GetMaxAmplitudeForInputDeviceSync(AudioDeviceDescriptor inputDevice)
{
    bool inputBArgTransFlag = false;
    double inputMaxAmplitude = INPUT_MAX_DEFAULT_DOUBLE;

    std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor> inputDeviceDescriptor =
        std::make_shared<OHOS::AudioStandard::AudioDeviceDescriptor>();
    TaiheParamUtils::GetAudioDeviceDescriptor(inputDeviceDescriptor, inputBArgTransFlag, inputDevice);
    if (inputDeviceDescriptor == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "inputDeviceDescriptor is nullptr");
        return inputMaxAmplitude;
    }
    if (audioGroupMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioGroupMngr_ is nullptr");
        return inputMaxAmplitude;
    }
    inputMaxAmplitude = audioGroupMngr_->GetMaxAmplitude(inputDeviceDescriptor->deviceId_);
    if (OHOS::AudioStandard::FLOAT_COMPARE_EQ(inputMaxAmplitude,
        static_cast<float>(OHOS::AudioStandard::ERR_INVALID_PARAM))) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "maxAmplitude invalid");
        return inputMaxAmplitude;
    } else if (inputMaxAmplitude < 0) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "System error. Internal variable exception.");
        return inputMaxAmplitude;
    } else if (!inputBArgTransFlag) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "System error. Set app volume fail.");
        return inputMaxAmplitude;
    }
    return inputMaxAmplitude;
}

double AudioVolumeGroupManagerImpl::GetMaxAmplitudeForOutputDeviceSync(AudioDeviceDescriptor inputDevice)
{
    bool outputBArgTransFlag = false;
    double outputMaxAmplitude = OUTPUT_MAX_DEFAULT_DOUBLE;

    std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor> outputDeviceDescriptor =
        std::make_shared<OHOS::AudioStandard::AudioDeviceDescriptor>();
    TaiheParamUtils::GetAudioDeviceDescriptor(outputDeviceDescriptor, outputBArgTransFlag, inputDevice);
    if (outputDeviceDescriptor == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "outputDeviceDescriptor is nullptr");
        return outputMaxAmplitude;
    }
    if (audioGroupMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioGroupMngr_ is nullptr");
        return outputMaxAmplitude;
    }
    outputMaxAmplitude = audioGroupMngr_->GetMaxAmplitude(outputDeviceDescriptor->deviceId_);
    if (OHOS::AudioStandard::FLOAT_COMPARE_EQ(outputMaxAmplitude,
        static_cast<float>(OHOS::AudioStandard::ERR_INVALID_PARAM))) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
            "Parmeter verification faild. OutputDevice not exist.");
        return outputMaxAmplitude;
    } else if (outputMaxAmplitude < 0) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "System error. Internal variable exception.");
        return outputMaxAmplitude;
    } else if (!outputBArgTransFlag) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "System error. Set app volume fail.");
        return outputMaxAmplitude;
    }
    return outputMaxAmplitude;
}
} // namespace ANI::Audio
