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
} // namespace AudioStandard
} // namespace OHOS