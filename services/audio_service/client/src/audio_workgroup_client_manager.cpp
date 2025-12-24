/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "WorkgroupPrioRecorderManager"
#endif

#include "audio_common_log.h"
#include "audio_errors.h"
#include "audio_utils.h"
#include "qos.h"
#include "rtg_interface.h"
#include "audio_qosmanager.h"
#include "audio_workgroup_callback_impl.h"
#include "audio_workgroup_callback_stub.h"
#include "audio_service_proxy.h"
#include "audio_workgroup_client_manager.h"

using namespace OHOS::RME;

namespace OHOS {
namespace AudioStandard {
constexpr unsigned int MS_PER_SECOND = 1000;
constexpr unsigned int AUDIO_DEADLINE_PARAM_MIN = 10;
constexpr unsigned int AUDIO_DEADLINE_PARAM_MAX = 50;

WorkgroupPrioRecorderManager &WorkgroupPrioRecorderManager::GetInstance()
{
    static WorkgroupPrioRecorderManager audioManager;
    return audioManager;
}

class AudioWorkgroupChangeCallbackImpl : public AudioWorkgroupChangeCallback {
public:
    AudioWorkgroupChangeCallbackImpl() {};
    ~AudioWorkgroupChangeCallbackImpl() {};
private:
    void OnWorkgroupChange(const AudioWorkgroupChangeInfo &info) override;
};

void AudioWorkgroupChangeCallbackImpl::OnWorkgroupChange(
    const AudioWorkgroupChangeInfo &info)
{
    WorkgroupPrioRecorderManager::GetInstance().OnWorkgroupChange(info);
}


WorkgroupPrioRecorder::WorkgroupPrioRecorder(int32_t grpId)
{
    grpId_ = grpId;
    restoreByPermission_ = false;
}

void WorkgroupPrioRecorder::SetRestoreByPermission(bool isByPermission)
{
    restoreByPermission_ = isByPermission;
}

bool WorkgroupPrioRecorder::GetRestoreByPermission()
{
    return restoreByPermission_;
}

void WorkgroupPrioRecorder::RecordThreadPrio(int32_t tokenId)
{
    std::lock_guard<std::mutex> lock(workgroupThreadsMutex_);
    auto it = threads_.find(tokenId);
    if (it == threads_.end()) {
        OHOS::QOS::QosLevel qosLevel;
        int32_t ret = OHOS::QOS::GetThreadQos(qosLevel);
        threads_[tokenId] = (ret == -1 ? -1 : static_cast<int32_t>(qosLevel));
    }
}

int32_t WorkgroupPrioRecorder::RestoreGroupPrio(bool isByPermission)
{
    const sptr<IStandardAudioService> gasp = AudioServiceProxy::GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, ERR_INVALID_PARAM, "Audio service unavailable.");

    std::lock_guard<std::mutex> lock(workgroupThreadsMutex_);
    if (gasp->RestoreAudioWorkgroupPrio(threads_) != AUDIO_OK) {
        AUDIO_ERR_LOG("[WorkgroupInClient] restore prio for workgroupId:%{public}d failed", GetGrpId());
        return AUDIO_ERR;
    }

    if (!isByPermission) {
        threads_.clear();
    } else {
        restoreByPermission_ = true;
    }
    return AUDIO_OK;
}

int32_t WorkgroupPrioRecorder::RestoreThreadPrio(int32_t tokenId)
{
    const sptr<IStandardAudioService> gasp = AudioServiceProxy::GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, ERR_INVALID_PARAM, "Audio service unavailable.");
    std::lock_guard<std::mutex> lock(workgroupThreadsMutex_);
    auto it = threads_.find(tokenId);
    int ipcRet;
    if (it != threads_.end()) {
        std::unordered_map<int32_t, int32_t> thread = {{it->first, it->second}};
        ipcRet = gasp->RestoreAudioWorkgroupPrio(thread);
        if (ipcRet != SUCCESS) {
            AUDIO_ERR_LOG("[WorkgroupInClient] change prio for tokenId:%{public}d failed, ret:%{public}d",
                tokenId, ipcRet);
            return AUDIO_ERR;
        } else {
            threads_.erase(tokenId);
        }
    }
    return AUDIO_OK;
}

int32_t WorkgroupPrioRecorder::GetGrpId()
{
    return grpId_;
}

std::shared_ptr<WorkgroupPrioRecorder> WorkgroupPrioRecorderManager::GetRecorderByGrpId(int32_t grpId)
{
    std::lock_guard<std::mutex> recorderLock(workgroupPrioRecorderMutex_);
    auto it = workgroupPrioRecorderMap_.find(grpId);
    if (it != workgroupPrioRecorderMap_.end()) {
        return it->second;
    }
    return nullptr;
}

int32_t WorkgroupPrioRecorderManager::ExecuteAudioWorkgroupPrioImprove(int32_t workgroupId,
    const std::unordered_map<int32_t, bool> threads, bool &needUpdatePrio)
{
    bool restoreByPermission = false;
    std::shared_ptr<WorkgroupPrioRecorder> recorder = GetRecorderByGrpId(workgroupId);
    if (!recorder) {
        AUDIO_ERR_LOG("[WorkgroupInClient] GetRecorderByGrpId workgroupId:%{public}d failed", workgroupId);
        return AUDIO_ERR;
    }

    restoreByPermission = recorder->GetRestoreByPermission();
    if (needUpdatePrio || restoreByPermission) {
        const sptr<IStandardAudioService> gasp = AudioServiceProxy::GetAudioSystemManagerProxy();
        CHECK_AND_RETURN_RET_LOG(gasp != nullptr, ERR_INVALID_PARAM, "Audio service unavailable.");
        int32_t ipcRet = gasp->ImproveAudioWorkgroupPrio(threads);
        if (ipcRet != SUCCESS) {
            AUDIO_ERR_LOG("[WorkgroupInClient] change prio for grp:%{public}d failed, ret:%{public}d",
                workgroupId, ipcRet);
            return AUDIO_ERR;
        }
        needUpdatePrio = false;
        recorder->SetRestoreByPermission(false);
    }

    return AUDIO_OK;
}

int32_t WorkgroupPrioRecorderManager::CreateAudioWorkgroup()
{
    hasSystemPermission_ = PermissionUtil::VerifySelfPermission();
    sptr<AudioWorkgroupCallbackImpl> workgroup = new(std::nothrow) AudioWorkgroupCallbackImpl();
    if (workgroup == nullptr) {
        AUDIO_ERR_LOG("[WorkgroupInClient] workgroup is null");
        return ERROR;
    }

    auto callback = std::make_shared<AudioWorkgroupChangeCallbackImpl>();
    if (callback == nullptr) {
        AUDIO_ERR_LOG("[WorkgroupInClient]workgroupChangeCallback_ Allocation Failed");
        return ERROR;
    }
    workgroup->AddWorkgroupChangeCallback(callback);

    sptr<IRemoteObject> object = workgroup->AsObject();
    if (object == nullptr) {
        AUDIO_ERR_LOG("[WorkgroupInClient] object is null");
        return ERROR;
    }

    const sptr<IStandardAudioService> gasp = AudioServiceProxy::GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, ERR_INVALID_PARAM, "Audio service unavailable.");
    int32_t workgroupId = 0;
    int32_t res = gasp->CreateAudioWorkgroup(object, workgroupId);
    CHECK_AND_RETURN_RET_LOG(res == SUCCESS && workgroupId >= 0, AUDIO_ERR,
        "CreateAudioWorkgroup failed, res:%{public}d workgroupId:%{public}d", res, workgroupId);

    std::lock_guard<std::mutex> recorderLock(workgroupPrioRecorderMutex_);
    workgroupPrioRecorderMap_.emplace(workgroupId, std::make_shared<WorkgroupPrioRecorder>(workgroupId));
    return workgroupId;
}

int32_t WorkgroupPrioRecorderManager::ReleaseAudioWorkgroup(int32_t workgroupId)
{
    const sptr<IStandardAudioService> gasp = AudioServiceProxy::GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, ERR_INVALID_PARAM, "Audio service unavailable.");
    int32_t ret = gasp->ReleaseAudioWorkgroup(workgroupId);

    std::shared_ptr<WorkgroupPrioRecorder> recorder = GetRecorderByGrpId(workgroupId);
    if (recorder != nullptr) {
        if (recorder->RestoreGroupPrio(false) != AUDIO_OK) {
            AUDIO_ERR_LOG("[WorkgroupInClient] restore grp:%{public}d prio failed", workgroupId);
        } else {
            std::lock_guard<std::mutex> recorderLock(workgroupPrioRecorderMutex_);
            workgroupPrioRecorderMap_.erase(workgroupId);
        }
    }

    int32_t pid = getpid();
    std::lock_guard<std::mutex> lock(startGroupPermissionMapMutex_);
    startGroupPermissionMap_[pid].erase(workgroupId);
    if (startGroupPermissionMap_[pid].size() == 0) {
        startGroupPermissionMap_.erase(pid);
    }
    return ret;
}

int32_t WorkgroupPrioRecorderManager::AddThreadToGroup(int32_t workgroupId, int32_t tokenId)
{
    const sptr<IStandardAudioService> gasp = AudioServiceProxy::GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, ERR_INVALID_PARAM, "Audio service unavailable.");

    std::shared_ptr<WorkgroupPrioRecorder> recorder = GetRecorderByGrpId(workgroupId);
    if (recorder != nullptr) {
        recorder->RecordThreadPrio(tokenId);
    }

    return gasp->AddThreadToGroup(workgroupId, tokenId);
}

int32_t WorkgroupPrioRecorderManager::RemoveThreadFromGroup(int32_t workgroupId, int32_t tokenId)
{
    const sptr<IStandardAudioService> gasp = AudioServiceProxy::GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, ERR_INVALID_PARAM, "Audio service unavailable.");

    std::shared_ptr<WorkgroupPrioRecorder> recorder = GetRecorderByGrpId(workgroupId);
    if (recorder != nullptr) {
        if (recorder->RestoreThreadPrio(tokenId) != AUDIO_OK) {
            AUDIO_INFO_LOG("[WorkgroupInClient] restore thread:%{public}d prio failed", tokenId);
        }
    }

    return gasp->RemoveThreadFromGroup(workgroupId, tokenId);
}

int32_t WorkgroupPrioRecorderManager::StartGroup(int32_t workgroupId, uint64_t startTime, uint64_t deadlineTime,
    const std::unordered_map<int32_t, bool> threads, bool &needUpdatePrio)
{
    if (!IsValidToStartGroup(workgroupId)) {
        StopGroup(workgroupId);
        return AUDIO_ERR;
    }

    Trace trace("[WorkgroupInClient] StartGroup workgroupId:" + std::to_string(workgroupId) +
        " startTime:" + std::to_string(startTime) + " deadlineTime:" + std::to_string(deadlineTime));
    CHECK_AND_RETURN_RET_LOG(deadlineTime > startTime, ERR_INVALID_PARAM, "Invalid Audio Deadline params");
    int32_t audioDeadlineRate = static_cast<int32_t>(MS_PER_SECOND / (deadlineTime - startTime));
    CHECK_AND_RETURN_RET_LOG(audioDeadlineRate >= AUDIO_DEADLINE_PARAM_MIN &&
        audioDeadlineRate <= AUDIO_DEADLINE_PARAM_MAX, ERR_INVALID_PARAM, "Invalid Audio Deadline Rate");
    RME::SetFrameRateAndPrioType(workgroupId, audioDeadlineRate, 0);

    if (ExecuteAudioWorkgroupPrioImprove(workgroupId, threads, needUpdatePrio) != AUDIO_OK) {
        AUDIO_ERR_LOG("[WorkgroupInClient] execute audioworkgroup prio improve failed");
        return AUDIO_ERR;
    }

    if (RME::BeginFrameFreq(deadlineTime - startTime) != 0) {
        AUDIO_ERR_LOG("[WorkgroupInClient] Audio Deadline BeginFrame failed");
        return AUDIO_ERR;
    }
    return AUDIO_OK;
}

int32_t WorkgroupPrioRecorderManager::StopGroup(int32_t workgroupId)
{
    if (EndFrameFreq(0) != 0) {
        AUDIO_ERR_LOG("[WorkgroupInClient] Audio Deadline EndFrame failed");
        return AUDIO_ERR;
    }
    return AUDIO_OK;
}

void WorkgroupPrioRecorderManager::OnWorkgroupChange(const AudioWorkgroupChangeInfo &info)
{
    std::lock_guard<std::mutex> lock(startGroupPermissionMapMutex_);
    startGroupPermissionMap_[info.pid][info.groupId] = info.startAllowed;
    std::vector<int32_t> workgroupIdNeedRestore;

    for (const auto &pair : startGroupPermissionMap_) {
        uint32_t pid = pair.first;
        const std::unordered_map<uint32_t, bool>& permissions = pair.second;
        for (const auto &innerPair : permissions) {
            uint32_t grpId = innerPair.first;
            bool permissionValue = innerPair.second;
            AUDIO_INFO_LOG("[WorkgroupInClient] pid = %{public}d, groupId = %{public}d, startAllowed = %{public}d",
                pid, grpId, permissionValue);
            if (permissionValue == false) {
                workgroupIdNeedRestore.push_back(grpId);
            }
        }
    }

    for (const auto &workgroupId : workgroupIdNeedRestore) {
        std::shared_ptr<WorkgroupPrioRecorder> recorder = GetRecorderByGrpId(workgroupId);
        if (recorder != nullptr) {
            if (recorder->RestoreGroupPrio(true) != AUDIO_OK) {
                AUDIO_INFO_LOG("[WorkgroupInClient] restore grp:%{public}d prio in cb failed", workgroupId);
            }
        }
    }
}

bool WorkgroupPrioRecorderManager::IsValidToStartGroup(int32_t workgroupId)
{
    if (hasSystemPermission_) {
        return true;
    }

    int32_t pid = getpid();
    std::lock_guard<std::mutex> lock(startGroupPermissionMapMutex_);
    auto outerIt = startGroupPermissionMap_.find(pid);
    if (outerIt == startGroupPermissionMap_.end()) {
        return false;
    }
    const auto& innerMap = outerIt->second;
    for (const auto& pair : innerMap) {
        if (pair.second) {
            return true;
        }
    }
    return false;
}
} // namespace AudioStandard
} // namespace OHOS
