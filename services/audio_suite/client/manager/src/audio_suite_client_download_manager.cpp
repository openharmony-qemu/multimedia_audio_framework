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

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

static constexpr int32_t IPC_TIMEOUT_MS = 3000;

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
        [&callback]() { return AudioSuiteClientPolicy::GetInstance().RegisterDownloadStatusCallback(callback); },
        "RegisterDownloadStatusCallback");
}

int32_t AudioSuiteDownloadManagerImpl::UnregisterDownloadStatusCallback()
{
    AUDIO_INFO_LOG("AudioSuiteDownloadManagerImpl::UnregisterDownloadStatusCallback");
    return CallWithTimeout([]() { return AudioSuiteClientPolicy::GetInstance().UnregisterDownloadStatusCallback(); },
                           "UnregisterDownloadStatusCallback");
}

int32_t AudioSuiteDownloadManagerImpl::StartDownload(const std::string& featureName)
{
    AUDIO_INFO_LOG("AudioSuiteDownloadManagerImpl::StartDownload, featureName=%{public}s", featureName.c_str());
    return CallWithTimeout(
        [&featureName]() { return AudioSuiteClientPolicy::GetInstance().StartDownload(featureName); }, "StartDownload");
}

int32_t AudioSuiteDownloadManagerImpl::PauseDownload(const std::string& featureName)
{
    AUDIO_INFO_LOG("AudioSuiteDownloadManagerImpl::PauseDownload, featureName=%{public}s", featureName.c_str());
    return CallWithTimeout(
        [&featureName]() { return AudioSuiteClientPolicy::GetInstance().PauseDownload(featureName); }, "PauseDownload");
}

int32_t AudioSuiteDownloadManagerImpl::CancelDownload(const std::string& featureName)
{
    AUDIO_INFO_LOG("AudioSuiteDownloadManagerImpl::CancelDownload, featureName=%{public}s", featureName.c_str());
    return CallWithTimeout(
        [&featureName]() { return AudioSuiteClientPolicy::GetInstance().CancelDownload(featureName); },
        "CancelDownload");
}

int32_t AudioSuiteDownloadManagerImpl::StartBackgroundDownload(const std::string& featureName)
{
    AUDIO_INFO_LOG("AudioSuiteDownloadManagerImpl::StartBackgroundDownload, featureName=%{public}s",
        featureName.c_str());
    return CallWithTimeout(
        [&featureName]() { return AudioSuiteClientPolicy::GetInstance().StartBackgroundDownload(featureName); },
        "StartBackgroundDownload");
}

int32_t AudioSuiteDownloadManagerImpl::GetDownloadStatus(
    const std::string& featureName, AudioSuiteDownloadStatusInfo& status)
{
    AUDIO_INFO_LOG("AudioSuiteDownloadManagerImpl::GetDownloadStatus");
    return CallWithTimeout(
        [&featureName, &status]() {
            return AudioSuiteClientPolicy::GetInstance().GetDownloadStatus(featureName, status);
        },
        "GetDownloadStatus");
}

int32_t AudioSuiteDownloadManagerImpl::UninstallCloudRom(const std::string& featureName)
{
    AUDIO_INFO_LOG("AudioSuiteDownloadManagerImpl::UninstallCloudRom, featureName=%{public}s", featureName.c_str());
    return CallWithTimeout(
        [&featureName]() { return AudioSuiteClientPolicy::GetInstance().UninstallCloudRom(featureName); },
        "UninstallCloudRom");
}

int32_t AudioSuiteDownloadManagerImpl::RegisterDownloadInterfaceCallback(
    const std::shared_ptr<AudioSuiteRomDownloadCallback> &callback)
{
    AUDIO_INFO_LOG("AudioSuiteDownloadManagerImpl::RegisterDownloadInterfaceCallback");
    return CallWithTimeout(
        [&callback]() { return AudioSuiteClientPolicy::GetInstance().RegisterDownloadInterfaceCallback(callback); },
        "RegisterDownloadInterfaceCallback");
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