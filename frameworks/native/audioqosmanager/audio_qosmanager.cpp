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
#include <thread>
#include "audio_common_log.h"
#include "qos.h"
#include "concurrent_task_client.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef QOSMANAGER_ENABLE
void SetThreadQosLevel(void)
{
    std::unordered_map<std::string, std::string> payload;
    payload["pid"] = std::to_string(getpid());
    OHOS::ConcurrentTask::ConcurrentTaskClient::GetInstance().RequestAuth(payload);
    int32_t ret = OHOS::QOS::SetThreadQos(OHOS::QOS::QosLevel::QOS_USER_INTERACTIVE);
    CHECK_AND_RETURN_LOG(ret == 0, "set thread qos failed, ret = %{public}d", ret);
    AUDIO_INFO_LOG("set thread qos success");
}

static void SetThreadQosLevelWithTid(int32_t tid)
{
    std::unordered_map<std::string, std::string> payload;
    payload["pid"] = std::to_string(getpid());
    OHOS::ConcurrentTask::ConcurrentTaskClient::GetInstance().RequestAuth(payload);
    int32_t ret = OHOS::QOS::SetQosForOtherThread(OHOS::QOS::QosLevel::QOS_USER_INTERACTIVE, tid);
    CHECK_AND_RETURN_LOG(ret == 0, "set qos for thread %{public}d failed, ret = %{public}d", tid, ret);
    AUDIO_INFO_LOG("set qos for thread %{public}d success", tid);
}

void SetThreadQosLevelAsync(void)
{
    int32_t tid = gettid();
    std::thread setThreadQosLevelThread = std::thread([=] { SetThreadQosLevelWithTid(tid); });
    setThreadQosLevelThread.detach();
}

void ReSetThreadQosLevel(void)
{
    OHOS::QOS::ResetThreadQos();
}
#else
void SetThreadQosLevel(void) {};
void SetThreadQosLevelAsync(void) {};
void ReSetThreadQosLevel(void) {};
#endif


#ifdef __cplusplus
}
#endif
