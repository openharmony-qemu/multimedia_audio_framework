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
    StreamPropInfo outModuleInfo;
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    std::string sRet;

    outModuleInfo.sampleRate_ = 41000;
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
    StreamPropInfo outModuleInfo;
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    std::string sRet;

    outModuleInfo.channelLayout_ = 3;
    sRet = ecManager.GetEcChannels(halName, outModuleInfo);
    EXPECT_EQ(sRet, "3");

    ecManager.dpSinkModuleInfo_.channels = "4";
    sRet = ecManager.GetEcChannels(halName, outModuleInfo);
    EXPECT_EQ(sRet, "4");

    halName = USB_CLASS;
    sRet = ecManager.GetEcChannels(halName, outModuleInfo);
    EXPECT_EQ(sRet, "3");

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
    StreamPropInfo outModuleInfo;
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    std::string sRet;

    outModuleInfo.format_ = "3";
    sRet = ecManager.GetEcFormat(halName, outModuleInfo);
    EXPECT_EQ(sRet, "3");

    ecManager.dpSinkModuleInfo_.format = "4";
    sRet = ecManager.GetEcFormat(halName, outModuleInfo);
    EXPECT_EQ(sRet, "4");

    halName = USB_CLASS;
    sRet = ecManager.GetEcFormat(halName, outModuleInfo);
    EXPECT_EQ(sRet, "3");

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
    PipeInfo pipeInfo;
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
* @tc.desc  : Test GetAudioEcInfo interface.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_011, TestSize.Level1)
{
    AudioEcInfo ecInfo;
    AudioEcManager& ecManager(AudioEcManager::GetInstance());

    ecManager.audioEcInfo_.channels = "3";
    ecInfo = ecManager.GetAudioEcInfo();
    EXPECT_EQ(ecInfo.channels, "3");
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: AudioEcManager_012
* @tc.desc  : Test UpdateArmModuleInfo interface.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_012, TestSize.Level1)
{
    std::string address = "00:11:22:33:44";
    DeviceRole role = INPUT_DEVICE;
    AudioModuleInfo moduleInfo;
    AudioEcManager& ecManager(AudioEcManager::GetInstance());

    ecManager.UpdateArmModuleInfo(address, role, moduleInfo);
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: AudioEcManager_013
* @tc.desc  : Test ReloadSourceForSession interface.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_013, TestSize.Level1)
{
    SessionInfo sessionInfo;
    AudioEcManager& ecManager(AudioEcManager::GetInstance());

    sessionInfo.sourceType = SOURCE_TYPE_INVALID;
    ecManager.ReloadSourceForSession(sessionInfo);
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
    AudioEcManager& ecManager(AudioEcManager::GetInstance());

    ecManager.UpdateStreamEcAndMicRefInfo(moduleInfo, sourceType);
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
} // namespace AudioStandard
} // namespace OHOS