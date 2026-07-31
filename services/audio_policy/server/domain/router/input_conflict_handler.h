/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef ST_INPUT_CONFLICT_HANDLER_H
#define ST_INPUT_CONFLICT_HANDLER_H

#include "conflict_handler.h"

namespace OHOS {
namespace AudioStandard {

class InputConflictHandler : public ConflictHandler {
public:
    ~InputConflictHandler() override = default;
    bool Detect(const std::shared_ptr<AudioDeviceDescriptor> &device, RouterType routerType) override;
    void FindHighestPriorityDevice();
    void ClearHighestPriorityDevice();
    std::shared_ptr<AudioDeviceDescriptor> GetHighestPriorityDevice() override;

private:
    bool CanUseConcurrentlyWithOtherDevice(const std::shared_ptr<AudioDeviceDescriptor> &device);

    std::shared_ptr<AudioDeviceDescriptor> highestPriorityDevice_ = nullptr;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_INPUT_CONFLICT_HANDLER_H
