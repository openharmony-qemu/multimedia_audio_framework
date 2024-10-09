/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "IpcStreamListenerStub"
#endif

#include "ipc_stream_listener_stub.h"
#include "audio_service_log.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
bool IpcStreamListenerStub::CheckInterfaceToken(MessageParcel &data)
{
    static auto localDescriptor = IpcStreamListener::GetDescriptor();
    auto remoteDescriptor = data.ReadInterfaceToken();
    if (remoteDescriptor != localDescriptor) {
        AUDIO_ERR_LOG("CheckInterFfaceToken failed.");
        return false;
    }
    return true;
}

int IpcStreamListenerStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    if (!CheckInterfaceToken(data)) {
        return AUDIO_ERR;
    }
    if (code >= IpcStreamListenerMsg::IPC_STREAM_LISTENER_MAX_MSG) {
        AUDIO_WARNING_LOG("OnRemoteRequest unsupported request code:%{public}d.", code);
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    switch (code) {
        case ON_OPERATION_HANDLED:
            return HandleOnOperationHandled(data, reply);
        default:
            AUDIO_WARNING_LOG("OnRemoteRequest unsupported request code:%{public}d.", code);
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
}

int32_t IpcStreamListenerStub::HandleOnOperationHandled(MessageParcel &data, MessageParcel &reply)
{
    int32_t temp = data.ReadInt32();
    if (temp < 0 || temp >= MAX_OPERATION_CODE) {
        reply.WriteInt32(AUDIO_INVALID_PARAM);
        AUDIO_ERR_LOG("HandleOnOperationHandled failed, invalid operation: %{public}d", temp);
        return AUDIO_INVALID_PARAM;
    }

    Operation operation = static_cast<Operation>(temp);
    int64_t result = data.ReadInt64();
    reply.WriteInt32(OnOperationHandled(operation, result));
    return AUDIO_OK;
}
} // namespace AudioStandard
} // namespace OHOS
