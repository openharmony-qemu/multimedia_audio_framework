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

#ifndef DOWNLOAD_MANAGER_H
#define DOWNLOAD_MANAGER_H

#include <string>
#include <memory>
#include <mutex>
#include <atomic>
#include <thread>
#include <vector>
#include <sstream>
#include <condition_variable>
#include "singleton.h"
#include "iremote_object.h"
#include "nocopyable.h"
#include "audio_suite_types.h"
#include "syspara/parameters.h"
#include "background_download.h"
#include "iaudio_suite_status_callback.h"
#include "iaudio_suite_download_callback.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

class DownloadManager {
    DECLARE_SINGLETON(DownloadManager)

public:
    int32_t RegisterDownloadStatusCallback(const int32_t callerPid, const sptr<IRemoteObject>& object);
    int32_t UnregisterDownloadStatusCallback(const int32_t callerPid);
    int32_t GetInstallStatus(const std::vector<std::string>& featureNames,
        std::vector<bool>& installStatus);
    int32_t StartDownload(const std::vector<std::string>& featureNames, bool allowedCellularNet);
    int32_t PauseDownload(const std::vector<std::string>& featureNames);
    int32_t CancelDownload(const std::vector<std::string>& featureNames);
    int32_t StartBackgroundDownload(const std::vector<std::string>& featureNames);
    int32_t GetDownloadStatus(const std::vector<std::string>& featureNames,
        std::vector<AudioSuiteDownloadStatusInfo>& status);
    int32_t GetFeatureStatus(const std::vector<std::string>& featureNames,
        std::vector<AudioSuiteDownloadStatusInfo>& status);
    int32_t GetFeatureNewVersion(const std::vector<std::string>& featureNames,
        std::vector<AudioSuiteCloudRomVersionInfo>& versionInfos);
    int32_t UninstallCloudRom(const std::vector<std::string>& featureNames);

    int32_t SetUpdateEngineCallback(const sptr<IRemoteObject> object);
    int32_t OnDownloadStatusChanged(const std::vector<AudioSuiteDownloadStatusInfo>& status);

    int32_t HasDownloadingTask();

private:
    bool WaitForCallbackRegistration();
    bool ValidateFeatureNames(const std::vector<std::string>& featureNames);
    bool ValidateFeatureNameSafe(const std::vector<std::string>& featureNames);
    int32_t QueuePendingFeatures(const std::vector<std::string>& pendingFeatures);
    int32_t SearchAndDownload(const std::vector<std::string>& featureNames);

    class AudioSuiteClientDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit AudioSuiteClientDeathRecipient(int32_t callerPid);
        virtual ~AudioSuiteClientDeathRecipient() = default;
        DISALLOW_COPY_AND_MOVE(AudioSuiteClientDeathRecipient);
        void OnRemoteDied(const wptr<IRemoteObject> &remote);

    private:
        const int32_t callerPid_;
    };

    class AudioServerDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit AudioServerDeathRecipient() = default;
        virtual ~AudioServerDeathRecipient() = default;
        DISALLOW_COPY_AND_MOVE(AudioServerDeathRecipient);
        void OnRemoteDied(const wptr<IRemoteObject> &remote);
    };

    std::mutex mutex_;
    // for download status notice
    std::mutex statusCallbacksMutex_;
    std::unordered_map<int32_t, sptr<IAudioSuiteStatusCallback>> statusCallbacks_;
    std::unordered_map<int32_t, sptr<AudioSuiteClientDeathRecipient>> deathRecipients_;
    bool registerStatusCallback_ = false;

    // for background download;
    std::shared_ptr<BackgroundDownload> backgroundDownload_ = nullptr;

    // for download Interface
    std::mutex downloadInterfaceMutex_;
    sptr<IAudioSuiteDownloadCallback> downloadCallback_ = nullptr;
    std::atomic<bool> isDownloadCallbackRegistered_ = false;
    std::condition_variable callbackRegisteredCond_;
    sptr<AudioServerDeathRecipient> audioServerdeathRecipient_ = nullptr;
};

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS

#endif // DOWNLOAD_MANAGER_H