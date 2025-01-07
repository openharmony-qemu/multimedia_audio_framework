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

#ifndef CALLBACK_HANDLER_H
#define CALLBACK_HANDLER_H

#include <cinttypes>
#include <memory>

namespace OHOS {
namespace AudioStandard {

class IHandler {
public:
    virtual ~IHandler() = default;
    virtual void OnHandle(uint32_t code, int64_t data) = 0;
};

class CallbackHandler {
public:
    virtual ~CallbackHandler() = default;
    static std::shared_ptr<CallbackHandler> GetInstance(std::shared_ptr<IHandler> iHandler,
        const std::string &handlerName);

    virtual void SendCallbackEvent(uint32_t code, int64_t data) = 0;
    virtual void SendCallbackEvent(uint32_t eventCode, int64_t data, int64_t delayTime) = 0;

    virtual void ReleaseEventRunner() = 0;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // CALLBACK_HANDLER_H
