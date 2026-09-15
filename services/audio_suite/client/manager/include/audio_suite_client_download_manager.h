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
#ifndef AUDIO_SUITE_CLIENT_DOWNLOAD_MANAGER_H
#define AUDIO_SUITE_CLIENT_DOWNLOAD_MANAGER_H

#include "audio_suite_download_manager.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

class AudioSuiteDownloadManagerImpl : public AudioSuiteDownloadManager {
public:
    AudioSuiteDownloadManagerImpl();
    ~AudioSuiteDownloadManagerImpl() override;

    AudioSuiteDownloadManagerImpl(const AudioSuiteDownloadManagerImpl&) = delete;
    AudioSuiteDownloadManagerImpl& operator=(const AudioSuiteDownloadManagerImpl&) = delete;

    int32_t RegisterDownloadStatusCallback(std::shared_ptr<DownloadStatusCallback> callback) override;
    int32_t UnregisterDownloadStatusCallback() override;
    int32_t GetInstallStatus(const std::vector<std::string>& featureNames,
        std::vector<bool>& installStatus) override;
    int32_t StartDownload(const std::vector<std::string>& featureNames, bool allowedCellularNet) override;
    int32_t PauseDownload(const std::vector<std::string>& featureNames) override;
    int32_t CancelDownload(const std::vector<std::string>& featureNames) override;
    int32_t StartBackgroundDownload(const std::vector<std::string>& featureNames) override;
    int32_t GetDownloadStatus(const std::vector<std::string>& featureNames,
        std::vector<AudioSuiteDownloadStatusInfo>& status) override;
    int32_t GetFeatureNewVersion(const std::vector<std::string>& featureNames,
        std::vector<AudioSuiteCloudRomVersionInfo>& versionInfos) override;
    int32_t UninstallCloudRom(const std::vector<std::string>& featureNames) override;

    int32_t StartDownloadDirect(const std::vector<std::string>& featureNames, bool allowedCellularNet) override;
    int32_t PauseDownloadDirect(const std::vector<std::string>& featureNames) override;
    int32_t CancelDownloadDirect(const std::vector<std::string>& featureNames) override;
    int32_t StartBackgroundDownloadDirect(const std::vector<std::string>& featureNames) override;
    int32_t GetDownloadStatusDirect(const std::vector<std::string>& featureNames,
        std::vector<AudioSuiteDownloadStatusInfo>& status) override;
    int32_t GetFeatureNewVersionDirect(const std::vector<std::string>& featureNames,
        std::vector<AudioSuiteCloudRomVersionInfo>& versionInfos) override;
    int32_t UninstallCloudRomDirect(const std::vector<std::string>& featureNames) override;

    int32_t RegisterDownloadInterfaceCallback(const std::shared_ptr<AudioSuiteRomDownloadCallback> &callback) override;
    int32_t UnregisterDownloadInterfaceCallback() override;
    void OnDownloadStatusChanged(const std::vector<AudioSuiteDownloadStatusInfo>& status) override;
};

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // AUDIO_SUITE_CLIENT_DOWNLOAD_MANAGER_H