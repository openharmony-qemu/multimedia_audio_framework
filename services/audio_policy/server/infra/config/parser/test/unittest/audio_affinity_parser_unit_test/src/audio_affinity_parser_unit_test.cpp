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

#include "audio_affinity_parser_unit_test.h"
#include "audio_errors.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioAffinityParserUnitTest::SetUp(void)
{
    AudioAffinityManager *affinityManager = &AudioAffinityManager::GetAudioAffinityManager();
    ASSERT_TRUE(affinityManager != nullptr);

    audioAffinityParser_ = std::make_shared<AudioAffinityParser>(nullptr);
    ASSERT_TRUE(audioAffinityParser_ != nullptr);

    mockAudioXmlNode_ = std::make_shared<MockAudioXmlNode>();
    ASSERT_TRUE(mockAudioXmlNode_ != nullptr);

    audioAffinityParser_->curNode_ = mockAudioXmlNode_;
    ASSERT_TRUE(audioAffinityParser_->curNode_ != nullptr);

    audioAffinityParser_->audioAffinityManager_ = &AudioAffinityManager::GetAudioAffinityManager();
    ASSERT_TRUE(audioAffinityParser_->audioAffinityManager_ != nullptr);
}
void AudioAffinityParserUnitTest::TearDown(void)
{
    EXPECT_CALL(*(mockAudioXmlNode_), FreeDoc()).Times(1);
    audioAffinityParser_ = nullptr;
    mockAudioXmlNode_ = nullptr;
}

/**
 * @tc.name  : Test LoadConfiguration.
 * @tc.number: AudioAffinityParserUnitTest_001
 * @tc.desc  : Test LoadConfiguration interface.
 */
HWTEST_F(AudioAffinityParserUnitTest, AudioAffinityParserUnitTest_001, TestSize.Level4)
{
    EXPECT_CALL(*(mockAudioXmlNode_), Config(testing::_, testing::_, testing::_)).WillOnce(Return(SUCCESS));

    EXPECT_CALL(*(mockAudioXmlNode_), MoveToChildren()).Times(1);

    EXPECT_CALL(*(mockAudioXmlNode_), IsNodeValid())
        .WillOnce(Return(true))
        .WillOnce(Return(false));

    EXPECT_CALL(*(mockAudioXmlNode_), GetCopyNode()).WillOnce(Return(mockAudioXmlNode_));

    bool ret = audioAffinityParser_->LoadConfiguration();
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name  : Test ParseInternal.
 * @tc.number: AudioAffinityParserUnitTest_002
 * @tc.desc  : Test ParseInternal interface.
 */
HWTEST_F(AudioAffinityParserUnitTest, AudioAffinityParserUnitTest_002, TestSize.Level4)
{
    EXPECT_CALL(*(mockAudioXmlNode_), IsNodeValid())
        .WillOnce(Return(false));

    bool ret = audioAffinityParser_->ParseInternal(mockAudioXmlNode_);
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name  : Test ParseInternal.
 * @tc.number: AudioAffinityParserUnitTest_003
 * @tc.desc  : Test ParseInternal interface.
 */
HWTEST_F(AudioAffinityParserUnitTest, AudioAffinityParserUnitTest_003, TestSize.Level4)
{
    EXPECT_CALL(*(mockAudioXmlNode_), IsNodeValid())
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(false))
        .WillOnce(Return(false));

    EXPECT_CALL(*(mockAudioXmlNode_), CompareName(StrEq("OutputDevices"))).WillOnce(Return(true));

    EXPECT_CALL(*(mockAudioXmlNode_), MoveToNext()).Times(1);
    EXPECT_CALL(*(mockAudioXmlNode_), MoveToChildren()).Times(1);
    EXPECT_CALL(*(mockAudioXmlNode_), GetCopyNode()).WillOnce(Return(mockAudioXmlNode_));

    bool ret = audioAffinityParser_->ParseInternal(mockAudioXmlNode_);
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name  : Test ParseInternal.
 * @tc.number: AudioAffinityParserUnitTest_004
 * @tc.desc  : Test ParseInternal interface.
 */
HWTEST_F(AudioAffinityParserUnitTest, AudioAffinityParserUnitTest_004, TestSize.Level4)
{
    EXPECT_CALL(*(mockAudioXmlNode_), IsNodeValid())
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(false))
        .WillOnce(Return(false));

    EXPECT_CALL(*(mockAudioXmlNode_), CompareName(StrEq("OutputDevices"))).WillOnce(Return(false));
    EXPECT_CALL(*(mockAudioXmlNode_), CompareName(StrEq("InputDevices"))).WillOnce(Return(true));

    EXPECT_CALL(*(mockAudioXmlNode_), MoveToNext()).Times(1);
    EXPECT_CALL(*(mockAudioXmlNode_), MoveToChildren()).Times(1);
    EXPECT_CALL(*(mockAudioXmlNode_), GetCopyNode()).WillOnce(Return(mockAudioXmlNode_));

    bool ret = audioAffinityParser_->ParseInternal(mockAudioXmlNode_);
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name  : Test ParseInternal.
 * @tc.number: AudioAffinityParserUnitTest_005
 * @tc.desc  : Test ParseInternal interface.
 */
HWTEST_F(AudioAffinityParserUnitTest, AudioAffinityParserUnitTest_005, TestSize.Level4)
{
    EXPECT_CALL(*(mockAudioXmlNode_), IsNodeValid())
        .WillOnce(Return(true))
        .WillOnce(Return(false));

    EXPECT_CALL(*(mockAudioXmlNode_), CompareName(StrEq("OutputDevices"))).WillOnce(Return(false));
    EXPECT_CALL(*(mockAudioXmlNode_), CompareName(StrEq("InputDevices"))).WillOnce(Return(false));

    EXPECT_CALL(*(mockAudioXmlNode_), MoveToNext()).Times(1);

    bool ret = audioAffinityParser_->ParseInternal(mockAudioXmlNode_);
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name  : Test ParserAffinityGroups.
 * @tc.number: AudioAffinityParserUnitTest_006
 * @tc.desc  : Test ParserAffinityGroups interface.
 */
HWTEST_F(AudioAffinityParserUnitTest, AudioAffinityParserUnitTest_006, TestSize.Level4)
{
    DeviceFlag deviceFlag = INPUT_DEVICES_FLAG;

    EXPECT_CALL(*(mockAudioXmlNode_), IsNodeValid())
        .WillOnce(Return(true))
        .WillOnce(Return(false));

    EXPECT_CALL(*(mockAudioXmlNode_), MoveToChildren()).Times(1);

    audioAffinityParser_->ParserAffinityGroups(mockAudioXmlNode_, deviceFlag);
}

/**
 * @tc.name  : Test ParserAffinityGroups.
 * @tc.number: AudioAffinityParserUnitTest_007
 * @tc.desc  : Test ParserAffinityGroups interface.
 */
HWTEST_F(AudioAffinityParserUnitTest, AudioAffinityParserUnitTest_007, TestSize.Level4)
{
    DeviceFlag deviceFlag = INPUT_DEVICES_FLAG;

    EXPECT_CALL(*(mockAudioXmlNode_), IsNodeValid())
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(false));

    EXPECT_CALL(*(mockAudioXmlNode_), MoveToChildren()).Times(1);
    EXPECT_CALL(*(mockAudioXmlNode_), CompareName(StrEq("AffinityGroups"))).WillOnce(Return(false));
    EXPECT_CALL(*(mockAudioXmlNode_), MoveToNext()).Times(1);

    audioAffinityParser_->ParserAffinityGroups(mockAudioXmlNode_, deviceFlag);
}

/**
 * @tc.name  : Test ParserAffinityGroups.
 * @tc.number: AudioAffinityParserUnitTest_008
 * @tc.desc  : Test ParserAffinityGroups interface.
 */
HWTEST_F(AudioAffinityParserUnitTest, AudioAffinityParserUnitTest_008, TestSize.Level4)
{
    DeviceFlag deviceFlag = INPUT_DEVICES_FLAG;

    EXPECT_CALL(*(mockAudioXmlNode_), IsNodeValid())
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(false))
        .WillOnce(Return(false));

    EXPECT_CALL(*(mockAudioXmlNode_), MoveToChildren()).Times(2);
    EXPECT_CALL(*(mockAudioXmlNode_), CompareName(StrEq("AffinityGroups"))).WillOnce(Return(true));
    EXPECT_CALL(*(mockAudioXmlNode_), MoveToNext()).Times(1);
    EXPECT_CALL(*(mockAudioXmlNode_), GetCopyNode()).WillOnce(Return(mockAudioXmlNode_));

    audioAffinityParser_->ParserAffinityGroups(mockAudioXmlNode_, deviceFlag);
}

/**
 * @tc.name  : Test ParserAffinityGroupAttribute.
 * @tc.number: AudioAffinityParserUnitTest_009
 * @tc.desc  : Test ParserAffinityGroupAttribute interface.
 */
HWTEST_F(AudioAffinityParserUnitTest, AudioAffinityParserUnitTest_009, TestSize.Level4)
{
    DeviceFlag deviceFlag = INPUT_DEVICES_FLAG;
    EXPECT_CALL(*(mockAudioXmlNode_), IsNodeValid())
        .WillOnce(Return(true))
        .WillOnce(Return(false));

    EXPECT_CALL(*(mockAudioXmlNode_), MoveToChildren()).Times(1);

    audioAffinityParser_->ParserAffinityGroupAttribute(mockAudioXmlNode_, deviceFlag);
}

/**
 * @tc.name  : Test ParserAffinityGroupAttribute.
 * @tc.number: AudioAffinityParserUnitTest_010
 * @tc.desc  : Test ParserAffinityGroupAttribute interface.
 */
HWTEST_F(AudioAffinityParserUnitTest, AudioAffinityParserUnitTest_010, TestSize.Level4)
{
    DeviceFlag deviceFlag = INPUT_DEVICES_FLAG;

    EXPECT_CALL(*(mockAudioXmlNode_), IsNodeValid())
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(false));

    EXPECT_CALL(*(mockAudioXmlNode_), MoveToChildren()).Times(1);

    EXPECT_CALL(*(mockAudioXmlNode_), CompareName(StrEq("AffinityGroup"))).WillOnce(Return(false));
    EXPECT_CALL(*(mockAudioXmlNode_), MoveToNext()).Times(1);

    audioAffinityParser_->ParserAffinityGroupAttribute(mockAudioXmlNode_, deviceFlag);
}

/**
 * @tc.name  : Test ParserAffinityGroupAttribute.
 * @tc.number: AudioAffinityParserUnitTest_011
 * @tc.desc  : Test ParserAffinityGroupAttribute interface.
 */
HWTEST_F(AudioAffinityParserUnitTest, AudioAffinityParserUnitTest_011, TestSize.Level4)
{
    DeviceFlag deviceFlag = INPUT_DEVICES_FLAG;

    EXPECT_CALL(*(mockAudioXmlNode_), IsNodeValid())
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(false))
        .WillOnce(Return(false));

    EXPECT_CALL(*(mockAudioXmlNode_), CompareName(StrEq("AffinityGroup"))).WillOnce(Return(true));

    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(testing::_, testing::_))
        .WillOnce(Return(ERROR))
        .WillOnce(Return(ERROR));
    EXPECT_CALL(*(mockAudioXmlNode_), GetCopyNode()).WillOnce(Return(mockAudioXmlNode_));

    EXPECT_CALL(*(mockAudioXmlNode_), MoveToChildren()).Times(2);
    EXPECT_CALL(*(mockAudioXmlNode_), MoveToNext()).Times(1);

    audioAffinityParser_->ParserAffinityGroupAttribute(mockAudioXmlNode_, deviceFlag);
}

/**
 * @tc.name  : Test ParserAffinityGroupAttribute.
 * @tc.number: AudioAffinityParserUnitTest_012
 * @tc.desc  : Test ParserAffinityGroupAttribute interface.
 */
HWTEST_F(AudioAffinityParserUnitTest, AudioAffinityParserUnitTest_012, TestSize.Level4)
{
    DeviceFlag deviceFlag = INPUT_DEVICES_FLAG;

    EXPECT_CALL(*(mockAudioXmlNode_), IsNodeValid())
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(false))
        .WillOnce(Return(false));

    EXPECT_CALL(*(mockAudioXmlNode_), CompareName(StrEq("AffinityGroup"))).WillOnce(Return(true));

    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("isPrimary"), _))
        .WillOnce(DoAll(SetArgReferee<1>("True"), Return(SUCCESS)));

    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("name"), _))
        .WillOnce(DoAll(SetArgReferee<1>("True"), Return(SUCCESS)));

    EXPECT_CALL(*(mockAudioXmlNode_), GetCopyNode()).WillOnce(Return(mockAudioXmlNode_));

    EXPECT_CALL(*(mockAudioXmlNode_), MoveToChildren()).Times(2);
    EXPECT_CALL(*(mockAudioXmlNode_), MoveToNext()).Times(1);

    audioAffinityParser_->ParserAffinityGroupAttribute(mockAudioXmlNode_, deviceFlag);
}

/**
 * @tc.name  : Test ParserAffinityGroupAttribute.
 * @tc.number: AudioAffinityParserUnitTest_013
 * @tc.desc  : Test ParserAffinityGroupAttribute interface.
 */
HWTEST_F(AudioAffinityParserUnitTest, AudioAffinityParserUnitTest_013, TestSize.Level4)
{
    DeviceFlag deviceFlag = INPUT_DEVICES_FLAG;

    EXPECT_CALL(*(mockAudioXmlNode_), IsNodeValid())
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(false))
        .WillOnce(Return(false));

    EXPECT_CALL(*(mockAudioXmlNode_), CompareName(StrEq("AffinityGroup"))).WillOnce(Return(true));

    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("isPrimary"), _))
        .WillOnce(DoAll(SetArgReferee<1>("False"), Return(SUCCESS)));

    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("name"), _))
        .WillOnce(DoAll(SetArgReferee<1>("True"), Return(SUCCESS)));

    EXPECT_CALL(*(mockAudioXmlNode_), GetCopyNode()).WillOnce(Return(mockAudioXmlNode_));

    EXPECT_CALL(*(mockAudioXmlNode_), MoveToChildren()).Times(2);
    EXPECT_CALL(*(mockAudioXmlNode_), MoveToNext()).Times(1);

    audioAffinityParser_->ParserAffinityGroupAttribute(mockAudioXmlNode_, deviceFlag);
}

/**
 * @tc.name  : Test ParserAffinityGroupDeviceInfos.
 * @tc.number: AudioAffinityParserUnitTest_014
 * @tc.desc  : Test ParserAffinityGroupDeviceInfos interface.
 */
HWTEST_F(AudioAffinityParserUnitTest, AudioAffinityParserUnitTest_014, TestSize.Level4)
{
    AffinityDeviceInfo deviceInfo;

    EXPECT_CALL(*(mockAudioXmlNode_), MoveToChildren()).Times(1);

    EXPECT_CALL(*(mockAudioXmlNode_), IsNodeValid())
        .WillOnce(Return(true))
        .WillOnce(Return(false));

    audioAffinityParser_->affinityDeviceInfoArray_.clear();
    audioAffinityParser_->ParserAffinityGroupDeviceInfos(mockAudioXmlNode_, deviceInfo);
    EXPECT_EQ(audioAffinityParser_->affinityDeviceInfoArray_.empty(), true);
}

/**
 * @tc.name  : Test ParserAffinityGroupDeviceInfos.
 * @tc.number: AudioAffinityParserUnitTest_015
 * @tc.desc  : Test ParserAffinityGroupDeviceInfos interface.
 */
HWTEST_F(AudioAffinityParserUnitTest, AudioAffinityParserUnitTest_015, TestSize.Level4)
{
    AffinityDeviceInfo deviceInfo;

    EXPECT_CALL(*(mockAudioXmlNode_), MoveToChildren()).Times(1);

    EXPECT_CALL(*(mockAudioXmlNode_), IsNodeValid())
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(false));

    EXPECT_CALL(*(mockAudioXmlNode_), CompareName(StrEq("Affinity"))).WillOnce(Return(false));
    EXPECT_CALL(*(mockAudioXmlNode_), MoveToNext()).Times(1);

    audioAffinityParser_->affinityDeviceInfoArray_.clear();
    audioAffinityParser_->ParserAffinityGroupDeviceInfos(mockAudioXmlNode_, deviceInfo);
    EXPECT_EQ(audioAffinityParser_->affinityDeviceInfoArray_.empty(), true);
}

/**
 * @tc.name  : Test ParserAffinityGroupDeviceInfos.
 * @tc.number: AudioAffinityParserUnitTest_016
 * @tc.desc  : Test ParserAffinityGroupDeviceInfos interface.
 */
HWTEST_F(AudioAffinityParserUnitTest, AudioAffinityParserUnitTest_016, TestSize.Level4)
{
    AffinityDeviceInfo deviceInfo;

    EXPECT_CALL(*(mockAudioXmlNode_), MoveToChildren()).Times(1);

    EXPECT_CALL(*(mockAudioXmlNode_), IsNodeValid())
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(false));

    EXPECT_CALL(*(mockAudioXmlNode_), CompareName(StrEq("Affinity"))).WillOnce(Return(true));

    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("networkId"), _))
        .WillOnce(DoAll(SetArgReferee<1>("False"), Return(SUCCESS)));

    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("deviceType"), _))
        .WillOnce(DoAll(SetArgReferee<1>("DEVICE_TYPE_WIRED_HEADSET"), Return(SUCCESS)));

    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("supportedConcurrency"), _))
        .WillOnce(DoAll(SetArgReferee<1>("True"), Return(SUCCESS)));

    EXPECT_CALL(*(mockAudioXmlNode_), MoveToNext()).Times(1);

    audioAffinityParser_->affinityDeviceInfoArray_.clear();
    audioAffinityParser_->ParserAffinityGroupDeviceInfos(mockAudioXmlNode_, deviceInfo);
    EXPECT_EQ(audioAffinityParser_->affinityDeviceInfoArray_.empty(), false);
}

/**
 * @tc.name  : Test ParserAffinityGroupDeviceInfos.
 * @tc.number: AudioAffinityParserUnitTest_017
 * @tc.desc  : Test ParserAffinityGroupDeviceInfos interface.
 */
HWTEST_F(AudioAffinityParserUnitTest, AudioAffinityParserUnitTest_017, TestSize.Level4)
{
    AffinityDeviceInfo deviceInfo;

    EXPECT_CALL(*(mockAudioXmlNode_), MoveToChildren()).Times(1);

    EXPECT_CALL(*(mockAudioXmlNode_), IsNodeValid())
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(false));

    EXPECT_CALL(*(mockAudioXmlNode_), CompareName(StrEq("Affinity"))).WillOnce(Return(true));

    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("networkId"), _))
        .WillOnce(DoAll(SetArgReferee<1>("False"), Return(SUCCESS)));

    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("deviceType"), _))
        .WillOnce(DoAll(SetArgReferee<1>("Test"), Return(SUCCESS)));

    EXPECT_CALL(*(mockAudioXmlNode_), GetProp(StrEq("supportedConcurrency"), _))
        .WillOnce(DoAll(SetArgReferee<1>("False"), Return(SUCCESS)));

    EXPECT_CALL(*(mockAudioXmlNode_), MoveToNext()).Times(1);

    audioAffinityParser_->affinityDeviceInfoArray_.clear();
    audioAffinityParser_->ParserAffinityGroupDeviceInfos(mockAudioXmlNode_, deviceInfo);
    EXPECT_EQ(audioAffinityParser_->affinityDeviceInfoArray_.empty(), false);
}

} // namespace AudioStandard
} // namespace