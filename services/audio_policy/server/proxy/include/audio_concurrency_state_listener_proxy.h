/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef AUDIO_CONCURRENCY_STATE_LISTENER_PROXY_H
#define AUDIO_CONCURRENCY_STATE_LISTENER_PROXY_H

#include "audio_concurrency_callback.h"
#include "audio_system_manager.h"
#include "i_standard_concurrency_state_listener.h"

namespace OHOS {
namespace AudioStandard {
class AudioConcurrencyStateListenerProxy : public IRemoteProxy<IStandardConcurrencyStateListener> {
public:
    explicit AudioConcurrencyStateListenerProxy(const sptr<IRemoteObject> &impl);
    virtual ~AudioConcurrencyStateListenerProxy();
    DISALLOW_COPY_AND_MOVE(AudioConcurrencyStateListenerProxy);
    void OnConcedeStream() override;

private:
    static inline BrokerDelegator<AudioConcurrencyStateListenerProxy> delegator_;
};

class AudioConcurrencyListenerCallback : public AudioConcurrencyCallback {
public:
    AudioConcurrencyListenerCallback(const sptr<IStandardConcurrencyStateListener> &listener);
    virtual ~AudioConcurrencyListenerCallback();
    DISALLOW_COPY_AND_MOVE(AudioConcurrencyListenerCallback);
    void OnConcedeStream() override;
private:
    sptr<IStandardConcurrencyStateListener> listener_ = nullptr;
};
} // namespace AudioStandard
} // namespace OHOS
#endif