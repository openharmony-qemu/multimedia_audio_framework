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

#ifndef ST_AUDIO_POLICY_CLIENT_H
#define ST_AUDIO_POLICY_CLIENT_H

#include "ipc_types.h"
#include "iremote_broker.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"
#include "audio_info.h"
#include "audio_system_manager.h"
#include "audio_interrupt_info.h"
#include "audio_stream_manager.h"

namespace OHOS {
namespace AudioStandard {
static const int32_t UPDATE_CALLBACK_CLIENT = 0;
static const int32_t API_VERSION_MAX = 1000;

enum class AudioPolicyClientCode {
    ON_VOLUME_KEY_EVENT = 0,
    ON_FOCUS_INFO_CHANGED,
    ON_FOCUS_REQUEST_CHANGED,
    ON_FOCUS_ABANDON_CHANGED,
    ON_DEVICE_CHANGE,
    ON_RINGERMODE_UPDATE,
    ON_MIC_STATE_UPDATED,
    ON_ACTIVE_OUTPUT_DEVICE_UPDATED,
    ON_ACTIVE_INPUT_DEVICE_UPDATED,
    ON_RENDERERSTATE_CHANGE,
    ON_CAPTURERSTATE_CHANGE,
    ON_RENDERER_DEVICE_CHANGE,
    ON_RECREATE_RENDERER_STREAM_EVENT,
    ON_RECREATE_CAPTURER_STREAM_EVENT,
    ON_HEAD_TRACKING_DEVICE_CHANGE,
    ON_SPATIALIZATION_ENABLED_CHANGE,
    ON_SPATIALIZATION_ENABLED_CHANGE_FOR_ANY_DEVICE,
    ON_HEAD_TRACKING_ENABLED_CHANGE,
    ON_HEAD_TRACKING_ENABLED_CHANGE_FOR_ANY_DEVICE,
    ON_AUDIO_SESSION_DEACTIVE,
    AUDIO_POLICY_CLIENT_CODE_MAX = ON_AUDIO_SESSION_DEACTIVE,
};
class IAudioPolicyClient : public IRemoteBroker {
public:
    virtual void OnVolumeKeyEvent(VolumeEvent volumeEvent) = 0;
    virtual void OnAudioFocusInfoChange(const std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList) = 0;
    virtual void OnAudioFocusRequested(const AudioInterrupt &requestFocus) = 0;
    virtual void OnAudioFocusAbandoned(const AudioInterrupt &abandonFocus) = 0;
    virtual void OnDeviceChange(const DeviceChangeAction &deviceChangeAction) =0;
    virtual void OnRingerModeUpdated(const AudioRingerMode &ringerMode) = 0;
    virtual void OnMicStateUpdated(const MicStateChangeEvent &micStateChangeEvent) = 0;
    virtual void OnPreferredOutputDeviceUpdated(const std::vector<sptr<AudioDeviceDescriptor>> &desc) = 0;
    virtual void OnPreferredInputDeviceUpdated(const std::vector<sptr<AudioDeviceDescriptor>> &desc) = 0;
    virtual void OnRendererStateChange(
        std::vector<std::unique_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos) = 0;
    virtual void OnCapturerStateChange(
        std::vector<std::unique_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos) = 0;
    virtual void OnRendererDeviceChange(const uint32_t sessionId,
        const DeviceInfo &deviceInfo, const AudioStreamDeviceChangeReasonExt reason) = 0;
    virtual void OnRecreateRendererStreamEvent(const uint32_t sessionId, const int32_t streamFlag,
        const AudioStreamDeviceChangeReasonExt reason) = 0;
    virtual void OnRecreateCapturerStreamEvent(const uint32_t sessionId, const int32_t streamFlag,
        const AudioStreamDeviceChangeReasonExt reason) = 0;
    virtual void OnHeadTrackingDeviceChange(const std::unordered_map<std::string, bool> &changeInfo) = 0;
    virtual void OnSpatializationEnabledChange(const bool &enabled) = 0;
    virtual void OnSpatializationEnabledChangeForAnyDevice(const sptr<AudioDeviceDescriptor> &deviceDescriptor,
        const bool &enabled) = 0;
    virtual void OnHeadTrackingEnabledChange(const bool &enabled) = 0;
    virtual void OnHeadTrackingEnabledChangeForAnyDevice(const sptr<AudioDeviceDescriptor> &deviceDescriptor,
        const bool &enabled) = 0;
    virtual void OnAudioSessionDeactive(const AudioSessionDeactiveEvent &deactiveEvent) = 0;

    bool hasBTPermission_ = true;
    bool hasSystemPermission_ = true;
    int32_t apiVersion_ = API_VERSION_MAX;

public:
    DECLARE_INTERFACE_DESCRIPTOR(u"IAudioPolicyClient");
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_POLICY_CLIENT_H
