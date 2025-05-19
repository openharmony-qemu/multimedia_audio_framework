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
#define LOG_TAG "AudioManagerImpl"
#endif

#include "taihe_audio_manager.h"

#include "audio_device_info.h"
#include "taihe_audio_error.h"
#include "taihe_audio_effect_manager.h"
#include "taihe_audio_routing_manager.h"
#include "taihe_audio_session_manager.h"
#include "taihe_audio_stream_manager.h"
#include "taihe_audio_volume_manager.h"
#include "taihe_audio_spatialization_manager.h"
#include "taihe_param_utils.h"

using namespace ANI::Audio;

namespace ANI::Audio {
AudioManagerImpl::AudioManagerImpl() : audioMngr_(nullptr) {}

AudioManagerImpl::AudioManagerImpl(std::unique_ptr<AudioManagerImpl> obj)
{
    if (obj != nullptr) {
        audioMngr_ = obj->audioMngr_;
        cachedClientId_ = obj->cachedClientId_;
    }
}

AudioManagerImpl::~AudioManagerImpl()
{
    AUDIO_DEBUG_LOG("AudioManagerImpl::~AudioManagerImpl()");
}

AudioVolumeManager AudioManagerImpl::GetVolumeManager()
{
    return AudioVolumeManagerImpl::CreateVolumeManagerWrapper();
}

AudioStreamManager AudioManagerImpl::GetStreamManager()
{
    return AudioStreamManagerImpl::CreateStreamManagerWrapper();
}

AudioRoutingManager AudioManagerImpl::GetRoutingManager()
{
    return AudioRoutingManagerImpl::CreateRoutingManagerWrapper();
}

AudioSessionManager AudioManagerImpl::GetSessionManager()
{
    return AudioSessionManagerImpl::CreateSessionManagerWrapper();
}

AudioEffectManager AudioManagerImpl::GetEffectManager()
{
    return AudioEffectManagerImpl::CreateEffectManagerWrapper();
}

AudioSpatializationManager AudioManagerImpl::GetSpatializationManager()
{
    return AudioSpatializationManagerImpl::CreateSpatializationManagerWrapper();
}

AudioScene AudioManagerImpl::GetAudioSceneSync()
{
    if (audioMngr_ == nullptr) {
        AUDIO_ERR_LOG("audioMngr_ is nullptr");
        return AudioScene::key_t::AUDIO_SCENE_DEFAULT;
    }
    OHOS::AudioStandard::AudioScene audioScene = audioMngr_->GetAudioScene();
    if (audioScene == OHOS::AudioStandard::AudioScene::AUDIO_SCENE_VOICE_RINGING) {
        audioScene = OHOS::AudioStandard::AudioScene::AUDIO_SCENE_RINGING;
    }
    return TaiheAudioEnum::ToTaiheAudioScene(audioScene);
}

void AudioManagerImpl::RegisterInterruptCallback(AudioInterrupt const &interrupt,
    std::shared_ptr<uintptr_t> &callback, AudioManagerImpl *audioMngrImpl)
{
    ani_env *env = get_env();
    if (audioMngrImpl->interruptCallbackTaihe_ == nullptr) {
        audioMngrImpl->interruptCallbackTaihe_ = std::make_shared<TaiheAudioManagerInterruptCallback>(env);
        int32_t ret = audioMngrImpl->audioMngr_->
            SetAudioManagerInterruptCallback(audioMngrImpl->interruptCallbackTaihe_);
        CHECK_AND_RETURN_LOG(ret == OHOS::AudioStandard::SUCCESS, "SetAudioManagerInterruptCallback Failed");
    }
    std::lock_guard<std::mutex> lock(audioMngrImpl->interruptCallbackTaihe_->cbMutex_);
    std::shared_ptr<TaiheAudioManagerInterruptCallback> cb =
        std::static_pointer_cast<TaiheAudioManagerInterruptCallback>(audioMngrImpl->interruptCallbackTaihe_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    cb->SaveCallbackReference(INTERRUPT_CALLBACK_NAME, callback);
    OHOS::AudioStandard::AudioInterrupt audioInterrupt;
    TaiheParamUtils::GetAudioInterrupt(audioInterrupt, interrupt);
    int32_t ret = audioMngrImpl->audioMngr_->RequestAudioFocus(audioInterrupt);
    CHECK_AND_RETURN_LOG(ret == OHOS::AudioStandard::SUCCESS, "RequestAudioFocus Failed");
    AUDIO_INFO_LOG("SetAudioManagerInterruptCallback and RequestAudioFocus is successful");
}

void AudioManagerImpl::RegisterVolumeChangeCallback(std::shared_ptr<uintptr_t> &callback,
    AudioManagerImpl *audioMngrImpl)
{
    ani_env *env = get_env();
    if (audioMngrImpl->volumeKeyEventCallbackTaihe_ == nullptr) {
        audioMngrImpl->volumeKeyEventCallbackTaihe_ = std::make_shared<TaiheAudioVolumeKeyEvent>(env);
        int32_t ret = audioMngrImpl->audioMngr_->RegisterVolumeKeyEventCallback(audioMngrImpl->cachedClientId_,
            audioMngrImpl->volumeKeyEventCallbackTaihe_, OHOS::AudioStandard::API_VERSION::API_8);
        CHECK_AND_RETURN_RET_LOG(ret == OHOS::AudioStandard::SUCCESS, TaiheAudioError::ThrowError(ret),
            "RegisterVolumeKeyEventCallback Failed %{public}d", ret);
    }
    std::shared_ptr<TaiheAudioVolumeKeyEvent> cb =
        std::static_pointer_cast<TaiheAudioVolumeKeyEvent>(audioMngrImpl->volumeKeyEventCallbackTaihe_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    cb->SaveCallbackReference(VOLUME_CHANGE_CALLBACK_NAME, callback);
}

void AudioManagerImpl::UnregisterInterruptCallback(AudioInterrupt const &interrupt,
    std::shared_ptr<uintptr_t> &callback, AudioManagerImpl *audioMngrImpl)
{
    int32_t callbackCount = 0;
    if (audioMngrImpl->interruptCallbackTaihe_ != nullptr) {
        std::shared_ptr<TaiheAudioManagerInterruptCallback> cb =
            std::static_pointer_cast<TaiheAudioManagerInterruptCallback>(audioMngrImpl->interruptCallbackTaihe_);
        CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
        if (callback == nullptr) {
            cb->RemoveAllCallbackReferences(INTERRUPT_CALLBACK_NAME);
        } else if (callback != nullptr) {
            cb->RemoveCallbackReference(INTERRUPT_CALLBACK_NAME, callback);
        }
        callbackCount = cb->GetInterruptCallbackListSize();
    }
    AUDIO_INFO_LOG("Remove Callback Reference success");
    if (callbackCount == 0) {
    OHOS::AudioStandard::AudioInterrupt audioInterrupt;
    TaiheParamUtils::GetAudioInterrupt(audioInterrupt, interrupt);
        int32_t ret = audioMngrImpl->audioMngr_->AbandonAudioFocus(audioInterrupt);
        if (ret) {
            AUDIO_ERR_LOG("Off AbandonAudioFocus Failed");
        }
        ret = audioMngrImpl->audioMngr_->UnsetAudioManagerInterruptCallback();
        CHECK_AND_RETURN_LOG(ret == OHOS::AudioStandard::SUCCESS,
            "Off UnsetAudioManagerInterruptCallback Failed");
        if (audioMngrImpl->interruptCallbackTaihe_ != nullptr) {
            std::lock_guard<std::mutex> lock(audioMngrImpl->interruptCallbackTaihe_->cbMutex_);
            audioMngrImpl->interruptCallbackTaihe_.reset();
            audioMngrImpl->interruptCallbackTaihe_ = nullptr;
        }
        AUDIO_INFO_LOG("Off Abandon Focus and UnsetAudioInterruptCallback success");
    }
}

void AudioManagerImpl::OnInterrupt(AudioInterrupt const &interrupt,
    callback_view<void(InterruptAction const&)> callback)
{
    CHECK_AND_RETURN_RET_LOG(audioMngr_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "audioMngr_ is nullptr");
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterInterruptCallback(interrupt, cacheCallback, this);
}

void AudioManagerImpl::OffInterrupt(AudioInterrupt const &interrupt,
    optional_view<callback<void(InterruptAction const&)>> callback)
{
    CHECK_AND_RETURN_RET_LOG(audioMngr_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "audioMngr_ is nullptr");
    std::shared_ptr<uintptr_t> cacheCallback;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterInterruptCallback(interrupt, cacheCallback, this);
}

void AudioManagerImpl::OnVolumeChange(callback_view<void(VolumeEvent const&)> callback)
{
    CHECK_AND_RETURN_RET_LOG(audioMngr_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "audioMngr_ is nullptr");
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterVolumeChangeCallback(cacheCallback, this);
}

AudioManager GetAudioManager()
{
    std::unique_ptr<AudioManagerImpl> audioMngrImpl = std::make_unique<AudioManagerImpl>();
    if (audioMngrImpl != nullptr) {
        audioMngrImpl->audioMngr_ = OHOS::AudioStandard::AudioSystemManager::GetInstance();
        audioMngrImpl->cachedClientId_ = getpid();
        return make_holder<AudioManagerImpl, AudioManager>(std::move(audioMngrImpl));
    }
    return make_holder<AudioManagerImpl, AudioManager>(nullptr);
}
} // namespace ANI::Audio

TH_EXPORT_CPP_API_GetAudioManager(GetAudioManager);
