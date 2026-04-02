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
#define LOG_TAG "AudioBackgroundTaskListener"
#endif

#include "audio_common_log.h"
#include "audio_background_manager.h"
#include "background_task_listener.h"


namespace OHOS {
namespace AudioStandard {
void BackgroundTaskListener::OnContinuousTaskStart(
    const std::shared_ptr<ContinuousTaskCallbackInfo> &ContinuousTaskCallbackInfo)
{
    CHECK_AND_RETURN_LOG(ContinuousTaskCallbackInfo != nullptr, "ContinuousTaskCallbackInfo is nullptr");
    auto uid = ContinuousTaskCallbackInfo->GetCreatorUid();
    auto pid = ContinuousTaskCallbackInfo->GetCreatorPid();
    auto continuousTaskId = ContinuousTaskCallbackInfo->GetContinuousTaskId();
    AUDIO_INFO_LOG("Background task start with: uid:%{public}d, pid:%{public}d, continuousTaskId:%{public}d",
        uid, pid, continuousTaskId);
    AddTaskId(pid, continuousTaskId);
    size_t size = GetSizeOfContinuousTaskIdSet(pid);
    if (size == 1) {
        AudioBackgroundManager::GetInstance().NotifyBackgroundTaskStateChange(uid, pid, true);
    }
}

void BackgroundTaskListener::OnContinuousTaskStop(
    const std::shared_ptr<ContinuousTaskCallbackInfo> &ContinuousTaskCallbackInfo)
{
    CHECK_AND_RETURN_LOG(ContinuousTaskCallbackInfo != nullptr, "ContinuousTaskCallbackInfo is nullptr");
    auto uid = ContinuousTaskCallbackInfo->GetCreatorUid();
    auto pid = ContinuousTaskCallbackInfo->GetCreatorPid();
    auto continuousTaskId = ContinuousTaskCallbackInfo->GetContinuousTaskId();
    DeleteTaskId(pid, continuousTaskId);
    size_t size = GetSizeOfContinuousTaskIdSet(pid);
    if (size == 0) {
        AudioBackgroundManager::GetInstance().NotifyBackgroundTaskStateChange(uid, pid, false);
    }
}

void BackgroundTaskListener::AddTaskId(int32_t pid, int32_t continuousTaskId)
{
    std::lock_guard<std::mutex> lock(appBackTaskTdMapMutex_);
    AUDIO_INFO_LOG("Background task add taskid with:pid:%{public}d, continuousTaskId:%{public}d",
        pid, continuousTaskId);
    continuousTaskIdMap_[pid].insert(continuousTaskId);
}

void BackgroundTaskListener::DeleteTaskId(int32_t pid, int32_t continuousTaskId)
{
    std::lock_guard<std::mutex> lock(appBackTaskTdMapMutex_);
    AUDIO_INFO_LOG("Background task delete taskid with:pid:%{public}d, continuousTaskId:%{public}d",
        pid, continuousTaskId);
    continuousTaskIdMap_[pid].erase(continuousTaskId);
}

size_t BackgroundTaskListener::GetSizeOfContinuousTaskIdSet(int32_t pid)
{
    std::lock_guard<std::mutex> lock(appBackTaskTdMapMutex_);
    AUDIO_INFO_LOG("Background task get size of continuous task Id set pid:%{public}d", pid);
    if (continuousTaskIdMap_.find(pid) != continuousTaskIdMap_.end()) {
        return continuousTaskIdMap_[pid].size();
    }
    return 0;
}
}
}
