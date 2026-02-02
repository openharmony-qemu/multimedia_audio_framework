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
#include <fuzzer/FuzzedDataProvider.h>
namespace OHOS {
namespace AudioStandard {
using namespace std;

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;

typedef void (*TestFuncs)();

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

void SetPreferredMediaRenderDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    AudioStateManager::GetAudioStateManager().SetPreferredMediaRenderDevice(desc);
}

void SetAndGetRecordCaptureDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    AudioStateManager::GetAudioStateManager().SetPreferredCallRenderDevice(desc, 0);
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    desc->deviceType_ = DeviceTypeVec[deviceTypeCount];
    AudioStateManager::GetAudioStateManager().SetPreferredCallRenderDevice(desc, 1);
    AudioStateManager::GetAudioStateManager().GetPreferredCallRenderDevice();
}

void SetPreferredCallCaptureDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    AudioStateManager::GetAudioStateManager().SetPreferredCallCaptureDevice(desc);
}

void SetPreferredRingRenderDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    AudioStateManager::GetAudioStateManager().SetPreferredRingRenderDevice(desc);
}

void SetPreferredRecordCaptureDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    AudioStateManager::GetAudioStateManager().SetPreferredRecordCaptureDevice(desc);
}

void SetPreferredToneRenderDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    AudioStateManager::GetAudioStateManager().SetPreferredToneRenderDevice(desc);
}

void UpdatePreferredMediaRenderDeviceConnectStateFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t stateCount = static_cast<int32_t>(ConnectState::DEACTIVE_CONNECTED) + 1;
    ConnectState state = static_cast<ConnectState>(GetData<uint8_t>() % stateCount);
    AudioStateManager::GetAudioStateManager().UpdatePreferredMediaRenderDeviceConnectState(state);
}

void UpdatePreferredCallRenderDeviceConnectStateFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t stateCount = static_cast<int32_t>(ConnectState::DEACTIVE_CONNECTED) + 1;
    ConnectState state = static_cast<ConnectState>(GetData<uint8_t>() % stateCount);
    AudioStateManager::GetAudioStateManager().UpdatePreferredCallRenderDeviceConnectState(state);
}

void UpdatePreferredCallCaptureDeviceConnectStateFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t stateCount = static_cast<int32_t>(ConnectState::DEACTIVE_CONNECTED) + 1;
    ConnectState state = static_cast<ConnectState>(GetData<uint8_t>() % stateCount);
    AudioStateManager::GetAudioStateManager().UpdatePreferredCallCaptureDeviceConnectState(state);
}

void UpdatePreferredRecordCaptureDeviceConnectStateFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t stateCount = static_cast<int32_t>(ConnectState::DEACTIVE_CONNECTED) + 1;
    ConnectState state = static_cast<ConnectState>(GetData<uint8_t>() % stateCount);
    AudioStateManager::GetAudioStateManager().UpdatePreferredRecordCaptureDeviceConnectState(state);
}

void SetAndGetPreferredRingRenderDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    if (desc == nullptr) {
        return;
    }
    AudioStateManager::GetAudioStateManager().SetPreferredRingRenderDevice(desc);
    AudioStateManager::GetAudioStateManager().GetPreferredRingRenderDevice();
}

void SetAndGetPreferredToneRenderDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    AudioStateManager::GetAudioStateManager().SetPreferredToneRenderDevice(desc);
    AudioStateManager::GetAudioStateManager().GetPreferredToneRenderDevice();
}

void SetAndGetPreferredRecognitionCaptureDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    AudioStateManager::GetAudioStateManager().SetPreferredRecognitionCaptureDevice(desc);
    AudioStateManager::GetAudioStateManager().GetPreferredRecognitionCaptureDevice();
}

void SetAudioClientInfoMgrCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    sptr<IStandardAudioPolicyManagerListener> desc = sptr<IStandardAudioPolicyManagerListener>();
    AudioStateManager::GetAudioStateManager().SetAudioClientInfoMgrCallback(desc);
}

void SetPreferredCallRenderDeviceAudioClinetInfoMgrCallbackHasValueFuzzTest(FuzzedDataProvider& fdp)
{
    sptr<IStandardAudioPolicyManagerListener> desc = sptr<IStandardAudioPolicyManagerListener>();
    AudioStateManager::GetAudioStateManager().SetAudioClientInfoMgrCallback(desc);
    shared_ptr<AudioDeviceDescriptor> desc_ = std::make_shared<AudioDeviceDescriptor>();
    int32_t uid = GetData<int32_t>();
    AudioStateManager::GetAudioStateManager().SetPreferredCallRenderDevice(desc_, uid);
}

void FindExcludedDevicesFuzzTest(FuzzedDataProvider& fdp)
{
    auto &man = AudioStateManager::GetAudioStateManager();
    man.FindExcludedDevices(MEDIA_OUTPUT_DEVICES);
    man.FindExcludedDevices(MEDIA_INPUT_DEVICES);
    man.FindExcludedDevices(CALL_OUTPUT_DEVICES);
    man.FindExcludedDevices(CALL_INPUT_DEVICES);
    man.FindExcludedDevices(D_ALL_DEVICES);
    man.FindExcludedDevices(GetData<AudioDeviceUsage>());
}

void ExcludeDevicesFuzzTest(FuzzedDataProvider& fdp)
{
    auto &man = AudioStateManager::GetAudioStateManager();
    auto dev = make_shared<AudioDeviceDescriptor>();
    dev->deviceId_ = GetData<int32_t>();
    dev->deviceType_ = GetData<DeviceType>();
    dev->deviceName_ = "--";
    dev->deviceRole_ = GetData<DeviceRole>();
    dev->macAddress_ = "card=2;device=0";
    man.ExcludeDevices(MEDIA_OUTPUT_DEVICES, {dev});
}

void UnexcludeDevicesFuzzTest(FuzzedDataProvider& fdp)
{
    auto &man = AudioStateManager::GetAudioStateManager();
    auto dev = make_shared<AudioDeviceDescriptor>();
    dev->deviceId_ = GetData<int32_t>();
    dev->deviceType_ = GetData<DeviceType>();
    dev->deviceName_ = "--";
    dev->deviceRole_ = GetData<DeviceRole>();
    dev->macAddress_ = "card=2;device=0";
    man.UnexcludeDevices(MEDIA_OUTPUT_DEVICES, {dev});
}

void SetAndGetPreferredCallRenderDeviceTypeNotEqTypeNoneFuzzTest(FuzzedDataProvider& fdp)
{
    shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    desc->deviceType_ = DeviceTypeVec[deviceTypeCount];
    int32_t uid = GetData<int32_t>();
    AudioStateManager::GetAudioStateManager().SetAudioSceneOwnerUid(uid);
    AudioStateManager::GetAudioStateManager().SetPreferredCallRenderDevice(desc, 1);
    AudioStateManager::GetAudioStateManager().GetPreferredCallRenderDevice();
}

void GetPreferredCallRenderDeviceForUidFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t clientUid = GetData<int32_t>();
    vector<int32_t> uidList = {
        SYSTEM_UID,
        clientUid,
        0,
    };
    uint32_t index = GetData<uint32_t>() % uidList.size();
    int32_t testId = uidList[index];
    std::string currentActivePort = "test";
    AudioStateManager::GetAudioStateManager().forcedDeviceMapList_.clear();
    shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    std::map<int32_t, std::shared_ptr<AudioDeviceDescriptor>> forcedDeviceMap;
    forcedDeviceMap.insert({testId, desc});
    AudioStateManager::GetAudioStateManager().forcedDeviceMapList_.push_back(forcedDeviceMap);
    AudioStateManager::GetAudioStateManager().GetPreferredCallRenderDeviceForUid(clientUid);
}

void SetAudioVKBInfoMgrCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    sptr<IStandardAudioPolicyManagerListener> callback = sptr<IStandardAudioPolicyManagerListener>();
    AudioStateManager::GetAudioStateManager().SetAudioVKBInfoMgrCallback(callback);
}

void CheckVKBInfoFuzzTest(FuzzedDataProvider& fdp)
{
    std::string bundleName = "testName";
    bool isValid = GetData<bool>();
    AudioStateManager::GetAudioStateManager().CheckVKBInfo(bundleName, isValid);
}

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
    SetPreferredMediaRenderDeviceFuzzTest,
    SetAndGetRecordCaptureDeviceFuzzTest,
    SetPreferredCallCaptureDeviceFuzzTest,
    SetPreferredRingRenderDeviceFuzzTest,
    SetPreferredRecordCaptureDeviceFuzzTest,
    SetPreferredToneRenderDeviceFuzzTest,
    UpdatePreferredMediaRenderDeviceConnectStateFuzzTest,
    UpdatePreferredCallRenderDeviceConnectStateFuzzTest,
    UpdatePreferredCallCaptureDeviceConnectStateFuzzTest,
    UpdatePreferredRecordCaptureDeviceConnectStateFuzzTest,
    FindExcludedDevicesFuzzTest,
    ExcludeDevicesFuzzTest,
    UnexcludeDevicesFuzzTest,
    SetAndGetPreferredRingRenderDeviceFuzzTest,
    SetAndGetPreferredToneRenderDeviceFuzzTest,
    SetAndGetPreferredRecognitionCaptureDeviceFuzzTest,
    SetAudioClientInfoMgrCallbackFuzzTest,
    SetPreferredCallRenderDeviceAudioClinetInfoMgrCallbackHasValueFuzzTest,
    SetAndGetPreferredCallRenderDeviceTypeNotEqTypeNoneFuzzTest,
    GetPreferredCallRenderDeviceForUidFuzzTest,
    SetAudioVKBInfoMgrCallbackFuzzTest,
    CheckVKBInfoFuzzTest,
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
} // namesapce OHOS

/* Fuzzer entry point */
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