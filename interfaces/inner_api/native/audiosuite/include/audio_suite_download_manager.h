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
#include "audio_stream_info.h"
#include "audio_suite_sa_types.h"
#include "idownload_callback.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

using SaDiedCallback = std::function<void()>;

class IDownloadCallbackListener {
public:
    virtual ~IDownloadCallbackListener() = default;
    virtual void OnDownloadStatusChanged(const DownloadStatus& status) = 0;
};

class AudioSuiteDownloadManager {
public:
    virtual ~AudioSuiteDownloadManager() = default;

    static AudioSuiteDownloadManager& GetInstance();

    virtual int32_t RegisterDownloadCallback(std::shared_ptr<IDownloadCallbackListener> callback) = 0;
    virtual int32_t StartDownload(const std::string& version) = 0;
    virtual int32_t CancelDownload() = 0;
    virtual int32_t StartBackgroundDownload(const std::string& version) = 0;
    virtual int32_t GetDownloadStatus(DownloadStatus& status, const std::string& version) = 0;
    virtual int32_t UninstallCloudRom(const std::string& version) = 0;
    virtual int32_t UnregisterDownloadCallback() = 0;
};

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_SUITE_DOWNLOAD_MANAGER_H