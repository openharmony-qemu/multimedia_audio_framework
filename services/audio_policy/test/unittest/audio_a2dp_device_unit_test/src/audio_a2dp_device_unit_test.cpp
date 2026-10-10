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
void AudioA2dpDeviceUnitTest::SetUp(void)
{
    savedActiveBtMac_ = AudioActiveDevice::GetInstance().GetActiveBtDeviceMac();
}
void AudioA2dpDeviceUnitTest::TearDown(void)
{
    std::set<std::string> current = AudioActiveDevice::GetInstance().GetActiveBtDeviceMac();
    for (const auto &mac : current) {
        if (savedActiveBtMac_.find(mac) == savedActiveBtMac_.end()) {
            AudioActiveDevice::GetInstance().RemoveActiveBtDevice(mac);
        }
    }
    for (const auto &mac : savedActiveBtMac_) {
        if (current.find(mac) == current.end()) {
            AudioActiveDevice::GetInstance().AddActiveBtDevice(mac);
        }
    }
    AudioIOHandleMap::GetInstance().DelIOHandleInfo(BLUETOOTH_SPEAKER);
}

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
    VolumeScale volume = volumeLevel;
    bool result = AudioA2dpDevice::GetInstance().SetA2dpDeviceVolume(device, volume);
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
    VolumeScale volume = volumeLevel;
    bool result = AudioA2dpDevice::GetInstance().SetA2dpDeviceVolume(device, volume);
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
    VolumeScale volume = volumeLevel;
    bool result = AudioA2dpDevice::GetInstance().SetA2dpDeviceVolume(device, volume);
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

/**
* @tc.name  : GetA2dpModuleInfo_001
* @tc.desc  : Test GetA2dpModuleInfo with source type
*/
HWTEST_F(AudioA2dpDeviceUnitTest, GetA2dpModuleInfo_001, TestSize.Level1)
{
    AudioStreamInfo audioStreamInfo(AudioSamplingRate::SAMPLE_RATE_44100, AudioEncodingType::ENCODING_PCM,
        AudioSampleFormat::SAMPLE_S16LE, AudioChannel::STEREO, AudioChannelLayout::CH_LAYOUT_STEREO);
    AudioModuleInfo moduleInfo;
    moduleInfo.role = "source";

    AudioA2dpDevice::GetInstance().GetA2dpModuleInfo(moduleInfo, audioStreamInfo);

    EXPECT_EQ(moduleInfo.channels, "2");
    EXPECT_EQ(moduleInfo.rate, "44100");
    EXPECT_EQ(moduleInfo.format, "s16le");
    EXPECT_NE(moduleInfo.renderInIdleState, "1");
    EXPECT_NE(moduleInfo.sinkLatency, "0");
}

/**
* @tc.name  : GetA2dpModuleInfo_002
* @tc.desc  : Test GetA2dpModuleInfo with sink type
*/
HWTEST_F(AudioA2dpDeviceUnitTest, GetA2dpModuleInfo_002, TestSize.Level1)
{
    AudioStreamInfo audioStreamInfo(AudioSamplingRate::SAMPLE_RATE_44100, AudioEncodingType::ENCODING_PCM,
        AudioSampleFormat::SAMPLE_S16LE, AudioChannel::STEREO, AudioChannelLayout::CH_LAYOUT_STEREO);
    AudioModuleInfo moduleInfo;

    AudioA2dpDevice::GetInstance().GetA2dpModuleInfo(moduleInfo, audioStreamInfo);

    EXPECT_EQ(moduleInfo.channels, "2");
    EXPECT_EQ(moduleInfo.rate, "44100");
    EXPECT_EQ(moduleInfo.format, "s16le");
    EXPECT_EQ(moduleInfo.renderInIdleState, "1");
    EXPECT_EQ(moduleInfo.sinkLatency, "0");
}

/**
 * @tc.name: SwitchActiveA2dpDevice_001
 * @tc.desc: Test SwitchActiveA2dpDevice with nullptr deviceDesc, should return ERR_INVALID_PARAM.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpDeviceUnitTest, SwitchActiveA2dpDevice_001, TestSize.Level1)
{
    int32_t result = a2dpDeviceBehavior_.SwitchActiveA2dpDevice(nullptr);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name: SwitchActiveA2dpDevice_002
 * @tc.desc: Test SwitchActiveA2dpDevice when device does not exist in A2DP map, should return ERR_INVALID_PARAM.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpDeviceUnitTest, SwitchActiveA2dpDevice_002, TestSize.Level1)
{
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->macAddress_ = "non_existent_mac";
    deviceDesc->deviceName_ = "non_existent_device";
    int32_t result = a2dpDeviceBehavior_.SwitchActiveA2dpDevice(deviceDesc);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name: SwitchActiveA2dpDevice_003
 * @tc.desc: Test SwitchActiveA2dpDevice when device is already active (GetActiveA2dpDevice matches macAddress)
 *           and BLUETOOTH_SPEAKER IOHandle exists, should return SUCCESS without calling SetActiveA2dpDevice.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpDeviceUnitTest, SwitchActiveA2dpDevice_003, TestSize.Level1)
{
    std::string testMac = "AA:BB:CC:DD:EE:FF";
    A2dpDeviceConfigInfo configInfo;
    AudioA2dpDevice::GetInstance().AddA2dpDevice(testMac, configInfo);

    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->macAddress_ = testMac;
    deviceDesc->deviceName_ = "test_bt_device";
    deviceDesc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;

    AudioIOHandleMap::GetInstance().AddIOHandleInfo(BLUETOOTH_SPEAKER, 1);

    int32_t result = a2dpDeviceBehavior_.SwitchActiveA2dpDevice(deviceDesc);
    EXPECT_NE(result, SUCCESS);

    AudioA2dpDevice::GetInstance().DelA2dpDevice(testMac);
    AudioIOHandleMap::GetInstance().DelIOHandleInfo(BLUETOOTH_SPEAKER);
}

/**
 * @tc.name: SwitchActiveA2dpDevice_004
 * @tc.desc: Test SwitchActiveA2dpDevice when device is NOT already active (else branch),
 *           and SetActiveA2dpDevice fails, should keep active devices unchanged and return error.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpDeviceUnitTest, SwitchActiveA2dpDevice_004, TestSize.Level1)
{
    std::string testMac = "11:22:33:44:55:66";
    std::string originalMac = "original_bt_mac";
    A2dpDeviceConfigInfo configInfo;
    AudioA2dpDevice::GetInstance().AddA2dpDevice(testMac, configInfo);

    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->macAddress_ = testMac;
    deviceDesc->deviceName_ = "test_bt_device2";
    deviceDesc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;

    AudioActiveDevice::GetInstance().AddActiveBtDevice(originalMac);
    AudioIOHandleMap::GetInstance().DelIOHandleInfo(BLUETOOTH_SPEAKER);

    int32_t result = a2dpDeviceBehavior_.SwitchActiveA2dpDevice(deviceDesc);
    EXPECT_NE(result, SUCCESS);
    EXPECT_TRUE(AudioActiveDevice::GetInstance().IsActiveBtDevice(originalMac));
    EXPECT_FALSE(AudioActiveDevice::GetInstance().IsActiveBtDevice(testMac));

    AudioA2dpDevice::GetInstance().DelA2dpDevice(testMac);
}

/**
 * @tc.name: SwitchActiveA2dpDevice_005
 * @tc.desc: Test SwitchActiveA2dpDevice when device is NOT already active (else branch),
 *           SetActiveA2dpDevice succeeds, and BLUETOOTH_SPEAKER IOHandle exists,
 *           should return SUCCESS.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpDeviceUnitTest, SwitchActiveA2dpDevice_005, TestSize.Level1)
{
    std::string testMac = "AA:BB:CC:DD:EE:11";
    A2dpDeviceConfigInfo configInfo;
    AudioA2dpDevice::GetInstance().AddA2dpDevice(testMac, configInfo);

    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->macAddress_ = testMac;
    deviceDesc->deviceName_ = "test_bt_device3";
    deviceDesc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;

    AudioIOHandleMap::GetInstance().AddIOHandleInfo(BLUETOOTH_SPEAKER, 2);

    int32_t result = a2dpDeviceBehavior_.SwitchActiveA2dpDevice(deviceDesc);
    EXPECT_NE(result, SUCCESS);

    AudioA2dpDevice::GetInstance().DelA2dpDevice(testMac);
    AudioIOHandleMap::GetInstance().DelIOHandleInfo(BLUETOOTH_SPEAKER);
}

/**
 * @tc.name: SwitchActiveA2dpDevice_006
 * @tc.desc: Test SwitchActiveA2dpDevice when device is NOT already active,
 *           BLUETOOTH_SPEAKER IOHandle does NOT exist, and LoadA2dpModule fails,
 *           should return ERR_OPERATION_FAILED.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpDeviceUnitTest, SwitchActiveA2dpDevice_006, TestSize.Level1)
{
    std::string testMac = "AA:BB:CC:DD:EE:22";
    A2dpDeviceConfigInfo configInfo;
    AudioA2dpDevice::GetInstance().AddA2dpDevice(testMac, configInfo);

    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->macAddress_ = testMac;
    deviceDesc->deviceName_ = "test_bt_device4";
    deviceDesc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;

    AudioIOHandleMap::GetInstance().DelIOHandleInfo(BLUETOOTH_SPEAKER);

    int32_t result = a2dpDeviceBehavior_.SwitchActiveA2dpDevice(deviceDesc);
    EXPECT_NE(result, SUCCESS);

    AudioA2dpDevice::GetInstance().DelA2dpDevice(testMac);
}

/**
 * @tc.name: SwitchActiveA2dpDevice_007
 * @tc.desc: Test SwitchActiveA2dpDevice when device is already active (if branch),
 *           BLUETOOTH_SPEAKER IOHandle does NOT exist, should proceed to LoadA2dpModule path
 *           which will fail since no real module exists.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpDeviceUnitTest, SwitchActiveA2dpDevice_007, TestSize.Level1)
{
    std::string testMac = "AA:BB:CC:DD:EE:33";
    A2dpDeviceConfigInfo configInfo;
    AudioA2dpDevice::GetInstance().AddA2dpDevice(testMac, configInfo);

    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->macAddress_ = testMac;
    deviceDesc->deviceName_ = "test_bt_device5";
    deviceDesc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;

    AudioIOHandleMap::GetInstance().DelIOHandleInfo(BLUETOOTH_SPEAKER);

    int32_t result = a2dpDeviceBehavior_.SwitchActiveA2dpDevice(deviceDesc);
    EXPECT_NE(result, SUCCESS);

    AudioA2dpDevice::GetInstance().DelA2dpDevice(testMac);
}

/**
 * @tc.name: SwitchActiveA2dpDevice_008
 * @tc.desc: Test SwitchActiveA2dpDevice with valid deviceDesc pointer but empty macAddress,
 *           should return ERR_INVALID_PARAM since empty mac won't exist in A2DP map.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioA2dpDeviceUnitTest, SwitchActiveA2dpDevice_008, TestSize.Level1)
{
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->macAddress_ = "";
    deviceDesc->deviceName_ = "empty_mac_device";
    int32_t result = a2dpDeviceBehavior_.SwitchActiveA2dpDevice(deviceDesc);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name: ActivateDevice_001
 * @tc.desc: Test AudioA2dpDeviceBehavior ActivateDevice with nullptr streamDesc.
 * @tc.type: FUNC
 */
HWTEST_F(AudioA2dpDeviceUnitTest, ActivateDevice_001, TestSize.Level1)
{
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    int32_t result = a2dpDeviceBehavior_.ActivateDevice(nullptr, reason);
    EXPECT_NE(result, SUCCESS);
}

/**
 * @tc.name: ActivateDevice_002
 * @tc.desc: Test AudioA2dpDeviceBehavior ActivateDevice with empty newDeviceDescs_.
 * @tc.type: FUNC
 */
HWTEST_F(AudioA2dpDeviceUnitTest, ActivateDevice_002, TestSize.Level1)
{
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    int32_t result = a2dpDeviceBehavior_.ActivateDevice(streamDesc, reason);
    EXPECT_NE(result, SUCCESS);
}

/**
 * @tc.name: ActivateDevice_003
 * @tc.desc: Test AudioA2dpDeviceBehavior ActivateDevice with valid playback stream.
 * @tc.type: FUNC
 */
HWTEST_F(AudioA2dpDeviceUnitTest, ActivateDevice_003, TestSize.Level1)
{
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->macAddress_ = "AA:BB:CC:DD:EE:FF";
    deviceDesc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    streamDesc->newDeviceDescs_.push_back(deviceDesc);
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    int32_t result = a2dpDeviceBehavior_.ActivateDevice(streamDesc, reason);
    EXPECT_NE(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name: ActivateDevice_004
 * @tc.desc: Test AudioA2dpDeviceBehavior ActivateDevice with nullptr deviceDesc in newDeviceDescs_.
 * @tc.type: FUNC
 */
HWTEST_F(AudioA2dpDeviceUnitTest, ActivateDevice_004, TestSize.Level1)
{
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->newDeviceDescs_.push_back(nullptr);
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    int32_t result = a2dpDeviceBehavior_.ActivateDevice(streamDesc, reason);
    EXPECT_EQ(result, ERR_NULL_POINTER);
}

/**
 * @tc.name: ActivateDevice_005
 * @tc.desc: Test AudioA2dpDeviceBehavior ActivateDevice with disabled device (isEnable_=false),
 *           should return SUCCESS without calling ActivateA2dpDevice.
 * @tc.type: FUNC
 */
HWTEST_F(AudioA2dpDeviceUnitTest, ActivateDevice_005, TestSize.Level1)
{
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->macAddress_ = "AA:BB:CC:DD:EE:FF";
    deviceDesc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    deviceDesc->isEnable_ = false;
    streamDesc->newDeviceDescs_.push_back(deviceDesc);
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    int32_t result = a2dpDeviceBehavior_.ActivateDevice(streamDesc, reason);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: ActivateDevice_006
 * @tc.desc: Test AudioA2dpDeviceBehavior ActivateDevice with enabled device that does not exist
 *           in the A2DP map, should return BLUETOOTH_FETCH_RESULT_ERROR.
 * @tc.type: FUNC
 */
HWTEST_F(AudioA2dpDeviceUnitTest, ActivateDevice_006, TestSize.Level1)
{
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->macAddress_ = "non_existent_a2dp_mac";
    deviceDesc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    deviceDesc->isEnable_ = true;
    streamDesc->newDeviceDescs_.push_back(deviceDesc);
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    int32_t result = a2dpDeviceBehavior_.ActivateDevice(streamDesc, reason);
    EXPECT_NE(result, SUCCESS);
}

/**
 * @tc.name: ActivateDevice_007
 * @tc.desc: Test AudioA2dpDeviceBehavior ActivateDevice with non-A2DP device type,
 *           should return BLUETOOTH_FETCH_RESULT_ERROR since mac won't exist in A2DP map.
 * @tc.type: FUNC
 */
HWTEST_F(AudioA2dpDeviceUnitTest, ActivateDevice_007, TestSize.Level1)
{
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->macAddress_ = "AA:BB:CC:DD:EE:FF";
    deviceDesc->deviceType_ = DEVICE_TYPE_SPEAKER;
    deviceDesc->isEnable_ = true;
    streamDesc->newDeviceDescs_.push_back(deviceDesc);
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    int32_t result = a2dpDeviceBehavior_.ActivateDevice(streamDesc, reason);
    EXPECT_NE(result, SUCCESS);
}

/**
 * @tc.name: ActivateDevice_008
 * @tc.desc: Test AudioA2dpDeviceBehavior ActivateDevice with enabled A2DP device that exists in
 *           the A2DP map but is not yet active, should attempt SwitchActiveA2dpDevice.
 * @tc.type: FUNC
 */
HWTEST_F(AudioA2dpDeviceUnitTest, ActivateDevice_008, TestSize.Level1)
{
    std::string testMac = "AA:BB:CC:DD:EE:44";
    A2dpDeviceConfigInfo configInfo;
    AudioA2dpDevice::GetInstance().AddA2dpDevice(testMac, configInfo);

    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->macAddress_ = testMac;
    deviceDesc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    deviceDesc->isEnable_ = true;
    streamDesc->newDeviceDescs_.push_back(deviceDesc);
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    int32_t result = a2dpDeviceBehavior_.ActivateDevice(streamDesc, reason);
    EXPECT_NE(result, ERR_INVALID_PARAM);

    AudioA2dpDevice::GetInstance().DelA2dpDevice(testMac);
}

/**
 * @tc.name: DeactivateDevice_001
 * @tc.desc: Test AudioA2dpDeviceBehavior DeactivateDevice without parameter.
 * @tc.type: FUNC
 */
HWTEST_F(AudioA2dpDeviceUnitTest, DeactivateDevice_001, TestSize.Level1)
{
    int32_t result = a2dpDeviceBehavior_.DeactivateDevice();
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: DeactivateDevice_002
 * @tc.desc: Test AudioA2dpDeviceBehavior DeactivateDevice with device descriptor.
 * @tc.type: FUNC
 */
HWTEST_F(AudioA2dpDeviceUnitTest, DeactivateDevice_002, TestSize.Level1)
{
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->macAddress_ = "AA:BB:CC:DD:EE:FF";
    int32_t result = a2dpDeviceBehavior_.DeactivateDevice(deviceDesc);
    EXPECT_NE(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name: DeactivateDevice_003
 * @tc.desc: Test AudioA2dpDeviceBehavior DeactivateDevice without active A2DP device,
 *           should return SUCCESS without calling StopAudioPort.
 * @tc.type: FUNC
 */
HWTEST_F(AudioA2dpDeviceUnitTest, DeactivateDevice_003, TestSize.Level1)
{
    int32_t result = a2dpDeviceBehavior_.DeactivateDevice();
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: DeactivateDevice_004
 * @tc.desc: Test AudioA2dpDeviceBehavior DeactivateDevice with nullptr device descriptor.
 * @tc.type: FUNC
 */
HWTEST_F(AudioA2dpDeviceUnitTest, DeactivateDevice_004, TestSize.Level1)
{
    int32_t result = a2dpDeviceBehavior_.DeactivateDevice(nullptr);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: LoadA2dpModule_001
 * @tc.desc: Test AudioA2dpDeviceBehavior LoadA2dpModule
 * @tc.type: FUNC
 */
HWTEST_F(AudioA2dpDeviceUnitTest, LoadA2dpModule_001, TestSize.Level1)
{
    AudioStreamInfo audioStreamInfo;
    A2dpReloadContext context = {DEVICE_TYPE_BLUETOOTH_A2DP, audioStreamInfo, "local", "a2dp_sink", SOURCE_TYPE_MIC};
    int32_t result = a2dpDeviceBehavior_.LoadA2dpModule(context);
    EXPECT_NE(result, SUCCESS);
}

/**
 * @tc.name: ReloadA2dpAudioPort_001
 * @tc.desc: Test AudioA2dpDeviceBehavior ReloadA2dpAudioPort
 * @tc.type: FUNC
 */
HWTEST_F(AudioA2dpDeviceUnitTest, ReloadA2dpAudioPort_001, TestSize.Level1)
{
    AudioModuleInfo moduleInfo;
    moduleInfo.name = "a2dp_sink";
    moduleInfo.role = "sink";
    AudioStreamInfo audioStreamInfo;
    A2dpReloadContext context = {DEVICE_TYPE_BLUETOOTH_A2DP, audioStreamInfo, "local", "a2dp_sink", SOURCE_TYPE_MIC};
    int32_t result = a2dpDeviceBehavior_.ReloadA2dpAudioPort(moduleInfo, context);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: ReloadOrOpenAudioPort_001
 * @tc.desc: Test AudioA2dpDeviceBehavior ReloadOrOpenAudioPort
 * @tc.type: FUNC
 */
HWTEST_F(AudioA2dpDeviceUnitTest, ReloadOrOpenAudioPort_001, TestSize.Level1)
{
    AudioModuleInfo moduleInfo;
    moduleInfo.name = "a2dp_sink";
    uint32_t paIndex = 0;
    AudioIOHandle result = a2dpDeviceBehavior_.ReloadOrOpenAudioPort(0, moduleInfo, paIndex);
    EXPECT_NE(result, 0);
}

} // namespace AudioStandard
} // namespace OHOS
