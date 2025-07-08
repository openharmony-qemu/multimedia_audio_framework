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

namespace OHOS {
namespace AudioStandard {
using namespace std;

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
const uint8_t TESTSIZE = 6;
static int32_t NUM_2 = 2;
const int32_t NUM_4 = 2;
const int32_t NUM_8 = 2;

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

void S16StereoToF32StereoFuzzTest()
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    uint8_t srcBuffer[NUM_4] = {0};
    uint8_t dstBuffer[NUM_8] = {0};
    srcDesc.bufLength = NUM_4;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = NUM_2;
    dstDesc.buffer = dstBuffer;
    FormatConverter::S16StereoToF32Stereo(srcDesc, dstDesc);
    dstDesc.bufLength = NUM_8;
    FormatConverter::S16StereoToF32Stereo(srcDesc, dstDesc);
}

void S16StereoToF32MonoFuzzTest()
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    uint8_t srcBuffer[NUM_4] = {0};
    uint8_t dstBuffer[NUM_4] = {0};
    srcDesc.bufLength = NUM_4;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = NUM_2;
    dstDesc.buffer = dstBuffer;
    FormatConverter::S16StereoToF32Mono(srcDesc, dstDesc);
    dstDesc.bufLength = NUM_4;
    FormatConverter::S16StereoToF32Mono(srcDesc, dstDesc);
}

void F32MonoToS16StereoFuzzTest()
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    uint8_t srcBuffer[NUM_4] = {0};
    uint8_t dstBuffer[NUM_4] = {0};
    srcDesc.bufLength = NUM_4;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = NUM_2;
    dstDesc.buffer = dstBuffer;
    FormatConverter::F32MonoToS16Stereo(srcDesc, dstDesc);
    dstDesc.bufLength = NUM_4;
    FormatConverter::F32MonoToS16Stereo(srcDesc, dstDesc);
}

void F32StereoToS16StereoFuzzTest()
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    uint8_t srcBuffer[NUM_8] = {0};
    uint8_t dstBuffer[NUM_4] = {0};
    srcDesc.bufLength = NUM_8;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = NUM_2;
    dstDesc.buffer = dstBuffer;
    FormatConverter::F32StereoToS16Stereo(srcDesc, dstDesc);
    dstDesc.bufLength = NUM_4;
    FormatConverter::F32StereoToS16Stereo(srcDesc, dstDesc);
}

void S16MonoToS16StereoFuzzTest()
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    uint8_t srcBuffer[NUM_8] = {0};
    uint8_t dstBuffer[NUM_4] = {0};
    srcDesc.bufLength = NUM_2;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = NUM_8;
    dstDesc.buffer = dstBuffer;
    FormatConverter::S16MonoToS16Stereo(srcDesc, dstDesc);
    srcDesc.bufLength = NUM_4;
    FormatConverter::S16MonoToS16Stereo(srcDesc, dstDesc);
}

void DataAccumulationFromVolumeFuzzTest()
{
    uint8_t srcBuffer[NUM_8] = {0};
    BufferDesc srcDesc = {srcBuffer, NUM_8, NUM_8};
    AudioStreamData srcData;
    srcData.streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S32LE, STEREO};
    srcData.bufferDesc = srcDesc;
    std::vector<AudioStreamData> srcDataList = {srcData};
    uint8_t dstBuffer[NUM_8] = {0};
    BufferDesc dstDesc = {dstBuffer, NUM_8, NUM_8};
    AudioStreamData dstData;
    dstData.streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S32LE, STEREO};
    dstData.bufferDesc = dstDesc;
    FormatConverter::DataAccumulationFromVolume(srcDataList, dstData);
}

TestFuncs g_testFuncs[TESTSIZE] = {
    S16StereoToF32StereoFuzzTest,
    S16StereoToF32MonoFuzzTest,
    F32MonoToS16StereoFuzzTest,
    F32StereoToS16StereoFuzzTest,
    S16MonoToS16StereoFuzzTest,
    DataAccumulationFromVolumeFuzzTest,
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
