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
} // namespace AudioStandard
} // namespace OHOS
