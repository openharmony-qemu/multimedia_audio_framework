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

#ifndef ST_AUDIO_INTERRUPT_STRATEGY_H
#define ST_AUDIO_INTERRUPT_STRATEGY_H

#include <atomic>
#include "audio_errors.h"
#include "audio_interrupt_info.h"
#include "audio_interrupt_utils.h"
#include "audio_utils.h"
#include "iremote_object.h"
#include "istandard_audio_policy_manager_listener.h"
#include "audio_session_service.h"

namespace OHOS {
namespace AudioStandard {

class AudioInterruptStrategy {
public:
    AudioInterruptStrategy();
    void UpdateAudioFocusStrategy(const AudioInterrupt &currentInterrupt,
        const AudioInterrupt &incomingInterrupt, AudioFocusEntry &focusEntry);
    int32_t SetQueryBundleNameListCallback(const sptr<IRemoteObject> &object);
    void OnQueryBundleNameIsInList(const std::string &bundleName, const std::string &params, bool& ret);
    void RegisterPCModeListener();
    int32_t OnQueryPausedTime();

private:
    static constexpr int32_t DEFAULT_PAUSE_TIMEOUT_S = 5;
    sptr<IStandardAudioPolicyManagerListener> queryBundleNameListCallback_ = nullptr;
    std::atomic_bool isPCMode_ = false;

    void UpdateMuteAudioFocusStrategy(const AudioInterrupt &currentInterrupt,
        const AudioInterrupt &incomingInterrupt, AudioFocusEntry &focusEntry);
    void UpdateWindowFocusStrategy(const AudioInterrupt &currentInterrupt,
        const AudioInterrupt &incomingInterrupt, AudioFocusEntry &focusEntry);
    void UpdatePCModeFocusStrategy(const AudioInterrupt &currentInterrupt,
        const AudioInterrupt &incomingInterrupt, AudioFocusEntry &focusEntry);
    void UpdatePauseAudioFocusStrategy(const AudioInterrupt &currentInterrupt,
        const AudioInterrupt &incomingInterrupt, AudioFocusEntry &focusEntry);
    void UpdateMicFocusByUid(const AudioInterrupt &currentInterrupt,
        const AudioInterrupt &incomingInterrupt, AudioFocusEntry &focusEntry);
    void UpdateMicFocusStrategy(const AudioFocusType &existAudioFocusType,
        const AudioFocusType &incomingAudioFocusType, const std::string &currentBundleName,
        const std::string &incomingBundleName, AudioFocusEntry &focusEntry);
    void UpdateAIBaseFocusStrategy(const AudioInterrupt &currentInterrupt, const AudioInterrupt &incomingInterrupt,
        AudioFocusEntry &focusEntry);
    void UpdateVoipCaptureFocusStrategy(const AudioInterrupt &currentInterrupt, const AudioInterrupt &incomingInterrupt,
        AudioFocusEntry &focusEntry);
    void UpdateMapFocusStrategy(const std::string &bundleName,
        AudioFocusEntry &focusEntry, bool isExistMediaStream, SourceType incomingSourceType);
    void UpdateFocusStrategy(const std::string& bundleName,
        AudioFocusEntry& focusEntry, bool isExistMediaStream, bool isIncomingMediaStream, bool isCheckMuteStream);
    bool IsMicSource(SourceType sourceType);
    void UpdateMediaWithMixStrategy(const AudioInterrupt &activeInterrupt,
        const AudioInterrupt &incomingInterrupt, AudioFocusEntry &focusEntry);
    void UpdateMediaWithDuckStrategy(const AudioInterrupt &activeInterrupt,
        const AudioInterrupt &incomingInterrupt, AudioFocusEntry &focusEntry);
    bool IsMediaWithMixStrategy(const AudioInterrupt &currentInterrupt);
    bool IsCanMixwithMedia(const AudioInterrupt &currentInterrupt);
 
    AudioSessionService &sessionService_;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // ST_AUDIO_INTERRUPT_STRATEGY_H