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

using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;
using namespace HPAE;
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
            uint8_t *unProcessedData = inputs[0]->GetPcmData();
            if (unProcessedData != nullptr) {
                *unProcessedData = 1;
            }
            return inputs[0];
        }
        return nullptr;
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
    void SetUp() override
    {
        // init nodeinfo
        AudioFormat audioFormat = {
            {CH_LAYOUT_STEREO, STEREO},
            SAMPLE_S16LE,
            SAMPLE_RATE_48000};
        node_ = std::make_shared<TestAudioSuiteProcessNode>(NODE_TYPE_EQUALIZER, audioFormat);
        node_->InitOutputStream();
        TestReadTapCallBack::testFlag = false;
    }
    void TearDown() override
    {
        if (node_ == nullptr) {
            return;
        }
        node_->Flush();
        TestReadTapCallBack::testFlag = false;
    };

    std::shared_ptr<TestAudioSuiteProcessNode> node_;
    PcmBufferFormat outFormat_ = {SAMPLE_RATE_48000, STEREO, CH_LAYOUT_STEREO, SAMPLE_S16LE};
};

static int32_t g_expectedGetOutputPortCalls = 2;         // Times of GetOutputPort called in DoProcess

HWTEST_F(AudioSuiteProcessNodeTest, ConstructorTest, TestSize.Level0) {
    // test constructor
    EXPECT_NE(node_->GetOutputPort(), nullptr);
    EXPECT_EQ(node_->GetNodeBypassStatus(), false);
}

HWTEST_F(AudioSuiteProcessNodeTest, DoProcessDefaultTest, TestSize.Level0)
{
    std::unique_ptr<AudioSuitePcmBuffer> buffer = std::make_unique<AudioSuitePcmBuffer>(outFormat_);
    std::shared_ptr<MockInputNode> mockInputNode_ = std::make_unique<MockInputNode>();
    std::shared_ptr<OutputPort<AudioSuitePcmBuffer*>> inputNodeOutputPort =
        std::make_shared<OutputPort<AudioSuitePcmBuffer*>>(mockInputNode_);
    inputNodeOutputPort->WriteDataToOutput(buffer.get());
    EXPECT_CALL(*mockInputNode_, DoProcess()).Times(1).WillRepeatedly(::testing::Return(SUCCESS));
    EXPECT_CALL(*mockInputNode_, GetOutputPort())
        .Times(g_expectedGetOutputPortCalls).WillRepeatedly(::testing::Return(inputNodeOutputPort));
    node_->Connect(mockInputNode_);
    EXPECT_EQ(inputNodeOutputPort->GetInputNum(), 1);
    std::shared_ptr<OutputPort<AudioSuitePcmBuffer*>> nodeOutputPort =
        node_->GetOutputPort();
    std::vector<AudioSuitePcmBuffer *> result = nodeOutputPort->PullOutputData(outFormat_, true);
    EXPECT_EQ(result.size(), 1);
    node_->DisConnect(mockInputNode_);
    EXPECT_EQ(inputNodeOutputPort->GetInputNum(), 0);
    testing::Mock::VerifyAndClearExpectations(mockInputNode_.get());
    mockInputNode_.reset();
    inputNodeOutputPort.reset();
}

HWTEST_F(AudioSuiteProcessNodeTest, DoProcessWithEnableProcessFalseTest, TestSize.Level0)
{
    node_->SetBypassEffectNode(true);
    std::unique_ptr<AudioSuitePcmBuffer> buffer = std::make_unique<AudioSuitePcmBuffer>(outFormat_);
    std::shared_ptr<MockInputNode> mockInputNode_ = std::make_unique<MockInputNode>();
    std::shared_ptr<OutputPort<AudioSuitePcmBuffer*>> inputNodeOutputPort =
        std::make_shared<OutputPort<AudioSuitePcmBuffer*>>(mockInputNode_);
    inputNodeOutputPort->WriteDataToOutput(buffer.get());
    EXPECT_CALL(*mockInputNode_, DoProcess()).Times(1).WillRepeatedly(::testing::Return(SUCCESS));
    EXPECT_CALL(*mockInputNode_, GetOutputPort())
        .Times(g_expectedGetOutputPortCalls).WillRepeatedly(::testing::Return(inputNodeOutputPort));
    node_->Connect(mockInputNode_);
    EXPECT_EQ(inputNodeOutputPort->GetInputNum(), 1);
    std::shared_ptr<OutputPort<AudioSuitePcmBuffer*>> nodeOutputPort =
        node_->GetOutputPort();
    std::vector<AudioSuitePcmBuffer *> result = nodeOutputPort->PullOutputData(outFormat_, false);
    EXPECT_EQ(result.size(), 1);
    node_->DisConnect(mockInputNode_);
    EXPECT_EQ(inputNodeOutputPort->GetInputNum(), 0);
    testing::Mock::VerifyAndClearExpectations(mockInputNode_.get());
    mockInputNode_.reset();
    inputNodeOutputPort.reset();
    node_->SetBypassEffectNode(false);
}

HWTEST_F(AudioSuiteProcessNodeTest, DoProcessWithFinishedPcmBufferTest, TestSize.Level0)
{
    std::unique_ptr<AudioSuitePcmBuffer> buffer = std::make_unique<AudioSuitePcmBuffer>(outFormat_);
    buffer->SetIsFinished(true);
    std::shared_ptr<MockInputNode> mockInputNode_ = std::make_unique<MockInputNode>();
    std::shared_ptr<OutputPort<AudioSuitePcmBuffer*>> inputNodeOutputPort =
        std::make_shared<OutputPort<AudioSuitePcmBuffer*>>(mockInputNode_);
    inputNodeOutputPort->WriteDataToOutput(buffer.get());
    EXPECT_CALL(*mockInputNode_, DoProcess()).Times(1).WillRepeatedly(::testing::Return(SUCCESS));
    EXPECT_CALL(*mockInputNode_, GetOutputPort())
        .Times(2).WillRepeatedly(::testing::Return(inputNodeOutputPort));
    node_->Connect(mockInputNode_);
    EXPECT_EQ(inputNodeOutputPort->GetInputNum(), 1);
    std::shared_ptr<OutputPort<AudioSuitePcmBuffer*>> nodeOutputPort =
        node_->GetOutputPort();
    std::vector<AudioSuitePcmBuffer *> result = nodeOutputPort->PullOutputData(outFormat_, true);
    EXPECT_EQ(result.size(), 1);
    EXPECT_NE(result[0], nullptr);
    EXPECT_EQ(result[0]->GetIsFinished(), true);
    EXPECT_EQ(node_->GetAudioNodeDataFinishedFlag(), true);
    std::vector<AudioSuitePcmBuffer *> resultWhenNodeFinished = nodeOutputPort->PullOutputData(outFormat_, true);
    EXPECT_EQ(resultWhenNodeFinished.size(), 0);
    node_->DisConnect(mockInputNode_);
    EXPECT_EQ(inputNodeOutputPort->GetInputNum(), 0);
    testing::Mock::VerifyAndClearExpectations(mockInputNode_.get());
    mockInputNode_.reset();
    inputNodeOutputPort.reset();
}

HWTEST_F(AudioSuiteProcessNodeTest, DoProcessGetBypassTest, TestSize.Level0)
{
    int32_t ret = node_->SetBypassEffectNode(true);
    EXPECT_EQ(ret, SUCCESS);

    ret = node_->DoProcess();
    EXPECT_EQ(ret, ERROR);

    std::unique_ptr<AudioSuitePcmBuffer> buffer = std::make_unique<AudioSuitePcmBuffer>(outFormat_);
    std::shared_ptr<MockInputNode> mockInputNode_ = std::make_unique<MockInputNode>();
    std::shared_ptr<OutputPort<AudioSuitePcmBuffer*>> inputNodeOutputPort =
        std::make_shared<OutputPort<AudioSuitePcmBuffer*>>(mockInputNode_);
    inputNodeOutputPort->WriteDataToOutput(buffer.get());
    EXPECT_CALL(*mockInputNode_, GetOutputPort())
        .Times(g_expectedGetOutputPortCalls).WillRepeatedly(::testing::Return(inputNodeOutputPort));
    node_->Connect(mockInputNode_);

    ret = node_->DoProcess();
    EXPECT_EQ(ret, SUCCESS);
    node_->DisConnect(mockInputNode_);
    testing::Mock::VerifyAndClearExpectations(mockInputNode_.get());
    mockInputNode_.reset();
    inputNodeOutputPort.reset();
}

HWTEST_F(AudioSuiteProcessNodeTest, FlushTest, TestSize.Level0)
{
    AudioFormat audioFormat = {
            {CH_LAYOUT_STEREO, STEREO},
            SAMPLE_S16LE,
            SAMPLE_RATE_48000};

    auto node = TestAudioSuiteProcessNode(NODE_TYPE_EQUALIZER, audioFormat);
    EXPECT_EQ(SUCCESS, node.Flush());
    node = TestAudioSuiteProcessNode(NODE_TYPE_NOISE_REDUCTION, audioFormat);
    EXPECT_EQ(SUCCESS, node.Flush());
    node = TestAudioSuiteProcessNode(NODE_TYPE_SOUND_FIELD, audioFormat);
    EXPECT_EQ(SUCCESS, node.Flush());
    node = TestAudioSuiteProcessNode(NODE_TYPE_AUDIO_SEPARATION, audioFormat);
    EXPECT_EQ(SUCCESS, node.Flush());
    node = TestAudioSuiteProcessNode(NODE_TYPE_VOICE_BEAUTIFIER, audioFormat);
    EXPECT_EQ(SUCCESS, node.Flush());
    node = TestAudioSuiteProcessNode(NODE_TYPE_ENVIRONMENT_EFFECT, audioFormat);
    EXPECT_EQ(SUCCESS, node.Flush());
    node = TestAudioSuiteProcessNode(NODE_TYPE_AUDIO_MIXER, audioFormat);
    EXPECT_EQ(SUCCESS, node.Flush());
    node = TestAudioSuiteProcessNode(NODE_TYPE_SPACE_RENDER, audioFormat);
    EXPECT_EQ(SUCCESS, node.Flush());
    node = TestAudioSuiteProcessNode(NODE_TYPE_PURE_VOICE_CHANGE, audioFormat);
    EXPECT_EQ(SUCCESS, node.Flush());
    node = TestAudioSuiteProcessNode(NODE_TYPE_GENERAL_VOICE_CHANGE, audioFormat);
    EXPECT_EQ(SUCCESS, node.Flush());
    node = TestAudioSuiteProcessNode(NODE_TYPE_TEMPO_PITCH, audioFormat);
    EXPECT_EQ(SUCCESS, node.Flush());
    node.DeInit();
}

HWTEST_F(AudioSuiteProcessNodeTest, CheckEffectNodeOvertimeCountTest_001, TestSize.Level0)
{
    AudioFormat audioFormat = {
            {CH_LAYOUT_STEREO, STEREO},
            SAMPLE_S16LE,
            SAMPLE_RATE_48000};

    auto node = TestAudioSuiteProcessNode(NODE_TYPE_EQUALIZER, audioFormat);

    int32_t dataDurationMS = 20;  // 20 ms pcmbuf duration for example
    // processDurationBase is for compare use, dataduration * rtfBase(0.15 for eq node)
    uint64_t processDurationBase = dataDurationMS * MILLISECONDS_TO_MICROSECONDS * node.nodeCapability.realtimeFactor;
    uint64_t testDurationNormal = 1;  // 1 microsecond
    uint64_t testDurationBase = processDurationBase * RTF_OVERTIME_THRESHOLDS[RtfOvertimeLevel::OVER_BASE];
    uint64_t testDuration110Base = processDurationBase * RTF_OVERTIME_THRESHOLDS[RtfOvertimeLevel::OVER_110BASE];
    uint64_t testDuration120Base = processDurationBase * RTF_OVERTIME_THRESHOLDS[RtfOvertimeLevel::OVER_120BASE];
    uint64_t testDurationOver120Base = testDuration120Base + 1;
    uint64_t testDuration100 = dataDurationMS * MILLISECONDS_TO_MICROSECONDS + 1;
    uint64_t testDurationOver100 = testDuration100 + 1;

    // expected OvertimeCounters
    std::array<int32_t, RTF_OVERTIME_LEVELS> expectedArrayEmpty = {0, 0, 0};
    std::array<int32_t, RTF_OVERTIME_LEVELS> expectedArrayBase = {1, 0, 0};
    std::array<int32_t, RTF_OVERTIME_LEVELS> expectedArrayMultiple = {6, 5, 4};

    std::array<PipelineWorkMode, 2> workModeArray = {PIPELINE_REALTIME_MODE, PIPELINE_EDIT_MODE};
    for (PipelineWorkMode testWorkMode : workModeArray) {
        node.SetAudioNodeWorkMode(testWorkMode);

        // rtf equal baseline
        node.CheckEffectNodeProcessTime(dataDurationMS, testDurationBase);
        EXPECT_EQ(node.rtfOvertimeCounters_, expectedArrayBase);
        EXPECT_EQ(node.rtfOver100Count_, 0);
        EXPECT_EQ(node.signalProcessTotalCount_, 1);
        node.CheckEffectNodeOvertimeCount();
        EXPECT_EQ(node.rtfOvertimeCounters_, expectedArrayEmpty);
        EXPECT_EQ(node.rtfOver100Count_, 0);
        EXPECT_EQ(node.signalProcessTotalCount_, 0);

        // check multiple times
        node.CheckEffectNodeProcessTime(dataDurationMS, testDurationNormal);
        node.CheckEffectNodeProcessTime(dataDurationMS, testDurationNormal);
        node.CheckEffectNodeProcessTime(dataDurationMS, testDurationBase);
        node.CheckEffectNodeProcessTime(dataDurationMS, testDuration110Base);
        node.CheckEffectNodeProcessTime(dataDurationMS, testDuration120Base);
        node.CheckEffectNodeProcessTime(dataDurationMS, testDurationOver120Base);
        node.CheckEffectNodeProcessTime(dataDurationMS, testDuration100);
        node.CheckEffectNodeProcessTime(dataDurationMS, testDurationOver100);
        node.CheckEffectNodeProcessTime(0, 1);  // invalid data duration, ignore.
        EXPECT_EQ(node.signalProcessTotalCount_, 8);
        EXPECT_EQ(node.rtfOvertimeCounters_, expectedArrayMultiple);
        EXPECT_EQ(node.rtfOver100Count_, 2);

        node.CheckEffectNodeOvertimeCount();
        EXPECT_EQ(node.signalProcessTotalCount_, 0);
        EXPECT_EQ(node.rtfOvertimeCounters_, expectedArrayEmpty);
        EXPECT_EQ(node.rtfOver100Count_, 0);
    }
}

}  // namespace