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

const uint32_t AUDIO_DATA_SIZE = 1024;
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