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
#include "hpae_process_cluster.h"
#include "test_case_common.h"
#include "audio_errors.h"
#include "hpae_sink_input_node.h"
#include "hpae_output_cluster.h"
using namespace testing::ext;
using namespace testing;
namespace OHOS {
namespace AudioStandard {
namespace HPAE {
constexpr uint32_t NODE_ID = 1243;
constexpr uint32_t SESSION_ID_1 = 12345;
constexpr uint32_t SESSION_ID_2 = 12346;
constexpr uint32_t FRAME_LEN = 960;
constexpr uint32_t FRAME_LEN_2 = 820;
constexpr uint32_t NUM_TWO = 2;
constexpr int32_t TEST_VALUE_1 = 300;
constexpr int32_t TEST_VALUE_2 = 400;

static std::string g_deviceClass = "file_io";
static std::string g_deviceNetId = "LocalDevice";


static int32_t TestRendererRenderFrame(const char *data, uint64_t len)
{
    float curGain = 0.0f;
    float targetGain = 1.0f;
    uint64_t frameLen = len / (SAMPLE_F32LE * STEREO);
    float stepGain = (targetGain - curGain) / frameLen;
    
    const float *tempData = reinterpret_cast<const float *>(data);
    for (int32_t i = 0; i < frameLen; i++) {
        const float left = tempData[NUM_TWO * i];
        const float right = tempData[NUM_TWO * i + 1];
        const float expectedValue = TEST_VALUE_1 * (curGain + i * stepGain) + TEST_VALUE_2 * (curGain + i * stepGain);
        EXPECT_EQ(left, expectedValue);
        EXPECT_EQ(right, expectedValue);
    }
    return 0;
}

static void InitHpaeWriteDataOutSessionTest(HpaeNodeInfo &nodeInfo, HpaeSinkInfo &dummySinkInfo)
{
    nodeInfo.nodeId = NODE_ID;
    nodeInfo.frameLen = FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    dummySinkInfo.frameLen = FRAME_LEN;
    dummySinkInfo.samplingRate = SAMPLE_RATE_48000;
    dummySinkInfo.channels = STEREO;
    dummySinkInfo.format = SAMPLE_F32LE;
    dummySinkInfo.deviceClass = g_deviceClass;
    dummySinkInfo.deviceNetId = g_deviceNetId;
}

class HpaeOutputClusterTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void HpaeOutputClusterTest::SetUp()
{}

void HpaeOutputClusterTest::TearDown()
{}

HWTEST_F(HpaeOutputClusterTest, constructHpaeOutputClusterNode, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = NODE_ID;
    nodeInfo.frameLen = FRAME_LEN;
    nodeInfo.sessionId = SESSION_ID_1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    int32_t syncId = 123;

    std::shared_ptr<HpaeOutputCluster> hpaeoutputCluster = std::make_shared<HpaeOutputCluster>(nodeInfo);
    EXPECT_EQ(hpaeoutputCluster->GetSampleRate(), nodeInfo.samplingRate);
    EXPECT_EQ(hpaeoutputCluster->GetFrameLen(), nodeInfo.frameLen);
    EXPECT_EQ(hpaeoutputCluster->GetChannelCount(), nodeInfo.channels);
    EXPECT_EQ(hpaeoutputCluster->GetBitWidth(), nodeInfo.format);
 
    std::shared_ptr<HpaeSinkInputNode> hpaeSinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    hpaeoutputCluster->Connect(hpaeSinkInputNode);
    EXPECT_EQ(hpaeSinkInputNode.use_count(), NUM_TWO);
    EXPECT_EQ(hpaeoutputCluster->GetConverterNodeCount(), 1);
    nodeInfo.frameLen = FRAME_LEN_2;
    nodeInfo.sessionId = SESSION_ID_2;
    nodeInfo.samplingRate = SAMPLE_RATE_44100;
    std::shared_ptr<HpaeSinkInputNode> hpaeSinkInputNode1 = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    hpaeoutputCluster->Connect(hpaeSinkInputNode1);
    EXPECT_EQ(hpaeSinkInputNode1.use_count(), NUM_TWO);
    EXPECT_EQ(hpaeoutputCluster->GetConverterNodeCount(), 1);
    EXPECT_EQ(hpaeoutputCluster->SetSyncId(syncId), SUCCESS);
}

HWTEST_F(HpaeOutputClusterTest, testHpaeWriteDataOutSessionTest, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    HpaeSinkInfo dummySinkInfo;
    InitHpaeWriteDataOutSessionTest(nodeInfo, dummySinkInfo);
    std::shared_ptr<HpaeOutputCluster> hpaeOutputCluster = std::make_shared<HpaeOutputCluster>(nodeInfo);
    nodeInfo.sessionId = SESSION_ID_1;
    nodeInfo.streamType = STREAM_MUSIC;
    if (hpaeOutputCluster->mixerNode_) {
        hpaeOutputCluster->mixerNode_->limiter_ = nullptr;
    }
    std::shared_ptr<HpaeSinkInputNode> musicSinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    nodeInfo.sessionId = SESSION_ID_2;
    nodeInfo.streamType = STREAM_RING;
    std::shared_ptr<HpaeSinkInputNode> ringSinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    std::shared_ptr<HpaeProcessCluster> muiscProcessCluster =
        std::make_shared<HpaeProcessCluster>(nodeInfo, dummySinkInfo);
    nodeInfo.sceneType = HPAE_SCENE_RING;
    std::shared_ptr<HpaeProcessCluster> ringProcessCluster =
        std::make_shared<HpaeProcessCluster>(nodeInfo, dummySinkInfo);
    muiscProcessCluster->Connect(musicSinkInputNode);
    ringProcessCluster->Connect(ringSinkInputNode);
    hpaeOutputCluster->Connect(muiscProcessCluster);
    hpaeOutputCluster->Connect(ringProcessCluster);

    EXPECT_EQ(ringProcessCluster->GetGainNodeCount(), 1);
    EXPECT_EQ(muiscProcessCluster->GetGainNodeCount(), 1);
    EXPECT_EQ(muiscProcessCluster->GetConverterNodeCount(), 1);
    EXPECT_EQ(ringProcessCluster->GetConverterNodeCount(), 1);
    EXPECT_EQ(hpaeOutputCluster->GetConverterNodeCount(), NUM_TWO);
    EXPECT_EQ(hpaeOutputCluster->GetPreOutNum(), NUM_TWO);

    EXPECT_EQ(hpaeOutputCluster->GetInstance(g_deviceClass, g_deviceNetId), 0);
    EXPECT_EQ(musicSinkInputNode.use_count(), NUM_TWO);
    EXPECT_EQ(ringSinkInputNode.use_count(), NUM_TWO);
    EXPECT_EQ(muiscProcessCluster.use_count(), 1);
    std::shared_ptr<WriteFixedValueCb> writeFixedValueCb0 =
        std::make_shared<WriteFixedValueCb>(SAMPLE_F32LE, TEST_VALUE_1);
    musicSinkInputNode->RegisterWriteCallback(writeFixedValueCb0);
    std::shared_ptr<WriteFixedValueCb> writeFixedValueCb1 =
        std::make_shared<WriteFixedValueCb>(SAMPLE_F32LE, TEST_VALUE_2);
    ringSinkInputNode->RegisterWriteCallback(writeFixedValueCb1);
    hpaeOutputCluster->DoProcess();
    TestRendererRenderFrame(hpaeOutputCluster->GetFrameData(),
        nodeInfo.frameLen * nodeInfo.channels * GetSizeFromFormat(nodeInfo.format));
    muiscProcessCluster->DisConnect(musicSinkInputNode);
    EXPECT_EQ(musicSinkInputNode.use_count(), 1);
    EXPECT_EQ(muiscProcessCluster->GetGainNodeCount(), 0);
    ringProcessCluster->DisConnect(ringSinkInputNode);
    EXPECT_EQ(ringSinkInputNode.use_count(), 1);
    hpaeOutputCluster->DisConnect(muiscProcessCluster);
    EXPECT_EQ(hpaeOutputCluster->GetPreOutNum(), 1);
    hpaeOutputCluster->DisConnect(ringProcessCluster);
    EXPECT_EQ(hpaeOutputCluster->GetPreOutNum(), 0);
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS