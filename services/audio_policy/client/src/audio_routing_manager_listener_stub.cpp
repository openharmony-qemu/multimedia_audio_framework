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
#define LOG_TAG "AudioRoutingManagerListenerStub"
#endif

#include "audio_routing_manager_listener_stub.h"

#include "audio_errors.h"
#include "audio_policy_log.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {

static const int32_t PREFERRED_DEVICE_VALID_SIZE = 128;

AudioRoutingManagerListenerStub::AudioRoutingManagerListenerStub()
{
}

AudioRoutingManagerListenerStub::~AudioRoutingManagerListenerStub()
{
}

int AudioRoutingManagerListenerStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    CHECK_AND_RETURN_RET_LOG(data.ReadInterfaceToken() == GetDescriptor(),
        -1, "AudioRingerModeUpdateListenerStub: ReadInterfaceToken failed");
    switch (code) {
        case ON_DISTRIBUTED_ROUTING_ROLE_CHANGE: {
            sptr<AudioDeviceDescriptor> descriptor = AudioDeviceDescriptor::Unmarshalling(data);
            CastType type = static_cast<CastType>(data.ReadInt32());
            OnDistributedRoutingRoleChange(descriptor, type);
            return AUDIO_OK;
        }
        case ON_AUDIO_OUTPUT_DEVICE_REFINERD: {
            OnAudioOutputDeviceRefinedInternal(data, reply);
            return AUDIO_OK;
        }
        case ON_AUDIO_INPUT_DEVICE_REFINERD: {
            OnAudioInputDeviceRefinedInternal(data, reply);
            return AUDIO_OK;
        }
        default: {
            AUDIO_ERR_LOG("default case, need check AudioListenerStub");
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
    }
}

void AudioRoutingManagerListenerStub::OnDistributedRoutingRoleChange(const sptr<AudioDeviceDescriptor> descriptor,
    const CastType type)
{
    std::shared_ptr<AudioDistributedRoutingRoleCallback> audioDistributedRoutingRoleCallback =
        audioDistributedRoutingRoleCallback_.lock();

    CHECK_AND_RETURN_LOG(audioDistributedRoutingRoleCallback != nullptr,
        "OnDistributedRoutingRoleChange: audioDistributedRoutingRoleCallback_ is nullptr");

    audioDistributedRoutingRoleCallback->OnDistributedRoutingRoleChange(descriptor, type);
}

void AudioRoutingManagerListenerStub::SetDistributedRoutingRoleCallback(
    const std::weak_ptr<AudioDistributedRoutingRoleCallback> &callback)
{
    audioDistributedRoutingRoleCallback_ = callback;
}

void AudioRoutingManagerListenerStub::SetAudioDeviceRefinerCallback(const std::weak_ptr<AudioDeviceRefiner> &callback)
{
    audioDeviceRefinerCallback_ = callback;
    std::shared_ptr<AudioDeviceRefiner> audioDeviceRefinerCallback = audioDeviceRefinerCallback_.lock();
    CHECK_AND_RETURN_LOG(audioDeviceRefinerCallback != nullptr, "audioDeviceRefinerCallback_ is nullptr");
}

void AudioRoutingManagerListenerStub::OnAudioOutputDeviceRefinedInternal(MessageParcel &data, MessageParcel &reply)
{
    std::vector<std::unique_ptr<AudioDeviceDescriptor>> descs;
    int32_t size = data.ReadInt32();
    CHECK_AND_RETURN_LOG(size < PREFERRED_DEVICE_VALID_SIZE, "get invalid size : %{public}d", size);
    for (int32_t i = 0; i < size; i++) {
        descs.push_back(make_unique<AudioDeviceDescriptor>(AudioDeviceDescriptor::Unmarshalling(data)));
    }
    RouterType routerType = static_cast<RouterType>(data.ReadInt32());
    StreamUsage streamUsage = static_cast<StreamUsage>(data.ReadInt32());
    int32_t clientUid = data.ReadInt32();
    AudioPipeType audioPipeType = static_cast<AudioPipeType>(data.ReadInt32());

    int32_t result = OnAudioOutputDeviceRefined(descs, routerType, streamUsage, clientUid, audioPipeType);
    if (result == SUCCESS) {
        reply.WriteInt32(result);
        reply.WriteInt32(descs.size());
        for (auto &desc : descs) {
            desc->Marshalling(reply);
        }
    } else {
        reply.WriteInt32(result);
    }
}

void AudioRoutingManagerListenerStub::OnAudioInputDeviceRefinedInternal(MessageParcel &data, MessageParcel &reply)
{
    std::vector<std::unique_ptr<AudioDeviceDescriptor>> descs;
    int32_t size = data.ReadInt32();
    CHECK_AND_RETURN_LOG(size < PREFERRED_DEVICE_VALID_SIZE, "get invalid size : %{public}d", size);
    for (int32_t i = 0; i < size; i++) {
        descs.push_back(make_unique<AudioDeviceDescriptor>(AudioDeviceDescriptor::Unmarshalling(data)));
    }
    RouterType routerType = static_cast<RouterType>(data.ReadInt32());
    SourceType sourceType = static_cast<SourceType>(data.ReadInt32());
    int32_t clientUid = data.ReadInt32();
    AudioPipeType audioPipeType = static_cast<AudioPipeType>(data.ReadInt32());

    int32_t result = OnAudioInputDeviceRefined(descs, routerType, sourceType, clientUid, audioPipeType);
    if (result == SUCCESS) {
        reply.WriteInt32(result);
        reply.WriteInt32(descs.size());
        for (auto &desc : descs) {
            desc->Marshalling(reply);
        }
    } else {
        reply.WriteInt32(result);
    }
}

int32_t AudioRoutingManagerListenerStub::OnAudioOutputDeviceRefined(
    std::vector<std::unique_ptr<AudioDeviceDescriptor>> &descs, RouterType routerType, StreamUsage streamUsage,
    int32_t clientUid, AudioPipeType audioPipeType)
{
    std::shared_ptr<AudioDeviceRefiner> audioDeviceRefinerCallback = audioDeviceRefinerCallback_.lock();
    CHECK_AND_RETURN_RET_LOG(audioDeviceRefinerCallback != nullptr,
        ERR_CALLBACK_NOT_REGISTERED, "audioDeviceRefinerCallback_ is nullptr");

    return audioDeviceRefinerCallback->OnAudioOutputDeviceRefined(descs, routerType, streamUsage, clientUid,
        audioPipeType);
}

int32_t AudioRoutingManagerListenerStub::OnAudioInputDeviceRefined(
    std::vector<std::unique_ptr<AudioDeviceDescriptor>> &descs, RouterType routerType, SourceType sourceType,
    int32_t clientUid, AudioPipeType audioPipeType)
{
    std::shared_ptr<AudioDeviceRefiner> audioDeviceRefinerCallback = audioDeviceRefinerCallback_.lock();
    CHECK_AND_RETURN_RET_LOG(audioDeviceRefinerCallback != nullptr, ERR_CALLBACK_NOT_REGISTERED,
        "audioDeviceRefinerCallback_ is nullptr");

    return audioDeviceRefinerCallback->OnAudioInputDeviceRefined(descs, routerType,
        sourceType, clientUid, audioPipeType);
}

} // namespace AudioStandard
} // namespace OHOS
