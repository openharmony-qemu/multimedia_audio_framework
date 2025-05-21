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
    AUDIO_INFO_LOG("Background task start with: uid:%{public}d, pid:%{public}d", uid, pid);
    AudioBackgroundManager::GetInstance().NotifyBackgroundTaskStateChange(uid, pid, true);
}

void BackgroundTaskListener::OnContinuousTaskStop(
    const std::shared_ptr<ContinuousTaskCallbackInfo> &ContinuousTaskCallbackInfo)
{
    CHECK_AND_RETURN_LOG(ContinuousTaskCallbackInfo != nullptr, "ContinuousTaskCallbackInfo is nullptr");
    auto uid = ContinuousTaskCallbackInfo->GetCreatorUid();
    auto pid = ContinuousTaskCallbackInfo->GetCreatorPid();
    AUDIO_INFO_LOG("Background task stop with: uid:%{public}d, pid:%{public}d", uid, pid);
    AudioBackgroundManager::GetInstance().NotifyBackgroundTaskStateChange(uid, pid, false);
}
}
}
