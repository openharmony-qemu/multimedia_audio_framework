/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioPolicyManagerListenerStub"
#endif


#include "audio_policy_log.h"
#include "audio_policy_manager_listener_stub.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {

static const int32_t DEVICE_CHANGE_VALID_SIZE = 128;

AudioPolicyManagerListenerStub::AudioPolicyManagerListenerStub()
{
}

AudioPolicyManagerListenerStub::~AudioPolicyManagerListenerStub()
{
}

void AudioPolicyManagerListenerStub::ReadInterruptEventParams(MessageParcel &data,
                                                              InterruptEventInternal &interruptEvent)
{
    interruptEvent.eventType = static_cast<InterruptType>(data.ReadInt32());
    interruptEvent.forceType = static_cast<InterruptForceType>(data.ReadInt32());
    interruptEvent.hintType = static_cast<InterruptHint>(data.ReadInt32());
    interruptEvent.duckVolume = data.ReadFloat();
}

void AudioPolicyManagerListenerStub::ReadAudioDeviceChangeData(MessageParcel &data, DeviceChangeAction &devChange)
{
    std::vector<sptr<AudioDeviceDescriptor>> deviceChangeDesc = {};

    int32_t type = data.ReadInt32();
    int32_t flag = data.ReadInt32();
    int32_t size = data.ReadInt32();
    CHECK_AND_RETURN_LOG(size < DEVICE_CHANGE_VALID_SIZE, "get invalid size : %{public}d", size);

    for (int32_t i = 0; i < size; i++) {
        deviceChangeDesc.push_back(AudioDeviceDescriptor::Unmarshalling(data));
    }

    devChange.type = static_cast<DeviceChangeType>(type);
    devChange.flag = static_cast<DeviceFlag>(flag);
    devChange.deviceDescriptors = deviceChangeDesc;
}

int AudioPolicyManagerListenerStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    CHECK_AND_RETURN_RET_LOG(data.ReadInterfaceToken() == GetDescriptor(), AUDIO_INVALID_PARAM,
        "ReadInterfaceToken failed");
    Trace trace("AudioPolicyManagerListenerStub::OnRemoteRequest:" + std::to_string(code));
    switch (code) {
        case ON_INTERRUPT: {
            InterruptEventInternal interruptEvent = {};
            ReadInterruptEventParams(data, interruptEvent);
            // To be modified by enqueuing the interrupt action scheduler
            OnInterrupt(interruptEvent);
            return AUDIO_OK;
        }
        case ON_AVAILABLE_DEVICE_CAHNGE: {
            AudioDeviceUsage usage = static_cast<AudioDeviceUsage>(data.ReadInt32());
            DeviceChangeAction deviceChangeAction = {};
            ReadAudioDeviceChangeData(data, deviceChangeAction);
            OnAvailableDeviceChange(usage, deviceChangeAction);
            return AUDIO_OK;
        }
        case ON_QUERY_CLIENT_TYPE: {
            std::string bundleName = data.ReadString();
            uint32_t uid = data.ReadUint32();
            OnQueryClientType(bundleName, uid);
            return AUDIO_OK;
        }
        default: {
            AUDIO_ERR_LOG("default case, need check AudioListenerStub");
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
    }
}

void AudioPolicyManagerListenerStub::OnInterrupt(const InterruptEventInternal &interruptEvent)
{
    std::shared_ptr<AudioInterruptCallback> cb = callback_.lock();
    if (cb != nullptr) {
        cb->OnInterrupt(interruptEvent);
    } else {
        AUDIO_WARNING_LOG("AudioPolicyManagerListenerStub: callback_ is nullptr");
    }
}

void AudioPolicyManagerListenerStub::OnAvailableDeviceChange(const AudioDeviceUsage usage,
    const DeviceChangeAction &deviceChangeAction)
{
    std::shared_ptr<AudioManagerAvailableDeviceChangeCallback> availabledeviceChangedCallback =
        audioAvailableDeviceChangeCallback_.lock();

    CHECK_AND_RETURN_LOG(availabledeviceChangedCallback != nullptr,
        "OnAvailableDeviceChange: deviceChangeCallback_ or deviceChangeAction is nullptr");

    availabledeviceChangedCallback->OnAvailableDeviceChange(usage, deviceChangeAction);
}

bool AudioPolicyManagerListenerStub::OnQueryClientType(const std::string &bundleName, uint32_t uid)
{
    std::shared_ptr<AudioQueryClientTypeCallback> audioQueryClientTypeCallback =
        audioQueryClientTypeCallback_.lock();

    CHECK_AND_RETURN_RET_LOG(audioQueryClientTypeCallback != nullptr, false,
        "audioQueryClientTypeCallback_ is nullptr");

    return audioQueryClientTypeCallback->OnQueryClientType(bundleName, uid);
}

void AudioPolicyManagerListenerStub::SetInterruptCallback(const std::weak_ptr<AudioInterruptCallback> &callback)
{
    callback_ = callback;
}

void AudioPolicyManagerListenerStub::SetAvailableDeviceChangeCallback(
    const std::weak_ptr<AudioManagerAvailableDeviceChangeCallback> &cb)
{
    audioAvailableDeviceChangeCallback_ = cb;
}

void AudioPolicyManagerListenerStub::SetQueryClientTypeCallback(const std::weak_ptr<AudioQueryClientTypeCallback> &cb)
{
    audioQueryClientTypeCallback_ = cb;
}
} // namespace AudioStandard
} // namespace OHOS
