/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "IpcOfflineStreamStub"
#endif

#include "ipc_offline_stream_stub.h"
#include "audio_service_log.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
IpcOfflineStreamStub::IpcOfflineStreamStub()
{
    handlerMap_ = {
        {CREATE_OFFLINE_EFFECT_CHAIN, [this](MessageParcel &data, MessageParcel &reply) {
            return HandleCreateOfflineEffectChain(data, reply);
        }},
        {CONFIGURE_OFFLINE_EFFECT_CHAIN, [this](MessageParcel &data, MessageParcel &reply) {
            return HandleConfigureOfflineEffectChain(data, reply);
        }},
        {PROCESS_OFFLINE_EFFECT_CHAIN, [this](MessageParcel &data, MessageParcel &reply) {
            return HandleProcessOfflineEffectChain(data, reply);
        }},
        {PREPARE_OFFLINE_EFFECT_CHAIN, [this](MessageParcel &data, MessageParcel &reply) {
            return HandlePrepareOfflineEffectChain(data, reply);
        }},
        {RELEASE_OFFLINE_EFFECT_CHAIN, [this](MessageParcel &data, MessageParcel &reply) {
            return HandleReleaseOfflineEffectChain(data, reply);
        }}
    };
}

bool IpcOfflineStreamStub::CheckInterfaceToken(MessageParcel &data)
{
    static auto localDescriptor = IpcOfflineStream::GetDescriptor();
    auto remoteDescriptor = data.ReadInterfaceToken();
    if (remoteDescriptor != localDescriptor) {
        AUDIO_ERR_LOG("CheckInterFfaceToken failed.");
        return false;
    }
    return true;
}

int32_t IpcOfflineStreamStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    CHECK_AND_RETURN_RET(CheckInterfaceToken(data), AUDIO_ERR);
    auto it = handlerMap_.find(code);
    if (it != handlerMap_.end()) {
        return it->second(data, reply);
    }
    AUDIO_WARNING_LOG("OnRemoteRequest unsupported request code:%{public}d.", code);
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t IpcOfflineStreamStub::HandleCreateOfflineEffectChain(MessageParcel &data, MessageParcel &reply)
{
#ifdef FEATURE_OFFLINE_EFFECT
    std::string chainName = data.ReadString();
    int32_t ret = CreateOfflineEffectChain(chainName);
    reply.WriteInt32(ret);
    return AUDIO_OK;
#endif
    return ERR_NOT_SUPPORTED;
}

int32_t IpcOfflineStreamStub::HandleConfigureOfflineEffectChain(MessageParcel &data, MessageParcel &reply)
{
#ifdef FEATURE_OFFLINE_EFFECT
    AudioStreamInfo inInfo;
    AudioStreamInfo outInfo;
    inInfo.Unmarshalling(data);
    outInfo.Unmarshalling(data);
    int32_t ret = ConfigureOfflineEffectChain(inInfo, outInfo);
    reply.WriteInt32(ret);
    return AUDIO_OK;
#endif
    return ERR_NOT_SUPPORTED;
}

int32_t IpcOfflineStreamStub::HandlePrepareOfflineEffectChain(MessageParcel &data, MessageParcel &reply)
{
#ifdef FEATURE_OFFLINE_EFFECT
    (void)data;
    std::shared_ptr<AudioSharedMemory> inBuf = nullptr;
    std::shared_ptr<AudioSharedMemory> outBuf = nullptr;
    int32_t ret = PrepareOfflineEffectChain(inBuf, outBuf);
    AudioSharedMemory::WriteToParcel(inBuf, reply);
    AudioSharedMemory::WriteToParcel(outBuf, reply);
    reply.WriteInt32(ret);
    return AUDIO_OK;
#endif
    return ERR_NOT_SUPPORTED;
}

int32_t IpcOfflineStreamStub::HandleProcessOfflineEffectChain(MessageParcel &data, MessageParcel &reply)
{
#ifdef FEATURE_OFFLINE_EFFECT
    uint32_t inSize;
    uint32_t outSize;
    inSize = data.ReadUint32();
    outSize = data.ReadUint32();
    int32_t ret = ProcessOfflineEffectChain(inSize, outSize);
    return ret;
#endif
    return ERR_NOT_SUPPORTED;
}

int32_t IpcOfflineStreamStub::HandleReleaseOfflineEffectChain(MessageParcel &data, MessageParcel &reply)
{
#ifdef FEATURE_OFFLINE_EFFECT
    ReleaseOfflineEffectChain();
    return SUCCESS;
#endif
    return ERR_NOT_SUPPORTED;
}
} // namespace AudioStandard
} // namespace OHOS
