/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "audio_info.h"
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
}  // namespace AudioStandard
}  // namespace OHOS
