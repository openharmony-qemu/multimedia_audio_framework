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
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_ILLEGAL_STATE, "audioGroupMngr_ is nullptr");
        return;
    }
    int32_t ret = audioGroupMngr_->SetVolume(TaiheAudioEnum::GetNativeAudioVolumeType(volType), volLevel);
    CHECK_AND_RETURN_RET(ret == OHOS::AudioStandard::SUCCESS, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_SYSTEM, "setvolume failed"));
    return;
}

AudioVolumeType AudioVolumeGroupManagerImpl::GetActiveVolumeTypeSync(int32_t uid)
{
    int32_t clientUid = uid;
    OHOS::AudioStandard::AudioStreamType volType = OHOS::AudioStandard::AudioStreamType::STREAM_DEFAULT;
    if (audioGroupMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_ILLEGAL_STATE, "audioGroupMngr_ is nullptr");
        return AudioVolumeType(static_cast<AudioVolumeType::key_t>(volType));
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
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_ILLEGAL_STATE, "audioGroupMngr_ is nullptr");
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
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_ILLEGAL_STATE, "audioGroupMngr_ is nullptr");
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
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_ILLEGAL_STATE, "audioGroupMngr_ is nullptr");
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
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_ILLEGAL_STATE, "audioGroupMngr_ is nullptr");
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
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_ILLEGAL_STATE, "audioGroupMngr_ is nullptr");
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
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_ILLEGAL_STATE, "audioGroupMngr_ is nullptr");
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
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_ILLEGAL_STATE, "audioGroupMngr_ is nullptr");
        return AudioRingMode(static_cast<AudioRingMode::key_t>(ringerMode));
    }
    ringerMode = audioGroupMngr_->GetRingerMode();
    return AudioRingMode(static_cast<AudioRingMode::key_t>(ringerMode));
}

void AudioVolumeGroupManagerImpl::SetMicrophoneMuteSync(bool mute)
{
    if (audioGroupMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_ILLEGAL_STATE, "audioGroupMngr_ is nullptr");
        return;
    }
    int32_t ret = audioGroupMngr_->SetMicrophoneMute(mute);
    CHECK_AND_RETURN_RET(ret == OHOS::AudioStandard::SUCCESS, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_SYSTEM, "setmicrophonemute failed"));
    return;
}

void AudioVolumeGroupManagerImpl::SetMicMuteSync(bool mute)
{
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return;
    }
    if (audioGroupMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_ILLEGAL_STATE, "audioGroupMngr_ is nullptr");
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
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_ILLEGAL_STATE, "audioGroupMngr_ is nullptr");
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
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_ILLEGAL_STATE, "audioGroupMngr_ is nullptr");
        return isPersistentMicMute;
    }
    isPersistentMicMute = audioGroupMngr_->GetPersistentMicMuteState();
    return isPersistentMicMute;
}

bool AudioVolumeGroupManagerImpl::IsMicrophoneMuteSync()
{
    if (audioGroupMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_ILLEGAL_STATE, "audioGroupMngr_ is nullptr");
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
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_ILLEGAL_STATE, "audioGroupMngr_ is nullptr");
        return;
    }
    int32_t ret = audioGroupMngr_->
    AdjustVolumeByStep(static_cast<OHOS::AudioStandard::VolumeAdjustType>(adjustTypeInt32));
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

void AudioVolumeGroupManagerImpl::OnRingerModeChange(callback_view<void(AudioRingMode)> callback)
{
    CHECK_AND_RETURN_RET_LOG(audioGroupMngr_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "audioGroupMngr_ is nullptr");
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterRingModeCallback(cacheCallback, RINGERMODE_CALLBACK_NAME, this);
}

void AudioVolumeGroupManagerImpl::RegisterRingModeCallback(std::shared_ptr<uintptr_t> &callback,
    const std::string &cbName, AudioVolumeGroupManagerImpl *audioVolumeGroupManagerImpl)
{
    CHECK_AND_RETURN_RET_LOG(audioVolumeGroupManagerImpl != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "audioVolumeGroupManagerImpl is nullptr");
    ani_env *env = get_env();
    CHECK_AND_RETURN_LOG(env != nullptr, "get_env() fail");
    if (audioVolumeGroupManagerImpl->ringerModecallbackTaihe_ == nullptr) {
        audioVolumeGroupManagerImpl->ringerModecallbackTaihe_ = std::make_shared<TaiheAudioRingerModeCallback>(env);
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
    CHECK_AND_RETURN_RET_LOG(audioGroupMngr_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "audioGroupMngr_ is nullptr");
    std::shared_ptr<uintptr_t> cacheCallback;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterRingerModeCallback(cacheCallback, this);
}

void AudioVolumeGroupManagerImpl::UnregisterRingerModeCallback(std::shared_ptr<uintptr_t> &callback,
    AudioVolumeGroupManagerImpl *audioVolumeGroupManagerImpl)
{
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
}

void AudioVolumeGroupManagerImpl::OnMicStateChange(callback_view<void(MicStateChangeEvent const&)> callback)
{
    CHECK_AND_RETURN_RET_LOG(audioGroupMngr_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "audioGroupMngr_ is nullptr");
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterMicStateChangeCallback(cacheCallback, MIC_STATE_CHANGE_CALLBACK_NAME, this);
}

void AudioVolumeGroupManagerImpl::RegisterMicStateChangeCallback(std::shared_ptr<uintptr_t> &callback,
    const std::string &cbName, AudioVolumeGroupManagerImpl *audioVolumeGroupManagerImpl)
{
    CHECK_AND_RETURN_RET_LOG(audioVolumeGroupManagerImpl != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "audioVolumeGroupManagerImpl is nullptr");
    ani_env *env = get_env();
    CHECK_AND_RETURN_LOG(env != nullptr, "get_env() fail");
    if (!audioVolumeGroupManagerImpl->micStateChangeCallbackTaihe_) {
        audioVolumeGroupManagerImpl->micStateChangeCallbackTaihe_ =
            std::make_shared<TaiheAudioManagerMicStateChangeCallback>(env);
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
    std::shared_ptr<uintptr_t> cacheCallback;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterMicStateChangeCallback(cacheCallback, this);
}

void AudioVolumeGroupManagerImpl::UnregisterMicStateChangeCallback(std::shared_ptr<uintptr_t> &callback,
    AudioVolumeGroupManagerImpl *audioVolumeGroupManagerImpl)
{
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
    audioVolumeGroupManagerImpl->ringerModecallbackTaihe_ = nullptr;
}
} // namespace ANI::Audio
