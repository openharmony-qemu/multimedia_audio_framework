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

class AudioInterruptDfx : public std::enable_shared_from_this<AudioInterruptDfx> {
public:
    AudioInterruptDfx();
    void ActivateAudioSessionErrorEvent(
        const std::list<std::pair<AudioInterrupt, AudioFocuState>> &audioFocusInfoList, const int32_t callerPid);
    void DeactivateAudioSessionErrorEvent(
        const std::vector<AudioInterrupt> &streamsInSession, const int32_t callerPid);
    void AddInterruptErrorEvent(const AudioInterrupt &audioInterrupt, const int32_t callerPid);

private:
    void WriteAudioInterruptErrorEvent(const AudioFocusErrorEvent &interruptError);
    bool IsInterruptErrorEvent(AudioStreamType sceneStreamType, AudioStreamType incomingStreamType);

    AudioSessionService &sessionService_;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // ST_AUDIO_INTERRUPT_DFX_H