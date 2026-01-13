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

#ifndef AUDIO_ENGINE_CLIENT_MANAGER_H
#define AUDIO_ENGINE_CLIENT_MANAGER_H

#include <mutex>
#include "singleton.h"

#include "iremote_object.h"

#include "audio_engine_callback_types.h"
#include "istandard_audio_service.h"
#include "audio_engine_callback_handle_stub.h"

namespace OHOS {
namespace AudioStandard {

class AudioEngineClientManager {
    DECLARE_DELAYED_SINGLETON(AudioEngineClientManager)
public:
    int32_t GetCurrentOutputPipeChangeInfos(
        std::vector<std::shared_ptr<AudioOutputPipeInfo>> &pipeChangeInfos);
    int32_t GetCurrentInputPipeChangeInfos(
        std::vector<std::shared_ptr<AudioInputPipeInfo>> &pipeChangeInfos);
    // Do not register or unregister in the callback function, this will lead to deadlock
    int32_t RegisterOutputPipeChangeCallback(std::shared_ptr<AudioOutputPipeCallback> &callback);
    int32_t UnregisterOutputPipeChangeCallback(std::shared_ptr<AudioOutputPipeCallback> &callback);
    int32_t RegisterInputPipeChangeCallback(std::shared_ptr<AudioInputPipeCallback> &callback);
    int32_t UnregisterInputPipeChangeCallback(std::shared_ptr<AudioInputPipeCallback> &callback);
    int32_t SetAuxiliarySinkEnable(bool isEnabled);
private:
    class CallbackHandle : public IRemoteObject::DeathRecipient,
                           public AudioEngineCallbackHandleStub {
    public:
        explicit CallbackHandle() = default;
        virtual ~CallbackHandle() = default;
        DISALLOW_COPY_AND_MOVE(CallbackHandle);

        // Implements IRemoteObject::DeathRecipient interfaces
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override;

        // Implements AudioEngineCallbackHandleStub interfaces
        int32_t OnOutputPipeChange(int32_t changeType,
            const std::shared_ptr<AudioOutputPipeInfo> &changedPipeInfo) override;
        int32_t OnInputPipeChange(int32_t changeType,
            const std::shared_ptr<AudioInputPipeInfo> &changedPipeInfo) override;

        // Self funcs
        bool AddOutputPipeChangeCallback(std::shared_ptr<AudioOutputPipeCallback> &callback);
        bool RemoveOutputPipeChangeCallback(std::shared_ptr<AudioOutputPipeCallback> &callback);
        bool IsOutputPipeChangeEnable();
        bool AddInputPipeChangeCallback(std::shared_ptr<AudioInputPipeCallback> &callback);
        bool RemoveInputPipeChangeCallback(std::shared_ptr<AudioInputPipeCallback> &callback);
        bool IsInputPipeChangeEnable();

    private:
        std::mutex lock_;
        std::vector<std::shared_ptr<AudioOutputPipeCallback>> outputPipeCbs_;
        std::vector<std::shared_ptr<AudioInputPipeCallback>> inputPipeCbs_;
    };

private:
    // helper to get sa proxy and callback handle
    static const sptr<IStandardAudioService> InitAndGetAudioServiceProxy();

    static sptr<IStandardAudioService> gServerProxy;
    static sptr<CallbackHandle> gCallbackHandle;
    static std::mutex gServerProxyLock;

    std::mutex lock_;
};

} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_ENGINE_CLIENT_MANAGER_H
