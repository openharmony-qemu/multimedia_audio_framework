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
#include <cstdio>
#include "hpae_sink_input_node.h"
#include "hpae_render_effect_node.h"
#include "hpae_sink_output_node.h"
#include "hpae_source_input_node.h"
#include <fstream>
#include <streambuf>
#include <string>
#include "test_case_common.h"
#include "audio_errors.h"
#include "audio_effect_chain_manager.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace HPAE;
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
static constexpr uint32_t TEST_ID = 1266;
static constexpr uint32_t TEST_FRAMELEN1 = 960;
static constexpr uint32_t NODEINFO_EFFECTSCENEVALID = 100;
std::vector<EffectChain> DEFAULT_EFFECT_CHAINS = {
    {"EFFECTCHAIN_SPK_MUSIC", {"apply1", "apply2", "apply3"}, ""},
    {"EFFECTCHAIN_BT_MUSIC", {}, ""}
};

EffectChainManagerParam DEFAULT_EFFECT_CHAIN_MANAGER_PARAM{
    3,
    "SCENE_DEFAULT",
    {},
    {{"SCENE_MOVIE_&_EFFECT_DEFAULT_&_DEVICE_TYPE_SPEAKER", "EFFECTCHAIN_SPK_MUSIC"},
        {"SCENE_MOVIE_&_EFFECT_DEFAULT_&_DEVICE_TYPE_BLUETOOTH_A2DP", "EFFECTCHAIN_BT_MUSIC"}},
    {{"effect1", "property1"}, {"effect4", "property5"}, {"effect1", "property4"}}
};

std::vector<std::shared_ptr<AudioEffectLibEntry>> DEFAULT_EFFECT_LIBRARY_LIST = {};
class HpaeRenderEffectNodeTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void HpaeRenderEffectNodeTest::SetUp()
{}

void HpaeRenderEffectNodeTest::TearDown()
{}

void RunHpaeRenderEffectNodeTest(OHOS::AudioStandard::AudioSamplingRate format_val,
                               OHOS::AudioStandard::AudioChannel sample_rate_val,
                               OHOS::AudioStandard::AudioSampleFormat channels_val)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = sample_rate_val;
    nodeInfo.channels = channels_val;
    nodeInfo.format = format_val;
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceClass = "remote_offload";
    std::shared_ptr<HpaeRenderEffectNode> hpaeRenderEffectNode = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    nodeInfo.effectInfo.effectScene = (AudioEffectScene)0xff;
    EXPECT_EQ(hpaeRenderEffectNode->AudioRendererCreate(nodeInfo), 0);
    EXPECT_EQ(hpaeRenderEffectNode->AudioOffloadRendererCreate(nodeInfo, sinkInfo), 0);
    EXPECT_NE(hpaeRenderEffectNode->ReleaseAudioEffectChain(nodeInfo), 0);
}

#define DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(format_val, sample_rate_val, channels_val, test_name) \
HWTEST_F(HpaeRenderEffectNodeTest, test_name, TestSize.Level2) \
{ \
    RunHpaeRenderEffectNodeTest(format_val, sample_rate_val, channels_val); \
}

DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S16LE, SAMPLE_RATE_48000, STEREO, testCreate_001)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S16LE, SAMPLE_RATE_44100, STEREO, testCreate_002)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_F32LE, SAMPLE_RATE_48000, STEREO, testCreate_003)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S32LE, SAMPLE_RATE_48000, STEREO, testCreate_004)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S24LE, SAMPLE_RATE_48000, STEREO, testCreate_005)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S16LE, SAMPLE_RATE_96000, STEREO, testCreate_006)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_F32LE, SAMPLE_RATE_96000, STEREO, testCreate_007)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S16LE, SAMPLE_RATE_48000, MONO, testCreate_008)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S16LE, SAMPLE_RATE_44100, MONO, testCreate_009)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_F32LE, SAMPLE_RATE_48000, MONO, testCreate_010)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S16LE, SAMPLE_RATE_8000, STEREO, testCreate_011)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S16LE, SAMPLE_RATE_16000, STEREO, testCreate_012)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S16LE, SAMPLE_RATE_32000, STEREO, testCreate_013)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S16LE, SAMPLE_RATE_22050, STEREO, testCreate_014)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S16LE, SAMPLE_RATE_24000, STEREO, testCreate_015)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_U8, SAMPLE_RATE_48000, STEREO, testCreate_016)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_U8, SAMPLE_RATE_44100, STEREO, testCreate_017)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_U8, SAMPLE_RATE_48000, MONO, testCreate_018)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S32LE, SAMPLE_RATE_44100, STEREO, testCreate_019)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S24LE, SAMPLE_RATE_44100, STEREO, testCreate_020)

DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S16LE, SAMPLE_RATE_48000, CHANNEL_3, testCreate_021)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S16LE, SAMPLE_RATE_48000, CHANNEL_4, testCreate_022)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S16LE, SAMPLE_RATE_48000, CHANNEL_5, testCreate_023)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S16LE, SAMPLE_RATE_48000, CHANNEL_6, testCreate_024)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S16LE, SAMPLE_RATE_48000, CHANNEL_7, testCreate_025)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S16LE, SAMPLE_RATE_48000, CHANNEL_8, testCreate_026)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_F32LE, SAMPLE_RATE_48000, CHANNEL_6, testCreate_027)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_F32LE, SAMPLE_RATE_48000, CHANNEL_8, testCreate_028)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S32LE, SAMPLE_RATE_48000, CHANNEL_6, testCreate_029)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S32LE, SAMPLE_RATE_48000, CHANNEL_8, testCreate_030)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S16LE, SAMPLE_RATE_44100, CHANNEL_6, testCreate_031)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S16LE, SAMPLE_RATE_44100, CHANNEL_8, testCreate_032)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S16LE, SAMPLE_RATE_96000, CHANNEL_6, testCreate_033)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S16LE, SAMPLE_RATE_96000, CHANNEL_8, testCreate_034)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_F32LE, SAMPLE_RATE_96000, CHANNEL_6, testCreate_035)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_F32LE, SAMPLE_RATE_96000, CHANNEL_8, testCreate_036)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S16LE, SAMPLE_RATE_48000, CHANNEL_9, testCreate_037)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S16LE, SAMPLE_RATE_48000, CHANNEL_10, testCreate_038)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S16LE, SAMPLE_RATE_48000, CHANNEL_16, testCreate_039)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_F32LE, SAMPLE_RATE_48000, CHANNEL_16, testCreate_040)

DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S16LE, SAMPLE_RATE_176400, STEREO, testCreate_041)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S16LE, SAMPLE_RATE_192000, STEREO, testCreate_042)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S16LE, SAMPLE_RATE_384000, STEREO, testCreate_043)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_F32LE, SAMPLE_RATE_176400, STEREO, testCreate_044)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_F32LE, SAMPLE_RATE_192000, STEREO, testCreate_045)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_F32LE, SAMPLE_RATE_384000, STEREO, testCreate_046)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S32LE, SAMPLE_RATE_176400, STEREO, testCreate_047)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S32LE, SAMPLE_RATE_192000, STEREO, testCreate_048)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S32LE, SAMPLE_RATE_384000, STEREO, testCreate_049)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S24LE, SAMPLE_RATE_176400, STEREO, testCreate_050)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S24LE, SAMPLE_RATE_192000, STEREO, testCreate_051)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S24LE, SAMPLE_RATE_384000, STEREO, testCreate_052)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S16LE, SAMPLE_RATE_176400, MONO, testCreate_053)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S16LE, SAMPLE_RATE_192000, MONO, testCreate_054)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S16LE, SAMPLE_RATE_384000, MONO, testCreate_055)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_F32LE, SAMPLE_RATE_176400, MONO, testCreate_056)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_F32LE, SAMPLE_RATE_192000, MONO, testCreate_057)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_F32LE, SAMPLE_RATE_384000, MONO, testCreate_058)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S16LE, SAMPLE_RATE_88200, STEREO, testCreate_059)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S16LE, SAMPLE_RATE_64000, STEREO, testCreate_060)

DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_U8, SAMPLE_RATE_8000, STEREO, testCreate_061)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_U8, SAMPLE_RATE_16000, STEREO, testCreate_062)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_U8, SAMPLE_RATE_32000, STEREO, testCreate_063)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_U8, SAMPLE_RATE_44100, MONO, testCreate_064)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_U8, SAMPLE_RATE_48000, MONO, testCreate_065)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S24LE, SAMPLE_RATE_8000, STEREO, testCreate_066)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S24LE, SAMPLE_RATE_16000, STEREO, testCreate_067)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S24LE, SAMPLE_RATE_32000, STEREO, testCreate_068)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S24LE, SAMPLE_RATE_44100, STEREO, testCreate_069)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S32LE, SAMPLE_RATE_8000, STEREO, testCreate_070)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S32LE, SAMPLE_RATE_16000, STEREO, testCreate_071)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S32LE, SAMPLE_RATE_32000, STEREO, testCreate_072)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S32LE, SAMPLE_RATE_44100, STEREO, testCreate_073)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_F32LE, SAMPLE_RATE_8000, STEREO, testCreate_074)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_F32LE, SAMPLE_RATE_16000, STEREO, testCreate_075)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_F32LE, SAMPLE_RATE_32000, STEREO, testCreate_076)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_F32LE, SAMPLE_RATE_44100, STEREO, testCreate_077)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S16LE, SAMPLE_RATE_11025, STEREO, testCreate_078)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S16LE, SAMPLE_RATE_12000, STEREO, testCreate_079)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S16LE, SAMPLE_RATE_22050, MONO, testCreate_080)

DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S16LE, SAMPLE_RATE_48000, CHANNEL_UNKNOW, testCreate_081)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_F32LE, SAMPLE_RATE_48000, CHANNEL_UNKNOW, testCreate_082)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S16LE, SAMPLE_RATE_44100, CHANNEL_UNKNOW, testCreate_083)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(INVALID_WIDTH, SAMPLE_RATE_48000, STEREO, testCreate_084)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(INVALID_WIDTH, SAMPLE_RATE_44100, STEREO, testCreate_085)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(INVALID_WIDTH, SAMPLE_RATE_48000, MONO, testCreate_086)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_U8, SAMPLE_RATE_48000, CHANNEL_6, testCreate_087)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_U8, SAMPLE_RATE_44100, CHANNEL_6, testCreate_088)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S24LE, SAMPLE_RATE_48000, CHANNEL_6, testCreate_089)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S24LE, SAMPLE_RATE_44100, CHANNEL_6, testCreate_090)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S32LE, SAMPLE_RATE_48000, CHANNEL_6, testCreate_091)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S32LE, SAMPLE_RATE_44100, CHANNEL_6, testCreate_092)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_F32LE, SAMPLE_RATE_48000, CHANNEL_3, testCreate_093)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_F32LE, SAMPLE_RATE_44100, CHANNEL_3, testCreate_094)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S16LE, SAMPLE_RATE_48000, CHANNEL_12, testCreate_095)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S16LE, SAMPLE_RATE_44100, CHANNEL_12, testCreate_096)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_F32LE, SAMPLE_RATE_48000, CHANNEL_12, testCreate_097)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_F32LE, SAMPLE_RATE_44100, CHANNEL_12, testCreate_098)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S16LE, SAMPLE_RATE_48000, CHANNEL_16, testCreate_099)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_F32LE, SAMPLE_RATE_48000, CHANNEL_16, testCreate_100)

HWTEST_F(HpaeRenderEffectNodeTest, testCreate_002, TestSize.Level0)
{
    constexpr uint32_t idOffset = 5;
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceClass = "remote_offload";
    std::shared_ptr<HpaeRenderEffectNode> hpaeRenderEffectNode = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    EXPECT_EQ(hpaeRenderEffectNode->AudioRendererCreate(nodeInfo), 0);
    EXPECT_EQ(hpaeRenderEffectNode->AudioOffloadRendererCreate(nodeInfo, sinkInfo), 0);
    HpaeNodeInfo nodeInfo2 = nodeInfo;
    nodeInfo2.nodeId += idOffset;
    EXPECT_NE(hpaeRenderEffectNode->ReleaseAudioEffectChain(nodeInfo2), 0);
    EXPECT_EQ(hpaeRenderEffectNode->ReleaseAudioEffectChain(nodeInfo), 0);
}

HWTEST_F(HpaeRenderEffectNodeTest, testSignalProcess_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    HpaeNodeInfo dstNodeInfo;
    std::shared_ptr<HpaeRenderEffectNode> hpaeRenderEffectNode = std::make_shared<HpaeRenderEffectNode>(nodeInfo);

    std::vector<HpaePcmBuffer *> inputs;
    EXPECT_EQ(hpaeRenderEffectNode->SignalProcess(inputs), nullptr);
    PcmBufferInfo pcmBufferInfo(MONO, TEST_FRAMELEN1, SAMPLE_RATE_44100);
    HpaePcmBuffer hpaePcmBuffer(pcmBufferInfo);
    inputs.emplace_back(&hpaePcmBuffer);
    EXPECT_NE(hpaeRenderEffectNode->SignalProcess(inputs), nullptr);
}

HWTEST_F(HpaeRenderEffectNodeTest, testSignalProcess_002, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    std::shared_ptr<HpaeRenderEffectNode> hpaeRenderEffectNode = std::make_shared<HpaeRenderEffectNode>(nodeInfo);

    std::vector<HpaePcmBuffer *> inputs;
    PcmBufferInfo pcmBufferInfo(MONO, TEST_FRAMELEN1, SAMPLE_RATE_44100);
    HpaePcmBuffer hpaePcmBuffer(pcmBufferInfo);
    hpaePcmBuffer.SetBufferSilence(true);
    inputs.emplace_back(&hpaePcmBuffer);
    EXPECT_NE(hpaeRenderEffectNode->SignalProcess(inputs), nullptr);
    hpaeRenderEffectNode->ReconfigOutputBuffer();
}

HWTEST_F(HpaeRenderEffectNodeTest, testSignalProcess_003, TestSize.Level0)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    std::string sceneStr = "SCENE_MUSIC";
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(sceneStr);
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_16000;
    nodeInfo.channels = CHANNEL_6;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.effectInfo.effectScene = SCENE_MUSIC;
    std::shared_ptr<HpaeRenderEffectNode> hpaeRenderEffectNode = std::make_shared<HpaeRenderEffectNode>(nodeInfo);

    std::vector<HpaePcmBuffer *> inputs;
    PcmBufferInfo pcmBufferInfo(MONO, TEST_FRAMELEN1, SAMPLE_RATE_44100);
    HpaePcmBuffer hpaePcmBuffer(pcmBufferInfo);
    hpaePcmBuffer.SetBufferSilence(true);
    inputs.emplace_back(&hpaePcmBuffer);
    EXPECT_NE(hpaeRenderEffectNode->SignalProcess(inputs), nullptr);
    hpaeRenderEffectNode->ReconfigOutputBuffer();
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

HWTEST_F(HpaeRenderEffectNodeTest, testModifyAudioEffectChainInfo_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    std::shared_ptr<HpaeRenderEffectNode> hpaeRenderEffectNode = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    EXPECT_NE(hpaeRenderEffectNode, nullptr);
    ModifyAudioEffectChainInfoReason testReason = static_cast<ModifyAudioEffectChainInfoReason>(2);
    hpaeRenderEffectNode->ModifyAudioEffectChainInfo(nodeInfo, testReason);
    nodeInfo.effectInfo.effectScene = (AudioEffectScene)0xff;
    hpaeRenderEffectNode->ModifyAudioEffectChainInfo(nodeInfo, testReason);
}

HWTEST_F(HpaeRenderEffectNodeTest, testUpdateAudioEffectChainInfo_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    std::shared_ptr<HpaeRenderEffectNode> hpaeRenderEffectNode = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    EXPECT_NE(hpaeRenderEffectNode, nullptr);
    hpaeRenderEffectNode->UpdateAudioEffectChainInfo(nodeInfo);
    nodeInfo.effectInfo.effectScene = (AudioEffectScene)0xff;
    hpaeRenderEffectNode->UpdateAudioEffectChainInfo(nodeInfo);
}

HWTEST_F(HpaeRenderEffectNodeTest, testHpaeRenderEffectNode_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    //1, default nodeInfo.sceneType
    nodeInfo.sceneType = HPAE_SCENE_DEFAULT;
    std::shared_ptr<HpaeRenderEffectNode> hpaeRenderEffectNode_0 = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    EXPECT_NE(hpaeRenderEffectNode_0, nullptr);
}

HWTEST_F(HpaeRenderEffectNodeTest, testHpaeRenderEffectNode_002, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    //2, non default nodeInfo.sceneType
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.effectInfo.effectScene = SCENE_COLLABORATIVE;
    std::shared_ptr<HpaeRenderEffectNode> hpaeRenderEffectNode_1 = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    EXPECT_NE(hpaeRenderEffectNode_1, nullptr);
}

HWTEST_F(HpaeRenderEffectNodeTest, testHpaeRenderEffectNode_003, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    //3, else branch 00
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.effectInfo.effectScene = SCENE_SPEECH;
    std::shared_ptr<HpaeRenderEffectNode> hpaeRenderEffectNode_2 = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    EXPECT_NE(hpaeRenderEffectNode_2, nullptr);
}

HWTEST_F(HpaeRenderEffectNodeTest, testHpaeRenderEffectNode_004, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    //4, else branch 01 NODEINFO_EFFECTSCENEVALID
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.effectInfo.effectScene = static_cast<AudioEffectScene>(NODEINFO_EFFECTSCENEVALID);
    std::shared_ptr<HpaeRenderEffectNode> hpaeRenderEffectNode_3 = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    EXPECT_NE(hpaeRenderEffectNode_3, nullptr);
}

HWTEST_F(HpaeRenderEffectNodeTest, testInitEffectBuffer_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    std::shared_ptr<HpaeRenderEffectNode> hpaeRenderEffectNode = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    string sessionId = std::to_string(TEST_ID);
    hpaeRenderEffectNode->InitEffectBuffer(TEST_ID);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->InitEffectBuffer(sessionId), SUCCESS);
}

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS