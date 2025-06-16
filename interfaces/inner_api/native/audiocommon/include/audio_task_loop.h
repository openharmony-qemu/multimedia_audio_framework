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
#ifndef AUDIO_TASK_LOOP_H
#define AUDIO_TASK_LOOP_H

#include <atomic>
#include <thread>
#include <functional>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <queue>
namespace OHOS {
namespace AudioStandard {
class AudioTaskLoop {
public:
    friend class AudioLoopThread;

    AudioTaskLoop() = default;

    ~AudioTaskLoop() = default;

    AudioTaskLoop(const AudioTaskLoop&) = delete;
    AudioTaskLoop(AudioTaskLoop&&) = delete;
    AudioTaskLoop& operator=(const AudioTaskLoop&) = delete;
    AudioTaskLoop& operator=(AudioTaskLoop&&) = delete;

    void AsyncExit()
    {
        std::lock_guard lock(mutex_);
        isExited_ = true;
        cV_.notify_all();
    }

    void PostTask(const std::function<void()> &task)
    {
        std::lock_guard lock(mutex_);
        tasks_.push(task);
        cV_.notify_all();
    }

    void PostTask(std::function<void()> &&task)
    {
        std::lock_guard lock(mutex_);
        tasks_.push(std::move(task));
        cV_.notify_all();
    }
private:
    void Wait()
    {
        std::unique_lock lock(mutex_);
        cV_.wait(lock, [this] () {
            return (isExited_ || !tasks_.empty());
        });
    }

    static void ProcessTasks(std::queue<std::function<void()>> &tasks)
    {
        while (!tasks.empty()) {
            auto &&task = tasks.front();
            if (task) {
                task();
            }
            tasks.pop();
        }
    }

    void ProcessTasks()
    {
        std::queue<std::function<void()>> tasks;
        {
            std::lock_guard lock(mutex_);
            tasks.swap(tasks_);
        }
        ProcessTasks(tasks);
    }

    void Loop()
    {
        while (true) {
            Wait();
            ProcessTasks();

            std::lock_guard lock(mutex_);
            if (isExited_ && tasks_.empty()) { return; }
        }
    }

    std::queue<std::function<void()>> tasks_;
    std::mutex mutex_;
    std::condition_variable cV_;
    bool isExited_ = false;
};

class AudioLoopThread {
public:
    AudioLoopThread(const std::string &threadName)
    {
        if (loop_ != nullptr) {
            auto strongRef = loop_;
            std::thread loopThread([strongRef] () {
                strongRef->Loop();
            });
            pthread_setname_np(loopThread.native_handle(), threadName.c_str());
            loopThread.detach();
        }
    }

    AudioLoopThread(const AudioLoopThread&) = delete;
    AudioLoopThread(AudioLoopThread&&) = delete;
    AudioLoopThread& operator=(const AudioLoopThread&) = delete;
    AudioLoopThread& operator=(AudioLoopThread&&) = delete;

    void PostTask(const std::function<void()> &task)
    {
        if (loop_ != nullptr) {
            loop_->PostTask(task);
        }
    }

    void PostTask(std::function<void()> &&task)
    {
        if (loop_ != nullptr) {
            loop_->PostTask(std::move(task));
        }
    }

    ~AudioLoopThread()
    {
        if (loop_ != nullptr) {
            loop_->AsyncExit();
        }
    }

private:
    std::shared_ptr<AudioTaskLoop> loop_ = std::make_shared<AudioTaskLoop>();
};
} // namespace AudioStandard
} // namespace OHOS
#endif
