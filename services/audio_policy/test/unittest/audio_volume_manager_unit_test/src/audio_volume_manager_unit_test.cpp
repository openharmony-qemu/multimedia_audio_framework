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
const int32_t RESTORE_VOLUME_NOTIFICATION_ID = 116000;
const int32_t INCREASE_VOLUME_NOTIFICATION_ID = 116001;
const uint32_t NOTIFICATION_BANNER_FLAG = 1 << 9;

void AudioVolumeManagerUnitTest::SetUpTestCase(void) {}
void AudioVolumeManagerUnitTest::TearDownTestCase(void) {}

void AudioVolumeManagerUnitTest::SetUp(void)
{
    std::shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler(
        DelayedSingleton<AudioPolicyServerHandler>::GetInstance());
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.Init(audioPolicyServerHandler);
}

void AudioVolumeManagerUnitTest::TearDown(void)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.DeInit();
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_002
* @tc.desc  : Test InitSharedVolume interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_002, TestSize.Level1)
{
    std::shared_ptr<AudioSharedMemory> buffer;
    int32_t ret;
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    ret = audioVolumeManager.InitSharedVolume(buffer);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_003
* @tc.desc  : Test SetVoiceRingtoneMute interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_003, TestSize.Level1)
{
    bool isMute = true;
    int32_t ret;
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    ret = audioVolumeManager.SetVoiceRingtoneMute(isMute);
    EXPECT_EQ(ret, SUCCESS);

    isMute = false;
    ret = audioVolumeManager.SetVoiceRingtoneMute(isMute);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_004
* @tc.desc  : Test HandleAbsBluetoothVolume interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_004, TestSize.Level1)
{
    std::string macAddress = "11:22:33:44:55:66";
    int32_t volumeLevel = 0;
    int32_t ret;
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.isBtFirstBoot_ = true;
    ret = audioVolumeManager.HandleAbsBluetoothVolume(macAddress, volumeLevel);
    EXPECT_EQ(ret, 0);

    audioVolumeManager.isBtFirstBoot_ = false;
    audioVolumeManager.audioActiveDevice_.currentActiveDevice_.deviceCategory_ = BT_CAR;
    ret = audioVolumeManager.HandleAbsBluetoothVolume(macAddress, volumeLevel);
    EXPECT_EQ(ret, 0);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_005
* @tc.desc  : Test IsWiredHeadSet interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_005, TestSize.Level1)
{
    DeviceType deviceType = DEVICE_TYPE_NONE;
    bool bRet;
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    bRet = audioVolumeManager.IsWiredHeadSet(deviceType);
    EXPECT_EQ(bRet, false);

    deviceType = DEVICE_TYPE_WIRED_HEADSET;
    bRet = audioVolumeManager.IsWiredHeadSet(deviceType);
    EXPECT_EQ(bRet, true);

    deviceType = DEVICE_TYPE_WIRED_HEADPHONES;
    bRet = audioVolumeManager.IsWiredHeadSet(deviceType);
    EXPECT_EQ(bRet, true);

    deviceType = DEVICE_TYPE_USB_HEADSET;
    bRet = audioVolumeManager.IsWiredHeadSet(deviceType);
    EXPECT_EQ(bRet, true);

    deviceType = DEVICE_TYPE_USB_ARM_HEADSET;
    bRet = audioVolumeManager.IsWiredHeadSet(deviceType);
    EXPECT_EQ(bRet, true);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_006
* @tc.desc  : Test IsBlueTooth interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_006, TestSize.Level1)
{
    DeviceType deviceType = DEVICE_TYPE_NONE;
    bool bRet;
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    bRet = audioVolumeManager.IsBlueTooth(deviceType);
    EXPECT_EQ(bRet, false);

    audioVolumeManager.audioActiveDevice_.currentActiveDevice_.deviceCategory_ = BT_CAR;
    deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    bRet = audioVolumeManager.IsBlueTooth(deviceType);
    EXPECT_EQ(bRet, true);

    audioVolumeManager.audioActiveDevice_.currentActiveDevice_.deviceCategory_ = BT_SOUNDBOX;
    deviceType = DEVICE_TYPE_BLUETOOTH_SCO;
    bRet = audioVolumeManager.IsBlueTooth(deviceType);
    EXPECT_EQ(bRet, true);

    audioVolumeManager.audioActiveDevice_.currentActiveDevice_.deviceCategory_ = CATEGORY_DEFAULT;
    deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    bRet = audioVolumeManager.IsBlueTooth(deviceType);
    EXPECT_EQ(bRet, true);

    deviceType = DEVICE_TYPE_BLUETOOTH_SCO;
    bRet = audioVolumeManager.IsBlueTooth(deviceType);
    EXPECT_EQ(bRet, true);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_007
* @tc.desc  : Test CheckMixActiveMusicTime interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_007, TestSize.Level1)
{
    int32_t safeVolume = 0;
    bool bRet;
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.activeSafeTimeBt_ = 0;
    audioVolumeManager.activeSafeTime_ = 0;
    bRet = audioVolumeManager.CheckMixActiveMusicTime(safeVolume);
    EXPECT_EQ(bRet, false);

    audioVolumeManager.activeSafeTimeBt_ = 100000;
    audioVolumeManager.activeSafeTime_ = 100000;
    bRet = audioVolumeManager.CheckMixActiveMusicTime(safeVolume);
    EXPECT_EQ(bRet, true);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_008
* @tc.desc  : Test CheckBlueToothActiveMusicTime interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_008, TestSize.Level1)
{
    int32_t safeVolume = 0;
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.startSafeTimeBt_ = 0;
    audioVolumeManager.activeSafeTimeBt_ = 10;
    audioVolumeManager.activeSafeTime_ = 100;
    audioVolumeManager.CheckBlueToothActiveMusicTime(safeVolume);
    EXPECT_EQ(audioVolumeManager.startSafeTime_, 0);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_009
* @tc.desc  : Test CheckWiredActiveMusicTime interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_009, TestSize.Level1)
{
    int32_t safeVolume = 0;
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.startSafeTime_ = 0;
    audioVolumeManager.activeSafeTimeBt_ = 10;
    audioVolumeManager.activeSafeTime_ = 100;
    audioVolumeManager.CheckWiredActiveMusicTime(safeVolume);
    EXPECT_EQ(audioVolumeManager.startSafeTimeBt_, 0);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_010
* @tc.desc  : Test RestoreSafeVolume interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_010, TestSize.Level1)
{
    AudioStreamType streamType = STREAM_RING;
    int32_t safeVolume = 0;
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.RestoreSafeVolume(streamType, safeVolume);
    EXPECT_EQ(audioVolumeManager.GetSystemVolumeLevel(streamType), 0);

    safeVolume = -1;
    audioVolumeManager.RestoreSafeVolume(streamType, safeVolume);
    EXPECT_EQ(audioVolumeManager.GetSystemVolumeLevel(streamType), 0);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_011
* @tc.desc  : Test SetSafeVolumeCallback interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_011, TestSize.Level1)
{
    AudioStreamType streamType = STREAM_MUSIC;
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    audioVolumeManager.audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    ASSERT_TRUE(audioVolumeManager.audioPolicyServerHandler_ != nullptr);

    audioVolumeManager.SetSafeVolumeCallback(streamType);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_012
* @tc.desc  : Test ChangeDeviceSafeStatus interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_012, TestSize.Level1)
{
    SafeStatus safeStatus = SAFE_UNKNOWN;
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.ChangeDeviceSafeStatus(safeStatus);
    EXPECT_EQ(audioVolumeManager.safeStatus_, SAFE_UNKNOWN);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_013
* @tc.desc  : Test SetAbsVolumeSceneAsync interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_013, TestSize.Level1)
{
    std::string macAddress = "11:22:33:44:55:66";
    bool support = true;
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.audioActiveDevice_.SetActiveBtDeviceMac(macAddress);
    audioVolumeManager.SetAbsVolumeSceneAsync(macAddress, support);
    EXPECT_EQ(audioVolumeManager.audioActiveDevice_.GetActiveBtDeviceMac(), macAddress);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_014
* @tc.desc  : Test DealWithEventVolume interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_014, TestSize.Level1)
{
    int32_t notificationId;
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    int32_t ret;

    notificationId = RESTORE_VOLUME_NOTIFICATION_ID;
    audioVolumeManager.audioActiveDevice_.currentActiveDevice_.deviceCategory_ = CATEGORY_DEFAULT;
    audioVolumeManager.audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    ret = audioVolumeManager.DealWithEventVolume(notificationId);
    EXPECT_GE(ret, 0);

    notificationId = INCREASE_VOLUME_NOTIFICATION_ID;
    ret = audioVolumeManager.DealWithEventVolume(notificationId);
    EXPECT_NE(ret, 0);

    notificationId = NOTIFICATION_BANNER_FLAG;
    ret = audioVolumeManager.DealWithEventVolume(notificationId);
    EXPECT_GE(ret, 0);

    audioVolumeManager.audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_WIRED_HEADSET;
    notificationId = RESTORE_VOLUME_NOTIFICATION_ID;
    ret = audioVolumeManager.DealWithEventVolume(notificationId);
    EXPECT_GE(ret, 0);

    notificationId = INCREASE_VOLUME_NOTIFICATION_ID;
    ret = audioVolumeManager.DealWithEventVolume(notificationId);
    EXPECT_GE(ret, 0);

    notificationId = NOTIFICATION_BANNER_FLAG;
    ret = audioVolumeManager.DealWithEventVolume(notificationId);
    EXPECT_GE(ret, 0);

    audioVolumeManager.audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_NONE;
    ret = audioVolumeManager.DealWithEventVolume(notificationId);
    EXPECT_NE(ret, 0);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_015
* @tc.desc  : Test UpdateSafeVolumeByS4 interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_015, TestSize.Level1)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    AudioDeviceDescriptor audioDeviceDescriptor;

    audioVolumeManager.UpdateSafeVolumeByS4();
    audioVolumeManager.audioPolicyManager_.SetVolumeForSwitchDevice(audioDeviceDescriptor);
    EXPECT_EQ(audioVolumeManager.isBtFirstBoot_, true);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: SetVolumeForSwitchDevice_001
* @tc.desc  : Test AudioVolumeManager::SetVolumeForSwitchDevice interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, SetVolumeForSwitchDevice_001, TestSize.Level1)
{
    AudioVolumeManager &audioVolumeManager(AudioVolumeManager::GetInstance());
    AudioSceneManager &audioSceneManager(AudioSceneManager::GetInstance());
    AudioDeviceDescriptor audioDeviceDescriptor;

    audioSceneManager.audioScene_ = AUDIO_SCENE_PHONE_CALL;
    int32_t ret = audioVolumeManager.SetVolumeForSwitchDevice(
        audioDeviceDescriptor, PORT_NONE, true);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: SetVolumeForSwitchDevice_002
* @tc.desc  : Test AudioVolumeManager::SetVolumeForSwitchDevice interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, SetVolumeForSwitchDevice_002, TestSize.Level1)
{
    AudioVolumeManager &audioVolumeManager(AudioVolumeManager::GetInstance());
    AudioSceneManager &audioSceneManager(AudioSceneManager::GetInstance());
    AudioDeviceDescriptor audioDeviceDescriptor;

    audioSceneManager.audioScene_ = AUDIO_SCENE_PHONE_CALL;
    int32_t ret = audioVolumeManager.SetVolumeForSwitchDevice(
        audioDeviceDescriptor, PORT_NONE, false);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: SetVolumeForSwitchDevice_003
* @tc.desc  : Test AudioVolumeManager::SetVolumeForSwitchDevice interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, SetVolumeForSwitchDevice_003, TestSize.Level1)
{
    AudioVolumeManager &audioVolumeManager(AudioVolumeManager::GetInstance());
    AudioSceneManager &audioSceneManager(AudioSceneManager::GetInstance());
    AudioDeviceDescriptor audioDeviceDescriptor;

    audioSceneManager.audioScene_ = AUDIO_SCENE_DEFAULT;
    int32_t ret = audioVolumeManager.SetVolumeForSwitchDevice(
        audioDeviceDescriptor, PORT_NONE, true);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: SetVolumeForSwitchDevice_004
* @tc.desc  : Test AudioVolumeManager::SetVolumeForSwitchDevice interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, SetVolumeForSwitchDevice_004, TestSize.Level1)
{
    AudioVolumeManager &audioVolumeManager(AudioVolumeManager::GetInstance());
    AudioSceneManager &audioSceneManager(AudioSceneManager::GetInstance());
    AudioDeviceDescriptor audioDeviceDescriptor;

    audioSceneManager.audioScene_ = AUDIO_SCENE_DEFAULT;
    int32_t ret = audioVolumeManager.SetVolumeForSwitchDevice(
        audioDeviceDescriptor, PORT_NONE, false);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_016
* @tc.desc  : Test SetAbsVolumeSceneAsync interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_016, TestSize.Level1)
{
    std::string macAddress = "11:22:33:44:55:66";
    bool support = true;
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.audioActiveDevice_.SetActiveBtDeviceMac(macAddress);
    auto ret = audioVolumeManager.SetDeviceAbsVolumeSupported(macAddress, support);
    EXPECT_NE(ret, 0);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_017
* @tc.desc  : Test CheckMixActiveMusicTime interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_017, TestSize.Level1)
{
    int32_t safeVolume = 0;
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.activeSafeTimeBt_ = 0;
    audioVolumeManager.activeSafeTime_ = 0;
    audioVolumeManager.CheckBlueToothActiveMusicTime(safeVolume);

    audioVolumeManager.activeSafeTimeBt_ = 100000;
    audioVolumeManager.activeSafeTime_ = 100000;
    audioVolumeManager.CheckBlueToothActiveMusicTime(safeVolume);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_018
* @tc.desc  : Test CheckMixActiveMusicTime interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_018, TestSize.Level1)
{
    int32_t safeVolume = 0;
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.activeSafeTimeBt_ = 0;
    audioVolumeManager.activeSafeTime_ = 0;
    audioVolumeManager.CheckWiredActiveMusicTime(safeVolume);

    audioVolumeManager.activeSafeTimeBt_ = 100000;
    audioVolumeManager.activeSafeTime_ = 100000;
    audioVolumeManager.CheckWiredActiveMusicTime(safeVolume);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_019
* @tc.desc  : Test CheckMixActiveMusicTime interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_019, TestSize.Level1)
{
    AudioVolumeType streamType = AudioStreamType::STREAM_DEFAULT;
    bool mute = true;
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    bool bRet;

    bRet = audioVolumeManager.SetStreamMute(streamType, mute);
    EXPECT_FALSE(bRet);

    StreamUsage streamUsage = STREAM_USAGE_MEDIA;
    DeviceType deviceType = DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP;
    bRet = audioVolumeManager.SetStreamMute(streamType, mute, streamUsage, deviceType);
    EXPECT_FALSE(bRet);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_020
* @tc.desc  : Test GetMaxVolumeLevel interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_020, TestSize.Level1)
{
    AudioVolumeType streamType = AudioStreamType::STREAM_ALL;
    int32_t bRet;
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    bRet = audioVolumeManager.GetMaxVolumeLevel(streamType);
    EXPECT_EQ(bRet, 15);

    streamType = AudioStreamType::STREAM_MUSIC;
    bRet = audioVolumeManager.GetMaxVolumeLevel(streamType);
    EXPECT_EQ(bRet, 15);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_021
* @tc.desc  : Test GetMinVolumeLevel interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_021, TestSize.Level1)
{
    AudioVolumeType streamType = AudioStreamType::STREAM_ALL;
    int32_t bRet;
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    bRet = audioVolumeManager.GetMinVolumeLevel(streamType);
    EXPECT_EQ(bRet, 0);

    streamType = AudioStreamType::STREAM_MUSIC;
    bRet = audioVolumeManager.GetMinVolumeLevel(streamType);
    EXPECT_EQ(bRet, 0);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_022
* @tc.desc  : Test GetAllDeviceVolumeInfo interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_022, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    ASSERT_TRUE(audioVolumeManager != nullptr);

    std::shared_ptr<AudioDeviceDescriptor> remoteDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_EARPIECE, DeviceRole::OUTPUT_DEVICE);
    audioVolumeManager->audioConnectedDevice_.AddConnectedDevice(remoteDeviceDescriptor);
    audioVolumeManager->GetAllDeviceVolumeInfo();
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_023
* @tc.desc  : Test ForceVolumeKeyControlType interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_023, TestSize.Level1)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    audioVolumeManager.ForceVolumeKeyControlType(STREAM_RING, 3);
    EXPECT_EQ(audioVolumeManager.forceControlVolumeType_, STREAM_RING);
    EXPECT_EQ(audioVolumeManager.needForceControlVolumeType_, true);

    audioVolumeManager.ForceVolumeKeyControlType(STREAM_MEDIA, 3);
    EXPECT_EQ(audioVolumeManager.forceControlVolumeType_, STREAM_MEDIA);
    EXPECT_EQ(audioVolumeManager.needForceControlVolumeType_, true);

    audioVolumeManager.ForceVolumeKeyControlType(STREAM_MEDIA, -1);
    EXPECT_EQ(audioVolumeManager.forceControlVolumeType_, STREAM_DEFAULT);
    EXPECT_EQ(audioVolumeManager.needForceControlVolumeType_, false);

    audioVolumeManager.ForceVolumeKeyControlType(STREAM_MEDIA, 1);
    EXPECT_EQ(audioVolumeManager.forceControlVolumeType_, STREAM_MEDIA);
    EXPECT_EQ(audioVolumeManager.needForceControlVolumeType_, true);
    usleep(1500000);
    EXPECT_EQ(audioVolumeManager.forceControlVolumeType_, STREAM_DEFAULT);
    EXPECT_EQ(audioVolumeManager.needForceControlVolumeType_, false);
}
} // namespace AudioStandard
} // namespace OHOS

