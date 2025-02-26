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
const std::string BOOT_ANIMATION_FINISHED_EVENT = "bootevent.bootanimation.finished";
constexpr int32_t WAIT_FOR_BOOT_ANIMATION_S = 14;
constexpr int32_t WAIT_TIME_FOR_UNSCHEDULE_MS = 100;
constexpr int32_t WAIT_COUNT_FOR_UNSCHEDULE = 5;
void SetThreadQosLevel(void)
{
    std::unordered_map<std::string, std::string> payload;
    payload["pid"] = std::to_string(getpid());
    OHOS::ConcurrentTask::ConcurrentTaskClient::GetInstance().RequestAuth(payload);
    int32_t ret = OHOS::QOS::SetThreadQos(OHOS::QOS::QosLevel::QOS_USER_INTERACTIVE);
    CHECK_AND_RETURN_LOG(ret == 0, "set thread qos failed, ret = %{public}d", ret);
    AUDIO_INFO_LOG("set thread qos success");
}

static void SetThreadQosLevelWithTid(int32_t pid, int32_t tid)
{
    int32_t ret = WaitParameter(BOOT_ANIMATION_FINISHED_EVENT.c_str(), "true", WAIT_FOR_BOOT_ANIMATION_S);
    if (ret != 0) {
        AUDIO_ERR_LOG("wait for boot animation failed or timeout, ret = %{public}d", ret);
    }
    UnscheduleThreadInServer(static_cast<uint32_t>(pid), static_cast<uint32_t>(tid));
    std::unordered_map<std::string, std::string> payload;
    payload["pid"] = std::to_string(pid);
    OHOS::ConcurrentTask::ConcurrentTaskClient::GetInstance().RequestAuth(payload);
    int32_t retryCount = 0;
    while (retryCount < WAIT_COUNT_FOR_UNSCHEDULE) {
        ret = OHOS::QOS::SetQosForOtherThread(OHOS::QOS::QosLevel::QOS_USER_INTERACTIVE, tid);
        if (ret != 0) {
            AUDIO_WARNING_LOG("set qos for thread %{public}d failed, current retry count %{public}d, ret = %{public}d",
                tid, retryCount, ret);
            retryCount++;
            std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME_FOR_UNSCHEDULE_MS));
        } else {
            AUDIO_INFO_LOG("set qos for thread %{public}d success", tid);
            return;
        }
    }
    AUDIO_ERR_LOG("set qos for thread %{public}d finally failed", tid);
}

void SetThreadQosLevelAsync(void)
{
    int32_t tid = gettid();
    int32_t pid = getpid();
    std::thread setThreadQosLevelThread = std::thread([=] { SetThreadQosLevelWithTid(pid, tid); });
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
