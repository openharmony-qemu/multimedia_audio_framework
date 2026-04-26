/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioAbilityManager"
#endif

#include "audio_bundle_manager.h"
#include "audio_common_log.h"
#include "audio_utils.h"
#include "audio_errors.h"
#include "bundle_mgr_proxy.h"
#include "bundle_mgr_interface.h"
#include "iservice_registry.h"
#include "tokenid_kit.h"
#include "ipc_skeleton.h"
#include "access_token.h"
#include "accesstoken_kit.h"

namespace {
constexpr unsigned int GET_BUNDLE_TIME_OUT_SECONDS = 10;
constexpr int BUNDLE_MGR_SERVICE_SYS_ABILITY_ID = 401;
}

namespace OHOS {
namespace AudioStandard {

AudioBundleManager& AudioBundleManager::GetInstance()
{
    static AudioBundleManager instance;
    return instance;
}

std::string AudioBundleManager::FindBundleNameFromMap(int32_t uid)
{
    std::shared_lock<std::shared_mutex> lock(mapMutex_);
    auto it = bundleNameMap_.find(uid);
    if (it != bundleNameMap_.end()) {
        return it->second;
    }
    return "";
}

void AudioBundleManager::InsertBundleNameToMap(int32_t uid, const std::string &name)
{
    std::lock_guard<std::shared_mutex> lock(mapMutex_);
    bundleNameMap_[uid] = name;
}

void AudioBundleManager::EraseBundleNameFromMap(int32_t uid)
{
    std::lock_guard<std::shared_mutex> lock(mapMutex_);
    bundleNameMap_.erase(uid);
}

int32_t AudioBundleManager::GetUidByBundleName(std::string bundleName, int userId)
{
    return GetInstance().GetUidByBundleNameImpl(bundleName, userId);
}

std::string AudioBundleManager::GetBundleName()
{
    return GetInstance().GetBundleNameImpl();
}

std::string AudioBundleManager::GetBundleNameFromUid(int32_t callingUid)
{
    return GetInstance().GetBundleNameFromUidImpl(callingUid);
}

std::string AudioBundleManager::GetBundleNameFromUidCached(int32_t callingUid)
{
    return GetInstance().GetBundleNameFromUidCachedImpl(callingUid);
}

void AudioBundleManager::RemoveBundleNameByUid(int32_t callingUid)
{
    GetInstance().RemoveBundleNameByUidImpl(callingUid);
}

AppExecFwk::BundleInfo AudioBundleManager::GetBundleInfo()
{
    return GetInstance().GetBundleInfoImpl();
}

AppExecFwk::BundleInfo AudioBundleManager::GetBundleInfoFromUid(int32_t callingUid)
{
    return GetInstance().GetBundleInfoFromUidImpl(callingUid);
}

void AudioBundleManager::RemoveBundleInfoByUid(int32_t callingUid)
{
    GetInstance().RemoveBundleInfoByUidImpl(callingUid);
}

std::string AudioBundleManager::GetBundleNameByToken(const uint32_t tokenIdNum)
{
    return GetInstance().GetBundleNameByTokenImpl(tokenIdNum);
}

int32_t AudioBundleManager::IsBundleInstalled(
    const std::string &bundleName, int32_t userId, int32_t appIndex, bool &isInstalled)
{
    return GetInstance().IsBundleInstalledImpl(bundleName, userId, appIndex, isInstalled);
}

int32_t AudioBundleManager::GetUidByBundleNameImpl(std::string bundleName, int userId)
{
    AudioXCollie audioXCollie("AudioBundleManager::GetUidByBundleName",
        GET_BUNDLE_TIME_OUT_SECONDS, nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    WatchTimeout guard("AudioBundleManager::GetUidByBundleName");
    auto systemAbilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_AND_RETURN_RET_LOG(systemAbilityManager != nullptr, ERR_INVALID_PARAM, "systemAbilityManager is nullptr");
    guard.CheckCurrTimeout();

    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    CHECK_AND_RETURN_RET_PRELOG(remoteObject != nullptr, ERR_INVALID_PARAM, "remoteObject is nullptr");

    sptr<AppExecFwk::IBundleMgr> bundleMgrProxy = OHOS::iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
    CHECK_AND_RETURN_RET_LOG(bundleMgrProxy != nullptr, ERR_INVALID_PARAM, "bundleMgrProxy is nullptr");

    WatchTimeout reguard("bundleMgrProxy->GetUidByBundleName:GetUidByBundleName");
    int32_t iUid = bundleMgrProxy->GetUidByBundleName(bundleName, userId);
    reguard.CheckCurrTimeout();

    return iUid;
}

std::string AudioBundleManager::GetBundleNameImpl()
{
    std::string bundleName = GetBundleNameFromUid(IPCSkeleton::GetCallingUid());
    return bundleName;
}

std::string AudioBundleManager::GetBundleNameFromUidImpl(int32_t callingUid)
{
    std::lock_guard<std::mutex> processLock(processMutex_);

    std::string bundleName = FindBundleNameFromMap(callingUid);
    if (!bundleName.empty()) {
        return bundleName;
    }

    AudioXCollie audioXCollie("AudioBundleManager::GetBundleNameFromUid",
        GET_BUNDLE_TIME_OUT_SECONDS, nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    WatchTimeout guard("AudioBundleManager:GetBundleNameFromUid");
    auto systemAbilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_AND_RETURN_RET_LOG(systemAbilityManager != nullptr, "", "systemAbilityManager is nullptr");
    guard.CheckCurrTimeout();

    sptr<IRemoteObject> remoteObject = systemAbilityManager->CheckSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    CHECK_AND_RETURN_RET_PRELOG(remoteObject != nullptr, "", "remoteObject is nullptr");

    sptr<AppExecFwk::IBundleMgr> bundleMgrProxy = OHOS::iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
    CHECK_AND_RETURN_RET_LOG(bundleMgrProxy != nullptr, "", "bundleMgrProxy is nullptr");

    WatchTimeout reguard("bundleMgrProxy->GetBundleNameForUid:GetBundleNameFromUid");
    bundleMgrProxy->GetBundleNameForUid(callingUid, bundleName);
    reguard.CheckCurrTimeout();

    InsertBundleNameToMap(callingUid, bundleName);
    return bundleName;
}

std::string AudioBundleManager::GetBundleNameFromUidCachedImpl(int32_t callingUid)
{
    return FindBundleNameFromMap(callingUid);
}

void AudioBundleManager::RemoveBundleNameByUidImpl(int32_t callingUid)
{
    std::lock_guard<std::mutex> processLock(processMutex_);
    EraseBundleNameFromMap(callingUid);
}

AppExecFwk::BundleInfo AudioBundleManager::GetBundleInfoImpl()
{
    return GetBundleInfoFromUid(IPCSkeleton::GetCallingUid());
}

AppExecFwk::BundleInfo AudioBundleManager::GetBundleInfoFromUidImpl(int32_t callingUid)
{
    std::lock_guard<std::mutex> lock(bundleInfoMapMutex_);
    auto it = bundleInfoMap_.find(callingUid);
    if (it != bundleInfoMap_.end()) {
        return it->second;
    }

    AudioXCollie audioXCollie("AudioBundleManager::GetBundleInfoFromUid",
        GET_BUNDLE_TIME_OUT_SECONDS, nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    std::string bundleName = "";
    AppExecFwk::BundleInfo bundleInfo;
    WatchTimeout guard("AudioBundleManager:GetBundleInfoFromUid");
    auto systemAbilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_AND_RETURN_RET_LOG(systemAbilityManager != nullptr, bundleInfo, "systemAbilityManager is nullptr");
    guard.CheckCurrTimeout();

    sptr<IRemoteObject> remoteObject = systemAbilityManager->CheckSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    CHECK_AND_RETURN_RET_PRELOG(remoteObject != nullptr, bundleInfo, "remoteObject is nullptr");

    sptr<AppExecFwk::IBundleMgr> bundleMgrProxy = OHOS::iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
    CHECK_AND_RETURN_RET_LOG(bundleMgrProxy != nullptr, bundleInfo, "bundleMgrProxy is nullptr");

    WatchTimeout reguard("bundleMgrProxy->GetBundleNameForUid:GetBundleInfoFromUid");
    bundleMgrProxy->GetBundleNameForUid(callingUid, bundleName);

    bundleMgrProxy->GetBundleInfoV9(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT |
        AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITIES |
        AppExecFwk::BundleFlag::GET_BUNDLE_WITH_REQUESTED_PERMISSION |
        AppExecFwk::BundleFlag::GET_BUNDLE_WITH_EXTENSION_INFO |
        AppExecFwk::BundleFlag::GET_BUNDLE_WITH_HASH_VALUE,
        bundleInfo,
        AppExecFwk::Constants::ALL_USERID);
    reguard.CheckCurrTimeout();
    bundleInfoMap_[callingUid] = bundleInfo;
    return bundleInfo;
}

void AudioBundleManager::RemoveBundleInfoByUidImpl(int32_t callingUid)
{
    std::lock_guard<std::mutex> lock(bundleInfoMapMutex_);
    bundleInfoMap_.erase(callingUid);
}

std::string AudioBundleManager::GetBundleNameByTokenImpl(const uint32_t tokenIdNum)
{
    using namespace Security::AccessToken;
    AUDIO_INFO_LOG("GetBundlNameByToken id %{public}u", tokenIdNum);
    AccessTokenID tokenId = static_cast<AccessTokenID>(tokenIdNum);
    ATokenTypeEnum tokenType = AccessTokenKit::GetTokenTypeFlag(tokenId);
    CHECK_AND_RETURN_RET_LOG(tokenType == TOKEN_HAP || tokenType == TOKEN_NATIVE, "unknown",
        "invalid token type %{public}u", tokenType);
    if (tokenType == TOKEN_HAP) {
        HapTokenInfo tokenInfo = {};
        int32_t ret = AccessTokenKit::GetHapTokenInfo(tokenId, tokenInfo);
        CHECK_AND_RETURN_RET_LOG(ret == 0, "unknown-hap", "hap %{public}u failed: %{public}d", tokenIdNum, ret);
        return tokenInfo.bundleName;
    } else {
        NativeTokenInfo tokenInfo = {};
        int32_t ret = AccessTokenKit::GetNativeTokenInfo(tokenId, tokenInfo);
        CHECK_AND_RETURN_RET_LOG(ret == 0, "unknown-native", "native %{public}u failed: %{public}d", tokenIdNum, ret);
        return tokenInfo.processName;
    }
}

int32_t AudioBundleManager::IsBundleInstalledImpl(
    const std::string &bundleName, int32_t userId, int32_t appIndex, bool &isInstalled)
{
    AudioXCollie audioXCollie("AudioBundleManager::IsBundleInstalled",
        GET_BUNDLE_TIME_OUT_SECONDS, nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    WatchTimeout guard("AudioBundleManager:IsBundleInstalled");
    auto systemAbilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_AND_RETURN_RET_LOG(systemAbilityManager != nullptr, ERR_NULL_POINTER, "systemAbilityManager is nullptr");
    guard.CheckCurrTimeout();

    sptr<IRemoteObject> remoteObject = systemAbilityManager->CheckSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    CHECK_AND_RETURN_RET_PRELOG(remoteObject != nullptr, ERR_NULL_POINTER, "remoteObject is nullptr");

    sptr<AppExecFwk::IBundleMgr> bundleMgrProxy = OHOS::iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
    CHECK_AND_RETURN_RET_LOG(bundleMgrProxy != nullptr, ERR_NULL_POINTER, "bundleMgrProxy is nullptr");

    WatchTimeout reguard("bundleMgrProxy->IsBundleInstalled:IsBundleInstalled");
    int32_t ret = bundleMgrProxy->IsBundleInstalled(bundleName, userId, appIndex, isInstalled);
    reguard.CheckCurrTimeout();
    return ret;
}
} // namespace AudioStandard
} // namespace OHOS