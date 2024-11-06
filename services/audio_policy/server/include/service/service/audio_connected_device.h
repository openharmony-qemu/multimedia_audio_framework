/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#include "singleton.h"
#include "audio_group_handle.h"
#include "audio_info.h"
#include "audio_manager_base.h"
#include "audio_module_info.h"
#include "audio_volume_config.h"
#include "audio_utils.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {

class AudioConnectedDevice {
public:
    static AudioConnectedDevice& GetInstance()
    {
        static AudioConnectedDevice instance;
        return instance;
    }
    void UpdateConnectDevice(DeviceType deviceType, const std::string &macAddress,
        const std::string &deviceName, const AudioStreamInfo &streamInfo);
    bool IsConnectedOutputDevice(const sptr<AudioDeviceDescriptor> &desc);
    bool CheckDeviceConnected(std::string selectedDevice);
    void SetDisplayName(const std::string &deviceName, bool isLocalDevice);
    std::vector<sptr<AudioDeviceDescriptor>> GetDevicesInner(DeviceFlag deviceFlag);
    sptr<AudioDeviceDescriptor> GetConnectedDeviceByType(int32_t deviceType);
    sptr<AudioDeviceDescriptor> GetConnectedDeviceByType(std::string networkId, int32_t deviceType);
    sptr<AudioDeviceDescriptor> GetConnectedDeviceByType(std::string networkId, int32_t deviceType,
        std::string macAddress);
    void DelConnectedDevice(std::string networkId, int32_t deviceType);
    void DelConnectedDevice(std::string networkId, int32_t deviceType, std::string macAddress);
    void AddConnectedDevice(sptr<AudioDeviceDescriptor> remoteDeviceDescriptor);
    DeviceType FindConnectedHeadset();
    void SetDisplayName(const std::string macAddress, const std::string deviceName);
    sptr<AudioDeviceDescriptor> CheckExistInputDevice(DeviceType activeDevice);
    sptr<AudioDeviceDescriptor> CheckExistOutputDevice(DeviceType activeDevice, std::string macAddress);
    std::vector<sptr<AudioDeviceDescriptor>> GetCopy();
    std::vector<sptr<AudioDeviceDescriptor>> GetDevicesForGroup(GroupType type, int32_t groupId);
private:
    AudioConnectedDevice() {}
    ~AudioConnectedDevice() {}
private:
    std::vector<sptr<AudioDeviceDescriptor>> connectedDevices_;
};

}
}

#endif