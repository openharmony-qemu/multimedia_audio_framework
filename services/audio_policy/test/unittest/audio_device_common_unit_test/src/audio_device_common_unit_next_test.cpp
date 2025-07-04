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
#include "audio_device_common_unit_next_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
static const int32_t GET_RESULT_NO_VALUE = 0;
static const int32_t GET_RESULT_HAS_VALUE = 1;

void AudioDeviceCommonUnitNextTest::SetUpTestCase(void) {}
void AudioDeviceCommonUnitNextTest::TearDownTestCase(void) {}
void AudioDeviceCommonUnitNextTest::SetUp(void) {}
void AudioDeviceCommonUnitNextTest::TearDown(void) {}

/**
* @tc.name  : Test GetPreferredInputDeviceDescInner.
* @tc.number: GetPreferredInputDeviceDescInner_001
* @tc.desc  : Test GetPreferredInputDeviceDescInner interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, GetPreferredInputDeviceDescInner_001, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();
    AudioCapturerInfo captureInfo;
    captureInfo.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    std::string networkId = LOCAL_NETWORK_ID;
    audioDeviceCommon.audioRouterCenter_.FetchInputDevice(captureInfo.sourceType, -1)->deviceType_ = DEVICE_TYPE_NONE;

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceList =
        audioDeviceCommon.GetPreferredInputDeviceDescInner(captureInfo, networkId);
    EXPECT_NE(deviceList.size(), 0);
}

/**
* @tc.name  : Test GetPreferredInputDeviceDescInner.
* @tc.number: GetPreferredInputDeviceDescInner_002
* @tc.desc  : Test GetPreferredInputDeviceDescInner interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, GetPreferredInputDeviceDescInner_002, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();
    AudioCapturerInfo captureInfo;
    captureInfo.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    std::string networkId = REMOTE_NETWORK_ID;

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceList =
        audioDeviceCommon.GetPreferredInputDeviceDescInner(captureInfo, networkId);
    EXPECT_EQ(deviceList.size(), 0);
}

/**
* @tc.name  : Test GetPreferredInputDeviceDescInner.
* @tc.number: GetPreferredInputDeviceDescInner_003
* @tc.desc  : Test GetPreferredInputDeviceDescInner interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, GetPreferredInputDeviceDescInner_003, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();
    AudioCapturerInfo captureInfo;
    captureInfo.sourceType = SOURCE_TYPE_REMOTE_CAST;
    std::string networkId = LOCAL_NETWORK_ID;
    audioDeviceCommon.audioRouterCenter_.FetchInputDevice(captureInfo.sourceType, -1)->deviceType_ = DEVICE_TYPE_NONE;

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceList =
        audioDeviceCommon.GetPreferredInputDeviceDescInner(captureInfo, networkId);
    EXPECT_NE(deviceList.size(), 0);
}

/**
* @tc.name  : Test UpdateDeviceInfo.
* @tc.number: UpdateDeviceInfo_001
* @tc.desc  : Test UpdateDeviceInfo interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, UpdateDeviceInfo_001, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();
    AudioDeviceDescriptor deviceInfo;
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();

    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    bool hasBTPermission = true;
    bool hasSystemPermission = true;
    audioDeviceCommon.UpdateDeviceInfo(deviceInfo, desc, hasBTPermission, hasSystemPermission);
    EXPECT_EQ(deviceInfo.deviceType_, DEVICE_TYPE_BLUETOOTH_A2DP);
    EXPECT_EQ(deviceInfo.a2dpOffloadFlag_, audioDeviceCommon.audioA2dpOffloadFlag_.GetA2dpOffloadFlag());
}

/**
* @tc.name  : Test UpdateDeviceInfo.
* @tc.number: UpdateDeviceInfo_002
* @tc.desc  : Test UpdateDeviceInfo interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, UpdateDeviceInfo_002, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();
    AudioDeviceDescriptor deviceInfo;
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();

    bool hasBTPermission = false;
    bool hasSystemPermission = false;
    audioDeviceCommon.UpdateDeviceInfo(deviceInfo, desc, hasBTPermission, hasSystemPermission);
    EXPECT_EQ(deviceInfo.deviceName_, "");
    EXPECT_EQ(deviceInfo.networkId_, "");
}

/**
* @tc.name  : Test UpdateConnectedDevicesWhenDisconnecting.
* @tc.number: UpdateConnectedDevicesWhenDisconnecting_001
* @tc.desc  : Test UpdateDeviceInfo interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, UpdateConnectedDevicesWhenDisconnecting_001, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();
    AudioDeviceDescriptor updatedDesc;
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_DP;

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descForCb;
    descForCb.push_back(desc);

    std::shared_ptr<AudioDeviceDescriptor> preferredMediaRenderDevice = std::make_shared<AudioDeviceDescriptor>();
    preferredMediaRenderDevice->deviceType_ = desc->deviceType_;
    preferredMediaRenderDevice->macAddress_ = desc->macAddress_;
    preferredMediaRenderDevice->deviceRole_ = desc->deviceRole_;
    preferredMediaRenderDevice->networkId_ = desc->networkId_;
    audioDeviceCommon.audioStateManager_.SetPreferredMediaRenderDevice(preferredMediaRenderDevice);

    audioDeviceCommon.UpdateConnectedDevicesWhenDisconnecting(updatedDesc, descForCb);
    EXPECT_EQ(desc->deviceType_, DEVICE_TYPE_DP);
}

/**
* @tc.name  : Test UpdateConnectedDevicesWhenDisconnecting.
* @tc.number: UpdateConnectedDevicesWhenDisconnecting_002
* @tc.desc  : Test UpdateDeviceInfo interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, UpdateConnectedDevicesWhenDisconnecting_002, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();
    AudioDeviceDescriptor updatedDesc;
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_USB_HEADSET;

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descForCb;
    descForCb.push_back(desc);

    std::shared_ptr<AudioDeviceDescriptor> preferredCallCaptureDevice = std::make_shared<AudioDeviceDescriptor>();
    preferredCallCaptureDevice->deviceType_ = desc->deviceType_;
    preferredCallCaptureDevice->macAddress_ = desc->macAddress_;
    preferredCallCaptureDevice->deviceRole_ = desc->deviceRole_;
    preferredCallCaptureDevice->networkId_ = desc->networkId_;
    audioDeviceCommon.audioStateManager_.SetPreferredCallCaptureDevice(preferredCallCaptureDevice);

    audioDeviceCommon.UpdateConnectedDevicesWhenDisconnecting(updatedDesc, descForCb);
    EXPECT_EQ(desc->deviceType_, DEVICE_TYPE_USB_HEADSET);
}

/**
* @tc.name  : Test UpdateConnectedDevicesWhenDisconnecting.
* @tc.number: UpdateConnectedDevicesWhenDisconnecting_003
* @tc.desc  : Test UpdateDeviceInfo interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, UpdateConnectedDevicesWhenDisconnecting_003, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();
    AudioDeviceDescriptor updatedDesc;
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descForCb;
    descForCb.push_back(desc);

    std::shared_ptr<AudioDeviceDescriptor> preferredRecordCaptureDevice = std::make_shared<AudioDeviceDescriptor>();
    preferredRecordCaptureDevice->deviceType_ = desc->deviceType_;
    preferredRecordCaptureDevice->macAddress_ = desc->macAddress_;
    preferredRecordCaptureDevice->deviceRole_ = desc->deviceRole_;
    preferredRecordCaptureDevice->networkId_ = desc->networkId_;
    audioDeviceCommon.audioStateManager_.SetPreferredRecordCaptureDevice(preferredRecordCaptureDevice);

    audioDeviceCommon.UpdateConnectedDevicesWhenDisconnecting(updatedDesc, descForCb);
    EXPECT_EQ(desc->deviceType_, DEVICE_TYPE_USB_ARM_HEADSET);
}

/**
* @tc.name  : Test SetDeviceConnectedFlagWhenFetchOutputDevice.
* @tc.number: SetDeviceConnectedFlagWhenFetchOutputDevice_001
* @tc.desc  : Test SetDeviceConnectedFlagWhenFetchOutputDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, SetDeviceConnectedFlagWhenFetchOutputDevice_001, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    AudioDeviceDescriptor deviceDescriptor;
    deviceDescriptor.deviceType_ = DEVICE_TYPE_USB_HEADSET;
    audioDeviceCommon.audioActiveDevice_.SetCurrentOutputDevice(deviceDescriptor);
    audioDeviceCommon.SetDeviceConnectedFlagWhenFetchOutputDevice();
    EXPECT_EQ(audioDeviceCommon.audioActiveDevice_.GetCurrentOutputDevice().deviceType_, DEVICE_TYPE_USB_HEADSET);
}

/**
* @tc.name  : Test SetDeviceConnectedFlagWhenFetchOutputDevice.
* @tc.number: SetDeviceConnectedFlagWhenFetchOutputDevice_002
* @tc.desc  : Test SetDeviceConnectedFlagWhenFetchOutputDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, SetDeviceConnectedFlagWhenFetchOutputDevice_002, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    AudioDeviceDescriptor deviceDescriptor;
    deviceDescriptor.deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;
    audioDeviceCommon.audioActiveDevice_.SetCurrentOutputDevice(deviceDescriptor);
    audioDeviceCommon.SetDeviceConnectedFlagWhenFetchOutputDevice();
    EXPECT_EQ(audioDeviceCommon.audioActiveDevice_.GetCurrentOutputDevice().deviceType_, DEVICE_TYPE_USB_ARM_HEADSET);
}

/**
* @tc.name  : Test SetDeviceConnectedFlagWhenFetchOutputDevice.
* @tc.number: SetDeviceConnectedFlagWhenFetchOutputDevice_003
* @tc.desc  : Test SetDeviceConnectedFlagWhenFetchOutputDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, SetDeviceConnectedFlagWhenFetchOutputDevice_003, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    AudioDeviceDescriptor deviceDescriptor;
    deviceDescriptor.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP_IN;
    audioDeviceCommon.audioActiveDevice_.SetCurrentOutputDevice(deviceDescriptor);
    audioDeviceCommon.SetDeviceConnectedFlagWhenFetchOutputDevice();
    EXPECT_EQ(audioDeviceCommon.audioActiveDevice_.GetCurrentOutputDevice().deviceType_, DEVICE_TYPE_BLUETOOTH_A2DP_IN);
}

/**
* @tc.name  : Test FetchOutputDevice
* @tc.number: FetchOutputDevice_001
* @tc.desc  : Test FetchOutputDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, FetchOutputDevice_001, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> rendererChangeInfos;
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfos.push_back(rendererChangeInfo);
    rendererChangeInfo->rendererInfo.streamUsage = STREAM_USAGE_VOICE_MODEM_COMMUNICATION;
    audioDeviceCommon.audioSceneManager_.audioScene_ = AUDIO_SCENE_PHONE_CALL;

    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    audioDeviceCommon.FetchOutputDevice(rendererChangeInfos, reason);
    EXPECT_NE(rendererChangeInfo, nullptr);
}

/**
* @tc.name  : Test HandleDeviceChangeForFetchOutputDevice
* @tc.number: HandleDeviceChangeForFetchOutputDevice_001
* @tc.desc  : Test HandleDeviceChangeForFetchOutputDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, HandleDeviceChangeForFetchOutputDevice_001, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_SPEAKER;
    desc->connectState_ = CONNECTED;
    desc->descriptorType_ = AudioDeviceDescriptor::AUDIO_DEVICE_DESCRIPTOR;

    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->outputDeviceInfo = AudioDeviceDescriptor(*desc);

    audioDeviceCommon.shouldUpdateDeviceDueToDualTone_ = false;
    audioDeviceCommon.audioSceneManager_.audioScene_ = AUDIO_SCENE_DEFAULT;
    audioDeviceCommon.audioActiveDevice_.currentActiveDevice_ = AudioDeviceDescriptor(*desc);

    int32_t result = audioDeviceCommon.HandleDeviceChangeForFetchOutputDevice(desc, rendererChangeInfo,
        AudioStreamDeviceChangeReason::UNKNOWN);
    EXPECT_EQ(result, ERR_NEED_NOT_SWITCH_DEVICE);
}

/**
* @tc.name  : Test HandleDeviceChangeForFetchOutputDevice.
* @tc.number: HandleDeviceChangeForFetchOutputDevice_002
* @tc.desc  : Test HandleDeviceChangeForFetchOutputDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, HandleDeviceChangeForFetchOutputDevice_002, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_NONE;
    desc->connectState_ = CONNECTED;

    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->clientUID = 12345;
    audioDeviceCommon.shouldUpdateDeviceDueToDualTone_ = false;
    audioDeviceCommon.audioSceneManager_.audioScene_ = AUDIO_SCENE_DEFAULT;

    AudioDeviceDescriptor tmpOutputDeviceDesc;
    tmpOutputDeviceDesc.deviceType_ = DEVICE_TYPE_SPEAKER;
    audioDeviceCommon.audioActiveDevice_.currentActiveDevice_ = tmpOutputDeviceDesc;

    std::shared_ptr<AudioDeviceDescriptor> preferredDesc = std::make_shared<AudioDeviceDescriptor>();
    preferredDesc->deviceType_ = DEVICE_TYPE_SPEAKER;
    audioDeviceCommon.audioAffinityManager_.activeRendererDeviceMap_[rendererChangeInfo->clientUID] = preferredDesc;

    int32_t result = audioDeviceCommon.HandleDeviceChangeForFetchOutputDevice(desc, rendererChangeInfo,
        AudioStreamDeviceChangeReason::UNKNOWN);
    EXPECT_EQ(result, ERR_NEED_NOT_SWITCH_DEVICE);
}

/**
* @tc.name  : Test MuteSinkPortForSwitchDevice
* @tc.number: MuteSinkPortForSwitchDevice_001
* @tc.desc  : Test MuteSinkPortForSwitchDevice.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, MuteSinkPortForSwitchDevice_001, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->outputDeviceInfo.deviceType_ = DEVICE_TYPE_SPEAKER;
    rendererChangeInfo->rendererInfo.streamUsage = STREAM_USAGE_VOICE_RINGTONE;
    rendererChangeInfo->sessionId = 12345;

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> outputDevices;
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceType_ = DEVICE_TYPE_EARPIECE;
    outputDevices.push_back(deviceDesc);

    audioDeviceCommon.audioSceneManager_.audioScene_ = AUDIO_SCENE_PHONE_CALL;
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    audioDeviceCommon.MuteSinkPortForSwitchDevice(rendererChangeInfo, outputDevices, reason);
    EXPECT_NE(rendererChangeInfo->outputDeviceInfo.deviceType_, deviceDesc->deviceType_);
}

/**
* @tc.name  : Test MuteSinkPortForSwitchDevice
* @tc.number: MuteSinkPortForSwitchDevice_002
* @tc.desc  : Test MuteSinkPortForSwitchDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, MuteSinkPortForSwitchDevice_002, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->outputDeviceInfo.deviceType_ = DEVICE_TYPE_SPEAKER;
    rendererChangeInfo->rendererInfo.streamUsage = STREAM_USAGE_VOICE_MODEM_COMMUNICATION;
    rendererChangeInfo->sessionId = 12345;

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> outputDevices;
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceType_ = DEVICE_TYPE_EARPIECE;
    outputDevices.push_back(deviceDesc);

    audioDeviceCommon.audioSceneManager_.audioScene_ = AUDIO_SCENE_PHONE_CALL;
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    audioDeviceCommon.MuteSinkPortForSwitchDevice(rendererChangeInfo, outputDevices, reason);
    EXPECT_NE(rendererChangeInfo->outputDeviceInfo.deviceType_, deviceDesc->deviceType_);
}

/**
* @tc.name  : Test MuteSinkPortForSwitchDevice
* @tc.number: MuteSinkPortForSwitchDevice_003
* @tc.desc  : Test MuteSinkPortForSwitchDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, MuteSinkPortForSwitchDevice_003, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->outputDeviceInfo.deviceType_ = DEVICE_TYPE_SPEAKER;
    rendererChangeInfo->rendererInfo.streamUsage = STREAM_USAGE_VOICE_MODEM_COMMUNICATION;
    rendererChangeInfo->sessionId = 12345;

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> outputDevices;
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceType_ = DEVICE_TYPE_EARPIECE;
    outputDevices.push_back(deviceDesc);

    audioDeviceCommon.audioSceneManager_.audioScene_ = AUDIO_SCENE_PHONE_CALL;

    audioDeviceCommon.streamCollector_.audioRendererChangeInfos_.clear();
    std::shared_ptr<AudioRendererChangeInfo> changeInfo = std::make_shared<AudioRendererChangeInfo>();
    changeInfo->rendererState = RENDERER_PREPARED;
    changeInfo->rendererInfo.streamUsage = STREAM_USAGE_VOICE_MODEM_COMMUNICATION;
    changeInfo->sessionId = 12345;
    audioDeviceCommon.streamCollector_.audioRendererChangeInfos_.push_back(changeInfo);

    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    audioDeviceCommon.MuteSinkPortForSwitchDevice(rendererChangeInfo, outputDevices, reason);
    EXPECT_NE(rendererChangeInfo->outputDeviceInfo.deviceType_, deviceDesc->deviceType_);
}

/**
* @tc.name  : Test SelectRingerOrAlarmDevices
* @tc.number: SelectRingerOrAlarmDevices_001
* @tc.desc  : Test SelectRingerOrAlarmDevices interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, SelectRingerOrAlarmDevices_001, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->sessionId = 12345;
    rendererChangeInfo->rendererInfo.streamUsage = STREAM_USAGE_RINGTONE;

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc1 = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc1->deviceType_ = DEVICE_TYPE_REMOTE_CAST;
    deviceDesc1->networkId_ = "LocalNetworkId1";
    deviceDesc1->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    descs.push_back(deviceDesc1);

    std::shared_ptr<AudioDeviceDescriptor> deviceDesc2 = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc2->deviceType_ = DEVICE_TYPE_REMOTE_CAST;
    deviceDesc2->networkId_ = "LocalNetworkId2";
    deviceDesc1->deviceRole_ = DeviceRole::INPUT_DEVICE;
    descs.push_back(deviceDesc2);

    audioDeviceCommon.enableDualHalToneState_ = true;
    audioDeviceCommon.enableDualHalToneSessionId_ = 54321;
    audioDeviceCommon.audioPolicyManager_.SetRingerMode(RINGER_MODE_SILENT);

    bool result = audioDeviceCommon.SelectRingerOrAlarmDevices(descs, rendererChangeInfo);
    EXPECT_FALSE(result);
}

/**
* @tc.name  : Test SelectRingerOrAlarmDevices
* @tc.number: SelectRingerOrAlarmDevices_002
* @tc.desc  : Test SelectRingerOrAlarmDevices interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, SelectRingerOrAlarmDevices_002, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->sessionId = 12345;
    rendererChangeInfo->rendererInfo.streamUsage = STREAM_USAGE_RINGTONE;

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc1 = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc1->deviceType_ = DEVICE_TYPE_REMOTE_CAST;
    deviceDesc1->networkId_ = "LocalNetworkId1";
    deviceDesc1->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    descs.push_back(deviceDesc1);

    std::shared_ptr<AudioDeviceDescriptor> deviceDesc2 = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc2->deviceType_ = DEVICE_TYPE_REMOTE_CAST;
    deviceDesc2->networkId_ = "LocalNetworkId2";
    deviceDesc1->deviceRole_ = DeviceRole::INPUT_DEVICE;
    descs.push_back(deviceDesc2);

    audioDeviceCommon.enableDualHalToneState_ = false;
    audioDeviceCommon.enableDualHalToneSessionId_ = 54321;
    audioDeviceCommon.audioPolicyManager_.SetRingerMode(RINGER_MODE_NORMAL);

    bool result = audioDeviceCommon.SelectRingerOrAlarmDevices(descs, rendererChangeInfo);
    EXPECT_TRUE(result);
}

/**
* @tc.name  : Test SelectRingerOrAlarmDevices
* @tc.number: SelectRingerOrAlarmDevices_003
* @tc.desc  : Test SelectRingerOrAlarmDevices interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, SelectRingerOrAlarmDevices_003, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->sessionId = 12345;
    rendererChangeInfo->rendererInfo.streamUsage = STREAM_USAGE_ALARM;

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc1 = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc1->deviceType_ = DEVICE_TYPE_REMOTE_CAST;
    deviceDesc1->networkId_ = "LocalNetworkId1";
    deviceDesc1->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    descs.push_back(deviceDesc1);

    std::shared_ptr<AudioDeviceDescriptor> deviceDesc2 = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc2->deviceType_ = DEVICE_TYPE_REMOTE_CAST;
    deviceDesc2->networkId_ = "LocalNetworkId2";
    deviceDesc1->deviceRole_ = DeviceRole::INPUT_DEVICE;
    descs.push_back(deviceDesc2);

    audioDeviceCommon.enableDualHalToneState_ = true;
    audioDeviceCommon.enableDualHalToneSessionId_ = 12345;
    audioDeviceCommon.audioPolicyManager_.SetRingerMode(RINGER_MODE_SILENT);

    bool result = audioDeviceCommon.SelectRingerOrAlarmDevices(descs, rendererChangeInfo);
    EXPECT_TRUE(result);
}

/**
* @tc.name  : Test SelectRingerOrAlarmDevices
* @tc.number: SelectRingerOrAlarmDevices_004
* @tc.desc  : Test SelectRingerOrAlarmDevices interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, SelectRingerOrAlarmDevices_004, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->sessionId = 12345;
    rendererChangeInfo->rendererInfo.streamUsage = STREAM_USAGE_ALARM;

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc1 = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc1->deviceType_ = DEVICE_TYPE_REMOTE_CAST;
    deviceDesc1->networkId_ = "LocalNetworkId1";
    deviceDesc1->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    descs.push_back(deviceDesc1);

    std::shared_ptr<AudioDeviceDescriptor> deviceDesc2 = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc2->deviceType_ = DEVICE_TYPE_REMOTE_CAST;
    deviceDesc2->networkId_ = "LocalNetworkId1";
    deviceDesc1->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    descs.push_back(deviceDesc2);

    audioDeviceCommon.enableDualHalToneState_ = true;
    audioDeviceCommon.enableDualHalToneSessionId_ = 12345;
    audioDeviceCommon.audioPolicyManager_.SetRingerMode(RINGER_MODE_SILENT);

    bool result = audioDeviceCommon.SelectRingerOrAlarmDevices(descs, rendererChangeInfo);
    EXPECT_TRUE(result);
}

/**
* @tc.name  : Test SelectRingerOrAlarmDevices
* @tc.number: SelectRingerOrAlarmDevices_005
* @tc.desc  : Test SelectRingerOrAlarmDevices interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, SelectRingerOrAlarmDevices_005, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->sessionId = 12345;
    rendererChangeInfo->rendererInfo.streamUsage = STREAM_USAGE_ALARM;

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc1 = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc1->deviceType_ = DEVICE_TYPE_REMOTE_CAST;
    deviceDesc1->networkId_ = "LocalNetworkId1";
    deviceDesc1->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    descs.push_back(deviceDesc1);

    std::shared_ptr<AudioDeviceDescriptor> deviceDesc2 = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc2->deviceType_ = DEVICE_TYPE_REMOTE_CAST;
    deviceDesc2->networkId_ = "LocalNetworkId1";
    deviceDesc1->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    descs.push_back(deviceDesc2);

    audioDeviceCommon.enableDualHalToneState_ = false;
    audioDeviceCommon.enableDualHalToneSessionId_ = 12345;
    audioDeviceCommon.audioPolicyManager_.SetRingerMode(RINGER_MODE_SILENT);

    bool result = audioDeviceCommon.SelectRingerOrAlarmDevices(descs, rendererChangeInfo);
    EXPECT_TRUE(result);
}

/**
* @tc.name  : Test SelectRingerOrAlarmDevices
* @tc.number: SelectRingerOrAlarmDevices_006
* @tc.desc  : Test SelectRingerOrAlarmDevices interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, SelectRingerOrAlarmDevices_006, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->sessionId = 12345;
    rendererChangeInfo->rendererInfo.streamUsage = STREAM_USAGE_ALARM;

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc1 = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc1->deviceType_ = DEVICE_TYPE_REMOTE_CAST;
    deviceDesc1->networkId_ = "LocalNetworkId1";
    deviceDesc1->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    descs.push_back(deviceDesc1);

    std::shared_ptr<AudioDeviceDescriptor> deviceDesc2 = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc2->deviceType_ = DEVICE_TYPE_REMOTE_CAST;
    deviceDesc2->networkId_ = "LocalNetworkId1";
    deviceDesc1->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    descs.push_back(deviceDesc2);

    audioDeviceCommon.enableDualHalToneState_ = true;
    audioDeviceCommon.enableDualHalToneSessionId_ = 54321;
    audioDeviceCommon.audioPolicyManager_.SetRingerMode(RINGER_MODE_SILENT);

    bool result = audioDeviceCommon.SelectRingerOrAlarmDevices(descs, rendererChangeInfo);
    EXPECT_TRUE(result);
}

/**
* @tc.name  : Test SelectRingerOrAlarmDevices
* @tc.number: SelectRingerOrAlarmDevices_007
* @tc.desc  : Test SelectRingerOrAlarmDevices interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, SelectRingerOrAlarmDevices_007, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->sessionId = 12345;
    rendererChangeInfo->rendererInfo.streamUsage = STREAM_USAGE_ALARM;

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc1 = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc1->deviceType_ = DEVICE_TYPE_LINE_DIGITAL;
    deviceDesc1->networkId_ = "LocalNetworkId1";
    deviceDesc1->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    descs.push_back(deviceDesc1);

    std::shared_ptr<AudioDeviceDescriptor> deviceDesc2 = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc2->deviceType_ = DEVICE_TYPE_LINE_DIGITAL;
    deviceDesc2->networkId_ = "LocalNetworkId1";
    deviceDesc1->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    descs.push_back(deviceDesc2);

    bool result = audioDeviceCommon.SelectRingerOrAlarmDevices(descs, rendererChangeInfo);
    EXPECT_FALSE(result);
}

/**
* @tc.name  : Test SelectRingerOrAlarmDevices
* @tc.number: SelectRingerOrAlarmDevices_008
* @tc.desc  : Test SelectRingerOrAlarmDevices interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, SelectRingerOrAlarmDevices_008, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->sessionId = 12345;
    rendererChangeInfo->rendererInfo.streamUsage = STREAM_USAGE_ALARM;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;

    bool result = audioDeviceCommon.SelectRingerOrAlarmDevices(descs, rendererChangeInfo);
    EXPECT_FALSE(result);
}

/**
* @tc.name  : Test HandleDeviceChangeForFetchInputDevice
* @tc.number: HandleDeviceChangeForFetchInputDevice_001
* @tc.desc  : Test HandleDeviceChangeForFetchInputDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, HandleDeviceChangeForFetchInputDevice_001, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_MIC;
    desc->connectState_ = CONNECTED;
    desc->networkId_ = "LocalNetworkId";
    desc->macAddress_ = "00:11:22:33:44:55";

    std::shared_ptr<AudioCapturerChangeInfo> capturerChangeInfo = std::make_shared<AudioCapturerChangeInfo>();
    capturerChangeInfo->sessionId = 12345;
    capturerChangeInfo->inputDeviceInfo = AudioDeviceDescriptor(*desc);
    capturerChangeInfo->clientUID = 54321;

    AudioDeviceDescriptor currentInputDevice;
    currentInputDevice.deviceType_ = DEVICE_TYPE_MIC;
    currentInputDevice.networkId_ = "LocalNetworkId";
    currentInputDevice.macAddress_ = "00:11:22:33:44:55";
    currentInputDevice.connectState_ = CONNECTED;
    audioDeviceCommon.audioActiveDevice_.currentActiveInputDevice_ = currentInputDevice;

    std::shared_ptr<AudioDeviceDescriptor> preferredDesc = std::make_shared<AudioDeviceDescriptor>();
    preferredDesc->deviceType_ = DEVICE_TYPE_MIC;
    audioDeviceCommon.audioAffinityManager_.activeCapturerDeviceMap_[capturerChangeInfo->clientUID] = preferredDesc;

    int32_t result = audioDeviceCommon.HandleDeviceChangeForFetchInputDevice(desc, capturerChangeInfo);
    EXPECT_EQ(result, ERR_NEED_NOT_SWITCH_DEVICE);
}

/**
* @tc.name  : Test FetchInputDeviceInner.
* @tc.number: FetchInputDeviceInner_001
* @tc.desc  : Test FetchInputDeviceInner interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, FetchInputDeviceInner_001, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> capturerChangeInfos;
    std::shared_ptr<AudioCapturerChangeInfo> capturerChangeInfo = std::make_shared<AudioCapturerChangeInfo>();
    capturerChangeInfo->capturerInfo.sourceType = SOURCE_TYPE_VIRTUAL_CAPTURE;
    capturerChangeInfo->prerunningState = false;
    capturerChangeInfo->sessionId = 12345;
    capturerChangeInfos.push_back(capturerChangeInfo);

    audioDeviceCommon.audioSceneManager_.SetAudioScenePre(AUDIO_SCENE_DEFAULT);
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    bool needUpdateActiveDevice = true;
    bool isUpdateActiveDevice = false;
    int32_t runningStreamCount = 0;

    audioDeviceCommon.FetchInputDeviceInner(capturerChangeInfos, reason, needUpdateActiveDevice,
        isUpdateActiveDevice, runningStreamCount);
    EXPECT_EQ(runningStreamCount, 0);
}

/**
* @tc.name  : Test FetchInputDeviceInner.
* @tc.number: FetchInputDeviceInner_002
* @tc.desc  : Test FetchInputDeviceInner interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, FetchInputDeviceInner_002, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> capturerChangeInfos;
    std::shared_ptr<AudioCapturerChangeInfo> capturerChangeInfo = std::make_shared<AudioCapturerChangeInfo>();
    capturerChangeInfo->capturerInfo.sourceType = SOURCE_TYPE_VIRTUAL_CAPTURE; // Valid source type
    capturerChangeInfo->capturerState = CAPTURER_RUNNING;
    capturerChangeInfo->prerunningState = false;
    capturerChangeInfo->sessionId = 12345;
    capturerChangeInfos.push_back(capturerChangeInfo);

    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_MIC;
    desc->connectState_ = CONNECTED;

    capturerChangeInfo->inputDeviceInfo.deviceType_ = DEVICE_TYPE_MIC;
    capturerChangeInfo->inputDeviceInfo.connectState_ = CONNECTED;

    audioDeviceCommon.audioRouterCenter_.FetchInputDevice(capturerChangeInfo->capturerInfo.sourceType,
        capturerChangeInfo->clientUID, capturerChangeInfo->sessionId);
    audioDeviceCommon.audioSceneManager_.SetAudioScenePre(AUDIO_SCENE_PHONE_CALL);

    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    bool needUpdateActiveDevice = true;
    bool isUpdateActiveDevice = false;
    int32_t runningStreamCount = 0;

    audioDeviceCommon.FetchInputDeviceInner(capturerChangeInfos, reason, needUpdateActiveDevice,
        isUpdateActiveDevice, runningStreamCount);
    EXPECT_EQ(runningStreamCount, 1);
    EXPECT_FALSE(isUpdateActiveDevice);
}

/**
* @tc.name  : Test FetchInputDeviceInner.
* @tc.number: FetchInputEnd_001
* @tc.desc  : Test FetchInputDeviceInner interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, FetchInputEnd_001, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    bool isUpdateActiveDevice = true;
    int32_t runningStreamCount = 1;

    audioDeviceCommon.FetchInputEnd(isUpdateActiveDevice, runningStreamCount);
    EXPECT_TRUE(isUpdateActiveDevice);
}

/**
* @tc.name  : Test MoveToNewInputDevice.
* @tc.number: MoveToNewInputDevice_001
* @tc.desc  : Test MoveToNewInputDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, MoveToNewInputDevice_001, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    std::shared_ptr<AudioCapturerChangeInfo> capturerChangeInfo = std::make_shared<AudioCapturerChangeInfo>();
    capturerChangeInfo->sessionId = 12345;
    capturerChangeInfo->inputDeviceInfo.deviceType_ = DEVICE_TYPE_MIC;
    capturerChangeInfo->inputDeviceInfo.macAddress_ = "00:11:22:33:44:55";
    capturerChangeInfo->inputDeviceInfo.networkId_ = LOCAL_NETWORK_ID;

    std::shared_ptr<AudioDeviceDescriptor> inputDevice = std::make_shared<AudioDeviceDescriptor>();
    inputDevice->deviceType_ = DEVICE_TYPE_USB_HEADSET;
    inputDevice->macAddress_ = "66:77:88:99:AA:BB";
    inputDevice->networkId_ = LOCAL_NETWORK_ID;

    audioDeviceCommon.audioConfigManager_.OnUpdateRouteSupport(true);
    audioDeviceCommon.MoveToNewInputDevice(capturerChangeInfo, inputDevice);
    EXPECT_EQ(capturerChangeInfo->inputDeviceInfo.deviceType_, inputDevice->deviceType_);
    EXPECT_EQ(capturerChangeInfo->inputDeviceInfo.macAddress_, inputDevice->macAddress_);
    EXPECT_EQ(capturerChangeInfo->inputDeviceInfo.networkId_, inputDevice->networkId_);
}

/**
* @tc.name  : Test MoveToNewInputDevice.
* @tc.number: MoveToNewInputDevice_002
* @tc.desc  : Test MoveToNewInputDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, MoveToNewInputDevice_002, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    std::shared_ptr<AudioCapturerChangeInfo> capturerChangeInfo = std::make_shared<AudioCapturerChangeInfo>();
    capturerChangeInfo->sessionId = 12345;
    capturerChangeInfo->inputDeviceInfo.deviceType_ = DEVICE_TYPE_MIC;
    capturerChangeInfo->inputDeviceInfo.macAddress_ = "00:11:22:33:44:55";
    capturerChangeInfo->inputDeviceInfo.networkId_ = LOCAL_NETWORK_ID;

    std::shared_ptr<AudioDeviceDescriptor> inputDevice = std::make_shared<AudioDeviceDescriptor>();
    inputDevice->deviceType_ = DEVICE_TYPE_USB_HEADSET;
    inputDevice->macAddress_ = "66:77:88:99:AA:BB";
    inputDevice->networkId_ = LOCAL_NETWORK_ID;

    audioDeviceCommon.audioConfigManager_.OnUpdateRouteSupport(false);
    audioDeviceCommon.MoveToNewInputDevice(capturerChangeInfo, inputDevice);
    EXPECT_EQ(capturerChangeInfo->inputDeviceInfo.deviceType_, inputDevice->deviceType_);
    EXPECT_EQ(capturerChangeInfo->inputDeviceInfo.macAddress_, inputDevice->macAddress_);
    EXPECT_EQ(capturerChangeInfo->inputDeviceInfo.networkId_, inputDevice->networkId_);
}

/**
* @tc.name  : Test MoveToNewInputDevice.
* @tc.number: MoveToNewInputDevice_003
* @tc.desc  : Test MoveToNewInputDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, MoveToNewInputDevice_003, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    std::shared_ptr<AudioCapturerChangeInfo> capturerChangeInfo = std::make_shared<AudioCapturerChangeInfo>();
    capturerChangeInfo->sessionId = 12345;
    capturerChangeInfo->inputDeviceInfo.deviceType_ = DEVICE_TYPE_MIC;
    capturerChangeInfo->inputDeviceInfo.macAddress_ = "00:11:22:33:44:55";
    capturerChangeInfo->inputDeviceInfo.networkId_ = LOCAL_NETWORK_ID;

    std::shared_ptr<AudioDeviceDescriptor> inputDevice = std::make_shared<AudioDeviceDescriptor>();
    inputDevice->deviceType_ = DEVICE_TYPE_USB_HEADSET;
    inputDevice->macAddress_ = "66:77:88:99:AA:BB";
    inputDevice->networkId_ = REMOTE_NETWORK_ID;

    audioDeviceCommon.audioConfigManager_.OnUpdateRouteSupport(true);
    audioDeviceCommon.MoveToNewInputDevice(capturerChangeInfo, inputDevice);
    EXPECT_EQ(capturerChangeInfo->inputDeviceInfo.deviceType_, inputDevice->deviceType_);
    EXPECT_EQ(capturerChangeInfo->inputDeviceInfo.macAddress_, inputDevice->macAddress_);
    EXPECT_EQ(capturerChangeInfo->inputDeviceInfo.networkId_, inputDevice->networkId_);
}

/**
* @tc.name  : Test FetchInputDeviceWhenNoRunningStream.
* @tc.number: FetchInputDeviceWhenNoRunningStream_001
* @tc.desc  : Test FetchInputDeviceWhenNoRunningStream interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, FetchInputDeviceWhenNoRunningStream_001, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    AudioDeviceDescriptor tempDesc;
    tempDesc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioDeviceCommon.audioActiveDevice_.SetCurrentInputDevice(tempDesc);
    audioDeviceCommon.FetchInputDeviceWhenNoRunningStream();
    EXPECT_EQ(audioDeviceCommon.audioActiveDevice_.currentActiveInputDevice_.deviceType_, DEVICE_TYPE_BLUETOOTH_A2DP);
}

/**
* @tc.name  : Test FetchInputDeviceWhenNoRunningStream.
* @tc.number: FetchInputDeviceWhenNoRunningStream_002
* @tc.desc  : Test FetchInputDeviceWhenNoRunningStream interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, FetchInputDeviceWhenNoRunningStream_002, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    AudioDeviceDescriptor tempDesc;
    tempDesc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioDeviceCommon.audioActiveDevice_.SetCurrentInputDevice(tempDesc);
    audioDeviceCommon.FetchInputDeviceWhenNoRunningStream();
    EXPECT_EQ(audioDeviceCommon.audioActiveDevice_.currentActiveInputDevice_.deviceType_, DEVICE_TYPE_BLUETOOTH_A2DP);
}

/**
* @tc.name  : Test BluetoothScoFetch.
* @tc.number: BluetoothScoFetch_001
* @tc.desc  : Test BluetoothScoFetch interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, BluetoothScoFetch_001, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    desc->macAddress_ = "00:11:22:33:44:55";
    desc->networkId_ = LOCAL_NETWORK_ID;

    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> capturerChangeInfos;
    std::shared_ptr<AudioCapturerChangeInfo> capturerChangeInfo = std::make_shared<AudioCapturerChangeInfo>();
    capturerChangeInfo->capturerInfo.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    capturerChangeInfos.push_back(capturerChangeInfo);

    audioDeviceCommon.audioActiveDevice_.SetCurrentInputDevice(*desc);
    audioDeviceCommon.BluetoothScoFetch(desc, capturerChangeInfos, SOURCE_TYPE_VOICE_RECOGNITION);
    const AudioDeviceDescriptor& activeDevice = audioDeviceCommon.audioActiveDevice_.GetCurrentInputDevice();
    EXPECT_EQ(activeDevice.deviceType_, DEVICE_TYPE_BLUETOOTH_SCO);
    EXPECT_EQ(activeDevice.macAddress_, "00:11:22:33:44:55");
    EXPECT_EQ(activeDevice.networkId_, LOCAL_NETWORK_ID);
}

/**
* @tc.name  : Test MoveToRemoteOutputDevice.
* @tc.number: MoveToRemoteOutputDevice_001
* @tc.desc  : Test MoveToRemoteOutputDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, MoveToRemoteOutputDevice_001, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    std::vector<SinkInput> sinkInputs;
    SinkInput sinkInput = {};
    sinkInputs.push_back(sinkInput);

    std::shared_ptr<AudioDeviceDescriptor> remoteDevice = std::make_shared<AudioDeviceDescriptor>();
    remoteDevice->deviceType_ = DEVICE_TYPE_REMOTE_CAST;
    remoteDevice->macAddress_ = "00:11:22:33:44:55";
    remoteDevice->networkId_ = REMOTE_NETWORK_ID;

    std::string moduleName = "123";
    AudioIOHandle moduleId = 1;
    audioDeviceCommon.audioIOHandleMap_.AddIOHandleInfo(moduleName, moduleId);

    int32_t result = audioDeviceCommon.MoveToRemoteOutputDevice(sinkInputs, remoteDevice);
    EXPECT_EQ(result, ERR_OPERATION_FAILED);
    EXPECT_TRUE(audioDeviceCommon.audioIOHandleMap_.CheckIOHandleExist(moduleName));
}

/**
* @tc.name  : Test MoveToRemoteOutputDevice.
* @tc.number: MoveToRemoteOutputDevice_002
* @tc.desc  : Test MoveToRemoteOutputDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, MoveToRemoteOutputDevice_002, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    std::vector<SinkInput> sinkInputs;
    SinkInput sinkInput = {};
    sinkInputs.push_back(sinkInput);

    std::string moduleName = "123";
    std::shared_ptr<AudioDeviceDescriptor> remoteDevice = std::make_shared<AudioDeviceDescriptor>();
    remoteDevice->deviceType_ = DEVICE_TYPE_REMOTE_CAST;
    remoteDevice->macAddress_ = "00:11:22:33:44:55";
    remoteDevice->networkId_ = REMOTE_NETWORK_ID;

    audioDeviceCommon.audioIOHandleMap_.IOHandles_.clear();
    audioDeviceCommon.isOpenRemoteDevice = false;

    int32_t result = audioDeviceCommon.MoveToRemoteOutputDevice(sinkInputs, remoteDevice);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
    EXPECT_FALSE(audioDeviceCommon.audioIOHandleMap_.CheckIOHandleExist(moduleName));
}

/**
* @tc.name  : Test MoveToRemoteOutputDevice.
* @tc.number: MoveToRemoteOutputDevice_003
* @tc.desc  : Test MoveToRemoteOutputDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, MoveToRemoteOutputDevice_003, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    std::vector<SinkInput> sinkInputs;
    SinkInput sinkInput = {};
    sinkInputs.push_back(sinkInput);

    std::string moduleName = "123";
    std::shared_ptr<AudioDeviceDescriptor> remoteDevice = std::make_shared<AudioDeviceDescriptor>();
    remoteDevice->deviceType_ = DEVICE_TYPE_REMOTE_CAST;
    remoteDevice->macAddress_ = "00:11:22:33:44:55";
    remoteDevice->networkId_ = REMOTE_NETWORK_ID;

    audioDeviceCommon.audioIOHandleMap_.IOHandles_.clear();
    audioDeviceCommon.isOpenRemoteDevice = true;

    int32_t result = audioDeviceCommon.MoveToRemoteOutputDevice(sinkInputs, remoteDevice);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_FALSE(audioDeviceCommon.audioIOHandleMap_.CheckIOHandleExist(moduleName));
}

/**
* @tc.name  : Test ScoInputDeviceFetchedForRecongnition.
* @tc.number: ScoInputDeviceFetchedForRecongnition_001
* @tc.desc  : Test ScoInputDeviceFetchedForRecongnition interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, ScoInputDeviceFetchedForRecongnition_001, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    bool handleFlag = false;
    std::string address = "00:11:22:33:44:55";
    ConnectState connectState = DEACTIVE_CONNECTED;

    int32_t result = audioDeviceCommon.ScoInputDeviceFetchedForRecongnition(handleFlag, address, connectState);
    EXPECT_EQ(result, ERROR);
}

/**
* @tc.name  : Test ScoInputDeviceFetchedForRecongnition.
* @tc.number: ScoInputDeviceFetchedForRecongnition_002
* @tc.desc  : Test ScoInputDeviceFetchedForRecongnition interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, ScoInputDeviceFetchedForRecongnition_002, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    bool handleFlag = true;
    std::string address = "00:11:22:33:44:55";
    ConnectState connectState = DEACTIVE_CONNECTED;

    int32_t result = audioDeviceCommon.ScoInputDeviceFetchedForRecongnition(handleFlag, address, connectState);
    EXPECT_EQ(result, ERROR);
}

/**
* @tc.name  : Test MoveToRemoteInputDevice.
* @tc.number: MoveToRemoteInputDevice_001
* @tc.desc  : Test MoveToRemoteInputDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, MoveToRemoteInputDevice_001, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    std::vector<SourceOutput> sourceOutputs;
    SourceOutput sourceOutput = {1};
    sourceOutputs.push_back(sourceOutput);

    std::shared_ptr<AudioDeviceDescriptor> remoteDevice = std::make_shared<AudioDeviceDescriptor>();
    remoteDevice->deviceType_ = DEVICE_TYPE_REMOTE_CAST;
    remoteDevice->macAddress_ = "00:11:22:33:44:55";
    remoteDevice->networkId_ = REMOTE_NETWORK_ID;

    std::string moduleName = "123";
    AudioIOHandle moduleId = 1;
    audioDeviceCommon.audioIOHandleMap_.AddIOHandleInfo(moduleName, moduleId);

    int32_t result = audioDeviceCommon.MoveToRemoteInputDevice(sourceOutputs, remoteDevice);
    EXPECT_EQ(result, ERR_OPERATION_FAILED);
    EXPECT_TRUE(audioDeviceCommon.audioIOHandleMap_.CheckIOHandleExist(moduleName));
}

/**
* @tc.name  : Test MoveToRemoteInputDevice.
* @tc.number: MoveToRemoteInputDevice_002
* @tc.desc  : Test MoveToRemoteInputDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, MoveToRemoteInputDevice_002, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    std::vector<SourceOutput> sourceOutputs;
    SourceOutput sourceOutput = {1};
    sourceOutputs.push_back(sourceOutput);

    std::string moduleName = "123";
    std::shared_ptr<AudioDeviceDescriptor> remoteDevice = std::make_shared<AudioDeviceDescriptor>();
    remoteDevice->deviceType_ = DEVICE_TYPE_REMOTE_CAST;
    remoteDevice->macAddress_ = "00:11:22:33:44:55";
    remoteDevice->networkId_ = REMOTE_NETWORK_ID;

    audioDeviceCommon.audioIOHandleMap_.IOHandles_.clear();
    audioDeviceCommon.isOpenRemoteDevice = false;

    int32_t result = audioDeviceCommon.MoveToRemoteInputDevice(sourceOutputs, remoteDevice);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
    EXPECT_FALSE(audioDeviceCommon.audioIOHandleMap_.CheckIOHandleExist(moduleName));
}

/**
* @tc.name  : Test MoveToRemoteInputDevice.
* @tc.number: MoveToRemoteInputDevice_003
* @tc.desc  : Test MoveToRemoteInputDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, MoveToRemoteInputDevice_003, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    std::vector<SourceOutput> sourceOutputs;
    SourceOutput sourceOutput = {1};
    sourceOutputs.push_back(sourceOutput);

    std::string moduleName = "123";
    std::shared_ptr<AudioDeviceDescriptor> remoteDevice = std::make_shared<AudioDeviceDescriptor>();
    remoteDevice->deviceType_ = DEVICE_TYPE_REMOTE_CAST;
    remoteDevice->macAddress_ = "00:11:22:33:44:55";
    remoteDevice->networkId_ = REMOTE_NETWORK_ID;

    audioDeviceCommon.audioIOHandleMap_.IOHandles_.clear();
    audioDeviceCommon.isOpenRemoteDevice = true;

    int32_t result = audioDeviceCommon.MoveToRemoteInputDevice(sourceOutputs, remoteDevice);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_FALSE(audioDeviceCommon.audioIOHandleMap_.CheckIOHandleExist(moduleName));
}

/**
* @tc.name  : Test IsSameDevice
* @tc.number: IsSameDevice_001
* @tc.desc  : Test IsSameDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, IsSameDevice_001, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->networkId_ = "LocalNetworkId";
    desc->deviceType_ = DEVICE_TYPE_USB_HEADSET;
    desc->macAddress_ = "00:11:22:33:44:55";
    desc->connectState_ = CONNECTED;
    desc->deviceRole_ = INPUT_DEVICE;

    const AudioDeviceDescriptor deviceDesc = AudioDeviceDescriptor(*desc);
    bool result = audioDeviceCommon.IsSameDevice(desc, deviceDesc);
    EXPECT_TRUE(result);
}

/**
* @tc.name  : Test IsSameDevice
* @tc.number: IsSameDevice_002
* @tc.desc  : Test IsSameDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, IsSameDevice_002, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->networkId_ = "LocalNetworkId";
    desc->deviceType_ = DEVICE_TYPE_NONE;
    desc->macAddress_ = "00:11:22:33:44:55";
    desc->connectState_ = CONNECTED;
    desc->deviceRole_ = INPUT_DEVICE;

    const AudioDeviceDescriptor deviceDesc = AudioDeviceDescriptor(*desc);
    bool result = audioDeviceCommon.IsSameDevice(desc, deviceDesc);
    EXPECT_TRUE(result);
}

/**
* @tc.name  : Test IsSameDevice
* @tc.number: IsSameDevice_003
* @tc.desc  : Test IsSameDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, IsSameDevice_003, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->networkId_ = "LocalNetworkId";
    desc->deviceType_ = DEVICE_TYPE_NONE;
    desc->macAddress_ = "00:11:22:33:44:55";
    desc->connectState_ = CONNECTED;
    desc->deviceRole_ = INPUT_DEVICE;

    const AudioDeviceDescriptor deviceDesc = AudioDeviceDescriptor(*desc);
    const_cast<AudioDeviceDescriptor&>(deviceDesc).deviceRole_ = OUTPUT_DEVICE;
    const_cast<AudioDeviceDescriptor&>(deviceDesc).networkId_ = "RemoteNetworkId";
    bool result = audioDeviceCommon.IsSameDevice(desc, deviceDesc);
    EXPECT_FALSE(result);
}

/**
* @tc.name  : Test IsSameDevice.
* @tc.number: IsSameDevice_004
* @tc.desc  : Test IsSameDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, IsSameDevice_004, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->networkId_ = "LocalNetworkId";
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    desc->macAddress_ = "00:11:22:33:44:55";
    desc->connectState_ = CONNECTED;
    desc->a2dpOffloadFlag_ = A2DP_OFFLOAD;

    AudioDeviceDescriptor deviceInfo = AudioDeviceDescriptor(*desc);
    deviceInfo.a2dpOffloadFlag_ = A2DP_OFFLOAD;
    deviceInfo.descriptorType_ = DEVICE_TYPE_BLUETOOTH_A2DP;

    audioDeviceCommon.audioA2dpOffloadFlag_.SetA2dpOffloadFlag(A2DP_NOT_OFFLOAD);
    bool result = audioDeviceCommon.IsSameDevice(desc, deviceInfo);
    EXPECT_FALSE(result);
}

/**
* @tc.name  : Test IsSameDevice.
* @tc.number: IsSameDevice_005
* @tc.desc  : Test IsSameDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, IsSameDevice_005, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->networkId_ = "LocalNetworkId";
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    desc->macAddress_ = "00:11:22:33:44:55";
    desc->connectState_ = CONNECTED;
    desc->a2dpOffloadFlag_ = A2DP_NOT_OFFLOAD;

    AudioDeviceDescriptor deviceInfo = AudioDeviceDescriptor(*desc);
    deviceInfo.descriptorType_ = DEVICE_TYPE_BLUETOOTH_A2DP;

    audioDeviceCommon.audioA2dpOffloadFlag_.SetA2dpOffloadFlag(A2DP_OFFLOAD);
    bool result = audioDeviceCommon.IsSameDevice(desc, deviceInfo);
    EXPECT_FALSE(result);
}

/**
* @tc.name  : Test IsSameDevice.
* @tc.number: IsSameDevice_006
* @tc.desc  : Test IsSameDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, IsSameDevice_006, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->networkId_ = "LocalNetworkId";
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    desc->macAddress_ = "00:11:22:33:44:55";
    desc->connectState_ = CONNECTED;
    desc->a2dpOffloadFlag_ = A2DP_OFFLOAD;

    AudioDeviceDescriptor deviceInfo = AudioDeviceDescriptor(*desc);
    deviceInfo.descriptorType_ = DEVICE_TYPE_BLUETOOTH_A2DP;

    audioDeviceCommon.audioA2dpOffloadFlag_.SetA2dpOffloadFlag(A2DP_OFFLOAD);
    bool result = audioDeviceCommon.IsSameDevice(desc, deviceInfo);
    EXPECT_TRUE(result);
}

/**
* @tc.name  : Test ClientDiedDisconnectScoNormal.
* @tc.number: ClientDiedDisconnectScoNormal_001
* @tc.desc  : Test ClientDiedDisconnectScoNormal interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, ClientDiedDisconnectScoNormal_001, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    AudioDeviceDescriptor outputDevice;
    outputDevice.deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    audioDeviceCommon.audioActiveDevice_.SetCurrentOutputDevice(outputDevice);

    audioDeviceCommon.streamCollector_.audioRendererChangeInfos_.clear();
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->rendererState = RENDERER_RUNNING;
    rendererChangeInfo->rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
    audioDeviceCommon.streamCollector_.audioRendererChangeInfos_.push_back(rendererChangeInfo);

    audioDeviceCommon.ClientDiedDisconnectScoNormal();
    EXPECT_EQ(audioDeviceCommon.audioActiveDevice_.GetCurrentOutputDeviceType(), DEVICE_TYPE_BLUETOOTH_SCO);
}

/**
* @tc.name  : Test ClientDiedDisconnectScoNormal.
* @tc.number: ClientDiedDisconnectScoNormal_002
* @tc.desc  : Test ClientDiedDisconnectScoNormal interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, ClientDiedDisconnectScoNormal_002, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    AudioDeviceDescriptor outputDevice;
    outputDevice.deviceType_ = DEVICE_TYPE_SPEAKER;
    audioDeviceCommon.audioActiveDevice_.SetCurrentOutputDevice(outputDevice);

    audioDeviceCommon.streamCollector_.audioRendererChangeInfos_.clear();
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->rendererState = RENDERER_RUNNING;
    rendererChangeInfo->rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
    audioDeviceCommon.streamCollector_.audioRendererChangeInfos_.push_back(rendererChangeInfo);

    audioDeviceCommon.ClientDiedDisconnectScoNormal();
    EXPECT_EQ(audioDeviceCommon.audioActiveDevice_.GetCurrentOutputDeviceType(), DEVICE_TYPE_SPEAKER);
}

/**
* @tc.name  : Test ClientDiedDisconnectScoRecognition.
* @tc.number: ClientDiedDisconnectScoRecognition_001
* @tc.desc  : Test ClientDiedDisconnectScoRecognition interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, ClientDiedDisconnectScoRecognition_001, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    std::shared_ptr<AudioCapturerChangeInfo> capturerChangeInfo = std::make_shared<AudioCapturerChangeInfo>();
    capturerChangeInfo->capturerState = CAPTURER_RUNNING;
    capturerChangeInfo->capturerInfo.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    audioDeviceCommon.streamCollector_.audioCapturerChangeInfos_.push_back(capturerChangeInfo);

    audioDeviceCommon.ClientDiedDisconnectScoRecognition();
    EXPECT_TRUE(audioDeviceCommon.streamCollector_.HasRunningRecognitionCapturerStream());
}

/**
* @tc.name  : Test ClientDiedDisconnectScoRecognition.
* @tc.number: ClientDiedDisconnectScoRecognition_002
* @tc.desc  : Test ClientDiedDisconnectScoRecognition interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, ClientDiedDisconnectScoRecognition_002, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    AudioDeviceDescriptor inputDevice;
    inputDevice.deviceType_ = DEVICE_TYPE_MIC;
    audioDeviceCommon.audioActiveDevice_.SetCurrentInputDevice(inputDevice);

    audioDeviceCommon.streamCollector_.audioCapturerChangeInfos_.clear();
    audioDeviceCommon.ClientDiedDisconnectScoRecognition();
    EXPECT_EQ(audioDeviceCommon.audioActiveDevice_.GetCurrentInputDeviceType(), DEVICE_TYPE_MIC);
}

/**
* @tc.name  : Test GetA2dpModuleInfo.
* @tc.number: GetA2dpModuleInfo_001
* @tc.desc  : Test GetA2dpModuleInfo function, entering the if branch.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, GetA2dpModuleInfo_001, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    AudioModuleInfo moduleInfo;
    AudioStreamInfo audioStreamInfo;
    audioStreamInfo.samplingRate = SAMPLE_RATE_48000;
    audioStreamInfo.format = SAMPLE_S16LE;
    audioStreamInfo.channels = STEREO;


    moduleInfo.role = "sink";
    audioDeviceCommon.GetA2dpModuleInfo(moduleInfo, audioStreamInfo, SOURCE_TYPE_MIC);
    EXPECT_EQ(moduleInfo.channels, "2");
    EXPECT_EQ(moduleInfo.rate, "48000");
    EXPECT_EQ(moduleInfo.format, "s16le");
    EXPECT_EQ(moduleInfo.renderInIdleState, "1");
    EXPECT_EQ(moduleInfo.sinkLatency, "0");
}

/**
* @tc.name  : Test GetA2dpModuleInfo.
* @tc.number: GetA2dpModuleInfo_002
* @tc.desc  : Test GetA2dpModuleInfo function, not entering the if branch.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, GetA2dpModuleInfo_002, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    AudioModuleInfo moduleInfo;
    AudioStreamInfo audioStreamInfo;
    audioStreamInfo.samplingRate = SAMPLE_RATE_44100;
    audioStreamInfo.format = SAMPLE_S24LE;
    audioStreamInfo.channels = MONO;

    moduleInfo.role = "source";
    audioDeviceCommon.GetA2dpModuleInfo(moduleInfo, audioStreamInfo, SOURCE_TYPE_MIC);
    EXPECT_EQ(moduleInfo.channels, "1");
    EXPECT_EQ(moduleInfo.rate, "44100");
    EXPECT_EQ(moduleInfo.format, "s24le");
    EXPECT_EQ(moduleInfo.renderInIdleState, "");
    EXPECT_EQ(moduleInfo.sinkLatency, "");
}

/**
* @tc.name  : Test LoadA2dpModule
* @tc.number: LoadA2dpModule_001
* @tc.desc  : Test LoadA2dpModule interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, LoadA2dpModule_001, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    AudioStreamInfo audioStreamInfo;
    audioStreamInfo.samplingRate = SAMPLE_RATE_8000;
    audioStreamInfo.encoding = ENCODING_PCM;
    audioStreamInfo.format = SAMPLE_S24LE;
    audioStreamInfo.channels = STEREO;

    DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    std::string networkID = "LocalNetworkId";
    std::string sinkName = "bt_a2dp_sink";
    SourceType sourceType = SOURCE_TYPE_MIC;

    AudioModuleInfo moduleInfo;
    moduleInfo.name = "bt_a2dp";
    moduleInfo.role = "sink";
    std::list<AudioModuleInfo> moduleInfoList;
    moduleInfoList.push_back(moduleInfo);

    audioDeviceCommon.audioConfigManager_.deviceClassInfo_[ClassType::TYPE_A2DP] = moduleInfoList;
    int32_t result = audioDeviceCommon.LoadA2dpModule(deviceType, audioStreamInfo, networkID, sinkName, sourceType);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test SwitchActiveA2dpDevice
* @tc.number: SwitchActiveA2dpDevice_001
* @tc.desc  : Test SwitchActiveA2dpDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, SwitchActiveA2dpDevice_001, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    deviceDescriptor->macAddress_ = Bluetooth::AudioA2dpManager::GetActiveA2dpDevice();
    deviceDescriptor->deviceName_ = "TestA2dpDevice";
    deviceDescriptor->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioDeviceCommon.audioA2dpDevice_.connectedA2dpDeviceMap_[deviceDescriptor->macAddress_] = A2dpDeviceConfigInfo();

    audioDeviceCommon.audioIOHandleMap_.IOHandles_[BLUETOOTH_SPEAKER] = 12345;
    int32_t result = audioDeviceCommon.SwitchActiveA2dpDevice(deviceDescriptor);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test SwitchActiveA2dpDevice
* @tc.number: SwitchActiveA2dpDevice_002
* @tc.desc  : Test SwitchActiveA2dpDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, SwitchActiveA2dpDevice_002, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    deviceDescriptor->macAddress_ = "00:11:22:33:44:55";
    deviceDescriptor->deviceName_ = "TestA2dpDevice";
    deviceDescriptor->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioDeviceCommon.audioA2dpDevice_.connectedA2dpDeviceMap_[deviceDescriptor->macAddress_] = A2dpDeviceConfigInfo();

    audioDeviceCommon.audioIOHandleMap_.IOHandles_.clear();
    int32_t result = audioDeviceCommon.SwitchActiveA2dpDevice(deviceDescriptor);
    EXPECT_EQ(result, ERROR);
}
} // namespace AudioStandard
} // namespace OHOS