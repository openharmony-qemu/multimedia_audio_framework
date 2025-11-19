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
#include "hpae_manager.h"
#include "audio_info.h"
#include "bluetooth_host.h"

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

vector<AudioDeviceUsage> AudioDeviceUsageVec = {
    MEDIA_OUTPUT_DEVICES,
    MEDIA_INPUT_DEVICES,
    ALL_MEDIA_DEVICES,
    CALL_OUTPUT_DEVICES,
    CALL_INPUT_DEVICES,
    ALL_CALL_DEVICES,
    D_ALL_DEVICES,
};

const vector<SourceType> g_testSourceTypes = {
    SOURCE_TYPE_INVALID,
    SOURCE_TYPE_MIC,
    SOURCE_TYPE_VOICE_RECOGNITION,
    SOURCE_TYPE_PLAYBACK_CAPTURE,
    SOURCE_TYPE_WAKEUP,
    SOURCE_TYPE_VOICE_CALL,
    SOURCE_TYPE_VOICE_COMMUNICATION,
    SOURCE_TYPE_ULTRASONIC,
    SOURCE_TYPE_VIRTUAL_CAPTURE,
    SOURCE_TYPE_VOICE_MESSAGE,
    SOURCE_TYPE_REMOTE_CAST,
    SOURCE_TYPE_VOICE_TRANSCRIPTION,
    SOURCE_TYPE_CAMCORDER,
    SOURCE_TYPE_UNPROCESSED,
    SOURCE_TYPE_EC,
    SOURCE_TYPE_MIC_REF,
    SOURCE_TYPE_LIVE,
    SOURCE_TYPE_MAX,
};

void OnStop()
{
    Bluetooth::BluetoothHost::GetDefaultHost().Close();
}

void GetActiveA2dpDeviceStreamInfoFuzzTest()
{
    AudioStreamInfo streamInfo;
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    DeviceType deviceType = DeviceTypeVec[deviceTypeCount];
    audioActiveDevice->GetActiveA2dpDeviceStreamInfo(deviceType, streamInfo);
    OnStop();
}

void GetMaxAmplitudeFuzzTest()
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    int32_t deviceId = AudioActiveDevice::GetInstance().GetCurrentInputDevice().deviceId_;
    AudioInterrupt audioInterrupt;
    audioActiveDevice->GetMaxAmplitude(deviceId, audioInterrupt);
    OnStop();
}

void UpdateDeviceFuzzTest()
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    DeviceType deviceType = DeviceTypeVec[deviceTypeCount];
    auto desc = std::make_shared<AudioDeviceDescriptor>(deviceType);
    int32_t reasonCount = static_cast<int32_t>(AudioStreamDeviceChangeReason::OVERRODE) + 1;
    auto reason_ = static_cast<AudioStreamDeviceChangeReason>(GetData<uint8_t>() % reasonCount);
    AudioStreamDeviceChangeReasonExt reason(reason_);
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->clientUID = GetData<int32_t>();
    rendererChangeInfo->createrUID = GetData<int32_t>();
    rendererChangeInfo->sessionId = GetData<int32_t>();
    audioActiveDevice->UpdateDevice(desc, reason, rendererChangeInfo);
    OnStop();
}

void HandleActiveBtFuzzTest()
{
    std::string macAddress = "test";
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    AudioDeviceDescriptor deviceDescriptor;
    deviceDescriptor.deviceType_ = DeviceTypeVec[deviceTypeCount];
    audioActiveDevice->SetCurrentOutputDevice(deviceDescriptor);
    deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    DeviceType deviceType = DeviceTypeVec[deviceTypeCount];
    audioActiveDevice->HandleActiveBt(deviceType, macAddress);
    OnStop();
}

void HandleNegtiveBtFuzzTest()
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    AudioDeviceDescriptor deviceDescriptor;
    deviceDescriptor.deviceType_ = DeviceTypeVec[deviceTypeCount];
    audioActiveDevice->SetCurrentOutputDevice(deviceDescriptor);
    deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    DeviceType deviceType = DeviceTypeVec[deviceTypeCount];
    audioActiveDevice->HandleNegtiveBt(deviceType);
    OnStop();
}

void SetDeviceActiveFuzzTest()
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    uint32_t usageCount = GetData<uint32_t>() % AudioDeviceUsageVec.size();
    AudioDeviceUsage usage = AudioDeviceUsageVec[usageCount];
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> callDevices =
        AudioPolicyUtils::GetInstance().GetAvailableDevicesInner(usage);
    for (const auto &desc : callDevices) {
        uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
        DeviceType deviceType = DeviceTypeVec[deviceTypeCount];
        bool active = GetData<uint32_t>() % NUM_2;
        int32_t uid = GetData<int32_t>();
        audioActiveDevice->SetDeviceActive(deviceType, active, uid);
    }
    OnStop();
}

void SetCallDeviceActiveFuzzTest()
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    uint32_t usageCount = GetData<uint32_t>() % AudioDeviceUsageVec.size();
    AudioDeviceUsage usage = AudioDeviceUsageVec[usageCount];
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> callDevices =
        AudioPolicyUtils::GetInstance().GetAvailableDevicesInner(usage);
    for (const auto &desc : callDevices) {
        bool active = GetData<uint32_t>() % NUM_2;
        audioActiveDevice->SetCallDeviceActive(desc->deviceType_, active, desc->macAddress_);
    }
    OnStop();
}

void IsDirectSupportedDeviceFuzzTest()
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    DeviceType deviceType = DeviceTypeVec[deviceTypeCount];
    uint32_t roleCount = GetData<uint32_t>() % DeviceRoleVec.size();
    DeviceRole role = DeviceRoleVec[roleCount];
    AudioDeviceDescriptor audioDeviceDescriptor(deviceType, role);
    audioActiveDevice->SetCurrentOutputDevice(audioDeviceDescriptor);
    audioActiveDevice->IsDirectSupportedDevice();
    OnStop();
}

void IsDeviceActiveFuzzTest()
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    DeviceType deviceType = DeviceTypeVec[deviceTypeCount];
    uint32_t roleCount = GetData<uint32_t>() % DeviceRoleVec.size();
    DeviceRole role = DeviceRoleVec[roleCount];
    AudioDeviceDescriptor audioDeviceDescriptor(deviceType, role);
    audioActiveDevice->SetCurrentOutputDevice(audioDeviceDescriptor);
    deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    deviceType = DeviceTypeVec[deviceTypeCount];
    audioActiveDevice->IsDeviceActive(deviceType);
    OnStop();
}

void AudioActiveDeviceGetCurrentOutputDeviceCategoryFuzzTest()
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    if (audioActiveDevice == nullptr) {
        return;
    }
    audioActiveDevice->GetCurrentInputDeviceMacAddr();
    audioActiveDevice->GetCurrentOutputDeviceCategory();
    OnStop();
}

void AudioActiveDeviceNotifyUserSelectionEventToBtFuzzTest()
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    if (audioActiveDevice == nullptr || audioDeviceDescriptor == nullptr || DeviceTypeVec.size() == 0) {
        return;
    }
    audioDeviceDescriptor->deviceType_ = DeviceTypeVec[GetData<uint32_t>() % DeviceTypeVec.size()];
    audioActiveDevice->currentActiveInputDevice_.deviceType_ =
        DeviceTypeVec[GetData<uint32_t>() % DeviceTypeVec.size()];
    StreamUsage streamUsage = STREAM_USAGE_ALARM;
    audioActiveDevice->NotifyUserSelectionEventToBt(audioDeviceDescriptor, streamUsage);
    OnStop();
}

void AudioActiveDeviceNotifyUserDisSelectionEventToBtFuzzTest()
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    if (audioActiveDevice == nullptr || audioDeviceDescriptor == nullptr || DeviceTypeVec.size() == 0) {
        return;
    }
    audioDeviceDescriptor->deviceType_ = DeviceTypeVec[GetData<uint32_t>() % DeviceTypeVec.size()];
    audioActiveDevice->currentActiveInputDevice_.deviceType_ =
        DeviceTypeVec[GetData<uint32_t>() % DeviceTypeVec.size()];
    audioActiveDevice->NotifyUserDisSelectionEventToBt(audioDeviceDescriptor);
    OnStop();
}

void AudioActiveDeviceNotifyUserSelectionEventForInputFuzzTest()
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    if (audioActiveDevice == nullptr || audioDeviceDescriptor == nullptr || DeviceTypeVec.size() == 0
        || g_testSourceTypes.size() == 0) {
        return;
    }
    audioDeviceDescriptor->deviceType_ = DeviceTypeVec[GetData<uint32_t>() % DeviceTypeVec.size()];
    audioActiveDevice->currentActiveInputDevice_.deviceType_ =
        DeviceTypeVec[GetData<uint32_t>() % DeviceTypeVec.size()];
    SourceType sourceType = g_testSourceTypes[GetData<uint32_t>() % g_testSourceTypes.size()];
    audioActiveDevice->NotifyUserSelectionEventForInput(audioDeviceDescriptor, sourceType);
    OnStop();
}

void AudioActiveDeviceSetDeviceActiveFuzzTest()
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    if (audioActiveDevice == nullptr || DeviceTypeVec.size() == 0) {
        return;
    }
    DeviceType deviceType = DeviceTypeVec[GetData<uint32_t>() % DeviceTypeVec.size()];
    bool active = GetData<uint32_t>() % NUM_2;
    int32_t uid = GetData<int32_t>();
    audioActiveDevice->SetDeviceActive(deviceType, active, uid);
    OnStop();
}

void AudioActiveDeviceSetCallDeviceActiveFuzzTest()
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    if (audioActiveDevice == nullptr || DeviceTypeVec.size() == 0) {
        return;
    }
    DeviceType deviceType = DeviceTypeVec[GetData<uint32_t>() % DeviceTypeVec.size()];
    bool active = GetData<uint32_t>() % NUM_2;
    int32_t uid = GetData<int32_t>();
    std::string address = "testAddress";
    audioActiveDevice->SetCallDeviceActive(deviceType, active, address, uid);
    OnStop();
}

void AudioActiveDeviceIsDeviceInVectorFuzzTest()
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    if (audioActiveDevice == nullptr) {
        return;
    }
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    descs.push_back(desc);
    audioActiveDevice->IsDeviceInVector(desc, descs);
    OnStop();
}

void AudioDeviceDescriptorSetClientInfoFuzzTest()
{
    AudioDeviceDescriptor deviceDescriptor;
    AudioDeviceDescriptor::ClientInfo clientInfo;
    deviceDescriptor.GetDeviceCategory();
    deviceDescriptor.SetClientInfo(clientInfo);
    OnStop();
}

void AudioDeviceDescriptorFixApiCompatibilityFuzzTest()
{
    AudioDeviceDescriptor deviceDescriptor;
    int apiVersion = GetData<int>();
    DeviceRole deviceRole = GetData<DeviceRole>();
    DeviceType deviceType = GetData<DeviceType>();
    int32_t deviceId = GetData<int32_t>();
    DeviceStreamInfo streamInfo;
    std::list<DeviceStreamInfo> streamInfos;
    streamInfos.push_back(streamInfo);
    deviceDescriptor.FixApiCompatibility(apiVersion, deviceRole, deviceType, deviceId, streamInfos);
    OnStop();
}

void AudioDeviceDescriptorGetKeyFuzzTest()
{
    AudioDeviceDescriptor deviceDescriptor;
    deviceDescriptor.GetKey();
    OnStop();
}

TestFuncs g_testFuncs[] = {
    GetActiveA2dpDeviceStreamInfoFuzzTest,
    GetMaxAmplitudeFuzzTest,
    UpdateDeviceFuzzTest,
    HandleActiveBtFuzzTest,
    HandleNegtiveBtFuzzTest,
    SetDeviceActiveFuzzTest,
    SetCallDeviceActiveFuzzTest,
    IsDirectSupportedDeviceFuzzTest,
    IsDeviceActiveFuzzTest,
    AudioActiveDeviceGetCurrentOutputDeviceCategoryFuzzTest,
    AudioActiveDeviceNotifyUserSelectionEventToBtFuzzTest,
    AudioActiveDeviceNotifyUserDisSelectionEventToBtFuzzTest,
    AudioActiveDeviceNotifyUserSelectionEventForInputFuzzTest,
    AudioActiveDeviceSetDeviceActiveFuzzTest,
    AudioActiveDeviceSetCallDeviceActiveFuzzTest,
    AudioActiveDeviceIsDeviceInVectorFuzzTest,
    AudioDeviceDescriptorSetClientInfoFuzzTest,
    AudioDeviceDescriptorFixApiCompatibilityFuzzTest,
    AudioDeviceDescriptorGetKeyFuzzTest,
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
