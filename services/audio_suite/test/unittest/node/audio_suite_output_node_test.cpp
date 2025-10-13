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
#include <string>
#include <thread>
#include <cstdio>
#include <fstream>
#include <vector>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <algorithm>
#include <gtest/gtest.h>
#include "audio_errors.h"
#include "audio_suite_log.h"
#include "audio_suite_output_node.h"
#include "audio_suite_input_node.h"
#include "audio_suite_pcm_buffer.h"
#include "audio_suite_aiss_node.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

struct FormatConversionInfo {
    std::string inputFileName;
    std::string outputFileName;
    std::string compareFileName;
    AudioFormat inputFormat;
    AudioFormat outputFormat;
};

static std::string g_outputNodeTestDir = "/data/audiosuite/outputnode/";
static FormatConversionInfo g_info[] = {
    {"in_44100_2_f32le.wav", "out1.pcm", "compare_44100_2_s16le.pcm",
        {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_44100},
        {{CH_LAYOUT_STEREO, 2}, SAMPLE_S16LE, SAMPLE_RATE_44100}},
    {"in_192000_2_f32le.wav", "out2.pcm", "compare_8000_1_u8.pcm",
        {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_192000},
        {{CH_LAYOUT_MONO, 1}, SAMPLE_U8, SAMPLE_RATE_8000}},
    {"in_96000_2_f32le.wav", "out3.pcm", "compare_44100_1_s24le.pcm",
        {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_96000},
        {{CH_LAYOUT_MONO, 1}, SAMPLE_S24LE, SAMPLE_RATE_44100}},
    {"in_12000_1_f32le.wav", "out4.pcm", "compare_24000_2_s24le.pcm",
        {{CH_LAYOUT_MONO, 1}, SAMPLE_F32LE, SAMPLE_RATE_12000},
        {{CH_LAYOUT_STEREO, 2}, SAMPLE_S24LE, SAMPLE_RATE_24000}},
    {"in_48000_1_f32le.wav", "out5.pcm", "compare_44100_2_s16le_2.pcm",
        {{CH_LAYOUT_MONO, 1}, SAMPLE_F32LE, SAMPLE_RATE_48000},
        {{CH_LAYOUT_STEREO, 2}, SAMPLE_S16LE, SAMPLE_RATE_44100}},
    {"in_32000_2_f32le.wav", "out6.pcm", "compare_64000_2_u8.pcm",
        { {CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_32000},
        {{CH_LAYOUT_STEREO, 2}, SAMPLE_U8, SAMPLE_RATE_64000}},
    {"in_8000_2_f32le.wav", "out7.pcm", "compare_192000_1_u8.pcm",
        {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_8000},
        {{CH_LAYOUT_MONO, 1}, SAMPLE_U8, SAMPLE_RATE_192000}},
    {"in_48000_2_f32le.wav", "out8.pcm", "compare_44100_1_u8.pcm",
        {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_48000},
        {{CH_LAYOUT_MONO, 1}, SAMPLE_U8, SAMPLE_RATE_44100}},
    {"in_96000_2_f32le.wav", "out9.pcm", "compare_96000_1_u8.pcm",
        {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_96000},
        {{CH_LAYOUT_MONO, 1}, SAMPLE_U8, SAMPLE_RATE_96000}},
    {"in_176400_2_f32le.wav", "out10.pcm", "compare_88200_1_u8.pcm",
        {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_176400},
        {{CH_LAYOUT_MONO, 1}, SAMPLE_U8, SAMPLE_RATE_88200}},
};

const uint32_t AUDIO_DATA_SIZE = 1024;
const uint32_t HEADER_SIZE = 44;
class AudioSuiteOutputNodeTest : public testing::Test {
public:
    void SetUp() {};
    void TearDown() {};
};

static void OutputFormatConvert(
    AudioFormat outputFormat, std::string inputFileName, AudioFormat inputFormat, string outputFileName)
{
    AudioOutputNode outputNode(outputFormat);
    outputNode.Init();

    std::ifstream inputFile(inputFileName, std::ios::binary | std::ios::ate);
    if (!inputFile.is_open()) {
        return;
    }

    std::ofstream outputFile(outputFileName, std::ios::binary);
    if (!outputFile.is_open()) {
        inputFile.close();
        return;
    }

    outputNode.SetInDataFormat(inputFormat.audioChannelInfo.numChannels,
        inputFormat.audioChannelInfo.channelLayout, inputFormat.format, inputFormat.rate);
    size_t inputLen = (inputFormat.rate * inputFormat.audioChannelInfo.numChannels *
        AudioSuiteUtil::GetSampleSize(inputFormat.format) * 20) / 1000;
    size_t outputLen = (outputFormat.rate * outputFormat.audioChannelInfo.numChannels *
        AudioSuiteUtil::GetSampleSize(outputFormat.format) * 20) / 1000;
    std::vector<uint8_t> inputData;
    std::vector<uint8_t> outputData;

    inputFile.seekg(HEADER_SIZE, std::ios::beg);
    bool exitFlag = true;
    while (exitFlag) {
        inputData.resize(inputLen, 0);
        outputData.resize(outputLen, 0);

        inputFile.read(reinterpret_cast<char *>(inputData.data()), inputLen);
        if (inputFile.eof()) {
            exitFlag = false;
            break;
        }

        outputNode.FormatConversion(reinterpret_cast<float *>(inputData.data()), inputData.size(),
            outputData.data(), outputData.size());
        outputFile.write(reinterpret_cast<char *>(outputData.data()), outputData.size());
    }

    inputFile.close();
    outputFile.close();
}

HWTEST_F(AudioSuiteOutputNodeTest, FormatConversion_001, TestSize.Level0)
{
    for (size_t idx = 0; idx < (sizeof(g_info) / sizeof(g_info[0])); idx++) {
        AUDIO_INFO_LOG("start Convert file %{public}s", g_info[idx].inputFileName.c_str());
        OutputFormatConvert(g_info[idx].outputFormat, g_outputNodeTestDir + g_info[idx].inputFileName,
            g_info[idx].inputFormat, g_outputNodeTestDir + g_info[idx].outputFileName);

        std::ifstream outFile(g_outputNodeTestDir + g_info[idx].outputFileName, std::ios::binary);
        std::ifstream baseFile(g_outputNodeTestDir + g_info[idx].compareFileName, std::ios::binary);
        ASSERT_TRUE(outFile.is_open());
        ASSERT_TRUE(baseFile.is_open());

        std::vector<char> out_data;
        std::vector<char> base_data;

        outFile.seekg(0, std::ios::end);
        out_data.resize(outFile.tellg());
        outFile.seekg(0, std::ios::beg);
        outFile.read(out_data.data(), out_data.size());

        baseFile.seekg(0, std::ios::end);
        base_data.resize(baseFile.tellg());
        baseFile.seekg(0, std::ios::beg);
        baseFile.read(base_data.data(), base_data.size());

        outFile.close();
        baseFile.close();

        AUDIO_INFO_LOG("out_data.size: %{public}zu base_data.size: %{public}zu", out_data.size(), base_data.size());
        EXPECT_EQ(out_data, base_data);
    }
}

HWTEST_F(AudioSuiteOutputNodeTest, AudioSuiteOutputNodeCreateTest, TestSize.Level0)
{
    AudioFormat format;
    std::shared_ptr<AudioOutputNode> outputNode = std::make_shared<AudioOutputNode>(format);
    EXPECT_NE(outputNode, nullptr);

    auto ret = outputNode->Flush();
    EXPECT_EQ(ret, 0);

    outputNode->DeInit();
    EXPECT_EQ(outputNode->inputStream_.outputPorts_.size(), 0);
}

HWTEST_F(AudioSuiteOutputNodeTest, Connection_001, TestSize.Level0)
{
    AudioFormat format;
    std::shared_ptr<AudioOutputNode> outputNode = std::make_shared<AudioOutputNode>(format);
    EXPECT_NE(outputNode, nullptr);

    std::shared_ptr<AudioInputNode> inputNode = std::make_shared<AudioInputNode>(format);
    EXPECT_NE(inputNode, nullptr);
    inputNode->Init();
    AudioNodePortType type = AUDIO_NODE_DEFAULT_OUTPORT_TYPE;
    auto ret = outputNode->Connect(inputNode, type);
    EXPECT_EQ(ret, SUCCESS);

    ret = outputNode->DisConnect(inputNode);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuiteOutputNodeTest, Connection_002, TestSize.Level0)
{
    AudioFormat format;
    std::shared_ptr<AudioOutputNode> outputNode = std::make_shared<AudioOutputNode>(format);
    EXPECT_NE(outputNode, nullptr);

    std::shared_ptr<AudioSuiteAissNode> node = std::make_shared<AudioSuiteAissNode>();
    EXPECT_NE(node, nullptr);
    auto ret = outputNode->Connect(node, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, SUCCESS);

    ret = outputNode->DisConnect(node);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuiteOutputNodeTest, Connection_003, TestSize.Level0)
{
    AudioFormat format;
    std::shared_ptr<AudioOutputNode> outputNode = std::make_shared<AudioOutputNode>(format);
    EXPECT_NE(outputNode, nullptr);

    auto ret = outputNode->Connect(nullptr, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST_F(AudioSuiteOutputNodeTest, CacheBuffer_001, TestSize.Level0)
{
    AudioFormat outformat = {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_44100};
    std::shared_ptr<AudioOutputNode> outputNode = std::make_shared<AudioOutputNode>(outformat);
    EXPECT_NE(outputNode, nullptr);
    outputNode->bufferUsedOffset_ = outputNode->cacheBuffer_[0].size() + 1;

    EXPECT_EQ(outputNode->GetCacheBufferDataLen(), 0);
}

HWTEST_F(AudioSuiteOutputNodeTest, ParamCheck_001, TestSize.Level0)
{
    AudioFormat outformat = {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_44100};
    std::shared_ptr<AudioOutputNode> outputNode = std::make_shared<AudioOutputNode>(outformat);
    EXPECT_NE(outputNode, nullptr);

    bool finished = false;
    int32_t writeDataSize = 0;
    uint8_t *audioDataArray[] = { nullptr };
    int32_t ret = outputNode->DoProcessParamCheck(nullptr, 0, 0, nullptr, nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    ret = outputNode->DoProcessParamCheck(audioDataArray, 0, 0, nullptr, nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    ret = outputNode->DoProcessParamCheck(audioDataArray, 0, 0, &writeDataSize, nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    ret = outputNode->DoProcessParamCheck(audioDataArray, 0, 0, &writeDataSize, &finished);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    ret = outputNode->DoProcessParamCheck(audioDataArray, 2, 0, &writeDataSize, &finished);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    ret = outputNode->DoProcessParamCheck(audioDataArray, 1, 0, &writeDataSize, &finished);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    std::vector<uint8_t> audioData(AUDIO_DATA_SIZE);
    audioDataArray[0] = audioData.data();
    ret = outputNode->DoProcessParamCheck(audioDataArray, 1, 0, &writeDataSize, &finished);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    ret = outputNode->DoProcessParamCheck(audioDataArray, 1, AUDIO_DATA_SIZE, &writeDataSize, &finished);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuiteOutputNodeTest, DoProcess_001, TestSize.Level0)
{
    AudioFormat format;
    std::shared_ptr<AudioOutputNode> outputNode = std::make_shared<AudioOutputNode>(format);
    EXPECT_NE(outputNode, nullptr);

    auto ret = outputNode->DoProcess();
    EXPECT_EQ(ret, ERROR);
}

HWTEST_F(AudioSuiteOutputNodeTest, DoProcess_002, TestSize.Level0)
{
    AudioFormat outformat = {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_44100};
    std::shared_ptr<AudioOutputNode> outputNode = std::make_shared<AudioOutputNode>(outformat);
    EXPECT_NE(outputNode, nullptr);
    outputNode->Init();

    AudioFormat informat = {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_44100};
    std::shared_ptr<AudioInputNode> inputNode = std::make_shared<AudioInputNode>(informat);
    EXPECT_NE(inputNode, nullptr);
    inputNode->Init();
    AudioNodePortType type = AUDIO_NODE_DEFAULT_OUTPORT_TYPE;
    std::unique_ptr<AudioSuitePcmBuffer> data = std::make_unique<AudioSuitePcmBuffer>(
        SAMPLE_RATE_44100, 2, AudioChannelLayout::CH_LAYOUT_STEREO);
    inputNode->GetOutputPort(type).get()->outputData_.push_back(data.get());
    outputNode->Connect(inputNode, type);
    auto ret = outputNode->DoProcess();
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuiteOutputNodeTest, DoProcess_003, TestSize.Level0)
{
    AudioFormat outformat = {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_44100};
    std::shared_ptr<AudioOutputNode> outputNode = std::make_shared<AudioOutputNode>(outformat);
    EXPECT_NE(outputNode, nullptr);
    outputNode->Init();

    AudioFormat informat = {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_48000};
    std::shared_ptr<AudioInputNode> inputNode = std::make_shared<AudioInputNode>(informat);
    EXPECT_NE(inputNode, nullptr);
    inputNode->Init();
    AudioNodePortType type = AUDIO_NODE_DEFAULT_OUTPORT_TYPE;
    std::unique_ptr<AudioSuitePcmBuffer> data = std::make_unique<AudioSuitePcmBuffer>(
        SAMPLE_RATE_44100, 2, AudioChannelLayout::CH_LAYOUT_STEREO);
    inputNode->GetOutputPort(type).get()->outputData_.push_back(data.get());
    outputNode->Connect(inputNode, type);
    auto ret = outputNode->DoProcess();
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuiteOutputNodeTest, DoProcess_004, TestSize.Level0)
{
    AudioFormat outformat = {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_44100};
    std::shared_ptr<AudioOutputNode> outputNode = std::make_shared<AudioOutputNode>(outformat);
    EXPECT_NE(outputNode, nullptr);
    outputNode->Init();

    AudioFormat informat = {{CH_LAYOUT_MONO, 1}, SAMPLE_S16LE, SAMPLE_RATE_48000};
    std::shared_ptr<AudioInputNode> inputNode = std::make_shared<AudioInputNode>(informat);
    EXPECT_NE(inputNode, nullptr);
    inputNode->Init();
    AudioNodePortType type = AUDIO_NODE_DEFAULT_OUTPORT_TYPE;
    std::unique_ptr<AudioSuitePcmBuffer> data = std::make_unique<AudioSuitePcmBuffer>(
        SAMPLE_RATE_44100, 2, AudioChannelLayout::CH_LAYOUT_STEREO);
    inputNode->GetOutputPort(type).get()->outputData_.push_back(data.get());
    outputNode->Connect(inputNode, type);
    auto ret = outputNode->DoProcess();
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuiteOutputNodeTest, DoProcess_005, TestSize.Level0)
{
    AudioFormat format;
    std::shared_ptr<AudioOutputNode> outputNode = std::make_shared<AudioOutputNode>(format);
    EXPECT_NE(outputNode, nullptr);
    outputNode->Init();

    std::vector<uint8_t> audioData(AUDIO_DATA_SIZE);
    int32_t frameSize = 128;
    bool finished = false;
    int32_t writeDataSize = 0;
    outputNode->SetAudioNodeDataFinishedFlag(true);
    int32_t ret = outputNode->DoProcess(audioData.data(), frameSize, &writeDataSize, &finished);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

HWTEST_F(AudioSuiteOutputNodeTest, DoProcess_006, TestSize.Level0)
{
    AudioFormat format = {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_44100};
    std::shared_ptr<AudioOutputNode> outputNode = std::make_shared<AudioOutputNode>(format);
    EXPECT_NE(outputNode, nullptr);
    outputNode->Init();
    outputNode->bufferUsedOffset_ = 0;

    std::vector<uint8_t> audioData(AUDIO_DATA_SIZE);
    bool finished = false;
    int32_t writeDataSize = 0;
    int32_t ret = outputNode->DoProcess(audioData.data(), AUDIO_DATA_SIZE, &writeDataSize, &finished);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuiteOutputNodeTest, InstallTap_001, TestSize.Level0)
{
    AudioFormat format;
    std::shared_ptr<AudioOutputNode> outputNode = std::make_shared<AudioOutputNode>(format);
    EXPECT_NE(outputNode, nullptr);

    AudioNodePortType portType = AUDIO_NODE_DEFAULT_OUTPORT_TYPE;
    std::shared_ptr<SuiteNodeReadTapDataCallback> callback;
    auto ret = outputNode->InstallTap(portType, callback);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
}

HWTEST_F(AudioSuiteOutputNodeTest, RemoveTap_001, TestSize.Level0)
{
    AudioFormat format;
    std::shared_ptr<AudioOutputNode> outputNode = std::make_shared<AudioOutputNode>(format);
    EXPECT_NE(outputNode, nullptr);

    AudioNodePortType portType = AUDIO_NODE_DEFAULT_OUTPORT_TYPE;
    auto ret = outputNode->RemoveTap(portType);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
}

}
}
}  // namespace