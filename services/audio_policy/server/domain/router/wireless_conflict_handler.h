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

#ifndef ST_WIRELESS_CONFLICT_HANDLER_H
#define ST_WIRELESS_CONFLICT_HANDLER_H

#include "conflict_handler.h"
#include <vector>

namespace OHOS {
namespace AudioStandard {

constexpr int PRIORITY_CALL_OUTPUT = 5;
constexpr int PRIORITY_FORCED_INPUT = 4;
constexpr int PRIORITY_NORMAL_INPUT = 3;
constexpr int PRIORITY_FORCED_OUTPUT = 2;
constexpr int PRIORITY_NORMAL_OUTPUT = 1;
constexpr int PRIORITY_NONE = 0;

class WirelessConflictHandler : public ConflictHandler {
public:
    ~WirelessConflictHandler() override = default;
    bool Detect(const std::shared_ptr<AudioDeviceDescriptor> &device, RouterType routerType) override;
    void SetSelectedDevices(const std::vector<StreamDeviceInfo> &outputDevices,
        const std::vector<StreamDeviceInfo> &inputDevices);
    void ClearSelectedDevices();
    bool AllScoPairSelected();

private:
    bool IsCallStreamUsage(StreamUsage streamUsage);
    bool IsForcedRouterType(RouterType routerType);
    int GetPriority(StreamUsage streamUsage, SourceType sourceType,
        RouterType routerType, bool isOutput);
    bool IsScoDevice(const std::shared_ptr<AudioDeviceDescriptor> &device);
    bool IsNearlinkDevice(const std::shared_ptr<AudioDeviceDescriptor> &device);
    bool IsWirelessDevice(const std::shared_ptr<AudioDeviceDescriptor> &device);
    void FindHighestPriorityDevice();
    
    std::vector<StreamDeviceInfo> outputStreamDevices_;
    std::vector<StreamDeviceInfo> inputStreamDevices_;
    int cachedHighestPriority_ = 0;
    std::string cachedHighestPriorityMac_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_WIRELESS_CONFLICT_HANDLER_H
