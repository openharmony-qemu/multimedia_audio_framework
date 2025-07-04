/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioWorkgroupCallbackStub"
#endif
 
#include "audio_workgroup_callback_stub.h"
#include "audio_common_log.h"
 
namespace OHOS {
namespace AudioStandard {
 
AudioWorkgroupCallbackStub::AudioWorkgroupCallbackStub()
{
    AUDIO_DEBUG_LOG("AudioWorkgroupCallbackStub ctor");
}
 
AudioWorkgroupCallbackStub::~AudioWorkgroupCallbackStub()
{
    AUDIO_DEBUG_LOG("~AudioWorkgroupCallbackStub dtor");
}

int AudioWorkgroupCallbackStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    CHECK_AND_RETURN_RET_LOG(data.ReadInterfaceToken() == GetDescriptor(),
        -1, "AudioManagerStub: ReadInterfaceToken failed");

    switch (code) {
        case ON_WORKGROUP_CHANGE: {
            AudioWorkgroupChangeInfo info = {
                .pid = data.ReadInt32(),
                .groupId = data.ReadInt32(),
                .startAllowed = data.ReadBool(),
            };
            OnWorkgroupChange(info);
            return AUDIO_OK;
        }
        default: {
            return IRemoteStub<IAudioWorkgroupCallback>::OnRemoteRequest(code, data, reply, option);
        }
    }
}

void AudioWorkgroupCallbackStub::AddWorkgroupChangeCallback(
    std::shared_ptr<AudioWorkgroupChangeCallback> cb)
{
    workgroupCb_ = cb;
}

void AudioWorkgroupCallbackStub::RemoveWorkgroupChangeCallback()
{
    workgroupCb_ = nullptr;
}

void AudioWorkgroupCallbackStub::OnWorkgroupChange(
    const AudioWorkgroupChangeInfo &info)
{
    if (workgroupCb_ == nullptr) {
        return;
    }
    workgroupCb_->OnWorkgroupChange(info);
}

} // namespace AudioStandard
} // namespace OHOS