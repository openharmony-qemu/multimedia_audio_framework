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

#ifndef AUDIO_WORKGROUP_CLIENT_MANAGER_H
#define AUDIO_WORKGROUP_CLIENT_MANAGER_H

#include <mutex>
#include <unordered_map>
#include "audio_workgroup_ipc.h"

namespace OHOS {
namespace AudioStandard {
// Workgroup priority recorder and related helpers, included inside AudioSystemManager class.
class WorkgroupPrioRecorder {
public:
    WorkgroupPrioRecorder(int32_t grpId);
    ~WorkgroupPrioRecorder() = default;
    void SetRestoreByPermission(bool isByPermission);
    bool GetRestoreByPermission();
    int32_t GetGrpId();
    void RecordThreadPrio(int32_t tokenId);
    int32_t RestoreGroupPrio(bool isByPermission);
    int32_t RestoreThreadPrio(int32_t tokenId);
private:
    int32_t grpId_;
    std::unordered_map<int32_t, int32_t> threads_;
    bool restoreByPermission_;
    std::mutex workgroupThreadsMutex_;
};

class WorkgroupPrioRecorderManager {
public:
    static WorkgroupPrioRecorderManager &GetInstance();

    /**
    * @brief create audio workgroup
    *
    * @return Returns id of workgroup. id < 0 if failed.
    * @test
    */
    int32_t CreateAudioWorkgroup();

    /**
    * @brief release audio workgroup.
    *
    * @param workgroupId audio workgroup id.
    * @return Returns {@link AUDIO_OK} if the operation is successfully.
    * @test
    */
    int32_t ReleaseAudioWorkgroup(int32_t workgroupId);

    /**
    * @brief add thread to audio workgroup.
    *
    * @param workgroupId workgroupId audio workgroup id.
    * @param tokenId the thread id of add workgroupId.
    * @return Returns {@link AUDIO_OK} if the operation is successfully.
    * @test
    */
    int32_t AddThreadToGroup(int32_t workgroupId, int32_t tokenId);

    /**
    * @brief remove thread to audio workgroup.y
    *
    * @param workgroupId workgroupId audio workgroup id.
    * @param tokenId the thread id of remove workgroupId.
    * @return Returns {@link AUDIO_OK} if the operation is successfully.
    * @test
    */
    int32_t RemoveThreadFromGroup(int32_t workgroupId, int32_t tokenId);

    /**
    * @brief the deadline workgroup starts to take effect.
    *
    * @param workgroupId workgroupId audio workgroup id.
    * @param startTime timestamp when the deadline task starts to be executed.
    * @param deadlineTime complete a periodic task within the time specified by deadlineTime.
    * @return Returns {@link AUDIO_OK} if the operation is successfully.
    * @test
    */
    int32_t StartGroup(int32_t workgroupId, uint64_t startTime, uint64_t deadlineTime,
        std::unordered_map<int32_t, bool> threads, bool &needUpdatePrio);

    /**
    * @brief stop the deadline workgroup.
    *
    * @param workgroupId workgroupId audio workgroup id.
    * @return Returns {@link AUDIO_OK} if the operation is successfully.
    * @test
    */
    int32_t StopGroup(int32_t workgroupId);

    void OnWorkgroupChange(const AudioWorkgroupChangeInfo &info);
    std::shared_ptr<WorkgroupPrioRecorder> GetRecorderByGrpId(int32_t grpId);
    int32_t ExecuteAudioWorkgroupPrioImprove(int32_t workgroupId,
        const std::unordered_map<int32_t, bool> threads, bool &needUpdatePrio);
private:
    WorkgroupPrioRecorderManager() = default;
    ~WorkgroupPrioRecorderManager() = default;

    bool IsValidToStartGroup(int32_t workgroupId);
    
    bool hasSystemPermission_ = false;
    std::mutex startGroupPermissionMapMutex_;
    std::mutex workgroupPrioRecorderMutex_;
    std::unordered_map<int32_t, std::shared_ptr<WorkgroupPrioRecorder>> workgroupPrioRecorderMap_;
    std::unordered_map<uint32_t, std::unordered_map<uint32_t, bool>> startGroupPermissionMap_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_WORKGROUP_CLIENT_MANAGER_H
