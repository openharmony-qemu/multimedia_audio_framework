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
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <chrono>
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

class DownloadManagerTest : public Test {
public:
    void SetUp() override
    {
        manager = std::make_unique<DownloadManager>();
        callbackInvoked = false;
        lastStatus = DownloadStatus();
        manager->RegisterDownloadCallback([this](const DownloadStatus &s) {
            std::lock_guard<std::mutex> lock(cbMutex_);
            callbackInvoked = true;
            lastStatus = s;
            cbCond_.notify_one();
        });
    }

    void TearDown() override
    {
        if (manager) {
            manager->Release();
            manager.reset();
        }
    }

    std::unique_ptr<DownloadManager> manager;
    bool callbackInvoked = false;
    DownloadStatus lastStatus;
    std::mutex cbMutex_;
    std::condition_variable cbCond_;

    bool WaitForCallback(int32_t timeoutMs = 500)
    {
        std::unique_lock<std::mutex> lock(cbMutex_);
        return cbCond_.wait_for(lock, std::chrono::milliseconds(timeoutMs),
            [this] { return callbackInvoked; });
    }

    void ResetCallbackState()
    {
        std::lock_guard<std::mutex> lock(cbMutex_);
        callbackInvoked = false;
        lastStatus = DownloadStatus();
    }
};

HWTEST_F(DownloadManagerTest, ConstructorTest, TestSize.Level0)
{
    auto newManager = std::make_unique<DownloadManager>();
    EXPECT_NE(newManager, nullptr);
    EXPECT_EQ(newManager->downloadState_.load(), DownloadState::DOWNLOAD_IDLE);
    EXPECT_EQ(newManager->downloadProgress_.load(), 0u);
    EXPECT_EQ(newManager->isDownloading_.load(), false);
    EXPECT_EQ(newManager->downloadRetryCount_.load(), 0);
    EXPECT_EQ(newManager->cancelRequested_.load(), false);
    EXPECT_EQ(newManager->isCallbackRegistered_.load(), false);
    EXPECT_EQ(newManager->isBackgroundDownload_.load(), false);
}

HWTEST_F(DownloadManagerTest, InitTest, TestSize.Level0)
{
    int32_t ret = manager->Init();
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(DownloadManagerTest, ReleaseIdleTest, TestSize.Level0)
{
    int32_t ret = manager->Release();
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(DownloadManagerTest, ReleaseDuringDownloadTest, TestSize.Level0)
{
    manager->downloadState_ = DownloadState::DOWNLOAD_IN_PROGRESS;
    manager->isDownloading_ = false;
    manager->currentVersion_ = "v1";
    int32_t ret = manager->Release();
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(manager->downloadState_.load(), DownloadState::DOWNLOAD_IDLE);
    EXPECT_EQ(manager->isDownloading_.load(), false);
    EXPECT_EQ(manager->downloadRetryCount_.load(), 0);
}

HWTEST_F(DownloadManagerTest, StartDownloadEmptyVersionTest, TestSize.Level0)
{
    int32_t ret = manager->StartDownload("");
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST_F(DownloadManagerTest, CancelDownloadIdleTest, TestSize.Level0)
{
    int32_t ret = manager->CancelDownload("v1");
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

HWTEST_F(DownloadManagerTest, CancelDownloadWaitingTest, TestSize.Level0)
{
    manager->downloadState_ = DownloadState::DOWNLOAD_WAITING;
    manager->currentVersion_ = "v1";
    manager->cancelRequested_ = false;
    manager->isDownloading_ = false;
    int32_t ret = manager->CancelDownload("v1");
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(manager->downloadState_.load(), DownloadState::DOWNLOAD_CANCELLED);
}

HWTEST_F(DownloadManagerTest, StartBackgroundDownloadEmptyVersionTest, TestSize.Level0)
{
    int32_t ret = manager->StartBackgroundDownload("");
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST_F(DownloadManagerTest, StartBackgroundDownloadInProgressTest, TestSize.Level0)
{
    manager->downloadState_ = DownloadState::DOWNLOAD_IN_PROGRESS;
    int32_t ret = manager->StartBackgroundDownload("v1");
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

HWTEST_F(DownloadManagerTest, RegisterDownloadCallbackNullTest, TestSize.Level0)
{
    int32_t ret = manager->RegisterDownloadCallback(nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST_F(DownloadManagerTest, RegisterDownloadCallbackSuccessTest, TestSize.Level0)
{
    int32_t ret = manager->RegisterDownloadCallback([](const DownloadStatus &) {});
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(DownloadManagerTest, SetUpdateEngineCallbackNullTest, TestSize.Level0)
{
    manager->SetUpdateEngineCallback(nullptr);
    EXPECT_EQ(manager->updateEngineCallback_, nullptr);
}

HWTEST_F(DownloadManagerTest, WaitForCallbackRegistrationTimeoutTest, TestSize.Level0)
{
    auto newManager = std::make_unique<DownloadManager>();
    bool waited = newManager->WaitForCallbackRegistration(50);
    EXPECT_FALSE(waited);
    newManager->Release();
}

HWTEST_F(DownloadManagerTest, FillDownloadStatusDownloadingTest, TestSize.Level0)
{
    DownloadStatus status;
    StatusInfo statusInfo = CreateStatusInfo("v1", CloudRomStatus::DOWNLOADING, 50, "/data/rom");
    CloudRomNewVersionInfo versionInfo = CreateNewVersionInfo("v1", 1024000);
    manager->FillDownloadStatus(status, "v1", statusInfo, versionInfo);
    EXPECT_EQ(status.state, DownloadState::DOWNLOAD_IN_PROGRESS);
    EXPECT_EQ(status.progress, 50u);
    EXPECT_EQ(status.romPath, "/data/rom");
    EXPECT_EQ(status.fileSize, 1024000);
    EXPECT_EQ(status.version, "v1");
}

HWTEST_F(DownloadManagerTest, FillDownloadStatusSuccessTest, TestSize.Level0)
{
    DownloadStatus status;
    StatusInfo statusInfo = CreateStatusInfo("v1", CloudRomStatus::DOWNLOAD_SUCCESS, 100, "/data/rom");
    CloudRomNewVersionInfo versionInfo = CreateNewVersionInfo("v1", 1024000);
    manager->FillDownloadStatus(status, "v1", statusInfo, versionInfo);
    EXPECT_EQ(status.state, DownloadState::DOWNLOAD_COMPLETED);
    EXPECT_EQ(status.progress, 100u);
}

HWTEST_F(DownloadManagerTest, FillDownloadStatusInstallSuccessTest, TestSize.Level0)
{
    DownloadStatus status;
    StatusInfo statusInfo = CreateStatusInfo("v1", CloudRomStatus::INSTALL_SUCCESS, 100, "/data/rom");
    CloudRomNewVersionInfo versionInfo = CreateNewVersionInfo("v1", 2048000);
    manager->FillDownloadStatus(status, "v1", statusInfo, versionInfo);
    EXPECT_EQ(status.state, DownloadState::DOWNLOAD_COMPLETED);
    EXPECT_EQ(status.fileSize, 2048000);
}

HWTEST_F(DownloadManagerTest, FillDownloadStatusFailTest, TestSize.Level0)
{
    DownloadStatus status;
    StatusInfo statusInfo = CreateStatusInfo("v1", CloudRomStatus::DOWNLOAD_FAIL, 30, "");
    statusInfo.errorInfo.errorNum = 1;
    statusInfo.errorInfo.message = "network error";
    CloudRomNewVersionInfo versionInfo = CreateNewVersionInfo("v1", 1024000);
    manager->FillDownloadStatus(status, "v1", statusInfo, versionInfo);
    EXPECT_EQ(status.state, DownloadState::DOWNLOAD_FAILED);
    EXPECT_EQ(status.errorCode, 1);
    EXPECT_EQ(status.errMessage, "network error");
}

HWTEST_F(DownloadManagerTest, FillDownloadStatusInstallFailTest, TestSize.Level0)
{
    DownloadStatus status;
    StatusInfo statusInfo = CreateStatusInfo("v1", CloudRomStatus::INSTALL_FAIL, 100, "");
    CloudRomNewVersionInfo versionInfo = CreateNewVersionInfo("v1", 1024000);
    manager->FillDownloadStatus(status, "v1", statusInfo, versionInfo);
    EXPECT_EQ(status.state, DownloadState::DOWNLOAD_FAILED);
}

HWTEST_F(DownloadManagerTest, FillDownloadStatusDefaultTest, TestSize.Level0)
{
    DownloadStatus status;
    StatusInfo statusInfo = CreateStatusInfo("v1", CloudRomStatus::INIT, 0, "");
    CloudRomNewVersionInfo versionInfo = CreateNewVersionInfo("v1", 1024000);
    manager->FillDownloadStatus(status, "v1", statusInfo, versionInfo);
    EXPECT_EQ(status.state, DownloadState::DOWNLOAD_IDLE);
}

HWTEST_F(DownloadManagerTest, NotifyDownloadStatusChangedWithCallbackTest, TestSize.Level0)
{
    ResetCallbackState();
    DownloadStatus testStatus;
    testStatus.state = DownloadState::DOWNLOAD_IN_PROGRESS;
    testStatus.progress = 60;
    testStatus.version = "v1";
    manager->NotifyDownloadStatusChanged(testStatus);
    EXPECT_TRUE(WaitForCallback());
    EXPECT_EQ(lastStatus.state, DownloadState::DOWNLOAD_IN_PROGRESS);
    EXPECT_EQ(lastStatus.progress, 60u);
}

HWTEST_F(DownloadManagerTest, NotifyDownloadStatusChangedNullCallbackTest, TestSize.Level0)
{
    auto newManager = std::make_unique<DownloadManager>();
    newManager->downloadCallback_ = nullptr;
    DownloadStatus testStatus;
    testStatus.state = DownloadState::DOWNLOAD_IDLE;
    newManager->NotifyDownloadStatusChanged(testStatus);
    EXPECT_EQ(newManager->downloadCallback_, nullptr);
    newManager->Release();
}

HWTEST_F(DownloadManagerTest, UpdateDownloadStateTest, TestSize.Level0)
{
    manager->UpdateDownloadState(DownloadState::DOWNLOAD_WAITING);
    EXPECT_EQ(manager->downloadState_.load(), DownloadState::DOWNLOAD_WAITING);

    manager->UpdateDownloadState(DownloadState::DOWNLOAD_IN_PROGRESS, ERR_AUDIO_SUITE_SA_DOWNLOAD_FAILED);
    EXPECT_EQ(manager->downloadState_.load(), DownloadState::DOWNLOAD_IN_PROGRESS);
    EXPECT_EQ(manager->errorCode_, ERR_AUDIO_SUITE_SA_DOWNLOAD_FAILED);
}

HWTEST_F(DownloadManagerTest, OnCloudRomCallbackFeatureMismatchTest, TestSize.Level0)
{
    ResetCallbackState();
    manager->currentVersion_ = "v1";
    CloudRomCallback callbackData;
    callbackData.statusInfos.push_back(CreateStatusInfo("v_other", CloudRomStatus::DOWNLOADING, 50));
    ErrCode ret = manager->OnCloudRomCallback(callbackData);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_FALSE(callbackInvoked);
}

HWTEST_F(DownloadManagerTest, OnCloudRomCallbackEmptyStatusInfosTest, TestSize.Level0)
{
    ResetCallbackState();
    CloudRomCallback callbackData;
    ErrCode ret = manager->OnCloudRomCallback(callbackData);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_FALSE(callbackInvoked);
}

HWTEST_F(DownloadManagerTest, HandleDownloadingTest, TestSize.Level0)
{
    ResetCallbackState();
    manager->currentVersion_ = "v1";
    StatusInfo info = CreateStatusInfo("v1", CloudRomStatus::DOWNLOADING, 30);
    manager->HandleDownloading(info);
    EXPECT_TRUE(WaitForCallback());
    EXPECT_EQ(lastStatus.state, DownloadState::DOWNLOAD_IN_PROGRESS);
    EXPECT_EQ(lastStatus.progress, 30u);
    EXPECT_EQ(lastStatus.version, "v1");
    EXPECT_EQ(lastStatus.errMessage, std::string(""));
}

HWTEST_F(DownloadManagerTest, HandleInstallSuccessTest, TestSize.Level0)
{
    ResetCallbackState();
    manager->currentVersion_ = "v1";
    StatusInfo info = CreateStatusInfo("v1", CloudRomStatus::INSTALL_SUCCESS, 100, "/data/rom");
    manager->HandleInstallSuccess(info);
    EXPECT_TRUE(WaitForCallback());
    EXPECT_EQ(lastStatus.state, DownloadState::DOWNLOAD_COMPLETED);
    EXPECT_EQ(lastStatus.progress, static_cast<uint32_t>(DOWNLOAD_COMPLETE));
    EXPECT_EQ(lastStatus.version, "v1");
    EXPECT_EQ(lastStatus.romPath, "/data/rom/v1");
    EXPECT_EQ(manager->isDownloading_.load(), false);
    EXPECT_EQ(manager->downloadRetryCount_.load(), 0);
}

HWTEST_F(DownloadManagerTest, HandleInstallSuccessWithPathTest, TestSize.Level0)
{
    ResetCallbackState();
    manager->currentVersion_ = "v2";
    StatusInfo info = CreateStatusInfo("v2", CloudRomStatus::DOWNLOAD_SUCCESS, 100, "/opt/rom");
    manager->HandleInstallSuccess(info);
    EXPECT_TRUE(WaitForCallback());
    EXPECT_EQ(lastStatus.romPath, "/opt/rom/v2");
}

HWTEST_F(DownloadManagerTest, HandleDownloadFailMaxRetryTest, TestSize.Level0)
{
    ResetCallbackState();
    manager->currentVersion_ = "v1";
    manager->downloadRetryCount_ = MAX_RETRY_COUNT;
    manager->downloadProgress_ = 30;
    manager->isDownloading_ = true;

    StatusInfo info = CreateStatusInfo("v1", CloudRomStatus::DOWNLOAD_FAIL, 30);
    manager->HandleDownloadFail(info);

    EXPECT_TRUE(WaitForCallback(1000));
    EXPECT_EQ(lastStatus.state, DownloadState::DOWNLOAD_FAILED);
    EXPECT_EQ(lastStatus.version, "v1");
    EXPECT_EQ(lastStatus.errorCode, ERR_AUDIO_SUITE_SA_DOWNLOAD_FAILED);
}

HWTEST_F(DownloadManagerTest, NotifyDownloadFailedAfterRetryTest, TestSize.Level0)
{
    ResetCallbackState();
    manager->currentVersion_ = "v1";
    manager->downloadProgress_ = 30;
    manager->NotifyDownloadFailedAfterRetry();
    EXPECT_TRUE(WaitForCallback());
    EXPECT_EQ(lastStatus.state, DownloadState::DOWNLOAD_FAILED);
    EXPECT_EQ(lastStatus.version, "v1");
    EXPECT_EQ(lastStatus.errorCode, ERR_AUDIO_SUITE_SA_DOWNLOAD_FAILED);
    EXPECT_EQ(lastStatus.progress, 30u);
    EXPECT_EQ(manager->downloadRetryCount_.load(), 0);
}

HWTEST_F(DownloadManagerTest, OnCloudRomCallbackInstallSuccessTest, TestSize.Level0)
{
    ResetCallbackState();
    manager->currentVersion_ = "v1";
    CloudRomCallback callbackData = CreateCloudRomCallback("v1", CloudRomStatus::INSTALL_SUCCESS, 100);
    ErrCode ret = manager->OnCloudRomCallback(callbackData);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(WaitForCallback());
    EXPECT_EQ(lastStatus.state, DownloadState::DOWNLOAD_COMPLETED);
}

HWTEST_F(DownloadManagerTest, OnCloudRomCallbackDownloadSuccessTest, TestSize.Level0)
{
    ResetCallbackState();
    manager->currentVersion_ = "v1";
    CloudRomCallback callbackData = CreateCloudRomCallback("v1", CloudRomStatus::DOWNLOAD_SUCCESS, 100);
    ErrCode ret = manager->OnCloudRomCallback(callbackData);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(WaitForCallback());
    EXPECT_EQ(lastStatus.state, DownloadState::DOWNLOAD_COMPLETED);
}

HWTEST_F(DownloadManagerTest, OnCloudRomCallbackDownloadingTest, TestSize.Level0)
{
    ResetCallbackState();
    manager->currentVersion_ = "v1";
    CloudRomCallback callbackData = CreateCloudRomCallback("v1", CloudRomStatus::DOWNLOADING, 75);
    ErrCode ret = manager->OnCloudRomCallback(callbackData);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(WaitForCallback());
    EXPECT_EQ(lastStatus.state, DownloadState::DOWNLOAD_IN_PROGRESS);
    EXPECT_EQ(lastStatus.progress, 75u);
}

HWTEST_F(DownloadManagerTest, OnCloudRomCallbackDownloadFailTest, TestSize.Level0)
{
    ResetCallbackState();
    manager->currentVersion_ = "v1";
    manager->downloadRetryCount_ = MAX_RETRY_COUNT;
    manager->isDownloading_ = true;

    CloudRomCallback callbackData = CreateCloudRomCallback("v1", CloudRomStatus::DOWNLOAD_FAIL, 30);
    ErrCode ret = manager->OnCloudRomCallback(callbackData);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(WaitForCallback(1000));
    EXPECT_EQ(lastStatus.state, DownloadState::DOWNLOAD_FAILED);
}

HWTEST_F(DownloadManagerTest, ProcessCloudRomStatusDownloadingTest, TestSize.Level0)
{
    ResetCallbackState();
    manager->currentVersion_ = "v1";
    StatusInfo info = CreateStatusInfo("v1", CloudRomStatus::DOWNLOADING, 40);
    manager->ProcessCloudRomStatus(info);
    EXPECT_TRUE(WaitForCallback());
    EXPECT_EQ(lastStatus.state, DownloadState::DOWNLOAD_IN_PROGRESS);
}

HWTEST_F(DownloadManagerTest, ProcessCloudRomStatusInstallSuccessTest, TestSize.Level0)
{
    ResetCallbackState();
    manager->currentVersion_ = "v1";
    StatusInfo info = CreateStatusInfo("v1", CloudRomStatus::INSTALL_SUCCESS, 100, "/data");
    manager->ProcessCloudRomStatus(info);
    EXPECT_TRUE(WaitForCallback());
    EXPECT_EQ(lastStatus.state, DownloadState::DOWNLOAD_COMPLETED);
}

HWTEST_F(DownloadManagerTest, ProcessCloudRomStatusDownloadSuccessTest, TestSize.Level0)
{
    ResetCallbackState();
    manager->currentVersion_ = "v1";
    StatusInfo info = CreateStatusInfo("v1", CloudRomStatus::DOWNLOAD_SUCCESS, 100, "/data");
    manager->ProcessCloudRomStatus(info);
    EXPECT_TRUE(WaitForCallback());
    EXPECT_EQ(lastStatus.state, DownloadState::DOWNLOAD_COMPLETED);
}

HWTEST_F(DownloadManagerTest, ProcessCloudRomStatusDefaultTest, TestSize.Level0)
{
    ResetCallbackState();
    manager->currentVersion_ = "v1";
    StatusInfo info = CreateStatusInfo("v1", CloudRomStatus::CHECKING_VERSION, 0);
    manager->ProcessCloudRomStatus(info);
    EXPECT_FALSE(callbackInvoked);
}

HWTEST_F(DownloadManagerTest, UninstallCloudRomNoCallbackTest, TestSize.Level0)
{
    int32_t ret = manager->UninstallCloudRom("v1");
    EXPECT_EQ(ret, ERR_CALLBACK_NOT_REGISTERED);
}

HWTEST_F(DownloadManagerTest, GetDownloadStatusEmptyVersionTest, TestSize.Level0)
{
    DownloadStatus status;
    int32_t ret = manager->GetDownloadStatus(status, "");
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST_F(DownloadManagerTest, GetDownloadStatusNoCallbackTest, TestSize.Level0)
{
    DownloadStatus status;
    int32_t ret = manager->GetDownloadStatus(status, "v1");
    EXPECT_EQ(ret, ERR_CALLBACK_NOT_REGISTERED);
}

HWTEST_F(DownloadManagerTest, RegisterCallbackNoUpdateEngineTest, TestSize.Level0)
{
    bool ret = manager->RegisterCallback();
    EXPECT_FALSE(ret);
}

HWTEST_F(DownloadManagerTest, IsDownloadedNoCallbackTest, TestSize.Level0)
{
    bool ret = manager->IsDownloaded("v1", false);
    EXPECT_FALSE(ret);
}

HWTEST_F(DownloadManagerTest, GetRemoteInfoNoCallbackTest, TestSize.Level0)
{
    bool ret = manager->GetRemoteInfo("v1", false);
    EXPECT_FALSE(ret);
}

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS
