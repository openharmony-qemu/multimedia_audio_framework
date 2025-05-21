/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioClientTrackerCallbackProxy"
#endif

#include "audio_policy_log.h"
#include "audio_client_tracker_callback_proxy.h"

namespace OHOS {
namespace AudioStandard {
AudioClientTrackerCallbackProxy::AudioClientTrackerCallbackProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IStandardClientTracker>(impl) { }

void AudioClientTrackerCallbackProxy::MuteStreamImpl(
    const StreamSetStateEventInternal &streamSetStateEventInternal)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_LOG(ret, "WriteInterfaceToken failed");

    data.WriteInt32(static_cast<int32_t>(streamSetStateEventInternal.streamSetState));
    data.WriteInt32(static_cast<int32_t>(streamSetStateEventInternal.streamUsage));
    int error = Remote()->SendRequest(MUTESTREAM, data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_WARNING_LOG("MuteStreamImpl failed, error: %{public}d", error);
    }
}

void AudioClientTrackerCallbackProxy::UnmuteStreamImpl(
    const StreamSetStateEventInternal &streamSetStateEventInternal)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_LOG(ret, "WriteInterfaceToken failed");

    data.WriteInt32(static_cast<int32_t>(streamSetStateEventInternal.streamSetState));
    data.WriteInt32(static_cast<int32_t>(streamSetStateEventInternal.streamUsage));
    int error = Remote()->SendRequest(UNMUTESTREAM, data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_WARNING_LOG("UnmuteStreamImpl failed, error: %{public}d", error);
    }
}

void AudioClientTrackerCallbackProxy::PausedStreamImpl(
    const StreamSetStateEventInternal &streamSetStateEventInternal)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_LOG(ret, "WriteInterfaceToken failed");

    data.WriteInt32(static_cast<int32_t>(streamSetStateEventInternal.streamSetState));
    data.WriteInt32(static_cast<int32_t>(streamSetStateEventInternal.streamUsage));
    int error = Remote()->SendRequest(PAUSEDSTREAM, data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_WARNING_LOG("PausedStreamImpl failed, error: %{public}d", error);
    }
}

void AudioClientTrackerCallbackProxy::ResumeStreamImpl(
    const StreamSetStateEventInternal &streamSetStateEventInternal)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_LOG(ret, "WriteInterfaceToken failed");

    data.WriteInt32(static_cast<int32_t>(streamSetStateEventInternal.streamSetState));
    data.WriteInt32(static_cast<int32_t>(streamSetStateEventInternal.streamUsage));
    int error = Remote()->SendRequest(RESUMESTREAM, data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_WARNING_LOG("ResumeStreamImpl failed, error: %{public}d", error);
    }
}

void AudioClientTrackerCallbackProxy::SetLowPowerVolumeImpl(float volume)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_LOG(ret, "WriteInterfaceToken failed");

    data.WriteFloat(static_cast<float>(volume));
    int error = Remote()->SendRequest(SETLOWPOWERVOL, data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_WARNING_LOG("SETLOWPOWERVOL failed, error: %{public}d", error);
    }
}

void AudioClientTrackerCallbackProxy::GetLowPowerVolumeImpl(float &volume)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_LOG(ret, "WriteInterfaceToken failed");

    int error = Remote()->SendRequest(GETLOWPOWERVOL, data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_WARNING_LOG("GETLOWPOWERVOL failed, error: %{public}d", error);
    }

    volume = reply.ReadFloat();
}

void AudioClientTrackerCallbackProxy::GetSingleStreamVolumeImpl(float &volume)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_LOG(ret, "WriteInterfaceToken failed");

    int error = Remote()->SendRequest(GETSINGLESTREAMVOL, data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_WARNING_LOG("GETSINGLESTREAMVOL failed, error: %{public}d", error);
    }

    volume = reply.ReadFloat();
}

void AudioClientTrackerCallbackProxy::SetOffloadModeImpl(int32_t state, bool isAppBack)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_LOG(ret, "WriteInterfaceToken failed");

    data.WriteInt32(static_cast<int32_t>(state));
    data.WriteBool(static_cast<bool>(isAppBack));

    int error = Remote()->SendRequest(SETOFFLOADMODE, data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_WARNING_LOG("SETOFFLOADMODE failed, error: %{public}d", error);
    }
}

void AudioClientTrackerCallbackProxy::UnsetOffloadModeImpl()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_LOG(ret, "WriteInterfaceToken failed");

    int error = Remote()->SendRequest(UNSETOFFLOADMODE, data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_WARNING_LOG("UNSETOFFLOADMODE failed, error: %{public}d", error);
    }
}

ClientTrackerCallbackListener::ClientTrackerCallbackListener(const sptr<IStandardClientTracker> &listener)
    : listener_(listener)
{
}

ClientTrackerCallbackListener::~ClientTrackerCallbackListener()
{
}

void ClientTrackerCallbackListener::MuteStreamImpl(
    const StreamSetStateEventInternal &streamSetStateEventInternal)
{
    if (listener_ != nullptr) {
        listener_->MuteStreamImpl(streamSetStateEventInternal);
    }
}

void ClientTrackerCallbackListener::UnmuteStreamImpl(
    const StreamSetStateEventInternal &streamSetStateEventInternal)
{
    if (listener_ != nullptr) {
        listener_->UnmuteStreamImpl(streamSetStateEventInternal);
    }
}

void ClientTrackerCallbackListener::PausedStreamImpl(
    const StreamSetStateEventInternal &streamSetStateEventInternal)
{
    if (listener_ != nullptr) {
        listener_->PausedStreamImpl(streamSetStateEventInternal);
    }
}

void ClientTrackerCallbackListener::ResumeStreamImpl(
    const StreamSetStateEventInternal &streamSetStateEventInternal)
{
    if (listener_ != nullptr) {
        listener_->ResumeStreamImpl(streamSetStateEventInternal);
    }
}

void ClientTrackerCallbackListener::SetLowPowerVolumeImpl(float volume)
{
    if (listener_ != nullptr) {
        listener_->SetLowPowerVolumeImpl(volume);
    }
}

void ClientTrackerCallbackListener::GetLowPowerVolumeImpl(float &volume)
{
    if (listener_ != nullptr) {
        listener_->GetLowPowerVolumeImpl(volume);
    }
}

void ClientTrackerCallbackListener::GetSingleStreamVolumeImpl(float &volume)
{
    if (listener_ != nullptr) {
        listener_->GetSingleStreamVolumeImpl(volume);
    }
}

void ClientTrackerCallbackListener::SetOffloadModeImpl(int32_t state, bool isAppBack)
{
    if (listener_ != nullptr) {
        listener_->SetOffloadModeImpl(state, isAppBack);
    }
}

void ClientTrackerCallbackListener::UnsetOffloadModeImpl()
{
    if (listener_ != nullptr) {
        listener_->UnsetOffloadModeImpl();
    }
}
} // namespace AudioStandard
} // namespace OHOS
