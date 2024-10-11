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
#ifndef LOG_TAG
#define LOG_TAG "AudioSettingProvider"
#endif

#include "audio_setting_provider.h"

#include "iservice_registry.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
AudioSettingProvider* AudioSettingProvider::instance_;
std::mutex AudioSettingProvider::mutex_;
sptr<IRemoteObject> AudioSettingProvider::remoteObj_;

const std::string SETTING_COLUMN_KEYWORD = "KEYWORD";
const std::string SETTING_COLUMN_VALUE = "VALUE";
const std::string SETTING_URI_PROXY = "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true";
const std::string SETTING_USER_URI_PROXY = "datashare:///com.ohos.settingsdata/entry/settingsdata/USER_SETTINGSDATA_";
const std::string SETTING_USER_SECURE_URI_PROXY =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/USER_SETTINGSDATA_SECURE_";
constexpr const char *SETTINGS_DATA_EXT_URI = "datashare:///com.ohos.settingsdata.DataAbility";
constexpr int32_t RETRY_TIMES = 5;
constexpr int64_t SLEEP_TIME = 1;

AudioSettingProvider::~AudioSettingProvider()
{
    instance_ = nullptr;
    remoteObj_ = nullptr;
}

void AudioSettingObserver::OnChange()
{
    if (update_) {
        update_(key_);
    }
}

void AudioSettingObserver::SetKey(const std::string &key)
{
    key_ = key;
}

const std::string& AudioSettingObserver::GetKey()
{
    return key_;
}

void AudioSettingObserver::SetUpdateFunc(UpdateFunc &func)
{
    update_ = func;
}

AudioSettingProvider& AudioSettingProvider::GetInstance(
    int32_t systemAbilityId)
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (instance_ == nullptr) {
            instance_ = new AudioSettingProvider();
            Initialize(systemAbilityId);
        }
    }
    return *instance_;
}

ErrCode AudioSettingProvider::GetIntValue(const std::string &key, int32_t &value,
    std::string tableType)
{
    int64_t valueLong;
    ErrCode ret = GetLongValue(key, valueLong, tableType);
    if (ret != ERR_OK) {
        return ret;
    }
    value = static_cast<int32_t>(valueLong);
    return ERR_OK;
}

ErrCode AudioSettingProvider::GetLongValue(const std::string &key, int64_t &value,
    std::string tableType)
{
    std::string valueStr;
    ErrCode ret = GetStringValue(key, valueStr, tableType);
    if (ret != ERR_OK) {
        return ret;
    }
    value = static_cast<int64_t>(strtoll(valueStr.c_str(), nullptr, MAX_STRING_LENGTH));
    return ERR_OK;
}

ErrCode AudioSettingProvider::GetBoolValue(const std::string &key, bool &value,
    std::string tableType)
{
    std::string valueStr;
    ErrCode ret = GetStringValue(key, valueStr, tableType);
    if (ret != ERR_OK) {
        return ret;
    }
    value = (valueStr == "true");
    return ERR_OK;
}

ErrCode AudioSettingProvider::PutIntValue(const std::string &key, int32_t value,
    std::string tableType, bool needNotify)
{
    return PutStringValue(key, std::to_string(value), tableType, needNotify);
}

ErrCode AudioSettingProvider::PutLongValue(const std::string &key, int64_t value,
    std::string tableType, bool needNotify)
{
    return PutStringValue(key, std::to_string(value), tableType, needNotify);
}

ErrCode AudioSettingProvider::PutBoolValue(const std::string &key, bool value,
    std::string tableType, bool needNotify)
{
    std::string valueStr = value ? "true" : "false";
    return PutStringValue(key, valueStr, tableType, needNotify);
}

bool AudioSettingProvider::IsValidKey(const std::string &key)
{
    std::string value;
    ErrCode ret = GetStringValue(key, value);
    return (ret != ERR_NAME_NOT_FOUND) && (!value.empty());
}

sptr<AudioSettingObserver> AudioSettingProvider::CreateObserver(
    const std::string &key, AudioSettingObserver::UpdateFunc &func)
{
    sptr<AudioSettingObserver> observer = new AudioSettingObserver();
    observer->SetKey(key);
    observer->SetUpdateFunc(func);
    return observer;
}

void AudioSettingProvider::ExecRegisterCb(const sptr<AudioSettingObserver> &observer)
{
    if (observer == nullptr) {
        AUDIO_ERR_LOG("observer is nullptr");
        return;
    }
    observer->OnChange();
}

ErrCode AudioSettingProvider::RegisterObserver(const sptr<AudioSettingObserver> &observer)
{
    std::string callingIdentity = IPCSkeleton::ResetCallingIdentity();
    auto uri = AssembleUri(observer->GetKey());
    auto helper = CreateDataShareHelper();
    if (helper == nullptr) {
        IPCSkeleton::SetCallingIdentity(callingIdentity);
        return ERR_NO_INIT;
    }
    helper->RegisterObserver(uri, observer);
    helper->NotifyChange(uri);
    auto execFirCb = ([observer] { ExecRegisterCb(observer); });
    std::thread execCb(execFirCb);
    execCb.detach();
    ReleaseDataShareHelper(helper);
    IPCSkeleton::SetCallingIdentity(callingIdentity);
    AUDIO_DEBUG_LOG("succeed to register observer of uri=%{public}s", uri.ToString().c_str());
    return ERR_OK;
}

ErrCode AudioSettingProvider::UnregisterObserver(const sptr<AudioSettingObserver> &observer)
{
    std::string callingIdentity = IPCSkeleton::ResetCallingIdentity();
    auto uri = AssembleUri(observer->GetKey());
    auto helper = CreateDataShareHelper();
    if (helper == nullptr) {
        IPCSkeleton::SetCallingIdentity(callingIdentity);
        return ERR_NO_INIT;
    }
    helper->UnregisterObserver(uri, observer);
    ReleaseDataShareHelper(helper);
    IPCSkeleton::SetCallingIdentity(callingIdentity);
    AUDIO_DEBUG_LOG("succeed to unregister observer of uri=%{public}s", uri.ToString().c_str());
    return ERR_OK;
}

void AudioSettingProvider::Initialize(int32_t systemAbilityId)
{
    auto sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        AUDIO_ERR_LOG("GetSystemAbilityManager return nullptr");
        return;
    }
    auto remoteObj = sam->GetSystemAbility(systemAbilityId);
    if (remoteObj == nullptr) {
        AUDIO_ERR_LOG("GetSystemAbility return nullptr, systemAbilityId=%{public}d", systemAbilityId);
        return;
    }
    remoteObj_ = remoteObj;
}

ErrCode AudioSettingProvider::GetStringValue(const std::string &key,
    std::string &value, std::string tableType)
{
    std::string callingIdentity = IPCSkeleton::ResetCallingIdentity();
    auto helper = CreateDataShareHelper(tableType);
    if (helper == nullptr) {
        IPCSkeleton::SetCallingIdentity(callingIdentity);
        return ERR_NO_INIT;
    }
    std::vector<std::string> columns = {SETTING_COLUMN_VALUE};
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(SETTING_COLUMN_KEYWORD, key);
    Uri uri(AssembleUri(key, tableType));
    auto resultSet = helper->Query(uri, predicates, columns);
    ReleaseDataShareHelper(helper);
    if (resultSet == nullptr) {
        AUDIO_ERR_LOG("helper->Query return nullptr");
        IPCSkeleton::SetCallingIdentity(callingIdentity);
        return ERR_INVALID_OPERATION;
    }
    int32_t count;
    resultSet->GetRowCount(count);
    if (count == 0) {
        AUDIO_WARNING_LOG("not found value, key=%{public}s, count=%{public}d", key.c_str(), count);
        IPCSkeleton::SetCallingIdentity(callingIdentity);
        resultSet->Close();
        return ERR_NAME_NOT_FOUND;
    }
    const int32_t INDEX = 0;
    resultSet->GoToRow(INDEX);
    int32_t ret = resultSet->GetString(INDEX, value);
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("resultSet->GetString return not ok, ret=%{public}d", ret);
        IPCSkeleton::SetCallingIdentity(callingIdentity);
        resultSet->Close();
        return ERR_INVALID_VALUE;
    }
    resultSet->Close();
    IPCSkeleton::SetCallingIdentity(callingIdentity);
    return ERR_OK;
}

ErrCode AudioSettingProvider::PutStringValue(const std::string &key, const std::string &value,
    std::string tableType, bool needNotify)
{
    std::string callingIdentity = IPCSkeleton::ResetCallingIdentity();
    auto helper = CreateDataShareHelper(tableType);
    if (helper == nullptr) {
        IPCSkeleton::SetCallingIdentity(callingIdentity);
        return ERR_NO_INIT;
    }
    DataShare::DataShareValueObject keyObj(key);
    DataShare::DataShareValueObject valueObj(value);
    DataShare::DataShareValuesBucket bucket;
    bucket.Put(SETTING_COLUMN_KEYWORD, keyObj);
    bucket.Put(SETTING_COLUMN_VALUE, valueObj);
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(SETTING_COLUMN_KEYWORD, key);
    Uri uri(AssembleUri(key, tableType));
    if (helper->Update(uri, predicates, bucket) <= 0) {
        AUDIO_DEBUG_LOG("no data exist, insert one row");
        helper->Insert(uri, bucket);
    }
    if (needNotify) {
        helper->NotifyChange(AssembleUri(key, tableType));
    }
    ReleaseDataShareHelper(helper);
    IPCSkeleton::SetCallingIdentity(callingIdentity);
    return ERR_OK;
}

int32_t AudioSettingProvider::GetCurrentUserId()
{
    std::vector<int> ids;
    int32_t currentuserId = -1;
    ErrCode result;
    int32_t retry = RETRY_TIMES;
    while (retry--) {
        result = AccountSA::OsAccountManager::QueryActiveOsAccountIds(ids);
        if (result == ERR_OK && !ids.empty()) {
            currentuserId = ids[0];
            AUDIO_DEBUG_LOG("current userId is :%{public}d", currentuserId);
            break;
        }
        // sleep and wait for 1 millisecond
        sleep(SLEEP_TIME);
    }
    if (result != ERR_OK || ids.empty()) {
        AUDIO_WARNING_LOG("current userId is empty");
    }
    return currentuserId;
}

std::shared_ptr<DataShare::DataShareHelper> AudioSettingProvider::CreateDataShareHelper(
    std::string tableType)
{
#ifdef SUPPORT_USER_ACCOUNT
    int32_t currentuserId = GetCurrentUserId();
    if (currentuserId < MIN_USER_ACCOUNT) {
        currentuserId = MIN_USER_ACCOUNT;
    }
#else
    int32_t currentuserId = -1;
#endif
    std::shared_ptr<DataShare::DataShareHelper> helper = nullptr;
    std::string SettingSystemUrlProxy = "";
    // deal with multi useraccount table
    if (currentuserId > 0 && tableType == "system") {
        SettingSystemUrlProxy =
            SETTING_USER_URI_PROXY + std::to_string(currentuserId) + "?Proxy=true";
        helper = DataShare::DataShareHelper::Creator(remoteObj_, SettingSystemUrlProxy, SETTINGS_DATA_EXT_URI);
    } else if (currentuserId > 0 && tableType == "secure") {
        SettingSystemUrlProxy =
            SETTING_USER_SECURE_URI_PROXY + std::to_string(currentuserId) + "?Proxy=true";
        helper = DataShare::DataShareHelper::Creator(remoteObj_, SettingSystemUrlProxy, SETTINGS_DATA_EXT_URI);
    } else {
        helper = DataShare::DataShareHelper::Creator(remoteObj_, SETTING_URI_PROXY, SETTINGS_DATA_EXT_URI);
    }
    if (helper == nullptr) {
        AUDIO_WARNING_LOG("helper is nullptr, uri=%{public}s", SettingSystemUrlProxy.c_str());
        return nullptr;
    }
    return helper;
}

bool AudioSettingProvider::ReleaseDataShareHelper(
    std::shared_ptr<DataShare::DataShareHelper> &helper)
{
    if (!helper->Release()) {
        AUDIO_WARNING_LOG("release helper fail");
        return false;
    }
    return true;
}

Uri AudioSettingProvider::AssembleUri(const std::string &key, std::string tableType)
{
#ifdef SUPPORT_USER_ACCOUNT
    int32_t currentuserId = GetCurrentUserId();
    if (currentuserId < MIN_USER_ACCOUNT) {
        currentuserId = MIN_USER_ACCOUNT;
    }
#else
    int32_t currentuserId = -1;
#endif
    std::string SettingSystemUrlProxy = "";

    // deal with multi useraccount table
    if (currentuserId > 0 && tableType == "system") {
        SettingSystemUrlProxy = SETTING_USER_URI_PROXY + std::to_string(currentuserId) + "?Proxy=true";
        Uri uri(SettingSystemUrlProxy + "&key=" + key);
        return uri;
    } else if (currentuserId > 0 && tableType == "secure") {
        SettingSystemUrlProxy = SETTING_USER_SECURE_URI_PROXY + std::to_string(currentuserId) + "?Proxy=true";
        Uri uri(SettingSystemUrlProxy + "&key=" + key);
        return uri;
    }
    Uri uri(SETTING_URI_PROXY + "&key=" + key);
    return uri;
}
} // namespace AudioStandard
} // namespace OHOS
