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
#define LOG_TAG "AudioManagerListenerProxy"
#endif

#include "audio_manager_listener_proxy.h"
#include "audio_system_manager.h"
#include "audio_service_log.h"

namespace OHOS {
namespace AudioStandard {
AudioManagerListenerProxy::AudioManagerListenerProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IStandardAudioServerManagerListener>(impl)
{
}

AudioManagerListenerProxy::~AudioManagerListenerProxy()
{
}

void AudioManagerListenerProxy::WriteParameterEventParams(MessageParcel& data, const std::string networkId,
    const AudioParamKey key, const std::string& condition, const std::string& value)
{
    data.WriteString(static_cast<std::string>(networkId));
    data.WriteInt32(static_cast<std::int32_t>(key));
    data.WriteString(static_cast<std::string>(condition));
    data.WriteString(static_cast<std::string>(value));
}

void AudioManagerListenerProxy::OnAudioParameterChange(const std::string networkId, const AudioParamKey key,
    const std::string& condition, const std::string& value)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_LOG(ret, "WriteInterfaceToken failed");

    data.WriteString(static_cast<std::string>(networkId));
    data.WriteInt32(static_cast<std::int32_t>(key));
    data.WriteString(static_cast<std::string>(condition));
    data.WriteString(static_cast<std::string>(value));

    int error = Remote()->SendRequest(ON_PARAMETER_CHANGED, data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_ERR_LOG("ON_PARAMETER_CHANGED failed, error: %{public}d", error);
    }
}

void AudioManagerListenerProxy::OnCapturerState(bool isActive)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_LOG(ret, "WriteInterfaceToken failed");

    data.WriteBool(isActive);

    int error = Remote()->SendRequest(ON_CAPTURER_STATE, data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_ERR_LOG("ON_CAPTURER_STATE failed, error: %{public}d", error);
    }
}

void AudioManagerListenerProxy::OnWakeupClose()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_LOG(ret, "WriteInterfaceToken failed");

    int error = Remote()->SendRequest(ON_WAKEUP_CLOSE, data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_ERR_LOG("ON_WAKEUP_CLOSE failed, error: %{public}d", error);
    }
}

AudioManagerListenerCallback::AudioManagerListenerCallback(const sptr<IStandardAudioServerManagerListener>& listener)
    : listener_(listener)
{
}

AudioManagerListenerCallback::~AudioManagerListenerCallback()
{
}

void AudioManagerListenerCallback::OnAudioParameterChange(const std::string networkId, const AudioParamKey key,
    const std::string& condition, const std::string& value)
{
    if (listener_ != nullptr) {
        listener_->OnAudioParameterChange(networkId, key, condition, value);
    }
}

void AudioManagerListenerCallback::OnCapturerState(bool isActive)
{
    if (listener_ != nullptr) {
        isFirstOnCapturerStateCallbackSent_ = true;
        listener_->OnCapturerState(isActive);
    }
}

void AudioManagerListenerCallback::OnWakeupClose()
{
    if (listener_ != nullptr) {
        listener_->OnWakeupClose();
    }
}

void AudioManagerListenerCallback::TrigerFirstOnCapturerStateCallback(bool isActive)
{
    if (!isFirstOnCapturerStateCallbackSent_.exchange(true)) {
        OnCapturerState(isActive);
    }
}
} // namespace AudioStandard
} // namespace OHOS
