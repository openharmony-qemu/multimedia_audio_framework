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
#include "taihe_audio_collaborative_manager.h"
#include "taihe_audio_scene_callbacks.h"
#include "taihe_param_utils.h"

namespace ANI::Audio {
AudioManagerImpl::AudioManagerImpl() : audioMngr_(nullptr) {}

AudioManagerImpl::AudioManagerImpl(std::shared_ptr<AudioManagerImpl> obj)
    : audioMngr_(nullptr)
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

void AudioManagerImpl::SetExtraParametersSync(string_view mainKey, map_view<string, string> kvpairs)
{
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return;
    }

    std::string key = std::string(mainKey);
    std::vector<std::pair<std::string, std::string>> subKvpairs;
    int32_t result = TaiheParamUtils::GetExtraParametersSubKV(subKvpairs, kvpairs);
    if (result != AUDIO_OK) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INPUT_INVALID, "get sub key and value failed");
        return;
    }
    if (key.empty()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
            "parameter verification failed: get main key failed");
        return;
    }
    if (subKvpairs.empty()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
            "parameter verification failed: sub key and value is empty");
        return;
    }
    if (audioMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioMngr_ is nullptr");
        return;
    }
    int32_t intValue = audioMngr_->SetExtraParameters(key, subKvpairs);
    if (intValue == OHOS::AudioStandard::ERR_PERMISSION_DENIED) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_PERMISSION, "permission denied");
        return;
    }
    if (intValue != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "SetExtraParameters failed");
        return;
    }
    return;
}

map<string, string> AudioManagerImpl::GetExtraParametersSync(string_view mainKey, optional_view<array<string>> subKeys)
{
    map<string, string> kvpairs;
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return kvpairs;
    }
    std::string key = std::string(mainKey);
    std::vector<std::string> subKeysInner;
    if (subKeys.has_value()) {
        for (const auto &subKey : *subKeys) {
            subKeysInner.push_back(std::string(subKey));
        }
    }
    if (key.empty()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
            "parameter verification failed: get main key failed");
        return kvpairs;
    }
    if (audioMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioMngr_ is nullptr");
        return kvpairs;
    }
    std::vector<std::pair<std::string, std::string>> subKvpairs;
    int32_t intValue = audioMngr_->GetExtraParameters(key, subKeysInner, subKvpairs);
    if (intValue != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "GetExtraParameters failed");
        return kvpairs;
    }
    for (const auto& [key, value] : subKvpairs) {
        kvpairs.emplace(taihe::string(key), taihe::string(value));
    }
    return kvpairs;
}

void AudioManagerImpl::SetAudioSceneSync(AudioScene scene)
{
    int32_t sceneInner = scene.get_value();
    if (audioMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioMngr_ is nullptr");
        return;
    }
    int32_t intValue = audioMngr_->SetAudioScene(static_cast<OHOS::AudioStandard::AudioScene>(sceneInner));
    if (intValue != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "SetAudioScene failed");
        return;
    }
}

void AudioManagerImpl::DisableSafeMediaVolumeSync()
{
    if (audioMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioMngr_ is nullptr");
        return;
    }
    int32_t intValue = audioMngr_->DisableSafeMediaVolume();
    if (intValue == OHOS::AudioStandard::ERR_PERMISSION_DENIED) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_PERMISSION);
        return;
    } else if (intValue == OHOS::AudioStandard::ERR_SYSTEM_PERMISSION_DENIED) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED);
        return;
    }
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

AudioCollaborativeManager AudioManagerImpl::GetCollaborativeManager()
{
    return AudioCollaborativeManagerImpl::CreateCollaborativeManagerWrapper();
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

void AudioManagerImpl::OnAudioSceneChange(callback_view<void(AudioScene data)> callback)
{
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterAudioSceneChangeCallback(cacheCallback, this);
}

void AudioManagerImpl::RegisterAudioSceneChangeCallback(
    std::shared_ptr<uintptr_t> &callback, AudioManagerImpl *audioMngrImpl)
{
    CHECK_AND_RETURN_RET_LOG(audioMngrImpl != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "audioMngrImpl is nullptr");
    CHECK_AND_RETURN_RET_LOG(audioMngrImpl->audioMngr_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "audioMngr_ is nullptr");

    if (audioMngrImpl->audioSceneChangedCallbackTaihe_ == nullptr) {
        auto audioSceneChangedCallbackTaihe = std::make_shared<TaiheAudioSceneChangedCallback>();
        CHECK_AND_RETURN_LOG(audioSceneChangedCallbackTaihe != nullptr, "no memory");

        int32_t ret = audioMngrImpl->audioMngr_->SetAudioSceneChangeCallback(audioSceneChangedCallbackTaihe);
        CHECK_AND_RETURN_LOG(ret == OHOS::AudioStandard::SUCCESS, "SetAudioSceneChangeCallback Failed %{public}d", ret);
        audioMngrImpl->audioSceneChangedCallbackTaihe_ = audioSceneChangedCallbackTaihe;
    }

    std::shared_ptr<TaiheAudioSceneChangedCallback> cb =
        std::static_pointer_cast<TaiheAudioSceneChangedCallback>(audioMngrImpl->audioSceneChangedCallbackTaihe_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "static_pointer_cast failed");

    cb->SaveCallbackReference(AUDIO_SCENE_CHANGE_CALLBACK_NAME, callback);
}

void AudioManagerImpl::OffAudioSceneChange(optional_view<callback<void(AudioScene data)>> callback)
{
    std::shared_ptr<uintptr_t> cacheCallback = nullptr;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterAudioSceneChangeCallback(cacheCallback, this);
}

void AudioManagerImpl::UnregisterAudioSceneChangeCallback(std::shared_ptr<uintptr_t> &callback,
    AudioManagerImpl *audioMngrImpl)
{
    CHECK_AND_RETURN_RET_LOG(audioMngrImpl != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "audioMngrImpl is nullptr");
    CHECK_AND_RETURN_RET_LOG(audioMngrImpl->audioMngr_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "audioMngr_ is nullptr");
    CHECK_AND_RETURN_RET_LOG(audioMngrImpl->audioSceneChangedCallbackTaihe_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM), "audioSceneChangedCallbackTaihe_ is nullptr");

    std::shared_ptr<TaiheAudioSceneChangedCallback> cb =
        std::static_pointer_cast<TaiheAudioSceneChangedCallback>(audioMngrImpl->audioSceneChangedCallbackTaihe_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "static_pointer_cast failed");

    if (callback != nullptr) {
        cb->RemoveCallbackReference(callback);
    }
    if (callback == nullptr || cb->GetAudioSceneCbListSize() == 0) {
        int32_t ret = audioMngrImpl->audioMngr_->UnsetAudioSceneChangeCallback(
            audioMngrImpl->audioSceneChangedCallbackTaihe_);
        CHECK_AND_RETURN_LOG(ret == OHOS::AudioStandard::SUCCESS, "UnsetAudioSceneChangeCallback Failed");
        audioMngrImpl->audioSceneChangedCallbackTaihe_.reset();
        audioMngrImpl->audioSceneChangedCallbackTaihe_ = nullptr;
        cb->RemoveAllCallbackReference();
    }
}

AudioManager GetAudioManager()
{
    std::shared_ptr<AudioManagerImpl> audioMngrImpl = std::make_shared<AudioManagerImpl>();
    if (audioMngrImpl == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioMngrImpl is nullptr");
        return make_holder<AudioManagerImpl, AudioManager>(nullptr);
    }
    audioMngrImpl->audioMngr_ = OHOS::AudioStandard::AudioSystemManager::GetInstance();
    audioMngrImpl->cachedClientId_ = getpid();
    return make_holder<AudioManagerImpl, AudioManager>(audioMngrImpl);
}
} // namespace ANI::Audio

TH_EXPORT_CPP_API_GetAudioManager(ANI::Audio::GetAudioManager);
