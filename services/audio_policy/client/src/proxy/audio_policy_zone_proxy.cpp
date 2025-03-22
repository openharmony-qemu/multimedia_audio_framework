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
#define LOG_TAG "AudioPolicyZoneProxy"
#endif

#include "audio_policy_log.h"
#include "audio_policy_proxy.h"

namespace OHOS {
namespace AudioStandard {
int32_t AudioPolicyProxy::RegisterAudioZoneClient(const sptr<IRemoteObject>& object)
{
    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_NULL_OBJECT, "object is nullptr");
    
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "WriteInterfaceToken failed");

    (void)data.WriteRemoteObject(object);
    int32_t error = Remote()->SendRequest(
        static_cast<int32_t>(AudioPolicyInterfaceCode::REGISTER_AUDIO_ZONE_CLIENT), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "SendRequest failed, error: %{public}d", error);
    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::CreateAudioZone(const std::string &name, const AudioZoneContext &context)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "WriteInterfaceToken failed");

    data.WriteString(name);
    context.Marshalling(data);
    int32_t error = Remote()->SendRequest(
        static_cast<int32_t>(AudioPolicyInterfaceCode::CREATE_AUDIO_ZONE), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "SendRequest failed, error: %{public}d", error);
    return reply.ReadInt32();
}

void AudioPolicyProxy::ReleaseAudioZone(int32_t zoneId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    CHECK_AND_RETURN_LOG(data.WriteInterfaceToken(GetDescriptor()), "WriteInterfaceToken failed");

    data.WriteInt32(zoneId);
    int32_t error = Remote()->SendRequest(
        static_cast<int32_t>(AudioPolicyInterfaceCode::RELEASE_AUDIO_ZONE), data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_ERR_LOG("ReleaseAudioZone failed, error: %{public}d", error);
    }
}

const std::vector<sptr<AudioZoneDescriptor>> AudioPolicyProxy::GetAllAudioZone()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    std::vector<sptr<AudioZoneDescriptor>> zoneDescriptor;
    
    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), zoneDescriptor,
        "WriteInterfaceToken failed");
    int32_t error = Remote()->SendRequest(
        static_cast<int32_t>(AudioPolicyInterfaceCode::GET_ALL_AUDIO_ZONE), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, zoneDescriptor, "SendRequest failed, error: %{public}d", error);
    
    int32_t retCode = reply.ReadInt32();
    CHECK_AND_RETURN_RET_LOG(retCode == ERR_NONE, zoneDescriptor, "GetAllAudioZone failed, error: %{public}d", retCode);

    int32_t zoneSize = reply.ReadInt32();
    for (int32_t i = 0; i < zoneSize; i++) {
        zoneDescriptor.push_back(AudioZoneDescriptor::UnmarshallingPtr(reply));
    }
    return zoneDescriptor;
}

const sptr<AudioZoneDescriptor> AudioPolicyProxy::GetAudioZone(int32_t zoneId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), nullptr, "WriteInterfaceToken failed");

    data.WriteInt32(zoneId);
    int32_t error = Remote()->SendRequest(
        static_cast<int32_t>(AudioPolicyInterfaceCode::GET_AUDIO_ZONE_BY_ID), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, nullptr, "SendRequest failed, error: %{public}d", error);

    int32_t retCode = reply.ReadInt32();
    CHECK_AND_RETURN_RET_LOG(retCode == ERR_NONE, nullptr, "GetAudioZone failed, error: %{public}d", retCode);
    return AudioZoneDescriptor::UnmarshallingPtr(reply);
}

int32_t AudioPolicyProxy::BindDeviceToAudioZone(int32_t zoneId, std::vector<sptr<AudioDeviceDescriptor>> devices)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "WriteInterfaceToken failed");
    data.WriteInt32(zoneId);
    data.WriteInt32(static_cast<int32_t>(devices.size()));
    for (auto &device : devices) {
        bool ret = device->Marshalling(data);
        CHECK_AND_RETURN_RET_LOG(ret, ERROR, "Marshalling device failed");
    }
    int32_t error = Remote()->SendRequest(
        static_cast<int32_t>(AudioPolicyInterfaceCode::BIND_AUDIO_ZONE_DEVICE), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "SendRequest failed, error: %{public}d", error);
    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::UnBindDeviceToAudioZone(int32_t zoneId, std::vector<sptr<AudioDeviceDescriptor>> devices)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "WriteInterfaceToken failed");
    data.WriteInt32(zoneId);
    data.WriteInt32(static_cast<int32_t>(devices.size()));
    for (auto &device : devices) {
        bool ret = device->Marshalling(data);
        CHECK_AND_RETURN_RET_LOG(ret, ERROR, "Marshalling device failed");
    }
    int32_t error = Remote()->SendRequest(
        static_cast<int32_t>(AudioPolicyInterfaceCode::UNBIND_AUDIO_ZONE_DEVICE), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "SendRequest failed, error: %{public}d", error);
    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::EnableAudioZoneReport(bool enable)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "WriteInterfaceToken failed");
    
    (void)data.WriteBool(enable);
    int32_t error = Remote()->SendRequest(
        static_cast<int32_t>(AudioPolicyInterfaceCode::ENABLE_AUDIO_ZONE_REPORT), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "SendRequest failed, error: %{public}d", error);
    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::EnableAudioZoneChangeReport(int32_t zoneId, bool enable)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "WriteInterfaceToken failed");

    (void)data.WriteInt32(zoneId);
    (void)data.WriteBool(enable);
    int32_t error = Remote()->SendRequest(
        static_cast<int32_t>(AudioPolicyInterfaceCode::ENABLE_AUDIO_ZONE_CHANGE_REPORT), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "SendRequest failed, error: %{public}d", error);
    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::AddUidToAudioZone(int32_t zoneId, int32_t uid)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "WriteInterfaceToken failed");

    (void)data.WriteInt32(zoneId);
    (void)data.WriteInt32(uid);
    int32_t error = Remote()->SendRequest(
        static_cast<int32_t>(AudioPolicyInterfaceCode::ADD_UID_TO_AUDIO_ZONE), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "SendRequest failed, error: %{public}d", error);
    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::RemoveUidFromAudioZone(int32_t zoneId, int32_t uid)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "WriteInterfaceToken failed");

    (void)data.WriteInt32(zoneId);
    (void)data.WriteInt32(uid);
    int32_t error = Remote()->SendRequest(
        static_cast<int32_t>(AudioPolicyInterfaceCode::REMOVE_UID_FROM_AUDIO_ZONE), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "SendRequest failed, error: %{public}d", error);
    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::EnableSystemVolumeProxy(int32_t zoneId, bool enable)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "WriteInterfaceToken failed");

    (void)data.WriteInt32(zoneId);
    (void)data.WriteBool(enable);
    int32_t error = Remote()->SendRequest(
        static_cast<int32_t>(AudioPolicyInterfaceCode::ENABLE_SYSTEM_VOLUME_PROXY), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "SendRequest failed, error: %{public}d", error);
    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::SetSystemVolumeLevelForZone(const int32_t zoneId, const AudioVolumeType volumeType,
    const int32_t volumeLevel, const int32_t volumeFlag)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "WriteInterfaceToken failed");

    (void)data.WriteInt32(zoneId);
    (void)data.WriteInt32(static_cast<int32_t>(volumeType));
    (void)data.WriteInt32(volumeLevel);
    (void)data.WriteInt32(volumeFlag);
    int32_t error = Remote()->SendRequest(
        static_cast<int32_t>(AudioPolicyInterfaceCode::SET_SYSTEM_VOLUME_LEVEL_FOR_ZONE), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "SendRequest failed, error: %{public}d", error);
    return reply.ReadInt32();
}


int32_t AudioPolicyProxy::GetSystemVolumeLevelForZone(int32_t zoneId, AudioVolumeType volumeType)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "WriteInterfaceToken failed");

    (void)data.WriteInt32(zoneId);
    (void)data.WriteInt32(static_cast<int32_t>(volumeType));
    int32_t error = Remote()->SendRequest(
        static_cast<int32_t>(AudioPolicyInterfaceCode::GET_SYSTEM_VOLUME_LEVEL_FOR_ZONE), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, -1, "SendRequest failed, error: %{public}d", error);
    int32_t retCode = reply.ReadInt32();
    CHECK_AND_RETURN_RET_LOG(retCode == ERR_NONE, -1, "GetSystemVolumeLevelForZone failed, error: %{public}d", retCode);
    return reply.ReadInt32();
}

std::list<std::pair<AudioInterrupt, AudioFocuState>> AudioPolicyProxy::GetAudioInterruptForZone(
    int32_t zoneId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    std::list<std::pair<AudioInterrupt, AudioFocuState>> interrupts;
    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), interrupts, "WriteInterfaceToken failed");

    (void)data.WriteInt32(zoneId);
    int32_t error = Remote()->SendRequest(
        static_cast<int32_t>(AudioPolicyInterfaceCode::GET_AUDIO_INTERRUPT_FOR_ZONE), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, interrupts, "SendRequest failed, error: %{public}d", error);

    int32_t retCode = reply.ReadInt32();
    CHECK_AND_RETURN_RET_LOG(retCode == ERR_NONE, interrupts,
        "GetAudioInterruptForZone failed, error: %{public}d", retCode);
    int32_t size = reply.ReadInt32();
    for (int i = 0; i < size; i++) {
        AudioInterrupt temp;
        AudioInterrupt::Unmarshalling(reply, temp);
        AudioFocuState state = static_cast<AudioFocuState>(reply.ReadInt32());
        interrupts.emplace_back(std::make_pair(temp, state));
    }
    return interrupts;
}

std::list<std::pair<AudioInterrupt, AudioFocuState>> AudioPolicyProxy::GetAudioInterruptForZone(
    int32_t zoneId, int32_t deviceId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    std::list<std::pair<AudioInterrupt, AudioFocuState>> interrupts;
    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), interrupts, "WriteInterfaceToken failed");

    (void)data.WriteInt32(zoneId);
    (void)data.WriteInt32(deviceId);
    int32_t error = Remote()->SendRequest(
        static_cast<int32_t>(AudioPolicyInterfaceCode::GET_AUDIO_INTERRUPT_OF_DEVICE_FOR_ZONE), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, interrupts, "SendRequest failed, error: %{public}d", error);
    
    int32_t retCode = reply.ReadInt32();
    CHECK_AND_RETURN_RET_LOG(retCode == ERR_NONE, interrupts,
        "GetAudioInterruptForZone failed, error: %{public}d", retCode);
    int32_t size = reply.ReadInt32();
    for (int i = 0; i < size; i++) {
        AudioInterrupt temp;
        AudioInterrupt::Unmarshalling(reply, temp);
        AudioFocuState state = static_cast<AudioFocuState>(reply.ReadInt32());
        interrupts.emplace_back(std::make_pair(temp, state));
    }
    return interrupts;
}

int32_t AudioPolicyProxy::EnableAudioZoneInterruptReport(int32_t zoneId, int32_t deviceId, bool enable)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "WriteInterfaceToken failed");

    (void)data.WriteInt32(zoneId);
    (void)data.WriteInt32(deviceId);
    (void)data.WriteBool(enable);
    int32_t error = Remote()->SendRequest(
        static_cast<int32_t>(AudioPolicyInterfaceCode::ENABLE_AUDIO_ZONE_INTERRUPT_REPORT), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "SendRequest failed, error: %{public}d", error);
    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::InjectInterruptToAudioZone(int32_t zoneId,
    const std::list<std::pair<AudioInterrupt, AudioFocuState>> &interrupts)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "WriteInterfaceToken failed");

    (void)data.WriteInt32(zoneId);
    (void)data.WriteInt32(static_cast<int32_t>(interrupts.size()));
    for (auto &it : interrupts) {
        AudioInterrupt::Marshalling(data, it.first);
        (void)data.WriteInt32(static_cast<int32_t>(it.second));
    }
    int32_t error = Remote()->SendRequest(
        static_cast<int32_t>(AudioPolicyInterfaceCode::INJECT_INTERRUPT_TO_AUDIO_ZONE), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "SendRequest failed, error: %{public}d", error);
    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::InjectInterruptToAudioZone(int32_t zoneId, int32_t deviceId,
    const std::list<std::pair<AudioInterrupt, AudioFocuState>> &interrupts)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "WriteInterfaceToken failed");

    (void)data.WriteInt32(zoneId);
    (void)data.WriteInt32(deviceId);
    (void)data.WriteInt32(static_cast<int32_t>(interrupts.size()));
    for (auto &it : interrupts) {
        AudioInterrupt::Marshalling(data, it.first);
        (void)data.WriteInt32(static_cast<int32_t>(it.second));
    }
    int32_t error = Remote()->SendRequest(
        static_cast<int32_t>(AudioPolicyInterfaceCode::INJECT_INTERRUPT_OF_DEVICE_TO_AUDIO_ZONE), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "SendRequest failed, error: %{public}d", error);
    return reply.ReadInt32();
}
} // namespace AudioStandard
} // namespace OHOS