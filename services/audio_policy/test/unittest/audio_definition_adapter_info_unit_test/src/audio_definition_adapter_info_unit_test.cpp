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

#include "audio_definition_adapter_info_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioDefinitionAdapterInfoUnitTest::SetUpTestCase(void) {}
void AudioDefinitionAdapterInfoUnitTest::TearDownTestCase(void) {}
void AudioDefinitionAdapterInfoUnitTest::SetUp(void) {}
void AudioDefinitionAdapterInfoUnitTest::TearDown(void) {}

/**
* @tc.name  : Test AudioDefinitionAdapterInfoUnitTest.
* @tc.number: PolicyAdapterInfo_001
* @tc.desc  : Test GetAdapterType
*/
HWTEST(AudioDefinitionAdapterInfoUnitTest, PolicyAdapterInfo_001, TestSize.Level1)
{
    auto policyAdapter = std::make_shared<PolicyAdapterInfo>();
    EXPECT_NE(policyAdapter, nullptr);

    auto ret = policyAdapter->GetAdapterType(ADAPTER_TYPE_PRIMARY);
    EXPECT_EQ(AudioAdapterType::TYPE_PRIMARY, ret);

    ret = policyAdapter->GetAdapterType(ADAPTER_TYPE_A2DP);
    EXPECT_EQ(AudioAdapterType::TYPE_A2DP, ret);

    ret = policyAdapter->GetAdapterType(ADAPTER_TYPE_HEARING_AID);
    EXPECT_EQ(AudioAdapterType::TYPE_HEARING_AID, ret);

    ret = policyAdapter->GetAdapterType(ADAPTER_TYPE_REMOTE);
    EXPECT_EQ(AudioAdapterType::TYPE_REMOTE_AUDIO, ret);

    ret = policyAdapter->GetAdapterType(ADAPTER_TYPE_FILE);
    EXPECT_EQ(AudioAdapterType::TYPE_FILE_IO, ret);

    ret = policyAdapter->GetAdapterType(ADAPTER_TYPE_USB);
    EXPECT_EQ(AudioAdapterType::TYPE_USB, ret);

    ret = policyAdapter->GetAdapterType(ADAPTER_TYPE_DP);
    EXPECT_EQ(AudioAdapterType::TYPE_DP, ret);

    ret = policyAdapter->GetAdapterType(ADAPTER_TYPE_SLE);
    EXPECT_EQ(AudioAdapterType::TYPE_SLE, ret);

    ret = policyAdapter->GetAdapterType(ADAPTER_TYPE_VA);
    EXPECT_EQ(AudioAdapterType::TYPE_VA, ret);

    ret = policyAdapter->GetAdapterType(ADAPTER_TYPE_ACCESSORY);
    EXPECT_EQ(AudioAdapterType::TYPE_ACCESSORY, ret);

    ret = policyAdapter->GetAdapterType("");
    EXPECT_EQ(AudioAdapterType::TYPE_INVALID, ret);
}

/**
* @tc.name  : Test AudioDefinitionAdapterInfoUnitTest.
* @tc.number: PolicyAdapterInfo_002
* @tc.desc  : Test GetAdapterType
*/
HWTEST(AudioDefinitionAdapterInfoUnitTest, PolicyAdapterInfo_002, TestSize.Level1)
{
    auto policyAdapter = std::make_shared<PolicyAdapterInfo>();
    EXPECT_NE(policyAdapter, nullptr);

    const std::string TEST_PIPE_NAME{"test1"};
    auto pipeInfo = std::make_shared<AdapterPipeInfo>();
    EXPECT_NE(pipeInfo, nullptr);
    pipeInfo->name_ = TEST_PIPE_NAME;

    policyAdapter->pipeInfos.push_back(nullptr);
    policyAdapter->pipeInfos.push_back(pipeInfo);
    auto ret1 = policyAdapter->GetPipeInfoByName(TEST_PIPE_NAME);
    EXPECT_NE(ret1, nullptr);
    auto ret2 = policyAdapter->GetPipeInfoByName("");
    EXPECT_EQ(ret2, nullptr);

    auto deviceInfo = std::make_shared<AdapterDeviceInfo>();
    EXPECT_NE(deviceInfo, nullptr);
    deviceInfo->type_ = DeviceType::DEVICE_TYPE_EARPIECE;
    deviceInfo->role_ = DeviceRole::OUTPUT_DEVICE;
    policyAdapter->deviceInfos.push_back(deviceInfo);
    auto ret3 = policyAdapter->GetDeviceInfoByType(DeviceType::DEVICE_TYPE_EARPIECE, DeviceRole::OUTPUT_DEVICE);
    EXPECT_NE(ret3, nullptr);
    auto ret4 = policyAdapter->GetDeviceInfoByType(DeviceType::DEVICE_TYPE_NONE, DeviceRole::DEVICE_ROLE_NONE);
    EXPECT_EQ(ret4, nullptr);
}

/**
* @tc.name  : Test AudioDefinitionAdapterInfoUnitTest.
* @tc.number: DeviceAdapterInfo_001
* @tc.desc  : Test GetAdapterType
*/
HWTEST(AudioDefinitionAdapterInfoUnitTest, DeviceAdapterInfo_001, TestSize.Level1)
{
    auto deviceAdapter = std::make_shared<AdapterDeviceInfo>();
    EXPECT_NE(deviceAdapter, nullptr);

    auto adapterInfoPtr = std::make_shared<PolicyAdapterInfo>();
    deviceAdapter->adapterInfo_ = adapterInfoPtr;

    const std::string TEST_PIPE_NAME{"PIPE1"};
    deviceAdapter->supportPipes_.push_back(TEST_PIPE_NAME);

    auto pipeInfo = std::make_shared<AdapterPipeInfo>();
    EXPECT_NE(pipeInfo, nullptr);
    pipeInfo->adapterInfo_ = adapterInfoPtr;
    auto pipestreamPtr = std::make_shared<PipeStreamPropInfo>();
    pipestreamPtr->pipeInfo_ = std::make_shared<AdapterPipeInfo>();
    pipestreamPtr->supportDevices_.push_back(TEST_PIPE_NAME);
    pipestreamPtr->supportDevices_.push_back("");

    auto deviceAdapter2 = std::make_shared<AdapterDeviceInfo>();
    deviceAdapter2->name_ = TEST_PIPE_NAME;
    pipestreamPtr->supportDeviceMap_.insert(std::make_pair(DeviceType::DEVICE_TYPE_EARPIECE, deviceAdapter2));
    pipeInfo->streamPropInfos_.push_back(pipestreamPtr);

    pipeInfo->name_ = TEST_PIPE_NAME;
    deviceAdapter->supportPipeMap_.insert(std::make_pair(0, std::make_shared<AdapterPipeInfo>()));
    deviceAdapter->supportPipeMap_.insert(std::make_pair(1, pipeInfo));
}

/**
* @tc.name  : Test AudioDefinitionAdapterInfoUnitTest.
* @tc.number: AudioPolicyConfigData_001
* @tc.desc  : Test GetAdapterType
*/
HWTEST(AudioDefinitionAdapterInfoUnitTest, AudioPolicyConfigData_001, TestSize.Level1)
{
    auto audioPolicyConfigData = std::make_shared<AudioPolicyConfigData>();
    EXPECT_NE(audioPolicyConfigData, nullptr);

    DeviceType type_ = DEVICE_TYPE_REMOTE_CAST;
    DeviceRole role_ = INPUT_DEVICE;
    std::string networkId_ = "abc";
    uint32_t flags = 0;

    std::pair<DeviceType, DeviceRole> deviceMapKey = std::make_pair(DEVICE_TYPE_SPEAKER, role_);
    std::set<std::shared_ptr<AdapterDeviceInfo>> adapterDeviceInfoSet = {};
    audioPolicyConfigData->deviceInfoMap.insert({deviceMapKey, adapterDeviceInfoSet});

    auto ret = audioPolicyConfigData->GetAdapterDeviceInfo(type_, role_, networkId_, flags);
    EXPECT_EQ(ret, nullptr);
}

/**
* @tc.name  : Test AudioDefinitionAdapterInfoUnitTest.
* @tc.number: AudioPolicyConfigData_002
* @tc.desc  : Test GetAdapterType
*/
HWTEST(AudioDefinitionAdapterInfoUnitTest, AudioPolicyConfigData_002, TestSize.Level1)
{
    auto audioPolicyConfigData = std::make_shared<AudioPolicyConfigData>();
    EXPECT_NE(audioPolicyConfigData, nullptr);

    DeviceType type_ = DEVICE_TYPE_REMOTE_CAST;
    DeviceRole role_ = INPUT_DEVICE;
    std::string networkId_ = "abc";
    uint32_t flags = 0;

    std::pair<DeviceType, DeviceRole> deviceMapKey = std::make_pair(DEVICE_TYPE_SPEAKER, role_);

    auto adapterDeviceInfo = std::make_shared<AdapterDeviceInfo>();
    std::set<std::shared_ptr<AdapterDeviceInfo>> adapterDeviceInfoSet;
    adapterDeviceInfoSet.insert(adapterDeviceInfo);

    audioPolicyConfigData->deviceInfoMap.insert({deviceMapKey, adapterDeviceInfoSet});

    auto ret = audioPolicyConfigData->GetAdapterDeviceInfo(type_, role_, networkId_, flags);
    EXPECT_NE(ret, nullptr);
}

/**
* @tc.name  : Test AudioDefinitionAdapterInfoUnitTest.
* @tc.number: AudioPolicyConfigData_003
* @tc.desc  : Test GetAdapterType
*/
HWTEST(AudioDefinitionAdapterInfoUnitTest, AudioPolicyConfigData_003, TestSize.Level1)
{
    auto audioPolicyConfigData = std::make_shared<AudioPolicyConfigData>();
    EXPECT_NE(audioPolicyConfigData, nullptr);

    DeviceType type_ = DEVICE_TYPE_REMOTE_CAST;
    DeviceRole role_ = INPUT_DEVICE;
    std::string networkId_ = "abc";
    uint32_t flags = 0;

    std::pair<DeviceType, DeviceRole> deviceMapKey = std::make_pair(DEVICE_TYPE_SPEAKER, role_);

    auto adapterDeviceInfo = std::make_shared<AdapterDeviceInfo>();
    auto adapterDeviceInfo2 = std::make_shared<AdapterDeviceInfo>();
    std::set<std::shared_ptr<AdapterDeviceInfo>> adapterDeviceInfoSet;

    adapterDeviceInfoSet.insert(adapterDeviceInfo);
    adapterDeviceInfoSet.insert(adapterDeviceInfo2);

    audioPolicyConfigData->deviceInfoMap.insert({deviceMapKey, adapterDeviceInfoSet});

    auto ret = audioPolicyConfigData->GetAdapterDeviceInfo(type_, role_, networkId_, flags);
    EXPECT_EQ(ret, nullptr);
}

/**
* @tc.name  : Test AudioDefinitionAdapterInfoUnitTest.
* @tc.number: AudioPolicyConfigData_004
* @tc.desc  : Test GetAdapterType
*/
HWTEST(AudioDefinitionAdapterInfoUnitTest, AudioPolicyConfigData_004, TestSize.Level1)
{
    auto audioPolicyConfigData = std::make_shared<AudioPolicyConfigData>();
    EXPECT_NE(audioPolicyConfigData, nullptr);

    DeviceType type_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    DeviceRole role_ = INPUT_DEVICE;
    std::string networkId_ = LOCAL_NETWORK_ID;
    uint32_t flags = 0x200;

    std::pair<DeviceType, DeviceRole> deviceMapKey = std::make_pair(DEVICE_TYPE_BLUETOOTH_A2DP, role_);

    auto adapterDeviceInfo = std::make_shared<AdapterDeviceInfo>();
    auto adapterDeviceInfo2 = std::make_shared<AdapterDeviceInfo>();
    std::set<std::shared_ptr<AdapterDeviceInfo>> adapterDeviceInfoSet;

    adapterDeviceInfoSet.insert(adapterDeviceInfo);
    adapterDeviceInfoSet.insert(adapterDeviceInfo2);

    audioPolicyConfigData->deviceInfoMap.insert({deviceMapKey, adapterDeviceInfoSet});

    auto ret = audioPolicyConfigData->GetAdapterDeviceInfo(type_, role_, networkId_, flags);
    EXPECT_EQ(ret, nullptr);
}

/**
* @tc.name  : Test AudioDefinitionAdapterInfoUnitTest.
* @tc.number: AudioPolicyConfigData_005
* @tc.desc  : Test GetAdapterType
*/
HWTEST(AudioDefinitionAdapterInfoUnitTest, AudioPolicyConfigData_005, TestSize.Level1)
{
    auto audioPolicyConfigData = std::make_shared<AudioPolicyConfigData>();
    EXPECT_NE(audioPolicyConfigData, nullptr);

    DeviceType type_ = DEVICE_TYPE_REMOTE_CAST;
    DeviceRole role_ = INPUT_DEVICE;
    std::string networkId_ = LOCAL_NETWORK_ID;
    uint32_t flags = 0;

    std::pair<DeviceType, DeviceRole> deviceMapKey = std::make_pair(DEVICE_TYPE_SPEAKER, role_);

    auto adapterDeviceInfo = std::make_shared<AdapterDeviceInfo>();
    adapterDeviceInfo->adapterInfo_.reset();
    auto adapterDeviceInfo2 = std::make_shared<AdapterDeviceInfo>();
    std::set<std::shared_ptr<AdapterDeviceInfo>> adapterDeviceInfoSet;

    adapterDeviceInfoSet.insert(adapterDeviceInfo);
    adapterDeviceInfoSet.insert(adapterDeviceInfo2);

    audioPolicyConfigData->deviceInfoMap.insert({deviceMapKey, adapterDeviceInfoSet});

    auto ret = audioPolicyConfigData->GetAdapterDeviceInfo(type_, role_, networkId_, flags);
    EXPECT_EQ(ret, nullptr);
}

/**
* @tc.name  : Test AudioDefinitionAdapterInfoUnitTest.
* @tc.number: AudioPolicyConfigData_006
* @tc.desc  : Test GetAdapterType
*/
HWTEST(AudioDefinitionAdapterInfoUnitTest, AudioPolicyConfigData_006, TestSize.Level1)
{
    auto audioPolicyConfigData = std::make_shared<AudioPolicyConfigData>();
    EXPECT_NE(audioPolicyConfigData, nullptr);

    DeviceType type_ = DEVICE_TYPE_REMOTE_CAST;
    DeviceRole role_ = INPUT_DEVICE;
    std::string networkId_ = LOCAL_NETWORK_ID;
    uint32_t flags = 0;

    std::pair<DeviceType, DeviceRole> deviceMapKey = std::make_pair(DEVICE_TYPE_SPEAKER, role_);

    auto adapterDeviceInfo = std::make_shared<AdapterDeviceInfo>();
    std::shared_ptr<PolicyAdapterInfo> adapterInfo = std::make_shared<PolicyAdapterInfo>();
    adapterInfo->adapterName = "primary";
    adapterDeviceInfo->adapterInfo_ = adapterInfo;
    auto adapterDeviceInfo2 = std::make_shared<AdapterDeviceInfo>();
    std::set<std::shared_ptr<AdapterDeviceInfo>> adapterDeviceInfoSet;

    adapterDeviceInfoSet.insert(adapterDeviceInfo);
    adapterDeviceInfoSet.insert(adapterDeviceInfo2);

    audioPolicyConfigData->deviceInfoMap.insert({deviceMapKey, adapterDeviceInfoSet});

    auto ret = audioPolicyConfigData->GetAdapterDeviceInfo(type_, role_, networkId_, flags);
    EXPECT_EQ(ret, adapterDeviceInfo);
}

/**
* @tc.name  : Test AudioDefinitionAdapterInfoUnitTest.
* @tc.number: AudioPolicyConfigData_007
* @tc.desc  : Test GetAdapterType
*/
HWTEST(AudioDefinitionAdapterInfoUnitTest, AudioPolicyConfigData_007, TestSize.Level1)
{
    auto audioPolicyConfigData = std::make_shared<AudioPolicyConfigData>();
    EXPECT_NE(audioPolicyConfigData, nullptr);

    DeviceType type_ = DEVICE_TYPE_REMOTE_CAST;
    DeviceRole role_ = INPUT_DEVICE;
    std::string networkId_ = LOCAL_NETWORK_ID;
    uint32_t flags = 0;

    std::pair<DeviceType, DeviceRole> deviceMapKey = std::make_pair(DEVICE_TYPE_SPEAKER, role_);

    auto adapterDeviceInfo = std::make_shared<AdapterDeviceInfo>();
    std::shared_ptr<PolicyAdapterInfo> adapterInfo = std::make_shared<PolicyAdapterInfo>();
    adapterInfo->adapterName = "abc";
    adapterDeviceInfo->adapterInfo_ = adapterInfo;
    auto adapterDeviceInfo2 = std::make_shared<AdapterDeviceInfo>();
    std::set<std::shared_ptr<AdapterDeviceInfo>> adapterDeviceInfoSet;

    adapterDeviceInfoSet.insert(adapterDeviceInfo);
    adapterDeviceInfoSet.insert(adapterDeviceInfo2);

    audioPolicyConfigData->deviceInfoMap.insert({deviceMapKey, adapterDeviceInfoSet});

    auto ret = audioPolicyConfigData->GetAdapterDeviceInfo(type_, role_, networkId_, flags);
    EXPECT_EQ(ret, nullptr);
}

/**
* @tc.name  : Test AudioDefinitionAdapterInfoUnitTest.
* @tc.number: AudioPolicyConfigData_008
* @tc.desc  : Test GetAdapterType
*/
HWTEST(AudioDefinitionAdapterInfoUnitTest, AudioPolicyConfigData_008, TestSize.Level1)
{
    auto audioPolicyConfigData = std::make_shared<AudioPolicyConfigData>();
    EXPECT_NE(audioPolicyConfigData, nullptr);

    DeviceType type_ = DEVICE_TYPE_REMOTE_CAST;
    DeviceRole role_ = INPUT_DEVICE;
    std::string networkId_ = LOCAL_NETWORK_ID;
    uint32_t flags = 0;

    std::pair<DeviceType, DeviceRole> deviceMapKey = std::make_pair(DEVICE_TYPE_SPEAKER, role_);

    auto adapterDeviceInfo = std::make_shared<AdapterDeviceInfo>();
    std::shared_ptr<PolicyAdapterInfo> adapterInfo = std::make_shared<PolicyAdapterInfo>();
    adapterInfo->adapterName = "abc";
    adapterDeviceInfo->adapterInfo_ = adapterInfo;
    auto adapterDeviceInfo2 = std::make_shared<AdapterDeviceInfo>();
    std::shared_ptr<PolicyAdapterInfo> adapterInfo2 = std::make_shared<PolicyAdapterInfo>();
    adapterInfo2->adapterName = "";
    adapterDeviceInfo2->adapterInfo_ = adapterInfo2;
    std::set<std::shared_ptr<AdapterDeviceInfo>> adapterDeviceInfoSet;

    adapterDeviceInfoSet.insert(adapterDeviceInfo);
    adapterDeviceInfoSet.insert(adapterDeviceInfo2);

    audioPolicyConfigData->deviceInfoMap.insert({deviceMapKey, adapterDeviceInfoSet});
    auto ret = audioPolicyConfigData->GetAdapterDeviceInfo(type_, role_, networkId_, flags);
    EXPECT_EQ(ret, nullptr);
}

/**
* @tc.name  : Test AudioDefinitionAdapterInfoUnitTest.
* @tc.number: AudioPolicyConfigData_009
* @tc.desc  : Test SetSupportDeviceAndPipeMap
*/
HWTEST(AudioDefinitionAdapterInfoUnitTest, AudioPolicyConfigData_009, TestSize.Level1)
{
    auto audioPolicyConfigData = std::make_shared<AudioPolicyConfigData>();
    EXPECT_NE(audioPolicyConfigData, nullptr);
    auto pipeInfo = std::make_shared<AdapterPipeInfo>();
    EXPECT_NE(pipeInfo, nullptr);
    pipeInfo->streamPropInfos_ = {};
    pipeInfo->supportDevices_.push_back("test");
    pipeInfo->supportDevices_.push_back("test1");
    std::unordered_map<std::string, std::shared_ptr<AdapterDeviceInfo>> deviceInfoMap;
    deviceInfoMap["test"] = std::make_shared<AdapterDeviceInfo>();
    audioPolicyConfigData->SetSupportDeviceAndPipeMap(pipeInfo, deviceInfoMap);

    auto pipeStreamPtr = std::make_shared<PipeStreamPropInfo>();
    pipeInfo->streamPropInfos_.push_back(pipeStreamPtr);
    audioPolicyConfigData->SetSupportDeviceAndPipeMap(pipeInfo, deviceInfoMap);
    EXPECT_NE(deviceInfoMap["test"]->supportPipeMap_.size(), 0);
}

/**
* @tc.name  : Test AudioDefinitionAdapterInfoUnitTest.
* @tc.number: AudioPolicyConfigData_010
* @tc.desc  : Test SetDeviceInfoMap
*/
HWTEST(AudioDefinitionAdapterInfoUnitTest, AudioPolicyConfigData_010, TestSize.Level4)
{
    auto audioPolicyConfigData = std::make_shared<AudioPolicyConfigData>();
    std::unordered_map<std::string, std::shared_ptr<AdapterDeviceInfo>> deviceInfoMap;
    std::list<std::shared_ptr<AdapterDeviceInfo>> deviceInfos;
    audioPolicyConfigData->SetDeviceInfoMap(deviceInfos, deviceInfoMap);
    EXPECT_TRUE(deviceInfoMap.empty());
}

/**
* @tc.name  : Test AudioDefinitionAdapterInfoUnitTest.
* @tc.number: AudioPolicyConfigData_011
* @tc.desc  : Test SetSupportDeviceAndPipeMap
*/
HWTEST(AudioDefinitionAdapterInfoUnitTest, AudioPolicyConfigData_011, TestSize.Level4)
{
    auto audioPolicyConfigData = std::make_shared<AudioPolicyConfigData>();
    auto pipeInfo = std::make_shared<AdapterPipeInfo>();
    auto streamPropInfo = std::make_shared<PipeStreamPropInfo>();
    streamPropInfo->supportDevices_ = { "test_device1" };
    pipeInfo->streamPropInfos_.push_back(streamPropInfo);
    std::unordered_map<std::string, std::shared_ptr<AdapterDeviceInfo>> deviceInfoMap;
    deviceInfoMap["test_device1"] = std::make_shared<AdapterDeviceInfo>();
    deviceInfoMap["test_device2"] = std::make_shared<AdapterDeviceInfo>();
    audioPolicyConfigData->SetSupportDeviceAndPipeMap(pipeInfo, deviceInfoMap);
    EXPECT_EQ(pipeInfo->streamPropInfos_.front()->supportDeviceMap_.size(), 1);
}

/**
* @tc.name  : Test AudioDefinitionAdapterInfoUnitTest.
* @tc.number: AudioPolicyConfigData_012
* @tc.desc  : Test Reorganize
*/
HWTEST(AudioDefinitionAdapterInfoUnitTest, AudioPolicyConfigData_012, TestSize.Level4)
{
    auto audioPolicyConfigData = std::make_shared<AudioPolicyConfigData>();
    audioPolicyConfigData->adapterInfoMap.emplace(
        AudioAdapterType::TYPE_PRIMARY, std::make_shared<PolicyAdapterInfo>());
    EXPECT_NO_THROW(audioPolicyConfigData->Reorganize());
}

/**
* @tc.name  : Test AudioDefinitionAdapterInfoUnitTest.
* @tc.number: AudioPolicyConfigData_013
* @tc.desc  : Test SetVersion
*/
HWTEST(AudioDefinitionAdapterInfoUnitTest, AudioPolicyConfigData_013, TestSize.Level4)
{
    auto audioPolicyConfigData = std::make_shared<AudioPolicyConfigData>();
    std::string version = "";
    audioPolicyConfigData->SetVersion(version);
    EXPECT_EQ(audioPolicyConfigData->version_, STR_INITED);
}

/**
* @tc.name  : Test AudioDefinitionAdapterInfoUnitTest.
* @tc.number: AudioPolicyConfigData_014
* @tc.desc  : Test GetAdapterDeviceInfo
*/
HWTEST(AudioDefinitionAdapterInfoUnitTest, AudioPolicyConfigData_014, TestSize.Level4)
{
    auto audioPolicyConfigData = std::make_shared<AudioPolicyConfigData>();
    DeviceType type = DEVICE_TYPE_BLUETOOTH_A2DP;
    DeviceRole role = INPUT_DEVICE;
    std::string networkId = REMOTE_NETWORK_ID;
    uint32_t flags = 0;
    std::pair<DeviceType, DeviceRole> deviceMapKey = std::make_pair(type, role);
    std::set<std::shared_ptr<AdapterDeviceInfo>> adapterDeviceInfoSet;
    audioPolicyConfigData->deviceInfoMap.insert({deviceMapKey, adapterDeviceInfoSet});
    auto deviceSetIt = audioPolicyConfigData->deviceInfoMap.find(deviceMapKey);
    EXPECT_EQ(deviceSetIt->second.size(), 0);
    EXPECT_EQ(audioPolicyConfigData->GetAdapterDeviceInfo(type, role, networkId, flags), nullptr);
    
    auto adapterDeviceInfo1 = std::make_shared<AdapterDeviceInfo>();
    adapterDeviceInfoSet.insert(adapterDeviceInfo1);
    audioPolicyConfigData->deviceInfoMap[deviceMapKey] = adapterDeviceInfoSet;
    EXPECT_EQ(deviceSetIt->second.size(), 1);
    EXPECT_EQ(audioPolicyConfigData->GetAdapterDeviceInfo(type, role, networkId, flags), adapterDeviceInfo1);

    auto adapterDeviceInfo2 = std::make_shared<AdapterDeviceInfo>();
    adapterDeviceInfoSet.insert(adapterDeviceInfo2);
    audioPolicyConfigData->deviceInfoMap[deviceMapKey] = adapterDeviceInfoSet;
    EXPECT_EQ(deviceSetIt->second.size(), 2);
    EXPECT_EQ(audioPolicyConfigData->GetAdapterDeviceInfo(type, role, networkId, flags), nullptr);
}

/**
* @tc.name  : Test AudioDefinitionAdapterInfoUnitTest.
* @tc.number: AudioPolicyConfigData_015
* @tc.desc  : Test PolicyAdapterInfo GetAdapterType
*/
HWTEST(AudioDefinitionAdapterInfoUnitTest, AudioPolicyConfigData_015, TestSize.Level4)
{
    auto policyAdapterInfo = std::make_shared<PolicyAdapterInfo>();
    std::string adapterName = ADAPTER_TYPE_ACCESSORY;
    EXPECT_EQ(policyAdapterInfo->GetAdapterType(adapterName), AudioAdapterType::TYPE_ACCESSORY);
}

/**
* @tc.name  : Test AudioDefinitionAdapterInfoUnitTest.
* @tc.number: AudioPolicyConfigData_016
* @tc.desc  : Test PolicyAdapterInfo GetPipeInfoByName
*/
HWTEST(AudioDefinitionAdapterInfoUnitTest, AudioPolicyConfigData_016, TestSize.Level4)
{
    auto policyAdapterInfo = std::make_shared<PolicyAdapterInfo>();
    policyAdapterInfo->pipeInfos.emplace_back(nullptr);
    EXPECT_EQ(policyAdapterInfo->GetPipeInfoByName(""), nullptr);
}

/**
* @tc.name  : Test AudioDefinitionAdapterInfoUnitTest.
* @tc.number: AudioPolicyConfigData_017
* @tc.desc  : Test PolicyAdapterInfo GetPipeInfoByName
*/
HWTEST(AudioDefinitionAdapterInfoUnitTest, AudioPolicyConfigData_017, TestSize.Level4)
{
    auto policyAdapterInfo = std::make_shared<PolicyAdapterInfo>();
    auto deviceInfo = std::make_shared<AdapterDeviceInfo>();
    deviceInfo->type_ = DEVICE_TYPE_EARPIECE;
    deviceInfo->role_ = INPUT_DEVICE;
    policyAdapterInfo->deviceInfos.emplace_back(deviceInfo);

    DeviceType deviceType = DEVICE_TYPE_NONE;
    DeviceRole role = DEVICE_ROLE_NONE;
    EXPECT_EQ(policyAdapterInfo->GetDeviceInfoByType(deviceType, role), nullptr);

    deviceType = DEVICE_TYPE_EARPIECE;
    EXPECT_EQ(policyAdapterInfo->GetDeviceInfoByType(deviceType, role), nullptr);

    deviceType = DEVICE_TYPE_NONE;
    role = INPUT_DEVICE;
    EXPECT_EQ(policyAdapterInfo->GetDeviceInfoByType(deviceType, role), nullptr);

    deviceType = DEVICE_TYPE_EARPIECE;
    EXPECT_EQ(policyAdapterInfo->GetDeviceInfoByType(deviceType, role), deviceInfo);
}

/**
* @tc.name  : Test AudioDefinitionAdapterInfoUnitTest.
* @tc.number: AudioPolicyConfigData_018
* @tc.desc  : Test PipeStreamPropInfo SelfCheck
*/
HWTEST(AudioDefinitionAdapterInfoUnitTest, AudioPolicyConfigData_018, TestSize.Level4)
{
    auto pipeStreamPropInfo = std::make_shared<PipeStreamPropInfo>();
    auto adapterPipeInfo = std::make_shared<AdapterPipeInfo>();
    pipeStreamPropInfo->pipeInfo_ = adapterPipeInfo;
    auto deviceInfo = std::make_shared<AdapterDeviceInfo>();
    deviceInfo->name_ = "test_device";
    pipeStreamPropInfo->supportDeviceMap_.insert(std::pair(DEVICE_TYPE_EARPIECE, deviceInfo));
    pipeStreamPropInfo->supportDevices_ = { "test_device1" };
    EXPECT_NO_THROW(pipeStreamPropInfo->SelfCheck());
}

/**
* @tc.name  : Test AudioDefinitionAdapterInfoUnitTest.
* @tc.number: SetDeviceInfoMap_001
* @tc.desc  : Test SetDeviceInfoMap
*/
HWTEST(AudioDefinitionAdapterInfoUnitTest, SetDeviceInfoMap_001, TestSize.Level1)
{
    auto audioPolicyConfigData = std::make_shared<AudioPolicyConfigData>();
    EXPECT_NE(audioPolicyConfigData, nullptr);
    std::list<std::shared_ptr<AdapterDeviceInfo>> deviceInfo;
    std::unordered_map<std::string, std::shared_ptr<AdapterDeviceInfo>> deviceInfoMap;
    deviceInfoMap["test"] = std::make_shared<AdapterDeviceInfo>();
    EXPECT_NO_THROW(audioPolicyConfigData->SetDeviceInfoMap(deviceInfo, deviceInfoMap));
}

/**
* @tc.name  : Test AudioDefinitionAdapterInfoUnitTest.
* @tc.number: SetVersion_001
* @tc.desc  : Test SSetVersion
*/
HWTEST(AudioDefinitionAdapterInfoUnitTest, SetVersion_001, TestSize.Level1)
{
    auto audioPolicyConfigData = std::make_shared<AudioPolicyConfigData>();
    EXPECT_NE(audioPolicyConfigData, nullptr);
    
    std::string version = "";
    EXPECT_NO_THROW(audioPolicyConfigData->SetVersion(version));
}
} // namespace AudioStandard
} // namespace OHOS