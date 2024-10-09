/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioPolicyProxy"
#endif


#include "audio_policy_log.h"
#include "audio_policy_proxy.h"


namespace OHOS {
namespace AudioStandard {
using namespace std;

int32_t AudioPolicyProxy::SetAudioInterruptCallback(const uint32_t sessionID, const sptr<IRemoteObject> &object,
    uint32_t clientUid, const int32_t zoneID)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_NULL_OBJECT,
        "SetAudioInterruptCallback object is null");
    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");
    data.WriteUint32(sessionID);
    (void)data.WriteRemoteObject(object);
    data.WriteInt32(zoneID);
    data.WriteUint32(clientUid);
    int error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_CALLBACK), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error,
        "set callback failed, error: %{public}d", error);

    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::UnsetAudioInterruptCallback(const uint32_t sessionID,
    const int32_t zoneID)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");
    data.WriteUint32(sessionID);
    data.WriteInt32(zoneID);
    int error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::UNSET_CALLBACK), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error,
        "unset callback failed, error: %{public}d", error);

    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::SetAudioManagerInterruptCallback(const int32_t clientId, const sptr<IRemoteObject> &object)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_NULL_OBJECT,
        "SetAudioManagerInterruptCallback object is null");
    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");
    data.WriteInt32(clientId);
    (void)data.WriteRemoteObject(object);
    int error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_INTERRUPT_CALLBACK), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error,
        "set callback failed, error: %{public}d", error);

    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::UnsetAudioManagerInterruptCallback(const int32_t clientId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");

    data.WriteInt32(clientId);

    int error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::UNSET_INTERRUPT_CALLBACK), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error,
        "unset callback failed, error: %{public}d", error);

    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::SetQueryClientTypeCallback(const sptr<IRemoteObject> &object)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_NULL_OBJECT,
        "SetAudioInterruptCallback object is null");
    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");
    (void)data.WriteRemoteObject(object);
    int error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_QUERY_CLIENT_TYPE_CALLBACK), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error,
        "set callback failed, error: %{public}d", error);
    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::SetAvailableDeviceChangeCallback(const int32_t clientId, const AudioDeviceUsage usage,
    const sptr<IRemoteObject> &object)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_NULL_OBJECT, "SetAvailableDeviceChangeCallback object is null");

    bool token = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, ERROR, "data WriteInterfaceToken failed");
    token = data.WriteInt32(clientId) && data.WriteInt32(usage);
    CHECK_AND_RETURN_RET_LOG(token, ERROR, "data write failed");

    token = data.WriteRemoteObject(object);
    CHECK_AND_RETURN_RET_LOG(token, ERROR, "data WriteRemoteObject failed");

    int error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_AVAILABLE_DEVICE_CHANGE_CALLBACK), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error,
        "SetAvailableDeviceChangeCallback failed, error: %{public}d", error);

    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::UnsetAvailableDeviceChangeCallback(const int32_t clientId, AudioDeviceUsage usage)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, ERROR, "data WriteInterfaceToken failed");
    token = data.WriteInt32(clientId) && data.WriteInt32(usage);
    CHECK_AND_RETURN_RET_LOG(token, ERROR, "data write failed");

    int error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::UNSET_AVAILABLE_DEVICE_CHANGE_CALLBACK), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error,
        "UnsetAvailableDeviceChangeCallback failed, error: %{public}d", error);

    return reply.ReadInt32();
}


int32_t AudioPolicyProxy::SetAudioConcurrencyCallback(const uint32_t sessionID,
    const sptr<IRemoteObject> &object)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_NULL_OBJECT,
        "SetAudioConcurrencyCallback object is null");
    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");

    data.WriteUint32(sessionID);
    (void)data.WriteRemoteObject(object);
    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_AUDIO_CONCURRENCY_CALLBACK), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "SendRequest failed, error: %{public}d", error);

    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::UnsetAudioConcurrencyCallback(const uint32_t sessionID)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");
    data.WriteUint32(sessionID);
    int error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::UNSET_AUDIO_CONCURRENCY_CALLBACK), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error,
        "unset concurrency callback failed, error: %{public}d", error);

    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::SetDistributedRoutingRoleCallback(const sptr<IRemoteObject> &object)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_NULL_OBJECT, "object is null");
    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");
    (void)data.WriteRemoteObject(object);
    int error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_DISTRIBUTED_ROUTING_ROLE_CALLBACK), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "failed error : %{public}d", error);
    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::UnsetDistributedRoutingRoleCallback()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool token = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(token, ERROR, "data writeInterfaceToken failed");

    int error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::UNSET_DISTRIBUTED_ROUTING_ROLE_CALLBACK), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error,
        "AudioPolicyProxy UnsetDistributedRoutingRoleCallback failed error : %{public}d", error);
    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::SetClientCallbacksEnable(const CallbackChange &callbackchange, const bool &enable)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");
    data.WriteInt32(static_cast<int32_t>(callbackchange));
    data.WriteBool(enable);

    int error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_CALLBACKS_ENABLE), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "Set client callback failed, error: %{public}d", error);

    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::RegisterPolicyCallbackClient(const sptr<IRemoteObject> &object, const int32_t zoneID)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_NULL_OBJECT, "RegisterPolicyCallbackClient object is null");
    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, ERROR, "WriteInterfaceToken failed");

    data.WriteRemoteObject(object);
    data.WriteInt32(zoneID);
    int error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::REGISTER_POLICY_CALLBACK_CLIENT), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, ERROR, "SendRequest failed , error: %{public}d", error);
    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::SetAudioDeviceRefinerCallback(const sptr<IRemoteObject> &object)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_NULL_OBJECT, "object is null");

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");

    (void)data.WriteRemoteObject(object);
    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_AUDIO_DEVICE_REFINER_CALLBACK), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "SendRequest failed, error: %{public}d", error);
    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::UnsetAudioDeviceRefinerCallback()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");

    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::UNSET_AUDIO_DEVICE_REFINER_CALLBACK), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "SendRequest failed, error: %{public}d", error);
    return reply.ReadInt32();
}


int32_t AudioPolicyProxy::RegisterSpatializationStateEventListener(const uint32_t sessionID,
    const StreamUsage streamUsage, const sptr<IRemoteObject> &object)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, ERROR, "WriteInterfaceToken failed");
    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_NULL_OBJECT, "SpatializationStateEventListener object is null");

    data.WriteInt32(static_cast<int32_t>(sessionID));
    data.WriteInt32(static_cast<int32_t>(streamUsage));
    data.WriteRemoteObject(object);
    int32_t error = Remote() ->SendRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::REGISTER_SPATIALIZATION_STATE_EVENT), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, ERROR, "SendRequest failed , error: %{public}d", error);

    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::UnregisterSpatializationStateEventListener(const uint32_t sessionID)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, ERROR, "WriteInterfaceToken failed");

    data.WriteInt32(static_cast<int32_t>(sessionID));
    int32_t error = Remote() ->SendRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::UNREGISTER_SPATIALIZATION_STATE_EVENT), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, ERROR, "SendRequest failed , error: %{public}d", error);

    return reply.ReadInt32();
}

} // namespace AudioStandard
} // namespace OHOS