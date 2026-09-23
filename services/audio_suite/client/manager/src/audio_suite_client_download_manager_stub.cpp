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
#define LOG_TAG "audio_suite_client_download_manager_stub"
#endif

#include "audio_suite_client_download_manager.h"
#include "audio_suite_log.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

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
    AUDIO_INFO_LOG("RegisterDownloadStatusCallback, device not support");
    return ERROR;
}

int32_t AudioSuiteDownloadManagerImpl::UnregisterDownloadStatusCallback()
{
    AUDIO_INFO_LOG("UnregisterDownloadStatusCallback, device not support");
    return ERROR;
}

int32_t AudioSuiteDownloadManagerImpl::GetInstallStatus(
    const std::vector<std::string>& featureNames, std::vector<bool>& installStatus)
{
    AUDIO_INFO_LOG("GetInstallStatus, device not support, featureNames size:%{public}zu",
        featureNames.size());
    return ERROR;
}

int32_t AudioSuiteDownloadManagerImpl::StartDownload(
    const std::vector<std::string>& featureNames, bool allowedCellularNet)
{
    AUDIO_INFO_LOG("StartDownload, device not support, featureNames size:%{public}zu, allowedCellularNet:%{public}d",
        featureNames.size(), allowedCellularNet);
    return ERROR;
}

int32_t AudioSuiteDownloadManagerImpl::PauseDownload(const std::vector<std::string>& featureNames)
{
    AUDIO_INFO_LOG("PauseDownload, device not support, featureNames size:%{public}zu", featureNames.size());
    return ERROR;
}

int32_t AudioSuiteDownloadManagerImpl::CancelDownload(const std::vector<std::string>& featureNames)
{
    AUDIO_INFO_LOG("CancelDownload, device not support, featureNames size:%{public}zu", featureNames.size());
    return ERROR;
}

int32_t AudioSuiteDownloadManagerImpl::StartBackgroundDownload(const std::vector<std::string>& featureNames)
{
    AUDIO_INFO_LOG("StartBackgroundDownload, device not support, featureNames size:%{public}zu", featureNames.size());
    return ERROR;
}

int32_t AudioSuiteDownloadManagerImpl::GetDownloadStatus(
    const std::vector<std::string>& featureNames, std::vector<AudioSuiteDownloadStatusInfo>& status)
{
    AUDIO_INFO_LOG("GetDownloadStatus, device not support, featureNames size:%{public}zu", featureNames.size());
    return ERROR;
}

int32_t AudioSuiteDownloadManagerImpl::GetFeatureNewVersion(
    const std::vector<std::string>& featureNames, std::vector<AudioSuiteCloudRomVersionInfo>& versionInfos)
{
    AUDIO_INFO_LOG("GetFeatureNewVersion, device not support, featureNames size:%{public}zu", featureNames.size());
    return ERROR;
}

int32_t AudioSuiteDownloadManagerImpl::UninstallCloudRom(const std::vector<std::string>& featureNames)
{
    AUDIO_INFO_LOG("UninstallCloudRom, device not support, featureNames size:%{public}zu", featureNames.size());
    return ERROR;
}

int32_t AudioSuiteDownloadManagerImpl::StartDownloadDirect(
    const std::vector<std::string>& featureNames, bool allowedCellularNet)
{
    AUDIO_INFO_LOG("StartDownloadDirect, device not support, featureNames size:%{public}zu, "
        "allowedCellularNet:%{public}d", featureNames.size(), allowedCellularNet);
    return ERROR;
}

int32_t AudioSuiteDownloadManagerImpl::PauseDownloadDirect(const std::vector<std::string>& featureNames)
{
    AUDIO_INFO_LOG("PauseDownloadDirect, device not support, featureNames size:%{public}zu", featureNames.size());
    return ERROR;
}

int32_t AudioSuiteDownloadManagerImpl::CancelDownloadDirect(const std::vector<std::string>& featureNames)
{
    AUDIO_INFO_LOG("CancelDownloadDirect, device not support, featureNames size:%{public}zu", featureNames.size());
    return ERROR;
}

int32_t AudioSuiteDownloadManagerImpl::StartBackgroundDownloadDirect(const std::vector<std::string>& featureNames)
{
    AUDIO_INFO_LOG("StartBackgroundDownloadDirect, device not support, featureNames size:%{public}zu",
        featureNames.size());
    return ERROR;
}

int32_t AudioSuiteDownloadManagerImpl::GetDownloadStatusDirect(
    const std::vector<std::string>& featureNames, std::vector<AudioSuiteDownloadStatusInfo>& status)
{
    AUDIO_INFO_LOG("GetDownloadStatusDirect, device not support, featureNames size:%{public}zu", featureNames.size());
    return ERROR;
}

int32_t AudioSuiteDownloadManagerImpl::GetFeatureNewVersionDirect(
    const std::vector<std::string>& featureNames, std::vector<AudioSuiteCloudRomVersionInfo>& versionInfos)
{
    AUDIO_INFO_LOG("GetFeatureNewVersionDirect, device not support, featureNames size:%{public}zu",
        featureNames.size());
    return ERROR;
}

int32_t AudioSuiteDownloadManagerImpl::UninstallCloudRomDirect(const std::vector<std::string>& featureNames)
{
    AUDIO_INFO_LOG("UninstallCloudRomDirect, device not support, featureNames size:%{public}zu", featureNames.size());
    return ERROR;
}

int32_t AudioSuiteDownloadManagerImpl::RegisterDownloadInterfaceCallback(
    const std::shared_ptr<AudioSuiteRomDownloadCallback> &callback)
{
    AUDIO_INFO_LOG("RegisterDownloadInterfaceCallback, device not support");
    return ERROR;
}

int32_t AudioSuiteDownloadManagerImpl::UnregisterDownloadInterfaceCallback()
{
    AUDIO_INFO_LOG("UnregisterDownloadInterfaceCallback, device not support");
    return ERROR;
}

void AudioSuiteDownloadManagerImpl::OnDownloadStatusChanged(const std::vector<AudioSuiteDownloadStatusInfo>& status)
{
    AUDIO_INFO_LOG("OnDownloadStatusChanged, device not support");
}

AudioSuiteDownloadManager& AudioSuiteDownloadManager::GetInstance()
{
    static AudioSuiteDownloadManagerImpl downloadManagerImpl;
    return downloadManagerImpl;
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS
