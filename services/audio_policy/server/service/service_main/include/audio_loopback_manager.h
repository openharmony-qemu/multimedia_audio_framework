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

#ifndef AUDIO_LOOPBACK_MANAGER_H
#define AUDIO_LOOPBACK_MANAGER_H

#include <mutex>
#include <map>
#include <memory>

#include "iremote_object.h"
#include "audio_info.h"
#include "iloopback_callback.h"

namespace OHOS {
namespace AudioStandard {

class LoopbackInServer;

class LoopbackDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    explicit LoopbackDeathRecipient(std::function<void()> callback) : callback_(callback) {}
    ~LoopbackDeathRecipient() = default;
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override
    {
        if (callback_) {
            callback_();
        }
    }
private:
    std::function<void()> callback_;
};

class AudioLoopbackManager : public std::enable_shared_from_this<AudioLoopbackManager> {
public:
    AudioLoopbackManager();
    ~AudioLoopbackManager();

    int32_t CreateLoopback(AudioLoopbackMode mode, LoopbackType type,
        const sptr<ILoopbackCallback> &callback, sptr<IRemoteObject> &loopback, int32_t pid);

    int32_t DestroyLoopback(LoopbackType type, int32_t pid);

    int32_t Enable(LoopbackType type, bool enable, int32_t pid);

    int32_t GetStatus(LoopbackType type, int &status, int32_t pid);

    int32_t GetVolume(LoopbackType type, float &volume, int32_t pid);

    void OnHandlerDied();
    void OnControlDied(int32_t pid);
    void OnNormalDied(int32_t pid);

private:
    std::mutex mutex_;

    std::map<int32_t, sptr<LoopbackInServer>> normalLoopbacks_;

    sptr<ILoopbackCallback> globalHandlerCallback_;
    int32_t handlerPid_ = -1;
    AudioLoopbackStatus globalHandlerState_ = LOOPBACK_AVAILABLE_IDLE;

    std::map<int32_t, sptr<LoopbackInServer>> globalControls_;
    std::map<int32_t, sptr<ILoopbackCallback>> globalControlCallbacks_;

    float globalHandlerVolume_ = 0.0f;

    int32_t CheckCreateCondition(LoopbackType type, int32_t pid);
    void RegisterLoopback(LoopbackType type, int32_t pid, const sptr<ILoopbackCallback> &callback,
        const sptr<LoopbackInServer> &loopbackInServer);

    // Must be called with mutex_.
    void BroadcastToControls(int command, bool success, int errorCode);
    void BroadcastStatusChange(int status, int reason);
};

} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_LOOPBACK_MANAGER_H