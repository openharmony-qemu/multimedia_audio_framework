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

#ifndef ST_AUDIO_SESSION_TIMER_H
#define ST_AUDIO_SESSION_TIMER_H

#include <chrono>
#include <memory>
#include <mutex>
#include <thread>

#include "audio_session_info.h"

namespace OHOS {
namespace AudioStandard {
class AudioSessionTimerCallback {
public:
    virtual ~AudioSessionTimerCallback() = default;

    virtual void OnAudioSessionTimeOut(const int32_t callerPid) = 0;
};

enum class TimerState {
    TIMER_INVALID = -1,
    TIMER_NEW = 0,
    TIMER_RUNNING = 1,
    TIMER_STOPPED = 2,
};

class AudioSessionTimer {
public:
    AudioSessionTimer();
    virtual ~AudioSessionTimer();

    void StartTimer(const int32_t callerPid);
    void StopTimer(const int32_t callerPid);
    bool IsSessionTimerRunning(const int32_t callerPid);
    int32_t SetAudioSessionTimerCallback(const std::shared_ptr<AudioSessionTimerCallback> sessionTimerCallback);

private:
    void SendSessionTimeOutCallback(const int32_t callerPid);
    void TimerLoopFunc();

    std::mutex sessionTimerMutex_;

    TimerState state_ = TimerState::TIMER_INVALID;
    std::unordered_map<int32_t, std::time_t> timerMap_;
    std::shared_ptr<std::thread> timerThread_;
    std::condition_variable timerCond_;
    std::mutex timerLoopMutex_;
    std::weak_ptr<AudioSessionTimerCallback> timerCallback_;
};
} // namespace AudioStandard
} // namespace OHOS

#endif // ST_AUDIO_SESSION_SERVICE_H