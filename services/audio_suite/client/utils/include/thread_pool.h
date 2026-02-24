/*
* Copyright (c) 2026 Huawei Device Co., Ltd.
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

#pragma once
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace OHOS {
namespace AudioStandard {

class ThreadPool {
public:
    explicit ThreadPool(const std::string &name = "AudioSuitePullPool");
    ~ThreadPool();

    void Start(size_t threadNum);
    void Stop();
    void SetMaxTaskNum(size_t n);
    template <class F, class... Args>
    auto Submit(F &&f, Args &&... args) -> std::future<std::invoke_result_t<F, Args...>>
    {
        using Ret = std::invoke_result_t<F, Args...>;
        auto task =
            std::make_shared<std::packaged_task<Ret()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        std::future<Ret> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if (stop_) {
                auto dummy = std::make_shared<std::packaged_task<Ret()>>(
                    []() -> Ret { return Ret{}; }
                );
                return dummy->get_future();
            }
            condFull_.wait(lock, [this]() { return taskQueue_.size() < maxTaskNum_ || stop_; });
            taskQueue_.emplace([task]() { (*task)(); });
        }
        cond_.notify_one();
        return res;
    }

private:
    void Worker();

    std::string name_;
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> taskQueue_;
    std::mutex mutex_;
    std::condition_variable cond_;
    std::condition_variable condFull_;
    bool stop_ {false};
    size_t maxTaskNum_ {32};
};

} // namespace AudioStandard
} // namespace OHOS