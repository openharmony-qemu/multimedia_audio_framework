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
#define LOG_TAG "AudioProcessStub"
#endif

#include "audio_process_stub.h"
#include "audio_service_log.h"
#include "audio_errors.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {
ProcessCbProxy::ProcessCbProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IProcessCb>(impl)
{
}

ProcessCbProxy::~ProcessCbProxy()
{
}

int32_t ProcessCbProxy::OnEndpointChange(int32_t status)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERR_OPERATION_FAILED,
        "Write descriptor failed!");

    data.WriteInt32(status);
    int ret = Remote()->SendRequest(IProcessCbMsg::ON_ENDPOINT_CHANGE, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ERR_OPERATION_FAILED, "OnEndpointChange failed, error: %{public}d", ret);
    return reply.ReadInt32();
}

bool AudioProcessStub::CheckInterfaceToken(MessageParcel &data)
{
    static auto localDescriptor = IAudioProcess::GetDescriptor();
    auto remoteDescriptor = data.ReadInterfaceToken();
    CHECK_AND_RETURN_RET_LOG(remoteDescriptor == localDescriptor, false, "CheckInterFfaceToken failed.");
    return true;
}

int AudioProcessStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    bool ret = CheckInterfaceToken(data);
    CHECK_AND_RETURN_RET(ret, AUDIO_ERR);
    Trace trace("AudioProcess::Handle::" + std::to_string(code));
    if (code >= IAudioProcessMsg::PROCESS_MAX_MSG) {
        AUDIO_WARNING_LOG("OnRemoteRequest unsupported request code:%{public}d.", code);
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    switch (code) {
        case ON_RESOLVE_BUFFER:
            return HandleResolveBuffer(data, reply);
        case OH_GET_SESSIONID:
            return HandleGetSessionId(data, reply);
        case ON_START:
            return HandleStart(data, reply);
        case ON_PAUSE:
            return HandlePause(data, reply);
        case ON_RESUME:
            return HandleResume(data, reply);
        case ON_STOP:
            return HandleStop(data, reply);
        case ON_REQUEST_HANDLE_INFO:
            return HandleRequestHandleInfo(data, reply);
        case ON_RELEASE:
            return HandleRelease(data, reply);
        case ON_REGISTER_PROCESS_CB:
            return HandleRegisterProcessCb(data, reply);
        case ON_REGISTER_THREAD_PRIORITY:
            return HandleRegisterThreadPriority(data, reply);
        default:
            AUDIO_WARNING_LOG("OnRemoteRequest unsupported request code:%{public}d.", code);
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}
}

int32_t AudioProcessStub::HandleResolveBuffer(MessageParcel &data, MessageParcel &reply)
{
    AUDIO_INFO_LOG("HandleResolveBuffer");
    (void)data;
    std::shared_ptr<OHAudioBuffer> buffer;
    int32_t ret = ResolveBuffer(buffer);
    reply.WriteInt32(ret);
    if (ret == AUDIO_OK && buffer != nullptr) {
        OHAudioBuffer::WriteToParcel(buffer, reply);
    } else {
        AUDIO_ERR_LOG("error: ResolveBuffer failed.");
        return AUDIO_INVALID_PARAM;
    }

    return AUDIO_OK;
}

int32_t AudioProcessStub::HandleGetSessionId(MessageParcel &data, MessageParcel &reply)
{
    (void)data;
    uint32_t sessionId = 0;
    int32_t ret = GetSessionId(sessionId);
    reply.WriteInt32(ret);
    reply.WriteUint32(sessionId);
    return AUDIO_OK;
}

int32_t AudioProcessStub::HandleStart(MessageParcel &data, MessageParcel &reply)
{
    (void)data;
    reply.WriteInt32(Start());
    return AUDIO_OK;
}

int32_t AudioProcessStub::HandleResume(MessageParcel &data, MessageParcel &reply)
{
    (void)data;
    reply.WriteInt32(Resume());
    return AUDIO_OK;
}

int32_t AudioProcessStub::HandlePause(MessageParcel &data, MessageParcel &reply)
{
    bool isFlush = data.ReadBool();
    reply.WriteInt32(Pause(isFlush));
    return AUDIO_OK;
}

int32_t AudioProcessStub::HandleStop(MessageParcel &data, MessageParcel &reply)
{
    (void)data;
    reply.WriteInt32(Stop());
    return AUDIO_OK;
}

int32_t AudioProcessStub::HandleRequestHandleInfo(MessageParcel &data, MessageParcel &reply)
{
    (void)data;
    reply.WriteInt32(RequestHandleInfo());
    return AUDIO_OK;
}

int32_t AudioProcessStub::HandleRelease(MessageParcel &data, MessageParcel &reply)
{
    (void)data;
    reply.WriteInt32(Release());
    return AUDIO_OK;
}

int32_t AudioProcessStub::HandleRegisterProcessCb(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> object = data.ReadRemoteObject();
    CHECK_AND_RETURN_RET_LOG(object != nullptr, AUDIO_INVALID_PARAM, "obj is null");
    reply.WriteInt32(RegisterProcessCb(object));
    return AUDIO_OK;
}

int32_t AudioProcessStub::HandleRegisterThreadPriority(MessageParcel &data, MessageParcel &reply)
{
    uint32_t tid = data.ReadUint32();
    std::string bundleName = data.ReadString();
    reply.WriteInt32(RegisterThreadPriority(tid, bundleName));
    return AUDIO_OK;
}
} // namespace AudioStandard
} // namespace OHOS
