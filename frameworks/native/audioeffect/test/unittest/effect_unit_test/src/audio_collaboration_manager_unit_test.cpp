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
* @tc.number : LoadCollaborationConfigInner_004
* @tc.desc   : Test LoadCollaborationConfigInner interface(using empty use case).
*/
HWTEST(AudioCollaborationManagerUnitTest, LoadCollaborationConfigInner_004, TestSize.Level1)
{
    auto mockNode = std::make_shared<MockAudioXmlNode>();
    EXPECT_CALL(*mockNode, IsNodeValid()).WillOnce(Return(true)).WillOnce(Return(true)).WillOnce(Return(false));
    EXPECT_CALL(*mockNode, MoveToChildren()).Times(1);
    EXPECT_CALL(*mockNode, IsElementNode()).WillOnce(Return(true));
    EXPECT_CALL(*mockNode, CompareName("product")).WillOnce(Return(false));
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
    EXPECT_CALL(*mockNode, IsNodeValid()).WillOnce(Return(true)).WillOnce(Return(true)).WillOnce(Return(false));
    EXPECT_CALL(*mockNode, MoveToChildren()).Times(1);
    EXPECT_CALL(*mockNode, IsElementNode()).WillOnce(Return(true));
    EXPECT_CALL(*mockNode, CompareName("product")).WillOnce(Return(true));
    EXPECT_CALL(*mockNode, GetProp("id", _)).WillOnce(Return(-1));
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
    EXPECT_CALL(*mockNode, IsNodeValid()).WillOnce(Return(true)).WillOnce(Return(true)).WillOnce(Return(false));
    EXPECT_CALL(*mockNode, MoveToChildren()).Times(1);
    EXPECT_CALL(*mockNode, IsElementNode()).WillOnce(Return(true));
    EXPECT_CALL(*mockNode, CompareName("product")).WillOnce(Return(true));
    EXPECT_CALL(*mockNode, GetProp("id", _)).WillOnce(DoAll(SetArgReferee<1>("testProduct"), Return(SUCCESS)));
    EXPECT_CALL(*mockNode, GetProp("type", _)).WillOnce(Return(-1));
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
    EXPECT_CALL(*mockNode, CompareName("product")).WillOnce(Return(true));
    EXPECT_CALL(*mockNode, GetProp("id", _)).WillOnce(DoAll(SetArgReferee<1>("testProduct"), Return(SUCCESS)));
    EXPECT_CALL(*mockNode, GetProp("type", _)).WillOnce(DoAll(SetArgReferee<1>("10"), Return(SUCCESS)));
    EXPECT_CALL(*mockNode, GetProp("name", _)).WillOnce(Return(-1));
    EXPECT_CALL(*mockNode, MoveToNext()).Times(1);

    AudioCollaborationManager::GetInstance()->LoadCollaborationConfigInner(mockNode);
    EXPECT_TRUE(AudioCollaborationManager::GetInstance()->collaborativeEarphoneNameConfig_.empty());
}

/**
* @tc.name   : Test LoadCollaborationConfigInner API
* @tc.number : LoadCollaborationConfigInner_008
* @tc.desc   : Test LoadCollaborationConfigInner interface(using empty use case).
*/
HWTEST(AudioCollaborationManagerUnitTest, LoadCollaborationConfigInner_008, TestSize.Level1)
{
    std::string prodcutId = "testProduct";
    auto mockNode = std::make_shared<MockAudioXmlNode>();
    auto mockSecondNode = std::make_shared<MockAudioXmlNode>();
    EXPECT_CALL(*mockNode, IsNodeValid()).WillOnce(Return(true)).WillOnce(Return(true)).WillOnce(Return(false));
    EXPECT_CALL(*mockNode, MoveToChildren()).Times(1);
    EXPECT_CALL(*mockNode, IsElementNode()).WillOnce(Return(true));
    EXPECT_CALL(*mockNode, CompareName("product")).WillOnce(Return(true));
    EXPECT_CALL(*mockNode, GetProp("id", _)).WillOnce(DoAll(SetArgReferee<1>("testProduct"), Return(SUCCESS)));
    EXPECT_CALL(*mockNode, GetProp("type", _)).WillOnce(DoAll(SetArgReferee<1>("10"), Return(SUCCESS)));
    EXPECT_CALL(*mockNode, GetProp("name", _)).WillOnce(DoAll(SetArgReferee<1>("test"), Return(SUCCESS)));
    EXPECT_CALL(*mockSecondNode, MoveToChildren()).Times(1);
    EXPECT_CALL(*mockSecondNode, IsNodeValid()).WillOnce(Return(false));
    EXPECT_CALL(*mockNode, GetCopyNode()).Times(1).WillOnce(Return(mockSecondNode));
    EXPECT_CALL(*mockNode, MoveToNext()).Times(1);

    AudioCollaborationManager::GetInstance()->LoadCollaborationConfigInner(mockNode);
    EXPECT_EQ(AudioCollaborationManager::GetInstance()->collaborativeEarphoneProductConfig_[prodcutId], 10);
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
} // namespace AudioStandard
} // namespace OHOS