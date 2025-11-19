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
#include "system_ability_definition.h"
#include "audio_utils.h"
#include "audio_log.h"

namespace OHOS {
namespace AudioStandard {
std::mutex AudioSettingProvider::mutex_;
std::atomic<bool> AudioSettingProvider::isDataShareReady_ = false;
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
    static AudioSettingProvider instance_;
    std::lock_guard<std::mutex> lock(mutex_);
    if (remoteObj_ == nullptr) {
        Initialize(systemAbilityId);
    }
    return instance_;
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

ErrCode AudioSettingProvider::GetFloatValue(const std::string &key, float &value,
    std::string tableType)
{
    std::string valueStr;
    ErrCode ret = GetStringValue(key, valueStr, tableType);
    if (ret != ERR_OK) {
        return ret;
    }
    CHECK_AND_RETURN_RET_LOG(StringConverterFloat(valueStr, value), ERR_INVALID_PARAM,
        "GetFloatValue error! invalid valueStr = %{public}s", valueStr.c_str());
    return ERR_OK;
}

ErrCode AudioSettingProvider::GetBoolValue(const std::string &key, bool &value,
    std::string tableType, int32_t userId)
{
    std::string valueStr;
    ErrCode ret = GetStringValue(key, valueStr, tableType, userId);
    if (ret != ERR_OK) {
        return ret;
    }
    value = (valueStr == "true");
    return ERR_OK;
}

ErrCode AudioSettingProvider::GetMapValue(const std::string &key,
    std::vector<std::map<std::string, std::string>> &value, std::string tableType)
{
    std::string valueStr;
    ErrCode ret = GetStringValue(key, valueStr, tableType);
    if (ret != ERR_OK) {
        return ret;
    }
    value = ParseJsonArray(valueStr);
    return ERR_OK;
}

std::vector<std::map<std::string, std::string>> AudioSettingProvider::ParseJsonArray(const std::string& input)
{
    std::vector<std::map<std::string, std::string>> result;
    size_t pos = 0;
    const size_t len = input.length();
    //skip the space value
    auto skipWhitespace = [&]() {
        while (pos < len && isspace(input[pos])) pos++;
    };

    skipWhitespace();
    if (input[pos++] != '[') return {};
    while (pos < len) {
        skipWhitespace();
        if (input[pos] == ']') break;
        if (input[pos++]!= '{') return {};
        std::map<std::string, std::string> obj;
        while (pos < len) {
            skipWhitespace();
            if (input[pos] == '}') {
                pos++;
                break;
            }
            std::string key = ParseFirstOfKey(pos, len, input);
            if (key != "uid" && input.find(',', pos) != std::string::npos) {
                pos = input.find(',', pos);
                pos++;
                continue;
            }
            if (key != "uid") {
                continue;
            }
            skipWhitespace();
            if (input[pos++] != ':') return {};
            std::string value = ParseSecondOfValue(pos, len, input);
            if (!key.empty() || !value.empty()) {
                obj[value] = "1";
            }
            skipWhitespace();
            if (input[pos] == ',') pos++;
        }
        result.push_back(obj);
        skipWhitespace();
        if (input[pos] == ',') pos++;
    }
    return result;
}

std::string AudioSettingProvider::ParseFirstOfKey(size_t &pos, size_t len, std::string input)
{
    // parse the key of input
    while (pos < len && isspace(input[pos])) {
        pos++;
    }
    if (pos >= len) {
        return "";
    }
    size_t start = ++pos;
    while (pos < len && input[pos] != '"') {
        pos++;
    }
    std::string str = input.substr(start, pos - start);
    if (pos < len) {
        pos++;
    }
    return str;
}

std::string AudioSettingProvider::ParseSecondOfValue(size_t &pos, size_t len, std::string input)
{
    // parse the value of input
    while (pos < len && isspace(input[pos])) {
        pos++;
    }
    if (pos >= len) {
        return "";
    }
    size_t start = pos;
    while (pos < len && input[pos] != ',') {
        pos++;
    }
    std::string str = input.substr(start, pos - start);
    if (pos < len) {
        pos++;
    }
    return str;
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
    std::string tableType, bool needNotify, int32_t userId)
{
    std::string valueStr = value ? "true" : "false";
    return PutStringValue(key, valueStr, tableType, needNotify, userId);
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

ErrCode AudioSettingProvider::RegisterObserver(const sptr<AudioSettingObserver> &observer, std::string tableType)
{
    std::string callingIdentity = IPCSkeleton::ResetCallingIdentity();
    auto uri = AssembleUri(observer->GetKey(), tableType);
    if (!isDataShareReady_) {
        AUDIO_WARNING_LOG("DataShareHelper is not ready");
        IPCSkeleton::SetCallingIdentity(callingIdentity);
        return ERR_NO_INIT;
    }
    auto helper = CreateDataShareHelper(tableType);
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

ErrCode AudioSettingProvider::UnregisterObserver(const sptr<AudioSettingObserver> &observer, std::string tableType)
{
    std::string callingIdentity = IPCSkeleton::ResetCallingIdentity();
    auto uri = AssembleUri(observer->GetKey(), tableType);
    auto helper = CreateDataShareHelper(tableType);
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
    std::string &value, std::string tableType, int32_t userId)
{
    std::string callingIdentity = IPCSkeleton::ResetCallingIdentity();
    auto helper = CreateDataShareHelper(tableType, userId);
    if (helper == nullptr) {
        IPCSkeleton::SetCallingIdentity(callingIdentity);
        return ERR_NO_INIT;
    }
    std::vector<std::string> columns = {SETTING_COLUMN_VALUE};
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(SETTING_COLUMN_KEYWORD, key);
    Uri uri(AssembleUri(key, tableType, userId));
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
        AUDIO_WARNING_LOG("not found value, key=%{public}s, uri=%{public}s, count=%{public}d", key.c_str(),
            uri.ToString().c_str(), count);
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
    } else {
        AUDIO_INFO_LOG("Read audio_info_database with key: %{public}s value: %{public}s in uri=%{public}s ",
            key.c_str(), value.c_str(), uri.ToString().c_str());
    }
    resultSet->Close();
    IPCSkeleton::SetCallingIdentity(callingIdentity);
    return ERR_OK;
}

ErrCode AudioSettingProvider::PutStringValue(const std::string &key, const std::string &value,
    std::string tableType, bool needNotify, int32_t userId)
{
    AUDIO_INFO_LOG("Write audio_info_database with key: %{public}s value: %{public}s", key.c_str(), value.c_str());
    std::string callingIdentity = IPCSkeleton::ResetCallingIdentity();
    auto helper = CreateDataShareHelper(tableType, userId);
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
    Uri uri(AssembleUri(key, tableType, userId));
    if (helper->Update(uri, predicates, bucket) <= 0) {
        AUDIO_INFO_LOG("audio_info_database no data exist, insert one row");
        helper->Insert(uri, bucket);
    }
    if (needNotify) {
        helper->NotifyChange(AssembleUri(key, tableType, userId));
    }
    ReleaseDataShareHelper(helper);
    IPCSkeleton::SetCallingIdentity(callingIdentity);
    return ERR_OK;
}

int32_t AudioSettingProvider::GetCurrentUserId(int32_t specificUserId)
{
    if (specificUserId != INVALID_ACCOUNT_ID && specificUserId >= MIN_USER_ACCOUNT) {
        AUDIO_INFO_LOG("just use specific id: %{public}d", specificUserId);
        return specificUserId;
    }
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
        // sleep and wait for 1 second
        sleep(SLEEP_TIME);
    }
    if (result != ERR_OK || ids.empty()) {
        AUDIO_WARNING_LOG("current userId is empty");
    }
    return currentuserId;
}

bool AudioSettingProvider::CheckOsAccountReady()
{
    std::vector<int> ids;
    ErrCode result = AccountSA::OsAccountManager::QueryActiveOsAccountIds(ids);
    return (result == ERR_OK && !ids.empty());
}

void AudioSettingProvider::SetDataShareReady(std::atomic<bool> isDataShareReady)
{
    AUDIO_INFO_LOG("Receive event DATA_SHARE_READY");
    isDataShareReady_.store(isDataShareReady);
}

std::shared_ptr<DataShare::DataShareHelper> AudioSettingProvider::CreateDataShareHelper(
    std::string tableType, int32_t userId)
{
    CHECK_AND_RETURN_RET_LOG(isDataShareReady_.load(), nullptr,
        "DATA_SHARE_READY not received, create DataShareHelper failed");
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (remoteObj_ == nullptr) {
            AUDIO_WARNING_LOG("remoteObj_ is nullptr");
            Initialize(AUDIO_POLICY_SERVICE_ID);
        }
    }
#ifdef SUPPORT_USER_ACCOUNT
    int32_t currentuserId = GetCurrentUserId(userId);
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
        WatchTimeout guard("DataShare::DataShareHelper::Creator:CreateDataShareHelper.SettingSystemUrlProxy");
        helper = DataShare::DataShareHelper::Creator(remoteObj_, SettingSystemUrlProxy, SETTINGS_DATA_EXT_URI);
        guard.CheckCurrTimeout();
    } else {
        WatchTimeout guard("DataShare::DataShareHelper::Creator:CreateDataShareHelper.SETTING_URI_PROXY");
        helper = DataShare::DataShareHelper::Creator(remoteObj_, SETTING_URI_PROXY, SETTINGS_DATA_EXT_URI);
        guard.CheckCurrTimeout();
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

Uri AudioSettingProvider::AssembleUri(const std::string &key, std::string tableType, int32_t userId)
{
#ifdef SUPPORT_USER_ACCOUNT
    int32_t currentuserId = GetCurrentUserId(userId);
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

// rewrite database operations
void AudioSettingProvider::TrimLeft(std::string &str)
{
    if (str.empty()) {
        return;
    }
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start != std::string::npos) {
        str =str.substr(start);
    } else {
        str.clear();
    }
}

int32_t AudioSettingProvider::StringToInt32(const std::string &str, int32_t &result)
{
    if (str.empty()) {
        return ERROR;
    }
    int32_t oldValue = result;

    std::string s = str;
    TrimLeft(s);
    if (s.empty()) {
        return ERROR;
    }

    const auto *first = s.data();
    const auto *last = first + s.size();
    std::from_chars_result res = std::from_chars(first, last, result);
    if (res.ec == std::errc{} && res.ptr ==last) {
        return SUCCESS;
    } else {
        result = oldValue;
        return ERROR;
    }
}

int64_t AudioSettingProvider::StringToInt64(const std::string &str, int64_t &result)
{
    if (str.empty()) {
        return ERROR;
    }
    int64_t oldValue = result;

    std::string s = str;
    TrimLeft(s);
    if (s.empty()) {
        return ERROR;
    }

    const auto *first = s.data();
    const auto *last = first + s.size();
    std::from_chars_result res = std::from_chars(first, last, result);
    if (res.ec == std::errc{} && res.ptr ==last) {
        return SUCCESS;
    } else {
        result = oldValue;
        return ERROR;
    }
}

void AudioSettingProvider::GetIntValues(std::vector<IntValueInfo> &infos, std::string tableType)
{
    std::string callingIdentity = IPCSkeleton::ResetCallingIdentity();
    for (auto &info : infos) {
        info.value = info.defaultValue;
    }
    GetIntValuesInner(infos, tableType);
    IPCSkeleton::SetCallingIdentity(callingIdentity);
}

void AudioSettingProvider::GetIntValuesInner(std::vector<IntValueInfo> &infos, std::string tableType)
{
    auto helper = CreateDataShareHelper(tableType);
    CHECK_AND_RETURN_LOG(helper != nullptr, "helper is null");
    for (auto &info : infos) {
        int32_t res = 0;
        int32_t ret = GetIntValueInner(helper, info.key, tableType, res);
        CHECK_AND_CONTINUE(ret == SUCCESS);
        info.value = res;
    }
    ReleaseDataShareHelper(helper);
}

ErrCode AudioSettingProvider::GetIntValueInner(std::shared_ptr<DataShare::DataShareHelper> helper,
    std::string key, std::string tableType, int32_t &res)
{
    CHECK_AND_RETURN_RET_LOG(helper != nullptr, ERR_NO_INIT, "helper is null");
    std::vector<std::string> columns = {SETTING_COLUMN_VALUE};
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(SETTING_COLUMN_KEYWORD, key);
    Uri uri(AssembleUri(key, tableType));
    auto resultSet = helper->Query(uri, predicates, columns);
    CHECK_AND_RETURN_RET_LOG(resultSet != nullptr, ERR_INVALID_OPERATION, "helper->Query return nullptr");

    int32_t count;
    resultSet->GetRowCount(count);
    if (count == 0) {
        AUDIO_WARNING_LOG("not found value, key=%{public}s, uri=%{public}s, count=%{public}d",
            key.c_str(), uri.ToString().c_str(), count);
        resultSet->Close();
        return ERR_NAME_NOT_FOUND;
    }
    const int32_t INDEX = 0;
    resultSet->GoToRow(INDEX);

    std::string value;
    int32_t ret = resultSet->GetString(INDEX, value);
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("resultSet->GetString return not ok, ret=%{public}d", ret);
        resultSet->Close();
        return ERR_INVALID_VALUE;
    }

    AUDIO_INFO_LOG("Read audio_info_database with key: %{public}s value: %{public}s in uri=%{public}s ",
        key.c_str(), value.c_str(), uri.ToString().c_str());
    ret = StringToInt32(value, res);
    resultSet->Close();
    return ret;
}

ErrCode AudioSettingProvider::PutIntValues(std::vector<IntValueInfo> &infos, std::string tableType)
{
    std::string callingIdentity = IPCSkeleton::ResetCallingIdentity();
    int32_t ret = PutIntValuesInner(infos, tableType);
    IPCSkeleton::SetCallingIdentity(callingIdentity);
    return ret;
}

ErrCode AudioSettingProvider::PutIntValuesInner(std::vector<IntValueInfo> &infos, std::string tableType)
{
    auto helper =  CreateDataShareHelper(tableType);
    CHECK_AND_RETURN_RET_LOG(helper != nullptr, ERR_NO_INIT, "helper is null");
    for (auto &info : infos) {
        PutIntValueInner(helper, info.key, std::to_string(info.value), tableType);
    }
    ReleaseDataShareHelper(helper);
    return SUCCESS;
}

ErrCode AudioSettingProvider::PutIntValueInner(std::shared_ptr<DataShare::DataShareHelper> helper,
    std::string key, std::string value, std::string tableType)
{
    CHECK_AND_RETURN_RET_LOG(helper != nullptr, ERR_NO_INIT, "helper is null");
    AUDIO_INFO_LOG("Write audio_info_database with key: %{public}s value: %{public}s", key.c_str(), value.c_str());
    DataShare::DataShareValueObject keyObj(key);
    DataShare::DataShareValueObject valueObj(value);
    DataShare::DataShareValuesBucket bucket;
    bucket.Put(SETTING_COLUMN_KEYWORD, keyObj);
    bucket.Put(SETTING_COLUMN_VALUE, valueObj);
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(SETTING_COLUMN_KEYWORD, key);
    Uri uri(AssembleUri(key, tableType));
    if (helper->Update(uri, predicates, bucket) <= 0) {
        AUDIO_INFO_LOG("audio_info_database no data exist, insert one row");
        helper->Insert(uri, bucket);
    }
    helper->NotifyChange(AssembleUri(key, tableType));
    return SUCCESS;
}


void AudioSettingProvider::GetBoolValues(std::vector<BoolValueInfo> &infos, std::string tableType)
{
    std::string callingIdentity = IPCSkeleton::ResetCallingIdentity();
    for (auto &info : infos) {
        info.value = info.defaultValue;
    }
    GetBoolValuesInner(infos, tableType);
    IPCSkeleton::SetCallingIdentity(callingIdentity);
}

void AudioSettingProvider::GetBoolValuesInner(std::vector<BoolValueInfo> &infos, std::string tableType)
{
    auto helper = CreateDataShareHelper(tableType);
    CHECK_AND_RETURN_LOG(helper != nullptr, "helper is null");
    for (auto &info : infos) {
        bool res = false;
        int32_t ret = GetBoolValueInner(helper, info.key, tableType, res);
        CHECK_AND_CONTINUE(ret == SUCCESS);
        info.value = res;
    }
    ReleaseDataShareHelper(helper);
}

ErrCode AudioSettingProvider::GetBoolValueInner(std::shared_ptr<DataShare::DataShareHelper> helper,
    std::string key, std::string tableType, bool &res)
{
    CHECK_AND_RETURN_RET_LOG(helper != nullptr, ERR_NO_INIT, "helper is null");
    std::vector<std::string> columns = {SETTING_COLUMN_VALUE};
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(SETTING_COLUMN_KEYWORD, key);
    Uri uri(AssembleUri(key, tableType));
    auto resultSet = helper->Query(uri, predicates, columns);
    CHECK_AND_RETURN_RET_LOG(resultSet != nullptr, ERR_INVALID_OPERATION, "helper->Query return nullptr");

    int32_t count;
    resultSet->GetRowCount(count);
    if (count == 0) {
        AUDIO_WARNING_LOG("not found value, key=%{public}s, uri=%{public}s, count=%{public}d",
            key.c_str(), uri.ToString().c_str(), count);
        resultSet->Close();
        return ERR_NAME_NOT_FOUND;
    }
    const int32_t INDEX = 0;
    resultSet->GoToRow(INDEX);

    std::string value;
    int32_t ret = resultSet->GetString(INDEX, value);
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("resultSet->GetString return not ok, ret=%{public}d", ret);
        resultSet->Close();
        return ERR_INVALID_VALUE;
    }

    AUDIO_INFO_LOG("Read audio_info_database with key: %{public}s value: %{public}s in uri=%{public}s ",
        key.c_str(), value.c_str(), uri.ToString().c_str());
    res = (value == "true");
    resultSet->Close();
    return ret;
}
} // namespace AudioStandard
} // namespace OHOS
