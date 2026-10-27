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
    AudioPolicyUtils::GetInstance().UpdateDeviceConnectTimestamp(desc);
    EXPECT_GE(desc.connectTimeStamp_, ts);

    desc.deviceCategory_ = BT_GLASSES;
    desc.glassesWearCount_ = 0;
    AudioPolicyUtils::GetInstance().UpdateDeviceConnectTimestamp(desc);
    EXPECT_GE(desc.connectTimeStamp_, ts);

    desc.deviceCategory_ = BT_GLASSES;
    desc.glassesWearCount_ = 1;
    AudioPolicyUtils::GetInstance().UpdateDeviceConnectTimestamp(desc);
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
    if (desc && audioDeviceManager.speaker_) {
        EXPECT_EQ(audioDeviceManager.speaker_->IsSameDeviceDescPtr(desc), true);
    }
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
    std::shared_ptr<AudioDeviceDescriptor> updated = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, DeviceRole::OUTPUT_DEVICE);
    deviceManager.connectedDevices_ = { desc };

    // CATEGORY_DEFAULT -> CATEGORY_DEFAULT
    EXPECT_FALSE(AudioPolicyUtils::GetInstance().UpdateDeviceCategory(deviceManager.connectedDevices_, updated));

    // BT_UNWEAR_HEADPHONE -> BT_HEADPHONE
    desc->deviceCategory_ = BT_UNWEAR_HEADPHONE;
    updated->deviceCategory_ = BT_HEADPHONE;
    EXPECT_TRUE(AudioPolicyUtils::GetInstance().UpdateDeviceCategory(deviceManager.connectedDevices_, updated));

    // BT_HEADPHONE -> BT_UNWEAR_HEADPHONE
    desc->deviceCategory_ = BT_HEADPHONE;
    updated->deviceCategory_ = BT_UNWEAR_HEADPHONE;
    EXPECT_TRUE(AudioPolicyUtils::GetInstance().UpdateDeviceCategory(deviceManager.connectedDevices_, updated));

    // BT_UNWEAR_HEADPHONE -> BT_GLASSES
    desc->deviceCategory_ = BT_UNWEAR_HEADPHONE;
    updated->deviceCategory_ = BT_GLASSES;
    EXPECT_TRUE(AudioPolicyUtils::GetInstance().UpdateDeviceCategory(deviceManager.connectedDevices_, updated));
    EXPECT_EQ(desc->glassesWearCount_, 1);

    // BT_GLASSES -> BT_UNWEAR_HEADPHONE
    desc->deviceCategory_ = BT_GLASSES;
    updated->deviceCategory_ = BT_UNWEAR_HEADPHONE;
    EXPECT_TRUE(AudioPolicyUtils::GetInstance().UpdateDeviceCategory(deviceManager.connectedDevices_, updated));
    EXPECT_EQ(desc->glassesWearCount_, 1);

    // BT_UNWEAR_HEADPHONE -> BT_GLASSES
    desc->deviceCategory_ = BT_UNWEAR_HEADPHONE;
    updated->deviceCategory_ = BT_GLASSES;
    EXPECT_TRUE(AudioPolicyUtils::GetInstance().UpdateDeviceCategory(deviceManager.connectedDevices_, updated));
    EXPECT_EQ(desc->glassesWearCount_, 2);

    deviceManager.connectedDevices_ = backup;
}

/**
 * @tc.name  : Test AudioDeviceManager.
 * @tc.number: UpdateEnableState_001.
 * @tc.desc  : Test UpdateEnableState.
 */
HWTEST_F(AudioDeviceManagerUnitTest, UpdateEnableState_001, TestSize.Level4)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();
    auto backup = deviceManager.connectedDevices_;

    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, DeviceRole::OUTPUT_DEVICE);
    std::shared_ptr<AudioDeviceDescriptor> updated = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, DeviceRole::OUTPUT_DEVICE);
    deviceManager.connectedDevices_ = { desc };

    // true -> true
    desc->isEnable_ = true;
    updated->isEnable_ = true;
    EXPECT_FALSE(AudioPolicyUtils::GetInstance().UpdateEnableState(deviceManager.connectedDevices_, updated));

    // true -> false
    desc->isEnable_ = true;
    updated->isEnable_ = false;
    EXPECT_TRUE(AudioPolicyUtils::GetInstance().UpdateEnableState(deviceManager.connectedDevices_, updated));

    // false -> true
    desc->isEnable_ = false;
    updated->isEnable_ = true;
    EXPECT_TRUE(AudioPolicyUtils::GetInstance().UpdateEnableState(deviceManager.connectedDevices_, updated));

    // false -> false
    desc->isEnable_ = false;
    updated->isEnable_ = false;
    EXPECT_FALSE(AudioPolicyUtils::GetInstance().UpdateEnableState(deviceManager.connectedDevices_, updated));

    deviceManager.connectedDevices_ = backup;
}

/**
 * @tc.name  : Test AudioDeviceManager.
 * @tc.number: UpdateDeviceUsage_001.
 * @tc.desc  : Test UpdateDeviceUsage.
 */
HWTEST_F(AudioDeviceManagerUnitTest, UpdateDeviceUsage_001, TestSize.Level4)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();
    auto backup = deviceManager.connectedDevices_;

    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_NEARLINK, DeviceRole::OUTPUT_DEVICE);
    std::shared_ptr<AudioDeviceDescriptor> updated = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_NEARLINK, DeviceRole::OUTPUT_DEVICE);
    deviceManager.connectedDevices_ = { desc };
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;

    // ALL_USAGE -> ALL_USAGE
    desc->deviceUsage_ = ALL_USAGE;
    updated->deviceUsage_ = ALL_USAGE;
    EXPECT_FALSE(AudioPolicyUtils::GetInstance().UpdateDeviceUsage(deviceManager.connectedDevices_, updated, reason));

    // ALL_USAGE -> VOICE
    desc->deviceUsage_ = ALL_USAGE;
    updated->deviceUsage_ = VOICE;
    EXPECT_TRUE(AudioPolicyUtils::GetInstance().UpdateDeviceUsage(deviceManager.connectedDevices_, updated, reason));

    // VOICE -> ALL_USAGE
    desc->deviceUsage_ = VOICE;
    updated->deviceUsage_ = ALL_USAGE;
    EXPECT_TRUE(AudioPolicyUtils::GetInstance().UpdateDeviceUsage(deviceManager.connectedDevices_, updated, reason));

    // VOICE -> VOICE
    desc->deviceUsage_ = VOICE;
    updated->deviceUsage_ = VOICE;
    EXPECT_FALSE(AudioPolicyUtils::GetInstance().UpdateDeviceUsage(deviceManager.connectedDevices_, updated, reason));

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
    bool result = AudioPolicyUtils::GetInstance().TriggerDeviceInfoUpdatedCallback(desc, descForCb);
    EXPECT_TRUE(result);

    // Test case 2: glasses first unwear
    desc.deviceCategory_ = DeviceCategory::BT_UNWEAR_HEADPHONE;
    desc.glassesWearCount_ = 1;
    result = AudioPolicyUtils::GetInstance().TriggerDeviceInfoUpdatedCallback(desc, descForCb);
    EXPECT_FALSE(result);

    // Test case 3: glasses unwear again
    desc.deviceCategory_ = DeviceCategory::BT_UNWEAR_HEADPHONE;
    desc.glassesWearCount_ = 2;
    result = AudioPolicyUtils::GetInstance().TriggerDeviceInfoUpdatedCallback(desc, descForCb);
    EXPECT_FALSE(result);

    // Test case 4: not glasses
    desc.deviceCategory_ = DeviceCategory::CATEGORY_DEFAULT;
    desc.glassesWearCount_ = 0;  // WEAR_FIRST is 1, so 0 < 1
    result = AudioPolicyUtils::GetInstance().TriggerDeviceInfoUpdatedCallback(desc, descForCb);
    EXPECT_TRUE(result);

    // Test case 5: glasses first wear
    desc.deviceCategory_ = DeviceCategory::BT_GLASSES;
    desc.glassesWearCount_ = 1;
    result = AudioPolicyUtils::GetInstance().TriggerDeviceInfoUpdatedCallback(desc, descForCb);
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

/**
 * @tc.name  : Test AudioDeviceManager.
 * @tc.number: UpdateDevicesListInfo_001.
 * @tc.desc  : Test UpdateDevicesListInfo with CATEGORY_UPDATE.
 */
HWTEST_F(AudioDeviceManagerUnitTest, UpdateDevicesListInfo_001, TestSize.Level1)
{
    auto &dm = AudioDeviceManager::GetAudioDeviceManager();
    auto backup = dm.connectedDevices_;
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    auto desc = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_A2DP, OUTPUT_DEVICE);
    desc->deviceCategory_ = BT_GLASSES;
    dm.connectedDevices_ = { std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_A2DP, OUTPUT_DEVICE) };
    EXPECT_TRUE(dm.UpdateDevicesListInfo(desc, CATEGORY_UPDATE, reason));
    EXPECT_EQ(dm.connectedDevices_[0]->deviceCategory_, BT_GLASSES);
    EXPECT_EQ(reason, AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE);

    auto unwearDesc = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_A2DP, OUTPUT_DEVICE);
    unwearDesc->deviceCategory_ = BT_UNWEAR_HEADPHONE;
    EXPECT_TRUE(dm.UpdateDevicesListInfo(unwearDesc, CATEGORY_UPDATE, reason));
    EXPECT_EQ(dm.connectedDevices_[0]->deviceCategory_, BT_UNWEAR_HEADPHONE);
    EXPECT_EQ(reason, AudioStreamDeviceChangeReason::OLD_DEVICE_UNAVALIABLE);
    dm.connectedDevices_ = backup;
}

/**
 * @tc.name  : HasSpecificDevicesConnected_WirelessDeviceConnected
 * @tc.number: HasSpecificDevicesConnected_001
 * @tc.desc  : Test HasSpecificDevicesConnected returns true for wireless device with non-virtual connect state
 */
HWTEST_F(AudioDeviceManagerUnitTest, HasSpecificDevicesConnected_WirelessDeviceConnected, TestSize.Level1)
{
    auto &dm = AudioDeviceManager::GetAudioDeviceManager();
    auto backup = dm.connectedDevices_;

    auto desc = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_A2DP, OUTPUT_DEVICE);
    desc->connectState_ = CONNECTED;
    dm.connectedDevices_ = { desc };

    bool result = dm.HasSpecificDevicesConnected();
    EXPECT_TRUE(result);

    dm.connectedDevices_ = backup;
}

/**
 * @tc.name  : HasSpecificDevicesConnected_WirelessDeviceVirtualConnected
 * @tc.number: HasSpecificDevicesConnected_002
 * @tc.desc  : Test HasSpecificDevicesConnected returns false for wireless device with VIRTUAL_CONNECTED state
 */
HWTEST_F(AudioDeviceManagerUnitTest, HasSpecificDevicesConnected_WirelessDeviceVirtualConnected, TestSize.Level1)
{
    auto &dm = AudioDeviceManager::GetAudioDeviceManager();
    auto backup = dm.connectedDevices_;

    auto desc = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_A2DP, OUTPUT_DEVICE);
    desc->connectState_ = VIRTUAL_CONNECTED;
    dm.connectedDevices_ = { desc };

    bool result = dm.HasSpecificDevicesConnected();
    EXPECT_FALSE(result);

    dm.connectedDevices_ = backup;
}

/**
 * @tc.name  : HasSpecificDevicesConnected_UsbArmHeadset
 * @tc.number: HasSpecificDevicesConnected_003
 * @tc.desc  : Test HasSpecificDevicesConnected returns true for DEVICE_TYPE_USB_ARM_HEADSET
 */
HWTEST_F(AudioDeviceManagerUnitTest, HasSpecificDevicesConnected_UsbArmHeadset, TestSize.Level1)
{
    auto &dm = AudioDeviceManager::GetAudioDeviceManager();
    auto backup = dm.connectedDevices_;

    auto desc = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_USB_ARM_HEADSET, INPUT_DEVICE);
    dm.connectedDevices_ = { desc };

    bool result = dm.HasSpecificDevicesConnected();
    EXPECT_TRUE(result);

    dm.connectedDevices_ = backup;
}

/**
 * @tc.name  : HasSpecificDevicesConnected_Accessory
 * @tc.number: HasSpecificDevicesConnected_004
 * @tc.desc  : Test HasSpecificDevicesConnected returns true for DEVICE_TYPE_ACCESSORY
 */
HWTEST_F(AudioDeviceManagerUnitTest, HasSpecificDevicesConnected_Accessory, TestSize.Level1)
{
    auto &dm = AudioDeviceManager::GetAudioDeviceManager();
    auto backup = dm.connectedDevices_;

    auto desc = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_ACCESSORY, INPUT_DEVICE);
    dm.connectedDevices_ = { desc };

    bool result = dm.HasSpecificDevicesConnected();
    EXPECT_TRUE(result);

    dm.connectedDevices_ = backup;
}

/**
 * @tc.name  : HasSpecificDevicesConnected_BtSpp
 * @tc.number: HasSpecificDevicesConnected_005
 * @tc.desc  : Test HasSpecificDevicesConnected returns true for DEVICE_TYPE_BT_SPP
 */
HWTEST_F(AudioDeviceManagerUnitTest, HasSpecificDevicesConnected_BtSpp, TestSize.Level1)
{
    auto &dm = AudioDeviceManager::GetAudioDeviceManager();
    auto backup = dm.connectedDevices_;

    auto desc = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BT_SPP, INPUT_DEVICE);
    dm.connectedDevices_ = { desc };

    bool result = dm.HasSpecificDevicesConnected();
    EXPECT_TRUE(result);

    dm.connectedDevices_ = backup;
}

/**
 * @tc.name  : HasSpecificDevicesConnected_NoSpecificDevice
 * @tc.number: HasSpecificDevicesConnected_006
 * @tc.desc  : Test HasSpecificDevicesConnected returns false for non-specific non-wireless device
 */
HWTEST_F(AudioDeviceManagerUnitTest, HasSpecificDevicesConnected_NoSpecificDevice, TestSize.Level1)
{
    auto &dm = AudioDeviceManager::GetAudioDeviceManager();
    auto backup = dm.connectedDevices_;

    auto desc = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    dm.connectedDevices_ = { desc };

    bool result = dm.HasSpecificDevicesConnected();
    EXPECT_FALSE(result);

    dm.connectedDevices_ = backup;
}

/**
 * @tc.name  : HasSpecificDevicesConnected_EmptyDevices
 * @tc.number: HasSpecificDevicesConnected_007
 * @tc.desc  : Test HasSpecificDevicesConnected returns false when no devices connected
 */
HWTEST_F(AudioDeviceManagerUnitTest, HasSpecificDevicesConnected_EmptyDevices, TestSize.Level1)
{
    auto &dm = AudioDeviceManager::GetAudioDeviceManager();
    auto backup = dm.connectedDevices_;

    dm.connectedDevices_ = {};

    bool result = dm.HasSpecificDevicesConnected();
    EXPECT_FALSE(result);

    dm.connectedDevices_ = backup;
}

/**
 * @tc.name  : HasSpecificDevicesConnected_NullptrDevice
 * @tc.number: HasSpecificDevicesConnected_008
 * @tc.desc  : Test HasSpecificDevicesConnected skips nullptr device in connectedDevices_
 */
HWTEST_F(AudioDeviceManagerUnitTest, HasSpecificDevicesConnected_NullptrDevice, TestSize.Level1)
{
    auto &dm = AudioDeviceManager::GetAudioDeviceManager();
    auto backup = dm.connectedDevices_;

    dm.connectedDevices_ = { nullptr };

    bool result = dm.HasSpecificDevicesConnected();
    EXPECT_FALSE(result);

    dm.connectedDevices_ = backup;
}

/**
 * @tc.name  : HasScoDevicesRealConnected_ScoConnected
 * @tc.number: HasScoDevicesRealConnected_001
 * @tc.desc  : Test HasScoDevicesRealConnected returns true for SCO device with non-virtual connect state
 */
HWTEST_F(AudioDeviceManagerUnitTest, HasScoDevicesRealConnected_ScoConnected, TestSize.Level1)
{
    auto &dm = AudioDeviceManager::GetAudioDeviceManager();
    auto backup = dm.connectedDevices_;

    auto desc = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_SCO, OUTPUT_DEVICE);
    desc->connectState_ = CONNECTED;
    dm.connectedDevices_ = { desc };

    bool result = dm.HasScoDevicesRealConnected();
    EXPECT_TRUE(result);

    dm.connectedDevices_ = backup;
}

/**
 * @tc.name  : HasScoDevicesRealConnected_ScoVirtualConnected
 * @tc.number: HasScoDevicesRealConnected_002
 * @tc.desc  : Test HasScoDevicesRealConnected returns false for SCO device with VIRTUAL_CONNECTED state
 */
HWTEST_F(AudioDeviceManagerUnitTest, HasScoDevicesRealConnected_ScoVirtualConnected, TestSize.Level1)
{
    auto &dm = AudioDeviceManager::GetAudioDeviceManager();
    auto backup = dm.connectedDevices_;

    auto desc = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_SCO, OUTPUT_DEVICE);
    desc->connectState_ = VIRTUAL_CONNECTED;
    dm.connectedDevices_ = { desc };

    bool result = dm.HasScoDevicesRealConnected();
    EXPECT_FALSE(result);

    dm.connectedDevices_ = backup;
}

/**
 * @tc.name  : HasScoDevicesRealConnected_A2dpOnlyConnected
 * @tc.number: HasScoDevicesRealConnected_003
 * @tc.desc  : Test HasScoDevicesRealConnected returns false when only A2DP device is connected
 */
HWTEST_F(AudioDeviceManagerUnitTest, HasScoDevicesRealConnected_A2dpOnlyConnected, TestSize.Level1)
{
    auto &dm = AudioDeviceManager::GetAudioDeviceManager();
    auto backup = dm.connectedDevices_;

    auto desc = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_A2DP, OUTPUT_DEVICE);
    desc->connectState_ = CONNECTED;
    dm.connectedDevices_ = { desc };

    bool result = dm.HasScoDevicesRealConnected();
    EXPECT_FALSE(result);

    dm.connectedDevices_ = backup;
}

/**
 * @tc.name  : HasScoDevicesRealConnected_NoScoDevice
 * @tc.number: HasScoDevicesRealConnected_004
 * @tc.desc  : Test HasScoDevicesRealConnected returns false for non-SCO device
 */
HWTEST_F(AudioDeviceManagerUnitTest, HasScoDevicesRealConnected_NoScoDevice, TestSize.Level1)
{
    auto &dm = AudioDeviceManager::GetAudioDeviceManager();
    auto backup = dm.connectedDevices_;

    auto desc = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    dm.connectedDevices_ = { desc };

    bool result = dm.HasScoDevicesRealConnected();
    EXPECT_FALSE(result);

    dm.connectedDevices_ = backup;
}

/**
 * @tc.name  : HasScoDevicesRealConnected_EmptyDevices
 * @tc.number: HasScoDevicesRealConnected_005
 * @tc.desc  : Test HasScoDevicesRealConnected returns false when no devices connected
 */
HWTEST_F(AudioDeviceManagerUnitTest, HasScoDevicesRealConnected_EmptyDevices, TestSize.Level1)
{
    auto &dm = AudioDeviceManager::GetAudioDeviceManager();
    auto backup = dm.connectedDevices_;

    dm.connectedDevices_ = {};

    bool result = dm.HasScoDevicesRealConnected();
    EXPECT_FALSE(result);

    dm.connectedDevices_ = backup;
}

/**
 * @tc.name  : HasScoDevicesRealConnected_NullptrDevice
 * @tc.number: HasScoDevicesRealConnected_006
 * @tc.desc  : Test HasScoDevicesRealConnected skips nullptr device in connectedDevices_
 */
HWTEST_F(AudioDeviceManagerUnitTest, HasScoDevicesRealConnected_NullptrDevice, TestSize.Level1)
{
    auto &dm = AudioDeviceManager::GetAudioDeviceManager();
    auto backup = dm.connectedDevices_;

    dm.connectedDevices_ = { nullptr };

    bool result = dm.HasScoDevicesRealConnected();
    EXPECT_FALSE(result);

    dm.connectedDevices_ = backup;
}

/**
 * @tc.name  : OnXmlParsingCompleted_NegativeDevice
 * @tc.number: OnXmlParsingCompleted_NegativeDevice_001
 * @tc.desc  : Test OnXmlParsingCompleted parses TYPE_NEGATIVE devices correctly
 */
HWTEST_F(AudioDeviceManagerUnitTest, OnXmlParsingCompleted_NegativeDevice, TestSize.Level1)
{
    auto &dm = AudioDeviceManager::GetAudioDeviceManager();

    DevicePrivacyInfo negativeInfo = {};
    negativeInfo.deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    negativeInfo.deviceCategory = BT_WATCH;
    negativeInfo.deviceUsage = MEDIA;

    std::unordered_map<AudioDevicePrivacyType, std::list<DevicePrivacyInfo>> xmlData;
    xmlData[TYPE_NEGATIVE].push_back(negativeInfo);

    dm.OnXmlParsingCompleted(xmlData);

    EXPECT_FALSE(dm.negativeDeviceList_.empty());
    EXPECT_EQ(dm.negativeDeviceList_.front().deviceType, DEVICE_TYPE_BLUETOOTH_A2DP);
    EXPECT_EQ(dm.negativeDeviceList_.front().deviceCategory, BT_WATCH);
}

/**
 * @tc.name  : DeviceAttrMatch_NegativeType
 * @tc.number: DeviceAttrMatch_NegativeType_001
 * @tc.desc  : Test DeviceAttrMatch returns true when privacyType is TYPE_NEGATIVE and device matches
 */
HWTEST_F(AudioDeviceManagerUnitTest, DeviceAttrMatch_NegativeType, TestSize.Level1)
{
    auto &dm = AudioDeviceManager::GetAudioDeviceManager();

    DevicePrivacyInfo negativeInfo = {};
    negativeInfo.deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    negativeInfo.deviceCategory = BT_WATCH;
    negativeInfo.deviceUsage = MEDIA;

    dm.negativeDeviceList_.push_back(negativeInfo);

    auto desc = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_A2DP, OUTPUT_DEVICE);
    desc->deviceCategory_ = BT_WATCH;
    desc->deviceUsage_ = MEDIA;
    desc->connectState_ = CONNECTED;

    bool result = dm.DeviceAttrMatch(desc, TYPE_NEGATIVE, OUTPUT_DEVICE, MEDIA);
    EXPECT_TRUE(result);
}

/**
 * @tc.name  : GetDevicePrivacyType_ReturnNegative
 * @tc.number: GetDevicePrivacyType_ReturnNegative_001
 * @tc.desc  : Test GetDevicePrivacyType returns TYPE_NEGATIVE when device doesn't match privacy or public list
 */
HWTEST_F(AudioDeviceManagerUnitTest, GetDevicePrivacyType_ReturnNegative, TestSize.Level1)
{
    auto &dm = AudioDeviceManager::GetAudioDeviceManager();

    dm.privacyDeviceList_.clear();
    dm.publicDeviceList_.clear();
    dm.negativeDeviceList_.clear();

    auto desc = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    desc->deviceUsage_ = MEDIA;

    auto result = dm.GetDevicePrivacyType(desc);
    EXPECT_EQ(result, TYPE_NEGATIVE);
}

/**
 * @tc.name  : RemoveCommunicationDevices_Negative
 * @tc.number: RemoveCommunicationDevices_Negative_001
 * @tc.desc  : Test RemoveCommunicationDevices removes negative devices correctly
 */
HWTEST_F(AudioDeviceManagerUnitTest, RemoveCommunicationDevices_Negative, TestSize.Level1)
{
    auto &dm = AudioDeviceManager::GetAudioDeviceManager();

    auto desc = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_A2DP, OUTPUT_DEVICE);
    desc->macAddress_ = "00:11:22:33:44:55";
    desc->deviceUsage_ = VOICE;
    desc->connectState_ = CONNECTED;

    dm.commRenderNegativeDevices_.push_back(desc);
    EXPECT_EQ(dm.commRenderNegativeDevices_.size(), 1);

    AudioDeviceDescriptor removeDesc(DEVICE_TYPE_BLUETOOTH_A2DP, OUTPUT_DEVICE);
    removeDesc.macAddress_ = "00:11:22:33:44:55";
    removeDesc.deviceUsage_ = VOICE;

    dm.RemoveCommunicationDevices(removeDesc);
    EXPECT_EQ(dm.commRenderNegativeDevices_.size(), 0);
}

/**
 * @tc.name  : RemoveMediaDevices_Negative
 * @tc.number: RemoveMediaDevices_Negative_001
 * @tc.desc  : Test RemoveMediaDevices removes negative devices correctly
 */
HWTEST_F(AudioDeviceManagerUnitTest, RemoveMediaDevices_Negative, TestSize.Level1)
{
    auto &dm = AudioDeviceManager::GetAudioDeviceManager();

    auto desc = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_A2DP, OUTPUT_DEVICE);
    desc->macAddress_ = "00:11:22:33:44:55";
    desc->deviceUsage_ = MEDIA;
    desc->connectState_ = CONNECTED;

    dm.mediaRenderNegativeDevices_.push_back(desc);
    EXPECT_EQ(dm.mediaRenderNegativeDevices_.size(), 1);

    AudioDeviceDescriptor removeDesc(DEVICE_TYPE_BLUETOOTH_A2DP, OUTPUT_DEVICE);
    removeDesc.macAddress_ = "00:11:22:33:44:55";
    removeDesc.deviceUsage_ = MEDIA;

    dm.RemoveMediaDevices(removeDesc);
    EXPECT_EQ(dm.mediaRenderNegativeDevices_.size(), 0);
}

/**
 * @tc.name  : RemoveCaptureDevices_Negative
 * @tc.number: RemoveCaptureDevices_Negative_001
 * @tc.desc  : Test RemoveCaptureDevices removes negative devices correctly
 */
HWTEST_F(AudioDeviceManagerUnitTest, RemoveCaptureDevices_Negative, TestSize.Level1)
{
    auto &dm = AudioDeviceManager::GetAudioDeviceManager();

    auto desc = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_A2DP, INPUT_DEVICE);
    desc->macAddress_ = "00:11:22:33:44:55";
    desc->deviceUsage_ = MEDIA;
    desc->connectState_ = CONNECTED;

    dm.captureNegativeDevices_.push_back(desc);
    EXPECT_EQ(dm.captureNegativeDevices_.size(), 1);

    AudioDeviceDescriptor removeDesc(DEVICE_TYPE_BLUETOOTH_A2DP, INPUT_DEVICE);
    removeDesc.macAddress_ = "00:11:22:33:44:55";
    removeDesc.deviceUsage_ = MEDIA;

    dm.RemoveCaptureDevices(removeDesc);
    EXPECT_EQ(dm.captureNegativeDevices_.size(), 0);
}

/**
 * @tc.name  : GetCommRenderNegativeDevices_Basic
 * @tc.number: GetCommRenderNegativeDevices_001
 * @tc.desc  : Test GetCommRenderNegativeDevices returns correct negative devices
 */
HWTEST_F(AudioDeviceManagerUnitTest, GetCommRenderNegativeDevices_Basic, TestSize.Level1)
{
    auto &dm = AudioDeviceManager::GetAudioDeviceManager();

    auto desc = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_A2DP, OUTPUT_DEVICE);
    desc->deviceUsage_ = VOICE;
    desc->connectState_ = CONNECTED;
    desc->connectTimeStamp_ = 1000;

    dm.commRenderNegativeDevices_.push_back(desc);
    auto result = dm.GetCommRenderNegativeDevices();

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0]->deviceType_, DEVICE_TYPE_BLUETOOTH_A2DP);
    EXPECT_EQ(result[0]->deviceUsage_, VOICE);

    dm.commRenderNegativeDevices_.clear();
}

/**
 * @tc.name  : GetCommCaptureNegativeDevices_Basic
 * @tc.number: GetCommCaptureNegativeDevices_001
 * @tc.desc  : Test GetCommCaptureNegativeDevices returns correct negative devices
 */
HWTEST_F(AudioDeviceManagerUnitTest, GetCommCaptureNegativeDevices_Basic, TestSize.Level1)
{
    auto &dm = AudioDeviceManager::GetAudioDeviceManager();

    auto desc = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_A2DP, INPUT_DEVICE);
    desc->deviceUsage_ = VOICE;
    desc->connectState_ = CONNECTED;
    desc->connectTimeStamp_ = 1000;

    dm.commCaptureNegativeDevices_.push_back(desc);
    auto result = dm.GetCommCaptureNegativeDevices();

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0]->deviceType_, DEVICE_TYPE_BLUETOOTH_A2DP);
    EXPECT_EQ(result[0]->deviceRole_, INPUT_DEVICE);

    dm.commCaptureNegativeDevices_.clear();
}

/**
 * @tc.name  : GetMediaRenderNegativeDevices_Basic
 * @tc.number: GetMediaRenderNegativeDevices_001
 * @tc.desc  : Test GetMediaRenderNegativeDevices returns correct negative devices
 */
HWTEST_F(AudioDeviceManagerUnitTest, GetMediaRenderNegativeDevices_Basic, TestSize.Level1)
{
    auto &dm = AudioDeviceManager::GetAudioDeviceManager();

    auto desc = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_A2DP, OUTPUT_DEVICE);
    desc->deviceUsage_ = MEDIA;
    desc->connectState_ = CONNECTED;
    desc->connectTimeStamp_ = 1000;

    dm.mediaRenderNegativeDevices_.push_back(desc);
    auto result = dm.GetMediaRenderNegativeDevices();

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0]->deviceType_, DEVICE_TYPE_BLUETOOTH_A2DP);
    EXPECT_EQ(result[0]->deviceUsage_, MEDIA);

    dm.mediaRenderNegativeDevices_.clear();
}

/**
 * @tc.name  : GetMediaCaptureNegativeDevices_Basic
 * @tc.number: GetMediaCaptureNegativeDevices_001
 * @tc.desc  : Test GetMediaCaptureNegativeDevices returns correct negative devices
 */
HWTEST_F(AudioDeviceManagerUnitTest, GetMediaCaptureNegativeDevices_Basic, TestSize.Level1)
{
    auto &dm = AudioDeviceManager::GetAudioDeviceManager();

    auto desc = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_A2DP, INPUT_DEVICE);
    desc->deviceUsage_ = MEDIA;
    desc->connectState_ = CONNECTED;
    desc->connectTimeStamp_ = 1000;

    dm.mediaCaptureNegativeDevices_.push_back(desc);
    auto result = dm.GetMediaCaptureNegativeDevices();

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0]->deviceType_, DEVICE_TYPE_BLUETOOTH_A2DP);
    EXPECT_EQ(result[0]->deviceUsage_, MEDIA);

    dm.mediaCaptureNegativeDevices_.clear();
}

/**
 * @tc.name  : GetCaptureNegativeDevices_Basic
 * @tc.number: GetCaptureNegativeDevices_001
 * @tc.desc  : Test GetCaptureNegativeDevices returns correct negative devices
 */
HWTEST_F(AudioDeviceManagerUnitTest, GetCaptureNegativeDevices_Basic, TestSize.Level1)
{
    auto &dm = AudioDeviceManager::GetAudioDeviceManager();

    auto desc = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_A2DP, INPUT_DEVICE);
    desc->deviceUsage_ = MEDIA;
    desc->connectState_ = CONNECTED;
    desc->connectTimeStamp_ = 1000;

    dm.captureNegativeDevices_.push_back(desc);
    auto result = dm.GetCaptureNegativeDevices();

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0]->deviceType_, DEVICE_TYPE_BLUETOOTH_A2DP);
    EXPECT_EQ(result[0]->deviceUsage_, MEDIA);

    dm.captureNegativeDevices_.clear();
}

/**
 * @tc.name  : GetCommRenderNegativeDevices_NullptrFiltered
 * @tc.number: GetCommRenderNegativeDevices_002
 * @tc.desc  : Test GetCommRenderNegativeDevices filters nullptr elements correctly.
 */
HWTEST_F(AudioDeviceManagerUnitTest, GetCommRenderNegativeDevices_NullptrFiltered, TestSize.Level1)
{
    auto &dm = AudioDeviceManager::GetAudioDeviceManager();

    std::shared_ptr<AudioDeviceDescriptor> nullDesc = nullptr;
    auto desc = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_A2DP, OUTPUT_DEVICE);
    desc->deviceUsage_ = VOICE;
    desc->connectState_ = CONNECTED;

    dm.commRenderNegativeDevices_.push_back(nullDesc);
    dm.commRenderNegativeDevices_.push_back(desc);

    auto result = dm.GetCommRenderNegativeDevices();

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0]->deviceType_, DEVICE_TYPE_BLUETOOTH_A2DP);

    dm.commRenderNegativeDevices_.clear();
}
} // namespace AudioStandard
} // namespace OHOS