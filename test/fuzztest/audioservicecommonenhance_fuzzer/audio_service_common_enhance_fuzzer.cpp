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
#include "audio_down_mix_stereo.h"
#include "audio_log_utils.h"
#include "audio_volume.h"
#include "format_converter.h"
#include "futex_tool.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {
const int32_t LIMITSIZE = 4;
const int32_t FORMAT_COUNT = 5;
const int32_t VOLSTART_COUNT = 1;
const int64_t SILENT_COUNT = 1;
const int64_t SOUND_COUNT = -1;
const uint8_t BUFFER_CONSTANT = 1;
const uint32_t FRAMESIZE = 5;
const uint32_t FRAMESIZE_NEW = 1;
const size_t SIZE_FLOAT = 5;
const float FLOAT_BUFFER = 5.0f;
const float FLOAT_VOLUME = 1.0f;
constexpr int32_t AUDIO_SAMPLE_FORMAT_8BIT = 0;
constexpr int32_t AUDIO_SAMPLE_FORMAT_16BIT = 1;
constexpr int32_t AUDIO_SAMPLE_FORMAT_24BIT = 2;
constexpr int32_t AUDIO_SAMPLE_FORMAT_32BIT = 3;
constexpr int32_t AUDIO_SAMPLE_FORMAT_32F_BIT = 4;

void AudioCommonConverterFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    BufferBaseInfo srcBuffer;
    srcBuffer.frameSize = FRAMESIZE;
    size_t floatBufferSize = SIZE_FLOAT;
    std::vector<float> floatBuffer(floatBufferSize, FLOAT_BUFFER);
    AudioCommonConverter::ConvertBufferToFloat(srcBuffer, floatBuffer);
    AudioCommonConverter::ConvertFloatToFloatWithVolume(srcBuffer, floatBuffer);

    BufferBaseInfo srcBufferTo;
    srcBufferTo.frameSize = FRAMESIZE_NEW;
    std::vector<char> dstBuffer32Bit{'0', '0', '0', '0'};
    std::vector<char> dstBuffer16Bit{'0', '0'};

    srcBufferTo.format = AUDIO_SAMPLE_FORMAT_8BIT;
    AudioCommonConverter::ConvertBufferTo32Bit(srcBufferTo, dstBuffer32Bit);
    AudioCommonConverter::ConvertBufferTo16Bit(srcBufferTo, dstBuffer16Bit);
    srcBufferTo.format = AUDIO_SAMPLE_FORMAT_16BIT;
    AudioCommonConverter::ConvertBufferTo32Bit(srcBufferTo, dstBuffer32Bit);
    AudioCommonConverter::ConvertBufferTo16Bit(srcBufferTo, dstBuffer16Bit);
    srcBufferTo.format = AUDIO_SAMPLE_FORMAT_24BIT;
    AudioCommonConverter::ConvertBufferTo32Bit(srcBufferTo, dstBuffer32Bit);
    AudioCommonConverter::ConvertBufferTo16Bit(srcBufferTo, dstBuffer16Bit);
    srcBufferTo.format = AUDIO_SAMPLE_FORMAT_32BIT;
    AudioCommonConverter::ConvertBufferTo32Bit(srcBufferTo, dstBuffer32Bit);
    AudioCommonConverter::ConvertBufferTo16Bit(srcBufferTo, dstBuffer16Bit);
    srcBufferTo.format = AUDIO_SAMPLE_FORMAT_32F_BIT;
    AudioCommonConverter::ConvertBufferTo32Bit(srcBufferTo, dstBuffer32Bit);
    AudioCommonConverter::ConvertBufferTo16Bit(srcBufferTo, dstBuffer16Bit);
    srcBufferTo.format = FORMAT_COUNT;
    AudioCommonConverter::ConvertBufferTo32Bit(srcBufferTo, dstBuffer32Bit);
    AudioCommonConverter::ConvertBufferTo16Bit(srcBufferTo, dstBuffer16Bit);
}

void AudioDownMixStereoFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioDownMixStereo> audioDownMixStereo = std::make_shared<AudioDownMixStereo>();

    AudioChannelLayout mode = CH_LAYOUT_MONO;
    int32_t channels = *reinterpret_cast<const int32_t*>(rawData);
    audioDownMixStereo->InitMixer(mode, channels);

    int32_t frameLength = *reinterpret_cast<const int32_t*>(rawData);
    float *input = const_cast<float*>(reinterpret_cast<const float*>(rawData));
    float *output = const_cast<float*>(reinterpret_cast<const float*>(rawData));
    audioDownMixStereo->Apply(frameLength, input, output);
}

void AudioLogUtilsFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::string logTag = "logTag";
    ChannelVolumes vols;
    int64_t countSilent = SILENT_COUNT;
    AudioLogUtils::ProcessVolumeData(logTag, vols, countSilent);

    ChannelVolumes volumes;
    volumes.volStart[0] = VOLSTART_COUNT;
    int64_t countSound = SOUND_COUNT;
    AudioLogUtils::ProcessVolumeData(logTag, volumes, countSound);
}

void AudioVolumeFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioVolume> audioVolume = std::make_shared<AudioVolume>();
    uint32_t sessionId = *reinterpret_cast<const uint32_t*>(rawData);
    audioVolume->GetHistoryVolume(sessionId);

    float volume = FLOAT_VOLUME;
    audioVolume->SetHistoryVolume(sessionId, volume);
    audioVolume->SetStreamVolumeDuckFactor(sessionId, volume);
    audioVolume->SetStreamVolumeLowPowerFactor(sessionId, volume);
    audioVolume->GetStreamVolumeFade(sessionId);
    audioVolume->SetStreamVolumeFade(sessionId, volume, volume);

    int32_t volumeType = *reinterpret_cast<const int32_t*>(rawData);
    int32_t volumeLevel = *reinterpret_cast<const int32_t*>(rawData);
    std::string deviceClass = "primary";
    audioVolume->SetSystemVolume(volumeType, deviceClass, volume, volumeLevel);
    audioVolume->SetSystemVolumeMute(volumeType, deviceClass, true);

    std::string streamType = "streamType";
    audioVolume->ConvertStreamTypeStrToInt(streamType);

    float x = FLOAT_VOLUME;
    float y = FLOAT_VOLUME;
    audioVolume->IsSameVolume(x, y);

    std::string dumpString = "dumpString";
    audioVolume->Dump(dumpString);
}

void AudioFormatConverterFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

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
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::AudioStandard::AudioCommonConverterFuzzTest(data, size);
    OHOS::AudioStandard::AudioDownMixStereoFuzzTest(data, size);
    OHOS::AudioStandard::AudioLogUtilsFuzzTest(data, size);
    OHOS::AudioStandard::AudioVolumeFuzzTest(data, size);
    OHOS::AudioStandard::AudioFormatConverterFuzzTest(data, size);
    return 0;
}
