
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
#ifndef TAIHE_AUDIO_SESSION_MANAGER_H
#define TAIHE_AUDIO_SESSION_MANAGER_H

#include "audio_system_manager.h"
#include "audio_session_info.h"
#include "audio_session_manager.h"
#include "taihe_audio_session_callback.h"

namespace ANI::Audio {
using namespace taihe;
using namespace ohos::multimedia::audio;

const std::string AUDIOSESSION_CALLBACK_NAME = "audioSessionDeactivated";

class AudioSessionManagerImpl {
public:
    AudioSessionManagerImpl();
    explicit AudioSessionManagerImpl(OHOS::AudioStandard::AudioSessionManager *audioSessionMngr);
    ~AudioSessionManagerImpl();

    static AudioSessionManager CreateSessionManagerWrapper();

    void ActivateAudioSessionSync(AudioSessionStrategy const &strategy);
    void DeactivateAudioSessionSync();
    bool IsAudioSessionActivated();
    void OnAudioSessionDeactivated(callback_view<void(AudioSessionDeactivatedEvent const&)> callback);
    void OffAudioSessionDeactivated(optional_view<callback<void(AudioSessionDeactivatedEvent const&)>> callback);

private:
    static void RegisterAudioSessionCallback(std::shared_ptr<uintptr_t> &callback,
        AudioSessionManagerImpl *taiheSessionManager);
    static void UnregisterCallbackCarryParam(std::shared_ptr<uintptr_t> &callback,
        AudioSessionManagerImpl *taiheSessionManager);
    static void UnregisterCallback(AudioSessionManagerImpl *taiheSessionManager);

    OHOS::AudioStandard::AudioSessionManager *audioSessionMngr_;
    std::shared_ptr<OHOS::AudioStandard::AudioSessionCallback> audioSessionCallbackTaihe_ = nullptr;
    std::mutex mutex_;
};
}  // namespace ANI::Audio
#endif // TAIHE_AUDIO_SESSION_MANAGER_H