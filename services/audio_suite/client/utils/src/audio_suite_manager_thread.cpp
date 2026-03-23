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
#include "audio_system_manager.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

static constexpr int32_t FRAME_DURATION_MS = 20;  // 20 ms, frame duration ms

AudioSuiteManagerThread::~AudioSuiteManagerThread()
{
    DeactivateThread();
}

void AudioSuiteManagerThread::ActivateThread(IAudioSuiteManagerThread *audioSuiteManager, const std::string &threadName)
{
    running_.store(true);
    m_audioSuiteManager = audioSuiteManager;
    auto threadFunc = std::bind(&AudioSuiteManagerThread::Run, this);
    thread_ = std::thread(threadFunc);
    int ret = pthread_setname_np(thread_.native_handle(), threadName.c_str());
    CHECK_AND_RETURN_LOG(ret == 0, "Failed to set thread name: %s", strerror(ret));
}

void AudioSuiteManagerThread::Run()
{
    AddThread();
    ScheduleThreadInServer(getpid(), gettid());
    while (running_.load() && m_audioSuiteManager != nullptr) {
        Start();
        {
            bool isProcessing = m_audioSuiteManager->IsMsgProcessing();
            std::unique_lock<std::mutex> lock(mutex_);
            bool signal = recvSignal_.load();
            Trace trace("AudioSuite runFunc:" + std::to_string(signal) +
                " isPorcessing:" + std::to_string(isProcessing));
            condition_.wait(lock, [this] { return m_audioSuiteManager->IsMsgProcessing() || recvSignal_.load(); });
        }
        m_audioSuiteManager->HandleMsg();
        recvSignal_.store(false);
        Stop();
    }
    UnscheduleThreadInServer(getpid(), gettid());
    RemoveThread();
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

int32_t AudioSuiteManagerThread::CreateGroup()
{
    CHECK_AND_RETURN_RET_LOG(AudioSystemManager::GetInstance() != nullptr, workGroupId_,
        "AudioSystemManager::GetInstance() failed!");
    workGroupId_ = AudioSystemManager::GetInstance()->CreateAudioWorkgroup();
    CHECK_AND_RETURN_RET_LOG(workGroupId_ > 0, workGroupId_, "CreateAudioWorkgroup failed.");
    AUDIO_INFO_LOG("CreateAudioWorkgroup success, group id:%{public}d", workGroupId_);
    return workGroupId_;
}

void AudioSuiteManagerThread::ReleaseGroup()
{
    CHECK_AND_RETURN_LOG(AudioSystemManager::GetInstance() != nullptr, "AudioSystemManager::GetInstance() failed!");
    AudioSystemManager::GetInstance()->ReleaseAudioWorkgroup(workGroupId_);
    AUDIO_INFO_LOG("ReleaseAudioWorkgroup, group id:%{public}d", workGroupId_);
    workGroupId_ = -1;
}

void AudioSuiteManagerThread::AddThread()
{
    CHECK_AND_RETURN_LOG(AudioSystemManager::GetInstance() != nullptr, "AudioSystemManager::GetInstance() failed!");
    CHECK_AND_RETURN_LOG(workGroupId_ > 0, "workgroup id is invalid.");
    int32_t ret = AudioSystemManager::GetInstance()->AddThreadToGroup(workGroupId_, gettid());
    AUDIO_INFO_LOG("Add current thread %{public}d to group %{public}d", gettid(), workGroupId_);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "AddThreadToGroup failed.");
}

void AudioSuiteManagerThread::RemoveThread()
{
    CHECK_AND_RETURN_LOG(AudioSystemManager::GetInstance() != nullptr, "AudioSystemManager::GetInstance() failed!");
    AudioSystemManager::GetInstance()->RemoveThreadFromGroup(workGroupId_, gettid());
    AUDIO_INFO_LOG("Remove thread %{public}d from group %{public}d", gettid(), workGroupId_);
}

void AudioSuiteManagerThread::Start()
{
    uint64_t startTime = 0;
    CHECK_AND_RETURN_LOG(AudioSystemManager::GetInstance() != nullptr, "AudioSystemManager::GetInstance() failed!");
    uint64_t deadlineTime = FRAME_DURATION_MS;
    std::unordered_map<int32_t, bool> threads{{gettid(), true}};
    bool needUpdatePrio = true;
    AudioSystemManager::GetInstance()->StartGroup(workGroupId_, startTime, deadlineTime,
        threads, needUpdatePrio);
}

void AudioSuiteManagerThread::Stop()
{
    CHECK_AND_RETURN_LOG(AudioSystemManager::GetInstance() != nullptr, "AudioSystemManager::GetInstance() failed!");
    AudioSystemManager::GetInstance()->StopGroup(workGroupId_);
}

void AudioSuiteManagerThread::SetWorkGroupId(int32_t workGroupId)
{
    workGroupId_ = workGroupId;
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS
