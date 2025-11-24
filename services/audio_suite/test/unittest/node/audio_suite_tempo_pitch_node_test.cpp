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
#include "audio_suite_tempo_pitch_node.h"
#include "audio_suite_unittest_tools.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;
using namespace testing;
using namespace std;

namespace {
static std::string g_inputfile001 = "/data/audiosuite/tempo_pitch/in_48000_1_s16le.pcm";
static std::string g_outfile001 = "/data/audiosuite/tempo_pitch/out_48000_1_s16le_0.8_0.8.pcm";
static std::string g_outfile002 = "/data/audiosuite/tempo_pitch/out_48000_1_s16le_1.0_0.8.pcm";
static std::string g_outfile003 = "/data/audiosuite/tempo_pitch/out_48000_1_s16le_0.8_1.0.pcm";
static std::string g_targetfile001 = "/data/audiosuite/tempo_pitch/target_48000_1_s16le_0.8_0.8.pcm";
static std::string g_targetfile002 = "/data/audiosuite/tempo_pitch/target_48000_1_s16le_1.0_0.8.pcm";
static std::string g_targetfile003 = "/data/audiosuite/tempo_pitch/target_48000_1_s16le_0.8_1.0.pcm";

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
class AudioSuiteTempoPitchNodeTest : public ::testing::Test {
public:
    void SetUp() override
    {
        std::filesystem::remove(g_outfile001);
        std::filesystem::remove(g_outfile002);
        std::filesystem::remove(g_outfile003);
    }
    void TearDown(void){};
    int32_t DoprocessTest(float speed, float pitch, std::string inputFile, std::string outputFile);

    PcmBufferFormat outFormat_ = {SAMPLE_RATE_48000, MONO, CH_LAYOUT_MONO, SAMPLE_S16LE};
    std::unique_ptr<AudioSuitePcmBuffer> buffer = std::make_unique<AudioSuitePcmBuffer>(outFormat_);
};

int32_t AudioSuiteTempoPitchNodeTest::DoprocessTest(float speed, float pitch, std::string inputFile, std::string outputFile)
{
    std::shared_ptr<AudioSuiteTempoPitchNode> node = std::make_shared<AudioSuiteTempoPitchNode>();
    node->Init();
    std::shared_ptr<MockInputNode> mockInputNode_ = std::make_shared<MockInputNode>();
    std::shared_ptr<OutputPort<AudioSuitePcmBuffer*>> inputNodeOutputPort =
        std::make_shared<OutputPort<AudioSuitePcmBuffer*>>(mockInputNode_);
    EXPECT_CALL(*mockInputNode_, GetOutputPort())
        .Times(1).WillRepeatedly(::testing::Return(inputNodeOutputPort));

    std::string option_value = std::to_string(speed) + "," + std::to_string(pitch);
    int32_t ret = node->SetOptions("speedAndPitch", option_value);
    CHECK_AND_RETURN_RET(ret == SUCCESS, ret);
    node->Connect(mockInputNode_);
    CHECK_AND_RETURN_RET(inputNodeOutputPort->GetInputNum() == 1, ERROR);
    std::shared_ptr<OutputPort<AudioSuitePcmBuffer*>> nodeOutputPort =
        node->GetOutputPort();

    size_t frameSizeInput = buffer->GetDataSize();
    // Read input file
    std::ifstream ifs(inputFile, std::ios::binary);
    CHECK_AND_RETURN_RET(ifs.is_open(), ERROR);
    ifs.seekg(0, std::ios::end);
    size_t inputFileSize = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    // Padding zero then send to apply
    EXPECT_NE(frameSizeInput, 0);
    size_t zeroPaddingSize =
        (inputFileSize % frameSizeInput == 0) ? 0 : (frameSizeInput - inputFileSize % frameSizeInput);
    size_t inputFileBufferSize = inputFileSize + zeroPaddingSize;
    std::vector<uint8_t> inputfileBuffer(inputFileBufferSize, 0);  // PCM data padding 0
    ifs.read(reinterpret_cast<char *>(inputfileBuffer.data()), inputFileSize);
    ifs.close();
    // open outfile
    std::ofstream outFile(outputFile, std::ios::binary | std::ios::out | std::ios::app);

    uint8_t *readPtr = inputfileBuffer.data();
    int32_t frames = inputFileBufferSize / frameSizeInput;
    int32_t frameIndex = 0;
    while(!node->GetAudioNodeDataFinishedFlag()) {
        EXPECT_CALL(*mockInputNode_, DoProcess())
            .WillRepeatedly(::testing::Invoke([&]() {
            if (frameIndex == frames - 1) {
                buffer->SetIsFinished(true);
            }
            memcpy_s(buffer->GetPcmData(), frameSizeInput, readPtr, frameSizeInput);
            inputNodeOutputPort->WriteDataToOutput(buffer.get());
            frameIndex++;
            readPtr += frameSizeInput;
            return SUCCESS;
        }));
        std::vector<AudioSuitePcmBuffer *> result = nodeOutputPort->PullOutputData(outFormat_, true);
        CHECK_AND_RETURN_RET(result.size() == 1, ERROR);
        outFile.write(reinterpret_cast<const char *>(result[0]->GetPcmData()), frameSizeInput);
    }

    outFile.close();
    node->DisConnect(mockInputNode_);
    EXPECT_EQ(inputNodeOutputPort->GetInputNum(), 0);
    testing::Mock::VerifyAndClearExpectations(mockInputNode_.get());
    buffer->SetIsFinished(false);
    node->Flush();
    mockInputNode_.reset();
    inputNodeOutputPort.reset();
    return SUCCESS;
}

HWTEST_F(AudioSuiteTempoPitchNodeTest, DoProcessTest, TestSize.Level0)
{
    float speed = 0.8f;
    float pitch = 0.8f;
    int ret = DoprocessTest(speed, pitch, g_inputfile001, g_outfile001);
    EXPECT_EQ(ret, SUCCESS);
    bool isFileEqual = IsFilesEqual(g_outfile001, g_targetfile001);
    EXPECT_EQ(isFileEqual, true);

    speed = 1.0f;
    pitch = 0.8f;
    ret = DoprocessTest(speed, pitch, g_inputfile001, g_outfile002);
    EXPECT_EQ(ret, SUCCESS);
    isFileEqual = IsFilesEqual(g_outfile002, g_targetfile002);
    EXPECT_EQ(isFileEqual, true);

    speed = 0.8f;
    pitch = 1.0f;
    ret = DoprocessTest(speed, pitch, g_inputfile001, g_outfile003);
    EXPECT_EQ(ret, SUCCESS);
    isFileEqual = IsFilesEqual(g_outfile003, g_targetfile003);
    EXPECT_EQ(isFileEqual, true);
}

HWTEST_F(AudioSuiteTempoPitchNodeTest, InitTest, TestSize.Level0)
{
    std::shared_ptr<AudioSuiteTempoPitchNode> node = std::make_shared<AudioSuiteTempoPitchNode>();
    int ret = node->Init();
    EXPECT_EQ(ret, SUCCESS);

    ret = node->Init();
    EXPECT_EQ(ret, ERROR);
}

HWTEST_F(AudioSuiteTempoPitchNodeTest, DeInitTest, TestSize.Level0)
{
    std::shared_ptr<AudioSuiteTempoPitchNode> node = std::make_shared<AudioSuiteTempoPitchNode>();
    std::vector<uint8_t> tempOutput;
    node->readyDataBuffer_.push(tempOutput);
    int ret = node->DeInit();
    EXPECT_EQ(ret, ERROR);
}

HWTEST_F(AudioSuiteTempoPitchNodeTest, DoProcessPreOutputsTest_001, TestSize.Level0)
{
    std::shared_ptr<AudioSuiteTempoPitchNode> node = std::make_shared<AudioSuiteTempoPitchNode>();
    AudioSuitePcmBuffer* tempOut = nullptr;
    int ret = node->DoProcessPreOutputs(&tempOut);
    EXPECT_EQ(ret, ERROR);

    node->SetBypassEffectNode(true);
    ret = node->DoProcessPreOutputs(&tempOut);
    EXPECT_EQ(ret, ERROR);
}

HWTEST_F(AudioSuiteTempoPitchNodeTest, DoProcessPreOutputsTest_002, TestSize.Level0)
{
    std::shared_ptr<AudioSuiteTempoPitchNode> node = std::make_shared<AudioSuiteTempoPitchNode>();
    node->Init();
    node->SetBypassEffectNode(true);
    std::shared_ptr<MockInputNode> mockInputNode_ = std::make_shared<MockInputNode>();
    std::shared_ptr<OutputPort<AudioSuitePcmBuffer*>> inputNodeOutputPort =
        std::make_shared<OutputPort<AudioSuitePcmBuffer*>>(mockInputNode_);
    EXPECT_CALL(*mockInputNode_, GetOutputPort())
        .Times(1).WillRepeatedly(::testing::Return(inputNodeOutputPort));

    node->Connect(mockInputNode_);
    EXPECT_EQ(inputNodeOutputPort->GetInputNum(), 1);
    std::shared_ptr<OutputPort<AudioSuitePcmBuffer*>> nodeOutputPort =
        node->GetOutputPort();
    EXPECT_CALL(*mockInputNode_, DoProcess())
            .WillRepeatedly(::testing::Invoke([&]() {
            std::vector<uint8_t> tempData(1920, 0);
            memcpy_s(buffer->GetPcmData(), 1920, tempData.data(), 1920);
            inputNodeOutputPort->WriteDataToOutput(buffer.get());
            return SUCCESS;
        }));
    std::vector<AudioSuitePcmBuffer *> result = nodeOutputPort->PullOutputData(outFormat_, true);
    EXPECT_EQ(result.size() ,1);

    node->DisConnect(mockInputNode_);
    EXPECT_EQ(inputNodeOutputPort->GetInputNum(), 0);
    testing::Mock::VerifyAndClearExpectations(mockInputNode_.get());
    node->Flush();
    mockInputNode_.reset();
    inputNodeOutputPort.reset();
}
}  // namespace