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
#include "audio_policy_utils.h"

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
    virtual std::shared_ptr<AudioDeviceDescriptor> GetCallCaptureDevice(SourceType sourceType, int32_t clientUID,
        const uint32_t sessionID = 0) = 0;
    virtual vector<std::shared_ptr<AudioDeviceDescriptor>> GetRingRenderDevices(StreamUsage streamUsage,
        int32_t clientUID) = 0;
    virtual std::shared_ptr<AudioDeviceDescriptor> GetRecordCaptureDevice(SourceType sourceType, int32_t clientUID,
        const uint32_t sessionID = 0) = 0;
    virtual std::shared_ptr<AudioDeviceDescriptor> GetToneRenderDevice(StreamUsage streamUsage, int32_t clientUID) = 0;
    virtual RouterType GetRouterType() = 0;

    virtual std::string GetClassName()
    {
        return name_;
    }

    bool IsDeviceUsageSupported(AudioDeviceUsage audioDevUsage,
        const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc)
    {
        CHECK_AND_RETURN_RET_LOG(deviceDesc != nullptr, false, "deviceDesc is nullptr");
        uint32_t deviceUsage = static_cast<uint32_t>(deviceDesc->deviceUsage_);
        switch (audioDevUsage) {
            case MEDIA_OUTPUT_DEVICES:
            case MEDIA_INPUT_DEVICES:
                return (deviceUsage & MEDIA) != 0;
            case CALL_OUTPUT_DEVICES:
            case CALL_INPUT_DEVICES:
                return (deviceUsage & VOICE) != 0;
            default:
                return false;
        }
    }

    std::shared_ptr<AudioDeviceDescriptor> GetLatestNonExcludedConnectDevice(AudioDeviceUsage audioDevUsage,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs)
    {
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> filteredDescs;
        // remove abnormal device or excluded device
        for (const auto &desc : descs) {
            CHECK_AND_CONTINUE(desc != nullptr);
            if (desc->exceptionFlag_ || !desc->isEnable_ ||
                (desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO &&
                (desc->connectState_ == SUSPEND_CONNECTED || AudioPolicyUtils::GetInstance().GetScoExcluded())) ||
                AudioStateManager::GetAudioStateManager().IsExcludedDevice(audioDevUsage, desc) ||
                !IsDeviceUsageSupported(audioDevUsage, desc)) {
                continue;
            }
            CHECK_AND_CONTINUE(!ExistSameRemoteCarWithA2DP(desc));
            filteredDescs.push_back(desc);
        }
        if (filteredDescs.size() > 0) {
            auto compare = [&] (std::shared_ptr<AudioDeviceDescriptor> &desc1,
                std::shared_ptr<AudioDeviceDescriptor> &desc2) {
                return desc1->connectTimeStamp_ < desc2->connectTimeStamp_;
            };
            sort(filteredDescs.begin(), filteredDescs.end(), compare);
            return std::move(filteredDescs.back());
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
                device->macAddress_ != targetDevice->macAddress_ ||
                ((device->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP || device->deviceType_ == DEVICE_TYPE_NEARLINK) &&
                AudioDeviceManager::GetAudioDeviceManager().GetScoState())) {
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

    bool NeedLatestConnectWithDefaultDevices(DeviceType type)
    {
        return type == DEVICE_TYPE_WIRED_HEADSET ||
            type == DEVICE_TYPE_WIRED_HEADPHONES ||
            type == DEVICE_TYPE_BLUETOOTH_SCO ||
            type == DEVICE_TYPE_USB_HEADSET ||
            type == DEVICE_TYPE_BLUETOOTH_A2DP ||
            type == DEVICE_TYPE_USB_ARM_HEADSET ||
            type == DEVICE_TYPE_HEARING_AID ||
            type == DEVICE_TYPE_NEARLINK;
    }

    bool ExistSameRemoteCarWithA2DP(std::shared_ptr<AudioDeviceDescriptor> desc)
    {
        CHECK_AND_RETURN_RET_LOG(desc != nullptr, false, "desc is nullptr");
        if (desc->deviceType_ != DEVICE_TYPE_BLUETOOTH_A2DP) {
            return false;
        }
        if (desc->deviceCategory_ != BT_CAR) {
            return false;
        }
        return AudioDeviceManager::GetAudioDeviceManager().ExistSameRemoteDeviceByMacAddress(desc);
    }
};
} // namespace AudioStandard
} // namespace OHOS

#endif // ST_ROUTER_BASE_H
