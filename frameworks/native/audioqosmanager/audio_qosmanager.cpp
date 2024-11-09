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

#include "audio_qosmanager.h"
#include <unistd.h>
#include <cstring>
#include <unordered_map>

#ifdef QOSMANAGER_ENABLE
#include "qos.h"
#include "concurrent_task_client.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef QOSMANAGER_ENABLE
void SetThreadQosLevel()
{
    std::unordered_map<std::string, std::string> payload;
    payload["pid"] = std::to_string(getpid());
    OHOS::ConcurrentTask::ConcurrentTaskClient::GetInstance().RequestAuth(payload);
    OHOS::QOS::SetThreadQos(OHOS::QOS::QosLevel::QOS_USER_INTERACTIVE);
}
void ReSetThreadQosLevel()
{
    OHOS::QOS::ResetThreadQos();
}
#else
void SetThreadQosLevel() {};
void ReSetThreadQosLevel() {};
#endif


#ifdef __cplusplus
}
#endif
