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

    AudioDeviceDescriptor deviceDescriptor;
    deviceDescriptor.deviceType_ = DEVICE_TYPE_USB_HEADSET;
    audioDeviceStatus.audioActiveDevice_.SetCurrentOutputDevice(deviceDescriptor);
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
    EXPECT_EQ(bRet, true);
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
* @tc.number: GetModuleNameByType_001
* @tc.desc  : Test GetModuleNameByType interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, GetModuleNameByType_001, TestSize.Level1)
{
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    unordered_map<ClassType, list<AudioModuleInfo>> deviceClassInfo;
    AudioPolicyConfigManager::GetInstance().GetDeviceClassInfo(deviceClassInfo);
    if (!deviceClassInfo.empty()) {
        auto type = deviceClassInfo.cbegin()->first;
        auto moduleName = audioDeviceStatus.GetModuleNameByType(type);
        list<AudioModuleInfo> &moduleList = deviceClassInfo[type];
        if (!moduleList.empty()) {
            EXPECT_EQ(moduleName, moduleList.front().name);
        }
    }
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

    devType = DEVICE_TYPE_USB_HEADSET;
    role = OUTPUT_DEVICE;
    isConnected = true;
    audioDeviceStatus.OnDeviceStatusUpdated(devType, isConnected, macAddress, deviceName, streamInfo, role, hasPair);
    EXPECT_NE(audioDeviceStatus.audioA2dpOffloadManager_, nullptr);

    isConnected = false;
    audioDeviceStatus.OnDeviceStatusUpdated(devType, isConnected, macAddress, deviceName, streamInfo, role, hasPair);
    EXPECT_NE(audioDeviceStatus.audioA2dpOffloadManager_, nullptr);

    devType = DEVICE_TYPE_BLUETOOTH_A2DP;
    AudioDeviceDescriptor deviceDesc(devType, AudioPolicyUtils::GetInstance().GetDeviceRole(devType));
    bool isActualConnected = true;
    audioDeviceStatus.OnDeviceStatusUpdated(deviceDesc, devType, macAddress, deviceName, isActualConnected,
        streamInfo, isConnected);
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

    DeviceStreamInfo deviceStreamInfo = {};
    std::list<DeviceStreamInfo> streamInfoList = { deviceStreamInfo };
    statusInfo.streamInfo = streamInfoList;
    std::string testNetworkId = "testNetworkId_0";
    auto res = strncpy_s(statusInfo.networkId, NETWORK_ID_SIZE, testNetworkId.c_str(), testNetworkId.size());
    EXPECT_EQ(res, 0);
    DeviceType devType = audioDeviceStatus.GetDeviceTypeFromPin(statusInfo.hdiPin);
    std::shared_ptr<AudioDeviceDescriptor> audioDescriptor =
        std::make_shared<AudioDeviceDescriptor>(devType, OUTPUT_DEVICE);
    audioDescriptor->networkId_ = statusInfo.networkId;
    audioDeviceStatus.audioConnectedDevice_.AddConnectedDevice(audioDescriptor);

    statusInfo.isConnected = true;
    reason = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    ret = audioDeviceStatus.HandleDistributedDeviceUpdate(statusInfo, descForCb, reason);
    audioDeviceStatus.audioConnectedDevice_.DelConnectedDevice(audioDescriptor->networkId_,
        audioDescriptor->deviceType_);
    EXPECT_EQ(ret, SUCCESS_BUT_NOT_CONTINUE);
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

    desc.deviceType_ = DEVICE_TYPE_NEARLINK;
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

    desc.deviceType_ = DEVICE_TYPE_NEARLINK;
    auto preferredDeviceDesc = std::make_shared<AudioDeviceDescriptor>(desc);
    audioDeviceStatus.audioStateManager_.SetPreferredMediaRenderDevice(preferredDeviceDesc);
    updateCommand = USAGE_UPDATE;
    desc.deviceUsage_ = VOICE;
    audioDeviceStatus.OnPreferredStateUpdated(desc, updateCommand, reason);
    auto targetDevice = audioDeviceStatus.audioStateManager_.GetPreferredMediaRenderDevice();
    bool result = targetDevice->deviceType_ == DEVICE_TYPE_NEARLINK && targetDevice->deviceUsage_ == VOICE;
    audioDeviceStatus.audioStateManager_.SetPreferredMediaRenderDevice(std::make_shared<AudioDeviceDescriptor>());
    EXPECT_EQ(true, result);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_011
* @tc.desc  : Test HandleDistributedDeviceUpdate interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_011, TestSize.Level1)
{
    DeviceType devType = DEVICE_TYPE_USB_HEADSET;
    DeviceBlockStatus status = DEVICE_BLOCKED;
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioDeviceStatus.OnMicrophoneBlockedUpdate(devType, status);

    std::vector<shared_ptr<AudioDeviceDescriptor>> descForCb = {};
    shared_ptr<AudioDeviceDescriptor> audioDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioDescriptor->deviceType_ = devType;
    audioDescriptor->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    descForCb.push_back(audioDescriptor);
    audioDeviceStatus.TriggerMicrophoneBlockedCallback(descForCb, status);
    EXPECT_NE(audioDeviceStatus.audioA2dpOffloadManager_, nullptr);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_012
* @tc.desc  : Test HandleDistributedDeviceUpdate interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_012, TestSize.Level1)
{
    DeviceType devType = DEVICE_TYPE_BLUETOOTH_A2DP;
    std::string macAddress = "00:11:22:33:44:55";
    std::string deviceName = "testDevice";
    AudioStreamInfo streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioDeviceStatus.ReloadA2dpOffloadOnDeviceChanged(devType, macAddress, deviceName, streamInfo);
    EXPECT_NE(audioDeviceStatus.audioA2dpOffloadManager_, nullptr);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_013
* @tc.desc  : Test HandleDistributedDeviceUpdate interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_013, TestSize.Level1)
{
    DStatusInfo statusInfo;
    statusInfo.hdiPin = AUDIO_PIN_IN_DAUDIO_DEFAULT;
    statusInfo.macAddress = "00:11:22:33:44:55";
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioDeviceStatus.OnDeviceStatusUpdated(statusInfo, true);
    audioDeviceStatus.OnDeviceStatusUpdated(statusInfo, false);
    EXPECT_NE(audioDeviceStatus.audioA2dpOffloadManager_, nullptr);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_014
* @tc.desc  : Test HandleDistributedDeviceUpdate interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_014, TestSize.Level1)
{
    AudioDeviceDescriptor desc;
    desc.deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioDeviceStatus.CheckAndActiveHfpDevice(desc);
    EXPECT_NE(audioDeviceStatus.audioA2dpOffloadManager_, nullptr);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_015
* @tc.desc  : Test WriteHeadsetSysEvents interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_015, TestSize.Level1)
{
    shared_ptr<AudioDeviceDescriptor> desc = nullptr;
    bool isConnected = false;
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    audioDeviceStatus.WriteHeadsetSysEvents(desc, isConnected);
    EXPECT_NE(audioDeviceStatus.audioA2dpOffloadManager_, nullptr);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_016
* @tc.desc  : Test WriteHeadsetSysEvents interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_016, TestSize.Level1)
{
    shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    desc->deviceType_ = DEVICE_TYPE_WIRED_HEADSET;
    bool isConnected = true;

    audioDeviceStatus.WriteHeadsetSysEvents(desc, isConnected);
    EXPECT_NE(audioDeviceStatus.audioA2dpOffloadManager_, nullptr);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_017
* @tc.desc  : Test WriteHeadsetSysEvents interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_017, TestSize.Level1)
{
    shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    desc->deviceType_ = DEVICE_TYPE_USB_HEADSET;
    bool isConnected = false;

    audioDeviceStatus.WriteHeadsetSysEvents(desc, isConnected);
    EXPECT_NE(audioDeviceStatus.audioA2dpOffloadManager_, nullptr);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_018
* @tc.desc  : Test WriteHeadsetSysEvents interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_018, TestSize.Level1)
{
    shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    desc->deviceType_ = DEVICE_TYPE_WIRED_HEADPHONES;
    bool isConnected = true;

    audioDeviceStatus.WriteHeadsetSysEvents(desc, isConnected);
    EXPECT_NE(audioDeviceStatus.audioA2dpOffloadManager_, nullptr);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_019
* @tc.desc  : Test WriteAllDeviceSysEvents interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_019, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorSptr = std::make_shared<AudioDeviceDescriptor>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptorSptrVector;
    audioDeviceDescriptorSptrVector.push_back(audioDeviceDescriptorSptr);

    bool isConnected = false;

    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    audioDeviceStatus.WriteAllDeviceSysEvents(audioDeviceDescriptorSptrVector, isConnected);
    EXPECT_NE(audioDeviceStatus.audioA2dpOffloadManager_, nullptr);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_020
* @tc.desc  : Test WriteAllDeviceSysEvents interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_020, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorSptr = std::make_shared<AudioDeviceDescriptor>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptorSptrVector;
    audioDeviceDescriptorSptrVector.push_back(audioDeviceDescriptorSptr);

    bool isConnected = true;

    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    audioDeviceStatus.WriteAllDeviceSysEvents(audioDeviceDescriptorSptrVector, isConnected);
    EXPECT_NE(audioDeviceStatus.audioA2dpOffloadManager_, nullptr);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_021
* @tc.desc  : Test TriggerAvailableDeviceChangedCallback interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_021, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorSptr = std::make_shared<AudioDeviceDescriptor>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptorSptrVector;
    audioDeviceDescriptorSptrVector.push_back(audioDeviceDescriptorSptr);

    bool isConnected = true;

    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    audioDeviceStatus.DeInit();
    audioDeviceStatus.TriggerAvailableDeviceChangedCallback(audioDeviceDescriptorSptrVector, isConnected);
    EXPECT_EQ(audioDeviceStatus.audioPolicyServerHandler_, nullptr);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_022
* @tc.desc  : Test TriggerAvailableDeviceChangedCallback interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_022, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorSptr = std::make_shared<AudioDeviceDescriptor>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptorSptrVector;
    audioDeviceDescriptorSptrVector.push_back(audioDeviceDescriptorSptr);

    bool isConnected = true;

    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    audioDeviceStatus.TriggerAvailableDeviceChangedCallback(audioDeviceDescriptorSptrVector, isConnected);
    EXPECT_NE(audioDeviceStatus.audioPolicyServerHandler_, nullptr);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_023
* @tc.desc  : Test TriggerDeviceChangedCallback interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_023, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorSptr = std::make_shared<AudioDeviceDescriptor>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptorSptrVector;
    audioDeviceDescriptorSptrVector.push_back(audioDeviceDescriptorSptr);

    bool isConnected = true;

    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    audioDeviceStatus.DeInit();
    audioDeviceStatus.TriggerDeviceChangedCallback(audioDeviceDescriptorSptrVector, isConnected);
    EXPECT_EQ(audioDeviceStatus.audioPolicyServerHandler_, nullptr);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_024
* @tc.desc  : Test TriggerDeviceChangedCallback interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_024, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorSptr = std::make_shared<AudioDeviceDescriptor>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptorSptrVector;
    audioDeviceDescriptorSptrVector.push_back(audioDeviceDescriptorSptr);

    bool isConnected = true;

    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    audioDeviceStatus.TriggerDeviceChangedCallback(audioDeviceDescriptorSptrVector, isConnected);
    EXPECT_NE(audioDeviceStatus.audioPolicyServerHandler_, nullptr);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_025
* @tc.desc  : Test HandleDpDevice interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_025, TestSize.Level1)
{
    DeviceType deviceType = DEVICE_TYPE_NONE;
    std::string address = "00:11:22:33:44:55";
    int32_t ret = 0;
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    ret = audioDeviceStatus.HandleDpDevice(deviceType, address);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: HandleLocalDeviceConnected_001
* @tc.desc  : Test HandleLocalDeviceConnected virtual connected DEVICE_TYPE_BLUETOOTH_A2DP.
*/
HWTEST_F(AudioDeviceStatusUnitTest, HandleLocalDeviceConnected_001, TestSize.Level1)
{
    AudioDeviceStatus &audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioDeviceStatus.audioDeviceManager_.connectedDevices_.clear();

    AudioDeviceDescriptor desc;
    desc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    desc.macAddress_ = "00:11:22:33:44:55";
    desc.connectState_ = VIRTUAL_CONNECTED;

    int32_t result = audioDeviceStatus.HandleLocalDeviceConnected(desc);
    audioDeviceStatus.audioDeviceManager_.connectedDevices_.clear();

    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: HandleLocalDeviceConnected_002
* @tc.desc  : Test HandleLocalDeviceConnected first connected DEVICE_TYPE_BLUETOOTH_A2DP.
*/
HWTEST_F(AudioDeviceStatusUnitTest, HandleLocalDeviceConnected_002, TestSize.Level1)
{
    AudioDeviceStatus &audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioDeviceStatus.audioDeviceManager_.connectedDevices_.clear();

    AudioDeviceDescriptor desc;
    desc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    desc.macAddress_ = "00:11:22:33:44:55";
    desc.connectState_ = CONNECTED;

    int32_t result = audioDeviceStatus.HandleLocalDeviceConnected(desc);
    audioDeviceStatus.audioDeviceManager_.connectedDevices_.clear();

    EXPECT_NE(result, SUCCESS);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: HandleLocalDeviceConnected_003
* @tc.desc  : Test HandleLocalDeviceConnected first connected DEVICE_TYPE_BLUETOOTH_A2DP.
*/
HWTEST_F(AudioDeviceStatusUnitTest, HandleLocalDeviceConnected_003, TestSize.Level1)
{
    AudioDeviceStatus &audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioDeviceStatus.audioDeviceManager_.connectedDevices_.clear();

    AudioDeviceDescriptor descA;
    descA.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    descA.macAddress_ = "00:11:22:33:44:55";
    descA.connectState_ = VIRTUAL_CONNECTED;
    audioDeviceStatus.audioDeviceManager_.connectedDevices_.push_back(
        std::make_shared<AudioDeviceDescriptor>(descA));

    AudioDeviceDescriptor descB;
    descB.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    descB.macAddress_ = "11:22:33:44:55:66";
    descB.connectState_ = CONNECTED;

    int32_t result = audioDeviceStatus.HandleLocalDeviceConnected(descB);
    audioDeviceStatus.audioDeviceManager_.connectedDevices_.clear();

    EXPECT_NE(result, SUCCESS);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: HandleLocalDeviceConnected_004
* @tc.desc  : Test HandleLocalDeviceConnected second connected DEVICE_TYPE_BLUETOOTH_A2DP.
*/
HWTEST_F(AudioDeviceStatusUnitTest, HandleLocalDeviceConnected_004, TestSize.Level1)
{
    AudioDeviceStatus &audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioDeviceStatus.audioDeviceManager_.connectedDevices_.clear();

    AudioDeviceDescriptor descA;
    descA.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    descA.macAddress_ = "00:11:22:33:44:55";
    descA.connectState_ = CONNECTED;
    audioDeviceStatus.audioDeviceManager_.connectedDevices_.push_back(
        std::make_shared<AudioDeviceDescriptor>(descA));

    AudioDeviceDescriptor descB;
    descB.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    descB.macAddress_ = "11:22:33:44:55:66";
    descB.connectState_ = CONNECTED;

    int32_t result = audioDeviceStatus.HandleLocalDeviceConnected(descB);
    audioDeviceStatus.audioDeviceManager_.connectedDevices_.clear();

    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_027
* @tc.desc : Test HandleLocalDeviceConnected interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_027, TestSize.Level1)
{
    AudioDeviceDescriptor updatedDesc;
    int32_t result;

    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    updatedDesc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP_IN;

    result = audioDeviceStatus.HandleLocalDeviceConnected(updatedDesc);

    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_028
* @tc.desc : Test HandleLocalDeviceConnected interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_028, TestSize.Level1)
{
    AudioDeviceDescriptor updatedDesc;
    int32_t result;

    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    updatedDesc.deviceType_ = DEVICE_TYPE_DP;

    result = audioDeviceStatus.HandleLocalDeviceConnected(updatedDesc);

    EXPECT_EQ(result, ERROR);
}

/**
* @tc.name : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_029
* @tc.desc : Test HandleLocalDeviceConnected interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_029, TestSize.Level1)
{
    AudioDeviceDescriptor updatedDesc;
    int32_t result;

    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    updatedDesc.deviceType_ = DEVICE_TYPE_DP;

    result = audioDeviceStatus.HandleLocalDeviceConnected(updatedDesc);

    EXPECT_EQ(result, ERROR);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_070
* @tc.desc  : Test HandleAccessoryDevice interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_070, TestSize.Level1)
{
    DeviceType deviceType = DEVICE_TYPE_NONE;
    std::string address = "00:11:22:33:44:55";
    int32_t ret = 0;
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    ret = audioDeviceStatus.HandleAccessoryDevice(deviceType, address);
    EXPECT_NE(ret, SUCCESS);
}

/**
* @tc.name : Test AudioDeviceStatus.
* @tc.number: HandleLocalDeviceConnected_005
* @tc.desc : Test HandleLocalDeviceConnected interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, HandleLocalDeviceConnected_005, TestSize.Level1)
{
    AudioDeviceDescriptor updatedDesc;
    int32_t result;

    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    updatedDesc.deviceType_ = DEVICE_TYPE_NEARLINK;

    result = audioDeviceStatus.HandleLocalDeviceConnected(updatedDesc);

    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_030
* @tc.desc : Test HandleLocalDeviceConnected interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_030, TestSize.Level1)
{
    AudioDeviceDescriptor updatedDesc;
    int32_t result;

    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    updatedDesc.deviceType_ = DEVICE_TYPE_USB_HEADSET;

    result = audioDeviceStatus.HandleLocalDeviceConnected(updatedDesc);

    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_031
* @tc.desc : Test HandleLocalDeviceConnected interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_031, TestSize.Level1)
{
    AudioDeviceDescriptor updatedDesc;
    int32_t result;

    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    updatedDesc.deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;

    result = audioDeviceStatus.HandleLocalDeviceConnected(updatedDesc);

    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_072
* @tc.desc : Test HandleLocalDeviceConnected interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_072, TestSize.Level1)
{
    AudioDeviceDescriptor updatedDesc;
    int32_t result;

    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    updatedDesc.deviceType_ = DEVICE_TYPE_BT_SPP;

    result = audioDeviceStatus.HandleLocalDeviceConnected(updatedDesc);

    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_032
* @tc.desc : Test HandleLocalDeviceDisconnected interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_032, TestSize.Level1)
{
    int32_t result;
    AudioDeviceDescriptor updatedDesc;
    updatedDesc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;

    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    result = audioDeviceStatus.HandleLocalDeviceDisconnected(updatedDesc);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_033
* @tc.desc : Test HandleLocalDeviceDisconnected interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_033, TestSize.Level1)
{
    int32_t result;
    AudioDeviceDescriptor updatedDesc;
    updatedDesc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP_IN;
    updatedDesc.macAddress_ = "00:11:22:33:44:55";

    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    result = audioDeviceStatus.HandleLocalDeviceDisconnected(updatedDesc);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_034
* @tc.desc : Test HandleLocalDeviceDisconnected interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_034, TestSize.Level1)
{
    int32_t result;
    AudioDeviceDescriptor updatedDesc;
    updatedDesc.deviceType_ = DEVICE_TYPE_DP;

    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    result = audioDeviceStatus.HandleLocalDeviceDisconnected(updatedDesc);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_035
* @tc.desc : Test HandleLocalDeviceDisconnected interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_035, TestSize.Level1)
{
    int32_t result;
    AudioDeviceDescriptor updatedDesc;
    updatedDesc.deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;

    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    result = audioDeviceStatus.HandleLocalDeviceDisconnected(updatedDesc);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_036
* @tc.desc : Test HandleLocalDeviceDisconnected interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_036, TestSize.Level1)
{
    int32_t result;
    AudioDeviceDescriptor updatedDesc;
    updatedDesc.deviceType_ = DEVICE_TYPE_USB_HEADSET;

    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    result = audioDeviceStatus.HandleLocalDeviceDisconnected(updatedDesc);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_073
* @tc.desc : Test HandleLocalDeviceDisconnected interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_073, TestSize.Level1)
{
    int32_t result;
    AudioDeviceDescriptor updatedDesc;
    updatedDesc.deviceType_ = DEVICE_TYPE_BT_SPP;

    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    result = audioDeviceStatus.HandleLocalDeviceDisconnected(updatedDesc);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_037
* @tc.desc : Test HandleArmUsbDevice interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_037, TestSize.Level1)
{
    int32_t result;
    DeviceType deviceType = DEVICE_TYPE_USB_ARM_HEADSET;
    DeviceRole deviceRole = INPUT_DEVICE;
    std::string address = "00:11:22:33:44:55";

    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    result = audioDeviceStatus.HandleArmUsbDevice(deviceType, deviceRole, address);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_038
* @tc.desc : Test HandleArmUsbDevice interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_038, TestSize.Level1)
{
    int32_t result;
    DeviceType deviceType = DEVICE_TYPE_USB_HEADSET;
    DeviceRole deviceRole = OUTPUT_DEVICE;
    std::string address = "00:11:22:33:44:55";

    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    AudioDeviceDescriptor deviceDescriptor;
    deviceDescriptor.deviceType_ = DEVICE_TYPE_USB_HEADSET;
    audioDeviceStatus.audioActiveDevice_.SetCurrentOutputDevice(deviceDescriptor);

    result = audioDeviceStatus.HandleArmUsbDevice(deviceType, deviceRole, address);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_039
* @tc.desc : Test HandleSpecialDeviceType interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_039, TestSize.Level1)
{
    int32_t result;
    DeviceType deviceType = DEVICE_TYPE_WAKEUP;
    DeviceRole deviceRole = OUTPUT_DEVICE;
    std::string address = "00:11:22:33:44:55";
    bool isConnected = true;

    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    result = audioDeviceStatus.HandleSpecialDeviceType(deviceType, isConnected, address, deviceRole);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_040
* @tc.desc : Test HandleSpecialDeviceType interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_040, TestSize.Level1)
{
    int32_t result;
    DeviceType deviceType = DEVICE_TYPE_USB_HEADSET;
    DeviceRole deviceRole = OUTPUT_DEVICE;
    std::string address = "";
    bool isConnected = true;

    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    result = audioDeviceStatus.HandleSpecialDeviceType(deviceType, isConnected, address, deviceRole);
    EXPECT_EQ(result, ERROR);
}

/**
* @tc.name : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_041
* @tc.desc : Test HandleSpecialDeviceType interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_041, TestSize.Level1)
{
    int32_t result;
    DeviceType deviceType = DEVICE_TYPE_USB_ARM_HEADSET;
    DeviceRole deviceRole = OUTPUT_DEVICE;
    std::string address = "";
    bool isConnected = true;

    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    result = audioDeviceStatus.HandleSpecialDeviceType(deviceType, isConnected, address, deviceRole);
    EXPECT_EQ(result, ERROR);
}

/**
* @tc.name : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_042
* @tc.desc : Test HandleSpecialDeviceType interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_042, TestSize.Level1)
{
    int32_t result;
    DeviceType deviceType = DEVICE_TYPE_USB_ARM_HEADSET;
    DeviceRole deviceRole = DEVICE_ROLE_NONE;
    std::string address = "00:11:22:33:44:55";
    bool isConnected = true;

    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    result = audioDeviceStatus.HandleSpecialDeviceType(deviceType, isConnected, address, deviceRole);
    EXPECT_EQ(result, ERROR);
}

/**
* @tc.name : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_043
* @tc.desc : Test HandleSpecialDeviceType interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_043, TestSize.Level1)
{
    int32_t result;
    DeviceType deviceType = DEVICE_TYPE_USB_ARM_HEADSET;
    DeviceRole deviceRole = OUTPUT_DEVICE;
    std::string address = "00:11:22:33:44:55";
    bool isConnected = true;

    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    result = audioDeviceStatus.HandleSpecialDeviceType(deviceType, isConnected, address, deviceRole);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_044
* @tc.desc : Test HandleSpecialDeviceType interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_044, TestSize.Level1)
{
    int32_t result;
    DeviceType deviceType = DEVICE_TYPE_USB_ARM_HEADSET;
    DeviceRole deviceRole = OUTPUT_DEVICE;
    std::string address = "00:11:22:33:44:55";
    bool isConnected = false;

    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    result = audioDeviceStatus.HandleSpecialDeviceType(deviceType, isConnected, address, deviceRole);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_045
* @tc.desc : Test HandleSpecialDeviceType interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_045, TestSize.Level1)
{
    int32_t result;
    DeviceType deviceType = DEVICE_TYPE_EXTERN_CABLE;
    DeviceRole deviceRole = OUTPUT_DEVICE;
    std::string address = "00:11:22:33:44:55";
    bool isConnected = false;

    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    result = audioDeviceStatus.HandleSpecialDeviceType(deviceType, isConnected, address, deviceRole);
    EXPECT_EQ(result, ERROR);
}

/**
* @tc.name : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_046
* @tc.desc : Test HandleSpecialDeviceType interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_046, TestSize.Level1)
{
    int32_t result;
    DeviceType deviceType = DEVICE_TYPE_EXTERN_CABLE;
    DeviceRole deviceRole = OUTPUT_DEVICE;
    std::string address = "00:11:22:33:44:55";
    bool isConnected = true;

    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    result = audioDeviceStatus.HandleSpecialDeviceType(deviceType, isConnected, address, deviceRole);
    EXPECT_EQ(result, ERROR);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_047
* @tc.desc  : Test HandleSpecialDeviceType with USB_HEADSET and connected.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_047, TestSize.Level1)
{
    DeviceType deviceType = DEVICE_TYPE_USB_HEADSET;
    bool isConnected = true;
    std::string address = "00:11:22:33:44:55";
    DeviceRole deviceRole = OUTPUT_DEVICE;
    int32_t ret = 0;

    std::shared_ptr<AudioDeviceDescriptor> remoteDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    remoteDeviceDescriptor->deviceType_ = DEVICE_TYPE_USB_HEADSET;
    remoteDeviceDescriptor->macAddress_ = address;
    remoteDeviceDescriptor->deviceRole_ = DeviceRole::OUTPUT_DEVICE;

    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioDeviceStatus.audioConnectedDevice_.AddConnectedDevice(remoteDeviceDescriptor);

    ret = audioDeviceStatus.HandleSpecialDeviceType(deviceType, isConnected, address, deviceRole);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_048
* @tc.desc  : Test HandleSpecialDeviceType with USB_HEADSET and connected.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_048, TestSize.Level1)
{
    DeviceType deviceType = DEVICE_TYPE_USB_HEADSET;
    bool isConnected = true;
    std::string address = "00:11:22:33:44:55";
    DeviceRole deviceRole = OUTPUT_DEVICE;
    int32_t ret = 0;

    std::shared_ptr<AudioDeviceDescriptor> remoteDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    remoteDeviceDescriptor->deviceType_ = DEVICE_TYPE_USB_HEADSET;
    remoteDeviceDescriptor->macAddress_ = "";
    remoteDeviceDescriptor->deviceRole_ = DeviceRole::OUTPUT_DEVICE;

    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioDeviceStatus.audioConnectedDevice_.AddConnectedDevice(remoteDeviceDescriptor);

    ret = audioDeviceStatus.HandleSpecialDeviceType(deviceType, isConnected, address, deviceRole);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_049
* @tc.desc  : Test HandleSpecialDeviceType with USB_HEADSET and connected.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_049, TestSize.Level1)
{
    DeviceType deviceType = DEVICE_TYPE_USB_ARM_HEADSET;
    bool isConnected = false;
    std::string address = "00:11:22:33:44:55";
    DeviceRole deviceRole = OUTPUT_DEVICE;
    int32_t ret = 0;

    std::shared_ptr<AudioDeviceDescriptor> remoteDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    remoteDeviceDescriptor->deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;
    remoteDeviceDescriptor->macAddress_ = "00:11:22:33:44:55";
    remoteDeviceDescriptor->deviceRole_ = DeviceRole::OUTPUT_DEVICE;

    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioDeviceStatus.audioConnectedDevice_.AddConnectedDevice(remoteDeviceDescriptor);

    ret = audioDeviceStatus.HandleSpecialDeviceType(deviceType, isConnected, address, deviceRole);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_050
* @tc.desc  : Test OnPnpDeviceStatusUpdated with DEVICE_TYPE_NONE.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_050, TestSize.Level1)
{
    AudioDeviceDescriptor desc;
    desc.deviceType_ = DEVICE_TYPE_NONE;
    desc.macAddress_ = "00:11:22:33:44:55";
    desc.deviceName_ = "NONE";
    bool isConnected = true;

    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    audioDeviceStatus.OnPnpDeviceStatusUpdated(desc, isConnected);
    EXPECT_NE(audioDeviceStatus.audioPolicyServerHandler_, nullptr);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_051
* @tc.desc  : Test OnPnpDeviceStatusUpdated hasModulesLoaded is false.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_051, TestSize.Level1)
{
    AudioDeviceDescriptor desc;
    desc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    desc.macAddress_ = "00:11:22:33:44:55";
    desc.deviceName_ = "Bluetooth A2DP Device";
    bool isConnected = false;

    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioDeviceStatus.hasModulesLoaded = false;

    audioDeviceStatus.OnPnpDeviceStatusUpdated(desc, isConnected);
    EXPECT_NE(audioDeviceStatus.audioPolicyServerHandler_, nullptr);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_052
* @tc.desc  : Test OnPnpDeviceStatusUpdated hasModulesLoaded is true.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_052, TestSize.Level1)
{
    AudioDeviceDescriptor desc;
    desc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    desc.macAddress_ = "00:11:22:33:44:55";
    desc.deviceName_ = "Bluetooth A2DP Device";
    bool isConnected = true;

    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioDeviceStatus.hasModulesLoaded = true;

    audioDeviceStatus.OnPnpDeviceStatusUpdated(desc, isConnected);
    EXPECT_NE(audioDeviceStatus.audioPolicyServerHandler_, nullptr);
    desc.deviceType_ = DEVICE_TYPE_DP;
    audioDeviceStatus.OnPnpDeviceStatusUpdated(desc, true);
    audioDeviceStatus.OnPnpDeviceStatusUpdated(desc, true);
    audioDeviceStatus.OnPnpDeviceStatusUpdated(desc, false);
    audioDeviceStatus.OnPnpDeviceStatusUpdated(desc, false);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_053
* @tc.desc  : Test UpdateActiveA2dpDeviceWhenDisconnecting
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_053, TestSize.Level1)
{
    std::string address = "00:11:22:33:44:55";

    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioDeviceStatus.audioDeviceManager_.connectedDevices_.clear();

    audioDeviceStatus.UpdateActiveA2dpDeviceWhenDisconnecting(address);
    EXPECT_NE(audioDeviceStatus.audioPolicyServerHandler_, nullptr);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_054
* @tc.desc  : Test UpdateActiveA2dpDeviceWhenDisconnecting.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_054, TestSize.Level1)
{
    std::string address = "00:11:22:33:44:55";

    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioDeviceStatus.audioDeviceManager_.connectedDevices_.clear();

    AudioDeviceDescriptor descA;
    descA.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    descA.macAddress_ = "11:22:33:44:55:66";
    descA.connectState_ = CONNECTED;
    audioDeviceStatus.audioDeviceManager_.connectedDevices_.push_back(
        std::make_shared<AudioDeviceDescriptor>(descA));

    audioDeviceStatus.UpdateActiveA2dpDeviceWhenDisconnecting(address);
    audioDeviceStatus.audioDeviceManager_.connectedDevices_.clear();
    EXPECT_NE(audioDeviceStatus.audioPolicyServerHandler_, nullptr);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_055
* @tc.desc  : Test IsConfigurationUpdated.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_055, TestSize.Level1)
{
    DeviceType deviceType = DEVICE_TYPE_INVALID;
    AudioStreamInfo streamInfo;
    bool result;

    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    result = audioDeviceStatus.IsConfigurationUpdated(deviceType, streamInfo);
    EXPECT_EQ(result, false);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_056
* @tc.desc  : Test IsConfigurationUpdated.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_056, TestSize.Level1)
{
    DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    AudioStreamInfo streamInfo;
    streamInfo.format = AudioSampleFormat::SAMPLE_S32LE;
    bool result;
    std::string device = "00:11:22:33:44:55";
    A2dpDeviceConfigInfo config;
    config.streamInfo.format = AudioSampleFormat::SAMPLE_S24LE;

    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    audioDeviceStatus.audioA2dpDevice_.AddA2dpInDevice(device, config);
    audioDeviceStatus.audioActiveDevice_.SetActiveBtDeviceMac(device);

    result = audioDeviceStatus.IsConfigurationUpdated(deviceType, streamInfo);
    EXPECT_EQ(result, false);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_057
* @tc.desc  : Test IsConfigurationUpdated.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_057, TestSize.Level1)
{
    DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    AudioStreamInfo streamInfo;
    streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    bool result;
    std::string device = "00:11:22:33:44:55";
    A2dpDeviceConfigInfo config;
    config.streamInfo.format = SAMPLE_S32LE;

    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    audioDeviceStatus.audioA2dpDevice_.AddA2dpInDevice(device, config);
    audioDeviceStatus.audioActiveDevice_.SetActiveBtDeviceMac(device);

    result = audioDeviceStatus.IsConfigurationUpdated(deviceType, streamInfo);
    EXPECT_EQ(result, false);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_058
* @tc.desc  : Test IsConfigurationUpdated.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_058, TestSize.Level1)
{
    DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    AudioStreamInfo streamInfo;
    streamInfo.channels = MONO;
    bool result;
    std::string device = "00:11:22:33:44:55";
    A2dpDeviceConfigInfo config;
    config.streamInfo.format = SAMPLE_S32LE;

    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    audioDeviceStatus.audioA2dpDevice_.AddA2dpInDevice(device, config);
    audioDeviceStatus.audioActiveDevice_.SetActiveBtDeviceMac(device);

    result = audioDeviceStatus.IsConfigurationUpdated(deviceType, streamInfo);
    EXPECT_EQ(result, false);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_059
* @tc.desc  : Test OpenPortAndAddDeviceOnServiceConnected.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_059, TestSize.Level1)
{
    bool result = false;
    AudioModuleInfo moduleInfo;
    moduleInfo.name = "file_source";

    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    result = audioDeviceStatus.OpenPortAndAddDeviceOnServiceConnected(moduleInfo);
    EXPECT_EQ(result, true);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_060
* @tc.desc  : Test OpenPortAndAddDeviceOnServiceConnected.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_060, TestSize.Level1)
{
    bool result = false;
    AudioModuleInfo moduleInfo;
    moduleInfo.name = "Built_in_mic";

    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    result = audioDeviceStatus.OpenPortAndAddDeviceOnServiceConnected(moduleInfo);
    EXPECT_EQ(result, true);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_061
* @tc.desc  : Test OpenPortAndAddDeviceOnServiceConnected.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_061, TestSize.Level1)
{
    bool result = false;
    AudioModuleInfo moduleInfo;
    moduleInfo.name = "Speaker";

    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    result = audioDeviceStatus.OpenPortAndAddDeviceOnServiceConnected(moduleInfo);
    EXPECT_EQ(result, false);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_062
* @tc.desc  : Test OnForcedDeviceSelected.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_062, TestSize.Level1)
{
    bool result = false;
    std::string macAddress = "00:11:22:33:44:55";
    DeviceType devType = DEVICE_TYPE_BLUETOOTH_SCO;

    std::shared_ptr<AudioDeviceDescriptor> remoteDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    remoteDeviceDescriptor->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    remoteDeviceDescriptor->macAddress_ = "00:11:22:33:44:55";
    remoteDeviceDescriptor->deviceRole_ = DeviceRole::INPUT_DEVICE;

    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioDeviceStatus.audioConnectedDevice_.AddConnectedDevice(remoteDeviceDescriptor);


    audioDeviceStatus.OnForcedDeviceSelected(devType, macAddress);
    EXPECT_NE(audioDeviceStatus.audioPolicyServerHandler_, nullptr);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_063
* @tc.desc  : Test AddAudioDevice.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_063, TestSize.Level1)
{
    AudioModuleInfo info;
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    audioDeviceStatus.AddAudioDevice(info, DEVICE_TYPE_SPEAKER);
    info = {};
    info.supportedRate_.insert(CH_LAYOUT_STEREO);
    audioDeviceStatus.AddAudioDevice(info, DEVICE_TYPE_SPEAKER);
    info = {};
    info.supportedChannelLayout_.insert(SAMPLE_RATE_48000);
    audioDeviceStatus.AddAudioDevice(info, DEVICE_TYPE_SPEAKER);
    info = {};
    info.supportedRate_.insert(CH_LAYOUT_STEREO);
    info.supportedChannelLayout_.insert(SAMPLE_RATE_48000);
    audioDeviceStatus.AddAudioDevice(info, DEVICE_TYPE_SPEAKER);
    EXPECT_NE(audioDeviceStatus.audioConnectedDevice_.connectedDevices_.size(), 0);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_068
* @tc.desc  : Test HandleLocalDeviceConnected interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_068, TestSize.Level1)
{
    AudioDeviceDescriptor updatedDesc;
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    updatedDesc.deviceType_ = DEVICE_TYPE_HEARING_AID;
    int32_t result = audioDeviceStatus.HandleLocalDeviceConnected(updatedDesc);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: AudioDeviceStatus_069
* @tc.desc  : Test HandleLocalDeviceDisconnected interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AudioDeviceStatus_069, TestSize.Level1)
{
    AudioDeviceDescriptor updatedDesc;
    updatedDesc.deviceType_ = DEVICE_TYPE_HEARING_AID;
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    int32_t result = audioDeviceStatus.HandleLocalDeviceDisconnected(updatedDesc);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: WriteOutputDeviceChangedSysEvents_001
* @tc.desc  : Test WriteOutputDeviceChangedSysEvents interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, WriteOutputDeviceChangedSysEvents_001, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    SinkInput sinkInput;
    deviceDescriptor->deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;
    deviceDescriptor->macAddress_ = "00:11:22:33:44:55";
    deviceDescriptor->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    deviceDescriptor->networkId_ = "123456";
    deviceDescriptor->deviceName_ = "usb_headset";
    deviceDescriptor->deviceCategory_ = BT_UNWEAR_HEADPHONE;
    sinkInput.streamId = 1;
    sinkInput.streamType = STREAM_RING;
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    audioDeviceStatus.WriteOutputDeviceChangedSysEvents(deviceDescriptor, sinkInput);
    EXPECT_EQ(deviceDescriptor->deviceId_, 0);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: WriteInputDeviceChangedSysEvents_001
* @tc.desc  : Test WriteInputDeviceChangedSysEvents interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, WriteInputDeviceChangedSysEvents_001, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    SourceOutput sourceOutput;
    deviceDescriptor->deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;
    deviceDescriptor->macAddress_ = "00:11:22:33:44:55";
    deviceDescriptor->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    deviceDescriptor->networkId_ = "123456";
    deviceDescriptor->deviceName_ = "usb_headset";
    deviceDescriptor->deviceCategory_ = BT_UNWEAR_HEADPHONE;
    sourceOutput.streamId = 1;
    sourceOutput.streamType = STREAM_RING;
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    audioDeviceStatus.WriteInputDeviceChangedSysEvents(deviceDescriptor, sourceOutput);
    EXPECT_EQ(deviceDescriptor->deviceId_, 0);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: RemoveDeviceFromGlobalOnly_001
* @tc.desc  : Test RemoveDeviceFromGlobalOnly interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, RemoveDeviceFromGlobalOnly_001, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    deviceDescriptor->deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;
    deviceDescriptor->macAddress_ = "00:11:22:33:44:55";
    deviceDescriptor->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    deviceDescriptor->networkId_ = "123456";
    deviceDescriptor->deviceName_ = "usb_headset";
    deviceDescriptor->deviceCategory_ = BT_UNWEAR_HEADPHONE;
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    audioDeviceStatus.RemoveDeviceFromGlobalOnly(deviceDescriptor);
    EXPECT_EQ(deviceDescriptor->deviceId_, 0);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: AddDeviceBackToGlobalOnly_001
* @tc.desc  : Test AddDeviceBackToGlobalOnly interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, AddDeviceBackToGlobalOnly_001, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    deviceDescriptor->deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;
    deviceDescriptor->macAddress_ = "00:11:22:33:44:55";
    deviceDescriptor->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    deviceDescriptor->networkId_ = "123456";
    deviceDescriptor->deviceName_ = "usb_headset";
    deviceDescriptor->deviceCategory_ = BT_UNWEAR_HEADPHONE;
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    audioDeviceStatus.AddDeviceBackToGlobalOnly(deviceDescriptor);
    EXPECT_EQ(deviceDescriptor->deviceId_, 0);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: GetDmDeviceType_001
* @tc.desc  : Test GetDmDeviceType interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, GetDmDeviceType_001, TestSize.Level1)
{
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    uint16_t ret = audioDeviceStatus.GetDmDeviceType();
    EXPECT_EQ(ret, 0);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: GetPaIndexByPortName_001
* @tc.desc  : Test GetPaIndexByPortName interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, GetPaIndexByPortName_001, TestSize.Level1)
{
    string portName = PRIMARY_SPEAKER;
    AudioIOHandle moduleId = 2;
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();

    audioDeviceStatus.audioIOHandleMap_.AddIOHandleInfo(portName, moduleId);
    uint32_t ret = audioDeviceStatus.GetPaIndexByPortName(portName);
    EXPECT_NE(ret, moduleId);
}

/**
* @tc.name  : Test AudioDeviceStatus.
* @tc.number: UpdateNearlinkDeviceVolume_001
* @tc.desc  : Test UpdateNearlinkDeviceVolume interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, UpdateNearlinkDeviceVolume_001, TestSize.Level1)
{
    AudioDeviceDescriptor desc;
    desc.deviceType_ = DEVICE_TYPE_NEARLINK;
    desc.deviceCategory_ = CATEGORY_DEFAULT;
    desc.macAddress_ = "";

    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    int32_t result;
    VolumeUtils::SetPCVolumeEnable(true);
    result = audioDeviceStatus.UpdateNearlinkDeviceVolume(desc);
    EXPECT_EQ(result, SUCCESS);

    VolumeUtils::SetPCVolumeEnable(false);
    result = audioDeviceStatus.UpdateNearlinkDeviceVolume(desc);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test ClearActiveHfpDevice.
* @tc.number: ClearActiveHfpDevice_001
* @tc.desc  : Test ClearActiveHfpDevice interface.
*/
HWTEST_F(AudioDeviceStatusUnitTest, ClearActiveHfpDevice_001, TestSize.Level1)
{
    AudioDeviceDescriptor desc;
    desc.deviceType_ = DEVICE_TYPE_NEARLINK;
    desc.macAddress_ = "LOCALDEVICE";
    AudioDeviceStatus& audioDeviceStatus = AudioDeviceStatus::GetInstance();
    AudioStreamDeviceChangeReasonExt::ExtEnum oldDevice =
        AudioStreamDeviceChangeReasonExt::ExtEnum::OLD_DEVICE_UNAVALIABLE;
    AudioStreamDeviceChangeReasonExt reason(oldDevice);
    audioDeviceStatus.ClearActiveHfpDevice(desc, CATEGORY_UPDATE, reason);
    EXPECT_EQ(desc.macAddress_, "LOCALDEVICE");

    desc.deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    desc.deviceCategory_ = BT_HEADPHONE;
    desc.isEnable_ = true;
    desc.connectState_ = DEACTIVE_CONNECTED;
    desc.exceptionFlag_ = false;
    audioDeviceStatus.ClearActiveHfpDevice(desc, CATEGORY_UPDATE, reason);
    EXPECT_EQ(desc.macAddress_, "LOCALDEVICE");
    desc.deviceCategory_ = BT_UNWEAR_HEADPHONE;
    audioDeviceStatus.ClearActiveHfpDevice(desc, CATEGORY_UPDATE, reason);
    EXPECT_EQ(desc.macAddress_, "LOCALDEVICE");
    audioDeviceStatus.ClearActiveHfpDevice(desc, ENABLE_UPDATE, reason);
    EXPECT_EQ(desc.macAddress_, "LOCALDEVICE");
    desc.isEnable_ = false;
    audioDeviceStatus.ClearActiveHfpDevice(desc, ENABLE_UPDATE, reason);
    EXPECT_EQ(desc.macAddress_, "LOCALDEVICE");
    audioDeviceStatus.ClearActiveHfpDevice(desc, CONNECTSTATE_UPDATE, reason);
    EXPECT_EQ(desc.macAddress_, "LOCALDEVICE");
    desc.connectState_ = SUSPEND_CONNECTED;
    audioDeviceStatus.ClearActiveHfpDevice(desc, CONNECTSTATE_UPDATE, reason);
    EXPECT_EQ(desc.macAddress_, "LOCALDEVICE");
    audioDeviceStatus.ClearActiveHfpDevice(desc, EXCEPTION_FLAG_UPDATE, reason);
    EXPECT_EQ(desc.macAddress_, "LOCALDEVICE");
    desc.exceptionFlag_ = true;
    audioDeviceStatus.ClearActiveHfpDevice(desc, EXCEPTION_FLAG_UPDATE, reason);
    EXPECT_EQ(desc.macAddress_, "LOCALDEVICE");
}
} // namespace AudioStandard
} // namespace OHOS
