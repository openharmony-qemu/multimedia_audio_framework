/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "audio_loud_volume_manager_unit_test.h"
#include "audio_errors.h"
#include "audio_policy_log.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void LoudVolumeManagerUnitTest::SetUpTestCase(void) {}
void LoudVolumeManagerUnitTest::TearDownTestCase(void) {}

void LoudVolumeManagerUnitTest::SetUp(void)
{
    loudVolumeManager_ = new LoudVolumeManager();
    loudVolumeManager_->loudVolumeSupportMode_ = LOUD_VOLUME_SUPPORT;
}

void LoudVolumeManagerUnitTest::TearDown(void)
{
    delete loudVolumeManager_;
    loudVolumeManager_ = nullptr;
}

/**
 * @tc.name  : ReloadLoudVolumeMode_NotSupport
 * @tc.number: ReloadLoudVolumeMode_001
 * @tc.desc  : Test ReloadLoudVolumeMode returns false when loudVolumeSupportMode_ is NOT_SUPPORT
 */
HWTEST_F(LoudVolumeManagerUnitTest, ReloadLoudVolumeMode_NotSupport_001, TestSize.Level1)
{
    loudVolumeManager_->loudVolumeSupportMode_ = LOUD_VOLUME_NOT_SUPPORT;

    bool result = loudVolumeManager_->ReloadLoudVolumeMode(STREAM_MUSIC, LOUD_VOLUME_SWITCH_ON, nullptr);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : ReloadLoudVolumeMode_SkipCloseType
 * @tc.number: ReloadLoudVolumeMode_002
 * @tc.desc  : Test ReloadLoudVolumeMode returns false when streamInFocus is skip close type (STREAM_RING)
 */
HWTEST_F(LoudVolumeManagerUnitTest, ReloadLoudVolumeMode_SkipCloseType_002, TestSize.Level1)
{
    loudVolumeManager_->loudVolumeSupportMode_ = LOUD_VOLUME_SUPPORT;

    bool result = loudVolumeManager_->ReloadLoudVolumeMode(STREAM_RING, LOUD_VOLUME_SWITCH_ON, nullptr);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : ReloadLoudVolumeMode_UnsupportedStream
 * @tc.number: ReloadLoudVolumeMode_003
 * @tc.desc  : Test ReloadLoudVolumeMode returns false when streamInFocus is not in loud volume enable map
 */
HWTEST_F(LoudVolumeManagerUnitTest, ReloadLoudVolumeMode_UnsupportedStream_003, TestSize.Level1)
{
    loudVolumeManager_->loudVolumeSupportMode_ = LOUD_VOLUME_SUPPORT;

    bool result = loudVolumeManager_->ReloadLoudVolumeMode(STREAM_ALARM, LOUD_VOLUME_SWITCH_ON, nullptr);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : ReloadLoudVolumeMode_NonLocalDevice
 * @tc.number: ReloadLoudVolumeMode_004
 * @tc.desc  : Test ReloadLoudVolumeMode returns false when device networkId_ is not LOCAL_NETWORK_ID
 */
HWTEST_F(LoudVolumeManagerUnitTest, ReloadLoudVolumeMode_NonLocalDevice_004, TestSize.Level1)
{
    loudVolumeManager_->loudVolumeSupportMode_ = LOUD_VOLUME_SUPPORT;
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_SPEAKER;
    device->networkId_ = "RemoteDevice";

    bool result = loudVolumeManager_->ReloadLoudVolumeMode(STREAM_MUSIC, LOUD_VOLUME_SWITCH_ON, device);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : ReloadLoudVolumeMode_NonSpeakerDevice
 * @tc.number: ReloadLoudVolumeMode_005
 * @tc.desc  : Test ReloadLoudVolumeMode returns false when device type is not SPEAKER with LOCAL_NETWORK_ID
 */
HWTEST_F(LoudVolumeManagerUnitTest, ReloadLoudVolumeMode_NonSpeakerDevice_005, TestSize.Level1)
{
    loudVolumeManager_->loudVolumeSupportMode_ = LOUD_VOLUME_SUPPORT;
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    device->networkId_ = LOCAL_NETWORK_ID;

    bool result = loudVolumeManager_->ReloadLoudVolumeMode(STREAM_MUSIC, LOUD_VOLUME_SWITCH_ON, device);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : ReloadLoudVolumeMode_NullDevice
 * @tc.number: ReloadLoudVolumeMode_006
 * @tc.desc  : Test ReloadLoudVolumeMode passes all checks with null device and calls ReloadLoudVolumeModeSwitch
 */
HWTEST_F(LoudVolumeManagerUnitTest, ReloadLoudVolumeMode_NullDevice_006, TestSize.Level1)
{
    loudVolumeManager_->loudVolumeSupportMode_ = LOUD_VOLUME_SUPPORT;
    loudVolumeManager_->loudVolumeHoldMap_.clear();

    bool result = loudVolumeManager_->ReloadLoudVolumeMode(STREAM_MUSIC, LOUD_VOLUME_SWITCH_ON, nullptr);
    EXPECT_TRUE(result);
}

/**
 * @tc.name  : ReloadLoudVolumeMode_ValidSpeakerDevice
 * @tc.number: ReloadLoudVolumeMode_007
 * @tc.desc  : Test ReloadLoudVolumeMode passes all checks with valid speaker device
 */
HWTEST_F(LoudVolumeManagerUnitTest, ReloadLoudVolumeMode_ValidSpeakerDevice_007, TestSize.Level1)
{
    loudVolumeManager_->loudVolumeSupportMode_ = LOUD_VOLUME_SUPPORT;
    loudVolumeManager_->loudVolumeHoldMap_.clear();
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_SPEAKER;
    device->networkId_ = LOCAL_NETWORK_ID;

    bool result = loudVolumeManager_->ReloadLoudVolumeMode(STREAM_MUSIC, LOUD_VOLUME_SWITCH_ON, device);
    EXPECT_TRUE(result);
}

/**
 * @tc.name  : CheckLoudVolumeMode_NotSupport
 * @tc.number: CheckLoudVolumeMode_001
 * @tc.desc  : Test CheckLoudVolumeMode returns false when loudVolumeSupportMode_ is NOT_SUPPORT
 */
HWTEST_F(LoudVolumeManagerUnitTest, CheckLoudVolumeMode_NotSupport_001, TestSize.Level1)
{
    loudVolumeManager_->loudVolumeSupportMode_ = LOUD_VOLUME_NOT_SUPPORT;

    bool result = loudVolumeManager_->CheckLoudVolumeMode(
        15, OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP, STREAM_MUSIC, nullptr);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : CheckLoudVolumeMode_UnsupportedStream
 * @tc.number: CheckLoudVolumeMode_002
 * @tc.desc  : Test CheckLoudVolumeMode returns false when streamInFocus is not in loud volume enable map
 */
HWTEST_F(LoudVolumeManagerUnitTest, CheckLoudVolumeMode_UnsupportedStream_002, TestSize.Level1)
{
    loudVolumeManager_->loudVolumeSupportMode_ = LOUD_VOLUME_SUPPORT;

    bool result = loudVolumeManager_->CheckLoudVolumeMode(
        15, OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP, STREAM_ALARM, nullptr);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : CheckLoudVolumeMode_UpAlreadyHolding
 * @tc.number: CheckLoudVolumeMode_003
 * @tc.desc  : Test CheckLoudVolumeMode returns false when VOLUME_UP at max level and already holding loud volume
 */
HWTEST_F(LoudVolumeManagerUnitTest, CheckLoudVolumeMode_UpAlreadyHolding_003, TestSize.Level1)
{
    loudVolumeManager_->loudVolumeSupportMode_ = LOUD_VOLUME_SUPPORT;
    loudVolumeManager_->triggerTime = 0;
    loudVolumeManager_->upTriggerTimeMSec = 0;
    loudVolumeManager_->SetLoudVolumeHoldMap(LOUD_VOLUME_MODE_MUSIC, true);

    bool result = loudVolumeManager_->CheckLoudVolumeMode(
        15, OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP, STREAM_MUSIC, nullptr);
    EXPECT_FALSE(result);

    loudVolumeManager_->ClearLoudVolumeHoldMap(LOUD_VOLUME_MODE_MUSIC);
}

/**
 * @tc.name  : CheckLoudVolumeMode_UpTriggerOn
 * @tc.number: CheckLoudVolumeMode_004
 * @tc.desc  : Test CheckLoudVolumeMode triggers loud volume on when triggerTime=MAX and within time window
 */
HWTEST_F(LoudVolumeManagerUnitTest, CheckLoudVolumeMode_UpTriggerOn_004, TestSize.Level1)
{
    loudVolumeManager_->loudVolumeSupportMode_ = LOUD_VOLUME_SUPPORT;
    loudVolumeManager_->triggerTime = 2;
    loudVolumeManager_->upTriggerTimeMSec = INT64_MAX / 2;
    loudVolumeManager_->loudVolumeHoldMap_.clear();

    bool result = loudVolumeManager_->CheckLoudVolumeMode(
        15, OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP, STREAM_MUSIC, nullptr);
    EXPECT_TRUE(result);
}

/**
 * @tc.name  : CheckLoudVolumeMode_UpTriggerIncrement
 * @tc.number: CheckLoudVolumeMode_005
 * @tc.desc  : Test CheckLoudVolumeMode increments triggerTime when triggerTime=1 and beyond min time
 */
HWTEST_F(LoudVolumeManagerUnitTest, CheckLoudVolumeMode_UpTriggerIncrement_005, TestSize.Level1)
{
    loudVolumeManager_->loudVolumeSupportMode_ = LOUD_VOLUME_SUPPORT;
    loudVolumeManager_->triggerTime = 1;
    loudVolumeManager_->upTriggerTimeMSec = 0;
    loudVolumeManager_->loudVolumeHoldMap_.clear();

    bool result = loudVolumeManager_->CheckLoudVolumeMode(
        15, OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP, STREAM_MUSIC, nullptr);
    EXPECT_FALSE(result);
    EXPECT_EQ(loudVolumeManager_->triggerTime, 2);
}

/**
 * @tc.name  : CheckLoudVolumeMode_UpFirstTrigger
 * @tc.number: CheckLoudVolumeMode_006
 * @tc.desc  : Test CheckLoudVolumeMode sets triggerTime to 1 on first VOLUME_UP trigger at max level
 */
HWTEST_F(LoudVolumeManagerUnitTest, CheckLoudVolumeMode_UpFirstTrigger_006, TestSize.Level1)
{
    loudVolumeManager_->loudVolumeSupportMode_ = LOUD_VOLUME_SUPPORT;
    loudVolumeManager_->triggerTime = 0;
    loudVolumeManager_->upTriggerTimeMSec = 0;
    loudVolumeManager_->loudVolumeHoldMap_.clear();

    bool result = loudVolumeManager_->CheckLoudVolumeMode(
        15, OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP, STREAM_MUSIC, nullptr);
    EXPECT_FALSE(result);
    EXPECT_EQ(loudVolumeManager_->triggerTime, 1);
}

/**
 * @tc.name  : CheckLoudVolumeMode_DownWithHolding
 * @tc.number: CheckLoudVolumeMode_007
 * @tc.desc  : Test CheckLoudVolumeMode turns off loud volume on VOLUME_DOWN when holding
 */
HWTEST_F(LoudVolumeManagerUnitTest, CheckLoudVolumeMode_DownWithHolding_007, TestSize.Level1)
{
    loudVolumeManager_->loudVolumeSupportMode_ = LOUD_VOLUME_SUPPORT;
    loudVolumeManager_->triggerTime = 0;
    loudVolumeManager_->upTriggerTimeMSec = 0;
    loudVolumeManager_->SetLoudVolumeHoldMap(LOUD_VOLUME_MODE_MUSIC, true);

    bool result = loudVolumeManager_->CheckLoudVolumeMode(
        5, OHOS::MMI::KeyEvent::KEYCODE_VOLUME_DOWN, STREAM_MUSIC, nullptr);
    EXPECT_TRUE(result);
}

/**
 * @tc.name  : CheckLoudVolumeMode_ElseReset
 * @tc.number: CheckLoudVolumeMode_008
 * @tc.desc  : Test CheckLoudVolumeMode resets triggers when keyType is neither UP at max nor DOWN holding
 */
HWTEST_F(LoudVolumeManagerUnitTest, CheckLoudVolumeMode_ElseReset_008, TestSize.Level1)
{
    loudVolumeManager_->loudVolumeSupportMode_ = LOUD_VOLUME_SUPPORT;
    loudVolumeManager_->triggerTime = 2;
    loudVolumeManager_->upTriggerTimeMSec = 1000;
    loudVolumeManager_->loudVolumeHoldMap_.clear();
    constexpr int32_t INVALID_KEY_TYPE = 9999;

    bool result = loudVolumeManager_->CheckLoudVolumeMode(
        5, INVALID_KEY_TYPE, STREAM_MUSIC, nullptr);
    EXPECT_FALSE(result);
    EXPECT_EQ(loudVolumeManager_->triggerTime, 0);
    EXPECT_EQ(loudVolumeManager_->upTriggerTimeMSec, 0);
}

} // namespace AudioStandard
} // namespace OHOS