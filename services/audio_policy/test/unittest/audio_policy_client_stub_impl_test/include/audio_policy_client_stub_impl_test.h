/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef AUDIO_POLICY_CLIENT_STUB_IMPL_TEST_H
#define AUDIO_POLICY_CLIENT_STUB_IMPL_TEST_H

#include "gtest/gtest.h"
#include "audio_policy_client_stub_impl.h"

namespace OHOS {
namespace AudioStandard {

class AudioPolicyClientStubImplTest : public testing::Test {
public:
    // SetUpTestCase: Called before all test cases
    static void SetUpTestCase(void);
    // TearDownTestCase: Called after all test case
    static void TearDownTestCase(void);
    // SetUp: Called before each test cases
    void SetUp(void);
    // TearDown: Called after each test cases
    void TearDown(void);
};

class ConcreteVolumeKeyEventCallback : public VolumeKeyEventCallback {
public:
    ConcreteVolumeKeyEventCallback() {}
    void OnVolumeKeyEvent(VolumeEvent volumeEvent) override {}
};

class ConcreteAudioFocusInfoChangeCallback : public AudioFocusInfoChangeCallback {
public:
    ConcreteAudioFocusInfoChangeCallback() {}
    void OnAudioFocusInfoChange(const std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList) override {}
};

class ConcreteAudioManagerDeviceChangeCallback : public AudioManagerDeviceChangeCallback {
public:
    ConcreteAudioManagerDeviceChangeCallback() {}
    void OnDeviceChange(const DeviceChangeAction &deviceChangeAction) override {}
};

class ConcreteAudioRingerModeCallback : public AudioRingerModeCallback {
public:
    ConcreteAudioRingerModeCallback() {};
    void OnRingerModeUpdated(const AudioRingerMode &ringerMode) override {}
};

class ConcreteAudioSessionCallback : public AudioSessionCallback {
public:
    void OnAudioSessionDeactive(const AudioSessionDeactiveEvent &deactiveEvent) override {}
};

class ConcreteAudioManagerMicStateChangeCallback : public AudioManagerMicStateChangeCallback {
    void OnMicStateUpdated(const MicStateChangeEvent &micStateChangeEvent) override {}
};

class ConcreteAudioManagerMicrophoneBlockedCallback : public AudioManagerMicrophoneBlockedCallback {
    void OnMicrophoneBlocked(const MicrophoneBlockedInfo &microphoneBlockedInfo) override {}
};

class ConcreteAudioPreferredOutputDeviceChangeCallback : public AudioPreferredOutputDeviceChangeCallback {
    void OnPreferredOutputDeviceUpdated(const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc) override {}
};

class ConcreteAudioPreferredInputDeviceChangeCallback : public AudioPreferredInputDeviceChangeCallback {
    void OnPreferredInputDeviceUpdated(const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc) override {}
};

class ConcreteAudioRendererStateChangeCallback : public AudioRendererStateChangeCallback {
    void OnRendererStateChange(
        const std::vector<std::shared_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos) override {}
};

class ConcreteDeviceChangeWithInfoCallback : public DeviceChangeWithInfoCallback {
    void OnDeviceChangeWithInfo(const uint32_t sessionId, const AudioDeviceDescriptor &deviceInfo,
            const AudioStreamDeviceChangeReasonExt reason) override {}
    void OnRecreateStreamEvent(const uint32_t sessionId, const int32_t streamFlag,
        const AudioStreamDeviceChangeReasonExt reason) override {}
};

class ConcreteAudioCapturerStateChangeCallback : public AudioCapturerStateChangeCallback {
    void OnCapturerStateChange(
        const std::vector<std::shared_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos) override {}
};

class ConcreteHeadTrackingDataRequestedChangeCallback : public HeadTrackingDataRequestedChangeCallback {
    void OnHeadTrackingDataRequestedChange(bool isRequested) override {}
};

class ConcreteAudioSpatializationEnabledChangeCallback : public AudioSpatializationEnabledChangeCallback {
    void OnSpatializationEnabledChange(const bool &enabled) override {}
    void OnSpatializationEnabledChangeForAnyDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor,
        const bool &enabled) override {}
};

class ConcreteAudioHeadTrackingEnabledChangeCallback : public AudioHeadTrackingEnabledChangeCallback {
    void OnHeadTrackingEnabledChange(const bool &enabled) override {}
    void OnHeadTrackingEnabledChangeForAnyDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor,
        const bool &enabled) override {}
};

class ConcreteSpatialEnabledChangeForCurrentDeviceCb : public AudioSpatializationEnabledChangeForCurrentDeviceCallback {
    void OnSpatializationEnabledChangeForCurrentDevice(const bool &enabled) override {}
};

class ConcreteAudioManagerAppVolumeChangeCallback : public AudioManagerAppVolumeChangeCallback {
    void OnAppVolumeChangedForUid(int32_t appUid, const VolumeEvent &event) override {};

    void OnSelfAppVolumeChanged(const VolumeEvent &event) override {};
};

class ConcreteAudioManagerAudioSceneChangedCallback : public AudioManagerAudioSceneChangedCallback {
    void OnAudioSceneChange(const AudioScene audioScene) override {};
};

class ConcreteAudioFormatUnsupportedErrorCallback : public AudioFormatUnsupportedErrorCallback {
    void OnFormatUnsupportedError(const AudioErrors &errorCode) override {};
};

class ConcreteSystemVolumeChangeCallback : public SystemVolumeChangeCallback {
    void OnSystemVolumeChange(VolumeEvent volumeEvent) override {};
};

class ConcreteAudioManagerActiveVolumeTypeChangeCallback : public AudioManagerActiveVolumeTypeChangeCallback {
    void OnActiveVolumeTypeChanged(const AudioVolumeType &event) override {};
};

class ConcreteAudioSessionStateChangedCallback : public AudioSessionStateChangedCallback {
    void OnAudioSessionStateChanged(const AudioSessionStateChangedEvent &stateChangedEvent) override {};
};

class ConcreteAudioSessionCurrentDeviceChangedCallback : public AudioSessionCurrentDeviceChangedCallback {
    void OnAudioSessionCurrentDeviceChanged(const CurrentOutputDeviceChangedEvent &deviceChangedEvent) override {};
};

class ConcreteStreamVolumeChangeCallback : public StreamVolumeChangeCallback {
    void OnStreamVolumeChange(StreamVolumeEvent streamVolumeEvent) override {};
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_POLICY_CLIENT_STUB_IMPL_TEST_H
