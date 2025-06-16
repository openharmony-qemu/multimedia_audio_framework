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

#ifndef ST_AUDIO_SESSION_SERVICE_H
#define ST_AUDIO_SESSION_SERVICE_H

#include <mutex>

#include "audio_session.h"
#include "audio_session_state_monitor.h"

namespace OHOS {
namespace AudioStandard {
class SessionTimeOutCallback {
public:
    virtual ~SessionTimeOutCallback() = default;

    virtual void OnSessionTimeout(const int32_t pid) = 0; // 超时释放
};

class AudioSessionService : public AudioSessionStateMonitor, public std::enable_shared_from_this<AudioSessionService> {
public:
    AudioSessionService();
    ~AudioSessionService() override;

    // Audio session manager interfaces
    int32_t ActivateAudioSession(const int32_t callerPid, const AudioSessionStrategy &strategy);
    int32_t DeactivateAudioSession(const int32_t callerPid);
    bool IsAudioSessionActivated(const int32_t callerPid);

    // Audio session timer callback
    void OnAudioSessionTimeOut(int32_t callerPid) override;

    // other public interfaces
    int32_t SetSessionTimeOutCallback(const std::shared_ptr<SessionTimeOutCallback> &timeOutCallback);
    std::shared_ptr<AudioSession> GetAudioSessionByPid(const int32_t callerPid);

    static bool IsSameTypeForAudioSession(const AudioStreamType incomingType, const AudioStreamType existedType);
    // Dump AudioSession Info
    void AudioSessionInfoDump(std::string &dumpString);

private:
    int32_t DeactivateAudioSessionInternal(const int32_t callerPid, bool isSessionTimeout = false);
    std::shared_ptr<AudioSessionStateMonitor> GetSelfSharedPtr() override;

private:
    std::mutex sessionServiceMutex_;
    std::unordered_map<int32_t, std::shared_ptr<AudioSession>> sessionMap_;
    std::weak_ptr<SessionTimeOutCallback> timeOutCallback_;
};
} // namespace AudioStandard
} // namespace OHOS

#endif // ST_AUDIO_SESSION_SERVICE_H