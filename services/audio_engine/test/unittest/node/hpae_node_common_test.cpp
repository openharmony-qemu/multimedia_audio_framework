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
} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS