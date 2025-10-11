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

HWTEST_F(HpaeRenderEffectNodeTest, testCreate_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceClass = "remote_offload";
    std::shared_ptr<HpaeRenderEffectNode> hpaeRenderEffectNode = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    nodeInfo.effectInfo.effectScene = (AudioEffectScene)0xff;
    EXPECT_EQ(hpaeRenderEffectNode->AudioRendererCreate(nodeInfo), 0);
    EXPECT_EQ(hpaeRenderEffectNode->AudioOffloadRendererCreate(nodeInfo, sinkInfo), 0);
    EXPECT_NE(hpaeRenderEffectNode->ReleaseAudioEffectChain(nodeInfo), 0);
}

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