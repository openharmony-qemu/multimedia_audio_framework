/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef LOG_TAG
#define LOG_TAG "AudioCollaborationManagerUnitTest"
#endif

#include "audio_collaboration_manager_unit_test.h"

#include <chrono>
#include <thread>
#include <fstream>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "audio_effect.h"
#include "audio_effect_log.h"
#include "audio_effect_chain_manager.h"
#include "audio_effect_rotation.h"
#include "audio_errors.h"
#include "audio_effect_chain.h"
#include "audio_xml_node_mock.h"

using namespace std;
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {

namespace {
static constexpr int32_t DEFAULT_LATENCY_TEST = 205;
}

void AudioCollaborationManagerUnitTest::SetUpTestCase(void) {}
void AudioCollaborationManagerUnitTest::TearDownTestCase(void) {}
void AudioCollaborationManagerUnitTest::SetUp(void) {}
void AudioCollaborationManagerUnitTest::TearDown(void) {}

/**
* @tc.name   : Test UpdateCollaborativeProductId API
* @tc.number : updateCollaborativeProductId_001
* @tc.desc   : Test UpdateCollaborativeProductId interface(using empty use case).
*/
HWTEST(AudioCollaborationManagerUnitTest, updateCollaborativeProductId_001, TestSize.Level1)
{
    AudioCollaborationManager::GetInstance()->productId_ = "testProductId";
    int32_t ret = AudioCollaborationManager::GetInstance()->UpdateCollaborativeProductId("testProductId_new");
    EXPECT_EQ(ret, SUCCESS);

    ret = AudioCollaborationManager::GetInstance()->UpdateCollaborativeProductId("newProductId_new");
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name   : Test UpdateCollaborativeProductId API
* @tc.number : updateCollaborativeProductId_002
* @tc.desc   : Test UpdateCollaborativeProductId interface(using empty use case).
*/
HWTEST(AudioCollaborationManagerUnitTest, updateCollaborativeProductId_002, TestSize.Level1)
{
    std::string productId = "11_123456";
    AudioCollaborationManager::GetInstance()->collaborativeEarphoneProductConfig_ = {{"00014B", 1}, {"000167", 2}};
    int32_t ret = AudioCollaborationManager::GetInstance()->UpdateCollaborativeProductId(productId);
    EXPECT_EQ(ret, ERROR);

    productId = "00014B_07_4113";
    ret = AudioCollaborationManager::GetInstance()->UpdateCollaborativeProductId(productId);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(AudioCollaborationManager::GetInstance()->productId_, "00014B");
}

/**
* @tc.name   : Test UpdateCollaborativeProductId API
* @tc.number : updateCollaborativeProductId_003
* @tc.desc   : Test UpdateCollaborativeProductId interface(using empty use case).
*/
HWTEST(AudioCollaborationManagerUnitTest, updateCollaborativeProductId_003, TestSize.Level1)
{
    std::string productId = "testProduct_123";
    std::string temproductId = "testProduct";
    int32_t earphoneProduct = 1;
    AudioCollaborationManager::GetInstance()->collaborativeEarphoneProductConfig_[temproductId] = earphoneProduct;
    int32_t ret = AudioCollaborationManager::GetInstance()->UpdateCollaborativeProductId(productId);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(AudioCollaborationManager::GetInstance()->productId_, temproductId);
}

/**
* @tc.name   : Test updateLatencyInner API
* @tc.number : updateLatencyInner_001
* @tc.desc   : Test updateLatencyInner interface(using empty use case).
*/
HWTEST(AudioCollaborationManagerUnitTest, updateLatencyInner_001, TestSize.Level1)
{
    AudioCollaborationManager::GetInstance()->LoadCollaborationConfig();


    AudioCollaborationManager::GetInstance()->productId_ = "00014B0";
    AudioCollaborationManager::GetInstance()->updateLatencyInner();
    EXPECT_EQ(AudioCollaborationManager::GetInstance()->latencyMs_, DEFAULT_LATENCY_TEST);

    AudioCollaborationManager::GetInstance()->twsMode_ = TWS_MODE_OTHERS;
    AudioCollaborationManager::GetInstance()->updateLatencyInner();
    EXPECT_EQ(AudioCollaborationManager::GetInstance()->latencyMs_, DEFAULT_LATENCY_TEST);

    AudioCollaborationManager::GetInstance()->productId_ = "00014B";
    AudioCollaborationManager::GetInstance()->twsMode_ = TWS_MODE_DEFAULT;
    AudioCollaborationManager::GetInstance()->updateLatencyInner();
}

/**
* @tc.name   : Test LoadCollaborationLatencyInner API
* @tc.number : LoadCollaborationLatencyInner_001
* @tc.desc   : Test LoadCollaborationLatencyInner interface(using empty use case).
*/
HWTEST(AudioCollaborationManagerUnitTest, LoadCollaborationLatencyInner_001, TestSize.Level1)
{
    std::shared_ptr<AudioXmlNode> secondNode = nullptr;
    std::string prodcutId = "testProduct";
    AudioCollaborationManager::GetInstance()->LoadCollaborationLatencyInner(secondNode, prodcutId);
    EXPECT_TRUE(AudioCollaborationManager::GetInstance()->collaborativeLatencyConfig_.empty());

    std::shared_ptr<AudioXmlNode> node = AudioXmlNode::Create();
    AudioCollaborationManager::GetInstance()->LoadCollaborationLatencyInner(node, prodcutId);
    EXPECT_FALSE(AudioCollaborationManager::GetInstance()->collaborativeLatencyConfig_.empty());
}

/**
* @tc.name   : Test LoadCollaborationLatencyInner API
* @tc.number : LoadCollaborationLatencyInner_002
* @tc.desc   : Test LoadCollaborationLatencyInner interface(using empty use case).
*/
HWTEST(AudioCollaborationManagerUnitTest, LoadCollaborationLatencyInner_002, TestSize.Level1)
{
    auto mockNode = std::make_shared<MockAudioXmlNode>();
    EXPECT_CALL(*mockNode, IsNodeValid()).WillOnce(Return(false));

    std::string prodcutId = "testProduct";

    AudioCollaborationManager::GetInstance()->LoadCollaborationLatencyInner(mockNode, prodcutId);
    EXPECT_FALSE(AudioCollaborationManager::GetInstance()->collaborativeLatencyConfig_.empty());
}

/**
* @tc.name   : Test LoadCollaborationLatencyInner API
* @tc.number : LoadCollaborationLatencyInner_003
* @tc.desc   : Test LoadCollaborationLatencyInner interface(using empty use case).
*/
HWTEST(AudioCollaborationManagerUnitTest, LoadCollaborationLatencyInner_003, TestSize.Level1)
{
    auto mockNode = std::make_shared<MockAudioXmlNode>();
    EXPECT_CALL(*mockNode, IsNodeValid()).WillOnce(Return(true)).WillOnce(Return(false));
    EXPECT_CALL(*mockNode, IsElementNode()).WillOnce(Return(false));
    EXPECT_CALL(*mockNode, MoveToNext()).Times(1);

    std::string prodcutId = "testProduct";

    AudioCollaborationManager::GetInstance()->LoadCollaborationLatencyInner(mockNode, prodcutId);
    EXPECT_FALSE(AudioCollaborationManager::GetInstance()->collaborativeLatencyConfig_.empty());
}

/**
* @tc.name   : Test LoadCollaborationLatencyInner API
* @tc.number : LoadCollaborationLatencyInner_004
* @tc.desc   : Test LoadCollaborationLatencyInner interface(using empty use case).
*/
HWTEST(AudioCollaborationManagerUnitTest, LoadCollaborationLatencyInner_004, TestSize.Level1)
{
    auto mockNode = std::make_shared<MockAudioXmlNode>();
    EXPECT_CALL(*mockNode, IsNodeValid()).WillOnce(Return(true)).WillOnce(Return(false));
    EXPECT_CALL(*mockNode, IsElementNode()).WillOnce(Return(true));
    EXPECT_CALL(*mockNode, CompareName("tws_mode")).WillOnce(Return(false));
    EXPECT_CALL(*mockNode, MoveToNext()).Times(1);

    std::string prodcutId = "testProduct";

    AudioCollaborationManager::GetInstance()->LoadCollaborationLatencyInner(mockNode, prodcutId);
    EXPECT_FALSE(AudioCollaborationManager::GetInstance()->collaborativeLatencyConfig_.empty());
}

/**
* @tc.name   : Test LoadCollaborationLatencyInner API
* @tc.number : LoadCollaborationLatencyInner_005
* @tc.desc   : Test LoadCollaborationLatencyInner interface(using empty use case).
*/
HWTEST(AudioCollaborationManagerUnitTest, LoadCollaborationLatencyInner_005, TestSize.Level1)
{
    auto mockNode = std::make_shared<MockAudioXmlNode>();
    EXPECT_CALL(*mockNode, IsNodeValid()).WillOnce(Return(true)).WillOnce(Return(false));
    EXPECT_CALL(*mockNode, IsElementNode()).WillOnce(Return(true));
    EXPECT_CALL(*mockNode, CompareName("tws_mode")).WillOnce(Return(true));
    EXPECT_CALL(*mockNode, GetProp("name", _)).WillOnce(Return(-1));
    EXPECT_CALL(*mockNode, MoveToNext()).Times(1);

    std::string prodcutId = "testProduct";

    AudioCollaborationManager::GetInstance()->LoadCollaborationLatencyInner(mockNode, prodcutId);
    EXPECT_FALSE(AudioCollaborationManager::GetInstance()->collaborativeLatencyConfig_.empty());
}

/**
* @tc.name   : Test LoadCollaborationLatencyInner API
* @tc.number : LoadCollaborationLatencyInner_006
* @tc.desc   : Test LoadCollaborationLatencyInner interface(using empty use case).
*/
HWTEST(AudioCollaborationManagerUnitTest, LoadCollaborationLatencyInner_006, TestSize.Level1)
{
    auto mockNode = std::make_shared<MockAudioXmlNode>();
    EXPECT_CALL(*mockNode, IsNodeValid()).WillOnce(Return(true)).WillOnce(Return(false));
    EXPECT_CALL(*mockNode, IsElementNode()).WillOnce(Return(true));
    EXPECT_CALL(*mockNode, CompareName("tws_mode")).WillOnce(Return(true));
    EXPECT_CALL(*mockNode, GetProp("name", _)).WillOnce(DoAll(SetArgReferee<1>("testMode"), Return(SUCCESS)));
    EXPECT_CALL(*mockNode, GetProp("latency_ms", _)).WillOnce(Return(-1));
    EXPECT_CALL(*mockNode, MoveToNext()).Times(1);

    std::string prodcutId = "testProduct";

    AudioCollaborationManager::GetInstance()->LoadCollaborationLatencyInner(mockNode, prodcutId);
    EXPECT_FALSE(AudioCollaborationManager::GetInstance()->collaborativeLatencyConfig_.empty());
}

/**
* @tc.name   : Test LoadCollaborationLatencyInner API
* @tc.number : LoadCollaborationLatencyInner_007
* @tc.desc   : Test LoadCollaborationLatencyInner interface(using empty use case).
*/
HWTEST(AudioCollaborationManagerUnitTest, LoadCollaborationLatencyInner_007, TestSize.Level1)
{
    auto mockNode = std::make_shared<MockAudioXmlNode>();
    EXPECT_CALL(*mockNode, IsNodeValid()).WillOnce(Return(true)).WillOnce(Return(false));
    EXPECT_CALL(*mockNode, IsElementNode()).WillOnce(Return(true));
    EXPECT_CALL(*mockNode, CompareName("tws_mode")).WillOnce(Return(true));
    EXPECT_CALL(*mockNode, GetProp("name", _)).WillOnce(DoAll(SetArgReferee<1>("testMode"), Return(SUCCESS)));
    EXPECT_CALL(*mockNode, GetProp("latency_ms", _)).WillOnce(DoAll(SetArgReferee<1>("100"), Return(SUCCESS)));
    EXPECT_CALL(*mockNode, MoveToNext()).Times(1);

    std::string prodcutId = "testProduct";

    AudioCollaborationManager::GetInstance()->LoadCollaborationLatencyInner(mockNode, prodcutId);
    EXPECT_FALSE(AudioCollaborationManager::GetInstance()->collaborativeLatencyConfig_.empty());
}

/**
* @tc.name   : Test LoadCollaborationLatencyInner API
* @tc.number : LoadCollaborationLatencyInner_008
* @tc.desc   : Test LoadCollaborationLatencyInner interface(using empty use case).
*/
HWTEST(AudioCollaborationManagerUnitTest, LoadCollaborationLatencyInner_008, TestSize.Level1)
{
    auto mockNode = std::make_shared<MockAudioXmlNode>();
    EXPECT_CALL(*mockNode, IsNodeValid()).WillOnce(Return(true)).WillOnce(Return(false));
    EXPECT_CALL(*mockNode, IsElementNode()).WillOnce(Return(true));
    EXPECT_CALL(*mockNode, CompareName("tws_mode")).WillOnce(Return(true));
    EXPECT_CALL(*mockNode, GetProp("name", _)).WillOnce(DoAll(SetArgReferee<1>("listen"), Return(SUCCESS)));
    EXPECT_CALL(*mockNode, GetProp("latency_ms", _)).WillOnce(DoAll(SetArgReferee<1>("100"), Return(SUCCESS)));
    EXPECT_CALL(*mockNode, MoveToNext()).Times(1);

    std::string prodcutId = "testProduct";

    AudioCollaborationManager::GetInstance()->LoadCollaborationLatencyInner(mockNode, prodcutId);
    EXPECT_EQ(AudioCollaborationManager::GetInstance()->collaborativeLatencyConfig_[prodcutId][TWS_MODE_LISTEN], 100);
}

/**
* @tc.name   : Test LoadCollaborationConfigInner API
* @tc.number : LoadCollaborationConfigInner_001
* @tc.desc   : Test LoadCollaborationConfigInner interface(using empty use case).
*/
HWTEST(AudioCollaborationManagerUnitTest, LoadCollaborationConfigInner_001, TestSize.Level1)
{
    AudioCollaborationManager::GetInstance()->LoadCollaborationConfigInner(nullptr);
    EXPECT_TRUE(AudioCollaborationManager::GetInstance()->collaborativeEarphoneNameConfig_.empty());
}

/**
* @tc.name   : Test LoadCollaborationConfigInner API
* @tc.number : LoadCollaborationConfigInner_002
* @tc.desc   : Test LoadCollaborationConfigInner interface(using empty use case).
*/
HWTEST(AudioCollaborationManagerUnitTest, LoadCollaborationConfigInner_002, TestSize.Level1)
{
    auto mockNode = std::make_shared<MockAudioXmlNode>();
    EXPECT_CALL(*mockNode, IsNodeValid()).WillOnce(Return(false));

    AudioCollaborationManager::GetInstance()->LoadCollaborationConfigInner(mockNode);
    EXPECT_TRUE(AudioCollaborationManager::GetInstance()->collaborativeEarphoneNameConfig_.empty());
}

/**
* @tc.name   : Test LoadCollaborationConfigInner API
* @tc.number : LoadCollaborationConfigInner_003
* @tc.desc   : Test LoadCollaborationConfigInner interface(using empty use case).
*/
HWTEST(AudioCollaborationManagerUnitTest, LoadCollaborationConfigInner_003, TestSize.Level1)
{
    auto mockNode = std::make_shared<MockAudioXmlNode>();
    EXPECT_CALL(*mockNode, IsNodeValid()).WillOnce(Return(true)).WillOnce(Return(true)).WillOnce(Return(false));
    EXPECT_CALL(*mockNode, MoveToChildren()).Times(1);
    EXPECT_CALL(*mockNode, IsElementNode()).WillOnce(Return(false));
    EXPECT_CALL(*mockNode, MoveToNext()).Times(1);

    AudioCollaborationManager::GetInstance()->LoadCollaborationConfigInner(mockNode);
    EXPECT_TRUE(AudioCollaborationManager::GetInstance()->collaborativeEarphoneNameConfig_.empty());
}

/**
* @tc.name   : Test LoadCollaborationConfigInner API
* @tc.number : LoadCollaborationConfigInner_005
* @tc.desc   : Test LoadCollaborationConfigInner interface(using empty use case).
*/
HWTEST(AudioCollaborationManagerUnitTest, LoadCollaborationConfigInner_005, TestSize.Level1)
{
    auto mockNode = std::make_shared<MockAudioXmlNode>();
    auto mockCopyNode = std::make_shared<MockAudioXmlNode>();
    EXPECT_CALL(*mockNode, IsNodeValid()).WillOnce(Return(true)).WillOnce(Return(true)).WillOnce(Return(false));
    EXPECT_CALL(*mockNode, MoveToChildren()).Times(1);
    EXPECT_CALL(*mockNode, IsElementNode()).WillOnce(Return(true));
    EXPECT_CALL(*mockNode, CompareName("collabSupportedStreamUsage")).WillOnce(Return(true));
    EXPECT_CALL(*mockNode, GetCopyNode()).Times(1).WillOnce(Return(mockCopyNode));
    EXPECT_CALL(*mockCopyNode, GetProp("deviceType", _)).WillOnce(Return(-1));
    EXPECT_CALL(*mockNode, MoveToNext()).Times(1);

    AudioCollaborationManager::GetInstance()->LoadCollaborationConfigInner(mockNode);
    EXPECT_TRUE(AudioCollaborationManager::GetInstance()->collaborativeEarphoneNameConfig_.empty());
}

/**
* @tc.name   : Test LoadCollaborationConfigInner API
* @tc.number : LoadCollaborationConfigInner_006
* @tc.desc   : Test LoadCollaborationConfigInner interface(using empty use case).
*/
HWTEST(AudioCollaborationManagerUnitTest, LoadCollaborationConfigInner_006, TestSize.Level1)
{
    auto mockNode = std::make_shared<MockAudioXmlNode>();
    auto mockCopyNode = std::make_shared<MockAudioXmlNode>();
    EXPECT_CALL(*mockNode, IsNodeValid()).WillOnce(Return(true)).WillOnce(Return(true)).WillOnce(Return(false));
    EXPECT_CALL(*mockNode, MoveToChildren()).Times(1);
    EXPECT_CALL(*mockNode, IsElementNode()).WillOnce(Return(true));
    EXPECT_CALL(*mockNode, CompareName("collabSupportedStreamUsage")).WillOnce(Return(true));
    EXPECT_CALL(*mockNode, GetCopyNode()).Times(1).WillOnce(Return(mockCopyNode));
    EXPECT_CALL(*mockCopyNode, GetProp("deviceType", _)).WillOnce(Return(-1));
    EXPECT_CALL(*mockNode, MoveToNext()).Times(1);

    AudioCollaborationManager::GetInstance()->LoadCollaborationConfigInner(mockNode);
    EXPECT_TRUE(AudioCollaborationManager::GetInstance()->collaborativeEarphoneNameConfig_.empty());
}

/**
* @tc.name   : Test LoadCollaborationConfigInner API
* @tc.number : LoadCollaborationConfigInner_007
* @tc.desc   : Test LoadCollaborationConfigInner interface(using empty use case).
*/
HWTEST(AudioCollaborationManagerUnitTest, LoadCollaborationConfigInner_007, TestSize.Level1)
{
    auto mockNode = std::make_shared<MockAudioXmlNode>();
    EXPECT_CALL(*mockNode, IsNodeValid()).WillOnce(Return(true)).WillOnce(Return(true)).WillOnce(Return(false));
    EXPECT_CALL(*mockNode, MoveToChildren()).Times(1);
    EXPECT_CALL(*mockNode, IsElementNode()).WillOnce(Return(true));
    EXPECT_CALL(*mockNode, CompareName("collabSupportedStreamUsage")).WillOnce(Return(true));
    EXPECT_CALL(*mockNode, MoveToNext()).Times(1);

    AudioCollaborationManager::GetInstance()->LoadCollaborationConfigInner(mockNode);
    EXPECT_TRUE(AudioCollaborationManager::GetInstance()->collaborativeEarphoneNameConfig_.empty());
}

/**
* @tc.name   : Test LoadCollaborationConfig API
* @tc.number : LoadCollaborationConfig_001
* @tc.desc   : Test LoadCollaborationConfig interface(using empty use case).
*/
HWTEST(AudioCollaborationManagerUnitTest, LoadCollaborationConfig_001, TestSize.Level1)
{
    AudioCollaborationManager::GetInstance()->collaborativeEarphoneProductConfig_ = {{"00014B", 1}, {"000167", 2}};
    AudioCollaborationManager::GetInstance()->LoadCollaborationConfig();
    EXPECT_TRUE(true);
}

/**
* @tc.name   : Test LoadCollaborationConfig API
* @tc.number : LoadCollaborationConfig_002
* @tc.desc   : Test LoadCollaborationConfig interface(using empty use case).
*/
HWTEST(AudioCollaborationManagerUnitTest, LoadCollaborationConfig_002, TestSize.Level1)
{
    AudioCollaborationManager::GetInstance()->collaborativeEarphoneNameConfig_ =
        {{"00014B", "DOVE"}, {"000167", "ROBIN"}};
    AudioCollaborationManager::GetInstance()->LoadCollaborationConfig();
    EXPECT_TRUE(true);
}

/**
* @tc.name   : Test LoadCollaborationConfig API
* @tc.number : LoadCollaborationConfig_003
* @tc.desc   : Test LoadCollaborationConfig interface(using empty use case).
*/
HWTEST(AudioCollaborationManagerUnitTest, LoadCollaborationConfig_003, TestSize.Level1)
{
    AudioCollaborationManager::GetInstance()->collaborativeEarphoneProductConfig_.clear();
    AudioCollaborationManager::GetInstance()->collaborativeEarphoneNameConfig_.clear();
    AudioCollaborationManager::GetInstance()->collaborativeLatencyConfig_.clear();
    AudioCollaborationManager::GetInstance()->LoadCollaborationConfig();
    EXPECT_TRUE(true);
}

// ===== Tests for IsStreamUsageSupported =====

/**
* @tc.name   : Test IsStreamUsageSupported with BT A2DP device
* @tc.number : IsStreamUsageSupported_BtA2dp_001
* @tc.desc   : When BT A2DP config has MOVIE and MUSIC, should return true for those and false for others.
*/
HWTEST(AudioCollaborationManagerUnitTest, IsStreamUsageSupported_BtA2dp_001, TestSize.Level1)
{
    auto *inst = AudioCollaborationManager::GetInstance();
    inst->collAbsSupportedStreamUsageConfig_.clear();
    inst->collAbsSupportedStreamUsageConfig_[DEVICE_TYPE_BLUETOOTH_A2DP].insert(STREAM_USAGE_MOVIE);
    inst->collAbsSupportedStreamUsageConfig_[DEVICE_TYPE_BLUETOOTH_A2DP].insert(STREAM_USAGE_MUSIC);

    EXPECT_TRUE(inst->IsStreamUsageSupported(DEVICE_TYPE_BLUETOOTH_A2DP, STREAM_USAGE_MOVIE));
    EXPECT_TRUE(inst->IsStreamUsageSupported(DEVICE_TYPE_BLUETOOTH_A2DP, STREAM_USAGE_MUSIC));
    EXPECT_FALSE(inst->IsStreamUsageSupported(DEVICE_TYPE_BLUETOOTH_A2DP, STREAM_USAGE_VOICE_COMMUNICATION));
}

/**
* @tc.name   : Test IsStreamUsageSupported with NearLink device
* @tc.number : IsStreamUsageSupported_Nl_002
* @tc.desc   : When NearLink config has MOVIE only, should return true for MOVIE and false for MUSIC.
*/
HWTEST(AudioCollaborationManagerUnitTest, IsStreamUsageSupported_Nl_002, TestSize.Level1)
{
    auto *inst = AudioCollaborationManager::GetInstance();
    inst->collAbsSupportedStreamUsageConfig_.clear();
    inst->collAbsSupportedStreamUsageConfig_[DEVICE_TYPE_NEARLINK].insert(STREAM_USAGE_MOVIE);

    EXPECT_TRUE(inst->IsStreamUsageSupported(DEVICE_TYPE_NEARLINK, STREAM_USAGE_MOVIE));
    EXPECT_FALSE(inst->IsStreamUsageSupported(DEVICE_TYPE_NEARLINK, STREAM_USAGE_MUSIC));
}

/**
* @tc.name   : Test IsStreamUsageSupported with empty config
* @tc.number : IsStreamUsageSupported_EmptyCfg_003
* @tc.desc   : When config is empty, should return false.
*/
HWTEST(AudioCollaborationManagerUnitTest, IsStreamUsageSupported_EmptyCfg_003, TestSize.Level1)
{
    auto *inst = AudioCollaborationManager::GetInstance();
    inst->collAbsSupportedStreamUsageConfig_.clear();
    EXPECT_FALSE(inst->IsStreamUsageSupported(DEVICE_TYPE_BLUETOOTH_A2DP, STREAM_USAGE_MOVIE));
}

// ===== Tests for LoadCollAbsSupportedStreamUsageInner =====

/**
* @tc.name   : Test LoadCollAbsSupportedStreamUsageInner with nullptr
* @tc.number : LoadCollAbsSupportedStreamUsageInner_Null_001
* @tc.desc   : Passing nullptr should not crash and config should remain empty.
*/
HWTEST(AudioCollaborationManagerUnitTest, LoadCollAbsSupportedStreamUsageInner_Null_001, TestSize.Level1)
{
    auto *inst = AudioCollaborationManager::GetInstance();
    inst->collAbsSupportedStreamUsageConfig_.clear();
    inst->LoadCollAbsSupportedStreamUsageInner(nullptr);
    EXPECT_TRUE(inst->collAbsSupportedStreamUsageConfig_.empty());
}

/**
* @tc.name   : Test LoadCollAbsSupportedStreamUsageInner missing deviceType prop
* @tc.number : LoadCollAbsSupportedStreamUsageInner_NoDevType_002
* @tc.desc   : Node without deviceType property should return early.
*/
HWTEST(AudioCollaborationManagerUnitTest, LoadCollAbsSupportedStreamUsageInner_NoDevType_002, TestSize.Level1)
{
    auto *inst = AudioCollaborationManager::GetInstance();
    inst->collAbsSupportedStreamUsageConfig_.clear();
    auto mockNode = std::make_shared<MockAudioXmlNode>();
    EXPECT_CALL(*mockNode, GetProp("deviceType", _)).WillOnce(Return(-1));
    inst->LoadCollAbsSupportedStreamUsageInner(mockNode);
    EXPECT_TRUE(inst->collAbsSupportedStreamUsageConfig_.empty());
}

/**
* @tc.name   : Test LoadCollAbsSupportedStreamUsageInner invalid deviceType
* @tc.number : LoadCollAbsSupportedStreamUsageInner_InvalidDevType_003
* @tc.desc   : Node with unrecognized deviceType string should return early.
*/
HWTEST(AudioCollaborationManagerUnitTest, LoadCollAbsSupportedStreamUsageInner_InvalidDevType_003, TestSize.Level1)
{
    auto *inst = AudioCollaborationManager::GetInstance();
    inst->collAbsSupportedStreamUsageConfig_.clear();
    auto mockNode = std::make_shared<MockAudioXmlNode>();
    EXPECT_CALL(*mockNode, GetProp("deviceType", _))
        .WillOnce(DoAll(SetArgReferee<1>("INVALID_DEVICE"), Return(SUCCESS)));
    inst->LoadCollAbsSupportedStreamUsageInner(mockNode);
    EXPECT_TRUE(inst->collAbsSupportedStreamUsageConfig_.empty());
}

/**
* @tc.name   : Test LoadCollAbsSupportedStreamUsageInner with valid deviceType but no children
* @tc.number : LoadCollAbsSupportedStreamUsageInner_NoChildren_004
* @tc.desc   : Valid deviceType but node has no valid children.
*/
HWTEST(AudioCollaborationManagerUnitTest, LoadCollAbsSupportedStreamUsageInner_NoChildren_004, TestSize.Level1)
{
    auto *inst = AudioCollaborationManager::GetInstance();
    inst->collAbsSupportedStreamUsageConfig_.clear();
    auto mockNode = std::make_shared<MockAudioXmlNode>();
    EXPECT_CALL(*mockNode, GetProp("deviceType", _))
        .WillOnce(DoAll(SetArgReferee<1>("DEVICE_TYPE_BLUETOOTH_A2DP"), Return(SUCCESS)));
    EXPECT_CALL(*mockNode, MoveToChildren()).Times(1);
    EXPECT_CALL(*mockNode, IsNodeValid()).WillOnce(Return(false));
    inst->LoadCollAbsSupportedStreamUsageInner(mockNode);
    EXPECT_TRUE(inst->collAbsSupportedStreamUsageConfig_.empty());
}

/**
* @tc.name   : Test LoadCollAbsSupportedStreamUsageInner with non-element child
* @tc.number : LoadCollAbsSupportedStreamUsageInner_NonElement_005
* @tc.desc   : Child node is not an element node, should skip.
*/
HWTEST(AudioCollaborationManagerUnitTest, LoadCollAbsSupportedStreamUsageInner_NonElement_005, TestSize.Level1)
{
    auto *inst = AudioCollaborationManager::GetInstance();
    inst->collAbsSupportedStreamUsageConfig_.clear();
    auto mockNode = std::make_shared<MockAudioXmlNode>();
    EXPECT_CALL(*mockNode, GetProp("deviceType", _))
        .WillOnce(DoAll(SetArgReferee<1>("DEVICE_TYPE_NEARLINK"), Return(SUCCESS)));
    EXPECT_CALL(*mockNode, MoveToChildren()).Times(1);
    EXPECT_CALL(*mockNode, IsNodeValid()).WillOnce(Return(true)).WillOnce(Return(false));
    EXPECT_CALL(*mockNode, IsElementNode()).WillOnce(Return(false));
    EXPECT_CALL(*mockNode, MoveToNext()).Times(1);
    inst->LoadCollAbsSupportedStreamUsageInner(mockNode);
    EXPECT_TRUE(inst->collAbsSupportedStreamUsageConfig_.empty());
}

/**
* @tc.name   : Test LoadCollAbsSupportedStreamUsageInner child not named streamUsage
* @tc.number : LoadCollAbsSupportedStreamUsageInner_WrongName_006
* @tc.desc   : Child node is element but not named streamUsage, should skip.
*/
HWTEST(AudioCollaborationManagerUnitTest, LoadCollAbsSupportedStreamUsageInner_WrongName_006, TestSize.Level1)
{
    auto *inst = AudioCollaborationManager::GetInstance();
    inst->collAbsSupportedStreamUsageConfig_.clear();
    auto mockNode = std::make_shared<MockAudioXmlNode>();
    EXPECT_CALL(*mockNode, GetProp("deviceType", _))
        .WillOnce(DoAll(SetArgReferee<1>("DEVICE_TYPE_NEARLINK"), Return(SUCCESS)));
    EXPECT_CALL(*mockNode, MoveToChildren()).Times(1);
    EXPECT_CALL(*mockNode, IsNodeValid()).WillOnce(Return(true)).WillOnce(Return(false));
    EXPECT_CALL(*mockNode, IsElementNode()).WillOnce(Return(true));
    EXPECT_CALL(*mockNode, CompareName("streamUsage")).WillOnce(Return(false));
    EXPECT_CALL(*mockNode, MoveToNext()).Times(1);
    inst->LoadCollAbsSupportedStreamUsageInner(mockNode);
    EXPECT_TRUE(inst->collAbsSupportedStreamUsageConfig_.empty());
}

/**
* @tc.name   : Test LoadCollAbsSupportedStreamUsageInner child missing name prop
* @tc.number : LoadCollAbsSupportedStreamUsageInner_NoName_007
* @tc.desc   : Child streamUsage node missing name property, should skip.
*/
HWTEST(AudioCollaborationManagerUnitTest, LoadCollAbsSupportedStreamUsageInner_NoName_007, TestSize.Level1)
{
    auto *inst = AudioCollaborationManager::GetInstance();
    inst->collAbsSupportedStreamUsageConfig_.clear();
    auto mockNode = std::make_shared<MockAudioXmlNode>();
    EXPECT_CALL(*mockNode, GetProp("deviceType", _))
        .WillOnce(DoAll(SetArgReferee<1>("DEVICE_TYPE_BLUETOOTH_A2DP"), Return(SUCCESS)));
    EXPECT_CALL(*mockNode, MoveToChildren()).Times(1);
    EXPECT_CALL(*mockNode, IsNodeValid()).WillOnce(Return(true)).WillOnce(Return(false));
    EXPECT_CALL(*mockNode, IsElementNode()).WillOnce(Return(true));
    EXPECT_CALL(*mockNode, CompareName("streamUsage")).WillOnce(Return(true));
    EXPECT_CALL(*mockNode, GetProp("name", _)).WillOnce(Return(-1));
    EXPECT_CALL(*mockNode, MoveToNext()).Times(1);
    inst->LoadCollAbsSupportedStreamUsageInner(mockNode);
    EXPECT_TRUE(inst->collAbsSupportedStreamUsageConfig_.empty());
}

/**
* @tc.name   : Test LoadCollAbsSupportedStreamUsageInner invalid streamUsage name
* @tc.number : LoadCollAbsSupportedStreamUsageInner_InvalidUsage_008
* @tc.desc   : Child streamUsage node with invalid name, should skip.
*/
HWTEST(AudioCollaborationManagerUnitTest, LoadCollAbsSupportedStreamUsageInner_InvalidUsage_008, TestSize.Level1)
{
    auto *inst = AudioCollaborationManager::GetInstance();
    inst->collAbsSupportedStreamUsageConfig_.clear();
    auto mockNode = std::make_shared<MockAudioXmlNode>();
    EXPECT_CALL(*mockNode, GetProp("deviceType", _))
        .WillOnce(DoAll(SetArgReferee<1>("DEVICE_TYPE_BLUETOOTH_A2DP"), Return(SUCCESS)));
    EXPECT_CALL(*mockNode, MoveToChildren()).Times(1);
    EXPECT_CALL(*mockNode, IsNodeValid()).WillOnce(Return(true)).WillOnce(Return(false));
    EXPECT_CALL(*mockNode, IsElementNode()).WillOnce(Return(true));
    EXPECT_CALL(*mockNode, CompareName("streamUsage")).WillOnce(Return(true));
    EXPECT_CALL(*mockNode, GetProp("name", _))
        .WillOnce(DoAll(SetArgReferee<1>("INVALID_USAGE"), Return(SUCCESS)));
    EXPECT_CALL(*mockNode, MoveToNext()).Times(1);
    inst->LoadCollAbsSupportedStreamUsageInner(mockNode);
    EXPECT_TRUE(inst->collAbsSupportedStreamUsageConfig_.empty());
}

/**
* @tc.name   : Test LoadCollAbsSupportedStreamUsageInner valid config
* @tc.number : LoadCollAbsSupportedStreamUsageInner_Valid_009
* @tc.desc   : Valid deviceType and streamUsage, config should be populated.
*/
HWTEST(AudioCollaborationManagerUnitTest, LoadCollAbsSupportedStreamUsageInner_Valid_009, TestSize.Level1)
{
    auto *inst = AudioCollaborationManager::GetInstance();
    inst->collAbsSupportedStreamUsageConfig_.clear();
    auto mockNode = std::make_shared<MockAudioXmlNode>();
    EXPECT_CALL(*mockNode, GetProp("deviceType", _))
        .WillOnce(DoAll(SetArgReferee<1>("DEVICE_TYPE_NEARLINK"), Return(SUCCESS)));
    EXPECT_CALL(*mockNode, MoveToChildren()).Times(1);
    EXPECT_CALL(*mockNode, IsNodeValid()).WillOnce(Return(true)).WillOnce(Return(false));
    EXPECT_CALL(*mockNode, IsElementNode()).WillOnce(Return(true));
    EXPECT_CALL(*mockNode, CompareName("streamUsage")).WillOnce(Return(true));
    EXPECT_CALL(*mockNode, GetProp("name", _))
        .WillOnce(DoAll(SetArgReferee<1>("STREAM_USAGE_MOVIE"), Return(SUCCESS)));
    EXPECT_CALL(*mockNode, MoveToNext()).Times(1);
    inst->LoadCollAbsSupportedStreamUsageInner(mockNode);
    EXPECT_FALSE(inst->collAbsSupportedStreamUsageConfig_.empty());
    EXPECT_EQ(inst->collAbsSupportedStreamUsageConfig_.count(DEVICE_TYPE_NEARLINK), 1u);
    EXPECT_EQ(inst->collAbsSupportedStreamUsageConfig_[DEVICE_TYPE_NEARLINK].count(STREAM_USAGE_MOVIE), 1u);
}

/**
* @tc.name   : Test LoadCollAbsSupportedStreamUsageInner multiple streamUsages
* @tc.number : LoadCollAbsSupportedStreamUsageInner_Multi_010
* @tc.desc   : Multiple valid streamUsage entries for one deviceType.
*/
HWTEST(AudioCollaborationManagerUnitTest, LoadCollAbsSupportedStreamUsageInner_Multi_010, TestSize.Level1)
{
    auto *inst = AudioCollaborationManager::GetInstance();
    inst->collAbsSupportedStreamUsageConfig_.clear();
    auto mockNode = std::make_shared<MockAudioXmlNode>();
    EXPECT_CALL(*mockNode, GetProp("deviceType", _))
        .WillOnce(DoAll(SetArgReferee<1>("DEVICE_TYPE_BLUETOOTH_A2DP"), Return(SUCCESS)));
    EXPECT_CALL(*mockNode, MoveToChildren()).Times(1);
    // Three iterations: two valid, then exit
    EXPECT_CALL(*mockNode, IsNodeValid())
        .WillOnce(Return(true)).WillOnce(Return(true)).WillOnce(Return(false));
    EXPECT_CALL(*mockNode, IsElementNode()).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*mockNode, CompareName("streamUsage")).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*mockNode, GetProp("name", _))
        .WillOnce(DoAll(SetArgReferee<1>("STREAM_USAGE_MOVIE"), Return(SUCCESS)))
        .WillOnce(DoAll(SetArgReferee<1>("STREAM_USAGE_MUSIC"), Return(SUCCESS)));
    EXPECT_CALL(*mockNode, MoveToNext()).Times(2);
    inst->LoadCollAbsSupportedStreamUsageInner(mockNode);
    EXPECT_EQ(inst->collAbsSupportedStreamUsageConfig_[DEVICE_TYPE_BLUETOOTH_A2DP].size(), 2u);
}

// ===== Tests for IsStreamUsageSupported (device not configured) =====

/**
* @tc.name   : Test IsStreamUsageSupported device not in config
* @tc.number : IsStreamUsageSupported_NoDev_001
* @tc.desc   : Device not present in config should return false.
*/
HWTEST(AudioCollaborationManagerUnitTest, IsStreamUsageSupported_NoDev_001, TestSize.Level1)
{
    auto *inst = AudioCollaborationManager::GetInstance();
    inst->collAbsSupportedStreamUsageConfig_.clear();
    inst->collAbsSupportedStreamUsageConfig_[DEVICE_TYPE_BLUETOOTH_A2DP].insert(STREAM_USAGE_MOVIE);
    // NEARLINK device not in config
    EXPECT_FALSE(inst->IsStreamUsageSupported(DEVICE_TYPE_NEARLINK, STREAM_USAGE_MOVIE));
}

/**
* @tc.name   : Test IsStreamUsageSupported stream not in config
* @tc.number : IsStreamUsageSupported_NoUsage_002
* @tc.desc   : Device present but specific stream usage not in config.
*/
HWTEST(AudioCollaborationManagerUnitTest, IsStreamUsageSupported_NoUsage_002, TestSize.Level1)
{
    auto *inst = AudioCollaborationManager::GetInstance();
    inst->collAbsSupportedStreamUsageConfig_.clear();
    inst->collAbsSupportedStreamUsageConfig_[DEVICE_TYPE_BLUETOOTH_A2DP].insert(STREAM_USAGE_MOVIE);
    EXPECT_FALSE(inst->IsStreamUsageSupported(DEVICE_TYPE_BLUETOOTH_A2DP, STREAM_USAGE_GAME));
}

} // namespace AudioStandard
} // namespace OHOS