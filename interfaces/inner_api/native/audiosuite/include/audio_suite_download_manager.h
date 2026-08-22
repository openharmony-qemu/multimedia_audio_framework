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

#ifndef AUDIO_SUITE_DOWNLOAD_MANAGER_H
#define AUDIO_SUITE_DOWNLOAD_MANAGER_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include "audio_suite_types.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

class DownloadStatusCallback {
public:
    virtual ~DownloadStatusCallback() = default;
    virtual void OnDownloadStatusChanged(const std::vector<AudioSuiteDownloadStatusInfo>& status) = 0;
};

class AudioSuiteRomDownloadCallback {
public:
    AudioSuiteRomDownloadCallback() = default;
    virtual ~AudioSuiteRomDownloadCallback() = default;

    virtual int32_t GetFeatureNewVersion(
        const std::string featureName, AudioSuiteCloudRomVersionInfo &newfeatureNameInfos, int32_t &funcResult) = 0;
    virtual int32_t GetFeatureStatus(
        const std::string featureName, AudioSuiteDownloadStatusInfo &statusInfos, int32_t &funcResult) = 0;
    virtual int32_t DownloadAndInstall(
        const bool downloadCondition, const std::string featureName, int32_t &funcResult) = 0;
    virtual int32_t PauseCloudRom(const std::string featureName, int32_t &funcResult) = 0;
    virtual int32_t CancelCloudRom(const std::string featureName, int32_t &funcResult) = 0;
    virtual int32_t UninstallCloudRom(const std::string featureName, int32_t &funcResult) = 0;

    virtual int32_t RegisterDownloadStatusCallback(int32_t &funcResult) = 0;
    virtual int32_t UnregisterDownloadStatusCallback(int32_t &funcResult) = 0;
};

class AudioSuiteDownloadManager {
public:
    static AudioSuiteDownloadManager& GetInstance();
    virtual ~AudioSuiteDownloadManager() = default;

    // for ohaudiosuite
    virtual int32_t RegisterDownloadStatusCallback(std::shared_ptr<DownloadStatusCallback> callback) = 0;
    virtual int32_t UnregisterDownloadStatusCallback() = 0;
    virtual int32_t StartDownload(const std::string& featureName, int32_t networkType = 0) = 0;
    virtual int32_t PauseDownload(const std::string& featureName) = 0;
    virtual int32_t CancelDownload(const std::string& featureName) = 0;
    virtual int32_t StartBackgroundDownload(const std::string& featureName) = 0;
    virtual int32_t GetDownloadStatus(const std::string& featureName, AudioSuiteDownloadStatusInfo& status) = 0;
    virtual int32_t UninstallCloudRom(const std::string& featureName) = 0;

    // for audio server
    virtual int32_t RegisterDownloadInterfaceCallback(
        const std::shared_ptr<AudioSuiteRomDownloadCallback> &callback) = 0;
    virtual int32_t UnregisterDownloadInterfaceCallback() = 0;
    virtual void OnDownloadStatusChanged(const std::vector<AudioSuiteDownloadStatusInfo>& status) = 0;
};

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_SUITE_DOWNLOAD_MANAGER_H