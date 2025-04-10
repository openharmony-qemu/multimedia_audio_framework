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
    
    policyAdapter->SelfCheck();
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
    policyAdapter->SelfCheck();
    policyAdapter->deviceInfos.push_back(deviceInfo);
    policyAdapter->SelfCheck();
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

    deviceAdapter->SelfCheck();
    EXPECT_EQ(deviceAdapter->adapterInfo_.lock(), nullptr);

    auto adapterInfoPtr = std::make_shared<PolicyAdapterInfo>();
    deviceAdapter->adapterInfo_ = adapterInfoPtr;
    deviceAdapter->SelfCheck();
    EXPECT_EQ(deviceAdapter->supportPipeMap_.size(), 0);

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
    pipeInfo->SelfCheck();

    pipeInfo->name_ = TEST_PIPE_NAME;
    deviceAdapter->supportPipeMap_.insert(std::make_pair(0, std::make_shared<AdapterPipeInfo>()));
    deviceAdapter->SelfCheck();
    deviceAdapter->supportPipeMap_.insert(std::make_pair(1, pipeInfo));
    deviceAdapter->SelfCheck();
}
} // namespace AudioStandard
} // namespace OHOS