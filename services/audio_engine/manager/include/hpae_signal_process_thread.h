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
#ifndef HPAE_SIGNAL_PROCESS_THREAD_H
#define HPAE_SIGNAL_PROCESS_THREAD_H
#include <atomic>
#include <thread>
#include <condition_variable>
#include <mutex>
#include "hpae_stream_manager.h"
namespace OHOS {
namespace AudioStandard {
namespace HPAE {
class HpaeSignalProcessThread {
public:
    HpaeSignalProcessThread() : running_(false), recvSignal_(false)
    {}
    ~HpaeSignalProcessThread();
    void ActivateThread(const std::weak_ptr<HpaeStreamManager>& streamManager);
    void DeactivateThread();
    void Notify();
    void Run();
    bool IsRunning() const
    {
        return running_.load();
    }
    bool IsMsgProcessing() const
    {
        return recvSignal_.load();
    }
private:

    std::atomic<bool> running_;
    std::atomic<bool> recvSignal_;
    std::weak_ptr<HpaeStreamManager> streamManager_;
    std::thread thread_;
    std::condition_variable condition_;
    std::mutex mutex_;
};
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif