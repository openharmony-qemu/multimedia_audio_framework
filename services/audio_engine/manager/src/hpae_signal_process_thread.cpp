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
#include "hpae_signal_process_thread.h"
#include "audio_qosmanager.h"
#include "parameter.h"
#include "audio_engine_log.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
HpaeSignalProcessThread::~HpaeSignalProcessThread()
{
    DeactivateThread();
}

void HpaeSignalProcessThread::ActivateThread(const std::weak_ptr<HpaeStreamManager> &streamManager)
{
    streamManager_ = streamManager;
    running_.store(true);
    auto threadFunc = std::bind(&HpaeSignalProcessThread::Run, this);
    thread_ = std::thread(threadFunc);
    if (auto manager = streamManager_.lock()) {
        pthread_setname_np(thread_.native_handle(), manager->GetThreadName().c_str());
    }
}

void HpaeSignalProcessThread::DeactivateThread()
{
    running_.store(false);
    Notify();
    if (thread_.joinable()) {
        thread_.join();
    }
}

void HpaeSignalProcessThread::Notify()
{
    std::unique_lock<std::mutex> lock(mutex_);
    recvSignal_.store(true);
    condition_.notify_all();
}

void HpaeSignalProcessThread::Run()
{
    int32_t setPriority = GetIntParameter("const.multimedia.audio_setPriority", 1);
    SetThreadQosLevelAsync(setPriority);
    auto manager = streamManager_.lock();
    while (running_.load() && manager != nullptr) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            condition_.wait(lock, [this, manager] {
                return !running_.load() || manager->IsRunning() ||
                    manager->IsMsgProcessing() || recvSignal_.load();
            });
        }
        manager->HandleMsg();
        manager->Process();
        recvSignal_.store(false);
    }
    ResetThreadQosLevel();
}

void HpaeSignalProcessThread::SleepUntilNotify(int64_t sleepInUs)
{
    auto manager = streamManager_.lock();
    CHECK_AND_RETURN(manager);
    std::unique_lock<std::mutex> lock(mutex_);
    auto duration = std::chrono::microseconds(sleepInUs);
    condition_.wait_for(lock, duration, [this, manager] {
        return !running_.load() ||
               manager->IsMsgProcessing() ||
               recvSignal_.load();
    });
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
