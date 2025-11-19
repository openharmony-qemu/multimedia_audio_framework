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

#ifndef TAIHE_AUDIO_MANAGER_H
#define TAIHE_AUDIO_MANAGER_H

#include "audio_log.h"
#include "audio_policy_interface.h"
#include "audio_system_manager.h"
#include "taihe_audio_volume_key_event.h"

namespace ANI::Audio {
using namespace taihe;
using namespace ohos::multimedia::audio;

const std::string VOLUME_CHANGE_CALLBACK_NAME = "volumeChange";

class AudioManagerImpl {
public:
    AudioManagerImpl();
    explicit AudioManagerImpl(std::shared_ptr<AudioManagerImpl> obj);
    ~AudioManagerImpl();

    void SetExtraParametersSync(string_view mainKey, map_view<string, string> kvpairs);
    map<string, string> GetExtraParametersSync(string_view mainKey, optional_view<array<string>> subKeys);
    void SetAudioSceneSync(AudioScene scene);
    void DisableSafeMediaVolumeSync();

    AudioVolumeManager GetVolumeManager();
    AudioStreamManager GetStreamManager();
    AudioRoutingManager GetRoutingManager();
    AudioSessionManager GetSessionManager();
    AudioEffectManager GetEffectManager();
    AudioScene GetAudioSceneSync();
    AudioSpatializationManager GetSpatializationManager();
    AudioCollaborativeManager GetCollaborativeManager();
    void OnAudioSceneChange(callback_view<void(AudioScene data)> callback);
    void OffAudioSceneChange(optional_view<callback<void(AudioScene data)>> callback);

    friend AudioManager GetAudioManager();

private:
    static void RegisterAudioSceneChangeCallback(std::shared_ptr<uintptr_t> &callback, AudioManagerImpl *audioMngrImpl);
    static void UnregisterAudioSceneChangeCallback(std::shared_ptr<uintptr_t> &callback,
        AudioManagerImpl *audioMngrImpl);

    OHOS::AudioStandard::AudioSystemManager *audioMngr_;
    std::shared_ptr<OHOS::AudioStandard::AudioManagerAudioSceneChangedCallback>
        audioSceneChangedCallbackTaihe_ = nullptr;
    int32_t cachedClientId_ = -1;
};
} // namespace ANI::Audio
#endif // TAIHE_AUDIO_MANAGER_H
