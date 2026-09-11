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
#define LOG_TAG "download_manager"
#endif

#include "download_manager.h"
#include "audio_errors.h"
#include "parameter.h"
#include "audio_suite_log.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

#ifndef AUDIOSUITE_SA_UT
static constexpr int32_t DOWNLOAD_REGISTER_TIMEOUT_MS = 1000;  // 回调注册超时时间 1 秒
#endif

static constexpr int32_t FEATURE_INSTALL_SUCCESS = 36;

DownloadManager::DownloadManager()
{
    AUDIO_INFO_LOG("DownloadManager constructor");
}

DownloadManager::~DownloadManager()
{
    std::unique_lock<std::mutex> lock(mutex_);
    AUDIO_INFO_LOG("DownloadManager destructor");
    if (backgroundDownload_ != nullptr) {
        backgroundDownload_ = nullptr;
    }

    {
        std::unique_lock<std::mutex> statusCallbacksLock(statusCallbacksMutex_);
        statusCallbacks_.clear();
    }
    std::unique_lock<std::mutex> downloadLock(downloadInterfaceMutex_);
    downloadCallback_ = nullptr;
    isDownloadCallbackRegistered_ = false;
}

int32_t DownloadManager::StartDownload(const std::string& featureName, bool allowedCellularNet)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_INFO_LOG("StartDownload: featureName=%{public}s", featureName.c_str());
    
    CHECK_AND_RETURN_RET_LOG(!featureName.empty(), ERR_INVALID_PARAM, "Download featureName is empty");
    CHECK_AND_RETURN_RET_LOG(WaitForCallbackRegistration(), ERR_CALLBACK_NOT_REGISTERED, "donwload callback not reg");

    std::unique_lock<std::mutex> downloadLock(downloadInterfaceMutex_);
    CHECK_AND_RETURN_RET_LOG(downloadCallback_ != nullptr, ERR_CALLBACK_NOT_REGISTERED, "donwload callback is null");

    int32_t funcResult = SUCCESS;
    AudioSuiteCloudRomVersionInfo newVersionInfo;
    int32_t ret = downloadCallback_->GetFeatureNewVersion(featureName, newVersionInfo, funcResult);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetFeatureNewVersion failed, ret=%{public}d", ret);
    CHECK_AND_RETURN_RET_LOG(funcResult == SUCCESS, funcResult,
        "GetFeatureNewVersion failed, funcResult=%{public}d", funcResult);
    CHECK_AND_RETURN_RET_LOG(newVersionInfo.errorCode == 0, newVersionInfo.errorCode,
        "GetFeatureNewVersion failed, errorCode=%{public}d", newVersionInfo.errorCode);
    CHECK_AND_RETURN_RET_LOG(newVersionInfo.featureName == featureName, ERR_INVALID_PARAM,
        "GetFeatureNewVersion: failed, newVersionInfo.featureName:%{public}s", newVersionInfo.featureName.c_str());

    AUDIO_INFO_LOG("DownloadAndInstall: featureName=%{public}s", featureName.c_str());
    ret = downloadCallback_->DownloadAndInstall(!allowedCellularNet, featureName, funcResult);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "DownloadAndInstall failed, ret=%{public}d", ret);
    CHECK_AND_RETURN_RET_LOG(funcResult == SUCCESS, funcResult,
        "DownloadAndInstall failed, funcResult=%{public}d", funcResult);
    AUDIO_INFO_LOG("StartDownload success");
    return SUCCESS;
}

int32_t DownloadManager::PauseDownload(const std::string& featureName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_INFO_LOG("PauseDownload: featureName=%{public}s", featureName.c_str());
    
    CHECK_AND_RETURN_RET_LOG(!featureName.empty(), ERR_INVALID_PARAM, "Download featureName is empty");
    CHECK_AND_RETURN_RET_LOG(WaitForCallbackRegistration(), ERR_CALLBACK_NOT_REGISTERED, "donwload callback not reg");

    std::unique_lock<std::mutex> downloadLock(downloadInterfaceMutex_);
    CHECK_AND_RETURN_RET_LOG(downloadCallback_ != nullptr, ERR_CALLBACK_NOT_REGISTERED, "donwload callback is null");

    int32_t funcResult = SUCCESS;
    int32_t ret = downloadCallback_->PauseCloudRom(featureName, funcResult);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "PauseDownload failed, ret=%{public}d", ret);
    CHECK_AND_RETURN_RET_LOG(funcResult == SUCCESS, funcResult,
        "PauseDownload failed, funcResult=%{public}d", funcResult);
    AUDIO_INFO_LOG("PauseDownload success");
    return SUCCESS;
}

int32_t DownloadManager::CancelDownload(const std::string& featureName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_INFO_LOG("CancelDownload: featureName=%{public}s", featureName.c_str());
    
    CHECK_AND_RETURN_RET_LOG(!featureName.empty(), ERR_INVALID_PARAM, "Download featureName is empty");
    CHECK_AND_RETURN_RET_LOG(WaitForCallbackRegistration(), ERR_CALLBACK_NOT_REGISTERED, "donwload callback not reg");

    std::unique_lock<std::mutex> downloadLock(downloadInterfaceMutex_);
    CHECK_AND_RETURN_RET_LOG(downloadCallback_ != nullptr, ERR_CALLBACK_NOT_REGISTERED, "donwload callback is null");
    if (backgroundDownload_ != nullptr) {
        backgroundDownload_->RemoveFeature(featureName);
    }

    int32_t funcResult = SUCCESS;
    int32_t ret = downloadCallback_->CancelCloudRom(featureName, funcResult);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "PauseDownload failed, ret=%{public}d", ret);
    CHECK_AND_RETURN_RET_LOG(funcResult == SUCCESS, funcResult,
        "PauseDownload failed, funcResult=%{public}d", funcResult);
    AUDIO_INFO_LOG("CancelDownload success");
    return SUCCESS;
}

int32_t DownloadManager::StartBackgroundDownload(const std::string& featureName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_INFO_LOG("StartBackgroundDownload: featureName=%{public}s", featureName.c_str());
    CHECK_AND_RETURN_RET_LOG(!featureName.empty(), ERR_INVALID_PARAM, "Download featureName is empty");
    CHECK_AND_RETURN_RET_LOG(WaitForCallbackRegistration(), ERR_CALLBACK_NOT_REGISTERED, "donwload callback not reg");

    if (!BackgroundDownload::AreConditionsMet(featureName)) {
        if (backgroundDownload_ == nullptr) {
            AUDIO_INFO_LOG("BackgroundDownload start init backgroundDownload.");
            auto boundFunc = std::bind(&DownloadManager::StartDownload, this, std::placeholders::_1, false);
            backgroundDownload_ = std::make_shared<BackgroundDownload>(boundFunc);
            if (backgroundDownload_->Init() != 0) {
                backgroundDownload_ = nullptr;
                AUDIO_ERR_LOG("backgroundDownload_ init failed");
                return ERROR;
            }
        }
        CHECK_AND_RETURN_RET_LOG(backgroundDownload_->AddFeature(featureName) == SUCCESS, ERROR,
            "Download conditions not met, adding to queue task failed");
        AUDIO_INFO_LOG("BackgroundDownload conditions not met, adding to queue task");
        return SUCCESS;
    }
    if (backgroundDownload_ != nullptr) {
        backgroundDownload_->RemoveFeature(featureName);
    }
    AUDIO_INFO_LOG("Download conditions met, start download");

    int32_t funcResult = SUCCESS;
    AudioSuiteCloudRomVersionInfo newVersionInfo;
    std::unique_lock<std::mutex> downloadLock(downloadInterfaceMutex_);
    CHECK_AND_RETURN_RET_LOG(downloadCallback_ != nullptr, ERR_CALLBACK_NOT_REGISTERED, "donwload callback is null");
    int32_t ret = downloadCallback_->GetFeatureNewVersion(featureName, newVersionInfo, funcResult);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetFeatureNewVersion failed, ret=%{public}d", ret);
    CHECK_AND_RETURN_RET_LOG(funcResult == SUCCESS, funcResult,
        "GetFeatureNewVersion failed, funcResult=%{public}d", funcResult);
    CHECK_AND_RETURN_RET_LOG(newVersionInfo.errorCode == 0, newVersionInfo.errorCode,
        "GetFeatureNewVersion failed, errorCode=%{public}d", newVersionInfo.errorCode);
    CHECK_AND_RETURN_RET_LOG(newVersionInfo.featureName == featureName, ERR_INVALID_PARAM,
        "GetFeatureNewVersion: failed, newVersionInfo.featureName:%{public}s", newVersionInfo.featureName.c_str());

    ret = downloadCallback_->DownloadAndInstall(false, featureName, funcResult);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "DownloadAndInstall failed, ret=%{public}d", ret);
    CHECK_AND_RETURN_RET_LOG(funcResult == SUCCESS, funcResult,
        "DownloadAndInstall failed, funcResult=%{public}d", funcResult);
    AUDIO_INFO_LOG("StartBackgroundDownload success");
    return SUCCESS;
}

int32_t DownloadManager::GetDownloadStatus(const std::string& featureName, AudioSuiteDownloadStatusInfo& status)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_INFO_LOG("GetDownloadStatus: featureName=%{public}s", featureName.c_str());
    
    CHECK_AND_RETURN_RET_LOG(!featureName.empty(), ERR_INVALID_PARAM, "Download featureName is empty");
    CHECK_AND_RETURN_RET_LOG(WaitForCallbackRegistration(), ERR_CALLBACK_NOT_REGISTERED, "donwload callback not reg");

    std::unique_lock<std::mutex> downloadLock(downloadInterfaceMutex_);
    CHECK_AND_RETURN_RET_LOG(downloadCallback_ != nullptr, ERR_CALLBACK_NOT_REGISTERED, "donwload callback is null");

    int32_t funcResult = SUCCESS;
    int32_t ret = downloadCallback_->GetFeatureStatus(featureName, status, funcResult);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetFeatureStatus failed, ret=%{public}d", ret);
    CHECK_AND_RETURN_RET_LOG(funcResult == SUCCESS, funcResult,
        "GetFeatureStatus failed, funcResult=%{public}d", funcResult);
    CHECK_AND_RETURN_RET_LOG(status.errorCode == 0, status.errorCode,
        "GetFeatureStatus failed, errorCode=%{public}d", status.errorCode);
    CHECK_AND_RETURN_RET_LOG(status.featureName == featureName, ERR_INVALID_PARAM,
        "GetFeatureStatus: failed, status.featureName:%{public}s", status.featureName.c_str());
    status.size = 0;
    
    // Do not search for packages upon successful installation.
    if (status.state == FEATURE_INSTALL_SUCCESS) {
        AUDIO_INFO_LOG("%{public}s Installation successful.", status.featureName.c_str());
        return SUCCESS;
    }

    AudioSuiteCloudRomVersionInfo newVersionInfo;
    ret = downloadCallback_->GetFeatureNewVersion(featureName, newVersionInfo, funcResult);
    if (ret == SUCCESS && funcResult == SUCCESS && newVersionInfo.errorCode == 0 &&
        newVersionInfo.featureName == featureName) {
        status.size = newVersionInfo.size;
    } else {
        AUDIO_ERR_LOG("Failed to search for the %{public}s ret = %{public}d, funcResult = %{public}d, errorCode = "
                      "%{public}d, newVersionInfo.featureName = %{public}s.", featureName.c_str(), ret, funcResult,
                      newVersionInfo.errorCode, newVersionInfo.featureName.c_str());
    }
    AUDIO_INFO_LOG("GetDownloadStatus success, state=%{public}d, progress=%{public}u", status.state, status.progress);
    return SUCCESS;
}

int32_t DownloadManager::UninstallCloudRom(const std::string& featureName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_INFO_LOG("UninstallCloudRom: featureName=%{public}s", featureName.c_str());
    
    CHECK_AND_RETURN_RET_LOG(!featureName.empty(), ERR_INVALID_PARAM, "Download featureName is empty");
    CHECK_AND_RETURN_RET_LOG(WaitForCallbackRegistration(), ERR_CALLBACK_NOT_REGISTERED, "donwload callback not reg");

    std::unique_lock<std::mutex> downloadLock(downloadInterfaceMutex_);
    CHECK_AND_RETURN_RET_LOG(downloadCallback_ != nullptr, ERR_CALLBACK_NOT_REGISTERED, "donwload callback is null");

    int32_t funcResult = SUCCESS;
    int32_t ret = downloadCallback_->UninstallCloudRom(featureName, funcResult);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "UninstallCloudRom failed, ret=%{public}d", ret);
    CHECK_AND_RETURN_RET_LOG(funcResult == SUCCESS, funcResult,
        "UninstallCloudRom failed, funcResult=%{public}d", funcResult);
    AUDIO_INFO_LOG("UninstallCloudRom success");
    return SUCCESS;
}

int32_t DownloadManager::RegisterDownloadStatusCallback(const int32_t callerPid, const sptr<IRemoteObject>& object)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_INFO_LOG("RegisterDownloadStatusCallback start");
    CHECK_AND_RETURN_RET_LOG(WaitForCallbackRegistration(), ERR_CALLBACK_NOT_REGISTERED, "Download callback not reg.");

    sptr<IAudioSuiteStatusCallback> statusCallback = iface_cast<IAudioSuiteStatusCallback>(object);
    CHECK_AND_RETURN_RET_LOG(statusCallback != nullptr, ERR_INVALID_PARAM, "statusCallback is nullptr");

    std::unique_lock<std::mutex> downloadLock(downloadInterfaceMutex_);
    CHECK_AND_RETURN_RET_LOG(downloadCallback_ != nullptr, ERR_CALLBACK_NOT_REGISTERED, "downloadCallback_ is null.");
    if (!registerStatusCallback_) {
        int32_t funcResult = 0;
        int32_t ret = downloadCallback_->RegisterDownloadStatusCallback(funcResult);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "RegisterDownloadStatusCallback failed, ret=%{public}d", ret);
        CHECK_AND_RETURN_RET_LOG(funcResult == SUCCESS, funcResult,
                                 "RegisterDownloadStatusCallback failed, funcResult=%{public}d", funcResult);
        registerStatusCallback_ = true;
    }

    if (object != nullptr) {
        sptr<AudioSuiteClientDeathRecipient> deathRecipient =
            new(std::nothrow) AudioSuiteClientDeathRecipient(callerPid);
        if (deathRecipient != nullptr && object->AddDeathRecipient(deathRecipient)) {
            deathRecipients_[callerPid] = deathRecipient;
            AUDIO_INFO_LOG("RegisterDownloadStatusCallback AddDeathRecipient success");
        }
    }
    {
        std::unique_lock<std::mutex> statusCallbacksLock(statusCallbacksMutex_);
        statusCallbacks_[callerPid] = statusCallback;
        AUDIO_INFO_LOG("RegisterDownloadStatusCallback success, "
            "statusCallbacks Size:%{public}zu", statusCallbacks_.size());
    }
    return SUCCESS;
}

int32_t DownloadManager::UnregisterDownloadStatusCallback(int32_t callerPid)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_INFO_LOG("UnregisterDownloadStatusCallback start");
    CHECK_AND_RETURN_RET_LOG(WaitForCallbackRegistration(), ERR_CALLBACK_NOT_REGISTERED, "Download callback not reg.");

    if (deathRecipients_.find(callerPid) != deathRecipients_.end()) {
        deathRecipients_.erase(callerPid);
    }

    std::unique_lock<std::mutex> downloadLock(downloadInterfaceMutex_);
    CHECK_AND_RETURN_RET_LOG(downloadCallback_ != nullptr, ERR_CALLBACK_NOT_REGISTERED, "downloadCallback_ is null.");

    std::unique_lock<std::mutex> statusCallbacksLock(statusCallbacksMutex_);
    if (statusCallbacks_.find(callerPid) != statusCallbacks_.end()) {
        statusCallbacks_.erase(callerPid);
    }

    if (statusCallbacks_.empty()) {
        int32_t funcResult = 0;
        int32_t ret = downloadCallback_->UnregisterDownloadStatusCallback(funcResult);
        if (ret != 0 || funcResult != 0) {
            AUDIO_ERR_LOG("UnRegisterCloudRomDownloadCallback failed, ret=%{public}d", ret);
            return ERROR;
        }
        registerStatusCallback_ = false;
        AUDIO_INFO_LOG("UnRegisterCloudRomDownloadCallback success, "
            "statusCallbacks Size:%{public}zu", statusCallbacks_.size());
    }

    return SUCCESS;
}

int32_t DownloadManager::SetUpdateEngineCallback(const sptr<IRemoteObject> object)
{
    std::unique_lock<std::mutex> downloadLock(downloadInterfaceMutex_);

    // Remove old death recipient from old remote object before overwriting callback
    if (audioServerdeathRecipient_ != nullptr && downloadCallback_ != nullptr) {
        downloadCallback_->AsObject()->RemoveDeathRecipient(audioServerdeathRecipient_);
    }
    audioServerdeathRecipient_ = nullptr;

    downloadCallback_ = iface_cast<IAudioSuiteDownloadCallback>(object);
    isDownloadCallbackRegistered_ = downloadCallback_ != nullptr;

    if (object != nullptr) {
        sptr<AudioServerDeathRecipient> audioServerdeathRecipient = new(std::nothrow) AudioServerDeathRecipient();
        if (audioServerdeathRecipient != nullptr && object->AddDeathRecipient(audioServerdeathRecipient)) {
            audioServerdeathRecipient_ = audioServerdeathRecipient;
            AUDIO_INFO_LOG("SetUpdateEngineCallback AddDeathRecipient");
        }
    }
    callbackRegisteredCond_.notify_all();
    AUDIO_INFO_LOG("SetUpdateEngineCallback: %{public}s", downloadCallback_ != nullptr ? "set" : "cleared");
    return SUCCESS;
}

int32_t DownloadManager::OnDownloadStatusChanged(const std::vector<AudioSuiteDownloadStatusInfo>& status)
{
    std::unique_lock<std::mutex> statusCallbacksLock(statusCallbacksMutex_);
    for (const auto& pair : statusCallbacks_) {
        if (pair.second) {
            pair.second->OnDownloadStatusChanged(status);
        }
    }
    return SUCCESS;
}

bool DownloadManager::WaitForCallbackRegistration()
{
#ifndef AUDIOSUITE_SA_UT
    std::unique_lock<std::mutex> downloadLock(downloadInterfaceMutex_);
    if (isDownloadCallbackRegistered_) {
        return true;
    }

    AUDIO_INFO_LOG("Waiting for download callback registration.");
    auto status = callbackRegisteredCond_.wait_for(
        downloadLock, std::chrono::milliseconds(DOWNLOAD_REGISTER_TIMEOUT_MS),
        [this] { return isDownloadCallbackRegistered_.load(); });
    if (!status) {
        AUDIO_ERR_LOG("WaitForCallbackRegistration timeout after %{public}d ms", DOWNLOAD_REGISTER_TIMEOUT_MS);
        return false;
    }
#endif
    AUDIO_INFO_LOG("WaitForCallbackRegistration success");
    return true;
}

int32_t DownloadManager::HasDownloadingTask()
{
    std::unique_lock<std::mutex> lock(mutex_);
    bool hasBackgroundDownloadTask = false;
    if (backgroundDownload_ != nullptr) {
        hasBackgroundDownloadTask = backgroundDownload_->HasBackgroundDownloadTask();
    }

    bool hasRegTask = false;
    {
        std::unique_lock<std::mutex> statusCallbacksLock(statusCallbacksMutex_);
        hasRegTask = !statusCallbacks_.empty();
    }
    AUDIO_INFO_LOG("backgroundDownloadTask:%{public}d, hasRegTask:%{public}d", hasBackgroundDownloadTask, hasRegTask);
    return  hasBackgroundDownloadTask || hasRegTask;
}

DownloadManager::AudioSuiteClientDeathRecipient::AudioSuiteClientDeathRecipient(int32_t callerPid)
    : callerPid_(callerPid)
{
}

void DownloadManager::AudioSuiteClientDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    OHOS::Singleton<DownloadManager>::GetInstance().UnregisterDownloadStatusCallback(callerPid_);
    AUDIO_INFO_LOG("AudioSuiteClientDeathRecipient::OnRemoteDied, pid:%{public}d.", callerPid_);
}

void DownloadManager::AudioServerDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    OHOS::Singleton<DownloadManager>::GetInstance().SetUpdateEngineCallback(nullptr);
    AUDIO_INFO_LOG("AudioServerDeathRecipient::OnRemoteDied.");
}


} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS