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
#define LOG_TAG "AudioPolicyManagerZoneStub"
#endif

#include "audio_policy_manager_stub.h"
#include "audio_errors.h"
#include "audio_policy_log.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {
static constexpr int32_t MAX_SIZE = 1024;

void AudioPolicyManagerStub::OnAudioZoneRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    switch (code) {
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::REGISTER_AUDIO_ZONE_CLIENT):
            HandleRegisterAudioZoneClient(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::CREATE_AUDIO_ZONE):
            HandleCreateAudioZone(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::RELEASE_AUDIO_ZONE):
            HandleReleaseAudioZone(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_ALL_AUDIO_ZONE):
            HandleGetAllAudioZone(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_AUDIO_ZONE_BY_ID):
            HandleGetAudioZone(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::BIND_AUDIO_ZONE_DEVICE):
            HandleBindAudioZoneDevice(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::UNBIND_AUDIO_ZONE_DEVICE):
            HandleUnBindAudioZoneDevice(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::ENABLE_AUDIO_ZONE_REPORT):
            HandleEnableAudioZoneReport(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::ENABLE_AUDIO_ZONE_CHANGE_REPORT):
            HandleEnableAudioZoneChangeReport(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::ADD_UID_TO_AUDIO_ZONE):
            HandleAddUidToAudioZone(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::REMOVE_UID_FROM_AUDIO_ZONE):
            HandleRemoveUidFromAudioZone(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::ADD_STREAM_TO_AUDIO_ZONE):
            HandleAddStreamToAudioZone(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::REMOVE_STREAM_FROM_AUDIO_ZONE):
            HandleRemoveStreamFromAudioZone(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::ENABLE_SYSTEM_VOLUME_PROXY):
            HandleEnableSystemVolumeProxy(data, reply);
            break;
        default:
            OnAudioZoneRemoteRequestExt(code, data, reply, option);
            break;
    }
}

void AudioPolicyManagerStub::OnAudioZoneRemoteRequestExt(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    switch (code) {
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_AUDIO_INTERRUPT_FOR_ZONE):
            HandleGetAudioInterruptForZone(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_AUDIO_INTERRUPT_OF_DEVICE_FOR_ZONE):
            HandleGetAudioInterruptForZoneDevice(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::ENABLE_AUDIO_ZONE_INTERRUPT_REPORT):
            HandleEnableAudioZoneInterruptReport(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::INJECT_INTERRUPT_TO_AUDIO_ZONE):
            HandleInjectInterruptToAudioZone(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::INJECT_INTERRUPT_OF_DEVICE_TO_AUDIO_ZONE):
            HandleInjectInterruptToAudioZoneDevice(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_ZONE_DEVICE_VISIBLE):
            HandleSetZoneDeviceVisible(data, reply);
            break;
        default:
            break;
    }
}
void AudioPolicyManagerStub::HandleRegisterAudioZoneClient(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> object = data.ReadRemoteObject();
    CHECK_AND_RETURN_LOG(object != nullptr, "object is nullptr");
    reply.WriteInt32(RegisterAudioZoneClient(object));
}

void AudioPolicyManagerStub::HandleCreateAudioZone(MessageParcel &data, MessageParcel &reply)
{
    std::string name = data.ReadString();
    AudioZoneContext context;
    context.Unmarshalling(data);
    CHECK_AND_RETURN_LOG(!name.empty(), "audio zone name is empty");
    reply.WriteInt32(CreateAudioZone(name, context));
}

void AudioPolicyManagerStub::HandleReleaseAudioZone(MessageParcel &data, MessageParcel &reply)
{
    int32_t zoneId = data.ReadInt32();
    CHECK_AND_RETURN_LOG(zoneId > 0, "audio zone id is invalid");
    ReleaseAudioZone(zoneId);
}

void AudioPolicyManagerStub::HandleGetAllAudioZone(MessageParcel &data, MessageParcel &reply)
{
    auto zoneDescriptors = GetAllAudioZone();
    reply.WriteInt32(SUCCESS);
    int32_t size = static_cast<int32_t>(zoneDescriptors.size());
    reply.WriteInt32(size);
    for (int i = 0; i < size; i++) {
        zoneDescriptors[i]->Marshalling(reply);
    }
}

void AudioPolicyManagerStub::HandleGetAudioZone(MessageParcel &data, MessageParcel &reply)
{
    int32_t zoneId = data.ReadInt32();
    CHECK_AND_RETURN_LOG(zoneId > 0, "audio zone id is invalid");
    auto zoneDescriptor = GetAudioZone(zoneId);
    if (zoneDescriptor == nullptr) {
        reply.WriteInt32(ERR_NULL_POINTER);
    } else {
        reply.WriteInt32(SUCCESS);
        zoneDescriptor->Marshalling(reply);
    }
}

void AudioPolicyManagerStub::HandleBindAudioZoneDevice(MessageParcel &data, MessageParcel &reply)
{
    int32_t zoneId = data.ReadInt32();
    CHECK_AND_RETURN_LOG(zoneId > 0, "audio zone id is invalid");
    int32_t size = data.ReadInt32();
    CHECK_AND_RETURN_LOG(size > 0 && size < MAX_SIZE, "invalid device size: %{public}d", size);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    for (int32_t i = 0; i < size; i++) {
        auto device = AudioDeviceDescriptor::UnmarshallingPtr(data);
        devices.emplace_back(device);
    }
    reply.WriteInt32(BindDeviceToAudioZone(zoneId, devices));
}

void AudioPolicyManagerStub::HandleUnBindAudioZoneDevice(MessageParcel &data, MessageParcel &reply)
{
    int32_t zoneId = data.ReadInt32();
    CHECK_AND_RETURN_LOG(zoneId > 0, "audio zone id is invalid");
    int32_t size = data.ReadInt32();
    CHECK_AND_RETURN_LOG(size > 0 && size < MAX_SIZE, "invalid device size: %{public}d", size);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    for (int32_t i = 0; i < size; i++) {
        auto device = AudioDeviceDescriptor::UnmarshallingPtr(data);
        devices.emplace_back(device);
    }
    reply.WriteInt32(UnBindDeviceToAudioZone(zoneId, devices));
}

void AudioPolicyManagerStub::HandleEnableAudioZoneReport(MessageParcel &data, MessageParcel &reply)
{
    reply.WriteInt32(EnableAudioZoneReport(data.ReadBool()));
}

void AudioPolicyManagerStub::HandleEnableAudioZoneChangeReport(MessageParcel &data, MessageParcel &reply)
{
    int32_t zoneId = data.ReadInt32();
    CHECK_AND_RETURN_LOG(zoneId > 0, "audio zone id is invalid");
    bool enable = data.ReadBool();
    reply.WriteInt32(EnableAudioZoneChangeReport(zoneId, enable));
}

void AudioPolicyManagerStub::HandleAddUidToAudioZone(MessageParcel &data, MessageParcel &reply)
{
    int32_t zoneId = data.ReadInt32();
    CHECK_AND_RETURN_LOG(zoneId > 0, "audio zone id is invalid");
    int32_t uid = data.ReadInt32();
    reply.WriteInt32(AddUidToAudioZone(zoneId, uid));
}

void AudioPolicyManagerStub::HandleRemoveUidFromAudioZone(MessageParcel &data, MessageParcel &reply)
{
    int32_t zoneId = data.ReadInt32();
    CHECK_AND_RETURN_LOG(zoneId > 0, "audio zone id is invalid");
    int32_t uid = data.ReadInt32();
    reply.WriteInt32(RemoveUidFromAudioZone(zoneId, uid));
}

void AudioPolicyManagerStub::HandleAddStreamToAudioZone(MessageParcel &data, MessageParcel &reply)
{
    int32_t zoneId = data.ReadInt32();
    CHECK_AND_RETURN_LOG(zoneId > 0, "audio zone id is invalid");
    AudioZoneStream stream;
    stream.Unmarshalling(data);
    reply.WriteInt32(AddStreamToAudioZone(zoneId, stream));
}

void AudioPolicyManagerStub::HandleRemoveStreamFromAudioZone(MessageParcel &data, MessageParcel &reply)
{
    int32_t zoneId = data.ReadInt32();
    CHECK_AND_RETURN_LOG(zoneId > 0, "audio zone id is invalid");
    AudioZoneStream stream;
    stream.Unmarshalling(data);
    reply.WriteInt32(RemoveStreamFromAudioZone(zoneId, stream));
}

void AudioPolicyManagerStub::HandleSetZoneDeviceVisible(MessageParcel &data, MessageParcel &reply)
{
    bool visible = data.ReadBool();
    SetZoneDeviceVisible(visible);
}

void AudioPolicyManagerStub::HandleEnableSystemVolumeProxy(MessageParcel &data, MessageParcel &reply)
{
    int32_t zoneId = data.ReadInt32();
    CHECK_AND_RETURN_LOG(zoneId > 0, "audio zone id is invalid");
    bool enable = data.ReadBool();
    reply.WriteInt32(EnableSystemVolumeProxy(zoneId, enable));
}

void AudioPolicyManagerStub::HandleGetAudioInterruptForZone(MessageParcel &data, MessageParcel &reply)
{
    int32_t zoneId = data.ReadInt32();
    CHECK_AND_RETURN_LOG(zoneId > 0, "audio zone id is invalid");
    auto interrupts = GetAudioInterruptForZone(zoneId);
    reply.WriteInt32(SUCCESS);
    reply.WriteInt32(static_cast<int32_t>(interrupts.size()));
    for (const auto &it : interrupts) {
        AudioInterrupt::Marshalling(reply, it.first);
        reply.WriteInt32(static_cast<int32_t>(it.second));
    }
}

void AudioPolicyManagerStub::HandleGetAudioInterruptForZoneDevice(MessageParcel &data, MessageParcel &reply)
{
    int32_t zoneId = data.ReadInt32();
    CHECK_AND_RETURN_LOG(zoneId > 0, "audio zone id is invalid");
    std::string deviceTag = data.ReadString();
    auto interrupts = GetAudioInterruptForZone(zoneId, deviceTag);
    reply.WriteInt32(SUCCESS);
    reply.WriteInt32(static_cast<int32_t>(interrupts.size()));
    for (const auto &it : interrupts) {
        AudioInterrupt::Marshalling(reply, it.first);
        reply.WriteInt32(static_cast<int32_t>(it.second));
    }
}

void AudioPolicyManagerStub::HandleEnableAudioZoneInterruptReport(MessageParcel &data, MessageParcel &reply)
{
    int32_t zoneId = data.ReadInt32();
    CHECK_AND_RETURN_LOG(zoneId > 0, "audio zone id is invalid");
    std::string deviceTag = data.ReadString();
    bool enable = data.ReadBool();
    reply.WriteInt32(EnableAudioZoneInterruptReport(zoneId, deviceTag, enable));
}

void AudioPolicyManagerStub::HandleInjectInterruptToAudioZone(MessageParcel &data, MessageParcel &reply)
{
    int32_t zoneId = data.ReadInt32();
    CHECK_AND_RETURN_LOG(zoneId > 0, "audio zone id is invalid");
    int32_t size = data.ReadInt32();
    CHECK_AND_RETURN_LOG(size > 0 && size < MAX_SIZE, "invalid interrupt size: %{public}d", size);
    std::list<std::pair<AudioInterrupt, AudioFocuState>> interrupts;
    for (int32_t i = 0; i < size; i++) {
        AudioInterrupt temp;
        AudioInterrupt::Unmarshalling(data, temp);
        AudioFocuState state = static_cast<AudioFocuState>(data.ReadInt32());
        interrupts.emplace_back(std::make_pair(temp, state));
    }
    reply.WriteInt32(InjectInterruptToAudioZone(zoneId, interrupts));
}

void AudioPolicyManagerStub::HandleInjectInterruptToAudioZoneDevice(MessageParcel &data, MessageParcel &reply)
{
    int32_t zoneId = data.ReadInt32();
    CHECK_AND_RETURN_LOG(zoneId > 0, "audio zone id is invalid");
    std::string deviceTag = data.ReadString();
    int32_t size = data.ReadInt32();
    CHECK_AND_RETURN_LOG(size > 0 && size < MAX_SIZE, "invalid interrupt size: %{public}d", size);
    std::list<std::pair<AudioInterrupt, AudioFocuState>> interrupts;
    for (int32_t i = 0; i < size; i++) {
        AudioInterrupt temp;
        AudioInterrupt::Unmarshalling(data, temp);
        AudioFocuState state = static_cast<AudioFocuState>(data.ReadInt32());
        interrupts.emplace_back(std::make_pair(temp, state));
    }
    reply.WriteInt32(InjectInterruptToAudioZone(zoneId, deviceTag, interrupts));
}
} // namespace AudioStandard
} // namespace OHOS