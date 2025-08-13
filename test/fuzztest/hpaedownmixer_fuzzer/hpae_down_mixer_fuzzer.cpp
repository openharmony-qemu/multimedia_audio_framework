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
#include "audio_device_info.h"
#include "audio_utils.h"
#include "audio_channel_blend.h"
#include "volume_ramp.h"
#include <algorithm>
#include <cinttypes>
#include "down_mixer.h"
#include "audio_engine_log.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
using namespace HPAE;

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
static std::string g_rootCapturerPath = "/data/source_file_io_48000_2_s16le.pcm";
const char* DEFAULT_TEST_DEVICE_CLASS = "file_io";
const char* DEFAULT_TEST_DEVICE_NETWORKID = "LocalDevice";
constexpr size_t THRESHOLD = 10;
constexpr uint8_t TESTSIZE = 4;

// need full audio channel layouts to cover all cases during setting up downmix table -- first part
constexpr static AudioChannelLayout FIRST_PART_CH_LAYOUTS = static_cast<AudioChannelLayout> (
    FRONT_LEFT | FRONT_RIGHT | FRONT_CENTER | LOW_FREQUENCY |
    BACK_LEFT | BACK_RIGHT |
    FRONT_LEFT_OF_CENTER | FRONT_RIGHT_OF_CENTER |
    BACK_CENTER | SIDE_LEFT | SIDE_RIGHT |
    TOP_CENTER | TOP_FRONT_LEFT | TOP_FRONT_CENTER | TOP_FRONT_RIGHT | TOP_BACK_LEFT
);

// need full audio channel layouts to cover all cases during setting up downmix table -- second part
constexpr static AudioChannelLayout SECOND_PART_CH_LAYOUTS = static_cast<AudioChannelLayout> (
    TOP_CENTER | TOP_BACK_LEFT | TOP_BACK_CENTER | TOP_BACK_RIGHT |
    STEREO_LEFT | STEREO_RIGHT |
    WIDE_LEFT | WIDE_RIGHT |
    SURROUND_DIRECT_LEFT | SURROUND_DIRECT_RIGHT | LOW_FREQUENCY_2 |
    TOP_SIDE_LEFT | TOP_SIDE_RIGHT |
    BOTTOM_FRONT_CENTER | BOTTOM_FRONT_LEFT | BOTTOM_FRONT_RIGHT
);
// for test predefined downmix rules
const static std::set<AudioChannelLayout> OUTPUT_CH_LAYOUT_SET = {
    CH_LAYOUT_STEREO,
    CH_LAYOUT_5POINT1,
    CH_LAYOUT_5POINT1POINT2,
    CH_LAYOUT_5POINT1POINT4,
    CH_LAYOUT_7POINT1,
    CH_LAYOUT_7POINT1POINT2,
    CH_LAYOUT_7POINT1POINT4
};

const static std::set<AudioChannelLayout> GENERAL_OUTPUT_CH_LAYOUT_SET = {
    CH_LAYOUT_SURROUND,
    CH_LAYOUT_3POINT1,
    CH_LAYOUT_4POINT0,
    CH_LAYOUT_QUAD_SIDE,
    CH_LAYOUT_QUAD,
    CH_LAYOUT_4POINT1,
    CH_LAYOUT_5POINT0,
    CH_LAYOUT_5POINT0_BACK,
    CH_LAYOUT_2POINT1POINT2,
    CH_LAYOUT_3POINT0POINT2,
    CH_LAYOUT_5POINT1_BACK,
    CH_LAYOUT_6POINT0,
    CH_LAYOUT_HEXAGONAL,
    CH_LAYOUT_3POINT1POINT2,
    CH_LAYOUT_6POINT0_FRONT,
    CH_LAYOUT_6POINT1,
    CH_LAYOUT_6POINT1_BACK,
    CH_LAYOUT_7POINT0,
    CH_LAYOUT_OCTAGONAL,
    CH_LAYOUT_7POINT1_WIDE_BACK,
    CH_LAYOUT_7POINT1_WIDE,
    CH_LAYOUT_10POINT2,
    CH_LAYOUT_9POINT1POINT4,
};

// define channelLayout set to cover all channels as input
const static std::set<AudioChannelLayout> FULL_CH_LAYOUT_SET = {
    FIRST_PART_CH_LAYOUTS,
    SECOND_PART_CH_LAYOUTS
};

const static std::map<AudioChannel, AudioChannelLayout> DOWNMIX_CHANNEL_COUNT_MAP = {
    {MONO, CH_LAYOUT_MONO},
    {STEREO, CH_LAYOUT_STEREO},
    {CHANNEL_3, CH_LAYOUT_SURROUND},
    {CHANNEL_4, CH_LAYOUT_3POINT1},
    {CHANNEL_5, CH_LAYOUT_4POINT1},
    {CHANNEL_6, CH_LAYOUT_5POINT1},
    {CHANNEL_7, CH_LAYOUT_6POINT1},
    {CHANNEL_8, CH_LAYOUT_5POINT1POINT2},
    {CHANNEL_9, CH_LAYOUT_HOA_ORDER2_ACN_N3D},
    {CHANNEL_10, CH_LAYOUT_7POINT1POINT2},
    {CHANNEL_12, CH_LAYOUT_7POINT1POINT4},
    {CHANNEL_13, CH_LAYOUT_UNKNOWN},
    {CHANNEL_14, CH_LAYOUT_9POINT1POINT4},
    {CHANNEL_16, CH_LAYOUT_9POINT1POINT6}
};

constexpr uint32_t TEST_FORMAT_SIZE = 4;
constexpr uint32_t TEST_FRAME_LEN = 100;
constexpr uint32_t TEST_BUFFER_LEN = 10;
constexpr bool MIX_FLE = true;

static uint32_t BitCounts(uint64_t bits)
{
    uint32_t num = 0;
    for (; bits != 0; bits &= bits - 1) {
        num++;
    }
    return num;
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

template<class T>
uint32_t GetArrLength(T& arr)
{
    if (arr == nullptr) {
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
        return 0;
    }
    return sizeof(arr) / sizeof(arr[0]);
}

void SetParamFuzzTest()
{
       // invalid input Param
    DownMixer downMixer;
    AudioChannelInfo inChannelInfo;
    AudioChannelInfo outChannelInfo;
    inChannelInfo.numChannels = MAX_CHANNELS + 1;
    inChannelInfo.channelLayout = CH_LAYOUT_UNKNOWN;
    outChannelInfo.numChannels = MAX_CHANNELS + 1;
    outChannelInfo.channelLayout = CH_LAYOUT_UNKNOWN;
    downMixer.SetParam(inChannelInfo, outChannelInfo, TEST_FORMAT_SIZE, MIX_FLE);
    
    // valid param, predefined downmix rules
    for (AudioChannelLayout outLayout: OUTPUT_CH_LAYOUT_SET) {
        outChannelInfo.numChannels = BitCounts(outLayout);
        outChannelInfo.channelLayout = outLayout;
        for (AudioChannelLayout inLayout: FULL_CH_LAYOUT_SET) {
            inChannelInfo.channelLayout = inLayout;
            inChannelInfo.numChannels = MAX_CHANNELS;
            downMixer.SetParam(inChannelInfo, outChannelInfo, TEST_FORMAT_SIZE, MIX_FLE);
        }
    }
    
    // valid param, general downmix table rule
    for (AudioChannelLayout outLayout: GENERAL_OUTPUT_CH_LAYOUT_SET) {
        outChannelInfo.numChannels = BitCounts(outLayout);
        outChannelInfo.channelLayout = outLayout;
        for (AudioChannelLayout inLayout: FULL_CH_LAYOUT_SET) {
            inChannelInfo.channelLayout = inLayout;
            inChannelInfo.numChannels = MAX_CHANNELS;
            downMixer.SetParam(inChannelInfo, outChannelInfo, TEST_FORMAT_SIZE, MIX_FLE);
        }
    }
}

void SetDefaultChannelLayoutFuzzTest()
{
    DownMixer downMixer;
    for (auto pair : DOWNMIX_CHANNEL_COUNT_MAP) {
        downMixer.SetDefaultChannelLayout(pair.first);
    }
    downMixer.CheckIsHOA(CH_LAYOUT_HOA_ORDER2_ACN_SN3D);
    downMixer.CheckIsHOA(CH_LAYOUT_UNKNOWN);
}

void ProcesFuzzTest1()
{
    AudioChannelInfo inChannelInfo;
    AudioChannelInfo outChannelInfo;
    inChannelInfo.channelLayout = CH_LAYOUT_5POINT1;
    inChannelInfo.numChannels = CHANNEL_6;
    outChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    outChannelInfo.numChannels = STEREO;

    // test uninitialized
    DownMixer downMixer;
    std::vector<float> in(TEST_BUFFER_LEN * CHANNEL_6, 0.0f);
    std::vector<float> out(TEST_BUFFER_LEN * STEREO, 0.0f);
    uint32_t testInBufferSize = in.size() * TEST_FORMAT_SIZE;
    uint32_t testOutBufferSize = out.size() * TEST_FORMAT_SIZE;
    downMixer.Process(TEST_BUFFER_LEN, in.data(), testInBufferSize, out.data(), testOutBufferSize);
    
    // test input and output buffer length smaller than expected
    downMixer.SetParam(inChannelInfo, outChannelInfo, TEST_FORMAT_SIZE, MIX_FLE);
    downMixer.Process(TEST_FRAME_LEN, in.data(), testInBufferSize, out.data(), testOutBufferSize);

    // test process usual channel layout
    downMixer.Process(TEST_BUFFER_LEN, in.data(), testInBufferSize, out.data(), testOutBufferSize);

    // test process HOA
    inChannelInfo.channelLayout = CH_LAYOUT_HOA_ORDER2_ACN_SN3D;
    inChannelInfo.numChannels = CHANNEL_9;
    in.resize(CHANNEL_9 * TEST_BUFFER_LEN, 0.0f);
    testInBufferSize = in.size() * TEST_FORMAT_SIZE;
    downMixer.SetParam(inChannelInfo, outChannelInfo, TEST_FORMAT_SIZE, MIX_FLE);
    downMixer.Process(TEST_BUFFER_LEN, in.data(), testInBufferSize, out.data(), testOutBufferSize);
}

void ProcesFuzzTest2()
{
    AudioChannelInfo inChannelInfo;
    AudioChannelInfo outChannelInfo;
    inChannelInfo.channelLayout = CH_LAYOUT_5POINT1;
    inChannelInfo.numChannels = CHANNEL_6;
    outChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    outChannelInfo.numChannels = STEREO;

    // test uninitialized
    DownMixer downMixer;
    std::vector<float> in(TEST_BUFFER_LEN * CHANNEL_6, 0.0f);
    std::vector<float> out(TEST_BUFFER_LEN * STEREO, 0.0f);
    for (size_t i = 0; i < TEST_BUFFER_LEN * CHANNEL_6; i++) {
        in[i] = GetData<float>();
    }
    uint32_t testInBufferSize = in.size() * TEST_FORMAT_SIZE;
    uint32_t testOutBufferSize = out.size() * TEST_FORMAT_SIZE;
    downMixer.Process(TEST_BUFFER_LEN, in.data(), testInBufferSize, out.data(), testOutBufferSize);
    
    // test input and output buffer length smaller than expected
    downMixer.SetParam(inChannelInfo, outChannelInfo, TEST_FORMAT_SIZE, MIX_FLE);
    downMixer.Process(TEST_FRAME_LEN, in.data(), testInBufferSize, out.data(), testOutBufferSize);

    // test process usual channel layout
    downMixer.Process(TEST_BUFFER_LEN, in.data(), testInBufferSize, out.data(), testOutBufferSize);

    // test process HOA
    inChannelInfo.channelLayout = CH_LAYOUT_HOA_ORDER2_ACN_SN3D;
    inChannelInfo.numChannels = CHANNEL_9;
    in.resize(CHANNEL_9 * TEST_BUFFER_LEN, 0.0f);
    testInBufferSize = in.size() * TEST_FORMAT_SIZE;
    downMixer.SetParam(inChannelInfo, outChannelInfo, TEST_FORMAT_SIZE, MIX_FLE);
    downMixer.Process(TEST_BUFFER_LEN, in.data(), testInBufferSize, out.data(), testOutBufferSize);
}

typedef void (*TestFuncs)();
TestFuncs g_testFuncs[TESTSIZE] = {
    SetParamFuzzTest,
    SetDefaultChannelLayoutFuzzTest,
    ProcesFuzzTest1,
    ProcesFuzzTest2,
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
