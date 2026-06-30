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

#ifndef ST_AUDIO_ZONE_ROUTER_H
#define ST_AUDIO_ZONE_ROUTER_H

#include <vector>
#include <list>
#include <memory>
#include <utility>
#include "audio_device_descriptor.h"

namespace OHOS {
namespace AudioStandard {

enum ZoneDevicePriority {
    ZONE_DEVICE_PRIORITY_DEFAULT = 0,
    ZONE_DEVICE_PRIORITY_HEADREST = 1,
    ZONE_DEVICE_PRIORITY_BLUETOOTH = 2,
};

class AudioZoneRouter {
public:
    AudioZoneRouter() = default;
    ~AudioZoneRouter() = default;

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> FetchOutputDevices(
        const std::list<std::pair<std::shared_ptr<AudioDeviceDescriptor>, bool>> &devices);
    std::shared_ptr<AudioDeviceDescriptor> FetchInputDevice(
        const std::list<std::pair<std::shared_ptr<AudioDeviceDescriptor>, bool>> &devices);

private:
    static ZoneDevicePriority GetZoneDevicePriority(const std::shared_ptr<AudioDeviceDescriptor> &device);
    static bool IsAvailableOutput(const std::pair<std::shared_ptr<AudioDeviceDescriptor>, bool> &item);
    static bool IsAvailableInput(const std::pair<std::shared_ptr<AudioDeviceDescriptor>, bool> &item);
};

} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_ZONE_ROUTER_H
