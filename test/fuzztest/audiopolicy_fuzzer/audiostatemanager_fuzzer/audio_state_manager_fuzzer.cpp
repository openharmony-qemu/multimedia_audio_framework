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

const size_t THRESHOLD = 10;
const size_t MAX_STRING_LEN = 64;
const std::string DEFAULT_MAC_ADDRESS = "00:11:22:33:44:55";
typedef void (*TestPtr)(FuzzedDataProvider&);

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

const vector<DeviceType> g_testOutputDeviceTypes = {
    DEVICE_TYPE_EARPIECE,
    DEVICE_TYPE_SPEAKER,
    DEVICE_TYPE_WIRED_HEADSET,
    DEVICE_TYPE_WIRED_HEADPHONES,
    DEVICE_TYPE_BLUETOOTH_SCO,
    DEVICE_TYPE_BLUETOOTH_A2DP,
    DEVICE_TYPE_USB_HEADSET,
    DEVICE_TYPE_DP,
    DEVICE_TYPE_NEARLINK,
};

const vector<DeviceType> g_testInputDeviceTypes = {
    DEVICE_TYPE_MIC,
    DEVICE_TYPE_WIRED_HEADSET,
    DEVICE_TYPE_BLUETOOTH_SCO,
    DEVICE_TYPE_USB_HEADSET,
    DEVICE_TYPE_BLUETOOTH_A2DP_IN,
    DEVICE_TYPE_NEARLINK_IN,
};

const vector<DeviceRole> g_testDeviceRoles = {
    INPUT_DEVICE,
    OUTPUT_DEVICE,
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

std::shared_ptr<AudioDeviceDescriptor> MakeDeviceDescriptor(FuzzedDataProvider& fdp, DeviceRole role)
{
    const auto& deviceTypes = (role == INPUT_DEVICE) ? g_testInputDeviceTypes : g_testOutputDeviceTypes;
    if (deviceTypes.size() == 0 || g_testDeviceCategories.size() == 0) {
        return std::make_shared<AudioDeviceDescriptor>();
    }
    auto desc = std::make_shared<AudioDeviceDescriptor>(PickValue(fdp, deviceTypes), role);
    desc->networkId_ = ConsumeString(fdp, LOCAL_NETWORK_ID);
    desc->macAddress_ = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    desc->deviceName_ = ConsumeString(fdp, "audio_fuzz_device");
    desc->deviceCategory_ = PickValue(fdp, g_testDeviceCategories);
    desc->deviceId_ = fdp.ConsumeIntegral<int32_t>();
    return desc;
}

void SetPreferredRingRenderDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    shared_ptr<AudioDeviceDescriptor> desc = MakeDeviceDescriptor(fdp, OUTPUT_DEVICE);
    AudioStateManager::GetAudioStateManager().SetPreferredRingRenderDevice(desc);
}

void SetPreferredToneRenderDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    shared_ptr<AudioDeviceDescriptor> desc = MakeDeviceDescriptor(fdp, OUTPUT_DEVICE);
    AudioStateManager::GetAudioStateManager().SetPreferredToneRenderDevice(desc);
}

void SetAndGetPreferredRingRenderDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    shared_ptr<AudioDeviceDescriptor> desc = MakeDeviceDescriptor(fdp, OUTPUT_DEVICE);
    if (desc == nullptr) {
        return;
    }
    AudioStateManager::GetAudioStateManager().SetPreferredRingRenderDevice(desc);
    AudioStateManager::GetAudioStateManager().GetPreferredRingRenderDevice();
}

void SetAndGetPreferredToneRenderDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    shared_ptr<AudioDeviceDescriptor> desc = MakeDeviceDescriptor(fdp, OUTPUT_DEVICE);
    AudioStateManager::GetAudioStateManager().SetPreferredToneRenderDevice(desc);
    AudioStateManager::GetAudioStateManager().GetPreferredToneRenderDevice();
}

void SetAudioVKBInfoMgrCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    sptr<IStandardAudioPolicyManagerListener> callback = sptr<IStandardAudioPolicyManagerListener>();
    AudioStateManager::GetAudioStateManager().SetAudioVKBInfoMgrCallback(callback);
    std::string bundleName = ConsumeString(fdp, "testName");
    bool isValid = fdp.ConsumeBool();
    AudioStateManager::GetAudioStateManager().CheckVKBInfo(bundleName, isValid);
}

void CheckVKBInfoFuzzTest(FuzzedDataProvider& fdp)
{
    std::string bundleName = ConsumeString(fdp, "testName");
    bool isValid = fdp.ConsumeBool();
    AudioStateManager::GetAudioStateManager().CheckVKBInfo(bundleName, isValid);
}

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
    SetPreferredRingRenderDeviceFuzzTest,
    SetPreferredToneRenderDeviceFuzzTest,
    SetAndGetPreferredRingRenderDeviceFuzzTest,
    SetAndGetPreferredToneRenderDeviceFuzzTest,
    SetAudioVKBInfoMgrCallbackFuzzTest,
    CheckVKBInfoFuzzTest,
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

