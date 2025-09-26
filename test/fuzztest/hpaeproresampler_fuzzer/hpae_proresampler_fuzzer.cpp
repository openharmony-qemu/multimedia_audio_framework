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
const static std::vector<uint32_t>  TEST_CHANNELS = {MONO, STEREO, CHANNEL_6};

constexpr uint32_t QUALITY_ONE = 1;
constexpr uint32_t FRAME_LEN_100MS = 100;
constexpr uint32_t FRAME_LEN_40MS = 40;
constexpr uint32_t FRAME_LEN_20MS = 20;
constexpr uint32_t MS_PER_SECOND = 1000;
constexpr uint32_t PROCESS_LOOP_COUNT = 10;
constexpr uint32_t CUSTOME_RATE_MULTIPLE = 50;
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

// test initialize resampler, update resampler
void ResampleBaseTest()
{
    uint32_t inRate = GetData<uint32_t>();
    uint32_t outRate = GetData<uint32_t>();
    uint32_t quality = GetData<uint32_t>();
    uint32_t channels = GetData<uint32_t>();

    ProResampler resampler(inRate, outRate, channels, quality);

    inRate = GetData<uint32_t>();
    outRate = GetData<uint32_t>();
    resampler.UpdateRates(inRate, outRate);
    
    channels = GetData<uint32_t>();
    resampler.UpdateChannels(channels);
}

void ResampleProcessTest()
{
    uint32_t inRate = GetData<uint32_t>();
    uint32_t outRate = GetData<uint32_t>();

    CHECK_AND_RETURN_LOG(inRate != 0, "divisor cannot be 0");

    uint32_t inFrameLen = 0;
    if (inRate % CUSTOME_RATE_MULTIPLE == 0) {
        inFrameLen = FRAME_LEN_20MS * inRate / MS_PER_SECOND;
    } else if (inRate == SAMPLE_RATE_11025) {
        inFrameLen = FRAME_LEN_40MS * inRate / MS_PER_SECOND;
    } else {
        inFrameLen = FRAME_LEN_100MS * inRate / MS_PER_SECOND;
    }
    uint32_t outFrameLen = inFrameLen / inRate * outRate;
    
    for (uint32_t channels: TEST_CHANNELS) {
        ProResampler resampler(inRate, outRate, channels, QUALITY_ONE);
        vector<float> in(inFrameLen, 0.0f);
        vector<float> out(outFrameLen, 0.0f);
        for (uint32_t i = 0; i < inFrameLen; i++) {
            in[i] = GetData<float>();
        }
        for (uint32_t i = 0; i < PROCESS_LOOP_COUNT; i++) {
            resampler.Process(in.data(), inFrameLen, out.data(), outFrameLen);
        }
    }
}

// special testcase for ensuring 11025 coverage
void ResampleProcessTest11025()
{
    uint32_t inRate = SAMPLE_RATE_11025;
    uint32_t outRate = GetData<uint32_t>();

    uint32_t inFrameLen = FRAME_LEN_40MS * inRate / MS_PER_SECOND;
    uint32_t outFrameLen = inFrameLen / inRate * outRate;
    
    for (uint32_t channels: TEST_CHANNELS) {
        ProResampler resampler(inRate, outRate, channels, QUALITY_ONE);
        vector<float> in(inFrameLen, 0.0f);
        vector<float> out(outFrameLen, 0.0f);
        for (uint32_t i = 0; i < inFrameLen; i++) {
            in[i] = GetData<float>();
        }
        for (uint32_t i = 0; i < PROCESS_LOOP_COUNT; i++) {
            resampler.Process(in.data(), inFrameLen, out.data(), outFrameLen);
        }
    }
}

typedef void (*TestFuncs)();
TestFuncs g_testFuncs[] = {
    ResampleBaseTest,
    ResampleProcessTest,
    ResampleProcessTest11025,
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
