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

#include "audio_device_status_extended_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
void AudioDeviceStatusExtendedTest::SetUpTestCase(void) {}
void AudioDeviceStatusExtendedTest::TearDownTestCase(void) {}
void AudioDeviceStatusExtendedTest::SetUp(void)
{
    audioDeviceStatus_ = std::make_shared<AudioDeviceStatus>();
}

void AudioDeviceStatusExtendedTest::TearDown(void)
{
    audioDeviceStatus_ = nullptr;
}

/**
 * @tc.name  : Test AudioDeviceStatus.
 * @tc.number: AudioDeviceStatus_001
 * @tc.desc  : Test WriteAllDeviceSysEvents deviceDescriptor == nullptr.
 */
HWTEST_F(AudioDeviceStatusExtendedTest, AudioDeviceStatus_001, TestSize.Level4)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc = {};
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = nullptr;
    desc.push_back(deviceDescriptor);
    bool isConnected = true;
    EXPECT_NO_THROW(audioDeviceStatus_->WriteAllDeviceSysEvents(desc, isConnected));
}

/**
 * @tc.name  : Test AudioDeviceStatus.
 * @tc.number: AudioDeviceStatus_002
 * @tc.desc  : Test RehandlePnpDevice deviceType == DEVICE_TYPE_USB_HEADSET.
 */
HWTEST_F(AudioDeviceStatusExtendedTest, AudioDeviceStatus_002, TestSize.Level4)
{
    DeviceType deviceType = DEVICE_TYPE_USB_HEADSET;
    DeviceRole deviceRole = INPUT_DEVICE;
    const std::string &address = "usb:1";
    EXPECT_EQ(audioDeviceStatus_->RehandlePnpDevice(deviceType, deviceRole, address), SUCCESS);
}

/**
 * @tc.name  : Test AudioDeviceStatus.
 * @tc.number: AudioDeviceStatus_003
 * @tc.desc  : Test HandleDpDevice deviceType != DEVICE_TYPE_DP.
 */
HWTEST_F(AudioDeviceStatusExtendedTest, AudioDeviceStatus_003, TestSize.Level4)
{
    DeviceType deviceType = DEVICE_TYPE_INVALID;
    std::string address = "00:11:22:33:44:55";
    audioDeviceStatus_->audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_DP;
    EXPECT_EQ(audioDeviceStatus_->HandleDpDevice(deviceType, address), SUCCESS);
}

/**
 * @tc.name  : Test AudioDeviceStatus.
 * @tc.number: AudioDeviceStatus_004
 * @tc.desc  : Test HandleAccessoryDevice LoadAccessoryModule(defaulyAccessoryInfo) != SUCCESS.
 */
HWTEST_F(AudioDeviceStatusExtendedTest, AudioDeviceStatus_004, TestSize.Level4)
{
    DeviceType deviceType = DEVICE_TYPE_INVALID;
    std::string address = "00:11:22:33:44:55";
    audioDeviceStatus_->audioConfigManager_.deviceClassInfo_.clear();
    EXPECT_EQ(audioDeviceStatus_->HandleAccessoryDevice(deviceType, address), ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test AudioDeviceStatus.
 * @tc.number: AudioDeviceStatus_005
 * @tc.desc  : Test HandleLocalDeviceConnected updatedDesc.deviceType_ == DEVICE_TYPE_ACCESSORY.
 */
HWTEST_F(AudioDeviceStatusExtendedTest, AudioDeviceStatus_005, TestSize.Level4)
{
    AudioDeviceDescriptor updatedDesc;
    updatedDesc.deviceType_ = DEVICE_TYPE_ACCESSORY;
    EXPECT_EQ(audioDeviceStatus_->HandleLocalDeviceConnected(updatedDesc), ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test AudioDeviceStatus.
 * @tc.number: AudioDeviceStatus_006
 * @tc.desc  : Test HandleLocalDeviceDisconnected updatedDesc.deviceType_ == DEVICE_TYPE_ACCESSORY.
 */
HWTEST_F(AudioDeviceStatusExtendedTest, AudioDeviceStatus_006, TestSize.Level4)
{
    AudioDeviceDescriptor updatedDesc;
    updatedDesc.deviceType_ = DEVICE_TYPE_ACCESSORY;
    EXPECT_EQ(audioDeviceStatus_->HandleLocalDeviceDisconnected(updatedDesc), SUCCESS);
}

/**
 * @tc.name  : Test AudioDeviceStatus.
 * @tc.number: AudioDeviceStatus_007
 * @tc.desc  : Test HandleLocalDeviceDisconnected audioA2dpDevice_.DelHearingAidDevice(updatedDesc.macAddress_) != 0.
 */
HWTEST_F(AudioDeviceStatusExtendedTest, AudioDeviceStatus_007, TestSize.Level4)
{
    AudioDeviceDescriptor updatedDesc;
    updatedDesc.deviceType_ = DEVICE_TYPE_HEARING_AID;
    updatedDesc.macAddress_ = "00:11:22:33:44:55";
    std::string testMacAddress = "11:11:11:11:11:11";
    audioDeviceStatus_->audioA2dpDevice_.connectedHearingAidDeviceMap_[testMacAddress] = A2dpDeviceConfigInfo();
    audioDeviceStatus_->HandleLocalDeviceDisconnected(updatedDesc);
    EXPECT_EQ(audioDeviceStatus_->audioA2dpDevice_.connectedHearingAidDeviceMap_.size(), 1);
}

/**
 * @tc.name  : Test AudioDeviceStatus.
 * @tc.number: AudioDeviceStatus_008
 * @tc.desc  : Test LoadDpModule audioIOHandleMap_.CheckIOHandleExist(moduleInfo.name) == true.
 */
HWTEST_F(AudioDeviceStatusExtendedTest, AudioDeviceStatus_008, TestSize.Level4)
{
    AudioModuleInfo moduleInfo;
    moduleInfo.role = ROLE_SOURCE;
    moduleInfo.name = "testDpModule";
    audioDeviceStatus_->audioConfigManager_.deviceClassInfo_[ClassType::TYPE_DP] = { moduleInfo };
    AudioIOHandle ioHandle = 1;
    audioDeviceStatus_->audioIOHandleMap_.IOHandles_[moduleInfo.name] = ioHandle;
    std::string deviceInfo = "testDeviceInfo";
    EXPECT_EQ(audioDeviceStatus_->LoadDpModule(deviceInfo), SUCCESS);
}

/**
 * @tc.name  : Test AudioDeviceStatus.
 * @tc.number: AudioDeviceStatus_009
 * @tc.desc  : Test LoadAccessoryModule audioIOHandleMap_.CheckIOHandleExist(moduleInfo.name).
 */
HWTEST_F(AudioDeviceStatusExtendedTest, AudioDeviceStatus_009, TestSize.Level4)
{
    AudioModuleInfo moduleInfo;
    moduleInfo.role = ROLE_SOURCE;
    moduleInfo.name = "testAccessoryModule";
    audioDeviceStatus_->audioConfigManager_.deviceClassInfo_[ClassType::TYPE_ACCESSORY] = { moduleInfo };
    std::string deviceInfo = "rate=480000";
    audioDeviceStatus_->audioIOHandleMap_.IOHandles_[moduleInfo.name] = 1;
    EXPECT_EQ(audioDeviceStatus_->LoadAccessoryModule(deviceInfo), SUCCESS);
}

/**
 * @tc.name  : Test AudioDeviceStatus.
 * @tc.number: AudioDeviceStatus_010
 * @tc.desc  : Test HandleSpecialDeviceType connectedHeadsetType != DEVICE_TYPE_NONE.
 */
HWTEST_F(AudioDeviceStatusExtendedTest, AudioDeviceStatus_010, TestSize.Level4)
{
    DeviceType devType = DEVICE_TYPE_EXTERN_CABLE;
    bool isConnected = true;
    std::string address = "00:11:22:33:44:55";
    DeviceRole role = INPUT_DEVICE;
    auto audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_WIRED_HEADSET, role);
    audioDeviceStatus_->audioConnectedDevice_.connectedDevices_.emplace_back(audioDeviceDescriptor);
    audioDeviceStatus_->HandleSpecialDeviceType(devType, isConnected, address, role);
    EXPECT_FALSE(isConnected);
}

/**
 * @tc.name  : Test AudioDeviceStatus.
 * @tc.number: AudioDeviceStatus_011
 * @tc.desc  : Test OnBlockedStatusUpdated (*it)->capturerState == CAPTURER_RUNNING.
 */
HWTEST_F(AudioDeviceStatusExtendedTest, AudioDeviceStatus_011, TestSize.Level4)
{
    auto changeInfo = std::make_shared<AudioCapturerChangeInfo>();
    changeInfo->clientUID = 1;
    changeInfo->capturerState = CAPTURER_RUNNING;
    audioDeviceStatus_->streamCollector_.audioCapturerChangeInfos_.push_back(changeInfo);
    audioDeviceStatus_->audioPolicyServerHandler_.reset();

    DeviceType devType = DEVICE_TYPE_INVALID;
    DeviceBlockStatus status = DEVICE_BLOCKED;
    audioDeviceStatus_->OnBlockedStatusUpdated(devType, status);

    changeInfo->capturerState = CAPTURER_NEW;
    audioDeviceStatus_->OnBlockedStatusUpdated(devType, status);
    EXPECT_EQ(audioDeviceStatus_->streamCollector_.audioCapturerChangeInfos_.size(), 1);
}

/**
 * @tc.name  : Test AudioDeviceStatus.
 * @tc.number: AudioDeviceStatus_012
 * @tc.desc  : Test OnDeviceConfigurationChanged audioA2dpOffloadManager_ == nullptr.
 */
HWTEST_F(AudioDeviceStatusExtendedTest, AudioDeviceStatus_012, TestSize.Level4)
{
    DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    std::string macAddress = "00:11:22:33:44:55";
    std::string deviceName = "testDevice";
    AudioStreamInfo streamInfo = AudioStreamInfo();
    audioDeviceStatus_->audioActiveDevice_.activeBTDevice_ = macAddress;
    audioDeviceStatus_->audioA2dpOffloadManager_ = nullptr;
    audioDeviceStatus_->OnDeviceConfigurationChanged(deviceType, macAddress, deviceName, streamInfo);
    EXPECT_EQ(audioDeviceStatus_->audioA2dpDevice_.connectedA2dpDeviceMap_.size(), 0);
}

/**
 * @tc.name  : Test AudioDeviceStatus.
 * @tc.number: AudioDeviceStatus_013
 * @tc.desc  : Test IsConfigurationUpdated audioStreamInfo.samplingRate != streamInfo.samplingRate.
 */
HWTEST_F(AudioDeviceStatusExtendedTest, AudioDeviceStatus_013, TestSize.Level4)
{
    DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    AudioStreamInfo streamInfo = AudioStreamInfo();
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = AudioSampleFormat::SAMPLE_U8;
    streamInfo.channels = AudioChannel::CHANNEL_16;
    audioDeviceStatus_->audioActiveDevice_.activeBTDevice_ = "testBTDevice";
    A2dpDeviceConfigInfo config;
    config.streamInfo.samplingRate.insert(AudioSamplingRate::SAMPLE_RATE_44100);
    config.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    config.streamInfo.channelLayout.insert(AudioChannelLayout::CH_LAYOUT_MONO);
    audioDeviceStatus_->audioActiveDevice_.audioA2dpDevice_.connectedA2dpDeviceMap_["testBTDevice"] = config;
    EXPECT_TRUE(audioDeviceStatus_->IsConfigurationUpdated(deviceType, streamInfo));
}

/**
 * @tc.name  : Test AudioDeviceStatus.
 * @tc.number: AudioDeviceStatus_014
 * @tc.desc  : Test GetDeviceTypeFromPin hdiPin == AUDIO_PIN_IN_UWB.
 */
HWTEST_F(AudioDeviceStatusExtendedTest, AudioDeviceStatus_014, TestSize.Level4)
{
    AudioPin hdiPin = AudioPin::AUDIO_PIN_IN_UWB;
    DeviceType deviceType = audioDeviceStatus_->GetDeviceTypeFromPin(hdiPin);
    EXPECT_EQ(deviceType, DEVICE_TYPE_ACCESSORY);
}

/**
 * @tc.name  : Test AudioDeviceStatus.
 * @tc.number: AudioDeviceStatus_015
 * @tc.desc  : Test OnDeviceStatusUpdated deviceRole != DeviceRole::INPUT_DEVICE.
 */
HWTEST_F(AudioDeviceStatusExtendedTest, AudioDeviceStatus_015, TestSize.Level4)
{
    DStatusInfo statusInfo;
    statusInfo.isConnected = false;
    std::string testNetworkId = "testNetworkId";
    strncpy_s(statusInfo.networkId, NETWORK_ID_SIZE, testNetworkId.c_str(), NETWORK_ID_SIZE - 1);
    statusInfo.hdiPin = AudioPin::AUDIO_PIN_OUT_SPEAKER;
    bool isStop = false;
    audioDeviceStatus_->audioA2dpOffloadManager_.reset();
    audioDeviceStatus_->OnDeviceStatusUpdated(statusInfo, isStop);
    EXPECT_FALSE(audioDeviceStatus_->remoteCapturerSwitch_);
}

/**
 * @tc.name  : Test AudioDeviceStatus.
 * @tc.number: AudioDeviceStatus_016
 * @tc.desc  : Test HandleDistributedDeviceUpdate
               audioConnectedDevice_.GetConnectedDeviceByType(networkId, devType) != nullptr.
 */
HWTEST_F(AudioDeviceStatusExtendedTest, AudioDeviceStatus_016, TestSize.Level4)
{
    DStatusInfo statusInfo;
    statusInfo.isConnected = true;
    std::string testNetworkId = "testNetworkId";
    strncpy_s(statusInfo.networkId, NETWORK_ID_SIZE, testNetworkId.c_str(), NETWORK_ID_SIZE - 1);
    statusInfo.hdiPin = AudioPin::AUDIO_PIN_OUT_SPEAKER;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descForCb;
    AudioStreamDeviceChangeReasonExt reason;
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceType_ = DEVICE_TYPE_SPEAKER;
    deviceDesc->networkId_ = "testNetworkId";
    audioDeviceStatus_->audioConnectedDevice_.connectedDevices_.push_back(deviceDesc);
    EXPECT_EQ(audioDeviceStatus_->HandleDistributedDeviceUpdate(statusInfo, descForCb, reason), ERROR);
}

/**
 * @tc.name  : Test AudioDeviceStatus.
 * @tc.number: AudioDeviceStatus_017
 * @tc.desc  : Test AddEarpiece audioConfigManager_.GetHasEarpiece() == false
               and audioConnectedDevice_.GetConnectedDeviceByType(DEVICE_TYPE_SPEAKER) == nullptr.
 */
HWTEST_F(AudioDeviceStatusExtendedTest, AudioDeviceStatus_017, TestSize.Level4)
{
    audioDeviceStatus_->audioConfigManager_.hasEarpiece_ = false;
    audioDeviceStatus_->AddEarpiece();

    audioDeviceStatus_->audioConfigManager_.hasEarpiece_ = true;
    audioDeviceStatus_->audioConnectedDevice_.connectedDevices_.clear();
    audioDeviceStatus_->AddEarpiece();
    EXPECT_FALSE(audioDeviceStatus_->audioDeviceManager_.connectedDevices_.empty());
}

/**
 * @tc.name  : Test AudioDeviceStatus.
 * @tc.number: AudioDeviceStatus_018
 * @tc.desc  : Test OnServiceConnected device.first != ClassType::TYPE_PRIMARY and result != SUCCESS.
 */
HWTEST_F(AudioDeviceStatusExtendedTest, AudioDeviceStatus_018, TestSize.Level4)
{
    std::list<AudioModuleInfo> deviceClassInfo = {};
    audioDeviceStatus_->audioConfigManager_.deviceClassInfo_[ClassType::TYPE_FILE_IO] = deviceClassInfo;
    AudioServiceIndex serviceIndex = AUDIO_SERVICE_INDEX;
    EXPECT_EQ(audioDeviceStatus_->OnServiceConnected(serviceIndex), ERROR);
}

/**
 * @tc.name  : Test AudioDeviceStatus.
 * @tc.number: AudioDeviceStatus_019
 * @tc.desc  : Test OnForcedDeviceSelected dec->deviceRole_ != DeviceRole::OUTPUT_DEVICE.
 */
HWTEST_F(AudioDeviceStatusExtendedTest, AudioDeviceStatus_019, TestSize.Level4)
{
    DeviceType devType = DEVICE_TYPE_SPEAKER;
    std::string macAddress = "00:11:22:33:44:55";
    auto dec1 = std::make_shared<AudioDeviceDescriptor>(devType, DeviceRole::OUTPUT_DEVICE);
    dec1->macAddress_ = macAddress;
    dec1->isEnable_ = false;
    auto dec2 = std::make_shared<AudioDeviceDescriptor>(devType, DeviceRole::OUTPUT_DEVICE);
    dec2->macAddress_ = macAddress;
    dec2->isEnable_ = false;
    audioDeviceStatus_->audioDeviceManager_.connectedDevices_ = { dec1, dec2 };
    audioDeviceStatus_->OnForcedDeviceSelected(devType, macAddress);
    auto audioDeviceDescriptors = audioDeviceStatus_->audioDeviceManager_.connectedDevices_;
    EXPECT_NE(audioDeviceStatus_->audioDeviceCommon_.DeviceParamsCheck(DeviceRole::OUTPUT_DEVICE,
        audioDeviceDescriptors), SUCCESS);
}

/**
 * @tc.name  : Test AudioDeviceStatus.
 * @tc.number: AudioDeviceStatus_020
 * @tc.desc  : Test OnDeviceStatusUpdated !isActualConnection
               && audioDeviceManager_.IsConnectedDevices(devDesc) == true.
 */
HWTEST_F(AudioDeviceStatusExtendedTest, AudioDeviceStatus_020, TestSize.Level4)
{
    AudioDeviceDescriptor updatedDesc;
    updatedDesc.deviceType_ = DEVICE_TYPE_SPEAKER;
    updatedDesc.networkId_ = "testNetworkId";
    updatedDesc.macAddress_ = "00:11:22:33:44:55";
    updatedDesc.connectState_ = CONNECTED;
    auto devDesc = make_shared<AudioDeviceDescriptor>(updatedDesc);
    audioDeviceStatus_->audioDeviceManager_.connectedDevices_.push_back(devDesc);
    DeviceType devType = DEVICE_TYPE_SPEAKER;
    std::string macAddress = "00:11:22:33:44:55";
    std::string deviceName = "testDeviceName";
    bool isActualConnection = false;
    AudioStreamInfo streamInfo;
    bool isConnected = true;
    audioDeviceStatus_->audioDeviceManager_.virtualDevices_.clear();
    audioDeviceStatus_->OnDeviceStatusUpdated(updatedDesc, devType, macAddress,
        deviceName, isActualConnection, streamInfo, isConnected);
    EXPECT_EQ(audioDeviceStatus_->audioDeviceManager_.virtualDevices_.size(), 1);
}

/**
 * @tc.name  : Test AudioDeviceStatus.
 * @tc.number: AudioDeviceStatus_021
 * @tc.desc  : Test OnDeviceInfoUpdated desc.deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO.
 */
HWTEST_F(AudioDeviceStatusExtendedTest, AudioDeviceStatus_021, TestSize.Level4)
{
    AudioDeviceDescriptor desc;
    desc.macAddress_ = "00:11:22:33:44:55";
    desc.isEnable_ = true;
    desc.deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    DeviceInfoUpdateCommand command = ENABLE_UPDATE;
    AudioStateManager::GetAudioStateManager().ownerUid_ = 0;
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>(desc);
    AudioStateManager::GetAudioStateManager().forcedDeviceMapList_.push_back({ {1, deviceDesc} });
    audioDeviceStatus_->OnDeviceInfoUpdated(desc, command);
    auto descs = AudioPolicyUtils::GetInstance().audioDeviceManager_.GetDevicesByFilter(
        DEVICE_TYPE_BLUETOOTH_SCO, DEVICE_ROLE_NONE, desc.macAddress_, "", SUSPEND_CONNECTED);
    EXPECT_TRUE(descs.empty());
}

/**
 * @tc.name  : Test AudioDeviceStatus.
 * @tc.number: AudioDeviceStatus_022
 * @tc.desc  : Test OnDeviceInfoUpdated command == ENABLE_UPDATE && !desc.isEnable_
 *             && desc.deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP
 *             && audioActiveDevice_.GetCurrentOutputDeviceMacAddr() == desc.macAddress_.
 */
HWTEST_F(AudioDeviceStatusExtendedTest, AudioDeviceStatus_022, TestSize.Level4)
{
    AudioStateManager::GetAudioStateManager().forcedDeviceMapList_.clear();
    AudioDeviceDescriptor desc;
    desc.macAddress_ = "00:11:22:33:44:55";
    desc.isEnable_ = false;
    desc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    DeviceInfoUpdateCommand command = ENABLE_UPDATE;
    audioDeviceStatus_->audioA2dpOffloadManager_.reset();
    audioDeviceStatus_->audioActiveDevice_.currentActiveDevice_.macAddress_ = desc.macAddress_;
    EXPECT_NO_THROW(audioDeviceStatus_->OnDeviceInfoUpdated(desc, command));
    audioDeviceStatus_->audioActiveDevice_.currentActiveDevice_.macAddress_ = "";
    EXPECT_NO_THROW(audioDeviceStatus_->OnDeviceInfoUpdated(desc, command));
    desc.deviceType_ = DEVICE_TYPE_EARPIECE;
    EXPECT_NO_THROW(audioDeviceStatus_->OnDeviceInfoUpdated(desc, command));
    desc.isEnable_ = true;
    EXPECT_NO_THROW(audioDeviceStatus_->OnDeviceInfoUpdated(desc, command));
    audioDeviceStatus_->audioActiveDevice_.currentActiveDevice_.macAddress_ = desc.macAddress_;
    EXPECT_NO_THROW(audioDeviceStatus_->OnDeviceInfoUpdated(desc, command));
    desc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    EXPECT_NO_THROW(audioDeviceStatus_->OnDeviceInfoUpdated(desc, command));
    desc.isEnable_ = false;
    desc.deviceType_ = DEVICE_TYPE_EARPIECE;
    EXPECT_NO_THROW(audioDeviceStatus_->OnDeviceInfoUpdated(desc, command));
    desc.isEnable_ = true;
    desc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioDeviceStatus_->audioActiveDevice_.currentActiveDevice_.macAddress_ = "";
    EXPECT_NO_THROW(audioDeviceStatus_->OnDeviceInfoUpdated(desc, command));

    command = USAGE_UPDATE;
    audioDeviceStatus_->audioA2dpOffloadManager_ = std::make_shared<AudioA2dpOffloadManager>();
    desc.deviceType_ = DEVICE_TYPE_EARPIECE;
    EXPECT_NO_THROW(audioDeviceStatus_->OnDeviceInfoUpdated(desc, command));
    audioDeviceStatus_->audioActiveDevice_.currentActiveDevice_.macAddress_ = desc.macAddress_;
    EXPECT_NO_THROW(audioDeviceStatus_->OnDeviceInfoUpdated(desc, command));
    desc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    EXPECT_NO_THROW(audioDeviceStatus_->OnDeviceInfoUpdated(desc, command));
    desc.isEnable_ = false;
    EXPECT_NO_THROW(audioDeviceStatus_->OnDeviceInfoUpdated(desc, command));
    desc.deviceType_ = DEVICE_TYPE_EARPIECE;
    EXPECT_NO_THROW(audioDeviceStatus_->OnDeviceInfoUpdated(desc, command));
    audioDeviceStatus_->audioActiveDevice_.currentActiveDevice_.macAddress_ = "";
    EXPECT_NO_THROW(audioDeviceStatus_->OnDeviceInfoUpdated(desc, command));
    desc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    EXPECT_NO_THROW(audioDeviceStatus_->OnDeviceInfoUpdated(desc, command));
    desc.isEnable_ = true;
    EXPECT_NO_THROW(audioDeviceStatus_->OnDeviceInfoUpdated(desc, command));
}

/**
 * @tc.name  : Test AudioDeviceStatus.
 * @tc.number: AudioDeviceStatus_023
 * @tc.desc  : Test DeactivateNearlinkDevice desc.macAddress_ == audioActiveDevice_.GetCurrentOutputDeviceMacAddr().
 */
HWTEST_F(AudioDeviceStatusExtendedTest, AudioDeviceStatus_023, TestSize.Level4)
{
    AudioDeviceDescriptor desc;
    desc.deviceType_ = DEVICE_TYPE_NEARLINK;
    desc.macAddress_ = "00:11:22:33:44:55";
    audioDeviceStatus_->audioActiveDevice_.currentActiveDevice_.macAddress_ = desc.macAddress_;
    audioDeviceStatus_->DeactivateNearlinkDevice(desc);
    EXPECT_EQ(desc.macAddress_, audioDeviceStatus_->audioActiveDevice_.GetCurrentOutputDeviceMacAddr());
}

/**
 * @tc.name  : Test AudioDeviceStatus.
 * @tc.number: AudioDeviceStatus_024
 * @tc.desc  : Test OnPreferredStateUpdated updateCommand != ENABLE_UPDATE.
 */
HWTEST_F(AudioDeviceStatusExtendedTest, AudioDeviceStatus_024, TestSize.Level4)
{
    AudioDeviceDescriptor desc;
    desc.isEnable_ = true;
    DeviceInfoUpdateCommand updateCommand = EXCEPTION_FLAG_UPDATE;
    AudioStreamDeviceChangeReasonExt reason;
    audioDeviceStatus_->OnPreferredStateUpdated(desc, updateCommand, reason);

    updateCommand = USAGE_UPDATE;
    audioDeviceStatus_->OnPreferredStateUpdated(desc, updateCommand, reason);
    EXPECT_EQ(reason, AudioStreamDeviceChangeReason::UNKNOWN);
}

/**
 * @tc.name  : Test AudioDeviceStatus.
 * @tc.number: AudioDeviceStatus_025
 * @tc.desc  : Test UpdateAllUserSelectDevice connectState_ == VIRTUAL_CONNECTED.
 */
HWTEST_F(AudioDeviceStatusExtendedTest, AudioDeviceStatus_025, TestSize.Level4)
{
    AudioDeviceDescriptor desc;
    desc.deviceType_ = DEVICE_TYPE_EARPIECE;
    desc.macAddress_ = "00:11:22:33:44:55";
    desc.connectState_ = VIRTUAL_CONNECTED;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> userSelectDeviceMap = {
        std::make_shared<AudioDeviceDescriptor>(desc),
        std::make_shared<AudioDeviceDescriptor>(desc),
        std::make_shared<AudioDeviceDescriptor>(desc),
        std::make_shared<AudioDeviceDescriptor>(desc),
    };
    
    std::shared_ptr<AudioDeviceDescriptor> selectDesc;
    audioDeviceStatus_->UpdateAllUserSelectDevice(userSelectDeviceMap, desc, selectDesc);
    EXPECT_EQ(audioDeviceStatus_->audioStateManager_.preferredMediaRenderDevice_->connectState_, VIRTUAL_CONNECTED);
}

/**
 * @tc.name  : Test AudioDeviceStatus.
 * @tc.number: AudioDeviceStatus_026
 * @tc.desc  : Test HandleOfflineDistributedDevice AudioPolicyUtils::GetInstance()
 *             .GetDeviceRole(deviceDesc->deviceType_) == DeviceRole::INPUT_DEVICE.
 */
HWTEST_F(AudioDeviceStatusExtendedTest, AudioDeviceStatus_026, TestSize.Level4)
{
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->networkId_ = "testNetworkId";
    desc->deviceType_ = DEVICE_TYPE_MIC;
    audioDeviceStatus_->audioConnectedDevice_.connectedDevices_ = { desc };
    audioDeviceStatus_->HandleOfflineDistributedDevice();
    EXPECT_TRUE(audioDeviceStatus_->remoteCapturerSwitch_);
}

/**
 * @tc.name  : Test AudioDeviceStatus.
 * @tc.number: AudioDeviceStatus_027
 * @tc.desc  : Test RestoreNewA2dpPort ioHandle == HDI_INVALID_ID.
 */
HWTEST_F(AudioDeviceStatusExtendedTest, AudioDeviceStatus_027, TestSize.Level4)
{
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs;
    AudioModuleInfo moduleInfo;
    std::string currentActivePort = "";
    AudioPolicyManagerFactory::GetAudioPolicyManager().ConnectServiceAdapter();
    EXPECT_EQ(audioDeviceStatus_->RestoreNewA2dpPort(streamDescs, moduleInfo, currentActivePort), ERROR);
}
} // namespace AudioStandard
} // namespace OHOS