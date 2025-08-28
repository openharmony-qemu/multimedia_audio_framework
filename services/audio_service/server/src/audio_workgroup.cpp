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

#ifndef LOG_TAG
#define LOG_TAG "AudioWorkgroup"
#endif

#include "audio_workgroup.h"
#include "rtg_interface.h"
#include "audio_common_log.h"
#include "audio_utils.h"
#include "concurrent_task_client.h"

namespace OHOS {
namespace AudioStandard {
constexpr unsigned int MS_PER_SECOND = 1000;

AudioWorkgroup::AudioWorkgroup(int32_t id) : workgroupId(id)
{
    AUDIO_INFO_LOG("OHAudioWorkgroup Constructor is called\n");
    SetCgroupLimitParams(0, -1);
}

int32_t AudioWorkgroup::GetWorkgroupId()
{
    return workgroupId;
}

uint32_t AudioWorkgroup::GetThreadsNums()
{
    return threads.size();
}

int32_t AudioWorkgroup::AddThread(int32_t tid)
{
    Trace trace("[WorkgroupInServer] AddThread tid:" + std::to_string(tid) +
        " workgroupId:" + std::to_string(workgroupId));
    ConcurrentTask::IntervalReply reply;
    reply.paramA = cgroupLimit.clientPid;
    reply.paramB = cgroupLimit.globalCgroupId;
    OHOS::ConcurrentTask::ConcurrentTaskClient::GetInstance().SetAudioDeadline(
        ConcurrentTask::AUDIO_DDL_ADD_THREAD, tid, workgroupId, reply);
    if (reply.paramA < 0) {
        AUDIO_INFO_LOG("[WorkgroupInServer] AudioWorkgroup AddThread Failed\n");
        return AUDIO_ERR;
    }
    threads[tid] = true;
    return AUDIO_OK;
}

int32_t AudioWorkgroup::RemoveThread(int32_t tid)
{
    Trace trace("[WorkgroupInServer] RemoveThread tid:" + std::to_string(tid) +
        " workgroupId:" + std::to_string(workgroupId));
    ConcurrentTask::IntervalReply reply;
    reply.paramA = cgroupLimit.clientPid;
    reply.paramB = cgroupLimit.globalCgroupId;
    OHOS::ConcurrentTask::ConcurrentTaskClient::GetInstance().SetAudioDeadline(
        ConcurrentTask::AUDIO_DDL_REMOVE_THREAD, tid, workgroupId, reply);
    if (reply.paramA < 0) {
        AUDIO_INFO_LOG("[WorkgroupInServer] AudioWorkgroup RemoveThread Failed\n");
        return AUDIO_ERR;
    }
    threads.erase(tid);
    return AUDIO_OK;
}

int32_t AudioWorkgroup::Start(uint64_t startTime, uint64_t deadlineTime)
{
    Trace trace("[WorkgroupInServer] Start workgroupId:" + std::to_string(workgroupId) +
        " startTime:" + std::to_string(startTime) + " deadlineTime:" + std::to_string(deadlineTime));
    if (deadlineTime <= startTime) {
        AUDIO_ERR_LOG("[WorkgroupInServer] Invalid params When Start.");
        return AUDIO_ERR;
    }
    RME::SetFrameRateAndPrioType(workgroupId, MS_PER_SECOND/(deadlineTime - startTime), 0);
    if (RME::BeginFrameFreq(deadlineTime - startTime) != 0) {
        AUDIO_ERR_LOG("[WorkgroupInServer] Audio Deadline BeginFrame failed");
        return AUDIO_ERR;
    }
    return AUDIO_OK;
}

int32_t AudioWorkgroup::Stop()
{
    Trace trace("[WorkgroupInServer] Stop workgroupId:" + std::to_string(workgroupId));
    if (RME::EndFrameFreq(0) != 0) {
        AUDIO_ERR_LOG("[WorkgroupInServer] Audio Deadline EndFrame failed");
        return AUDIO_ERR;
    }
    return AUDIO_OK;
}

int32_t AudioWorkgroup::GetCgroupLimitId()
{
    return cgroupLimit.globalCgroupId;
}

void AudioWorkgroup::SetCgroupLimitParams(int32_t pid, int32_t globalCgroupId)
{
    cgroupLimit.clientPid = pid;
    cgroupLimit.globalCgroupId = globalCgroupId;
}

} // namespace AudioStandard
} // namespace OHOS