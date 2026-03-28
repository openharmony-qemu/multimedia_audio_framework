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
#define LOG_TAG "AudioSelectInterfaceService"
#endif

#include "audio_select_interface_service.h"

#include "audio_active_device.h"
#include "audio_core_service.h"
#include "audio_device_factory.h"
#include "audio_errors.h"
#include "audio_policy_utils.h"
#include "audio_router_center.h"
#include "audio_router_select_strategy.h"
#include "audio_server_proxy.h"

namespace OHOS {
namespace AudioStandard {
int32_t AudioSelectInterfaceService::SetDeviceActive(DeviceType deviceType, bool active,
    const std::string &address, const int32_t uid)
{
    return SUCCESS;
}

int32_t AudioSelectInterfaceService::SelectOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc, const int32_t audioDeviceSelectMode,
    const bool isNeedNotifyBt)
{
    return SUCCESS;
}

int32_t AudioSelectInterfaceService::SelectInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc)
{
    return SUCCESS;
}

int32_t AudioSelectInterfaceService::SelectInputDeviceByUid(const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc,
    int32_t uid)
{
    return SUCCESS;
}

int32_t AudioSelectInterfaceService::ExcludeOutputDevices(AudioDeviceUsage audioDevUsage,
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &deviceDescs)
{
    AUDIO_WARNING_LOG("audioDevUsage[%{public}d], Exclude devices list size [%{public}zu], %{public}s",
        audioDevUsage, deviceDescs.size(),
        AudioPolicyUtils::GetInstance().GetDevicesStr(deviceDescs).c_str());

    CHECK_AND_RETURN_RET_LOG(deviceDescs.size() > 0, ERR_INVALID_PARAM, "No device to exclude");
    for (const auto& deviceDesc : deviceDescs) {
        CHECK_AND_RETURN_RET_LOG(deviceDesc != nullptr, ERROR_INVALID_PARAM, "deviceDesc is null");
        AUDIO_INFO_LOG("deviceDesc->dmDeviceType_ %{public}d", deviceDesc->dmDeviceType_);
        if (deviceDesc->dmDeviceType_ == DM_DEVICE_TYPE_MUSIC_HOST) {
            std::string taskId = "{\"taskId\":\"0\"}";
            AudioServerProxy::GetInstance().NotifyTaskIdInfoProxy(taskId, false);
            AUDIO_INFO_LOG("taskId clean success!, is not connected");
        }
    }
    if (deviceDescs.front()->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO &&
        deviceDescs.front()->macAddress_.empty()) {
        AudioRouterSelectStrategy::GetInstance().SetScoExcluded(true);
        return SUCCESS;
    }

    int32_t ret = ExcludeOutputDevicesInner(audioDevUsage, deviceDescs);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Exclude devices failed");

    auto coreService = AudioCoreService::GetCoreService();
    CHECK_AND_RETURN_RET_LOG(coreService != nullptr, ERR_NULL_POINTER, "coreService is null");
    coreService->FetchOutputDeviceAndRoute("ExcludeOutputDevices",
        AudioStreamDeviceChangeReason::OVERRODE);
    coreService->FetchInputDeviceAndRoute("ExcludeOutputDevices");
    AudioDeviceDescriptor currentOutputDevice = AudioActiveDevice::GetInstance().GetCurrentOutputDevice();
    AudioDeviceDescriptor currentInputDevice = AudioActiveDevice::GetInstance().GetCurrentInputDevice();
    coreService->ReloadSourceForDeviceChange(
        currentInputDevice, currentOutputDevice, "ExcludeOutputDevices");

    if (audioA2dpOffloadManager_ != nullptr) {
        if ((currentOutputDevice.deviceType_ != DEVICE_TYPE_BLUETOOTH_A2DP) ||
            (currentOutputDevice.networkId_ != LOCAL_NETWORK_ID)) {
            audioA2dpOffloadManager_->UpdateA2dpOffloadFlagForA2dpDeviceOut();
        } else {
            audioA2dpOffloadManager_->UpdateA2dpOffloadFlagForAllStream(currentOutputDevice.deviceType_);
        }
    }

    for (const auto &deviceDesc : deviceDescs) {
        CHECK_AND_RETURN_RET_LOG(deviceDesc != nullptr, ERR_INVALID_PARAM, "Invalid device descriptor");
        coreService->NotifyRemoteDeviceStatusUpdate(deviceDesc);
    }
    coreService->OnPreferredOutputDeviceUpdated(currentOutputDevice,
        AudioStreamDeviceChangeReason::OVERRODE);
    return SUCCESS;
}

int32_t AudioSelectInterfaceService::ExcludeOutputDevicesInner(AudioDeviceUsage audioDevUsage,
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &deviceDescs)
{
    const std::string macAddress = deviceDescs.front()->macAddress_;
    auto allDevices = AudioDeviceManager::GetAudioDeviceManager().GetConnectedDevices();

    std::unordered_set<std::shared_ptr<AudioDeviceDescriptor>> excludeSet;
    if (!(audioDevUsage == ALL_MEDIA_DEVICES && !macAddress.empty())) {
        excludeSet.insert(deviceDescs.begin(), deviceDescs.end());
    }

    auto matchCondition = [&](const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc) -> bool {
        if (audioDevUsage == ALL_MEDIA_DEVICES && !macAddress.empty()) {
            // Find Sco device when exclude all media usage for A2dp device
            return !deviceDesc->macAddress_.empty() &&
                deviceDesc->macAddress_ == macAddress &&
                deviceDesc->deviceRole_ == OUTPUT_DEVICE;
        }
        return std::any_of(excludeSet.begin(), excludeSet.end(),
            [&](const auto &excludeDesc) {
                return deviceDesc->IsSameDeviceDescPtr(excludeDesc);
            });
    };

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescriptors;
    std::copy_if(allDevices.begin(), allDevices.end(),
        std::back_inserter(deviceDescriptors), matchCondition);

    AudioRouterSelectStrategy::GetInstance().ExcludeDevices(deviceDescriptors, audioDevUsage);
    for (const auto &deviceDesc : deviceDescriptors) {
        CHECK_AND_RETURN_RET_LOG(deviceDesc != nullptr, ERR_INVALID_PARAM, "Invalid device descriptor");
        ClearActiveHfpDevice(deviceDesc);
        AudioActiveDevice::GetInstance().NotifyUserDisSelectionEventToBt(deviceDesc);
    }
    return SUCCESS;
}

int32_t AudioSelectInterfaceService::ClearActiveHfpDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc)
{
    if (deviceDesc->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO) {
        auto &audioDeviceFactory = AudioDeviceFactory::GetInstance();
        auto deviceDescs = {deviceDesc};
        return audioDeviceFactory.DeactivateDevice(deviceDescs);
    }
    return SUCCESS;
}

int32_t AudioSelectInterfaceService::UnexcludeOutputDevices(AudioDeviceUsage audioDevUsage,
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &deviceDescs)
{
    AUDIO_WARNING_LOG("audioDevUsage[%{public}d], Unexclude devices list size [%{public}zu], %{public}s",
        audioDevUsage, deviceDescs.size(),
        AudioPolicyUtils::GetInstance().GetDevicesStr(deviceDescs).c_str());
    if (deviceDescs.front()->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO &&
        deviceDescs.front()->macAddress_.empty()) {
        AudioRouterSelectStrategy::GetInstance().SetScoExcluded(false);
        return SUCCESS;
    }
    int32_t ret = UnexcludeOutputDevicesInner(audioDevUsage, deviceDescs);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Unexclude devices failed");

    auto coreService = AudioCoreService::GetCoreService();
    CHECK_AND_RETURN_RET_LOG(coreService != nullptr, ERR_NULL_POINTER, "coreService is null");
    coreService->FetchOutputDeviceAndRoute("UnexcludeOutputDevices",
        AudioStreamDeviceChangeReason::OVERRODE);
    coreService->FetchInputDeviceAndRoute("UnexcludeOutputDevices");
    AudioDeviceDescriptor currentOutputDevice = AudioActiveDevice::GetInstance().GetCurrentOutputDevice();
    AudioDeviceDescriptor currentInputDevice = AudioActiveDevice::GetInstance().GetCurrentInputDevice();
    coreService->ReloadSourceForDeviceChange(
        currentInputDevice, currentOutputDevice, "UnexcludeOutputDevices");
    if (audioA2dpOffloadManager_ != nullptr) {
        if ((currentOutputDevice.deviceType_ != DEVICE_TYPE_BLUETOOTH_A2DP) ||
            (currentOutputDevice.networkId_ != LOCAL_NETWORK_ID)) {
            audioA2dpOffloadManager_->UpdateA2dpOffloadFlagForA2dpDeviceOut();
        } else {
            audioA2dpOffloadManager_->UpdateA2dpOffloadFlagForAllStream(currentOutputDevice.deviceType_);
        }
    }
    return SUCCESS;
}

int32_t AudioSelectInterfaceService::UnexcludeOutputDevicesInner(AudioDeviceUsage audioDevUsage,
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &deviceDescs)
{
    CHECK_AND_RETURN_RET_LOG(deviceDescs.size() > 0, ERR_INVALID_PARAM, "No device to exclude");
    AudioRouterSelectStrategy::GetInstance().UnexcludeDevices(deviceDescs, audioDevUsage);
    return SUCCESS;
}

int32_t AudioSelectInterfaceService::SetSessionDefaultOutputDevice(const int32_t callerPid,
    const DeviceType &deviceType)
{
    return SUCCESS;
}

int32_t AudioSelectInterfaceService::SetDefaultOutputDevice(
    const DeviceType deviceType, const uint32_t sessionID, const StreamUsage streamUsage, bool isRunning,
    bool skipForce)
{
    return SUCCESS;
}

int32_t AudioSelectInterfaceService::SetInputDevice(const DeviceType deviceType, const uint32_t sessionID,
    const SourceType sourceType, bool isRunning)
{
    return SUCCESS;
}

int32_t AudioSelectInterfaceService::PreferBluetoothAndNearlinkRecordByUid(int32_t uid,
    BluetoothAndNearlinkPreferredRecordCategory category)
{
    return SUCCESS;
}

void AudioSelectInterfaceService::SetPreferredInputDeviceIfValid(std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    return;
}

void AudioSelectInterfaceService::OnForcedDeviceSelected(DeviceType devType, const std::string &macAddress,
    sptr<AudioRendererFilter> filter, const std::string &caller)
{
    return;
}

void AudioSelectInterfaceService::OnPrivacyDeviceSelected(DeviceType devType, const std::string &macAddress,
    const std::string &caller)
{
    return;
}

int32_t AudioSelectInterfaceService::SetA2dpDeviceOffloadManager(
    std::shared_ptr<AudioA2dpOffloadManager> audioA2dpOffloadManager)
{
    CHECK_AND_RETURN_RET_LOG(audioA2dpOffloadManager != nullptr, ERR_INVALID_PARAM,
        "audioA2dpOffloadManager is nullptr");
    audioA2dpOffloadManager_ = audioA2dpOffloadManager;
    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS
