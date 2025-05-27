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
    static constexpr int32_t AUDIO_MAX_PROCESS = 2;
    static constexpr int32_t AUDIO_MAX_GRP_PER_PROCESS = 4;
    static constexpr int32_t AUDIO_MAX_RT_THREADS = 4;
}

std::unordered_map<int32_t, std::unordered_map<int32_t, std::shared_ptr<AudioWorkgroup>>>
    AudioResourceService::audioWorkgroupMap;
std::mutex audioWorkgroupMapMutex_;

AudioResourceService::AudioResourceService()
{
}

AudioResourceService::~AudioResourceService()
{
}

static bool IsValidPid(int32_t pid)
{
    return pid > 0;
}
 
int32_t AudioResourceService::CreateAudioWorkgroupCheck(int32_t inPid)
{
    uint32_t pidCount = 0;
    std::lock_guard<std::mutex> lock(audioWorkgroupMapMutex_);
    for (const auto &outerPair : audioWorkgroupMap) {
        int32_t pid = outerPair.first;
 
        AUDIO_INFO_LOG("[AudioResourceService] workgroup pid=%{public}d", pid);
        if (IsValidPid(pid) && (++pidCount >= AUDIO_MAX_PROCESS)) {
            return ERR_NOT_SUPPORTED;
        }
        AUDIO_INFO_LOG("[AudioResourceService] workgroup count=%{public}zu", outerPair.second.size());
        if ((inPid == pid) && (outerPair.second.size() >= AUDIO_MAX_GRP_PER_PROCESS)) {
            AUDIO_INFO_LOG("pid=%{public}d more than 4 groups is not allowd\n", pid);
            return ERR_NOT_SUPPORTED;
        }
    }
    return SUCCESS;
}

int32_t AudioResourceService::CreateAudioWorkgroup(int32_t pid)
{
    CHECK_AND_RETURN_RET_LOG(pid > 0, ERR_INVALID_PARAM, "CreateAudioWorkgroup for pid < 0");

    int ret = CreateAudioWorkgroupCheck(pid);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_SUPPORTED, "[AudioResourceService]:"
        "1, Maximum 2 processes can create deadline workgroup."
        "2, Maximum 4 workgroups can be created per process.");

    ConcurrentTask::IntervalReply reply;
    OHOS::ConcurrentTask::ConcurrentTaskClient::GetInstance().SetAudioDeadline(
        ConcurrentTask::AUDIO_DDL_CREATE_GRP, -1, -1, reply);
    if (reply.rtgId != -1) {
        std::lock_guard<std::mutex> lock(audioWorkgroupMapMutex_);
        audioWorkgroupMap[pid][reply.rtgId] = std::make_shared<AudioWorkgroup>(reply.rtgId);
    }
    return reply.rtgId;
}
 
int32_t AudioResourceService::ReleaseAudioWorkgroup(int32_t pid, int32_t workgroupId)
{
    if (!IsValidPid(pid)) {
        AUDIO_ERR_LOG("[AudioResourceService] ReleaseAudioWorkgroup failed, err pid:%{public}d", pid);
        return ERR_OPERATION_FAILED;
    }
    AudioWorkgroup *group = GetAudioWorkgroupPtr(pid, workgroupId);
    CHECK_AND_RETURN_RET_LOG(group != nullptr, ERR_INVALID_PARAM, "AudioWorkgroup operated is not exsit");
    ConcurrentTask::IntervalReply reply;
    OHOS::ConcurrentTask::ConcurrentTaskClient::GetInstance().SetAudioDeadline(
        ConcurrentTask::AUDIO_DDL_DESTROY_GRP, -1, workgroupId, reply);
    if (reply.paramA != 0) {
        AUDIO_ERR_LOG("[AudioResourceService] ReleaseAudioWorkgroup failed, workgroupId:%{public}d", workgroupId);
        return ERR_OPERATION_FAILED;
    }
    std::lock_guard<std::mutex> lock(audioWorkgroupMapMutex_);
    audioWorkgroupMap[pid].erase(workgroupId);
    if (audioWorkgroupMap[pid].size() == 0) {
        audioWorkgroupMap.erase(pid);
    }
    return SUCCESS;
}

int32_t AudioResourceService::AddThreadToGroup(int32_t pid, int32_t workgroupId, int32_t tokenId)
{
    AudioWorkgroup *group = GetAudioWorkgroupPtr(pid, workgroupId);
    CHECK_AND_RETURN_RET_LOG(group != nullptr, ERR_INVALID_PARAM, "AudioWorkgroup operated is not exsit");

    if (pid == tokenId) {
        AUDIO_ERR_LOG("error: pid=%{public}d, tid=%{public}d", pid, tokenId);
        return ERR_NOT_SUPPORTED;
    }
 
    AUDIO_INFO_LOG("[AudioResourceService] workgroup=%{public}d threads num=%{public}d",
        workgroupId, group->GetThreadsNums());
    if (group->GetThreadsNums() >= AUDIO_MAX_RT_THREADS) {
        AUDIO_ERR_LOG("error: Maximum 4 threads can be added per group");
        return ERR_NOT_SUPPORTED;

    }

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
    std::lock_guard<std::mutex> lock(audioWorkgroupMapMutex_);
    std::shared_ptr<AudioWorkgroup> group_ptr = audioWorkgroupMap[pid][workgroupId];
    if (!group_ptr) {
        AUDIO_ERR_LOG("get AudioWorkgroup ptr failed\n");
        return nullptr;
    }
    return group_ptr.get();
}

} // namespace AudioStandard
} // namespace OHOS