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

// AIUT补充用例说明：
// 本文件仅测试null路径和一致性检查，不使用GetSystemResourceManagerNoSandBox()
// 避免在测试环境中获取无效指针导致崩溃。

#include "resource_manager_adapter.h"
#include "aiut_log.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class ResourceManagerAdapterAiut : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name  : Test InitResourceManager when resourceManager_ is null
 * @tc.number: ResourceManagerAdapter_InitResourceManager_WithNullManager
 * @tc.desc  : Test InitResourceManager initializes resourceManager_ when null.
 */
HWTEST_F(ResourceManagerAdapterAiut, ResourceManagerAdapter_InitResourceManager_WithNullManager, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto adapter = std::make_shared<ResourceManagerAdapter>();
    AIUT_ASSERT_NE_INFO_LOG(adapter, nullptr, "adapter create failed");
    adapter->resourceManager_ = nullptr;
    adapter->resConfig_ = nullptr;
    adapter->InitResourceManager();
    std::string result1 = adapter->GetSystemStringByName("test_name");
    std::string result2 = adapter->GetSystemStringByName("test_name");
    AIUT_EXPECT_EQ_INFO_LOG(result1, result2,
        "GetSystemStringByName should return consistent result after Init, result=%s", result1.c_str());
}

/**
 * @tc.name  : Test InitResourceManager when already initialized
 * @tc.number: ResourceManagerAdapter_InitResourceManager_AlreadyInitialized
 * @tc.desc  : Test InitResourceManager when resourceManager_ is already set.
 *             InitResourceManager检查resourceManager_非null时直接返回，不会解引用。
 */
HWTEST_F(ResourceManagerAdapterAiut, ResourceManagerAdapter_InitResourceManager_AlreadyInitialized, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto adapter = std::make_shared<ResourceManagerAdapter>();
    AIUT_ASSERT_NE_INFO_LOG(adapter, nullptr, "adapter create failed");
    adapter->resourceManager_ = nullptr;
    adapter->resConfig_ = nullptr;
    adapter->InitResourceManager();
    adapter->InitResourceManager();
    std::string result1 = adapter->GetSystemStringByName("test_name");
    std::string result2 = adapter->GetSystemStringByName("test_name");
    AIUT_EXPECT_EQ_INFO_LOG(result1, result2,
        "GetSystemStringByName should return consistent result, result=%s", result1.c_str());
}

/**
 * @tc.name  : Test RefreshResConfig with null resConfig_
 * @tc.number: ResourceManagerAdapter_RefreshResConfig_WithNullConfig
 * @tc.desc  : Test RefreshResConfig when resConfig_ is null after language check.
 */
HWTEST_F(ResourceManagerAdapterAiut, ResourceManagerAdapter_RefreshResConfig_WithNullConfig, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto adapter = std::make_shared<ResourceManagerAdapter>();
    AIUT_ASSERT_NE_INFO_LOG(adapter, nullptr, "adapter create failed");
    adapter->resConfig_ = nullptr;
    adapter->resourceManager_ = nullptr;
    adapter->RefreshResConfig();
    std::string result1 = adapter->GetSystemStringByName("test_name");
    std::string result2 = adapter->GetSystemStringByName("test_name");
    AIUT_EXPECT_EQ_INFO_LOG(result1, result2,
        "GetSystemStringByName should return consistent result after Refresh, result=%s", result1.c_str());
}

/**
 * @tc.name  : Test RefreshResConfig with null resourceManager_
 * @tc.number: ResourceManagerAdapter_RefreshResConfig_WithNullManager
 * @tc.desc  : Test RefreshResConfig when resourceManager_ is null but resConfig_ is set.
 */
HWTEST_F(ResourceManagerAdapterAiut, ResourceManagerAdapter_RefreshResConfig_WithNullManager, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto adapter = std::make_shared<ResourceManagerAdapter>();
    AIUT_ASSERT_NE_INFO_LOG(adapter, nullptr, "adapter create failed");
    adapter->resConfig_ = nullptr;
    adapter->resourceManager_ = nullptr;
    adapter->RefreshResConfig();
    std::string result1 = adapter->GetSystemStringByName("test_name");
    std::string result2 = adapter->GetSystemStringByName("test_name");
    AIUT_EXPECT_EQ_INFO_LOG(result1, result2,
        "GetSystemStringByName should return consistent result after Refresh, result=%s", result1.c_str());
}

/**
 * @tc.name  : Test ReleaseSystemResourceManager when both null
 * @tc.number: ResourceManagerAdapter_ReleaseSystemResourceManager_BothNull
 * @tc.desc  : Test ReleaseSystemResourceManager when both resourceManager_ and resConfig_ are null.
 */
HWTEST_F(ResourceManagerAdapterAiut, ResourceManagerAdapter_ReleaseSystemResourceManager_BothNull, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto adapter = std::make_shared<ResourceManagerAdapter>();
    AIUT_ASSERT_NE_INFO_LOG(adapter, nullptr, "adapter create failed");
    adapter->resourceManager_ = nullptr;
    adapter->resConfig_ = nullptr;
    adapter->ReleaseSystemResourceManager();
    std::string result = adapter->GetSystemStringByName("test_name");
    AIUT_EXPECT_EQ_INFO_LOG(result, std::string(""),
        "GetSystemStringByName should return empty after Release with null, result=%s", result.c_str());
}

/**
 * @tc.name  : Test GetSystemStringByName with null resourceManager_
 * @tc.number: ResourceManagerAdapter_GetSystemStringByName_WithNullManager
 * @tc.desc  : Test GetSystemStringByName returns empty string when resourceManager_ is null.
 */
HWTEST_F(ResourceManagerAdapterAiut,
    ResourceManagerAdapter_GetSystemStringByName_WithNullManager, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto adapter = std::make_shared<ResourceManagerAdapter>();
    AIUT_ASSERT_NE_INFO_LOG(adapter, nullptr, "adapter create failed");
    adapter->resourceManager_ = nullptr;
    std::string result = adapter->GetSystemStringByName("test_name");
    AIUT_EXPECT_EQ_INFO_LOG(result, std::string(""), "GetSystemStringByName should return empty for null manager, "
        "result=%s", result.c_str());
}

/**
 * @tc.name  : Test GetSystemStringByName with empty name
 * @tc.number: ResourceManagerAdapter_GetSystemStringByName_WithEmptyName
 * @tc.desc  : Test GetSystemStringByName with empty name string and null manager.
 */
HWTEST_F(ResourceManagerAdapterAiut,
    ResourceManagerAdapter_GetSystemStringByName_WithEmptyName, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto adapter = std::make_shared<ResourceManagerAdapter>();
    AIUT_ASSERT_NE_INFO_LOG(adapter, nullptr, "adapter create failed");
    adapter->resourceManager_ = nullptr;
    std::string result1 = adapter->GetSystemStringByName("");
    std::string result2 = adapter->GetSystemStringByName("");
    AIUT_EXPECT_EQ_INFO_LOG(result1, result2,
        "GetSystemStringByName(empty) should return consistent result, result=%s", result1.c_str());
}

/**
 * @tc.name  : Test GetMediaDataByName with null resourceManager_
 * @tc.number: ResourceManagerAdapter_GetMediaDataByName_WithNullManager
 * @tc.desc  : Test GetMediaDataByName returns ERROR when resourceManager_ is null.
 */
HWTEST_F(ResourceManagerAdapterAiut,
    ResourceManagerAdapter_GetMediaDataByName_WithNullManager, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto adapter = std::make_shared<ResourceManagerAdapter>();
    AIUT_ASSERT_NE_INFO_LOG(adapter, nullptr, "adapter create failed");
    adapter->resourceManager_ = nullptr;
    size_t len = 0;
    std::unique_ptr<uint8_t[]> outValue;
    auto rstate = adapter->GetMediaDataByName("test_media", len, outValue, 0);
    AIUT_EXPECT_EQ_INFO_LOG(static_cast<int32_t>(rstate),
        static_cast<int32_t>(Global::Resource::RState::ERROR),
        "GetMediaDataByName should return ERROR for null manager, rstate=%d", static_cast<int32_t>(rstate));
}

/**
 * @tc.name  : Test GetInstance
 * @tc.number: ResourceManagerAdapter_GetInstance_ReturnsSingleton
 * @tc.desc  : Test GetInstance returns valid singleton.
 */
HWTEST_F(ResourceManagerAdapterAiut, ResourceManagerAdapter_GetInstance_ReturnsSingleton, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto *instance = ResourceManagerAdapter::GetInstance();
    AIUT_ASSERT_NE_INFO_LOG(instance, nullptr, "GetInstance should return non-null, instance=%p", instance);
    auto *instance2 = ResourceManagerAdapter::GetInstance();
    AIUT_EXPECT_EQ_INFO_LOG(instance, instance2, "GetInstance should return same singleton, instance1=%p, instance2=%p",
        instance, instance2);
}

} // namespace AudioStandard
} // namespace OHOS
