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
#ifndef LOG_TAG
#define LOG_TAG "WirelessConflictHandler"
#endif

#include "wireless_conflict_handler.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {
bool WirelessConflictHandler::IsCallStreamUsage(StreamUsage streamUsage)
{
    return streamUsage == STREAM_USAGE_VOICE_COMMUNICATION ||
           streamUsage == STREAM_USAGE_VOICE_MODEM_COMMUNICATION ||
           streamUsage == STREAM_USAGE_VIDEO_COMMUNICATION ||
           streamUsage == STREAM_USAGE_RINGTONE ||
           streamUsage == STREAM_USAGE_VOICE_RINGTONE;
}

bool WirelessConflictHandler::IsForcedRouterType(RouterType routerType)
{
    return routerType == ROUTER_TYPE_USER_SELECT ||
           routerType == ROUTER_TYPE_APP_SELECT;
}

int WirelessConflictHandler::GetPriority(StreamUsage streamUsage, SourceType sourceType,
    RouterType routerType, bool isOutput)
{
    if (isOutput && IsCallStreamUsage(streamUsage)) {
        return PRIORITY_CALL_OUTPUT;
    }
    if (!isOutput && IsForcedRouterType(routerType)) {
        return PRIORITY_FORCED_INPUT;
    }
    if (!isOutput) {
        return PRIORITY_NORMAL_INPUT;
    }
    if (isOutput && IsForcedRouterType(routerType)) {
        return PRIORITY_FORCED_OUTPUT;
    }
    if (isOutput) {
        return PRIORITY_NORMAL_OUTPUT;
    }
    return PRIORITY_NONE;
}

bool WirelessConflictHandler::Detect(const std::shared_ptr<AudioDeviceDescriptor> &device, RouterType routerType,
    StreamUsage streamUsage, SourceType sourceType, bool isOutput)
{
    if (device == nullptr || device->macAddress_.empty()) {
        return false;
    }
    
    int currentPriority = GetPriority(streamUsage, sourceType, routerType, isOutput);
    int highestPriority = 0;
    std::string highestPriorityMac;
    
    for (auto &info : outputStreamDevices_) {
        if (info.device == nullptr || info.device->macAddress_.empty()) {
            continue;
        }
        int priority = GetPriority(info.streamUsage, SOURCE_TYPE_INVALID, info.routerType, true);
        if (priority > highestPriority) {
            highestPriority = priority;
            highestPriorityMac = info.device->macAddress_;
        }
    }
    
    for (auto &info : inputStreamDevices_) {
        if (info.device == nullptr || info.device->macAddress_.empty()) {
            continue;
        }
        int priority = GetPriority(STREAM_USAGE_UNKNOWN, info.sourceType, info.routerType, false);
        if (priority > highestPriority) {
            highestPriority = priority;
            highestPriorityMac = info.device->macAddress_;
        }
    }
    
    if (highestPriority > currentPriority && highestPriorityMac != device->macAddress_) {
        return true;
    }
    
    return false;
}

void WirelessConflictHandler::SetSelectedDevices(const std::vector<StreamDeviceInfo> &outputDevices,
    const std::vector<StreamDeviceInfo> &inputDevices)
{
    outputStreamDevices_ = outputDevices;
    inputStreamDevices_ = inputDevices;
}

void WirelessConflictHandler::ClearSelectedDevices()
{
    outputStreamDevices_.clear();
    inputStreamDevices_.clear();
}

} // namespace AudioStandard
} // namespace OHOS
