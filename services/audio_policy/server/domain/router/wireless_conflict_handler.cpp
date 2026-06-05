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
           streamUsage == STREAM_USAGE_VOICE_TRANSFER_COMMUNICATION ||
           streamUsage == STREAM_USAGE_VIDEO_COMMUNICATION ||
           streamUsage == STREAM_USAGE_RINGTONE ||
           streamUsage == STREAM_USAGE_VOICE_RINGTONE ||
           streamUsage == STREAM_USAGE_VOICE_TRANSFER_RINGTONE;
}

bool WirelessConflictHandler::IsForcedRouterType(RouterType routerType)
{
    return routerType == ROUTER_TYPE_USER_SELECT;
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

bool WirelessConflictHandler::IsA2dpDevice(const std::shared_ptr<AudioDeviceDescriptor> &device)
{
    CHECK_AND_RETURN_RET(device != nullptr, false);
    return device->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP ||
           device->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP_IN;
}

bool WirelessConflictHandler::IsNearlinkDevice(const std::shared_ptr<AudioDeviceDescriptor> &device)
{
    CHECK_AND_RETURN_RET(device != nullptr, false);
    return device->deviceType_ == DEVICE_TYPE_NEARLINK || device->deviceType_ == DEVICE_TYPE_NEARLINK_IN;
}

bool WirelessConflictHandler::IsWirelessDevice(const std::shared_ptr<AudioDeviceDescriptor> &device)
{
    CHECK_AND_RETURN_RET(device != nullptr, false);
    return IsScoDevice(device) || IsA2dpDevice(device) || IsNearlinkDevice(device);
}

void WirelessConflictHandler::FindHighestPriorityDevice()
{
    cachedHighestPriority_ = 0;
    cachedHighestPriorityDevice_ = nullptr;
    
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
                cachedHighestPriorityDevice_ = info.device;
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
    
    CHECK_AND_RETURN_RET(cachedHighestPriorityDevice_ != nullptr, false);
    if (!IsScoDevice(cachedHighestPriorityDevice_)) {
        return true;
    }
    
    return cachedHighestPriority_ > 0 && cachedHighestPriorityDevice_->macAddress_ != device->macAddress_;
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
    cachedHighestPriorityDevice_ = nullptr;
}

} // namespace AudioStandard
} // namespace OHOS
