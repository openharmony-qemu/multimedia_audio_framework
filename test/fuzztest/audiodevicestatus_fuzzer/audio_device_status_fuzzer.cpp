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
#include <cstring>
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

namespace OHOS {
namespace AudioStandard {
using namespace std;

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
static int32_t NUM_2 = 2;

typedef void (*TestFuncs)();

template<class T>
T GetData()
{
    T object {};
    size_t objectSize = sizeof(object);
    if (RAW_DATA == nullptr || objectSize > g_dataSize - g_pos) {
        return object;
    }
    errno_t ret = memcpy_s(&object, objectSize, RAW_DATA + g_pos, objectSize);
    if (ret != EOK) {
        return {};
    }
    g_pos += objectSize;
    return object;
}

template<class T>
uint32_t GetArrLength(T& arr)
{
    if (arr == nullptr) {
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
        return 0;
    }
    return sizeof(arr) / sizeof(arr[0]);
}

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

vector<DeviceRole> DeviceRoleVec = {
    DEVICE_ROLE_NONE,
    INPUT_DEVICE,
    OUTPUT_DEVICE,
    DEVICE_ROLE_MAX,
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

void HandleArmUsbDeviceFuzzTest()
{
    std::shared_ptr<AudioA2dpOffloadManager> audioA2dpOffloadManager = std::make_shared<AudioA2dpOffloadManager>();
    std::shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler =
        DelayedSingleton<AudioPolicyServerHandler>::GetInstance();
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioA2dpOffloadManager->Init();
    audioDeviceStatus.Init(audioA2dpOffloadManager, audioPolicyServerHandler);

    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    DeviceType deviceType = DeviceTypeVec[deviceTypeCount];
    uint32_t roleCount = GetData<uint32_t>() % DeviceRoleVec.size();
    DeviceRole deviceRole = DeviceRoleVec[roleCount];
    std::string address = "00:11:22:33:44:55";
    audioDeviceStatus.HandleArmUsbDevice(deviceType, deviceRole, address);

    audioDeviceStatus.DeInit();
}

void RehandlePnpDeviceFuzzTest()
{
    std::shared_ptr<AudioA2dpOffloadManager> audioA2dpOffloadManager = std::make_shared<AudioA2dpOffloadManager>();
    std::shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler =
        DelayedSingleton<AudioPolicyServerHandler>::GetInstance();
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioA2dpOffloadManager->Init();
    audioDeviceStatus.Init(audioA2dpOffloadManager, audioPolicyServerHandler);

    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    DeviceType deviceType = DeviceTypeVec[deviceTypeCount];
    uint32_t roleCount = GetData<uint32_t>() % DeviceRoleVec.size();
    DeviceRole deviceRole = DeviceRoleVec[roleCount];
    std::string address = "00:11:22:33:44:55";
    audioDeviceStatus.RehandlePnpDevice(deviceType, deviceRole, address);

    audioDeviceStatus.DeInit();
}

void NoNeedChangeUsbDeviceFuzzTest()
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
}

void TriggerMicrophoneBlockedCallbackFuzzTest()
{
    std::shared_ptr<AudioA2dpOffloadManager> audioA2dpOffloadManager = std::make_shared<AudioA2dpOffloadManager>();
    std::shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler =
        DelayedSingleton<AudioPolicyServerHandler>::GetInstance();
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioA2dpOffloadManager->Init();
    audioDeviceStatus.Init(audioA2dpOffloadManager, audioPolicyServerHandler);

    vector<std::shared_ptr<AudioDeviceDescriptor>> desc;
    int32_t statusCount = static_cast<int32_t>(DeviceBlockStatus::DEVICE_BLOCKED) + 1;
    DeviceBlockStatus status = static_cast<DeviceBlockStatus>(GetData<uint8_t>() % statusCount);
    audioDeviceStatus.TriggerMicrophoneBlockedCallback(desc, status);

    audioDeviceStatus.DeInit();
}

void ReloadA2dpOffloadOnDeviceChangedFuzzTest()
{
    std::shared_ptr<AudioA2dpOffloadManager> audioA2dpOffloadManager = std::make_shared<AudioA2dpOffloadManager>();
    std::shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler =
        DelayedSingleton<AudioPolicyServerHandler>::GetInstance();
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioA2dpOffloadManager->Init();
    audioDeviceStatus.Init(audioA2dpOffloadManager, audioPolicyServerHandler);

    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    DeviceType deviceType = DeviceTypeVec[deviceTypeCount];
    std::string macAddress = "00:11:22:33:44:55";
    std::string deviceName = "usb_headset";
    AudioStreamInfo streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    std::unique_ptr <AudioAffinityManager> audioAffinityManager = std::make_unique<AudioAffinityManager>();
    int32_t classTypeCount = static_cast<int32_t>(ClassType::TYPE_INVALID) + 1;
    ClassType classType = static_cast<ClassType>(GetData<uint8_t>() % classTypeCount);
    AudioModuleInfo moduleInfo = {"className", "TEST", "TEST"};
    moduleInfo.name = "testModule";
    std::list<AudioModuleInfo> audioModuleListData = {};
    audioModuleListData.push_back(moduleInfo);
    audioDeviceStatus.audioConfigManager_.deviceClassInfo_[classType] = audioModuleListData;
    AudioIOHandle audioIoHandle = GetData<uint32_t>();
    audioDeviceStatus.audioIOHandleMap_.IOHandles_.insert({moduleInfo.name, audioIoHandle});
    audioDeviceStatus.ReloadA2dpOffloadOnDeviceChanged(deviceType, macAddress, deviceName, streamInfo);

    audioDeviceStatus.DeInit();
}

void GetDeviceTypeFromPinFuzzTest()
{
    std::shared_ptr<AudioA2dpOffloadManager> audioA2dpOffloadManager = std::make_shared<AudioA2dpOffloadManager>();
    std::shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler =
        DelayedSingleton<AudioPolicyServerHandler>::GetInstance();
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioA2dpOffloadManager->Init();
    audioDeviceStatus.Init(audioA2dpOffloadManager, audioPolicyServerHandler);

    uint32_t hdiPinCount = GetData<uint32_t>() % AudioPinVec.size();
    AudioPin hdiPin = AudioPinVec[hdiPinCount];
    audioDeviceStatus.GetDeviceTypeFromPin(hdiPin);

    audioDeviceStatus.DeInit();
}

void OnDeviceStatusUpdatedFuzzTest()
{
    DStatusInfo statusInfo;
    uint32_t hdiPinCount = GetData<uint32_t>() % AudioPinVec.size();
    statusInfo.hdiPin = AudioPinVec[hdiPinCount];
    statusInfo.macAddress = "00:11:22:33:44:55";
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioDeviceStatus.OnDeviceStatusUpdated(statusInfo, GetData<uint32_t>() % NUM_2);
}

void HandleDistributedDeviceUpdateFuzzTest()
{
    DStatusInfo statusInfo;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descForCb;
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    uint32_t hdiPinCount = GetData<uint32_t>() % AudioPinVec.size();
    statusInfo.hdiPin = AudioPinVec[hdiPinCount];
    statusInfo.deviceName = "test";
    statusInfo.macAddress = "00:11:22:33:44:55";
    statusInfo.isConnected = GetData<uint32_t>() % NUM_2;
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    audioDeviceStatus.HandleDistributedDeviceUpdate(statusInfo, descForCb, reason);
}

void UpdateDeviceListFuzzTest()
{
    AudioDeviceDescriptor updatedDesc;
    bool isConnected = GetData<uint32_t>() % NUM_2;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descForCb;
    AudioStreamDeviceChangeReasonExt::ExtEnum oldDevice =
        AudioStreamDeviceChangeReasonExt::ExtEnum::OLD_DEVICE_UNAVALIABLE;
    AudioStreamDeviceChangeReasonExt reason(oldDevice);
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    updatedDesc.deviceType_ = DeviceTypeVec[deviceTypeCount];
    updatedDesc.macAddress_ = "00:11:22:33:44:55";
    uint32_t roleCount = GetData<uint32_t>() % DeviceRoleVec.size();
    updatedDesc.deviceRole_ = DeviceRoleVec[roleCount];
    audioDeviceStatus.UpdateDeviceList(updatedDesc, isConnected, descForCb, reason);
}

void OnPreferredStateUpdatedFuzzTest()
{
    AudioDeviceDescriptor desc;
    int32_t updateCommandCount = static_cast<int32_t>(DeviceInfoUpdateCommand::EXCEPTION_FLAG_UPDATE -
        DeviceInfoUpdateCommand::CATEGORY_UPDATE) + 1;
    DeviceInfoUpdateCommand updateCommand =
        static_cast<DeviceInfoUpdateCommand>(GetData<uint8_t>() % updateCommandCount + 1);
    AudioStreamDeviceChangeReasonExt::ExtEnum oldDevice =
        AudioStreamDeviceChangeReasonExt::ExtEnum::OLD_DEVICE_UNAVALIABLE;
    AudioStreamDeviceChangeReasonExt reason(oldDevice);
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    uint32_t deviceCategoryCount = GetData<uint32_t>() % DeviceCategoryVec.size();
    desc.deviceCategory_ = DeviceCategoryVec[deviceCategoryCount];
    audioDeviceStatus.OnPreferredStateUpdated(desc, updateCommand, reason);
}

void CheckAndActiveHfpDeviceFuzzTest()
{
    AudioDeviceDescriptor desc;
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    desc.deviceType_ = DeviceTypeVec[deviceTypeCount];
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioDeviceStatus.CheckAndActiveHfpDevice(desc);
}

void TriggerAvailableDeviceChangedCallbackFuzzTest()
{
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorSptr = std::make_shared<AudioDeviceDescriptor>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptorSptrVector;
    audioDeviceDescriptorSptrVector.push_back(audioDeviceDescriptorSptr);
    bool isConnected = GetData<uint32_t>() % NUM_2;
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioDeviceStatus.DeInit();
    audioDeviceStatus.TriggerAvailableDeviceChangedCallback(audioDeviceDescriptorSptrVector, isConnected);
}

void TriggerDeviceChangedCallbackFuzzTest()
{
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorSptr = std::make_shared<AudioDeviceDescriptor>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptorSptrVector;
    audioDeviceDescriptorSptrVector.push_back(audioDeviceDescriptorSptr);
    bool isConnected = GetData<uint32_t>() % NUM_2;
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioDeviceStatus.TriggerDeviceChangedCallback(audioDeviceDescriptorSptrVector, isConnected);
}

void HandleDpDeviceFuzzTest()
{
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    DeviceType deviceType = DeviceTypeVec[deviceTypeCount];
    std::string address = "00:11:22:33:44:55";
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioDeviceStatus.HandleDpDevice(deviceType, address);
}

void HandleLocalDeviceConnectedFuzzTest()
{
    AudioDeviceDescriptor updatedDesc;
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    updatedDesc.deviceType_ = DeviceTypeVec[deviceTypeCount];
    audioDeviceStatus.HandleLocalDeviceConnected(updatedDesc);
}

void HandleLocalDeviceDisconnectedFuzzTest()
{
    AudioDeviceDescriptor updatedDesc;
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    updatedDesc.deviceType_ = DeviceTypeVec[deviceTypeCount];
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioDeviceStatus.HandleLocalDeviceDisconnected(updatedDesc);
}

void HandleSpecialDeviceTypeFuzzTest()
{
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    DeviceType deviceType = DeviceTypeVec[deviceTypeCount];
    uint32_t roleCount = GetData<uint32_t>() % DeviceRoleVec.size();
    DeviceRole deviceRole = DeviceRoleVec[roleCount];
    std::string address = "00:11:22:33:44:55";
    bool isConnected = GetData<uint32_t>() % NUM_2;
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioDeviceStatus.HandleSpecialDeviceType(deviceType, isConnected, address, deviceRole);
}

void OnPnpDeviceStatusUpdatedFuzzTest()
{
    AudioDeviceDescriptor desc;
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    desc.deviceType_ = DeviceTypeVec[deviceTypeCount];
    desc.macAddress_ = "00:11:22:33:44:55";
    desc.deviceName_ = "NONE";
    bool isConnected = GetData<uint32_t>() % NUM_2;
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioDeviceStatus.audioA2dpOffloadManager_ = std::make_shared<AudioA2dpOffloadManager>();
    audioDeviceStatus.hasModulesLoaded = GetData<bool>();
    audioDeviceStatus.OnPnpDeviceStatusUpdated(desc, isConnected);
}

void UpdateActiveA2dpDeviceWhenDisconnectingFuzzTest()
{
    std::string address = "00:11:22:33:44:55";
    std::string device = address;
    A2dpDeviceConfigInfo config;
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioDeviceStatus.audioA2dpDevice_.AddA2dpInDevice(device, config);
    audioDeviceStatus.UpdateActiveA2dpDeviceWhenDisconnecting(address);
}

void IsConfigurationUpdatedFuzzTest()
{
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    DeviceType deviceType = DeviceTypeVec[deviceTypeCount];
    AudioStreamInfo streamInfo;
    AudioDeviceStatus audioDeviceStatus;
    audioDeviceStatus.IsConfigurationUpdated(deviceType, streamInfo);
}

void OpenPortAndAddDeviceOnServiceConnectedFuzzTest()
{
    AudioModuleInfo moduleInfo;
    vector<string> moduleInfoNameList = {
        "file_source",
        "Built_in_mic",
        "Speaker",
    };
    uint32_t moduleInfoNameCount = GetData<uint32_t>() % moduleInfoNameList.size();
    moduleInfo.name = moduleInfoNameList[moduleInfoNameCount];
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioDeviceStatus.OpenPortAndAddDeviceOnServiceConnected(moduleInfo);
}

void OnForcedDeviceSelectedFuzzTest()
{
    std::string macAddress = "00:11:22:33:44:55";
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    DeviceType devType = DeviceTypeVec[deviceTypeCount];

    std::shared_ptr<AudioDeviceDescriptor> remoteDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    remoteDeviceDescriptor->deviceType_ = devType;
    remoteDeviceDescriptor->macAddress_ = "00:11:22:33:44:55";
    uint32_t roleCount = GetData<uint32_t>() % DeviceRoleVec.size();
    remoteDeviceDescriptor->deviceRole_ = DeviceRoleVec[roleCount];

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
}

void AudioDeviceStatusLoadAccessoryModuleFuzzTest()
{
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    std::string deviceInfo = "testDeviceInfo";
    ClassType classType = GetData<ClassType>();
    AudioModuleInfo moduleInfo;
    std::list<AudioModuleInfo> moduleInfoList;
    moduleInfoList.push_back(moduleInfo);
    audioDeviceStatus.audioConfigManager_.deviceClassInfo_.insert({classType, moduleInfoList});

    audioDeviceStatus.LoadAccessoryModule(deviceInfo);
}

void AudioDeviceStatusOnDeviceConfigurationChangedFuzzTest()
{
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    DeviceType deviceType = GetData<DeviceType>();
    std::string macAddress = "00:11:22:33:44:55";
    std::string deviceName = "testDevice";
    AudioStreamInfo streamInfo;
    audioDeviceStatus.audioActiveDevice_.SetActiveBtDeviceMac("00:11:22:33:44:50");
    audioDeviceStatus.audioA2dpOffloadManager_ = std::make_shared<AudioA2dpOffloadManager>();

    audioDeviceStatus.OnDeviceConfigurationChanged(deviceType, macAddress, deviceName, streamInfo);
}

TestFuncs g_testFuncs[] = {
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
    CheckAndActiveHfpDeviceFuzzTest,
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
};

bool FuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr) {
        return false;
    }

    // initialize data
    RAW_DATA = rawData;
    g_dataSize = size;
    g_pos = 0;

    uint32_t code = GetData<uint32_t>();
    uint32_t len = GetArrLength(g_testFuncs);
    if (len > 0) {
        g_testFuncs[code % len]();
    } else {
        AUDIO_INFO_LOG("%{public}s: The len length is equal to 0", __func__);
    }

    return true;
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::FuzzTest(data, size);
    return 0;
}
