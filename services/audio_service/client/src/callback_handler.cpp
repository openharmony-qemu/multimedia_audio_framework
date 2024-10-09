/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "CallbackHandlerInner"
#endif

#include "callback_handler.h"
#include "event_handler.h"
#include "event_runner.h"
#include "audio_service_log.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
class CallbackHandlerInner : public CallbackHandler, public AppExecFwk::EventHandler {
public:
    explicit CallbackHandlerInner(std::shared_ptr<IHandler> iHandler);
    ~CallbackHandlerInner();

    void SendCallbackEvent(uint32_t eventCode, int64_t data) override;

    void ReleaseEventRunner() override;

protected:
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;

private:
    std::weak_ptr<IHandler> iHandler_;
};

std::shared_ptr<CallbackHandler> CallbackHandler::GetInstance(std::shared_ptr<IHandler> iHandler)
{
    return std::make_shared<CallbackHandlerInner>(iHandler);
}

CallbackHandlerInner::CallbackHandlerInner(std::shared_ptr<IHandler> iHandler)
    : AppExecFwk::EventHandler(AppExecFwk::EventRunner::Create("OS_AudioStateCB"))
{
    iHandler_ = iHandler;
}

CallbackHandlerInner::~CallbackHandlerInner()
{
    AUDIO_WARNING_LOG("Destructor callback handler inner");
}

void CallbackHandlerInner::SendCallbackEvent(uint32_t eventCode, int64_t data)
{
    SendEvent(AppExecFwk::InnerEvent::Get(eventCode, data));
}

void CallbackHandlerInner::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    uint32_t eventCode = event->GetInnerEventId();
    int64_t data = event->GetParam();
    std::shared_ptr<IHandler> handler = iHandler_.lock();
    if (handler == nullptr) {
        AUDIO_ERR_LOG("iHandler is nullptr");
        return;
    }
    handler->OnHandle(eventCode, data);
}

void CallbackHandlerInner::ReleaseEventRunner()
{
    SetEventRunner(nullptr);
}
} // namespace AudioStandard
} // namespace OHOS
