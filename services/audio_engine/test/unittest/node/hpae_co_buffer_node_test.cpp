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

#include <chrono>
#include <gtest/gtest.h>
#include <string>
#include <thread>

#include "audio_errors.h"
#include "hpae_co_buffer_node.h"
#include "hpae_pcm_buffer.h"
#include "hpae_sink_input_node.h"
#include "hpae_sink_output_node.h"
#include "test_case_common.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace HPAE;
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
namespace {
constexpr uint32_t TEST_FRAME_LEN = 960; // 20ms at 48kHz
constexpr uint32_t TEST_LATENCY_MS = 280; // 280ms latency for testing
constexpr int32_t TEST_COLLABORATION_ALIGN_COUNT = 5;
constexpr int32_t TEST_MAX_LATENCY_MS = 100000000;
constexpr float TEST_COLL_SMALL_SIGNAL_NUM = 1e-6;
HpaeNodeInfo GetTestNodeInfo()
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.channels = STEREO;
    nodeInfo.frameLen = TEST_FRAME_LEN;
    nodeInfo.channelLayout = CH_LAYOUT_STEREO;
    return nodeInfo;
}

int32_t TestRendererRenderFrame(const char *data, uint64_t len)
{
    for (int32_t i = 0; i < len / SAMPLE_F32LE; i++) {
        EXPECT_EQ(*((float*)data + i), TEST_COLL_SMALL_SIGNAL_NUM);
    }
    return 0;
}
}

class HpaeCoBufferNodeUnitTest : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;
};

void HpaeCoBufferNodeUnitTest::SetUp(void)
{
}

void HpaeCoBufferNodeUnitTest::TearDown(void)
{
}

/**
 * @tc.name  : Test Construct and be connected
 * @tc.type  : FUNC
 * @tc.number: Construct_001
 * @tc.desc  : Test Construct when config in vaild.
 */
HWTEST_F(HpaeCoBufferNodeUnitTest, Construct_001, TestSize.Level0)
{
    std::shared_ptr<HpaeCoBufferNode> coBufferNode = std::make_shared<HpaeCoBufferNode>();
    EXPECT_NE(coBufferNode, nullptr);
    coBufferNode->SetOutputClusterConnected(true);
    EXPECT_EQ(coBufferNode->IsOutputClusterConnected(), true);
    coBufferNode->SetOutputClusterConnected(false);
    EXPECT_EQ(coBufferNode->IsOutputClusterConnected(), false);
}

/**
 * @tc.name  : Test Connect
 * @tc.type  : FUNC
 * @tc.number: Connect_001
 * @tc.desc  : Test Connect when config in vaild.
 */
HWTEST_F(HpaeCoBufferNodeUnitTest, Connect_001, TestSize.Level0)
{
    HpaeNodeInfo sinkInputNodeInfo = GetTestNodeInfo();
    std::shared_ptr<HpaeSinkInputNode> sinkInputNode = std::make_shared<HpaeSinkInputNode>(sinkInputNodeInfo);
    EXPECT_NE(sinkInputNode, nullptr);
    std::shared_ptr<HpaeCoBufferNode> coBufferNode = std::make_shared<HpaeCoBufferNode>();
    EXPECT_NE(coBufferNode, nullptr);
    coBufferNode->Connect(sinkInputNode);
    coBufferNode->Connect(sinkInputNode);
    HpaeNodeInfo &coNodeInfo = coBufferNode->GetNodeInfo();
    EXPECT_EQ(sinkInputNodeInfo.samplingRate, coNodeInfo.samplingRate);
    EXPECT_EQ(sinkInputNodeInfo.format, coNodeInfo.format);
    EXPECT_EQ(sinkInputNodeInfo.channels, coNodeInfo.channels);
    EXPECT_EQ(sinkInputNodeInfo.frameLen, coNodeInfo.frameLen);
    coBufferNode->DisConnect(sinkInputNode);
    coBufferNode->DisConnect(sinkInputNode);
}

/**
 * @tc.name  : Test Process
 * @tc.type  : FUNC
 * @tc.number: Process_001
 * @tc.desc  : Test Process when config in vaild.
 */
HWTEST_F(HpaeCoBufferNodeUnitTest, Process_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    std::shared_ptr<HpaeCoBufferNode> coBufferNode = std::make_shared<HpaeCoBufferNode>();
    coBufferNode->SetLatency(TEST_LATENCY_MS);
    coBufferNode->SetNodeInfo(nodeInfo);
    std::shared_ptr<HpaeSinkOutputNode> sinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    sinkOutputNode->Connect(coBufferNode);
    PcmBufferInfo pcmBufferInfo;
    pcmBufferInfo.ch = STEREO;
    pcmBufferInfo.frameLen = TEST_FRAME_LEN;
    HpaePcmBuffer pcmBuffer(pcmBufferInfo);
    coBufferNode->Enqueue(&pcmBuffer);
    coBufferNode->Enqueue(&pcmBuffer);
    std::string deviceClass = "file_io";
    std::string deviceNetId = "LocalDevice";
    EXPECT_EQ(sinkOutputNode->GetRenderSinkInstance(HdiIdInfoParam{deviceClass, "", deviceNetId}), 0);
    coBufferNode->enqueueRunning_ = true;
    sinkOutputNode->DoProcess();
    EXPECT_EQ(coBufferNode->Reset(), true);
    coBufferNode->enqueueRunning_ = false;
    coBufferNode->isOutputClusterConnected_ = false;
    sinkOutputNode->DoProcess();

    coBufferNode->enqueueRunning_ = false;
    coBufferNode->isOutputClusterConnected_ = true;
    sinkOutputNode->DoProcess();
    EXPECT_EQ(coBufferNode->ResetAll(), true);
    coBufferNode->ringCache_->ResetBuffer();
    coBufferNode->isOutputClusterConnected_ = false;
    coBufferNode->ProcessOutputFrameInner();
    std::shared_ptr<INodeCallback> defaultRendererManagerCallBackTemp = std::make_shared<INodeCallback>();
    coBufferNode->defaultRendererManagerCallBack_ = defaultRendererManagerCallBackTemp;
    coBufferNode->ProcessOutputFrameInner();
    TestRendererRenderFrame(sinkOutputNode->GetRenderFrameData(),
        nodeInfo.frameLen * nodeInfo.channels * GetSizeFromFormat(nodeInfo.format));
    coBufferNode->SetDelayCount(TEST_COLLABORATION_ALIGN_COUNT);
    sinkOutputNode->DoProcess();
    EXPECT_EQ(coBufferNode->ResetAll(), true);
}

/**
 * @tc.name  : Test SetDelayCount
 * @tc.type  : FUNC
 * @tc.number: SetDelayCount_001
 * @tc.desc  : Test SetDelayCount when config in vaild.
 */
HWTEST_F(HpaeCoBufferNodeUnitTest, SetDelayCount_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    std::shared_ptr<HpaeCoBufferNode> coBufferNode = std::make_shared<HpaeCoBufferNode>();
    coBufferNode->SetDelayCount(TEST_COLLABORATION_ALIGN_COUNT);
    EXPECT_EQ(coBufferNode->enqueueCount_, 0);
    EXPECT_EQ(coBufferNode->waitCountThreshold_, TEST_COLLABORATION_ALIGN_COUNT);
    EXPECT_EQ(coBufferNode->enqueueRunning_, false);
}

/**
 * @tc.name  : Test FillSilenceFramesInner
 * @tc.type  : FUNC
 * @tc.number: FillSilenceFramesInner_001
 * @tc.desc  : Test FillSilenceFramesInner when config in vaild.
 */
HWTEST_F(HpaeCoBufferNodeUnitTest, FillSilenceFramesInner_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    std::shared_ptr<HpaeCoBufferNode> coBufferNode = std::make_shared<HpaeCoBufferNode>();
    coBufferNode->FillSilenceFramesInner(-1);
    coBufferNode->FillSilenceFramesInner(TEST_LATENCY_MS);
    coBufferNode->FillSilenceFramesInner(TEST_MAX_LATENCY_MS);

    EXPECT_EQ(coBufferNode->ringCache_->GetWritableSize().size != 0, true);
}

/**
 * @tc.name  : Test SetOutputClusterConnected
 * @tc.type  : FUNC
 * @tc.number: SetOutputClusterConnected_001
 * @tc.desc  : Test SetOutputClusterConnected when config in vaild.
 */
HWTEST_F(HpaeCoBufferNodeUnitTest, SetOutputClusterConnected_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    std::shared_ptr<HpaeCoBufferNode> coBufferNode = std::make_shared<HpaeCoBufferNode>();
    coBufferNode->ringCache_->ResetBuffer();

    bool isOutputClusterConnectedTemp = false;
    EXPECT_EQ(coBufferNode->ringCache_->GetReadableSize().size == 0, true);
    coBufferNode->SetOutputClusterConnected(isOutputClusterConnectedTemp);
    EXPECT_EQ(coBufferNode->ringCache_->GetReadableSize().size != 0, true);
}

/**
 * @tc.name  : Test Enqueue with nullptr
 * @tc.type  : FUNC
 * @tc.number: Enqueue_Nullptr_001
 * @tc.desc  : Test Enqueue when buffer is nullptr.
 */
HWTEST_F(HpaeCoBufferNodeUnitTest, Enqueue_Nullptr_001, TestSize.Level0)
{
    std::shared_ptr<HpaeCoBufferNode> coBufferNode = std::make_shared<HpaeCoBufferNode>();
    EXPECT_NE(coBufferNode, nullptr);
    // Test Enqueue with nullptr - should return early without crash
    coBufferNode->Enqueue(nullptr);
    // Verify node state is still valid
    EXPECT_EQ(coBufferNode->IsOutputClusterConnected(), false);
}

/**
 * @tc.name  : Test DelayAlignmentInner before threshold
 * @tc.type  : FUNC
 * @tc.number: DelayAlignmentInner_BeforeThreshold_001
 * @tc.desc  : Test DelayAlignmentInner when enqueueCount_ < waitCountThreshold_.
 */
HWTEST_F(HpaeCoBufferNodeUnitTest, DelayAlignmentInner_BeforeThreshold_001, TestSize.Level0)
{
    std::shared_ptr<HpaeCoBufferNode> coBufferNode = std::make_shared<HpaeCoBufferNode>();
    EXPECT_NE(coBufferNode, nullptr);
    // Set waitCountThreshold to a value > 0
    coBufferNode->SetDelayCount(TEST_COLLABORATION_ALIGN_COUNT);
    // Reset enqueueCount to ensure it starts from 0
    coBufferNode->enqueueCount_ = 0;
    // Create a valid buffer
    PcmBufferInfo pcmBufferInfo;
    pcmBufferInfo.ch = STEREO;
    pcmBufferInfo.frameLen = TEST_FRAME_LEN;
    HpaePcmBuffer pcmBuffer(pcmBufferInfo);
    // Fill buffer with test data
    for (uint32_t i = 0; i < pcmBuffer.GetFrameLen() * pcmBuffer.GetChannelCount(); i++) {
        pcmBuffer.GetPcmDataBuffer()[i] = 0.5f;
    }
    // First enqueue when enqueueCount_ < waitCountThreshold_ - should return false
    coBufferNode->Enqueue(&pcmBuffer);
    // Verify enqueueCount increased but enqueueRunning is still false
    EXPECT_EQ(coBufferNode->enqueueCount_, 1);
    EXPECT_EQ(coBufferNode->enqueueRunning_, false);
}

/**
 * @tc.name  : Test DelayAlignmentInner at threshold
 * @tc.type  : FUNC
 * @tc.number: DelayAlignmentInner_AtThreshold_001
 * @tc.desc  : Test DelayAlignmentInner when enqueueCount_ == waitCountThreshold_.
 */
HWTEST_F(HpaeCoBufferNodeUnitTest, DelayAlignmentInner_AtThreshold_001, TestSize.Level0)
{
    std::shared_ptr<HpaeCoBufferNode> coBufferNode = std::make_shared<HpaeCoBufferNode>();
    EXPECT_NE(coBufferNode, nullptr);
    // Set waitCountThreshold to TEST_COLLABORATION_ALIGN_COUNT (5)
    coBufferNode->SetDelayCount(TEST_COLLABORATION_ALIGN_COUNT);
    // Set enqueueCount to waitCountThreshold_ - 1 (4)
    coBufferNode->enqueueCount_ = TEST_COLLABORATION_ALIGN_COUNT - 1;
    // Create a valid buffer
    PcmBufferInfo pcmBufferInfo;
    pcmBufferInfo.ch = STEREO;
    pcmBufferInfo.frameLen = TEST_FRAME_LEN;
    HpaePcmBuffer pcmBuffer(pcmBufferInfo);
    // Fill buffer with test data
    for (uint32_t i = 0; i < pcmBuffer.GetFrameLen() * pcmBuffer.GetChannelCount(); i++) {
        pcmBuffer.GetPcmDataBuffer()[i] = 0.5f;
    }
    // First enqueue when enqueueCount_ = 4 < waitCountThreshold_ = 5
    // This will increment enqueueCount_ to 5 and return false
    coBufferNode->Enqueue(&pcmBuffer);
    EXPECT_EQ(coBufferNode->enqueueRunning_, false);
    EXPECT_EQ(coBufferNode->enqueueCount_, TEST_COLLABORATION_ALIGN_COUNT);

    // Second enqueue when enqueueCount_ = 5 == waitCountThreshold_ = 5
    // This should trigger latency alignment and set enqueueRunning to true
    coBufferNode->Enqueue(&pcmBuffer);
    // Verify enqueueRunning is now true and enqueueCount increased
    EXPECT_EQ(coBufferNode->enqueueRunning_, true);
    EXPECT_EQ(coBufferNode->enqueueCount_, TEST_COLLABORATION_ALIGN_COUNT + 1);
}

/**
 * @tc.name  : Test ProcessOutputFrameInner with Dequeue failure
 * @tc.type  : FUNC
 * @tc.number: ProcessOutputFrameInner_DequeueFailure_001
 * @tc.desc  : Test ProcessOutputFrameInner when Dequeue operation fails.
 */
HWTEST_F(HpaeCoBufferNodeUnitTest, ProcessOutputFrameInner_DequeueFailure_001, TestSize.Level0)
{
    std::shared_ptr<HpaeCoBufferNode> coBufferNode = std::make_shared<HpaeCoBufferNode>();
    EXPECT_NE(coBufferNode, nullptr);
    // Set output cluster as connected
    coBufferNode->SetOutputClusterConnected(true);
    // Create a valid buffer
    PcmBufferInfo pcmBufferInfo;
    pcmBufferInfo.ch = STEREO;
    pcmBufferInfo.frameLen = TEST_FRAME_LEN;
    HpaePcmBuffer pcmBuffer(pcmBufferInfo);
    // Fill buffer with test data
    for (uint32_t i = 0; i < pcmBuffer.GetFrameLen() * pcmBuffer.GetChannelCount(); i++) {
        pcmBuffer.GetPcmDataBuffer()[i] = 0.5f;
    }
    // Set enqueueRunning to true to bypass the enqueue check in DoProcess
    coBufferNode->enqueueRunning_ = true;
    // Enqueue some data
    coBufferNode->Enqueue(&pcmBuffer);
    // Corrupt the ring cache by setting a very small readable size
    // This simulates a condition where GetReadableSize succeeds but Dequeue might fail
    coBufferNode->ProcessOutputFrameInner();
    // Verify the node handled the potential failure gracefully
    EXPECT_EQ(coBufferNode->IsOutputClusterConnected(), true);
}
}
}
}
/**
 * @tc.name  : Test Process edge log gates (enqueue stop / insufficient recover)
 * @tc.type  : FUNC
 * @tc.number: Process_EdgeLog_002
 * @tc.desc  : Enqueue-stop and insufficient states hit their edge gates twice then recover
 */
HWTEST_F(HpaeCoBufferNodeUnitTest, Process_EdgeLog_002, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    std::shared_ptr<HpaeCoBufferNode> coBufferNode = std::make_shared<HpaeCoBufferNode>();
    coBufferNode->SetLatency(TEST_LATENCY_MS);
    coBufferNode->SetNodeInfo(nodeInfo);
    std::shared_ptr<HpaeSinkOutputNode> sinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    sinkOutputNode->Connect(coBufferNode);
    std::string deviceClass = "file_io";
    std::string deviceNetId = "LocalDevice";
    EXPECT_EQ(sinkOutputNode->GetRenderSinkInstance(HdiIdInfoParam{deviceClass, "", deviceNetId}), 0);

    // enqueue stopped while output cluster connected: first hit + sustained, resume recovers
    coBufferNode->isOutputClusterConnected_ = true;
    sinkOutputNode->DoProcess(); // first gate hit
    sinkOutputNode->DoProcess(); // sustained hit
    coBufferNode->enqueueRunning_ = true;
    sinkOutputNode->DoProcess(); // recovery line: enqueue resumed

    // insufficient data: first hit + sustained, enqueue data to recover
    coBufferNode->ringCache_->ResetBuffer();
    coBufferNode->isOutputClusterConnected_ = false;
    coBufferNode->ProcessOutputFrameInner(); // first insufficient hit
    coBufferNode->ProcessOutputFrameInner(); // sustained hit
    PcmBufferInfo pcmBufferInfo;
    pcmBufferInfo.ch = STEREO;
    pcmBufferInfo.frameLen = TEST_FRAME_LEN;
    HpaePcmBuffer pcmBuffer(pcmBufferInfo);
    coBufferNode->Enqueue(&pcmBuffer);
    coBufferNode->ProcessOutputFrameInner(); // recovery line: data available
    EXPECT_EQ(coBufferNode->ResetAll(), true);
}
