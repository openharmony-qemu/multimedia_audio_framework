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

    int32_t RegisterDownloadCallback(std::shared_ptr<IDownloadCallbackListener> callback) override;
    int32_t UnregisterDownloadCallback() override;
    int32_t StartDownload(const std::string& version) override;
    int32_t CancelDownload() override;
    int32_t StartBackgroundDownload(const std::string& version) override;
    int32_t GetDownloadStatus(DownloadStatus& status, const std::string& version) override;
    int32_t UninstallCloudRom(const std::string& version) override;
    int32_t Init();
};

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // AUDIO_SUITE_CLIENT_DOWNLOAD_MANAGER_H