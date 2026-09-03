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

#include "audio_device_manager.h"
#include "audio_policy_manager_factory.h"
#include "audio_policy_log.h"
#include "audio_state_manager.h"
#include "audio_policy_utils.h"
#include "audio_router_select_strategy.h"
#include "conflict_handler.h"
#include "audio_bluetooth_manager.h"

namespace OHOS {
namespace AudioStandard {
class RouterBase {
public:
    std::string name_;
    IAudioPolicyInterface& audioPolicyManager_;
    RouterBase() : audioPolicyManager_(AudioPolicyManagerFactory::GetAudioPolicyManager()) {}
    virtual ~RouterBase() {};

    virtual std::shared_ptr<AudioDeviceDescriptor> GetMediaRenderDevice(StreamUsage streamUsage, int32_t clientUID,
        const uint32_t sessionID = INVALID_STREAM_ID) = 0;
    virtual std::shared_ptr<AudioDeviceDescriptor> GetCallRenderDevice(StreamUsage streamUsage, int32_t clientUID,
        const uint32_t sessionID = INVALID_STREAM_ID) = 0;
    virtual std::shared_ptr<AudioDeviceDescriptor> GetCallCaptureDevice(SourceType sourceType, int32_t clientUID,
        const uint32_t sessionID = INVALID_STREAM_ID) = 0;
    virtual vector<std::shared_ptr<AudioDeviceDescriptor>> GetRingRenderDevices(StreamUsage streamUsage,
        int32_t clientUID) = 0;
    virtual std::shared_ptr<AudioDeviceDescriptor> GetRecordCaptureDevice(SourceType sourceType, int32_t clientUID,
        const uint32_t sessionID = INVALID_STREAM_ID) = 0;
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
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs, StreamUsage streamUsage = STREAM_USAGE_UNKNOWN)
    {
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> filteredDescs;
        // remove abnormal device or excluded device
        for (const auto &desc : descs) {
            CHECK_AND_CONTINUE(desc != nullptr &&
                !AudioRouterSelectStrategy::GetInstance().IsDeviceTypeSkipped(desc->deviceType_));
            if (desc->exceptionFlag_ || !desc->isEnable_ ||
                (desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO &&
                    (desc->connectState_ == SUSPEND_CONNECTED ||
                        AudioRouterSelectStrategy::GetInstance().GetScoExcluded())) ||
                AudioRouterSelectStrategy::GetInstance().IsDeviceExcluded(desc, audioDevUsage) ||
                !IsDeviceUsageSupported(audioDevUsage, desc) ||
                IsNeedSilentControl(desc, streamUsage)) {
                continue;
            }
            CHECK_AND_CONTINUE(!ExistSameRemoteCarWithA2DP(desc));
            CHECK_AND_CONTINUE(!CheckDpStreamUsage(desc, streamUsage));
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

    // Check whether stream usage is special (call-related): voice, video, or modem call
    bool IsSpecialStreamUsage(StreamUsage streamUsage)
    {
        static const std::set<StreamUsage> specialStreamUsages = {
            STREAM_USAGE_VOICE_COMMUNICATION,
            STREAM_USAGE_VIDEO_COMMUNICATION,
            STREAM_USAGE_VOICE_MODEM_COMMUNICATION
        };
        return specialStreamUsages.find(streamUsage) != specialStreamUsages.end();
    }

    // Check whether device needs silent control filtering: wireless + silent + non-special stream
    bool IsNeedSilentControl(const std::shared_ptr<AudioDeviceDescriptor> &device, StreamUsage streamUsage)
    {
        CHECK_AND_RETURN_RET_LOG(device != nullptr, false, "device is nullptr");
        if (!AudioPolicyUtils::GetInstance().IsWirelessDevice(device->deviceType_)) {
            return false;  // Non-wireless devices are not subject to silent control
        }
        if (!device->isSilent_) {
            return false;  // Device is not silent, no need to filter
        }
        if (IsSpecialStreamUsage(streamUsage)) {
            return false;  // Special call streams are not subject to silent control
        }
        return true;  // Wireless + silent + non-special = needs filtering
    }

    std::shared_ptr<AudioDeviceDescriptor> GetPairDevice(std::shared_ptr<AudioDeviceDescriptor> &targetDevice,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &deviceList, AudioDeviceUsage audioDevUsage)
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
                device->connectState_ != VIRTUAL_CONNECTED &&
                IsDeviceUsageSupported(audioDevUsage, device) &&
                !(Util::IsEnhancedRoutingSupported() &&
                    AudioRouterSelectStrategy::GetInstance().IsDeviceExcluded(device, audioDevUsage))) {
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

    bool CheckDpStreamUsage(std::shared_ptr<AudioDeviceDescriptor> desc, StreamUsage streamUsage)
    {
        // Wired cast scenarios, ALARM, NOTIFICATION and ENFORCED_TONE should be played on the local device according to
        // the rules.
        CHECK_AND_RETURN_RET(desc != nullptr && desc->getType() == DEVICE_TYPE_DP &&
            audioPolicyManager_.IsDPCastingConnect(), false);
        CHECK_AND_RETURN_RET(streamUsage == STREAM_USAGE_NOTIFICATION || streamUsage == STREAM_USAGE_ENFORCED_TONE ||
            streamUsage == STREAM_USAGE_ALARM || streamUsage == STREAM_USAGE_VOICE_ASSISTANT, false);

        AUDIO_INFO_LOG("Wired cast, ALARM, NOTIFICATION, ENFORCED_TONE and VOICE_ASSISTANT sound on local device");
        return true;
    }

    void RegisterConflictHandler(std::shared_ptr<ConflictHandler> handler)
    {
        handlers.push_back(handler);
    }

    void ClearConflictHandler()
    {
        handlers.clear();
    }

    void ClearConflictDevice(std::vector<std::shared_ptr<AudioDeviceDescriptor>> &deviceList, RouterType routerType)
    {
        for (auto iter = deviceList.begin(); iter != deviceList.end();) {
            bool hasConflict = false;
            for (auto &handler : handlers) {
                CHECK_AND_CONTINUE(handler != nullptr);
                if (handler->Detect(*iter, routerType)) {
                    hasConflict = true;
                    break;
                }
            }
            if (hasConflict) {
                iter = deviceList.erase(iter);
            } else {
                ++iter;
            }
        }
    }

    void ReplaceConflictA2dpWithSco(std::shared_ptr<AudioDeviceDescriptor> &perDev, RouterType routerType)
    {
        CHECK_AND_RETURN(perDev != nullptr);
        for (auto &handler : handlers) {
            CHECK_AND_CONTINUE(handler != nullptr);
            auto highestPriorityDevice = handler->GetHighestPriorityDevice();
            CHECK_AND_CONTINUE(highestPriorityDevice != nullptr);
            if (highestPriorityDevice->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO &&
                perDev->macAddress_ == highestPriorityDevice->macAddress_) {
                auto scoDevice = AudioDeviceManager::GetAudioDeviceManager().GetActiveScoDevice(
                    perDev->macAddress_, OUTPUT_DEVICE);
                CHECK_AND_RETURN(scoDevice != nullptr && scoDevice->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO);
                perDev = scoDevice;
                return;
            }
        }
    }

    void AppendActiveScoDevice(std::vector<std::shared_ptr<AudioDeviceDescriptor>> &deviceList)
    {
        CHECK_AND_RETURN(handlers.size() != 0);
        for (auto &handler : handlers) {
            CHECK_AND_CONTINUE(handler != nullptr);
            std::shared_ptr<AudioDeviceDescriptor> highestPriorityDevice = handler->GetHighestPriorityDevice();
            CHECK_AND_CONTINUE(highestPriorityDevice != nullptr);
            std::shared_ptr<AudioDeviceDescriptor> activeScoDevice =
                AudioDeviceManager::GetAudioDeviceManager().GetActiveScoDevice(highestPriorityDevice->macAddress_,
                    OUTPUT_DEVICE);
            CHECK_AND_CONTINUE(activeScoDevice != nullptr && activeScoDevice->deviceType_ != DEVICE_TYPE_NONE);
            deviceList.push_back(activeScoDevice);
        }
    }

private:
    std::vector<std::shared_ptr<ConflictHandler>> handlers;
};
} // namespace AudioStandard
} // namespace OHOS

#endif // ST_ROUTER_BASE_H
