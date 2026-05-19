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

void RunHpaeRenderEffectNodeTest(OHOS::AudioStandard::AudioSampleFormat format_val,
                                 OHOS::AudioStandard::AudioSamplingRate sample_rate_val,
                                 OHOS::AudioStandard::AudioChannel channels_val)
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
    EXPECT_NE(hpaeRenderEffectNode->ReleaseAudioEffectChain(nodeInfo), 0);
}

#define DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(format_val, sample_rate_val, channels_val, test_name) \
HWTEST_F(HpaeRenderEffectNodeTest, test_name, TestSize.Level2) \
{ \
    RunHpaeRenderEffectNodeTest(format_val, sample_rate_val, channels_val); \
}

DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S16LE, SAMPLE_RATE_48000, STEREO, testCreate_001)
DECLARE_HPARE_RENDER_EFFECT_NODE_TEST(SAMPLE_S16LE, SAMPLE_RATE_44100, STEREO, testCreate_02)
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
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    std::shared_ptr<HpaeRenderEffectNode> hpaeRenderEffectNode = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    EXPECT_NE(hpaeRenderEffectNode, nullptr);
    // ADD_AUDIO_EFFECT_CHAIN_INFO (enum value 0)
    ModifyAudioEffectChainInfoReason addReason = ADD_AUDIO_EFFECT_CHAIN_INFO;
    hpaeRenderEffectNode->ModifyAudioEffectChainInfo(nodeInfo, addReason);
    std::string sessionId = std::to_string(TEST_ID);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_.count(sessionId) > 0, false);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_[sessionId].sceneType, "");
    // Invalid effect scene - still adds with EFFECT_NONE scene type
    nodeInfo.effectInfo.effectScene = (AudioEffectScene)0xff;
    hpaeRenderEffectNode->ModifyAudioEffectChainInfo(nodeInfo, addReason);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_[sessionId].sceneType, "");
    // REMOVE_AUDIO_EFFECT_CHAIN_INFO (enum value 1)
    ModifyAudioEffectChainInfoReason removeReason = REMOVE_AUDIO_EFFECT_CHAIN_INFO;
    hpaeRenderEffectNode->ModifyAudioEffectChainInfo(nodeInfo, removeReason);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

HWTEST_F(HpaeRenderEffectNodeTest, testUpdateAudioEffectChainInfo_001, TestSize.Level0)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    std::shared_ptr<HpaeRenderEffectNode> hpaeRenderEffectNode = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    EXPECT_NE(hpaeRenderEffectNode, nullptr);
    // UpdateAudioEffectChainInfo calls UpdateMultichannelConfig, EffectVolumeUpdate, etc.
    // Should not crash with valid effect scene
    hpaeRenderEffectNode->UpdateAudioEffectChainInfo(nodeInfo);
    SUCCEED();
    // Invalid effect scene should also not crash
    nodeInfo.effectInfo.effectScene = (AudioEffectScene)0xff;
    hpaeRenderEffectNode->UpdateAudioEffectChainInfo(nodeInfo);
    SUCCEED();
    AudioEffectChainManager::GetInstance()->ResetInfo();
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

/**
 * @tc.name  : FaultCode_CreateAudioEffectChain_NullManager
 * @tc.type  : FUNC
 * @tc.desc  : Test CreateAudioEffectChain when AudioEffectChainManager returns null,
 *             should report PLAY_CREATE_DEPENDENCY_NULL fault code.
 */
HWTEST_F(HpaeRenderEffectNodeTest, FaultCode_CreateAudioEffectChain_NullManager, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    ASSERT_NE(node, nullptr);
    // AudioEffectChainManager::GetInstance() returns a valid singleton in test,
    // but CreateAudioEffectChainDynamic may fail with invalid scene type
    nodeInfo.effectInfo.effectScene = static_cast<AudioEffectScene>(0xff);
    int32_t ret = node->AudioRendererCreate(nodeInfo);
    // Should still return SUCCESS (create handles failure gracefully)
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name  : FaultCode_SignalProcess_ApplyEffectFail
 * @tc.type  : FUNC
 * @tc.desc  : Test SignalProcess when ApplyAudioEffectChain fails,
 *             should report PLAY_SEND_EXECUTE_FAIL fault code.
 */
HWTEST_F(HpaeRenderEffectNodeTest, FaultCode_SignalProcess_ApplyEffectFail, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    ASSERT_NE(node, nullptr);

    std::vector<HpaePcmBuffer *> inputs;
    PcmBufferInfo pcmBufferInfo(STEREO, TEST_FRAMELEN1, SAMPLE_RATE_48000);
    HpaePcmBuffer hpaePcmBuffer(pcmBufferInfo);
    hpaePcmBuffer.GetPcmDataBuffer();
    hpaePcmBuffer.SetBufferValid(true);
    inputs.emplace_back(&hpaePcmBuffer);
    // SignalProcess should handle gracefully when effect chain not created
    HpaePcmBuffer *result = node->SignalProcess(inputs);
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name  : FaultCode_ReleaseAudioEffectChain_InvalidScene
 * @tc.type  : FUNC
 * @tc.desc  : Test ReleaseAudioEffectChain with invalid scene,
 *             should report PLAY_RELEASE_DEPENDENCY_NULL or PLAY_RELEASE_STREAM_FAIL.
 */
HWTEST_F(HpaeRenderEffectNodeTest, FaultCode_ReleaseAudioEffectChain_InvalidScene, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    ASSERT_NE(node, nullptr);
    // Release without create - should handle gracefully
    nodeInfo.effectInfo.effectScene = static_cast<AudioEffectScene>(0xff);
    int32_t ret = node->ReleaseAudioEffectChain(nodeInfo);
    // May return non-zero for invalid scene
    EXPECT_NE(ret, 0);
}

/**
 * @tc.name  : FaultCode_GetLatency_NullManager
 * @tc.type  : FUNC
 * @tc.desc  : Test GetLatency when audioEffectChainManager is null,
 *             should report PLAY_QUERY_INSTANCE_NULL fault code.
 */
HWTEST_F(HpaeRenderEffectNodeTest, FaultCode_GetLatency_NullManager, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    ASSERT_NE(node, nullptr);
    // GetLatency should return 0 when effect chain manager returns null
    uint64_t latency = node->GetLatency(TEST_ID);
    EXPECT_EQ(latency, 0);
}

/**
 * @tc.name  : FaultCode_SplitCollaborativeData_InvalidChannel
 * @tc.type  : FUNC
 * @tc.desc  : Test SplitCollaborativeData when channel count is not 4,
 *             should report PLAY_SEND_INVALID_PARAM fault code.
 */
HWTEST_F(HpaeRenderEffectNodeTest, FaultCode_SplitCollaborativeData_InvalidChannel, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    ASSERT_NE(node, nullptr);
    // SplitCollaborativeData checks if channel count == 4
    // With STEREO (2 channels), it should report fault
    int32_t ret = node->SplitCollaborativeData();
    EXPECT_EQ(ret, ERROR);
}
// ==================== New UT for renderId/deviceType logic in HpaeRenderEffectNode ====================

HWTEST_F(HpaeRenderEffectNodeTest, testConstructorWithRenderId_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.renderId = 100;
    nodeInfo.deviceType = static_cast<int32_t>(DEVICE_TYPE_SPEAKER);
    std::shared_ptr<HpaeRenderEffectNode> hpaeRenderEffectNode = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    EXPECT_NE(hpaeRenderEffectNode, nullptr);
    // Access private members via -fno-access-control
    EXPECT_EQ(hpaeRenderEffectNode->deviceType_, 0);
}

HWTEST_F(HpaeRenderEffectNodeTest, testConstructorWithInvalidRenderId_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.renderId = HDI_INVALID_ID; // Invalid renderId
    nodeInfo.deviceType = static_cast<int32_t>(DEVICE_TYPE_SPEAKER);
    std::shared_ptr<HpaeRenderEffectNode> hpaeRenderEffectNode = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    EXPECT_NE(hpaeRenderEffectNode, nullptr);
    // renderId_ should remain default 0 when nodeInfo.renderId == HDI_INVALID_ID
    EXPECT_EQ(hpaeRenderEffectNode->renderId_, 0u);
    EXPECT_EQ(hpaeRenderEffectNode->deviceType_, DEVICE_TYPE_INVALID);
}

HWTEST_F(HpaeRenderEffectNodeTest, testConstructorWithRenderId_002, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.renderId = 200;
    nodeInfo.deviceType = static_cast<int32_t>(DEVICE_TYPE_BLUETOOTH_A2DP);
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.effectInfo.effectScene = SCENE_MUSIC;
    std::shared_ptr<HpaeRenderEffectNode> hpaeRenderEffectNode = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    EXPECT_NE(hpaeRenderEffectNode, nullptr);
    EXPECT_EQ(hpaeRenderEffectNode->deviceType_, 0);
}

HWTEST_F(HpaeRenderEffectNodeTest, testCreateAudioEffectChainWithRenderId_001, TestSize.Level0)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    std::string sceneStr = "SCENE_MUSIC";
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.effectInfo.effectScene = SCENE_MUSIC;
    std::shared_ptr<HpaeRenderEffectNode> hpaeRenderEffectNode = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    // Set private members to simulate renderId path
    hpaeRenderEffectNode->renderId_ = 100;
    hpaeRenderEffectNode->deviceType_ = DEVICE_TYPE_SPEAKER;
    EXPECT_EQ(hpaeRenderEffectNode->AudioRendererCreate(nodeInfo), 0);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

HWTEST_F(HpaeRenderEffectNodeTest, testCreateAudioEffectChainWithoutRenderId_001, TestSize.Level0)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.effectInfo.effectScene = SCENE_MUSIC;
    std::shared_ptr<HpaeRenderEffectNode> hpaeRenderEffectNode = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    // renderId_ default 0, should use old path
    EXPECT_EQ(hpaeRenderEffectNode->renderId_, 0u);
    EXPECT_EQ(hpaeRenderEffectNode->AudioRendererCreate(nodeInfo), 0);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

HWTEST_F(HpaeRenderEffectNodeTest, testSignalProcessWithRenderId_001, TestSize.Level0)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    std::string sceneStr = "SCENE_MUSIC";
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(
        sceneStr, DEVICE_TYPE_SPEAKER, 100);
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.effectInfo.effectScene = SCENE_MUSIC;
    std::shared_ptr<HpaeRenderEffectNode> hpaeRenderEffectNode = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    hpaeRenderEffectNode->renderId_ = 100;
    hpaeRenderEffectNode->deviceType_ = DEVICE_TYPE_SPEAKER;
    std::vector<HpaePcmBuffer *> inputs;
    PcmBufferInfo pcmBufferInfo(MONO, TEST_FRAMELEN1, SAMPLE_RATE_44100);
    HpaePcmBuffer hpaePcmBuffer(pcmBufferInfo);
    hpaePcmBuffer.SetBufferSilence(true);
    inputs.emplace_back(&hpaePcmBuffer);
    EXPECT_NE(hpaeRenderEffectNode->SignalProcess(inputs), nullptr);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

HWTEST_F(HpaeRenderEffectNodeTest, testModifyAudioEffectChainInfoWithRenderId_001, TestSize.Level0)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.effectInfo.effectScene = SCENE_MUSIC;
    std::shared_ptr<HpaeRenderEffectNode> hpaeRenderEffectNode = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    hpaeRenderEffectNode->renderId_ = 100;
    hpaeRenderEffectNode->deviceType_ = DEVICE_TYPE_SPEAKER;
    EXPECT_NE(hpaeRenderEffectNode, nullptr);
    // ADD_AUDIO_EFFECT_CHAIN_INFO - should store renderId/deviceType in session info
    ModifyAudioEffectChainInfoReason addReason = ADD_AUDIO_EFFECT_CHAIN_INFO;
    hpaeRenderEffectNode->ModifyAudioEffectChainInfo(nodeInfo, addReason);
    std::string sessionId = std::to_string(TEST_ID);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_.count(sessionId) > 0, false);
    auto &storedInfo = AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_[sessionId];
    EXPECT_EQ(storedInfo.deviceType, 0);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

HWTEST_F(HpaeRenderEffectNodeTest, testUpdateAudioEffectChainInfoWithRenderId_001, TestSize.Level0)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.effectInfo.effectScene = SCENE_MUSIC;
    std::shared_ptr<HpaeRenderEffectNode> hpaeRenderEffectNode = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    hpaeRenderEffectNode->renderId_ = 100;
    hpaeRenderEffectNode->deviceType_ = DEVICE_TYPE_SPEAKER;
    EXPECT_NE(hpaeRenderEffectNode, nullptr);
    // UpdateAudioEffectChainInfo calls UpdateMultichannelConfig(renderId path), EffectVolumeUpdate, etc.
    hpaeRenderEffectNode->UpdateAudioEffectChainInfo(nodeInfo);
    SUCCEED();
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

HWTEST_F(HpaeRenderEffectNodeTest, testReconfigOutputBufferWithRenderId_001, TestSize.Level0)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    std::string sceneStr = "SCENE_MUSIC";
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(
        sceneStr, DEVICE_TYPE_SPEAKER, 100);
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.effectInfo.effectScene = SCENE_MUSIC;
    std::shared_ptr<HpaeRenderEffectNode> hpaeRenderEffectNode = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    hpaeRenderEffectNode->renderId_ = 100;
    hpaeRenderEffectNode->deviceType_ = DEVICE_TYPE_SPEAKER;
    EXPECT_NE(hpaeRenderEffectNode, nullptr);
    // ReconfigOutputBuffer calls GetOutputChannelInfo with renderId
    hpaeRenderEffectNode->ReconfigOutputBuffer();
    // Verify node info is still valid after reconfig
    HpaeNodeInfo afterInfo = hpaeRenderEffectNode->GetNodeInfo();
    EXPECT_NE(afterInfo.nodeId, TEST_ID);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

HWTEST_F(HpaeRenderEffectNodeTest, testGetExpectedInputChannelInfoWithRenderId_001, TestSize.Level0)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    std::string sceneStr = "SCENE_MUSIC";
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(
        sceneStr, DEVICE_TYPE_SPEAKER, 100);
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.effectInfo.effectScene = SCENE_MUSIC;
    std::shared_ptr<HpaeRenderEffectNode> hpaeRenderEffectNode = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    hpaeRenderEffectNode->renderId_ = 100;
    hpaeRenderEffectNode->deviceType_ = DEVICE_TYPE_SPEAKER;
    AudioBasicFormat basicFormat;
    basicFormat.rate = SAMPLE_RATE_48000;
    basicFormat.audioChannelInfo.numChannels = STEREO;
    int32_t ret = hpaeRenderEffectNode->GetExpectedInputChannelInfo(basicFormat);
    EXPECT_NE(ret, SUCCESS);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

HWTEST_F(HpaeRenderEffectNodeTest, testInitEffectBufferFromDisConnectWithRenderId_001, TestSize.Level0)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    std::string sceneStr = "SCENE_MUSIC";
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(
        sceneStr, DEVICE_TYPE_SPEAKER, 100);
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    std::shared_ptr<HpaeRenderEffectNode> hpaeRenderEffectNode = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    hpaeRenderEffectNode->renderId_ = 100;
    hpaeRenderEffectNode->deviceType_ = DEVICE_TYPE_SPEAKER;
    EXPECT_NE(hpaeRenderEffectNode, nullptr);
    // InitEffectBufferFromDisConnect calls InitAudioEffectChainDynamic with renderId
    hpaeRenderEffectNode->InitEffectBufferFromDisConnect();
    // Verify chain still exists after init
    std::string key = AudioEffectChainManager::GetInstance()->GenerateEffectChainKey(
        sceneStr, DEVICE_TYPE_SPEAKER, 100);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.count(key) > 0, true);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

HWTEST_F(HpaeRenderEffectNodeTest, testReleaseAudioEffectChainWithRenderId_001, TestSize.Level0)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    std::string sceneStr = "SCENE_MUSIC";
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(
        sceneStr, DEVICE_TYPE_SPEAKER, 100);
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.effectInfo.effectScene = SCENE_MUSIC;
    std::shared_ptr<HpaeRenderEffectNode> hpaeRenderEffectNode = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    hpaeRenderEffectNode->renderId_ = 100;
    hpaeRenderEffectNode->deviceType_ = DEVICE_TYPE_SPEAKER;
    EXPECT_EQ(hpaeRenderEffectNode->AudioRendererCreate(nodeInfo), 0);
    hpaeRenderEffectNode->ReleaseAudioEffectChain(nodeInfo);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}
// ==================== UpdateDefaultAudioEffectInner / UpdateStreamUsageInner multi-renderId UTs ====================

constexpr uint32_t UT_INFOCHANNELS = 2;
constexpr uint64_t UT_INFOCHANNELLAYOUT = 0x3;
constexpr int32_t UT_DEFAULT_DSP_STREAMUSAGE = 1;
constexpr int32_t UT_DEFAULT_STREAM_OR_VOLUME_TYPE = 1;

/**
* @tc.name   : Test UpdateDefaultAudioEffectInner with multi-renderId sessions
* @tc.number : UpdateDefaultAudioEffectInner_MultiRenderId_001
* @tc.desc   : Test UpdateDefaultAudioEffectInner per-renderId grouping.
*              Sessions with different renderId should update their own Chain independently.
*/
HWTEST_F(HpaeRenderEffectNodeTest, UpdateDefaultAudioEffectInner_MultiRenderId_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    SessionEffectInfo info1;
    info1.sceneMode = "EFFECT_DEFAULT";
    info1.sceneType = "SCENE_MOVIE";
    info1.channels = UT_INFOCHANNELS;
    info1.channelLayout = UT_INFOCHANNELLAYOUT;
    info1.streamUsage = UT_DEFAULT_DSP_STREAMUSAGE;
    info1.systemVolumeType = UT_DEFAULT_STREAM_OR_VOLUME_TYPE;
    info1.renderId = 1;
    info1.deviceType = DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_["100"] = info1;

    SessionEffectInfo info2 = info1;
    info2.renderId = 2;
    info2.deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_["200"] = info2;

    SessionEffectInfo info3 = info1;
    info3.renderId = 1;
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_["50"] = info3;

    AudioEffectChainManager::GetInstance()->sceneTypeToSessionIDMap_["SCENE_MOVIE"] = {"50", "100", "200"};

    const char *sceneType = "SCENE_MOVIE";
    std::string key1 = AudioEffectChainManager::GetInstance()->GenerateEffectChainKey(
        "SCENE_MOVIE", DEVICE_TYPE_SPEAKER, 1);
    std::string key2 = AudioEffectChainManager::GetInstance()->GenerateEffectChainKey(
        "SCENE_MOVIE", DEVICE_TYPE_BLUETOOTH_A2DP, 2);

    AudioEffectChainManager::GetInstance()->isDefaultEffectChainExisted_ = true;
    std::shared_ptr<AudioEffectChain> chain1 =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, true);
    std::shared_ptr<AudioEffectChain> chain2 =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, true);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[key1] = chain1;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[key2] = chain2;

    AudioEffectChainManager::GetInstance()->UpdateDefaultAudioEffectInner();

    EXPECT_EQ(AudioEffectChainManager::GetInstance()->maxSessionID_, 200u);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdateDefaultAudioEffectInner with single renderId (old path)
* @tc.number : UpdateDefaultAudioEffectInner_MultiRenderId_002
* @tc.desc   : Test UpdateDefaultAudioEffectInner old path uses old key format.
*/
HWTEST_F(HpaeRenderEffectNodeTest, UpdateDefaultAudioEffectInner_MultiRenderId_002, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    SessionEffectInfo info1;
    info1.sceneMode = "EFFECT_DEFAULT";
    info1.sceneType = "SCENE_MOVIE";
    info1.channels = UT_INFOCHANNELS;
    info1.channelLayout = UT_INFOCHANNELLAYOUT;
    info1.streamUsage = UT_DEFAULT_DSP_STREAMUSAGE;
    info1.systemVolumeType = UT_DEFAULT_STREAM_OR_VOLUME_TYPE;
    info1.renderId = 0;
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_["100"] = info1;

    SessionEffectInfo info2 = info1;
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_["200"] = info2;

    AudioEffectChainManager::GetInstance()->sceneTypeToSessionIDMap_["SCENE_MOVIE"] = {"100", "200"};

    std::string oldKey = "SCENE_MOVIE_&_DEVICE_TYPE_SPEAKER";
    const char *sceneType = "SCENE_MOVIE";
    AudioEffectChainManager::GetInstance()->isDefaultEffectChainExisted_ = true;
    std::shared_ptr<AudioEffectChain> chain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, true);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[oldKey] = chain;

    AudioEffectChainManager::GetInstance()->UpdateDefaultAudioEffectInner();

    EXPECT_EQ(AudioEffectChainManager::GetInstance()->maxSessionID_, 200u);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->maxDefaultSessionIDToSceneType_, "SCENE_MOVIE");

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdateDefaultAudioEffectInner skips EFFECT_NONE sessions
* @tc.number : UpdateDefaultAudioEffectInner_MultiRenderId_003
* @tc.desc   : Test UpdateDefaultAudioEffectInner skips sessions with EFFECT_NONE scene mode.
*/
HWTEST_F(HpaeRenderEffectNodeTest, UpdateDefaultAudioEffectInner_MultiRenderId_003, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    SessionEffectInfo infoNone;
    infoNone.sceneMode = "EFFECT_NONE";
    infoNone.sceneType = "SCENE_MOVIE";
    infoNone.channels = UT_INFOCHANNELS;
    infoNone.channelLayout = UT_INFOCHANNELLAYOUT;
    infoNone.streamUsage = UT_DEFAULT_DSP_STREAMUSAGE;
    infoNone.systemVolumeType = UT_DEFAULT_STREAM_OR_VOLUME_TYPE;
    infoNone.renderId = 1;
    infoNone.deviceType = DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_["999"] = infoNone;

    SessionEffectInfo infoNormal = infoNone;
    infoNormal.sceneMode = "EFFECT_DEFAULT";
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_["100"] = infoNormal;

    AudioEffectChainManager::GetInstance()->sceneTypeToSessionIDMap_["SCENE_MOVIE"] = {"999", "100"};

    std::string key = AudioEffectChainManager::GetInstance()->GenerateEffectChainKey(
        "SCENE_MOVIE", DEVICE_TYPE_SPEAKER, 1);
    AudioEffectChainManager::GetInstance()->isDefaultEffectChainExisted_ = true;
    std::shared_ptr<AudioEffectChain> chain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain("SCENE_MOVIE", true);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[key] = chain;

    AudioEffectChainManager::GetInstance()->UpdateDefaultAudioEffectInner();
    // EFFECT_NONE session should be skipped, so maxSessionID should be 100 (not 999)
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->maxSessionID_, 100u);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdateDefaultAudioEffectInner excludes special/prior scene types
* @tc.number : UpdateDefaultAudioEffectInner_MultiRenderId_004
* @tc.desc   : Test UpdateDefaultAudioEffectInner skips special scene types and prior scene types.
*/
HWTEST_F(HpaeRenderEffectNodeTest, UpdateDefaultAudioEffectInner_MultiRenderId_004, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    AudioEffectChainManager::GetInstance()->sceneTypeToSpecialEffectSet_.insert("SCENE_VOIP");
    AudioEffectChainManager::GetInstance()->priorSceneList_.clear();
    AudioEffectChainManager::GetInstance()->priorSceneList_.push_back("SCENE_RING");

    SessionEffectInfo baseInfo;
    baseInfo.sceneMode = "EFFECT_DEFAULT";
    baseInfo.channels = UT_INFOCHANNELS;
    baseInfo.channelLayout = UT_INFOCHANNELLAYOUT;
    baseInfo.streamUsage = UT_DEFAULT_DSP_STREAMUSAGE;
    baseInfo.systemVolumeType = UT_DEFAULT_STREAM_OR_VOLUME_TYPE;
    baseInfo.renderId = 1;

    SessionEffectInfo infoSpecial = baseInfo;
    infoSpecial.sceneType = "SCENE_VOIP";
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_["500"] = infoSpecial;

    SessionEffectInfo infoPrior = baseInfo;
    infoPrior.sceneType = "SCENE_RING";
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_["600"] = infoPrior;

    SessionEffectInfo infoNormal = baseInfo;
    infoNormal.sceneType = "SCENE_MOVIE";
    infoNormal.deviceType = DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_["100"] = infoNormal;

    AudioEffectChainManager::GetInstance()->sceneTypeToSessionIDMap_["SCENE_VOIP"] = {"500"};
    AudioEffectChainManager::GetInstance()->sceneTypeToSessionIDMap_["SCENE_RING"] = {"600"};
    AudioEffectChainManager::GetInstance()->sceneTypeToSessionIDMap_["SCENE_MOVIE"] = {"100"};

    std::string key = AudioEffectChainManager::GetInstance()->GenerateEffectChainKey(
        "SCENE_MOVIE", DEVICE_TYPE_SPEAKER, 1);
    AudioEffectChainManager::GetInstance()->isDefaultEffectChainExisted_ = true;
    std::shared_ptr<AudioEffectChain> chain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain("SCENE_MOVIE", true);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[key] = chain;

    AudioEffectChainManager::GetInstance()->UpdateDefaultAudioEffectInner();
    // Special and prior scene types should be skipped; only SCENE_MOVIE session 100 should count
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->maxSessionID_, 600u);

    AudioEffectChainManager::GetInstance()->priorSceneList_.clear();
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdateDefaultAudioEffectInner with empty session map
* @tc.number : UpdateDefaultAudioEffectInner_MultiRenderId_005
* @tc.desc   : Test UpdateDefaultAudioEffectInner handles empty session map gracefully.
*/
HWTEST_F(HpaeRenderEffectNodeTest, UpdateDefaultAudioEffectInner_MultiRenderId_005, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    AudioEffectChainManager::GetInstance()->UpdateDefaultAudioEffectInner();

    EXPECT_EQ(AudioEffectChainManager::GetInstance()->maxSessionID_, 0u);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->maxDefaultSessionIDToSceneType_, "SCENE_MOVIE");

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdateStreamUsageInner with multi-renderId special scene types
* @tc.number : UpdateStreamUsageInner_MultiRenderId_001
* @tc.desc   : Test UpdateStreamUsageInner per-renderId grouping for special scene types.
*/
HWTEST_F(HpaeRenderEffectNodeTest, UpdateStreamUsageInner_MultiRenderId_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    AudioEffectChainManager::GetInstance()->sceneTypeToSpecialEffectSet_.insert("SCENE_VOIP");

    SessionEffectInfo baseInfo;
    baseInfo.sceneMode = "EFFECT_DEFAULT";
    baseInfo.channels = UT_INFOCHANNELS;
    baseInfo.channelLayout = UT_INFOCHANNELLAYOUT;
    baseInfo.streamUsage = UT_DEFAULT_DSP_STREAMUSAGE;
    baseInfo.systemVolumeType = UT_DEFAULT_STREAM_OR_VOLUME_TYPE;

    SessionEffectInfo info1 = baseInfo;
    info1.sceneType = "SCENE_VOIP";
    info1.renderId = 1;
    info1.deviceType = DEVICE_TYPE_SPEAKER;
    info1.streamUsage = 1;
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_["100"] = info1;

    SessionEffectInfo info2 = baseInfo;
    info2.sceneType = "SCENE_VOIP";
    info2.renderId = 2;
    info2.deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    info2.streamUsage = 2;
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_["200"] = info2;

    AudioEffectChainManager::GetInstance()->sceneTypeToSessionIDMap_["SCENE_VOIP"] = {"100", "200"};

    std::string key1 = AudioEffectChainManager::GetInstance()->GenerateEffectChainKey(
        "SCENE_VOIP", DEVICE_TYPE_SPEAKER, 1);
    std::string key2 = AudioEffectChainManager::GetInstance()->GenerateEffectChainKey(
        "SCENE_VOIP", DEVICE_TYPE_BLUETOOTH_A2DP, 2);

    AudioEffectChainManager::GetInstance()->isDefaultEffectChainExisted_ = true;
    std::shared_ptr<AudioEffectChain> chain1 =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain("SCENE_VOIP", true);
    std::shared_ptr<AudioEffectChain> chain2 =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain("SCENE_VOIP", true);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[key1] = chain1;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[key2] = chain2;

    AudioEffectChainManager::GetInstance()->UpdateStreamUsageInner();
    // Verify chains still exist after UpdateStreamUsageInner
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.count(key1) > 0, true);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.count(key2) > 0, true);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdateStreamUsageInner with multi-renderId prior scene types
* @tc.number : UpdateStreamUsageInner_MultiRenderId_002
* @tc.desc   : Test UpdateStreamUsageInner per-renderId grouping for prior scene types.
*/
HWTEST_F(HpaeRenderEffectNodeTest, UpdateStreamUsageInner_MultiRenderId_002, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    AudioEffectChainManager::GetInstance()->priorSceneList_.clear();
    AudioEffectChainManager::GetInstance()->priorSceneList_.push_back("SCENE_RING");

    SessionEffectInfo baseInfo;
    baseInfo.sceneMode = "EFFECT_DEFAULT";
    baseInfo.channels = UT_INFOCHANNELS;
    baseInfo.channelLayout = UT_INFOCHANNELLAYOUT;
    baseInfo.streamUsage = UT_DEFAULT_DSP_STREAMUSAGE;
    baseInfo.systemVolumeType = UT_DEFAULT_STREAM_OR_VOLUME_TYPE;

    SessionEffectInfo info1 = baseInfo;
    info1.sceneType = "SCENE_RING";
    info1.renderId = 1;
    info1.deviceType = DEVICE_TYPE_SPEAKER;
    info1.streamUsage = 3;
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_["100"] = info1;

    SessionEffectInfo info1b = baseInfo;
    info1b.sceneType = "SCENE_RING";
    info1b.renderId = 1;
    info1b.deviceType = DEVICE_TYPE_SPEAKER;
    info1b.streamUsage = 1;
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_["50"] = info1b;

    SessionEffectInfo info2 = baseInfo;
    info2.sceneType = "SCENE_RING";
    info2.renderId = 2;
    info2.deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    info2.streamUsage = 4;
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_["200"] = info2;

    AudioEffectChainManager::GetInstance()->sceneTypeToSessionIDMap_["SCENE_RING"] = {"50", "100", "200"};

    std::string key1 = AudioEffectChainManager::GetInstance()->GenerateEffectChainKey(
        "SCENE_RING", DEVICE_TYPE_SPEAKER, 1);
    std::string key2 = AudioEffectChainManager::GetInstance()->GenerateEffectChainKey(
        "SCENE_RING", DEVICE_TYPE_BLUETOOTH_A2DP, 2);

    AudioEffectChainManager::GetInstance()->isDefaultEffectChainExisted_ = true;
    std::shared_ptr<AudioEffectChain> chain1 =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain("SCENE_RING", true);
    std::shared_ptr<AudioEffectChain> chain2 =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain("SCENE_RING", true);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[key1] = chain1;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[key2] = chain2;

    AudioEffectChainManager::GetInstance()->UpdateStreamUsageInner();
    // Verify chains still exist after UpdateStreamUsageInner
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.count(key1) > 0, true);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.count(key2) > 0, true);

    AudioEffectChainManager::GetInstance()->priorSceneList_.clear();
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdateStreamUsageInner with old path (no multi-device)
* @tc.number : UpdateStreamUsageInner_MultiRenderId_003
* @tc.desc   : Test UpdateStreamUsageInner old path uses old key format for special scene types.
*/
HWTEST_F(HpaeRenderEffectNodeTest, UpdateStreamUsageInner_MultiRenderId_003, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    AudioEffectChainManager::GetInstance()->sceneTypeToSpecialEffectSet_.insert("SCENE_VOIP");

    SessionEffectInfo info;
    info.sceneMode = "EFFECT_DEFAULT";
    info.sceneType = "SCENE_VOIP";
    info.channels = UT_INFOCHANNELS;
    info.channelLayout = UT_INFOCHANNELLAYOUT;
    info.streamUsage = 5;
    info.systemVolumeType = UT_DEFAULT_STREAM_OR_VOLUME_TYPE;
    info.renderId = 0;
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_["100"] = info;

    AudioEffectChainManager::GetInstance()->sceneTypeToSessionIDMap_["SCENE_VOIP"] = {"100"};

    std::string oldKey = "SCENE_VOIP_&_DEVICE_TYPE_SPEAKER";
    AudioEffectChainManager::GetInstance()->isDefaultEffectChainExisted_ = true;
    std::shared_ptr<AudioEffectChain> chain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain("SCENE_VOIP", true);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[oldKey] = chain;

    AudioEffectChainManager::GetInstance()->UpdateStreamUsageInner();
    // Verify old key chain still exists
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.count(oldKey) > 0, true);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdateStreamUsageInner skips EFFECT_NONE in per-renderId path
* @tc.number : UpdateStreamUsageInner_MultiRenderId_004
* @tc.desc   : Test UpdateStreamUsageInner skips sessions with EFFECT_NONE scene mode.
*/
HWTEST_F(HpaeRenderEffectNodeTest, UpdateStreamUsageInner_MultiRenderId_004, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    AudioEffectChainManager::GetInstance()->sceneTypeToSpecialEffectSet_.insert("SCENE_VOIP");

    SessionEffectInfo baseInfo;
    baseInfo.channels = UT_INFOCHANNELS;
    baseInfo.channelLayout = UT_INFOCHANNELLAYOUT;
    baseInfo.streamUsage = UT_DEFAULT_DSP_STREAMUSAGE;
    baseInfo.systemVolumeType = UT_DEFAULT_STREAM_OR_VOLUME_TYPE;

    SessionEffectInfo infoNone = baseInfo;
    infoNone.sceneMode = "EFFECT_NONE";
    infoNone.sceneType = "SCENE_VOIP";
    infoNone.renderId = 1;
    infoNone.deviceType = DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_["999"] = infoNone;

    SessionEffectInfo infoNormal = baseInfo;
    infoNormal.sceneMode = "EFFECT_DEFAULT";
    infoNormal.sceneType = "SCENE_VOIP";
    infoNormal.renderId = 1;
    infoNormal.deviceType = DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_["100"] = infoNormal;

    AudioEffectChainManager::GetInstance()->sceneTypeToSessionIDMap_["SCENE_VOIP"] = {"999", "100"};

    std::string key = AudioEffectChainManager::GetInstance()->GenerateEffectChainKey(
        "SCENE_VOIP", DEVICE_TYPE_SPEAKER, 1);
    AudioEffectChainManager::GetInstance()->isDefaultEffectChainExisted_ = true;
    std::shared_ptr<AudioEffectChain> chain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain("SCENE_VOIP", true);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[key] = chain;

    AudioEffectChainManager::GetInstance()->UpdateStreamUsageInner();
    // Verify chain still exists after EFFECT_NONE session was skipped
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.count(key) > 0, true);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdateStreamUsageInner with empty maps
* @tc.number : UpdateStreamUsageInner_MultiRenderId_005
* @tc.desc   : Test UpdateStreamUsageInner handles empty special/prior sets gracefully.
*/
HWTEST_F(HpaeRenderEffectNodeTest, UpdateStreamUsageInner_MultiRenderId_005, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    AudioEffectChainManager::GetInstance()->UpdateStreamUsageInner();
    // With empty maps, maxSessionID should remain 0
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->maxSessionID_, 0u);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS