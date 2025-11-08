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

#ifndef AUDIO_POLICY_CLIENT_HOLDER_H
#define AUDIO_POLICY_CLIENT_HOLDER_H

#include "audio_policy_client.h"
#include "iaudio_policy_client.h"

namespace OHOS {
namespace AudioStandard {
class AudioPolicyClientHolder {
public:
    AudioPolicyClientHolder(sptr<IAudioPolicyClient> audioPolicyClient)
        :audioPolicyClient_(audioPolicyClient) {}

    void OnVolumeKeyEvent(VolumeEvent volumeEvent);
    void OnAudioFocusInfoChange(const std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList);
    void OnAudioFocusRequested(const AudioInterrupt &requestFocus);
    void OnAudioFocusAbandoned(const AudioInterrupt &abandonFocus);
    void OnDeviceChange(const DeviceChangeAction &deviceChangeAction);
    void OnDeviceInfoUpdate(const DeviceChangeAction &deviceChangeAction);
    void OnMicrophoneBlocked(const MicrophoneBlockedInfo &microphoneBlockedInfo);
    void OnRingerModeUpdated(const AudioRingerMode &ringerMode);
    void OnAppVolumeChanged(int32_t appUid, const VolumeEvent& volumeEvent);
    void OnActiveVolumeTypeChanged(const AudioVolumeType& volumeType);
    void OnMicStateUpdated(const MicStateChangeEvent &micStateChangeEvent);
    void OnPreferredOutputDeviceUpdated(const AudioRendererInfo &rendererInfo,
        const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc);
    void OnPreferredInputDeviceUpdated(const AudioCapturerInfo &capturerInfo,
        const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc);
    void OnRendererStateChange(
        std::vector<std::shared_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos);
    void OnCapturerStateChange(
        std::vector<std::shared_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos);
    void OnRendererDeviceChange(const uint32_t sessionId,
        const AudioDeviceDescriptor &deviceInfo, const AudioStreamDeviceChangeReasonExt reason);
    void OnRecreateRendererStreamEvent(const uint32_t sessionId, const int32_t streamFlag,
        const AudioStreamDeviceChangeReasonExt reason);
    void OnRecreateCapturerStreamEvent(const uint32_t sessionId, const int32_t streamFlag,
        const AudioStreamDeviceChangeReasonExt reason);
    void OnHeadTrackingDeviceChange(const std::unordered_map<std::string, bool> &changeInfo);
    void OnSpatializationEnabledChange(const bool &enabled);
    void OnSpatializationEnabledChangeForAnyDevice(
        const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor, const bool &enabled);
    void OnSpatializationEnabledChangeForCurrentDevice(const bool &enabled);
    void OnHeadTrackingEnabledChange(const bool &enabled);
    void OnHeadTrackingEnabledChangeForAnyDevice(
        const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor, const bool &enabled);
    void OnNnStateChange(const int32_t &nnState);
    void OnAudioSessionDeactive(const AudioSessionDeactiveEvent &deactiveEvent);
    void OnAudioSceneChange(const AudioScene &audioScene);
    void OnFormatUnsupportedError(const AudioErrors &errorCode);
    void OnStreamVolumeChange(StreamVolumeEvent streamVolumeEvent);
    void OnSystemVolumeChange(VolumeEvent volumeEvent);
    void OnAudioSessionStateChanged(const AudioSessionStateChangedEvent &stateChangedEvent);
    void OnAudioSessionCurrentDeviceChanged(const CurrentOutputDeviceChangedEvent &deviceChangedEvent);
    void OnAudioSessionCurrentInputDeviceChanged(const CurrentInputDeviceChangedEvent &deviceChangedEvent);
    void OnVolumeDegreeEvent(const VolumeEvent &volumeEvent);
    void OnCollaborationEnabledChangeForCurrentDevice(const bool &enabled);

public:
    bool hasBTPermission_ = true;
    bool hasSystemPermission_ = true;
    int32_t apiVersion_ = API_VERSION_MAX;
    bool isSupportedNearlink_ = true;

private:
    sptr<IAudioPolicyClient> audioPolicyClient_ = nullptr;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_POLICY_CLIENT_HOLDER_H