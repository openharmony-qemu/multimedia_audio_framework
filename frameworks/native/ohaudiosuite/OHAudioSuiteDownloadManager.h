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

#ifndef OH_AUDIOSUITE_DOWNLOAD_MANAGER_H
#define OH_AUDIOSUITE_DOWNLOAD_MANAGER_H

#include <mutex>
#include <cstdint>
#include <memory>
#include <string>
#include "native_audio_suite_download_manager.h"
#include "audio_suite_download_manager.h"

namespace OHOS {
namespace AudioStandard {

class OHAudioSuiteDownloadCallback;

class OHAudioSuiteDownloadManager {
public:
    ~OHAudioSuiteDownloadManager();

    static OHAudioSuiteDownloadManager& GetInstance();
    int32_t RegisterDownloadStatusCallback(const OH_AudioSuite_DownloadCallback callback);
    int32_t UnregisterDownloadStatusCallback(const OH_AudioSuite_DownloadCallback callback);
    int32_t StartDownload(const char *featureName);
    int32_t PauseDownload(const char *featureName);
    int32_t CancelDownload(const char *featureName);
    int32_t StartBackgroundDownload(const char *featureName);
    int32_t GetDownloadStatus(const char *featureName, OH_AudioSuite_DownloadStatusInfo *status);
    int32_t UninstallCloudRom(const char *featureName);

    int32_t IsFeatureInstalled(const std::vector<std::string>& featureNames,
        bool *installStatus, uint32_t statusInfosSize);
    int32_t GetFeatureStatus(const std::vector<std::string>& featureNames,
        OH_AudioSuite_DownloadStatusInfo *statusInfos, uint32_t statusInfosSize);
    int32_t GetNewVersionInfo(const std::vector<std::string>& featureNames,
        OH_AudioSuite_FeatureVersionInfo *versionInfos, uint32_t statusInfosSize);

private:
    OHAudioSuiteDownloadManager();

    std::mutex downloadCbMutex_;
    std::shared_ptr<OHAudioSuiteDownloadCallback> downloadStatusCallback_ = nullptr;
};

} // namespace AudioStandard
} // namespace OHOS
#endif // OH_AUDIOSUITE_DOWNLOAD_MANAGER_H