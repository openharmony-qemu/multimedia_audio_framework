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
#include "audio_thread_task.h"
#include <pthread.h>
#include "ipc_skeleton.h"
#include "audio_schedule_guard.h"

namespace OHOS {
namespace AudioStandard {
constexpr int32_t TIME_OUT_MS = 500;
constexpr int32_t MAX_THREAD_NAME_LENGTH = 15;
AudioThreadTask::AudioThreadTask(const std::string &name) : name_(name), state_(RunningState::STOPPED), loop_(nullptr)
{
}

AudioThreadTask::~AudioThreadTask()
{
    state_ = RunningState::STOPPED;
    cond_.notify_all();
    if (loop_) {
        if (loop_->joinable()) {
            loop_->join();
        }
        loop_ = nullptr;
    }
}

void AudioThreadTask::Start()
{
    std::unique_lock lock(stateMutex_);
    if (state_.load() == RunningState::STOPPING) {
        cond_.wait(lock, [this] { return state_.load() == RunningState::STOPPED; });
    }
    if (state_.load() == RunningState::STOPPED) {
        if (loop_) {
            if (loop_->joinable()) {
                loop_->join();
            }
            loop_ = nullptr;
        }
    }

    state_ = RunningState::STARTED;

    if (!loop_) {
        loop_ = std::make_unique<std::thread>([this] { this->RunJob(); });
        pthread_setname_np(loop_->native_handle(), name_.substr(0, MAX_THREAD_NAME_LENGTH).c_str());
    }
    cond_.notify_all();
}

void AudioThreadTask::Stop()
{
    std::unique_lock lock(stateMutex_);
    if (state_.load() != RunningState::STOPPED) {
        state_ = RunningState::STOPPING;
        cond_.notify_all();
        cond_.wait(lock, [this] { return state_.load() == RunningState::STOPPED; });
        if (loop_) {
            if (loop_->joinable()) {
                loop_->join();
            }
            loop_ = nullptr;
        }
    }
}

bool AudioThreadTask::CheckThreadIsRunning() const noexcept
{
    return state_.load() == RunningState::STARTED;
}

void AudioThreadTask::StopAsync()
{
    std::lock_guard lock(stateMutex_);
    if (state_.load() != RunningState::STOPPING && state_.load() != RunningState::STOPPED) {
        state_ = RunningState::STOPPING;
        cond_.notify_all();
    }
}

void AudioThreadTask::Pause()
{
    std::unique_lock lock(stateMutex_);
    switch (state_.load()) {
        case RunningState::STARTED: {
            state_ = RunningState::PAUSING;
            cond_.wait(lock, [this] {
                return state_.load() == RunningState::PAUSED || state_.load() == RunningState::STOPPED;
            });
            break;
        }
        case RunningState::STOPPING: {
            cond_.wait(lock, [this] { return state_.load() == RunningState::STOPPED; });
            break;
        }
        case RunningState::PAUSING: {
            cond_.wait(lock, [this] { return state_.load() == RunningState::PAUSED; });
            break;
        }
        default:
            break;
    }
}

void AudioThreadTask::PauseAsync()
{
    std::lock_guard lock(stateMutex_);
    if (state_.load() == RunningState::STARTED) {
        state_ = RunningState::PAUSING;
    }
}

void AudioThreadTask::RegisterJob(std::function<void()> &&job)
{
    job_ = std::move(job);
}

void AudioThreadTask::doEmptyJob() {}

void AudioThreadTask::RunJob()
{
    AudioScheduleGuard scheduleGuard(getpid(), gettid());
    for (;;) {
        if (state_.load() == RunningState::STARTED) {
            job_();
        }
        std::unique_lock lock(stateMutex_);
        if (state_.load() == RunningState::PAUSING || state_.load() == RunningState::PAUSED) {
            state_ = RunningState::PAUSED;
            cond_.notify_all();
            cond_.wait_for(lock, std::chrono::milliseconds(TIME_OUT_MS),
                [this] { return state_.load() != RunningState::PAUSED; });
        }
        if (state_.load() == RunningState::STOPPING || state_.load() == RunningState::STOPPED) {
            state_ = RunningState::STOPPED;
            cond_.notify_all();
            break;
        }
    }
}
} // namespace AudioStandard
} // namespace OHOS
