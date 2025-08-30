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
#include "audio_volume_manager_unit_test.h"
#include "i_policy_provider.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
const int32_t RESTORE_VOLUME_NOTIFICATION_ID = 116000;
const int32_t INCREASE_VOLUME_NOTIFICATION_ID = 116001;
const uint32_t NOTIFICATION_BANNER_FLAG = 1 << 9;
const std::string AUDIO_RESTORE_VOLUME_EVENT = "AUDIO_RESTORE_VOLUME_EVENT";
const std::string AUDIO_INCREASE_VOLUME_EVENT = "AUDIO_INCREASE_VOLUME_EVENT";

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
* @tc.number: GetAllDeviceVolumeInfo_001
* @tc.desc  : Test GetAllDeviceVolumeInfo interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, GetAllDeviceVolumeInfo_001, TestSize.Level1)
{
    AudioVolumeManager audioVolumeManager;
    std::vector<std::shared_ptr<AllDeviceVolumeInfo>> allDeviceVolumeInfo = audioVolumeManager.GetAllDeviceVolumeInfo();
    EXPECT_EQ(allDeviceVolumeInfo.size(), 0);
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
* @tc.number: GetSystemVolumeLevel_001
* @tc.desc  : Test GetSystemVolumeLevel interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, GetSystemVolumeLevel_001, TestSize.Level1)
{
    AudioStreamType streamType = STREAM_NOTIFICATION;
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
* @tc.desc  : Test SetStreamMute interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_019, TestSize.Level1)
{
    AudioVolumeType streamType = AudioStreamType::STREAM_DEFAULT;
    bool mute = true;
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    bool bRet;

    bRet = audioVolumeManager.SetStreamMute(streamType, mute);
    EXPECT_NE(bRet, SUCCESS);

    StreamUsage streamUsage = STREAM_USAGE_MEDIA;
    DeviceType deviceType = DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP;
    bRet = audioVolumeManager.SetStreamMute(streamType, mute, streamUsage, deviceType);
    EXPECT_NE(bRet, SUCCESS);
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

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_024
* @tc.desc  : Test SetSharedVolume interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_024, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    ASSERT_TRUE(audioVolumeManager != nullptr);

    AudioVolumeType streamType = STREAM_RING;
    DeviceType deviceType = DEVICE_TYPE_NONE;
    Volume vol;
    audioVolumeManager->volumeVector_ = new Volume();
    auto ret = audioVolumeManager->SetSharedVolume(streamType, deviceType, vol);
    EXPECT_EQ(ret, false);
    delete audioVolumeManager->volumeVector_;
    audioVolumeManager->volumeVector_ = nullptr;
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_025
* @tc.desc  : Test SetSharedVolume interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_025, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    ASSERT_TRUE(audioVolumeManager != nullptr);

    AudioVolumeType streamType = STREAM_VOICE_CALL;
    DeviceType deviceType = DEVICE_TYPE_NEARLINK;
    Volume vol;
    audioVolumeManager->volumeVector_ = new Volume();
    auto ret = audioVolumeManager->SetSharedVolume(streamType, deviceType, vol);
    EXPECT_EQ(ret, true);
    delete audioVolumeManager->volumeVector_;
    audioVolumeManager->volumeVector_ = nullptr;
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_026
* @tc.desc  : Test SetSharedVolume interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_026, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    ASSERT_TRUE(audioVolumeManager != nullptr);
    size_t vecSize = IPolicyProvider::GetVolumeVectorSize();
    ASSERT_GT(vecSize, 0u);
    audioVolumeManager->volumeVector_ = new Volume[vecSize]();

    AudioVolumeType streamType = STREAM_VOICE_CALL;
    DeviceType deviceType = DEVICE_TYPE_SPEAKER;
    Volume vol{};
    vol.isMute = false;
    vol.volumeFloat = 0.5f;
    vol.volumeInt = 8;
    vol.volumeDegree = 0;
    auto ret = audioVolumeManager->SetSharedVolume(streamType, deviceType, vol);
    EXPECT_EQ(ret, true);
    delete[] audioVolumeManager->volumeVector_;
    audioVolumeManager->volumeVector_ = nullptr;
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_027
* @tc.desc  : Test SetSharedVolume interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_027, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    ASSERT_TRUE(audioVolumeManager != nullptr);

    AudioVolumeType streamType = STREAM_RING;
    DeviceType deviceType = DEVICE_TYPE_NEARLINK;
    Volume vol;
    audioVolumeManager->volumeVector_ = new Volume();
    auto ret = audioVolumeManager->SetSharedVolume(streamType, deviceType, vol);
    EXPECT_EQ(ret, true);
    delete audioVolumeManager->volumeVector_;
    audioVolumeManager->volumeVector_ = nullptr;
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_028
* @tc.desc  : Test SetSharedVolume interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_028, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    ASSERT_TRUE(audioVolumeManager != nullptr);

    AudioVolumeType streamType = STREAM_RING;
    DeviceType deviceType = DEVICE_TYPE_SPEAKER;
    Volume vol;
    audioVolumeManager->volumeVector_ = new Volume();
    auto ret = audioVolumeManager->SetSharedVolume(streamType, deviceType, vol);
    EXPECT_EQ(ret, true);
    delete audioVolumeManager->volumeVector_;
    audioVolumeManager->volumeVector_ = nullptr;
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_029
* @tc.desc  : Test GetSystemVolumeLevel interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_029, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    ASSERT_TRUE(audioVolumeManager != nullptr);

    int32_t zoneId = 10;
    AudioStreamType streamType = STREAM_MUSIC;
    audioVolumeManager->GetSystemVolumeLevel(streamType, zoneId);

    zoneId = 0;
    audioVolumeManager->ringerModeMute_ = true;
    auto ret = audioVolumeManager->GetSystemVolumeLevel(streamType, zoneId);
    EXPECT_EQ(ret, 0);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_030
* @tc.desc  : Test GetSystemVolumeLevel interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_030, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    ASSERT_TRUE(audioVolumeManager != nullptr);

    int32_t zoneId = 0;
    AudioStreamType streamType = STREAM_MUSIC;
    audioVolumeManager->audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_NEARLINK;
    audioVolumeManager->GetSystemVolumeLevel(streamType, zoneId);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_031
* @tc.desc  : Test GetSystemVolumeLevel interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_031, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    ASSERT_TRUE(audioVolumeManager != nullptr);

    int32_t zoneId = 0;
    AudioStreamType streamType = STREAM_MUSIC;
    audioVolumeManager->audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    audioVolumeManager->GetSystemVolumeLevel(streamType, zoneId);

    streamType = STREAM_ALL;
    audioVolumeManager->GetSystemVolumeLevel(streamType, zoneId);

    audioVolumeManager->audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_NEARLINK;
    audioVolumeManager->GetSystemVolumeLevel(streamType, zoneId);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_032
* @tc.desc  : Test SetVoiceCallVolume interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_032, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    ASSERT_TRUE(audioVolumeManager != nullptr);

    int32_t volumeLevel = -1;
    audioVolumeManager->audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_NEARLINK;
    audioVolumeManager->SetVoiceCallVolume(volumeLevel);

    audioVolumeManager->audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_LINE_DIGITAL;
    audioVolumeManager->SetVoiceCallVolume(volumeLevel);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_033
* @tc.desc  : Test DeviceIsSupportSafeVolume interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_033, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    ASSERT_TRUE(audioVolumeManager != nullptr);

    AudioAdapterManager audioAdapterManager;
    audioAdapterManager.currentActiveDevice_.deviceCategory_ = BT_SOUNDBOX;
    audioVolumeManager->audioPolicyManager_ = audioAdapterManager;
    audioVolumeManager->audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    auto ret = audioVolumeManager->DeviceIsSupportSafeVolume();
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_034
* @tc.desc  : Test HandleNearlinkDeviceAbsVolume interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_034, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    ASSERT_TRUE(audioVolumeManager != nullptr);

    int32_t volumeLevel = 1;
    AudioStreamType streamType = STREAM_MUSIC;
    DeviceType curOutputDeviceType = DEVICE_TYPE_SPEAKER;
    audioVolumeManager->audioActiveDevice_.currentActiveDevice_.macAddress_ = "";
    auto ret = audioVolumeManager->HandleNearlinkDeviceAbsVolume(streamType, volumeLevel, curOutputDeviceType);
    EXPECT_EQ(ret, ERR_UNKNOWN);

    audioVolumeManager->audioActiveDevice_.currentActiveDevice_.macAddress_ = "test";
    ret = audioVolumeManager->HandleNearlinkDeviceAbsVolume(streamType, volumeLevel, curOutputDeviceType);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_035
* @tc.desc  : Test SetSystemVolumeLevel interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_035, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    ASSERT_TRUE(audioVolumeManager != nullptr);

    int32_t zoneId = 10;
    int32_t volumeLevel = 1;
    AudioStreamType streamType = STREAM_MUSIC;
    auto ret = audioVolumeManager->SetSystemVolumeLevel(streamType, volumeLevel, zoneId);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);

    zoneId = 0;
    audioVolumeManager->audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_NEARLINK;
    audioVolumeManager->SetSystemVolumeLevel(streamType, volumeLevel, zoneId);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_036
* @tc.desc  : Test SetSystemVolumeLevel interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_036, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    ASSERT_TRUE(audioVolumeManager != nullptr);

    int32_t zoneId = 0;
    int32_t volumeLevel = 1;
    AudioStreamType streamType = STREAM_MUSIC;
    audioVolumeManager->audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    audioVolumeManager->SetSystemVolumeLevel(streamType, volumeLevel, zoneId);

    streamType = STREAM_VOICE_CALL_ASSISTANT;
    audioVolumeManager->SetSystemVolumeLevel(streamType, volumeLevel, zoneId);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_037
* @tc.desc  : Test SetSystemVolumeLevel interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_037, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    ASSERT_TRUE(audioVolumeManager != nullptr);

    int32_t zoneId = 0;
    int32_t volumeLevel = 1;
    AudioStreamType streamType = STREAM_VOICE_CALL_ASSISTANT;
    audioVolumeManager->audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_NEARLINK;
    audioVolumeManager->SetSystemVolumeLevel(streamType, volumeLevel, zoneId);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_038
* @tc.desc  : Test SaveSpecifiedDeviceVolume interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_038, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    ASSERT_TRUE(audioVolumeManager != nullptr);

    int32_t volumeLevel = 1000;
    DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    AudioStreamType streamType = STREAM_VOICE_CALL_ASSISTANT;
    auto ret = audioVolumeManager->SaveSpecifiedDeviceVolume(streamType, volumeLevel, deviceType);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);

    deviceType = DEVICE_TYPE_BLUETOOTH_SCO;
    audioVolumeManager->SaveSpecifiedDeviceVolume(streamType, volumeLevel, deviceType);

    deviceType = DEVICE_TYPE_USB_HEADSET;
    audioVolumeManager->SaveSpecifiedDeviceVolume(streamType, volumeLevel, deviceType);

    deviceType = DEVICE_TYPE_USB_ARM_HEADSET;
    audioVolumeManager->SaveSpecifiedDeviceVolume(streamType, volumeLevel, deviceType);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_039
* @tc.desc  : Test SaveSpecifiedDeviceVolume interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_039, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    ASSERT_TRUE(audioVolumeManager != nullptr);

    int32_t volumeLevel = 1000;
    DeviceType deviceType = DEVICE_TYPE_WIRED_HEADSET;
    AudioStreamType streamType = STREAM_VOICE_CALL_ASSISTANT;
    auto ret = audioVolumeManager->SaveSpecifiedDeviceVolume(streamType, volumeLevel, deviceType);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);

    deviceType = DEVICE_TYPE_WIRED_HEADPHONES;
    audioVolumeManager->SaveSpecifiedDeviceVolume(streamType, volumeLevel, deviceType);

    deviceType = DEVICE_TYPE_NEARLINK;
    audioVolumeManager->SaveSpecifiedDeviceVolume(streamType, volumeLevel, deviceType);

    deviceType = DEVICE_TYPE_NONE;
    ret = audioVolumeManager->SaveSpecifiedDeviceVolume(streamType, volumeLevel, deviceType);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_040
* @tc.desc  : Test SetA2dpDeviceVolume interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_040, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    ASSERT_TRUE(audioVolumeManager != nullptr);

    std::string macAddress = "test";
    int32_t volumeLevel = 1000;
    bool internalCall = true;
    A2dpDeviceConfigInfo a2dpDeviceConfigInfo;
    a2dpDeviceConfigInfo.absVolumeSupport = true;
    audioVolumeManager->audioA2dpDevice_.connectedA2dpDeviceMap_[macAddress] = a2dpDeviceConfigInfo;
    audioVolumeManager->SetA2dpDeviceVolume(macAddress, volumeLevel, internalCall);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_041
* @tc.desc  : Test SetA2dpDeviceVolume interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_041, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    ASSERT_TRUE(audioVolumeManager != nullptr);

    std::string macAddress = "test";
    int32_t volumeLevel = 1000;
    bool internalCall = false;
    A2dpDeviceConfigInfo a2dpDeviceConfigInfo;
    a2dpDeviceConfigInfo.absVolumeSupport = true;
    audioVolumeManager->audioA2dpDevice_.connectedA2dpDeviceMap_[macAddress] = a2dpDeviceConfigInfo;
    audioVolumeManager->SetA2dpDeviceVolume(macAddress, volumeLevel, internalCall);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_042
* @tc.desc  : Test HandleAbsBluetoothVolume interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_042, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    ASSERT_TRUE(audioVolumeManager != nullptr);

    std::string macAddress = "test";
    int32_t volumeLevel = 0;
    bool isNearlinkDevice = true;
    AudioStreamType streamType = STREAM_RING;
    audioVolumeManager->isBtFirstBoot_ = false;
    audioVolumeManager->HandleAbsBluetoothVolume(macAddress, volumeLevel, isNearlinkDevice, streamType);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_043
* @tc.desc  : Test HandleAbsBluetoothVolume interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_043, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    ASSERT_TRUE(audioVolumeManager != nullptr);

    std::string macAddress = "test";
    int32_t volumeLevel = 0;
    bool isNearlinkDevice = true;
    AudioStreamType streamType = STREAM_RING;
    audioVolumeManager->isBtFirstBoot_ = true;
    audioVolumeManager->HandleAbsBluetoothVolume(macAddress, volumeLevel, isNearlinkDevice, streamType);

    isNearlinkDevice = false;
    audioVolumeManager->HandleAbsBluetoothVolume(macAddress, volumeLevel, isNearlinkDevice, streamType);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_044
* @tc.desc  : Test SetNearlinkDeviceVolume interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_044, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    ASSERT_TRUE(audioVolumeManager != nullptr);

    std::string macAddress = "test";
    AudioStreamType streamType = STREAM_MUSIC;
    int32_t volumeLevel = 1000;
    bool internalCall = true;
    SleVolumeConfigInfo configInfo;
    std::pair<SleVolumeConfigInfo, SleVolumeConfigInfo> pairConfigInfo = std::make_pair(configInfo, configInfo);
    SleAudioDeviceManager::GetInstance().deviceVolumeConfigInfo_["test"] = pairConfigInfo;
    auto ret = audioVolumeManager->SetNearlinkDeviceVolume(macAddress, streamType, volumeLevel, internalCall);
    EXPECT_EQ(ret, ERR_UNKNOWN);

    internalCall = false;
    ret = audioVolumeManager->SetNearlinkDeviceVolume(macAddress, streamType, volumeLevel, internalCall);
    EXPECT_EQ(ret, ERR_UNKNOWN);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_045
* @tc.desc  : Test SetNearlinkDeviceVolume interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_045, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    ASSERT_TRUE(audioVolumeManager != nullptr);

    std::string macAddress = "test";
    AudioStreamType streamType = STREAM_MUSIC;
    int32_t volumeLevel = -10;
    bool internalCall = true;
    SleVolumeConfigInfo configInfo;
    std::pair<SleVolumeConfigInfo, SleVolumeConfigInfo> pairConfigInfo = std::make_pair(configInfo, configInfo);
    SleAudioDeviceManager::GetInstance().deviceVolumeConfigInfo_["test"] = pairConfigInfo;
    audioVolumeManager->SetNearlinkDeviceVolume(macAddress, streamType, volumeLevel, internalCall);

    streamType = STREAM_VOICE_CALL_ASSISTANT;
    audioVolumeManager->SetNearlinkDeviceVolume(macAddress, streamType, volumeLevel, internalCall);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_046
* @tc.desc  : Test SelectDealSafeVolume interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_046, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    ASSERT_TRUE(audioVolumeManager != nullptr);

    int32_t volumeLevel = 1000;
    DeviceType deviceType = DEVICE_TYPE_SPEAKER;
    AudioStreamType streamType = STREAM_MUSIC;
    audioVolumeManager->audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_NEARLINK;

    AudioAdapterManager audioAdapterManager;
    audioAdapterManager.currentActiveDevice_.deviceCategory_ = BT_SOUNDBOX;
    audioVolumeManager->audioPolicyManager_ = audioAdapterManager;
    auto ret = audioVolumeManager->SelectDealSafeVolume(streamType, volumeLevel, deviceType);
    EXPECT_EQ(ret, volumeLevel);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_047
* @tc.desc  : Test SelectDealSafeVolume interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_047, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    ASSERT_TRUE(audioVolumeManager != nullptr);

    int32_t volumeLevel = 1000;
    DeviceType deviceType = DEVICE_TYPE_SPEAKER;
    AudioStreamType streamType = STREAM_MUSIC;
    audioVolumeManager->audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_NEARLINK;

    AudioAdapterManager audioAdapterManager;
    audioAdapterManager.currentActiveDevice_.deviceCategory_ = BT_CAR;
    audioVolumeManager->audioPolicyManager_ = audioAdapterManager;
    auto ret = audioVolumeManager->SelectDealSafeVolume(streamType, volumeLevel, deviceType);
    EXPECT_EQ(ret, volumeLevel);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_048
* @tc.desc  : Test SelectDealSafeVolume interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_048, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    ASSERT_TRUE(audioVolumeManager != nullptr);

    int32_t volumeLevel = 1000;
    DeviceType deviceType = DEVICE_TYPE_SPEAKER;
    AudioStreamType streamType = STREAM_MUSIC;
    audioVolumeManager->audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_NEARLINK;

    AudioAdapterManager audioAdapterManager;
    audioAdapterManager.currentActiveDevice_.deviceCategory_ = BT_WATCH;
    audioVolumeManager->audioPolicyManager_ = audioAdapterManager;
    audioVolumeManager->isBtFirstBoot_ = true;
    audioVolumeManager->SelectDealSafeVolume(streamType, volumeLevel, deviceType);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_049
* @tc.desc  : Test CreateCheckMusicActiveThread interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_049, TestSize.Level1)
{
    AudioVolumeManager::GetInstance().calculateLoopSafeTime_ = nullptr;
    AudioVolumeManager::GetInstance().CreateCheckMusicActiveThread();
    EXPECT_TRUE(AudioVolumeManager::GetInstance().calculateLoopSafeTime_ != nullptr);
    AudioVolumeManager::GetInstance().CreateCheckMusicActiveThread();
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_050
* @tc.desc  : Test SetRestoreVolumeLevel interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_050, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    ASSERT_TRUE(audioVolumeManager != nullptr);

    int32_t curDeviceVolume = 1000;
    DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioVolumeManager->SetRestoreVolumeLevel(deviceType, curDeviceVolume);
    EXPECT_EQ(audioVolumeManager->btRestoreVol_, curDeviceVolume);

    deviceType = DEVICE_TYPE_WIRED_HEADSET;
    audioVolumeManager->SetRestoreVolumeLevel(deviceType, curDeviceVolume);
    EXPECT_EQ(audioVolumeManager->wiredRestoreVol_, curDeviceVolume);

    deviceType = DEVICE_TYPE_SPEAKER;
    audioVolumeManager->SetRestoreVolumeLevel(deviceType, curDeviceVolume);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_051
* @tc.desc  : Test CheckLowerDeviceVolume interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_051, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    ASSERT_TRUE(audioVolumeManager != nullptr);

    audioVolumeManager->audioPolicyManager_.Init();
    DeviceType deviceType = DEVICE_TYPE_WIRED_HEADSET;
    audioVolumeManager->CheckLowerDeviceVolume(deviceType);

    deviceType = DEVICE_TYPE_BLUETOOTH_SCO;
    audioVolumeManager->CheckLowerDeviceVolume(deviceType);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_052
* @tc.desc  : Test SetSafeVolumeCallback interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_052, TestSize.Level1)
{
    AudioStreamType streamType = STREAM_MUSIC;
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    ASSERT_TRUE(audioVolumeManager != nullptr);

    audioVolumeManager->ringerModeMute_ = false;
    audioVolumeManager->audioPolicyServerHandler_ = nullptr;
    audioVolumeManager->SetSafeVolumeCallback(streamType);

    audioVolumeManager->audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    audioVolumeManager->SetSafeVolumeCallback(streamType);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_053
* @tc.desc  : Test OnReceiveEvent interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_053, TestSize.Level1)
{
    AAFwk::Want want;
    EventFwk::CommonEventData eventData;
    std::string action = AUDIO_RESTORE_VOLUME_EVENT;
    want.SetAction(action);
    eventData.SetWant(want);
    AudioVolumeManager::GetInstance().OnReceiveEvent(eventData);
    EXPECT_EQ(AudioVolumeManager::GetInstance().safeStatusBt_, SAFE_INACTIVE);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_054
* @tc.desc  : Test OnReceiveEvent interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_054, TestSize.Level1)
{
    AAFwk::Want want;
    EventFwk::CommonEventData eventData;
    std::string action = AUDIO_INCREASE_VOLUME_EVENT;
    want.SetAction(action);
    eventData.SetWant(want);
    AudioVolumeManager::GetInstance().OnReceiveEvent(eventData);
    EXPECT_EQ(AudioVolumeManager::GetInstance().safeStatusBt_, SAFE_INACTIVE);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_055
* @tc.desc  : Test OnReceiveEvent interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_055, TestSize.Level1)
{
    AAFwk::Want want;
    EventFwk::CommonEventData eventData;
    std::string action = "test";
    want.SetAction(action);
    eventData.SetWant(want);
    AudioVolumeManager::GetInstance().OnReceiveEvent(eventData);
    EXPECT_EQ(AudioVolumeManager::GetInstance().safeStatusBt_, SAFE_INACTIVE);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_056
* @tc.desc  : Test SetDeviceSafeVolumeStatus interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_056, TestSize.Level1)
{
    AudioVolumeManager::GetInstance().userSelect_ = true;
    AudioVolumeManager::GetInstance().audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    AudioVolumeManager::GetInstance().SetDeviceSafeVolumeStatus();
    EXPECT_EQ(AudioVolumeManager::GetInstance().safeStatusBt_, SAFE_INACTIVE);

    AudioVolumeManager::GetInstance().audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_WIRED_HEADSET;
    AudioVolumeManager::GetInstance().SetDeviceSafeVolumeStatus();
    EXPECT_EQ(AudioVolumeManager::GetInstance().safeStatus_, SAFE_INACTIVE);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_057
* @tc.desc  : Test SetAbsVolumeSceneAsync interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_057, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    ASSERT_TRUE(audioVolumeManager != nullptr);

    bool support = true;
    std::string macAddress = "11:22:33:44:55:66";
    audioVolumeManager->audioActiveDevice_.activeBTDevice_ = "test";
    audioVolumeManager->SetAbsVolumeSceneAsync(macAddress, support);

    audioVolumeManager->audioActiveDevice_.activeBTDevice_ = macAddress;
    audioVolumeManager->SetAbsVolumeSceneAsync(macAddress, support);
    EXPECT_EQ(audioVolumeManager->audioActiveDevice_.GetActiveBtDeviceMac(), macAddress);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_058
* @tc.desc  : Test SetAbsVolumeSceneAsync interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_058, TestSize.Level1)
{
    std::string macAddress = "test";
    bool support = true;
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.audioA2dpDevice_.connectedA2dpDeviceMap_.clear();
    auto ret = audioVolumeManager.SetDeviceAbsVolumeSupported(macAddress, support);
    EXPECT_NE(ret, 0);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_059
* @tc.desc  : Test SetDeviceAbsVolumeSupported interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_059, TestSize.Level1)
{
    std::string macAddress = "test";
    bool support = true;
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    A2dpDeviceConfigInfo a2dpDeviceConfigInfo;
    a2dpDeviceConfigInfo.absVolumeSupport = true;
    audioVolumeManager.audioA2dpDevice_.connectedA2dpDeviceMap_[macAddress] = a2dpDeviceConfigInfo;
    auto ret = audioVolumeManager.SetDeviceAbsVolumeSupported(macAddress, support);
    EXPECT_EQ(ret, 0);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_060
* @tc.desc  : Test SetStreamMute interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_060, TestSize.Level1)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    AudioStreamType streamType = STREAM_MUSIC;
    bool mute = true;
    StreamUsage streamUsage = STREAM_USAGE_MUSIC;
    DeviceType deviceType = DEVICE_TYPE_SPEAKER;
    int32_t zoneId = 10;
    auto ret = audioVolumeManager.SetStreamMute(streamType, mute, streamUsage, deviceType, zoneId);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_061
* @tc.desc  : Test GetStreamMute interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_061, TestSize.Level1)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    AudioStreamType streamType = STREAM_MUSIC;
    int32_t zoneId = 10;
    auto ret = audioVolumeManager.GetStreamMute(streamType, zoneId);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_062
* @tc.desc  : Test ResetRingerModeMute interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_062, TestSize.Level1)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    auto ret = audioVolumeManager.ResetRingerModeMute();
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManagerDegree_001
* @tc.desc  : Test SetSystemVolumeDegree interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManagerDegree_001, TestSize.Level1)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    int32_t zoneId = 0;
    int32_t ret = audioVolumeManager.SetAdjustVolumeForZone(zoneId);
    EXPECT_EQ(ret, SUCCESS);
    zoneId = 1;
    ret = audioVolumeManager.SetAdjustVolumeForZone(zoneId);
    EXPECT_NE(ret, SUCCESS);

    AudioStreamType streamType = STREAM_MUSIC;
    int32_t setDegree = audioVolumeManager.GetMinVolumeDegree(streamType);
    ret = audioVolumeManager.SetSystemVolumeDegree(streamType, setDegree, 0);
    EXPECT_EQ(ret, SUCCESS);

    int32_t gotDegree = audioVolumeManager.GetSystemVolumeDegree(streamType);
    EXPECT_EQ(VolumeUtils::VolumeDegreeToLevel(gotDegree), VolumeUtils::VolumeDegreeToLevel(setDegree));

    ret = audioVolumeManager.GetMinVolumeDegree(streamType);
    EXPECT_EQ(ret, 0);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_063
* @tc.desc  : Test Init interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_063, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    ASSERT_TRUE(audioVolumeManager != nullptr);
    audioVolumeManager->DeInit();
    EXPECT_EQ(audioVolumeManager->forceControlVolumeTypeMonitor_, nullptr);

    bool ret;
    ret = audioVolumeManager->Init(nullptr);
    EXPECT_EQ(ret, true);
    EXPECT_NE(audioVolumeManager->forceControlVolumeTypeMonitor_, nullptr);
    ret = audioVolumeManager->Init(nullptr);
    EXPECT_EQ(ret, true);
    EXPECT_NE(audioVolumeManager->forceControlVolumeTypeMonitor_, nullptr);
    audioVolumeManager->DeInit();
    EXPECT_EQ(audioVolumeManager->forceControlVolumeTypeMonitor_, nullptr);
}
/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_064
* @tc.desc  : Test SetAdjustVolumeForZone
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_064, TestSize.Level1)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    int32_t zoneId = 0;
    auto ret = audioVolumeManager.SetAdjustVolumeForZone(zoneId);
    EXPECT_EQ(ret, SUCCESS);
    zoneId = 1;
    ret = audioVolumeManager.SetAdjustVolumeForZone(zoneId);
    EXPECT_NE(ret, SUCCESS);
}
} // namespace AudioStandard
} // namespace OHOS
