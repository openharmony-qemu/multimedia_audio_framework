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
 
#include <gmock/gmock.h>
#include "audio_suite_unittest_tools.h"
#include <gtest/gtest.h>
#include <vector>
#include <cmath>
#include <memory>
#include <fstream>
#include <cstring>
#include "audio_suite_node.h"
#include "audio_suite_output_node.h"
#include "audio_suite_process_node.h"
#include "audio_errors.h"
 
#include "audio_suite_input_node.h"
#include "audio_suite_space_render_node.h"
#include "audio_suite_output_node.h"
#include "audio_suite_pcm_buffer.h"
 
using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;
using namespace testing;
 
namespace {
 
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
 
std::string g_fileNameOne = "/data/audiosuite/sr/48000_2_16.pcm";
std::string g_outFilename = "/data/audiosuite/sr/out.pcm";
std::string g_basePositionFilename = "/data/audiosuite/sr/base_position.pcm";
std::string g_baseRotationFilename = "/data/audiosuite/sr/base_rotation.pcm";
std::string g_baseExtensionFilename = "/data/audiosuite/sr/base_extension.pcm";
 
class AudioSuiteSpaceRenderTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
 
    int32_t DoprocessTest(std::string inputFile, std::string outputFile, std::string name, std::string value);
 
    std::vector<uint8_t> ReadInputFile(std::string inputFile, size_t frameSizeInput);
 
    PcmBufferFormat outFormat_ = {SAMPLE_RATE_48000, STEREO, CH_LAYOUT_STEREO, SAMPLE_S16LE};
    std::unique_ptr<AudioSuitePcmBuffer> buffer = std::make_unique<AudioSuitePcmBuffer>(outFormat_);
};
 
void AudioSuiteSpaceRenderTest::SetUp()
{}
 
void AudioSuiteSpaceRenderTest::TearDown()
{}
 
std::vector<uint8_t> AudioSuiteSpaceRenderTest::ReadInputFile(std::string inputFile, size_t frameSizeInput)
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
 
int32_t AudioSuiteSpaceRenderTest::DoprocessTest(std::string inputFile, std::string outputFile,
    std::string name, std::string value)
{
    std::shared_ptr<AudioSuiteSpaceRenderNode> node = std::make_shared<AudioSuiteSpaceRenderNode>();
 
    node->Init();
    std::shared_ptr<MockInputNode> mockInputNode_ = std::make_shared<MockInputNode>();
    std::shared_ptr<OutputPort<AudioSuitePcmBuffer*>> inputNodeOutputPort =
        std::make_shared<OutputPort<AudioSuitePcmBuffer*>>(mockInputNode_);
    EXPECT_CALL(*mockInputNode_, GetOutputPort())
        .Times(1).WillRepeatedly(::testing::Return(inputNodeOutputPort));
    
    int32_t ret = node->SetOptions(name, value);
    CHECK_AND_RETURN_RET(ret == SUCCESS, ret);
    node->Connect(mockInputNode_);
    CHECK_AND_RETURN_RET(inputNodeOutputPort->GetInputNum() == 1, ERROR);
    std::shared_ptr<OutputPort<AudioSuitePcmBuffer*>> nodeOutputPort =
        node->GetOutputPort();
 
    size_t frameSizeInput = buffer->GetDataSize();
    CHECK_AND_RETURN_RET(frameSizeInput > 0, ERROR);
    // Read input file
    std::vector<uint8_t> inputfileBuffer = ReadInputFile(inputFile, frameSizeInput);
    CHECK_AND_RETURN_RET(inputfileBuffer.empty() == false, ERROR);
    std::ofstream outFile(outputFile, std::ios::binary | std::ios::out | std::ios::app);
 
    uint8_t *readPtr = inputfileBuffer.data();
    int32_t frames = inputfileBuffer.size() / frameSizeInput;
    int32_t frameIndex = 0;
    while (!node->GetAudioNodeDataFinishedFlag()) {
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
    CHECK_AND_RETURN_RET(inputNodeOutputPort->GetInputNum() == 0, ERROR);
    testing::Mock::VerifyAndClearExpectations(mockInputNode_.get());
    buffer->SetIsFinished(false);
    node->Flush();
    mockInputNode_.reset();
    inputNodeOutputPort.reset();
    return SUCCESS;
}
 
HWTEST_F(AudioSuiteSpaceRenderTest, SpaceRenderPositionParams001, TestSize.Level0)
{
    DoprocessTest(g_fileNameOne, g_outFilename, "AudioSpaceRenderPositionParams", "1,1,1");
 
    std::ifstream outFile(g_outFilename, std::ios::binary);
    std::ifstream baseFile(g_basePositionFilename, std::ios::binary);
 
    ASSERT_TRUE(outFile.is_open()) << "Failed to open out.pcm";
    ASSERT_TRUE(baseFile.is_open()) << "Failed to open base.pcm";
 
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
 
    EXPECT_EQ(out_data, base_data) << "out.pcm and base.pcm are not identical";
    std::remove(g_outFilename.c_str());
}
 
HWTEST_F(AudioSuiteSpaceRenderTest, SpaceRenderRotationParams001, TestSize.Level0)
{
    DoprocessTest(g_fileNameOne, g_outFilename, "AudioSpaceRenderRotationParams", "1,1,1,2,0");
 
    std::ifstream outFile(g_outFilename, std::ios::binary);
    std::ifstream baseFile(g_baseRotationFilename, std::ios::binary);
 
    ASSERT_TRUE(outFile.is_open()) << "Failed to open out.pcm";
    ASSERT_TRUE(baseFile.is_open()) << "Failed to open base.pcm";
 
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
 
    EXPECT_EQ(out_data, base_data) << "out.pcm and base.pcm are not identical";
    std::remove(g_outFilename.c_str());
}
 
HWTEST_F(AudioSuiteSpaceRenderTest, SpaceRenderExtensionParams001, TestSize.Level0)
{
    DoprocessTest(g_fileNameOne, g_outFilename, "AudioSpaceRenderExtensionParams", "2,90");
 
    std::ifstream outFile(g_outFilename, std::ios::binary);
    std::ifstream baseFile(g_baseExtensionFilename, std::ios::binary);
 
    ASSERT_TRUE(outFile.is_open()) << "Failed to open out.pcm";
    ASSERT_TRUE(baseFile.is_open()) << "Failed to open base.pcm";
 
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
 
    EXPECT_EQ(out_data, base_data) << "out.pcm and base.pcm are not identical";
    std::remove(g_outFilename.c_str());
}
 
HWTEST_F(AudioSuiteSpaceRenderTest, SpaceRenderSetParameterParams001, TestSize.Level0)
{
    AudioSuiteSpaceRenderNode spacerender;
    spacerender.Init();
    int32_t ret = spacerender.SetOptions("test", "1,1,1");
    EXPECT_EQ(ret, ERROR);
}
 
HWTEST_F(AudioSuiteSpaceRenderTest, SpaceRenderGetParameterParams001, TestSize.Level0)
{
    std::string paramValue;
    int32_t ret;
 
    AudioSuiteSpaceRenderNode spacerender;
 
    ret = spacerender.Init();
    EXPECT_EQ(SUCCESS, ret);
 
    ret = spacerender.Init();
    EXPECT_EQ(ERROR, ret);
 
    spacerender.SetOptions("AudioSpaceRenderPositionParams", "1,1,1");
    ret = spacerender.GetOptions("AudioSpaceRenderPositionParams", paramValue);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ('1', paramValue[0]);
 
    spacerender.SetOptions("AudioSpaceRenderRotationParams", "1,1,1,2,0");
    ret = spacerender.GetOptions("AudioSpaceRenderRotationParams", paramValue);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ('1', paramValue[0]);
 
    spacerender.SetOptions("AudioSpaceRenderExtensionParams", "2,90");
    ret = spacerender.GetOptions("AudioSpaceRenderExtensionParams", paramValue);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ('2', paramValue[0]);
 
    ret = spacerender.GetOptions("test", paramValue);
    EXPECT_EQ(ret, ERROR);
 
    ret = spacerender.DeInit();
    EXPECT_EQ(SUCCESS, ret);
 
    ret = spacerender.DeInit();
    EXPECT_EQ(ERROR, ret);
}
 
HWTEST_F(AudioSuiteSpaceRenderTest, SpaceRenderDoProcess001, TestSize.Level0)
{
    std::shared_ptr<AudioSuiteSpaceRenderNode> node = std::make_shared<AudioSuiteSpaceRenderNode>();
 
    node->SetAudioNodeDataFinishedFlag(true);
    int32_t ret = node->DoProcess();
    EXPECT_EQ(SUCCESS, ret);
 
    node->SetAudioNodeDataFinishedFlag(false);
    node->outputStream_ = nullptr;
    node->inputStream_ = nullptr;
    ret = node->DoProcess();
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}
}  // namespace