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
#include "audio_policy_utils.h"
#include "audio_a2dp_device_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioA2dpDeviceUnitTest::SetUpTestCase(void) {}
void AudioA2dpDeviceUnitTest::TearDownTestCase(void) {}
void AudioA2dpDeviceUnitTest::SetUp(void) {}
void AudioA2dpDeviceUnitTest::TearDown(void) {}

/**
 * @tc.name: GetA2dpDeviceInfo_001
 * @tc.desc: Test GetA2dpDeviceInfo when the device exists in connectedA2dpDeviceMap_.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpDeviceUnitTest, GetA2dpDeviceInfo_001, TestSize.Level1)
{
    DeviceStreamInfo streamInfo(AudioSamplingRate::SAMPLE_RATE_44100, AudioEncodingType::ENCODING_PCM,
        AudioSampleFormat::SAMPLE_S16LE, AudioChannelLayout::CH_LAYOUT_STEREO);
    A2dpDeviceConfigInfo configInfo;
    configInfo.streamInfo = streamInfo;
    configInfo.absVolumeSupport = true;
    configInfo.volumeLevel = 50;
    configInfo.mute = false;
    std::string device = "test_device";
    AudioA2dpDevice::GetInstance().AddA2dpDevice(device, configInfo);
    A2dpDeviceConfigInfo info;
    bool result = AudioA2dpDevice::GetInstance().GetA2dpDeviceInfo(device, info);
    EXPECT_TRUE(result);
    EXPECT_EQ(info.streamInfo.encoding, AudioEncodingType::ENCODING_PCM);
    EXPECT_EQ(info.streamInfo.format, AudioSampleFormat::SAMPLE_S16LE);
    EXPECT_EQ(info.streamInfo.samplingRate.size(), 1);
    EXPECT_EQ(*info.streamInfo.samplingRate.begin(), AudioSamplingRate::SAMPLE_RATE_44100);
    EXPECT_EQ(info.streamInfo.channelLayout.size(), 1);
    EXPECT_EQ(*info.streamInfo.channelLayout.begin(), AudioChannelLayout::CH_LAYOUT_STEREO);
    EXPECT_TRUE(info.absVolumeSupport);
    EXPECT_EQ(info.volumeLevel, 50);
    EXPECT_FALSE(info.mute);
    AudioA2dpDevice::GetInstance().DelA2dpDevice(device);
}

/**
 * @tc.name: GetA2dpDeviceInfo_002
 * @tc.desc: Test GetA2dpDeviceInfo when the device does not exist in connectedA2dpDeviceMap_.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpDeviceUnitTest, GetA2dpDeviceInfo_002, TestSize.Level1)
{
    std::string device = "non_existent_device";
    A2dpDeviceConfigInfo info;
    bool result = AudioA2dpDevice::GetInstance().GetA2dpDeviceInfo(device, info);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: GetA2dpInDeviceInfo_001
 * @tc.desc: Test GetA2dpInDeviceInfo when the device exists in connectedA2dpInDeviceMap_.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpDeviceUnitTest, GetA2dpInDeviceInfo_001, TestSize.Level1)
{
    DeviceStreamInfo streamInfo(AudioSamplingRate::SAMPLE_RATE_44100, AudioEncodingType::ENCODING_PCM,
        AudioSampleFormat::SAMPLE_S16LE, AudioChannelLayout::CH_LAYOUT_STEREO);
    A2dpDeviceConfigInfo configInfo;
    configInfo.streamInfo = streamInfo;
    configInfo.absVolumeSupport = true;
    configInfo.volumeLevel = 50;
    configInfo.mute = false;
    std::string device = "test_device";
    AudioA2dpDevice::GetInstance().AddA2dpInDevice(device, configInfo);
    A2dpDeviceConfigInfo info;
    bool result = AudioA2dpDevice::GetInstance().GetA2dpInDeviceInfo(device, info);
    EXPECT_TRUE(result);
    EXPECT_EQ(info.streamInfo.encoding, AudioEncodingType::ENCODING_PCM);
    EXPECT_EQ(info.streamInfo.format, AudioSampleFormat::SAMPLE_S16LE);
    EXPECT_EQ(info.streamInfo.samplingRate.size(), 1);
    EXPECT_EQ(*info.streamInfo.samplingRate.begin(), AudioSamplingRate::SAMPLE_RATE_44100);
    EXPECT_EQ(info.streamInfo.channelLayout.size(), 1);
    EXPECT_EQ(*info.streamInfo.channelLayout.begin(), CH_LAYOUT_STEREO);
    EXPECT_TRUE(info.absVolumeSupport);
    EXPECT_EQ(info.volumeLevel, 50);
    EXPECT_FALSE(info.mute);
    AudioA2dpDevice::GetInstance().DelA2dpInDevice(device);
}

/**
 * @tc.name: GetA2dpDeviceVolumeLevel_001
 * @tc.desc: Test GetA2dpDeviceVolumeLevel when the device exists in connectedA2dpDeviceMap_.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpDeviceUnitTest, GetA2dpDeviceVolumeLevel_001, TestSize.Level1)
{
    A2dpDeviceConfigInfo configInfo;
    configInfo.volumeLevel = 50;
    std::string device = "test_device";
    AudioA2dpDevice::GetInstance().AddA2dpDevice(device, configInfo);
    int32_t volumeLevel;
    bool result = AudioA2dpDevice::GetInstance().GetA2dpDeviceVolumeLevel(device, volumeLevel);
    EXPECT_TRUE(result);
    EXPECT_EQ(volumeLevel, 50);
    AudioA2dpDevice::GetInstance().DelA2dpDevice(device);
}

/**
 * @tc.name: GetA2dpDeviceVolumeLevel_002
 * @tc.desc: Test GetA2dpDeviceVolumeLevel when the device does not exist in connectedA2dpDeviceMap_.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpDeviceUnitTest, GetA2dpDeviceVolumeLevel_002, TestSize.Level1)
{
    std::string device = "non_existent_device";
    int32_t volumeLevel;
    bool result = AudioA2dpDevice::GetInstance().GetA2dpDeviceVolumeLevel(device, volumeLevel);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: CheckA2dpDeviceExist_001
 * @tc.desc: Test CheckA2dpDeviceExist when the device exists in connectedA2dpDeviceMap_.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpDeviceUnitTest, CheckA2dpDeviceExist_001, TestSize.Level1)
{
    A2dpDeviceConfigInfo configInfo;
    std::string device = "test_device";
    AudioA2dpDevice::GetInstance().AddA2dpDevice(device, configInfo);
    bool result = AudioA2dpDevice::GetInstance().CheckA2dpDeviceExist(device);
    EXPECT_TRUE(result);
    AudioA2dpDevice::GetInstance().DelA2dpDevice(device);
}

/**
 * @tc.name: SetA2dpDeviceMute_001
 * @tc.desc: Test SetA2dpDeviceMute when the device does not exist in connectedA2dpDeviceMap_.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpDeviceUnitTest, SetA2dpDeviceMute_001, TestSize.Level1)
{
    std::string device = "non_existent_device";
    bool result = AudioA2dpDevice::GetInstance().SetA2dpDeviceMute(device, true);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: SetA2dpDeviceMute_002
 * @tc.desc: Test SetA2dpDeviceMute when the device exists but does not support absolute volume control.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpDeviceUnitTest, SetA2dpDeviceMute_002, TestSize.Level1)
{
    A2dpDeviceConfigInfo configInfo;
    configInfo.absVolumeSupport = false;
    std::string device = "test_device";
    AudioA2dpDevice::GetInstance().AddA2dpDevice(device, configInfo);
    bool result = AudioA2dpDevice::GetInstance().SetA2dpDeviceMute(device, true);
    EXPECT_FALSE(result);
    AudioA2dpDevice::GetInstance().DelA2dpDevice(device);
}

/**
 * @tc.name: SetA2dpDeviceMute_003
 * @tc.desc: Test SetA2dpDeviceMute when the device exists and supports absolute volume control.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpDeviceUnitTest, SetA2dpDeviceMute_003, TestSize.Level1)
{
    A2dpDeviceConfigInfo configInfo;
    configInfo.absVolumeSupport = true;
    std::string device = "test_device";
    AudioA2dpDevice::GetInstance().AddA2dpDevice(device, configInfo);
    bool result = AudioA2dpDevice::GetInstance().SetA2dpDeviceMute(device, true);
    EXPECT_TRUE(result);
    A2dpDeviceConfigInfo info;
    bool getInfoResult = AudioA2dpDevice::GetInstance().GetA2dpDeviceInfo(device, info);
    EXPECT_TRUE(getInfoResult);
    EXPECT_TRUE(info.mute);
    AudioA2dpDevice::GetInstance().DelA2dpDevice(device);
}

/**
 * @tc.name: GetA2dpDeviceMute_001
 * @tc.desc: Test GetA2dpDeviceMute when the device does not exist in connectedA2dpDeviceMap_.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpDeviceUnitTest, GetA2dpDeviceMute_001, TestSize.Level1)
{
    std::string device = "non_existent_device";
    bool isMute = false;
    bool result = AudioA2dpDevice::GetInstance().GetA2dpDeviceMute(device, isMute);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: GetA2dpDeviceMute_002
 * @tc.desc: Test GetA2dpDeviceMute when the device exists but does not support absolute volume control.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpDeviceUnitTest, GetA2dpDeviceMute_002, TestSize.Level1)
{
    A2dpDeviceConfigInfo configInfo;
    configInfo.absVolumeSupport = false;
    std::string device = "test_device";
    AudioA2dpDevice::GetInstance().AddA2dpDevice(device, configInfo);

    bool isMute = false;
    bool result = AudioA2dpDevice::GetInstance().GetA2dpDeviceMute(device, isMute);
    EXPECT_FALSE(result);
    AudioA2dpDevice::GetInstance().DelA2dpDevice(device);
}

/**
 * @tc.name: GetA2dpDeviceMute_003
 * @tc.desc: Test GetA2dpDeviceMute when the device exists and supports absolute volume control.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpDeviceUnitTest, GetA2dpDeviceMute_003, TestSize.Level1)
{
    A2dpDeviceConfigInfo configInfo;
    configInfo.absVolumeSupport = true;
    configInfo.mute = true;
    std::string device = "test_device";
    AudioA2dpDevice::GetInstance().AddA2dpDevice(device, configInfo);

    bool isMute = false;
    bool result = AudioA2dpDevice::GetInstance().GetA2dpDeviceMute(device, isMute);
    EXPECT_TRUE(result);
    EXPECT_TRUE(isMute);
    AudioA2dpDevice::GetInstance().DelA2dpDevice(device);
}

/**
 * @tc.name: SetA2dpDeviceAbsVolumeSupport_001
 * @tc.desc: Test SetA2dpDeviceAbsVolumeSupport when the device exists in connectedA2dpDeviceMap_.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpDeviceUnitTest, SetA2dpDeviceAbsVolumeSupport_001, TestSize.Level1)
{
    A2dpDeviceConfigInfo configInfo;
    configInfo.absVolumeSupport = false;
    std::string device = "test_device";
    AudioA2dpDevice::GetInstance().AddA2dpDevice(device, configInfo);
    bool support = true;
    int32_t volume = 0;
    bool mute = true;
    bool result = AudioA2dpDevice::GetInstance().SetA2dpDeviceAbsVolumeSupport(device, support, volume, mute);
    EXPECT_TRUE(result);
    A2dpDeviceConfigInfo info;
    bool getInfoResult = AudioA2dpDevice::GetInstance().GetA2dpDeviceInfo(device, info);
    EXPECT_TRUE(getInfoResult);
    EXPECT_TRUE(info.absVolumeSupport);
    AudioA2dpDevice::GetInstance().DelA2dpDevice(device);
}

/**
 * @tc.name: SetA2dpDeviceAbsVolumeSupport_002
 * @tc.desc: Test SetA2dpDeviceAbsVolumeSupport when the device does not exist in connectedA2dpDeviceMap_.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpDeviceUnitTest, SetA2dpDeviceAbsVolumeSupport_002, TestSize.Level1)
{
    std::string device = "non_existent_device";
    bool support = true;
    int32_t volume = 0;
    bool mute = true;
    bool result = AudioA2dpDevice::GetInstance().SetA2dpDeviceAbsVolumeSupport(device, support, volume, mute);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: SetA2dpDeviceVolumeLevel_001
 * @tc.desc: Test SetA2dpDeviceVolumeLevel when the device does not exist in connectedA2dpDeviceMap_.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpDeviceUnitTest, SetA2dpDeviceVolumeLevel_001, TestSize.Level1)
{
    std::string device = "non_existent_device";
    int32_t volumeLevel = 50;
    bool result = AudioA2dpDevice::GetInstance().SetA2dpDeviceVolumeLevel(device, volumeLevel);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: SetA2dpDeviceVolumeLevel_002
 * @tc.desc: Test SetA2dpDeviceVolumeLevel when the device exists but does not support absolute volume control.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpDeviceUnitTest, SetA2dpDeviceVolumeLevel_002, TestSize.Level1)
{
    A2dpDeviceConfigInfo configInfo;
    configInfo.absVolumeSupport = false;
    std::string device = "test_device";
    AudioA2dpDevice::GetInstance().AddA2dpDevice(device, configInfo);
    int32_t volumeLevel = 50;
    bool result = AudioA2dpDevice::GetInstance().SetA2dpDeviceVolumeLevel(device, volumeLevel);
    EXPECT_FALSE(result);
    AudioA2dpDevice::GetInstance().DelA2dpDevice(device);
}

/**
 * @tc.name: SetA2dpDeviceVolumeLevel_003
 * @tc.desc: Test SetA2dpDeviceVolumeLevel when the device exists and supports absolute volume control.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpDeviceUnitTest, SetA2dpDeviceVolumeLevel_003, TestSize.Level1)
{
    A2dpDeviceConfigInfo configInfo;
    configInfo.absVolumeSupport = true;
    std::string device = "test_device";
    AudioA2dpDevice::GetInstance().AddA2dpDevice(device, configInfo);
    int32_t volumeLevel = 50;
    bool result = AudioA2dpDevice::GetInstance().SetA2dpDeviceVolumeLevel(device, volumeLevel);
    EXPECT_TRUE(result);
    A2dpDeviceConfigInfo info;
    bool getInfoResult = AudioA2dpDevice::GetInstance().GetA2dpDeviceInfo(device, info);
    EXPECT_TRUE(getInfoResult);
    EXPECT_EQ(info.volumeLevel, volumeLevel);
    AudioA2dpDevice::GetInstance().DelA2dpDevice(device);
}

/**
 * @tc.name: CheckHearingAidDeviceExist_001
 * @tc.desc: Test CheckHearingAidDeviceExist_001 when the device exists and supports absolute volume control.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpDeviceUnitTest, CheckHearingAidDeviceExist_001, TestSize.Level1)
{
    A2dpDeviceConfigInfo configInfo;
    std::string device = "test_device";
    AudioA2dpDevice::GetInstance().AddHearingAidDevice(device, configInfo);

    bool result = AudioA2dpDevice::GetInstance().CheckHearingAidDeviceExist(device);
    EXPECT_TRUE(result);
    AudioA2dpDevice::GetInstance().DelHearingAidDevice(device);

    result = AudioA2dpDevice::GetInstance().CheckHearingAidDeviceExist(device);
    EXPECT_FALSE(result);
}
} // namespace AudioStandard
} // namespace OHOS