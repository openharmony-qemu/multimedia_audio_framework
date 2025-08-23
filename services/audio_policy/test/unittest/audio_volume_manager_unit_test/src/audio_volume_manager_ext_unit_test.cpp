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

#include "sle_audio_device_manager.h"
#include "audio_volume_manager_ext_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
const int32_t RESTORE_VOLUME_NOTIFICATION_ID = 116000;
const int32_t INCREASE_VOLUME_NOTIFICATION_ID = 116001;
const uint32_t NOTIFICATION_BANNER_FLAG = 1 << 9;
const std::string AUDIO_RESTORE_VOLUME_EVENT = "AUDIO_RESTORE_VOLUME_EVENT";
const std::string AUDIO_INCREASE_VOLUME_EVENT = "AUDIO_INCREASE_VOLUME_EVENT";

void AudioVolumeManagerExtUnitTest::SetUpTestCase(void) {}
void AudioVolumeManagerExtUnitTest::TearDownTestCase(void) {}

void AudioVolumeManagerExtUnitTest::SetUp(void)
{
    std::shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler(
        DelayedSingleton<AudioPolicyServerHandler>::GetInstance());
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.Init(audioPolicyServerHandler);
}

void AudioVolumeManagerExtUnitTest::TearDown(void)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.DeInit();
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: GetSystemVolumeLevel_001
* @tc.desc  : Test GetSystemVolumeLevel interface.
*/
HWTEST_F(AudioVolumeManagerExtUnitTest, GetSystemVolumeLevel_001, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    ASSERT_TRUE(audioVolumeManager != nullptr);

    int32_t zoneId = 0;
    AudioStreamType streamType = STREAM_RING;
    audioVolumeManager->audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    audioVolumeManager->SetRingerModeMute(false);
    int32_t ret = audioVolumeManager->GetSystemVolumeLevel(streamType, zoneId);
    EXPECT_EQ(ret, 0);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: HandleA2dpAbsVolume_001
* @tc.desc  : Test HandleA2dpAbsVolume interface.
*/
HWTEST_F(AudioVolumeManagerExtUnitTest, HandleA2dpAbsVolume_001, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    ASSERT_TRUE(audioVolumeManager != nullptr);

    int32_t volumeLevel = 0;
    AudioStreamType streamType = STREAM_RING;
    DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    int32_t ret = audioVolumeManager->HandleA2dpAbsVolume(streamType, volumeLevel, deviceType);
    EXPECT_NE(ret, 0);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: HandleNearlinkDeviceAbsVolume_001
* @tc.desc  : Test HandleNearlinkDeviceAbsVolume interface.
*/
HWTEST_F(AudioVolumeManagerExtUnitTest, HandleNearlinkDeviceAbsVolume_001, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    ASSERT_TRUE(audioVolumeManager != nullptr);

    DeviceType curOutputDeviceType = DEVICE_TYPE_SPEAKER;
    AudioStreamType streamType = STREAM_MUSIC;
    int32_t volumeLevel = 1000;
    bool internalCall = true;
    SleVolumeConfigInfo configInfo;
    std::pair<SleVolumeConfigInfo, SleVolumeConfigInfo> pairConfigInfo = std::make_pair(configInfo, configInfo);
    SleAudioDeviceManager::GetInstance().deviceVolumeConfigInfo_["test"] = pairConfigInfo;
    auto ret = audioVolumeManager->HandleNearlinkDeviceAbsVolume(streamType, volumeLevel, curOutputDeviceType);
    EXPECT_NE(ret, ERROR);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: HandleNearlinkDeviceAbsVolume_002
* @tc.desc  : Test HandleNearlinkDeviceAbsVolume interface.
*/
HWTEST_F(AudioVolumeManagerExtUnitTest, HandleNearlinkDeviceAbsVolume_002, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    ASSERT_TRUE(audioVolumeManager != nullptr);

    DeviceType curOutputDeviceType = DEVICE_TYPE_SPEAKER;
    std::string macAddress = "test";
    AudioStreamType streamType = STREAM_RING;
    int32_t volumeLevel = 1000;
    SleVolumeConfigInfo configInfo;
    std::pair<SleVolumeConfigInfo, SleVolumeConfigInfo> pairConfigInfo = std::make_pair(configInfo, configInfo);
    SleAudioDeviceManager::GetInstance().deviceVolumeConfigInfo_["test"] = pairConfigInfo;
    auto ret = audioVolumeManager->HandleNearlinkDeviceAbsVolume(streamType, volumeLevel, curOutputDeviceType);
    EXPECT_NE(ret, ERROR);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: SetSystemVolumeLevel_001
* @tc.desc  : Test SetSystemVolumeLevel.
*/
HWTEST_F(AudioVolumeManagerExtUnitTest, SetSystemVolumeLevel_001, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    EXPECT_NE(audioVolumeManager, nullptr);

    audioVolumeManager->audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    AudioStreamType streamType = STREAM_RING;
    int32_t volumeLevel = 0;
    EXPECT_EQ(VolumeUtils::GetVolumeTypeFromStreamType(streamType), STREAM_RING);

    auto ret = audioVolumeManager->SetSystemVolumeLevel(streamType, volumeLevel);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: CheckBlueToothActiveMusicTime_001
 * @tc.desc  : CheckBlueToothActiveMusicTime
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, CheckBlueToothActiveMusicTime_001, TestSize.Level1)
{
    auto& volumeManager = AudioVolumeManager::GetInstance();
    volumeManager.activeSafeTimeBt_ = 68400;
    volumeManager.CheckBlueToothActiveMusicTime(0);
    EXPECT_TRUE(volumeManager.restoreNIsShowing_);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: CheckBlueToothActiveMusicTime_002
 * @tc.desc  : CheckBlueToothActiveMusicTime
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, CheckBlueToothActiveMusicTime_002, TestSize.Level1)
{
    auto& volumeManager = AudioVolumeManager::GetInstance();
    volumeManager.activeSafeTimeBt_ = 0;
    volumeManager.activeSafeTime_ = 68400;
    volumeManager.CheckBlueToothActiveMusicTime(0);
    EXPECT_TRUE(volumeManager.restoreNIsShowing_);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: CheckBlueToothActiveMusicTime_003
 * @tc.desc  : CheckBlueToothActiveMusicTime
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, CheckBlueToothActiveMusicTime_003, TestSize.Level1)
{
    auto& volumeManager = AudioVolumeManager::GetInstance();
    int32_t currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    volumeManager.startSafeTimeBt_ = currentTime - 60;
    volumeManager.CheckBlueToothActiveMusicTime(0);
    EXPECT_GT(volumeManager.startSafeTimeBt_, currentTime - 60);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: CheckWiredActiveMusicTime_001
 * @tc.desc  : CheckWiredActiveMusicTime
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, CheckWiredActiveMusicTime_001, TestSize.Level1)
{
    auto& volumeManager = AudioVolumeManager::GetInstance();
    volumeManager.activeSafeTime_ = 68400;
    volumeManager.CheckWiredActiveMusicTime(0);
    EXPECT_TRUE(volumeManager.restoreNIsShowing_);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: CheckWiredActiveMusicTime_002
 * @tc.desc  : CheckWiredActiveMusicTime
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, CheckWiredActiveMusicTime_002, TestSize.Level1)
{
    auto& volumeManager = AudioVolumeManager::GetInstance();
    volumeManager.activeSafeTimeBt_ = 68400;
    volumeManager.activeSafeTime_ = 0;
    volumeManager.CheckWiredActiveMusicTime(0);
    EXPECT_TRUE(volumeManager.restoreNIsShowing_);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: CheckWiredActiveMusicTime_003
 * @tc.desc  : CheckWiredActiveMusicTime
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, CheckWiredActiveMusicTime_003, TestSize.Level1)
{
    auto& volumeManager = AudioVolumeManager::GetInstance();
    int32_t currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    volumeManager.startSafeTime_ = currentTime - 60;
    volumeManager.CheckWiredActiveMusicTime(0);
    EXPECT_GT(volumeManager.startSafeTime_, currentTime - 60);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: HandleAbsBluetoothVolume_004
* @tc.desc  : Test HandleAbsBluetoothVolume interface.
*/
HWTEST_F(AudioVolumeManagerExtUnitTest, HandleAbsBluetoothVolume_004, TestSize.Level1)
{
    std::string macAddress = "11:22:33:44:55:66";
    int32_t volumeLevel = 0;
    int32_t ret;
    AudioStreamType streamType = STREAM_MUSIC;
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.isBtFirstBoot_ = false;
    ret = audioVolumeManager.HandleAbsBluetoothVolume(macAddress, volumeLevel, true, streamType);
    EXPECT_EQ(ret, 0);

    audioVolumeManager.isBtFirstBoot_ = false;
    audioVolumeManager.audioActiveDevice_.currentActiveDevice_.deviceCategory_ = BT_HEADPHONE;
    ret = audioVolumeManager.HandleAbsBluetoothVolume(macAddress, volumeLevel, false, streamType);
    EXPECT_EQ(ret, 0);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: DealWithSafeVolume_001
* @tc.desc  : Test DealWithSafeVolume interface.
*/
HWTEST_F(AudioVolumeManagerExtUnitTest, DealWithSafeVolume_001, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    EXPECT_NE(audioVolumeManager, nullptr);
    int32_t volumeLevel = 0;
    audioVolumeManager->audioActiveDevice_.currentActiveDevice_.deviceCategory_ = BT_CAR;
    int32_t ret = audioVolumeManager->DealWithSafeVolume(volumeLevel, true);
    EXPECT_EQ(ret, 0);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: DealWithSafeVolume_002
* @tc.desc  : Test DealWithSafeVolume interface.
*/
HWTEST_F(AudioVolumeManagerExtUnitTest, DealWithSafeVolume_002, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    EXPECT_NE(audioVolumeManager, nullptr);
    int32_t volumeLevel = 0;
    audioVolumeManager->audioActiveDevice_.currentActiveDevice_.deviceCategory_ = BT_SOUNDBOX;
    int32_t ret = audioVolumeManager->DealWithSafeVolume(volumeLevel, true);
    EXPECT_EQ(ret, 0);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: DealWithSafeVolume_003
* @tc.desc  : Test DealWithSafeVolume interface.
*/
HWTEST_F(AudioVolumeManagerExtUnitTest, DealWithSafeVolume_003, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    EXPECT_NE(audioVolumeManager, nullptr);
    int32_t volumeLevel = 0;
    audioVolumeManager->audioActiveDevice_.currentActiveDevice_.deviceCategory_ = BT_HEADPHONE;
    audioVolumeManager->safeStatusBt_ = SAFE_INACTIVE;
    int32_t ret = audioVolumeManager->DealWithSafeVolume(volumeLevel, true);
    EXPECT_EQ(ret, 0);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: DealWithSafeVolume_004
* @tc.desc  : Test DealWithSafeVolume interface.
*/
HWTEST_F(AudioVolumeManagerExtUnitTest, DealWithSafeVolume_004, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    EXPECT_NE(audioVolumeManager, nullptr);
    int32_t volumeLevel = 0;
    audioVolumeManager->audioActiveDevice_.currentActiveDevice_.deviceCategory_ = BT_HEADPHONE;
    audioVolumeManager->safeStatus_ = SAFE_INACTIVE;
    int32_t ret = audioVolumeManager->DealWithSafeVolume(volumeLevel, false);
    EXPECT_EQ(ret, 0);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: DealWithSafeVolume_005
* @tc.desc  : Test DealWithSafeVolume interface.
*/
HWTEST_F(AudioVolumeManagerExtUnitTest, DealWithSafeVolume_005, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    EXPECT_NE(audioVolumeManager, nullptr);
    int32_t volumeLevel = 0;
    audioVolumeManager->audioActiveDevice_.currentActiveDevice_.deviceCategory_ = BT_HEADPHONE;
    audioVolumeManager->safeStatus_ = SAFE_ACTIVE;
    int32_t ret = audioVolumeManager->DealWithSafeVolume(volumeLevel, false);
    EXPECT_EQ(ret, 0);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: DealWithSafeVolume_006
* @tc.desc  : Test DealWithSafeVolume interface.
*/
HWTEST_F(AudioVolumeManagerExtUnitTest, DealWithSafeVolume_006, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    EXPECT_NE(audioVolumeManager, nullptr);
    int32_t volumeLevel = 0;
    audioVolumeManager->audioActiveDevice_.currentActiveDevice_.deviceCategory_ = BT_HEADPHONE;
    audioVolumeManager->safeStatusBt_ = SAFE_ACTIVE;
    int32_t ret = audioVolumeManager->DealWithSafeVolume(volumeLevel, true);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: GetForceControlVolumeType_001
 * @tc.desc  : Test GetForceControlVolumeType.
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, GetForceControlVolumeType_001, TestSize.Level4)
{
    auto &audioVolumeManager = AudioVolumeManager::GetInstance();
    ASSERT_EQ(audioVolumeManager.GetForceControlVolumeType(), STREAM_DEFAULT);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: IsNeedForceControlVolumeType_001
 * @tc.desc  : Test IsNeedForceControlVolumeType.
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, IsNeedForceControlVolumeType_001, TestSize.Level4)
{
    auto &audioVolumeManager = AudioVolumeManager::GetInstance();
    ASSERT_EQ(audioVolumeManager.IsNeedForceControlVolumeType(), false);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: GetVolumeGroupInfosNotWait_001
 * @tc.desc  : Test GetVolumeGroupInfosNotWait.
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, GetVolumeGroupInfosNotWait_001, TestSize.Level4)
{
    auto &audioVolumeManager = AudioVolumeManager::GetInstance();
    std::vector<sptr<VolumeGroupInfo>> infos;
    ASSERT_EQ(audioVolumeManager.GetVolumeGroupInfosNotWait(infos), false);
}
} // namespace AudioStandard
} // namespace OHOS