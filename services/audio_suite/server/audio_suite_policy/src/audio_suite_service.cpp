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

#ifndef LOG_TAG
#define LOG_TAG "audio_suite_service"
#endif

#include "audio_suite_service.h"
#include "audio_suite_log.h"
#include "audio_utils.h"
#include "download_manager.h"
#ifndef AUDIOSUITE_SA_UT
#include "iservice_registry.h"
#include "system_ability_definition.h"
#endif
#include "hisysevent.h"
#include <fcntl.h>
#include <unistd.h>
#include <ctime>

// AccessToken 相关头文件
#include "ipc_skeleton.h"
#include "accesstoken_kit.h"
#include "access_token.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

#ifndef AUDIOSUITE_SA_UT
static constexpr const char* TASK_ID = "AudioSuiteSaUnloadTask";
static constexpr int32_t AUDIO_SUITE_SA_ID = 3015;
static constexpr int32_t DELAY_TIME = 300000; // 5 minutes in milliseconds
#endif

#ifndef AUDIOSUITE_SA_UT
REGISTER_SYSTEM_ABILITY_BY_ID(AudioSuiteService, AUDIO_SUITE_SA_ID, true);
#endif

AudioSuiteService::AudioSuiteService(int32_t systemAbilityId, bool runOnCreate)
    : SystemAbility(systemAbilityId, runOnCreate), downloadManager_(OHOS::Singleton<DownloadManager>::GetInstance())
{
    AUDIO_INFO_LOG("AudioSuiteService constructor start, saId: %{public}d", systemAbilityId);
}

AudioSuiteService::~AudioSuiteService()
{
    std::shared_ptr<AppExecFwk::EventRunner> runnerCopy;
    {
        std::lock_guard<std::mutex> lock(eventMutex_);
        if (unloadHandler_ != nullptr) {
            unloadHandler_->RemoveAllEvents();
            unloadHandler_.reset();
        }
        runnerCopy = unloadRunner_;
        unloadRunner_.reset();
    }
    if (runnerCopy != nullptr) {
        runnerCopy->Stop();
    }
}

void AudioSuiteService::OnStart()
{
    {
        std::lock_guard<std::mutex> lock(eventMutex_);
        unloadRunner_ = AppExecFwk::EventRunner::Create("AudioSuiteSaUnloadRunner");
        CHECK_AND_RETURN_LOG(unloadRunner_ != nullptr, "Failed to create unload runner");
        unloadHandler_ = std::make_shared<AppExecFwk::EventHandler>(unloadRunner_);
        CHECK_AND_RETURN_LOG(unloadHandler_ != nullptr, "Failed to create unload handler");
    }

    std::lock_guard<std::mutex> lock(mutex_);
    bool res = Publish(this);
    if (!res) {
        AUDIO_ERR_LOG("AudioSuiteService: Publish failed");
        return;
    }
    PostDelayUnloadTask();
    AUDIO_INFO_LOG("AudioSuiteService: Publish success");
}

void AudioSuiteService::OnStop()
{
    AUDIO_INFO_LOG("AudioSuiteService::OnStop");
}

ErrCode AudioSuiteService::RegisterDownloadStatusCallback(const sptr<IRemoteObject>& object, ErrCode& result)
{
    AUDIO_INFO_LOG("AudioSuiteService::RegisterDownloadStatusCallback");
    PostDelayUnloadTask();
    CHECK_AND_RETURN_RET_LOG(CheckPermission(result), SUCCESS, "RegisterDownloadStatusCallback: not permission");

    int32_t callerPid = IPCSkeleton::GetCallingPid();
    result = downloadManager_.RegisterDownloadStatusCallback(callerPid, object);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, SUCCESS,
        "RegisterDownloadStatusCallback failed, callerPid:%{public}d result:%{public}d", callerPid, result);
    return SUCCESS;
}

ErrCode AudioSuiteService::UnregisterDownloadStatusCallback(ErrCode &result)
{
    AUDIO_INFO_LOG("AudioSuiteService::UnregisterDownloadStatusCallback");
    PostDelayUnloadTask();
    CHECK_AND_RETURN_RET_LOG(CheckPermission(result), SUCCESS, "UnregisterDownloadStatusCallback: not permission");

    int32_t callerPid = IPCSkeleton::GetCallingPid();
    result = downloadManager_.UnregisterDownloadStatusCallback(callerPid);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, SUCCESS,
        "UnregisterDownloadStatusCallback failed, result:%{public}d", result);
    return SUCCESS;
}

ErrCode AudioSuiteService::RegisterDownloadInterfaceCallback(const sptr<IRemoteObject> &object, ErrCode& result)
{
    AUDIO_INFO_LOG("AudioSuiteService::RegisterDownloadInterfaceCallback");
    PostDelayUnloadTask();
    CHECK_AND_RETURN_RET_LOG(CheckPermission(result), SUCCESS, "RegisterDownloadInterfaceCallback: not permission");

    result = downloadManager_.SetUpdateEngineCallback(object);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, SUCCESS,
        "RegisterDownloadInterfaceCallback failed, result:%{public}d", result);
    return SUCCESS;
}

ErrCode AudioSuiteService::UnregisterDownloadInterfaceCallback(ErrCode& result)
{
    AUDIO_INFO_LOG("AudioSuiteService::UnregisterDownloadInterfaceCallback");
    PostDelayUnloadTask();
    CHECK_AND_RETURN_RET_LOG(CheckPermission(result), SUCCESS, "UnregisterDownloadInterfaceCallback: not permission");

    result = downloadManager_.SetUpdateEngineCallback(nullptr);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, SUCCESS,
        "UnregisterDownloadInterfaceCallback failed, result:%{public}d", result);
    return SUCCESS;
}

ErrCode AudioSuiteService::StartDownload(const std::string& featureName, ErrCode& result)
{
    AUDIO_INFO_LOG("AudioSuiteService::StartDownload, featureName=%{public}s", featureName.c_str());
    PostDelayUnloadTask();
    CHECK_AND_RETURN_RET_LOG(CheckPermission(result), SUCCESS, "RegisterDownloadStatusCallback: not permission");

    result = downloadManager_.StartDownload(featureName);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, SUCCESS, "StartDownload failed, result:%{public}d", result);
    return SUCCESS;
}

ErrCode AudioSuiteService::PauseDownload(const std::string& featureName, ErrCode& result)
{
    AUDIO_INFO_LOG("AudioSuiteService::PauseDownload, featureName=%{public}s", featureName.c_str());
    PostDelayUnloadTask();
    CHECK_AND_RETURN_RET_LOG(CheckPermission(result), SUCCESS, "RegisterDownloadStatusCallback: not permission");

    result = downloadManager_.PauseDownload(featureName);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, SUCCESS, "PauseDownload failed, result:%{public}d", result);
    return SUCCESS;
}

ErrCode AudioSuiteService::CancelDownload(const std::string& featureName, ErrCode& result)
{
    AUDIO_INFO_LOG("AudioSuiteService::CancelDownload, featureName=%{public}s", featureName.c_str());
    PostDelayUnloadTask();
    CHECK_AND_RETURN_RET_LOG(CheckPermission(result), SUCCESS, "RegisterDownloadStatusCallback: not permission");

    result = downloadManager_.CancelDownload(featureName);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, SUCCESS, "CancelDownload failed, result:%{public}d", result);
    return SUCCESS;
}

ErrCode AudioSuiteService::StartBackgroundDownload(const std::string& featureName, ErrCode& result)
{
    AUDIO_INFO_LOG("AudioSuiteService::StartBackgroundDownload, featureName=%{public}s", featureName.c_str());
    PostDelayUnloadTask();
    CHECK_AND_RETURN_RET_LOG(CheckPermission(result), SUCCESS, "RegisterDownloadStatusCallback: not permission");

    result = downloadManager_.StartBackgroundDownload(featureName);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, SUCCESS, "StartBackgroundDownload failed, result:%{public}d", result);
    return SUCCESS;
}

ErrCode AudioSuiteService::UninstallCloudRom(const std::string &featureName, ErrCode &result)
{
    AUDIO_INFO_LOG("AudioSuiteService::UninstallCloudRom");
    PostDelayUnloadTask();
    CHECK_AND_RETURN_RET_LOG(CheckPermission(result), SUCCESS, "RegisterDownloadStatusCallback: not permission");
 
    result = downloadManager_.UninstallCloudRom(featureName);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, SUCCESS, "UninstallCloudRom failed, result:%{public}d", result);
    return SUCCESS;
}

ErrCode AudioSuiteService::GetDownloadStatus(
    const std::string &featureName, AudioSuiteDownloadStatusInfo &status, ErrCode &result)
{
    AUDIO_INFO_LOG("AudioSuiteService::GetDownloadStatus");
    PostDelayUnloadTask();
    CHECK_AND_RETURN_RET_LOG(CheckPermission(result), SUCCESS, "RegisterDownloadStatusCallback: not permission");

    result = downloadManager_.GetDownloadStatus(featureName, status);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, SUCCESS, "GetDownloadStatus failed, result:%{public}d", result);
    return SUCCESS;
}


ErrCode AudioSuiteService::OnDownloadStatusChanged(const std::vector<AudioSuiteDownloadStatusInfo>& status)
{
    AUDIO_INFO_LOG("AudioSuiteService::OnDownloadStatusChanged");
    PostDelayUnloadTask();

    int32_t result = downloadManager_.OnDownloadStatusChanged(status);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, SUCCESS, "OnDownloadStatusChanged failed, result:%{public}d", result);
    return SUCCESS;
}

bool AudioSuiteService::CheckPermission(ErrCode &result)
{
#ifndef AUDIOSUITE_SA_UT
    Security::AccessToken::AccessTokenID token = OHOS::IPCSkeleton::GetCallingTokenID();
    Security::AccessToken::ATokenTypeEnum type = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(token);
    if (type != Security::AccessToken::TOKEN_NATIVE && type != Security::AccessToken::TOKEN_HAP &&
        type != Security::AccessToken::TOKEN_SHELL) {
        AUDIO_ERR_LOG("CheckPermission: Invalid token type=%{public}d", type);
        result = ERR_PERMISSION_DENIED;
        return false;
    }
#endif
    return true;
}

bool AudioSuiteService::ShouldExtendUnloadTimer()
{
    return downloadManager_.HasDownloadingTask();
}

void AudioSuiteService::PostDelayUnloadTask()
{
#ifndef AUDIOSUITE_SA_UT
    std::lock_guard<std::mutex> lock(eventMutex_);
    CHECK_AND_RETURN_LOG(unloadHandler_ != nullptr, "PostDelayUnloadTask: unloadHandler_ is null");

    std::function<void()> task = [this, &task]() {
        if (ShouldExtendUnloadTimer()) {
            std::lock_guard<std::mutex> lock(eventMutex_);
            CHECK_AND_RETURN_LOG(unloadHandler_ != nullptr, "unloadHandler_ is null");
            unloadHandler_->RemoveTask(TASK_ID);
            unloadHandler_->PostTask(task, TASK_ID, DELAY_TIME);
            return;
        }

        auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        CHECK_AND_RETURN_LOG(samgrProxy != nullptr, "get samgr failed");
        AUDIO_INFO_LOG("start UnloadSystemAbility");
        int32_t ret = samgrProxy->UnloadSystemAbility(AUDIO_SUITE_SA_ID);
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "unload system ability failed, ret=%{public}d", ret);
    };
    unloadHandler_->RemoveTask(TASK_ID);
    unloadHandler_->PostTask(task, TASK_ID, DELAY_TIME);
#endif
}

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS