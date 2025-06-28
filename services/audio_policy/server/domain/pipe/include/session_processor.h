/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#ifndef SESSION_PROCESSOR_H
#define SESSION_PROCESSOR_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include "audio_policy_server.h"

namespace OHOS {
namespace AudioStandard {
struct SessionEvent {
    enum class Type {
        ADD,
        REMOVE,
        CLOSE_WAKEUP_SOURCE,
    };

    SessionEvent() = delete;

    Type type;
    uint64_t sessionID;
    SessionInfo sessionInfo_ = {};
};
} // namespace AudioStandard
} // namespace OHOS
#endif // SESSION_PROCESSOR_H