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

#ifndef OH_AUDIO_MANAGER_H
#define OH_AUDIO_MANAGER_H

#include "audio_info.h"
#include "audio_policy_interface.h"
#include "audio_system_manager.h"
#include "native_audio_common.h"
#include "native_audio_manager.h"

#include <map>

namespace OHOS {
namespace AudioStandard {

class OHAudioManagerAudioSceneChangedCallback : public AudioManagerAudioSceneChangedCallback {
public:
    OHAudioManagerAudioSceneChangedCallback(OH_AudioManager_OnAudioSceneChangeCallback callback, void *userData)
        : callback_(callback), userData_(userData)
    {
    }

    void OnAudioSceneChange(const AudioScene audioScene) override;
private:
    OH_AudioManager_OnAudioSceneChangeCallback callback_ = nullptr;
    void *userData_ = nullptr;
};

class OHAudioManager {
public:
    ~OHAudioManager() {};

    static OHAudioManager *GetInstance();
    AudioScene GetAudioScene();
    int32_t SetAudioSceneChangeCallback(OH_AudioManager_OnAudioSceneChangeCallback callback, void *userData);
    int32_t UnsetAudioSceneChangeCallback(OH_AudioManager_OnAudioSceneChangeCallback callback);
private:
    OHAudioManager() {};
    std::map<OH_AudioManager_OnAudioSceneChangeCallback,
        std::shared_ptr<OHAudioManagerAudioSceneChangedCallback>> callbacks_;
};

} // namespace AudioStandard
} // namespace OHOS
#endif // OH_AUDIO_MANAGER_H