/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "AppBundleManager"
#endif

#include "app_bundle_manager.h"

#include "system_ability_definition.h"
#include "bundle_mgr_interface.h"
#include "bundle_mgr_proxy.h"
#include "iservice_registry.h"

#include "audio_common_log.h"
#include "audio_errors.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {
static constexpr unsigned int GET_BUNDLE_TIME_OUT_SECONDS = 10;
constexpr int32_t BOOTUP_MUSIC_UID = 1003;
std::string AppBundleManager::GetBundleNameFromUid(int32_t uid)
{
    CHECK_AND_RETURN_RET_LOG(uid != BOOTUP_MUSIC_UID, "", "no need query from BMS.");
    AudioXCollie audioXCollie("AudioServer::GetBundleNameFromUid",
        GET_BUNDLE_TIME_OUT_SECONDS, nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    std::string bundleName {""};
    WatchTimeout guard("SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager():GetBundleNameFromUid");
    auto systemAbilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_AND_RETURN_RET_LOG(systemAbilityManager != nullptr, "", "systemAbilityManager is nullptr");
    guard.CheckCurrTimeout();

    sptr<IRemoteObject> remoteObject = systemAbilityManager->CheckSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    CHECK_AND_RETURN_RET_LOG(remoteObject != nullptr, "", "remoteObject is nullptr");

    sptr<AppExecFwk::IBundleMgr> bundleMgrProxy = OHOS::iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
    CHECK_AND_RETURN_RET_LOG(bundleMgrProxy != nullptr, "", "bundleMgrProxy is nullptr");

    WatchTimeout reguard("bundleMgrProxy->GetNameForUid:GetBundleNameFromUid");
    bundleMgrProxy->GetNameForUid(uid, bundleName);
    reguard.CheckCurrTimeout();

    return bundleName;
}
} // namespace AudioStandard
} // namespace OHOS