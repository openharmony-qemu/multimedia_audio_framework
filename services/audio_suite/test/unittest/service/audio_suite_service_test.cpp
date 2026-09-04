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
        (const std::string& featureName, AudioSuiteCloudRomVersionInfo& newVersion, int32_t& funcResult), (override));
    MOCK_METHOD(ErrCode, GetFeatureStatus,
        (const std::string& featureName, AudioSuiteDownloadStatusInfo& statusInfos, int32_t& funcResult), (override));
    MOCK_METHOD(ErrCode, DownloadAndInstall,
        (bool downloadCondition, const std::string& featureName, int32_t& funcResult), (override));
    MOCK_METHOD(ErrCode, PauseCloudRom,
        (const std::string& featureName, int32_t& funcResult), (override));
    MOCK_METHOD(ErrCode, CancelCloudRom,
        (const std::string& featureName, int32_t& funcResult), (override));
    MOCK_METHOD(ErrCode, UninstallCloudRom,
        (const std::string& featureName, int32_t& funcResult), (override));
    MOCK_METHOD(ErrCode, RegisterDownloadStatusCallback,
        (int32_t& funcResult), (override));
    MOCK_METHOD(ErrCode, UnregisterDownloadStatusCallback,
        (int32_t& funcResult), (override));

    int OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override
    {
        return 0;
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
    ErrCode ret = service->StartDownload("", true, result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, ERR_INVALID_PARAM);

    ret = service->StartDownload("v1", true, result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, ERR_CALLBACK_NOT_REGISTERED);
}

HWTEST_F(AudioSuiteServiceTest, CancelDownloadTest, TestSize.Level0)
{
    ErrCode result = SUCCESS;
    ErrCode ret = service->CancelDownload("", result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, ERR_INVALID_PARAM);

    ret = service->CancelDownload("v1", result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, ERR_CALLBACK_NOT_REGISTERED);
}

HWTEST_F(AudioSuiteServiceTest, PauseDownloadTest, TestSize.Level0)
{
    ErrCode result = SUCCESS;
    ErrCode ret = service->PauseDownload("", result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, ERR_INVALID_PARAM);

    ret = service->PauseDownload("v1", result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, ERR_CALLBACK_NOT_REGISTERED);
}

HWTEST_F(AudioSuiteServiceTest, GetDownloadStatusTest, TestSize.Level0)
{
    AudioSuiteDownloadStatusInfo status;
    ErrCode result = SUCCESS;
    ErrCode ret = service->GetDownloadStatus("", status, result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, ERR_INVALID_PARAM);

    ret = service->GetDownloadStatus("v1", status, result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, ERR_CALLBACK_NOT_REGISTERED);
}

HWTEST_F(AudioSuiteServiceTest, UninstallCloudRomTest, TestSize.Level0)
{
    ErrCode result = SUCCESS;
    ErrCode ret = service->UninstallCloudRom("", result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, ERR_INVALID_PARAM);

    ret = service->UninstallCloudRom("v1", result);
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
        .WillOnce(Invoke([](const std::string& name, AudioSuiteDownloadStatusInfo& status, int32_t& funcResult) {
            status.state = FEATURE_INSTALL_SUCCESS;
            status.featureName = name;
            status.errorCode = 0;
            funcResult = SUCCESS;
            return SUCCESS;
        }));
    EXPECT_CALL(*mockStub, GetFeatureNewVersion(_, _, _)).Times(0);

    AudioSuiteDownloadStatusInfo status;
    EXPECT_EQ(Singleton<DownloadManager>::GetInstance().GetDownloadStatus("feat1", status), SUCCESS);
    EXPECT_EQ(status.size, 0);

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
        .WillOnce(Invoke([](const std::string& name, AudioSuiteDownloadStatusInfo& status, int32_t& funcResult) {
            status.state = 0;
            status.featureName = name;
            status.errorCode = 0;
            funcResult = SUCCESS;
            return SUCCESS;
        }));
    EXPECT_CALL(*mockStub, GetFeatureNewVersion(_, _, _))
        .WillOnce(Invoke([](const std::string& name, AudioSuiteCloudRomVersionInfo& info, int32_t& funcResult) {
            info.featureName = name;
            info.errorCode = 0;
            info.size = 1024;
            funcResult = SUCCESS;
            return SUCCESS;
        }));

    AudioSuiteDownloadStatusInfo status;
    EXPECT_EQ(Singleton<DownloadManager>::GetInstance().GetDownloadStatus("feat1", status), SUCCESS);
    EXPECT_EQ(status.size, 1024);

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

    EXPECT_CALL(*mockStub, GetFeatureStatus(_, _, _))
        .WillRepeatedly(Invoke([](const std::string& name, AudioSuiteDownloadStatusInfo& status, int32_t& funcResult) {
            status.state = 0;
            status.featureName = name;
            status.errorCode = 0;
            funcResult = SUCCESS;
            return SUCCESS;
        }));
    EXPECT_CALL(*mockStub, GetFeatureNewVersion(_, _, _))
        .WillOnce(Invoke([](const std::string&, AudioSuiteCloudRomVersionInfo& info, int32_t& funcResult) {
            info.featureName = "feat1";
            info.errorCode = 0;
            funcResult = SUCCESS;
            return ERROR; // condition: ret != SUCCESS
        }))
        .WillOnce(Invoke([](const std::string&, AudioSuiteCloudRomVersionInfo& info, int32_t& funcResult) {
            info.featureName = "feat1";
            info.errorCode = 0;
            funcResult = ERROR; // condition: funcResult != SUCCESS
            return SUCCESS;
        }))
        .WillOnce(Invoke([](const std::string&, AudioSuiteCloudRomVersionInfo& info, int32_t& funcResult) {
            info.featureName = "feat1";
            info.errorCode = -1; // condition: errorCode != 0
            funcResult = SUCCESS;
            return SUCCESS;
        }))
        .WillOnce(Invoke([](const std::string&, AudioSuiteCloudRomVersionInfo& info, int32_t& funcResult) {
            info.featureName = "otherFeature"; // condition: featureName mismatch
            info.errorCode = 0;
            funcResult = SUCCESS;
            return SUCCESS;
        }));

    AudioSuiteDownloadStatusInfo status;
    EXPECT_EQ(Singleton<DownloadManager>::GetInstance().GetDownloadStatus("feat1", status), SUCCESS);
    EXPECT_EQ(status.size, 0);
    EXPECT_EQ(Singleton<DownloadManager>::GetInstance().GetDownloadStatus("feat1", status), SUCCESS);
    EXPECT_EQ(status.size, 0);
    EXPECT_EQ(Singleton<DownloadManager>::GetInstance().GetDownloadStatus("feat1", status), SUCCESS);
    EXPECT_EQ(status.size, 0);
    EXPECT_EQ(Singleton<DownloadManager>::GetInstance().GetDownloadStatus("feat1", status), SUCCESS);
    EXPECT_EQ(status.size, 0);

    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(nullptr);
}

/**
 * @tc.name  SetUpdateEngineCallback_FirstCallNoRemove_001
 * @tc.desc  Test SetUpdateEngineCallback when called for the first time,
 *           audioServerdeathRecipient_ is nullptr so RemoveDeathRecipient branch is skipped,
 *           should not crash and callback should be registered.
 */
HWTEST_F(AudioSuiteServiceTest, SetUpdateEngineCallback_FirstCallNoRemove_001, TestSize.Level0)
{
    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(nullptr);
 
    auto mockStub = sptr<MockDownloadCallbackStub>(new MockDownloadCallbackStub());
    int32_t result = Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(mockStub);
    EXPECT_EQ(result, SUCCESS);
 
    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(nullptr);
}
 
/**
 * @tc.name  SetUpdateEngineCallback_RemoveOldDeathRecipient_001
 * @tc.desc  Test SetUpdateEngineCallback when called twice with different objects,
 *           the second call enters the RemoveDeathRecipient branch
 *           (audioServerdeathRecipient_ != nullptr && downloadCallback_ != nullptr).
 */
HWTEST_F(AudioSuiteServiceTest, SetUpdateEngineCallback_RemoveOldDeathRecipient_001, TestSize.Level0)
{
    auto mockStub1 = sptr<MockDownloadCallbackStub>(new MockDownloadCallbackStub());
    auto mockStub2 = sptr<MockDownloadCallbackStub>(new MockDownloadCallbackStub());
 
    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(mockStub1);
    int32_t result = Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(mockStub2);
    EXPECT_EQ(result, SUCCESS);
 
    // Verify active callback is mockStub2
    EXPECT_CALL(*mockStub2, GetFeatureStatus(_, _, _))
        .WillOnce(Invoke([](const std::string& name, AudioSuiteDownloadStatusInfo& status, int32_t& funcResult) {
            status.state = FEATURE_INSTALL_SUCCESS;
            status.featureName = name;
            status.errorCode = 0;
            funcResult = SUCCESS;
            return SUCCESS;
        }));
    EXPECT_CALL(*mockStub1, GetFeatureStatus(_, _, _)).Times(0);
 
    AudioSuiteDownloadStatusInfo status;
    EXPECT_EQ(Singleton<DownloadManager>::GetInstance().GetDownloadStatus("feat1", status), SUCCESS);
 
    Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(nullptr);
}

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS
