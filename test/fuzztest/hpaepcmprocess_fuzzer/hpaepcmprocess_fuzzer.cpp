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
#undef private
#include "audio_info.h"
#include "simd_utils.h"
#include "hpae_pcm_buffer.h"
#include "hpae_pcm_process.h"
#include "audio_log.h"
using namespace std;
using namespace OHOS::AudioStandard::HPAE;

namespace OHOS {
namespace AudioStandard {
using namespace std;
static const uint8_t *RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
typedef void (*TestPtr)(const uint8_t *, size_t);

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

void SizeFuzzTest()
{
    std::vector<float> pcmData = {GetData<float>(), GetData<float>(), GetData<float>(), GetData<float>(),
        GetData<float>()};
    float *begin = pcmData.data();
    size_t size = pcmData.size();
    auto hpaePcmProcess = std::make_shared<HpaePcmProcess>(begin, size);
    hpaePcmProcess->Size();
}

void BeginFuzzTest()
{
    std::vector<float> pcmData = {GetData<float>(), GetData<float>(), GetData<float>(), GetData<float>(),
        GetData<float>()};
    float *begin = pcmData.data();
    size_t size = pcmData.size();
    auto hpaePcmProcess = std::make_shared<HpaePcmProcess>(begin, size);
    hpaePcmProcess->Begin();
}

void EndFuzzTest()
{
    std::vector<float> pcmData = {GetData<float>(), GetData<float>(), GetData<float>(), GetData<float>(),
        GetData<float>()};
    float *begin = pcmData.data();
    size_t size = pcmData.size();
    auto hpaePcmProcess = std::make_shared<HpaePcmProcess>(begin, size);
    hpaePcmProcess->End();
}

void ResetFuzzTest()
{
    std::vector<float> pcmData = {GetData<float>(), GetData<float>(), GetData<float>(), GetData<float>(),
        GetData<float>()};
    float *begin = pcmData.data();
    size_t size = pcmData.size();
    auto hpaePcmProcess = std::make_shared<HpaePcmProcess>(begin, size);
    hpaePcmProcess->Reset();
}

void GetErrNoFuzzTest()
{
    std::vector<float> pcmData = {GetData<float>(), GetData<float>(), GetData<float>(), GetData<float>(),
        GetData<float>()};
    float *begin = pcmData.data();
    size_t size = pcmData.size();
    auto hpaePcmProcess = std::make_shared<HpaePcmProcess>(begin, size);
    hpaePcmProcess->GetErrNo();
}

typedef void (*TestFuncs[5])();

TestFuncs g_testFuncs = {
    SizeFuzzTest,
    BeginFuzzTest,
    EndFuzzTest,
    ResetFuzzTest,
    GetErrNoFuzzTest,
};

bool FuzzTest(const uint8_t *rawData, size_t size)
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
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::FuzzTest(data, size);
    return 0;
}