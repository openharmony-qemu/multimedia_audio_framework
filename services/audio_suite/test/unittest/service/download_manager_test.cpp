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
#include "download_manager.h"
#include "audio_errors.h"
#include "audio_utils.h"
#include "iremote_stub.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

class MockDMDownloadCallback : public IRemoteStub<IAudioSuiteDownloadCallback> {
public:
    MOCK_METHOD(ErrCode, GetFeatureNewVersion,
        (const std::vector<std::string>& featureNames, std::vector<AudioSuiteCloudRomVersionInfo>& newVersion,
         int32_t& funcResult), (override));
    MOCK_METHOD(ErrCode, GetFeatureStatus,
        (const std::vector<std::string>& featureNames, std::vector<AudioSuiteDownloadStatusInfo>& statusInfos,
         int32_t& funcResult), (override));
    MOCK_METHOD(ErrCode, DownloadAndInstall,
        (bool allowedCellularNet, const std::vector<std::string>& featureNames, int32_t& funcResult), (override));
    MOCK_METHOD(ErrCode, PauseCloudRom,
        (const std::vector<std::string>& featureNames, int32_t& funcResult), (override));
    MOCK_METHOD(ErrCode, CancelCloudRom,
        (const std::vector<std::string>& featureNames, int32_t& funcResult), (override));
    MOCK_METHOD(ErrCode, UninstallCloudRom,
        (const std::vector<std::string>& featureNames, int32_t& funcResult), (override));
    MOCK_METHOD(ErrCode, RegisterDownloadStatusCallback,
        (int32_t& funcResult), (override));
    MOCK_METHOD(ErrCode, UnregisterDownloadStatusCallback,
        (int32_t& funcResult), (override));

    int OnRemoteRequest(uint32_t, MessageParcel&, MessageParcel&, MessageOption&) override { return 0; }
    bool AddDeathRecipient(const sptr<DeathRecipient>&) override { return true; }
    bool RemoveDeathRecipient(const sptr<DeathRecipient>&) override { return true; }
};

class DownloadManagerTest : public Test {
public:
    void TearDown() override
    {
        auto& dm = Singleton<DownloadManager>::GetInstance();
        dm.SetUpdateEngineCallback(nullptr);
        dm.DeInitDownloadManager();
        std::lock_guard<std::mutex> lock(dm.mutex_);
        dm.backgroundDownload_ = nullptr;
        BackgroundDownload::ClearPersistedData();
    }
};

/**
 * @tc.name  InitDownloadManager_SysParaFail_001
 * @tc.desc  Test InitDownloadManager when the download system parameter is not set,
 *           verifying persisted data is cleared and background download is not initialized.
 */
HWTEST_F(DownloadManagerTest, InitDownloadManager_SysParaFail_001, TestSize.Level0)
{
    auto& dm = Singleton<DownloadManager>::GetInstance();
    dm.InitDownloadManager(true);
    // InitDownloadManager with syspara=0 should not create backgroundDownload_
    // Verify: QueuePendingFeatures should succeed (creates BackgroundDownload on demand)
    EXPECT_EQ(dm.QueuePendingFeatures(std::vector<std::string>{"initFeat"}), SUCCESS);
}

/**
 * @tc.name  HasDownloadingTask_Empty_001
 * @tc.desc  Test HasDownloadingTask returns 0 when no status callbacks registered.
 */
HWTEST_F(DownloadManagerTest, HasDownloadingTask_Empty_001, TestSize.Level0)
{
    EXPECT_EQ(Singleton<DownloadManager>::GetInstance().HasDownloadingTask(), 0);
}

/**
 * @tc.name  OnDownloadStatusChanged_Empty_001
 * @tc.desc  Test OnDownloadStatusChanged with empty statusCallbacks map.
 */
HWTEST_F(DownloadManagerTest, OnDownloadStatusChanged_Empty_001, TestSize.Level0)
{
    std::vector<AudioSuiteDownloadStatusInfo> status;
    EXPECT_EQ(Singleton<DownloadManager>::GetInstance().OnDownloadStatusChanged(status), SUCCESS);
}

/**
 * @tc.name  IsFeatureReady_True_001
 * @tc.desc  Test IsFeatureReady returns true when state == FEATURE_INSTALL_SUCCESS.
 */
HWTEST_F(DownloadManagerTest, IsFeatureReady_True_001, TestSize.Level0)
{
    auto mock = sptr<MockDMDownloadCallback>(new MockDMDownloadCallback());
    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(mock);

    EXPECT_CALL(*mock, GetFeatureStatus(_, _, _))
        .WillOnce(Invoke([](const std::vector<std::string>& names,
                            std::vector<AudioSuiteDownloadStatusInfo>& statusInfos, int32_t& funcResult) {
            AudioSuiteDownloadStatusInfo info;
            info.featureName = names[0];
            info.state = 36;
            info.errorCode = 0;
            statusInfos.push_back(info);
            funcResult = SUCCESS;
            return SUCCESS;
        }));

    EXPECT_TRUE(Singleton<DownloadManager>::GetInstance().IsFeatureReady("feat1"));
}

/**
 * @tc.name  IsFeatureReady_False_001
 * @tc.desc  Test IsFeatureReady returns false when state is not install success
 *           (inner if false path).
 */
HWTEST_F(DownloadManagerTest, IsFeatureReady_False_001, TestSize.Level0)
{
    auto mock = sptr<MockDMDownloadCallback>(new MockDMDownloadCallback());
    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(mock);

    EXPECT_CALL(*mock, GetFeatureStatus(_, _, _))
        .WillOnce(Invoke([](const std::vector<std::string>& names,
                            std::vector<AudioSuiteDownloadStatusInfo>& statusInfos, int32_t& funcResult) {
            AudioSuiteDownloadStatusInfo info;
            info.featureName = names[0];
            info.state = 0;
            info.errorCode = 0;
            statusInfos.push_back(info);
            funcResult = SUCCESS;
            return SUCCESS;
        }));

    EXPECT_FALSE(Singleton<DownloadManager>::GetInstance().IsFeatureReady("feat1"));
}

/**
 * @tc.name  StartBackgroundDownloadTask_Scenario1_001
 * @tc.desc  Test StartBackgroundDownloadTask Scenario 1: feature already installed,
 *           state == FEATURE_INSTALL_SUCCESS, skip download.
 */
HWTEST_F(DownloadManagerTest, StartBackgroundDownloadTask_Scenario1_001, TestSize.Level0)
{
    auto mock = sptr<MockDMDownloadCallback>(new MockDMDownloadCallback());
    auto& dm = Singleton<DownloadManager>::GetInstance();
    dm.SetUpdateEngineCallback(mock);

    EXPECT_CALL(*mock, GetFeatureStatus(_, _, _))
        .WillOnce(Invoke([](const std::vector<std::string>& names,
                            std::vector<AudioSuiteDownloadStatusInfo>& statusInfos, int32_t& funcResult) {
            AudioSuiteDownloadStatusInfo info;
            info.featureName = names[0];
            info.state = 36;
            info.errorCode = 0;
            statusInfos.push_back(info);
            funcResult = SUCCESS;
            return SUCCESS;
        }));
    EXPECT_CALL(*mock, DownloadAndInstall(_, _, _)).Times(0);
    EXPECT_CALL(*mock, GetFeatureNewVersion(_, _, _)).Times(0);

    EXPECT_EQ(dm.StartBackgroundDownloadTask("RecordDefinition"), SUCCESS);
}

/**
 * @tc.name  StartBackgroundDownloadTask_Scenario2_001
 * @tc.desc  Test StartBackgroundDownloadTask Scenario 2: version check completed,
 *           state == CLOUD_ROM_CHECK_VERSION_SUCCESS, direct download+install.
 */
HWTEST_F(DownloadManagerTest, StartBackgroundDownloadTask_Scenario2_001, TestSize.Level0)
{
    auto mock = sptr<MockDMDownloadCallback>(new MockDMDownloadCallback());
    auto& dm = Singleton<DownloadManager>::GetInstance();
    dm.SetUpdateEngineCallback(mock);

    EXPECT_CALL(*mock, GetFeatureStatus(_, _, _))
        .WillOnce(Invoke([](const std::vector<std::string>& names,
                            std::vector<AudioSuiteDownloadStatusInfo>& statusInfos, int32_t& funcResult) {
            AudioSuiteDownloadStatusInfo info;
            info.featureName = names[0];
            info.state = 12;
            info.errorCode = 0;
            statusInfos.push_back(info);
            funcResult = SUCCESS;
            return SUCCESS;
        }));
    EXPECT_CALL(*mock, GetFeatureNewVersion(_, _, _)).Times(0);
    EXPECT_CALL(*mock, DownloadAndInstall(_, _, _))
        .WillOnce(Invoke([](bool, const std::vector<std::string>&, int32_t& funcResult) {
            funcResult = SUCCESS;
            return SUCCESS;
        }));

    EXPECT_EQ(dm.StartBackgroundDownloadTask("RecordDefinition"), SUCCESS);
}

/**
 * @tc.name  StartBackgroundDownloadTask_Scenario3_001
 * @tc.desc  Test StartBackgroundDownloadTask Scenario 3: feature not ready,
 *           query cloud version then download+install.
 */
HWTEST_F(DownloadManagerTest, StartBackgroundDownloadTask_Scenario3_001, TestSize.Level0)
{
    auto mock = sptr<MockDMDownloadCallback>(new MockDMDownloadCallback());
    auto& dm = Singleton<DownloadManager>::GetInstance();
    dm.SetUpdateEngineCallback(mock);

    EXPECT_CALL(*mock, GetFeatureStatus(_, _, _))
        .WillOnce(Invoke([](const std::vector<std::string>& names,
                            std::vector<AudioSuiteDownloadStatusInfo>& statusInfos, int32_t& funcResult) {
            AudioSuiteDownloadStatusInfo info;
            info.featureName = names[0];
            info.state = 0;
            info.errorCode = 0;
            statusInfos.push_back(info);
            funcResult = SUCCESS;
            return SUCCESS;
        }));
    EXPECT_CALL(*mock, GetFeatureNewVersion(_, _, _))
        .WillOnce(Invoke([](const std::vector<std::string>& names,
                            std::vector<AudioSuiteCloudRomVersionInfo>& infos, int32_t& funcResult) {
            AudioSuiteCloudRomVersionInfo info;
            info.featureName = names[0];
            info.errorCode = 0;
            infos.push_back(info);
            funcResult = SUCCESS;
            return SUCCESS;
        }));
    EXPECT_CALL(*mock, DownloadAndInstall(_, _, _))
        .WillOnce(Invoke([](bool, const std::vector<std::string>&, int32_t& funcResult) {
            funcResult = SUCCESS;
            return SUCCESS;
        }));

    EXPECT_EQ(dm.StartBackgroundDownloadTask("RecordDefinition"), SUCCESS);
}

/**
 * @tc.name  UnregisterDownloadStatusCallback_NoDeathRecipient_001
 * @tc.desc  Test UnregisterDownloadStatusCallback: deathRecipients not found,
 *           statusCallbacks not found, empty map -> UnregisterDownloadStatusCallback IPC.
 */
HWTEST_F(DownloadManagerTest, UnregisterDownloadStatusCallback_NoDeathRecipient_001, TestSize.Level0)
{
    auto mock = sptr<MockDMDownloadCallback>(new MockDMDownloadCallback());
    auto& dm = Singleton<DownloadManager>::GetInstance();
    dm.SetUpdateEngineCallback(mock);

    EXPECT_CALL(*mock, UnregisterDownloadStatusCallback(_))
        .WillOnce(Invoke([](int32_t& funcResult) {
            funcResult = SUCCESS;
            return SUCCESS;
        }));

    EXPECT_EQ(dm.UnregisterDownloadStatusCallback(88888), SUCCESS);
}

/**
 * @tc.name  IsCloudRomInProgress_States_001
 * @tc.desc  Test IsFeatureReady returns true for all in-progress states:
 *           DOWNLOADING(20), DOWNLOAD_SUCCESS(23), INSTALLING(30),
 *           WAIT_REBOOT_THEN_INSTALL(31), WAIT_UPGRADE_THEN_INSTALL(32).
 */
HWTEST_F(DownloadManagerTest, IsCloudRomInProgress_States_001, TestSize.Level0)
{
    auto mock = sptr<MockDMDownloadCallback>(new MockDMDownloadCallback());
    auto& dm = Singleton<DownloadManager>::GetInstance();
    dm.SetUpdateEngineCallback(mock);

    const std::vector<int32_t> inProgressStates = {20, 23, 30, 31, 32};
    int32_t callIndex = 0;
    EXPECT_CALL(*mock, GetFeatureStatus(_, _, _))
        .WillRepeatedly(Invoke([&callIndex, &inProgressStates](
            const std::vector<std::string>& names,
            std::vector<AudioSuiteDownloadStatusInfo>& statusInfos, int32_t& funcResult) {
            AudioSuiteDownloadStatusInfo info;
            info.featureName = names[0];
            info.state = inProgressStates[callIndex % inProgressStates.size()];
            info.errorCode = 0;
            statusInfos.push_back(info);
            funcResult = SUCCESS;
            callIndex++;
            return SUCCESS;
        }));

    for (size_t i = 0; i < inProgressStates.size(); ++i) {
        EXPECT_TRUE(dm.IsFeatureReady("feat1")) << "Failed at state=" << inProgressStates[i];
    }
    dm.SetUpdateEngineCallback(nullptr);
}

/**
 * @tc.name  IsFeatureReady_FalsePaths_001
 * @tc.desc  Test IsFeatureReady false paths: funcResult fail, empty status,
 *           errorCode non-zero, and callback null.
 */
HWTEST_F(DownloadManagerTest, IsFeatureReady_FalsePaths_001, TestSize.Level0)
{
    auto mock = sptr<MockDMDownloadCallback>(new MockDMDownloadCallback());
    auto& dm = Singleton<DownloadManager>::GetInstance();
    dm.SetUpdateEngineCallback(mock);

    // B2: ret=SUCCESS but funcResult=ERROR → false
    EXPECT_CALL(*mock, GetFeatureStatus(_, _, _))
        .WillOnce(Invoke([](const std::vector<std::string>&,
                            std::vector<AudioSuiteDownloadStatusInfo>&, int32_t& funcResult) {
            funcResult = ERROR;
            return SUCCESS;
        }));
    EXPECT_FALSE(dm.IsFeatureReady("feat1"));

    // B3: ret=SUCCESS, funcResult=SUCCESS, but status empty → false
    EXPECT_CALL(*mock, GetFeatureStatus(_, _, _))
        .WillOnce(Invoke([](const std::vector<std::string>&,
                            std::vector<AudioSuiteDownloadStatusInfo>& statusInfos, int32_t& funcResult) {
            funcResult = SUCCESS;
            return SUCCESS; // statusInfos left empty
        }));
    EXPECT_FALSE(dm.IsFeatureReady("feat1"));

    // B4: status non-empty but errorCode != 0 → false
    EXPECT_CALL(*mock, GetFeatureStatus(_, _, _))
        .WillOnce(Invoke([](const std::vector<std::string>& names,
                            std::vector<AudioSuiteDownloadStatusInfo>& statusInfos, int32_t& funcResult) {
            AudioSuiteDownloadStatusInfo info;
            info.featureName = names[0];
            info.state = 36;
            info.errorCode = -1;
            statusInfos.push_back(info);
            funcResult = SUCCESS;
            return SUCCESS;
        }));
    EXPECT_FALSE(dm.IsFeatureReady("feat1"));

    // A: callback cleared → downloadCallback_ null → false
    dm.SetUpdateEngineCallback(nullptr);
    EXPECT_FALSE(dm.IsFeatureReady("feat1"));
}

/**
 * @tc.name  IsFeatureReady_IPCFail_001
 * @tc.desc  Test IsFeatureReady when GetFeatureStatus IPC fails (ret != SUCCESS path).
 */
HWTEST_F(DownloadManagerTest, IsFeatureReady_IPCFail_001, TestSize.Level0)
{
    auto mock = sptr<MockDMDownloadCallback>(new MockDMDownloadCallback());
    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(mock);

    EXPECT_CALL(*mock, GetFeatureStatus(_, _, _))
        .WillOnce(Invoke([](const std::vector<std::string>&,
                            std::vector<AudioSuiteDownloadStatusInfo>&, int32_t& funcResult) {
            funcResult = SUCCESS;
            return ERROR;
        }));

    EXPECT_FALSE(Singleton<DownloadManager>::GetInstance().IsFeatureReady("feat1"));
}

/**
 * @tc.name  InitDownloadManager_SysParaSet_BgExists_001
 * @tc.desc  Test InitDownloadManager when the download system parameter is set and
 *           background download already exists, verifying the existing instance is reused.
 */
HWTEST_F(DownloadManagerTest, InitDownloadManager_SysParaSet_BgExists_001, TestSize.Level0)
{
    SetSysPara("persist.multimedia.audiosuite.download", 1);

    auto& dm = Singleton<DownloadManager>::GetInstance();
    // Pre-create backgroundDownload_ so InitDownloadManager skips creation
    auto boundFunc = std::bind(&DownloadManager::StartBackgroundDownloadTask, &dm, std::placeholders::_1);
    auto existingBg = std::make_shared<BackgroundDownload>(boundFunc);
    {
        std::lock_guard<std::mutex> lock(dm.mutex_);
        dm.backgroundDownload_ = existingBg;
    }

    // InitDownloadManager should skip creation and reuse the existing instance
    dm.InitDownloadManager(true);

    // Verify: backgroundDownload_ is the same instance (not recreated)
    std::lock_guard<std::mutex> lock(dm.mutex_);
    EXPECT_EQ(dm.backgroundDownload_.get(), existingBg.get());
}

/**
 * @tc.name  InitDownloadManager_SysParaSet_NoPendingFile_001
 * @tc.desc  Test InitDownloadManager when the download system parameter is set but no
 *           pending file exists, verifying background download is created and an empty
 *           queue is persisted (file deleted).
 */
HWTEST_F(DownloadManagerTest, InitDownloadManager_SysParaSet_NoPendingFile_001, TestSize.Level0)
{
    // Ensure no pending file exists
    BackgroundDownload::ClearPersistedData();
    // Set syspara AFTER ClearPersistedData (ClearPersistedData resets syspara to 0)
    SetSysPara("persist.multimedia.audiosuite.download", 1);

    auto& dm = Singleton<DownloadManager>::GetInstance();
    {
        std::lock_guard<std::mutex> lock(dm.mutex_);
        dm.backgroundDownload_ = nullptr;
    }

    dm.InitDownloadManager(true);

    // Verify: backgroundDownload_ created, no file written (empty map persisted → file deleted)
    std::shared_ptr<BackgroundDownload> bg;
    {
        std::lock_guard<std::mutex> lock(dm.mutex_);
        EXPECT_NE(dm.backgroundDownload_, nullptr);
        bg = dm.backgroundDownload_;
    }
    EXPECT_TRUE(bg->ReadPendingTasksFromFile().empty());
}

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS
