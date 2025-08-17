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

#include <gtest/gtest.h>
#include "resource_manager_adapter.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
class ResourceManagerAdapterUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ResourceManagerAdapterUnitTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void ResourceManagerAdapterUnitTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void ResourceManagerAdapterUnitTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void ResourceManagerAdapterUnitTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

/**
 * @tc.name  : Test ResourceManagerAdapter API
 * @tc.type  : FUNC
 * @tc.number: ResourceManagerAdapter_001
 * @tc.desc  : Test IpcStreamInServer interface.
 */
HWTEST(ResourceManagerAdapterUnitTest, ResourceManagerAdapter_001, TestSize.Level1)
{
    auto resourceManagerAdapter = std::make_shared<ResourceManagerAdapter>();
    ASSERT_TRUE(resourceManagerAdapter != nullptr);
    resourceManagerAdapter->InitResourceManager();
}

/**
 * @tc.name  : Test ResourceManagerAdapter API
 * @tc.type  : FUNC
 * @tc.number: ResourceManagerAdapter_002
 * @tc.desc  : Test InitResourceManager interface.
 */
HWTEST(ResourceManagerAdapterUnitTest, ResourceManagerAdapter_002, TestSize.Level1)
{
    auto resourceManagerAdapter = std::make_shared<ResourceManagerAdapter>();
    ASSERT_TRUE(resourceManagerAdapter != nullptr);

    resourceManagerAdapter->resourceManager_ = Global::Resource::GetSystemResourceManagerNoSandBox();
    resourceManagerAdapter->resConfig_ = Global::Resource::CreateResConfig();
    resourceManagerAdapter->InitResourceManager();
}

/**
 * @tc.name  : Test ResourceManagerAdapter API
 * @tc.type  : FUNC
 * @tc.number: ResourceManagerAdapter_003
 * @tc.desc  : Test RefreshResConfig interface.
 */
HWTEST(ResourceManagerAdapterUnitTest, ResourceManagerAdapter_003, TestSize.Level1)
{
    auto resourceManagerAdapter = std::make_shared<ResourceManagerAdapter>();
    ASSERT_TRUE(resourceManagerAdapter != nullptr);

    resourceManagerAdapter->resourceManager_ = Global::Resource::GetSystemResourceManagerNoSandBox();
    resourceManagerAdapter->resConfig_ = Global::Resource::CreateResConfig();
    resourceManagerAdapter->RefreshResConfig();
}

/**
 * @tc.name  : Test ResourceManagerAdapter API
 * @tc.type  : FUNC
 * @tc.number: ResourceManagerAdapter_004
 * @tc.desc  : Test RefreshResConfig interface.
 */
HWTEST(ResourceManagerAdapterUnitTest, ResourceManagerAdapter_004, TestSize.Level1)
{
    auto resourceManagerAdapter = std::make_shared<ResourceManagerAdapter>();
    ASSERT_TRUE(resourceManagerAdapter != nullptr);

    resourceManagerAdapter->resConfig_ = Global::Resource::CreateResConfig();
    resourceManagerAdapter->RefreshResConfig();
}

/**
 * @tc.name  : Test ResourceManagerAdapter API
 * @tc.type  : FUNC
 * @tc.number: ResourceManagerAdapter_005
 * @tc.desc  : Test RefreshResConfig interface.
 */
HWTEST(ResourceManagerAdapterUnitTest, ResourceManagerAdapter_005, TestSize.Level1)
{
    auto resourceManagerAdapter = std::make_shared<ResourceManagerAdapter>();
    ASSERT_TRUE(resourceManagerAdapter != nullptr);

    resourceManagerAdapter->RefreshResConfig();
}

/**
 * @tc.name  : Test ResourceManagerAdapter API
 * @tc.type  : FUNC
 * @tc.number: ResourceManagerAdapter_006
 * @tc.desc  : Test ReleaseSystemResourceManager interface.
 */
HWTEST(ResourceManagerAdapterUnitTest, ResourceManagerAdapter_006, TestSize.Level1)
{
    auto resourceManagerAdapter = std::make_shared<ResourceManagerAdapter>();
    ASSERT_TRUE(resourceManagerAdapter != nullptr);
    
    resourceManagerAdapter->ReleaseSystemResourceManager();
}

/**
 * @tc.name  : Test ResourceManagerAdapter API
 * @tc.type  : FUNC
 * @tc.number: ResourceManagerAdapter_007
 * @tc.desc  : Test ReleaseSystemResourceManager interface.
 */
HWTEST(ResourceManagerAdapterUnitTest, ResourceManagerAdapter_007, TestSize.Level1)
{
    auto resourceManagerAdapter = std::make_shared<ResourceManagerAdapter>();
    ASSERT_TRUE(resourceManagerAdapter != nullptr);
    
    resourceManagerAdapter->resourceManager_ = Global::Resource::GetSystemResourceManagerNoSandBox();
    resourceManagerAdapter->resConfig_ = Global::Resource::CreateResConfig();
    resourceManagerAdapter->ReleaseSystemResourceManager();
}

/**
 * @tc.name  : Test ResourceManagerAdapter API
 * @tc.type  : FUNC
 * @tc.number: ResourceManagerAdapter_008
 * @tc.desc  : Test GetSystemStringByName interface.
 */
HWTEST(ResourceManagerAdapterUnitTest, ResourceManagerAdapter_008, TestSize.Level1)
{
    auto resourceManagerAdapter = std::make_shared<ResourceManagerAdapter>();
    ASSERT_TRUE(resourceManagerAdapter != nullptr);

    std::string name = "test";
    auto result = resourceManagerAdapter->GetSystemStringByName(name);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name  : Test ResourceManagerAdapter API
 * @tc.type  : FUNC
 * @tc.number: ResourceManagerAdapter_009
 * @tc.desc  : Test GetSystemStringByName interface.
 */
HWTEST(ResourceManagerAdapterUnitTest, ResourceManagerAdapter_009, TestSize.Level1)
{
    auto resourceManagerAdapter = std::make_shared<ResourceManagerAdapter>();
    ASSERT_TRUE(resourceManagerAdapter != nullptr);

    std::string name = "test";
    resourceManagerAdapter->resourceManager_ = Global::Resource::GetSystemResourceManagerNoSandBox();
    auto result = resourceManagerAdapter->GetSystemStringByName(name);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name  : Test ResourceManagerAdapter API
 * @tc.type  : FUNC
 * @tc.number: ResourceManagerAdapter_010
 * @tc.desc  : Test GetSystemStringByName interface.
 */
HWTEST(ResourceManagerAdapterUnitTest, ResourceManagerAdapter_010, TestSize.Level1)
{
    auto resourceManagerAdapter = std::make_shared<ResourceManagerAdapter>();
    ASSERT_TRUE(resourceManagerAdapter != nullptr);

    std::string name = "test";
    resourceManagerAdapter->resourceManager_ = nullptr;
    auto result = resourceManagerAdapter->GetSystemStringByName(name);
    EXPECT_EQ("", result);
}
}
}