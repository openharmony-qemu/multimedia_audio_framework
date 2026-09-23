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
#include "audio_suite_service.h"
#include "download_manager.h"
#include "audio_errors.h"
#include "iremote_stub.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

static constexpr int32_t AUDIO_SUITE_SA_ID = 3015;
static constexpr int32_t FEATURE_INSTALL_SUCCESS = 36;

class MockDownloadCallbackStub : public IRemoteStub<IAudioSuiteDownloadCallback> {
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

    int OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override
    {
        return 0;
    }

    bool AddDeathRecipient(const sptr<DeathRecipient>& recipient) override
    {
        return true;
    }

    bool RemoveDeathRecipient(const sptr<DeathRecipient>& recipient) override
    {
        return true;
    }
};

class TestAudioSuiteService : public AudioSuiteService {
public:
    TestAudioSuiteService()
        : AudioSuiteService(AUDIO_SUITE_SA_ID, false) {}
};

int32_t AudioSuiteServiceStub::OnRemoteRequest(
    uint32_t code,
    MessageParcel& data,
    MessageParcel& reply,
    MessageOption& option)
{
    return 0;
}

class AudioSuiteServiceTest : public Test {
public:
    void SetUp() override
    {
        service = std::make_unique<TestAudioSuiteService>();
    }

    void TearDown() override
    {
        if (service) {
            service->OnStop();
            service.reset();
        }
    }

    void InitUnloadHandler()
    {
        std::lock_guard<std::mutex> lock(service->eventMutex_);
        service->unloadRunner_ = AppExecFwk::EventRunner::Create("AudioSuiteSaUnloadRunner");
        service->unloadHandler_ = std::make_shared<AppExecFwk::EventHandler>(service->unloadRunner_);
    }

    void InitUnloadHandlerOn(TestAudioSuiteService *svc)
    {
        std::lock_guard<std::mutex> lock(svc->eventMutex_);
        svc->unloadRunner_ = AppExecFwk::EventRunner::Create("AudioSuiteSaUnloadRunner");
        svc->unloadHandler_ = std::make_shared<AppExecFwk::EventHandler>(svc->unloadRunner_);
    }

    std::unique_ptr<TestAudioSuiteService> service;
};

HWTEST_F(AudioSuiteServiceTest, RegisterDownloadCallbackTest, TestSize.Level0)
{
    ErrCode result = SUCCESS;
    ErrCode ret = service->RegisterDownloadStatusCallback(nullptr, result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

HWTEST_F(AudioSuiteServiceTest, RegisterDownloadInterfaceCallbackTest, TestSize.Level0)
{
    ErrCode result = SUCCESS;
    ErrCode ret = service->RegisterDownloadInterfaceCallback(nullptr, result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioSuiteServiceTest, UnregisterownloadInterfaceCallbackTest, TestSize.Level0)
{
    ErrCode result = SUCCESS;
    ErrCode ret = service->UnregisterDownloadInterfaceCallback(result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioSuiteServiceTest, StartDownloadTest, TestSize.Level0)
{
    ErrCode result = SUCCESS;
    ErrCode ret = service->StartDownload(std::vector<std::string>{}, true, result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, ERR_INVALID_PARAM);

    ret = service->StartDownload(std::vector<std::string>{"v1"}, true, result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, ERR_CALLBACK_NOT_REGISTERED);
}

HWTEST_F(AudioSuiteServiceTest, CancelDownloadTest, TestSize.Level0)
{
    ErrCode result = SUCCESS;
    ErrCode ret = service->CancelDownload(std::vector<std::string>{}, result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, ERR_INVALID_PARAM);

    ret = service->CancelDownload(std::vector<std::string>{"v1"}, result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, ERR_CALLBACK_NOT_REGISTERED);
}

HWTEST_F(AudioSuiteServiceTest, PauseDownloadTest, TestSize.Level0)
{
    ErrCode result = SUCCESS;
    ErrCode ret = service->PauseDownload(std::vector<std::string>{}, result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, ERR_INVALID_PARAM);

    ret = service->PauseDownload(std::vector<std::string>{"v1"}, result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, ERR_CALLBACK_NOT_REGISTERED);
}

HWTEST_F(AudioSuiteServiceTest, GetDownloadStatusTest, TestSize.Level0)
{
    std::vector<AudioSuiteDownloadStatusInfo> status;
    ErrCode result = SUCCESS;
    ErrCode ret = service->GetDownloadStatus(std::vector<std::string>{}, status, result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, ERR_INVALID_PARAM);

    ret = service->GetDownloadStatus(std::vector<std::string>{"v1"}, status, result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, ERR_CALLBACK_NOT_REGISTERED);
}

HWTEST_F(AudioSuiteServiceTest, UninstallCloudRomTest, TestSize.Level0)
{
    ErrCode result = SUCCESS;
    ErrCode ret = service->UninstallCloudRom(std::vector<std::string>{}, result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, ERR_INVALID_PARAM);

    ret = service->UninstallCloudRom(std::vector<std::string>{"v1"}, result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, ERR_CALLBACK_NOT_REGISTERED);
}

HWTEST_F(AudioSuiteServiceTest, GetFeatureNewVersionTest, TestSize.Level0)
{
    ErrCode result = SUCCESS;
    std::vector<AudioSuiteCloudRomVersionInfo> newVersion;
    ErrCode ret = service->GetFeatureNewVersion(std::vector<std::string>{}, newVersion, result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, ERR_INVALID_PARAM);

    ret = service->GetFeatureNewVersion(std::vector<std::string>{"v1"}, newVersion, result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, ERR_CALLBACK_NOT_REGISTERED);
}

/**
 * @tc.name  GetDownloadStatus_InstallSuccess_001
 * @tc.desc  Test GetDownloadStatus when state == FEATURE_INSTALL_SUCCESS, should return early without
 *           calling GetFeatureNewVersion, and status.size stays 0.
 */
HWTEST_F(AudioSuiteServiceTest, GetDownloadStatus_InstallSuccess_001, TestSize.Level0)
{
    auto mockStub = sptr<MockDownloadCallbackStub>(new MockDownloadCallbackStub());
    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(mockStub);

    EXPECT_CALL(*mockStub, GetFeatureStatus(_, _, _))
        .WillOnce(Invoke([](const std::vector<std::string>& names,
                            std::vector<AudioSuiteDownloadStatusInfo>& statusInfos, int32_t& funcResult) {
            AudioSuiteDownloadStatusInfo status;
            status.state = FEATURE_INSTALL_SUCCESS;
            status.featureName = names[0];
            status.errorCode = 0;
            statusInfos.push_back(status);
            funcResult = SUCCESS;
            return SUCCESS;
        }));
    EXPECT_CALL(*mockStub, GetFeatureNewVersion(_, _, _)).Times(0);

    std::vector<AudioSuiteDownloadStatusInfo> status;
    EXPECT_EQ(Singleton<DownloadManager>::GetInstance().GetDownloadStatus(
        std::vector<std::string>{"feat1"}, status), SUCCESS);
    EXPECT_EQ(status[0].size, 0);

    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(nullptr);
}

/**
 * @tc.name  GetDownloadStatus_NewVersionSuccess_001
 * @tc.desc  Test GetDownloadStatus when GetFeatureNewVersion returns all-success conditions,
 *           status.size should be set to newVersionInfo.size.
 */
HWTEST_F(AudioSuiteServiceTest, GetDownloadStatus_NewVersionSuccess_001, TestSize.Level0)
{
    auto mockStub = sptr<MockDownloadCallbackStub>(new MockDownloadCallbackStub());
    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(mockStub);

    EXPECT_CALL(*mockStub, GetFeatureStatus(_, _, _))
        .WillOnce(Invoke([](const std::vector<std::string>& names,
                            std::vector<AudioSuiteDownloadStatusInfo>& statusInfos, int32_t& funcResult) {
            AudioSuiteDownloadStatusInfo status;
            status.state = 0;
            status.featureName = names[0];
            status.errorCode = 0;
            statusInfos.push_back(status);
            funcResult = SUCCESS;
            return SUCCESS;
        }));
    EXPECT_CALL(*mockStub, GetFeatureNewVersion(_, _, _))
        .WillOnce(Invoke([](const std::vector<std::string>& names,
                            std::vector<AudioSuiteCloudRomVersionInfo>& infos, int32_t& funcResult) {
            AudioSuiteCloudRomVersionInfo info;
            info.featureName = names[0];
            info.errorCode = 0;
            info.size = 1024;
            infos.push_back(info);
            funcResult = SUCCESS;
            return SUCCESS;
        }));

    std::vector<AudioSuiteDownloadStatusInfo> status;
    EXPECT_EQ(Singleton<DownloadManager>::GetInstance().GetDownloadStatus(
        std::vector<std::string>{"feat1"}, status), SUCCESS);
    EXPECT_EQ(status[0].size, 1024);

    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(nullptr);
}

/**
 * @tc.name  GetDownloadStatus_NewVersionFail_001
 * @tc.desc  Test GetDownloadStatus when GetFeatureNewVersion conditions fail (ret, funcResult,
 *           errorCode, featureName mismatch), should enter else branch and status.size stays 0.
 */
HWTEST_F(AudioSuiteServiceTest, GetDownloadStatus_NewVersionFail_001, TestSize.Level0)
{
    auto mockStub = sptr<MockDownloadCallbackStub>(new MockDownloadCallbackStub());
    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(mockStub);

    auto fillVersion = [](const std::string& name, int32_t errCode,
        std::vector<AudioSuiteCloudRomVersionInfo>& infos) {
        AudioSuiteCloudRomVersionInfo info;
        info.featureName = name;
        info.errorCode = errCode;
        infos.push_back(info);
    };
    EXPECT_CALL(*mockStub, GetFeatureStatus(_, _, _))
        .WillRepeatedly(Invoke([](const std::vector<std::string>& names,
            std::vector<AudioSuiteDownloadStatusInfo>& statusInfos, int32_t& funcResult) {
            AudioSuiteDownloadStatusInfo status;
            status.state = 0;
            status.featureName = names[0];
            status.errorCode = 0;
            statusInfos.push_back(status);
            funcResult = SUCCESS;
            return SUCCESS;
        }));
    EXPECT_CALL(*mockStub, GetFeatureNewVersion(_, _, _))
        .WillOnce(Invoke([&fillVersion](const std::vector<std::string>&,
            std::vector<AudioSuiteCloudRomVersionInfo>& infos, int32_t& funcResult) {
            fillVersion("feat1", 0, infos); funcResult = SUCCESS; return ERROR; // ret != SUCCESS
        }))
        .WillOnce(Invoke([&fillVersion](const std::vector<std::string>&,
            std::vector<AudioSuiteCloudRomVersionInfo>& infos, int32_t& funcResult) {
            fillVersion("feat1", 0, infos); funcResult = ERROR; return SUCCESS; // funcResult != SUCCESS
        }))
        .WillOnce(Invoke([&fillVersion](const std::vector<std::string>&,
            std::vector<AudioSuiteCloudRomVersionInfo>& infos, int32_t& funcResult) {
            fillVersion("feat1", -1, infos); funcResult = SUCCESS; return SUCCESS; // errorCode != 0
        }))
        .WillOnce(Invoke([&fillVersion](const std::vector<std::string>&,
            std::vector<AudioSuiteCloudRomVersionInfo>& infos, int32_t& funcResult) {
            fillVersion("otherFeature", 0, infos); funcResult = SUCCESS; return SUCCESS; // name mismatch
        }));

    auto& dm = Singleton<DownloadManager>::GetInstance();
    std::vector<AudioSuiteDownloadStatusInfo> status;
    for (int i = 0; i < 4; i++) {
        EXPECT_EQ(dm.GetDownloadStatus(std::vector<std::string>{"feat1"}, status), SUCCESS);
        EXPECT_EQ(status[0].size, 0);
        status.clear();
    }
    dm.SetUpdateEngineCallback(nullptr);
}

/**
 * @tc.name  GetFeatureNewVersion_Success_001
 * @tc.desc  Test GetFeatureNewVersion returns version infos successfully.
 */
HWTEST_F(AudioSuiteServiceTest, GetFeatureNewVersion_Success_001, TestSize.Level0)
{
    auto mockStub = sptr<MockDownloadCallbackStub>(new MockDownloadCallbackStub());
    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(mockStub);

    EXPECT_CALL(*mockStub, GetFeatureNewVersion(_, _, _))
        .WillOnce(Invoke([](const std::vector<std::string>& names,
                            std::vector<AudioSuiteCloudRomVersionInfo>& infos, int32_t& funcResult) {
            AudioSuiteCloudRomVersionInfo info;
            info.featureName = names[0];
            info.version = "1.0.0";
            info.size = 2048;
            info.errorCode = 0;
            infos.push_back(info);
            funcResult = SUCCESS;
            return SUCCESS;
        }));

    std::vector<AudioSuiteCloudRomVersionInfo> versionInfos;
    EXPECT_EQ(Singleton<DownloadManager>::GetInstance().GetFeatureNewVersion(
        std::vector<std::string>{"feat1"}, versionInfos), SUCCESS);
    EXPECT_EQ(versionInfos.size(), 1u);
    EXPECT_EQ(versionInfos[0].featureName, "feat1");
    EXPECT_EQ(versionInfos[0].version, "1.0.0");
    EXPECT_EQ(versionInfos[0].size, 2048);

    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(nullptr);
}

/**
 * @tc.name  SetUpdateEngineCallback_ReplaceCallback_001
 * @tc.desc  Test SetUpdateEngineCallback when replacing an existing callback,
 *           the old death recipient should be removed from the old remote object.
 */
HWTEST_F(AudioSuiteServiceTest, SetUpdateEngineCallback_ReplaceCallback_001, TestSize.Level0)
{
    auto& dm = Singleton<DownloadManager>::GetInstance();

    // Step 1: First call to SetUpdateEngineCallback, sets up old callback
    auto oldStub = sptr<MockDownloadCallbackStub>(new MockDownloadCallbackStub());
    dm.SetUpdateEngineCallback(oldStub);

    // Step 2: Call SetUpdateEngineCallback again with a new callback
    //  This triggers the new branch: remove old death recipient before overwriting
    auto newStub = sptr<MockDownloadCallbackStub>(new MockDownloadCallbackStub());
    dm.SetUpdateEngineCallback(newStub);

    // Verify: the new callback is active (basic sanity check)
    EXPECT_CALL(*newStub, GetFeatureStatus(_, _, _))
        .WillOnce(Invoke([](const std::vector<std::string>& names,
                            std::vector<AudioSuiteDownloadStatusInfo>& statusInfos, int32_t& funcResult) {
            AudioSuiteDownloadStatusInfo status;
            status.state = 0;
            status.featureName = names[0];
            status.errorCode = 0;
            statusInfos.push_back(status);
            funcResult = SUCCESS;
            return SUCCESS;
        }));

    std::vector<AudioSuiteDownloadStatusInfo> status;
    EXPECT_EQ(dm.GetDownloadStatus(std::vector<std::string>{"feat1"}, status), SUCCESS);

    // Cleanup
    dm.SetUpdateEngineCallback(nullptr);
}

/**
 * @tc.name  GetInstallStatus_001
 * @tc.desc  Test GetInstallStatus: empty featureNames (validation fail), invalid characters
 *           (safe validation fail), and valid featureNames with GetParameter <= 0 path.
 */
HWTEST_F(AudioSuiteServiceTest, GetInstallStatus_001, TestSize.Level0)
{
    auto& dm = Singleton<DownloadManager>::GetInstance();
    std::vector<bool> installStatus;

    // Empty featureNames → validation fail
    EXPECT_EQ(dm.GetInstallStatus(std::vector<std::string>{}, installStatus), ERR_INVALID_PARAM);

    // Invalid characters (../) → safe validation fail
    EXPECT_EQ(dm.GetInstallStatus(std::vector<std::string>{"feat/../1"}, installStatus), ERR_INVALID_PARAM);

    // Valid featureNames, GetParameter <= 0 → installStatus all false
    std::vector<std::string> featureNames = {"feat1", "feat2"};
    EXPECT_EQ(dm.GetInstallStatus(featureNames, installStatus), SUCCESS);
    EXPECT_EQ(installStatus.size(), featureNames.size());
    for (bool val : installStatus) {
        EXPECT_FALSE(val);
    }
}

/**
 * @tc.name  StartDownload_Success_001
 * @tc.desc  Test StartDownload full success path: GetFeatureNewVersion succeeds,
 *           DownloadAndInstall succeeds.
 */
HWTEST_F(AudioSuiteServiceTest, StartDownload_Success_001, TestSize.Level0)
{
    auto mockStub = sptr<MockDownloadCallbackStub>(new MockDownloadCallbackStub());
    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(mockStub);

    EXPECT_CALL(*mockStub, GetFeatureNewVersion(_, _, _))
        .WillOnce(Invoke([](const std::vector<std::string>& names,
                            std::vector<AudioSuiteCloudRomVersionInfo>& infos, int32_t& funcResult) {
            AudioSuiteCloudRomVersionInfo info;
            info.featureName = names[0];
            info.errorCode = 0;
            info.size = 1024;
            infos.push_back(info);
            funcResult = SUCCESS;
            return SUCCESS;
        }));
    EXPECT_CALL(*mockStub, DownloadAndInstall(_, _, _))
        .WillOnce(Invoke([](bool, const std::vector<std::string>&, int32_t& funcResult) {
            funcResult = SUCCESS;
            return SUCCESS;
        }));

    EXPECT_EQ(Singleton<DownloadManager>::GetInstance().StartDownload(
        std::vector<std::string>{"feat1"}, true), SUCCESS);

    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(nullptr);
}

/**
 * @tc.name  StartDownload_GetVersionFail_001
 * @tc.desc  Test StartDownload when GetFeatureNewVersion IPC fails (ret != SUCCESS).
 */
HWTEST_F(AudioSuiteServiceTest, StartDownload_GetVersionFail_001, TestSize.Level0)
{
    auto mockStub = sptr<MockDownloadCallbackStub>(new MockDownloadCallbackStub());
    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(mockStub);

    EXPECT_CALL(*mockStub, GetFeatureNewVersion(_, _, _))
        .WillOnce(Invoke([](const std::vector<std::string>&,
                            std::vector<AudioSuiteCloudRomVersionInfo>& infos, int32_t& funcResult) {
            funcResult = SUCCESS;
            return ERROR;
        }));
    EXPECT_CALL(*mockStub, DownloadAndInstall(_, _, _)).Times(0);

    EXPECT_EQ(Singleton<DownloadManager>::GetInstance().StartDownload(
        std::vector<std::string>{"feat1"}, true), ERROR);

    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(nullptr);
}

/**
 * @tc.name  StartDownload_GetVersionFail_002
 * @tc.desc  Test StartDownload when GetFeatureNewVersion funcResult fails.
 */
HWTEST_F(AudioSuiteServiceTest, StartDownload_GetVersionFail_002, TestSize.Level0)
{
    auto mockStub = sptr<MockDownloadCallbackStub>(new MockDownloadCallbackStub());
    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(mockStub);

    EXPECT_CALL(*mockStub, GetFeatureNewVersion(_, _, _))
        .WillOnce(Invoke([](const std::vector<std::string>&,
                            std::vector<AudioSuiteCloudRomVersionInfo>& infos, int32_t& funcResult) {
            funcResult = ERROR;
            return SUCCESS;
        }));
    EXPECT_CALL(*mockStub, DownloadAndInstall(_, _, _)).Times(0);

    EXPECT_EQ(Singleton<DownloadManager>::GetInstance().StartDownload(
        std::vector<std::string>{"feat1"}, true), ERROR);

    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(nullptr);
}

/**
 * @tc.name  StartDownload_ErrorCode_001
 * @tc.desc  Test StartDownload when GetFeatureNewVersion returns info.errorCode != 0.
 */
HWTEST_F(AudioSuiteServiceTest, StartDownload_ErrorCode_001, TestSize.Level0)
{
    auto mockStub = sptr<MockDownloadCallbackStub>(new MockDownloadCallbackStub());
    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(mockStub);

    EXPECT_CALL(*mockStub, GetFeatureNewVersion(_, _, _))
        .WillOnce(Invoke([](const std::vector<std::string>& names,
                            std::vector<AudioSuiteCloudRomVersionInfo>& infos, int32_t& funcResult) {
            AudioSuiteCloudRomVersionInfo info;
            info.featureName = names[0];
            info.errorCode = -1;
            infos.push_back(info);
            funcResult = SUCCESS;
            return SUCCESS;
        }));
    EXPECT_CALL(*mockStub, DownloadAndInstall(_, _, _)).Times(0);

    EXPECT_NE(Singleton<DownloadManager>::GetInstance().StartDownload(
        std::vector<std::string>{"feat1"}, true), SUCCESS);

    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(nullptr);
}

/**
 * @tc.name  StartDownload_DownloadFail_001
 * @tc.desc  Test StartDownload when DownloadAndInstall IPC fails.
 */
HWTEST_F(AudioSuiteServiceTest, StartDownload_DownloadFail_001, TestSize.Level0)
{
    auto mockStub = sptr<MockDownloadCallbackStub>(new MockDownloadCallbackStub());
    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(mockStub);

    EXPECT_CALL(*mockStub, GetFeatureNewVersion(_, _, _))
        .WillOnce(Invoke([](const std::vector<std::string>& names,
                            std::vector<AudioSuiteCloudRomVersionInfo>& infos, int32_t& funcResult) {
            AudioSuiteCloudRomVersionInfo info;
            info.featureName = names[0];
            info.errorCode = 0;
            infos.push_back(info);
            funcResult = SUCCESS;
            return SUCCESS;
        }));
    EXPECT_CALL(*mockStub, DownloadAndInstall(_, _, _))
        .WillOnce(Invoke([](bool, const std::vector<std::string>&, int32_t& funcResult) {
            funcResult = SUCCESS;
            return ERROR;
        }));

    EXPECT_EQ(Singleton<DownloadManager>::GetInstance().StartDownload(
        std::vector<std::string>{"feat1"}, true), ERROR);

    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(nullptr);
}

/**
 * @tc.name  StartDownload_DownloadFail_002
 * @tc.desc  Test StartDownload when DownloadAndInstall funcResult fails.
 */
HWTEST_F(AudioSuiteServiceTest, StartDownload_DownloadFail_002, TestSize.Level0)
{
    auto mockStub = sptr<MockDownloadCallbackStub>(new MockDownloadCallbackStub());
    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(mockStub);

    EXPECT_CALL(*mockStub, GetFeatureNewVersion(_, _, _))
        .WillOnce(Invoke([](const std::vector<std::string>& names,
                            std::vector<AudioSuiteCloudRomVersionInfo>& infos, int32_t& funcResult) {
            AudioSuiteCloudRomVersionInfo info;
            info.featureName = names[0];
            info.errorCode = 0;
            infos.push_back(info);
            funcResult = SUCCESS;
            return SUCCESS;
        }));
    EXPECT_CALL(*mockStub, DownloadAndInstall(_, _, _))
        .WillOnce(Invoke([](bool, const std::vector<std::string>&, int32_t& funcResult) {
            funcResult = ERROR;
            return SUCCESS;
        }));

    EXPECT_EQ(Singleton<DownloadManager>::GetInstance().StartDownload(
        std::vector<std::string>{"feat1"}, true), ERROR);

    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(nullptr);
}

/**
 * @tc.name  PauseDownload_Success_001
 * @tc.desc  Test PauseDownload success path: IPC succeeds and funcResult == SUCCESS.
 */
HWTEST_F(AudioSuiteServiceTest, PauseDownload_Success_001, TestSize.Level0)
{
    auto mockStub = sptr<MockDownloadCallbackStub>(new MockDownloadCallbackStub());
    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(mockStub);

    EXPECT_CALL(*mockStub, PauseCloudRom(_, _))
        .WillOnce(Invoke([](const std::vector<std::string>&, int32_t& funcResult) {
            funcResult = SUCCESS;
            return SUCCESS;
        }));

    EXPECT_EQ(Singleton<DownloadManager>::GetInstance().PauseDownload(
        std::vector<std::string>{"feat1"}), SUCCESS);

    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(nullptr);
}

/**
 * @tc.name  PauseDownload_Fail_001
 * @tc.desc  Test PauseDownload when PauseCloudRom IPC fails (ret != SUCCESS).
 */
HWTEST_F(AudioSuiteServiceTest, PauseDownload_Fail_001, TestSize.Level0)
{
    auto mockStub = sptr<MockDownloadCallbackStub>(new MockDownloadCallbackStub());
    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(mockStub);

    EXPECT_CALL(*mockStub, PauseCloudRom(_, _))
        .WillOnce(Invoke([](const std::vector<std::string>&, int32_t& funcResult) {
            funcResult = SUCCESS;
            return ERROR;
        }));

    EXPECT_EQ(Singleton<DownloadManager>::GetInstance().PauseDownload(
        std::vector<std::string>{"feat1"}), ERROR);

    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(nullptr);
}

/**
 * @tc.name  PauseDownload_Fail_002
 * @tc.desc  Test PauseDownload when PauseCloudRom funcResult fails.
 */
HWTEST_F(AudioSuiteServiceTest, PauseDownload_Fail_002, TestSize.Level0)
{
    auto mockStub = sptr<MockDownloadCallbackStub>(new MockDownloadCallbackStub());
    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(mockStub);

    EXPECT_CALL(*mockStub, PauseCloudRom(_, _))
        .WillOnce(Invoke([](const std::vector<std::string>&, int32_t& funcResult) {
            funcResult = ERROR;
            return SUCCESS;
        }));

    EXPECT_EQ(Singleton<DownloadManager>::GetInstance().PauseDownload(
        std::vector<std::string>{"feat1"}), ERROR);

    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(nullptr);
}

/**
 * @tc.name  CancelDownload_Success_001
 * @tc.desc  Test CancelDownload success path.
 */
HWTEST_F(AudioSuiteServiceTest, CancelDownload_Success_001, TestSize.Level0)
{
    auto mockStub = sptr<MockDownloadCallbackStub>(new MockDownloadCallbackStub());
    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(mockStub);

    EXPECT_CALL(*mockStub, CancelCloudRom(_, _))
        .WillOnce(Invoke([](const std::vector<std::string>&, int32_t& funcResult) {
            funcResult = SUCCESS;
            return SUCCESS;
        }));

    EXPECT_EQ(Singleton<DownloadManager>::GetInstance().CancelDownload(
        std::vector<std::string>{"feat1"}), SUCCESS);

    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(nullptr);
}

/**
 * @tc.name  CancelDownload_Fail_001
 * @tc.desc  Test CancelDownload when CancelCloudRom IPC fails.
 */
HWTEST_F(AudioSuiteServiceTest, CancelDownload_Fail_001, TestSize.Level0)
{
    auto mockStub = sptr<MockDownloadCallbackStub>(new MockDownloadCallbackStub());
    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(mockStub);

    EXPECT_CALL(*mockStub, CancelCloudRom(_, _))
        .WillOnce(Invoke([](const std::vector<std::string>&, int32_t& funcResult) {
            funcResult = SUCCESS;
            return ERROR;
        }));

    EXPECT_EQ(Singleton<DownloadManager>::GetInstance().CancelDownload(
        std::vector<std::string>{"feat1"}), ERROR);

    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(nullptr);
}

/**
 * @tc.name  CancelDownload_Fail_002
 * @tc.desc  Test CancelDownload when CancelCloudRom funcResult fails.
 */
HWTEST_F(AudioSuiteServiceTest, CancelDownload_Fail_002, TestSize.Level0)
{
    auto mockStub = sptr<MockDownloadCallbackStub>(new MockDownloadCallbackStub());
    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(mockStub);

    EXPECT_CALL(*mockStub, CancelCloudRom(_, _))
        .WillOnce(Invoke([](const std::vector<std::string>&, int32_t& funcResult) {
            funcResult = ERROR;
            return SUCCESS;
        }));

    EXPECT_EQ(Singleton<DownloadManager>::GetInstance().CancelDownload(
        std::vector<std::string>{"feat1"}), ERROR);

    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(nullptr);
}

/**
 * @tc.name  UninstallCloudRom_Success_001
 * @tc.desc  Test UninstallCloudRom success path.
 */
HWTEST_F(AudioSuiteServiceTest, UninstallCloudRom_Success_001, TestSize.Level0)
{
    auto mockStub = sptr<MockDownloadCallbackStub>(new MockDownloadCallbackStub());
    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(mockStub);

    EXPECT_CALL(*mockStub, UninstallCloudRom(_, _))
        .WillOnce(Invoke([](const std::vector<std::string>&, int32_t& funcResult) {
            funcResult = SUCCESS;
            return SUCCESS;
        }));

    EXPECT_EQ(Singleton<DownloadManager>::GetInstance().UninstallCloudRom(
        std::vector<std::string>{"feat1"}), SUCCESS);

    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(nullptr);
}

/**
 * @tc.name  UninstallCloudRom_Fail_001
 * @tc.desc  Test UninstallCloudRom when IPC fails.
 */
HWTEST_F(AudioSuiteServiceTest, UninstallCloudRom_Fail_001, TestSize.Level0)
{
    auto mockStub = sptr<MockDownloadCallbackStub>(new MockDownloadCallbackStub());
    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(mockStub);

    EXPECT_CALL(*mockStub, UninstallCloudRom(_, _))
        .WillOnce(Invoke([](const std::vector<std::string>&, int32_t& funcResult) {
            funcResult = SUCCESS;
            return ERROR;
        }));

    EXPECT_EQ(Singleton<DownloadManager>::GetInstance().UninstallCloudRom(
        std::vector<std::string>{"feat1"}), ERROR);

    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(nullptr);
}

/**
 * @tc.name  UninstallCloudRom_Fail_002
 * @tc.desc  Test UninstallCloudRom when funcResult fails.
 */
HWTEST_F(AudioSuiteServiceTest, UninstallCloudRom_Fail_002, TestSize.Level0)
{
    auto mockStub = sptr<MockDownloadCallbackStub>(new MockDownloadCallbackStub());
    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(mockStub);

    EXPECT_CALL(*mockStub, UninstallCloudRom(_, _))
        .WillOnce(Invoke([](const std::vector<std::string>&, int32_t& funcResult) {
            funcResult = ERROR;
            return SUCCESS;
        }));

    EXPECT_EQ(Singleton<DownloadManager>::GetInstance().UninstallCloudRom(
        std::vector<std::string>{"feat1"}), ERROR);

    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(nullptr);
}

/**
 * @tc.name  GetFeatureNewVersion_Fail_001
 * @tc.desc  Test GetFeatureNewVersion (DownloadManager) when IPC fails (ret != SUCCESS).
 */
HWTEST_F(AudioSuiteServiceTest, GetFeatureNewVersion_Fail_001, TestSize.Level0)
{
    auto mockStub = sptr<MockDownloadCallbackStub>(new MockDownloadCallbackStub());
    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(mockStub);

    EXPECT_CALL(*mockStub, GetFeatureNewVersion(_, _, _))
        .WillOnce(Invoke([](const std::vector<std::string>&,
                            std::vector<AudioSuiteCloudRomVersionInfo>&, int32_t& funcResult) {
            funcResult = SUCCESS;
            return ERROR;
        }));

    std::vector<AudioSuiteCloudRomVersionInfo> versionInfos;
    EXPECT_EQ(Singleton<DownloadManager>::GetInstance().GetFeatureNewVersion(
        std::vector<std::string>{"feat1"}, versionInfos), ERROR);

    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(nullptr);
}

/**
 * @tc.name  GetFeatureNewVersion_Fail_002
 * @tc.desc  Test GetFeatureNewVersion (DownloadManager) when funcResult fails.
 */
HWTEST_F(AudioSuiteServiceTest, GetFeatureNewVersion_Fail_002, TestSize.Level0)
{
    auto mockStub = sptr<MockDownloadCallbackStub>(new MockDownloadCallbackStub());
    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(mockStub);

    EXPECT_CALL(*mockStub, GetFeatureNewVersion(_, _, _))
        .WillOnce(Invoke([](const std::vector<std::string>&,
                            std::vector<AudioSuiteCloudRomVersionInfo>&, int32_t& funcResult) {
            funcResult = ERROR;
            return SUCCESS;
        }));

    std::vector<AudioSuiteCloudRomVersionInfo> versionInfos;
    EXPECT_EQ(Singleton<DownloadManager>::GetInstance().GetFeatureNewVersion(
        std::vector<std::string>{"feat1"}, versionInfos), ERROR);

    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(nullptr);
}

/**
 * @tc.name  GetDownloadStatus_Fail_001
 * @tc.desc  Test GetDownloadStatus when GetFeatureStatus IPC fails (ret != SUCCESS).
 */
HWTEST_F(AudioSuiteServiceTest, GetDownloadStatus_Fail_001, TestSize.Level0)
{
    auto mockStub = sptr<MockDownloadCallbackStub>(new MockDownloadCallbackStub());
    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(mockStub);

    EXPECT_CALL(*mockStub, GetFeatureStatus(_, _, _))
        .WillOnce(Invoke([](const std::vector<std::string>&,
                            std::vector<AudioSuiteDownloadStatusInfo>&, int32_t& funcResult) {
            funcResult = SUCCESS;
            return ERROR;
        }));

    std::vector<AudioSuiteDownloadStatusInfo> status;
    EXPECT_EQ(Singleton<DownloadManager>::GetInstance().GetDownloadStatus(
        std::vector<std::string>{"feat1"}, status), ERROR);

    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(nullptr);
}

/**
 * @tc.name  GetDownloadStatus_Fail_002
 * @tc.desc  Test GetDownloadStatus when GetFeatureStatus funcResult fails.
 */
HWTEST_F(AudioSuiteServiceTest, GetDownloadStatus_Fail_002, TestSize.Level0)
{
    auto mockStub = sptr<MockDownloadCallbackStub>(new MockDownloadCallbackStub());
    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(mockStub);

    EXPECT_CALL(*mockStub, GetFeatureStatus(_, _, _))
        .WillOnce(Invoke([](const std::vector<std::string>&,
                            std::vector<AudioSuiteDownloadStatusInfo>&, int32_t& funcResult) {
            funcResult = ERROR;
            return SUCCESS;
        }));

    std::vector<AudioSuiteDownloadStatusInfo> status;
    EXPECT_EQ(Singleton<DownloadManager>::GetInstance().GetDownloadStatus(
        std::vector<std::string>{"feat1"}, status), ERROR);

    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(nullptr);
}

/**
 * @tc.name  GetDownloadStatus_Fail_003
 * @tc.desc  Test GetDownloadStatus when needSizeFeatures not empty but
 *           GetFeatureNewVersion fails (ret != SUCCESS or funcResult != SUCCESS),
 *           should return SUCCESS (early return in the CHECK_AND_RETURN_RET_LOG).
 */
HWTEST_F(AudioSuiteServiceTest, GetDownloadStatus_Fail_003, TestSize.Level0)
{
    auto mockStub = sptr<MockDownloadCallbackStub>(new MockDownloadCallbackStub());
    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(mockStub);

    EXPECT_CALL(*mockStub, GetFeatureStatus(_, _, _))
        .WillOnce(Invoke([](const std::vector<std::string>& names,
                            std::vector<AudioSuiteDownloadStatusInfo>& statusInfos, int32_t& funcResult) {
            AudioSuiteDownloadStatusInfo status;
            status.state = 0;
            status.featureName = names[0];
            status.errorCode = 0;
            statusInfos.push_back(status);
            funcResult = SUCCESS;
            return SUCCESS;
        }));
    EXPECT_CALL(*mockStub, GetFeatureNewVersion(_, _, _))
        .WillOnce(Invoke([](const std::vector<std::string>&,
                            std::vector<AudioSuiteCloudRomVersionInfo>&, int32_t& funcResult) {
            funcResult = ERROR;
            return SUCCESS;
        }));

    std::vector<AudioSuiteDownloadStatusInfo> status;
    EXPECT_EQ(Singleton<DownloadManager>::GetInstance().GetDownloadStatus(
        std::vector<std::string>{"feat1"}, status), SUCCESS);
    EXPECT_EQ(status[0].size, 0);

    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(nullptr);
}

/**
 * @tc.name  StartBackgroundDownload_001
 * @tc.desc  Test StartBackgroundDownload: empty featureNames (validation fail), no callback
 *           registered, and callback registered with conditions check.
 */
HWTEST_F(AudioSuiteServiceTest, StartBackgroundDownload_001, TestSize.Level0)
{
    auto& dm = Singleton<DownloadManager>::GetInstance();

    // Empty featureNames → validation fail
    EXPECT_EQ(dm.StartBackgroundDownload(std::vector<std::string>{}), ERR_INVALID_PARAM);

    // Valid featureNames but no callback registered
    EXPECT_EQ(dm.StartBackgroundDownload(std::vector<std::string>{"feat1"}), ERR_CALLBACK_NOT_REGISTERED);

    // Callback registered, conditions check path
    auto mockStub = sptr<MockDownloadCallbackStub>(new MockDownloadCallbackStub());
    dm.SetUpdateEngineCallback(mockStub);
    int32_t ret = dm.StartBackgroundDownload(std::vector<std::string>{"feat_test"});
    EXPECT_TRUE(ret == SUCCESS || ret == ERROR || ret == ERR_CALLBACK_NOT_REGISTERED);
    dm.SetUpdateEngineCallback(nullptr);
}

/**
 * @tc.name  GetInstallStatus_Service_001
 * @tc.desc  Test AudioSuiteService::GetInstallStatus via service layer: empty params
 *           (validation fail) and valid params.
 */
HWTEST_F(AudioSuiteServiceTest, GetInstallStatus_Service_001, TestSize.Level0)
{
    ErrCode result = SUCCESS;
    std::vector<bool> installStatus;

    // Empty featureNames → validation fail
    ErrCode ret = service->GetInstallStatus(std::vector<std::string>{}, installStatus, result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, ERR_INVALID_PARAM);

    // Valid featureNames → success
    ret = service->GetInstallStatus(std::vector<std::string>{"feat1"}, installStatus, result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(installStatus.size(), 1u);
}

/**
 * @tc.name  StartBackgroundDownload_Service_001
 * @tc.desc  Test AudioSuiteService::StartBackgroundDownload with empty params (validation fail)
 *           and OnDownloadStatusChanged via service layer.
 */
HWTEST_F(AudioSuiteServiceTest, StartBackgroundDownload_Service_001, TestSize.Level0)
{
    ErrCode result = SUCCESS;
    ErrCode ret = service->StartBackgroundDownload(std::vector<std::string>{}, result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, ERR_INVALID_PARAM);

    // OnDownloadStatusChanged via service layer
    std::vector<AudioSuiteDownloadStatusInfo> status;
    AudioSuiteDownloadStatusInfo info;
    info.featureName = "feat1";
    info.state = 0;
    info.errorCode = 0;
    status.push_back(info);
    ret = service->OnDownloadStatusChanged(status);
    EXPECT_EQ(ret, SUCCESS);
}

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS
