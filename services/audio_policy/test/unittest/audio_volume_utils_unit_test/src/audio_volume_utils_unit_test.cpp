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

#include "audio_volume_utils_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
static constexpr int32_t MAX_VOLUME_LEVEL = 15;
static constexpr int32_t MIN_VOLUME_LEVEL = 0;
static constexpr int32_t DEFAULT_VOLUME_LEVEL = 7;
static constexpr int32_t DP_DEFAULT_VOLUME_LEVEL = 25;
static constexpr float HEARING_AID_MAX_VOLUME_PROP = 0.8;
void AudioVolumeUtilsUnitTest::SetUpTestCase(void) {}
void AudioVolumeUtilsUnitTest::TearDownTestCase(void) {}
void AudioVolumeUtilsUnitTest::SetUp(void) {}
void AudioVolumeUtilsUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test GetDefaultVolumeLevel.
 * @tc.number: GetDefaultVolumeLevel
 * @tc.desc  : Test GetDefaultVolumeLevel.
 */
HWTEST_F(AudioVolumeUtilsUnitTest, GetDefaultVolumeLevel, TestSize.Level1)
{
    AudioVolumeUtils utils = AudioVolumeUtils::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    AudioStreamType type = STREAM_MUSIC;
    AudioStreamType type2 = STREAM_RING;

    EXPECT_EQ(utils.GetDefaultVolumeLevel(desc, type), DEFAULT_VOLUME_LEVEL);
    
    desc->deviceType_ = DEVICE_TYPE_REMOTE_CAST;
    EXPECT_EQ(utils.GetDefaultVolumeLevel(desc, type), MAX_VOLUME_LEVEL);
    EXPECT_EQ(utils.GetDefaultVolumeLevel(desc, type2), DEFAULT_VOLUME_LEVEL);
    desc->deviceType_ = DEVICE_TYPE_HEARING_AID;
    EXPECT_EQ(utils.GetDefaultVolumeLevel(desc, type),
        static_cast<int32_t>(std::ceil(MAX_VOLUME_LEVEL * HEARING_AID_MAX_VOLUME_PROP)));
    EXPECT_EQ(utils.GetDefaultVolumeLevel(desc, type2), DEFAULT_VOLUME_LEVEL);
    desc->deviceType_ = DEVICE_TYPE_DP;
    EXPECT_EQ(utils.GetDefaultVolumeLevel(desc, type), MAX_VOLUME_LEVEL);
    EXPECT_EQ(utils.GetDefaultVolumeLevel(desc, type2), DEFAULT_VOLUME_LEVEL);

    desc->deviceType_ = DEVICE_TYPE_HDMI;
    EXPECT_EQ(utils.GetDefaultVolumeLevel(desc, type), MAX_VOLUME_LEVEL);
    EXPECT_EQ(utils.GetDefaultVolumeLevel(desc, type2), DEFAULT_VOLUME_LEVEL);

    desc->deviceType_ = DEVICE_TYPE_SPEAKER;
    EXPECT_EQ(utils.GetDefaultVolumeLevel(desc, type), MAX_VOLUME_LEVEL);
    EXPECT_EQ(utils.GetDefaultVolumeLevel(desc, type2), DEFAULT_VOLUME_LEVEL);

    desc->networkId_ = "123";
    EXPECT_EQ(utils.GetDefaultVolumeLevel(desc, type), MAX_VOLUME_LEVEL);
    EXPECT_EQ(utils.GetDefaultVolumeLevel(desc, type2), DEFAULT_VOLUME_LEVEL);
}

/**
 * @tc.name  : Test GetDefaultVolumeLevel.
 * @tc.number: GetMaxVolumeLevelFromConfig
 * @tc.desc  : Test GetDefaultVolumeLevel.
 */
HWTEST_F(AudioVolumeUtilsUnitTest, GetMaxVolumeLevelFromConfig, TestSize.Level1)
{
    AudioVolumeUtils utils = AudioVolumeUtils::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    int32_t volume = MAX_VOLUME_LEVEL;
    AudioStreamType type = STREAM_MUSIC;
    utils.GetMaxVolumeLevelFromConfig(desc, type, volume);
    EXPECT_EQ(volume, MAX_VOLUME_LEVEL);
    utils.GetMaxVolumeLevelFromConfig(desc, type, volume);
    EXPECT_EQ(volume, MAX_VOLUME_LEVEL);

    utils.GetMaxVolumeLevelFromConfig(desc, STREAM_APP, volume);
    EXPECT_EQ(volume, MAX_VOLUME_LEVEL);
    desc->deviceType_ = DEVICE_TYPE_SPEAKER;
    utils.GetMaxVolumeLevelFromConfig(desc, type, volume);
    EXPECT_EQ(volume, MAX_VOLUME_LEVEL);
}

/**
 * @tc.name  : Test GetDefaultVolumeLevel.
 * @tc.number: GetMinVolumeLevelFromConfig
 * @tc.desc  : Test GetDefaultVolumeLevel.
 */
HWTEST_F(AudioVolumeUtilsUnitTest, GetMinVolumeLevelFromConfig, TestSize.Level1)
{
    AudioVolumeUtils utils = AudioVolumeUtils::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    int32_t volume = MIN_VOLUME_LEVEL;
    AudioStreamType type = STREAM_MUSIC;
    utils.GetMinVolumeLevelFromConfig(desc, type, volume);
    EXPECT_EQ(volume, MIN_VOLUME_LEVEL);
    utils.GetMinVolumeLevelFromConfig(desc, type, volume);
    EXPECT_EQ(volume, MIN_VOLUME_LEVEL);

    utils.GetMinVolumeLevelFromConfig(desc, STREAM_APP, volume);
    EXPECT_EQ(volume, MIN_VOLUME_LEVEL);
    desc->deviceType_ = DEVICE_TYPE_SPEAKER;
    utils.GetMinVolumeLevelFromConfig(desc, type, volume);
    EXPECT_EQ(volume, MIN_VOLUME_LEVEL);
}

/**
 * @tc.name  : Test GetDefaultVolumeLevel.
 * @tc.number: IsDistributedDevice
 * @tc.desc  : Test IsDistributedDevice.
 */
HWTEST_F(AudioVolumeUtilsUnitTest, IsDistributedDevice, TestSize.Level1)
{
    AudioVolumeUtils utils = AudioVolumeUtils::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    EXPECT_EQ(utils.IsDistributedDevice(desc), false);
    desc->deviceType_ = DEVICE_TYPE_REMOTE_CAST;
    EXPECT_EQ(utils.IsDistributedDevice(desc), true);
    desc->deviceType_ = DEVICE_TYPE_SPEAKER;
    EXPECT_EQ(utils.IsDistributedDevice(desc), false);
    desc->networkId_ = "123";
    EXPECT_EQ(utils.IsDistributedDevice(desc), true);
    desc->deviceType_ = DEVICE_TYPE_DP;
    EXPECT_EQ(utils.IsDistributedDevice(desc), false);
}

} // namespace AudioStandard
} // namespace OHOS