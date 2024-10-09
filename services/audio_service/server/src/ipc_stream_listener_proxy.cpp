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
#define LOG_TAG "IpcStreamListenerProxy"
#endif

#include "ipc_stream_listener_proxy.h"
#include "audio_service_log.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
IpcStreamListenerProxy::IpcStreamListenerProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IpcStreamListener>(impl)
{
    AUDIO_INFO_LOG("IpcStreamListenerProxy()");
}

IpcStreamListenerProxy::~IpcStreamListenerProxy()
{
    AUDIO_INFO_LOG("~IpcStreamListenerProxy()");
}

int32_t IpcStreamListenerProxy::OnOperationHandled(Operation operation, int64_t result)
{
    MessageParcel data;
    MessageParcel reply;
    int32_t flag = operation == SET_OFFLOAD_ENABLE ? (MessageOption::TF_ASYNC | MessageOption::TF_ASYNC_WAKEUP_LATER)
        : MessageOption::TF_ASYNC;
    MessageOption option(flag); // server call client in async

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERR_OPERATION_FAILED,
        "Write descriptor failed!");

    data.WriteInt32(operation);
    data.WriteInt64(result);
    int ret = Remote()->SendRequest(IpcStreamListenerMsg::ON_OPERATION_HANDLED, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ERR_OPERATION_FAILED, "OnEndpointChange failed, error: %{public}d", ret);
    return reply.ReadInt32();
}
} // namespace AudioStandard
} // namespace OHOS
