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
        EXPECT_EQ(*((float*)data + i), 0);
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
    EXPECT_EQ(sinkOutputNode->GetRenderSinkInstance(deviceClass, deviceNetId), 0);
    sinkOutputNode->DoProcess();
    TestRendererRenderFrame(sinkOutputNode->GetRenderFrameData(),
        nodeInfo.frameLen * nodeInfo.channels * GetSizeFromFormat(nodeInfo.format));
}
}
}
}