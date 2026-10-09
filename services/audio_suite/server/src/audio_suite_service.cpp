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
#define LOG_TAG "audio_suite_service"
#endif

#include "audio_suite_service.h"
#include "audio_suite_log.h"
#include "audio_utils.h"
#include "download_manager.h"
#ifndef AUDIOSUITE_SA_UT
#include "iservice_registry.h"
#include "system_ability_definition.h"
#endif
#include "hisysevent.h"
#include <fcntl.h>
#include <unistd.h>
#include <ctime>

#include "ipc_skeleton.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

#ifndef AUDIOSUITE_SA_UT
static constexpr const char* TASK_ID = "AudioSuiteSaUnloadTask";
static constexpr int32_t AUDIO_SUITE_SA_ID = 3015;
static constexpr int32_t DELAY_TIME = 300000; // 5 minutes in milliseconds
static constexpr int32_t AUDIO_SUITE_SERVICE_IMPL_TIMEOUT = 10; // 10s
static constexpr uid_t UID_AUDIO = 1041;
#endif

#ifndef AUDIOSUITE_SA_UT
REGISTER_SYSTEM_ABILITY_BY_ID(AudioSuiteService, AUDIO_SUITE_SA_ID, true);
#endif

AudioSuiteService::AudioSuiteService(int32_t systemAbilityId, bool runOnCreate)
    : SystemAbility(systemAbilityId, runOnCreate), downloadManager_(OHOS::Singleton<DownloadManager>::GetInstance())
{
    AUDIO_INFO_LOG("AudioSuiteService constructor start, saId: %{public}d", systemAbilityId);
}

AudioSuiteService::~AudioSuiteService()
{
    std::shared_ptr<AppExecFwk::EventRunner> runnerCopy;
    {
        std::lock_guard<std::mutex> lock(eventMutex_);
        if (unloadHandler_ != nullptr) {
            unloadHandler_->RemoveAllEvents();
            unloadHandler_.reset();
        }
        runnerCopy = unloadRunner_;
        unloadRunner_.reset();
    }
    if (runnerCopy != nullptr) {
        runnerCopy->Stop();
    }
}

void AudioSuiteService::OnStart(const SystemAbilityOnDemandReason& startReason)
{
    std::string reasonName = startReason.GetName();
    AUDIO_INFO_LOG("audio_suite_server, reasonName:%{public}s", reasonName.c_str());
    {
        std::lock_guard<std::mutex> lock(eventMutex_);
        unloadRunner_ = AppExecFwk::EventRunner::Create("AudioSuiteSaUnloadRunner");
        CHECK_AND_RETURN_LOG(unloadRunner_ != nullptr, "Failed to create unload runner");
        unloadHandler_ = std::make_shared<AppExecFwk::EventHandler>(unloadRunner_);
        CHECK_AND_RETURN_LOG(unloadHandler_ != nullptr, "Failed to create unload handler");
    }

    std::lock_guard<std::mutex> lock(mutex_);
    bool res = Publish(this);
    if (!res) {
        AUDIO_ERR_LOG("AudioSuiteService: Publish failed");
        return;
    }
    PostDelayUnloadTask();

    // Initialize background download subsystem
    bool syncProcess = (reasonName != "load");
    downloadManager_.InitDownloadManager(syncProcess);

    AUDIO_INFO_LOG("AudioSuiteService: Publish success");
}

void AudioSuiteService::OnStop()
{
    AUDIO_INFO_LOG("AudioSuiteService::OnStop");
    downloadManager_.DeInitDownloadManager();
}

ErrCode AudioSuiteService::RegisterDownloadStatusCallback(const sptr<IRemoteObject>& object, ErrCode& result)
{
    AUDIO_INFO_LOG("AudioSuiteService::RegisterDownloadStatusCallback");
#ifndef AUDIOSUITE_SA_UT
    AudioXCollie audioXCollie("AudioSuiteService::RegisterDownloadStatusCallback", AUDIO_SUITE_SERVICE_IMPL_TIMEOUT,
        [](void *) {
            AUDIO_ERR_LOG("[xcollie] RegisterDownloadStatusCallback timeout");
        }, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
#endif
    CHECK_AND_RETURN_RET_LOG(VerifyIsSystemApp(result), SUCCESS, "RegisterDownloadStatusCallback: not permission");
    PostDelayUnloadTask();

    int32_t callerPid = IPCSkeleton::GetCallingPid();
    result = downloadManager_.RegisterDownloadStatusCallback(callerPid, object);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, SUCCESS,
        "RegisterDownloadStatusCallback failed, callerPid:%{public}d result:%{public}d", callerPid, result);
    return SUCCESS;
}

ErrCode AudioSuiteService::UnregisterDownloadStatusCallback(ErrCode &result)
{
    AUDIO_INFO_LOG("AudioSuiteService::UnregisterDownloadStatusCallback");
#ifndef AUDIOSUITE_SA_UT
    AudioXCollie audioXCollie("AudioSuiteService::UnregisterDownloadStatusCallback", AUDIO_SUITE_SERVICE_IMPL_TIMEOUT,
        [](void *) {
            AUDIO_ERR_LOG("[xcollie] UnregisterDownloadStatusCallback timeout");
        }, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
#endif
    CHECK_AND_RETURN_RET_LOG(VerifyIsSystemApp(result), SUCCESS, "UnregisterDownloadStatusCallback: not permission");
    PostDelayUnloadTask();

    int32_t callerPid = IPCSkeleton::GetCallingPid();
    result = downloadManager_.UnregisterDownloadStatusCallback(callerPid);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, SUCCESS,
        "UnregisterDownloadStatusCallback failed, result:%{public}d", result);
    return SUCCESS;
}

ErrCode AudioSuiteService::RegisterDownloadInterfaceCallback(const sptr<IRemoteObject> &object, ErrCode& result)
{
    AUDIO_INFO_LOG("AudioSuiteService::RegisterDownloadInterfaceCallback");
#ifndef AUDIOSUITE_SA_UT
    CHECK_AND_RETURN_RET_LOG(IPCSkeleton::GetCallingUid() == UID_AUDIO, ERR_PERMISSION_DENIED,
        "RegisterDownloadInterfaceCallback callerUid:%{public}d is not allowed", IPCSkeleton::GetCallingUid());
#endif
    PostDelayUnloadTask();

    result = downloadManager_.SetUpdateEngineCallback(object);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, SUCCESS,
        "RegisterDownloadInterfaceCallback failed, result:%{public}d", result);
    return SUCCESS;
}

ErrCode AudioSuiteService::UnregisterDownloadInterfaceCallback(ErrCode& result)
{
    AUDIO_INFO_LOG("AudioSuiteService::UnregisterDownloadInterfaceCallback");
#ifndef AUDIOSUITE_SA_UT
    CHECK_AND_RETURN_RET_LOG(IPCSkeleton::GetCallingUid() == UID_AUDIO, ERR_PERMISSION_DENIED,
        "UnregisterDownloadInterfaceCallback callerUid:%{public}d is not allowed", IPCSkeleton::GetCallingUid());
#endif
    PostDelayUnloadTask();

    result = downloadManager_.SetUpdateEngineCallback(nullptr);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, SUCCESS,
        "UnregisterDownloadInterfaceCallback failed, result:%{public}d", result);
    return SUCCESS;
}

ErrCode AudioSuiteService::GetInstallStatus(
    const std::vector<std::string> &featureNames, std::vector<bool> &installStatus, ErrCode &result)
{
    AUDIO_INFO_LOG("AudioSuiteService::GetInstallStatus, featureNames size=%{public}zu",
        featureNames.size());
#ifndef AUDIOSUITE_SA_UT
    AudioXCollie audioXCollie("AudioSuiteService::GetInstallStatus", AUDIO_SUITE_SERVICE_IMPL_TIMEOUT,
        [](void *) {
            AUDIO_ERR_LOG("[xcollie] GetInstallStatus timeout");
        }, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
#endif
    CHECK_AND_RETURN_RET_LOG(VerifyIsSystemApp(result), SUCCESS, "GetInstallStatus: not permission");
    PostDelayUnloadTask();

    result = downloadManager_.GetInstallStatus(featureNames, installStatus);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, SUCCESS,
        "GetInstallStatus failed, result:%{public}d", result);
    return SUCCESS;
}

ErrCode AudioSuiteService::StartDownload(
    const std::vector<std::string>& featureNames, bool allowedCellularNet, ErrCode& result)
{
    AUDIO_INFO_LOG("AudioSuiteService::StartDownload, featureNames size=%{public}zu, allowedCellularNet=%{public}d",
        featureNames.size(), allowedCellularNet);
#ifndef AUDIOSUITE_SA_UT
    AudioXCollie audioXCollie("AudioSuiteService::StartDownload", AUDIO_SUITE_SERVICE_IMPL_TIMEOUT,
        [](void *) {
            AUDIO_ERR_LOG("[xcollie] StartDownload timeout");
        }, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
#endif
    CHECK_AND_RETURN_RET_LOG(VerifyIsSystemApp(result), SUCCESS, "StartDownload: not permission");
    PostDelayUnloadTask();

    result = downloadManager_.StartDownload(featureNames, allowedCellularNet);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, SUCCESS, "StartDownload failed, result:%{public}d", result);
    return SUCCESS;
}

ErrCode AudioSuiteService::PauseDownload(const std::vector<std::string>& featureNames, ErrCode& result)
{
    AUDIO_INFO_LOG("AudioSuiteService::PauseDownload, featureNames size=%{public}zu", featureNames.size());
#ifndef AUDIOSUITE_SA_UT
    AudioXCollie audioXCollie("AudioSuiteService::PauseDownload", AUDIO_SUITE_SERVICE_IMPL_TIMEOUT,
        [](void *) {
            AUDIO_ERR_LOG("[xcollie] PauseDownload timeout");
        }, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
#endif
    CHECK_AND_RETURN_RET_LOG(VerifyIsSystemApp(result), SUCCESS, "PauseDownload: not permission");
    PostDelayUnloadTask();

    result = downloadManager_.PauseDownload(featureNames);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, SUCCESS, "PauseDownload failed, result:%{public}d", result);
    return SUCCESS;
}

ErrCode AudioSuiteService::CancelDownload(const std::vector<std::string>& featureNames, ErrCode& result)
{
    AUDIO_INFO_LOG("AudioSuiteService::CancelDownload, featureNames size=%{public}zu", featureNames.size());
#ifndef AUDIOSUITE_SA_UT
    AudioXCollie audioXCollie("AudioSuiteService::CancelDownload", AUDIO_SUITE_SERVICE_IMPL_TIMEOUT,
        [](void *) {
            AUDIO_ERR_LOG("[xcollie] CancelDownload timeout");
        }, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
#endif
    CHECK_AND_RETURN_RET_LOG(VerifyIsSystemApp(result), SUCCESS, "CancelDownload: not permission");
    PostDelayUnloadTask();

    result = downloadManager_.CancelDownload(featureNames);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, SUCCESS, "CancelDownload failed, result:%{public}d", result);
    return SUCCESS;
}

ErrCode AudioSuiteService::StartBackgroundDownload(const std::vector<std::string>& featureNames, ErrCode& result)
{
    AUDIO_INFO_LOG("AudioSuiteService::StartBackgroundDownload, featureNames size=%{public}zu", featureNames.size());
#ifndef AUDIOSUITE_SA_UT
    AudioXCollie audioXCollie("AudioSuiteService::StartBackgroundDownload", AUDIO_SUITE_SERVICE_IMPL_TIMEOUT,
        [](void *) {
            AUDIO_ERR_LOG("[xcollie] StartBackgroundDownload timeout");
        }, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
#endif
    CHECK_AND_RETURN_RET_LOG(VerifyIsSystemApp(result), SUCCESS, "StartBackgroundDownload: not permission");
    PostDelayUnloadTask();

    result = downloadManager_.StartBackgroundDownload(featureNames);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, SUCCESS, "StartBackgroundDownload failed, result:%{public}d", result);
    return SUCCESS;
}

ErrCode AudioSuiteService::UninstallCloudRom(const std::vector<std::string> &featureNames, ErrCode &result)
{
    AUDIO_INFO_LOG("AudioSuiteService::UninstallCloudRom, featureNames size=%{public}zu", featureNames.size());
#ifndef AUDIOSUITE_SA_UT
    AudioXCollie audioXCollie("AudioSuiteService::UninstallCloudRom", AUDIO_SUITE_SERVICE_IMPL_TIMEOUT,
        [](void *) {
            AUDIO_ERR_LOG("[xcollie] UninstallCloudRom timeout");
        }, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
#endif
    CHECK_AND_RETURN_RET_LOG(VerifyIsSystemApp(result), SUCCESS, "UninstallCloudRom: not permission");
    PostDelayUnloadTask();

    result = downloadManager_.UninstallCloudRom(featureNames);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, SUCCESS, "UninstallCloudRom failed, result:%{public}d", result);
    return SUCCESS;
}

ErrCode AudioSuiteService::GetDownloadStatus(
    const std::vector<std::string> &featureNames, std::vector<AudioSuiteDownloadStatusInfo> &status, ErrCode &result)
{
    AUDIO_INFO_LOG("AudioSuiteService::GetDownloadStatus, featureNames size=%{public}zu", featureNames.size());
#ifndef AUDIOSUITE_SA_UT
    AudioXCollie audioXCollie("AudioSuiteService::GetDownloadStatus", AUDIO_SUITE_SERVICE_IMPL_TIMEOUT,
        [](void *) {
            AUDIO_ERR_LOG("[xcollie] GetDownloadStatus timeout");
        }, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
#endif
    CHECK_AND_RETURN_RET_LOG(VerifyIsSystemApp(result), SUCCESS, "GetDownloadStatus: not permission");
    PostDelayUnloadTask();

    result = downloadManager_.GetDownloadStatus(featureNames, status);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, SUCCESS, "GetDownloadStatus failed, result:%{public}d", result);
    return SUCCESS;
}

ErrCode AudioSuiteService::GetFeatureStatus(
    const std::vector<std::string> &featureNames, std::vector<AudioSuiteDownloadStatusInfo> &status, ErrCode &result)
{
    AUDIO_INFO_LOG("AudioSuiteService::GetFeatureStatus, featureNames size=%{public}zu", featureNames.size());
#ifndef AUDIOSUITE_SA_UT
    AudioXCollie audioXCollie("AudioSuiteService::GetFeatureStatus", AUDIO_SUITE_SERVICE_IMPL_TIMEOUT,
        [](void *) {
            AUDIO_ERR_LOG("[xcollie] GetFeatureStatus timeout");
        }, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
#endif
    CHECK_AND_RETURN_RET_LOG(VerifyIsSystemApp(result), SUCCESS, "GetFeatureStatus: not permission");
    PostDelayUnloadTask();

    result = downloadManager_.GetFeatureStatus(featureNames, status);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, SUCCESS, "GetFeatureStatus failed, result:%{public}d", result);
    return SUCCESS;
}

ErrCode AudioSuiteService::GetFeatureNewVersion(
    const std::vector<std::string> &featureNames, std::vector<AudioSuiteCloudRomVersionInfo> &newVersion,
    ErrCode &result)
{
    AUDIO_INFO_LOG("AudioSuiteService::GetFeatureNewVersion, featureNames size=%{public}zu", featureNames.size());
#ifndef AUDIOSUITE_SA_UT
    AudioXCollie audioXCollie("AudioSuiteService::GetFeatureNewVersion", AUDIO_SUITE_SERVICE_IMPL_TIMEOUT,
        [](void *) {
            AUDIO_ERR_LOG("[xcollie] GetFeatureNewVersion timeout");
        }, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
#endif
    CHECK_AND_RETURN_RET_LOG(VerifyIsSystemApp(result), SUCCESS, "GetFeatureNewVersion: not permission");
    PostDelayUnloadTask();

    result = downloadManager_.GetFeatureNewVersion(featureNames, newVersion);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, SUCCESS, "GetFeatureNewVersion failed, result:%{public}d", result);
    return SUCCESS;
}


ErrCode AudioSuiteService::OnDownloadStatusChanged(const std::vector<AudioSuiteDownloadStatusInfo>& status)
{
    AUDIO_INFO_LOG("AudioSuiteService::OnDownloadStatusChanged");
#ifndef AUDIOSUITE_SA_UT
    CHECK_AND_RETURN_RET_LOG(IPCSkeleton::GetCallingUid() == UID_AUDIO, ERR_PERMISSION_DENIED,
        "OnDownloadStatusChanged callerUid:%{public}d is not allowed", IPCSkeleton::GetCallingUid());
#endif
    PostDelayUnloadTask();

    int32_t result = downloadManager_.OnDownloadStatusChanged(status);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, SUCCESS, "OnDownloadStatusChanged failed, result:%{public}d", result);
    return SUCCESS;
}

bool AudioSuiteService::VerifyIsSystemApp(ErrCode& result)
{
#ifndef AUDIOSUITE_SA_UT
    if (!PermissionUtil::VerifySystemPermission()) {
        result = ERR_SYSTEM_PERMISSION_DENIED;
        return false;
    }
#endif
    return true;
}

bool AudioSuiteService::ShouldExtendUnloadTimer()
{
    return downloadManager_.HasDownloadingTask();
}

void AudioSuiteService::PostDelayUnloadTask()
{
#ifndef AUDIOSUITE_SA_UT
    std::lock_guard<std::mutex> lock(eventMutex_);
    CHECK_AND_RETURN_LOG(unloadHandler_ != nullptr, "PostDelayUnloadTask: unloadHandler_ is null");

    unloadHandler_->RemoveTask(TASK_ID);
    unloadHandler_->PostTask(
        [this]() {
            if (ShouldExtendUnloadTimer()) {
                PostDelayUnloadTask();
                return;
            }

            auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
            CHECK_AND_RETURN_LOG(samgrProxy != nullptr, "get samgr failed");
            AUDIO_INFO_LOG("start UnloadSystemAbility");
            int32_t ret = samgrProxy->UnloadSystemAbility(AUDIO_SUITE_SA_ID);
            CHECK_AND_RETURN_LOG(ret == SUCCESS, "unload system ability failed, ret=%{public}d", ret);
        },
        TASK_ID,
        DELAY_TIME);
#endif
}

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS