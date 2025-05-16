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
constexpr int32_t API_VERSION_REMAINDER = 1000;
constexpr int BUNDLE_MGR_SERVICE_SYS_ABILITY_ID = 401;
}

namespace OHOS {
namespace AudioStandard {
int32_t AudioBundleManager::GetUidByBundleName(std::string bundleName, int userId)
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

std::string AudioBundleManager::GetBundleName()
{
    std::string bundleName = GetBundleNameFromUid(IPCSkeleton::GetCallingUid());
    return bundleName;
}

std::string AudioBundleManager::GetBundleNameFromUid(int32_t callingUid)
{
    AudioXCollie audioXCollie("AudioBundleManager::GetBundleNameFromUid",
        GET_BUNDLE_TIME_OUT_SECONDS, nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    std::string bundleName = "";
    WatchTimeout guard("AudioBundleManager:GetBundleNameFromUid");
    auto systemAbilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_AND_RETURN_RET_LOG(systemAbilityManager != nullptr, "", "systemAbilityManager is nullptr");
    guard.CheckCurrTimeout();

    sptr<IRemoteObject> remoteObject = systemAbilityManager->CheckSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    CHECK_AND_RETURN_RET_PRELOG(remoteObject != nullptr, "", "remoteObject is nullptr");

    sptr<AppExecFwk::IBundleMgr> bundleMgrProxy = OHOS::iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
    CHECK_AND_RETURN_RET_LOG(bundleMgrProxy != nullptr, "", "bundleMgrProxy is nullptr");

    WatchTimeout reguard("bundleMgrProxy->GetNameForUid:GetBundleNameFromUid");
    bundleMgrProxy->GetNameForUid(callingUid, bundleName);
    reguard.CheckCurrTimeout();

    return bundleName;
}

AppExecFwk::BundleInfo AudioBundleManager::GetBundleInfo()
{
    return GetBundleInfoFromUid(IPCSkeleton::GetCallingUid());
}

AppExecFwk::BundleInfo AudioBundleManager::GetBundleInfoFromUid(int32_t callingUid)
{
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

    WatchTimeout reguard("bundleMgrProxy->GetNameForUid:GetBundleInfoFromUid");
    bundleMgrProxy->GetNameForUid(callingUid, bundleName);

    bundleMgrProxy->GetBundleInfoV9(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT |
        AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITIES |
        AppExecFwk::BundleFlag::GET_BUNDLE_WITH_REQUESTED_PERMISSION |
        AppExecFwk::BundleFlag::GET_BUNDLE_WITH_EXTENSION_INFO |
        AppExecFwk::BundleFlag::GET_BUNDLE_WITH_HASH_VALUE,
        bundleInfo,
        AppExecFwk::Constants::ALL_USERID);
    reguard.CheckCurrTimeout();

    return bundleInfo;
}
} // namespace AudioStandard
} // namespace OHOS