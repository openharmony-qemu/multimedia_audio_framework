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

#include "audio_affinity_manager_unit_test.h"
#include "audio_policy_log.h"
#include "audio_errors.h"
#include "audio_pnp_server.h"
#include <thread>
#include <string>
#include <memory>
#include <vector>
#include <sys/socket.h>
#include <cerrno>
#include <fstream>
#include <algorithm>
using namespace std;
using namespace std::chrono;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioAffinityManagerUnitTest::SetUpTestCase(void) {}
void AudioAffinityManagerUnitTest::TearDownTestCase(void) {}
void AudioAffinityManagerUnitTest::SetUp(void) {}
void AudioAffinityManagerUnitTest::TearDown(void) {}

AffinityDeviceInfo device1 = {
    .groupName = "group1",
    .deviceType = DEVICE_TYPE_SPEAKER,
    .deviceFlag = NONE_DEVICES_FLAG,
    .networkID = "network1",
    .chooseTimeStamp = 123456,
    .isPrimary = true,
    .SupportedConcurrency = true
};

AffinityDeviceInfo device2 = {
    .groupName = "group2",
    .deviceType = DEVICE_TYPE_BLUETOOTH_A2DP,
    .deviceFlag = NONE_DEVICES_FLAG,
    .networkID = "network2",
    .chooseTimeStamp = 234567,
    .isPrimary = false,
    .SupportedConcurrency = true
};

AffinityDeviceInfo device3 = {
    .groupName = "group2",
    .deviceType = DEVICE_TYPE_WIRED_HEADSET,
    .deviceFlag = NONE_DEVICES_FLAG,
    .networkID = "network3",
    .chooseTimeStamp = 345678,
    .isPrimary = true,
    .SupportedConcurrency = false
};

std::unordered_map<int32_t, AffinityDeviceInfo> group1Devices = {
    {1, device1},
    {2, device2}
};

std::unordered_map<int32_t, AffinityDeviceInfo> group2Devices = {
    {3, device3}
};

typedef std::unordered_map<std::string, std::unordered_map<int32_t, AffinityDeviceInfo>> AFFINITYDEVINFOMAP;

AFFINITYDEVINFOMAP testActiveGroupMap_ = {
    {"group1", group1Devices},
    {"group2", group2Devices}
};

std::vector<AffinityDeviceInfo> testDevices_ = {device1, device2};

constexpr int32_t K_HUNDRED = 100;
constexpr int32_t K_TIME_SPAN_IN_MILLISECONDS_FOR_SELECTION = 200;
const int32_t DEVICE_INFO_INDEX_1 = 1;
const int32_t DEVICE_INFO_INDEX_2 = 2;
const bool FALSE_FLAG = false;
const bool NOT_SUPPORTED = false;

void PrepareTestData(AffinityDeviceInfo& deviceInfo1, AffinityDeviceInfo& deviceInfo2,
                     std::unordered_map<int32_t, AffinityDeviceInfo>& testDeviceInfoMap)
{
    deviceInfo1.groupName = "group1";
    deviceInfo1.deviceType = DeviceType::DEVICE_TYPE_SPEAKER;
    deviceInfo1.networkID = "network1";
    deviceInfo1.chooseTimeStamp = K_HUNDRED;
    deviceInfo1.isPrimary = true;
    deviceInfo1.SupportedConcurrency = true;

    deviceInfo2.groupName = "group1";
    deviceInfo2.deviceType = DeviceType::DEVICE_TYPE_EARPIECE;
    deviceInfo2.networkID = "network2";
    deviceInfo2.chooseTimeStamp = K_TIME_SPAN_IN_MILLISECONDS_FOR_SELECTION;
    deviceInfo2.isPrimary = FALSE_FLAG;
    deviceInfo2.SupportedConcurrency = NOT_SUPPORTED;

    testDeviceInfoMap[DEVICE_INFO_INDEX_1] = deviceInfo1;
    testDeviceInfoMap[DEVICE_INFO_INDEX_2] = deviceInfo2;
}

#define PRINT_LINE printf("debug __LINE__:%d\n", __LINE__)

/**
* @tc.name  : Test AudioAffnityManager.
* @tc.number: AudioAffnityManager_001
* @tc.desc  : Test ParseAffinityXml.
*/
HWTEST_F(AudioAffinityManagerUnitTest, AudioAffnityManager_001, TestSize.Level1)
{
    std::unique_ptr <AudioAffinityManager> audioAffinityManager = std::make_unique<AudioAffinityManager>();
    audioAffinityManager->ParseAffinityXml();
    // Verify the state of the method after it is executed
    EXPECT_FALSE(audioAffinityManager->rendererAffinityDeviceArray_.empty() &&
                 audioAffinityManager->capturerAffinityDeviceArray_.empty());
    if (!audioAffinityManager->rendererAffinityDeviceArray_.empty()) {
        const auto& outputDevice = audioAffinityManager->rendererAffinityDeviceArray_[0];
        EXPECT_FALSE(outputDevice.groupName.empty());
        EXPECT_EQ(outputDevice.deviceFlag, DeviceFlag::OUTPUT_DEVICES_FLAG);
    }
    if (!audioAffinityManager->capturerAffinityDeviceArray_.empty()) {
        const auto& inputDevice = audioAffinityManager->capturerAffinityDeviceArray_[0];
        EXPECT_FALSE(inputDevice.groupName.empty());
        EXPECT_EQ(inputDevice.deviceFlag, DeviceFlag::INPUT_DEVICES_FLAG);
    }
}

/**
* @tc.name  : Test AudioAffnityManager.
* @tc.number: AudioAffnityManager_002
* @tc.desc  : Test OnXmlParsingCompleted.
*/
HWTEST_F(AudioAffinityManagerUnitTest, AudioAffnityManager_002, TestSize.Level1)
{
    std::unique_ptr <AudioAffinityManager> audioAffinityManager = std::make_unique<AudioAffinityManager>();
    // Test Data
    std::vector<AffinityDeviceInfo> xmlData = {
        {"group1", DeviceType::DEVICE_TYPE_SPEAKER, DeviceFlag::OUTPUT_DEVICES_FLAG, "network1", 0, true, true},
        {"group2", DeviceType::DEVICE_TYPE_MIC, DeviceFlag::INPUT_DEVICES_FLAG, "network2", 0, false, false}
    };
    audioAffinityManager->OnXmlParsingCompleted(xmlData);
    EXPECT_EQ(audioAffinityManager->rendererAffinityDeviceArray_.size(), 1);
    EXPECT_EQ(audioAffinityManager->capturerAffinityDeviceArray_.size(), 1);
    const auto& outputDevice = audioAffinityManager->rendererAffinityDeviceArray_[0];
    EXPECT_EQ(outputDevice.groupName, "group1");
    EXPECT_EQ(outputDevice.deviceType, DeviceType::DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(outputDevice.deviceFlag, DeviceFlag::OUTPUT_DEVICES_FLAG);
    EXPECT_EQ(outputDevice.networkID, "network1");
    EXPECT_TRUE(outputDevice.isPrimary);
    EXPECT_TRUE(outputDevice.SupportedConcurrency);
    const auto& inputDevice = audioAffinityManager->capturerAffinityDeviceArray_[0];
    EXPECT_EQ(inputDevice.groupName, "group2");
    EXPECT_EQ(inputDevice.deviceType, DeviceType::DEVICE_TYPE_MIC);
    EXPECT_EQ(inputDevice.deviceFlag, DeviceFlag::INPUT_DEVICES_FLAG);
    EXPECT_EQ(inputDevice.networkID, "network2");
    EXPECT_FALSE(inputDevice.isPrimary);
    EXPECT_FALSE(inputDevice.SupportedConcurrency);
}

/**
* @tc.name  : Test AudioAffnityManager.
* @tc.number: AudioAffnityManager_003
* @tc.desc  : Test OnXmlParsingCompleted.
*/
HWTEST_F(AudioAffinityManagerUnitTest, AudioAffnityManager_003, TestSize.Level1)
{
    std::unique_ptr <AudioAffinityManager> audioAffinityManager = std::make_unique<AudioAffinityManager>();
    // Test empty data
    std::vector<AffinityDeviceInfo> emptyXmlData;
    audioAffinityManager->OnXmlParsingCompleted(emptyXmlData);
    EXPECT_TRUE(audioAffinityManager->rendererAffinityDeviceArray_.empty());
    EXPECT_TRUE(audioAffinityManager->capturerAffinityDeviceArray_.empty());
}

/**
* @tc.name  : Test AudioAffnityManager.
* @tc.number: AudioAffnityManager_004
* @tc.desc  : Test OnXmlParsingCompleted.
*/
HWTEST_F(AudioAffinityManagerUnitTest, AudioAffnityManager_004, TestSize.Level1)
{
    std::unique_ptr <AudioAffinityManager> audioAffinityManager = std::make_unique<AudioAffinityManager>();
    // Test not exising clientUID
    int32_t nonExistentClientUID = 9999;
    std::shared_ptr<AudioDeviceDescriptor> result = audioAffinityManager->GetRendererDevice(nonExistentClientUID);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->getType(), DeviceType::DEVICE_TYPE_NONE);
    EXPECT_EQ(result->getRole(), DeviceRole::DEVICE_ROLE_NONE);
}

/**
* @tc.name  : Test AudioAffnityManager.
* @tc.number: AudioAffnityManager_005
* @tc.desc  : Test GetRendererDevice.
*/
HWTEST_F(AudioAffinityManagerUnitTest, AudioAffnityManager_005, TestSize.Level1)
{
    std::unique_ptr <AudioAffinityManager> audioAffinityManager = std::make_unique<AudioAffinityManager>();
    int32_t testClientUID = 1000;
    DeviceType testDeviceType = DeviceType::DEVICE_TYPE_SPEAKER;
    DeviceRole testDeviceRole = DeviceRole::OUTPUT_DEVICE;
    int32_t testInterruptGroupId = 1;
    int32_t testVolumeGroupId = 1;
    std::string testNetworkId = "test_network";
    std::shared_ptr<AudioDeviceDescriptor> testDescriptor = std::make_shared<AudioDeviceDescriptor>(
        testDeviceType, testDeviceRole, testInterruptGroupId, testVolumeGroupId, testNetworkId);
    audioAffinityManager->activeRendererDeviceMap_[testClientUID] = testDescriptor;
    std::shared_ptr<AudioDeviceDescriptor> result = audioAffinityManager->GetRendererDevice(testClientUID);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->getType(), testDeviceType);
    EXPECT_EQ(result->getRole(), testDeviceRole);
    EXPECT_EQ(result->interruptGroupId_, testInterruptGroupId);
    EXPECT_EQ(result->volumeGroupId_, testVolumeGroupId);
    EXPECT_EQ(result->networkId_, testNetworkId);
}

/**
* @tc.name  : Test AudioAffnityManager.
* @tc.number: AudioAffnityManager_006
* @tc.desc  : Test GetRendererDevice.
*/
HWTEST_F(AudioAffinityManagerUnitTest, AudioAffnityManager_006, TestSize.Level1)
{
    std::unique_ptr <AudioAffinityManager> audioAffinityManager = std::make_unique<AudioAffinityManager>();
    // Test no existing clientUID
    int32_t nonExistentClientUID = 9999;
    std::shared_ptr<AudioDeviceDescriptor> result = audioAffinityManager->GetRendererDevice(nonExistentClientUID);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->getType(), DeviceType::DEVICE_TYPE_NONE);
    EXPECT_EQ(result->getRole(), DeviceRole::DEVICE_ROLE_NONE);
}

/**
* @tc.name  : Test AudioAffnityManager.
* @tc.number: AudioAffnityManager_007
* @tc.desc  : Test GetRendererDevice.
*/
HWTEST_F(AudioAffinityManagerUnitTest, AudioAffnityManager_007, TestSize.Level1)
{
    std::unique_ptr <AudioAffinityManager> audioAffinityManager = std::make_unique<AudioAffinityManager>();
    // Add a null pointer to the map
    int32_t testClientUID = 2000;
    audioAffinityManager->activeRendererDeviceMap_[testClientUID] = nullptr;
    std::shared_ptr<AudioDeviceDescriptor> result = audioAffinityManager->GetRendererDevice(testClientUID);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->getType(), DeviceType::DEVICE_TYPE_NONE);
    EXPECT_EQ(result->getRole(), DeviceRole::DEVICE_ROLE_NONE);
}

/**
* @tc.name  : Test AudioAffnityManager.
* @tc.number: AudioAffnityManager_008
* @tc.desc  : Test GetCapturerDevice.
*/
HWTEST_F(AudioAffinityManagerUnitTest, AudioAffnityManager_008, TestSize.Level1)
{
    std::unique_ptr <AudioAffinityManager> audioAffinityManager = std::make_unique<AudioAffinityManager>();
    int32_t clientUID = 1000;
    std::shared_ptr<AudioDeviceDescriptor> result = audioAffinityManager->GetCapturerDevice(clientUID);
    EXPECT_NE(nullptr, result);
    // For non-existing clientUID, expect a new empty AudioDeviceDescriptor
    EXPECT_NE(result->networkId_, "");
    EXPECT_EQ(result->deviceRole_, DeviceRole::DEVICE_ROLE_NONE);
    EXPECT_EQ(result->deviceType_, DeviceType::DEVICE_TYPE_NONE);
}

/**
* @tc.name  : Test AudioAffnityManager.
* @tc.number: AudioAffnityManager_009
* @tc.desc  : Test GetCapturerDevice.
*/
HWTEST_F(AudioAffinityManagerUnitTest, AudioAffnityManager_009, TestSize.Level1)
{
    std::unique_ptr <AudioAffinityManager> audioAffinityManager = std::make_unique<AudioAffinityManager>();
    int32_t clientUID = 1001;
    audioAffinityManager->activeCapturerDeviceMap_[clientUID] = nullptr;
    std::shared_ptr<AudioDeviceDescriptor> result = audioAffinityManager->GetCapturerDevice(clientUID);
    EXPECT_NE(nullptr, result);
    // For null descriptor, expect a new empty AudioDeviceDescriptor
    EXPECT_NE(result->networkId_, "");
    EXPECT_EQ(result->deviceRole_, DeviceRole::DEVICE_ROLE_NONE);
    EXPECT_EQ(result->deviceType_, DeviceType::DEVICE_TYPE_NONE);
}

/**
* @tc.name  : Test AudioAffnityManager.
* @tc.number: AudioAffnityManager_010
* @tc.desc  : Test GetCapturerDevice.
*/
HWTEST_F(AudioAffinityManagerUnitTest, AudioAffnityManager_010, TestSize.Level1)
{
    std::unique_ptr <AudioAffinityManager> audioAffinityManager = std::make_unique<AudioAffinityManager>();
    int32_t clientUID = 1002;
    std::shared_ptr<AudioDeviceDescriptor> descriptor = std::make_shared<AudioDeviceDescriptor>();
    descriptor->networkId_ = "test_network";
    descriptor->deviceRole_ = DeviceRole::INPUT_DEVICE;
    descriptor->deviceType_ = DeviceType::DEVICE_TYPE_MIC;
    audioAffinityManager->activeCapturerDeviceMap_[clientUID] = descriptor;
    std::shared_ptr<AudioDeviceDescriptor> result = audioAffinityManager->GetCapturerDevice(clientUID);
    EXPECT_NE(nullptr, result);
    // Verify the returned descriptor matches the original
    EXPECT_EQ(result->networkId_, "test_network");
    EXPECT_EQ(result->deviceRole_, DeviceRole::INPUT_DEVICE);
    EXPECT_EQ(result->deviceType_, DeviceType::DEVICE_TYPE_MIC);
}

/**
* @tc.name  : Test AudioAffnityManager.
* @tc.number: AudioAffnityManager_011
* @tc.desc  : Test GetCapturerDevice.
*/
HWTEST_F(AudioAffinityManagerUnitTest, AudioAffnityManager_011, TestSize.Level1)
{
    std::unique_ptr <AudioAffinityManager> audioAffinityManager = std::make_unique<AudioAffinityManager>();
    int32_t clientUID = INT32_MAX;
    auto result = audioAffinityManager->GetCapturerDevice(clientUID);
    EXPECT_NE(nullptr, result);
    // For non-existing clientUID, expect a new empty AudioDeviceDescriptor
    EXPECT_NE(result->networkId_, "");
    EXPECT_EQ(result->deviceRole_, DeviceRole::DEVICE_ROLE_NONE);
    EXPECT_EQ(result->deviceType_, DeviceType::DEVICE_TYPE_NONE);
}

/**
* @tc.name  : Test AudioAffnityManager.
* @tc.number: AudioAffnityManager_012
* @tc.desc  : Test DelSelectRendererDevice.
*/
HWTEST_F(AudioAffinityManagerUnitTest, AudioAffnityManager_012, TestSize.Level1)
{
    std::unique_ptr <AudioAffinityManager> audioAffinityManager = std::make_unique<AudioAffinityManager>();
    int32_t clientUID = 1000;
    std::shared_ptr<AudioDeviceDescriptor> descriptor = std::make_shared<AudioDeviceDescriptor>();
    // Verify initial state
    EXPECT_EQ(audioAffinityManager->activeRendererDeviceMap_.count(clientUID), 0);
    // Execute deletion
    audioAffinityManager->DelSelectRendererDevice(clientUID);
    // Verify nothing changed
    EXPECT_EQ(audioAffinityManager->activeRendererDeviceMap_.count(clientUID), 0);
}

/**
* @tc.name  : Test AudioAffnityManager.
* @tc.number: AudioAffnityManager_013
* @tc.desc  : Test DelSelectRendererDevice.
*/
HWTEST_F(AudioAffinityManagerUnitTest, AudioAffnityManager_013, TestSize.Level1)
{
    std::unique_ptr <AudioAffinityManager> audioAffinityManager = std::make_unique<AudioAffinityManager>();
    int32_t clientUID = 1001;
    std::shared_ptr<AudioDeviceDescriptor> descriptor = std::make_shared<AudioDeviceDescriptor>();
    // Initialize with null descriptor
    audioAffinityManager->activeRendererDeviceMap_[clientUID] = nullptr;
    // Execute deletion
    audioAffinityManager->DelSelectRendererDevice(clientUID);
    // Verify map still contains the entry (due to early return in CHECK_AND_RETURN_LOG)
    EXPECT_EQ(audioAffinityManager->activeRendererDeviceMap_.count(clientUID), 1);
}

/**
* @tc.name  : Test AudioAffnityManager.
* @tc.number: AudioAffnityManager_014
* @tc.desc  : Test DelSelectRendererDevice.
*/
HWTEST_F(AudioAffinityManagerUnitTest, AudioAffnityManager_014, TestSize.Level1)
{
    std::unique_ptr <AudioAffinityManager> audioAffinityManager = std::make_unique<AudioAffinityManager>();
    int32_t clientUID = 1002;
    std::shared_ptr<AudioDeviceDescriptor> descriptor = std::make_shared<AudioDeviceDescriptor>();
    std::string networkId = "test_network";
    std::string groupName = "test_group";
    // Verify initial state
    EXPECT_NE(audioAffinityManager->activeRendererDeviceMap_.count(clientUID), 1);
    EXPECT_FALSE(audioAffinityManager->activeRendererGroupAffinityMap_.count(groupName) > 0);
    // Execute deletion
    audioAffinityManager->DelSelectRendererDevice(clientUID);
    // Verify device was removed from activeRendererDeviceMap
    EXPECT_EQ(audioAffinityManager->activeRendererDeviceMap_.count(clientUID), 0);
    // Verify client was removed from group affinity map
    if (audioAffinityManager->activeRendererGroupAffinityMap_.count(groupName) > 0) {
        EXPECT_EQ(audioAffinityManager->activeRendererGroupAffinityMap_[groupName].count(clientUID), 0);
    }
}

/**
* @tc.name  : Test AudioAffnityManager.
* @tc.number: AudioAffnityManager_015
* @tc.desc  : Test DelSelectRendererDevice.
*/
HWTEST_F(AudioAffinityManagerUnitTest, AudioAffnityManager_015, TestSize.Level1)
{
    std::unique_ptr <AudioAffinityManager> audioAffinityManager = std::make_unique<AudioAffinityManager>();
    int32_t clientUID1 = 1003;
    int32_t clientUID2 = 1004;
    std::shared_ptr<AudioDeviceDescriptor> descriptor = std::make_shared<AudioDeviceDescriptor>();
    std::string networkId = "test_network";
    std::string groupName = "test_group";
    // Verify initial state
    EXPECT_NE(audioAffinityManager->activeRendererDeviceMap_.count(clientUID1), 1);
    EXPECT_NE(audioAffinityManager->activeRendererDeviceMap_.count(clientUID2), 1);
    EXPECT_FALSE(audioAffinityManager->activeRendererGroupAffinityMap_[groupName].count(clientUID1) > 0);
    EXPECT_FALSE(audioAffinityManager->activeRendererGroupAffinityMap_[groupName].count(clientUID2) > 0);
    // Delete first client
    audioAffinityManager->DelSelectRendererDevice(clientUID1);
    // Verify clientUID1 was removed but clientUID2 remains
    EXPECT_EQ(audioAffinityManager->activeRendererDeviceMap_.count(clientUID1), 0);
    EXPECT_NE(audioAffinityManager->activeRendererDeviceMap_.count(clientUID2), 1);
    EXPECT_EQ(audioAffinityManager->activeRendererGroupAffinityMap_[groupName].count(clientUID1), 0);
    EXPECT_FALSE(audioAffinityManager->activeRendererGroupAffinityMap_[groupName].count(clientUID2) > 0);
}

/**
* @tc.name  : Test AudioAffnityManager.
* @tc.number: AudioAffnityManager_016
* @tc.desc  : Test DelSelectCapturerDevice.
*/
HWTEST_F(AudioAffinityManagerUnitTest, AudioAffnityManager_016, TestSize.Level1)
{
    std::unique_ptr <AudioAffinityManager> audioAffinityManager = std::make_unique<AudioAffinityManager>();
    int32_t clientUID = 1000;
    EXPECT_EQ(audioAffinityManager->activeCapturerDeviceMap_.count(clientUID), 0);
    audioAffinityManager->DelSelectCapturerDevice(clientUID);
    EXPECT_EQ(audioAffinityManager->activeCapturerDeviceMap_.count(clientUID), 0);
}

/**
* @tc.name  : Test AudioAffnityManager.
* @tc.number: AudioAffnityManager_017
* @tc.desc  : Test DelSelectCapturerDevice.
*/
HWTEST_F(AudioAffinityManagerUnitTest, AudioAffnityManager_017, TestSize.Level1)
{
    std::unique_ptr <AudioAffinityManager> audioAffinityManager = std::make_unique<AudioAffinityManager>();
    int32_t clientUID = 1001;
    audioAffinityManager->activeCapturerDeviceMap_[clientUID] = nullptr;
    audioAffinityManager->DelSelectCapturerDevice(clientUID);
    EXPECT_EQ(audioAffinityManager->activeCapturerDeviceMap_.count(clientUID), 1);
}

/**
* @tc.name  : Test AudioAffnityManager.
* @tc.number: AudioAffnityManager_018
* @tc.desc  : Test DelSelectCapturerDevice.
*/
HWTEST_F(AudioAffinityManagerUnitTest, AudioAffnityManager_018, TestSize.Level1)
{
    std::unique_ptr <AudioAffinityManager> audioAffinityManager = std::make_unique<AudioAffinityManager>();
    int32_t clientUID = 1002;
    std::string networkId = "test_network";
    std::string groupName = "test_group";
    EXPECT_NE(audioAffinityManager->activeCapturerDeviceMap_.count(clientUID), 1);
    EXPECT_FALSE(audioAffinityManager->activeCapturerGroupAffinityMap_.count(groupName) > 0);
    audioAffinityManager->DelSelectCapturerDevice(clientUID);
    EXPECT_EQ(audioAffinityManager->activeCapturerDeviceMap_.count(clientUID), 0);
    if (audioAffinityManager->activeCapturerGroupAffinityMap_.count(groupName) > 0) {
        EXPECT_EQ(audioAffinityManager->activeCapturerGroupAffinityMap_[groupName].count(clientUID), 0);
    }
}

/**
* @tc.name  : Test AudioAffnityManager.
* @tc.number: AudioAffnityManager_019
* @tc.desc  : Test DelSelectCapturerDevice.
*/
HWTEST_F(AudioAffinityManagerUnitTest, AudioAffnityManager_019, TestSize.Level1)
{
    std::unique_ptr <AudioAffinityManager> audioAffinityManager = std::make_unique<AudioAffinityManager>();
    int32_t clientUID1 = 1003;
    int32_t clientUID2 = 1004;
    std::string networkId = "test_network";
    std::string groupName = "test_group";
    EXPECT_NE(audioAffinityManager->activeCapturerDeviceMap_.count(clientUID1), 1);
    EXPECT_NE(audioAffinityManager->activeCapturerDeviceMap_.count(clientUID2), 1);
    EXPECT_FALSE(audioAffinityManager->activeCapturerGroupAffinityMap_[groupName].count(clientUID1) > 0);
    EXPECT_FALSE(audioAffinityManager->activeCapturerGroupAffinityMap_[groupName].count(clientUID2) > 0);
    audioAffinityManager->DelSelectCapturerDevice(clientUID1);
    EXPECT_EQ(audioAffinityManager->activeCapturerDeviceMap_.count(clientUID1), 0);
    EXPECT_NE(audioAffinityManager->activeCapturerDeviceMap_.count(clientUID2), 1);
    EXPECT_EQ(audioAffinityManager->activeCapturerGroupAffinityMap_[groupName].count(clientUID1), 0);
    EXPECT_FALSE(audioAffinityManager->activeCapturerGroupAffinityMap_[groupName].count(clientUID2) > 0);
}

/**
* @tc.name  : Test AudioAffnityManager.
* @tc.number: AudioAffnityManager_020
* @tc.desc  : Test RemoveOfflineRendererDevice.
*/
HWTEST_F(AudioAffinityManagerUnitTest, AudioAffnityManager_020, TestSize.Level1)
{
    AudioAffinityManager affinityManager;
    // Create test AudioDeviceDescriptor
    DeviceType testDeviceType = DEVICE_TYPE_SPEAKER;
    DeviceRole testDeviceRole = OUTPUT_DEVICE;
    int32_t testInterruptGroupId = 1;
    int32_t testVolumeGroupId = 1;
    std::string testNetworkID = "test_network_id";

    AudioDeviceDescriptor updateDesc(
        testDeviceType,
        testDeviceRole,
        testInterruptGroupId,
        testVolumeGroupId,
        testNetworkID
    );
    // Add test equipment to the activeRendererDeviceMap_
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>(updateDesc);
    int32_t testClientUID = 1000;
    affinityManager.activeRendererDeviceMap_[testClientUID] = deviceDesc;
    // Add AffinityDeviceInfo to rendererAffinityDeviceArray_
    AffinityDeviceInfo affinityInfo;
    affinityInfo.deviceType = testDeviceType;
    affinityInfo.networkID = testNetworkID;
    affinityInfo.groupName = "test_group";
    affinityManager.rendererAffinityDeviceArray_.push_back(affinityInfo);
    affinityManager.RemoveOfflineRendererDevice(updateDesc);
    // Verify that the device was successfully removed
    auto it = affinityManager.activeRendererDeviceMap_.find(testClientUID);
    EXPECT_EQ(it, affinityManager.activeRendererDeviceMap_.end());
    // Verify activeRendererDeviceMap_ is empty
    EXPECT_TRUE(affinityManager.activeRendererDeviceMap_.empty());
}

/**
* @tc.name  : Test AudioAffnityManager.
* @tc.number: AudioAffnityManager_021
* @tc.desc  : Test RemoveOfflineCapturerDevice.
*/
HWTEST_F(AudioAffinityManagerUnitTest, AudioAffnityManager_021, TestSize.Level1)
{
    AudioAffinityManager affinityManager;
    // Create Test AudioDeviceDescriptor
    DeviceType testDeviceType = DEVICE_TYPE_MIC;
    DeviceRole testDeviceRole = OUTPUT_DEVICE;
    int32_t testInterruptGroupId = 2;
    int32_t testVolumeGroupId = 2;
    std::string testNetworkID = "test_capturer_network_id";
    AudioDeviceDescriptor updateDesc(
        testDeviceType,
        testDeviceRole,
        testInterruptGroupId,
        testVolumeGroupId,
        testNetworkID
    );
    // Add test equipment to the activeCapturerDeviceMap_
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>(updateDesc);
    int32_t testClientUID = 2000;
    affinityManager.activeCapturerDeviceMap_[testClientUID] = deviceDesc;
    // Add AffinityDeviceInfo to capturerAffinityDeviceArray_
    AffinityDeviceInfo affinityInfo;
    affinityInfo.deviceType = testDeviceType;
    affinityInfo.networkID = testNetworkID;
    affinityInfo.groupName = "test_capturer_group";
    affinityManager.capturerAffinityDeviceArray_.push_back(affinityInfo);
    affinityManager.RemoveOfflineCapturerDevice(updateDesc);
    // Verify that the device was successfully removed
    auto it = affinityManager.activeCapturerDeviceMap_.find(testClientUID);
    EXPECT_EQ(it, affinityManager.activeCapturerDeviceMap_.end());
    // Verify activeCapturerDeviceMap_ is empty
    EXPECT_TRUE(affinityManager.activeCapturerDeviceMap_.empty());
}

/**
* @tc.name  : Test AudioAffnityManager.
* @tc.number: AudioAffnityManager_022
* @tc.desc  : Test GetAffinityDeviceInfoByDeviceType.
*/
HWTEST_F(AudioAffinityManagerUnitTest, AudioAffnityManager_022, TestSize.Level1)
{
    std::unique_ptr <AudioAffinityManager> audioAffinityManager = std::make_unique<AudioAffinityManager>();
    DeviceType targetType = DEVICE_TYPE_SPEAKER;
    std::string targetNetwork = "network1";
    AffinityDeviceInfo result = audioAffinityManager->GetAffinityDeviceInfoByDeviceType(
        testDevices_, targetType, targetNetwork);
    EXPECT_EQ(result.deviceType, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(result.networkID, "network1");
    EXPECT_EQ(result.groupName, "group1");
    EXPECT_TRUE(result.isPrimary);
}

/**
* @tc.name  : Test AudioAffnityManager.
* @tc.number: AudioAffnityManager_023
* @tc.desc  : Test GetAffinityDeviceInfoByDeviceType.
*/
HWTEST_F(AudioAffinityManagerUnitTest, AudioAffnityManager_023, TestSize.Level1)
{
    std::unique_ptr <AudioAffinityManager> audioAffinityManager = std::make_unique<AudioAffinityManager>();
    DeviceType targetType =  DEVICE_TYPE_SPEAKER;
    std::string targetNetwork = "network1";
    AffinityDeviceInfo result = audioAffinityManager->GetAffinityDeviceInfoByDeviceType(
        testDevices_, targetType, targetNetwork);
    EXPECT_NE(result.deviceType, DEVICE_TYPE_NONE);
    EXPECT_NE(result.networkID, "");
    EXPECT_NE(result.groupName, "");
    EXPECT_TRUE(result.isPrimary);
}

/**
* @tc.name  : Test AudioAffnityManager.
* @tc.number: AudioAffnityManager_024
* @tc.desc  : Test GetActiveAffinityDeviceMapByGroupName.
*/
HWTEST_F(AudioAffinityManagerUnitTest, AudioAffnityManager_024, TestSize.Level1)
{
    std::unique_ptr <AudioAffinityManager> audioAffinityManager = std::make_unique<AudioAffinityManager>();
    std::string targetGroup = "group1";
    auto result = audioAffinityManager->GetActiveAffinityDeviceMapByGroupName(testActiveGroupMap_, targetGroup);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[1].deviceType, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(result[2].deviceType, DEVICE_TYPE_BLUETOOTH_A2DP);
    EXPECT_EQ(result[1].groupName, "group1");
    EXPECT_TRUE(result[1].isPrimary);
}

/**
* @tc.name  : Test AudioAffnityManager.
* @tc.number: AudioAffnityManager_025
* @tc.desc  : Test GetActiveAffinityDeviceMapByGroupName.
*/
HWTEST_F(AudioAffinityManagerUnitTest, AudioAffnityManager_025, TestSize.Level1)
{
    std::unique_ptr <AudioAffinityManager> audioAffinityManager = std::make_unique<AudioAffinityManager>();
    std::string targetGroup = "group2";
    auto result = audioAffinityManager->GetActiveAffinityDeviceMapByGroupName(testActiveGroupMap_, targetGroup);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[3].deviceType, DEVICE_TYPE_WIRED_HEADSET);
    EXPECT_EQ(result[3].groupName, "group2");
    EXPECT_TRUE(result[3].isPrimary);
    EXPECT_FALSE(result[3].SupportedConcurrency);
}

/**
* @tc.name  : Test AudioAffnityManager.
* @tc.number: AudioAffnityManager_026
* @tc.desc  : Test GetActiveAffinityDeviceMapByGroupName.
*/
HWTEST_F(AudioAffinityManagerUnitTest, AudioAffnityManager_026, TestSize.Level1)
{
    std::unique_ptr <AudioAffinityManager> audioAffinityManager = std::make_unique<AudioAffinityManager>();
    std::string targetGroup = "nonexistentgroup";
    auto result = audioAffinityManager->GetActiveAffinityDeviceMapByGroupName(testActiveGroupMap_, targetGroup);
    EXPECT_TRUE(result.empty());
}

/**
* @tc.name  : Test AudioAffnityManager.
* @tc.number: AudioAffnityManager_027
* @tc.desc  : Test GetActiveAffinityDeviceMapByGroupName.
*/
HWTEST_F(AudioAffinityManagerUnitTest, AudioAffnityManager_027, TestSize.Level1)
{
    std::unique_ptr <AudioAffinityManager> audioAffinityManager = std::make_unique<AudioAffinityManager>();
    std::string targetGroup = "";
    auto result = audioAffinityManager->GetActiveAffinityDeviceMapByGroupName(testActiveGroupMap_, targetGroup);
    EXPECT_TRUE(result.empty());
}

/**
* @tc.name  : Test AudioAffnityManager.
* @tc.number: AudioAffnityManager_028
* @tc.desc  : Test GetActiveAffinityDeviceMapByGroupName.
*/
HWTEST_F(AudioAffinityManagerUnitTest, AudioAffnityManager_028, TestSize.Level1)
{
    std::unique_ptr <AudioAffinityManager> audioAffinityManager = std::make_unique<AudioAffinityManager>();
    AFFINITYDEVINFOMAP emptyMap;
    std::string targetGroup = "group1";
    auto result = audioAffinityManager->GetActiveAffinityDeviceMapByGroupName(emptyMap, targetGroup);
    EXPECT_TRUE(result.empty());
}

/**
* @tc.name  : Test AudioAffnityManager.
* @tc.number: AudioAffnityManager_029
* @tc.desc  : Test GetAffinityClientUID.
*/
HWTEST_F(AudioAffinityManagerUnitTest, AudioAffnityManager_029, TestSize.Level1)
{
    AudioAffinityManager affinityManager;
    std::unordered_map<int32_t, AffinityDeviceInfo> affinityDeviceInfoMap;
    AffinityDeviceInfo deviceInfo1;
    deviceInfo1.SupportedConcurrency = true;
    deviceInfo1.chooseTimeStamp = 100;
    int32_t clientUID = 1001;
    affinityDeviceInfoMap[clientUID] = deviceInfo1;
    int32_t result = affinityManager.GetAffinityClientUID(clientUID, affinityDeviceInfoMap);
    EXPECT_EQ(result, clientUID);
}

/**
* @tc.name  : Test AudioAffnityManager.
* @tc.number: AudioAffnityManager_030
* @tc.desc  : Test GetAffinityClientUID.
*/
HWTEST_F(AudioAffinityManagerUnitTest, AudioAffnityManager_030, TestSize.Level1)
{
    AudioAffinityManager affinityManager;
    std::unordered_map<int32_t, AffinityDeviceInfo> affinityDeviceInfoMap;
    AffinityDeviceInfo deviceInfo1;
    deviceInfo1.SupportedConcurrency = false;
    deviceInfo1.chooseTimeStamp = 100;
    AffinityDeviceInfo deviceInfo2;
    deviceInfo2.SupportedConcurrency = false;
    deviceInfo2.chooseTimeStamp = 200;
    affinityDeviceInfoMap[1001] = deviceInfo1;
    affinityDeviceInfoMap[1002] = deviceInfo2;
    int32_t result = affinityManager.GetAffinityClientUID(1003, affinityDeviceInfoMap);
    EXPECT_EQ(result, 1002);  // Should return client with highest timestamp
}

/**
* @tc.name  : Test AudioAffnityManager.
* @tc.number: AudioAffnityManager_031
* @tc.desc  : Test GetAffinityClientUID.
*/
HWTEST_F(AudioAffinityManagerUnitTest, AudioAffnityManager_031, TestSize.Level1)
{
    AudioAffinityManager affinityManager;
    std::unordered_map<int32_t, AffinityDeviceInfo> affinityDeviceInfoMap;
    AffinityDeviceInfo deviceInfo1;
    deviceInfo1.SupportedConcurrency = false;
    deviceInfo1.chooseTimeStamp = 100;
    AffinityDeviceInfo deviceInfo2;
    deviceInfo2.SupportedConcurrency = false;
    deviceInfo2.chooseTimeStamp = 200;
    int32_t clientUID = 1001;
    affinityDeviceInfoMap[clientUID] = deviceInfo1;
    affinityDeviceInfoMap[1002] = deviceInfo2;
    int32_t result = affinityManager.GetAffinityClientUID(clientUID, affinityDeviceInfoMap);
    EXPECT_EQ(result, 1002);
}

/**
* @tc.name  : Test AudioAffnityManager.
* @tc.number: AudioAffnityManager_032
* @tc.desc  : Test GetAffinityClientUID.
*/
HWTEST_F(AudioAffinityManagerUnitTest, AudioAffnityManager_032, TestSize.Level1)
{
    AudioAffinityManager affinityManager;
    std::unordered_map<int32_t, AffinityDeviceInfo> affinityDeviceInfoMap;
    int32_t result = affinityManager.GetAffinityClientUID(1001, affinityDeviceInfoMap);
    EXPECT_EQ(result, 0);
}

/**
* @tc.name  : Test AudioAffnityManager.
* @tc.number: AudioAffnityManager_033
* @tc.desc  : Test GetAffinityClientUID.
*/
HWTEST_F(AudioAffinityManagerUnitTest, AudioAffnityManager_033, TestSize.Level1)
{
    AudioAffinityManager affinityManager;
    std::unordered_map<int32_t, AffinityDeviceInfo> affinityDeviceInfoMap;
    AffinityDeviceInfo deviceInfo1;
    deviceInfo1.SupportedConcurrency = true;
    deviceInfo1.chooseTimeStamp = 100;
    AffinityDeviceInfo deviceInfo2;
    deviceInfo2.SupportedConcurrency = true;
    deviceInfo2.chooseTimeStamp = 200;
    affinityDeviceInfoMap[1001] = deviceInfo1;
    affinityDeviceInfoMap[1002] = deviceInfo2;
    int32_t result = affinityManager.GetAffinityClientUID(1003, affinityDeviceInfoMap);
    EXPECT_EQ(result, 0);
}

/**
* @tc.name  : Test AudioAffnityManager.
* @tc.number: AudioAffnityManager_034
* @tc.desc  : Test DelActiveGroupAffinityMap.
*/
HWTEST_F(AudioAffinityManagerUnitTest, AudioAffnityManager_034, TestSize.Level1)
{
    AudioAffinityManager affinityManager;
    AffinityDeviceInfo deviceInfo1, deviceInfo2;
    std::unordered_map<int32_t, AffinityDeviceInfo> testDeviceInfoMap;
    PrepareTestData(deviceInfo1, deviceInfo2, testDeviceInfoMap);
    int32_t clientUID = 1001;
    affinityManager.DelActiveGroupAffinityMap(clientUID, testDeviceInfoMap);
    EXPECT_EQ(testDeviceInfoMap.find(clientUID), testDeviceInfoMap.end());
    auto remainingItem = testDeviceInfoMap.find(1002);
    EXPECT_EQ(remainingItem, testDeviceInfoMap.end());
}

/**
* @tc.name  : Test AudioAffnityManager.
* @tc.number: AudioAffnityManager_035
* @tc.desc  : Test DelActiveGroupAffinityMap.
*/
HWTEST_F(AudioAffinityManagerUnitTest, AudioAffnityManager_035, TestSize.Level1)
{
    AudioAffinityManager affinityManager;
    AffinityDeviceInfo deviceInfo1, deviceInfo2;
    std::unordered_map<int32_t, AffinityDeviceInfo> testDeviceInfoMap;
    PrepareTestData(deviceInfo1, deviceInfo2, testDeviceInfoMap);
    int32_t clientUID = 1001;
    affinityManager.DelActiveGroupAffinityMap(clientUID, testDeviceInfoMap);
    EXPECT_EQ(testDeviceInfoMap.find(clientUID), testDeviceInfoMap.end());
    auto remainingItem = testDeviceInfoMap.find(1002);
    EXPECT_EQ(remainingItem, testDeviceInfoMap.end());
}

/**
* @tc.name  : Test AudioAffnityManager.
* @tc.number: AudioAffnityManager_036
* @tc.desc  : Test DelActiveGroupAffinityMap.
*/
HWTEST_F(AudioAffinityManagerUnitTest, AudioAffnityManager_036, TestSize.Level1)
{
    AudioAffinityManager affinityManager;
    AffinityDeviceInfo deviceInfo1, deviceInfo2;
    std::unordered_map<int32_t, AffinityDeviceInfo> testDeviceInfoMap;
    PrepareTestData(deviceInfo1, deviceInfo2, testDeviceInfoMap);
    int32_t nonExistingClientUID = 1003;
    affinityManager.DelActiveGroupAffinityMap(nonExistingClientUID, testDeviceInfoMap);
    auto item1 = testDeviceInfoMap.find(1001);
    EXPECT_EQ(item1, testDeviceInfoMap.end());
    auto item2 = testDeviceInfoMap.find(1002);
    EXPECT_EQ(item2, testDeviceInfoMap.end());
}

/**
* @tc.name  : Test AudioAffnityManager.
* @tc.number: AudioAffnityManager_037
* @tc.desc  : Test DelActiveGroupAffinityMap.
*/
HWTEST_F(AudioAffinityManagerUnitTest, AudioAffnityManager_037, TestSize.Level1)
{
    AudioAffinityManager affinityManager;
    AffinityDeviceInfo deviceInfo1, deviceInfo2;
    std::unordered_map<int32_t, AffinityDeviceInfo> emptyMap;
    int32_t clientUID = 1001;
    affinityManager.DelActiveGroupAffinityMap(clientUID, emptyMap);
    EXPECT_TRUE(emptyMap.empty());
}

/**
* @tc.name  : Test AudioAffnityManager.
* @tc.number: AudioAffnityManager_038
* @tc.desc  : Test DelActiveGroupAffinityMap.
*/
HWTEST_F(AudioAffinityManagerUnitTest, AudioAffnityManager_038, TestSize.Level1)
{
    AudioAffinityManager affinityManager;
    AffinityDeviceInfo deviceInfo1, deviceInfo2;
    std::unordered_map<int32_t, AffinityDeviceInfo> testDeviceInfoMap;
    PrepareTestData(deviceInfo1, deviceInfo2, testDeviceInfoMap);
    int32_t clientUID = 1001;
    affinityManager.DelActiveGroupAffinityMap(clientUID, testDeviceInfoMap);
    affinityManager.DelActiveGroupAffinityMap(clientUID, testDeviceInfoMap);
    auto remainingItem = testDeviceInfoMap.find(1002);
    EXPECT_EQ(remainingItem, testDeviceInfoMap.end());
}
} // namespace AudioStandard
} // namespace OHOS
