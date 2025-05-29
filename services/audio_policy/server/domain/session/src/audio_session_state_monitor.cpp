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
#undef LOG_TAG
#define LOG_TAG "AudioSessionStateMonitor"

#include "audio_session_state_monitor.h"
#include "audio_policy_state_monitor.h"
#include "audio_policy_log.h"

namespace OHOS {
namespace AudioStandard {
constexpr time_t AUDIO_SESSION_TIME_OUT_DURATION_S = 60; // Audio session timeout duration : 60 seconds

class AudioSessionStateMonitorCallback : public AudioPolicyStateMonitorCallback {
public:
    AudioSessionStateMonitorCallback(int32_t pid, const std::shared_ptr<AudioSessionStateMonitor> &stateMonitor)
        : pid(pid), stateMonitor_(stateMonitor)
    {}
    
    void OnTimeOut() override
    {
        auto monitor = stateMonitor_.lock();
        if (monitor == nullptr) {
            AUDIO_ERR_LOG("stateMonitor_ is nullptr");
            return;
        }
        monitor->OnAudioSessionTimeOut(pid);
        monitor->RemoveFromMonitorMap(pid);
    }

private:
    int32_t pid;
    std::weak_ptr<AudioSessionStateMonitor> stateMonitor_;
};

void AudioSessionStateMonitor::StartMonitor(int32_t pid)
{
    std::unique_lock<std::mutex> lock(sessionMonitorMutex_);
    if (pidCbIdMap_.count(pid) > 0) {
        AUDIO_INFO_LOG("pid %{public}d monitor is already running", pid);
        return;
    }
    lock.unlock();

    auto cb = std::make_shared<AudioSessionStateMonitorCallback>(pid, GetSelfSharedPtr());
    int32_t cbId = DelayedSingleton<AudioPolicyStateMonitor>::GetInstance()->RegisterCallback(
        cb, AUDIO_SESSION_TIME_OUT_DURATION_S, CallbackType::ONE_TIME);
    if (cbId == INVALID_CB_ID) {
        AUDIO_ERR_LOG("Register AudioSessionStateMonitorCallback failed.");
    } else {
        std::unique_lock<std::mutex> lock(sessionMonitorMutex_);
        pidCbIdMap_[pid] = cbId;
    }
}

void AudioSessionStateMonitor::RemoveFromMonitorMap(int32_t pid)
{
    std::unique_lock<std::mutex> lock(sessionMonitorMutex_);
    if (pidCbIdMap_.count(pid) == 0) {
        AUDIO_INFO_LOG("Can't not find pid %{public}d from monitor map", pid);
        return;
    }

    pidCbIdMap_.erase(pid);
}

void AudioSessionStateMonitor::StopMonitor(int32_t pid)
{
    std::unique_lock<std::mutex> lock(sessionMonitorMutex_);
    if (pidCbIdMap_.count(pid) == 0) {
        AUDIO_INFO_LOG("pid %{public}d monitor has stopped running", pid);
        return;
    }

    int32_t cbId = pidCbIdMap_[pid];
    pidCbIdMap_.erase(pid);
    lock.unlock();
    
    DelayedSingleton<AudioPolicyStateMonitor>::GetInstance()->UnRegisterCallback(cbId);
}

} // namespace AudioStandard
} // namespace OHOS
