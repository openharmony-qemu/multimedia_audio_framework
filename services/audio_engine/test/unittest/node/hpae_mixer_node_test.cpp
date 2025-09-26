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
#include <cmath>
#include <memory>
#include "hpae_sink_input_node.h"
#include "hpae_mixer_node.h"
#include "hpae_sink_output_node.h"
#include "hpae_source_input_cluster.h"
#include "test_case_common.h"
#include "audio_errors.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace HPAE;
using namespace testing::ext;
using namespace testing;
namespace {
static constexpr int32_t TEST_VALUE1 = 100;
static constexpr int32_t TEST_VALUE2 = 200;
static constexpr uint32_t TEST_ID = 1243;
static constexpr uint32_t TEST_FRAMELEN = 960;
class HpaeMixerNodeTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void HpaeMixerNodeTest::SetUp()
{}

void HpaeMixerNodeTest::TearDown()
{}

static int32_t g_testValue = 0;

HWTEST_F(HpaeMixerNodeTest, constructHpaeMixerNode, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    std::shared_ptr<HpaeMixerNode> hpaeMixerNode = std::make_shared<HpaeMixerNode>(nodeInfo);
    EXPECT_EQ(hpaeMixerNode->GetSampleRate(), nodeInfo.samplingRate);
    EXPECT_EQ(hpaeMixerNode->GetFrameLen(), nodeInfo.frameLen);
    EXPECT_EQ(hpaeMixerNode->GetChannelCount(), nodeInfo.channels);
    EXPECT_EQ(hpaeMixerNode->GetBitWidth(), nodeInfo.format);
    HpaeNodeInfo &retNi = hpaeMixerNode->GetNodeInfo();
    EXPECT_EQ(retNi.samplingRate, nodeInfo.samplingRate);
    EXPECT_EQ(retNi.frameLen, nodeInfo.frameLen);
    EXPECT_EQ(retNi.channels, nodeInfo.channels);
    EXPECT_EQ(retNi.format, nodeInfo.format);
}
static int32_t TestRendererRenderFrame(const char *data, uint64_t len)
{
    for (int32_t i = 0; i < len / SAMPLE_F32LE; i++) {
        float diff = *((float*)data + i) - g_testValue;
        EXPECT_EQ(fabs(diff) < TEST_VALUE_PRESION, true);
    }
    return 0;
}

HWTEST_F(HpaeMixerNodeTest, testHpaePlayOutConnectNode, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    std::shared_ptr<HpaeSinkOutputNode> hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    std::shared_ptr<HpaeSinkInputNode> hpaeSinkInputNode0 = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    std::shared_ptr<HpaeSinkInputNode> hpaeSinkInputNode1 = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    std::shared_ptr<HpaeMixerNode> hpaeMixerNode = std::make_shared<HpaeMixerNode>(nodeInfo);
    hpaeMixerNode->Connect(hpaeSinkInputNode0);
    hpaeMixerNode->Connect(hpaeSinkInputNode1);
    std::cout << "hpaeMixerNode->GetPreOutNum():" << hpaeMixerNode->GetPreOutNum() << std::endl;
    EXPECT_EQ(hpaeSinkOutputNode->GetPreOutNum(), 0);
    hpaeSinkOutputNode->Connect(hpaeMixerNode);
    std::string deviceClass = "file_io";
    std::string deviceNetId = "LocalDevice";
    EXPECT_EQ(hpaeSinkOutputNode->GetPreOutNum(), 1);
    EXPECT_EQ(hpaeSinkOutputNode->GetRenderSinkInstance(deviceClass, deviceNetId), 0);
    EXPECT_EQ(hpaeSinkInputNode0.use_count(), 1 + 1);
    EXPECT_EQ(hpaeSinkInputNode1.use_count(), 1 + 1);
    EXPECT_EQ(hpaeMixerNode.use_count(), 1 + 1);
    g_testValue = 0;
    int32_t testValue = TEST_VALUE1;
    std::shared_ptr<WriteFixedValueCb> writeFixedValueCb0 =
        std::make_shared<WriteFixedValueCb>(SAMPLE_F32LE, testValue);
    g_testValue = g_testValue + testValue;
    hpaeSinkInputNode0->RegisterWriteCallback(writeFixedValueCb0);
    testValue = TEST_VALUE2;
    std::shared_ptr<WriteFixedValueCb> writeFixedValueCb1 =
        std::make_shared<WriteFixedValueCb>(SAMPLE_F32LE, testValue);
    g_testValue = g_testValue + testValue;
    hpaeSinkInputNode1->RegisterWriteCallback(writeFixedValueCb1);
    hpaeSinkOutputNode->DoProcess();
    TestRendererRenderFrame(hpaeSinkOutputNode->GetRenderFrameData(), nodeInfo.frameLen * nodeInfo.channels *
        GetSizeFromFormat(nodeInfo.format));
    hpaeSinkOutputNode->DisConnect(hpaeMixerNode);
    EXPECT_EQ(hpaeSinkOutputNode->GetPreOutNum(), 0);
    EXPECT_EQ(hpaeMixerNode.use_count(), 1);
    hpaeMixerNode->DisConnect(hpaeSinkInputNode0);
    EXPECT_EQ(hpaeSinkInputNode0.use_count(), 1);
    EXPECT_EQ(hpaeMixerNode->GetPreOutNum(), 1);
    hpaeMixerNode->DisConnect(hpaeSinkInputNode1);
    EXPECT_EQ(hpaeSinkInputNode1.use_count(), 1);
    EXPECT_EQ(hpaeMixerNode->GetPreOutNum(), 0);
}

HWTEST_F(HpaeMixerNodeTest, testMixerConnectWithInfo, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    std::shared_ptr<HpaeMixerNode> hpaeMixerNode = std::make_shared<HpaeMixerNode>(nodeInfo);
    nodeInfo.channels = MONO;
    std::shared_ptr<HpaeSourceInputCluster> cluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    EXPECT_EQ(cluster->fmtConverterNodeMap_.size() == 0, true);
    hpaeMixerNode->ConnectWithInfo(cluster, hpaeMixerNode->GetNodeInfo());
    EXPECT_EQ(cluster->fmtConverterNodeMap_.size() == 1, true);
    hpaeMixerNode->DisConnectWithInfo(cluster, hpaeMixerNode->GetNodeInfo());
    EXPECT_EQ(cluster->fmtConverterNodeMap_.size() == 1, true); // not delete convertnode, equal 1
}
} // namespace