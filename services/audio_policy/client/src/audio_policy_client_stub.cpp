/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioPolicyClientStub"
#endif

#include "audio_policy_client_stub.h"
#include "audio_errors.h"
#include "audio_policy_log.h"
#include "audio_utils.h"

using namespace std;
namespace OHOS {
namespace AudioStandard {

static const int32_t FOCUS_INFO_VALID_SIZE = 128;
static const int32_t DEVICE_CHANGE_VALID_SIZE = 128;
static const int32_t PREFERRED_DEVICE_VALID_SIZE = 128;
static const int32_t STATE_VALID_SIZE = 1024;
static const int32_t MIC_BLOCKED_VALID_SIZE = 128;

AudioPolicyClientStub::AudioPolicyClientStub()
{}

AudioPolicyClientStub::~AudioPolicyClientStub()
{}

void AudioPolicyClientStub::OnFirMaxRemoteRequest(uint32_t updateCode, MessageParcel &data, MessageParcel &reply)
{
    switch (updateCode) {
        case static_cast<uint32_t>(AudioPolicyClientCode::ON_HEAD_TRACKING_DEVICE_CHANGE):
            HandleHeadTrackingDeviceChange(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyClientCode::ON_SPATIALIZATION_ENABLED_CHANGE):
            HandleSpatializationEnabledChange(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyClientCode::ON_SPATIALIZATION_ENABLED_CHANGE_FOR_ANY_DEVICE):
            HandleSpatializationEnabledChangeForAnyDevice(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyClientCode::ON_HEAD_TRACKING_ENABLED_CHANGE):
            HandleHeadTrackingEnabledChange(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyClientCode::ON_HEAD_TRACKING_ENABLED_CHANGE_FOR_ANY_DEVICE):
            HandleHeadTrackingEnabledChangeForAnyDevice(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyClientCode::ON_AUDIO_SESSION_DEACTIVE):
            HandleAudioSessionCallback(data, reply);
            break;
        default:
            break;
    }
}

void AudioPolicyClientStub::OnMaxRemoteRequest(uint32_t updateCode, MessageParcel &data, MessageParcel &reply)
{
    switch (updateCode) {
        case static_cast<uint32_t>(AudioPolicyClientCode::ON_RINGERMODE_UPDATE):
            HandleRingerModeUpdated(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyClientCode::ON_MIC_STATE_UPDATED):
            HandleMicStateChange(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyClientCode::ON_ACTIVE_OUTPUT_DEVICE_UPDATED):
            HandlePreferredOutputDeviceUpdated(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyClientCode::ON_ACTIVE_INPUT_DEVICE_UPDATED):
            HandlePreferredInputDeviceUpdated(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyClientCode::ON_RENDERERSTATE_CHANGE):
            HandleRendererStateChange(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyClientCode::ON_CAPTURERSTATE_CHANGE):
            HandleCapturerStateChange(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyClientCode::ON_RENDERER_DEVICE_CHANGE):
            HandleRendererDeviceChange(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyClientCode::ON_RECREATE_RENDERER_STREAM_EVENT):
            HandleRecreateRendererStreamEvent(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyClientCode::ON_RECREATE_CAPTURER_STREAM_EVENT):
            HandleRecreateCapturerStreamEvent(data, reply);
            break;
        default:
            OnFirMaxRemoteRequest(updateCode, data, reply);
            break;
    }
}

int AudioPolicyClientStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        AUDIO_ERR_LOG("AudioPolicyClientStub: ReadInterfaceToken failed");
        return -1;
    }
    switch (code) {
        case UPDATE_CALLBACK_CLIENT: {
            uint32_t updateCode = static_cast<uint32_t>(data.ReadInt32());
            if (updateCode > static_cast<uint32_t>(AudioPolicyClientCode::AUDIO_POLICY_CLIENT_CODE_MAX)) {
                return -1;
            }
            switch (updateCode) {
                case static_cast<uint32_t>(AudioPolicyClientCode::ON_VOLUME_KEY_EVENT):
                    HandleVolumeKeyEvent(data, reply);
                    break;
                case static_cast<uint32_t>(AudioPolicyClientCode::ON_FOCUS_INFO_CHANGED):
                    HandleAudioFocusInfoChange(data, reply);
                    break;
                case static_cast<uint32_t>(AudioPolicyClientCode::ON_FOCUS_REQUEST_CHANGED):
                    HandleAudioFocusRequested(data, reply);
                    break;
                case static_cast<uint32_t>(AudioPolicyClientCode::ON_FOCUS_ABANDON_CHANGED):
                    HandleAudioFocusAbandoned(data, reply);
                    break;
                case static_cast<uint32_t>(AudioPolicyClientCode::ON_DEVICE_CHANGE):
                    HandleDeviceChange(data, reply);
                    break;
                case static_cast<uint32_t>(AudioPolicyClientCode::ON_MICRO_PHONE_BLOCKED):
                    HandleMicrophoneBlocked(data, reply);
                    break;
                default:
                    OnMaxRemoteRequest(updateCode, data, reply);
                    break;
            }
            break;
        }
        default: {
            reply.WriteInt32(ERR_INVALID_OPERATION);
            break;
        }
    }
    return SUCCESS;
}

void AudioPolicyClientStub::HandleVolumeKeyEvent(MessageParcel &data, MessageParcel &reply)
{
    VolumeEvent event;
    event.volumeType = static_cast<AudioStreamType>(data.ReadInt32());
    event.volume = data.ReadInt32();
    event.updateUi = data.ReadBool();
    event.volumeGroupId = data.ReadInt32();
    event.networkId = data.ReadString();
    OnVolumeKeyEvent(event);
}

void AudioPolicyClientStub::HandleAudioFocusInfoChange(MessageParcel &data, MessageParcel &reply)
{
    std::list<std::pair<AudioInterrupt, AudioFocuState>> infoList;
    std::pair<AudioInterrupt, AudioFocuState> focusInfo = {};
    int32_t size = data.ReadInt32();
    CHECK_AND_RETURN_LOG(size < FOCUS_INFO_VALID_SIZE, "get invalid size : %{public}d", size);

    for (int32_t i = 0; i < size; i++) {
        AudioInterrupt::Unmarshalling(data, focusInfo.first);
        focusInfo.second = static_cast<AudioFocuState>(data.ReadInt32());
        infoList.emplace_back(focusInfo);
    }
    OnAudioFocusInfoChange(infoList);
}

void AudioPolicyClientStub::HandleAudioFocusRequested(MessageParcel &data, MessageParcel &reply)
{
    AudioInterrupt requestFocus = {};
    AudioInterrupt::Unmarshalling(data, requestFocus);
    OnAudioFocusRequested(requestFocus);
}

void AudioPolicyClientStub::HandleAudioFocusAbandoned(MessageParcel &data, MessageParcel &reply)
{
    AudioInterrupt abandonFocus = {};
    AudioInterrupt::Unmarshalling(data, abandonFocus);
    OnAudioFocusAbandoned(abandonFocus);
}

void AudioPolicyClientStub::HandleDeviceChange(MessageParcel &data, MessageParcel &reply)
{
    DeviceChangeAction deviceChange;
    deviceChange.type = static_cast<DeviceChangeType>(data.ReadUint32());
    deviceChange.flag = static_cast<DeviceFlag>(data.ReadUint32());
    int32_t size = data.ReadInt32();
    CHECK_AND_RETURN_LOG(size < DEVICE_CHANGE_VALID_SIZE, "get invalid size : %{public}d", size);

    for (int32_t i = 0; i < size; i++) {
        deviceChange.deviceDescriptors.emplace_back(AudioDeviceDescriptor::UnmarshallingPtr(data));
    }
    OnDeviceChange(deviceChange);
}

void AudioPolicyClientStub::HandleMicrophoneBlocked(MessageParcel &data, MessageParcel &reply)
{
    MicrophoneBlockedInfo microphoneBlocked;
    microphoneBlocked.blockStatus = static_cast<DeviceBlockStatus>(data.ReadUint32());
    int32_t size = data.ReadInt32();
    CHECK_AND_RETURN_LOG(size < MIC_BLOCKED_VALID_SIZE, "get invalid size : %{public}d", size);

    for (int32_t i = 0; i < size; i++) {
        microphoneBlocked.devices.emplace_back(AudioDeviceDescriptor::UnmarshallingPtr(data));
    }
    OnMicrophoneBlocked(microphoneBlocked);
}

void AudioPolicyClientStub::HandleRingerModeUpdated(MessageParcel &data, MessageParcel &reply)
{
    AudioRingerMode ringMode = static_cast<AudioRingerMode>(data.ReadInt32());
    OnRingerModeUpdated(ringMode);
}

void AudioPolicyClientStub::HandleMicStateChange(MessageParcel &data, MessageParcel &reply)
{
    MicStateChangeEvent micState;
    micState.mute = data.ReadBool();
    OnMicStateUpdated(micState);
}

void AudioPolicyClientStub::HandlePreferredOutputDeviceUpdated(MessageParcel &data, MessageParcel &reply)
{
    std::vector<sptr<AudioDeviceDescriptor>> deviceDescriptor;
    int32_t size = data.ReadInt32();
    CHECK_AND_RETURN_LOG(size < PREFERRED_DEVICE_VALID_SIZE, "get invalid size : %{public}d", size);

    for (int32_t i = 0; i < size; i++) {
        deviceDescriptor.push_back(AudioDeviceDescriptor::UnmarshallingPtr(data));
    }
    OnPreferredOutputDeviceUpdated(deviceDescriptor);
}

void AudioPolicyClientStub::HandlePreferredInputDeviceUpdated(MessageParcel &data, MessageParcel &reply)
{
    std::vector<sptr<AudioDeviceDescriptor>> deviceDescriptor;
    int32_t size = data.ReadInt32();
    CHECK_AND_RETURN_LOG(size < PREFERRED_DEVICE_VALID_SIZE, "get invalid size : %{public}d", size);

    for (int32_t i = 0; i < size; i++) {
        deviceDescriptor.push_back(AudioDeviceDescriptor::UnmarshallingPtr(data));
    }
    OnPreferredInputDeviceUpdated(deviceDescriptor);
}

void AudioPolicyClientStub::HandleRendererStateChange(MessageParcel &data, MessageParcel &reply)
{
    std::vector<std::unique_ptr<AudioRendererChangeInfo>> audioRenderChangeInfo;
    int32_t size = data.ReadInt32();
    Trace trace("HandleRendererStateChange size:" + std::to_string(size));
    CHECK_AND_RETURN_LOG(size < STATE_VALID_SIZE, "get invalid size : %{public}d", size);

    while (size > 0) {
        std::unique_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_unique<AudioRendererChangeInfo>();
        if (rendererChangeInfo == nullptr) {
            AUDIO_ERR_LOG("AudioPolicyClientStub::HandleRendererStateChange, No memory!");
            return;
        }
        rendererChangeInfo->Unmarshalling(data);
        audioRenderChangeInfo.push_back(move(rendererChangeInfo));
        size--;
    }
    OnRendererStateChange(audioRenderChangeInfo);
}

void AudioPolicyClientStub::HandleCapturerStateChange(MessageParcel &data, MessageParcel &reply)
{
    std::vector<std::unique_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfo;
    int32_t size = data.ReadInt32();
    CHECK_AND_RETURN_LOG(size < STATE_VALID_SIZE, "get invalid size : %{public}d", size);

    while (size > 0) {
        std::unique_ptr<AudioCapturerChangeInfo> capturerChangeInfo = std::make_unique<AudioCapturerChangeInfo>();
        if (capturerChangeInfo == nullptr) {
            AUDIO_ERR_LOG("AudioPolicyClientStub::HandleCapturerStateChange, No memory!");
            return;
        }
        capturerChangeInfo->Unmarshalling(data);
        audioCapturerChangeInfo.push_back(move(capturerChangeInfo));
        size--;
    }
    OnCapturerStateChange(audioCapturerChangeInfo);
}

void AudioPolicyClientStub::HandleRendererDeviceChange(MessageParcel &data, MessageParcel &reply)
{
    const uint32_t sessionId = data.ReadUint32();
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    deviceInfo.Unmarshalling(data);
    const AudioStreamDeviceChangeReasonExt reason
        = static_cast<AudioStreamDeviceChangeReasonExt::ExtEnum> (data.ReadInt32());

    OnRendererDeviceChange(sessionId, deviceInfo, reason);
}

void AudioPolicyClientStub::HandleRecreateRendererStreamEvent(MessageParcel &data, MessageParcel &reply)
{
    const uint32_t sessionId = data.ReadUint32();
    const uint32_t streamFlag = data.ReadUint32();
    const AudioStreamDeviceChangeReasonExt reason
        = static_cast<AudioStreamDeviceChangeReasonExt::ExtEnum> (data.ReadInt32());
    OnRecreateRendererStreamEvent(sessionId, streamFlag, reason);
}

void AudioPolicyClientStub::HandleRecreateCapturerStreamEvent(MessageParcel &data, MessageParcel &reply)
{
    const uint32_t sessionId = data.ReadUint32();
    const uint32_t streamFlag = data.ReadUint32();
    const AudioStreamDeviceChangeReasonExt reason
        = static_cast<AudioStreamDeviceChangeReasonExt::ExtEnum> (data.ReadInt32());
    OnRecreateCapturerStreamEvent(sessionId, streamFlag, reason);
}

void AudioPolicyClientStub::HandleHeadTrackingDeviceChange(MessageParcel &data, MessageParcel &reply)
{
    std::unordered_map<std::string, bool> changeInfo;
    int32_t size = data.ReadInt32();
    CHECK_AND_RETURN_LOG(size < DEVICE_CHANGE_VALID_SIZE, "get invalid size : %{public}d", size);

    while (size > 0) {
        std::string macAddress = data.ReadString();
        bool isHeadTrackingDataRequested = data.ReadBool();
        changeInfo[macAddress] = isHeadTrackingDataRequested;
        size--;
    }
    OnHeadTrackingDeviceChange(changeInfo);
}

void AudioPolicyClientStub::HandleSpatializationEnabledChange(MessageParcel &data, MessageParcel &reply)
{
    bool enabled = data.ReadBool();
    OnSpatializationEnabledChange(enabled);
}

void AudioPolicyClientStub::HandleSpatializationEnabledChangeForAnyDevice(MessageParcel &data, MessageParcel &reply)
{
    sptr<AudioDeviceDescriptor> audioDeviceDescriptor = AudioDeviceDescriptor::UnmarshallingPtr(data);
    CHECK_AND_RETURN_LOG(audioDeviceDescriptor != nullptr, "Unmarshalling fail.");
    bool enabled = data.ReadBool();
    OnSpatializationEnabledChangeForAnyDevice(audioDeviceDescriptor, enabled);
}

void AudioPolicyClientStub::HandleHeadTrackingEnabledChange(MessageParcel &data, MessageParcel &reply)
{
    bool enabled = data.ReadBool();
    OnHeadTrackingEnabledChange(enabled);
}

void AudioPolicyClientStub::HandleHeadTrackingEnabledChangeForAnyDevice(MessageParcel &data, MessageParcel &reply)
{
    sptr<AudioDeviceDescriptor> audioDeviceDescriptor = AudioDeviceDescriptor::UnmarshallingPtr(data);
    CHECK_AND_RETURN_LOG(audioDeviceDescriptor != nullptr, "Unmarshalling fail.");
    bool enabled = data.ReadBool();
    OnHeadTrackingEnabledChangeForAnyDevice(audioDeviceDescriptor, enabled);
}

void AudioPolicyClientStub::HandleAudioSessionCallback(MessageParcel &data, MessageParcel &reply)
{
    AUDIO_INFO_LOG("HandleAudioSessionCallback");
    AudioSessionDeactiveEvent deactiveEvent;
    deactiveEvent.deactiveReason = static_cast<AudioSessionDeactiveReason>(data.ReadInt32());
    OnAudioSessionDeactive(deactiveEvent);
}
} // namespace AudioStandard
} // namespace OHOS
