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

#include "audio_device_status_unit_test.h"
#include "audio_device_info.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
void AudioDeviceStatusUnitTest::SetUpTestCase(void) {}
void AudioDeviceStatusUnitTest::TearDownTestCase(void) {}
void AudioDeviceStatusUnitTest::SetUp(void)
{
    std::shared_ptr<AudioA2dpOffloadManager> audioA2dpOffloadManager = std::make_shared<AudioA2dpOffloadManager>();
    std::shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler =
        DelayedSingleton<AudioPolicyServerHandler>::GetInstance();
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    audioA2dpOffloadManager->Init();
    audioDeviceStatus.Init(audioA2dpOffloadManager, audioPolicyServerHandler);
}

void AudioDeviceStatusUnitTest::TearDown(void)
{
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    audioDeviceStatus.DeInit();
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_001
* @tc.desc  : Test RehandlePnpDevice interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_001, TestSize.Level1)
{
    DeviceType deviceType = DEVICE_TYPE_USB_HEADSET;
    DeviceRole deviceRole = DEVICE_ROLE_NONE;
    std::string address = "00:11:22:33:44:55";
    int32_t ret = 0;
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    ret = audioDeviceStatus.RehandlePnpDevice(deviceType, deviceRole, address);
    EXPECT_NE(ret, 0);

    deviceRole = INPUT_DEVICE;
    ret = audioDeviceStatus.RehandlePnpDevice(deviceType, deviceRole, address);
    EXPECT_EQ(ret, 0);

    deviceType = DEVICE_TYPE_USB_ARM_HEADSET;
    ret = audioDeviceStatus.RehandlePnpDevice(deviceType, deviceRole, address);
    EXPECT_EQ(ret, 0);

    deviceType = DEVICE_TYPE_DP;
    ret = audioDeviceStatus.RehandlePnpDevice(deviceType, deviceRole, address);
    EXPECT_NE(ret, 0);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_002
* @tc.desc  : Test HandleArmUsbDevice interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_002, TestSize.Level1)
{
    DeviceType deviceType = DEVICE_TYPE_USB_HEADSET;
    DeviceRole deviceRole = DEVICE_ROLE_NONE;
    std::string address = "00:11:22:33:44:55";
    int32_t ret = 0;
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    audioDeviceStatus.audioActiveDevice_.SetCurrentOutputDeviceType(DEVICE_TYPE_USB_HEADSET);
    ret = audioDeviceStatus.HandleArmUsbDevice(deviceType, deviceRole, address);
    EXPECT_EQ(ret, 0);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_003
* @tc.desc  : Test NoNeedChangeUsbDevice interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_003, TestSize.Level1)
{
    std::string address = "00:11:22:33:44:55";
    bool bRet = true;
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    bRet = audioDeviceStatus.NoNeedChangeUsbDevice(address);
    EXPECT_EQ(bRet, false);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_004
* @tc.desc  : Test TriggerMicrophoneBlockedCallback interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_004, TestSize.Level1)
{
    vector<std::shared_ptr<AudioDeviceDescriptor>> desc;
    DeviceBlockStatus status = DEVICE_UNBLOCKED;
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    audioDeviceStatus.TriggerMicrophoneBlockedCallback(desc, status);
    EXPECT_NE(audioDeviceStatus.audioPolicyServerHandler_, nullptr);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_005
* @tc.desc  : Test ReloadA2dpOffloadOnDeviceChanged interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_005, TestSize.Level1)
{
    DeviceType deviceType = DEVICE_TYPE_USB_HEADSET;
    std::string macAddress = "00:11:22:33:44:55";
    std::string deviceName = "usb_headset";
    AudioStreamInfo streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    std::unique_ptr <AudioAffinityManager> audioAffinityManager = std::make_unique<AudioAffinityManager>();

    ClassType classType = TYPE_A2DP;
    AudioModuleInfo moduleInfo = {"className", "TEST", "TEST"};
    std::list<AudioModuleInfo> audioModuleListData = {};

    audioModuleListData.push_back(moduleInfo);
    audioDeviceStatus.audioConfigManager_.deviceClassInfo_[classType] = audioModuleListData;

    audioDeviceStatus.ReloadA2dpOffloadOnDeviceChanged(deviceType, macAddress, deviceName, streamInfo);
    EXPECT_EQ(audioDeviceStatus.audioIOHandleMap_.CheckIOHandleExist(moduleInfo.name), false);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_006
* @tc.desc  : Test GetDeviceTypeFromPin interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_006, TestSize.Level1)
{
    AudioPin hdiPin = AUDIO_PIN_NONE;
    DeviceType deviceType = DEVICE_TYPE_NONE;
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    deviceType = audioDeviceStatus.GetDeviceTypeFromPin(hdiPin);
    EXPECT_EQ(deviceType, DEVICE_TYPE_DEFAULT);

    hdiPin = AUDIO_PIN_OUT_SPEAKER;
    deviceType = audioDeviceStatus.GetDeviceTypeFromPin(hdiPin);
    EXPECT_EQ(deviceType, DEVICE_TYPE_SPEAKER);

    hdiPin = AUDIO_PIN_OUT_DAUDIO_DEFAULT;
    deviceType = audioDeviceStatus.GetDeviceTypeFromPin(hdiPin);
    EXPECT_EQ(deviceType, DEVICE_TYPE_SPEAKER);

    hdiPin = AUDIO_PIN_OUT_HEADSET;
    deviceType = audioDeviceStatus.GetDeviceTypeFromPin(hdiPin);
    EXPECT_EQ(deviceType, DEVICE_TYPE_DEFAULT);

    hdiPin = AUDIO_PIN_OUT_LINEOUT;
    deviceType = audioDeviceStatus.GetDeviceTypeFromPin(hdiPin);
    EXPECT_EQ(deviceType, DEVICE_TYPE_DEFAULT);

    hdiPin = AUDIO_PIN_OUT_HDMI;
    deviceType = audioDeviceStatus.GetDeviceTypeFromPin(hdiPin);
    EXPECT_EQ(deviceType, DEVICE_TYPE_DEFAULT);

    hdiPin = AUDIO_PIN_OUT_USB;
    deviceType = audioDeviceStatus.GetDeviceTypeFromPin(hdiPin);
    EXPECT_EQ(deviceType, DEVICE_TYPE_DEFAULT);

    hdiPin = AUDIO_PIN_OUT_USB_EXT;
    deviceType = audioDeviceStatus.GetDeviceTypeFromPin(hdiPin);
    EXPECT_EQ(deviceType, DEVICE_TYPE_DEFAULT);

    hdiPin = AUDIO_PIN_OUT_USB_HEADSET;
    deviceType = audioDeviceStatus.GetDeviceTypeFromPin(hdiPin);
    EXPECT_EQ(deviceType, DEVICE_TYPE_USB_ARM_HEADSET);

    hdiPin = AUDIO_PIN_IN_USB_HEADSET;
    deviceType = audioDeviceStatus.GetDeviceTypeFromPin(hdiPin);
    EXPECT_EQ(deviceType, DEVICE_TYPE_USB_ARM_HEADSET);

    hdiPin = AUDIO_PIN_IN_MIC;
    deviceType = audioDeviceStatus.GetDeviceTypeFromPin(hdiPin);
    EXPECT_EQ(deviceType, DEVICE_TYPE_MIC);

    hdiPin = AUDIO_PIN_IN_DAUDIO_DEFAULT;
    deviceType = audioDeviceStatus.GetDeviceTypeFromPin(hdiPin);
    EXPECT_EQ(deviceType, DEVICE_TYPE_MIC);

    hdiPin = AUDIO_PIN_IN_HS_MIC;
    deviceType = audioDeviceStatus.GetDeviceTypeFromPin(hdiPin);
    EXPECT_EQ(deviceType, DEVICE_TYPE_DEFAULT);

    hdiPin = AUDIO_PIN_IN_LINEIN;
    deviceType = audioDeviceStatus.GetDeviceTypeFromPin(hdiPin);
    EXPECT_EQ(deviceType, DEVICE_TYPE_DEFAULT);

    hdiPin = AUDIO_PIN_IN_USB_EXT;
    deviceType = audioDeviceStatus.GetDeviceTypeFromPin(hdiPin);
    EXPECT_EQ(deviceType, DEVICE_TYPE_DEFAULT);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_007
* @tc.desc  : Test OnDeviceStatusUpdated interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_007, TestSize.Level1)
{
    DeviceType devType = DEVICE_TYPE_INVALID;
    bool isConnected = true;
    std::string macAddress = "00:11:22:33:44:55";
    std::string deviceName = "testDevice";
    AudioStreamInfo streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    DeviceRole role = DEVICE_ROLE_NONE;
    bool hasPair = false;
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    audioDeviceStatus.OnDeviceStatusUpdated(devType, isConnected, macAddress, deviceName, streamInfo, role, hasPair);
    EXPECT_NE(audioDeviceStatus.audioA2dpOffloadManager_, nullptr);

    isConnected = false;
    audioDeviceStatus.OnDeviceStatusUpdated(devType, isConnected, macAddress, deviceName, streamInfo, role, hasPair);
    EXPECT_NE(audioDeviceStatus.audioA2dpOffloadManager_, nullptr);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_008
* @tc.desc  : Test HandleDistributedDeviceUpdate interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_008, TestSize.Level1)
{
    DStatusInfo statusInfo;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descForCb;
    int32_t ret;
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    statusInfo.hdiPin = AUDIO_PIN_NONE;
    statusInfo.deviceName = "test";
    statusInfo.macAddress = "00:11:22:33:44:55";
    statusInfo.isConnected = true;
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    ret = audioDeviceStatus.HandleDistributedDeviceUpdate(statusInfo, descForCb, reason);
    EXPECT_EQ(ret, SUCCESS);

    statusInfo.isConnected = false;
    reason = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    ret = audioDeviceStatus.HandleDistributedDeviceUpdate(statusInfo, descForCb, reason);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_009
* @tc.desc  : Test UpdateDeviceList interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_009, TestSize.Level1)
{
    AudioDeviceDescriptor updatedDesc;
    bool isConnected = true;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descForCb;
    AudioStreamDeviceChangeReasonExt::ExtEnum oldDevice =
        AudioStreamDeviceChangeReasonExt::ExtEnum::OLD_DEVICE_UNAVALIABLE;
    AudioStreamDeviceChangeReasonExt reason(oldDevice);
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    updatedDesc.deviceType_ = DEVICE_TYPE_INVALID;
    updatedDesc.macAddress_ = "00:11:22:33:44:55";
    updatedDesc.deviceRole_ = DEVICE_ROLE_NONE;
    audioDeviceStatus.UpdateDeviceList(updatedDesc, isConnected, descForCb, reason);
    EXPECT_EQ(reason, AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE);

    isConnected = false;
    audioDeviceStatus.UpdateDeviceList(updatedDesc, isConnected, descForCb, reason);
    EXPECT_EQ(reason, AudioStreamDeviceChangeReason::OLD_DEVICE_UNAVALIABLE);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_010
* @tc.desc  : Test OnPreferredStateUpdated interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_010, TestSize.Level1)
{
    AudioDeviceDescriptor desc;
    DeviceInfoUpdateCommand updateCommand = CATEGORY_UPDATE;
    AudioStreamDeviceChangeReasonExt::ExtEnum oldDevice =
        AudioStreamDeviceChangeReasonExt::ExtEnum::OLD_DEVICE_UNAVALIABLE;
    AudioStreamDeviceChangeReasonExt reason(oldDevice);
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    desc.deviceCategory_ = BT_UNWEAR_HEADPHONE;
    audioDeviceStatus.OnPreferredStateUpdated(desc, updateCommand, reason);
    EXPECT_EQ(reason, AudioStreamDeviceChangeReason::OLD_DEVICE_UNAVALIABLE);

    desc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    desc.deviceCategory_ = CATEGORY_DEFAULT;
    audioDeviceStatus.OnPreferredStateUpdated(desc, updateCommand, reason);
    EXPECT_EQ(reason, AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE);

    desc.deviceType_ = DEVICE_TYPE_INVALID;
    desc.deviceCategory_ = CATEGORY_DEFAULT;
    audioDeviceStatus.OnPreferredStateUpdated(desc, updateCommand, reason);
    EXPECT_EQ(reason, AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE);

    updateCommand = ENABLE_UPDATE;
    desc.isEnable_ = false;
    desc.deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    audioDeviceStatus.OnPreferredStateUpdated(desc, updateCommand, reason);
    EXPECT_EQ(reason, AudioStreamDeviceChangeReason::OLD_DEVICE_UNAVALIABLE);
}
} // namespace AudioStandard
} // namespace OHOS
