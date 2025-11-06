/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <iostream>
#include "bluetooth_device_manager.h"
#include "bluetooth_device_utils.h"
#include "bluetooth_host.h"
#include "audio_info.h"
#include "audio_engine_log.h"
#include "idevice_status_observer.h"
#include "../fuzz_utils.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
using namespace Bluetooth;

FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
const int32_t TRANSPORT = 2;
const string DEFAULT_BLUETOOTH_MAC_ADDRESS = "00:11:22:33:44:55";

typedef void (*TestFuncs)();

class DummyDeviceStatusObserver : public IDeviceStatusObserver {
public:
    void OnDeviceStatusUpdated(DeviceType devType, bool isConnected,
        const std::string &macAddress, const std::string &deviceName,
        const AudioStreamInfo &streamInfo, DeviceRole role = DEVICE_ROLE_NONE, bool hasPair = false) override {}

    void OnMicrophoneBlockedUpdate(DeviceType devType, DeviceBlockStatus status) override {}

    void OnPnpDeviceStatusUpdated(AudioDeviceDescriptor &desc, bool isConnected) override {}

    void OnDeviceConfigurationChanged(DeviceType deviceType,
        const std::string &macAddress, const std::string &deviceName,
        const AudioStreamInfo &streamInfo) override {}

    void OnDeviceStatusUpdated(DStatusInfo statusInfo, bool isStop = false) override {}

    void OnServiceConnected(AudioServiceIndex serviceIndex) override {}

    void OnServiceDisconnected(AudioServiceIndex serviceIndex) override {}

    void OnForcedDeviceSelected(DeviceType devType, const std::string &macAddress,
        sptr<AudioRendererFilter> filter = nullptr) override {}

    void OnPrivacyDeviceSelected(DeviceType devType, const std::string &macAddress) override {};

    void OnDeviceStatusUpdated(AudioDeviceDescriptor &desc, bool isConnected) override {}

    void OnDeviceInfoUpdated(AudioDeviceDescriptor &desc, const DeviceInfoUpdateCommand updateCommand) override {}
};

void OnStop()
{
    Bluetooth::BluetoothHost::GetDefaultHost().Close();
}

void RegisterDeviceObserverFuzzTest()
{
    DummyDeviceStatusObserver observer;
    RegisterDeviceObserver(observer);
    UnregisterDeviceObserver();
    OnStop();
}

void SetMediaStackFuzzTest()
{
    BluetoothRemoteDevice device(DEFAULT_BLUETOOTH_MAC_ADDRESS, TRANSPORT);
    int32_t action = g_fuzzUtils.GetData<int32_t>();

    MediaBluetoothDeviceManager deviceManager;
    deviceManager.SetMediaStack(device, action);
    OnStop();
}

void SendUserSelectionEventFuzzTest()
{
    DeviceType devType = g_fuzzUtils.GetData<DeviceType>();
    std::string macAddress = DEFAULT_BLUETOOTH_MAC_ADDRESS;
    int32_t eventType = g_fuzzUtils.GetData<int32_t>();
    SendUserSelectionEvent(devType, macAddress, eventType);
    OnStop();
}

void NotifyToUpdateAudioDeviceFuzzTest()
{
    BluetoothRemoteDevice device(DEFAULT_BLUETOOTH_MAC_ADDRESS, TRANSPORT);
    AudioDeviceDescriptor desc;
    DeviceStatus status = g_fuzzUtils.GetData<DeviceStatus>();
    MediaBluetoothDeviceManager deviceManager;
    deviceManager.NotifyToUpdateAudioDevice(device, desc, status);
    OnStop();
}

void IsA2dpBluetoothDeviceConnectingFuzzTest()
{
    MediaBluetoothDeviceManager deviceManager;
    BluetoothRemoteDevice device(DEFAULT_BLUETOOTH_MAC_ADDRESS, TRANSPORT);
    bool isConnect = g_fuzzUtils.GetData<bool>();
    int32_t action = g_fuzzUtils.GetData<BluetoothDeviceAction>();
    if (isConnect) {
        deviceManager.SetMediaStack(device, action);
    }
    deviceManager.IsA2dpBluetoothDeviceConnecting(DEFAULT_BLUETOOTH_MAC_ADDRESS);
    OnStop();
}

void UpdateA2dpDeviceConfigurationFuzzTest()
{
    BluetoothRemoteDevice device(DEFAULT_BLUETOOTH_MAC_ADDRESS, TRANSPORT);
    AudioStreamInfo streamInfo;
    MediaBluetoothDeviceManager deviceManager;
    deviceManager.UpdateA2dpDeviceConfiguration(device, streamInfo);
    deviceManager.GetAllA2dpBluetoothDevice();
    deviceManager.GetA2dpVirtualDeviceList();
    deviceManager.ClearAllA2dpBluetoothDevice();
    OnStop();
}

void SetA2dpInStackFuzzTest()
{
    BluetoothRemoteDevice device(DEFAULT_BLUETOOTH_MAC_ADDRESS, TRANSPORT);
    int32_t action = g_fuzzUtils.GetData<int32_t>();

    AudioStreamInfo streamInfo;

    A2dpInBluetoothDeviceManager deviceManager;
    deviceManager.SetA2dpInStack(device, streamInfo, action);
    OnStop();
}

void A2dpInNotifyToUpdateAudioDeviceFuzzTest()
{
    BluetoothRemoteDevice device(DEFAULT_BLUETOOTH_MAC_ADDRESS, TRANSPORT);
    AudioStreamInfo streamInfo;
    AudioDeviceDescriptor desc;
    DeviceStatus status = g_fuzzUtils.GetData<DeviceStatus>();
    A2dpInBluetoothDeviceManager deviceManager;
    deviceManager.NotifyToUpdateAudioDevice(device, streamInfo, desc, status);
    OnStop();
}

void GetA2dpInDeviceStreamInfoFuzzTest()
{
    BluetoothRemoteDevice device(DEFAULT_BLUETOOTH_MAC_ADDRESS, TRANSPORT);
    std::vector<std::string> macAddresses = {
        DEFAULT_BLUETOOTH_MAC_ADDRESS,
        "AA:BB:CC:DD:EE:FF",
        "12:34:56:78:9A:BC"
    };
    size_t idx = g_fuzzUtils.GetData<size_t>() % macAddresses.size();

    AudioStreamInfo streamInfo;
    A2dpInBluetoothDeviceManager deviceManager;
    int32_t action = g_fuzzUtils.GetData<BluetoothDeviceAction>();
    deviceManager.SetA2dpInStack(device, streamInfo, action);

    deviceManager.GetA2dpInDeviceStreamInfo(macAddresses[idx], streamInfo);
    OnStop();
}

void GetAllA2dpInBluetoothDeviceFuzzTest()
{
    BluetoothRemoteDevice device(DEFAULT_BLUETOOTH_MAC_ADDRESS, TRANSPORT);
    AudioStreamInfo streamInfo;
    A2dpInBluetoothDeviceManager deviceManager;
    deviceManager.GetAllA2dpInBluetoothDevice();
    deviceManager.ClearAllA2dpInBluetoothDevice();
    deviceManager.ClearAllA2dpInStreamInfo();
    OnStop();
}

void SetHfpStackFuzzTest()
{
    BluetoothRemoteDevice device(DEFAULT_BLUETOOTH_MAC_ADDRESS, TRANSPORT);
    int32_t action = g_fuzzUtils.GetData<int32_t>();

    HfpBluetoothDeviceManager deviceManager;
    deviceManager.SetHfpStack(device, action);
    OnStop();
}

void HfpNotifyToUpdateAudioDeviceFuzzTest()
{
    BluetoothRemoteDevice device(DEFAULT_BLUETOOTH_MAC_ADDRESS, TRANSPORT);
    AudioDeviceDescriptor desc;
    DeviceStatus status = g_fuzzUtils.GetData<DeviceStatus>();
    HfpBluetoothDeviceManager deviceManager;
    deviceManager.NotifyToUpdateAudioDevice(device, desc, status);
    OnStop();
}

void IsHfpBluetoothDeviceConnectingFuzzTest()
{
    HfpBluetoothDeviceManager deviceManager;
    BluetoothRemoteDevice device(DEFAULT_BLUETOOTH_MAC_ADDRESS, TRANSPORT);
    bool isConnect = g_fuzzUtils.GetData<bool>();
    int32_t action = g_fuzzUtils.GetData<BluetoothDeviceAction>();
    if (isConnect) {
        deviceManager.SetHfpStack(device, action);
    }

    deviceManager.IsHfpBluetoothDeviceConnecting(DEFAULT_BLUETOOTH_MAC_ADDRESS);
    OnStop();
}

void GetConnectedHfpBluetoothDeviceFuzzTest()
{
    BluetoothRemoteDevice device(DEFAULT_BLUETOOTH_MAC_ADDRESS, TRANSPORT);
    std::vector<std::string> macAddresses = {
        DEFAULT_BLUETOOTH_MAC_ADDRESS,
        "AA:BB:CC:DD:EE:FF",
        "12:34:56:78:9A:BC"
    };
    size_t idx = g_fuzzUtils.GetData<size_t>() % macAddresses.size();

    HfpBluetoothDeviceManager deviceManager;
    int32_t action = g_fuzzUtils.GetData<BluetoothDeviceAction>();
    deviceManager.SetHfpStack(device, action);
    deviceManager.GetConnectedHfpBluetoothDevice(macAddresses[idx], device);
    OnStop();
}

void GetAllHfpBluetoothDeviceFuzzTest()
{
    BluetoothRemoteDevice device(DEFAULT_BLUETOOTH_MAC_ADDRESS, TRANSPORT);
    HfpBluetoothDeviceManager deviceManager;
    deviceManager.GetAllHfpBluetoothDevice();
    deviceManager.GetHfpVirtualDeviceList();
    deviceManager.ClearAllHfpBluetoothDevice();
    OnStop();
}

void OnScoStateChangedFuzzTest()
{
    BluetoothRemoteDevice device(DEFAULT_BLUETOOTH_MAC_ADDRESS, TRANSPORT);
    HfpBluetoothDeviceManager deviceManager;
    bool isScoConnected = g_fuzzUtils.GetData<bool>();
    int32_t reason = g_fuzzUtils.GetData<int32_t>();
    deviceManager.OnScoStateChanged(device, isScoConnected, reason);
    OnStop();
}

void RegisterDisconnectScoFuncFuzzTest()
{
    HfpBluetoothDeviceManager deviceManager;
    deviceManager.RegisterDisconnectScoFunc(nullptr);
    OnStop();
}

void HandleDisconnectDeviceFuzzTest()
{
    MediaBluetoothDeviceManager deviceManager;
    BluetoothRemoteDevice device(DEFAULT_BLUETOOTH_MAC_ADDRESS, TRANSPORT);
    deviceManager.a2dpBluetoothDeviceMap_[DEFAULT_BLUETOOTH_MAC_ADDRESS] = device;
    deviceManager.HandleDisconnectDevice(device);
    OnStop();
}

void HandleWearDeviceFuzzTest()
{
    MediaBluetoothDeviceManager deviceManager;
    BluetoothRemoteDevice device(DEFAULT_BLUETOOTH_MAC_ADDRESS, TRANSPORT);
    deviceManager.a2dpBluetoothDeviceMap_[DEFAULT_BLUETOOTH_MAC_ADDRESS] = device;
    deviceManager.HandleWearDevice(device);
    OnStop();
}

void HandleUnwearDeviceFuzzTest()
{
    MediaBluetoothDeviceManager deviceManager;
    BluetoothRemoteDevice device(DEFAULT_BLUETOOTH_MAC_ADDRESS, TRANSPORT);
    deviceManager.a2dpBluetoothDeviceMap_[DEFAULT_BLUETOOTH_MAC_ADDRESS] = device;
    deviceManager.HandleUnwearDevice(device);
    OnStop();
}

void HandleEnableDeviceFuzzTest()
{
    MediaBluetoothDeviceManager deviceManager;
    BluetoothRemoteDevice device(DEFAULT_BLUETOOTH_MAC_ADDRESS, TRANSPORT);
    deviceManager.a2dpBluetoothDeviceMap_[DEFAULT_BLUETOOTH_MAC_ADDRESS] = device;
    deviceManager.HandleEnableDevice(device);
    OnStop();
}

void HandleDisableDeviceFuzzTest()
{
    MediaBluetoothDeviceManager deviceManager;
    BluetoothRemoteDevice device(DEFAULT_BLUETOOTH_MAC_ADDRESS, TRANSPORT);
    deviceManager.a2dpBluetoothDeviceMap_[DEFAULT_BLUETOOTH_MAC_ADDRESS] = device;
    deviceManager.HandleDisableDevice(device);
    OnStop();
}

void HandleWearEnableFuzzTest()
{
    MediaBluetoothDeviceManager deviceManager;
    BluetoothRemoteDevice device(DEFAULT_BLUETOOTH_MAC_ADDRESS, TRANSPORT);
    deviceManager.a2dpBluetoothDeviceMap_[DEFAULT_BLUETOOTH_MAC_ADDRESS] = device;
    deviceManager.HandleWearEnable(device);
    OnStop();
}

void HandleUpdateDeviceCategoryFuzzTest()
{
    MediaBluetoothDeviceManager deviceManager;
    BluetoothRemoteDevice device(DEFAULT_BLUETOOTH_MAC_ADDRESS, TRANSPORT);
    deviceManager.a2dpBluetoothDeviceMap_[DEFAULT_BLUETOOTH_MAC_ADDRESS] = device;
    deviceManager.HandleUpdateDeviceCategory(device);
    OnStop();
}

void HfpHandleDisconnectDeviceFuzzTest()
{
    HfpBluetoothDeviceManager deviceManager;
    BluetoothRemoteDevice device(DEFAULT_BLUETOOTH_MAC_ADDRESS, TRANSPORT);
    deviceManager.hfpBluetoothDeviceMap_[DEFAULT_BLUETOOTH_MAC_ADDRESS] = device;
    deviceManager.HandleDisconnectDevice(device);
    OnStop();
}

void HfpHandleWearDeviceFuzzTest()
{
    HfpBluetoothDeviceManager deviceManager;
    BluetoothRemoteDevice device(DEFAULT_BLUETOOTH_MAC_ADDRESS, TRANSPORT);
    deviceManager.hfpBluetoothDeviceMap_[DEFAULT_BLUETOOTH_MAC_ADDRESS] = device;
    deviceManager.HandleWearDevice(device);
    OnStop();
}

void HfpHandleUnwearDeviceFuzzTest()
{
    HfpBluetoothDeviceManager deviceManager;
    BluetoothRemoteDevice device(DEFAULT_BLUETOOTH_MAC_ADDRESS, TRANSPORT);
    deviceManager.hfpBluetoothDeviceMap_[DEFAULT_BLUETOOTH_MAC_ADDRESS] = device;
    deviceManager.HandleUnwearDevice(device);
    OnStop();
}

void HfpHandleEnableDeviceFuzzTest()
{
    HfpBluetoothDeviceManager deviceManager;
    BluetoothRemoteDevice device(DEFAULT_BLUETOOTH_MAC_ADDRESS, TRANSPORT);
    deviceManager.hfpBluetoothDeviceMap_[DEFAULT_BLUETOOTH_MAC_ADDRESS] = device;
    deviceManager.HandleEnableDevice(device);
    OnStop();
}

void HfpHandleDisableDeviceFuzzTest()
{
    HfpBluetoothDeviceManager deviceManager;
    BluetoothRemoteDevice device(DEFAULT_BLUETOOTH_MAC_ADDRESS, TRANSPORT);
    deviceManager.hfpBluetoothDeviceMap_[DEFAULT_BLUETOOTH_MAC_ADDRESS] = device;
    deviceManager.HandleDisableDevice(device);
    OnStop();
}

void HfpHandleUpdateDeviceCategoryFuzzTest()
{
    HfpBluetoothDeviceManager deviceManager;
    BluetoothRemoteDevice device(DEFAULT_BLUETOOTH_MAC_ADDRESS, TRANSPORT);
    deviceManager.hfpBluetoothDeviceMap_[DEFAULT_BLUETOOTH_MAC_ADDRESS] = device;
    deviceManager.HandleUpdateDeviceCategory(device);
    OnStop();
}

void OnDeviceCategoryUpdatedFuzzTest()
{
    HfpBluetoothDeviceManager deviceManager;
    BluetoothRemoteDevice device(DEFAULT_BLUETOOTH_MAC_ADDRESS, TRANSPORT);
    deviceManager.hfpBluetoothDeviceMap_[DEFAULT_BLUETOOTH_MAC_ADDRESS] = device;
    AudioDeviceDescriptor desc;
    desc.deviceCategory_ = BT_UNWEAR_HEADPHONE;
    deviceManager.OnDeviceCategoryUpdated(device, desc);
    OnStop();
}

void OnDeviceEnableUpdatedFuzzTest()
{
    HfpBluetoothDeviceManager deviceManager;
    BluetoothRemoteDevice device(DEFAULT_BLUETOOTH_MAC_ADDRESS, TRANSPORT);
    deviceManager.hfpBluetoothDeviceMap_[DEFAULT_BLUETOOTH_MAC_ADDRESS] = device;
    AudioDeviceDescriptor desc;
    desc.isEnable_ = g_fuzzUtils.GetData<bool>();
    deviceManager.OnDeviceEnableUpdated(device, desc);
    OnStop();
}

void TryDisconnectScoSyncFuzzTest()
{
    HfpBluetoothDeviceManager deviceManager;
    BluetoothRemoteDevice device(DEFAULT_BLUETOOTH_MAC_ADDRESS, TRANSPORT);
    deviceManager.hfpBluetoothDeviceMap_[DEFAULT_BLUETOOTH_MAC_ADDRESS] = device;
    Bluetooth::HfpBluetoothDeviceManager::DisconnectScoForDevice func;
    deviceManager.RegisterDisconnectScoFunc(func);
    std::string reason = "test";
    deviceManager.TryDisconnectScoSync(device, reason);
    OnStop();
}

vector<TestFuncs> g_testFuncs = {
    RegisterDeviceObserverFuzzTest,
    SetMediaStackFuzzTest,
    SendUserSelectionEventFuzzTest,
    NotifyToUpdateAudioDeviceFuzzTest,
    IsA2dpBluetoothDeviceConnectingFuzzTest,
    UpdateA2dpDeviceConfigurationFuzzTest,
    SetA2dpInStackFuzzTest,
    A2dpInNotifyToUpdateAudioDeviceFuzzTest,
    GetA2dpInDeviceStreamInfoFuzzTest,
    GetAllA2dpInBluetoothDeviceFuzzTest,
    SetHfpStackFuzzTest,
    HfpNotifyToUpdateAudioDeviceFuzzTest,
    IsHfpBluetoothDeviceConnectingFuzzTest,
    GetConnectedHfpBluetoothDeviceFuzzTest,
    GetAllHfpBluetoothDeviceFuzzTest,
    OnScoStateChangedFuzzTest,
    RegisterDisconnectScoFuncFuzzTest,
    HandleDisconnectDeviceFuzzTest,
    HandleWearDeviceFuzzTest,
    HandleUnwearDeviceFuzzTest,
    HandleEnableDeviceFuzzTest,
    HandleDisableDeviceFuzzTest,
    HandleWearEnableFuzzTest,
    HandleUpdateDeviceCategoryFuzzTest,
    HfpHandleDisconnectDeviceFuzzTest,
    HfpHandleWearDeviceFuzzTest,
    HfpHandleUnwearDeviceFuzzTest,
    HfpHandleEnableDeviceFuzzTest,
    HfpHandleDisableDeviceFuzzTest,
    HfpHandleUpdateDeviceCategoryFuzzTest,
    OnDeviceCategoryUpdatedFuzzTest,
    OnDeviceEnableUpdatedFuzzTest,
    TryDisconnectScoSyncFuzzTest,
};

} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::AudioStandard::g_fuzzUtils.fuzzTest(data, size, OHOS::AudioStandard::g_testFuncs);
    return 0;
}