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
#include <atomic>
#include "audio_suite_types.h"
#include "iaudio_suite_service.h"
#include "iremote_object.h"
#include "audio_suite_download_manager.h"
#include "audio_suite_status_callback_stub_impl.h"
#include "audio_suite_download_callback_stub_impl.h"
#include "system_ability_status_change_stub.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

class AudioSuiteSystemAbilityListener : public SystemAbilityStatusChangeStub {
public:
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
};

class AudioSuiteClientPolicy {
public:
    static AudioSuiteClientPolicy& GetInstance();

    AudioSuiteClientPolicy(const AudioSuiteClientPolicy&) = delete;
    AudioSuiteClientPolicy& operator=(const AudioSuiteClientPolicy&) = delete;

    int32_t RegisterDownloadStatusCallback(std::shared_ptr<DownloadStatusCallback> callback);
    int32_t UnregisterDownloadStatusCallback();
    int32_t GetInstallStatus(const std::vector<std::string>& featureNames,
        std::vector<bool>& installStatus);
    int32_t StartDownload(const std::vector<std::string>& featureNames, bool allowedCellularNet);
    int32_t PauseDownload(const std::vector<std::string>& featureNames);
    int32_t CancelDownload(const std::vector<std::string>& featureNames);
    int32_t StartBackgroundDownload(const std::vector<std::string>& featureNames);
    int32_t GetDownloadStatus(const std::vector<std::string>& featureNames,
        std::vector<AudioSuiteDownloadStatusInfo>& status);
    int32_t GetFeatureNewVersion(const std::vector<std::string>& featureNames,
        std::vector<AudioSuiteCloudRomVersionInfo>& versionInfos);
    int32_t UninstallCloudRom(const std::vector<std::string>& featureNames);

    int32_t RegisterDownloadInterfaceCallback(const std::shared_ptr<AudioSuiteRomDownloadCallback> &callback);
    int32_t UnregisterDownloadInterfaceCallback();
    void OnDownloadStatusChanged(const std::vector<AudioSuiteDownloadStatusInfo>& status);

    void DisconnectService();
    void AudioSuiteServerRecover(int32_t& systemAbilityId);

private:
    AudioSuiteClientPolicy();
    ~AudioSuiteClientPolicy();
    bool ConnectService();

    class AudioSuiteServerDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit AudioSuiteServerDeathRecipient() = default;
        virtual ~AudioSuiteServerDeathRecipient() = default;
        DISALLOW_COPY_AND_MOVE(AudioSuiteServerDeathRecipient);
        void OnRemoteDied(const wptr<IRemoteObject> &remote);
    };

    std::mutex mutex_;
    std::atomic<bool> connected_ = false;
    sptr<IAudioSuiteService> proxy_ = nullptr;
    sptr<AudioSuiteStatusCallbackStubImpl> statusCallbackStub_ = nullptr;
    sptr<AudioSuiteDownloadCallbackStubImpl> downloadCallbackStub_ = nullptr;
    sptr<AudioSuiteServerDeathRecipient> deathRecipient_ = nullptr;
    sptr<ISystemAbilityStatusChange> statusListener_ = nullptr;
};

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // AUDIO_SUITE_CLIENT_POLICY_H