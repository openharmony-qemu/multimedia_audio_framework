/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

namespace OHOS {
namespace AudioStandard {
constexpr int32_t MAX_STRING_LENGTH = 10;
constexpr int32_t MIN_USER_ACCOUNT = 100;

struct IntValueInfo {
    std::string key;
    int32_t defaultValue;
    int32_t value;
    int32_t maxValue;
};

struct BoolValueInfo {
    std::string key;
    bool defaultValue;
    bool value;
};

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
    static constexpr int32_t INVALID_ACCOUNT_ID = -1;
    static constexpr int32_t MAIN_USER_ID = 100;

    static AudioSettingProvider& GetInstance(int32_t systemAbilityId);
    static int32_t GetCurrentUserId(int32_t specificUserId = INVALID_ACCOUNT_ID);
    static bool CheckOsAccountReady();
    ErrCode GetStringValue(const std::string &key, std::string &value, std::string tableType = "",
        int32_t userId = INVALID_ACCOUNT_ID);
    ErrCode GetIntValue(const std::string &key, int32_t &value, std::string tableType = "");
    ErrCode GetLongValue(const std::string &key, int64_t &value, std::string tableType = "");
    ErrCode GetFloatValue(const std::string &key, float &value, std::string tableType = "");
    ErrCode GetBoolValue(const std::string &key, bool &value, std::string tableType = "",
        int32_t userId = INVALID_ACCOUNT_ID);
    ErrCode GetMapValue(const std::string &key, std::vector<std::map<std::string, std::string>> &value,
        std::string tableType = "");
    ErrCode PutStringValue(const std::string &key, const std::string &value,
        std::string tableType = "", bool needNotify = true, int32_t userId = INVALID_ACCOUNT_ID);
    ErrCode PutIntValue(const std::string &key, int32_t value, std::string tableType = "", bool needNotify = true);
    ErrCode PutLongValue(const std::string &key, int64_t value, std::string tableType = "", bool needNotify = true);
    ErrCode PutBoolValue(const std::string &key, bool value, std::string tableType = "", bool needNotify = true,
        int32_t userId = INVALID_ACCOUNT_ID);
    bool IsValidKey(const std::string &key);
    void SetDataShareReady(std::atomic<bool> isDataShareReady);
    sptr<AudioSettingObserver> CreateObserver(const std::string &key, AudioSettingObserver::UpdateFunc &func);
    static void ExecRegisterCb(const sptr<AudioSettingObserver> &observer);
    ErrCode RegisterObserver(const sptr<AudioSettingObserver> &observer, std::string tableType = "");
    ErrCode UnregisterObserver(const sptr<AudioSettingObserver> &observer, std::string tableType = "");
    std::vector<std::map<std::string, std::string>> ParseJsonArray(const std::string& input);
    std::string ParseFirstOfKey(size_t &pos, size_t len, std::string input);
    std::string ParseSecondOfValue(size_t &pos, size_t len, std::string input);

// rewrite database operations
public:
    void GetIntValues(std::vector<IntValueInfo> &infos, std::string tableType);
    ErrCode PutIntValues(std::vector<IntValueInfo>& infos, std::string tableType);
    void GetBoolValues(std::vector<BoolValueInfo> &infos, std::string tableType);
private:
    void GetIntValuesInner(std::vector<IntValueInfo> &infos, std::string tableType);
    void GetBoolValuesInner(std::vector<BoolValueInfo> &infos, std::string tableType);
    ErrCode GetIntValueInner(std::shared_ptr<DataShare::DataShareHelper> helper,
        std::string key, std::string tableType, int32_t &res);
    ErrCode GetBoolValueInner(std::shared_ptr<DataShare::DataShareHelper> helper,
        std::string key, std::string tableType, bool &res);

    ErrCode PutIntValuesInner(std::vector<IntValueInfo> &infos, std::string tableType);
    ErrCode PutIntValueInner(std::shared_ptr<DataShare::DataShareHelper> helper,
        std::string key, std::string value, std::string tableType);

// tools
public:
    void TrimLeft(std::string &str);
    int32_t StringToInt32(const std::string &str, int32_t &result);
    int64_t StringToInt64(const std::string &str, int64_t &result);

protected:
    ~AudioSettingProvider() override;

private:
    static std::atomic<bool> isDataShareReady_;
    static void Initialize(int32_t systemAbilityId);
    static std::shared_ptr<DataShare::DataShareHelper> CreateDataShareHelper(std::string tableType = "",
        int32_t userId = INVALID_ACCOUNT_ID);
    static bool ReleaseDataShareHelper(std::shared_ptr<DataShare::DataShareHelper> &helper);
    static Uri AssembleUri(const std::string &key, std::string tableType = "", int32_t userId = INVALID_ACCOUNT_ID);

    static std::mutex mutex_;
    static sptr<IRemoteObject> remoteObj_;
    static std::string SettingSystemUrlProxy_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_SETTING_PROVIDER_H
