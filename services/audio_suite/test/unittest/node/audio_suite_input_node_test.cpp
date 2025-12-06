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
#include "audio_suite_input_node.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;
using namespace testing;

class InputNodeRequestDataCallBack;
namespace {

static constexpr uint32_t TEST_CACHE_SIZE1 = 882;

class AudioSuiteInputNodeTest : public testing::Test {
public:
    void SetUp() {};
    void TearDown() {};
    AudioFormat GetTestAudioFormat()
    {
        AudioFormat audioFormat;
        audioFormat.audioChannelInfo.numChannels = 1;
        audioFormat.format = AudioSampleFormat::SAMPLE_S16LE;
        audioFormat.rate = AudioSamplingRate::SAMPLE_RATE_44100;
        return audioFormat;
    }
};

class SuiteInputNodeRequestDataCallBackTest : public AudioSuite::InputNodeRequestDataCallBack {
    int32_t OnRequestDataCallBack(void *audioData, int32_t audioDataSize, bool* finished) override
    {
        std::vector<uint8_t> data;
        data.assign(TEST_CACHE_SIZE1, 0);
        if (memcpy_s(audioData, audioDataSize, data.data(), TEST_CACHE_SIZE1) != 0) {
            return -1;
        }
        *finished = true;
        return TEST_CACHE_SIZE1;
    }
};

class SuiteInputNodeRequestDataCallBackTestErr : public AudioSuite::InputNodeRequestDataCallBack {
    int32_t OnRequestDataCallBack(void *audioData, int32_t audioDataSize, bool* finished) override
    {
        return 0;
    }
};

class SuiteNodeReadTapDataCallbackTest : public AudioSuite::SuiteNodeReadTapDataCallback {
    void OnReadTapDataCallback(void *audioData, int32_t audioDataSize) override
    {
        return;
    }
};

HWTEST_F(AudioSuiteInputNodeTest, AudioSuiteInputNodeConstructor_001, TestSize.Level0)
{
    AudioFormat audioFormat = GetTestAudioFormat();
    std::shared_ptr<AudioInputNode> inputNode = std::make_shared<AudioInputNode>(audioFormat);
    EXPECT_NE(inputNode, nullptr);
}

HWTEST_F(AudioSuiteInputNodeTest, AudioSuiteInputNodeConnect_001, TestSize.Level0)
{
    AudioFormat audioFormat = GetTestAudioFormat();
    std::shared_ptr<AudioInputNode> inputNode = std::make_shared<AudioInputNode>(audioFormat);
    EXPECT_NE(inputNode, nullptr);

    auto ret = inputNode->Connect(inputNode);
    EXPECT_EQ(ret, ERROR);

    ret = inputNode->DisConnect(inputNode);
    EXPECT_EQ(ret, ERROR);

    ret = inputNode->Init();
    EXPECT_EQ(ret, SUCCESS);

    ret = inputNode->DeInit();
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuiteInputNodeTest, AudioSuiteInputNodeInit_001, TestSize.Level0)
{
    AudioFormat audioFormat = GetTestAudioFormat();
    std::shared_ptr<AudioInputNode> inputNode = std::make_shared<AudioInputNode>(audioFormat);
    EXPECT_NE(inputNode, nullptr);

    auto ret = inputNode->Init();
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuiteInputNodeTest, AudioSuiteInputNodeSetFormat_001, TestSize.Level0)
{
    AudioFormat audioFormat = GetTestAudioFormat();
    std::shared_ptr<AudioInputNode> inputNode = std::make_shared<AudioInputNode>(audioFormat);
    EXPECT_NE(inputNode, nullptr);
    inputNode->Init();
    audioFormat.format = AudioSampleFormat::SAMPLE_S16LE;
    audioFormat.audioChannelInfo.numChannels = 2;
    inputNode->SetAudioNodeFormat(audioFormat);
}

HWTEST_F(AudioSuiteInputNodeTest, AudioSuiteInputNodeFlush_001, TestSize.Level0)
{
    AudioFormat audioFormat = GetTestAudioFormat();
    std::shared_ptr<AudioInputNode> inputNode = std::make_shared<AudioInputNode>(audioFormat);
    EXPECT_NE(inputNode, nullptr);
    inputNode->Init();

    auto ret = inputNode->Flush();
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuiteInputNodeTest, AudioSuiteInputNodeGetOutputPort_001, TestSize.Level0)
{
    AudioFormat audioFormat = GetTestAudioFormat();
    std::shared_ptr<AudioInputNode> inputNode = std::make_shared<AudioInputNode>(audioFormat);
    EXPECT_NE(inputNode, nullptr);
    inputNode->Init();

    std::shared_ptr<OutputPort<AudioSuitePcmBuffer*>> outport =
        inputNode->GetOutputPort();
    EXPECT_NE(outport, nullptr);
}

HWTEST_F(AudioSuiteInputNodeTest, AudioSuiteInputNodeSetRequestDataCallback_001, TestSize.Level0)
{
    AudioFormat audioFormat = GetTestAudioFormat();
    std::shared_ptr<AudioInputNode> inputNode = std::make_shared<AudioInputNode>(audioFormat);
    EXPECT_NE(inputNode, nullptr);
    inputNode->Init();

    auto ret = inputNode->SetRequestDataCallback(nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    std::shared_ptr<SuiteInputNodeRequestDataCallBackTest> testCallback =
        std::make_shared<SuiteInputNodeRequestDataCallBackTest>();
    ret = inputNode->SetRequestDataCallback(testCallback);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuiteInputNodeTest, AudioSuiteInputNodeGetDataFromUser_001, TestSize.Level0)
{
    AudioFormat audioFormat = GetTestAudioFormat();
    std::shared_ptr<AudioInputNode> inputNode = std::make_shared<AudioInputNode>(audioFormat);
    EXPECT_NE(inputNode, nullptr);
    auto ret = inputNode->Init();
    ASSERT_EQ(ret, SUCCESS);

    ret = inputNode->GetDataFromUser();
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    std::shared_ptr<SuiteInputNodeRequestDataCallBackTest> testCallback =
        std::make_shared<SuiteInputNodeRequestDataCallBackTest>();
    inputNode->SetRequestDataCallback(testCallback);

    inputNode->SetAudioNodeDataFinishedFlag(true);
    ret = inputNode->GetDataFromUser();
    EXPECT_EQ(ret, SUCCESS);

    inputNode->SetAudioNodeDataFinishedFlag(false);
    inputNode->cachedBuffer_.size_ = inputNode->cachedBuffer_.capacity_;
    ret = inputNode->GetDataFromUser();
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuiteInputNodeTest, AudioSuiteInputNodeGetDataFromUser_002, TestSize.Level0)
{
    AudioFormat audioFormat = GetTestAudioFormat();
    std::shared_ptr<AudioInputNode> inputNode = std::make_shared<AudioInputNode>(audioFormat);
    EXPECT_NE(inputNode, nullptr);
    auto ret = inputNode->Init();
    ASSERT_EQ(ret, SUCCESS);
    
    std::shared_ptr<SuiteInputNodeRequestDataCallBackTestErr> testCallback =
        std::make_shared<SuiteInputNodeRequestDataCallBackTestErr>();
    ret = inputNode->SetRequestDataCallback(testCallback);
    EXPECT_EQ(ret, SUCCESS);

    inputNode->singleRequestSize_ = inputNode->cachedBuffer_.GetRestSpace() + 1;
    ret = inputNode->GetDataFromUser();
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuiteInputNodeTest, AudioSuiteInputNodeGetDataFromUser_003, TestSize.Level0)
{
    AudioFormat audioFormat = GetTestAudioFormat();
    std::shared_ptr<AudioInputNode> inputNode = std::make_shared<AudioInputNode>(audioFormat);
    EXPECT_NE(inputNode, nullptr);
    auto ret = inputNode->Init();
    ASSERT_EQ(ret, SUCCESS);
    
    std::shared_ptr<SuiteInputNodeRequestDataCallBackTest> testCallback =
        std::make_shared<SuiteInputNodeRequestDataCallBackTest>();
    ret = inputNode->SetRequestDataCallback(testCallback);
    EXPECT_EQ(ret, SUCCESS);

    ret = inputNode->GetDataFromUser();
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuiteInputNodeTest, AudioSuiteInputNodeGetDataFromUser_004, TestSize.Level0)
{
    AudioFormat audioFormat = GetTestAudioFormat();
    audioFormat.rate = AudioSamplingRate::SAMPLE_RATE_11025;
    std::shared_ptr<AudioInputNode> inputNode = std::make_shared<AudioInputNode>(audioFormat);
    EXPECT_NE(inputNode, nullptr);
    inputNode->Init();
    
    std::shared_ptr<SuiteInputNodeRequestDataCallBackTest> testCallback =
        std::make_shared<SuiteInputNodeRequestDataCallBackTest>();
    auto ret = inputNode->SetRequestDataCallback(testCallback);
    EXPECT_EQ(ret, SUCCESS);

    ret = inputNode->GetDataFromUser();
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuiteInputNodeTest, AudioSuiteInputNodeGeneratePushBuffer_001, TestSize.Level0)
{
    AudioFormat audioFormat = GetTestAudioFormat();
    std::shared_ptr<AudioInputNode> inputNode = std::make_shared<AudioInputNode>(audioFormat);
    EXPECT_NE(inputNode, nullptr);
    inputNode->Init();
    inputNode->cachedBuffer_.ResizeBuffer(10);
    std::vector<uint8_t> data(10);
    inputNode->cachedBuffer_.PushData(data.data(), 10);
    auto ret = inputNode->GeneratePushBuffer();
    EXPECT_EQ(ret, ERROR);
}

HWTEST_F(AudioSuiteInputNodeTest, AudioSuiteInputNodeDoProcess_001, TestSize.Level0)
{
    AudioFormat audioFormat = GetTestAudioFormat();
    std::shared_ptr<AudioInputNode> inputNode = std::make_shared<AudioInputNode>(audioFormat);
    EXPECT_NE(inputNode, nullptr);
    inputNode->Init();

    auto ret = inputNode->DoProcess();
    EXPECT_EQ(ret, ERR_WRITE_FAILED);

    std::shared_ptr<SuiteInputNodeRequestDataCallBackTest> testCallback =
        std::make_shared<SuiteInputNodeRequestDataCallBackTest>();
    inputNode->SetRequestDataCallback(testCallback);
    ret = inputNode->DoProcess();
    EXPECT_EQ(ret, SUCCESS);
}

}