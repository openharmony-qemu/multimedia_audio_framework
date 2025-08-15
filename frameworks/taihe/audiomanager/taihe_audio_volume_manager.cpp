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
#define LOG_TAG "AudioVolumeManagerImpl"
#endif

#include "taihe_audio_volume_manager.h"
#include "audio_log.h"
#include "taihe_audio_error.h"
#include "taihe_param_utils.h"
#include "taihe_appvolume_change_callback.h"

namespace ANI::Audio {
AudioVolumeManagerImpl::AudioVolumeManagerImpl() : audioSystemMngr_(nullptr) {}

AudioVolumeManagerImpl::AudioVolumeManagerImpl(std::shared_ptr<AudioVolumeManagerImpl> obj)
    : audioSystemMngr_(nullptr)
{
    if (obj != nullptr) {
        audioSystemMngr_ = obj->audioSystemMngr_;
        cachedClientId_ = obj->cachedClientId_;
    }
}

AudioVolumeManagerImpl::~AudioVolumeManagerImpl()
{
    AUDIO_DEBUG_LOG("AudioVolumeManagerImpl::~AudioVolumeManagerImpl()");
}

array<VolumeGroupInfo> AudioVolumeManagerImpl::GetVolumeGroupInfosSync(string_view networkId)
{
    std::string innerNetworkId = std::string(networkId);
    std::vector<VolumeGroupInfo> emptyResult;
    if (audioSystemMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioSystemMngr_ is nullptr");
        return array<VolumeGroupInfo>(emptyResult);
    }
    std::vector<OHOS::sptr<OHOS::AudioStandard::VolumeGroupInfo>> volumeGroupInfos;
    int32_t ret = audioSystemMngr_->GetVolumeGroups(innerNetworkId, volumeGroupInfos);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, array<VolumeGroupInfo>(emptyResult), "GetVolumeGroups failure!");
    return TaiheParamUtils::SetVolumeGroupInfos(volumeGroupInfos);
}

int32_t AudioVolumeManagerImpl::GetAppVolumePercentageForUidSync(int32_t uid)
{
    int32_t appUid = uid;
    int32_t volLevel = 0;
    if (audioSystemMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioSystemMngr_ is nullptr");
        return volLevel;
    }
    int32_t ret = audioSystemMngr_->GetAppVolume(appUid, volLevel);
    CHECK_AND_RETURN_RET(ret != OHOS::AudioStandard::SUCCESS, volLevel);
    if (ret == OHOS::AudioStandard::ERR_PERMISSION_DENIED) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_PERMISSION);
        return volLevel;
    } else if (ret == OHOS::AudioStandard::ERR_SYSTEM_PERMISSION_DENIED) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED);
        return volLevel;
    } else {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM);
        return volLevel;
    }
}

void AudioVolumeManagerImpl::SetAppVolumePercentageForUidSync(int32_t uid, int32_t volume)
{
    int32_t appUid = uid;
    int32_t volLevel = volume;
    if (audioSystemMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioSystemMngr_ is nullptr");
        return;
    }
    int32_t intValue = audioSystemMngr_->SetAppVolume(appUid, volLevel);
    CHECK_AND_RETURN(intValue != OHOS::AudioStandard::SUCCESS);
    if (intValue == OHOS::AudioStandard::ERR_PERMISSION_DENIED) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_PERMISSION);
        return;
    } else if (intValue == OHOS::AudioStandard::ERR_SYSTEM_PERMISSION_DENIED) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED);
        return;
    } else if (intValue == OHOS::AudioStandard::ERR_INVALID_PARAM) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM);
        return;
    } else {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "System error. Set app volume fail.");
        return;
    }
}

bool AudioVolumeManagerImpl::IsAppVolumeMutedForUidSync(int32_t uid, bool owned)
{
    int32_t appUid = uid;
    bool isOwned = owned;
    bool isMute = false;
    if (audioSystemMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioSystemMngr_ is nullptr");
        return isMute;
    }
    int32_t ret = audioSystemMngr_->IsAppVolumeMute(appUid, isOwned, isMute);
    CHECK_AND_RETURN_RET(ret != OHOS::AudioStandard::SUCCESS, isMute);
    if (ret == OHOS::AudioStandard::ERR_PERMISSION_DENIED) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_PERMISSION);
        return isMute;
    } else if (ret == OHOS::AudioStandard::ERR_SYSTEM_PERMISSION_DENIED) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED);
        return isMute;
    } else {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM);
        return isMute;
    }
}

void AudioVolumeManagerImpl::SetAppVolumeMutedForUidSync(int32_t uid, bool muted)
{
    int32_t appUid = uid;
    bool isMute = muted;
    if (audioSystemMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioSystemMngr_ is nullptr");
        return;
    }
    int32_t intValue = audioSystemMngr_->SetAppVolumeMuted(appUid, isMute);
    CHECK_AND_RETURN(intValue != OHOS::AudioStandard::SUCCESS);
    if (intValue == OHOS::AudioStandard::ERR_PERMISSION_DENIED) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_PERMISSION);
        return;
    } else if (intValue == OHOS::AudioStandard::ERR_SYSTEM_PERMISSION_DENIED) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED);
        return;
    } else {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "System error. Set app volume fail.");
        return;
    }
}

int32_t AudioVolumeManagerImpl::GetAppVolumePercentageSync()
{
    int32_t volLevel = 0;
    if (audioSystemMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioSystemMngr_ is nullptr");
        return volLevel;
    }
    int32_t ret = audioSystemMngr_->GetSelfAppVolume(volLevel);
    if (ret != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM);
        return volLevel;
    } else {
        return volLevel;
    }
}

void AudioVolumeManagerImpl::SetAppVolumePercentageSync(int32_t volume)
{
    int32_t volLevel = volume;
    if (audioSystemMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioSystemMngr_ is nullptr");
        return;
    }
    int32_t intValue = audioSystemMngr_->SetSelfAppVolume(volLevel);
    CHECK_AND_RETURN(intValue != OHOS::AudioStandard::SUCCESS);
    if (intValue == OHOS::AudioStandard::ERR_INVALID_PARAM) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM);
        return;
    } else {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "System error. Set app volume fail.");
        return;
    }
}

AudioVolumeManager AudioVolumeManagerImpl::CreateVolumeManagerWrapper()
{
    std::shared_ptr<AudioVolumeManagerImpl> audioVolMngrImpl = std::make_shared<AudioVolumeManagerImpl>();
    if (audioVolMngrImpl != nullptr) {
        audioVolMngrImpl->audioSystemMngr_ = OHOS::AudioStandard::AudioSystemManager::GetInstance();
        audioVolMngrImpl->cachedClientId_ = getpid();
        return make_holder<AudioVolumeManagerImpl, AudioVolumeManager>(audioVolMngrImpl);
    }
    TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioVolMngrImpl is nullptr");
    return make_holder<AudioVolumeManagerImpl, AudioVolumeManager>(nullptr);
}

AudioVolumeGroupManager AudioVolumeManagerImpl::GetVolumeGroupManagerSync(int32_t groupId)
{
    return AudioVolumeGroupManagerImpl::CreateAudioVolumeGroupManagerWrapper(groupId);
}

void AudioVolumeManagerImpl::RegisterCallback(std::shared_ptr<uintptr_t> &callback,
    const std::string &cbName, AudioVolumeManagerImpl *audioVolMngrImpl)
{
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "audioVolMngrImpl is nullptr");
    std::lock_guard<std::mutex> lock(audioVolMngrImpl->mutex_);
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl->audioSystemMngr_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "audioSystemMngr_ is nullptr");
    if (audioVolMngrImpl->volumeKeyEventCallbackTaihe_ == nullptr) {
        audioVolMngrImpl->volumeKeyEventCallbackTaihe_ = std::make_shared<TaiheAudioVolumeKeyEvent>();
        int32_t ret =
            audioVolMngrImpl->audioSystemMngr_->RegisterVolumeKeyEventCallback(audioVolMngrImpl->cachedClientId_,
                audioVolMngrImpl->volumeKeyEventCallbackTaihe_);
        audioVolMngrImpl->volumeKeyEventCallbackTaiheList_.push_back(
            std::static_pointer_cast<TaiheAudioVolumeKeyEvent>(audioVolMngrImpl->volumeKeyEventCallbackTaihe_));
        if (ret) {
            AUDIO_ERR_LOG("RegisterVolumeKeyEventCallback Failed");
        }
    }
    std::shared_ptr<TaiheAudioVolumeKeyEvent> cb =
        std::static_pointer_cast<TaiheAudioVolumeKeyEvent>(audioVolMngrImpl->volumeKeyEventCallbackTaihe_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    cb->SaveCallbackReference(cbName, callback);
}

void AudioVolumeManagerImpl::RegisterAppVolumeChangeForUidCallback(int32_t appUid, std::shared_ptr<uintptr_t> &callback,
    const std::string &cbName, AudioVolumeManagerImpl *audioVolMngrImpl)
{
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "audioVolMngrImpl is nullptr");
    std::lock_guard<std::mutex> lock(audioVolMngrImpl->mutex_);
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl->audioSystemMngr_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "audioSystemMngr_ is nullptr");
    if (audioVolMngrImpl->appVolumeChangeCallbackForUidTaihe_ == nullptr) {
        audioVolMngrImpl->appVolumeChangeCallbackForUidTaihe_ =
            std::make_shared<TaiheAudioManagerAppVolumeChangeCallback>();
    }
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl->appVolumeChangeCallbackForUidTaihe_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "System error"),
        "RegisterAppVolumeChangeForUidCallback: Memory Allocation Failed !");
    int32_t ret = audioVolMngrImpl->audioSystemMngr_->SetAppVolumeCallbackForUid(appUid,
        audioVolMngrImpl->appVolumeChangeCallbackForUidTaihe_);
    if (ret != OHOS::AudioStandard::SUCCESS) {
        if (ret == OHOS::AudioStandard::ERROR_INVALID_PARAM) {
            TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
                "Parameter verification failed. Invalid callback.");
        } else if (ret == OHOS::AudioStandard::ERR_PERMISSION_DENIED) {
            TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "Permission denied");
        } else {
            TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "System error.");
        }
    }
    std::shared_ptr<TaiheAudioManagerAppVolumeChangeCallback> cb =
        std::static_pointer_cast<TaiheAudioManagerAppVolumeChangeCallback>(
        audioVolMngrImpl->appVolumeChangeCallbackForUidTaihe_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    cb->SaveVolumeChangeCallbackForUidReference(cbName, callback, appUid);
}

void AudioVolumeManagerImpl::RegisterSelfAppVolumeChangeCallback(std::shared_ptr<uintptr_t> &callback,
    const std::string &cbName, AudioVolumeManagerImpl *audioVolMngrImpl)
{
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "audioVolMngrImpl is nullptr");
    std::lock_guard<std::mutex> lock(audioVolMngrImpl->mutex_);
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl->audioSystemMngr_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "audioSystemMngr_ is nullptr");
    if (audioVolMngrImpl->selfAppVolumeChangeCallbackTaihe_ == nullptr) {
        audioVolMngrImpl->selfAppVolumeChangeCallbackTaihe_ =
            std::make_shared<TaiheAudioManagerAppVolumeChangeCallback>();
    }
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl->selfAppVolumeChangeCallbackTaihe_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "System error"),
        "audioVolMngrImpl: Memory Allocation Failed !");
    int32_t ret = audioVolMngrImpl->audioSystemMngr_->SetSelfAppVolumeCallback(
        audioVolMngrImpl->selfAppVolumeChangeCallbackTaihe_);
    if (ret != OHOS::AudioStandard::SUCCESS) {
        if (ret == OHOS::AudioStandard::ERROR_INVALID_PARAM) {
            TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
                "Parameter verification failed. Invalid callback.");
        } else {
            TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "System error.");
        }
    }
    std::shared_ptr<TaiheAudioManagerAppVolumeChangeCallback> cb =
        std::static_pointer_cast<TaiheAudioManagerAppVolumeChangeCallback>(
        audioVolMngrImpl->selfAppVolumeChangeCallbackTaihe_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    cb->SaveSelfVolumdChangeCallbackReference(cbName, callback);
}

void AudioVolumeManagerImpl::UnregisterCallback(std::shared_ptr<uintptr_t> &callback,
    AudioVolumeManagerImpl *audioVolMngrImpl)
{
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "audioVolMngrImpl is nullptr");
    std::lock_guard<std::mutex> lock(audioVolMngrImpl->mutex_);
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl->audioSystemMngr_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "audioSystemMngr_ is nullptr");
    if (callback != nullptr) {
        std::shared_ptr<TaiheAudioVolumeKeyEvent> cb =
            GetVolumeEventTaiheCallback(callback, audioVolMngrImpl);
        CHECK_AND_RETURN_LOG(cb != nullptr, "TaiheAudioVolumeKeyEvent is nullptr");
        int32_t ret = audioVolMngrImpl->audioSystemMngr_->UnregisterVolumeKeyEventCallback(
            audioVolMngrImpl->cachedClientId_, cb);
        CHECK_AND_RETURN_LOG(ret == OHOS::AudioStandard::SUCCESS, "Unset of VolumeKeyEventCallback failed");
        audioVolMngrImpl->volumeKeyEventCallbackTaiheList_.remove(cb);
        audioVolMngrImpl->volumeKeyEventCallbackTaihe_.reset();
        audioVolMngrImpl->volumeKeyEventCallbackTaihe_ = nullptr;
    } else {
        int32_t retValue = audioVolMngrImpl->audioSystemMngr_->UnregisterVolumeKeyEventCallback(
            audioVolMngrImpl->cachedClientId_);
        CHECK_AND_RETURN_LOG(retValue == OHOS::AudioStandard::SUCCESS, "Unset of VolumeKeyEventCallback failed");
        audioVolMngrImpl->volumeKeyEventCallbackTaiheList_.clear();
        audioVolMngrImpl->volumeKeyEventCallbackTaihe_.reset();
        audioVolMngrImpl->volumeKeyEventCallbackTaihe_ = nullptr;
    }
}

void AudioVolumeManagerImpl::UnregisterAppVolumeChangeForUidCallback(std::shared_ptr<uintptr_t> &callback,
    AudioVolumeManagerImpl *audioVolMngrImpl)
{
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "audioVolMngrImpl is nullptr");
    std::lock_guard<std::mutex> lock(audioVolMngrImpl->mutex_);
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl->audioSystemMngr_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "audioSystemMngr_ is nullptr");
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl->appVolumeChangeCallbackForUidTaihe_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM), "UnregisterAppVolumeChangeForUidCallback failed");
    std::shared_ptr<TaiheAudioManagerAppVolumeChangeCallback> cb =
        std::static_pointer_cast<TaiheAudioManagerAppVolumeChangeCallback>(
        audioVolMngrImpl->appVolumeChangeCallbackForUidTaihe_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    if (callback != nullptr) {
        cb->RemoveAudioVolumeChangeForUidCbRef(callback);
    }

    if (callback == nullptr || cb->GetAppVolumeChangeForUidListSize() == 0) {
        audioVolMngrImpl->audioSystemMngr_->UnsetAppVolumeCallbackForUid();
        audioVolMngrImpl->appVolumeChangeCallbackForUidTaihe_.reset();
        audioVolMngrImpl->appVolumeChangeCallbackForUidTaihe_ = nullptr;
        cb->RemoveAllAudioVolumeChangeForUidCbRef();
    }
}

void AudioVolumeManagerImpl::UnregisterSelfAppVolumeChangeCallback(std::shared_ptr<uintptr_t> &callback,
    AudioVolumeManagerImpl *audioVolMngrImpl)
{
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "audioVolMngrImpl is nullptr");
    std::lock_guard<std::mutex> lock(audioVolMngrImpl->mutex_);
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl->audioSystemMngr_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "audioSystemMngr_ is nullptr");
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl->selfAppVolumeChangeCallbackTaihe_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM), "UnregisterSelfAppVolumeChangeCallback failed");
    std::shared_ptr<TaiheAudioManagerAppVolumeChangeCallback> cb =
        std::static_pointer_cast<TaiheAudioManagerAppVolumeChangeCallback>(
        audioVolMngrImpl->selfAppVolumeChangeCallbackTaihe_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    if (callback != nullptr) {
        cb->RemoveSelfAudioVolumeChangeCbRef(callback);
    }
    if (callback == nullptr || cb->GetSelfAppVolumeChangeListSize() == 0) {
        audioVolMngrImpl->audioSystemMngr_->UnsetSelfAppVolumeCallback();
        audioVolMngrImpl->selfAppVolumeChangeCallbackTaihe_.reset();
        audioVolMngrImpl->selfAppVolumeChangeCallbackTaihe_ = nullptr;
        cb->RemoveAllSelfAudioVolumeChangeCbRef();
    }
}

void AudioVolumeManagerImpl::OnVolumeChange(callback_view<void(VolumeEvent const&)> callback)
{
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterCallback(cacheCallback, VOLUME_KEY_EVENT_CALLBACK_NAME, this);
}

void AudioVolumeManagerImpl::OffVolumeChange(optional_view<callback<void(VolumeEvent const&)>> callback)
{
    std::shared_ptr<uintptr_t> cacheCallback;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterCallback(cacheCallback, this);
}

void AudioVolumeManagerImpl::OnAppVolumeChange(callback_view<void(VolumeEvent const&)> callback)
{
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterSelfAppVolumeChangeCallback(cacheCallback, APP_VOLUME_CHANGE_CALLBACK_NAME, this);
}

void AudioVolumeManagerImpl::OffAppVolumeChange(optional_view<callback<void(VolumeEvent const&)>> callback)
{
    std::shared_ptr<uintptr_t> cacheCallback;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterSelfAppVolumeChangeCallback(cacheCallback, this);
}

void AudioVolumeManagerImpl::OnAppVolumeChangeForUid(int32_t uid, callback_view<void(VolumeEvent const&)> callback)
{
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterAppVolumeChangeForUidCallback(uid, cacheCallback, APP_VOLUME_CHANGE_CALLBACK_NAME_FOR_UID, this);
}

void AudioVolumeManagerImpl::OffAppVolumeChangeForUid(optional_view<callback<void(VolumeEvent const&)>> callback)
{
    std::shared_ptr<uintptr_t> cacheCallback;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterAppVolumeChangeForUidCallback(cacheCallback, this);
}

std::shared_ptr<TaiheAudioVolumeKeyEvent> AudioVolumeManagerImpl::GetVolumeEventTaiheCallback(
    std::shared_ptr<uintptr_t> callback, AudioVolumeManagerImpl *audioVolMngrImpl)
{
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl != nullptr, nullptr, "audioVolMngrImpl is nullptr");
    std::shared_ptr<TaiheAudioVolumeKeyEvent> cb = nullptr;
    for (auto &iter : audioVolMngrImpl->volumeKeyEventCallbackTaiheList_) {
        if (iter == nullptr) {
            AUDIO_ERR_LOG("iter is null");
            continue;
        }
        if (iter->ContainSameJsCallback(callback)) {
            cb = iter;
        }
    }
    return cb;
}
} // namespace ANI::Audio
