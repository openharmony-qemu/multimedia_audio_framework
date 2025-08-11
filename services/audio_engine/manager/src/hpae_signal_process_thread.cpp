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
    if (streamManager_.lock() != nullptr) {
        pthread_setname_np(thread_.native_handle(), streamManager_.lock()->GetThreadName().c_str());
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
    while (running_.load() && streamManager_.lock() != nullptr) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            condition_.wait(lock, [this] {
                return !running_.load() || streamManager_.lock()->IsRunning() ||
                    streamManager_.lock()->IsMsgProcessing() || recvSignal_.load();
            });
        }
        if (streamManager_.lock()) {
            streamManager_.lock()->HandleMsg();
            streamManager_.lock()->Process();
        }
        recvSignal_.store(false);
    }
    ResetThreadQosLevel();
}

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
