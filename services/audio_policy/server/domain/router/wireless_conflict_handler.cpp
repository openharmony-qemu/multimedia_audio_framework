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

#include "audio_policy_log.h"

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

bool WirelessConflictHandler::IsScoDevice(const std::shared_ptr<AudioDeviceDescriptor> &device)
{
    CHECK_AND_RETURN_RET(device != nullptr, false);
    return device->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO;
}

bool WirelessConflictHandler::IsNearlinkDevice(const std::shared_ptr<AudioDeviceDescriptor> &device)
{
    CHECK_AND_RETURN_RET(device != nullptr, false);
    return device->deviceType_ == DEVICE_TYPE_NEARLINK || device->deviceType_ == DEVICE_TYPE_NEARLINK_IN;
}

bool WirelessConflictHandler::IsWirelessDevice(const std::shared_ptr<AudioDeviceDescriptor> &device)
{
    CHECK_AND_RETURN_RET(device != nullptr, false);
    return IsScoDevice(device) || IsNearlinkDevice(device) ||
           device->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP ||
           device->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP_IN;
}

bool WirelessConflictHandler::AllScoPairSelected()
{
    bool allScoPair = true;
    for (auto &info : outputStreamDevices_) {
        if (IsScoDevice(info.device) && info.routerType != ROUTER_TYPE_PAIR_DEVICE) {
            allScoPair = false;
        }
    }
    for (auto &info : inputStreamDevices_) {
        if (IsScoDevice(info.device) && info.routerType != ROUTER_TYPE_PAIR_DEVICE) {
            allScoPair = false;
        }
    }
    return allScoPair;
}

void WirelessConflictHandler::FindHighestPriorityDevice()
{
    cachedHighestPriority_ = 0;
    cachedHighestPriorityMac_.clear();
    
    auto findPriority = [this](const std::vector<StreamDeviceInfo>& devices, bool isOutput, bool requireRunning) {
        for (auto &info : devices) {
            if (info.device == nullptr || info.device->macAddress_.empty() || !IsWirelessDevice(info.device)) continue;
            if (requireRunning && !info.isRunning) continue;
            int priority = GetPriority(
                isOutput ? info.streamUsage : STREAM_USAGE_UNKNOWN,
                isOutput ? SOURCE_TYPE_INVALID : info.sourceType,
                info.routerType, isOutput);
            if (priority > cachedHighestPriority_) {
                cachedHighestPriority_ = priority;
                cachedHighestPriorityMac_ = info.device->macAddress_;
            }
        }
    };
    
    findPriority(outputStreamDevices_, true, true);
    findPriority(inputStreamDevices_, false, true);
    
    if (cachedHighestPriority_ > 0) {
        return;
    }
    
    findPriority(outputStreamDevices_, true, false);
    findPriority(inputStreamDevices_, false, false);
}

bool WirelessConflictHandler::Detect(const std::shared_ptr<AudioDeviceDescriptor> &device, RouterType routerType)
{
    if (device == nullptr || device->macAddress_.empty() || !IsWirelessDevice(device)) {
        return false;
    }
    
    if (IsScoDevice(device) && routerType == ROUTER_TYPE_PAIR_DEVICE && AllScoPairSelected()) {
        return true;
    }
    
    return cachedHighestPriority_ > 0 && cachedHighestPriorityMac_ != device->macAddress_;
}

void WirelessConflictHandler::SetSelectedDevices(const std::vector<StreamDeviceInfo> &outputDevices,
    const std::vector<StreamDeviceInfo> &inputDevices)
{
    outputStreamDevices_ = outputDevices;
    inputStreamDevices_ = inputDevices;
    FindHighestPriorityDevice();
}

void WirelessConflictHandler::ClearSelectedDevices()
{
    outputStreamDevices_.clear();
    inputStreamDevices_.clear();
    cachedHighestPriority_ = 0;
    cachedHighestPriorityMac_.clear();
}

} // namespace AudioStandard
} // namespace OHOS
