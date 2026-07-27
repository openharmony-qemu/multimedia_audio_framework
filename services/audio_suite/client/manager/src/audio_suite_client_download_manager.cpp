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
    AUDIO_INFO_LOG("AudioSuiteDownloadManagerImpl::RegisterDownloadStatusCallback");
    return AudioSuiteClientPolicy::GetInstance().RegisterDownloadStatusCallback(callback);
}

int32_t AudioSuiteDownloadManagerImpl::UnregisterDownloadStatusCallback()
{
    AUDIO_INFO_LOG("AudioSuiteDownloadManagerImpl::UnregisterDownloadStatusCallback");
    return AudioSuiteClientPolicy::GetInstance().UnregisterDownloadStatusCallback();
}

int32_t AudioSuiteDownloadManagerImpl::StartDownload(const std::string& featureName)
{
    AUDIO_INFO_LOG("AudioSuiteDownloadManagerImpl::StartDownload, featureName=%{public}s", featureName.c_str());
    return AudioSuiteClientPolicy::GetInstance().StartDownload(featureName);
}

int32_t AudioSuiteDownloadManagerImpl::PauseDownload(const std::string& featureName)
{
    AUDIO_INFO_LOG("AudioSuiteDownloadManagerImpl::PauseDownload, featureName=%{public}s", featureName.c_str());
    return AudioSuiteClientPolicy::GetInstance().PauseDownload(featureName);
}

int32_t AudioSuiteDownloadManagerImpl::CancelDownload(const std::string& featureName)
{
    AUDIO_INFO_LOG("AudioSuiteDownloadManagerImpl::CancelDownload, featureName=%{public}s", featureName.c_str());
    return AudioSuiteClientPolicy::GetInstance().CancelDownload(featureName);
}

int32_t AudioSuiteDownloadManagerImpl::StartBackgroundDownload(const std::string& featureName)
{
    AUDIO_INFO_LOG("AudioSuiteDownloadManagerImpl::StartBackgroundDownload, featureName=%{public}s",
        featureName.c_str());
    return AudioSuiteClientPolicy::GetInstance().StartBackgroundDownload(featureName);
}

int32_t AudioSuiteDownloadManagerImpl::GetDownloadStatus(
    const std::string& featureName, AudioSuiteDownloadStatusInfo& status)
{
    AUDIO_INFO_LOG("AudioSuiteDownloadManagerImpl::GetDownloadStatus");
    return AudioSuiteClientPolicy::GetInstance().GetDownloadStatus(featureName, status);
}

int32_t AudioSuiteDownloadManagerImpl::UninstallCloudRom(const std::string& featureName)
{
    AUDIO_INFO_LOG("AudioSuiteDownloadManagerImpl::UninstallCloudRom, featureName=%{public}s", featureName.c_str());
    return AudioSuiteClientPolicy::GetInstance().UninstallCloudRom(featureName);
}

int32_t AudioSuiteDownloadManagerImpl::RegisterDownloadInterfaceCallback(
    const std::shared_ptr<AudioSuiteRomDownloadCallback> &callback)
{
    AUDIO_INFO_LOG("AudioSuiteDownloadManagerImpl::RegisterDownloadInterfaceCallback");
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