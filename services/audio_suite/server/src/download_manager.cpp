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
#include <cctype>
#include <unistd.h>
#include <sys/stat.h>
#include <cinttypes>

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

#ifndef AUDIOSUITE_SA_UT
static constexpr int32_t DOWNLOAD_REGISTER_TIMEOUT_MS = 1000;  // 回调注册超时时间 1 秒
#endif

static constexpr int32_t FEATURE_INSTALL_SUCCESS = 36;
static constexpr size_t DOWNLOAD_FEATURE_NAMES_MAX_SIZE = 128;
static constexpr size_t DOWNLOAD_FEATURE_NAME_MAX_LEN = 256;
static const std::string CLOUD_ROM_BASE_PATH = "/data/service/el1/public/update/cloudRom/";
static const std::string PRODUCT_BASE_VERSION_KEY = "const.comp.hl.product_base_version.real";

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

bool DownloadManager::ValidateFeatureNames(const std::vector<std::string>& featureNames)
{
    CHECK_AND_RETURN_RET_LOG(!featureNames.empty(), false, "ValidateFeatureNames failed: featureNames is empty");
    CHECK_AND_RETURN_RET_LOG(featureNames.size() <= DOWNLOAD_FEATURE_NAMES_MAX_SIZE, false,
        "ValidateFeatureNames failed: featureNames size %{public}zu exceeds max %{public}zu",
        featureNames.size(), DOWNLOAD_FEATURE_NAMES_MAX_SIZE);
    for (const auto& featureName : featureNames) {
        CHECK_AND_RETURN_RET_LOG(!featureName.empty() && featureName.length() < DOWNLOAD_FEATURE_NAME_MAX_LEN, false,
            "ValidateFeatureNames failed: featureName length %{public}zu is invalid", featureName.length());
    }
    std::string logMsg = "ValidateFeatureNames: ";
    for (size_t i = 0; i < featureNames.size(); i++) {
        if (i > 0) {
            logMsg += ", ";
        }
        logMsg += featureNames[i];
    }
    AUDIO_INFO_LOG("%{public}s", logMsg.c_str());
    return true;
}

bool DownloadManager::ValidateFeatureNameSafe(const std::vector<std::string>& featureNames)
{
    for (const auto& featureName : featureNames) {
        for (char c : featureName) {
            CHECK_AND_RETURN_RET_LOG(std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-',
                false, "ValidateFeatureNameSafe: featureName contains invalid character, "
                "featureName: %{public}s, invalid char: %{public}c", featureName.c_str(), c);
        }
    }
    return true;
}

int32_t DownloadManager::GetInstallStatus(
    const std::vector<std::string>& featureNames, std::vector<bool>& installStatus)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_INFO_LOG("GetInstallStatus: featureNames size=%{public}zu", featureNames.size());

    CHECK_AND_RETURN_RET_LOG(ValidateFeatureNames(featureNames), ERR_INVALID_PARAM,
        "GetInstallStatus featureNames validation failed");
    CHECK_AND_RETURN_RET_LOG(ValidateFeatureNameSafe(featureNames), ERR_INVALID_PARAM,
        "GetInstallStatus featureName safe validation failed");
    installStatus.clear();

    char versionBuf[DOWNLOAD_FEATURE_NAME_MAX_LEN] = {0};
    int32_t ret = GetParameter(PRODUCT_BASE_VERSION_KEY.c_str(), "", versionBuf, sizeof(versionBuf));
    if (ret <= 0) {
        installStatus.assign(featureNames.size(), false);
        return SUCCESS;
    }

    std::string version(versionBuf);
    for (const auto& featureName : featureNames) {
        std::string path = CLOUD_ROM_BASE_PATH + version + "/" + featureName;
        bool installed = access(path.c_str(), F_OK) == 0;
        AUDIO_INFO_LOG("featureName:%{public}s, installed:%{public}d", featureName.c_str(), installed);
        installStatus.push_back(installed);
    }
    return SUCCESS;
}

int32_t DownloadManager::StartDownload(const std::vector<std::string>& featureNames, bool allowedCellularNet)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_INFO_LOG("StartDownload: featureNames size=%{public}zu", featureNames.size());

    CHECK_AND_RETURN_RET_LOG(ValidateFeatureNames(featureNames), ERR_INVALID_PARAM,
        "StartDownload featureNames validation failed");
    CHECK_AND_RETURN_RET_LOG(WaitForCallbackRegistration(), ERR_CALLBACK_NOT_REGISTERED, "download callback not reg");

    std::unique_lock<std::mutex> downloadLock(downloadInterfaceMutex_);
    CHECK_AND_RETURN_RET_LOG(downloadCallback_ != nullptr, ERR_CALLBACK_NOT_REGISTERED, "download callback is null");

    int32_t funcResult = SUCCESS;
    std::vector<AudioSuiteCloudRomVersionInfo> newVersionInfos;
    int32_t ret = downloadCallback_->GetFeatureNewVersion(featureNames, newVersionInfos, funcResult);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetFeatureNewVersion IPC failed, ret=%{public}d", ret);
    CHECK_AND_RETURN_RET_LOG(funcResult == SUCCESS, funcResult,
        "GetFeatureNewVersion failed, funcResult=%{public}d", funcResult);
    for (const auto& info : newVersionInfos) {
        CHECK_AND_RETURN_RET_LOG(info.errorCode == 0, info.errorCode,
            "GetFeatureNewVersion failed, errorCode=%{public}d", info.errorCode);
    }

    AUDIO_INFO_LOG("DownloadAndInstall: featureNames size=%{public}zu", featureNames.size());
    ret = downloadCallback_->DownloadAndInstall(allowedCellularNet, featureNames, funcResult);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "DownloadAndInstall IPC failed, ret=%{public}d", ret);
    CHECK_AND_RETURN_RET_LOG(funcResult == SUCCESS, funcResult,
        "DownloadAndInstall failed, funcResult=%{public}d", funcResult);
    AUDIO_INFO_LOG("StartDownload success");
    return SUCCESS;
}

int32_t DownloadManager::PauseDownload(const std::vector<std::string>& featureNames)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_INFO_LOG("PauseDownload: featureNames size=%{public}zu", featureNames.size());

    CHECK_AND_RETURN_RET_LOG(ValidateFeatureNames(featureNames), ERR_INVALID_PARAM,
        "PauseDownload featureNames validation failed");
    CHECK_AND_RETURN_RET_LOG(WaitForCallbackRegistration(), ERR_CALLBACK_NOT_REGISTERED, "download callback not reg");

    std::unique_lock<std::mutex> downloadLock(downloadInterfaceMutex_);
    CHECK_AND_RETURN_RET_LOG(downloadCallback_ != nullptr, ERR_CALLBACK_NOT_REGISTERED, "download callback is null");

    int32_t funcResult = SUCCESS;
    int32_t ret = downloadCallback_->PauseCloudRom(featureNames, funcResult);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "PauseCloudRom IPC failed, ret=%{public}d", ret);
    CHECK_AND_RETURN_RET_LOG(funcResult == SUCCESS, funcResult,
        "PauseCloudRom failed, funcResult=%{public}d", funcResult);
    AUDIO_INFO_LOG("PauseDownload success");
    return SUCCESS;
}

int32_t DownloadManager::CancelDownload(const std::vector<std::string>& featureNames)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_INFO_LOG("CancelDownload: featureNames size=%{public}zu", featureNames.size());

    CHECK_AND_RETURN_RET_LOG(ValidateFeatureNames(featureNames), ERR_INVALID_PARAM,
        "CancelDownload featureNames validation failed");
    CHECK_AND_RETURN_RET_LOG(WaitForCallbackRegistration(), ERR_CALLBACK_NOT_REGISTERED, "download callback not reg");

    std::unique_lock<std::mutex> downloadLock(downloadInterfaceMutex_);
    CHECK_AND_RETURN_RET_LOG(downloadCallback_ != nullptr, ERR_CALLBACK_NOT_REGISTERED, "download callback is null");
    if (backgroundDownload_ != nullptr) {
        for (const auto& featureName : featureNames) {
            backgroundDownload_->RemoveFeature(featureName);
        }
    }

    int32_t funcResult = SUCCESS;
    int32_t ret = downloadCallback_->CancelCloudRom(featureNames, funcResult);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "CancelCloudRom IPC failed, ret=%{public}d", ret);
    CHECK_AND_RETURN_RET_LOG(funcResult == SUCCESS, funcResult,
        "CancelCloudRom failed, funcResult=%{public}d", funcResult);
    AUDIO_INFO_LOG("CancelDownload success");
    return SUCCESS;
}

int32_t DownloadManager::StartBackgroundDownload(const std::vector<std::string>& featureNames)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_INFO_LOG("StartBackgroundDownload: featureNames size=%{public}zu", featureNames.size());
    CHECK_AND_RETURN_RET_LOG(ValidateFeatureNames(featureNames), ERR_INVALID_PARAM,
        "StartBackgroundDownload featureNames validation failed");
    CHECK_AND_RETURN_RET_LOG(WaitForCallbackRegistration(), ERR_CALLBACK_NOT_REGISTERED, "download callback not reg");

    std::vector<std::string> readyFeatures;
    std::vector<std::string> pendingFeatures;
    for (const auto& featureName : featureNames) {
        if (BackgroundDownload::AreConditionsMet(featureName)) {
            readyFeatures.push_back(featureName);
            if (backgroundDownload_ != nullptr) {
                backgroundDownload_->RemoveFeature(featureName);
            }
        } else {
            pendingFeatures.push_back(featureName);
        }
    }

    if (!pendingFeatures.empty()) {
        CHECK_AND_RETURN_RET_LOG(QueuePendingFeatures(pendingFeatures) == SUCCESS, ERROR,
            "QueuePendingFeatures failed");
    }

    if (readyFeatures.empty()) {
        AUDIO_INFO_LOG("No features ready for immediate download");
        return SUCCESS;
    }

    AUDIO_INFO_LOG("Download conditions met, start download, count=%{public}zu", readyFeatures.size());
    return SearchAndDownload(readyFeatures);
}

int32_t DownloadManager::QueuePendingFeatures(const std::vector<std::string>& pendingFeatures)
{
    if (backgroundDownload_ == nullptr) {
        AUDIO_INFO_LOG("BackgroundDownload start init backgroundDownload.");
        auto boundFunc = [this](const std::string& featureName) {
            return StartDownload(std::vector<std::string>{featureName}, false);
        };
        backgroundDownload_ = std::make_shared<BackgroundDownload>(boundFunc);
        int32_t bgInitRet = backgroundDownload_->Init();
        if (bgInitRet != 0) {
            backgroundDownload_ = nullptr;
            AUDIO_ERR_LOG("backgroundDownload_ init failed, ret=%{public}d", bgInitRet);
            return ERROR;
        }
    }
    for (const auto& featureName : pendingFeatures) {
        CHECK_AND_RETURN_RET_LOG(backgroundDownload_->AddFeature(featureName) == SUCCESS, ERROR,
            "Download conditions not met, adding to queue task failed");
    }
    AUDIO_INFO_LOG("BackgroundDownload conditions not met, adding to queue task, count=%{public}zu",
        pendingFeatures.size());
    return SUCCESS;
}

int32_t DownloadManager::SearchAndDownload(const std::vector<std::string>& featureNames)
{
    int32_t funcResult = SUCCESS;
    std::vector<AudioSuiteCloudRomVersionInfo> newVersionInfos;
    std::unique_lock<std::mutex> downloadLock(downloadInterfaceMutex_);
    CHECK_AND_RETURN_RET_LOG(downloadCallback_ != nullptr, ERR_CALLBACK_NOT_REGISTERED, "download callback is null");
    int32_t ret = downloadCallback_->GetFeatureNewVersion(featureNames, newVersionInfos, funcResult);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetFeatureNewVersion IPC failed, ret=%{public}d", ret);
    CHECK_AND_RETURN_RET_LOG(funcResult == SUCCESS, funcResult,
        "GetFeatureNewVersion failed, funcResult=%{public}d", funcResult);
    for (const auto& info : newVersionInfos) {
        CHECK_AND_RETURN_RET_LOG(info.errorCode == 0, info.errorCode,
            "GetFeatureNewVersion failed, errorCode=%{public}d", info.errorCode);
    }

    ret = downloadCallback_->DownloadAndInstall(false, featureNames, funcResult);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "DownloadAndInstall IPC failed, ret=%{public}d", ret);
    CHECK_AND_RETURN_RET_LOG(funcResult == SUCCESS, funcResult,
        "DownloadAndInstall failed, funcResult=%{public}d", funcResult);
    AUDIO_INFO_LOG("SearchAndDownload success");
    return SUCCESS;
}

int32_t DownloadManager::GetDownloadStatus(
    const std::vector<std::string>& featureNames, std::vector<AudioSuiteDownloadStatusInfo>& status)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_INFO_LOG("GetDownloadStatus: featureNames size=%{public}zu", featureNames.size());

    CHECK_AND_RETURN_RET_LOG(ValidateFeatureNames(featureNames), ERR_INVALID_PARAM,
        "GetDownloadStatus featureNames validation failed");
    CHECK_AND_RETURN_RET_LOG(WaitForCallbackRegistration(), ERR_CALLBACK_NOT_REGISTERED, "download callback not reg");

    std::unique_lock<std::mutex> downloadLock(downloadInterfaceMutex_);
    CHECK_AND_RETURN_RET_LOG(downloadCallback_ != nullptr, ERR_CALLBACK_NOT_REGISTERED, "download callback is null");

    int32_t funcResult = SUCCESS;
    int32_t ret = downloadCallback_->GetFeatureStatus(featureNames, status, funcResult);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetFeatureStatus IPC failed, ret=%{public}d", ret);
    CHECK_AND_RETURN_RET_LOG(funcResult == SUCCESS, funcResult,
        "GetFeatureStatus failed, funcResult=%{public}d", funcResult);

    std::vector<std::string> needSizeFeatures;
    for (auto& s : status) {
        s.size = 0;
        AUDIO_INFO_LOG("%{public}s GetDownloadStatus status:%{public}d, errorCode:%{public}d",
            s.featureName.c_str(), s.state, s.errorCode);
        if (s.state != FEATURE_INSTALL_SUCCESS) {
            needSizeFeatures.push_back(s.featureName);
        }
    }

    CHECK_AND_RETURN_RET_LOG(!needSizeFeatures.empty(), SUCCESS,
        "GetDownloadStatus success, status count=%{public}zu", status.size());
    std::vector<AudioSuiteCloudRomVersionInfo> newVersionInfos;
    ret = downloadCallback_->GetFeatureNewVersion(needSizeFeatures, newVersionInfos, funcResult);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS && funcResult == SUCCESS, SUCCESS,
        "Failed to search for versions, ret=%{public}d, funcResult=%{public}d", ret, funcResult);

    for (const auto& versionInfo : newVersionInfos) {
        AUDIO_INFO_LOG("%{public}s GetFeatureNewVersion, size:%{public}" PRId64 ", errorCode:%{public}d",
            versionInfo.featureName.c_str(), versionInfo.size, versionInfo.errorCode);
        CHECK_AND_CONTINUE(versionInfo.errorCode == SUCCESS);

        for (auto& s : status) {
            if (s.featureName == versionInfo.featureName) {
                s.size = versionInfo.size;
                break;
            }
        }
    }
    return SUCCESS;
}

int32_t DownloadManager::GetFeatureNewVersion(
    const std::vector<std::string>& featureNames, std::vector<AudioSuiteCloudRomVersionInfo>& versionInfos)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_INFO_LOG("GetFeatureNewVersion: featureNames size=%{public}zu", featureNames.size());

    CHECK_AND_RETURN_RET_LOG(ValidateFeatureNames(featureNames), ERR_INVALID_PARAM,
        "GetFeatureNewVersion featureNames validation failed");
    CHECK_AND_RETURN_RET_LOG(WaitForCallbackRegistration(), ERR_CALLBACK_NOT_REGISTERED,
        "GetFeatureNewVersion callback not reg");

    std::unique_lock<std::mutex> downloadLock(downloadInterfaceMutex_);
    CHECK_AND_RETURN_RET_LOG(downloadCallback_ != nullptr, ERR_CALLBACK_NOT_REGISTERED,
        "GetFeatureNewVersion callback is null");

    int32_t funcResult = SUCCESS;
    int32_t ret = downloadCallback_->GetFeatureNewVersion(featureNames, versionInfos, funcResult);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetFeatureNewVersion IPC failed, ret=%{public}d", ret);
    CHECK_AND_RETURN_RET_LOG(funcResult == SUCCESS, funcResult,
        "GetFeatureNewVersion failed, funcResult=%{public}d", funcResult);
    AUDIO_INFO_LOG("GetFeatureNewVersion success, versionInfos count=%{public}zu", versionInfos.size());
    return SUCCESS;
}

int32_t DownloadManager::UninstallCloudRom(const std::vector<std::string>& featureNames)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_INFO_LOG("UninstallCloudRom: featureNames size=%{public}zu", featureNames.size());

    CHECK_AND_RETURN_RET_LOG(ValidateFeatureNames(featureNames), ERR_INVALID_PARAM,
        "UninstallCloudRom featureNames validation failed");
    CHECK_AND_RETURN_RET_LOG(WaitForCallbackRegistration(), ERR_CALLBACK_NOT_REGISTERED, "download callback not reg");

    std::unique_lock<std::mutex> downloadLock(downloadInterfaceMutex_);
    CHECK_AND_RETURN_RET_LOG(downloadCallback_ != nullptr, ERR_CALLBACK_NOT_REGISTERED, "download callback is null");

    int32_t funcResult = SUCCESS;
    int32_t ret = downloadCallback_->UninstallCloudRom(featureNames, funcResult);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "UninstallCloudRom IPC failed, ret=%{public}d", ret);
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
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "RegisterDownloadStatusCallback IPC failed, ret=%{public}d", ret);
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
            AUDIO_ERR_LOG("UnRegisterCloudRomDownloadCallback failed, ret=%{public}d, funcResult=%{public}d",
                ret, funcResult);
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