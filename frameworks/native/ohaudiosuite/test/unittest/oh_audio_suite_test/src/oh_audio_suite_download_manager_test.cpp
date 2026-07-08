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

void OHAudioSuiteDownloadManagerNullTest::SetUpTestCase(void) {}
void OHAudioSuiteDownloadManagerNullTest::TearDownTestCase(void) {}
void OHAudioSuiteDownloadManagerNullTest::SetUp(void) {}
void OHAudioSuiteDownloadManagerNullTest::TearDown(void) {}

HWTEST_F(OHAudioSuiteDownloadManagerNullTest, RegisterDownloadCallbackNullManager, TestSize.Level0)
{
    int32_t ret = OH_AudioSuite_RegisterDownloadCallback(nullptr, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);
}

HWTEST_F(OHAudioSuiteDownloadManagerNullTest, StartDownloadNullManager, TestSize.Level0)
{
    int32_t ret = OH_AudioSuite_StartDownload(nullptr, "v1");
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);
}

HWTEST_F(OHAudioSuiteDownloadManagerNullTest, StartDownloadNullVersion, TestSize.Level0)
{
    OH_AudioSuiteDownloadManager *manager = nullptr;
    OH_AudioSuite_GetDownloadManager(&manager);
    if (manager != nullptr) {
        int32_t ret = OH_AudioSuite_StartDownload(manager, nullptr);
        EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);
    }
}

HWTEST_F(OHAudioSuiteDownloadManagerNullTest, CancelDownloadNullManager, TestSize.Level0)
{
    int32_t ret = OH_AudioSuite_CancelDownload(nullptr, "v1");
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);
}

HWTEST_F(OHAudioSuiteDownloadManagerNullTest, StartBackgroundDownloadNullManager, TestSize.Level0)
{
    int32_t ret = OH_AudioSuite_StartBackgroundDownload(nullptr, "v1");
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);
}

HWTEST_F(OHAudioSuiteDownloadManagerNullTest, StartBackgroundDownloadNullVersion, TestSize.Level0)
{
    OH_AudioSuiteDownloadManager *manager = nullptr;
    OH_AudioSuite_GetDownloadManager(&manager);
    if (manager != nullptr) {
        int32_t ret = OH_AudioSuite_StartBackgroundDownload(manager, nullptr);
        EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);
    }
}

HWTEST_F(OHAudioSuiteDownloadManagerNullTest, GetDownloadStatusNullManager, TestSize.Level0)
{
    OH_AudioSuite_DownloadStatusInfo status;
    int32_t ret = OH_AudioSuite_GetDownloadStatus(nullptr, "v1", &status);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);
}

HWTEST_F(OHAudioSuiteDownloadManagerNullTest, GetDownloadStatusNullStatus, TestSize.Level0)
{
    OH_AudioSuiteDownloadManager *manager = nullptr;
    OH_AudioSuite_GetDownloadManager(&manager);
    if (manager != nullptr) {
        int32_t ret = OH_AudioSuite_GetDownloadStatus(manager, "v1", nullptr);
        EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);
    }
}

HWTEST_F(OHAudioSuiteDownloadManagerNullTest, UninstallCloudRomNullManager, TestSize.Level0)
{
    int32_t ret = OH_AudioSuite_UninstallCloudRom(nullptr, "v1");
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);
}

HWTEST_F(OHAudioSuiteDownloadManagerNullTest, UninstallCloudRomNullVersion, TestSize.Level0)
{
    OH_AudioSuiteDownloadManager *manager = nullptr;
    OH_AudioSuite_GetDownloadManager(&manager);
    if (manager != nullptr) {
        int32_t ret = OH_AudioSuite_UninstallCloudRom(manager, nullptr);
        EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);
    }
}

HWTEST_F(OHAudioSuiteDownloadManagerNullTest, UnregisterDownloadCallbackNullManager, TestSize.Level0)
{
    OH_AudioSuite_DownloadCallback cb = [](OH_AudioSuite_DownloadStatusInfoArray *) {};
    int32_t ret = OH_AudioSuite_UnregisterDownloadCallback(nullptr, &cb);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);
}

void ConvertToSaResultTest::SetUpTestCase(void) {}
void ConvertToSaResultTest::TearDownTestCase(void) {}
void ConvertToSaResultTest::SetUp(void) {}
void ConvertToSaResultTest::TearDown(void) {}

HWTEST_F(ConvertToSaResultTest, SuccessMapping, TestSize.Level0)
{
    EXPECT_EQ(AUDIOCOMMON_RESULT_SUCCESS, 0);
}

HWTEST_F(ConvertToSaResultTest, InvalidParamMapping, TestSize.Level0)
{
    EXPECT_EQ(AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, 6800101);
}

HWTEST_F(ConvertToSaResultTest, IllegalStateMapping, TestSize.Level0)
{
    EXPECT_EQ(AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE, 6800103);
}

HWTEST_F(ConvertToSaResultTest, SystemMapping, TestSize.Level0)
{
    EXPECT_EQ(AUDIOCOMMON_RESULT_ERROR_SYSTEM, 6800301);
}

void CApiStatusConstantTest::SetUpTestCase(void) {}
void CApiStatusConstantTest::TearDownTestCase(void) {}
void CApiStatusConstantTest::SetUp(void) {}
void CApiStatusConstantTest::TearDown(void) {}

HWTEST_F(CApiStatusConstantTest, DownloadStatusInfoStructSize, TestSize.Level0)
{
    OH_AudioSuite_DownloadStatusInfo info;
    memset_s(&info, sizeof(info), 0, sizeof(info));
    EXPECT_EQ(sizeof(info.featureName), 256u);
    EXPECT_EQ(sizeof(info.installPath), 256u);
}

HWTEST_F(CApiStatusConstantTest, AudiosuiteErrorCodeConstants, TestSize.Level0)
{
    EXPECT_EQ(AUDIOSUITE_SUCCESS, 0);
    EXPECT_EQ(AUDIOSUITE_ERROR_INVALID_PARAM, 1);
    EXPECT_EQ(AUDIOSUITE_ERROR_INVALID_STATE, 2);
    EXPECT_EQ(AUDIOSUITE_ERROR_SYSTEM, 3);
}

} // namespace AudioStandard
} // namespace OHOS
