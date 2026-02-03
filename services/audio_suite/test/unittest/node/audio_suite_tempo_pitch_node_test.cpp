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
#include "audio_suite_log.h"

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
static int32_t g_expectedGetOutputPortCalls = 2;      // Times of GetOutputPort called in DoProcess
static constexpr uint32_t needDataLength = 20;
static constexpr int32_t frameBytes = 4352;
const int MAX_FRAMES = 2000;

class MockInputNode : public AudioNode {
public:
    MockInputNode() : AudioNode(NODE_TYPE_EQUALIZER)
    {}
    ~MockInputNode() {}
    MOCK_METHOD(int32_t, DoProcess, (uint32_t needDataLength), (override));
    MOCK_METHOD(OutputPort<AudioSuitePcmBuffer*>*, GetOutputPort, ());
    MOCK_METHOD(int32_t, Flush, (), ());
    MOCK_METHOD(int32_t, Connect, (const std::shared_ptr<AudioNode> &preNode, AudioNodePortType type), ());
    MOCK_METHOD(int32_t, Connect, (const std::shared_ptr<AudioNode> &preNode), ());
    MOCK_METHOD(int32_t, DisConnect, (const std::shared_ptr<AudioNode> &preNode), ());
};
class AudioSuiteTempoPitchNodeTest : public ::testing::Test {
public:
    void SetUp() override
    {
        if (!AllNodeTypesSupported()) {
            GTEST_SKIP() << "not support all node types, skip this test";
        }
        std::filesystem::remove(g_outfile001);
        std::filesystem::remove(g_outfile002);
        std::filesystem::remove(g_outfile003);
    }
    void TearDown(void){};
    int32_t DoprocessTest(float speed, float pitch, std::string inputFile, std::string outputFile);
    std::vector<uint8_t> ReadInputFile(std::string inputFile, size_t frameSizeInput);

    PcmBufferFormat outFormat_ = {SAMPLE_RATE_48000, MONO, CH_LAYOUT_MONO, SAMPLE_S16LE};
    std::unique_ptr<AudioSuitePcmBuffer> buffer = std::make_unique<AudioSuitePcmBuffer>(outFormat_, needDataLength);
};

std::vector<uint8_t> AudioSuiteTempoPitchNodeTest::ReadInputFile(std::string inputFile, size_t frameSizeInput)
{
    std::ifstream ifs(inputFile, std::ios::binary);
    CHECK_AND_RETURN_RET(ifs.is_open(), std::vector<uint8_t>());
    ifs.seekg(0, std::ios::end);
    size_t inputFileSize = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    // Padding zero then send to apply
    CHECK_AND_RETURN_RET(frameSizeInput > 0, std::vector<uint8_t>());
    size_t zeroPaddingSize =
        (inputFileSize % frameSizeInput == 0) ? 0 : (frameSizeInput - inputFileSize % frameSizeInput);
    size_t inputFileBufferSize = inputFileSize + zeroPaddingSize;
    std::vector<uint8_t> inputfileBuffer(inputFileBufferSize, 0);  // PCM data padding 0
    ifs.read(reinterpret_cast<char *>(inputfileBuffer.data()), inputFileSize);
    ifs.close();
    return inputfileBuffer;
}

int32_t AudioSuiteTempoPitchNodeTest::DoprocessTest(
    float speed, float pitch, std::string inputFile, std::string outputFile)
{
    std::shared_ptr<AudioSuiteTempoPitchNode> node = std::make_shared<AudioSuiteTempoPitchNode>();
    node->Init();
    std::shared_ptr<MockInputNode> mockInputNode_ = std::make_shared<MockInputNode>();
    OutputPort<AudioSuitePcmBuffer*> inputNodeOutputPort;
    inputNodeOutputPort.SetOutputPort(mockInputNode_);
    EXPECT_CALL(*mockInputNode_, GetOutputPort())
        .Times(g_expectedGetOutputPortCalls).WillRepeatedly(::testing::Return(&inputNodeOutputPort));

    std::string option_value = std::to_string(speed) + "," + std::to_string(pitch);
    EXPECT_EQ(node->SetOptions("speedAndPitch", option_value), SUCCESS);
    node->Connect(mockInputNode_);
    CHECK_AND_RETURN_RET(inputNodeOutputPort.GetInputNum() == 1, ERROR);
    OutputPort<AudioSuitePcmBuffer*>* nodeOutputPort = node->GetOutputPort();

    size_t frameSizeInput = buffer->GetDataSize();
    CHECK_AND_RETURN_RET(frameSizeInput > 0, ERROR);
    std::vector<uint8_t> inputfileBuffer = ReadInputFile(inputFile, frameSizeInput);
    CHECK_AND_RETURN_RET(inputfileBuffer.empty() == false, ERROR);
    std::ofstream outFile(outputFile, std::ios::binary | std::ios::out);
    
    uint8_t *readPtr = inputfileBuffer.data();
    int32_t frames = inputfileBuffer.size() / frameSizeInput;
    int32_t frameIndex = 0;
    uint32_t loopCount = 0;
    while (loopCount < MAX_FRAMES) {
        EXPECT_CALL(*mockInputNode_, DoProcess(needDataLength))
            .WillRepeatedly(::testing::Invoke([&]() {
            if (frameIndex == frames - 1) {
                buffer->SetIsFinished(true);
            }
            memcpy_s(buffer->GetPcmData(), frameSizeInput, readPtr, frameSizeInput);
            inputNodeOutputPort.WriteDataToOutput(buffer.get());
            frameIndex++;
            readPtr += frameSizeInput;
            return SUCCESS;
        }));
        std::vector<AudioSuitePcmBuffer *> result = nodeOutputPort->PullOutputData(outFormat_, false, needDataLength);
        CHECK_AND_RETURN_RET(result.size() == 1, ERROR);
        outFile.write(reinterpret_cast<const char *>(result[0]->GetPcmData()), frameSizeInput);
        if (result[0]->GetIsFinished()  || ++loopCount >= MAX_FRAMES) {
            break;
        }
    }

    outFile.close();
    node->DisConnect(mockInputNode_);
    CHECK_AND_RETURN_RET(inputNodeOutputPort.GetInputNum() == 0, ERROR);
    testing::Mock::VerifyAndClearExpectations(mockInputNode_.get());
    buffer->SetIsFinished(false);
    node->Flush();
    mockInputNode_.reset();
    return SUCCESS;
}

HWTEST_F(AudioSuiteTempoPitchNodeTest, DoProcessTest, TestSize.Level0)
{
    float speed = 0.8f;
    float pitch = 0.8f;
    int ret = DoprocessTest(speed, pitch, g_inputfile001, g_outfile001);
    EXPECT_EQ(SUCCESS, ret);
    bool isFileEqual = IsFilesEqual(g_outfile001, g_targetfile001);
    EXPECT_EQ(true, isFileEqual);

    speed = 1.0f;
    pitch = 0.8f;
    ret = DoprocessTest(speed, pitch, g_inputfile001, g_outfile002);
    EXPECT_EQ(SUCCESS, ret);
    isFileEqual = IsFilesEqual(g_outfile002, g_targetfile002);
    EXPECT_EQ(true, isFileEqual);

    speed = 0.8f;
    pitch = 1.0f;
    ret = DoprocessTest(speed, pitch, g_inputfile001, g_outfile003);
    EXPECT_EQ(SUCCESS, ret);
    isFileEqual = IsFilesEqual(g_outfile003, g_targetfile003);
    EXPECT_EQ(true, isFileEqual);
}

HWTEST_F(AudioSuiteTempoPitchNodeTest, InitTest, TestSize.Level0)
{
    std::shared_ptr<AudioSuiteTempoPitchNode> node = std::make_shared<AudioSuiteTempoPitchNode>();
    int ret = node->Init();
    EXPECT_EQ(SUCCESS, ret);

    ret = node->Init();
    EXPECT_EQ(ERROR, ret);
}

HWTEST_F(AudioSuiteTempoPitchNodeTest, DeInitTest, TestSize.Level0)
{
    std::shared_ptr<AudioSuiteTempoPitchNode> node = std::make_shared<AudioSuiteTempoPitchNode>();
    std::vector<uint8_t> tempOutput;
    int ret = node->DeInit();
    EXPECT_EQ(ERROR, ret);
}

HWTEST_F(AudioSuiteTempoPitchNodeTest, CalculationNeedBytesTest001, TestSize.Level0)
{
    std::shared_ptr<AudioSuiteTempoPitchNode> node = std::make_shared<AudioSuiteTempoPitchNode>();
    node->Init();
    node->speedRate_ = 1.0;

    int32_t ret = node->CalculationNeedBytes(needDataLength);
    EXPECT_EQ(ret, frameBytes);
}

}  // namespace