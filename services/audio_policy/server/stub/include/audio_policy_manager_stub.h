/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
    virtual bool IsArmUsbDevice(const AudioDeviceDescriptor &desc) = 0;
    virtual void MapExternalToInternalDeviceType(AudioDeviceDescriptor &desc) = 0;
protected:
    virtual int32_t GetApiTargetVersion() = 0;
private:
    void GetMaxVolumeLevelInternal(MessageParcel &data, MessageParcel &reply);
    void GetMinVolumeLevelInternal(MessageParcel &data, MessageParcel &reply);
    void SetSystemVolumeLevelLegacyInternal(MessageParcel &data, MessageParcel &reply);
    void SetSystemVolumeLevelInternal(MessageParcel &data, MessageParcel &reply);
    void SetSystemVolumeLevelWithDeviceInternal(MessageParcel &data, MessageParcel &reply);
    void SetAppVolumeLevelInternal(MessageParcel &data, MessageParcel &reply);
    void SetAppVolumeMutedInternal(MessageParcel &data, MessageParcel &reply);
    void SetSelfAppVolumeLevelInternal(MessageParcel &data, MessageParcel &reply);
    void GetSystemActiveVolumeTypeInternal(MessageParcel& data, MessageParcel& reply);
    void GetSystemVolumeLevelInternal(MessageParcel &data, MessageParcel &reply);
    void GetAppVolumeIsMuteInternal(MessageParcel &data, MessageParcel &reply);
    void GetAppVolumeLevelInternal(MessageParcel &data, MessageParcel &reply);
    void GetSelfAppVolumeLevelInternal(MessageParcel &data, MessageParcel &reply);
    void SetStreamMuteLegacyInternal(MessageParcel &data, MessageParcel &reply);
    void SetStreamMuteInternal(MessageParcel &data, MessageParcel &reply);
    void GetStreamMuteInternal(MessageParcel &data, MessageParcel &reply);
    void IsStreamActiveInternal(MessageParcel &data, MessageParcel &reply);
    void IsFastPlaybackSupportedInternal(MessageParcel &data, MessageParcel &reply);
    void IsFastRecordingSupportedInternal(MessageParcel &data, MessageParcel &reply);
    void SetDeviceActiveInternal(MessageParcel &data, MessageParcel &reply);
    void IsDeviceActiveInternal(MessageParcel &data, MessageParcel &reply);
    void GetActiveOutputDeviceInternal(MessageParcel &data, MessageParcel &reply);
    void GetDmDeviceTypeInternal(MessageParcel &data, MessageParcel &reply);
    void GetActiveInputDeviceInternal(MessageParcel &data, MessageParcel &reply);
    void GetOutputDeviceInternal(MessageParcel &data, MessageParcel &reply);
    void GetInputDeviceInternal(MessageParcel &data, MessageParcel &reply);
    void SetRingerModeLegacyInternal(MessageParcel &data, MessageParcel &reply);
    void SetRingerModeInternal(MessageParcel &data, MessageParcel &reply);
    void GetRingerModeInternal(MessageParcel &data, MessageParcel &reply);
    void SetAudioSceneInternal(MessageParcel &data, MessageParcel &reply);
    void GetAudioSceneInternal(MessageParcel &data, MessageParcel &reply);
    void SetMicrophoneMuteInternal(MessageParcel &data, MessageParcel &reply);
    void SetMicrophoneMuteAudioConfigInternal(MessageParcel &data, MessageParcel &reply);
    void IsMicrophoneMuteLegacyInternal(MessageParcel &data, MessageParcel &reply);
    void IsMicrophoneMuteInternal(MessageParcel &data, MessageParcel &reply);
    void SetInterruptCallbackInternal(MessageParcel &data, MessageParcel &reply);
    void UnsetInterruptCallbackInternal(MessageParcel &data, MessageParcel &reply);
    void ActivateInterruptInternal(MessageParcel &data, MessageParcel &reply);
    void DeactivateInterruptInternal(MessageParcel &data, MessageParcel &reply);
    void ActivatePreemptModeInternal(MessageParcel &data, MessageParcel &reply);
    void DeactivatePreemptModeInternal(MessageParcel &data, MessageParcel &reply);
    void SetAudioManagerInterruptCbInternal(MessageParcel &data, MessageParcel &reply);
    void UnsetAudioManagerInterruptCbInternal(MessageParcel &data, MessageParcel &reply);
    void RequestAudioFocusInternal(MessageParcel &data, MessageParcel &reply);
    void AbandonAudioFocusInternal(MessageParcel &data, MessageParcel &reply);
    void GetStreamInFocusInternal(MessageParcel &data, MessageParcel &reply);
    void GetStreamInFocusByUidInternal(MessageParcel &data, MessageParcel &reply);
    void GetSessionInfoInFocusInternal(MessageParcel &data, MessageParcel &reply);
    void GetDevicesInternal(MessageParcel &data, MessageParcel &reply);
    void SelectOutputDeviceInternal(MessageParcel &data, MessageParcel &reply);
    void GetSelectedDeviceInfoInternal(MessageParcel &data, MessageParcel &reply);
    void SelectInputDeviceInternal(MessageParcel &data, MessageParcel &reply);
    void ExcludeOutputDevicesInternal(MessageParcel &data, MessageParcel &reply);
    void UnexcludeOutputDevicesInternal(MessageParcel &data, MessageParcel &reply);
    void GetExcludedDevicesInternal(MessageParcel &data, MessageParcel &reply);
    void ReconfigureAudioChannelInternal(MessageParcel &data, MessageParcel &reply);
    void GetPreferredOutputStreamTypeInternal(MessageParcel &data, MessageParcel &reply);
    void GetPreferredInputStreamTypeInternal(MessageParcel &data, MessageParcel &reply);
    void CreateRendererClientInternal(MessageParcel &data, MessageParcel &reply);
    void CreateCapturerClientInternal(MessageParcel &data, MessageParcel &reply);
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
    void GetPreferredOutputDeviceDescriptorsInternal(MessageParcel &data, MessageParcel &reply);
    void GetPreferredInputDeviceDescriptorsInternal(MessageParcel &data, MessageParcel &reply);
    void SetClientCallbacksEnableInternal(MessageParcel &data, MessageParcel &reply);
    void SetCallbackRendererInfoInternal(MessageParcel &data, MessageParcel &reply);
    void SetCallbackCapturerInfoInternal(MessageParcel &data, MessageParcel &reply);
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
    void GetHardwareOutputSamplingRateInternal(MessageParcel &data, MessageParcel &reply);
    void GetAudioCapturerMicrophoneDescriptorsInternal(MessageParcel &data, MessageParcel &reply);
    void GetAvailableMicrophonesInternal(MessageParcel &data, MessageParcel &reply);
    void SetDeviceAbsVolumeSupportedInternal(MessageParcel &data, MessageParcel &reply);
    void IsAbsVolumeSceneInternal(MessageParcel &data, MessageParcel &reply);
    void SetA2dpDeviceVolumeInternal(MessageParcel &data, MessageParcel &reply);
    void SetNearlinkDeviceVolumeInternal(MessageParcel &data, MessageParcel &reply);
    void ReadStreamChangeInfo(MessageParcel &data, const AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo);
    void WriteAudioFocusInfo(MessageParcel &data,
        const std::pair<AudioInterrupt, AudioFocuState> &focusInfo);
    void GetAvailableDevicesInternal(MessageParcel &data, MessageParcel &reply);
    void SetAvailableDeviceChangeCallbackInternal(MessageParcel &data, MessageParcel &reply);
    void UnsetAvailableDeviceChangeCallbackInternal(MessageParcel &data, MessageParcel &reply);
    void IsSpatializationEnabledInternal(MessageParcel &data, MessageParcel &reply);
    void IsSpatializationEnabledForDeviceInternal(MessageParcel &data, MessageParcel &reply);
    void IsSpatializationEnabledForCurrentDeviceInternal(MessageParcel &data, MessageParcel &reply);
    void SetSpatializationEnabledInternal(MessageParcel &data, MessageParcel &reply);
    void SetSpatializationEnabledForDeviceInternal(MessageParcel &data, MessageParcel &reply);
    void IsHeadTrackingEnabledInternal(MessageParcel &data, MessageParcel &reply);
    void IsHeadTrackingEnabledForDeviceInternal(MessageParcel &data, MessageParcel &reply);
    void SetHeadTrackingEnabledInternal(MessageParcel &data, MessageParcel &reply);
    void SetHeadTrackingEnabledForDeviceInternal(MessageParcel &data, MessageParcel &reply);
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

    void HandleRegisterAudioZoneClient(MessageParcel &data, MessageParcel &reply);
    void HandleCreateAudioZone(MessageParcel &data, MessageParcel &reply);
    void HandleReleaseAudioZone(MessageParcel &data, MessageParcel &reply);
    void HandleGetAllAudioZone(MessageParcel &data, MessageParcel &reply);
    void HandleGetAudioZone(MessageParcel &data, MessageParcel &reply);
    void HandleBindAudioZoneDevice(MessageParcel &data, MessageParcel &reply);
    void HandleUnBindAudioZoneDevice(MessageParcel &data, MessageParcel &reply);
    void HandleEnableAudioZoneReport (MessageParcel &data, MessageParcel &reply);
    void HandleEnableAudioZoneChangeReport(MessageParcel &data, MessageParcel &reply);
    void HandleAddUidToAudioZone(MessageParcel &data, MessageParcel &reply);
    void HandleRemoveUidFromAudioZone(MessageParcel &data, MessageParcel &reply);
    void HandleEnableSystemVolumeProxy(MessageParcel &data, MessageParcel &reply);
    void HandleGetAudioInterruptForZone(MessageParcel &data, MessageParcel &reply);
    void HandleGetAudioInterruptForZoneDevice(MessageParcel &data, MessageParcel &reply);
    void HandleEnableAudioZoneInterruptReport(MessageParcel &data, MessageParcel &reply);
    void HandleInjectInterruptToAudioZone(MessageParcel &data, MessageParcel &reply);
    void HandleInjectInterruptToAudioZoneDevice(MessageParcel &data, MessageParcel &reply);

    void SetCallDeviceActiveInternal(MessageParcel &data, MessageParcel &reply);
    void GetConverterConfigInternal(MessageParcel &data, MessageParcel &reply);
    void GetActiveBluetoothDeviceInternal(MessageParcel &data, MessageParcel &reply);
    void FetchOutputDeviceForTrackInternal(MessageParcel &data, MessageParcel &reply);
    void FetchInputDeviceForTrackInternal(MessageParcel &data, MessageParcel &reply);
    void IsHighResolutionExistInternal(MessageParcel &data, MessageParcel &reply);
    void SetHighResolutionExistInternal(MessageParcel &data, MessageParcel &reply);
    void GetSpatializationSceneTypeInternal(MessageParcel &data, MessageParcel &reply);
    void SetSpatializationSceneTypeInternal(MessageParcel &data, MessageParcel &reply);
    void GetMaxAmplitudeInternal(MessageParcel &data, MessageParcel &reply);
    void DisableSafeMediaVolumeInternal(MessageParcel &data, MessageParcel &reply);
    void IsHeadTrackingDataRequestedInternal(MessageParcel &data, MessageParcel &reply);
    void SetAudioDeviceRefinerCallbackInternal(MessageParcel &data, MessageParcel &reply);
    void UnsetAudioDeviceRefinerCallbackInternal(MessageParcel &data, MessageParcel &reply);
    void TriggerFetchDeviceInternal(MessageParcel &data, MessageParcel &reply);
    void SetPreferredDeviceInternal(MessageParcel &data, MessageParcel &reply);
    void SaveRemoteInfoInternal(MessageParcel &data, MessageParcel &reply);
    void SetAudioDeviceAnahsCallbackInternal(MessageParcel &data, MessageParcel &reply);
    void UnsetAudioDeviceAnahsCallbackInternal(MessageParcel &data, MessageParcel &reply);
    void MoveToNewTypeInternal(MessageParcel &data, MessageParcel &reply);
    void GetDevicesInnerInternal(MessageParcel &data, MessageParcel &reply);
    void SetConcurrencyCallbackInternal(MessageParcel &data, MessageParcel &reply);
    void UnsetConcurrencyCallbackInternal(MessageParcel &data, MessageParcel &reply);
    void ActivateAudioConcurrencyInternal(MessageParcel &data, MessageParcel &reply);
    void SetRingerStreamMuteInternal(MessageParcel &data, MessageParcel &reply);
    void SetMicrophoneMutePersistentInternal(MessageParcel &data, MessageParcel &reply);
    void GetMicrophoneMutePersistentInternal(MessageParcel &data, MessageParcel &reply);
    void GetSupportedAudioEffectPropertyV3Internal(MessageParcel &data, MessageParcel &reply);
    void SetAudioEffectPropertyV3Internal(MessageParcel &data, MessageParcel &reply);
    void GetAudioEffectPropertyV3Internal(MessageParcel &data, MessageParcel &reply);
    void GetSupportedAudioEnhancePropertyInternal(MessageParcel &data, MessageParcel &reply);
    void GetSupportedAudioEffectPropertyInternal(MessageParcel &data, MessageParcel &reply);
    void SetAudioEffectPropertyInternal(MessageParcel &data, MessageParcel &reply);
    void GetAudioEffectPropertyInternal(MessageParcel &data, MessageParcel &reply);
    void SetAudioEnhancePropertyInternal(MessageParcel &data, MessageParcel &reply);
    void GetAudioEnhancePropertyInternal(MessageParcel &data, MessageParcel &reply);
    void InjectInterruptionInternal(MessageParcel &data, MessageParcel &reply);
    void ActivateAudioSessionInternal(MessageParcel &data, MessageParcel &reply);
    void DeactivateAudioSessionInternal(MessageParcel &data, MessageParcel &reply);
    void SetInputDeviceInternal(MessageParcel &data, MessageParcel &reply);
    void IsAudioSessionActivatedInternal(MessageParcel &data, MessageParcel &reply);
    void LoadSplitModuleInternal(MessageParcel &data, MessageParcel &reply);
    void IsAllowedPlaybackInternal(MessageParcel &data, MessageParcel &reply);
    void SetVoiceRingtoneMuteInternal(MessageParcel &data, MessageParcel &reply);
    void SetQueryClientTypeCallbackInternal(MessageParcel &data, MessageParcel &reply);
    void SetAudioClientInfoMgrCallbackInternal(MessageParcel &data, MessageParcel &reply);
    void SetVirtualCallInternal(MessageParcel &data, MessageParcel &reply);
    void SetDeviceConnectionStatusInternal(MessageParcel &data, MessageParcel &reply);
    void SetQueryAllowedPlaybackCallbackInternal(MessageParcel &data, MessageParcel &reply);
    void SetBackgroundMuteCallbackInternal(MessageParcel &data, MessageParcel &reply);
    void GetDirectPlaybackSupportInternal(MessageParcel &data, MessageParcel &reply);
    void SetQueryBundleNameListCallbackInternal(MessageParcel &data, MessageParcel &reply);
    void NotifySessionStateChangeInternal(MessageParcel &data, MessageParcel &reply);
    void NotifyFreezeStateChangeInternal(MessageParcel &data, MessageParcel &reply);
    void ResetAllProxyInternal(MessageParcel &data, MessageParcel &reply);
    void IsAcousticEchoCancelerSupportedInternal(MessageParcel &data, MessageParcel &reply);
    void ForceStopAudioStreamInternal(MessageParcel &data, MessageParcel &reply);
    void IsCapturerFocusAvailableInternal(MessageParcel &data, MessageParcel &reply);
    void GetMaxVolumeLevelByUsageInternal(MessageParcel &data, MessageParcel &reply);
    void GetMinVolumeLevelByUsageInternal(MessageParcel &data, MessageParcel &reply);
    void GetVolumeLevelByUsageInternal(MessageParcel &data, MessageParcel &reply);
    void GetStreamMuteByUsageInternal(MessageParcel &data, MessageParcel &reply);
    void SetCallbackStreamUsageInfoInternal(MessageParcel &data, MessageParcel &reply);
    void UpdateDeviceInfoInternal(MessageParcel &data, MessageParcel &reply);
    void SetSleAudioOperationCallbackInternal(MessageParcel &data, MessageParcel &reply);
    void IsCollaborativePlaybackSupportedInternal(MessageParcel &data, MessageParcel &reply);
    void SetCollaborativePlayBackEnabledForDeviceInternal(MessageParcel &data, MessageParcel &reply);
    void IsCollaborativePlaybackEnabledForDeviceInternal(MessageParcel &data, MessageParcel &reply);

    void OnMiddleTweRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnMiddleEleRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnMiddleTenRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnMiddleNinRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnMiddleEigRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnMiddleSevRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnMiddleSixRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnMiddleFifRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnMiddleFouRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnMiddleTirRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnMiddleSecRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnMiddleFirRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnMiddlesRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnMidRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnAudioZoneRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnAudioZoneRemoteRequestExt(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_POLICY_MANAGER_STUB_H
