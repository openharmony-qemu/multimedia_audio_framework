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
#define LOG_TAG "IpcStreamProxy"
#endif

#include "ipc_stream_proxy.h"
#include "audio_service_log.h"
#include "audio_errors.h"
#include "audio_process_config.h"

namespace OHOS {
namespace AudioStandard {
IpcStreamProxy::IpcStreamProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IpcStream>(impl)
{
    AUDIO_INFO_LOG("IpcStreamProxy()");
}

IpcStreamProxy::~IpcStreamProxy()
{
    AUDIO_INFO_LOG("~IpcStreamProxy()");
}

int32_t IpcStreamProxy::RegisterStreamListener(sptr<IRemoteObject> object)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");

    if (object == nullptr) {
        AUDIO_ERR_LOG("RegisterStreamListener object is null");
        return ERR_NULL_OBJECT;
    }

    data.WriteRemoteObject(object);

    int ret = Remote()->SendRequest(IpcStreamMsg::ON_REGISTER_STREAM_LISTENER, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ERR_OPERATION_FAILED, "RegisterStreamListener failed,"
        "error: %{public}d", ret);

    return reply.ReadInt32();
}

int32_t IpcStreamProxy::ResolveBuffer(std::shared_ptr<OHAudioBuffer> &buffer)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");

    int ret = Remote()->SendRequest(IpcStreamMsg::ON_RESOLVE_BUFFER, data, reply, option);
    CHECK_AND_RETURN_RET_LOG((ret == AUDIO_OK && reply.ReadInt32() == AUDIO_OK), ERR_OPERATION_FAILED,
        "ResolveBuffer failed, error: %{public}d", ret);
    buffer = OHAudioBuffer::ReadFromParcel(reply);
    CHECK_AND_RETURN_RET_LOG(buffer != nullptr, ERR_OPERATION_FAILED, "ReadFromParcel failed");
    return SUCCESS;
}

int32_t IpcStreamProxy::UpdatePosition()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");

    int ret = Remote()->SendRequest(IpcStreamMsg::ON_UPDATE_POSITION, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ERR_OPERATION_FAILED, "UpdatePosition failed, error: %{public}d", ret);

    return reply.ReadInt32();
}

int32_t IpcStreamProxy::GetAudioSessionID(uint32_t &sessionId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");

    int ret = Remote()->SendRequest(IpcStreamMsg::ON_GET_AUDIO_SESSIONID, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ERR_OPERATION_FAILED, "GetAudioSessionID failed, error: %{public}d", ret);
    ret = reply.ReadInt32();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetAudioSessionID failed, error: %{public}d", ret);
    sessionId = reply.ReadUint32();
    return ret;
}

int32_t IpcStreamProxy::Start()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");

    int ret = Remote()->SendRequest(IpcStreamMsg::ON_START, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ERR_OPERATION_FAILED, "Start failed, error: %{public}d", ret);

    return reply.ReadInt32();
}

int32_t IpcStreamProxy::Pause()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");
    int ret = Remote()->SendRequest(IpcStreamMsg::ON_PAUSE, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ERR_OPERATION_FAILED, "Pause failed, error: %{public}d", ret);

    return reply.ReadInt32();
}

int32_t IpcStreamProxy::Stop()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");

    int ret = Remote()->SendRequest(IpcStreamMsg::ON_STOP, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ERR_OPERATION_FAILED, "Stop failed, error: %{public}d", ret);

    return reply.ReadInt32();
}

int32_t IpcStreamProxy::Release()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");

    int ret = Remote()->SendRequest(IpcStreamMsg::ON_RELEASE, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ERR_OPERATION_FAILED, "Release failed, error: %{public}d", ret);

    return reply.ReadInt32();
}

int32_t IpcStreamProxy::Flush()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");

    int ret = Remote()->SendRequest(IpcStreamMsg::ON_FLUSH, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ERR_OPERATION_FAILED, "Flush failed, error: %{public}d", ret);

    return reply.ReadInt32();
}

int32_t IpcStreamProxy::Drain(bool stopFlag)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");
    data.WriteBool(stopFlag);
    int ret = Remote()->SendRequest(IpcStreamMsg::ON_DRAIN, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ret, "Drain failed, ipc error: %{public}d", ret);
    return reply.ReadInt32();
}

int32_t IpcStreamProxy::UpdatePlaybackCaptureConfig(const AudioPlaybackCaptureConfig &config)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");

    int32_t ret = ProcessConfig::WriteInnerCapConfigToParcel(config, data);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Write config failed");

    ret = Remote()->SendRequest(IpcStreamMsg::ON_UPDATA_PLAYBACK_CAPTURER_CONFIG, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ret, "Failed, ipc error: %{public}d", ret);
    return reply.ReadInt32();
}

int32_t IpcStreamProxy::GetAudioTime(uint64_t &framePos, uint64_t &timestamp)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");

    int ret = Remote()->SendRequest(IpcStreamMsg::OH_GET_AUDIO_TIME, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ret, "GetAudioTime failed, ipc error: %{public}d", ret);
    ret = reply.ReadInt32();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetAudioTime failed, error: %{public}d", ret);
    framePos = reply.ReadUint64();
    timestamp = reply.ReadUint64();
    return ret;
}

int32_t IpcStreamProxy::GetAudioPosition(uint64_t &framePos, uint64_t &timestamp, uint64_t &latency)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");

    int ret = Remote()->SendRequest(IpcStreamMsg::OH_GET_AUDIO_POSITION, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ret, "ipc error: %{public}d", ret);
    ret = reply.ReadInt32();
    CHECK_AND_RETURN_RET_PRELOG(ret == SUCCESS, ret, "error: %{public}d", ret);
    framePos = reply.ReadUint64();
    timestamp = reply.ReadUint64();
    latency = reply.ReadUint64();
    return ret;
}

int32_t IpcStreamProxy::GetLatency(uint64_t &latency)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");

    int ret = Remote()->SendRequest(IpcStreamMsg::ON_GET_LATENCY, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ret, "GetLatency failed, ipc error: %{public}d", ret);
    ret = reply.ReadInt32();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetLatency failed, error: %{public}d", ret);
    latency = reply.ReadUint64();

    return ret;
}

int32_t IpcStreamProxy::SetRate(int32_t rate)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");
    data.WriteInt32(rate);
    int ret = Remote()->SendRequest(IpcStreamMsg::ON_SET_RATE, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ERR_OPERATION_FAILED, "SetRate failed, ipc error: %{public}d", ret);

    return reply.ReadInt32();
}

int32_t IpcStreamProxy::GetRate(int32_t &rate)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");

    int ret = Remote()->SendRequest(IpcStreamMsg::ON_GET_RATE, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ret, "GetRate failed, ipc error: %{public}d", ret);
    ret = reply.ReadInt32();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetRate failed, error: %{public}d", ret);
    rate = reply.ReadInt32();

    return ret;
}

int32_t IpcStreamProxy::SetLowPowerVolume(float volume)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");

    data.WriteFloat(volume);
    int ret = Remote()->SendRequest(IpcStreamMsg::ON_SET_LOWPOWER_VOLUME, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ERR_OPERATION_FAILED, "SetLowPowerVolume failed, error:%{public}d", ret);

    return reply.ReadInt32();
}

int32_t IpcStreamProxy::GetLowPowerVolume(float &volume)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");

    int ret = Remote()->SendRequest(IpcStreamMsg::ON_GET_LOWPOWER_VOLUME, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ret, "GetLowPowerVolume failed, ipc error: %{public}d", ret);
    ret = reply.ReadInt32();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetLowPowerVolume failed, error: %{public}d", ret);
    volume = reply.ReadFloat();

    return ret;
}

int32_t IpcStreamProxy::SetAudioEffectMode(int32_t effectMode)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");

    data.WriteInt32(effectMode);
    int ret = Remote()->SendRequest(IpcStreamMsg::ON_SET_EFFECT_MODE, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ERR_OPERATION_FAILED, "SetAudioEffectMode failed, ipc error: %{public}d",
        ret);

    return reply.ReadInt32();
}

int32_t IpcStreamProxy::GetAudioEffectMode(int32_t &effectMode)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");

    int ret = Remote()->SendRequest(IpcStreamMsg::ON_GET_EFFECT_MODE, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ret, "GetAudioEffectMode failed, ipc error: %{public}d", ret);
    ret = reply.ReadInt32();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetAudioEffectMode failed, error: %{public}d", ret);
    effectMode = reply.ReadInt32();

    return ret;
}

int32_t IpcStreamProxy::SetPrivacyType(int32_t privacyType)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");

    int ret = Remote()->SendRequest(IpcStreamMsg::ON_SET_PRIVACY_TYPE, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ERR_OPERATION_FAILED, "SetPrivacyType failed, error: %{public}d", ret);

    return reply.ReadInt32();
}

int32_t IpcStreamProxy::GetPrivacyType(int32_t &privacyType)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");

    int ret = Remote()->SendRequest(IpcStreamMsg::ON_GET_PRIVACY_TYPE, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ret, "GetPrivacyType failed, ipc error: %{public}d", ret);
    ret = reply.ReadInt32();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetPrivacyType failed, error: %{public}d", ret);
    privacyType = reply.ReadInt32();

    return ret;
}

int32_t IpcStreamProxy::SetOffloadMode(int32_t state, bool isAppBack)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");

    data.WriteInt32(state);
    data.WriteBool(isAppBack);
    int ret = Remote()->SendRequest(IpcStreamMsg::ON_SET_OFFLOAD_MODE, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ret, "failed, ipc error: %{public}d", ret);
    ret = reply.ReadInt32();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "failed, error: %{public}d", ret);

    return ret;
}

int32_t IpcStreamProxy::UnsetOffloadMode()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");

    int ret = Remote()->SendRequest(IpcStreamMsg::ON_UNSET_OFFLOAD_MODE, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ret, "failed, ipc error: %{public}d", ret);
    ret = reply.ReadInt32();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "failed, error: %{public}d", ret);

    return ret;
}

int32_t IpcStreamProxy::GetOffloadApproximatelyCacheTime(uint64_t &timestamp, uint64_t &paWriteIndex,
    uint64_t &cacheTimeDsp, uint64_t &cacheTimePa)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");

    data.WriteUint64(timestamp);
    data.WriteUint64(paWriteIndex);
    data.WriteUint64(cacheTimeDsp);
    data.WriteUint64(cacheTimePa);
    int ret = Remote()->SendRequest(IpcStreamMsg::ON_GET_OFFLOAD_APPROXIMATELY_CACHE_TIME, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ret, "failed, ipc error: %{public}d",
        ret);
    ret = reply.ReadInt32();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "failed, error: %{public}d", ret);
    timestamp = reply.ReadUint64();
    paWriteIndex = reply.ReadUint64();
    cacheTimeDsp = reply.ReadUint64();
    cacheTimePa = reply.ReadUint64();

    return ret;
}

int32_t IpcStreamProxy::UpdateSpatializationState(bool spatializationEnabled, bool headTrackingEnabled)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");

    data.WriteBool(spatializationEnabled);
    data.WriteBool(headTrackingEnabled);
    int ret = Remote()->SendRequest(IpcStreamMsg::ON_UPDATE_SPATIALIZATION_STATE, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ret, "failed, ipc error: %{public}d", ret);
    ret = reply.ReadInt32();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "failed, error: %{public}d", ret);
    return ret;
}

int32_t IpcStreamProxy::GetStreamManagerType()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");
    int ret = Remote()->SendRequest(IpcStreamMsg::ON_GET_STREAM_MANAGER_TYPE, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ret, "failed, ipc error: %{public}d", ret);
    return reply.ReadInt32();
}

int32_t IpcStreamProxy::SetSilentModeAndMixWithOthers(bool on)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");

    data.WriteBool(on);
    int ret = Remote()->SendRequest(IpcStreamMsg::ON_SET_SILENT_MODE_AND_MIX_WITH_OTHERS, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ret, "failed, ipc error: %{public}d", ret);
    ret = reply.ReadInt32();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "failed, error: %{public}d", ret);
    return ret;
}

int32_t IpcStreamProxy::SetClientVolume()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");

    int ret = Remote()->SendRequest(IpcStreamMsg::ON_SET_CLIENT_VOLUME, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ret, "set client volume failed, ipc error: %{public}d", ret);
    return reply.ReadInt32();
}

int32_t IpcStreamProxy::SetMute(bool isMute)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");

    data.WriteBool(isMute);
    int ret = Remote()->SendRequest(IpcStreamMsg::ON_SET_MUTE, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ret, "set mute failed, ipc error: %{public}d", ret);
    return reply.ReadInt32();
}

int32_t IpcStreamProxy::SetDuckFactor(float duckFactor)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");

    data.WriteFloat(duckFactor);
    int ret = Remote()->SendRequest(IpcStreamMsg::ON_SET_DUCK_FACTOR, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ret, "set duck failed, ipc error: %{public}d", ret);
    return reply.ReadInt32();
}

int32_t IpcStreamProxy::RegisterThreadPriority(uint32_t tid, const std::string &bundleName)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "Write descriptor failed!");
    data.WriteUint32(tid);
    data.WriteString(bundleName);
    int ret = Remote()->SendRequest(IpcStreamMsg::ON_REGISTER_THREAD_PRIORITY, data, reply, option);
    CHECK_AND_RETURN_RET(ret == AUDIO_OK, ret, "failed, ipc error: %{public}d", ret);
    ret = reply.ReadInt32();
    CHECK_AND_RETURN_RET(ret == SUCCESS, ret, "failed, error: %{public}d", ret);
    return ret;
}
} // namespace AudioStandard
} // namespace OHOS
