/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "audio_device_descriptor.h"
#include "audio_device_manager_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioDeviceManagerUnitTest::SetUpTestCase(void) {}
void AudioDeviceManagerUnitTest::TearDownTestCase(void) {}
void AudioDeviceManagerUnitTest::SetUp(void) {}
void AudioDeviceManagerUnitTest::TearDown(void) {}

/**
* @tc.name  : Test AudioDeviceManager.
* @tc.number: AudioDeviceManagerUnitTest_001.
* @tc.desc  : Test GetActiveScoDevice.
*/
HWTEST_F(AudioDeviceManagerUnitTest, AudioDeviceManagerUnitTest_001, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_SCO, DeviceRole::OUTPUT_DEVICE);
    desc->macAddress_ = "00:11:22:33:44:55";
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc);
    std::string scoMac = "00:11:22:33:44:55";
    auto result = AudioDeviceManager::GetAudioDeviceManager().GetActiveScoDevice(scoMac,
        DeviceRole::OUTPUT_DEVICE);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DeviceType::DEVICE_TYPE_BLUETOOTH_SCO);
    EXPECT_EQ(result->macAddress_, scoMac);
    EXPECT_EQ(result->deviceRole_, DeviceRole::OUTPUT_DEVICE);
}

/**
* @tc.name  : Test AudioDeviceManager.
* @tc.number: AudioDeviceManagerUnitTest_002.
* @tc.desc  : Test GetActiveScoDevice.
*/
HWTEST_F(AudioDeviceManagerUnitTest, AudioDeviceManagerUnitTest_002, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_SCO, DeviceRole::OUTPUT_DEVICE);
    desc->macAddress_ = "00:11:22:33:44:55";
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc);
    std::string scoMac = "00:11:22:33:44:66";
    auto result = AudioDeviceManager::GetAudioDeviceManager().GetActiveScoDevice(scoMac,
        DeviceRole::OUTPUT_DEVICE);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->macAddress_, "");
}

/**
* @tc.name  : Test AudioDeviceManager.
* @tc.number: AudioDeviceManagerUnitTest_003.
* @tc.desc  : Test GetActiveScoDevice.
*/
HWTEST_F(AudioDeviceManagerUnitTest, AudioDeviceManagerUnitTest_003, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_SCO, DeviceRole::OUTPUT_DEVICE);
    desc->macAddress_ = "00:11:22:33:44:55";
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc);
    std::string scoMac = "00:11:22:33:44:55";
    auto result = AudioDeviceManager::GetAudioDeviceManager().GetActiveScoDevice(scoMac,
        DeviceRole::INPUT_DEVICE);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->macAddress_, "");
}

/**
* @tc.name  : Test AudioDeviceManager.
* @tc.number: AudioDeviceManagerUnitTest_004.
* @tc.desc  : Test GetActiveScoDevice.
*/
HWTEST_F(AudioDeviceManagerUnitTest, AudioDeviceManagerUnitTest_004, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, DeviceRole::OUTPUT_DEVICE);
    desc->macAddress_ = "00:11:22:33:44:99";
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc);
    std::string scoMac = "00:11:22:33:44:99";
    auto result = AudioDeviceManager::GetAudioDeviceManager().GetActiveScoDevice(scoMac,
        DeviceRole::OUTPUT_DEVICE);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->macAddress_, "");
}

/**
* @tc.name  : Test AudioDeviceManager.
* @tc.number: AudioDeviceManagerUnitTest_005.
* @tc.desc  : Test GetActiveScoDevice.
*/
HWTEST_F(AudioDeviceManagerUnitTest, AudioDeviceManagerUnitTest_005, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, DeviceRole::OUTPUT_DEVICE);
    desc->macAddress_ = "00:11:22:33:44:55";
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc);
    std::string scoMac = "00:11:22:33:44:55";
    auto result = AudioDeviceManager::GetAudioDeviceManager().GetActiveScoDevice(scoMac,
        DeviceRole::INPUT_DEVICE);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->macAddress_, "");
}

/**
* @tc.name  : Test AudioDeviceManager.
* @tc.number: AudioDeviceManagerUnitTest_006.
* @tc.desc  : Test GetActiveScoDevice.
*/
HWTEST_F(AudioDeviceManagerUnitTest, AudioDeviceManagerUnitTest_006, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_SCO, DeviceRole::OUTPUT_DEVICE);
    desc->macAddress_ = "00:11:22:33:44:55";
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc);
    std::string scoMac = "00:11:22:33:44:66";
    auto result = AudioDeviceManager::GetAudioDeviceManager().GetActiveScoDevice(scoMac,
        DeviceRole::INPUT_DEVICE);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->macAddress_, "");
}

/**
* @tc.name  : Test AudioDeviceManager.
* @tc.number: AudioDeviceManagerUnitTest_007.
* @tc.desc  : Test GetActiveScoDevice.
*/
HWTEST_F(AudioDeviceManagerUnitTest, AudioDeviceManagerUnitTest_007, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, DeviceRole::OUTPUT_DEVICE);
    desc->macAddress_ = "00:11:22:33:44:55";
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc);
    std::string scoMac = "00:11:22:33:44:66";
    auto result = AudioDeviceManager::GetAudioDeviceManager().GetActiveScoDevice(scoMac,
        DeviceRole::OUTPUT_DEVICE);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->macAddress_, "");
}

/**
* @tc.name  : Test AudioDeviceManager.
* @tc.number: AudioDeviceManagerUnitTest_008.
* @tc.desc  : Test GetActiveScoDevice.
*/
HWTEST_F(AudioDeviceManagerUnitTest, AudioDeviceManagerUnitTest_008, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, DeviceRole::OUTPUT_DEVICE);
    desc->macAddress_ = "00:11:22:33:44:55";
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc);
    std::string scoMac = "00:11:22:33:44:66";
    auto result = AudioDeviceManager::GetAudioDeviceManager().GetActiveScoDevice(scoMac,
        DeviceRole::INPUT_DEVICE);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->macAddress_, "");
}

/**
* @tc.name  : Test AudioDeviceManager.
* @tc.number: MakePairedDeviceDescriptor_001.
* @tc.desc  : Test MakePairedDeviceDescriptor.
*/
HWTEST_F(AudioDeviceManagerUnitTest, MakePairedDeviceDescriptor_001, TestSize.Level4)
{
    std::shared_ptr<AudioDeviceDescriptor> outDesc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_NEARLINK, DeviceRole::OUTPUT_DEVICE);
    outDesc->macAddress_ = "00:11:22:33:44:55";
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(outDesc);
    auto inDesc = std::make_shared<AudioDeviceDescriptor>(outDesc);
    inDesc->deviceRole_ = DeviceRole::INPUT_DEVICE;
    inDesc->deviceType_ = DeviceType::DEVICE_TYPE_NEARLINK_IN;
    AudioDeviceManager::GetAudioDeviceManager().MakePairedDeviceDescriptor(inDesc);
    EXPECT_EQ(outDesc->pairDeviceDescriptor_ != nullptr, true);
    AudioDeviceManager::GetAudioDeviceManager().RemoveConnectedDevices(outDesc);
}

/**
* @tc.name  : Test AudioDeviceManager.
* @tc.number: MakePairedDeviceDescriptor_002.
* @tc.desc  : Test MakePairedDeviceDescriptor.
*/
HWTEST_F(AudioDeviceManagerUnitTest, MakePairedDeviceDescriptor_002, TestSize.Level4)
{
    std::shared_ptr<AudioDeviceDescriptor> outDesc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, DeviceRole::OUTPUT_DEVICE);
    outDesc->macAddress_ = "00:11:22:33:44:55";
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(outDesc);
    auto inDesc = std::make_shared<AudioDeviceDescriptor>(outDesc);
    inDesc->deviceRole_ = DeviceRole::INPUT_DEVICE;
    inDesc->deviceType_ = DeviceType::DEVICE_TYPE_NEARLINK_IN;
    AudioDeviceManager::GetAudioDeviceManager().MakePairedDeviceDescriptor(inDesc);
    EXPECT_EQ(outDesc->pairDeviceDescriptor_ != nullptr, false);
    AudioDeviceManager::GetAudioDeviceManager().RemoveConnectedDevices(outDesc);
}
} // namespace AudioStandard
} // namespace OHOS