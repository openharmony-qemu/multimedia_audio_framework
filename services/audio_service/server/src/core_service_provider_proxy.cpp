/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "CoreServiceProviderProxy"
#endif

#include "core_service_provider_proxy.h"
#include "audio_service_log.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
CoreServiceProviderProxy::CoreServiceProviderProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<ICoreServiceProviderIpc>(impl)
{
}

CoreServiceProviderProxy::~CoreServiceProviderProxy()
{
}

int32_t CoreServiceProviderProxy::UpdateSessionOperation(uint32_t sessionId, SessionOperation operation,
    SessionOperationMsg opMsg)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");
    data.WriteUint32(sessionId);
    data.WriteUint32(operation);
    data.WriteUint32(opMsg);

    int ret = Remote()->SendRequest(ICoreServiceProviderMsg::UPDATE_SESSION_OPERATION, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ERR_OPERATION_FAILED, "failed, error: %{public}d", ret);

    return reply.ReadInt32();
}

int32_t CoreServiceProviderProxy::SetDefaultOutputDevice(const DeviceType defaultOutputDevice, const uint32_t sessionID,
    const StreamUsage streamUsage, bool isRunning)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");
    data.WriteInt32(defaultOutputDevice);
    data.WriteUint32(sessionID);
    data.WriteInt32(streamUsage);
    data.WriteBool(isRunning);
    int ret = Remote()->SendRequest(ICoreServiceProviderMsg::SET_DEFAULT_OUTPUT_DEVICE, data, reply, option);
    CHECK_AND_RETURN_RET(ret == AUDIO_OK, ret, "set default output device failed, ipc error: %{public}d", ret);
    return reply.ReadInt32();
}

std::string CoreServiceProviderProxy::GetAdapterNameBySessionId(uint32_t sessionId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), "", "Write descriptor failed!");
    data.WriteUint32(sessionId);
    int ret = Remote()->SendRequest(ICoreServiceProviderMsg::GET_ADAPTER_NAME_BY_SESSION_ID, data, reply, option);
    CHECK_AND_RETURN_RET(ret == AUDIO_OK, "", "set default output device failed, ipc error: %{public}d", ret);
    return reply.ReadString();
}

int32_t CoreServiceProviderProxy::GetProcessDeviceInfoBySessionId(uint32_t sessionId, AudioDeviceDescriptor &deviceInfo)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");
    data.WriteUint32(sessionId);
    int ret = Remote()->SendRequest(
        ICoreServiceProviderMsg::GET_PROCESS_DEVICE_INFO_BY_SESSION_ID, data, reply, option);
    CHECK_AND_RETURN_RET(ret == AUDIO_OK, ret, "set default output device failed, ipc error: %{public}d", ret);
    deviceInfo.Unmarshalling(reply);
    return SUCCESS;
}

uint32_t CoreServiceProviderProxy::GenerateSessionId()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");
    int ret = Remote()->SendRequest(ICoreServiceProviderMsg::GENERATE_SESSION_ID, data, reply, option);
    CHECK_AND_RETURN_RET(ret == AUDIO_OK, 0, "set default output device failed, ipc error: %{public}d", ret);
    return reply.ReadUint32();
}
} // namespace AudioStandard
} // namespace OHOS
