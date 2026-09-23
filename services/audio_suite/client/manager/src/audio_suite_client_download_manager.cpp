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
#define LOG_TAG "audio_suite_client_download_manager"
#endif

#include "audio_suite_client_download_manager.h"
#include "audio_suite_client_policy.h"
#include "audio_suite_log.h"
#include "audio_errors.h"
#include "ffrt_inner.h"
#include <memory>

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

static constexpr int32_t IPC_TIMEOUT_MS = 2500;

template<typename F>
static int32_t CallWithTimeout(F&& func, const char* methodName)
{
    auto future = ffrt::async(std::forward<F>(func));
    if (future.wait_for(std::chrono::milliseconds(IPC_TIMEOUT_MS)) == ffrt::future_status::timeout) {
        AUDIO_ERR_LOG("%{public}s IPC timeout after %{public}d ms", methodName, IPC_TIMEOUT_MS);
        return ERR_IPC;
    }
    return future.get();
}

AudioSuiteDownloadManagerImpl::AudioSuiteDownloadManagerImpl()
{
    AUDIO_INFO_LOG("AudioSuiteDownloadManagerImpl constructor");
}

AudioSuiteDownloadManagerImpl::~AudioSuiteDownloadManagerImpl()
{
    AUDIO_INFO_LOG("AudioSuiteDownloadManagerImpl destructor");
}

int32_t AudioSuiteDownloadManagerImpl::RegisterDownloadStatusCallback(
    std::shared_ptr<DownloadStatusCallback> callback)
{
    AUDIO_INFO_LOG("AudioSuiteDownloadManagerImpl::RegisterDownloadStatusCallback");
    return CallWithTimeout(
        [callback]() { return AudioSuiteClientPolicy::GetInstance().RegisterDownloadStatusCallback(callback); },
        "RegisterDownloadStatusCallback");
}

int32_t AudioSuiteDownloadManagerImpl::UnregisterDownloadStatusCallback()
{
    AUDIO_INFO_LOG("AudioSuiteDownloadManagerImpl::UnregisterDownloadStatusCallback");
    return CallWithTimeout([]() { return AudioSuiteClientPolicy::GetInstance().UnregisterDownloadStatusCallback(); },
                           "UnregisterDownloadStatusCallback");
}

int32_t AudioSuiteDownloadManagerImpl::GetInstallStatus(
    const std::vector<std::string>& featureNames, std::vector<bool>& installStatus)
{
    AUDIO_INFO_LOG("AudioSuiteDownloadManagerImpl::GetInstallStatus");
    auto installStatusPtr = std::make_shared<std::vector<bool>>();
    int32_t ret = CallWithTimeout(
        [featureNames, installStatusPtr]() {
            return AudioSuiteClientPolicy::GetInstance().GetInstallStatus(featureNames, *installStatusPtr);
        },
        "GetInstallStatus");
    if (ret == SUCCESS) {
        installStatus = *installStatusPtr;
    }
    return ret;
}

int32_t AudioSuiteDownloadManagerImpl::StartDownload(
    const std::vector<std::string>& featureNames, bool allowedCellularNet)
{
    AUDIO_INFO_LOG("AudioSuiteDownloadManagerImpl::StartDownload, featureNames size=%{public}zu, "
        "allowedCellularNet=%{public}d", featureNames.size(), allowedCellularNet);
    return CallWithTimeout(
        [featureNames, allowedCellularNet]() {
            return AudioSuiteClientPolicy::GetInstance().StartDownload(featureNames, allowedCellularNet);
        }, "StartDownload");
}

int32_t AudioSuiteDownloadManagerImpl::PauseDownload(const std::vector<std::string>& featureNames)
{
    AUDIO_INFO_LOG("AudioSuiteDownloadManagerImpl::PauseDownload, featureNames size=%{public}zu", featureNames.size());
    return CallWithTimeout(
        [featureNames]() { return AudioSuiteClientPolicy::GetInstance().PauseDownload(featureNames); },
        "PauseDownload");
}

int32_t AudioSuiteDownloadManagerImpl::CancelDownload(const std::vector<std::string>& featureNames)
{
    AUDIO_INFO_LOG("AudioSuiteDownloadManagerImpl::CancelDownload, featureNames size=%{public}zu", featureNames.size());
    return CallWithTimeout(
        [featureNames]() { return AudioSuiteClientPolicy::GetInstance().CancelDownload(featureNames); },
        "CancelDownload");
}

int32_t AudioSuiteDownloadManagerImpl::StartBackgroundDownload(const std::vector<std::string>& featureNames)
{
    AUDIO_INFO_LOG("AudioSuiteDownloadManagerImpl::StartBackgroundDownload, featureNames size=%{public}zu",
        featureNames.size());
    return CallWithTimeout(
        [featureNames]() { return AudioSuiteClientPolicy::GetInstance().StartBackgroundDownload(featureNames); },
        "StartBackgroundDownload");
}

int32_t AudioSuiteDownloadManagerImpl::GetDownloadStatus(
    const std::vector<std::string>& featureNames, std::vector<AudioSuiteDownloadStatusInfo>& status)
{
    AUDIO_INFO_LOG("AudioSuiteDownloadManagerImpl::GetDownloadStatus");
    auto statusPtr = std::make_shared<std::vector<AudioSuiteDownloadStatusInfo>>();
    int32_t ret = CallWithTimeout(
        [featureNames, statusPtr]() {
            return AudioSuiteClientPolicy::GetInstance().GetDownloadStatus(featureNames, *statusPtr);
        },
        "GetDownloadStatus");
    if (ret == SUCCESS) {
        status = *statusPtr;
    }
    return ret;
}

int32_t AudioSuiteDownloadManagerImpl::GetFeatureNewVersion(
    const std::vector<std::string>& featureNames, std::vector<AudioSuiteCloudRomVersionInfo>& versionInfos)
{
    AUDIO_INFO_LOG("AudioSuiteDownloadManagerImpl::GetFeatureNewVersion");
    auto versionInfosPtr = std::make_shared<std::vector<AudioSuiteCloudRomVersionInfo>>();
    int32_t ret = CallWithTimeout(
        [featureNames, versionInfosPtr]() {
            return AudioSuiteClientPolicy::GetInstance().GetFeatureNewVersion(featureNames, *versionInfosPtr);
        },
        "GetFeatureNewVersion");
    if (ret == SUCCESS) {
        versionInfos = *versionInfosPtr;
    }
    return ret;
}

int32_t AudioSuiteDownloadManagerImpl::UninstallCloudRom(const std::vector<std::string>& featureNames)
{
    AUDIO_INFO_LOG("AudioSuiteDownloadManagerImpl::UninstallCloudRom, featureNames size=%{public}zu",
        featureNames.size());
    return CallWithTimeout(
        [featureNames]() { return AudioSuiteClientPolicy::GetInstance().UninstallCloudRom(featureNames); },
        "UninstallCloudRom");
}

int32_t AudioSuiteDownloadManagerImpl::StartDownloadDirect(
    const std::vector<std::string>& featureNames, bool allowedCellularNet)
{
    return AudioSuiteClientPolicy::GetInstance().StartDownload(featureNames, allowedCellularNet);
}

int32_t AudioSuiteDownloadManagerImpl::PauseDownloadDirect(const std::vector<std::string>& featureNames)
{
    return AudioSuiteClientPolicy::GetInstance().PauseDownload(featureNames);
}

int32_t AudioSuiteDownloadManagerImpl::CancelDownloadDirect(const std::vector<std::string>& featureNames)
{
    return AudioSuiteClientPolicy::GetInstance().CancelDownload(featureNames);
}

int32_t AudioSuiteDownloadManagerImpl::StartBackgroundDownloadDirect(const std::vector<std::string>& featureNames)
{
    return AudioSuiteClientPolicy::GetInstance().StartBackgroundDownload(featureNames);
}

int32_t AudioSuiteDownloadManagerImpl::GetDownloadStatusDirect(
    const std::vector<std::string>& featureNames, std::vector<AudioSuiteDownloadStatusInfo>& status)
{
    return AudioSuiteClientPolicy::GetInstance().GetDownloadStatus(featureNames, status);
}

int32_t AudioSuiteDownloadManagerImpl::GetFeatureNewVersionDirect(
    const std::vector<std::string>& featureNames, std::vector<AudioSuiteCloudRomVersionInfo>& versionInfos)
{
    return AudioSuiteClientPolicy::GetInstance().GetFeatureNewVersion(featureNames, versionInfos);
}

int32_t AudioSuiteDownloadManagerImpl::UninstallCloudRomDirect(const std::vector<std::string>& featureNames)
{
    return AudioSuiteClientPolicy::GetInstance().UninstallCloudRom(featureNames);
}

int32_t AudioSuiteDownloadManagerImpl::RegisterDownloadInterfaceCallback(
    const std::shared_ptr<AudioSuiteRomDownloadCallback> &callback)
{
    return AudioSuiteClientPolicy::GetInstance().RegisterDownloadInterfaceCallback(callback);
}

int32_t AudioSuiteDownloadManagerImpl::UnregisterDownloadInterfaceCallback()
{
    AUDIO_INFO_LOG("AudioSuiteDownloadManagerImpl::UnregisterDownloadInterfaceCallback");
    return AudioSuiteClientPolicy::GetInstance().UnregisterDownloadInterfaceCallback();
}

void AudioSuiteDownloadManagerImpl::OnDownloadStatusChanged(const std::vector<AudioSuiteDownloadStatusInfo>& status)
{
    AUDIO_INFO_LOG("AudioSuiteDownloadManagerImpl::OnDownloadStatusChanged");
    AudioSuiteClientPolicy::GetInstance().OnDownloadStatusChanged(status);
}

AudioSuiteDownloadManager& AudioSuiteDownloadManager::GetInstance()
{
    static AudioSuiteDownloadManagerImpl downloadManagerImpl;
    return downloadManagerImpl;
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS