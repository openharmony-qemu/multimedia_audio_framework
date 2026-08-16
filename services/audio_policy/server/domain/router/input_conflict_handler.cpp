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
#define LOG_TAG "InputConflictHandler"
#endif

#include "input_conflict_handler.h"
#include "audio_router_infra.h"
#include "audio_router_select_strategy.h"
#include "audio_policy_log.h"

namespace OHOS {
namespace AudioStandard {

bool InputConflictHandler::CanUseConcurrentlyWithOtherDevice(const std::shared_ptr<AudioDeviceDescriptor> &device)
{
    CHECK_AND_RETURN_RET(device != nullptr, false);
    return device->deviceType_ == DEVICE_TYPE_USB_ARM_HEADSET ||
        device->deviceType_ == DEVICE_TYPE_ACCESSORY ||
        device->deviceType_ == DEVICE_TYPE_BT_SPP;
}

void InputConflictHandler::FindHighestPriorityDevice()
{
    highestPriorityDevice_ = AudioRouterSelectStrategy::GetInstance().GetHighestPriorityMediaInputDevice();
}

bool InputConflictHandler::Detect(const std::shared_ptr<AudioDeviceDescriptor> &device, RouterType routerType)
{
    if (device == nullptr || highestPriorityDevice_ == nullptr ||
        highestPriorityDevice_->deviceType_ == DEVICE_TYPE_NONE) {
        return false;
    }

    if (device->deviceRole_ != INPUT_DEVICE) {
        return false;
    }

    if (CanUseConcurrentlyWithOtherDevice(device)) {
        return false;
    }

    if (highestPriorityDevice_->deviceType_ == device->deviceType_) {
        return false;
    }

    return true;
}

void InputConflictHandler::ClearHighestPriorityDevice()
{
    highestPriorityDevice_ = nullptr;
}

std::shared_ptr<AudioDeviceDescriptor> InputConflictHandler::GetHighestPriorityDevice()
{
    return highestPriorityDevice_;
}

} // namespace AudioStandard
} // namespace OHOS
