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

#ifndef AUDIO_CONCURRENCY_STATE_LISTENER_STUB_H
#define AUDIO_CONCURRENCY_STATE_LISTENER_STUB_H

#include "audio_concurrency_callback.h"
#include "i_standard_concurrency_state_listener.h"

namespace OHOS {
namespace AudioStandard {
class AudioConcurrencyStateListenerStub : public IRemoteStub<IStandardConcurrencyStateListener> {
public:
    AudioConcurrencyStateListenerStub();
    virtual ~AudioConcurrencyStateListenerStub();

    // IStandardAudioManagerListener override
    int OnRemoteRequest(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option) override;
    void OnConcedeStream() override;
    void SetConcurrencyCallback(const std::weak_ptr<AudioConcurrencyCallback> &callback);
private:
    std::weak_ptr<AudioConcurrencyCallback> callback_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_POLICY_MANAGER_LISTENER_STUB_H
