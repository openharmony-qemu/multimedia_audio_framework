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

using namespace std;
namespace OHOS {
namespace AudioStandard {

static const int32_t FOCUS_INFO_VALID_SIZE = 128;
static const int32_t DEVICE_CHANGE_VALID_SIZE = 128;
static const int32_t PREFERRED_DEVICE_VALID_SIZE = 128;
static const int32_t STATE_VALID_SIZE = 128;

AudioPolicyClientStub::AudioPolicyClientStub()
{}

AudioPolicyClientStub::~AudioPolicyClientStub()
{}

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
            (this->*handlers[updateCode])(data, reply);
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
        deviceChange.deviceDescriptors.emplace_back(AudioDeviceDescriptor::Unmarshalling(data));
    }
    OnDeviceChange(deviceChange);
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
        deviceDescriptor.push_back(AudioDeviceDescriptor::Unmarshalling(data));
    }
    OnPreferredOutputDeviceUpdated(deviceDescriptor);
}

void AudioPolicyClientStub::HandlePreferredInputDeviceUpdated(MessageParcel &data, MessageParcel &reply)
{
    std::vector<sptr<AudioDeviceDescriptor>> deviceDescriptor;
    int32_t size = data.ReadInt32();
    CHECK_AND_RETURN_LOG(size < PREFERRED_DEVICE_VALID_SIZE, "get invalid size : %{public}d", size);

    for (int32_t i = 0; i < size; i++) {
        deviceDescriptor.push_back(AudioDeviceDescriptor::Unmarshalling(data));
    }
    OnPreferredInputDeviceUpdated(deviceDescriptor);
}

void AudioPolicyClientStub::HandleRendererStateChange(MessageParcel &data, MessageParcel &reply)
{
    std::vector<std::unique_ptr<AudioRendererChangeInfo>> audioRenderChangeInfo;
    int32_t size = data.ReadInt32();
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
    DeviceInfo deviceInfo;
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
    sptr<AudioDeviceDescriptor> audioDeviceDescriptor = AudioDeviceDescriptor::Unmarshalling(data);
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
    sptr<AudioDeviceDescriptor> audioDeviceDescriptor = AudioDeviceDescriptor::Unmarshalling(data);
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
