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

#include "thread_pool.h"
#include <unistd.h>

namespace OHOS {
namespace AudioStandard {

ThreadPool::ThreadPool(const std::string &name) : name_(name) {}

ThreadPool::~ThreadPool()
{
    Stop();
}

void ThreadPool::SetMaxTaskNum(size_t n)
{
    maxTaskNum_ = (n == 0) ? 1 : n;
}

void ThreadPool::Start(size_t threadNum)
{
    Stop();
    stop_ = false;
    threadNum = (threadNum == 0) ? 1 : threadNum;
    for (size_t i = 0; i < threadNum; ++i) {
        workers_.emplace_back([this]() { Worker(); });
    }
}

void ThreadPool::Stop()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stop_ = true;
    }
    cond_.notify_all();
    condFull_.notify_all();
    for (auto &t : workers_) {
        if (t.joinable()) {
            t.join();
        }
    }
    workers_.clear();
    std::queue<std::function<void()>> empty;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        std::swap(taskQueue_, empty);
    }
}

void ThreadPool::Worker()
{
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cond_.wait(lock, [this]() { return stop_ || !taskQueue_.empty(); });
            if (stop_ && taskQueue_.empty()) {
                return;
            }
            task = std::move(taskQueue_.front());
            taskQueue_.pop();
            condFull_.notify_one();
        }
        task();
    }
}

} // namespace AudioStandard
} // namespace OHOS