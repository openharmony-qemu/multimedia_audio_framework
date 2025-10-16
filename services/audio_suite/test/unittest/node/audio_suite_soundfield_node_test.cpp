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
#include "audio_suite_soundfield_node.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;

namespace {
static std::string g_inputfile001 = "/data/audiosuite/soundfield_input_48000_2_F32LE.pcm";
static std::string g_inputfile002 = "/data/audiosuite/soundfield_input_44100_1_F32LE.pcm";

static std::string g_targetfile001 = "/data/audiosuite/soundfield_target_48000_2_to_48000_2_F32LE.pcm";
static std::string g_targetfile002 = "/data/audiosuite/soundfield_target_44100_1_to_48000_2_F32LE.pcm";

static std::string g_outputfile001 = "/data/audiosuite/soundfield_output_48000_2_to_48000_2_F32LE.pcm";
static std::string g_outputfile002 = "/data/audiosuite/soundfield_output_44100_1_to_48000_2_F32LE.pcm";

class AudioSuiteSoundFieldNodeUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        std::filesystem::remove(g_outputfile001);
        std::filesystem::remove(g_outputfile002);
    }
    static void TearDownTestCase(void){};
    void SetUp(void);
    void TearDown(void);

    int32_t RunSignalProcessTest(std::shared_ptr<AudioSuiteSoundFieldNode> node,
        const std::vector<AudioSuitePcmBuffer *> &inputs, const std::string &inputFile, const std::string &outputFile,
        const std::string &targetFile);

    std::shared_ptr<AudioSuiteSoundFieldNode> node_;
    std::vector<AudioSuitePcmBuffer *> inputs_;
    std::unique_ptr<AudioSuitePcmBuffer> pcmBufferOutput_;
};

void AudioSuiteSoundFieldNodeUnitTest::SetUp(void)
{
    node_ = std::make_shared<AudioSuiteSoundFieldNode>();
    inputs_.resize(1);
    pcmBufferOutput_ = std::make_unique<AudioSuitePcmBuffer>(SAMPLE_RATE_48000, STEREO, CH_LAYOUT_STEREO);
}

void AudioSuiteSoundFieldNodeUnitTest::TearDown(void)
{
    node_.reset();
    pcmBufferOutput_.reset();
}

int32_t AudioSuiteSoundFieldNodeUnitTest::RunSignalProcessTest(std::shared_ptr<AudioSuiteSoundFieldNode> node,
    const std::vector<AudioSuitePcmBuffer *> &inputs, const std::string &inputFile, const std::string &outputFile,
    const std::string &targetFile)
{
    size_t frameSizeInput = inputs[0]->GetFrameLen() * sizeof(float);
    size_t frameSizeOutput = pcmBufferOutput_->GetFrameLen() * sizeof(float);
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
    size_t inputfileBufferSize = inputFileSize + zeroPaddingSize;
    std::vector<float> inputfileBuffer(inputfileBufferSize / sizeof(float), 0.0f);  // 32bit-float PCM data
    ifs.read(reinterpret_cast<char *>(inputfileBuffer.data()), inputFileSize);
    ifs.close();

    // apply data
    size_t outputfileBufferSize = inputfileBufferSize * frameSizeOutput / frameSizeInput;
    std::vector<uint8_t> outputfileBuffer(outputfileBufferSize);
    uint8_t *readPtr = reinterpret_cast<uint8_t *>(inputfileBuffer.data());
    uint8_t *writePtr = outputfileBuffer.data();
    for (int32_t i = 0; i + frameSizeInput <= inputfileBufferSize; i += frameSizeInput) {
        memcpy_s(reinterpret_cast<char *>(inputData), frameSizeInput, readPtr, frameSizeInput);

        AudioSuitePcmBuffer *pcmBufferOutputPtr = node->SignalProcess(inputs);

        uint8_t *outputData = reinterpret_cast<uint8_t *>(pcmBufferOutputPtr->GetPcmDataBuffer());
        memcpy_s(writePtr, frameSizeOutput, outputData, frameSizeOutput);

        readPtr += frameSizeInput;
        writePtr += frameSizeOutput;
    }

    // write to output file
    bool isCreateFileSucc = CreateOutputPcmFile(outputFile);
    CHECK_AND_RETURN_RET(isCreateFileSucc, ERROR);
    bool isWriteFileSucc = WritePcmFile(outputFile, outputfileBuffer.data(), outputfileBufferSize);
    CHECK_AND_RETURN_RET(isWriteFileSucc, ERROR);

    // compare the output file with target file
    bool isFileEqual = IsFilesEqual(outputFile, targetFile);
    CHECK_AND_RETURN_RET(isFileEqual, ERROR);

    return SUCCESS;
}

HWTEST_F(AudioSuiteSoundFieldNodeUnitTest, TestInitAndDeinit_001, TestSize.Level0)
{
    EXPECT_EQ(node_->Init(), SUCCESS);
    EXPECT_EQ(node_->DeInit(), SUCCESS);
}

HWTEST_F(AudioSuiteSoundFieldNodeUnitTest, TestSetOptions_001, TestSize.Level0)
{
    EXPECT_EQ(node_->Init(), SUCCESS);

    std::string name = "SoundFieldType";
    std::string value;

    // 前置
    value = std::to_string(static_cast<int32_t>(AUDIO_SUITE_SOUND_FIELD_FRONT_FACING));
    EXPECT_EQ(node_->SetOptions(name, value), SUCCESS);

    // 宏大
    value = std::to_string(static_cast<int32_t>(AUDIO_SUITE_SOUND_FIELD_GRAND));
    EXPECT_EQ(node_->SetOptions(name, value), SUCCESS);

    // 聆听
    value = std::to_string(static_cast<int32_t>(AUDIO_SUITE_SOUND_FIELD_NEAR));
    EXPECT_EQ(node_->SetOptions(name, value), SUCCESS);

    // 宽广
    value = std::to_string(static_cast<int32_t>(AUDIO_SUITE_SOUND_FIELD_WIDE));
    EXPECT_EQ(node_->SetOptions(name, value), SUCCESS);

    // 无效值
    EXPECT_EQ(node_->SetOptions(name, "9"), ERROR);

    EXPECT_EQ(node_->DeInit(), SUCCESS);
}

HWTEST_F(AudioSuiteSoundFieldNodeUnitTest, TestSignalProcess_WithoutConvert_001, TestSize.Level0)
{
    EXPECT_EQ(node_->Init(), SUCCESS);

    AudioSuitePcmBuffer pcmBufferInput(SAMPLE_RATE_48000, STEREO, CH_LAYOUT_STEREO);
    inputs_[0] = &pcmBufferInput;
    int32_t ret = RunSignalProcessTest(node_, inputs_, g_inputfile001, g_outputfile001, g_targetfile001);
    EXPECT_EQ(ret, SUCCESS);

    EXPECT_EQ(node_->DeInit(), SUCCESS);
}

HWTEST_F(AudioSuiteSoundFieldNodeUnitTest, TestSignalProcess_Resample_and_ChannelConvert_002, TestSize.Level0)
{
    EXPECT_EQ(node_->Init(), SUCCESS);

    AudioSuitePcmBuffer pcmBufferInput(SAMPLE_RATE_44100, MONO, CH_LAYOUT_MONO);
    inputs_[0] = &pcmBufferInput;
    int32_t ret = RunSignalProcessTest(node_, inputs_, g_inputfile002, g_outputfile002, g_targetfile002);
    EXPECT_EQ(ret, SUCCESS);

    EXPECT_EQ(node_->DeInit(), SUCCESS);
}

HWTEST_F(AudioSuiteSoundFieldNodeUnitTest, TestSignalProcess_NotInit_003, TestSize.Level0)
{
    AudioSuitePcmBuffer pcmBufferInput(SAMPLE_RATE_48000, STEREO, CH_LAYOUT_STEREO);
    AudioSuitePcmBuffer silenceData(SAMPLE_RATE_48000, STEREO, CH_LAYOUT_STEREO);
    inputs_[0] = &pcmBufferInput;
    float *sData = silenceData.GetPcmDataBuffer();
    float *inputData = pcmBufferInput.GetPcmDataBuffer();
    uint32_t dataLen = pcmBufferInput.GetFrameLen();
    for (uint32_t i = 0; i < dataLen; i++) {
        inputData[i] = 0.1f;
    }

    // not init -> output silence data
    float *outputData1 = node_->SignalProcess(inputs_)->GetPcmDataBuffer();
    EXPECT_EQ(std::memcmp(outputData1, sData, dataLen), 0);

    // init
    EXPECT_EQ(node_->Init(), SUCCESS);
    float *outputData2 = node_->SignalProcess(inputs_)->GetPcmDataBuffer();
    EXPECT_NE(std::memcmp(outputData2, sData, dataLen), 0);

    // deinit -> output silence data
    EXPECT_EQ(node_->DeInit(), SUCCESS);
    float *outputData3 = node_->SignalProcess(inputs_)->GetPcmDataBuffer();
    EXPECT_EQ(std::memcmp(outputData3, sData, dataLen), 0);
}

HWTEST_F(AudioSuiteSoundFieldNodeUnitTest, TestSignalProcess_InvalidInput_004, TestSize.Level0)
{
    EXPECT_EQ(node_->Init(), SUCCESS);

    AudioSuitePcmBuffer silenceData(SAMPLE_RATE_48000, STEREO, CH_LAYOUT_STEREO);
    float *sData = silenceData.GetPcmDataBuffer();
    uint32_t dataLen = silenceData.GetFrameLen();

    // inputs_ data is nullptr -> output silence data
    inputs_[0] = nullptr;
    float *outputData1 = node_->SignalProcess(inputs_)->GetPcmDataBuffer();
    EXPECT_EQ(std::memcmp(outputData1, sData, dataLen), 0);

    EXPECT_EQ(node_->DeInit(), SUCCESS);
}

}  // namespace