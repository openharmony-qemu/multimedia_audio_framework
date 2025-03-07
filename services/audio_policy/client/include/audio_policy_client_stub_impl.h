/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#ifndef AUDIO_POLICY_CLIENT_STUB_IMPL_H
#define AUDIO_POLICY_CLIENT_STUB_IMPL_H

#include "audio_policy_client_stub.h"
#include "audio_device_info.h"
#include "audio_session_manager.h"
#include "audio_system_manager.h"
#include "audio_interrupt_info.h"
#include "audio_group_manager.h"
#include "audio_routing_manager.h"
#include "audio_spatialization_manager.h"
#include "audio_combine_denoising_manager.h"
#include "audio_policy_interface.h"

namespace OHOS {
namespace AudioStandard {
class AudioPolicyClientStubImpl : public AudioPolicyClientStub {
public:
    int32_t AddVolumeKeyEventCallback(const std::shared_ptr<VolumeKeyEventCallback> &cb);
    int32_t RemoveVolumeKeyEventCallback(const std::shared_ptr<VolumeKeyEventCallback> &cb);
    size_t GetVolumeKeyEventCallbackSize() const;
    int32_t AddFocusInfoChangeCallback(const std::shared_ptr<AudioFocusInfoChangeCallback> &cb);
    int32_t RemoveFocusInfoChangeCallback();
    int32_t AddDeviceChangeCallback(const DeviceFlag &flag,
        const std::shared_ptr<AudioManagerDeviceChangeCallback> &cb);
    int32_t RemoveDeviceChangeCallback(DeviceFlag flag, std::shared_ptr<AudioManagerDeviceChangeCallback> &cb);
    size_t GetDeviceChangeCallbackSize() const;
    int32_t AddRingerModeCallback(const std::shared_ptr<AudioRingerModeCallback> &cb);
    int32_t AddAppVolumeChangeForUidCallback(const int32_t appUid,
        const std::shared_ptr<AudioManagerAppVolumeChangeCallback> &cb);
    int32_t RemoveAppVolumeChangeForUidCallback(
        const std::shared_ptr<AudioManagerAppVolumeChangeCallback> &cb);
    int32_t RemoveAllAppVolumeChangeForUidCallback();
    size_t GetAppVolumeChangeCallbackForUidSize() const;
    size_t GetSelfAppVolumeChangeCallbackSize() const;
    int32_t AddSelfAppVolumeChangeCallback(int32_t appUid,
        const std::shared_ptr<AudioManagerAppVolumeChangeCallback> &cb);
    int32_t RemoveSelfAppVolumeChangeCallback(int32_t appUid,
        const std::shared_ptr<AudioManagerAppVolumeChangeCallback> &cb);
    int32_t RemoveAllSelfAppVolumeChangeCallback(int32_t appUid);
    int32_t RemoveRingerModeCallback();
    int32_t RemoveRingerModeCallback(const std::shared_ptr<AudioRingerModeCallback> &cb);
    size_t GetRingerModeCallbackSize() const;
    int32_t AddMicStateChangeCallback(const std::shared_ptr<AudioManagerMicStateChangeCallback> &cb);
    int32_t RemoveMicStateChangeCallback();
    size_t GetMicStateChangeCallbackSize() const;
    bool HasMicStateChangeCallback();
    int32_t AddPreferredOutputDeviceChangeCallback(const AudioRendererInfo &rendererInfo,
        const std::shared_ptr<AudioPreferredOutputDeviceChangeCallback> &cb);
    int32_t RemovePreferredOutputDeviceChangeCallback(
        const std::shared_ptr<AudioPreferredOutputDeviceChangeCallback> &cb = nullptr);
    size_t GetPreferredOutputDeviceChangeCallbackSize() const;
    int32_t AddPreferredInputDeviceChangeCallback(const AudioCapturerInfo &capturerInfo,
        const std::shared_ptr<AudioPreferredInputDeviceChangeCallback> &cb);
    int32_t RemovePreferredInputDeviceChangeCallback(
        const std::shared_ptr<AudioPreferredInputDeviceChangeCallback> &cb = nullptr);
    size_t GetPreferredInputDeviceChangeCallbackSize() const;
    int32_t AddRendererStateChangeCallback(const std::shared_ptr<AudioRendererStateChangeCallback> &cb);
    int32_t RemoveRendererStateChangeCallback(
        const std::vector<std::shared_ptr<AudioRendererStateChangeCallback>> &callbacks);
    int32_t RemoveRendererStateChangeCallback(
        const std::shared_ptr<AudioRendererStateChangeCallback> &callback);
    size_t GetRendererStateChangeCallbackSize() const;
    int32_t AddCapturerStateChangeCallback(const std::shared_ptr<AudioCapturerStateChangeCallback> &cb);
    int32_t RemoveCapturerStateChangeCallback();
    size_t GetCapturerStateChangeCallbackSize() const;
    int32_t AddDeviceChangeWithInfoCallback(
        const uint32_t sessionId, const std::weak_ptr<DeviceChangeWithInfoCallback> &cb);
    int32_t RemoveDeviceChangeWithInfoCallback(const uint32_t sessionId);
    size_t GetDeviceChangeWithInfoCallbackkSize() const;
    int32_t AddMicrophoneBlockedCallback(const int32_t clientId,
        const std::shared_ptr<AudioManagerMicrophoneBlockedCallback> &cb);
    int32_t RemoveMicrophoneBlockedCallback(const int32_t clientId,
        const std::shared_ptr<AudioManagerMicrophoneBlockedCallback> &cb);
    size_t GetMicrophoneBlockedCallbackSize() const;
    int32_t AddHeadTrackingDataRequestedChangeCallback(const std::string &macAddress,
        const std::shared_ptr<HeadTrackingDataRequestedChangeCallback> &cb);
    int32_t RemoveHeadTrackingDataRequestedChangeCallback(const std::string &macAddress);
    size_t GetHeadTrackingDataRequestedChangeCallbackSize() const;
    int32_t AddSpatializationEnabledChangeCallback(const std::shared_ptr<AudioSpatializationEnabledChangeCallback> &cb);
    int32_t RemoveSpatializationEnabledChangeCallback();
    size_t GetSpatializationEnabledChangeCallbackSize() const;
    int32_t AddSpatializationEnabledChangeForCurrentDeviceCallback(
        const std::shared_ptr<AudioSpatializationEnabledChangeForCurrentDeviceCallback> &cb);
    int32_t RemoveSpatializationEnabledChangeForCurrentDeviceCallback();
    size_t GetSpatializationEnabledChangeForCurrentDeviceCallbackSize() const;
    int32_t AddHeadTrackingEnabledChangeCallback(const std::shared_ptr<AudioHeadTrackingEnabledChangeCallback> &cb);
    int32_t RemoveHeadTrackingEnabledChangeCallback();
    size_t GetHeadTrackingEnabledChangeCallbacSize() const;
    int32_t AddNnStateChangeCallback(const std::shared_ptr<AudioNnStateChangeCallback> &cb);
    int32_t RemoveNnStateChangeCallback();
    size_t GetNnStateChangeCallbackSize() const;
    size_t GetFocusInfoChangeCallbackSize() const;
    int32_t AddAudioSessionCallback(const std::shared_ptr<AudioSessionCallback> &cb);
    int32_t RemoveAudioSessionCallback();
    int32_t RemoveAudioSessionCallback(const std::shared_ptr<AudioSessionCallback> &cb);
    size_t GetAudioSessionCallbackSize() const;
    int32_t AddAudioSceneChangedCallback(const int32_t clientId,
        const std::shared_ptr<AudioManagerAudioSceneChangedCallback> &cb);
    int32_t RemoveAudioSceneChangedCallback(
        const std::shared_ptr<AudioManagerAudioSceneChangedCallback> &cb);
    size_t GetAudioSceneChangedCallbackSize() const;
    int32_t SetDistribuitedOutputChangeCallback(const std::shared_ptr<AudioDistribuitedOutputChangeCallback> &cb);

    void OnRecreateRendererStreamEvent(const uint32_t sessionId, const int32_t streamFlag,
        const AudioStreamDeviceChangeReasonExt reason) override;
    void OnRecreateCapturerStreamEvent(const uint32_t sessionId, const int32_t streamFlag,
        const AudioStreamDeviceChangeReasonExt reason) override;
    void OnVolumeKeyEvent(VolumeEvent volumeEvent) override;
    void OnAudioFocusInfoChange(const std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList) override;
    void OnAudioFocusRequested(const AudioInterrupt &requestFocus) override;
    void OnAudioFocusAbandoned(const AudioInterrupt &abandonFocus) override;
    void OnDeviceChange(const DeviceChangeAction &deviceChangeAction) override;
    void OnMicrophoneBlocked(const MicrophoneBlockedInfo &microphoneBlockedInfo) override;
    void OnRingerModeUpdated(const AudioRingerMode &ringerMode) override;
    void OnAppVolumeChanged(int32_t appUid, const VolumeEvent& volumeEvent) override;
    void OnMicStateUpdated(const MicStateChangeEvent &micStateChangeEvent) override;
    void OnPreferredOutputDeviceUpdated(const AudioRendererInfo &rendererInfo,
        const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc) override;
    void OnPreferredInputDeviceUpdated(const AudioCapturerInfo &capturerInfo,
        const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc) override;
    void OnRendererStateChange(
        std::vector<std::shared_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos) override;
    void OnCapturerStateChange(
        std::vector<std::shared_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos) override;
    void OnRendererDeviceChange(const uint32_t sessionId,
        const AudioDeviceDescriptor &deviceInfo, const AudioStreamDeviceChangeReasonExt reason) override;
    void OnDistribuitedOutputChange(const AudioDeviceDescriptor &deviceDesc, bool isRemote) override;
    void OnHeadTrackingDeviceChange(const std::unordered_map<std::string, bool> &changeInfo) override;
    void OnSpatializationEnabledChange(const bool &enabled) override;
    void OnSpatializationEnabledChangeForAnyDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor,
        const bool &enabled) override;
    void OnSpatializationEnabledChangeForCurrentDevice(const bool &enabled) override;
    void OnHeadTrackingEnabledChange(const bool &enabled) override;
    void OnHeadTrackingEnabledChangeForAnyDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor,
        const bool &enabled) override;
    void OnNnStateChange(const int32_t &nnState) override;
    void OnAudioSessionDeactive(const AudioSessionDeactiveEvent &deactiveEvent) override;
    void OnAudioSceneChange(const AudioScene &audioScene) override;

private:
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> DeviceFilterByFlag(DeviceFlag flag,
        const std::vector<std::shared_ptr<AudioDeviceDescriptor>>& desc);

    std::vector<std::weak_ptr<VolumeKeyEventCallback>> volumeKeyEventCallbackList_;
    std::vector<std::shared_ptr<AudioFocusInfoChangeCallback>> focusInfoChangeCallbackList_;
    std::vector<std::pair<DeviceFlag, std::shared_ptr<AudioManagerDeviceChangeCallback>>> deviceChangeCallbackList_;
    std::vector<std::shared_ptr<AudioRingerModeCallback>> ringerModeCallbackList_;
    std::vector<std::pair<int32_t, std::shared_ptr<
        AudioManagerAppVolumeChangeCallback>>> appVolumeChangeForUidCallback_;
    std::map<int32_t, int32_t> appVolumeChangeForUidCallbackNum;
    std::vector<std::pair<int32_t, std::shared_ptr<AudioManagerAppVolumeChangeCallback>>> selfAppVolumeChangeCallback_;
    std::map<int32_t, int32_t> selfAppVolumeChangeCallbackNum_;
    std::vector<std::shared_ptr<AudioManagerMicStateChangeCallback>> micStateChangeCallbackList_;
    std::vector<std::shared_ptr<AudioRendererStateChangeCallback>> rendererStateChangeCallbackList_;
    std::vector<std::weak_ptr<AudioCapturerStateChangeCallback>> capturerStateChangeCallbackList_;
    std::vector<std::shared_ptr<AudioSpatializationEnabledChangeCallback>> spatializationEnabledChangeCallbackList_;
    std::vector<std::shared_ptr<AudioSpatializationEnabledChangeForCurrentDeviceCallback>>
        spatializationEnabledChangeForCurrentDeviceCallbackList_;
    std::vector<std::shared_ptr<AudioHeadTrackingEnabledChangeCallback>> headTrackingEnabledChangeCallbackList_;
    std::vector<std::shared_ptr<AudioNnStateChangeCallback>> nnStateChangeCallbackList_;
    std::vector<std::shared_ptr<AudioSessionCallback>> audioSessionCallbackList_;
    std::vector<std::pair<int32_t, std::shared_ptr<AudioManagerMicrophoneBlockedCallback>>>
        microphoneBlockedCallbackList_;
    std::vector<std::shared_ptr<AudioManagerAudioSceneChangedCallback>> audioSceneChangedCallbackList_;
    std::vector<std::shared_ptr<AudioDistribuitedOutputChangeCallback>> distribuitedOutputChangeCallback_;

    std::unordered_map<StreamUsage,
        std::vector<std::shared_ptr<AudioPreferredOutputDeviceChangeCallback>>> preferredOutputDeviceCallbackMap_;
    std::unordered_map<SourceType,
        std::vector<std::shared_ptr<AudioPreferredInputDeviceChangeCallback>>> preferredInputDeviceCallbackMap_;

    std::unordered_map<uint32_t,
        std::weak_ptr<DeviceChangeWithInfoCallback>> deviceChangeWithInfoCallbackMap_;

    std::unordered_map<std::string,
        std::shared_ptr<HeadTrackingDataRequestedChangeCallback>> headTrackingDataRequestedChangeCallbackMap_;

    mutable std::mutex focusInfoChangeMutex_;
    mutable std::mutex rendererStateChangeMutex_;
    mutable std::mutex capturerStateChangeMutex_;
    mutable std::mutex pOutputDeviceChangeMutex_;
    mutable std::mutex pInputDeviceChangeMutex_;
    mutable std::mutex volumeKeyEventMutex_;
    mutable std::mutex deviceChangeMutex_;
    mutable std::mutex ringerModeMutex_;
    mutable std::mutex appVolumeChangeForUidMutex_;
    mutable std::mutex selfAppVolumeChangeMutex_;
    mutable std::mutex micStateChangeMutex_;
    mutable std::mutex deviceChangeWithInfoCallbackMutex_;
    mutable std::mutex headTrackingDataRequestedChangeMutex_;
    mutable std::mutex spatializationEnabledChangeMutex_;
    mutable std::mutex spatializationEnabledChangeForCurrentDeviceMutex_;
    mutable std::mutex headTrackingEnabledChangeMutex_;
    mutable std::mutex nnStateChangeMutex_;
    mutable std::mutex audioSessionMutex_;
    mutable std::mutex microphoneBlockedMutex_;
    mutable std::mutex audioSceneChangedMutex_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_POLICY_CLIENT_STUB_IMPL_H