/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you you may not use this file except in compliance with the License.
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

#ifndef AUDIO_DEVICE_SIMPLE_DESCRIPTOR_H
#define AUDIO_DEVICE_SIMPLE_DESCRIPTOR_H

#include <memory>
#include <string>
#include "audio_device_descriptor.h"

namespace OHOS {
namespace AudioStandard {

class AudioDeviceSimpleDescriptor {
public:
    DeviceType deviceType_ = DEVICE_TYPE_NONE;
    DeviceRole deviceRole_ = DEVICE_ROLE_NONE;
    int32_t deviceId_ = 0;
    std::string networkId_;
    std::string macAddress_;

    AudioDeviceSimpleDescriptor() = default;
    AudioDeviceSimpleDescriptor(DeviceType type, DeviceRole role, int32_t deviceId, const std::string& networkId,
        const std::string& macAddress)
        : deviceType_(type), deviceRole_(role), deviceId_(deviceId), networkId_(networkId), macAddress_(macAddress) {}
    ~AudioDeviceSimpleDescriptor() = default;

    bool operator==(const AudioDeviceSimpleDescriptor& other) const
    {
        return deviceId_ == other.deviceId_;
    }

    bool operator!=(const AudioDeviceSimpleDescriptor& other) const
    {
        return !(*this == other);
    }

    bool operator==(const AudioDeviceDescriptor& other) const
    {
        return deviceId_ == other.deviceId_;
    }

    bool operator!=(const AudioDeviceDescriptor& other) const
    {
        return !(*this == other);
    }

    bool operator<(const AudioDeviceSimpleDescriptor& other) const
    {
        if (deviceType_ == other.deviceType_) {
            return deviceId_ < other.deviceId_;
        }
        return deviceType_ < other.deviceType_;
    }

    std::shared_ptr<AudioDeviceDescriptor> GetOnlineDeviceDescriptor() const;
};

}
}
#endif
