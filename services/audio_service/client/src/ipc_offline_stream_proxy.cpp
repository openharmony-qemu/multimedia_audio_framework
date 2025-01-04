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
#define LOG_TAG "IpcOfflineStreamProxy"
#endif

#include "ipc_offline_stream_proxy.h"

#include "ipc_offline_stream_stub.h"
#include "audio_service_log.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
IpcOfflineStreamProxy::IpcOfflineStreamProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IpcOfflineStream>(impl) {}

IpcOfflineStreamProxy::~IpcOfflineStreamProxy() = default;

#ifdef FEATURE_OFFLINE_EFFECT
int32_t IpcOfflineStreamProxy::CreateOfflineEffectChain(const std::string &chainName)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");
    data.WriteString(chainName);
    int ret = Remote()->SendRequest(IpcOfflineStreamMsg::CREATE_OFFLINE_EFFECT_CHAIN, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ERR_OPERATION_FAILED, "Create failed, error: %{public}d", ret);

    ret = reply.ReadInt32();
    return ret;
}

int32_t IpcOfflineStreamProxy::ConfigureOfflineEffectChain(const AudioStreamInfo &inInfo,
    const AudioStreamInfo &outInfo)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");
    inInfo.Marshalling(data);
    outInfo.Marshalling(data);
    int ret = Remote()->SendRequest(IpcOfflineStreamMsg::CONFIGURE_OFFLINE_EFFECT_CHAIN, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ERR_OPERATION_FAILED, "Configure failed, error: %{public}d", ret);

    ret = reply.ReadInt32();
    return ret;
}


int32_t IpcOfflineStreamProxy::PrepareOfflineEffectChain(std::shared_ptr<AudioSharedMemory> &inBuffer,
    std::shared_ptr<AudioSharedMemory> &outBuffer)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");
    int ret = Remote()->SendRequest(IpcOfflineStreamMsg::PREPARE_OFFLINE_EFFECT_CHAIN, data, reply, option);
    inBuffer = AudioSharedMemory::ReadFromParcel(reply);
    outBuffer = AudioSharedMemory::ReadFromParcel(reply);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ERR_OPERATION_FAILED, "Configure failed, error: %{public}d", ret);

    ret = reply.ReadInt32();
    return ret;
}

int32_t IpcOfflineStreamProxy::ProcessOfflineEffectChain(uint32_t inSize, uint32_t outSize)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");
    data.WriteUint32(inSize);
    data.WriteUint32(outSize);
    int ret = Remote()->SendRequest(IpcOfflineStreamMsg::PROCESS_OFFLINE_EFFECT_CHAIN, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ERR_OPERATION_FAILED, "Process failed, error: %{public}d", ret);

    ret = reply.ReadInt32();
    return ret;
}

void IpcOfflineStreamProxy::ReleaseOfflineEffectChain()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_LOG(data.WriteInterfaceToken(GetDescriptor()), "Write descriptor failed!");
    int ret = Remote()->SendRequest(IpcOfflineStreamMsg::RELEASE_OFFLINE_EFFECT_CHAIN, data, reply, option);
    CHECK_AND_RETURN_LOG(ret == AUDIO_OK, "Relase failed, error: %{public}d", ret);
    return;
}
#endif
} // namespace AudioStandard
} // namespace OHOS
