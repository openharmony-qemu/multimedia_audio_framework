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

#include "audio_core_service_private_unit_test.h"
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

static const int32_t BLUETOOTH_FETCH_RESULT_CONTINUE = 1;
static const int32_t BLUETOOTH_FETCH_RESULT_ERROR = 2;

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_001
 * @tc.desc  : Test AudioCoreService::GetEncryptAddr()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::string addr = "abc";

    auto ret = audioCoreService->GetEncryptAddr(addr);
    EXPECT_EQ(ret, "");
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_002
 * @tc.desc  : Test AudioCoreService::GetEncryptAddr()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_002, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::string addr = "";

    auto ret = audioCoreService->GetEncryptAddr(addr);
    EXPECT_EQ(ret, "");
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_003
 * @tc.desc  : Test AudioCoreService::GetEncryptAddr()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_003, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::string addr = "12345678901234567";

    auto ret = audioCoreService->GetEncryptAddr(addr);
    EXPECT_EQ(ret, "123456**:**:**567");
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_004
 * @tc.desc  : Test AudioCoreService::HandleScoInputDeviceFetched()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_004, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    streamDesc->newDeviceDescs_.push_back(audioDeviceDescriptor);

    auto ret = audioCoreService->HandleScoInputDeviceFetched(streamDesc);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_005
 * @tc.desc  : Test AudioCoreService::ScoInputDeviceFetchedForRecongnition()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_005, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    bool handleFlag = true;
    std::string address = "abc";
    ConnectState connectState = CONNECTED;

    auto ret = audioCoreService->ScoInputDeviceFetchedForRecongnition(handleFlag, address, connectState);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_006
 * @tc.desc  : Test AudioCoreService::ScoInputDeviceFetchedForRecongnition()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_006, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    bool handleFlag = true;
    std::string address = "abc";
    ConnectState connectState = DEACTIVE_CONNECTED;

    auto ret = audioCoreService->ScoInputDeviceFetchedForRecongnition(handleFlag, address, connectState);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_007
 * @tc.desc  : Test AudioCoreService::ScoInputDeviceFetchedForRecongnition()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_007, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    bool handleFlag = false;
    std::string address = "abc";
    ConnectState connectState = DEACTIVE_CONNECTED;

    auto ret = audioCoreService->ScoInputDeviceFetchedForRecongnition(handleFlag, address, connectState);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_008
 * @tc.desc  : Test AudioCoreService::BluetoothScoFetch()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_008, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    streamDesc->newDeviceDescs_.push_back(audioDeviceDescriptor);

    audioCoreService->BluetoothScoFetch(streamDesc);
    EXPECT_EQ(Util::IsScoSupportSource(streamDesc->capturerInfo_.sourceType), true);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_009
 * @tc.desc  : Test AudioCoreService::CheckModemScene()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_009, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    EXPECT_NE(audioCoreService->pipeManager_, nullptr);

    audioCoreService->CheckModemScene(reason);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_010
 * @tc.desc  : Test AudioCoreService::CheckModemScene()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_010, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    EXPECT_NE(audioCoreService->pipeManager_, nullptr);
    audioCoreService->pipeManager_->modemCommunicationIdMap_.insert(std::make_pair(0, 0));

    audioCoreService->CheckModemScene(reason);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_011
 * @tc.desc  : Test AudioCoreService::HandleAudioCaptureState()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_011, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    AudioMode mode = AUDIO_MODE_RECORD;
    AudioStreamChangeInfo streamChangeInfo;
    streamChangeInfo.audioCapturerChangeInfo.capturerState = CAPTURER_RELEASED;
    streamChangeInfo.audioCapturerChangeInfo.capturerInfo.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;

    audioCoreService->HandleAudioCaptureState(mode, streamChangeInfo);
    EXPECT_NE(audioCoreService, nullptr);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_012
 * @tc.desc  : Test AudioCoreService::HandleAudioCaptureState()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_012, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    AudioMode mode = AUDIO_MODE_RECORD;
    AudioStreamChangeInfo streamChangeInfo;
    streamChangeInfo.audioCapturerChangeInfo.capturerState = CAPTURER_STOPPED;
    streamChangeInfo.audioCapturerChangeInfo.capturerInfo.sourceType = SOURCE_TYPE_MIC;

    audioCoreService->HandleAudioCaptureState(mode, streamChangeInfo);
    EXPECT_NE(audioCoreService, nullptr);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_013
 * @tc.desc  : Test AudioCoreService::HandleAudioCaptureState()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_013, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    AudioMode mode = AUDIO_MODE_RECORD;
    AudioStreamChangeInfo streamChangeInfo;
    streamChangeInfo.audioCapturerChangeInfo.capturerState = CAPTURER_NEW;

    audioCoreService->HandleAudioCaptureState(mode, streamChangeInfo);
    EXPECT_NE(audioCoreService, nullptr);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_014
 * @tc.desc  : Test AudioCoreService::HandleAudioCaptureState()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_014, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    AudioMode mode = AUDIO_MODE_PLAYBACK;
    AudioStreamChangeInfo streamChangeInfo;
    streamChangeInfo.audioCapturerChangeInfo.capturerState = CAPTURER_NEW;

    audioCoreService->HandleAudioCaptureState(mode, streamChangeInfo);
    EXPECT_NE(audioCoreService, nullptr);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_015
 * @tc.desc  : Test AudioCoreService::BluetoothDeviceFetchOutputHandle()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_015, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioDeviceDescriptor> desc = nullptr;
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    std::string encryptMacAddr = "abc";

    auto ret = audioCoreService->BluetoothDeviceFetchOutputHandle(desc, reason, encryptMacAddr);
    EXPECT_EQ(ret, BLUETOOTH_FETCH_RESULT_CONTINUE);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_016
 * @tc.desc  : Test AudioCoreService::BluetoothDeviceFetchOutputHandle()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_016, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    std::string encryptMacAddr = "abc";

    auto ret = audioCoreService->BluetoothDeviceFetchOutputHandle(desc, reason, encryptMacAddr);
    EXPECT_EQ(ret, BLUETOOTH_FETCH_RESULT_ERROR);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_017
 * @tc.desc  : Test AudioCoreService::BluetoothDeviceFetchOutputHandle()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_017, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    std::string encryptMacAddr = "abc";

    auto ret = audioCoreService->BluetoothDeviceFetchOutputHandle(desc, reason, encryptMacAddr);
    EXPECT_EQ(ret, BLUETOOTH_FETCH_RESULT_ERROR);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_018
 * @tc.desc  : Test AudioCoreService::ActivateA2dpDeviceWhenDescEnabled()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_018, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->isEnable_ = true;
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    AudioStreamDeviceChangeReasonExt reason(extEnum);

    auto ret = audioCoreService->ActivateA2dpDeviceWhenDescEnabled(desc, reason);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_019
 * @tc.desc  : Test AudioCoreService::ActivateA2dpDeviceWhenDescEnabled()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_019, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->isEnable_ = false;
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    AudioStreamDeviceChangeReasonExt reason(extEnum);

    auto ret = audioCoreService->ActivateA2dpDeviceWhenDescEnabled(desc, reason);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_020
 * @tc.desc  : Test AudioCoreService::ActivateA2dpDeviceWhenDescEnabled()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_020, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->isEnable_ = false;
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    AudioStreamDeviceChangeReasonExt reason(extEnum);

    auto ret = audioCoreService->ActivateA2dpDeviceWhenDescEnabled(desc, reason);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_021
 * @tc.desc  : Test AudioCoreService::SwitchActiveA2dpDevice()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_021, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    auto deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    EXPECT_NE(deviceDescriptor, nullptr);

    auto ret = audioCoreService->SwitchActiveA2dpDevice(deviceDescriptor);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_022
 * @tc.desc  : Test AudioCoreService::LoadA2dpModule()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_022, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    DeviceType deviceType = DEVICE_TYPE_INVALID;
    AudioStreamInfo audioStreamInfo;
    std::string networkId = "abc";
    std::string sinkName = "abc";
    SourceType sourceType = SOURCE_TYPE_MIC;

    auto ret = audioCoreService->LoadA2dpModule(deviceType, audioStreamInfo, networkId, sinkName, sourceType);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_023
 * @tc.desc  : Test AudioCoreService::ReloadA2dpAudioPort()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_023, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    AudioModuleInfo moduleInfo;
    moduleInfo.role = "abc";
    DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    AudioStreamInfo audioStreamInfo;
    std::string networkId = "abc";
    std::string sinkName = "abc";
    SourceType sourceType = SOURCE_TYPE_MIC;

    auto ret = audioCoreService->ReloadA2dpAudioPort(moduleInfo, deviceType, audioStreamInfo,
        networkId, sinkName, sourceType);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_024
 * @tc.desc  : Test AudioCoreService::ReloadA2dpAudioPort()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_024, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    AudioModuleInfo moduleInfo;
    moduleInfo.role = "sink";
    DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_A2DP_IN;
    AudioStreamInfo audioStreamInfo;
    std::string networkId = "abc";
    std::string sinkName = "abc";
    SourceType sourceType = SOURCE_TYPE_MIC;

    auto ret = audioCoreService->ReloadA2dpAudioPort(moduleInfo, deviceType, audioStreamInfo,
        networkId, sinkName, sourceType);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_025
 * @tc.desc  : Test AudioCoreService::GetA2dpModuleInfo()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_025, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    AudioModuleInfo moduleInfo;
    moduleInfo.role = "source";
    AudioStreamInfo audioStreamInfo;
    SourceType sourceType = SOURCE_TYPE_MIC;

    audioCoreService->GetA2dpModuleInfo(moduleInfo, audioStreamInfo, sourceType);
    EXPECT_NE(audioCoreService, nullptr);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_026
 * @tc.desc  : Test AudioCoreService::GetA2dpModuleInfo()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_026, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    AudioModuleInfo moduleInfo;
    moduleInfo.role = "sink";
    AudioStreamInfo audioStreamInfo;
    SourceType sourceType = SOURCE_TYPE_MIC;

    audioCoreService->GetA2dpModuleInfo(moduleInfo, audioStreamInfo, sourceType);
    EXPECT_NE(audioCoreService, nullptr);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_027
 * @tc.desc  : Test AudioCoreService::IsSameDevice()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_027, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->networkId_ = "networkId";
    desc->deviceType_ = DEVICE_TYPE_NONE;
    desc->macAddress_ = "macAddress";
    desc->connectState_ = CONNECTED;

    AudioDeviceDescriptor deviceInfo;
    deviceInfo.networkId_ = "networkId";
    deviceInfo.deviceType_ = DEVICE_TYPE_NONE;
    deviceInfo.macAddress_ = "macAddress";
    deviceInfo.connectState_ = CONNECTED;
    deviceInfo.descriptorType_ = AudioDeviceDescriptor::AUDIO_DEVICE_DESCRIPTOR;

    auto ret = audioCoreService->IsSameDevice(desc, deviceInfo);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_028
 * @tc.desc  : Test AudioCoreService::IsSameDevice()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_028, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->networkId_ = "networkId";
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    desc->macAddress_ = "macAddress";
    desc->connectState_ = CONNECTED;

    AudioDeviceDescriptor deviceInfo;
    deviceInfo.networkId_ = "networkId";
    deviceInfo.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    deviceInfo.macAddress_ = "macAddress";
    deviceInfo.connectState_ = CONNECTED;
    deviceInfo.descriptorType_ = AudioDeviceDescriptor::DEVICE_INFO;
    deviceInfo.a2dpOffloadFlag_ = A2DP_OFFLOAD;

    audioCoreService->audioA2dpOffloadFlag_.a2dpOffloadFlag_ = NO_A2DP_DEVICE;

    auto ret = audioCoreService->IsSameDevice(desc, deviceInfo);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_029
 * @tc.desc  : Test AudioCoreService::IsSameDevice()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_029, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->networkId_ = "networkId";
    desc->deviceType_ = DEVICE_TYPE_INVALID;
    desc->macAddress_ = "macAddress";
    desc->connectState_ = CONNECTED;

    AudioDeviceDescriptor deviceInfo;
    deviceInfo.networkId_ = "networkId";
    deviceInfo.deviceType_ = DEVICE_TYPE_INVALID;
    deviceInfo.macAddress_ = "macAddress";
    deviceInfo.connectState_ = CONNECTED;
    deviceInfo.descriptorType_ = AudioDeviceDescriptor::DEVICE_INFO;

    auto ret = audioCoreService->IsSameDevice(desc, deviceInfo);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_030
 * @tc.desc  : Test AudioCoreService::IsSameDevice()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_030, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->networkId_ = "networkId";
    desc->deviceType_ = DEVICE_TYPE_USB_HEADSET;
    desc->macAddress_ = "macAddress";
    desc->connectState_ = CONNECTED;
    desc->deviceRole_ = DEVICE_ROLE_NONE;

    AudioDeviceDescriptor deviceInfo;
    deviceInfo.networkId_ = "networkId";
    deviceInfo.deviceType_ = DEVICE_TYPE_USB_HEADSET;
    deviceInfo.macAddress_ = "macAddress";
    deviceInfo.connectState_ = CONNECTED;
    deviceInfo.descriptorType_ = AudioDeviceDescriptor::DEVICE_INFO;
    deviceInfo.deviceRole_ = DEVICE_ROLE_NONE;

    auto ret = audioCoreService->IsSameDevice(desc, deviceInfo);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_031
 * @tc.desc  : Test AudioCoreService::IsSameDevice()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_031, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->networkId_ = "networkId";
    desc->deviceType_ = DEVICE_TYPE_USB_HEADSET;
    desc->macAddress_ = "macAddress";
    desc->connectState_ = CONNECTED;

    AudioDeviceDescriptor deviceInfo;
    deviceInfo.networkId_ = "networkId";
    deviceInfo.deviceType_ = DEVICE_TYPE_USB_HEADSET;
    deviceInfo.macAddress_ = "macAddress";
    deviceInfo.connectState_ = SUSPEND_CONNECTED;

    auto ret = audioCoreService->IsSameDevice(desc, deviceInfo);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_032
 * @tc.desc  : Test AudioCoreService::IsSameDevice()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_032, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->networkId_ = "networkId";
    desc->deviceType_ = DEVICE_TYPE_USB_HEADSET;
    desc->macAddress_ = "macAddress";
    desc->connectState_ = CONNECTED;

    AudioDeviceDescriptor deviceInfo;
    deviceInfo.networkId_ = "networkId";
    deviceInfo.deviceType_ = DEVICE_TYPE_USB_HEADSET;
    deviceInfo.macAddress_ = "abc";
    deviceInfo.connectState_ = SUSPEND_CONNECTED;

    auto ret = audioCoreService->IsSameDevice(desc, deviceInfo);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_033
 * @tc.desc  : Test AudioCoreService::IsSameDevice()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_033, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->networkId_ = "networkId";
    desc->deviceType_ = DEVICE_TYPE_USB_HEADSET;
    desc->macAddress_ = "macAddress";
    desc->connectState_ = CONNECTED;

    AudioDeviceDescriptor deviceInfo;
    deviceInfo.networkId_ = "networkId";
    deviceInfo.deviceType_ = DEVICE_TYPE_DP;
    deviceInfo.macAddress_ = "abc";
    deviceInfo.connectState_ = SUSPEND_CONNECTED;

    auto ret = audioCoreService->IsSameDevice(desc, deviceInfo);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_034
 * @tc.desc  : Test AudioCoreService::IsSameDevice()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_034, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->networkId_ = "networkId";
    desc->deviceType_ = DEVICE_TYPE_USB_HEADSET;
    desc->macAddress_ = "macAddress";
    desc->connectState_ = CONNECTED;

    AudioDeviceDescriptor deviceInfo;
    deviceInfo.networkId_ = "abc";
    deviceInfo.deviceType_ = DEVICE_TYPE_DP;
    deviceInfo.macAddress_ = "abc";
    deviceInfo.connectState_ = SUSPEND_CONNECTED;

    auto ret = audioCoreService->IsSameDevice(desc, deviceInfo);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_037
 * @tc.desc  : Test AudioCoreService::ProcessOutputPipeNew()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_037, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->streamAction_ = AUDIO_STREAM_ACTION_NEW;
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);
    uint32_t flag = 0;
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);

    audioCoreService->ProcessOutputPipeNew(pipeInfo, flag, reason);
    EXPECT_NE(audioCoreService->pipeManager_, nullptr);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_038
 * @tc.desc  : Test AudioCoreService::ProcessOutputPipeNew()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_038, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->streamAction_ = AUDIO_STREAM_ACTION_MOVE;
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);
    uint32_t flag = 0;
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);

    audioCoreService->ProcessOutputPipeNew(pipeInfo, flag, reason);
    EXPECT_NE(audioCoreService->pipeManager_, nullptr);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_039
 * @tc.desc  : Test AudioCoreService::ProcessOutputPipeNew()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_039, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->streamAction_ = AUDIO_STREAM_ACTION_RECREATE;
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);
    uint32_t flag = 0;
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);

    audioCoreService->ProcessOutputPipeNew(pipeInfo, flag, reason);
    EXPECT_NE(audioCoreService->pipeManager_, nullptr);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_040
 * @tc.desc  : Test AudioCoreService::ProcessOutputPipeNew()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_040, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->streamAction_ = AUDIO_STREAM_ACTION_DEFAULT;
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);
    uint32_t flag = 0;
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);

    audioCoreService->ProcessOutputPipeNew(pipeInfo, flag, reason);
    EXPECT_NE(audioCoreService->pipeManager_, nullptr);
}
} // namespace AudioStandard
} // namespace OHOS