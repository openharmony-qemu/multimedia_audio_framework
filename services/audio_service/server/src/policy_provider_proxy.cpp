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
#define LOG_TAG "PolicyProviderProxy"
#endif

#include "policy_provider_proxy.h"
#include "audio_service_log.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
PolicyProviderProxy::PolicyProviderProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IPolicyProviderIpc>(impl)
{
}

PolicyProviderProxy::~PolicyProviderProxy()
{
}

int32_t PolicyProviderProxy::GetProcessDeviceInfo(const AudioProcessConfig &config, bool lockFlag,
    AudioDeviceDescriptor &deviceInfo)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");

    ProcessConfig::WriteConfigToParcel(config, data);
    data.WriteBool(lockFlag);
    int ret = Remote()->SendRequest(IPolicyProviderMsg::GET_DEVICE_INFO, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ERR_OPERATION_FAILED, "GetProcessDeviceInfo failed, error: %{public}d",
        ret);
    deviceInfo.Unmarshalling(reply);
    return SUCCESS;
}

int32_t PolicyProviderProxy::InitSharedVolume(std::shared_ptr<AudioSharedMemory> &buffer)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");

    int ret = Remote()->SendRequest(IPolicyProviderMsg::INIT_VOLUME_MAP, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ERR_OPERATION_FAILED, "InitSharedVolume failed, error: %{public}d", ret);
    buffer = AudioSharedMemory::ReadFromParcel(reply);
    CHECK_AND_RETURN_RET_LOG(buffer != nullptr, ERR_OPERATION_FAILED, "ReadFromParcel failed");
    return SUCCESS;
}

int32_t PolicyProviderProxy::SetWakeUpAudioCapturerFromAudioServer(const AudioProcessConfig &config)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");

    ProcessConfig::WriteConfigToParcel(config, data);
    int ret = Remote()->SendRequest(IPolicyProviderMsg::SET_WAKEUP_ADUIO_CAPTURER, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ERR_OPERATION_FAILED, "failed, error: %{public}d", ret);

    return reply.ReadInt32();
}

int32_t PolicyProviderProxy::NotifyCapturerAdded(AudioCapturerInfo capturerInfo, AudioStreamInfo streamInfo,
    uint32_t sessionId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");
    capturerInfo.Marshalling(data);
    streamInfo.Marshalling(data);
    data.WriteUint32(sessionId);

    int ret = Remote()->SendRequest(IPolicyProviderMsg::SET_AUDIO_CAPTURER, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ERR_OPERATION_FAILED, "failed, error: %{public}d", ret);

    return reply.ReadInt32();
}

int32_t PolicyProviderProxy::NotifyWakeUpCapturerRemoved()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");

    int ret = Remote()->SendRequest(IPolicyProviderMsg::REMOVE_WAKEUP_CAPUTER, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ERR_OPERATION_FAILED, "failed, error: %{public}d", ret);

    return reply.ReadInt32();
}

bool PolicyProviderProxy::IsAbsVolumeSupported()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");

    int ret = Remote()->SendRequest(IPolicyProviderMsg::IS_ABS_VOLUME_SUPPORTED, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ERR_OPERATION_FAILED, "failed, error: %{public}d", ret);

    return reply.ReadBool();
}

int32_t PolicyProviderProxy::OffloadGetRenderPosition(uint32_t &delayValue, uint64_t &sendDataSize, uint32_t &timeStamp)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");

    int ret = Remote()->SendRequest(IPolicyProviderMsg::OFFLOAD_GET_RENDER_POSITION, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ERR_OPERATION_FAILED, "failed, error: %{public}d", ret);
    ret = reply.ReadInt32();
    delayValue = reply.ReadUint32();
    sendDataSize = reply.ReadUint64();
    timeStamp = reply.ReadUint32();
    return ret;
}

int32_t PolicyProviderProxy::GetAndSaveClientType(uint32_t uid, const std::string &bundleName)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");

    data.WriteUint32(uid);
    data.WriteString(bundleName);
    int ret = Remote()->SendRequest(IPolicyProviderMsg::GET_AND_SAVE_CLIENT_TYPE, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ERR_OPERATION_FAILED, "failed, error: %{public}d", ret);
    ret = reply.ReadInt32();
    return ret;
}

int32_t PolicyProviderProxy::GetMaxRendererInstances()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");

    int ret = Remote()->SendRequest(IPolicyProviderMsg::GET_MAX_RENDERER_INSTANCES, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ERR_OPERATION_FAILED, "failed, error: %{public}d", ret);

    return reply.ReadInt32();
}

int32_t PolicyProviderProxy::ActivateConcurrencyFromServer(AudioPipeType incomingPipe)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");

    data.WriteInt32(incomingPipe);
    int ret = Remote()->SendRequest(IPolicyProviderMsg::ACTIVATE_CONCURRENCY_FROM_SERVER, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ERR_OPERATION_FAILED,
        "failed, error: %{public}d", ret);
    return reply.ReadInt32();
}

int32_t PolicyProviderProxy::NotifyCapturerRemoved(uint64_t sessionId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteUint64(sessionId);

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");

    int ret = Remote()->SendRequest(IPolicyProviderMsg::REMOVE_AUDIO_CAPTURER, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ERR_OPERATION_FAILED, "failed, error: %{public}d", ret);

    return reply.ReadInt32();
}

int32_t PolicyProviderProxy::SetDefaultOutputDevice(const DeviceType defaultOutputDevice, const uint32_t sessionID,
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
    int ret = Remote()->SendRequest(IPolicyProviderMsg::SET_DEFAULT_OUTPUT_DEVICE, data, reply, option);
    CHECK_AND_RETURN_RET(ret == AUDIO_OK, ret, "set default output device failed, ipc error: %{public}d", ret);
    return reply.ReadInt32();
}

#ifdef HAS_FEATURE_INNERCAPTURER
int32_t PolicyProviderProxy::LoadModernInnerCapSink(int32_t innerCapId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");
    data.WriteInt32(innerCapId);
    int ret = Remote()->SendRequest(IPolicyProviderMsg::LOAD_MODERN_INNER_CAPTURE_SINK, data, reply, option);
    CHECK_AND_RETURN_RET(ret == AUDIO_OK, ret, "LoadModernInnerCapSink failed, ipc error: %{public}d", ret);
    return reply.ReadInt32();
}

int32_t PolicyProviderProxy::UnloadModernInnerCapSink(int32_t innerCapId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");
    data.WriteInt32(innerCapId);
    int ret = Remote()->SendRequest(IPolicyProviderMsg::UNLOAD_MODERN_INNER_CAPTURE_SINK, data, reply, option);
    CHECK_AND_RETURN_RET(ret == AUDIO_OK, ret, "UnloadModernInnerCapSink failed, ipc error: %{public}d", ret);
    return reply.ReadInt32();
}
#endif
} // namespace AudioStandard
} // namespace OHOS
