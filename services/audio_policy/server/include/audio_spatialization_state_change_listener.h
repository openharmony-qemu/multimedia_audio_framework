/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef AUDIO_STANDARD_SPATIALIZATION_STATE_CHANGE_LISTENER_H
#define AUDIO_STANDARD_SPATIALIZATION_STATE_CHANGE_LISTENER_H

#include "standard_spatialization_state_change_listener_stub.h"
#include "audio_spatialization_manager.h"
#include "audio_effect.h"

namespace OHOS {
namespace AudioStandard {

class AudioSpatializationStateChangeListener : public StandardSpatializationStateChangeListenerStub {
public:
    AudioSpatializationStateChangeListener();
    virtual ~AudioSpatializationStateChangeListener();

    int32_t OnSpatializationStateChange(const AudioSpatializationState &spatializationState) override;
    void SetCallback(const std::weak_ptr<AudioSpatializationStateChangeCallback> &callback);
private:
    std::weak_ptr<AudioSpatializationStateChangeCallback> callback_;
};
} // AudioStandard
} // OHOS

#endif // AUDIO_STANDARD_SPATIALIZATION_STATE_CHANGE_LISTENER_H