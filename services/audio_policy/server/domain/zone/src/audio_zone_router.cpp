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

#undef LOG_TAG
#define LOG_TAG "AudioZoneRouter"

#include "audio_zone_router.h"
#include "audio_log.h"
#include "audio_errors.h"
#include "audio_policy_utils.h"

namespace OHOS {
namespace AudioStandard {

ZoneDevicePriority AudioZoneRouter::GetZoneDevicePriority(const std::shared_ptr<AudioDeviceDescriptor> &device)
{
    CHECK_AND_RETURN_RET_LOG(device != nullptr, ZoneDevicePriority::ZONE_DEVICE_PRIORITY_DEFAULT, "device is nullptr");
    return IsBtDevice(device->deviceType_) ? ZoneDevicePriority::ZONE_DEVICE_PRIORITY_BLUETOOTH :
        device->deviceType_ == DEVICE_TYPE_SPEAKER ? ZoneDevicePriority::ZONE_DEVICE_PRIORITY_HEADREST :
        ZoneDevicePriority::ZONE_DEVICE_PRIORITY_DEFAULT;
}

bool AudioZoneRouter::IsAvailableOutput(const std::pair<std::shared_ptr<AudioDeviceDescriptor>, bool> &item)
{
    return item.second && item.first != nullptr && item.first->deviceRole_ == OUTPUT_DEVICE;
}

bool AudioZoneRouter::IsAvailableInput(const std::pair<std::shared_ptr<AudioDeviceDescriptor>, bool> &item)
{
    return item.second && item.first != nullptr && item.first->deviceRole_ == INPUT_DEVICE;
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioZoneRouter::FetchOutputDevices(
    const std::list<std::pair<std::shared_ptr<AudioDeviceDescriptor>, bool>> &devices)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> result;
    for (const auto &item : devices) {
        CHECK_AND_CONTINUE(IsAvailableOutput(item));
        result.emplace_back(item.first);
    }

    std::sort(result.begin(), result.end(),
        [](const std::shared_ptr<AudioDeviceDescriptor> &a, const std::shared_ptr<AudioDeviceDescriptor> &b) {
            ZoneDevicePriority priorityA = AudioZoneRouter::GetZoneDevicePriority(a);
            ZoneDevicePriority priorityB = AudioZoneRouter::GetZoneDevicePriority(b);
            if (priorityA != priorityB) {
                return static_cast<int32_t>(priorityA) > static_cast<int32_t>(priorityB);
            }
            if (priorityA == ZoneDevicePriority::ZONE_DEVICE_PRIORITY_BLUETOOTH) {
                return a->connectTimeStamp_ > b->connectTimeStamp_;
            }
            return false;
        });

    return result;
}

std::shared_ptr<AudioDeviceDescriptor> AudioZoneRouter::FetchInputDevice(
    const std::list<std::pair<std::shared_ptr<AudioDeviceDescriptor>, bool>> &devices)
{
    for (const auto &item : devices) {
        CHECK_AND_CONTINUE(IsAvailableInput(item));
        return item.first;
    }
    return nullptr;
}

} // namespace AudioStandard
} // namespace OHOS
