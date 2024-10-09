/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef AUDIO_SPATIALIZATION_STATE_CHANGE_LISTENER_PROXY_H
#define AUDIO_SPATIALIZATION_STATE_CHANGE_LISTENER_PROXY_H

#include "audio_spatialization_manager.h"
#include "i_standard_spatialization_state_change_listener.h"

namespace OHOS {
namespace AudioStandard {
class AudioSpatializationStateChangeListenerProxy :
    public IRemoteProxy<IStandardSpatializationStateChangeListener> {
public:
    explicit AudioSpatializationStateChangeListenerProxy(const sptr<IRemoteObject> &impl);
    virtual ~AudioSpatializationStateChangeListenerProxy();
    DISALLOW_COPY_AND_MOVE(AudioSpatializationStateChangeListenerProxy);
    void OnSpatializationStateChange(const AudioSpatializationState &spatializationState) override;

private:
    static inline BrokerDelegator<AudioSpatializationStateChangeListenerProxy> delegator_;
};

class AudioSpatializationStateChangeListenerCallback : public AudioSpatializationStateChangeCallback {
public:
    AudioSpatializationStateChangeListenerCallback(const sptr<IStandardSpatializationStateChangeListener> &listener);
    virtual ~AudioSpatializationStateChangeListenerCallback();
    DISALLOW_COPY_AND_MOVE(AudioSpatializationStateChangeListenerCallback);
    void OnSpatializationStateChange(const AudioSpatializationState &spatializationState) override;
private:
    sptr<IStandardSpatializationStateChangeListener> listener_ = nullptr;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_SPATIALIZATION_STATE_CHANGE_LISTENER_PROXY_H
