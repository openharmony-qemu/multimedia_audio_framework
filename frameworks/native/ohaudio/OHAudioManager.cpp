/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "OHAudioManager.h"

#include <set>

#include "audio_common_log.h"
#include "audio_system_manager.h"

namespace {
// should be same with OH_AudioScene in native_audio_common.h
const std::set<OHOS::AudioStandard::AudioScene> INVALID_AUDIO_SCENES = {
    OHOS::AudioStandard::AUDIO_SCENE_DEFAULT,
    OHOS::AudioStandard::AUDIO_SCENE_RINGING,
    OHOS::AudioStandard::AUDIO_SCENE_PHONE_CALL,
    OHOS::AudioStandard::AUDIO_SCENE_PHONE_CHAT,
    OHOS::AudioStandard::AUDIO_SCENE_VOICE_RINGING
};
}
using OHOS::AudioStandard::OHAudioManager;
static OHOS::AudioStandard::OHAudioManager *convertManager(OH_AudioManager *audioManager)
{
    return (OHAudioManager*) audioManager;
}

OH_AudioCommon_Result OH_GetAudioManager(OH_AudioManager **audioManager)
{
    if (audioManager == nullptr) {
        AUDIO_ERR_LOG("invalid OH_AudioManager");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }
    OHAudioManager *manager = OHAudioManager::GetInstance();
    *audioManager = (OH_AudioManager *)manager;
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OH_GetAudioScene(OH_AudioManager* manager, OH_AudioScene *scene)
{
    if (manager == nullptr || scene == nullptr) {
        AUDIO_ERR_LOG("invalid OH_AudioManager or scene");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }
    *scene = static_cast<OH_AudioScene>(convertManager(manager)->GetAudioScene());
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OH_AudioManager_RegisterAudioSceneChangeCallback(OH_AudioManager *manager,
    OH_AudioManager_OnAudioSceneChangeCallback callback, void *userData)
{
    if (manager == nullptr || callback == nullptr) {
        AUDIO_ERR_LOG("invalid OH_AudioManager or callback");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }

    OHAudioManager *ohAudioManager = (OHAudioManager*)manager;
    int32_t result = ohAudioManager->SetAudioSceneChangeCallback(callback, userData);
    return static_cast<OH_AudioCommon_Result>(result);
}

OH_AudioCommon_Result OH_AudioManager_UnregisterAudioSceneChangeCallback(OH_AudioManager *manager,
    OH_AudioManager_OnAudioSceneChangeCallback callback)
{
    if (manager == nullptr || callback == nullptr) {
        AUDIO_ERR_LOG("invalid OH_AudioManager or callback");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }

    OHAudioManager *ohAudioManager = (OHAudioManager*)manager;
    int32_t result = ohAudioManager->UnsetAudioSceneChangeCallback(callback);
    return static_cast<OH_AudioCommon_Result>(result);
}

namespace OHOS {
namespace AudioStandard {
OHAudioManager *OHAudioManager::GetInstance()
{
    static OHAudioManager manager;
    return &manager;
}

AudioScene OHAudioManager::GetAudioScene()
{
    AudioScene scene = AudioSystemManager::GetInstance()->GetAudioScene();
    if (!INVALID_AUDIO_SCENES.count(scene)) {
        AUDIO_WARNING_LOG("Get scene:%{public}d that is not defined, return defalut!", scene);
        return AUDIO_SCENE_DEFAULT;
    }
    if (scene == AUDIO_SCENE_VOICE_RINGING) {
        return AUDIO_SCENE_RINGING;
    }
    return scene;
}

int32_t OHAudioManager::SetAudioSceneChangeCallback(OH_AudioManager_OnAudioSceneChangeCallback callback, void *userData)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM,
        "Failed, pointer to the fuction is nullptr");
    if (callbacks_.count(callback) != 0) {
        AUDIO_INFO_LOG("Callback is already registered");
        return AUDIOCOMMON_RESULT_SUCCESS;
    }

    auto ohAudioManagerAudioSceneChangedCallback =
        std::make_shared<OHAudioManagerAudioSceneChangedCallback>(callback, userData);
    CHECK_AND_RETURN_RET_LOG(ohAudioManagerAudioSceneChangedCallback != nullptr, AUDIOCOMMON_RESULT_ERROR_SYSTEM,
        "Failed, create callback failed");

    AudioSystemManager *audioSystemManager = AudioSystemManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioSystemManager != nullptr, AUDIOCOMMON_RESULT_ERROR_SYSTEM,
        "Failed, audio system manager is nullptr");

    int32_t result = audioSystemManager->SetAudioSceneChangeCallback(
        ohAudioManagerAudioSceneChangedCallback);
    if (result == AUDIOCOMMON_RESULT_SUCCESS) {
        callbacks_.emplace(callback, ohAudioManagerAudioSceneChangedCallback);
    }
    return result == AUDIOCOMMON_RESULT_SUCCESS ? AUDIOCOMMON_RESULT_SUCCESS : AUDIOCOMMON_RESULT_ERROR_SYSTEM;
}

int32_t OHAudioManager::UnsetAudioSceneChangeCallback(OH_AudioManager_OnAudioSceneChangeCallback callback)
{
    if (callback == nullptr || !callbacks_.count(callback)) {
        AUDIO_ERR_LOG("Invalid callback or callback not registered");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }

    AudioSystemManager *audioSystemManager = AudioSystemManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioSystemManager != nullptr, AUDIOCOMMON_RESULT_ERROR_SYSTEM,
        "Failed, audio system manager is nullptr");

    int32_t result = audioSystemManager->UnsetAudioSceneChangeCallback(callbacks_[callback]);
    if (result == AUDIOCOMMON_RESULT_SUCCESS) {
        callbacks_.erase(callback);
    }
    return result == AUDIOCOMMON_RESULT_SUCCESS ? AUDIOCOMMON_RESULT_SUCCESS : AUDIOCOMMON_RESULT_ERROR_SYSTEM;
}

void OHAudioManagerAudioSceneChangedCallback::OnAudioSceneChange(const AudioScene audioScene)
{
    CHECK_AND_RETURN_LOG(callback_ != nullptr, "Failed, pointer to the fuction is nullptr");
    AudioScene scene = audioScene;
    if (!INVALID_AUDIO_SCENES.count(audioScene)) {
        AUDIO_WARNING_LOG("Get scene:%{public}d that is not defined, return defalut!", scene);
        scene = AUDIO_SCENE_DEFAULT;
    }
    if (audioScene == AUDIO_SCENE_VOICE_RINGING) {
        scene = AUDIO_SCENE_RINGING;
    }
    callback_(userData_, static_cast<OH_AudioScene>(scene));
}
}  // namespace AudioStandard
}  // namespace OHOS
