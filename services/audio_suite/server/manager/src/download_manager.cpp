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

namespace {
constexpr const char* CHECK_CONDITION_REGION = "china";
constexpr const char* EMPTY_STRING = "";
constexpr int32_t MAX_RETRY_COUNT = 3;
} // namespace

#include "audio_errors.h"
#include "parameter.h"
#include "audio_suite_log.h"
#include "update_engine_callback_proxy.h"
#include "cloud_rom_download_callback_impl.h"

using namespace OHOS::AudioStandard::AudioSuite;

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {


DownloadManager::DownloadManager()
    : downloadState_(DownloadState::DOWNLOAD_IDLE),
      downloadProgress_(0),
      errorCode_(0),
      isDownloading_(false),
      downloadRetryCount_(0),
      cancelRequested_(false),
      isCallbackRegistered_(false),
      isBackgroundDownload_(false)
{
    AUDIO_INFO_LOG("DownloadManager constructor");
}

DownloadManager::~DownloadManager()
{
    AUDIO_INFO_LOG("DownloadManager destructor");
    Release();
}

int32_t DownloadManager::Init()
{
    std::lock_guard<std::mutex> lock(mutex_);

    AUDIO_INFO_LOG("DownloadManager Init success");
    return 0;
}

int32_t DownloadManager::Release()
{
    std::unique_lock<std::mutex> lock(mutex_);

    AUDIO_INFO_LOG("DownloadManager Release");

    if (downloadState_ == DownloadState::DOWNLOAD_IN_PROGRESS) {
        cancelRequested_ = true;

        if (isDownloading_) {
            std::string versionToCancel = currentVersion_;
            lock.unlock();
            PauseDownloadCloudRomViaCallback(versionToCancel);
            lock.lock();
        }
    }

    downloadCallback_ = nullptr;

    downloadState_ = DownloadState::DOWNLOAD_IDLE;
    downloadProgress_ = 0;
    isDownloading_ = false;
    downloadRetryCount_ = 0;

    lock.unlock();

    AUDIO_INFO_LOG("DownloadManager Release success");
    return 0;
}

int32_t DownloadManager::StartDownload(const std::string& version)
{
    std::lock_guard<std::mutex> lock(mutex_);

    AUDIO_INFO_LOG("StartDownload: version=%{public}s", version.c_str());

    if (version.empty()) {
        AUDIO_ERR_LOG("Invalid parameter: version is empty");
        return ERR_INVALID_PARAM;
    }

    // 保存版本信息
    currentVersion_ = version;
    isBackgroundDownload_ = false;  // 正常下载

    // 重置状态
    downloadProgress_ = 0;
    errorCode_ = 0;
    cancelRequested_ = false;

    // 更新状态为等待中
    UpdateDownloadState(DownloadState::DOWNLOAD_WAITING);

    DoDownload();

    AUDIO_INFO_LOG("StartDownload success");
    return 0;
}

int32_t DownloadManager::CancelDownload()
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_INFO_LOG("CancelDownload Start");

    bool needCancel = false;
    std::string versionToCancel;

    if (downloadState_ != DownloadState::DOWNLOAD_IN_PROGRESS && downloadState_ != DownloadState::DOWNLOAD_WAITING) {
        AUDIO_WARNING_LOG("No download to cancel, state=%{public}d", downloadState_.load());
        return ERR_OPERATION_FAILED;
    }

    cancelRequested_ = true;
    needCancel = isDownloading_;
    versionToCancel = currentVersion_;

    if (needCancel) {
        PauseDownloadCloudRomViaCallback(versionToCancel);
    }

    UpdateDownloadState(DownloadState::DOWNLOAD_CANCELLED);

    AUDIO_INFO_LOG("CancelDownload success");
    return 0;
}

int32_t DownloadManager::StartBackgroundDownload(const std::string& version)
{
    std::lock_guard<std::mutex> lock(mutex_);

    AUDIO_INFO_LOG("StartBackgroundDownload: version=%{public}s", version.c_str());

    if (version.empty()) {
        AUDIO_ERR_LOG("Invalid parameter: version is empty");
        return ERR_INVALID_PARAM;
    }

    if (downloadState_ == DownloadState::DOWNLOAD_IN_PROGRESS) {
        AUDIO_ERR_LOG("Download already in progress");
        return ERR_OPERATION_FAILED;
    }

    // 保存版本信息和条件监听器
    currentVersion_ = version;
    isBackgroundDownload_ = true;   // 后台下载

    // 重置状态
    downloadProgress_ = 0;
    errorCode_ = 0;
    cancelRequested_ = false;

    // 更新状态为等待中
    UpdateDownloadState(DownloadState::DOWNLOAD_WAITING);

    DoDownload();

    AUDIO_INFO_LOG("StartBackgroundDownload success");
    return 0;
}

int32_t DownloadManager::GetDownloadStatus(DownloadStatus& status, const std::string& version)
{
    std::lock_guard<std::mutex> lock(mutex_);

    AUDIO_INFO_LOG("GetDownloadStatus: version=%{public}s", version.c_str());

    if (version.empty()) {
        AUDIO_ERR_LOG("GetDownloadStatus: version is empty");
        return ERR_INVALID_PARAM;
    }

    auto proxy = GetUpdateEngineCallbackProxy();
    if (proxy == nullptr) {
        return ERR_AUDIO_SUITE_SA_CALLBACK_NOT_REGISTERED;
    }

    StatusInfo statusInfo;
    int32_t ret = GetFeatureStatusInfo(proxy, version, statusInfo);
    if (ret != SUCCESS) {
        return ret;
    }

    CloudRomNewVersionInfo versionInfo;
    ret = GetFeatureNewVersionInfo(proxy, version, versionInfo);
    if (ret != SUCCESS) {
        return ret;
    }

    FillDownloadStatus(status, version, statusInfo, versionInfo);

    AUDIO_INFO_LOG("GetDownloadStatus success, state=%{public}d, progress=%{public}u",
                status.state, status.progress);
    return SUCCESS;
}

sptr<IUpdateEngineCallback> DownloadManager::GetUpdateEngineCallbackProxy()
{
    sptr<IRemoteObject> callbackCopy;
    {
        std::lock_guard<std::mutex> lock(updateEngineCallbackMutex_);
        callbackCopy = updateEngineCallback_;
    }

    if (callbackCopy == nullptr) {
        AUDIO_ERR_LOG("GetDownloadStatus: UpdateEngine callback is null");
        return nullptr;
    }

    auto proxy = iface_cast<IUpdateEngineCallback>(callbackCopy);
    if (proxy == nullptr) {
        AUDIO_ERR_LOG("GetDownloadStatus: Failed to cast IUpdateEngineCallback");
        return nullptr;
    }

    return proxy;
}

int32_t DownloadManager::GetFeatureStatusInfo(const sptr<IUpdateEngineCallback>& proxy,
    const std::string& version, StatusInfo& statusInfo)
{
    std::vector<FeatureInfo> featureInfos;
    featureInfos.emplace_back(FeatureInfo{.featureName = version, .info = EMPTY_STRING});
    StatusInfoResult statusInfoResult;

    int32_t ret = proxy->GetFeatureStatus(featureInfos, statusInfoResult, ret);
    if (ret != SUCCESS || statusInfoResult.errorInfo.errorNum != 0) {
        AUDIO_ERR_LOG("GetDownloadStatus: GetFeatureStatus failed, ret=%{public}d, errorNum=%{public}d",
            ret, statusInfoResult.errorInfo.errorNum);
        AUDIO_ERR_LOG("errorMsg=%{public}s", statusInfoResult.errorInfo.message.c_str());
        return ERR_AUDIO_SUITE_SA_CALLBACK_NOT_REGISTERED;
    }

    if (statusInfoResult.statusInfos.empty()) {
        AUDIO_ERR_LOG("GetDownloadStatus: GetFeatureStatus returned empty statusInfos");
        return ERR_AUDIO_SUITE_SA_CALLBACK_NOT_REGISTERED;
    }

    const auto& info = statusInfoResult.statusInfos[0];
    if (info.featureInfo.featureName != version) {
        AUDIO_ERR_LOG("GetDownloadStatus: featureName mismatch, expected=%{public}s, got=%{public}s",
            version.c_str(), info.featureInfo.featureName.c_str());
        return ERR_INVALID_PARAM;
    }

    statusInfo = info;
    return SUCCESS;
}

int32_t DownloadManager::GetFeatureNewVersionInfo(const sptr<IUpdateEngineCallback>& proxy,
    const std::string& version, CloudRomNewVersionInfo& versionInfo)
{
    std::vector<FeatureInfo> featureInfos;
    featureInfos.emplace_back(FeatureInfo{.featureName = version, .info = EMPTY_STRING});
    CloudRomCheckResult newVersionInfos;
    CheckCondition checkCondition = {CHECK_CONDITION_REGION, EMPTY_STRING};

    int32_t ret = proxy->GetFeatureNewVersion(checkCondition, featureInfos, newVersionInfos, ret);
    if (ret != SUCCESS || newVersionInfos.errorInfo.errorNum != 0) {
        AUDIO_ERR_LOG("GetDownloadStatus: GetFeatureNewVersion failed, ret=%{public}d, errorNum=%{public}d",
            ret, newVersionInfos.errorInfo.errorNum);
        AUDIO_ERR_LOG("errorMsg=%{public}s", newVersionInfos.errorInfo.message.c_str());
        return ERR_AUDIO_SUITE_SA_CALLBACK_NOT_REGISTERED;
    }

    if (newVersionInfos.newVersionInfos.empty()) {
        AUDIO_ERR_LOG("GetDownloadStatus: GetFeatureNewVersion returned empty newVersionInfos");
        return ERR_AUDIO_SUITE_SA_CALLBACK_NOT_REGISTERED;
    }

    const auto& info = newVersionInfos.newVersionInfos[0];
    if (info.featureInfo.featureName != version) {
        AUDIO_ERR_LOG("GetDownloadStatus: featureName mismatch in newVersionInfos, expected=%{public}s, got=%{public}s",
            version.c_str(), info.featureInfo.featureName.c_str());
        return ERR_INVALID_PARAM;
    }

    versionInfo = info;
    return SUCCESS;
}

void DownloadManager::FillDownloadStatus(DownloadStatus& status, const std::string& version,
    const StatusInfo& statusInfo, const CloudRomNewVersionInfo& versionInfo)
{
    status.version = version;
    status.progress = statusInfo.progress;
    status.romPath = statusInfo.installPath;
    status.fileSize = versionInfo.size;
    status.errorCode = statusInfo.errorInfo.errorNum;
    status.errMessage = statusInfo.errorInfo.message;

    switch (statusInfo.status) {
        case CloudRomStatus::DOWNLOADING:
            status.state = DownloadState::DOWNLOAD_IN_PROGRESS;
            break;
        case CloudRomStatus::DOWNLOAD_SUCCESS:
        case CloudRomStatus::INSTALL_SUCCESS:
            status.state = DownloadState::DOWNLOAD_COMPLETED;
            break;
        case CloudRomStatus::DOWNLOAD_FAIL:
        case CloudRomStatus::INSTALL_FAIL:
            status.state = DownloadState::DOWNLOAD_FAILED;
            break;
        default:
            status.state = DownloadState::DOWNLOAD_IDLE;
            break;
    }
}

void DownloadManager::RegisterDownloadCallback(std::function<void(const DownloadStatus &)> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    downloadCallback_ = callback;
}

void DownloadManager::DoDownload()
{
    AUDIO_INFO_LOG("DoDownload started");

    // 更新状态为下载中
    isDownloading_ = true;
    UpdateDownloadStateInternal(DownloadState::DOWNLOAD_IN_PROGRESS);

    // 调用真实的云 ROM 下载接口（DownloadAndInstall 内部有锁）
    if (!DownloadAndInstall(currentVersion_)) {
        errorCode_ = ERR_AUDIO_SUITE_SA_DOWNLOAD_FAILED;
        errMessage_ = "Download and install failed";
        UpdateDownloadStateInternal(DownloadState::DOWNLOAD_FAILED);
        DownloadStatus status;
        status.state = DownloadState::DOWNLOAD_FAILED;
        status.progress = 0;
        status.version = currentVersion_;
        status.romPath = EMPTY_STRING;
        status.errorCode = errorCode_;
        status.errMessage = errMessage_;
        NotifyDownloadStatusChanged(status);
        return;
    }
    // 注意：下载完成/失败通过 OnCloudRomCallback 回调异步处理
    AUDIO_INFO_LOG("DoDownload initiated successfully");
}

void DownloadManager::UpdateDownloadStateInternal(DownloadState state, int32_t errorCode)
{
    downloadState_ = state;
    errorCode_ = errorCode;

    AUDIO_INFO_LOG("Download state changed to %{public}d, errorCode=%{public}d", state, errorCode);
}

void DownloadManager::UpdateDownloadState(DownloadState state, int32_t errorCode)
{
    UpdateDownloadStateInternal(state, errorCode);
}

void DownloadManager::NotifyDownloadStatusChanged(const DownloadStatus &status)
{
    AUDIO_INFO_LOG(
        "NotifyDownloadStatusChanged: version=%{public}s, state=%{public}d, progress=%{public}u, errorCode=%{public}d",
        status.version.c_str(),
        static_cast<int32_t>(status.state),
        status.progress,
        status.errorCode);

    std::function<void(const DownloadStatus &)> callback;

    callback = downloadCallback_;

    if (callback != nullptr) {
        AUDIO_INFO_LOG("NotifyDownloadStatusChanged: invoking callback");
        callback(status);
    } else {
        AUDIO_WARNING_LOG("NotifyDownloadStatusChanged: callback is null");
    }
}

bool DownloadManager::GetRemoteInfo(const std::string &version, bool useCache)
{
    // 等待回调注册，超时 1 秒
    if (!WaitForCallbackRegistration(CALLBACK_REGISTER_TIMEOUT_MS)) {
        AUDIO_ERR_LOG("UpdateEngine callback not registered within timeout");
        return false;
    }

    return GetRemoteInfoViaCallback(version, useCache);
}

bool DownloadManager::IsDownloaded(const std::string &modelName, bool useCache)
{
    // 等待回调注册，超时 1 秒
    if (!WaitForCallbackRegistration(CALLBACK_REGISTER_TIMEOUT_MS)) {
        AUDIO_ERR_LOG("UpdateEngine callback not registered within timeout");
        return false;
    }

    return IsDownloadedViaCallback(modelName, useCache);
}

bool DownloadManager::DownloadAndInstall(const std::string &version)
{
    if (IsDownloaded(version, false)) {
        return true;
    }

    if (!GetRemoteInfo(version, false)) {
        isDownloading_ = false;
        return false;
    }

    return DownloadAndInstallViaCallback(version);
}

int32_t DownloadManager::UninstallCloudRom(const std::string& version)
{
    // 等待回调注册，超时 1 秒
    if (!WaitForCallbackRegistration(CALLBACK_REGISTER_TIMEOUT_MS)) {
        AUDIO_ERR_LOG("UpdateEngine callback not registered within timeout");
        return ERR_AUDIO_SUITE_SA_CALLBACK_NOT_REGISTERED;
    }

    return UninstallCloudRomViaCallback(version);
}

ErrCode DownloadManager::OnCloudRomCallback(const CloudRomCallback &cloudRomData)
{
    static int callbackCount = 0;
    callbackCount++;
    AUDIO_INFO_LOG("OnCloudRomCallback: ENTRY (count=%{public}d), statusInfos size=%{public}zu",
                  callbackCount, cloudRomData.statusInfos.size());
    AUDIO_INFO_LOG("currentVersion_=%{public}s", currentVersion_.c_str());

    AUDIO_INFO_LOG("OnCloudRomCallback478: statusInfos size=%{public}zu, currentVersion_=%{public}s",
                  cloudRomData.statusInfos.size(), currentVersion_.c_str());

    for (const StatusInfo &info : cloudRomData.statusInfos) {
        AUDIO_INFO_LOG("OnCloudRomCallback: processing featureName=%{public}s, status=%{public}d, progress=%{public}d",
                      info.featureInfo.featureName.c_str(),
                      static_cast<int32_t>(info.status),
                      info.progress);

        if (info.featureInfo.featureName != currentVersion_) {
            AUDIO_INFO_LOG("OnCloudRomCallback: featureName mismatch, skipping");
            continue;
        }

        ProcessCloudRomStatus(info);
        break;
    }
    AUDIO_INFO_LOG("OnCloudRomCallback: EXIT, returning SUCCESS");
    return SUCCESS;
}

void DownloadManager::ProcessCloudRomStatus(const StatusInfo &info)
{
    switch (info.status) {
        case CloudRomStatus::INSTALL_SUCCESS:
            HandleInstallSuccess(info);
            break;
        case CloudRomStatus::DOWNLOAD_FAIL:
            HandleDownloadFail(info);
            break;
        case CloudRomStatus::DOWNLOADING:
            HandleDownloading(info);
            break;
        default:
            AUDIO_INFO_LOG("version %{public}s download Status, status %{public}d",
                           info.featureInfo.featureName.c_str(), info.status);
            break;
    }
}

void DownloadManager::HandleInstallSuccess(const StatusInfo &info)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_INFO_LOG("version %{public}s install success", info.featureInfo.featureName.c_str());
    romPath_ = info.installPath + "/" + info.featureInfo.featureName;
    isDownloading_ = false;
    downloadRetryCount_ = 0;
    UpdateDownloadStateInternal(DownloadState::DOWNLOAD_COMPLETED);

    DownloadStatus status;
    status.version = currentVersion_;
    status.state = DownloadState::DOWNLOAD_COMPLETED;
    status.progress = 100;
    status.romPath = romPath_;
    status.errMessage = EMPTY_STRING;
    NotifyDownloadStatusChanged(status);
}

void DownloadManager::HandleDownloadFail(const StatusInfo &info)
{
    std::unique_lock<std::mutex> lock(mutex_);
    AUDIO_ERR_LOG("version %{public}s download failed", info.featureInfo.featureName.c_str());
    isDownloading_ = false;
    errorCode_ = ERR_AUDIO_SUITE_SA_DOWNLOAD_FAILED;

    bool shouldRetry = (downloadRetryCount_ < MAX_RETRY_COUNT);
    if (shouldRetry) {
        downloadRetryCount_++;
        lock.unlock();

        AUDIO_INFO_LOG("Retry download version: %{public}s, retryCount=%{public}d",
                       info.featureInfo.featureName.c_str(), downloadRetryCount_.load());
        if (!DownloadAndInstall(info.featureInfo.featureName)) {
            NotifyDownloadFailedAfterRetry();
        }
    } else {
        downloadRetryCount_ = 0;
        lock.unlock();

        DownloadStatus status;
        status.version = currentVersion_;
        status.state = DownloadState::DOWNLOAD_FAILED;
        status.progress = downloadProgress_.load();
        status.errorCode = errorCode_;
        status.errMessage = "Download failed";
        NotifyDownloadStatusChanged(status);
    }
}

void DownloadManager::NotifyDownloadFailedAfterRetry()
{
    std::lock_guard<std::mutex> lock(mutex_);
    downloadRetryCount_ = 0;
    UpdateDownloadStateInternal(DownloadState::DOWNLOAD_FAILED);

    DownloadStatus status;
    status.version = currentVersion_;
    status.state = DownloadState::DOWNLOAD_FAILED;
    status.progress = downloadProgress_.load();
    status.errorCode = errorCode_;
    status.errMessage = "Download and install failed after retry";
    NotifyDownloadStatusChanged(status);
}

void DownloadManager::HandleDownloading(const StatusInfo &info)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_DEBUG_LOG("version %{public}s install progress = %{public}d",
                    info.featureInfo.featureName.c_str(), info.progress);
    downloadProgress_ = info.progress;

    DownloadStatus status;
    status.version = currentVersion_;
    status.state = DownloadState::DOWNLOAD_IN_PROGRESS;
    status.progress = info.progress;
    status.errMessage = EMPTY_STRING;
    NotifyDownloadStatusChanged(status);
}

bool DownloadManager::RegisterCallback()
{
    std::lock_guard<std::mutex> lock(mutex_);

    AUDIO_INFO_LOG("RegisterCallback start");
    // 等待 UpdateEngine 回调注册完成
    if (!WaitForCallbackRegistration()) {
        AUDIO_ERR_LOG("WaitForCallbackRegistration timeout");
        return false;
    }

    // 注册云 ROM 下载回调到 audio_policy_ext
    if (!RegisterCloudRomDownloadCallback()) {
        AUDIO_ERR_LOG("RegisterCloudRomDownloadCallback failed");
        return false;
    }

    return true;
}

void DownloadManager::SetUpdateEngineCallback(const sptr<IRemoteObject>& updateEngineCallback)
{
    {
        std::lock_guard<std::mutex> lock(updateEngineCallbackMutex_);
        updateEngineCallback_ = updateEngineCallback;
    }
     // 通知回调注册完成
    {
        std::unique_lock<std::mutex> callbackLock(callbackMutex_);
        isCallbackRegistered_ = true;
        callbackRegisteredCond_.notify_all();
    }
    AUDIO_INFO_LOG("SetUpdateEngineCallback: %{public}s", updateEngineCallback ? "set" : "cleared");
}

bool DownloadManager::WaitForCallbackRegistration(int timeoutMs)
{
    std::unique_lock<std::mutex> lock(callbackMutex_);

    if (isCallbackRegistered_) {
        return true;
    }

    AUDIO_INFO_LOG("Waiting for callback registration, timeout=%{public}dms", timeoutMs);

    auto status = callbackRegisteredCond_.wait_for(lock, std::chrono::milliseconds(timeoutMs),
        [this] { return isCallbackRegistered_.load(); });

    if (!status) {
        AUDIO_ERR_LOG("WaitForCallbackRegistration timeout after %{public}dms", timeoutMs);
        return false;
    }

    AUDIO_INFO_LOG("WaitForCallbackRegistration success");
    return true;
}

bool DownloadManager::RegisterCloudRomDownloadCallback()
{
    std::lock_guard<std::mutex> lock(updateEngineCallbackMutex_);

    if (!updateEngineCallback_) {
        AUDIO_ERR_LOG("UpdateEngine callback not set");
        return false;
    }

    // 创建云 ROM 下载回调实现
    cloudRomDownloadCallback_ = sptr<CloudRomDownloadCallbackImpl>::MakeSptr(this);
    if (!cloudRomDownloadCallback_) {
        AUDIO_ERR_LOG("Failed to create CloudRomDownloadCallbackImpl");
        return false;
    }

    // 通过 UpdateEngine 回调接口注册到 audio_policy_ext
    auto updateEngineCallbackProxy = iface_cast<IUpdateEngineCallback>(updateEngineCallback_);
    if (!updateEngineCallbackProxy) {
        AUDIO_ERR_LOG("Failed to cast updateEngineCallback to IUpdateEngineCallback");
        return false;
    }

    int32_t ret = updateEngineCallbackProxy->RegisterCloudRomCallback(cloudRomDownloadCallback_->AsObject(), ret);
    if (ret != 0) {
        AUDIO_ERR_LOG("RegisterCloudRomCallback failed, ret=%{public}d", ret);
        return false;
    }

    AUDIO_INFO_LOG("RegisterCloudRomDownloadCallback success");

    return true;
}

// CloudRomDownloadCallbackImpl 实现
ErrCode CloudRomDownloadCallbackImpl::OnCloudRomCallback(const CloudRomCallback& cloudRomData)
{
    static int stubCallbackCount = 0;
    stubCallbackCount++;
    AUDIO_INFO_LOG("CloudRomDownloadCallbackImpl::OnCloudRomCallback: ENTRY (count=%{public}d)",
                  stubCallbackCount);
    AUDIO_INFO_LOG("statusInfos size=%{public}zu", cloudRomData.statusInfos.size());

    if (!downloadManager_) {
        AUDIO_ERR_LOG("DownloadManager is null");
        return ERR_INVALID_OPERATION;
    }

    // 直接调用 DownloadManager 的回调处理
    return downloadManager_->OnCloudRomCallback(cloudRomData);
}

bool DownloadManager::GetRemoteInfoViaCallback(const std::string &version, bool useCache)
{
    if (useCache) {
        return false;
    }

    auto proxy = GetUpdateEngineCallbackProxy();
    if (proxy == nullptr) {
        return false;
    }

    return GetRemoteInfoFromProxy(proxy, version);
}

bool DownloadManager::GetRemoteInfoFromProxy(const sptr<IUpdateEngineCallback>& proxy,
    const std::string& version)
{
    std::vector<FeatureInfo> featureInfos;
    featureInfos.emplace_back(FeatureInfo{.featureName = version, .info = EMPTY_STRING});
    CloudRomCheckResult newVersionInfos;
    CheckCondition checkCondition = {CHECK_CONDITION_REGION, EMPTY_STRING};

    int32_t ret = proxy->GetFeatureNewVersion(checkCondition, featureInfos, newVersionInfos, ret);
    if (ret != SUCCESS || newVersionInfos.errorInfo.errorNum != 0) {
        AUDIO_ERR_LOG("GetFeatureNewVersion %{public}s failed, errorNum: %{public}d",
                      version.c_str(), newVersionInfos.errorInfo.errorNum);
        AUDIO_ERR_LOG("errorMsg: %{public}s", newVersionInfos.errorInfo.message.c_str());
        return false;
    }

    for (const auto &info : newVersionInfos.newVersionInfos) {
        if (info.featureInfo.featureName == version) {
            AUDIO_INFO_LOG("Get %{public}s Version success", version.c_str());
            return true;
        }
    }
    return false;
}

// 新增：通过回调检查是否已下载
bool DownloadManager::IsDownloadedViaCallback(const std::string &modelName, bool useCache)
{
    sptr<IRemoteObject> callbackCopy;
    {
        std::lock_guard<std::mutex> lock(updateEngineCallbackMutex_);
        callbackCopy = updateEngineCallback_;
    }

    if (callbackCopy == nullptr) {
        AUDIO_ERR_LOG("UpdateEngine callback is null");
        return false;
    }

    auto proxy = iface_cast<IUpdateEngineCallback>(callbackCopy);
    if (proxy == nullptr) {
        AUDIO_ERR_LOG("Failed to cast IUpdateEngineCallback");
        return false;
    }

    std::vector<FeatureInfo> featureInfos;
    featureInfos.emplace_back(FeatureInfo{.featureName = modelName, .info = EMPTY_STRING});
    StatusInfoResult statusInfoResult;

    int32_t ret = proxy->GetFeatureStatus(featureInfos, statusInfoResult, ret);
    if (ret != SUCCESS || statusInfoResult.errorInfo.errorNum != 0) {
        AUDIO_ERR_LOG("GetFeatureStatus failed, ret=%{public}d, errorNum=%{public}d, errorMsg=%{public}s",
            ret, statusInfoResult.errorInfo.errorNum, statusInfoResult.errorInfo.message.c_str());
        return false;
    }

    if (statusInfoResult.statusInfos.empty()) {
        return false;
    }

    const auto &info = statusInfoResult.statusInfos[0];
    if (info.featureInfo.featureName != modelName || info.status != CloudRomStatus::INSTALL_SUCCESS) {
        return false;
    }
    return true;
}

// 新增：通过回调下载并安装
bool DownloadManager::DownloadAndInstallViaCallback(const std::string &version)
{
    sptr<IRemoteObject> callbackCopy;
    {
        std::lock_guard<std::mutex> lock(updateEngineCallbackMutex_);
        callbackCopy = updateEngineCallback_;
    }

    if (callbackCopy == nullptr) {
        AUDIO_ERR_LOG("UpdateEngine callback is null");
        return false;
    }

    auto proxy = iface_cast<IUpdateEngineCallback>(callbackCopy);
    if (proxy == nullptr) {
        AUDIO_ERR_LOG("Failed to cast IUpdateEngineCallback");
        return false;
    }

    std::vector<FeatureInfo> featureInfos;
    featureInfos.emplace_back(FeatureInfo{.featureName = version, .info = EMPTY_STRING});
    BusinessError businessError;
    DownloadCondition downloadCondition = {CLOUD_ROM_NETTYPE, EMPTY_STRING, isBackgroundDownload_.load()};

    int32_t ret = proxy->DownloadAndInstall(downloadCondition, featureInfos, businessError, ret);
    if (businessError.errorNum != 0) {
        AUDIO_ERR_LOG("%{public}s failed, errorNum: %{public}d errorMsg: %{public}s", version.c_str(),
                      businessError.errorNum, businessError.message.c_str());
        isDownloading_ = false;
        return false;
    }
    return true;
}

// 新增：通过回调取消下载
void DownloadManager::PauseDownloadCloudRomViaCallback(const std::string &version)
{
    sptr<IRemoteObject> callbackCopy;
    {
        std::lock_guard<std::mutex> lock(updateEngineCallbackMutex_);
        callbackCopy = updateEngineCallback_;
    }

    if (callbackCopy == nullptr) {
        AUDIO_ERR_LOG("UpdateEngine callback is null");
        return;
    }

    auto proxy = iface_cast<IUpdateEngineCallback>(callbackCopy);
    if (proxy == nullptr) {
        AUDIO_ERR_LOG("Failed to cast IUpdateEngineCallback");
        return;
    }

    std::vector<FeatureInfo> featureInfos;
    featureInfos.emplace_back(FeatureInfo{.featureName = version, .info = EMPTY_STRING});
    BusinessError businessError;
    PauseCondition pauseCondition = {EMPTY_STRING, false, EMPTY_STRING};

    int32_t pauseRet = proxy->PauseDownloadCloudRom(pauseCondition, featureInfos, businessError, pauseRet);
}

// 新增：通过回调卸载云 ROM
int32_t DownloadManager::UninstallCloudRomViaCallback(const std::string& version)
{
    sptr<IRemoteObject> callbackCopy;
    {
        std::lock_guard<std::mutex> lock(updateEngineCallbackMutex_);
        callbackCopy = updateEngineCallback_;
    }

    if (callbackCopy == nullptr) {
        AUDIO_ERR_LOG("UpdateEngine callback is null");
        return ERR_INVALID_PARAM;
    }

    auto proxy = iface_cast<IUpdateEngineCallback>(callbackCopy);
    if (proxy == nullptr) {
        AUDIO_ERR_LOG("Failed to cast IUpdateEngineCallback");
        return ERR_INVALID_PARAM;
    }

    std::vector<FeatureInfo> featureInfos;
    featureInfos.emplace_back(FeatureInfo{.featureName = version, .info = EMPTY_STRING});
    BusinessError businessError;

    int32_t ret = proxy->UninstallCloudRom(featureInfos, businessError, ret);
    if (ret != SUCCESS || businessError.errorNum != 0) {
        AUDIO_ERR_LOG("UninstallCloudRom failed, ret=%{public}d, errorNum=%{public}d, errorMsg=%{public}s",
            ret, businessError.errorNum, businessError.message.c_str());
        return ERR_INVALID_PARAM;
    }
    return SUCCESS;
}

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS