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
    static constexpr int32_t AUDIO_MAX_GRP_PER_PROCESS = 4;
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

static bool IsValidPid(int32_t pid)
{
    return pid > 0;
}

int32_t AudioResourceService::AudioWorkgroupCheck(int32_t pid)
{
    bool inGroup = IsProcessInWorkgroup(pid);
    std::lock_guard<std::mutex> lock(workgroupLock_);
    if (inGroup) {
        if (audioWorkgroupMap_[pid].groups.size() >= AUDIO_MAX_GRP_PER_PROCESS) {
            AUDIO_INFO_LOG("[WorkgroupInServer] pid=%{public}d more than 4 groups is not allowed\n", pid);
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
    CHECK_AND_RETURN_RET_LOG(pid > 0, ERR_INVALID_PARAM, "CreateAudioWorkgroup for pid < 0");

    if (!object) {
        AUDIO_ERR_LOG("[AudioResourceService] object is nullptr!");
        return ERR_OPERATION_FAILED;
    }

    int ret = AudioWorkgroupCheck(pid);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_SUPPORTED, "[WorkgroupInServer]:"
        "1, Maximum 2 processes can create deadline workgroup."
        "2, Maximum 4 workgroups can be created per process.");

    ConcurrentTask::IntervalReply reply;
    OHOS::ConcurrentTask::ConcurrentTaskClient::GetInstance().SetAudioDeadline(
        ConcurrentTask::AUDIO_DDL_CREATE_GRP, -1, -1, reply);
    if (reply.rtgId != -1) {
        std::lock_guard<std::mutex> lock(workgroupLock_);
        auto workgroup = std::make_shared<AudioWorkgroup>(reply.rtgId);
        audioWorkgroupMap_[pid].groups[reply.rtgId] = workgroup;

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
    if (!IsValidPid(pid)) {
        AUDIO_ERR_LOG("[AudioResourceService] ReleaseAudioWorkgroup failed, err pid:%{public}d", pid);
        return ERR_OPERATION_FAILED;
    }

    Trace trace("[WorkgroupInServer] WorkgroupInServer pid:" + std::to_string(pid) +
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

    std::lock_guard<std::mutex> lock(workgroupLock_);
    auto workgroupPtr = audioWorkgroupMap_[pid].groups[workgroupId];
    if (workgroupPtr) {
        auto deathIt = deathRecipientMap_.find(workgroupPtr);
        if (deathIt != deathRecipientMap_.end()) {
            ReleaseWorkgroupDeathRecipient(workgroupPtr, deathIt->second.first);
        }
    }

    audioWorkgroupMap_[pid].groups.erase(workgroupId);
    if (audioWorkgroupMap_[pid].groups.size() == 0) {
        audioWorkgroupMap_.erase(pid);
    }
    DumpAudioWorkgroupMap();
    return SUCCESS;
}

int32_t AudioResourceService::AddThreadToGroup(int32_t pid, int32_t workgroupId, int32_t tokenId)
{
    if (pid == tokenId) {
        AUDIO_ERR_LOG("[WorkgroupInServer] main thread pid=%{public}d is not allowed to be added", pid);
        return ERR_OPERATION_FAILED;
    }
    AudioWorkgroup *group = GetAudioWorkgroupPtr(pid, workgroupId);
    CHECK_AND_RETURN_RET_LOG(group != nullptr, ERR_INVALID_PARAM, "AudioWorkgroup operated is not exsit");

    if (GetThreadsNumPerProcess(pid) >= AUDIO_MAX_RT_THREADS) {
        AUDIO_ERR_LOG("error: Maximum 4 threads can be added per process");
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
    std::lock_guard<std::mutex> lock(workgroupLock_);
    std::shared_ptr<AudioWorkgroup> group_ptr = audioWorkgroupMap_[pid].groups[workgroupId];
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
    std::lock_guard<std::mutex> lock(workgroupLock_);
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
    // Even though the caller has checked once, here checks is still allowed
    if (!IsProcessInWorkgroup(pid)) {
        return;
    }

    std::lock_guard<std::mutex> lock(workgroupLock_);
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
    for (const auto &tid : threads) {
        AUDIO_INFO_LOG("[WorkgroupInServer]set pid:%{public}d tid:%{public}d to qos_level7", pid, tid.first);
        ScheduleReportData(pid, tid.first, "audio_server");
    }
    return AUDIO_OK;
}
 
int32_t AudioResourceService::RestoreAudioWorkgroupPrio(int32_t pid,
    const std::unordered_map<int32_t, int32_t> &threads)
{
    for (const auto &tid : threads) {
        AUDIO_INFO_LOG("[WorkgroupInServer]set pid:%{public}d tid:%{public}d to qos%{public}d",
            pid, tid.first, tid.second);
        ScheduleReportDataWithQosLevel(pid, tid.first, "audio_server", tid.second);
    }
    return AUDIO_OK;
}
} // namespce AudioStandard
} // namespace OHOS
