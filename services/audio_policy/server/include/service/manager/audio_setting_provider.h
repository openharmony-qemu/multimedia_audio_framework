/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifndef AUDIO_SETTING_PROVIDER_H
#define AUDIO_SETTING_PROVIDER_H

#include <list>
#include <unordered_map>
#include <cinttypes>

#include "os_account_manager.h"
#include "ipc_skeleton.h"
#include "datashare_helper.h"
#include "errors.h"
#include "mutex"
#include "data_ability_observer_stub.h"

#include "audio_policy_log.h"
#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {
constexpr int32_t MAX_STRING_LENGTH = 10;
constexpr int32_t MIN_USER_ACCOUNT = 100;

class AudioSettingObserver : public AAFwk::DataAbilityObserverStub {
public:
    AudioSettingObserver() = default;
    ~AudioSettingObserver() = default;
    void OnChange() override;
    void SetKey(const std::string& key);
    const std::string& GetKey();

    using UpdateFunc = std::function<void(const std::string&)>;
    void SetUpdateFunc(UpdateFunc& func);

private:
    std::string key_ {};
    UpdateFunc update_ = nullptr;
};

class AudioSettingProvider : public NoCopyable {
public:
    static AudioSettingProvider& GetInstance(int32_t systemAbilityId);
    ErrCode GetStringValue(const std::string &key, std::string &value, std::string tableType = "");
    ErrCode GetIntValue(const std::string &key, int32_t &value, std::string tableType = "");
    ErrCode GetLongValue(const std::string &key, int64_t &value, std::string tableType = "");
    ErrCode GetBoolValue(const std::string &key, bool &value, std::string tableType = "");
    ErrCode PutStringValue(const std::string &key, const std::string &value,
        std::string tableType = "", bool needNotify = true);
    ErrCode PutIntValue(const std::string &key, int32_t value, std::string tableType = "", bool needNotify = true);
    ErrCode PutLongValue(const std::string &key, int64_t value, std::string tableType = "", bool needNotify = true);
    ErrCode PutBoolValue(const std::string &key, bool value, std::string tableType = "", bool needNotify = true);
    bool IsValidKey(const std::string &key);
    sptr<AudioSettingObserver> CreateObserver(const std::string &key, AudioSettingObserver::UpdateFunc &func);
    static void ExecRegisterCb(const sptr<AudioSettingObserver> &observer);
    ErrCode RegisterObserver(const sptr<AudioSettingObserver> &observer);
    ErrCode UnregisterObserver(const sptr<AudioSettingObserver> &observer);

protected:
    ~AudioSettingProvider() override;

private:
    static void Initialize(int32_t systemAbilityId);
    static std::shared_ptr<DataShare::DataShareHelper> CreateDataShareHelper(std::string tableType = "");
    static bool ReleaseDataShareHelper(std::shared_ptr<DataShare::DataShareHelper> &helper);
    static Uri AssembleUri(const std::string &key, std::string tableType = "");
    static int32_t GetCurrentUserId();

    static AudioSettingProvider *instance_;
    static std::mutex mutex_;
    static sptr<IRemoteObject> remoteObj_;
    static std::string SettingSystemUrlProxy_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_SETTING_PROVIDER_H
