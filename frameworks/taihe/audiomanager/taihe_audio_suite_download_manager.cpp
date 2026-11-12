/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "TaiheAudioSuiteDownloadManager"
#endif

#include "taihe_audio_suite_download_manager.h"
#include "taihe_audio_error.h"
#include "taihe_param_utils.h"
#include "audio_log.h"
#include "audio_errors.h"

namespace ANI::Audio {

using OHOS::AudioStandard::AudioSuite::AudioSuiteDownloadStatusInfo;
using OHOS::AudioStandard::AudioSuite::AudioSuiteCloudRomVersionInfo;
using NativeAudioSuiteDownloadManager = OHOS::AudioStandard::AudioSuite::AudioSuiteDownloadManager;

static constexpr int32_t DOWNLOAD_MEMORY_NOT_ENOUGH = 402;
static constexpr int32_t DOWNLOAD_NET_ERROR = 503;

static std::string FeatureTypeToName(int32_t featureType)
{
    using FeatureType = AudioSuiteFeatureType::key_t;
    static const std::map<FeatureType, std::string> featureTypeToNameMap = {
        {FeatureType::AUDIO_DIALOG_ENHANCEMENT, "RecordDialogAlgoRes"},
        {FeatureType::AUDIO_CENTER_VOICE_ENHANCEMENT, "RecordPicAlgoRes"},
        {FeatureType::AUDIO_VOICE_ENHANCEMENT, "RecordDefinition"},
        {FeatureType::AUDIO_MUSIC_ENHANCEMENT, "MusicMasterAlgoRes"},
        {FeatureType::AUDIO_SOURCE_SEPARATION, "AudioSourceSeparationAlgoRes"},
    };
    auto it = featureTypeToNameMap.find(static_cast<FeatureType>(featureType));
    return (it != featureTypeToNameMap.end()) ? it->second : "";
}

static int32_t FeatureNameToType(const std::string &featureName)
{
    using FeatureType = AudioSuiteFeatureType::key_t;
    static const std::map<std::string, FeatureType> featureNameToTypeMap = {
        {"RecordDialogAlgoRes", FeatureType::AUDIO_DIALOG_ENHANCEMENT},
        {"RecordPicAlgoRes", FeatureType::AUDIO_CENTER_VOICE_ENHANCEMENT},
        {"RecordDefinition", FeatureType::AUDIO_VOICE_ENHANCEMENT},
        {"MusicMasterAlgoRes", FeatureType::AUDIO_MUSIC_ENHANCEMENT},
        {"AudioSourceSeparationAlgoRes", FeatureType::AUDIO_SOURCE_SEPARATION},
    };
    auto it = featureNameToTypeMap.find(featureName);
    return (it != featureNameToTypeMap.end()) ? static_cast<int32_t>(it->second) : -1;
}

static int32_t ConvertNativeErrorToTaihe(int32_t nativeError)
{
    if (nativeError == OHOS::AudioStandard::SUCCESS) {
        return 0;
    }
    if (nativeError == OHOS::AudioStandard::ERR_INVALID_PARAM) {
        return TAIHE_ERR_INVALID_PARAM;
    }
    if (nativeError == OHOS::AudioStandard::ERR_SYSTEM_PERMISSION_DENIED) {
        return TAIHE_ERR_PERMISSION_DENIED;
    }
    if (nativeError == DOWNLOAD_MEMORY_NOT_ENOUGH) {
        return TAIHE_ERR_STORAGE_NOT_ENOUGH;
    }
    if (nativeError == DOWNLOAD_NET_ERROR) {
        return TAIHE_ERR_NETWORK_CONDITION_NOT_MET;
    }
    return TAIHE_ERR_SERVICE_DIED;
}

static bool ConvertFeatureTypesToNames(
    array_view<AudioSuiteFeatureType> featureTypes, std::vector<std::string> &featureNames)
{
    for (size_t i = 0; i < featureTypes.size(); i++) {
        int32_t type = featureTypes[i].get_value();
        std::string name = FeatureTypeToName(type);
        if (name.empty()) {
            return false;
        }
        featureNames.push_back(name);
    }
    return !featureNames.empty();
}


AudioSuiteDownloadManagerImpl::AudioSuiteDownloadManagerImpl() {}

AudioSuiteDownloadManagerImpl::AudioSuiteDownloadManagerImpl(std::shared_ptr<AudioSuiteDownloadManagerImpl> obj)
{
}

AudioSuiteDownloadManagerImpl::~AudioSuiteDownloadManagerImpl() = default;

AudioSuiteDownloadManager AudioSuiteDownloadManagerImpl::CreateSuiteDownloadManagerWrapper()
{
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return make_holder<AudioSuiteDownloadManagerImpl, AudioSuiteDownloadManager>(nullptr);
    }
    auto impl = std::make_shared<AudioSuiteDownloadManagerImpl>();
    if (impl != nullptr) {
        AUDIO_INFO_LOG("createSuiteDownloadManager success");
        return make_holder<AudioSuiteDownloadManagerImpl, AudioSuiteDownloadManager>(impl);
    }
    TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "CreateSuiteDownloadManager failed");
    return make_holder<AudioSuiteDownloadManagerImpl, AudioSuiteDownloadManager>(nullptr);
}

void AudioSuiteDownloadManagerImpl::StartDownloadSync(
    array_view<AudioSuiteFeatureType> featureTypes, NetworkType networkType)
{
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return;
    }
    auto networkTypeKey = networkType.get_key();
    if (networkTypeKey != NetworkType::key_t::WIFI && networkTypeKey != NetworkType::key_t::CELLULAR_AND_WIFI) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "invalid networkType");
        return;
    }
    std::vector<std::string> featureNames;
    if (!ConvertFeatureTypesToNames(featureTypes, featureNames)) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "invalid featureTypes");
        return;
    }
    bool allowCellular = (networkTypeKey == NetworkType::key_t::CELLULAR_AND_WIFI);
    int32_t ret = NativeAudioSuiteDownloadManager::GetInstance().StartDownloadDirect(featureNames, allowCellular);
    if (ret != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(ConvertNativeErrorToTaihe(ret), "startDownload failed");
        return;
    }
    AUDIO_INFO_LOG("startDownload success");
}

void AudioSuiteDownloadManagerImpl::PauseDownloadSync(array_view<AudioSuiteFeatureType> featureTypes)
{
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return;
    }
    std::vector<std::string> featureNames;
    if (!ConvertFeatureTypesToNames(featureTypes, featureNames)) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "invalid featureTypes");
        return;
    }
    int32_t ret = NativeAudioSuiteDownloadManager::GetInstance().PauseDownloadDirect(featureNames);
    if (ret != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(ConvertNativeErrorToTaihe(ret), "pauseDownload failed");
        return;
    }
    AUDIO_INFO_LOG("pauseDownload success");
}

void AudioSuiteDownloadManagerImpl::CancelDownloadSync(array_view<AudioSuiteFeatureType> featureTypes)
{
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return;
    }
    std::vector<std::string> featureNames;
    if (!ConvertFeatureTypesToNames(featureTypes, featureNames)) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "invalid featureTypes");
        return;
    }
    int32_t ret = NativeAudioSuiteDownloadManager::GetInstance().CancelDownloadDirect(featureNames);
    if (ret != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(ConvertNativeErrorToTaihe(ret), "cancelDownload failed");
        return;
    }
    AUDIO_INFO_LOG("cancelDownload success");
}

void AudioSuiteDownloadManagerImpl::StartBackgroundDownloadSync(array_view<AudioSuiteFeatureType> featureTypes)
{
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return;
    }
    std::vector<std::string> featureNames;
    if (!ConvertFeatureTypesToNames(featureTypes, featureNames)) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "invalid featureTypes");
        return;
    }
    int32_t ret = NativeAudioSuiteDownloadManager::GetInstance().StartBackgroundDownloadDirect(featureNames);
    if (ret != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(ConvertNativeErrorToTaihe(ret), "startBackgroundDownload failed");
        return;
    }
    AUDIO_INFO_LOG("startBackgroundDownload success");
}

array<AudioSuiteFeatureStatusInfo> AudioSuiteDownloadManagerImpl::GetFeatureStatusSync(
    array_view<AudioSuiteFeatureType> featureTypes)
{
    std::vector<AudioSuiteFeatureStatusInfo> emptyResult;
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return array<AudioSuiteFeatureStatusInfo>(emptyResult);
    }
    std::vector<std::string> featureNames;
    if (!ConvertFeatureTypesToNames(featureTypes, featureNames)) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "invalid featureTypes");
        return array<AudioSuiteFeatureStatusInfo>(emptyResult);
    }
    std::vector<AudioSuiteDownloadStatusInfo> statusInfos;
    int32_t ret = NativeAudioSuiteDownloadManager::GetInstance().GetDownloadStatusDirect(featureNames, statusInfos);
    if (ret != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(ConvertNativeErrorToTaihe(ret), "getFeatureStatus failed");
        return array<AudioSuiteFeatureStatusInfo>(emptyResult);
    }
    std::vector<AudioSuiteFeatureStatusInfo> result;
    for (const auto &info : statusInfos) {
        if (info.featureName != "") {
            AudioSuiteFeatureStatusInfo taiheInfo {
                AudioSuiteFeatureType::from_value(FeatureNameToType(info.featureName)),
                AudioSuiteFeatureStatus::from_value(info.state),
                TaiheParamUtils::ToTaiheString(info.romPath),
                info.progress,
                info.errorCode
            };
            result.emplace_back(taiheInfo);
        }
    }
    AUDIO_INFO_LOG("getFeatureStatus success, size:%{public}zu", result.size());
    return array<AudioSuiteFeatureStatusInfo>(result);
}

void AudioSuiteDownloadManagerImpl::UninstallFeatureSync(array_view<AudioSuiteFeatureType> featureTypes)
{
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return;
    }
    std::vector<std::string> featureNames;
    if (!ConvertFeatureTypesToNames(featureTypes, featureNames)) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "invalid featureTypes");
        return;
    }
    int32_t ret = NativeAudioSuiteDownloadManager::GetInstance().UninstallCloudRomDirect(featureNames);
    if (ret != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(ConvertNativeErrorToTaihe(ret), "uninstallFeature failed");
        return;
    }
    AUDIO_INFO_LOG("uninstallFeature success");
}

array<bool> AudioSuiteDownloadManagerImpl::IsFeatureInstalledSync(array_view<AudioSuiteFeatureType> featureTypes)
{
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return array<bool>(nullptr, 0);
    }
    std::vector<std::string> featureNames;
    if (!ConvertFeatureTypesToNames(featureTypes, featureNames)) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "invalid featureTypes");
        return array<bool>(nullptr, 0);
    }
    std::vector<bool> installStatus;
    int32_t ret = NativeAudioSuiteDownloadManager::GetInstance().GetInstallStatus(featureNames, installStatus);
    if (ret != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(ConvertNativeErrorToTaihe(ret), "isFeatureInstalled failed");
        return array<bool>(nullptr, 0);
    }
    array<bool> result(installStatus.size());
    for (size_t i = 0; i < installStatus.size(); i++) {
        result[i] = installStatus[i];
    }
    AUDIO_INFO_LOG("isFeatureInstalled success, size:%{public}zu", installStatus.size());
    return result;
}

array<AudioSuiteFeatureVersionInfo> AudioSuiteDownloadManagerImpl::GetNewVersionInfoSync(
    array_view<AudioSuiteFeatureType> featureTypes)
{
    std::vector<AudioSuiteFeatureVersionInfo> emptyResult;
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return array<AudioSuiteFeatureVersionInfo>(emptyResult);
    }
    std::vector<std::string> featureNames;
    if (!ConvertFeatureTypesToNames(featureTypes, featureNames)) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "invalid featureTypes");
        return array<AudioSuiteFeatureVersionInfo>(emptyResult);
    }
    std::vector<AudioSuiteCloudRomVersionInfo> versionInfos;
    int32_t ret = NativeAudioSuiteDownloadManager::GetInstance().GetFeatureNewVersionDirect(featureNames, versionInfos);
    if (ret != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(ConvertNativeErrorToTaihe(ret), "getNewVersionInfo failed");
        return array<AudioSuiteFeatureVersionInfo>(emptyResult);
    }
    std::vector<AudioSuiteFeatureVersionInfo> result;
    for (const auto &info : versionInfos) {
        if (info.featureName != "") {
            AudioSuiteFeatureVersionInfo taiheInfo {
                AudioSuiteFeatureType::from_value(FeatureNameToType(info.featureName)),
                AudioSuiteFeatureStatus::from_value(info.status),
                TaiheParamUtils::ToTaiheString(info.version),
                info.size,
                info.errorCode
            };
            result.emplace_back(taiheInfo);
        }
    }
    AUDIO_INFO_LOG("getNewVersionInfo success, size:%{public}zu", result.size());
    return array<AudioSuiteFeatureVersionInfo>(result);
}

void AudioSuiteDownloadManagerImpl::OnDownloadStatusChange(
    callback_view<void(array_view<AudioSuiteFeatureStatusInfo> data)> callback)
{
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return;
    }
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterDownloadStatusCallback(cacheCallback, this);
    AUDIO_INFO_LOG("onDownloadStatusChange success");
}

void AudioSuiteDownloadManagerImpl::RegisterDownloadStatusCallback(
    std::shared_ptr<uintptr_t> &callback, AudioSuiteDownloadManagerImpl *impl)
{
    CHECK_AND_RETURN_RET_LOG(impl != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "impl is nullptr");
    std::lock_guard<std::mutex> lock(impl->downloadCbMutex_);

    if (impl->downloadCallback_ == nullptr) {
        auto downloadCallback = std::make_shared<TaiheAudioSuiteDownloadCallback>();
        CHECK_AND_RETURN_LOG(downloadCallback != nullptr, "no memory");
        int32_t ret = NativeAudioSuiteDownloadManager::GetInstance().RegisterDownloadStatusCallback(downloadCallback);
        if (ret != OHOS::AudioStandard::SUCCESS) {
            AUDIO_ERR_LOG("RegisterDownloadStatusCallback Failed %{public}d", ret);
            TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "Registering callback failed");
            return;
        }
        impl->downloadCallback_ = downloadCallback;
    }

    if (impl->downloadCallback_->ContainSameCallback(callback)) {
        AUDIO_DEBUG_LOG("has same callback, nothing to do");
        return;
    }
    impl->downloadCallback_->SaveCallbackReference(callback);
}

void AudioSuiteDownloadManagerImpl::OffDownloadStatusChange(
    optional_view<callback<void(array_view<AudioSuiteFeatureStatusInfo> data)>> callback)
{
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return;
    }
    std::shared_ptr<uintptr_t> cacheCallback = nullptr;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterDownloadStatusCallback(cacheCallback, this);
    AUDIO_INFO_LOG("offDownloadStatusChange success");
}

void AudioSuiteDownloadManagerImpl::UnregisterDownloadStatusCallback(
    std::shared_ptr<uintptr_t> &callback, AudioSuiteDownloadManagerImpl *impl)
{
    CHECK_AND_RETURN_RET_LOG(impl != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "impl is nullptr");
    std::lock_guard<std::mutex> lock(impl->downloadCbMutex_);
    CHECK_AND_RETURN_RET_LOG(impl->downloadCallback_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM), "downloadCallback_ is nullptr");

    if (callback != nullptr) {
        impl->downloadCallback_->RemoveCallbackReference(callback);
    } else {
        impl->downloadCallback_->RemoveAllCallbackReference();
    }
    if (impl->downloadCallback_->IsEmpty()) {
        int32_t ret = NativeAudioSuiteDownloadManager::GetInstance().UnregisterDownloadStatusCallback();
        CHECK_AND_RETURN_LOG(ret == OHOS::AudioStandard::SUCCESS, "UnregisterDownloadStatusCallback Failed");
        impl->downloadCallback_ = nullptr;
    }
}
} // namespace ANI::Audio
