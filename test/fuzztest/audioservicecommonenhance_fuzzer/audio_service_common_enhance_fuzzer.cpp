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
#include "audio_common_converter.h"
#include "pro_renderer_stream_impl.h"
#include "securec.h"
#include "audio_service_log.h"
#include "audio_down_mix_stereo.h"
#include "audio_log_utils.h"
#include "audio_volume.h"
#include "format_converter.h"
#include "futex_tool.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {
const int32_t FORMAT_COUNT = 5;
const int32_t VOLSTART_COUNT = 1;
const int64_t SILENT_COUNT = 1;
const int64_t SOUND_COUNT = -1;
const uint8_t BUFFER_CONSTANT = 1;
const uint32_t FRAMESIZE = 5;
const uint32_t FRAMESIZE_NEW = 1;
const size_t SIZE_FLOAT = 5;
const uint32_t CHANNEL_COUNT = 2;
const float FLOAT_BUFFER = 5.0f;
constexpr int32_t AUDIO_SAMPLE_FORMAT_8BIT = 0;
constexpr int32_t AUDIO_SAMPLE_FORMAT_16BIT = 1;
constexpr int32_t AUDIO_SAMPLE_FORMAT_24BIT = 2;
constexpr int32_t AUDIO_SAMPLE_FORMAT_32BIT = 3;
constexpr int32_t AUDIO_SAMPLE_FORMAT_32F_BIT = 4;
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

void AudioCommonConverterFuzzTest()
{
    BufferBaseInfo srcBuffer;
    std::unique_ptr<int32_t[]> buffer = std::make_unique<int32_t[]>(FRAMESIZE);
    for (size_t i = 0; i < FRAMESIZE; ++i) {
        buffer[i] = static_cast<int32_t>(i);
    }
    srcBuffer.buffer = reinterpret_cast<uint8_t *>(buffer.get());
    srcBuffer.samplePerFrame = 1;
    srcBuffer.channelCount = CHANNEL_COUNT;
    srcBuffer.volumeBg = 0.0f;
    srcBuffer.volumeEd = 1.0f;
    srcBuffer.bufLength = FRAMESIZE;
    srcBuffer.format = AUDIO_SAMPLE_FORMAT_8BIT;
    srcBuffer.frameSize = FRAMESIZE;
    size_t floatBufferSize = SIZE_FLOAT;
    std::vector<float> floatBuffer(floatBufferSize, FLOAT_BUFFER);
    AudioCommonConverter::ConvertBufferToFloat(srcBuffer, floatBuffer);
    AudioCommonConverter::ConvertFloatToFloatWithVolume(srcBuffer, floatBuffer);

    BufferBaseInfo srcBufferTo;
    srcBufferTo.buffer = reinterpret_cast<uint8_t *>(buffer.get());
    srcBufferTo.channelCount = CHANNEL_COUNT;
    srcBufferTo.volumeBg = 0.0f;
    srcBufferTo.volumeEd = 1.0f;
    srcBufferTo.frameSize = FRAMESIZE_NEW;
    std::vector<char> dstBuffer32Bit{'0', '0', '0', '0'};
    std::vector<char> dstBuffer16Bit{'0', '0'};

    std::vector<int32_t> formatVec = {
        AUDIO_SAMPLE_FORMAT_8BIT,
        AUDIO_SAMPLE_FORMAT_16BIT,
        AUDIO_SAMPLE_FORMAT_24BIT,
        AUDIO_SAMPLE_FORMAT_32BIT,
        AUDIO_SAMPLE_FORMAT_32F_BIT,
        FORMAT_COUNT,
    };
    int32_t formatInt = GetData<int32_t>() % formatVec.size();
    srcBufferTo.format = formatVec[formatInt];
    AudioCommonConverter::ConvertBufferTo32Bit(srcBufferTo, dstBuffer32Bit);
    AudioCommonConverter::ConvertBufferTo16Bit(srcBufferTo, dstBuffer16Bit);
}

void AudioDownMixStereoFuzzTest()
{
    std::shared_ptr<AudioDownMixStereo> audioDownMixStereo = std::make_shared<AudioDownMixStereo>();

    AudioChannelLayout mode = GetData<AudioChannelLayout>();
    int32_t channels = GetData<int32_t>();
    audioDownMixStereo->InitMixer(mode, channels);

    int32_t frameLength = GetData<int32_t>();
    float *input = GetData<float*>();
    float *output = GetData<float*>();
    audioDownMixStereo->Apply(frameLength, input, output);
}

void AudioLogUtilsFuzzTest()
{
    std::string logTag = "logTag";
    ChannelVolumes vols;
    int64_t countSilent = SILENT_COUNT;
    AudioLogUtils::ProcessVolumeData(logTag, vols, countSilent);

    ChannelVolumes volumes;
    volumes.volStart[0] = VOLSTART_COUNT;
    int64_t countSound = SOUND_COUNT;
    AudioLogUtils::ProcessVolumeData(logTag, volumes, countSound);
}

void AudioVolumeFuzzTest()
{
    std::shared_ptr<AudioVolume> audioVolume = std::make_shared<AudioVolume>();
    uint32_t sessionId = GetData<uint32_t>();
    audioVolume->GetHistoryVolume(sessionId);

    float volume = GetData<float>();
    audioVolume->SetHistoryVolume(sessionId, volume);
    audioVolume->SetStreamVolumeDuckFactor(sessionId, volume);
    audioVolume->SetStreamVolumeLowPowerFactor(sessionId, volume);

    int32_t volumeType = GetData<int32_t>();
    int32_t volumeLevel = GetData<int32_t>();
    std::string deviceClass = "primary";
    audioVolume->SetSystemVolume(volumeType, deviceClass, volume, volumeLevel);
    audioVolume->SetSystemVolumeMute(volumeType, deviceClass, true);

    std::string streamType = "streamType";
    audioVolume->ConvertStreamTypeStrToInt(streamType);

    float x = GetData<float>();
    float y = GetData<float>();
    audioVolume->IsSameVolume(x, y);

    std::string dumpString = "dumpString";
    audioVolume->Dump(dumpString);
}

void AudioFormatConverterFuzzTest()
{
    BufferDesc srcDesc;
    uint8_t srcBuffer[4] = {0};
    srcBuffer[0] = BUFFER_CONSTANT;
    srcDesc.buffer = srcBuffer;
    BufferDesc dstDesc;
    uint8_t dstBuffer[4] = {0};
    dstBuffer[0] = BUFFER_CONSTANT;
    dstDesc.buffer = dstBuffer;
    FormatConverter::S16MonoToS16Stereo(srcDesc, dstDesc);
    FormatConverter::S16StereoToS16Mono(srcDesc, dstDesc);
}

typedef void (*TestFuncs[5])();

TestFuncs g_testFuncs = {
    AudioCommonConverterFuzzTest,
    AudioDownMixStereoFuzzTest,
    AudioLogUtilsFuzzTest,
    AudioVolumeFuzzTest,
    AudioFormatConverterFuzzTest,
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
