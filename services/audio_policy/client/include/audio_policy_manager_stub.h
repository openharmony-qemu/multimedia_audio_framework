/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef AUDIO_POLICY_MANAGER_STUB_H
#define AUDIO_POLICY_MANAGER_STUB_H

#include "audio_policy_base.h"

namespace OHOS {
namespace AudioStandard {
class AudioPolicyManagerStub : public IRemoteStub<IAudioPolicy> {
public:
    virtual int32_t OnRemoteRequest(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option) override;

private:
    void GetMaxVolumeLevelInternal(MessageParcel &data, MessageParcel &reply);
    void GetMinVolumeLevelInternal(MessageParcel &data, MessageParcel &reply);
    void SetSystemVolumeLevelInternal(MessageParcel &data, MessageParcel &reply);
    void GetSystemVolumeLevelInternal(MessageParcel &data, MessageParcel &reply);
    void SetStreamMuteInternal(MessageParcel &data, MessageParcel &reply);
    void GetStreamMuteInternal(MessageParcel &data, MessageParcel &reply);
    void IsStreamActiveInternal(MessageParcel &data, MessageParcel &reply);
    void SetDeviceActiveInternal(MessageParcel &data, MessageParcel &reply);
    void IsDeviceActiveInternal(MessageParcel &data, MessageParcel &reply);
    void GetActiveOutputDeviceInternal(MessageParcel &data, MessageParcel &reply);
    void GetActiveInputDeviceInternal(MessageParcel &data, MessageParcel &reply);
    void SetRingerModeInternal(MessageParcel &data, MessageParcel &reply);
    void GetRingerModeInternal(MessageParcel &data, MessageParcel &reply);
    void SetAudioSceneInternal(MessageParcel &data, MessageParcel &reply);
    void GetAudioSceneInternal(MessageParcel &data, MessageParcel &reply);
    void SetMicrophoneMuteInternal(MessageParcel &data, MessageParcel &reply);
    void SetMicrophoneMuteAudioConfigInternal(MessageParcel &data, MessageParcel &reply);
    void IsMicrophoneMuteInternal(MessageParcel &data, MessageParcel &reply);
    void SetInterruptCallbackInternal(MessageParcel &data, MessageParcel &reply);
    void UnsetInterruptCallbackInternal(MessageParcel &data, MessageParcel &reply);
    void ActivateInterruptInternal(MessageParcel &data, MessageParcel &reply);
    void DeactivateInterruptInternal(MessageParcel &data, MessageParcel &reply);
    void SetAudioManagerInterruptCbInternal(MessageParcel &data, MessageParcel &reply);
    void UnsetAudioManagerInterruptCbInternal(MessageParcel &data, MessageParcel &reply);
    void RequestAudioFocusInternal(MessageParcel &data, MessageParcel &reply);
    void AbandonAudioFocusInternal(MessageParcel &data, MessageParcel &reply);
    void GetStreamInFocusInternal(MessageParcel &data, MessageParcel &reply);
    void GetSessionInfoInFocusInternal(MessageParcel &data, MessageParcel &reply);
    void GetDevicesInternal(MessageParcel &data, MessageParcel &reply);
    void NotifyCapturerAddedInternal(MessageParcel &data, MessageParcel &reply);
    void CheckRecordingCreateInternal(MessageParcel &data, MessageParcel &reply);
    void SelectOutputDeviceInternal(MessageParcel &data, MessageParcel &reply);
    void GetSelectedDeviceInfoInternal(MessageParcel &data, MessageParcel &reply);
    void SelectInputDeviceInternal(MessageParcel &data, MessageParcel &reply);
    void ReconfigureAudioChannelInternal(MessageParcel &data, MessageParcel &reply);
    void GetAudioLatencyFromXmlInternal(MessageParcel &data, MessageParcel &reply);
    void GetSinkLatencyFromXmlInternal(MessageParcel &data, MessageParcel &reply);
    void RegisterTrackerInternal(MessageParcel &data, MessageParcel &reply);
    void UpdateTrackerInternal(MessageParcel &data, MessageParcel &reply);
    void GetRendererChangeInfosInternal(MessageParcel &data, MessageParcel &reply);
    void GetCapturerChangeInfosInternal(MessageParcel &data, MessageParcel &reply);
    void SetLowPowerVolumeInternal(MessageParcel &data, MessageParcel &reply);
    void GetLowPowerVolumeInternal(MessageParcel &data, MessageParcel &reply);
    void UpdateStreamStateInternal(MessageParcel& data, MessageParcel& reply);
    void GetSingleStreamVolumeInternal(MessageParcel &data, MessageParcel &reply);
    void GetVolumeGroupInfoInternal(MessageParcel& data, MessageParcel& reply);
    void GetNetworkIdByGroupIdInternal(MessageParcel& data, MessageParcel& reply);
#ifdef FEATURE_DTMF_TONE
    void GetToneInfoInternal(MessageParcel &data, MessageParcel &reply);
    void GetSupportedTonesInternal(MessageParcel &data, MessageParcel &reply);
#endif
    void IsAudioRendererLowLatencySupportedInternal(MessageParcel &data, MessageParcel &reply);
    void CheckRecordingStateChangeInternal(MessageParcel &data, MessageParcel &reply);
    void GetPreferredOutputDeviceDescriptorsInternal(MessageParcel &data, MessageParcel &reply);
    void GetPreferredInputDeviceDescriptorsInternal(MessageParcel &data, MessageParcel &reply);
    void GetAudioFocusInfoListInternal(MessageParcel &data, MessageParcel &reply);
    void SetSystemSoundUriInternal(MessageParcel &data, MessageParcel &reply);
    void GetSystemSoundUriInternal(MessageParcel &data, MessageParcel &reply);
    void GetMinStreamVolumeInternal(MessageParcel &data, MessageParcel &reply);
    void GetMaxStreamVolumeInternal(MessageParcel &data, MessageParcel &reply);
    void GetMaxRendererInstancesInternal(MessageParcel &data, MessageParcel &reply);
    void IsVolumeUnadjustableInternal(MessageParcel &data, MessageParcel &reply);
    void AdjustVolumeByStepInternal(MessageParcel &data, MessageParcel &reply);
    void AdjustSystemVolumeByStepInternal(MessageParcel &data, MessageParcel &reply);
    void GetSystemVolumeInDbInternal(MessageParcel &data, MessageParcel &reply);
    void QueryEffectSceneModeInternal(MessageParcel &data, MessageParcel &reply);
    void SetPlaybackCapturerFilterInfosInternal(MessageParcel &data, MessageParcel &reply);
    void SetCaptureSilentStateInternal(MessageParcel &data, MessageParcel &reply);
    void GetHardwareOutputSamplingRateInternal(MessageParcel &data, MessageParcel &reply);
    void GetAudioCapturerMicrophoneDescriptorsInternal(MessageParcel &data, MessageParcel &reply);
    void GetAvailableMicrophonesInternal(MessageParcel &data, MessageParcel &reply);
    void SetDeviceAbsVolumeSupportedInternal(MessageParcel &data, MessageParcel &reply);
    void IsAbsVolumeSceneInternal(MessageParcel &data, MessageParcel &reply);
    void SetA2dpDeviceVolumeInternal(MessageParcel &data, MessageParcel &reply);
    void ReadStreamChangeInfo(MessageParcel &data, const AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo);
    void WriteAudioFocusInfo(MessageParcel &data,
        const std::pair<AudioInterrupt, AudioFocuState> &focusInfo);
    void GetAvailableDevicesInternal(MessageParcel &data, MessageParcel &reply);
    void SetAvailableDeviceChangeCallbackInternal(MessageParcel &data, MessageParcel &reply);
    void UnsetAvailableDeviceChangeCallbackInternal(MessageParcel &data, MessageParcel &reply);
    void IsSpatializationEnabledInternal(MessageParcel &data, MessageParcel &reply);
    void SetSpatializationEnabledInternal(MessageParcel &data, MessageParcel &reply);
    void IsHeadTrackingEnabledInternal(MessageParcel &data, MessageParcel &reply);
    void SetHeadTrackingEnabledInternal(MessageParcel &data, MessageParcel &reply);
    void RegisterSpatializationEnabledEventListenerInternal(MessageParcel &data, MessageParcel &reply);
    void RegisterHeadTrackingEnabledEventListenerInternal(MessageParcel &data, MessageParcel &reply);
    void UnregisterSpatializationEnabledEventListenerInternal(MessageParcel &data, MessageParcel &reply);
    void UnregisterHeadTrackingEnabledEventListenerInternal(MessageParcel &data, MessageParcel &reply);
    void GetSpatializationStateInternal(MessageParcel &data, MessageParcel &reply);
    void IsSpatializationSupportedInternal(MessageParcel &data, MessageParcel &reply);
    void IsSpatializationSupportedForDeviceInternal(MessageParcel &data, MessageParcel &reply);
    void IsHeadTrackingSupportedInternal(MessageParcel &data, MessageParcel &reply);
    void IsHeadTrackingSupportedForDeviceInternal(MessageParcel &data, MessageParcel &reply);
    void UpdateSpatialDeviceStateInternal(MessageParcel &data, MessageParcel &reply);
    void RegisterSpatializationStateEventListenerInternal(MessageParcel &data, MessageParcel &reply);
    void ConfigDistributedRoutingRoleInternal(MessageParcel &data, MessageParcel &reply);
    void SetDistributedRoutingRoleCallbackInternal(MessageParcel &data, MessageParcel &reply);
    void UnsetDistributedRoutingRoleCallbackInternal(MessageParcel &data, MessageParcel &reply);
    void UnregisterSpatializationStateEventListenerInternal(MessageParcel &data, MessageParcel &reply);
    void RegisterPolicyCallbackClientInternal(MessageParcel &data, MessageParcel &reply);
    void CreateAudioInterruptZoneInternal(MessageParcel &data, MessageParcel &reply);
    void AddAudioInterruptZonePidsInternal(MessageParcel &data, MessageParcel &reply);
    void RemoveAudioInterruptZonePidsInternal(MessageParcel &data, MessageParcel &reply);
    void ReleaseAudioInterruptZoneInternal(MessageParcel &data, MessageParcel &reply);
    void SetCallDeviceActiveInternal(MessageParcel &data, MessageParcel &reply);
    void GetActiveBluetoothDeviceInternal(MessageParcel &data, MessageParcel &reply);

    using HandlerFunc = void(AudioPolicyManagerStub::*)(MessageParcel &data, MessageParcel &reply);
    static inline HandlerFunc handlers[] = {
        &AudioPolicyManagerStub::GetMaxVolumeLevelInternal,
        &AudioPolicyManagerStub::GetMinVolumeLevelInternal,
        &AudioPolicyManagerStub::SetSystemVolumeLevelInternal,
        &AudioPolicyManagerStub::GetSystemVolumeLevelInternal,
        &AudioPolicyManagerStub::SetStreamMuteInternal,
        &AudioPolicyManagerStub::GetStreamMuteInternal,
        &AudioPolicyManagerStub::IsStreamActiveInternal,
        &AudioPolicyManagerStub::SetDeviceActiveInternal,
        &AudioPolicyManagerStub::IsDeviceActiveInternal,
        &AudioPolicyManagerStub::GetActiveOutputDeviceInternal,
        &AudioPolicyManagerStub::GetActiveInputDeviceInternal,
        &AudioPolicyManagerStub::SetRingerModeInternal,
        &AudioPolicyManagerStub::GetRingerModeInternal,
        &AudioPolicyManagerStub::SetAudioSceneInternal,
        &AudioPolicyManagerStub::GetAudioSceneInternal,
        &AudioPolicyManagerStub::SetMicrophoneMuteInternal,
        &AudioPolicyManagerStub::SetMicrophoneMuteAudioConfigInternal,
        &AudioPolicyManagerStub::IsMicrophoneMuteInternal,
        &AudioPolicyManagerStub::SetInterruptCallbackInternal,
        &AudioPolicyManagerStub::UnsetInterruptCallbackInternal,
        &AudioPolicyManagerStub::ActivateInterruptInternal,
        &AudioPolicyManagerStub::DeactivateInterruptInternal,
        &AudioPolicyManagerStub::SetAudioManagerInterruptCbInternal,
        &AudioPolicyManagerStub::UnsetAudioManagerInterruptCbInternal,
        &AudioPolicyManagerStub::RequestAudioFocusInternal,
        &AudioPolicyManagerStub::AbandonAudioFocusInternal,
        &AudioPolicyManagerStub::GetStreamInFocusInternal,
        &AudioPolicyManagerStub::GetSessionInfoInFocusInternal,
        &AudioPolicyManagerStub::GetDevicesInternal,
        &AudioPolicyManagerStub::NotifyCapturerAddedInternal,
        &AudioPolicyManagerStub::CheckRecordingCreateInternal,
        &AudioPolicyManagerStub::SelectOutputDeviceInternal,
        &AudioPolicyManagerStub::GetSelectedDeviceInfoInternal,
        &AudioPolicyManagerStub::SelectInputDeviceInternal,
        &AudioPolicyManagerStub::ReconfigureAudioChannelInternal,
        &AudioPolicyManagerStub::GetAudioLatencyFromXmlInternal,
        &AudioPolicyManagerStub::GetSinkLatencyFromXmlInternal,
        &AudioPolicyManagerStub::RegisterTrackerInternal,
        &AudioPolicyManagerStub::UpdateTrackerInternal,
        &AudioPolicyManagerStub::GetRendererChangeInfosInternal,
        &AudioPolicyManagerStub::GetCapturerChangeInfosInternal,
        &AudioPolicyManagerStub::SetLowPowerVolumeInternal,
        &AudioPolicyManagerStub::GetLowPowerVolumeInternal,
        &AudioPolicyManagerStub::UpdateStreamStateInternal,
        &AudioPolicyManagerStub::GetSingleStreamVolumeInternal,
        &AudioPolicyManagerStub::GetVolumeGroupInfoInternal,
        &AudioPolicyManagerStub::GetNetworkIdByGroupIdInternal,
#ifdef FEATURE_DTMF_TONE
        &AudioPolicyManagerStub::GetToneInfoInternal,
        &AudioPolicyManagerStub::GetSupportedTonesInternal,
#endif
        &AudioPolicyManagerStub::IsAudioRendererLowLatencySupportedInternal,
        &AudioPolicyManagerStub::CheckRecordingStateChangeInternal,
        &AudioPolicyManagerStub::GetPreferredOutputDeviceDescriptorsInternal,
        &AudioPolicyManagerStub::GetPreferredInputDeviceDescriptorsInternal,
        &AudioPolicyManagerStub::GetAudioFocusInfoListInternal,
        &AudioPolicyManagerStub::SetSystemSoundUriInternal,
        &AudioPolicyManagerStub::GetSystemSoundUriInternal,
        &AudioPolicyManagerStub::GetMinStreamVolumeInternal,
        &AudioPolicyManagerStub::GetMaxStreamVolumeInternal,
        &AudioPolicyManagerStub::GetMaxRendererInstancesInternal,
        &AudioPolicyManagerStub::IsVolumeUnadjustableInternal,
        &AudioPolicyManagerStub::AdjustVolumeByStepInternal,
        &AudioPolicyManagerStub::AdjustSystemVolumeByStepInternal,
        &AudioPolicyManagerStub::GetSystemVolumeInDbInternal,
        &AudioPolicyManagerStub::QueryEffectSceneModeInternal,
        &AudioPolicyManagerStub::SetPlaybackCapturerFilterInfosInternal,
        &AudioPolicyManagerStub::SetCaptureSilentStateInternal,
        &AudioPolicyManagerStub::GetHardwareOutputSamplingRateInternal,
        &AudioPolicyManagerStub::GetAudioCapturerMicrophoneDescriptorsInternal,
        &AudioPolicyManagerStub::GetAvailableMicrophonesInternal,
        &AudioPolicyManagerStub::SetDeviceAbsVolumeSupportedInternal,
        &AudioPolicyManagerStub::IsAbsVolumeSceneInternal,
        &AudioPolicyManagerStub::SetA2dpDeviceVolumeInternal,
        &AudioPolicyManagerStub::GetAvailableDevicesInternal,
        &AudioPolicyManagerStub::SetAvailableDeviceChangeCallbackInternal,
        &AudioPolicyManagerStub::UnsetAvailableDeviceChangeCallbackInternal,
        &AudioPolicyManagerStub::IsSpatializationEnabledInternal,
        &AudioPolicyManagerStub::SetSpatializationEnabledInternal,
        &AudioPolicyManagerStub::IsHeadTrackingEnabledInternal,
        &AudioPolicyManagerStub::SetHeadTrackingEnabledInternal,
        &AudioPolicyManagerStub::RegisterSpatializationEnabledEventListenerInternal,
        &AudioPolicyManagerStub::RegisterHeadTrackingEnabledEventListenerInternal,
        &AudioPolicyManagerStub::UnregisterSpatializationEnabledEventListenerInternal,
        &AudioPolicyManagerStub::UnregisterHeadTrackingEnabledEventListenerInternal,
        &AudioPolicyManagerStub::GetSpatializationStateInternal,
        &AudioPolicyManagerStub::IsSpatializationSupportedInternal,
        &AudioPolicyManagerStub::IsSpatializationSupportedForDeviceInternal,
        &AudioPolicyManagerStub::IsHeadTrackingSupportedInternal,
        &AudioPolicyManagerStub::IsHeadTrackingSupportedForDeviceInternal,
        &AudioPolicyManagerStub::UpdateSpatialDeviceStateInternal,
        &AudioPolicyManagerStub::RegisterSpatializationStateEventListenerInternal,
        &AudioPolicyManagerStub::ConfigDistributedRoutingRoleInternal,
        &AudioPolicyManagerStub::SetDistributedRoutingRoleCallbackInternal,
        &AudioPolicyManagerStub::UnsetDistributedRoutingRoleCallbackInternal,
        &AudioPolicyManagerStub::UnregisterSpatializationStateEventListenerInternal,
        &AudioPolicyManagerStub::RegisterPolicyCallbackClientInternal,
        &AudioPolicyManagerStub::CreateAudioInterruptZoneInternal,
        &AudioPolicyManagerStub::AddAudioInterruptZonePidsInternal,
        &AudioPolicyManagerStub::RemoveAudioInterruptZonePidsInternal,
        &AudioPolicyManagerStub::ReleaseAudioInterruptZoneInternal,
        &AudioPolicyManagerStub::SetCallDeviceActiveInternal,
        &AudioPolicyManagerStub::GetActiveBluetoothDeviceInternal,
    };
    static constexpr size_t handlersNums = sizeof(handlers) / sizeof(HandlerFunc);
    static_assert(handlersNums == (static_cast<size_t> (AudioPolicyInterfaceCode::AUDIO_POLICY_MANAGER_CODE_MAX) + 1),
        "please check audio_policy_ipc_interface_code");
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_POLICY_MANAGER_STUB_H
