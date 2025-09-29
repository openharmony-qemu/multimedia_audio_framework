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
#define LOG_TAG "AudioEditManagerThread"
#endif

#include <any>
#include <mutex>
#include <thread>
#include <functional>
#include <unistd.h>
#include "audio_utils.h"
#include "audio_suite_log.h"
#include "audio_schedule.h"
#include "audio_suite_engine.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

AudioSuiteManagerThread::~AudioSuiteManagerThread()
{
    DeactivateThread();
}

void AudioSuiteManagerThread::ActivateThread(IAudioSuiteManagerThread *audioSuiteManager)
{
    running_.store(true);
    m_audioSuiteManager = audioSuiteManager;
    auto threadFunc = std::bind(&AudioSuiteManagerThread::Run, this);
    thread_ = std::thread(threadFunc);
    pthread_setname_np(thread_.native_handle(), "AudioSuiteManager");
}

void AudioSuiteManagerThread::Run()
{
    ScheduleThreadInServer(getpid(), gettid());
    while (running_.load() && m_audioSuiteManager != nullptr) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            bool isProcessing = m_audioSuiteManager->IsMsgProcessing();
            bool signal = recvSignal_.load();
            Trace trace("AudioSuite runFunc:" + std::to_string(signal) +
                " isPorcessing:" + std::to_string(isProcessing));
            condition_.wait(lock, [this] { return m_audioSuiteManager->IsMsgProcessing() || recvSignal_.load(); });
        }
        m_audioSuiteManager->HandleMsg();
        recvSignal_.store(false);
    }
    UnscheduleThreadInServer(getpid(), gettid());
}

void AudioSuiteManagerThread::Notify()
{
    std::unique_lock<std::mutex> lock(mutex_);
    recvSignal_.store(true);
    condition_.notify_all();
}

void AudioSuiteManagerThread::DeactivateThread()
{
    running_.store(false);
    Notify();
    if (thread_.joinable()) {
        thread_.join();
    }
    AUDIO_INFO_LOG("DeactivateThread finish.");
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS
