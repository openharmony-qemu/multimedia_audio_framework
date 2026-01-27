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

#include "audio_ec_manager_unit_test.h"
#include "audio_device_info.h"
#include "audio_ec_manager.cpp"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

const static uint64_t TEST_SESSION_ID = 1;

void AudioEcManagerUnitTest::SetUpTestCase(void) {}
void AudioEcManagerUnitTest::TearDownTestCase(void) {}
void AudioEcManagerUnitTest::SetUp(void) {}
void AudioEcManagerUnitTest::TearDown(void) {}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: AudioEcManager_001
* @tc.desc  : Test GetEcSamplingRate interface.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_001, TestSize.Level1)
{
    std::string halName = DP_CLASS;
    std::shared_ptr<PipeStreamPropInfo> outModuleInfo = std::make_shared<PipeStreamPropInfo>();
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    std::string sRet;

    outModuleInfo->sampleRate_ = 41000;
    sRet = ecManager.GetEcSamplingRate(halName, outModuleInfo);
    EXPECT_EQ(sRet, "41000");

    ecManager.dpSinkModuleInfo_.rate = "48000";
    sRet = ecManager.GetEcSamplingRate(halName, outModuleInfo);
    EXPECT_EQ(sRet, "48000");

    halName = USB_CLASS;
    sRet = ecManager.GetEcSamplingRate(halName, outModuleInfo);
    EXPECT_EQ(sRet, "41000");

    ecManager.usbSinkModuleInfo_.rate = "48000";
    sRet = ecManager.GetEcSamplingRate(halName, outModuleInfo);
    EXPECT_EQ(sRet, "48000");

    halName = "TEST";
    ecManager.primaryMicModuleInfo_.rate = "40000";
    sRet = ecManager.GetEcSamplingRate(halName, outModuleInfo);
    EXPECT_EQ(sRet, "40000");
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: AudioEcManager_002
* @tc.desc  : Test GetEcChannels interface.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_002, TestSize.Level1)
{
    std::string halName = DP_CLASS;
    std::shared_ptr<PipeStreamPropInfo> outModuleInfo = std::make_shared<PipeStreamPropInfo>();
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    std::string sRet;

    outModuleInfo->channelLayout_ = CH_LAYOUT_STEREO;
    ecManager.dpSinkModuleInfo_.channels = "";
    sRet = ecManager.GetEcChannels(halName, outModuleInfo);
    EXPECT_EQ(sRet, "0");

    ecManager.dpSinkModuleInfo_.channels = "3";
    sRet = ecManager.GetEcChannels(halName, outModuleInfo);
    EXPECT_EQ(sRet, "3");

    halName = USB_CLASS;
    ecManager.usbSinkModuleInfo_.channels = "";
    sRet = ecManager.GetEcChannels(halName, outModuleInfo);
    EXPECT_EQ(sRet, "0");

    ecManager.usbSinkModuleInfo_.channels = "5";
    sRet = ecManager.GetEcChannels(halName, outModuleInfo);
    EXPECT_EQ(sRet, "5");

    halName = "TEST";
    sRet = ecManager.GetEcChannels(halName, outModuleInfo);
    EXPECT_EQ(sRet, "2");
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: AudioEcManager_003
* @tc.desc  : Test GetEcFormat interface.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_003, TestSize.Level1)
{
    std::string halName = DP_CLASS;
    std::shared_ptr<PipeStreamPropInfo> outModuleInfo = std::make_shared<PipeStreamPropInfo>();
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    std::string sRet;

    outModuleInfo->format_ = SAMPLE_S32LE;
    ecManager.dpSinkModuleInfo_.format = "";
    sRet = ecManager.GetEcFormat(halName, outModuleInfo);
    EXPECT_EQ(sRet, "s32le");

    ecManager.dpSinkModuleInfo_.format = "4";
    sRet = ecManager.GetEcFormat(halName, outModuleInfo);
    EXPECT_EQ(sRet, "4");

    halName = USB_CLASS;
    ecManager.usbSinkModuleInfo_.format = "";
    sRet = ecManager.GetEcFormat(halName, outModuleInfo);
    EXPECT_EQ(sRet, "s32le");

    ecManager.usbSinkModuleInfo_.format = "5";
    sRet = ecManager.GetEcFormat(halName, outModuleInfo);
    EXPECT_EQ(sRet, "5");

    halName = "TEST";
    ecManager.primaryMicModuleInfo_.format = "2";
    sRet = ecManager.GetEcFormat(halName, outModuleInfo);
    EXPECT_EQ(sRet, "2");
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: AudioEcManager_004
* @tc.desc  : Test GetPipeNameByDeviceForEc interface.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_004, TestSize.Level1)
{
    std::string role;
    DeviceType deviceType = DEVICE_TYPE_SPEAKER;
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    std::string sRet;

    sRet = ecManager.GetPipeNameByDeviceForEc(role, deviceType);
    EXPECT_EQ(sRet, PIPE_PRIMARY_OUTPUT);

    deviceType = DEVICE_TYPE_WIRED_HEADSET;
    sRet = ecManager.GetPipeNameByDeviceForEc(role, deviceType);
    EXPECT_EQ(sRet, PIPE_PRIMARY_OUTPUT);

    deviceType = DEVICE_TYPE_USB_HEADSET;
    sRet = ecManager.GetPipeNameByDeviceForEc(role, deviceType);
    EXPECT_EQ(sRet, PIPE_PRIMARY_OUTPUT);

    deviceType = DEVICE_TYPE_BLUETOOTH_SCO;
    sRet = ecManager.GetPipeNameByDeviceForEc(role, deviceType);
    EXPECT_EQ(sRet, PIPE_PRIMARY_OUTPUT);

    deviceType = DEVICE_TYPE_NEARLINK;
    sRet = ecManager.GetPipeNameByDeviceForEc(role, deviceType);
    EXPECT_EQ(sRet, PIPE_PRIMARY_OUTPUT);

    role = ROLE_SOURCE;
    deviceType = DEVICE_TYPE_WIRED_HEADSET;
    sRet = ecManager.GetPipeNameByDeviceForEc(role, deviceType);
    EXPECT_EQ(sRet, PIPE_PRIMARY_INPUT);

    deviceType = DEVICE_TYPE_USB_HEADSET;
    sRet = ecManager.GetPipeNameByDeviceForEc(role, deviceType);
    EXPECT_EQ(sRet, PIPE_PRIMARY_INPUT);

    deviceType = DEVICE_TYPE_BLUETOOTH_SCO;
    sRet = ecManager.GetPipeNameByDeviceForEc(role, deviceType);
    EXPECT_EQ(sRet, PIPE_PRIMARY_INPUT);

    deviceType = DEVICE_TYPE_NEARLINK_IN;
    sRet = ecManager.GetPipeNameByDeviceForEc(role, deviceType);
    EXPECT_EQ(sRet, PIPE_PRIMARY_INPUT);

    deviceType = DEVICE_TYPE_MIC;
    sRet = ecManager.GetPipeNameByDeviceForEc(role, deviceType);
    EXPECT_EQ(sRet, PIPE_PRIMARY_INPUT);

    deviceType = DEVICE_TYPE_USB_ARM_HEADSET;
    sRet = ecManager.GetPipeNameByDeviceForEc(role, deviceType);
    EXPECT_EQ(sRet, PIPE_USB_ARM_INPUT);

    role = "TEST";
    sRet = ecManager.GetPipeNameByDeviceForEc(role, deviceType);
    EXPECT_EQ(sRet, PIPE_USB_ARM_OUTPUT);

    deviceType = DEVICE_TYPE_DP;
    sRet = ecManager.GetPipeNameByDeviceForEc(role, deviceType);
    EXPECT_EQ(sRet, PIPE_DP_OUTPUT);

    deviceType = DEVICE_TYPE_NONE;
    sRet = ecManager.GetPipeNameByDeviceForEc(role, deviceType);
    EXPECT_EQ(sRet, PIPE_PRIMARY_OUTPUT);
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: AudioEcManager_005
* @tc.desc  : Test GetPipeInfoByDeviceTypeForEc interface.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_005, TestSize.Level1)
{
    std::string role = ROLE_SOURCE;
    DeviceType deviceType = DEVICE_TYPE_SPEAKER;
    std::shared_ptr<AdapterPipeInfo> pipeInfo;
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    int32_t ret;

    ret = ecManager.GetPipeInfoByDeviceTypeForEc(role, deviceType, pipeInfo);
    EXPECT_NE(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: AudioEcManager_006
* @tc.desc  : Test GetEcType interface.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_006, TestSize.Level1)
{
    DeviceType inputDevice;
    DeviceType outputDevice;
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    EcType ecRet;

    inputDevice = DEVICE_TYPE_MIC;
    outputDevice = DEVICE_TYPE_SPEAKER;
    ecRet = ecManager.GetEcType(inputDevice, outputDevice);
    EXPECT_EQ(ecRet, EC_TYPE_SAME_ADAPTER);

    outputDevice = DEVICE_TYPE_MIC;
    ecRet = ecManager.GetEcType(inputDevice, outputDevice);
    EXPECT_EQ(ecRet, EC_TYPE_NONE);
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: AudioEcManager_007
* @tc.desc  : Test UpdateAudioEcInfo interface.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_007, TestSize.Level1)
{
    AudioDeviceDescriptor inputDevice;
    AudioDeviceDescriptor outputDevice;
    AudioEcManager& ecManager(AudioEcManager::GetInstance());

    inputDevice.deviceType_ = DEVICE_TYPE_MIC;
    inputDevice.macAddress_ = "00:11:22:33:44:55";
    inputDevice.networkId_ = "1234567890";
    inputDevice.deviceRole_ = DEVICE_ROLE_NONE;
    outputDevice.deviceType_ = DEVICE_TYPE_MIC;
    outputDevice.macAddress_ = "00:11:22:33:44:55";
    outputDevice.networkId_ = "1234567890";
    outputDevice.deviceRole_ = DEVICE_ROLE_NONE;

    ecManager.audioEcInfo_.inputDevice.deviceType_ = DEVICE_TYPE_MIC;
    ecManager.audioEcInfo_.inputDevice.macAddress_ = "00:11:22:33:44:55";
    ecManager.audioEcInfo_.inputDevice.networkId_ = "1234567890";
    ecManager.audioEcInfo_.inputDevice.deviceRole_ = DEVICE_ROLE_NONE;
    ecManager.audioEcInfo_.outputDevice.deviceType_ = DEVICE_TYPE_MIC;
    ecManager.audioEcInfo_.outputDevice.macAddress_ = "00:11:22:33:44:55";
    ecManager.audioEcInfo_.outputDevice.networkId_ = "1234567890";
    ecManager.audioEcInfo_.outputDevice.deviceRole_ = DEVICE_ROLE_NONE;

    ecManager.isEcFeatureEnable_ = false;
    ecManager.UpdateAudioEcInfo(inputDevice, outputDevice);
    EXPECT_EQ(ecManager.isEcFeatureEnable_, false);

    ecManager.isEcFeatureEnable_ = true;
    ecManager.UpdateAudioEcInfo(inputDevice, outputDevice);
    EXPECT_EQ(ecManager.audioEcInfo_.inputDevice.IsSameDeviceDesc(inputDevice), true);

    inputDevice.networkId_ = "12345678";
    outputDevice.networkId_ = "12345678";
    ecManager.UpdateAudioEcInfo(inputDevice, outputDevice);
    EXPECT_EQ(ecManager.isEcFeatureEnable_, true);
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: AudioEcManager_008
* @tc.desc  : Test UpdateModuleInfoForEc interface.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_008, TestSize.Level1)
{
    AudioModuleInfo moduleInfo;
    AudioEcManager& ecManager(AudioEcManager::GetInstance());

    ecManager.audioEcInfo_.channels = "5";
    ecManager.UpdateModuleInfoForEc(moduleInfo);
    EXPECT_EQ(moduleInfo.ecChannels, "5");
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: AudioEcManager_009
* @tc.desc  : Test ShouldOpenMicRef interface.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_009, TestSize.Level1)
{
    SourceType source = SOURCE_TYPE_VOICE_COMMUNICATION;
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    std::string sRet;

    ecManager.isMicRefFeatureEnable_ = false;
    sRet = ecManager.ShouldOpenMicRef(source);
    EXPECT_EQ(sRet, "0");

    ecManager.isMicRefFeatureEnable_ = true;
    sRet = ecManager.ShouldOpenMicRef(source);
    EXPECT_EQ(sRet, "0");
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: AudioEcManager_010
* @tc.desc  : Test UpdateModuleInfoForMicRef interface.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_010, TestSize.Level1)
{
    AudioModuleInfo moduleInfo;
    SourceType source = SOURCE_TYPE_VOICE_COMMUNICATION;
    AudioEcManager& ecManager(AudioEcManager::GetInstance());

    ecManager.isMicRefFeatureEnable_ = false;
    ecManager.UpdateModuleInfoForMicRef(moduleInfo, source);
    EXPECT_EQ(moduleInfo.micRefChannels, "4");
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: AudioEcManager_011
* @tc.desc  : Test GetAudioEcInfo & ResetAudioEcInfo interface.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_011, TestSize.Level1)
{
    AudioEcInfo ecInfo;
    AudioEcManager& ecManager(AudioEcManager::GetInstance());

    ecManager.audioEcInfo_.channels = "3";
    ecInfo = ecManager.GetAudioEcInfo();
    EXPECT_EQ(ecInfo.channels, "3");

    ecManager.ResetAudioEcInfo();
    ecInfo = ecManager.GetAudioEcInfo();
    EXPECT_EQ(ecInfo.inputDevice.deviceType_, DEVICE_TYPE_NONE);
    EXPECT_EQ(ecInfo.outputDevice.deviceType_, DEVICE_TYPE_NONE);
}

/**
 * @tc.name  : Test UpdateArmModuleInfo API with valid parameters
 * @tc.type  : FUNC
 * @tc.number: AudioEcManagerUpdateArmModuleInfo_001
 * @tc.desc  : Test UpdateArmModuleInfo interface with valid device descriptor.
 */
HWTEST_F(AudioEcManagerUnitTest, AudioEcManagerUpdateArmModuleInfo_001, TestSize.Level1)
{
    AudioEcManager& ecManager = AudioEcManager::GetInstance();
    // Create valid AudioDeviceDescriptor
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    DeviceStreamInfo streamInfo;
    streamInfo.samplingRate = {AudioSamplingRate::SAMPLE_RATE_48000};
    streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    std::list<DeviceStreamInfo> streamInfos = {streamInfo};
    deviceDesc->audioStreamInfo_ = streamInfos;
    
    AudioModuleInfo moduleInfo;
    moduleInfo.channels = "2";
    
    ecManager.UpdateArmModuleInfo(deviceDesc, moduleInfo);
    
    EXPECT_EQ(moduleInfo.rate, "48000");
    EXPECT_EQ(moduleInfo.format, "s16le");
    EXPECT_FALSE(moduleInfo.bufferSize.empty());
}

/**
 * @tc.name  : Test UpdateArmModuleInfo API with null device descriptor
 * @tc.type  : FUNC
 * @tc.number: AudioEcManagerUpdateArmModuleInfo_002
 * @tc.desc  : Test UpdateArmModuleInfo interface with null device descriptor.
 */
HWTEST_F(AudioEcManagerUnitTest, AudioEcManagerUpdateArmModuleInfo_002, TestSize.Level1)
{
    AudioEcManager& ecManager = AudioEcManager::GetInstance();
    
    AudioModuleInfo moduleInfo;
    moduleInfo.rate = "44100";
    moduleInfo.format = "s16le";
    moduleInfo.channels = "2";
    
    // Save original values
    std::string originalRate = moduleInfo.rate;
    std::string originalFormat = moduleInfo.format;
    std::string originalChannels = moduleInfo.channels;
    
    // Pass null pointer, function should return directly without modifying moduleInfo
    ecManager.UpdateArmModuleInfo(nullptr, moduleInfo);
    
    EXPECT_EQ(moduleInfo.rate, originalRate);
    EXPECT_EQ(moduleInfo.format, originalFormat);
    EXPECT_EQ(moduleInfo.channels, originalChannels);
}

/**
 * @tc.name  : Test UpdateArmModuleInfo API with empty stream info
 * @tc.type  : FUNC
 * @tc.number: AudioEcManagerUpdateArmModuleInfo_003
 * @tc.desc  : Test UpdateArmModuleInfo interface with empty stream info.
 */
HWTEST_F(AudioEcManagerUnitTest, AudioEcManagerUpdateArmModuleInfo_003, TestSize.Level1)
{
    AudioEcManager& ecManager = AudioEcManager::GetInstance();
    
    // Create AudioDeviceDescriptor without setting AudioStreamInfo
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    
    AudioModuleInfo moduleInfo;
    moduleInfo.rate = "44100";
    moduleInfo.format = "s16le";
    moduleInfo.channels = "2";
    
    // Save original values
    std::string originalRate = moduleInfo.rate;
    std::string originalFormat = moduleInfo.format;
    std::string originalChannels = moduleInfo.channels;
    
    ecManager.UpdateArmModuleInfo(deviceDesc, moduleInfo);
    
    // Since AudioStreamInfo is empty, function should return directly without modifying moduleInfo
    EXPECT_EQ(moduleInfo.rate, originalRate);
    EXPECT_EQ(moduleInfo.format, originalFormat);
    EXPECT_EQ(moduleInfo.channels, originalChannels);
}

/**
 * @tc.name  : Test UpdateArmModuleInfo API with empty sampling rate
 * @tc.type  : FUNC
 * @tc.number: AudioEcManagerUpdateArmModuleInfo_004
 * @tc.desc  : Test UpdateArmModuleInfo interface with empty sampling rate.
 */
HWTEST_F(AudioEcManagerUnitTest, AudioEcManagerUpdateArmModuleInfo_004, TestSize.Level1)
{
    AudioEcManager& ecManager = AudioEcManager::GetInstance();
    
    // Create AudioDeviceDescriptor but set empty samplingRate
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    DeviceStreamInfo streamInfo;
    // Don't set samplingRate, keep it empty
    streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    std::list<DeviceStreamInfo> streamInfos = {streamInfo};
    deviceDesc->audioStreamInfo_ = streamInfos;
    
    AudioModuleInfo moduleInfo;
    moduleInfo.rate = "44100";
    moduleInfo.format = "s16le";
    moduleInfo.channels = "2";
    
    // Save original values
    std::string originalRate = moduleInfo.rate;
    std::string originalFormat = moduleInfo.format;
    std::string originalChannels = moduleInfo.channels;
    
    ecManager.UpdateArmModuleInfo(deviceDesc, moduleInfo);
    
    // Since samplingRate is empty, function should return directly without modifying moduleInfo
    EXPECT_EQ(moduleInfo.rate, originalRate);
    EXPECT_EQ(moduleInfo.format, originalFormat);
    EXPECT_EQ(moduleInfo.channels, originalChannels);
}

/**
 * @tc.name  : Test UpdateArmModuleInfo API with different sampling rates and formats
 * @tc.type  : FUNC
 * @tc.number: AudioEcManagerUpdateArmModuleInfo_005
 * @tc.desc  : Test UpdateArmModuleInfo interface with different sampling rates and formats.
 */
HWTEST_F(AudioEcManagerUnitTest, AudioEcManagerUpdateArmModuleInfo_005, TestSize.Level1)
{
    AudioEcManager& ecManager = AudioEcManager::GetInstance();
    
    // Test with different sampling rate
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    DeviceStreamInfo streamInfo;
    streamInfo.samplingRate = {AudioSamplingRate::SAMPLE_RATE_44100};  // Different sampling rate
    streamInfo.format = AudioSampleFormat::SAMPLE_S24LE;
    std::list<DeviceStreamInfo> streamInfos = {streamInfo};
    deviceDesc->audioStreamInfo_ = streamInfos;
    
    AudioModuleInfo moduleInfo;
    moduleInfo.channels = "1";  // Different channel count
    
    ecManager.UpdateArmModuleInfo(deviceDesc, moduleInfo);
    
    EXPECT_EQ(moduleInfo.rate, "44100");
    EXPECT_EQ(moduleInfo.format, "s24le");
    EXPECT_FALSE(moduleInfo.bufferSize.empty());
    
    // Verify buffer size calculation is correct (44100 * 1 * 3 * 20 / 1000 = 2646)
    uint32_t expectedBufferSize = 44100 * 1 * 3 * 20 / 1000;
    EXPECT_EQ(moduleInfo.bufferSize, std::to_string(expectedBufferSize));
}

/**
 * @tc.name  : Test UpdateArmModuleInfo API with buffer size calculation
 * @tc.type  : FUNC
 * @tc.number: AudioEcManagerUpdateArmModuleInfo_006
 * @tc.desc  : Test UpdateArmModuleInfo interface buffer size calculation.
 */
HWTEST_F(AudioEcManagerUnitTest, AudioEcManagerUpdateArmModuleInfo_006, TestSize.Level1)
{
    AudioEcManager& ecManager = AudioEcManager::GetInstance();
    
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    DeviceStreamInfo streamInfo;
    streamInfo.samplingRate = {AudioSamplingRate::SAMPLE_RATE_96000};
    streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    std::list<DeviceStreamInfo> streamInfos = {streamInfo};
    deviceDesc->audioStreamInfo_ = streamInfos;
    
    AudioModuleInfo moduleInfo;
    moduleInfo.channels = "4";
    
    ecManager.UpdateArmModuleInfo(deviceDesc, moduleInfo);
    
    EXPECT_EQ(moduleInfo.rate, "96000");
    EXPECT_EQ(moduleInfo.format, "s16le");
    EXPECT_FALSE(moduleInfo.bufferSize.empty());
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: AudioEcManager_013
* @tc.desc  : Test ReloadSourceForSession interface.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_013, TestSize.Level1)
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    SessionInfo sessionInfo;
    sessionInfo.sourceType = SOURCE_TYPE_MIC;

    int32_t ret = ecManager.ReloadSourceForSession(sessionInfo);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: AudioEcManager_014
* @tc.desc  : Test GetMicRefFeatureEnable interface.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_014, TestSize.Level1)
{
    bool bRet;
    AudioEcManager& ecManager(AudioEcManager::GetInstance());

    ecManager.isMicRefFeatureEnable_ = true;
    bRet = ecManager.GetMicRefFeatureEnable();
    EXPECT_EQ(bRet, true);
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: AudioEcManager_015
* @tc.desc  : Test UpdateStreamEcAndMicRefInfo interface.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_015, TestSize.Level1)
{
    AudioModuleInfo moduleInfo;
    SourceType sourceType = SOURCE_TYPE_INVALID;
    auto ecManager = std::make_shared<AudioEcManager>();
    ASSERT_TRUE(ecManager != nullptr);

    ecManager->UpdateStreamEcAndMicRefInfo(moduleInfo, sourceType);
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: AudioEcManager_016
* @tc.desc  : Test GetHalNameForDevice interface.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_016, TestSize.Level1)
{
    std::string role;
    DeviceType deviceType;
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    std::string sRet;

    role = ROLE_SOURCE;
    deviceType = DEVICE_TYPE_MIC;
    sRet = ecManager.GetHalNameForDevice(role, deviceType);
    EXPECT_EQ(sRet, "");

    role = ROLE_SINK;
    sRet = ecManager.GetHalNameForDevice(role, deviceType);
    EXPECT_EQ(sRet, "");
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: AudioEcManager_017
* @tc.desc  : Test Init & GetEcFeatureEnable & GetMicRefFeatureEnable interface.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_017, TestSize.Level1)
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());

    ecManager.Init(0, 1);
    EXPECT_EQ(ecManager.GetEcFeatureEnable(), false);
    EXPECT_EQ(ecManager.GetMicRefFeatureEnable(), true);

    ecManager.Init(1, 0);
    EXPECT_EQ(ecManager.GetEcFeatureEnable(), true);
    EXPECT_EQ(ecManager.GetMicRefFeatureEnable(), false);

    ecManager.Init(1, 1);
    EXPECT_EQ(ecManager.GetEcFeatureEnable(), true);
    EXPECT_EQ(ecManager.GetMicRefFeatureEnable(), true);

    ecManager.Init(0, 0);
    EXPECT_EQ(ecManager.GetEcFeatureEnable(), false);
    EXPECT_EQ(ecManager.GetMicRefFeatureEnable(), false);
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: AudioEcManager_018
* @tc.desc  : Test CloseNormalSource & GetSourceOpened interface.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_018, TestSize.Level1)
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());

    EXPECT_EQ(ecManager.GetSourceOpened(), SOURCE_TYPE_MIC);

    ecManager.Init(1, 0);
    bool isEcFeatureEnable = ecManager.isEcFeatureEnable_;
    ecManager.isEcFeatureEnable_ = true;
    ecManager.CloseNormalSource();
    ecManager.isEcFeatureEnable_ = isEcFeatureEnable;
    EXPECT_EQ(ecManager.GetSourceOpened(), SOURCE_TYPE_INVALID);
    ecManager.Init(0, 0);
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: AudioEcManager_021
* @tc.desc  : Test GetTargetSourceTypeAndMatchingFlag interface.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_021, TestSize.Level1)
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());

    SourceType targetSource;
    bool useMatchingPropInfo;

    ecManager.GetTargetSourceTypeAndMatchingFlag(SOURCE_TYPE_VOICE_RECOGNITION, targetSource, useMatchingPropInfo);
    EXPECT_EQ(targetSource, SOURCE_TYPE_VOICE_RECOGNITION);

    ecManager.GetTargetSourceTypeAndMatchingFlag(SOURCE_TYPE_VOICE_COMMUNICATION, targetSource, useMatchingPropInfo);
    EXPECT_EQ(targetSource, SOURCE_TYPE_VOICE_COMMUNICATION);

    ecManager.GetTargetSourceTypeAndMatchingFlag(SOURCE_TYPE_VOICE_CALL, targetSource, useMatchingPropInfo);
    EXPECT_EQ(targetSource, SOURCE_TYPE_VOICE_CALL);

    ecManager.GetTargetSourceTypeAndMatchingFlag(SOURCE_TYPE_UNPROCESSED, targetSource, useMatchingPropInfo);
    EXPECT_EQ(targetSource, SOURCE_TYPE_UNPROCESSED);

    ecManager.GetTargetSourceTypeAndMatchingFlag(SOURCE_TYPE_MIC, targetSource, useMatchingPropInfo);
    EXPECT_EQ(targetSource, SOURCE_TYPE_MIC);
}

/**
* @tc.name  : Test AudioEcManager ActivateArmDevice with multiple modules.
* @tc.number: AudioEcManager_ActivateArmDevice_002
* @tc.desc  : Test ActivateArmDevice interface with multiple modules in the list.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_ActivateArmDevice_002, TestSize.Level1)
{
    AudioEcManager& ecManager = AudioEcManager::GetInstance();
    
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    std::string inputAddress = "card=2;device=0";
    deviceDesc->macAddress_ = inputAddress;
    deviceDesc->deviceRole_ = DeviceRole::INPUT_DEVICE;
    
    ecManager.isEcFeatureEnable_ = true;
    ecManager.activeArmInputAddr_ = "";
    
    // Setup multiple modules - one matching role, one not matching
    AudioModuleInfo sinkModule{.role = "sink", .name = "usb_sink_module", .rate = "48000"};
    AudioModuleInfo sourceModule{.role = "source", .name = "usb_source_module", .rate = "48000"};
    std::list<AudioModuleInfo> multiModuleList{sinkModule, sourceModule};
    ecManager.audioConfigManager_.deviceClassInfo_[ClassType::TYPE_USB] = multiModuleList;
    
    ecManager.ActivateArmDevice(deviceDesc);
    EXPECT_EQ(ecManager.activeArmInputAddr_, inputAddress);
    EXPECT_EQ(ecManager.usbSourceModuleInfo_.name, "usb_source_module");
}

/**
* @tc.name  : Test AudioEcManager ActivateArmDevice with different roles.
* @tc.number: AudioEcManager_ActivateArmDevice_003
* @tc.desc  : Test ActivateArmDevice interface with different device roles.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_ActivateArmDevice_003, TestSize.Level1)
{
    AudioEcManager& ecManager = AudioEcManager::GetInstance();
    
    // Test with INPUT_DEVICE role
    auto inputDeviceDesc = std::make_shared<AudioDeviceDescriptor>();
    std::string inputAddress = "card=2;device=0";
    inputDeviceDesc->macAddress_ = inputAddress;
    inputDeviceDesc->deviceRole_ = DeviceRole::INPUT_DEVICE;
    
    ecManager.isEcFeatureEnable_ = true;
    ecManager.activeArmInputAddr_ = "";
    
    AudioModuleInfo inputModule{.role = "source", .name = "input_module", .rate = "44100"};
    std::list<AudioModuleInfo> inputModuleList{inputModule};
    ecManager.audioConfigManager_.deviceClassInfo_[ClassType::TYPE_USB] = inputModuleList;
    
    ecManager.ActivateArmDevice(inputDeviceDesc);
    EXPECT_EQ(ecManager.activeArmInputAddr_, inputAddress);
    
    // Test with OUTPUT_DEVICE role
    ecManager.activeArmOutputAddr_ = "";
    auto outputDeviceDesc = std::make_shared<AudioDeviceDescriptor>();
    std::string outputAddress = "card=2;device=0";
    outputDeviceDesc->macAddress_ = outputAddress;
    outputDeviceDesc->deviceType_ = DeviceType::DEVICE_TYPE_USB_ARM_HEADSET;
    outputDeviceDesc->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    std::string parameters = "sink_format:AUDIO_FORMAT_PCM_16_BIT;sink_rate:48000;"
        "source_format:AUDIO_FORMAT_PCM_16_BIT;source_rate:8000;sink_mmap:1";
    outputDeviceDesc->ParseAudioParameters(parameters);
    parameters = "sink_format:AUDIO_FORMAT_PCM_24_BIT;sink_rate:48000;"
        "source_format:AUDIO_FORMAT_PCM_16_BIT;source_rate:8000;sink_mmap:1";
    outputDeviceDesc->ParseAudioParameters(parameters);
    parameters = "sink_format:AUDIO_FORMAT_PCM_24_BIT_PACKED;sink_rate:48000;"
        "source_format:AUDIO_FORMAT_PCM_16_BIT;source_rate:8000;sink_mmap:1";
    outputDeviceDesc->ParseAudioParameters(parameters);
    parameters = "sink_format:AUDIO_FORMAT_PCM_32_BIT;sink_rate:48000;"
        "source_format:AUDIO_FORMAT_PCM_16_BIT;source_rate:8000;sink_mmap:1";
    outputDeviceDesc->ParseAudioParameters(parameters);
    parameters = "sink_format:AUDIO_FORMAT_PCM_64_BIT;sink_rate:48000;"
        "source_format:AUDIO_FORMAT_PCM_16_BIT;source_rate:8000;sink_mmap:1";
    outputDeviceDesc->ParseAudioParameters(parameters);
    AudioModuleInfo outputModule{.role = "sink", .name = "output_module", .rate = "48000"};
    std::list<AudioModuleInfo> outputModuleList{outputModule};
    ecManager.audioConfigManager_.deviceClassInfo_[ClassType::TYPE_USB] = outputModuleList;
    
    ecManager.ActivateArmDevice(outputDeviceDesc);
    EXPECT_EQ(ecManager.activeArmOutputAddr_, outputAddress);
    EXPECT_EQ(ecManager.usbSinkModuleInfo_.name, "output_module");
}

/**
* @tc.name  : Test AudioEcManager ActivateArmDevice with no matching modules.
* @tc.number: AudioEcManager_ActivateArmDevice_004
* @tc.desc  : Test ActivateArmDevice interface when no modules match the device role.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_ActivateArmDevice_004, TestSize.Level1)
{
    AudioEcManager& ecManager = AudioEcManager::GetInstance();
    
    auto inputDeviceDesc = std::make_shared<AudioDeviceDescriptor>();
    std::string inputAddress = "card=2;device=0";
    inputDeviceDesc->macAddress_ = inputAddress;
    inputDeviceDesc->deviceRole_ = DeviceRole::INPUT_DEVICE;
    
    ecManager.isEcFeatureEnable_ = true;
    ecManager.activeArmInputAddr_ = "";
    
    // Setup module list with only sink modules (no source modules for input device)
    AudioModuleInfo sinkModule{.role = "sink", .name = "sink_module", .rate = "48000"};
    std::list<AudioModuleInfo> moduleList{sinkModule};
    ecManager.audioConfigManager_.deviceClassInfo_[ClassType::TYPE_USB] = moduleList;
    
    // This should not crash but also not update the usbSourceModuleInfo_ since no matching role exists
    ecManager.ActivateArmDevice(inputDeviceDesc);
    EXPECT_EQ(ecManager.activeArmInputAddr_, inputAddress);
}

/**
* @tc.name  : Test AudioEcManager ActivateArmDevice with mixed role modules.
* @tc.number: AudioEcManager_ActivateArmDevice_005
* @tc.desc  : Test ActivateArmDevice interface with modules of mixed roles.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_ActivateArmDevice_005, TestSize.Level1)
{
    AudioEcManager& ecManager = AudioEcManager::GetInstance();
    
    auto inputDeviceDesc = std::make_shared<AudioDeviceDescriptor>();
    std::string inputAddress = "card=2;device=0";
    inputDeviceDesc->macAddress_ = inputAddress;
    inputDeviceDesc->deviceRole_ = DeviceRole::INPUT_DEVICE;
    
    ecManager.isEcFeatureEnable_ = true;
    ecManager.activeArmInputAddr_ = "";
    
    // Setup module list with both sink and source modules
    AudioModuleInfo sinkModule{.role = "sink", .name = "sink_module", .rate = "48000"};
    AudioModuleInfo sourceModule{.role = "source", .name = "source_module", .rate = "48000"};
    std::list<AudioModuleInfo> mixedModuleList{sinkModule, sourceModule};
    ecManager.audioConfigManager_.deviceClassInfo_[ClassType::TYPE_USB] = mixedModuleList;
    
    ecManager.ActivateArmDevice(inputDeviceDesc);
    EXPECT_EQ(ecManager.activeArmInputAddr_, inputAddress);
    EXPECT_EQ(ecManager.usbSourceModuleInfo_.name, "source_module");
}

/**
* @tc.name  : Test AudioEcManager ActivateArmDevice with EC disabled and output device.
* @tc.number: AudioEcManager_ActivateArmDevice_006
* @tc.desc  : Test ActivateArmDevice interface with EC feature disabled for output device.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_ActivateArmDevice_006, TestSize.Level1)
{
    AudioEcManager& ecManager = AudioEcManager::GetInstance();
    
    auto outputDeviceDesc = std::make_shared<AudioDeviceDescriptor>();
    std::string outputAddress = "card=2;device=0";
    outputDeviceDesc->macAddress_ = outputAddress;
    outputDeviceDesc->deviceRole_ = DeviceRole::INPUT_DEVICE;
    
    ecManager.isEcFeatureEnable_ = false;
    ecManager.activeArmOutputAddr_ = "";
    
    // Setup module list with sink role
    AudioModuleInfo outputModule{.role = "sink", .name = "output_module_no_ec", .rate = "48000"};
    std::list<AudioModuleInfo> outputModuleList{outputModule};
    ecManager.audioConfigManager_.deviceClassInfo_[ClassType::TYPE_USB] = outputModuleList;
    
    ecManager.ActivateArmDevice(outputDeviceDesc);
    EXPECT_EQ(ecManager.activeArmOutputAddr_, "");
    // When EC is disabled, usbSinkModuleInfo_ should still be set for output devices
    EXPECT_EQ(ecManager.usbSinkModuleInfo_.name, "output_module");
}

/**
* @tc.name  : Test AudioEcManager ActivateArmDevice with existing IO handle.
* @tc.number: AudioEcManager_ActivateArmDevice_007
* @tc.desc  : Test ActivateArmDevice interface when there's an existing IO handle for input device.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_ActivateArmDevice_007, TestSize.Level1)
{
    AudioEcManager& ecManager = AudioEcManager::GetInstance();
    
    // First, add an IO handle to simulate existing handle
    AudioIOHandle testHandle = 12345;
    ecManager.audioIOHandleMap_.AddIOHandleInfo("existing_module", testHandle);
    
    auto inputDeviceDesc = std::make_shared<AudioDeviceDescriptor>();
    std::string inputAddress = "card=2;device=0";
    inputDeviceDesc->macAddress_ = inputAddress;
    inputDeviceDesc->deviceRole_ = DeviceRole::INPUT_DEVICE;
    
    ecManager.isEcFeatureEnable_ = true;
    ecManager.activeArmInputAddr_ = "";
    
    // Setup module list with source role
    AudioModuleInfo inputModule{.role = "source", .name = "existing_module", .rate = "48000"};
    std::list<AudioModuleInfo> inputModuleList{inputModule};
    ecManager.audioConfigManager_.deviceClassInfo_[ClassType::TYPE_USB] = inputModuleList;
    
    ecManager.ActivateArmDevice(inputDeviceDesc);
    EXPECT_EQ(ecManager.activeArmInputAddr_, inputAddress);
    EXPECT_EQ(ecManager.usbSourceModuleInfo_.name, "existing_module");
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: AudioEcManager_024
* @tc.desc  : Test UpdateStreamEcInfo interface.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_024, TestSize.Level4)
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    AudioModuleInfo moduleInfo;
    SourceType sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    EXPECT_NO_THROW(ecManager.UpdateStreamEcInfo(moduleInfo, sourceType));
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: AudioEcManager_025
* @tc.desc  : Test PresetArmIdleInput interface.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_025, TestSize.Level4)
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    ecManager.isEcFeatureEnable_ = false;
    ecManager.usbSourceModuleInfo_.role = "";
    ecManager.PresetArmIdleInput(deviceDesc);
    EXPECT_TRUE(ecManager.usbSourceModuleInfo_.role.empty());
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: AudioEcManager_026
* @tc.desc  : Test CloseUsbArmDevice interface.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_026, TestSize.Level4)
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    AudioDeviceDescriptor device(DEVICE_TYPE_EARPIECE, INPUT_DEVICE);
    device.macAddress_ = "00:11:22:33:44:55";
    ecManager.activeArmInputAddr_ = device.macAddress_;
    EXPECT_NO_THROW(ecManager.CloseUsbArmDevice(device));

    device.deviceRole_ = OUTPUT_DEVICE;
    ecManager.activeArmOutputAddr_ = device.macAddress_;
    EXPECT_NO_THROW(ecManager.CloseUsbArmDevice(device));
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: AudioEcManager_027
* @tc.desc  : Test GetTargetSourceTypeAndMatchingFlag interface.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_027, TestSize.Level4)
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    SourceType source = SOURCE_TYPE_LIVE;
    SourceType targetSource = SOURCE_TYPE_INVALID;
    bool useMatchingPropInfo = false;
    ecManager.GetTargetSourceTypeAndMatchingFlag(source, targetSource, useMatchingPropInfo);
    EXPECT_EQ(targetSource, SOURCE_TYPE_LIVE);
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: AudioEcManager_028
* @tc.desc  : Test UpdateStreamCommonInfo interface.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_028, TestSize.Level4)
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    bool originIsEcFeatureEnable = ecManager.isEcFeatureEnable_;
    ecManager.isEcFeatureEnable_ = false;
    AudioModuleInfo moduleInfo = {};
    PipeStreamPropInfo targetInfo = PipeStreamPropInfo();
    SourceType sourceType = SourceType::SOURCE_TYPE_MIC;
    ecManager.UpdateStreamCommonInfo(moduleInfo, targetInfo, sourceType);
    EXPECT_EQ(moduleInfo.sourceType, "0");

    ecManager.isEcFeatureEnable_ = true;
    ecManager.UpdateStreamCommonInfo(moduleInfo, targetInfo, sourceType);
    EXPECT_EQ(moduleInfo.sourceType, "0");

    ecManager.isEcFeatureEnable_ = originIsEcFeatureEnable;
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: AudioEcManager_029
* @tc.desc  : Test UpdateEnhanceEffectState interface.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_029, TestSize.Level4)
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    SourceType sourceType = SourceType::SOURCE_TYPE_MIC;
    ecManager.UpdateEnhanceEffectState(sourceType);
    EXPECT_EQ(ecManager.isMicRefRecordOn_, false);
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: AudioEcManager_030
* @tc.desc  : Test UpdateStreamMicRefInfo interface.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_030, TestSize.Level4)
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    AudioModuleInfo moduleInfo = {};
    SourceType sourceType = SourceType::SOURCE_TYPE_MIC;
    EXPECT_NO_THROW(
        ecManager.UpdateStreamMicRefInfo(moduleInfo, sourceType);
    );

    sourceType = SourceType::SOURCE_TYPE_VOICE_COMMUNICATION;
    EXPECT_NO_THROW(
        ecManager.UpdateStreamMicRefInfo(moduleInfo, sourceType);
    );

    sourceType = SOURCE_TYPE_INVALID;
    EXPECT_NO_THROW(
        ecManager.UpdateStreamMicRefInfo(moduleInfo, sourceType);
    );
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: AudioEcManager_031
* @tc.desc  : Test ReloadNormalSource interface.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_031, TestSize.Level4)
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    SessionInfo sessionInfo = {};
    PipeStreamPropInfo targetInfo = PipeStreamPropInfo();
    SourceType targetSource = SourceType::SOURCE_TYPE_MIC;
    int32_t ret = ecManager.ReloadNormalSource(sessionInfo, targetInfo, targetSource);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: AudioEcManager_032
* @tc.desc  : Test GetOpenedNormalSourceSessionId interface.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_032, TestSize.Level4)
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    uint64_t ret = ecManager.GetOpenedNormalSourceSessionId();
    EXPECT_EQ(ret, 0);
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: AudioEcManager_033
* @tc.desc  : Test SetOpenedNormalSourceSessionId interface.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_033, TestSize.Level4)
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    uint64_t originSessionId = ecManager.sessionIdUsedToOpenSource_;
    uint64_t sessionId = TEST_SESSION_ID;
    ecManager.SetOpenedNormalSourceSessionId(sessionId);
    EXPECT_EQ(ecManager.sessionIdUsedToOpenSource_, sessionId);

    ecManager.sessionIdUsedToOpenSource_ = originSessionId;
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: AudioEcManager_034
* @tc.desc  : Test PrepareNormalSource interface.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_034, TestSize.Level4)
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    EXPECT_NO_THROW(
        ecManager.PrepareNormalSource(pipeInfo, streamDesc);
    );
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: AudioEcManager_035
* @tc.desc  : Test SetOpenedNormalSource interface.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_035, TestSize.Level4)
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    SourceType origin = ecManager.normalSourceOpened_;
    SourceType sourceType = SourceType::SOURCE_TYPE_MIC;
    ecManager.SetOpenedNormalSource(sourceType);
    EXPECT_EQ(sourceType, ecManager.normalSourceOpened_);

    ecManager.normalSourceOpened_ = origin;
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: AudioEcManager_036
* @tc.desc  : Test SetPrimaryMicModuleInfo interface.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_036, TestSize.Level4)
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    AudioModuleInfo moduleInfo = {};
    moduleInfo.name = "test";
    ecManager.SetPrimaryMicModuleInfo(moduleInfo);
    EXPECT_EQ(ecManager.primaryMicModuleInfo_.name, moduleInfo.name);
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: AudioEcManager_037
* @tc.desc  : Test SetDpSinkModuleInfo interface.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_037, TestSize.Level4)
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    AudioModuleInfo moduleInfo = {};
    moduleInfo.className = "AudioEcManagerUnitTest";
    ecManager.SetDpSinkModuleInfo(moduleInfo);
    EXPECT_EQ(ecManager.dpSinkModuleInfo_.className, moduleInfo.className);
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: AudioEcManager_038
* @tc.desc  : Test FetchTargetInfoForSessionAdd interface.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_038, TestSize.Level4)
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    SessionInfo sessionInfo = {};
    PipeStreamPropInfo targetInfo = PipeStreamPropInfo();
    SourceType targetSourceType = SourceType::SOURCE_TYPE_MIC;
    int32_t ret = ecManager.FetchTargetInfoForSessionAdd(sessionInfo, targetInfo, targetSourceType);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: AudioEcManager_039
* @tc.desc  : Test FetchTargetInfoForSessionAdd interface.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_039, TestSize.Level4)
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    SessionInfo sessionInfo = {};
    PipeStreamPropInfo targetInfo = PipeStreamPropInfo();
    SourceType targetSourceType = SourceType::SOURCE_TYPE_MIC;
    bool originEcFeatureEnable_ = ecManager.isEcFeatureEnable_;
    std::string originMicSpeaker = ecManager.primaryMicModuleInfo_.OpenMicSpeaker;

    ecManager.isEcFeatureEnable_ = false;
    ecManager.primaryMicModuleInfo_.OpenMicSpeaker = "0";
    int32_t ret = ecManager.FetchTargetInfoForSessionAdd(sessionInfo, targetInfo, targetSourceType);
    EXPECT_EQ(ret, ERROR);

    ecManager.isEcFeatureEnable_ = true;
    ecManager.primaryMicModuleInfo_.OpenMicSpeaker = "1";
    ret = ecManager.FetchTargetInfoForSessionAdd(sessionInfo, targetInfo, targetSourceType);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: AudioEcManager_040
* @tc.desc  : Test UpdatePrimaryMicModuleInfo interface.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_040, TestSize.Level4)
{
    SourceType sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    auto ecManager = std::make_shared<AudioEcManager>();
    ASSERT_TRUE(ecManager != nullptr);
 
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    ecManager->isEcFeatureEnable_ = true;
    ecManager->UpdatePrimaryMicModuleInfo(pipeInfo, sourceType);
    EXPECT_EQ(ecManager->primaryMicModuleInfo_.channels, "");
    EXPECT_EQ(ecManager->primaryMicModuleInfo_.rate, "");
    EXPECT_EQ(ecManager->primaryMicModuleInfo_.format, "");
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: AudioEcManager_041
* @tc.desc  : Test ReloadNormalSource interface with invalid sessionId.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_041, TestSize.Level4)
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    SessionInfo sessionInfo = {};
    PipeStreamPropInfo targetInfo = PipeStreamPropInfo();
    SourceType targetSource = SourceType::SOURCE_TYPE_MIC;
    uint32_t testSessionId = 123;
    int32_t ret = ecManager.ReloadNormalSource(sessionInfo, targetInfo, targetSource, testSessionId);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: AudioEcManager_042
* @tc.desc  : Test ReloadNormalSource interface with invalid sessionId.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_042, TestSize.Level4)
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    SessionInfo sessionInfo = {};
    PipeStreamPropInfo targetInfo = PipeStreamPropInfo();
    SourceType targetSource = SourceType::SOURCE_TYPE_VOICE_COMMUNICATION;
    uint32_t testSessionId = 1234;

    AudioInjectorPolicy::GetInstance().SetCapturePortIdx(4321);
    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->paIndex_ = 4321;
    pipe1->pipeRole_ = PIPE_ROLE_INPUT;
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = 1234;
    pipe1->streamDescriptors_.push_back(desc);
    AudioPipeManager::GetPipeManager()->AddAudioPipeInfo(pipe1);
    int32_t ret = ecManager.ReloadNormalSource(sessionInfo, targetInfo, targetSource, testSessionId);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: AudioEcManager_043
* @tc.desc  : Test ReloadNormalSource interface with invalid sessionId.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_043, TestSize.Level4)
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    SessionInfo sessionInfo = {};
    PipeStreamPropInfo targetInfo = PipeStreamPropInfo();
    SourceType targetSource = SourceType::SOURCE_TYPE_VOICE_CALL;
    uint32_t testSessionId = 1234;

    AudioInjectorPolicy::GetInstance().SetCapturePortIdx(4321);
    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->paIndex_ = 4321;
    pipe1->pipeRole_ = PIPE_ROLE_INPUT;
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = 1234;
    pipe1->streamDescriptors_.push_back(desc);
    AudioPipeManager::GetPipeManager()->AddAudioPipeInfo(pipe1);
    int32_t ret = ecManager.ReloadNormalSource(sessionInfo, targetInfo, targetSource, testSessionId);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: AudioEcManager_044
* @tc.desc  : Test ReloadNormalSource interface with invalid sessionId.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_044, TestSize.Level4)
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    SessionInfo sessionInfo = {};
    PipeStreamPropInfo targetInfo = PipeStreamPropInfo();
    SourceType targetSource = SourceType::SOURCE_TYPE_VOICE_COMMUNICATION;
    uint32_t testSessionId = 1234;

    AudioInjectorPolicy::GetInstance().SetCapturePortIdx(4321);
    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->paIndex_ = UINT32_INVALID_VALUE;
    pipe1->pipeRole_ = PIPE_ROLE_INPUT;
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = 1234;
    pipe1->streamDescriptors_.push_back(desc);
    AudioPipeManager::GetPipeManager()->AddAudioPipeInfo(pipe1);
    int32_t ret = ecManager.ReloadNormalSource(sessionInfo, targetInfo, targetSource, testSessionId);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: AudioEcManager_045
* @tc.desc  : Test ReloadNormalSource interface with invalid sessionId.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_045, TestSize.Level4)
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    SessionInfo sessionInfo = {};
    PipeStreamPropInfo targetInfo = PipeStreamPropInfo();
    SourceType targetSource = SourceType::SOURCE_TYPE_VOICE_COMMUNICATION;
    uint32_t testSessionId = 1234;

    AudioInjectorPolicy::GetInstance().SetCapturePortIdx(6789);
    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->paIndex_ = 4321;
    pipe1->pipeRole_ = PIPE_ROLE_OUTPUT;
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = 1234;
    pipe1->streamDescriptors_.push_back(desc);
    AudioPipeManager::GetPipeManager()->AddAudioPipeInfo(pipe1);
    int32_t ret = ecManager.ReloadNormalSource(sessionInfo, targetInfo, targetSource, testSessionId);
    EXPECT_EQ(ret, SUCCESS);
}

} // namespace AudioStandard
} // namespace OHOS