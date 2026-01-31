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

#include "audio_volume_parser_unit_test.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
const std::vector<VolumePoint> VOLUME_POINTS = {
    {0, -2700},
    {33, -1800},
    {66, -900},
    {100, 0}
};

void AudioVolumeParserUnitTest::SetUpTestCase(void) {}
void AudioVolumeParserUnitTest::TearDownTestCase(void) {}
void AudioVolumeParserUnitTest::SetUp(void)
{
    audioVolumeParser_ = std::make_shared<AudioVolumeParser>();
    ASSERT_TRUE(audioVolumeParser_ != nullptr);
}
void AudioVolumeParserUnitTest::TearDown(void)
{
    audioVolumeParser_ = nullptr;
}

/**
 * @tc.name  : Test UseVoiceAssistantFixedVolumeConfig.
 * @tc.number: AudioVolumeParserUnitTest_001
 * @tc.desc  : Test UseVoiceAssistantFixedVolumeConfig. No STREAM_VOICE_ASSISTANT info.
 */
HWTEST_F(AudioVolumeParserUnitTest, AudioVolumeParserUnitTest_001, TestSize.Level2)
{
    StreamVolumeInfoMap streamVolumeInfoMap;
    int32_t result = SUCCESS;

    result = audioVolumeParser_->UseVoiceAssistantFixedVolumeConfig(streamVolumeInfoMap);
    EXPECT_EQ(result, ERROR);

    streamVolumeInfoMap[STREAM_VOICE_ASSISTANT] = nullptr;
    result = audioVolumeParser_->UseVoiceAssistantFixedVolumeConfig(streamVolumeInfoMap);
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name  : Test UseVoiceAssistantFixedVolumeConfig.
 * @tc.number: AudioVolumeParserUnitTest_002
 * @tc.desc  : Test UseVoiceAssistantFixedVolumeConfig. Incorrect STREAM_VOICE_ASSISTANT info.
 */
HWTEST_F(AudioVolumeParserUnitTest, AudioVolumeParserUnitTest_002, TestSize.Level2)
{
    StreamVolumeInfoMap streamVolumeInfoMap;

    // Init streamVolumeInfoMap[STREAM_VOICE_ASSISTANT].
    std::shared_ptr<StreamVolumeInfo> streamVolumeInfo = std::make_shared<StreamVolumeInfo>();
    streamVolumeInfo->streamType = STREAM_VOICE_ASSISTANT;
    streamVolumeInfo->minLevel = 1;
    streamVolumeInfo->maxLevel = 15;
    streamVolumeInfo->defaultLevel = 7;
    std::shared_ptr<DeviceVolumeInfo> earpieceDeviceVolumeInfo = std::make_shared<DeviceVolumeInfo>();
    earpieceDeviceVolumeInfo->deviceType = EARPIECE_VOLUME_TYPE;
    earpieceDeviceVolumeInfo->volumePoints = VOLUME_POINTS;
    streamVolumeInfo->deviceVolumeInfos[EARPIECE_VOLUME_TYPE] = earpieceDeviceVolumeInfo;
    std::shared_ptr<DeviceVolumeInfo> speakerDeviceVolumeInfo = std::make_shared<DeviceVolumeInfo>();
    speakerDeviceVolumeInfo->deviceType = SPEAKER_VOLUME_TYPE;
    speakerDeviceVolumeInfo->volumePoints = VOLUME_POINTS;
    streamVolumeInfo->deviceVolumeInfos[SPEAKER_VOLUME_TYPE] =speakerDeviceVolumeInfo;
    std::shared_ptr<DeviceVolumeInfo> headsetDeviceVolumeInfo = std::make_shared<DeviceVolumeInfo>();
    headsetDeviceVolumeInfo->deviceType = HEADSET_VOLUME_TYPE;
    headsetDeviceVolumeInfo->volumePoints = VOLUME_POINTS;
    streamVolumeInfo->deviceVolumeInfos[HEADSET_VOLUME_TYPE] = headsetDeviceVolumeInfo;
    streamVolumeInfoMap[STREAM_VOICE_ASSISTANT] = streamVolumeInfo;

    // Test UseVoiceAssistantFixedVolumeConfig function.
    int32_t result = audioVolumeParser_->UseVoiceAssistantFixedVolumeConfig(streamVolumeInfoMap);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(streamVolumeInfoMap[STREAM_VOICE_ASSISTANT]->minLevel, 0);
    EXPECT_EQ(
        streamVolumeInfoMap[STREAM_VOICE_ASSISTANT]->deviceVolumeInfos[EARPIECE_VOLUME_TYPE]->volumePoints[0].index, 1);
    EXPECT_EQ(
        streamVolumeInfoMap[STREAM_VOICE_ASSISTANT]->deviceVolumeInfos[SPEAKER_VOLUME_TYPE]->volumePoints[0].index, 1);
    EXPECT_EQ(
        streamVolumeInfoMap[STREAM_VOICE_ASSISTANT]->deviceVolumeInfos[HEADSET_VOLUME_TYPE]->volumePoints[0].index, 1);
}

/**
 * @tc.name  : Test UseVoiceAssistantFixedVolumeConfig.
 * @tc.number: AudioVolumeParserUnitTest_003
 * @tc.desc  : Test UseVoiceAssistantFixedVolumeConfig. Incorrect STREAM_VOICE_ASSISTANT info.
 */
HWTEST_F(AudioVolumeParserUnitTest, AudioVolumeParserUnitTest_003, TestSize.Level2)
{
    StreamVolumeInfoMap streamVolumeInfoMap;

    // Init streamVolumeInfoMap[STREAM_VOICE_ASSISTANT].
    std::shared_ptr<StreamVolumeInfo> streamVolumeInfo = std::make_shared<StreamVolumeInfo>();
    streamVolumeInfo->streamType = STREAM_VOICE_ASSISTANT;
    streamVolumeInfo->minLevel = 1;
    streamVolumeInfo->maxLevel = 15;
    streamVolumeInfo->defaultLevel = 7;
    std::shared_ptr<DeviceVolumeInfo> earpieceDeviceVolumeInfo = std::make_shared<DeviceVolumeInfo>();
    earpieceDeviceVolumeInfo->deviceType = EARPIECE_VOLUME_TYPE;
    earpieceDeviceVolumeInfo->volumePoints = VOLUME_POINTS;
    streamVolumeInfo->deviceVolumeInfos[EARPIECE_VOLUME_TYPE] = earpieceDeviceVolumeInfo;
    std::shared_ptr<DeviceVolumeInfo> speakerDeviceVolumeInfo = std::make_shared<DeviceVolumeInfo>();
    speakerDeviceVolumeInfo->deviceType = SPEAKER_VOLUME_TYPE;
    speakerDeviceVolumeInfo->volumePoints = {};
    streamVolumeInfo->deviceVolumeInfos[SPEAKER_VOLUME_TYPE] =speakerDeviceVolumeInfo;
    streamVolumeInfoMap[STREAM_VOICE_ASSISTANT] = streamVolumeInfo;

    // Test UseVoiceAssistantFixedVolumeConfig function.
    int32_t result = audioVolumeParser_->UseVoiceAssistantFixedVolumeConfig(streamVolumeInfoMap);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(streamVolumeInfoMap[STREAM_VOICE_ASSISTANT]->minLevel, 0);
    EXPECT_EQ(
        streamVolumeInfoMap[STREAM_VOICE_ASSISTANT]->deviceVolumeInfos[EARPIECE_VOLUME_TYPE]->volumePoints[0].index, 1);
}
} // namespace AudioStandard
} // namespace OHOS