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
#include <cmath>

#include "concurrent_task_client.h"
#include "rtg_interface.h"
#include "audio_errors.h"
#include "audio_utils.h"
#include "audio_workgroup_callback_proxy.h"
#include "audio_workgroup_callback.h"
#include "audio_schedule.h"

namespace OHOS {
namespace AudioStandard {
namespace {
    static constexpr int32_t AUDIO_MAX_PROCESS = 2;
    static constexpr int32_t AUDIO_MAX_GRP_PER_PROCESS = 2;
    static constexpr int32_t AUDIO_MAX_RT_THREADS = 4;
}

AudioResourceService *AudioResourceService::GetInstance()
{
    static AudioResourceService audioResource;
    return &audioResource;
}

AudioResourceService::AudioResourceService()
{
}

AudioResourceService::~AudioResourceService()
{
}

int32_t AudioResourceService::AudioWorkgroupCheck(int32_t pid)
{
    bool inGroup = (audioWorkgroupMap_.find(pid) != audioWorkgroupMap_.end());
    if (inGroup) {
        if (audioWorkgroupMap_[pid].groups.size() >= AUDIO_MAX_GRP_PER_PROCESS) {
            AUDIO_INFO_LOG("[WorkgroupInServer] pid=%{public}d more than 2 groups is not allowed\n", pid);
            return ERR_NOT_SUPPORTED;
        }
    } else {
        uint32_t normalPidCount = 0;
        for (const auto& [key, process] : audioWorkgroupMap_) {
            if (!process.hasSystemPermission) {
                normalPidCount++;
            }
        }
        if (normalPidCount >= AUDIO_MAX_PROCESS) {
            AUDIO_INFO_LOG("[WorkgroupInServer] more than %{public}d processes is not allowed\n", AUDIO_MAX_PROCESS);
            return ERR_NOT_SUPPORTED;
        }
    }
    return SUCCESS;
}

int32_t AudioResourceService::CreateAudioWorkgroup(int32_t pid, const sptr<IRemoteObject> &object)
{
    CHECK_AND_RETURN_RET_LOG(pid > 0, ERR_INVALID_PARAM, "[WorkgroupInServer]"
        "CreateAudioWorkgroup failed, err pid:%{public}d", pid);

    if (!object) {
        AUDIO_ERR_LOG("[AudioResourceService] object is nullptr!");
        return ERR_OPERATION_FAILED;
    }

    std::lock_guard<std::mutex> lock(workgroupLock_);
    int ret = AudioWorkgroupCheck(pid);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_SUPPORTED, "[WorkgroupInServer]:"
        "1, Maximum 2 processes can create deadline workgroup."
        "2, Maximum 2 workgroups can be created per process.");

    ConcurrentTask::IntervalReply reply;
    OHOS::ConcurrentTask::ConcurrentTaskClient::GetInstance().SetAudioDeadline(
        ConcurrentTask::AUDIO_DDL_CREATE_GRP, -1, -1, reply);
    if (reply.rtgId != -1) {
        auto workgroup = std::make_shared<AudioWorkgroup>(reply.rtgId);
        audioWorkgroupMap_[pid].groups[reply.rtgId] = workgroup;
        FillAudioWorkgroupCgroupLimit(pid, workgroup);

        sptr<AudioWorkgroupDeathRecipient> deathRecipient = new AudioWorkgroupDeathRecipient();
        deathRecipient->SetNotifyCb([this, workgroup, object]() {
            this->OnWorkgroupRemoteDied(workgroup, object);
        });
        object->AddDeathRecipient(deathRecipient);
        deathRecipientMap_[workgroup] = std::make_pair(object, deathRecipient);
        RegisterAudioWorkgroupMonitor(pid, reply.rtgId, object);
        DumpAudioWorkgroupMap();
    }

    Trace trace("[WorkgroupInServer] CreateAudioWorkgroup pid:" + std::to_string(pid) +
        " groupId:" + std::to_string(reply.rtgId));

    return reply.rtgId;
}

int32_t AudioResourceService::ReleaseAudioWorkgroup(int32_t pid, int32_t workgroupId)
{
    std::lock_guard<std::mutex> lock(workgroupLock_);
    auto group = GetAudioWorkgroup(pid, workgroupId);
    CHECK_AND_RETURN_RET_LOG(group != nullptr, ERR_INVALID_PARAM, "AudioWorkgroup operated is not exist");

    Trace trace("[WorkgroupInServer] WorkgroupInServer pid:" + std::to_string(pid) +
        " workgroupId:" + std::to_string(workgroupId));

    ConcurrentTask::IntervalReply reply;
    OHOS::ConcurrentTask::ConcurrentTaskClient::GetInstance().SetAudioDeadline(
        ConcurrentTask::AUDIO_DDL_DESTROY_GRP, -1, workgroupId, reply);
    if (reply.paramA != 0) {
        AUDIO_ERR_LOG("[WorkgroupInServer] ReleaseAudioWorkgroup failed, workgroupId:%{public}d", workgroupId);
        return ERR_OPERATION_FAILED;
    }

    auto pidIt = audioWorkgroupMap_.find(pid);
    if (pidIt == audioWorkgroupMap_.end()) {
        AUDIO_ERR_LOG("[WorkgroupInServer] ReleaseAudioWorkgroup pid:%{public}d already removed", pid);
        return ERR_INVALID_PARAM;
    }
 
    auto &groups = pidIt->second.groups;
    auto grpIt = groups.find(workgroupId);
    if (grpIt == groups.end() || !grpIt->second) {
        AUDIO_ERR_LOG("[WorkgroupInServer] ReleaseAudioWorkgroup pid:%{public}d grpId:%{public}d not exist",
            pid, workgroupId);
        return ERR_INVALID_PARAM;
    }
 
    auto deathIt = deathRecipientMap_.find(grpIt->second);
    if (deathIt != deathRecipientMap_.end()) {
        ReleaseWorkgroupDeathRecipient(grpIt->second, deathIt->second.first);
    }
 
    groups.erase(grpIt);
    if (groups.empty()) {
        audioWorkgroupMap_.erase(pidIt);
    }

    DumpAudioWorkgroupMap();
    return SUCCESS;
}

int32_t AudioResourceService::AddThreadToGroup(int32_t pid, int32_t workgroupId, int32_t tokenId)
{
    std::lock_guard<std::mutex> lock(workgroupLock_);
    if (pid == tokenId) {
        AUDIO_ERR_LOG("[WorkgroupInServer] main thread pid=%{public}d is not allowed to be added", pid);
        return ERR_OPERATION_FAILED;
    }
    auto group = GetAudioWorkgroup(pid, workgroupId);
    CHECK_AND_RETURN_RET_LOG(group != nullptr, ERR_INVALID_PARAM, "AudioWorkgroup operated is not exist");

    if (GetThreadsNumPerProcess(pid) >= AUDIO_MAX_RT_THREADS) {
        AUDIO_ERR_LOG("error: Maximum 4 threads can be added per process");
        return ERR_NOT_SUPPORTED;
    }

    int32_t ret = group->AddThread(tokenId);
    return ret;
}

int32_t AudioResourceService::RemoveThreadFromGroup(int32_t pid, int32_t workgroupId, int32_t tokenId)
{
    std::lock_guard<std::mutex> lock(workgroupLock_);
    auto group = GetAudioWorkgroup(pid, workgroupId);
    CHECK_AND_RETURN_RET_LOG(group != nullptr, ERR_INVALID_PARAM, "AudioWorkgroup operated is not exist");
    int32_t ret = group->RemoveThread(tokenId);
    return ret;
}

int32_t AudioResourceService::StartGroup(int32_t pid, int32_t workgroupId, uint64_t startTime, uint64_t deadlineTime)
{
    std::lock_guard<std::mutex> lock(workgroupLock_);
    auto group = GetAudioWorkgroup(pid, workgroupId);
    CHECK_AND_RETURN_RET_LOG(group != nullptr, ERR_INVALID_PARAM, "AudioWorkgroup operated is not exist");
    int32_t ret = group->Start(startTime, deadlineTime);
    return ret;
}
 
int32_t AudioResourceService::StopGroup(int32_t pid, int32_t workgroupId)
{
    std::lock_guard<std::mutex> lock(workgroupLock_);
    auto group = GetAudioWorkgroup(pid, workgroupId);
    CHECK_AND_RETURN_RET_LOG(group != nullptr, ERR_INVALID_PARAM, "AudioWorkgroup operated is not exist");
    int32_t ret = group->Stop();
    return ret;
}

std::shared_ptr<AudioWorkgroup> AudioResourceService::GetAudioWorkgroup(int32_t pid, int32_t workgroupId)
{
    auto pidIt = audioWorkgroupMap_.find(pid);
    if (pidIt == audioWorkgroupMap_.end()) {
        AUDIO_ERR_LOG("[WorkgroupInServer] GetAudioWorkgroup: pid:%{public}d not found", pid);
        return nullptr;
    }
 
    auto &groups = pidIt->second.groups;
    auto grpIt = groups.find(workgroupId);
    if (grpIt == groups.end() || !grpIt->second) {
        AUDIO_ERR_LOG("[WorkgroupInServer] GetAudioWorkgroup: workgroupId=%{public}d"
            " not found for pid=%{public}d", workgroupId, pid);
        return nullptr;
    }
 
    return grpIt->second;
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
    ReleaseWorkgroupDeathRecipient(workgroup, remoteObj);

    std::vector<int> pidsToDelete;
    for (auto& [pid, process] : audioWorkgroupMap_) {
        bool isGroupsCleared = false;
        for (auto it = process.groups.begin(); it != process.groups.end();) {
            if (it->second == workgroup) {
                it = process.groups.erase(it);
                isGroupsCleared = true;
            } else {
                ++it;
            }
        }
        if (isGroupsCleared && process.groups.empty()) {
            pidsToDelete.push_back(pid);
        }
    }
    for (int32_t pid : pidsToDelete) {
        AUDIO_INFO_LOG("[WorkgroupInServer] All workgroups for pid:%{public}d released", pid);
        audioWorkgroupMap_.erase(pid);
    }
    DumpAudioWorkgroupMap();
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

int32_t AudioResourceService::GetThreadsNumPerProcess(int32_t pid)
{
    uint32_t count = 0;
    for (const auto &[id, group]: audioWorkgroupMap_[pid].groups) {
        AUDIO_INFO_LOG("[WorkgroupInServer] pid=%{public}d groupID=%{public}d\n", pid, id);
        if (group != nullptr) {
            count += group->GetThreadsNums();
        }
    }
    AUDIO_INFO_LOG("[WorkgroupInServer] pid=%{public}d total threads=%{public}d\n", pid, count);
    return count;
}

bool AudioResourceService::IsProcessInWorkgroup(int32_t pid)
{
    std::lock_guard<std::mutex> lock(workgroupLock_);
    return audioWorkgroupMap_.find(pid) != audioWorkgroupMap_.end();
}

bool AudioResourceService::IsProcessHasSystemPermission(int32_t pid)
{
    std::lock_guard<std::mutex> lock(workgroupLock_);
    return audioWorkgroupMap_[pid].hasSystemPermission;
}


int32_t AudioResourceService::RegisterAudioWorkgroupMonitor(int32_t pid, int32_t groupId,
    const sptr<IRemoteObject> &object)
{
    uint32_t ret = SUCCESS;

    // system permission HAP no need manage
    audioWorkgroupMap_[pid].hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (audioWorkgroupMap_[pid].hasSystemPermission) {
        return SUCCESS;
    }

    sptr<IAudioWorkgroupCallback > listener = iface_cast<IAudioWorkgroupCallback >(object);

    CHECK_AND_RETURN_RET_LOG(listener != nullptr, ERR_INVALID_PARAM, "AudioServer: listener obj cast failed");

    std::shared_ptr<AudioWorkgroupCallbackForMonitor> callback = std::make_shared<AudioWorkgroupCallback>(listener);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "failed to  create callback obj");

    for (auto &[id, group]: audioWorkgroupMap_[pid].groups) {
        if (id == groupId) {
            group->callback = callback;
            AUDIO_INFO_LOG("[WorkgroupInServer] pid[%{public}d] groudId[%{public}d] registered", pid, id);
        }
    }
    return ret;
}

void AudioResourceService::WorkgroupRendererMonitor(int32_t pid, const bool isAllowed)
{
    std::lock_guard<std::mutex> lock(workgroupLock_);
    // Even though the caller has checked once, here checks is still allowed
    CHECK_AND_RETURN_LOG(audioWorkgroupMap_.find(pid) != audioWorkgroupMap_.end(),
        "[WorkgroupInServer]WorkgroupRendererMonitor failed, err pid:%{public}d", pid);

    if (isAllowed == audioWorkgroupMap_[pid].permission) {
        return;
    }
    audioWorkgroupMap_[pid].permission = isAllowed;

    struct AudioWorkgroupChangeInfo info = {
        .pid = pid,
        .startAllowed = audioWorkgroupMap_[pid].permission,
    };

    for (const auto &[id, group]: audioWorkgroupMap_[pid].groups) {
        const auto &callback = group->callback;
        if (callback == nullptr) {
            break;
        }
        info.groupId = id;
        AUDIO_INFO_LOG("[WorkgroupInServer] pid:%{public}d, groupId:%{public}d startAllowed:%{public}d",
            info.pid, info.groupId, info.startAllowed);
        callback->OnWorkgroupChange(info);
    }
}

void AudioResourceService::DumpAudioWorkgroupMap()
{
    for (const auto& [key, process] : audioWorkgroupMap_) {
        const auto& groups = process.groups;
        for (const auto& [groupKey, audioWorkgroup] : groups) {
            if (audioWorkgroup != nullptr) {
                AUDIO_INFO_LOG("[WorkgroupInServer] pid:%{public}d, group:%{public}d, "
                    "permission:%{public}d, hasSystemPermission:%{public}d, callback:%{public}s",
                    key, groupKey, process.permission, process.hasSystemPermission,
                    ((audioWorkgroup->callback != nullptr) ? "registered" : "no register"));
            }
        }
    }
}

std::vector<int32_t> AudioResourceService::GetProcessesOfAudioWorkgroup()
{
    std::vector<int32_t> keys;
    std::lock_guard<std::mutex> lock(workgroupLock_);
    for (const auto& pair : audioWorkgroupMap_) {
        keys.push_back(pair.first);
    }
    return keys;
}

int32_t AudioResourceService::ImproveAudioWorkgroupPrio(int32_t pid,
    const std::unordered_map<int32_t, bool> &threads)
{
    std::lock_guard<std::mutex> lock(workgroupLock_);
    CHECK_AND_RETURN_RET_LOG(!threads.empty(), ERR_INVALID_PARAM, "[WorkgroupInServer] No thread to improve prio");
    for (const auto &tid : threads) {
        AUDIO_INFO_LOG("[WorkgroupInServer]set pid:%{public}d tid:%{public}d to qos_level7", pid, tid.first);
        ScheduleReportData(pid, tid.first, "audio_server");
    }
    return AUDIO_OK;
}
 
int32_t AudioResourceService::RestoreAudioWorkgroupPrio(int32_t pid,
    const std::unordered_map<int32_t, int32_t> &threads)
{
    std::lock_guard<std::mutex> lock(workgroupLock_);
    CHECK_AND_RETURN_RET_LOG(!threads.empty(), ERR_INVALID_PARAM, "[WorkgroupInServer] No thread to restore prio");
    for (const auto &tid : threads) {
        AUDIO_INFO_LOG("[WorkgroupInServer]set pid:%{public}d tid:%{public}d to qos%{public}d",
            pid, tid.first, tid.second);
        ScheduleReportDataWithQosLevel(pid, tid.first, "audio_server", tid.second);
    }
    return AUDIO_OK;
}

void AudioResourceService::FillAudioWorkgroupCgroupLimit(int32_t pid,
    std::shared_ptr<AudioWorkgroup> &workgroup)
{
    if (workgroup == nullptr) {
        AUDIO_ERR_LOG("[WorkgroupInServer]workgroup is nullptr");
        return;
    }
    int32_t cgroupId = -1;
    std::set<int32_t> usedGroupLimitIds;
    for (const auto &group : audioWorkgroupMap_[pid].groups) {
        int32_t currId = (group.second ? group.second->GetCgroupLimitId() : -1);
        if (currId != -1) {
            usedGroupLimitIds.insert(currId);
        }
    }
    for (int32_t i = 0; i < AUDIO_MAX_GRP_PER_PROCESS; i++) {
        if (usedGroupLimitIds.count(i) == 0) {
            cgroupId = i;
            break;
        }
    }
    workgroup->SetCgroupLimitParams(pid, cgroupId);
}
} // namespce AudioStandard
} // namespace OHOS
