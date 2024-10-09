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

#ifndef ST_AUDIO_SESSION_H
#define ST_AUDIO_SESSION_H

#include <mutex>

#include "audio_interrupt_info.h"
#include "audio_session_info.h"
#include "audio_session_timer.h"

namespace OHOS {
namespace AudioStandard {
enum class AudioSessionState {
    SESSION_INVALID = -1,
    SESSION_NEW = 0,
    SESSION_ACTIVE = 1,
    SESSION_DEACTIVE = 2,
    SESSION_RELEASED = 3,
};

class AudioSession {
public:
    AudioSession(const int32_t callerPid, const AudioSessionStrategy &strategy,
        const std::shared_ptr<AudioSessionTimer> sessionTimer);
    ~AudioSession();

    int32_t Activate();
    int32_t Deactivate();
    AudioSessionState GetSessionState();
    void SetSessionStrategy(const AudioSessionStrategy strategy);
    AudioSessionStrategy GetSessionStrategy();
    int32_t AddAudioInterrpt(const std::pair<AudioInterrupt, AudioFocuState> interruptPair);
    int32_t RemoveAudioInterrpt(const std::pair<AudioInterrupt, AudioFocuState> interruptPair);
    int32_t RemoveAudioInterrptByStreamId(const uint32_t &streamId);
    bool IsAudioSessionEmpty();

private:
    std::mutex sessionMutex_;

    int32_t callerPid_;
    AudioSessionStrategy strategy_;
    std::shared_ptr<AudioSessionTimer> sessionTimer_;

    AudioSessionState state_ = AudioSessionState::SESSION_INVALID;
    std::unordered_map<uint32_t, std::pair<AudioInterrupt, AudioFocuState>> interruptMap_;
};
} // namespace AudioStandard
} // namespace OHOS

#endif // ST_AUDIO_SESSION_SERVICE_H