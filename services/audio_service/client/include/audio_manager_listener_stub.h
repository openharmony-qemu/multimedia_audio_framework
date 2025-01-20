/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef AUDIO_MANAGER_LISTENER_STUB_H
#define AUDIO_MANAGER_LISTENER_STUB_H

#include <thread>

#include "audio_policy_interface.h"
#include "audio_system_manager.h"
#include "i_standard_audio_server_manager_listener.h"

namespace OHOS {
namespace AudioStandard {
class AudioManagerListenerStub : public IRemoteStub<IStandardAudioServerManagerListener> {
public:
    AudioManagerListenerStub();
    virtual ~AudioManagerListenerStub();

    // IStandardAudioManagerListener override
    int OnRemoteRequest(uint32_t code, MessageParcel& data,
    MessageParcel& reply, MessageOption& option) override;
    void OnAudioParameterChange(const std::string networkId, const AudioParamKey key, const std::string& condition,
        const std::string& value) override;
    void OnCapturerState(bool isActive) override;
    void OnWakeupClose() override;

    // AudioManagerListenerStub
    void SetParameterCallback(const std::weak_ptr<AudioParameterCallback>& callback);
    void SetWakeupSourceCallback(const std::weak_ptr<WakeUpSourceCallback>& callback);
private:
    std::weak_ptr<AudioParameterCallback> callback_;
    std::weak_ptr<WakeUpSourceCallback> wakeUpCallback_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_MANAGER_LISTENER_STUB_H
