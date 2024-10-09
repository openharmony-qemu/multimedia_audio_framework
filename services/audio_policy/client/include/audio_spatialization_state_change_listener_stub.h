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

#ifndef AUDIO_SPATIALIZATION_STATE_CHANGE_LISTENER_STUB_H
#define AUDIO_SPATIALIZATION_STATE_CHANGE_LISTENER_STUB_H

#include "audio_spatialization_manager.h"
#include "i_standard_spatialization_state_change_listener.h"

namespace OHOS {
namespace AudioStandard {
class AudioSpatializationStateChangeListenerStub : public IRemoteStub<IStandardSpatializationStateChangeListener> {
public:
    AudioSpatializationStateChangeListenerStub();
    virtual ~AudioSpatializationStateChangeListenerStub();

    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
    void OnSpatializationStateChange(const AudioSpatializationState &spatializationState) override;
    void SetCallback(const std::weak_ptr<AudioSpatializationStateChangeCallback> &callback);
private:
    std::weak_ptr<AudioSpatializationStateChangeCallback> callback_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_SPATIALIZATION_STATE_CHANGE_LISTENER_STUB_H
