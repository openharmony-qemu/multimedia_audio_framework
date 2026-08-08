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

#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "audio_suite_client_policy.h"
#include "audio_suite_log.h"
#include "audio_errors.h"
#include "audio_suite_download_callback_stub_impl.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

static constexpr int32_t SA_ID = 3015;
static constexpr int32_t LOAD_TIMEOUT_MS = 5000;

AudioSuiteClientPolicy::AudioSuiteClientPolicy()
{
    AUDIO_INFO_LOG("AudioSuiteClientPolicy constructor");
}

AudioSuiteClientPolicy::~AudioSuiteClientPolicy()
{
    AUDIO_INFO_LOG("AudioSuiteClientPolicy destructor");
    UnregisterDownloadStatusCallback();
    UnregisterDownloadInterfaceCallback();

    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (proxy_ != nullptr && deathRecipient_ != nullptr) {
            proxy_->AsObject()->RemoveDeathRecipient(deathRecipient_);
        }
        deathRecipient_ = nullptr;
    }

    DisconnectService();
    std::lock_guard<std::mutex> lock(mutex_);
    if (statusCallbackStub_ != nullptr) {
        statusCallbackStub_->ClearCallback();
        statusCallbackStub_ = nullptr;
    }

    if (downloadCallbackStub_ != nullptr) {
        downloadCallbackStub_->ClearCallback();
        downloadCallbackStub_ = nullptr;
    }
}

AudioSuiteClientPolicy& AudioSuiteClientPolicy::GetInstance()
{
    static AudioSuiteClientPolicy instance;
    return instance;
}

void AudioSuiteClientPolicy::DisconnectService()
{
    std::lock_guard<std::mutex> lock(mutex_);
    proxy_ = nullptr;
    connected_ = false;
    AUDIO_INFO_LOG("AudioSuiteClientPolicy::DisconnectService");
}

bool AudioSuiteClientPolicy::ConnectService()
{
    if (connected_ && proxy_ != nullptr) {
        return true;
    }

    AUDIO_INFO_LOG("AudioSuiteClientPolicy::ConnectService");
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_AND_RETURN_RET_LOG(samgr != nullptr, false, "GetSystemAbilityManager failed");

    auto object = samgr->LoadSystemAbility(SA_ID, LOAD_TIMEOUT_MS);
    CHECK_AND_RETURN_RET_LOG(object != nullptr, false, "LoadSystemAbility audio suite sa failed.");

    proxy_ = iface_cast<IAudioSuiteService>(object);
    CHECK_AND_RETURN_RET_LOG(proxy_ != nullptr, false, "iface_cast audio suite sa proxy failed.");

    connected_ = true;
    AUDIO_INFO_LOG("ConnectService success");

    sptr<AudioSuiteServerDeathRecipient> deathRecipient = new(std::nothrow) AudioSuiteServerDeathRecipient();
    if (deathRecipient != nullptr && object->AddDeathRecipient(deathRecipient)) {
        deathRecipient_ = deathRecipient;
        AUDIO_INFO_LOG("AddDeathRecipient deathRecipient success");
    }
    return true;
}

int32_t AudioSuiteClientPolicy::RegisterDownloadStatusCallback(std::shared_ptr<DownloadStatusCallback> callback)
{
    AUDIO_INFO_LOG("AudioSuiteClientPolicy::RegisterDownloadStatusCallback");
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(ConnectService(), ERR_NOT_STARTED, "audio suite service not connected");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    if (statusCallbackStub_ != nullptr) {
        statusCallbackStub_->ClearCallback();
        statusCallbackStub_ = nullptr;
    }
    statusCallbackStub_ = new(std::nothrow) AudioSuiteStatusCallbackStubImpl();
    CHECK_AND_RETURN_RET_LOG(statusCallbackStub_ != nullptr, ERROR, "statusCallbackStub_ is nullptr");

    int32_t result = SUCCESS;
    ErrCode ret = proxy_->RegisterDownloadStatusCallback(statusCallbackStub_->AsObject(), result);
    if (ret != SUCCESS || result != SUCCESS) {
        statusCallbackStub_ = nullptr;
    }
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "RegisterDownloadStatusCallback IPC ret: %{public}d", ret);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, result, "RegisterDownloadStatusCallback result: %{public}d", result);

    statusCallbackStub_->SetCallback(callback);
    if (statusListener_ == nullptr) {
        statusListener_ = sptr<AudioSuiteSystemAbilityListener>(new (std::nothrow) AudioSuiteSystemAbilityListener());
        CHECK_AND_RETURN_RET_LOG(
            statusListener_ != nullptr, SUCCESS, "Failed to create the AudioSuiteSystemAbilityListener variable.");
        sptr<ISystemAbilityManager> samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        CHECK_AND_RETURN_RET_LOG(
            samgrProxy != nullptr, SUCCESS, "SubscribeSystemAbility GetSystemAbilityManager failed");
        result = samgrProxy->SubscribeSystemAbility(SA_ID, statusListener_);
        CHECK_AND_RETURN_RET_LOG(result == ERR_OK, SUCCESS, "SubcribeSystemAbility result is %{public}d!", result);
    }
    return SUCCESS;
}

int32_t AudioSuiteClientPolicy::UnregisterDownloadStatusCallback()
{
    AUDIO_INFO_LOG("AudioSuiteClientPolicy::UnregisterDownloadStatusCallback");
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(ConnectService(), ERR_NOT_STARTED, "audio suite service not connected");
    CHECK_AND_RETURN_RET_LOG(statusCallbackStub_ != nullptr, SUCCESS, "not RegisterDownloadStatusCallback");

    int32_t result = SUCCESS;
    ErrCode ret = proxy_->UnregisterDownloadStatusCallback(result);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "UnregisterDownloadStatusCallback IPC ret: %{public}d", ret);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, result, "UnregisterDownloadStatusCallback result: %{public}d", result);

    if (statusCallbackStub_ != nullptr) {
        statusCallbackStub_->ClearCallback();
        statusCallbackStub_ = nullptr;
    }

    if (statusListener_ != nullptr) {
        sptr<ISystemAbilityManager> samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (samgrProxy != nullptr) {
            samgrProxy->UnSubscribeSystemAbility(SA_ID, statusListener_);
        }
        statusListener_ = nullptr;
    }

    return SUCCESS;
}

int32_t AudioSuiteClientPolicy::StartDownload(const std::string& featureName)
{
    AUDIO_INFO_LOG("AudioSuiteClientPolicy::StartDownload, featureName=%{public}s", featureName.c_str());

    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(ConnectService(), ERR_NOT_STARTED, "audio suite service not connected");

    int32_t result = SUCCESS;
    ErrCode ret = proxy_->StartDownload(featureName, result);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "StartDownload IPC ret: %{public}d", ret);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, result, "StartDownload result: %{public}d", result);
    return SUCCESS;
}

int32_t AudioSuiteClientPolicy::PauseDownload(const std::string& featureName)
{
    AUDIO_INFO_LOG("AudioSuiteClientPolicy::PauseDownload, featureName=%{public}s", featureName.c_str());

    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(ConnectService(), ERR_NOT_STARTED, "audio suite service not connected");

    int32_t result = SUCCESS;
    ErrCode ret = proxy_->PauseDownload(featureName, result);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "PauseDownload IPC ret: %{public}d", ret);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, result, "PauseDownload result: %{public}d", result);
    return SUCCESS;
}

int32_t AudioSuiteClientPolicy::CancelDownload(const std::string& featureName)
{
    AUDIO_INFO_LOG("AudioSuiteClientPolicy::CancelDownload, featureName=%{public}s", featureName.c_str());

    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(ConnectService(), ERR_NOT_STARTED, "audio suite service not connected");

    int32_t result = SUCCESS;
    ErrCode ret = proxy_->CancelDownload(featureName, result);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "CancelDownload IPC ret: %{public}d", ret);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, result, "CancelDownload result: %{public}d", result);
    return SUCCESS;
}

int32_t AudioSuiteClientPolicy::StartBackgroundDownload(const std::string& featureName)
{
    AUDIO_INFO_LOG("AudioSuiteClientPolicy::StartBackgroundDownload, featureName=%{public}s", featureName.c_str());

    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(ConnectService(), ERR_NOT_STARTED, "audio suite service not connected");

    int32_t result = SUCCESS;
    ErrCode ret = proxy_->StartBackgroundDownload(featureName, result);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "StartBackgroundDownload IPC ret: %{public}d", ret);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, result, "StartBackgroundDownload result: %{public}d", result);
    return SUCCESS;
}

int32_t AudioSuiteClientPolicy::GetDownloadStatus(const std::string& featureName, AudioSuiteDownloadStatusInfo& status)
{
    AUDIO_INFO_LOG("AudioSuiteClientPolicy::GetDownloadStatus, featureName=%{public}s", featureName.c_str());

    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(ConnectService(), ERR_NOT_STARTED, "audio suite service not connected");

    int32_t result = SUCCESS;
    ErrCode ret = proxy_->GetDownloadStatus(featureName, status, result);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetDownloadStatus IPC ret: %{public}d", ret);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, result, "GetDownloadStatus result: %{public}d", result);
    return SUCCESS;
}

int32_t AudioSuiteClientPolicy::UninstallCloudRom(const std::string& featureName)
{
    AUDIO_INFO_LOG("AudioSuiteClientPolicy::UninstallCloudRom, featureName=%{public}s", featureName.c_str());

    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(ConnectService(), ERR_NOT_STARTED, "audio suite service not connected");

    int32_t result = SUCCESS;
    ErrCode ret = proxy_->UninstallCloudRom(featureName, result);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "UninstallCloudRom IPC ret: %{public}d", ret);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, result, "UninstallCloudRom result: %{public}d", result);
    return SUCCESS;
}

int32_t AudioSuiteClientPolicy::RegisterDownloadInterfaceCallback(
    const std::shared_ptr<AudioSuiteRomDownloadCallback> &callback)
{
    AUDIO_INFO_LOG("AudioSuiteClientPolicy::RegisterDownloadInterfaceCallback");

    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(ConnectService(), ERR_NOT_STARTED, "audio suite service not connected");
    if (downloadCallbackStub_ != nullptr) {
        downloadCallbackStub_->ClearCallback();
        downloadCallbackStub_ = nullptr;
    }

    downloadCallbackStub_ = new(std::nothrow) AudioSuiteDownloadCallbackStubImpl();
    CHECK_AND_RETURN_RET_LOG(downloadCallbackStub_ != nullptr, ERROR, "downloadCallbackStub_ is nullptr");

    downloadCallbackStub_->SetAudioSuiteRomDownloadCallback(callback);
    int32_t result = SUCCESS;
    ErrCode ret = proxy_->RegisterDownloadInterfaceCallback(downloadCallbackStub_->AsObject(), result);
    if (ret != SUCCESS || result != SUCCESS) {
        downloadCallbackStub_ = nullptr;
    }
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "UninstallCloudRom IPC ret: %{public}d", ret);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, result, "UninstallCloudRom result: %{public}d", result);
    return SUCCESS;
}

int32_t AudioSuiteClientPolicy::UnregisterDownloadInterfaceCallback()
{
    AUDIO_INFO_LOG("AudioSuiteClientPolicy::UnregisterDownloadInterfaceCallback");
    std::lock_guard<std::mutex> lock(mutex_);
    if (downloadCallbackStub_ != nullptr) {
        downloadCallbackStub_->ClearCallback();
        downloadCallbackStub_ = nullptr;
    }
    return SUCCESS;
}

void AudioSuiteClientPolicy::OnDownloadStatusChanged(const std::vector<AudioSuiteDownloadStatusInfo>& status)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_LOG(ConnectService(), "audio suite service not connected");

    proxy_->OnDownloadStatusChanged(status);
}

void AudioSuiteClientPolicy::AudioSuiteServerDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    AudioSuiteClientPolicy::GetInstance().DisconnectService();
    AUDIO_INFO_LOG("AudioSuiteServerDeathRecipient::OnRemoteDied DisconnectService");
}

void AudioSuiteClientPolicy::AudioSuiteServerRecover(int32_t &systemAbilityId)
{
    CHECK_AND_RETURN_LOG(systemAbilityId == SA_ID, "Restart failed, invalid SA ID");
    AUDIO_INFO_LOG("OnAddSystemAbility, systemAbilityId=%{public}d", systemAbilityId);

    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_LOG(statusCallbackStub_ != nullptr, "OnAddSystemAbility: no callback to re-register");
    CHECK_AND_RETURN_LOG(ConnectService(), "OnAddSystemAbility: ConnectService failed");

    int32_t result = SUCCESS;
    ErrCode ret = proxy_->RegisterDownloadStatusCallback(statusCallbackStub_->AsObject(), result);
    CHECK_AND_RETURN_LOG(ret == SUCCESS && result == SUCCESS,
                         "OnAddSystemAbility: re-register status callback failed, ret=%{public}d result=%{public}d",
                         ret, result);
    AUDIO_INFO_LOG("OnAddSystemAbility: re-register status callback success");
}

void AudioSuiteSystemAbilityListener::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    AudioSuiteClientPolicy::GetInstance().AudioSuiteServerRecover(systemAbilityId);
    AUDIO_INFO_LOG("AudioSuiteSystemAbilityListener::OnAddSystemAbility RecoverAudioSuiteService");
}

void AudioSuiteSystemAbilityListener::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    AUDIO_INFO_LOG("OnRemoveSystemAbility, systemAbilityId=%{public}d", systemAbilityId);
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS