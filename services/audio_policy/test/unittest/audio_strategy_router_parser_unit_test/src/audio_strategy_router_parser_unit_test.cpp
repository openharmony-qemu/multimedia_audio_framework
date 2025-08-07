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

#include "audio_strategy_router_parser_unit_test.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioStrategyRouterParserUnitTest::SetUp(void)
{
    audioStrategyRouterParser_ = std::make_shared<AudioStrategyRouterParser>();
    ASSERT_TRUE(audioStrategyRouterParser_ != nullptr);

    mockAudioXmlNode_ = std::make_shared<testing::NiceMock<MockAudioXmlNode>>();
    ASSERT_TRUE(mockAudioXmlNode_ != nullptr);

    ON_CALL(*mockAudioXmlNode_, IsNodeValid()).WillByDefault(Return(false));
    ON_CALL(*mockAudioXmlNode_, IsElementNode()).WillByDefault(Return(true));
    ON_CALL(*mockAudioXmlNode_, CompareName(_)).WillByDefault(Return(false));
    ON_CALL(*mockAudioXmlNode_, GetProp(_, _)).WillByDefault(Return(0));
    ON_CALL(*mockAudioXmlNode_, GetCopyNode()).WillByDefault(Return(mockAudioXmlNode_));
    ON_CALL(*mockAudioXmlNode_, GetChildrenNode()).WillByDefault(Return(mockAudioXmlNode_));
}
void AudioStrategyRouterParserUnitTest::TearDown(void)
{
    audioStrategyRouterParser_ = nullptr;
    mockAudioXmlNode_ = nullptr;
}

/**
 * @tc.name  : Test ParserStrategyInfo.
 * @tc.number: AudioStrategyRouterParserUnitTest_001
 * @tc.desc  : Test ParserStrategyInfo interface.
 */
HWTEST_F(AudioStrategyRouterParserUnitTest, AudioStrategyRouterParserUnitTest_001, TestSize.Level1)
{
    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("name"), _))
        .WillOnce(DoAll(SetArgReferee<1>("MEDIA_RENDER"), Return(0)));

    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("routers"), _))
        .WillOnce(DoAll(SetArgReferee<1>("MEDIA_RENDER"), Return(0)));

    audioStrategyRouterParser_->ParserStrategyInfo(mockAudioXmlNode_);
}

/**
 * @tc.name  : Test ParserStrategyInfo.
 * @tc.number: AudioStrategyRouterParserUnitTest_002
 * @tc.desc  : Test ParserStrategyInfo interface.
 */
HWTEST_F(AudioStrategyRouterParserUnitTest, AudioStrategyRouterParserUnitTest_002, TestSize.Level1)
{
    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("name"), _))
        .WillOnce(DoAll(SetArgReferee<1>("CALL_RENDER"), Return(0)));

    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("routers"), _))
        .WillOnce(DoAll(SetArgReferee<1>("CALL_RENDER"), Return(0)));

    audioStrategyRouterParser_->ParserStrategyInfo(mockAudioXmlNode_);
}

/**
 * @tc.name  : Test ParserStrategyInfo.
 * @tc.number: AudioStrategyRouterParserUnitTest_003
 * @tc.desc  : Test ParserStrategyInfo interface.
 */
HWTEST_F(AudioStrategyRouterParserUnitTest, AudioStrategyRouterParserUnitTest_003, TestSize.Level1)
{
    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("name"), _))
        .WillOnce(DoAll(SetArgReferee<1>("RING_RENDER"), Return(0)));

    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("routers"), _))
        .WillOnce(DoAll(SetArgReferee<1>("RING_RENDER"), Return(0)));

    audioStrategyRouterParser_->ParserStrategyInfo(mockAudioXmlNode_);
}

/**
 * @tc.name  : Test ParserStrategyInfo.
 * @tc.number: AudioStrategyRouterParserUnitTest_004
 * @tc.desc  : Test ParserStrategyInfo interface.
 */
HWTEST_F(AudioStrategyRouterParserUnitTest, AudioStrategyRouterParserUnitTest_004, TestSize.Level1)
{
    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("name"), _))
        .WillOnce(DoAll(SetArgReferee<1>("TONE_RENDER"), Return(0)));

    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("routers"), _))
        .WillOnce(DoAll(SetArgReferee<1>("TONE_RENDER"), Return(0)));

    audioStrategyRouterParser_->ParserStrategyInfo(mockAudioXmlNode_);
}

/**
 * @tc.name  : Test ParserStrategyInfo.
 * @tc.number: AudioStrategyRouterParserUnitTest_005
 * @tc.desc  : Test ParserStrategyInfo interface.
 */
HWTEST_F(AudioStrategyRouterParserUnitTest, AudioStrategyRouterParserUnitTest_005, TestSize.Level1)
{
    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("name"), _))
        .WillOnce(DoAll(SetArgReferee<1>("RECORD_CAPTURE"), Return(0)));

    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("routers"), _))
        .WillOnce(DoAll(SetArgReferee<1>("RECORD_CAPTURE"), Return(0)));

    audioStrategyRouterParser_->ParserStrategyInfo(mockAudioXmlNode_);
}

/**
 * @tc.name  : Test ParserStrategyInfo.
 * @tc.number: AudioStrategyRouterParserUnitTest_006
 * @tc.desc  : Test ParserStrategyInfo interface.
 */
HWTEST_F(AudioStrategyRouterParserUnitTest, AudioStrategyRouterParserUnitTest_006, TestSize.Level1)
{
    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("name"), _))
        .WillOnce(DoAll(SetArgReferee<1>("CALL_CAPTURE"), Return(0)));

    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("routers"), _))
        .WillOnce(DoAll(SetArgReferee<1>("CALL_CAPTURE"), Return(0)));

    audioStrategyRouterParser_->ParserStrategyInfo(mockAudioXmlNode_);
}

/**
 * @tc.name  : Test ParserStrategyInfo.
 * @tc.number: AudioStrategyRouterParserUnitTest_007
 * @tc.desc  : Test ParserStrategyInfo interface.
 */
HWTEST_F(AudioStrategyRouterParserUnitTest, AudioStrategyRouterParserUnitTest_007, TestSize.Level1)
{
    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("name"), _))
        .WillOnce(DoAll(SetArgReferee<1>("VOICE_MESSAGE_CAPTURE"), Return(0)));

    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("routers"), _))
        .WillOnce(DoAll(SetArgReferee<1>("VOICE_MESSAGE_CAPTURE"), Return(0)));

    audioStrategyRouterParser_->ParserStrategyInfo(mockAudioXmlNode_);
}

/**
 * @tc.name  : Test ParserStrategyInfo.
 * @tc.number: AudioStrategyRouterParserUnitTest_008
 * @tc.desc  : Test ParserStrategyInfo interface.
 */
HWTEST_F(AudioStrategyRouterParserUnitTest, AudioStrategyRouterParserUnitTest_008, TestSize.Level1)
{
    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("name"), _))
        .WillOnce(DoAll(SetArgReferee<1>("TEST"), Return(0)));

    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("routers"), _))
        .WillOnce(DoAll(SetArgReferee<1>("TEST"), Return(0)));

    audioStrategyRouterParser_->ParserStrategyInfo(mockAudioXmlNode_);
}

/**
 * @tc.name  : Test ParseInternal.
 * @tc.number: AudioStrategyRouterParserUnitTest_009
 * @tc.desc  : Test ParseInternal interface.
 */
HWTEST_F(AudioStrategyRouterParserUnitTest, AudioStrategyRouterParserUnitTest_009, TestSize.Level1)
{
    EXPECT_CALL(*(mockAudioXmlNode_), IsNodeValid()).WillOnce(Return(false));

    bool ret = audioStrategyRouterParser_->ParseInternal(mockAudioXmlNode_);
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name  : Test ParseInternal.
 * @tc.number: AudioStrategyRouterParserUnitTest_010
 * @tc.desc  : Test ParseInternal interface.
 */
HWTEST_F(AudioStrategyRouterParserUnitTest, AudioStrategyRouterParserUnitTest_010, TestSize.Level1)
{
    EXPECT_CALL(*(mockAudioXmlNode_), IsNodeValid())
        .WillOnce(Return(true))
        .WillRepeatedly(Return(false));

    EXPECT_CALL(*(mockAudioXmlNode_), MoveToNext()).Times(1);

    EXPECT_CALL(*(mockAudioXmlNode_), IsElementNode()).WillOnce(Return(false));

    bool ret = audioStrategyRouterParser_->ParseInternal(mockAudioXmlNode_);
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name  : Test ParseInternal.
 * @tc.number: AudioStrategyRouterParserUnitTest_011
 * @tc.desc  : Test ParseInternal interface.
 */
HWTEST_F(AudioStrategyRouterParserUnitTest, AudioStrategyRouterParserUnitTest_011, TestSize.Level1)
{
    EXPECT_CALL(*(mockAudioXmlNode_), IsNodeValid())
        .WillOnce(Return(true))
        .WillRepeatedly(Return(false));

    EXPECT_CALL(*(mockAudioXmlNode_), MoveToNext()).Times(1);
    EXPECT_CALL(*(mockAudioXmlNode_), GetChildrenNode()).WillOnce(Return(mockAudioXmlNode_));

    EXPECT_CALL(*(mockAudioXmlNode_), IsElementNode()).WillOnce(Return(true));

    EXPECT_CALL(*(mockAudioXmlNode_), CompareName(testing::_)).WillOnce(Return(false));

    bool ret = audioStrategyRouterParser_->ParseInternal(mockAudioXmlNode_);
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name  : Test ParseInternal.
 * @tc.number: AudioStrategyRouterParserUnitTest_012
 * @tc.desc  : Test ParseInternal interface.
 */
HWTEST_F(AudioStrategyRouterParserUnitTest, AudioStrategyRouterParserUnitTest_012, TestSize.Level1)
{
    EXPECT_CALL(*(mockAudioXmlNode_), IsNodeValid())
        .WillOnce(Return(true))
        .WillRepeatedly(Return(false));

    EXPECT_CALL(*(mockAudioXmlNode_), IsElementNode()).WillOnce(Return(true));

    EXPECT_CALL(*(mockAudioXmlNode_), MoveToNext()).Times(1);
    EXPECT_CALL(*(mockAudioXmlNode_), GetCopyNode()).WillOnce(Return(mockAudioXmlNode_));
    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(testing::_, testing::_)).Times(2);
    EXPECT_CALL(*(mockAudioXmlNode_), CompareName(testing::_)).WillOnce(Return(true));

    bool ret = audioStrategyRouterParser_->ParseInternal(mockAudioXmlNode_);
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name  : Test split.
 * @tc.number: AudioStrategyRouterParserUnitTest_013
 * @tc.desc  : Test split interface.
 */
HWTEST_F(AudioStrategyRouterParserUnitTest, AudioStrategyRouterParserUnitTest_013, TestSize.Level1)
{
    std::string line = "http//::TestApp";
    std::string sep = "123";
    std::vector<std::string> buf;

    buf = audioStrategyRouterParser_->split(line, sep);
    EXPECT_EQ(false, buf.empty());
}

/**
 * @tc.name  : Test split.
 * @tc.number: AudioStrategyRouterParserUnitTest_014
 * @tc.desc  : Test split interface.
 */
HWTEST_F(AudioStrategyRouterParserUnitTest, AudioStrategyRouterParserUnitTest_014, TestSize.Level1)
{
    std::string line = "http//::TestApp";
    std::string sep = "Test";
    std::vector<std::string> buf;

    buf = audioStrategyRouterParser_->split(line, sep);
    EXPECT_EQ(false, buf.empty());
}

/**
 * @tc.name  : Test AddRouters.
 * @tc.number: AudioStrategyRouterParserUnitTest_015
 * @tc.desc  : Test AddRouters interface.
 */
HWTEST_F(AudioStrategyRouterParserUnitTest, AudioStrategyRouterParserUnitTest_015, TestSize.Level1)
{
    string routeName = "AppSelectRouter";
    std::vector<std::unique_ptr<RouterBase>> routers;

    audioStrategyRouterParser_->AddRouters(routers, routeName);
    EXPECT_EQ(false, routers.empty());
}

/**
 * @tc.name  : Test AddRouters.
 * @tc.number: AudioStrategyRouterParserUnitTest_016
 * @tc.desc  : Test AddRouters interface.
 */
HWTEST_F(AudioStrategyRouterParserUnitTest, AudioStrategyRouterParserUnitTest_016, TestSize.Level1)
{
    string routeName = "UserSelectRouter";
    std::vector<std::unique_ptr<RouterBase>> routers;

    audioStrategyRouterParser_->AddRouters(routers, routeName);
    EXPECT_EQ(false, routers.empty());
}

/**
 * @tc.name  : Test AddRouters.
 * @tc.number: AudioStrategyRouterParserUnitTest_017
 * @tc.desc  : Test AddRouters interface.
 */
HWTEST_F(AudioStrategyRouterParserUnitTest, AudioStrategyRouterParserUnitTest_017, TestSize.Level1)
{
    string routeName = "PrivacyPriorityRouter";
    std::vector<std::unique_ptr<RouterBase>> routers;

    audioStrategyRouterParser_->AddRouters(routers, routeName);
    EXPECT_EQ(false, routers.empty());
}

/**
 * @tc.name  : Test AddRouters.
 * @tc.number: AudioStrategyRouterParserUnitTest_018
 * @tc.desc  : Test AddRouters interface.
 */
HWTEST_F(AudioStrategyRouterParserUnitTest, AudioStrategyRouterParserUnitTest_018, TestSize.Level1)
{
    string routeName = "PublicPriorityRouter";
    std::vector<std::unique_ptr<RouterBase>> routers;

    audioStrategyRouterParser_->AddRouters(routers, routeName);
    EXPECT_EQ(false, routers.empty());
}

/**
 * @tc.name  : Test AddRouters.
 * @tc.number: AudioStrategyRouterParserUnitTest_019
 * @tc.desc  : Test AddRouters interface.
 */
HWTEST_F(AudioStrategyRouterParserUnitTest, AudioStrategyRouterParserUnitTest_019, TestSize.Level1)
{
    string routeName = "StreamFilterRouter";
    std::vector<std::unique_ptr<RouterBase>> routers;

    audioStrategyRouterParser_->AddRouters(routers, routeName);
    EXPECT_EQ(false, routers.empty());
}

/**
 * @tc.name  : Test AddRouters.
 * @tc.number: AudioStrategyRouterParserUnitTest_020
 * @tc.desc  : Test AddRouters interface.
 */
HWTEST_F(AudioStrategyRouterParserUnitTest, AudioStrategyRouterParserUnitTest_020, TestSize.Level1)
{
    string routeName = "DefaultRouter";
    std::vector<std::unique_ptr<RouterBase>> routers;

    audioStrategyRouterParser_->AddRouters(routers, routeName);
    EXPECT_EQ(false, routers.empty());
}

/**
 * @tc.name  : Test AddRouters.
 * @tc.number: AudioStrategyRouterParserUnitTest_021
 * @tc.desc  : Test AddRouters interface.
 */
HWTEST_F(AudioStrategyRouterParserUnitTest, AudioStrategyRouterParserUnitTest_021, TestSize.Level1)
{
    string routeName = "CockpitPhoneRouter";
    std::vector<std::unique_ptr<RouterBase>> routers;

    audioStrategyRouterParser_->AddRouters(routers, routeName);
    EXPECT_EQ(false, routers.empty());
}

/**
 * @tc.name  : Test AddRouters.
 * @tc.number: AudioStrategyRouterParserUnitTest_022
 * @tc.desc  : Test AddRouters interface.
 */
HWTEST_F(AudioStrategyRouterParserUnitTest, AudioStrategyRouterParserUnitTest_022, TestSize.Level1)
{
    string routeName = "PairDeviceRouter";
    std::vector<std::unique_ptr<RouterBase>> routers;

    audioStrategyRouterParser_->AddRouters(routers, routeName);
    EXPECT_EQ(false, routers.empty());
}

/**
 * @tc.name  : Test AddRouters.
 * @tc.number: AudioStrategyRouterParserUnitTest_023
 * @tc.desc  : Test AddRouters interface.
 */
HWTEST_F(AudioStrategyRouterParserUnitTest, AudioStrategyRouterParserUnitTest_023, TestSize.Level1)
{
    string routeName = "Test";
    std::vector<std::unique_ptr<RouterBase>> routers;

    audioStrategyRouterParser_->AddRouters(routers, routeName);
    EXPECT_EQ(true, routers.empty());
}
} // namespace AudioStandard
} // namespace