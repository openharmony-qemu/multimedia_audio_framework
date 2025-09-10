/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioQosManager"
#endif

#include "audio_qosmanager.h"
#include <unistd.h>
#include <cstring>
#include <unordered_map>

#ifdef QOSMANAGER_ENABLE
#include <chrono>
#include <thread>
#include "audio_common_log.h"
#include "audio_schedule.h"
#include "qos.h"
#include "concurrent_task_client.h"
#include "parameter.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef QOSMANAGER_ENABLE

constexpr int32_t AUDIO_PROC_QOS_TABLE = 7;

void SetThreadQosLevel(void)
{
    std::unordered_map<std::string, std::string> payload;
    payload["groupId"] = std::to_string(AUDIO_PROC_QOS_TABLE);
    payload["pid"] = std::to_string(getpid());
    OHOS::ConcurrentTask::ConcurrentTaskClient::GetInstance().RequestAuth(payload);
    int32_t ret = OHOS::QOS::SetThreadQos(OHOS::QOS::QosLevel::QOS_USER_INTERACTIVE);
    CHECK_AND_RETURN_LOG(ret == 0, "set thread qos failed, ret = %{public}d", ret);
    AUDIO_INFO_LOG("set thread qos success");
}

static void SetThreadQosLevelWithTid(int32_t pid, int32_t tid, int32_t setPriority)
{
    std::unordered_map<std::string, std::string> payload;
    payload["groupId"] = std::to_string(AUDIO_PROC_QOS_TABLE);
    payload["pid"] = std::to_string(pid);
    OHOS::ConcurrentTask::ConcurrentTaskClient::GetInstance().RequestAuth(payload);

    int32_t ret;
    if (setPriority == 1) {
        ret = OHOS::QOS::SetQosForOtherThread(OHOS::QOS::QosLevel::QOS_USER_INTERACTIVE, tid);
        CHECK_AND_RETURN_LOG(ret == 0, "set thread qos failed, ret = %{public}d", ret);
        AUDIO_INFO_LOG("set qos %{public}d for thread %{public}d success",
            OHOS::QOS::QosLevel::QOS_USER_INTERACTIVE, tid);
        return;
    }

    ret = OHOS::QOS::SetQosForOtherThread(OHOS::QOS::QosLevel::QOS_KEY_BACKGROUND, tid);
    CHECK_AND_RETURN_LOG(ret == 0, "set thread qos failed, ret = %{public}d", ret);
    AUDIO_INFO_LOG("set qos %{public}d for thread %{public}d success", OHOS::QOS::QosLevel::QOS_KEY_BACKGROUND, tid);
}

void SetThreadQosLevelAsync(int32_t setPriority)
{
    AUDIO_INFO_LOG("set thread qos level start");
    int32_t tid = gettid();
    int32_t pid = getpid();
    std::thread setThreadQosLevelThread = std::thread([=] { SetThreadQosLevelWithTid(pid, tid, setPriority); });
    setThreadQosLevelThread.detach();
}

void ResetThreadQosLevel(void)
{
    OHOS::QOS::ResetThreadQos();
}
#else
void SetThreadQosLevel(void) {};
void SetThreadQosLevelAsync(int32_t setPriority) {};
void ResetThreadQosLevel(void) {};
#endif


#ifdef __cplusplus
}
#endif
