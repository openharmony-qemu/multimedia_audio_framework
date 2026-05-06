/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef ST_AUDIO_INTERRUPT_DFX_H
#define ST_AUDIO_INTERRUPT_DFX_H

#include <list>
#include <memory>
#include <thread>
#include <chrono>
#include "audio_interrupt_info.h"
#include "audio_policy_log.h"
#include "audio_session_service.h"

namespace OHOS {
namespace AudioStandard {

struct AudioFocusErrorEvent {
    int32_t callerPid;
    std::string errorInfo;
    InterruptHint hintType;
    std::string appName;
    std::string rendererInfo;
    std::string audiosessionInfo;
    int32_t isAppInForeground;
    int32_t rendererPlayTimes;
    std::string interruptedAppName;
    std::string interruptedRendererInfo;
    std::string interruptedAudiosessionInfo;
};

struct AudioInterruptCheckInfo {
    bool isMuteStream;
    std::chrono::steady_clock::time_point startTime;
    std::list<AudioFocusErrorEvent> errorEvents;
};

enum InterruptErrorCode {
    TYPE_MEDIA_PLAYBACK_NOT_LISTENING_INTERRUPT_CHANGE,
    TYPE_MEDIA_PLAYBACK_ACTIVATED_LISTENING_INTERRUPT_CHANGE,
    TYPE_AUDIOSESSION_NOT_LISTENING_STATE_CHANGE,
    TYPE_AUDIOSESSION_ACTIVATED_LISTENING_STATE_CHANGE,
};

class AudioInterruptDfx : public std::enable_shared_from_this<AudioInterruptDfx> {
public:
    AudioInterruptDfx();
    void ActivateAudioSessionErrorEvent(
        const std::list<std::pair<AudioInterrupt, AudioFocuState>> &audioFocusInfoList, const int32_t callerPid);
    void DeactivateAudioSessionErrorEvent(
        const std::vector<AudioInterrupt> &streamsInSession, const int32_t callerPid);
    void AddInterruptErrorEvent(const AudioInterrupt &audioInterrupt, const int32_t callerPid);
    void RecordInterruptEvent(const InterruptEventInternal &interruptEvent,
        const AudioInterrupt &activeInterrupt, const AudioInterrupt &incomingInterrupt);
    void NotifyStreamSilentChange(uint32_t streamId);
    void CheckInterruptEvent(uint32_t streamId);
    void DelayCheckMuteStreamInterrupt(uint32_t streamId);
    void HandleInterruptCallbackEvent(const AudioInterrupt &audioInterrupt);
    void HandleStateChangeCallbackEvent(const int32_t usage, const int32_t uid, const int32_t type);

private:
    void WriteAudioInterruptErrorEvent(const AudioFocusErrorEvent &interruptError);
    bool IsInterruptErrorEvent(AudioStreamType sceneStreamType, AudioStreamType incomingStreamType);
    std::string GetInterruptRendererInfo(const AudioInterrupt &audioInterrupt);
    std::string GetInterruptSessionInfo(const AudioInterrupt &audioInterrupt);
    void RecordMediaInterruptEvent(const InterruptEventInternal &interruptEvent,
        const AudioInterrupt &activeInterrupt, const AudioInterrupt &incomingInterrupt);
    void CheckMuteInterruptEvent(uint32_t streamId);
    void CheckShortInterruptEvent(uint32_t streamId);
    bool IsCheckDfxStream(AudioStreamType audioStreamType);
    void WriteInterruptOrSessionCallbackErrorEvent(const int32_t usage, const int32_t uid, const int32_t type);

    AudioSessionService &sessionService_;

    std::mutex cachedFocusMutex_;
    std::unordered_map<uint32_t, AudioInterruptCheckInfo> audioInterruptCheckInfo_;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // ST_AUDIO_INTERRUPT_DFX_H