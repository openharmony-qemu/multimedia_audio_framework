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


#include "audio_spatialization_state_change_listener.h"
#include "audio_policy_log.h"

namespace OHOS {
namespace AudioStandard {

AudioSpatializationStateChangeListener::AudioSpatializationStateChangeListener()
{
    AUDIO_DEBUG_LOG("Instance create");
}

AudioSpatializationStateChangeListener::~AudioSpatializationStateChangeListener()
{
    AUDIO_DEBUG_LOG("Instance destroy");
}

int32_t AudioSpatializationStateChangeListener::OnSpatializationStateChange(
    const AudioSpatializationState &spatializationState)
{
    AUDIO_DEBUG_LOG("OnSpatializationStateChange");

    std::shared_ptr<AudioSpatializationStateChangeCallback> cb = callback_.lock();
    if (cb == nullptr) {
        AUDIO_ERR_LOG("callback_ is nullptr");
        return AUDIO_ERR;
    }

    cb->OnSpatializationStateChange(spatializationState);
    return AUDIO_OK;
}

void AudioSpatializationStateChangeListener::SetCallback(
    const std::weak_ptr<AudioSpatializationStateChangeCallback> &callback)
{
    AUDIO_DEBUG_LOG("SetCallback");
    callback_ = callback;
}

} // AudioStandard
} // OHOS
