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
#define LOG_TAG "AudioDeviceFactory"
#endif

#include "audio_device_factory.h"

#include "audio_arm_device_behavior.h"
#include "audio_a2dp_device_behavior.h"
#include "audio_errors.h"
#include "audio_hearing_aid_device_behavior.h"
#include "audio_log.h"
#include "audio_sco_device_behavior.h"
#include "audio_sle_device_behavior.h"
#include "stream_dfx_manager.h"
#include "ipc_skeleton.h"
#include "sle_audio_device_manager.h"

namespace OHOS {
namespace AudioStandard {
std::shared_ptr<IDeviceBehaviorBase> AudioDeviceFactory::CreateDeviceBehavior(DeviceType deviceType)
{
    switch (deviceType) {
        case DEVICE_TYPE_BLUETOOTH_SCO:
            return std::make_shared<AudioScoDeviceBehavior>();
        case DEVICE_TYPE_BLUETOOTH_A2DP:
            return std::make_shared<AudioA2dpDeviceBehavior>();
        case DEVICE_TYPE_NEARLINK:
        case DEVICE_TYPE_NEARLINK_IN:
            return std::make_shared<AudioSleDeviceBehavior>();
        case DEVICE_TYPE_USB_ARM_HEADSET:
            return std::make_shared<AudioArmDeviceBehavior>();
        case DEVICE_TYPE_HEARING_AID:
            return std::make_shared<AudioHearingAidDeviceBehavior>();
        default:
            return nullptr;
    }
}

int32_t AudioDeviceFactory::ActivateDevice(const std::shared_ptr<AudioStreamDescriptor> &streamDesc,
    const AudioStreamDeviceChangeReasonExt reason)
{
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr, ERR_NULL_POINTER, "streamDesc is nullptr");
    CHECK_AND_RETURN_RET_LOG(!streamDesc->newDeviceDescs_.empty(), ERR_INVALID_PARAM, "newDeviceDescs_ is empty");
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = streamDesc->newDeviceDescs_.front();
    CHECK_AND_RETURN_RET_LOG(deviceDesc != nullptr, ERR_NULL_POINTER, "deviceDesc is nullptr");
    DeviceType deviceType = deviceDesc->deviceType_;

    std::lock_guard<std::mutex> lock(cacheMutex_);
    auto it = deviceBehaviorCache_.find(deviceType);
    std::shared_ptr<IDeviceBehaviorBase> deviceBehavior;

    if (it != deviceBehaviorCache_.end()) {
        deviceBehavior = it->second;
    } else {
        deviceBehavior = CreateDeviceBehavior(deviceType);
        if (deviceBehavior == nullptr) {
            // If device behavior is not initialized, return success directly,
            // because device can be activated without specific behavior.
            return SUCCESS;
        }
        deviceBehaviorCache_[deviceType] = deviceBehavior;
    }
    if (streamDesc->streamStatus_ == STREAM_STATUS_STARTED) {
        HandleDeviceConflictLocked(deviceType);
    }
    auto result = deviceBehavior->ActivateDevice(streamDesc, reason);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(result == SUCCESS, result,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(IPCSkeleton::GetCallingUid(),
            ERR_DEVICE_SWITCH_OPERATION_FAILED, "Active device failded", false),
        "ActivateDevice failed");
    SleAudioDeviceManager::GetInstance().UpdateSleStreamTypeCount(streamDesc);
    activatedDevices_[deviceDesc->deviceId_] = deviceDesc;
    return result;
}

void AudioDeviceFactory::HandleDeviceConflictLocked(DeviceType deviceType)
{
    if (deviceType == DEVICE_TYPE_NEARLINK || deviceType == DEVICE_TYPE_NEARLINK_IN) {
        DeactivateDeviceLocked(DEVICE_TYPE_BLUETOOTH_A2DP);
        DeactivateDeviceLocked(DEVICE_TYPE_BLUETOOTH_SCO);
    } else if (deviceType == DEVICE_TYPE_BLUETOOTH_A2DP || deviceType == DEVICE_TYPE_BLUETOOTH_SCO) {
        DeactivateDeviceLocked(DEVICE_TYPE_NEARLINK);
    }
}

int32_t AudioDeviceFactory::DeactivateDeviceLocked(DeviceType deviceType)
{
    auto it = deviceBehaviorCache_.find(deviceType);
    if (it == deviceBehaviorCache_.end() || it->second == nullptr) {
        return SUCCESS;
    }

    auto result = it->second->DeactivateDevice();
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, result, "DeactivateDevice failed");

    for (auto devIt = activatedDevices_.begin(); devIt != activatedDevices_.end();) {
        if (devIt->second->deviceType_ == deviceType) {
            devIt = activatedDevices_.erase(devIt);
        } else {
            ++devIt;
        }
    }
    return SUCCESS;
}

int32_t AudioDeviceFactory::DeactivateDevice(DeviceType deviceType)
{
    std::lock_guard<std::mutex> lock(cacheMutex_);
    return DeactivateDeviceLocked(deviceType);
}

int32_t AudioDeviceFactory::DeactivateDevice(const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &deviceDescs)
{
    CHECK_AND_RETURN_RET_LOG(!deviceDescs.empty(), ERR_INVALID_PARAM, "deviceDescs is empty");
    std::lock_guard<std::mutex> lock(cacheMutex_);
    for (const auto &deviceDesc : deviceDescs) {
        auto it = deviceBehaviorCache_.find(deviceDesc->deviceType_);
        CHECK_AND_CONTINUE_LOG(!(it == deviceBehaviorCache_.end() || it->second == nullptr),
            "DeviceType: %{public}d is not activated", deviceDesc->deviceType_);
        auto result = it->second->DeactivateDevice(deviceDesc);
        CHECK_AND_CONTINUE_LOG(result == SUCCESS, "DeactivateDevice failed");
        activatedDevices_.erase(deviceDesc->deviceId_);
    }
    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS
