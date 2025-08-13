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
#include <vector>
#include "audio_session.h"
#include "audio_session_state_monitor.h"
#include "audio_device_info.h"

namespace OHOS {
namespace AudioStandard {
class SessionTimeOutCallback {
public:
    virtual ~SessionTimeOutCallback() = default;

    virtual void OnSessionTimeout(const int32_t pid) = 0;
};

class AudioSessionService : public AudioSessionStateMonitor, public std::enable_shared_from_this<AudioSessionService> {
public:
    AudioSessionService();
    ~AudioSessionService() override;

    // Audio session manager interfaces
    static std::shared_ptr<AudioSessionService> GetAudioSessionService(void);
    int32_t ActivateAudioSession(const int32_t callerPid, const AudioSessionStrategy &strategy);
    int32_t DeactivateAudioSession(const int32_t callerPid);
    bool IsAudioSessionActivated(const int32_t callerPid);

    // Audio session timer callback
    void OnAudioSessionTimeOut(int32_t callerPid) override;

    // other public interfaces
    int32_t SetSessionTimeOutCallback(const std::shared_ptr<SessionTimeOutCallback> &timeOutCallback);
    std::shared_ptr<AudioSession> GetAudioSessionByPid(const int32_t callerPid);

    // Dump AudioSession Info
    void AudioSessionInfoDump(std::string &dumpString);
    int32_t SetSessionDefaultOutputDevice(const int32_t callerPid, const DeviceType &deviceType);
    bool IsStreamAllowedToSetDevice(const uint32_t streamId);
    DeviceType GetSessionDefaultOutputDevice(const int32_t callerPid);
    bool IsSessionNeedToFetchOutputDevice(const int32_t callerPid);

    int32_t SetAudioSessionScene(int32_t callerPid, AudioSessionScene scene);
    StreamUsage GetAudioSessionStreamUsage(int32_t callerPid);
    bool IsAudioSessionFocusMode(int32_t callerPid);
    bool ShouldBypassFocusForStream(const AudioInterrupt &audioInterrupt);
    bool ShouldExcludeStreamType(const AudioInterrupt &audioInterrupt);
    std::vector<AudioInterrupt> GetStreams(int32_t callerPid);
    AudioInterrupt GenerateFakeAudioInterrupt(int32_t callerPid);
    void RemoveStreamInfo(const AudioInterrupt &audioInterrupt);
    void ClearStreamInfo(const int32_t callerPid);
    bool ShouldAudioSessionProcessHintType(InterruptHint hintType);
    bool ShouldAudioStreamProcessHintType(InterruptHint hintType);
    static bool IsSameTypeForAudioSession(const AudioStreamType incomingType, const AudioStreamType existedType);
    void NotifyAppStateChange(const int32_t pid, bool isBackState);
    bool HasStreamForDeviceType(int32_t callerPid, DeviceType deviceType);

private:
    int32_t DeactivateAudioSessionInternal(const int32_t callerPid, bool isSessionTimeout = false);
    std::shared_ptr<AudioSessionStateMonitor> GetSelfSharedPtr() override;
    void GenerateFakeStreamId(int32_t callerPid);

private:
    std::mutex sessionServiceMutex_;
    std::unordered_map<int32_t, std::shared_ptr<AudioSession>> sessionMap_;
    std::weak_ptr<SessionTimeOutCallback> timeOutCallback_;
};
} // namespace AudioStandard
} // namespace OHOS

#endif // ST_AUDIO_SESSION_SERVICE_H