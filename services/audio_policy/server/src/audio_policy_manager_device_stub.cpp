/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioPolicyManagerStub"
#endif

#include "audio_policy_manager_stub.h"

#include "audio_errors.h"
#include "audio_policy_log.h"

namespace OHOS {
namespace AudioStandard {

void AudioPolicyManagerStub::GetDevicesInternal(MessageParcel &data, MessageParcel &reply)
{
    int deviceFlag = data.ReadInt32();
    DeviceFlag deviceFlagConfig = static_cast<DeviceFlag>(deviceFlag);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices = GetDevices(deviceFlagConfig);
    int32_t size = static_cast<int32_t>(devices.size());
    reply.WriteInt32(size);
    for (int i = 0; i < size; i++) {
        devices[i]->Marshalling(reply);
    }
}

void AudioPolicyManagerStub::GetDevicesInnerInternal(MessageParcel &data, MessageParcel &reply)
{
    int deviceFlag = data.ReadInt32();
    DeviceFlag deviceFlagConfig = static_cast<DeviceFlag>(deviceFlag);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices = GetDevicesInner(deviceFlagConfig);
    int32_t size = static_cast<int32_t>(devices.size());
    reply.WriteInt32(size);
    for (int i = 0; i < size; i++) {
        devices[i]->Marshalling(reply);
    }
}

void AudioPolicyManagerStub::GetPreferredOutputDeviceDescriptorsInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioRendererInfo rendererInfo;
    rendererInfo.Unmarshalling(data);
    bool forceNoBTPermission = data.ReadBool();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices =
        GetPreferredOutputDeviceDescriptors(rendererInfo, forceNoBTPermission);
    int32_t size = static_cast<int32_t>(devices.size());
    reply.WriteInt32(size);
    for (int i = 0; i < size; i++) {
        devices[i]->Marshalling(reply);
    }
}

void AudioPolicyManagerStub::GetPreferredInputDeviceDescriptorsInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioCapturerInfo captureInfo;
    captureInfo.Unmarshalling(data);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices = GetPreferredInputDeviceDescriptors(captureInfo);
    uint32_t size = static_cast<uint32_t>(devices.size());
    reply.WriteInt32(size);
    for (uint32_t i = 0; i < size; i++) {
        devices[i]->Marshalling(reply);
    }
}

void AudioPolicyManagerStub::SetDeviceActiveInternal(MessageParcel &data, MessageParcel &reply)
{
    InternalDeviceType deviceType = static_cast<InternalDeviceType>(data.ReadInt32());
    bool active = data.ReadBool();
    int32_t uid = data.ReadInt32();
    int32_t result = SetDeviceActive(deviceType, active, uid);
    if (result == SUCCESS)
        reply.WriteInt32(AUDIO_OK);
    else
        reply.WriteInt32(AUDIO_ERR);
}

void AudioPolicyManagerStub::IsDeviceActiveInternal(MessageParcel &data, MessageParcel &reply)
{
    InternalDeviceType deviceType = static_cast<InternalDeviceType>(data.ReadInt32());
    bool result = IsDeviceActive(deviceType);
    reply.WriteBool(result);
}

void AudioPolicyManagerStub::GetActiveOutputDeviceInternal(MessageParcel &data, MessageParcel &reply)
{
    InternalDeviceType deviceType = GetActiveOutputDevice();
    reply.WriteInt32(static_cast<int>(deviceType));
}

void AudioPolicyManagerStub::GetActiveInputDeviceInternal(MessageParcel &data, MessageParcel &reply)
{
    InternalDeviceType deviceType = GetActiveInputDevice();
    reply.WriteInt32(static_cast<int>(deviceType));
}

void AudioPolicyManagerStub::SelectOutputDeviceInternal(MessageParcel &data, MessageParcel &reply)
{
    sptr<AudioRendererFilter> audioRendererFilter = AudioRendererFilter::Unmarshalling(data);
    CHECK_AND_RETURN_LOG(audioRendererFilter != nullptr, "AudioRendererFilter unmarshall fail.");

    int validSize = 20; // Use 20 as limit.
    int size = data.ReadInt32();
    if (size <= 0 || size > validSize) {
        AUDIO_ERR_LOG("SelectOutputDevice get invalid device size.");
        return;
    }
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> targetOutputDevice;
    for (int i = 0; i < size; i++) {
        std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = AudioDeviceDescriptor::UnmarshallingPtr(data);
        CHECK_AND_RETURN_LOG(audioDeviceDescriptor != nullptr, "Unmarshalling fail.");
        MapExternalToInternalDeviceType(*audioDeviceDescriptor);
        targetOutputDevice.push_back(audioDeviceDescriptor);
    }

    int32_t ret = SelectOutputDevice(audioRendererFilter, targetOutputDevice);
    reply.WriteInt32(ret);
}

void AudioPolicyManagerStub::GetSelectedDeviceInfoInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t uid = data.ReadInt32();
    int32_t pid = data.ReadInt32();
    AudioStreamType streamType =  static_cast<AudioStreamType>(data.ReadInt32());

    std::string deviceName = GetSelectedDeviceInfo(uid, pid, streamType);
    reply.WriteString(deviceName);
}

void AudioPolicyManagerStub::SelectInputDeviceInternal(MessageParcel &data, MessageParcel &reply)
{
    sptr<AudioCapturerFilter> audioCapturerFilter = AudioCapturerFilter::Unmarshalling(data);
    CHECK_AND_RETURN_LOG(audioCapturerFilter != nullptr, "AudioCapturerFilter unmarshall fail.");

    int validSize = 10; // Use 10 as limit.
    int size = data.ReadInt32();
    CHECK_AND_RETURN_LOG(size > 0 && size <= validSize, "SelectInputDevice get invalid device size.");
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> targetInputDevice;
    for (int i = 0; i < size; i++) {
        std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = AudioDeviceDescriptor::UnmarshallingPtr(data);
        CHECK_AND_RETURN_LOG(audioDeviceDescriptor != nullptr, "Unmarshalling fail.");
        MapExternalToInternalDeviceType(*audioDeviceDescriptor);
        targetInputDevice.push_back(audioDeviceDescriptor);
    }

    int32_t ret = SelectInputDevice(audioCapturerFilter, targetInputDevice);
    reply.WriteInt32(ret);
}

void AudioPolicyManagerStub::ExcludeOutputDevicesInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioDeviceUsage audioDevUsage = static_cast<AudioDeviceUsage>(data.ReadInt32());
    int validSize = 20; // Use 20 as limit.
    int size = data.ReadInt32();
    CHECK_AND_RETURN_LOG(size > 0 && size <= validSize, "ExcludeOutputDevices get invalid device size.");
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors;
    for (int i = 0; i < size; i++) {
        std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = AudioDeviceDescriptor::UnmarshallingPtr(data);
        CHECK_AND_RETURN_LOG(audioDeviceDescriptor != nullptr, "Unmarshalling fail.");
        audioDeviceDescriptors.push_back(audioDeviceDescriptor);
    }

    int32_t ret = ExcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
    reply.WriteInt32(ret);
}

void AudioPolicyManagerStub::UnexcludeOutputDevicesInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioDeviceUsage audioDevUsage = static_cast<AudioDeviceUsage>(data.ReadInt32());
    int validSize = 20; // Use 20 as limit.
    int size = data.ReadInt32();
    CHECK_AND_RETURN_LOG(size > 0 && size <= validSize, "UnexcludeOutputDevices get invalid device size.");
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors;
    for (int i = 0; i < size; i++) {
        std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = AudioDeviceDescriptor::UnmarshallingPtr(data);
        CHECK_AND_RETURN_LOG(audioDeviceDescriptor != nullptr, "Unmarshalling fail.");
        audioDeviceDescriptors.push_back(audioDeviceDescriptor);
    }

    int32_t ret = UnexcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
    reply.WriteInt32(ret);
}

void AudioPolicyManagerStub::GetExcludedDevicesInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioDeviceUsage audioDevUsage = static_cast<AudioDeviceUsage>(data.ReadInt32());
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices = GetExcludedDevices(audioDevUsage);
    int32_t size = static_cast<int32_t>(devices.size());
    reply.WriteInt32(size);
    for (int i = 0; i < size; i++) {
        devices[i]->Marshalling(reply);
    }
}

void AudioPolicyManagerStub::GetAvailableDevicesInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioDeviceUsage usage  = static_cast<AudioDeviceUsage>(data.ReadInt32());
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs = GetAvailableDevices(usage);
    int32_t size = static_cast<int32_t>(descs.size());
    reply.WriteInt32(size);
    for (int32_t i = 0; i < size; i++) {
        descs[i]->Marshalling(reply);
    }
}

void AudioPolicyManagerStub::SetAvailableDeviceChangeCallbackInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t clientId = data.ReadInt32();
    AudioDeviceUsage usage = static_cast<AudioDeviceUsage>(data.ReadInt32());
    sptr<IRemoteObject> object = data.ReadRemoteObject();
    CHECK_AND_RETURN_LOG(object != nullptr, "AudioInterruptCallback obj is null");
    int32_t result = SetAvailableDeviceChangeCallback(clientId, usage, object);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::UnsetAvailableDeviceChangeCallbackInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t clientId = data.ReadInt32();
    AudioDeviceUsage usage = static_cast<AudioDeviceUsage>(data.ReadInt32());
    int32_t result = UnsetAvailableDeviceChangeCallback(clientId, usage);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::SetCallDeviceActiveInternal(MessageParcel &data, MessageParcel &reply)
{
    InternalDeviceType deviceType = static_cast<InternalDeviceType>(data.ReadInt32());
    bool active = data.ReadBool();
    std::string address = data.ReadString();
    int32_t uid = data.ReadInt32();
    int32_t result = SetCallDeviceActive(deviceType, active, address, uid);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::GetActiveBluetoothDeviceInternal(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = GetActiveBluetoothDevice();
    desc->Marshalling(reply);
}

void AudioPolicyManagerStub::FetchOutputDeviceForTrackInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioStreamChangeInfo streamChangeInfo = {};
    streamChangeInfo.audioRendererChangeInfo.Unmarshalling(data);
    AudioStreamDeviceChangeReasonExt reason = static_cast<AudioStreamDeviceChangeReasonExt::ExtEnum>(data.ReadInt32());
    FetchOutputDeviceForTrack(streamChangeInfo, reason);
}

void AudioPolicyManagerStub::FetchInputDeviceForTrackInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioStreamChangeInfo streamChangeInfo = {};
    streamChangeInfo.audioCapturerChangeInfo.Unmarshalling(data);
    FetchInputDeviceForTrack(streamChangeInfo);
}

void AudioPolicyManagerStub::GetOutputDeviceInternal(MessageParcel &data, MessageParcel &reply)
{
    sptr<AudioRendererFilter> audioRendererFilter = AudioRendererFilter::Unmarshalling(data);
    CHECK_AND_RETURN_LOG(audioRendererFilter != nullptr, "AudioRendererFilter unmarshall fail.");
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices = GetOutputDevice(audioRendererFilter);
    int32_t size = static_cast<int32_t>(devices.size());
    reply.WriteInt32(size);
    for (int i = 0; i < size; i++) {
        devices[i]->Marshalling(reply);
    }
}

void AudioPolicyManagerStub::GetInputDeviceInternal(MessageParcel &data, MessageParcel &reply)
{
    sptr<AudioCapturerFilter> audioCapturerFilter = AudioCapturerFilter::Unmarshalling(data);
    CHECK_AND_RETURN_LOG(audioCapturerFilter != nullptr, "AudioCapturerFilter unmarshall fail.");
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices = GetInputDevice(audioCapturerFilter);
    int32_t size = static_cast<int32_t>(devices.size());
    reply.WriteInt32(size);
    for (int i = 0; i < size; i++) {
        devices[i]->Marshalling(reply);
    }
}
} // namespace AudioStandard
} // namespace OHOS
