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
 */

#include "oh_audio_suite_download_manager_test.h"
#include "native_audio_suite_download_manager.h"
#include "native_audio_common.h"
#include "native_audio_suite_base.h"
#include "audio_errors.h"
#include "securec.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void OHAudioSuiteDownloadManagerTest::SetUpTestCase(void) {}
void OHAudioSuiteDownloadManagerTest::TearDownTestCase(void) {}
void OHAudioSuiteDownloadManagerTest::SetUp(void) {}
void OHAudioSuiteDownloadManagerTest::TearDown(void) {}

HWTEST_F(OHAudioSuiteDownloadManagerTest, RegisterDownloadCallbackNullManager, TestSize.Level0)
{
    int32_t ret = OH_AudioSuite_RegisterDownloadCallback(nullptr, nullptr);
    EXPECT_EQ(ret, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

HWTEST_F(OHAudioSuiteDownloadManagerTest, StartDownloadNullManager, TestSize.Level0)
{
    int32_t ret = OH_AudioSuite_StartDownload(nullptr, "v1");
    EXPECT_EQ(ret, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

HWTEST_F(OHAudioSuiteDownloadManagerTest, StartDownloadNullVersion, TestSize.Level0)
{
    OH_AudioSuite_DownloadManager *manager = nullptr;
    OH_AudioSuite_GetDownloadManager(&manager);
    if (manager != nullptr) {
        int32_t ret = OH_AudioSuite_StartDownload(manager, nullptr);
        EXPECT_EQ(ret, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
    }
}

HWTEST_F(OHAudioSuiteDownloadManagerTest, CancelDownloadNullManager, TestSize.Level0)
{
    int32_t ret = OH_AudioSuite_CancelDownload(nullptr, "v1");
    EXPECT_EQ(ret, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

HWTEST_F(OHAudioSuiteDownloadManagerTest, StartBackgroundDownloadNullManager, TestSize.Level0)
{
    int32_t ret = OH_AudioSuite_StartBackgroundDownload(nullptr, "v1");
    EXPECT_EQ(ret, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

HWTEST_F(OHAudioSuiteDownloadManagerTest, StartBackgroundDownloadNullVersion, TestSize.Level0)
{
    OH_AudioSuite_DownloadManager *manager = nullptr;
    OH_AudioSuite_GetDownloadManager(&manager);
    if (manager != nullptr) {
        int32_t ret = OH_AudioSuite_StartBackgroundDownload(manager, nullptr);
        EXPECT_EQ(ret, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
    }
}

HWTEST_F(OHAudioSuiteDownloadManagerTest, GetDownloadStatusNullManager, TestSize.Level0)
{
    OH_AudioSuite_DownloadStatusInfo status;
    int32_t ret = OH_AudioSuite_GetDownloadStatus(nullptr, "v1", &status);
    EXPECT_EQ(ret, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

HWTEST_F(OHAudioSuiteDownloadManagerTest, GetDownloadStatusNullStatus, TestSize.Level0)
{
    OH_AudioSuite_DownloadManager *manager = nullptr;
    OH_AudioSuite_GetDownloadManager(&manager);
    if (manager != nullptr) {
        int32_t ret = OH_AudioSuite_GetDownloadStatus(manager, "v1", nullptr);
        EXPECT_EQ(ret, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
    }
}

HWTEST_F(OHAudioSuiteDownloadManagerTest, UninstallCloudRomNullManager, TestSize.Level0)
{
    int32_t ret = OH_AudioSuite_UninstallCloudRom(nullptr, "v1");
    EXPECT_EQ(ret, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

HWTEST_F(OHAudioSuiteDownloadManagerTest, UninstallCloudRomNullVersion, TestSize.Level0)
{
    OH_AudioSuite_DownloadManager *manager = nullptr;
    OH_AudioSuite_GetDownloadManager(&manager);
    if (manager != nullptr) {
        int32_t ret = OH_AudioSuite_UninstallCloudRom(manager, nullptr);
        EXPECT_EQ(ret, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
    }
}

HWTEST_F(OHAudioSuiteDownloadManagerTest, UnregisterDownloadCallbackNullManager, TestSize.Level0)
{
    OH_AudioSuite_DownloadCallback cb = [](OH_AudioSuite_DownloadStatusInfoArray *) {};
    int32_t ret = OH_AudioSuite_UnregisterDownloadCallback(nullptr, &cb);
    EXPECT_EQ(ret, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

HWTEST_F(OHAudioSuiteDownloadManagerTest, IsFeatureInstalledNullManager, TestSize.Level0)
{
    bool installStatus[1] = {false};
    const char *names[] = {"v1"};
    int32_t ret = OH_AudioSuite_IsFeatureInstalled(nullptr, names, 1, installStatus, 1);
    EXPECT_EQ(ret, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

HWTEST_F(OHAudioSuiteDownloadManagerTest, IsFeatureInstalledNullInstallStatus, TestSize.Level0)
{
    OH_AudioSuite_DownloadManager *manager = nullptr;
    OH_AudioSuite_GetDownloadManager(&manager);
    ASSERT_NE(manager, nullptr);
    const char *names[] = {"v1"};
    int32_t ret = OH_AudioSuite_IsFeatureInstalled(manager, names, 1, nullptr, 1);
    EXPECT_EQ(ret, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

HWTEST_F(OHAudioSuiteDownloadManagerTest, IsFeatureInstalledSizeLessThanCount, TestSize.Level0)
{
    OH_AudioSuite_DownloadManager *manager = nullptr;
    OH_AudioSuite_GetDownloadManager(&manager);
    ASSERT_NE(manager, nullptr);
    bool installStatus[1] = {false};
    const char *names[] = {"v1"};
    int32_t ret = OH_AudioSuite_IsFeatureInstalled(manager, names, 2, installStatus, 1);
    EXPECT_EQ(ret, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

HWTEST_F(OHAudioSuiteDownloadManagerTest, IsFeatureInstalledNullFeatureNames, TestSize.Level0)
{
    OH_AudioSuite_DownloadManager *manager = nullptr;
    OH_AudioSuite_GetDownloadManager(&manager);
    ASSERT_NE(manager, nullptr);
    bool installStatus[1] = {false};
    int32_t ret = OH_AudioSuite_IsFeatureInstalled(manager, nullptr, 1, installStatus, 1);
    EXPECT_EQ(ret, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

HWTEST_F(OHAudioSuiteDownloadManagerTest, IsFeatureInstalledCountZero, TestSize.Level0)
{
    OH_AudioSuite_DownloadManager *manager = nullptr;
    OH_AudioSuite_GetDownloadManager(&manager);
    ASSERT_NE(manager, nullptr);
    bool installStatus[1] = {false};
    const char *names[] = {"v1"};
    int32_t ret = OH_AudioSuite_IsFeatureInstalled(manager, names, 0, installStatus, 0);
    EXPECT_EQ(ret, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

HWTEST_F(OHAudioSuiteDownloadManagerTest, IsFeatureInstalledCountExceedMax, TestSize.Level0)
{
    OH_AudioSuite_DownloadManager *manager = nullptr;
    OH_AudioSuite_GetDownloadManager(&manager);
    ASSERT_NE(manager, nullptr);
    bool installStatus[6] = {false};
    const char *names[] = {"v1", "v2", "v3", "v4", "v5", "v6"};
    int32_t ret = OH_AudioSuite_IsFeatureInstalled(manager, names, 6, installStatus, 6);
    EXPECT_EQ(ret, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

HWTEST_F(OHAudioSuiteDownloadManagerTest, IsFeatureInstalledNullElement, TestSize.Level0)
{
    OH_AudioSuite_DownloadManager *manager = nullptr;
    OH_AudioSuite_GetDownloadManager(&manager);
    ASSERT_NE(manager, nullptr);
    bool installStatus[1] = {false};
    const char *names[] = {nullptr};
    int32_t ret = OH_AudioSuite_IsFeatureInstalled(manager, names, 1, installStatus, 1);
    EXPECT_EQ(ret, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

HWTEST_F(OHAudioSuiteDownloadManagerTest, IsFeatureInstalledInvalidManager, TestSize.Level0)
{
    int dummy = 0;
    OH_AudioSuite_DownloadManager *fakeManager = reinterpret_cast<OH_AudioSuite_DownloadManager *>(&dummy);
    bool installStatus[1] = {false};
    const char *names[] = {"v1"};
    int32_t ret = OH_AudioSuite_IsFeatureInstalled(fakeManager, names, 1, installStatus, 1);
    EXPECT_EQ(ret, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

HWTEST_F(OHAudioSuiteDownloadManagerTest, GetFeatureStatusNullManager, TestSize.Level0)
{
    OH_AudioSuite_DownloadStatusInfo statusInfos[1];
    const char *names[] = {"v1"};
    int32_t ret = OH_AudioSuite_GetFeatureStatus(nullptr, names, 1, statusInfos, 1);
    EXPECT_EQ(ret, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

HWTEST_F(OHAudioSuiteDownloadManagerTest, GetFeatureStatusNullStatusInfos, TestSize.Level0)
{
    OH_AudioSuite_DownloadManager *manager = nullptr;
    OH_AudioSuite_GetDownloadManager(&manager);
    ASSERT_NE(manager, nullptr);
    const char *names[] = {"v1"};
    int32_t ret = OH_AudioSuite_GetFeatureStatus(manager, names, 1, nullptr, 1);
    EXPECT_EQ(ret, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

HWTEST_F(OHAudioSuiteDownloadManagerTest, GetFeatureStatusSizeLessThanCount, TestSize.Level0)
{
    OH_AudioSuite_DownloadManager *manager = nullptr;
    OH_AudioSuite_GetDownloadManager(&manager);
    ASSERT_NE(manager, nullptr);
    OH_AudioSuite_DownloadStatusInfo statusInfos[1];
    const char *names[] = {"v1"};
    int32_t ret = OH_AudioSuite_GetFeatureStatus(manager, names, 2, statusInfos, 1);
    EXPECT_EQ(ret, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

HWTEST_F(OHAudioSuiteDownloadManagerTest, GetFeatureStatusNullFeatureNames, TestSize.Level0)
{
    OH_AudioSuite_DownloadManager *manager = nullptr;
    OH_AudioSuite_GetDownloadManager(&manager);
    ASSERT_NE(manager, nullptr);
    OH_AudioSuite_DownloadStatusInfo statusInfos[1];
    int32_t ret = OH_AudioSuite_GetFeatureStatus(manager, nullptr, 1, statusInfos, 1);
    EXPECT_EQ(ret, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

HWTEST_F(OHAudioSuiteDownloadManagerTest, GetNewVersionInfoNullManager, TestSize.Level0)
{
    OH_AudioSuite_CloudRomVersionInfo versionInfos[1];
    const char *names[] = {"v1"};
    int32_t ret = OH_AudioSuite_GetNewVersionInfo(nullptr, names, 1, versionInfos, 1);
    EXPECT_EQ(ret, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

HWTEST_F(OHAudioSuiteDownloadManagerTest, GetNewVersionInfoNullVersionInfos, TestSize.Level0)
{
    OH_AudioSuite_DownloadManager *manager = nullptr;
    OH_AudioSuite_GetDownloadManager(&manager);
    ASSERT_NE(manager, nullptr);
    const char *names[] = {"v1"};
    int32_t ret = OH_AudioSuite_GetNewVersionInfo(manager, names, 1, nullptr, 1);
    EXPECT_EQ(ret, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

HWTEST_F(OHAudioSuiteDownloadManagerTest, GetNewVersionInfoSizeLessThanCount, TestSize.Level0)
{
    OH_AudioSuite_DownloadManager *manager = nullptr;
    OH_AudioSuite_GetDownloadManager(&manager);
    ASSERT_NE(manager, nullptr);
    OH_AudioSuite_CloudRomVersionInfo versionInfos[1];
    const char *names[] = {"v1"};
    int32_t ret = OH_AudioSuite_GetNewVersionInfo(manager, names, 2, versionInfos, 1);
    EXPECT_EQ(ret, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

HWTEST_F(OHAudioSuiteDownloadManagerTest, GetNewVersionInfoNullFeatureNames, TestSize.Level0)
{
    OH_AudioSuite_DownloadManager *manager = nullptr;
    OH_AudioSuite_GetDownloadManager(&manager);
    ASSERT_NE(manager, nullptr);
    OH_AudioSuite_CloudRomVersionInfo versionInfos[1];
    int32_t ret = OH_AudioSuite_GetNewVersionInfo(manager, nullptr, 1, versionInfos, 1);
    EXPECT_EQ(ret, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

} // namespace AudioStandard
} // namespace OHOS
