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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>
#include <vector>

#include "audio_bundle_manager.h"
#include "util/audio_playing_app_manager.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
constexpr int32_t TEST_FIRST_APP_UID = 1001;
constexpr int32_t TEST_SECOND_APP_UID = 1002;
constexpr int32_t TEST_UNKNOWN_APP_UID = 1003;
const std::string TEST_FIRST_BUNDLE_NAME = "test.bundle.first";
const std::string TEST_SECOND_BUNDLE_NAME = "test.bundle.second";

void SetTestBundleName(int32_t uid, const std::string &bundleName)
{
    AudioBundleManager::GetInstance().InsertBundleNameToMap(uid, bundleName);
}

void ClearTestBundleNames()
{
    AudioBundleManager::GetInstance().EraseBundleNameFromMap(TEST_FIRST_APP_UID);
    AudioBundleManager::GetInstance().EraseBundleNameFromMap(TEST_SECOND_APP_UID);
    AudioBundleManager::GetInstance().EraseBundleNameFromMap(TEST_UNKNOWN_APP_UID);
}

class AudioPlayingAppManagerUnitTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override
    {
        ClearTestBundleNames();
    }
    void TearDown() override
    {
        ClearTestBundleNames();
    }
};

/**
 * @tc.name   : Test AudioPlayingAppManager API
 * @tc.number : AudioPlayingAppManagerUnitTest_001
 * @tc.desc   : Test playing app name is updated after start
 */
HWTEST_F(AudioPlayingAppManagerUnitTest, AudioPlayingAppManagerUnitTest_001, TestSize.Level1)
{
    SetTestBundleName(TEST_FIRST_APP_UID, TEST_FIRST_BUNDLE_NAME);
    std::vector<std::string> params;
    AudioPlayingAppManager manager([&params](const std::string &param) {
        params.push_back(param);
        return true;
    });

    manager.Start();
    EXPECT_TRUE(params.empty());

    std::vector<int32_t> appsUid = { TEST_FIRST_APP_UID };
    manager.UpdateAppsUid(appsUid.cbegin(), appsUid.cend());
    manager.UpdatePlayingAppNameToHdi();

    ASSERT_EQ(params.size(), 1);
    EXPECT_EQ(params[0], "PLAYING_APP_NAME=[" + TEST_FIRST_BUNDLE_NAME + "]");
}

/**
 * @tc.name   : Test AudioPlayingAppManager API
 * @tc.number : AudioPlayingAppManagerUnitTest_002
 * @tc.desc   : Test playing app name is not updated before start
 */
HWTEST_F(AudioPlayingAppManagerUnitTest, AudioPlayingAppManagerUnitTest_002, TestSize.Level1)
{
    SetTestBundleName(TEST_FIRST_APP_UID, TEST_FIRST_BUNDLE_NAME);
    std::vector<std::string> params;
    AudioPlayingAppManager manager([&params](const std::string &param) {
        params.push_back(param);
        return true;
    });

    std::vector<int32_t> appsUid = { TEST_FIRST_APP_UID };
    manager.UpdateAppsUid(appsUid.cbegin(), appsUid.cend());
    manager.UpdatePlayingAppNameToHdi();

    EXPECT_TRUE(params.empty());
}

/**
 * @tc.name   : Test AudioPlayingAppManager API
 * @tc.number : AudioPlayingAppManagerUnitTest_003
 * @tc.desc   : Test duplicate uid is normalized
 */
HWTEST_F(AudioPlayingAppManagerUnitTest, AudioPlayingAppManagerUnitTest_003, TestSize.Level1)
{
    SetTestBundleName(TEST_FIRST_APP_UID, TEST_FIRST_BUNDLE_NAME);
    SetTestBundleName(TEST_SECOND_APP_UID, TEST_SECOND_BUNDLE_NAME);
    std::vector<std::string> params;
    AudioPlayingAppManager manager([&params](const std::string &param) {
        params.push_back(param);
        return true;
    });

    manager.Start();
    std::vector<int32_t> appsUid = { TEST_FIRST_APP_UID, TEST_SECOND_APP_UID, TEST_FIRST_APP_UID };
    manager.UpdateAppsUid(appsUid.cbegin(), appsUid.cend());
    manager.UpdatePlayingAppNameToHdi();

    ASSERT_EQ(params.size(), 1);
    EXPECT_EQ(params[0], "PLAYING_APP_NAME=[" + TEST_FIRST_BUNDLE_NAME + "," + TEST_SECOND_BUNDLE_NAME + "]");
}

/**
 * @tc.name   : Test AudioPlayingAppManager API
 * @tc.number : AudioPlayingAppManagerUnitTest_004
 * @tc.desc   : Test same uid is not updated repeatedly
 */
HWTEST_F(AudioPlayingAppManagerUnitTest, AudioPlayingAppManagerUnitTest_004, TestSize.Level1)
{
    SetTestBundleName(TEST_FIRST_APP_UID, TEST_FIRST_BUNDLE_NAME);
    std::vector<std::string> params;
    AudioPlayingAppManager manager([&params](const std::string &param) {
        params.push_back(param);
        return true;
    });

    manager.Start();
    std::vector<int32_t> appsUid = { TEST_FIRST_APP_UID };
    manager.UpdateAppsUid(appsUid.cbegin(), appsUid.cend());
    manager.UpdatePlayingAppNameToHdi();
    manager.UpdateAppsUid(appsUid.cbegin(), appsUid.cend());
    manager.UpdatePlayingAppNameToHdi();

    ASSERT_EQ(params.size(), 1);
    EXPECT_EQ(params[0], "PLAYING_APP_NAME=[" + TEST_FIRST_BUNDLE_NAME + "]");
}

/**
 * @tc.name   : Test AudioPlayingAppManager API
 * @tc.number : AudioPlayingAppManagerUnitTest_005
 * @tc.desc   : Test failed update is retried
 */
HWTEST_F(AudioPlayingAppManagerUnitTest, AudioPlayingAppManagerUnitTest_005, TestSize.Level1)
{
    SetTestBundleName(TEST_FIRST_APP_UID, TEST_FIRST_BUNDLE_NAME);
    std::vector<std::string> params;
    bool setParamResult = false;
    AudioPlayingAppManager manager([&params, &setParamResult](const std::string &param) {
        params.push_back(param);
        return setParamResult;
    });

    manager.Start();
    std::vector<int32_t> appsUid = { TEST_FIRST_APP_UID };
    manager.UpdateAppsUid(appsUid.cbegin(), appsUid.cend());
    manager.UpdatePlayingAppNameToHdi();
    setParamResult = true;
    manager.UpdatePlayingAppNameToHdi();

    ASSERT_EQ(params.size(), 2);
    EXPECT_EQ(params[0], "PLAYING_APP_NAME=[" + TEST_FIRST_BUNDLE_NAME + "]");
    EXPECT_EQ(params[1], "PLAYING_APP_NAME=[" + TEST_FIRST_BUNDLE_NAME + "]");
}

/**
 * @tc.name   : Test AudioPlayingAppManager API
 * @tc.number : AudioPlayingAppManagerUnitTest_006
 * @tc.desc   : Test stop clears playing app name
 */
HWTEST_F(AudioPlayingAppManagerUnitTest, AudioPlayingAppManagerUnitTest_006, TestSize.Level1)
{
    SetTestBundleName(TEST_FIRST_APP_UID, TEST_FIRST_BUNDLE_NAME);
    std::vector<std::string> params;
    AudioPlayingAppManager manager([&params](const std::string &param) {
        params.push_back(param);
        return true;
    });

    manager.Start();
    std::vector<int32_t> appsUid = { TEST_FIRST_APP_UID };
    manager.UpdateAppsUid(appsUid.cbegin(), appsUid.cend());
    manager.UpdatePlayingAppNameToHdi();
    manager.Stop();
    manager.UpdatePlayingAppNameToHdi();

    ASSERT_EQ(params.size(), 2);
    EXPECT_EQ(params[0], "PLAYING_APP_NAME=[" + TEST_FIRST_BUNDLE_NAME + "]");
    EXPECT_EQ(params[1], "PLAYING_APP_NAME=[]");
}

/**
 * @tc.name   : Test AudioPlayingAppManager API
 * @tc.number : AudioPlayingAppManagerUnitTest_007
 * @tc.desc   : Test unknown uid is skipped
 */
HWTEST_F(AudioPlayingAppManagerUnitTest, AudioPlayingAppManagerUnitTest_007, TestSize.Level1)
{
    SetTestBundleName(TEST_FIRST_APP_UID, TEST_FIRST_BUNDLE_NAME);
    SetTestBundleName(TEST_UNKNOWN_APP_UID, "");
    std::vector<std::string> params;
    AudioPlayingAppManager manager([&params](const std::string &param) {
        params.push_back(param);
        return true;
    });

    manager.Start();
    std::vector<int32_t> appsUid = { TEST_FIRST_APP_UID, TEST_UNKNOWN_APP_UID };
    manager.UpdateAppsUid(appsUid.cbegin(), appsUid.cend());
    manager.UpdatePlayingAppNameToHdi();

    ASSERT_EQ(params.size(), 1);
    EXPECT_EQ(params[0], "PLAYING_APP_NAME=[" + TEST_FIRST_BUNDLE_NAME + "]");
}

/**
 * @tc.name   : Test AudioPlayingAppManager API
 * @tc.number : AudioPlayingAppManagerUnitTest_008
 * @tc.desc   : Test empty uid list sets empty playing app name
 */
HWTEST_F(AudioPlayingAppManagerUnitTest, AudioPlayingAppManagerUnitTest_008, TestSize.Level1)
{
    std::vector<std::string> params;
    AudioPlayingAppManager manager([&params](const std::string &param) {
        params.push_back(param);
        return true;
    });

    manager.Start();
    std::vector<int32_t> appsUid = {};
    manager.UpdateAppsUid(appsUid.cbegin(), appsUid.cend());
    manager.UpdatePlayingAppNameToHdi();

    ASSERT_EQ(params.size(), 0);
}

/**
 * @tc.name   : Test AudioPlayingAppManager API
 * @tc.number : AudioPlayingAppManagerUnitTest_009
 * @tc.desc   : Test restart after stop can update again
 */
HWTEST_F(AudioPlayingAppManagerUnitTest, AudioPlayingAppManagerUnitTest_009, TestSize.Level1)
{
    SetTestBundleName(TEST_FIRST_APP_UID, TEST_FIRST_BUNDLE_NAME);
    SetTestBundleName(TEST_SECOND_APP_UID, TEST_SECOND_BUNDLE_NAME);
    std::vector<std::string> params;
    AudioPlayingAppManager manager([&params](const std::string &param) {
        params.push_back(param);
        return true;
    });

    manager.Start();
    std::vector<int32_t> appsUid = { TEST_FIRST_APP_UID };
    manager.UpdateAppsUid(appsUid.cbegin(), appsUid.cend());
    manager.UpdatePlayingAppNameToHdi();
    manager.Stop();
    
    manager.Start();
    std::vector<int32_t> newAppsUid = { TEST_SECOND_APP_UID };
    manager.UpdateAppsUid(newAppsUid.cbegin(), newAppsUid.cend());
    manager.UpdatePlayingAppNameToHdi();

    ASSERT_EQ(params.size(), 3);
    EXPECT_EQ(params[0], "PLAYING_APP_NAME=[" + TEST_FIRST_BUNDLE_NAME + "]");
    EXPECT_EQ(params[1], "PLAYING_APP_NAME=[]");
    EXPECT_EQ(params[2], "PLAYING_APP_NAME=[" + TEST_SECOND_BUNDLE_NAME + "]");
}

/**
 * @tc.name   : Test AudioPlayingAppManager API
 * @tc.number : AudioPlayingAppManagerUnitTest_010
 * @tc.desc   : Test null setter does not crash in UpdatePlayingAppNameToHdi
 */
HWTEST_F(AudioPlayingAppManagerUnitTest, AudioPlayingAppManagerUnitTest_010, TestSize.Level1)
{
    SetTestBundleName(TEST_FIRST_APP_UID, TEST_FIRST_BUNDLE_NAME);
    AudioPlayingAppManager manager(nullptr);

    manager.Start();
    std::vector<int32_t> appsUid = { TEST_FIRST_APP_UID };
    manager.UpdateAppsUid(appsUid.cbegin(), appsUid.cend());
    manager.UpdatePlayingAppNameToHdi();
}

/**
 * @tc.name   : Test AudioPlayingAppManager API
 * @tc.number : AudioPlayingAppManagerUnitTest_011
 * @tc.desc   : Test null setter does not crash in Stop
 */
HWTEST_F(AudioPlayingAppManagerUnitTest, AudioPlayingAppManagerUnitTest_011, TestSize.Level1)
{
    SetTestBundleName(TEST_FIRST_APP_UID, TEST_FIRST_BUNDLE_NAME);
    AudioPlayingAppManager manager(nullptr);

    manager.Start();
    std::vector<int32_t> appsUid = { TEST_FIRST_APP_UID };
    manager.UpdateAppsUid(appsUid.cbegin(), appsUid.cend());
    manager.UpdatePlayingAppNameToHdi();
    manager.Stop();
}
} // namespace AudioStandard
} // namespace OHOS
