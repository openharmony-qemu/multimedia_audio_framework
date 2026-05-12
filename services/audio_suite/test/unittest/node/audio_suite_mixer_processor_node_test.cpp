/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include <vector>
#include <cmath>
#include <memory>
#include <fstream>
#include <cstring>
#include "audio_suite_node.h"
#include "audio_errors.h"

#include "audio_suite_mixer_node.h"
#include "audio_limiter.h"
#include "audio_suite_unittest_tools.h"
#include "audio_suite_mixer_processor_node.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;
using namespace testing;

namespace {
class AudioSuiteMixerProcessorTest : public testing::Test {
public:
    void SetUp();
    void TearDown() {};
};

void AudioSuiteMixerProcessorTest::SetUp()
{
    if (!AllNodeTypesSupported()) {
        GTEST_SKIP() << "not support all node types, skip this test";
    }
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorInit_NormalCase_StereoF32LE, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_F32LE;
    format.rate = SAMPLE_RATE_48000;

    int32_t ret = mixerProcessor.Init(format);
    EXPECT_EQ(ret, SUCCESS);
    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorInit_AlreadyInitialized, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_F32LE;
    format.rate = SAMPLE_RATE_48000;

    int32_t ret = mixerProcessor.Init(format);
    EXPECT_EQ(ret, SUCCESS);

    ret = mixerProcessor.Init(format);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorInit_InvalidChannelLayout, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = static_cast<AudioChannelLayout>(999);
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_F32LE;
    format.rate = SAMPLE_RATE_48000;

    int32_t ret = mixerProcessor.Init(format);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorInit_ChannelNumMismatch, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_MONO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_F32LE;
    format.rate = SAMPLE_RATE_48000;

    int32_t ret = mixerProcessor.Init(format);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorInit_InvalidFormat, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = static_cast<AudioSampleFormat>(999);
    format.rate = SAMPLE_RATE_48000;

    int32_t ret = mixerProcessor.Init(format);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorInit_InvalidSampleRate, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_F32LE;
    format.rate = static_cast<AudioSamplingRate>(12345);

    int32_t ret = mixerProcessor.Init(format);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorInit_MonoToStereo, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_MONO;
    format.audioChannelInfo.numChannels = MONO;
    format.format = SAMPLE_F32LE;
    format.rate = SAMPLE_RATE_48000;

    int32_t ret = mixerProcessor.Init(format);
    EXPECT_EQ(ret, SUCCESS);
    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorInit_S16LEFormat, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_S16LE;
    format.rate = SAMPLE_RATE_48000;

    int32_t ret = mixerProcessor.Init(format);
    EXPECT_EQ(ret, SUCCESS);
    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorInit_U8Format, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_U8;
    format.rate = SAMPLE_RATE_48000;

    int32_t ret = mixerProcessor.Init(format);
    EXPECT_EQ(ret, SUCCESS);
    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorInit_S24LEFormat, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_S24LE;
    format.rate = SAMPLE_RATE_48000;

    int32_t ret = mixerProcessor.Init(format);
    EXPECT_EQ(ret, SUCCESS);
    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorInit_S32LEFormat, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_S32LE;
    format.rate = SAMPLE_RATE_48000;

    int32_t ret = mixerProcessor.Init(format);
    EXPECT_EQ(ret, SUCCESS);
    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorInit_DifferentSampleRates, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_F32LE;

    std::vector<AudioSamplingRate> sampleRates = {
        SAMPLE_RATE_8000, SAMPLE_RATE_11025, SAMPLE_RATE_12000,
        SAMPLE_RATE_16000, SAMPLE_RATE_22050, SAMPLE_RATE_24000,
        SAMPLE_RATE_32000, SAMPLE_RATE_44100, SAMPLE_RATE_48000,
        SAMPLE_RATE_64000, SAMPLE_RATE_88200, SAMPLE_RATE_96000,
        SAMPLE_RATE_176400, SAMPLE_RATE_192000, SAMPLE_RATE_384000
    };

    for (auto rate : sampleRates) {
        format.rate = rate;
        int32_t ret = mixerProcessor.Init(format);
        EXPECT_EQ(ret, SUCCESS);
        mixerProcessor.Destroy();
    }
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_NotInitialized, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;

    std::vector<AudioMixStream> streams;
    AudioMixStream stream;
    stream.data = nullptr;
    stream.dataSize = 0;
    streams.push_back(stream);

    std::vector<uint8_t> outputBuffer(1024);
    uint32_t outSize = 0;

    int32_t ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_EmptyStreams, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_F32LE;
    format.rate = SAMPLE_RATE_48000;

    mixerProcessor.Init(format);

    std::vector<AudioMixStream> streams;
    std::vector<uint8_t> outputBuffer(1024);
    uint32_t outSize = 0;

    int32_t ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_NullOutData, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_F32LE;
    format.rate = SAMPLE_RATE_48000;

    mixerProcessor.Init(format);

    std::vector<AudioMixStream> streams;
    AudioMixStream stream;
    std::vector<float> inputData(256);
    stream.data = inputData.data();
    stream.dataSize = inputData.size() * sizeof(float);
    streams.push_back(stream);

    uint32_t outSize = 0;

    int32_t ret = mixerProcessor.Process(streams, nullptr, 1024, &outSize);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_NullOutSize, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_F32LE;
    format.rate = SAMPLE_RATE_48000;

    mixerProcessor.Init(format);

    std::vector<AudioMixStream> streams;
    AudioMixStream stream;
    std::vector<float> inputData(256);
    stream.data = inputData.data();
    stream.dataSize = inputData.size() * sizeof(float);
    streams.push_back(stream);

    std::vector<uint8_t> outputBuffer(1024);

    int32_t ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_StreamDataSizeMismatch, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_F32LE;
    format.rate = SAMPLE_RATE_48000;

    mixerProcessor.Init(format);

    std::vector<AudioMixStream> streams;
    AudioMixStream stream1;
    std::vector<float> inputData1(256);
    stream1.data = inputData1.data();
    stream1.dataSize = inputData1.size() * sizeof(float);
    streams.push_back(stream1);

    AudioMixStream stream2;
    std::vector<float> inputData2(128);
    stream2.data = inputData2.data();
    stream2.dataSize = inputData2.size() * sizeof(float);
    streams.push_back(stream2);

    std::vector<uint8_t> outputBuffer(1024);
    uint32_t outSize = 0;

    int32_t ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_ExceedMaxProcessLen, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_F32LE;
    format.rate = SAMPLE_RATE_48000;

    mixerProcessor.Init(format);

    std::vector<AudioMixStream> streams;
    AudioMixStream stream;
    uint32_t maxDataLen = STEREO * sizeof(float) * 48000 * 60 / 1000;
    std::vector<float> inputData(maxDataLen / sizeof(float) + 100);
    stream.data = inputData.data();
    stream.dataSize = inputData.size() * sizeof(float);
    streams.push_back(stream);

    std::vector<uint8_t> outputBuffer(stream.dataSize);
    uint32_t outSize = 0;

    int32_t ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_NotAlignedDataSize, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_F32LE;
    format.rate = SAMPLE_RATE_48000;

    mixerProcessor.Init(format);

    std::vector<AudioMixStream> streams;
    AudioMixStream stream;
    std::vector<float> inputData(255);
    stream.data = inputData.data();
    stream.dataSize = inputData.size() * sizeof(float);
    streams.push_back(stream);

    std::vector<uint8_t> outputBuffer(1024);
    uint32_t outSize = 0;

    int32_t ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_InsufficientOutputCapacity, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_F32LE;
    format.rate = SAMPLE_RATE_48000;

    mixerProcessor.Init(format);

    std::vector<AudioMixStream> streams;
    AudioMixStream stream;
    std::vector<float> inputData(256);
    stream.data = inputData.data();
    stream.dataSize = inputData.size() * sizeof(float);
    streams.push_back(stream);

    std::vector<uint8_t> outputBuffer(100);
    uint32_t outSize = 0;

    int32_t ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_NullStreamData, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_F32LE;
    format.rate = SAMPLE_RATE_48000;

    mixerProcessor.Init(format);

    std::vector<AudioMixStream> streams;
    AudioMixStream stream;
    stream.data = nullptr;
    stream.dataSize = 256 * sizeof(float);
    streams.push_back(stream);

    std::vector<uint8_t> outputBuffer(1024);
    uint32_t outSize = 0;

    int32_t ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_NoLimiterNullStreamData, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_F32LE;
    format.rate = SAMPLE_RATE_48000;

    mixerProcessor.Init(format, false);

    std::vector<AudioMixStream> streams;
    AudioMixStream stream;
    stream.data = nullptr;
    stream.dataSize = 256 * sizeof(float);
    streams.push_back(stream);

    std::vector<uint8_t> outputBuffer(1024);
    uint32_t outSize = 0;

    int32_t ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_NormalCase_SingleStream, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_F32LE;
    format.rate = SAMPLE_RATE_48000;

    mixerProcessor.Init(format);

    std::vector<AudioMixStream> streams;
    AudioMixStream stream;
    std::vector<float> inputData(256);
    for (size_t i = 0; i < inputData.size(); ++i) {
        inputData[i] = 0.1f;
    }
    stream.data = inputData.data();
    stream.dataSize = inputData.size() * sizeof(float);
    streams.push_back(stream);

    std::vector<uint8_t> outputBuffer(1024);
    uint32_t outSize = 0;

    int32_t ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(outSize, stream.dataSize);

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_NormalCase_MultipleStreams, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_F32LE;
    format.rate = SAMPLE_RATE_48000;
    mixerProcessor.Init(format);

    std::vector<AudioMixStream> streams;
    const size_t frameCount = 256;
    const float value = 0.1f;
    for (int i = 0; i < 3; ++i) {
        AudioMixStream stream;
        std::vector<float> inputData(frameCount, value);

        stream.data = inputData.data();
        stream.dataSize = inputData.size() * sizeof(float);
        streams.push_back(stream);
    }

    std::vector<uint8_t> outputBuffer(1024);
    uint32_t outSize = 0;
    int32_t ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(outSize, streams[0].dataSize);
    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_WithFormatConvert_S16LE, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_S16LE;
    format.rate = SAMPLE_RATE_48000;

    mixerProcessor.Init(format);

    std::vector<AudioMixStream> streams;
    AudioMixStream stream;
    std::vector<int16_t> inputData(256);
    for (size_t i = 0; i < inputData.size(); ++i) {
        inputData[i] = 1000;
    }
    stream.data = inputData.data();
    stream.dataSize = inputData.size() * sizeof(int16_t);
    streams.push_back(stream);

    std::vector<uint8_t> outputBuffer(1024);
    uint32_t outSize = 0;

    int32_t ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(outSize, stream.dataSize);

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_WithChannelConvert_Mono, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_MONO;
    format.audioChannelInfo.numChannels = MONO;
    format.format = SAMPLE_F32LE;
    format.rate = SAMPLE_RATE_48000;

    mixerProcessor.Init(format);

    std::vector<AudioMixStream> streams;
    AudioMixStream stream;
    std::vector<float> inputData(128);
    for (size_t i = 0; i < inputData.size(); ++i) {
        inputData[i] = 0.1f;
    }
    stream.data = inputData.data();
    stream.dataSize = inputData.size() * sizeof(float);
    streams.push_back(stream);

    std::vector<uint8_t> outputBuffer(1024);
    uint32_t outSize = 0;

    int32_t ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(outSize, stream.dataSize);

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_WithBothConverts_MonoS16LE, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_MONO;
    format.audioChannelInfo.numChannels = MONO;
    format.format = SAMPLE_S16LE;
    format.rate = SAMPLE_RATE_48000;

    mixerProcessor.Init(format);

    std::vector<AudioMixStream> streams;
    AudioMixStream stream;
    std::vector<int16_t> inputData(128);
    for (size_t i = 0; i < inputData.size(); ++i) {
        inputData[i] = 1000;
    }
    stream.data = inputData.data();
    stream.dataSize = inputData.size() * sizeof(int16_t);
    streams.push_back(stream);

    std::vector<uint8_t> outputBuffer(1024);
    uint32_t outSize = 0;

    int32_t ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(outSize, stream.dataSize);

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_MultipleTimes, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_F32LE;
    format.rate = SAMPLE_RATE_48000;

    mixerProcessor.Init(format);

    std::vector<AudioMixStream> streams;
    AudioMixStream stream;
    std::vector<float> inputData(256);
    for (size_t i = 0; i < inputData.size(); ++i) {
        inputData[i] = 0.1f;
    }
    stream.data = inputData.data();
    stream.dataSize = inputData.size() * sizeof(float);
    streams.push_back(stream);

    std::vector<uint8_t> outputBuffer(1024);
    uint32_t outSize = 0;

    for (int i = 0; i < 10; ++i) {
        outSize = 0;
        int32_t ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
        EXPECT_EQ(ret, SUCCESS);
        EXPECT_EQ(outSize, stream.dataSize);
    }

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_DifferentChunkSizes, TestSize.Level0)
{
    std::vector<uint32_t> chunkSizes = {128, 256, 512, 1024};
    for (auto chunkSize : chunkSizes) {
        MixerProcessorNode mixerProcessor;
        AudioFormat format;
        format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
        format.audioChannelInfo.numChannels = STEREO;
        format.format = SAMPLE_F32LE;
        format.rate = SAMPLE_RATE_48000;

        mixerProcessor.Init(format);

        std::vector<AudioMixStream> streams;
        AudioMixStream stream;
        std::vector<float> inputData(chunkSize * STEREO);
        for (size_t i = 0; i < inputData.size(); ++i) {
            inputData[i] = 0.1f;
        }
        stream.data = inputData.data();
        stream.dataSize = inputData.size() * sizeof(float);
        streams.push_back(stream);

        std::vector<uint8_t> outputBuffer(chunkSize * sizeof(float) * STEREO);
        uint32_t outSize = 0;

        int32_t ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
        EXPECT_EQ(ret, SUCCESS);
        EXPECT_EQ(outSize, stream.dataSize);

        mixerProcessor.Destroy();
    }
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorDestroy_MultipleTimes, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_F32LE;
    format.rate = SAMPLE_RATE_48000;

    mixerProcessor.Init(format);

    mixerProcessor.Destroy();
    mixerProcessor.Destroy();

    std::vector<AudioMixStream> streams;
    AudioMixStream stream;
    std::vector<float> inputData(256);
    stream.data = inputData.data();
    stream.dataSize = inputData.size() * sizeof(float);
    streams.push_back(stream);

    std::vector<uint8_t> outputBuffer(1024);
    uint32_t outSize = 0;

    int32_t ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_5Point1Channel, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_5POINT1;
    format.audioChannelInfo.numChannels = CHANNEL_6;
    format.format = SAMPLE_F32LE;
    format.rate = SAMPLE_RATE_48000;

    int32_t ret = mixerProcessor.Init(format);
    EXPECT_EQ(ret, SUCCESS);

    std::vector<AudioMixStream> streams;
    AudioMixStream stream;
    std::vector<float> inputData(256 * 3);
    for (size_t i = 0; i < inputData.size(); ++i) {
        inputData[i] = 0.1f;
    }
    stream.data = inputData.data();
    stream.dataSize = inputData.size() * sizeof(float);
    streams.push_back(stream);

    std::vector<uint8_t> outputBuffer(stream.dataSize);
    uint32_t outSize = 0;

    ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(outSize, stream.dataSize);

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_7Point1Channel, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_7POINT1;
    format.audioChannelInfo.numChannels = CHANNEL_8;
    format.format = SAMPLE_F32LE;
    format.rate = SAMPLE_RATE_48000;

    int32_t ret = mixerProcessor.Init(format);
    EXPECT_EQ(ret, SUCCESS);

    std::vector<AudioMixStream> streams;
    AudioMixStream stream;
    std::vector<float> inputData(256 * 4);
    for (size_t i = 0; i < inputData.size(); ++i) {
        inputData[i] = 0.1f;
    }
    stream.data = inputData.data();
    stream.dataSize = inputData.size() * sizeof(float);
    streams.push_back(stream);

    std::vector<uint8_t> outputBuffer(stream.dataSize);
    uint32_t outSize = 0;

    ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(outSize, stream.dataSize);

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_ZeroDataSize, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_F32LE;
    format.rate = SAMPLE_RATE_48000;

    mixerProcessor.Init(format);

    std::vector<AudioMixStream> streams;
    AudioMixStream stream;
    std::vector<float> inputData(0);
    stream.data = inputData.data();
    stream.dataSize = 0;
    streams.push_back(stream);

    std::vector<uint8_t> outputBuffer(1024);
    uint32_t outSize = 0;

    int32_t ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_VolumeAttribute, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_F32LE;
    format.rate = SAMPLE_RATE_48000;

    mixerProcessor.Init(format);

    std::vector<AudioMixStream> streams;
    AudioMixStream stream;
    std::vector<float> inputData(256);
    for (size_t i = 0; i < inputData.size(); ++i) {
        inputData[i] = 0.1f;
    }
    stream.data = inputData.data();
    stream.dataSize = inputData.size() * sizeof(float);
    stream.volume = 0.5f;
    streams.push_back(stream);

    std::vector<uint8_t> outputBuffer(1024);
    uint32_t outSize = 0;

    int32_t ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(outSize, stream.dataSize);

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_MinSampleRate, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_F32LE;
    format.rate = SAMPLE_RATE_8000;

    mixerProcessor.Init(format);

    std::vector<AudioMixStream> streams;
    AudioMixStream stream;
    std::vector<float> inputData(256);
    stream.data = inputData.data();
    stream.dataSize = inputData.size() * sizeof(float);
    streams.push_back(stream);

    std::vector<uint8_t> outputBuffer(1024);
    uint32_t outSize = 0;

    int32_t ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, SUCCESS);

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_MaxSampleRate, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_F32LE;
    format.rate = SAMPLE_RATE_384000;

    mixerProcessor.Init(format);

    std::vector<AudioMixStream> streams;
    AudioMixStream stream;
    std::vector<float> inputData(256);
    stream.data = inputData.data();
    stream.dataSize = inputData.size() * sizeof(float);
    streams.push_back(stream);

    std::vector<uint8_t> outputBuffer(1024);
    uint32_t outSize = 0;

    int32_t ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, SUCCESS);

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_WithFormatConvert_U8, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_U8;
    format.rate = SAMPLE_RATE_48000;

    mixerProcessor.Init(format);

    std::vector<AudioMixStream> streams;
    AudioMixStream stream;
    std::vector<uint8_t> inputData(256);
    for (size_t i = 0; i < inputData.size(); ++i) {
        inputData[i] = 128;
    }
    stream.data = inputData.data();
    stream.dataSize = inputData.size() * sizeof(uint8_t);
    streams.push_back(stream);

    std::vector<uint8_t> outputBuffer(1024);
    uint32_t outSize = 0;

    int32_t ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(outSize, stream.dataSize);

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_WithFormatConvert_S24LE, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_S24LE;
    format.rate = SAMPLE_RATE_48000;

    mixerProcessor.Init(format);

    std::vector<AudioMixStream> streams;
    AudioMixStream stream;
    std::vector<int32_t> inputData(256);
    for (size_t i = 0; i < inputData.size(); ++i) {
        inputData[i] = 1000;
    }
    stream.data = inputData.data();
    stream.dataSize = inputData.size() * 3; // S24LE uses 3 bytes per sample
    streams.push_back(stream);

    std::vector<uint8_t> outputBuffer(1024);
    uint32_t outSize = 0;

    int32_t ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(outSize, stream.dataSize);

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_WithFormatConvert_S32LE, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_S32LE;
    format.rate = SAMPLE_RATE_48000;

    mixerProcessor.Init(format);

    std::vector<AudioMixStream> streams;
    AudioMixStream stream;
    std::vector<int32_t> inputData(256);
    for (size_t i = 0; i < inputData.size(); ++i) {
        inputData[i] = 1000;
    }
    stream.data = inputData.data();
    stream.dataSize = inputData.size() * sizeof(int32_t);
    streams.push_back(stream);

    std::vector<uint8_t> outputBuffer(1024);
    uint32_t outSize = 0;

    int32_t ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(outSize, stream.dataSize);

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_VolumeMin, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_F32LE;
    format.rate = SAMPLE_RATE_48000;

    mixerProcessor.Init(format);

    std::vector<AudioMixStream> streams;
    AudioMixStream stream;
    std::vector<float> inputData(256);
    for (size_t i = 0; i < inputData.size(); ++i) {
        inputData[i] = 0.1f;
    }
    stream.data = inputData.data();
    stream.dataSize = inputData.size() * sizeof(float);
    stream.volume = 0.0f;
    streams.push_back(stream);

    std::vector<uint8_t> outputBuffer(1024);
    uint32_t outSize = 0;

    int32_t ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(outSize, stream.dataSize);

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_VolumeMax, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_F32LE;
    format.rate = SAMPLE_RATE_48000;

    mixerProcessor.Init(format);

    std::vector<AudioMixStream> streams;
    AudioMixStream stream;
    std::vector<float> inputData(256);
    for (size_t i = 0; i < inputData.size(); ++i) {
        inputData[i] = 0.1f;
    }
    stream.data = inputData.data();
    stream.dataSize = inputData.size() * sizeof(float);
    stream.volume = 1.0f;
    streams.push_back(stream);

    std::vector<uint8_t> outputBuffer(1024);
    uint32_t outSize = 0;

    int32_t ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(outSize, stream.dataSize);

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_VolumeOutOfRange_Negative, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_F32LE;
    format.rate = SAMPLE_RATE_48000;

    mixerProcessor.Init(format);

    std::vector<AudioMixStream> streams;
    AudioMixStream stream;
    std::vector<float> inputData(256);
    for (size_t i = 0; i < inputData.size(); ++i) {
        inputData[i] = 0.1f;
    }
    stream.data = inputData.data();
    stream.dataSize = inputData.size() * sizeof(float);
    stream.volume = -0.1f;
    streams.push_back(stream);

    std::vector<uint8_t> outputBuffer(1024);
    uint32_t outSize = 0;

    int32_t ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_VolumeOutOfRange_ExceedMax, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_F32LE;
    format.rate = SAMPLE_RATE_48000;

    mixerProcessor.Init(format);

    std::vector<AudioMixStream> streams;
    AudioMixStream stream;
    std::vector<float> inputData(256);
    for (size_t i = 0; i < inputData.size(); ++i) {
        inputData[i] = 0.1f;
    }
    stream.data = inputData.data();
    stream.dataSize = inputData.size() * sizeof(float);
    stream.volume = 1.1f;
    streams.push_back(stream);

    std::vector<uint8_t> outputBuffer(1024);
    uint32_t outSize = 0;

    int32_t ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_ChunkSizeChanged, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_F32LE;
    format.rate = SAMPLE_RATE_48000;

    mixerProcessor.Init(format);

    std::vector<AudioMixStream> streams;
    AudioMixStream stream;
    std::vector<float> inputData(256);
    for (size_t i = 0; i < inputData.size(); ++i) {
        inputData[i] = 0.1f;
    }
    stream.data = inputData.data();
    stream.dataSize = inputData.size() * sizeof(float);
    streams.push_back(stream);

    std::vector<uint8_t> outputBuffer(1024);
    uint32_t outSize = 0;

    // First process with 256 samples
    int32_t ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, SUCCESS);

    // Second process with different size (128 samples)
    std::vector<float> inputData2(128);
    for (size_t i = 0; i < inputData2.size(); ++i) {
        inputData2[i] = 0.1f;
    }
    stream.data = inputData2.data();
    stream.dataSize = inputData2.size() * sizeof(float);
    streams[0] = stream;

    ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorInit_EnableLimiterFalse, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_F32LE;
    format.rate = SAMPLE_RATE_48000;

    int32_t ret = mixerProcessor.Init(format, false);
    EXPECT_EQ(ret, SUCCESS);
    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_EnableLimiterFalse_OddFrameCount, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_F32LE;
    format.rate = SAMPLE_RATE_48000;

    mixerProcessor.Init(format, false);

    std::vector<AudioMixStream> streams;
    AudioMixStream stream;
    // 480 samples (odd number) - should pass when enableLimiter=false
    std::vector<float> inputData(480);
    for (size_t i = 0; i < inputData.size(); ++i) {
        inputData[i] = 0.1f;
    }
    stream.data = inputData.data();
    stream.dataSize = inputData.size() * sizeof(float);
    streams.push_back(stream);

    std::vector<uint8_t> outputBuffer(inputData.size() * sizeof(float));
    uint32_t outSize = 0;

    int32_t ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(outSize, stream.dataSize);

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_EnableLimiterTrue_OddFrameCount, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_F32LE;
    format.rate = SAMPLE_RATE_48000;

    mixerProcessor.Init(format, true);

    std::vector<AudioMixStream> streams;
    AudioMixStream stream;
    // 255 samples (odd number) - should fail when enableLimiter=true
    std::vector<float> inputData(255);
    for (size_t i = 0; i < inputData.size(); ++i) {
        inputData[i] = 0.1f;
    }
    stream.data = inputData.data();
    stream.dataSize = inputData.size() * sizeof(float);
    streams.push_back(stream);

    std::vector<uint8_t> outputBuffer(1024);
    uint32_t outSize = 0;

    int32_t ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_EnableLimiterFalse_ChunkSizeChanged, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_F32LE;
    format.rate = SAMPLE_RATE_48000;

    mixerProcessor.Init(format, false);

    std::vector<AudioMixStream> streams;
    AudioMixStream stream;
    std::vector<float> inputData(256);
    for (size_t i = 0; i < inputData.size(); ++i) {
        inputData[i] = 0.1f;
    }
    stream.data = inputData.data();
    stream.dataSize = inputData.size() * sizeof(float);
    streams.push_back(stream);

    std::vector<uint8_t> outputBuffer(1024);
    uint32_t outSize = 0;

    // First process with 256 samples
    int32_t ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, SUCCESS);

    // Second process with different size (128 samples) - should pass when enableLimiter=false
    std::vector<float> inputData2(128);
    for (size_t i = 0; i < inputData2.size(); ++i) {
        inputData2[i] = 0.1f;
    }
    stream.data = inputData2.data();
    stream.dataSize = inputData2.size() * sizeof(float);
    streams[0] = stream;

    ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, SUCCESS);

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_EnableLimiterFalse_U8Format, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_U8;
    format.rate = SAMPLE_RATE_48000;

    mixerProcessor.Init(format, false);

    std::vector<AudioMixStream> streams;
    AudioMixStream stream;
    std::vector<uint8_t> inputData(256);
    for (size_t i = 0; i < inputData.size(); ++i) {
        inputData[i] = 128;
    }
    stream.data = inputData.data();
    stream.dataSize = inputData.size() * sizeof(uint8_t);
    streams.push_back(stream);

    std::vector<uint8_t> outputBuffer(1024);
    uint32_t outSize = 0;

    int32_t ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(outSize, stream.dataSize);

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_EnableLimiterFalse_S16LEFormat, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_S16LE;
    format.rate = SAMPLE_RATE_48000;

    mixerProcessor.Init(format, false);

    std::vector<AudioMixStream> streams;
    AudioMixStream stream;
    std::vector<int16_t> inputData(256);
    for (size_t i = 0; i < inputData.size(); ++i) {
        inputData[i] = 1000;
    }
    stream.data = inputData.data();
    stream.dataSize = inputData.size() * sizeof(int16_t);
    streams.push_back(stream);

    std::vector<uint8_t> outputBuffer(1024);
    uint32_t outSize = 0;

    int32_t ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(outSize, stream.dataSize);

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_EnableLimiterFalse_S24LEFormat, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_S24LE;
    format.rate = SAMPLE_RATE_48000;

    mixerProcessor.Init(format, false);

    std::vector<AudioMixStream> streams;
    AudioMixStream stream;
    std::vector<int32_t> inputData(256);
    for (size_t i = 0; i < inputData.size(); ++i) {
        inputData[i] = 1000;
    }
    stream.data = inputData.data();
    stream.dataSize = inputData.size() * 3; // S24LE uses 3 bytes per sample
    streams.push_back(stream);

    std::vector<uint8_t> outputBuffer(1024);
    uint32_t outSize = 0;

    int32_t ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(outSize, stream.dataSize);

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_EnableLimiterFalse_S32LEFormat, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_S32LE;
    format.rate = SAMPLE_RATE_48000;

    mixerProcessor.Init(format, false);

    std::vector<AudioMixStream> streams;
    AudioMixStream stream;
    std::vector<int32_t> inputData(256);
    for (size_t i = 0; i < inputData.size(); ++i) {
        inputData[i] = 1000;
    }
    stream.data = inputData.data();
    stream.dataSize = inputData.size() * sizeof(int32_t);
    streams.push_back(stream);

    std::vector<uint8_t> outputBuffer(1024);
    uint32_t outSize = 0;

    int32_t ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(outSize, stream.dataSize);

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_EnableLimiterFalse_F32LEFormat, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_F32LE;
    format.rate = SAMPLE_RATE_48000;

    mixerProcessor.Init(format, false);

    std::vector<AudioMixStream> streams;
    AudioMixStream stream;
    std::vector<float> inputData(256);
    for (size_t i = 0; i < inputData.size(); ++i) {
        inputData[i] = 0.1f;
    }
    stream.data = inputData.data();
    stream.dataSize = inputData.size() * sizeof(float);
    streams.push_back(stream);

    std::vector<uint8_t> outputBuffer(1024);
    uint32_t outSize = 0;

    int32_t ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(outSize, stream.dataSize);

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_EnableLimiterFalse_MultipleStreams, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_F32LE;
    format.rate = SAMPLE_RATE_48000;

    mixerProcessor.Init(format, false);

    std::vector<AudioMixStream> streams;
    // 48 input audio samples
    const size_t frameCount = 48;
    const float value = 0.1f;
    for (int i = 0; i < 3; ++i) {
        AudioMixStream stream;
        std::vector<float> inputData(frameCount * STEREO, value);

        stream.data = inputData.data();
        stream.dataSize = inputData.size() * sizeof(float);
        streams.push_back(stream);
    }

    std::vector<uint8_t> outputBuffer(frameCount * STEREO * sizeof(float));
    uint32_t outSize = 0;
    int32_t ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(outSize, streams[0].dataSize);

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_EnableLimiterFalse_MonoWithChannelConvert, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_MONO;
    format.audioChannelInfo.numChannels = MONO;
    format.format = SAMPLE_F32LE;
    format.rate = SAMPLE_RATE_48000;

    mixerProcessor.Init(format, false);

    std::vector<AudioMixStream> streams;
    AudioMixStream stream;
    std::vector<float> inputData(128);
    for (size_t i = 0; i < inputData.size(); ++i) {
        inputData[i] = 0.1f;
    }
    stream.data = inputData.data();
    stream.dataSize = inputData.size() * sizeof(float);
    streams.push_back(stream);

    std::vector<uint8_t> outputBuffer(1024);
    uint32_t outSize = 0;

    int32_t ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(outSize, stream.dataSize);

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_MonoS16LEWithBothConverts, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_MONO;
    format.audioChannelInfo.numChannels = MONO;
    format.format = SAMPLE_S16LE;
    format.rate = SAMPLE_RATE_48000;

    mixerProcessor.Init(format, false);

    std::vector<AudioMixStream> streams;
    AudioMixStream stream;
    std::vector<int16_t> inputData(128);
    for (size_t i = 0; i < inputData.size(); ++i) {
        inputData[i] = 1000;
    }
    stream.data = inputData.data();
    stream.dataSize = inputData.size() * sizeof(int16_t);
    streams.push_back(stream);

    std::vector<uint8_t> outputBuffer(1024);
    uint32_t outSize = 0;

    int32_t ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(outSize, stream.dataSize);

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_EnableLimiterTrue_StereoF32LE, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_F32LE;
    format.rate = SAMPLE_RATE_48000;

    mixerProcessor.Init(format, true);

    std::vector<AudioMixStream> streams;
    AudioMixStream stream;
    std::vector<float> inputData(256);
    for (size_t i = 0; i < inputData.size(); ++i) {
        inputData[i] = 0.1f;
    }
    stream.data = inputData.data();
    stream.dataSize = inputData.size() * sizeof(float);
    streams.push_back(stream);

    std::vector<uint8_t> outputBuffer(1024);
    uint32_t outSize = 0;

    int32_t ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(outSize, stream.dataSize);

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_EnableLimiterTrue_MonoWithChannelConvert, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_MONO;
    format.audioChannelInfo.numChannels = MONO;
    format.format = SAMPLE_F32LE;
    format.rate = SAMPLE_RATE_48000;

    mixerProcessor.Init(format, true);

    std::vector<AudioMixStream> streams;
    AudioMixStream stream;
    std::vector<float> inputData(128);
    for (size_t i = 0; i < inputData.size(); ++i) {
        inputData[i] = 0.1f;
    }
    stream.data = inputData.data();
    stream.dataSize = inputData.size() * sizeof(float);
    streams.push_back(stream);

    std::vector<uint8_t> outputBuffer(1024);
    uint32_t outSize = 0;

    int32_t ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(outSize, stream.dataSize);

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_EnableLimiterTrue_S16LEWithFormatConvert, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_S16LE;
    format.rate = SAMPLE_RATE_48000;

    mixerProcessor.Init(format, true);

    std::vector<AudioMixStream> streams;
    AudioMixStream stream;
    std::vector<int16_t> inputData(256);
    for (size_t i = 0; i < inputData.size(); ++i) {
        inputData[i] = 1000;
    }
    stream.data = inputData.data();
    stream.dataSize = inputData.size() * sizeof(int16_t);
    streams.push_back(stream);

    std::vector<uint8_t> outputBuffer(1024);
    uint32_t outSize = 0;

    int32_t ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(outSize, stream.dataSize);

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_EnableLimiterTrue_MultipleStreams, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_F32LE;
    format.rate = SAMPLE_RATE_48000;

    mixerProcessor.Init(format, true);

    std::vector<AudioMixStream> streams;
    // 48 input audio samples
    const size_t frameCount = 48;
    const float value = 0.1f;
    for (int i = 0; i < 3; ++i) {
        AudioMixStream stream;
        std::vector<float> inputData(frameCount, value);

        stream.data = inputData.data();
        stream.dataSize = inputData.size() * sizeof(float);
        streams.push_back(stream);
    }

    std::vector<uint8_t> outputBuffer(frameCount * sizeof(float));
    uint32_t outSize = 0;
    int32_t ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(outSize, streams[0].dataSize);

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_EnableLimiterTrue_5Point1Channel, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_5POINT1;
    format.audioChannelInfo.numChannels = CHANNEL_6;
    format.format = SAMPLE_F32LE;
    format.rate = SAMPLE_RATE_48000;

    int32_t ret = mixerProcessor.Init(format, true);
    EXPECT_EQ(ret, SUCCESS);

    std::vector<AudioMixStream> streams;
    AudioMixStream stream;
    std::vector<float> inputData(256 * 3);
    for (size_t i = 0; i < inputData.size(); ++i) {
        inputData[i] = 0.1f;
    }
    stream.data = inputData.data();
    stream.dataSize = inputData.size() * sizeof(float);
    streams.push_back(stream);

    std::vector<uint8_t> outputBuffer(stream.dataSize);
    uint32_t outSize = 0;

    ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(outSize, stream.dataSize);

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_EnableLimiterTrue_7Point1Channel, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_7POINT1;
    format.audioChannelInfo.numChannels = CHANNEL_8;
    format.format = SAMPLE_F32LE;
    format.rate = SAMPLE_RATE_48000;

    int32_t ret = mixerProcessor.Init(format, true);
    EXPECT_EQ(ret, SUCCESS);

    std::vector<AudioMixStream> streams;
    AudioMixStream stream;
    std::vector<float> inputData(256 * 4);
    for (size_t i = 0; i < inputData.size(); ++i) {
        inputData[i] = 0.1f;
    }
    stream.data = inputData.data();
    stream.dataSize = inputData.size() * sizeof(float);
    streams.push_back(stream);

    std::vector<uint8_t> outputBuffer(stream.dataSize);
    uint32_t outSize = 0;

    ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(outSize, stream.dataSize);

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_EnableLimiterTrue_S24LEWithFormatConvert, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_S24LE;
    format.rate = SAMPLE_RATE_48000;

    mixerProcessor.Init(format, true);

    std::vector<AudioMixStream> streams;
    AudioMixStream stream;
    std::vector<int32_t> inputData(256);
    for (size_t i = 0; i < inputData.size(); ++i) {
        inputData[i] = 1000;
    }
    stream.data = inputData.data();
    stream.dataSize = inputData.size() * 3; // S24LE uses 3 bytes per sample
    streams.push_back(stream);

    std::vector<uint8_t> outputBuffer(1024);
    uint32_t outSize = 0;

    int32_t ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(outSize, stream.dataSize);

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_EnableLimiterTrue_S32LEWithFormatConvert, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_S32LE;
    format.rate = SAMPLE_RATE_48000;

    mixerProcessor.Init(format, true);

    std::vector<AudioMixStream> streams;
    AudioMixStream stream;
    std::vector<int32_t> inputData(256);
    for (size_t i = 0; i < inputData.size(); ++i) {
        inputData[i] = 1000;
    }
    stream.data = inputData.data();
    stream.dataSize = inputData.size() * sizeof(int32_t);
    streams.push_back(stream);

    std::vector<uint8_t> outputBuffer(1024);
    uint32_t outSize = 0;

    int32_t ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(outSize, stream.dataSize);

    mixerProcessor.Destroy();
}

HWTEST_F(AudioSuiteMixerProcessorTest, MixerProcessorProcess_EnableLimiterTrue_U8WithFormatConvert, TestSize.Level0)
{
    MixerProcessorNode mixerProcessor;
    AudioFormat format;
    format.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    format.audioChannelInfo.numChannels = STEREO;
    format.format = SAMPLE_U8;
    format.rate = SAMPLE_RATE_48000;

    mixerProcessor.Init(format, true);

    std::vector<AudioMixStream> streams;
    AudioMixStream stream;
    std::vector<uint8_t> inputData(256);
    for (size_t i = 0; i < inputData.size(); ++i) {
        inputData[i] = 128;
    }
    stream.data = inputData.data();
    stream.dataSize = inputData.size() * sizeof(uint8_t);
    streams.push_back(stream);

    std::vector<uint8_t> outputBuffer(1024);
    uint32_t outSize = 0;

    int32_t ret = mixerProcessor.Process(streams, outputBuffer.data(), outputBuffer.size(), &outSize);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(outSize, stream.dataSize);

    mixerProcessor.Destroy();
}

}  // namespace