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

#include "audio_log.h"
#include "audio_router_map.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
static int32_t NUM_2 = 2;
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
    if (g_dataSize < g_pos) {
        return object;
    }
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

void AudioRouteMapGetDeviceInfoByUidAndPidFuzzTest()
{
    int32_t uid = GetData<int32_t>();
    int32_t pid = GetData<int32_t>() % NUM_2;

    int32_t num = GetData<int32_t>() % NUM_2;
    AudioRouteMap::GetInstance().routerMap_.insert({uid, std::make_pair("test", num)});
    AudioRouteMap::GetInstance().GetDeviceInfoByUidAndPid(uid, pid);
}

void AudioRouteMapDelRouteMapInfoByKeyFuzzTest()
{
    int32_t uid = GetData<int32_t>();
    AudioRouteMap::GetInstance().DelRouteMapInfoByKey(uid);
}

void AudioRouteMapAddRouteMapInfoFuzzTest()
{
    int32_t uid = GetData<int32_t>();
    std::string device = "";
    int32_t pid = GetData<int32_t>();
    AudioRouteMap::GetInstance().AddRouteMapInfo(uid, device, pid);
}

void AudioRouteMapAddFastRouteMapInfoFuzzTest()
{
    int32_t uid = GetData<int32_t>();
    std::string device = "";
    DeviceRole role = GetData<DeviceRole>();
    AudioRouteMap::GetInstance().AddFastRouteMapInfo(uid, device, role);
}

void AudioRouteMapGetNetworkIDInFastRouterMapFuzzTest()
{
    int32_t uid = GetData<int32_t>();
    DeviceRole role = GetData<DeviceRole>();
    std::string newworkId = "testId";

    AudioRouteMap::GetInstance().fastRouterMap_.insert({uid, std::make_pair("test", role)});
    AudioRouteMap::GetInstance().GetNetworkIDInFastRouterMap(uid, role, newworkId);
}

TestPtr g_testPtrs[] = {
    AudioRouteMapGetDeviceInfoByUidAndPidFuzzTest,
    AudioRouteMapDelRouteMapInfoByKeyFuzzTest,
    AudioRouteMapAddRouteMapInfoFuzzTest,
    AudioRouteMapAddFastRouteMapInfoFuzzTest,
    AudioRouteMapGetNetworkIDInFastRouterMapFuzzTest,
};

void FuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr) {
        return;
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
    return;
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
