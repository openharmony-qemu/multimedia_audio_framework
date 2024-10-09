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
#define LOG_TAG "AudioRoutingManagerListenerProxy"
#endif

#include "audio_routing_manager_listener_proxy.h"
#include "audio_policy_log.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
const int32_t DEVICE_SIZE_LIMIT = 100;
AudioRoutingManagerListenerProxy::AudioRoutingManagerListenerProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IStandardAudioRoutingManagerListener>(impl)
{
    AUDIO_DEBUG_LOG("Instances create");
}

AudioRoutingManagerListenerProxy::~AudioRoutingManagerListenerProxy()
{
    AUDIO_DEBUG_LOG("~AudioRoutingManagerListenerProxy: Instance destroy");
}

void AudioRoutingManagerListenerProxy::OnDistributedRoutingRoleChange(const sptr<AudioDeviceDescriptor> desciptor,
    const CastType type)
{
    AUDIO_DEBUG_LOG("AudioRoutingManagerListenerProxy: OnDistributedRoutingRoleChange as listener proxy");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    CHECK_AND_RETURN_LOG(data.WriteInterfaceToken(GetDescriptor()),
        "OnDistributedRoutingRoleChange: WriteInterfaceToken failed");

    desciptor->Marshalling(data);
    data.WriteInt32(type);

    int error = Remote()->SendRequest(ON_DISTRIBUTED_ROUTING_ROLE_CHANGE, data, reply, option);
    CHECK_AND_RETURN_LOG(error == ERR_NONE, "OnDistributedRoutingRoleChangefailed, error: %{public}d", error);
}

int32_t AudioRoutingManagerListenerProxy::OnAudioOutputDeviceRefined(
    std::vector<std::unique_ptr<AudioDeviceDescriptor>> &descs, RouterType routerType, StreamUsage streamUsage,
    int32_t clientUid, AudioPipeType audioPipeType)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR,
        "OnAudioOutputDeviceRefined: WriteInterfaceToken failed");
    
    data.WriteInt32(descs.size());
    for (auto &desc : descs) {
        desc->Marshalling(data);
    }
    data.WriteInt32(routerType);
    data.WriteInt32(streamUsage);
    data.WriteInt32(clientUid);
    data.WriteInt32(audioPipeType);

    int error = Remote()->SendRequest(ON_AUDIO_OUTPUT_DEVICE_REFINERD, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "OnAudioOutputDeviceRefined, error: %{public}d", error);

    int32_t result = reply.ReadInt32();
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, result,
        "OnAudioOutputDeviceRefined callback failed, error %{public}d", result);

    descs.clear();
    int32_t size = reply.ReadInt32();
    CHECK_AND_RETURN_RET_LOG(size < DEVICE_SIZE_LIMIT, ERROR, "reply size reach limit");
    for (int32_t i = 0; i < size; i++) {
        descs.push_back(std::make_unique<AudioDeviceDescriptor>(AudioDeviceDescriptor::Unmarshalling(reply)));
    }
    return SUCCESS;
}

int32_t AudioRoutingManagerListenerProxy::OnAudioInputDeviceRefined(
    std::vector<std::unique_ptr<AudioDeviceDescriptor>> &descs, RouterType routerType, SourceType sourceType,
    int32_t clientUid, AudioPipeType audioPipeType)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR,
        "OnAudioInputDeviceRefined: WriteInterfaceToken failed");
    
    data.WriteInt32(descs.size());
    for (auto &desc : descs) {
        desc->Marshalling(data);
    }
    data.WriteInt32(routerType);
    data.WriteInt32(sourceType);
    data.WriteInt32(clientUid);
    data.WriteInt32(audioPipeType);

    int error = Remote()->SendRequest(ON_AUDIO_INPUT_DEVICE_REFINERD, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "OnAudioInputDeviceRefined, error: %{public}d", error);

    int32_t result = reply.ReadInt32();
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, result,
        "OnAudioInputDeviceRefined callback failed, error %{public}d", result);

    descs.clear();
    int32_t size = reply.ReadInt32();
    CHECK_AND_RETURN_RET_LOG(size < DEVICE_SIZE_LIMIT, ERROR, "reply size reach limit");
    for (int32_t i = 0; i < size; i++) {
        descs.push_back(std::make_unique<AudioDeviceDescriptor>(AudioDeviceDescriptor::Unmarshalling(reply)));
    }
    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS
