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

#ifndef AUDIO_RUNNING_LOCK_H
#define AUDIO_RUNNING_LOCK_H

#ifdef FEATURE_POWER_MANAGER
#include <iostream>
#include <unordered_set>
#include <mutex>
#include "running_lock.h"
#include "power_mgr_client.h"
#include "audio_utils.h"
#endif

namespace OHOS {
namespace AudioStandard {
#ifdef FEATURE_POWER_MANAGER
class AudioRunningLock {
public:
    explicit AudioRunningLock(const std::string &lockName);
    int32_t Lock(const int32_t timeoutMs);
    int32_t UnLock(void);
    template<typename T>
    void UpdateAppsUid(const T &itBegin, const T &itEnd)
    {
        Trace trace("AudioRunningLock::UpdateAppsUid");
        std::lock_guard<std::mutex> lock(mutex_);
        std::unordered_set<int32_t> appsUidSet(itBegin, itEnd);
        currentAppsUid_ = std::move(appsUidSet);
    }
    int32_t UpdateAppsUidToPowerMgr(void);

private:
    static constexpr uint32_t LOCK_TIMEOUT_SECONDS = 8;

    std::shared_ptr<PowerMgr::RunningLock> runningLock_ = nullptr;
    std::mutex mutex_;
    std::unordered_set<int32_t> currentAppsUid_;
    std::unordered_set<int32_t> lastAppsUid_;
    std::atomic<bool> isLocked_ = false;
};
#endif

} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_RUNNING_LOCK_H
