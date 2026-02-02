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

#ifndef LOG_TAG
#define LOG_TAG "AudioInterruptCustom"
#endif

#include "audio_interrupt_custom.h"
#include "audio_utils.h"
#include "audio_log.h"
#include "audio_common_log.h"
#include "audio_source_type.h"
#include "audio_bundle_manager.h"
#include "ipc_skeleton.h"

namespace OHOS {
namespace AudioStandard {

const std::string CELIA_APP_NAME = "vassistant";

static const std::map<std::pair<SourceType, SourceType>, std::pair<AudioFocuState, InterruptHint>> ULTRASONIC_MAP = {
    {{SOURCE_TYPE_VOICE_CALL, SOURCE_TYPE_ULTRASONIC}, {ACTIVE, INTERRUPT_HINT_NONE}},
    {{SOURCE_TYPE_VOICE_COMMUNICATION, SOURCE_TYPE_ULTRASONIC}, {ACTIVE, INTERRUPT_HINT_NONE}},
    {{SOURCE_TYPE_ULTRASONIC, SOURCE_TYPE_VOICE_COMMUNICATION}, {ACTIVE, INTERRUPT_HINT_NONE}}
};

static const std::map<std::pair<SourceType, SourceType>, InterruptHint> ULTRASONIC_STRATEGY_MAP = {
    {{SOURCE_TYPE_VOICE_CALL, SOURCE_TYPE_ULTRASONIC}, INTERRUPT_HINT_NONE},
    {{SOURCE_TYPE_VOICE_COMMUNICATION, SOURCE_TYPE_ULTRASONIC}, INTERRUPT_HINT_NONE},
    {{SOURCE_TYPE_ULTRASONIC, SOURCE_TYPE_VOICE_COMMUNICATION}, INTERRUPT_HINT_NONE}
};

void AudioInterruptCustom::UltraSonicCustomFocus(const AudioInterrupt &incomingInterrupt,
    const AudioInterrupt &activeInterrupt, AudioFocuState &incomingState, InterruptEventInternal &interruptEvent)
{
    SourceType incomingSourceType = incomingInterrupt.audioFocusType.sourceType;
    SourceType activeSourceType = activeInterrupt.audioFocusType.sourceType;
    if (activeSourceType != SOURCE_TYPE_ULTRASONIC && incomingSourceType != SOURCE_TYPE_ULTRASONIC) {
        return;
    }
    if (incomingState < PAUSE) {
        return;
    }
    if (!SolePipe::IsSolePipeSource(SOURCE_TYPE_ULTRASONIC)) {
        return;
    }

    std::pair<SourceType, SourceType> ultraSonicFocus = {activeSourceType, incomingSourceType};
    auto it = ULTRASONIC_MAP.find(ultraSonicFocus);
    if (it != ULTRASONIC_MAP.end()) {
        AUDIO_INFO_LOG("incomingSourceType %{public}d activeSourceType %{public}d set incomingState is %{public}d",
            incomingSourceType, activeSourceType, it->second.first);
        incomingState = it->second.first;
        interruptEvent.hintType = it->second.second;
    }
}

static const std::map<std::pair<AudioStreamType, SourceType>,
    std::pair<AudioFocuState, InterruptHint>> CELIA_FOCUS_MAP = {
    {{STREAM_VOICE_CALL, SOURCE_TYPE_VOICE_RECOGNITION}, {ACTIVE, INTERRUPT_HINT_NONE}}
};

void AudioInterruptCustom::CeliaCustomFocus(const AudioInterrupt &incomingInterrupt,
    const AudioInterrupt &activeInterrupt, AudioFocuState &incomingState,
    InterruptEventInternal &interruptEvent, const std::string &appName)
{
    if (appName.empty() || appName != CELIA_APP_NAME) {
        return;
    }
    SourceType incomingSourceType = incomingInterrupt.audioFocusType.sourceType;
    AudioStreamType activeStreamType = activeInterrupt.audioFocusType.streamType;
    std::pair<AudioStreamType, SourceType> CeliaFocus = {activeStreamType, incomingSourceType};
    auto it = CELIA_FOCUS_MAP.find(CeliaFocus);
    if (it != CELIA_FOCUS_MAP.end()) {
        incomingState = it->second.first;
        interruptEvent.hintType = it->second.second;
        AUDIO_INFO_LOG("Two streams can mix because of %{public}s, incoming %{public}d, active %{public}d",
            appName.c_str(), incomingSourceType, activeStreamType);
    }
}

void AudioInterruptCustom::ProcessActiveStreamCustomFocus(const AudioInterrupt &incomingInterrupt,
    const AudioInterrupt &activeInterrupt, AudioFocuState &incomingState, InterruptEventInternal &interruptEvent)
{
    std::string bundleName = incomingInterrupt.bundleName;
    if (bundleName.empty()) {
        AUDIO_INFO_LOG("bundleName is empty");
        bundleName = AudioBundleManager::GetBundleNameFromUid(incomingInterrupt.uid);
    }
    CeliaCustomFocus(incomingInterrupt, activeInterrupt, incomingState, interruptEvent, bundleName);
    UltraSonicCustomFocus(incomingInterrupt, activeInterrupt, incomingState, interruptEvent);
}

void AudioInterruptCustom::UpdateUltraSonicCustomFocus(const AudioInterrupt &currentInterrupt,
    const AudioInterrupt &incomingInterrupt, AudioFocusEntry &focusEntry)
{
    SourceType incomingSourceType = incomingInterrupt.audioFocusType.sourceType;
    SourceType activeSourceType = currentInterrupt.audioFocusType.sourceType;
    if (activeSourceType != SOURCE_TYPE_ULTRASONIC && incomingSourceType != SOURCE_TYPE_ULTRASONIC) {
        return;
    }
    if (focusEntry.hintType != INTERRUPT_HINT_STOP && focusEntry.hintType != INTERRUPT_HINT_PAUSE) {
        return;
    }
    if (!SolePipe::IsSolePipeSource(SOURCE_TYPE_ULTRASONIC)) {
        return;
    }

    std::pair<SourceType, SourceType> ultraSonicFocus = {activeSourceType, incomingSourceType};
    auto it = ULTRASONIC_STRATEGY_MAP.find(ultraSonicFocus);
    if (it != ULTRASONIC_STRATEGY_MAP.end()) {
        AUDIO_INFO_LOG("activeSourceType %{public}d incomingSourceType %{public}d set activeState is %{public}d",
            activeSourceType, incomingSourceType, it->second);
        focusEntry.hintType = it->second;
    }
}

void AudioInterruptCustom::UpdateCustomFocusStrategy(const AudioInterrupt &currentInterrupt,
    const AudioInterrupt &incomingInterrupt, AudioFocusEntry &focusEntry)
{
    UpdateUltraSonicCustomFocus(currentInterrupt, incomingInterrupt, focusEntry);
}

} // namespace AudioStandard
} // namespace OHOS