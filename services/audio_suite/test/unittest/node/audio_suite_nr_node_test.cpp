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
#include <vector>
#include <memory>
#include <fstream>
#include <gtest/gtest.h>
#include "audio_errors.h"
#include "audio_suite_log.h"
#include "audio_suite_unittest_tools.h"
#include "audio_suite_nr_node.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;

namespace {
static std::string g_inputPcmFilePath = "/data/ainr_source_48000_2_F32LE.pcm";
static std::string g_targetPcmFilePath = "/data/ainr_dest_16000_1_F32LE.pcm";
static std::string g_outputPcmFilePath = "/data/ainr_ut_output_16000_1_F32LE.pcm";

class AudioSuiteNrNodeUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void){};
    static void TearDownTestCase(void){};
    void SetUp(void);
    void TearDown(void);
};

void AudioSuiteNrNodeUnitTest::SetUp(void)
{
    std::filesystem::remove(g_outputPcmFilePath);
}

void AudioSuiteNrNodeUnitTest::TearDown(void)
{}

HWTEST_F(AudioSuiteNrNodeUnitTest, TestNrNodeInitAndDeinit_001, TestSize.Level0)
{
    std::shared_ptr<AudioSuiteNrNode> nrNode = std::make_shared<AudioSuiteNrNode>();
    EXPECT_NE(nrNode, nullptr);

    EXPECT_EQ(nrNode->Init(), 0);
    EXPECT_EQ(nrNode->DeInit(), 0);
}

HWTEST_F(AudioSuiteNrNodeUnitTest, TestCopyPcmBuffer_001, TestSize.Level0)
{
    std::shared_ptr<AudioSuiteNrNode> nrNode = std::make_shared<AudioSuiteNrNode>();
    EXPECT_NE(nrNode, nullptr);

    AudioSuitePcmBuffer pcmBufferInput(SAMPLE_RATE_16000, MONO, CH_LAYOUT_MONO);
    AudioSuitePcmBuffer pcmBufferOutput(SAMPLE_RATE_16000, MONO, CH_LAYOUT_MONO);
    float *inputData = pcmBufferInput.GetPcmDataBuffer();
    float *outputData = pcmBufferOutput.GetPcmDataBuffer();
    uint32_t maxIndex = pcmBufferInput.GetFrameLen() - 1;  // 320 - 1
    inputData[0] = 0.1f;
    inputData[100] = -0.5f;
    inputData[maxIndex] = 0.99f;

    EXPECT_EQ(nrNode->CopyPcmBuffer(&pcmBufferInput, &pcmBufferOutput), 0);

    EXPECT_EQ(outputData[0], 0.1f);
    EXPECT_EQ(outputData[100], -0.5f);
    EXPECT_EQ(outputData[maxIndex], 0.99f);
}

HWTEST_F(AudioSuiteNrNodeUnitTest, TestDoChannelConvert_001, TestSize.Level0)
{
    std::shared_ptr<AudioSuiteNrNode> nrNode = std::make_shared<AudioSuiteNrNode>();
    EXPECT_NE(nrNode, nullptr);

    AudioSuitePcmBuffer pcmBufferInput(SAMPLE_RATE_16000, STEREO, CH_LAYOUT_STEREO);
    AudioSuitePcmBuffer pcmBufferOutput(SAMPLE_RATE_16000, MONO, CH_LAYOUT_MONO);
    float *inputData = pcmBufferInput.GetPcmDataBuffer();
    float *outputData = pcmBufferOutput.GetPcmDataBuffer();
    for (uint32_t i = 0; i < pcmBufferInput.GetFrameLen(); i++) {
        inputData[i] = 0.1f;
    }
    inputData[0] = 0.05f;
    inputData[1] = 0.05f;
    inputData[2] = -0.5f;
    inputData[3] = -0.5f;

    EXPECT_EQ(nrNode->DoChannelConvert(&pcmBufferInput, &pcmBufferOutput), 0);

    // 单声道采样点数据为左右声道合并后结果
    const float epsilon = 0.0001f;
    EXPECT_NEAR(outputData[0], 0.05f, epsilon);
    EXPECT_NEAR(outputData[1], -0.5f, epsilon);
    EXPECT_NEAR(outputData[319], 0.1f, epsilon);
}

HWTEST_F(AudioSuiteNrNodeUnitTest, TestDoResample_001, TestSize.Level0)
{
    std::shared_ptr<AudioSuiteNrNode> nrNode = std::make_shared<AudioSuiteNrNode>();
    EXPECT_NE(nrNode, nullptr);

    AudioSuitePcmBuffer pcmBufferInput(SAMPLE_RATE_48000, MONO, CH_LAYOUT_MONO);
    AudioSuitePcmBuffer pcmBufferOutput(SAMPLE_RATE_16000, MONO, CH_LAYOUT_MONO);
    float *inputData = pcmBufferInput.GetPcmDataBuffer();
    float *outputData = pcmBufferOutput.GetPcmDataBuffer();
    for (uint32_t i = 0; i < pcmBufferInput.GetFrameLen(); i++) {
        inputData[i] = 0.1f;
    }

    EXPECT_EQ(nrNode->DoResample(&pcmBufferInput, &pcmBufferOutput), 0);

    // 重采样后单个采样点数据不变，采样点个数变化。
    const float epsilon = 0.0001f;
    EXPECT_NEAR(outputData[0], 0.1f, epsilon);
    EXPECT_NEAR(outputData[159], 0.1f, epsilon);
    EXPECT_NEAR(outputData[319], 0.1f, epsilon);
}

HWTEST_F(AudioSuiteNrNodeUnitTest, TestConvertProcess_001, TestSize.Level0)
{
    std::shared_ptr<AudioSuiteNrNode> nrNode = std::make_shared<AudioSuiteNrNode>();
    EXPECT_NE(nrNode, nullptr);

    AudioSuitePcmBuffer pcmBufferInput(SAMPLE_RATE_48000, STEREO, CH_LAYOUT_STEREO);
    float *inputData = pcmBufferInput.GetPcmDataBuffer();
    for (uint32_t i = 0; i < pcmBufferInput.GetFrameLen(); i++) {
        inputData[i] = 0.1f;
    }

    EXPECT_EQ(nrNode->ConvertProcess(&pcmBufferInput), 0);

    float *outputData = nrNode->pcmBufferOutput_.GetPcmDataBuffer();
    const float epsilon = 0.0001f;
    EXPECT_NEAR(outputData[0], 0.1f, epsilon);
    EXPECT_NEAR(outputData[159], 0.1f, epsilon);
    EXPECT_NEAR(outputData[319], 0.1f, epsilon);
}

HWTEST_F(AudioSuiteNrNodeUnitTest, TestNrNodeSignalProcess_001, TestSize.Level0)
{
    std::shared_ptr<AudioSuiteNrNode> nrNode = std::make_shared<AudioSuiteNrNode>();
    EXPECT_NE(nrNode, nullptr);

    EXPECT_EQ(nrNode->Init(), 0);

    AudioSuitePcmBuffer pcmBufferInput(SAMPLE_RATE_48000, STEREO, CH_LAYOUT_STEREO);
    std::vector<AudioSuitePcmBuffer *> inputs(1);
    inputs[0] = &pcmBufferInput;

    size_t frameSizeInput = pcmBufferInput.GetFrameLen() * sizeof(float);
    size_t frameSizeOutput = nrNode->pcmBufferOutput_.GetFrameLen() * sizeof(float);
    float *inputData = pcmBufferInput.GetPcmDataBuffer();

    // 处理输入文件
    std::ifstream ifs(g_inputPcmFilePath, std::ios::binary);
    ifs.seekg(0, std::ios::end);
    size_t inputFileSize = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    // pcm文件长度可能不是帧长的整数倍，补0后再传给算法处理
    size_t zeroPaddingSize =
        (inputFileSize % frameSizeInput == 0) ? 0 : (frameSizeInput - inputFileSize % frameSizeInput);
    size_t fileBufferSize = inputFileSize + zeroPaddingSize;
    std::vector<float> inputfileBuffer(fileBufferSize / sizeof(float), 0.0f);  // 32 float PCM data
    ifs.read(reinterpret_cast<char *>(inputfileBuffer.data()), inputFileSize);
    ifs.close();

    std::vector<uint8_t> outputfileBuffer(fileBufferSize);
    uint8_t *readPtr = reinterpret_cast<uint8_t *>(inputfileBuffer.data());
    uint8_t *writePtr = outputfileBuffer.data();
    size_t outputFileSize = 0;
    for (int32_t i = 0; i + frameSizeInput <= fileBufferSize; i += frameSizeInput) {
        // 从inputfileBuffer拷贝一帧数据到pcmBufferInput
        memcpy_s(reinterpret_cast<char *>(inputData), frameSizeInput, readPtr, frameSizeInput);

        AudioSuitePcmBuffer *pcmBufferOutputPtr = nrNode->SignalProcess(inputs);

        // 处理后数据拷贝到outputfileBuffer
        uint8_t *outputData = reinterpret_cast<uint8_t *>(pcmBufferOutputPtr->GetPcmDataBuffer());
        memcpy_s(writePtr, frameSizeOutput, outputData, frameSizeOutput);

        readPtr += frameSizeInput;
        writePtr += frameSizeOutput;
        outputFileSize += frameSizeOutput;
    }

    // 输出pcm数据写入文件
    ASSERT_EQ(CreateOutputPcmFile(g_outputPcmFilePath), true);
    bool isWriteFileSucc = WritePcmFile(g_outputPcmFilePath, outputfileBuffer.data(), outputFileSize);
    ASSERT_EQ(isWriteFileSucc, true);

    // 和归档结果比对
    EXPECT_EQ(IsFilesEqual(g_outputPcmFilePath, g_targetPcmFilePath), true);

    EXPECT_EQ(nrNode->DeInit(), 0);
}

}  // namespace