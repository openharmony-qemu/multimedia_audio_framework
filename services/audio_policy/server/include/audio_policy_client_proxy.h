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

#ifndef ST_AUDIO_POLICY_CLIENT_PROXY_H
#define ST_AUDIO_POLICY_CLIENT_PROXY_H

#include "audio_interrupt_info.h"
#include "audio_interrupt_callback.h"
#include "audio_policy_client.h"

namespace OHOS {
namespace AudioStandard {
class AudioPolicyClientProxy : public IRemoteProxy<IAudioPolicyClient> {
public:
    AudioPolicyClientProxy(const sptr<IRemoteObject> &impl);
    virtual ~AudioPolicyClientProxy();
    int32_t RegisterPolicyCallbackClient(const sptr<IRemoteObject> &object);
    void UnregisterPolicyCallbackClient();

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
    void OnRecreateRendererStreamEvent(const uint32_t sessionId, const int32_t streamFlag,
        const AudioStreamDeviceChangeReasonExt) override;
    void OnRecreateCapturerStreamEvent(const uint32_t sessionId, const int32_t streamFlag,
        const AudioStreamDeviceChangeReasonExt) override;
    void OnHeadTrackingDeviceChange(const std::unordered_map<std::string, bool> &changeInfo) override;
    void OnSpatializationEnabledChange(const bool &enabled) override;
    void OnSpatializationEnabledChangeForAnyDevice(const sptr<AudioDeviceDescriptor> &deviceDescriptor,
        const bool &enabled) override;
    void OnHeadTrackingEnabledChange(const bool &enabled) override;
    void OnHeadTrackingEnabledChangeForAnyDevice(const sptr<AudioDeviceDescriptor> &deviceDescriptor,
        const bool &enabled) override;
    void OnAudioSessionDeactive(const AudioSessionDeactiveEvent &deactiveEvent) override;

private:
    static inline BrokerDelegator<AudioPolicyClientProxy> delegator_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_POLICY_CLIENT_PROXY_H
