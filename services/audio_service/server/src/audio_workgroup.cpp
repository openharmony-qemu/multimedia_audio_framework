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
#include "audio_common_log.h"
#include "rtg_interface.h"
#include "audio_utils.h"
#include "concurrent_task_client.h"

using namespace OHOS::RME;

namespace OHOS {
namespace AudioStandard {
constexpr unsigned int MS_PER_SECOND = 1000;

AudioWorkgroup::AudioWorkgroup(int32_t id) : workgroupId(id)
{
    AUDIO_INFO_LOG("OHAudioWorkgroup Constructor is called\n");
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
    ConcurrentTask::IntervalReply reply;
    OHOS::ConcurrentTask::ConcurrentTaskClient::GetInstance().SetAudioDeadline(
        ConcurrentTask::AUDIO_DDL_ADD_THREAD, tid, workgroupId, reply);
    if (reply.paramA < 0) {
        AUDIO_INFO_LOG("AudioWorkgroup AddThread Failed!\n");
        return AUDIO_ERR;
    }
    threads[tid] = true;
    return AUDIO_OK;
}

int32_t AudioWorkgroup::RemoveThread(int32_t tid)
{
    ConcurrentTask::IntervalReply reply;
    OHOS::ConcurrentTask::ConcurrentTaskClient::GetInstance().SetAudioDeadline(
        ConcurrentTask::AUDIO_DDL_REMOVE_THREAD, tid, workgroupId, reply);
    if (reply.paramA < 0) {
        AUDIO_INFO_LOG("AudioWorkgroup RemoveThread Failed!\n");
        return AUDIO_ERR;
    }
    threads.erase(tid);
    return AUDIO_OK;
}

int32_t AudioWorkgroup::Start(uint64_t startTime, uint64_t deadlineTime)
{
    if (deadlineTime <= startTime) {
        AUDIO_ERR_LOG("[WorkgroupInServer] Invalid params When Start!");
        return AUDIO_ERR;
    }
    SetFrameRateAndPrioType(workgroupId, MS_PER_SECOND/(deadlineTime - startTime), 0);
    if (BeginFrameFreq(0) != 0) {
        AUDIO_ERR_LOG("[WorkgroupInServer] Audio Deadline BeginFrame Failed!");
        return AUDIO_ERR;
    }
    return AUDIO_OK;
}

int32_t AudioWorkgroup::Stop()
{
    if (EndFrameFreq(0) != 0) {
        AUDIO_ERR_LOG("[WorkgroupInServer] Audio Deadline EndFrame Failed!");
        return AUDIO_ERR;
    }
    return AUDIO_OK;
}
} // namespace AudioStandard
} // namespace OHOS