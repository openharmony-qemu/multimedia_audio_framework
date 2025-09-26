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
#include <unistd.h>
#include <gtest/gtest.h>
#include "audio_errors.h"
#include "audio_suite_output_node.h"
#include "audio_suite_input_node.h"
#include "audio_suite_pcm_buffer.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

const uint32_t AUDIO_DATA_SIZE = 1024;
const uint32_t CACHE_BUFFER_SIZE = 1024;

class AudioSuiteOutputNodeTest : public testing::Test {
public:
    void SetUp() {};
    void TearDown() {};
};

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
    AudioFormat format;
    std::shared_ptr<AudioOutputNode> outputNode = std::make_shared<AudioOutputNode>(format);
    EXPECT_NE(outputNode, nullptr);

    std::shared_ptr<AudioInputNode> inputNode = std::make_shared<AudioInputNode>(format);
    EXPECT_NE(inputNode, nullptr);
    inputNode->Init();
    AudioNodePortType type = AUDIO_NODE_DEFAULT_OUTPORT_TYPE;
    std::unique_ptr<AudioSuitePcmBuffer> data = std::make_unique<AudioSuitePcmBuffer>(
        48000, 2, AudioChannelLayout::CH_LAYOUT_STEREO);
    inputNode->GetOutputPort(type).get()->outputData_.push_back(data.get());
    outputNode->Connect(inputNode, type);
    auto ret = outputNode->DoProcess();
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuiteOutputNodeTest, DoProcess_003, TestSize.Level0)
{
    AudioFormat format;
    std::shared_ptr<AudioOutputNode> outputNode = std::make_shared<AudioOutputNode>(format);
    EXPECT_NE(outputNode, nullptr);

    uint8_t *audioData = new uint8_t[AUDIO_DATA_SIZE];
    int32_t frameSize = 128;
    bool finished = false;
    int32_t writeDataSize = 0;
    outputNode->SetAudioNodeDataFinishedFlag(true);
    int32_t ret = outputNode->DoProcess(audioData, frameSize, &writeDataSize, &finished);
    EXPECT_EQ(ret, ERROR);
}

HWTEST_F(AudioSuiteOutputNodeTest, DoProcess_004, TestSize.Level0)
{
    AudioFormat format;
    std::shared_ptr<AudioOutputNode> outputNode = std::make_shared<AudioOutputNode>(format);
    EXPECT_NE(outputNode, nullptr);

    std::vector<uint8_t> cacheBuffer(CACHE_BUFFER_SIZE);
    outputNode->SetCacheBuffer(cacheBuffer);

    uint8_t *audioData = new uint8_t[AUDIO_DATA_SIZE];
    int32_t frameSize = 128;
    bool finished = false;
    int32_t writeDataSize = 0;
    int32_t ret = outputNode->DoProcess(audioData, frameSize, &writeDataSize, &finished);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuiteOutputNodeTest, DoProcess_005, TestSize.Level0)
{
    AudioFormat format;
    std::shared_ptr<AudioOutputNode> outputNode = std::make_shared<AudioOutputNode>(format);
    EXPECT_NE(outputNode, nullptr);

    uint8_t *audioData = new uint8_t[AUDIO_DATA_SIZE];
    int32_t frameSize = 0;
    bool finished = false;
    int32_t writeDataSize = 0;
    int32_t ret = outputNode->DoProcess(audioData, frameSize, &writeDataSize, &finished);
    EXPECT_EQ(ret, ERROR);

    frameSize = 128;
    ret = outputNode->DoProcess(audioData, frameSize, &writeDataSize, &finished);
    EXPECT_EQ(ret, ERROR);
}

HWTEST_F(AudioSuiteOutputNodeTest, SetCacheBuffer_001, TestSize.Level0)
{
    AudioFormat format;
    std::shared_ptr<AudioOutputNode> outputNode = std::make_shared<AudioOutputNode>(format);
    EXPECT_NE(outputNode, nullptr);

    std::vector<uint8_t> cacheBuffer = {128};
    outputNode->SetCacheBuffer(cacheBuffer);
    EXPECT_EQ(outputNode->cacheBuffer_[0], 128);
}

HWTEST_F(AudioSuiteOutputNodeTest, GetCacheBuffer_001, TestSize.Level0)
{
    AudioFormat format;
    std::shared_ptr<AudioOutputNode> outputNode = std::make_shared<AudioOutputNode>(format);
    EXPECT_NE(outputNode, nullptr);

    std::vector<uint8_t> cacheBuffer = outputNode->GetCacheBuffer();
    EXPECT_EQ(cacheBuffer.size(), 0);
}

HWTEST_F(AudioSuiteOutputNodeTest, GetProcessedAudioData_001, TestSize.Level0)
{
    AudioFormat format;
    std::shared_ptr<AudioOutputNode> outputNode = std::make_shared<AudioOutputNode>(format);
    EXPECT_NE(outputNode, nullptr);

    size_t bytes = 0;
    uint8_t* ret = outputNode->GetProcessedAudioData(bytes);
    EXPECT_EQ(ret, nullptr);
}

HWTEST_F(AudioSuiteOutputNodeTest, GetProcessedAudioData_002, TestSize.Level0)
{
    AudioFormat format;
    std::shared_ptr<AudioOutputNode> outputNode = std::make_shared<AudioOutputNode>(format);
    EXPECT_NE(outputNode, nullptr);

    size_t bytes = 0;
    AudioSuitePcmBuffer* pcmBuff = new AudioSuitePcmBuffer(48000, 2, CH_LAYOUT_MONO);
    outputNode->inputStream_.inputData_.push_back(pcmBuff);
    uint8_t* ret = outputNode->GetProcessedAudioData(bytes);
    EXPECT_NE(ret, nullptr);
}

HWTEST_F(AudioSuiteOutputNodeTest, CopyDataFromCache_001, TestSize.Level0)
{
    AudioFormat format;
    std::shared_ptr<AudioOutputNode> outputNode = std::make_shared<AudioOutputNode>(format);
    EXPECT_NE(outputNode, nullptr);

    uint8_t *audioData = new uint8_t[AUDIO_DATA_SIZE];
    int32_t frameSize = 128;
    int32_t audioDataOffset = 0;
    bool finished = false;
    auto ret = outputNode->CopyDataFromCache(audioData, frameSize, audioDataOffset, &finished);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuiteOutputNodeTest, CopyDataFromCache_002, TestSize.Level0)
{
    AudioFormat format;
    std::shared_ptr<AudioOutputNode> outputNode = std::make_shared<AudioOutputNode>(format);
    EXPECT_NE(outputNode, nullptr);

    std::vector<uint8_t> cacheBuffer(CACHE_BUFFER_SIZE);
    outputNode->SetCacheBuffer(cacheBuffer);

    uint8_t *audioData = new uint8_t[AUDIO_DATA_SIZE];
    int32_t frameSize = 128;
    int32_t audioDataOffset = 0;
    bool finished = false;
    auto ret = outputNode->CopyDataFromCache(audioData, frameSize, audioDataOffset, &finished);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuiteOutputNodeTest, CopyDataFromCache_003, TestSize.Level0)
{
    AudioFormat format;
    std::shared_ptr<AudioOutputNode> outputNode = std::make_shared<AudioOutputNode>(format);
    EXPECT_NE(outputNode, nullptr);

    std::vector<uint8_t> cacheBuffer(CACHE_BUFFER_SIZE);
    outputNode->SetCacheBuffer(cacheBuffer);

    uint8_t *audioData = new uint8_t[AUDIO_DATA_SIZE];
    int32_t frameSize = 1024;
    int32_t audioDataOffset = 0;
    bool finished = false;
    auto ret = outputNode->CopyDataFromCache(audioData, frameSize, audioDataOffset, &finished);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuiteOutputNodeTest, CopyDataFromCache_004, TestSize.Level0)
{
    AudioFormat format;
    std::shared_ptr<AudioOutputNode> outputNode = std::make_shared<AudioOutputNode>(format);
    EXPECT_NE(outputNode, nullptr);

    std::vector<uint8_t> cacheBuffer(CACHE_BUFFER_SIZE);
    outputNode->SetCacheBuffer(cacheBuffer);

    uint8_t *audioData = new uint8_t[AUDIO_DATA_SIZE];
    int32_t frameSize = 1024;
    int32_t audioDataOffset = 0;
    bool finished = false;
    auto ret = outputNode->CopyDataFromCache(audioData, frameSize, audioDataOffset, &finished);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuiteOutputNodeTest, FillRemainingAudioData_001, TestSize.Level0)
{
    AudioFormat format;
    std::shared_ptr<AudioOutputNode> outputNode = std::make_shared<AudioOutputNode>(format);
    EXPECT_NE(outputNode, nullptr);

    std::vector<uint8_t> cacheBuffer(CACHE_BUFFER_SIZE);
    outputNode->SetCacheBuffer(cacheBuffer);

    uint8_t *audioData = new uint8_t[AUDIO_DATA_SIZE];
    int32_t remainingBytes = 128;
    int32_t writeDataSize = 0;
    bool finished = false;
    int32_t frameSize = 1024;
    outputNode->audioNodeInfo_.finishedFlag = true;
    auto ret = outputNode->FillRemainingAudioData(audioData, remainingBytes, &writeDataSize, &finished, frameSize);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuiteOutputNodeTest, FillRemainingAudioData_002, TestSize.Level0)
{
    AudioFormat format;
    std::shared_ptr<AudioOutputNode> outputNode = std::make_shared<AudioOutputNode>(format);
    EXPECT_NE(outputNode, nullptr);

    uint8_t *audioData = new uint8_t[AUDIO_DATA_SIZE];
    int32_t remainingBytes = 128;
    int32_t writeDataSize = 0;
    bool finished = false;
    int32_t frameSize = 1024;
    auto ret = outputNode->FillRemainingAudioData(audioData, remainingBytes, &writeDataSize, &finished, frameSize);
    EXPECT_EQ(ret, ERROR);

    std::shared_ptr<AudioInputNode> inputNode = std::make_shared<AudioInputNode>(format);
    EXPECT_NE(inputNode, nullptr);
    inputNode->Init();
    std::unique_ptr<AudioSuitePcmBuffer> data =
        std::make_unique<AudioSuitePcmBuffer>(48000, 0, AudioChannelLayout::CH_LAYOUT_STEREO);
    AudioNodePortType type = AUDIO_NODE_DEFAULT_OUTPORT_TYPE;
    inputNode->GetOutputPort(type).get()->outputData_.push_back(data.get());
    outputNode->Connect(inputNode, type);
    ret = outputNode->FillRemainingAudioData(audioData, remainingBytes, &writeDataSize, &finished, frameSize);
    EXPECT_EQ(ret, ERROR);
}

HWTEST_F(AudioSuiteOutputNodeTest, FillRemainingAudioData_003, TestSize.Level0)
{
    AudioFormat format;
    std::shared_ptr<AudioOutputNode> outputNode = std::make_shared<AudioOutputNode>(format);
    EXPECT_NE(outputNode, nullptr);

    std::shared_ptr<AudioInputNode> inputNode = std::make_shared<AudioInputNode>(format);
    EXPECT_NE(inputNode, nullptr);
    inputNode->Init();
    AudioNodePortType type = AUDIO_NODE_DEFAULT_OUTPORT_TYPE;
    std::unique_ptr<AudioSuitePcmBuffer> data =
        std::make_unique<AudioSuitePcmBuffer>(48000, 2, AudioChannelLayout::CH_LAYOUT_STEREO);
    inputNode->GetOutputPort(type).get()->outputData_.push_back(data.get());
    outputNode->Connect(inputNode, type);

    uint8_t *audioData = new uint8_t[AUDIO_DATA_SIZE];
    int32_t remainingBytes = 128;
    int32_t *writeDataSize = new int32_t;
    bool finished = false;
    int32_t frameSize = 1024;
    auto ret = outputNode->FillRemainingAudioData(audioData, remainingBytes, writeDataSize, &finished, frameSize);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuiteOutputNodeTest, FillRemainingAudioData_004, TestSize.Level0)
{
    AudioFormat format;
    std::shared_ptr<AudioOutputNode> outputNode = std::make_shared<AudioOutputNode>(format);
    EXPECT_NE(outputNode, nullptr);

    std::shared_ptr<AudioInputNode> inputNode = std::make_shared<AudioInputNode>(format);
    EXPECT_NE(inputNode, nullptr);
    inputNode->Init();
    AudioNodePortType type = AUDIO_NODE_DEFAULT_OUTPORT_TYPE;
    std::unique_ptr<AudioSuitePcmBuffer> data_1 =
        std::make_unique<AudioSuitePcmBuffer>(48000, 2, AudioChannelLayout::CH_LAYOUT_STEREO);
    inputNode->GetOutputPort(type).get()->outputData_.push_back(data_1.get());
    std::unique_ptr<AudioSuitePcmBuffer> data_2 =
        std::make_unique<AudioSuitePcmBuffer>(48000, 2, AudioChannelLayout::CH_LAYOUT_STEREO);
    inputNode->GetOutputPort(type).get()->outputData_.push_back(data_2.get());
    outputNode->Connect(inputNode, type);

    uint8_t *audioData = new uint8_t[AUDIO_DATA_SIZE];
    int32_t remainingBytes = 15360;
    int32_t *writeDataSize = new int32_t;
    bool finished = false;
    int32_t frameSize = 1024;
    auto ret = outputNode->FillRemainingAudioData(audioData, remainingBytes, writeDataSize, &finished, frameSize);
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
    EXPECT_EQ(ret, ERROR);
}

HWTEST_F(AudioSuiteOutputNodeTest, RemoveTap_001, TestSize.Level0)
{
    AudioFormat format;
    std::shared_ptr<AudioOutputNode> outputNode = std::make_shared<AudioOutputNode>(format);
    EXPECT_NE(outputNode, nullptr);

    AudioNodePortType portType = AUDIO_NODE_DEFAULT_OUTPORT_TYPE;
    auto ret = outputNode->RemoveTap(portType);
    EXPECT_EQ(ret, ERROR);
}

}
}
}  // namespace