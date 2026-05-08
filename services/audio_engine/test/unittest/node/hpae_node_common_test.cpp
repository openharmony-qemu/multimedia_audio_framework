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
#include "audio_errors.h"
#include "hpae_node_common.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

std::string DEFAULT_NODE_STRING_KEY = "1_48000_2_1_4"; // _4 is channelLayout mono
std::string DEFAULT_SCENE_MUSIC = "SCENE_MUSIC";
std::string DEFAULT_SCENE_EXTRA = "SCENE_EXTRA";
constexpr size_t DEFAULT_LEN = 100;
constexpr size_t DEFAULT_US_TIME = 100000;
constexpr uint64_t DEFAULT_BUFFER_SIZE = 100;
constexpr uint64_t DEFAULT_CONVERTER_US_TIME = 520;
constexpr size_t DEFAULT_FRAME_COUNT = 48;

class HpaeNodeCommonTest : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;
};

void HpaeNodeCommonTest::SetUp()
{}

void HpaeNodeCommonTest::TearDown()
{}

static HpaeNodeInfo GetTestNodeInfo()
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.channels = STEREO;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channelLayout = CH_LAYOUT_MONO;
    nodeInfo.format = SAMPLE_S16LE;
    nodeInfo.frameLen = DEFAULT_LEN;
    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MIC;
    return nodeInfo;
}

HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo1 = GetTestNodeInfo();
    HpaeNodeInfo nodeInfo2 = GetTestNodeInfo();
    EXPECT_EQ(CheckHpaeNodeInfoIsSame(nodeInfo1, nodeInfo2), true);
    nodeInfo2.channels = MONO;
    EXPECT_EQ(CheckHpaeNodeInfoIsSame(nodeInfo1, nodeInfo2), false);
}

HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_002, TestSize.Level0)
{
    EXPECT_EQ(TransStreamTypeToSceneType(STREAM_MUSIC), HPAE_SCENE_MUSIC);
    EXPECT_EQ(TransStreamTypeToSceneType(STREAM_RECORDING), HPAE_SCENE_EFFECT_NONE);

    EXPECT_EQ(TransSourceTypeToSceneType(SOURCE_TYPE_MIC), HPAE_SCENE_RECORD);
    EXPECT_EQ(TransSourceTypeToSceneType(SOURCE_TYPE_INVALID), HPAE_SCENE_EFFECT_NONE);

    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    EXPECT_EQ(TransNodeInfoToStringKey(nodeInfo), DEFAULT_NODE_STRING_KEY);

    EXPECT_EQ(TransProcessType2EnhanceScene(HPAE_SCENE_RECORD), SCENE_RECORD);
    EXPECT_EQ(TransProcessType2EnhanceScene(HPAE_SCENE_MUSIC), SCENE_NONE);

    EXPECT_EQ(TransProcessorTypeToSceneType(HPAE_SCENE_MUSIC), DEFAULT_SCENE_MUSIC);
    EXPECT_EQ(TransProcessorTypeToSceneType(HPAE_SCENE_RECORD), DEFAULT_SCENE_EXTRA);
}

HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_003, TestSize.Level0)
{
    EXPECT_EQ(CheckSceneTypeNeedEc(HPAE_SCENE_VOIP_UP), true);
    EXPECT_EQ(CheckSceneTypeNeedMicRef(HPAE_SCENE_VOIP_UP), true);
}

HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_004, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    EXPECT_EQ(ConvertDatalenToUs(DEFAULT_BUFFER_SIZE, nodeInfo), DEFAULT_CONVERTER_US_TIME);
    EXPECT_EQ(ConvertUsToFrameCount(DEFAULT_US_TIME, nodeInfo), DEFAULT_FRAME_COUNT);
}

/**
 * @tc.name: TestHpaeNodeCommon_005
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_005
 * @tc.desc: Test TransStreamTypeToSceneType with valid music and game stream types
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_005, TestSize.Level0)
{
    EXPECT_EQ(TransStreamTypeToSceneType(STREAM_MUSIC), HPAE_SCENE_MUSIC);
    EXPECT_EQ(TransStreamTypeToSceneType(STREAM_GAME), HPAE_SCENE_GAME);
    EXPECT_EQ(TransStreamTypeToSceneType(STREAM_MOVIE), HPAE_SCENE_MOVIE);
}

/**
 * @tc.name: TestHpaeNodeCommon_006
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_006
 * @tc.desc: Test TransStreamTypeToSceneType with valid speech, ring and communication stream types
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_006, TestSize.Level0)
{
    EXPECT_EQ(TransStreamTypeToSceneType(STREAM_SPEECH), HPAE_SCENE_SPEECH);
    EXPECT_EQ(TransStreamTypeToSceneType(STREAM_VOICE_RING), HPAE_SCENE_RING);
    EXPECT_EQ(TransStreamTypeToSceneType(STREAM_VOICE_COMMUNICATION), HPAE_SCENE_VOIP_DOWN);
    EXPECT_EQ(TransStreamTypeToSceneType(STREAM_MEDIA), HPAE_SCENE_OTHERS);
}

/**
 * @tc.name: TestHpaeNodeCommon_007
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_007
 * @tc.desc: Test TransStreamTypeToSceneType with invalid stream type
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_007, TestSize.Level0)
{
    AudioStreamType invalidStreamType = static_cast<AudioStreamType>(999);
    HpaeProcessorType result = TransStreamTypeToSceneType(invalidStreamType);
    EXPECT_EQ(result, HPAE_SCENE_EFFECT_NONE);
}

/**
 * @tc.name: TestHpaeNodeCommon_008
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_008
 * @tc.desc: Test TransEffectSceneToSceneType with valid music, movie and game effect scenes
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_008, TestSize.Level0)
{
    EXPECT_EQ(TransEffectSceneToSceneType(SCENE_OTHERS), HPAE_SCENE_OTHERS);
    EXPECT_EQ(TransEffectSceneToSceneType(SCENE_MUSIC), HPAE_SCENE_MUSIC);
    EXPECT_EQ(TransEffectSceneToSceneType(SCENE_MOVIE), HPAE_SCENE_MOVIE);
    EXPECT_EQ(TransEffectSceneToSceneType(SCENE_GAME), HPAE_SCENE_GAME);
}

/**
 * @tc.name: TestHpaeNodeCommon_009
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_009
 * @tc.desc: Test TransEffectSceneToSceneType with valid speech, ring and voip effect scenes
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_009, TestSize.Level0)
{
    EXPECT_EQ(TransEffectSceneToSceneType(SCENE_SPEECH), HPAE_SCENE_SPEECH);
    EXPECT_EQ(TransEffectSceneToSceneType(SCENE_RING), HPAE_SCENE_RING);
    EXPECT_EQ(TransEffectSceneToSceneType(SCENE_VOIP_DOWN), HPAE_SCENE_VOIP_DOWN);
    EXPECT_EQ(TransEffectSceneToSceneType(SCENE_COLLABORATIVE), HPAE_SCENE_COLLABORATIVE);
}

/**
 * @tc.name: TestHpaeNodeCommon_010
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_010
 * @tc.desc: Test TransEffectSceneToSceneType with invalid effect scene
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_010, TestSize.Level0)
{
    AudioEffectScene invalidEffectScene = static_cast<AudioEffectScene>(999);
    HpaeProcessorType result = TransEffectSceneToSceneType(invalidEffectScene);
    EXPECT_EQ(result, HPAE_SCENE_EFFECT_NONE);
}

/**
 * @tc.name: TestHpaeNodeCommon_011
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_011
 * @tc.desc: Test TransSourceTypeToSceneType with valid mic and camcorder source types
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_011, TestSize.Level0)
{
    EXPECT_EQ(TransSourceTypeToSceneType(SOURCE_TYPE_MIC), HPAE_SCENE_RECORD);
    EXPECT_EQ(TransSourceTypeToSceneType(SOURCE_TYPE_CAMCORDER), HPAE_SCENE_RECORD);
    EXPECT_EQ(TransSourceTypeToSceneType(SOURCE_TYPE_VOICE_CALL), HPAE_SCENE_VOIP_UP);
    EXPECT_EQ(TransSourceTypeToSceneType(SOURCE_TYPE_VOICE_COMMUNICATION), HPAE_SCENE_VOIP_UP);
}

/**
 * @tc.name: TestHpaeNodeCommon_012
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_012
 * @tc.desc: Test TransSourceTypeToSceneType with valid transcription, message and recognition source types
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_012, TestSize.Level0)
{
    EXPECT_EQ(TransSourceTypeToSceneType(SOURCE_TYPE_VOICE_TRANSCRIPTION), HPAE_SCENE_PRE_ENHANCE);
    EXPECT_EQ(TransSourceTypeToSceneType(SOURCE_TYPE_VOICE_MESSAGE), HPAE_SCENE_VOICE_MESSAGE);
    EXPECT_EQ(TransSourceTypeToSceneType(SOURCE_TYPE_VOICE_RECOGNITION), HPAE_SCENE_RECOGNITION);
}

/**
 * @tc.name: TestHpaeNodeCommon_013
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_013
 * @tc.desc: Test TransSourceTypeToSceneType with invalid source type
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_013, TestSize.Level0)
{
    SourceType invalidSourceType = SOURCE_TYPE_INVALID;
    HpaeProcessorType result = TransSourceTypeToSceneType(invalidSourceType);
    EXPECT_EQ(result, HPAE_SCENE_EFFECT_NONE);
}

/**
 * @tc.name: TestHpaeNodeCommon_014
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_014
 * @tc.desc: Test CheckSceneTypeNeedEc with valid scene types that need EC
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_014, TestSize.Level0)
{
    EXPECT_TRUE(CheckSceneTypeNeedEc(HPAE_SCENE_VOIP_UP));
    EXPECT_TRUE(CheckSceneTypeNeedEc(HPAE_SCENE_PRE_ENHANCE));
    EXPECT_TRUE(CheckSceneTypeNeedEc(HPAE_SCENE_RECOGNITION));
}

/**
 * @tc.name: TestHpaeNodeCommon_015
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_015
 * @tc.desc: Test CheckSceneTypeNeedEc with invalid scene types that don't need EC
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_015, TestSize.Level0)
{
    EXPECT_FALSE(CheckSceneTypeNeedEc(HPAE_SCENE_MUSIC));
    EXPECT_FALSE(CheckSceneTypeNeedEc(HPAE_SCENE_RECORD));
    EXPECT_FALSE(CheckSceneTypeNeedEc(HPAE_SCENE_GAME));
    EXPECT_FALSE(CheckSceneTypeNeedEc(HPAE_SCENE_MOVIE));
}

/**
 * @tc.name: TestHpaeNodeCommon_016
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_016
 * @tc.desc: Test CheckSceneTypeNeedMicRef with valid scene types that need mic reference
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_016, TestSize.Level0)
{
    EXPECT_TRUE(CheckSceneTypeNeedMicRef(HPAE_SCENE_VOIP_UP));
    EXPECT_TRUE(CheckSceneTypeNeedMicRef(HPAE_SCENE_RECORD));
}

/**
 * @tc.name: TestHpaeNodeCommon_017
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_017
 * @tc.desc: Test CheckSceneTypeNeedMicRef with invalid scene types that don't need mic reference
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_017, TestSize.Level0)
{
    EXPECT_FALSE(CheckSceneTypeNeedMicRef(HPAE_SCENE_MUSIC));
    EXPECT_FALSE(CheckSceneTypeNeedMicRef(HPAE_SCENE_PRE_ENHANCE));
    EXPECT_FALSE(CheckSceneTypeNeedMicRef(HPAE_SCENE_RECOGNITION));
    EXPECT_FALSE(CheckSceneTypeNeedMicRef(HPAE_SCENE_VOICE_MESSAGE));
}

/**
 * @tc.name: TestHpaeNodeCommon_018
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_018
 * @tc.desc: Test TransProcessorTypeToSceneType with valid default, music and game processor types
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_018, TestSize.Level0)
{
    EXPECT_EQ(TransProcessorTypeToSceneType(HPAE_SCENE_DEFAULT), "HPAE_SCENE_DEFAULT");
    EXPECT_EQ(TransProcessorTypeToSceneType(HPAE_SCENE_OTHERS), "SCENE_OTHERS");
    EXPECT_EQ(TransProcessorTypeToSceneType(HPAE_SCENE_MUSIC), "SCENE_MUSIC");
    EXPECT_EQ(TransProcessorTypeToSceneType(HPAE_SCENE_GAME), "SCENE_GAME");
}

/**
 * @tc.name: TestHpaeNodeCommon_019
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_019
 * @tc.desc: Test TransProcessorTypeToSceneType with valid movie, speech, ring and collaborative processor types
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_019, TestSize.Level0)
{
    EXPECT_EQ(TransProcessorTypeToSceneType(HPAE_SCENE_MOVIE), "SCENE_MOVIE");
    EXPECT_EQ(TransProcessorTypeToSceneType(HPAE_SCENE_SPEECH), "SCENE_SPEECH");
    EXPECT_EQ(TransProcessorTypeToSceneType(HPAE_SCENE_RING), "SCENE_RING");
    EXPECT_EQ(TransProcessorTypeToSceneType(HPAE_SCENE_VOIP_DOWN), "SCENE_VOIP_DOWN");
    EXPECT_EQ(TransProcessorTypeToSceneType(HPAE_SCENE_COLLABORATIVE), "SCENE_COLLABORATIVE");
}

/**
 * @tc.name: TestHpaeNodeCommon_020
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_020
 * @tc.desc: Test TransProcessorTypeToSceneType with invalid processor type
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_020, TestSize.Level0)
{
    HpaeProcessorType invalidProcessorType = static_cast<HpaeProcessorType>(999);
    std::string result = TransProcessorTypeToSceneType(invalidProcessorType);
    EXPECT_EQ(result, "SCENE_EXTRA");
}

/**
 * @tc.name: TestHpaeNodeCommon_021
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_021
 * @tc.desc: Test TransProcessType2EnhanceScene with valid record, voip and pre-enhance processor types
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_021, TestSize.Level0)
{
    EXPECT_EQ(TransProcessType2EnhanceScene(HPAE_SCENE_RECORD), SCENE_RECORD);
    EXPECT_EQ(TransProcessType2EnhanceScene(HPAE_SCENE_VOIP_UP), SCENE_VOIP_UP);
    EXPECT_EQ(TransProcessType2EnhanceScene(HPAE_SCENE_PRE_ENHANCE), SCENE_PRE_ENHANCE);
}

/**
 * @tc.name: TestHpaeNodeCommon_022
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_022
 * @tc.desc: Test TransProcessType2EnhanceScene with valid voice message and recognition processor types
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_022, TestSize.Level0)
{
    EXPECT_EQ(TransProcessType2EnhanceScene(HPAE_SCENE_VOICE_MESSAGE), SCENE_VOICE_MESSAGE);
    EXPECT_EQ(TransProcessType2EnhanceScene(HPAE_SCENE_RECOGNITION), SCENE_RECOGNITION);
}

/**
 * @tc.name: TestHpaeNodeCommon_023
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_023
 * @tc.desc: Test TransProcessType2EnhanceScene with invalid processor types
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_023, TestSize.Level0)
{
    EXPECT_EQ(TransProcessType2EnhanceScene(HPAE_SCENE_MUSIC), SCENE_NONE);
    EXPECT_EQ(TransProcessType2EnhanceScene(HPAE_SCENE_GAME), SCENE_NONE);
    EXPECT_EQ(TransProcessType2EnhanceScene(HPAE_SCENE_MOVIE), SCENE_NONE);

    HpaeProcessorType invalidProcessorType = static_cast<HpaeProcessorType>(999);
    AudioEnhanceScene result = TransProcessType2EnhanceScene(invalidProcessorType);
    EXPECT_EQ(result, SCENE_NONE);
}

/**
 * @tc.name: TestHpaeNodeCommon_023b
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_023b
 * @tc.desc: Test TransProcessType2EnhanceScene with HPAE_SCENE_COLLABORATIVE and collaborative related mappings
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_023b, TestSize.Level0)
{
    // collaborative scene mapping
    EXPECT_EQ(TransProcessType2EnhanceScene(HPAE_SCENE_COLLABORATIVE_RECORD), SCENE_COLLABORATIVE_RECORD);

    // verify TransEffectSceneToSceneType round-trip for collaborative
    EXPECT_EQ(TransEffectSceneToSceneType(SCENE_COLLABORATIVE), HPAE_SCENE_COLLABORATIVE);

    // verify TransProcessorTypeToSceneType round-trip for collaborative
    EXPECT_EQ(TransProcessorTypeToSceneType(HPAE_SCENE_COLLABORATIVE), "SCENE_COLLABORATIVE");
}

/**
 * @tc.name: TestHpaeNodeCommon_024
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_024
 * @tc.desc: Test ConvertSessionState2Str with valid new, prepared and running session states
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_024, TestSize.Level0)
{
    EXPECT_EQ(ConvertSessionState2Str(HPAE_SESSION_NEW), "NEW");
    EXPECT_EQ(ConvertSessionState2Str(HPAE_SESSION_PREPARED), "PREPARED");
    EXPECT_EQ(ConvertSessionState2Str(HPAE_SESSION_RUNNING), "RUNNING");
    EXPECT_EQ(ConvertSessionState2Str(HPAE_SESSION_PAUSING), "PAUSING");
}

/**
 * @tc.name: TestHpaeNodeCommon_025
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_025
 * @tc.desc: Test ConvertSessionState2Str with valid paused, stopping, stopped and released session states
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_025, TestSize.Level0)
{
    EXPECT_EQ(ConvertSessionState2Str(HPAE_SESSION_PAUSED), "PAUSED");
    EXPECT_EQ(ConvertSessionState2Str(HPAE_SESSION_STOPPING), "STOPPING");
    EXPECT_EQ(ConvertSessionState2Str(HPAE_SESSION_STOPPED), "STOPPED");
    EXPECT_EQ(ConvertSessionState2Str(HPAE_SESSION_RELEASED), "RELEASED");
}

/**
 * @tc.name: TestHpaeNodeCommon_026
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_026
 * @tc.desc: Test ConvertSessionState2Str with invalid session state
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_026, TestSize.Level0)
{
    HpaeSessionState invalidState = static_cast<HpaeSessionState>(999);
    std::string result = ConvertSessionState2Str(invalidState);
    EXPECT_EQ(result, "UNKNOWN");
}

/**
 * @tc.name: TestHpaeNodeCommon_027
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_027
 * @tc.desc: Test ConvertStreamManagerState2Str with valid new, idle and running stream manager states
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_027, TestSize.Level0)
{
    EXPECT_EQ(ConvertStreamManagerState2Str(STREAM_MANAGER_NEW), "NEW");
    EXPECT_EQ(ConvertStreamManagerState2Str(STREAM_MANAGER_IDLE), "IDLE");
    EXPECT_EQ(ConvertStreamManagerState2Str(STREAM_MANAGER_RUNNING), "RUNNING");
}

/**
 * @tc.name: TestHpaeNodeCommon_028
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_028
 * @tc.desc: Test ConvertStreamManagerState2Str with valid suspended and released stream manager states
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_028, TestSize.Level0)
{
    EXPECT_EQ(ConvertStreamManagerState2Str(STREAM_MANAGER_SUSPENDED), "SUSPENDED");
    EXPECT_EQ(ConvertStreamManagerState2Str(STREAM_MANAGER_RELEASED), "RELEASED");
}

/**
 * @tc.name: TestHpaeNodeCommon_029
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_029
 * @tc.desc: Test ConvertStreamManagerState2Str with invalid stream manager state
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_029, TestSize.Level0)
{
    StreamManagerState invalidState = static_cast<StreamManagerState>(999);
    std::string result = ConvertStreamManagerState2Str(invalidState);
    EXPECT_EQ(result, "UNKNOWN");
}

/**
 * @tc.name: TestHpaeNodeCommon_030
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_030
 * @tc.desc: Test ConvertDeviceClassToPipe with valid primary, a2dp and remote device classes
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_030, TestSize.Level0)
{
    EXPECT_EQ(ConvertDeviceClassToPipe("primary"), PIPE_TYPE_OUT_NORMAL);
    EXPECT_EQ(ConvertDeviceClassToPipe("a2dp"), PIPE_TYPE_OUT_NORMAL);
    EXPECT_EQ(ConvertDeviceClassToPipe("remote"), PIPE_TYPE_OUT_NORMAL);
}

/**
 * @tc.name: TestHpaeNodeCommon_031
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_031
 * @tc.desc: Test ConvertDeviceClassToPipe with valid dp and multichannel device classes
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_031, TestSize.Level0)
{
    EXPECT_EQ(ConvertDeviceClassToPipe("dp"), PIPE_TYPE_OUT_NORMAL);
    EXPECT_EQ(ConvertDeviceClassToPipe("multichannel"), PIPE_TYPE_OUT_MULTICHANNEL);
}

/**
 * @tc.name: TestHpaeNodeCommon_032
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_032
 * @tc.desc: Test ConvertDeviceClassToPipe with invalid device classes
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_032, TestSize.Level0)
{
    EXPECT_EQ(ConvertDeviceClassToPipe("invalid"), PIPE_TYPE_UNKNOWN);
    EXPECT_EQ(ConvertDeviceClassToPipe(""), PIPE_TYPE_UNKNOWN);
    EXPECT_EQ(ConvertDeviceClassToPipe("unknown"), PIPE_TYPE_UNKNOWN);
}

/**
 * @tc.name: TestHpaeNodeCommon_033
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_033
 * @tc.desc: Test CheckHpaeNodeInfoIsSame with identical node info
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_033, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo1 = GetTestNodeInfo();
    HpaeNodeInfo nodeInfo2 = nodeInfo1;
    
    bool result = CheckHpaeNodeInfoIsSame(nodeInfo1, nodeInfo2);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: TestHpaeNodeCommon_034
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_034
 * @tc.desc: Test CheckHpaeNodeInfoIsSame with different channels and sampling rate
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_034, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo1 = GetTestNodeInfo();
    HpaeNodeInfo nodeInfo2 = nodeInfo1;
    nodeInfo2.channels = MONO;
    
    bool result = CheckHpaeNodeInfoIsSame(nodeInfo1, nodeInfo2);
    EXPECT_FALSE(result);
    
    HpaeNodeInfo nodeInfo3 = nodeInfo1;
    nodeInfo3.samplingRate = SAMPLE_RATE_44100;
    result = CheckHpaeNodeInfoIsSame(nodeInfo1, nodeInfo3);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: TestHpaeNodeCommon_035
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_035
 * @tc.desc: Test CheckHpaeNodeInfoIsSame with different channel layout
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_035, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo1 = GetTestNodeInfo();
    HpaeNodeInfo nodeInfo2 = nodeInfo1;
    nodeInfo2.channelLayout = CH_LAYOUT_UNKNOWN;
    
    bool result = CheckHpaeNodeInfoIsSame(nodeInfo1, nodeInfo2);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: TestHpaeNodeCommon_036
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_036
 * @tc.desc: Test TransNodeInfoToStringKey with different node info parameters
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_036, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MIC;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S16LE;
    nodeInfo.channelLayout = CH_LAYOUT_MONO;
    
    std::string result = TransNodeInfoToStringKey(nodeInfo);
    
    EXPECT_FALSE(result.empty());
    EXPECT_NE(result.find("1"), std::string::npos);
    EXPECT_NE(result.find("48000"), std::string::npos);
    EXPECT_NE(result.find("2"), std::string::npos);
}

/**
 * @tc.name: TestHpaeNodeCommon_037
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_037
 * @tc.desc: Test TransNodeInfoToStringKey with zero values
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_037, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_DEFAULT;
    nodeInfo.samplingRate = SAMPLE_RATE_8000;
    nodeInfo.channels = CHANNEL_UNKNOW;
    nodeInfo.format = SAMPLE_S16LE;
    nodeInfo.channelLayout = CH_LAYOUT_UNKNOWN;
    
    std::string result = TransNodeInfoToStringKey(nodeInfo);
    
    EXPECT_FALSE(result.empty());
    EXPECT_NE(result.find("0"), std::string::npos);
}

/**
 * @tc.name: TestHpaeNodeCommon_038
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_038
 * @tc.desc: Test ConvertUsToFrameCount with different time values
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_038, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.frameLen = 1024;
    
    uint64_t testTimeUs1 = 1000000; // 1 second
    size_t result1 = ConvertUsToFrameCount(testTimeUs1, nodeInfo);
    EXPECT_GT(result1, 0U);
    
    uint64_t testTimeUs2 = 500000; // 0.5 second
    size_t result2 = ConvertUsToFrameCount(testTimeUs2, nodeInfo);
    EXPECT_GT(result2, 0U);
    EXPECT_LT(result2, result1);
}

/**
 * @tc.name: TestHpaeNodeCommon_039
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_039
 * @tc.desc: Test ConvertUsToFrameCount with different sample rates
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_039, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo1 = GetTestNodeInfo();
    nodeInfo1.samplingRate = SAMPLE_RATE_48000;
    nodeInfo1.frameLen = 1024;
    
    HpaeNodeInfo nodeInfo2 = GetTestNodeInfo();
    nodeInfo2.samplingRate = SAMPLE_RATE_16000;
    nodeInfo2.frameLen = 1024;
    
    uint64_t testTimeUs = 1000000; // 1 second
    size_t result1 = ConvertUsToFrameCount(testTimeUs, nodeInfo1);
    size_t result2 = ConvertUsToFrameCount(testTimeUs, nodeInfo2);
    
    EXPECT_GT(result1, 0U);
    EXPECT_GT(result2, 0U);
}

/**
 * @tc.name: TestHpaeNodeCommon_040
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_040
 * @tc.desc: Test ConvertDatalenToUs with valid node info parameters
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_040, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S16LE;
    
    size_t bufferSize1 = 4096; // Example buffer size
    uint64_t result1 = ConvertDatalenToUs(bufferSize1, nodeInfo);
    EXPECT_GT(result1, 0U);
    
    size_t bufferSize2 = 8192; // Larger buffer size
    uint64_t result2 = ConvertDatalenToUs(bufferSize2, nodeInfo);
    EXPECT_GT(result2, 0U);
    EXPECT_GT(result2, result1);
}

/**
 * @tc.name: TestHpaeNodeCommon_041
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_041
 * @tc.desc: Test ConvertDatalenToUs with different audio formats
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_041, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo1 = GetTestNodeInfo();
    nodeInfo1.samplingRate = SAMPLE_RATE_48000;
    nodeInfo1.channels = STEREO;
    nodeInfo1.format = SAMPLE_S16LE;
    
    HpaeNodeInfo nodeInfo2 = GetTestNodeInfo();
    nodeInfo2.samplingRate = SAMPLE_RATE_48000;
    nodeInfo2.channels = STEREO;
    nodeInfo2.format = SAMPLE_S32LE;
    
    size_t bufferSize = 4096;
    uint64_t result1 = ConvertDatalenToUs(bufferSize, nodeInfo1);
    uint64_t result2 = ConvertDatalenToUs(bufferSize, nodeInfo2);
    
    EXPECT_GT(result1, 0U);
    EXPECT_GT(result2, 0U);
}

/**
 * @tc.name: TestHpaeNodeCommon_042
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_042
 * @tc.desc: Test ConvertDatalenToUs with invalid node info parameters
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_042, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    size_t bufferSize = 4096;
    
    HpaeNodeInfo invalidNodeInfo1 = nodeInfo;
    invalidNodeInfo1.channels = CHANNEL_UNKNOW;
    uint64_t result1 = ConvertDatalenToUs(bufferSize, invalidNodeInfo1);
    EXPECT_EQ(result1, 0U);
    
    HpaeNodeInfo invalidNodeInfo2 = nodeInfo;
    invalidNodeInfo2.format = INVALID_WIDTH;
    uint64_t result2 = ConvertDatalenToUs(bufferSize, invalidNodeInfo2);
    EXPECT_EQ(result2, 0U);
    
    HpaeNodeInfo invalidNodeInfo3 = nodeInfo;
    invalidNodeInfo3.samplingRate = static_cast<AudioSamplingRate>(0);
    uint64_t result3 = ConvertDatalenToUs(bufferSize, invalidNodeInfo3);
    EXPECT_EQ(result3, 0U);
}

/**
 * @tc.name: TestHpaeNodeCommon_043
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_043
 * @tc.desc: Test TransFormatFromStringToEnum with s16 and s24 formats
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_043, TestSize.Level0)
{
    EXPECT_EQ(TransFormatFromStringToEnum("s16"), SAMPLE_S16LE);
    EXPECT_EQ(TransFormatFromStringToEnum("s16le"), SAMPLE_S16LE);
    EXPECT_EQ(TransFormatFromStringToEnum("s24"), SAMPLE_S24LE);
    EXPECT_EQ(TransFormatFromStringToEnum("s24le"), SAMPLE_S24LE);
}

/**
 * @tc.name: TestHpaeNodeCommon_044
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_044
 * @tc.desc: Test TransFormatFromStringToEnum with s32 and f32 formats
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_044, TestSize.Level0)
{
    EXPECT_EQ(TransFormatFromStringToEnum("s32"), SAMPLE_S32LE);
    EXPECT_EQ(TransFormatFromStringToEnum("s32le"), SAMPLE_S32LE);
    EXPECT_EQ(TransFormatFromStringToEnum("f32"), SAMPLE_F32LE);
    EXPECT_EQ(TransFormatFromStringToEnum("f32le"), SAMPLE_F32LE);
}

/**
 * @tc.name: TestHpaeNodeCommon_045
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_045
 * @tc.desc: Test TransFormatFromEnumToString with S16 and S24 formats
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_045, TestSize.Level0)
{
    EXPECT_EQ(TransFormatFromEnumToString(SAMPLE_S16LE), "s16le");
    EXPECT_EQ(TransFormatFromEnumToString(SAMPLE_S24LE), "s24le");
}

/**
 * @tc.name: TestHpaeNodeCommon_046
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_046
 * @tc.desc: Test TransFormatFromEnumToString with S32 and F32 formats
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_046, TestSize.Level0)
{
    EXPECT_EQ(TransFormatFromEnumToString(SAMPLE_S32LE), "s32le");
    EXPECT_EQ(TransFormatFromEnumToString(SAMPLE_F32LE), "f32le");
}

/**
 * @tc.name: TestHpaeNodeCommon_047
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_047
 * @tc.desc: Test TransFormatFromEnumToString with invalid format
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_047, TestSize.Level0)
{
    AudioSampleFormat invalidFormat = static_cast<AudioSampleFormat>(999);
    std::string result = TransFormatFromEnumToString(invalidFormat);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name: TestHpaeNodeCommon_049
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_049
 * @tc.desc: Test TransNodeInfoForCollaboration enabling collaboration for movie scene
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_049, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    nodeInfo.effectInfo.effectScene = SCENE_MOVIE;
    nodeInfo.sceneType = HPAE_SCENE_MOVIE;
    
    TransNodeInfoForCollaboration(nodeInfo, true);
    EXPECT_EQ(nodeInfo.effectInfo.effectScene, SCENE_COLLABORATIVE);
    EXPECT_EQ(nodeInfo.effectInfo.lastEffectScene, SCENE_MOVIE);
    EXPECT_EQ(nodeInfo.sceneType, HPAE_SCENE_COLLABORATIVE);
}

/**
 * @tc.name: TestHpaeNodeCommon_050
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_050
 * @tc.desc: Test TransNodeInfoForCollaboration disabling collaboration
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_050, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    nodeInfo.effectInfo.effectScene = SCENE_MUSIC;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    
    TransNodeInfoForCollaboration(nodeInfo, true);
    TransNodeInfoForCollaboration(nodeInfo, false);
    EXPECT_EQ(nodeInfo.effectInfo.effectScene, SCENE_MUSIC);
    EXPECT_EQ(nodeInfo.sceneType, HPAE_SCENE_MUSIC);
}

/**
 * @tc.name: TestHpaeNodeCommon_051
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_051
 * @tc.desc: Test RecoverNodeInfoForCollaboration with collaborative scene
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_051, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    nodeInfo.effectInfo.effectScene = SCENE_COLLABORATIVE;
    nodeInfo.effectInfo.lastEffectScene = SCENE_MOVIE;
    
    RecoverNodeInfoForCollaboration(nodeInfo);
    EXPECT_EQ(nodeInfo.effectInfo.effectScene, SCENE_MOVIE);
}

/**
 * @tc.name: TestHpaeNodeCommon_052
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_052
 * @tc.desc: Test CalculateFrameLenBySampleRate with 11025 sample rate
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_052, TestSize.Level0)
{
    size_t result = CalculateFrameLenBySampleRate(SAMPLE_RATE_11025);
    EXPECT_EQ(result, 220U); // 220 for 11025 samplerate (20ms unified)
}

/**
 * @tc.name: TestHpaeNodeCommon_053
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_053
 * @tc.desc: Test CalculateFrameLenBySampleRate with multiples of 50 sample rates
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_053, TestSize.Level0)
{
    size_t result1 = CalculateFrameLenBySampleRate(SAMPLE_RATE_8000);
    size_t result2 = CalculateFrameLenBySampleRate(SAMPLE_RATE_16000);
    size_t result3 = CalculateFrameLenBySampleRate(SAMPLE_RATE_48000);
    
    EXPECT_EQ(result1, 160U);
    EXPECT_EQ(result2, 320U);
    EXPECT_EQ(result3, 960U);
}

/**
 * @tc.name: TestHpaeNodeCommon_054
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_054
 * @tc.desc: Test CalculateFrameLenBySampleRate with non-multiples of 50 sample rates
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_054, TestSize.Level0)
{
    size_t result1 = CalculateFrameLenBySampleRate(SAMPLE_RATE_44100);
    size_t result2 = CalculateFrameLenBySampleRate(SAMPLE_RATE_22050);
    
    EXPECT_EQ(result1, 882U);
    EXPECT_EQ(result2, 441U);
}

/**
 * @tc.name: TestHpaeNodeCommon_055
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_055
 * @tc.desc: Test CalculateFrameLenBySampleRate with uint32_t parameter
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_055, TestSize.Level0)
{
    size_t result1 = CalculateFrameLenBySampleRate(48000U);
    size_t result2 = CalculateFrameLenBySampleRate(44100U);
    
    EXPECT_EQ(result1, 960U);
    EXPECT_EQ(result2, 882U);
}

/**
 * @tc.name: TestHpaeNodeCommon_056
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_056
 * @tc.desc: Test CheckStreamInfo with valid frame length
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_056, TestSize.Level0)
{
    HpaeStreamInfo validStreamInfo;
    validStreamInfo.frameLen = 1024;
    
    EXPECT_EQ(CheckStreamInfo(validStreamInfo), SUCCESS);
}

/**
 * @tc.name: TestHpaeNodeCommon_057
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_057
 * @tc.desc: Test CheckStreamInfo with zero frame length
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_057, TestSize.Level0)
{
    HpaeStreamInfo invalidStreamInfo;
    invalidStreamInfo.frameLen = 0;
    
    EXPECT_EQ(CheckStreamInfo(invalidStreamInfo), ERROR);
}

/**
 * @tc.name: TestHpaeNodeCommon_058
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_058
 * @tc.desc: Test CheckStreamInfo with oversized frame length
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_058, TestSize.Level0)
{
    HpaeStreamInfo invalidStreamInfo;
    invalidStreamInfo.frameLen = 38401;
    
    EXPECT_EQ(CheckStreamInfo(invalidStreamInfo), ERROR);
}

/**
 * @tc.name: TestHpaeNodeCommon_059
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_059
 * @tc.desc: Test CheckFramelen with valid sink info frame length
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_059, TestSize.Level0)
{
    HpaeSinkInfo validSinkInfo;
    validSinkInfo.frameLen = 1024;
    
    EXPECT_EQ(CheckFramelen(validSinkInfo), SUCCESS);
}

/**
 * @tc.name: TestHpaeNodeCommon_060
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_060
 * @tc.desc: Test CheckFramelen with invalid sink info frame lengths
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_060, TestSize.Level0)
{
    HpaeSinkInfo invalidSinkInfo1;
    invalidSinkInfo1.frameLen = 0;
    EXPECT_EQ(CheckFramelen(invalidSinkInfo1), ERROR);
    
    HpaeSinkInfo invalidSinkInfo2;
    invalidSinkInfo2.frameLen = 38401;
    EXPECT_EQ(CheckFramelen(invalidSinkInfo2), ERROR);
}

/**
 * @tc.name: TestHpaeNodeCommon_061
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_061
 * @tc.desc: Test CheckSourceInfoFramelen with valid source info frame length
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_061, TestSize.Level0)
{
    HpaeSourceInfo validSourceInfo;
    validSourceInfo.frameLen = 1024;
    
    EXPECT_EQ(CheckSourceInfoFramelen(validSourceInfo), SUCCESS);
}

/**
 * @tc.name: TestHpaeNodeCommon_062
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_062
 * @tc.desc: Test CheckSourceInfoFramelen with invalid source info frame lengths
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_062, TestSize.Level0)
{
    HpaeSourceInfo invalidSourceInfo1;
    invalidSourceInfo1.frameLen = 0;
    EXPECT_EQ(CheckSourceInfoFramelen(invalidSourceInfo1), ERROR);
    
    HpaeSourceInfo invalidSourceInfo2;
    invalidSourceInfo2.frameLen = 38401;
    EXPECT_EQ(CheckSourceInfoFramelen(invalidSourceInfo2), ERROR);
}

/**
 * @tc.name: TestHpaeNodeCommon_063
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_063
 * @tc.desc: Test IsCollabRecordSupported with supported and unsupported source types
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_063, TestSize.Level0)
{
    EXPECT_TRUE(IsCollabRecordSupported(SOURCE_TYPE_LIVE));
    EXPECT_FALSE(IsCollabRecordSupported(SOURCE_TYPE_MIC));
    EXPECT_FALSE(IsCollabRecordSupported(SOURCE_TYPE_CAMCORDER));
    EXPECT_FALSE(IsCollabRecordSupported(SOURCE_TYPE_VOICE_CALL));
}

/**
 * @tc.name: TestHpaeNodeCommon_064
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_064
 * @tc.desc: Test TransNodeInfoForCollaboration with non-movie scene when collaboration enabled
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_064, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    nodeInfo.effectInfo.effectScene = SCENE_MUSIC;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    
    TransNodeInfoForCollaboration(nodeInfo, true);
    EXPECT_EQ(nodeInfo.effectInfo.effectScene, SCENE_MUSIC);
    EXPECT_EQ(nodeInfo.sceneType, HPAE_SCENE_MUSIC);
}

/**
 * @tc.name: TestHpaeNodeCommon_065
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_065
 * @tc.desc: Test RecoverNodeInfoForCollaboration with non-collaborative scene
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_065, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    nodeInfo.effectInfo.effectScene = SCENE_MUSIC;
    nodeInfo.effectInfo.lastEffectScene = SCENE_OTHERS;
    
    RecoverNodeInfoForCollaboration(nodeInfo);
    EXPECT_EQ(nodeInfo.effectInfo.effectScene, SCENE_MUSIC);
}

/**
 * @tc.name: TestHpaeNodeCommon_066
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_066
 * @tc.desc: Test ConvertHpaeToRendererState with invalid session state
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_066, TestSize.Level0)
{
    HpaeSessionState invalidState = static_cast<HpaeSessionState>(999);
    RendererState result = ConvertHpaeToRendererState(invalidState);
    EXPECT_EQ(result, RENDERER_INVALID);
}

/**
 * @tc.name: TestHpaeNodeCommon_067
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_067
 * @tc.desc: Test ConvertHpaeToCapturerState with invalid session state
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_067, TestSize.Level0)
{
    HpaeSessionState invalidState = static_cast<HpaeSessionState>(999);
    CapturerState result = ConvertHpaeToCapturerState(invalidState);
    EXPECT_EQ(result, CAPTURER_INVALID);
}

/**
 * @tc.name: TestHpaeNodeCommon_068
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_068
 * @tc.desc: Test ConvertHpaeToRendererState with valid session states
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_068, TestSize.Level0)
{
    EXPECT_EQ(ConvertHpaeToRendererState(HPAE_SESSION_INVALID), RENDERER_INVALID);
    EXPECT_EQ(ConvertHpaeToRendererState(HPAE_SESSION_NEW), RENDERER_NEW);
    EXPECT_EQ(ConvertHpaeToRendererState(HPAE_SESSION_PREPARED), RENDERER_PREPARED);
    EXPECT_EQ(ConvertHpaeToRendererState(HPAE_SESSION_RUNNING), RENDERER_RUNNING);
    EXPECT_EQ(ConvertHpaeToRendererState(HPAE_SESSION_PAUSING), RENDERER_PAUSED);
    EXPECT_EQ(ConvertHpaeToRendererState(HPAE_SESSION_PAUSED), RENDERER_PAUSED);
    EXPECT_EQ(ConvertHpaeToRendererState(HPAE_SESSION_STOPPING), RENDERER_STOPPED);
    EXPECT_EQ(ConvertHpaeToRendererState(HPAE_SESSION_STOPPED), RENDERER_STOPPED);
    EXPECT_EQ(ConvertHpaeToRendererState(HPAE_SESSION_RELEASED), RENDERER_RELEASED);
}

/**
 * @tc.name: TestHpaeNodeCommon_069
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_069
 * @tc.desc: Test ConvertHpaeToCapturerState with valid session states
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_069, TestSize.Level0)
{
    EXPECT_EQ(ConvertHpaeToCapturerState(HPAE_SESSION_INVALID), CAPTURER_INVALID);
    EXPECT_EQ(ConvertHpaeToCapturerState(HPAE_SESSION_NEW), CAPTURER_NEW);
    EXPECT_EQ(ConvertHpaeToCapturerState(HPAE_SESSION_PREPARED), CAPTURER_PREPARED);
    EXPECT_EQ(ConvertHpaeToCapturerState(HPAE_SESSION_RUNNING), CAPTURER_RUNNING);
    EXPECT_EQ(ConvertHpaeToCapturerState(HPAE_SESSION_PAUSING), CAPTURER_PAUSED);
    EXPECT_EQ(ConvertHpaeToCapturerState(HPAE_SESSION_PAUSED), CAPTURER_PAUSED);
    EXPECT_EQ(ConvertHpaeToCapturerState(HPAE_SESSION_STOPPING), CAPTURER_STOPPED);
    EXPECT_EQ(ConvertHpaeToCapturerState(HPAE_SESSION_STOPPED), CAPTURER_STOPPED);
    EXPECT_EQ(ConvertHpaeToCapturerState(HPAE_SESSION_RELEASED), CAPTURER_RELEASED);
}

/**
 * @tc.name: TestHpaeNodeCommon_070
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_070
 * @tc.desc: Test ConfigNodeInfo with valid stream info
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_070, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    HpaeStreamInfo streamInfo;
    streamInfo.channels = STEREO;
    streamInfo.format = SAMPLE_S16LE;
    streamInfo.frameLen = 1024;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.sessionId = 100;
    streamInfo.customSampleRate = 0;
    streamInfo.channelLayout = CH_LAYOUT_STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.effectInfo.effectScene = SCENE_MUSIC;
    streamInfo.fadeType = NONE_FADE;
    streamInfo.sourceType = SOURCE_TYPE_MIC;
    streamInfo.keepRunning = false;
    
    ConfigNodeInfo(nodeInfo, streamInfo);
    
    EXPECT_EQ(nodeInfo.channels, STEREO);
    EXPECT_EQ(nodeInfo.format, SAMPLE_S16LE);
    EXPECT_EQ(nodeInfo.frameLen, 1024);
    EXPECT_EQ(nodeInfo.streamType, STREAM_MUSIC);
    EXPECT_EQ(nodeInfo.sessionId, 100);
}

/**
 * @tc.name: TestHpaeNodeCommon_071
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_071
 * @tc.desc: Test ConvertDeviceClassToPipe with voip and direct device classes
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_071, TestSize.Level0)
{
    EXPECT_EQ(ConvertDeviceClassToPipe("voip"), PIPE_TYPE_OUT_DIRECT_NORMAL);
    EXPECT_EQ(ConvertDeviceClassToPipe("direct"), PIPE_TYPE_OUT_DIRECT_NORMAL);
}

/**
 * @tc.name: FaultCode_TransModuleInfoToHpaeSourceInfo_InvalidFormat
 * @tc.type: FUNC
 * @tc.number: FaultCode_TransModuleInfoToHpaeSourceInfo_InvalidFormat
 * @tc.desc: Test TransModuleInfoToHpaeSourceInfo with invalid format,
 *           should report FAULT_NO_SOUND/RECORD/CREATE/ERR_INVALID_PARAM
 *           or return ERROR.
 */
HWTEST_F(HpaeNodeCommonTest,
    FaultCode_TransModuleInfoToHpaeSourceInfo_InvalidFormat, TestSize.Level1)
{
    AudioModuleInfo moduleInfo;
    moduleInfo.rate = "48000";
    moduleInfo.channels = "2";
    moduleInfo.format = "invalid_format";
    moduleInfo.bufferSize = "3840";
    moduleInfo.networkId = "local";
    moduleInfo.className = "primary";
    moduleInfo.channelLayout = "3";
    moduleInfo.deviceType = "1";

    HpaeSourceInfo sourceInfo;
    int32_t ret = TransModuleInfoToHpaeSourceInfo(moduleInfo, sourceInfo);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name: FaultCode_TransModuleInfoToHpaeSourceInfo_InvalidChannels
 * @tc.type: FUNC
 * @tc.number: FaultCode_TransModuleInfoToHpaeSourceInfo_InvalidChannels
 * @tc.desc: Test TransModuleInfoToHpaeSourceInfo with invalid channels,
 *           should report FAULT_NO_SOUND/RECORD/SEND/ERR_INVALID_PARAM
 *           fault code.
 */
HWTEST_F(HpaeNodeCommonTest,
    FaultCode_TransModuleInfoToHpaeSourceInfo_InvalidChannels, TestSize.Level1)
{
    AudioModuleInfo moduleInfo;
    moduleInfo.rate = "48000";
    moduleInfo.channels = "0";
    moduleInfo.format = "s16le";
    moduleInfo.bufferSize = "3840";
    moduleInfo.networkId = "local";
    moduleInfo.className = "primary";
    moduleInfo.channelLayout = "0";
    moduleInfo.deviceType = "1";

    HpaeSourceInfo sourceInfo;
    int32_t ret = TransModuleInfoToHpaeSourceInfo(moduleInfo, sourceInfo);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name: TestHpaeNodeCommon_072
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_072
 * @tc.desc: Test CheckSourceInfoIsDifferent with same source info
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_072, TestSize.Level0)
{
    HpaeSourceInfo info1;
    info1.deviceNetId = "local";
    info1.deviceClass = "primary";
    info1.adapterName = "adapter";
    info1.sourceName = "source";
    info1.sourceType = SOURCE_TYPE_MIC;
    info1.filePath = "/path";
    info1.deviceName = "device";
    info1.frameLen = 960;
    info1.samplingRate = SAMPLE_RATE_48000;
    info1.format = SAMPLE_S16LE;
    info1.channels = STEREO;
    info1.channelLayout = CH_LAYOUT_STEREO;
    info1.deviceType = 1;
    info1.volume = 1.0f;
    info1.openMicSpeaker = 0;
    info1.ecType = HPAE_EC_TYPE_NONE;
    info1.ecFrameLen = 960;
    info1.ecSamplingRate = SAMPLE_RATE_48000;
    info1.ecFormat = SAMPLE_S16LE;
    info1.ecChannels = STEREO;
    info1.micRef = HPAE_REF_OFF;
    info1.micRefFrameLen = 960;
    info1.micRefSamplingRate = SAMPLE_RATE_48000;
    info1.micRefFormat = SAMPLE_S16LE;
    info1.micRefChannels = STEREO;
    info1.micInSamplingRate = SAMPLE_RATE_48000;
    info1.micInFormat = SAMPLE_S16LE;
    info1.micInChannels = STEREO;
    info1.routeFlag = 0;
    
    HpaeSourceInfo info2 = info1;
    
    EXPECT_FALSE(CheckSourceInfoIsDifferent(info1, info2));
}

/**
 * @tc.name: TestHpaeNodeCommon_073
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_073
 * @tc.desc: Test CheckSourceInfoIsDifferent with different source info
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_073, TestSize.Level0)
{
    HpaeSourceInfo info1;
    info1.deviceNetId = "local";
    info1.frameLen = 960;
    
    HpaeSourceInfo info2;
    info2.deviceNetId = "remote";
    info2.frameLen = 960;
    
    EXPECT_TRUE(CheckSourceInfoIsDifferent(info1, info2));
}

/**
 * @tc.name: TestHpaeNodeCommon_074
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_074
 * @tc.desc: Test TransSinkInfoToNodeInfo with valid sink info
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_074, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.channels = STEREO;
    sinkInfo.format = SAMPLE_S16LE;
    sinkInfo.frameLen = 1024;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.deviceNetId = "local";
    sinkInfo.deviceClass = "primary";
    
    std::weak_ptr<INodeCallback> statusCallback;
    HpaeNodeInfo nodeInfo;
    
    TransSinkInfoToNodeInfo(sinkInfo, statusCallback, nodeInfo);
    
    EXPECT_EQ(nodeInfo.channels, STEREO);
    EXPECT_EQ(nodeInfo.format, SAMPLE_S16LE);
    EXPECT_EQ(nodeInfo.frameLen, 1024);
    EXPECT_EQ(nodeInfo.samplingRate, SAMPLE_RATE_48000);
    EXPECT_EQ(nodeInfo.sceneType, HPAE_SCENE_EFFECT_OUT);
    EXPECT_EQ(nodeInfo.deviceNetId, "local");
    EXPECT_EQ(nodeInfo.deviceClass, "primary");
}

/**
 * @tc.name: TestHpaeNodeCommon_075
 * @tc.type: FUNC
 * @tc.number: TestHpaeNodeCommon_075
 * @tc.desc: Test TransStreamInfoToStreamDumpInfo with valid stream info map
 */
HWTEST_F(HpaeNodeCommonTest, TestHpaeNodeCommon_075, TestSize.Level0)
{
    std::unordered_map<uint32_t, HpaeSessionInfo> streamInfoMap;
    HpaeSessionInfo sessionInfo;
    sessionInfo.streamInfo.sessionId = 100;
    sessionInfo.streamInfo.deviceName = "Speaker";
    sessionInfo.streamInfo.uid = 1000;
    sessionInfo.streamInfo.pid = 1001;
    sessionInfo.streamInfo.tokenId = 1002;
    sessionInfo.offloadEnable = false;
    sessionInfo.streamInfo.privacyType = PRIVACY_TYPE_PUBLIC;
    sessionInfo.state = HPAE_SESSION_RUNNING;
    sessionInfo.startTime = 0;
    
    streamInfoMap[100] = sessionInfo;
    
    std::vector<HpaeInputOutputInfo> dumpInfo;
    TransStreamInfoToStreamDumpInfo(streamInfoMap, dumpInfo);
    
    EXPECT_EQ(dumpInfo.size(), 1);
    EXPECT_EQ(dumpInfo[0].sessionId, 100);
    EXPECT_EQ(dumpInfo[0].deviceName, "Speaker");
    EXPECT_EQ(dumpInfo[0].uid, 1000);
}

} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS