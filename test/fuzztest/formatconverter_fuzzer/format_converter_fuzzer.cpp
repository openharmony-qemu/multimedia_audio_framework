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
#include "../fuzz_utils.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
const int32_t BUFFER_SIZE_SMALL = 2;
const int32_t BUFFER_SIZE_MEDIUM = 4;
const int32_t BUFFER_SIZE_LARGE = 8;

typedef void (*TestFuncs)();

void S16StereoToF32StereoFuzzTest()
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    uint8_t srcBuffer[BUFFER_SIZE_MEDIUM] = {0};
    uint8_t dstBuffer[BUFFER_SIZE_LARGE] = {0};
    srcDesc.bufLength = BUFFER_SIZE_MEDIUM;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = BUFFER_SIZE_SMALL;
    dstDesc.buffer = dstBuffer;
    FormatConverter::S16StereoToF32Stereo(srcDesc, dstDesc);
    dstDesc.bufLength = BUFFER_SIZE_LARGE;
    FormatConverter::S16StereoToF32Stereo(srcDesc, dstDesc);
}

void S16StereoToF32MonoFuzzTest()
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    uint8_t srcBuffer[BUFFER_SIZE_MEDIUM] = {0};
    uint8_t dstBuffer[BUFFER_SIZE_MEDIUM] = {0};
    srcDesc.bufLength = BUFFER_SIZE_MEDIUM;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = BUFFER_SIZE_SMALL;
    dstDesc.buffer = dstBuffer;
    FormatConverter::S16StereoToF32Mono(srcDesc, dstDesc);
    dstDesc.bufLength = BUFFER_SIZE_MEDIUM;
    FormatConverter::S16StereoToF32Mono(srcDesc, dstDesc);
}

void F32MonoToS16StereoFuzzTest()
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    uint8_t srcBuffer[BUFFER_SIZE_MEDIUM] = {0};
    uint8_t dstBuffer[BUFFER_SIZE_MEDIUM] = {0};
    srcDesc.bufLength = BUFFER_SIZE_MEDIUM;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = BUFFER_SIZE_SMALL;
    dstDesc.buffer = dstBuffer;
    FormatConverter::F32MonoToS16Stereo(srcDesc, dstDesc);
    dstDesc.bufLength = BUFFER_SIZE_MEDIUM;
    FormatConverter::F32MonoToS16Stereo(srcDesc, dstDesc);
}

void F32StereoToS16StereoFuzzTest()
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    uint8_t srcBuffer[BUFFER_SIZE_LARGE] = {0};
    uint8_t dstBuffer[BUFFER_SIZE_MEDIUM] = {0};
    srcDesc.bufLength = BUFFER_SIZE_LARGE;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = BUFFER_SIZE_SMALL;
    dstDesc.buffer = dstBuffer;
    FormatConverter::F32StereoToS16Stereo(srcDesc, dstDesc);
    dstDesc.bufLength = BUFFER_SIZE_MEDIUM;
    FormatConverter::F32StereoToS16Stereo(srcDesc, dstDesc);
}

void S16MonoToS16StereoFuzzTest()
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    uint8_t srcBuffer[BUFFER_SIZE_LARGE] = {0};
    uint8_t dstBuffer[BUFFER_SIZE_MEDIUM] = {0};
    srcDesc.bufLength = BUFFER_SIZE_SMALL;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = BUFFER_SIZE_LARGE;
    dstDesc.buffer = dstBuffer;
    FormatConverter::S16MonoToS16Stereo(srcDesc, dstDesc);
    srcDesc.buffer = nullptr;
    FormatConverter::S16MonoToS16Stereo(srcDesc, dstDesc);
}

void DataAccumulationFromVolumeFuzzTest()
{
    uint8_t srcBuffer[BUFFER_SIZE_LARGE] = {0};
    BufferDesc srcDesc = {srcBuffer, BUFFER_SIZE_LARGE, BUFFER_SIZE_LARGE};
    AudioStreamData srcData;
    srcData.streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S32LE, STEREO};
    srcData.bufferDesc = srcDesc;
    std::vector<AudioStreamData> srcDataList = {srcData};
    uint8_t dstBuffer[BUFFER_SIZE_LARGE] = {0};
    BufferDesc dstDesc = {dstBuffer, BUFFER_SIZE_LARGE, BUFFER_SIZE_LARGE};
    AudioStreamData dstData;
    dstData.streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S32LE, STEREO};
    dstData.bufferDesc = dstDesc;
    dstData.streamInfo.format = g_fuzzUtils.GetData<AudioSampleFormat>();
    FormatConverter::DataAccumulationFromVolume(srcDataList, dstData);
}

void FormatConverterS32MonoToS16StereoFuzzTest()
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    uint8_t srcBuffer[BUFFER_SIZE_LARGE] = {0};
    uint8_t dstBuffer[BUFFER_SIZE_LARGE] = {0};

    srcDesc.bufLength = BUFFER_SIZE_LARGE;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = BUFFER_SIZE_LARGE;
    dstDesc.buffer = dstBuffer;
    FormatConverter::S32MonoToS16Stereo(srcDesc, dstDesc);
}

void FormatConverterS32StereoToS16StereoFuzzTest()
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    uint8_t srcBuffer[BUFFER_SIZE_LARGE] = {0};
    uint8_t dstBuffer[BUFFER_SIZE_MEDIUM] = {0};

    srcDesc.bufLength = BUFFER_SIZE_LARGE;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = BUFFER_SIZE_MEDIUM;
    dstDesc.buffer = dstBuffer;
    FormatConverter::S32StereoToS16Stereo(srcDesc, dstDesc);
}

void FormatConverterS16StereoToS32StereoFuzzTest()
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    uint8_t srcBuffer[BUFFER_SIZE_MEDIUM] = {0};
    uint8_t dstBuffer[BUFFER_SIZE_LARGE] = {0};

    srcDesc.bufLength = BUFFER_SIZE_MEDIUM;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = BUFFER_SIZE_LARGE;
    dstDesc.buffer = dstBuffer;
    FormatConverter::S16StereoToS32Stereo(srcDesc, dstDesc);
}

void FormatConverterS16MonoToS32StereoFuzzTest()
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    uint8_t srcBuffer[BUFFER_SIZE_SMALL] = {0};
    uint8_t dstBuffer[BUFFER_SIZE_LARGE] = {0};

    srcDesc.bufLength = BUFFER_SIZE_SMALL;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = BUFFER_SIZE_LARGE;
    dstDesc.buffer = dstBuffer;
    FormatConverter::S16MonoToS32Stereo(srcDesc, dstDesc);
}

void FormatConverterS32MonoToS32StereoFuzzTest()
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    uint8_t srcBuffer[BUFFER_SIZE_MEDIUM] = {0};
    uint8_t dstBuffer[BUFFER_SIZE_LARGE] = {0};

    srcDesc.bufLength = BUFFER_SIZE_MEDIUM;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = BUFFER_SIZE_LARGE;
    dstDesc.buffer = dstBuffer;
    FormatConverter::S32MonoToS32Stereo(srcDesc, dstDesc);
    FormatConverter::F32MonoToS32Stereo(srcDesc, dstDesc);
}

void FormatConverterF32StereoToS32StereoFuzzTest()
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    uint8_t srcBuffer[BUFFER_SIZE_MEDIUM] = {0};
    uint8_t dstBuffer[BUFFER_SIZE_MEDIUM] = {0};

    srcDesc.bufLength = BUFFER_SIZE_MEDIUM;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = BUFFER_SIZE_MEDIUM;
    dstDesc.buffer = dstBuffer;
    FormatConverter::F32StereoToS32Stereo(srcDesc, dstDesc);
}

vector<TestFuncs> g_testFuncs = {
    S16StereoToF32StereoFuzzTest,
    S16StereoToF32MonoFuzzTest,
    F32MonoToS16StereoFuzzTest,
    F32StereoToS16StereoFuzzTest,
    S16MonoToS16StereoFuzzTest,
    DataAccumulationFromVolumeFuzzTest,
    FormatConverterS32MonoToS16StereoFuzzTest,
    FormatConverterS32StereoToS16StereoFuzzTest,
    FormatConverterS16StereoToS32StereoFuzzTest,
    FormatConverterS16MonoToS32StereoFuzzTest,
    FormatConverterS32MonoToS32StereoFuzzTest,
    FormatConverterF32StereoToS32StereoFuzzTest,
};
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::AudioStandard::g_fuzzUtils.fuzzTest(data, size, OHOS::AudioStandard::g_testFuncs);
    return 0;
}
