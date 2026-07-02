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
#include "audio_suite_pcm_buffer.h"
#include "audio_suite_base.h"
#include "audio_stream_info.h"
#include "audio_suite_sa_types.h"
#include "audio_shared_memory.h"
#include "iaudio_suite_sa_service.h"
#include "audio_suite_sa_service_stub.h"
#include "iupdate_engine_callback.h"

namespace OHOS {
namespace AudioStandard {

namespace AudioSuite {

class DownloadManager;
class ConditionMonitor;

class AudioSuiteRomDownloadService : public SystemAbility, public AudioSuiteSaServiceStub  {
    DECLARE_SYSTEM_ABILITY(AudioSuiteRomDownloadService);

public:
    DISALLOW_COPY_AND_MOVE(AudioSuiteRomDownloadService);
    explicit AudioSuiteRomDownloadService(int32_t systemAbilityId, bool runOnCreate = false);
    virtual ~AudioSuiteRomDownloadService() = default;

    void OnStart() override;
    void OnStop() override;
    int32_t Dump(int32_t fd, const std::vector<std::u16string>& args) override;

    ErrCode RegisterDownloadCallback(const sptr<IRemoteObject> &callback, ErrCode &result) override;
    ErrCode StartDownload(const std::string &version, ErrCode &result) override;
    ErrCode CancelDownload(ErrCode &result) override;
    ErrCode StartBackgroundDownload(const std::string &version, ErrCode &result) override;
    ErrCode GetDownloadStatus(DownloadStatus &status, const std::string& version, ErrCode &result) override;
    ErrCode UninstallCloudRom(const std::string &version, ErrCode &result) override;
    ErrCode UnregisterDownloadCallback(ErrCode& result) override;
    ErrCode RegisterUpdateEngineCallback(const sptr<IRemoteObject> &callback, ErrCode &result) override;
    ErrCode UnregisterUpdateEngineCallback(ErrCode& result) override;

    static constexpr int32_t SA_ID = 3015;

protected:
    virtual bool CheckPermission();

private:
    void NotifyDownloadStatusChanged(const DownloadStatus& status);

    std::shared_ptr<DownloadManager> downloadManager_;
    sptr<IRemoteObject> downloadCallback_;
    sptr<IRemoteObject> updateEngineCallback_;

    std::mutex mutex_;
};

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_SUITE_SA_SERVICE_H