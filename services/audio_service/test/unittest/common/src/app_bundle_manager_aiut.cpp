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

#include "app_bundle_manager.h"

#include "aiut_log.h"

#include <gtest/gtest.h>
#include <string>

using namespace testing::ext;

namespace {
constexpr int32_t BOOTUP_MUSIC_UID_TEST = 1003;
} // namespace

namespace OHOS {
namespace AudioStandard {

class AppBundleManagerAiut : public testing::Test {
public:
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.name  : AppBundleManager_GetBundleNameFromUid_BootupMusicUid
 * @tc.number: AppBundleManager_GetBundleNameFromUid_BootupMusicUid
 * @tc.desc  : Test GetBundleNameFromUid returns empty string when uid is BOOTUP_MUSIC_UID.
 */
HWTEST_F(AppBundleManagerAiut, AppBundleManager_GetBundleNameFromUid_BootupMusicUid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    std::string result = AppBundleManager::GetBundleNameFromUid(BOOTUP_MUSIC_UID_TEST);
    AIUT_EXPECT_EQ_INFO_LOG(result, std::string(""),
        "GetBundleNameFromUid should return empty for BOOTUP_MUSIC_UID, result=%s", result.c_str());
}

/**
 * @tc.name  : AppBundleManager_GetBundleNameFromUid_NormalUid
 * @tc.number: AppBundleManager_GetBundleNameFromUid_NormalUid
 * @tc.desc  : Test GetBundleNameFromUid with a normal uid (system ability may not be available in test).
 */
HWTEST_F(AppBundleManagerAiut, AppBundleManager_GetBundleNameFromUid_NormalUid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    std::string result = AppBundleManager::GetBundleNameFromUid(1001);
    AIUT_INFO_LOG("GetBundleNameFromUid with uid=1001 returned: %s", result.c_str());
    AIUT_EXPECT_TRUE_INFO_LOG(result.empty(),
        "GetBundleNameFromUid should return empty when BMS not available in test env, result=%s", result.c_str());
}

/**
 * @tc.name  : AppBundleManager_GetBundleNameFromUid_UidZero
 * @tc.number: AppBundleManager_GetBundleNameFromUid_UidZero
 * @tc.desc  : Test GetBundleNameFromUid with uid 0.
 */
HWTEST_F(AppBundleManagerAiut, AppBundleManager_GetBundleNameFromUid_UidZero, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    std::string result = AppBundleManager::GetBundleNameFromUid(0);
    AIUT_INFO_LOG("GetBundleNameFromUid with uid=0 returned: %s", result.c_str());
    AIUT_EXPECT_TRUE_INFO_LOG(result.empty(),
        "GetBundleNameFromUid should return empty for uid=0 in test env, result=%s", result.c_str());
}

/**
 * @tc.name  : AppBundleManager_GetSelfBundleName_NormalUid
 * @tc.number: AppBundleManager_GetSelfBundleName_NormalUid
 * @tc.desc  : Test GetSelfBundleName(int32_t uid) with a normal uid.
 */
HWTEST_F(AppBundleManagerAiut, AppBundleManager_GetSelfBundleName_NormalUid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    std::string result = AppBundleManager::GetSelfBundleName(1001);
    AIUT_INFO_LOG("GetSelfBundleName with uid=1001 returned: %s", result.c_str());
    AIUT_EXPECT_TRUE_INFO_LOG(result.empty(),
        "GetSelfBundleName should return empty when BMS not available in test env, result=%s", result.c_str());
}

/**
 * @tc.name  : AppBundleManager_GetSelfBundleName_UidZero
 * @tc.number: AppBundleManager_GetSelfBundleName_UidZero
 * @tc.desc  : Test GetSelfBundleName(int32_t uid) with uid 0.
 */
HWTEST_F(AppBundleManagerAiut, AppBundleManager_GetSelfBundleName_UidZero, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    std::string result = AppBundleManager::GetSelfBundleName(0);
    AIUT_INFO_LOG("GetSelfBundleName with uid=0 returned: %s", result.c_str());
    AIUT_EXPECT_TRUE_INFO_LOG(result.empty(),
        "GetSelfBundleName should return empty for uid=0 in test env, result=%s", result.c_str());
}

} // namespace AudioStandard
} // namespace OHOS
