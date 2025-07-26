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

OHAudioWorkgroup::OHAudioWorkgroup(int id) : workgroupId(id)
{
    AUDIO_INFO_LOG("OHAudioWorkgroup Constructor is called\n");
}

OHAudioWorkgroup::~OHAudioWorkgroup()
{
}

bool OHAudioWorkgroup::AddThread(int32_t tokenId)
{
    if (AudioSystemManager::GetInstance()->AddThreadToGroup(workgroupId, tokenId) == AUDIO_OK) {
        threads_[tokenId] = true;
        SetNeedUpdatePrioFlag(true);
        return true;
    }
    return false;
}

bool OHAudioWorkgroup::RemoveThread(int32_t tokenId)
{
    if (AudioSystemManager::GetInstance()->RemoveThreadFromGroup(workgroupId, tokenId) == AUDIO_OK) {
        threads_.erase(tokenId);
        return true;
    }
    return false;
}

bool OHAudioWorkgroup::Start(uint64_t startTime, uint64_t deadlineTime)
{
    bool isUpdatePrio = GetNeedUpdatePrioFlag();
    if (AudioSystemManager::GetInstance()->StartGroup(workgroupId, startTime, deadlineTime,
        threads_, isUpdatePrio) == AUDIO_OK) {
        SetNeedUpdatePrioFlag(isUpdatePrio);
        return true;
    }
    return false;
}

bool OHAudioWorkgroup::Stop()
{
    if (AudioSystemManager::GetInstance()->StopGroup(workgroupId) == AUDIO_OK) {
        return true;
    }
    return false;
}

bool OHAudioWorkgroup::GetNeedUpdatePrioFlag()
{
    return isNeedUpdatePrio_;
}

void OHAudioWorkgroup::SetNeedUpdatePrioFlag(bool flag)
{
    isNeedUpdatePrio_ = flag;
}

}  // namespace AudioStandard
}  // namespace OHOS