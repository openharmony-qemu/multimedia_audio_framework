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
#ifndef ST_AUDIO_CONNECTED_DEVICE_H
#define ST_AUDIO_CONNECTED_DEVICE_H

#include <bitset>
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <shared_mutex>
#include "singleton.h"
#include "audio_group_handle.h"
#include "audio_manager_base.h"
#include "audio_module_info.h"
#include "audio_volume_config.h"
#include "audio_errors.h"
#include "audio_device_descriptor.h"

namespace OHOS {
namespace AudioStandard {

struct DmDevice {
    std::string deviceName_;
    std::string networkId_;
    uint16_t dmDeviceType_{0};
};

class AudioConnectedDevice {
public:
    static AudioConnectedDevice& GetInstance()
    {
        static AudioConnectedDevice instance;
        return instance;
    }
    void UpdateConnectDevice(DeviceType deviceType, const std::string &macAddress,
        const std::string &deviceName, const AudioStreamInfo &streamInfo);
    bool IsConnectedOutputDevice(const std::shared_ptr<AudioDeviceDescriptor> &desc);
    bool CheckDeviceConnected(std::string selectedDevice);
    void SetDisplayName(const std::string &deviceName, bool isLocalDevice);
    void UpdateDmDeviceMap(DmDevice &&dmDevice, bool isConnect);
    void UpdateDeviceDesc4DmDevice(AudioDeviceDescriptor &deviceDesc);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetDevicesInner(DeviceFlag deviceFlag);
    std::shared_ptr<AudioDeviceDescriptor> GetConnectedDeviceByType(int32_t deviceType);
    std::shared_ptr<AudioDeviceDescriptor> GetConnectedDeviceByType(std::string networkId, DeviceType deviceType);
    std::shared_ptr<AudioDeviceDescriptor> GetConnectedDeviceByType(std::string networkId, DeviceType deviceType,
        std::string macAddress);
    void GetAllConnectedDeviceByType(std::string networkId, DeviceType deviceType,
        std::string macAddress, DeviceRole deviceRole, std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descForCb);
    void DelConnectedDevice(std::string networkId, DeviceType deviceType);
    void DelConnectedDevice(std::string networkId, DeviceType deviceType, std::string macAddress);
    void DelConnectedDevice(std::string networkId, DeviceType deviceType, std::string macAddress,
        DeviceRole deviceRole);
    void AddConnectedDevice(std::shared_ptr<AudioDeviceDescriptor> remoteDeviceDescriptor);
    DeviceType FindConnectedHeadset();
    void SetDisplayName(const std::string macAddress, const std::string deviceName);
    std::shared_ptr<AudioDeviceDescriptor> CheckExistInputDevice(DeviceType activeDevice);
    std::shared_ptr<AudioDeviceDescriptor> CheckExistOutputDevice(DeviceType activeDevice, std::string macAddress);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetCopy();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetDevicesForGroup(GroupType type, int32_t groupId);
    bool HasArm(const DeviceRole role);
    bool HasHifi(const DeviceRole role);
    bool IsArmDevice(const std::string &address, const DeviceRole role);
    std::shared_ptr<AudioDeviceDescriptor> GetUsbDeviceDescriptor(const std::string &address, const DeviceRole role);
    void UpdateSpatializationSupported(const std::string macAddress, const bool support);
    void RegisterNameMonitorHelper();
    bool IsEmpty();
    std::shared_ptr<AudioDeviceDescriptor> GetDeviceByDeviceType(DeviceType type,
        std::string networkId = LOCAL_NETWORK_ID);
private:
    AudioConnectedDevice() {}
    ~AudioConnectedDevice() {}
private:
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> connectedDevices_;
    std::mutex dmDeviceMtx_;
    std::unordered_map<std::string, DmDevice> dmDeviceMap_;
    std::shared_mutex mutex_;
    std::shared_ptr<AudioDeviceDescriptor> defaultOutputDevice_ =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
};
}
}
#endif