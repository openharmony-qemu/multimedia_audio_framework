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
#include "taihe_audio_manager_interrupt_callback.h"

namespace ANI::Audio {
using namespace taihe;
using namespace ohos::multimedia::audio;

const std::string VOLUME_CHANGE_CALLBACK_NAME = "volumeChange";

class AudioManagerImpl {
public:
    AudioManagerImpl();
    explicit AudioManagerImpl(std::unique_ptr<AudioManagerImpl> obj);
    ~AudioManagerImpl();

    AudioVolumeManager GetVolumeManager();
    AudioStreamManager GetStreamManager();
    AudioRoutingManager GetRoutingManager();
    AudioSessionManager GetSessionManager();
    AudioEffectManager GetEffectManager();
    AudioScene GetAudioSceneSync();
    AudioSpatializationManager GetSpatializationManager();

    void OnInterrupt(AudioInterrupt const &interrupt, callback_view<void(InterruptAction const&)> callback);
    void OffInterrupt(AudioInterrupt const &interrupt, optional_view<callback<void(InterruptAction const&)>> callback);
    void OnVolumeChange(callback_view<void(VolumeEvent const&)> callback);

    friend AudioManager GetAudioManager();

private:
    static void RegisterInterruptCallback(AudioInterrupt const &interrupt,
        std::shared_ptr<uintptr_t> &callback, AudioManagerImpl *audioMngrImpl);
    static void UnregisterInterruptCallback(AudioInterrupt const &interrupt,
        std::shared_ptr<uintptr_t> &callback, AudioManagerImpl *audioMngrImpl);
    static void RegisterVolumeChangeCallback(std::shared_ptr<uintptr_t> &callback, AudioManagerImpl *audioMngrImpl);

    OHOS::AudioStandard::AudioSystemManager* audioMngr_;
    int32_t cachedClientId_ = -1;
    std::shared_ptr<OHOS::AudioStandard::AudioManagerCallback> interruptCallbackTaihe_ = nullptr;
    std::shared_ptr<OHOS::AudioStandard::VolumeKeyEventCallback> volumeKeyEventCallbackTaihe_ = nullptr;
};
} // namespace ANI::Audio
#endif // TAIHE_AUDIO_MANAGER_H
