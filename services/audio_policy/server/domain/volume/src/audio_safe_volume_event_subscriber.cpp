/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "SafeVolumeEventSubscriber"
#endif

#include <functional>
#include "common_event_subscriber.h"
#include "audio_safe_volume_event_subscriber.h"
#include "audio_volume_manager.h"
#include "audio_safe_volume_notification.h"
#include "audio_policy_log.h"
#include "common_event_manager.h"

namespace OHOS {
namespace AudioStandard {

SafeVolumeEventSubscriber::SafeVolumeEventSubscriber()
{
}

SafeVolumeEventSubscriber::~SafeVolumeEventSubscriber()
{
}

class SafeVolumeEventHandler : public EventFwk::CommonEventSubscriber {
public:
    explicit SafeVolumeEventHandler(const EventFwk::CommonEventSubscribeInfo &subscribeInfo,
        std::function<void(const EventFwk::CommonEventData&)> receiver)
        : EventFwk::CommonEventSubscriber(subscribeInfo), eventReceiver_(receiver) {}
    ~SafeVolumeEventHandler() {}
    void OnReceiveEvent(const EventFwk::CommonEventData &eventData) override
    {
        if (eventReceiver_ == nullptr) {
            AUDIO_ERR_LOG("eventReceiver_ is nullptr.");
            return;
        }
        eventReceiver_(eventData);
    }
private:
    SafeVolumeEventHandler() = default;
    std::function<void(const EventFwk::CommonEventData&)> eventReceiver_;
};

void SafeVolumeEventSubscriber::SubscribeSafeVolumeEvent()
{
    AUDIO_INFO_LOG("enter.");
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(AUDIO_RESTORE_VOLUME_EVENT);
    matchingSkills.AddEvent(AUDIO_INCREASE_VOLUME_EVENT);
    matchingSkills.AddEvent(AUDIO_LEGACY_HIGH_VOLUME_EVENT);
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto commonSubscribePtr = std::make_shared<SafeVolumeEventHandler>(subscribeInfo,
        [](const EventFwk::CommonEventData& eventData) {
            AudioVolumeManager::GetInstance().OnReceiveEvent(eventData);
        });
    if (commonSubscribePtr == nullptr) {
        AUDIO_ERR_LOG("commonSubscribePtr is nullptr");
        return;
    }
    EventFwk::CommonEventManager::SubscribeCommonEvent(commonSubscribePtr);
}

}  // namespace AudioStandard
}  // namespace OHOS
