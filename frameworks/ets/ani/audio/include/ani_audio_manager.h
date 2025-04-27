/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#ifndef ANI_AUDIO_MANAGER_H
#define ANI_AUDIO_MANAGER_H

#include "ani.h"
#include "audio_system_manager.h"
#include "audio_common_log.h"
#include "ani_audio_volume_manager.h"
#include "ani_audio_volume_key_event.h"

namespace OHOS {
namespace AudioStandard {
const std::string ANI_AUDIO_MNGR_CLASS_NAME = "AudioManager";
const std::string VOLUME_CHANGE_CALLBACK_NAME = "volumeChange";

class AniAudioManager {
public:
    AniAudioManager();
    ~AniAudioManager();

    static ani_status AudioManagerAniInit(ani_env *env);
    static ani_object Constructor([[maybe_unused]] ani_env *env);
    static AniAudioManager* Unwrap(ani_env *env, ani_object object);
    static ani_object GetVolumeManager(ani_env *env, ani_object object);
    static void On(ani_env *env, ani_object object, ani_string type, ani_object callback, ani_object info);
    static void RegisterInterruptCallback(ani_env *env,
        ani_object info, ani_object callback, AniAudioManager *audioManagerAni) {};
    static void RegisterVolumeChangeCallback(ani_env *env, ani_object callback, AniAudioManager *audioManagerAni);
    static void RegisterRingerModeCallback(ani_env *env, ani_object callback, AniAudioManager *audioManagerAni) {};
    static void RegisterDeviceChangeCallback(ani_env *env, ani_object callback, AniAudioManager *audioManagerAni) {};
    static void RegisterCallback(ani_env *env, ani_object jsThis,
        const std::string &cbName, ani_object info, ani_object callback);

private:
    AudioSystemManager *audioMngr_;
    ani_env *env_;
    int32_t cachedClientId_ = -1;
    std::shared_ptr<VolumeKeyEventCallback> volumeKeyEventCallbackAni_ = nullptr;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif //  ANI_AUDIO_MANAGER_H
