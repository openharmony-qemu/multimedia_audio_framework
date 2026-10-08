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
#include "audio_errors.h"
#include "audio_suite_client_download_manager.h"
#include "audio_suite_client_policy.h"
#include "iaudio_suite_service.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

class MockDownloadStatusCallback : public DownloadStatusCallback {
public:
    MOCK_METHOD(void, OnDownloadStatusChanged,
        (const std::vector<AudioSuiteDownloadStatusInfo>& status), (override));
};

class MockAudioSuiteRomDownloadCallback : public AudioSuiteRomDownloadCallback {
public:
    MOCK_METHOD(int32_t, GetFeatureNewVersion,
                (const std::vector<std::string>& featureNames,
                 std::vector<AudioSuiteCloudRomVersionInfo>& newfeatureNameInfos),
                (override));
    MOCK_METHOD(int32_t, GetFeatureStatus,
                (const std::vector<std::string>& featureNames,
                 std::vector<AudioSuiteDownloadStatusInfo>& statusInfos),
                (override));
    MOCK_METHOD(int32_t, DownloadAndInstall,
                (const bool allowedCellularNet, const std::vector<std::string>& featureNames),
                (override));
    MOCK_METHOD(int32_t, PauseCloudRom,
                (const std::vector<std::string>& featureNames), (override));
    MOCK_METHOD(int32_t, CancelCloudRom,
                (const std::vector<std::string>& featureNames), (override));
    MOCK_METHOD(int32_t, UninstallCloudRom,
                (const std::vector<std::string>& featureNames), (override));
    MOCK_METHOD(int32_t, RegisterDownloadStatusCallback, (), (override));
    MOCK_METHOD(int32_t, UnregisterDownloadStatusCallback, (), (override));
};

class MockIAudioSuiteService : public IAudioSuiteService {
public:
    MOCK_METHOD(ErrCode, RegisterDownloadStatusCallback,
        (const sptr<IRemoteObject>& object, int32_t& funcResult), (override));
    MOCK_METHOD(ErrCode, UnregisterDownloadStatusCallback,
        (int32_t& funcResult), (override));
    MOCK_METHOD(ErrCode, GetInstallStatus,
        (const std::vector<std::string>& featureNames, std::vector<bool>& installStatus, ErrCode& result), (override));
    MOCK_METHOD(ErrCode, StartDownload,
        (const std::vector<std::string>& featureNames, bool allowedCellularNet, int32_t& funcResult), (override));
    MOCK_METHOD(ErrCode, PauseDownload,
        (const std::vector<std::string>& featureNames, int32_t& funcResult), (override));
    MOCK_METHOD(ErrCode, CancelDownload,
        (const std::vector<std::string>& featureNames, int32_t& funcResult), (override));
    MOCK_METHOD(ErrCode, StartBackgroundDownload,
        (const std::vector<std::string>& featureNames, int32_t& funcResult), (override));
    MOCK_METHOD(ErrCode, GetDownloadStatus,
        (const std::vector<std::string>& featureNames,
         std::vector<AudioSuiteDownloadStatusInfo>& status, int32_t& funcResult), (override));
    MOCK_METHOD(ErrCode, GetFeatureStatus,
        (const std::vector<std::string>& featureNames,
         std::vector<AudioSuiteDownloadStatusInfo>& status, int32_t& funcResult), (override));
    MOCK_METHOD(ErrCode, GetFeatureNewVersion,
        (const std::vector<std::string>& featureNames,
         std::vector<AudioSuiteCloudRomVersionInfo>& newVersion, int32_t& funcResult), (override));
    MOCK_METHOD(ErrCode, UninstallCloudRom,
        (const std::vector<std::string>& featureNames, int32_t& funcResult), (override));
    MOCK_METHOD(ErrCode, RegisterDownloadInterfaceCallback,
        (const sptr<IRemoteObject>& object, int32_t& funcResult), (override));
    MOCK_METHOD(ErrCode, UnregisterDownloadInterfaceCallback,
        (int32_t& funcResult), (override));
    MOCK_METHOD(ErrCode, OnDownloadStatusChanged,
        (const std::vector<AudioSuiteDownloadStatusInfo>& status), (override));

    sptr<IRemoteObject> AsObject() override
    {
        return nullptr;
    }
};

class AudioSuiteDownloadManagerUnitTest : public Test {
public:
    void SetUp() override
    {
        mockService_ = new MockIAudioSuiteService();
        AudioSuiteClientPolicy::GetInstance().proxy_ = mockService_;
        AudioSuiteClientPolicy::GetInstance().connected_ = true;
    }

    void TearDown() override
    {
        AudioSuiteClientPolicy::GetInstance().proxy_ = nullptr;
        AudioSuiteClientPolicy::GetInstance().connected_ = false;
    }

    sptr<MockIAudioSuiteService> mockService_;
};

HWTEST_F(AudioSuiteDownloadManagerUnitTest, RegisterDownloadStatusCallback_001, TestSize.Level0)
{
    int32_t ret = AudioSuiteDownloadManager::GetInstance().RegisterDownloadStatusCallback(nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST_F(AudioSuiteDownloadManagerUnitTest, RegisterDownloadStatusCallback_002, TestSize.Level0)
{
    int32_t result = SUCCESS;
    EXPECT_CALL(*mockService_, RegisterDownloadStatusCallback(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(result), Return(SUCCESS)));

    auto callback = std::make_shared<MockDownloadStatusCallback>();
    int32_t ret = AudioSuiteDownloadManager::GetInstance().RegisterDownloadStatusCallback(callback);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuiteDownloadManagerUnitTest, StartDownload_001, TestSize.Level0)
{
    int32_t result = SUCCESS;
    EXPECT_CALL(*mockService_, StartDownload(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(result), Return(SUCCESS)));

    int32_t ret = AudioSuiteDownloadManager::GetInstance().StartDownload(std::vector<std::string>{"v1"}, true);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuiteDownloadManagerUnitTest, StartDownload_002, TestSize.Level0)
{
    int32_t result = ERR_INVALID_PARAM;
    EXPECT_CALL(*mockService_, StartDownload(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(result), Return(SUCCESS)));

    int32_t ret = AudioSuiteDownloadManager::GetInstance().StartDownload(std::vector<std::string>{"v1"}, true);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST_F(AudioSuiteDownloadManagerUnitTest, PauseDownload_001, TestSize.Level0)
{
    int32_t result = SUCCESS;
    EXPECT_CALL(*mockService_, PauseDownload(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(result), Return(SUCCESS)));

    int32_t ret = AudioSuiteDownloadManager::GetInstance().PauseDownload(std::vector<std::string>{"v1"});
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuiteDownloadManagerUnitTest, PauseDownload_002, TestSize.Level0)
{
    int32_t result = ERR_INVALID_PARAM;
    EXPECT_CALL(*mockService_, PauseDownload(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(result), Return(SUCCESS)));

    int32_t ret = AudioSuiteDownloadManager::GetInstance().PauseDownload(std::vector<std::string>{"v1"});
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST_F(AudioSuiteDownloadManagerUnitTest, CancelDownload_001, TestSize.Level0)
{
    int32_t result = SUCCESS;
    EXPECT_CALL(*mockService_, CancelDownload(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(result), Return(SUCCESS)));

    int32_t ret = AudioSuiteDownloadManager::GetInstance().CancelDownload(std::vector<std::string>{"v1"});
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuiteDownloadManagerUnitTest, CancelDownload_002, TestSize.Level0)
{
    int32_t result = ERR_INVALID_PARAM;
    EXPECT_CALL(*mockService_, CancelDownload(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(result), Return(SUCCESS)));

    int32_t ret = AudioSuiteDownloadManager::GetInstance().CancelDownload(std::vector<std::string>{"v1"});
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST_F(AudioSuiteDownloadManagerUnitTest, StartBackgroundDownload_001, TestSize.Level0)
{
    int32_t result = SUCCESS;
    EXPECT_CALL(*mockService_, StartBackgroundDownload(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(result), Return(SUCCESS)));

    int32_t ret = AudioSuiteDownloadManager::GetInstance().StartBackgroundDownload(std::vector<std::string>{"v1"});
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuiteDownloadManagerUnitTest, StartBackgroundDownload_002, TestSize.Level0)
{
    int32_t result = ERR_INVALID_PARAM;
    EXPECT_CALL(*mockService_, StartBackgroundDownload(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(result), Return(SUCCESS)));

    int32_t ret = AudioSuiteDownloadManager::GetInstance().StartBackgroundDownload(std::vector<std::string>{"v1"});
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST_F(AudioSuiteDownloadManagerUnitTest, GetDownloadStatus_001, TestSize.Level0)
{
    std::vector<AudioSuiteDownloadStatusInfo> status;
    int32_t result = SUCCESS;
    EXPECT_CALL(*mockService_, GetDownloadStatus(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(result), Return(SUCCESS)));

    int32_t ret = AudioSuiteDownloadManager::GetInstance().GetDownloadStatus(
        std::vector<std::string>{"v1"}, status);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuiteDownloadManagerUnitTest, GetDownloadStatus_002, TestSize.Level0)
{
    std::vector<AudioSuiteDownloadStatusInfo> status;
    int32_t result = ERR_INVALID_PARAM;
    EXPECT_CALL(*mockService_, GetDownloadStatus(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(result), Return(SUCCESS)));

    int32_t ret = AudioSuiteDownloadManager::GetInstance().GetDownloadStatus(
        std::vector<std::string>{"v1"}, status);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST_F(AudioSuiteDownloadManagerUnitTest, GetFeatureNewVersion_001, TestSize.Level0)
{
    int32_t result = SUCCESS;
    EXPECT_CALL(*mockService_, GetFeatureNewVersion(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(result), Return(SUCCESS)));

    std::vector<AudioSuiteCloudRomVersionInfo> versionInfos;
    int32_t ret = AudioSuiteDownloadManager::GetInstance().GetFeatureNewVersion(
        std::vector<std::string>{"v1"}, versionInfos);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuiteDownloadManagerUnitTest, GetFeatureNewVersion_002, TestSize.Level0)
{
    int32_t result = ERR_INVALID_PARAM;
    EXPECT_CALL(*mockService_, GetFeatureNewVersion(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(result), Return(SUCCESS)));

    std::vector<AudioSuiteCloudRomVersionInfo> versionInfos;
    int32_t ret = AudioSuiteDownloadManager::GetInstance().GetFeatureNewVersion(
        std::vector<std::string>{"v1"}, versionInfos);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST_F(AudioSuiteDownloadManagerUnitTest, UninstallCloudRom_001, TestSize.Level0)
{
    int32_t result = SUCCESS;
    EXPECT_CALL(*mockService_, UninstallCloudRom(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(result), Return(SUCCESS)));

    int32_t ret = AudioSuiteDownloadManager::GetInstance().UninstallCloudRom(std::vector<std::string>{"v1"});
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuiteDownloadManagerUnitTest, UninstallCloudRom_002, TestSize.Level0)
{
    int32_t result = ERR_INVALID_PARAM;
    EXPECT_CALL(*mockService_, UninstallCloudRom(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(result), Return(SUCCESS)));

    int32_t ret = AudioSuiteDownloadManager::GetInstance().UninstallCloudRom(std::vector<std::string>{"v1"});
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST_F(AudioSuiteDownloadManagerUnitTest, RegisterDownloadInterfaceCallback_001, TestSize.Level0)
{
    int32_t ret = AudioSuiteDownloadManager::GetInstance().RegisterDownloadInterfaceCallback(nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST_F(AudioSuiteDownloadManagerUnitTest, RegisterDownloadInterfaceCallback_002, TestSize.Level0)
{
    int32_t result = SUCCESS;
    EXPECT_CALL(*mockService_, RegisterDownloadInterfaceCallback(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(result), Return(SUCCESS)));

    auto callback = std::make_shared<MockAudioSuiteRomDownloadCallback>();
    int32_t ret = AudioSuiteDownloadManager::GetInstance().RegisterDownloadInterfaceCallback(callback);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuiteDownloadManagerUnitTest, UnregisterDownloadStatusCallback_001, TestSize.Level0)
{
    int32_t ret = AudioSuiteDownloadManager::GetInstance().UnregisterDownloadStatusCallback();
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuiteDownloadManagerUnitTest, OnDownloadStatusChanged_001, TestSize.Level0)
{
    std::vector<AudioSuiteDownloadStatusInfo> statusVec;
    AudioSuiteDownloadStatusInfo status;
    status.progress = 10;
    statusVec.push_back(status);

    EXPECT_CALL(*mockService_, OnDownloadStatusChanged(_))
        .WillOnce(Return(SUCCESS));

    AudioSuiteDownloadManager::GetInstance().OnDownloadStatusChanged(statusVec);
}

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS