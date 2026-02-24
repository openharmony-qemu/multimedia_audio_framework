/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#include "audio_iohandle_map.h"
#include "audio_module_info.h"
#include "audio_pipe_info.h"
#include "audio_device_info.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
static int32_t NUM_2 = 2;
static int32_t NUM_100 = 100;
static int32_t NUM_200 = 200;
static int32_t NUM_1000 = 1000;
const size_t THRESHOLD = 10;
constexpr size_t MAX_RANDOM_STRING_LENGTH = 128;
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

template<>
std::string GetData<std::string>()
{
    const size_t len = GetData<size_t>() / MAX_RANDOM_STRING_LENGTH;
    std::string ret(len, ' ');
    for (auto &c : ret) {
        c = GetData<char>();
    }
    return ret;
}

void DelIOHandleInfoFuzzTest()
{
    std::string moduleName = GetData<std::string>();
    AudioIOHandleMap::GetInstance().DelIOHandleInfo(moduleName);
}

void GetSinkIOHandleFuzzTest()
{
    DeviceType deviceType = GetData<DeviceType>();
    AudioIOHandle handle = AudioIOHandleMap::GetInstance().GetSinkIOHandle(deviceType);
}

void GetSourceIOHandleFuzzTest()
{
    DeviceType deviceType = GetData<DeviceType>();
    AudioIOHandle handle = AudioIOHandleMap::GetInstance().GetSourceIOHandle(deviceType);
}

void MuteDefaultSinkPortFuzzTest()
{
    std::string networkID = "networkID";
    std::string sinkName = "sinkName";
    AudioIOHandleMap::GetInstance().MuteDefaultSinkPort(networkID, sinkName);
}

void MuteDefaultSinkPortBranchTest()
{
    AudioIOHandleMap::GetInstance().MuteDefaultSinkPort("RemoteDevice", "Speaker");
}

void MuteDefaultSinkPortBranch2Test()
{
    AudioIOHandleMap::GetInstance().MuteDefaultSinkPort(LOCAL_NETWORK_ID, "USB_Speaker");
}

void MuteDefaultSinkPortBothBranchTest()
{
    AudioIOHandleMap::GetInstance().MuteDefaultSinkPort("RemoteDevice", "USB_Speaker");
}

void MuteDefaultSinkPortNoBranchTest()
{
    AudioIOHandleMap::GetInstance().MuteDefaultSinkPort(LOCAL_NETWORK_ID, PRIMARY_SPEAKER);
}

void UnmutePortAfterMuteDurationFuzzTest()
{
    int32_t muteDuration = GetData<int32_t>();
    std::string portName = GetData<std::string>();
    AudioIOHandleMap::GetInstance().NotifyUnmutePort();
    AudioIOHandleMap::GetInstance().UnmutePortAfterMuteDuration(muteDuration, portName);
}

void DoUnmutePortFuzzTest()
{
    int32_t muteDuration = GetData<int32_t>();
    std::string portName = GetData<std::string>();
    AudioIOHandleMap::GetInstance().DoUnmutePort(muteDuration, portName);
}

void DoUnmutePortBranchTest()
{
    AudioIOHandleMap::GetInstance().DoUnmutePort(NUM_1000, PRIMARY_SPEAKER);
}

void DoUnmutePortBranch2Test()
{
    AudioIOHandleMap::GetInstance().DoUnmutePort(NUM_1000, "Unknown_Port");
}

void ReloadPortAndUpdateIOHandleFuzzTest()
{
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->name_ = "test_pipe";
    AudioModuleInfo moduleInfo;
    moduleInfo.name = "test_module";
    bool softLinkFlag = GetData<bool>();

    AudioIOHandleMap::GetInstance().AddIOHandleInfo("test_module", NUM_100);
    AudioIOHandleMap::GetInstance().ReloadPortAndUpdateIOHandle(pipeInfo, moduleInfo, softLinkFlag);
}

void ReloadPortAndUpdateIOHandleBranchTest()
{
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->name_ = "test_pipe";
    AudioModuleInfo moduleInfo;
    moduleInfo.name = "nonexistent_module";
    AudioIOHandleMap::GetInstance().ReloadPortAndUpdateIOHandle(pipeInfo, moduleInfo, false);
}

void ReloadPortAndUpdateIOHandleBranch2Test()
{
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->name_ = "test_pipe";
    AudioModuleInfo moduleInfo;
    moduleInfo.name = "existing_module";
    AudioIOHandleMap::GetInstance().AddIOHandleInfo("existing_module", NUM_200);
    AudioIOHandleMap::GetInstance().ReloadPortAndUpdateIOHandle(pipeInfo, moduleInfo, true);
}

TestPtr g_testPtrs[] = {
    DelIOHandleInfoFuzzTest,
    GetSinkIOHandleFuzzTest,
    GetSourceIOHandleFuzzTest,
    MuteDefaultSinkPortFuzzTest,
    MuteDefaultSinkPortBranchTest,
    MuteDefaultSinkPortBranch2Test,
    MuteDefaultSinkPortBothBranchTest,
    MuteDefaultSinkPortNoBranchTest,
    UnmutePortAfterMuteDurationFuzzTest,
    DoUnmutePortFuzzTest,
    DoUnmutePortBranchTest,
    DoUnmutePortBranch2Test,
    ReloadPortAndUpdateIOHandleFuzzTest,
    ReloadPortAndUpdateIOHandleBranchTest,
    ReloadPortAndUpdateIOHandleBranch2Test,
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
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }
    OHOS::AudioStandard::FuzzTest(data, size);
    return 0;
}
