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
#define LOG_TAG "audio_suite_rom_download_service"
#endif

#include "audio_suite_rom_download_service.h"
#include "audio_suite_log.h"
#include "audio_utils.h"
#include "download_manager.h"
#ifndef AUDIOSUITE_SA_UT
#include "iservice_registry.h"
#include "system_ability_definition.h"
#endif
#include "download_callback_proxy.h"
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

static constexpr const char* TASK_ID = "AudioSuiteSaUnloadTask";
static constexpr int32_t DELAY_TIME = 300000; // 5 minutes in milliseconds

#ifndef AUDIOSUITE_SA_UT
REGISTER_SYSTEM_ABILITY_BY_ID(AudioSuiteRomDownloadService, AudioSuiteRomDownloadService::SA_ID, true);
#endif

AudioSuiteRomDownloadService::AudioSuiteRomDownloadService(int32_t systemAbilityId, bool runOnCreate)
    : SystemAbility(systemAbilityId, runOnCreate)
{
    AUDIO_INFO_LOG("AudioSuiteRomDownloadService constructor START, SA_ID: %{public}d", systemAbilityId);
}

AudioSuiteRomDownloadService::~AudioSuiteRomDownloadService()
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

void AudioSuiteRomDownloadService::OnStart()
{
    {
        std::lock_guard<std::mutex> lock(eventMutex_);
        unloadRunner_ = AppExecFwk::EventRunner::Create("AudioSuiteSaUnloadRunner");
        CHECK_AND_RETURN_LOG(unloadRunner_ != nullptr, "Failed to create unload runner");
        unloadHandler_ = std::make_shared<AppExecFwk::EventHandler>(unloadRunner_);
        CHECK_AND_RETURN_LOG(unloadHandler_ != nullptr, "Failed to create unload handler");
    }

    downloadManager_ = std::make_shared<DownloadManager>();
    if (downloadManager_) {
        downloadManager_->Init();
        int32_t ret = downloadManager_->RegisterDownloadCallback(
            [this](const DownloadStatus &status) { NotifyDownloadStatusChanged(status); });
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("RegisterDownloadCallback failed, ret=%{public}d", ret);
        }
    }

    bool res = Publish(this);
    if (!res) {
        AUDIO_ERR_LOG("AudioSuiteRomDownloadService: Publish failed");
        return;
    }
    PostDelayUnloadTask();
    AUDIO_INFO_LOG("AudioSuiteRomDownloadService: Publish success");
}

void AudioSuiteRomDownloadService::OnStop()
{
    AUDIO_INFO_LOG("AudioSuiteRomDownloadService::OnStop");

    std::lock_guard<std::mutex> lock(mutex_);

    if (downloadManager_) {
        downloadManager_.reset();
    }

    downloadCallback_ = nullptr;
    updateEngineCallback_ = nullptr;
    AUDIO_INFO_LOG("AudioSuiteRomDownloadService: OnStop completed");
}

ErrCode AudioSuiteRomDownloadService::RegisterDownloadCallback(const sptr<IRemoteObject>& callback, ErrCode& result)
{
    AUDIO_INFO_LOG("AudioSuiteRomDownloadService::RegisterDownloadCallback");
    PostDelayUnloadTask();

    if (!CheckPermission()) {
        AUDIO_ERR_LOG("RegisterDownloadCallback: Permission check failed");
        result = ERR_PERMISSION_DENIED;
        return SUCCESS;
    }

    downloadCallback_ = callback;
    if (!downloadManager_->RegisterCallback()) {
        AUDIO_ERR_LOG("Failed to register cloud ROM download callback");
        result = ERR_OPERATION_FAILED;
        return SUCCESS;
    }
    result = SUCCESS;
    return SUCCESS;
}

ErrCode AudioSuiteRomDownloadService::UnregisterDownloadCallback(ErrCode &result)
{
    AUDIO_INFO_LOG("AudioSuiteRomDownloadService::UnregisterDownloadCallback");

    if (!CheckPermission()) {
        AUDIO_ERR_LOG("UnregisterDownloadCallback: Permission check failed");
        result = ERR_PERMISSION_DENIED;
        return SUCCESS;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    downloadCallback_ = nullptr;
    result = SUCCESS;
    return SUCCESS;
}

ErrCode AudioSuiteRomDownloadService::RegisterUpdateEngineCallback(const sptr<IRemoteObject>& callback, ErrCode& result)
{
    AUDIO_INFO_LOG("AudioSuiteRomDownloadService::RegisterUpdateEngineCallback");
    PostDelayUnloadTask();

    if (!CheckPermission()) {
        AUDIO_ERR_LOG("RegisterUpdateEngineCallback: Permission check failed");
        result = ERR_PERMISSION_DENIED;
        return SUCCESS;
    }

    // 设置 DownloadManager 的 UpdateEngine 回调
    if (downloadManager_) {
        downloadManager_->SetUpdateEngineCallback(callback);
    }

    result = SUCCESS;
    return SUCCESS;
}

ErrCode AudioSuiteRomDownloadService::UnregisterUpdateEngineCallback(ErrCode& result)
{
    AUDIO_INFO_LOG("AudioSuiteRomDownloadService::UnregisterUpdateEngineCallback");

    if (!CheckPermission()) {
        AUDIO_ERR_LOG("UnregisterUpdateEngineCallback: Permission check failed");
        result = ERR_PERMISSION_DENIED;
        return SUCCESS;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    updateEngineCallback_ = nullptr;

    // 清除 DownloadManager 的 UpdateEngine 回调
    if (downloadManager_) {
        downloadManager_->SetUpdateEngineCallback(nullptr);
    }
    result = SUCCESS;
    return SUCCESS;
}

ErrCode AudioSuiteRomDownloadService::StartDownload(const std::string& version, ErrCode& result)
{
    AUDIO_INFO_LOG("AudioSuiteRomDownloadService::StartDownload, version=%{public}s", version.c_str());
    PostDelayUnloadTask();

    if (!CheckPermission()) {
        AUDIO_ERR_LOG("StartDownload: Permission check failed");
        result = ERR_PERMISSION_DENIED;
        return SUCCESS;
    }

    if (!downloadManager_) {
        AUDIO_ERR_LOG("StartDownload: DownloadManager not initialized");
        result = ERR_NOT_STARTED;
        return SUCCESS;
    }
 
    result = downloadManager_->StartDownload(version);
    if (result != SUCCESS) {
        AUDIO_ERR_LOG("StartDownload failed, error=%{public}d", result);
    }
    return SUCCESS;
}

ErrCode AudioSuiteRomDownloadService::CancelDownload(const std::string& version, ErrCode& result)
{
    AUDIO_INFO_LOG("AudioSuiteRomDownloadService::CancelDownload, version=%{public}s", version.c_str());
    PostDelayUnloadTask();

    if (!CheckPermission()) {
        AUDIO_ERR_LOG("CancelDownload: Permission check failed");
        result = ERR_PERMISSION_DENIED;
        return SUCCESS;
    }

    if (!downloadManager_) {
        AUDIO_ERR_LOG("CancelDownload: DownloadManager not initialized");
        result = ERR_NOT_STARTED;
        return SUCCESS;
    }
 
    result = downloadManager_->CancelDownload(version);
    if (result != SUCCESS) {
        AUDIO_ERR_LOG("CancelDownload failed, error=%{public}d", result);
    }
    return SUCCESS;
}

ErrCode AudioSuiteRomDownloadService::StartBackgroundDownload(const std::string& version, ErrCode& result)
{
    AUDIO_INFO_LOG("AudioSuiteRomDownloadService::StartBackgroundDownload, version=%{public}s", version.c_str());
    PostDelayUnloadTask();

    if (!CheckPermission()) {
        AUDIO_ERR_LOG("StartBackgroundDownload: Permission check failed");
        result = ERR_PERMISSION_DENIED;
        return SUCCESS;
    }

    if (!downloadManager_) {
        AUDIO_ERR_LOG("StartBackgroundDownload: DownloadManager not initialized");
        result = ERR_NOT_STARTED;
        return SUCCESS;
    }
 
    result = downloadManager_->StartBackgroundDownload(version);
    if (result != SUCCESS) {
        AUDIO_ERR_LOG("StartBackgroundDownload failed, error=%{public}d", result);
    }
    return SUCCESS;
}

ErrCode AudioSuiteRomDownloadService::UninstallCloudRom(const std::string &version, ErrCode &result)
{
    AUDIO_INFO_LOG("AudioSuiteRomDownloadService::UninstallCloudRom");
    PostDelayUnloadTask();

    if (!CheckPermission()) {
        AUDIO_ERR_LOG("UninstallCloudRom: Permission check failed");
        result = ERR_PERMISSION_DENIED;
        return SUCCESS;
    }

    if (!downloadManager_) {
        AUDIO_ERR_LOG("UninstallCloudRom: DownloadManager not initialized");
        result = ERR_NOT_STARTED;
        return SUCCESS;
    }
 
    result = downloadManager_->UninstallCloudRom(version);
    if (result != SUCCESS) {
        AUDIO_ERR_LOG("UninstallCloudRom failed, error=%{public}d", result);
    }
    return SUCCESS;
}

ErrCode AudioSuiteRomDownloadService::GetDownloadStatus(
    DownloadStatus &status, const std::string &version, ErrCode &result)
{
    AUDIO_INFO_LOG("AudioSuiteRomDownloadService::GetDownloadStatus");
    PostDelayUnloadTask();

    if (!CheckPermission()) {
        AUDIO_ERR_LOG("GetDownloadStatus: Permission check failed");
        result = ERR_PERMISSION_DENIED;
        return SUCCESS;
    }

    result = downloadManager_->GetDownloadStatus(status, version);
    if (result != SUCCESS) {
        AUDIO_ERR_LOG("GetDownloadStatus failed, error=%{public}d", result);
    }

    return SUCCESS;
}

bool AudioSuiteRomDownloadService::CheckPermission()
{
    Security::AccessToken::AccessTokenID token = OHOS::IPCSkeleton::GetCallingTokenID();
    Security::AccessToken::ATokenTypeEnum type = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(token);
    if (type != Security::AccessToken::TOKEN_NATIVE && type != Security::AccessToken::TOKEN_HAP &&
        type != Security::AccessToken::TOKEN_SHELL) {
        AUDIO_ERR_LOG("CheckPermission: Invalid token type=%{public}d", type);
        return false;
    }
    return true;
}

void AudioSuiteRomDownloadService::NotifyDownloadStatusChanged(const DownloadStatus& status)
{
    sptr<IRemoteObject> callbackCopy;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        callbackCopy = downloadCallback_;
    }

    if (callbackCopy == nullptr) {
        AUDIO_ERR_LOG("NotifyDownloadStatusChanged: callback is null");
        return;
    }

    sptr<DownloadCallbackProxy> callbackProxy = iface_cast<DownloadCallbackProxy>(callbackCopy);
    if (callbackProxy == nullptr) {
        AUDIO_ERR_LOG("NotifyDownloadStatusChanged: failed to cast callback proxy");
        return;
    }

    callbackProxy->OnDownloadStatusChanged(status);
    AUDIO_DEBUG_LOG("NotifyDownloadStatusChanged: state=%{public}d, progress=%{public}u",
        static_cast<int32_t>(status.state), status.progress);
}

bool AudioSuiteRomDownloadService::ShouldExtendUnloadTimer()
{
    bool hasDownloadingTask = false;
    bool hasRegisteredCallback = HasRegisteredCallback();
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (downloadManager_ != nullptr) {
            DownloadStatus status;
            ErrCode ret = downloadManager_->GetDownloadStatus(status, "");
            hasDownloadingTask = (ret == SUCCESS && status.state == DownloadState::DOWNLOAD_IN_PROGRESS);
        }
    }
    AUDIO_INFO_LOG("ShouldExtendUnloadTimer: hasDownloadingTask=%{public}d, hasRegisteredCallback=%{public}d",
        hasDownloadingTask, hasRegisteredCallback);
    return hasDownloadingTask || hasRegisteredCallback;
}

void AudioSuiteRomDownloadService::PostDelayUnloadTask()
{
    std::lock_guard<std::mutex> lock(eventMutex_);
    CHECK_AND_RETURN_LOG(unloadHandler_ != nullptr, "PostDelayUnloadTask: unloadHandler_ is null");

    auto task = [this]() {
        if (ShouldExtendUnloadTimer()) {
            std::lock_guard<std::mutex> lock(eventMutex_);
            CHECK_AND_RETURN_LOG(unloadHandler_ != nullptr, "unloadHandler_ is null");
            auto extendedTask = []() {
#ifndef AUDIOSUITE_SA_UT
                auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
                CHECK_AND_RETURN_LOG(samgrProxy != nullptr, "get samgr failed");
                int32_t ret = samgrProxy->UnloadSystemAbility(AudioSuiteRomDownloadService::SA_ID);
                CHECK_AND_RETURN_LOG(ret == SUCCESS, "unload system ability failed, ret=%{public}d", ret);
#endif
            };
            unloadHandler_->RemoveTask(TASK_ID);
            unloadHandler_->PostTask(extendedTask, TASK_ID, DELAY_TIME);
            return;
        }

#ifndef AUDIOSUITE_SA_UT
        auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        CHECK_AND_RETURN_LOG(samgrProxy != nullptr, "get samgr failed");
        int32_t ret = samgrProxy->UnloadSystemAbility(AudioSuiteRomDownloadService::SA_ID);
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "unload system ability failed, ret=%{public}d", ret);
#endif
    };
    unloadHandler_->RemoveTask(TASK_ID);
    unloadHandler_->PostTask(task, TASK_ID, DELAY_TIME);
}

bool AudioSuiteRomDownloadService::HasRegisteredCallback()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return downloadCallback_ != nullptr;
}

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS