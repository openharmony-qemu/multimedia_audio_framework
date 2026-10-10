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
#include "audio_interrupt_service.h"
#include "sle_audio_device_manager.h"
#include "audio_volume_manager_unit_test.h"

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
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    std::shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler(
        DelayedSingleton<AudioPolicyServerHandler>::GetInstance());
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.Init(audioPolicyServerHandler, interruptService);
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

constexpr int32_t TEST_DEVICE_ID = 1001;
static AudioDeviceDescriptor& CurrentActiveDevice()
{
    auto descs = AudioRouterSelectStrategy::GetInstance().GetCurrentOutputDevice(SYSTEM_UID);
    auto desc = descs.empty() || !descs.back() ? make_shared<AudioDeviceDescriptor>() : descs.back();
    if (desc->deviceId_ == 0) {
        desc->deviceId_ = TEST_DEVICE_ID;
        AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc);
        AudioRouterSelectStrategy::GetInstance().UpdateCurrentOutputDevice(SYSTEM_UID, {desc});
    }
    desc = AudioDeviceManager::GetAudioDeviceManager().FindConnectedDeviceById(desc->deviceId_);
    return *desc;
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
    VolumeScale safeVolume = audioVolumeManager.HandleAbsBluetoothVolume(macAddress, volumeLevel);
    ret = safeVolume.VolumeLevel();
    EXPECT_EQ(ret, 0);

    audioVolumeManager.isBtFirstBoot_ = false;
    auto btCarDevice = std::make_shared<AudioDeviceDescriptor>();
    btCarDevice->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    btCarDevice->deviceCategory_ = BT_CAR;
    btCarDevice->networkId_ = LOCAL_NETWORK_ID;
    AudioConnectedDevice::GetInstance().AddConnectedDevice(btCarDevice);
    safeVolume = audioVolumeManager.HandleAbsBluetoothVolume(macAddress, volumeLevel);
    ret = safeVolume.VolumeLevel();
    EXPECT_EQ(ret, 0);
    AudioConnectedDevice::GetInstance().DelConnectedDevice(LOCAL_NETWORK_ID, DEVICE_TYPE_BLUETOOTH_A2DP);
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

    bRet = audioVolumeManager.safeVolumeManager_.IsWiredHeadSet(deviceType);
    EXPECT_EQ(bRet, false);

    deviceType = DEVICE_TYPE_WIRED_HEADSET;
    bRet = audioVolumeManager.safeVolumeManager_.IsWiredHeadSet(deviceType);
    EXPECT_EQ(bRet, true);

    deviceType = DEVICE_TYPE_WIRED_HEADPHONES;
    bRet = audioVolumeManager.safeVolumeManager_.IsWiredHeadSet(deviceType);
    EXPECT_EQ(bRet, true);

    deviceType = DEVICE_TYPE_USB_HEADSET;
    bRet = audioVolumeManager.safeVolumeManager_.IsWiredHeadSet(deviceType);
    EXPECT_EQ(bRet, true);

    deviceType = DEVICE_TYPE_USB_ARM_HEADSET;
    bRet = audioVolumeManager.safeVolumeManager_.IsWiredHeadSet(deviceType);
    EXPECT_EQ(bRet, true);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_006
* @tc.desc  : Test IsBlueTooth interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_006, TestSize.Level1)
{
    bool bRet;
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceType_ = DEVICE_TYPE_NONE;
    bRet = audioVolumeManager.safeVolumeManager_.IsBlueTooth(deviceDesc);
    EXPECT_EQ(bRet, false);

    deviceDesc->deviceCategory_ = BT_CAR;
    deviceDesc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    bRet = audioVolumeManager.safeVolumeManager_.IsBlueTooth(deviceDesc);
    EXPECT_EQ(bRet, false);

    deviceDesc->deviceCategory_ = BT_SOUNDBOX;
    deviceDesc->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    bRet = audioVolumeManager.safeVolumeManager_.IsBlueTooth(deviceDesc);
    EXPECT_EQ(bRet, false);

    deviceDesc->deviceCategory_ = CATEGORY_DEFAULT;
    deviceDesc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    bRet = audioVolumeManager.safeVolumeManager_.IsBlueTooth(deviceDesc);
    EXPECT_EQ(bRet, true);

    deviceDesc->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    bRet = audioVolumeManager.safeVolumeManager_.IsBlueTooth(deviceDesc);
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

    audioVolumeManager.audioActiveDevice_.AddActiveBtDevice(macAddress);
    audioVolumeManager.SetAbsVolumeSceneAsync(macAddress, support, 0);
    EXPECT_TRUE(audioVolumeManager.audioActiveDevice_.IsActiveBtDevice(macAddress));
}

/**
 * @tc.name  : DealWithEventVolume_BlueToothRestore
 * @tc.number: DealWithEventVolume_001
 * @tc.desc  : Test DealWithEventVolume with bluetooth device and RESTORE_VOLUME notification
 */
HWTEST_F(AudioVolumeManagerUnitTest, DealWithEventVolume_BlueToothRestore, TestSize.Level1)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    auto btDevice = std::make_shared<AudioDeviceDescriptor>();
    btDevice->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    btDevice->deviceCategory_ = CATEGORY_DEFAULT;
    AudioConnectedDevice::GetInstance().AddConnectedDevice(btDevice);
    audioVolumeManager.audioActiveDevice_.volumeTypeDeviceMap_[STREAM_MUSIC] = {btDevice};

    int32_t ret = audioVolumeManager.DealWithEventVolume(RESTORE_VOLUME_NOTIFICATION_ID);
    EXPECT_NE(ret, ERROR);

    AudioConnectedDevice::GetInstance().DelConnectedDevice(btDevice->networkId_, btDevice->deviceType_);
    audioVolumeManager.audioActiveDevice_.volumeTypeDeviceMap_.erase(STREAM_MUSIC);
}

/**
 * @tc.name  : DealWithEventVolume_WiredHeadSetRestore
 * @tc.number: DealWithEventVolume_002
 * @tc.desc  : Test DealWithEventVolume with wired headset device and RESTORE_VOLUME notification
 */
HWTEST_F(AudioVolumeManagerUnitTest, DealWithEventVolume_WiredHeadSetRestore, TestSize.Level1)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    auto wiredDevice = std::make_shared<AudioDeviceDescriptor>();
    wiredDevice->deviceType_ = DEVICE_TYPE_WIRED_HEADSET;
    AudioConnectedDevice::GetInstance().AddConnectedDevice(wiredDevice);
    audioVolumeManager.audioActiveDevice_.volumeTypeDeviceMap_[STREAM_MUSIC] = {wiredDevice};

    int32_t ret = audioVolumeManager.DealWithEventVolume(RESTORE_VOLUME_NOTIFICATION_ID);
    EXPECT_NE(ret, ERROR);

    AudioConnectedDevice::GetInstance().DelConnectedDevice(wiredDevice->networkId_, wiredDevice->deviceType_);
    audioVolumeManager.audioActiveDevice_.volumeTypeDeviceMap_.erase(STREAM_MUSIC);
}

/**
 * @tc.name  : DealWithEventVolume_NearLinkRestore
 * @tc.number: DealWithEventVolume_003
 * @tc.desc  : Test DealWithEventVolume with nearlink device and RESTORE_VOLUME notification
 */
HWTEST_F(AudioVolumeManagerUnitTest, DealWithEventVolume_NearLinkRestore, TestSize.Level1)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    auto nearLinkDevice = std::make_shared<AudioDeviceDescriptor>();
    nearLinkDevice->deviceType_ = DEVICE_TYPE_NEARLINK;
    nearLinkDevice->networkId_ = LOCAL_NETWORK_ID;
    AudioConnectedDevice::GetInstance().AddConnectedDevice(nearLinkDevice);
    audioVolumeManager.audioActiveDevice_.volumeTypeDeviceMap_[STREAM_MUSIC] = {nearLinkDevice};

    int32_t ret = audioVolumeManager.DealWithEventVolume(RESTORE_VOLUME_NOTIFICATION_ID);
    EXPECT_NE(ret, ERROR);

    AudioConnectedDevice::GetInstance().DelConnectedDevice(LOCAL_NETWORK_ID, DEVICE_TYPE_NEARLINK);
    audioVolumeManager.audioActiveDevice_.volumeTypeDeviceMap_.erase(STREAM_MUSIC);
}

/**
 * @tc.name  : DealWithEventVolume_UnsupportedDevice
 * @tc.number: DealWithEventVolume_004
 * @tc.desc  : Test DealWithEventVolume with unsupported device type returns ERROR
 */
HWTEST_F(AudioVolumeManagerUnitTest, DealWithEventVolume_UnsupportedDevice, TestSize.Level1)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    auto hdmiDevice = std::make_shared<AudioDeviceDescriptor>();
    hdmiDevice->deviceType_ = DEVICE_TYPE_HDMI;
    AudioConnectedDevice::GetInstance().AddConnectedDevice(hdmiDevice);
    audioVolumeManager.audioActiveDevice_.volumeTypeDeviceMap_[STREAM_MUSIC] = {hdmiDevice};

    int32_t ret = audioVolumeManager.DealWithEventVolume(INCREASE_VOLUME_NOTIFICATION_ID);
    EXPECT_EQ(ret, ERROR);

    AudioConnectedDevice::GetInstance().DelConnectedDevice(hdmiDevice->networkId_, hdmiDevice->deviceType_);
    audioVolumeManager.audioActiveDevice_.volumeTypeDeviceMap_.erase(STREAM_MUSIC);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: SetVoiceCallVolumeOnDeviceSwitch_001
* @tc.desc  : Test AudioVolumeManager::SetVoiceCallVolumeOnDeviceSwitch interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, SetVoiceCallVolumeOnDeviceSwitch_001, TestSize.Level1)
{
    AudioVolumeManager &audioVolumeManager(AudioVolumeManager::GetInstance());
    AudioSceneManager &audioSceneManager(AudioSceneManager::GetInstance());
    AudioDeviceDescriptor audioDeviceDescriptor;

    audioSceneManager.audioScene_ = AUDIO_SCENE_PHONE_CALL;
    int32_t ret = audioVolumeManager.SetVoiceCallVolumeOnDeviceSwitch(
        audioDeviceDescriptor, true);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: SetVoiceCallVolumeOnDeviceSwitch_002
* @tc.desc  : Test AudioVolumeManager::SetVoiceCallVolumeOnDeviceSwitch interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, SetVoiceCallVolumeOnDeviceSwitch_002, TestSize.Level1)
{
    AudioVolumeManager &audioVolumeManager(AudioVolumeManager::GetInstance());
    AudioSceneManager &audioSceneManager(AudioSceneManager::GetInstance());
    AudioDeviceDescriptor audioDeviceDescriptor;

    audioSceneManager.audioScene_ = AUDIO_SCENE_PHONE_CALL;
    int32_t ret = audioVolumeManager.SetVoiceCallVolumeOnDeviceSwitch(
        audioDeviceDescriptor, false);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: SetVoiceCallVolumeOnDeviceSwitch_003
* @tc.desc  : Test AudioVolumeManager::SetVoiceCallVolumeOnDeviceSwitch interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, SetVoiceCallVolumeOnDeviceSwitch_003, TestSize.Level1)
{
    AudioVolumeManager &audioVolumeManager(AudioVolumeManager::GetInstance());
    AudioSceneManager &audioSceneManager(AudioSceneManager::GetInstance());
    AudioDeviceDescriptor audioDeviceDescriptor;

    audioSceneManager.audioScene_ = AUDIO_SCENE_DEFAULT;
    int32_t ret = audioVolumeManager.SetVoiceCallVolumeOnDeviceSwitch(
        audioDeviceDescriptor, true);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: SetVoiceCallVolumeOnDeviceSwitch_004
* @tc.desc  : Test AudioVolumeManager::SetVoiceCallVolumeOnDeviceSwitch interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, SetVoiceCallVolumeOnDeviceSwitch_004, TestSize.Level1)
{
    AudioVolumeManager &audioVolumeManager(AudioVolumeManager::GetInstance());
    AudioSceneManager &audioSceneManager(AudioSceneManager::GetInstance());
    AudioDeviceDescriptor audioDeviceDescriptor;

    audioSceneManager.audioScene_ = AUDIO_SCENE_DEFAULT;
    int32_t ret = audioVolumeManager.SetVoiceCallVolumeOnDeviceSwitch(
        audioDeviceDescriptor, false);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: SetVoiceCallVolumeOnDeviceSwitch_005
* @tc.desc  : Test AudioVolumeManager::SetVoiceCallVolumeOnDeviceSwitch interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, SetVoiceCallVolumeOnDeviceSwitch_005, TestSize.Level1)
{
    AudioVolumeManager &audioVolumeManager(AudioVolumeManager::GetInstance());
    AudioSceneManager &audioSceneManager(AudioSceneManager::GetInstance());
    AudioDeviceDescriptor audioDeviceDescriptor;

    audioSceneManager.audioScene_ = AUDIO_SCENE_DEFAULT;
    audioVolumeManager.increaseNIsShowing_ = true;
    int32_t ret = audioVolumeManager.SetVoiceCallVolumeOnDeviceSwitch(
        audioDeviceDescriptor, false);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: SetVoiceCallVolumeOnDeviceSwitch_006
* @tc.desc  : Test AudioVolumeManager::SetVoiceCallVolumeOnDeviceSwitch interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, SetVoiceCallVolumeOnDeviceSwitch_006, TestSize.Level1)
{
    AudioVolumeManager &audioVolumeManager(AudioVolumeManager::GetInstance());
    AudioSceneManager &audioSceneManager(AudioSceneManager::GetInstance());
    AudioDeviceDescriptor audioDeviceDescriptor;

    audioSceneManager.audioScene_ = AUDIO_SCENE_DEFAULT;
    audioVolumeManager.restoreNIsShowing_ = true;
    int32_t ret = audioVolumeManager.SetVoiceCallVolumeOnDeviceSwitch(
        audioDeviceDescriptor, false);
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

    audioVolumeManager.audioActiveDevice_.AddActiveBtDevice(macAddress);
    auto ret = audioVolumeManager.SetDeviceAbsVolumeSupported(macAddress, support, 0);
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
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_SPEAKER;
    device->networkId_ = LOCAL_NETWORK_ID;
    AudioConnectedDevice::GetInstance().AddConnectedDevice(device);
    audioVolumeManager->GetSystemVolumeLevel(streamType, zoneId);

    zoneId = 0;
    audioVolumeManager->SetRingerModeMute(true);
    audioVolumeManager->GetSystemVolumeLevel(streamType, zoneId);

    AudioConnectedDevice::GetInstance().DelConnectedDevice(LOCAL_NETWORK_ID, DEVICE_TYPE_SPEAKER);
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
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_NEARLINK;
    device->networkId_ = LOCAL_NETWORK_ID;
    AudioConnectedDevice::GetInstance().AddConnectedDevice(device);
    audioVolumeManager->GetSystemVolumeLevel(streamType, zoneId);
    AudioConnectedDevice::GetInstance().DelConnectedDevice(LOCAL_NETWORK_ID, DEVICE_TYPE_NEARLINK);
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
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    device->networkId_ = LOCAL_NETWORK_ID;
    AudioConnectedDevice::GetInstance().AddConnectedDevice(device);
    audioVolumeManager->GetSystemVolumeLevel(streamType, zoneId);
    AudioConnectedDevice::GetInstance().DelConnectedDevice(LOCAL_NETWORK_ID, DEVICE_TYPE_BLUETOOTH_SCO);

    streamType = STREAM_ALL;
    audioVolumeManager->GetSystemVolumeLevel(streamType, zoneId);

    device->deviceType_ = DEVICE_TYPE_NEARLINK;
    device->networkId_ = LOCAL_NETWORK_ID;
    AudioConnectedDevice::GetInstance().AddConnectedDevice(device);
    audioVolumeManager->GetSystemVolumeLevel(streamType, zoneId);
    AudioConnectedDevice::GetInstance().DelConnectedDevice(LOCAL_NETWORK_ID, DEVICE_TYPE_NEARLINK);
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
    auto nearLinkDevice = std::make_shared<AudioDeviceDescriptor>();
    nearLinkDevice->deviceType_ = DEVICE_TYPE_NEARLINK;
    nearLinkDevice->networkId_ = LOCAL_NETWORK_ID;
    AudioConnectedDevice::GetInstance().AddConnectedDevice(nearLinkDevice);
    audioVolumeManager->SetVoiceCallVolume(volumeLevel);
    AudioConnectedDevice::GetInstance().DelConnectedDevice(LOCAL_NETWORK_ID, DEVICE_TYPE_NEARLINK);

    auto lineDigitalDevice = std::make_shared<AudioDeviceDescriptor>();
    lineDigitalDevice->deviceType_ = DEVICE_TYPE_LINE_DIGITAL;
    lineDigitalDevice->networkId_ = LOCAL_NETWORK_ID;
    AudioConnectedDevice::GetInstance().AddConnectedDevice(lineDigitalDevice);
    audioVolumeManager->SetVoiceCallVolume(volumeLevel);
    AudioConnectedDevice::GetInstance().DelConnectedDevice(LOCAL_NETWORK_ID, DEVICE_TYPE_LINE_DIGITAL);
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
    VolumeScale volume{volumeLevel, volumeLevel};
    AudioStreamType streamType = STREAM_MUSIC;
    DeviceType curOutputDeviceType = DEVICE_TYPE_SPEAKER;
    auto device1 = std::make_shared<AudioDeviceDescriptor>();
    device1->deviceType_ = DEVICE_TYPE_SPEAKER;
    device1->networkId_ = LOCAL_NETWORK_ID;
    device1->macAddress_ = "";
    AudioConnectedDevice::GetInstance().AddConnectedDevice(device1);
    AudioVolumeManager::GetInstance().audioActiveDevice_.volumeTypeDeviceMap_[STREAM_MUSIC] = {device1};
    auto ret = audioVolumeManager->HandleNearlinkDeviceAbsVolume(streamType, volume, curOutputDeviceType);
    EXPECT_EQ(ret, ERR_UNKNOWN);

    auto device2 = std::make_shared<AudioDeviceDescriptor>();
    device2->deviceType_ = DEVICE_TYPE_SPEAKER;
    device2->networkId_ = LOCAL_NETWORK_ID;
    device2->macAddress_ = "test";
    AudioConnectedDevice::GetInstance().AddConnectedDevice(device2);
    AudioVolumeManager::GetInstance().audioActiveDevice_.volumeTypeDeviceMap_[STREAM_MUSIC] = {device2};
    ret = audioVolumeManager->HandleNearlinkDeviceAbsVolume(streamType, volume, curOutputDeviceType);
    EXPECT_NE(ret, SUCCESS);

    AudioVolumeManager::GetInstance().audioActiveDevice_.volumeTypeDeviceMap_.erase(STREAM_MUSIC);
    AudioConnectedDevice::GetInstance().DelConnectedDevice(LOCAL_NETWORK_ID, DEVICE_TYPE_SPEAKER);
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
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = nullptr;
    auto nearLinkDevice = std::make_shared<AudioDeviceDescriptor>();
    nearLinkDevice->deviceType_ = DEVICE_TYPE_NEARLINK;
    nearLinkDevice->networkId_ = LOCAL_NETWORK_ID;
    AudioConnectedDevice::GetInstance().AddConnectedDevice(nearLinkDevice);
    // Zone 10 does not exist: the flow falls back to GetDeviceForVolume instead of failing
    // with the zone device error.
    auto ret = audioVolumeManager->SetSystemVolumeLevel(streamType, volumeLevel, deviceDesc, zoneId);
    EXPECT_NE(ret, ERR_OPERATION_FAILED);

    zoneId = 0;
    audioVolumeManager->SetSystemVolumeLevel(streamType, volumeLevel, deviceDesc, zoneId);
    AudioConnectedDevice::GetInstance().DelConnectedDevice(LOCAL_NETWORK_ID, DEVICE_TYPE_NEARLINK);
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
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = nullptr;
    auto btScoDevice = std::make_shared<AudioDeviceDescriptor>();
    btScoDevice->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    btScoDevice->networkId_ = LOCAL_NETWORK_ID;
    AudioConnectedDevice::GetInstance().AddConnectedDevice(btScoDevice);
    audioVolumeManager->SetSystemVolumeLevel(streamType, volumeLevel, deviceDesc, zoneId);

    streamType = STREAM_VOICE_CALL_ASSISTANT;
    audioVolumeManager->SetSystemVolumeLevel(streamType, volumeLevel, deviceDesc, zoneId);
    AudioConnectedDevice::GetInstance().DelConnectedDevice(LOCAL_NETWORK_ID, DEVICE_TYPE_BLUETOOTH_SCO);
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
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = nullptr;
    auto nearLinkDevice = std::make_shared<AudioDeviceDescriptor>();
    nearLinkDevice->deviceType_ = DEVICE_TYPE_NEARLINK;
    nearLinkDevice->networkId_ = LOCAL_NETWORK_ID;
    AudioConnectedDevice::GetInstance().AddConnectedDevice(nearLinkDevice);
    audioVolumeManager->SetSystemVolumeLevel(streamType, volumeLevel, deviceDesc, zoneId);
    AudioConnectedDevice::GetInstance().DelConnectedDevice(LOCAL_NETWORK_ID, DEVICE_TYPE_NEARLINK);
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
    VolumeScale volume{volumeLevel, volumeLevel};
    bool internalCall = true;
    A2dpDeviceConfigInfo a2dpDeviceConfigInfo;
    a2dpDeviceConfigInfo.absVolumeSupport = true;
    audioVolumeManager->audioA2dpDevice_.connectedA2dpDeviceMap_[macAddress] = a2dpDeviceConfigInfo;
    audioVolumeManager->SetA2dpDeviceVolume(macAddress, volume, internalCall);
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
    VolumeScale volume{volumeLevel, volumeLevel};
    bool internalCall = false;
    A2dpDeviceConfigInfo a2dpDeviceConfigInfo;
    a2dpDeviceConfigInfo.absVolumeSupport = true;
    audioVolumeManager->audioA2dpDevice_.connectedA2dpDeviceMap_[macAddress] = a2dpDeviceConfigInfo;
    audioVolumeManager->SetA2dpDeviceVolume(macAddress, volume, internalCall);
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
    int32_t volumeLevel = 1;
    VolumeScale volume{volumeLevel, volumeLevel};
    bool internalCall = true;
    SleVolumeConfigInfo configInfo;
    std::pair<SleVolumeConfigInfo, SleVolumeConfigInfo> pairConfigInfo = std::make_pair(configInfo, configInfo);
    SleAudioDeviceManager::GetInstance().deviceVolumeConfigInfo_["test"] = pairConfigInfo;
    auto ret = audioVolumeManager->SetNearlinkDeviceVolume(macAddress, streamType, volume, internalCall);
    EXPECT_EQ(ret, ERR_UNKNOWN);

    internalCall = false;
    ret = audioVolumeManager->SetNearlinkDeviceVolume(macAddress, streamType, volume, internalCall);
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
    VolumeScale volume{volumeLevel, volumeLevel};
    bool internalCall = true;
    SleVolumeConfigInfo configInfo;
    std::pair<SleVolumeConfigInfo, SleVolumeConfigInfo> pairConfigInfo = std::make_pair(configInfo, configInfo);
    SleAudioDeviceManager::GetInstance().deviceVolumeConfigInfo_["test"] = pairConfigInfo;
    audioVolumeManager->SetNearlinkDeviceVolume(macAddress, streamType, volume, internalCall);

    streamType = STREAM_VOICE_CALL_ASSISTANT;
    audioVolumeManager->SetNearlinkDeviceVolume(macAddress, streamType, volume, internalCall);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_048
* @tc.desc  : Test SetNearlinkDeviceVolume interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_048, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    ASSERT_TRUE(audioVolumeManager != nullptr);

    std::string macAddress = "test";
    AudioStreamType streamType = STREAM_MUSIC;
    int32_t volumeLevel = 1000;
    VolumeScale volume{volumeLevel, volumeLevel};
    bool internalCall = true;
    SleVolumeConfigInfo configInfo;
    std::pair<SleVolumeConfigInfo, SleVolumeConfigInfo> pairConfigInfo = std::make_pair(configInfo, configInfo);
    SleAudioDeviceManager::GetInstance().deviceVolumeConfigInfo_["test"] = pairConfigInfo;
    auto ret = audioVolumeManager->SetNearlinkDeviceVolume(macAddress, streamType, volume, internalCall);
    EXPECT_NE(ret, SUCCESS);

    internalCall = false;
    ret = audioVolumeManager->SetNearlinkDeviceVolume(macAddress, streamType, volume, internalCall);
    EXPECT_NE(ret, SUCCESS);
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

    audioVolumeManager->SetRingerModeMute(false);
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
    auto btA2dpDevice = std::make_shared<AudioDeviceDescriptor>();
    btA2dpDevice->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    btA2dpDevice->networkId_ = LOCAL_NETWORK_ID;
    AudioConnectedDevice::GetInstance().AddConnectedDevice(btA2dpDevice);
    AudioVolumeManager::GetInstance().SetDeviceSafeVolumeStatus();
    EXPECT_EQ(AudioVolumeManager::GetInstance().safeStatusBt_, SAFE_INACTIVE);
    AudioConnectedDevice::GetInstance().DelConnectedDevice(LOCAL_NETWORK_ID, DEVICE_TYPE_BLUETOOTH_A2DP);

    auto wiredHeadsetDevice = std::make_shared<AudioDeviceDescriptor>();
    wiredHeadsetDevice->deviceType_ = DEVICE_TYPE_WIRED_HEADSET;
    wiredHeadsetDevice->networkId_ = LOCAL_NETWORK_ID;
    AudioConnectedDevice::GetInstance().AddConnectedDevice(wiredHeadsetDevice);
    AudioVolumeManager::GetInstance().SetDeviceSafeVolumeStatus();
    EXPECT_EQ(AudioVolumeManager::GetInstance().safeStatus_, SAFE_INACTIVE);
    AudioConnectedDevice::GetInstance().DelConnectedDevice(LOCAL_NETWORK_ID, DEVICE_TYPE_WIRED_HEADSET);
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
    audioVolumeManager->audioActiveDevice_.AddActiveBtDevice("test");
    audioVolumeManager->SetAbsVolumeSceneAsync(macAddress, support, 0);

    audioVolumeManager->audioActiveDevice_.AddActiveBtDevice(macAddress);
    audioVolumeManager->SetAbsVolumeSceneAsync(macAddress, support, 0);
    EXPECT_TRUE(audioVolumeManager->audioActiveDevice_.IsActiveBtDevice(macAddress));
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
    auto ret = audioVolumeManager.SetDeviceAbsVolumeSupported(macAddress, support, 0);
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
    auto ret = audioVolumeManager.SetDeviceAbsVolumeSupported(macAddress, support, 0);
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
    // Zone id 10 does not exist: the flow falls back to the default device instead of failing.
    auto ret = audioVolumeManager.SetStreamMute(streamType, mute, streamUsage, deviceType, zoneId);
    EXPECT_EQ(ret, SUCCESS);
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
    // Restore the mute state so that the assertion checks the fallback read only.
    audioVolumeManager.SetStreamMute(streamType, false, STREAM_USAGE_MUSIC, DEVICE_TYPE_SPEAKER, 0);
    auto ret = audioVolumeManager.GetStreamMute(streamType, zoneId);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test SetStreamMute zone fallback.
* @tc.number: AudioVolumeManagerZoneFallback_001
* @tc.desc  : zoneId > 0 but the zone does not exist: the device fetch falls back to the
*             default device and the mute is applied instead of failing with a zone error.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManagerZoneFallback_001, TestSize.Level4)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    AudioStreamType streamType = STREAM_MUSIC;
    bool mute = true;
    StreamUsage streamUsage = STREAM_USAGE_MUSIC;
    DeviceType deviceType = DEVICE_TYPE_NEARLINK;
    int32_t zoneId = 10; // zone id that does not exist
    auto ret = audioVolumeManager.SetStreamMute(streamType, mute, streamUsage, deviceType, zoneId);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test GetStreamMute zone fallback.
* @tc.number: AudioVolumeManagerZoneFallback_002
* @tc.desc  : zoneId > 0 but the zone does not exist: GetStreamMute falls back to the
*             default device and reads the mute state applied to it.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManagerZoneFallback_002, TestSize.Level4)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    AudioStreamType streamType = STREAM_MUSIC;
    // The zone does not exist, so the mute is applied to the default device.
    auto ret = audioVolumeManager.SetStreamMute(streamType, true, STREAM_USAGE_MUSIC, DEVICE_TYPE_SPEAKER, 10);
    EXPECT_EQ(ret, SUCCESS);
    bool isMuted = audioVolumeManager.GetStreamMute(streamType, 0);
    EXPECT_TRUE(isMuted);
    audioVolumeManager.SetStreamMute(streamType, false, STREAM_USAGE_MUSIC, DEVICE_TYPE_SPEAKER, 0);
}

/**
* @tc.name  : Test A2DP absolute volume is keyed by the target device mac address.
* @tc.number: AudioVolumeManagerA2dpAbsVolume_001
* @tc.desc  : GetSystemVolumeLevel reads the absolute volume of the fetched A2DP device by
*             its mac address instead of the single active bluetooth device.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManagerA2dpAbsVolume_001, TestSize.Level4)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    ASSERT_TRUE(audioVolumeManager != nullptr);

    std::string macAddress = "11:22:33:44:55:66";
    auto a2dpDevice = std::make_shared<AudioDeviceDescriptor>();
    a2dpDevice->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    a2dpDevice->macAddress_ = macAddress;
    a2dpDevice->networkId_ = LOCAL_NETWORK_ID;
    AudioConnectedDevice::GetInstance().AddConnectedDevice(a2dpDevice);
    A2dpDeviceConfigInfo info;
    info.absVolumeSupport = true;
    info.volumeLevel = 5;
    info.mute = false;
    audioVolumeManager->audioA2dpDevice_.AddA2dpDevice(macAddress, info);
    AudioVolumeManager::GetInstance().audioActiveDevice_.volumeTypeDeviceMap_[STREAM_MUSIC] = {a2dpDevice};

    int32_t volumeLevel = audioVolumeManager->GetSystemVolumeLevel(STREAM_MUSIC);
    EXPECT_EQ(volumeLevel, 5);

    info.mute = true;
    audioVolumeManager->audioA2dpDevice_.AddA2dpDevice(macAddress, info);
    volumeLevel = audioVolumeManager->GetSystemVolumeLevel(STREAM_MUSIC);
    EXPECT_EQ(volumeLevel, 0);

    AudioVolumeManager::GetInstance().audioActiveDevice_.volumeTypeDeviceMap_.erase(STREAM_MUSIC);
    AudioConnectedDevice::GetInstance().DelConnectedDevice(LOCAL_NETWORK_ID, DEVICE_TYPE_BLUETOOTH_A2DP);
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
    AudioVolumeManager &audioVolumeManager(AudioVolumeManager::GetInstance());
    AudioStreamType streamType = STREAM_MUSIC;
    int32_t volumeLevel = 7;
    int32_t volumeDegree = 44;
    VolumeScale volume{volumeLevel, volumeDegree};
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = nullptr;
    int32_t ret = audioVolumeManager.SetSystemVolumeLevel(streamType, volume, deviceDesc, 0);
    EXPECT_EQ(ret, SUCCESS);

    ret = audioVolumeManager.GetSystemVolumeDegree(STREAM_ALL);
    EXPECT_EQ(ret, volumeDegree);

    ret = audioVolumeManager.GetMinVolumeDegree(streamType, DEVICE_TYPE_NONE);
    EXPECT_EQ(ret, 0);

    ret = audioVolumeManager.GetMinVolumeDegree(STREAM_ALL, DEVICE_TYPE_NONE);
    EXPECT_EQ(ret, 0);

    bool oldMuteState = audioVolumeManager.IsRingerModeMute();
    audioVolumeManager.SetRingerModeMute(false);
    ret = audioVolumeManager.GetSystemVolumeDegree(STREAM_RING);
    EXPECT_EQ(ret, 0);

    int32_t zoneId = 0;
    volume = volumeLevel;
    auto btScoDevice = std::make_shared<AudioDeviceDescriptor>();
    btScoDevice->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    btScoDevice->networkId_ = LOCAL_NETWORK_ID;
    AudioConnectedDevice::GetInstance().AddConnectedDevice(btScoDevice);
    audioVolumeManager.SetSystemVolumeLevel(streamType, volume, deviceDesc, zoneId);
    ret = audioVolumeManager.GetSystemVolumeDegree(streamType);
    EXPECT_GT(ret, 0);

    audioVolumeManager.SetSystemVolumeLevel(streamType, volume, deviceDesc, zoneId);
    ret = audioVolumeManager.GetSystemVolumeDegree(streamType);
    EXPECT_GT(ret, 0);
    AudioConnectedDevice::GetInstance().DelConnectedDevice(LOCAL_NETWORK_ID, DEVICE_TYPE_BLUETOOTH_SCO);

    audioVolumeManager.SetRingerModeMute(oldMuteState);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManagerDegree_002
* @tc.desc  : Test SetSystemVolumeDegreeToDbInner interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManagerDegree_002, TestSize.Level1)
{
    AudioVolumeManager &audioVolumeManager(AudioVolumeManager::GetInstance());
    int32_t invalidZone = 1;
    AudioStreamType streamType = STREAM_MUSIC;
    int32_t volumeDegree = 44;
    int32_t volumeLevel = 7;
    VolumeScale volume{volumeLevel, volumeDegree};
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = nullptr;
    // Zone 1 does not exist: the flow falls back to the default device instead of failing.
    int32_t ret = audioVolumeManager.SetSystemVolumeLevel(streamType, volume,
        deviceDesc, invalidZone);
    EXPECT_EQ(ret, SUCCESS);
    // The degree read also falls back to the default device and returns the set degree.
    ret = audioVolumeManager.GetSystemVolumeDegree(streamType, invalidZone);
    EXPECT_EQ(ret, volumeDegree);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManagerDegree_003
* @tc.desc  : Test GetSystemVolumeDegree returns A2DP abs volume degree.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManagerDegree_003, TestSize.Level1)
{
    AudioVolumeManager &audioVolumeManager(AudioVolumeManager::GetInstance());
    std::string macAddress = "11:22:33:44:55:77";
    int32_t expectedVolumeDegree = 66;
    auto btA2dpDevice = std::make_shared<AudioDeviceDescriptor>();
    btA2dpDevice->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    btA2dpDevice->networkId_ = LOCAL_NETWORK_ID;
    btA2dpDevice->macAddress_ = macAddress;
    AudioConnectedDevice::GetInstance().AddConnectedDevice(btA2dpDevice);
    audioVolumeManager.audioActiveDevice_.AddActiveBtDevice(macAddress);

    A2dpDeviceConfigInfo a2dpDeviceConfigInfo;
    a2dpDeviceConfigInfo.absVolumeSupport = true;
    a2dpDeviceConfigInfo.mute = false;
    a2dpDeviceConfigInfo.volumeDegree = expectedVolumeDegree;
    audioVolumeManager.audioA2dpDevice_.connectedA2dpDeviceMap_[macAddress] = a2dpDeviceConfigInfo;

    auto ret = audioVolumeManager.GetSystemVolumeDegree(STREAM_MUSIC, DEVICE_TYPE_BLUETOOTH_A2DP);
    EXPECT_EQ(ret, expectedVolumeDegree);

    audioVolumeManager.audioA2dpDevice_.connectedA2dpDeviceMap_.erase(macAddress);
    audioVolumeManager.audioActiveDevice_.RemoveActiveBtDevice(macAddress);
    AudioConnectedDevice::GetInstance().DelConnectedDevice(LOCAL_NETWORK_ID, DEVICE_TYPE_BLUETOOTH_A2DP);
}


/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManagerDegree_004
* @tc.desc  : Test GetSystemVolumeDegree returns NearLink voice call degree.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManagerDegree_004, TestSize.Level1)
{
    AudioVolumeManager &audioVolumeManager(AudioVolumeManager::GetInstance());
    std::string macAddress = "nearlink_test_device";
    int32_t expectedVolumeDegree = 52;
    auto nearLinkDevice = std::make_shared<AudioDeviceDescriptor>();
    nearLinkDevice->deviceType_ = DEVICE_TYPE_NEARLINK;
    nearLinkDevice->networkId_ = LOCAL_NETWORK_ID;
    nearLinkDevice->macAddress_ = macAddress;
    AudioConnectedDevice::GetInstance().AddConnectedDevice(nearLinkDevice);

    SleVolumeConfigInfo musicConfigInfo;
    SleVolumeConfigInfo callConfigInfo;
    callConfigInfo.volumeDegree = expectedVolumeDegree;
    SleAudioDeviceManager::GetInstance().deviceVolumeConfigInfo_[macAddress] =
        std::make_pair(musicConfigInfo, callConfigInfo);

    auto ret = audioVolumeManager.GetSystemVolumeDegree(STREAM_VOICE_CALL, DEVICE_TYPE_NEARLINK);
    EXPECT_EQ(ret, expectedVolumeDegree);

    SleAudioDeviceManager::GetInstance().deviceVolumeConfigInfo_.erase(macAddress);
    AudioConnectedDevice::GetInstance().DelConnectedDevice(LOCAL_NETWORK_ID, DEVICE_TYPE_NEARLINK);
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
    ret = audioVolumeManager->Init(nullptr, nullptr);
    EXPECT_EQ(ret, true);
    EXPECT_NE(audioVolumeManager->forceControlVolumeTypeMonitor_, nullptr);
    ret = audioVolumeManager->Init(nullptr, nullptr);
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
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_067
* @tc.desc  : Test SetNearlinkDeviceVolume interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_067, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    ASSERT_TRUE(audioVolumeManager != nullptr);

    AudioStreamType streamType = STREAM_MUSIC;
    int32_t volumeLevel = 8;
    auto devDesc = make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_NEARLINK, DeviceRole::OUTPUT_DEVICE);
    devDesc->deviceId_ = TEST_DEVICE_ID;
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(devDesc);
    AudioRouterSelectStrategy::GetInstance().UpdateCurrentOutputDevice(SYSTEM_UID, {devDesc});
    auto ret = audioVolumeManager->SetNearlinkDeviceVolumeEx(streamType, volumeLevel);
    EXPECT_EQ(ret, SUCCESS);
    auto devDesc2 = make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_SPEAKER, DeviceRole::OUTPUT_DEVICE);
    devDesc2->deviceId_ = 1002;
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(devDesc2);
    AudioRouterSelectStrategy::GetInstance().UpdateCurrentOutputDevice(SYSTEM_UID, {devDesc2});
    ret = audioVolumeManager->SetNearlinkDeviceVolumeEx(streamType, volumeLevel);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_068
* @tc.desc  : Test CheckLowerDeviceVolume interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_068, TestSize.Level1)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    audioVolumeManager.audioPolicyManager_.Init();
    audioVolumeManager.audioPolicyManager_.SetDataShareReady(true);
    const int32_t curVolume = 10;
    auto ret = audioVolumeManager.audioPolicyManager_.SetRestoreVolumeLevel(DEVICE_TYPE_BLUETOOTH_A2DP, curVolume);
    EXPECT_EQ(ret, SUCCESS);
    ret = audioVolumeManager.audioPolicyManager_.SetRestoreVolumeLevel(DEVICE_TYPE_WIRED_HEADSET, curVolume);
    EXPECT_EQ(ret, SUCCESS);

    audioVolumeManager.CheckLowerDeviceVolume(DEVICE_TYPE_BLUETOOTH_A2DP);
    audioVolumeManager.CheckLowerDeviceVolume(DEVICE_TYPE_WIRED_HEADSET);
    audioVolumeManager.CheckLowerDeviceVolume(DEVICE_TYPE_SPEAKER);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_VolumeLimit_001
* @tc.desc  : Test volume limit interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_VolumeLimit_001, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    ASSERT_TRUE(audioVolumeManager != nullptr);

    int32_t zoneId = 0;
    int32_t volumeLevel = 2;
    AudioStreamType streamType = STREAM_VOICE_CALL;

    auto &manager = static_cast<AudioAdapterManager &>(audioVolumeManager->audioPolicyManager_);
    float oldLimit = manager.volumeLimit_.load();
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = nullptr;
    audioVolumeManager->audioSceneManager_.audioScene_ = AUDIO_SCENE_PHONE_CALL;
    audioVolumeManager->CheckReduceOtherActiveVolume(STREAM_MUSIC, volumeLevel);
    audioVolumeManager->SetSystemVolumeLevel(streamType, volumeLevel, deviceDesc, zoneId);
    audioVolumeManager->audioSceneManager_.audioScene_ = AUDIO_SCENE_DEFAULT;
    audioVolumeManager->CheckReduceOtherActiveVolume(STREAM_MUSIC, volumeLevel);
    audioVolumeManager->CheckReduceOtherActiveVolume(streamType, volumeLevel);
    float newLimit = manager.volumeLimit_.load();
    EXPECT_NE(newLimit, oldLimit);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_069
* @tc.desc  : Test CheckRestoreDeviceVolume interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_069, TestSize.Level1)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    audioVolumeManager.audioPolicyManager_.Init();
    audioVolumeManager.audioPolicyManager_.SetDataShareReady(true);
    const int32_t curVolume = 10;
    auto ret = audioVolumeManager.audioPolicyManager_.SetRestoreVolumeLevel(DEVICE_TYPE_BLUETOOTH_A2DP, curVolume);
    EXPECT_EQ(ret, SUCCESS);
    ret = audioVolumeManager.audioPolicyManager_.SetRestoreVolumeLevel(DEVICE_TYPE_WIRED_HEADSET, curVolume);
    EXPECT_EQ(ret, SUCCESS);

    ret = audioVolumeManager.CheckRestoreDeviceVolume(DEVICE_TYPE_BLUETOOTH_A2DP);
    EXPECT_EQ(ret, SUCCESS);
    ret = audioVolumeManager.CheckRestoreDeviceVolume(DEVICE_TYPE_WIRED_HEADSET);
    EXPECT_EQ(ret, SUCCESS);
    ret = audioVolumeManager.CheckRestoreDeviceVolume(DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_070
* @tc.desc  : Test SetRestoreVolumeLevel interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_070, TestSize.Level1)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    audioVolumeManager.audioPolicyManager_.Init();
    audioVolumeManager.audioPolicyManager_.SetDataShareReady(true);
    const int32_t curVolume = 100;
    audioVolumeManager.SetRestoreVolumeLevel(DEVICE_TYPE_BLUETOOTH_A2DP, curVolume);
    EXPECT_EQ(audioVolumeManager.btRestoreVol_, curVolume);
    audioVolumeManager.SetRestoreVolumeLevel(DEVICE_TYPE_WIRED_HEADSET, curVolume);
    EXPECT_EQ(audioVolumeManager.wiredRestoreVol_, curVolume);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: SetVoiceCallVolumeOnDeviceSwitch_008
* @tc.desc  : Test AudioVolumeManager::SetVoiceCallVolumeOnDeviceSwitch interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, SetVoiceCallVolumeOnDeviceSwitch_008, TestSize.Level1)
{
    auto avm = std::make_shared<AudioVolumeManager>();
    AudioDeviceDescriptor desc;
    avm->SetVoiceCallVolumeOnDeviceSwitch(desc, false);
    desc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    avm->SetVoiceCallVolumeOnDeviceSwitch(desc, false);
    avm->audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    avm->SetVoiceCallVolumeOnDeviceSwitch(desc, false);
    desc.deviceType_ = DEVICE_TYPE_SPEAKER;
    int32_t ret = avm->SetVoiceCallVolumeOnDeviceSwitch(desc, false);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_071
* @tc.desc  : Test OnCheckActiveMusicTime interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_071, TestSize.Level1)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    std::string reason = "Started";
    audioVolumeManager.startSafeTime_ = 0;
    audioVolumeManager.OnCheckActiveMusicTime(reason);
    EXPECT_EQ(audioVolumeManager.startSafeTime_, 0);
    reason = "Paused";
    audioVolumeManager.OnCheckActiveMusicTime(reason);
    EXPECT_EQ(audioVolumeManager.startSafeTime_, 0);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_072
* @tc.desc  : Test CheckActiveMusicTime interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_072, TestSize.Level1)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    std::string reason = "Default";
    audioVolumeManager.safeVolumeExit_ = true;
    int32_t ret = audioVolumeManager.CheckActiveMusicTime(reason);
    EXPECT_EQ(ret, 0);
    audioVolumeManager.safeVolumeExit_ = false;
    audioVolumeManager.startSafeTime_ = 0;
    audioVolumeManager.CheckActiveMusicTime(reason);
    audioVolumeManager.safeVolumeExit_ = true;
    EXPECT_EQ(audioVolumeManager.startSafeTime_, 0);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_073
* @tc.desc  : Test CheckActiveMusicTime interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_073, TestSize.Level1)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.safeVolumeExit_ = false;
    std::string reason = "Offload";
    audioVolumeManager.startSafeTimeBt_ = 0;
    audioVolumeManager.safeStatusBt_ = SAFE_INACTIVE;
    audioVolumeManager.CheckActiveMusicTime(reason);
    EXPECT_EQ(audioVolumeManager.startSafeTimeBt_, 0);
    audioVolumeManager.startSafeTimeBt_ = 0;
    audioVolumeManager.safeStatusBt_ = SAFE_ACTIVE;
    audioVolumeManager.CheckActiveMusicTime(reason);
    EXPECT_EQ(audioVolumeManager.startSafeTimeBt_, 0);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_074
* @tc.desc  : Test CheckActiveMusicTime interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_074, TestSize.Level1)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.safeVolumeExit_ = false;
    std::string reason = "Offload";
    audioVolumeManager.startSafeTime_ = 0;
    audioVolumeManager.safeStatus_ = SAFE_INACTIVE;
    audioVolumeManager.CheckActiveMusicTime(reason);
    EXPECT_EQ(audioVolumeManager.startSafeTime_, 0);
    audioVolumeManager.startSafeTime_ = 0;
    audioVolumeManager.safeStatus_ = SAFE_ACTIVE;
    audioVolumeManager.CheckActiveMusicTime(reason);
    EXPECT_EQ(audioVolumeManager.startSafeTime_, 0);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_075
* @tc.desc  : Test CheckActiveMusicTime interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_075, TestSize.Level1)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.safeVolumeExit_ = false;
    std::string reason = "Offload";
    audioVolumeManager.startSafeTimeSle_ = 0;
    audioVolumeManager.safeStatusSle_ = SAFE_INACTIVE;
    audioVolumeManager.CheckActiveMusicTime(reason);
    EXPECT_EQ(audioVolumeManager.startSafeTimeSle_, 0);
    audioVolumeManager.startSafeTimeSle_ = 0;
    audioVolumeManager.safeStatusSle_ = SAFE_ACTIVE;
    audioVolumeManager.CheckActiveMusicTime(reason);
    EXPECT_EQ(audioVolumeManager.startSafeTimeSle_, 0);
}

/**
* @tc.name  : Test AudioVolumeManager.
* @tc.number: AudioVolumeManager_076
* @tc.desc  : Test SetAbsVolumeSceneAsync interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_076, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    ASSERT_TRUE(audioVolumeManager != nullptr);

    bool support = false;
    std::string macAddress = "11:22:33:44:55:66";
    audioVolumeManager->audioActiveDevice_.AddActiveBtDevice("test");
    audioVolumeManager->SetAbsVolumeSceneAsync(macAddress, support, 0);

    audioVolumeManager->audioActiveDevice_.AddActiveBtDevice(macAddress);
    audioVolumeManager->SetAbsVolumeSceneAsync(macAddress, support, 0);
    EXPECT_TRUE(audioVolumeManager->audioActiveDevice_.IsActiveBtDevice(macAddress));
}

/**
* @tc.name  : Test SetStreamMute.
* @tc.number: SetStreamMute_001
* @tc.desc  : Test SetStreamMute interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, SetStreamMute_001, TestSize.Level1)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    AudioStreamType streamType = STREAM_MUSIC;
    bool mute = true;
    StreamUsage streamUsage = STREAM_USAGE_MUSIC;
    DeviceType deviceType = DEVICE_TYPE_NEARLINK;
    int32_t zoneId = -1;
    auto ret = audioVolumeManager.SetStreamMute(streamType, mute, streamUsage, deviceType, zoneId);
    EXPECT_EQ(ret, 0);
}

/**
* @tc.name  : Test SetStreamMute.
* @tc.number: SetStreamMute_002
* @tc.desc  : Test SetStreamMute interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, SetStreamMute_002, TestSize.Level1)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    AudioStreamType streamType = STREAM_MUSIC;
    bool mute = false;
    StreamUsage streamUsage = STREAM_USAGE_RINGTONE;
    DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    int32_t zoneId = -1;
    auto ret = audioVolumeManager.SetStreamMute(streamType, mute, streamUsage, deviceType, zoneId);
    EXPECT_EQ(ret, 0);
}

/**
* @tc.name  : Test GetStreamMute.
* @tc.number: GetStreamMute_001
* @tc.desc  : Test GetStreamMute interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, GetStreamMute_001, TestSize.Level1)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    AudioStreamType streamType = STREAM_RING;
    bool mute = true;
    StreamUsage streamUsage = STREAM_USAGE_RINGTONE;
    DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    int32_t resultSet;
    int32_t zoneId = 0;
    resultSet = audioVolumeManager.SetStreamMute(streamType, mute, streamUsage, deviceType, zoneId);
    EXPECT_EQ(resultSet, 0);
    bool ret = audioVolumeManager.GetStreamMute(streamType, zoneId);
    EXPECT_TRUE(ret);
}

/**
* @tc.name  : Test GetStreamMute.
* @tc.number: GetStreamMute_002
* @tc.desc  : Test GetStreamMute interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, GetStreamMute_002, TestSize.Level1)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    AudioStreamType streamType = STREAM_MUSIC;
    bool mute = true;
    StreamUsage streamUsage = STREAM_USAGE_MUSIC;
    DeviceType deviceType = DEVICE_TYPE_NEARLINK;
    int32_t resultSet;
    int32_t zoneId = 0;
    resultSet = audioVolumeManager.SetStreamMute(streamType, mute, streamUsage, deviceType, zoneId);
    EXPECT_EQ(resultSet, 0);
    bool ret = audioVolumeManager.GetStreamMute(streamType, zoneId);
    EXPECT_TRUE(ret);
}

/**
* @tc.name  : Test GetStreamMute.
* @tc.number: GetStreamMute_003
* @tc.desc  : Test GetStreamMute interface.
*/
HWTEST_F(AudioVolumeManagerUnitTest, GetStreamMute_003, TestSize.Level1)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    AudioStreamType streamType = STREAM_MUSIC;
    bool mute = true;
    StreamUsage streamUsage = STREAM_USAGE_MUSIC;
    DeviceType deviceType = DEVICE_TYPE_NEARLINK;
    int32_t resultSet;
    int32_t zoneId = 0;
    resultSet = audioVolumeManager.SetStreamMute(streamType, mute, streamUsage, deviceType, zoneId);
    EXPECT_EQ(resultSet, 0);
    bool ret = audioVolumeManager.GetStreamMute(streamType, zoneId);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: AudioVolumeManager_InitAudioZoneVolume_001
 * @tc.desc  : Call InitAudioZoneVolume with null and valid device descriptor to ensure no crash.
 */
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_InitAudioZoneVolume_001, TestSize.Level1)
{
    AudioVolumeManager &audioVolumeManager = AudioVolumeManager::GetInstance();

    // Case 1: nullptr descriptor
    std::shared_ptr<AudioDeviceDescriptor> nullDesc = nullptr;
    audioVolumeManager.InitAudioZoneVolume(0, nullDesc);

    // Case 2: valid descriptor
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_SPEAKER;
    desc->networkId_ = "unit_test";
    audioVolumeManager.InitAudioZoneVolume(1, desc);

    // No direct return value to assert — ensure function executes without crashing.
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test AudioVolumeManager::SetAudioZoneVolumeConfig.
 * @tc.number: AudioVolumeManager_SetAudioZoneVolumeConfig_001
 * @tc.desc  : Call SetAudioZoneVolumeConfig with empty and non-empty config vector.
 */
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_SetAudioZoneVolumeConfig_001, TestSize.Level1)
{
    AudioVolumeManager &audioVolumeManager = AudioVolumeManager::GetInstance();

    std::string zoneName = "UnitTestZone";
    std::vector<AudioZoneVolumeConfig> emptyConfigs;
    int32_t ret = audioVolumeManager.SetAudioZoneVolumeConfig(zoneName, emptyConfigs);
    // Expect not a generic ERROR (implementation may return SUCCESS or other codes)
    EXPECT_NE(ret, ERROR);

    // Provide a minimal config if type is default-constructible
    std::vector<AudioZoneVolumeConfig> configs;
    AudioZoneVolumeConfig cfg;
    // populate minimal fields if available (best-effort, leave defaults otherwise)
    configs.push_back(cfg);
    ret = audioVolumeManager.SetAudioZoneVolumeConfig(zoneName, configs);
    EXPECT_NE(ret, ERROR);
}

/**
 * @tc.name  : Test AudioVolumeManager::TriggerAudioZoneVolumeSync.
 * @tc.number: AudioVolumeManager_TriggerAudioZoneVolumeSync_001
 * @tc.desc  : Call TriggerAudioZoneVolumeSync for default and a specific zone id.
 */
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_TriggerAudioZoneVolumeSync_001, TestSize.Level1)
{
    AudioVolumeManager &audioVolumeManager = AudioVolumeManager::GetInstance();

    // Trigger default zone sync (zoneId = 0)
    audioVolumeManager.TriggerAudioZoneVolumeSync(0);

    // Trigger sync for a specific zone
    audioVolumeManager.TriggerAudioZoneVolumeSync(1);

    // No return value — test passes if no crash
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test ClearLoudVolumeHoldMapForCall.
 * @tc.number: ClearLoudVolumeHoldMapForCall_001
 * @tc.desc  : Test ClearLoudVolumeHoldMapForCall interface with normal case.
 */
HWTEST_F(AudioVolumeManagerUnitTest, ClearLoudVolumeHoldMapForCall_001, TestSize.Level1)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    int32_t ret = audioVolumeManager.ClearLoudVolumeHoldMapForCall();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test ClearLoudVolumeHoldMapForCall.
 * @tc.number: ClearLoudVolumeHoldMapForCall_002
 * @tc.desc  : Test ClearLoudVolumeHoldMapForCall interface with nullptr loudVolumeManager.
 */
HWTEST_F(AudioVolumeManagerUnitTest, ClearLoudVolumeHoldMapForCall_002, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    ASSERT_TRUE(audioVolumeManager != nullptr);

    audioVolumeManager->loudVolumeManager_ = nullptr;
    int32_t ret = audioVolumeManager->ClearLoudVolumeHoldMapForCall();
    EXPECT_EQ(ret, SUCCESS);
}

// ---------------------------------------------------------------------------
// Nearlink karaoke volume mode UT
// Coverage scope: guard / idempotent / error branches that do not require a
// real nearlink device to be active. The device-dependent happy paths
// (enable/disable main flow, RefreshKaraokePipeVolume
// success) need an active nearlink device + HAL and are left to integration
// tests. Private members/methods are reachable via -fno-access-control.
// ---------------------------------------------------------------------------

/**
* @tc.name  : Test IsNearlinkKaraokeVolumeManaged.
* @tc.number: IsNearlinkKaraokeVolumeManaged_001
* @tc.desc  : Non-MUSIC stream is not managed by nearlink karaoke volume.
*/
HWTEST_F(AudioVolumeManagerUnitTest, IsNearlinkKaraokeVolumeManaged_001, TestSize.Level1)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    // streamType != STREAM_MUSIC branch -> false
    EXPECT_FALSE(audioVolumeManager.IsNearlinkKaraokeVolumeManaged(STREAM_VOICE_CALL));
}

/**
* @tc.name  : Test IsNearlinkKaraokeVolumeManaged.
* @tc.number: IsNearlinkKaraokeVolumeManaged_002
* @tc.desc  : MUSIC on non-nearlink output is not managed by nearlink karaoke volume.
*/
HWTEST_F(AudioVolumeManagerUnitTest, IsNearlinkKaraokeVolumeManaged_002, TestSize.Level1)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    // active output device is not nearlink branch -> false
    EXPECT_FALSE(audioVolumeManager.IsNearlinkKaraokeVolumeManaged(STREAM_MUSIC));
}

/**
* @tc.name  : Test SetNearlinkKaraokeVolumeMode.
* @tc.number: SetNearlinkKaraokeVolumeMode_001
* @tc.desc  : Enable returns ERROR when no nearlink device is active (device check guard).
*/
HWTEST_F(AudioVolumeManagerUnitTest, SetNearlinkKaraokeVolumeMode_001, TestSize.Level1)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    // curOutputDeviceType != DEVICE_TYPE_NEARLINK -> ERROR
    EXPECT_EQ(audioVolumeManager.SetNearlinkKaraokeVolumeMode(true), ERROR);
}

/**
* @tc.name  : Test SetNearlinkKaraokeVolumeMode.
* @tc.number: SetNearlinkKaraokeVolumeMode_002
* @tc.desc  : Disable with no nearlink device still restores mode to NEARLINK and returns ERROR.
*/
HWTEST_F(AudioVolumeManagerUnitTest, SetNearlinkKaraokeVolumeMode_002, TestSize.Level1)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    audioVolumeManager.nearlinkKaraokeMode_ = NearlinkKaraokeVolumeMode::VOLUME_FWK;
    // curOutputDeviceType != DEVICE_TYPE_NEARLINK -> restore mode flag, return ERROR
    EXPECT_EQ(audioVolumeManager.SetNearlinkKaraokeVolumeMode(false), ERROR);
    EXPECT_EQ(audioVolumeManager.GetNearlinkKaraokeMode(), NearlinkKaraokeVolumeMode::VOLUME_NEARLINK);
}

/**
* @tc.name  : Test EnableNearlinkKaraokeVolume idempotent.
* @tc.number: EnableNearlinkKaraokeVolume_001
* @tc.desc  : Enable returns SUCCESS immediately when already in FWK mode (idempotent guard).
*/
HWTEST_F(AudioVolumeManagerUnitTest, EnableNearlinkKaraokeVolume_001, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    ASSERT_TRUE(audioVolumeManager != nullptr);
    // already VOLUME_FWK -> idempotent return SUCCESS before touching device/HAL
    audioVolumeManager->nearlinkKaraokeMode_ = NearlinkKaraokeVolumeMode::VOLUME_FWK;
    EXPECT_EQ(audioVolumeManager->EnableNearlinkKaraokeVolume(), SUCCESS);
}

/**
* @tc.name  : Test DisableNearlinkKaraokeVolume idempotent.
* @tc.number: DisableNearlinkKaraokeVolume_001
* @tc.desc  : Disable returns SUCCESS immediately when already in NEARLINK mode (idempotent guard).
*/
HWTEST_F(AudioVolumeManagerUnitTest, DisableNearlinkKaraokeVolume_001, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    ASSERT_TRUE(audioVolumeManager != nullptr);
    // already VOLUME_NEARLINK -> idempotent return SUCCESS before touching device/HAL
    audioVolumeManager->nearlinkKaraokeMode_ = NearlinkKaraokeVolumeMode::VOLUME_NEARLINK;
    EXPECT_EQ(audioVolumeManager->DisableNearlinkKaraokeVolume(), SUCCESS);
}
} // namespace AudioStandard
} // namespace OHOS
