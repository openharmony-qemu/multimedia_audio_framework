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

#include <memory>
#include "rtg_interface.h"
#include "audio_common_log.h"
#include "audio_errors.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {
namespace {
    static constexpr int32_t AUDIO_MAX_PROCESS = 2;
    static constexpr int32_t AUDIO_MAX_GRP_PER_PROCESS = 4;
    static constexpr int32_t AUDIO_MAX_RT_THREADS = 4;
}

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
    for (const auto &outerPair : audioWorkgroupMap) {
        int32_t pid = outerPair.first;
        if (IsValidPid(pid) && (++pidCount >= AUDIO_MAX_PROCESS)) {
            return ERR_NOT_SUPPORTED;
        }
        if ((inPid == pid) && (outerPair.second.size() >= AUDIO_MAX_GRP_PER_PROCESS)) {
            AUDIO_INFO_LOG("pid=%{public}d more than 4 groups is not allowd\n", pid);
            return ERR_NOT_SUPPORTED;
        }
    }
    return SUCCESS;
}

int32_t AudioResourceService::CreateAudioWorkgroup(int32_t pid, const sptr<IRemoteObject> &object)
{
    std::lock_guard<std::mutex> lock(workgroupLock_);
    CHECK_AND_RETURN_RET_LOG(pid > 0, ERR_INVALID_PARAM, "CreateAudioWorkgroup for pid < 0");

    int ret = CreateAudioWorkgroupCheck(pid);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_SUPPORTED, "[AudioResourceService]:"
        "1, Maximum 2 processes can create deadline workgroup."
        "2, Maximum 4 workgroups can be created per process.");

    ConcurrentTask::IntervalReply reply;
    OHOS::ConcurrentTask::ConcurrentTaskClient::GetInstance().SetAudioDeadline(
        ConcurrentTask::AUDIO_DDL_CREATE_GRP, -1, -1, reply);
    if (reply.rtgId != -1) {
        auto workgroup = std::make_shared<AudioWorkgroup>(reply.rtgId);
        audioWorkgroupMap[pid][reply.rtgId] = workgroup;
 
        sptr<AudioWorkgroupDeathRecipient> deathRecipient = new AudioWorkgroupDeathRecipient();
        deathRecipient->SetNotifyCb([this, workgroup, object]() {
            this->OnWorkgroupRemoteDied(workgroup, object);
        });
        object->AddDeathRecipient(deathRecipient);
        deathRecipientMap_[workgroup] = std::make_pair(object, deathRecipient);
    }
    Trace trace("[WorkgroupInServer] CreateAudioWorkgroup pid:" + std::to_string(pid) +
        " groupId:" + std::to_string(reply.rtgId));
    return reply.rtgId;
}

int32_t AudioResourceService::ReleaseAudioWorkgroup(int32_t pid, int32_t workgroupId)
{
    if (!IsValidPid(pid)) {
        AUDIO_ERR_LOG("[AudioResourceService] ReleaseAudioWorkgroup failed, err pid:%{public}d", pid);
        return ERR_OPERATION_FAILED;
    }
    std::lock_guard<std::mutex> lock(workgroupLock_);
    Trace trace("[WorkgroupInServer] ReleaseAudioWorkgroup pid:" + std::to_string(pid) +
        " workgroupId:" + std::to_string(workgroupId));

    AudioWorkgroup *group = GetAudioWorkgroupPtr(pid, workgroupId);
    CHECK_AND_RETURN_RET_LOG(group != nullptr, ERR_INVALID_PARAM, "AudioWorkgroup operated is not exsit");
    ConcurrentTask::IntervalReply reply;
    OHOS::ConcurrentTask::ConcurrentTaskClient::GetInstance().SetAudioDeadline(
        ConcurrentTask::AUDIO_DDL_DESTROY_GRP, -1, workgroupId, reply);
    if (reply.paramA != 0) {
        AUDIO_ERR_LOG("[WorkgroupInServer] ReleaseAudioWorkgroup failed, workgroupId:%{public}d", workgroupId);
        return ERR_OPERATION_FAILED;
    }

    std::shared_ptr<AudioWorkgroup> workgroupPtr;
    auto it = audioWorkgroupMap.find(pid);
    if (it != audioWorkgroupMap.end()) {
        auto wgIt = it->second.find(workgroupId);
        if (wgIt != it->second.end()) {
            workgroupPtr = wgIt->second;
        }
    }
 
    if (workgroupPtr) {
        auto deathIt = deathRecipientMap_.find(workgroupPtr);
        if (deathIt != deathRecipientMap_.end()) {
            ReleaseWorkgroupDeathRecipient(workgroupPtr, deathIt->second.first);
        }
    }

    audioWorkgroupMap[pid].erase(workgroupId);
    if (audioWorkgroupMap[pid].size() == 0) {
        audioWorkgroupMap.erase(pid);
    }
    return SUCCESS;
}

int32_t AudioResourceService::AddThreadToGroup(int32_t pid, int32_t workgroupId, int32_t tokenId)
{
    std::lock_guard<std::mutex> lock(workgroupLock_);
    if (pid == tokenId) {
        AUDIO_ERR_LOG("[WorkgroupInServer] main thread pid=%{public}d is not allowed to be added", pid);
        return ERR_OPERATION_FAILED;
    }
    AudioWorkgroup *group = GetAudioWorkgroupPtr(pid, workgroupId);
    CHECK_AND_RETURN_RET_LOG(group != nullptr, ERR_INVALID_PARAM, "AudioWorkgroup operated is not exsit");

    if (pid == tokenId) {
        AUDIO_ERR_LOG("error: pid=%{public}d, tid=%{public}d", pid, tokenId);
        return ERR_NOT_SUPPORTED;
    }

    if (group->GetThreadsNums() >= AUDIO_MAX_RT_THREADS) {
        AUDIO_ERR_LOG("error: Maximum 4 threads can be added per group");
        return ERR_NOT_SUPPORTED;
    }
    int32_t ret = group->AddThread(tokenId);
    return ret;
}

int32_t AudioResourceService::RemoveThreadFromGroup(int32_t pid, int32_t workgroupId, int32_t tokenId)
{
    std::lock_guard<std::mutex> lock(workgroupLock_);
    AudioWorkgroup *group = GetAudioWorkgroupPtr(pid, workgroupId);
    CHECK_AND_RETURN_RET_LOG(group != nullptr, ERR_INVALID_PARAM, "AudioWorkgroup operated is not exsit");
    int32_t ret = group->RemoveThread(tokenId);
    return ret;
}

int32_t AudioResourceService::StartGroup(int32_t pid, int32_t workgroupId, uint64_t startTime, uint64_t deadlineTime)
{
    std::lock_guard<std::mutex> lock(workgroupLock_);
    AudioWorkgroup *group = GetAudioWorkgroupPtr(pid, workgroupId);
    CHECK_AND_RETURN_RET_LOG(group != nullptr, ERR_INVALID_PARAM, "AudioWorkgroup operated is not exsit");
    int32_t ret = group->Start(startTime, deadlineTime);
    return ret;
}
 
int32_t AudioResourceService::StopGroup(int32_t pid, int32_t workgroupId)
{
    std::lock_guard<std::mutex> lock(workgroupLock_);
    AudioWorkgroup *group = GetAudioWorkgroupPtr(pid, workgroupId);
    CHECK_AND_RETURN_RET_LOG(group != nullptr, ERR_INVALID_PARAM, "AudioWorkgroup operated is not exsit");
    int32_t ret = group->Stop();
    return ret;
}

AudioWorkgroup *AudioResourceService::GetAudioWorkgroupPtr(int32_t pid, int32_t workgroupId)
{
    std::shared_ptr<AudioWorkgroup> group_ptr = audioWorkgroupMap[pid][workgroupId];
    if (!group_ptr) {
        AUDIO_ERR_LOG("[WorkgroupInServer] get AudioWorkgroup ptr failed\n");
        return nullptr;
    }
    return group_ptr.get();
}

AudioResourceService::AudioWorkgroupDeathRecipient::AudioWorkgroupDeathRecipient()
{
    AUDIO_ERR_LOG("[WorkgroupInServer] AudioWorkgroupDeathRecipient ctor");
}
 
void AudioResourceService::AudioWorkgroupDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    if (diedCb_ != nullptr) {
        diedCb_();
    }
}
 
void AudioResourceService::AudioWorkgroupDeathRecipient::SetNotifyCb(NotifyCbFunc func)
{
    diedCb_ = func;
}
 
void AudioResourceService::OnWorkgroupRemoteDied(const std::shared_ptr<AudioWorkgroup> &workgroup,
    const sptr<IRemoteObject> &remoteObj)
{
    std::lock_guard<std::mutex> lock(workgroupLock_);
    ReleaseWorkgroupDeathRecipient(workgroup, remoteObj);
 
    for (auto pidIt = audioWorkgroupMap.begin(); pidIt != audioWorkgroupMap.end();) {
        for (auto groupIt = pidIt->second.begin(); groupIt != pidIt->second.end();) {
            if (groupIt->second == workgroup) {
                groupIt = pidIt->second.erase(groupIt);
            } else {
                ++groupIt;
            }
        }
        if (pidIt->second.empty()) {
            AUDIO_INFO_LOG("[WorkgroupInServer] All workgroups for pid:%{public}d released", pidIt->first);
            pidIt = audioWorkgroupMap.erase(pidIt);
        } else {
            ++pidIt;
        }
    }
}
 
void AudioResourceService::ReleaseWorkgroupDeathRecipient(const std::shared_ptr<AudioWorkgroup> &workgroup,
    const sptr<IRemoteObject> &remoteObj)
{
    auto it = deathRecipientMap_.find(workgroup);
    if (it != deathRecipientMap_.end()) {
        if (it->second.first == remoteObj) {
            remoteObj->RemoveDeathRecipient(it->second.second);
            deathRecipientMap_.erase(it);
        }
    }
}

} // namespce AudioStandard
} // namespace OHOS
