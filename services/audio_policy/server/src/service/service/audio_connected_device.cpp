/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioConnectedDevice"
#endif

#include "audio_connected_device.h"
#include <ability_manager_client.h>
#include "iservice_registry.h"
#include "parameter.h"
#include "parameters.h"
#include "audio_policy_log.h"
#include "audio_manager_listener_stub.h"
#include "audio_inner_call.h"
#include "media_monitor_manager.h"
#include "audio_spatialization_service.h"


#include "audio_policy_utils.h"

namespace OHOS {
namespace AudioStandard {

bool AudioConnectedDevice::IsConnectedOutputDevice(const std::shared_ptr<AudioDeviceDescriptor> &desc)
{
    DeviceType deviceType = desc->deviceType_;

    CHECK_AND_RETURN_RET_LOG(desc->deviceRole_ == DeviceRole::OUTPUT_DEVICE, false,
        "Not output device!");

    auto isPresent = [&deviceType] (const std::shared_ptr<AudioDeviceDescriptor> &desc) {
        CHECK_AND_RETURN_RET_LOG(desc != nullptr, false, "Invalid device descriptor");
        if (deviceType == DEVICE_TYPE_FILE_SINK) {
            return false;
        }
        return ((deviceType == desc->deviceType_) && (desc->deviceRole_ == DeviceRole::OUTPUT_DEVICE));
    };

    auto itr = std::find_if(connectedDevices_.begin(), connectedDevices_.end(), isPresent);
    CHECK_AND_RETURN_RET_LOG(itr != connectedDevices_.end(), false, "Device not available");

    return true;
}

std::shared_ptr<AudioDeviceDescriptor> AudioConnectedDevice::CheckExistOutputDevice(DeviceType activeDevice,
    std::string macAddress)
{
    auto isOutputDevicePresent = [&activeDevice, &macAddress] (const std::shared_ptr<AudioDeviceDescriptor> &desc) {
        CHECK_AND_RETURN_RET_LOG(desc != nullptr, false, "Invalid device descriptor");
        if ((activeDevice == desc->deviceType_) && (OUTPUT_DEVICE == desc->deviceRole_)) {
            if (activeDevice == DEVICE_TYPE_BLUETOOTH_A2DP) {
                // If the device type is A2DP, need to compare mac address in addition.
                return desc->macAddress_ == macAddress;
            }
            return true;
        }
        return false;
    };

    auto itr = std::find_if(connectedDevices_.begin(), connectedDevices_.end(), isOutputDevicePresent);
    if (itr != connectedDevices_.end()) {
        return *itr;
    }
    return nullptr;
}

std::shared_ptr<AudioDeviceDescriptor> AudioConnectedDevice::CheckExistInputDevice(DeviceType activeDevice)
{
    auto isInputDevicePresent = [&activeDevice] (const std::shared_ptr<AudioDeviceDescriptor> &desc) {
        CHECK_AND_RETURN_RET_LOG(desc != nullptr, false, "Invalid device descriptor");
        return ((activeDevice == desc->deviceType_) && (INPUT_DEVICE == desc->deviceRole_));
    };

    auto itr = std::find_if(connectedDevices_.begin(), connectedDevices_.end(), isInputDevicePresent);
    if (itr != connectedDevices_.end()) {
        return *itr;
    }
    return nullptr;
}

std::shared_ptr<AudioDeviceDescriptor> AudioConnectedDevice::GetConnectedDeviceByType(int32_t deviceType)
{
    auto isPresent = [&deviceType] (const std::shared_ptr<AudioDeviceDescriptor> &desc) {
        if (deviceType == desc->deviceType_) {
            return true;
        }
        return false;
    };
    auto it = std::find_if(connectedDevices_.begin(), connectedDevices_.end(), isPresent);
    if (it != connectedDevices_.end()) {
        return *it;
    }
    return nullptr;
}

std::shared_ptr<AudioDeviceDescriptor> AudioConnectedDevice::GetConnectedDeviceByType(
    std::string networkId, DeviceType deviceType)
{
    auto isPresent = [&networkId, &deviceType] (const std::shared_ptr<AudioDeviceDescriptor> &desc) {
        if (deviceType == desc->deviceType_ && networkId == desc->networkId_) {
            return true;
        }
        return false;
    };
    auto it = std::find_if(connectedDevices_.begin(), connectedDevices_.end(), isPresent);
    if (it != connectedDevices_.end()) {
        return *it;
    }
    return nullptr;
}

std::shared_ptr<AudioDeviceDescriptor> AudioConnectedDevice::GetConnectedDeviceByType(
    std::string networkId, DeviceType deviceType, std::string macAddress)
{
    auto isPresent = [&networkId, &deviceType, &macAddress] (const std::shared_ptr<AudioDeviceDescriptor> &desc) {
        if (deviceType == desc->deviceType_ && networkId == desc->networkId_ && macAddress == desc->macAddress_) {
            return true;
        }
        return false;
    };
    auto it = std::find_if(connectedDevices_.begin(), connectedDevices_.end(), isPresent);
    if (it != connectedDevices_.end()) {
        return *it;
    }
    return nullptr;
}

void AudioConnectedDevice::GetAllConnectedDeviceByType(std::string networkId, DeviceType deviceType,
    std::string macAddress, DeviceRole deviceRole, std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descForCb)
{
    auto isPresent =
        [&networkId, &deviceType, &macAddress, &deviceRole] (const std::shared_ptr<AudioDeviceDescriptor> &desc) {
        if (deviceType == desc->deviceType_ && networkId == desc->networkId_ && macAddress == desc->macAddress_
            && (!IsUsb(desc->deviceType_) || deviceRole == desc->deviceRole_)) {
            return true;
        }
        return false;
    };
    auto it = std::find_if(connectedDevices_.begin(), connectedDevices_.end(), isPresent);
    while (it != connectedDevices_.end()) {
        descForCb.push_back(*it);
        it = std::find_if(std::next(it), connectedDevices_.end(), isPresent);
    }
    return;
}

void AudioConnectedDevice::DelConnectedDevice(std::string networkId, DeviceType deviceType, std::string macAddress,
    DeviceRole deviceRole)
{
    auto isPresent = [&deviceType, &networkId, &macAddress,
        &deviceRole] (const std::shared_ptr<AudioDeviceDescriptor> &descriptor) {
        return descriptor->deviceType_ == deviceType && descriptor->networkId_ == networkId
            && descriptor->macAddress_ == macAddress &&
            (!IsUsb(descriptor->deviceType_) || descriptor->deviceRole_ == deviceRole);
    };

    connectedDevices_.erase(std::remove_if(connectedDevices_.begin(), connectedDevices_.end(), isPresent),
        connectedDevices_.end());
    return;
}

void AudioConnectedDevice::DelConnectedDevice(std::string networkId, DeviceType deviceType, std::string macAddress)
{
    auto isPresent =
        [&deviceType, &networkId, &macAddress] (const std::shared_ptr<AudioDeviceDescriptor> &descriptor) {
        return descriptor->deviceType_ == deviceType && descriptor->networkId_ == networkId
            && descriptor->macAddress_ == macAddress;
    };

    connectedDevices_.erase(std::remove_if(connectedDevices_.begin(), connectedDevices_.end(), isPresent),
        connectedDevices_.end());
    return;
}

void AudioConnectedDevice::DelConnectedDevice(std::string networkId, DeviceType deviceType)
{
    auto isPresent = [&deviceType, &networkId] (const std::shared_ptr<AudioDeviceDescriptor> &descriptor) {
        return descriptor->deviceType_ == deviceType && descriptor->networkId_ == networkId;
    };

    connectedDevices_.erase(std::remove_if(connectedDevices_.begin(), connectedDevices_.end(), isPresent),
        connectedDevices_.end());
    return;
}

void AudioConnectedDevice::AddConnectedDevice(std::shared_ptr<AudioDeviceDescriptor> remoteDeviceDescriptor)
{
    connectedDevices_.insert(connectedDevices_.begin(), remoteDeviceDescriptor);
    return;
}

bool AudioConnectedDevice::CheckDeviceConnected(std::string selectedDevice)
{
    for (auto device : connectedDevices_) {
        if (AudioPolicyUtils::GetInstance().GetRemoteModuleName(device->networkId_, device->deviceRole_)
            == selectedDevice) {
            return true;
        }
    }
    return false;
}

void AudioConnectedDevice::SetDisplayName(const std::string &deviceName, bool isLocalDevice)
{
    for (const auto& deviceInfo : connectedDevices_) {
        if ((isLocalDevice && deviceInfo->networkId_ == LOCAL_NETWORK_ID) ||
            (!isLocalDevice && deviceInfo->networkId_ != LOCAL_NETWORK_ID)) {
            deviceInfo->displayName_ = deviceName;
        }
    }
}

void AudioConnectedDevice::SetDmDeviceType(const uint16_t dmDeviceType)
{
    for (const auto& deviceInfo : connectedDevices_) {
        if (deviceInfo->networkId_ != LOCAL_NETWORK_ID) {
            deviceInfo->dmDeviceType_ = dmDeviceType;
        }
    }
}

void AudioConnectedDevice::SetDisplayName(const std::string macAddress, const std::string deviceName)
{
    for (auto device : connectedDevices_) {
        if (device->macAddress_ == macAddress) {
            device->deviceName_ = deviceName;
            int32_t bluetoothId_ = device->deviceId_;
            std::string name_ = device->deviceName_;
            AUDIO_INFO_LOG("bluetoothId %{public}d alias name changing to %{public}s", bluetoothId_, name_.c_str());
        }
    }
}

void AudioConnectedDevice::UpdateConnectDevice(DeviceType deviceType, const std::string &macAddress,
    const std::string &deviceName, const AudioStreamInfo &streamInfo)
{
    auto isPresent = [&deviceType, &macAddress] (const std::shared_ptr<AudioDeviceDescriptor> &descriptor) {
        return descriptor->macAddress_ == macAddress && descriptor->deviceType_ == deviceType;
    };

    auto it = std::find_if(connectedDevices_.begin(), connectedDevices_.end(), isPresent);
    if (it != connectedDevices_.end()) {
        (*it)->deviceName_ = deviceName;
        (*it)->audioStreamInfo_ = streamInfo;
    }
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioConnectedDevice::GetDevicesInner(DeviceFlag deviceFlag)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceList = {};

    CHECK_AND_RETURN_RET_LOG(deviceFlag >= DeviceFlag::OUTPUT_DEVICES_FLAG &&
        deviceFlag <= DeviceFlag::ALL_L_D_DEVICES_FLAG,
        deviceList, "Invalid flag provided %{public}d", deviceFlag);

    CHECK_AND_RETURN_RET(deviceFlag != DeviceFlag::ALL_L_D_DEVICES_FLAG, connectedDevices_);

    for (auto device : connectedDevices_) {
        if (device == nullptr) {
            continue;
        }
        bool filterAllLocal = deviceFlag == DeviceFlag::ALL_DEVICES_FLAG && device->networkId_ == LOCAL_NETWORK_ID;
        bool filterLocalOutput = deviceFlag == DeviceFlag::OUTPUT_DEVICES_FLAG
            && device->networkId_ == LOCAL_NETWORK_ID
            && device->deviceRole_ == DeviceRole::OUTPUT_DEVICE;
        bool filterLocalInput = deviceFlag == DeviceFlag::INPUT_DEVICES_FLAG
            && device->networkId_ == LOCAL_NETWORK_ID
            && device->deviceRole_ == DeviceRole::INPUT_DEVICE;

        bool filterAllRemote = deviceFlag == DeviceFlag::ALL_DISTRIBUTED_DEVICES_FLAG
            && device->networkId_ != LOCAL_NETWORK_ID;
        bool filterRemoteOutput = deviceFlag == DeviceFlag::DISTRIBUTED_OUTPUT_DEVICES_FLAG
            && (device->networkId_ != LOCAL_NETWORK_ID || device->deviceType_ == DEVICE_TYPE_REMOTE_CAST)
            && device->deviceRole_ == DeviceRole::OUTPUT_DEVICE;
        bool filterRemoteInput = deviceFlag == DeviceFlag::DISTRIBUTED_INPUT_DEVICES_FLAG
            && device->networkId_ != LOCAL_NETWORK_ID
            && device->deviceRole_ == DeviceRole::INPUT_DEVICE;

        if (filterAllLocal || filterLocalOutput || filterLocalInput || filterAllRemote || filterRemoteOutput
            || filterRemoteInput) {
            std::shared_ptr<AudioDeviceDescriptor> devDesc = std::make_shared<AudioDeviceDescriptor>(*device);
            deviceList.push_back(devDesc);
        }
    }

    AUDIO_DEBUG_LOG("GetDevices list size = [%{public}zu]", deviceList.size());
    return deviceList;
}

DeviceType AudioConnectedDevice::FindConnectedHeadset()
{
    const auto& itr = std::find_if(connectedDevices_.begin(), connectedDevices_.end(),
        [](const std::shared_ptr<AudioDeviceDescriptor> &devDesc) {
        CHECK_AND_RETURN_RET_LOG(devDesc != nullptr, false, "Invalid device descriptor");
        return ((devDesc->deviceType_ == DEVICE_TYPE_WIRED_HEADSET) ||
            (devDesc->deviceType_ == DEVICE_TYPE_WIRED_HEADPHONES) ||
            (devDesc->deviceType_ == DEVICE_TYPE_USB_HEADSET) ||
            (devDesc->deviceType_ == DEVICE_TYPE_DP) ||
            (devDesc->deviceType_ == DEVICE_TYPE_USB_ARM_HEADSET) ||
            (devDesc->deviceType_ == DEVICE_TYPE_HDMI));
    });

    DeviceType retType = DEVICE_TYPE_NONE;
    if (itr != connectedDevices_.end()) {
        retType = (*itr)->deviceType_;
    }
    return retType;
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioConnectedDevice::GetCopy()
{
    return connectedDevices_;
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioConnectedDevice::GetDevicesForGroup(GroupType type,
    int32_t groupId)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices = {};
    for (auto devDes : connectedDevices_) {
        if (devDes == nullptr) {
            continue;
        }
        bool inVolumeGroup = type == VOLUME_TYPE && devDes->volumeGroupId_ == groupId;
        bool inInterruptGroup = type == INTERRUPT_TYPE && devDes->interruptGroupId_ == groupId;

        if (inVolumeGroup || inInterruptGroup) {
            std::shared_ptr<AudioDeviceDescriptor> device = std::make_shared<AudioDeviceDescriptor>(*devDes);
            devices.push_back(device);
        }
    }
    return devices;
}

bool AudioConnectedDevice::IsArmDevice(const std::string& address, const DeviceRole role)
{
    return std::any_of(connectedDevices_.begin(), connectedDevices_.end(),
        [&address, &role](const auto& item) {
            return (item->deviceType_ == DEVICE_TYPE_USB_ARM_HEADSET &&
                item->macAddress_ == address && item->deviceRole_ == role);
        });
}

bool AudioConnectedDevice::HasArm(const DeviceRole role)
{
    return std::find_if(connectedDevices_.cbegin(), connectedDevices_.cend(), [role](const auto& item) {
        return item->deviceType_ == DEVICE_TYPE_USB_ARM_HEADSET && item->deviceRole_ == role;
    }) != connectedDevices_.cend();
}

bool AudioConnectedDevice::HasHifi(const DeviceRole role)
{
    return std::find_if(connectedDevices_.cbegin(), connectedDevices_.cend(), [role](const auto& item) {
        return item->deviceType_ == DEVICE_TYPE_USB_HEADSET && item->deviceRole_ == role;
    }) != connectedDevices_.cend();
}

std::shared_ptr<AudioDeviceDescriptor> AudioConnectedDevice::GetUsbDeviceDescriptor(const std::string &address,
    const DeviceRole role)
{
    auto it = std::find_if(connectedDevices_.cbegin(), connectedDevices_.cend(), [&address, role](const auto &item) {
        return IsUsb(item->deviceType_) && item->macAddress_ == address && item->deviceRole_ == role;
    });
    if (it != connectedDevices_.cend()) {
        return *it;
    }
    return nullptr;
}

static std::string GetSha256EncryptAddress(const std::string& address)
{
    const int32_t HexWidth = 2;
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char *>(address.c_str()), address.size(), hash);
    std::stringstream ss;
    for (int32_t i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(HexWidth) << std::setfill('0') << (int32_t)hash[i];
    }
    return ss.str();
}

void AudioConnectedDevice::UpdateSpatializationSupported(const std::string macAddress, const bool support)
{
    for (auto device : connectedDevices_) {
        std::string encryAddress = GetSha256EncryptAddress(device->macAddress_);
        if (encryAddress == macAddress && device->deviceType_ ==  DEVICE_TYPE_BLUETOOTH_A2DP &&
            device->spatializationSupported_ != support) {
            device->spatializationSupported_ = support;
            AUDIO_INFO_LOG("spatializationSupported is set to %{public}d", support);
        }
    }
}
}
}