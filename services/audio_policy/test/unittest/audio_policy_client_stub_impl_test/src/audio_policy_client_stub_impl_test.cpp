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

#include "audio_policy_client_stub_impl_test.h"
#include "audio_policy_client.h"

#include <iostream>
#include "gtest/gtest.h"
#include <gmock/gmock.h>
#include "audio_errors.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioPolicyClientStubImplTest::SetUpTestCase(void) {}
void AudioPolicyClientStubImplTest::TearDownTestCase(void) {}
void AudioPolicyClientStubImplTest::SetUp(void) {}
void AudioPolicyClientStubImplTest::TearDown(void) {}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_001
* @tc.desc  : Test AddVolumeKeyEventCallback/RemoveVolumeKeyEventCallback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_001, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto mockCallback0 = std::make_shared<ConcreteVolumeKeyEventCallback>();
    int32_t result = audioPolicyClient->AddVolumeKeyEventCallback(mockCallback0);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(audioPolicyClient->volumeKeyEventCallbackList_.size(), 1);

    auto mockCallback1 = std::make_shared<ConcreteVolumeKeyEventCallback>();
    EXPECT_EQ(audioPolicyClient->AddVolumeKeyEventCallback(mockCallback1), SUCCESS);
    EXPECT_EQ(audioPolicyClient->volumeKeyEventCallbackList_.size(), 2);
    EXPECT_EQ(audioPolicyClient->RemoveVolumeKeyEventCallback(mockCallback0), SUCCESS);
    EXPECT_EQ(audioPolicyClient->volumeKeyEventCallbackList_.size(), 1);
    EXPECT_EQ(audioPolicyClient->RemoveVolumeKeyEventCallback(nullptr), SUCCESS);
    EXPECT_EQ(audioPolicyClient->volumeKeyEventCallbackList_.size(), 0);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_002
* @tc.desc  : Test AddFocusInfoChangeCallback/RemoveFocusInfoChangeCallback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_002, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    int32_t result = audioPolicyClient->AddFocusInfoChangeCallback(nullptr);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(audioPolicyClient->focusInfoChangeCallbackList_.size(), 1);

    auto mockCallback0 = std::make_shared<ConcreteAudioFocusInfoChangeCallback>();
    result = audioPolicyClient->AddFocusInfoChangeCallback(mockCallback0);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(audioPolicyClient->focusInfoChangeCallbackList_.size(), 2);

    result = audioPolicyClient->RemoveFocusInfoChangeCallback();
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(audioPolicyClient->focusInfoChangeCallbackList_.size(), 0);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_004
* @tc.desc  : Test DeviceFilterByFlag.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_004, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescs;
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDescs.push_back(deviceDesc);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> result = audioPolicyClient->
        DeviceFilterByFlag(DeviceFlag::ALL_DEVICES_FLAG, deviceDescs);
    EXPECT_NE(audioPolicyClient, nullptr);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_005
* @tc.desc  : Test DeviceFilterByFlag.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_005, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescs;
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDescs.push_back(deviceDesc);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> result = audioPolicyClient->
        DeviceFilterByFlag(DeviceFlag::ALL_DISTRIBUTED_DEVICES_FLAG, deviceDescs);
    EXPECT_NE(audioPolicyClient, nullptr);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_006
* @tc.desc  : Test DeviceFilterByFlag.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_006, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescs;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> result = audioPolicyClient->
        DeviceFilterByFlag(DeviceFlag::ALL_L_D_DEVICES_FLAG, deviceDescs);
    EXPECT_NE(audioPolicyClient, nullptr);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_007
* @tc.desc  : Test DeviceFilterByFlag.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_007, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescs;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> result = audioPolicyClient->
        DeviceFilterByFlag(DeviceFlag::OUTPUT_DEVICES_FLAG, deviceDescs);
    EXPECT_NE(audioPolicyClient, nullptr);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_008
* @tc.desc  : Test DeviceFilterByFlag.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_008, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescs;
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDescs.push_back(deviceDesc);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> result = audioPolicyClient->
        DeviceFilterByFlag(DeviceFlag::INPUT_DEVICES_FLAG, deviceDescs);
    EXPECT_NE(audioPolicyClient, nullptr);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_009
* @tc.desc  : Test DeviceFilterByFlag.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_009, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescs;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> result = audioPolicyClient->
        DeviceFilterByFlag(DeviceFlag::DISTRIBUTED_OUTPUT_DEVICES_FLAG, deviceDescs);
    EXPECT_NE(audioPolicyClient, nullptr);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_010
* @tc.desc  : Test DeviceFilterByFlag.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_010, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescs;
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDescs.push_back(deviceDesc);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> result = audioPolicyClient->
        DeviceFilterByFlag(DeviceFlag::DISTRIBUTED_INPUT_DEVICES_FLAG, deviceDescs);
    EXPECT_NE(audioPolicyClient, nullptr);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_011
* @tc.desc  : Test DeviceFilterByFlag.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_011, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescs;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> result = audioPolicyClient->
        DeviceFilterByFlag(DeviceFlag::DEVICE_FLAG_MAX, deviceDescs);
    EXPECT_NE(audioPolicyClient, nullptr);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_012
* @tc.desc  : Test AddDeviceChangeCallback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_012, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    std::shared_ptr<AudioManagerDeviceChangeCallback> cb = std::make_shared<ConcreteAudioManagerDeviceChangeCallback>();
    int32_t result = audioPolicyClient->AddDeviceChangeCallback(DeviceFlag::DISTRIBUTED_INPUT_DEVICES_FLAG, cb);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(audioPolicyClient->deviceChangeCallbackList_.size(), 1);

    std::shared_ptr<AudioManagerDeviceChangeCallback> rcb =
        std::make_shared<ConcreteAudioManagerDeviceChangeCallback>();
    rcb = nullptr;
    result = audioPolicyClient->RemoveDeviceChangeCallback(DeviceFlag::DISTRIBUTED_INPUT_DEVICES_FLAG, rcb);
    EXPECT_EQ(result, SUCCESS);

    result = audioPolicyClient->RemoveDeviceChangeCallback(DeviceFlag::DISTRIBUTED_INPUT_DEVICES_FLAG, cb);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_013
* @tc.desc  : Test AddRingerModeCallback/RemoveRingerModeCallback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_013, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb0 = std::make_shared<ConcreteAudioRingerModeCallback>();
    auto cb1 = std::make_shared<ConcreteAudioRingerModeCallback>();
    EXPECT_EQ(audioPolicyClient->AddRingerModeCallback(cb0), SUCCESS);
    EXPECT_EQ(audioPolicyClient->ringerModeCallbackList_.size(), 1);
    EXPECT_EQ(audioPolicyClient->AddRingerModeCallback(cb1), SUCCESS);
    EXPECT_EQ(audioPolicyClient->ringerModeCallbackList_.size(), 2);

    audioPolicyClient->OnRingerModeUpdated(AudioRingerMode::RINGER_MODE_SILENT);
    EXPECT_NE(audioPolicyClient, nullptr);

    int32_t result = audioPolicyClient->RemoveRingerModeCallback(cb1);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(audioPolicyClient->ringerModeCallbackList_.size(), 1);

    result = audioPolicyClient->RemoveRingerModeCallback();
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(audioPolicyClient->ringerModeCallbackList_.size(), 0);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_014
* @tc.desc  : Test AddAudioSessionCallback/RemoveAudioSessionCallback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_014, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    std::shared_ptr<AudioSessionCallback> cb0 = std::make_shared<ConcreteAudioSessionCallback>();
    std::shared_ptr<AudioSessionCallback> cb1 = std::make_shared<ConcreteAudioSessionCallback>();
    EXPECT_EQ(audioPolicyClient->AddAudioSessionCallback(cb0), SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetAudioSessionCallbackSize(), 1);

    EXPECT_EQ(audioPolicyClient->AddAudioSessionCallback(cb1), SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetAudioSessionCallbackSize(), 2);

    int32_t deactiveEvent = 0;
    audioPolicyClient->OnAudioSessionDeactive(deactiveEvent);
    EXPECT_NE(audioPolicyClient, nullptr);

    int32_t result = audioPolicyClient->RemoveAudioSessionCallback(cb1);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetAudioSessionCallbackSize(), 1);

    result = audioPolicyClient->RemoveAudioSessionCallback();
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetAudioSessionCallbackSize(), 0);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_015
* @tc.desc  : Test AddMicStateChangeCallback/RemoveMicStateChangeCallback/OnMicStateUpdated.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_015, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    std::shared_ptr<AudioManagerMicStateChangeCallback> cb0 =
        std::make_shared<ConcreteAudioManagerMicStateChangeCallback>();
    std::shared_ptr<AudioManagerMicStateChangeCallback> cb1 =
        std::make_shared<ConcreteAudioManagerMicStateChangeCallback>();
    EXPECT_FALSE(audioPolicyClient->HasMicStateChangeCallback());
    EXPECT_EQ(audioPolicyClient->AddMicStateChangeCallback(cb0), SUCCESS);
    EXPECT_EQ(audioPolicyClient->AddMicStateChangeCallback(cb1), SUCCESS);

    MicStateChangeEvent micStateChangeEvent;
    audioPolicyClient->OnMicStateUpdated(micStateChangeEvent);
    EXPECT_NE(audioPolicyClient, nullptr);

    EXPECT_TRUE(audioPolicyClient->HasMicStateChangeCallback());

    EXPECT_EQ(audioPolicyClient->RemoveMicStateChangeCallback(), SUCCESS);
    EXPECT_FALSE(audioPolicyClient->HasMicStateChangeCallback());
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_016
* @tc.desc  : Test OnVolumeKeyEvent.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_016, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto mockCallback0 = std::make_shared<ConcreteVolumeKeyEventCallback>();
    int32_t result = audioPolicyClient->AddVolumeKeyEventCallback(mockCallback0);
    EXPECT_EQ(result, SUCCESS);
    VolumeEvent volumeEvent;
    audioPolicyClient->OnVolumeKeyEvent(volumeEvent);
    EXPECT_NE(audioPolicyClient, nullptr);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_017
* @tc.desc  : Test OnDeviceChange.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_017, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb0 = std::make_shared<ConcreteAudioManagerDeviceChangeCallback>();
    int32_t result = audioPolicyClient->AddDeviceChangeCallback(DeviceFlag::DISTRIBUTED_INPUT_DEVICES_FLAG, cb0);
    EXPECT_EQ(result, SUCCESS);

    auto cb1 = std::make_shared<ConcreteAudioManagerDeviceChangeCallback>();
    result = audioPolicyClient->AddDeviceChangeCallback(DeviceFlag::DISTRIBUTED_INPUT_DEVICES_FLAG, cb1);
    EXPECT_EQ(result, SUCCESS);

    DeviceChangeAction dca;
    audioPolicyClient->OnDeviceChange(dca);
    EXPECT_NE(audioPolicyClient, nullptr);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_018
* @tc.desc  : Test OnMicrophoneBlocked/RemoveMicrophoneBlockedCallback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_018, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb1 = std::make_shared<ConcreteAudioManagerMicrophoneBlockedCallback>();
    int32_t result = audioPolicyClient->AddMicrophoneBlockedCallback(1, cb1);
    EXPECT_EQ(result, SUCCESS);

    auto cb2 = std::make_shared<ConcreteAudioManagerMicrophoneBlockedCallback>();
    result = audioPolicyClient->AddMicrophoneBlockedCallback(2, cb2);
    EXPECT_EQ(result, SUCCESS);

    MicrophoneBlockedInfo blockedInfo;
    audioPolicyClient->OnMicrophoneBlocked(blockedInfo);
    EXPECT_NE(audioPolicyClient, nullptr);

    result = audioPolicyClient->RemoveMicrophoneBlockedCallback(1, nullptr);
    EXPECT_EQ(result, SUCCESS);

    result = audioPolicyClient->RemoveMicrophoneBlockedCallback(2, nullptr);
    EXPECT_EQ(result, SUCCESS);

    result = audioPolicyClient->RemoveMicrophoneBlockedCallback(1, cb1);
    EXPECT_EQ(result, SUCCESS);

    result = audioPolicyClient->RemoveMicrophoneBlockedCallback(2, cb2);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_019
* @tc.desc  : Test OnPreferredOutputDeviceUpdated.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_019, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteAudioPreferredOutputDeviceChangeCallback>();
    AudioRendererInfo rendererInfo;
    int32_t result = audioPolicyClient->AddPreferredOutputDeviceChangeCallback(rendererInfo, cb);
    EXPECT_EQ(result, SUCCESS);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc;
    audioPolicyClient->OnPreferredOutputDeviceUpdated(rendererInfo, desc);
    EXPECT_NE(audioPolicyClient, nullptr);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_020
* @tc.desc  : Test OnPreferredInputDeviceUpdated.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_020, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteAudioPreferredInputDeviceChangeCallback>();
    AudioCapturerInfo capturerInfo;
    int32_t result = audioPolicyClient->AddPreferredInputDeviceChangeCallback(capturerInfo, cb);
    EXPECT_EQ(result, SUCCESS);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc;
    audioPolicyClient->OnPreferredInputDeviceUpdated(capturerInfo, desc);
    EXPECT_NE(audioPolicyClient, nullptr);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_021
* @tc.desc  : Test AddRendererStateChangeCallback/RemoveRendererStateChangeCallback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_021, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteAudioRendererStateChangeCallback>();
    int32_t result = audioPolicyClient->AddRendererStateChangeCallback(cb);
    EXPECT_EQ(result, SUCCESS);

    cb = nullptr;
    result = audioPolicyClient->AddRendererStateChangeCallback(cb);
    EXPECT_EQ(result, ERR_INVALID_PARAM);

    std::vector<std::shared_ptr<AudioRendererStateChangeCallback>> callbacks;
    result = audioPolicyClient->RemoveRendererStateChangeCallback(callbacks);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_022
* @tc.desc  : Test OnRendererDeviceChange.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_022, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteDeviceChangeWithInfoCallback>();
    int32_t result = audioPolicyClient->AddDeviceChangeWithInfoCallback(1, cb);
    EXPECT_EQ(result, SUCCESS);

    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    AudioStreamDeviceChangeReasonExt reason(AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE);
    audioPolicyClient->OnRendererDeviceChange(0, deviceInfo, reason);
    EXPECT_NE(audioPolicyClient, nullptr);

    audioPolicyClient->OnRendererDeviceChange(1, deviceInfo, reason);
    EXPECT_NE(audioPolicyClient, nullptr);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_023
* @tc.desc  : Test OnRendererStateChange.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_023, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteAudioRendererStateChangeCallback>();
    int32_t result = audioPolicyClient->AddRendererStateChangeCallback(cb);
    EXPECT_EQ(result, SUCCESS);

    std::vector<std::shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    audioPolicyClient->OnRendererStateChange(audioRendererChangeInfos);
    EXPECT_NE(audioPolicyClient, nullptr);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_024
* @tc.desc  : Test OnRecreateRendererStreamEvent/OnRecreateCapturerStreamEvent.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_024, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteDeviceChangeWithInfoCallback>();
    int32_t result = audioPolicyClient->AddDeviceChangeWithInfoCallback(1, cb);
    EXPECT_EQ(result, SUCCESS);

    AudioStreamDeviceChangeReasonExt reason(AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE);
    audioPolicyClient->OnRecreateRendererStreamEvent(0, 0, reason);
    EXPECT_NE(audioPolicyClient, nullptr);
    audioPolicyClient->OnRecreateCapturerStreamEvent(0, 0, reason);
    EXPECT_NE(audioPolicyClient, nullptr);

    audioPolicyClient->OnRecreateRendererStreamEvent(1, 0, reason);
    EXPECT_NE(audioPolicyClient, nullptr);
    audioPolicyClient->OnRecreateCapturerStreamEvent(1, 0, reason);
    EXPECT_NE(audioPolicyClient, nullptr);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_025
* @tc.desc  : Test OnCapturerStateChange.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_025, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteAudioCapturerStateChangeCallback>();
    int32_t result = audioPolicyClient->AddCapturerStateChangeCallback(cb);
    EXPECT_EQ(result, SUCCESS);

    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    audioPolicyClient->OnCapturerStateChange(audioCapturerChangeInfos);
    EXPECT_NE(audioPolicyClient, nullptr);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_026
* @tc.desc  : Test AddHeadTrackingDataRequestedChangeCallback/OnHeadTrackingDeviceChange.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_026, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    std::unordered_map<std::string, bool> changeInfo;
    changeInfo.insert({"test1", true});
    changeInfo.insert({"test2", false});
    audioPolicyClient->OnHeadTrackingDeviceChange(changeInfo);
    EXPECT_NE(audioPolicyClient, nullptr);

    auto cb0 = std::make_shared<ConcreteHeadTrackingDataRequestedChangeCallback>();
    int32_t result = audioPolicyClient->AddHeadTrackingDataRequestedChangeCallback("test", cb0);
    EXPECT_EQ(result, SUCCESS);

    auto cb1 = std::make_shared<ConcreteHeadTrackingDataRequestedChangeCallback>();
    audioPolicyClient->AddHeadTrackingDataRequestedChangeCallback("test", cb1);
    EXPECT_EQ(result, SUCCESS);

    audioPolicyClient->OnHeadTrackingDeviceChange(changeInfo);
    EXPECT_NE(audioPolicyClient, nullptr);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_027
* @tc.desc  : Test OnSpatializationEnabledChange/OnSpatializationEnabledChangeForAnyDevice.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_027, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteAudioSpatializationEnabledChangeCallback>();
    int32_t result = audioPolicyClient->AddSpatializationEnabledChangeCallback(cb);
    EXPECT_EQ(result, SUCCESS);

    bool enabled = true;
    audioPolicyClient->OnSpatializationEnabledChange(enabled);
    EXPECT_NE(audioPolicyClient, nullptr);

    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioPolicyClient->OnSpatializationEnabledChangeForAnyDevice(deviceDescriptor, enabled);
    EXPECT_NE(audioPolicyClient, nullptr);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_028
* @tc.desc  : Test OnHeadTrackingEnabledChange/OnHeadTrackingEnabledChangeForAnyDevice.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_028, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteAudioHeadTrackingEnabledChangeCallback>();
    int32_t result = audioPolicyClient->AddHeadTrackingEnabledChangeCallback(cb);
    EXPECT_EQ(result, SUCCESS);

    bool enabled = true;
    audioPolicyClient->OnHeadTrackingEnabledChange(enabled);
    EXPECT_NE(audioPolicyClient, nullptr);

    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioPolicyClient->OnHeadTrackingEnabledChangeForAnyDevice(deviceDescriptor, enabled);
    EXPECT_NE(audioPolicyClient, nullptr);
}

/**
* @tc.name  : Test AudioDeviceDescriptor.
* @tc.number: AudioDeviceDescriptor_001
* @tc.desc  : Test AudioDeviceDescriptor/MapInternalToExternalDeviceType.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioDeviceDescriptor_001, TestSize.Level1)
{
    AudioDeviceDescriptor deviceDescriptor;
    deviceDescriptor.hasPair_ = true;
    deviceDescriptor.deviceType_ = DEVICE_TYPE_USB_HEADSET;
    EXPECT_EQ(deviceDescriptor.MapInternalToExternalDeviceType(API_VERSION_MAX), DEVICE_TYPE_USB_HEADSET);
    deviceDescriptor.deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;
    EXPECT_EQ(deviceDescriptor.MapInternalToExternalDeviceType(API_VERSION_MAX), DEVICE_TYPE_USB_HEADSET);
    deviceDescriptor.hasPair_ = false;
    deviceDescriptor.deviceRole_ = OUTPUT_DEVICE;
    deviceDescriptor.MapInternalToExternalDeviceType(API_VERSION_MAX);
    deviceDescriptor.deviceRole_ = INPUT_DEVICE;
    EXPECT_EQ(deviceDescriptor.MapInternalToExternalDeviceType(API_VERSION_MAX), DEVICE_TYPE_USB_DEVICE);
    deviceDescriptor.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP_IN;
    EXPECT_EQ(deviceDescriptor.MapInternalToExternalDeviceType(API_VERSION_MAX), DEVICE_TYPE_BLUETOOTH_A2DP);
    deviceDescriptor.deviceType_ = DEVICE_TYPE_SPEAKER;
    EXPECT_EQ(deviceDescriptor.MapInternalToExternalDeviceType(API_VERSION_MAX), DEVICE_TYPE_SPEAKER);
}

/**
* @tc.name  : Test AudioDeviceDescriptor.
* @tc.number: GetDeviceCategory_001
* @tc.desc  : Test AudioDeviceDescriptor/GetDeviceCategory.
*/
HWTEST(AudioPolicyClientStubImplTest, GetDeviceCategory_001, TestSize.Level1)
{
    AudioDeviceDescriptor deviceDescriptor;
    deviceDescriptor.hasPair_ = true;
    deviceDescriptor.deviceType_ = DEVICE_TYPE_USB_HEADSET;
    EXPECT_EQ(deviceDescriptor.GetDeviceCategory(), CATEGORY_DEFAULT);
}

/**
* @tc.name  : Test AudioDeviceDescriptor.
* @tc.number: Dump_001
* @tc.desc  : Test AudioDeviceDescriptor/Dump.
*/
HWTEST(AudioPolicyClientStubImplTest, Dump_001, TestSize.Level1)
{
    string dumpString = "";
    AudioDeviceDescriptor deviceDescriptor;
    deviceDescriptor.deviceName_ = "Test";
    deviceDescriptor.deviceRole_ = DEVICE_ROLE_NONE;
    deviceDescriptor.hasPair_ = true;
    deviceDescriptor.deviceType_ = DEVICE_TYPE_USB_HEADSET;

    deviceDescriptor.Dump(dumpString);
    EXPECT_EQ(dumpString.empty(), false);

    dumpString = "";
    deviceDescriptor.deviceRole_ = INPUT_DEVICE;
    deviceDescriptor.Dump(dumpString);
    EXPECT_EQ(dumpString.empty(), false);

    dumpString = "";
    deviceDescriptor.deviceRole_ = OUTPUT_DEVICE;
    deviceDescriptor.Dump(dumpString);
    EXPECT_EQ(dumpString.empty(), false);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_029
* @tc.desc  : Test DeviceFilterByFlag.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_029, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClient != nullptr);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc;
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->networkId_ = LOCAL_NETWORK_ID;
    desc.push_back(deviceDesc);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> result = audioPolicyClient->
        DeviceFilterByFlag(DeviceFlag::ALL_DEVICES_FLAG, desc);
    EXPECT_TRUE(result.size() != 0);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_030
* @tc.desc  : Test DeviceFilterByFlag.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_030, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClient != nullptr);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc;
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->networkId_ = LOCAL_NETWORK_ID;
    desc.push_back(deviceDesc);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> result = audioPolicyClient->
        DeviceFilterByFlag(DeviceFlag::ALL_DISTRIBUTED_DEVICES_FLAG, desc);
    EXPECT_TRUE(result.size() == 0);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_031
* @tc.desc  : Test DeviceFilterByFlag.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_031, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClient != nullptr);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc;
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->networkId_ = LOCAL_NETWORK_ID;
    deviceDesc->deviceRole_ = INPUT_DEVICE;
    desc.push_back(deviceDesc);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> result = audioPolicyClient->
        DeviceFilterByFlag(DeviceFlag::OUTPUT_DEVICES_FLAG, desc);
    EXPECT_TRUE(result.size() == 0);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_032
* @tc.desc  : Test DeviceFilterByFlag.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_032, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClient != nullptr);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc;
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->networkId_ = "test";
    deviceDesc->deviceRole_ = OUTPUT_DEVICE;
    desc.push_back(deviceDesc);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> result = audioPolicyClient->
        DeviceFilterByFlag(DeviceFlag::OUTPUT_DEVICES_FLAG, desc);
    EXPECT_TRUE(result.size() == 0);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_033
* @tc.desc  : Test DeviceFilterByFlag.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_033, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClient != nullptr);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc;
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->networkId_ = LOCAL_NETWORK_ID;
    deviceDesc->deviceRole_ = INPUT_DEVICE;
    desc.push_back(deviceDesc);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> result = audioPolicyClient->
        DeviceFilterByFlag(DeviceFlag::DISTRIBUTED_INPUT_DEVICES_FLAG, desc);
        EXPECT_TRUE(result.size() == 0);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_034
* @tc.desc  : Test DeviceFilterByFlag.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_034, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClient != nullptr);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc;
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceRole_ = INPUT_DEVICE;
    deviceDesc->networkId_ = "test";
    desc.push_back(deviceDesc);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> result = audioPolicyClient->
        DeviceFilterByFlag(DeviceFlag::DISTRIBUTED_INPUT_DEVICES_FLAG, desc);
    EXPECT_NE(audioPolicyClient, nullptr);
    EXPECT_TRUE(result.size() != 0);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_035
* @tc.desc  : Test DeviceFilterByFlag.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_035, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClient != nullptr);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc;
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceRole_ = OUTPUT_DEVICE;
    deviceDesc->networkId_ = "test";
    desc.push_back(deviceDesc);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> result = audioPolicyClient->
        DeviceFilterByFlag(DeviceFlag::DISTRIBUTED_INPUT_DEVICES_FLAG, desc);
    EXPECT_TRUE(result.size() == 0);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_036
* @tc.desc  : Test OnDeviceChange.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_036, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClient != nullptr);

    auto cb0 = std::make_shared<ConcreteAudioManagerDeviceChangeCallback>();
    audioPolicyClient->AddDeviceChangeCallback(DeviceFlag::ALL_DEVICES_FLAG, cb0);

    DeviceChangeAction dca;
    audioPolicyClient->OnDeviceChange(dca);
    EXPECT_NE(audioPolicyClient, nullptr);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_037
* @tc.desc  : Test OnDeviceChange.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_037, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClient != nullptr);

    audioPolicyClient->AddDeviceChangeCallback(DeviceFlag::ALL_DEVICES_FLAG, nullptr);

    DeviceChangeAction dca;
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->networkId_ = LOCAL_NETWORK_ID;
    dca.deviceDescriptors.push_back(deviceDesc);
    audioPolicyClient->OnDeviceChange(dca);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_038
* @tc.desc  : Test OnMicrophoneBlocked.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_038, TestSize.Level1)
{
    int32_t clientId = 1;
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClient != nullptr);

    auto cb1 = std::make_shared<ConcreteAudioManagerMicrophoneBlockedCallback>();
    audioPolicyClient->AddMicrophoneBlockedCallback(clientId, cb1);

    MicrophoneBlockedInfo blockedInfo;
    audioPolicyClient->OnMicrophoneBlocked(blockedInfo);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_039
* @tc.desc  : Test OnMicrophoneBlocked.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_039, TestSize.Level1)
{
    int32_t clientId = 1;
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClient != nullptr);

    auto cb1 = std::make_shared<ConcreteAudioManagerMicrophoneBlockedCallback>();
    audioPolicyClient->AddMicrophoneBlockedCallback(clientId, cb1);

    MicrophoneBlockedInfo blockedInfo;
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    blockedInfo.devices.push_back(deviceDesc);
    audioPolicyClient->OnMicrophoneBlocked(blockedInfo);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_040
* @tc.desc  : Test RemovePreferredOutputDeviceChangeCallback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_040, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClient != nullptr);

    AudioRendererInfo rendererInfo;
    auto cb = std::make_shared<ConcreteAudioPreferredOutputDeviceChangeCallback>();
    audioPolicyClient->AddPreferredOutputDeviceChangeCallback(rendererInfo, cb);
    auto ret = audioPolicyClient->RemovePreferredOutputDeviceChangeCallback(cb);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_041
* @tc.desc  : Test RemovePreferredInputDeviceChangeCallback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_041, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClient != nullptr);

    AudioCapturerInfo capturerInfo;
    auto cb = std::make_shared<ConcreteAudioPreferredInputDeviceChangeCallback>();
    audioPolicyClient->AddPreferredInputDeviceChangeCallback(capturerInfo, cb);
    auto ret = audioPolicyClient->RemovePreferredInputDeviceChangeCallback(cb);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_042
* @tc.desc  : Test OnRendererDeviceChange.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_042, TestSize.Level1)
{
    uint32_t sessionId = 1;
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClient != nullptr);

    auto cb = std::make_shared<ConcreteDeviceChangeWithInfoCallback>();
    audioPolicyClient->AddDeviceChangeWithInfoCallback(sessionId, cb);

    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    AudioStreamDeviceChangeReasonExt reason(AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE);
    audioPolicyClient->OnRendererDeviceChange(sessionId, deviceInfo, reason);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_043
* @tc.desc  : Test OnRecreateRendererStreamEvent.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_043, TestSize.Level1)
{
    uint32_t sessionId0 = 0;
    uint32_t sessionId1 = 1;
    int32_t streamFlag = 1;
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClient != nullptr);

    auto cb = std::make_shared<ConcreteDeviceChangeWithInfoCallback>();
    audioPolicyClient->AddDeviceChangeWithInfoCallback(sessionId0, cb);

    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    AudioStreamDeviceChangeReasonExt reason(AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE);
    audioPolicyClient->OnRecreateRendererStreamEvent(sessionId0, streamFlag, reason);

    cb = nullptr;
    audioPolicyClient->RemoveDeviceChangeWithInfoCallback(sessionId0);
    audioPolicyClient->AddDeviceChangeWithInfoCallback(sessionId1, cb);
    audioPolicyClient->OnRecreateRendererStreamEvent(sessionId1, streamFlag, reason);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_044
* @tc.desc  : Test OnRecreateCapturerStreamEvent.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_044, TestSize.Level1)
{
    uint32_t sessionId0 = 0;
    uint32_t sessionId1 = 1;
    int32_t streamFlag = 1;
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClient != nullptr);

    auto cb = std::make_shared<ConcreteDeviceChangeWithInfoCallback>();
    audioPolicyClient->AddDeviceChangeWithInfoCallback(sessionId0, cb);

    AudioStreamDeviceChangeReasonExt reason(AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE);
    audioPolicyClient->OnRecreateCapturerStreamEvent(sessionId0, streamFlag, reason);

    cb = nullptr;
    audioPolicyClient->RemoveDeviceChangeWithInfoCallback(sessionId0);
    audioPolicyClient->AddDeviceChangeWithInfoCallback(sessionId1, cb);
    audioPolicyClient->OnRecreateRendererStreamEvent(sessionId1, streamFlag, reason);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_045
* @tc.desc  : Test OnCapturerStateChange.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_045, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClient != nullptr);

    auto cb = std::make_shared<ConcreteAudioCapturerStateChangeCallback>();
    audioPolicyClient->AddCapturerStateChangeCallback(cb);

    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    audioPolicyClient->OnCapturerStateChange(audioCapturerChangeInfos);

    cb = nullptr;
    audioPolicyClient->RemoveCapturerStateChangeCallback();
    audioPolicyClient->AddCapturerStateChangeCallback(cb);
    audioPolicyClient->OnCapturerStateChange(audioCapturerChangeInfos);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_046
* @tc.desc  : Test OnHeadTrackingDeviceChange.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_046, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClient != nullptr);

    auto cb0 = std::make_shared<ConcreteHeadTrackingDataRequestedChangeCallback>();
    audioPolicyClient->AddHeadTrackingDataRequestedChangeCallback("test1", cb0);

    std::unordered_map<std::string, bool> changeInfo;
    changeInfo.insert({"test1", true});
    audioPolicyClient->OnHeadTrackingDeviceChange(changeInfo);
    EXPECT_NE(audioPolicyClient, nullptr);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_047
* @tc.desc  : Test OnHeadTrackingDeviceChange.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_047, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClient != nullptr);

    std::shared_ptr<HeadTrackingDataRequestedChangeCallback> cb = nullptr;
    audioPolicyClient->AddHeadTrackingDataRequestedChangeCallback("test1", cb);

    std::unordered_map<std::string, bool> changeInfo;
    changeInfo.insert({"test1", true});
    audioPolicyClient->OnHeadTrackingDeviceChange(changeInfo);
    EXPECT_NE(audioPolicyClient, nullptr);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_048
* @tc.desc  : Test RemoveAllSelfAppVolumeChangeCallback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_048, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClient != nullptr);

    int32_t appUid = 0;
    audioPolicyClient->RemoveAllSelfAppVolumeChangeCallback(appUid);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_049
* @tc.desc  : Test RemoveAllSelfAppVolumeChangeCallback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_049, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClient != nullptr);

    int32_t appUid = 0;
    audioPolicyClient->selfAppVolumeChangeCallbackNum_[appUid] = 1;
    audioPolicyClient->RemoveAllSelfAppVolumeChangeCallback(appUid);
    EXPECT_EQ(audioPolicyClient->selfAppVolumeChangeCallbackNum_[appUid], 0);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_050
* @tc.desc  : Test RemoveAllSelfAppVolumeChangeCallback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_050, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClient != nullptr);

    int32_t appUid0 = 0;
    int32_t appUid1 = 1;
    auto cb = std::make_shared<ConcreteAudioManagerAppVolumeChangeCallback>();
    audioPolicyClient->selfAppVolumeChangeCallback_.push_back({appUid0, cb});
    audioPolicyClient->selfAppVolumeChangeCallback_.push_back({appUid1, cb});
    EXPECT_NE(audioPolicyClient->selfAppVolumeChangeCallback_.size(), 0);

    audioPolicyClient->selfAppVolumeChangeCallbackNum_[appUid0] = 1;
    audioPolicyClient->RemoveAllSelfAppVolumeChangeCallback(appUid0);
    EXPECT_EQ(audioPolicyClient->selfAppVolumeChangeCallbackNum_[appUid0], 0);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_051
* @tc.desc  : Test RemoveSelfAppVolumeChangeCallback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_051, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClient != nullptr);

    int32_t appUid0 = 0;
    int32_t appUid1 = 1;
    auto cb = std::make_shared<ConcreteAudioManagerAppVolumeChangeCallback>();
    audioPolicyClient->selfAppVolumeChangeCallback_.push_back({appUid0, cb});

    auto ret = audioPolicyClient->RemoveSelfAppVolumeChangeCallback(appUid1, nullptr);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_052
* @tc.desc  : Test RemoveSelfAppVolumeChangeCallback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_052, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClient != nullptr);

    int32_t appUid0 = 0;
    int32_t appUid1 = 1;
    auto cb = std::make_shared<ConcreteAudioManagerAppVolumeChangeCallback>();
    audioPolicyClient->selfAppVolumeChangeCallback_.push_back({appUid0, cb});

    auto ret = audioPolicyClient->RemoveSelfAppVolumeChangeCallback(appUid1, cb);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_053
* @tc.desc  : Test RemoveSelfAppVolumeChangeCallback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_053, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClient != nullptr);

    int32_t appUid0 = 0;
    auto cb = std::make_shared<ConcreteAudioManagerAppVolumeChangeCallback>();
    audioPolicyClient->selfAppVolumeChangeCallback_.push_back({appUid0, cb});

    auto ret = audioPolicyClient->RemoveSelfAppVolumeChangeCallback(appUid0, nullptr);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_054
* @tc.desc  : Test RemoveSelfAppVolumeChangeCallback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_054, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClient != nullptr);

    int32_t appUid0 = 0;
    auto cb = std::make_shared<ConcreteAudioManagerAppVolumeChangeCallback>();
    audioPolicyClient->selfAppVolumeChangeCallback_.push_back({appUid0, cb});
    audioPolicyClient->selfAppVolumeChangeCallback_.push_back({appUid0, cb});

    audioPolicyClient->selfAppVolumeChangeCallbackNum_[appUid0] = 1;
    auto ret = audioPolicyClient->RemoveSelfAppVolumeChangeCallback(appUid0, nullptr);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_055
* @tc.desc  : Test RemoveAppVolumeChangeForUidCallback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_055, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClient != nullptr);

    int32_t appUid0 = 0;
    auto cb = std::make_shared<ConcreteAudioManagerAppVolumeChangeCallback>();
    audioPolicyClient->appVolumeChangeForUidCallback_.push_back({appUid0, cb});
    audioPolicyClient->appVolumeChangeForUidCallback_.push_back({appUid0, nullptr});

    auto ret = audioPolicyClient->RemoveAppVolumeChangeForUidCallback(cb);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_056
* @tc.desc  : Test AddAppVolumeChangeForUidCallback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_056, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClient != nullptr);

    int32_t appUid0 = 0;
    int32_t appUid1 = 1;
    auto cb = std::make_shared<ConcreteAudioManagerAppVolumeChangeCallback>();
    audioPolicyClient->appVolumeChangeForUidCallback_.push_back({appUid1, nullptr});
    audioPolicyClient->appVolumeChangeForUidCallback_.push_back({appUid0, nullptr});
    audioPolicyClient->appVolumeChangeForUidCallback_.push_back({appUid1, cb});
    audioPolicyClient->appVolumeChangeForUidCallback_.push_back({appUid0, cb});

    auto ret = audioPolicyClient->AddAppVolumeChangeForUidCallback(appUid0, cb);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_057
* @tc.desc  : Test OnAppVolumeChanged.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_057, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClient != nullptr);

    int32_t appUid0 = 0;
    int32_t appUid1 = 1;
    auto cb = std::make_shared<ConcreteAudioManagerAppVolumeChangeCallback>();
    audioPolicyClient->appVolumeChangeForUidCallback_.push_back({appUid0, cb});
    audioPolicyClient->appVolumeChangeForUidCallback_.push_back({appUid1, cb});

    VolumeEvent volumeEvent;
    audioPolicyClient->OnAppVolumeChanged(appUid0, volumeEvent);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_058
* @tc.desc  : Test OnAppVolumeChanged.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_058, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClient != nullptr);

    int32_t appUid0 = 0;
    int32_t appUid1 = 1;
    auto cb = std::make_shared<ConcreteAudioManagerAppVolumeChangeCallback>();
    audioPolicyClient->selfAppVolumeChangeCallback_.push_back({appUid0, cb});
    audioPolicyClient->selfAppVolumeChangeCallback_.push_back({appUid1, cb});

    VolumeEvent volumeEvent;
    audioPolicyClient->OnAppVolumeChanged(appUid0, volumeEvent);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_059
* @tc.desc  : Test AddSelfAppVolumeChangeCallback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_059, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClient != nullptr);

    int32_t appUid0 = 0;
    int32_t appUid1 = 1;
    auto cb = std::make_shared<ConcreteAudioManagerAppVolumeChangeCallback>();
    audioPolicyClient->selfAppVolumeChangeCallback_.push_back({appUid1, nullptr});
    audioPolicyClient->selfAppVolumeChangeCallback_.push_back({appUid0, nullptr});
    audioPolicyClient->selfAppVolumeChangeCallback_.push_back({appUid1, cb});
    audioPolicyClient->selfAppVolumeChangeCallback_.push_back({appUid0, cb});

    auto ret = audioPolicyClient->AddSelfAppVolumeChangeCallback(appUid0, cb);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_060
* @tc.desc  : Test RemoveAudioSceneChangedCallback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_060, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClient != nullptr);

    int32_t clientId = 0;
    auto cb = std::make_shared<ConcreteAudioManagerAudioSceneChangedCallback>();
    audioPolicyClient->AddAudioSceneChangedCallback(clientId, cb);
    audioPolicyClient->AddAudioSceneChangedCallback(clientId, nullptr);

    auto ret = audioPolicyClient->RemoveAudioSceneChangedCallback(cb);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_061
* @tc.desc  : Test OnSpatializationEnabledChangeForCurrentDevice.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_061, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteSpatialEnabledChangeForCurrentDeviceCb>();
    int32_t result = audioPolicyClient->AddSpatializationEnabledChangeForCurrentDeviceCallback(cb);
    EXPECT_EQ(result, SUCCESS);

    bool enabled = true;
    audioPolicyClient->OnSpatializationEnabledChangeForCurrentDevice(enabled);
    EXPECT_NE(audioPolicyClient, nullptr);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_062
* @tc.desc  : Test AddAudioFormatUnsupportedErrorCallback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_062, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteAudioFormatUnsupportedErrorCallback>();
    int32_t ret = audioPolicyClient->AddAudioFormatUnsupportedErrorCallback(cb);
    EXPECT_EQ(ret, SUCCESS);

    auto size = audioPolicyClient->GetAudioFormatUnsupportedErrorCallbackSize();
    EXPECT_EQ(size, 1);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_063
* @tc.desc  : Test RemoveAudioFormatUnsupportedErrorCallback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_063, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb1 = std::make_shared<ConcreteAudioFormatUnsupportedErrorCallback>();
    auto cb2 = std::make_shared<ConcreteAudioFormatUnsupportedErrorCallback>();
    int32_t ret = audioPolicyClient->AddAudioFormatUnsupportedErrorCallback(cb1);
    ret = audioPolicyClient->AddAudioFormatUnsupportedErrorCallback(cb2);
    EXPECT_EQ(ret, SUCCESS);

    ret = audioPolicyClient->RemoveAudioFormatUnsupportedErrorCallback();
    EXPECT_EQ(ret, SUCCESS);

    auto size = audioPolicyClient->GetAudioFormatUnsupportedErrorCallbackSize();
    EXPECT_EQ(size, 0);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_064
* @tc.desc  : Test OnFormatUnsupportedError.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_064, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteAudioFormatUnsupportedErrorCallback>();
    int32_t ret = audioPolicyClient->AddAudioFormatUnsupportedErrorCallback(cb);
    EXPECT_EQ(ret, SUCCESS);

    AudioErrors errorCode = ERROR_UNSUPPORTED_FORMAT;
    audioPolicyClient->OnFormatUnsupportedError(errorCode);
    EXPECT_NE(audioPolicyClient, nullptr);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_0065
* @tc.desc  : Test RemoveVolumeKeyEventCallback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_0065, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto mockCallback = std::make_shared<ConcreteVolumeKeyEventCallback>();
    EXPECT_EQ(audioPolicyClient->RemoveVolumeKeyEventCallback(mockCallback), SUCCESS);
    EXPECT_EQ(audioPolicyClient->volumeKeyEventCallbackList_.size(), 0);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_0066
* @tc.desc  : Test AddSystemVolumeChangeCallback/RemoveSystemVolumeChangeCallback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_0066, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto mockCallback = std::make_shared<ConcreteSystemVolumeChangeCallback>();
    EXPECT_EQ(audioPolicyClient->RemoveSystemVolumeChangeCallback(mockCallback), SUCCESS);
    EXPECT_EQ(audioPolicyClient->systemVolumeChangeCallbackList_.size(), 0);

    EXPECT_EQ(audioPolicyClient->AddSystemVolumeChangeCallback(mockCallback), SUCCESS);
    EXPECT_EQ(audioPolicyClient->systemVolumeChangeCallbackList_.size(), 1);
    EXPECT_EQ(audioPolicyClient->RemoveSystemVolumeChangeCallback(mockCallback), SUCCESS);
    EXPECT_EQ(audioPolicyClient->systemVolumeChangeCallbackList_.size(), 0);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_067
* @tc.desc  : Test OnSystemVolumeChange.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_067, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto mockCallback = std::make_shared<ConcreteSystemVolumeChangeCallback>();
    EXPECT_EQ(audioPolicyClient->AddSystemVolumeChangeCallback(mockCallback), SUCCESS);
    EXPECT_EQ(audioPolicyClient->OnSystemVolumeChange(VolumeEvent()), SUCCESS);

    mockCallback.reset();
    EXPECT_EQ(audioPolicyClient->OnSystemVolumeChange(VolumeEvent()), SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_068
* @tc.desc  : Test DeviceFilterByFlag.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_068, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescs;
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->networkId_ = REMOTE_NETWORK_ID;
    deviceDescs.push_back(deviceDesc);
    auto descRet = audioPolicyClient->DeviceFilterByFlag(DeviceFlag::ALL_DEVICES_FLAG, deviceDescs);
    EXPECT_TRUE(descRet.empty());
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_069
* @tc.desc  : Test DeviceFilterByFlag.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_069, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescs;
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->networkId_ = REMOTE_NETWORK_ID;
    deviceDescs.push_back(deviceDesc);
    auto descRet = audioPolicyClient->DeviceFilterByFlag(DeviceFlag::ALL_DISTRIBUTED_DEVICES_FLAG, deviceDescs);
    EXPECT_EQ(descRet.size(), 1);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_070
* @tc.desc  : Test DeviceFilterByFlag.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_070, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescs;
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->networkId_ = LOCAL_NETWORK_ID;
    deviceDesc->deviceRole_ = INPUT_DEVICE;
    deviceDescs.push_back(deviceDesc);
    auto descRet = audioPolicyClient->DeviceFilterByFlag(DeviceFlag::INPUT_DEVICES_FLAG, deviceDescs);
    EXPECT_EQ(descRet.size(), 1);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_071
* @tc.desc  : Test AddDeviceChangeCallback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_071, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    std::shared_ptr<AudioManagerDeviceChangeCallback> mockCallback =
        std::make_shared<ConcreteAudioManagerDeviceChangeCallback>();
    EXPECT_EQ(audioPolicyClient->AddDeviceChangeCallback(DeviceFlag::ALL_DEVICES_FLAG, mockCallback), SUCCESS);
    EXPECT_EQ(audioPolicyClient->RemoveDeviceChangeCallback(DeviceFlag::NONE_DEVICES_FLAG, mockCallback), SUCCESS);
    EXPECT_EQ(audioPolicyClient->deviceChangeCallbackList_.size(), 1);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_072
* @tc.desc  : Test OnDeviceChange.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_072, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto mockCallback = std::make_shared<ConcreteAudioManagerDeviceChangeCallback>();
    audioPolicyClient->AddDeviceChangeCallback(DeviceFlag::ALL_DEVICES_FLAG, mockCallback);

    DeviceChangeAction dca;
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    dca.deviceDescriptors.push_back(deviceDesc);
    EXPECT_EQ(audioPolicyClient->OnDeviceChange(dca), SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_073
* @tc.desc  : Test AddActiveVolumeTypeChangeCallback/RemoveActiveVolumeTypeChangeCallback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_073, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto mockCallback1 = std::make_shared<ConcreteAudioManagerActiveVolumeTypeChangeCallback>();
    auto mockCallback2 = std::make_shared<ConcreteAudioManagerActiveVolumeTypeChangeCallback>();
    auto mockCallback3 = std::make_shared<ConcreteAudioManagerActiveVolumeTypeChangeCallback>();
    audioPolicyClient->AddActiveVolumeTypeChangeCallback(mockCallback1);
    audioPolicyClient->AddActiveVolumeTypeChangeCallback(mockCallback1);
    audioPolicyClient->AddActiveVolumeTypeChangeCallback(mockCallback2);

    audioPolicyClient->RemoveActiveVolumeTypeChangeCallback(nullptr);
    audioPolicyClient->RemoveActiveVolumeTypeChangeCallback(mockCallback1);
    audioPolicyClient->RemoveActiveVolumeTypeChangeCallback(mockCallback3);

    audioPolicyClient->RemoveAllActiveVolumeTypeChangeCallback();
    EXPECT_TRUE(audioPolicyClient->activeVolumeTypeChangeCallbackList_.empty());
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_074
* @tc.desc  : Test RemoveSelfAppVolumeChangeCallback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_074, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    int32_t appUid = 1;
    auto mockCallback = std::make_shared<ConcreteAudioManagerAppVolumeChangeCallback>();
    EXPECT_EQ(audioPolicyClient->AddSelfAppVolumeChangeCallback(appUid, mockCallback), SUCCESS);
    EXPECT_EQ(audioPolicyClient->selfAppVolumeChangeCallback_.size(), 1);

    EXPECT_EQ(audioPolicyClient->RemoveSelfAppVolumeChangeCallback(appUid, mockCallback), SUCCESS);
    EXPECT_EQ(audioPolicyClient->selfAppVolumeChangeCallback_.size(), 0);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_075
* @tc.desc  : Test OnActiveVolumeTypeChanged.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_075, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto mockCallback = std::make_shared<ConcreteAudioManagerActiveVolumeTypeChangeCallback>();
    audioPolicyClient->AddActiveVolumeTypeChangeCallback(mockCallback);

    int32_t volumeType = 1;
    EXPECT_EQ(audioPolicyClient->OnActiveVolumeTypeChanged(volumeType), SUCCESS);
    audioPolicyClient->activeVolumeTypeChangeCallbackList_.front().reset();
    EXPECT_EQ(audioPolicyClient->OnActiveVolumeTypeChanged(volumeType), SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_076
* @tc.desc  : Test OnActiveVolumeTypeChanged.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_076, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto mockCallback1 = std::make_shared<ConcreteAudioSessionStateChangedCallback>();
    auto mockCallback2 = std::make_shared<ConcreteAudioSessionStateChangedCallback>();
    audioPolicyClient->AddAudioSessionStateCallback(mockCallback1);

    audioPolicyClient->RemoveAudioSessionStateCallback(mockCallback2);
    EXPECT_EQ(audioPolicyClient->GetAudioSessionStateCallbackSize(), 1);
    audioPolicyClient->RemoveAudioSessionStateCallback(mockCallback1);
    EXPECT_EQ(audioPolicyClient->GetAudioSessionStateCallbackSize(), 0);

    int32_t stateChangeHint = 1;
    audioPolicyClient->AddAudioSessionStateCallback(mockCallback1);
    EXPECT_EQ(audioPolicyClient->OnAudioSessionStateChanged(stateChangeHint), SUCCESS);
    audioPolicyClient->audioSessionStateCallbackList_.front().reset();
    EXPECT_EQ(audioPolicyClient->OnAudioSessionStateChanged(stateChangeHint), SUCCESS);

    audioPolicyClient->RemoveAudioSessionStateCallback();
    EXPECT_TRUE(audioPolicyClient->audioSessionStateCallbackList_.empty());
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_077
* @tc.desc  : Test OnAudioSessionCurrentDeviceChanged.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_077, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto mockCallback1 = std::make_shared<ConcreteAudioSessionCurrentDeviceChangedCallback>();
    auto mockCallback2 = std::make_shared<ConcreteAudioSessionCurrentDeviceChangedCallback>();
    audioPolicyClient->AddAudioSessionDeviceCallback(mockCallback1);

    audioPolicyClient->RemoveAudioSessionDeviceCallback(mockCallback2);
    EXPECT_EQ(audioPolicyClient->GetAudioSessionDeviceCallbackSize(), 1);
    audioPolicyClient->RemoveAudioSessionDeviceCallback(mockCallback1);
    EXPECT_EQ(audioPolicyClient->GetAudioSessionDeviceCallbackSize(), 0);

    CurrentOutputDeviceChangedEvent deviceChangedEvent;
    audioPolicyClient->AddAudioSessionDeviceCallback(mockCallback1);
    EXPECT_EQ(audioPolicyClient->OnAudioSessionCurrentDeviceChanged(deviceChangedEvent), SUCCESS);
    audioPolicyClient->audioSessionDeviceCallbackList_.front().reset();
    EXPECT_EQ(audioPolicyClient->OnAudioSessionCurrentDeviceChanged(deviceChangedEvent), SUCCESS);

    audioPolicyClient->RemoveAudioSessionDeviceCallback();
    EXPECT_TRUE(audioPolicyClient->audioSessionDeviceCallbackList_.empty());
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_078
* @tc.desc  : Test RemovePreferredOutputDeviceChangeCallback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_078, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    AudioRendererInfo rendererInfo;
    auto mockCallback1 = std::make_shared<ConcreteAudioPreferredOutputDeviceChangeCallback>();
    auto mockCallback2 = std::make_shared<ConcreteAudioPreferredOutputDeviceChangeCallback>();
    audioPolicyClient->AddPreferredOutputDeviceChangeCallback(rendererInfo, mockCallback1);
    audioPolicyClient->RemovePreferredOutputDeviceChangeCallback(mockCallback2);
    EXPECT_EQ(audioPolicyClient->GetPreferredOutputDeviceChangeCallbackSize(), 1);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_079
* @tc.desc  : Test RemovePreferredInputDeviceChangeCallback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_079, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    AudioCapturerInfo capturerInfo;
    auto mockCallback1 = std::make_shared<ConcreteAudioPreferredInputDeviceChangeCallback>();
    auto mockCallback2 = std::make_shared<ConcreteAudioPreferredInputDeviceChangeCallback>();
    audioPolicyClient->AddPreferredInputDeviceChangeCallback(capturerInfo, mockCallback1);
    audioPolicyClient->RemovePreferredInputDeviceChangeCallback(mockCallback2);
    EXPECT_EQ(audioPolicyClient->GetPreferredInputDeviceChangeCallbackSize(), 1);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_080
* @tc.desc  : Test OnRendererDeviceChange.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_080, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    uint32_t sessionId = 1;
    auto mockCallback = std::make_shared<ConcreteDeviceChangeWithInfoCallback>();
    audioPolicyClient->AddDeviceChangeWithInfoCallback(sessionId, mockCallback);

    mockCallback.reset();
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    AudioStreamDeviceChangeReasonExt reason(AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE);
    EXPECT_EQ(audioPolicyClient->OnRendererDeviceChange(sessionId, deviceInfo, reason), ERR_CALLBACK_NOT_REGISTERED);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_081
* @tc.desc  : Test OnRecreateCapturerStreamEvent.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_081, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    uint32_t sessionId = 1;
    auto mockCallback = std::make_shared<ConcreteDeviceChangeWithInfoCallback>();
    audioPolicyClient->AddDeviceChangeWithInfoCallback(sessionId, mockCallback);

    int32_t streamFlag = 1;
    AudioStreamDeviceChangeReasonExt reason(AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE);
    mockCallback.reset();
    EXPECT_EQ(audioPolicyClient->OnRecreateRendererStreamEvent(sessionId, streamFlag, reason), SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_082
* @tc.desc  : Test OnStreamVolumeChange.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_082, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    std::set<StreamUsage> streamUsages = {
        StreamUsage::STREAM_USAGE_MEDIA
    };
    auto mockCallback1 = std::make_shared<ConcreteStreamVolumeChangeCallback>();
    auto mockCallback2 = std::make_shared<ConcreteStreamVolumeChangeCallback>();
    audioPolicyClient->AddStreamVolumeChangeCallback(streamUsages, mockCallback1);

    audioPolicyClient->RemoveStreamVolumeChangeCallback(mockCallback2);
    EXPECT_EQ(audioPolicyClient->GetStreamVolumeChangeCallbackSize(), 1);

    StreamVolumeEvent streamVolumeEvent;
    EXPECT_EQ(audioPolicyClient->OnStreamVolumeChange(streamVolumeEvent), SUCCESS);
    streamVolumeEvent.streamUsage = STREAM_USAGE_MEDIA;
    EXPECT_EQ(audioPolicyClient->OnStreamVolumeChange(streamVolumeEvent), SUCCESS);
    mockCallback1.reset();
    EXPECT_EQ(audioPolicyClient->OnStreamVolumeChange(streamVolumeEvent), SUCCESS);
    streamVolumeEvent.streamUsage = STREAM_USAGE_INVALID;
    EXPECT_EQ(audioPolicyClient->OnStreamVolumeChange(streamVolumeEvent), SUCCESS);
}
} // namespace AudioStandard
} // namespace OHOS
