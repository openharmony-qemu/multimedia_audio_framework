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
#ifndef AUDIO_POLICY_ASYNC_ACTION_HANDLER_H
#define AUDIO_POLICY_ASYNC_ACTION_HANDLER_H

#include <mutex>
#include "singleton.h"
#include "event_handler.h"
#include "event_runner.h"

namespace OHOS {
namespace AudioStandard {

// Priority for the Action
enum class ActionPriority : uint32_t {
    // Action that should be distributed at once if possible.
    IMMEDIATE = 0,
    // High priority action, sorted by handle time, should be distributed before low priority action.
    HIGH,
    // Normal action, sorted by handle time.
    LOW,
};

class PolicyAsyncAction {
public:
    virtual void Exec() = 0;
    virtual ~PolicyAsyncAction() = default;
};

struct AsyncActionDesc {
    std::shared_ptr<PolicyAsyncAction> action;
    int64_t delayTimeMs = 0; // Process the action after 'delayTimeMs' milliseconds.
    ActionPriority priority = ActionPriority::LOW;
};

class AudioPolicyAsyncActionHandler : public AppExecFwk::EventHandler {
    DECLARE_DELAYED_SINGLETON(AudioPolicyAsyncActionHandler)
public:
    bool PostAsyncAction(const AsyncActionDesc &desc);

private:
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;

private:
    std::mutex actionMutex_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_POLICY_ASYNC_ACTION_HANDLER_H
