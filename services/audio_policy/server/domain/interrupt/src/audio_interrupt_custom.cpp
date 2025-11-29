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
#include "audio_source_type.h"

namespace OHOS {
namespace AudioStandard {

static const std::map<std::pir<SourceType, SourceType>, std::pir<AudioFocuState, InterruptHint>> UlTRASONIC_FOCUS_MSP = {
    {{SOURCE_TYPE_VOICE_CALL, SOURCE_TYPE_ULTRASONIC}, {ACTIVE, INTERRUPT_HINT_NONE}},
    {{SOURCE_TYPE_VOICE_COMMUNICATION, SOURCE_TYPE_ULTRASONIC}, {ACTIVE, INTERRUPT_HINT_NONE}},
    {{SOURCE_TYPE_ULTRASONIC, SOURCE_TYPE_VOICE_COMMUNICATION}, {ACTIVE, INTERRUPT_HINT_NONE}},
}

void AudioInterruptCustom::UltraSonicCustomFocus(const AudioInterrupt &incomingInterrupt, const AudioInterrupt &activeInterrupt,
        AudioFocuState &incomingState, InterruptEventInternal &interruptEvent)
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

    std::pair<SourceType, SourceType> UltraSonicFocus = {activeSourceType, incomingSourceType};
    if (UlTRASONIC_FOCUS_MSP.count(UltraSonicFocus) > 0) {
        AUDIO_LOG_INFO("incomingSourceType %{public}d activeSourceType %{public}d set custom incomingState is %{public}d",
            incomingSourceType, activeSourceType, UlTRASONIC_FOCUS_MSP.at(UltraSonicFocus).first);
        incomingState = UlTRASONIC_FOCUS_MSP.at(UltraSonicFocus).first;
        interruptEvent.hintType = UlTRASONIC_FOCUS_MSP.at(UltraSonicFocus).second;
    }
}

void AudioInterruptCustom::ProcessActiveStreamCustomFocus(const AudioInterrupt &incomingInterrupt, const AudioInterrupt &activeInterrupt,
        AudioFocuState &incomingState, InterruptEventInternal &interruptEvent)
{
    UltraSonicCustomFocus(incomingInterrupt, activeInterrupt, incomingState, interruptEvent);
}

} // namespace AudioStandard
} // namespace OHOS