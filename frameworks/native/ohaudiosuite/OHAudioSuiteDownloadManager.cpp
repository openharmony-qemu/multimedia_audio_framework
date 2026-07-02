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

using OHOS::AudioStandard::OHAudioSuiteDownloadManager;
            
static OH_AudioSuite_Result ConvertToSaResult(int32_t internalError)
{
    using namespace OHOS::AudioStandard;

    if (internalError == SUCCESS) {
        return AUDIOSUITE_SUCCESS;
    }
    if (internalError == ERR_INVALID_PARAM) {
        return AUDIOSUITE_ERROR_INVALID_PARAM;
    }
    if (internalError == ERR_ILLEGAL_STATE || internalError == ERR_NOT_STARTED) {
        return AUDIOSUITE_ERROR_INVALID_STATE;
    }
    if (internalError == ERR_IPC) {
        return AUDIOSUITE_ERROR_IPC_FAILED;
    }
    if (internalError == ERR_AUDIO_SUITE_SA_PIPELINE_NOT_EXIST) {
        return AUDIOSUITE_ERROR_PIPELINE_NOT_EXIST;
    }
    if (internalError == ERR_MEMORY_ALLOC_FAILED) {
        return AUDIOSUITE_ERROR_MEMORY_ALLOC_FAILED;
    }
    if (internalError == ERR_NOT_SUPPORTED || internalError == ERR_DEVICE_NOT_SUPPORTED) {
        return AUDIOSUITE_ERROR_UNSUPPORTED;
    }
    if (internalError == ERR_AUDIO_SUITE_SA_PIPELINE_BUSY) {
        return AUDIOSUITE_ERROR_PIPELINE_BUSY;
    }
    if (internalError == ERR_AUDIO_SUITE_SA_NOT_READY) {
        return AUDIOSUITE_ERROR_NOT_READY;
    }
    if (internalError == ERR_AUDIO_SUITE_SA_DOWNLOAD_FAILED) {
        return AUDIOSUITE_ERROR_DOWNLOAD_FAILED;
    }
    if (internalError == ERR_AUDIO_SUITE_SA_CONDITION_NOT_MET) {
        return AUDIOSUITE_ERROR_CONDITION_NOT_MET;
    }
    if (internalError == ERR_AUDIO_SUITE_SA_FREQUENT_CRASH) {
        return AUDIOSUITE_ERROR_FREQUENT_CRASH;
    }
    if (internalError == ERR_OPERATION_FAILED || internalError == ERR_AUDIO_SUITE_SA_PROCESS_FAILED) {
        return AUDIOSUITE_ERROR_SYSTEM;
    }

    return AUDIOSUITE_ERROR_SYSTEM;
}

static OHOS::AudioStandard::OHAudioSuiteDownloadManager *ConvertAudioSuitDownloadManager(
    OH_AudioSuiteDownloadManager *downloadManager)
{
    return reinterpret_cast<OHAudioSuiteDownloadManager *>(downloadManager);
}

int32_t OH_AudioSuite_GetDownloadManager(OH_AudioSuiteDownloadManager **downloadManager)
{
    OHAudioSuiteDownloadManager* manager = OHAudioSuiteDownloadManager::GetInstance();
    if (manager == nullptr) {
        AUDIO_ERR_LOG("downloadManager is nullptr");
        return AUDIOSUITE_SUCCESS;
    }
    *downloadManager = reinterpret_cast<OH_AudioSuiteDownloadManager*>(manager);
    return AUDIOSUITE_SUCCESS;
}

int32_t OH_AudioSuite_RegisterDownloadCallback(
    OH_AudioSuiteDownloadManager* downloadManager, const OH_AudioSuite_DownloadCallback* callback)
{
    if (downloadManager == nullptr) {
        AUDIO_ERR_LOG("OH_AudioSuite_RegisterDownloadCallback: manager is null");
        return AUDIOSUITE_ERROR_INVALID_PARAM;
    }

    AUDIO_INFO_LOG("OH_AudioSuite_RegisterDownloadCallback called");

    auto* cppManager = ConvertAudioSuitDownloadManager(downloadManager);
    int32_t ret = cppManager->RegisterDownloadCallback(callback);
    return ConvertToSaResult(ret);
}

int32_t OH_AudioSuite_StartDownload(
    OH_AudioSuiteDownloadManager *downloadManager, const char *modelVersion)
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
    OH_AudioSuiteDownloadManager *downloadManager, const char *modelVersion)
{
    if (downloadManager == nullptr) {
        AUDIO_ERR_LOG("OH_AudioSuite_CancelDownload: Invalid parameter");
        return AUDIOSUITE_ERROR_INVALID_PARAM;
    }

    // modelVersion parameter is reserved for future use, not used currently
    AUDIO_INFO_LOG("OH_AudioSuite_CancelDownload");
    auto* cppManager = ConvertAudioSuitDownloadManager(downloadManager);
    int32_t ret = cppManager->CancelDownload();
    return ConvertToSaResult(ret);
}

int32_t OH_AudioSuite_StartBackgroundDownload(
    OH_AudioSuiteDownloadManager *downloadManager, const char *modelVersion)
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
    OH_AudioSuiteDownloadManager *downloadManager, const char *modelVersion,
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
    OH_AudioSuiteDownloadManager *downloadManager, const char *modelVersion)
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
    OH_AudioSuiteDownloadManager* downloadManager, const OH_AudioSuite_DownloadCallback* callback)
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

int32_t OHAudioSuiteDownloadManager::CancelDownload()
{
    auto& manager = OHOS::AudioStandard::AudioSuite::AudioSuiteDownloadManager::GetInstance();
    int32_t ret = manager.CancelDownload();
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

int32_t OHAudioSuiteDownloadManager::GetDownloadStatus(OH_AudioSuite_DownloadStatusInfo *status, const char *modelVersion)
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

        // Map DownloadState to cloudRomStatus
        switch (internalStatus.state) {
            case OHOS::AudioStandard::AudioSuite::DownloadState::DOWNLOAD_IDLE:
                status->cloudRomStatus = 0;  // AUDIOSUITE_CLOUD_ROM_STATUS_INIT
                break;
            case OHOS::AudioStandard::AudioSuite::DownloadState::DOWNLOAD_WAITING:
                status->cloudRomStatus = 0;  // AUDIOSUITE_CLOUD_ROM_STATUS_INIT
                break;
            case OHOS::AudioStandard::AudioSuite::DownloadState::DOWNLOAD_IN_PROGRESS:
                status->cloudRomStatus = 20;  // AUDIOSUITE_CLOUD_ROM_STATUS_DOWNLOADING
                break;
            case OHOS::AudioStandard::AudioSuite::DownloadState::DOWNLOAD_COMPLETED:
                status->cloudRomStatus = 23;  // AUDIOSUITE_CLOUD_ROM_STATUS_DOWNLOAD_SUCCESS
                break;
            case OHOS::AudioStandard::AudioSuite::DownloadState::DOWNLOAD_FAILED:
                status->cloudRomStatus = 22;  // AUDIOSUITE_CLOUD_ROM_STATUS_DOWNLOAD_FAIL
                break;
            case OHOS::AudioStandard::AudioSuite::DownloadState::DOWNLOAD_CANCELLED:
                status->cloudRomStatus = 0;  // AUDIOSUITE_CLOUD_ROM_STATUS_INIT
                break;
            default:
                status->cloudRomStatus = -1;  // AUDIOSUITE_CLOUD_ROM_STATUS_ERROR
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
