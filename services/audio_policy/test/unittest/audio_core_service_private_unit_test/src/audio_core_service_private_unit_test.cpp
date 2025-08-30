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
    EXPECT_EQ(ret, SUCCESS);
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
    EXPECT_EQ(ret, SUCCESS);
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
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> modemDescs;
    audioCoreService->CheckModemScene(modemDescs, reason);
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

    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = AudioStreamDeviceChangeReasonExt::ExtEnum::OVERRODE;
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    EXPECT_NE(audioCoreService->pipeManager_, nullptr);
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    audioCoreService->pipeManager_->modemCommunicationIdMap_.insert(std::make_pair(0, streamDesc));
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> modemDescs;
    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_PHONE_CALL;
    audioCoreService->CheckModemScene(modemDescs, reason);
    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_DEFAULT;
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

    std::shared_ptr<AudioStreamDescriptor> desc = nullptr;
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    std::string encryptMacAddr = "abc";

    auto ret = audioCoreService->BluetoothDeviceFetchOutputHandle(desc, reason, encryptMacAddr);
    EXPECT_EQ(ret, BLUETOOTH_FETCH_RESULT_ERROR);
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

    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    desc->newDeviceDescs_.push_back(deviceDesc);
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

    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    desc->newDeviceDescs_.push_back(deviceDesc);
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

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_043
 * @tc.desc  : Test AudioCoreService::BluetoothScoFetch
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_043, TestSize.Level1)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    streamDesc->newDeviceDescs_.push_back(audioDeviceDescriptor);

    auto audioCoreService = AudioCoreService::GetCoreService();

    audioCoreService->BluetoothScoFetch(streamDesc);

    EXPECT_NE(audioCoreService, nullptr);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_044
 * @tc.desc : Test AudioCoreService::HandleAudioCaptureState
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_044, TestSize.Level1)
{
    AudioMode mode = AUDIO_MODE_RECORD;
    AudioStreamChangeInfo streamChangeInfo;
    streamChangeInfo.audioCapturerChangeInfo.capturerState = CAPTURER_RELEASED;
    streamChangeInfo.audioCapturerChangeInfo.capturerInfo.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;

    auto audioCoreService = AudioCoreService::GetCoreService();

    audioCoreService->HandleAudioCaptureState(mode, streamChangeInfo);

    EXPECT_NE(audioCoreService, nullptr);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_045
 * @tc.desc : Test AudioCoreService::HandleAudioCaptureState
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_045, TestSize.Level1)
{
    AudioMode mode = AUDIO_MODE_RECORD;
    AudioStreamChangeInfo streamChangeInfo;
    streamChangeInfo.audioCapturerChangeInfo.capturerState = CAPTURER_RELEASED;
    streamChangeInfo.audioCapturerChangeInfo.capturerInfo.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;

    auto audioCoreService = AudioCoreService::GetCoreService();

    audioCoreService->HandleAudioCaptureState(mode, streamChangeInfo);

    EXPECT_NE(audioCoreService, nullptr);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_046
 * @tc.desc : Test AudioCoreService::HandleAudioCaptureState
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_046, TestSize.Level1)
{
    AudioMode mode = AUDIO_MODE_RECORD;
    AudioStreamChangeInfo streamChangeInfo;
    streamChangeInfo.audioCapturerChangeInfo.capturerState = CAPTURER_STOPPED;
    streamChangeInfo.audioCapturerChangeInfo.capturerInfo.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;

    auto audioCoreService = AudioCoreService::GetCoreService();

    audioCoreService->HandleAudioCaptureState(mode, streamChangeInfo);

    EXPECT_NE(audioCoreService, nullptr);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_047
 * @tc.desc : Test AudioCoreService::HandleAudioCaptureState
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_047, TestSize.Level1)
{
    AudioMode mode = AUDIO_MODE_RECORD;
    AudioStreamChangeInfo streamChangeInfo;
    streamChangeInfo.audioCapturerChangeInfo.capturerState = CAPTURER_INVALID;

    auto audioCoreService = AudioCoreService::GetCoreService();

    audioCoreService->HandleAudioCaptureState(mode, streamChangeInfo);

    EXPECT_NE(audioCoreService, nullptr);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_048
 * @tc.desc : Test AudioCoreService::HandleAudioCaptureState
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_048, TestSize.Level1)
{
    AudioMode mode = AUDIO_MODE_PLAYBACK;
    AudioStreamChangeInfo streamChangeInfo;
    streamChangeInfo.audioCapturerChangeInfo.capturerState = CAPTURER_STOPPED;

    auto audioCoreService = AudioCoreService::GetCoreService();

    audioCoreService->HandleAudioCaptureState(mode, streamChangeInfo);

    EXPECT_NE(audioCoreService, nullptr);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_049
 * @tc.desc : Test AudioCoreService::HasLowLatencyCapability
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_049, TestSize.Level1)
{
    auto audioCoreService = AudioCoreService::GetCoreService();

    bool result = audioCoreService->HasLowLatencyCapability(DEVICE_TYPE_INVALID, true);

    EXPECT_EQ(result, true);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_050
 * @tc.desc : Test AudioCoreService::HasLowLatencyCapability
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_050, TestSize.Level1)
{
    DeviceType deviceType = DEVICE_TYPE_EARPIECE;

    auto audioCoreService = AudioCoreService::GetCoreService();

    bool result = audioCoreService->HasLowLatencyCapability(deviceType, false);

    EXPECT_EQ(result, true);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_051
 * @tc.desc : Test AudioCoreService::HasLowLatencyCapability
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_051, TestSize.Level1)
{
    DeviceType deviceType = DEVICE_TYPE_SPEAKER;

    auto audioCoreService = AudioCoreService::GetCoreService();

    bool result = audioCoreService->HasLowLatencyCapability(deviceType, false);

    EXPECT_EQ(result, true);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_052
 * @tc.desc : Test AudioCoreService::HasLowLatencyCapability
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_052, TestSize.Level1)
{
    DeviceType deviceType = DEVICE_TYPE_WIRED_HEADSET;

    auto audioCoreService = AudioCoreService::GetCoreService();

    bool result = audioCoreService->HasLowLatencyCapability(deviceType, false);

    EXPECT_EQ(result, true);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_053
 * @tc.desc : Test AudioCoreService::HasLowLatencyCapability
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_053, TestSize.Level1)
{
    DeviceType deviceType = DEVICE_TYPE_WIRED_HEADPHONES;

    auto audioCoreService = AudioCoreService::GetCoreService();

    bool result = audioCoreService->HasLowLatencyCapability(deviceType, false);

    EXPECT_EQ(result, true);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_054
 * @tc.desc : Test AudioCoreService::HasLowLatencyCapability
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_054, TestSize.Level1)
{
    DeviceType deviceType = DEVICE_TYPE_USB_HEADSET;
    auto audioCoreService = AudioCoreService::GetCoreService();

    bool result = audioCoreService->HasLowLatencyCapability(deviceType, false);

    EXPECT_EQ(result, true);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_055
 * @tc.desc : Test AudioCoreService::HasLowLatencyCapability
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_055, TestSize.Level1)
{
    DeviceType deviceType = DEVICE_TYPE_DP;
    auto audioCoreService = AudioCoreService::GetCoreService();

    bool result = audioCoreService->HasLowLatencyCapability(deviceType, false);

    EXPECT_EQ(result, true);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_056
 * @tc.desc : Test AudioCoreService::HasLowLatencyCapability
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_056, TestSize.Level1)
{
    DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_SCO;
    auto audioCoreService = AudioCoreService::GetCoreService();

    bool result = audioCoreService->HasLowLatencyCapability(deviceType, false);

    EXPECT_EQ(result, false);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_057
 * @tc.desc : Test AudioCoreService::HasLowLatencyCapability
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_057, TestSize.Level1)
{
    DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    auto audioCoreService = AudioCoreService::GetCoreService();

    bool result = audioCoreService->HasLowLatencyCapability(deviceType, false);

    EXPECT_EQ(result, false);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_058
 * @tc.desc : Test AudioCoreService::HasLowLatencyCapability
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_058, TestSize.Level1)
{
    DeviceType deviceType = DEVICE_TYPE_DEFAULT;
    auto audioCoreService = AudioCoreService::GetCoreService();

    bool result = audioCoreService->HasLowLatencyCapability(deviceType, false);

    EXPECT_EQ(result, false);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_059
 * @tc.desc : Test AudioCoreService::GetRealUid
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_059, TestSize.Level1)
{
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->callerUid_ = 1013;
    streamDesc->appInfo_.appUid = 1013;

    auto audioCoreService = AudioCoreService::GetCoreService();

    int32_t result = audioCoreService->GetRealUid(streamDesc);

    EXPECT_EQ(result, 1013);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_060
 * @tc.desc : Test AudioCoreService::GetRealUid
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_060, TestSize.Level1)
{
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    auto audioCoreService = AudioCoreService::GetCoreService();

    streamDesc->callerUid_ = 0;

    int32_t result = audioCoreService->GetRealUid(streamDesc);

    EXPECT_EQ(result, 0);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_GetRealPid
 * @tc.desc : Test AudioCoreService::GetRealPid
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_GetRealPid, TestSize.Level1)
{
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->callerUid_ = 1013;
    streamDesc->callerPid_ = 1013;
    streamDesc->appInfo_.appPid = 1013;

    auto audioCoreService = AudioCoreService::GetCoreService();

    int32_t result = audioCoreService->GetRealPid(streamDesc);

    EXPECT_EQ(result, 1013);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_GetRealPid_02
 * @tc.desc : Test AudioCoreService::GetRealPid
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_GetRealPid_02, TestSize.Level1)
{
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    auto audioCoreService = AudioCoreService::GetCoreService();

    streamDesc->callerPid_ = 0;

    int32_t result = audioCoreService->GetRealPid(streamDesc);

    EXPECT_EQ(result, 0);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_061
 * @tc.desc : Test AudioCoreService::UpdateRendererInfoWhenNoPermission
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_061, TestSize.Level1)
{
    auto audioRendererChangeInfos = std::make_shared<AudioRendererChangeInfo>();
    bool hasSystemPermission = true;

    auto audioCoreService = AudioCoreService::GetCoreService();

    audioCoreService->UpdateRendererInfoWhenNoPermission(audioRendererChangeInfos, hasSystemPermission);

    EXPECT_NE(audioCoreService, nullptr);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_062
 * @tc.desc : Test AudioCoreService::UpdateRendererInfoWhenNoPermission
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_062, TestSize.Level1)
{
    auto audioRendererChangeInfos = std::make_shared<AudioRendererChangeInfo>();
    bool hasSystemPermission = false;

    auto audioCoreService = AudioCoreService::GetCoreService();

    audioCoreService->UpdateRendererInfoWhenNoPermission(audioRendererChangeInfos, hasSystemPermission);

    EXPECT_NE(audioCoreService, nullptr);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_063
 * @tc.desc : Test AudioCoreService::UpdateRendererInfoWhenNoPermission
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_063, TestSize.Level1)
{
    auto audioCapturerChangeInfos = std::make_shared<AudioCapturerChangeInfo>();
    bool hasSystemPermission = true;

    auto audioCoreService = AudioCoreService::GetCoreService();

    audioCoreService->UpdateCapturerInfoWhenNoPermission(audioCapturerChangeInfos, hasSystemPermission);

    EXPECT_NE(audioCoreService, nullptr);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_064
 * @tc.desc : Test AudioCoreService::UpdateRendererInfoWhenNoPermission
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_064, TestSize.Level1)
{
    auto audioCapturerChangeInfos = std::make_shared<AudioCapturerChangeInfo>();
    bool hasSystemPermission = false;

    auto audioCoreService = AudioCoreService::GetCoreService();

    audioCoreService->UpdateCapturerInfoWhenNoPermission(audioCapturerChangeInfos, hasSystemPermission);

    EXPECT_NE(audioCoreService, nullptr);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_065
 * @tc.desc : Test AudioCoreService::GetFastControlParam
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_065, TestSize.Level1)
{
    auto audioCoreService = AudioCoreService::GetCoreService();
    audioCoreService->isFastControlled_ = true;

    SetSysPara("persist.multimedia.audioflag.fastcontrolled", 0);

    bool result = audioCoreService->GetFastControlParam();

    EXPECT_EQ(result, false);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_066
 * @tc.desc : Test AudioCoreService::GetFastControlParam
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_066, TestSize.Level1)
{
    auto audioCoreService = AudioCoreService::GetCoreService();
    audioCoreService->isFastControlled_ = true;

    SetSysPara("persist.multimedia.audioflag.fastcontrolled", 1);

    bool result = audioCoreService->GetFastControlParam();

    EXPECT_EQ(result, true);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_067
 * @tc.desc : Test AudioCoreService::NeedRehandleA2DPDevice
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_067, TestSize.Level1)
{
    auto desc = std::make_shared<AudioDeviceDescriptor>();

    auto audioCoreService = AudioCoreService::GetCoreService();
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;

    bool result = audioCoreService->NeedRehandleA2DPDevice(desc);

    EXPECT_EQ(result, false);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_068
 * @tc.desc : Test AudioCoreService::NeedRehandleA2DPDevice
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_068, TestSize.Level1)
{
    auto desc = std::make_shared<AudioDeviceDescriptor>();

    auto audioCoreService = AudioCoreService::GetCoreService();
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    std::string moduleName = BLUETOOTH_SPEAKER;
    AudioIOHandle moduleId = 0;

    audioCoreService->audioIOHandleMap_.AddIOHandleInfo(moduleName, moduleId);
    bool result = audioCoreService->NeedRehandleA2DPDevice(desc);

    EXPECT_EQ(result, false);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_069
 * @tc.desc : Test AudioCoreService::NeedRehandleA2DPDevice
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_069, TestSize.Level1)
{
    auto desc = std::make_shared<AudioDeviceDescriptor>();

    auto audioCoreService = AudioCoreService::GetCoreService();
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    std::string moduleName = BLUETOOTH_MIC;
    AudioIOHandle moduleId = 0;

    audioCoreService->audioIOHandleMap_.AddIOHandleInfo(moduleName, moduleId);
    bool result = audioCoreService->NeedRehandleA2DPDevice(desc);

    EXPECT_EQ(result, false);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_070
 * @tc.desc : Test AudioCoreService::TriggerRecreateRendererStreamCallback
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_070, TestSize.Level1)
{
    shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->callerPid_ = 0;
    streamDesc->sessionId_ = 0;
    streamDesc->routeFlag_ = true;
    streamDesc->rendererInfo_.isOffloadAllowed = true;
    streamDesc->streamInfo_.channels = STEREO;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MUSIC;
    streamDesc->rendererInfo_.playerType = PLAYER_TYPE_SOUND_POOL;
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    streamDesc->oldDeviceDescs_.push_back(deviceDesc);
    streamDesc->newDeviceDescs_.push_back(deviceDesc);
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum =
        AudioStreamDeviceChangeReasonExt::ExtEnum::SET_DEFAULT_OUTPUT_DEVICE;
    AudioStreamDeviceChangeReasonExt reason(extEnum);

    auto audioCoreService = AudioCoreService::GetCoreService();
    audioCoreService->SetCallbackHandler(nullptr);

    audioCoreService->TriggerRecreateRendererStreamCallback(streamDesc, reason);
    EXPECT_EQ(audioCoreService->audioPolicyServerHandler_, nullptr);

    bool isSupportLowPower = audioCoreService->IsStreamSupportLowpower(streamDesc);
    EXPECT_EQ(isSupportLowPower, false);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_071
 * @tc.desc : Test AudioCoreService::TriggerRecreateRendererStreamCallback
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_071, TestSize.Level1)
{
    shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->callerPid_ = 0;
    streamDesc->sessionId_ = 0;
    streamDesc->routeFlag_ = true;
    streamDesc->rendererInfo_.isOffloadAllowed = true;
    streamDesc->streamInfo_.channels = STEREO;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MUSIC;
    streamDesc->rendererInfo_.playerType = PLAYER_TYPE_SOUND_POOL;
    std::shared_ptr<AudioDeviceDescriptor> oldDeviceDesc = std::make_shared<AudioDeviceDescriptor>();
    streamDesc->oldDeviceDescs_.push_back(oldDeviceDesc);
    std::shared_ptr<AudioDeviceDescriptor> newDeviceDesc = std::make_shared<AudioDeviceDescriptor>();
    newDeviceDesc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    streamDesc->newDeviceDescs_.push_back(newDeviceDesc);
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum =
        AudioStreamDeviceChangeReasonExt::ExtEnum::SET_DEFAULT_OUTPUT_DEVICE;
    AudioStreamDeviceChangeReasonExt reason(extEnum);

    auto audioCoreService = AudioCoreService::GetCoreService();
    std::shared_ptr<AudioPolicyServerHandler> handler = std::make_shared<AudioPolicyServerHandler>();
    audioCoreService->SetCallbackHandler(handler);

    audioCoreService->TriggerRecreateRendererStreamCallback(streamDesc, reason);
    EXPECT_NE(audioCoreService->audioPolicyServerHandler_, nullptr);

    bool isSupportLowPower = audioCoreService->IsStreamSupportLowpower(streamDesc);
    EXPECT_EQ(isSupportLowPower, false);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_072
 * @tc.desc : Test AudioCoreService::TriggerRecreateCapturerStreamCallback
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_072, TestSize.Level1)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = 0,
    streamDesc->callerUid_ = 0;
    streamDesc->appInfo_.appUid = 0;
    streamDesc->appInfo_.appPid = 0;
    streamDesc->appInfo_.appTokenId = 0;
    streamDesc->streamStatus_ = STREAM_STATUS_NEW;
    streamDesc->routeFlag_ = true;

    auto audioCoreService = AudioCoreService::GetCoreService();
    audioCoreService->SetCallbackHandler(nullptr);

    audioCoreService->TriggerRecreateCapturerStreamCallback(streamDesc);

    EXPECT_EQ(audioCoreService->audioPolicyServerHandler_, nullptr);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_073
 * @tc.desc : Test AudioCoreService::TriggerRecreateCapturerStreamCallback
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_073, TestSize.Level1)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = 123456,
    streamDesc->callerUid_ = 0;
    streamDesc->appInfo_.appUid = 0;
    streamDesc->appInfo_.appPid = 0;
    streamDesc->appInfo_.appTokenId = 0;
    streamDesc->streamStatus_ = STREAM_STATUS_NEW;
    streamDesc->routeFlag_ = true;

    auto audioCoreService = AudioCoreService::GetCoreService();
    std::shared_ptr<AudioPolicyServerHandler> handler = std::make_shared<AudioPolicyServerHandler>();
    audioCoreService->SetCallbackHandler(handler);
    
    audioCoreService->TriggerRecreateCapturerStreamCallback(streamDesc);
    EXPECT_NE(audioCoreService->audioPolicyServerHandler_, nullptr);

    bool ret = SwitchStreamUtil::RemoveAllRecordBySessionId(123456);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_074
 * @tc.desc : Test AudioCoreService::HandleStreamStatusToCapturerState
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_074, TestSize.Level2)
{
    auto audioCoreService = AudioCoreService::GetCoreService();
    
    CapturerState state = audioCoreService->HandleStreamStatusToCapturerState(STREAM_STATUS_NEW);
    EXPECT_EQ(state, CAPTURER_PREPARED);

    state = audioCoreService->HandleStreamStatusToCapturerState(STREAM_STATUS_STARTED);
    EXPECT_EQ(state, CAPTURER_RUNNING);

    state = audioCoreService->HandleStreamStatusToCapturerState(STREAM_STATUS_PAUSED);
    EXPECT_EQ(state, CAPTURER_PAUSED);

    state = audioCoreService->HandleStreamStatusToCapturerState(STREAM_STATUS_STOPPED);
    EXPECT_EQ(state, CAPTURER_STOPPED);

    state = audioCoreService->HandleStreamStatusToCapturerState(STREAM_STATUS_RELEASED);
    EXPECT_EQ(state, CAPTURER_RELEASED);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_075
 * @tc.desc  : Test AudioCoreService::ProcessOutputPipeNew()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_075, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
 
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->streamAction_ = AUDIO_STREAM_ACTION_MOVE;
    audioStreamDescriptor->streamStatus_ = STREAM_STATUS_STARTED;
    audioStreamDescriptor->appInfo_.appUid = AUDIO_ID;
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);
    uint32_t flag = 0;
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
 
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    audioStreamDescriptor->oldDeviceDescs_.push_back(audioDeviceDescriptor);
 
    audioCoreService->ProcessOutputPipeNew(pipeInfo, flag, reason);
    EXPECT_NE(audioCoreService->pipeManager_, nullptr);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_100
 * @tc.desc  : Test AudioCoreService::ProcessOutputPipeUpdate()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_100, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->streamAction_ = AUDIO_STREAM_ACTION_NEW;
    audioStreamDescriptor->routeFlag_ = AUDIO_OUTPUT_FLAG_DIRECT;
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);
    uint32_t flag = 0;
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);

    audioCoreService->ProcessOutputPipeUpdate(pipeInfo, flag, reason);
    EXPECT_EQ(flag, AUDIO_OUTPUT_FLAG_DIRECT);
    ASSERT_NE(audioCoreService->pipeManager_, nullptr);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_101
 * @tc.desc  : Test AudioCoreService::ProcessOutputPipeUpdate()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_101, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

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

    audioCoreService->ProcessOutputPipeUpdate(pipeInfo, flag, reason);
    ASSERT_NE(audioCoreService->pipeManager_, nullptr);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_102
 * @tc.desc  : Test AudioCoreService::ProcessOutputPipeUpdate()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_102, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->streamAction_ = AUDIO_STREAM_ACTION_RECREATE;
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);
    pipeInfo->moduleInfo_.name = BLUETOOTH_MIC;

    uint32_t flag = 0;

    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    AudioStreamDeviceChangeReasonExt reason(extEnum);

    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);

    audioCoreService->ProcessOutputPipeUpdate(pipeInfo, flag, reason);
    ASSERT_NE(audioCoreService->pipeManager_, nullptr);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_103
 * @tc.desc  : Test AudioCoreService::ProcessOutputPipeUpdate()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_103, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->streamAction_ = static_cast<AudioStreamAction>(5);
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);
    pipeInfo->moduleInfo_.name = OFFLOAD_PRIMARY_SPEAKER;

    uint32_t flag = 0;

    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    AudioStreamDeviceChangeReasonExt reason(extEnum);

    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);

    audioCoreService->ProcessOutputPipeUpdate(pipeInfo, flag, reason);
    ASSERT_NE(audioCoreService->pipeManager_, nullptr);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_104
 * @tc.desc  : Test AudioCoreService::ProcessInputPipeNew()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_104, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->streamAction_ = AUDIO_STREAM_ACTION_NEW;
    audioStreamDescriptor->routeFlag_ = AUDIO_OUTPUT_FLAG_DIRECT;
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);

    uint32_t flag = 0;

    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);

    audioCoreService->ProcessInputPipeNew(pipeInfo, flag);
    EXPECT_EQ(flag, AUDIO_OUTPUT_FLAG_DIRECT);
    ASSERT_NE(audioCoreService->pipeManager_, nullptr);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_105
 * @tc.desc  : Test AudioCoreService::ProcessInputPipeNew()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_105, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->streamAction_ = AUDIO_STREAM_ACTION_DEFAULT;
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);

    uint32_t flag = 0;

    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);

    audioCoreService->ProcessInputPipeNew(pipeInfo, flag);
    ASSERT_NE(audioCoreService->pipeManager_, nullptr);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_106
 * @tc.desc  : Test AudioCoreService::ProcessInputPipeNew()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_106, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->streamAction_ = AUDIO_STREAM_ACTION_RECREATE;
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);

    uint32_t flag = 0;

    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);

    audioCoreService->ProcessInputPipeNew(pipeInfo, flag);
    ASSERT_NE(audioCoreService->pipeManager_, nullptr);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_107
 * @tc.desc  : Test AudioCoreService::ProcessInputPipeNew()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_107, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->streamAction_ = static_cast<AudioStreamAction>(5);
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);

    uint32_t flag = 0;

    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);

    audioCoreService->ProcessInputPipeNew(pipeInfo, flag);
    ASSERT_NE(audioCoreService->pipeManager_, nullptr);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_108
 * @tc.desc  : Test AudioCoreService::ProcessInputPipeUpdate()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_108, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->streamAction_ = AUDIO_STREAM_ACTION_NEW;
    audioStreamDescriptor->routeFlag_ = AUDIO_OUTPUT_FLAG_DIRECT;
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);

    uint32_t flag = 0;

    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);

    audioCoreService->ProcessInputPipeUpdate(pipeInfo, flag);
    EXPECT_EQ(flag, AUDIO_OUTPUT_FLAG_DIRECT);
    ASSERT_NE(audioCoreService->pipeManager_, nullptr);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_109
 * @tc.desc  : Test AudioCoreService::ProcessInputPipeUpdate()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_109, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->streamAction_ = AUDIO_STREAM_ACTION_DEFAULT;
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);

    uint32_t flag = 0;

    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);

    audioCoreService->ProcessInputPipeUpdate(pipeInfo, flag);
    ASSERT_NE(audioCoreService->pipeManager_, nullptr);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_110
 * @tc.desc  : Test AudioCoreService::ProcessInputPipeUpdate()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_110, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->streamAction_ = AUDIO_STREAM_ACTION_RECREATE;
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);

    uint32_t flag = 0;

    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);

    audioCoreService->ProcessInputPipeUpdate(pipeInfo, flag);
    ASSERT_NE(audioCoreService->pipeManager_, nullptr);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_111
 * @tc.desc  : Test AudioCoreService::ProcessInputPipeUpdate()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_111, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->streamAction_ = static_cast<AudioStreamAction>(5);
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);

    uint32_t flag = 0;

    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);

    audioCoreService->ProcessInputPipeUpdate(pipeInfo, flag);
    ASSERT_NE(audioCoreService->pipeManager_, nullptr);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_112
 * @tc.desc  : Test AudioCoreService::SwitchActiveA2dpDevice()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_112, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    auto deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(deviceDescriptor, nullptr);

    A2dpDeviceConfigInfo a2dpDeviceConfigInfo;
    audioCoreService->audioA2dpDevice_.connectedA2dpDeviceMap_.insert({"00:00:00:00:00:00", a2dpDeviceConfigInfo});
    deviceDescriptor->macAddress_ = "00:00:00:00:00:00";
    AudioIOHandle audioIOHandle;
    audioCoreService->audioIOHandleMap_.IOHandles_.insert({BLUETOOTH_SPEAKER, audioIOHandle});

    auto ret = audioCoreService->SwitchActiveA2dpDevice(deviceDescriptor);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_113
 * @tc.desc  : Test AudioCoreService::SwitchActiveA2dpDevice()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_113, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    auto deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(deviceDescriptor, nullptr);

    A2dpDeviceConfigInfo a2dpDeviceConfigInfo;
    audioCoreService->audioA2dpDevice_.connectedA2dpDeviceMap_.insert({"00:00:00:00:00:00", a2dpDeviceConfigInfo});
    deviceDescriptor->macAddress_ = "00:00:00:00:00:00";
    AudioIOHandle audioIOHandle;
    audioCoreService->audioIOHandleMap_.IOHandles_.insert({"abc", audioIOHandle});

    auto ret = audioCoreService->SwitchActiveA2dpDevice(deviceDescriptor);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_114
 * @tc.desc  : Test AudioCoreService::SwitchActiveA2dpDevice()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_114, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    auto deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(deviceDescriptor, nullptr);

    A2dpDeviceConfigInfo a2dpDeviceConfigInfo;
    audioCoreService->audioA2dpDevice_.connectedA2dpDeviceMap_.insert({"abc", a2dpDeviceConfigInfo});
    deviceDescriptor->macAddress_ = "abc";
    AudioIOHandle audioIOHandle;
    audioCoreService->audioIOHandleMap_.IOHandles_.insert({BLUETOOTH_SPEAKER, audioIOHandle});

    auto ret = audioCoreService->SwitchActiveA2dpDevice(deviceDescriptor);
    EXPECT_NE(Bluetooth::AudioA2dpManager::GetActiveA2dpDevice(), "00:00:00:00:00:00");
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_115
 * @tc.desc  : Test AudioCoreService::MoveToNewInputDevice()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_115, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    ASSERT_NE(streamDesc, nullptr);

    streamDesc->oldDeviceDescs_.clear();
    EXPECT_EQ(streamDesc->oldDeviceDescs_.size(), 0);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    streamDesc->newDeviceDescs_.push_back(audioDeviceDescriptor);

    audioCoreService->MoveToNewInputDevice(streamDesc);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_116
 * @tc.desc  : Test AudioCoreService::MoveToNewInputDevice()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_116, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    ASSERT_NE(streamDesc, nullptr);

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    streamDesc->oldDeviceDescs_.push_back(audioDeviceDescriptor);
    EXPECT_NE(streamDesc->oldDeviceDescs_.size(), 0);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor2 = std::make_shared<AudioDeviceDescriptor>();
    streamDesc->newDeviceDescs_.push_back(audioDeviceDescriptor2);

    audioCoreService->MoveToNewInputDevice(streamDesc);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_117
 * @tc.desc  : Test AudioCoreService::IsNewDevicePlaybackSupported()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_117, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = nullptr;
    bool ret = audioCoreService->IsNewDevicePlaybackSupported(streamDesc);
    EXPECT_EQ(ret, false);

    streamDesc = std::make_shared<AudioStreamDescriptor>();
    ASSERT_NE(streamDesc, nullptr);
    ret = audioCoreService->IsNewDevicePlaybackSupported(streamDesc);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_118
 * @tc.desc  : Test AudioCoreService::IsNewDevicePlaybackSupported()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_118, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    ASSERT_NE(streamDesc, nullptr);
    streamDesc->newDeviceDescs_.push_back(nullptr);
    bool ret = audioCoreService->IsNewDevicePlaybackSupported(streamDesc);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_119
 * @tc.desc  : Test AudioCoreService::IsNewDevicePlaybackSupported()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_119, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    ASSERT_NE(streamDesc, nullptr);
    std::shared_ptr<AudioDeviceDescriptor> newDeviceDesc = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(newDeviceDesc, nullptr);
    newDeviceDesc->deviceType_ = DEVICE_TYPE_SPEAKER;
    streamDesc->newDeviceDescs_.push_back(newDeviceDesc);
    streamDesc->streamInfo_.encoding = ENCODING_EAC3;

    bool ret = audioCoreService->IsNewDevicePlaybackSupported(streamDesc);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_120
 * @tc.desc  : Test AudioCoreService::IsNewDevicePlaybackSupported()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_120, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    ASSERT_NE(streamDesc, nullptr);
    std::shared_ptr<AudioDeviceDescriptor> newDeviceDesc = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(newDeviceDesc, nullptr);
    newDeviceDesc->deviceType_ = DEVICE_TYPE_HDMI;
    streamDesc->newDeviceDescs_.push_back(newDeviceDesc);
    streamDesc->streamInfo_.encoding = ENCODING_EAC3;

    bool ret = audioCoreService->IsNewDevicePlaybackSupported(streamDesc);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_121
 * @tc.desc  : Test AudioCoreService::UpdateInputDeviceWhenStopping
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_121, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    int32_t uid = getuid();
    std::vector<uint32_t> sessionIDSet  = audioCoreService->streamCollector_.GetAllCapturerSessionIDForUID(uid);
    std::shared_ptr<AudioDeviceDescriptor> device;

    for (const auto &sessionID : sessionIDSet) {
        audioCoreService->audioDeviceManager_.SetInputDevice(DEVICE_TYPE_MIC, sessionID, SOURCE_TYPE_MIC, 1);
        device = audioCoreService->audioDeviceManager_.GetSelectedCaptureDevice(sessionID);
        EXPECT_EQ(device->deviceType_ == DEVICE_TYPE_MIC, true);
    }

    audioCoreService->UpdateInputDeviceWhenStopping(uid);

    for (const auto &sessionID : sessionIDSet) {
        device = audioCoreService->audioDeviceManager_.GetSelectedCaptureDevice(sessionID);
        EXPECT_EQ(device->deviceType_ != DEVICE_TYPE_MIC, true);
    }
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: IsFastAllowedTest_001
 * @tc.desc  : Test AudioCoreService::IsFastAllowed, return true when bundleName is null.
 */
HWTEST(AudioCoreServicePrivateTest, IsFastAllowedTest_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    std::string bundleName = "";
    EXPECT_EQ(audioCoreService->IsFastAllowed(bundleName), true);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: IsFastAllowedTest_002
 * @tc.desc  : Test AudioCoreService::IsFastAllowed, return true when bundleName is normal app.
 */
HWTEST(AudioCoreServicePrivateTest, IsFastAllowedTest_002, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    std::string bundleName = "com.example.app";
    streamDesc->SetBunduleName(bundleName);
    EXPECT_EQ(audioCoreService->IsFastAllowed(streamDesc->bundleName_), true);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: HandleFetchOutputWhenNoRunningStream_001
 * @tc.desc  : Test AudioCoreService::HandleFetchOutputWhenNoRunningStream, fetch output when no running stream.
 */
HWTEST(AudioCoreServicePrivateTest, HandleFetchOutputWhenNoRunningStream_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    auto ret = audioCoreService->HandleFetchOutputWhenNoRunningStream(AudioStreamDeviceChangeReason::UNKNOWN);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: CheckModemScene_001
 * @tc.desc  : Test AudioCoreService::CheckModemScene, set streamStatus to started if scene is AUDIO_SCENE_PHONE_CALL
 */
HWTEST(AudioCoreServicePrivateTest, CheckModemScene_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_PHONE_CALL;
    AudioDeviceDescriptor curDesc(DeviceType::DEVICE_TYPE_SPEAKER, DeviceRole::OUTPUT_DEVICE);
    audioCoreService->audioActiveDevice_.SetCurrentOutputDevice(curDesc);
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = AudioStreamDeviceChangeReasonExt::ExtEnum::OVERRODE;
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    audioCoreService->pipeManager_->modemCommunicationIdMap_.insert(std::make_pair(0, streamDesc));
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> modemDescs;
    audioCoreService->CheckModemScene(modemDescs, reason);
    EXPECT_NE(audioCoreService->pipeManager_->modemCommunicationIdMap_[0], nullptr);
    AudioStreamStatus status = audioCoreService->pipeManager_->modemCommunicationIdMap_[0]->streamStatus_;
    EXPECT_EQ(status, STREAM_STATUS_STARTED);
    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_DEFAULT;
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: CheckModemScene_002
 * @tc.desc  : Test AudioCoreService::CheckModemScene, set streamStatus to stopped if scene is AUDIO_SCENE_DEFAULT
 */
HWTEST(AudioCoreServicePrivateTest, CheckModemScene_002, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_DEFAULT;
    AudioDeviceDescriptor curDesc(DeviceType::DEVICE_TYPE_SPEAKER, DeviceRole::OUTPUT_DEVICE);
    audioCoreService->audioActiveDevice_.SetCurrentOutputDevice(curDesc);
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    audioCoreService->pipeManager_->modemCommunicationIdMap_.insert(std::make_pair(0, streamDesc));
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> modemDescs;
    audioCoreService->CheckModemScene(modemDescs, reason);
    EXPECT_NE(audioCoreService->pipeManager_->modemCommunicationIdMap_[0], nullptr);
    AudioStreamStatus status = audioCoreService->pipeManager_->modemCommunicationIdMap_[0]->streamStatus_;
    EXPECT_EQ(status, STREAM_STATUS_STOPPED);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: CheckModemScene_003
 * @tc.desc  : Test AudioCoreService::CheckModemScene, set streamStatus to started if scene is AUDIO_SCENE_PHONE_CALL
 */
HWTEST(AudioCoreServicePrivateTest, CheckModemScene_003, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_PHONE_CALL;
    AudioDeviceDescriptor curDesc(DeviceType::DEVICE_TYPE_SPEAKER, DeviceRole::OUTPUT_DEVICE);
    audioCoreService->audioActiveDevice_.SetCurrentOutputDevice(curDesc);
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    audioCoreService->pipeManager_->modemCommunicationIdMap_.insert(std::make_pair(0, streamDesc));
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> modemDescs;
    audioCoreService->CheckModemScene(modemDescs, reason);
    EXPECT_NE(audioCoreService->pipeManager_->modemCommunicationIdMap_[0], nullptr);
    AudioStreamStatus status = audioCoreService->pipeManager_->modemCommunicationIdMap_[0]->streamStatus_;
    EXPECT_EQ(status, STREAM_STATUS_STARTED);
    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_DEFAULT;
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: CheckModemScene_004
 * @tc.desc  : Test AudioCoreService::CheckModemScene, set streamStatus to stopped if scene is AUDIO_SCENE_DEFAULT
 */
HWTEST(AudioCoreServicePrivateTest, CheckModemScene_004, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_DEFAULT;
    AudioDeviceDescriptor curDesc(DeviceType::DEVICE_TYPE_SPEAKER, DeviceRole::OUTPUT_DEVICE);
    audioCoreService->audioActiveDevice_.SetCurrentOutputDevice(curDesc);
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = AudioStreamDeviceChangeReasonExt::ExtEnum::OVERRODE;
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    audioCoreService->pipeManager_->modemCommunicationIdMap_.insert(std::make_pair(0, streamDesc));
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> modemDescs;
    audioCoreService->CheckModemScene(modemDescs, reason);
    EXPECT_NE(audioCoreService->pipeManager_->modemCommunicationIdMap_[0], nullptr);
    AudioStreamStatus status = audioCoreService->pipeManager_->modemCommunicationIdMap_[0]->streamStatus_;
    EXPECT_EQ(status, STREAM_STATUS_STOPPED);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: CheckModemScene_005
 * @tc.desc  : Test AudioCoreService::CheckModemScene, set streamStatus to started if scene is AUDIO_SCENE_PHONE_CALL
 */
HWTEST(AudioCoreServicePrivateTest, CheckModemScene_005, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_PHONE_CALL;
    AudioDeviceDescriptor curDesc(DeviceType::DEVICE_TYPE_EARPIECE, DeviceRole::OUTPUT_DEVICE);
    audioCoreService->audioActiveDevice_.SetCurrentOutputDevice(curDesc);
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = AudioStreamDeviceChangeReasonExt::ExtEnum::OVERRODE;
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    audioCoreService->pipeManager_->modemCommunicationIdMap_.insert(std::make_pair(0, streamDesc));
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> modemDescs;
    audioCoreService->CheckModemScene(modemDescs, reason);
    EXPECT_NE(audioCoreService->pipeManager_->modemCommunicationIdMap_[0], nullptr);
    AudioStreamStatus status = audioCoreService->pipeManager_->modemCommunicationIdMap_[0]->streamStatus_;
    EXPECT_EQ(status, STREAM_STATUS_STARTED);
    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_DEFAULT;
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: CheckModemScene_006
 * @tc.desc  : Test AudioCoreService::CheckModemScene, set streamStatus to stopped if scene is AUDIO_SCENE_DEFAULT
 */
HWTEST(AudioCoreServicePrivateTest, CheckModemScene_006, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_DEFAULT;
    AudioDeviceDescriptor curDesc(DeviceType::DEVICE_TYPE_EARPIECE, DeviceRole::OUTPUT_DEVICE);
    audioCoreService->audioActiveDevice_.SetCurrentOutputDevice(curDesc);
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    audioCoreService->pipeManager_->modemCommunicationIdMap_.insert(std::make_pair(0, streamDesc));
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> modemDescs;
    audioCoreService->CheckModemScene(modemDescs, reason);
    EXPECT_NE(audioCoreService->pipeManager_->modemCommunicationIdMap_[0], nullptr);
    AudioStreamStatus status = audioCoreService->pipeManager_->modemCommunicationIdMap_[0]->streamStatus_;
    EXPECT_EQ(status, STREAM_STATUS_STOPPED);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: CheckModemScene_007
 * @tc.desc  : Test AudioCoreService::CheckModemScene, set streamStatus to started if scene is AUDIO_SCENE_PHONE_CALL
 */
HWTEST(AudioCoreServicePrivateTest, CheckModemScene_007, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_PHONE_CALL;
    AudioDeviceDescriptor curDesc(DeviceType::DEVICE_TYPE_EARPIECE, DeviceRole::OUTPUT_DEVICE);
    audioCoreService->audioActiveDevice_.SetCurrentOutputDevice(curDesc);
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    audioCoreService->pipeManager_->modemCommunicationIdMap_.insert(std::make_pair(0, streamDesc));
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> modemDescs;
    audioCoreService->CheckModemScene(modemDescs, reason);
    EXPECT_NE(audioCoreService->pipeManager_->modemCommunicationIdMap_[0], nullptr);
    AudioStreamStatus status = audioCoreService->pipeManager_->modemCommunicationIdMap_[0]->streamStatus_;
    EXPECT_EQ(status, STREAM_STATUS_STARTED);
    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_DEFAULT;
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: CheckModemScene_008
 * @tc.desc  : Test AudioCoreService::CheckModemScene, set streamStatus to stopped if scene is AUDIO_SCENE_DEFAULT
 */
HWTEST(AudioCoreServicePrivateTest, CheckModemScene_008, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_DEFAULT;
    AudioDeviceDescriptor curDesc(DeviceType::DEVICE_TYPE_EARPIECE, DeviceRole::OUTPUT_DEVICE);
    audioCoreService->audioActiveDevice_.SetCurrentOutputDevice(curDesc);
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = AudioStreamDeviceChangeReasonExt::ExtEnum::OVERRODE;
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    audioCoreService->pipeManager_->modemCommunicationIdMap_.insert(std::make_pair(0, streamDesc));
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> modemDescs;
    audioCoreService->CheckModemScene(modemDescs, reason);
    EXPECT_NE(audioCoreService->pipeManager_->modemCommunicationIdMap_[0], nullptr);
    AudioStreamStatus status = audioCoreService->pipeManager_->modemCommunicationIdMap_[0]->streamStatus_;
    EXPECT_EQ(status, STREAM_STATUS_STOPPED);
}


/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: UpdateModemRoute_001
 * @tc.desc  : Test AudioCoreService::UpdateModemRoute
 */
HWTEST(AudioCoreServicePrivateTest, UpdateModemRoute_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    ASSERT_NE(audioCoreService->pipeManager_, nullptr);
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    audioCoreService->pipeManager_->modemCommunicationIdMap_.insert(std::make_pair(0, streamDesc));
    EXPECT_NE(audioCoreService->pipeManager_->modemCommunicationIdMap_[0], nullptr);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> modemDescs =
        audioCoreService->audioRouterCenter_.FetchOutputDevices(STREAM_USAGE_VOICE_MODEM_COMMUNICATION, -1, "");
    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_PHONE_CALL;
    int32_t ret = audioCoreService->UpdateModemRoute(modemDescs);
    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_DEFAULT;
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: UpdateModemRoute_002
 * @tc.desc  : Test AudioCoreService::UpdateModemRoute
 */
HWTEST(AudioCoreServicePrivateTest, UpdateModemRoute_002, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    ASSERT_NE(audioCoreService->pipeManager_, nullptr);
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    audioCoreService->pipeManager_->modemCommunicationIdMap_.insert(std::make_pair(0, streamDesc));
    EXPECT_NE(audioCoreService->pipeManager_->modemCommunicationIdMap_[0], nullptr);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> modemDescs =
        audioCoreService->audioRouterCenter_.FetchOutputDevices(STREAM_USAGE_VOICE_MODEM_COMMUNICATION, -1, "");
    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_DEFAULT;
    int32_t ret = audioCoreService->UpdateModemRoute(modemDescs);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: IsStreamSupportLowpower_001
 * @tc.desc  : Test AudioCoreService::IsStreamSupportLowpower, if playerType is PLAYER_TYPE_SOUND_POOL, return false
 */
HWTEST(AudioCoreServicePrivateTest, IsStreamSupportLowpower_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->rendererInfo_.isOffloadAllowed = true;
    streamDesc->streamInfo_.channels = STEREO;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MUSIC;
    streamDesc->rendererInfo_.playerType = PLAYER_TYPE_SOUND_POOL;
    bool isSupportLowPower = audioCoreService->IsStreamSupportLowpower(streamDesc);
    EXPECT_EQ(isSupportLowPower, false);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: IsStreamSupportLowpower_002
 * @tc.desc  : Test AudioCoreService::IsStreamSupportLowpower, if playerType is PLAYER_TYPE_OPENSL_ES, return false
 */
HWTEST(AudioCoreServicePrivateTest, IsStreamSupportLowpower_002, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->rendererInfo_.isOffloadAllowed = true;
    streamDesc->streamInfo_.channels = STEREO;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MUSIC;
    streamDesc->rendererInfo_.playerType = PLAYER_TYPE_OPENSL_ES;
    bool isSupportLowPower = audioCoreService->IsStreamSupportLowpower(streamDesc);
    EXPECT_EQ(isSupportLowPower, false);
}

/**
* @tc.name  : Test AudioCoreService
* @tc.number: IsStreamSupportLowpower_003
* @tc.desc  : Test interface IsStreamSupportLowpower
*/
HWTEST(AudioCoreServicePrivateTest, IsStreamSupportLowpower_003, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServicePrivateTest IsStreamSupportLower start");
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    EXPECT_NE(streamDesc, nullptr);
    const int32_t AUDIO_EXT_UID = 1041;
    streamDesc->callerUid_ = AUDIO_EXT_UID;
    bool isSupportLowPower = audioCoreService->IsStreamSupportLowpower(streamDesc);
    AUDIO_INFO_LOG("AudioCoreServicePrivateTest IsStreamSupportLower_003 end");
    EXPECT_EQ(isSupportLowPower, false);
}

/**
* @tc.name  : Test AudioCoreService
* @tc.number: IsStreamSupportLowpower_004
* @tc.desc  : Test interface IsStreamSupportLowpower
*/
HWTEST(AudioCoreServicePrivateTest, IsStreamSupportLowpower_004, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServicePrivateTest IsStreamSupportLower start");
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    EXPECT_NE(streamDesc, nullptr);
    const int32_t MEDIA_SERVICE_UID = 1013;
    const int32_t AUDIO_EXT_UID = 1041;
    streamDesc->callerUid_ = MEDIA_SERVICE_UID;
    streamDesc->appInfo_.appUid = AUDIO_EXT_UID;
    bool isSupportLowPower = audioCoreService->IsStreamSupportLowpower(streamDesc);
    AUDIO_INFO_LOG("AudioCoreServicePrivateTest IsStreamSupportLower_004 end");
    EXPECT_EQ(isSupportLowPower, false);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_122
 * @tc.desc  : Test AudioCoreService::ReConfigOffloadStatus
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_122, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    ASSERT_NE(pipeInfo, nullptr);
    std::string sinkName = "test";

    pipeInfo->moduleInfo_.name = "test";
    pipeInfo->moduleInfo_.className = "test";
    audioCoreService->ReConfigOffloadStatus(0, pipeInfo, sinkName);

    pipeInfo->moduleInfo_.name = "test";
    pipeInfo->moduleInfo_.className = "remote_offload";
    audioCoreService->ReConfigOffloadStatus(0, pipeInfo, sinkName);

    pipeInfo->moduleInfo_.name = "Offload_Speaker";
    pipeInfo->moduleInfo_.className = "test";
    audioCoreService->ReConfigOffloadStatus(0, pipeInfo, sinkName);

    pipeInfo->moduleInfo_.name = "Offload_Speaker";
    pipeInfo->moduleInfo_.className = "remote_offload";
    audioCoreService->ReConfigOffloadStatus(0, pipeInfo, sinkName);
    EXPECT_EQ(audioCoreService->audioOffloadStream_.offloadSessionID_.has_value(), true);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_123
 * @tc.desc  : Test AudioCoreService::RemoveUnusedPipe
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_123, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    ASSERT_NE(pipe1, nullptr);
    pipe1->routeFlag_ = AUDIO_OUTPUT_FLAG_LOWPOWER | AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD;
    pipe1->streamDescriptors_.clear();
    pipe1->moduleInfo_.className = "offload";
    audioCoreService->pipeManager_->AddAudioPipeInfo(pipe1);

    std::shared_ptr<AudioPipeInfo> pipe2 = std::make_shared<AudioPipeInfo>();
    ASSERT_NE(pipe2, nullptr);
    pipe2->routeFlag_ = AUDIO_OUTPUT_FLAG_LOWPOWER | AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD;
    pipe2->streamDescriptors_.clear();
    pipe2->moduleInfo_.className = "remote_offload";
    audioCoreService->pipeManager_->AddAudioPipeInfo(pipe2);

    audioCoreService->RemoveUnusedPipe();
    EXPECT_EQ(audioCoreService->pipeManager_->GetUnusedPipe().size(), 2); // 2: unused pipe size
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: LoadSplitModule_001
 * @tc.desc  : Test AudioCoreService::LoadSplitModule.
 */
HWTEST(AudioCoreServicePrivateTest, LoadSplitModule_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    auto ret = audioCoreService->LoadSplitModule("", "");
    EXPECT_NE(ret, 0);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: LoadSplitModule_002
 * @tc.desc  : Test AudioCoreService::LoadSplitModule.
 */
HWTEST(AudioCoreServicePrivateTest, LoadSplitModule_002, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::string splitArgs = "";
    std::string networkId = "b94d27b9934d3e08a52e52d7da";
    auto ret = audioCoreService->LoadSplitModule(splitArgs, networkId);
    EXPECT_NE(ret, 0);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: LoadSplitModule_003
 * @tc.desc  : Test AudioCoreService::LoadSplitModule.
 */
HWTEST(AudioCoreServicePrivateTest, LoadSplitModule_003, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::string splitArgs = "8:4096:1";
    std::string networkId = "";
    auto ret = audioCoreService->LoadSplitModule(splitArgs, networkId);
    EXPECT_NE(ret, 0);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: LoadSplitModule_004
 * @tc.desc  : Test AudioCoreService::LoadSplitModule.
 */
HWTEST(AudioCoreServicePrivateTest, LoadSplitModule_004, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::string splitArgs = "8:4096:1";
    std::string networkId = "b94d27b9934d3e08a52e52d7da7dabfac484efe37a5380ee9088f7ace2efcde9";
    auto ret = audioCoreService->LoadSplitModule(splitArgs, networkId);
    EXPECT_EQ(ret, 0);
}


/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_124
 * @tc.desc  : Test AudioCoreService::OpenNewAudioPortAndRoute()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_124, TestSize.Level1)
{
    uint32_t sessionIDTest = 100;

    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    ASSERT_NE(pipeInfo, nullptr);
    auto audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    ASSERT_NE(audioStreamDescriptor, nullptr);
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);

    auto audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(audioStreamDescriptor, nullptr);
    audioDeviceDescriptor->deviceType_ = DEVICE_TYPE_REMOTE_CAST;
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    audioStreamDescriptor->sessionId_ = sessionIDTest;
    pipeInfo->streamDescriptors_[0]->newDeviceDescs_.push_back(audioDeviceDescriptor);

    uint32_t paIndex = 0;
    auto ret = audioCoreService->OpenNewAudioPortAndRoute(pipeInfo, paIndex);

    EXPECT_EQ(ret, sessionIDTest);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreServicePrivate_125
 * @tc.desc  : Test AudioCoreService::OpenNewAudioPortAndRoute()
 */
HWTEST(AudioCoreServicePrivateTest, AudioCoreServicePrivate_125, TestSize.Level1)
{
    uint32_t sessionIDTest = 0;

    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    AudioStreamInfo audioStreamInfo = {};
    audioStreamInfo.samplingRate =  AudioSamplingRate::SAMPLE_RATE_48000;
    audioStreamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    audioStreamInfo.channels = AudioChannel::STEREO;
    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    ASSERT_NE(pipeInfo, nullptr);
    auto audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    ASSERT_NE(audioStreamDescriptor, nullptr);
    audioStreamDescriptor->streamInfo_ = audioStreamInfo;
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);
    pipeInfo->moduleInfo_.name = BLUETOOTH_MIC;

    auto audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(audioStreamDescriptor, nullptr);
    audioDeviceDescriptor->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP_IN;
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    pipeInfo->streamDescriptors_[0]->newDeviceDescs_.push_back(audioDeviceDescriptor);

    uint32_t paIndex = 0;
    auto ret = audioCoreService->OpenNewAudioPortAndRoute(pipeInfo, paIndex);

    EXPECT_NE(ret, sessionIDTest);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: IsRingerOrAlarmerDualDevicesRange_001.
 * @tc.desc  : Test IsRingerOrAlarmerDualDevicesRange.
 */
HWTEST(AudioCoreServicePrivateTest, IsRingerOrAlarmerDualDevicesRange_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    bool ret = audioCoreService->IsRingerOrAlarmerDualDevicesRange(DEVICE_TYPE_HEARING_AID);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: SwitchActiveHearingAidDevice_001.
 * @tc.desc  : Test SwitchActiveHearingAidDevice.
 */
HWTEST(AudioCoreServicePrivateTest, SwitchActiveHearingAidDevice_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    auto deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    EXPECT_NE(deviceDescriptor, nullptr);

    deviceDescriptor->deviceType_ = DEVICE_TYPE_HEARING_AID;
    deviceDescriptor->macAddress_ = "12:45:56:65:21:43";
    DeviceStreamInfo audioStreamInfo = {AudioSamplingRate::SAMPLE_RATE_16000, AudioEncodingType::ENCODING_PCM,
        AudioSampleFormat::SAMPLE_S16LE, AudioChannel::STEREO};
    deviceDescriptor->audioStreamInfo_ = {audioStreamInfo};

    A2dpDeviceConfigInfo configInfo;
    std::string device = deviceDescriptor->macAddress_;
    int32_t ret = audioCoreService->SwitchActiveHearingAidDevice(deviceDescriptor);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    audioCoreService->audioA2dpDevice_.AddHearingAidDevice(device, configInfo);
    std::string moduleName = HEARING_AID_SPEAKER;
    AudioIOHandle moduleId = 0;
    audioCoreService->audioIOHandleMap_.AddIOHandleInfo(moduleName, moduleId);
    ret = audioCoreService->SwitchActiveHearingAidDevice(deviceDescriptor);
    EXPECT_EQ(ret, SUCCESS);
    audioCoreService->audioIOHandleMap_.DelIOHandleInfo(moduleName);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: ResetNearlinkDeviceState_001.
 * @tc.desc  : Test ResetNearlinkDeviceState.
 */
HWTEST(AudioCoreServicePrivateTest, ResetNearlinkDeviceState_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    auto deviceDesc1 = std::make_shared<AudioDeviceDescriptor>();
    EXPECT_NE(deviceDesc1, nullptr);
    auto deviceDesc2 = std::make_shared<AudioDeviceDescriptor>();
    EXPECT_NE(deviceDesc2, nullptr);

    auto mac1 = "12:45:56:65:21:43";
    auto mac2 = "12:45:56:65:21:44";
    deviceDesc1->deviceType_ = DEVICE_TYPE_NEARLINK;
    deviceDesc2->deviceType_ = DEVICE_TYPE_NEARLINK;
    deviceDesc1->macAddress_ = mac1;
    deviceDesc2->macAddress_ = mac2;

    audioCoreService->audioActiveDevice_.SetCurrentOutputDevice(*deviceDesc1);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = 100;
    streamDesc->streamStatus_ = STREAM_STATUS_STARTED;
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MUSIC;
    streamDesc->newDeviceDescs_.push_back(deviceDesc1);

    audioCoreService->sleAudioDeviceManager_.UpdateSleStreamTypeCount(streamDesc);
    auto beforeState = audioCoreService->sleAudioDeviceManager_.GetNearlinkStreamTypeMapByDevice(mac1);
    EXPECT_EQ(beforeState[0x00000002].size(), 1); // 0x00000002: SLE_AUDIO_STREAM_MUSIC

    audioCoreService->ResetNearlinkDeviceState(deviceDesc2);

    auto afterState = audioCoreService->sleAudioDeviceManager_.GetNearlinkStreamTypeMapByDevice(mac1);
    EXPECT_TRUE(afterState[0x00000002].empty()); // 0x00000002: SLE_AUDIO_STREAM_MUSIC
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: ResetNearlinkDeviceState_002.
 * @tc.desc  : Test ResetNearlinkDeviceState.
 */
HWTEST(AudioCoreServicePrivateTest, ResetNearlinkDeviceState_002, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    auto deviceDesc1 = std::make_shared<AudioDeviceDescriptor>();
    EXPECT_NE(deviceDesc1, nullptr);
    auto deviceDesc2 = std::make_shared<AudioDeviceDescriptor>();
    EXPECT_NE(deviceDesc2, nullptr);

    auto mac1 = "12:45:56:65:21:43";
    auto mac2 = "12:45:56:65:21:44";
    deviceDesc1->deviceType_ = DEVICE_TYPE_NEARLINK_IN;
    deviceDesc2->deviceType_ = DEVICE_TYPE_NEARLINK_IN;
    deviceDesc1->macAddress_ = mac1;
    deviceDesc2->macAddress_ = mac2;

    audioCoreService->audioActiveDevice_.SetCurrentInputDevice(*deviceDesc1);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = 100;
    streamDesc->streamStatus_ = STREAM_STATUS_STARTED;
    streamDesc->audioMode_ = AUDIO_MODE_RECORD;
    streamDesc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    streamDesc->newDeviceDescs_.push_back(deviceDesc1);

    audioCoreService->sleAudioDeviceManager_.UpdateSleStreamTypeCount(streamDesc);
    auto beforeState = audioCoreService->sleAudioDeviceManager_.GetNearlinkStreamTypeMapByDevice(mac1);
    EXPECT_EQ(beforeState[0x00000020].size(), 1); // 0x00000020: SLE_AUDIO_STREAM_VOIP
    audioCoreService->ResetNearlinkDeviceState(deviceDesc2);

    auto afterState = audioCoreService->sleAudioDeviceManager_.GetNearlinkStreamTypeMapByDevice(mac1);
    EXPECT_TRUE(afterState[0x00000020].empty()); // 0x00000020: SLE_AUDIO_STREAM_VOIP
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: CaptureConcurrentCheck_001
 * @tc.desc  : Test AudioCoreService::CaptureConcurrentCheck()
 */
HWTEST(AudioCoreServicePrivateTest, CaptureConcurrentCheck_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServicePrivateTest CaptureConcurrentCheck_001 start");
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs = {
        std::make_shared<AudioStreamDescriptor>(),
        std::make_shared<AudioStreamDescriptor>()
    };
    uint32_t flag[2] = {AUDIO_INPUT_FLAG_NORMAL, AUDIO_INPUT_FLAG_FAST};
    uint32_t originalSessionId[2] = {0};
    for (int i = 0; i < 2; i++) {
        streamDescs[i]->streamInfo_.format = AudioSampleFormat::SAMPLE_S32LE;
        streamDescs[i]->streamInfo_.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
        streamDescs[i]->streamInfo_.channels = AudioChannel::STEREO;
        streamDescs[i]->streamInfo_.encoding = AudioEncodingType::ENCODING_PCM;
        streamDescs[i]->streamInfo_.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
        streamDescs[i]->rendererInfo_.streamUsage = STREAM_USAGE_MOVIE;
 
        streamDescs[i]->audioMode_ = AUDIO_MODE_RECORD;
        streamDescs[i]->createTimeStamp_ = ClockTime::GetCurNano();
        streamDescs[i]->startTimeStamp_ = streamDescs[i]->createTimeStamp_ + 1;
        streamDescs[i]->callerUid_ = getuid();
        auto result = audioCoreService->CreateCapturerClient(streamDescs[i], flag[i], originalSessionId[i]);
        EXPECT_EQ(result, SUCCESS);
    }
    auto dfxResult = std::make_unique<struct ConcurrentCaptureDfxResult>();
    audioCoreService->WriteCapturerConcurrentMsg(streamDescs[0], dfxResult);
    audioCoreService->LogCapturerConcurrentResult(dfxResult);
    audioCoreService->WriteCapturerConcurrentEvent(dfxResult);
    AUDIO_INFO_LOG("AudioCoreServicePrivateTest CaptureConcurrentCheck_001 end");
}
 
/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: CaptureConcurrentCheck_002
 * @tc.desc  : Test AudioCoreService::CaptureConcurrentCheck()
 */
HWTEST(AudioCoreServicePrivateTest, CaptureConcurrentCheck_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServicePrivateTest CaptureConcurrentCheck_002 start");
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    auto dfxResult = std::make_unique<struct ConcurrentCaptureDfxResult>();
    for (int i = 0; i < 5; i++) {
        dfxResult->existingAppName.push_back("www.test.com");
        dfxResult->existingAppState.push_back(static_cast<uint8_t>(2));
        dfxResult->existingSourceType.push_back(static_cast<uint8_t>(SourceType::SOURCE_TYPE_MIC));
        dfxResult->existingCaptureState.push_back(static_cast<uint8_t>(2));
        dfxResult->existingCreateDuration.push_back(static_cast<uint32_t>(0));
        dfxResult->existingStartDuration.push_back(static_cast<uint32_t>(i));
        dfxResult->existingFastFlag.push_back(static_cast<bool>(0));
    }
    dfxResult->hdiSourceType = 1;
    dfxResult->hdiSourceAlg = "develope test";
    dfxResult->deviceType = DEVICE_TYPE_MIC;
    audioCoreService->LogCapturerConcurrentResult(dfxResult);
    audioCoreService->WriteCapturerConcurrentEvent(dfxResult);
    AUDIO_INFO_LOG("AudioCoreServicePrivateTest CaptureConcurrentCheck_002 end");
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: ActivateInputDevice_001
 * @tc.desc  : Test AudioCoreService::ActivateInputDevice()
 */
HWTEST(AudioCoreServicePrivateTest, ActivateInputDevice_001, TestSize.Level4)
{
    AUDIO_INFO_LOG("AudioCoreServicePrivateTest ActivateInputDevice_001 start");
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    EXPECT_NE(streamDesc, nullptr);

    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_USB_ARM_HEADSET,
        DeviceRole::INPUT_DEVICE);
    EXPECT_NE(deviceDesc, nullptr);
    streamDesc->newDeviceDescs_.push_back(deviceDesc);

    auto result = audioCoreService->ActivateInputDevice(streamDesc);
    ASSERT_EQ(result, SUCCESS);
    AUDIO_INFO_LOG("AudioCoreServicePrivateTest ActivateInputDevice_001 end");
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: ActivateInputDevice_002
 * @tc.desc  : Test AudioCoreService::ActivateInputDevice()
 */
HWTEST(AudioCoreServicePrivateTest, ActivateInputDevice_002, TestSize.Level4)
{
    AUDIO_INFO_LOG("AudioCoreServicePrivateTest ActivateInputDevice_002 start");
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    EXPECT_NE(streamDesc, nullptr);

    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_NONE,
        DeviceRole::INPUT_DEVICE);
    EXPECT_NE(deviceDesc, nullptr);
    streamDesc->newDeviceDescs_.push_back(deviceDesc);

    auto result = audioCoreService->ActivateInputDevice(streamDesc);
    EXPECT_EQ(result, SUCCESS);
    AUDIO_INFO_LOG("AudioCoreServicePrivateTest ActivateInputDevice_002 end");
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: CheckAndSleepBeforeRingDualDeviceSet_001
 * @tc.desc  : Test AudioCoreService::CheckAndSleepBeforeRingDualDeviceSet()
 */
HWTEST(AudioCoreServicePrivateTest, CheckAndSleepBeforeRingDualDeviceSet_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    // Test1
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->streamStatus_ == STREAM_STATUS_NEW;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_RINGTONE;

    AudioStreamDeviceChangeReasonExt reason(AudioStreamDeviceChangeReasonExt::ExtEnum::SET_AUDIO_SCENE);

    std::shared_ptr<AudioDeviceDescriptor> newDesc1 = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_USB_HEADSET, DeviceRole::OUTPUT_DEVICE);
    std::shared_ptr<AudioDeviceDescriptor> newDesc2 = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_SPEAKER, DeviceRole::OUTPUT_DEVICE);
    streamDesc->newDeviceDescs_.push_back(newDesc1);
    streamDesc->newDeviceDescs_.push_back(newDesc2);

    auto info = std::make_shared<AudioRendererChangeInfo>();
    info->rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    info->rendererState = RENDERER_RUNNING;
    audioCoreService->streamCollector_.audioRendererChangeInfos_.push_back(info);

    auto start = std::chrono::steady_clock::now();
    audioCoreService->CheckAndSleepBeforeRingDualDeviceSet(streamDesc, reason);
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    uint32_t targetTime = 120000; //120ms
    EXPECT_GE(duration, targetTime);
    streamDesc->newDeviceDescs_.clear();
    audioCoreService->streamCollector_.audioRendererChangeInfos_.clear();

    // Test2
    std::shared_ptr<AudioDeviceDescriptor> newDesc3 = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_EARPIECE, DeviceRole::OUTPUT_DEVICE);
    streamDesc->newDeviceDescs_.push_back(newDesc3);

    start = std::chrono::steady_clock::now();
    audioCoreService->CheckAndSleepBeforeRingDualDeviceSet(streamDesc, reason);
    end = std::chrono::steady_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    uint32_t deltaUs = 5000; // 5ms
    EXPECT_LE(duration, deltaUs);
    audioCoreService->streamCollector_.audioRendererChangeInfos_.clear();
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: CheckAndSleepBeforeRingDualDeviceSet_002
 * @tc.desc  : Test AudioCoreService::CheckAndSleepBeforeRingDualDeviceSet()
 */
HWTEST(AudioCoreServicePrivateTest, CheckAndSleepBeforeRingDualDeviceSet_002, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    // Test3
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->streamStatus_ == STREAM_STATUS_NEW;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_RINGTONE;

    AudioStreamDeviceChangeReasonExt reason(AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN);

    std::shared_ptr<AudioDeviceDescriptor> newDesc1 = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_USB_HEADSET, DeviceRole::OUTPUT_DEVICE);
    std::shared_ptr<AudioDeviceDescriptor> newDesc2 = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_SPEAKER, DeviceRole::OUTPUT_DEVICE);
    streamDesc->newDeviceDescs_.push_back(newDesc1);
    streamDesc->newDeviceDescs_.push_back(newDesc2);

    auto info = std::make_shared<AudioRendererChangeInfo>();
    info->rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    info->rendererState = RENDERER_RUNNING;
    audioCoreService->streamCollector_.audioRendererChangeInfos_.push_back(info);

    auto start = std::chrono::steady_clock::now();
    audioCoreService->CheckAndSleepBeforeRingDualDeviceSet(streamDesc, reason);
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    uint32_t deltaUs = 5000; // 5ms
    EXPECT_LE(duration, deltaUs);
    streamDesc->newDeviceDescs_.clear();
    audioCoreService->streamCollector_.audioRendererChangeInfos_.clear();

    // Test4
    streamDesc->streamStatus_ == STREAM_STATUS_NEW;

    reason = AudioStreamDeviceChangeReasonExt::ExtEnum::SET_AUDIO_SCENE;

    start = std::chrono::steady_clock::now();
    audioCoreService->CheckAndSleepBeforeRingDualDeviceSet(streamDesc, reason);
    end = std::chrono::steady_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    EXPECT_LE(duration, deltaUs);
    audioCoreService->streamCollector_.audioRendererChangeInfos_.clear();
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: CheckAndSleepBeforeRingDualDeviceSet_003
 * @tc.desc  : Test AudioCoreService::CheckAndSleepBeforeRingDualDeviceSet()
 */
HWTEST(AudioCoreServicePrivateTest, CheckAndSleepBeforeRingDualDeviceSet_003, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    // Test5
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->streamStatus_ == STREAM_STATUS_NEW;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MOVIE;

    AudioStreamDeviceChangeReasonExt reason(AudioStreamDeviceChangeReasonExt::ExtEnum::SET_AUDIO_SCENE);

    std::shared_ptr<AudioDeviceDescriptor> newDesc1 = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_USB_HEADSET, DeviceRole::OUTPUT_DEVICE);
    std::shared_ptr<AudioDeviceDescriptor> newDesc2 = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_SPEAKER, DeviceRole::OUTPUT_DEVICE);
    streamDesc->newDeviceDescs_.push_back(newDesc1);
    streamDesc->newDeviceDescs_.push_back(newDesc2);

    auto info = std::make_shared<AudioRendererChangeInfo>();
    info->rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    info->rendererState = RENDERER_RUNNING;
    audioCoreService->streamCollector_.audioRendererChangeInfos_.push_back(info);

    auto start = std::chrono::steady_clock::now();
    audioCoreService->CheckAndSleepBeforeRingDualDeviceSet(streamDesc, reason);
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    uint32_t deltaUs = 5000; // 5ms
    EXPECT_LE(duration, deltaUs);
    streamDesc->newDeviceDescs_.clear();
    audioCoreService->streamCollector_.audioRendererChangeInfos_.clear();

    // Test6
    streamDesc->streamStatus_ == STREAM_STATUS_NEW;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_ALARM;

    info->rendererState = RENDERER_STOPPED;
    audioCoreService->streamCollector_.audioRendererChangeInfos_.push_back(info);

    start = std::chrono::steady_clock::now();
    audioCoreService->CheckAndSleepBeforeRingDualDeviceSet(streamDesc, reason);
    end = std::chrono::steady_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    EXPECT_LE(duration, deltaUs);
    audioCoreService->streamCollector_.audioRendererChangeInfos_.clear();
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: SleepForSwitchDevice_001
 * @tc.desc  : Test AudioCoreService::SleepForSwitchDevice()
 */
HWTEST(AudioCoreServicePrivateTest, SleepForSwitchDevice_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    std::shared_ptr<AudioDeviceDescriptor> oldDesc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, DeviceRole::OUTPUT_DEVICE);
    std::shared_ptr<AudioDeviceDescriptor> newDesc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, DeviceRole::OUTPUT_DEVICE);
    streamDesc->oldDeviceDescs_.push_back(oldDesc);
    streamDesc->oldDeviceDescs_.push_back(newDesc);

    AudioStreamDeviceChangeReasonExt::ExtEnum extReason = AudioStreamDeviceChangeReasonExt::ExtEnum::OVERRODE;
    AudioStreamDeviceChangeReasonExt reason(extReason);

    auto start = std::chrono::steady_clock::now();
    audioCoreService->SleepForSwitchDevice(streamDesc, reason);
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    uint32_t deltaUs = 5000; // 5ms
    EXPECT_LE(duration, deltaUs);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: SleepForSwitchDevice_002
 * @tc.desc  : Test AudioCoreService::SleepForSwitchDevice()
 */
HWTEST(AudioCoreServicePrivateTest, SleepForSwitchDevice_002, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    std::shared_ptr<AudioDeviceDescriptor> oldDesc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, DeviceRole::OUTPUT_DEVICE);
    std::shared_ptr<AudioDeviceDescriptor> newDesc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_SPEAKER, DeviceRole::OUTPUT_DEVICE);
    streamDesc->oldDeviceDescs_.push_back(oldDesc);
    streamDesc->newDeviceDescs_.push_back(newDesc);

    AudioStreamDeviceChangeReasonExt::ExtEnum extReason = AudioStreamDeviceChangeReasonExt::ExtEnum::OVERRODE;
    AudioStreamDeviceChangeReasonExt reason(extReason);

    auto start = std::chrono::steady_clock::now();
    audioCoreService->SleepForSwitchDevice(streamDesc, reason);
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    uint32_t deltaUs = 5000; // 5ms
    uint32_t targetTime = 160000; // 160ms
    EXPECT_GE(duration, targetTime);
    EXPECT_LE(duration, targetTime + deltaUs);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: SleepForSwitchDevice_003
 * @tc.desc  : Test AudioCoreService::SleepForSwitchDevice()
 */
HWTEST(AudioCoreServicePrivateTest, SleepForSwitchDevice_003, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    std::shared_ptr<AudioDeviceDescriptor> oldDesc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, DeviceRole::OUTPUT_DEVICE);
    std::shared_ptr<AudioDeviceDescriptor> newDesc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_SPEAKER, DeviceRole::OUTPUT_DEVICE);
    streamDesc->oldDeviceDescs_.push_back(oldDesc);
    streamDesc->oldDeviceDescs_.push_back(newDesc);

    AudioStreamDeviceChangeReasonExt::ExtEnum extReason =
        AudioStreamDeviceChangeReasonExt::ExtEnum::OLD_DEVICE_UNAVALIABLE;
    AudioStreamDeviceChangeReasonExt reason(extReason);

    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_PHONE_CALL;

    auto start = std::chrono::steady_clock::now();
    audioCoreService->SleepForSwitchDevice(streamDesc, reason);
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    uint32_t deltaUs = 5000; // 5ms
    EXPECT_LE(duration, deltaUs);

    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_DEFAULT;
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AddSessionId_001
 * @tc.desc  : Test AudioCoreService::AddSessionId()
 */
HWTEST(AudioCoreServicePrivateTest, AddSessionId_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    uint32_t sessionId = 1;
    audioCoreService->AddSessionId(sessionId);
    ASSERT_EQ(audioCoreService->sessionIdMap_.count(sessionId), 1);
    audioCoreService->DeleteSessionId(sessionId);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: CheckAndUpdateHearingAidCall_001
 * @tc.desc  : Test AudioCoreService::CheckAndUpdateHearingAidCall
 */
HWTEST(AudioCoreServicePrivateTest, CheckAndUpdateHearingAidCall_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_PHONE_CALL;
    audioCoreService->CheckAndUpdateHearingAidCall(DeviceType::DEVICE_TYPE_HEARING_AID);
    audioCoreService->CheckAndUpdateHearingAidCall(DeviceType::DEVICE_TYPE_EARPIECE);
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    auto pipeInfo = audioPipeManager->GetPipeinfoByNameAndFlag("primary", AUDIO_INPUT_FLAG_NORMAL);
    EXPECT_EQ(pipeInfo, nullptr);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: ActivateOutputDevice_001
 * @tc.desc  : Test AudioCoreService::ActivateOutputDevice
 */
HWTEST(AudioCoreServicePrivateTest, ActivateOutputDevice_001, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> newDeviceDesc = std::make_shared<AudioDeviceDescriptor>();
    newDeviceDesc->deviceType_ = DEVICE_TYPE_HEARING_AID;
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->newDeviceDescs_.push_back(newDeviceDesc);
    auto audioCoreService = std::make_shared<AudioCoreService>();
    int32_t ret = audioCoreService->ActivateOutputDevice(streamDesc);
    ASSERT_EQ(ret, SUCCESS);
}
} // namespace AudioStandard
} // namespace OHOS
