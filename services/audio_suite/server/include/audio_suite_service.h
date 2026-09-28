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

#ifndef AUDIO_SUITE_SA_SERVICE_H
#define AUDIO_SUITE_SA_SERVICE_H

#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <chrono>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <queue>
#include <iremote_object.h>
#include "system_ability.h"
#include "audio_errors.h"
#include "audio_suite_types.h"
#include "audio_suite_service_stub.h"
#include "event_handler.h"

namespace OHOS {
namespace AudioStandard {

namespace AudioSuite {

class DownloadManager;
class ConditionMonitor;

class AudioSuiteService : public SystemAbility, public AudioSuiteServiceStub  {
    DECLARE_SYSTEM_ABILITY(AudioSuiteService);

public:
    DISALLOW_COPY_AND_MOVE(AudioSuiteService);
    explicit AudioSuiteService(int32_t systemAbilityId, bool runOnCreate = false);
    virtual ~AudioSuiteService();

    void OnStart() override;
    void OnStop() override;

    ErrCode RegisterDownloadStatusCallback(const sptr<IRemoteObject> &object, ErrCode &result) override;
    ErrCode UnregisterDownloadStatusCallback(ErrCode& result) override;
    ErrCode GetInstallStatus(
        const std::vector<std::string> &featureNames, std::vector<bool> &installStatus, ErrCode &result) override;
    ErrCode StartDownload(
        const std::vector<std::string> &featureNames, bool allowedCellularNet, ErrCode &result) override;
    ErrCode PauseDownload(const std::vector<std::string>& featureNames, ErrCode& result) override;
    ErrCode CancelDownload(const std::vector<std::string> &featureNames, ErrCode &result) override;
    ErrCode StartBackgroundDownload(const std::vector<std::string> &featureNames, ErrCode &result) override;
    ErrCode GetDownloadStatus(
        const std::vector<std::string>& featureNames, std::vector<AudioSuiteDownloadStatusInfo> &status,
        ErrCode &result) override;
    ErrCode GetFeatureStatus(
        const std::vector<std::string>& featureNames, std::vector<AudioSuiteDownloadStatusInfo> &status,
        ErrCode &result) override;
    ErrCode GetFeatureNewVersion(
        const std::vector<std::string>& featureNames, std::vector<AudioSuiteCloudRomVersionInfo> &newVersion,
        ErrCode &result) override;
    ErrCode UninstallCloudRom(const std::vector<std::string> &featureNames, ErrCode &result) override;

    ErrCode RegisterDownloadInterfaceCallback(const sptr<IRemoteObject> &object, ErrCode &result) override;
    ErrCode UnregisterDownloadInterfaceCallback(ErrCode& result) override;
    ErrCode OnDownloadStatusChanged(const std::vector<AudioSuiteDownloadStatusInfo>& status) override;

protected:
    bool VerifyIsSystemApp(ErrCode& result);

private:
    void PostDelayUnloadTask();
    bool ShouldExtendUnloadTimer();

    std::mutex mutex_;
    DownloadManager &downloadManager_;

    std::mutex eventMutex_;
    std::shared_ptr<AppExecFwk::EventRunner> unloadRunner_;
    std::shared_ptr<AppExecFwk::EventHandler> unloadHandler_;
};

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_SUITE_SA_SERVICE_H