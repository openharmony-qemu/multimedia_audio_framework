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

#ifndef I_AUDIO_ZONE_EVENT_DISPATCHER_H
#define I_AUDIO_ZONE_EVENT_DISPATCHER_H

#include <list>

#include "audio_zone_info.h"
#include "audio_interrupt_info.h"

namespace OHOS {
namespace AudioStandard {
enum AudioZoneEventType {
    AUDIO_ZONE_INVALID_EVENT = 0,
    AUDIO_ZONE_ADD_EVENT,
    AUDIO_ZONE_REMOVE_EVENT,
    AUDIO_ZONE_CHANGE_EVENT,
    AUDIO_ZONE_INTERRUPT_EVENT,
};

struct AudioZoneEvent {
    AudioZoneEventType type;
    pid_t clientPid;
    int32_t zoneId;
    std::string deviceTag;
    std::shared_ptr<AudioZoneDescriptor> descriptor;
    AudioZoneChangeReason zoneChangeReason;
    std::list<std::pair<AudioInterrupt, AudioFocuState>> interrupts;
    AudioZoneInterruptReason zoneInterruptReason;
};

class IAudioZoneEventDispatcher {
public:
    virtual void DispatchEvent(std::shared_ptr<AudioZoneEvent> event) = 0;
};
} // namespace AudioStandard
} // namespace OHOS

#endif // I_AUDIO_ZONE_EVENT_DISPATCHER_H