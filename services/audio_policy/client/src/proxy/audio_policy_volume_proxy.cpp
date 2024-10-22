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
#define LOG_TAG "AudioPolicyProxy"
#endif

#include "audio_policy_proxy.h"

#include "audio_policy_log.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

int32_t AudioPolicyProxy::GetMaxVolumeLevel(AudioVolumeType volumeType)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");

    data.WriteInt32(static_cast<int32_t>(volumeType));
    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_MAX_VOLUMELEVEL), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "get max volume failed, error: %d", error);
    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::GetMinVolumeLevel(AudioVolumeType volumeType)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");
    data.WriteInt32(static_cast<int32_t>(volumeType));
    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_MIN_VOLUMELEVEL), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "get min volume failed, error: %d", error);
    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::SetSystemVolumeLevelLegacy(AudioVolumeType volumeType, int32_t volumeLevel)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");

    data.WriteInt32(static_cast<int32_t>(volumeType));
    data.WriteInt32(volumeLevel);
    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_SYSTEM_VOLUMELEVEL_LEGACY), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "set volume failed, error: %d", error);
    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::SetSystemVolumeLevel(AudioVolumeType volumeType, int32_t volumeLevel, int32_t volumeFlag)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");

    data.WriteInt32(static_cast<int32_t>(volumeType));
    data.WriteInt32(volumeLevel);
    data.WriteInt32(volumeFlag);
    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_SYSTEM_VOLUMELEVEL), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "set volume failed, error: %d", error);
    return reply.ReadInt32();
}

AudioStreamType AudioPolicyProxy::GetSystemActiveVolumeType(const int32_t clientUid)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, STREAM_DEFAULT, "WriteInterfaceToken failed");
    data.WriteInt32(clientUid);
    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_SYSTEM_ACTIVEVOLUME_TYPE), data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_ERR_LOG("get stream in focus failed, error: %d", error);
    }
    return static_cast<AudioStreamType>(reply.ReadInt32());
}

int32_t AudioPolicyProxy::GetSystemVolumeLevel(AudioVolumeType volumeType)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");
    data.WriteInt32(static_cast<int32_t>(volumeType));
    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_SYSTEM_VOLUMELEVEL), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "get volume failed, error: %d", error);
    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::SetLowPowerVolume(int32_t streamId, float volume)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");

    data.WriteInt32(streamId);
    data.WriteFloat(volume);
    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_LOW_POWER_STREM_VOLUME), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "set low power stream volume failed, error: %d", error);
    return reply.ReadInt32();
}

float AudioPolicyProxy::GetLowPowerVolume(int32_t streamId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");
    data.WriteInt32(streamId);
    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_LOW_POWRR_STREM_VOLUME), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "get low power stream volume failed, error: %d", error);
    return reply.ReadFloat();
}

float AudioPolicyProxy::GetSingleStreamVolume(int32_t streamId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");
    data.WriteInt32(streamId);
    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_SINGLE_STREAM_VOLUME), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "get single stream volume failed, error: %d", error);
    return reply.ReadFloat();
}

bool AudioPolicyProxy::IsVolumeUnadjustable()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, false, "WriteInterfaceToken failed");
    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::IS_VOLUME_UNADJUSTABLE), data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_ERR_LOG("isvolumeadjustable failed, error: %d", error);
    }
    return reply.ReadBool();
}

int32_t AudioPolicyProxy::AdjustVolumeByStep(VolumeAdjustType adjustType)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, IPC_PROXY_ERR, "WriteInterfaceToken failed");

    data.WriteInt32(adjustType);

    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::ADJUST_VOLUME_BY_STEP), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, ERR_TRANSACTION_FAILED,
        "GetAudioLatencyFromXml, error: %d", error);

    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::AdjustSystemVolumeByStep(AudioVolumeType volumeType, VolumeAdjustType adjustType)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, IPC_PROXY_ERR, "WriteInterfaceToken failed");
    data.WriteInt32(volumeType);
    data.WriteInt32(adjustType);

    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::ADJUST_SYSTEM_VOLUME_BY_STEP), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, ERR_TRANSACTION_FAILED,
        "GetAudioLatencyFromXml, error: %d", error);

    return reply.ReadInt32();
}

float AudioPolicyProxy::GetSystemVolumeInDb(AudioVolumeType volumeType, int32_t volumeLevel, DeviceType deviceType)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, false, "WriteInterfaceToken failed");

    data.WriteInt32(static_cast<int32_t>(volumeType));
    data.WriteInt32(volumeLevel);
    data.WriteInt32(static_cast<int32_t>(deviceType));

    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_SYSTEM_VOLUME_IN_DB), data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_ERR_LOG("GetSystemVolumeInDb failed, error: %d", error);
    }

    return reply.ReadFloat();
}

int32_t AudioPolicyProxy::GetVolumeGroupInfos(std::string networkId, std::vector<sptr<VolumeGroupInfo>> &infos)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool res = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(res, ERROR, "WriteInterfaceToken failed");

    data.WriteString(networkId);
    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_VOLUME_GROUP_INFO), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "GetVolumeGroupInfo, error: %d", error);

    int32_t ret = reply.ReadInt32();
    if (ret > 0) {
        for (int32_t i = 0; i < ret; i++) {
            infos.push_back(VolumeGroupInfo::Unmarshalling(reply));
        }
        return SUCCESS;
    } else {
        return ret;
    }
}

float AudioPolicyProxy::GetMinStreamVolume()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");
    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_MIN_VOLUME_STREAM), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "get min volume for stream failed, error: %d", error);
    return reply.ReadFloat();
}

float AudioPolicyProxy::GetMaxStreamVolume()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");
    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_MAX_VOLUME_STREAM), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "get max volume for stream failed, error: %d", error);
    return reply.ReadFloat();
}

int32_t AudioPolicyProxy::SetDeviceAbsVolumeSupported(const std::string &macAddress, const bool support)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, ERROR, "WriteInterfaceToken failed");
    data.WriteString(macAddress);
    data.WriteBool(support);

    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_DEVICE_ABSOLUTE_VOLUME_SUPPORTED), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, ERROR,
        "SetDeviceAbsVolumeSupported failed, error: %d", error);
    return reply.ReadInt32();
}

bool AudioPolicyProxy::IsAbsVolumeScene()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, ERROR, "WriteInterfaceToken failed");

    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_ABS_VOLUME_SCENE), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, ERROR, "IsAbsVolumeScene failed, error: %d", error);
    return reply.ReadBool();
}

int32_t AudioPolicyProxy::SetA2dpDeviceVolume(const std::string &macAddress, const int32_t volume,
    const bool updateUi)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, ERROR, "WriteInterfaceToken failed");
    data.WriteString(macAddress);
    data.WriteInt32(volume);
    data.WriteBool(updateUi);

    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_A2DP_DEVICE_VOLUME), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, ERROR, "SetA2dpDeviceVolume failed, error: %d", error);
    return reply.ReadInt32();
}

} // namespace AudioStandard
} // namespace OHOS