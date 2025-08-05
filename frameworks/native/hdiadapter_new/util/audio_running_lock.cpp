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
#define LOG_TAG "AudioRunningLock"
#endif

#include "util/audio_running_lock.h"
#include "audio_hdi_log.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
AudioRunningLock::AudioRunningLock(const std::string &lockName)
{
    runningLock_ = PowerMgr::PowerMgrClient::GetInstance().CreateRunningLock(lockName,
        PowerMgr::RunningLockType::RUNNINGLOCK_BACKGROUND_AUDIO);
    CHECK_AND_RETURN_LOG(runningLock_ != nullptr, "create lock fail");
}

int32_t AudioRunningLock::Lock(const int32_t timeoutMs)
{
    CHECK_AND_RETURN_RET_LOG(runningLock_ != nullptr, -1, "lock is nullptr");
    Trace lockTrace("AudioRunningLock::Lock");
    std::lock_guard<std::mutex> lock(mutex_);
    lastAppsUid_ = {};

    Trace innerLockTrace("AudioRunningLock::runningLock_->Lock");
    AudioXCollie audioXCollie("PowerMgr::RunningLock::Lock", LOCK_TIMEOUT_SECONDS,
        [](void *) {
            AUDIO_ERR_LOG("PowerMgr lock timeout");
        }, nullptr, AUDIO_XCOLLIE_FLAG_LOG);
    WatchTimeout guard("PowerMgr Lock timeout");
    int32_t ret = runningLock_->Lock(timeoutMs);
    isLocked_ = true;
    AUDIO_INFO_LOG("lock end, ret: %{public}d", ret);
    return ret;
}

int32_t AudioRunningLock::UnLock(void)
{
    CHECK_AND_RETURN_RET_LOG(runningLock_ != nullptr, -1, "lock is nullptr");
    Trace unlockTrace("AudioRunningLock::UnLock");
    std::lock_guard<std::mutex> lock(mutex_);
    isLocked_ = false;
    currentAppsUid_ = {};
    lastAppsUid_ = {};

    Trace innerUpdateWorkSourceTrace("AudioRunningLock::runningLock_->UpdateWorkSource");
    int32_t ret = runningLock_->UpdateWorkSource({});
    AUDIO_INFO_LOG("update work source end, ret: %{public}d", ret);
    Trace innerUnlockTrace("AudioRunningLock::runningLock_->UnLock");
    ret = runningLock_->UnLock();
    AUDIO_INFO_LOG("unlock end, ret: %{public}d", ret);
    return ret;
}

int32_t AudioRunningLock::UpdateAppsUidToPowerMgr(void)
{
    CHECK_AND_RETURN_RET_LOG(runningLock_ != nullptr, -1, "lock is nullptr");
    Trace trace("AudioRunningLock::UpdateAppsUidToPowerMgr");
    std::lock_guard<std::mutex> lock(mutex_);
    if ((!isLocked_) || currentAppsUid_ == lastAppsUid_) {
        return SUCCESS;
    }
    lastAppsUid_ = currentAppsUid_;
    std::vector<int32_t> appsUid;
    appsUid.insert(appsUid.end(), currentAppsUid_.begin(), currentAppsUid_.end());

    std::string appsUidInfo;
    for (auto uid : appsUid) {
        appsUidInfo += (std::to_string(uid) + ',');
    }

    Trace innerUpdateWorkSourceTrace("AudioRunningLock::runningLock_->UpdateWorkSource");
    int32_t ret = runningLock_->UpdateWorkSource(appsUid);
    AUDIO_INFO_LOG("uidInfo: %{public}s, size: %{public}zu, ret: %{public}d", appsUidInfo.c_str(),
        appsUid.size(), ret);
    return ret;
}

} // namespace AudioStandard
} // namespace OHOS
