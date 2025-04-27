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
#undef LOG_TAG
#define LOG_TAG "AudioZoneClientProxy"

#include "audio_zone_client_proxy.h"
#include "audio_log.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
AudioZoneClientProxy::AudioZoneClientProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IStandardAudioZoneClient>(impl)
{}

AudioZoneClientProxy::~AudioZoneClientProxy()
{}

void AudioZoneClientProxy::OnAudioZoneAdd(const AudioZoneDescriptor &zoneDescriptor)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    CHECK_AND_RETURN_LOG(data.WriteInterfaceToken(GetDescriptor()), "write interface token failed");
    
    zoneDescriptor.Marshalling(data);
    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioZoneClientCode::ON_AUDIO_ZONE_ADD), data, reply, option);
    CHECK_AND_RETURN_LOG(error == ERR_NONE, "send request failed, error: %{public}d", error);
}

void AudioZoneClientProxy::OnAudioZoneRemove(int32_t zoneId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    CHECK_AND_RETURN_LOG(data.WriteInterfaceToken(GetDescriptor()), "write interface token failed");

    data.WriteInt32(zoneId);
    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioZoneClientCode::ON_AUDIO_ZONE_REMOVE), data, reply, option);
    CHECK_AND_RETURN_LOG(error == ERR_NONE, "send request failed, error: %{public}d", error);
}

void AudioZoneClientProxy::OnAudioZoneChange(int32_t zoneId, const AudioZoneDescriptor &zoneDescriptor,
    AudioZoneChangeReason reason)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    CHECK_AND_RETURN_LOG(data.WriteInterfaceToken(GetDescriptor()), "write interface token failed");

    data.WriteInt32(zoneId);
    zoneDescriptor.Marshalling(data);
    data.WriteInt32(static_cast<int32_t>(reason));
    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioZoneClientCode::ON_AUDIO_ZONE_CHANGE), data, reply, option);
    CHECK_AND_RETURN_LOG(error == ERR_NONE, "send request failed, error: %{public}d", error);
}

void AudioZoneClientProxy::OnInterruptEvent(int32_t zoneId,
    const std::list<std::pair<AudioInterrupt, AudioFocuState>> &interrupts,
    AudioZoneInterruptReason reason)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    CHECK_AND_RETURN_LOG(data.WriteInterfaceToken(GetDescriptor()), "write interface token failed");

    data.WriteInt32(zoneId);
    data.WriteInt32(static_cast<int32_t>(interrupts.size()));
    for (const auto &it : interrupts) {
        AudioInterrupt::Marshalling(data, it.first);
        data.WriteInt32(static_cast<int32_t>(it.second));
    }
    data.WriteInt32(static_cast<int32_t>(reason));
    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioZoneClientCode::ON_AUDIO_ZONE_INTERRUPT), data, reply, option);
    CHECK_AND_RETURN_LOG(error == ERR_NONE, "send request failed, error: %{public}d", error);
}

void AudioZoneClientProxy::OnInterruptEvent(int32_t zoneId, const std::string &deviceTag,
    const std::list<std::pair<AudioInterrupt, AudioFocuState>> &interrupts,
    AudioZoneInterruptReason reason)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    CHECK_AND_RETURN_LOG(data.WriteInterfaceToken(GetDescriptor()), "write interface token failed");

    data.WriteInt32(zoneId);
    data.WriteString(deviceTag);
    data.WriteInt32(static_cast<int32_t>(interrupts.size()));
    for (const auto &it : interrupts) {
        AudioInterrupt::Marshalling(data, it.first);
        data.WriteInt32(static_cast<int32_t>(it.second));
    }
    data.WriteInt32(static_cast<int32_t>(reason));
    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioZoneClientCode::ON_AUDIO_ZONE_DEVICE_INTERRUPT), data, reply, option);
    CHECK_AND_RETURN_LOG(error == ERR_NONE, "send request failed, error: %{public}d", error);
}

int32_t AudioZoneClientProxy::SetSystemVolume(const int32_t zoneId, const AudioVolumeType volumeType,
    const int32_t volumeLevel, const int32_t volumeFlag)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), -1, "write interface token failed");

    data.WriteInt32(zoneId);
    data.WriteInt32(static_cast<int32_t>(volumeType));
    data.WriteInt32(volumeLevel);
    data.WriteInt32(volumeFlag);
    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioZoneClientCode::ON_AUDIO_ZONE_SYSTEM_VOLUME_SET), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, -1, "send request failed, error: %{public}d", error);
    return reply.ReadInt32();
}

int32_t AudioZoneClientProxy::GetSystemVolume(int32_t zoneId, AudioVolumeType volumeType)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), -1, "write interface token failed");

    data.WriteInt32(zoneId);
    data.WriteInt32(static_cast<int32_t>(volumeType));
    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioZoneClientCode::ON_AUDIO_ZONE_SYSTEM_VOLUME_GET), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, -1, "send request failed, error: %{public}d", error);
    return reply.ReadInt32();
}
} // namespace AudioStandard
} // namespace OHOS