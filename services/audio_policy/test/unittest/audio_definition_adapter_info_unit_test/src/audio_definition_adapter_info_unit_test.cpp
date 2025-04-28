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
} // namespace AudioStandard
} // namespace OHOS