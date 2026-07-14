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
#define LOG_TAG "OHAudioSuiteDownloadManager"
#endif

#include <cinttypes>
#include <string>
#include <thread>
#include "audio_utils.h"
#include "audio_errors.h"
#include "audio_common_log.h"
#include "OHAudioSuiteDownloadManager.h"
#include "audio_suite_download_manager.h"
#include "native_audio_suite_base.h"
#include "native_audio_common.h"
#include "audio_suite_sa_types.h"
#include "securec.h"

using OHOS::AudioStandard::OHAudioSuiteDownloadManager;

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

class CDownloadCallbackAdapter : public IDownloadCallbackListener {
public:
    explicit CDownloadCallbackAdapter(const OH_AudioSuite_DownloadCallback &cCallback)
        : callback_(cCallback)
    {
        statusArray_.size = STATUS_ARRAY_SIZE;
        statusArray_.downloadStatusInfo = &statusInfoPtr_;
        statusInfo_.installPath[0] = '\0';
        statusInfoPtr_ = &statusInfo_;
    }

    ~CDownloadCallbackAdapter() override = default;

    void OnDownloadStatusChanged(const DownloadStatus& status) override
    {
        statusInfo_.downloadStatus = ConvertDownloadStateToCloudRomStatus(status.state);
        statusInfo_.progress = status.progress;
        statusInfo_.errorCode = status.errorCode;

        strncpy_s(statusInfo_.featureName, sizeof(statusInfo_.featureName), status.version.c_str(),
                  status.version.length());
        statusInfo_.featureName[sizeof(statusInfo_.featureName) - 1] = '\0';

        strncpy_s(statusInfo_.installPath, sizeof(statusInfo_.installPath), status.romPath.c_str(),
                  status.romPath.length());
        statusInfo_.installPath[sizeof(statusInfo_.installPath) - 1] = '\0';

        callback_(&statusArray_);
    }

private:
    static int32_t ConvertDownloadStateToCloudRomStatus(DownloadState state)
    {
        switch (state) {
            case DownloadState::DOWNLOAD_IDLE:
                return CLOUD_ROM_STATUS_INIT;
            case DownloadState::DOWNLOAD_WAITING:
                return CLOUD_ROM_STATUS_INIT;
            case DownloadState::DOWNLOAD_IN_PROGRESS:
                return CLOUD_ROM_STATUS_DOWNLOADING;
            case DownloadState::DOWNLOAD_COMPLETED:
                return CLOUD_ROM_STATUS_DOWNLOAD_SUCCESS;
            case DownloadState::DOWNLOAD_FAILED:
                return CLOUD_ROM_STATUS_DOWNLOAD_FAIL;
            case DownloadState::DOWNLOAD_CANCELLED:
                return CLOUD_ROM_STATUS_DOWNLOAD_FAIL;
            default:
                return CLOUD_ROM_STATUS_ERROR;
        }
    }

    OH_AudioSuite_DownloadCallback callback_;
    OH_AudioSuite_DownloadStatusInfo statusInfo_;
    OH_AudioSuite_DownloadStatusInfo* statusInfoPtr_;
    OH_AudioSuite_DownloadStatusInfoArray statusArray_;
};

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS

static OH_AudioCommon_Result ConvertToSaResult(int32_t internalError)
{
    using namespace OHOS::AudioStandard;

    if (internalError == SUCCESS) {
        return AUDIOCOMMON_RESULT_SUCCESS;
    }
    if (internalError == ERR_INVALID_PARAM) {
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }
    if (internalError == ERR_ILLEGAL_STATE || internalError == ERR_NOT_STARTED) {
        return AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE;
    }
    if (internalError == ERR_OPERATION_FAILED || internalError == ERR_IPC) {
        return AUDIOCOMMON_RESULT_ERROR_SYSTEM;
    }

    return AUDIOCOMMON_RESULT_ERROR_SYSTEM;
}

static OHOS::AudioStandard::OHAudioSuiteDownloadManager *ConvertAudioSuitDownloadManager(
    OH_AudioSuite_DownloadManager *downloadManager)
{
    return reinterpret_cast<OHAudioSuiteDownloadManager *>(downloadManager);
}

int32_t OH_AudioSuite_GetDownloadManager(OH_AudioSuite_DownloadManager **downloadManager)
{
    OHAudioSuiteDownloadManager* manager = OHAudioSuiteDownloadManager::GetInstance();
    if (manager == nullptr) {
        AUDIO_ERR_LOG("downloadManager is nullptr");
        return AUDIOSUITE_SUCCESS;
    }
    *downloadManager = reinterpret_cast<OH_AudioSuite_DownloadManager*>(manager);
    return AUDIOSUITE_SUCCESS;
}

int32_t OH_AudioSuite_RegisterDownloadCallback(
    OH_AudioSuite_DownloadManager* downloadManager, const OH_AudioSuite_DownloadCallback* callback)
{
    if (downloadManager == nullptr || callback == nullptr) {
        AUDIO_ERR_LOG("OH_AudioSuite_RegisterDownloadCallback: manager or callback is null");
        return AUDIOSUITE_ERROR_INVALID_PARAM;
    }

    AUDIO_INFO_LOG("OH_AudioSuite_RegisterDownloadCallback called");

    auto* cppManager = ConvertAudioSuitDownloadManager(downloadManager);
    int32_t ret = cppManager->RegisterDownloadCallback(callback);
    return ConvertToSaResult(ret);
}

int32_t OH_AudioSuite_StartDownload(
    OH_AudioSuite_DownloadManager *downloadManager, const char *modelVersion)
{
    if (downloadManager == nullptr || modelVersion == nullptr) {
        AUDIO_ERR_LOG("OH_AudioSuite_StartDownload: Invalid parameter");
        return AUDIOSUITE_ERROR_INVALID_PARAM;
    }
    auto* cppManager = ConvertAudioSuitDownloadManager(downloadManager);
    int32_t ret = cppManager->StartDownload(modelVersion);
    return ConvertToSaResult(ret);
}

int32_t OH_AudioSuite_CancelDownload(
    OH_AudioSuite_DownloadManager *downloadManager, const char *modelVersion)
{
    if (downloadManager == nullptr || modelVersion == nullptr) {
        AUDIO_ERR_LOG("OH_AudioSuite_CancelDownload: Invalid parameter");
        return AUDIOSUITE_ERROR_INVALID_PARAM;
    }

    AUDIO_INFO_LOG("OH_AudioSuite_CancelDownload: modelVersion=%{public}s", modelVersion);
    auto* cppManager = ConvertAudioSuitDownloadManager(downloadManager);
    int32_t ret = cppManager->CancelDownload(modelVersion);
    return ConvertToSaResult(ret);
}

int32_t OH_AudioSuite_StartBackgroundDownload(
    OH_AudioSuite_DownloadManager *downloadManager, const char *modelVersion)
{
    if (downloadManager == nullptr || modelVersion == nullptr) {
        AUDIO_ERR_LOG("OH_AudioSuite_StartBackgroundDownload: Invalid parameter");
        return AUDIOSUITE_ERROR_INVALID_PARAM;
    }

    auto* cppManager = ConvertAudioSuitDownloadManager(downloadManager);
    int32_t ret = cppManager->StartBackgroundDownload(modelVersion);
    return ConvertToSaResult(ret);
}

int32_t OH_AudioSuite_GetDownloadStatus(
    OH_AudioSuite_DownloadManager *downloadManager, const char *modelVersion,
    OH_AudioSuite_DownloadStatusInfo *status)
{
    if (downloadManager == nullptr || status == nullptr) {
        AUDIO_ERR_LOG("OH_AudioSuite_GetDownloadStatus: Invalid parameter");
        return AUDIOSUITE_ERROR_INVALID_PARAM;
    }

    auto* cppManager = ConvertAudioSuitDownloadManager(downloadManager);
    int32_t ret = cppManager->GetDownloadStatus(status, modelVersion);
    return ConvertToSaResult(ret);
}

int32_t OH_AudioSuite_UninstallCloudRom(
    OH_AudioSuite_DownloadManager *downloadManager, const char *modelVersion)
{
    if (downloadManager == nullptr || modelVersion == nullptr) {
        AUDIO_ERR_LOG("OH_AudioSuite_UninstallCloudRom: Invalid parameter");
        return AUDIOSUITE_ERROR_INVALID_PARAM;
    }
    auto* cppManager = ConvertAudioSuitDownloadManager(downloadManager);
    int32_t ret = cppManager->UninstallCloudRom(modelVersion);
    return ConvertToSaResult(ret);
}

int32_t OH_AudioSuite_UnregisterDownloadCallback(
    OH_AudioSuite_DownloadManager* downloadManager, const OH_AudioSuite_DownloadCallback* callback)
{
    if (downloadManager == nullptr) {
        AUDIO_ERR_LOG("OH_AudioSuite_UnregisterDownloadCallback: manager is null");
        return AUDIOSUITE_ERROR_INVALID_PARAM;
    }

    AUDIO_INFO_LOG("OH_AudioSuite_UnregisterDownloadCallback called");

    auto* cppManager = ConvertAudioSuitDownloadManager(downloadManager);
    int32_t ret = cppManager->UnregisterDownloadCallback(callback);
    return ConvertToSaResult(ret);
}

namespace OHOS {
namespace AudioStandard {

OHAudioSuiteDownloadManager::OHAudioSuiteDownloadManager()
{
    AUDIO_INFO_LOG("OHAudioSuiteDownloadManager created!");
}

OHAudioSuiteDownloadManager::~OHAudioSuiteDownloadManager()
{
    AUDIO_INFO_LOG("OHAudioSuiteDownloadManager destroyed!");
}

int32_t OHAudioSuiteDownloadManager::StartDownload(const char *modelVersion)
{
    auto& manager = OHOS::AudioStandard::AudioSuite::AudioSuiteDownloadManager::GetInstance();
    int32_t ret = manager.StartDownload(std::string(modelVersion));
    return ret;
}

int32_t OHAudioSuiteDownloadManager::RegisterDownloadCallback(const OH_AudioSuite_DownloadCallback* callback)
{
    auto& manager = OHOS::AudioStandard::AudioSuite::AudioSuiteDownloadManager::GetInstance();
    int32_t ret;
    if (callback == nullptr) {
        AUDIO_INFO_LOG("OHAudioSuiteDownloadManager::RegisterDownloadCallback: unregister callback");
        ret = manager.RegisterDownloadCallback(nullptr);
    } else {
        auto callbackAdapter = std::make_shared<OHOS::AudioStandard::AudioSuite::CDownloadCallbackAdapter>(*callback);
        ret = manager.RegisterDownloadCallback(callbackAdapter);
    }
    return ret;
}

int32_t OHAudioSuiteDownloadManager::UnregisterDownloadCallback(const OH_AudioSuite_DownloadCallback* callback)
{
    auto &manager = OHOS::AudioStandard::AudioSuite::AudioSuiteDownloadManager::GetInstance();
    AUDIO_INFO_LOG("OHAudioSuiteDownloadManager::UnregisterDownloadCallback");
    int32_t ret = manager.UnregisterDownloadCallback();
    return ret;
}

int32_t OHAudioSuiteDownloadManager::CancelDownload(const char *modelVersion)
{
    auto& manager = OHOS::AudioStandard::AudioSuite::AudioSuiteDownloadManager::GetInstance();
    int32_t ret = manager.CancelDownload(std::string(modelVersion));
    return ret;
}

int32_t OHAudioSuiteDownloadManager::StartBackgroundDownload(const char *modelVersion)
{
    auto& manager = OHOS::AudioStandard::AudioSuite::AudioSuiteDownloadManager::GetInstance();
    int32_t ret = manager.StartBackgroundDownload(std::string(modelVersion));
    return ret;
}

int32_t OHAudioSuiteDownloadManager::UninstallCloudRom(const char *modelVersion)
{
    auto& manager = OHOS::AudioStandard::AudioSuite::AudioSuiteDownloadManager::GetInstance();
    int32_t ret = manager.UninstallCloudRom(std::string(modelVersion));
    return ret;
}

int32_t OHAudioSuiteDownloadManager::GetDownloadStatus(
    OH_AudioSuite_DownloadStatusInfo *status, const char *modelVersion)
{
    auto& manager = OHOS::AudioStandard::AudioSuite::AudioSuiteDownloadManager::GetInstance();
    OHOS::AudioStandard::AudioSuite::DownloadStatus internalStatus;
    int32_t ret = manager.GetDownloadStatus(internalStatus, std::string(modelVersion));
    if (ret == OHOS::AudioStandard::SUCCESS) {
        // Map internal status to new status structure
        if (strncpy_s(status->featureName, sizeof(status->featureName), internalStatus.version.c_str(),
                      internalStatus.version.length()) != EOK) {
            status->featureName[0] = '\0';
        }

        // Map DownloadState to downloadStatus
        switch (internalStatus.state) {
            case OHOS::AudioStandard::AudioSuite::DownloadState::DOWNLOAD_IDLE:
                status->downloadStatus = AudioSuite::CLOUD_ROM_STATUS_INIT;
                break;
            case OHOS::AudioStandard::AudioSuite::DownloadState::DOWNLOAD_WAITING:
                status->downloadStatus = AudioSuite::CLOUD_ROM_STATUS_INIT;
                break;
            case OHOS::AudioStandard::AudioSuite::DownloadState::DOWNLOAD_IN_PROGRESS:
                status->downloadStatus = AudioSuite::CLOUD_ROM_STATUS_DOWNLOADING;
                break;
            case OHOS::AudioStandard::AudioSuite::DownloadState::DOWNLOAD_COMPLETED:
                status->downloadStatus = AudioSuite::CLOUD_ROM_STATUS_DOWNLOAD_SUCCESS;
                break;
            case OHOS::AudioStandard::AudioSuite::DownloadState::DOWNLOAD_FAILED:
                status->downloadStatus = AudioSuite::CLOUD_ROM_STATUS_DOWNLOAD_FAIL;
                break;
            case OHOS::AudioStandard::AudioSuite::DownloadState::DOWNLOAD_CANCELLED:
                status->downloadStatus = AudioSuite::CLOUD_ROM_STATUS_DOWNLOAD_FAIL;
                break;
            default:
                status->downloadStatus = AudioSuite::CLOUD_ROM_STATUS_ERROR;
                break;
        }

        if (strncpy_s(status->installPath, sizeof(status->installPath), internalStatus.romPath.c_str(),
                      internalStatus.romPath.length()) != EOK) {
            status->installPath[0] = '\0';
        }

        status->size = internalStatus.fileSize;
        status->progress = internalStatus.progress;
        status->errorCode = internalStatus.errorCode;
    }

    return ret;
}

}  // namespace AudioStandard
}  // namespace OHOS
