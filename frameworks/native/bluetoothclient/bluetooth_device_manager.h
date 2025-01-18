/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#ifndef BLUETOOTH_DEVICE_MANAGER_H
#define BLUETOOTH_DEVICE_MANAGER_H

#include <mutex>
#include "bluetooth_hfp_ag.h"
#include "bluetooth_device_utils.h"
#include "audio_errors.h"
#include "audio_common_log.h"
#include "audio_system_manager.h"
#include "idevice_status_observer.h"

namespace OHOS {
namespace Bluetooth {

int32_t RegisterDeviceObserver(AudioStandard::IDeviceStatusObserver &observer);
void UnregisterDeviceObserver();
void SendUserSelectionEvent(AudioStandard::DeviceType devType, const std::string &macAddress, int32_t eventType);
bool IsBTWearDetectionEnable(const BluetoothRemoteDevice &device);
std::string GetEncryptAddr(const std::string &addr);

class MediaBluetoothDeviceManager {
public:
    MediaBluetoothDeviceManager() = default;
    virtual ~MediaBluetoothDeviceManager() = default;
    static void SetMediaStack(const BluetoothRemoteDevice &device, int action);
    static void HandleConnectingDevice(const BluetoothRemoteDevice &device);
    static void HandleConnectDevice(const BluetoothRemoteDevice &device);
    static void HandleDisconnectDevice(const BluetoothRemoteDevice &device);
    static void HandleWearDevice(const BluetoothRemoteDevice &device);
    static void HandleUnwearDevice(const BluetoothRemoteDevice &device);
    static void HandleEnableDevice(const BluetoothRemoteDevice &device);
    static void HandleDisableDevice(const BluetoothRemoteDevice &device);
    static void HandleWearEnable(const BluetoothRemoteDevice &device);
    static void HandleWearDisable(const BluetoothRemoteDevice &device);
    static void HandleUserSelection(const BluetoothRemoteDevice &device);
    static void HandleVirtualConnectDevice(const BluetoothRemoteDevice &device);
    static void HandleRemoveVirtualConnectDevice(const BluetoothRemoteDevice &device);
    static void AddDeviceInConfigVector(const BluetoothRemoteDevice &device,
        std::vector<BluetoothRemoteDevice> &deviceVector);
    static void RemoveDeviceInConfigVector(const BluetoothRemoteDevice &device,
        std::vector<BluetoothRemoteDevice> &deviceVector);
    static void NotifyToUpdateAudioDevice(const BluetoothRemoteDevice &device,
        AudioStandard::AudioDeviceDescriptor &desc, DeviceStatus deviceStatus);
    static void NotifyToUpdateVirtualDevice(const BluetoothRemoteDevice &device,
        AudioStandard::AudioDeviceDescriptor &desc, DeviceStatus deviceStatus);
    static bool IsA2dpBluetoothDeviceExist(const std::string& macAddress);
    static bool IsA2dpBluetoothDeviceConnecting(const std::string& macAddress);
    static int32_t GetConnectedA2dpBluetoothDevice(const std::string& macAddress, BluetoothRemoteDevice &device);
    static void UpdateA2dpDeviceConfiguration(const BluetoothRemoteDevice &device,
        const AudioStandard::AudioStreamInfo &streamInfo);
    static std::vector<BluetoothRemoteDevice> GetAllA2dpBluetoothDevice();
    static void ClearAllA2dpBluetoothDevice();
    static std::vector<BluetoothRemoteDevice> GetA2dpVirtualDeviceList();

private:
    static std::map<std::string, BluetoothRemoteDevice> a2dpBluetoothDeviceMap_;
    static std::map<std::string, BluetoothDeviceAction> wearDetectionStateMap_;
    static std::vector<BluetoothRemoteDevice> privacyDevices_;
    static std::vector<BluetoothRemoteDevice> commonDevices_;
    static std::vector<BluetoothRemoteDevice> negativeDevices_;
    static std::vector<BluetoothRemoteDevice> connectingDevices_;
    static std::vector<BluetoothRemoteDevice> virtualDevices_;
    static void HandleUpdateDeviceCategory(const BluetoothRemoteDevice &device);
    static AudioStandard::AudioDeviceDescriptor HandleConnectDeviceInner(const BluetoothRemoteDevice &device);
};

class A2dpInBluetoothDeviceManager {
public:
    A2dpInBluetoothDeviceManager() = default;
    virtual ~A2dpInBluetoothDeviceManager() = default;
    static void SetA2dpInStack(const BluetoothRemoteDevice &device,
        const AudioStandard::AudioStreamInfo &streamInfo, int32_t action);
    static void HandleConnectDevice(const BluetoothRemoteDevice &device,
        const AudioStandard::AudioStreamInfo &streamInfo);
    static void HandleDisconnectDevice(const BluetoothRemoteDevice &device,
        const AudioStandard::AudioStreamInfo &streamInfo);
    static void NotifyToUpdateAudioDevice(const BluetoothRemoteDevice &device,
        const AudioStandard::AudioStreamInfo &streamInfo,
        AudioStandard::AudioDeviceDescriptor &desc,
        DeviceStatus deviceStatus);
    static bool GetA2dpInDeviceStreamInfo(const std::string& macAddress,
        AudioStandard::AudioStreamInfo &streamInfo);
    static bool IsA2dpInBluetoothDeviceExist(const std::string& macAddress);
    static std::vector<BluetoothRemoteDevice> GetAllA2dpInBluetoothDevice();
    static void ClearAllA2dpInBluetoothDevice();
    static void ClearAllA2dpInStreamInfo();

private:
    static std::map<std::string, BluetoothRemoteDevice> a2dpInBluetoothDeviceMap_;
    static std::map<std::string, AudioStandard::AudioStreamInfo> a2dpInStreamInfoMap_;
};

struct BluetoothStopVirtualCallHandle {
    BluetoothRemoteDevice device;
    bool isWaitingForStoppingVirtualCall;
};

class HfpBluetoothDeviceManager {
public:
    HfpBluetoothDeviceManager() = default;
    virtual ~HfpBluetoothDeviceManager() = default;
    static void SetHfpStack(const BluetoothRemoteDevice &device, int action);
    static void HandleConnectingDevice(const BluetoothRemoteDevice &device);
    static void HandleConnectDevice(const BluetoothRemoteDevice &device);
    static void HandleDisconnectDevice(const BluetoothRemoteDevice &device);
    static void HandleWearDevice(const BluetoothRemoteDevice &device);
    static void HandleUnwearDevice(const BluetoothRemoteDevice &device);
    static void HandleEnableDevice(const BluetoothRemoteDevice &device);
    static void HandleDisableDevice(const BluetoothRemoteDevice &device);
    static void HandleWearEnable(const BluetoothRemoteDevice &device);
    static void HandleWearDisable(const BluetoothRemoteDevice &device);
    static void HandleUserSelection(const BluetoothRemoteDevice &device);
    static void HandleStopVirtualCall(const BluetoothRemoteDevice &device);
    static void HandleVirtualConnectDevice(const BluetoothRemoteDevice &device);
    static void HandleRemoveVirtualConnectDevice(const BluetoothRemoteDevice &device);
    static void AddDeviceInConfigVector(const BluetoothRemoteDevice &device,
        std::vector<BluetoothRemoteDevice> &deviceVector);
    static void RemoveDeviceInConfigVector(const BluetoothRemoteDevice &device,
        std::vector<BluetoothRemoteDevice> &deviceVector);
    static void NotifyToUpdateAudioDevice(const BluetoothRemoteDevice &device,
        AudioStandard::AudioDeviceDescriptor &desc, DeviceStatus deviceStatus);
    static void NotifyToUpdateVirtualDevice(const BluetoothRemoteDevice &device,
        AudioStandard::AudioDeviceDescriptor &desc, DeviceStatus deviceStatus);
    static bool IsHfpBluetoothDeviceExist(const std::string& macAddress);
    static bool IsHfpBluetoothDeviceConnecting(const std::string& macAddress);
    static void UpdateHfpDeviceConfiguration(const BluetoothRemoteDevice &device,
        const AudioStandard::AudioStreamInfo &streamInfo);
    static void OnScoStateChanged(const BluetoothRemoteDevice &device, bool isConnected, int reason);
    static int32_t GetConnectedHfpBluetoothDevice(const std::string& macAddress, BluetoothRemoteDevice &device);
    static std::vector<BluetoothRemoteDevice> GetAllHfpBluetoothDevice();
    static void ClearAllHfpBluetoothDevice();
    static std::vector<BluetoothRemoteDevice> GetHfpVirtualDeviceList();

private:
    static void HandleUpdateDeviceCategory(const BluetoothRemoteDevice &device);
    static AudioStandard::AudioDeviceDescriptor HandleConnectDeviceInner(const BluetoothRemoteDevice &device);
    static std::map<std::string, BluetoothRemoteDevice> hfpBluetoothDeviceMap_;
    static std::map<std::string, BluetoothDeviceAction> wearDetectionStateMap_;
    static std::vector<BluetoothRemoteDevice> privacyDevices_;
    static std::vector<BluetoothRemoteDevice> commonDevices_;
    static std::vector<BluetoothRemoteDevice> negativeDevices_;
    static std::vector<BluetoothRemoteDevice> connectingDevices_;
    static std::vector<BluetoothRemoteDevice> virtualDevices_;
    static std::mutex stopVirtualCallHandleLock_;
    static BluetoothStopVirtualCallHandle stopVirtualCallHandle_;
};
} // namespace Bluetooth
} // namespace OHOS

#endif // BLUETOOTH_DEVICE_MANAGER_H