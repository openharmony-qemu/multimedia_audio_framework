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
#ifndef LOG_TAG
#define LOG_TAG "AudioSpatializationStateChangeListenerStub"
#endif

#include "audio_spatialization_state_change_listener_stub.h"
#include "audio_policy_log.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {
AudioSpatializationStateChangeListenerStub::AudioSpatializationStateChangeListenerStub()
{
    AUDIO_DEBUG_LOG("AudioSpatializationStateChangeListenerStub Instance create");
}

AudioSpatializationStateChangeListenerStub::~AudioSpatializationStateChangeListenerStub()
{
    AUDIO_DEBUG_LOG("AudioSpatializationStateChangeListenerStub Instance destroy");
}

int AudioSpatializationStateChangeListenerStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        AUDIO_ERR_LOG("AudioSpatializationStateChangeListenerStub: ReadInterfaceToken failed");
        return -1;
    }

    switch (code) {
        case ON_SPATIALIZATION_STATE_CHANGE: {
            AudioSpatializationState spatializationState;
            spatializationState.spatializationEnabled = data.ReadBool();
            spatializationState.headTrackingEnabled = data.ReadBool();
            OnSpatializationStateChange(spatializationState);
            return AUDIO_OK;
        }
        default: {
            AUDIO_ERR_LOG("default case, need check AudioListenerStub");
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
    }
}

void AudioSpatializationStateChangeListenerStub::OnSpatializationStateChange(
    const AudioSpatializationState &spatializationState)
{
    AUDIO_DEBUG_LOG("AudioSpatializationStateChangeListenerStub OnSpatializationStateChange");

    shared_ptr<AudioSpatializationStateChangeCallback> cb = callback_.lock();
    if (cb == nullptr) {
        AUDIO_ERR_LOG("AudioSpatializationStateChangeListenerStub: callback_ is nullptr");
        return;
    }

    cb->OnSpatializationStateChange(spatializationState);
    return;
}

void AudioSpatializationStateChangeListenerStub::SetCallback(
    const weak_ptr<AudioSpatializationStateChangeCallback> &callback)
{
    AUDIO_DEBUG_LOG("AudioSpatializationStateChangeListenerStub SetCallback");
    callback_ = callback;
}
} // namespace AudioStandard
} // namespace OHOS
