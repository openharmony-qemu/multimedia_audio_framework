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
#include <unordered_map>
#include <memory>
#include <string>
#include "native_audio_suite_download_manager.h"
#include "audio_suite_download_manager.h"

namespace OHOS {
namespace AudioStandard {

class OHAudioSuiteDownloadManager {
public:
    ~OHAudioSuiteDownloadManager();

    static OHAudioSuiteDownloadManager* GetInstance()
    {
        static OHAudioSuiteDownloadManager ohAudioSuiteDownloadManager;
        return &ohAudioSuiteDownloadManager;
    }

    int32_t StartDownload(const char *modelVersion);
    int32_t RegisterDownloadCallback(const OH_AudioSuite_DownloadCallback* callback);
    int32_t CancelDownload();
    int32_t StartBackgroundDownload(const char *modelVersion);
    int32_t GetDownloadStatus(OH_AudioSuite_DownloadStatusInfo *status, const char *modelVersion);
    int32_t UninstallCloudRom(const char *modelVersion);
    int32_t UnregisterDownloadCallback(const OH_AudioSuite_DownloadCallback* callback);
private:
    OHAudioSuiteDownloadManager();
};

namespace AudioSuite {

// 常量定义
constexpr int32_t STATUS_ARRAY_SIZE = 1;
constexpr int32_t CLOUD_ROM_STATUS_INIT = 0;
constexpr int32_t CLOUD_ROM_STATUS_DOWNLOADING = 20;
constexpr int32_t CLOUD_ROM_STATUS_DOWNLOAD_FAIL = 22;
constexpr int32_t CLOUD_ROM_STATUS_DOWNLOAD_SUCCESS = 23;
constexpr int32_t CLOUD_ROM_STATUS_ERROR = -1;

class CDownloadCallbackAdapter : public IDownloadCallbackListener {
public:
    explicit CDownloadCallbackAdapter(const OH_AudioSuite_DownloadCallback &cCallback)
        : callback_(cCallback)
    {
        statusArray_.size = STATUS_ARRAY_SIZE;
        statusArray_.downloadStatusInfo = &statusInfoPtr_;
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
                return CLOUD_ROM_STATUS_INIT;
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
#endif // OH_AUDIOSUITE_DOWNLOAD_MANAGER_H