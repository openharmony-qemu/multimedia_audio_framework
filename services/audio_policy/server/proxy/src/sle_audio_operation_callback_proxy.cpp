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
#define LOG_TAG "SleAudioOperationCallbackProxy"
#endif

#include "sle_audio_operation_callback_proxy.h"
#include "audio_policy_log.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
namespace {
const int32_t DEVICE_SIZE_LIMIT = 100;
} // namespace
SleAudioOperationCallbackProxy::SleAudioOperationCallbackProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IStandardSleAudioOperationCallback>(impl)
{
    AUDIO_DEBUG_LOG("Instances create");
}

SleAudioOperationCallbackProxy::~SleAudioOperationCallbackProxy()
{
    AUDIO_DEBUG_LOG("~SleAudioOperationCallbackProxy: Instance destroy");
}

void SleAudioOperationCallbackProxy::GetSleAudioDeviceList(std::vector<AudioDeviceDescriptor> &devices)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    CHECK_AND_RETURN_LOG(data.WriteInterfaceToken(GetDescriptor()), "WriteInterfaceToken failed");

    int error = Remote()->SendRequest(GET_SLE_AUDIO_DEVICE_LIST, data, reply, option);
    CHECK_AND_RETURN_LOG(error == ERR_NONE, "Failed, error: %{public}d", error);

    int32_t size = reply.ReadInt32();
    CHECK_AND_RETURN_LOG(size < DEVICE_SIZE_LIMIT && size >= 0, "reply size reach limit");
    for (int32_t i = 0; i < size; i++) {
        auto device = std::make_shared<AudioDeviceDescriptor>(AudioDeviceDescriptor::UnmarshallingPtr(reply));
        CHECK_AND_RETURN_LOG(device != nullptr, "UnmarshallingPtr failed");
        devices.push_back(device);
    }
}

void SleAudioOperationCallbackProxy::GetSleVirtualAudioDeviceList(std::vector<AudioDeviceDescriptor> &devices)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_LOG(data.WriteInterfaceToken(GetDescriptor()), "WriteInterfaceToken failed");

    int error = Remote()->SendRequest(GET_SLE_VIRTUAL_AUDIO_DEVICE_LIST, data, reply, option);
    CHECK_AND_RETURN_LOG(error == ERR_NONE, "Failed, error: %{public}d", error);

    int32_t size = reply.ReadInt32();
    CHECK_AND_RETURN_LOG(size < DEVICE_SIZE_LIMIT && size >= 0, "reply size reach limit");
    for (int32_t i = 0; i < size; i++) {
        auto device = std::make_shared<AudioDeviceDescriptor>(AudioDeviceDescriptor::UnmarshallingPtr(reply));
        CHECK_AND_RETURN_LOG(device != nullptr, "UnmarshallingPtr failed");
        devices.push_back(device);
    }
}

bool SleAudioOperationCallbackProxy::IsInBandRingOpen(const std::string &device)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "WriteInterfaceToken failed");

    data.WriteString(device);
    int error = Remote()->SendRequest(IS_IN_BAND_RING_OPEN, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, false, "Failed, error: %{public}d", error);

    return reply.ReadBool();
}

uint32_t SleAudioOperationCallbackProxy::GetSupportStreamType(const std::string &device)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), 0, "WriteInterfaceToken failed");

    data.WriteString(device);
    int error = Remote()->SendRequest(GET_SUPPORT_STREAM_TYPE, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, 0, "Failed, error: %{public}d", error);

    return reply.ReadUint32();
}

int32_t SleAudioOperationCallbackProxy::SetActiveSinkDevice(const std::string &device, uint32_t streamType)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "WriteInterfaceToken failed");

    data.WriteString(device);
    data.WriteUint32(streamType);
    int error = Remote()->SendRequest(SET_ACTIVE_SINK_DEVICE, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "Failed, error: %{public}d", error);

    return reply.ReadInt32();
}

int32_t SleAudioOperationCallbackProxy::StartPlaying(const std::string &device, uint32_t streamType)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "WriteInterfaceToken failed");

    data.WriteString(device);
    data.WriteUint32(streamType);
    int error = Remote()->SendRequest(START_PLAYING, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "Failed, error: %{public}d", error);

    return reply.ReadInt32();
}

int32_t SleAudioOperationCallbackProxy::StopPlaying(const std::string &device, uint32_t streamType)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "WriteInterfaceToken failed");

    data.WriteString(device);
    data.WriteUint32(streamType);
    int error = Remote()->SendRequest(STOP_PLAYING, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "Failed, error: %{public}d", error);

    return reply.ReadInt32();
}

int32_t SleAudioOperationCallbackProxy::ConnectAllowedProfiles(const std::string &remoteAddr)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "WriteInterfaceToken failed");

    data.WriteString(remoteAddr);
    int error = Remote()->SendRequest(CONNECT_ALLOWED_PROFILES, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "Failed, error: %{public}d", error);

    return reply.ReadInt32();
}

int32_t SleAudioOperationCallbackProxy::SetDeviceAbsVolume(const std::string &remoteAddr, uint32_t volume,
    uint32_t streamType)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "WriteInterfaceToken failed");

    data.WriteString(remoteAddr);
    data.WriteUint32(volume);
    data.WriteUint32(streamType);
    int error = Remote()->SendRequest(SET_DEVICE_ABS_VOLUME, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "Failed, error: %{public}d", error);

    return reply.ReadInt32();
}

int32_t SleAudioOperationCallbackProxy::SendUserSelection(const std::string &device, uint32_t streamType)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR, "WriteInterfaceToken failed");

    data.WriteString(device);
    data.WriteUint32(streamType);
    int error = Remote()->SendRequest(SEND_USER_SELECTION, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "Failed, error: %{public}d", error);

    return reply.ReadInt32();
}
} // namespace AudioStandard
} // namespace OHOS
