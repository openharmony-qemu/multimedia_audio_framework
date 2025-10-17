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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <vector>
#include <string>
#include "audio_suite_process_node.h"
#include "audio_suite_manager.h"
#include "audio_suite_unittest_tools.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;
using namespace HPAE;
namespace {

constexpr uint32_t TEST_BUFFER_LEN = 10;
constexpr bool MIX_FLE = true;

static std::string g_inputPcmFilePath001 = "/data/audiosuite/processnode/input_48000_2_F32LE.pcm";
static std::string g_inputPcmFilePath002 = "/data/audiosuite/processnode/input_44100_2_F32LE.pcm";
static std::string g_inputPcmFilePath003 = "/data/audiosuite/processnode/input_44100_1_F32LE.pcm";

static std::string g_targetPcmFilePath001 = "/data/audiosuite/processnode/target_48000_2_to_48000_2_F32LE.pcm";
static std::string g_targetPcmFilePath002 = "/data/audiosuite/processnode/target_44100_2_to_48000_2_F32LE.pcm";
static std::string g_targetPcmFilePath003 = "/data/audiosuite/processnode/target_44100_2_to_44100_1_F32LE.pcm";
static std::string g_targetPcmFilePath004 = "/data/audiosuite/processnode/target_44100_1_to_44100_2_F32LE.pcm";
static std::string g_targetPcmFilePath005 = "/data/audiosuite/processnode/target_48000_2_to_16000_1_F32LE.pcm";
static std::string g_targetPcmFilePath006 = "/data/audiosuite/processnode/target_44100_1_to_48000_2_F32LE.pcm";

static std::string g_outputPcmFilePath001 = "/data/audiosuite/processnode/output_48000_2_copy_48000_2_F32LE.pcm";
static std::string g_outputPcmFilePath002 = "/data/audiosuite/processnode/output_44100_2_resample_48000_2_F32LE.pcm";
static std::string g_outputPcmFilePath003 = "/data/audiosuite/processnode/output_44100_2_downmix_44100_1_F32LE.pcm";
static std::string g_outputPcmFilePath004 = "/data/audiosuite/processnode/output_44100_1_upmix_44100_2_F32LE.pcm";
static std::string g_outputPcmFilePath005 = "/data/audiosuite/processnode/output_48000_2_to_16000_1_F32LE.pcm";
static std::string g_outputPcmFilePath006 = "/data/audiosuite/processnode/output_44100_1_to_48000_2_F32LE.pcm";

class MockInputNode : public AudioNode {
public:
    MockInputNode() : AudioNode(NODE_TYPE_EQUALIZER)
    {}
    ~MockInputNode() {}
    MOCK_METHOD(int32_t, DoProcess, (), ());
    MOCK_METHOD(std::shared_ptr<OutputPort<AudioSuitePcmBuffer*>>, GetOutputPort, ());
    MOCK_METHOD(int32_t, Flush, (), ());
    MOCK_METHOD(int32_t, Connect, (const std::shared_ptr<AudioNode> &preNode, AudioNodePortType type), ());
    MOCK_METHOD(int32_t, Connect, (const std::shared_ptr<AudioNode> &preNode), ());
    MOCK_METHOD(int32_t, DisConnect, (const std::shared_ptr<AudioNode> &preNode), ());
};
class TestAudioSuiteProcessNode : public AudioSuiteProcessNode {
public:
    TestAudioSuiteProcessNode(AudioNodeType nodeType, AudioFormat audioFormat)
        : AudioSuiteProcessNode(nodeType, audioFormat) {}
    ~TestAudioSuiteProcessNode() override = default;
    AudioSuitePcmBuffer* SignalProcess(const std::vector<AudioSuitePcmBuffer*>& inputs) override
    {
        if (!inputs.empty()) {
            // simulate a signal process.
            if (inputs[0] == nullptr) {
                return nullptr;
            }
            float* unProcessedData = inputs[0]->GetPcmDataBuffer();
            if (unProcessedData != nullptr) {
                *unProcessedData = 1.0f;
            }
            return inputs[0];
        }
        return nullptr;
    }
    bool Reset() override
    {
        return true;
    }
    std::shared_ptr<InputPort<AudioSuitePcmBuffer*>> GetInputPort()
    {
        return inputStream_;
    }
};
class TestReadTapCallBack : public SuiteNodeReadTapDataCallback {
public:
    static bool testFlag;
    void OnReadTapDataCallback(void *audioData, int32_t audioDataSize) override
    {
        testFlag = true;
    }
};
bool TestReadTapCallBack::testFlag = false;
class AudioSuiteProcessNodeTest : public ::testing::Test {
public:
    static void SetUpTestCase(void)
    {
        std::filesystem::remove(g_outputPcmFilePath001);
        std::filesystem::remove(g_outputPcmFilePath002);
        std::filesystem::remove(g_outputPcmFilePath003);
        std::filesystem::remove(g_outputPcmFilePath004);
        std::filesystem::remove(g_outputPcmFilePath005);
        std::filesystem::remove(g_outputPcmFilePath006);
    }
    static void TearDownTestCase(void){};
    void SetUp() override
    {
        // init nodeinfo
        AudioChannelInfo audioChannelInfo;
        AudioFormat audioFormat = {
            .audioChannelInfo = audioChannelInfo
        };
        node_ = std::make_shared<TestAudioSuiteProcessNode>(NODE_TYPE_EQUALIZER, audioFormat);
        TestReadTapCallBack::testFlag = false;
        tmpPcmBuffer_ = new AudioSuitePcmBuffer(SAMPLE_RATE_48000, STEREO, CH_LAYOUT_STEREO);
    }
    void TearDown() override
    {
        if (node_ == nullptr) {
            return;
        }
        node_->Flush();
        node_->Reset();
        TestReadTapCallBack::testFlag = false;
    };

    int32_t RunConvertProcessTest(const std::string &inputFile, const std::string &outputFile,
        const std::string &targetFile, AudioSuitePcmBuffer *inputPcmBuffer, AudioSuitePcmBuffer *outputPcmBuffer);

    std::shared_ptr<TestAudioSuiteProcessNode> node_;
    AudioSuitePcmBuffer *tmpPcmBuffer_;
};

int32_t AudioSuiteProcessNodeTest::RunConvertProcessTest(const std::string &inputFile, const std::string &outputFile,
    const std::string &targetFile, AudioSuitePcmBuffer *pcmBufferInput, AudioSuitePcmBuffer *pcmBufferOutput)
{
    size_t frameSizeInput = pcmBufferInput->GetFrameLen() * sizeof(float);
    size_t frameSizeOutput = pcmBufferOutput->GetFrameLen() * sizeof(float);
    float *inputData = pcmBufferInput->GetPcmDataBuffer();
    float *outputData = pcmBufferOutput->GetPcmDataBuffer();

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
    std::vector<float> inputfileBuffer(fileBufferSize / sizeof(float), 0.0f);  // 32 float PCM data
    ifs.read(reinterpret_cast<char *>(inputfileBuffer.data()), inputFileSize);
    ifs.close();

    // apply data
    std::vector<uint8_t> outputfileBuffer(fileBufferSize);
    uint8_t *readPtr = reinterpret_cast<uint8_t *>(inputfileBuffer.data());
    uint8_t *writePtr = outputfileBuffer.data();
    size_t outputFileSize = 0;
    for (size_t i = 0; i + frameSizeInput <= fileBufferSize; i += frameSizeInput) {
        memcpy_s(reinterpret_cast<char *>(inputData), frameSizeInput, readPtr, frameSizeInput);
        int32_t ret = node_->ConvertProcess(pcmBufferInput, pcmBufferOutput, tmpPcmBuffer_);
        CHECK_AND_RETURN_RET(ret == SUCCESS, ERROR);
        memcpy_s(writePtr, frameSizeOutput, reinterpret_cast<uint8_t *>(outputData), frameSizeOutput);
        
        readPtr += frameSizeInput;
        writePtr += frameSizeOutput;
        outputFileSize += frameSizeOutput;
    }

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

HWTEST_F(AudioSuiteProcessNodeTest, ConstructorTest, TestSize.Level0) {
    // test constructor
    EXPECT_NE(node_->GetInputPort(), nullptr);
    EXPECT_NE(node_->GetOutputPort(), nullptr);
    EXPECT_TRUE(node_->GetNodeBypassStatus());
}

HWTEST_F(AudioSuiteProcessNodeTest, ChannelConverterTestProcessTest, TestSize.Level0)
{
    // test channelConverter
    AudioChannelInfo inChannelInfo;
    AudioChannelInfo outChannelInfo;
    inChannelInfo.numChannels = MONO;
    inChannelInfo.channelLayout = CH_LAYOUT_MONO;
    outChannelInfo.numChannels = STEREO;
    outChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    std::vector<float> in(TEST_BUFFER_LEN * MONO, 0.0f);
    std::vector<float> out(TEST_BUFFER_LEN * STEREO, 0.0f);
    EXPECT_EQ(node_->SetChannelConvertProcessParam(inChannelInfo, outChannelInfo, SAMPLE_F32LE, MIX_FLE),
        SUCCESS);
    EXPECT_EQ(node_->ChannelConvertProcess(TEST_BUFFER_LEN, in.data(), in.size() * sizeof(float), out.data(),
        out.size() * sizeof(float)), SUCCESS);
    // test downmix
    inChannelInfo.numChannels = CHANNEL_6;
    inChannelInfo.channelLayout = CH_LAYOUT_5POINT1;
    in.resize(TEST_BUFFER_LEN * CHANNEL_6, 0.0f);
    EXPECT_EQ(node_->SetChannelConvertProcessParam(inChannelInfo, outChannelInfo, SAMPLE_F32LE, MIX_FLE),
        SUCCESS);
    EXPECT_EQ(node_->ChannelConvertProcess(TEST_BUFFER_LEN, in.data(), in.size() * sizeof(float), out.data(),
        out.size() * sizeof(float)), SUCCESS);
}

HWTEST_F(AudioSuiteProcessNodeTest, DoProcessDefaultTest, TestSize.Level0)
{
    std::unique_ptr<AudioSuitePcmBuffer> buffer = std::make_unique<AudioSuitePcmBuffer>(SAMPLE_RATE_48000,
        static_cast<uint32_t>(4), AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    std::shared_ptr<MockInputNode> mockInputNode_ = std::make_unique<MockInputNode>();
    std::shared_ptr<OutputPort<AudioSuitePcmBuffer*>> inputNodeOutputPort =
        std::make_shared<OutputPort<AudioSuitePcmBuffer*>>(mockInputNode_);
    inputNodeOutputPort->WriteDataToOutput(buffer.get());
    EXPECT_CALL(*mockInputNode_, DoProcess()).Times(1).WillRepeatedly(::testing::Return(SUCCESS));
    EXPECT_CALL(*mockInputNode_, GetOutputPort())
        .Times(1).WillRepeatedly(::testing::Return(inputNodeOutputPort));
    node_->Connect(mockInputNode_);
    EXPECT_EQ(inputNodeOutputPort->GetInputNum(), 1);
    std::shared_ptr<OutputPort<AudioSuitePcmBuffer*>> nodeOutputPort =
        node_->GetOutputPort();
    std::vector<AudioSuitePcmBuffer *> result = nodeOutputPort->PullOutputData();
    EXPECT_EQ(result.size(), 1);
    node_->DisConnect(mockInputNode_);
    EXPECT_EQ(inputNodeOutputPort->GetInputNum(), 0);
    testing::Mock::VerifyAndClearExpectations(mockInputNode_.get());
    mockInputNode_.reset();
    inputNodeOutputPort.reset();
}

HWTEST_F(AudioSuiteProcessNodeTest, DoProcessWithEnableProcessFalseTest, TestSize.Level0)
{
    node_->SetBypassEffectNode(false);
    std::unique_ptr<AudioSuitePcmBuffer> buffer = std::make_unique<AudioSuitePcmBuffer>(SAMPLE_RATE_48000,
        static_cast<uint32_t>(4), AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    std::shared_ptr<MockInputNode> mockInputNode_ = std::make_unique<MockInputNode>();
    std::shared_ptr<OutputPort<AudioSuitePcmBuffer*>> inputNodeOutputPort =
        std::make_shared<OutputPort<AudioSuitePcmBuffer*>>(mockInputNode_);
    inputNodeOutputPort->WriteDataToOutput(buffer.get());
    EXPECT_CALL(*mockInputNode_, DoProcess()).Times(1).WillRepeatedly(::testing::Return(SUCCESS));
    EXPECT_CALL(*mockInputNode_, GetOutputPort())
        .Times(1).WillRepeatedly(::testing::Return(inputNodeOutputPort));
    node_->Connect(mockInputNode_);
    EXPECT_EQ(inputNodeOutputPort->GetInputNum(), 1);
    std::shared_ptr<OutputPort<AudioSuitePcmBuffer*>> nodeOutputPort =
        node_->GetOutputPort();
    std::vector<AudioSuitePcmBuffer *> result = nodeOutputPort->PullOutputData();
    EXPECT_EQ(result.size(), 1);
    node_->DisConnect(mockInputNode_);
    EXPECT_EQ(inputNodeOutputPort->GetInputNum(), 0);
    testing::Mock::VerifyAndClearExpectations(mockInputNode_.get());
    mockInputNode_.reset();
    inputNodeOutputPort.reset();
    node_->SetBypassEffectNode(true);
}

HWTEST_F(AudioSuiteProcessNodeTest, DoProcessWithFinishedPcmBufferTest, TestSize.Level0)
{
    std::unique_ptr<AudioSuitePcmBuffer> buffer = std::make_unique<AudioSuitePcmBuffer>(SAMPLE_RATE_48000,
        static_cast<uint32_t>(4), AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    buffer->SetIsFinished(true);
    std::shared_ptr<MockInputNode> mockInputNode_ = std::make_unique<MockInputNode>();
    std::shared_ptr<OutputPort<AudioSuitePcmBuffer*>> inputNodeOutputPort =
        std::make_shared<OutputPort<AudioSuitePcmBuffer*>>(mockInputNode_);
    inputNodeOutputPort->WriteDataToOutput(buffer.get());
    EXPECT_CALL(*mockInputNode_, DoProcess()).Times(1).WillRepeatedly(::testing::Return(SUCCESS));
    EXPECT_CALL(*mockInputNode_, GetOutputPort())
        .Times(1).WillRepeatedly(::testing::Return(inputNodeOutputPort));
    node_->Connect(mockInputNode_);
    EXPECT_EQ(inputNodeOutputPort->GetInputNum(), 1);
    std::shared_ptr<OutputPort<AudioSuitePcmBuffer*>> nodeOutputPort =
        node_->GetOutputPort();
    std::vector<AudioSuitePcmBuffer *> result = nodeOutputPort->PullOutputData();
    EXPECT_EQ(result.size(), 1);
    EXPECT_NE(result[0], nullptr);
    EXPECT_EQ(result[0]->GetIsFinished(), true);
    EXPECT_EQ(node_->GetAudioNodeDataFinishedFlag(), true);
    std::vector<AudioSuitePcmBuffer *> resultWhenNodeFinished = nodeOutputPort->PullOutputData();
    EXPECT_EQ(resultWhenNodeFinished.size(), 0);
    node_->DisConnect(mockInputNode_);
    EXPECT_EQ(inputNodeOutputPort->GetInputNum(), 0);
    testing::Mock::VerifyAndClearExpectations(mockInputNode_.get());
    mockInputNode_.reset();
    inputNodeOutputPort.reset();
}

HWTEST_F(AudioSuiteProcessNodeTest, DoProcessInstallTapTest, TestSize.Level0)
{
    std::unique_ptr<AudioSuitePcmBuffer> buffer = std::make_unique<AudioSuitePcmBuffer>(SAMPLE_RATE_48000,
        static_cast<uint32_t>(4), AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    std::shared_ptr<MockInputNode> mockInputNode_ = std::make_unique<MockInputNode>();
    std::shared_ptr<OutputPort<AudioSuitePcmBuffer*>> inputNodeOutputPort =
        std::make_shared<OutputPort<AudioSuitePcmBuffer*>>(mockInputNode_);
    inputNodeOutputPort->WriteDataToOutput(buffer.get());
    EXPECT_CALL(*mockInputNode_, DoProcess()).Times(1).WillRepeatedly(::testing::Return(SUCCESS));
    EXPECT_CALL(*mockInputNode_, GetOutputPort())
        .Times(1).WillRepeatedly(::testing::Return(inputNodeOutputPort));
    node_->Connect(mockInputNode_);
    EXPECT_EQ(inputNodeOutputPort->GetInputNum(), 1);
    std::shared_ptr<OutputPort<AudioSuitePcmBuffer*>> nodeOutputPort =
        node_->GetOutputPort();
    std::shared_ptr<SuiteNodeReadTapDataCallback> testCallback = std::make_shared<TestReadTapCallBack>();
    EXPECT_FALSE(TestReadTapCallBack::testFlag);
    std::vector<AudioSuitePcmBuffer *> result = nodeOutputPort->PullOutputData();
    EXPECT_EQ(result.size(), 1);
    EXPECT_NE(result[0], nullptr);
    EXPECT_TRUE(TestReadTapCallBack::testFlag);
    node_->DisConnect(mockInputNode_);
    EXPECT_EQ(inputNodeOutputPort->GetInputNum(), 0);
    testing::Mock::VerifyAndClearExpectations(mockInputNode_.get());
    mockInputNode_.reset();
    inputNodeOutputPort.reset();
}

HWTEST_F(AudioSuiteProcessNodeTest, TestConvertProcess_001_CopyPcmBuffer, TestSize.Level0)
{
    AudioSuitePcmBuffer pcmBufferInput(SAMPLE_RATE_48000, STEREO, CH_LAYOUT_STEREO);
    AudioSuitePcmBuffer pcmBufferOutput(SAMPLE_RATE_48000, STEREO, CH_LAYOUT_STEREO);
    int32_t ret = RunConvertProcessTest(g_inputPcmFilePath001, g_outputPcmFilePath001, g_targetPcmFilePath001,
        &pcmBufferInput, &pcmBufferOutput);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(AudioSuiteProcessNodeTest, TestConvertProcess_002_Resample, TestSize.Level0)
{
    AudioSuitePcmBuffer pcmBufferInput(SAMPLE_RATE_44100, STEREO, CH_LAYOUT_STEREO);
    AudioSuitePcmBuffer pcmBufferOutput(SAMPLE_RATE_48000, STEREO, CH_LAYOUT_STEREO);
    int32_t ret = RunConvertProcessTest(g_inputPcmFilePath002, g_outputPcmFilePath002, g_targetPcmFilePath002,
        &pcmBufferInput, &pcmBufferOutput);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(AudioSuiteProcessNodeTest, TestConvertProcess_003_ChannelConvert_downmix, TestSize.Level0)
{
    AudioSuitePcmBuffer pcmBufferInput(SAMPLE_RATE_44100, STEREO, CH_LAYOUT_STEREO);
    AudioSuitePcmBuffer pcmBufferOutput(SAMPLE_RATE_44100, MONO, CH_LAYOUT_MONO);
    int32_t ret = RunConvertProcessTest(g_inputPcmFilePath002, g_outputPcmFilePath003, g_targetPcmFilePath003,
        &pcmBufferInput, &pcmBufferOutput);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(AudioSuiteProcessNodeTest, TestConvertProcess_004_ChannelConvert_upmix, TestSize.Level0)
{
    AudioSuitePcmBuffer pcmBufferInput(SAMPLE_RATE_44100, MONO, CH_LAYOUT_MONO);
    AudioSuitePcmBuffer pcmBufferOutput(SAMPLE_RATE_44100, STEREO, CH_LAYOUT_STEREO);
    int32_t ret = RunConvertProcessTest(g_inputPcmFilePath003, g_outputPcmFilePath004, g_targetPcmFilePath004,
        &pcmBufferInput, &pcmBufferOutput);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(AudioSuiteProcessNodeTest, TestConvertProcess_005_Resample_and_ChannelConvert, TestSize.Level0)
{
    AudioSuitePcmBuffer pcmBufferInput(SAMPLE_RATE_48000, STEREO, CH_LAYOUT_STEREO);
    AudioSuitePcmBuffer pcmBufferOutput(SAMPLE_RATE_16000, MONO, CH_LAYOUT_MONO);
    int32_t ret = RunConvertProcessTest(g_inputPcmFilePath001, g_outputPcmFilePath005, g_targetPcmFilePath005,
        &pcmBufferInput, &pcmBufferOutput);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(AudioSuiteProcessNodeTest, TestConvertProcess_006_ChannelConvert_and_Resample, TestSize.Level0)
{
    AudioSuitePcmBuffer pcmBufferInput(SAMPLE_RATE_44100, MONO, CH_LAYOUT_MONO);
    AudioSuitePcmBuffer pcmBufferOutput(SAMPLE_RATE_48000, STEREO, CH_LAYOUT_STEREO);
    int32_t ret = RunConvertProcessTest(g_inputPcmFilePath003, g_outputPcmFilePath006, g_targetPcmFilePath006,
        &pcmBufferInput, &pcmBufferOutput);
    EXPECT_EQ(ret, 0);
}

}