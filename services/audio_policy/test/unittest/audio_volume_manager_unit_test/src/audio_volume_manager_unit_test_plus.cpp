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

#include "audio_volume_manager_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_100
* @tc.desc  : Test AudioVolumeManager.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_100, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    EXPECT_NE(audioVolumeManager, nullptr);

    audioVolumeManager->calculateLoopSafeTime_ = nullptr;
    audioVolumeManager->safeVolumeDialogThrd_ = std::make_unique<std::thread>([]() { return; });
    EXPECT_NE(audioVolumeManager->safeVolumeDialogThrd_, nullptr);
    EXPECT_EQ(audioVolumeManager->safeVolumeDialogThrd_->joinable(), true);
    audioVolumeManager->DeInit();
    EXPECT_EQ(audioVolumeManager->safeVolumeDialogThrd_, nullptr);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_101
* @tc.desc  : Test AudioVolumeManager.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_101, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    EXPECT_NE(audioVolumeManager, nullptr);
    audioVolumeManager->audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioVolumeManager->audioPolicyServerHandler_, nullptr);

    audioVolumeManager->calculateLoopSafeTime_ = std::make_unique<std::thread>();
    EXPECT_NE(audioVolumeManager->calculateLoopSafeTime_, nullptr);
    EXPECT_EQ(audioVolumeManager->calculateLoopSafeTime_->joinable(), false);

    audioVolumeManager->safeVolumeDialogThrd_ = std::make_unique<std::thread>();
    EXPECT_NE(audioVolumeManager->safeVolumeDialogThrd_, nullptr);
    EXPECT_EQ(audioVolumeManager->safeVolumeDialogThrd_->joinable(), false);
    audioVolumeManager->DeInit();
    EXPECT_EQ(audioVolumeManager->audioPolicyServerHandler_, nullptr);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_102
* @tc.desc  : Test AudioVolumeManager.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_102, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    EXPECT_NE(audioVolumeManager, nullptr);
    audioVolumeManager->audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioVolumeManager->audioPolicyServerHandler_, nullptr);

    audioVolumeManager->calculateLoopSafeTime_ = std::make_unique<std::thread>();
    EXPECT_NE(audioVolumeManager->calculateLoopSafeTime_, nullptr);
    EXPECT_EQ(audioVolumeManager->calculateLoopSafeTime_->joinable(), false);

    audioVolumeManager->safeVolumeDialogThrd_ = nullptr;
    audioVolumeManager->DeInit();
    EXPECT_EQ(audioVolumeManager->audioPolicyServerHandler_, nullptr);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_103
* @tc.desc  : Test AudioVolumeManager.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_103, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    EXPECT_NE(audioVolumeManager, nullptr);

    AudioStreamType streamType = STREAM_MUSIC;
    int32_t volumeLevel = -1;
    audioVolumeManager->audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;
    audioVolumeManager->increaseNIsShowing_ = true;
    audioVolumeManager->restoreNIsShowing_ = true;
    EXPECT_EQ(VolumeUtils::GetVolumeTypeFromStreamType(streamType), STREAM_MUSIC);

    audioVolumeManager->CheckToCloseNotification(streamType, volumeLevel);
    EXPECT_EQ(audioVolumeManager->increaseNIsShowing_, false);
    EXPECT_EQ(audioVolumeManager->restoreNIsShowing_, false);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_104
* @tc.desc  : Test AudioVolumeManager.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_104, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    EXPECT_NE(audioVolumeManager, nullptr);

    AudioStreamType streamType = STREAM_MUSIC;
    int32_t volumeLevel = -1;
    audioVolumeManager->audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;
    audioVolumeManager->increaseNIsShowing_ = false;
    audioVolumeManager->restoreNIsShowing_ = false;
    EXPECT_EQ(VolumeUtils::GetVolumeTypeFromStreamType(streamType), STREAM_MUSIC);

    audioVolumeManager->CheckToCloseNotification(streamType, volumeLevel);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: AudioVolumeManager_105
 * @tc.desc  : Test AudioVolumeManager.
 */
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_105, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    EXPECT_NE(audioVolumeManager, nullptr);

    AudioStreamType streamType = STREAM_VOICE_CALL;
    int32_t volumeLevel = -1;
    audioVolumeManager->audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;
    EXPECT_NE(VolumeUtils::GetVolumeTypeFromStreamType(streamType), STREAM_MUSIC);

    StreamUsage streamUsage = STREAM_USAGE_MUSIC;
    audioVolumeManager->CheckToCloseNotification(streamType, volumeLevel);
    AudioVolumeType audioVolumeType = VolumeUtils::GetVolumeTypeFromStreamUsage(streamUsage);
    EXPECT_EQ(audioVolumeType, STREAM_MUSIC);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_106
* @tc.desc  : Test AudioVolumeManager.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_106, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    EXPECT_NE(audioVolumeManager, nullptr);

    AudioStreamType streamType = STREAM_VOICE_CALL;
    int32_t volumeLevel = -1;
    audioVolumeManager->audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_MAX;
    EXPECT_NE(VolumeUtils::GetVolumeTypeFromStreamType(streamType), STREAM_MUSIC);
    EXPECT_EQ(audioVolumeManager->DeviceIsSupportSafeVolume(), false);

    audioVolumeManager->CheckToCloseNotification(streamType, volumeLevel);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_107
* @tc.desc  : Test AudioVolumeManager.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_107, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    EXPECT_NE(audioVolumeManager, nullptr);

    AudioStreamType streamType = STREAM_VOICE_CALL;
    int32_t volumeLevel = 1;
    audioVolumeManager->audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_MAX;
    EXPECT_NE(VolumeUtils::GetVolumeTypeFromStreamType(streamType), STREAM_MUSIC);
    EXPECT_EQ(audioVolumeManager->DeviceIsSupportSafeVolume(), false);

    audioVolumeManager->CheckToCloseNotification(streamType, volumeLevel);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_108
* @tc.desc  : Test AudioVolumeManager.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_108, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    EXPECT_NE(audioVolumeManager, nullptr);

    audioVolumeManager->audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioVolumeManager->audioActiveDevice_.currentActiveDevice_.deviceCategory_ = CATEGORY_DEFAULT;

    auto ret = audioVolumeManager->DeviceIsSupportSafeVolume();
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_109
* @tc.desc  : Test AudioVolumeManager.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_109, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    EXPECT_NE(audioVolumeManager, nullptr);

    audioVolumeManager->audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioVolumeManager->audioActiveDevice_.currentActiveDevice_.deviceCategory_ = BT_CAR;

    auto ret = audioVolumeManager->DeviceIsSupportSafeVolume();
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_110
* @tc.desc  : Test AudioVolumeManager.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_110, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    EXPECT_NE(audioVolumeManager, nullptr);

    audioVolumeManager->audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioVolumeManager->audioActiveDevice_.currentActiveDevice_.deviceCategory_ = BT_SOUNDBOX;

    auto ret = audioVolumeManager->DeviceIsSupportSafeVolume();
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_111
* @tc.desc  : Test AudioVolumeManager.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_111, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    EXPECT_NE(audioVolumeManager, nullptr);

    audioVolumeManager->audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_WIRED_HEADSET;

    auto ret = audioVolumeManager->DeviceIsSupportSafeVolume();
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_112
* @tc.desc  : Test AudioVolumeManager.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_112, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    EXPECT_NE(audioVolumeManager, nullptr);

    audioVolumeManager->audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;

    AudioStreamType streamType = STREAM_MUSIC;
    int32_t volumeLevel = 0;
    EXPECT_EQ(VolumeUtils::GetVolumeTypeFromStreamType(streamType), STREAM_MUSIC);

    auto ret = audioVolumeManager->SetSystemVolumeLevel(streamType, volumeLevel);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_113
* @tc.desc  : Test AudioVolumeManager.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_113, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    EXPECT_NE(audioVolumeManager, nullptr);

    audioVolumeManager->audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;

    AudioStreamType streamType = STREAM_ALL;
    int32_t volumeLevel = 0;
    EXPECT_EQ(VolumeUtils::GetVolumeTypeFromStreamType(streamType), STREAM_ALL);

    auto ret = audioVolumeManager->SetSystemVolumeLevel(streamType, volumeLevel);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_114
* @tc.desc  : Test AudioVolumeManager.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_114, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    EXPECT_NE(audioVolumeManager, nullptr);

    std::string macAddress = "11:22:33:44:55:66";
    int32_t volumeLevel = 1;
    bool internalCall = true;

    EXPECT_NE(audioVolumeManager->audioA2dpDevice_.SetA2dpDeviceVolumeLevel(macAddress, volumeLevel), true);

    auto ret = audioVolumeManager->SetA2dpDeviceVolume(macAddress, volumeLevel, internalCall);
    EXPECT_NE(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_115
* @tc.desc  : Test AudioVolumeManager.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_115, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    EXPECT_NE(audioVolumeManager, nullptr);

    std::string macAddress = "11:22:33:44:55:66";
    int32_t volumeLevel = 1;
    bool internalCall = false;

    EXPECT_NE(audioVolumeManager->audioA2dpDevice_.SetA2dpDeviceVolumeLevel(macAddress, volumeLevel), true);

    auto ret = audioVolumeManager->SetA2dpDeviceVolume(macAddress, volumeLevel, internalCall);
    EXPECT_NE(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_116
* @tc.desc  : Test AudioVolumeManager.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_116, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    EXPECT_NE(audioVolumeManager, nullptr);

    std::string macAddress = "11:22:33:44:55:66";
    int32_t volumeLevel = -1;
    bool internalCall = false;

    EXPECT_NE(audioVolumeManager->audioA2dpDevice_.SetA2dpDeviceVolumeLevel(macAddress, volumeLevel), true);

    auto ret = audioVolumeManager->SetA2dpDeviceVolume(macAddress, volumeLevel, internalCall);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: AudioVolumeManager_117
 * @tc.desc  : Test AudioVolumeManager.
 */
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_117, TestSize.Level1)
{
    VolumeUtils::SetPCVolumeEnable(true);
    std::set<StreamUsage> streamUsages = VolumeUtils::GetStreamUsageSetForVolumeType(STREAM_MUSIC);
    EXPECT_FALSE(streamUsages.empty());

    VolumeUtils::SetPCVolumeEnable(false);
    streamUsages = VolumeUtils::GetStreamUsageSetForVolumeType(STREAM_MUSIC);
    EXPECT_FALSE(streamUsages.empty());
}


/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: AudioVolumeManager_118
 * @tc.desc  : Test AudioVolumeManager.
 */
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_118, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<ForceControlVolumeTypeMonitor>();
    EXPECT_NE(audioVolumeManager, nullptr);

    EXPECT_NO_THROW(audioVolumeManager->SetTimer(-1, nullptr));
    EXPECT_NO_THROW(audioVolumeManager->SetTimer(1, nullptr));
}
} // namespace AudioStandard
} // namespace OHOS