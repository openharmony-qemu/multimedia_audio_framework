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
* @tc.name  : Test AudioEcManager.
* @tc.number: AudioEcManager_012
* @tc.desc  : Test UpdateArmModuleInfo interface.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_012, TestSize.Level1)
{
    std::string address = "00:11:22:33:44";
    DeviceRole role = INPUT_DEVICE;
    AudioModuleInfo moduleInfo;
    auto ecManager = std::make_shared<AudioEcManager>();
    ASSERT_TRUE(ecManager != nullptr);

    ecManager->UpdateArmModuleInfo(address, role, moduleInfo);
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
* @tc.number: AudioEcManager_019
* @tc.desc  : Test PrepareAndOpenNormalSource interface.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_019, TestSize.Level1)
{
    EXPECT_EQ(ParseAudioFormat("AUDIO_FORMAT_PCM_16_BIT"), "s16le");
    EXPECT_EQ(ParseAudioFormat("AUDIO_FORMAT_PCM_24_BIT"), "s24le");
    EXPECT_EQ(ParseAudioFormat("AUDIO_FORMAT_PCM_32_BIT"), "s32le");
    EXPECT_EQ(ParseAudioFormat(""), "s16le");
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: AudioEcManager_020
* @tc.desc  : Test GetUsbModuleInfo interface.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_020, TestSize.Level1)
{
    AudioModuleInfo moduleInfo;

    moduleInfo.role = "sink";
    string deviceInfo = "sink_rate:1;sink_format:AUDIO_FORMAT_PCM_16_BIT";
    GetUsbModuleInfo(deviceInfo, moduleInfo);
    EXPECT_EQ(moduleInfo.rate, "1");

    moduleInfo.channels = "2";
    GetUsbModuleInfo(deviceInfo, moduleInfo);
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
* @tc.name  : Test AudioEcManager.
* @tc.number: AudioEcManager_022
* @tc.desc  : Test ActivateArmDevice interface.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_022, TestSize.Level1)
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());

    const std::string badAddress{"bad address"};
    const std::string goodAddress{"address=card=2;device=0 role=0"};
    ecManager.isEcFeatureEnable_ = true;

    ecManager.ActivateArmDevice(badAddress, DeviceRole::INPUT_DEVICE);
    EXPECT_NE(ecManager.activeArmInputAddr_, badAddress);

    ecManager.ActivateArmDevice(badAddress, DeviceRole::OUTPUT_DEVICE);
    EXPECT_NE(ecManager.activeArmOutputAddr_, badAddress);

    std::list<AudioModuleInfo> moduleInfoList{};
    ecManager.audioConfigManager_.GetModuleListByType(ClassType::TYPE_USB, moduleInfoList);
    if (moduleInfoList.empty()) {
        AudioModuleInfo testModuleInfo1{.role = "sink", .name="m1"};
        AudioModuleInfo testModuleInfo2{.role = "source", .name="m2"};
        std::list<AudioModuleInfo> testModules{testModuleInfo1, testModuleInfo2};
        ecManager.audioConfigManager_.deviceClassInfo_.insert(
            std::make_pair(ClassType::TYPE_USB, testModules));

        ecManager.ActivateArmDevice(goodAddress, DeviceRole::OUTPUT_DEVICE);
        ecManager.ActivateArmDevice(goodAddress, DeviceRole::INPUT_DEVICE);
        ecManager.activeArmOutputAddr_ = {};
        ecManager.activeArmInputAddr_ = {};

        ecManager.PresetArmIdleInput(goodAddress);
        ecManager.isEcFeatureEnable_ = false;
        ecManager.ActivateArmDevice(goodAddress, DeviceRole::INPUT_DEVICE);
    }
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: AudioEcManager_023
* @tc.desc  : Test GetUsbModuleInfo interface.
*/
HWTEST_F(AudioEcManagerUnitTest, AudioEcManager_023, TestSize.Level4)
{
    AudioModuleInfo moduleInfo;
    moduleInfo.role = "source";
    string deviceInfo = "source_rate:1;source_format:AUDIO_FORMAT_PCM_16_BIT";
    GetUsbModuleInfo(deviceInfo, moduleInfo);
    EXPECT_EQ(moduleInfo.rate, "1");
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
    ecManager.isEcFeatureEnable_ = false;
    ecManager.usbSourceModuleInfo_.role = "";
    const std::string goodAddress{"address=card=2;device=0 role=0"};
    ecManager.PresetArmIdleInput(goodAddress);
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