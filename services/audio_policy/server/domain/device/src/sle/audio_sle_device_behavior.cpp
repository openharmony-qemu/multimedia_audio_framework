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
#define LOG_TAG "SleAudioDeviceManager"
#endif

#include "audio_sle_device_behavior.h"

#include "audio_active_device.h"
#include "audio_core_service.h"
#include "audio_device_manager.h"
#include "audio_policy_utils.h"
#include "sle_audio_device_manager.h"


namespace OHOS {
namespace AudioStandard {
namespace {
static const int32_t REFETCH_DEVICE = 4;
constexpr int32_t REMOTE_USER_TERMINATED = 200;
constexpr int32_t DUAL_CONNECTION_FAILURE = 201;
} // namespace
int32_t AudioSleDeviceBehavior::ActivateDevice(const std::shared_ptr<AudioStreamDescriptor> &streamDesc,
    const AudioStreamDeviceChangeReasonExt reason)
{
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr, ERR_INVALID_PARAM, "Stream desc is nullptr");
    auto realUid = streamDesc->GetRealUid();

    CHECK_AND_RETURN_RET_LOG(!streamDesc->newDeviceDescs_.empty(), ERR_INVALID_PARAM, "newDeviceDescs_ is empty");
    auto deviceDesc = streamDesc->newDeviceDescs_.front();
    CHECK_AND_RETURN_RET_LOG(deviceDesc != nullptr, ERR_INVALID_PARAM, "Device desc is nullptr");

    std::variant<StreamUsage, SourceType> audioStreamConfig;
    bool isRunning = streamDesc->streamStatus_ == STREAM_STATUS_STARTED && !streamDesc->isStandby_;
    bool isVoiceType = true;
    bool isLoopback = false;
    if (streamDesc->audioMode_ == AUDIO_MODE_PLAYBACK) {
        audioStreamConfig = streamDesc->rendererInfo_.streamUsage;
        isVoiceType = AudioPolicyUtils::GetInstance().IsVoiceStreamType(streamDesc->rendererInfo_.streamUsage);
        isLoopback = streamDesc->rendererInfo_.isLoopback;
    } else {
        audioStreamConfig = streamDesc->capturerInfo_.sourceType;
        isVoiceType = AudioPolicyUtils::GetInstance().IsVoiceSourceType(streamDesc->capturerInfo_.sourceType);
    }

    auto device = SleAudioDeviceManager::GetInstance().GetActivatedDevice();
    if (device != "" && device != deviceDesc->macAddress_) {
        AUDIO_INFO_LOG("Another nearlink device[%{public}s] is already activated",
            AudioPolicyUtils::GetInstance().GetEncryptAddr(device).c_str());
        SleAudioDeviceManager::GetInstance().ResetSleStreamTypeCount(device);
        return SUCCESS;
    }

    auto runDeviceActivationFlow = [&deviceDesc, &isRunning, &realUid, &isLoopback](auto &&config) -> int32_t {
        int32_t ret = SleAudioDeviceManager::GetInstance().SetActiveDevice(*deviceDesc, config, realUid, isLoopback);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Activating Nearlink device fails");
        CHECK_AND_RETURN_RET_LOG(isRunning, ret, "Stream is not running, no needs start playing");
        return SleAudioDeviceManager::GetInstance().StartPlaying(*deviceDesc, config, realUid, isLoopback);
    };

    int32_t result = std::visit(runDeviceActivationFlow, audioStreamConfig);
    if (result != SUCCESS) {
        AUDIO_ERR_LOG("Nearlink device activation failed, macAddress: %{public}s, result: %{public}d",
            AudioPolicyUtils::GetInstance().GetEncryptAddr(deviceDesc->macAddress_).c_str(), result);
        HandleNearlinkErrResult(result, deviceDesc, isVoiceType);
        return REFETCH_DEVICE;
    }
    SleAudioDeviceManager::GetInstance().UpdateSleStreamTypeCount(streamDesc);
    return SUCCESS;
}

int32_t AudioSleDeviceBehavior::DeactivateDevice()
{
    auto device = SleAudioDeviceManager::GetInstance().GetActivatedDevice();
    if (device != "") {
        AUDIO_INFO_LOG("Reset nearlink device state, macAddress: %{public}s",
            AudioPolicyUtils::GetInstance().GetEncryptAddr(device).c_str());
        SleAudioDeviceManager::GetInstance().ResetSleStreamTypeCount(device);
    }
    return SUCCESS;
}

int32_t AudioSleDeviceBehavior::DeactivateDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc)
{
    return SUCCESS;
}

void AudioSleDeviceBehavior::HandleNearlinkErrResult(int32_t result, shared_ptr<AudioDeviceDescriptor> devDesc,
    bool isVoiceType)
{
    CHECK_AND_RETURN(result != SUCCESS);
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    if (result == REMOTE_USER_TERMINATED) {
        auto deviceDescriptor = make_shared<AudioDeviceDescriptor>(devDesc);
        AUDIO_INFO_LOG("Set connect state to SUSPEND_CONNECTED");
        deviceDescriptor->connectState_ = SUSPEND_CONNECTED;
        deviceDescriptor->deviceType_ = DEVICE_TYPE_NEARLINK;
        AudioDeviceManager::GetAudioDeviceManager().UpdateDevicesListInfo(
            deviceDescriptor, CONNECTSTATE_UPDATE, reason);
        deviceDescriptor->deviceType_ = DEVICE_TYPE_NEARLINK_IN;
        AudioDeviceManager::GetAudioDeviceManager().UpdateDevicesListInfo(
            deviceDescriptor, CONNECTSTATE_UPDATE, reason);
    } else if (result == DUAL_CONNECTION_FAILURE) {
        if (isVoiceType) {
            devDesc->deviceUsage_ = static_cast<DeviceUsage>(static_cast<uint32_t>(devDesc->deviceUsage_) &
                ~static_cast<uint32_t>(DeviceUsage::VOICE));
        } else {
            devDesc->deviceUsage_ = static_cast<DeviceUsage>(static_cast<uint32_t>(devDesc->deviceUsage_) &
                ~static_cast<uint32_t>(DeviceUsage::MEDIA));
        }
        AudioDeviceManager::GetAudioDeviceManager().UpdateDevicesListInfo(devDesc, USAGE_UPDATE, reason);
    } else {
        devDesc->exceptionFlag_ = true;
        AudioDeviceManager::GetAudioDeviceManager().UpdateDevicesListInfo(
            devDesc, EXCEPTION_FLAG_UPDATE, reason);
    }
}
} // namespace AudioStandard
} // namespace OHOS
