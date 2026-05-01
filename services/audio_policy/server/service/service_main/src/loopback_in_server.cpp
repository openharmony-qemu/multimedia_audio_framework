/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "loopback_in_server.h"
#include "audio_loopback_manager.h"
#include "audio_policy_log.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {

LoopbackInServer::LoopbackInServer(AudioLoopbackMode mode, LoopbackType type, int32_t pid,
    std::shared_ptr<AudioLoopbackManager> manager)
    : mode_(mode), type_(type), pid_(pid), manager_(manager)
{
    AUDIO_INFO_LOG("LoopbackInServer created, mode %{public}d, type %{public}d, pid %{public}d",
        static_cast<int32_t>(mode), static_cast<int32_t>(type), pid);
}

LoopbackInServer::~LoopbackInServer()
{
    AUDIO_INFO_LOG("LoopbackInServer destroyed, pid %{public}d", pid_);
}

int32_t LoopbackInServer::Enable(bool enable, int32_t &ret)
{
    AUDIO_INFO_LOG("enable %{public}d, type %{public}d", enable, static_cast<int32_t>(type_));
    
    auto manager = manager_.lock();
    if (manager == nullptr) {
        AUDIO_ERR_LOG("manager_ is nullptr");
        ret = ERR_NULL_POINTER;
        return SUCCESS;
    }

    ret = manager->Enable(type_, enable, pid_);
    return SUCCESS;
}

int32_t LoopbackInServer::GetStatus(int32_t &status, int32_t &ret)
{
    AUDIO_DEBUG_LOG("LoopbackInServer::GetStatus, type %{public}d", static_cast<int32_t>(type_));
    
    auto manager = manager_.lock();
    if (manager == nullptr) {
        AUDIO_ERR_LOG("manager_ is nullptr");
        ret = ERR_NULL_POINTER;
        return SUCCESS;
    }

    int statusInt = 0;
    ret = manager->GetStatus(type_, statusInt, pid_);
    status = static_cast<int32_t>(statusInt);
    return SUCCESS;
}

int32_t LoopbackInServer::GetVolume(float &volume, int32_t &ret)
{
    AUDIO_DEBUG_LOG("LoopbackInServer::GetVolume, type %{public}d", static_cast<int32_t>(type_));
    
    auto manager = manager_.lock();
    if (manager == nullptr) {
        AUDIO_ERR_LOG("manager_ is nullptr");
        ret = ERR_NULL_POINTER;
        return SUCCESS;
    }

    ret = manager->GetVolume(type_, volume, pid_);
    return SUCCESS;
}

} // namespace AudioStandard
} // namespace OHOS