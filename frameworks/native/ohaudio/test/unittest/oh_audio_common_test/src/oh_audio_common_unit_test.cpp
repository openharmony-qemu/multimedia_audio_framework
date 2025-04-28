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

#include "oh_audio_common_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void OHAudioCommonUnitTest::SetUpTestCase(void) { }

void OHAudioCommonUnitTest::TearDownTestCase(void) { }

void OHAudioCommonUnitTest::SetUp(void) { }

void OHAudioCommonUnitTest::TearDown(void) { }

/**
 * @tc.name  : Test ConvertLayoutToChannel.
 * @tc.number: ConvertLayoutToChannel_001
 * @tc.desc  : Test ConvertLayoutToChannel.
 */
HWTEST(OHAudioCommonUnitTest, ConvertLayoutToChannel_001, TestSize.Level0)
{
    OH_AudioChannelLayout layout = OH_AudioChannelLayout::CH_LAYOUT_MONO;
    AudioChannel channel = OHAudioCommon::ConvertLayoutToChannel(layout);
    EXPECT_EQ(AudioChannel::MONO, channel);
}

/**
 * @tc.name  : Test ConvertLayoutToChannel.
 * @tc.number: ConvertLayoutToChannel_002
 * @tc.desc  : Test ConvertLayoutToChannel.
 */
HWTEST(OHAudioCommonUnitTest, ConvertLayoutToChannel_002, TestSize.Level0)
{
    OH_AudioChannelLayout layout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    AudioChannel channel = OHAudioCommon::ConvertLayoutToChannel(layout);
    EXPECT_EQ(AudioChannel::STEREO, channel);
}

/**
 * @tc.name  : Test ConvertLayoutToChannel.
 * @tc.number: ConvertLayoutToChannel_003
 * @tc.desc  : Test ConvertLayoutToChannel.
 */
HWTEST(OHAudioCommonUnitTest, ConvertLayoutToChannel_003, TestSize.Level0)
{
    OH_AudioChannelLayout layout = OH_AudioChannelLayout::CH_LAYOUT_2POINT1;
    AudioChannel channel = OHAudioCommon::ConvertLayoutToChannel(layout);
    EXPECT_EQ(AudioChannel::CHANNEL_3, channel);
}

/**
 * @tc.name  : Test ConvertLayoutToChannel.
 * @tc.number: ConvertLayoutToChannel_004
 * @tc.desc  : Test ConvertLayoutToChannel.
 */
HWTEST(OHAudioCommonUnitTest, ConvertLayoutToChannel_004, TestSize.Level0)
{
    OH_AudioChannelLayout layout = OH_AudioChannelLayout::CH_LAYOUT_3POINT1;
    AudioChannel channel = OHAudioCommon::ConvertLayoutToChannel(layout);
    EXPECT_EQ(AudioChannel::CHANNEL_4, channel);
}

/**
 * @tc.name  : Test ConvertLayoutToChannel.
 * @tc.number: ConvertLayoutToChannel_005
 * @tc.desc  : Test ConvertLayoutToChannel.
 */
HWTEST(OHAudioCommonUnitTest, ConvertLayoutToChannel_005, TestSize.Level0)
{
    OH_AudioChannelLayout layout = OH_AudioChannelLayout::CH_LAYOUT_2POINT1POINT2;
    AudioChannel channel = OHAudioCommon::ConvertLayoutToChannel(layout);
    EXPECT_EQ(AudioChannel::CHANNEL_5, channel);
}

/**
 * @tc.name  : Test ConvertLayoutToChannel.
 * @tc.number: ConvertLayoutToChannel_006
 * @tc.desc  : Test ConvertLayoutToChannel.
 */
HWTEST(OHAudioCommonUnitTest, ConvertLayoutToChannel_006, TestSize.Level0)
{
    OH_AudioChannelLayout layout = OH_AudioChannelLayout::CH_LAYOUT_5POINT1;
    AudioChannel channel = OHAudioCommon::ConvertLayoutToChannel(layout);
    EXPECT_EQ(AudioChannel::CHANNEL_6, channel);
}

/**
 * @tc.name  : Test ConvertLayoutToChannel.
 * @tc.number: ConvertLayoutToChannel_007
 * @tc.desc  : Test ConvertLayoutToChannel.
 */
HWTEST(OHAudioCommonUnitTest, ConvertLayoutToChannel_007, TestSize.Level0)
{
    OH_AudioChannelLayout layout = OH_AudioChannelLayout::CH_LAYOUT_7POINT0;
    AudioChannel channel = OHAudioCommon::ConvertLayoutToChannel(layout);
    EXPECT_EQ(AudioChannel::CHANNEL_7, channel);
}

/**
 * @tc.name  : Test ConvertLayoutToChannel.
 * @tc.number: ConvertLayoutToChannel_008
 * @tc.desc  : Test ConvertLayoutToChannel.
 */
HWTEST(OHAudioCommonUnitTest, ConvertLayoutToChannel_008, TestSize.Level0)
{
    OH_AudioChannelLayout layout = OH_AudioChannelLayout::CH_LAYOUT_7POINT1;
    AudioChannel channel = OHAudioCommon::ConvertLayoutToChannel(layout);
    EXPECT_EQ(AudioChannel::CHANNEL_8, channel);
}

/**
 * @tc.name  : Test ConvertLayoutToChannel.
 * @tc.number: ConvertLayoutToChannel_009
 * @tc.desc  : Test ConvertLayoutToChannel.
 */
HWTEST(OHAudioCommonUnitTest, ConvertLayoutToChannel_009, TestSize.Level0)
{
    OH_AudioChannelLayout layout = OH_AudioChannelLayout::CH_LAYOUT_7POINT1POINT2;
    AudioChannel channel = OHAudioCommon::ConvertLayoutToChannel(layout);
    EXPECT_EQ(AudioChannel::CHANNEL_10, channel);
}

/**
 * @tc.name  : Test ConvertLayoutToChannel.
 * @tc.number: ConvertLayoutToChannel_010
 * @tc.desc  : Test ConvertLayoutToChannel.
 */
HWTEST(OHAudioCommonUnitTest, ConvertLayoutToChannel_010, TestSize.Level0)
{
    OH_AudioChannelLayout layout = OH_AudioChannelLayout::CH_LAYOUT_7POINT1POINT4;
    AudioChannel channel = OHAudioCommon::ConvertLayoutToChannel(layout);
    EXPECT_EQ(AudioChannel::CHANNEL_12, channel);
}

/**
 * @tc.name  : Test ConvertLayoutToChannel.
 * @tc.number: ConvertLayoutToChannel_011
 * @tc.desc  : Test ConvertLayoutToChannel.
 */
HWTEST(OHAudioCommonUnitTest, ConvertLayoutToChannel_011, TestSize.Level0)
{
    OH_AudioChannelLayout layout = OH_AudioChannelLayout::CH_LAYOUT_UNKNOWN;
    AudioChannel channel = OHAudioCommon::ConvertLayoutToChannel(layout);
    EXPECT_EQ(AudioChannel::CHANNEL_UNKNOW, channel);
}

} // namespace AudioStandard
} // namespace OHOS
