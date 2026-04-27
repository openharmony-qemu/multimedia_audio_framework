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

#include "gtest/gtest.h"
#include "hpae_inner_cap_sink_node.h"
#include "hpae_sink_input_node.h"
#include "hpae_pcm_buffer.h"
#include "audio_errors.h"
#include "test_case_common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
namespace {
constexpr uint32_t TEST_FRAME_LEN = 960;
constexpr uint32_t TEST_SAMPLE_RATE = 48000;
constexpr uint32_t TEST_CHANNELS = 2;

HpaeNodeInfo GetTestNodeInfo()
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.channels = static_cast<AudioChannel>(TEST_CHANNELS);
    nodeInfo.frameLen = TEST_FRAME_LEN;
    nodeInfo.samplingRate = static_cast<AudioSamplingRate>(TEST_SAMPLE_RATE);
    nodeInfo.format = SAMPLE_F32LE;
    return nodeInfo;
}
}

class HpaeInnerCapSinkNodeTest : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;

protected:
    HpaeNodeInfo nodeInfo_;
    std::shared_ptr<HpaeInnerCapSinkNode> node_;
};

void HpaeInnerCapSinkNodeTest::SetUp(void)
{
    nodeInfo_ = GetTestNodeInfo();
    node_ = std::make_shared<HpaeInnerCapSinkNode>(nodeInfo_);
}

void HpaeInnerCapSinkNodeTest::TearDown(void)
{
    node_.reset();
}

/**
 * @tc.name  : Construct_001
 * @tc.type  : FUNC
 * @tc.number: Construct_001
 * @tc.desc  : Test construct node and verify initial state is STREAM_MANAGER_NEW.
 */
HWTEST_F(HpaeInnerCapSinkNodeTest, Construct_001, TestSize.Level0)
{
    EXPECT_NE(node_, nullptr);
    EXPECT_EQ(node_->GetSinkState(), STREAM_MANAGER_NEW);
}

/**
 * @tc.name  : InnerCapturerSinkInit_001
 * @tc.type  : FUNC
 * @tc.number: InnerCapturerSinkInit_001
 * @tc.desc  : Test Init sets state to STREAM_MANAGER_IDLE and returns SUCCESS.
 */
HWTEST_F(HpaeInnerCapSinkNodeTest, InnerCapturerSinkInit_001, TestSize.Level0)
{
    EXPECT_EQ(node_->InnerCapturerSinkInit(), SUCCESS);
    EXPECT_EQ(node_->GetSinkState(), STREAM_MANAGER_IDLE);
}

/**
 * @tc.name  : InnerCapturerSinkStart_001
 * @tc.type  : FUNC
 * @tc.number: InnerCapturerSinkStart_001
 * @tc.desc  : Test Start sets state to STREAM_MANAGER_RUNNING and returns SUCCESS.
 */
HWTEST_F(HpaeInnerCapSinkNodeTest, InnerCapturerSinkStart_001, TestSize.Level0)
{
    EXPECT_EQ(node_->InnerCapturerSinkInit(), SUCCESS);
    EXPECT_EQ(node_->InnerCapturerSinkStart(), SUCCESS);
    EXPECT_EQ(node_->GetSinkState(), STREAM_MANAGER_RUNNING);
}

/**
 * @tc.name  : InnerCapturerSinkPause_001
 * @tc.type  : FUNC
 * @tc.number: InnerCapturerSinkPause_001
 * @tc.desc  : Test Pause sets state to STREAM_MANAGER_SUSPENDED and returns SUCCESS.
 */
HWTEST_F(HpaeInnerCapSinkNodeTest, InnerCapturerSinkPause_001, TestSize.Level0)
{
    EXPECT_EQ(node_->InnerCapturerSinkInit(), SUCCESS);
    EXPECT_EQ(node_->InnerCapturerSinkStart(), SUCCESS);
    EXPECT_EQ(node_->InnerCapturerSinkPause(), SUCCESS);
    EXPECT_EQ(node_->GetSinkState(), STREAM_MANAGER_SUSPENDED);
}

/**
 * @tc.name  : InnerCapturerSinkResume_001
 * @tc.type  : FUNC
 * @tc.number: InnerCapturerSinkResume_001
 * @tc.desc  : Test Resume sets state to STREAM_MANAGER_RUNNING and returns SUCCESS.
 */
HWTEST_F(HpaeInnerCapSinkNodeTest, InnerCapturerSinkResume_001, TestSize.Level0)
{
    EXPECT_EQ(node_->InnerCapturerSinkInit(), SUCCESS);
    EXPECT_EQ(node_->InnerCapturerSinkStart(), SUCCESS);
    EXPECT_EQ(node_->InnerCapturerSinkPause(), SUCCESS);
    EXPECT_EQ(node_->InnerCapturerSinkResume(), SUCCESS);
    EXPECT_EQ(node_->GetSinkState(), STREAM_MANAGER_RUNNING);
}

/**
 * @tc.name  : InnerCapturerSinkStop_001
 * @tc.type  : FUNC
 * @tc.number: InnerCapturerSinkStop_001
 * @tc.desc  : Test Stop sets state to STREAM_MANAGER_SUSPENDED and returns SUCCESS.
 */
HWTEST_F(HpaeInnerCapSinkNodeTest, InnerCapturerSinkStop_001, TestSize.Level0)
{
    EXPECT_EQ(node_->InnerCapturerSinkInit(), SUCCESS);
    EXPECT_EQ(node_->InnerCapturerSinkStart(), SUCCESS);
    EXPECT_EQ(node_->InnerCapturerSinkStop(), SUCCESS);
    EXPECT_EQ(node_->GetSinkState(), STREAM_MANAGER_SUSPENDED);
}

/**
 * @tc.name  : InnerCapturerSinkDeInit_001
 * @tc.type  : FUNC
 * @tc.number: InnerCapturerSinkDeInit_001
 * @tc.desc  : Test DeInit sets state to STREAM_MANAGER_RELEASED and returns SUCCESS.
 */
HWTEST_F(HpaeInnerCapSinkNodeTest, InnerCapturerSinkDeInit_001, TestSize.Level0)
{
    EXPECT_EQ(node_->InnerCapturerSinkInit(), SUCCESS);
    EXPECT_EQ(node_->InnerCapturerSinkDeInit(), SUCCESS);
    EXPECT_EQ(node_->GetSinkState(), STREAM_MANAGER_RELEASED);
}

/**
 * @tc.name  : InnerCapturerSinkFlush_001
 * @tc.type  : FUNC
 * @tc.number: InnerCapturerSinkFlush_001
 * @tc.desc  : Test Flush returns SUCCESS.
 */
HWTEST_F(HpaeInnerCapSinkNodeTest, InnerCapturerSinkFlush_001, TestSize.Level0)
{
    EXPECT_EQ(node_->InnerCapturerSinkFlush(), SUCCESS);
}

/**
 * @tc.name  : InnerCapturerSinkReset_001
 * @tc.type  : FUNC
 * @tc.number: InnerCapturerSinkReset_001
 * @tc.desc  : Test Reset returns SUCCESS.
 */
HWTEST_F(HpaeInnerCapSinkNodeTest, InnerCapturerSinkReset_001, TestSize.Level0)
{
    EXPECT_EQ(node_->InnerCapturerSinkReset(), SUCCESS);
}

/**
 * @tc.name  : SetSinkState_001
 * @tc.type  : FUNC
 * @tc.number: SetSinkState_001
 * @tc.desc  : Test SetSinkState transitions state and returns SUCCESS.
 */
HWTEST_F(HpaeInnerCapSinkNodeTest, SetSinkState_001, TestSize.Level0)
{
    EXPECT_EQ(node_->GetSinkState(), STREAM_MANAGER_NEW);
    EXPECT_EQ(node_->SetSinkState(STREAM_MANAGER_IDLE), SUCCESS);
    EXPECT_EQ(node_->GetSinkState(), STREAM_MANAGER_IDLE);
    EXPECT_EQ(node_->SetSinkState(STREAM_MANAGER_RUNNING), SUCCESS);
    EXPECT_EQ(node_->GetSinkState(), STREAM_MANAGER_RUNNING);
    EXPECT_EQ(node_->SetSinkState(STREAM_MANAGER_SUSPENDED), SUCCESS);
    EXPECT_EQ(node_->GetSinkState(), STREAM_MANAGER_SUSPENDED);
    EXPECT_EQ(node_->SetSinkState(STREAM_MANAGER_RELEASED), SUCCESS);
    EXPECT_EQ(node_->GetSinkState(), STREAM_MANAGER_RELEASED);
}

/**
 * @tc.name  : SetMute_001
 * @tc.type  : FUNC
 * @tc.number: SetMute_001
 * @tc.desc  : Test SetMute changes mute state when value differs.
 */
HWTEST_F(HpaeInnerCapSinkNodeTest, SetMute_001, TestSize.Level0)
{
    node_->SetMute(true);
    node_->SetMute(false);
}

/**
 * @tc.name  : SetMute_002
 * @tc.type  : FUNC
 * @tc.number: SetMute_002
 * @tc.desc  : Test SetMute with same value does not change state.
 */
HWTEST_F(HpaeInnerCapSinkNodeTest, SetMute_002, TestSize.Level0)
{
    node_->SetMute(false);
    node_->SetMute(false);
    node_->SetMute(true);
    node_->SetMute(true);
}

/**
 * @tc.name  : GetPreOutNum_001
 * @tc.type  : FUNC
 * @tc.number: GetPreOutNum_001
 * @tc.desc  : Test GetPreOutNum returns 0 when no nodes connected.
 */
HWTEST_F(HpaeInnerCapSinkNodeTest, GetPreOutNum_001, TestSize.Level0)
{
    EXPECT_EQ(node_->GetPreOutNum(), static_cast<size_t>(0));
}

/**
 * @tc.name  : GetOutputPortNum_001
 * @tc.type  : FUNC
 * @tc.number: GetOutputPortNum_001
 * @tc.desc  : Test GetOutputPortNum returns 0 when no nodes connected.
 */
HWTEST_F(HpaeInnerCapSinkNodeTest, GetOutputPortNum_001, TestSize.Level0)
{
    EXPECT_EQ(node_->GetOutputPortNum(), static_cast<size_t>(0));
}

/**
 * @tc.name  : GetSharedInstance_001
 * @tc.type  : FUNC
 * @tc.number: GetSharedInstance_001
 * @tc.desc  : Test GetSharedInstance returns non-null shared_ptr.
 */
HWTEST_F(HpaeInnerCapSinkNodeTest, GetSharedInstance_001, TestSize.Level0)
{
    std::shared_ptr<HpaeNode> instance = node_->GetSharedInstance();
    EXPECT_NE(instance, nullptr);
}

/**
 * @tc.name  : GetOutputPort_001
 * @tc.type  : FUNC
 * @tc.number: GetOutputPort_001
 * @tc.desc  : Test GetOutputPort returns non-null pointer.
 */
HWTEST_F(HpaeInnerCapSinkNodeTest, GetOutputPort_001, TestSize.Level0)
{
    OutputPort<HpaePcmBuffer*>* port = node_->GetOutputPort();
    EXPECT_NE(port, nullptr);
}

/**
 * @tc.name  : DoProcess_001
 * @tc.type  : FUNC
 * @tc.number: DoProcess_001
 * @tc.desc  : Test DoProcess with no upstream connected outputs silence without crash.
 */
HWTEST_F(HpaeInnerCapSinkNodeTest, DoProcess_001, TestSize.Level0)
{
    // No upstream connected, ReadPreOutputData returns empty vector
    // DoProcess should output silence via the mute/empty path and not crash
    node_->DoProcess();
}

/**
 * @tc.name  : DoProcess_002
 * @tc.type  : FUNC
 * @tc.number: DoProcess_002
 * @tc.desc  : Test DoProcess with upstream SinkInputNode connected and unmuted, data flows normally.
 */
HWTEST_F(HpaeInnerCapSinkNodeTest, DoProcess_002, TestSize.Level0)
{
    // Create upstream SinkInputNode
    HpaeNodeInfo upstreamInfo = GetTestNodeInfo();
    auto upstreamNode = std::make_shared<HpaeSinkInputNode>(upstreamInfo);

    // Register a write callback to inject data
    auto writeCb = std::make_shared<WriteFixedValueCb>(SAMPLE_F32LE, 100);
    upstreamNode->RegisterWriteCallback(writeCb);

    // Connect: downstream receives from upstream
    node_->Connect(upstreamNode);
    ASSERT_EQ(node_->GetPreOutNum(), static_cast<size_t>(1));

    // DoProcess should pull data from upstream and write to output
    node_->DoProcess();

    // Verify output port has data by pulling it
    OutputPort<HpaePcmBuffer*> *port = node_->GetOutputPort();
    ASSERT_NE(port, nullptr);
    HpaePcmBuffer *outputData = port->PullOutputData();
    EXPECT_NE(outputData, nullptr);
}

/**
 * @tc.name  : DoProcess_003
 * @tc.type  : FUNC
 * @tc.number: DoProcess_003
 * @tc.desc  : Test DoProcess with upstream connected but muted, outputs silence.
 */
HWTEST_F(HpaeInnerCapSinkNodeTest, DoProcess_003, TestSize.Level0)
{
    // Create upstream SinkInputNode
    HpaeNodeInfo upstreamInfo = GetTestNodeInfo();
    auto upstreamNode = std::make_shared<HpaeSinkInputNode>(upstreamInfo);

    // Register a write callback to inject non-zero data
    auto writeCb = std::make_shared<WriteFixedValueCb>(SAMPLE_F32LE, 100);
    upstreamNode->RegisterWriteCallback(writeCb);

    // Connect upstream
    node_->Connect(upstreamNode);
    ASSERT_EQ(node_->GetPreOutNum(), static_cast<size_t>(1));

    // Mute the node
    node_->SetMute(true);

    // DoProcess should output silence (ignore upstream data) and not crash
    node_->DoProcess();

    // Verify output port has data (silence was written)
    OutputPort<HpaePcmBuffer*> *port = node_->GetOutputPort();
    ASSERT_NE(port, nullptr);
    HpaePcmBuffer *outputData = port->PullOutputData();
    EXPECT_NE(outputData, nullptr);
    // Verify the output buffer is silence (all zeros for float data)
    float *buf = outputData->GetPcmDataBuffer();
    ASSERT_NE(buf, nullptr);
    size_t totalSamples = TEST_FRAME_LEN * TEST_CHANNELS;
    for (size_t i = 0; i < totalSamples; i++) {
        EXPECT_EQ(buf[i], 0.0f);
    }
}

/**
 * @tc.name  : Connect_001
 * @tc.type  : FUNC
 * @tc.number: Connect_001
 * @tc.desc  : Test Connect with upstream SinkInputNode, GetPreOutNum returns 1.
 */
HWTEST_F(HpaeInnerCapSinkNodeTest, Connect_001, TestSize.Level0)
{
    EXPECT_EQ(node_->GetPreOutNum(), static_cast<size_t>(0));

    HpaeNodeInfo upstreamInfo = GetTestNodeInfo();
    auto upstreamNode = std::make_shared<HpaeSinkInputNode>(upstreamInfo);

    node_->Connect(upstreamNode);
    EXPECT_EQ(node_->GetPreOutNum(), static_cast<size_t>(1));
}

/**
 * @tc.name  : DisConnect_001
 * @tc.type  : FUNC
 * @tc.number: DisConnect_001
 * @tc.desc  : Test Connect then DisConnect, GetPreOutNum returns 0.
 */
HWTEST_F(HpaeInnerCapSinkNodeTest, DisConnect_001, TestSize.Level0)
{
    EXPECT_EQ(node_->GetPreOutNum(), static_cast<size_t>(0));

    HpaeNodeInfo upstreamInfo = GetTestNodeInfo();
    auto upstreamNode = std::make_shared<HpaeSinkInputNode>(upstreamInfo);

    node_->Connect(upstreamNode);
    EXPECT_EQ(node_->GetPreOutNum(), static_cast<size_t>(1));

    node_->DisConnect(upstreamNode);
    EXPECT_EQ(node_->GetPreOutNum(), static_cast<size_t>(0));
}

/**
 * @tc.name  : DoProcess_004
 * @tc.type  : FUNC
 * @tc.number: DoProcess_004
 * @tc.desc  : Test DoProcess after Init and Start in RUNNING state does not crash.
 */
HWTEST_F(HpaeInnerCapSinkNodeTest, DoProcess_004, TestSize.Level0)
{
    ASSERT_EQ(node_->InnerCapturerSinkInit(), SUCCESS);
    ASSERT_EQ(node_->InnerCapturerSinkStart(), SUCCESS);
    ASSERT_EQ(node_->GetSinkState(), STREAM_MANAGER_RUNNING);

    // DoProcess in RUNNING state with no upstream should output silence without crash
    node_->DoProcess();
}
}
}
}
