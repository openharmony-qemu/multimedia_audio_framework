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
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_SPEAKER;
    device->networkId_ = LOCAL_NETWORK_ID;

    bool result = loudVolumeManager_->ReloadLoudVolumeMode(STREAM_MUSIC, LOUD_VOLUME_SWITCH_ON, device);
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
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_SPEAKER;
    device->networkId_ = LOCAL_NETWORK_ID;

    bool result = loudVolumeManager_->ReloadLoudVolumeMode(STREAM_RING, LOUD_VOLUME_SWITCH_ON, device);
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
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_SPEAKER;
    device->networkId_ = LOCAL_NETWORK_ID;

    bool result = loudVolumeManager_->ReloadLoudVolumeMode(STREAM_ALARM, LOUD_VOLUME_SWITCH_ON, device);
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
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_SPEAKER;
    device->networkId_ = LOCAL_NETWORK_ID;

    bool result = loudVolumeManager_->CheckLoudVolumeMode(
        15, OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP, STREAM_MUSIC, device);
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
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_SPEAKER;
    device->networkId_ = LOCAL_NETWORK_ID;

    bool result = loudVolumeManager_->CheckLoudVolumeMode(
        15, OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP, STREAM_ALARM, device);
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
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_SPEAKER;
    device->networkId_ = LOCAL_NETWORK_ID;

    bool result = loudVolumeManager_->CheckLoudVolumeMode(
        15, OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP, STREAM_MUSIC, device);
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
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_SPEAKER;
    device->networkId_ = LOCAL_NETWORK_ID;

    bool result = loudVolumeManager_->CheckLoudVolumeMode(
        15, OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP, STREAM_MUSIC, device);
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
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_SPEAKER;
    device->networkId_ = LOCAL_NETWORK_ID;

    bool result = loudVolumeManager_->CheckLoudVolumeMode(
        15, OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP, STREAM_MUSIC, device);
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
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_SPEAKER;
    device->networkId_ = LOCAL_NETWORK_ID;

    bool result = loudVolumeManager_->CheckLoudVolumeMode(
        15, OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP, STREAM_MUSIC, device);
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
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_SPEAKER;
    device->networkId_ = LOCAL_NETWORK_ID;

    bool result = loudVolumeManager_->CheckLoudVolumeMode(
        5, OHOS::MMI::KeyEvent::KEYCODE_VOLUME_DOWN, STREAM_MUSIC, device);
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
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_SPEAKER;
    device->networkId_ = LOCAL_NETWORK_ID;

    bool result = loudVolumeManager_->CheckLoudVolumeMode(
        5, INVALID_KEY_TYPE, STREAM_MUSIC, device);
    EXPECT_FALSE(result);
    EXPECT_EQ(loudVolumeManager_->triggerTime, 0);
    EXPECT_EQ(loudVolumeManager_->upTriggerTimeMSec, 0);
}

/**
 * @tc.name  : ClearLoudVolumeHoldMap_ExistingEntry
 * @tc.number: ClearLoudVolumeHoldMap_001
 * @tc.desc  : Test ClearLoudVolumeHoldMap returns true when entry exists in map
 */
HWTEST_F(LoudVolumeManagerUnitTest, ClearLoudVolumeHoldMap_ExistingEntry_001, TestSize.Level1)
{
    loudVolumeManager_->SetLoudVolumeHoldMap(LOUD_VOLUME_MODE_MUSIC, true);
    bool result = loudVolumeManager_->ClearLoudVolumeHoldMap(LOUD_VOLUME_MODE_MUSIC);
    EXPECT_TRUE(result);
}

/**
 * @tc.name  : ClearLoudVolumeHoldMap_NonExistingEntry
 * @tc.number: ClearLoudVolumeHoldMap_002
 * @tc.desc  : Test ClearLoudVolumeHoldMap returns false when entry does not exist
 */
HWTEST_F(LoudVolumeManagerUnitTest, ClearLoudVolumeHoldMap_NonExistingEntry_002, TestSize.Level1)
{
    loudVolumeManager_->loudVolumeHoldMap_.clear();
    bool result = loudVolumeManager_->ClearLoudVolumeHoldMap(LOUD_VOLUME_MODE_MUSIC);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : ReloadLoudVolumeModeSwitch_InvalidFuncHoldType
 * @tc.number: ReloadLoudVolumeModeSwitch_001
 * @tc.desc  : Test ReloadLoudVolumeModeSwitch returns false when funcHoldType is invalid
 */
HWTEST_F(LoudVolumeManagerUnitTest, ReloadLoudVolumeModeSwitch_InvalidFuncHoldType_001, TestSize.Level1)
{
    bool result = loudVolumeManager_->ReloadLoudVolumeModeSwitch(
        LOUD_VOLUME_MODE_INVALID, LOUD_VOLUME_SWITCH_ON);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : ReloadLoudVolumeModeSwitch_SwitchOnNotInMapMusic
 * @tc.number: ReloadLoudVolumeModeSwitch_002
 * @tc.desc  : Test ReloadLoudVolumeModeSwitch ON with MUSIC not in map sets hold true
 */
HWTEST_F(LoudVolumeManagerUnitTest, ReloadLoudVolumeModeSwitch_SwitchOnNotInMapMusic_002, TestSize.Level1)
{
    loudVolumeManager_->loudVolumeHoldMap_.clear();
    bool result = loudVolumeManager_->ReloadLoudVolumeModeSwitch(
        LOUD_VOLUME_MODE_MUSIC, LOUD_VOLUME_SWITCH_ON);
    EXPECT_TRUE(result);
    bool state = false;
    loudVolumeManager_->GetLoudVolumeHoldMap(LOUD_VOLUME_MODE_MUSIC, state);
    EXPECT_TRUE(state);
}

/**
 * @tc.name  : ReloadLoudVolumeModeSwitch_SwitchOnNotInMapVoice
 * @tc.number: ReloadLoudVolumeModeSwitch_003
 * @tc.desc  : Test ReloadLoudVolumeModeSwitch ON with VOICE not in map sets hold false
 */
HWTEST_F(LoudVolumeManagerUnitTest, ReloadLoudVolumeModeSwitch_SwitchOnNotInMapVoice_003, TestSize.Level1)
{
    loudVolumeManager_->loudVolumeHoldMap_.clear();
    bool result = loudVolumeManager_->ReloadLoudVolumeModeSwitch(
        LOUD_VOLUME_MODE_VOICE, LOUD_VOLUME_SWITCH_ON);
    EXPECT_TRUE(result);
    bool state = true;
    loudVolumeManager_->GetLoudVolumeHoldMap(LOUD_VOLUME_MODE_VOICE, state);
    EXPECT_FALSE(state);
}

/**
 * @tc.name  : ReloadLoudVolumeModeSwitch_SwitchOnAlreadyInMap
 * @tc.number: ReloadLoudVolumeModeSwitch_004
 * @tc.desc  : Test ReloadLoudVolumeModeSwitch ON skips when already in map
 */
HWTEST_F(LoudVolumeManagerUnitTest, ReloadLoudVolumeModeSwitch_SwitchOnAlreadyInMap_004, TestSize.Level1)
{
    loudVolumeManager_->SetLoudVolumeHoldMap(LOUD_VOLUME_MODE_MUSIC, true);
    bool result = loudVolumeManager_->ReloadLoudVolumeModeSwitch(
        LOUD_VOLUME_MODE_MUSIC, LOUD_VOLUME_SWITCH_ON);
    EXPECT_TRUE(result);
}

/**
 * @tc.name  : ReloadLoudVolumeModeSwitch_SwitchOffInMap
 * @tc.number: ReloadLoudVolumeModeSwitch_005
 * @tc.desc  : Test ReloadLoudVolumeModeSwitch OFF clears and sends when in map
 */
HWTEST_F(LoudVolumeManagerUnitTest, ReloadLoudVolumeModeSwitch_SwitchOffInMap_005, TestSize.Level1)
{
    loudVolumeManager_->SetLoudVolumeHoldMap(LOUD_VOLUME_MODE_MUSIC, true);
    bool result = loudVolumeManager_->ReloadLoudVolumeModeSwitch(
        LOUD_VOLUME_MODE_MUSIC, LOUD_VOLUME_SWITCH_OFF);
    EXPECT_TRUE(result);
    EXPECT_EQ(loudVolumeManager_->loudVolumeHoldMap_.count(LOUD_VOLUME_MODE_MUSIC), 0u);
}

/**
 * @tc.name  : ReloadLoudVolumeModeSwitch_SwitchOffNotInMap
 * @tc.number: ReloadLoudVolumeModeSwitch_006
 * @tc.desc  : Test ReloadLoudVolumeModeSwitch OFF skips when not in map
 */
HWTEST_F(LoudVolumeManagerUnitTest, ReloadLoudVolumeModeSwitch_SwitchOffNotInMap_006, TestSize.Level1)
{
    loudVolumeManager_->loudVolumeHoldMap_.clear();
    bool result = loudVolumeManager_->ReloadLoudVolumeModeSwitch(
        LOUD_VOLUME_MODE_MUSIC, LOUD_VOLUME_SWITCH_OFF);
    EXPECT_TRUE(result);
}

/**
 * @tc.name  : ReloadLoudVolumeModeSwitch_SwitchPauseInMap
 * @tc.number: ReloadLoudVolumeModeSwitch_007
 * @tc.desc  : Test ReloadLoudVolumeModeSwitch PAUSE sends false when in map
 */
HWTEST_F(LoudVolumeManagerUnitTest, ReloadLoudVolumeModeSwitch_SwitchPauseInMap_007, TestSize.Level1)
{
    loudVolumeManager_->SetLoudVolumeHoldMap(LOUD_VOLUME_MODE_MUSIC, true);
    bool result = loudVolumeManager_->ReloadLoudVolumeModeSwitch(
        LOUD_VOLUME_MODE_MUSIC, LOUD_VOLUME_SWITCH_PAUSE);
    EXPECT_TRUE(result);
}

/**
 * @tc.name  : ReloadLoudVolumeModeSwitch_SwitchPauseNotInMap
 * @tc.number: ReloadLoudVolumeModeSwitch_008
 * @tc.desc  : Test ReloadLoudVolumeModeSwitch PAUSE skips when not in map
 */
HWTEST_F(LoudVolumeManagerUnitTest, ReloadLoudVolumeModeSwitch_SwitchPauseNotInMap_008, TestSize.Level1)
{
    loudVolumeManager_->loudVolumeHoldMap_.clear();
    bool result = loudVolumeManager_->ReloadLoudVolumeModeSwitch(
        LOUD_VOLUME_MODE_MUSIC, LOUD_VOLUME_SWITCH_PAUSE);
    EXPECT_TRUE(result);
}

/**
 * @tc.name  : ReloadLoudVolumeModeSwitch_SwitchAutoInMapHolding
 * @tc.number: ReloadLoudVolumeModeSwitch_009
 * @tc.desc  : Test ReloadLoudVolumeModeSwitch AUTO sends true when in map and holding
 */
HWTEST_F(LoudVolumeManagerUnitTest, ReloadLoudVolumeModeSwitch_SwitchAutoInMapHolding_009, TestSize.Level1)
{
    loudVolumeManager_->SetLoudVolumeHoldMap(LOUD_VOLUME_MODE_MUSIC, true);
    bool result = loudVolumeManager_->ReloadLoudVolumeModeSwitch(
        LOUD_VOLUME_MODE_MUSIC, LOUD_VOLUME_SWITCH_AUTO);
    EXPECT_TRUE(result);
}

/**
 * @tc.name  : ReloadLoudVolumeModeSwitch_SwitchAutoNotInMap
 * @tc.number: ReloadLoudVolumeModeSwitch_010
 * @tc.desc  : Test ReloadLoudVolumeModeSwitch AUTO logs info when not in map
 */
HWTEST_F(LoudVolumeManagerUnitTest, ReloadLoudVolumeModeSwitch_SwitchAutoNotInMap_010, TestSize.Level1)
{
    loudVolumeManager_->loudVolumeHoldMap_.clear();
    bool result = loudVolumeManager_->ReloadLoudVolumeModeSwitch(
        LOUD_VOLUME_MODE_MUSIC, LOUD_VOLUME_SWITCH_AUTO);
    EXPECT_TRUE(result);
}

/**
 * @tc.name  : ReloadLoudVolumeModeSwitch_SwitchAutoInMapNotHolding
 * @tc.number: ReloadLoudVolumeModeSwitch_011
 * @tc.desc  : Test ReloadLoudVolumeModeSwitch AUTO logs info when in map but not holding
 */
HWTEST_F(LoudVolumeManagerUnitTest, ReloadLoudVolumeModeSwitch_SwitchAutoInMapNotHolding_011, TestSize.Level1)
{
    loudVolumeManager_->SetLoudVolumeHoldMap(LOUD_VOLUME_MODE_MUSIC, false);
    bool result = loudVolumeManager_->ReloadLoudVolumeModeSwitch(
        LOUD_VOLUME_MODE_MUSIC, LOUD_VOLUME_SWITCH_AUTO);
    EXPECT_TRUE(result);
}

/**
 * @tc.name  : ReloadLoudVolumeModeSwitch_DefaultMode
 * @tc.number: ReloadLoudVolumeModeSwitch_012
 * @tc.desc  : Test ReloadLoudVolumeModeSwitch returns false for invalid setVolMode
 */
HWTEST_F(LoudVolumeManagerUnitTest, ReloadLoudVolumeModeSwitch_DefaultMode_012, TestSize.Level1)
{
    bool result = loudVolumeManager_->ReloadLoudVolumeModeSwitch(
        LOUD_VOLUME_MODE_MUSIC, LOUD_VOLUME_SWITCH_INVALID);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : CheckLoudVolumeMode_UpNotAtMaxLevel
 * @tc.number: CheckLoudVolumeMode_009
 * @tc.desc  : Test CheckLoudVolumeMode UP key but not at max level falls to else
 */
HWTEST_F(LoudVolumeManagerUnitTest, CheckLoudVolumeMode_UpNotAtMaxLevel_009, TestSize.Level1)
{
    loudVolumeManager_->loudVolumeSupportMode_ = LOUD_VOLUME_SUPPORT;
    loudVolumeManager_->triggerTime = 2;
    loudVolumeManager_->upTriggerTimeMSec = 1000;
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_SPEAKER;
    device->networkId_ = LOCAL_NETWORK_ID;

    bool result = loudVolumeManager_->CheckLoudVolumeMode(
        1, OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP, STREAM_MUSIC, device);
    EXPECT_FALSE(result);
    EXPECT_EQ(loudVolumeManager_->triggerTime, 0);
    EXPECT_EQ(loudVolumeManager_->upTriggerTimeMSec, 0);
}

/**
 * @tc.name  : CheckLoudVolumeMode_DownNotHolding
 * @tc.number: CheckLoudVolumeMode_010
 * @tc.desc  : Test CheckLoudVolumeMode DOWN key but not holding falls to else
 */
HWTEST_F(LoudVolumeManagerUnitTest, CheckLoudVolumeMode_DownNotHolding_010, TestSize.Level1)
{
    loudVolumeManager_->loudVolumeSupportMode_ = LOUD_VOLUME_SUPPORT;
    loudVolumeManager_->triggerTime = 2;
    loudVolumeManager_->upTriggerTimeMSec = 1000;
    loudVolumeManager_->loudVolumeHoldMap_.clear();
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_SPEAKER;
    device->networkId_ = LOCAL_NETWORK_ID;

    bool result = loudVolumeManager_->CheckLoudVolumeMode(
        5, OHOS::MMI::KeyEvent::KEYCODE_VOLUME_DOWN, STREAM_MUSIC, device);
    EXPECT_FALSE(result);
    EXPECT_EQ(loudVolumeManager_->triggerTime, 0);
    EXPECT_EQ(loudVolumeManager_->upTriggerTimeMSec, 0);
}

/**
 * @tc.name  : CheckLoudVolumeMode_UpTriggerOnReloadFailed
 * @tc.number: CheckLoudVolumeMode_011
 * @tc.desc  : Test CheckLoudVolumeMode UP trigger on but ReloadLoudVolumeMode fails
 */
HWTEST_F(LoudVolumeManagerUnitTest, CheckLoudVolumeMode_UpTriggerOnReloadFailed_011, TestSize.Level1)
{
    loudVolumeManager_->loudVolumeSupportMode_ = LOUD_VOLUME_SUPPORT;
    loudVolumeManager_->triggerTime = 2;
    loudVolumeManager_->upTriggerTimeMSec = INT64_MAX / 2;
    loudVolumeManager_->loudVolumeHoldMap_.clear();
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_SPEAKER;
    device->networkId_ = "RemoteDevice";

    bool result = loudVolumeManager_->CheckLoudVolumeMode(
        15, OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP, STREAM_MUSIC, device);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : CheckLoudVolumeMode_DownReloadFailed
 * @tc.number: CheckLoudVolumeMode_012
 * @tc.desc  : Test CheckLoudVolumeMode DOWN with holding but ReloadLoudVolumeMode fails
 */
HWTEST_F(LoudVolumeManagerUnitTest, CheckLoudVolumeMode_DownReloadFailed_012, TestSize.Level1)
{
    loudVolumeManager_->loudVolumeSupportMode_ = LOUD_VOLUME_SUPPORT;
    loudVolumeManager_->triggerTime = 0;
    loudVolumeManager_->upTriggerTimeMSec = 0;
    loudVolumeManager_->SetLoudVolumeHoldMap(LOUD_VOLUME_MODE_MUSIC, true);
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_SPEAKER;
    device->networkId_ = "RemoteDevice";

    bool result = loudVolumeManager_->CheckLoudVolumeMode(
        5, OHOS::MMI::KeyEvent::KEYCODE_VOLUME_DOWN, STREAM_MUSIC, device);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : CheckLoudVolumeMode_UpMaxTriggerTimeout
 * @tc.number: CheckLoudVolumeMode_013
 * @tc.desc  : Test CheckLoudVolumeMode UP at max with triggerTime=2 and beyond timeout
 */
HWTEST_F(LoudVolumeManagerUnitTest, CheckLoudVolumeMode_UpMaxTriggerTimeout_013, TestSize.Level1)
{
    loudVolumeManager_->loudVolumeSupportMode_ = LOUD_VOLUME_SUPPORT;
    loudVolumeManager_->triggerTime = 2;
    loudVolumeManager_->upTriggerTimeMSec = 0;
    loudVolumeManager_->loudVolumeHoldMap_.clear();
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_SPEAKER;
    device->networkId_ = LOCAL_NETWORK_ID;

    bool result = loudVolumeManager_->CheckLoudVolumeMode(
        15, OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP, STREAM_MUSIC, device);
    EXPECT_FALSE(result);
    EXPECT_EQ(loudVolumeManager_->triggerTime, 1);
}

/**
 * @tc.name  : CheckLoudVolumeMode_UpEnableTriggerWithinMin
 * @tc.number: CheckLoudVolumeMode_014
 * @tc.desc  : Test CheckLoudVolumeMode UP at max with triggerTime=1 and within min time
 */
HWTEST_F(LoudVolumeManagerUnitTest, CheckLoudVolumeMode_UpEnableTriggerWithinMin_014, TestSize.Level1)
{
    loudVolumeManager_->loudVolumeSupportMode_ = LOUD_VOLUME_SUPPORT;
    loudVolumeManager_->triggerTime = 1;
    loudVolumeManager_->upTriggerTimeMSec = INT64_MAX / 2;
    loudVolumeManager_->loudVolumeHoldMap_.clear();
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_SPEAKER;
    device->networkId_ = LOCAL_NETWORK_ID;

    bool result = loudVolumeManager_->CheckLoudVolumeMode(
        15, OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP, STREAM_MUSIC, device);
    EXPECT_FALSE(result);
    EXPECT_EQ(loudVolumeManager_->triggerTime, 1);
}

/**
 * @tc.name  : GetMaxVolumeLevel_StreamAll
 * @tc.number: GetMaxVolumeLevel_001
 * @tc.desc  : Test GetMaxVolumeLevel converts STREAM_ALL to STREAM_MUSIC
 */
HWTEST_F(LoudVolumeManagerUnitTest, GetMaxVolumeLevel_StreamAll_001, TestSize.Level1)
{
    int32_t resultAll = loudVolumeManager_->GetMaxVolumeLevel(
        STREAM_ALL, DEVICE_TYPE_SPEAKER);
    int32_t resultMusic = loudVolumeManager_->GetMaxVolumeLevel(
        STREAM_MUSIC, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(resultAll, resultMusic);
}

/**
 * @tc.name  : GetMaxVolumeLevel_OtherStreamType
 * @tc.number: GetMaxVolumeLevel_002
 * @tc.desc  : Test GetMaxVolumeLevel with non-STREAM_ALL type uses as-is
 */
HWTEST_F(LoudVolumeManagerUnitTest, GetMaxVolumeLevel_OtherStreamType_002, TestSize.Level1)
{
    int32_t result = loudVolumeManager_->GetMaxVolumeLevel(
        STREAM_MUSIC, DEVICE_TYPE_SPEAKER);
    EXPECT_GT(result, 0);
}

/**
 * @tc.name  : PublishLoudVolumeNotification_DlopenFailed
 * @tc.number: PublishLoudVolumeNotification_001
 * @tc.desc  : Test PublishLoudVolumeNotification handles dlopen failure gracefully
 */
HWTEST_F(LoudVolumeManagerUnitTest, PublishLoudVolumeNotification_DlopenFailed_001, TestSize.Level1)
{
    loudVolumeManager_->PublishLoudVolumeNotification(6);
}

/**
 * @tc.name  : SendLoudVolumeMode_StateTrueRepeatTrue
 * @tc.number: SendLoudVolumeMode_001
 * @tc.desc  : Test SendLoudVolumeMode with state=true and repeatTrigNotif=true
 */
HWTEST_F(LoudVolumeManagerUnitTest, SendLoudVolumeMode_StateTrueRepeatTrue_001, TestSize.Level1)
{
    loudVolumeManager_->SendLoudVolumeMode(LOUD_VOLUME_MODE_MUSIC, true, true);
}

/**
 * @tc.name  : SendLoudVolumeMode_StateFalseRepeatFalse
 * @tc.number: SendLoudVolumeMode_002
 * @tc.desc  : Test SendLoudVolumeMode with state=false and repeatTrigNotif=false
 */
HWTEST_F(LoudVolumeManagerUnitTest, SendLoudVolumeMode_StateFalseRepeatFalse_002, TestSize.Level1)
{
    loudVolumeManager_->SendLoudVolumeMode(LOUD_VOLUME_MODE_MUSIC, false, false);
}

/**
 * @tc.name  : SendLoudVolumeMode_StateTrueRepeatFalse
 * @tc.number: SendLoudVolumeMode_003
 * @tc.desc  : Test SendLoudVolumeMode with state=true and repeatTrigNotif=false
 */
HWTEST_F(LoudVolumeManagerUnitTest, SendLoudVolumeMode_StateTrueRepeatFalse_003, TestSize.Level1)
{
    loudVolumeManager_->SendLoudVolumeMode(LOUD_VOLUME_MODE_MUSIC, true, false);
}

/**
 * @tc.name  : SendLoudVolumeMode_StateFalseRepeatTrue
 * @tc.number: SendLoudVolumeMode_004
 * @tc.desc  : Test SendLoudVolumeMode with state=false and repeatTrigNotif=true
 */
HWTEST_F(LoudVolumeManagerUnitTest, SendLoudVolumeMode_StateFalseRepeatTrue_004, TestSize.Level1)
{
    loudVolumeManager_->SendLoudVolumeMode(LOUD_VOLUME_MODE_MUSIC, false, true);
}

} // namespace AudioStandard
} // namespace OHOS
