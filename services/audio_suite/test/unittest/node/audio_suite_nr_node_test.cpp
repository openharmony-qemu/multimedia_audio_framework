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
static std::string g_inputPcmFilePath001 = "/data/audiosuite/nr/ainr_input_48000_2_F32LE.pcm";
static std::string g_inputPcmFilePath002 = "/data/audiosuite/nr/ainr_input_48000_1_F32LE.pcm";
static std::string g_inputPcmFilePath003 = "/data/audiosuite/nr/ainr_input_16000_2_F32LE.pcm";
static std::string g_inputPcmFilePath004 = "/data/audiosuite/nr/ainr_input_16000_1_F32LE.pcm";
            
static std::string g_targetPcmFilePath001 = "/data/audiosuite/nr/ainr_target_48000_2_to_16000_1_F32LE.pcm";
static std::string g_targetPcmFilePath002 = "/data/audiosuite/nr/ainr_target_48000_1_to_16000_1_F32LE.pcm";
static std::string g_targetPcmFilePath003 = "/data/audiosuite/nr/ainr_target_16000_2_to_16000_1_F32LE.pcm";
static std::string g_targetPcmFilePath004 = "/data/audiosuite/nr/ainr_target_16000_1_to_16000_1_F32LE.pcm";

static std::string g_outputPcmFilePath001 = "/data/audiosuite/nr/ainr_output_48000_2_to_16000_1_F32LE.pcm";
static std::string g_outputPcmFilePath002 = "/data/audiosuite/nr/ainr_output_48000_1_to_16000_1_F32LE.pcm";
static std::string g_outputPcmFilePath003 = "/data/audiosuite/nr/ainr_output_16000_2_to_16000_1_F32LE.pcm";
static std::string g_outputPcmFilePath004 = "/data/audiosuite/nr/ainr_output_16000_1_to_16000_1_F32LE.pcm";

class AudioSuiteNrNodeUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        std::filesystem::remove(g_outputPcmFilePath001);
        std::filesystem::remove(g_outputPcmFilePath002);
        std::filesystem::remove(g_outputPcmFilePath003);
        std::filesystem::remove(g_outputPcmFilePath004);
    }
    static void TearDownTestCase(void){};
    void SetUp(void);
    void TearDown(void);

    int32_t RunSignalProcessTest(std::shared_ptr<AudioSuiteNrNode> node, const std::string &inputFile,
        const std::string &outputFile, const std::string &targetFile, const std::vector<AudioSuitePcmBuffer *> &inputs);

    std::shared_ptr<AudioSuiteNrNode> nrNode_;
    std::vector<AudioSuitePcmBuffer *> inputs_;
};

void AudioSuiteNrNodeUnitTest::SetUp(void)
{
    nrNode_ = std::make_shared<AudioSuiteNrNode>();
    inputs_.resize(1);
}

void AudioSuiteNrNodeUnitTest::TearDown(void)
{
    nrNode_.reset();
}


int32_t AudioSuiteNrNodeUnitTest::RunSignalProcessTest(std::shared_ptr<AudioSuiteNrNode> node,
    const std::string &inputFile, const std::string &outputFile, const std::string &targetFile,
    const std::vector<AudioSuitePcmBuffer *> &inputs)
{
    CHECK_AND_RETURN_RET(node->Init() == SUCCESS, ERROR);

    size_t frameSizeInput = inputs[0]->GetFrameLen() * sizeof(float);
    size_t frameSizeOutput = node->pcmBufferOutput_.GetFrameLen() * sizeof(float);
    float *inputData = inputs[0]->GetPcmDataBuffer();

    // Read input file
    std::ifstream ifs(inputFile, std::ios::binary);
    CHECK_AND_RETURN_RET(ifs.is_open(), ERROR);
    ifs.seekg(0, std::ios::end);
    size_t inputFileSize = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    // Padding zero then send to apply
    size_t zeroPaddingSize =
        (inputFileSize % frameSizeInput == 0) ? 0 : (frameSizeInput - inputFileSize % frameSizeInput);
    size_t fileBufferSize = inputFileSize + zeroPaddingSize;
    std::vector<float> inputfileBuffer(fileBufferSize / sizeof(float), 0.0f);  // 32bit-float PCM data
    ifs.read(reinterpret_cast<char *>(inputfileBuffer.data()), inputFileSize);
    ifs.close();

    // apply data
    std::vector<uint8_t> outputfileBuffer(fileBufferSize);
    uint8_t *readPtr = reinterpret_cast<uint8_t *>(inputfileBuffer.data());
    uint8_t *writePtr = outputfileBuffer.data();
    size_t outputFileSize = 0;
    for (int32_t i = 0; i + frameSizeInput <= fileBufferSize; i += frameSizeInput) {
        memcpy_s(reinterpret_cast<char *>(inputData), frameSizeInput, readPtr, frameSizeInput);

        AudioSuitePcmBuffer *pcmBufferOutputPtr = node->SignalProcess(inputs);

        uint8_t *outputData = reinterpret_cast<uint8_t *>(pcmBufferOutputPtr->GetPcmDataBuffer());
        memcpy_s(writePtr, frameSizeOutput, outputData, frameSizeOutput);

        readPtr += frameSizeInput;
        writePtr += frameSizeOutput;
        outputFileSize += frameSizeOutput;
    }

    CHECK_AND_RETURN_RET(node->DeInit() == SUCCESS, ERROR);

    // write to output file
    bool isCreateFileSucc = CreateOutputPcmFile(outputFile);
    CHECK_AND_RETURN_RET(isCreateFileSucc, ERROR);
    bool isWriteFileSucc = WritePcmFile(outputFile, outputfileBuffer.data(), outputFileSize);
    CHECK_AND_RETURN_RET(isWriteFileSucc, ERROR);

    // compare the output file with target file
    bool isFileEqual = IsFilesEqual(outputFile, targetFile);
    CHECK_AND_RETURN_RET(isFileEqual, ERROR);

    return SUCCESS;
}

HWTEST_F(AudioSuiteNrNodeUnitTest, TestInitAndDeinit_001, TestSize.Level0)
{
    EXPECT_EQ(nrNode_->Init(), SUCCESS);
    EXPECT_EQ(nrNode_->DeInit(), SUCCESS);
}

HWTEST_F(AudioSuiteNrNodeUnitTest, TestSignalProcess_Resample_and_ChannelConvert_001, TestSize.Level0)
{
    AudioSuitePcmBuffer pcmBufferInput(SAMPLE_RATE_48000, STEREO, CH_LAYOUT_STEREO);
    inputs_[0] = &pcmBufferInput;

    int32_t ret = RunSignalProcessTest(nrNode_, g_inputPcmFilePath001, g_outputPcmFilePath001, g_targetPcmFilePath001,
        inputs_);

    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuiteNrNodeUnitTest, TestSignalProcess_Resample_002, TestSize.Level0)
{
    AudioSuitePcmBuffer pcmBufferInput(SAMPLE_RATE_48000, MONO, CH_LAYOUT_MONO);
    inputs_[0] = &pcmBufferInput;

    int32_t ret = RunSignalProcessTest(nrNode_, g_inputPcmFilePath002, g_outputPcmFilePath002, g_targetPcmFilePath002,
        inputs_);

    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuiteNrNodeUnitTest, TestSignalProcess_ChannelConvert_003, TestSize.Level0)
{
    AudioSuitePcmBuffer pcmBufferInput(SAMPLE_RATE_16000, STEREO, CH_LAYOUT_STEREO);
    inputs_[0] = &pcmBufferInput;

    int32_t ret = RunSignalProcessTest(nrNode_, g_inputPcmFilePath003, g_outputPcmFilePath003, g_targetPcmFilePath003,
        inputs_);

    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuiteNrNodeUnitTest, TestSignalProcess_OnlyAlgo_004, TestSize.Level0)
{
    AudioSuitePcmBuffer pcmBufferInput(SAMPLE_RATE_16000, MONO, CH_LAYOUT_MONO);
    inputs_[0] = &pcmBufferInput;

    int32_t ret = RunSignalProcessTest(nrNode_, g_inputPcmFilePath004, g_outputPcmFilePath004, g_targetPcmFilePath004,
        inputs_);

    EXPECT_EQ(ret, SUCCESS);
}


HWTEST_F(AudioSuiteNrNodeUnitTest, TestSignalProcess_NotInit_001, TestSize.Level0)
{
    AudioSuitePcmBuffer pcmBufferInput(SAMPLE_RATE_48000, MONO, CH_LAYOUT_MONO);
    AudioSuitePcmBuffer silenceData(SAMPLE_RATE_16000, MONO, CH_LAYOUT_MONO);
    float *sData = silenceData.GetPcmDataBuffer();
    float *inputData = pcmBufferInput.GetPcmDataBuffer();
    uint32_t dataLen = pcmBufferInput.GetFrameLen();
    for (uint32_t i = 0; i < dataLen; i++) {
        inputData[i] = 0.1f;
    }

    inputs_[0] = &pcmBufferInput;
    float *outputData1 = nrNode_->SignalProcess(inputs_)->GetPcmDataBuffer();
    EXPECT_EQ(std::memcmp(outputData1, sData, dataLen), 0);

    // init
    EXPECT_EQ(nrNode_->Init(), SUCCESS);
    inputs_[0] = &pcmBufferInput;
    float *outputData2 = nrNode_->SignalProcess(inputs_)->GetPcmDataBuffer();
    EXPECT_NE(std::memcmp(outputData2, sData, dataLen), 0);

    // deinit
    EXPECT_EQ(nrNode_->DeInit(), SUCCESS);
    inputs_[0] = &pcmBufferInput;
    float *outputData3 = nrNode_->SignalProcess(inputs_)->GetPcmDataBuffer();
    EXPECT_NE(std::memcmp(outputData3, sData, dataLen), 0);
}

HWTEST_F(AudioSuiteNrNodeUnitTest, TestSignalProcess_InvalidInput_001, TestSize.Level0)
{
    EXPECT_EQ(nrNode_->Init(), SUCCESS);

    AudioSuitePcmBuffer silenceData(SAMPLE_RATE_16000, MONO, CH_LAYOUT_MONO);
    float *sData = silenceData.GetPcmDataBuffer();
    uint32_t dataLen = silenceData.GetFrameLen();

    // inputs_ data is nullptr
    inputs_[0] = nullptr;
    float *outputData1 = nrNode_->SignalProcess(inputs_)->GetPcmDataBuffer();
    EXPECT_EQ(std::memcmp(outputData1, sData, dataLen), 0);

    EXPECT_EQ(nrNode_->DeInit(), SUCCESS);
}

}  // namespace