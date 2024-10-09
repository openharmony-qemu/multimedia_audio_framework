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

#ifndef OH_AUDIO_SESSION_MANAGER_H
#define OH_AUDIO_SESSION_MANAGER_H

#include "audio_log.h"
#include "native_audio_session_manager.h"
#include "audio_session_manager.h"

namespace OHOS {
namespace AudioStandard {

class OHAudioSessionCallback : public AudioSessionCallback {
public:
    explicit OHAudioSessionCallback(OH_AudioSession_DeactivatedCallback callback)
        : callback_(callback)
    {
    }

    void OnAudioSessionDeactive(const AudioSessionDeactiveEvent &deactiveEvent) override;

    OH_AudioSession_DeactivatedCallback GetCallback()
    {
        return callback_;
    }

    ~OHAudioSessionCallback()
    {
        AUDIO_INFO_LOG("~OHAudioSessionCallback called.");
        callback_ = nullptr;
    }
private:
    OH_AudioSession_DeactivatedCallback callback_;
};

class OHAudioSessionManager {
public:
    ~OHAudioSessionManager();

    static OHAudioSessionManager* GetInstance()
    {
        if (!ohAudioSessionManager_) {
            ohAudioSessionManager_ = new OHAudioSessionManager();
        }
        return ohAudioSessionManager_;
    }

    OH_AudioCommon_Result ActivateAudioSession(const AudioSessionStrategy &strategy);

    OH_AudioCommon_Result DeactivateAudioSession();

    bool IsAudioSessionActivated();

    OH_AudioCommon_Result SetAudioSessionCallback(OH_AudioSession_DeactivatedCallback callback);

    OH_AudioCommon_Result UnsetAudioSessionCallback(OH_AudioSession_DeactivatedCallback callback);

private:
    OHAudioSessionManager();
    
    static OHAudioSessionManager *ohAudioSessionManager_;

    AudioSessionManager *audioSessionManager_ = AudioSessionManager::GetInstance();
};

OHAudioSessionManager* OHAudioSessionManager::ohAudioSessionManager_ = nullptr;

} // namespace AudioStandard
} // namespace OHOS
#endif // OH_AUDIO_SESSION_MANAGER_H