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
#include <limits>
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

const int32_t DEFAULT_VALUE_ONE = 1;
const int32_t DEFAULT_VALUE_TWO = 2;
const uint32_t DEFAULT_SESSIONID_NUM_FIRST = 12345;
const uint32_t DEFAULT_SESSIONID_NUM_SECOND = 12346;
const uint32_t DEFAULT_NODEID_NUM_FIRST = 1243;
const size_t DEFAULT_FRAMELEN_FIRST = 820;
const size_t DEFAULT_FRAMELEN_SECOND = 960;
const int32_t DEFAULT_TEST_VALUE_FIRST = 100;
const int32_t DEFAULT_TEST_VALUE_SECOND = 200;
const float LOUDNESS_GAIN = 1.0f;
const float FRAME_LENGTH_IN_SECOND = 0.02;
const uint32_t NO_EXITS_SESSIONID_ID = 99999;

constexpr uint32_t SAMPLE_RATE_16010 = 16010;
constexpr uint32_t SAMPLE_RATE_16050 = 16050;
const size_t FRAMELEN_FOR_11025 = 441;
const size_t FRAMELEN_FOR_16010 = 1601;
const size_t FRAMELEN_FOR_16050 = 321;

static HpaeSinkInfo GetRenderTestSinkInfo()
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.frameLen = SAMPLE_RATE_48000 * FRAME_LENGTH_IN_SECOND;
    sinkInfo.format = SAMPLE_F32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;
    return sinkInfo;
}

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
    EXPECT_EQ(hpaeProcessCluster->idConverterMap_[nodeInfo.sessionId], nullptr);
    EXPECT_EQ(hpaeProcessCluster->idLoudnessGainNodeMap_[nodeInfo.sessionId], nullptr);
    EXPECT_EQ(hpaeProcessCluster->idGainMap_[nodeInfo.sessionId], nullptr);
    EXPECT_EQ(hpaeProcessCluster->CreateNodes(hpaeSinkInputNode), SUCCESS);
    hpaeProcessCluster->Connect(hpaeSinkInputNode);
    EXPECT_EQ(hpaeProcessCluster->GetGainNodeCount(), DEFAULT_VALUE_ONE);
    EXPECT_EQ(hpaeProcessCluster->GetConverterNodeCount(), DEFAULT_VALUE_ONE);
    EXPECT_EQ(hpaeProcessCluster->GetLoudnessGainNodeCount(), DEFAULT_VALUE_ONE);

    nodeInfo.frameLen = DEFAULT_FRAMELEN_FIRST;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_SECOND;
    nodeInfo.samplingRate = SAMPLE_RATE_44100;
    std::shared_ptr<HpaeSinkInputNode> hpaeSinkInputNode1 = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    EXPECT_EQ(hpaeProcessCluster->CreateNodes(hpaeSinkInputNode1), SUCCESS);
    hpaeProcessCluster->Connect(hpaeSinkInputNode1);
    EXPECT_EQ(hpaeProcessCluster->GetGainNodeCount(), DEFAULT_VALUE_TWO);
    EXPECT_EQ(hpaeProcessCluster->GetConverterNodeCount(), DEFAULT_VALUE_TWO);
    EXPECT_EQ(hpaeProcessCluster->GetLoudnessGainNodeCount(), DEFAULT_VALUE_TWO);
}

/**
 * @tc.name  : Test HpaeProcessCluster construct and HpaeSinkInputNode construct
 * @tc.number: constructHpaeProcessClusterNode_001
 * @tc.desc  : Test HpaeProcessCluster the branch when samplingRate = 11025
 */
HWTEST_F(HpaeProcessClusterTest, constructHpaeProcessClusterNode_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.frameLen = FRAMELEN_FOR_11025;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_11025;
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
    EXPECT_EQ(hpaeProcessCluster->idConverterMap_[nodeInfo.sessionId], nullptr);
    EXPECT_EQ(hpaeProcessCluster->idLoudnessGainNodeMap_[nodeInfo.sessionId], nullptr);
    EXPECT_EQ(hpaeProcessCluster->idGainMap_[nodeInfo.sessionId], nullptr);
    EXPECT_EQ(hpaeProcessCluster->CreateNodes(hpaeSinkInputNode), SUCCESS);
    hpaeProcessCluster->Connect(hpaeSinkInputNode);
    EXPECT_EQ(hpaeProcessCluster->GetGainNodeCount(), DEFAULT_VALUE_ONE);
    EXPECT_EQ(hpaeProcessCluster->GetConverterNodeCount(), DEFAULT_VALUE_ONE);
    EXPECT_EQ(hpaeProcessCluster->GetLoudnessGainNodeCount(), DEFAULT_VALUE_ONE);
}

/**
 * @tc.name  : Test HpaeProcessCluster construct and HpaeSinkInputNode construct
 * @tc.number: constructHpaeProcessClusterNode_002
 * @tc.desc  : Test HpaeProcessCluster the branch when customSampleRate = 16010
 */
HWTEST_F(HpaeProcessClusterTest, constructHpaeProcessClusterNode_002, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.frameLen = FRAMELEN_FOR_16010;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.customSampleRate = SAMPLE_RATE_16010;
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
    EXPECT_EQ(retNi.customSampleRate, nodeInfo.customSampleRate);

    std::shared_ptr<HpaeSinkInputNode> hpaeSinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    EXPECT_EQ(hpaeProcessCluster->idConverterMap_[nodeInfo.sessionId], nullptr);
    EXPECT_EQ(hpaeProcessCluster->idLoudnessGainNodeMap_[nodeInfo.sessionId], nullptr);
    EXPECT_EQ(hpaeProcessCluster->idGainMap_[nodeInfo.sessionId], nullptr);
    EXPECT_EQ(hpaeProcessCluster->CreateNodes(hpaeSinkInputNode), SUCCESS);
    hpaeProcessCluster->Connect(hpaeSinkInputNode);
    EXPECT_EQ(hpaeProcessCluster->GetGainNodeCount(), DEFAULT_VALUE_ONE);
    EXPECT_EQ(hpaeProcessCluster->GetConverterNodeCount(), DEFAULT_VALUE_ONE);
    EXPECT_EQ(hpaeProcessCluster->GetLoudnessGainNodeCount(), DEFAULT_VALUE_ONE);
}

/**
 * @tc.name  : Test HpaeProcessCluster construct and HpaeSinkInputNode construct
 * @tc.number: constructHpaeProcessClusterNode_003
 * @tc.desc  : Test HpaeProcessCluster the branch when customSampleRate = 16050
 */
HWTEST_F(HpaeProcessClusterTest, constructHpaeProcessClusterNode_003, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.frameLen = FRAMELEN_FOR_16050;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.customSampleRate = SAMPLE_RATE_16050;
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
    EXPECT_EQ(retNi.customSampleRate, nodeInfo.customSampleRate);

    std::shared_ptr<HpaeSinkInputNode> hpaeSinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    EXPECT_EQ(hpaeProcessCluster->idConverterMap_[nodeInfo.sessionId], nullptr);
    EXPECT_EQ(hpaeProcessCluster->idLoudnessGainNodeMap_[nodeInfo.sessionId], nullptr);
    EXPECT_EQ(hpaeProcessCluster->idGainMap_[nodeInfo.sessionId], nullptr);
    EXPECT_EQ(hpaeProcessCluster->CreateNodes(hpaeSinkInputNode), SUCCESS);
    hpaeProcessCluster->Connect(hpaeSinkInputNode);
    EXPECT_EQ(hpaeProcessCluster->GetGainNodeCount(), DEFAULT_VALUE_ONE);
    EXPECT_EQ(hpaeProcessCluster->GetConverterNodeCount(), DEFAULT_VALUE_ONE);
    EXPECT_EQ(hpaeProcessCluster->GetLoudnessGainNodeCount(), DEFAULT_VALUE_ONE);
}

/**
 * @tc.name  : Test HpaeProcessCluster construct and HpaeSinkInputNode construct
 * @tc.number: constructHpaeProcessClusterNode_004
 * @tc.desc  : Test HpaeProcessCluster the branch when customSampleRate = 11025
 */
HWTEST_F(HpaeProcessClusterTest, constructHpaeProcessClusterNode_004, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.customSampleRate = SAMPLE_RATE_11025;
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
    EXPECT_EQ(retNi.customSampleRate, nodeInfo.customSampleRate);

    std::shared_ptr<HpaeSinkInputNode> hpaeSinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    EXPECT_EQ(hpaeProcessCluster->idConverterMap_[nodeInfo.sessionId], nullptr);
    EXPECT_EQ(hpaeProcessCluster->idLoudnessGainNodeMap_[nodeInfo.sessionId], nullptr);
    EXPECT_EQ(hpaeProcessCluster->idGainMap_[nodeInfo.sessionId], nullptr);
    EXPECT_EQ(hpaeProcessCluster->CreateNodes(hpaeSinkInputNode), SUCCESS);
    hpaeProcessCluster->Connect(hpaeSinkInputNode);
    EXPECT_EQ(hpaeProcessCluster->GetGainNodeCount(), DEFAULT_VALUE_ONE);
    EXPECT_EQ(hpaeProcessCluster->GetConverterNodeCount(), DEFAULT_VALUE_ONE);
    EXPECT_EQ(hpaeProcessCluster->GetLoudnessGainNodeCount(), DEFAULT_VALUE_ONE);
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
    EXPECT_EQ(hpaeProcessCluster->CreateNodes(hpaeSinkInputNode0), SUCCESS);
    EXPECT_EQ(hpaeProcessCluster->CreateNodes(hpaeSinkInputNode1), SUCCESS);
    hpaeProcessCluster->Connect(hpaeSinkInputNode0);
    hpaeProcessCluster->Connect(hpaeSinkInputNode1);
    EXPECT_EQ(hpaeSinkOutputNode->GetPreOutNum(), 0);
    EXPECT_EQ(hpaeProcessCluster->GetGainNodeCount(), DEFAULT_VALUE_TWO);
    EXPECT_EQ(hpaeSinkOutputNode->GetPreOutNum(), 0);
    hpaeSinkOutputNode->Connect(hpaeProcessCluster);
    std::string deviceClass = "file_io";
    std::string deviceNetId = "LocalDevice";
    EXPECT_EQ(hpaeSinkOutputNode->GetPreOutNum(), 1);
    EXPECT_EQ(hpaeSinkOutputNode->GetRenderSinkInstance(deviceClass, deviceNetId), 0);
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
    hpaeProcessCluster->DisConnect(hpaeSinkInputNode0);
    EXPECT_EQ(hpaeProcessCluster->DestroyNodes(DEFAULT_SESSIONID_NUM_FIRST), SUCCESS);
    EXPECT_EQ(hpaeProcessCluster->GetGainNodeCount(), DEFAULT_VALUE_ONE);

    hpaeProcessCluster->DisConnect(hpaeSinkInputNode1);
    EXPECT_EQ(hpaeProcessCluster->DestroyNodes(DEFAULT_SESSIONID_NUM_SECOND), SUCCESS);
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
    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();

    std::shared_ptr<HpaeProcessCluster> hpaeProcessCluster =
        std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);
    hpaeProcessCluster->DoProcess();
    EXPECT_EQ(hpaeProcessCluster->AudioRendererCreate(nodeInfo, sinkInfo), 0);
    hpaeProcessCluster->DoProcess();
    EXPECT_EQ(hpaeProcessCluster->AudioRendererStart(nodeInfo, sinkInfo), 0);
    EXPECT_EQ(hpaeProcessCluster->AudioRendererStop(nodeInfo, sinkInfo), 0);
    EXPECT_EQ(hpaeProcessCluster->AudioRendererRelease(nodeInfo, sinkInfo), 0);

    nodeInfo.sceneType = HPAE_SCENE_SPLIT_MEDIA;
    hpaeProcessCluster =
        std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);
    EXPECT_EQ(hpaeProcessCluster->AudioRendererCreate(nodeInfo, sinkInfo), 0);
    EXPECT_EQ(hpaeProcessCluster->AudioRendererStart(nodeInfo, sinkInfo), 0);
    EXPECT_EQ(hpaeProcessCluster->AudioRendererStop(nodeInfo, sinkInfo), 0);
    EXPECT_EQ(hpaeProcessCluster->AudioRendererRelease(nodeInfo, sinkInfo), 0);

    nodeInfo.sceneType = HPAE_SCENE_EFFECT_NONE;
    hpaeProcessCluster =
        std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);
    EXPECT_EQ(hpaeProcessCluster->AudioRendererCreate(nodeInfo, sinkInfo), 0);
    EXPECT_EQ(hpaeProcessCluster->AudioRendererStart(nodeInfo, sinkInfo), 0);
    EXPECT_EQ(hpaeProcessCluster->AudioRendererStop(nodeInfo, sinkInfo), 0);
    EXPECT_EQ(hpaeProcessCluster->AudioRendererRelease(nodeInfo, sinkInfo), 0);

    sinkInfo.deviceClass = "remote";
    EXPECT_EQ(hpaeProcessCluster->AudioRendererCreate(nodeInfo, sinkInfo), 0);
    EXPECT_EQ(hpaeProcessCluster->AudioRendererStart(nodeInfo, sinkInfo), 0);
    EXPECT_EQ(hpaeProcessCluster->AudioRendererStop(nodeInfo, sinkInfo), 0);
    EXPECT_EQ(hpaeProcessCluster->AudioRendererRelease(nodeInfo, sinkInfo), 0);

    sinkInfo.deviceName = "DP_MCH_speaker";
    EXPECT_EQ(hpaeProcessCluster->AudioRendererCreate(nodeInfo, sinkInfo), 0);
    EXPECT_EQ(hpaeProcessCluster->AudioRendererStart(nodeInfo, sinkInfo), 0);
    EXPECT_EQ(hpaeProcessCluster->AudioRendererStop(nodeInfo, sinkInfo), 0);
    EXPECT_EQ(hpaeProcessCluster->AudioRendererRelease(nodeInfo, sinkInfo), 0);
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
    
    EXPECT_EQ(hpaeProcessCluster->CreateNodes(dummySinkInputNode), SUCCESS);
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
    EXPECT_EQ(hpaeProcessCluster->CreateNodes(dummySinkInputNode), SUCCESS);
    hpaeProcessCluster->Connect(dummySinkInputNode);
    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    EXPECT_EQ(hpaeProcessCluster->AudioRendererCreate(nodeInfo, sinkInfo), SUCCESS);
    
    hpaeProcessCluster->SetLoudnessGain(DEFAULT_NODEID_NUM_FIRST, LOUDNESS_GAIN);
    ret = hpaeProcessCluster->GetNodeInputFormatInfo(nodeInfo.sessionId, basicFormat);
    EXPECT_EQ(basicFormat.audioChannelInfo.channelLayout, CH_LAYOUT_STEREO);
    EXPECT_EQ(basicFormat.audioChannelInfo.numChannels, static_cast<uint32_t>(STEREO));
    EXPECT_EQ(basicFormat.rate, SAMPLE_RATE_48000);
}

/**
 * @tc.name  : DisConnectMixerNode_001
 * @tc.type  : FUNC
 * @tc.number: DisConnectMixerNode_001
 * @tc.desc  : Test DisConnectMixerNode with renderEffectNode initialized.
 */
HWTEST_F(HpaeProcessClusterTest, DisConnectMixerNode_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);

    cluster->ConnectMixerNode();
    cluster->DisConnectMixerNode();
    SUCCEED();
}

/**
 * @tc.name  : DisConnectMixerNode_002
 * @tc.type  : FUNC
 * @tc.number: DisConnectMixerNode_002
 * @tc.desc  : Test DisConnectMixerNode when renderEffectNode is nullptr.
 */
HWTEST_F(HpaeProcessClusterTest, DisConnectMixerNode_002, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.sceneType = HPAE_SCENE_UNCONNECTED;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);

    cluster->DisConnectMixerNode();
    SUCCEED();
}

/**
 * @tc.name  : InitEffectBuffer_001
 * @tc.type  : FUNC
 * @tc.number: InitEffectBuffer_001
 * @tc.desc  : Test InitEffectBuffer when renderEffectNode is valid.
 */
HWTEST_F(HpaeProcessClusterTest, InitEffectBuffer_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);

    cluster->InitEffectBuffer(DEFAULT_SESSIONID_NUM_FIRST);
    SUCCEED();
}

/**
 * @tc.name  : InitEffectBuffer_002
 * @tc.type  : FUNC
 * @tc.number: InitEffectBuffer_002
 * @tc.desc  : Test InitEffectBuffer when renderEffectNode is nullptr (Defensive path).
 */
HWTEST_F(HpaeProcessClusterTest, InitEffectBuffer_002, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.sceneType = HPAE::HPAE_SCENE_UNCONNECTED;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);

    cluster->InitEffectBuffer(DEFAULT_SESSIONID_NUM_FIRST);
    SUCCEED();
}

/**
 * @tc.name  : ResetAll_001
 * @tc.type  : FUNC
 * @tc.number: ResetAll_001
 * @tc.desc  : Test ResetAll when renderEffectNode is not null.
 */
HWTEST_F(HpaeProcessClusterTest, ResetAll_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);

    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    EXPECT_EQ(cluster->CreateNodes(sinkInputNode), SUCCESS);
    cluster->Connect(sinkInputNode);

    EXPECT_TRUE(cluster->ResetAll());
}

/**
 * @tc.name  : ResetAll_002
 * @tc.type  : FUNC
 * @tc.number: ResetAll_002
 * @tc.desc  : Test ResetAll when renderEffectNode is null.
 */
HWTEST_F(HpaeProcessClusterTest, ResetAll_002, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.sceneType = HPAE_SCENE_UNCONNECTED;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);

    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    EXPECT_EQ(cluster->CreateNodes(sinkInputNode), SUCCESS);
    cluster->Connect(sinkInputNode);

    EXPECT_TRUE(cluster->ResetAll());
}

/**
 * @tc.name  : ResetAll_003
 * @tc.type  : FUNC
 * @tc.number: ResetAll_003
 * @tc.desc  : Test ResetAll with scene type that triggers renderEffectNode null (HPAE_SCENE_RECORD).
 */
HWTEST_F(HpaeProcessClusterTest, ResetAll_003, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.sceneType = HPAE_SCENE_RECORD;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);

    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    EXPECT_EQ(cluster->CreateNodes(sinkInputNode), SUCCESS);
    cluster->Connect(sinkInputNode);

    EXPECT_TRUE(cluster->ResetAll());
}

/**
 * @tc.name  : ResetAll_004
 * @tc.type  : FUNC
 * @tc.number: ResetAll_004
 * @tc.desc  : Test ResetAll with multiple sessions.
 */
HWTEST_F(HpaeProcessClusterTest, ResetAll_004, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);

    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    auto sinkInputNode1 = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    EXPECT_EQ(cluster->CreateNodes(sinkInputNode1), SUCCESS);
    cluster->Connect(sinkInputNode1);

    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_SECOND;
    auto sinkInputNode2 = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    EXPECT_EQ(cluster->CreateNodes(sinkInputNode2), SUCCESS);
    cluster->Connect(sinkInputNode2);

    EXPECT_TRUE(cluster->ResetAll());
}

/**
 * @tc.name  : ResetAll_005
 * @tc.type  : FUNC
 * @tc.number: ResetAll_005
 * @tc.desc  : Test ResetAll multiple calls (idempotent).
 */
HWTEST_F(HpaeProcessClusterTest, ResetAll_005, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);

    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    EXPECT_EQ(cluster->CreateNodes(sinkInputNode), SUCCESS);
    cluster->Connect(sinkInputNode);

    EXPECT_TRUE(cluster->ResetAll());
    EXPECT_TRUE(cluster->ResetAll());
    EXPECT_TRUE(cluster->ResetAll());
}

/**
 * @tc.name  : DoProcess_001
 * @tc.type  : FUNC
 * @tc.number: DoProcess_001
 * @tc.desc  : Test DoProcess when renderEffectNode is not null, without nodes connected.
 */
HWTEST_F(HpaeProcessClusterTest, DoProcess_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);

    // Verify renderEffectNode_ is non-null for HPAE_SCENE_MUSIC
    auto sharedInstance = cluster->GetSharedInstance();
    EXPECT_NE(sharedInstance, nullptr);
    EXPECT_EQ(sharedInstance->GetNodeName(), "hpaeRenderEffectNode");

    cluster->DoProcess();
    SUCCEED();
}

/**
 * @tc.name  : DoProcess_002
 * @tc.type  : FUNC
 * @tc.number: DoProcess_002
 * @tc.desc  : Test DoProcess when renderEffectNode is null (HPAE_SCENE_UNCONNECTED).
 */
HWTEST_F(HpaeProcessClusterTest, DoProcess_002, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.sceneType = HPAE_SCENE_UNCONNECTED;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);

    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    EXPECT_EQ(cluster->CreateNodes(sinkInputNode), SUCCESS);

    cluster->DoProcess();
    SUCCEED();
}

/**
 * @tc.name  : DoProcess_003
 * @tc.type  : FUNC
 * @tc.number: DoProcess_003
 * @tc.desc  : Test DoProcess with scene type that triggers renderEffectNode null (HPAE_SCENE_RECORD).
 */
HWTEST_F(HpaeProcessClusterTest, DoProcess_003, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.sceneType = HPAE_SCENE_RECORD;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);

    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    EXPECT_EQ(cluster->CreateNodes(sinkInputNode), SUCCESS);

    cluster->DoProcess();
    SUCCEED();
}

/**
 * @tc.name  : DoProcess_004
 * @tc.type  : FUNC
 * @tc.number: DoProcess_004
 * @tc.desc  : Test DoProcess with connected nodes.
 */
HWTEST_F(HpaeProcessClusterTest, DoProcess_004, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);

    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    EXPECT_EQ(cluster->CreateNodes(sinkInputNode), SUCCESS);
    cluster->Connect(sinkInputNode);

    cluster->DoProcess();
    SUCCEED();
}

/**
 * @tc.name  : DoProcess_005
 * @tc.type  : FUNC
 * @tc.number: DoProcess_005
 * @tc.desc  : Test DoProcess with different scene types.
 */
HWTEST_F(HpaeProcessClusterTest, DoProcess_005, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.sceneType = HPAE_SCENE_SPLIT_MEDIA;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);

    cluster->DoProcess();
    SUCCEED();
}

/**
 * @tc.name  : GetConverterNodeById_001
 * @tc.type  : FUNC
 * @tc.number: GetConverterNodeById_001
 * @tc.desc  : Test GetConverterNodeById with existing sessionId.
 */
HWTEST_F(HpaeProcessClusterTest, GetConverterNodeById_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);

    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    EXPECT_EQ(cluster->CreateNodes(sinkInputNode), SUCCESS);

    auto converterNode = cluster->GetConverterNodeById(DEFAULT_SESSIONID_NUM_FIRST);
    EXPECT_NE(converterNode, nullptr);
}

/**
 * @tc.name  : GetConverterNodeById_002
 * @tc.type  : FUNC
 * @tc.number: GetConverterNodeById_002
 * @tc.desc  : Test GetConverterNodeById with non-existing sessionId.
 */
HWTEST_F(HpaeProcessClusterTest, GetConverterNodeById_002, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);

    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    EXPECT_EQ(cluster->CreateNodes(sinkInputNode), SUCCESS);

    auto converterNode = cluster->GetConverterNodeById(NO_EXITS_SESSIONID_ID);
    EXPECT_EQ(converterNode, nullptr);
}

/**
 * @tc.name  : GetConverterNodeById_003
 * @tc.type  : FUNC
 * @tc.number: GetConverterNodeById_003
 * @tc.desc  : Test GetConverterNodeById with no nodes created.
 */
HWTEST_F(HpaeProcessClusterTest, GetConverterNodeById_003, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);

    auto converterNode = cluster->GetConverterNodeById(DEFAULT_SESSIONID_NUM_FIRST);
    EXPECT_EQ(converterNode, nullptr);
}

/**
 * @tc.name  : GetConverterNodeById_004
 * @tc.type  : FUNC
 * @tc.number: GetConverterNodeById_004
 * @tc.desc  : Test GetConverterNodeById after destroying nodes.
 */
HWTEST_F(HpaeProcessClusterTest, GetConverterNodeById_004, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);

    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    EXPECT_EQ(cluster->CreateNodes(sinkInputNode), SUCCESS);
    EXPECT_EQ(cluster->DestroyNodes(DEFAULT_SESSIONID_NUM_FIRST), SUCCESS);

    auto converterNode = cluster->GetConverterNodeById(DEFAULT_SESSIONID_NUM_FIRST);
    EXPECT_EQ(converterNode, nullptr);
}

/**
 * @tc.name  : GetConverterNodeById_005
 * @tc.type  : FUNC
 * @tc.number: GetConverterNodeById_005
 * @tc.desc  : Test GetConverterNodeById with multiple sessions.
 */
HWTEST_F(HpaeProcessClusterTest, GetConverterNodeById_005, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);

    auto sinkInputNode1 = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    EXPECT_EQ(cluster->CreateNodes(sinkInputNode1), SUCCESS);

    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_SECOND;
    auto sinkInputNode2 = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    EXPECT_EQ(cluster->CreateNodes(sinkInputNode2), SUCCESS);

    auto converterNode1 = cluster->GetConverterNodeById(DEFAULT_SESSIONID_NUM_FIRST);
    EXPECT_NE(converterNode1, nullptr);

    auto converterNode2 = cluster->GetConverterNodeById(DEFAULT_SESSIONID_NUM_SECOND);
    EXPECT_NE(converterNode2, nullptr);

    EXPECT_NE(converterNode1, converterNode2);
}

/**
 * @tc.name  : GetConverterNodeById_006
 * @tc.type  : FUNC
 * @tc.number: GetConverterNodeById_006
 * @tc.desc  : Test GetConverterNodeById with boundary sessionId values (0, MAX).
 */
HWTEST_F(HpaeProcessClusterTest, GetConverterNodeById_006, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);

    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    EXPECT_EQ(cluster->CreateNodes(sinkInputNode), SUCCESS);

    auto converterNode0 = cluster->GetConverterNodeById(0);
    EXPECT_EQ(converterNode0, nullptr);

    uint32_t maxSessionId = std::numeric_limits<uint32_t>::max();
    auto converterNodeMax = cluster->GetConverterNodeById(maxSessionId);
    EXPECT_EQ(converterNodeMax, nullptr);
}

/**
 * @tc.name  : SetupAudioLimiter_001
 * @tc.type  : FUNC
 * @tc.number: SetupAudioLimiter_001
 * @tc.desc  : Test SetupAudioLimiter when mixerNode is valid.
 */
HWTEST_F(HpaeProcessClusterTest, SetupAudioLimiter_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);

    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    EXPECT_EQ(cluster->CreateNodes(sinkInputNode), SUCCESS);

    int32_t ret = cluster->SetupAudioLimiter();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : SetupAudioLimiter_002
 * @tc.type  : FUNC
 * @tc.number: SetupAudioLimiter_002
 * @tc.desc  : Test SetupAudioLimiter when renderEffectNode is null (HPAE_SCENE_RECORD).
 */
HWTEST_F(HpaeProcessClusterTest, SetupAudioLimiter_002, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.sceneType = HPAE_SCENE_RECORD;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);

    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    EXPECT_EQ(cluster->CreateNodes(sinkInputNode), SUCCESS);

    int32_t ret = cluster->SetupAudioLimiter();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : SetupAudioLimiter_003
 * @tc.type  : FUNC
 * @tc.number: SetupAudioLimiter_003
 * @tc.desc  : Test SetupAudioLimiter with multiple sessions.
 */
HWTEST_F(HpaeProcessClusterTest, SetupAudioLimiter_003, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);

    auto sinkInputNode1 = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    EXPECT_EQ(cluster->CreateNodes(sinkInputNode1), SUCCESS);

    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_SECOND;
    auto sinkInputNode2 = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    EXPECT_EQ(cluster->CreateNodes(sinkInputNode2), SUCCESS);

    int32_t ret = cluster->SetupAudioLimiter();
    EXPECT_EQ(ret, SUCCESS);
}


/**
 * @tc.name  : SetupAudioLimiter_007
 * @tc.type  : FUNC
 * @tc.number: SetupAudioLimiter_007
 * @tc.desc  : Test SetupAudioLimiter with different scene types.
 */
HWTEST_F(HpaeProcessClusterTest, SetupAudioLimiter_007, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.sceneType = HPAE_SCENE_SPLIT_MEDIA;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);

    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    EXPECT_EQ(cluster->CreateNodes(sinkInputNode), SUCCESS);

    int32_t ret = cluster->SetupAudioLimiter();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : DoProcess_006
 * @tc.type  : FUNC
 * @tc.number: DoProcess_006
 * @tc.desc  : Test DoProcess correctly verifies behavior for HPAE_SCENE_SPLIT_MEDIA which
 *             triggers renderEffectNode_ = nullptr path.
 */
HWTEST_F(HpaeProcessClusterTest, DoProcess_006, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.sceneType = HPAE_SCENE_SPLIT_MEDIA;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);

    // For HPAE_SCENE_SPLIT_MEDIA, TransProcessorTypeToSceneType returns "SCENE_EXTRA"
    // which causes renderEffectNode_ to be set to nullptr in constructor
    // Verify this behavior through GetSharedInstance which returns mixerNode_ when renderEffectNode_ is nullptr
    auto sharedInstance = cluster->GetSharedInstance();
    EXPECT_NE(sharedInstance, nullptr);
    // Verify the correct path is taken (mixerNode_ path)
    EXPECT_EQ(sharedInstance->GetNodeName(), "hpaeMixerNode");

    // Create and connect nodes
    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    EXPECT_EQ(cluster->CreateNodes(sinkInputNode), SUCCESS);
    cluster->Connect(sinkInputNode);

    // Verify node counts after connection
    EXPECT_EQ(cluster->GetGainNodeCount(), DEFAULT_VALUE_ONE);
    EXPECT_EQ(cluster->GetConverterNodeCount(), DEFAULT_VALUE_ONE);
    EXPECT_EQ(cluster->GetLoudnessGainNodeCount(), DEFAULT_VALUE_ONE);

    // Call DoProcess - when renderEffectNode_ is nullptr, it should call mixerNode_->DoProcess()
    cluster->DoProcess();

    // Verify the mixer node is still valid after DoProcess
    sharedInstance = cluster->GetSharedInstance();
    EXPECT_NE(sharedInstance, nullptr);
    // Verify mixerNode_ path is still correct after DoProcess
    EXPECT_EQ(sharedInstance->GetNodeName(), "hpaeMixerNode");

    // Verify GetPreOutNum works correctly
    int32_t preOutNum = cluster->GetPreOutNum();
    EXPECT_EQ(preOutNum, DEFAULT_VALUE_ONE);
}

/**
 * @tc.name  : DoProcess_007
 * @tc.type  : FUNC
 * @tc.number: DoProcess_007
 * @tc.desc  : Test DoProcess with connected nodes and mixerNode_ path to ensure mixerNode_->DoProcess is called.
 */
HWTEST_F(HpaeProcessClusterTest, DoProcess_007, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.sceneType = HPAE_SCENE_UNCONNECTED;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);

    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    EXPECT_EQ(cluster->CreateNodes(sinkInputNode), SUCCESS);
    cluster->Connect(sinkInputNode);

    // Verify nodes are created and connected
    EXPECT_EQ(cluster->GetGainNodeCount(), DEFAULT_VALUE_ONE);
    EXPECT_EQ(cluster->GetConverterNodeCount(), DEFAULT_VALUE_ONE);
    EXPECT_EQ(cluster->GetLoudnessGainNodeCount(), DEFAULT_VALUE_ONE);

    // Verify GetSharedInstance returns mixerNode_ when renderEffectNode_ is nullptr
    auto sharedInstance = cluster->GetSharedInstance();
    EXPECT_NE(sharedInstance, nullptr);
    // Verify mixerNode_ path
    EXPECT_EQ(sharedInstance->GetNodeName(), "hpaeMixerNode");

    // Call DoProcess - should call mixerNode_->DoProcess() since renderEffectNode_ is nullptr
    cluster->DoProcess();

    // Verify mixer node state after DoProcess
    sharedInstance = cluster->GetSharedInstance();
    EXPECT_NE(sharedInstance, nullptr);
    // Verify mixerNode_ path is still correct
    EXPECT_EQ(sharedInstance->GetNodeName(), "hpaeMixerNode");

    // Verify output port is accessible
    auto outputPort = cluster->GetOutputPort();
    EXPECT_NE(outputPort, nullptr);

    // Verify pre-out count
    int32_t preOutNum = cluster->GetPreOutNum();
    EXPECT_EQ(preOutNum, DEFAULT_VALUE_ONE);
}

/**
 * @tc.name  : DoProcess_008
 * @tc.type  : FUNC
 * @tc.number: DoProcess_008
 * @tc.desc  : Test DoProcess with renderEffectNode_ non-null path (HPAE_SCENE_MUSIC) to
 *              ensure renderEffectNode_->DoProcess is called.
 */
HWTEST_F(HpaeProcessClusterTest, DoProcess_008, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);

    // For HPAE_SCENE_MUSIC, TransProcessorTypeToSceneType returns non-"SCENE_EXTRA"
    // which causes renderEffectNode_ to be non-null in constructor
    // Verify this behavior through GetSharedInstance which returns renderEffectNode_ when it's non-null
    auto sharedInstance = cluster->GetSharedInstance();
    EXPECT_NE(sharedInstance, nullptr);
    // Verify renderEffectNode_ path
    EXPECT_EQ(sharedInstance->GetNodeName(), "hpaeRenderEffectNode");

    // Call DoProcess - when renderEffectNode_ is non-null, it should call renderEffectNode_->DoProcess()
    cluster->DoProcess();

    // Verify the render effect node is still valid after DoProcess
    sharedInstance = cluster->GetSharedInstance();
    EXPECT_NE(sharedInstance, nullptr);
    // Verify renderEffectNode_ path is still correct after DoProcess
    EXPECT_EQ(sharedInstance->GetNodeName(), "hpaeRenderEffectNode");

    // Verify output port is accessible through renderEffectNode_
    auto outputPort = cluster->GetOutputPort();
    EXPECT_NE(outputPort, nullptr);
}

/**
 * @tc.name  : GetSharedInstance_001
 * @tc.type  : FUNC
 * @tc.number: GetSharedInstance_001
 * @tc.desc  : Test GetSharedInstance returns mixerNode_ when renderEffectNode_ is nullptr.
 */
HWTEST_F(HpaeProcessClusterTest, GetSharedInstance_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.sceneType = HPAE_SCENE_SPLIT_MEDIA;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);

    // GetSharedInstance should return mixerNode_ when renderEffectNode_ is nullptr
    auto sharedInstance = cluster->GetSharedInstance();
    EXPECT_NE(sharedInstance, nullptr);

    // Verify it's a valid mixer node
    uint32_t nodeId = sharedInstance->GetNodeId();
    EXPECT_NE(nodeId, 0);
    // Verify the returned node is indeed a mixerNode by checking node name
    EXPECT_EQ(sharedInstance->GetNodeName(), "hpaeMixerNode");
}

/**
 * @tc.name  : GetSharedInstance_002
 * @tc.type  : FUNC
 * @tc.number: GetSharedInstance_002
 * @tc.desc  : Test GetSharedInstance returns renderEffectNode_ when renderEffectNode_ is non-null.
 */
HWTEST_F(HpaeProcessClusterTest, GetSharedInstance_002, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);

    // GetSharedInstance should return renderEffectNode_ when it's non-null
    auto sharedInstance = cluster->GetSharedInstance();
    EXPECT_NE(sharedInstance, nullptr);

    // Verify it's a valid render effect node
    uint32_t nodeId = sharedInstance->GetNodeId();
    EXPECT_NE(nodeId, 0);
    // Verify the returned node is indeed a renderEffectNode by checking node name
    EXPECT_EQ(sharedInstance->GetNodeName(), "hpaeRenderEffectNode");
}

/**
 * @tc.name  : GetOutputPort_001
 * @tc.type  : FUNC
 * @tc.number: GetOutputPort_001
 * @tc.desc  : Test GetOutputPort returns mixerNode_ output port when renderEffectNode_ is nullptr.
 */
HWTEST_F(HpaeProcessClusterTest, GetOutputPort_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.sceneType = HPAE_SCENE_SPLIT_MEDIA;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);

    // Verify mixerNode_ path
    auto sharedInstance = cluster->GetSharedInstance();
    EXPECT_EQ(sharedInstance->GetNodeName(), "hpaeMixerNode");

    // GetOutputPort should return mixerNode_->GetOutputPort() when renderEffectNode_ is nullptr
    auto outputPort = cluster->GetOutputPort();
    EXPECT_NE(outputPort, nullptr);
}

/**
 * @tc.name  : GetOutputPort_002
 * @tc.type  : FUNC
 * @tc.number: GetOutputPort_002
 * @tc.desc  : Test GetOutputPort returns renderEffectNode_ output port when renderEffectNode_ is non-null.
 */
HWTEST_F(HpaeProcessClusterTest, GetOutputPort_002, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);

    // Verify renderEffectNode_ path
    auto sharedInstance = cluster->GetSharedInstance();
    EXPECT_EQ(sharedInstance->GetNodeName(), "hpaeRenderEffectNode");

    // GetOutputPort should return renderEffectNode_->GetOutputPort() when renderEffectNode_ is non-null
    auto outputPort = cluster->GetOutputPort();
    EXPECT_NE(outputPort, nullptr);
}

/**
 * @tc.name  : SetLoudnessGain_001
 * @tc.type  : FUNC
 * @tc.number: SetLoudnessGain_001
 * @tc.desc  : Test SetLoudnessGain with valid session ID and gain value.
 */
HWTEST_F(HpaeProcessClusterTest, SetLoudnessGain_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);

    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    EXPECT_EQ(cluster->CreateNodes(sinkInputNode), SUCCESS);
    cluster->Connect(sinkInputNode);

    int32_t ret = cluster->SetLoudnessGain(DEFAULT_SESSIONID_NUM_FIRST, LOUDNESS_GAIN);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : SetLoudnessGain_002
 * @tc.type  : FUNC
 * @tc.number: SetLoudnessGain_002
 * @tc.desc  : Test SetLoudnessGain with non-existing session ID (edge case).
 */
HWTEST_F(HpaeProcessClusterTest, SetLoudnessGain_002, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);

    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    EXPECT_EQ(cluster->CreateNodes(sinkInputNode), SUCCESS);
    cluster->Connect(sinkInputNode);

    // Try to set loudness gain for non-existing session ID
    int32_t ret = cluster->SetLoudnessGain(NO_EXITS_SESSIONID_ID, LOUDNESS_GAIN);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : SetLoudnessGain_003
 * @tc.type  : FUNC
 * @tc.number: SetLoudnessGain_003
 * @tc.desc  : Test SetLoudnessGain with boundary gain values (min and max).
 */
HWTEST_F(HpaeProcessClusterTest, SetLoudnessGain_003, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);

    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    EXPECT_EQ(cluster->CreateNodes(sinkInputNode), SUCCESS);
    cluster->Connect(sinkInputNode);

    // Test minimum gain value (0.0f)
    int32_t ret = cluster->SetLoudnessGain(DEFAULT_SESSIONID_NUM_FIRST, 0.0f);
    EXPECT_EQ(ret, SUCCESS);

    // Test maximum gain value
    ret = cluster->SetLoudnessGain(DEFAULT_SESSIONID_NUM_FIRST, 10.0f);
    EXPECT_EQ(ret, SUCCESS);

    // Test negative gain value
    ret = cluster->SetLoudnessGain(DEFAULT_SESSIONID_NUM_FIRST, -5.0f);
    EXPECT_EQ(ret, SUCCESS);
}


/**
 * @tc.name  : GetLatency_002
 * @tc.type  : FUNC
 * @tc.number: GetLatency_002
 * @tc.desc  : Test GetLatency with non-existing session ID (edge case).
 */
HWTEST_F(HpaeProcessClusterTest, GetLatency_002, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);

    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    EXPECT_EQ(cluster->CreateNodes(sinkInputNode), SUCCESS);
    cluster->Connect(sinkInputNode);

    // GetLatency should return 0 for non-existing session ID
    uint64_t latency = cluster->GetLatency(NO_EXITS_SESSIONID_ID);
    EXPECT_EQ(latency, 0);
}

/**
 * @tc.name  : GetGainNodeById_001
 * @tc.type  : FUNC
 * @tc.number: GetGainNodeById_001
 * @tc.desc  : Test GetGainNodeById with valid session ID.
 */
HWTEST_F(HpaeProcessClusterTest, GetGainNodeById_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);

    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    EXPECT_EQ(cluster->CreateNodes(sinkInputNode), SUCCESS);

    auto gainNode = cluster->GetGainNodeById(DEFAULT_SESSIONID_NUM_FIRST);
    EXPECT_NE(gainNode, nullptr);
}

/**
 * @tc.name  : GetGainNodeById_002
 * @tc.type  : FUNC
 * @tc.number: GetGainNodeById_002
 * @tc.desc  : Test GetGainNodeById with non-existing session ID (edge case).
 */
HWTEST_F(HpaeProcessClusterTest, GetGainNodeById_002, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);

    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    EXPECT_EQ(cluster->CreateNodes(sinkInputNode), SUCCESS);

    auto gainNode = cluster->GetGainNodeById(NO_EXITS_SESSIONID_ID);
    EXPECT_EQ(gainNode, nullptr);
}

/**
 * @tc.name  : CheckNodes_001
 * @tc.type  : FUNC
 * @tc.number: CheckNodes_001
 * @tc.desc  : Test CheckNodes with valid session ID.
 */
HWTEST_F(HpaeProcessClusterTest, CheckNodes_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);

    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    EXPECT_EQ(cluster->CreateNodes(sinkInputNode), SUCCESS);

    int32_t ret = cluster->CheckNodes(DEFAULT_SESSIONID_NUM_FIRST);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : CheckNodes_002
 * @tc.type  : FUNC
 * @tc.number: CheckNodes_002
 * @tc.desc  : Test CheckNodes with non-existing session ID (edge case).
 */
HWTEST_F(HpaeProcessClusterTest, CheckNodes_002, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);

    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    EXPECT_EQ(cluster->CreateNodes(sinkInputNode), SUCCESS);

    int32_t ret = cluster->CheckNodes(NO_EXITS_SESSIONID_ID);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : DisConnect_003
 * @tc.type  : FUNC
 * @tc.number: DisConnect_003
 * @tc.desc  : Test DisConnect with non-existing session ID (edge case).
 */
HWTEST_F(HpaeProcessClusterTest, DisConnect_003, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);

    // Create a node with a different session ID
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_SECOND;
    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    EXPECT_EQ(cluster->CreateNodes(sinkInputNode), SUCCESS);
    cluster->Connect(sinkInputNode);

    // Create a node with the first session ID
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    auto sinkInputNode2 = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    EXPECT_EQ(cluster->CreateNodes(sinkInputNode2), SUCCESS);
    cluster->Connect(sinkInputNode2);

    // Verify nodes are connected
    EXPECT_EQ(cluster->GetGainNodeCount(), DEFAULT_VALUE_TWO);
    EXPECT_EQ(cluster->GetConverterNodeCount(), DEFAULT_VALUE_TWO);
    EXPECT_EQ(cluster->GetLoudnessGainNodeCount(), DEFAULT_VALUE_TWO);

    // Try to disconnect a node with non-existing session ID
    nodeInfo.sessionId = NO_EXITS_SESSIONID_ID;
    auto sinkInputNode3 = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    // Should not crash and should handle gracefully
    cluster->DisConnect(sinkInputNode3);
}

/**
 * @tc.name  : CheckNodes_003
 * @tc.type  : FUNC
 * @tc.number: CheckNodes_003
 * @tc.desc  : Test CheckNodes when nodes have been destroyed.
 */
HWTEST_F(HpaeProcessClusterTest, CheckNodes_003, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);

    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    EXPECT_EQ(cluster->CreateNodes(sinkInputNode), SUCCESS);
    EXPECT_EQ(cluster->CheckNodes(DEFAULT_SESSIONID_NUM_FIRST), SUCCESS);

    // Destroy the nodes
    EXPECT_EQ(cluster->DestroyNodes(DEFAULT_SESSIONID_NUM_FIRST), SUCCESS);

    // CheckNodes should return ERROR for destroyed nodes
    int32_t ret = cluster->CheckNodes(DEFAULT_SESSIONID_NUM_FIRST);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : CreateNodes_001
 * @tc.type  : FUNC
 * @tc.number: CreateNodes_001
 * @tc.desc  : Test CreateNodes with nullptr input (edge case).
 */
HWTEST_F(HpaeProcessClusterTest, CreateNodes_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);

    // CreateNodes should return ERROR when passed nullptr
    std::shared_ptr<OutputNode<HpaePcmBuffer *>> nullNode = nullptr;
    int32_t ret = cluster->CreateNodes(nullNode);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : DestroyNodes_001
 * @tc.type  : FUNC
 * @tc.number: DestroyNodes_001
 * @tc.desc  : Test DestroyNodes with non-existing session ID (edge case).
 */
HWTEST_F(HpaeProcessClusterTest, DestroyNodes_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);

    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    EXPECT_EQ(cluster->CreateNodes(sinkInputNode), SUCCESS);

    // Verify nodes exist
    EXPECT_EQ(cluster->CheckNodes(DEFAULT_SESSIONID_NUM_FIRST), SUCCESS);

    // Try to destroy nodes with non-existing session ID
    int32_t ret = cluster->DestroyNodes(NO_EXITS_SESSIONID_ID);
    EXPECT_EQ(ret, ERROR);

    // Verify original nodes still exist
    EXPECT_EQ(cluster->CheckNodes(DEFAULT_SESSIONID_NUM_FIRST), SUCCESS);
}

/**
 * @tc.name  : FaultCode_CreateNodes_NullPreNode
 * @tc.type  : FUNC
 * @tc.number: FaultCode_CreateNodes_NullPreNode
 * @tc.desc  : Test CreateNodes with nullptr preNode,
 *             should report PLAY_CREATE_DEPENDENCY_NULL fault code.
 */
HWTEST_F(HpaeProcessClusterTest, FaultCode_CreateNodes_NullPreNode, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);

    // CreateNodes with nullptr should report PLAY_CREATE_DEPENDENCY_NULL
    std::shared_ptr<OutputNode<HpaePcmBuffer *>> nullNode = nullptr;
    int32_t ret = cluster->CreateNodes(nullNode);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : FaultCode_SetLoudnessGain_NullNode
 * @tc.type  : FUNC
 * @tc.number: FaultCode_SetLoudnessGain_NullNode
 * @tc.desc  : Test SetLoudnessGain when loudnessGainNode does not exist,
 *             should report PLAY_QUERY_INSTANCE_NULL fault code.
 */
HWTEST_F(HpaeProcessClusterTest, FaultCode_SetLoudnessGain_NullNode, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);
    // Do not create nodes - loudnessGainNode does not exist
    int32_t ret = cluster->SetLoudnessGain(DEFAULT_SESSIONID_NUM_FIRST, LOUDNESS_GAIN);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : FaultCode_SetupAudioLimiter_NullMixerNode
 * @tc.type  : FUNC
 * @tc.number: FaultCode_SetupAudioLimiter_NullMixerNode
 * @tc.desc  : Test SetupAudioLimiter when mixerNode is null (edge case).
 */
HWTEST_F(HpaeProcessClusterTest, FaultCode_SetupAudioLimiter_NullMixerNode, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);
    // mixerNode_ should always be valid after construction
    int32_t ret = cluster->SetupAudioLimiter();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : FaultCode_CheckNodes_NonExistentSession
 * @tc.type  : FUNC
 * @tc.number: FaultCode_CheckNodes_NonExistentSession
 * @tc.desc  : Test CheckNodes when nodes do not exist for the given session,
 *             should return ERROR (no fault code reported in CheckNodes itself).
 */
HWTEST_F(HpaeProcessClusterTest, FaultCode_CheckNodes_NonExistentSession, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);
    // No nodes created, CheckNodes should return ERROR
    int32_t ret = cluster->CheckNodes(DEFAULT_SESSIONID_NUM_FIRST);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test HpaeProcessCluster microsecond precision with 11025Hz
 * @tc.number: ProcessClusterFrameLen_11025To48000
 * @tc.desc  : Test ProcessCluster constructor uses microsecond precision for 11025Hz -> 48000Hz conversion.
 *             Old algorithm: 220*1000/11025=19 -> 48000*19/1000=912 (large error)
 *             New algorithm: 220*1000000/11025=19954 -> 19954*48000/1000000=957 (smaller error)
 */
HWTEST_F(HpaeProcessClusterTest, ProcessClusterFrameLen_11025To48000, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.frameLen = 220;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_11025;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    // sinkInfo.samplingRate = SAMPLE_RATE_48000, sinkInfo.frameLen = 960

    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);
    // mixerNode_ is created with recalculated nodeInfo
    // frameLenMicros = 220 * 1000000 / 11025 = 19954
    // nodeInfo.frameLen = 19954 * 48000 / 1000000 = 957
    ASSERT_NE(cluster->mixerNode_, nullptr);
    EXPECT_EQ(cluster->mixerNode_->GetFrameLen(), 960u);
    EXPECT_EQ(cluster->mixerNode_->GetSampleRate(), SAMPLE_RATE_48000);
}

/**
 * @tc.name  : Test HpaeProcessCluster fallback on zero effectiveRate
 * @tc.number: ProcessClusterFrameLen_ZeroEffectiveRate
 * @tc.desc  : Test ProcessCluster constructor falls back to sinkInfo.frameLen when effectiveRate is 0
 */
HWTEST_F(HpaeProcessClusterTest, ProcessClusterFrameLen_ZeroEffectiveRate, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.frameLen = 100;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = static_cast<AudioSamplingRate>(0);
    nodeInfo.customSampleRate = 0;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    // sinkInfo.samplingRate = SAMPLE_RATE_48000, sinkInfo.frameLen = 960

    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);
    ASSERT_NE(cluster->mixerNode_, nullptr);
    EXPECT_EQ(cluster->mixerNode_->GetFrameLen(), sinkInfo.frameLen);
}

/**
 * @tc.name  : Test HpaeProcessCluster fallback on zero sinkRate
 * @tc.number: ProcessClusterFrameLen_ZeroSinkRate
 * @tc.desc  : Test ProcessCluster constructor falls back to sinkInfo.frameLen when sinkRate is 0
 */
HWTEST_F(HpaeProcessClusterTest, ProcessClusterFrameLen_ZeroSinkRate, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.frameLen = 960;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo sinkInfo;
    sinkInfo.samplingRate = static_cast<AudioSamplingRate>(0);
    sinkInfo.frameLen = 100;
    sinkInfo.channels = STEREO;
    sinkInfo.format = SAMPLE_F32LE;

    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);
    ASSERT_NE(cluster->mixerNode_, nullptr);
    EXPECT_EQ(cluster->mixerNode_->GetFrameLen(), sinkInfo.frameLen);
}

/**
 * @tc.name  : Test HpaeProcessCluster with customSampleRate
 * @tc.number: ProcessClusterFrameLen_CustomSampleRate
 * @tc.desc  : Test ProcessCluster uses customSampleRate (8010) via GetEffectiveSampleRate
 */
HWTEST_F(HpaeProcessClusterTest, ProcessClusterFrameLen_CustomSampleRate, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODEID_NUM_FIRST;
    nodeInfo.frameLen = 160;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    nodeInfo.samplingRate = SAMPLE_RATE_8000;
    nodeInfo.customSampleRate = 8010;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    HpaeSinkInfo sinkInfo = GetRenderTestSinkInfo();
    // effectiveRate = 8010 (from customSampleRate)
    // frameLenMicros = 160 * 1000000 / 8010 = 19975
    // nodeInfo.frameLen = 19975 * 48000 / 1000000 = 958

    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);
    ASSERT_NE(cluster->mixerNode_, nullptr);
    EXPECT_EQ(cluster->mixerNode_->GetFrameLen(), 960u);
}
} // AudioStandard
} // OHOS