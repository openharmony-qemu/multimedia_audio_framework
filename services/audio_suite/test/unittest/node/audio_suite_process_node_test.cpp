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

constexpr uint32_t TEST_BUFFER_LEN = 10;
constexpr bool MIX_FLE = true;

class MockInputNode : public AudioNode {
public:
    MockInputNode() : AudioNode(NODE_TYPE_EQUALIZER)
    {}
    ~MockInputNode() {}
    MOCK_METHOD(int32_t, DoProcess, (), ());
    MOCK_METHOD(std::shared_ptr<OutputPort<AudioSuitePcmBuffer*>>, GetOutputPort, (AudioNodePortType type), ());
    MOCK_METHOD(int32_t, Flush, (), ());
    MOCK_METHOD(int32_t, RemoveTap, (AudioNodePortType portType), ());
    MOCK_METHOD(int32_t, Connect, (const std::shared_ptr<AudioNode> &preNode, AudioNodePortType type), ());
    MOCK_METHOD(int32_t, DisConnect, (const std::shared_ptr<AudioNode> &preNode), ());
    MOCK_METHOD(int32_t, InstallTap, (
        AudioNodePortType portType, std::shared_ptr<SuiteNodeReadTapDataCallback> callback), ());
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
            float* unProcessedData = inputs[0]->GetPcmDataBuffer();
            if (unProcessedData != nullptr) {
                *unProcessedData = 1.0f;
            }
            return inputs[0];
        }
        return nullptr;
    }
    bool Reset() override
    {
        return true;
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
protected:
    void SetUp() override
    {
        // init nodeinfo
        AudioChannelInfo audioChannelInfo;
        AudioFormat audioFormat = {
            .audioChannelInfo = audioChannelInfo
        };
        node_ = std::make_shared<TestAudioSuiteProcessNode>(NODE_TYPE_EQUALIZER, audioFormat);
        TestReadTapCallBack::testFlag = false;
    }
    void TearDown() override
    {
        if (node_ == nullptr) {
            return;
        }
        node_->Flush();
        node_->Reset();
        TestReadTapCallBack::testFlag = false;
    };

    std::shared_ptr<TestAudioSuiteProcessNode> node_;
};

HWTEST_F(AudioSuiteProcessNodeTest, ConstructorTest, TestSize.Level0) {
    // test constructor
    EXPECT_NE(node_->GetInputPort(), nullptr);
    EXPECT_NE(node_->GetOutputPort(AUDIO_NODE_DEFAULT_OUTPORT_TYPE), nullptr);
    EXPECT_TRUE(node_->GetNodeEnableStatus());
}

HWTEST_F(AudioSuiteProcessNodeTest, ChannelConverterTestProcessTest, TestSize.Level0)
{
    // test channelConverter
    AudioChannelInfo inChannelInfo;
    AudioChannelInfo outChannelInfo;
    inChannelInfo.numChannels = MONO;
    inChannelInfo.channelLayout = CH_LAYOUT_MONO;
    outChannelInfo.numChannels = STEREO;
    outChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    std::vector<float> in(TEST_BUFFER_LEN * MONO, 0.0f);
    std::vector<float> out(TEST_BUFFER_LEN * STEREO, 0.0f);
    EXPECT_EQ(node_->SetChannelConvertProcessParam(inChannelInfo, outChannelInfo, SAMPLE_F32LE, MIX_FLE),
        MIX_ERR_SUCCESS);
    EXPECT_EQ(node_->ChannelConvertProcess(TEST_BUFFER_LEN, in.data(), in.size() * sizeof(float), out.data(),
        out.size() * sizeof(float)), MIX_ERR_SUCCESS);
    // test downmix
    inChannelInfo.numChannels = CHANNEL_6;
    inChannelInfo.channelLayout = CH_LAYOUT_5POINT1;
    in.resize(TEST_BUFFER_LEN * CHANNEL_6, 0.0f);
    EXPECT_EQ(node_->SetChannelConvertProcessParam(inChannelInfo, outChannelInfo, SAMPLE_F32LE, MIX_FLE),
        MIX_ERR_SUCCESS);
    EXPECT_EQ(node_->ChannelConvertProcess(TEST_BUFFER_LEN, in.data(), in.size() * sizeof(float), out.data(),
        out.size() * sizeof(float)), MIX_ERR_SUCCESS);
}

HWTEST_F(AudioSuiteProcessNodeTest, DoProcessDefaultTest, TestSize.Level0)
{
    std::unique_ptr<AudioSuitePcmBuffer> buffer = std::make_unique<AudioSuitePcmBuffer>(SAMPLE_RATE_48000,
        static_cast<uint32_t>(4), AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    std::shared_ptr<MockInputNode> mockInputNode_ = std::make_unique<MockInputNode>();
    std::shared_ptr<OutputPort<AudioSuitePcmBuffer*>> inputNodeOutputPort =
        std::make_shared<OutputPort<AudioSuitePcmBuffer*>>(mockInputNode_);
    inputNodeOutputPort->WriteDataToOutput(buffer.get());
    EXPECT_CALL(*mockInputNode_, DoProcess()).Times(1).WillRepeatedly(::testing::Return(SUCCESS));
    EXPECT_CALL(*mockInputNode_, GetOutputPort(::testing::_))
        .Times(1).WillRepeatedly(::testing::Return(inputNodeOutputPort));
    node_->Connect(mockInputNode_, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(inputNodeOutputPort->GetInputNum(), 1);
    std::shared_ptr<OutputPort<AudioSuitePcmBuffer*>> nodeOutputPort =
        node_->GetOutputPort(AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    node_->InstallTap(AUDIO_NODE_DEFAULT_OUTPORT_TYPE, nullptr);
    AudioSuitePcmBuffer* result = nodeOutputPort->PullOutputData();
    EXPECT_EQ(*(result->GetPcmDataBuffer()), 1.0f);
    node_->RemoveTap(AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    node_->DisConnect(mockInputNode_);
    EXPECT_EQ(inputNodeOutputPort->GetInputNum(), 0);
    testing::Mock::VerifyAndClearExpectations(mockInputNode_.get());
    mockInputNode_.reset();
    inputNodeOutputPort.reset();
}

HWTEST_F(AudioSuiteProcessNodeTest, DoProcessWithEnableProcessFalseTest, TestSize.Level0)
{
    node_->SetNodeEnableStatus(NODE_DISABLE);
    std::unique_ptr<AudioSuitePcmBuffer> buffer = std::make_unique<AudioSuitePcmBuffer>(SAMPLE_RATE_48000,
        static_cast<uint32_t>(4), AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    std::shared_ptr<MockInputNode> mockInputNode_ = std::make_unique<MockInputNode>();
    std::shared_ptr<OutputPort<AudioSuitePcmBuffer*>> inputNodeOutputPort =
        std::make_shared<OutputPort<AudioSuitePcmBuffer*>>(mockInputNode_);
    inputNodeOutputPort->WriteDataToOutput(buffer.get());
    EXPECT_CALL(*mockInputNode_, DoProcess()).Times(1).WillRepeatedly(::testing::Return(SUCCESS));
    EXPECT_CALL(*mockInputNode_, GetOutputPort(::testing::_))
        .Times(1).WillRepeatedly(::testing::Return(inputNodeOutputPort));
    node_->Connect(mockInputNode_, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(inputNodeOutputPort->GetInputNum(), 1);
    std::shared_ptr<OutputPort<AudioSuitePcmBuffer*>> nodeOutputPort =
        node_->GetOutputPort(AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    node_->InstallTap(AUDIO_NODE_DEFAULT_OUTPORT_TYPE, nullptr);
    AudioSuitePcmBuffer* result = nodeOutputPort->PullOutputData();
    EXPECT_EQ(*(result->GetPcmDataBuffer()), 0.0f);
    node_->RemoveTap(AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    node_->DisConnect(mockInputNode_);
    EXPECT_EQ(inputNodeOutputPort->GetInputNum(), 0);
    testing::Mock::VerifyAndClearExpectations(mockInputNode_.get());
    mockInputNode_.reset();
    inputNodeOutputPort.reset();
    node_->SetNodeEnableStatus(NODE_ENABLE);
}

HWTEST_F(AudioSuiteProcessNodeTest, DoProcessWithFinishedPcmBufferTest, TestSize.Level0)
{
    std::unique_ptr<AudioSuitePcmBuffer> buffer = std::make_unique<AudioSuitePcmBuffer>(SAMPLE_RATE_48000,
        static_cast<uint32_t>(4), AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    buffer->SetIsFinished(true);
    std::shared_ptr<MockInputNode> mockInputNode_ = std::make_unique<MockInputNode>();
    std::shared_ptr<OutputPort<AudioSuitePcmBuffer*>> inputNodeOutputPort =
        std::make_shared<OutputPort<AudioSuitePcmBuffer*>>(mockInputNode_);
    inputNodeOutputPort->WriteDataToOutput(buffer.get());
    EXPECT_CALL(*mockInputNode_, DoProcess()).Times(1).WillRepeatedly(::testing::Return(SUCCESS));
    EXPECT_CALL(*mockInputNode_, GetOutputPort(::testing::_))
        .Times(1).WillRepeatedly(::testing::Return(inputNodeOutputPort));
    node_->Connect(mockInputNode_, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(inputNodeOutputPort->GetInputNum(), 1);
    std::shared_ptr<OutputPort<AudioSuitePcmBuffer*>> nodeOutputPort =
        node_->GetOutputPort(AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    node_->InstallTap(AUDIO_NODE_DEFAULT_OUTPORT_TYPE, nullptr);
    AudioSuitePcmBuffer* result = nodeOutputPort->PullOutputData();
    EXPECT_EQ(*(result->GetPcmDataBuffer()), 1.0f);
    EXPECT_EQ(result->GetIsFinished(), true);
    EXPECT_EQ(node_->GetAudioNodeDataFinishedFlag(), true);
    AudioSuitePcmBuffer* resultWhenNodeFinished = nodeOutputPort->PullOutputData();
    EXPECT_EQ(resultWhenNodeFinished, nullptr);
    node_->RemoveTap(AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    node_->DisConnect(mockInputNode_);
    EXPECT_EQ(inputNodeOutputPort->GetInputNum(), 0);
    testing::Mock::VerifyAndClearExpectations(mockInputNode_.get());
    mockInputNode_.reset();
    inputNodeOutputPort.reset();
}

HWTEST_F(AudioSuiteProcessNodeTest, DoProcessInstallTapTest, TestSize.Level0)
{
    std::unique_ptr<AudioSuitePcmBuffer> buffer = std::make_unique<AudioSuitePcmBuffer>(SAMPLE_RATE_48000,
        static_cast<uint32_t>(4), AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    std::shared_ptr<MockInputNode> mockInputNode_ = std::make_unique<MockInputNode>();
    std::shared_ptr<OutputPort<AudioSuitePcmBuffer*>> inputNodeOutputPort =
        std::make_shared<OutputPort<AudioSuitePcmBuffer*>>(mockInputNode_);
    inputNodeOutputPort->WriteDataToOutput(buffer.get());
    EXPECT_CALL(*mockInputNode_, DoProcess()).Times(1).WillRepeatedly(::testing::Return(SUCCESS));
    EXPECT_CALL(*mockInputNode_, GetOutputPort(::testing::_))
        .Times(1).WillRepeatedly(::testing::Return(inputNodeOutputPort));
    node_->Connect(mockInputNode_, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(inputNodeOutputPort->GetInputNum(), 1);
    std::shared_ptr<OutputPort<AudioSuitePcmBuffer*>> nodeOutputPort =
        node_->GetOutputPort(AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    std::shared_ptr<SuiteNodeReadTapDataCallback> testCallback = std::make_shared<TestReadTapCallBack>();
    EXPECT_FALSE(TestReadTapCallBack::testFlag);
    node_->InstallTap(AUDIO_NODE_DEFAULT_OUTPORT_TYPE, testCallback);
    AudioSuitePcmBuffer* result = nodeOutputPort->PullOutputData();
    EXPECT_TRUE(TestReadTapCallBack::testFlag);
    EXPECT_EQ(*(result->GetPcmDataBuffer()), 1.0f);
    node_->RemoveTap(AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    node_->DisConnect(mockInputNode_);
    EXPECT_EQ(inputNodeOutputPort->GetInputNum(), 0);
    testing::Mock::VerifyAndClearExpectations(mockInputNode_.get());
    mockInputNode_.reset();
    inputNodeOutputPort.reset();
}
}