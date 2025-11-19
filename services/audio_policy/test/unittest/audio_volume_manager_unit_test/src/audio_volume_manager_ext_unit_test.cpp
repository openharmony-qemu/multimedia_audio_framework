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

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: CheckLowerDeviceVolume_001
 * @tc.desc  : Test CheckLowerDeviceVolume.
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, CheckLowerDeviceVolume_001, TestSize.Level4)
{
    auto &audioVolumeManager = AudioVolumeManager::GetInstance();
    DeviceType deviceType = DEVICE_TYPE_NEARLINK;

    audioVolumeManager.SetRestoreVolumeLevel(DEVICE_TYPE_NEARLINK, 0);
    audioVolumeManager.CheckLowerDeviceVolume(deviceType);
    EXPECT_EQ(deviceType, DEVICE_TYPE_NEARLINK);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: CheckRestoreDeviceVolume_001
 * @tc.desc  : Test CheckRestoreDeviceVolume.
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, CheckRestoreDeviceVolume_001, TestSize.Level4)
{
    auto &audioVolumeManager = AudioVolumeManager::GetInstance();
    DeviceType deviceType = DEVICE_TYPE_USB_ARM_HEADSET;

    audioVolumeManager.SetRestoreVolumeLevel(DEVICE_TYPE_NEARLINK, 0);
    auto ret = audioVolumeManager.CheckRestoreDeviceVolume(deviceType);
    EXPECT_NE(ret, ERROR);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: CheckRestoreDeviceVolume_002
 * @tc.desc  : Test CheckRestoreDeviceVolume.
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, CheckRestoreDeviceVolume_002, TestSize.Level4)
{
    auto &audioVolumeManager = AudioVolumeManager::GetInstance();
    DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;

    audioVolumeManager.SetRestoreVolumeLevel(DEVICE_TYPE_NEARLINK, 0);
    auto ret = audioVolumeManager.CheckRestoreDeviceVolume(deviceType);
    EXPECT_NE(ret, ERROR);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: SelectDealSafeVolume_001
 * @tc.desc  : Test SelectDealSafeVolume.
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, SelectDealSafeVolume_001, TestSize.Level4)
{
    auto &audioVolumeManager = AudioVolumeManager::GetInstance();
    AudioStreamType streamType = STREAM_VOICE_CALL;
    int32_t volumeLevel = 5;
    DeviceType deviceType = DEVICE_TYPE_NEARLINK;

    auto ret = audioVolumeManager.SelectDealSafeVolume(streamType, volumeLevel, deviceType);
    EXPECT_NE(ret, -2);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: SelectDealSafeVolume_002
 * @tc.desc  : Test SelectDealSafeVolume.
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, SelectDealSafeVolume_002, TestSize.Level4)
{
    auto &audioVolumeManager = AudioVolumeManager::GetInstance();
    AudioStreamType streamType = STREAM_VOICE_CALL;
    int32_t volumeLevel = 5;
    DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;

    audioVolumeManager.SelectDealSafeVolume(streamType, volumeLevel, deviceType);
    EXPECT_EQ(audioVolumeManager.isBtFirstBoot_, false);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: SelectDealSafeVolume_003
 * @tc.desc  : Test SelectDealSafeVolume.
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, SelectDealSafeVolume_003, TestSize.Level4)
{
    auto &audioVolumeManager = AudioVolumeManager::GetInstance();
    AudioStreamType streamType = STREAM_VOICE_CALL;
    int32_t volumeLevel = 5;
    DeviceType deviceType = DEVICE_TYPE_USB_ARM_HEADSET;

    auto ret = audioVolumeManager.SelectDealSafeVolume(streamType, volumeLevel, deviceType);
    EXPECT_LT(-10, ret);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: SelectDealSafeVolume_004
 * @tc.desc  : Test SelectDealSafeVolume.
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, SelectDealSafeVolume_004, TestSize.Level4)
{
    auto &audioVolumeManager = AudioVolumeManager::GetInstance();
    AudioStreamType streamType = STREAM_VOICE_CALL;
    int32_t volumeLevel = 5;
    DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_SCO;

    auto ret = audioVolumeManager.SelectDealSafeVolume(streamType, volumeLevel, deviceType);
    EXPECT_LT(-10, ret);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: SetRestoreVolumeLevel_001
 * @tc.desc  : Test SetRestoreVolumeLevel.
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, SetRestoreVolumeLevel_001, TestSize.Level4)
{
    auto &audioVolumeManager = AudioVolumeManager::GetInstance();
    int32_t volumeLevel = 5;
    DeviceType deviceType = DEVICE_TYPE_NEARLINK;

    audioVolumeManager.SetRestoreVolumeLevel(deviceType, volumeLevel);
    EXPECT_EQ(volumeLevel, 5);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: SetRestoreVolumeLevel_002
 * @tc.desc  : Test SetRestoreVolumeLevel.
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, SetRestoreVolumeLevel_002, TestSize.Level4)
{
    auto &audioVolumeManager = AudioVolumeManager::GetInstance();
    int32_t volumeLevel = 5;
    DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_SCO;

    audioVolumeManager.SetRestoreVolumeLevel(deviceType, volumeLevel);
    EXPECT_EQ(volumeLevel, 5);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: CheckActiveMusicTime_001
 * @tc.desc  : Test CheckActiveMusicTime.
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, CheckActiveMusicTime_001, TestSize.Level4)
{
    auto &audioVolumeManager = AudioVolumeManager::GetInstance();
    audioVolumeManager.safeStatusSle_ = SAFE_INACTIVE;
    audioVolumeManager.safeVolumeExit_ = false;

    std::string reason = "Defalut";
    std::thread t(&AudioVolumeManager::CheckActiveMusicTime, &audioVolumeManager, reason);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    audioVolumeManager.safeVolumeExit_ = true;
    t.join();
    EXPECT_EQ(audioVolumeManager.safeVolumeExit_, true);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: CheckActiveMusicTime_002
 * @tc.desc  : Test CheckActiveMusicTime.
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, CheckActiveMusicTime_002, TestSize.Level4)
{
    auto &audioVolumeManager = AudioVolumeManager::GetInstance();
    audioVolumeManager.safeStatusSle_ = SAFE_ACTIVE;
    audioVolumeManager.safeVolumeExit_ = false;

    std::string reason = "Defalut";
    std::thread t(&AudioVolumeManager::CheckActiveMusicTime, &audioVolumeManager, reason);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    audioVolumeManager.safeVolumeExit_ = true;
    t.join();
    EXPECT_EQ(audioVolumeManager.safeVolumeExit_, true);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: CheckActiveMusicTime_003
 * @tc.desc  : Test CheckActiveMusicTime.
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, CheckActiveMusicTime_003, TestSize.Level4)
{
    auto &audioVolumeManager = AudioVolumeManager::GetInstance();
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();

    rendererChangeInfo->createrUID = 1001;
    rendererChangeInfo->clientUID = 1001;
    rendererChangeInfo->sessionId = 2001;
    rendererChangeInfo->rendererState = RENDERER_RUNNING;
    AudioStreamCollector &collector = audioVolumeManager.audioSceneManager_.streamCollector_;
    collector.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));

    audioVolumeManager.safeStatusSle_ = SAFE_INACTIVE;
    audioVolumeManager.safeVolumeExit_ = false;
    std::string reason = "Defalut";
    std::thread t(&AudioVolumeManager::CheckActiveMusicTime, &audioVolumeManager, reason);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    audioVolumeManager.safeVolumeExit_ = true;
    t.join();
    EXPECT_EQ(audioVolumeManager.safeVolumeExit_, true);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: CheckActiveMusicTime_004
 * @tc.desc  : Test CheckActiveMusicTime.
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, CheckActiveMusicTime_004, TestSize.Level4)
{
    auto &audioVolumeManager = AudioVolumeManager::GetInstance();
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();

    rendererChangeInfo->createrUID = 1001;
    rendererChangeInfo->clientUID = 1001;
    rendererChangeInfo->sessionId = 2001;
    rendererChangeInfo->rendererState = RENDERER_RUNNING;
    AudioStreamCollector &collector = audioVolumeManager.audioSceneManager_.streamCollector_;
    collector.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));

    audioVolumeManager.safeStatusSle_ = SAFE_ACTIVE;
    audioVolumeManager.safeVolumeExit_ = false;
    std::string reason = "Defalut";
    std::thread t(&AudioVolumeManager::CheckActiveMusicTime, &audioVolumeManager, reason);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    audioVolumeManager.safeVolumeExit_ = true;
    t.join();
    EXPECT_EQ(audioVolumeManager.safeVolumeExit_, true);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: CheckMixActiveMusicTime_001
 * @tc.desc  : Test CheckMixActiveMusicTime.
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, CheckMixActiveMusicTime_001, TestSize.Level4)
{
    auto &audioVolumeManager = AudioVolumeManager::GetInstance();
    audioVolumeManager.activeSafeTimeBt_ = 10000;
    audioVolumeManager.activeSafeTime_ = 10000;
    audioVolumeManager.activeSafeTimeSle_ = 10000;
    int32_t safeVolume = 5;

    auto ret = audioVolumeManager.CheckMixActiveMusicTime(safeVolume);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: CheckBlueToothActiveMusicTime_004
 * @tc.desc  : Test CheckBlueToothActiveMusicTime.
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, CheckBlueToothActiveMusicTime_004, TestSize.Level4)
{
    auto &audioVolumeManager = AudioVolumeManager::GetInstance();
    int32_t safeVolume = 5;

    audioVolumeManager.CheckBlueToothActiveMusicTime(safeVolume);
    EXPECT_EQ(safeVolume, 5);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: DealWithSafeVolume_007
 * @tc.desc  : Test DealWithSafeVolume.
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, DealWithSafeVolume_007, TestSize.Level4)
{
    auto &audioVolumeManager = AudioVolumeManager::GetInstance();
    int32_t volumeLevel = 5;
    bool isBtDevice = false;

    auto ret = audioVolumeManager.DealWithSafeVolume(volumeLevel, isBtDevice);
    EXPECT_LT(-10, ret);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: DealWithSafeVolume_008
 * @tc.desc  : Test DealWithSafeVolume.
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, DealWithSafeVolume_008, TestSize.Level4)
{
    auto &audioVolumeManager = AudioVolumeManager::GetInstance();
    int32_t volumeLevel = 5;
    bool isBtDevice = false;
    audioVolumeManager.safeStatusSle_ = SAFE_ACTIVE;
    audioVolumeManager.safeStatus_ = SAFE_UNKNOWN;

    auto ret = audioVolumeManager.DealWithSafeVolume(volumeLevel, isBtDevice);
    EXPECT_LT(-10, ret);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: CheckNearlinkActiveMusicTime_001
 * @tc.desc  : Test CheckNearlinkActiveMusicTime.
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, CheckNearlinkActiveMusicTime_001, TestSize.Level4)
{
    auto &audioVolumeManager = AudioVolumeManager::GetInstance();
    int32_t volumeLevel = 5;
    audioVolumeManager.startSafeTimeSle_ = 0;

    audioVolumeManager.CheckNearlinkActiveMusicTime(volumeLevel);
    EXPECT_EQ(audioVolumeManager.startSafeTimeBt_, 0);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: CheckNearlinkActiveMusicTime_002
 * @tc.desc  : Test CheckNearlinkActiveMusicTime.
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, CheckNearlinkActiveMusicTime_002, TestSize.Level4)
{
    auto &audioVolumeManager = AudioVolumeManager::GetInstance();
    int32_t volumeLevel = 5;
    audioVolumeManager.startSafeTimeSle_ = 1;

    audioVolumeManager.CheckNearlinkActiveMusicTime(volumeLevel);
    EXPECT_EQ(audioVolumeManager.startSafeTimeBt_, 0);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: CheckNearlinkActiveMusicTime_003
 * @tc.desc  : Test CheckNearlinkActiveMusicTime.
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, CheckNearlinkActiveMusicTime_003, TestSize.Level4)
{
    auto &audioVolumeManager = AudioVolumeManager::GetInstance();
    int32_t volumeLevel = 5;
    audioVolumeManager.activeSafeTimeSle_ = 65000;

    audioVolumeManager.CheckNearlinkActiveMusicTime(volumeLevel);
    EXPECT_EQ(audioVolumeManager.startSafeTimeBt_, 0);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: CheckNearlinkActiveMusicTime_004
 * @tc.desc  : Test CheckNearlinkActiveMusicTime.
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, CheckNearlinkActiveMusicTime_004, TestSize.Level4)
{
    auto &audioVolumeManager = AudioVolumeManager::GetInstance();
    int32_t volumeLevel = 5;
    audioVolumeManager.activeSafeTimeSle_ = 64800;

    audioVolumeManager.CheckNearlinkActiveMusicTime(volumeLevel);
    EXPECT_EQ(audioVolumeManager.startSafeTimeBt_, 0);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: CheckNearlinkActiveMusicTime_005
 * @tc.desc  : Test CheckNearlinkActiveMusicTime.
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, CheckNearlinkActiveMusicTime_005, TestSize.Level4)
{
    auto &audioVolumeManager = AudioVolumeManager::GetInstance();
    int32_t volumeLevel = 5;
    audioVolumeManager.activeSafeTimeSle_ = 1;

    audioVolumeManager.CheckNearlinkActiveMusicTime(volumeLevel);
    EXPECT_EQ(audioVolumeManager.startSafeTimeBt_, 0);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: CheckNearlinkActiveMusicTime_006
 * @tc.desc  : Test CheckNearlinkActiveMusicTime.
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, CheckNearlinkActiveMusicTime_006, TestSize.Level4)
{
    auto &audioVolumeManager = AudioVolumeManager::GetInstance();
    int32_t volumeLevel = 5;
    audioVolumeManager.activeSafeTimeBt_ = 40000;
    audioVolumeManager.activeSafeTime_ = 40000;
    audioVolumeManager.activeSafeTimeSle_ = 40000;

    audioVolumeManager.CheckNearlinkActiveMusicTime(volumeLevel);
    EXPECT_EQ(audioVolumeManager.startSafeTimeBt_, 0);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: CheckNearlinkActiveMusicTime_007
 * @tc.desc  : Test CheckNearlinkActiveMusicTime.
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, CheckNearlinkActiveMusicTime_007, TestSize.Level4)
{
    auto &audioVolumeManager = AudioVolumeManager::GetInstance();
    int32_t volumeLevel = 5;
    audioVolumeManager.startSafeTimeSle_ = 10000;
    audioVolumeManager.activeSafeTimeSle_ = 1;
    audioVolumeManager.activeSafeTimeBt_ = 1;
    audioVolumeManager.activeSafeTime_ = 1;
    audioVolumeManager.activeSafeTimeSle_ = 1;

    audioVolumeManager.CheckNearlinkActiveMusicTime(volumeLevel);
    EXPECT_EQ(audioVolumeManager.startSafeTimeBt_, 0);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: CheckWiredActiveMusicTime_004
 * @tc.desc  : Test CheckWiredActiveMusicTime.
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, CheckWiredActiveMusicTime_004, TestSize.Level4)
{
    auto &audioVolumeManager = AudioVolumeManager::GetInstance();
    int32_t volumeLevel = 5;

    audioVolumeManager.CheckWiredActiveMusicTime(volumeLevel);
    EXPECT_EQ(audioVolumeManager.startSafeTimeBt_, 0);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: CheckLowerDeviceVolume_002
 * @tc.desc  : Test CheckLowerDeviceVolume.
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, CheckLowerDeviceVolume_002, TestSize.Level4)
{
    auto &audioVolumeManager = AudioVolumeManager::GetInstance();
    DeviceType deviceType = DEVICE_TYPE_NEARLINK;

    audioVolumeManager.CheckLowerDeviceVolume(deviceType);
    EXPECT_EQ(deviceType, DEVICE_TYPE_NEARLINK);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: SetDeviceSafeVolumeStatus_001
 * @tc.desc  : Test SetDeviceSafeVolumeStatus.
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, SetDeviceSafeVolumeStatus_001, TestSize.Level4)
{
    auto &audioVolumeManager = AudioVolumeManager::GetInstance();
    audioVolumeManager.audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_NEARLINK;
    audioVolumeManager.userSelect_ = true;

    audioVolumeManager.SetDeviceSafeVolumeStatus();
    EXPECT_EQ(audioVolumeManager.safeStatusSle_, SAFE_INACTIVE);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: ChangeDeviceSafeStatus_001
 * @tc.desc  : Test ChangeDeviceSafeStatus.
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, ChangeDeviceSafeStatus_001, TestSize.Level4)
{
    auto &audioVolumeManager = AudioVolumeManager::GetInstance();

    audioVolumeManager.ChangeDeviceSafeStatus(SAFE_INACTIVE);
    EXPECT_EQ(audioVolumeManager.safeStatusSle_, SAFE_INACTIVE);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: CheckRestoreDeviceVolume_003
 * @tc.desc  : Test CheckRestoreDeviceVolume.
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, CheckRestoreDeviceVolume_003, TestSize.Level4)
{
    auto &audioVolumeManager = AudioVolumeManager::GetInstance();
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    DeviceType deviceType = DEVICE_TYPE_NEARLINK;

    auto ret = audioVolumeManager.CheckRestoreDeviceVolume(deviceType);
    EXPECT_NE(ret, ERROR);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: CheckRestoreDeviceVolume_004
 * @tc.desc  : Test CheckRestoreDeviceVolume.
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, CheckRestoreDeviceVolume_004, TestSize.Level4)
{
    auto &audioVolumeManager = AudioVolumeManager::GetInstance();
    DeviceType deviceType = DEVICE_TYPE_USB_ARM_HEADSET;

    auto ret = audioVolumeManager.CheckRestoreDeviceVolume(deviceType);
    EXPECT_NE(ret, ERROR);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: CheckRestoreDeviceVolume_005
 * @tc.desc  : Test CheckRestoreDeviceVolume.
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, CheckRestoreDeviceVolume_005, TestSize.Level4)
{
    auto &audioVolumeManager = AudioVolumeManager::GetInstance();
    DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;

    auto ret = audioVolumeManager.CheckRestoreDeviceVolume(deviceType);
    EXPECT_NE(ret, ERROR);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: CheckRestoreDeviceVolumeNearlink_001
 * @tc.desc  : Test CheckRestoreDeviceVolumeNearlink.
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, CheckRestoreDeviceVolumeNearlink_001, TestSize.Level4)
{
    auto &audioVolumeManager = AudioVolumeManager::GetInstance();
    auto &audioAdapterManager = AudioAdapterManager::GetInstance();
    int32_t btRestoreVolume = 10;
    int32_t wiredRestoreVolume = 10;
    int32_t sleRestoreVolume = 10;
    int32_t safeVolume = 1;

    auto ret = audioVolumeManager.CheckRestoreDeviceVolumeNearlink(btRestoreVolume, wiredRestoreVolume,
                sleRestoreVolume, safeVolume);
    EXPECT_NE(ret, ERROR);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: CheckRestoreDeviceVolumeNearlink_002
 * @tc.desc  : Test CheckRestoreDeviceVolumeNearlink.
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, CheckRestoreDeviceVolumeNearlink_002, TestSize.Level4)
{
    auto &audioVolumeManager = AudioVolumeManager::GetInstance();
    auto &audioAdapterManager = AudioAdapterManager::GetInstance();
    int32_t btRestoreVolume = 1;
    int32_t wiredRestoreVolume = 1;
    int32_t sleRestoreVolume = 1;
    int32_t safeVolume = 10;

    auto ret = audioVolumeManager.CheckRestoreDeviceVolumeNearlink(btRestoreVolume, wiredRestoreVolume,
                sleRestoreVolume, safeVolume);
    EXPECT_NE(ret, ERROR);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: DealWithEventVolume_001
 * @tc.desc  : Test DealWithEventVolume.
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, DealWithEventVolume_001, TestSize.Level4)
{
    auto &audioVolumeManager = AudioVolumeManager::GetInstance();
    audioVolumeManager.audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_NEARLINK;
    const int32_t notificationId = RESTORE_VOLUME_NOTIFICATION_ID;

    auto ret = audioVolumeManager.DealWithEventVolume(notificationId);
    EXPECT_NE(ret, ERROR);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: DealWithEventVolume_002
 * @tc.desc  : Test DealWithEventVolume.
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, DealWithEventVolume_002, TestSize.Level4)
{
    auto &audioVolumeManager = AudioVolumeManager::GetInstance();
    audioVolumeManager.audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_NEARLINK;
    const int32_t notificationId = INCREASE_VOLUME_NOTIFICATION_ID;

    auto ret = audioVolumeManager.DealWithEventVolume(notificationId);
    EXPECT_NE(ret, ERROR);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: DealWithEventVolume_003
 * @tc.desc  : Test DealWithEventVolume.
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, DealWithEventVolume_003, TestSize.Level4)
{
    auto &audioVolumeManager = AudioVolumeManager::GetInstance();
    audioVolumeManager.audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_NEARLINK;
    const int32_t notificationId = 1;

    auto ret = audioVolumeManager.DealWithEventVolume(notificationId);
    EXPECT_NE(ret, ERROR);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: DealWithEventVolume_004
 * @tc.desc  : Test DealWithEventVolume.
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, DealWithEventVolume_004, TestSize.Level4)
{
    auto &audioVolumeManager = AudioVolumeManager::GetInstance();
    audioVolumeManager.audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_HDMI;
    const int32_t notificationId = 1;

    auto ret = audioVolumeManager.DealWithEventVolume(notificationId);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: IsNearLink_001
 * @tc.desc  : Test IsNearLink.
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, IsNearLink_001, TestSize.Level4)
{
    auto &audioVolumeManager = AudioVolumeManager::GetInstance();
    DeviceType deviceType = DEVICE_TYPE_NEARLINK;

    auto ret = audioVolumeManager.IsNearLink(deviceType);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: IsNearLink_002
 * @tc.desc  : Test IsNearLink.
 */
HWTEST_F(AudioVolumeManagerExtUnitTest, IsNearLink_002, TestSize.Level4)
{
    auto &audioVolumeManager = AudioVolumeManager::GetInstance();
    DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;

    auto ret = audioVolumeManager.IsNearLink(deviceType);
    EXPECT_EQ(ret, false);
}
} // namespace AudioStandard
} // namespace OHOS