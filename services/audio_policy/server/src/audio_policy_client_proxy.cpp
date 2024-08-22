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
#define LOG_TAG "AudioPolicyClientProxy"
#endif

#include "audio_policy_client_proxy.h"
#include "audio_policy_log.h"

namespace OHOS {
namespace AudioStandard {
AudioPolicyClientProxy::AudioPolicyClientProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IAudioPolicyClient>(impl)
{}

AudioPolicyClientProxy::~AudioPolicyClientProxy()
{}

void AudioPolicyClientProxy::OnVolumeKeyEvent(VolumeEvent volumeEvent)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC | MessageOption::TF_ASYNC_WAKEUP_LATER);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("WriteInterfaceToken failed");
        return;
    }
    data.WriteInt32(static_cast<int32_t>(AudioPolicyClientCode::ON_VOLUME_KEY_EVENT));
    data.WriteInt32(static_cast<int32_t>(volumeEvent.volumeType));
    data.WriteInt32(volumeEvent.volume);
    data.WriteBool(volumeEvent.updateUi);
    data.WriteInt32(volumeEvent.volumeGroupId);
    data.WriteString(volumeEvent.networkId);
    int error = Remote()->SendRequest(static_cast<uint32_t>(UPDATE_CALLBACK_CLIENT), data, reply, option);
    if (error != 0) {
        AUDIO_ERR_LOG("Error while sending volume key event %{public}d", error);
    }
    reply.ReadInt32();
}

void AudioPolicyClientProxy::OnAudioFocusInfoChange(
    const std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("WriteInterfaceToken failed");
        return;
    }
    data.WriteInt32(static_cast<int32_t>(AudioPolicyClientCode::ON_FOCUS_INFO_CHANGED));
    size_t size = focusInfoList.size();
    data.WriteInt32(static_cast<int32_t>(size));
    for (auto iter = focusInfoList.begin(); iter != focusInfoList.end(); ++iter) {
        AudioInterrupt::Marshalling(data, iter->first);
        data.WriteInt32(iter->second);
    }
    int error = Remote()->SendRequest(static_cast<uint32_t>(UPDATE_CALLBACK_CLIENT), data, reply, option);
    if (error != 0) {
        AUDIO_ERR_LOG("Error while sending focus change info: %{public}d", error);
    }
    reply.ReadInt32();
}

void AudioPolicyClientProxy::OnAudioFocusRequested(const AudioInterrupt &requestFocus)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("WriteInterfaceToken failed");
        return;
    }

    data.WriteInt32(static_cast<int32_t>(AudioPolicyClientCode::ON_FOCUS_REQUEST_CHANGED));
    AudioInterrupt::Marshalling(data, requestFocus);
    int error = Remote()->SendRequest(static_cast<uint32_t>(UPDATE_CALLBACK_CLIENT), data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_ERR_LOG("OnAudioFocusRequested failed, error: %{public}d", error);
    }
}

void AudioPolicyClientProxy::OnAudioFocusAbandoned(const AudioInterrupt &abandonFocus)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("WriteInterfaceToken failed");
        return;
    }

    data.WriteInt32(static_cast<int32_t>(AudioPolicyClientCode::ON_FOCUS_ABANDON_CHANGED));
    AudioInterrupt::Marshalling(data, abandonFocus);
    int error = Remote()->SendRequest(static_cast<uint32_t>(UPDATE_CALLBACK_CLIENT), data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_ERR_LOG("OnAudioFocusAbandoned failed, error: %{public}d", error);
    }
}

void AudioPolicyClientProxy::OnDeviceChange(const DeviceChangeAction &deviceChangeAction)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("WriteInterfaceToken failed");
        return;
    }

    auto devices = deviceChangeAction.deviceDescriptors;
    size_t size = deviceChangeAction.deviceDescriptors.size();
    data.WriteInt32(static_cast<int32_t>(AudioPolicyClientCode::ON_DEVICE_CHANGE));
    data.WriteInt32(deviceChangeAction.type);
    data.WriteInt32(deviceChangeAction.flag);
    data.WriteInt32(static_cast<int32_t>(size));
    for (size_t i = 0; i < size; i++) {
        devices[i]->Marshalling(data);
    }
    int error = Remote()->SendRequest(static_cast<uint32_t>(UPDATE_CALLBACK_CLIENT), data, reply, option);
    if (error != 0) {
        AUDIO_ERR_LOG("Error while sending device change info: %{public}d", error);
    }
    reply.ReadInt32();
}

void AudioPolicyClientProxy::OnRingerModeUpdated(const AudioRingerMode &ringerMode)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("AudioPolicyClientProxy::OnRingerModeUpdated: WriteInterfaceToken failed");
        return;
    }

    data.WriteInt32(static_cast<int32_t>(AudioPolicyClientCode::ON_RINGERMODE_UPDATE));
    data.WriteInt32(static_cast<int32_t>(ringerMode));
    int error = Remote()->SendRequest(static_cast<uint32_t>(UPDATE_CALLBACK_CLIENT), data, reply, option);
    if (error != 0) {
        AUDIO_ERR_LOG("Error while sending ringer mode updated info: %{public}d", error);
    }
    reply.ReadInt32();
}

void AudioPolicyClientProxy::OnMicStateUpdated(const MicStateChangeEvent &micStateChangeEvent)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("WriteInterfaceToken failed");
        return;
    }

    data.WriteInt32(static_cast<int32_t>(AudioPolicyClientCode::ON_MIC_STATE_UPDATED));
    data.WriteBool(micStateChangeEvent.mute);
    int error = Remote()->SendRequest(static_cast<uint32_t>(UPDATE_CALLBACK_CLIENT), data, reply, option);
    if (error != 0) {
        AUDIO_ERR_LOG("Error while sending ringer mode updated info: %{public}d", error);
    }
    reply.ReadInt32();
}

void AudioPolicyClientProxy::OnPreferredOutputDeviceUpdated(const std::vector<sptr<AudioDeviceDescriptor>> &desc)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("WriteInterfaceToken failed");
        return;
    }

    data.WriteInt32(static_cast<int32_t>(AudioPolicyClientCode::ON_ACTIVE_OUTPUT_DEVICE_UPDATED));
    int32_t size = static_cast<int32_t>(desc.size());
    data.WriteInt32(size);
    for (int i = 0; i < size; i++) {
        desc[i]->Marshalling(data);
    }

    int error = Remote()->SendRequest(static_cast<uint32_t>(UPDATE_CALLBACK_CLIENT), data, reply, option);
    if (error != 0) {
        AUDIO_ERR_LOG("Error while sending preferred output device updated info: %{public}d", error);
    }
    reply.ReadInt32();
}

void AudioPolicyClientProxy::OnPreferredInputDeviceUpdated(const std::vector<sptr<AudioDeviceDescriptor>> &desc)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("WriteInterfaceToken failed");
        return;
    }

    data.WriteInt32(static_cast<int32_t>(AudioPolicyClientCode::ON_ACTIVE_INPUT_DEVICE_UPDATED));
    int32_t size = static_cast<int32_t>(desc.size());
    data.WriteInt32(size);
    for (int i = 0; i < size; i++) {
        desc[i]->Marshalling(data);
    }

    int error = Remote()->SendRequest(static_cast<uint32_t>(UPDATE_CALLBACK_CLIENT), data, reply, option);
    if (error != 0) {
        AUDIO_ERR_LOG("Error while sending preferred input device updated info: %{public}d", error);
    }
    reply.ReadInt32();
}

void AudioPolicyClientProxy::OnRendererStateChange(
    std::vector<std::unique_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("WriteInterfaceToken failed");
        return;
    }

    size_t size = audioRendererChangeInfos.size();
    data.WriteInt32(static_cast<int32_t>(AudioPolicyClientCode::ON_RENDERERSTATE_CHANGE));
    data.WriteInt32(size);
    for (const std::unique_ptr<AudioRendererChangeInfo> &rendererChangeInfo: audioRendererChangeInfos) {
        if (!rendererChangeInfo) {
            AUDIO_ERR_LOG("Renderer change info null, something wrong!!");
            continue;
        }
        rendererChangeInfo->Marshalling(data, hasBTPermission_, hasSystemPermission_, apiVersion_);
    }
    int error = Remote()->SendRequest(static_cast<uint32_t>(UPDATE_CALLBACK_CLIENT), data, reply, option);
    if (error != 0) {
        AUDIO_ERR_LOG("Error while sending renderer state change info: %{public}d", error);
    }
    reply.ReadInt32();
}

void AudioPolicyClientProxy::OnCapturerStateChange(
    std::vector<std::unique_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("WriteInterfaceToken failed");
        return;
    }

    size_t size = audioCapturerChangeInfos.size();
    data.WriteInt32(static_cast<int32_t>(AudioPolicyClientCode::ON_CAPTURERSTATE_CHANGE));
    data.WriteInt32(size);
    for (const std::unique_ptr<AudioCapturerChangeInfo> &capturerChangeInfo: audioCapturerChangeInfos) {
        if (!capturerChangeInfo) {
            AUDIO_ERR_LOG("Capturer change info null, something wrong!!");
            continue;
        }
        capturerChangeInfo->Marshalling(data, hasBTPermission_, hasSystemPermission_, apiVersion_);
    }

    int error = Remote()->SendRequest(static_cast<uint32_t>(UPDATE_CALLBACK_CLIENT), data, reply, option);
    if (error != 0) {
        AUDIO_ERR_LOG("Error while sending capturer state change info: %{public}d", error);
    }
    reply.ReadInt32();
}

void AudioPolicyClientProxy::OnRendererDeviceChange(const uint32_t sessionId,
    const DeviceInfo &deviceInfo, const AudioStreamDeviceChangeReasonExt reason)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("WriteInterfaceToken failed");
        return;
    }

    data.WriteInt32(static_cast<int32_t>(AudioPolicyClientCode::ON_RENDERER_DEVICE_CHANGE));

    data.WriteUint32(sessionId);
    deviceInfo.Marshalling(data);
    data.WriteInt32(static_cast<int32_t>(reason));
    int error = Remote()->SendRequest(static_cast<uint32_t>(UPDATE_CALLBACK_CLIENT), data, reply, option);
    if (error != 0) {
        AUDIO_ERR_LOG("Error while sending DeviceChange: %{public}d", error);
    }
    reply.ReadInt32();
}

void AudioPolicyClientProxy::OnRecreateRendererStreamEvent(const uint32_t sessionId, const int32_t streamFlag,
    const AudioStreamDeviceChangeReasonExt reason)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("WriteInterfaceToken failed");
        return;
    }

    data.WriteInt32(static_cast<int32_t>(AudioPolicyClientCode::ON_RECREATE_RENDERER_STREAM_EVENT));

    data.WriteUint32(sessionId);
    data.WriteInt32(streamFlag);
    data.WriteInt32(static_cast<int32_t>(reason));
    int error = Remote()->SendRequest(static_cast<uint32_t>(UPDATE_CALLBACK_CLIENT), data, reply, option);
    if (error != 0) {
        AUDIO_ERR_LOG("Error while sending recreate stream event: %{public}d", error);
    }
    reply.ReadInt32();
}

void AudioPolicyClientProxy::OnRecreateCapturerStreamEvent(const uint32_t sessionId, const int32_t streamFlag,
    const AudioStreamDeviceChangeReasonExt reason)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("WriteInterfaceToken failed");
        return;
    }

    data.WriteInt32(static_cast<int32_t>(AudioPolicyClientCode::ON_RECREATE_CAPTURER_STREAM_EVENT));

    data.WriteUint32(sessionId);
    data.WriteInt32(streamFlag);
    data.WriteInt32(static_cast<int32_t>(reason));
    int error = Remote()->SendRequest(static_cast<uint32_t>(UPDATE_CALLBACK_CLIENT), data, reply, option);
    if (error != 0) {
        AUDIO_ERR_LOG("Error while sending recreate stream event: %{public}d", error);
    }
    reply.ReadInt32();
}

void AudioPolicyClientProxy::OnHeadTrackingDeviceChange(const std::unordered_map<std::string, bool> &changeInfo)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("WriteInterfaceToken failed");
        return;
    }

    data.WriteInt32(static_cast<int32_t>(AudioPolicyClientCode::ON_HEAD_TRACKING_DEVICE_CHANGE));

    int32_t size = static_cast<int32_t>(changeInfo.size());
    data.WriteInt32(size);

    for (const auto &pair : changeInfo) {
        data.WriteString(pair.first);
        data.WriteBool(pair.second);
    }

    int error = Remote()->SendRequest(static_cast<uint32_t>(UPDATE_CALLBACK_CLIENT), data, reply, option);
    if (error != 0) {
        AUDIO_ERR_LOG("Error while sending change info: %{public}d", error);
    }
    reply.ReadInt32();
}

void AudioPolicyClientProxy::OnSpatializationEnabledChange(const bool &enabled)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("WriteInterfaceToken failed");
        return;
    }

    data.WriteInt32(static_cast<int32_t>(AudioPolicyClientCode::ON_SPATIALIZATION_ENABLED_CHANGE));

    if (hasSystemPermission_) {
        data.WriteBool(enabled);
    } else {
        data.WriteBool(false);
    }

    int error = Remote()->SendRequest(static_cast<uint32_t>(UPDATE_CALLBACK_CLIENT), data, reply, option);
    if (error != 0) {
        AUDIO_ERR_LOG("Error while sending enabled info: %{public}d", error);
    }
    reply.ReadInt32();
}

void AudioPolicyClientProxy::OnSpatializationEnabledChangeForAnyDevice(const sptr<AudioDeviceDescriptor>
    &deviceDescriptor, const bool &enabled)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("WriteInterfaceToken failed");
        return;
    }

    data.WriteInt32(static_cast<int32_t>(AudioPolicyClientCode::ON_SPATIALIZATION_ENABLED_CHANGE_FOR_ANY_DEVICE));

    if (hasSystemPermission_) {
        deviceDescriptor->Marshalling(data);
        data.WriteBool(enabled);
    } else {
        data.WriteBool(false);
    }

    int error = Remote()->SendRequest(static_cast<uint32_t>(UPDATE_CALLBACK_CLIENT), data, reply, option);
    if (error != 0) {
        AUDIO_ERR_LOG("Error while sending enabled info: %{public}d", error);
    }
    reply.ReadInt32();
}

void AudioPolicyClientProxy::OnHeadTrackingEnabledChange(const bool &enabled)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("WriteInterfaceToken failed");
        return;
    }

    data.WriteInt32(static_cast<int32_t>(AudioPolicyClientCode::ON_HEAD_TRACKING_ENABLED_CHANGE));

    if (hasSystemPermission_) {
        data.WriteBool(enabled);
    } else {
        data.WriteBool(false);
    }

    int error = Remote()->SendRequest(static_cast<uint32_t>(UPDATE_CALLBACK_CLIENT), data, reply, option);
    if (error != 0) {
        AUDIO_ERR_LOG("Error while sending enabled info: %{public}d", error);
    }
    reply.ReadInt32();
}

void AudioPolicyClientProxy::OnHeadTrackingEnabledChangeForAnyDevice(const sptr<AudioDeviceDescriptor>
    &deviceDescriptor, const bool &enabled)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("WriteInterfaceToken failed");
        return;
    }
    
    data.WriteInt32(static_cast<int32_t>(AudioPolicyClientCode::ON_HEAD_TRACKING_ENABLED_CHANGE_FOR_ANY_DEVICE));

    if (hasSystemPermission_) {
        deviceDescriptor->Marshalling(data);
        data.WriteBool(enabled);
    } else {
        data.WriteBool(false);
    }

    int error = Remote()->SendRequest(static_cast<uint32_t>(UPDATE_CALLBACK_CLIENT), data, reply, option);
    if (error != 0) {
        AUDIO_ERR_LOG("Error while sending enabled info: %{public}d", error);
    }
    reply.ReadInt32();
}

void AudioPolicyClientProxy::OnAudioSessionDeactive(const AudioSessionDeactiveEvent &deactiveEvent)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC | MessageOption::TF_ASYNC_WAKEUP_LATER);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("WriteInterfaceToken failed");
        return;
    }
    data.WriteInt32(static_cast<int32_t>(AudioPolicyClientCode::ON_AUDIO_SESSION_DEACTIVE));
    data.WriteInt32(static_cast<int32_t>(deactiveEvent.deactiveReason));
    int error = Remote()->SendRequest(static_cast<uint32_t>(UPDATE_CALLBACK_CLIENT), data, reply, option);
    if (error != 0) {
        AUDIO_ERR_LOG("Error while sending volume key event %{public}d", error);
    }
    reply.ReadInt32();
}
} // namespace AudioStandard
} // namespace OHOS
