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

#include "audio_stream_checker_thread.h"
#include "audio_log.h"

namespace OHOS {
namespace AudioStandard {
const int64_t STREAM_CHECK_INTERVAL_TIME = 500000000;  // 500ms

std::shared_ptr<AudioStreamCheckerThread> AudioStreamCheckerThread::GetInstance()
{
    static std::shared_ptr<AudioStreamCheckerThread> streamCheckerThread =
        std::make_shared<AudioStreamCheckerThread>();
    return streamCheckerThread;
}

void AudioStreamCheckerThread::AddThreadTask(std::shared_ptr<AudioStreamChecker> checker)
{
    CHECK_AND_RETURN_LOG(checker != nullptr, "checker is nullptr");
    CHECK_AND_RETURN_LOG(checker->isNeedCreateThread_.load(), "this sessionid already in check");
    checker->isNeedCreateThread_.store(false);

    uint32_t prevCount = currentTaskCount_.fetch_add(1);
    {
        std::lock_guard<std::mutex> lock(checkerVecMutex_);
        checkerVec_.push_back(checker);
    }

    CHECK_AND_RETURN(prevCount == 0);
    auto weakThreadTask = weak_from_this();
    taskLoop_ = std::make_shared<AudioLoopThread>("StreamCheck");
    taskLoop_->PostTask([weakThreadTask] () {
        auto sharedThreadTask = weakThreadTask.lock();
        CHECK_AND_RETURN_LOG(sharedThreadTask, "ThreadTask is null");
        do {
            std::vector<std::shared_ptr<AudioStreamChecker>> tempCheckers;
            {
                std::lock_guard<std::mutex> lock(sharedThreadTask->checkerVecMutex_);
                tempCheckers = sharedThreadTask->checkerVec_;
            }
            for (auto it : tempCheckers) {
                it->MonitorCheckFrame();
            }
            ClockTime::RelativeSleep(STREAM_CHECK_INTERVAL_TIME);
        } while (sharedThreadTask->currentTaskCount_.load() != 0);
    });
}

void AudioStreamCheckerThread::DeleteThreadTask(std::shared_ptr<AudioStreamChecker> checker)
{
    {
        std::lock_guard<std::mutex> lock(checkerVecMutex_);
        auto it = std::find(checkerVec_.begin(), checkerVec_.end(), checker);
        if (it != checkerVec_.end()) {
            checkerVec_.erase(it);
            currentTaskCount_.fetch_sub(1);
        }
    }
    if (currentTaskCount_.load() == 0) {
        taskLoop_ = nullptr;
    }
}
}
}