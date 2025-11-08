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
    void SetUp() override
    {
        // init nodeinfo
        AudioFormat audioFormat = {
            {CH_LAYOUT_STEREO, STEREO},
            SAMPLE_S16LE,
            SAMPLE_RATE_48000};
        node_ = std::make_shared<TestAudioSuiteProcessNode>(NODE_TYPE_EQUALIZER, audioFormat);
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

HWTEST_F(AudioSuiteProcessNodeTest, ConstructorTest, TestSize.Level0) {
    // test constructor
    EXPECT_NE(node_->GetInputPort(), nullptr);
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
        .Times(1).WillRepeatedly(::testing::Return(inputNodeOutputPort));
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
        .Times(1).WillRepeatedly(::testing::Return(inputNodeOutputPort));
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
        .Times(1).WillRepeatedly(::testing::Return(inputNodeOutputPort));
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
        .Times(1).WillRepeatedly(::testing::Return(inputNodeOutputPort));
    node_->Connect(mockInputNode_);

    ret = node_->DoProcess();
    EXPECT_EQ(ret, SUCCESS);
    node_->DisConnect(mockInputNode_);
    testing::Mock::VerifyAndClearExpectations(mockInputNode_.get());
    mockInputNode_.reset();
    inputNodeOutputPort.reset();
}

}