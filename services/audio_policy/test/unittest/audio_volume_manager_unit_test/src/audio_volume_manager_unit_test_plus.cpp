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
#include "audio_utils.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
constexpr int32_t DEFAULT_ZONE_ID = 0;

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
* @tc.number: AudioVolumeManager_103
* @tc.desc  : Test AudioVolumeManager.
*/
HWTEST_F(AudioVolumeManagerUnitTest, AudioVolumeManager_103, TestSize.Level1)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    EXPECT_NE(audioVolumeManager, nullptr);

    AudioStreamType streamType = STREAM_MUSIC;
    int32_t volumeLevel = -1;
    auto usbHeadsetDevice = std::make_shared<AudioDeviceDescriptor>();
    usbHeadsetDevice->deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;
    usbHeadsetDevice->networkId_ = LOCAL_NETWORK_ID;
    AudioConnectedDevice::GetInstance().AddConnectedDevice(usbHeadsetDevice);
    audioVolumeManager->increaseNIsShowing_ = true;
    audioVolumeManager->restoreNIsShowing_ = true;
    EXPECT_EQ(VolumeUtils::GetVolumeTypeFromStreamType(streamType), STREAM_MUSIC);

    audioVolumeManager->CheckToCloseNotification(streamType, volumeLevel);
    EXPECT_EQ(audioVolumeManager->increaseNIsShowing_, true);
    EXPECT_EQ(audioVolumeManager->restoreNIsShowing_, true);
    AudioConnectedDevice::GetInstance().DelConnectedDevice(LOCAL_NETWORK_ID, DEVICE_TYPE_USB_ARM_HEADSET);
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
    auto usbHeadsetDevice = std::make_shared<AudioDeviceDescriptor>();
    usbHeadsetDevice->deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;
    usbHeadsetDevice->networkId_ = LOCAL_NETWORK_ID;
    AudioConnectedDevice::GetInstance().AddConnectedDevice(usbHeadsetDevice);
    audioVolumeManager->increaseNIsShowing_ = false;
    audioVolumeManager->restoreNIsShowing_ = false;
    EXPECT_EQ(VolumeUtils::GetVolumeTypeFromStreamType(streamType), STREAM_MUSIC);

    audioVolumeManager->CheckToCloseNotification(streamType, volumeLevel);
    AudioConnectedDevice::GetInstance().DelConnectedDevice(LOCAL_NETWORK_ID, DEVICE_TYPE_USB_ARM_HEADSET);
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
    auto usbHeadsetDevice = std::make_shared<AudioDeviceDescriptor>();
    usbHeadsetDevice->deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;
    usbHeadsetDevice->networkId_ = LOCAL_NETWORK_ID;
    AudioConnectedDevice::GetInstance().AddConnectedDevice(usbHeadsetDevice);
    EXPECT_NE(VolumeUtils::GetVolumeTypeFromStreamType(streamType), STREAM_MUSIC);

    StreamUsage streamUsage = STREAM_USAGE_MUSIC;
    audioVolumeManager->CheckToCloseNotification(streamType, volumeLevel);
    AudioVolumeType audioVolumeType = VolumeUtils::GetVolumeTypeFromStreamUsage(streamUsage);
    EXPECT_EQ(audioVolumeType, STREAM_MUSIC);
    AudioConnectedDevice::GetInstance().DelConnectedDevice(LOCAL_NETWORK_ID, DEVICE_TYPE_USB_ARM_HEADSET);
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
    auto maxDevice = std::make_shared<AudioDeviceDescriptor>();
    maxDevice->deviceType_ = DEVICE_TYPE_MAX;
    maxDevice->networkId_ = LOCAL_NETWORK_ID;
    AudioConnectedDevice::GetInstance().AddConnectedDevice(maxDevice);
    EXPECT_NE(VolumeUtils::GetVolumeTypeFromStreamType(streamType), STREAM_MUSIC);
    auto curDevice = audioVolumeManager->audioActiveDevice_.GetDeviceForVolume(streamType);
    EXPECT_EQ(audioVolumeManager->safeVolumeManager_.DeviceIsSupportSafeVolume(curDevice), false);

    audioVolumeManager->CheckToCloseNotification(streamType, volumeLevel);
    AudioConnectedDevice::GetInstance().DelConnectedDevice(LOCAL_NETWORK_ID, DEVICE_TYPE_MAX);
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
    auto maxDevice = std::make_shared<AudioDeviceDescriptor>();
    maxDevice->deviceType_ = DEVICE_TYPE_MAX;
    maxDevice->networkId_ = LOCAL_NETWORK_ID;
    AudioConnectedDevice::GetInstance().AddConnectedDevice(maxDevice);
    EXPECT_NE(VolumeUtils::GetVolumeTypeFromStreamType(streamType), STREAM_MUSIC);
    auto curDevice = audioVolumeManager->audioActiveDevice_.GetDeviceForVolume(streamType);
    EXPECT_EQ(audioVolumeManager->safeVolumeManager_.DeviceIsSupportSafeVolume(curDevice), false);

    audioVolumeManager->CheckToCloseNotification(streamType, volumeLevel);
    AudioConnectedDevice::GetInstance().DelConnectedDevice(LOCAL_NETWORK_ID, DEVICE_TYPE_MAX);
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

    auto btA2dpDevice = std::make_shared<AudioDeviceDescriptor>();
    btA2dpDevice->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    btA2dpDevice->deviceCategory_ = CATEGORY_DEFAULT;
    btA2dpDevice->networkId_ = LOCAL_NETWORK_ID;

    auto ret = audioVolumeManager->safeVolumeManager_.DeviceIsSupportSafeVolume(btA2dpDevice);
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

    auto btCarDevice = std::make_shared<AudioDeviceDescriptor>();
    btCarDevice->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    btCarDevice->deviceCategory_ = BT_CAR;
    btCarDevice->networkId_ = LOCAL_NETWORK_ID;

    auto ret = audioVolumeManager->safeVolumeManager_.DeviceIsSupportSafeVolume(btCarDevice);
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

    auto btSoundboxDevice = std::make_shared<AudioDeviceDescriptor>();
    btSoundboxDevice->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    btSoundboxDevice->deviceCategory_ = BT_SOUNDBOX;
    btSoundboxDevice->networkId_ = LOCAL_NETWORK_ID;

    auto ret = audioVolumeManager->safeVolumeManager_.DeviceIsSupportSafeVolume(btSoundboxDevice);
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

    auto wiredHeadsetDevice = std::make_shared<AudioDeviceDescriptor>();
    wiredHeadsetDevice->deviceType_ = DEVICE_TYPE_WIRED_HEADSET;
    wiredHeadsetDevice->networkId_ = LOCAL_NETWORK_ID;

    auto ret = audioVolumeManager->safeVolumeManager_.DeviceIsSupportSafeVolume(wiredHeadsetDevice);
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

    auto btA2dpDevice = std::make_shared<AudioDeviceDescriptor>();
    btA2dpDevice->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    btA2dpDevice->networkId_ = LOCAL_NETWORK_ID;
    AudioConnectedDevice::GetInstance().AddConnectedDevice(btA2dpDevice);

    AudioStreamType streamType = STREAM_MUSIC;
    int32_t volumeLevel = 0;
    EXPECT_EQ(VolumeUtils::GetVolumeTypeFromStreamType(streamType), STREAM_MUSIC);

    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = nullptr;
    auto ret = audioVolumeManager->SetSystemVolumeLevel(streamType, volumeLevel, deviceDesc);
    EXPECT_EQ(ret, SUCCESS);
    AudioConnectedDevice::GetInstance().DelConnectedDevice(LOCAL_NETWORK_ID, DEVICE_TYPE_BLUETOOTH_A2DP);
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

    auto btA2dpDevice = std::make_shared<AudioDeviceDescriptor>();
    btA2dpDevice->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    btA2dpDevice->networkId_ = LOCAL_NETWORK_ID;
    AudioConnectedDevice::GetInstance().AddConnectedDevice(btA2dpDevice);

    AudioStreamType streamType = STREAM_ALL;
    int32_t volumeLevel = 0;
    EXPECT_EQ(VolumeUtils::GetVolumeTypeFromStreamType(streamType), STREAM_ALL);

    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = nullptr;
    auto ret = audioVolumeManager->SetSystemVolumeLevel(streamType, volumeLevel, deviceDesc);
    EXPECT_EQ(ret, SUCCESS);
    AudioConnectedDevice::GetInstance().DelConnectedDevice(LOCAL_NETWORK_ID, DEVICE_TYPE_BLUETOOTH_A2DP);
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
    VolumeScale volume{volumeLevel, volumeLevel};
    bool internalCall = true;

    EXPECT_NE(audioVolumeManager->audioA2dpDevice_.SetA2dpDeviceVolume(macAddress, volume), true);

    auto ret = audioVolumeManager->SetA2dpDeviceVolume(macAddress, volume, internalCall);
    EXPECT_NE(ret, SUCCESS);

    AudioStreamType streamType = STREAM_MUSIC;
    int32_t volumeLevel2 = 2;
    int32_t volumeDegree2 = 8;
    volume = {volumeLevel2, volumeDegree2};
    ret = audioVolumeManager->SetSystemVolumeExternal(streamType, volume, DEVICE_TYPE_BLUETOOTH_A2DP);
    EXPECT_NE(ret, SUCCESS);
    ret = audioVolumeManager->SetSystemVolumeExternal(streamType, volume, DEVICE_TYPE_BLUETOOTH_A2DP);
    EXPECT_NE(ret, SUCCESS);

    int32_t retDegree = audioVolumeManager->GetSystemVolumeDegree(streamType);
    EXPECT_NE(volumeDegree2, retDegree);

    ret = audioVolumeManager->SetSystemVolumeExternal(streamType, volume, DEVICE_TYPE_NEARLINK);
    EXPECT_NE(ret, SUCCESS);
    ret = audioVolumeManager->SetSystemVolumeExternal(streamType, volume, DEVICE_TYPE_NEARLINK);
    EXPECT_NE(ret, SUCCESS);

    retDegree = audioVolumeManager->GetSystemVolumeDegree(streamType);
    EXPECT_NE(volumeDegree2, retDegree);
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
    VolumeScale volume{volumeLevel, volumeLevel};
    bool internalCall = false;

    EXPECT_NE(audioVolumeManager->audioA2dpDevice_.SetA2dpDeviceVolume(macAddress, volume), true);

    auto ret = audioVolumeManager->SetA2dpDeviceVolume(macAddress, volume, internalCall);
    EXPECT_NE(ret, SUCCESS);


    std::string macAddress2 = "11:22:33:44:55:77";
    int32_t volumeDegree = 10;
    volume.volumeDegree_ = volumeDegree;
    audioVolumeManager->audioA2dpDevice_.AddA2dpDevice(macAddress, {.absVolumeSupport = true});
    EXPECT_NE(audioVolumeManager->audioA2dpDevice_.SetA2dpDeviceVolume(macAddress2, volume), true);
    audioVolumeManager->audioA2dpDevice_.AddA2dpDevice(macAddress2, {});
    EXPECT_NE(audioVolumeManager->audioA2dpDevice_.SetA2dpDeviceVolume(macAddress2, volume), true);
    EXPECT_EQ(audioVolumeManager->audioA2dpDevice_.SetA2dpDeviceVolume(macAddress, volume), true);
    A2dpDeviceConfigInfo info;
    audioVolumeManager->audioA2dpDevice_.GetA2dpDeviceInfo(macAddress, info);
    EXPECT_EQ(info.volumeDegree, volumeDegree);

    audioVolumeManager->audioA2dpDevice_.DelA2dpDevice(macAddress);
    audioVolumeManager->audioA2dpDevice_.DelA2dpDevice(macAddress2);
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
    VolumeScale volume{volumeLevel, volumeLevel};
    bool internalCall = false;

    EXPECT_NE(audioVolumeManager->audioA2dpDevice_.SetA2dpDeviceVolume(macAddress, volume), true);

    auto ret = audioVolumeManager->SetA2dpDeviceVolume(macAddress, volume, internalCall);
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

/**
 * @tc.name  : AudioVolumeManager_DoLoopCheck_001
 * @tc.number: DoLoopCheck_001
 * @tc.desc  : Test DoLoopCheck() with async handler
 */
HWTEST_F(AudioVolumeManagerUnitTest, DoLoopCheck_001, TestSize.Level4)
{
    auto testVolumeManager = std::make_shared<AudioVolumeManager>();
    auto asyncHandler = std::make_shared<AsyncActionHandler>("OS_TestHandler");
    testVolumeManager->SetAsyncActionHandler(asyncHandler);

    std::string testReason = "Default";
    std::string outReason = testVolumeManager->DoLoopCheck(testReason);
    EXPECT_EQ("Default", outReason);

    testVolumeManager->DeInit();
}

/**
 * @tc.name  : AudioVolumeManager_DoLoopCheck_002
 * @tc.number: DoLoopCheck_002
 * @tc.desc  : Test DoLoopCheck() without async handler
 */
HWTEST_F(AudioVolumeManagerUnitTest, DoLoopCheck_002, TestSize.Level4)
{
    auto testVolumeManager = std::make_shared<AudioVolumeManager>();

    std::string testReason = "Default";
    std::string outReason = testVolumeManager->DoLoopCheck(testReason);
    EXPECT_EQ("Default", outReason);

    testVolumeManager->DeInit();
}

/**
 * @tc.name  : AudioVolumeManager_OnCheckActiveMusicTime_001
 * @tc.number: OnCheckActiveMusicTime_001
 * @tc.desc  : Test OnCheckActiveMusicTime() with async handler
 */
HWTEST_F(AudioVolumeManagerUnitTest, OnCheckActiveMusicTime_001, TestSize.Level4)
{
    auto testVolumeManager = std::make_shared<AudioVolumeManager>();
    auto asyncHandler = std::make_shared<AsyncActionHandler>("OS_TestHandler");
    testVolumeManager->SetAsyncActionHandler(asyncHandler);

    std::string testReason = "Default";
    testVolumeManager->OnCheckActiveMusicTime(testReason);
    EXPECT_EQ(SAFE_UNKNOWN, testVolumeManager->safeStatus_);

    testVolumeManager->DeInit();
}

/**
 * @tc.name  : AudioVolumeManager_OnCheckActiveMusicTime_002
 * @tc.number: OnCheckActiveMusicTime_002
 * @tc.desc  : Test OnCheckActiveMusicTime() without async handler
 */
HWTEST_F(AudioVolumeManagerUnitTest, OnCheckActiveMusicTime_002, TestSize.Level4)
{
    auto testVolumeManager = std::make_shared<AudioVolumeManager>();

    std::string testReason = "Default";
    testVolumeManager->OnCheckActiveMusicTime(testReason);
    EXPECT_EQ(SAFE_UNKNOWN, testVolumeManager->safeStatus_);

    testVolumeManager->DeInit();
}

/**
 * @tc.name  : AudioVolumeManager_SendLoudVolumeMode_002
 * @tc.number: OnCheckActiveMusicTime_002
 * @tc.desc  : Test SendLoudVolumeMode() without async handler
 */
#ifdef FEATURE_MULTIMODALINPUT_INPUT
HWTEST_F(AudioVolumeManagerUnitTest, SendLoudVolumeMode_001, TestSize.Level4)
{
    auto testVolumeManager = std::make_shared<AudioVolumeManager>();
    EXPECT_NE(testVolumeManager, nullptr);

    LoudVolumeHoldType funcHoldType = LOUD_VOLUME_MODE_MUSIC;
    bool state = true;
    bool repeatTrigNotif = true;
    testVolumeManager->loudVolumeManager_->SendLoudVolumeMode(funcHoldType, state, repeatTrigNotif);

    state = false;
    testVolumeManager->loudVolumeManager_->SendLoudVolumeMode(funcHoldType, state, repeatTrigNotif);

    repeatTrigNotif = false;
    testVolumeManager->loudVolumeManager_->SendLoudVolumeMode(funcHoldType, state, repeatTrigNotif);

    state = true;
    testVolumeManager->loudVolumeManager_->SendLoudVolumeMode(funcHoldType, state, repeatTrigNotif);

    std::string testReason = "Default";
    std::string outReason = testVolumeManager->DoLoopCheck(testReason);
    EXPECT_EQ("Default", outReason);

    testVolumeManager->DeInit();
}
#endif

/**
 * @tc.name  : SetVolumeProtectionInfo_001
 * @tc.number: SetVolumeProtectionInfo_001
 * @tc.desc  : Accept a valid volume-protection configuration and replace the stored state.
 */
HWTEST_F(AudioVolumeManagerUnitTest, SetVolumeProtectionInfo_001, TestSize.Level1)
{
    auto volumeManager = std::make_shared<AudioVolumeManager>();
    VolumeProtectionConfig config;
    config.rules = {{STREAM_MUSIC, 60}, {STREAM_RING, 80}};
    config.timeDuration = 24 * 60 * 60;

    const int64_t bootTimeNs = ClockTime::GetBootNano();
    ASSERT_GE(bootTimeNs, 0);
    const int64_t currentTime = bootTimeNs / static_cast<int64_t>(AUDIO_NS_PER_SECOND);
    EXPECT_EQ(volumeManager->SetVolumeProtectionInfo(config), SUCCESS);
    EXPECT_EQ(volumeManager->volumeProtectionConfig_.rules.at(STREAM_MUSIC), 60);
    EXPECT_EQ(volumeManager->volumeProtectionConfig_.rules.at(STREAM_RING), 80);
    EXPECT_EQ(volumeManager->volumeProtectionConfig_.timeDuration, 24 * 60 * 60);
    EXPECT_GE(volumeManager->volumeLimitExpiryTime_, currentTime);
    EXPECT_LE(volumeManager->volumeLimitExpiryTime_,
        ClockTime::GetBootNano() / static_cast<int64_t>(AUDIO_NS_PER_SECOND));
    const int32_t currentVolume = volumeManager->GetSystemVolumeDegreeInterface(STREAM_MUSIC, DEFAULT_ZONE_ID);
    EXPECT_EQ(volumeManager->CheckVolumeLimitExceeded(STREAM_MUSIC, 1000, 70, DEFAULT_ZONE_ID), 70 > currentVolume);

    volumeManager->volumeLimitExpiryTime_ += config.timeDuration;
    EXPECT_FALSE(volumeManager->CheckVolumeLimitExceeded(STREAM_MUSIC, 1000, 70, DEFAULT_ZONE_ID));
    EXPECT_EQ(volumeManager->SetVolumeProtectionInfo(config), SUCCESS);
    EXPECT_GE(volumeManager->volumeLimitExpiryTime_, currentTime);
    EXPECT_LE(volumeManager->volumeLimitExpiryTime_,
        ClockTime::GetBootNano() / static_cast<int64_t>(AUDIO_NS_PER_SECOND));
    EXPECT_EQ(volumeManager->CheckVolumeLimitExceeded(STREAM_MUSIC, 1000, 70, DEFAULT_ZONE_ID), 70 > currentVolume);
}

/**
 * @tc.name  : SetVolumeProtectionInfo_002
 * @tc.number: SetVolumeProtectionInfo_002
 * @tc.desc  : Reject invalid inputs without changing an accepted configuration.
 */
HWTEST_F(AudioVolumeManagerUnitTest, SetVolumeProtectionInfo_002, TestSize.Level1)
{
    auto volumeManager = std::make_shared<AudioVolumeManager>();
    VolumeProtectionConfig config;
    config.rules = {{STREAM_MUSIC, 60}};
    config.timeDuration = 60;
    ASSERT_EQ(volumeManager->SetVolumeProtectionInfo(config), SUCCESS);

    config.rules.clear();
    EXPECT_EQ(volumeManager->SetVolumeProtectionInfo(config), ERR_INVALID_PARAM);

    config.rules = {{STREAM_MUSIC, 101}};
    EXPECT_EQ(volumeManager->SetVolumeProtectionInfo(config), ERR_INVALID_PARAM);
    EXPECT_EQ(volumeManager->volumeProtectionConfig_.rules.at(STREAM_MUSIC), 60);

    config.rules = {{static_cast<AudioVolumeType>(1000), 50}};
    EXPECT_EQ(volumeManager->SetVolumeProtectionInfo(config), ERR_INVALID_PARAM);

    config.rules = {{STREAM_MUSIC, 50}};
    config.timeDuration = 0;
    EXPECT_EQ(volumeManager->SetVolumeProtectionInfo(config), ERR_INVALID_PARAM);
}

/**
 * @tc.name  : ClearVolumeProtection_001
 * @tc.number: ClearVolumeProtection_001
 * @tc.desc  : Expire notification suppression, retain the rules, and allow repeated calls.
 */
HWTEST_F(AudioVolumeManagerUnitTest, ClearVolumeProtection_001, TestSize.Level1)
{
    auto volumeManager = std::make_shared<AudioVolumeManager>();
    VolumeProtectionConfig config;
    config.rules = {{STREAM_MUSIC, 60}};
    config.timeDuration = 60;
    ASSERT_EQ(volumeManager->SetVolumeProtectionInfo(config), SUCCESS);
    volumeManager->volumeLimitExpiryTime_ += config.timeDuration;
    const int32_t currentVolume = volumeManager->GetSystemVolumeDegreeInterface(STREAM_MUSIC, DEFAULT_ZONE_ID);
    EXPECT_FALSE(volumeManager->CheckVolumeLimitExceeded(STREAM_MUSIC, 1000, 70, DEFAULT_ZONE_ID));

    const int64_t bootTimeNs = ClockTime::GetBootNano();
    ASSERT_GE(bootTimeNs, 0);
    const int64_t currentTime = bootTimeNs / static_cast<int64_t>(AUDIO_NS_PER_SECOND);
    EXPECT_EQ(volumeManager->ClearVolumeProtection(), SUCCESS);
    EXPECT_GE(volumeManager->volumeLimitExpiryTime_, currentTime);
    EXPECT_LE(volumeManager->volumeLimitExpiryTime_,
        ClockTime::GetBootNano() / static_cast<int64_t>(AUDIO_NS_PER_SECOND));
    EXPECT_EQ(volumeManager->ClearVolumeProtection(), SUCCESS);
    EXPECT_GE(volumeManager->volumeLimitExpiryTime_, currentTime);
    EXPECT_LE(volumeManager->volumeLimitExpiryTime_,
        ClockTime::GetBootNano() / static_cast<int64_t>(AUDIO_NS_PER_SECOND));
    EXPECT_EQ(volumeManager->volumeProtectionConfig_.rules.at(STREAM_MUSIC), 60);
    EXPECT_EQ(volumeManager->volumeProtectionConfig_.timeDuration, config.timeDuration);
    EXPECT_EQ(volumeManager->CheckVolumeLimitExceeded(STREAM_MUSIC, 1000, 70, DEFAULT_ZONE_ID), 70 > currentVolume);
}

/**
 * @tc.name  : VolumeProtectionLimitExceeded_001
 * @tc.number: VolumeProtectionLimitExceeded_001
 * @tc.desc  : Notify above the configured threshold when the suppression expiry is reached.
 */
HWTEST_F(AudioVolumeManagerUnitTest, VolumeProtectionLimitExceeded_001, TestSize.Level1)
{
    auto volumeManager = std::make_shared<AudioVolumeManager>();
    VolumeProtectionConfig config;
    config.rules = {{STREAM_MUSIC, 60}};
    config.timeDuration = 24 * 60 * 60;
    ASSERT_EQ(volumeManager->SetVolumeProtectionInfo(config), SUCCESS);

    const int32_t currentVolume = volumeManager->GetSystemVolumeDegreeInterface(STREAM_MUSIC, DEFAULT_ZONE_ID);
    EXPECT_FALSE(volumeManager->CheckVolumeLimitExceeded(STREAM_MUSIC, 1000, 60, DEFAULT_ZONE_ID));
    EXPECT_FALSE(volumeManager->CheckVolumeLimitExceeded(STREAM_RING, 1000, 70, DEFAULT_ZONE_ID));

    EXPECT_EQ(volumeManager->CheckVolumeLimitExceeded(STREAM_MUSIC, 1000, 70, DEFAULT_ZONE_ID), 70 > currentVolume);

    EXPECT_EQ(volumeManager->CheckVolumeLimitExceeded(STREAM_MUSIC, 1000, 61, DEFAULT_ZONE_ID), 61 > currentVolume);

    const int64_t bootTimeNs = ClockTime::GetBootNano();
    ASSERT_GE(bootTimeNs, 0);
    const int64_t currentTime = bootTimeNs / static_cast<int64_t>(AUDIO_NS_PER_SECOND);
    volumeManager->volumeLimitExpiryTime_ = currentTime + config.timeDuration;
    EXPECT_FALSE(volumeManager->CheckVolumeLimitExceeded(STREAM_MUSIC, 2000, 70, DEFAULT_ZONE_ID));
    volumeManager->volumeLimitExpiryTime_ = currentTime;
    EXPECT_EQ(volumeManager->CheckVolumeLimitExceeded(STREAM_MUSIC, 2000, 70, DEFAULT_ZONE_ID), 70 > currentVolume);
}

/**
 * @tc.name  : ConfirmVolumeLimitExceeded_001
 * @tc.number: ConfirmVolumeLimitExceeded_001
 * @tc.desc  : A positive confirmation sets the expiry to the current time plus the configured duration.
 */
HWTEST_F(AudioVolumeManagerUnitTest, ConfirmVolumeLimitExceeded_001, TestSize.Level1)
{
    auto volumeManager = std::make_shared<AudioVolumeManager>();
    VolumeProtectionConfig config;
    config.rules = {{STREAM_MUSIC, 60}};
    config.timeDuration = 60;
    ASSERT_EQ(volumeManager->SetVolumeProtectionInfo(config), SUCCESS);

    const int64_t initialExpiryTime = volumeManager->volumeLimitExpiryTime_;
    EXPECT_EQ(volumeManager->ConfirmVolumeLimitExceeded(STREAM_MUSIC, false), SUCCESS);
    EXPECT_EQ(volumeManager->volumeLimitExpiryTime_, initialExpiryTime);

    const int64_t bootTimeNs = ClockTime::GetBootNano();
    ASSERT_GE(bootTimeNs, 0);
    const int64_t currentTime = bootTimeNs / static_cast<int64_t>(AUDIO_NS_PER_SECOND);
    EXPECT_EQ(volumeManager->ConfirmVolumeLimitExceeded(STREAM_MUSIC, true), SUCCESS);
    EXPECT_GE(volumeManager->volumeLimitExpiryTime_, currentTime + config.timeDuration);
    EXPECT_LE(volumeManager->volumeLimitExpiryTime_,
        ClockTime::GetBootNano() / static_cast<int64_t>(AUDIO_NS_PER_SECOND) + config.timeDuration);
    EXPECT_FALSE(volumeManager->CheckVolumeLimitExceeded(STREAM_MUSIC, 1000, 70, DEFAULT_ZONE_ID));

    volumeManager->volumeLimitExpiryTime_ = 100;
    EXPECT_EQ(volumeManager->ConfirmVolumeLimitExceeded(STREAM_MUSIC, false), SUCCESS);
    EXPECT_EQ(volumeManager->volumeLimitExpiryTime_, 100);
    EXPECT_EQ(volumeManager->ConfirmVolumeLimitExceeded(static_cast<AudioVolumeType>(1000), true), ERR_INVALID_PARAM);
}

} // namespace AudioStandard
} // namespace OHOS
