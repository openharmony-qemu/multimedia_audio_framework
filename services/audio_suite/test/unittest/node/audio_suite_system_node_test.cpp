/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
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
#include "audio_suite_unittest_tools.h"
#include "audio_suite_eq_node.h"
#include "audio_suite_system_music_master_node.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;
using namespace testing;
using namespace std;

namespace {

static const AudioFormat TEST_FORMAT = {
    {CH_LAYOUT_STEREO, STEREO}, SAMPLE_S16LE, SAMPLE_RATE_48000,
    {CH_LAYOUT_STEREO, STEREO}, SAMPLE_S16LE, SAMPLE_RATE_48000
};

static constexpr int32_t TEST_BUFFER_SIZE = 1024;

class MockOfflineAudioEffectChain : public OfflineAudioEffectChain {
public:
    MOCK_METHOD(int32_t, Configure, (const AudioStreamInfo &inInfo, const AudioStreamInfo &outInfo), (override));
    MOCK_METHOD(int32_t, SetParam, (const std::vector<uint8_t> &param), (override));
    MOCK_METHOD(int32_t, GetParam, (std::vector<uint8_t> &param), (override));
    MOCK_METHOD(int32_t, Prepare, (), (override));
    MOCK_METHOD(int32_t, GetEffectBufferSize, (uint32_t &inBufferSize, uint32_t &outBufferSize), (override));
    MOCK_METHOD(int32_t, Process, (uint8_t *inBuffer, int32_t inSize, uint8_t *outBuffer, int32_t outSize), (override));
    MOCK_METHOD(void, Release, (), (override));
};

class AudioSuiteSystemNodeTest : public testing::Test {
public:
    void SetUp() override
    {
        if (!AllNodeTypesSupported()) {
            GTEST_SKIP() << "not support all node types, skip this test";
        }
        node_ = std::make_shared<AudioSuiteSystemMusicMasterNode>();
    }
    void TearDown() override
    {
        node_ = nullptr;
    }

    void AssignMockEffectChain()
    {
        auto mock = std::make_unique<MockOfflineAudioEffectChain>();
        mockChain_ = mock.get();
        node_->effectChain_ = std::move(mock);
    }

    void SetupNodeWithFormatsAndChain()
    {
        AssignMockEffectChain();
        EXPECT_CALL(*mockChain_, Configure(_, _)).WillOnce(Return(SUCCESS));
        EXPECT_CALL(*mockChain_, Prepare()).WillOnce(Return(SUCCESS));
        EXPECT_CALL(*mockChain_, GetEffectBufferSize(_, _))
            .WillOnce(DoAll(SetArgReferee<0>(TEST_BUFFER_SIZE), SetArgReferee<1>(TEST_BUFFER_SIZE), Return(SUCCESS)));

        node_->hasInputFormat_ = true;
        node_->hasOutputFormat_ = true;
        node_->inputFormat_ = TEST_FORMAT;
        node_->outputFormat_ = TEST_FORMAT;
        node_->TryConfigure();
    }

    std::shared_ptr<AudioSuiteSystemMusicMasterNode> node_;
    MockOfflineAudioEffectChain *mockChain_ = nullptr;
};

HWTEST_F(AudioSuiteSystemNodeTest, SystemNodeDeInit, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuiteSystemMusicMasterNode>();
    int32_t ret = node_->DeInit();
    EXPECT_EQ(ret, SUCCESS);

    node_ = std::make_shared<AudioSuiteSystemMusicMasterNode>();
    AssignMockEffectChain();
    EXPECT_CALL(*mockChain_, Release()).Times(1);
    ret = node_->DeInit();
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(node_->effectChain_, nullptr);
}

HWTEST_F(AudioSuiteSystemNodeTest, ConfigureEffectChain, TestSize.Level0)
{
    node_->effectChain_ = nullptr;
    int32_t ret = node_->ConfigureEffectChain();
    EXPECT_EQ(ret, ERROR);

    AssignMockEffectChain();
    EXPECT_CALL(*mockChain_, Configure(_, _)).WillOnce(Return(ERR_INVALID_PARAM));
    ret = node_->ConfigureEffectChain();
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST_F(AudioSuiteSystemNodeTest, PrepareEffectChain, TestSize.Level0)
{
    node_->effectChain_ = nullptr;
    int32_t ret = node_->PrepareEffectChain();
    EXPECT_EQ(ret, ERROR);

    AssignMockEffectChain();
    EXPECT_CALL(*mockChain_, Prepare()).WillOnce(Return(ERR_INVALID_PARAM));
    ret = node_->PrepareEffectChain();
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    AssignMockEffectChain();
    EXPECT_CALL(*mockChain_, Prepare()).WillOnce(Return(SUCCESS));
    ret = node_->PrepareEffectChain();
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuiteSystemNodeTest, SetSystemParam, TestSize.Level0)
{
    node_->effectChain_ = nullptr;
    std::vector<uint8_t> param = {1, 2, 3};
    int32_t ret = node_->SetSystemParam(param);
    EXPECT_EQ(ret, ERROR);

    AssignMockEffectChain();
    std::vector<uint8_t> paramEmpty;
    ret = node_->SetSystemParam(paramEmpty);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    AssignMockEffectChain();
    EXPECT_CALL(*mockChain_, SetParam(_)).WillOnce(Return(ERR_INVALID_PARAM));
    std::vector<uint8_t> paramInvalid = {1, 2, 3};
    ret = node_->SetSystemParam(paramInvalid);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    AssignMockEffectChain();
    EXPECT_CALL(*mockChain_, SetParam(_)).WillOnce(Return(SUCCESS));
    ret = node_->SetSystemParam(param);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuiteSystemNodeTest, GetSystemParam, TestSize.Level0)
{
    node_->effectChain_ = nullptr;
    std::vector<uint8_t> param;
    int32_t ret = node_->GetSystemParam(param);
    EXPECT_EQ(ret, ERROR);

    AssignMockEffectChain();
    EXPECT_CALL(*mockChain_, GetParam(_)).WillOnce(Return(ERR_INVALID_PARAM));
    std::vector<uint8_t> paramInvalid;
    ret = node_->GetSystemParam(paramInvalid);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    AssignMockEffectChain();
    EXPECT_CALL(*mockChain_, GetParam(_)).WillOnce(Return(SUCCESS));
    ret = node_->GetSystemParam(param);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuiteSystemNodeTest, GetInOutSize, TestSize.Level0)
{
    node_->effectChain_ = nullptr;
    uint32_t inSize = 0;
    uint32_t outSize = 0;
    int32_t ret = node_->GetInOutSize(inSize, outSize);
    EXPECT_EQ(ret, ERROR);

    AssignMockEffectChain();
    EXPECT_CALL(*mockChain_, GetEffectBufferSize(_, _)).WillOnce(Return(ERR_INVALID_PARAM));
    ret = node_->GetInOutSize(inSize, outSize);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    AssignMockEffectChain();
    EXPECT_CALL(*mockChain_, GetEffectBufferSize(_, _))
        .WillOnce(DoAll(SetArgReferee<0>(TEST_BUFFER_SIZE), SetArgReferee<1>(TEST_BUFFER_SIZE), Return(SUCCESS)));
    ret = node_->GetInOutSize(inSize, outSize);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(inSize, TEST_BUFFER_SIZE);
    EXPECT_EQ(outSize, TEST_BUFFER_SIZE);
}

HWTEST_F(AudioSuiteSystemNodeTest, Connect, TestSize.Level0)
{
    int32_t ret = node_->Connect(nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    std::shared_ptr<AudioNode> eqNode = std::make_shared<AudioSuiteEqNode>();
    ret = node_->Connect(eqNode);
    EXPECT_EQ(ret, ERR_AUDIO_SUITE_UNSUPPORT_CONNECT);
}

HWTEST_F(AudioSuiteSystemNodeTest, UpdateFormats, TestSize.Level0)
{
    node_->effectChain_ = nullptr;
    node_->UpdateInputFormat(TEST_FORMAT);
    EXPECT_TRUE(node_->hasInputFormat_);

    node_->UpdateOutputFormat(TEST_FORMAT);
    EXPECT_TRUE(node_->hasOutputFormat_);
}

HWTEST_F(AudioSuiteSystemNodeTest, SignalProcess_EmptyInputs, TestSize.Level0)
{
    node_->effectChain_ = nullptr;
    std::vector<AudioSuitePcmBuffer*> inputs;
    auto result = node_->SignalProcess(inputs);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], nullptr);
}

HWTEST_F(AudioSuiteSystemNodeTest, SignalProcess_NullInput, TestSize.Level0)
{
    node_->effectChain_ = nullptr;
    std::vector<AudioSuitePcmBuffer*> inputs = { nullptr };
    auto result = node_->SignalProcess(inputs);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], nullptr);
}

HWTEST_F(AudioSuiteSystemNodeTest, SignalProcess_ProcessFail, TestSize.Level0)
{
    SetupNodeWithFormatsAndChain();
    node_->InitOutputStream();

    PcmBufferFormat fmt = node_->GetAudioNodeInPcmFormat();
    AudioSuitePcmBuffer buffer(fmt);
    std::vector<AudioSuitePcmBuffer*> inputs = { &buffer };
    auto result = node_->SignalProcess(inputs);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], nullptr);
}

HWTEST_F(AudioSuiteSystemNodeTest, SignalProcess_Success, TestSize.Level0)
{
    SetupNodeWithFormatsAndChain();
    node_->InitOutputStream();
    EXPECT_CALL(*mockChain_, Process(_, _, _, _)).WillOnce(Return(SUCCESS));
    PcmBufferFormat fmt = node_->GetAudioNodeInPcmFormat();
    AudioSuitePcmBuffer buffer(fmt);
    AudioSuitePcmBuffer algoOutbuffer(fmt);
    node_->algoOutPcmBuffer_.resize(1);
    std::vector<AudioSuitePcmBuffer*> inputs = { &buffer };
    auto result = node_->SignalProcess(inputs);
    EXPECT_EQ(result.size(), 0);
}

HWTEST_F(AudioSuiteSystemNodeTest, DoProcess_DataFinished, TestSize.Level0)
{
    AudioMetaFrame frame;
    int32_t audioSize = 0;
    int32_t metaSize = 0;
    bool finished = false;
    node_->SetAudioNodeDataFinishedFlag(true);
    int32_t ret = node_->DoProcess(frame, &audioSize, &metaSize, &finished);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

HWTEST_F(AudioSuiteSystemNodeTest, DoProcess_NullChain, TestSize.Level0)
{
    AudioMetaFrame frame;
    int32_t audioSize = 0;
    int32_t metaSize = 0;
    bool finished = false;
    int32_t ret = node_->DoProcess(frame, &audioSize, &metaSize, &finished);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

HWTEST_F(AudioSuiteSystemNodeTest, DoProcess_NoPreNodes, TestSize.Level0)
{
    AssignMockEffectChain();
    uint8_t audioBuf[16] = {0};
    uint8_t metaBuf[16] = {0};
    AudioMetaFrame frame;
    frame.audioData = audioBuf;
    frame.audioDataSize = sizeof(audioBuf);
    frame.metaData = metaBuf;
    frame.metaDataSize = sizeof(metaBuf);
    int32_t audioSize = 0;
    int32_t metaSize = 0;
    bool finished = false;
    int32_t ret = node_->DoProcess(frame, &audioSize, &metaSize, &finished);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

HWTEST_F(AudioSuiteSystemNodeTest, Init_EmptyChainName, TestSize.Level0)
{
    node_->nodeName_ = "";
    int32_t ret = node_->Init();
    EXPECT_EQ(ret, ERROR);

    node_->nodeName_ = "test";
    ret = node_->Init();
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

}  // namespace
