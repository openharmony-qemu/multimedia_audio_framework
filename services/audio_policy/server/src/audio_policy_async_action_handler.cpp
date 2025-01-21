/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioPolicyAsyncActionHandler"
#endif

#include "audio_policy_async_action_handler.h"
#include "audio_policy_log.h"

namespace OHOS {
namespace AudioStandard {

enum {
    DEFAULT_EVENT_ID = 0,
};

AudioPolicyAsyncActionHandler::AudioPolicyAsyncActionHandler() : AppExecFwk::EventHandler(
    AppExecFwk::EventRunner::Create("OS_APAsyncActionHandler", AppExecFwk::ThreadMode::FFRT))
{
    AUDIO_DEBUG_LOG("ctor");
}

AudioPolicyAsyncActionHandler::~AudioPolicyAsyncActionHandler()
{
    AUDIO_WARNING_LOG("dtor should not happen");
};

bool AudioPolicyAsyncActionHandler::PostAsyncAction(const AsyncActionDesc &desc)
{
    bool ret = false;
    AUDIO_INFO_LOG("priority type = %{public}u", desc.priority);
    switch (desc.priority) {
        case ActionPriority::IMMEDIATE: {
            std::lock_guard<std::mutex> lock(actionMutex_);
            ret = SendImmediateEvent(DEFAULT_EVENT_ID, desc.action);
            break;
        }
        case ActionPriority::HIGH: {
            std::lock_guard<std::mutex> lock(actionMutex_);
            ret = SendHighPriorityEvent(AppExecFwk::InnerEvent::Get(DEFAULT_EVENT_ID, desc.action), desc.delayTimeMs);
            break;
        }
        case ActionPriority::LOW: {
            std::lock_guard<std::mutex> lock(actionMutex_);
            ret = SendEvent(DEFAULT_EVENT_ID, desc.action, desc.delayTimeMs);
            break;
        }
        default: {
            AUDIO_ERR_LOG("Unknown priority type = %{public}u", desc.priority);
            break;
        }
    }
    if (!ret) {
        CHECK_AND_RETURN_RET_LOG(ret, ret, "PostAsyncAction failed, priority type = %{public}u", desc.priority);
    }
    return ret;
}

void AudioPolicyAsyncActionHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    CHECK_AND_RETURN_LOG(event != nullptr, "event is nullptr");

    uint32_t eventId = event->GetInnerEventId();
    std::shared_ptr<PolicyAsyncAction> action = event->GetSharedObject<PolicyAsyncAction>();
    CHECK_AND_RETURN_LOG(action != nullptr, "action is nullptr");
    action->Exec();
}

} // namespace AudioStandard
} // namespace OHOS
