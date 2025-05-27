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

#include "audio_resource_service.h"
#include "audio_common_log.h"
#include "audio_errors.h"
#include "audio_server.h"
#include "rtg_interface.h"

namespace OHOS {
namespace AudioStandard {
namespace {
    
}

std::unordered_map<int32_t, std::unordered_map<int32_t,
    std::shared_ptr<AudioWorkgroup>>> AudioResourceService::audioWorkgroupMap;

AudioResourceService::AudioResourceService()
{
}

AudioResourceService::~AudioResourceService()
{
}

int32_t AudioResourceService::CreateAudioWorkgroup(int32_t pid)
{
    CHECK_AND_RETURN_RET_LOG(pid > 0, ERR_INVALID_PARAM, "CreateAudioWorkgroup for pid < 0");
    int id = 0;
    for (id = 0; id < audioWorkgroupMap[pid].size(); id++) {
        bool exist = false;
        for (auto iter = audioWorkgroupMap[pid].begin(); iter != audioWorkgroupMap[pid].end(); iter++) {
            if (iter->second != nullptr && iter->second->GetWorkgroupId() == id) {
                exist = true;
                break;
            }
        }
        if (!exist) {
            break;
        }
    }
    ConcurrentTask::IntervalReply reply;
    OHOS::ConcurrentTask::ConcurrentTaskClient::GetInstance().SetAudioDeadline(
        ConcurrentTask::AUDIO_DDL_CREATE_GRP, -1, -1, reply);
    if (reply.rtgId != -1) {
        audioWorkgroupMap[pid][reply.rtgId] = std::make_shared<AudioWorkgroup>(reply.rtgId);
    }
    return reply.rtgId;
}
 
int32_t AudioResourceService::ReleaseAudioWorkgroup(int32_t pid, int32_t workgroupId)
{
    AudioWorkgroup *group = GetAudioWorkgroupPtr(pid, workgroupId);
    CHECK_AND_RETURN_RET_LOG(group != nullptr, ERR_INVALID_PARAM, "AudioWorkgroup operated is not exsit");
    ConcurrentTask::IntervalReply reply;
    OHOS::ConcurrentTask::ConcurrentTaskClient::GetInstance().SetAudioDeadline(
        ConcurrentTask::AUDIO_DDL_DESTROY_GRP, -1, workgroupId, reply);
    if (reply.paramA != 0) {
        AUDIO_ERR_LOG("[AudioResourceService] ReleaseAudioWorkgroup failed, workgroupId:%{public}d", workgroupId);
        return ERR_OPERATION_FAILED;
    }
    audioWorkgroupMap[pid].erase(workgroupId);
    return SUCCESS;
}

int32_t AudioResourceService::AddThreadToGroup(int32_t pid, int32_t workgroupId, int32_t tokenId)
{
    AudioWorkgroup *group = GetAudioWorkgroupPtr(pid, workgroupId);
    CHECK_AND_RETURN_RET_LOG(group != nullptr, ERR_INVALID_PARAM, "AudioWorkgroup operated is not exsit");
    int32_t ret = group->AddThread(tokenId);
    return ret;
}

int32_t AudioResourceService::RemoveThreadFromGroup(int32_t pid, int32_t workgroupId, int32_t tokenId)
{
    AudioWorkgroup *group = GetAudioWorkgroupPtr(pid, workgroupId);
    CHECK_AND_RETURN_RET_LOG(group != nullptr, ERR_INVALID_PARAM, "AudioWorkgroup operated is not exsit");
    int32_t ret = group->RemoveThread(tokenId);
    return ret;
}

int32_t AudioResourceService::StartGroup(int32_t pid, int32_t workgroupId, uint64_t startTime, uint64_t deadlineTime)
{
    AudioWorkgroup *group = GetAudioWorkgroupPtr(pid, workgroupId);
    CHECK_AND_RETURN_RET_LOG(group != nullptr, ERR_INVALID_PARAM, "AudioWorkgroup operated is not exsit");
    int32_t ret = group->Start(startTime, deadlineTime);
    return ret;
}

int32_t AudioResourceService::StopGroup(int32_t pid, int32_t workgroupId)
{
    AudioWorkgroup *group = GetAudioWorkgroupPtr(pid, workgroupId);
    CHECK_AND_RETURN_RET_LOG(group != nullptr, ERR_INVALID_PARAM, "AudioWorkgroup operated is not exsit");
    int32_t ret = group->Stop();
    return ret;
}

AudioWorkgroup *AudioResourceService::GetAudioWorkgroupPtr(int32_t pid, int32_t workgroupId)
{
    std::shared_ptr<AudioWorkgroup> group_ptr = audioWorkgroupMap[pid][workgroupId];
    if (!group_ptr) {
        AUDIO_ERR_LOG("get AudioWorkgroup ptr failed\n");
        return nullptr;
    }
    return group_ptr.get();
}

} // namespace AudioStandard
} // namespace OHOS