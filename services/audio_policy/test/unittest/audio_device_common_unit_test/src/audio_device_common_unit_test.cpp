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

#include "audio_device_common_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
static const int32_t GET_RESULT_NO_VALUE = 0;
static const int32_t GET_RESULT_HAS_VALUE = 1;

void AudioDeviceCommonUnitTest::SetUpTestCase(void) {}
void AudioDeviceCommonUnitTest::TearDownTestCase(void) {}
void AudioDeviceCommonUnitTest::SetUp(void) {}
void AudioDeviceCommonUnitTest::TearDown(void) {}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_001
* @tc.desc  : Test AudioDeviceCommon interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_001, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.audioPolicyServerHandler_ = nullptr;
    AudioDeviceDescriptor deviceDescriptor;
    audioDeviceCommon.OnPreferredOutputDeviceUpdated(deviceDescriptor, AudioStreamDeviceChangeReason::UNKNOWN);
    EXPECT_NE(0, audioDeviceCommon.spatialDeviceMap_.size());

    DeviceType deviceType = DEVICE_TYPE_NONE;
    audioDeviceCommon.OnPreferredInputDeviceUpdated(deviceType, "");
    EXPECT_EQ(nullptr, audioDeviceCommon.audioPolicyServerHandler_);

    AudioRendererInfo rendererInfo;
    rendererInfo.streamUsage = static_cast<StreamUsage>(1000);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceOutputList =
        audioDeviceCommon.GetPreferredOutputDeviceDescInner(rendererInfo, "");
    EXPECT_EQ(1, deviceOutputList.size());

    AudioCapturerInfo captureInfo;
    captureInfo.sourceType = SOURCE_TYPE_INVALID;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceInputList =
        audioDeviceCommon.GetPreferredInputDeviceDescInner(captureInfo, "");
    EXPECT_EQ(1, deviceOutputList.size());
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_002
* @tc.desc  : Test IsRingerOrAlarmerDualDevicesRange interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_002, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    InternalDeviceType deviceType = DEVICE_TYPE_SPEAKER;
    bool ret = audioDeviceCommon.IsRingerOrAlarmerDualDevicesRange(deviceType);
    EXPECT_EQ(true, ret);

    deviceType = DEVICE_TYPE_WIRED_HEADSET;
    ret = audioDeviceCommon.IsRingerOrAlarmerDualDevicesRange(deviceType);
    EXPECT_EQ(true, ret);

    deviceType = DEVICE_TYPE_WIRED_HEADPHONES;
    ret = audioDeviceCommon.IsRingerOrAlarmerDualDevicesRange(deviceType);
    EXPECT_EQ(true, ret);

    deviceType = DEVICE_TYPE_BLUETOOTH_SCO;
    ret = audioDeviceCommon.IsRingerOrAlarmerDualDevicesRange(deviceType);
    EXPECT_EQ(true, ret);

    deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    ret = audioDeviceCommon.IsRingerOrAlarmerDualDevicesRange(deviceType);
    EXPECT_EQ(true, ret);

    deviceType = DEVICE_TYPE_USB_HEADSET;
    ret = audioDeviceCommon.IsRingerOrAlarmerDualDevicesRange(deviceType);
    EXPECT_EQ(true, ret);

    deviceType = DEVICE_TYPE_USB_ARM_HEADSET;
    ret = audioDeviceCommon.IsRingerOrAlarmerDualDevicesRange(deviceType);
    EXPECT_EQ(true, ret);

    deviceType = DEVICE_TYPE_REMOTE_CAST;
    ret = audioDeviceCommon.IsRingerOrAlarmerDualDevicesRange(deviceType);
    EXPECT_EQ(true, ret);

    deviceType = DEVICE_TYPE_EARPIECE;
    ret = audioDeviceCommon.IsRingerOrAlarmerDualDevicesRange(deviceType);
    EXPECT_EQ(false, ret);

    deviceType = DEVICE_TYPE_BLUETOOTH_A2DP_IN;
    ret = audioDeviceCommon.IsRingerOrAlarmerDualDevicesRange(deviceType);
    EXPECT_EQ(false, ret);

    deviceType = DEVICE_TYPE_MIC;
    ret = audioDeviceCommon.IsRingerOrAlarmerDualDevicesRange(deviceType);
    EXPECT_EQ(false, ret);

    deviceType = DEVICE_TYPE_DP;
    ret = audioDeviceCommon.IsRingerOrAlarmerDualDevicesRange(deviceType);
    EXPECT_EQ(false, ret);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_003
* @tc.desc  : Test GetPreferredOutputDeviceDescInner interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_003, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.audioPolicyServerHandler_ = nullptr;
    AudioDeviceDescriptor deviceDescriptor;
    audioDeviceCommon.OnPreferredOutputDeviceUpdated(deviceDescriptor, AudioStreamDeviceChangeReason::UNKNOWN);

    DeviceType deviceType = DEVICE_TYPE_NONE;
    audioDeviceCommon.OnPreferredInputDeviceUpdated(deviceType, "");

    AudioRendererInfo rendererInfo;
    rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> ret =
        audioDeviceCommon.GetPreferredOutputDeviceDescInner(rendererInfo, "LocalDevice");
    EXPECT_EQ(GET_RESULT_HAS_VALUE, ret.size());

    AudioCapturerInfo captureInfo;
    captureInfo.sourceType = SOURCE_TYPE_MAX;
    ret = audioDeviceCommon.GetPreferredInputDeviceDescInner(captureInfo, "");
    EXPECT_EQ(GET_RESULT_NO_VALUE, ret.size());
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_004
* @tc.desc  : Test GetPreferredOutputDeviceDescInner interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_004, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.audioPolicyServerHandler_ = nullptr;
    AudioDeviceDescriptor deviceDescriptor;
    audioDeviceCommon.OnPreferredOutputDeviceUpdated(deviceDescriptor, AudioStreamDeviceChangeReason::UNKNOWN);

    DeviceType deviceType = DEVICE_TYPE_NONE;
    audioDeviceCommon.OnPreferredInputDeviceUpdated(deviceType, "");

    AudioRendererInfo rendererInfo;
    rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> ret =
        audioDeviceCommon.GetPreferredOutputDeviceDescInner(rendererInfo, "");
    EXPECT_EQ(GET_RESULT_NO_VALUE, ret.size());

    AudioCapturerInfo captureInfo;
    captureInfo.sourceType = SOURCE_TYPE_WAKEUP;
    ret = audioDeviceCommon.GetPreferredInputDeviceDescInner(captureInfo, "");
    EXPECT_EQ(GET_RESULT_HAS_VALUE, ret.size());
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_005
* @tc.desc  : Test GetPreferredInputDeviceDescInner interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_005, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.audioPolicyServerHandler_ = nullptr;
    AudioDeviceDescriptor deviceDescriptor;
    audioDeviceCommon.OnPreferredOutputDeviceUpdated(deviceDescriptor, AudioStreamDeviceChangeReason::UNKNOWN);

    DeviceType deviceType = DEVICE_TYPE_NONE;
    audioDeviceCommon.OnPreferredInputDeviceUpdated(deviceType, "");

    AudioRendererInfo rendererInfo;
    rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> ret =
        audioDeviceCommon.GetPreferredOutputDeviceDescInner(rendererInfo, "");
    EXPECT_EQ(GET_RESULT_NO_VALUE, ret.size());

    AudioCapturerInfo captureInfo;
    captureInfo.sourceType = SOURCE_TYPE_MIC;
    ret = audioDeviceCommon.GetPreferredInputDeviceDescInner(captureInfo, "LocalDevice");
    EXPECT_EQ(GET_RESULT_HAS_VALUE, ret.size());
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_006
* @tc.desc  : Test GetPreferredInputDeviceDescInner interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_006, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.audioPolicyServerHandler_ = nullptr;
    AudioDeviceDescriptor deviceDescriptor;
    audioDeviceCommon.OnPreferredOutputDeviceUpdated(deviceDescriptor, AudioStreamDeviceChangeReason::UNKNOWN);

    DeviceType deviceType = DEVICE_TYPE_NONE;
    audioDeviceCommon.OnPreferredInputDeviceUpdated(deviceType, "");

    AudioRendererInfo rendererInfo;
    rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> ret =
        audioDeviceCommon.GetPreferredOutputDeviceDescInner(rendererInfo, "");
    EXPECT_EQ(GET_RESULT_NO_VALUE, ret.size());

    AudioCapturerInfo captureInfo;
    captureInfo.sourceType = SOURCE_TYPE_MIC;
    ret = audioDeviceCommon.GetPreferredInputDeviceDescInner(captureInfo, "");
    EXPECT_EQ(GET_RESULT_NO_VALUE, ret.size());
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_010
* @tc.desc  : Test IsRingerOrAlarmerDualDevicesRange interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_010, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    DeviceType deviceType = DEVICE_TYPE_EARPIECE;
    bool isRemote = true;
    bool ret = audioDeviceCommon.HasLowLatencyCapability(deviceType, isRemote);
    EXPECT_EQ(true, ret);

    isRemote = false;
    deviceType = DEVICE_TYPE_EARPIECE;
    ret = audioDeviceCommon.HasLowLatencyCapability(deviceType, isRemote);
    EXPECT_EQ(true, ret);

    isRemote = false;
    deviceType = DEVICE_TYPE_SPEAKER;
    ret = audioDeviceCommon.HasLowLatencyCapability(deviceType, isRemote);
    EXPECT_EQ(true, ret);

    isRemote = false;
    deviceType = DEVICE_TYPE_WIRED_HEADSET;
    ret = audioDeviceCommon.HasLowLatencyCapability(deviceType, isRemote);
    EXPECT_EQ(true, ret);

    isRemote = false;
    deviceType = DEVICE_TYPE_WIRED_HEADPHONES;
    ret = audioDeviceCommon.HasLowLatencyCapability(deviceType, isRemote);
    EXPECT_EQ(true, ret);

    isRemote = false;
    deviceType = DEVICE_TYPE_USB_HEADSET;
    ret = audioDeviceCommon.HasLowLatencyCapability(deviceType, isRemote);
    EXPECT_EQ(true, ret);

    isRemote = false;
    deviceType = DEVICE_TYPE_DP;
    ret = audioDeviceCommon.HasLowLatencyCapability(deviceType, isRemote);
    EXPECT_EQ(true, ret);

    isRemote = false;
    deviceType = DEVICE_TYPE_BLUETOOTH_SCO;
    ret = audioDeviceCommon.HasLowLatencyCapability(deviceType, isRemote);
    EXPECT_EQ(false, ret);

    isRemote = false;
    deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    ret = audioDeviceCommon.HasLowLatencyCapability(deviceType, isRemote);
    EXPECT_EQ(false, ret);

    isRemote = false;
    deviceType = DEVICE_TYPE_BLUETOOTH_A2DP_IN;
    ret = audioDeviceCommon.HasLowLatencyCapability(deviceType, isRemote);
    EXPECT_EQ(false, ret);

    isRemote = false;
    deviceType = DEVICE_TYPE_MIC;
    ret = audioDeviceCommon.HasLowLatencyCapability(deviceType, isRemote);
    EXPECT_EQ(false, ret);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_020
* @tc.desc  : Test UpdateDeviceInfo interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_020, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    AudioDeviceDescriptor deviceInfo;
    bool hasBTPermission = true;
    bool hasSystemPermission = true;
    BluetoothOffloadState state = NO_A2DP_DEVICE;
    audioDeviceCommon.audioA2dpOffloadFlag_.SetA2dpOffloadFlag(state);
    audioDeviceCommon.UpdateDeviceInfo(deviceInfo, std::make_shared<AudioDeviceDescriptor>(),
        hasBTPermission, hasSystemPermission);
    EXPECT_EQ(NO_A2DP_DEVICE, deviceInfo.a2dpOffloadFlag_);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_021
* @tc.desc  : Test UpdateDeviceInfo interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_021, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    AudioDeviceDescriptor deviceInfo;
    bool hasBTPermission = false;
    bool hasSystemPermission = true;
    BluetoothOffloadState state = NO_A2DP_DEVICE;
    audioDeviceCommon.audioA2dpOffloadFlag_.SetA2dpOffloadFlag(state);
    audioDeviceCommon.UpdateDeviceInfo(deviceInfo, std::make_shared<AudioDeviceDescriptor>(),
        hasBTPermission, hasSystemPermission);
    EXPECT_EQ(CATEGORY_DEFAULT, deviceInfo.deviceCategory_);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_022
* @tc.desc  : Test UpdateDeviceInfo interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_022, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    AudioDeviceDescriptor deviceInfo;
    bool hasBTPermission = false;
    bool hasSystemPermission = false;
    BluetoothOffloadState state = NO_A2DP_DEVICE;
    audioDeviceCommon.audioA2dpOffloadFlag_.SetA2dpOffloadFlag(state);
    audioDeviceCommon.UpdateDeviceInfo(deviceInfo, std::make_shared<AudioDeviceDescriptor>(),
        hasBTPermission, hasSystemPermission);
    EXPECT_EQ(GROUP_ID_NONE, deviceInfo.volumeGroupId_);
    EXPECT_EQ(GROUP_ID_NONE, deviceInfo.interruptGroupId_);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_023
* @tc.desc  : Test DeviceParamsCheck interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_023, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    DeviceRole targetRole = INPUT_DEVICE;
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorSptr = std::make_shared<AudioDeviceDescriptor>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptorSptrVector;
    audioDeviceDescriptorSptrVector.push_back(audioDeviceDescriptorSptr);
    int32_t ret = audioDeviceCommon.DeviceParamsCheck(targetRole, audioDeviceDescriptorSptrVector);
    EXPECT_EQ(ERR_INVALID_OPERATION, ret);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_024
* @tc.desc  : Test DeviceParamsCheck interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_024, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    DeviceRole targetRole = OUTPUT_DEVICE;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> sharedAudioDeviceDescriptors;
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorSptr = std::make_shared<AudioDeviceDescriptor>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptorSptrVector;
    audioDeviceDescriptorSptrVector.push_back(audioDeviceDescriptorSptr);
    int32_t ret = audioDeviceCommon.DeviceParamsCheck(targetRole, audioDeviceDescriptorSptrVector);
    EXPECT_EQ(ERR_INVALID_OPERATION, ret);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_026
* @tc.desc  : Test UpdateConnectedDevicesWhenConnecting interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_026, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    AudioDeviceDescriptor updatedDesc;
    updatedDesc.deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;
    updatedDesc.deviceRole_ = INPUT_DEVICE;
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorSptr = std::make_shared<AudioDeviceDescriptor>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptorSptrVector;
    audioDeviceDescriptorSptrVector.push_back(audioDeviceDescriptorSptr);
    audioDeviceCommon.UpdateConnectedDevicesWhenConnecting(updatedDesc, audioDeviceDescriptorSptrVector);
    audioDeviceCommon.RemoveOfflineDevice(updatedDesc);
    EXPECT_EQ(2, audioDeviceDescriptorSptrVector.size());
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_027
* @tc.desc  : Test UpdateConnectedDevicesWhenConnecting interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_027, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    AudioDeviceDescriptor updatedDesc;
    updatedDesc.deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;
    updatedDesc.deviceRole_ = OUTPUT_DEVICE;
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorSptr = std::make_shared<AudioDeviceDescriptor>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptorSptrVector;
    audioDeviceDescriptorSptrVector.push_back(audioDeviceDescriptorSptr);
    audioDeviceCommon.UpdateConnectedDevicesWhenConnecting(updatedDesc, audioDeviceDescriptorSptrVector);
    audioDeviceCommon.RemoveOfflineDevice(updatedDesc);
    EXPECT_EQ(2, audioDeviceDescriptorSptrVector.size());
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_028
* @tc.desc  : Test UpdateConnectedDevicesWhenDisconnecting interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_028, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    AudioDeviceDescriptor updatedDesc;
    updatedDesc.deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;
    updatedDesc.deviceRole_ = OUTPUT_DEVICE;
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorSptr = std::make_shared<AudioDeviceDescriptor>();
    audioDeviceDescriptorSptr->deviceType_ = DEVICE_TYPE_DP;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptorSptrVector;
    audioDeviceDescriptorSptrVector.push_back(audioDeviceDescriptorSptr);
    audioDeviceCommon.UpdateConnectedDevicesWhenDisconnecting(updatedDesc, audioDeviceDescriptorSptrVector);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_029
* @tc.desc  : Test UpdateConnectedDevicesWhenDisconnecting interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_029, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    AudioDeviceDescriptor updatedDesc;
    updatedDesc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    updatedDesc.deviceRole_ = INPUT_DEVICE;
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorSptr = std::make_shared<AudioDeviceDescriptor>();
    audioDeviceDescriptorSptr->deviceType_ = DEVICE_TYPE_DP;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptorSptrVector;
    audioDeviceDescriptorSptrVector.push_back(audioDeviceDescriptorSptr);
    audioDeviceCommon.UpdateConnectedDevicesWhenDisconnecting(updatedDesc, audioDeviceDescriptorSptrVector);
    EXPECT_EQ(false, audioDeviceCommon.audioDeviceManager_.ExistsByType(DEVICE_TYPE_DP));
    EXPECT_EQ(false, audioDeviceCommon.audioDeviceManager_.ExistsByTypeAndAddress(DEVICE_TYPE_DP,
        audioDeviceDescriptorSptr->macAddress_));
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_030
* @tc.desc  : Test UpdateConnectedDevicesWhenConnectingForInputDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_030, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    AudioDeviceDescriptor updatedDesc;
    updatedDesc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    updatedDesc.deviceRole_ = INPUT_DEVICE;
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorSptr = std::make_shared<AudioDeviceDescriptor>();
    audioDeviceDescriptorSptr->deviceType_ = DEVICE_TYPE_DP;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptorSptrVector;
    audioDeviceDescriptorSptrVector.push_back(audioDeviceDescriptorSptr);
    audioDeviceCommon.UpdateConnectedDevicesWhenConnectingForInputDevice(updatedDesc,
        audioDeviceDescriptorSptrVector);
    EXPECT_EQ(2, audioDeviceDescriptorSptrVector.size());
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_031
* @tc.desc  : Test UpdateConnectedDevicesWhenConnectingForInputDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_031, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    AudioDeviceDescriptor updatedDesc;
    updatedDesc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    updatedDesc.deviceRole_ = INPUT_DEVICE;
    updatedDesc.connectState_ = VIRTUAL_CONNECTED;
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorSptr = std::make_shared<AudioDeviceDescriptor>();
    audioDeviceDescriptorSptr->deviceType_ = DEVICE_TYPE_DP;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptorSptrVector;
    audioDeviceDescriptorSptrVector.push_back(audioDeviceDescriptorSptr);
    audioDeviceCommon.UpdateConnectedDevicesWhenConnectingForInputDevice(updatedDesc,
        audioDeviceDescriptorSptrVector);
    EXPECT_EQ(2, audioDeviceDescriptorSptrVector.size());
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_032
* @tc.desc  : Test UpdateConnectedDevicesWhenConnectingForOutputDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_032, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    AudioDeviceDescriptor updatedDesc;
    updatedDesc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    updatedDesc.deviceRole_ = INPUT_DEVICE;
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorSptr = std::make_shared<AudioDeviceDescriptor>();
    audioDeviceDescriptorSptr->deviceType_ = DEVICE_TYPE_DP;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptorSptrVector;
    audioDeviceDescriptorSptrVector.push_back(audioDeviceDescriptorSptr);
    audioDeviceCommon.UpdateConnectedDevicesWhenConnectingForOutputDevice(updatedDesc,
        audioDeviceDescriptorSptrVector);
    EXPECT_EQ(2, audioDeviceDescriptorSptrVector.size());
}

/**    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptorSptrVector;
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_033
* @tc.desc  : Test UpdateConnectedDevicesWhenConnectingForOutputDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_033, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    AudioDeviceDescriptor updatedDesc;
    updatedDesc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    updatedDesc.deviceRole_ = INPUT_DEVICE;
    updatedDesc.connectState_ = VIRTUAL_CONNECTED;
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorSptr = std::make_shared<AudioDeviceDescriptor>();
    audioDeviceDescriptorSptr->deviceType_ = DEVICE_TYPE_DP;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptorSptrVector;
    audioDeviceDescriptorSptrVector.push_back(audioDeviceDescriptorSptr);
    audioDeviceCommon.UpdateConnectedDevicesWhenConnectingForOutputDevice(updatedDesc,
        audioDeviceDescriptorSptrVector);
    EXPECT_EQ(2, audioDeviceDescriptorSptrVector.size());
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_034
* @tc.desc  : Test UpdateDualToneState interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_034, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    bool enable = true;
    int32_t sessionId = 0;
    audioDeviceCommon.UpdateDualToneState(enable, sessionId);
    EXPECT_EQ(sessionId, audioDeviceCommon.enableDualHalToneSessionId_);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_035
* @tc.desc  : Test UpdateDualToneState interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_035, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    bool enable = false;
    int32_t sessionId = 0;
    audioDeviceCommon.UpdateDualToneState(enable, sessionId);
    EXPECT_EQ(enable, audioDeviceCommon.enableDualHalToneState_);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_036
* @tc.desc  : Test IsFastFromA2dpToA2dp interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_036, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    rendererChangeInfo->outputDeviceInfo.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    rendererChangeInfo->rendererInfo.originalFlag = AUDIO_FLAG_MMAP;
    rendererChangeInfo->outputDeviceInfo.deviceId_ = 0;
    desc->deviceId_ = 1;
    bool ret = audioDeviceCommon.IsFastFromA2dpToA2dp(desc, rendererChangeInfo, reason);
    EXPECT_EQ(true, ret);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_037
* @tc.desc  : Test IsFastFromA2dpToA2dp interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_037, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    rendererChangeInfo->outputDeviceInfo.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    rendererChangeInfo->rendererInfo.originalFlag = AUDIO_FLAG_MMAP;
    rendererChangeInfo->outputDeviceInfo.deviceId_ = 0;
    desc->deviceId_ = 0;
    bool ret = audioDeviceCommon.IsFastFromA2dpToA2dp(desc, rendererChangeInfo, reason);
    EXPECT_EQ(false, ret);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_043
* @tc.desc  : Test MuteSinkForSwitchGeneralDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_043, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorUniqueptr = std::make_shared<AudioDeviceDescriptor>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptorUniqueptrVector;
    audioDeviceDescriptorUniqueptr->deviceType_ = DEVICE_TYPE_SPEAKER;
    audioDeviceDescriptorUniqueptrVector.push_back(std::move(audioDeviceDescriptorUniqueptr));
    audioDeviceCommon.MuteSinkForSwitchGeneralDevice(rendererChangeInfo,
        audioDeviceDescriptorUniqueptrVector, reason);
    EXPECT_EQ(1, audioDeviceDescriptorUniqueptrVector.size());
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_044
* @tc.desc  : Test MuteSinkForSwitchGeneralDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_044, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorUniqueptr = std::make_shared<AudioDeviceDescriptor>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptorUniqueptrVector;
    audioDeviceDescriptorUniqueptr->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioDeviceDescriptorUniqueptrVector.push_back(std::move(audioDeviceDescriptorUniqueptr));
    audioDeviceCommon.MuteSinkForSwitchGeneralDevice(rendererChangeInfo,
        audioDeviceDescriptorUniqueptrVector, reason);
    EXPECT_EQ(1, audioDeviceDescriptorUniqueptrVector.size());
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_045
* @tc.desc  : Test MuteSinkForSwitchGeneralDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_045, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorUniqueptr = std::make_shared<AudioDeviceDescriptor>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptorUniqueptrVector;
    audioDeviceDescriptorUniqueptr->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    audioDeviceDescriptorUniqueptrVector.push_back(std::move(audioDeviceDescriptorUniqueptr));
    audioDeviceCommon.MuteSinkForSwitchGeneralDevice(rendererChangeInfo,
        audioDeviceDescriptorUniqueptrVector, reason);
    EXPECT_EQ(1, audioDeviceDescriptorUniqueptrVector.size());
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_046
* @tc.desc  : Test MuteSinkForSwitchBluetoothDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_046, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorUniqueptr = std::make_shared<AudioDeviceDescriptor>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptorUniqueptrVector;
    audioDeviceDescriptorUniqueptr->deviceType_ = DEVICE_TYPE_SPEAKER;
    audioDeviceDescriptorUniqueptrVector.push_back(std::move(audioDeviceDescriptorUniqueptr));
    audioDeviceCommon.MuteSinkForSwitchBluetoothDevice(rendererChangeInfo,
        audioDeviceDescriptorUniqueptrVector, reason);
    EXPECT_EQ(1, audioDeviceDescriptorUniqueptrVector.size());
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_047
* @tc.desc  : Test MuteSinkForSwitchBluetoothDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_047, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorUniqueptr = std::make_shared<AudioDeviceDescriptor>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptorUniqueptrVector;
    audioDeviceDescriptorUniqueptr->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioDeviceDescriptorUniqueptrVector.push_back(std::move(audioDeviceDescriptorUniqueptr));
    audioDeviceCommon.MuteSinkForSwitchBluetoothDevice(rendererChangeInfo,
        audioDeviceDescriptorUniqueptrVector, reason);
    EXPECT_EQ(1, audioDeviceDescriptorUniqueptrVector.size());
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_048
* @tc.desc  : Test MuteSinkForSwitchBluetoothDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_048, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorUniqueptr = std::make_shared<AudioDeviceDescriptor>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptorUniqueptrVector;
    audioDeviceDescriptorUniqueptr->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    audioDeviceDescriptorUniqueptrVector.push_back(std::move(audioDeviceDescriptorUniqueptr));
    audioDeviceCommon.MuteSinkForSwitchBluetoothDevice(rendererChangeInfo,
        audioDeviceDescriptorUniqueptrVector, reason);
    EXPECT_EQ(1, audioDeviceDescriptorUniqueptrVector.size());
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_050
* @tc.desc  : Test IsRendererStreamRunning interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_050, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->rendererInfo.streamUsage = STREAM_USAGE_VOICE_MODEM_COMMUNICATION;
    audioDeviceCommon.audioSceneManager_.SetAudioScenePre(AUDIO_SCENE_RINGING);
    bool ret = audioDeviceCommon.IsRendererStreamRunning(rendererChangeInfo);
    EXPECT_EQ(false, ret);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_051
* @tc.desc  : Test ActivateA2dpDeviceWhenDescEnabled interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_051, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->isEnable_ = false;
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    vector<std::shared_ptr<AudioRendererChangeInfo>> rendererChangeInfos;
    rendererChangeInfos.push_back(std::move(rendererChangeInfo));
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    int32_t ret = audioDeviceCommon.ActivateA2dpDeviceWhenDescEnabled(desc, rendererChangeInfos, reason);
    EXPECT_EQ(SUCCESS, ret);

    desc->isEnable_ = true;
    ret = audioDeviceCommon.ActivateA2dpDeviceWhenDescEnabled(desc, rendererChangeInfos, reason);
    EXPECT_EQ(ERROR, ret);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_052
* @tc.desc  : Test HandleScoOutputDeviceFetched interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_052, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    vector<std::shared_ptr<AudioRendererChangeInfo>> rendererChangeInfos;
    rendererChangeInfos.push_back(std::move(rendererChangeInfo));
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    int32_t ret = audioDeviceCommon.HandleScoOutputDeviceFetched(desc, rendererChangeInfos, reason);
    EXPECT_EQ(ERROR, ret);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_053
* @tc.desc  : Test NotifyRecreateRendererStream interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_053, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    rendererChangeInfo->outputDeviceInfo.deviceType_ = DEVICE_TYPE_SPEAKER;
    desc->deviceType_ = DEVICE_TYPE_EARPIECE;
    rendererChangeInfo->rendererInfo.originalFlag = AUDIO_FLAG_MMAP;
    rendererChangeInfo->rendererInfo.originalFlag = AUDIO_FLAG_VOIP_DIRECT;
    rendererChangeInfo->outputDeviceInfo.deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;
    desc->deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;
    bool ret = audioDeviceCommon.NotifyRecreateRendererStream(desc, rendererChangeInfo, reason);
    EXPECT_EQ(false, ret);

    desc->deviceType_ = DEVICE_TYPE_SPEAKER;
    rendererChangeInfo->outputDeviceInfo.networkId_ = "test1";
    desc->networkId_ = "test2";
    ret = audioDeviceCommon.NotifyRecreateRendererStream(desc, rendererChangeInfo, reason);
    EXPECT_EQ(false, ret);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_054
* @tc.desc  : Test NeedRehandleA2DPDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_054, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_SPEAKER;
    bool ret = audioDeviceCommon.NeedRehandleA2DPDevice(desc);
    EXPECT_EQ(false, ret);

    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    ret = audioDeviceCommon.NeedRehandleA2DPDevice(desc);
    EXPECT_EQ(true, ret);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_055
* @tc.desc  : Test ActivateA2dpDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_055, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    vector<std::shared_ptr<AudioRendererChangeInfo>> rendererChangeInfos;
    rendererChangeInfos.push_back(std::move(rendererChangeInfo));
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    int32_t ret = audioDeviceCommon.ActivateA2dpDevice(desc, rendererChangeInfos, reason);
    EXPECT_EQ(ERROR, ret);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_056
* @tc.desc  : Test TriggerRecreateRendererStreamCallback interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_056, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    int32_t callerPid = 0;
    int32_t sessionId = 0;
    int32_t streamFlag = 0;
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    audioDeviceCommon.audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    audioDeviceCommon.TriggerRecreateRendererStreamCallback(callerPid, sessionId, streamFlag, reason);
    EXPECT_EQ(true, audioDeviceCommon.audioPolicyServerHandler_ != nullptr);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_057
* @tc.desc  : Test HandleBluetoothInputDeviceFetched interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_057, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    std::shared_ptr<AudioCapturerChangeInfo> captureChangeInfo = std::make_shared<AudioCapturerChangeInfo>();
    vector<std::shared_ptr<AudioCapturerChangeInfo>> captureChangeInfos;
    captureChangeInfos.push_back(std::move(captureChangeInfo));

    SourceType sourceType = SOURCE_TYPE_MIC;
    audioDeviceCommon.HandleBluetoothInputDeviceFetched(desc, captureChangeInfos, sourceType);
    EXPECT_EQ(1, captureChangeInfos.size());
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_058
* @tc.desc  : Test HandleBluetoothInputDeviceFetched interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_058, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP_IN;
    std::shared_ptr<AudioCapturerChangeInfo> captureChangeInfo = std::make_shared<AudioCapturerChangeInfo>();
    vector<std::shared_ptr<AudioCapturerChangeInfo>> captureChangeInfos;
    captureChangeInfos.push_back(std::move(captureChangeInfo));
    SourceType sourceType = SOURCE_TYPE_MIC;
    audioDeviceCommon.HandleBluetoothInputDeviceFetched(desc, captureChangeInfos, sourceType);
    EXPECT_EQ(1, captureChangeInfos.size());
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_059
* @tc.desc  : Test HandleBluetoothInputDeviceFetched interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_059, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_MIC;
    std::shared_ptr<AudioCapturerChangeInfo> captureChangeInfo = std::make_shared<AudioCapturerChangeInfo>();
    vector<std::shared_ptr<AudioCapturerChangeInfo>> captureChangeInfos;
    captureChangeInfos.push_back(std::move(captureChangeInfo));
    SourceType sourceType = SOURCE_TYPE_MIC;
    audioDeviceCommon.HandleBluetoothInputDeviceFetched(desc, captureChangeInfos, sourceType);
    EXPECT_EQ(1, captureChangeInfos.size());
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_060
* @tc.desc  : Test HandleBluetoothInputDeviceFetched interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_060, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_MIC;
    std::shared_ptr<AudioCapturerChangeInfo> captureChangeInfo = std::make_shared<AudioCapturerChangeInfo>();
    vector<std::shared_ptr<AudioCapturerChangeInfo>> captureChangeInfos;
    captureChangeInfos.push_back(std::move(captureChangeInfo));
    SourceType sourceType = SOURCE_TYPE_MIC;
    audioDeviceCommon.HandleBluetoothInputDeviceFetched(desc, captureChangeInfos, sourceType);
    EXPECT_EQ(1, captureChangeInfos.size());
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_061
* @tc.desc  : Test NotifyRecreateCapturerStream interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_061, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    bool isUpdateActiveDevice = true;
    std::shared_ptr<AudioCapturerChangeInfo> capturerChangeInfo = std::make_shared<AudioCapturerChangeInfo>();
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    capturerChangeInfo->capturerInfo.originalFlag = AUDIO_FLAG_MMAP;
    capturerChangeInfo->inputDeviceInfo.deviceType_ = DEVICE_TYPE_MIC;
    AudioDeviceDescriptor deviceDescriptor;
    deviceDescriptor.deviceType_ = DEVICE_TYPE_MIC;
    audioDeviceCommon.audioActiveDevice_.SetCurrentInputDevice(deviceDescriptor);
    capturerChangeInfo->inputDeviceInfo.networkId_ = "test";
    bool ret = audioDeviceCommon.NotifyRecreateCapturerStream(isUpdateActiveDevice,
        capturerChangeInfo, reason);
    EXPECT_EQ(true, ret);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_062
* @tc.desc  : Test NotifyRecreateCapturerStream interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_062, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    bool isUpdateActiveDevice = true;
    std::shared_ptr<AudioCapturerChangeInfo> capturerChangeInfo = std::make_shared<AudioCapturerChangeInfo>();
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    capturerChangeInfo->capturerInfo.originalFlag = AUDIO_FLAG_MMAP;
    capturerChangeInfo->inputDeviceInfo.deviceType_ = DEVICE_TYPE_MIC;
    AudioDeviceDescriptor deviceDescriptor;
    deviceDescriptor.deviceType_ = DEVICE_TYPE_MIC;
    audioDeviceCommon.audioActiveDevice_.SetCurrentInputDevice(deviceDescriptor);
    capturerChangeInfo->inputDeviceInfo.networkId_ = "LocalDevice";
    bool ret = audioDeviceCommon.NotifyRecreateCapturerStream(isUpdateActiveDevice,
        capturerChangeInfo, reason);
    EXPECT_EQ(false, ret);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_063
* @tc.desc  : Test ReloadA2dpAudioPort interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_063, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    AudioModuleInfo moduleInfo;
    DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    AudioStreamInfo audioStreamInfo;
    std::string networkID = "";
    std::string sinkName = "";
    SourceType sourceType = SOURCE_TYPE_MIC;
    int32_t ret = audioDeviceCommon.ReloadA2dpAudioPort(moduleInfo, deviceType, audioStreamInfo, networkID,
        sinkName, sourceType);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);

    deviceType = DEVICE_TYPE_BLUETOOTH_A2DP_IN;
    ret = audioDeviceCommon.ReloadA2dpAudioPort(moduleInfo, deviceType, audioStreamInfo, networkID,
        sinkName, sourceType);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_064
* @tc.desc  : Test ScoInputDeviceFetchedForRecongnition interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_064, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    bool handleFlag = true;
    std::string address = "";
    ConnectState connectState = CONNECTED;
    int32_t ret = audioDeviceCommon.ScoInputDeviceFetchedForRecongnition(handleFlag, address, connectState);
    EXPECT_EQ(SUCCESS, ret);

    handleFlag = false;
    DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_A2DP_IN;
    ret = audioDeviceCommon.ScoInputDeviceFetchedForRecongnition(handleFlag, address, connectState);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_065
* @tc.desc  : Test GetSpatialDeviceType interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_065, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::string macAddress = "F0-FA-C7-8C-46-01";
    DeviceType deviceType = audioDeviceCommon.GetSpatialDeviceType(macAddress);
    EXPECT_EQ(DEVICE_TYPE_NONE, deviceType);

    AudioDeviceDescriptor deviceDescriptor;
    deviceDescriptor.macAddress_ = "F0-FA-C7-8C-46-01";
    deviceDescriptor.deviceType_ = DEVICE_TYPE_SPEAKER;
    audioDeviceCommon.OnPreferredOutputDeviceUpdated(deviceDescriptor, AudioStreamDeviceChangeReason::UNKNOWN);
    deviceType = audioDeviceCommon.GetSpatialDeviceType(macAddress);
    EXPECT_EQ(DEVICE_TYPE_SPEAKER, deviceType);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_066
* @tc.desc  : Test GetDeviceDescriptorInner interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_066, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    VolumeUtils::SetPCVolumeEnable(true);
    audioDeviceCommon.isFirstScreenOn_ = false;
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    vector<std::shared_ptr<AudioDeviceDescriptor>> descs =
        audioDeviceCommon.GetDeviceDescriptorInner(rendererChangeInfo);
    EXPECT_NE(0, descs.size());

    audioDeviceCommon.isFirstScreenOn_ = true;
    rendererChangeInfo->rendererInfo.streamUsage = STREAM_USAGE_ULTRASONIC;
    rendererChangeInfo->clientUID = 0;
    descs = audioDeviceCommon.GetDeviceDescriptorInner(rendererChangeInfo);
    EXPECT_NE(0, descs.size());
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_067
* @tc.desc  : Test FetchOutputEnd interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_067, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    bool isUpdateActiveDevice = true;
    int32_t runningStreamCount = 0;
    audioDeviceCommon.FetchOutputEnd(isUpdateActiveDevice, runningStreamCount, AudioStreamDeviceChangeReason::UNKNOWN);
    EXPECT_NE(0, audioDeviceCommon.spatialDeviceMap_.size());
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_068
* @tc.desc  : Test HandleDeviceChangeForFetchOutputDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_068, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    desc->deviceType_ = DEVICE_TYPE_NONE;
    int32_t ret = audioDeviceCommon.HandleDeviceChangeForFetchOutputDevice(desc, rendererChangeInfo,
        AudioStreamDeviceChangeReason::UNKNOWN);
    EXPECT_EQ(ERR_NEED_NOT_SWITCH_DEVICE, ret);

    desc->deviceType_ = DEVICE_TYPE_EARPIECE;
    rendererChangeInfo->outputDeviceInfo.deviceType_ = DEVICE_TYPE_SPEAKER;
    ret = audioDeviceCommon.HandleDeviceChangeForFetchOutputDevice(desc, rendererChangeInfo,
        AudioStreamDeviceChangeReason::UNKNOWN);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_069
* @tc.desc  : Test IsSameDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_069, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    AudioDeviceDescriptor deviceInfo;
    desc->deviceType_ = DEVICE_TYPE_NONE;
    deviceInfo.deviceType_ = DEVICE_TYPE_EARPIECE;
    bool ret = audioDeviceCommon.IsSameDevice(desc, deviceInfo);
    EXPECT_EQ(false, ret);

    desc->networkId_ = "";
    deviceInfo.networkId_ = "";
    desc->macAddress_ = "";
    deviceInfo.macAddress_ = "";
    desc->connectState_ = CONNECTED;
    deviceInfo.connectState_ = CONNECTED;
    desc->deviceType_ = DEVICE_TYPE_USB_HEADSET;
    deviceInfo.deviceType_ = DEVICE_TYPE_USB_HEADSET;
    desc->deviceRole_ = DEVICE_ROLE_NONE;
    deviceInfo.deviceRole_ = DEVICE_ROLE_NONE;
    ret = audioDeviceCommon.IsSameDevice(desc, deviceInfo);
    EXPECT_EQ(true, ret);

    desc->deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;
    deviceInfo.deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;
    ret = audioDeviceCommon.IsSameDevice(desc, deviceInfo);
    EXPECT_EQ(true, ret);

    BluetoothOffloadState state = A2DP_NOT_OFFLOAD;
    audioDeviceCommon.audioA2dpOffloadFlag_.SetA2dpOffloadFlag(state);
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    deviceInfo.a2dpOffloadFlag_ = A2DP_OFFLOAD;
    ret = audioDeviceCommon.IsSameDevice(desc, deviceInfo);
    EXPECT_EQ(false, ret);

    deviceInfo.a2dpOffloadFlag_ = A2DP_NOT_OFFLOAD;
    state = A2DP_OFFLOAD;
    ret = audioDeviceCommon.IsSameDevice(desc, deviceInfo);
    EXPECT_EQ(false, ret);

    desc->deviceType_ = DEVICE_TYPE_SPEAKER;
    deviceInfo.deviceType_ = DEVICE_TYPE_SPEAKER;
    ret = audioDeviceCommon.IsSameDevice(desc, deviceInfo);
    EXPECT_EQ(true, ret);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_070
* @tc.desc  : Test IsSameDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_070, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_NONE;
    AudioDeviceDescriptor deviceDesc;
    deviceDesc.deviceType_ = DEVICE_TYPE_EARPIECE;
    audioDeviceCommon.audioActiveDevice_.SetCurrentOutputDevice(deviceDesc);
    bool ret = audioDeviceCommon.IsSameDevice(desc, deviceDesc);
    EXPECT_EQ(false, ret);

    desc->networkId_ = "";
    deviceDesc.networkId_ = "";
    desc->macAddress_ = "";
    deviceDesc.macAddress_ = "";
    desc->connectState_ = CONNECTED;
    deviceDesc.connectState_ = CONNECTED;
    desc->deviceType_ = DEVICE_TYPE_SPEAKER;
    deviceDesc.deviceType_ = DEVICE_TYPE_SPEAKER;
    ret = audioDeviceCommon.IsSameDevice(desc, deviceDesc);
    EXPECT_EQ(true, ret);

    desc->deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;
    deviceDesc.deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;
    desc->deviceRole_ = DEVICE_ROLE_NONE;
    deviceDesc.deviceRole_ = DEVICE_ROLE_NONE;
    ret = audioDeviceCommon.IsSameDevice(desc, deviceDesc);
    EXPECT_EQ(true, ret);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_072
* @tc.desc  : Test CheckAndNotifyUserSelectedDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_072, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    audioDeviceCommon.CheckAndNotifyUserSelectedDevice(desc);
    bool ret = audioDeviceCommon.audioActiveDevice_.CheckActiveOutputDeviceSupportOffload();
    EXPECT_EQ(false, ret);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_073
* @tc.desc  : Test ResetOffloadAndMchMode interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_073, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    vector<std::shared_ptr<AudioDeviceDescriptor>> outputDevices;
    std::shared_ptr<AudioDeviceDescriptor> outputDevice = std::make_shared<AudioDeviceDescriptor>();
    outputDevice->networkId_ = "";
    outputDevices.push_back(std::move(outputDevice));
    audioDeviceCommon.ResetOffloadAndMchMode(rendererChangeInfo, outputDevices);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_074
* @tc.desc  : Test ResetOffloadAndMchMode interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_074, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    vector<std::shared_ptr<AudioDeviceDescriptor>> outputDevices;
    std::shared_ptr<AudioDeviceDescriptor> outputDevice = std::make_shared<AudioDeviceDescriptor>();
    outputDevice->networkId_ = "LocalDevice";
    outputDevice->deviceType_ = DEVICE_TYPE_REMOTE_CAST;
    outputDevices.push_back(std::move(outputDevice));
    audioDeviceCommon.ResetOffloadAndMchMode(rendererChangeInfo, outputDevices);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_075
* @tc.desc  : Test ResetOffloadAndMchMode interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_075, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    vector<std::shared_ptr<AudioDeviceDescriptor>> outputDevices;
    std::shared_ptr<AudioDeviceDescriptor> outputDevice = std::make_shared<AudioDeviceDescriptor>();
    outputDevice->networkId_ = "LocalDevice";
    outputDevice->deviceType_ = DEVICE_TYPE_DP;
    outputDevices.push_back(std::move(outputDevice));
    audioDeviceCommon.ResetOffloadAndMchMode(rendererChangeInfo, outputDevices);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_076
* @tc.desc  : Test JudgeIfLoadMchModule interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_076, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.JudgeIfLoadMchModule();

    AudioIOHandle moduleId = 0;
    std::string moduleName = "MCH_Speaker";
    audioDeviceCommon.audioIOHandleMap_.AddIOHandleInfo(moduleName, moduleId);
    audioDeviceCommon.JudgeIfLoadMchModule();
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_077
* @tc.desc  : Test FetchStreamForA2dpMchStream interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_077, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    descs.push_back(std::move(desc));
    audioDeviceCommon.FetchStreamForA2dpMchStream(rendererChangeInfo, descs);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_078
* @tc.desc  : Test FetchStreamForSpkMchStream interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_078, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    descs.push_back(std::move(desc));
    audioDeviceCommon.FetchStreamForSpkMchStream(rendererChangeInfo, descs);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_079
* @tc.desc  : Test HandleDeviceChangeForFetchInputDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_079, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::shared_ptr<AudioCapturerChangeInfo> capturerChangeInfo = std::make_shared<AudioCapturerChangeInfo>();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_NONE;
    int32_t ret = audioDeviceCommon.HandleDeviceChangeForFetchInputDevice(desc, capturerChangeInfo);
    EXPECT_EQ(ERR_NEED_NOT_SWITCH_DEVICE, ret);

    desc->networkId_ = "";
    capturerChangeInfo->inputDeviceInfo.networkId_ = "";
    desc->macAddress_ = "";
    capturerChangeInfo->inputDeviceInfo.macAddress_ = "";
    desc->connectState_ = CONNECTED;
    capturerChangeInfo->inputDeviceInfo.connectState_ = CONNECTED;
    desc->deviceType_ = DEVICE_TYPE_USB_HEADSET;
    capturerChangeInfo->inputDeviceInfo.deviceType_ = DEVICE_TYPE_USB_HEADSET;
    desc->deviceRole_ = DEVICE_ROLE_NONE;
    capturerChangeInfo->inputDeviceInfo.deviceRole_ = DEVICE_ROLE_NONE;
    ret = audioDeviceCommon.HandleDeviceChangeForFetchInputDevice(desc, capturerChangeInfo);
    EXPECT_EQ(ERR_NEED_NOT_SWITCH_DEVICE, ret);

    desc->deviceType_ = DEVICE_TYPE_SPEAKER;
    desc->connectState_ = DEACTIVE_CONNECTED;
    ret = audioDeviceCommon.HandleDeviceChangeForFetchInputDevice(desc, capturerChangeInfo);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_080
* @tc.desc  : Test MoveToRemoteInputDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_080, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    SourceOutput sourceOutput;
    std::vector<SourceOutput> sourceOutputs;
    sourceOutputs.push_back(sourceOutput);
    std::shared_ptr<AudioDeviceDescriptor> remoteDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioDeviceCommon.isOpenRemoteDevice = false;
    int32_t ret = audioDeviceCommon.MoveToRemoteInputDevice(sourceOutputs, remoteDeviceDescriptor);
    EXPECT_EQ(ERR_INVALID_OPERATION, ret);

    audioDeviceCommon.isOpenRemoteDevice = true;
    ret = audioDeviceCommon.MoveToRemoteInputDevice(sourceOutputs, remoteDeviceDescriptor);
    EXPECT_EQ(ERR_INVALID_OPERATION, ret);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_081
* @tc.desc  : Test MuteSinkPort interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_081, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::string oldSinkname = "";
    std::string newSinkName = "Offload_Speaker";
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::OVERRODE;
    audioDeviceCommon.MuteSinkPort(oldSinkname, newSinkName, reason);

    oldSinkname = "Offload_Speaker";
    newSinkName = "";
    audioDeviceCommon.MuteSinkPort(oldSinkname, newSinkName, reason);

    reason = AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE;
    newSinkName = "Offload_Speaker";
    oldSinkname = "";
    audioDeviceCommon.MuteSinkPort(oldSinkname, newSinkName, reason);

    newSinkName = "";
    oldSinkname = "Offload_Speaker";
    audioDeviceCommon.MuteSinkPort(oldSinkname, newSinkName, reason);

    reason = AudioStreamDeviceChangeReason::OLD_DEVICE_UNAVALIABLE;
    audioDeviceCommon.audioSceneManager_.SetAudioScenePre(AUDIO_SCENE_DEFAULT);
    audioDeviceCommon.MuteSinkPort(oldSinkname, newSinkName, reason);

    audioDeviceCommon.audioSceneManager_.SetAudioScenePre(AUDIO_SCENE_RINGING);
    audioDeviceCommon.audioPolicyManager_.SetRingerMode(RINGER_MODE_SILENT);
    audioDeviceCommon.MuteSinkPort(oldSinkname, newSinkName, reason);

    reason = AudioStreamDeviceChangeReason::UNKNOWN;
    oldSinkname = "RemoteCastInnerCapturer";
    audioDeviceCommon.MuteSinkPort(oldSinkname, newSinkName, reason);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_082
* @tc.desc  : Test MoveToNewOutputDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_082, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->outputDeviceInfo.deviceType_ = DEVICE_TYPE_SPEAKER;
    rendererChangeInfo->outputDeviceInfo.macAddress_ = "";
    rendererChangeInfo->outputDeviceInfo.networkId_ = "";
    rendererChangeInfo->outputDeviceInfo.deviceRole_ = INPUT_DEVICE;
    std::shared_ptr<AudioDeviceDescriptor> outputdevice = std::make_shared<AudioDeviceDescriptor>();
    outputdevice->deviceType_ = DEVICE_TYPE_SPEAKER;
    outputdevice->macAddress_ = "";
    outputdevice->networkId_ = "";
    outputdevice->deviceRole_ = INPUT_DEVICE;
    vector<std::shared_ptr<AudioDeviceDescriptor>> outputDevices;
    outputDevices.push_back(std::move(outputdevice));
    std::vector<SinkInput> sinkInputs;
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::OVERRODE;
    audioDeviceCommon.audioConfigManager_.OnUpdateRouteSupport(true);
    audioDeviceCommon.MoveToNewOutputDevice(rendererChangeInfo, outputDevices, sinkInputs, reason);
    EXPECT_EQ(true, audioDeviceCommon.audioConfigManager_.GetUpdateRouteSupport());
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_083
* @tc.desc  : Test MoveToNewOutputDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_083, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->outputDeviceInfo.deviceType_ = DEVICE_TYPE_SPEAKER;
    rendererChangeInfo->outputDeviceInfo.macAddress_ = "";
    rendererChangeInfo->outputDeviceInfo.networkId_ = "";
    rendererChangeInfo->outputDeviceInfo.deviceRole_ = INPUT_DEVICE;
    std::shared_ptr<AudioDeviceDescriptor> outputdevice = std::make_shared<AudioDeviceDescriptor>();
    outputdevice->deviceType_ = DEVICE_TYPE_SPEAKER;
    outputdevice->macAddress_ = "";
    outputdevice->networkId_ = "LocalDevice";
    outputdevice->deviceRole_ = INPUT_DEVICE;
    vector<std::shared_ptr<AudioDeviceDescriptor>> outputDevices;
    outputDevices.push_back(std::move(outputdevice));
    std::vector<SinkInput> sinkInputs;
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::OVERRODE;
    audioDeviceCommon.audioConfigManager_.OnUpdateRouteSupport(true);
    audioDeviceCommon.audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    audioDeviceCommon.MoveToNewOutputDevice(rendererChangeInfo, outputDevices, sinkInputs, reason);
    EXPECT_EQ(true, audioDeviceCommon.audioConfigManager_.GetUpdateRouteSupport());
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_084
* @tc.desc  : Test UpdateRoute interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_084, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->rendererInfo.streamUsage = STREAM_USAGE_MEDIA;

    std::shared_ptr<AudioDeviceDescriptor> outputdevice = std::make_shared<AudioDeviceDescriptor>();
    vector<std::shared_ptr<AudioDeviceDescriptor>> outputDevices;
    outputDevices.push_back(std::move(outputdevice));
    audioDeviceCommon.enableDualHalToneState_ = true;
    audioDeviceCommon.UpdateRoute(rendererChangeInfo, outputDevices);
    EXPECT_EQ(0, audioDeviceCommon.enableDualHalToneSessionId_);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_085
* @tc.desc  : Test UpdateRoute interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_085, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->rendererInfo.streamUsage = STREAM_USAGE_MEDIA;

    std::shared_ptr<AudioDeviceDescriptor> outputdevice = std::make_shared<AudioDeviceDescriptor>();
    vector<std::shared_ptr<AudioDeviceDescriptor>> outputDevices;
    outputDevices.push_back(std::move(outputdevice));
    audioDeviceCommon.enableDualHalToneState_ = false;
    audioDeviceCommon.UpdateRoute(rendererChangeInfo, outputDevices);
    EXPECT_EQ(true, audioDeviceCommon.audioVolumeManager_.IsRingerModeMute());
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_086
* @tc.desc  : Test UpdateRoute interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_086, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->rendererInfo.streamUsage = STREAM_USAGE_ALARM;

    std::shared_ptr<AudioDeviceDescriptor> outputdevice = std::make_shared<AudioDeviceDescriptor>();
    outputdevice->deviceType_ = DEVICE_TYPE_WIRED_HEADSET;
    vector<std::shared_ptr<AudioDeviceDescriptor>> outputDevices;
    outputDevices.push_back(std::move(outputdevice));
    VolumeUtils::SetPCVolumeEnable(false);
    audioDeviceCommon.UpdateRoute(rendererChangeInfo, outputDevices);
    EXPECT_EQ(true, audioDeviceCommon.shouldUpdateDeviceDueToDualTone_);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_087
* @tc.desc  : Test UpdateRoute interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_087, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->rendererInfo.streamUsage = STREAM_USAGE_ALARM;

    std::shared_ptr<AudioDeviceDescriptor> outputdevice = std::make_shared<AudioDeviceDescriptor>();
    outputdevice->deviceType_ = DEVICE_TYPE_SPEAKER;
    vector<std::shared_ptr<AudioDeviceDescriptor>> outputDevices;
    outputDevices.push_back(std::move(outputdevice));
    VolumeUtils::SetPCVolumeEnable(false);
    audioDeviceCommon.UpdateRoute(rendererChangeInfo, outputDevices);
    EXPECT_EQ(true, audioDeviceCommon.audioVolumeManager_.IsRingerModeMute());
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_088
* @tc.desc  : Test UpdateRoute interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_088, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
    rendererChangeInfo->sessionId = 1;
    vector<std::shared_ptr<AudioDeviceDescriptor>> outputDevices;
    outputDevices.push_back(std::make_shared<AudioDeviceDescriptor>());

    audioDeviceCommon.UpdateRoute(rendererChangeInfo, outputDevices);
    EXPECT_EQ(0, audioDeviceCommon.streamsWhenRingDualOnPrimarySpeaker_.size());

    outputDevices.front()->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    audioDeviceCommon.UpdateRoute(rendererChangeInfo, outputDevices);
    EXPECT_EQ(0, audioDeviceCommon.streamsWhenRingDualOnPrimarySpeaker_.size());

    audioDeviceCommon.isRingDualToneOnPrimarySpeaker_ = true;
    audioDeviceCommon.UpdateRoute(rendererChangeInfo, outputDevices);
    EXPECT_EQ(1, audioDeviceCommon.streamsWhenRingDualOnPrimarySpeaker_.size());

    outputDevices.front()->deviceType_ = DEVICE_TYPE_INVALID;
    audioDeviceCommon.UpdateRoute(rendererChangeInfo, outputDevices);
    EXPECT_EQ(2, audioDeviceCommon.streamsWhenRingDualOnPrimarySpeaker_.size());
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_089
* @tc.desc  : Test IsDualStreamWhenRingDual interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_089, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();

    EXPECT_TRUE(audioDeviceCommon.IsDualStreamWhenRingDual(STREAM_RING));
    EXPECT_TRUE(audioDeviceCommon.IsDualStreamWhenRingDual(STREAM_ALARM));
    EXPECT_TRUE(audioDeviceCommon.IsDualStreamWhenRingDual(STREAM_ACCESSIBILITY));
    EXPECT_FALSE(audioDeviceCommon.IsDualStreamWhenRingDual(STREAM_MUSIC));
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_090
* @tc.desc  : Test IsRingDualToneOnPrimarySpeaker interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_090, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    EXPECT_FALSE(audioDeviceCommon.IsRingDualToneOnPrimarySpeaker(descs, 1));

    descs.push_back(std::make_shared<AudioDeviceDescriptor>());
    descs.push_back(std::make_shared<AudioDeviceDescriptor>());
    EXPECT_FALSE(audioDeviceCommon.IsRingDualToneOnPrimarySpeaker(descs, 1));

    descs.front()->deviceType_ = DEVICE_TYPE_EARPIECE;
    EXPECT_FALSE(audioDeviceCommon.IsRingDualToneOnPrimarySpeaker(descs, 1));

    descs.back()->deviceType_ = DEVICE_TYPE_EARPIECE;
    EXPECT_FALSE(audioDeviceCommon.IsRingDualToneOnPrimarySpeaker(descs, 1));

    descs.front()->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    EXPECT_FALSE(audioDeviceCommon.IsRingDualToneOnPrimarySpeaker(descs, 1));

    descs.back()->deviceType_ = DEVICE_TYPE_SPEAKER;
    EXPECT_TRUE(audioDeviceCommon.IsRingDualToneOnPrimarySpeaker(descs, 1));
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_091
* @tc.desc  : Test IsRingOverPlayback interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_091, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    AudioMode mode = AUDIO_MODE_RECORD;
    RendererState state  = RENDERER_RUNNING;
    EXPECT_FALSE(audioDeviceCommon.IsRingOverPlayback(mode, state));

    mode = AUDIO_MODE_PLAYBACK;
    EXPECT_FALSE(audioDeviceCommon.IsRingOverPlayback(mode, state));

    state = RENDERER_STOPPED;
    EXPECT_TRUE(audioDeviceCommon.IsRingOverPlayback(mode, state));

    state = RENDERER_RELEASED;
    EXPECT_TRUE(audioDeviceCommon.IsRingOverPlayback(mode, state));

    state = RENDERER_PAUSED;
    EXPECT_TRUE(audioDeviceCommon.IsRingOverPlayback(mode, state));
}
} // namespace AudioStandard
} // namespace OHOS