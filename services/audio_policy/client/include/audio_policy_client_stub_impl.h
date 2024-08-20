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

#ifndef AUDIO_POLICY_CLIENT_STUB_IMPL_H
#define AUDIO_POLICY_CLIENT_STUB_IMPL_H

#include "audio_policy_client_stub.h"
#include "audio_device_info.h"
#include "audio_system_manager.h"
#include "audio_interrupt_info.h"
#include "audio_group_manager.h"
#include "audio_routing_manager.h"
#include "audio_spatialization_manager.h"

namespace OHOS {
namespace AudioStandard {
class AudioPolicyClientStubImpl : public AudioPolicyClientStub {
public:
    int32_t AddVolumeKeyEventCallback(const std::shared_ptr<VolumeKeyEventCallback> &cb);
    int32_t RemoveVolumeKeyEventCallback(const std::shared_ptr<VolumeKeyEventCallback> &cb);
    int32_t AddFocusInfoChangeCallback(const std::shared_ptr<AudioFocusInfoChangeCallback> &cb);
    int32_t RemoveFocusInfoChangeCallback();
    int32_t AddDeviceChangeCallback(const DeviceFlag &flag,
        const std::shared_ptr<AudioManagerDeviceChangeCallback> &cb);
    int32_t RemoveDeviceChangeCallback(DeviceFlag flag, std::shared_ptr<AudioManagerDeviceChangeCallback> &cb);
    int32_t AddRingerModeCallback(const std::shared_ptr<AudioRingerModeCallback> &cb);
    int32_t RemoveRingerModeCallback();
    int32_t RemoveRingerModeCallback(const std::shared_ptr<AudioRingerModeCallback> &cb);
    int32_t AddMicStateChangeCallback(const std::shared_ptr<AudioManagerMicStateChangeCallback> &cb);
    int32_t RemoveMicStateChangeCallback();
    bool HasMicStateChangeCallback();
    int32_t AddPreferredOutputDeviceChangeCallback(
        const std::shared_ptr<AudioPreferredOutputDeviceChangeCallback> &cb);
    int32_t RemovePreferredOutputDeviceChangeCallback();
    int32_t AddPreferredInputDeviceChangeCallback(
        const std::shared_ptr<AudioPreferredInputDeviceChangeCallback> &cb);
    int32_t RemovePreferredInputDeviceChangeCallback();
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

    int32_t AddHeadTrackingDataRequestedChangeCallback(const std::string &macAddress,
        const std::shared_ptr<HeadTrackingDataRequestedChangeCallback> &cb);
    int32_t RemoveHeadTrackingDataRequestedChangeCallback(const std::string &macAddress);
    int32_t AddSpatializationEnabledChangeCallback(const std::shared_ptr<AudioSpatializationEnabledChangeCallback> &cb);
    int32_t RemoveSpatializationEnabledChangeCallback();
    int32_t AddHeadTrackingEnabledChangeCallback(const std::shared_ptr<AudioHeadTrackingEnabledChangeCallback> &cb);
    int32_t RemoveHeadTrackingEnabledChangeCallback();
    size_t GetFocusInfoChangeCallbackSize() const;

    void OnRecreateRendererStreamEvent(const uint32_t sessionId, const int32_t streamFlag,
        const AudioStreamDeviceChangeReasonExt reason) override;
    void OnRecreateCapturerStreamEvent(const uint32_t sessionId, const int32_t streamFlag,
        const AudioStreamDeviceChangeReasonExt reason) override;
    void OnVolumeKeyEvent(VolumeEvent volumeEvent) override;
    void OnAudioFocusInfoChange(const std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList) override;
    void OnAudioFocusRequested(const AudioInterrupt &requestFocus) override;
    void OnAudioFocusAbandoned(const AudioInterrupt &abandonFocus) override;
    void OnDeviceChange(const DeviceChangeAction &deviceChangeAction) override;
    void OnRingerModeUpdated(const AudioRingerMode &ringerMode) override;
    void OnMicStateUpdated(const MicStateChangeEvent &micStateChangeEvent) override;
    void OnPreferredOutputDeviceUpdated(const std::vector<sptr<AudioDeviceDescriptor>> &desc) override;
    void OnPreferredInputDeviceUpdated(const std::vector<sptr<AudioDeviceDescriptor>> &desc) override;
    void OnRendererStateChange(
        std::vector<std::unique_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos) override;
    void OnCapturerStateChange(
        std::vector<std::unique_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos) override;
    void OnRendererDeviceChange(const uint32_t sessionId,
        const DeviceInfo &deviceInfo, const AudioStreamDeviceChangeReasonExt reason) override;
    void OnHeadTrackingDeviceChange(const std::unordered_map<std::string, bool> &changeInfo) override;
    void OnSpatializationEnabledChange(const bool &enabled) override;
    void OnSpatializationEnabledChangeForAnyDevice(const sptr<AudioDeviceDescriptor> &deviceDescriptor,
        const bool &enabled) override;
    void OnHeadTrackingEnabledChange(const bool &enabled) override;
    void OnHeadTrackingEnabledChangeForAnyDevice(const sptr<AudioDeviceDescriptor> &deviceDescriptor,
        const bool &enabled) override;

private:
    std::vector<sptr<AudioDeviceDescriptor>> DeviceFilterByFlag(DeviceFlag flag,
        const std::vector<sptr<AudioDeviceDescriptor>>& desc);

    std::vector<std::weak_ptr<VolumeKeyEventCallback>> volumeKeyEventCallbackList_;
    std::vector<std::shared_ptr<AudioFocusInfoChangeCallback>> focusInfoChangeCallbackList_;
    std::vector<std::pair<DeviceFlag, std::shared_ptr<AudioManagerDeviceChangeCallback>>> deviceChangeCallbackList_;
    std::vector<std::shared_ptr<AudioRingerModeCallback>> ringerModeCallbackList_;
    std::vector<std::shared_ptr<AudioManagerMicStateChangeCallback>> micStateChangeCallbackList_;
    std::vector<std::shared_ptr<AudioPreferredOutputDeviceChangeCallback>> preferredOutputDeviceCallbackList_;
    std::vector<std::shared_ptr<AudioPreferredInputDeviceChangeCallback>> preferredInputDeviceCallbackList_;
    std::vector<std::shared_ptr<AudioRendererStateChangeCallback>> rendererStateChangeCallbackList_;
    std::vector<std::weak_ptr<AudioCapturerStateChangeCallback>> capturerStateChangeCallbackList_;
    std::vector<std::shared_ptr<AudioSpatializationEnabledChangeCallback>> spatializationEnabledChangeCallbackList_;
    std::vector<std::shared_ptr<AudioHeadTrackingEnabledChangeCallback>> headTrackingEnabledChangeCallbackList_;

    std::unordered_map<uint32_t,
        std::weak_ptr<DeviceChangeWithInfoCallback>> deviceChangeWithInfoCallbackMap_;

    std::unordered_map<std::string,
        std::shared_ptr<HeadTrackingDataRequestedChangeCallback>> headTrackingDataRequestedChangeCallbackMap_;

    std::mutex volumeKeyEventMutex_;
    mutable std::mutex focusInfoChangeMutex_;
    std::mutex deviceChangeMutex_;
    std::mutex ringerModeMutex_;
    std::mutex micStateChangeMutex_;
    std::mutex pOutputDeviceChangeMutex_;
    std::mutex pInputDeviceChangeMutex_;
    mutable std::mutex rendererStateChangeMutex_;
    mutable std::mutex capturerStateChangeMutex_;
    std::mutex deviceChangeWithInfoCallbackMutex_;
    std::mutex headTrackingDataRequestedChangeMutex_;
    std::mutex spatializationEnabledChangeMutex_;
    std::mutex headTrackingEnabledChangeMutex_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_POLICY_CLIENT_STUB_IMPL_H