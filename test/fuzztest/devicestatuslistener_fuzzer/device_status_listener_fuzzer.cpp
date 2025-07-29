/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include <cerrno>
#include "device_status_listener.h"
#include "audio_policy_service.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;

typedef void (*TestPtr)();

template<class T>
uint32_t GetArrLength(T& arr)
{
    if (arr == nullptr) {
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
        return 0;
    }
    return sizeof(arr) / sizeof(arr[0]);
}

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

void DeviceStatusListenerOnMicrophoneBlockedFuzzTest()
{
    static const vector<string> testInfo = {
        "EVENT_TYPE=1;DEVICE_TYPE=4;",
        "EVENT_TYPE=1;DEVICE_TYPE=2;",
        "EVENT_TYPE=1;DEVICE_TYPE=8;",
        "EVENT_TYPE=1;DEVICE_TYPE=2048;",
        "EVENT_TYPE=1;DEVICE_TYPE=4096;",
        "EVENT_TYPE=1;DEVICE_TYPE=8192;",
        "EVENT_TYPE=1;DEVICE_TYPE=1;",
        "abc",
    };
    auto deviceStatusListenerPtr = std::make_shared<DeviceStatusListener>(AudioPolicyService::GetAudioPolicyService());
    if (deviceStatusListenerPtr == nullptr) {
        return;
    }
    const std::string info = testInfo[GetData<uint32_t>() % testInfo.size()];
    deviceStatusListenerPtr->OnMicrophoneBlocked(info);
}

void DeviceStatusListenerSetAudioDeviceAnahsCallbackFuzzTest()
{
    auto deviceStatusListenerPtr = std::make_shared<DeviceStatusListener>(AudioPolicyService::GetAudioPolicyService());
    if (deviceStatusListenerPtr == nullptr) {
        return;
    }
}

void DeviceStatusListenerOnPnpDeviceStatusChangedFuzzTest()
{
    static const vector<string> testInfo = {
        "abc",
        "ANAHS_NAME=test;EVENT_TYPE=1;DEVICE_TYPE=1;DEVICE_ADDRESS=1;",
    };
    auto deviceStatusListenerPtr = std::make_shared<DeviceStatusListener>(AudioPolicyService::GetAudioPolicyService());
    if (deviceStatusListenerPtr == nullptr) {
        return;
    }

    const std::string info = testInfo[GetData<uint32_t>() % testInfo.size()];
    deviceStatusListenerPtr->OnPnpDeviceStatusChanged(info);
}

void DeviceStatusListenerUnRegisterDeviceStatusListenerFuzzTest()
{
    auto deviceStatusListenerPtr = std::make_shared<DeviceStatusListener>(AudioPolicyService::GetAudioPolicyService());
    if (deviceStatusListenerPtr == nullptr) {
        return;
    }
    HDIServiceManager hdiServiceManager;
    ServiceStatusListener listener;
    deviceStatusListenerPtr->hdiServiceManager_ = HDIServiceManagerGet();
    deviceStatusListenerPtr->listener_ = HdiServiceStatusListenerNewInstance();
    deviceStatusListenerPtr->audioPnpServer_ = &AudioPnpServer::GetAudioPnpServer();

    deviceStatusListenerPtr->UnRegisterDeviceStatusListener();
}

TestPtr g_testPtrs[] = {
    DeviceStatusListenerOnMicrophoneBlockedFuzzTest,
    DeviceStatusListenerSetAudioDeviceAnahsCallbackFuzzTest,
    DeviceStatusListenerOnPnpDeviceStatusChangedFuzzTest,
    DeviceStatusListenerUnRegisterDeviceStatusListenerFuzzTest,
};

bool FuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr) {
        return false;
    }

    RAW_DATA = rawData;
    g_dataSize = size;
    g_pos = 0;

    uint32_t code = GetData<uint32_t>();
    uint32_t len = GetArrLength(g_testPtrs);
    if (len > 0) {
        g_testPtrs[code % len]();
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