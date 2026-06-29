/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "audio_system_client_policy_manager_unit_test.h"
#include "audio_system_client_policy_manager.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioSystemClientPolicyManagerUnitTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void AudioSystemClientPolicyManagerUnitTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void AudioSystemClientPolicyManagerUnitTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void AudioSystemClientPolicyManagerUnitTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

// Test case example
/**
 * @tc.name  : Test ConfigDistributedRoutingRole
 * @tc.type  : FUNC
 * @tc.number: ConfigDistributedRoutingRole_001
 * @tc.desc  : Test ConfigDistributedRoutingRole with nullptr descriptor
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, ConfigDistributedRoutingRole_001, TestSize.Level0)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    int32_t ret = manager.ConfigDistributedRoutingRole(nullptr, CastType::CAST_TYPE_NONE);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test ConfigDistributedRoutingRole
 * @tc.type  : FUNC
 * @tc.number: ConfigDistributedRoutingRole_002
 * @tc.desc  : Test ConfigDistributedRoutingRole with non-output device
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, ConfigDistributedRoutingRole_002, TestSize.Level0)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> descriptor = std::make_shared<AudioDeviceDescriptor>();
    descriptor->deviceRole_ = DeviceRole::INPUT_DEVICE; // Not an output device
    int32_t ret = manager.ConfigDistributedRoutingRole(descriptor, CastType::CAST_TYPE_NONE);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}


} // namespace AudioStandard
} // namespace OHOS
