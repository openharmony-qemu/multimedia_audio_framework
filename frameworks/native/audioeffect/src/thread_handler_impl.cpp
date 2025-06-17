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
#define LOG_TAG "AudioThreadHandler"
#endif

#include "thread_handler.h"

#include <future>
#include <unistd.h>

#include "audio_effect_log.h"
#include "audio_schedule.h"
#include "nocopyable.h"
#include "thread_pool.h"

namespace OHOS {
namespace AudioStandard {
namespace {
const int32_t THREAD_NUM = 1;
const size_t MAX_TASK_NUM = 5;
} // namespace

class ThreadHandlerImpl final : public ThreadHandler, public NoCopyable {
public:
    explicit ThreadHandlerImpl(const std::string &threadName);
    ~ThreadHandlerImpl() override;
    void PostTask(const Task &task) override;
    void EnsureTask(const Task &task) override;

private:
    std::shared_ptr<OHOS::ThreadPool> threadPool_ { nullptr };
};

ThreadHandlerImpl::ThreadHandlerImpl(const std::string &threadName)
{
    threadPool_ = std::make_shared<OHOS::ThreadPool>(threadName);
    if (threadPool_ == nullptr) {
        AUDIO_ERR_LOG("create thread fail");
        return;
    }
    threadPool_->Start(THREAD_NUM);
    threadPool_->SetMaxTaskNum(MAX_TASK_NUM);
}

ThreadHandlerImpl::~ThreadHandlerImpl()
{
    auto task = []() {
        UnscheduleThreadInServer(getpid(), gettid());
    };
    threadPool_->AddTask(task);
    threadPool_->Stop();
    AUDIO_INFO_LOG("destroy thread handler succ");
}

void ThreadHandlerImpl::PostTask(const Task &task)
{
    threadPool_->AddTask(task);
}

void ThreadHandlerImpl::EnsureTask(const Task &task)
{
    std::promise<void> ensure;
    auto callback = [&ensure]() {
        ensure.set_value();
        return;
    };
    threadPool_->AddTask(task);
    threadPool_->AddTask(callback);
    ensure.get_future().get();
}

std::shared_ptr<ThreadHandler> ThreadHandler::NewInstance(const std::string &threadName)
{
    auto handler = std::make_shared<ThreadHandlerImpl>(threadName);
    if (handler == nullptr) {
        AUDIO_ERR_LOG("create thread handler impl fail");
        return nullptr;
    }

    auto task = []() {
        ScheduleThreadInServer(getpid(), gettid());
    };
    handler->PostTask(task);

    return handler;
}
} // namespace AudioStandard
} // namespace OHOS