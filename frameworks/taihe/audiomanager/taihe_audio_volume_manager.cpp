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

using namespace ANI::Audio;

namespace ANI::Audio {
AudioVolumeManagerImpl::AudioVolumeManagerImpl() : audioSystemMngr_(nullptr) {}

AudioVolumeManagerImpl::AudioVolumeManagerImpl(std::unique_ptr<AudioVolumeManagerImpl> obj)
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

AudioVolumeManager AudioVolumeManagerImpl::CreateVolumeManagerWrapper()
{
    std::unique_ptr<AudioVolumeManagerImpl> audioVolMngrImpl = std::make_unique<AudioVolumeManagerImpl>();
    if (audioVolMngrImpl != nullptr) {
        audioVolMngrImpl->audioSystemMngr_ = OHOS::AudioStandard::AudioSystemManager::GetInstance();
        audioVolMngrImpl->cachedClientId_ = getpid();
        return make_holder<AudioVolumeManagerImpl, AudioVolumeManager>(std::move(audioVolMngrImpl));
    }
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
    ani_env *env = get_env();
    if (audioVolMngrImpl->volumeKeyEventCallbackTaihe_ == nullptr) {
        audioVolMngrImpl->volumeKeyEventCallbackTaihe_ = std::make_shared<TaiheAudioVolumeKeyEvent>(env);
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

void AudioVolumeManagerImpl::RegisterAppVolumeChangeForUidCallback(double uid, std::shared_ptr<uintptr_t> &callback,
    const std::string &cbName, AudioVolumeManagerImpl *audioVolMngrImpl)
{
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "audioVolMngrImpl is nullptr");
    int32_t appUid = static_cast<int32_t>(uid);
    ani_env *env = get_env();
    if (audioVolMngrImpl->appVolumeChangeCallbackForUidTaihe_ == nullptr) {
        audioVolMngrImpl->appVolumeChangeCallbackForUidTaihe_ =
            std::make_shared<TaiheAudioManagerAppVolumeChangeCallback>(env);
    }
    CHECK_AND_RETURN_LOG(audioVolMngrImpl->appVolumeChangeCallbackForUidTaihe_,
        "RegisterAppVolumeChangeForUidCallback: Memory Allocation Failed !");
    int32_t ret = audioVolMngrImpl->audioSystemMngr_->SetAppVolumeCallbackForUid(appUid,
        audioVolMngrImpl->appVolumeChangeCallbackForUidTaihe_);
    CHECK_AND_RETURN_LOG(ret == OHOS::AudioStandard::SUCCESS, "SetAppVolumeCallbackForUid Failed");
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
    ani_env *env = get_env();
    if (audioVolMngrImpl->selfAppVolumeChangeCallbackTaihe_ == nullptr) {
        audioVolMngrImpl->selfAppVolumeChangeCallbackTaihe_ =
            std::make_shared<TaiheAudioManagerAppVolumeChangeCallback>(env);
    }
    CHECK_AND_RETURN_LOG(audioVolMngrImpl->selfAppVolumeChangeCallbackTaihe_,
        "audioVolMngrImpl: Memory Allocation Failed !");
    int32_t ret = audioVolMngrImpl->audioSystemMngr_->SetSelfAppVolumeCallback(
        audioVolMngrImpl->selfAppVolumeChangeCallbackTaihe_);
    CHECK_AND_RETURN_LOG(ret == OHOS::AudioStandard::SUCCESS, "SetSelfAppVolumeCallback Failed");
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
        int32_t ret = audioVolMngrImpl->audioSystemMngr_->UnregisterVolumeKeyEventCallback(
            audioVolMngrImpl->cachedClientId_);
        CHECK_AND_RETURN_LOG(ret == OHOS::AudioStandard::SUCCESS, "Unset of VolumeKeyEventCallback failed");
        audioVolMngrImpl->volumeKeyEventCallbackTaiheList_.clear();
        audioVolMngrImpl->volumeKeyEventCallbackTaihe_.reset();
        audioVolMngrImpl->volumeKeyEventCallbackTaihe_ = nullptr;
    }
}

void AudioVolumeManagerImpl::UnregisterAppVolumeChangeForUidCallback(std::shared_ptr<uintptr_t> &callback,
    AudioVolumeManagerImpl *audioVolMngrImpl)
{
    if (audioVolMngrImpl == nullptr) {
        AUDIO_ERR_LOG("audioVolMngrImpl is nullptr");
        return;
    }
    CHECK_AND_RETURN_LOG(audioVolMngrImpl->appVolumeChangeCallbackForUidTaihe_ != nullptr,
        "UnregisterDeviceChangeCallback: audio manager deviceChangeCallbackTaihe_ is null");
    std::shared_ptr<TaiheAudioManagerAppVolumeChangeCallback> cb =
        std::static_pointer_cast<TaiheAudioManagerAppVolumeChangeCallback>(
        audioVolMngrImpl->appVolumeChangeCallbackForUidTaihe_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    if (callback != nullptr) {
        cb->RemoveAudioVolumeChangeForUidCbRef(callback);
    }

    if (cb->GetAppVolumeChangeForUidListSize() == 0) {
        audioVolMngrImpl->audioSystemMngr_->UnsetAppVolumeCallbackForUid();
        audioVolMngrImpl->appVolumeChangeCallbackForUidTaihe_.reset();
        audioVolMngrImpl->appVolumeChangeCallbackForUidTaihe_ = nullptr;
        cb->RemoveAllAudioVolumeChangeForUidCbRef();
    }
}

void AudioVolumeManagerImpl::UnregisterSelfAppVolumeChangeCallback(std::shared_ptr<uintptr_t> &callback,
    AudioVolumeManagerImpl *audioVolMngrImpl)
{
    if (audioVolMngrImpl == nullptr) {
        AUDIO_ERR_LOG("audioVolMngrImpl is nullptr");
        return;
    }
    CHECK_AND_RETURN_LOG(audioVolMngrImpl->selfAppVolumeChangeCallbackTaihe_ != nullptr,
        "UnregisterDeviceChangeCallback: audio manager selfAppVolumeChangeCallbackTaihe_ is null");
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
    CHECK_AND_RETURN_RET_LOG(audioSystemMngr_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "audioSystemMngr_ is nullptr");
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterCallback(cacheCallback, VOLUME_KEY_EVENT_CALLBACK_NAME, this);
}

void AudioVolumeManagerImpl::OffVolumeChange(optional_view<callback<void(VolumeEvent const&)>> callback)
{
    CHECK_AND_RETURN_RET_LOG(audioSystemMngr_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "audioSystemMngr_ is nullptr");
    std::shared_ptr<uintptr_t> cacheCallback;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterCallback(cacheCallback, this);
}

void AudioVolumeManagerImpl::OnAppVolumeChange(callback_view<void(VolumeEvent const&)> callback)
{
    CHECK_AND_RETURN_RET_LOG(audioSystemMngr_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "audioSystemMngr_ is nullptr");
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterSelfAppVolumeChangeCallback(cacheCallback, APP_VOLUME_CHANGE_CALLBACK_NAME, this);
}

void AudioVolumeManagerImpl::OffAppVolumeChange(optional_view<callback<void(VolumeEvent const&)>> callback)
{
    CHECK_AND_RETURN_RET_LOG(audioSystemMngr_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "audioSystemMngr_ is nullptr");
    std::shared_ptr<uintptr_t> cacheCallback;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterSelfAppVolumeChangeCallback(cacheCallback, this);
}

void AudioVolumeManagerImpl::OnAppVolumeChangeForUid(double uid, callback_view<void(VolumeEvent const&)> callback)
{
    CHECK_AND_RETURN_RET_LOG(audioSystemMngr_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "audioSystemMngr_ is nullptr");
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterAppVolumeChangeForUidCallback(uid, cacheCallback, APP_VOLUME_CHANGE_CALLBACK_NAME_FOR_UID, this);
}

void AudioVolumeManagerImpl::OffAppVolumeChangeForUid(optional_view<callback<void(VolumeEvent const&)>> callback)
{
    CHECK_AND_RETURN_RET_LOG(audioSystemMngr_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "audioSystemMngr_ is nullptr");
    std::shared_ptr<uintptr_t> cacheCallback;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterAppVolumeChangeForUidCallback(cacheCallback, this);
}

std::shared_ptr<TaiheAudioVolumeKeyEvent> AudioVolumeManagerImpl::GetVolumeEventTaiheCallback(
    std::shared_ptr<uintptr_t> callback, AudioVolumeManagerImpl *audioVolMngrImpl)
{
    std::shared_ptr<TaiheAudioVolumeKeyEvent> cb = nullptr;
    for (auto &iter : audioVolMngrImpl->volumeKeyEventCallbackTaiheList_) {
        if (iter->ContainSameJsCallback(callback)) {
            cb = iter;
        }
    }
    return cb;
}
} // namespace ANI::Audio
