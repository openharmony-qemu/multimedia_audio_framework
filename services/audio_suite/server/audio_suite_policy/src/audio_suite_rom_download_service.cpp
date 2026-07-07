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

namespace {
#ifndef AUDIOSUITE_SA_UT
    __attribute__((constructor)) void LibraryLoaded() {
    }
#endif
}

#ifndef AUDIOSUITE_SA_UT
REGISTER_SYSTEM_ABILITY_BY_ID(AudioSuiteRomDownloadService, AudioSuiteRomDownloadService::SA_ID, true);
#endif

AudioSuiteRomDownloadService::AudioSuiteRomDownloadService(int32_t systemAbilityId, bool runOnCreate)
    : SystemAbility(systemAbilityId, runOnCreate)
{
    AUDIO_INFO_LOG("AudioSuiteRomDownloadService constructor START, SA_ID: %{public}d", systemAbilityId);
}

void AudioSuiteRomDownloadService::OnStart()
{
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

int32_t AudioSuiteRomDownloadService::Dump(int32_t fd, const std::vector<std::u16string>& args)
{
    AUDIO_INFO_LOG("AudioSuiteRomDownloadService::Dump");
    dprintf(fd, "AudioSuiteRomDownloadService:");
    dprintf(fd, "SA ID: %d", AudioSuiteRomDownloadService::SA_ID);
    return 0;
}

ErrCode AudioSuiteRomDownloadService::RegisterDownloadCallback(const sptr<IRemoteObject>& callback, ErrCode& result)
{
    AUDIO_INFO_LOG("AudioSuiteRomDownloadService::RegisterDownloadCallback");

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

ErrCode AudioSuiteRomDownloadService::CancelDownload(ErrCode& result)
{
    AUDIO_INFO_LOG("AudioSuiteRomDownloadService::CancelDownload");

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
 
    result = downloadManager_->CancelDownload();
    if (result != SUCCESS) {
        AUDIO_ERR_LOG("CancelDownload failed, error=%{public}d", result);
    }
    return SUCCESS;
}

ErrCode AudioSuiteRomDownloadService::StartBackgroundDownload(const std::string& version, ErrCode& result)
{
    AUDIO_INFO_LOG("AudioSuiteRomDownloadService::StartBackgroundDownload, version=%{public}s", version.c_str());

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

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS