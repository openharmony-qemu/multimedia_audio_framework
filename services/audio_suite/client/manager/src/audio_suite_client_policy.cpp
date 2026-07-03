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
#define LOG_TAG "AudioSuiteClientPolicy"
#endif

#include "audio_suite_client_policy.h"
#include "audio_suite_log.h"
#include "audio_errors.h"
#include "audio_suite_download_manager.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

AudioSuiteClientPolicy::AudioSuiteClientPolicy() : connected_(false), needsReconnect_(false)
{
    AUDIO_INFO_LOG("AudioSuiteClientPolicy constructor");
}

AudioSuiteClientPolicy::~AudioSuiteClientPolicy()
{
    AUDIO_INFO_LOG("AudioSuiteClientPolicy destructor");
    DisconnectService();

    downloadCallback_.reset();
    downloadCallbackStub_ = nullptr;
}

AudioSuiteClientPolicy& AudioSuiteClientPolicy::GetInstance()
{
    static AudioSuiteClientPolicy instance;
    return instance;
}

int32_t AudioSuiteClientPolicy::Init()
{
    AUDIO_INFO_LOG("AudioSuiteClientPolicy::Init");

    std::lock_guard<std::mutex> lock(mutex_);
    return EnsureConnected() ? SUCCESS : ERR_NOT_STARTED;
}

void AudioSuiteClientPolicy::DeInit()
{
    AUDIO_INFO_LOG("AudioSuiteClientPolicy::DeInit");
    DisconnectService();
}

bool AudioSuiteClientPolicy::ConnectService()
{
    AUDIO_INFO_LOG("AudioSuiteClientPolicy::ConnectService");

    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        AUDIO_ERR_LOG("GetSystemAbilityManager failed");
        return false;
    }

    constexpr int32_t LOAD_TIMEOUT_MS = 5000;
    auto object = samgr->LoadSystemAbility(SA_ID, LOAD_TIMEOUT_MS);
    if (object == nullptr) {
        AUDIO_ERR_LOG("LoadSystemAbility(%{public}d) failed, SA not registered or cannot start", SA_ID);
        return false;
    }

    proxy_ = iface_cast<IAudioSuiteSaService>(object);
    if (proxy_ == nullptr) {
        AUDIO_ERR_LOG("Create proxy failed");
        return false;
    }

    connected_ = true;
    needsReconnect_ = false;
    AUDIO_INFO_LOG("ConnectService success");
    return true;
}

void AudioSuiteClientPolicy::DisconnectService()
{
    AUDIO_INFO_LOG("AudioSuiteClientPolicy::DisconnectService");
    std::lock_guard<std::mutex> lock(mutex_);
    proxy_ = nullptr;
    connected_ = false;
    needsReconnect_ = false;
}

bool AudioSuiteClientPolicy::EnsureConnected()
{
    if (needsReconnect_ || !connected_) {
        AUDIO_INFO_LOG("AudioSuiteClientPolicy: connecting (reconnect=%{public}d)", needsReconnect_);
        if (!ConnectService()) {
            AUDIO_ERR_LOG("Connect failed");
            return false;
        }
        needsReconnect_ = false;
    }
    return connected_ && proxy_ != nullptr;
}

int32_t AudioSuiteClientPolicy::RegisterDownloadCallback(std::shared_ptr<IDownloadCallbackListener> callback)
{
    AUDIO_INFO_LOG("AudioSuiteClientPolicy::RegisterDownloadCallback");

    std::lock_guard<std::mutex> lock(mutex_);

    if (!EnsureConnected()) {
        AUDIO_ERR_LOG("Service not connected");
        return ERR_NOT_STARTED;
    }

    downloadCallback_ = callback;

    if (downloadCallbackStub_ == nullptr) {
        downloadCallbackStub_ = new DownloadCallbackStubImpl();
    }

    ErrCode result = SUCCESS;
    proxy_->RegisterDownloadCallback(downloadCallbackStub_->AsObject(), result);
    AUDIO_INFO_LOG("RegisterDownloadCallback IPC result: %{public}d", result);

    if (result != SUCCESS) {
        AUDIO_ERR_LOG("RegisterDownloadCallback IPC failed: %{public}d", result);
        return result;
    }

    if (downloadCallback_ != nullptr) {
        downloadCallbackStub_->SetCallback([this](const DownloadStatus& status) {
            if (downloadCallback_) {
                downloadCallback_->OnDownloadStatusChanged(status);
            }
        });
    } else {
        downloadCallbackStub_->ClearCallback();
    }
    return SUCCESS;
}

int32_t AudioSuiteClientPolicy::UnregisterDownloadCallback()
{
    AUDIO_INFO_LOG("AudioSuiteClientPolicy::UnregisterDownloadCallback");

    std::lock_guard<std::mutex> lock(mutex_);

    if (!EnsureConnected()) {
        AUDIO_ERR_LOG("Service not connected");
        return ERR_NOT_STARTED;
    }

    downloadCallback_ = nullptr;
    if (downloadCallbackStub_ != nullptr) {
        downloadCallbackStub_->ClearCallback();
    }

    ErrCode result = SUCCESS;
    proxy_->UnregisterDownloadCallback(result);
    AUDIO_INFO_LOG("UnregisterDownloadCallback IPC result: %{public}d", result);

    if (result != SUCCESS) {
        AUDIO_ERR_LOG("UnregisterDownloadCallback IPC failed: %{public}d", result);
        return result;
    }

    return SUCCESS;
}

int32_t AudioSuiteClientPolicy::StartDownload(const std::string& version)
{
    AUDIO_INFO_LOG("AudioSuiteClientPolicy::StartDownload, version=%{public}s", version.c_str());

    std::lock_guard<std::mutex> lock(mutex_);

    if (!EnsureConnected()) {
        AUDIO_ERR_LOG("Service not connected");
        return ERR_NOT_STARTED;
    }

    ErrCode result = SUCCESS;
    proxy_->StartDownload(version, result);
    return result;
}

int32_t AudioSuiteClientPolicy::CancelDownload()
{
    AUDIO_INFO_LOG("AudioSuiteClientPolicy::CancelDownload");

    std::lock_guard<std::mutex> lock(mutex_);

    if (!EnsureConnected()) {
        AUDIO_ERR_LOG("Service not connected");
        return ERR_NOT_STARTED;
    }

    ErrCode result = SUCCESS;
    proxy_->CancelDownload(result);
    return result;
}

int32_t AudioSuiteClientPolicy::StartBackgroundDownload(const std::string& version)
{
    AUDIO_INFO_LOG("AudioSuiteClientPolicy::StartBackgroundDownload, version=%{public}s",
        version.c_str());

    std::lock_guard<std::mutex> lock(mutex_);

    if (!EnsureConnected()) {
        AUDIO_ERR_LOG("Service not connected");
        return ERR_NOT_STARTED;
    }

    ErrCode result = SUCCESS;
    proxy_->StartBackgroundDownload(version, result);
    return result;
}

int32_t AudioSuiteClientPolicy::GetDownloadStatus(DownloadStatus& status, const std::string& version)
{
    AUDIO_INFO_LOG("AudioSuiteClientPolicy::GetDownloadStatus");

    std::lock_guard<std::mutex> lock(mutex_);

    if (!EnsureConnected()) {
        AUDIO_ERR_LOG("Service not connected");
        return ERR_NOT_STARTED;
    }

    ErrCode result = SUCCESS;
    proxy_->GetDownloadStatus(status, version, result);
    return result;
}

int32_t AudioSuiteClientPolicy::UninstallCloudRom(const std::string& version)
{
    AUDIO_INFO_LOG("AudioSuiteClientPolicy::UninstallCloudRom, version=%{public}s", version.c_str());

    std::lock_guard<std::mutex> lock(mutex_);

    if (!EnsureConnected()) {
        AUDIO_ERR_LOG("Service not connected");
        return ERR_NOT_STARTED;
    }

    ErrCode result = SUCCESS;
    proxy_->UninstallCloudRom(version, result);
    return result;
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS