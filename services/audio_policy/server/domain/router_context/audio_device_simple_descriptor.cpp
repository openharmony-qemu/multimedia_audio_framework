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
#define LOG_TAG "AudioDeviceSimpleDescriptor"
#endif

#include "audio_device_simple_descriptor.h"
#include "audio_device_manager.h"
#include "audio_policy_log.h"

namespace OHOS {
namespace AudioStandard {

std::shared_ptr<AudioDeviceDescriptor> AudioDeviceSimpleDescriptor::GetOnlineDeviceDescriptor() const
{
    auto devices = AudioDeviceManager::GetAudioDeviceManager().GetConnectedDevices();
    for (const auto& device : devices) {
        if (device->deviceType_ == deviceType_ &&
            device->deviceRole_ == deviceRole_ &&
            device->deviceId_ == deviceId_ &&
            device->networkId_ == networkId_ &&
            device->macAddress_ == macAddress_) {
            return device;
        }
    }

    for (const auto &device : devices) {
        if (device == nullptr || device->deviceType_ != deviceType_) {
            continue;
        }
        
        if (macAddress_.empty() || device->macAddress_ == macAddress_) {
            return device;
        }
    }
    return nullptr;
}

}
}
