/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "audio_loopback_manager.h"
#include "loopback_in_server.h"
#include "audio_policy_log.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {

AudioLoopbackManager::AudioLoopbackManager()
{
    AUDIO_INFO_LOG("AudioLoopbackManager constructed");
}

AudioLoopbackManager::~AudioLoopbackManager()
{
    std::lock_guard<std::mutex> lock(mutex_);
    normalLoopbacks_.clear();
    globalHandlerCallback_ = nullptr;
    globalControls_.clear();
    AUDIO_INFO_LOG("AudioLoopbackManager destructed");
}

int32_t AudioLoopbackManager::CreateLoopback(AudioLoopbackMode mode, LoopbackType type,
    const sptr<ILoopbackCallback> &callback, sptr<IRemoteObject> &loopback, int32_t pid)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (mode != LOOPBACK_HARDWARE) {
        AUDIO_ERR_LOG("mode not supported: %{public}d", mode);
        return ERR_NOT_SUPPORTED;
    }

    int32_t checkResult = CheckCreateCondition(type, pid);
    if (checkResult != SUCCESS) {
        AUDIO_ERR_LOG("CheckCreateCondition failed: %{public}d", checkResult);
        return checkResult;
    }

    sptr<LoopbackInServer> loopbackInServer = new LoopbackInServer(mode, type, pid,
        std::enable_shared_from_this<AudioLoopbackManager>::shared_from_this());
    loopback = loopbackInServer;

    RegisterLoopback(type, pid, callback, loopbackInServer);

    AUDIO_INFO_LOG("LoopbackInServer created for pid %{public}d, type %{public}d", pid, static_cast<int32_t>(type));
    return SUCCESS;
}

int32_t AudioLoopbackManager::CheckCreateCondition(LoopbackType type, int32_t pid)
{
    switch (type) {
        case LOOPBACK_TYPE_NORMAL:
            return SUCCESS;
        case LOOPBACK_TYPE_GLOBAL_HANDLER:
            if (globalHandlerCallback_ != nullptr) {
                AUDIO_ERR_LOG("global handler already exists, pid %{public}d", handlerPid_);
                return ERROR_LOOPBACK_HANDLER_ALREADY_EXIST;
            }
            return SUCCESS;
        case LOOPBACK_TYPE_GLOBAL_CONTROL:
            if (globalHandlerCallback_ == nullptr) {
                AUDIO_ERR_LOG("global handler not exist");
                return ERROR_LOOPBACK_HANDLER_NOT_EXIST;
            }
            return SUCCESS;
        default:
            AUDIO_ERR_LOG("unknown type: %{public}d", static_cast<int32_t>(type));
            return ERROR_LOOPBACK_OPERATION_NOT_SUPPORTED;
    }
}

void AudioLoopbackManager::RegisterLoopback(LoopbackType type, int32_t pid,
    const sptr<ILoopbackCallback> &callback, const sptr<LoopbackInServer> &loopbackInServer)
{
    switch (type) {
        case LOOPBACK_TYPE_NORMAL: {
            sptr<IRemoteObject::DeathRecipient> deathRecipient = new LoopbackDeathRecipient(
                [this, pid]() { OnNormalDied(pid); });
            callback->AsObject()->AddDeathRecipient(deathRecipient);
            normalLoopbacks_[pid] = loopbackInServer;
            AUDIO_INFO_LOG("normal loopback registered for pid %{public}d", pid);
            break;
        }
        case LOOPBACK_TYPE_GLOBAL_HANDLER: {
            sptr<IRemoteObject::DeathRecipient> handlerDeathRecipient = new LoopbackDeathRecipient([this]() {
                OnHandlerDied();
            });
            callback->AsObject()->AddDeathRecipient(handlerDeathRecipient);
            globalHandlerCallback_ = callback;
            handlerPid_ = pid;
            globalHandlerState_ = LOOPBACK_AVAILABLE_IDLE;
            globalHandlerVolume_ = 0.0f;
            AUDIO_INFO_LOG("global handler registered for pid %{public}d", pid);
            break;
        }
        case LOOPBACK_TYPE_GLOBAL_CONTROL: {
            sptr<IRemoteObject::DeathRecipient> deathRecipient = new LoopbackDeathRecipient(
                [this, pid]() { OnControlDied(pid); });
            callback->AsObject()->AddDeathRecipient(deathRecipient);
            globalControls_[pid] = loopbackInServer;
            globalControlCallbacks_[pid] = callback;
            AUDIO_INFO_LOG("global control registered for pid %{public}d", pid);
            break;
        }
        default:
            break;
    }
}

int32_t AudioLoopbackManager::DestroyLoopback(LoopbackType type, int32_t pid)
{
    std::lock_guard<std::mutex> lock(mutex_);

    switch (type) {
        case LOOPBACK_TYPE_NORMAL: {
            auto it = normalLoopbacks_.find(pid);
            if (it != normalLoopbacks_.end()) {
                normalLoopbacks_.erase(it);
                AUDIO_INFO_LOG("normal loopback destroyed for pid %{public}d", pid);
            }
            break;
        }
        case LOOPBACK_TYPE_GLOBAL_HANDLER:
            if (handlerPid_ == pid) {
                globalHandlerCallback_ = nullptr;
                handlerPid_ = -1;
                globalHandlerState_ = LOOPBACK_AVAILABLE_IDLE;
                globalHandlerVolume_ = 0.0f;
                BroadcastStatusChange(static_cast<int>(LOOPBACK_AVAILABLE_IDLE),
                    static_cast<int>(LOOPBACK_REASON_HANDLER_DIED));
                AUDIO_INFO_LOG("global handler destroyed for pid %{public}d", pid);
            }
            break;
        case LOOPBACK_TYPE_GLOBAL_CONTROL: {
            auto it = globalControls_.find(pid);
            if (it != globalControls_.end()) {
                globalControls_.erase(it);
                AUDIO_INFO_LOG("global control destroyed for pid %{public}d", pid);
            }
            auto itr = globalControlCallbacks_.find(pid);
            if (itr != globalControlCallbacks_.end()) {
                globalControlCallbacks_.erase(itr);
                AUDIO_INFO_LOG("global control callback destroyed for pid %{public}d", pid);
            }
            break;
        }
        default:
            AUDIO_ERR_LOG("unknown type: %{public}d", static_cast<int32_t>(type));
            return ERROR_LOOPBACK_OPERATION_NOT_SUPPORTED;
    }
    return SUCCESS;
}

int32_t AudioLoopbackManager::Enable(LoopbackType type, bool enable, int32_t pid)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (type == LOOPBACK_TYPE_GLOBAL_CONTROL) {
        if (globalHandlerCallback_ == nullptr) {
            AUDIO_ERR_LOG("global handler not exist");
            return ERROR_LOOPBACK_HANDLER_NOT_EXIST;
        }

        if (enable && globalHandlerState_ == LOOPBACK_AVAILABLE_RUNNING) {
            AUDIO_DEBUG_LOG("already enabled, return success");
            BroadcastToControls(static_cast<int>(LOOPBACK_CMD_ENABLE), true, SUCCESS);
            return SUCCESS;
        }

        if (!enable && globalHandlerState_ == LOOPBACK_AVAILABLE_IDLE) {
            AUDIO_DEBUG_LOG("already disabled, return success");
            BroadcastToControls(static_cast<int>(LOOPBACK_CMD_DISABLE), true, SUCCESS);
            return SUCCESS;
        }

        globalHandlerCallback_->OnCommandResult(enable ? LOOPBACK_CMD_ENABLE : LOOPBACK_CMD_DISABLE, true, SUCCESS);

        if (enable) {
            globalHandlerState_ = LOOPBACK_AVAILABLE_RUNNING;
        } else {
            globalHandlerState_ = LOOPBACK_AVAILABLE_IDLE;
        }

        BroadcastToControls(enable ? static_cast<int>(LOOPBACK_CMD_ENABLE) : static_cast<int>(LOOPBACK_CMD_DISABLE),
            true, SUCCESS);
        return SUCCESS;
    }

    if (type == LOOPBACK_TYPE_GLOBAL_HANDLER) {
        if (enable) {
            globalHandlerState_ = LOOPBACK_AVAILABLE_RUNNING;
        } else {
            globalHandlerState_ = LOOPBACK_AVAILABLE_IDLE;
        }
    }

    return SUCCESS;
}

int32_t AudioLoopbackManager::GetStatus(LoopbackType type, int &status, int32_t pid)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (type == LOOPBACK_TYPE_GLOBAL_CONTROL) {
        if (globalHandlerCallback_ == nullptr) {
            AUDIO_ERR_LOG("global handler not exist");
            return ERROR_LOOPBACK_HANDLER_NOT_EXIST;
        }
        status = static_cast<int>(globalHandlerState_);
        return SUCCESS;
    }

    AUDIO_ERR_LOG("GetStatus not supported for type %{public}d", static_cast<int32_t>(type));
    return ERROR_LOOPBACK_OPERATION_NOT_SUPPORTED;
}

int32_t AudioLoopbackManager::GetVolume(LoopbackType type, float &volume, int32_t pid)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (type == LOOPBACK_TYPE_GLOBAL_CONTROL) {
        if (globalHandlerCallback_ == nullptr) {
            AUDIO_ERR_LOG("global handler not exist");
            return ERROR_LOOPBACK_HANDLER_NOT_EXIST;
        }
        volume = globalHandlerVolume_;
        return SUCCESS;
    }

    AUDIO_ERR_LOG("GetVolume not supported for type %{public}d", static_cast<int32_t>(type));
    return ERROR_LOOPBACK_OPERATION_NOT_SUPPORTED;
}

void AudioLoopbackManager::OnHandlerDied()
{
    AUDIO_INFO_LOG("handler died");
    std::lock_guard<std::mutex> lock(mutex_);
    
    globalHandlerCallback_ = nullptr;
    handlerPid_ = -1;
    globalHandlerState_ = LOOPBACK_AVAILABLE_IDLE;
    globalHandlerVolume_ = 0.0f;

    BroadcastStatusChange(static_cast<int>(LOOPBACK_AVAILABLE_IDLE),
        static_cast<int>(LOOPBACK_REASON_HANDLER_DIED));
}

void AudioLoopbackManager::OnControlDied(int32_t pid)
{
    AUDIO_INFO_LOG("control died, pid %{public}d", pid);
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = globalControls_.find(pid);
    if (it != globalControls_.end()) {
        globalControls_.erase(it);
    }
    auto itr = globalControlCallbacks_.find(pid);
    if (itr != globalControlCallbacks_.end()) {
        globalControlCallbacks_.erase(itr);
    }
}

void AudioLoopbackManager::OnNormalDied(int32_t pid)
{
    AUDIO_INFO_LOG("normal died, pid %{public}d", pid);
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = normalLoopbacks_.find(pid);
    if (it != normalLoopbacks_.end()) {
        normalLoopbacks_.erase(it);
    }
}

void AudioLoopbackManager::BroadcastToControls(int command, bool success, int errorCode)
{
    for (auto &item : globalControlCallbacks_) {
        auto loopbackCb = item.second;
        if (loopbackCb != nullptr) {
            if (command == LOOPBACK_CMD_ENABLE) {
                loopbackCb->OnStatusChange(LOOPBACK_AVAILABLE_RUNNING, CMD_FROM_SYSTEM);
            } else if (command == LOOPBACK_CMD_DISABLE) {
                loopbackCb->OnStatusChange(LOOPBACK_AVAILABLE_IDLE, CMD_FROM_SYSTEM);
            }
        }
    }
}

void AudioLoopbackManager::BroadcastStatusChange(int status, int reason)
{
    for (auto &item : globalControlCallbacks_) {
        auto loopbackCb = item.second;
        if (loopbackCb != nullptr) {
            loopbackCb->OnStatusChange(LOOPBACK_AVAILABLE_IDLE, CMD_FROM_SYSTEM);
        }
    }
}

} // namespace AudioStandard
} // namespace OHOS