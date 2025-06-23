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

#ifndef AUDIO_THREAD_HANDLER_H
#define AUDIO_THREAD_HANDLER_H

#include <functional>
#include <memory>
#include <string>

namespace OHOS {
namespace AudioStandard {
class ThreadHandler {
public:
    static std::shared_ptr<ThreadHandler> NewInstance(const std::string &threadName);
    using Task = std::function<void(void)>;
    ThreadHandler() = default;
    virtual ~ThreadHandler() = default;
    virtual void PostTask(const Task &task) = 0;
    virtual void EnsureTask(const Task &task) = 0;
};
} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_THREAD_HANDLER_H