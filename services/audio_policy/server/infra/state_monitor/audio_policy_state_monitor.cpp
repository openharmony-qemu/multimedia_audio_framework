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
#define LOG_TAG "AudioPolicyStateMonitor"
#endif

#include <chrono>
#include "audio_policy_state_monitor.h"
#include "audio_policy_log.h"

namespace OHOS {
namespace AudioStandard {

AudioPolicyStateMonitor::AudioPolicyStateMonitor()
{
    if (stateMonitorThread_ == nullptr) {
        stateMonitorThread_ = std::make_shared<std::thread>([this] { CheckStateThreadMain(); });
        if (stateMonitorThread_ == nullptr) {
            AUDIO_ERR_LOG("Create state monitor thread failed");
        }
    }
}

AudioPolicyStateMonitor::~AudioPolicyStateMonitor()
{
    {
        std::unique_lock<std::mutex> condLock(condMutex_);
        threadStoped_ = true;
    }

    if (stateMonitorThread_ != nullptr && stateMonitorThread_->joinable()) {
        stateMonitorThread_->join();
        stateMonitorThread_ = nullptr;
    }
}

int32_t AudioPolicyStateMonitor::RegisterCallback(
    const std::shared_ptr<AudioPolicyStateMonitorCallback> &cb, std::time_t delayTime_, CallbackType callbackType)
{
    if (stateMonitorThread_ == nullptr) {
        AUDIO_ERR_LOG("RegisterCallback failed because no thread");
        return INVALID_CB_ID;
    }

    if (cb == nullptr) {
        AUDIO_ERR_LOG("RegisterCallback failed because cb is nullptr");
        return INVALID_CB_ID;
    }

    int32_t cbId = AllocateCbId();
    std::unique_lock<std::mutex> lock(monitorMutex_);
    if (cbId != INVALID_CB_ID) {
        cb->startTimeStamp_ = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        cb->delayTime_ = delayTime_;
        cb->callbackType_ = callbackType;
        monitoredObj_[cbId] = cb;
    }

    return cbId;
}

void AudioPolicyStateMonitor::UnRegisterCallback(int32_t cbId)
{
    if (stateMonitorThread_ == nullptr) {
        AUDIO_ERR_LOG("UnRegisterCallback failed because no thread");
        return;
    }

    std::unique_lock<std::mutex> lock(monitorMutex_);
    auto it = monitoredObj_.find(cbId);
    if (it != monitoredObj_.end()) {
        monitoredObj_.erase(it);
    }
    FreeCbId(cbId);
}

void AudioPolicyStateMonitor::TraverseAndInvokeTimeoutCallbacks()
{
    std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::unique_lock<std::mutex> monitorLock(monitorMutex_);
    auto it = monitoredObj_.begin();
    while (it != monitoredObj_.end()) {
        auto cb = it->second;
        if (cb == nullptr) {
            AUDIO_INFO_LOG("cb is nullptr");
            ++it;
            continue;
        }

        if (now - cb->startTimeStamp_ < cb->delayTime_) {
            ++it;
            continue;
        }

        // Running callback in a standalone thread
        std::thread callbackThread([](const std::shared_ptr<AudioPolicyStateMonitorCallback> cb) {
            if (cb == nullptr) {
                AUDIO_ERR_LOG("ExecCallbackInThread cb is nullptr");
                return;
            }
            cb->OnTimeOut();
        }, cb);
        callbackThread.detach();

        if (cb->callbackType_ == CallbackType::ONE_TIME) {
            FreeCbId(it->first);
            it = monitoredObj_.erase(it);
        } else {
            cb->startTimeStamp_ = now;
            ++it;
        }
    }
}

void AudioPolicyStateMonitor::CheckStateThreadMain()
{
    bool shouldExit = false;
    while (!shouldExit) {
        TraverseAndInvokeTimeoutCallbacks();

        std::unique_lock<std::mutex> condLock(condMutex_);
        shouldExit = stopCond_.wait_for(condLock, std::chrono::seconds(1), [this]() { return threadStoped_; });
    }

    AUDIO_INFO_LOG("AudioPolicyStateMonitor thread has been stopped.");
}

int32_t AudioPolicyStateMonitor::AllocateCbId()
{
    std::unique_lock<std::mutex> lock(monitorMutex_);
    for (int32_t i = 0; i < MAX_CB_ID_NUM; ++i) {
        if (!idAllocator_[i]) {
            idAllocator_[i] = true;
            return i;
        }
    }

    AUDIO_INFO_LOG("AllocateCbId failed because no free cbid");
    return INVALID_CB_ID;
}

void AudioPolicyStateMonitor::FreeCbId(int32_t cbId)
{
    if (cbId <= INVALID_CB_ID || cbId >= MAX_CB_ID_NUM) {
        AUDIO_INFO_LOG("Invalid cbId = %{public}d", cbId);
        return;
    }
    idAllocator_[cbId] = false;
}

} // namespace AudioStandard
} // namespace OHOS
