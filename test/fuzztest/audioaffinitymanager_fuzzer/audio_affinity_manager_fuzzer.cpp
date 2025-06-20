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

namespace OHOS {
namespace AudioStandard {
using namespace std;

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
const uint8_t TESTSIZE = 5;
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

vector<DeviceFlag> DeviceFlagVec = {
    NONE_DEVICES_FLAG,
    OUTPUT_DEVICES_FLAG,
    ALL_DEVICES_FLAG,
    DISTRIBUTED_OUTPUT_DEVICES_FLAG,
    DISTRIBUTED_INPUT_DEVICES_FLAG,
    ALL_DISTRIBUTED_DEVICES_FLAG,
    ALL_L_D_DEVICES_FLAG,
    DEVICE_FLAG_MAX,
};

vector<DeviceRole> DeviceRoleVec = {
    DEVICE_ROLE_NONE,
    INPUT_DEVICE,
    OUTPUT_DEVICE,
    DEVICE_ROLE_MAX,
};

void ParseAffinityXmlFuzzTest()
{
    std::unique_ptr <AudioAffinityManager> audioAffinityManager = std::make_unique<AudioAffinityManager>();
    audioAffinityManager->ParseAffinityXml();
    if (!audioAffinityManager->rendererAffinityDeviceArray_.empty()) {
        audioAffinityManager->rendererAffinityDeviceArray_[0];
    }
    if (!audioAffinityManager->capturerAffinityDeviceArray_.empty()) {
        audioAffinityManager->capturerAffinityDeviceArray_[0];
    }
}

void OnXmlParsingCompletedFuzzTest()
{
    std::unique_ptr <AudioAffinityManager> audioAffinityManager = std::make_unique<AudioAffinityManager>();
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    DeviceType deviceType = DeviceTypeVec[deviceTypeCount];
    uint32_t deviceFlagCount = GetData<uint32_t>() % DeviceFlagVec.size();
    DeviceFlag deviceFlag = DeviceFlagVec[deviceFlagCount];
    std::vector<AffinityDeviceInfo> xmlData = {
        {"group", deviceType, deviceFlag, "network", 0, GetData<uint8_t>() % NUM_2, GetData<uint8_t>() % NUM_2}
    };
    audioAffinityManager->OnXmlParsingCompleted(xmlData);
}

void GetRendererDeviceFuzzTest()
{
    std::unique_ptr <AudioAffinityManager> audioAffinityManager = std::make_unique<AudioAffinityManager>();
    int32_t testClientUID = GetData<int32_t>();
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    DeviceType testDeviceType = DeviceTypeVec[deviceTypeCount];
    uint32_t deviceRoleCount = GetData<uint32_t>() % DeviceRoleVec.size();
    DeviceRole testDeviceRole = DeviceRoleVec[deviceRoleCount];
    int32_t testInterruptGroupId = 1;
    int32_t testVolumeGroupId = 1;
    std::string testNetworkId = "test_network";
    std::shared_ptr<AudioDeviceDescriptor> testDescriptor = std::make_shared<AudioDeviceDescriptor>(
        testDeviceType, testDeviceRole, testInterruptGroupId, testVolumeGroupId, testNetworkId);
    audioAffinityManager->activeRendererDeviceMap_[testClientUID] = testDescriptor;
    std::shared_ptr<AudioDeviceDescriptor> result = audioAffinityManager->GetRendererDevice(testClientUID);
}

void GetCapturerDeviceFuzzTest()
{
    std::unique_ptr <AudioAffinityManager> audioAffinityManager = std::make_unique<AudioAffinityManager>();
    int32_t clientUID = GetData<int32_t>();
    std::shared_ptr<AudioDeviceDescriptor> descriptor = std::make_shared<AudioDeviceDescriptor>();
    descriptor->networkId_ = "test_network";
    uint32_t deviceRoleCount = GetData<uint32_t>() % DeviceRoleVec.size();
    descriptor->deviceRole_ = DeviceRoleVec[deviceRoleCount];
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    descriptor->deviceType_ = DeviceTypeVec[deviceTypeCount];
    audioAffinityManager->activeCapturerDeviceMap_[clientUID] = descriptor;
    std::shared_ptr<AudioDeviceDescriptor> result = audioAffinityManager->GetCapturerDevice(clientUID);
}

void DelSelectRendererDeviceFuzzTest()
{
    std::unique_ptr <AudioAffinityManager> audioAffinityManager = std::make_unique<AudioAffinityManager>();
    int32_t clientUID1 = GetData<int32_t>();
    int32_t clientUID2 = GetData<int32_t>();
    std::shared_ptr<AudioDeviceDescriptor> descriptor = std::make_shared<AudioDeviceDescriptor>();
    std::string networkId = "test_network";
    std::string groupName = "test_group";
    audioAffinityManager->DelSelectRendererDevice(clientUID1);
}

TestFuncs g_testFuncs[TESTSIZE] = {
    ParseAffinityXmlFuzzTest,
    OnXmlParsingCompletedFuzzTest,
    GetRendererDeviceFuzzTest,
    GetCapturerDeviceFuzzTest,
    DelSelectRendererDeviceFuzzTest,
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
