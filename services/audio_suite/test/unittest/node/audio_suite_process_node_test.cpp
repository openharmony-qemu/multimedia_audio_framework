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

#include <array>
#include <memory>
#include <string>
#include <vector>

#include "audio_suite_process_node.h"
#include "audio_suite_manager.h"
#include "audio_suite_unittest_tools.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;
using namespace HPAE;

namespace {

static constexpr uint32_t NEED_DATA_LENGTH = 20;

/**
 * Fake pre-node:
 * Mixer pulls upstream via PullOutputData();
 * ProcessNode also pulls upstream via ReadPreNodeData/PullOutputData;
 * So upstream can provide data to downstream by calling WriteOutputData() in DoProcess.
 */
class FakePreNode : public AudioNode {
public:
    FakePreNode() : AudioNode(NODE_TYPE_EQUALIZER) {}
    ~FakePreNode() override = default;

    MOCK_METHOD(int32_t, DoProcess, (uint32_t needDataLength), (override));

    int32_t Flush() override
    {
        return SUCCESS;
    }

    int32_t Connect(const std::shared_ptr<AudioNode> &preNode) override
    {
        if (!preNode) {
            return ERR_INVALID_PARAM;
        }
        AddPreNode(preNode);
        preNode->AddNextNode(GetSharedInstance());
        return SUCCESS;
    }

    int32_t DisConnect(const std::shared_ptr<AudioNode> &preNode) override
    {
        if (!preNode) {
            return ERR_INVALID_PARAM;
        }
        RemovePreNode(preNode);
        preNode->RemoveNextNode(GetSharedInstance());
        return SUCCESS;
    }

    void SetNextOutput(AudioSuitePcmBuffer *buf)
    {
        nextOut_ = buf;
    }

    int32_t DoProcessImpl(uint32_t needDataLength)
    {
        (void)needDataLength;
        if (nextOut_ != nullptr) {
            WriteOutputData(nextOut_);
        }
        return SUCCESS;
    }

private:
    AudioSuitePcmBuffer *nextOut_ {nullptr};
};

class TestAudioSuiteProcessNode : public AudioSuiteProcessNode {
public:
    TestAudioSuiteProcessNode(AudioNodeType nodeType, AudioFormat audioFormat)
        : AudioSuiteProcessNode(nodeType, audioFormat) {}
    ~TestAudioSuiteProcessNode() override = default;

    std::vector<AudioSuitePcmBuffer *> SignalProcess(const std::vector<AudioSuitePcmBuffer *> &inputs) override
    {
        if (!inputs.empty()) {
            if (inputs[0] == nullptr) {
                return intermediateResult_;
            }
            uint8_t *unProcessedData = inputs[0]->GetPcmData();
            if (unProcessedData != nullptr) {
                *unProcessedData = 1;
            }
            return inputs;
        }
        return intermediateResult_;
    }

    // Expose protected PullOutputData for UT (Port has been removed).
    std::vector<AudioSuitePcmBuffer *> PullForTest(PcmBufferFormat outFormat, bool needConvert, uint32_t needLen)
    {
        return PullOutputData(outFormat, needConvert, needLen);
    }
};

class TestReadTapCallBack : public SuiteNodeReadTapDataCallback {
public:
    static bool testFlag;
    void OnReadTapDataCallback(void *audioData, int32_t audioDataSize) override
    {
        (void)audioData;
        (void)audioDataSize;
        testFlag = true;
    }
};

bool TestReadTapCallBack::testFlag = false;

class AudioSuiteProcessNodeTest : public ::testing::Test {
public:
    void SetUp() override
    {
        if (!AllNodeTypesSupported()) {
            GTEST_SKIP() << "not support all node types, skip this test";
        }
        AudioFormat audioFormat = {
            {CH_LAYOUT_STEREO, STEREO}, SAMPLE_S16LE, SAMPLE_RATE_48000,
            {CH_LAYOUT_STEREO, STEREO}, SAMPLE_S16LE, SAMPLE_RATE_48000};
        node_ = std::make_shared<TestAudioSuiteProcessNode>(NODE_TYPE_EQUALIZER, audioFormat);
        node_->InitOutputStream();
        node_->nodeNeedDataDuration_ = NEED_DATA_LENGTH;
        TestReadTapCallBack::testFlag = false;
    }

    void TearDown() override
    {
        if (node_ == nullptr) {
            return;
        }
        node_->Flush();
        TestReadTapCallBack::testFlag = false;
    }

    std::shared_ptr<TestAudioSuiteProcessNode> node_;
    PcmBufferFormat outFormat_ = {SAMPLE_RATE_48000, STEREO, CH_LAYOUT_STEREO, SAMPLE_S16LE};
};

HWTEST_F(AudioSuiteProcessNodeTest, ConstructorTest, TestSize.Level0)
{
    EXPECT_EQ(node_->GetNodeBypassStatus(), false);
}

HWTEST_F(AudioSuiteProcessNodeTest, DoProcessDefaultTest, TestSize.Level0)
{
    auto buffer = std::make_unique<AudioSuitePcmBuffer>(outFormat_);

    auto preNode = std::make_shared<FakePreNode>();
    preNode->InitFormatConverters();
    preNode->SetNextOutput(buffer.get());

    EXPECT_CALL(*preNode, DoProcess(NEED_DATA_LENGTH))
        .Times(1)
        .WillOnce([&preNode](uint32_t needLen) { return preNode->DoProcessImpl(needLen); });

    node_->Connect(preNode);

    std::vector<AudioSuitePcmBuffer *> result = node_->PullForTest(outFormat_, true, NEED_DATA_LENGTH);
    EXPECT_EQ(result.size(), 1);

    node_->DisConnect(preNode);
}

HWTEST_F(AudioSuiteProcessNodeTest, DoProcessWithEnableProcessFalseTest, TestSize.Level0)
{
    node_->SetBypassEffectNode(true);

    auto buffer = std::make_unique<AudioSuitePcmBuffer>(outFormat_);
    auto preNode = std::make_shared<FakePreNode>();
    preNode->InitFormatConverters();
    preNode->SetNextOutput(buffer.get());

    EXPECT_CALL(*preNode, DoProcess(NEED_DATA_LENGTH))
        .Times(1)
        .WillOnce([&preNode](uint32_t needLen) { return preNode->DoProcessImpl(needLen); });

    node_->Connect(preNode);

    std::vector<AudioSuitePcmBuffer *> result = node_->PullForTest(outFormat_, false, NEED_DATA_LENGTH);
    EXPECT_EQ(result.size(), 1);

    node_->DisConnect(preNode);
    node_->SetBypassEffectNode(false);
}

HWTEST_F(AudioSuiteProcessNodeTest, DoProcessWithFinishedPcmBufferTest, TestSize.Level0)
{
    auto buffer = std::make_unique<AudioSuitePcmBuffer>(outFormat_);
    buffer->SetIsFinished(true);

    auto preNode = std::make_shared<FakePreNode>();
    preNode->InitFormatConverters();
    preNode->SetNextOutput(buffer.get());

    EXPECT_CALL(*preNode, DoProcess(NEED_DATA_LENGTH))
        .Times(1)
        .WillOnce([&preNode](uint32_t needLen) { return preNode->DoProcessImpl(needLen); });

    node_->Connect(preNode);

    std::vector<AudioSuitePcmBuffer *> result = node_->PullForTest(outFormat_, true, NEED_DATA_LENGTH);
    EXPECT_EQ(result.size(), 1);
    ASSERT_NE(result[0], nullptr);
    EXPECT_EQ(result[0]->GetIsFinished(), true);

    EXPECT_EQ(node_->GetAudioNodeDataFinishedFlag(), true);

    std::vector<AudioSuitePcmBuffer *> resultWhenNodeFinished =
        node_->PullForTest(outFormat_, true, NEED_DATA_LENGTH);
    EXPECT_EQ(resultWhenNodeFinished.size(), 0);

    node_->DisConnect(preNode);
}

HWTEST_F(AudioSuiteProcessNodeTest, DoProcessGetBypassTest, TestSize.Level0)
{
    int32_t ret = node_->SetBypassEffectNode(true);
    EXPECT_EQ(ret, SUCCESS);

    ret = node_->DoProcess(NEED_DATA_LENGTH);
    EXPECT_EQ(ret, ERROR);

    auto preNode = std::make_shared<FakePreNode>();
    node_->Connect(preNode);

    ret = node_->DoProcess(NEED_DATA_LENGTH);
    EXPECT_EQ(ret, ERROR);

    node_->DisConnect(preNode);
    node_->SetBypassEffectNode(false);
}

HWTEST_F(AudioSuiteProcessNodeTest, FlushTest, TestSize.Level0)
{
    AudioFormat audioFormat = {
            {CH_LAYOUT_STEREO, STEREO}, SAMPLE_S16LE, SAMPLE_RATE_48000,
            {CH_LAYOUT_STEREO, STEREO}, SAMPLE_S16LE, SAMPLE_RATE_48000};

    std::unique_ptr<TestAudioSuiteProcessNode> node;
    node = std::make_unique<TestAudioSuiteProcessNode>(NODE_TYPE_NOISE_REDUCTION, audioFormat);
    EXPECT_EQ(SUCCESS, node->Flush());
    node = std::make_unique<TestAudioSuiteProcessNode>(NODE_TYPE_SOUND_FIELD, audioFormat);
    EXPECT_EQ(SUCCESS, node->Flush());
    node = std::make_unique<TestAudioSuiteProcessNode>(NODE_TYPE_AUDIO_SEPARATION, audioFormat);
    EXPECT_EQ(SUCCESS, node->Flush());
    node = std::make_unique<TestAudioSuiteProcessNode>(NODE_TYPE_VOICE_BEAUTIFIER, audioFormat);
    EXPECT_EQ(SUCCESS, node->Flush());
    node = std::make_unique<TestAudioSuiteProcessNode>(NODE_TYPE_ENVIRONMENT_EFFECT, audioFormat);
    EXPECT_EQ(SUCCESS, node->Flush());
    node = std::make_unique<TestAudioSuiteProcessNode>(NODE_TYPE_AUDIO_MIXER, audioFormat);
    EXPECT_EQ(SUCCESS, node->Flush());
    node = std::make_unique<TestAudioSuiteProcessNode>(NODE_TYPE_SPACE_RENDER, audioFormat);
    EXPECT_EQ(SUCCESS, node->Flush());
    node = std::make_unique<TestAudioSuiteProcessNode>(NODE_TYPE_PURE_VOICE_CHANGE, audioFormat);
    EXPECT_EQ(SUCCESS, node->Flush());
    node = std::make_unique<TestAudioSuiteProcessNode>(NODE_TYPE_GENERAL_VOICE_CHANGE, audioFormat);
    EXPECT_EQ(SUCCESS, node->Flush());
    node = std::make_unique<TestAudioSuiteProcessNode>(NODE_TYPE_TEMPO_PITCH, audioFormat);
    EXPECT_EQ(SUCCESS, node->Flush());
    node = nullptr;
}

HWTEST_F(AudioSuiteProcessNodeTest, CheckEffectNodeOvertimeCountTest_001, TestSize.Level0)
{
    AudioFormat audioFormat = {
            {CH_LAYOUT_STEREO, STEREO}, SAMPLE_S16LE, SAMPLE_RATE_48000,
            {CH_LAYOUT_STEREO, STEREO}, SAMPLE_S16LE, SAMPLE_RATE_48000};

    std::unique_ptr<TestAudioSuiteProcessNode> node =
        std::make_unique<TestAudioSuiteProcessNode>(NODE_TYPE_EQUALIZER, audioFormat);

    int32_t dataDurationMS = 20;
    uint64_t processDurationBase = dataDurationMS * MILLISECONDS_TO_MICROSECONDS * node->nodeParameter_.realtimeFactor;
    uint64_t testDurationNormal = 1;
    uint64_t testDurationBase = processDurationBase * RTF_OVERTIME_THRESHOLDS[RtfOvertimeLevel::OVER_BASE];
    uint64_t testDuration110Base = processDurationBase * RTF_OVERTIME_THRESHOLDS[RtfOvertimeLevel::OVER_110BASE];
    uint64_t testDuration120Base = processDurationBase * RTF_OVERTIME_THRESHOLDS[RtfOvertimeLevel::OVER_120BASE];
    uint64_t testDurationOver120Base = testDuration120Base + 1;
    uint64_t testDuration100 = dataDurationMS * MILLISECONDS_TO_MICROSECONDS + 1;
    uint64_t testDurationOver100 = testDuration100 + 1;

    std::array<int32_t, RTF_OVERTIME_LEVELS> expectedArrayEmpty = {0, 0, 0};
    std::array<int32_t, RTF_OVERTIME_LEVELS> expectedArrayBase = {1, 0, 0};
    std::array<int32_t, RTF_OVERTIME_LEVELS> expectedArrayMultiple = {6, 5, 4};

    std::array<PipelineWorkMode, 2> workModeArray = {PIPELINE_REALTIME_MODE, PIPELINE_EDIT_MODE};
    for (PipelineWorkMode testWorkMode : workModeArray) {
        node->SetAudioNodeWorkMode(testWorkMode);

        node->CheckEffectNodeProcessTime(dataDurationMS, testDurationBase);
        EXPECT_EQ(node->rtfOvertimeCounters_, expectedArrayBase);
        EXPECT_EQ(node->rtfOver100Count_, 0);
        EXPECT_EQ(node->signalProcessTotalCount_, 1);
        node->CheckEffectNodeOvertimeCount();
        EXPECT_EQ(node->rtfOvertimeCounters_, expectedArrayEmpty);
        EXPECT_EQ(node->rtfOver100Count_, 0);
        EXPECT_EQ(node->signalProcessTotalCount_, 0);

        node->CheckEffectNodeProcessTime(dataDurationMS, testDurationNormal);
        node->CheckEffectNodeProcessTime(dataDurationMS, testDurationNormal);
        node->CheckEffectNodeProcessTime(dataDurationMS, testDurationBase);
        node->CheckEffectNodeProcessTime(dataDurationMS, testDuration110Base);
        node->CheckEffectNodeProcessTime(dataDurationMS, testDuration120Base);
        node->CheckEffectNodeProcessTime(dataDurationMS, testDurationOver120Base);
        node->CheckEffectNodeProcessTime(dataDurationMS, testDuration100);
        node->CheckEffectNodeProcessTime(dataDurationMS, testDurationOver100);
        node->CheckEffectNodeProcessTime(0, 1);
        EXPECT_EQ(node->signalProcessTotalCount_, 8);
        EXPECT_EQ(node->rtfOvertimeCounters_, expectedArrayMultiple);
        EXPECT_EQ(node->rtfOver100Count_, 2);

        node->CheckEffectNodeOvertimeCount();
        EXPECT_EQ(node->signalProcessTotalCount_, 0);
        EXPECT_EQ(node->rtfOvertimeCounters_, expectedArrayEmpty);
        EXPECT_EQ(node->rtfOver100Count_, 0);
    }
}

HWTEST_F(AudioSuiteProcessNodeTest, InitFormatConverters_Default, TestSize.Level0)
{
    AudioFormat audioFormat = {
            {CH_LAYOUT_STEREO, STEREO}, SAMPLE_S16LE, SAMPLE_RATE_48000,
            {CH_LAYOUT_STEREO, STEREO}, SAMPLE_S16LE, SAMPLE_RATE_48000};
    std::unique_ptr<TestAudioSuiteProcessNode> node =
        std::make_unique<TestAudioSuiteProcessNode>(NODE_TYPE_EQUALIZER, audioFormat);
    auto ret = node->InitFormatConverters();
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(node->formatConverters_.size(), 1);
    EXPECT_EQ(node->formatConversionBuffers_.size(), 1);
}

HWTEST_F(AudioSuiteProcessNodeTest, InitFormatConverters_AudioSeparation, TestSize.Level0)
{
    AudioFormat audioFormat = {
            {CH_LAYOUT_STEREO, STEREO}, SAMPLE_S16LE, SAMPLE_RATE_48000,
            {CH_LAYOUT_STEREO, STEREO}, SAMPLE_S16LE, SAMPLE_RATE_48000};
    std::unique_ptr<TestAudioSuiteProcessNode> node =
        std::make_unique<TestAudioSuiteProcessNode>(NODE_TYPE_AUDIO_SEPARATION, audioFormat);
    auto ret = node->InitFormatConverters();
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(node->formatConverters_.size(), 2);
    EXPECT_EQ(node->formatConversionBuffers_.size(), 2);
}

HWTEST_F(AudioSuiteProcessNodeTest, ReadProcessNodePreOutputData_WithPreNode, TestSize.Level0)
{
    auto buffer = std::make_unique<AudioSuitePcmBuffer>(outFormat_);
    auto preNode = std::make_shared<FakePreNode>();
    preNode->InitFormatConverters();
    preNode->SetNextOutput(buffer.get());

    EXPECT_CALL(*preNode, DoProcess(NEED_DATA_LENGTH))
        .Times(1)
        .WillOnce([&preNode](uint32_t needLen) { return preNode->DoProcessImpl(needLen); });

    node_->Connect(preNode);
    node_->requestPreNodeDuration_ = NEED_DATA_LENGTH;
    node_->ReadProcessNodePreOutputData();

    EXPECT_FALSE(node_->preNodeResult_.empty());
}

HWTEST_F(AudioSuiteProcessNodeTest, ReadProcessNodePreOutputData_WithMultiplePreNodes, TestSize.Level0)
{
    auto buffer1 = std::make_unique<AudioSuitePcmBuffer>(outFormat_);
    auto buffer2 = std::make_unique<AudioSuitePcmBuffer>(outFormat_);
    auto preNode1 = std::make_shared<FakePreNode>();
    auto preNode2 = std::make_shared<FakePreNode>();
    preNode1->InitFormatConverters();
    preNode2->InitFormatConverters();
    preNode1->SetNextOutput(buffer1.get());
    preNode2->SetNextOutput(buffer2.get());

    EXPECT_CALL(*preNode1, DoProcess(NEED_DATA_LENGTH))
        .Times(1)
        .WillOnce([&preNode1](uint32_t needLen) { return preNode1->DoProcessImpl(needLen); });
    EXPECT_CALL(*preNode2, DoProcess(NEED_DATA_LENGTH))
        .Times(1)
        .WillOnce([&preNode2](uint32_t needLen) { return preNode2->DoProcessImpl(needLen); });

    node_->Connect(preNode1);
    node_->Connect(preNode2);
    node_->requestPreNodeDuration_ = NEED_DATA_LENGTH;
    node_->ReadProcessNodePreOutputData();

    EXPECT_EQ(node_->preNodeResult_.size(), 2);
}

HWTEST_F(AudioSuiteProcessNodeTest, ReadProcessNodePreOutputData_WithExpiredPreNode, TestSize.Level0)
{
    auto buffer = std::make_unique<AudioSuitePcmBuffer>(outFormat_);
    auto preNode = std::make_shared<FakePreNode>();
    preNode->InitFormatConverters();
    preNode->SetNextOutput(buffer.get());

    EXPECT_CALL(*preNode, DoProcess(NEED_DATA_LENGTH))
        .Times(0);

    node_->Connect(preNode);
    preNode.reset();
    node_->requestPreNodeDuration_ = NEED_DATA_LENGTH;
    node_->ReadProcessNodePreOutputData();

    EXPECT_TRUE(node_->preNodeResult_.empty());
}

HWTEST_F(AudioSuiteProcessNodeTest, ReadProcessNodePreOutputData_WithFinishedBuffer, TestSize.Level0)
{
    auto buffer = std::make_unique<AudioSuitePcmBuffer>(outFormat_);
    buffer->SetIsFinished(true);
    auto preNode = std::make_shared<FakePreNode>();
    preNode->InitFormatConverters();
    preNode->SetNextOutput(buffer.get());

    EXPECT_CALL(*preNode, DoProcess(NEED_DATA_LENGTH))
        .Times(1)
        .WillOnce([&preNode](uint32_t needLen) { return preNode->DoProcessImpl(needLen); });

    node_->Connect(preNode);
    node_->requestPreNodeDuration_ = NEED_DATA_LENGTH;
    node_->ReadProcessNodePreOutputData();

    EXPECT_TRUE(node_->GetAudioNodeDataFinishedFlag());
}

HWTEST_F(AudioSuiteProcessNodeTest, ObtainProcessedData_EmptyPreOutputs, TestSize.Level0)
{
    auto preNode = std::make_shared<FakePreNode>();
    preNode->InitFormatConverters();
    node_->Connect(preNode);

    EXPECT_CALL(*preNode, DoProcess(NEED_DATA_LENGTH))
        .Times(1)
        .WillOnce([&preNode](uint32_t needLen) { return preNode->DoProcessImpl(needLen); });

    node_->requestPreNodeDuration_ = NEED_DATA_LENGTH;
    auto ret = node_->ObtainProcessedData();
    EXPECT_EQ(ret, ERROR);
}

HWTEST_F(AudioSuiteProcessNodeTest, ProcessBypassMode_WithPreNode, TestSize.Level0)
{
    node_->SetBypassEffectNode(true);
    auto buffer = std::make_unique<AudioSuitePcmBuffer>(outFormat_);
    auto preNode = std::make_shared<FakePreNode>();
    preNode->InitFormatConverters();
    preNode->SetNextOutput(buffer.get());

    EXPECT_CALL(*preNode, DoProcess(NEED_DATA_LENGTH))
        .Times(1)
        .WillOnce([&preNode](uint32_t needLen) { return preNode->DoProcessImpl(needLen); });

    node_->Connect(preNode);
    auto ret = node_->ProcessBypassMode(NEED_DATA_LENGTH);
    EXPECT_EQ(ret, SUCCESS);
}
}  // namespace