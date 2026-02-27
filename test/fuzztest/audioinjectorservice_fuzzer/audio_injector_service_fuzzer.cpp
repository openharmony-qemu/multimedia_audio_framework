/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "audio_info.h"
#include "audio_injector_service.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"
#include "sink/i_audio_render_sink.h"
#include "audio_device_info.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
const char *SINK_ADAPTER_NAME = "primary";
const uint64_t COMMON_UINT64_NUM = 2;
const uint32_t CHANNEL = 2;
const uint32_t SAMPLE_RATE = 48000;
static const uint8_t *RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;

/*
* describe: get data from outside untrusted data(RAW_DATA) which size is according to sizeof(T)
* tips: only support basic type
*/
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

void PeekAudioDataFuzzTest()
{
    auto audioInjectorService = std::make_shared<AudioInjectorService>();
    uint32_t sinkPortIndex = GetData<uint32_t>();
    uint8_t buffer = GetData<uint8_t>();
    size_t bufferSize = GetData<size_t>();
    AudioStreamInfo streamInfo;
    audioInjectorService->PeekAudioData(sinkPortIndex, &buffer, bufferSize, streamInfo);
}

void SetSinkPortIdxFuzzTest()
{
    auto audioInjectorService = std::make_shared<AudioInjectorService>();
    uint32_t sinkPortIdx = GetData<uint32_t>();
    audioInjectorService->SetSinkPortIdx(sinkPortIdx);
}

void GetSinkPortIdxFuzzTest()
{
    auto audioInjectorService = std::make_shared<AudioInjectorService>();
    audioInjectorService->GetSinkPortIdx();
}

void GetModuleInfoFuzzTest()
{
    auto audioInjectorService = std::make_shared<AudioInjectorService>();
    audioInjectorService->GetModuleInfo();
}

void SetModuleInfoFuzzTest()
{
    auto audioInjectorService = std::make_shared<AudioInjectorService>();
    AudioStreamInfo streamInfo;
    audioInjectorService->SetModuleInfo(streamInfo);
}

typedef void (*TestFuncs)();

TestFuncs g_testFuncs[] = {
    PeekAudioDataFuzzTest,
    SetSinkPortIdxFuzzTest,
    GetSinkPortIdxFuzzTest,
    GetModuleInfoFuzzTest,
    SetModuleInfoFuzzTest,
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
