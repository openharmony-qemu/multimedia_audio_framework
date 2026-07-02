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

int32_t AudioSuiteDownloadManagerImpl::Init()
{
    AUDIO_INFO_LOG("AudioSuiteDownloadManagerImpl::Init");
    return AudioSuiteClientPolicy::GetInstance().Init();
}

int32_t AudioSuiteDownloadManagerImpl::RegisterDownloadCallback(
    std::shared_ptr<IDownloadCallbackListener> callback)
{
    AUDIO_INFO_LOG("AudioSuiteDownloadManagerImpl::RegisterDownloadCallback");
    return AudioSuiteClientPolicy::GetInstance().RegisterDownloadCallback(callback);
}

int32_t AudioSuiteDownloadManagerImpl::UnregisterDownloadCallback()
{
    AUDIO_INFO_LOG("AudioSuiteDownloadManagerImpl::UnregisterDownloadCallback");
    return AudioSuiteClientPolicy::GetInstance().UnregisterDownloadCallback();
}

int32_t AudioSuiteDownloadManagerImpl::StartDownload(const std::string& version)
{
    AUDIO_INFO_LOG("AudioSuiteDownloadManagerImpl::StartDownload, version=%{public}s", version.c_str());
    return AudioSuiteClientPolicy::GetInstance().StartDownload(version);
}

int32_t AudioSuiteDownloadManagerImpl::CancelDownload()
{
    AUDIO_INFO_LOG("AudioSuiteDownloadManagerImpl::CancelDownload");
    return AudioSuiteClientPolicy::GetInstance().CancelDownload();
}

int32_t AudioSuiteDownloadManagerImpl::StartBackgroundDownload(const std::string& version)
{
    AUDIO_INFO_LOG("AudioSuiteDownloadManagerImpl::StartBackgroundDownload, version=%{public}s",
        version.c_str());
    return AudioSuiteClientPolicy::GetInstance().StartBackgroundDownload(version);
}

int32_t AudioSuiteDownloadManagerImpl::GetDownloadStatus(DownloadStatus& status, const std::string& version)
{
    AUDIO_INFO_LOG("AudioSuiteDownloadManagerImpl::GetDownloadStatus");
    return AudioSuiteClientPolicy::GetInstance().GetDownloadStatus(status, version);
}

int32_t AudioSuiteDownloadManagerImpl::UninstallCloudRom(const std::string& version)
{
    AUDIO_INFO_LOG("AudioSuiteDownloadManagerImpl::UninstallCloudRom, version=%{public}s", version.c_str());
    return AudioSuiteClientPolicy::GetInstance().UninstallCloudRom(version);
}

static std::unique_ptr<AudioSuiteDownloadManagerImpl> g_managerInstance;
static bool g_managerInitialized = false;

AudioSuiteDownloadManager& AudioSuiteDownloadManager::GetInstance()
{
    AUDIO_INFO_LOG("AudioSuiteDownloadManager::GetInstance");
    if (!g_managerInitialized || g_managerInstance == nullptr) {
        g_managerInstance = std::make_unique<AudioSuiteDownloadManagerImpl>();
        if (g_managerInstance->Init() == SUCCESS) {
            g_managerInitialized = true;
        }
    }
    return *g_managerInstance;
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS