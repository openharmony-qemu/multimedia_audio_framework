/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef ST_ROUTER_BASE_H
#define ST_ROUTER_BASE_H

#include "audio_system_manager.h"
#include "audio_device_manager.h"
#include "audio_policy_manager_factory.h"
#include "audio_policy_log.h"
#include "audio_state_manager.h"

namespace OHOS {
namespace AudioStandard {
class RouterBase {
public:
    std::string name_;
    IAudioPolicyInterface& audioPolicyManager_;
    RouterBase() : audioPolicyManager_(AudioPolicyManagerFactory::GetAudioPolicyManager()) {}
    virtual ~RouterBase() {};

    virtual std::shared_ptr<AudioDeviceDescriptor> GetMediaRenderDevice(StreamUsage streamUsage, int32_t clientUID) = 0;
    virtual std::shared_ptr<AudioDeviceDescriptor> GetCallRenderDevice(StreamUsage streamUsage, int32_t clientUID) = 0;
    virtual std::shared_ptr<AudioDeviceDescriptor> GetCallCaptureDevice(SourceType sourceType, int32_t clientUID) = 0;
    virtual vector<std::shared_ptr<AudioDeviceDescriptor>> GetRingRenderDevices(StreamUsage streamUsage,
        int32_t clientUID) = 0;
    virtual std::shared_ptr<AudioDeviceDescriptor> GetRecordCaptureDevice(SourceType sourceType, int32_t clientUID) = 0;
    virtual std::shared_ptr<AudioDeviceDescriptor> GetToneRenderDevice(StreamUsage streamUsage, int32_t clientUID) = 0;
    virtual RouterType GetRouterType() = 0;

    virtual std::string GetClassName()
    {
        return name_;
    }
    std::shared_ptr<AudioDeviceDescriptor> GetLatestNonExcludedConnectDevice(AudioDeviceUsage audioDevUsage,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs)
    {
        // remove abnormal device or excluded device
        for (size_t i = 0; i < descs.size(); i++) {
            if (descs[i]->exceptionFlag_ || !descs[i]->isEnable_ ||
                (descs[i]->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO && descs[i]->connectState_ == SUSPEND_CONNECTED) ||
                AudioStateManager::GetAudioStateManager().IsExcludedDevice(audioDevUsage, descs[i])) {
                descs.erase(descs.begin() + i);
                i--;
            }
        }
        if (descs.size() > 0) {
            auto compare = [&] (std::shared_ptr<AudioDeviceDescriptor> &desc1,
                std::shared_ptr<AudioDeviceDescriptor> &desc2) {
                return desc1->connectTimeStamp_ < desc2->connectTimeStamp_;
            };
            sort(descs.begin(), descs.end(), compare);
            return std::move(descs.back());
        }
        return std::make_shared<AudioDeviceDescriptor>();
    }

    std::shared_ptr<AudioDeviceDescriptor> GetPairDevice(std::shared_ptr<AudioDeviceDescriptor> &targetDevice,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &deviceList)
    {
        for (auto &device : deviceList) {
            if (device->deviceRole_ != targetDevice->deviceRole_ ||
                device->deviceType_ != targetDevice->deviceType_ ||
                device->networkId_ != targetDevice->networkId_ ||
                device->macAddress_ != targetDevice->macAddress_) {
                continue;
            }
            if (!device->exceptionFlag_ && device->isEnable_ &&
                (device->deviceType_ != DEVICE_TYPE_BLUETOOTH_SCO ||
                device->connectState_ != SUSPEND_CONNECTED) &&
                device->connectState_ != VIRTUAL_CONNECTED) {
                return std::move(device);
            }
            AUDIO_WARNING_LOG("unavailable device state, type[%{public}d] connectState[%{public}d] " \
                "isEnable[%{public}d] exceptionFlag[%{public}d]", device->deviceType_, device->connectState_,
                device->isEnable_, device->exceptionFlag_);
        }
        return std::make_shared<AudioDeviceDescriptor>();
    }
};
} // namespace AudioStandard
} // namespace OHOS

#endif // ST_ROUTER_BASE_H