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
#include <fuzzer/FuzzedDataProvider.h>
namespace OHOS {
namespace AudioStandard {
using namespace std;
using namespace Bluetooth;

const size_t THRESHOLD = 10;
const size_t MAX_STRING_LEN = 64;
const int32_t TRANSPORT = 2;
const string DEFAULT_BLUETOOTH_MAC_ADDRESS = "00:11:22:33:44:55";
typedef void (*TestPtr)(FuzzedDataProvider&);

template<typename T>
T ConsumeEnum(FuzzedDataProvider& fdp)
{
    return static_cast<T>(fdp.ConsumeIntegral<int32_t>());
}

template<typename T>
T PickValue(FuzzedDataProvider& fdp, const vector<T>& values)
{
    return values[fdp.ConsumeIntegralInRange<size_t>(0, values.size() - 1)];
}

std::string ConsumeString(FuzzedDataProvider& fdp, const std::string& fallback)
{
    std::string value = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    return value.empty() ? fallback : value;
}

const vector<DeviceStatus> g_testDeviceStatuses = {
    ADD,
    REMOVE,
    VIRTUAL_ADD,
    VIRTUAL_REMOVE,
};

const vector<BluetoothDeviceAction> g_testBluetoothDeviceActions = {
    WEAR_ACTION,
    UNWEAR_ACTION,
    ENABLEFROMREMOTE_ACTION,
    DISABLEFROMREMOTE_ACTION,
    ENABLE_WEAR_DETECTION_ACTION,
    DISABLE_WEAR_DETECTION_ACTION,
    USER_SELECTION_ACTION,
    STOP_VIRTUAL_CALL,
    CATEGORY_UPDATE_ACTION,
    CONNECTING_ACTION,
    CONNECT_ACTION,
    DISCONNECT_ACTION,
    VIRTUAL_DEVICE_ADD_ACTION,
    VIRTUAL_DEVICE_REMOVE_ACTION,
};

const vector<DeviceCategory> g_testDeviceCategories = {
    CATEGORY_DEFAULT,
    BT_HEADPHONE,
    BT_SOUNDBOX,
    BT_CAR,
    BT_GLASSES,
    BT_WATCH,
    BT_HEARAID,
    BT_UNWEAR_HEADPHONE,
};

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
        sptr<AudioRendererFilter> filter = nullptr, const std::string &caller = "") override {}

    void OnPrivacyDeviceSelected(DeviceType devType, const std::string &macAddress,
        const std::string &caller = "") override {};

    void OnDeviceStatusUpdated(AudioDeviceDescriptor &desc, bool isConnected) override {}

    void OnDeviceInfoUpdated(AudioDeviceDescriptor &desc, const DeviceInfoUpdateCommand updateCommand) override {}

    void OnConnectFailed(AudioDeviceDescriptor &desc) override {}
};

void OnStop()
{
    Bluetooth::BluetoothHost::GetDefaultHost().Close();
}

void RegisterDeviceObserverFuzzTest(FuzzedDataProvider& fdp)
{
    DummyDeviceStatusObserver observer;
    if (fdp.ConsumeBool()) {
        RegisterDeviceObserver(observer);
    }
    UnregisterDeviceObserver();
    OnStop();
}

void SetMediaStackFuzzTest(FuzzedDataProvider& fdp)
{
    std::string macAddress = ConsumeString(fdp, DEFAULT_BLUETOOTH_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    int32_t action = fdp.ConsumeIntegral<int32_t>();

    MediaBluetoothDeviceManager deviceManager;
    deviceManager.SetMediaStack(device, action);
    OnStop();
}

void SendUserSelectionEventFuzzTest(FuzzedDataProvider& fdp)
{
    DeviceType devType = ConsumeEnum<DeviceType>(fdp);
    std::string macAddress = ConsumeString(fdp, DEFAULT_BLUETOOTH_MAC_ADDRESS);
    int32_t eventType = fdp.ConsumeIntegral<int32_t>();
    SendUserSelectionEvent(devType, macAddress, eventType);
    OnStop();
}

void NotifyToUpdateAudioDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    std::string macAddress = ConsumeString(fdp, DEFAULT_BLUETOOTH_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    AudioDeviceDescriptor desc;
    DeviceStatus status = PickValue(fdp, g_testDeviceStatuses);
    MediaBluetoothDeviceManager deviceManager;
    deviceManager.NotifyToUpdateAudioDevice(device, desc, status);
    OnStop();
}

void IsA2dpBluetoothDeviceConnectingFuzzTest(FuzzedDataProvider& fdp)
{
    MediaBluetoothDeviceManager deviceManager;
    std::string macAddress = ConsumeString(fdp, DEFAULT_BLUETOOTH_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    bool isConnect = fdp.ConsumeBool();
    int32_t action = static_cast<int32_t>(PickValue(fdp, g_testBluetoothDeviceActions));
    if (isConnect) {
        deviceManager.SetMediaStack(device, action);
    }
    deviceManager.IsA2dpBluetoothDeviceConnecting(macAddress);
    OnStop();
}

void UpdateA2dpDeviceConfigurationFuzzTest(FuzzedDataProvider& fdp)
{
    std::string macAddress = ConsumeString(fdp, DEFAULT_BLUETOOTH_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    AudioStreamInfo streamInfo;
    MediaBluetoothDeviceManager deviceManager;
    deviceManager.UpdateA2dpDeviceConfiguration(device, streamInfo);
    deviceManager.GetAllA2dpBluetoothDevice();
    deviceManager.GetA2dpVirtualDeviceList();
    deviceManager.ClearAllA2dpBluetoothDevice();
    OnStop();
}

void SetA2dpInStackFuzzTest(FuzzedDataProvider& fdp)
{
    std::string macAddress = ConsumeString(fdp, DEFAULT_BLUETOOTH_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    int32_t action = fdp.ConsumeIntegral<int32_t>();

    AudioStreamInfo streamInfo;

    A2dpInBluetoothDeviceManager deviceManager;
    deviceManager.SetA2dpInStack(device, streamInfo, action);
    OnStop();
}

void A2dpInNotifyToUpdateAudioDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    std::string macAddress = ConsumeString(fdp, DEFAULT_BLUETOOTH_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    AudioStreamInfo streamInfo;
    AudioDeviceDescriptor desc;
    DeviceStatus status = PickValue(fdp, g_testDeviceStatuses);
    A2dpInBluetoothDeviceManager deviceManager;
    deviceManager.NotifyToUpdateAudioDevice(device, streamInfo, desc, status);
    OnStop();
}

void GetA2dpInDeviceStreamInfoFuzzTest(FuzzedDataProvider& fdp)
{
    std::string macAddress = ConsumeString(fdp, DEFAULT_BLUETOOTH_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    std::vector<std::string> macAddresses = {
        DEFAULT_BLUETOOTH_MAC_ADDRESS,
        "AA:BB:CC:DD:EE:FF",
        "12:34:56:78:9A:BC"
    };
    std::string selectedMac = PickValue(fdp, macAddresses);

    AudioStreamInfo streamInfo;
    A2dpInBluetoothDeviceManager deviceManager;
    int32_t action = static_cast<int32_t>(PickValue(fdp, g_testBluetoothDeviceActions));
    deviceManager.SetA2dpInStack(device, streamInfo, action);

    deviceManager.GetA2dpInDeviceStreamInfo(selectedMac, streamInfo);
    OnStop();
}

void GetAllA2dpInBluetoothDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    std::string macAddress = ConsumeString(fdp, DEFAULT_BLUETOOTH_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    AudioStreamInfo streamInfo;
    A2dpInBluetoothDeviceManager deviceManager;
    deviceManager.GetAllA2dpInBluetoothDevice();
    deviceManager.ClearAllA2dpInBluetoothDevice();
    deviceManager.ClearAllA2dpInStreamInfo();
    OnStop();
}

void SetHfpStackFuzzTest(FuzzedDataProvider& fdp)
{
    std::string macAddress = ConsumeString(fdp, DEFAULT_BLUETOOTH_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    int32_t action = fdp.ConsumeIntegral<int32_t>();

    HfpBluetoothDeviceManager deviceManager;
    deviceManager.SetHfpStack(device, action);
    OnStop();
}

void HfpNotifyToUpdateAudioDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    std::string macAddress = ConsumeString(fdp, DEFAULT_BLUETOOTH_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    AudioDeviceDescriptor desc;
    DeviceStatus status = PickValue(fdp, g_testDeviceStatuses);
    HfpBluetoothDeviceManager deviceManager;
    deviceManager.NotifyToUpdateAudioDevice(device, desc, status);
    OnStop();
}

void IsHfpBluetoothDeviceConnectingFuzzTest(FuzzedDataProvider& fdp)
{
    HfpBluetoothDeviceManager deviceManager;
    std::string macAddress = ConsumeString(fdp, DEFAULT_BLUETOOTH_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    bool isConnect = fdp.ConsumeBool();
    int32_t action = static_cast<int32_t>(PickValue(fdp, g_testBluetoothDeviceActions));
    if (isConnect) {
        deviceManager.SetHfpStack(device, action);
    }

    deviceManager.IsHfpBluetoothDeviceConnecting(macAddress);
    OnStop();
}

void GetConnectedHfpBluetoothDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    std::string macAddress = ConsumeString(fdp, DEFAULT_BLUETOOTH_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    std::vector<std::string> macAddresses = {
        DEFAULT_BLUETOOTH_MAC_ADDRESS,
        "AA:BB:CC:DD:EE:FF",
        "12:34:56:78:9A:BC"
    };
    std::string selectedMac = PickValue(fdp, macAddresses);

    HfpBluetoothDeviceManager deviceManager;
    int32_t action = static_cast<int32_t>(PickValue(fdp, g_testBluetoothDeviceActions));
    deviceManager.SetHfpStack(device, action);
    deviceManager.GetConnectedHfpBluetoothDevice(selectedMac, device);
    OnStop();
}

void GetAllHfpBluetoothDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    std::string macAddress = ConsumeString(fdp, DEFAULT_BLUETOOTH_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    HfpBluetoothDeviceManager deviceManager;
    deviceManager.GetAllHfpBluetoothDevice();
    deviceManager.GetHfpVirtualDeviceList();
    deviceManager.ClearAllHfpBluetoothDevice();
    OnStop();
}

void OnScoStateChangedFuzzTest(FuzzedDataProvider& fdp)
{
    std::string macAddress = ConsumeString(fdp, DEFAULT_BLUETOOTH_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    HfpBluetoothDeviceManager deviceManager;
    bool isScoConnected = fdp.ConsumeBool();
    int32_t reason = fdp.ConsumeIntegral<int32_t>();
    deviceManager.OnScoStateChanged(device, isScoConnected, reason);
    OnStop();
}

void RegisterDisconnectScoFuncFuzzTest(FuzzedDataProvider& fdp)
{
    HfpBluetoothDeviceManager deviceManager;
    std::string macAddress = ConsumeString(fdp, DEFAULT_BLUETOOTH_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    bool registerRealFunc = fdp.ConsumeBool();
    if (registerRealFunc) {
        HfpBluetoothDeviceManager::DisconnectScoForDevice func =
            [](const BluetoothRemoteDevice&) {};
        deviceManager.RegisterDisconnectScoFunc(func);
        deviceManager.hfpBluetoothDeviceMap_[macAddress] = device;
        std::string reason = ConsumeString(fdp, "test");
        deviceManager.TryDisconnectScoSync(device, reason);
    } else {
        deviceManager.RegisterDisconnectScoFunc(nullptr);
    }
    OnStop();
}

void HandleDisconnectDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    MediaBluetoothDeviceManager deviceManager;
    std::string macAddress = ConsumeString(fdp, DEFAULT_BLUETOOTH_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    deviceManager.a2dpBluetoothDeviceMap_[macAddress] = device;
    deviceManager.HandleDisconnectDevice(device);
    OnStop();
}

void HandleWearDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    MediaBluetoothDeviceManager deviceManager;
    std::string macAddress = ConsumeString(fdp, DEFAULT_BLUETOOTH_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    deviceManager.a2dpBluetoothDeviceMap_[macAddress] = device;
    deviceManager.HandleWearDevice(device);
    OnStop();
}

void HandleUnwearDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    MediaBluetoothDeviceManager deviceManager;
    std::string macAddress = ConsumeString(fdp, DEFAULT_BLUETOOTH_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    deviceManager.a2dpBluetoothDeviceMap_[macAddress] = device;
    deviceManager.HandleUnwearDevice(device);
    OnStop();
}

void HandleEnableDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    MediaBluetoothDeviceManager deviceManager;
    std::string macAddress = ConsumeString(fdp, DEFAULT_BLUETOOTH_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    deviceManager.a2dpBluetoothDeviceMap_[macAddress] = device;
    deviceManager.HandleEnableDevice(device);
    OnStop();
}

void HandleDisableDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    MediaBluetoothDeviceManager deviceManager;
    std::string macAddress = ConsumeString(fdp, DEFAULT_BLUETOOTH_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    deviceManager.a2dpBluetoothDeviceMap_[macAddress] = device;
    deviceManager.HandleDisableDevice(device);
    OnStop();
}

void HandleWearEnableFuzzTest(FuzzedDataProvider& fdp)
{
    MediaBluetoothDeviceManager deviceManager;
    std::string macAddress = ConsumeString(fdp, DEFAULT_BLUETOOTH_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    deviceManager.a2dpBluetoothDeviceMap_[macAddress] = device;
    deviceManager.HandleWearEnable(device);
    OnStop();
}

void HandleUpdateDeviceCategoryFuzzTest(FuzzedDataProvider& fdp)
{
    MediaBluetoothDeviceManager deviceManager;
    std::string macAddress = ConsumeString(fdp, DEFAULT_BLUETOOTH_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    deviceManager.a2dpBluetoothDeviceMap_[macAddress] = device;
    deviceManager.HandleUpdateDeviceCategory(device);
    OnStop();
}

void HfpHandleDisconnectDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    HfpBluetoothDeviceManager deviceManager;
    std::string macAddress = ConsumeString(fdp, DEFAULT_BLUETOOTH_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    deviceManager.hfpBluetoothDeviceMap_[macAddress] = device;
    deviceManager.HandleDisconnectDevice(device);
    OnStop();
}

void HfpHandleWearDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    HfpBluetoothDeviceManager deviceManager;
    std::string macAddress = ConsumeString(fdp, DEFAULT_BLUETOOTH_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    deviceManager.hfpBluetoothDeviceMap_[macAddress] = device;
    deviceManager.HandleWearDevice(device);
    OnStop();
}

void HfpHandleUnwearDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    HfpBluetoothDeviceManager deviceManager;
    std::string macAddress = ConsumeString(fdp, DEFAULT_BLUETOOTH_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    deviceManager.hfpBluetoothDeviceMap_[macAddress] = device;
    deviceManager.HandleUnwearDevice(device);
    OnStop();
}

void HfpHandleEnableDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    HfpBluetoothDeviceManager deviceManager;
    std::string macAddress = ConsumeString(fdp, DEFAULT_BLUETOOTH_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    deviceManager.hfpBluetoothDeviceMap_[macAddress] = device;
    deviceManager.HandleEnableDevice(device);
    OnStop();
}

void HfpHandleDisableDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    HfpBluetoothDeviceManager deviceManager;
    std::string macAddress = ConsumeString(fdp, DEFAULT_BLUETOOTH_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    deviceManager.hfpBluetoothDeviceMap_[macAddress] = device;
    deviceManager.HandleDisableDevice(device);
    OnStop();
}

void HfpHandleUpdateDeviceCategoryFuzzTest(FuzzedDataProvider& fdp)
{
    HfpBluetoothDeviceManager deviceManager;
    std::string macAddress = ConsumeString(fdp, DEFAULT_BLUETOOTH_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    deviceManager.hfpBluetoothDeviceMap_[macAddress] = device;
    deviceManager.HandleUpdateDeviceCategory(device);
    OnStop();
}

void OnDeviceCategoryUpdatedFuzzTest(FuzzedDataProvider& fdp)
{
    HfpBluetoothDeviceManager deviceManager;
    std::string macAddress = ConsumeString(fdp, DEFAULT_BLUETOOTH_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    deviceManager.hfpBluetoothDeviceMap_[macAddress] = device;
    AudioDeviceDescriptor desc;
    desc.deviceCategory_ = PickValue(fdp, g_testDeviceCategories);
    deviceManager.OnDeviceCategoryUpdated(device, desc);
    OnStop();
}

void OnDeviceEnableUpdatedFuzzTest(FuzzedDataProvider& fdp)
{
    HfpBluetoothDeviceManager deviceManager;
    std::string macAddress = ConsumeString(fdp, DEFAULT_BLUETOOTH_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    deviceManager.hfpBluetoothDeviceMap_[macAddress] = device;
    AudioDeviceDescriptor desc;
    desc.isEnable_ = fdp.ConsumeBool();
    deviceManager.OnDeviceEnableUpdated(device, desc);
    OnStop();
}

void TryDisconnectScoSyncFuzzTest(FuzzedDataProvider& fdp)
{
    HfpBluetoothDeviceManager deviceManager;
    std::string macAddress = ConsumeString(fdp, DEFAULT_BLUETOOTH_MAC_ADDRESS);
    BluetoothRemoteDevice device(macAddress, TRANSPORT);
    deviceManager.hfpBluetoothDeviceMap_[macAddress] = device;
    Bluetooth::HfpBluetoothDeviceManager::DisconnectScoForDevice func;
    deviceManager.RegisterDisconnectScoFunc(func);
    std::string reason = ConsumeString(fdp, "test");
    deviceManager.TryDisconnectScoSync(device, reason);
    OnStop();
}

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
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
    });
    func(fdp);
}
 
void Init()
{
}
} // namespace AudioStandard
} // namesapce OHOS
 
/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }
    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::Test(fdp);
    return 0;
}
extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv)
{
    OHOS::AudioStandard::Init();
    return 0;
}