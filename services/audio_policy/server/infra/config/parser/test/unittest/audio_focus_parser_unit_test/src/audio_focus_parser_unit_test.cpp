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

#include "audio_focus_parser_unit_test.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioFocusParserUnitTest::SetUpTestCase(void) {}
void AudioFocusParserUnitTest::TearDownTestCase(void) {}
void AudioFocusParserUnitTest::SetUp(void)
{
    audioFocusParser_ = std::make_shared<AudioFocusParser>();
    ASSERT_TRUE(audioFocusParser_ != nullptr);

    mockAudioXmlNode_ = std::make_shared<MockAudioXmlNode>();
    ASSERT_TRUE(mockAudioXmlNode_ != nullptr);
}
void AudioFocusParserUnitTest::TearDown(void)
{
    audioFocusParser_ = nullptr;
    mockAudioXmlNode_ = nullptr;
}

/**
 * @tc.name  : Test LoadConfig.
 * @tc.number: AudioFocusParserUnitTest_001
 * @tc.desc  : Test LoadConfig interface.
 */
HWTEST_F(AudioFocusParserUnitTest, AudioFocusParserUnitTest_001, TestSize.Level4)
{
    audioFocusParser_->curNode_ = mockAudioXmlNode_;
    EXPECT_CALL(*(mockAudioXmlNode_), Config(testing::_, testing::_, testing::_)).WillOnce(Return(ERROR));

    std::map<std::pair<AudioFocusType, AudioFocusType>, AudioFocusEntry> map;
    int32_t ret = audioFocusParser_->LoadConfig(map);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test LoadConfig.
 * @tc.number: AudioFocusParserUnitTest_002
 * @tc.desc  : Test LoadConfig interface.
 */
HWTEST_F(AudioFocusParserUnitTest, AudioFocusParserUnitTest_002, TestSize.Level4)
{
    audioFocusParser_->curNode_ = mockAudioXmlNode_;
    EXPECT_CALL(*(mockAudioXmlNode_), Config(testing::_, testing::_, testing::_)).WillOnce(Return(SUCCESS));

    EXPECT_CALL(*(mockAudioXmlNode_), IsNodeValid()).WillOnce(Return(true));

    EXPECT_CALL(*(mockAudioXmlNode_), CompareName(testing::_)).WillOnce(Return(false));

    std::map<std::pair<AudioFocusType, AudioFocusType>, AudioFocusEntry> map;
    int32_t ret = audioFocusParser_->LoadConfig(map);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test LoadConfig.
 * @tc.number: AudioFocusParserUnitTest_003
 * @tc.desc  : Test LoadConfig interface.
 */
HWTEST_F(AudioFocusParserUnitTest, AudioFocusParserUnitTest_003, TestSize.Level4)
{
    audioFocusParser_->curNode_ = mockAudioXmlNode_;
    EXPECT_CALL(*(mockAudioXmlNode_), Config(testing::_, testing::_, testing::_)).WillOnce(Return(SUCCESS));

    EXPECT_CALL(*(mockAudioXmlNode_), IsNodeValid())
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(false));

    EXPECT_CALL(*(mockAudioXmlNode_), CompareName(testing::_)).WillOnce(Return(true));
    EXPECT_CALL(*(mockAudioXmlNode_), MoveToChildren()).Times(1);

    std::map<std::pair<AudioFocusType, AudioFocusType>, AudioFocusEntry> map;
    int32_t ret = audioFocusParser_->LoadConfig(map);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test ParseStreams.
 * @tc.number: AudioFocusParserUnitTest_004
 * @tc.desc  : Test ParseStreams interface.
 */
HWTEST_F(AudioFocusParserUnitTest, AudioFocusParserUnitTest_004, TestSize.Level4)
{
    audioFocusParser_->curNode_ = mockAudioXmlNode_;

    EXPECT_CALL(*(mockAudioXmlNode_), IsNodeValid())
        .WillOnce(Return(true))
        .WillOnce(Return(false));

    EXPECT_CALL(*(mockAudioXmlNode_), IsElementNode()).WillOnce(Return(true));

    std::string input = "test";
    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("value"), _))
        .WillOnce(DoAll(SetArgReferee<1>(input), Return(0)));

    EXPECT_CALL(*(mockAudioXmlNode_), MoveToNext()).Times(1);

    std::map<std::pair<AudioFocusType, AudioFocusType>, AudioFocusEntry> map;
    audioFocusParser_->ParseStreams(mockAudioXmlNode_, map);
}

/**
 * @tc.name  : Test AddRejectedFocusEntry.
 * @tc.number: AudioFocusParserUnitTest_005
 * @tc.desc  : Test AddRejectedFocusEntry interface.
 */
HWTEST_F(AudioFocusParserUnitTest, AudioFocusParserUnitTest_005, TestSize.Level4)
{
    audioFocusParser_->curNode_ = mockAudioXmlNode_;

    std::string input = "test";
    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("value"), _))
        .WillOnce(DoAll(SetArgReferee<1>(input), Return(0)));

    std::map<std::pair<AudioFocusType, AudioFocusType>, AudioFocusEntry> map;
    audioFocusParser_->AddRejectedFocusEntry(mockAudioXmlNode_, input, map);
}

/**
 * @tc.name  : Test AddAllowedFocusEntry.
 * @tc.number: AudioFocusParserUnitTest_006
 * @tc.desc  : Test AddAllowedFocusEntry interface. it1 == audioFocusMap.end()
 */
HWTEST_F(AudioFocusParserUnitTest, AudioFocusParserUnitTest_006, TestSize.Level4)
{
    audioFocusParser_->curNode_ = mockAudioXmlNode_;

    std::string input = "test";
    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("value"), _))
        .WillOnce(DoAll(SetArgReferee<1>(input), Return(0)));

    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("action_on"), _))
        .WillOnce(DoAll(SetArgReferee<1>(input), Return(0)));

    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("action_type"), _))
        .WillOnce(DoAll(SetArgReferee<1>(input), Return(0)));

    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("is_forced"), _))
        .WillOnce(DoAll(SetArgReferee<1>(input), Return(0)));

    std::map<std::pair<AudioFocusType, AudioFocusType>, AudioFocusEntry> map;
    audioFocusParser_->AddAllowedFocusEntry(mockAudioXmlNode_, input, map);
}

/**
 * @tc.name  : Test AddAllowedFocusEntry.
 * @tc.number: AudioFocusParserUnitTest_007
 * @tc.desc  : Test AddAllowedFocusEntry interface.it2 == targetMap.end()
 */
HWTEST_F(AudioFocusParserUnitTest, AudioFocusParserUnitTest_007, TestSize.Level4)
{
    audioFocusParser_->curNode_ = mockAudioXmlNode_;

    std::string newStreamStr = "STREAM_MUSIC";
    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("value"), _))
        .WillOnce(DoAll(SetArgReferee<1>(newStreamStr), Return(0)));

    std::string input = "test";
    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("action_on"), _))
        .WillOnce(DoAll(SetArgReferee<1>(input), Return(0)));

    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("action_type"), _))
        .WillOnce(DoAll(SetArgReferee<1>(input), Return(0)));

    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("is_forced"), _))
        .WillOnce(DoAll(SetArgReferee<1>(input), Return(0)));

    std::map<std::pair<AudioFocusType, AudioFocusType>, AudioFocusEntry> map;
    audioFocusParser_->AddAllowedFocusEntry(mockAudioXmlNode_, input, map);
}

/**
 * @tc.name  : Test AddAllowedFocusEntry.
 * @tc.number: AudioFocusParserUnitTest_008
 * @tc.desc  : Test AddAllowedFocusEntry interface.it3 == actionMap.end()
 */
HWTEST_F(AudioFocusParserUnitTest, AudioFocusParserUnitTest_008, TestSize.Level4)
{
    audioFocusParser_->curNode_ = mockAudioXmlNode_;

    std::string newStreamStr = "STREAM_MUSIC";
    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("value"), _))
        .WillOnce(DoAll(SetArgReferee<1>(newStreamStr), Return(0)));

    std::string aTargetStr = "existing";
    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("action_on"), _))
        .WillOnce(DoAll(SetArgReferee<1>(aTargetStr), Return(0)));

    std::string input = "test";
    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("action_type"), _))
        .WillOnce(DoAll(SetArgReferee<1>(input), Return(0)));

    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("is_forced"), _))
        .WillOnce(DoAll(SetArgReferee<1>(input), Return(0)));

    std::map<std::pair<AudioFocusType, AudioFocusType>, AudioFocusEntry> map;
    audioFocusParser_->AddAllowedFocusEntry(mockAudioXmlNode_, input, map);
}

/**
 * @tc.name  : Test AddAllowedFocusEntry.
 * @tc.number: AudioFocusParserUnitTest_009
 * @tc.desc  : Test AddAllowedFocusEntry interface.it4 == forceMap.end()
 */
HWTEST_F(AudioFocusParserUnitTest, AudioFocusParserUnitTest_009, TestSize.Level4)
{
    audioFocusParser_->curNode_ = mockAudioXmlNode_;

    std::string newStreamStr = "STREAM_MUSIC";
    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("value"), _))
        .WillOnce(DoAll(SetArgReferee<1>(newStreamStr), Return(0)));

    std::string aTargetStr = "existing";
    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("action_on"), _))
        .WillOnce(DoAll(SetArgReferee<1>(aTargetStr), Return(0)));

    std::string aTypeStr = "PAUSE";
    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("action_type"), _))
        .WillOnce(DoAll(SetArgReferee<1>(aTypeStr), Return(0)));
    
    std::string input = "test";
    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("is_forced"), _))
        .WillOnce(DoAll(SetArgReferee<1>(input), Return(0)));

    std::map<std::pair<AudioFocusType, AudioFocusType>, AudioFocusEntry> map;
    audioFocusParser_->AddAllowedFocusEntry(mockAudioXmlNode_, input, map);
}

/**
 * @tc.name  : Test AddAllowedFocusEntry.
 * @tc.number: AudioFocusParserUnitTest_010
 * @tc.desc  : Test AddAllowedFocusEntry interface.it4 == forceMap.end()
 */
HWTEST_F(AudioFocusParserUnitTest, AudioFocusParserUnitTest_010, TestSize.Level4)
{
    audioFocusParser_->curNode_ = mockAudioXmlNode_;

    std::string newStreamStr = "STREAM_MUSIC";
    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("value"), _))
        .WillOnce(DoAll(SetArgReferee<1>(newStreamStr), Return(0)));

    std::string aTargetStr = "existing";
    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("action_on"), _))
        .WillOnce(DoAll(SetArgReferee<1>(aTargetStr), Return(0)));

    std::string aTypeStr = "PAUSE";
    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("action_type"), _))
        .WillOnce(DoAll(SetArgReferee<1>(aTypeStr), Return(0)));
    
    std::string input = "test";
    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("is_forced"), _))
        .WillOnce(DoAll(SetArgReferee<1>(input), Return(0)));

    std::map<std::pair<AudioFocusType, AudioFocusType>, AudioFocusEntry> map;
    audioFocusParser_->AddAllowedFocusEntry(mockAudioXmlNode_, input, map);
}

/**
 * @tc.name  : Test AddAllowedFocusEntry.
 * @tc.number: AudioFocusParserUnitTest_011
 * @tc.desc  : Test AddAllowedFocusEntry interface.it4 == forceMap.end()
 */
HWTEST_F(AudioFocusParserUnitTest, AudioFocusParserUnitTest_011, TestSize.Level4)
{
    audioFocusParser_->curNode_ = mockAudioXmlNode_;

    std::string newStreamStr = "STREAM_MUSIC";
    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("value"), _))
        .WillOnce(DoAll(SetArgReferee<1>(newStreamStr), Return(0)));

    std::string aTargetStr = "existing";
    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("action_on"), _))
        .WillOnce(DoAll(SetArgReferee<1>(aTargetStr), Return(0)));

    std::string aTypeStr = "PAUSE";
    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("action_type"), _))
        .WillOnce(DoAll(SetArgReferee<1>(aTypeStr), Return(0)));
    
    std::string isForcedStr = "true";
    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("is_forced"), _))
        .WillOnce(DoAll(SetArgReferee<1>(isForcedStr), Return(0)));

    std::string input = "test";
    std::map<std::pair<AudioFocusType, AudioFocusType>, AudioFocusEntry> map;
    audioFocusParser_->AddAllowedFocusEntry(mockAudioXmlNode_, input, map);
}
} // namespace AudioStandard
} // namespace OHOSu