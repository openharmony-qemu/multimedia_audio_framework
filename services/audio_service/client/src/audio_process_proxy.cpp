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
#define LOG_TAG "AudioProcessProxy"
#endif

#include "audio_process_cb_stub.h"
#include "audio_process_proxy.h"
#include "audio_service_log.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
bool ProcessCbStub::CheckInterfaceToken(MessageParcel &data)
{
    static auto localDescriptor = IProcessCb::GetDescriptor();
    auto remoteDescriptor = data.ReadInterfaceToken();
    CHECK_AND_RETURN_RET_LOG(remoteDescriptor == localDescriptor, false, "CheckInterFfaceToken failed.");
    return true;
}

int ProcessCbStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    bool ret = CheckInterfaceToken(data);
    CHECK_AND_RETURN_RET(ret, AUDIO_ERR);
    if (code >= IProcessCbMsg::PROCESS_CB_MAX_MSG) {
        AUDIO_WARNING_LOG("OnRemoteRequest unsupported request code:%{public}d.", code);
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    switch (code) {
        case ON_ENDPOINT_CHANGE:
            return HandleOnEndpointChange(data, reply);
        default:
            AUDIO_WARNING_LOG("OnRemoteRequest not supported code:%{public}d.", code);
            return AUDIO_ERR;
    }
}

int32_t ProcessCbStub::HandleOnEndpointChange(MessageParcel &data, MessageParcel &reply)
{
    int32_t status = data.ReadInt32();
    reply.WriteInt32(OnEndpointChange(status));
    return AUDIO_OK;
}

AudioProcessProxy::AudioProcessProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IAudioProcess>(impl)
{
}

AudioProcessProxy::~AudioProcessProxy()
{
}

int32_t AudioProcessProxy::ResolveBuffer(std::shared_ptr<OHAudioBuffer> &buffer)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");

    int ret = Remote()->SendRequest(IAudioProcessMsg::ON_RESOLVE_BUFFER, data, reply, option);
    CHECK_AND_RETURN_RET_LOG((ret == AUDIO_OK && reply.ReadInt32() == AUDIO_OK), ERR_OPERATION_FAILED,
        "ResolveBuffer failed, error: %{public}d", ret);
    buffer = OHAudioBuffer::ReadFromParcel(reply);
    CHECK_AND_RETURN_RET_LOG(buffer != nullptr, ERR_OPERATION_FAILED, "ReadFromParcel failed");
    return SUCCESS;
}

int32_t AudioProcessProxy::GetSessionId(uint32_t &sessionId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");

    int ret = Remote()->SendRequest(IAudioProcessMsg::OH_GET_SESSIONID, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ERR_OPERATION_FAILED, "Start failed, error: %{public}d", ret);

    ret = reply.ReadInt32();
    sessionId = reply.ReadUint32();
    return ret;
}

int32_t AudioProcessProxy::Start()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");

    int ret = Remote()->SendRequest(IAudioProcessMsg::ON_START, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ERR_OPERATION_FAILED, "Start failed, error: %{public}d", ret);

    return reply.ReadInt32();
}

int32_t AudioProcessProxy::Pause(bool isFlush)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");
    data.WriteBool(isFlush);
    int ret = Remote()->SendRequest(IAudioProcessMsg::ON_PAUSE, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ERR_OPERATION_FAILED, "Pause failed, error: %{public}d", ret);

    return reply.ReadInt32();
}

int32_t AudioProcessProxy::Resume()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");

    int ret = Remote()->SendRequest(IAudioProcessMsg::ON_RESUME, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ERR_OPERATION_FAILED, "Resume failed, error: %{public}d", ret);

    return reply.ReadInt32();
}

int32_t AudioProcessProxy::Stop()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");

    int ret = Remote()->SendRequest(IAudioProcessMsg::ON_STOP, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ERR_OPERATION_FAILED, "Stop failed, error: %{public}d", ret);

    return reply.ReadInt32();
}

int32_t AudioProcessProxy::RequestHandleInfo(bool isAsync)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = isAsync ? MessageOption(MessageOption::TF_ASYNC) : MessageOption();

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");

    int ret = Remote()->SendRequest(IAudioProcessMsg::ON_REQUEST_HANDLE_INFO, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ERR_OPERATION_FAILED, "RequestHandleInfo failed: %{public}d", ret);

    return reply.ReadInt32();
}

int32_t AudioProcessProxy::Release()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");

    int ret = Remote()->SendRequest(IAudioProcessMsg::ON_RELEASE, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ERR_OPERATION_FAILED, "Release failed, error: %{public}d", ret);

    return reply.ReadInt32();
}

int32_t AudioProcessProxy::RegisterProcessCb(sptr<IRemoteObject> object)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");

    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_NULL_OBJECT,
        "RegisterProcessCb object is null");

    data.WriteRemoteObject(object);

    int ret = Remote()->SendRequest(IAudioProcessMsg::ON_REGISTER_PROCESS_CB, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ERR_OPERATION_FAILED, "RegisterProcessCb failed, error: %{public}d", ret);

    return reply.ReadInt32();
}

int32_t AudioProcessProxy::RegisterThreadPriority(uint32_t tid, const std::string &bundleName)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");
    data.WriteUint32(tid);
    data.WriteString(bundleName);
    int ret = Remote()->SendRequest(IAudioProcessMsg::ON_REGISTER_THREAD_PRIORITY, data, reply, option);
    CHECK_AND_RETURN_RET(ret == AUDIO_OK, ret, "failed, ipc error: %{public}d", ret);
    ret = reply.ReadInt32();
    CHECK_AND_RETURN_RET(ret == SUCCESS, ret, "failed, error: %{public}d", ret);
    return ret;
}
} // namespace AudioStandard
} // namespace OHOS
