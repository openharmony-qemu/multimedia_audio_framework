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
#include "audio_server.h"
#include "audio_effect_volume.h"
#include "futex_tool.h"
#include "format_converter.h"
#include "audio_dump_pcm.h"
#include "audio_dump_pcm_private.h"

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

template<class T>
uint32_t GetArrLength(T& arr)
{
    if (arr == nullptr) {
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
        return 0;
    }
    return sizeof(arr) / sizeof(arr[0]);
}

void InitFuzzTest()
{
    AudioCacheMgrInner audioCacheMgrInner;
    audioCacheMgrInner.isInited_ = GetData<uint32_t>() % NUM_2;
    audioCacheMgrInner.Init();
}

void DeInitFuzzTest()
{
    AudioCacheMgrInner audioCacheMgrInner;
    audioCacheMgrInner.DeInit();
    audioCacheMgrInner.InitCallbackHandler();
    audioCacheMgrInner.DeInit();
}

void DumpAllMemBlockFuzzTest()
{
    AudioCacheMgrInner audioCacheMgrInner;
    std::shared_ptr<MemChunk> memChunk = std::make_shared<MemChunk>();
    audioCacheMgrInner.isInited_ = GetData<uint32_t>() % NUM_2;
    audioCacheMgrInner.memChunkDeque_.push_back(memChunk);
    audioCacheMgrInner.DumpAllMemBlock();
}

void CacheDataFuzzTest()
{
    std::string dumpFileName;
    void* srcDataPointer;
    size_t dataLength = THRESHOLD;
    uint8_t srcBuffer[THRESHOLD] = {0};
    auto audioCacheMgrInner = std::make_shared<AudioCacheMgrInner>();
    if (audioCacheMgrInner == nullptr) {
        return;
    }
    dumpFileName = "test.txt";
    srcDataPointer = static_cast<void *>(srcBuffer);
    audioCacheMgrInner->isInited_ = GetData<uint32_t>() % NUM_2;
    audioCacheMgrInner->CacheData(dumpFileName, srcDataPointer, dataLength);
    audioCacheMgrInner->isDumpingData_ = GetData<uint32_t>() % NUM_2;
    audioCacheMgrInner->CacheData(dumpFileName, srcDataPointer, dataLength);
    audioCacheMgrInner->totalMemChunkNums_ = (GetData<uint32_t>() % NUM_2) * NUM_2 - 1;
    audioCacheMgrInner->CacheData(dumpFileName, srcDataPointer, dataLength);
}

void GetCachedDurationFuzzTest()
{
    int64_t startTime = 0;
    int64_t endTime = 0;
    std::shared_ptr<MemChunk> memChunk = std::make_shared<MemChunk>();
    if (memChunk == nullptr) {
        return;
    }
    AudioCacheMgrInner audioCacheMgrInner;
    audioCacheMgrInner.isInited_ = GetData<uint32_t>() % NUM_2;
    audioCacheMgrInner.GetCachedDuration(startTime, endTime);
    audioCacheMgrInner.memChunkDeque_.push_back(memChunk);
    audioCacheMgrInner.memChunkDeque_.push_back(memChunk);
    audioCacheMgrInner.GetCachedDuration(startTime, endTime);
}

void GetDumpParameterFuzzTest()
{
    std::vector<std::string> subKeys;
    std::vector<std::pair<std::string, std::string>> result;
    AudioCacheMgrInner audioCacheMgrInner;
    std::vector<std::string> roleList = {"STATUS", "TIME", "MEMORY", "test"};
    uint32_t roleListCount = GetData<uint32_t>() % roleList.size();
    subKeys.push_back(roleList[roleListCount]);
    audioCacheMgrInner.GetDumpParameter(subKeys, result);
}

void SetDumpParameterFuzzTest()
{
    std::vector<std::pair<std::string, std::string>> params;
    AudioCacheMgrInner audioCacheMgrInner;
    std::vector<std::string> roleList = {"OPEN", "CLOSE", "UPLOAD", "test1"};
    uint32_t roleListCount = GetData<uint32_t>() % roleList.size();
    params.push_back(std::make_pair(roleList[roleListCount], "test"));
    audioCacheMgrInner.SetDumpParameter(params);
}

void OnHandleFuzzTest()
{
    int64_t data = 0;
    auto audioCacheMgrInner = std::make_shared<AudioCacheMgrInner>();
    if (audioCacheMgrInner == nullptr) {
        return;
    }
    uint32_t code = GetData<uint32_t>() % (AudioCacheMgrInner::RAISE_PRIORITY + 1);
    audioCacheMgrInner->OnHandle(code, data);
}

void GetMemBlockFuzzTest()
{
    size_t dataLength = GetData<size_t>();
    std::string dumpFileName = "abc";
    MemBlock curMemBlock;
    std::shared_ptr<MemChunk> memChunk = std::make_shared<MemChunk>();
    if (memChunk == nullptr) {
        return;
    }
    memChunk->GetMemBlock(dataLength, dumpFileName, curMemBlock);
}

void GetMemChunkDurationFuzzTest()
{
    int64_t startTime = GetData<int64_t>();
    int64_t endTime = GetData<int64_t>();
    std::shared_ptr<MemChunk> memChunk = std::make_shared<MemChunk>();
    if (memChunk == nullptr) {
        return;
    }
    memChunk->GetMemChunkDuration(startTime, endTime);
}

void GetCurUsedMemoryFuzzTest()
{
    size_t dataLength = GetData<size_t>();
    size_t bufferLength = GetData<size_t>();
    size_t structLength = GetData<size_t>();
    std::shared_ptr<MemChunk> memChunk = std::make_shared<MemChunk>();
    if (memChunk == nullptr) {
        return;
    }
    memChunk->GetCurUsedMemory(dataLength, bufferLength, structLength);
}

void ResetFuzzTest()
{
    std::shared_ptr<MemChunk> memChunk = std::make_shared<MemChunk>();
    if (memChunk == nullptr) {
        return;
    }
    memChunk->Reset();
}

void GetCurMemoryConditionFuzzTest()
{
    auto audioCacheMgrInner = std::make_shared<AudioCacheMgrInner>();
    if (audioCacheMgrInner == nullptr) {
        return;
    }
    size_t dataLength = GetData<size_t>();
    size_t bufferLength = GetData<size_t>();
    size_t structLength = GetData<size_t>();
    audioCacheMgrInner->GetCurMemoryCondition(dataLength, bufferLength, structLength);
}

void PrintCurMemoryConditionFuzzTest()
{
    AudioCacheMgrInner audioCacheMgrInner;
    audioCacheMgrInner.PrintCurMemoryCondition();
}

TestFuncs g_testFuncs[] = {
    InitFuzzTest,
    DeInitFuzzTest,
    DumpAllMemBlockFuzzTest,
    CacheDataFuzzTest,
    GetCachedDurationFuzzTest,
    GetDumpParameterFuzzTest,
    SetDumpParameterFuzzTest,
    OnHandleFuzzTest,
    GetMemBlockFuzzTest,
    GetMemChunkDurationFuzzTest,
    GetCurUsedMemoryFuzzTest,
    ResetFuzzTest,
    GetCurMemoryConditionFuzzTest,
    PrintCurMemoryConditionFuzzTest,
};

void FuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr) {
        return;
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
