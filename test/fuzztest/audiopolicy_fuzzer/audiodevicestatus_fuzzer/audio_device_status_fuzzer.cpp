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
#include <cstddef>
#include <cstdint>
#include "audio_info.h"
#include "audio_policy_server.h"
#include "audio_policy_service.h"
#include "audio_device_info.h"
#include "audio_utils.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "access_token.h"
#include "audio_channel_blend.h"
#include "volume_ramp.h"
#include "audio_speed.h"

#include "audio_policy_utils.h"
#include "audio_stream_descriptor.h"
#include "audio_limiter_manager.h"
#include "dfx_msg_manager.h"

#include "audio_source_clock.h"
#include "capturer_clock_manager.h"
#include "hpae_policy_manager.h"
#include "audio_policy_state_monitor.h"
#include "audio_device_info.h"
#include "bluetooth_host.h"
#include <fuzzer/FuzzedDataProvider.h>
namespace OHOS {
namespace AudioStandard {
using namespace std;

const size_t THRESHOLD = 10;

typedef void (*TestFuncs)();

template<class T>
uint32_t GetArrLength(T& arr)
{
    if (arr == nullptr) {
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
        return 0;
    }
    return sizeof(arr) / sizeof(arr[0]);
}

template<typename T>
T PickValue(FuzzedDataProvider &fdp, const std::vector<T> &values)
{
    if (values.empty()) {
        return T {};
    }
    return values[fdp.ConsumeIntegralInRange<size_t>(0, values.size() - 1)];
}

vector<DeviceRole> DeviceRoleVec = {
    DEVICE_ROLE_NONE,
    INPUT_DEVICE,
    OUTPUT_DEVICE,
    DEVICE_ROLE_MAX,
};

vector<DeviceType> DeviceTypeVec = {
    DEVICE_TYPE_NONE,
    DEVICE_TYPE_INVALID,
    DEVICE_TYPE_EARPIECE,
    DEVICE_TYPE_SPEAKER,
    DEVICE_TYPE_WIRED_HEADSET,
    DEVICE_TYPE_WIRED_HEADPHONES,
    DEVICE_TYPE_BLUETOOTH_SCO,
    DEVICE_TYPE_BLUETOOTH_A2DP,
    DEVICE_TYPE_BLUETOOTH_A2DP_IN,
    DEVICE_TYPE_MIC,
    DEVICE_TYPE_WAKEUP,
    DEVICE_TYPE_USB_HEADSET,
    DEVICE_TYPE_DP,
    DEVICE_TYPE_REMOTE_CAST,
    DEVICE_TYPE_USB_DEVICE,
    DEVICE_TYPE_ACCESSORY,
    DEVICE_TYPE_REMOTE_DAUDIO,
    DEVICE_TYPE_HDMI,
    DEVICE_TYPE_LINE_DIGITAL,
    DEVICE_TYPE_NEARLINK,
    DEVICE_TYPE_NEARLINK_IN,
    DEVICE_TYPE_FILE_SINK,
    DEVICE_TYPE_FILE_SOURCE,
    DEVICE_TYPE_EXTERN_CABLE,
    DEVICE_TYPE_DEFAULT,
    DEVICE_TYPE_USB_ARM_HEADSET,
    DEVICE_TYPE_MAX,
};

vector<AudioPin> AudioPinVec = {
    AUDIO_PIN_NONE,
    AUDIO_PIN_OUT_SPEAKER,
    AUDIO_PIN_OUT_HEADSET,
    AUDIO_PIN_OUT_LINEOUT,
    AUDIO_PIN_OUT_HDMI,
    AUDIO_PIN_OUT_USB,
    AUDIO_PIN_OUT_USB_EXT,
    AUDIO_PIN_OUT_EARPIECE,
    AUDIO_PIN_OUT_BLUETOOTH_SCO,
    AUDIO_PIN_OUT_DAUDIO_DEFAULT,
    AUDIO_PIN_OUT_HEADPHONE,
    AUDIO_PIN_OUT_USB_HEADSET,
    AUDIO_PIN_OUT_BLUETOOTH_A2DP,
    AUDIO_PIN_OUT_DP,
    AUDIO_PIN_OUT_NEARLINK,
    AUDIO_PIN_IN_MIC,
    AUDIO_PIN_IN_HS_MIC,
    AUDIO_PIN_IN_LINEIN,
    AUDIO_PIN_IN_USB_EXT,
    AUDIO_PIN_IN_BLUETOOTH_SCO_HEADSET,
    AUDIO_PIN_IN_DAUDIO_DEFAULT,
    AUDIO_PIN_IN_USB_HEADSET,
    AUDIO_PIN_IN_PENCIL,
    AUDIO_PIN_IN_UWB,
    AUDIO_PIN_IN_NEARLINK,
};

vector<DeviceCategory> DeviceCategoryVec = {
    CATEGORY_DEFAULT,
    BT_HEADPHONE,
    BT_SOUNDBOX,
    BT_CAR,
    BT_GLASSES,
    BT_WATCH,
    BT_HEARAID,
    BT_UNWEAR_HEADPHONE,
};

void OnStop()
{
    Bluetooth::BluetoothHost::GetDefaultHost().Close();
}

void HandleArmUsbDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioA2dpOffloadManager> audioA2dpOffloadManager = std::make_shared<AudioA2dpOffloadManager>();
    std::shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler =
        DelayedSingleton<AudioPolicyServerHandler>::GetInstance();
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioA2dpOffloadManager->Init();
    audioDeviceStatus.Init(audioA2dpOffloadManager, audioPolicyServerHandler);

    DeviceType deviceType = PickValue(fdp, DeviceTypeVec);
    DeviceRole deviceRole = PickValue(fdp, DeviceRoleVec);
    std::string address = "00:11:22:33:44:55";
    audioDeviceStatus.HandleArmUsbDevice(deviceType, deviceRole, address);

    audioDeviceStatus.DeInit();
    OnStop();
}

void NoNeedChangeUsbDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioA2dpOffloadManager> audioA2dpOffloadManager = std::make_shared<AudioA2dpOffloadManager>();
    std::shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler =
        DelayedSingleton<AudioPolicyServerHandler>::GetInstance();
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioA2dpOffloadManager->Init();
    audioDeviceStatus.Init(audioA2dpOffloadManager, audioPolicyServerHandler);

    std::string address = "00:11:22:33:44:55";
    audioDeviceStatus.NoNeedChangeUsbDevice(address);

    audioDeviceStatus.DeInit();
    OnStop();
}

void RehandlePnpDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioA2dpOffloadManager> audioA2dpOffloadManager = std::make_shared<AudioA2dpOffloadManager>();
    std::shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler =
        DelayedSingleton<AudioPolicyServerHandler>::GetInstance();
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioA2dpOffloadManager->Init();
    audioDeviceStatus.Init(audioA2dpOffloadManager, audioPolicyServerHandler);

    DeviceType deviceType = PickValue(fdp, DeviceTypeVec);
    DeviceRole deviceRole = PickValue(fdp, DeviceRoleVec);
    std::string address = "00:11:22:33:44:55";
    audioDeviceStatus.RehandlePnpDevice(deviceType, deviceRole, address);

    audioDeviceStatus.DeInit();
    OnStop();
}

void ReloadA2dpOffloadOnDeviceChangedFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioA2dpOffloadManager> audioA2dpOffloadManager = std::make_shared<AudioA2dpOffloadManager>();
    std::shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler =
        DelayedSingleton<AudioPolicyServerHandler>::GetInstance();
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioA2dpOffloadManager->Init();
    audioDeviceStatus.Init(audioA2dpOffloadManager, audioPolicyServerHandler);

    DeviceType deviceType = PickValue(fdp, DeviceTypeVec);
    std::string macAddress = "00:11:22:33:44:55";
    std::string deviceName = "usb_headset";
    AudioStreamInfo streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    int32_t classTypeCount = static_cast<int32_t>(ClassType::TYPE_INVALID) + 1;
    ClassType classType = static_cast<ClassType>(fdp.ConsumeIntegralInRange<int32_t>(0, classTypeCount - 1));
    AudioModuleInfo moduleInfo = {"className", "TEST", "TEST"};
    moduleInfo.name = "testModule";
    std::list<AudioModuleInfo> audioModuleListData = {};
    audioModuleListData.push_back(moduleInfo);
    audioDeviceStatus.audioConfigManager_.deviceClassInfo_[classType] = audioModuleListData;
    AudioIOHandle audioIoHandle = fdp.ConsumeIntegral<uint32_t>();
    audioDeviceStatus.audioIOHandleMap_.IOHandles_.insert({moduleInfo.name, audioIoHandle});
    audioDeviceStatus.ReloadA2dpOffloadOnDeviceChanged(deviceType, macAddress, deviceName, streamInfo);

    audioDeviceStatus.DeInit();
    OnStop();
}

void TriggerMicrophoneBlockedCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioA2dpOffloadManager> audioA2dpOffloadManager = std::make_shared<AudioA2dpOffloadManager>();
    std::shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler =
        DelayedSingleton<AudioPolicyServerHandler>::GetInstance();
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioA2dpOffloadManager->Init();
    audioDeviceStatus.Init(audioA2dpOffloadManager, audioPolicyServerHandler);

    vector<std::shared_ptr<AudioDeviceDescriptor>> desc;
    int32_t statusCount = static_cast<int32_t>(DeviceBlockStatus::DEVICE_BLOCKED) + 1;
    DeviceBlockStatus status = static_cast<DeviceBlockStatus>(
        fdp.ConsumeIntegralInRange<int32_t>(0, statusCount - 1));
    audioDeviceStatus.TriggerMicrophoneBlockedCallback(desc, status);

    audioDeviceStatus.DeInit();
    OnStop();
}

void OnDeviceStatusUpdatedFuzzTest(FuzzedDataProvider& fdp)
{
    DStatusInfo statusInfo;
    statusInfo.hdiPin = PickValue(fdp, AudioPinVec);
    statusInfo.macAddress = "00:11:22:33:44:55";
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioDeviceStatus.OnDeviceStatusUpdated(statusInfo, fdp.ConsumeBool());
    OnStop();
}

void GetDeviceTypeFromPinFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioA2dpOffloadManager> audioA2dpOffloadManager = std::make_shared<AudioA2dpOffloadManager>();
    std::shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler =
        DelayedSingleton<AudioPolicyServerHandler>::GetInstance();
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioA2dpOffloadManager->Init();
    audioDeviceStatus.Init(audioA2dpOffloadManager, audioPolicyServerHandler);

    AudioPin hdiPin = PickValue(fdp, AudioPinVec);
    audioDeviceStatus.GetDeviceTypeFromPin(hdiPin);

    audioDeviceStatus.DeInit();
    OnStop();
}

void HandleDistributedDeviceUpdateFuzzTest(FuzzedDataProvider& fdp)
{
    DStatusInfo statusInfo;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descForCb;
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    statusInfo.hdiPin = PickValue(fdp, AudioPinVec);
    statusInfo.deviceName = "test";
    statusInfo.macAddress = "00:11:22:33:44:55";
    statusInfo.isConnected = fdp.ConsumeBool();
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    audioDeviceStatus.HandleDistributedDeviceUpdate(statusInfo, descForCb, reason);
    OnStop();
}

void OnPreferredStateUpdatedFuzzTest(FuzzedDataProvider& fdp)
{
    AudioDeviceDescriptor desc;
    int32_t updateCommandCount = static_cast<int32_t>(DeviceInfoUpdateCommand::EXCEPTION_FLAG_UPDATE -
        DeviceInfoUpdateCommand::CATEGORY_UPDATE) + 1;
    DeviceInfoUpdateCommand updateCommand =
        static_cast<DeviceInfoUpdateCommand>(fdp.ConsumeIntegralInRange<int32_t>(1, updateCommandCount));
    AudioStreamDeviceChangeReasonExt::ExtEnum oldDevice =
        AudioStreamDeviceChangeReasonExt::ExtEnum::OLD_DEVICE_UNAVALIABLE;
    AudioStreamDeviceChangeReasonExt reason(oldDevice);
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    desc.deviceCategory_ = PickValue(fdp, DeviceCategoryVec);
    audioDeviceStatus.OnPreferredStateUpdated(desc, updateCommand, reason);
    OnStop();
}

void UpdateDeviceListFuzzTest(FuzzedDataProvider& fdp)
{
    AudioDeviceDescriptor updatedDesc;
    bool isConnected = fdp.ConsumeBool();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descForCb;
    AudioStreamDeviceChangeReasonExt::ExtEnum oldDevice =
        AudioStreamDeviceChangeReasonExt::ExtEnum::OLD_DEVICE_UNAVALIABLE;
    AudioStreamDeviceChangeReasonExt reason(oldDevice);
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    updatedDesc.deviceType_ = PickValue(fdp, DeviceTypeVec);
    updatedDesc.macAddress_ = "00:11:22:33:44:55";
    updatedDesc.deviceRole_ = PickValue(fdp, DeviceRoleVec);
    audioDeviceStatus.UpdateDeviceList(updatedDesc, isConnected, descForCb, reason);
    OnStop();
}

void TriggerDeviceChangedCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorSptr = std::make_shared<AudioDeviceDescriptor>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptorSptrVector;
    audioDeviceDescriptorSptrVector.push_back(audioDeviceDescriptorSptr);
    bool isConnected = fdp.ConsumeBool();
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioDeviceStatus.TriggerDeviceChangedCallback(audioDeviceDescriptorSptrVector, isConnected);
    OnStop();
}

void TriggerAvailableDeviceChangedCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorSptr = std::make_shared<AudioDeviceDescriptor>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptorSptrVector;
    audioDeviceDescriptorSptrVector.push_back(audioDeviceDescriptorSptr);
    bool isConnected = fdp.ConsumeBool();
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioDeviceStatus.DeInit();
    audioDeviceStatus.TriggerAvailableDeviceChangedCallback(audioDeviceDescriptorSptrVector, isConnected);
    OnStop();
}

void HandleDpDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    DeviceType deviceType = PickValue(fdp, DeviceTypeVec);
    std::string address = "00:11:22:33:44:55";
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioDeviceStatus.HandleDpDevice(deviceType, address);
    OnStop();
}

void HandleLocalDeviceConnectedFuzzTest(FuzzedDataProvider& fdp)
{
    AudioDeviceDescriptor updatedDesc;
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    updatedDesc.deviceType_ = PickValue(fdp, DeviceTypeVec);
    audioDeviceStatus.HandleLocalDeviceConnected(updatedDesc);
    OnStop();
}

void HandleSpecialDeviceTypeFuzzTest(FuzzedDataProvider& fdp)
{
    DeviceType deviceType = PickValue(fdp, DeviceTypeVec);
    DeviceRole deviceRole = PickValue(fdp, DeviceRoleVec);
    std::string address = "00:11:22:33:44:55";
    bool isConnected = fdp.ConsumeBool();
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioDeviceStatus.HandleSpecialDeviceType(deviceType, isConnected, address, deviceRole);
    OnStop();
}

void HandleLocalDeviceDisconnectedFuzzTest(FuzzedDataProvider& fdp)
{
    AudioDeviceDescriptor updatedDesc;
    updatedDesc.deviceType_ = PickValue(fdp, DeviceTypeVec);
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioDeviceStatus.HandleLocalDeviceDisconnected(updatedDesc);
    OnStop();
}

void UpdateActiveA2dpDeviceWhenDisconnectingFuzzTest(FuzzedDataProvider& fdp)
{
    std::string address = "00:11:22:33:44:55";
    std::string device = address;
    A2dpDeviceConfigInfo config;
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioDeviceStatus.audioA2dpDevice_.AddA2dpInDevice(device, config);
    audioDeviceStatus.UpdateActiveA2dpDeviceWhenDisconnecting(address);
    OnStop();
}

void OnPnpDeviceStatusUpdatedFuzzTest(FuzzedDataProvider& fdp)
{
    AudioDeviceDescriptor desc;
    desc.deviceType_ = PickValue(fdp, DeviceTypeVec);
    desc.macAddress_ = "00:11:22:33:44:55";
    desc.deviceName_ = "NONE";
    bool isConnected = fdp.ConsumeBool();
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioDeviceStatus.audioA2dpOffloadManager_ = std::make_shared<AudioA2dpOffloadManager>();
    audioDeviceStatus.hasModulesLoaded = fdp.ConsumeBool();
    audioDeviceStatus.OnPnpDeviceStatusUpdated(desc, isConnected);
    OnStop();
}

void IsConfigurationUpdatedFuzzTest(FuzzedDataProvider& fdp)
{
    DeviceType deviceType = PickValue(fdp, DeviceTypeVec);
    AudioStreamInfo streamInfo;
    AudioDeviceStatus audioDeviceStatus;
    audioDeviceStatus.IsConfigurationUpdated(deviceType, streamInfo);
    OnStop();
}

void OnForcedDeviceSelectedFuzzTest(FuzzedDataProvider& fdp)
{
    std::string macAddress = "00:11:22:33:44:55";
    DeviceType devType = PickValue(fdp, DeviceTypeVec);

    std::shared_ptr<AudioDeviceDescriptor> remoteDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    remoteDeviceDescriptor->deviceType_ = devType;
    remoteDeviceDescriptor->macAddress_ = "00:11:22:33:44:55";
    remoteDeviceDescriptor->deviceRole_ = PickValue(fdp, DeviceRoleVec);

    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioDeviceStatus.audioConnectedDevice_.AddConnectedDevice(remoteDeviceDescriptor);
    shared_ptr<AudioDeviceDescriptor> desc = make_shared<AudioDeviceDescriptor>();
    if (desc == nullptr) {
        return;
    }
    desc->deviceType_ = devType;
    desc->macAddress_ = macAddress;
    desc->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    audioDeviceStatus.audioDeviceManager_.connectedDevices_.push_back(desc);

    audioDeviceStatus.OnForcedDeviceSelected(devType, macAddress);
    OnStop();
}

void OpenPortAndAddDeviceOnServiceConnectedFuzzTest(FuzzedDataProvider& fdp)
{
    AudioModuleInfo moduleInfo;
    vector<string> moduleInfoNameList = {
        "file_source",
        "Built_in_mic",
        "Speaker",
    };
    moduleInfo.name = PickValue(fdp, moduleInfoNameList);
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioDeviceStatus.OpenPortAndAddDeviceOnServiceConnected(moduleInfo);
    OnStop();
}

void AudioDeviceStatusLoadAccessoryModuleFuzzTest(FuzzedDataProvider& fdp)
{
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    std::string deviceInfo = "testDeviceInfo";
    ClassType classType = static_cast<ClassType>(fdp.ConsumeIntegral<int32_t>());
    AudioModuleInfo moduleInfo;
    std::list<AudioModuleInfo> moduleInfoList;
    moduleInfoList.push_back(moduleInfo);
    audioDeviceStatus.audioConfigManager_.deviceClassInfo_.insert({classType, moduleInfoList});

    audioDeviceStatus.LoadAccessoryModule(deviceInfo);
    OnStop();
}

void AudioDeviceStatusOnDeviceConfigurationChangedFuzzTest(FuzzedDataProvider& fdp)
{
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    DeviceType deviceType = static_cast<DeviceType>(fdp.ConsumeIntegral<int32_t>());
    std::string macAddress = "00:11:22:33:44:55";
    std::string deviceName = "testDevice";
    AudioStreamInfo streamInfo;
    audioDeviceStatus.audioActiveDevice_.SetActiveBtDeviceMac("00:11:22:33:44:50");
    audioDeviceStatus.audioA2dpOffloadManager_ = std::make_shared<AudioA2dpOffloadManager>();

    audioDeviceStatus.OnDeviceConfigurationChanged(deviceType, macAddress, deviceName, streamInfo);
    OnStop();
}

void AudioDeviceStatusTriggerDeviceInfoUpdatedCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> descriptor = std::make_shared<AudioDeviceDescriptor>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc;
    desc.push_back(descriptor);
    audioDeviceStatus.TriggerDeviceInfoUpdatedCallback(desc);
    OnStop();
}

void AudioDeviceStatusActivateNewDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    std::string networkId = "testnetworkId";
    DeviceType deviceType = static_cast<DeviceType>(fdp.ConsumeIntegral<int32_t>());
    bool isRemote = fdp.ConsumeBool();
    audioDeviceStatus.ActivateNewDevice(networkId, deviceType, isRemote);
    OnStop();
}

void AudioDeviceStatusHandleOfflineDistributedDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> descriptor = std::make_shared<AudioDeviceDescriptor>();
    descriptor->networkId_ = LOCAL_NETWORK_ID;
    descriptor->deviceType_ = static_cast<DeviceType>(fdp.ConsumeIntegral<int32_t>());
    audioDeviceStatus.audioConnectedDevice_.connectedDevices_.push_back(descriptor);
    audioDeviceStatus.HandleOfflineDistributedDevice();
    OnStop();
}

void AudioDeviceStatusRestoreNewA2dpPortFuzzTest(FuzzedDataProvider& fdp)
{
    vector<string> moduleInfoNameList = {
        "sink",
        "Speaker",
    };
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs;
    std::shared_ptr<AudioStreamDescriptor> descriptor = std::make_shared<AudioStreamDescriptor>();
    streamDescs.push_back(descriptor);

    AudioModuleInfo moduleInfo;
    moduleInfo.role = PickValue(fdp, moduleInfoNameList);
    std::string currentActivePort = "test";
    audioDeviceStatus.GetDmDeviceType();
    audioDeviceStatus.RestoreNewA2dpPort(streamDescs, moduleInfo, currentActivePort);
    OnStop();
}

void AudioDeviceStatusGetPaIndexByPortNameFuzzTest(FuzzedDataProvider& fdp)
{
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    std::string portName = "testName";
    AudioIOHandle audioIOHandle = fdp.ConsumeIntegral<uint32_t>();
    audioDeviceStatus.audioIOHandleMap_.IOHandles_.insert({portName, audioIOHandle});
    audioDeviceStatus.GetPaIndexByPortName(portName);
    OnStop();
}

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
    HandleArmUsbDeviceFuzzTest,
    RehandlePnpDeviceFuzzTest,
    NoNeedChangeUsbDeviceFuzzTest,
    TriggerMicrophoneBlockedCallbackFuzzTest,
    ReloadA2dpOffloadOnDeviceChangedFuzzTest,
    GetDeviceTypeFromPinFuzzTest,
    OnDeviceStatusUpdatedFuzzTest,
    HandleDistributedDeviceUpdateFuzzTest,
    UpdateDeviceListFuzzTest,
    OnPreferredStateUpdatedFuzzTest,
    TriggerAvailableDeviceChangedCallbackFuzzTest,
    TriggerDeviceChangedCallbackFuzzTest,
    HandleDpDeviceFuzzTest,
    HandleLocalDeviceConnectedFuzzTest,
    HandleLocalDeviceDisconnectedFuzzTest,
    HandleSpecialDeviceTypeFuzzTest,
    OnPnpDeviceStatusUpdatedFuzzTest,
    UpdateActiveA2dpDeviceWhenDisconnectingFuzzTest,
    IsConfigurationUpdatedFuzzTest,
    OpenPortAndAddDeviceOnServiceConnectedFuzzTest,
    OnForcedDeviceSelectedFuzzTest,
    AudioDeviceStatusLoadAccessoryModuleFuzzTest,
    AudioDeviceStatusOnDeviceConfigurationChangedFuzzTest,
    AudioDeviceStatusTriggerDeviceInfoUpdatedCallbackFuzzTest,
    AudioDeviceStatusActivateNewDeviceFuzzTest,
    AudioDeviceStatusHandleOfflineDistributedDeviceFuzzTest,
    AudioDeviceStatusRestoreNewA2dpPortFuzzTest,
    AudioDeviceStatusGetPaIndexByPortNameFuzzTest,
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
extern "C" int LLVMFuzzerInitialize(const uint8_t* data, size_t size)
{
    OHOS::AudioStandard::Init();
    return 0;
}
