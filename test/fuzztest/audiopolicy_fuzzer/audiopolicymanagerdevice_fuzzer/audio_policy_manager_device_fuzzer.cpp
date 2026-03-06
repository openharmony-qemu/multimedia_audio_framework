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

#include <iostream>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include "audio_info.h"
#include "audio_policy_manager.h"
#include "audio_device_info.h"
#include "audio_utils.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "access_token.h"
#include <fuzzer/FuzzedDataProvider.h>

namespace OHOS {
namespace AudioStandard {
using namespace std;

const size_t THRESHOLD = 10;
static const uint8_t* RAW_DATA = nullptr;
static int32_t NUM_2 = 2;
static int32_t NUM_10 = 10;
static size_t g_dataSize = 0;
static size_t g_pos;

vector<AudioSampleFormat> AudioSampleFormatVec = {
    SAMPLE_U8,
    SAMPLE_S16LE,
    SAMPLE_S24LE,
    SAMPLE_S32LE,
    SAMPLE_F32LE,
};

vector<AudioSamplingRate> AudioSamplingRateVec = {
    SAMPLE_RATE_8000,
    SAMPLE_RATE_16000,
    SAMPLE_RATE_44100,
    SAMPLE_RATE_48000,
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

vector<AudioStreamType> AudioStreamTypeVec = {
    STREAM_DEFAULT,
    STREAM_VOICE_CALL,
    STREAM_MUSIC,
    STREAM_RING,
    STREAM_MEDIA,
    STREAM_VOICE_ASSISTANT,
    STREAM_SYSTEM,
    STREAM_ALARM,
    STREAM_NOTIFICATION,
    STREAM_BLUETOOTH_SCO,
    STREAM_ENFORCED_AUDIBLE,
    STREAM_DTMF,
    STREAM_TTS,
    STREAM_RECORDING,
    STREAM_MOVIE,
    STREAM_GAME,
    STREAM_SPEECH,
    STREAM_SYSTEM_ENFORCED,
    STREAM_ULTRASONIC,
    STREAM_WAKEUP,
    STREAM_VOICE_MESSAGE,
    STREAM_NAVIGATION,
    STREAM_INTERNAL_FORCE_STOP,
    STREAM_SOURCE_VOICE_CALL,
    STREAM_VOICE_COMMUNICATION,
    STREAM_VOICE_RING,
    STREAM_VOICE_CALL_ASSISTANT,
    STREAM_CAMCORDER,
    STREAM_APP,
    STREAM_TYPE_MAX,
    STREAM_ALL,
};

vector<DeviceFlag> DeviceFlagVec = {
    OUTPUT_DEVICES_FLAG,
    INPUT_DEVICES_FLAG,
    ALL_DEVICES_FLAG,
};

vector<AudioDeviceUsage> AudioDeviceUsageVec = {
    MEDIA_OUTPUT_DEVICES,
    MEDIA_INPUT_DEVICES,
    ALL_MEDIA_DEVICES,
    CALL_OUTPUT_DEVICES,
    CALL_INPUT_DEVICES,
    ALL_CALL_DEVICES,
    D_ALL_DEVICES,
};

vector<InternalDeviceType> InternalDeviceTypeVec = {
    DEVICE_TYPE_SPEAKER,
    DEVICE_TYPE_WIRED_HEADSET,
    DEVICE_TYPE_WIRED_HEADPHONES,
    DEVICE_TYPE_BLUETOOTH_SCO,
    DEVICE_TYPE_BLUETOOTH_A2DP,
    DEVICE_TYPE_USB_HEADSET,
    DEVICE_TYPE_USB_ARM_HEADSET,
    DEVICE_TYPE_NEARLINK,
    DEVICE_TYPE_HEARING_AID
};

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

void SelectOutputDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    sptr<AudioRendererFilter> audioRendererFilter = new AudioRendererFilter();
    audioRendererFilter->uid = GetData<int32_t>();
    audioRendererFilter->rendererInfo.contentType = static_cast<ContentType>(GetData<int32_t>() % NUM_10);
    audioRendererFilter->rendererInfo.streamUsage = static_cast<StreamUsage>(GetData<int32_t>() % NUM_10);
    
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    uint32_t deviceCount = GetData<uint32_t>() % 5 + 1;
    for (uint32_t i = 0; i < deviceCount; i++) {
        auto desc = std::make_shared<AudioDeviceDescriptor>();
        desc->deviceType_ = DeviceTypeVec[GetData<uint32_t>() % DeviceTypeVec.size()];
        desc->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
        devices.push_back(desc);
    }
    
    int32_t selectMode = GetData<int32_t>();
    AudioPolicyManager::GetInstance().SelectOutputDevice(audioRendererFilter, devices, selectMode);
}

void SelectPrivateDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    DeviceType devType = DeviceTypeVec[GetData<uint32_t>() % DeviceTypeVec.size()];
    std::string macAddress = fdp.ConsumeRandomLengthString(20);
    AudioPolicyManager::GetInstance().SelectPrivateDevice(devType, macAddress);
}

void ForceSelectDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    DeviceType devType = DeviceTypeVec[GetData<uint32_t>() % DeviceTypeVec.size()];
    std::string macAddress = fdp.ConsumeRandomLengthString(20);
    sptr<AudioRendererFilter> audioRendererFilter = new AudioRendererFilter();
    audioRendererFilter->uid = GetData<int32_t>();
    AudioPolicyManager::GetInstance().ForceSelectDevice(devType, macAddress, audioRendererFilter);
}

void SetActiveHfpDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    std::string macAddress = fdp.ConsumeRandomLengthString(20);
    AudioPolicyManager::GetInstance().SetActiveHfpDevice(macAddress);
}

void RestoreOutputDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    sptr<AudioRendererFilter> audioRendererFilter = new AudioRendererFilter();
    audioRendererFilter->uid = GetData<int32_t>();
    AudioPolicyManager::GetInstance().RestoreOutputDevice(audioRendererFilter);
}

void GetSelectedDeviceInfoFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t uid = GetData<int32_t>();
    int32_t pid = GetData<int32_t>();
    AudioStreamType streamType = AudioStreamTypeVec[GetData<uint32_t>() % AudioStreamTypeVec.size()];
    AudioPolicyManager::GetInstance().GetSelectedDeviceInfo(uid, pid, streamType);
}

void SelectInputDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    sptr<AudioCapturerFilter> audioCapturerFilter = new AudioCapturerFilter();
    audioCapturerFilter->uid = GetData<int32_t>();
    
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    uint32_t deviceCount = GetData<uint32_t>() % 5 + 1;
    for (uint32_t i = 0; i < deviceCount; i++) {
        auto desc = std::make_shared<AudioDeviceDescriptor>();
        desc->deviceType_ = DeviceTypeVec[GetData<uint32_t>() % DeviceTypeVec.size()];
        desc->deviceRole_ = DeviceRole::INPUT_DEVICE;
        devices.push_back(desc);
    }
    
    AudioPolicyManager::GetInstance().SelectInputDevice(audioCapturerFilter, devices);
}

void SelectInputDeviceSingleFuzzTest(FuzzedDataProvider& fdp)
{
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DeviceTypeVec[GetData<uint32_t>() % DeviceTypeVec.size()];
    desc->deviceRole_ = DeviceRole::INPUT_DEVICE;
    AudioPolicyManager::GetInstance().SelectInputDevice(desc);
}

void ExcludeOutputDevicesFuzzTest(FuzzedDataProvider& fdp)
{
    AudioDeviceUsage usage = AudioDeviceUsageVec[GetData<uint32_t>() % AudioDeviceUsageVec.size()];
    
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    uint32_t deviceCount = GetData<uint32_t>() % 5 + 1;
    for (uint32_t i = 0; i < deviceCount; i++) {
        auto desc = std::make_shared<AudioDeviceDescriptor>();
        desc->deviceType_ = DeviceTypeVec[GetData<uint32_t>() % DeviceTypeVec.size()];
        desc->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
        devices.push_back(desc);
    }
    
    AudioPolicyManager::GetInstance().ExcludeOutputDevices(usage, devices);
}

void UnexcludeOutputDevicesFuzzTest(FuzzedDataProvider& fdp)
{
    AudioDeviceUsage usage = AudioDeviceUsageVec[GetData<uint32_t>() % AudioDeviceUsageVec.size()];
    
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    uint32_t deviceCount = GetData<uint32_t>() % 5 + 1;
    for (uint32_t i = 0; i < deviceCount; i++) {
        auto desc = std::make_shared<AudioDeviceDescriptor>();
        desc->deviceType_ = DeviceTypeVec[GetData<uint32_t>() % DeviceTypeVec.size()];
        desc->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
        devices.push_back(desc);
    }
    
    AudioPolicyManager::GetInstance().UnexcludeOutputDevices(usage, devices);
}

void GetExcludedDevicesFuzzTest(FuzzedDataProvider& fdp)
{
    AudioDeviceUsage usage = AudioDeviceUsageVec[GetData<uint32_t>() % AudioDeviceUsageVec.size()];
    AudioPolicyManager::GetInstance().GetExcludedDevices(usage);
}

void GetDevicesFuzzTest(FuzzedDataProvider& fdp)
{
    DeviceFlag flag = DeviceFlagVec[GetData<uint32_t>() % DeviceFlagVec.size()];
    AudioPolicyManager::GetInstance().GetDevices(flag);
}

void GetDevicesInnerFuzzTest(FuzzedDataProvider& fdp)
{
    DeviceFlag flag = DeviceFlagVec[GetData<uint32_t>() % DeviceFlagVec.size()];
    AudioPolicyManager::GetInstance().GetDevicesInner(flag);
}

void GetPreferredOutputDeviceDescriptorsFuzzTest(FuzzedDataProvider& fdp)
{
    AudioRendererInfo rendererInfo;
    rendererInfo.contentType = static_cast<ContentType>(GetData<int32_t>() % NUM_10);
    rendererInfo.streamUsage = static_cast<StreamUsage>(GetData<int32_t>() % NUM_10);
    bool forceNoBTPermission = GetData<uint32_t>() % NUM_2;
    AudioPolicyManager::GetInstance().GetPreferredOutputDeviceDescriptors(rendererInfo, forceNoBTPermission);
}

void GetPreferredInputDeviceDescriptorsFuzzTest(FuzzedDataProvider& fdp)
{
    AudioCapturerInfo captureInfo;
    captureInfo.capturerFlags = GetData<int32_t>();
    captureInfo.sourceType = static_cast<SourceType>(GetData<int32_t>() % NUM_10);
    AudioPolicyManager::GetInstance().GetPreferredInputDeviceDescriptors(captureInfo);
}

void GetOutputDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    sptr<AudioRendererFilter> audioRendererFilter = new AudioRendererFilter();
    audioRendererFilter->uid = GetData<int32_t>();
    AudioPolicyManager::GetInstance().GetOutputDevice(audioRendererFilter);
}

void GetInputDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    sptr<AudioCapturerFilter> audioCapturerFilter = new AudioCapturerFilter();
    audioCapturerFilter->uid = GetData<int32_t>();
    AudioPolicyManager::GetInstance().GetInputDevice(audioCapturerFilter);
}

void SetDeviceActiveFuzzTest(FuzzedDataProvider& fdp)
{
    InternalDeviceType deviceType = InternalDeviceTypeVec[GetData<uint32_t>() % InternalDeviceTypeVec.size()];
    bool active = GetData<uint32_t>() % NUM_2;
    int32_t uid = GetData<int32_t>();
    AudioPolicyManager::GetInstance().SetDeviceActive(deviceType, active, uid);
}

void IsDeviceActiveFuzzTest(FuzzedDataProvider& fdp)
{
    InternalDeviceType deviceType = InternalDeviceTypeVec[GetData<uint32_t>() % InternalDeviceTypeVec.size()];
    AudioPolicyManager::GetInstance().IsDeviceActive(deviceType);
}

void GetActiveOutputDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    AudioPolicyManager::GetInstance().GetActiveOutputDevice();
}

void GetDmDeviceTypeFuzzTest(FuzzedDataProvider& fdp)
{
    AudioPolicyManager::GetInstance().GetDmDeviceType();
}

void GetActiveInputDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    AudioPolicyManager::GetInstance().GetActiveInputDevice();
}

void GetAvailableDevicesFuzzTest(FuzzedDataProvider& fdp)
{
    AudioDeviceUsage usage = AudioDeviceUsageVec[GetData<uint32_t>() % AudioDeviceUsageVec.size()];
    AudioPolicyManager::GetInstance().GetAvailableDevices(usage);
}

void GetSelectedInputDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    AudioPolicyManager::GetInstance().GetSelectedInputDevice();
}

void ClearSelectedInputDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    AudioPolicyManager::GetInstance().ClearSelectedInputDevice();
}

void PreferBluetoothAndNearlinkRecordFuzzTest(FuzzedDataProvider& fdp)
{
    BluetoothAndNearlinkPreferredRecordCategory category =
        static_cast<BluetoothAndNearlinkPreferredRecordCategory>(GetData<int32_t>() % 5);
    AudioPolicyManager::GetInstance().PreferBluetoothAndNearlinkRecord(category);
}

void GetPreferBluetoothAndNearlinkRecordFuzzTest(FuzzedDataProvider& fdp)
{
    AudioPolicyManager::GetInstance().GetPreferBluetoothAndNearlinkRecord();
}

void SetCallDeviceActiveFuzzTest(FuzzedDataProvider& fdp)
{
    InternalDeviceType deviceType = InternalDeviceTypeVec[GetData<uint32_t>() % InternalDeviceTypeVec.size()];
    bool active = GetData<uint32_t>() % NUM_2;
    std::string address = fdp.ConsumeRandomLengthString(20);
    int32_t uid = GetData<int32_t>();
    AudioPolicyManager::GetInstance().SetCallDeviceActive(deviceType, active, address, uid);
}

void GetActiveBluetoothDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    AudioPolicyManager::GetInstance().GetActiveBluetoothDevice();
}

void TriggerFetchDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = GetData<AudioStreamDeviceChangeReasonExt::ExtEnum>();
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    AudioPolicyManager::GetInstance().TriggerFetchDevice(reason);
}

void SetPreferredDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    PreferredType preferredType = static_cast<PreferredType>(GetData<int32_t>() % 5);
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DeviceTypeVec[GetData<uint32_t>() % DeviceTypeVec.size()];
    int32_t uid = GetData<int32_t>();
    AudioPolicyManager::GetInstance().SetPreferredDevice(preferredType, desc, uid);
}

void SetDeviceVolumeBehaviorFuzzTest(FuzzedDataProvider& fdp)
{
    std::string networkId = fdp.ConsumeRandomLengthString(20);
    DeviceType deviceType = DeviceTypeVec[GetData<uint32_t>() % DeviceTypeVec.size()];
    VolumeBehavior volumeBehavior;
    AudioPolicyManager::GetInstance().SetDeviceVolumeBehavior(networkId, deviceType, volumeBehavior);
}

void SetDeviceConnectionStatusFuzzTest(FuzzedDataProvider& fdp)
{
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DeviceTypeVec[GetData<uint32_t>() % DeviceTypeVec.size()];
    bool isConnected = GetData<uint32_t>() % NUM_2;
    AudioPolicyManager::GetInstance().SetDeviceConnectionStatus(desc, isConnected);
}

void UpdateDeviceInfoFuzzTest(FuzzedDataProvider& fdp)
{
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DeviceTypeVec[GetData<uint32_t>() % DeviceTypeVec.size()];
    DeviceInfoUpdateCommand command = static_cast<DeviceInfoUpdateCommand>(GetData<int32_t>() % 10);
    AudioPolicyManager::GetInstance().UpdateDeviceInfo(desc, command);
}

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
    SelectOutputDeviceFuzzTest,
    SelectPrivateDeviceFuzzTest,
    ForceSelectDeviceFuzzTest,
    SetActiveHfpDeviceFuzzTest,
    RestoreOutputDeviceFuzzTest,
    GetSelectedDeviceInfoFuzzTest,
    SelectInputDeviceFuzzTest,
    SelectInputDeviceSingleFuzzTest,
    ExcludeOutputDevicesFuzzTest,
    UnexcludeOutputDevicesFuzzTest,
    GetExcludedDevicesFuzzTest,
    GetDevicesFuzzTest,
    GetDevicesInnerFuzzTest,
    GetPreferredOutputDeviceDescriptorsFuzzTest,
    GetPreferredInputDeviceDescriptorsFuzzTest,
    GetOutputDeviceFuzzTest,
    GetInputDeviceFuzzTest,
    SetDeviceActiveFuzzTest,
    IsDeviceActiveFuzzTest,
    GetActiveOutputDeviceFuzzTest,
    GetDmDeviceTypeFuzzTest,
    GetActiveInputDeviceFuzzTest,
    GetAvailableDevicesFuzzTest,
    GetSelectedInputDeviceFuzzTest,
    ClearSelectedInputDeviceFuzzTest,
    PreferBluetoothAndNearlinkRecordFuzzTest,
    GetPreferBluetoothAndNearlinkRecordFuzzTest,
    SetCallDeviceActiveFuzzTest,
    GetActiveBluetoothDeviceFuzzTest,
    TriggerFetchDeviceFuzzTest,
    SetPreferredDeviceFuzzTest,
    SetDeviceVolumeBehaviorFuzzTest,
    SetDeviceConnectionStatusFuzzTest,
    UpdateDeviceInfoFuzzTest,
    });
    func(fdp);
}

void Init(const uint8_t* data, size_t size)
{
    if (data == nullptr) {
        return;
    }
    RAW_DATA = data;
    g_dataSize = size;
    g_pos = 0;
}

void Init()
{
}

} // namespace AudioStandard
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }
    OHOS::AudioStandard::Init(data, size);
    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::Test(fdp);
    return 0;
}

extern "C" int LLVMFuzzerInitialize(const uint8_t* data, size_t size)
{
    OHOS::AudioStandard::Init();
    return 0;
}
