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
#ifndef AUDIO_SUITE_MANAGER_THREAD_H
#define AUDIO_SUITE_MANAGER_THREAD_H

#include <any>
#include <mutex>
#include <thread>
#include <functional>
#include <condition_variable>
#include "i_audio_suite_manager_thread.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

class AudioSuiteManagerThread {
public:
    AudioSuiteManagerThread() : running_(false)
    {}
    ~AudioSuiteManagerThread();
    void ActivateThread(IAudioSuiteManagerThread *audioSuiteManager);
    void DeactivateThread();
    void Run();
    void Notify();
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
    IAudioSuiteManagerThread *m_audioSuiteManager = nullptr;
    std::condition_variable condition_;
    std::mutex mutex_;
    std::thread thread_;
};

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // AUDIO_SUITE_MANAGER_THREAD_H