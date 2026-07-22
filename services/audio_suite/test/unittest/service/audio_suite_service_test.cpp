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

using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

static constexpr int32_t AUDIO_SUITE_SA_ID = 3015;

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
    ErrCode ret = service->StartDownload("", result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, ERR_INVALID_PARAM);

    ret = service->StartDownload("v1", result);
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

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS
