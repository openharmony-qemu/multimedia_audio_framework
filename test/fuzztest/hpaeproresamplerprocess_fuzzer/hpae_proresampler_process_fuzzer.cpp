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
#include "audio_utils.h"
#include <algorithm>
#include <cinttypes>
#include "audio_proresampler_process.h"
#include "audio_engine_log.h"
#include "audio_proresampler.h"
#include "audio_stream_info.h"
namespace OHOS {
namespace AudioStandard {
using namespace std;
using namespace HPAE;
static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
const size_t NUM_TWO = 2;
const size_t NUM_SEVEN = 7;
const static std::vector<uint32_t>  TEST_CHANNELS = {MONO, STEREO, CHANNEL_6};

const static std::map<uint32_t, uint32_t> TEST_SAMPLE_RATE_COMBINATION = { // {input, output} combination
    {SAMPLE_RATE_24000, SAMPLE_RATE_48000},
    {SAMPLE_RATE_16000, SAMPLE_RATE_48000},
    {SAMPLE_RATE_44100, SAMPLE_RATE_192000},
    {SAMPLE_RATE_48000, SAMPLE_RATE_24000},
    {SAMPLE_RATE_48000, SAMPLE_RATE_16000},
    {SAMPLE_RATE_192000, SAMPLE_RATE_44100},
};

constexpr uint32_t INVALID_QUALITY = -1;
constexpr uint32_t QUALITY_ONE = 1;
constexpr uint32_t FRAME_LEN_20MS = 20;
constexpr uint32_t FRAME_LEN_40MS = 40;
constexpr uint32_t MS_PER_SECOND = 1000;
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

void SingleStagePolyphaseResamplerSetRate1()
{
    SingleStagePolyphaseResamplerState state;
    uint32_t decimateFactor = 0;
    uint32_t interpolateFactor = 0;
    SingleStagePolyphaseResamplerSetRate(&state, decimateFactor, interpolateFactor);
}

void SingleStagePolyphaseResamplerSetRate2()
{
    SingleStagePolyphaseResamplerState state;
    uint32_t decimateFactor = 0;
    uint32_t interpolateFactor = 2;
    SingleStagePolyphaseResamplerSetRate(&state, decimateFactor, interpolateFactor);
}

void SingleStagePolyphaseResamplerSetRate3()
{
    SingleStagePolyphaseResamplerState state;
    uint32_t decimateFactor = 2;
    uint32_t interpolateFactor = 0;
    SingleStagePolyphaseResamplerSetRate(&state, decimateFactor, interpolateFactor);
}

void SingleStagePolyphaseResamplerSetRate4()
{
    SingleStagePolyphaseResamplerState state;
    uint32_t decimateFactor = GetData<uint32_t>();
    uint32_t interpolateFactor = GetData<uint32_t>();
    SingleStagePolyphaseResamplerSetRate(&state, decimateFactor, interpolateFactor);
}

void InitTest()
{
    // test invalid input
    int32_t err = RESAMPLER_ERR_SUCCESS;
    SingleStagePolyphaseResamplerInit(STEREO, SAMPLE_RATE_24000, SAMPLE_RATE_48000, INVALID_QUALITY, &err);

    // test valid input
    SingleStagePolyphaseResamplerInit(STEREO, SAMPLE_RATE_24000, SAMPLE_RATE_48000, QUALITY_ONE, &err);

    // test 11025 input
    ProResampler resampler1(SAMPLE_RATE_11025, SAMPLE_RATE_48000, STEREO, QUALITY_ONE);

    // test other input
    ProResampler resampler2(SAMPLE_RATE_48000, SAMPLE_RATE_44100, STEREO, QUALITY_ONE);
}

void ProcessTest()
{
    for (uint32_t channels: TEST_CHANNELS) {
        for (auto pair: TEST_SAMPLE_RATE_COMBINATION) {
            uint32_t inRate = pair.first;
            uint32_t outRate = pair.second;
            uint32_t inFrameLen = inRate * FRAME_LEN_20MS / MS_PER_SECOND;
            uint32_t outFrameLen = outRate * FRAME_LEN_20MS / MS_PER_SECOND;
            ProResampler resampler(inRate, outRate, channels, QUALITY_ONE);
            std::vector<float> in(inFrameLen * channels);
            std::vector<float> out(outFrameLen * channels);
            resampler.Process(in.data(), inFrameLen, out.data(), outFrameLen);
        }
    }

    ProResampler resampler(SAMPLE_RATE_11025, SAMPLE_RATE_48000, STEREO, QUALITY_ONE);
    uint32_t inFrameLen = SAMPLE_RATE_11025 * FRAME_LEN_40MS / MS_PER_SECOND;
    uint32_t outFrameLen = SAMPLE_RATE_48000 * FRAME_LEN_20MS / MS_PER_SECOND;
    std::vector<float> in(inFrameLen * STEREO);
    std::vector<float> out(outFrameLen * STEREO);
    resampler.Process(in.data(), inFrameLen, out.data(), outFrameLen);

    inFrameLen = 0;
    resampler.Process(in.data(), inFrameLen, out.data(), outFrameLen);
    resampler.Process(in.data(), inFrameLen, out.data(), outFrameLen);
}

void UpdateRatesTest1()
{
    ProResampler resampler(SAMPLE_RATE_48000, SAMPLE_RATE_96000, STEREO, QUALITY_ONE);
    resampler.UpdateRates(SAMPLE_RATE_11025, SAMPLE_RATE_48000);
}

void UpdateRatesTest2()
{
    ProResampler resampler(SAMPLE_RATE_48000, SAMPLE_RATE_96000, STEREO, QUALITY_ONE);
    resampler.UpdateRates(NUM_TWO, SAMPLE_RATE_48000);
}

void UpdateChannel()
{
    ProResampler resampler(SAMPLE_RATE_48000, SAMPLE_RATE_96000, STEREO, QUALITY_ONE);

    resampler.UpdateChannels(CHANNEL_6);
}

void ErrCodeToString()
{
    ProResampler resampler(SAMPLE_RATE_48000, SAMPLE_RATE_96000, STEREO, QUALITY_ONE);
    resampler.ErrCodeToString(RESAMPLER_ERR_SUCCESS);
    resampler.ErrCodeToString(RESAMPLER_ERR_ALLOC_FAILED);
    resampler.ErrCodeToString(RESAMPLER_ERR_OVERFLOW);
    resampler.ErrCodeToString(NUM_SEVEN);
    resampler.ErrCodeToString(RESAMPLER_ERR_INVALID_ARG);
    resampler.ErrCodeToString(GetData<int32_t>());
}

typedef void (*TestFuncs)();
TestFuncs g_testFuncs[] = {
    SingleStagePolyphaseResamplerSetRate1,
    SingleStagePolyphaseResamplerSetRate2,
    SingleStagePolyphaseResamplerSetRate3,
    SingleStagePolyphaseResamplerSetRate4,
    InitTest,
    ProcessTest,
    UpdateRatesTest1,
    UpdateRatesTest2,
    UpdateChannel,
    ErrCodeToString,
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
