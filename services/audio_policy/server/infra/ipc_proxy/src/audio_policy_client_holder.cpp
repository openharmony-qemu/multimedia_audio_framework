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
#include "audio_errors.h"
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
    for (auto &deviceDesc : deviceChangeAction.deviceDescriptors) {
        CHECK_AND_CONTINUE_LOG(deviceDesc != nullptr, "deviceDesc is nullptr.");
        clientInfo.isSupportedDevice_ = CheckDeviceSupported(deviceDesc->deviceType_);
        deviceDesc->SetClientInfo(clientInfo);
    }
    audioPolicyClient_->OnDeviceChange(deviceChangeAction);
}

void AudioPolicyClientHolder::OnDeviceInfoUpdate(const DeviceChangeAction &deviceChangeAction)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    AudioDeviceDescriptor::ClientInfo clientInfo { apiVersion_ };
    for (auto &deviceDesc : deviceChangeAction.deviceDescriptors) {
        CHECK_AND_CONTINUE_LOG(deviceDesc != nullptr, "deviceDesc is nullptr.");
        clientInfo.isSupportedDevice_ = CheckDeviceSupported(deviceDesc->deviceType_);
        deviceDesc->SetClientInfo(clientInfo);
    }
    audioPolicyClient_->OnDeviceInfoUpdate(deviceChangeAction);
}

void AudioPolicyClientHolder::OnMicrophoneBlocked(const MicrophoneBlockedInfo &microphoneBlockedInfo)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    AudioDeviceDescriptor::ClientInfo clientInfo { apiVersion_ };
    for (auto &deviceDesc : microphoneBlockedInfo.devices) {
        CHECK_AND_CONTINUE_LOG(deviceDesc != nullptr, "deviceDesc is nullptr.");
        clientInfo.isSupportedDevice_ = CheckDeviceSupported(deviceDesc->deviceType_);
        deviceDesc->SetClientInfo(clientInfo);
    }
    audioPolicyClient_->OnMicrophoneBlocked(microphoneBlockedInfo);
}

void AudioPolicyClientHolder::OnRingerModeUpdated(const AudioRingerMode &ringerMode)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    audioPolicyClient_->OnRingerModeUpdated(ringerMode);
}

void AudioPolicyClientHolder::OnSelfAppVolumeChanged(int32_t appUid, const VolumeEvent& volumeEvent)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    audioPolicyClient_->OnSelfAppVolumeChanged(appUid, volumeEvent);
}

void AudioPolicyClientHolder::OnSystemAppVolumeChanged(int32_t appUid, const VolumeEvent& volumeEvent)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    audioPolicyClient_->OnSystemAppVolumeChanged(appUid, volumeEvent);
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

void AudioPolicyClientHolder::OnPreferredOutputDeviceUpdated(const AudioRendererInfo &rendererInfo, int32_t uid,
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    AudioDeviceDescriptor::ClientInfo clientInfo { apiVersion_ };
    for (auto &deviceDesc : desc) {
        CHECK_AND_CONTINUE_LOG(deviceDesc != nullptr, "deviceDesc is nullptr.");
        clientInfo.isSupportedDevice_ = CheckDeviceSupported(deviceDesc->deviceType_);
        deviceDesc->SetClientInfo(clientInfo);
    }
    audioPolicyClient_->OnPreferredOutputDeviceUpdated(rendererInfo, uid, desc);
}

void AudioPolicyClientHolder::OnPreferredInputDeviceUpdated(const AudioCapturerInfo &capturerInfo, int32_t uid,
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    AudioDeviceDescriptor::ClientInfo clientInfo { apiVersion_ };
    for (auto &deviceDesc : desc) {
        CHECK_AND_CONTINUE_LOG(deviceDesc != nullptr, "deviceDesc is nullptr.");
        clientInfo.isSupportedDevice_ = CheckDeviceSupported(deviceDesc->deviceType_);
        deviceDesc->SetClientInfo(clientInfo);
    }
    audioPolicyClient_->OnPreferredInputDeviceUpdated(capturerInfo, uid, desc);
}

void AudioPolicyClientHolder::OnRendererStateChange(
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    AudioDeviceDescriptor::ClientInfo clientInfo { hasBTPermission_, hasSystemPermission_, apiVersion_ };
    for (auto &audioRendererChangeInfo : audioRendererChangeInfos) {
        CHECK_AND_CONTINUE_LOG(audioRendererChangeInfo != nullptr, "audioRendererChangeInfo is nullptr.");
        clientInfo.isSupportedDevice_ = CheckDeviceSupported(audioRendererChangeInfo->outputDeviceInfo.deviceType_);
        audioRendererChangeInfo->SetClientInfo(clientInfo);
    }
    audioPolicyClient_->OnRendererStateChange(audioRendererChangeInfos);
}

void AudioPolicyClientHolder::OnCapturerStateChange(
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    AudioDeviceDescriptor::ClientInfo clientInfo { hasBTPermission_, hasSystemPermission_, apiVersion_ };
    for (auto &audioCapturerChangeInfo : audioCapturerChangeInfos) {
        CHECK_AND_CONTINUE_LOG(audioCapturerChangeInfo != nullptr, "audioCapturerChangeInfo is nullptr.");
        clientInfo.isSupportedDevice_ = CheckDeviceSupported(audioCapturerChangeInfo->inputDeviceInfo.deviceType_);
        audioCapturerChangeInfo->SetClientInfo(clientInfo);
    }
    audioPolicyClient_->OnCapturerStateChange(audioCapturerChangeInfos);
}

void AudioPolicyClientHolder::OnRendererDeviceChange(const uint32_t sessionId,
    const AudioDeviceDescriptor &deviceInfo, const AudioStreamDeviceChangeReasonExt reason,
    const AudioDeviceDescriptor &preDeviceInfo)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    AudioDeviceDescriptor::ClientInfo clientInfo { apiVersion_ };
    clientInfo.isSupportedDevice_ = CheckDeviceSupported(deviceInfo.deviceType_);
    deviceInfo.SetClientInfo(clientInfo);
    clientInfo.isSupportedDevice_ = CheckDeviceSupported(preDeviceInfo.deviceType_);
    preDeviceInfo.SetClientInfo(clientInfo);
    audioPolicyClient_->OnRendererDeviceChange(sessionId, deviceInfo, reason, preDeviceInfo);
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
        clientInfo.isSupportedDevice_ = CheckDeviceSupported(deviceDescriptor->deviceType_);
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
        clientInfo.isSupportedDevice_ = CheckDeviceSupported(deviceDescriptor->deviceType_);
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

void AudioPolicyClientHolder::OnPreferredDeviceSet(PreferredType preferredType,
    const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc, int32_t uid, const std::string &caller)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    audioPolicyClient_->OnPreferredDeviceSet(preferredType, deviceDesc, uid, caller);
}

void AudioPolicyClientHolder::OnCollaborationEnabledChangeForCurrentDevice(const bool &enabled)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    audioPolicyClient_->OnCollaborationEnabledChangeForCurrentDevice(enabled);
}

void AudioPolicyClientHolder::OnAdaptiveSpatialRenderingEnabledChangeForAnyDevice(
    const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor, const bool &enabled)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    CHECK_AND_RETURN_LOG(deviceDescriptor != nullptr, "deviceDescriptor is nullptr.");
    if (hasSystemPermission_) {
        AudioDeviceDescriptor::ClientInfo clientInfo { apiVersion_ };
        clientInfo.isSupportedDevice_ = CheckDeviceSupported(deviceDescriptor->deviceType_);
        deviceDescriptor->SetClientInfo(clientInfo);
        audioPolicyClient_->OnAdaptiveSpatialRenderingEnabledChangeForAnyDevice(deviceDescriptor, enabled);
    } else {
        audioPolicyClient_->OnAdaptiveSpatialRenderingEnabledChangeForAnyDevice(deviceDescriptor, false);
    }
}

void AudioPolicyClientHolder::OnPersonalizedSpatializationEnabledChangeForAnyDevice(
    const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor, const bool &enabled)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    CHECK_AND_RETURN_LOG(deviceDescriptor != nullptr, "deviceDescriptor is nullptr.");
    if (hasSystemPermission_) {
        AudioDeviceDescriptor::ClientInfo clientInfo { apiVersion_ };
        clientInfo.isSupportedDevice_ = CheckDeviceSupported(deviceDescriptor->deviceType_);
        deviceDescriptor->SetClientInfo(clientInfo);
        AUDIO_INFO_LOG("OnPersonalizedSpatializationEnabledChangeForAnyDevice %{public}d", enabled);
        audioPolicyClient_->OnPersonalizedSpatializationEnabledChangeForAnyDevice(deviceDescriptor, enabled);
    } else {
        AUDIO_INFO_LOG("OnPersonalizedSpatializationEnabledChangeForAnyDevice no permission");
        audioPolicyClient_->OnPersonalizedSpatializationEnabledChangeForAnyDevice(deviceDescriptor, false);
    }
}

void AudioPolicyClientHolder::OnSpatializationEnabledChangeForBTDevice(
    const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor, const bool &enabled,
    const bool &headTrackingEnabled)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    CHECK_AND_RETURN_LOG(deviceDescriptor != nullptr, "deviceDescriptor is nullptr.");
    if (hasSystemPermission_) {
        AudioDeviceDescriptor::ClientInfo clientInfo { apiVersion_ };
        clientInfo.isSupportedDevice_ = CheckDeviceSupported(deviceDescriptor->deviceType_);
        deviceDescriptor->SetClientInfo(clientInfo);
        AUDIO_INFO_LOG("OnSpatializationEnabledChangeForBTDevice %{public}d %{public}d", enabled, headTrackingEnabled);
        audioPolicyClient_->OnSpatializationEnabledChangeForBTDevice(deviceDescriptor, enabled, headTrackingEnabled);
    } else {
        AUDIO_INFO_LOG("OnSpatializationEnabledChangeForBTDevice 0 0");
        audioPolicyClient_->OnSpatializationEnabledChangeForBTDevice(deviceDescriptor, false, false);
    }
}

void AudioPolicyClientHolder::OnSpatialAudioSourceTypeChange(const SpatialAudioSourceType &mode)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    if (hasSystemPermission_) {
        AUDIO_INFO_LOG("AudioPolicyClientHolder::OnSpatialAudioSourceTypeChange %{public}d", mode);
        audioPolicyClient_->OnSpatialAudioSourceTypeChange(mode);
    } else {
        AUDIO_INFO_LOG("AudioPolicyClientHolder::OnSpatialAudioSourceTypeChange 0 0");
        audioPolicyClient_->OnSpatialAudioSourceTypeChange(SPATIAL_AUDIO_SOURCE_TYPE_STEREO);
    }
}

void AudioPolicyClientHolder::OnAudioSeparationEffectEnabledChange(const bool &enabled)
{
    CHECK_AND_RETURN_LOG(audioPolicyClient_ != nullptr, "audioPolicyClient_ is nullptr.");
    audioPolicyClient_->OnAudioSeparationEffectEnabledChange(hasSystemPermission_ && enabled);
}

int32_t AudioPolicyClientHolder::OnAudioRouteSelectRefined(
    const std::shared_ptr<AudioRouteSelectInfo> &routeSelectInfo, int32_t &selectResult)
{
    CHECK_AND_RETURN_RET_LOG(audioPolicyClient_ != nullptr, ERROR, "audioPolicyClient_ is nullptr.");
    CHECK_AND_RETURN_RET_LOG(routeSelectInfo != nullptr, ERROR, "routeSelectInfo is nullptr.");
    return audioPolicyClient_->OnAudioRouteSelectRefined(routeSelectInfo, selectResult);
}

bool AudioPolicyClientHolder::CheckDeviceSupported(DeviceType devicetype)
{
    CHECK_AND_RETURN_RET(needDeclaredDevices_.contains(devicetype), true);
    return alreadyDeclaredDevicesByApp_.contains(devicetype);
}
} // namespace AudioStandard
} // namespace OHOS