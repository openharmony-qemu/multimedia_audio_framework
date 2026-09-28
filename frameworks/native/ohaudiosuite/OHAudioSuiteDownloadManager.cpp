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
static constexpr int32_t DOWNLOAD_SYSTEM_PERMISSION_DENIED = 202;
static constexpr int32_t DOWNLOAD_ERROR_NETWORK = 503;
static constexpr int32_t DOWNLOAD_ERROR_STORAGE_NOT_ENOUGH = 402;
}

static int32_t ConvertToApiResult(int32_t errCode, int32_t defaultResult)
{
    using namespace OHOS::AudioStandard;

    if (errCode == SUCCESS) {
        return AUDIOCOMMON_RESULT_SUCCESS;
    }
    if (errCode == ERR_INVALID_PARAM) {
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }
    if (errCode == ERR_SYSTEM_PERMISSION_DENIED) {
        return DOWNLOAD_SYSTEM_PERMISSION_DENIED;
    }
    if (errCode == DOWNLOAD_ERROR_NETWORK) {
        return AUDIOCOMMON_RESULT_ERROR_NETWORK;
    }
    if (errCode == DOWNLOAD_ERROR_STORAGE_NOT_ENOUGH) {
        return AUDIOCOMMON_RESULT_ERROR_STORAGE_NOT_ENOUGH;
    }

    return defaultResult;
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
    return ConvertToApiResult(ret, AUDIOCOMMON_RESULT_ERROR_SYSTEM);
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
    return ConvertToApiResult(ret, AUDIOCOMMON_RESULT_ERROR_SYSTEM);
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
    return ConvertToApiResult(ret, AUDIOCOMMON_RESULT_ERROR_SYSTEM);
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
    return ConvertToApiResult(ret, AUDIOCOMMON_RESULT_ERROR_SYSTEM);
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
    return ConvertToApiResult(ret, AUDIOCOMMON_RESULT_ERROR_SYSTEM);
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
    return ConvertToApiResult(ret, AUDIOCOMMON_RESULT_ERROR_SYSTEM);
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
    return ConvertToApiResult(ret, AUDIOCOMMON_RESULT_ERROR_SYSTEM);
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
    return ConvertToApiResult(ret, AUDIOCOMMON_RESULT_ERROR_SYSTEM);
}

static constexpr uint32_t DOWNLOAD_FEATURE_NAMES_MAX_COUNT = 5;

static bool ConvertFeatureNameArray(const char **featureNames, uint32_t count,
    std::vector<std::string>& feature)
{
    if (featureNames == nullptr) {
        AUDIO_WARNING_LOG("featureNames is nullptr");
        return false;
    }
    if (count == 0 || count > DOWNLOAD_FEATURE_NAMES_MAX_COUNT) {
        AUDIO_WARNING_LOG("featureNames count is %{public}u", count);
        return false;
    }
    for (uint32_t i = 0; i < count; i++) {
        if (featureNames[i] == nullptr) {
            AUDIO_WARNING_LOG("featureNames[%{public}u] is nullptr", i);
            return false;
        }
        if (strnlen(featureNames[i], DOWNLOAD_FEATURE_NAME_MAX_LEN) >= DOWNLOAD_FEATURE_NAME_MAX_LEN) {
            AUDIO_WARNING_LOG("featureNames[%{public}u] exceeds the maximum character length", i);
            return false;
        }
        feature.emplace_back(featureNames[i]);
    }
    return true;
}

int32_t OH_AudioSuite_IsFeatureInstalled(
    OH_AudioSuite_DownloadManager *downloadManager,
    const char **featureNames, uint32_t count,
    bool *installStatus, uint32_t installStatusSize)
{
    if (downloadManager == nullptr) {
        AUDIO_WARNING_LOG("IsFeatureInstalled downloadManager is nullptr");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }
    if (reinterpret_cast<OHAudioSuiteDownloadManager *>(downloadManager) !=
        &OHAudioSuiteDownloadManager::GetInstance()) {
        AUDIO_WARNING_LOG("IsFeatureInstalled downloadManager is invalid");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }
    if (installStatus == nullptr) {
        AUDIO_WARNING_LOG("IsFeatureInstalled installStatus is nullptr");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }
    if (installStatusSize < count) {
        AUDIO_WARNING_LOG("IsFeatureInstalled installStatusSize is less than count");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }
    std::vector<std::string> names;
    if (!ConvertFeatureNameArray(featureNames, count, names)) {
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }
    int32_t ret = OHAudioSuiteDownloadManager::GetInstance().IsFeatureInstalled(
        names, installStatus, installStatusSize);
    return ConvertToApiResult(ret, AUDIOCOMMON_RESULT_ERROR_SERVICE_DIED);
}

int32_t OH_AudioSuite_GetFeatureStatus(
    OH_AudioSuite_DownloadManager *downloadManager,
    const char **featureNames, uint32_t count,
    OH_AudioSuite_DownloadStatusInfo *statusInfos, uint32_t statusInfosSize)
{
    if (downloadManager == nullptr) {
        AUDIO_WARNING_LOG("GetFeatureStatus downloadManager is nullptr");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }
    if (reinterpret_cast<OHAudioSuiteDownloadManager *>(downloadManager) !=
        &OHAudioSuiteDownloadManager::GetInstance()) {
        AUDIO_WARNING_LOG("GetFeatureStatus downloadManager is invalid");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }
    if (statusInfos == nullptr) {
        AUDIO_WARNING_LOG("GetFeatureStatus statusInfos is nullptr");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }
    if (statusInfosSize < count) {
        AUDIO_WARNING_LOG("GetFeatureStatus statusInfosSize is less than count");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }
    std::vector<std::string> names;
    if (!ConvertFeatureNameArray(featureNames, count, names)) {
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }
    int32_t ret = OHAudioSuiteDownloadManager::GetInstance().GetFeatureStatus(
        names, statusInfos, statusInfosSize);
    return ConvertToApiResult(ret, AUDIOCOMMON_RESULT_ERROR_SERVICE_DIED);
}

int32_t OH_AudioSuite_GetNewVersionInfo(
    OH_AudioSuite_DownloadManager *downloadManager,
    const char **featureNames, uint32_t count,
    OH_AudioSuite_CloudRomVersionInfo *versionInfos, uint32_t versionInfosSize)
{
    if (downloadManager == nullptr) {
        AUDIO_WARNING_LOG("GetNewVersionInfo downloadManager is nullptr");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }
    if (reinterpret_cast<OHAudioSuiteDownloadManager *>(downloadManager) !=
        &OHAudioSuiteDownloadManager::GetInstance()) {
        AUDIO_WARNING_LOG("GetNewVersionInfo downloadManager is invalid");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }
    if (versionInfos == nullptr) {
        AUDIO_WARNING_LOG("GetNewVersionInfo versionInfos is nullptr");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }
    if (versionInfosSize < count) {
        AUDIO_WARNING_LOG("GetNewVersionInfo versionInfosSize is less than count");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }
    std::vector<std::string> names;
    if (!ConvertFeatureNameArray(featureNames, count, names)) {
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }
    int32_t ret = OHAudioSuiteDownloadManager::GetInstance().GetNewVersionInfo(
        names, versionInfos, versionInfosSize);
    return ConvertToApiResult(ret, AUDIOCOMMON_RESULT_ERROR_SERVICE_DIED);
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

int32_t OHAudioSuiteDownloadManager::IsFeatureInstalled(
    const std::vector<std::string>& featureNames, bool *installStatus, uint32_t installStatusSize)
{
    std::vector<bool> installStatusVec;
    int32_t ret = AudioSuite::AudioSuiteDownloadManager::GetInstance().GetInstallStatus(
        featureNames, installStatusVec);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "IsFeatureInstalled failed, ret = %{public}d", ret);

    uint32_t writeCount = static_cast<uint32_t>(installStatusVec.size()) > installStatusSize ?
        installStatusSize : static_cast<uint32_t>(installStatusVec.size());
    for (uint32_t i = 0; i < writeCount; i++) {
        installStatus[i] = installStatusVec[i];
    }
    return SUCCESS;
}

int32_t OHAudioSuiteDownloadManager::GetFeatureStatus(const std::vector<std::string>& featureNames,
    OH_AudioSuite_DownloadStatusInfo *statusInfos, uint32_t statusInfosSize)
{
    std::vector<AudioSuite::AudioSuiteDownloadStatusInfo> statusInfoVec;
    int32_t ret = AudioSuite::AudioSuiteDownloadManager::GetInstance().GetDownloadStatusDirect(
        featureNames, statusInfoVec);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetFeatureStatus failed, ret = %{public}d", ret);

    uint32_t writeCount = static_cast<uint32_t>(statusInfoVec.size() > statusInfosSize) ?
        statusInfosSize : static_cast<uint32_t>(statusInfoVec.size());
    for (uint32_t i = 0; i < writeCount; i++) {
        auto res = strncpy_s(statusInfos[i].featureName, sizeof(statusInfos[i].featureName),
            statusInfoVec[i].featureName.c_str(), statusInfoVec[i].featureName.length());
        CHECK_AND_RETURN_RET_LOG(res == EOK, ERROR, "strncpy_s featureName failed, err = %{public}d", res);
        res = strncpy_s(statusInfos[i].installPath, sizeof(statusInfos[i].installPath),
            statusInfoVec[i].romPath.c_str(), statusInfoVec[i].romPath.length());
        CHECK_AND_RETURN_RET_LOG(res == EOK, ERROR, "strncpy_s installPath failed, err = %{public}d", res);
        statusInfos[i].downloadStatus = static_cast<int32_t>(statusInfoVec[i].state);
        statusInfos[i].size = 0;
        statusInfos[i].progress = statusInfoVec[i].progress;
        statusInfos[i].errorCode = statusInfoVec[i].errorCode;
    }
    return SUCCESS;
}

int32_t OHAudioSuiteDownloadManager::GetNewVersionInfo(const std::vector<std::string>& featureNames,
    OH_AudioSuite_CloudRomVersionInfo *versionInfos, uint32_t versionInfosSize)
{
    std::vector<AudioSuite::AudioSuiteCloudRomVersionInfo> versionInfoVec;
    int32_t ret = AudioSuite::AudioSuiteDownloadManager::GetInstance().GetFeatureNewVersionDirect(
        featureNames, versionInfoVec);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetNewVersionInfo failed, ret = %{public}d", ret);

    uint32_t writeCount = static_cast<uint32_t>(versionInfoVec.size()) > versionInfosSize ?
        versionInfosSize : static_cast<uint32_t>(versionInfoVec.size());
    for (uint32_t i = 0; i < writeCount; i++) {
        auto res = strncpy_s(versionInfos[i].featureName, sizeof(versionInfos[i].featureName),
            versionInfoVec[i].featureName.c_str(), versionInfoVec[i].featureName.length());
        CHECK_AND_RETURN_RET_LOG(res == EOK, ERROR, "strncpy_s featureName failed, err = %{public}d", res);
        res = strncpy_s(versionInfos[i].version, sizeof(versionInfos[i].version),
            versionInfoVec[i].version.c_str(), versionInfoVec[i].version.length());
        CHECK_AND_RETURN_RET_LOG(res == EOK, ERROR, "strncpy_s version failed, err = %{public}d", res);
        versionInfos[i].downloadStatus = versionInfoVec[i].status;
        versionInfos[i].size = versionInfoVec[i].size;
        versionInfos[i].errorCode = versionInfoVec[i].errorCode;
    }
    return SUCCESS;
}

}  // namespace AudioStandard
}  // namespace OHOS
