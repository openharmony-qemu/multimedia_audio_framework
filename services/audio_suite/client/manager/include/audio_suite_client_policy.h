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
#ifndef AUDIO_SUITE_CLIENT_POLICY_H
#define AUDIO_SUITE_CLIENT_POLICY_H

#include <memory>
#include <mutex>
#include <string>
#include <functional>
#include "audio_suite_sa_types.h"
#include "download_callback_stub_impl.h"
#include "iaudio_suite_sa_service.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

using SaDiedCallback = std::function<void()>;
class IDownloadCallbackListener;

class AudioSuiteClientPolicy {
public:
    static AudioSuiteClientPolicy& GetInstance();

    AudioSuiteClientPolicy(const AudioSuiteClientPolicy&) = delete;
    AudioSuiteClientPolicy& operator=(const AudioSuiteClientPolicy&) = delete;

    int32_t Init();
    void DeInit();

    int32_t RegisterDownloadCallback(std::shared_ptr<IDownloadCallbackListener> callback);
    int32_t UnregisterDownloadCallback();
    int32_t StartDownload(const std::string& version);
    int32_t CancelDownload(const std::string& version);
    int32_t StartBackgroundDownload(const std::string& version);
    int32_t GetDownloadStatus(DownloadStatus& status, const std::string& version);
    int32_t UninstallCloudRom(const std::string& version);

    static constexpr int32_t SA_ID = 3015;

private:
    AudioSuiteClientPolicy();
    ~AudioSuiteClientPolicy();

    bool ConnectService();
    void DisconnectService();
    bool EnsureConnected();

    sptr<IAudioSuiteSaService> proxy_;
    bool connected_ = false;
    bool needsReconnect_ = false;
    std::mutex mutex_;

    std::shared_ptr<IDownloadCallbackListener> downloadCallback_;
    sptr<DownloadCallbackStubImpl> downloadCallbackStub_;
};

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // AUDIO_SUITE_CLIENT_POLICY_H