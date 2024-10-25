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
#define LOG_TAG "AudioAnahsManagerListenerStub"
#endif

#include "audio_anahs_manager_listener_stub.h"

#include "audio_errors.h"
#include "audio_policy_log.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {

AudioAnahsManagerListenerStub::AudioAnahsManagerListenerStub()
{
}

AudioAnahsManagerListenerStub::~AudioAnahsManagerListenerStub()
{
}

int AudioAnahsManagerListenerStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    CHECK_AND_RETURN_RET_LOG(data.ReadInterfaceToken() == GetDescriptor(),
        -1, "AudioRingerModeUpdateListenerStub: ReadInterfaceToken failed");
    switch (code) {
        case ON_AUDIO_ANAHS_DEVICE_CHANGE: {
            OnExtPnpDeviceStatusChangedInternal(data, reply);
            return AUDIO_OK;
        }
        default: {
            AUDIO_ERR_LOG("default case, need check AudioListenerStub");
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
    }
}

void AudioAnahsManagerListenerStub::SetAudioDeviceAnahsCallback(const std::weak_ptr<AudioDeviceAnahs> &callback)
{
    audioDeviceAnahsCallback_ = callback;
    std::shared_ptr<AudioDeviceAnahs> audioDeviceAnahsCallback = audioDeviceAnahsCallback_.lock();
    CHECK_AND_RETURN_LOG(audioDeviceAnahsCallback != nullptr, "audioDeviceAnahsCallback_ is nullptr");
}

void AudioAnahsManagerListenerStub::OnExtPnpDeviceStatusChangedInternal(MessageParcel &data, MessageParcel &reply)
{
    std::string anahsName = data.ReadString();
    std::string anahsShowType = data.ReadString();
    int32_t result = OnExtPnpDeviceStatusChanged(anahsName, anahsShowType);
    reply.WriteInt32(result);
}

int32_t AudioAnahsManagerListenerStub::OnExtPnpDeviceStatusChanged(std::string anahsStatus, std::string anahsShowType)
{
    std::shared_ptr<AudioDeviceAnahs> audioDeviceAnahsCallback = audioDeviceAnahsCallback_.lock();
    CHECK_AND_RETURN_RET_LOG(audioDeviceAnahsCallback != nullptr, ERR_CALLBACK_NOT_REGISTERED,
        "audioDeviceAnahsCallback_ is nullptr");
    return audioDeviceAnahsCallback->OnExtPnpDeviceStatusChanged(anahsStatus, anahsShowType);
}

} // namespace AudioStandard
} // namespace OHOS
