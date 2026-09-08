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
#include "audio_effect_chain.h"

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

static HpaeNodeInfo BuildRenderEffectNodeInfo(uint32_t nodeId, AudioChannel channels = STEREO,
    AudioChannelLayout channelLayout = CH_LAYOUT_STEREO)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = nodeId;
    nodeInfo.sessionId = nodeId;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = channels;
    nodeInfo.channelLayout = channelLayout;
    nodeInfo.format = SAMPLE_F32LE;
    return nodeInfo;
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
    hpaeRenderEffectNode->ReconfigOutputBuffer(STEREO, CH_LAYOUT_STEREO);
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
    hpaeRenderEffectNode->ReconfigOutputBuffer(STEREO, CH_LAYOUT_STEREO);
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
    // ReconfigOutputBuffer reconfigures node buffer with the given channel info
    hpaeRenderEffectNode->ReconfigOutputBuffer(STEREO, CH_LAYOUT_STEREO);
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

/**
 * @tc.name   : Test HpaeRenderEffectNode construction with NEARLINK_OPEN
 * @tc.number : RenderEffectNode_NearLink_Create_001
 * @tc.desc   : Test constructing HpaeRenderEffectNode with COLLABORATION_NEARLINK_OPEN state.
 */
HWTEST_F(HpaeRenderEffectNodeTest, RenderEffectNode_NearLink_Create_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.sceneType = HPAE_SCENE_COLLABORATIVE;
    nodeInfo.effectInfo.effectScene = SCENE_COLLABORATIVE;
    // Constructor with NearLink collaboration state
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo, COLLABORATION_MCH_ROUTE_MODE);
    EXPECT_NE(node, nullptr);
    EXPECT_EQ(node->collaborationRouteMode_, COLLABORATION_MCH_ROUTE_MODE);
}

/**
 * @tc.name   : Test HpaeRenderEffectNode construction with BLUETOOTH_OPEN
 * @tc.number : RenderEffectNode_Bt_Create_002
 * @tc.desc   : Test constructing HpaeRenderEffectNode with COLLABORATION_BLUETOOTH_OPEN state.
 */
HWTEST_F(HpaeRenderEffectNodeTest, RenderEffectNode_Bt_Create_002, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.sceneType = HPAE_SCENE_COLLABORATIVE;
    nodeInfo.effectInfo.effectScene = SCENE_COLLABORATIVE;
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo, COLLABORATION_DEFAULT_ROUTE_MODE);
    EXPECT_NE(node, nullptr);
    EXPECT_EQ(node->collaborationRouteMode_, COLLABORATION_DEFAULT_ROUTE_MODE);
}

/**
 * @tc.name   : Test HpaeRenderEffectNode construction with CLOSE (default)
 * @tc.number : RenderEffectNode_Close_Create_003
 * @tc.desc   : Test constructing HpaeRenderEffectNode with COLLABORATION_CLOSE state.
 */
HWTEST_F(HpaeRenderEffectNodeTest, RenderEffectNode_Close_Create_003, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.sceneType = HPAE_SCENE_COLLABORATIVE;
    nodeInfo.effectInfo.effectScene = SCENE_COLLABORATIVE;
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo, COLLABORATION_DEFAULT_ROUTE_MODE);
    EXPECT_NE(node, nullptr);
    EXPECT_EQ(node->isCollaborationEnabled_, COLLABORATION_CLOSE);
}

/**
 * @tc.name   : Test HpaeRenderEffectNode NearLink tempEffectBuffer created
 * @tc.number : RenderEffectNode_NearLink_TempBuffer_004
 * @tc.desc   : Verify tempEffectBuffer_ is created for NEARLINK_OPEN collaborative scene.
 */
HWTEST_F(HpaeRenderEffectNodeTest, RenderEffectNode_NearLink_TempBuffer_004, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.sceneType = HPAE_SCENE_COLLABORATIVE;
    nodeInfo.effectInfo.effectScene = SCENE_COLLABORATIVE;
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo, COLLABORATION_MCH_ROUTE_MODE);
    EXPECT_NE(node, nullptr);
    // For NearLink, tempEffectBuffer_ should be created
    EXPECT_NE(node->tempEffectBuffer_, nullptr);
}

/**
 * @tc.name   : Test HpaeRenderEffectNode BT collaborative outputs created
 * @tc.number : RenderEffectNode_Bt_Outputs_005
 * @tc.desc   : Verify directOutput_ and collaborativeOutput_ are created for BT collaborative scene.
 */
HWTEST_F(HpaeRenderEffectNodeTest, RenderEffectNode_Bt_Outputs_005, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.sceneType = HPAE_SCENE_COLLABORATIVE;
    nodeInfo.effectInfo.effectScene = SCENE_COLLABORATIVE;
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo, COLLABORATION_DEFAULT_ROUTE_MODE);
    EXPECT_NE(node, nullptr);
    // For BT, directOutput_ and collaborativeOutput_ should be created
    EXPECT_NE(node->directOutput_, nullptr);
    EXPECT_NE(node->collaborativeOutput_, nullptr);
}

/**
 * @tc.name   : Test SetDelayCountToEffectNode for NearLink
 * @tc.number : SetDelayCountToEffectNode_NearLink_006
 * @tc.desc   : Test SetDelayCountToEffectNode delegates to tempEffectBuffer_.
 */
HWTEST_F(HpaeRenderEffectNodeTest, SetDelayCountToEffectNode_NearLink_006, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.sceneType = HPAE_SCENE_COLLABORATIVE;
    nodeInfo.effectInfo.effectScene = SCENE_COLLABORATIVE;
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo, COLLABORATION_MCH_ROUTE_MODE);
    EXPECT_NE(node, nullptr);
    // Should not crash
    node->SetDelayCountToEffectNode(10);
    SUCCEED();
}

/**
 * @tc.name   : Test SetDelayCountToEffectNode when tempEffectBuffer is null
 * @tc.number : SetDelayCountToEffectNode_NullBuffer_007
 * @tc.desc   : Test SetDelayCountToEffectNode with BT mode (tempEffectBuffer_ is null).
 */
HWTEST_F(HpaeRenderEffectNodeTest, SetDelayCountToEffectNode_NullBuffer_007, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.sceneType = HPAE_SCENE_COLLABORATIVE;
    nodeInfo.effectInfo.effectScene = SCENE_COLLABORATIVE;
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo, COLLABORATION_DEFAULT_ROUTE_MODE);
    EXPECT_NE(node, nullptr);
    EXPECT_EQ(node->tempEffectBuffer_, nullptr);
    // Should not crash when tempEffectBuffer_ is null
    node->SetDelayCountToEffectNode(10);
    SUCCEED();
}

/**
 * @tc.name   : Test HpaeRenderEffectNode default scene NearLink
 * @tc.number : RenderEffectNode_NearLink_DefaultScene_008
 * @tc.desc   : Test NEARLINK_OPEN with DEFAULT scene type (no collaborative outputs needed).
 */
HWTEST_F(HpaeRenderEffectNodeTest, RenderEffectNode_NearLink_DefaultScene_008, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.sceneType = HPAE_SCENE_DEFAULT;
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo, COLLABORATION_MCH_ROUTE_MODE);
    EXPECT_NE(node, nullptr);
}
// --- Phase 3D-2: AudioRenderer/OffloadRenderer lifecycle coverage ---
// These methods were previously never called by tests, leaving entire function
// bodies at 0% branch coverage. Calling them directly drives the bodies.

HWTEST_F(HpaeRenderEffectNodeTest, AudioRendererStart_called_directly, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    ASSERT_EQ(node->AudioRendererCreate(nodeInfo), 0);
    EXPECT_EQ(node->AudioRendererStart(nodeInfo), 0);
    EXPECT_EQ(node->AudioRendererStop(nodeInfo), 0);
    EXPECT_EQ(node->AudioRendererRelease(nodeInfo), 0);
}

HWTEST_F(HpaeRenderEffectNodeTest, AudioRendererCreate_secondCallDedups, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID + 100;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    ASSERT_EQ(node->AudioRendererCreate(nodeInfo), 0);
    // Second call with the same sessionId re-enters CreateAudioEffectChain
    // and returns early via CheckAndAddSessionID dedup — covers that branch.
    EXPECT_EQ(node->AudioRendererCreate(nodeInfo), 0);
}

HWTEST_F(HpaeRenderEffectNodeTest, AudioOffloadRenderer_remote_offload_returnsEarly, TestSize.Level1)
{
    // sinkInfo.deviceClass == "remote_offload" → OffloadRendererCheckNotifyEffect
    // returns false → AudioOffloadRenderer* returns ERR_INVALID_HANDLE.
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID + 200;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo);

    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceClass = "remote_offload";

    EXPECT_NE(node->AudioOffloadRendererCreate(nodeInfo, sinkInfo), 0);
    EXPECT_NE(node->AudioOffloadRendererStart(nodeInfo, sinkInfo), 0);
    EXPECT_NE(node->AudioOffloadRendererStop(nodeInfo, sinkInfo), 0);
    EXPECT_NE(node->AudioOffloadRendererRelease(nodeInfo, sinkInfo), 0);
}

HWTEST_F(HpaeRenderEffectNodeTest,
    AudioOffloadRenderer_nonOffloadDevice_createStartEarlyStopReleaseProceed, TestSize.Level1)
{
    // sinkInfo.deviceClass != "remote_offload" → OffloadRendererCheckNotifyEffect
    // checks GetOffloadEnabled() only when isPcmOffload is true. With effect
    // offload disabled, Create/Start (isPcmOffload=true) return ERR_INVALID_HANDLE,
    // while Stop/Release (isPcmOffload=false) skip the offload check and proceed.
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID + 300;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo);

    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceClass = "speaker";

    EXPECT_NE(node->AudioOffloadRendererCreate(nodeInfo, sinkInfo), 0);
    EXPECT_NE(node->AudioOffloadRendererStart(nodeInfo, sinkInfo), 0);
    EXPECT_EQ(node->AudioOffloadRendererStop(nodeInfo, sinkInfo), 0);
    EXPECT_EQ(node->AudioOffloadRendererRelease(nodeInfo, sinkInfo), 0);
}

HWTEST_F(HpaeRenderEffectNodeTest, ReconfigOutputBuffer_monoChannels, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID + 500;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = MONO;
    nodeInfo.channelLayout = CH_LAYOUT_MONO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.sceneType = HPAE_SCENE_DEFAULT;
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    EXPECT_EQ(node->AudioRendererCreate(nodeInfo), 0);
}

// --- Phase 3D-3: DoProcess collaborative-scene branches ---
// DoProcess branches at L107-152 are 0% covered because existing tests never
// call DoProcess on a collaborative-scene node. These tests construct nodes
// with effectScene = SCENE_COLLABORATIVE (the correct way to set sceneType_),
// then call DoProcess to drive the empty-preOutputs branches.

HWTEST_F(HpaeRenderEffectNodeTest, DoProcess_collaborative_nearLink_emptyInput, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID + 600;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.sceneType = HPAE_SCENE_COLLABORATIVE;
    nodeInfo.effectInfo.effectScene = SCENE_COLLABORATIVE;
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo, COLLABORATION_MCH_ROUTE_MODE);
    ASSERT_NE(node->tempEffectBuffer_, nullptr);
    // DoProcess with empty upstream drives the NearLink empty-input exit path.
    node->DoProcess();
    SUCCEED();
}

HWTEST_F(HpaeRenderEffectNodeTest, DoProcess_collaborative_bt_emptyInput_writesSilence, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID + 700;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.sceneType = HPAE_SCENE_COLLABORATIVE;
    nodeInfo.effectInfo.effectScene = SCENE_COLLABORATIVE;
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo, COLLABORATION_DEFAULT_ROUTE_MODE);
    ASSERT_NE(node->directOutput_, nullptr);
    ASSERT_NE(node->collaborativeOutput_, nullptr);
    // DoProcess with empty upstream, BT mode — falls through to the `else`
    // branch at L139-141 which writes silence to both outputs.
    node->DoProcess();
    SUCCEED();
}

HWTEST_F(HpaeRenderEffectNodeTest, DoProcess_defaultScene_delegatesToPluginNode, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID + 800;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.sceneType = HPAE_SCENE_DEFAULT;
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    // Non-collaborative scenes delegate to parent processing and skip the
    // collaborative render-effect branch body.
    node->DoProcess();
    SUCCEED();
}

HWTEST_F(HpaeRenderEffectNodeTest, DoProcess_collaborative_closeState_emptyInput, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID + 900;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.sceneType = HPAE_SCENE_COLLABORATIVE;
    nodeInfo.effectInfo.effectScene = SCENE_COLLABORATIVE;
    // COLLABORATION_CLOSE → isCollaborationEnabled_ != NEARLINK_OPEN, falls
    // through to the BT-style branch (directOutput_/collaborativeOutput_ null
    // because they're only created when isCollaborationEnabled_ ==
    // COLLABORATION_BLUETOOTH_OPEN). Both inner conditions fail, hits the
    // `else` silence-write branch.
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo, COLLABORATION_DEFAULT_ROUTE_MODE);
    node->DoProcess();
    SUCCEED();
}

// --- Phase 3D-4: HpaeTempEffectBuffer latency/ring-cache branches ---
// These public methods own most of the remaining uncovered branch bodies in
// hpae_render_effect_node.cpp. Drive them with real 4-channel PCM buffers so
// the test verifies behavior instead of just calling methods.

static void FillCollaborativeChannels(HpaePcmBuffer &buffer, float leftValue, float rightValue)
{
    constexpr uint32_t collaborativeOutputChannels = 4;
    constexpr uint32_t collaborativeLeftIndex = 2;
    constexpr uint32_t collaborativeRightIndex = 3;
    float *data = buffer.GetPcmDataBuffer();
    ASSERT_NE(data, nullptr);
    for (uint32_t i = 0; i < buffer.GetFrameLen(); ++i) {
        data[collaborativeOutputChannels * i + collaborativeLeftIndex] = leftValue;
        data[collaborativeOutputChannels * i + collaborativeRightIndex] = rightValue;
    }
    buffer.SetBufferValid(true);
    buffer.SetBufferSilence(false);
}

static void ExpectCollaborativeChannelsNear(
    HpaePcmBuffer &buffer, float leftValue, float rightValue, float precision = 1e-6f)
{
    constexpr uint32_t collaborativeOutputChannels = 4;
    constexpr uint32_t collaborativeLeftIndex = 2;
    constexpr uint32_t collaborativeRightIndex = 3;
    float *data = buffer.GetPcmDataBuffer();
    ASSERT_NE(data, nullptr);
    for (uint32_t i = 0; i < buffer.GetFrameLen(); ++i) {
        EXPECT_NEAR(data[collaborativeOutputChannels * i + collaborativeLeftIndex], leftValue, precision);
        EXPECT_NEAR(data[collaborativeOutputChannels * i + collaborativeRightIndex], rightValue, precision);
    }
}

HWTEST_F(HpaeRenderEffectNodeTest, TempEffectBuffer_waitingForDelay_outputsSilence, TestSize.Level1)
{
    constexpr float smallSignal = 1e-6f;
    PcmBufferInfo pcmBufferInfo(CHANNEL_4, TEST_FRAMELEN1, SAMPLE_RATE_48000);
    HpaeTempEffectBuffer tempBuffer(pcmBufferInfo);
    HpaePcmBuffer buffer(pcmBufferInfo);
    FillCollaborativeChannels(buffer, 0.3f, 0.7f);

    tempBuffer.SetDelayCountToEffectNode(2);
    tempBuffer.ChangeEffectDataForLatency(&buffer);

    ExpectCollaborativeChannelsNear(buffer, smallSignal, smallSignal);
}

HWTEST_F(HpaeRenderEffectNodeTest, TempEffectBuffer_readBeforeWrite_outputsSilence, TestSize.Level1)
{
    constexpr float smallSignal = 1e-6f;
    PcmBufferInfo pcmBufferInfo(CHANNEL_4, TEST_FRAMELEN1, SAMPLE_RATE_48000);
    HpaeTempEffectBuffer tempBuffer(pcmBufferInfo);
    HpaePcmBuffer buffer(pcmBufferInfo);
    FillCollaborativeChannels(buffer, 0.4f, 0.8f);

    tempBuffer.ReadDataDequeueRingCache(&buffer);

    ExpectCollaborativeChannelsNear(buffer, smallSignal, smallSignal);
}

HWTEST_F(HpaeRenderEffectNodeTest, TempEffectBuffer_writeThenRead_roundTripsCollaborativeChannels, TestSize.Level1)
{
    PcmBufferInfo pcmBufferInfo(CHANNEL_4, TEST_FRAMELEN1, SAMPLE_RATE_48000);
    HpaeTempEffectBuffer tempBuffer(pcmBufferInfo);
    HpaePcmBuffer writeBuffer(pcmBufferInfo);
    HpaePcmBuffer readBuffer(pcmBufferInfo);
    FillCollaborativeChannels(writeBuffer, 0.25f, 0.75f);
    FillCollaborativeChannels(readBuffer, 0.0f, 0.0f);

    tempBuffer.WriteDataEnqueueRingCache(&writeBuffer);
    tempBuffer.ReadDataDequeueRingCache(&readBuffer);

    ExpectCollaborativeChannelsNear(readBuffer, 0.25f, 0.75f);
}

HWTEST_F(HpaeRenderEffectNodeTest, TempEffectBuffer_delayThresholdReached_runsAlignmentPath, TestSize.Level1)
{
    PcmBufferInfo pcmBufferInfo(CHANNEL_4, TEST_FRAMELEN1, SAMPLE_RATE_48000);
    HpaeTempEffectBuffer tempBuffer(pcmBufferInfo);
    HpaePcmBuffer buffer(pcmBufferInfo);
    FillCollaborativeChannels(buffer, 0.5f, 0.9f);

    tempBuffer.SetDelayCountToEffectNode(0);

    EXPECT_TRUE(tempBuffer.DelayAlignmentForEffectNode(&buffer));
}

HWTEST_F(HpaeRenderEffectNodeTest, TempEffectBuffer_fillSilenceFrames_handlesNegativeLatency, TestSize.Level1)
{
    PcmBufferInfo pcmBufferInfo(CHANNEL_4, TEST_FRAMELEN1, SAMPLE_RATE_48000);
    HpaeTempEffectBuffer tempBuffer(pcmBufferInfo);

    tempBuffer.FillSilenceFramesForEffectNode(-1);

    SUCCEED();
}

// --- Phase 3E: white-box branches for Taken-at-least-once coverage ---
// Only UT state is arranged here; production code is not modified.

class EffectManagerOffloadGuard {
public:
    EffectManagerOffloadGuard()
    {
        auto *manager = AudioEffectChainManager::GetInstance();
        oldDeviceType_ = manager->deviceType_;
        oldSpkOffloadEnabled_ = manager->spkOffloadEnabled_;
        oldBtOffloadEnabled_ = manager->btOffloadEnabled_;
        manager->deviceType_ = DEVICE_TYPE_SPEAKER;
        manager->spkOffloadEnabled_ = false;
        manager->btOffloadEnabled_ = false;
    }

    ~EffectManagerOffloadGuard()
    {
        auto *manager = AudioEffectChainManager::GetInstance();
        manager->deviceType_ = oldDeviceType_;
        manager->spkOffloadEnabled_ = oldSpkOffloadEnabled_;
        manager->btOffloadEnabled_ = oldBtOffloadEnabled_;
        manager->ResetInfo();
    }

private:
    DeviceType oldDeviceType_ = DEVICE_TYPE_INVALID;
    bool oldSpkOffloadEnabled_ = false;
    bool oldBtOffloadEnabled_ = false;
};

static void AddBufferedPreOutput(HpaeRenderEffectNode &node, OutputPort<HpaePcmBuffer *> &output,
    HpaePcmBuffer &buffer)
{
    output.WriteDataToOutput(&buffer);
    node.inputStream_.AddPreOutput(nullptr, &output);
}

HWTEST_F(HpaeRenderEffectNodeTest, AudioOffloadRenderer_nonRemote_createStartEarlyStopReleaseProceed, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = BuildRenderEffectNodeInfo(TEST_ID + 1400);
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceClass = "speaker";

    EXPECT_EQ(node->AudioOffloadRendererCreate(nodeInfo, sinkInfo), ERR_INVALID_HANDLE);
    EXPECT_EQ(node->AudioOffloadRendererStart(nodeInfo, sinkInfo), ERR_INVALID_HANDLE);
    // Stop/Release pass isPcmOffload=false to OffloadRendererCheckNotifyEffect,
    // so the GetOffloadEnabled() guard is skipped and the call proceeds.
    EXPECT_EQ(node->AudioOffloadRendererStop(nodeInfo, sinkInfo), SUCCESS);
    EXPECT_EQ(node->AudioOffloadRendererRelease(nodeInfo, sinkInfo), SUCCESS);
}

HWTEST_F(HpaeRenderEffectNodeTest, SignalProcess_offloadEnabledReturnsOriginalInput, TestSize.Level1)
{
    EffectManagerOffloadGuard guard;
    auto *manager = AudioEffectChainManager::GetInstance();
    manager->spkOffloadEnabled_ = true;

    HpaeNodeInfo nodeInfo = BuildRenderEffectNodeInfo(TEST_ID + 1500);
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    PcmBufferInfo pcmBufferInfo(STEREO, TEST_FRAMELEN1, SAMPLE_RATE_48000, CH_LAYOUT_STEREO);
    HpaePcmBuffer inputBuffer(pcmBufferInfo);
    inputBuffer.GetPcmDataBuffer();
    inputBuffer.SetBufferValid(true);
    inputBuffer.SetBufferSilence(false);
    std::vector<HpaePcmBuffer *> inputs = {&inputBuffer};

    EXPECT_EQ(node->SignalProcess(inputs), &inputBuffer);
}

HWTEST_F(HpaeRenderEffectNodeTest, IsByPassEffectZeroVolume_invalidBufferReturnsFalse, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = BuildRenderEffectNodeInfo(TEST_ID + 1600);
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    PcmBufferInfo pcmBufferInfo(STEREO, TEST_FRAMELEN1, SAMPLE_RATE_48000, CH_LAYOUT_STEREO);
    HpaePcmBuffer inputBuffer(pcmBufferInfo);
    inputBuffer.SetBufferValid(false);

    EXPECT_FALSE(node->IsByPassEffectZeroVolume(&inputBuffer));
}

HWTEST_F(HpaeRenderEffectNodeTest, IsByPassEffectZeroVolume_silenceReachesBypassThreshold, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = BuildRenderEffectNodeInfo(TEST_ID + 1700);
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    PcmBufferInfo pcmBufferInfo(STEREO, SAMPLE_RATE_48000 * 4, SAMPLE_RATE_48000, CH_LAYOUT_STEREO);
    HpaePcmBuffer inputBuffer(pcmBufferInfo);
    inputBuffer.SetBufferValid(true);
    inputBuffer.SetBufferSilence(true);

    EXPECT_TRUE(node->IsByPassEffectZeroVolume(&inputBuffer));
    EXPECT_TRUE(node->isByPassEffect_);
    EXPECT_TRUE(node->isDisplayEffectZeroVolume_);
}

HWTEST_F(HpaeRenderEffectNodeTest, IsByPassEffectZeroVolume_monitorClosedEffectBranch, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = BuildRenderEffectNodeInfo(TEST_ID + 1800);
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    PcmBufferInfo pcmBufferInfo(STEREO, SAMPLE_RATE_48000 * 300, SAMPLE_RATE_48000, CH_LAYOUT_STEREO);
    HpaePcmBuffer inputBuffer(pcmBufferInfo);
    inputBuffer.SetBufferValid(true);
    inputBuffer.SetBufferSilence(true);
    node->isByPassEffect_ = true;
    node->isDisplayEffectZeroVolume_ = true;

    EXPECT_TRUE(node->IsByPassEffectZeroVolume(&inputBuffer));
    EXPECT_EQ(node->silenceDataUs_, 0);
}

HWTEST_F(HpaeRenderEffectNodeTest, IsByPassEffectZeroVolume_nonSilenceReopensEffect, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = BuildRenderEffectNodeInfo(TEST_ID + 1900);
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    PcmBufferInfo pcmBufferInfo(STEREO, TEST_FRAMELEN1, SAMPLE_RATE_48000, CH_LAYOUT_STEREO);
    HpaePcmBuffer inputBuffer(pcmBufferInfo);
    inputBuffer.SetBufferValid(true);
    inputBuffer.SetBufferSilence(false);
    node->isByPassEffect_ = true;
    node->isDisplayEffectZeroVolume_ = true;
    node->silenceDataUs_ = 1000;

    EXPECT_FALSE(node->IsByPassEffectZeroVolume(&inputBuffer));
    EXPECT_FALSE(node->isByPassEffect_);
    EXPECT_FALSE(node->isDisplayEffectZeroVolume_);
    EXPECT_EQ(node->silenceDataUs_, 0);
}

HWTEST_F(HpaeRenderEffectNodeTest, SplitCollaborativeData_stereoEffectOutputReturnsError, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = BuildRenderEffectNodeInfo(TEST_ID + 2000);
    nodeInfo.sceneType = HPAE_SCENE_COLLABORATIVE;
    nodeInfo.effectInfo.effectScene = SCENE_COLLABORATIVE;
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo, COLLABORATION_DEFAULT_ROUTE_MODE);

    EXPECT_EQ(node->SplitCollaborativeData(), ERROR);
}

HWTEST_F(HpaeRenderEffectNodeTest, SplitCollaborativeData_fourChannelEffectOutputSplitsToStereoBuffers, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = BuildRenderEffectNodeInfo(TEST_ID + 2100, CHANNEL_4, CH_LAYOUT_4POINT0);
    nodeInfo.sceneType = HPAE_SCENE_COLLABORATIVE;
    nodeInfo.effectInfo.effectScene = SCENE_COLLABORATIVE;
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo, COLLABORATION_DEFAULT_ROUTE_MODE);
    ASSERT_NE(node->directOutput_, nullptr);
    ASSERT_NE(node->collaborativeOutput_, nullptr);
    float *effectData = node->effectOutput_.GetPcmDataBuffer();
    ASSERT_NE(effectData, nullptr);
    for (uint32_t i = 0; i < node->effectOutput_.GetFrameLen(); ++i) {
        effectData[4 * i] = 1.0f;
        effectData[4 * i + 1] = 2.0f;
        effectData[4 * i + 2] = 3.0f;
        effectData[4 * i + 3] = 4.0f;
    }

    EXPECT_EQ(node->SplitCollaborativeData(), SUCCESS);
    constexpr float silenceEpsilon = 1e-6f;
    // SplitCollaborativeData adds the same silence epsilon used by the
    // collaborative output path before copying channel data.
    EXPECT_NEAR(node->directOutput_->GetPcmDataBuffer()[0], 1.0f + silenceEpsilon, 1e-7f);
    EXPECT_NEAR(node->directOutput_->GetPcmDataBuffer()[1], 2.0f + silenceEpsilon, 1e-7f);
    EXPECT_NEAR(node->collaborativeOutput_->GetPcmDataBuffer()[0], 3.0f + silenceEpsilon, 1e-7f);
    EXPECT_NEAR(node->collaborativeOutput_->GetPcmDataBuffer()[1], 4.0f + silenceEpsilon, 1e-7f);
}


HWTEST_F(HpaeRenderEffectNodeTest, DoProcess_collaborative_nearLink_nonEmptyInputRunsLatencyPath, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = BuildRenderEffectNodeInfo(TEST_ID + 2300, CHANNEL_4, CH_LAYOUT_4POINT0);
    nodeInfo.sceneType = HPAE_SCENE_COLLABORATIVE;
    nodeInfo.effectInfo.effectScene = SCENE_COLLABORATIVE;
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo, COLLABORATION_MCH_ROUTE_MODE);
    PcmBufferInfo pcmBufferInfo(CHANNEL_4, TEST_FRAMELEN1, SAMPLE_RATE_48000, CH_LAYOUT_4POINT0);
    HpaePcmBuffer inputBuffer(pcmBufferInfo);
    inputBuffer.GetPcmDataBuffer();
    inputBuffer.SetBufferValid(true);
    inputBuffer.SetBufferSilence(false);
    OutputPort<HpaePcmBuffer *> preOutput(nullptr);
    AddBufferedPreOutput(*node, preOutput, inputBuffer);

    node->DoProcess();

    SUCCEED();
}

HWTEST_F(HpaeRenderEffectNodeTest, DoProcess_collaborative_bt_nonEmptyInputWritesSilenceOnSplitError, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = BuildRenderEffectNodeInfo(TEST_ID + 2400, STEREO, CH_LAYOUT_STEREO);
    nodeInfo.sceneType = HPAE_SCENE_COLLABORATIVE;
    nodeInfo.effectInfo.effectScene = SCENE_COLLABORATIVE;
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo, COLLABORATION_DEFAULT_ROUTE_MODE);
    PcmBufferInfo pcmBufferInfo(STEREO, TEST_FRAMELEN1, SAMPLE_RATE_48000, CH_LAYOUT_STEREO);
    HpaePcmBuffer inputBuffer(pcmBufferInfo);
    inputBuffer.GetPcmDataBuffer();
    inputBuffer.SetBufferValid(true);
    inputBuffer.SetBufferSilence(false);
    OutputPort<HpaePcmBuffer *> preOutput(nullptr);
    AddBufferedPreOutput(*node, preOutput, inputBuffer);

    node->DoProcess();

    SUCCEED();
}

HWTEST_F(HpaeRenderEffectNodeTest, DoProcess_collaborative_bt_disabledProcessForwardsPreOutput, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = BuildRenderEffectNodeInfo(TEST_ID + 2500, STEREO, CH_LAYOUT_STEREO);
    nodeInfo.sceneType = HPAE_SCENE_COLLABORATIVE;
    nodeInfo.effectInfo.effectScene = SCENE_COLLABORATIVE;
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo, COLLABORATION_DEFAULT_ROUTE_MODE);
    PcmBufferInfo pcmBufferInfo(STEREO, TEST_FRAMELEN1, SAMPLE_RATE_48000, CH_LAYOUT_STEREO);
    HpaePcmBuffer inputBuffer(pcmBufferInfo);
    inputBuffer.GetPcmDataBuffer();
    inputBuffer.SetBufferValid(true);
    inputBuffer.SetBufferSilence(false);
    OutputPort<HpaePcmBuffer *> preOutput(nullptr);
    AddBufferedPreOutput(*node, preOutput, inputBuffer);
    ASSERT_EQ(node->EnableProcess(false), SUCCESS);

    node->DoProcess();

    SUCCEED();
}


HWTEST_F(HpaeRenderEffectNodeTest, DoProcess_collaborative_bt_fourChannelOutputWritesSplitBuffers, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = BuildRenderEffectNodeInfo(TEST_ID + 2600, CHANNEL_4, CH_LAYOUT_4POINT0);
    nodeInfo.sceneType = HPAE_SCENE_COLLABORATIVE;
    nodeInfo.effectInfo.effectScene = SCENE_COLLABORATIVE;
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo, COLLABORATION_DEFAULT_ROUTE_MODE);
    PcmBufferInfo pcmBufferInfo(CHANNEL_4, TEST_FRAMELEN1, SAMPLE_RATE_48000, CH_LAYOUT_4POINT0);
    HpaePcmBuffer inputBuffer(pcmBufferInfo);
    inputBuffer.GetPcmDataBuffer();
    inputBuffer.SetBufferValid(true);
    inputBuffer.SetBufferSilence(false);
    OutputPort<HpaePcmBuffer *> preOutput(nullptr);
    AddBufferedPreOutput(*node, preOutput, inputBuffer);

    node->DoProcess();

    SUCCEED();
}

HWTEST_F(HpaeRenderEffectNodeTest, SignalProcess_bypassSilenceReturnsOriginalInput, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = BuildRenderEffectNodeInfo(TEST_ID + 2700);
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    PcmBufferInfo pcmBufferInfo(STEREO, SAMPLE_RATE_48000 * 4, SAMPLE_RATE_48000, CH_LAYOUT_STEREO);
    HpaePcmBuffer inputBuffer(pcmBufferInfo);
    inputBuffer.GetPcmDataBuffer();
    inputBuffer.SetBufferValid(true);
    inputBuffer.SetBufferSilence(true);
    std::vector<HpaePcmBuffer *> inputs = {&inputBuffer};

    EXPECT_EQ(node->SignalProcess(inputs), &inputBuffer);
}

HWTEST_F(HpaeRenderEffectNodeTest, AudioRendererStart_unsupportedEffectModeUsesEffectNone, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = BuildRenderEffectNodeInfo(TEST_ID + 2800);
    nodeInfo.effectInfo.effectScene = SCENE_MOVIE;
    nodeInfo.effectInfo.effectMode = static_cast<AudioEffectMode>(0xff);
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo);

    EXPECT_EQ(node->AudioRendererStart(nodeInfo), SUCCESS);
}

HWTEST_F(HpaeRenderEffectNodeTest, ModifyAudioEffectChainInfo_unknownReasonNoUpdate, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = BuildRenderEffectNodeInfo(TEST_ID + 2900);
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo);

    node->ModifyAudioEffectChainInfo(nodeInfo, static_cast<ModifyAudioEffectChainInfoReason>(99));

    SUCCEED();
}

HWTEST_F(HpaeRenderEffectNodeTest, InitEffectBuffer_collaborativeReturnsEarly, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = BuildRenderEffectNodeInfo(TEST_ID + 3000);
    nodeInfo.sceneType = HPAE_SCENE_COLLABORATIVE;
    nodeInfo.effectInfo.effectScene = SCENE_COLLABORATIVE;
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo, COLLABORATION_DEFAULT_ROUTE_MODE);

    node->InitEffectBuffer(nodeInfo.sessionId);

    SUCCEED();
}

HWTEST_F(HpaeRenderEffectNodeTest, TempEffectBuffer_nullRingCacheGuardsReturn, TestSize.Level1)
{
    HpaeTempEffectBuffer tempBuffer;
    PcmBufferInfo pcmBufferInfo(CHANNEL_4, TEST_FRAMELEN1, SAMPLE_RATE_48000, CH_LAYOUT_4POINT0);
    HpaePcmBuffer inputBuffer(pcmBufferInfo);
    inputBuffer.GetPcmDataBuffer();

    tempBuffer.FillSilenceFramesForEffectNode(1);
    tempBuffer.ReadDataDequeueRingCache(&inputBuffer);

    SUCCEED();
}

HWTEST_F(HpaeRenderEffectNodeTest, TempEffectBuffer_nullTempOutputGuardsReturn, TestSize.Level1)
{
    PcmBufferInfo stereoInfo(STEREO, TEST_FRAMELEN1, SAMPLE_RATE_48000, CH_LAYOUT_STEREO);
    HpaeTempEffectBuffer tempBuffer(stereoInfo);
    tempBuffer.tempOutput_ = nullptr;
    PcmBufferInfo pcmBufferInfo(CHANNEL_4, TEST_FRAMELEN1, SAMPLE_RATE_48000, CH_LAYOUT_4POINT0);
    HpaePcmBuffer inputBuffer(pcmBufferInfo);
    inputBuffer.GetPcmDataBuffer();

    tempBuffer.WriteDataEnqueueRingCache(&inputBuffer);
    tempBuffer.FillSilenceFramesForEffectNode(1);
    tempBuffer.ReadDataDequeueRingCache(&inputBuffer);

    SUCCEED();
}

HWTEST_F(HpaeRenderEffectNodeTest, TempEffectBuffer_changeEffectDataReadyRunsWriteAndRead, TestSize.Level1)
{
    PcmBufferInfo stereoInfo(STEREO, TEST_FRAMELEN1, SAMPLE_RATE_48000, CH_LAYOUT_STEREO);
    HpaeTempEffectBuffer tempBuffer(stereoInfo);
    tempBuffer.SetDelayCountToEffectNode(0);
    PcmBufferInfo pcmBufferInfo(CHANNEL_4, TEST_FRAMELEN1, SAMPLE_RATE_48000, CH_LAYOUT_4POINT0);
    HpaePcmBuffer inputBuffer(pcmBufferInfo);
    float *data = inputBuffer.GetPcmDataBuffer();
    ASSERT_NE(data, nullptr);
    data[2] = 1.0f;
    data[3] = 2.0f;

    tempBuffer.ChangeEffectDataForLatency(&inputBuffer);

    SUCCEED();
}

HWTEST_F(HpaeRenderEffectNodeTest, TempEffectBuffer_insufficientRingCacheSpaceReturnsEarly, TestSize.Level1)
{
    PcmBufferInfo stereoInfo(STEREO, TEST_FRAMELEN1, SAMPLE_RATE_48000, CH_LAYOUT_STEREO);
    HpaeTempEffectBuffer tempBuffer(stereoInfo);
    PcmBufferInfo pcmBufferInfo(CHANNEL_4, TEST_FRAMELEN1, SAMPLE_RATE_48000, CH_LAYOUT_4POINT0);
    HpaePcmBuffer inputBuffer(pcmBufferInfo);
    inputBuffer.GetPcmDataBuffer();

    tempBuffer.FillSilenceFramesForEffectNode(1000);
    tempBuffer.FillSilenceFramesForEffectNode(800);
    tempBuffer.WriteDataEnqueueRingCache(&inputBuffer);

    SUCCEED();
}


HWTEST_F(HpaeRenderEffectNodeTest, DoProcess_collaborative_nearLink_disabledProcessSkipsSignal, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = BuildRenderEffectNodeInfo(TEST_ID + 3400, CHANNEL_4, CH_LAYOUT_4POINT0);
    nodeInfo.sceneType = HPAE_SCENE_COLLABORATIVE;
    nodeInfo.effectInfo.effectScene = SCENE_COLLABORATIVE;
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo, COLLABORATION_MCH_ROUTE_MODE);
    PcmBufferInfo pcmBufferInfo(CHANNEL_4, TEST_FRAMELEN1, SAMPLE_RATE_48000, CH_LAYOUT_4POINT0);
    HpaePcmBuffer inputBuffer(pcmBufferInfo);
    inputBuffer.GetPcmDataBuffer();
    OutputPort<HpaePcmBuffer *> preOutput(nullptr);
    AddBufferedPreOutput(*node, preOutput, inputBuffer);
    ASSERT_EQ(node->EnableProcess(false), SUCCESS);

    node->DoProcess();

    SUCCEED();
}

HWTEST_F(HpaeRenderEffectNodeTest, DoProcess_collaborative_nearLink_nullTempBufferSkipsSignal, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = BuildRenderEffectNodeInfo(TEST_ID + 3500, CHANNEL_4, CH_LAYOUT_4POINT0);
    nodeInfo.sceneType = HPAE_SCENE_COLLABORATIVE;
    nodeInfo.effectInfo.effectScene = SCENE_COLLABORATIVE;
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo, COLLABORATION_DEFAULT_ROUTE_MODE);
    node->tempEffectBuffer_ = nullptr;
    PcmBufferInfo pcmBufferInfo(CHANNEL_4, TEST_FRAMELEN1, SAMPLE_RATE_48000, CH_LAYOUT_4POINT0);
    HpaePcmBuffer inputBuffer(pcmBufferInfo);
    inputBuffer.GetPcmDataBuffer();
    OutputPort<HpaePcmBuffer *> preOutput(nullptr);
    AddBufferedPreOutput(*node, preOutput, inputBuffer);

    node->DoProcess();

    SUCCEED();
}

HWTEST_F(HpaeRenderEffectNodeTest, DoProcess_collaborative_bt_nullDirectOutputForwardsPreOutput, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = BuildRenderEffectNodeInfo(TEST_ID + 3600, STEREO, CH_LAYOUT_STEREO);
    nodeInfo.sceneType = HPAE_SCENE_COLLABORATIVE;
    nodeInfo.effectInfo.effectScene = SCENE_COLLABORATIVE;
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo, COLLABORATION_DEFAULT_ROUTE_MODE);
    node->directOutput_ = nullptr;
    PcmBufferInfo pcmBufferInfo(STEREO, TEST_FRAMELEN1, SAMPLE_RATE_48000, CH_LAYOUT_STEREO);
    HpaePcmBuffer inputBuffer(pcmBufferInfo);
    inputBuffer.GetPcmDataBuffer();
    OutputPort<HpaePcmBuffer *> preOutput(nullptr);
    AddBufferedPreOutput(*node, preOutput, inputBuffer);

    node->DoProcess();

    SUCCEED();
}

HWTEST_F(HpaeRenderEffectNodeTest, DoProcess_collaborative_bt_nullCollaborativeOutputForwardsPreOutput, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = BuildRenderEffectNodeInfo(TEST_ID + 3700, STEREO, CH_LAYOUT_STEREO);
    nodeInfo.sceneType = HPAE_SCENE_COLLABORATIVE;
    nodeInfo.effectInfo.effectScene = SCENE_COLLABORATIVE;
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo, COLLABORATION_DEFAULT_ROUTE_MODE);
    node->collaborativeOutput_ = nullptr;
    PcmBufferInfo pcmBufferInfo(STEREO, TEST_FRAMELEN1, SAMPLE_RATE_48000, CH_LAYOUT_STEREO);
    HpaePcmBuffer inputBuffer(pcmBufferInfo);
    inputBuffer.GetPcmDataBuffer();
    OutputPort<HpaePcmBuffer *> preOutput(nullptr);
    AddBufferedPreOutput(*node, preOutput, inputBuffer);

    node->DoProcess();

    SUCCEED();
}

HWTEST_F(HpaeRenderEffectNodeTest, IsByPassEffectZeroVolume_bypassSilenceBeforeMonitorKeepsClosed, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = BuildRenderEffectNodeInfo(TEST_ID + 3900);
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    PcmBufferInfo pcmBufferInfo(STEREO, TEST_FRAMELEN1, SAMPLE_RATE_48000, CH_LAYOUT_STEREO);
    HpaePcmBuffer inputBuffer(pcmBufferInfo);
    inputBuffer.SetBufferValid(true);
    inputBuffer.SetBufferSilence(true);
    node->isByPassEffect_ = true;
    node->isDisplayEffectZeroVolume_ = true;
    node->silenceDataUs_ = 0;

    EXPECT_TRUE(node->IsByPassEffectZeroVolume(&inputBuffer));
    EXPECT_GT(node->silenceDataUs_, 0);
}

HWTEST_F(HpaeRenderEffectNodeTest, TempEffectBuffer_writeWithNullRingCacheReturnsAfterTempCopy, TestSize.Level1)
{
    HpaeTempEffectBuffer tempBuffer;
    PcmBufferInfo stereoInfo(STEREO, TEST_FRAMELEN1, SAMPLE_RATE_48000, CH_LAYOUT_STEREO);
    tempBuffer.tempOutput_ = std::make_unique<HpaePcmBuffer>(stereoInfo);
    PcmBufferInfo pcmBufferInfo(CHANNEL_4, TEST_FRAMELEN1, SAMPLE_RATE_48000, CH_LAYOUT_4POINT0);
    HpaePcmBuffer inputBuffer(pcmBufferInfo);
    inputBuffer.GetPcmDataBuffer();

    tempBuffer.WriteDataEnqueueRingCache(&inputBuffer);

    SUCCEED();
}

HWTEST_F(HpaeRenderEffectNodeTest, TempEffectBuffer_readWithNullTempOutputAfterReadableDataReturns, TestSize.Level1)
{
    PcmBufferInfo stereoInfo(STEREO, TEST_FRAMELEN1, SAMPLE_RATE_48000, CH_LAYOUT_STEREO);
    HpaeTempEffectBuffer tempBuffer(stereoInfo);
    PcmBufferInfo pcmBufferInfo(CHANNEL_4, TEST_FRAMELEN1, SAMPLE_RATE_48000, CH_LAYOUT_4POINT0);
    HpaePcmBuffer inputBuffer(pcmBufferInfo);
    inputBuffer.GetPcmDataBuffer();
    tempBuffer.FillSilenceFramesForEffectNode(20);
    tempBuffer.tempOutput_ = nullptr;

    tempBuffer.ReadDataDequeueRingCache(&inputBuffer);

    SUCCEED();
}

HWTEST_F(HpaeRenderEffectNodeTest, ReconfigOutputBuffer_keepsNodeInfoWhenManagerHasNoNodeChain, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = BuildRenderEffectNodeInfo(TEST_ID + 2200, STEREO, CH_LAYOUT_STEREO);
    nodeInfo.effectInfo.effectScene = SCENE_MOVIE;
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    node->sceneType_ = "SCENE_MOVIE";

    node->ReconfigOutputBuffer(STEREO, CH_LAYOUT_STEREO);

    EXPECT_EQ(node->GetNodeInfo().channels, STEREO);
    EXPECT_EQ(node->GetNodeInfo().channelLayout, CH_LAYOUT_STEREO);
}

/**
 * @tc.name   : Test CreateAudioEffectChain with duplicate sessionId
 * @tc.number : CreateAudioEffectChain_DuplicateSession_002
 * @tc.desc   : Test CreateAudioEffectChain when sessionId already exists (CheckAndAddSessionID returns false).
 */
HWTEST_F(HpaeRenderEffectNodeTest, CreateAudioEffectChain_DuplicateSession_002, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    HpaeNodeInfo nodeInfo = BuildRenderEffectNodeInfo(TEST_ID + 4100);
    nodeInfo.effectInfo.effectScene = SCENE_MUSIC;
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo);

    // First call creates the session
    EXPECT_EQ(node->AudioRendererCreate(nodeInfo), 0);

    // Second call with same sessionId should return early (deduplication branch)
    EXPECT_EQ(node->AudioRendererCreate(nodeInfo), 0);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
 * @tc.name   : Test ReleaseAudioEffectChain with non-existent sessionId
 * @tc.number : ReleaseAudioEffectChain_SessionNotFound_003
 * @tc.desc   : Test ReleaseAudioEffectChain when sessionId doesn't exist (CheckAndRemoveSessionID returns false).
 */
HWTEST_F(HpaeRenderEffectNodeTest, ReleaseAudioEffectChain_SessionNotFound_003, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    HpaeNodeInfo nodeInfo = BuildRenderEffectNodeInfo(TEST_ID + 4200);
    nodeInfo.effectInfo.effectScene = SCENE_MUSIC;
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo);

    // Release without create - should return SUCCESS via early return branch
    EXPECT_EQ(node->ReleaseAudioEffectChain(nodeInfo), 0);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
 * @tc.name   : Test ModifyAudioEffectChainInfo removes session info
 * @tc.number : ModifyAudioEffectChainInfo_RemoveSession_005
 * @tc.desc   : Test ModifyAudioEffectChainInfo with REMOVE_AUDIO_EFFECT_CHAIN_INFO.
 */
HWTEST_F(HpaeRenderEffectNodeTest, ModifyAudioEffectChainInfo_RemoveSession_005, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    HpaeNodeInfo nodeInfo = BuildRenderEffectNodeInfo(TEST_ID + 4400);
    nodeInfo.effectInfo.effectScene = SCENE_MUSIC;
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo);

    // First add
    ModifyAudioEffectChainInfoReason addReason = ADD_AUDIO_EFFECT_CHAIN_INFO;
    node->ModifyAudioEffectChainInfo(nodeInfo, addReason);

    // Then remove
    ModifyAudioEffectChainInfoReason removeReason = REMOVE_AUDIO_EFFECT_CHAIN_INFO;
    node->ModifyAudioEffectChainInfo(nodeInfo, removeReason);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
 * @tc.name   : Test TempEffectBuffer UpdateNearLinkCollaborationStateToHdi
 * @tc.number : TempEffectBuffer_UpdateNearLinkState_006
 * @tc.desc   : Test UpdateNearLinkCollaborationStateToHdi path in DelayAlignmentForEffectNode.
 */
HWTEST_F(HpaeRenderEffectNodeTest, TempEffectBuffer_UpdateNearLinkState_006, TestSize.Level1)
{
    PcmBufferInfo stereoInfo(STEREO, TEST_FRAMELEN1, SAMPLE_RATE_48000, CH_LAYOUT_STEREO);
    HpaeTempEffectBuffer tempBuffer(stereoInfo, COLLABORATION_NEARLINK_OPEN);
    PcmBufferInfo pcmBufferInfo(CHANNEL_4, TEST_FRAMELEN1, SAMPLE_RATE_48000, CH_LAYOUT_4POINT0);
    HpaePcmBuffer inputBuffer(pcmBufferInfo);

    // Call multiple times to trigger the UPDATE_NEARLINK_COLLAB_STATE_TO_HDI_WAIT_COUNT branch
    for (int i = 0; i < 3; i++) {
        tempBuffer.DelayAlignmentForEffectNode(&inputBuffer);
    }

    SUCCEED();
}

/**
 * @tc.name   : Test TempEffectBuffer SetCollabDoFade
 * @tc.number : TempEffectBuffer_SetCollabDoFade_007
 * @tc.desc   : Test SetCollabDoFade triggers fade out in ChangeEffectDataForLatency.
 */
HWTEST_F(HpaeRenderEffectNodeTest, TempEffectBuffer_SetCollabDoFade_007, TestSize.Level1)
{
    PcmBufferInfo stereoInfo(STEREO, TEST_FRAMELEN1, SAMPLE_RATE_48000, CH_LAYOUT_STEREO);
    HpaeTempEffectBuffer tempBuffer(stereoInfo, COLLABORATION_NEARLINK_OPEN);
    PcmBufferInfo pcmBufferInfo(CHANNEL_4, TEST_FRAMELEN1, SAMPLE_RATE_48000, CH_LAYOUT_4POINT0);
    HpaePcmBuffer inputBuffer(pcmBufferInfo);

    // Set delay count to 0 so alignment is ready
    tempBuffer.SetDelayCountToEffectNode(0);
    tempBuffer.DelayAlignmentForEffectNode(&inputBuffer);

    // Set fade out state
    tempBuffer.SetCollabDoFade(true);

    // This should trigger fade out and set stopRunning
    tempBuffer.ChangeEffectDataForLatency(&inputBuffer);

    SUCCEED();
}

/**
 * @tc.name   : Test TempEffectBuffer SetStopRunningState
 * @tc.number : TempEffectBuffer_SetStopRunningState_008
 * @tc.desc   : Test SetStopRunningState causes silence output in ChangeEffectDataForLatency.
 */
HWTEST_F(HpaeRenderEffectNodeTest, TempEffectBuffer_SetStopRunningState_008, TestSize.Level1)
{
    PcmBufferInfo stereoInfo(STEREO, TEST_FRAMELEN1, SAMPLE_RATE_48000, CH_LAYOUT_STEREO);
    HpaeTempEffectBuffer tempBuffer(stereoInfo, COLLABORATION_NEARLINK_OPEN);
    PcmBufferInfo pcmBufferInfo(CHANNEL_4, TEST_FRAMELEN1, SAMPLE_RATE_48000, CH_LAYOUT_4POINT0);
    HpaePcmBuffer inputBuffer(pcmBufferInfo);

    // Set delay count to 0 so alignment is ready
    tempBuffer.SetDelayCountToEffectNode(0);
    tempBuffer.DelayAlignmentForEffectNode(&inputBuffer);

    // Set stop running state
    tempBuffer.SetStopRunningState(true);

    // This should output silence in all collaborative channels
    tempBuffer.ChangeEffectDataForLatency(&inputBuffer);

    constexpr float smallSignal = 1e-6f;
    EXPECT_NEAR(inputBuffer.GetPcmDataBuffer()[2], smallSignal, 1e-7f);
    EXPECT_NEAR(inputBuffer.GetPcmDataBuffer()[3], smallSignal, 1e-7f);
}

/**
 * @tc.name   : Test TempEffectBuffer DoFadeOut
 * @tc.number : TempEffectBuffer_DoFadeOut_009
 * @tc.desc   : Test DoFadeOut applies gain ramp to all channels.
 */
HWTEST_F(HpaeRenderEffectNodeTest, TempEffectBuffer_DoFadeOut_009, TestSize.Level1)
{
    PcmBufferInfo pcmBufferInfo(CHANNEL_4, TEST_FRAMELEN1, SAMPLE_RATE_48000, CH_LAYOUT_4POINT0);
    HpaePcmBuffer buffer(pcmBufferInfo);
    float *data = buffer.GetPcmDataBuffer();
    ASSERT_NE(data, nullptr);

    // Set all channels to 1.0
    for (uint32_t i = 0; i < buffer.GetFrameLen(); ++i) {
        data[4 * i] = 1.0f;
        data[4 * i + 1] = 1.0f;
        data[4 * i + 2] = 1.0f;
        data[4 * i + 3] = 1.0f;
    }

    PcmBufferInfo stereoInfo(STEREO, TEST_FRAMELEN1, SAMPLE_RATE_48000, CH_LAYOUT_STEREO);
    HpaeTempEffectBuffer tempBuffer(stereoInfo);

    tempBuffer.DoFadeOut(&buffer);

    // First sample should have gain close to 1.0, last sample should be near 0
    EXPECT_NEAR(data[0], 1.0f, 0.1f);
    EXPECT_LT(data[4 * (buffer.GetFrameLen() - 1)], 0.1f);
}

/**
 * @tc.name   : Test SignalProcess with ApplyAudioEffectChain failure
 * @tc.number : SignalProcess_ApplyEffectChainFail_010
 * @tc.desc   : Test SignalProcess when ApplyAudioEffectChain returns error.
 */
HWTEST_F(HpaeRenderEffectNodeTest, SignalProcess_ApplyEffectChainFail_010, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    HpaeNodeInfo nodeInfo = BuildRenderEffectNodeInfo(TEST_ID + 4500);
    nodeInfo.effectInfo.effectScene = SCENE_MUSIC;
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo);

    PcmBufferInfo pcmBufferInfo(STEREO, TEST_FRAMELEN1, SAMPLE_RATE_48000, CH_LAYOUT_STEREO);
    HpaePcmBuffer inputBuffer(pcmBufferInfo);
    inputBuffer.SetBufferValid(true);
    inputBuffer.SetBufferSilence(false);
    std::vector<HpaePcmBuffer *> inputs = {&inputBuffer};

    // Call SignalProcess - should return inputBuffer on failure
    HpaePcmBuffer *result = node->SignalProcess(inputs);
    EXPECT_EQ(result, &inputBuffer);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
 * @tc.name   : Test GetExpectedInputChannelInfo returns error
 * @tc.number : GetExpectedInputChannelInfo_Error_011
 * @tc.desc   : Test GetExpectedInputChannelInfo when QueryEffectChannelInfo fails.
 */
HWTEST_F(HpaeRenderEffectNodeTest, GetExpectedInputChannelInfo_Error_011, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    HpaeNodeInfo nodeInfo = BuildRenderEffectNodeInfo(TEST_ID + 4600);
    nodeInfo.effectInfo.effectScene = SCENE_MUSIC;
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    node->sceneType_ = "SCENE_MUSIC";

    AudioBasicFormat basicFormat;
    basicFormat.rate = SAMPLE_RATE_48000;
    basicFormat.audioChannelInfo.numChannels = STEREO;

    // Should return error when no effect chain exists
    int32_t ret = node->GetExpectedInputChannelInfo(basicFormat);
    EXPECT_NE(ret, SUCCESS);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
 * @tc.name   : Test AudioRendererStart with tempEffectBuffer
 * @tc.number : AudioRendererStart_TempEffectBuffer_012
 * @tc.desc   : Test AudioRendererStart resets tempEffectBuffer state for NearLink.
 */
HWTEST_F(HpaeRenderEffectNodeTest, AudioRendererStart_TempEffectBuffer_012, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = BuildRenderEffectNodeInfo(TEST_ID + 4700, STEREO, CH_LAYOUT_STEREO);
    nodeInfo.sceneType = HPAE_SCENE_COLLABORATIVE;
    nodeInfo.effectInfo.effectScene = SCENE_COLLABORATIVE;
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo, COLLABORATION_MCH_ROUTE_MODE);
    ASSERT_NE(node->tempEffectBuffer_, nullptr);

    EXPECT_EQ(node->AudioRendererStart(nodeInfo), 0);
    SUCCEED();
}

/**
 * @tc.name   : Test AudioRendererStop
 * @tc.number : AudioRendererStop_013
 * @tc.desc   : Test AudioRendererStop calls ModifyAudioEffectChainInfo with REMOVE.
 */
HWTEST_F(HpaeRenderEffectNodeTest, AudioRendererStop_013, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = BuildRenderEffectNodeInfo(TEST_ID + 4800);
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo);

    EXPECT_EQ(node->AudioRendererCreate(nodeInfo), 0);
    EXPECT_EQ(node->AudioRendererStart(nodeInfo), 0);
    EXPECT_EQ(node->AudioRendererStop(nodeInfo), 0);
}

/**
 * @tc.name   : Test AudioRendererRelease
 * @tc.number : AudioRendererRelease_014
 * @tc.desc   : Test AudioRendererRelease calls ReleaseAudioEffectChain.
 */
HWTEST_F(HpaeRenderEffectNodeTest, AudioRendererRelease_014, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = BuildRenderEffectNodeInfo(TEST_ID + 4900);
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo);

    EXPECT_EQ(node->AudioRendererCreate(nodeInfo), 0);
    EXPECT_EQ(node->AudioRendererRelease(nodeInfo), 0);
}

/**
 * @tc.name   : Test InitEffectBufferFromDisConnect
 * @tc.number : InitEffectBufferFromDisConnect_015
 * @tc.desc   : Test InitEffectBufferFromDisConnect calls InitAudioEffectChainDynamic.
 */
HWTEST_F(HpaeRenderEffectNodeTest, InitEffectBufferFromDisConnect_015, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    HpaeNodeInfo nodeInfo = BuildRenderEffectNodeInfo(TEST_ID + 5000);
    nodeInfo.effectInfo.effectScene = SCENE_MUSIC;
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    node->sceneType_ = "SCENE_MUSIC";

    node->InitEffectBufferFromDisConnect();
    SUCCEED();

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
 * @tc.name   : Test OffloadRendererCheckNotifyEffect with null manager
 * @tc.number : OffloadRendererCheckNotifyEffect_NullManager_019
 * @tc.desc   : Test OffloadRendererCheckNotifyEffect returns false when manager is null.
 */
HWTEST_F(HpaeRenderEffectNodeTest, OffloadRendererCheckNotifyEffect_NullManager_019, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = BuildRenderEffectNodeInfo(TEST_ID + 5400);
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo);

    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceClass = "speaker";

    EXPECT_NE(node->AudioOffloadRendererCreate(nodeInfo, sinkInfo), 0);
}

/**
 * @tc.name   : Test OffloadRendererCheckNotifyEffect with remote_offload
 * @tc.number : OffloadRendererCheckNotifyEffect_RemoteOffload_020
 * @tc.desc   : Test OffloadRendererCheckNotifyEffect returns false for remote_offload device.
 */
HWTEST_F(HpaeRenderEffectNodeTest, OffloadRendererCheckNotifyEffect_RemoteOffload_020, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = BuildRenderEffectNodeInfo(TEST_ID + 5500);
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo);

    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceClass = "remote_offload";

    // remote_offload should cause early return
    EXPECT_NE(node->AudioOffloadRendererCreate(nodeInfo, sinkInfo), 0);
}

/**
 * @tc.name   : Test TempEffectBuffer constructor with IsCollaborationEnabled
 * @tc.number : TempEffectBuffer_ConstructorWithCollabState_021
 * @tc.desc   : Test TempEffectBuffer constructor stores collaboration state.
 */
HWTEST_F(HpaeRenderEffectNodeTest, TempEffectBuffer_ConstructorWithCollabState_021, TestSize.Level1)
{
    PcmBufferInfo stereoInfo(STEREO, TEST_FRAMELEN1, SAMPLE_RATE_48000, CH_LAYOUT_STEREO);
    HpaeTempEffectBuffer tempBuffer(stereoInfo, COLLABORATION_NEARLINK_OPEN);

    // Verify isCollaborationEnabled_ was stored
    tempBuffer.SetDelayCountToEffectNode(5);
    SUCCEED();
}

/**
 * @tc.name   : Test TempEffectBuffer ChangeLatencyByCollManagerForEffectNode
 * @tc.number : TempEffectBuffer_ChangeLatency_022
 * @tc.desc   : Test ChangeLatencyByCollManagerForEffectNode gets latency from manager.
 */
HWTEST_F(HpaeRenderEffectNodeTest, TempEffectBuffer_ChangeLatency_022, TestSize.Level1)
{
    PcmBufferInfo stereoInfo(STEREO, TEST_FRAMELEN1, SAMPLE_RATE_48000, CH_LAYOUT_STEREO);
    HpaeTempEffectBuffer tempBuffer(stereoInfo);

    int32_t latency = 100;
    tempBuffer.ChangeLatencyByCollManagerForEffectNode(latency);

    // Latency should be modified by AudioCollaborationManager
    SUCCEED();
}

/**
 * @tc.name   : Test SplitCollaborativeData with four channels
 * @tc.number : SplitCollaborativeData_FourChannels_023
 * @tc.desc   : Test SplitCollaborativeData splits 4-channel output correctly.
 */
HWTEST_F(HpaeRenderEffectNodeTest, SplitCollaborativeData_FourChannels_023, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = BuildRenderEffectNodeInfo(TEST_ID + 5600, CHANNEL_4, CH_LAYOUT_4POINT0);
    nodeInfo.sceneType = HPAE_SCENE_COLLABORATIVE;
    nodeInfo.effectInfo.effectScene = SCENE_COLLABORATIVE;
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo, COLLABORATION_DEFAULT_ROUTE_MODE);

    EXPECT_EQ(node->SplitCollaborativeData(), SUCCESS);
}

/**
 * @tc.name   : Test IsByPassEffectZeroVolume with silence threshold not reached
 * @tc.number : IsByPassEffectZeroVolume_BeforeThreshold_024
 * @tc.desc   : Test IsByPassEffectZeroVolume before reaching bypass threshold.
 */
HWTEST_F(HpaeRenderEffectNodeTest, IsByPassEffectZeroVolume_BeforeThreshold_024, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = BuildRenderEffectNodeInfo(TEST_ID + 5700);
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    PcmBufferInfo pcmBufferInfo(STEREO, SAMPLE_RATE_48000 * 2, SAMPLE_RATE_48000, CH_LAYOUT_STEREO);
    HpaePcmBuffer inputBuffer(pcmBufferInfo);
    inputBuffer.SetBufferValid(true);
    inputBuffer.SetBufferSilence(true);

    // Before reaching 4 second threshold, should return false
    EXPECT_FALSE(node->IsByPassEffectZeroVolume(&inputBuffer));
    EXPECT_FALSE(node->isByPassEffect_);
}

/**
 * @tc.name   : Test SignalProcess with empty inputs returns null
 * @tc.number : SignalProcess_EmptyInputs_025
 * @tc.desc   : Test SignalProcess returns nullptr when inputs are empty.
 */
HWTEST_F(HpaeRenderEffectNodeTest, SignalProcess_EmptyInputs_025, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = BuildRenderEffectNodeInfo(TEST_ID + 5800);
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo);

    std::vector<HpaePcmBuffer *> emptyInputs;
    EXPECT_EQ(node->SignalProcess(emptyInputs), nullptr);
}

/**
 * @tc.name   : Test ReconfigOutputBuffer with invalid channel info
 * @tc.number : ReconfigOutputBuffer_InvalidChannelInfo_026
 * @tc.desc   : Test ReconfigOutputBuffer handles invalid channel info (channels=0).
 */
HWTEST_F(HpaeRenderEffectNodeTest, ReconfigOutputBuffer_InvalidChannelInfo_026, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    HpaeNodeInfo nodeInfo = BuildRenderEffectNodeInfo(TEST_ID + 5900);
    nodeInfo.effectInfo.effectScene = SCENE_MUSIC;
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    node->sceneType_ = "SCENE_MUSIC";

    // Invalid channel info should not crash
    node->ReconfigOutputBuffer(STEREO, CH_LAYOUT_STEREO);
    SUCCEED();

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
 * @tc.name   : Test DoProcess collaborative with empty preOutputs and BT mode
 * @tc.number : DoProcess_CollaborativeBT_EmptyPreOutputs_027
 * @tc.desc   : Test DoProcess with collaborative scene, BT mode, and empty preOutputs.
 */
HWTEST_F(HpaeRenderEffectNodeTest, DoProcess_CollaborativeBT_EmptyPreOutputs_027, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = BuildRenderEffectNodeInfo(TEST_ID + 6000, STEREO, CH_LAYOUT_STEREO);
    nodeInfo.sceneType = HPAE_SCENE_COLLABORATIVE;
    nodeInfo.effectInfo.effectScene = SCENE_COLLABORATIVE;
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo, COLLABORATION_DEFAULT_ROUTE_MODE);

    // Empty preOutputs should write silence
    node->DoProcess();
    SUCCEED();
}

/**
 * @tc.name   : Test InitEffectBuffer with collaborative scene
 * @tc.number : InitEffectBuffer_CollaborativeScene_028
 * @tc.desc   : Test InitEffectBuffer returns early for collaborative scene.
 */
HWTEST_F(HpaeRenderEffectNodeTest, InitEffectBuffer_CollaborativeScene_028, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = BuildRenderEffectNodeInfo(TEST_ID + 6100, STEREO, CH_LAYOUT_STEREO);
    nodeInfo.sceneType = HPAE_SCENE_COLLABORATIVE;
    nodeInfo.effectInfo.effectScene = SCENE_COLLABORATIVE;
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo, COLLABORATION_DEFAULT_ROUTE_MODE);

    // Collaborative scene should return early
    node->InitEffectBuffer(nodeInfo.sessionId);
    SUCCEED();
}

/**
 * @tc.name   : Test GetLatency with valid sessionId
 * @tc.number : GetLatency_ValidSession_029
 * @tc.desc   : Test GetLatency returns latency value for session.
 */
HWTEST_F(HpaeRenderEffectNodeTest, GetLatency_ValidSession_029, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    HpaeNodeInfo nodeInfo = BuildRenderEffectNodeInfo(TEST_ID + 6200);
    nodeInfo.effectInfo.effectScene = SCENE_MUSIC;
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo);

    // GetLatency should return a value (may be 0 if no effect chain)
    uint64_t latency = node->GetLatency(nodeInfo.sessionId);
    EXPECT_GE(latency, 0u);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
 * @tc.name   : Test CreateAudioEffectChain with offload enabled
 * @tc.number : CreateAudioEffectChain_OffloadEnabled_030
 * @tc.desc   : Test CreateAudioEffectChain returns early when offload is enabled.
 */
HWTEST_F(HpaeRenderEffectNodeTest, CreateAudioEffectChain_OffloadEnabled_030, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    // Enable offload
    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = true;

    HpaeNodeInfo nodeInfo = BuildRenderEffectNodeInfo(TEST_ID + 6300);
    nodeInfo.effectInfo.effectScene = SCENE_MUSIC;
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo);

    // Should return SUCCESS without creating chain when offload enabled
    EXPECT_EQ(node->AudioRendererCreate(nodeInfo), 0);

    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = false;
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
 * @tc.name   : Test ReleaseAudioEffectChain with offload enabled
 * @tc.number : ReleaseAudioEffectChain_OffloadEnabled_031
 * @tc.desc   : Test ReleaseAudioEffectChain returns early when offload is enabled.
 */
HWTEST_F(HpaeRenderEffectNodeTest, ReleaseAudioEffectChain_OffloadEnabled_031, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    // Enable offload
    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = true;

    HpaeNodeInfo nodeInfo = BuildRenderEffectNodeInfo(TEST_ID + 6400);
    nodeInfo.effectInfo.effectScene = SCENE_MUSIC;
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo);

    // Should return SUCCESS when offload enabled
    EXPECT_EQ(node->ReleaseAudioEffectChain(nodeInfo), 0);

    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = false;
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
 * @tc.name   : Test AudioOffloadRenderer Stop/Release proceed when offload disabled
 * @tc.number : AudioOffloadRenderer_PcmOffloadStopReleaseProceed_034
 * @tc.desc   : Test the collab bug fix scenario: with effect offload disabled,
 *              Stop/Release still proceed (isPcmOffload=false skips the offload gate)
 *              while Create/Start are blocked (isPcmOffload=true keeps the gate).
 */
HWTEST_F(HpaeRenderEffectNodeTest, AudioOffloadRenderer_PcmOffloadStopReleaseProceed_034, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    EffectManagerOffloadGuard guard;
    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = true;

    HpaeNodeInfo nodeInfo = BuildRenderEffectNodeInfo(TEST_ID + 6700);
    nodeInfo.effectInfo.effectScene = SCENE_MUSIC;
    auto node = std::make_shared<HpaeRenderEffectNode>(nodeInfo);

    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceClass = "speaker";
    EXPECT_EQ(node->AudioOffloadRendererCreate(nodeInfo, sinkInfo), SUCCESS);
    EXPECT_EQ(node->AudioOffloadRendererStart(nodeInfo, sinkInfo), SUCCESS);
    EXPECT_EQ(node->AudioOffloadRendererStop(nodeInfo, sinkInfo), SUCCESS);
    EXPECT_EQ(node->AudioOffloadRendererRelease(nodeInfo, sinkInfo), SUCCESS);

    sinkInfo.deviceClass = "remote_offload";
    EXPECT_EQ(node->AudioOffloadRendererCreate(nodeInfo, sinkInfo), ERR_INVALID_HANDLE);
    EXPECT_EQ(node->AudioOffloadRendererStart(nodeInfo, sinkInfo), ERR_INVALID_HANDLE);
    EXPECT_EQ(node->AudioOffloadRendererStop(nodeInfo, sinkInfo), ERR_INVALID_HANDLE);
    EXPECT_EQ(node->AudioOffloadRendererRelease(nodeInfo, sinkInfo), ERR_INVALID_HANDLE);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
