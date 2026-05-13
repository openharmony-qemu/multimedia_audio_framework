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
#include "audio_router_select_strategy.h"

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
* @tc.name  : Test UpdateDeviceInfo.
* @tc.number: UpdateDeviceInfo_001
* @tc.desc  : Test UpdateDeviceInfo interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, UpdateDeviceInfo_001, TestSize.Level4)
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
HWTEST_F(AudioDeviceCommonUnitNextTest, UpdateDeviceInfo_002, TestSize.Level4)
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
HWTEST_F(AudioDeviceCommonUnitNextTest, UpdateConnectedDevicesWhenDisconnecting_001, TestSize.Level4)
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
    AudioRouterSelectStrategy::GetInstance().SetMediaOutputDevice(
        SYSTEM_UID, INVALID_STREAM_ID, preferredMediaRenderDevice);

    audioDeviceCommon.UpdateConnectedDevicesWhenDisconnecting(updatedDesc, descForCb);
    EXPECT_EQ(desc->deviceType_, DEVICE_TYPE_DP);
}

/**
* @tc.name  : Test UpdateConnectedDevicesWhenDisconnecting.
* @tc.number: UpdateConnectedDevicesWhenDisconnecting_002
* @tc.desc  : Test UpdateDeviceInfo interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, UpdateConnectedDevicesWhenDisconnecting_002, TestSize.Level4)
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
    AudioRouterSelectStrategy::GetInstance().SetCallInputDevice(
        SYSTEM_UID, INVALID_STREAM_ID, preferredCallCaptureDevice);

    audioDeviceCommon.UpdateConnectedDevicesWhenDisconnecting(updatedDesc, descForCb);
    EXPECT_EQ(desc->deviceType_, DEVICE_TYPE_USB_HEADSET);
}

/**
* @tc.name  : Test UpdateConnectedDevicesWhenDisconnecting.
* @tc.number: UpdateConnectedDevicesWhenDisconnecting_003
* @tc.desc  : Test UpdateDeviceInfo interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, UpdateConnectedDevicesWhenDisconnecting_003, TestSize.Level4)
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
    AudioRouterSelectStrategy::GetInstance().SetMediaInputDevice(
        SYSTEM_UID, INVALID_STREAM_ID, preferredRecordCaptureDevice);

    audioDeviceCommon.UpdateConnectedDevicesWhenDisconnecting(updatedDesc, descForCb);
    EXPECT_EQ(desc->deviceType_, DEVICE_TYPE_USB_ARM_HEADSET);
}

/**
* @tc.name  : Test IsSameDevice
* @tc.number: IsSameDevice_001
* @tc.desc  : Test IsSameDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, IsSameDevice_001, TestSize.Level4)
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
HWTEST_F(AudioDeviceCommonUnitNextTest, IsSameDevice_002, TestSize.Level4)
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
HWTEST_F(AudioDeviceCommonUnitNextTest, IsSameDevice_003, TestSize.Level4)
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
HWTEST_F(AudioDeviceCommonUnitNextTest, IsSameDevice_004, TestSize.Level4)
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
HWTEST_F(AudioDeviceCommonUnitNextTest, IsSameDevice_005, TestSize.Level4)
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
HWTEST_F(AudioDeviceCommonUnitNextTest, IsSameDevice_006, TestSize.Level4)
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
* @tc.name  : Test ClientDiedDisconnectScoRecognition.
* @tc.number: ClientDiedDisconnectScoRecognition_001
* @tc.desc  : Test ClientDiedDisconnectScoRecognition interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, ClientDiedDisconnectScoRecognition_001, TestSize.Level4)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    std::shared_ptr<AudioCapturerChangeInfo> capturerChangeInfo = std::make_shared<AudioCapturerChangeInfo>();
    capturerChangeInfo->capturerState = CAPTURER_RUNNING;
    capturerChangeInfo->capturerInfo.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    audioDeviceCommon.streamCollector_.audioCapturerChangeInfos_.push_back(capturerChangeInfo);

    audioDeviceCommon.ClientDiedDisconnectScoRecognition();
    EXPECT_TRUE(audioDeviceCommon.pipeManager_->HasRunningRecognitionCapturerStream());
}

/**
* @tc.name  : Test ClientDiedDisconnectScoRecognition.
* @tc.number: ClientDiedDisconnectScoRecognition_002
* @tc.desc  : Test ClientDiedDisconnectScoRecognition interface.
*/
HWTEST_F(AudioDeviceCommonUnitNextTest, ClientDiedDisconnectScoRecognition_002, TestSize.Level4)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.DeInit();

    AudioDeviceDescriptor inputDevice;
    inputDevice.deviceType_ = DEVICE_TYPE_MIC;
    inputDevice.deviceId_ = 1001;
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(
        make_shared<AudioDeviceDescriptor>(inputDevice));
    AudioRouterSelectStrategy::GetInstance().UpdateCurrentInputDevice(SYSTEM_UID,
        {make_shared<AudioDeviceDescriptor>(inputDevice)});

    audioDeviceCommon.streamCollector_.audioCapturerChangeInfos_.clear();
    audioDeviceCommon.ClientDiedDisconnectScoRecognition();
    EXPECT_EQ(audioDeviceCommon.audioActiveDevice_.GetCurrentInputDeviceType(), DEVICE_TYPE_MIC);
}
} // namespace AudioStandard
} // namespace OHOS