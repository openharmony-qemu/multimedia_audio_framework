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

#ifndef AUDIO_MANAGER_LISTENER_STUB_IMPL_H
#define AUDIO_MANAGER_LISTENER_STUB_IMPL_H

#include <thread>

#include "audio_policy_interface.h"
#include "audio_system_manager.h"
#include "standard_audio_server_manager_listener_stub.h"

namespace OHOS {
namespace AudioStandard {
class AudioManagerListenerStubImpl : public StandardAudioServerManagerListenerStub {
public:
    int32_t OnAudioParameterChange(const std::string &networkId, int32_t key, const std::string& condition,
        const std::string& value) override;
    int32_t OnCapturerState(bool isActive) override;
    int32_t OnWakeupClose() override;
    int32_t OnDataTransferStateChange(int32_t callbackId,
        const AudioRendererDataTransferStateChangeInfo &info) override;
    int32_t OnMuteStateChange(int32_t callbackId, int32_t uid, uint32_t sessionId, bool isMuted) override;

    void SetParameterCallback(const std::weak_ptr<AudioParameterCallback>& callback);
    void SetWakeupSourceCallback(const std::weak_ptr<WakeUpSourceCallback>& callback);
    int32_t AddDataTransferStateChangeCallback(const DataTransferMonitorParam &param,
        std::shared_ptr<AudioRendererDataTransferStateChangeCallback> cb);
    std::vector<int32_t> RemoveDataTransferStateChangeCallback(
        std::shared_ptr<AudioRendererDataTransferStateChangeCallback> cb);
private:
    using ParamPair = std::pair<DataTransferMonitorParam,
        std::shared_ptr<AudioRendererDataTransferStateChangeCallback>>;
    std::weak_ptr<AudioParameterCallback> callback_;
    std::weak_ptr<WakeUpSourceCallback> wakeUpCallback_;
    int32_t callbackId_ = 0;
    std::mutex stateChangeMutex_;
    std::unordered_map<int32_t, ParamPair> stateChangeCallbackMap_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_MANAGER_LISTENER_STUB_IMPL_H
