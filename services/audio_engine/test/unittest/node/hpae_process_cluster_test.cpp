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
#include "hpae_sink_output_node.h"
#include "audio_effect.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace HPAE;
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {

const int32_t DEFAULT_VALUE_TWO = 2;
const uint32_t DEFAULT_SESSIONID_NUM_FIRST = 12345;
const uint32_t DEFAULT_SESSIONID_NUM_SECOND = 12346;
const uint32_t DEFAULT_NODEID_NUM_FIRST = 1243;
const size_t DEFAULT_FRAMELEN_FIRST = 820;
const size_t DEFAULT_FRAMELEN_SECOND = 960;
const int32_t DEFAULT_TEST_VALUE_FIRST = 100;
const int32_t DEFAULT_TEST_VALUE_SECOND = 200;
const float LOUDNESS_GAIN = 1.0f;


class HpaeProcessClusterTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void HpaeProcessClusterTest::SetUp()
{}

void HpaeProcessClusterTest::TearDown()
{}

HWTEST_F(HpaeProcessClusterTest, constructHpaeProcessClusterNode, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo dummySinkInfo;

    std::shared_ptr<HpaeProcessCluster> hpaeProcessCluster =
        std::make_shared<HpaeProcessCluster>(nodeInfo, dummySinkInfo);
    EXPECT_EQ(hpaeProcessCluster->GetSampleRate(), nodeInfo.samplingRate);
    EXPECT_EQ(hpaeProcessCluster->GetFrameLen(), nodeInfo.frameLen);
    EXPECT_EQ(hpaeProcessCluster->GetChannelCount(), nodeInfo.channels);
    EXPECT_EQ(hpaeProcessCluster->GetBitWidth(), nodeInfo.format);
    HpaeNodeInfo &retNi = hpaeProcessCluster->GetNodeInfo();
    EXPECT_EQ(retNi.samplingRate, nodeInfo.samplingRate);
    EXPECT_EQ(retNi.frameLen, nodeInfo.frameLen);
    EXPECT_EQ(retNi.channels, nodeInfo.channels);
    EXPECT_EQ(retNi.format, nodeInfo.format);

    std::shared_ptr<HpaeSinkInputNode> hpaeSinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    hpaeProcessCluster->Connect(hpaeSinkInputNode);
    EXPECT_EQ(hpaeSinkInputNode.use_count(), static_cast<long>(DEFAULT_VALUE_TWO));
    EXPECT_EQ(hpaeProcessCluster->GetGainNodeCount(), 1);
    EXPECT_EQ(hpaeProcessCluster->GetConverterNodeCount(), 1);
    nodeInfo.frameLen = DEFAULT_FRAMELEN_FIRST;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_SECOND;
    nodeInfo.samplingRate = SAMPLE_RATE_44100;
    std::shared_ptr<HpaeSinkInputNode> hpaeSinkInputNode1 = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    hpaeProcessCluster->Connect(hpaeSinkInputNode1);
    EXPECT_EQ(hpaeSinkInputNode1.use_count(), static_cast<long>(DEFAULT_VALUE_TWO));
    EXPECT_EQ(hpaeProcessCluster->GetGainNodeCount(), (DEFAULT_VALUE_TWO));
    EXPECT_EQ(hpaeProcessCluster->GetConverterNodeCount(), 1 + 1);
}
static int32_t g_testValue1 = 0;
static int32_t g_testValue2 = 0;
static int32_t TestRendererRenderFrame(const char *data, uint64_t len)
{
    float curGain = 0.0f;
    float targetGain = 1.0f;
    uint64_t frameLen = len / (SAMPLE_F32LE * STEREO);
    float stepGain = (targetGain - curGain) / frameLen;
    for (int32_t i = 0; i < frameLen; i++) {
        EXPECT_EQ(*((float *)data + i * STEREO + 1), (g_testValue1 * (curGain + i * stepGain) +
            g_testValue2 * (curGain + i * stepGain)));
        EXPECT_EQ(*((float *)data + i * STEREO), (g_testValue1 * (curGain + i * stepGain) +
            g_testValue2 * (curGain + i * stepGain)));
    }
    return 0;
}
static void CreateHpaeInfo(HpaeNodeInfo &nodeInfo, HpaeSinkInfo &dummySinkInfo)
{
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    dummySinkInfo.channels = STEREO;
    dummySinkInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    dummySinkInfo.format = SAMPLE_F32LE;
    dummySinkInfo.samplingRate = SAMPLE_RATE_48000;
}

HWTEST_F(HpaeProcessClusterTest, testHpaeWriteDataProcessSessionTest, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    HpaeSinkInfo dummySinkInfo;
    CreateHpaeInfo(nodeInfo, dummySinkInfo);
    std::shared_ptr<HpaeSinkOutputNode> hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    std::shared_ptr<HpaeSinkInputNode> hpaeSinkInputNode0 = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_SECOND;
    std::shared_ptr<HpaeSinkInputNode> hpaeSinkInputNode1 = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    std::shared_ptr<HpaeProcessCluster> hpaeProcessCluster =
        std::make_shared<HpaeProcessCluster>(nodeInfo, dummySinkInfo);
    hpaeProcessCluster->Connect(hpaeSinkInputNode0);
    hpaeProcessCluster->Connect(hpaeSinkInputNode1);
    EXPECT_EQ(hpaeSinkOutputNode->GetPreOutNum(), 0);
    EXPECT_EQ(hpaeProcessCluster->GetGainNodeCount(), DEFAULT_VALUE_TWO);
    EXPECT_EQ(hpaeProcessCluster->GetConverterNodeCount(), DEFAULT_VALUE_TWO);
    EXPECT_EQ(hpaeSinkOutputNode->GetPreOutNum(), 0);
    hpaeSinkOutputNode->Connect(hpaeProcessCluster);
    std::string deviceClass = "file_io";
    std::string deviceNetId = "LocalDevice";
    EXPECT_EQ(hpaeSinkOutputNode->GetPreOutNum(), 1);
    EXPECT_EQ(hpaeSinkOutputNode->GetRenderSinkInstance(deviceClass, deviceNetId), 0);
    EXPECT_EQ(hpaeSinkInputNode0.use_count(), static_cast<long>(DEFAULT_VALUE_TWO));
    EXPECT_EQ(hpaeSinkInputNode1.use_count(), static_cast<long>(DEFAULT_VALUE_TWO));
    EXPECT_EQ(hpaeProcessCluster.use_count(), 1);
    g_testValue1 = DEFAULT_TEST_VALUE_FIRST;
    std::shared_ptr<WriteFixedValueCb> writeFixedValueCb0 =
        std::make_shared<WriteFixedValueCb>(SAMPLE_F32LE, g_testValue1);
    hpaeSinkInputNode0->RegisterWriteCallback(writeFixedValueCb0);
    g_testValue2 = DEFAULT_TEST_VALUE_SECOND;
    std::shared_ptr<WriteFixedValueCb> writeFixedValueCb1 =
        std::make_shared<WriteFixedValueCb>(SAMPLE_F32LE, g_testValue2);
    hpaeSinkInputNode1->RegisterWriteCallback(writeFixedValueCb1);
    hpaeSinkOutputNode->DoProcess();
    TestRendererRenderFrame(hpaeSinkOutputNode->GetRenderFrameData(),
        nodeInfo.frameLen * nodeInfo.channels * GetSizeFromFormat(nodeInfo.format));
    hpaeSinkOutputNode->DisConnect(hpaeProcessCluster);
    EXPECT_EQ(hpaeSinkOutputNode->GetPreOutNum(), 0);
    EXPECT_EQ(hpaeProcessCluster.use_count(), 1);
    hpaeProcessCluster->DisConnect(hpaeSinkInputNode0);
    EXPECT_EQ(hpaeSinkInputNode0.use_count(), 1);
    EXPECT_EQ(hpaeProcessCluster->GetGainNodeCount(), 1);
    hpaeProcessCluster->DisConnect(hpaeSinkInputNode1);
    EXPECT_EQ(hpaeSinkInputNode1.use_count(), 1);
    EXPECT_EQ(hpaeProcessCluster->GetGainNodeCount(), 0);
}

HWTEST_F(HpaeProcessClusterTest, testEffectNode_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    HpaeSinkInfo dummySinkInfo;

    std::shared_ptr<HpaeProcessCluster> hpaeProcessCluster =
        std::make_shared<HpaeProcessCluster>(nodeInfo, dummySinkInfo);
    hpaeProcessCluster->DoProcess();
    EXPECT_EQ(hpaeProcessCluster->AudioRendererCreate(nodeInfo), 0);
    hpaeProcessCluster->DoProcess();
    EXPECT_EQ(hpaeProcessCluster->AudioRendererStart(nodeInfo), 0);
    EXPECT_EQ(hpaeProcessCluster->AudioRendererStop(nodeInfo), 0);
    EXPECT_EQ(hpaeProcessCluster->AudioRendererRelease(nodeInfo), 0);

    nodeInfo.sceneType = HPAE_SCENE_SPLIT_MEDIA;
    hpaeProcessCluster =
        std::make_shared<HpaeProcessCluster>(nodeInfo, dummySinkInfo);
    EXPECT_EQ(hpaeProcessCluster->AudioRendererCreate(nodeInfo), 0);
    EXPECT_EQ(hpaeProcessCluster->AudioRendererStart(nodeInfo), 0);
    EXPECT_EQ(hpaeProcessCluster->AudioRendererStop(nodeInfo), 0);
    EXPECT_EQ(hpaeProcessCluster->AudioRendererRelease(nodeInfo), 0);
}

HWTEST_F(HpaeProcessClusterTest, testGetNodeInputFormatInfo, TestSize.Level0)
{
    // test processCluster without effectnode and loundess algorithm handle
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_44100;
    nodeInfo.channels = CHANNEL_6;
    nodeInfo.channelLayout = CH_LAYOUT_5POINT1;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.sceneType = HPAE_SCENE_EFFECT_NONE;

    HpaeSinkInfo dummySinkInfo;
    dummySinkInfo.samplingRate = SAMPLE_RATE_96000;
    dummySinkInfo.channels = STEREO;
    dummySinkInfo.channelLayout = CH_LAYOUT_STEREO;

    auto dummySinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    std::shared_ptr<HpaeProcessCluster> hpaeProcessCluster =
        std::make_shared<HpaeProcessCluster>(nodeInfo, dummySinkInfo);
    
    hpaeProcessCluster->Connect(dummySinkInputNode);
    
    AudioBasicFormat basicFormat;
    hpaeProcessCluster->SetLoudnessGain(DEFAULT_NODEID_NUM_FIRST, 0.0f);
    int32_t ret = hpaeProcessCluster->GetNodeInputFormatInfo(nodeInfo.sessionId, basicFormat);

    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(basicFormat.audioChannelInfo.channelLayout, CH_LAYOUT_STEREO);
    EXPECT_EQ(basicFormat.audioChannelInfo.numChannels, static_cast<uint32_t>(STEREO));
    EXPECT_EQ(basicFormat.rate, SAMPLE_RATE_96000);

    // test processCluster with effectnode and loundess algorithm handle
    hpaeProcessCluster = nullptr;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    hpaeProcessCluster = std::make_shared<HpaeProcessCluster>(nodeInfo, dummySinkInfo);
    hpaeProcessCluster->Connect(dummySinkInputNode);
    EXPECT_EQ(hpaeProcessCluster->AudioRendererCreate(nodeInfo), SUCCESS);
    
    hpaeProcessCluster->SetLoudnessGain(DEFAULT_NODEID_NUM_FIRST, LOUDNESS_GAIN);
    ret = hpaeProcessCluster->GetNodeInputFormatInfo(nodeInfo.sessionId, basicFormat);
    EXPECT_EQ(basicFormat.audioChannelInfo.channelLayout, CH_LAYOUT_STEREO);
    EXPECT_EQ(basicFormat.audioChannelInfo.numChannels, static_cast<uint32_t>(STEREO));
    EXPECT_EQ(basicFormat.rate, SAMPLE_RATE_48000);
}
} // AudioStandard
} // OHOS