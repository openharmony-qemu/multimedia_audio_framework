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
#define LOG_TAG "SleAudioOperationCallbackStub"
#endif

#include "sle_audio_operation_callback_stub.h"

#include "audio_errors.h"
#include "audio_policy_log.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {
SleAudioOperationCallbackStub::SleAudioOperationCallbackStub()
{
}

SleAudioOperationCallbackStub::~SleAudioOperationCallbackStub()
{
}

int SleAudioOperationCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    CHECK_AND_RETURN_RET_LOG(data.ReadInterfaceToken() == GetDescriptor(),
        -1, "SleAudioOperationCallbackStub: ReadInterfaceToken failed");
    switch (code) {
        case GET_SLE_AUDIO_DEVICE_LIST: {
            GetSleAudioDeviceListInternal(data, reply);
            return AUDIO_OK;
        }
        case GET_SLE_VIRTUAL_AUDIO_DEVICE_LIST: {
            GetSleVirtualAudioDeviceListInternal(data, reply);
            return AUDIO_OK;
        }
        case IS_IN_BAND_RING_OPEN: {
            IsInBandRingOpenInternal(data, reply);
            return AUDIO_OK;
        }
        case GET_SUPPORT_STREAM_TYPE: {
            GetSupportStreamTypeInternal(data, reply);
            return AUDIO_OK;
        }
        case SET_ACTIVE_SINK_DEVICE: {
            SetActiveSinkDeviceInternal(data, reply);
            return AUDIO_OK;
        }
        case START_PLAYING: {
            StartPlayingInternal(data, reply);
            return AUDIO_OK;
        }
        case STOP_PLAYING: {
            StopPlayingInternal(data, reply);
            return AUDIO_OK;
        }
        case CONNECT_ALLOWED_PROFILES: {
            ConnectAllowedProfilesInternal(data, reply);
            return AUDIO_OK;
        }
        case SET_DEVICE_ABS_VOLUME: {
            SetDeviceAbsVolumeInternal(data, reply);
            return AUDIO_OK;
        }
        case SEND_USER_SELECTION: {
            SendUserSelectionInternal(data, reply);
            return AUDIO_OK;
        }
        default: {
            AUDIO_ERR_LOG("default case, need check AudioListenerStub");
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
    }
}

int32_t SleAudioOperationCallbackStub::SetSleAudioOperationCallback(
    const std::weak_ptr<SleAudioOperationCallback> &callback)
{
    std::lock_guard<std::mutex> lock(sleAudioOperationCallbackMutex_);
    sleAudioOperationCallback_ = callback;
    std::shared_ptr<SleAudioOperationCallback> sleAudioOperationCallback = sleAudioOperationCallback_.lock();
    CHECK_AND_RETURN_RET_LOG(sleAudioOperationCallback != nullptr, ERROR_INVALID_PARAM,
        "sleAudioOperationCallback_ is nullptr");
    return SUCCESS;
}

void SleAudioOperationCallbackStub::GetSleAudioDeviceListInternal(MessageParcel &data, MessageParcel &reply)
{
    std::vector<AudioDeviceDescriptor> devices;
    GetSleAudioDeviceList(devices);
    reply.WriteInt32(devices.size());
    for (auto &desc : devices) {
        desc.Marshalling(reply);
    }
}

void SleAudioOperationCallbackStub::GetSleVirtualAudioDeviceListInternal(MessageParcel &data, MessageParcel &reply)
{
    std::vector<AudioDeviceDescriptor> devices;
    GetSleVirtualAudioDeviceList(devices);
    reply.WriteInt32(devices.size());
    for (auto &desc : devices) {
        desc.Marshalling(reply);
    }
}

void SleAudioOperationCallbackStub::IsInBandRingOpenInternal(MessageParcel &data, MessageParcel &reply)
{
    std::string device = data.ReadString();
    int32_t result = IsInBandRingOpen(device);
    reply.WriteInt32(result);
}

void SleAudioOperationCallbackStub::GetSupportStreamTypeInternal(MessageParcel &data, MessageParcel &reply)
{
    std::string device = data.ReadString();
    uint32_t streamType = GetSupportStreamType(device);
    reply.WriteUint32(streamType);
}

void SleAudioOperationCallbackStub::SetActiveSinkDeviceInternal(MessageParcel &data, MessageParcel &reply)
{
    std::string device = data.ReadString();
    uint32_t streamType = data.ReadUint32();
    int32_t result = SetActiveSinkDevice(device, streamType);
    reply.WriteInt32(result);
}

void SleAudioOperationCallbackStub::StartPlayingInternal(MessageParcel &data, MessageParcel &reply)
{
    std::string device = data.ReadString();
    uint32_t streamType = data.ReadUint32();
    int32_t result = StartPlaying(device, streamType);
    reply.WriteInt32(result);
}

void SleAudioOperationCallbackStub::StopPlayingInternal(MessageParcel &data, MessageParcel &reply)
{
    std::string device = data.ReadString();
    uint32_t streamType = data.ReadUint32();
    int32_t result = StopPlaying(device, streamType);
    reply.WriteInt32(result);
}

void SleAudioOperationCallbackStub::ConnectAllowedProfilesInternal(MessageParcel &data, MessageParcel &reply)
{
    std::string remoteAddr = data.ReadString();
    int32_t result = ConnectAllowedProfiles(remoteAddr);
    reply.WriteInt32(result);
}

void SleAudioOperationCallbackStub::SetDeviceAbsVolumeInternal(MessageParcel &data, MessageParcel &reply)
{
    std::string remoteAddr = data.ReadString();
    uint32_t volume = data.ReadUint32();
    uint32_t streamType = data.ReadUint32();
    int32_t result = SetDeviceAbsVolume(remoteAddr, volume, streamType);
    reply.WriteInt32(result);
}

void SleAudioOperationCallbackStub::SendUserSelectionInternal(MessageParcel &data, MessageParcel &reply)
{
    std::string device = data.ReadString();
    uint32_t streamType = data.ReadUint32();
    int32_t result = SendUserSelection(device, streamType);
    reply.WriteInt32(result);
}

void SleAudioOperationCallbackStub::GetSleAudioDeviceList(std::vector<AudioDeviceDescriptor> &devices)
{
    std::unique_lock lock(sleAudioOperationCallbackMutex_);
    std::shared_ptr<SleAudioOperationCallback> sleAudioOperationCallback = sleAudioOperationCallback_.lock();
    CHECK_AND_RETURN_LOG(sleAudioOperationCallback != nullptr, "sleAudioOperationCallback_ is nullptr");
    lock.unlock();

    sleAudioOperationCallback->GetSleAudioDeviceList(devices);
}

void SleAudioOperationCallbackStub::GetSleVirtualAudioDeviceList(std::vector<AudioDeviceDescriptor> &devices)
{
    std::unique_lock lock(sleAudioOperationCallbackMutex_);
    std::shared_ptr<SleAudioOperationCallback> sleAudioOperationCallback = sleAudioOperationCallback_.lock();
    CHECK_AND_RETURN_LOG(sleAudioOperationCallback != nullptr, "sleAudioOperationCallback_ is nullptr");
    lock.unlock();

    sleAudioOperationCallback->GetSleVirtualAudioDeviceList(devices);
}

bool SleAudioOperationCallbackStub::IsInBandRingOpen(const std::string &device)
{
    std::unique_lock lock(sleAudioOperationCallbackMutex_);
    std::shared_ptr<SleAudioOperationCallback> sleAudioOperationCallback = sleAudioOperationCallback_.lock();
    CHECK_AND_RETURN_RET_LOG(sleAudioOperationCallback != nullptr, false, "sleAudioOperationCallback_ is nullptr");
    lock.unlock();

    return sleAudioOperationCallback->IsInBandRingOpen(device);
}

uint32_t SleAudioOperationCallbackStub::GetSupportStreamType(const std::string &device)
{
    uint32_t streamType = 0;
    std::unique_lock lock(sleAudioOperationCallbackMutex_);
    std::shared_ptr<SleAudioOperationCallback> sleAudioOperationCallback = sleAudioOperationCallback_.lock();
    CHECK_AND_RETURN_RET_LOG(sleAudioOperationCallback != nullptr, streamType, "sleAudioOperationCallback_ is nullptr");
    lock.unlock();

    return sleAudioOperationCallback->GetSupportStreamType(device);
}

int32_t SleAudioOperationCallbackStub::SetActiveSinkDevice(const std::string &device, uint32_t streamType)
{
    std::unique_lock lock(sleAudioOperationCallbackMutex_);
    std::shared_ptr<SleAudioOperationCallback> sleAudioOperationCallback = sleAudioOperationCallback_.lock();
    CHECK_AND_RETURN_RET_LOG(sleAudioOperationCallback != nullptr, ERROR, "sleAudioOperationCallback_ is nullptr");
    lock.unlock();

    return sleAudioOperationCallback->SetActiveSinkDevice(device, streamType);
}

int32_t SleAudioOperationCallbackStub::StartPlaying(const std::string &device, uint32_t streamType)
{
    std::unique_lock lock(sleAudioOperationCallbackMutex_);
    std::shared_ptr<SleAudioOperationCallback> sleAudioOperationCallback = sleAudioOperationCallback_.lock();
    CHECK_AND_RETURN_RET_LOG(sleAudioOperationCallback != nullptr, ERROR, "sleAudioOperationCallback_ is nullptr");
    lock.unlock();

    return sleAudioOperationCallback->StartPlaying(device, streamType);
}

int32_t SleAudioOperationCallbackStub::StopPlaying(const std::string &device, uint32_t streamType)
{
    std::unique_lock lock(sleAudioOperationCallbackMutex_);
    std::shared_ptr<SleAudioOperationCallback> sleAudioOperationCallback = sleAudioOperationCallback_.lock();
    CHECK_AND_RETURN_RET_LOG(sleAudioOperationCallback != nullptr, ERROR, "sleAudioOperationCallback_ is nullptr");
    lock.unlock();

    return sleAudioOperationCallback->StopPlaying(device, streamType);
}

int32_t SleAudioOperationCallbackStub::ConnectAllowedProfiles(const std::string &remoteAddr)
{
    std::unique_lock lock(sleAudioOperationCallbackMutex_);
    std::shared_ptr<SleAudioOperationCallback> sleAudioOperationCallback = sleAudioOperationCallback_.lock();
    CHECK_AND_RETURN_RET_LOG(sleAudioOperationCallback != nullptr, ERROR, "sleAudioOperationCallback_ is nullptr");
    lock.unlock();

    return sleAudioOperationCallback->ConnectAllowedProfiles(remoteAddr);
}

int32_t SleAudioOperationCallbackStub::SetDeviceAbsVolume(
    const std::string &remoteAddr, uint32_t volume, uint32_t streamType)
{
    std::unique_lock lock(sleAudioOperationCallbackMutex_);
    std::shared_ptr<SleAudioOperationCallback> sleAudioOperationCallback = sleAudioOperationCallback_.lock();
    CHECK_AND_RETURN_RET_LOG(sleAudioOperationCallback != nullptr, ERROR, "sleAudioOperationCallback_ is nullptr");
    lock.unlock();

    return sleAudioOperationCallback->SetDeviceAbsVolume(remoteAddr, volume, streamType);
}

int32_t SleAudioOperationCallbackStub::SendUserSelection(const std::string &device, uint32_t streamType)
{
    std::unique_lock lock(sleAudioOperationCallbackMutex_);
    std::shared_ptr<SleAudioOperationCallback> sleAudioOperationCallback = sleAudioOperationCallback_.lock();
    CHECK_AND_RETURN_RET_LOG(sleAudioOperationCallback != nullptr, ERROR, "sleAudioOperationCallback_ is nullptr");
    lock.unlock();

    return sleAudioOperationCallback->SendUserSelection(device, streamType);
}
} // namespace AudioStandard
} // namespace OHOS
