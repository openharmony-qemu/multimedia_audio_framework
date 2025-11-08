/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef MOCK_AUDIO_POLICY_CLIENT_H
#define MOCK_AUDIO_POLICY_CLIENT_H

#include <gmock/gmock.h>
#include "iaudio_policy_client.h"

namespace OHOS {
namespace AudioStandard {

class MockAudioPolicyClient : public IAudioPolicyClient {
public:
    MockAudioPolicyClient() = default;
    ~MockAudioPolicyClient() = default;

    MOCK_METHOD(sptr<IRemoteObject>, AsObject, (), (override));
    MOCK_METHOD(ErrCode, OnVolumeKeyEvent,
                (const VolumeEvent& volumeEvent), (override));
    MOCK_METHOD(ErrCode, OnAudioFocusInfoChange,
                ((const std::vector<std::map<AudioInterrupt, int32_t>>& focusInfoList)), (override));
    MOCK_METHOD(ErrCode, OnAudioFocusRequested,
                (const AudioInterrupt& requestFocus), (override));
    MOCK_METHOD(ErrCode, OnAudioFocusAbandoned,
                (const AudioInterrupt& abandonFocus), (override));
    MOCK_METHOD(ErrCode, OnDeviceChange,
                (const DeviceChangeAction& deviceChangeAction), (override));
    MOCK_METHOD(ErrCode, OnDeviceInfoUpdate,
                (const DeviceChangeAction& deviceChangeAction), (override));
    MOCK_METHOD(ErrCode, OnAppVolumeChanged,
                (int32_t appUid, const VolumeEvent& volumeEvent), (override));
    MOCK_METHOD(ErrCode, OnActiveVolumeTypeChanged,
                (int32_t volumeType), (override));
    MOCK_METHOD(ErrCode, OnRingerModeUpdated,
                (int32_t ringerMode), (override));
    MOCK_METHOD(ErrCode, OnMicStateUpdated,
                (const MicStateChangeEvent& micStateChangeEvent), (override));
    MOCK_METHOD(ErrCode, OnPreferredOutputDeviceUpdated,
                (const AudioRendererInfo& rendererInfo,
                 const std::vector<std::shared_ptr<AudioDeviceDescriptor>>& desc), (override));
    MOCK_METHOD(ErrCode, OnPreferredInputDeviceUpdated,
                (const AudioCapturerInfo& capturerInfo,
                 const std::vector<std::shared_ptr<AudioDeviceDescriptor>>& desc), (override));
    MOCK_METHOD(ErrCode, OnRendererStateChange,
                (const std::vector<std::shared_ptr<AudioRendererChangeInfo>>& audioRendererChangeInfos), (override));
    MOCK_METHOD(ErrCode, OnCapturerStateChange,
                (const std::vector<std::shared_ptr<AudioCapturerChangeInfo>>& audioCapturerChangeInfos), (override));
    MOCK_METHOD(ErrCode, OnRendererDeviceChange,
                (uint32_t sessionId,
                 const AudioDeviceDescriptor& deviceInfo,
                 const AudioStreamDeviceChangeReasonExt& reason), (override));
    MOCK_METHOD(ErrCode, OnRecreateRendererStreamEvent,
                (uint32_t sessionId,
                 int32_t streamFlag,
                 const AudioStreamDeviceChangeReasonExt& reason), (override));
    MOCK_METHOD(ErrCode, OnRecreateCapturerStreamEvent,
                (uint32_t sessionId,
                 int32_t streamFlag,
                 const AudioStreamDeviceChangeReasonExt& reason), (override));
    MOCK_METHOD(ErrCode, OnHeadTrackingDeviceChange,
                ((const std::unordered_map<std::string, bool>& changeInfo)), (override));
    MOCK_METHOD(ErrCode, OnSpatializationEnabledChange,
                (bool enabled), (override));
    MOCK_METHOD(ErrCode, OnSpatializationEnabledChangeForAnyDevice,
                (const std::shared_ptr<AudioDeviceDescriptor>& deviceDescriptor, bool enabled), (override));
    MOCK_METHOD(ErrCode, OnHeadTrackingEnabledChange,
                (bool enabled), (override));
    MOCK_METHOD(ErrCode, OnHeadTrackingEnabledChangeForAnyDevice,
                (const std::shared_ptr<AudioDeviceDescriptor>& deviceDescriptor, bool enabled), (override));
    MOCK_METHOD(ErrCode, OnNnStateChange,
                (int32_t nnState), (override));
    MOCK_METHOD(ErrCode, OnAudioSessionDeactive,
                (int32_t deactiveEvent), (override));
    MOCK_METHOD(ErrCode, OnMicrophoneBlocked,
                (const MicrophoneBlockedInfo& microphoneBlockedInfo), (override));
    MOCK_METHOD(ErrCode, OnAudioSceneChange,
                (int32_t audioScene), (override));
    MOCK_METHOD(ErrCode, OnSpatializationEnabledChangeForCurrentDevice,
                (bool enabled), (override));
    MOCK_METHOD(ErrCode, OnFormatUnsupportedError,
                (int32_t errorCode), (override));
    MOCK_METHOD(ErrCode, OnStreamVolumeChange,
                (const StreamVolumeEvent& streamVolumeEvent), (override));
    MOCK_METHOD(ErrCode, OnSystemVolumeChange,
                (const VolumeEvent& volumeEvent), (override));
    MOCK_METHOD(ErrCode, OnAudioSessionStateChanged,
                (int32_t stateChangeHint), (override));
    MOCK_METHOD(ErrCode, OnAudioSessionCurrentDeviceChanged,
                (const CurrentOutputDeviceChangedEvent& deviceChangedEvent), (override));
    MOCK_METHOD(ErrCode, OnAudioSessionCurrentInputDeviceChanged,
                (const CurrentInputDeviceChangedEvent& deviceChangedEvent), (override));
    MOCK_METHOD(ErrCode, OnVolumeDegreeEvent,
            (const VolumeEvent& volumeEvent), (override));
    MOCK_METHOD(ErrCode, OnCollaborationEnabledChangeForCurrentDevice,
            (const bool enabled), (override));
};

} // namespace AudioStandard
} // namespace OHOS

#endif // MOCK_AUDIO_POLICY_CLIENT_H