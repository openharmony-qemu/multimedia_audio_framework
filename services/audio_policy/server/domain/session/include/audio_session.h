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
#include <vector>
#include "audio_interrupt_info.h"
#include "audio_session_info.h"
#include "audio_device_info.h"
#include "audio_device_descriptor.h"

namespace OHOS {
namespace AudioStandard {
enum class AudioSessionState {
    SESSION_INVALID = -1,
    SESSION_NEW = 0,
    SESSION_ACTIVE = 1,
    SESSION_DEACTIVE = 2,
    SESSION_RELEASED = 3,
};

class AudioSessionStateMonitor;
class AudioDeviceManager;
class AudioPipeManager;

/*
 * AudioSession is an inner class of AudioSessionService, no mutex is added to AudioSession,
 * and classes other than AudioSessionService are not allowed to directly access AudioSession.
 */
class AudioSession {
public:
    AudioSession(const int32_t callerPid, const AudioSessionStrategy &strategy,
        AudioSessionStateMonitor &audioSessionStateMonitor);
    ~AudioSession();
    bool IsSceneParameterSet();
    int32_t SetAudioSessionScene(AudioSessionScene audioSessionScene);
    bool IsActivated();
    std::vector<AudioInterrupt> GetStreams();
    AudioStreamType GetFakeStreamType();
    void AddStreamInfo(const AudioInterrupt &incomingInterrupt);
    void RemoveStreamInfo(uint32_t streamId);
    void ClearStreamInfo(void);
    uint32_t GetFakeStreamId();
    void SaveFakeStreamId(uint32_t fakeStreamId);
    void Dump(std::string &dumpString);
    int32_t Activate(const AudioSessionStrategy strategy);
    int32_t Deactivate();
    AudioSessionStrategy GetSessionStrategy();
    bool IsAudioSessionEmpty();
    bool IsAudioRendererEmpty();
    int32_t SetSessionDefaultOutputDevice(const DeviceType &deviceType);
    void GetSessionDefaultOutputDevice(DeviceType &deviceType);
    bool IsStreamContainedInCurrentSession(const uint32_t &streamId);
    bool GetAndClearNeedToFetchFlag();
    bool IsRecommendToStopAudio(AudioStreamDeviceChangeReason changeReason,
        const std::shared_ptr<AudioDeviceDescriptor> desc);
    bool IsSessionOutputDeviceChanged(const std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor);
    bool IsSessionInputDeviceChanged(const std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor);
    StreamUsage GetSessionStreamUsage();
    bool IsBackGroundApp(void);
    void MarkSystemApp(void);
    bool IsSystemApp(void) const;

private:
    StreamUsage GetStreamUsageInner();
    bool IsLegalDevice(const DeviceType deviceType);
    void UpdateVoipStreamsDefaultOutputDevice();
    bool CanCurrentStreamSetDefaultOutputDevice(const AudioInterrupt &interrupt);
    int32_t EnableSingleVoipStreamDefaultOutputDevice(const AudioInterrupt &interrupt);
    int32_t EnableVoipStreamsDefaultOutputDevice();
    int32_t EnableDefaultDevice();
    void UpdateSingleVoipStreamDefaultOutputDevice(const AudioInterrupt &interrupt);
    bool IsSessionDefaultDeviceEnabled();
    bool IsOutputDeviceConfigurableByStreamUsage(const StreamUsage &streamUsage);
    int32_t callerPid_;
    bool needToFetch_ = false;
    AudioSessionStrategy strategy_ {AudioConcurrencyMode::INVALID};
    AudioSessionStateMonitor &audioSessionStateMonitor_;
    AudioSessionScene audioSessionScene_ {AudioSessionScene::INVALID};
    // These are streams included in audiosession focus.
    std::vector<AudioInterrupt> streamsInSession_;
    uint32_t fakeStreamId_ {0};
    AudioSessionState state_ = AudioSessionState::SESSION_INVALID;
    DeviceType defaultDeviceType_ = DEVICE_TYPE_INVALID;
    AudioDeviceDescriptor deviceDescriptor_;
    AudioDeviceDescriptor inputDeviceDescriptor_;
    std::shared_ptr<AudioPipeManager> pipeManager_ = nullptr;
    AudioDeviceManager &deviceManager_;
    bool isSystemApp_ {false};
};
} // namespace AudioStandard
} // namespace OHOS

#endif // ST_AUDIO_SESSION_SERVICE_H