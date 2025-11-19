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
#include "singleton.h"
#include "audio_session.h"
#include "audio_session_state_monitor.h"
#include "audio_device_info.h"
#include "audio_session_device_info.h"

namespace OHOS {
namespace AudioStandard {
class SessionTimeOutCallback {
public:
    virtual ~SessionTimeOutCallback() = default;

    virtual void OnSessionTimeout(const int32_t pid) = 0;
};

class AudioSessionService : public AudioSessionStateMonitor {
    DECLARE_SINGLETON(AudioSessionService)
public:
    // Audio session manager interfaces
    int32_t ActivateAudioSession(const int32_t callerPid, const AudioSessionStrategy &strategy);
    int32_t DeactivateAudioSession(const int32_t callerPid);
    bool IsAudioSessionActivated(const int32_t callerPid);

    // Audio session timer callback
    void OnAudioSessionTimeOut(int32_t callerPid) override;

    // other public interfaces
    int32_t SetSessionTimeOutCallback(const std::shared_ptr<SessionTimeOutCallback> &timeOutCallback);

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
    void AddStreamInfo(const AudioInterrupt &audioInterrupt);
    void RemoveStreamInfo(const int32_t callerPid, const uint32_t streamId);
    void ClearStreamInfo(const int32_t callerPid);
    bool IsStreamInfoEmpty(const int32_t callerPid);
    bool IsAudioRendererEmpty(const int32_t callerPid);
    AudioConcurrencyMode GetSessionStrategy(int32_t callerPid);
    bool ShouldAudioSessionProcessHintType(InterruptHint hintType);
    bool ShouldAudioStreamProcessHintType(InterruptHint hintType);
    static bool IsSameTypeForAudioSession(const AudioStreamType incomingType, const AudioStreamType existedType);
    void NotifyAppStateChange(const int32_t pid, bool isBackState);
    bool HasStreamForDeviceType(int32_t callerPid, DeviceType deviceType);
    int32_t FillCurrentOutputDeviceChangedEvent(
        int32_t callerPid,
        AudioStreamDeviceChangeReason changeReason,
        CurrentOutputDeviceChangedEvent &deviceChangedEvent);
    bool IsSessionInputDeviceChanged(int32_t callerPid, const std::shared_ptr<AudioDeviceDescriptor> desc);
    void MarkSystemApp(int32_t pid);
    bool IsSystemApp(int32_t pid);
    bool IsSystemAppWithMixStrategy(const AudioInterrupt &audioInterrupt);

private:
    int32_t DeactivateAudioSessionInternal(const int32_t callerPid, bool isSessionTimeout = false);
    void GenerateFakeStreamId(int32_t callerPid);
    std::shared_ptr<AudioSession> CreateAudioSession(
        int32_t callerPid, AudioSessionStrategy strategy = {AudioConcurrencyMode::INVALID});
    bool IsAudioSessionFocusModeInner(int32_t callerPid);
    bool ShouldExcludeStreamTypeInner(const AudioInterrupt &audioInterrupt);

private:
    std::mutex sessionServiceMutex_;
    std::unordered_map<int32_t, std::shared_ptr<AudioSession>> sessionMap_;
    std::weak_ptr<SessionTimeOutCallback> timeOutCallback_;
};
} // namespace AudioStandard
} // namespace OHOS

#endif // ST_AUDIO_SESSION_SERVICE_H