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
#undef LOG_TAG
#define LOG_TAG "AudioSessionTimer"

#include "audio_session_timer.h"

#include "audio_log.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
constexpr time_t SECONDS_OF_ONE_MINUTE = 60;

AudioSessionTimer::AudioSessionTimer()
{
    state_ = TimerState::TIMER_NEW;
}

AudioSessionTimer::~AudioSessionTimer()
{
}

void AudioSessionTimer::StartTimer(const int32_t callerPid)
{
    AUDIO_INFO_LOG("Audio session state change: StartTimer for pid %{public}d", callerPid);
    std::lock_guard<std::mutex> lock(sessionTimerMutex_);
    if (timerMap_.count(callerPid) != 0) {
        AUDIO_INFO_LOG("StartTimer: timer of callerPid %{public}d is already running", callerPid);
        // the time point will not be updated.
        return;
    }
    std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    timerMap_[callerPid] = now;
    if (!timerMap_.empty()) {
        std::lock_guard<std::mutex> lock(timerLoopMutex_);
        state_ = TimerState::TIMER_RUNNING;
    }

    if (timerThread_ != nullptr && timerThread_->joinable()) {
        // the old thread has been used and can be reset.
        timerThread_->join();
        timerThread_ = nullptr;
    }
    if (timerThread_ == nullptr) {
        timerThread_ = std::make_shared<std::thread>([this] { TimerLoopFunc(); });
    }
}

void AudioSessionTimer::StopTimer(const int32_t callerPid)
{
    AUDIO_INFO_LOG("Audio session state change: StopTimer for pid %{public}d", callerPid);
    std::lock_guard<std::mutex> lock(sessionTimerMutex_);
    if (timerMap_.count(callerPid) == 0) {
        AUDIO_WARNING_LOG("StopTimer: timer of callerPid %{public}d is already stopped", callerPid);
    }
    timerMap_.erase(callerPid);
    if (timerMap_.empty()) {
        {
            std::lock_guard<std::mutex> lock(timerLoopMutex_);
            state_ = TimerState::TIMER_STOPPED;
        }
        timerCond_.notify_all();
    }
}

bool AudioSessionTimer::IsSessionTimerRunning(const int32_t callerPid)
{
    AUDIO_INFO_LOG("IsSessionTimerRunning: callerPid %{public}d", callerPid);
    std::lock_guard<std::mutex> lock(sessionTimerMutex_);
    bool isRunning = (timerMap_.count(callerPid) > 0);
    AUDIO_INFO_LOG("IsSessionTimerRunning: callerPid %{public}d, result %{public}d", callerPid, isRunning);
    return isRunning;
}

void AudioSessionTimer::TimerLoopFunc()
{
    AUDIO_INFO_LOG("Start the session timer loop");
    for (;;) {
        std::unique_lock<std::mutex> lock(sessionTimerMutex_);
        if (timerMap_.empty()) {
            AUDIO_INFO_LOG("The audio session timer map is empty. Exit.");
            break;
        }

        std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        auto iter = timerMap_.begin();
        while (iter != timerMap_.end()) {
            if (now - iter->second >= SECONDS_OF_ONE_MINUTE) {
                SendSessionTimeOutCallback(iter->first);
                iter = timerMap_.erase(iter);
            } else {
                ++iter;
            }
        }
        lock.unlock();

        // Sleep for one second. Then enter the next cycle.
        std::unique_lock<std::mutex> loopLock(timerLoopMutex_);
        bool waitResult = timerCond_.wait_for(loopLock, std::chrono::seconds(1),
            [this]() { return (state_ == TimerState::TIMER_STOPPED); });
        if (!waitResult) {
            AUDIO_DEBUG_LOG("sleep 1s. continue.");
        }
        if (state_ == TimerState::TIMER_STOPPED) {
            AUDIO_INFO_LOG("The audio session timer has been stopped!");
            break;
        }
    }
}

void AudioSessionTimer::SendSessionTimeOutCallback(const int32_t callerPid)
{
    std::shared_ptr<AudioSessionTimerCallback> cb = timerCallback_.lock();
    if (cb == nullptr) {
        AUDIO_ERR_LOG("The audio session timer callback is nullptr!");
        return;
    }
    cb->OnAudioSessionTimeOut(callerPid);
}

int32_t AudioSessionTimer::SetAudioSessionTimerCallback(
    const std::shared_ptr<AudioSessionTimerCallback> sessionTimerCallback)
{
    AUDIO_INFO_LOG("SetAudioSessionTimerCallback in");
    if (sessionTimerCallback == nullptr) {
        AUDIO_ERR_LOG("sessionTimerCallback is nullptr!");
        return ERR_INVALID_PARAM;
    }

    std::lock_guard<std::mutex> lock(sessionTimerMutex_);
    timerCallback_ = sessionTimerCallback;
    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS
