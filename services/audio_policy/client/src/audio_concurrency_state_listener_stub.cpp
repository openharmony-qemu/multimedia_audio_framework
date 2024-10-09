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
#define LOG_TAG "AudioConcurrencyStateListenerStub"
#endif


#include "audio_policy_log.h"
#include "audio_concurrency_state_listener_stub.h"

namespace OHOS {
namespace AudioStandard {

AudioConcurrencyStateListenerStub::AudioConcurrencyStateListenerStub()
{
    AUDIO_DEBUG_LOG("concurrencyCbStub ctor");
}

AudioConcurrencyStateListenerStub::~AudioConcurrencyStateListenerStub()
{
    AUDIO_DEBUG_LOG("concurrencyCbStub dtor");
}

int AudioConcurrencyStateListenerStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    CHECK_AND_RETURN_RET_LOG(data.ReadInterfaceToken() == GetDescriptor(),
        AUDIO_INVALID_PARAM, "ReadInterfaceToken failed");
    switch (code) {
        case ON_CONCEDE_STREAM: {
            OnConcedeStream();
            return AUDIO_OK;
        }
        default: {
            AUDIO_ERR_LOG("default case, need check AudioListenerStub");
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
    }
}

void AudioConcurrencyStateListenerStub::OnConcedeStream()
{
    std::shared_ptr<AudioConcurrencyCallback> cb = callback_.lock();
    if (cb != nullptr) {
        cb->OnConcedeStream();
    } else {
        AUDIO_WARNING_LOG("callback is nullptr");
    }
}

void AudioConcurrencyStateListenerStub::SetConcurrencyCallback(const std::weak_ptr<AudioConcurrencyCallback> &callback)
{
    callback_ = callback;
}

} // namespace AudioStandard
} // namespace OHOS