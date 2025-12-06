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
class InputNodeRequestDataCallBack;
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
    {"in_48000_2_f32le.wav", "out11.pcm", "compare_11025_1_s16.pcm",
        {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_48000},
        {{CH_LAYOUT_MONO, 1}, SAMPLE_S16LE, SAMPLE_RATE_11025}},
    {"in_176400_2_f32le.wav", "out12.pcm", "compare_11025_1_u8.pcm",
        {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_176400},
        {{CH_LAYOUT_MONO, 1}, SAMPLE_U8, SAMPLE_RATE_11025}},
    {"in_96000_2_f32le.wav", "out13.pcm", "compare_11025_1_s24.pcm",
        {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_96000},
        {{CH_LAYOUT_MONO, 1}, SAMPLE_S24LE, SAMPLE_RATE_11025}},
};

static FormatConversionInfo g_inputInfo[] = {
    {"in_8000_1_u8.wav", "inout1.pcm", "compare_44100_2_s16le_3.pcm",
        {{CH_LAYOUT_MONO, 1}, SAMPLE_U8, SAMPLE_RATE_8000},
        {{CH_LAYOUT_STEREO, 2}, SAMPLE_S16LE, SAMPLE_RATE_44100}},
    {"in_48000_2_s16le.wav", "inout2.pcm", "compare_48000_2_s24le.pcm",
        {{CH_LAYOUT_STEREO, 2}, SAMPLE_S16LE, SAMPLE_RATE_48000},
        {{CH_LAYOUT_STEREO, 2}, SAMPLE_S24LE, SAMPLE_RATE_48000}},
    {"in_44100_2_s24le.wav", "inout3.pcm", "compare_192000_2_f32le.pcm",
        {{CH_LAYOUT_STEREO, 2}, SAMPLE_S24LE, SAMPLE_RATE_44100},
        {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_192000}},
    {"in_16000_1_s16le.wav", "inout4.pcm", "compare_48000_2_s16le.pcm",
        {{CH_LAYOUT_MONO, 1}, SAMPLE_S16LE, SAMPLE_RATE_16000},
        {{CH_LAYOUT_STEREO, 2}, SAMPLE_S16LE, SAMPLE_RATE_48000}},
    {"in_32000_1_u8.wav", "inout5.pcm", "compare_16000_1_s16le.pcm",
        {{CH_LAYOUT_MONO, 1}, SAMPLE_U8, SAMPLE_RATE_32000},
        {{CH_LAYOUT_MONO, 1}, SAMPLE_S16LE, SAMPLE_RATE_16000}},
    {"in_64000_2_s16le.wav", "inout7.pcm", "compare_32000_2_f32le.pcm",
        {{CH_LAYOUT_STEREO, 2}, SAMPLE_S16LE, SAMPLE_RATE_64000},
        {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_32000}},
    {"in_88200_2_u8.wav", "inout8.pcm", "compare_176400_2_s16le.pcm",
        {{CH_LAYOUT_STEREO, 2}, SAMPLE_U8, SAMPLE_RATE_88200},
        {{CH_LAYOUT_STEREO, 2}, SAMPLE_S16LE, SAMPLE_RATE_176400}},
    {"in_24000_1_s16le.wav", "inout9.pcm", "compare_12000_1_u8.pcm",
        {{CH_LAYOUT_MONO, 1}, SAMPLE_S16LE, SAMPLE_RATE_24000},
        {{CH_LAYOUT_MONO, 1}, SAMPLE_U8, SAMPLE_RATE_12000}},
    {"in_176400_2_s24le.wav", "inout10.pcm", "compare_88200_1_f32le.pcm",
        {{CH_LAYOUT_STEREO, 2}, SAMPLE_S24LE, SAMPLE_RATE_176400},
        {{CH_LAYOUT_MONO, 1}, SAMPLE_F32LE, SAMPLE_RATE_88200}},
    {"in_11025_1_f32le.wav", "inout11.pcm", "compare_22050_2_s24le.pcm",
        {{CH_LAYOUT_MONO, 1}, SAMPLE_F32LE, SAMPLE_RATE_11025},
        {{CH_LAYOUT_STEREO, 2}, SAMPLE_S24LE, SAMPLE_RATE_22050}},
    {"in_8000_2_s24le.wav", "inout12.pcm", "compare_192000_1_u8_2.pcm",
        {{CH_LAYOUT_STEREO, 2}, SAMPLE_S24LE, SAMPLE_RATE_8000},
        {{CH_LAYOUT_MONO, 1}, SAMPLE_U8, SAMPLE_RATE_192000}},
    {"in_192000_1_u8.wav", "inout13.pcm", "compare_8000_2_f32le.pcm",
        {{CH_LAYOUT_MONO, 1}, SAMPLE_U8, SAMPLE_RATE_192000},
        {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_8000}},
    {"in_44100_2_u8.wav", "inout14.pcm", "compare_48000_1_s24le.pcm",
        {{CH_LAYOUT_STEREO, 2}, SAMPLE_U8, SAMPLE_RATE_44100},
        {{CH_LAYOUT_MONO, 1}, SAMPLE_S24LE, SAMPLE_RATE_48000}},
    {"in_16000_2_s24le.wav", "inout15.pcm", "compare_96000_2_s16le.pcm",
        {{CH_LAYOUT_STEREO, 2}, SAMPLE_S24LE, SAMPLE_RATE_16000},
        {{CH_LAYOUT_STEREO, 2}, SAMPLE_S16LE, SAMPLE_RATE_96000}},
    {"in_96000_1_s16le.wav", "inout16.pcm", "compare_16000_1_f32le.pcm",
        {{CH_LAYOUT_MONO, 1}, SAMPLE_S16LE, SAMPLE_RATE_96000},
        {{CH_LAYOUT_MONO, 1}, SAMPLE_F32LE, SAMPLE_RATE_16000}},
    {"in_64000_1_s24le.wav", "inout18.pcm", "compare_32000_2_s16le.pcm",
        {{CH_LAYOUT_MONO, 1}, SAMPLE_S24LE, SAMPLE_RATE_64000},
        {{CH_LAYOUT_STEREO, 2}, SAMPLE_S16LE, SAMPLE_RATE_32000}},
    {"in_12000_2_s16le.wav", "inout19.pcm", "compare_24000_1_u8.pcm",
        {{CH_LAYOUT_STEREO, 2}, SAMPLE_S16LE, SAMPLE_RATE_12000},
        {{CH_LAYOUT_MONO, 1}, SAMPLE_U8, SAMPLE_RATE_24000}},
    {"in_192000_1_u8.wav", "inout20.pcm", "compare_8000_2_f32le_2.pcm",
        {{CH_LAYOUT_MONO, 1}, SAMPLE_U8, SAMPLE_RATE_192000},
        {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_8000}},
    {"in_44100_1_u8.wav", "inout21.pcm", "compare_48000_2_f32le.pcm",
        {{CH_LAYOUT_MONO, 1}, SAMPLE_U8, SAMPLE_RATE_44100},
        {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_48000}},
    {"in_16000_1_u8.wav", "inout22.pcm", "compare_16000_2_f32le.pcm",
        {{CH_LAYOUT_MONO, 1}, SAMPLE_U8, SAMPLE_RATE_16000},
        {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_16000}},
    {"in_32000_1_s24le.wav", "inout23.pcm", "compare_32000_2_s24le.pcm",
        {{CH_LAYOUT_MONO, 1}, SAMPLE_S24LE, SAMPLE_RATE_32000},
        {{CH_LAYOUT_STEREO, 2}, SAMPLE_S24LE, SAMPLE_RATE_32000}},
    {"in_22050_2_s16le.wav", "inout24.pcm", "compare_22050_1_s16le.pcm",
        {{CH_LAYOUT_STEREO, 2}, SAMPLE_S16LE, SAMPLE_RATE_22050},
        {{CH_LAYOUT_MONO, 1}, SAMPLE_S16LE, SAMPLE_RATE_22050}},
    {"in_88200_1_u8.wav", "inout25.pcm", "compare_176400_2_f32le.pcm",
        {{CH_LAYOUT_MONO, 1}, SAMPLE_U8, SAMPLE_RATE_88200},
        {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_176400}},
    {"in_12000_2_s24le.wav", "inout26.pcm", "compare_24000_1_s16le.pcm",
        {{CH_LAYOUT_STEREO, 2}, SAMPLE_S24LE, SAMPLE_RATE_12000},
        {{CH_LAYOUT_MONO, 1}, SAMPLE_S16LE, SAMPLE_RATE_24000}},
    {"in_24000_1_s16le.wav", "inout27.pcm", "compare_12000_2_s24le.pcm",
        {{CH_LAYOUT_MONO, 1}, SAMPLE_S16LE, SAMPLE_RATE_24000},
        {{CH_LAYOUT_STEREO, 2}, SAMPLE_S24LE, SAMPLE_RATE_12000}},
};

const uint32_t AUDIO_DATA_SIZE = 1024;
const uint32_t HEADER_SIZE = 44;
class AudioSuiteOutputNodeTest : public testing::Test {
public:
    void SetUp() {};
    void TearDown() {};
};

class SuiteInputNodeRequestDataCallBackTest : public AudioSuite::InputNodeRequestDataCallBack {
public:
    int32_t OnRequestDataCallBack(void *audioData, int32_t audioDataSize, bool* finished) override
    {
        if (audioData == nullptr || finished == nullptr) {
            return -1;
        }

        if (currentPos_ >= buffer_.size()) {
            *finished = true;
            return 0;
        }

        size_t copySize = std::min(static_cast<size_t>(audioDataSize), buffer_.size() - currentPos_);
        if (memcpy_s(audioData, audioDataSize, buffer_.data() + currentPos_, copySize) != 0) {
            return -1;
        }

        currentPos_ += copySize;
        *finished = (currentPos_ >= buffer_.size());
        return static_cast<int32_t>(copySize);
    }

    void SetData(const std::vector<uint8_t>& data)
    {
        buffer_ = data;
        currentPos_ = 0;
    }

private:
    size_t currentPos_ = 0;
    std::vector<uint8_t> buffer_;
};

static void CompareOutputWithReference(const std::string& outputFilePath, const std::string& compareFilePath)
{
    std::ifstream baseFile(compareFilePath, std::ios::binary);
    std::ifstream outFileStream(outputFilePath, std::ios::binary);
    ASSERT_TRUE(baseFile.is_open());
    ASSERT_TRUE(outFileStream.is_open());

    std::vector<char> out_data;
    std::vector<char> base_data;

    outFileStream.seekg(0, std::ios::end);
    out_data.resize(outFileStream.tellg());
    outFileStream.seekg(0, std::ios::beg);
    outFileStream.read(out_data.data(), out_data.size());

    baseFile.seekg(0, std::ios::end);
    base_data.resize(baseFile.tellg());
    baseFile.seekg(0, std::ios::beg);
    baseFile.read(base_data.data(), base_data.size());

    outFileStream.close();
    baseFile.close();

    AUDIO_INFO_LOG("out_data.size: %{public}zu base_data.size: %{public}zu", out_data.size(), base_data.size());
    EXPECT_EQ(out_data.size(), base_data.size());
}

static bool RunFormatConversionTest(const FormatConversionInfo& info,
    const std::string& inputFilePath, const std::string& outputFilePath)
{
    std::shared_ptr<AudioInputNode> inputNode = std::make_shared<AudioInputNode>(info.inputFormat);
    std::shared_ptr<AudioOutputNode> outputNode = std::make_shared<AudioOutputNode>(info.outputFormat);

    std::ifstream inputFile(inputFilePath, std::ios::binary | std::ios::ate);
    if (!inputFile.is_open()) {
        return false;
    }

    std::shared_ptr<SuiteInputNodeRequestDataCallBackTest> callback =
        std::make_shared<SuiteInputNodeRequestDataCallBackTest>();
    inputNode->SetRequestDataCallback(callback);

    inputNode->Init();
    outputNode->Init();

    auto ret = outputNode->Connect(inputNode);
    EXPECT_EQ(ret, SUCCESS);

    inputFile.seekg(HEADER_SIZE, std::ios::beg);
    std::vector<uint8_t> fileData((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
    inputFile.close();

    callback->SetData(fileData);

    std::vector<uint8_t> outputData;
    int32_t frameSize = 1024;
    int32_t writeSize = 0;
    bool finished = false;

    std::ofstream outFile(outputFilePath, std::ios::binary);
    do {
        outputData.resize(frameSize);
        ret = outputNode->DoProcess(outputData.data(), frameSize, &writeSize, &finished);
        EXPECT_EQ(ret, SUCCESS);
        if (ret != SUCCESS) {
            break;
        }

        if (writeSize > 0) {
            outFile.write(reinterpret_cast<char*>(outputData.data()), writeSize);
        }
        writeSize = 0;
    } while (!finished);
    outFile.close();

    ret = outputNode->DisConnect(inputNode);
    EXPECT_EQ(ret, SUCCESS);

    inputNode->DeInit();
    outputNode->DeInit();
    return true;
}

static void RunAllTestCases(const FormatConversionInfo* testCases, size_t count)
{
    for (size_t idx = 0; idx < count; idx++) {
        const FormatConversionInfo& info = testCases[idx];
        std::string inputFilePath = g_outputNodeTestDir + info.inputFileName;
        std::string outputFilePath = g_outputNodeTestDir + info.outputFileName;
        std::string compareFilePath = g_outputNodeTestDir + info.compareFileName;
        std::cout << testCases[idx].inputFileName << std::endl;
        std::cout << testCases[idx].outputFileName << std::endl;
        std::cout << testCases[idx].compareFileName << std::endl;

        if (!RunFormatConversionTest(info, inputFilePath, outputFilePath)) {
            continue;
        }

        CompareOutputWithReference(outputFilePath, compareFilePath);
    }
}

HWTEST_F(AudioSuiteOutputNodeTest, FormatConversion_002, TestSize.Level0)
{
    RunAllTestCases(g_info, sizeof(g_info) / sizeof(g_info[0]));
    RunAllTestCases(g_inputInfo, sizeof(g_inputInfo) / sizeof(g_inputInfo[0]));
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
    auto ret = outputNode->Connect(inputNode);
    EXPECT_EQ(ret, SUCCESS);

    ret = outputNode->DisConnect(inputNode);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuiteOutputNodeTest, Connection_003, TestSize.Level0)
{
    AudioFormat format;
    std::shared_ptr<AudioOutputNode> outputNode = std::make_shared<AudioOutputNode>(format);
    EXPECT_NE(outputNode, nullptr);

    auto ret = outputNode->Connect(nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
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
    std::unique_ptr<AudioSuitePcmBuffer> data = std::make_unique<AudioSuitePcmBuffer>(
        PcmBufferFormat(SAMPLE_RATE_44100, 2, AudioChannelLayout::CH_LAYOUT_STEREO, SAMPLE_F32LE));
    inputNode->GetOutputPort().get()->outputData_.push_back(data.get());
    outputNode->Connect(inputNode);
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
    std::unique_ptr<AudioSuitePcmBuffer> data = std::make_unique<AudioSuitePcmBuffer>(
        PcmBufferFormat(SAMPLE_RATE_44100, 2, AudioChannelLayout::CH_LAYOUT_STEREO, SAMPLE_F32LE));
    inputNode->GetOutputPort().get()->outputData_.push_back(data.get());
    outputNode->Connect(inputNode);
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
    std::unique_ptr<AudioSuitePcmBuffer> data = std::make_unique<AudioSuitePcmBuffer>(
        PcmBufferFormat(SAMPLE_RATE_44100, 2, AudioChannelLayout::CH_LAYOUT_STEREO, SAMPLE_F32LE));
    inputNode->GetOutputPort().get()->outputData_.push_back(data.get());
    outputNode->Connect(inputNode);
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

}
}
}  // namespace