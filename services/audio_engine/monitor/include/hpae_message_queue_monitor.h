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
#ifndef HPAE_MESSAGE_QUEUE_MONITOR_H
#define HPAE_MESSAGE_QUEUE_MONITOR_H
#include <cstdint>
#include <string>
namespace OHOS {
namespace AudioStandard {
namespace HPAE {
enum MessageQueueType : int32_t {
    HPAE_MANAGER_TYPE,
    HPAE_RENDERER_MANAGER_TYPE,
    HPAE_CAPTURE_MANAGER_TYPE,
    HPAE_INNER_CAPTURE_MANAGER_TYPE,
    HPAE_OFFLOAD_MANAGER_TYPE,
    HPAE_NO_LOCK_QUEUE_TYPE,
};
class HpaeMessageQueueMonitor {
public:
    static void ReportMessageQueueException(MessageQueueType type, const std::string &func, const std::string &error);
};
}
}
}
#endif