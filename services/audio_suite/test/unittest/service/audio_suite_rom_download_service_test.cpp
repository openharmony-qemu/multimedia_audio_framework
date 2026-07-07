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
#include <fcntl.h>
#include <unistd.h>
#include "audio_suite_rom_download_service.h"
#include "download_manager.h"
#include "audio_errors.h"
#include "download_ut_test_common.h"
#include "mock_update_engine_callback.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

class TestAudioSuiteRomDownloadService : public AudioSuiteRomDownloadService {
public:
    TestAudioSuiteRomDownloadService()
        : AudioSuiteRomDownloadService(SA_ID, false) {}

    bool checkPermissionResult_ = true;

protected:
    bool CheckPermission() override
    {
        return checkPermissionResult_;
    }
};

class AudioSuiteRomDownloadServiceTest : public Test {
public:
    void SetUp() override
    {
        service = std::make_unique<TestAudioSuiteRomDownloadService>();
        service->checkPermissionResult_ = true;
    }

    void TearDown() override
    {
        if (service) {
            service->OnStop();
            service.reset();
        }
    }

    std::unique_ptr<TestAudioSuiteRomDownloadService> service;
};

HWTEST_F(AudioSuiteRomDownloadServiceTest, RegisterDownloadCallbackPermissionDeniedTest, TestSize.Level0)
{
    service->checkPermissionResult_ = false;
    ErrCode result = SUCCESS;
    ErrCode ret = service->RegisterDownloadCallback(nullptr, result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
}

HWTEST_F(AudioSuiteRomDownloadServiceTest, UnregisterDownloadCallbackPermissionDeniedTest, TestSize.Level0)
{
    service->checkPermissionResult_ = false;
    ErrCode result = SUCCESS;
    ErrCode ret = service->UnregisterDownloadCallback(result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
}

HWTEST_F(AudioSuiteRomDownloadServiceTest, RegisterUpdateEngineCallbackPermissionDeniedTest, TestSize.Level0)
{
    service->checkPermissionResult_ = false;
    ErrCode result = SUCCESS;
    ErrCode ret = service->RegisterUpdateEngineCallback(nullptr, result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
}

HWTEST_F(AudioSuiteRomDownloadServiceTest, UnregisterUpdateEngineCallbackPermissionDeniedTest, TestSize.Level0)
{
    service->checkPermissionResult_ = false;
    ErrCode result = SUCCESS;
    ErrCode ret = service->UnregisterUpdateEngineCallback(result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
}

HWTEST_F(AudioSuiteRomDownloadServiceTest, StartDownloadPermissionDeniedTest, TestSize.Level0)
{
    service->checkPermissionResult_ = false;
    ErrCode result = SUCCESS;
    ErrCode ret = service->StartDownload("v1", result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
}

HWTEST_F(AudioSuiteRomDownloadServiceTest, CancelDownloadPermissionDeniedTest, TestSize.Level0)
{
    service->checkPermissionResult_ = false;
    ErrCode result = SUCCESS;
    ErrCode ret = service->CancelDownload(result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
}

HWTEST_F(AudioSuiteRomDownloadServiceTest, StartBackgroundDownloadPermissionDeniedTest, TestSize.Level0)
{
    service->checkPermissionResult_ = false;
    ErrCode result = SUCCESS;
    ErrCode ret = service->StartBackgroundDownload("v1", result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
}

HWTEST_F(AudioSuiteRomDownloadServiceTest, GetDownloadStatusPermissionDeniedTest, TestSize.Level0)
{
    service->checkPermissionResult_ = false;
    DownloadStatus status;
    ErrCode result = SUCCESS;
    ErrCode ret = service->GetDownloadStatus(status, "v1", result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
}

HWTEST_F(AudioSuiteRomDownloadServiceTest, UninstallCloudRomPermissionDeniedTest, TestSize.Level0)
{
    service->checkPermissionResult_ = false;
    ErrCode result = SUCCESS;
    ErrCode ret = service->UninstallCloudRom("v1", result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
}

HWTEST_F(AudioSuiteRomDownloadServiceTest, StartDownloadNoManagerTest, TestSize.Level0)
{
    service->downloadManager_ = nullptr;
    ErrCode result = SUCCESS;
    ErrCode ret = service->StartDownload("v1", result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, ERR_NOT_STARTED);
}

HWTEST_F(AudioSuiteRomDownloadServiceTest, CancelDownloadNoManagerTest, TestSize.Level0)
{
    service->downloadManager_ = nullptr;
    ErrCode result = SUCCESS;
    ErrCode ret = service->CancelDownload(result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, ERR_NOT_STARTED);
}

HWTEST_F(AudioSuiteRomDownloadServiceTest, StartBackgroundDownloadNoManagerTest, TestSize.Level0)
{
    service->downloadManager_ = nullptr;
    ErrCode result = SUCCESS;
    ErrCode ret = service->StartBackgroundDownload("v1", result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, ERR_NOT_STARTED);
}

HWTEST_F(AudioSuiteRomDownloadServiceTest, UninstallCloudRomNoManagerTest, TestSize.Level0)
{
    service->downloadManager_ = nullptr;
    ErrCode result = SUCCESS;
    ErrCode ret = service->UninstallCloudRom("v1", result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, ERR_NOT_STARTED);
}

HWTEST_F(AudioSuiteRomDownloadServiceTest, RegisterUpdateEngineCallbackSuccessTest, TestSize.Level0)
{
    ErrCode result = SUCCESS;
    sptr<IRemoteObject> obj = sptr<IRemoteObject>(new MockRemoteObject());
    ErrCode ret = service->RegisterUpdateEngineCallback(obj, result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioSuiteRomDownloadServiceTest, UnregisterUpdateEngineCallbackSuccessTest, TestSize.Level0)
{
    ErrCode result = SUCCESS;
    ErrCode ret = service->UnregisterUpdateEngineCallback(result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioSuiteRomDownloadServiceTest, UnregisterDownloadCallbackSuccessTest, TestSize.Level0)
{
    ErrCode result = SUCCESS;
    ErrCode ret = service->UnregisterDownloadCallback(result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(service->downloadCallback_, nullptr);
}

HWTEST_F(AudioSuiteRomDownloadServiceTest, DumpTest, TestSize.Level0)
{
    std::vector<std::u16string> args;
    int32_t fd = open("/dev/null", O_WRONLY);
    if (fd >= 0) {
        int32_t ret = service->Dump(fd, args);
        EXPECT_EQ(ret, 0);
        close(fd);
    }
}

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS
