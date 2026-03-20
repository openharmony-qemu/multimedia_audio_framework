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

const size_t ERROR_BUFFER_SIZE = 1;
const size_t S16MONO_BUFFER_SIZE = 2;
const size_t S16STEREO_BUFFER_SIZE = 4;
const size_t S24MONO_BUFFER_SIZE = 3;
const size_t S24STEREO_BUFFER_SIZE = 6;
const size_t S32MONO_BUFFER_SIZE = 4;
const size_t S32STEREO_BUFFER_SIZE = 8;
const size_t F32MONO_BUFFER_SIZE = 4;
const size_t F32STEREO_BUFFER_SIZE = 8;

typedef void (*TestFuncs)();

void S16StereoToF32StereoFuzzTest()
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    uint8_t srcBuffer[S16STEREO_BUFFER_SIZE] = {0};
    uint8_t dstBuffer[S32STEREO_BUFFER_SIZE] = {0};
    srcDesc.bufLength = S16STEREO_BUFFER_SIZE;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = ERROR_BUFFER_SIZE;
    dstDesc.buffer = dstBuffer;
    FormatConverter::S16StereoToF32Stereo(srcDesc, dstDesc);
    dstDesc.bufLength = S32STEREO_BUFFER_SIZE;
    FormatConverter::S16StereoToF32Stereo(srcDesc, dstDesc);
}

void S16StereoToF32MonoFuzzTest()
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    uint8_t srcBuffer[S16STEREO_BUFFER_SIZE] = {0};
    uint8_t dstBuffer[S32MONO_BUFFER_SIZE] = {0};
    srcDesc.bufLength = S16STEREO_BUFFER_SIZE;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = ERROR_BUFFER_SIZE;
    dstDesc.buffer = dstBuffer;
    FormatConverter::S16StereoToF32Mono(srcDesc, dstDesc);
    dstDesc.bufLength = S32MONO_BUFFER_SIZE;
    FormatConverter::S16StereoToF32Mono(srcDesc, dstDesc);
}

void F32MonoToS16StereoFuzzTest()
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    uint8_t srcBuffer[F32MONO_BUFFER_SIZE] = {0};
    uint8_t dstBuffer[S16STEREO_BUFFER_SIZE] = {0};
    srcDesc.bufLength = F32MONO_BUFFER_SIZE;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = ERROR_BUFFER_SIZE;
    dstDesc.buffer = dstBuffer;
    FormatConverter::F32MonoToS16Stereo(srcDesc, dstDesc);
    dstDesc.bufLength = S16STEREO_BUFFER_SIZE;
    FormatConverter::F32MonoToS16Stereo(srcDesc, dstDesc);
}

void F32StereoToS16StereoFuzzTest()
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    uint8_t srcBuffer[F32STEREO_BUFFER_SIZE] = {0};
    uint8_t dstBuffer[S16STEREO_BUFFER_SIZE] = {0};
    srcDesc.bufLength = S32STEREO_BUFFER_SIZE;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = ERROR_BUFFER_SIZE;
    dstDesc.buffer = dstBuffer;
    FormatConverter::F32StereoToS16Stereo(srcDesc, dstDesc);
    dstDesc.bufLength = S16STEREO_BUFFER_SIZE;
    FormatConverter::F32StereoToS16Stereo(srcDesc, dstDesc);
}

void S16MonoToS16StereoFuzzTest()
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    uint8_t srcBuffer[S32STEREO_BUFFER_SIZE] = {0};
    uint8_t dstBuffer[S16STEREO_BUFFER_SIZE] = {0};
    srcDesc.bufLength = S16MONO_BUFFER_SIZE;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = S32STEREO_BUFFER_SIZE;
    dstDesc.buffer = dstBuffer;
    FormatConverter::S16MonoToS16Stereo(srcDesc, dstDesc);
    srcDesc.buffer = nullptr;
    FormatConverter::S16MonoToS16Stereo(srcDesc, dstDesc);
}

void DataAccumulationFromVolumeFuzzTest()
{
    uint8_t srcBuffer[S32STEREO_BUFFER_SIZE] = {0};
    BufferDesc srcDesc = {srcBuffer, S32STEREO_BUFFER_SIZE, S32STEREO_BUFFER_SIZE};
    AudioStreamData srcData;
    srcData.streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S32LE, STEREO};
    srcData.bufferDesc = srcDesc;
    std::vector<AudioStreamData> srcDataList = {srcData};
    uint8_t dstBuffer[S32STEREO_BUFFER_SIZE] = {0};
    BufferDesc dstDesc = {dstBuffer, S32STEREO_BUFFER_SIZE, S32STEREO_BUFFER_SIZE};
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
    uint8_t srcBuffer[S32STEREO_BUFFER_SIZE] = {0};
    uint8_t dstBuffer[S32STEREO_BUFFER_SIZE] = {0};

    srcDesc.bufLength = S32STEREO_BUFFER_SIZE;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = S32STEREO_BUFFER_SIZE;
    dstDesc.buffer = dstBuffer;
    FormatConverter::S32MonoToS16Stereo(srcDesc, dstDesc);
}

void FormatConverterS32StereoToS16StereoFuzzTest()
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    uint8_t srcBuffer[S32STEREO_BUFFER_SIZE] = {0};
    uint8_t dstBuffer[S16STEREO_BUFFER_SIZE] = {0};

    srcDesc.bufLength = S32STEREO_BUFFER_SIZE;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = S16STEREO_BUFFER_SIZE;
    dstDesc.buffer = dstBuffer;
    FormatConverter::S32StereoToS16Stereo(srcDesc, dstDesc);
}

void FormatConverterS16StereoToS32StereoFuzzTest()
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    uint8_t srcBuffer[S16STEREO_BUFFER_SIZE] = {0};
    uint8_t dstBuffer[S32STEREO_BUFFER_SIZE] = {0};

    srcDesc.bufLength = S16STEREO_BUFFER_SIZE;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = S32STEREO_BUFFER_SIZE;
    dstDesc.buffer = dstBuffer;
    FormatConverter::S16StereoToS32Stereo(srcDesc, dstDesc);
}

void FormatConverterS16MonoToS32StereoFuzzTest()
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    uint8_t srcBuffer[S16MONO_BUFFER_SIZE] = {0};
    uint8_t dstBuffer[S32STEREO_BUFFER_SIZE] = {0};

    srcDesc.bufLength = S16MONO_BUFFER_SIZE;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = S32STEREO_BUFFER_SIZE;
    dstDesc.buffer = dstBuffer;
    FormatConverter::S16MonoToS32Stereo(srcDesc, dstDesc);
}

void FormatConverterS32MonoToS32StereoFuzzTest()
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    uint8_t srcBuffer[S16STEREO_BUFFER_SIZE] = {0};
    uint8_t dstBuffer[S32STEREO_BUFFER_SIZE] = {0};

    srcDesc.bufLength = S16STEREO_BUFFER_SIZE;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = S32STEREO_BUFFER_SIZE;
    dstDesc.buffer = dstBuffer;
    FormatConverter::S32MonoToS32Stereo(srcDesc, dstDesc);
    FormatConverter::F32MonoToS32Stereo(srcDesc, dstDesc);
}

void FormatConverterF32StereoToS32StereoFuzzTest()
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    uint8_t srcBuffer[S16STEREO_BUFFER_SIZE] = {0};
    uint8_t dstBuffer[S16STEREO_BUFFER_SIZE] = {0};

    srcDesc.bufLength = S16STEREO_BUFFER_SIZE;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = S16STEREO_BUFFER_SIZE;
    dstDesc.buffer = dstBuffer;
    FormatConverter::F32StereoToS32Stereo(srcDesc, dstDesc);
}

void DataAccumulationWithoutVolumeFuzzTest()
{
    uint8_t srcBuffer[S32STEREO_BUFFER_SIZE] = {0};
    BufferDesc srcDesc = {srcBuffer, S32STEREO_BUFFER_SIZE, S32STEREO_BUFFER_SIZE};
    AudioStreamData srcData;
    srcData.streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S32LE, STEREO};
    srcData.bufferDesc = srcDesc;
    std::vector<AudioStreamData> srcDataList = {srcData};
    uint8_t dstBuffer[S32STEREO_BUFFER_SIZE] = {0};
    BufferDesc dstDesc = {dstBuffer, S32STEREO_BUFFER_SIZE, S32STEREO_BUFFER_SIZE};
    AudioStreamData dstData;
    dstData.streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S32LE, STEREO};
    dstData.bufferDesc = dstDesc;
    dstData.streamInfo.format = g_fuzzUtils.GetData<AudioSampleFormat>();
    FormatConverter::DataAccumulationWithoutVolume(srcDataList, dstData);
}

void F32StereoToF32MonoFuzzTest()
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    uint8_t srcBuffer[S16STEREO_BUFFER_SIZE] = {0};
    uint8_t dstBuffer[S16STEREO_BUFFER_SIZE] = {0};

    srcDesc.bufLength = S16STEREO_BUFFER_SIZE;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = S16STEREO_BUFFER_SIZE;
    dstDesc.buffer = dstBuffer;
    FormatConverter::F32StereoToF32Mono(srcDesc, dstDesc);
}

void F32StereoToS16MonoFuzzTest()
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    uint8_t srcBuffer[S16STEREO_BUFFER_SIZE] = {0};
    uint8_t dstBuffer[S16STEREO_BUFFER_SIZE] = {0};

    srcDesc.bufLength = S16STEREO_BUFFER_SIZE;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = S16STEREO_BUFFER_SIZE;
    dstDesc.buffer = dstBuffer;
    FormatConverter::F32StereoToS16Mono(srcDesc, dstDesc);
}

void S16MonoToF32StereoFuzzTest()
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    uint8_t srcBuffer[S16STEREO_BUFFER_SIZE] = {0};
    uint8_t dstBuffer[S16STEREO_BUFFER_SIZE] = {0};

    srcDesc.bufLength = S16STEREO_BUFFER_SIZE;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = S16STEREO_BUFFER_SIZE;
    dstDesc.buffer = dstBuffer;
    FormatConverter::S16MonoToF32Stereo(srcDesc, dstDesc);
}

void S32MonoToF32StereoFuzzTest()
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    uint8_t srcBuffer[S16STEREO_BUFFER_SIZE] = {0};
    uint8_t dstBuffer[S16STEREO_BUFFER_SIZE] = {0};

    srcDesc.bufLength = S16STEREO_BUFFER_SIZE;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = S16STEREO_BUFFER_SIZE;
    dstDesc.buffer = dstBuffer;
    FormatConverter::S32MonoToF32Stereo(srcDesc, dstDesc);
}

void S32StereoToF32StereoFuzzTest()
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    uint8_t srcBuffer[S16STEREO_BUFFER_SIZE] = {0};
    uint8_t dstBuffer[S16STEREO_BUFFER_SIZE] = {0};

    srcDesc.bufLength = S16STEREO_BUFFER_SIZE;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = S16STEREO_BUFFER_SIZE;
    dstDesc.buffer = dstBuffer;
    FormatConverter::S32StereoToF32Stereo(srcDesc, dstDesc);
}

void F32MonoToF32StereoFuzzTest()
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    uint8_t srcBuffer[S16STEREO_BUFFER_SIZE] = {0};
    uint8_t dstBuffer[S16STEREO_BUFFER_SIZE] = {0};

    srcDesc.bufLength = S16STEREO_BUFFER_SIZE;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = S16STEREO_BUFFER_SIZE;
    dstDesc.buffer = dstBuffer;
    FormatConverter::F32MonoToF32Stereo(srcDesc, dstDesc);
}

void S16MonoToF32MonoFuzzTest()
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    uint8_t srcBuffer[S16STEREO_BUFFER_SIZE] = {0};
    uint8_t dstBuffer[S16STEREO_BUFFER_SIZE] = {0};

    srcDesc.bufLength = S16STEREO_BUFFER_SIZE;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = S16STEREO_BUFFER_SIZE;
    dstDesc.buffer = dstBuffer;
    FormatConverter::S16MonoToF32Mono(srcDesc, dstDesc);
}

void S32MonoToF32MonoFuzzTest()
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    uint8_t srcBuffer[S16STEREO_BUFFER_SIZE] = {0};
    uint8_t dstBuffer[S16STEREO_BUFFER_SIZE] = {0};

    srcDesc.bufLength = S16STEREO_BUFFER_SIZE;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = S16STEREO_BUFFER_SIZE;
    dstDesc.buffer = dstBuffer;
    FormatConverter::S32MonoToF32Mono(srcDesc, dstDesc);
}

void S32StereoToF32MonoFuzzTest()
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    uint8_t srcBuffer[S16STEREO_BUFFER_SIZE] = {0};
    uint8_t dstBuffer[S16STEREO_BUFFER_SIZE] = {0};

    srcDesc.bufLength = S16STEREO_BUFFER_SIZE;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = S16STEREO_BUFFER_SIZE;
    dstDesc.buffer = dstBuffer;
    FormatConverter::S32StereoToF32Mono(srcDesc, dstDesc);
}

void GetFormatHandlersFuzzTest()
{
    FormatConverter::GetFormatHandlers();
}

void S16MonoToS24StereoFuzzTest()
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    uint8_t srcBuffer[S16MONO_BUFFER_SIZE] = {0};
    uint8_t dstBuffer[S24STEREO_BUFFER_SIZE] = {0};
    srcDesc.bufLength = S16MONO_BUFFER_SIZE;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = S24STEREO_BUFFER_SIZE;
    dstDesc.buffer = dstBuffer;
    FormatConverter::S16MonoToS24Stereo(srcDesc, dstDesc);
}

void S16StereoToS24StereoFuzzTest()
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    uint8_t srcBuffer[S16STEREO_BUFFER_SIZE] = {0};
    uint8_t dstBuffer[S24STEREO_BUFFER_SIZE] = {0};
    srcDesc.bufLength = S16STEREO_BUFFER_SIZE;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = S24STEREO_BUFFER_SIZE;
    dstDesc.buffer = dstBuffer;
    FormatConverter::S16StereoToS24Stereo(srcDesc, dstDesc);
}

void S32MonoToS24StereoFuzzTest()
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    uint8_t srcBuffer[S16STEREO_BUFFER_SIZE] = {0};
    uint8_t dstBuffer[S24STEREO_BUFFER_SIZE] = {0};
    srcDesc.bufLength = S16STEREO_BUFFER_SIZE;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = S24STEREO_BUFFER_SIZE;
    dstDesc.buffer = dstBuffer;
    FormatConverter::S32MonoToS24Stereo(srcDesc, dstDesc);
}

void S32StereoToS24StereoFuzzTest()
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    uint8_t srcBuffer[S32STEREO_BUFFER_SIZE] = {0};
    uint8_t dstBuffer[S24STEREO_BUFFER_SIZE] = {0};
    srcDesc.bufLength = S32STEREO_BUFFER_SIZE;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = S24STEREO_BUFFER_SIZE;
    dstDesc.buffer = dstBuffer;
    FormatConverter::S32StereoToS24Stereo(srcDesc, dstDesc);
}

void F32StereoToS24StereoFuzzTest()
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    uint8_t srcBuffer[S16STEREO_BUFFER_SIZE] = {0};
    uint8_t dstBuffer[S24STEREO_BUFFER_SIZE] = {0};
    srcDesc.bufLength = S16STEREO_BUFFER_SIZE;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = S24STEREO_BUFFER_SIZE;
    dstDesc.buffer = dstBuffer;
    FormatConverter::F32StereoToS24Stereo(srcDesc, dstDesc);
}

void S16MonoToS24MonoFuzzTest()
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    uint8_t srcBuffer[S16MONO_BUFFER_SIZE] = {0};
    uint8_t dstBuffer[S24MONO_BUFFER_SIZE] = {0};
    srcDesc.bufLength = S16MONO_BUFFER_SIZE;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = S24MONO_BUFFER_SIZE;
    dstDesc.buffer = dstBuffer;
    FormatConverter::S16MonoToS24Mono(srcDesc, dstDesc);
}

void S16StereoToS24MonoFuzzTest()
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    uint8_t srcBuffer[S16STEREO_BUFFER_SIZE] = {0};
    uint8_t dstBuffer[S24MONO_BUFFER_SIZE] = {0};
    srcDesc.bufLength = S16STEREO_BUFFER_SIZE;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = S24MONO_BUFFER_SIZE;
    dstDesc.buffer = dstBuffer;
    FormatConverter::S16StereoToS24Mono(srcDesc, dstDesc);
}

void S32MonoToS24MonoFuzzTest()
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    uint8_t srcBuffer[S16STEREO_BUFFER_SIZE] = {0};
    uint8_t dstBuffer[S24MONO_BUFFER_SIZE] = {0};
    srcDesc.bufLength = S16STEREO_BUFFER_SIZE;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = S24MONO_BUFFER_SIZE;
    dstDesc.buffer = dstBuffer;
    FormatConverter::S32MonoToS24Mono(srcDesc, dstDesc);
}

void S32StereoToS24MonoFuzzTest()
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    uint8_t srcBuffer[S32STEREO_BUFFER_SIZE] = {0};
    uint8_t dstBuffer[S24MONO_BUFFER_SIZE] = {0};
    srcDesc.bufLength = S32STEREO_BUFFER_SIZE;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = S24MONO_BUFFER_SIZE;
    dstDesc.buffer = dstBuffer;
    FormatConverter::S32StereoToS24Mono(srcDesc, dstDesc);
}

void F32MonoToS24MonoFuzzTest()
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    uint8_t srcBuffer[S16STEREO_BUFFER_SIZE] = {0};
    uint8_t dstBuffer[S24MONO_BUFFER_SIZE] = {0};
    srcDesc.bufLength = S16STEREO_BUFFER_SIZE;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = S24MONO_BUFFER_SIZE;
    dstDesc.buffer = dstBuffer;
    FormatConverter::F32MonoToS24Mono(srcDesc, dstDesc);
}

void F32StereoToS24MonoFuzzTest()
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    uint8_t srcBuffer[S32STEREO_BUFFER_SIZE] = {0};
    uint8_t dstBuffer[S24MONO_BUFFER_SIZE] = {0};
    srcDesc.bufLength = S32STEREO_BUFFER_SIZE;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = S24MONO_BUFFER_SIZE;
    dstDesc.buffer = dstBuffer;
    FormatConverter::F32StereoToS24Mono(srcDesc, dstDesc);
}

void AutoConvertFuzzTest()
{
    FormatKey key;
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    uint8_t srcBuffer[S16STEREO_BUFFER_SIZE] = {0};
    uint8_t dstBuffer[S32STEREO_BUFFER_SIZE] = {0};
    srcDesc.bufLength = S16STEREO_BUFFER_SIZE;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = S16MONO_BUFFER_SIZE;
    dstDesc.buffer = dstBuffer;
    FormatConverter::AutoConvert(key, srcDesc, dstDesc);
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
    DataAccumulationWithoutVolumeFuzzTest,
    F32StereoToF32MonoFuzzTest,
    F32StereoToS16MonoFuzzTest,
    S16MonoToF32StereoFuzzTest,
    S32MonoToF32StereoFuzzTest,
    S32StereoToF32StereoFuzzTest,
    F32MonoToF32StereoFuzzTest,
    S16MonoToF32MonoFuzzTest,
    S32MonoToF32MonoFuzzTest,
    S32StereoToF32MonoFuzzTest,
    GetFormatHandlersFuzzTest,
    S16MonoToS24StereoFuzzTest,
    S16StereoToS24StereoFuzzTest,
    S32MonoToS24StereoFuzzTest,
    S32StereoToS24StereoFuzzTest,
    F32StereoToS24StereoFuzzTest,
    S16MonoToS24MonoFuzzTest,
    S16StereoToS24MonoFuzzTest,
    S32MonoToS24MonoFuzzTest,
    S32StereoToS24MonoFuzzTest,
    F32MonoToS24MonoFuzzTest,
    F32StereoToS24MonoFuzzTest,
};
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::AudioStandard::g_fuzzUtils.fuzzTest(data, size, OHOS::AudioStandard::g_testFuncs);
    return 0;
}
