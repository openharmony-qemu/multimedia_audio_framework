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

#include "audio_policy_client_holder.h"
#include "audio_service_log.h"
#include "audio_utils.h"
#include "audio_policy_utils.h"

namespace OHOS {
namespace AudioStandard {
namespace {
const std::string NEARLINK_LIST = "audio_nearlink_list";
}
void AudioPolicyClientHolder::OnVolumeKeyEvent(VolumeEvent volumeEvent)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    audioPolicyClient_->OnVolumeKeyEvent(volumeEvent);
}

void AudioPolicyClientHolder::OnAudioFocusInfoChange(
    const std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    audioPolicyClient_->OnAudioFocusInfoChange(ToIpcInterrupts(focusInfoList));
}

void AudioPolicyClientHolder::OnAudioFocusRequested(const AudioInterrupt &requestFocus)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    audioPolicyClient_->OnAudioFocusRequested(requestFocus);
}

void AudioPolicyClientHolder::OnAudioFocusAbandoned(const AudioInterrupt &abandonFocus)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    audioPolicyClient_->OnAudioFocusAbandoned(abandonFocus);
}

void AudioPolicyClientHolder::OnDeviceChange(const DeviceChangeAction &deviceChangeAction)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    AudioDeviceDescriptor::ClientInfo clientInfo { apiVersion_ };
    clientInfo.isSupportedNearlink_ = isSupportedNearlink_;
    deviceChangeAction.SetClientInfo(clientInfo);
    audioPolicyClient_->OnDeviceChange(deviceChangeAction);
}

void AudioPolicyClientHolder::OnDeviceInfoUpdate(const DeviceChangeAction &deviceChangeAction)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    AudioDeviceDescriptor::ClientInfo clientInfo { apiVersion_ };
    clientInfo.isSupportedNearlink_ = isSupportedNearlink_;
    deviceChangeAction.SetClientInfo(clientInfo);
    audioPolicyClient_->OnDeviceInfoUpdate(deviceChangeAction);
}

void AudioPolicyClientHolder::OnMicrophoneBlocked(const MicrophoneBlockedInfo &microphoneBlockedInfo)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    AudioDeviceDescriptor::ClientInfo clientInfo { apiVersion_ };
    clientInfo.isSupportedNearlink_ = isSupportedNearlink_;
    microphoneBlockedInfo.SetClientInfo(clientInfo);
    audioPolicyClient_->OnMicrophoneBlocked(microphoneBlockedInfo);
}

void AudioPolicyClientHolder::OnRingerModeUpdated(const AudioRingerMode &ringerMode)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    audioPolicyClient_->OnRingerModeUpdated(ringerMode);
}

void AudioPolicyClientHolder::OnAppVolumeChanged(int32_t appUid, const VolumeEvent& volumeEvent)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    audioPolicyClient_->OnAppVolumeChanged(appUid, volumeEvent);
}

void AudioPolicyClientHolder::OnActiveVolumeTypeChanged(const AudioVolumeType& volumeType)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    audioPolicyClient_->OnActiveVolumeTypeChanged(volumeType);
}

void AudioPolicyClientHolder::OnMicStateUpdated(const MicStateChangeEvent &micStateChangeEvent)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    audioPolicyClient_->OnMicStateUpdated(micStateChangeEvent);
}

void AudioPolicyClientHolder::OnPreferredOutputDeviceUpdated(const AudioRendererInfo &rendererInfo,
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    AudioDeviceDescriptor::ClientInfo clientInfo { apiVersion_ };
    clientInfo.isSupportedNearlink_ = isSupportedNearlink_;
    for (auto &deviceDesc : desc) {
        CHECK_AND_CONTINUE_LOG(deviceDesc != nullptr, "deviceDesc is nullptr.");
        deviceDesc->SetClientInfo(clientInfo);
    }
    audioPolicyClient_->OnPreferredOutputDeviceUpdated(rendererInfo, desc);
}

void AudioPolicyClientHolder::OnPreferredInputDeviceUpdated(const AudioCapturerInfo &capturerInfo,
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    AudioDeviceDescriptor::ClientInfo clientInfo { apiVersion_ };
    clientInfo.isSupportedNearlink_ = isSupportedNearlink_;
    for (auto &deviceDesc : desc) {
        CHECK_AND_CONTINUE_LOG(deviceDesc != nullptr, "deviceDesc is nullptr.");
        deviceDesc->SetClientInfo(clientInfo);
    }
    audioPolicyClient_->OnPreferredInputDeviceUpdated(capturerInfo, desc);
}

void AudioPolicyClientHolder::OnRendererStateChange(
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    AudioDeviceDescriptor::ClientInfo clientInfo { hasBTPermission_, hasSystemPermission_, apiVersion_ };
    clientInfo.isSupportedNearlink_ = isSupportedNearlink_;
    for (auto &audioRendererChangeInfo : audioRendererChangeInfos) {
        CHECK_AND_CONTINUE_LOG(audioRendererChangeInfo != nullptr, "audioRendererChangeInfo is nullptr.");
        audioRendererChangeInfo->SetClientInfo(clientInfo);
    }
    audioPolicyClient_->OnRendererStateChange(audioRendererChangeInfos);
}

void AudioPolicyClientHolder::OnCapturerStateChange(
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    AudioDeviceDescriptor::ClientInfo clientInfo { hasBTPermission_, hasSystemPermission_, apiVersion_ };
    clientInfo.isSupportedNearlink_ = isSupportedNearlink_;
    for (auto &audioCapturerChangeInfo : audioCapturerChangeInfos) {
        CHECK_AND_CONTINUE_LOG(audioCapturerChangeInfo != nullptr, "audioCapturerChangeInfo is nullptr.");
        audioCapturerChangeInfo->SetClientInfo(clientInfo);
    }
    audioPolicyClient_->OnCapturerStateChange(audioCapturerChangeInfos);
}

void AudioPolicyClientHolder::OnRendererDeviceChange(const uint32_t sessionId,
    const AudioDeviceDescriptor &deviceInfo, const AudioStreamDeviceChangeReasonExt reason)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    AudioDeviceDescriptor::ClientInfo clientInfo { apiVersion_ };
    clientInfo.isSupportedNearlink_ = isSupportedNearlink_;
    deviceInfo.SetClientInfo(clientInfo);
    audioPolicyClient_->OnRendererDeviceChange(sessionId, deviceInfo, reason);
}

void AudioPolicyClientHolder::OnRecreateRendererStreamEvent(const uint32_t sessionId, const int32_t streamFlag,
    const AudioStreamDeviceChangeReasonExt reason)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    audioPolicyClient_->OnRecreateRendererStreamEvent(sessionId, streamFlag, reason);
}

void AudioPolicyClientHolder::OnRecreateCapturerStreamEvent(const uint32_t sessionId, const int32_t streamFlag,
    const AudioStreamDeviceChangeReasonExt reason)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    audioPolicyClient_->OnRecreateCapturerStreamEvent(sessionId, streamFlag, reason);
}

void AudioPolicyClientHolder::OnHeadTrackingDeviceChange(const std::unordered_map<std::string, bool> &changeInfo)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    audioPolicyClient_->OnHeadTrackingDeviceChange(changeInfo);
}

void AudioPolicyClientHolder::OnSpatializationEnabledChange(const bool &enabled)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    if (hasSystemPermission_) {
        audioPolicyClient_->OnSpatializationEnabledChange(enabled);
    } else {
        audioPolicyClient_->OnSpatializationEnabledChange(false);
    }
}

void AudioPolicyClientHolder::OnSpatializationEnabledChangeForAnyDevice(
    const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor, const bool &enabled)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    CHECK_AND_RETURN_LOG(deviceDescriptor != nullptr, "deviceDescriptor is nullptr.");
    if (hasSystemPermission_) {
        AudioDeviceDescriptor::ClientInfo clientInfo { apiVersion_ };
        clientInfo.isSupportedNearlink_ = isSupportedNearlink_;
        deviceDescriptor->SetClientInfo(clientInfo);
        audioPolicyClient_->OnSpatializationEnabledChangeForAnyDevice(deviceDescriptor, enabled);
    } else {
        audioPolicyClient_->OnSpatializationEnabledChangeForAnyDevice(deviceDescriptor, false);
    }
}

void AudioPolicyClientHolder::OnSpatializationEnabledChangeForCurrentDevice(const bool &enabled)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    audioPolicyClient_->OnSpatializationEnabledChangeForCurrentDevice(enabled);
}

void AudioPolicyClientHolder::OnHeadTrackingEnabledChange(const bool &enabled)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    if (hasSystemPermission_) {
        audioPolicyClient_->OnHeadTrackingEnabledChange(enabled);
    } else {
        audioPolicyClient_->OnHeadTrackingEnabledChange(false);
    }
}

void AudioPolicyClientHolder::OnHeadTrackingEnabledChangeForAnyDevice(
    const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor, const bool &enabled)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    CHECK_AND_RETURN_LOG(deviceDescriptor != nullptr, "deviceDescriptor is nullptr.");
    if (hasSystemPermission_) {
        AudioDeviceDescriptor::ClientInfo clientInfo { apiVersion_ };
        clientInfo.isSupportedNearlink_ = isSupportedNearlink_;
        deviceDescriptor->SetClientInfo(clientInfo);
        audioPolicyClient_->OnHeadTrackingEnabledChangeForAnyDevice(deviceDescriptor, enabled);
    } else {
        audioPolicyClient_->OnHeadTrackingEnabledChangeForAnyDevice(deviceDescriptor, false);
    }
}

void AudioPolicyClientHolder::OnNnStateChange(const int32_t &nnState)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    audioPolicyClient_->OnNnStateChange(nnState);
}

void AudioPolicyClientHolder::OnAudioSessionDeactive(const AudioSessionDeactiveEvent &deactiveEvent)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    audioPolicyClient_->OnAudioSessionDeactive(static_cast<int32_t>(deactiveEvent.deactiveReason));
}

void AudioPolicyClientHolder::OnAudioSceneChange(const AudioScene &audioScene)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    audioPolicyClient_->OnAudioSceneChange(audioScene);
}

void AudioPolicyClientHolder::OnFormatUnsupportedError(const AudioErrors &errorCode)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    audioPolicyClient_->OnFormatUnsupportedError(errorCode);
}

void AudioPolicyClientHolder::OnStreamVolumeChange(StreamVolumeEvent streamVolumeEvent)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    audioPolicyClient_->OnStreamVolumeChange(streamVolumeEvent);
}

void AudioPolicyClientHolder::OnSystemVolumeChange(VolumeEvent volumeEvent)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    audioPolicyClient_->OnSystemVolumeChange(volumeEvent);
}

void AudioPolicyClientHolder::OnAudioSessionStateChanged(const AudioSessionStateChangedEvent &stateChangedEvent)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    int32_t stateChangeHint = static_cast<int32_t>(stateChangedEvent.stateChangeHint);
    audioPolicyClient_->OnAudioSessionStateChanged(stateChangeHint);
}

void AudioPolicyClientHolder::OnAudioSessionCurrentDeviceChanged(
    const CurrentOutputDeviceChangedEvent &deviceChangedEvent)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    audioPolicyClient_->OnAudioSessionCurrentDeviceChanged(deviceChangedEvent);
}

void AudioPolicyClientHolder::OnAudioSessionCurrentInputDeviceChanged(
    const CurrentInputDeviceChangedEvent &deviceChangedEvent)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    audioPolicyClient_->OnAudioSessionCurrentInputDeviceChanged(deviceChangedEvent);
}

void AudioPolicyClientHolder::OnVolumeDegreeEvent(const VolumeEvent &volumeEvent)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    audioPolicyClient_->OnVolumeDegreeEvent(volumeEvent);
}

void AudioPolicyClientHolder::OnCollaborationEnabledChangeForCurrentDevice(const bool &enabled)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    audioPolicyClient_->OnCollaborationEnabledChangeForCurrentDevice(enabled);
}
} // namespace AudioStandard
} // namespace OHOS