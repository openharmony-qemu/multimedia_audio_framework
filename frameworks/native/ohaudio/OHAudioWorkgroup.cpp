/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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

#include "OHAudioWorkgroup.h"
#include "audio_log.h"
#include <unistd.h>
#include <fcntl.h>

namespace OHOS {
namespace AudioStandard {

OHAudioWorkgroup::OHAudioWorkgroup(int id) : workgroupId_(id)
{
    AUDIO_INFO_LOG("OHAudioWorkgroup Constructor is called\n");
}

OHAudioWorkgroup::~OHAudioWorkgroup()
{
}

bool OHAudioWorkgroup::AddThread(int32_t tokenId)
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (AudioSystemManager::GetInstance()->AddThreadToGroup(workgroupId_, tokenId) == AUDIO_OK) {
        workgroupThreads_[tokenId] = true;
        SetNeedUpdatePrioFlag(true);
        return true;
    }
    return false;
}

bool OHAudioWorkgroup::RemoveThread(int32_t tokenId)
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (AudioSystemManager::GetInstance()->RemoveThreadFromGroup(workgroupId_, tokenId) == AUDIO_OK) {
        workgroupThreads_.erase(tokenId);
        return true;
    }
    return false;
}

bool OHAudioWorkgroup::Start(uint64_t startTime, uint64_t deadlineTime)
{
    std::lock_guard<std::mutex> lock(mtx_);
    bool isUpdatePrio = GetNeedUpdatePrioFlag();
    if (AudioSystemManager::GetInstance()->StartGroup(workgroupId_, startTime, deadlineTime,
        workgroupThreads_, isUpdatePrio) == AUDIO_OK) {
        SetNeedUpdatePrioFlag(isUpdatePrio);
        return true;
    }
    return false;
}

bool OHAudioWorkgroup::Stop()
{
    if (AudioSystemManager::GetInstance()->StopGroup(workgroupId_) == AUDIO_OK) {
        return true;
    }
    return false;
}

int32_t OHAudioWorkgroup::GetWorkgroupId() const
{
    return workgroupId_;
}
 
bool OHAudioWorkgroup::GetNeedUpdatePrioFlag() const
{
    return isNeedUpdatePrio_;
}

void OHAudioWorkgroup::SetNeedUpdatePrioFlag(bool flag)
{
    isNeedUpdatePrio_ = flag;
}

}  // namespace AudioStandard
}  // namespace OHOS