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

namespace ANI::Audio {
AudioManagerImpl::AudioManagerImpl() : audioMngr_(nullptr) {}

AudioManagerImpl::AudioManagerImpl(std::shared_ptr<AudioManagerImpl> obj)
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
