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
#include "audio_policy_utils.h"

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

/**
* @tc.name  : Test AudioDeviceManager.
* @tc.number: UpdateExistDeviceDescriptor.
* @tc.desc  : Test UpdateExistDeviceDescriptor.
*/
HWTEST_F(AudioDeviceManagerUnitTest, AudioDeviceManagerUnitTest_011, TestSize.Level4)
{
    std::shared_ptr<AudioDeviceDescriptor> desc1 = std::make_shared<AudioDeviceDescriptor>();
    AudioDeviceManager &adm = AudioDeviceManager::GetAudioDeviceManager();
    adm.AddConnectedDevices(desc1);
    std::shared_ptr<AudioDeviceDescriptor> desc2 = std::make_shared<AudioDeviceDescriptor>();
    EXPECT_EQ(adm.UpdateExistDeviceDescriptor(desc2), true);

    desc1->deviceType_ = DEVICE_TYPE_USB_HEADSET;
    desc2->deviceType_ = DEVICE_TYPE_USB_HEADSET;
    EXPECT_EQ(adm.UpdateExistDeviceDescriptor(desc2), true);
    desc1->deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;
    desc2->deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;
    EXPECT_EQ(adm.UpdateExistDeviceDescriptor(desc2), true);
    desc1->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    desc2->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    EXPECT_EQ(adm.UpdateExistDeviceDescriptor(desc2), true);
    desc1->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    desc2->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    EXPECT_EQ(adm.UpdateExistDeviceDescriptor(desc2), true);
    desc1->deviceType_ = DEVICE_TYPE_NEARLINK;
    desc2->deviceType_ = DEVICE_TYPE_NEARLINK;
    EXPECT_EQ(adm.UpdateExistDeviceDescriptor(desc2), true);
    desc1->deviceType_ = DEVICE_TYPE_NEARLINK_IN;
    desc2->deviceType_ = DEVICE_TYPE_NEARLINK_IN;
    EXPECT_EQ(adm.UpdateExistDeviceDescriptor(desc2), true);
    desc1->deviceType_ = DEVICE_TYPE_REMOTE_DAUDIO;
    desc2->deviceType_ = DEVICE_TYPE_REMOTE_DAUDIO;
    EXPECT_EQ(adm.UpdateExistDeviceDescriptor(desc2), true);
}

/**
* @tc.name  : Test AudioDeviceManager.
* @tc.number: MakePairedDPDeviceDescriptor.
* @tc.desc  : Test MakePairedDPDeviceDescriptor.
*/
HWTEST_F(AudioDeviceManagerUnitTest, AudioDeviceManagerUnitTest_012, TestSize.Level1)
{
    AudioDeviceManager &audioDeviceManager = AudioDeviceManager::GetAudioDeviceManager();
    std::shared_ptr<AudioDeviceDescriptor> desc1 = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_MIC, DeviceRole::INPUT_DEVICE);
    desc1->networkId_ = "444455556666abcdef";
    audioDeviceManager.AddConnectedDevices(desc1);

    std::shared_ptr<AudioDeviceDescriptor> desc2 = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_DP, DeviceRole::OUTPUT_DEVICE);
    desc2->networkId_ = "444455556666abcdef";

    audioDeviceManager.MakePairedDPDeviceDescriptor(desc2, DeviceRole::OUTPUT_DEVICE);
    EXPECT_NE(desc2->pairDeviceDescriptor_, desc1);
    audioDeviceManager.MakePairedDPDeviceDescriptor(desc2, DeviceRole::INPUT_DEVICE);
    EXPECT_EQ(desc2->pairDeviceDescriptor_, desc1);
    audioDeviceManager.RemoveConnectedDevices(desc1);
}

/**
* @tc.name  : Test AudioDeviceManager.
* @tc.number: MakePairedDPDeviceDescriptor.
* @tc.desc  : Test MakePairedDPDeviceDescriptor.
*/
HWTEST_F(AudioDeviceManagerUnitTest, AudioDeviceManagerUnitTest_013, TestSize.Level1)
{
    AudioDeviceManager &audioDeviceManager = AudioDeviceManager::GetAudioDeviceManager();
    std::shared_ptr<AudioDeviceDescriptor> desc1 = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_DP, DeviceRole::OUTPUT_DEVICE);
    desc1->networkId_ = "444455556666abcdef";
    audioDeviceManager.AddConnectedDevices(desc1);

    audioDeviceManager.MakePairedDPDeviceDescriptor(desc1, DeviceRole::INPUT_DEVICE);
    EXPECT_NE(desc1->pairDeviceDescriptor_, desc1);
    audioDeviceManager.RemoveConnectedDevices(desc1);
}

/**
* @tc.name  : Test AudioDeviceManager.
* @tc.number: MakePairedDPDeviceDescriptor.
* @tc.desc  : Test MakePairedDPDeviceDescriptor.
*/
HWTEST_F(AudioDeviceManagerUnitTest, AudioDeviceManagerUnitTest_014, TestSize.Level1)
{
    AudioDeviceManager &audioDeviceManager = AudioDeviceManager::GetAudioDeviceManager();
    std::shared_ptr<AudioDeviceDescriptor> desc1 = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_MIC, DeviceRole::INPUT_DEVICE);
    desc1->networkId_ = "444455556666123456";
    audioDeviceManager.AddConnectedDevices(desc1);

    std::shared_ptr<AudioDeviceDescriptor> desc2 = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_DP, DeviceRole::OUTPUT_DEVICE);
    desc2->networkId_ = "444455556666abcdef";

    audioDeviceManager.MakePairedDPDeviceDescriptor(desc2, DeviceRole::INPUT_DEVICE);
    EXPECT_NE(desc2->pairDeviceDescriptor_, desc1);
    audioDeviceManager.RemoveConnectedDevices(desc1);
}

/**
* @tc.name  : Test AudioDeviceManager.
* @tc.number: MakePairedDPDeviceDescriptor.
* @tc.desc  : Test MakePairedDPDeviceDescriptor.
*/
HWTEST_F(AudioDeviceManagerUnitTest, AudioDeviceManagerUnitTest_015, TestSize.Level1)
{
    AudioDeviceManager &audioDeviceManager = AudioDeviceManager::GetAudioDeviceManager();
    std::shared_ptr<AudioDeviceDescriptor> desc1 = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_MIC, DeviceRole::INPUT_DEVICE);
    desc1->networkId_ = "444455556666abcdef";
    
    std::shared_ptr<AudioDeviceDescriptor> desc2 = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_DP, DeviceRole::OUTPUT_DEVICE);
    desc2->networkId_ = "444455556666abcdef";
    audioDeviceManager.AddConnectedDevices(desc2);

    audioDeviceManager.MakePairedDPDeviceDescriptor(desc1, DeviceRole::INPUT_DEVICE);
    EXPECT_NE(desc1->pairDeviceDescriptor_, desc2);
    audioDeviceManager.MakePairedDPDeviceDescriptor(desc1, DeviceRole::OUTPUT_DEVICE);
    EXPECT_EQ(desc1->pairDeviceDescriptor_, desc2);
    audioDeviceManager.RemoveConnectedDevices(desc2);
}

/**
* @tc.name  : Test AudioDeviceManager.
* @tc.number: MakePairedDPDeviceDescriptor.
* @tc.desc  : Test MakePairedDPDeviceDescriptor.
*/
HWTEST_F(AudioDeviceManagerUnitTest, AudioDeviceManagerUnitTest_016, TestSize.Level1)
{
    AudioDeviceManager &audioDeviceManager = AudioDeviceManager::GetAudioDeviceManager();
    std::shared_ptr<AudioDeviceDescriptor> desc1 = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_MIC, DeviceRole::OUTPUT_DEVICE);
    desc1->networkId_ = "444455556666abcdef";
    audioDeviceManager.AddConnectedDevices(desc1);


    audioDeviceManager.MakePairedDPDeviceDescriptor(desc1, DeviceRole::OUTPUT_DEVICE);
    EXPECT_NE(desc1->pairDeviceDescriptor_, desc1);
    audioDeviceManager.RemoveConnectedDevices(desc1);
}

/**
* @tc.name  : Test AudioDeviceManager.
* @tc.number: MakePairedDPDeviceDescriptor.
* @tc.desc  : Test MakePairedDPDeviceDescriptor.
*/
HWTEST_F(AudioDeviceManagerUnitTest, AudioDeviceManagerUnitTest_017, TestSize.Level1)
{
    AudioDeviceManager &audioDeviceManager = AudioDeviceManager::GetAudioDeviceManager();
    std::shared_ptr<AudioDeviceDescriptor> desc1 = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_MIC, DeviceRole::INPUT_DEVICE);
    desc1->networkId_ = "444455556666abcdef";

    std::shared_ptr<AudioDeviceDescriptor> desc2 = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_DP, DeviceRole::OUTPUT_DEVICE);
    desc2->macAddress_ = "444455556666123456";
    audioDeviceManager.AddConnectedDevices(desc2);

    audioDeviceManager.MakePairedDPDeviceDescriptor(desc1, DeviceRole::OUTPUT_DEVICE);
    EXPECT_NE(desc1->pairDeviceDescriptor_, desc2);
    audioDeviceManager.RemoveConnectedDevices(desc2);
}

/**
 * @tc.name  : Test AudioDeviceManager.
 * @tc.number: CheckNearlinkInHQRecordingSupport_001.
 * @tc.desc  : Test CheckNearlinkInHQRecordingSupport.
 */
HWTEST_F(AudioDeviceManagerUnitTest, CheckNearlinkInHQRecordingSupport_001, TestSize.Level4)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_NEARLINK_IN, DeviceRole::INPUT_DEVICE);
    desc->highQualityRecordingSupported_ = true;
    bool result = AudioDeviceManager::GetAudioDeviceManager().CheckNearlinkInHQRecordingSupport(
        desc, DeviceRole::INPUT_DEVICE, DeviceUsage::MEDIA);
    EXPECT_EQ(result, true);

    desc->highQualityRecordingSupported_ = false;
    result = AudioDeviceManager::GetAudioDeviceManager().CheckNearlinkInHQRecordingSupport(
        desc, DeviceRole::INPUT_DEVICE, DeviceUsage::MEDIA);
    EXPECT_EQ(result, false);

    result = AudioDeviceManager::GetAudioDeviceManager().CheckNearlinkInHQRecordingSupport(
        desc, DeviceRole::INPUT_DEVICE, DeviceUsage::VOICE);
    EXPECT_EQ(result, true);

    result = AudioDeviceManager::GetAudioDeviceManager().CheckNearlinkInHQRecordingSupport(
        desc, DeviceRole::OUTPUT_DEVICE, DeviceUsage::VOICE);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name  : Test AudioDeviceManager.
 * @tc.number: UpdateDeviceConnectTimestamp_001.
 * @tc.desc  : Test UpdateDeviceConnectTimestamp.
 */
HWTEST_F(AudioDeviceManagerUnitTest, UpdateDeviceConnectTimestamp_001, TestSize.Level4)
{
    AudioDeviceDescriptor desc{DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, DeviceRole::OUTPUT_DEVICE};

    auto ts = AudioPolicyUtils::GetInstance().GetCurrentTimeMS();

    desc.deviceCategory_ = CATEGORY_DEFAULT;
    AudioDeviceManager::GetAudioDeviceManager().UpdateDeviceConnectTimestamp(desc);
    EXPECT_GE(desc.connectTimeStamp_, ts);

    desc.deviceCategory_ = BT_GLASSES;
    desc.glassesWearCount_ = 0;
    AudioDeviceManager::GetAudioDeviceManager().UpdateDeviceConnectTimestamp(desc);
    EXPECT_GE(desc.connectTimeStamp_, ts);

    desc.deviceCategory_ = BT_GLASSES;
    desc.glassesWearCount_ = 1;
    AudioDeviceManager::GetAudioDeviceManager().UpdateDeviceConnectTimestamp(desc);
    EXPECT_LE(desc.connectTimeStamp_, ts);
}

/**
 * @tc.name  : Test AudioDeviceManager.
 * @tc.number: AudioDeviceManagerUnitTest_021.
 * @tc.desc  : Test GetCommRenderDefaultDevice with STREAM_USAGE_INTERPHONE.
 */
HWTEST_F(AudioDeviceManagerUnitTest, AudioDeviceManagerUnitTest_021, TestSize.Level1)
{
    AudioDeviceManager &audioDeviceManager = AudioDeviceManager::GetAudioDeviceManager();
    shared_ptr<AudioDeviceDescriptor> desc = audioDeviceManager.GetCommRenderDefaultDevice(STREAM_USAGE_INTERPHONE);
    EXPECT_EQ(make_shared<AudioDeviceDescriptor>(audioDeviceManager.speaker_), desc);
}

/**
 * @tc.name  : Test AudioDeviceManager.
 * @tc.number: UpdateDeviceCategory_001.
 * @tc.desc  : Test UpdateDeviceCategory.
 */
HWTEST_F(AudioDeviceManagerUnitTest, UpdateDeviceCategory_001, TestSize.Level4)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();
    auto backup = deviceManager.connectedDevices_;

    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, DeviceRole::OUTPUT_DEVICE);
    desc->deviceCategory_ = BT_GLASSES;

    deviceManager.connectedDevices_ = { std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, DeviceRole::OUTPUT_DEVICE) };
    desc->glassesWearCount_ = 0;
    EXPECT_TRUE(deviceManager.UpdateDeviceCategory(desc));

    deviceManager.connectedDevices_ = { std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, DeviceRole::OUTPUT_DEVICE) };
    desc->glassesWearCount_ = 2;
    EXPECT_TRUE(deviceManager.UpdateDeviceCategory(desc));

    deviceManager.connectedDevices_ = backup;
}

/**
* @tc.name  : Test AudioDeviceManager.
* @tc.number: IsDeviceAvailable_001
* @tc.desc  : Test IsDeviceAvailable interface.
*/
HWTEST_F(AudioDeviceManagerUnitTest, IsDeviceAvailable_001, TestSize.Level1)
{
    AudioDeviceManager &audioDeviceManager = AudioDeviceManager::GetAudioDeviceManager();
    
    // Test with normal device
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_SPEAKER, DeviceRole::OUTPUT_DEVICE);
    desc->exceptionFlag_ = false;
    desc->isEnable_ = true;
    desc->connectState_ = CONNECTED;
    bool result = audioDeviceManager.IsDeviceAvailable(CALL_OUTPUT_DEVICES, desc);
    EXPECT_EQ(true, result);
    
    // Test with exception flag set
    desc->exceptionFlag_ = true;
    result = audioDeviceManager.IsDeviceAvailable(CALL_OUTPUT_DEVICES, desc);
    EXPECT_EQ(false, result);
    
    // Test with disabled device
    desc->exceptionFlag_ = false;
    desc->isEnable_ = false;
    result = audioDeviceManager.IsDeviceAvailable(CALL_OUTPUT_DEVICES, desc);
    EXPECT_EQ(false, result);
}

/**
* @tc.name  : Test AudioDeviceManager.
* @tc.number: IsDeviceAvailable_002
* @tc.desc  : Test IsDeviceAvailable interface for Bluetooth SCO.
*/
HWTEST_F(AudioDeviceManagerUnitTest, IsDeviceAvailable_002, TestSize.Level1)
{
    AudioDeviceManager &audioDeviceManager = AudioDeviceManager::GetAudioDeviceManager();
    
    // Test with Bluetooth SCO device in normal connected state
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_SCO, DeviceRole::OUTPUT_DEVICE);
    desc->exceptionFlag_ = false;
    desc->isEnable_ = true;
    desc->connectState_ = CONNECTED;
    bool result = audioDeviceManager.IsDeviceAvailable(CALL_OUTPUT_DEVICES, desc);
    EXPECT_EQ(true, result);
    
    // Test with Bluetooth SCO device in suspend connected state
    desc->connectState_ = SUSPEND_CONNECTED;
    result = audioDeviceManager.IsDeviceAvailable(CALL_OUTPUT_DEVICES, desc);
    EXPECT_EQ(false, result);
}

/**
* @tc.name  : Test AudioDeviceManager.
* @tc.number: IsDeviceAvailable_003
* @tc.desc  : Test IsDeviceAvailable interface for virtual connected devices.
*/
HWTEST_F(AudioDeviceManagerUnitTest, IsDeviceAvailable_003, TestSize.Level1)
{
    AudioDeviceManager &audioDeviceManager = AudioDeviceManager::GetAudioDeviceManager();
    
    // Test with virtual connected device
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_SPEAKER, DeviceRole::OUTPUT_DEVICE);
    desc->exceptionFlag_ = false;
    desc->isEnable_ = true;
    desc->connectState_ = VIRTUAL_CONNECTED;
    bool result = audioDeviceManager.IsDeviceAvailable(CALL_OUTPUT_DEVICES, desc);
    EXPECT_EQ(false, result);
}

/**
* @tc.name  : Test AudioDeviceManager.
* @tc.number: IsDeviceAvailable_004
* @tc.desc  : Test IsDeviceAvailable interface for Bluetooth A2DP.
*/
HWTEST_F(AudioDeviceManagerUnitTest, IsDeviceAvailable_004, TestSize.Level1)
{
    AudioDeviceManager &audioDeviceManager = AudioDeviceManager::GetAudioDeviceManager();
    
    // Test with Bluetooth A2DP device in suspend connected state (should be available)
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, DeviceRole::OUTPUT_DEVICE);
    desc->exceptionFlag_ = false;
    desc->isEnable_ = true;
    desc->connectState_ = SUSPEND_CONNECTED;
    bool result = audioDeviceManager.IsDeviceAvailable(CALL_OUTPUT_DEVICES, desc);
    EXPECT_EQ(true, result);
}

/**
* @tc.name  : Test AudioDeviceManager.
* @tc.number: IsDeviceAvailable_005
* @tc.desc  : Test IsDeviceAvailable interface with nullptr.
*/
HWTEST_F(AudioDeviceManagerUnitTest, IsDeviceAvailable_005, TestSize.Level1)
{
    AudioDeviceManager &audioDeviceManager = AudioDeviceManager::GetAudioDeviceManager();
    
    // Test with nullptr
    std::shared_ptr<AudioDeviceDescriptor> desc = nullptr;
    bool result = audioDeviceManager.IsDeviceAvailable(CALL_OUTPUT_DEVICES, desc);
    EXPECT_EQ(false, result);
}

/**
* @tc.name  : Test AudioDeviceManager.
* @tc.number: IsDeviceAvailable_006
* @tc.desc  : Test IsDeviceAvailable interface for Nearlink.
*/
HWTEST_F(AudioDeviceManagerUnitTest, IsDeviceAvailable_006, TestSize.Level1)
{
    AudioDeviceManager &audioDeviceManager = AudioDeviceManager::GetAudioDeviceManager();

    // Test with Nearlink device (should be unavailable)
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_NEARLINK, DeviceRole::OUTPUT_DEVICE);
    desc->exceptionFlag_ = false;
    desc->isEnable_ = true;
    desc->deviceUsage_ = RECOGNITION;
    bool result = audioDeviceManager.IsDeviceAvailable(CALL_OUTPUT_DEVICES, desc);
    EXPECT_EQ(false, result);

    desc->deviceUsage_ = ALL_USAGE;
    result = audioDeviceManager.IsDeviceAvailable(CALL_OUTPUT_DEVICES, desc);
    EXPECT_EQ(true, result);
}

/**
* @tc.name  : Test AudioDeviceManager.
* @tc.number: GetDevicePriority_001
* @tc.desc  : Test GetDevicePriority with different conditions.
*/
HWTEST_F(AudioDeviceManagerUnitTest, GetDevicePriority_001, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_DP,
        DeviceRole::OUTPUT_DEVICE);
    AudioDeviceManager &audioDeviceManager = AudioDeviceManager::GetAudioDeviceManager();
    EXPECT_EQ(audioDeviceManager.GetDevicePriority(desc), PRIORITY_BASE);
    desc->deviceType_ = DeviceType::DEVICE_TYPE_SPEAKER;
    desc->networkId_ = "RemoteDevice";
    EXPECT_EQ(audioDeviceManager.GetDevicePriority(desc), PRIORITY_DISTRIBUTED);
    desc->deviceType_ = DeviceType::DEVICE_TYPE_REMOTE_CAST;
    EXPECT_EQ(audioDeviceManager.GetDevicePriority(desc), PRIORITY_DISTRIBUTED);
}

/**
* @tc.name  : Test AudioDeviceManager.
* @tc.number: TriggerDeviceInfoUpdatedCallback_001
* @tc.desc  : Test TriggerDeviceInfoUpdatedCallback with different conditions.
*/
HWTEST_F(AudioDeviceManagerUnitTest, TriggerDeviceInfoUpdatedCallback_001, TestSize.Level1)
{
    AudioDeviceDescriptor desc{DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, DeviceRole::OUTPUT_DEVICE};
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descForCb = {};

    // Test case 1: not glasses
    desc.deviceCategory_ = DeviceCategory::BT_UNWEAR_HEADPHONE;
    desc.glassesWearCount_ = 0;
    bool result = AudioDeviceManager::GetAudioDeviceManager().TriggerDeviceInfoUpdatedCallback(desc, descForCb);
    EXPECT_TRUE(result);

    // Test case 2: glasses first unwear
    desc.deviceCategory_ = DeviceCategory::BT_UNWEAR_HEADPHONE;
    desc.glassesWearCount_ = 1;
    result = AudioDeviceManager::GetAudioDeviceManager().TriggerDeviceInfoUpdatedCallback(desc, descForCb);
    EXPECT_FALSE(result);

    // Test case 3: glasses unwear again
    desc.deviceCategory_ = DeviceCategory::BT_UNWEAR_HEADPHONE;
    desc.glassesWearCount_ = 2;
    result = AudioDeviceManager::GetAudioDeviceManager().TriggerDeviceInfoUpdatedCallback(desc, descForCb);
    EXPECT_FALSE(result);

    // Test case 4: not glasses
    desc.deviceCategory_ = DeviceCategory::CATEGORY_DEFAULT;
    desc.glassesWearCount_ = 0;  // WEAR_FIRST is 1, so 0 < 1
    result = AudioDeviceManager::GetAudioDeviceManager().TriggerDeviceInfoUpdatedCallback(desc, descForCb);
    EXPECT_TRUE(result);

    // Test case 5: glasses first wear
    desc.deviceCategory_ = DeviceCategory::BT_GLASSES;
    desc.glassesWearCount_ = 1;
    result = AudioDeviceManager::GetAudioDeviceManager().TriggerDeviceInfoUpdatedCallback(desc, descForCb);
    EXPECT_TRUE(result);
}

/**
* @tc.name  : Test AudioDeviceManager.
* @tc.number: AudioDeviceManagerUnitTest_020.
* @tc.desc  : Test GetCommRenderDefaultDevice.
*/
HWTEST_F(AudioDeviceManagerUnitTest, AudioDeviceManagerUnitTest_020, TestSize.Level1)
{
    AudioDeviceManager &audioDeviceManager = AudioDeviceManager::GetAudioDeviceManager();
    shared_ptr<AudioDeviceDescriptor> desc0 = audioDeviceManager.GetCommRenderDefaultDevice(STREAM_USAGE_MEDIA);
    shared_ptr<AudioDeviceDescriptor> desc1 = audioDeviceManager.GetCommRenderDefaultDevice(STREAM_USAGE_INVALID);
    EXPECT_NE(desc0, desc1);
    desc1 = audioDeviceManager.GetCommRenderDefaultDevice(STREAM_USAGE_VOICE_RINGTONE);
    EXPECT_NE(desc0, desc1);
    shared_ptr<AudioDeviceDescriptor> desc2 = audioDeviceManager.GetCommRenderDefaultDevice(STREAM_USAGE_ULTRASONIC);
    EXPECT_EQ(make_shared<AudioDeviceDescriptor>(audioDeviceManager.earpiece_), desc2);
    desc1 = audioDeviceManager.GetCommRenderDefaultDevice(STREAM_USAGE_VIDEO_COMMUNICATION);
    EXPECT_EQ(make_shared<AudioDeviceDescriptor>(audioDeviceManager.speaker_), desc1);
}
} // namespace AudioStandard
} // namespace OHOS