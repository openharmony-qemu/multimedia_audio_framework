/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

void AudioDeviceCommonUnitTest::SetUpTestCase(void) {}
void AudioDeviceCommonUnitTest::TearDownTestCase(void) {}
void AudioDeviceCommonUnitTest::SetUp(void) {}
void AudioDeviceCommonUnitTest::TearDown(void) {}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_001
* @tc.desc  : Test GetHasDpFlag interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_001, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.audioPolicyServerHandler_ = nullptr;
    AudioDeviceDescriptor deviceDescriptor;
    audioDeviceCommon.OnPreferredOutputDeviceUpdated(deviceDescriptor);

    DeviceType deviceType = DEVICE_TYPE_NONE;
    audioDeviceCommon.OnPreferredInputDeviceUpdated(deviceType, "");

    AudioRendererInfo rendererInfo;
    rendererInfo.streamUsage = static_cast<StreamUsage>(1000);
    audioDeviceCommon.GetPreferredOutputDeviceDescInner(rendererInfo, "");

    AudioCapturerInfo captureInfo;
    captureInfo.sourceType = SOURCE_TYPE_INVALID;
    audioDeviceCommon.GetPreferredInputDeviceDescInner(captureInfo, "");
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
    audioDeviceCommon.OnPreferredOutputDeviceUpdated(deviceDescriptor);

    DeviceType deviceType = DEVICE_TYPE_NONE;
    audioDeviceCommon.OnPreferredInputDeviceUpdated(deviceType, "");

    AudioRendererInfo rendererInfo;
    rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    audioDeviceCommon.GetPreferredOutputDeviceDescInner(rendererInfo, "LocalDevice");

    AudioCapturerInfo captureInfo;
    captureInfo.sourceType = SOURCE_TYPE_MAX;
    audioDeviceCommon.GetPreferredInputDeviceDescInner(captureInfo, "");
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
    audioDeviceCommon.OnPreferredOutputDeviceUpdated(deviceDescriptor);

    DeviceType deviceType = DEVICE_TYPE_NONE;
    audioDeviceCommon.OnPreferredInputDeviceUpdated(deviceType, "");

    AudioRendererInfo rendererInfo;
    rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    audioDeviceCommon.GetPreferredOutputDeviceDescInner(rendererInfo, "");

    AudioCapturerInfo captureInfo;
    captureInfo.sourceType = SOURCE_TYPE_WAKEUP;
    audioDeviceCommon.GetPreferredInputDeviceDescInner(captureInfo, "");
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
    audioDeviceCommon.OnPreferredOutputDeviceUpdated(deviceDescriptor);

    DeviceType deviceType = DEVICE_TYPE_NONE;
    audioDeviceCommon.OnPreferredInputDeviceUpdated(deviceType, "");

    AudioRendererInfo rendererInfo;
    rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    audioDeviceCommon.GetPreferredOutputDeviceDescInner(rendererInfo, "");

    AudioCapturerInfo captureInfo;
    captureInfo.sourceType = SOURCE_TYPE_MIC;
    audioDeviceCommon.GetPreferredInputDeviceDescInner(captureInfo, "LocalDevice");
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
    audioDeviceCommon.OnPreferredOutputDeviceUpdated(deviceDescriptor);

    DeviceType deviceType = DEVICE_TYPE_NONE;
    audioDeviceCommon.OnPreferredInputDeviceUpdated(deviceType, "");

    AudioRendererInfo rendererInfo;
    rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    audioDeviceCommon.GetPreferredOutputDeviceDescInner(rendererInfo, "");

    AudioCapturerInfo captureInfo;
    captureInfo.sourceType = SOURCE_TYPE_MIC;
    audioDeviceCommon.GetPreferredInputDeviceDescInner(captureInfo, "");
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_007
* @tc.desc  : Test GetPreferredOutputStreamTypeInner interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_007, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    StreamUsage streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;
    DeviceType deviceType = DEVICE_TYPE_EARPIECE;
    int32_t flags = 1;
    std::string networkId = "";
    AudioSamplingRate samplingRate = SAMPLE_RATE_8000;
    int32_t ret = audioDeviceCommon.GetPreferredOutputStreamTypeInner(streamUsage,
        deviceType, flags, networkId, samplingRate);
    EXPECT_EQ(AUDIO_FLAG_NORMAL, ret);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_008
* @tc.desc  : Test GetPreferredInputStreamTypeInner interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_008, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    SourceType sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    DeviceType deviceType = DEVICE_TYPE_MIC;
    int32_t flags = 1;
    std::string networkId = "LocalDevice";
    AudioSamplingRate samplingRate = SAMPLE_RATE_8000;
    int32_t ret = audioDeviceCommon.GetPreferredInputStreamTypeInner(sourceType,
        deviceType, flags, networkId, samplingRate);
    EXPECT_EQ(AUDIO_FLAG_NORMAL, ret);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_009
* @tc.desc  : Test GetPreferredInputStreamTypeInner interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_009, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    SourceType sourceType = SOURCE_TYPE_MIC;
    DeviceType deviceType = DEVICE_TYPE_MIC;
    int32_t flags = 1;
    std::string networkId = "LocalDevice";
    AudioSamplingRate samplingRate = SAMPLE_RATE_8000;
    int32_t ret = audioDeviceCommon.GetPreferredInputStreamTypeInner(sourceType,
        deviceType, flags, networkId, samplingRate);
    EXPECT_EQ(AUDIO_FLAG_NORMAL, ret);
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
* @tc.number: AudioDeviceCommon_011
* @tc.desc  : Test GetPreferredOutputStreamTypeInner interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_011, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    StreamUsage streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;
    DeviceType deviceType = DEVICE_TYPE_EARPIECE;
    int32_t flags = 1;
    std::string networkId = "LocalDevice";
    AudioSamplingRate samplingRate = SAMPLE_RATE_48000;
    audioDeviceCommon.audioConfigManager_.OnVoipConfigParsed(true);
    int32_t ret = audioDeviceCommon.GetPreferredOutputStreamTypeInner(streamUsage,
        deviceType, flags, networkId, samplingRate);
    EXPECT_EQ(AUDIO_FLAG_VOIP_FAST, ret);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_012
* @tc.desc  : Test GetPreferredOutputStreamTypeInner interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_012, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    StreamUsage streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;
    DeviceType deviceType = DEVICE_TYPE_EARPIECE;
    int32_t flags = 1;
    std::string networkId = "LocalDevice";
    AudioSamplingRate samplingRate = SAMPLE_RATE_48000;
    audioDeviceCommon.audioConfigManager_.OnVoipConfigParsed(false);
    int32_t ret = audioDeviceCommon.GetPreferredOutputStreamTypeInner(streamUsage,
        deviceType, flags, networkId, samplingRate);
    EXPECT_EQ(AUDIO_FLAG_VOIP_DIRECT, ret);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_013
* @tc.desc  : Test GetPreferredOutputStreamTypeInner interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_013, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    StreamUsage streamUsage = STREAM_USAGE_VIDEO_COMMUNICATION;
    DeviceType deviceType = DEVICE_TYPE_EARPIECE;
    int32_t flags = 1;
    std::string networkId = "LocalDevice";
    AudioSamplingRate samplingRate = SAMPLE_RATE_48000;
    audioDeviceCommon.audioConfigManager_.OnVoipConfigParsed(false);
    int32_t ret = audioDeviceCommon.GetPreferredOutputStreamTypeInner(streamUsage,
        deviceType, flags, networkId, samplingRate);
    EXPECT_EQ(AUDIO_FLAG_VOIP_DIRECT, ret);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_014
* @tc.desc  : Test GetPreferredOutputStreamTypeInner interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_014, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    StreamUsage streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;
    DeviceType deviceType = DEVICE_TYPE_EARPIECE;
    int32_t flags = 1;
    std::string networkId = "";
    AudioSamplingRate samplingRate = SAMPLE_RATE_8000;
    const std::unordered_map<AdaptersType, AudioAdapterInfo> adapterInfoMap;
    audioDeviceCommon.audioConfigManager_.OnAudioPolicyXmlParsingCompleted(adapterInfoMap);
    int32_t ret = audioDeviceCommon.GetPreferredOutputStreamTypeInner(streamUsage,
        deviceType, flags, networkId, samplingRate);
    EXPECT_EQ(AUDIO_FLAG_NORMAL, ret);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_015
* @tc.desc  : Test GetPreferredOutputStreamTypeInner interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_015, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    StreamUsage streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;
    DeviceType deviceType = DEVICE_TYPE_EARPIECE;
    int32_t flags = 1;
    std::string networkId = "";
    AudioSamplingRate samplingRate = SAMPLE_RATE_8000;
    AudioAdapterInfo adapterInfo = {};
    std::unordered_map<AdaptersType, AudioAdapterInfo> adapterInfoMap = {};
    adapterInfoMap.insert({AdaptersType::TYPE_PRIMARY, adapterInfo});
    audioDeviceCommon.audioConfigManager_.OnAudioPolicyXmlParsingCompleted(adapterInfoMap);
    int32_t ret = audioDeviceCommon.GetPreferredOutputStreamTypeInner(streamUsage,
        deviceType, flags, networkId, samplingRate);
    EXPECT_EQ(AUDIO_FLAG_INVALID, ret);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_016
* @tc.desc  : Test GetPreferredInputStreamTypeInner interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_016, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    SourceType sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    DeviceType deviceType = DEVICE_TYPE_MIC;
    int32_t flags = 1;
    std::string networkId = "LocalDevice";
    AudioSamplingRate samplingRate = SAMPLE_RATE_48000;
    audioDeviceCommon.audioConfigManager_.OnVoipConfigParsed(true);
    int32_t ret = audioDeviceCommon.GetPreferredInputStreamTypeInner(sourceType,
        deviceType, flags, networkId, samplingRate);
    EXPECT_EQ(AUDIO_FLAG_VOIP_FAST, ret);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_017
* @tc.desc  : Test GetPreferredInputStreamTypeInner interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_017, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    SourceType sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    DeviceType deviceType = DEVICE_TYPE_MIC;
    int32_t flags = 1;
    std::string networkId = "LocalDevice";
    AudioSamplingRate samplingRate = SAMPLE_RATE_16000;
    audioDeviceCommon.audioConfigManager_.OnVoipConfigParsed(true);
    int32_t ret = audioDeviceCommon.GetPreferredInputStreamTypeInner(sourceType,
        deviceType, flags, networkId, samplingRate);
    EXPECT_EQ(AUDIO_FLAG_VOIP_FAST, ret);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_018
* @tc.desc  : Test GetPreferredInputStreamTypeInner interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_018, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    SourceType sourceType = SOURCE_TYPE_MIC;
    DeviceType deviceType = DEVICE_TYPE_MIC;
    int32_t flags = 1;
    std::string networkId = "LocalDevice";
    AudioSamplingRate samplingRate = SAMPLE_RATE_16000;
    const std::unordered_map<AdaptersType, AudioAdapterInfo> adapterInfoMap;
    audioDeviceCommon.audioConfigManager_.OnAudioPolicyXmlParsingCompleted(adapterInfoMap);
    int32_t ret = audioDeviceCommon.GetPreferredInputStreamTypeInner(sourceType,
        deviceType, flags, networkId, samplingRate);
    EXPECT_EQ(AUDIO_FLAG_INVALID, ret);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_019
* @tc.desc  : Test GetPreferredInputStreamTypeInner interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_019, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    SourceType sourceType = SOURCE_TYPE_MIC;
    DeviceType deviceType = DEVICE_TYPE_MIC;
    int32_t flags = 1;
    std::string networkId = "LocalDevice";
    AudioSamplingRate samplingRate = SAMPLE_RATE_16000;
    AudioAdapterInfo adapterInfo = {};
    std::unordered_map<AdaptersType, AudioAdapterInfo> adapterInfoMap = {};
    adapterInfoMap.insert({AdaptersType::TYPE_PRIMARY, adapterInfo});
    audioDeviceCommon.audioConfigManager_.OnAudioPolicyXmlParsingCompleted(adapterInfoMap);
    int32_t ret = audioDeviceCommon.GetPreferredInputStreamTypeInner(sourceType,
        deviceType, flags, networkId, samplingRate);
    EXPECT_EQ(AUDIO_FLAG_INVALID, ret);
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
    EXPECT_EQ(false, audioDeviceCommon.hasDpDevice_);
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
    EXPECT_EQ(false, audioDeviceCommon.hasDpDevice_);
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
* @tc.number: AudioDeviceCommon_038
* @tc.desc  : Test NotifyRecreateDirectStream interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_038, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::unique_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_unique<AudioRendererChangeInfo>();
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    rendererChangeInfo->rendererInfo.pipeType = PIPE_TYPE_DIRECT_MUSIC;
    rendererChangeInfo->outputDeviceInfo.deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;
    bool ret = audioDeviceCommon.NotifyRecreateDirectStream(rendererChangeInfo, reason);
    EXPECT_EQ(false, ret);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_039
* @tc.desc  : Test NotifyRecreateDirectStream interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_039, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::unique_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_unique<AudioRendererChangeInfo>();
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    rendererChangeInfo->rendererInfo.pipeType = PIPE_TYPE_DIRECT_MUSIC;
    rendererChangeInfo->outputDeviceInfo.deviceType_ = DEVICE_TYPE_MIC;
    bool ret = audioDeviceCommon.NotifyRecreateDirectStream(rendererChangeInfo, reason);
    EXPECT_EQ(true, ret);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_040
* @tc.desc  : Test NotifyRecreateDirectStream interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_040, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::unique_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_unique<AudioRendererChangeInfo>();
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    audioDeviceCommon.audioActiveDevice_.SetCurrentOutputDeviceType(DEVICE_TYPE_WIRED_HEADSET);
    rendererChangeInfo->rendererInfo.pipeType = PIPE_TYPE_OFFLOAD;
    rendererChangeInfo->rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    rendererChangeInfo->rendererInfo.rendererFlags = AUDIO_FLAG_NORMAL;
    rendererChangeInfo->rendererInfo.samplingRate = SAMPLE_RATE_48000;
    rendererChangeInfo->rendererInfo.format = SAMPLE_S24LE;
    bool ret = audioDeviceCommon.NotifyRecreateDirectStream(rendererChangeInfo, reason);
    EXPECT_EQ(true, ret);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_041
* @tc.desc  : Test NotifyRecreateDirectStream interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_041, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::unique_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_unique<AudioRendererChangeInfo>();
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    audioDeviceCommon.audioActiveDevice_.SetCurrentOutputDeviceType(DEVICE_TYPE_WIRED_HEADSET);
    rendererChangeInfo->rendererInfo.pipeType = PIPE_TYPE_OFFLOAD;
    rendererChangeInfo->rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    rendererChangeInfo->rendererInfo.rendererFlags = AUDIO_FLAG_NORMAL;
    rendererChangeInfo->rendererInfo.samplingRate = SAMPLE_RATE_8000;
    rendererChangeInfo->rendererInfo.format = SAMPLE_S24LE;
    bool ret = audioDeviceCommon.NotifyRecreateDirectStream(rendererChangeInfo, reason);
    EXPECT_EQ(false, ret);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_042
* @tc.desc  : Test NotifyRecreateDirectStream interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_042, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::unique_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_unique<AudioRendererChangeInfo>();
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    audioDeviceCommon.audioActiveDevice_.SetCurrentOutputDeviceType(DEVICE_TYPE_USB_HEADSET);
    rendererChangeInfo->rendererInfo.pipeType = PIPE_TYPE_OFFLOAD;
    rendererChangeInfo->rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    rendererChangeInfo->rendererInfo.rendererFlags = AUDIO_FLAG_NORMAL;
    rendererChangeInfo->rendererInfo.samplingRate = SAMPLE_RATE_8000;
    rendererChangeInfo->rendererInfo.format = SAMPLE_S24LE;
    bool ret = audioDeviceCommon.NotifyRecreateDirectStream(rendererChangeInfo, reason);
    EXPECT_EQ(false, ret);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_043
* @tc.desc  : Test MuteSinkForSwtichGeneralDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_043, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::unique_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_unique<AudioRendererChangeInfo>();
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    std::unique_ptr<AudioDeviceDescriptor> audioDeviceDescriptorUniqueptr = std::make_unique<AudioDeviceDescriptor>();
    std::vector<std::unique_ptr<AudioDeviceDescriptor>> audioDeviceDescriptorUniqueptrVector;
    audioDeviceDescriptorUniqueptr->deviceType_ = DEVICE_TYPE_SPEAKER;
    audioDeviceDescriptorUniqueptrVector.push_back(std::move(audioDeviceDescriptorUniqueptr));
    audioDeviceCommon.MuteSinkForSwtichGeneralDevice(rendererChangeInfo, audioDeviceDescriptorUniqueptrVector, reason);
    EXPECT_EQ(1, audioDeviceDescriptorUniqueptrVector.size());
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_044
* @tc.desc  : Test MuteSinkForSwtichGeneralDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_044, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::unique_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_unique<AudioRendererChangeInfo>();
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    std::unique_ptr<AudioDeviceDescriptor> audioDeviceDescriptorUniqueptr = std::make_unique<AudioDeviceDescriptor>();
    std::vector<std::unique_ptr<AudioDeviceDescriptor>> audioDeviceDescriptorUniqueptrVector;
    audioDeviceDescriptorUniqueptr->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioDeviceDescriptorUniqueptrVector.push_back(std::move(audioDeviceDescriptorUniqueptr));
    audioDeviceCommon.MuteSinkForSwtichGeneralDevice(rendererChangeInfo, audioDeviceDescriptorUniqueptrVector, reason);
    EXPECT_EQ(1, audioDeviceDescriptorUniqueptrVector.size());
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_045
* @tc.desc  : Test MuteSinkForSwtichGeneralDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_045, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::unique_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_unique<AudioRendererChangeInfo>();
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    std::unique_ptr<AudioDeviceDescriptor> audioDeviceDescriptorUniqueptr = std::make_unique<AudioDeviceDescriptor>();
    std::vector<std::unique_ptr<AudioDeviceDescriptor>> audioDeviceDescriptorUniqueptrVector;
    audioDeviceDescriptorUniqueptr->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    audioDeviceDescriptorUniqueptrVector.push_back(std::move(audioDeviceDescriptorUniqueptr));
    audioDeviceCommon.MuteSinkForSwtichGeneralDevice(rendererChangeInfo, audioDeviceDescriptorUniqueptrVector, reason);
    EXPECT_EQ(1, audioDeviceDescriptorUniqueptrVector.size());
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_046
* @tc.desc  : Test MuteSinkForSwtichBluetoothDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_046, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::unique_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_unique<AudioRendererChangeInfo>();
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    std::unique_ptr<AudioDeviceDescriptor> audioDeviceDescriptorUniqueptr = std::make_unique<AudioDeviceDescriptor>();
    std::vector<std::unique_ptr<AudioDeviceDescriptor>> audioDeviceDescriptorUniqueptrVector;
    audioDeviceDescriptorUniqueptr->deviceType_ = DEVICE_TYPE_SPEAKER;
    audioDeviceDescriptorUniqueptrVector.push_back(std::move(audioDeviceDescriptorUniqueptr));
    audioDeviceCommon.MuteSinkForSwtichBluetoothDevice(rendererChangeInfo, audioDeviceDescriptorUniqueptrVector, reason);
    EXPECT_EQ(1, audioDeviceDescriptorUniqueptrVector.size());
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_047
* @tc.desc  : Test MuteSinkForSwtichBluetoothDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_047, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::unique_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_unique<AudioRendererChangeInfo>();
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    std::unique_ptr<AudioDeviceDescriptor> audioDeviceDescriptorUniqueptr = std::make_unique<AudioDeviceDescriptor>();
    std::vector<std::unique_ptr<AudioDeviceDescriptor>> audioDeviceDescriptorUniqueptrVector;
    audioDeviceDescriptorUniqueptr->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioDeviceDescriptorUniqueptrVector.push_back(std::move(audioDeviceDescriptorUniqueptr));
    audioDeviceCommon.MuteSinkForSwtichBluetoothDevice(rendererChangeInfo,
        audioDeviceDescriptorUniqueptrVector, reason);
    EXPECT_EQ(1, audioDeviceDescriptorUniqueptrVector.size());
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_048
* @tc.desc  : Test MuteSinkForSwtichBluetoothDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_048, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::unique_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_unique<AudioRendererChangeInfo>();
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    std::unique_ptr<AudioDeviceDescriptor> audioDeviceDescriptorUniqueptr = std::make_unique<AudioDeviceDescriptor>();
    std::vector<std::unique_ptr<AudioDeviceDescriptor>> audioDeviceDescriptorUniqueptrVector;
    audioDeviceDescriptorUniqueptr->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    audioDeviceDescriptorUniqueptrVector.push_back(std::move(audioDeviceDescriptorUniqueptr));
    audioDeviceCommon.MuteSinkForSwtichBluetoothDevice(rendererChangeInfo,
        audioDeviceDescriptorUniqueptrVector, reason);
    EXPECT_EQ(1, audioDeviceDescriptorUniqueptrVector.size());
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_049
* @tc.desc  : Test BluetoothDeviceFetchOutputHandle interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_049, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    unique_ptr<AudioDeviceDescriptor> desc = nullptr;
    unique_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_unique<AudioRendererChangeInfo>();
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    std::string encryptMacAddr;
    unique_ptr<AudioRendererChangeInfo> audioDeviceDescriptorUniqueptr = std::make_unique<AudioRendererChangeInfo>();
    vector<std::unique_ptr<AudioRendererChangeInfo>> rendererChangeInfos;
    rendererChangeInfos.push_back(std::move(audioDeviceDescriptorUniqueptr));
    int32_t ret = audioDeviceCommon.BluetoothDeviceFetchOutputHandle(desc, rendererChangeInfo, rendererChangeInfos,
        reason, encryptMacAddr);
    EXPECT_EQ(BLUETOOTH_FETCH_RESULT_CONTINUE, ret);

    desc = std::make_unique<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    rendererChangeInfo->outputDeviceInfo.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    rendererChangeInfo->rendererInfo.originalFlag = AUDIO_FLAG_MMAP;
    rendererChangeInfo->outputDeviceInfo.deviceId_ = 0;
    desc->deviceId_ = 1;
    ret = audioDeviceCommon.BluetoothDeviceFetchOutputHandle(desc, rendererChangeInfo, rendererChangeInfos,
        reason, encryptMacAddr);
    EXPECT_EQ(BLUETOOTH_FETCH_RESULT_CONTINUE, ret);

    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    ret = audioDeviceCommon.BluetoothDeviceFetchOutputHandle(desc, rendererChangeInfo, rendererChangeInfos,
        reason, encryptMacAddr);
    EXPECT_EQ(BLUETOOTH_FETCH_RESULT_ERROR, ret);

    desc->deviceType_ = DEVICE_TYPE_SPEAKER;
    ret = audioDeviceCommon.BluetoothDeviceFetchOutputHandle(desc, rendererChangeInfo, rendererChangeInfos,
        reason, encryptMacAddr);
    EXPECT_EQ(BLUETOOTH_FETCH_RESULT_DEFAULT, ret);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_050
* @tc.desc  : Test IsRendererStreamRunning interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_050, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    std::unique_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_unique<AudioRendererChangeInfo>();
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
    unique_ptr<AudioDeviceDescriptor> desc = std::make_unique<AudioDeviceDescriptor>();
    desc->isEnable_ = false;
    unique_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_unique<AudioRendererChangeInfo>();
    vector<std::unique_ptr<AudioRendererChangeInfo>> rendererChangeInfos;
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
    unique_ptr<AudioDeviceDescriptor> desc = std::make_unique<AudioDeviceDescriptor>();
    unique_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_unique<AudioRendererChangeInfo>();
    vector<std::unique_ptr<AudioRendererChangeInfo>> rendererChangeInfos;
    rendererChangeInfos.push_back(std::move(rendererChangeInfo));
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    int32_t ret = SUCCESS;
#ifdef BLUETOOTH_ENABLE
    ret = audioDeviceCommon.HandleScoOutputDeviceFetched(desc, rendererChangeInfos, reason);
    EXPECT_EQ(ERROR, ret);
#endif
    ret = audioDeviceCommon.HandleScoOutputDeviceFetched(desc, rendererChangeInfos, reason);
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
    unique_ptr<AudioDeviceDescriptor> desc = std::make_unique<AudioDeviceDescriptor>();
    unique_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_unique<AudioRendererChangeInfo>();
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
    EXPECT_EQ(true, ret);
}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_054
* @tc.desc  : Test NeedRehandleA2DPDevice interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_054, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    unique_ptr<AudioDeviceDescriptor> desc = std::make_unique<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_SPEAKER;
    bool ret = audioDeviceCommon.NeedRehandleA2DPDevice(desc);
    EXPECT_EQ(false, ret);

    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    ret = audioDeviceCommon.NeedRehandleA2DPDevice(desc);
    EXPECT_EQ(true, ret);
}
} // namespace AudioStandard
} // namespace OHOS
