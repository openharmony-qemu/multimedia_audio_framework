/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef AUDIO_MANAGER_LISTENER_PROXY_H
#define AUDIO_MANAGER_LISTENER_PROXY_H

#include "iremote_proxy.h"
#include "audio_system_manager.h"
#include "i_standard_audio_server_manager_listener.h"

namespace OHOS {
namespace AudioStandard {
class AudioManagerListenerProxy : public IRemoteProxy<IStandardAudioServerManagerListener> {
public:
    explicit AudioManagerListenerProxy(const sptr<IRemoteObject>& impl);
    virtual ~AudioManagerListenerProxy();
    DISALLOW_COPY_AND_MOVE(AudioManagerListenerProxy);
    void OnAudioParameterChange(const std::string networkId, const AudioParamKey key,
        const std::string& condition, const std::string& value) override;
    void OnCapturerState(bool isActive) override;
    void OnWakeupClose() override;

private:
    static inline BrokerDelegator<AudioManagerListenerProxy> delegator_;
    void WriteParameterEventParams(MessageParcel& data, const std::string networkId, const AudioParamKey key,
        const std::string& condition, const std::string& value);
};

class AudioManagerListenerCallback : public AudioParameterCallback, public WakeUpSourceCallback {
public:
    AudioManagerListenerCallback(const sptr<IStandardAudioServerManagerListener>& listener);
    virtual ~AudioManagerListenerCallback();
    DISALLOW_COPY_AND_MOVE(AudioManagerListenerCallback);
    void OnAudioParameterChange(const std::string networkId, const AudioParamKey key,
        const std::string& condition, const std::string& value) override;
    void OnCapturerState(bool isActive) override final;
    void OnWakeupClose() override;
    void TrigerFirstOnCapturerStateCallback(bool isActive);
private:
    sptr<IStandardAudioServerManagerListener> listener_ = nullptr;
    std::atomic<bool> isFirstOnCapturerStateCallbackSent_ = false;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_MANAGER_LISTENER_PROXY_H
