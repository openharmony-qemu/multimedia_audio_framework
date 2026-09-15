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
#include <vector>
#include <thread>
#include "audio_utils.h"
#include "audio_errors.h"
#include "audio_common_log.h"
#include "OHAudioSuiteDownloadManager.h"
#include "audio_suite_download_manager.h"
#include "native_audio_common.h"
#include "securec.h"

using OHOS::AudioStandard::OHAudioSuiteDownloadManager;

namespace {
static constexpr size_t DOWNLOAD_FEATURE_NAME_MAX_LEN = 256;
static constexpr size_t DOWNLOAD_SYSTEM_PERMISSION_DENIED = 202;
}

static int32_t ConvertToSaResult(int32_t internalError)
{
    using namespace OHOS::AudioStandard;

    if (internalError == SUCCESS) {
        return AUDIOCOMMON_RESULT_SUCCESS;
    }
    if (internalError == ERR_INVALID_PARAM) {
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }
    if (internalError == ERR_SYSTEM_PERMISSION_DENIED) {
        return DOWNLOAD_SYSTEM_PERMISSION_DENIED;
    }
    if (internalError == ERR_ILLEGAL_STATE || internalError == ERR_NOT_STARTED) {
        return AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE;
    }
    if (internalError == ERR_OPERATION_FAILED || internalError == ERR_IPC) {
        return AUDIOCOMMON_RESULT_ERROR_SYSTEM;
    }

    return AUDIOCOMMON_RESULT_ERROR_SYSTEM;
}

int32_t OH_AudioSuite_GetDownloadManager(OH_AudioSuite_DownloadManager **downloadManager)
{
    CHECK_AND_RETURN_RET_LOG(downloadManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "GetDownloadManager downloadManager is nullptr");

    OHAudioSuiteDownloadManager &manager = OHAudioSuiteDownloadManager::GetInstance();
    *downloadManager = reinterpret_cast<OH_AudioSuite_DownloadManager *>(&manager);
    return AUDIOCOMMON_RESULT_SUCCESS;
}

int32_t OH_AudioSuite_RegisterDownloadCallback(
    OH_AudioSuite_DownloadManager* downloadManager, const OH_AudioSuite_DownloadCallback* callback)
{
    CHECK_AND_RETURN_RET_LOG(downloadManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "RegisterDownloadStatusCallback downloadManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(reinterpret_cast<OHAudioSuiteDownloadManager *>(downloadManager) ==
        &OHAudioSuiteDownloadManager::GetInstance(), AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM,
        "RegisterDownloadStatusCallback downloadManager is invalid");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "RegisterDownloadStatusCallback callback is nullptr");
    CHECK_AND_RETURN_RET_LOG(*callback != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "RegisterDownloadStatusCallback callback function is nullptr");

    int32_t ret = OHAudioSuiteDownloadManager::GetInstance().RegisterDownloadStatusCallback(*callback);
    return ConvertToSaResult(ret);
}

int32_t OH_AudioSuite_UnregisterDownloadCallback(
    OH_AudioSuite_DownloadManager* downloadManager, const OH_AudioSuite_DownloadCallback* callback)
{
    CHECK_AND_RETURN_RET_LOG(downloadManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "UnregisterDownloadStatusCallback downloadManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(reinterpret_cast<OHAudioSuiteDownloadManager *>(downloadManager) ==
        &OHAudioSuiteDownloadManager::GetInstance(), AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM,
        "UnregisterDownloadStatusCallback downloadManager is invalid");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "UnregisterDownloadStatusCallback callback is nullptr");
    CHECK_AND_RETURN_RET_LOG(*callback != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "RegisterDownloadStatusCallback callback function is nullptr");

    int32_t ret = OHAudioSuiteDownloadManager::GetInstance().UnregisterDownloadStatusCallback(*callback);
    return ConvertToSaResult(ret);
}

int32_t OH_AudioSuite_StartDownload(
    OH_AudioSuite_DownloadManager *downloadManager, const char *featureName)
{
    CHECK_AND_RETURN_RET_LOG(downloadManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "StartDownload downloadManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(reinterpret_cast<OHAudioSuiteDownloadManager *>(downloadManager) ==
        &OHAudioSuiteDownloadManager::GetInstance(), AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM,
        "StartDownload downloadManager is invalid");
    CHECK_AND_RETURN_RET_LOG(featureName != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "StartDownload featureName is nullptr");
    CHECK_AND_RETURN_RET_LOG(strnlen(featureName, DOWNLOAD_FEATURE_NAME_MAX_LEN) < DOWNLOAD_FEATURE_NAME_MAX_LEN,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "StartDownload featureName exceeds the maximum character length");

    int32_t ret = OHAudioSuiteDownloadManager::GetInstance().StartDownload(featureName);
    return ConvertToSaResult(ret);
}

int32_t OH_AudioSuite_PauseDownload(OH_AudioSuite_DownloadManager *downloadManager, const char *featureName)
{
    CHECK_AND_RETURN_RET_LOG(downloadManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "PauseDownload downloadManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(reinterpret_cast<OHAudioSuiteDownloadManager *>(downloadManager) ==
        &OHAudioSuiteDownloadManager::GetInstance(), AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM,
        "PauseDownload downloadManager is invalid");
    CHECK_AND_RETURN_RET_LOG(featureName != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "PauseDownload featureName is nullptr");
    CHECK_AND_RETURN_RET_LOG(strnlen(featureName, DOWNLOAD_FEATURE_NAME_MAX_LEN) < DOWNLOAD_FEATURE_NAME_MAX_LEN,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "PauseDownload featureName exceeds the maximum character length");

    int32_t ret = OHAudioSuiteDownloadManager::GetInstance().PauseDownload(featureName);
    return ConvertToSaResult(ret);
}

int32_t OH_AudioSuite_CancelDownload(OH_AudioSuite_DownloadManager *downloadManager, const char *featureName)
{
    CHECK_AND_RETURN_RET_LOG(downloadManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "CancelDownload downloadManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(reinterpret_cast<OHAudioSuiteDownloadManager *>(downloadManager) ==
        &OHAudioSuiteDownloadManager::GetInstance(), AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM,
        "CancelDownload downloadManager is invalid");
    CHECK_AND_RETURN_RET_LOG(featureName != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "CancelDownload featureName is nullptr");
    CHECK_AND_RETURN_RET_LOG(strnlen(featureName, DOWNLOAD_FEATURE_NAME_MAX_LEN) < DOWNLOAD_FEATURE_NAME_MAX_LEN,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "CancelDownload featureName exceeds the maximum character length");

    int32_t ret = OHAudioSuiteDownloadManager::GetInstance().CancelDownload(featureName);
    return ConvertToSaResult(ret);
}

int32_t OH_AudioSuite_StartBackgroundDownload(
    OH_AudioSuite_DownloadManager *downloadManager, const char *featureName)
{
    CHECK_AND_RETURN_RET_LOG(downloadManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "StartBackgroundDownload downloadManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(reinterpret_cast<OHAudioSuiteDownloadManager *>(downloadManager) ==
        &OHAudioSuiteDownloadManager::GetInstance(), AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM,
        "StartBackgroundDownload downloadManager is invalid");
    CHECK_AND_RETURN_RET_LOG(featureName != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "StartBackgroundDownload featureName is nullptr");
    CHECK_AND_RETURN_RET_LOG(strnlen(featureName, DOWNLOAD_FEATURE_NAME_MAX_LEN) < DOWNLOAD_FEATURE_NAME_MAX_LEN,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM,
        "StartBackgroundDownload featureName exceeds the maximum character length");

    int32_t ret = OHAudioSuiteDownloadManager::GetInstance().StartBackgroundDownload(featureName);
    return ConvertToSaResult(ret);
}

int32_t OH_AudioSuite_GetDownloadStatus(
    OH_AudioSuite_DownloadManager *downloadManager, const char *featureName, OH_AudioSuite_DownloadStatusInfo *status)
{
    CHECK_AND_RETURN_RET_LOG(downloadManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "GetDownloadStatus downloadManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(reinterpret_cast<OHAudioSuiteDownloadManager *>(downloadManager) ==
        &OHAudioSuiteDownloadManager::GetInstance(), AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM,
        "GetDownloadStatus downloadManager is invalid");
    CHECK_AND_RETURN_RET_LOG(featureName != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "GetDownloadStatus featureName is nullptr");
    CHECK_AND_RETURN_RET_LOG(strnlen(featureName, DOWNLOAD_FEATURE_NAME_MAX_LEN) < DOWNLOAD_FEATURE_NAME_MAX_LEN,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "GetDownloadStatus featureName exceeds the maximum character length");
    CHECK_AND_RETURN_RET_LOG(status != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "GetDownloadStatus status is nullptr");

    int32_t ret = OHAudioSuiteDownloadManager::GetInstance().GetDownloadStatus(featureName, status);
    return ConvertToSaResult(ret);
}

int32_t OH_AudioSuite_UninstallCloudRom(OH_AudioSuite_DownloadManager *downloadManager, const char *featureName)
{
    CHECK_AND_RETURN_RET_LOG(downloadManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "UninstallCloudRom downloadManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(reinterpret_cast<OHAudioSuiteDownloadManager *>(downloadManager) ==
        &OHAudioSuiteDownloadManager::GetInstance(), AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM,
        "UninstallCloudRom downloadManager is invalid");
    CHECK_AND_RETURN_RET_LOG(featureName != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "UninstallCloudRom featureName is nullptr");
    CHECK_AND_RETURN_RET_LOG(strnlen(featureName, DOWNLOAD_FEATURE_NAME_MAX_LEN) < DOWNLOAD_FEATURE_NAME_MAX_LEN,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "UninstallCloudRom featureName exceeds the maximum character length");

    int32_t ret = OHAudioSuiteDownloadManager::GetInstance().UninstallCloudRom(featureName);
    return ConvertToSaResult(ret);
}

namespace OHOS {
namespace AudioStandard {

namespace {
static constexpr size_t STATUS_ARRAY_MAX_SZIE = 128;
}

class OHAudioSuiteDownloadCallback : public AudioSuite::DownloadStatusCallback {
public:
    explicit OHAudioSuiteDownloadCallback(OH_AudioSuite_DownloadCallback callback)
        : callback_(callback)
    {
        AUDIO_INFO_LOG("OHAudioSuiteDownloadCallback created!");
    }

    void OnDownloadStatusChanged(const std::vector<AudioSuite::AudioSuiteDownloadStatusInfo>& status) override
    {
        OH_AudioSuite_DownloadStatusInfoArray statusArray;
        statusArray.size = status.size() > STATUS_ARRAY_MAX_SZIE ? STATUS_ARRAY_MAX_SZIE : status.size();

        statusArray.downloadStatusInfo = (OH_AudioSuite_DownloadStatusInfo **)malloc(
            sizeof(OH_AudioSuite_DownloadStatusInfo *) * statusArray.size);
        CHECK_AND_RETURN_LOG(statusArray.downloadStatusInfo != nullptr,
            "OnDownloadStatusChanged malloc failed, size:%{public}u", statusArray.size);
        bool err = false;
        uint32_t freeIdx = statusArray.size;
        for (uint32_t idx = 0; idx < statusArray.size; idx++) {
            auto info = (OH_AudioSuite_DownloadStatusInfo *)malloc(sizeof(OH_AudioSuite_DownloadStatusInfo));
            if (info == nullptr) {
                AUDIO_WARNING_LOG("OnDownloadStatusChanged malloc failed!");
                err = true;
                freeIdx = idx;
                break;
            }
            statusArray.downloadStatusInfo[idx] = info;

            auto resFeature = strncpy_s(info->featureName, sizeof(info->featureName),
                status[idx].featureName.c_str(), status[idx].featureName.length());
            auto resPath = strncpy_s(info->installPath, sizeof(info->installPath),
                status[idx].romPath.c_str(), status[idx].romPath.length());
            if (resFeature != EOK || resPath != EOK) {
                AUDIO_WARNING_LOG("strncpy_s failed res:%{public}d, Path:%{public}d", resFeature, resPath);
                err = true;
                freeIdx = idx + 1;
                break;
            }
            info->downloadStatus = static_cast<int32_t>(status[idx].state);
            info->size = status[idx].size;
            info->progress = status[idx].progress;
            info->errorCode = status[idx].errorCode;
        }

        if (!err) {
            callback_(&statusArray);
        }

        for (uint32_t idx = 0; idx < freeIdx; idx++) {
            free(statusArray.downloadStatusInfo[idx]);
        }
        free(statusArray.downloadStatusInfo);
    }

    OH_AudioSuite_DownloadCallback GetCallback()
    {
        return callback_;
    }

    ~OHAudioSuiteDownloadCallback()
    {
        AUDIO_INFO_LOG("~OHAudioSuiteDownloadCallback called.");
        callback_ = nullptr;
    }

private:
    OH_AudioSuite_DownloadCallback callback_;
};

OHAudioSuiteDownloadManager::OHAudioSuiteDownloadManager()
{
    AUDIO_INFO_LOG("OHAudioSuiteDownloadManager created!");
}

OHAudioSuiteDownloadManager::~OHAudioSuiteDownloadManager()
{
    AUDIO_INFO_LOG("OHAudioSuiteDownloadManager destroyed!");
}

OHAudioSuiteDownloadManager& OHAudioSuiteDownloadManager::GetInstance()
{
    static OHAudioSuiteDownloadManager ohAudioSuiteDownloadManager;
    return ohAudioSuiteDownloadManager;
}

int32_t OHAudioSuiteDownloadManager::RegisterDownloadStatusCallback(const OH_AudioSuite_DownloadCallback callback)
{
    std::lock_guard<std::mutex> lock(downloadCbMutex_);
    downloadStatusCallback_ = std::make_shared<OHAudioSuiteDownloadCallback>(callback);
    return AudioSuite::AudioSuiteDownloadManager::GetInstance().RegisterDownloadStatusCallback(downloadStatusCallback_);
}

int32_t OHAudioSuiteDownloadManager::UnregisterDownloadStatusCallback(const OH_AudioSuite_DownloadCallback callback)
{
    std::lock_guard<std::mutex> lock(downloadCbMutex_);
    CHECK_AND_RETURN_RET_LOG(downloadStatusCallback_ != nullptr, SUCCESS, "AudioSuiteDownload not register callback");
    CHECK_AND_RETURN_RET_LOG(downloadStatusCallback_->GetCallback() == callback, ERR_INVALID_PARAM,
        "AudioSuiteDownload UnregisterDownloadStatusCallback failed, not same callback");

    int32_t ret = AudioSuite::AudioSuiteDownloadManager::GetInstance().UnregisterDownloadStatusCallback();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "AudioSuiteDownload unregister failed, ret = %{public}d", ret);
    downloadStatusCallback_ = nullptr;
    return SUCCESS;
}

int32_t OHAudioSuiteDownloadManager::StartDownload(const char *featureName)
{
    std::vector<std::string> featureNames = {std::string(featureName)};
    return AudioSuite::AudioSuiteDownloadManager::GetInstance().StartDownload(featureNames, true);
}

int32_t OHAudioSuiteDownloadManager::PauseDownload(const char *featureName)
{
    std::vector<std::string> featureNames = {std::string(featureName)};
    return AudioSuite::AudioSuiteDownloadManager::GetInstance().PauseDownload(featureNames);
}

int32_t OHAudioSuiteDownloadManager::CancelDownload(const char *featureName)
{
    std::vector<std::string> featureNames = {std::string(featureName)};
    return AudioSuite::AudioSuiteDownloadManager::GetInstance().CancelDownload(featureNames);
}

int32_t OHAudioSuiteDownloadManager::StartBackgroundDownload(const char *featureName)
{
    std::vector<std::string> featureNames = {std::string(featureName)};
    return AudioSuite::AudioSuiteDownloadManager::GetInstance().StartBackgroundDownload(featureNames);
}

int32_t OHAudioSuiteDownloadManager::UninstallCloudRom(const char *featureName)
{
    std::vector<std::string> featureNames = {std::string(featureName)};
    return AudioSuite::AudioSuiteDownloadManager::GetInstance().UninstallCloudRom(featureNames);
}

int32_t OHAudioSuiteDownloadManager::GetDownloadStatus(const char *featureName,
                                                       OH_AudioSuite_DownloadStatusInfo *status)
{
    std::vector<std::string> featureNames = {std::string(featureName)};
    std::vector<AudioSuite::AudioSuiteDownloadStatusInfo> statusInfos;
    int32_t ret = AudioSuite::AudioSuiteDownloadManager::GetInstance().GetDownloadStatus(featureNames, statusInfos);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetDownloadStatus failed, ret = %{public}d", ret);
    CHECK_AND_RETURN_RET_LOG(!statusInfos.empty(), ERROR, "GetDownloadStatus returned empty status");
    const auto& audioSuiteStatus = statusInfos[0];

    auto res = strncpy_s(status->featureName,
                         sizeof(status->featureName),
                         audioSuiteStatus.featureName.c_str(),
                         audioSuiteStatus.featureName.length());
    CHECK_AND_RETURN_RET_LOG(res == EOK, ERROR, "strncpy_s failed, err = %{public}d", res);
    res = strncpy_s(status->installPath,
                    sizeof(status->installPath),
                    audioSuiteStatus.romPath.c_str(),
                    audioSuiteStatus.romPath.length());
    CHECK_AND_RETURN_RET_LOG(res == EOK, ERROR, "strncpy_s failed, err = %{public}d", res);
    status->downloadStatus = static_cast<int32_t>(audioSuiteStatus.state);
    status->size = audioSuiteStatus.size;
    status->progress = audioSuiteStatus.progress;
    status->errorCode = audioSuiteStatus.errorCode;
    return SUCCESS;
}

}  // namespace AudioStandard
}  // namespace OHOS
