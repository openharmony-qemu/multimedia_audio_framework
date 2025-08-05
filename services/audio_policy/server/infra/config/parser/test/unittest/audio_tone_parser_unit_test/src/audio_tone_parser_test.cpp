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

#include "audio_tone_parser_test.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioToneParserSecondUnitTest::SetUpTestCase(void) {}
void AudioToneParserSecondUnitTest::TearDownTestCase(void) {}
void AudioToneParserSecondUnitTest::SetUp(void)
{
    audioToneParser_ = std::make_shared<AudioToneParser>();
    ASSERT_TRUE(audioToneParser_ != nullptr);

    mockAudioXmlNode_ = std::make_shared<MockAudioXmlNode>();
    ASSERT_TRUE(mockAudioXmlNode_ != nullptr);
}
void AudioToneParserSecondUnitTest::TearDown(void)
{
    audioToneParser_ = nullptr;
    mockAudioXmlNode_ = nullptr;
}

std::shared_ptr<AudioXmlNode> AudioXmlNode::Create()
{
    return std::make_shared<MockAudioXmlNode>();
}

/**
 * @tc.name  : Test ParseToneInfo.
 * @tc.number: AudioToneParserSecondUnitTest_001
 * @tc.desc  : Test ParseToneInfo interface. !curNode->CompareName("ToneInfo")
 */
HWTEST_F(AudioToneParserSecondUnitTest, AudioToneParserSecondUnitTest_001, TestSize.Level4)
{
    EXPECT_CALL(*(mockAudioXmlNode_), IsNodeValid())
        .WillOnce(Return(true))
        .WillOnce(Return(false));

    EXPECT_CALL(*(mockAudioXmlNode_), IsElementNode()).WillOnce(Return(true));

    EXPECT_CALL(*(mockAudioXmlNode_), CompareName(testing::_)).WillOnce(Return(false));

    EXPECT_CALL(*(mockAudioXmlNode_), MoveToNext()).Times(1);

    std::unordered_map<int32_t, std::shared_ptr<ToneInfo>> map;
    std::shared_ptr<ToneInfo> toneInfo = std::make_shared<ToneInfo>();
    map.insert(std::make_pair(1, toneInfo));
    std::vector<ToneInfoMap*> toneInfoMapArr;
    ToneInfoMap *toneInfoMapPtr = &map;
    toneInfoMapArr.push_back(toneInfoMapPtr);
    audioToneParser_->ParseToneInfo(mockAudioXmlNode_, toneInfoMapArr);
}

/**
 * @tc.name  : Test ParseToneInfo.
 * @tc.number: AudioToneParserSecondUnitTest_002
 * @tc.desc  : Test ParseToneInfo interface. curNode->GetProp("toneType", pToneType) != SUCCESS
 */
HWTEST_F(AudioToneParserSecondUnitTest, AudioToneParserSecondUnitTest_002, TestSize.Level4)
{
    EXPECT_CALL(*(mockAudioXmlNode_), IsNodeValid())
        .WillOnce(Return(true))
        .WillOnce(Return(false));

    EXPECT_CALL(*(mockAudioXmlNode_), IsElementNode()).WillOnce(Return(true));

    EXPECT_CALL(*(mockAudioXmlNode_), CompareName(testing::_)).WillOnce(Return(true));

    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(testing::_, testing::_)).WillOnce(Return(-1));

    EXPECT_CALL(*(mockAudioXmlNode_), MoveToNext()).Times(1);

    std::unordered_map<int32_t, std::shared_ptr<ToneInfo>> map;
    std::shared_ptr<ToneInfo> toneInfo = std::make_shared<ToneInfo>();
    map.insert(std::make_pair(1, toneInfo));
    std::vector<ToneInfoMap*> toneInfoMapArr;
    ToneInfoMap *toneInfoMapPtr = &map;
    toneInfoMapArr.push_back(toneInfoMapPtr);
    audioToneParser_->ParseToneInfo(mockAudioXmlNode_, toneInfoMapArr);
}

/**
 * @tc.name  : Test ParseToneInfo.
 * @tc.number: AudioToneParserSecondUnitTest_003
 * @tc.desc  : Test ParseToneInfo interface. toneDescriptorMap
 */
HWTEST_F(AudioToneParserSecondUnitTest, AudioToneParserSecondUnitTest_003, TestSize.Level4)
{
    EXPECT_CALL(*(mockAudioXmlNode_), IsNodeValid())
        .WillOnce(Return(true))
        .WillOnce(Return(false));

    EXPECT_CALL(*(mockAudioXmlNode_), IsElementNode()).WillOnce(Return(true));

    EXPECT_CALL(*(mockAudioXmlNode_), CompareName(testing::_)).WillOnce(Return(true));

    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("toneType"), _))
        .WillOnce(DoAll(SetArgReferee<1>("-0"), Return(0)));

    EXPECT_CALL(*(mockAudioXmlNode_), GetChildrenNode()).WillOnce(Return(mockAudioXmlNode_));

    EXPECT_CALL(*(mockAudioXmlNode_), MoveToNext()).Times(1);

    std::vector<ToneInfoMap*> toneInfoMapArr;
    ToneInfoMap *toneInfoMapPtr = nullptr;
    toneInfoMapArr.push_back(toneInfoMapPtr);
    audioToneParser_->ParseToneInfo(mockAudioXmlNode_, toneInfoMapArr);
}

/**
 * @tc.name  : Test ParseToneInfoAttribute.
 * @tc.number: AudioToneParserSecondUnitTest_004
 * @tc.desc  : Test ParseToneInfoAttribute interface. curNode->CompareName("Segment") is false
 */
HWTEST_F(AudioToneParserSecondUnitTest, AudioToneParserSecondUnitTest_004, TestSize.Level4)
{
    EXPECT_CALL(*(mockAudioXmlNode_), IsNodeValid())
        .WillOnce(Return(true))
        .WillOnce(Return(false));

    EXPECT_CALL(*(mockAudioXmlNode_), IsElementNode()).WillOnce(Return(true));

    EXPECT_CALL(*(mockAudioXmlNode_), CompareName(testing::_)).WillOnce(Return(false));

    EXPECT_CALL(*(mockAudioXmlNode_), MoveToNext()).Times(1);

    std::shared_ptr<ToneInfo> toneInfo = std::make_shared<ToneInfo>();
    audioToneParser_->ParseToneInfoAttribute(mockAudioXmlNode_, toneInfo);
}

/**
 * @tc.name  : Test ParseToneInfoAttribute.
 * @tc.number: AudioToneParserSecondUnitTest_005
 * @tc.desc  : Test ParseToneInfoAttribute interface. segInx >= segCnt
 */
HWTEST_F(AudioToneParserSecondUnitTest, AudioToneParserSecondUnitTest_005, TestSize.Level4)
{
    EXPECT_CALL(*(mockAudioXmlNode_), IsNodeValid())
        .WillOnce(Return(true))
        .WillOnce(Return(false));

    EXPECT_CALL(*(mockAudioXmlNode_), IsElementNode()).WillOnce(Return(true));

    EXPECT_CALL(*(mockAudioXmlNode_), CompareName(StrEq("RepeatCount"))).WillOnce(Return(false));
    EXPECT_CALL(*(mockAudioXmlNode_), CompareName(StrEq("RepeatSegment"))).WillOnce(Return(false));
    EXPECT_CALL(*(mockAudioXmlNode_), CompareName(StrEq("SegmentCount"))).WillOnce(Return(false));
    EXPECT_CALL(*(mockAudioXmlNode_), CompareName(StrEq("Segment"))).WillOnce(Return(true));

    EXPECT_CALL(*(mockAudioXmlNode_), MoveToNext()).Times(1);

    std::shared_ptr<ToneInfo> toneInfo = std::make_shared<ToneInfo>();
    audioToneParser_->ParseToneInfoAttribute(mockAudioXmlNode_, toneInfo);
}

/**
 * @tc.name  : Test ParseCustom.
 * @tc.number: AudioToneParserSecondUnitTest_006
 * @tc.desc  : Test ParseCustom interface. curNode->CompareName("CountryInfo") is false
 */
HWTEST_F(AudioToneParserSecondUnitTest, AudioToneParserSecondUnitTest_006, TestSize.Level4)
{
    EXPECT_CALL(*(mockAudioXmlNode_), IsNodeValid())
        .WillOnce(Return(true))
        .WillOnce(Return(false));

    EXPECT_CALL(*(mockAudioXmlNode_), IsElementNode()).WillOnce(Return(true));

    EXPECT_CALL(*(mockAudioXmlNode_), CompareName(testing::_)).WillOnce(Return(false));

    EXPECT_CALL(*(mockAudioXmlNode_), MoveToNext()).Times(1);

    std::unordered_map<int32_t, std::shared_ptr<ToneInfo>> map;
    std::shared_ptr<ToneInfo> toneInfo = std::make_shared<ToneInfo>();
    map.insert(std::make_pair(1, toneInfo));
    std::string mapStr = "DTMF";
    std::unordered_map<std::string, ToneInfoMap> toneInfoMap;
    toneInfoMap.insert(std::make_pair(mapStr, map));
    audioToneParser_->ParseCustom(mockAudioXmlNode_, toneInfoMap);
}

/**
 * @tc.name  : Test ParseCustom.
 * @tc.number: AudioToneParserSecondUnitTest_007
 * @tc.desc  : Test ParseCustom interface. ret != SUCCESS
 */
HWTEST_F(AudioToneParserSecondUnitTest, AudioToneParserSecondUnitTest_007, TestSize.Level4)
{
    EXPECT_CALL(*(mockAudioXmlNode_), IsNodeValid())
        .WillOnce(Return(true))
        .WillOnce(Return(false));

    EXPECT_CALL(*(mockAudioXmlNode_), IsElementNode()).WillOnce(Return(true));

    EXPECT_CALL(*(mockAudioXmlNode_), CompareName(testing::_)).WillOnce(Return(true));

    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(testing::_, testing::_)).WillOnce(Return(ERROR));

    EXPECT_CALL(*(mockAudioXmlNode_), MoveToNext()).Times(1);

    std::unordered_map<int32_t, std::shared_ptr<ToneInfo>> map;
    std::shared_ptr<ToneInfo> toneInfo = std::make_shared<ToneInfo>();
    map.insert(std::make_pair(1, toneInfo));
    std::string mapStr = "DTMF";
    std::unordered_map<std::string, ToneInfoMap> toneInfoMap;
    toneInfoMap.insert(std::make_pair(mapStr, map));
    audioToneParser_->ParseCustom(mockAudioXmlNode_, toneInfoMap);
}

/**
 * @tc.name  : Test SplitAndTrim.
 * @tc.number: AudioToneParserSecondUnitTest_009
 * @tc.desc  : Test SplitAndTrim interface. !trimmedToken.empty()
 */
HWTEST_F(AudioToneParserSecondUnitTest, AudioToneParserSecondUnitTest_009, TestSize.Level4)
{
    EXPECT_CALL(*(mockAudioXmlNode_), IsNodeValid())
        .WillOnce(Return(true))
        .WillOnce(Return(false));

    EXPECT_CALL(*(mockAudioXmlNode_), IsElementNode()).WillOnce(Return(true));

    EXPECT_CALL(*(mockAudioXmlNode_), CompareName(testing::_)).WillOnce(Return(true));

    std::string input = "  ,\t\n,\r  ,   ";
    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("names"), _))
        .WillOnce(DoAll(SetArgReferee<1>(input), Return(0)));

    EXPECT_CALL(*(mockAudioXmlNode_), MoveToNext()).Times(1);
    EXPECT_CALL(*(mockAudioXmlNode_), GetChildrenNode()).WillOnce(Return(mockAudioXmlNode_));

    std::unordered_map<int32_t, std::shared_ptr<ToneInfo>> map;
    std::shared_ptr<ToneInfo> toneInfo = std::make_shared<ToneInfo>();
    map.insert(std::make_pair(1, toneInfo));
    std::string mapStr = "DTMF";
    std::unordered_map<std::string, ToneInfoMap> toneInfoMap;
    toneInfoMap.insert(std::make_pair(mapStr, map));
    audioToneParser_->ParseCustom(mockAudioXmlNode_, toneInfoMap);
}

/**
 * @tc.name  : Test SplitAndTrim.
 * @tc.number: AudioToneParserSecondUnitTest_010
 * @tc.desc  : Test SplitAndTrim interface. result.empty()
 */
HWTEST_F(AudioToneParserSecondUnitTest, AudioToneParserSecondUnitTest_010, TestSize.Level4)
{
    EXPECT_CALL(*(mockAudioXmlNode_), IsNodeValid())
        .WillOnce(Return(true))
        .WillOnce(Return(false));

    EXPECT_CALL(*(mockAudioXmlNode_), IsElementNode()).WillOnce(Return(true));

    EXPECT_CALL(*(mockAudioXmlNode_), CompareName(testing::_)).WillOnce(Return(true));

    std::string input = "";
    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("names"), _))
        .WillOnce(DoAll(SetArgReferee<1>(input), Return(0)));

    EXPECT_CALL(*(mockAudioXmlNode_), MoveToNext()).Times(1);
    EXPECT_CALL(*(mockAudioXmlNode_), GetChildrenNode()).WillOnce(Return(mockAudioXmlNode_));

    std::unordered_map<int32_t, std::shared_ptr<ToneInfo>> map;
    std::shared_ptr<ToneInfo> toneInfo = std::make_shared<ToneInfo>();
    map.insert(std::make_pair(1, toneInfo));
    std::string mapStr = "DTMF";
    std::unordered_map<std::string, ToneInfoMap> toneInfoMap;
    toneInfoMap.insert(std::make_pair(mapStr, map));
    audioToneParser_->ParseCustom(mockAudioXmlNode_, toneInfoMap);
}

/**
 * @tc.name  : Test Trim.
 * @tc.number: AudioToneParserSecondUnitTest_011
 * @tc.desc  : Test Trim interface. result.empty()
 */
HWTEST_F(AudioToneParserSecondUnitTest, AudioToneParserSecondUnitTest_011, TestSize.Level4)
{
    EXPECT_CALL(*(mockAudioXmlNode_), IsNodeValid())
        .WillOnce(Return(true))
        .WillOnce(Return(false));

    EXPECT_CALL(*(mockAudioXmlNode_), IsElementNode()).WillOnce(Return(true));

    EXPECT_CALL(*(mockAudioXmlNode_), CompareName(testing::_)).WillOnce(Return(true));

    std::string input = "  hello  ,  ,\t,\rworld  ";
    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("names"), _))
        .WillOnce(DoAll(SetArgReferee<1>(input), Return(0)));

    EXPECT_CALL(*(mockAudioXmlNode_), MoveToNext()).Times(1);
    EXPECT_CALL(*(mockAudioXmlNode_), GetChildrenNode()).WillOnce(Return(mockAudioXmlNode_));

    std::unordered_map<int32_t, std::shared_ptr<ToneInfo>> map;
    std::shared_ptr<ToneInfo> toneInfo = std::make_shared<ToneInfo>();
    map.insert(std::make_pair(1, toneInfo));
    std::string mapStr = "DTMF";
    std::unordered_map<std::string, ToneInfoMap> toneInfoMap;
    toneInfoMap.insert(std::make_pair(mapStr, map));
    audioToneParser_->ParseCustom(mockAudioXmlNode_, toneInfoMap);
}

/**
 * @tc.name  : Test Trim.
 * @tc.number: AudioToneParserSecondUnitTest_012
 * @tc.desc  : Test Trim interface. result.empty()
 */
HWTEST_F(AudioToneParserSecondUnitTest, AudioToneParserSecondUnitTest_012, TestSize.Level4)
{
    EXPECT_CALL(*(mockAudioXmlNode_), IsNodeValid())
        .WillOnce(Return(true))
        .WillOnce(Return(false));

    EXPECT_CALL(*(mockAudioXmlNode_), IsElementNode()).WillOnce(Return(true));

    EXPECT_CALL(*(mockAudioXmlNode_), CompareName(testing::_)).WillOnce(Return(true));

    std::string input = "   \t\n\r   ";
    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("names"), _))
        .WillOnce(DoAll(SetArgReferee<1>(input), Return(0)));

    EXPECT_CALL(*(mockAudioXmlNode_), MoveToNext()).Times(1);
    EXPECT_CALL(*(mockAudioXmlNode_), GetChildrenNode()).WillOnce(Return(mockAudioXmlNode_));

    std::unordered_map<int32_t, std::shared_ptr<ToneInfo>> map;
    std::shared_ptr<ToneInfo> toneInfo = std::make_shared<ToneInfo>();
    map.insert(std::make_pair(1, toneInfo));
    std::string mapStr = "DTMF";
    std::unordered_map<std::string, ToneInfoMap> toneInfoMap;
    toneInfoMap.insert(std::make_pair(mapStr, map));
    audioToneParser_->ParseCustom(mockAudioXmlNode_, toneInfoMap);
}

/**
 * @tc.name  : Test Trim.
 * @tc.number: AudioToneParserSecondUnitTest_013
 * @tc.desc  : Test Trim interface. result.empty()
 */
HWTEST_F(AudioToneParserSecondUnitTest, AudioToneParserSecondUnitTest_013, TestSize.Level4)
{
    EXPECT_CALL(*(mockAudioXmlNode_), IsNodeValid())
        .WillOnce(Return(true))
        .WillOnce(Return(false));

    EXPECT_CALL(*(mockAudioXmlNode_), IsElementNode()).WillOnce(Return(true));

    EXPECT_CALL(*(mockAudioXmlNode_), CompareName(testing::_)).WillOnce(Return(true));

    std::string input = ",test,  ,example,";
    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("names"), _))
        .WillOnce(DoAll(SetArgReferee<1>(input), Return(0)));

    EXPECT_CALL(*(mockAudioXmlNode_), MoveToNext()).Times(1);
    EXPECT_CALL(*(mockAudioXmlNode_), GetChildrenNode()).WillOnce(Return(mockAudioXmlNode_));

    std::unordered_map<int32_t, std::shared_ptr<ToneInfo>> map;
    std::shared_ptr<ToneInfo> toneInfo = std::make_shared<ToneInfo>();
    map.insert(std::make_pair(1, toneInfo));
    std::string mapStr = "DTMF";
    std::unordered_map<std::string, ToneInfoMap> toneInfoMap;
    toneInfoMap.insert(std::make_pair(mapStr, map));
    audioToneParser_->ParseCustom(mockAudioXmlNode_, toneInfoMap);
}
} // namespace AudioStandard
} // namespace OHOSu