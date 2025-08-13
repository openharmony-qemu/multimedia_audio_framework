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

#include "audio_policy_dump_unit_test.h"
#include "audio_errors.h"
#include "audio_policy_log.h"

#include <thread>
#include <memory>
#include <vector>
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioPolicyDumpUnitTest::SetUpTestCase(void) {}
void AudioPolicyDumpUnitTest::TearDownTestCase(void) {}
void AudioPolicyDumpUnitTest::SetUp(void) {}
void AudioPolicyDumpUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test AllDeviceVolumeInfoDump.
 * @tc.number: AudioPolicyDumpUnitTest_001
 * @tc.desc  : Test AllDeviceVolumeInfoDump interface.
 */
HWTEST_F(AudioPolicyDumpUnitTest, AudioPolicyDumpUnitTest_001, TestSize.Level4)
{
    std::string dumpString = "";
    auto audioPolicyDumpTest = std::make_shared<AudioPolicyDump>();
    EXPECT_NE(audioPolicyDumpTest, nullptr);

    audioPolicyDumpTest->audioVolumeManager_.audioConnectedDevice_.connectedDevices_.clear();
    audioPolicyDumpTest->AllDeviceVolumeInfoDump(dumpString);
    EXPECT_TRUE(dumpString.size() > 0);
    std::string TestString = "nothing Info to hidumper\n";
    EXPECT_TRUE(dumpString.find(TestString) != std::string::npos);
}

/**
 * @tc.name  : Test AllDeviceVolumeInfoDump.
 * @tc.number: AudioPolicyDumpUnitTest_002
 * @tc.desc  : Test AllDeviceVolumeInfoDump interface.
 */
HWTEST_F(AudioPolicyDumpUnitTest, AudioPolicyDumpUnitTest_002, TestSize.Level4)
{
    std::string dumpString = "";
    auto audioPolicyDumpTest = std::make_shared<AudioPolicyDump>();
    EXPECT_NE(audioPolicyDumpTest, nullptr);

    audioPolicyDumpTest->audioVolumeManager_.audioConnectedDevice_.connectedDevices_.clear();
    auto remoteDeviceDescriptorTest = std::make_shared<AudioDeviceDescriptor>();
    audioPolicyDumpTest->audioVolumeManager_.audioConnectedDevice_.AddConnectedDevice(remoteDeviceDescriptorTest);

    audioPolicyDumpTest->AllDeviceVolumeInfoDump(dumpString);
    EXPECT_TRUE(dumpString.size() > 0);
    std::string TestString = "DeviceType";
    EXPECT_FALSE(dumpString.find(TestString) != std::string::npos);
}

/**
 * @tc.name  : Test GetDumpDevices.
 * @tc.number: AudioPolicyDumpUnitTest_003
 * @tc.desc  : Test GetDumpDevices interface. hasSystemPermission = false
 */
HWTEST_F(AudioPolicyDumpUnitTest, AudioPolicyDumpUnitTest_003, TestSize.Level4)
{
    auto audioPolicyDumpTest = std::make_shared<AudioPolicyDump>();
    EXPECT_NE(audioPolicyDumpTest, nullptr);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescsTest;
    deviceDescsTest = audioPolicyDumpTest->GetDumpDevices(ALL_L_D_DEVICES_FLAG);
    EXPECT_EQ(deviceDescsTest.empty(), false);
}

/**
 * @tc.name  : Test GetDumpDevices.
 * @tc.number: AudioPolicyDumpUnitTest_004
 * @tc.desc  : Test GetDumpDevices interface. hasSystemPermission = false
 */
HWTEST_F(AudioPolicyDumpUnitTest, AudioPolicyDumpUnitTest_004, TestSize.Level4)
{
    auto audioPolicyDumpTest = std::make_shared<AudioPolicyDump>();
    EXPECT_NE(audioPolicyDumpTest, nullptr);
    audioPolicyDumpTest->audioVolumeManager_.audioConnectedDevice_.connectedDevices_.clear();

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescsTest;
    deviceDescsTest = audioPolicyDumpTest->GetDumpDevices(INPUT_DEVICES_FLAG);
    EXPECT_EQ(deviceDescsTest.empty(), true);
}

/**
 * @tc.name  : Test GetDumpDevices.
 * @tc.number: AudioPolicyDumpUnitTest_005
 * @tc.desc  : Test GetDumpDevices interface. hasSystemPermission = false
 */
HWTEST_F(AudioPolicyDumpUnitTest, AudioPolicyDumpUnitTest_005, TestSize.Level4)
{
    auto audioPolicyDumpTest = std::make_shared<AudioPolicyDump>();
    EXPECT_NE(audioPolicyDumpTest, nullptr);

    audioPolicyDumpTest->audioVolumeManager_.audioConnectedDevice_.connectedDevices_.clear();
    auto remoteDeviceDescriptorTest = std::make_shared<AudioDeviceDescriptor>();
    audioPolicyDumpTest->audioVolumeManager_.audioConnectedDevice_.AddConnectedDevice(remoteDeviceDescriptorTest);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescsTest;
    deviceDescsTest = audioPolicyDumpTest->GetDumpDevices(INPUT_DEVICES_FLAG);
    EXPECT_EQ(deviceDescsTest.empty(), true);
}

/**
 * @tc.name  : Test GetDumpDeviceInfo.
 * @tc.number: AudioPolicyDumpUnitTest_006
 * @tc.desc  : Test GetDumpDeviceInfo interface
 */
HWTEST_F(AudioPolicyDumpUnitTest, AudioPolicyDumpUnitTest_006, TestSize.Level4)
{
    auto audioPolicyDumpTest = std::make_shared<AudioPolicyDump>();
    EXPECT_NE(audioPolicyDumpTest, nullptr);

    audioPolicyDumpTest->audioVolumeManager_.audioConnectedDevice_.connectedDevices_.clear();
    auto remoteDeviceDescriptorTest = std::make_shared<AudioDeviceDescriptor>();
    audioPolicyDumpTest->audioVolumeManager_.audioConnectedDevice_.AddConnectedDevice(remoteDeviceDescriptorTest);

    std::string dumpString = "";
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescsTest;
    deviceDescsTest = audioPolicyDumpTest->GetDumpDeviceInfo(dumpString, INPUT_DEVICES_FLAG);
    EXPECT_EQ(deviceDescsTest.empty(), true);
    EXPECT_EQ(audioPolicyDumpTest->conneceType_, CONNECT_TYPE_LOCAL);
}

/**
 * @tc.name  : Test GetDumpDeviceInfo.
 * @tc.number: AudioPolicyDumpUnitTest_007
 * @tc.desc  : Test GetDumpDeviceInfo interface
 */
HWTEST_F(AudioPolicyDumpUnitTest, AudioPolicyDumpUnitTest_007, TestSize.Level4)
{
    auto audioPolicyDumpTest = std::make_shared<AudioPolicyDump>();
    EXPECT_NE(audioPolicyDumpTest, nullptr);

    audioPolicyDumpTest->audioVolumeManager_.audioConnectedDevice_.connectedDevices_.clear();
    auto remoteDeviceDescriptorTest = std::make_shared<AudioDeviceDescriptor>();
    audioPolicyDumpTest->audioVolumeManager_.audioConnectedDevice_.AddConnectedDevice(remoteDeviceDescriptorTest);

    std::string dumpString = "";
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescsTest;
    deviceDescsTest = audioPolicyDumpTest->GetDumpDeviceInfo(dumpString, OUTPUT_DEVICES_FLAG);
    EXPECT_EQ(deviceDescsTest.empty(), true);
    EXPECT_EQ(audioPolicyDumpTest->conneceType_, CONNECT_TYPE_LOCAL);
}

/**
 * @tc.name  : Test GetDumpDeviceInfo.
 * @tc.number: AudioPolicyDumpUnitTest_008
 * @tc.desc  : Test GetDumpDeviceInfo interface
 */
HWTEST_F(AudioPolicyDumpUnitTest, AudioPolicyDumpUnitTest_008, TestSize.Level4)
{
    auto audioPolicyDumpTest = std::make_shared<AudioPolicyDump>();
    EXPECT_NE(audioPolicyDumpTest, nullptr);

    audioPolicyDumpTest->audioVolumeManager_.audioConnectedDevice_.connectedDevices_.clear();
    auto remoteDeviceDescriptorTest = std::make_shared<AudioDeviceDescriptor>();
    audioPolicyDumpTest->audioVolumeManager_.audioConnectedDevice_.AddConnectedDevice(remoteDeviceDescriptorTest);

    std::string dumpString = "";
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescsTest;
    deviceDescsTest = audioPolicyDumpTest->GetDumpDeviceInfo(dumpString, ALL_DEVICES_FLAG);
    EXPECT_EQ(deviceDescsTest.empty(), false);
    std::string TestString = "connect type";
    EXPECT_TRUE(dumpString.find(TestString) != std::string::npos);
}

/**
 * @tc.name  : Test GetDumpDeviceInfo.
 * @tc.number: AudioPolicyDumpUnitTest_009
 * @tc.desc  : Test GetDumpDeviceInfo interface
 */
HWTEST_F(AudioPolicyDumpUnitTest, AudioPolicyDumpUnitTest_009, TestSize.Level4)
{
    auto audioPolicyDumpTest = std::make_shared<AudioPolicyDump>();
    EXPECT_NE(audioPolicyDumpTest, nullptr);

    audioPolicyDumpTest->audioVolumeManager_.audioConnectedDevice_.connectedDevices_.clear();
    auto remoteDeviceDescriptorTest = std::make_shared<AudioDeviceDescriptor>();
    remoteDeviceDescriptorTest->audioStreamInfo_.clear();
    DeviceStreamInfo streamInfoTest(AudioSamplingRate::SAMPLE_RATE_44100,
        AudioEncodingType::ENCODING_PCM, AudioSampleFormat::SAMPLE_S16LE, AudioChannelLayout::CH_LAYOUT_MONO);
    remoteDeviceDescriptorTest->audioStreamInfo_.push_back(streamInfoTest);

    audioPolicyDumpTest->audioVolumeManager_.audioConnectedDevice_.AddConnectedDevice(remoteDeviceDescriptorTest);

    std::string dumpString = "";
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescsTest;
    deviceDescsTest = audioPolicyDumpTest->GetDumpDeviceInfo(dumpString, ALL_DEVICES_FLAG);
    EXPECT_EQ(deviceDescsTest.empty(), false);
    std::string TestString = "device sampleRates:";
    EXPECT_TRUE(dumpString.find(TestString) != std::string::npos);
}

/**
 * @tc.name  : Test GetRingerModeDump.
 * @tc.number: AudioPolicyDumpUnitTest_010
 * @tc.desc  : Test GetRingerModeDump interface
 */
HWTEST_F(AudioPolicyDumpUnitTest, AudioPolicyDumpUnitTest_010, TestSize.Level4)
{
    std::string dumpString = "";
    auto audioPolicyDumpTest = std::make_shared<AudioPolicyDump>();
    EXPECT_NE(audioPolicyDumpTest, nullptr);
    
    audioPolicyDumpTest->audioPolicyManager_.SetRingerMode(AudioRingerMode::RINGER_MODE_SILENT);
    audioPolicyDumpTest->GetRingerModeDump(dumpString);
    EXPECT_TRUE(dumpString.size() > 0);
    std::string TestString = "SILENT";
    EXPECT_TRUE(dumpString.find(TestString) != std::string::npos);
}

/**
 * @tc.name  : Test GetRingerModeDump.
 * @tc.number: AudioPolicyDumpUnitTest_011
 * @tc.desc  : Test GetRingerModeDump interface
 */
HWTEST_F(AudioPolicyDumpUnitTest, AudioPolicyDumpUnitTest_011, TestSize.Level4)
{
    std::string dumpString = "";
    auto audioPolicyDumpTest = std::make_shared<AudioPolicyDump>();
    EXPECT_NE(audioPolicyDumpTest, nullptr);
    
    audioPolicyDumpTest->audioPolicyManager_.SetRingerMode(AudioRingerMode::RINGER_MODE_VIBRATE);
    audioPolicyDumpTest->GetRingerModeDump(dumpString);
    EXPECT_TRUE(dumpString.size() > 0);
    std::string TestString = "VIBRATE";
    EXPECT_TRUE(dumpString.find(TestString) != std::string::npos);
}

/**
 * @tc.name  : Test GetRingerModeDump.
 * @tc.number: AudioPolicyDumpUnitTest_012
 * @tc.desc  : Test GetRingerModeDump interface.
 */
HWTEST_F(AudioPolicyDumpUnitTest, AudioPolicyDumpUnitTest_012, TestSize.Level4)
{
    std::string dumpString = "";
    auto audioPolicyDumpTest = std::make_shared<AudioPolicyDump>();
    EXPECT_NE(audioPolicyDumpTest, nullptr);
    
    audioPolicyDumpTest->audioPolicyManager_.SetRingerMode(static_cast<AudioRingerMode>(-1));

    audioPolicyDumpTest->GetRingerModeDump(dumpString);
    EXPECT_TRUE(dumpString.size() > 0);
    std::string TestString = "UNKNOWN";
    EXPECT_TRUE(dumpString.find(TestString) != std::string::npos);
}

/**
 * @tc.name  : Test GetRingerModeInfoDump.
 * @tc.number: AudioPolicyDumpUnitTest_013
 * @tc.desc  : Test GetRingerModeInfoDump interface.
 */
HWTEST_F(AudioPolicyDumpUnitTest, AudioPolicyDumpUnitTest_013, TestSize.Level4)
{
    std::string dumpString = "";
    auto audioPolicyDumpTest = std::make_shared<AudioPolicyDump>();
    EXPECT_NE(audioPolicyDumpTest, nullptr);

    AudioRingerMode ringMode = AudioRingerMode::RINGER_MODE_VIBRATE;
    std::string callerName = "Test";
    std::string invocationTime ="Test";
    audioPolicyDumpTest->audioPolicyManager_.SaveRingerModeInfo(ringMode, callerName, invocationTime);

    audioPolicyDumpTest->GetRingerModeInfoDump(dumpString);
    EXPECT_TRUE(dumpString.size() > 0);
    std::string TestString = "RINGER_MODE_VIBRATE";
    EXPECT_TRUE(dumpString.find(TestString) != std::string::npos);
}

/**
 * @tc.name  : Test GetRingerModeInfoDump.
 * @tc.number: AudioPolicyDumpUnitTest_014
 * @tc.desc  : Test GetRingerModeInfoDump interface.
 */
HWTEST_F(AudioPolicyDumpUnitTest, AudioPolicyDumpUnitTest_014, TestSize.Level4)
{
    std::string dumpString = "";
    auto audioPolicyDumpTest = std::make_shared<AudioPolicyDump>();
    EXPECT_NE(audioPolicyDumpTest, nullptr);

    AudioRingerMode ringMode = AudioRingerMode::RINGER_MODE_VIBRATE;
    audioPolicyDumpTest->GetRingerModeInfoDump(dumpString);
    EXPECT_TRUE(dumpString.size() > 0);
    std::string TestString = "nothing Info to hidumper\n";
    EXPECT_FALSE(dumpString.find(TestString) != std::string::npos);
}

/**
 * @tc.name  : Test StreamVolumesDump.
 * @tc.number: AudioPolicyDumpUnitTest_015
 * @tc.desc  : Test StreamVolumesDump interface. volumeKeyRegistrations.size() == 0
 */
HWTEST_F(AudioPolicyDumpUnitTest, AudioPolicyDumpUnitTest_015, TestSize.Level4)
{
    std::string dumpString = "";
    auto audioPolicyDumpTest = std::make_shared<AudioPolicyDump>();
    EXPECT_NE(audioPolicyDumpTest, nullptr);

    audioPolicyDumpTest->audioVolumeManager_.volumeKeyRegistrations_->Clear();

    audioPolicyDumpTest->StreamVolumesDump(dumpString);
    EXPECT_TRUE(dumpString.size() > 0);
    std::string TestString = "nothing Info to hidumper";
    EXPECT_TRUE(dumpString.find(TestString) != std::string::npos);
}

/**
 * @tc.name  : Test GetVolumeConfigDump.
 * @tc.number: AudioPolicyDumpUnitTest_016
 * @tc.desc  : Test GetVolumeConfigDump interface.
 */
HWTEST_F(AudioPolicyDumpUnitTest, AudioPolicyDumpUnitTest_016, TestSize.Level4)
{
    std::string dumpString = "";
    auto audioPolicyDumpTest = std::make_shared<AudioPolicyDump>();
    EXPECT_NE(audioPolicyDumpTest, nullptr);

    AudioAdapterManager audioAdapterManager;
    audioAdapterManager.streamVolumeInfos_.clear();

    std::shared_ptr<StreamVolumeInfo> streamVolumeInfoPtr = std::make_shared<StreamVolumeInfo>();
    streamVolumeInfoPtr->streamType = AudioVolumeType::STREAM_ALL;
    audioAdapterManager.streamVolumeInfos_.insert({streamVolumeInfoPtr->streamType, streamVolumeInfoPtr});

    audioPolicyDumpTest->audioPolicyManager_ = audioAdapterManager;
    audioPolicyDumpTest->GetVolumeConfigDump(dumpString);
    EXPECT_TRUE(dumpString.size() > 0);
    std::string TestString = "GetStreamMute of STREAM_ALL for streamType";
    EXPECT_FALSE(dumpString.find(TestString) != std::string::npos);
}

/**
 * @tc.name  : Test GetAdjustVolumeDump.
 * @tc.number: AudioPolicyDumpUnitTest_017
 * @tc.desc  : Test GetAdjustVolumeDump interface.
 */
HWTEST_F(AudioPolicyDumpUnitTest, AudioPolicyDumpUnitTest_017, TestSize.Level4)
{
    std::string dumpString = "";
    auto audioPolicyDumpTest = std::make_shared<AudioPolicyDump>();
    EXPECT_NE(audioPolicyDumpTest, nullptr);

    audioPolicyDumpTest->audioVolumeManager_.systemVolumeLevelInfo_->Clear();

    AudioStreamType streamType = STREAM_MEDIA;
    int32_t volumeLevel = 0;
    int32_t appUid = 0;
    std::string invocationTime = "";
    audioPolicyDumpTest->audioVolumeManager_.SaveSystemVolumeLevelInfo(streamType, volumeLevel, appUid, invocationTime);

    audioPolicyDumpTest->GetAdjustVolumeDump(dumpString);
    EXPECT_TRUE(dumpString.size() > 0);
    std::string TestString = "DeviceType";
    EXPECT_TRUE(dumpString.find(TestString) != std::string::npos);
}

/**
 * @tc.name  : Test AdjustVolumeAppend.
 * @tc.number: AudioPolicyDumpUnitTest_018
 * @tc.desc  : Test AdjustVolumeAppend interface.
 */
HWTEST_F(AudioPolicyDumpUnitTest, AudioPolicyDumpUnitTest_018, TestSize.Level4)
{
    std::string dumpString = "";
    auto audioPolicyDumpTest = std::make_shared<AudioPolicyDump>();
    EXPECT_NE(audioPolicyDumpTest, nullptr);

    std::vector<AdjustStreamVolumeInfo> adjustInfo;
    AdjustStreamVolumeInfo adjustStreamVolumeInfoTest;
    adjustInfo.push_back(adjustStreamVolumeInfoTest);

    audioPolicyDumpTest->AdjustVolumeAppend(adjustInfo, dumpString);
    EXPECT_TRUE(dumpString.size() > 0);
    std::string TestString = "VolumeValue";
    EXPECT_TRUE(dumpString.find(TestString) != std::string::npos);
}

/**
 * @tc.name  : Test AdjustVolumeAppend.
 * @tc.number: AudioPolicyDumpUnitTest_019
 * @tc.desc  : Test AdjustVolumeAppend interface.
 */
HWTEST_F(AudioPolicyDumpUnitTest, AudioPolicyDumpUnitTest_019, TestSize.Level4)
{
    std::string dumpString = "";
    auto audioPolicyDumpTest = std::make_shared<AudioPolicyDump>();
    EXPECT_NE(audioPolicyDumpTest, nullptr);

    std::vector<AdjustStreamVolumeInfo> adjustInfo;
    audioPolicyDumpTest->AdjustVolumeAppend(adjustInfo, dumpString);
    EXPECT_TRUE(dumpString.size() > 0);
    std::string TestString = "nothing Info to hidumper";
    EXPECT_TRUE(dumpString.find(TestString) != std::string::npos);
}

/**
 * @tc.name  : Test AudioStreamDump.
 * @tc.number: AudioPolicyDumpUnitTest_020
 * @tc.desc  : Test AudioStreamDump interface.
 */
HWTEST_F(AudioPolicyDumpUnitTest, AudioPolicyDumpUnitTest_020, TestSize.Level4)
{
    std::string dumpString = "";
    auto audioPolicyDumpTest = std::make_shared<AudioPolicyDump>();
    EXPECT_NE(audioPolicyDumpTest, nullptr);

    audioPolicyDumpTest->streamCollector_.audioRendererChangeInfos_.clear();

    audioPolicyDumpTest->AudioStreamDump(dumpString);
    EXPECT_TRUE(dumpString.size() > 0);
    std::string TestString = "audiorenderer stream size";
    EXPECT_TRUE(dumpString.find(TestString) != std::string::npos);
}

/**
 * @tc.name  : Test AudioStreamDump.
 * @tc.number: AudioPolicyDumpUnitTest_021
 * @tc.desc  : Test AudioStreamDump interface.
 */
HWTEST_F(AudioPolicyDumpUnitTest, AudioPolicyDumpUnitTest_021, TestSize.Level4)
{
    std::string dumpString = "";
    auto audioPolicyDumpTest = std::make_shared<AudioPolicyDump>();
    EXPECT_NE(audioPolicyDumpTest, nullptr);

    audioPolicyDumpTest->streamCollector_.audioRendererChangeInfos_.clear();
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    EXPECT_NE(rendererChangeInfo, nullptr);
    
    rendererChangeInfo->rendererInfo.rendererFlags = 0;
    audioPolicyDumpTest->streamCollector_.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));

    audioPolicyDumpTest->AudioStreamDump(dumpString);
    EXPECT_TRUE(dumpString.size() > 0);
    std::string TestString = "normal AudioCapturer stream";
    EXPECT_TRUE(dumpString.find(TestString) != std::string::npos);
}

/**
 * @tc.name  : Test AudioStreamDump.
 * @tc.number: AudioPolicyDumpUnitTest_022
 * @tc.desc  : Test AudioStreamDump interface.
 */
HWTEST_F(AudioPolicyDumpUnitTest, AudioPolicyDumpUnitTest_022, TestSize.Level4)
{
    std::string dumpString = "";
    auto audioPolicyDumpTest = std::make_shared<AudioPolicyDump>();
    EXPECT_NE(audioPolicyDumpTest, nullptr);

    audioPolicyDumpTest->streamCollector_.audioRendererChangeInfos_.clear();
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    EXPECT_NE(rendererChangeInfo, nullptr);
    
    rendererChangeInfo->rendererInfo.rendererFlags = 1;
    audioPolicyDumpTest->streamCollector_.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));

    audioPolicyDumpTest->AudioStreamDump(dumpString);
    EXPECT_TRUE(dumpString.size() > 0);
    std::string TestString = "fast AudioCapturer stream";
    EXPECT_TRUE(dumpString.find(TestString) != std::string::npos);
}

/**
 * @tc.name  : Test AudioStreamDump.
 * @tc.number: AudioPolicyDumpUnitTest_023
 * @tc.desc  : Test AudioStreamDump interface.
 */
HWTEST_F(AudioPolicyDumpUnitTest, AudioPolicyDumpUnitTest_023, TestSize.Level4)
{
    std::string dumpString = "";
    auto audioPolicyDumpTest = std::make_shared<AudioPolicyDump>();
    EXPECT_NE(audioPolicyDumpTest, nullptr);

    audioPolicyDumpTest->streamCollector_.audioRendererChangeInfos_.clear();
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    EXPECT_NE(rendererChangeInfo, nullptr);
    
    rendererChangeInfo->rendererInfo.rendererFlags = 2;
    audioPolicyDumpTest->streamCollector_.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));

    audioPolicyDumpTest->AudioStreamDump(dumpString);
    EXPECT_TRUE(dumpString.size() > 0);
    std::string TestString = "rendererStatus";
    EXPECT_TRUE(dumpString.find(TestString) != std::string::npos);
}

/**
 * @tc.name  : Test GetCapturerStreamDump.
 * @tc.number: AudioPolicyDumpUnitTest_024
 * @tc.desc  : Test GetCapturerStreamDump interface.
 */
HWTEST_F(AudioPolicyDumpUnitTest, AudioPolicyDumpUnitTest_024, TestSize.Level4)
{
    std::string dumpString = "";
    auto audioPolicyDumpTest = std::make_shared<AudioPolicyDump>();
    EXPECT_NE(audioPolicyDumpTest, nullptr);

    audioPolicyDumpTest->streamCollector_.audioCapturerChangeInfos_.clear();
    std::shared_ptr<AudioCapturerChangeInfo> capturerChangeInfos_ = make_shared<AudioCapturerChangeInfo>();
    EXPECT_NE(capturerChangeInfos_, nullptr);
    
    capturerChangeInfos_->clientUID = 0;
    capturerChangeInfos_->capturerInfo.capturerFlags = 0;
    audioPolicyDumpTest->streamCollector_.audioCapturerChangeInfos_.push_back(move(capturerChangeInfos_));

    audioPolicyDumpTest->GetCapturerStreamDump(dumpString);
    EXPECT_TRUE(dumpString.size() > 0);
    std::string TestString = "normal AudioCapturer stream";
    EXPECT_TRUE(dumpString.find(TestString) != std::string::npos);
}

/**
 * @tc.name  : Test GetCapturerStreamDump.
 * @tc.number: AudioPolicyDumpUnitTest_025
 * @tc.desc  : Test GetCapturerStreamDump interface.
 */
HWTEST_F(AudioPolicyDumpUnitTest, AudioPolicyDumpUnitTest_025, TestSize.Level4)
{
    std::string dumpString = "";
    auto audioPolicyDumpTest = std::make_shared<AudioPolicyDump>();
    EXPECT_NE(audioPolicyDumpTest, nullptr);

    audioPolicyDumpTest->streamCollector_.audioCapturerChangeInfos_.clear();
    std::shared_ptr<AudioCapturerChangeInfo> capturerChangeInfos_ = make_shared<AudioCapturerChangeInfo>();
    EXPECT_NE(capturerChangeInfos_, nullptr);
    
    capturerChangeInfos_->clientUID = 0;
    capturerChangeInfos_->capturerInfo.capturerFlags = 1;
    audioPolicyDumpTest->streamCollector_.audioCapturerChangeInfos_.push_back(move(capturerChangeInfos_));

    audioPolicyDumpTest->GetCapturerStreamDump(dumpString);
    EXPECT_TRUE(dumpString.size() > 0);
    std::string TestString = "fast AudioCapturer stream";
    EXPECT_TRUE(dumpString.find(TestString) != std::string::npos);
}

/**
 * @tc.name  : Test GetCapturerStreamDump.
 * @tc.number: AudioPolicyDumpUnitTest_026
 * @tc.desc  : Test GetCapturerStreamDump interface.
 */
HWTEST_F(AudioPolicyDumpUnitTest, AudioPolicyDumpUnitTest_026, TestSize.Level4)
{
    std::string dumpString = "";
    auto audioPolicyDumpTest = std::make_shared<AudioPolicyDump>();
    EXPECT_NE(audioPolicyDumpTest, nullptr);

    audioPolicyDumpTest->streamCollector_.audioCapturerChangeInfos_.clear();

    audioPolicyDumpTest->GetCapturerStreamDump(dumpString);
    EXPECT_TRUE(dumpString.size() > 0);
    std::string TestString = "audiocapturer stream size";
    EXPECT_TRUE(dumpString.find(TestString) != std::string::npos);
}

/**
 * @tc.name  : Test GetCapturerStreamDump.
 * @tc.number: AudioPolicyDumpUnitTest_027
 * @tc.desc  : Test GetCapturerStreamDump interface.
 */
HWTEST_F(AudioPolicyDumpUnitTest, AudioPolicyDumpUnitTest_027, TestSize.Level4)
{
    std::string dumpString = "";
    auto audioPolicyDumpTest = std::make_shared<AudioPolicyDump>();
    EXPECT_NE(audioPolicyDumpTest, nullptr);

    audioPolicyDumpTest->streamCollector_.audioCapturerChangeInfos_.clear();
    std::shared_ptr<AudioCapturerChangeInfo> capturerChangeInfos_ = make_shared<AudioCapturerChangeInfo>();
    EXPECT_NE(capturerChangeInfos_, nullptr);
    
    capturerChangeInfos_->clientUID = 0;
    capturerChangeInfos_->capturerInfo.capturerFlags = 2;
    audioPolicyDumpTest->streamCollector_.audioCapturerChangeInfos_.push_back(move(capturerChangeInfos_));

    audioPolicyDumpTest->GetCapturerStreamDump(dumpString);
    EXPECT_TRUE(dumpString.size() > 0);
    std::string TestString = "capturerState";
    EXPECT_TRUE(dumpString.find(TestString) != std::string::npos);
}

/**
 * @tc.name  : Test GetRingerModeType.
 * @tc.number: AudioPolicyDumpUnitTest_028
 * @tc.desc  : Test GetRingerModeType interface.
 */
HWTEST_F(AudioPolicyDumpUnitTest, AudioPolicyDumpUnitTest_028, TestSize.Level4)
{
    auto audioPolicyDumpTest = std::make_shared<AudioPolicyDump>();
    EXPECT_NE(audioPolicyDumpTest, nullptr);

    std::string ret = "Test";
    ret = audioPolicyDumpTest->GetRingerModeType(static_cast<AudioRingerMode>(-1));
    EXPECT_EQ(ret, "UNKNOWMTYPE");
}

/**
 * @tc.name  : Test GetRingerModeType.
 * @tc.number: AudioPolicyDumpUnitTest_029
 * @tc.desc  : Test GetRingerModeType interface.
 */
HWTEST_F(AudioPolicyDumpUnitTest, AudioPolicyDumpUnitTest_029, TestSize.Level4)
{
    auto audioPolicyDumpTest = std::make_shared<AudioPolicyDump>();
    EXPECT_NE(audioPolicyDumpTest, nullptr);

    std::string ret = "Test";
    ret = audioPolicyDumpTest->GetRingerModeType(RINGER_MODE_SILENT);
    EXPECT_EQ(ret, "RINGER_MODE_SILENT");
}

/**
 * @tc.name  : Test GetRingerModeType.
 * @tc.number: AudioPolicyDumpUnitTest_030
 * @tc.desc  : Test GetRingerModeType interface.
 */
HWTEST_F(AudioPolicyDumpUnitTest, AudioPolicyDumpUnitTest_030, TestSize.Level4)
{
    auto audioPolicyDumpTest = std::make_shared<AudioPolicyDump>();
    EXPECT_NE(audioPolicyDumpTest, nullptr);

    std::string ret = "Test";
    ret = audioPolicyDumpTest->GetRingerModeType(RINGER_MODE_VIBRATE);
    EXPECT_EQ(ret, "RINGER_MODE_VIBRATE");
}

/**
 * @tc.name  : Test GetRingerModeType.
 * @tc.number: AudioPolicyDumpUnitTest_031
 * @tc.desc  : Test GetRingerModeType interface.
 */
HWTEST_F(AudioPolicyDumpUnitTest, AudioPolicyDumpUnitTest_031, TestSize.Level4)
{
    auto audioPolicyDumpTest = std::make_shared<AudioPolicyDump>();
    EXPECT_NE(audioPolicyDumpTest, nullptr);

    std::string ret = "Test";
    ret = audioPolicyDumpTest->GetRingerModeType(RINGER_MODE_NORMAL);
    EXPECT_EQ(ret, "RINGER_MODE_NORMAL");
}

/**
* @tc.name  : Test AudioPolicyDumpUnitTest.
* @tc.number: AudioPolicyDumpUnitTest_001
* @tc.desc  : Test AllDeviceVolumeInfoDump interface.
*/
HWTEST_F(AudioPolicyDumpUnitTest, AudioPolicyDumpUnitTest_032, TestSize.Level4)
{
    auto audioPolicyDump = AudioPolicyDump::GetInstance();
    std::string dumpString = "";
    audioPolicyDump.AllDeviceVolumeInfoDump(dumpString);
    EXPECT_NE(0, dumpString.size());
}

/**
* @tc.name  : Test AudioPolicyDumpUnitTest.
* @tc.number: AudioPolicyDumpUnitTest_002
* @tc.desc  : Test GetRingerModeDump interface.
*/
HWTEST_F(AudioPolicyDumpUnitTest, AudioPolicyDumpUnitTest_033, TestSize.Level4)
{
    auto audioPolicyDump = AudioPolicyDump::GetInstance();
    std::string dumpString = "";
    AudioPolicyManagerFactory::GetAudioPolicyManager().SetRingerMode(RINGER_MODE_SILENT);
    audioPolicyDump.GetRingerModeDump(dumpString);
    AudioPolicyManagerFactory::GetAudioPolicyManager().SetRingerMode(RINGER_MODE_VIBRATE);
    audioPolicyDump.GetRingerModeDump(dumpString);
    AudioPolicyManagerFactory::GetAudioPolicyManager().SetRingerMode(static_cast<AudioRingerMode>(3));
    audioPolicyDump.GetRingerModeDump(dumpString);
    EXPECT_NE(0, dumpString.size());
}

/**
* @tc.name  : Test AudioPolicyDumpUnitTest.
* @tc.number: AudioPolicyDumpUnitTest_003
* @tc.desc  : Test GetRingerModeInfoDump interface.
*/
HWTEST_F(AudioPolicyDumpUnitTest, AudioPolicyDumpUnitTest_034, TestSize.Level4)
{
    auto audioPolicyDump = AudioPolicyDump::GetInstance();
    std::string dumpString = "";
    audioPolicyDump.GetRingerModeInfoDump(dumpString);
    EXPECT_NE(0, dumpString.size());
}


/**
* @tc.name  : Test AudioPolicyDumpUnitTest.
* @tc.number: AudioPolicyDumpUnitTest_004
* @tc.desc  : Test GetRingerModeType interface.
*/
HWTEST_F(AudioPolicyDumpUnitTest, AudioPolicyDumpUnitTest_035, TestSize.Level4)
{
    auto audioPolicyDump = AudioPolicyDump::GetInstance();
    AudioRingerMode ringerMode = RINGER_MODE_SILENT;
    auto ret = audioPolicyDump.GetRingerModeType(ringerMode);
    EXPECT_NE(0, ret.size());
    ringerMode = RINGER_MODE_VIBRATE;
    ret = audioPolicyDump.GetRingerModeType(ringerMode);
    EXPECT_NE(0, ret.size());
    ringerMode = RINGER_MODE_NORMAL;
    ret = audioPolicyDump.GetRingerModeType(ringerMode);
    EXPECT_NE(0, ret.size());
    ringerMode = static_cast<AudioRingerMode>(3);
    ret = audioPolicyDump.GetRingerModeType(ringerMode);
    EXPECT_NE(0, ret.size());
}
} // namespace AudioStandard
} // namespace OHOS