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
#ifndef LOG_TAG
#define LOG_TAG "AudioManagerListenerStub"
#endif

#include "audio_manager_listener_stub.h"
#include "audio_service_log.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {
AudioManagerListenerStub::AudioManagerListenerStub()
{
}
AudioManagerListenerStub::~AudioManagerListenerStub()
{
}
int AudioManagerListenerStub::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    CHECK_AND_RETURN_RET_LOG(data.ReadInterfaceToken() == GetDescriptor(),
        -1, "AudioManagerStub: ReadInterfaceToken failed");

    switch (code) {
        case ON_PARAMETER_CHANGED: {
            string networkId = data.ReadString();
            AudioParamKey key = static_cast<AudioParamKey>(data.ReadInt32());
            string condition = data.ReadString();
            string value = data.ReadString();
            OnAudioParameterChange(networkId, key, condition, value);
            return AUDIO_OK;
        }
        case ON_WAKEUP_CLOSE: {
            OnWakeupClose();
            return AUDIO_OK;
        }
        case ON_CAPTURER_STATE: {
            bool isActive = data.ReadBool();
            OnCapturerState(isActive);
            return AUDIO_OK;
        }
        default: {
            AUDIO_ERR_LOG("default case, need check AudioManagerStub");
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
    }
}

void AudioManagerListenerStub::SetParameterCallback(const std::weak_ptr<AudioParameterCallback>& callback)
{
    callback_ = callback;
}

void AudioManagerListenerStub::SetWakeupSourceCallback(const std::weak_ptr<WakeUpSourceCallback>& callback)
{
    wakeUpCallback_ = callback;
}

void AudioManagerListenerStub::OnAudioParameterChange(const std::string networkId, const AudioParamKey key,
    const std::string& condition, const std::string& value)
{
    std::shared_ptr<AudioParameterCallback> cb = callback_.lock();
    if (cb != nullptr) {
        cb->OnAudioParameterChange(networkId, key, condition, value);
    } else {
        AUDIO_WARNING_LOG("AudioRingerModeUpdateListenerStub: callback_ is nullptr");
    }
}

void AudioManagerListenerStub::OnCapturerState(bool isActive)
{
    std::shared_ptr<WakeUpSourceCallback> cb = wakeUpCallback_.lock();
    if (cb != nullptr) {
        cb->OnCapturerState(isActive);
    } else {
        AUDIO_WARNING_LOG("AudioManagerListenerStub: OnWakeupClose error");
    }
}

void AudioManagerListenerStub::OnWakeupClose()
{
    std::shared_ptr<WakeUpSourceCallback> cb = wakeUpCallback_.lock();
    if (cb != nullptr) {
        cb->OnWakeupClose();
    } else {
        AUDIO_WARNING_LOG("AudioManagerListenerStub: OnWakeupClose error");
    }
}

} // namespace AudioStandard
} // namespace OHOS
