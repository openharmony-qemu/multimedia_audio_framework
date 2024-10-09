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
#ifndef LOG_TAG
#define LOG_TAG "AudioConcurrencyStateListenerProxy"
#endif

#include "audio_concurrency_state_listener_proxy.h"
#include "audio_policy_log.h"

namespace OHOS {
namespace AudioStandard {
AudioConcurrencyStateListenerProxy::AudioConcurrencyStateListenerProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IStandardConcurrencyStateListener>(impl)
{
    AUDIO_DEBUG_LOG("instances create");
}

AudioConcurrencyStateListenerProxy::~AudioConcurrencyStateListenerProxy()
{
    AUDIO_DEBUG_LOG("instance destroy");
}

void AudioConcurrencyStateListenerProxy::OnConcedeStream()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("AudioConcurrencyStateListenerProxy: WriteInterfaceToken failed");
        return;
    }

    int error = Remote()->SendRequest(ON_CONCEDE_STREAM, data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_ERR_LOG("OnConcedeStream failed, error: %{public}d", error);
    }
}

AudioConcurrencyListenerCallback::AudioConcurrencyListenerCallback(
    const sptr<IStandardConcurrencyStateListener> &listener) : listener_(listener)
{
    AUDIO_DEBUG_LOG("instance create");
}

AudioConcurrencyListenerCallback::~AudioConcurrencyListenerCallback()
{
    AUDIO_DEBUG_LOG("instance destroy");
}

void AudioConcurrencyListenerCallback::OnConcedeStream()
{
    if (listener_ != nullptr) {
        listener_->OnConcedeStream();
    }
}

} // namespace AudioStandard
} // namespace OHOS