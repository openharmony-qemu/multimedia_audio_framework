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

constexpr int32_t TEST_UID = 1001;

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
    int32_t result = audioPolicyClient->AddPreferredOutputDeviceChangeCallback(rendererInfo, TEST_UID, cb);
    EXPECT_EQ(result, SUCCESS);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc;
    audioPolicyClient->OnPreferredOutputDeviceUpdated(rendererInfo, TEST_UID, desc);
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
    int32_t result = audioPolicyClient->AddPreferredInputDeviceChangeCallback(capturerInfo, TEST_UID, cb);
    EXPECT_EQ(result, SUCCESS);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc;
    audioPolicyClient->OnPreferredInputDeviceUpdated(capturerInfo, TEST_UID, desc);
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
    auto cb2 = std::make_shared<ConcreteAudioRendererStateChangeCallback>();
    EXPECT_NE(cb->GetCallbackId(), 0);
    EXPECT_NE(cb2->GetCallbackId(), 0);
    EXPECT_NE(cb->GetCallbackId(), cb2->GetCallbackId());

    int32_t result = audioPolicyClient->AddRendererStateChangeCallback(cb);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetRendererStateChangeCallbackSize(), 1);

    result = audioPolicyClient->AddRendererStateChangeCallback(cb2);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetRendererStateChangeCallbackSize(), 2);

    result = audioPolicyClient->RemoveRendererStateChangeCallback(cb);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetRendererStateChangeCallbackSize(), 1);

    std::shared_ptr<AudioRendererStateChangeCallback> nullCb = nullptr;
    result = audioPolicyClient->AddRendererStateChangeCallback(nullCb);
    EXPECT_EQ(result, ERR_INVALID_PARAM);

    std::vector<std::shared_ptr<AudioRendererStateChangeCallback>> callbacks = { cb2 };
    result = audioPolicyClient->RemoveRendererStateChangeCallback(callbacks);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetRendererStateChangeCallbackSize(), 0);
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
    AudioDeviceDescriptor preDeviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    AudioStreamDeviceChangeReasonExt reason(AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE);
    audioPolicyClient->OnRendererDeviceChange(0, deviceInfo, reason, preDeviceInfo);
    EXPECT_NE(audioPolicyClient, nullptr);

    audioPolicyClient->OnRendererDeviceChange(1, deviceInfo, reason, preDeviceInfo);
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
    deviceDescriptor.deviceType_ = DEVICE_TYPE_WIRED_HEADSET;
    EXPECT_EQ(deviceDescriptor.MapInternalToExternalDeviceType(API_VERSION_MAX), DEVICE_TYPE_WIRED_HEADSET);
    deviceDescriptor.deviceType_ = DEVICE_TYPE_WIRED_HEADPHONES;
    EXPECT_EQ(deviceDescriptor.MapInternalToExternalDeviceType(API_VERSION_MAX), DEVICE_TYPE_WIRED_HEADPHONES);
    deviceDescriptor.deviceType_ = DEVICE_TYPE_LINE_DIGITAL;
    EXPECT_EQ(deviceDescriptor.MapInternalToExternalDeviceType(API_VERSION_MAX), DEVICE_TYPE_LINE_DIGITAL);
    deviceDescriptor.deviceType_ = DEVICE_TYPE_WIRED_MICIN;
    EXPECT_EQ(deviceDescriptor.MapInternalToExternalDeviceType(API_VERSION_MAX), DEVICE_TYPE_WIRED_HEADSET);
    deviceDescriptor.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP_IN;
    EXPECT_EQ(deviceDescriptor.MapInternalToExternalDeviceType(API_VERSION_MAX), DEVICE_TYPE_BLUETOOTH_A2DP);
    deviceDescriptor.deviceType_ = DEVICE_TYPE_SPEAKER;
    EXPECT_EQ(deviceDescriptor.MapInternalToExternalDeviceType(API_VERSION_MAX), DEVICE_TYPE_SPEAKER);
    deviceDescriptor.deviceType_ = DEVICE_TYPE_NEARLINK;
    EXPECT_EQ(deviceDescriptor.MapInternalToExternalDeviceType(API_VERSION_MAX), DEVICE_TYPE_NEARLINK);
    int32_t apiVersion = 0;
    EXPECT_EQ(deviceDescriptor.MapInternalToExternalDeviceType(apiVersion), DEVICE_TYPE_BLUETOOTH_SCO);
    bool isSupportNearlink = false;
    EXPECT_EQ(deviceDescriptor.MapInternalToExternalDeviceType(apiVersion, isSupportNearlink),
        DEVICE_TYPE_BLUETOOTH_SCO);
    EXPECT_EQ(deviceDescriptor.MapInternalToExternalDeviceType(API_VERSION_MAX, isSupportNearlink),
        DEVICE_TYPE_BLUETOOTH_SCO);
}

/**
* @tc.name  : Test AudioDeviceDescriptor.
* @tc.number: AudioDeviceDescriptor_002
* @tc.desc  : Test AudioDeviceDescriptor::BuildCapabilitiesFromDeviceStreamInfo().
*/
HWTEST(AudioPolicyClientStubImplTest, AudioDeviceDescriptor_002, TestSize.Level1)
{
    AudioDeviceDescriptor deviceDesc;
    AudioStreamInfo audioStreamInfo;
    audioStreamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_96000;
    audioStreamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    audioStreamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    audioStreamInfo.channels = AudioChannel::STEREO;
    audioStreamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    DeviceStreamInfo deviceStreamInfo(audioStreamInfo);
    deviceDesc.audioStreamInfo_.push_back(deviceStreamInfo);

    // Test1
    deviceDesc.deviceType_ = DEVICE_TYPE_SPEAKER;
    deviceDesc.networkId_ = REMOTE_NETWORK_ID;
    deviceDesc.BuildCapabilitiesFromDeviceStreamInfo();
    EXPECT_EQ(deviceDesc.capabilities_.begin()->samplingRate, AudioSamplingRate::SAMPLE_RATE_96000);

    // Test2
    deviceDesc.deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    deviceDesc.BuildCapabilitiesFromDeviceStreamInfo();
    EXPECT_EQ(deviceDesc.capabilities_.begin()->samplingRate, AudioSamplingRate::SAMPLE_RATE_16000);
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
    audioPolicyClient->AddPreferredOutputDeviceChangeCallback(rendererInfo, TEST_UID, cb);
    auto ret = audioPolicyClient->RemovePreferredOutputDeviceChangeCallback(TEST_UID, rendererInfo.streamUsage, cb);
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
    audioPolicyClient->AddPreferredInputDeviceChangeCallback(capturerInfo, TEST_UID, cb);
    auto ret = audioPolicyClient->RemovePreferredInputDeviceChangeCallback(TEST_UID, capturerInfo.sourceType, cb);
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
    AudioDeviceDescriptor preDeviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    AudioStreamDeviceChangeReasonExt reason(AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE);
    audioPolicyClient->OnRendererDeviceChange(sessionId, deviceInfo, reason, preDeviceInfo);
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
* @tc.desc  : Test OnSelfAppVolumeChanged.
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
    audioPolicyClient->OnSelfAppVolumeChanged(appUid0, volumeEvent);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_058
* @tc.desc  : Test OnSelfAppVolumeChanged.
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
    audioPolicyClient->OnSelfAppVolumeChanged(appUid0, volumeEvent);
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
    audioPolicyClient->AddPreferredOutputDeviceChangeCallback(rendererInfo, TEST_UID, mockCallback1);
    audioPolicyClient->RemovePreferredOutputDeviceChangeCallback(TEST_UID, rendererInfo.streamUsage, mockCallback2);
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
    audioPolicyClient->AddPreferredInputDeviceChangeCallback(capturerInfo, TEST_UID, mockCallback1);
    audioPolicyClient->RemovePreferredInputDeviceChangeCallback(TEST_UID, capturerInfo.sourceType, mockCallback2);
    EXPECT_EQ(audioPolicyClient->GetPreferredInputDeviceChangeCallbackSize(), 1);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImplDegreeTest_001
* @tc.desc  : Test AddVolumeDegreeCallback/RemoveVolumeDegreeCallback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImplDegreeTest_001, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto mockCallback0 = std::make_shared<ConcreteVolumeKeyEventCallback>();
    auto mockCallback2 = std::make_shared<ConcreteVolumeKeyEventCallback>();
    int32_t result = audioPolicyClient->AddVolumeDegreeCallback(mockCallback0);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(audioPolicyClient->volumeDegreeCallbackList_.size(), 1);
    EXPECT_EQ(audioPolicyClient->RemoveVolumeDegreeCallback(mockCallback2), SUCCESS);

    auto mockCallback1 = std::make_shared<ConcreteVolumeKeyEventCallback>();
    EXPECT_EQ(audioPolicyClient->AddVolumeDegreeCallback(mockCallback1), SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetVolumeDegreeCallbackSize(), 2);
    EXPECT_EQ(audioPolicyClient->OnVolumeDegreeEvent({}), SUCCESS);
    EXPECT_EQ(audioPolicyClient->RemoveVolumeDegreeCallback(mockCallback0), SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetVolumeDegreeCallbackSize(), 1);
    EXPECT_EQ(audioPolicyClient->RemoveVolumeDegreeCallback(nullptr), SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetVolumeDegreeCallbackSize(), 0);

    audioPolicyClient->AddVolumeDegreeCallback(nullptr);
    EXPECT_EQ(audioPolicyClient->OnVolumeDegreeEvent({}), SUCCESS);
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
    AudioDeviceDescriptor preDeviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    AudioStreamDeviceChangeReasonExt reason(AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE);
    EXPECT_EQ(audioPolicyClient->OnRendererDeviceChange(sessionId, deviceInfo, reason, preDeviceInfo),
        ERR_CALLBACK_NOT_REGISTERED);
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

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_083
* @tc.desc  : Test CollaborationEnabledChangeForCurrentDeviceCallback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_083, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();

    EXPECT_EQ(audioPolicyClient->RemoveCollaborationEnabledChangeForCurrentDeviceCallback(), SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetCollaborationEnabledChangeForCurrentDeviceCallbackSize(), 0);
    auto testCallback = std::make_shared<AudioCollaborationEnabledChangeForCurrentDeviceCallback>();
    EXPECT_EQ(audioPolicyClient->AddCollaborationEnabledChangeForCurrentDeviceCallback(testCallback), SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetCollaborationEnabledChangeForCurrentDeviceCallbackSize(), 1);
    EXPECT_EQ(audioPolicyClient->RemoveCollaborationEnabledChangeForCurrentDeviceCallback(), SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetCollaborationEnabledChangeForCurrentDeviceCallbackSize(), 0);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_084
* @tc.desc  : Test OnCollaborationEnabledChangeForCurrentDevice.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_084, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();

    EXPECT_EQ(audioPolicyClient->RemoveCollaborationEnabledChangeForCurrentDeviceCallback(), SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetCollaborationEnabledChangeForCurrentDeviceCallbackSize(), 0);
    auto testCallbackFirst = std::make_shared<AudioCollaborationEnabledChangeForCurrentDeviceCallback>();
    auto testCallbackSecond = std::make_shared<AudioCollaborationEnabledChangeForCurrentDeviceCallback>();
    EXPECT_EQ(audioPolicyClient->AddCollaborationEnabledChangeForCurrentDeviceCallback(testCallbackFirst), SUCCESS);
    EXPECT_EQ(audioPolicyClient->AddCollaborationEnabledChangeForCurrentDeviceCallback(testCallbackSecond), SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetCollaborationEnabledChangeForCurrentDeviceCallbackSize(), 1 + 1);

    bool testEnabled = true;
    EXPECT_EQ(audioPolicyClient->OnCollaborationEnabledChangeForCurrentDevice(testEnabled), SUCCESS);
    testEnabled = false;
    EXPECT_EQ(audioPolicyClient->OnCollaborationEnabledChangeForCurrentDevice(testEnabled), SUCCESS);
    EXPECT_EQ(audioPolicyClient->RemoveCollaborationEnabledChangeForCurrentDeviceCallback(), SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetCollaborationEnabledChangeForCurrentDeviceCallbackSize(), 0);
}


/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_085
* @tc.desc  : Test onAdaptiveSpatialRenderingEnabledChangeForAnyDevice.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_085, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteAudioAdaptiveSpatialRenderingEnabledChangeCallback>();
    int32_t result = audioPolicyClient->AddAdaptiveSpatialRenderingEnabledChangeCallback(cb);
    EXPECT_EQ(result, SUCCESS);

    bool enabled = true;

    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioPolicyClient->OnAdaptiveSpatialRenderingEnabledChangeForAnyDevice(deviceDescriptor, enabled);
    EXPECT_NE(audioPolicyClient, nullptr);
}


/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_086
* @tc.desc  : Test onAdaptiveSpatialRenderingEnabledChangeForAnyDevice.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_086, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteAudioAdaptiveSpatialRenderingEnabledChangeCallback>();
    int32_t result = audioPolicyClient->AddAdaptiveSpatialRenderingEnabledChangeCallback(cb);
    EXPECT_EQ(result, SUCCESS);

    bool enabled = true;

    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioPolicyClient->OnAdaptiveSpatialRenderingEnabledChangeForAnyDevice(deviceDescriptor, enabled);
    EXPECT_NE(audioPolicyClient, nullptr);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_087
* @tc.desc  : Test OnPersonalizedSpatializationEnabledChangeForAnyDevice.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_087, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteAudioPersonalizedSpatializationEnabledChangeCallback>();
    int32_t result = audioPolicyClient->AddPersonalizedSpatializationEnabledChangeCallback(cb);
    EXPECT_EQ(result, SUCCESS);

    bool enabled = true;

    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioPolicyClient->OnPersonalizedSpatializationEnabledChangeForAnyDevice(deviceDescriptor, enabled);
    EXPECT_NE(audioPolicyClient, nullptr);
}


/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_088
* @tc.desc  : Test OnPersonalizedSpatializationEnabledChangeForAnyDevice.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_088, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteAudioPersonalizedSpatializationEnabledChangeCallback>();
    int32_t result = audioPolicyClient->AddPersonalizedSpatializationEnabledChangeCallback(cb);
    EXPECT_EQ(result, SUCCESS);

    bool enabled = false;

    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioPolicyClient->OnPersonalizedSpatializationEnabledChangeForAnyDevice(deviceDescriptor, enabled);
    EXPECT_NE(audioPolicyClient, nullptr);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_MultiCallback_001
* @tc.desc  : Test adding multiple output callbacks for same uid and streamUsage.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_MultiCallback_001, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClient != nullptr);

    AudioRendererInfo rendererInfo;
    rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    int32_t uid = TEST_UID;

    auto cb1 = std::make_shared<ConcreteAudioPreferredOutputDeviceChangeCallback>();
    auto cb2 = std::make_shared<ConcreteAudioPreferredOutputDeviceChangeCallback>();
    auto cb3 = std::make_shared<ConcreteAudioPreferredOutputDeviceChangeCallback>();

    // Add 3 callbacks
    EXPECT_EQ(audioPolicyClient->AddPreferredOutputDeviceChangeCallback(rendererInfo, uid, cb1), SUCCESS);
    EXPECT_EQ(audioPolicyClient->AddPreferredOutputDeviceChangeCallback(rendererInfo, uid, cb2), SUCCESS);
    EXPECT_EQ(audioPolicyClient->AddPreferredOutputDeviceChangeCallback(rendererInfo, uid, cb3), SUCCESS);

    // Verify callback count
    EXPECT_EQ(audioPolicyClient->GetPreferredOutputDeviceChangeCallbackSize(), 3);

    // Duplicate add should not increase count
    EXPECT_EQ(audioPolicyClient->AddPreferredOutputDeviceChangeCallback(rendererInfo, uid, cb1), SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetPreferredOutputDeviceChangeCallbackSize(), 3);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_MultiCallback_002
* @tc.desc  : Test removing specific output callback from vector.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_MultiCallback_002, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClient != nullptr);

    AudioRendererInfo rendererInfo;
    rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    int32_t uid = TEST_UID;

    auto cb1 = std::make_shared<ConcreteAudioPreferredOutputDeviceChangeCallback>();
    auto cb2 = std::make_shared<ConcreteAudioPreferredOutputDeviceChangeCallback>();
    auto cb3 = std::make_shared<ConcreteAudioPreferredOutputDeviceChangeCallback>();

    // Add 3 callbacks
    audioPolicyClient->AddPreferredOutputDeviceChangeCallback(rendererInfo, uid, cb1);
    audioPolicyClient->AddPreferredOutputDeviceChangeCallback(rendererInfo, uid, cb2);
    audioPolicyClient->AddPreferredOutputDeviceChangeCallback(rendererInfo, uid, cb3);
    EXPECT_EQ(audioPolicyClient->GetPreferredOutputDeviceChangeCallbackSize(), 3);

    // Remove cb2
    EXPECT_EQ(audioPolicyClient->RemovePreferredOutputDeviceChangeCallback(uid, rendererInfo.streamUsage, cb2),
        SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetPreferredOutputDeviceChangeCallbackSize(), 2);

    // Remove cb1
    EXPECT_EQ(audioPolicyClient->RemovePreferredOutputDeviceChangeCallback(uid, rendererInfo.streamUsage, cb1),
        SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetPreferredOutputDeviceChangeCallbackSize(), 1);

    // Remove cb3
    EXPECT_EQ(audioPolicyClient->RemovePreferredOutputDeviceChangeCallback(uid, rendererInfo.streamUsage, cb3),
        SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetPreferredOutputDeviceChangeCallbackSize(), 0);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_MultiCallback_003
* @tc.desc  : Test invoking all registered output callbacks.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_MultiCallback_003, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClient != nullptr);

    AudioRendererInfo rendererInfo;
    rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    int32_t uid = TEST_UID;

    auto cb1 = std::make_shared<ConcreteAudioPreferredOutputDeviceChangeCallback>();
    auto cb2 = std::make_shared<ConcreteAudioPreferredOutputDeviceChangeCallback>();
    auto cb3 = std::make_shared<ConcreteAudioPreferredOutputDeviceChangeCallback>();

    // Add 3 callbacks
    audioPolicyClient->AddPreferredOutputDeviceChangeCallback(rendererInfo, uid, cb1);
    audioPolicyClient->AddPreferredOutputDeviceChangeCallback(rendererInfo, uid, cb2);
    audioPolicyClient->AddPreferredOutputDeviceChangeCallback(rendererInfo, uid, cb3);

    // Invoke callbacks
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc;
    EXPECT_EQ(audioPolicyClient->OnPreferredOutputDeviceUpdated(rendererInfo, uid, desc), SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_MultiCallback_004
* @tc.desc  : Test adding multiple input callbacks for same uid and sourceType.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_MultiCallback_004, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClient != nullptr);

    AudioCapturerInfo capturerInfo;
    capturerInfo.sourceType = SOURCE_TYPE_MIC;
    int32_t uid = TEST_UID;

    auto cb1 = std::make_shared<ConcreteAudioPreferredInputDeviceChangeCallback>();
    auto cb2 = std::make_shared<ConcreteAudioPreferredInputDeviceChangeCallback>();

    // Add 2 callbacks
    EXPECT_EQ(audioPolicyClient->AddPreferredInputDeviceChangeCallback(capturerInfo, uid, cb1), SUCCESS);
    EXPECT_EQ(audioPolicyClient->AddPreferredInputDeviceChangeCallback(capturerInfo, uid, cb2), SUCCESS);

    // Verify callback count
    EXPECT_EQ(audioPolicyClient->GetPreferredInputDeviceChangeCallbackSize(), 2);

    // Duplicate add should not increase count
    EXPECT_EQ(audioPolicyClient->AddPreferredInputDeviceChangeCallback(capturerInfo, uid, cb1), SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetPreferredInputDeviceChangeCallbackSize(), 2);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_MultiCallback_005
* @tc.desc  : Test removing specific input callback from vector.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_MultiCallback_005, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClient != nullptr);

    AudioCapturerInfo capturerInfo;
    capturerInfo.sourceType = SOURCE_TYPE_MIC;
    int32_t uid = TEST_UID;

    auto cb1 = std::make_shared<ConcreteAudioPreferredInputDeviceChangeCallback>();
    auto cb2 = std::make_shared<ConcreteAudioPreferredInputDeviceChangeCallback>();

    // Add 2 callbacks
    audioPolicyClient->AddPreferredInputDeviceChangeCallback(capturerInfo, uid, cb1);
    audioPolicyClient->AddPreferredInputDeviceChangeCallback(capturerInfo, uid, cb2);
    EXPECT_EQ(audioPolicyClient->GetPreferredInputDeviceChangeCallbackSize(), 2);

    // Remove cb1
    EXPECT_EQ(audioPolicyClient->RemovePreferredInputDeviceChangeCallback(uid, capturerInfo.sourceType, cb1), SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetPreferredInputDeviceChangeCallbackSize(), 1);

    // Remove cb2
    EXPECT_EQ(audioPolicyClient->RemovePreferredInputDeviceChangeCallback(uid, capturerInfo.sourceType, cb2), SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetPreferredInputDeviceChangeCallbackSize(), 0);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_MultiCallback_006
* @tc.desc  : Test nullptr callback handling.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_MultiCallback_006, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClient != nullptr);

    AudioRendererInfo rendererInfo;
    int32_t uid = TEST_UID;

    // nullptr callback should return error
    EXPECT_EQ(audioPolicyClient->AddPreferredOutputDeviceChangeCallback(rendererInfo, uid, nullptr), ERR_INVALID_PARAM);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_MultiCallback_007
* @tc.desc  : Test removing all callbacks with nullptr.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_MultiCallback_007, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClient != nullptr);

    AudioRendererInfo rendererInfo;
    int32_t uid = TEST_UID;

    auto cb1 = std::make_shared<ConcreteAudioPreferredOutputDeviceChangeCallback>();
    auto cb2 = std::make_shared<ConcreteAudioPreferredOutputDeviceChangeCallback>();

    // Add 2 callbacks
    audioPolicyClient->AddPreferredOutputDeviceChangeCallback(rendererInfo, uid, cb1);
    audioPolicyClient->AddPreferredOutputDeviceChangeCallback(rendererInfo, uid, cb2);
    EXPECT_EQ(audioPolicyClient->GetPreferredOutputDeviceChangeCallbackSize(), 2);

    // Remove all callbacks for uid with nullptr
    EXPECT_EQ(audioPolicyClient->RemovePreferredOutputDeviceChangeCallback(uid, rendererInfo.streamUsage, nullptr),
        SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetPreferredOutputDeviceChangeCallbackSize(), 0);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_089
* @tc.desc  : Test AddRecordControllerEnabledChangeCallback/RemoveRecordControllerEnabledChangeCallback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_089, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    std::shared_ptr<ConcreteSystemRecordControllerEnabledChangeCallback> cb0 =
        std::make_shared<ConcreteSystemRecordControllerEnabledChangeCallback>();
    std::shared_ptr<ConcreteSystemRecordControllerEnabledChangeCallback> cb1 =
        std::make_shared<ConcreteSystemRecordControllerEnabledChangeCallback>();

    EXPECT_FALSE(audioPolicyClient->HasRecordControllerEnabledChangeCallback());
    EXPECT_EQ(audioPolicyClient->AddRecordControllerEnabledChangeCallback(cb0), SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetRecordControllerEnabledChangeCallbackSize(), 1);
    EXPECT_EQ(audioPolicyClient->AddRecordControllerEnabledChangeCallback(cb1), SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetRecordControllerEnabledChangeCallbackSize(), 2);
    EXPECT_TRUE(audioPolicyClient->HasRecordControllerEnabledChangeCallback());

    SystemRecordControllerChangeInfo changeInfo;
    changeInfo.enabled = true;
    changeInfo.uid = TEST_UID;
    changeInfo.sourceType = SOURCE_TYPE_MIC;
    audioPolicyClient->OnRecordControllerEnabledChange(changeInfo);
    EXPECT_TRUE(cb0->isCallbackInvoked_);
    EXPECT_TRUE(cb0->lastChangeInfo_.enabled);
    EXPECT_EQ(cb0->lastChangeInfo_.uid, TEST_UID);
    EXPECT_EQ(cb0->lastChangeInfo_.sourceType, SOURCE_TYPE_MIC);

    EXPECT_EQ(audioPolicyClient->RemoveRecordControllerEnabledChangeCallback(), SUCCESS);
    EXPECT_FALSE(audioPolicyClient->HasRecordControllerEnabledChangeCallback());
    EXPECT_EQ(audioPolicyClient->GetRecordControllerEnabledChangeCallbackSize(), 0);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_090
* @tc.desc  : Test OnRecordControllerEnabledChange with empty callback list.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_090, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    SystemRecordControllerChangeInfo changeInfo;
    changeInfo.enabled = false;
    changeInfo.uid = TEST_UID;
    changeInfo.sourceType = SOURCE_TYPE_CAMCORDER;
    EXPECT_EQ(audioPolicyClient->OnRecordControllerEnabledChange(changeInfo), SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_091
* @tc.desc  : Test OnRecordControllerEnabledChange with single callback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_091, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteSystemRecordControllerEnabledChangeCallback>();
    audioPolicyClient->AddRecordControllerEnabledChangeCallback(cb);

    SystemRecordControllerChangeInfo changeInfo;
    changeInfo.enabled = true;
    changeInfo.uid = TEST_UID;
    changeInfo.sourceType = SOURCE_TYPE_LIVE;
    audioPolicyClient->OnRecordControllerEnabledChange(changeInfo);
    EXPECT_TRUE(cb->isCallbackInvoked_);
    EXPECT_TRUE(cb->lastChangeInfo_.enabled);
    EXPECT_EQ(cb->lastChangeInfo_.uid, TEST_UID);
    EXPECT_EQ(cb->lastChangeInfo_.sourceType, SOURCE_TYPE_LIVE);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_092
* @tc.desc  : Test AddRecordControllerEnabledChangeCallback with nullptr.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_092, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    EXPECT_EQ(audioPolicyClient->AddRecordControllerEnabledChangeCallback(nullptr), ERR_INVALID_PARAM);
    EXPECT_EQ(audioPolicyClient->GetRecordControllerEnabledChangeCallbackSize(), 0);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_093
* @tc.desc  : Test RemoveRecordControllerEnabledChangeCallback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_093, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClient != nullptr);

    auto cb = std::make_shared<ConcreteSystemRecordControllerEnabledChangeCallback>();
    audioPolicyClient->AddRecordControllerEnabledChangeCallback(cb);
    EXPECT_EQ(audioPolicyClient->GetRecordControllerEnabledChangeCallbackSize(), 1);
    EXPECT_TRUE(audioPolicyClient->HasRecordControllerEnabledChangeCallback());

    audioPolicyClient->RemoveRecordControllerEnabledChangeCallback();
    EXPECT_EQ(audioPolicyClient->GetRecordControllerEnabledChangeCallbackSize(), 0);
    EXPECT_FALSE(audioPolicyClient->HasRecordControllerEnabledChangeCallback());
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_094
* @tc.desc  : Test OnRecordControllerEnabledChange without callback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_094, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClient != nullptr);

    EXPECT_FALSE(audioPolicyClient->HasRecordControllerEnabledChangeCallback());
    SystemRecordControllerChangeInfo changeInfo;
    changeInfo.enabled = false;
    changeInfo.uid = TEST_UID;
    changeInfo.sourceType = SOURCE_TYPE_MIC;
    audioPolicyClient->OnRecordControllerEnabledChange(changeInfo);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_095
* @tc.desc  : Test OnRecordControllerEnabledChange with multiple callbacks.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_095, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClient != nullptr);

    auto cb1 = std::make_shared<ConcreteSystemRecordControllerEnabledChangeCallback>();
    auto cb2 = std::make_shared<ConcreteSystemRecordControllerEnabledChangeCallback>();
    audioPolicyClient->AddRecordControllerEnabledChangeCallback(cb1);
    audioPolicyClient->AddRecordControllerEnabledChangeCallback(cb2);
    EXPECT_EQ(audioPolicyClient->GetRecordControllerEnabledChangeCallbackSize(), 2);

    SystemRecordControllerChangeInfo changeInfo;
    changeInfo.enabled = true;
    changeInfo.uid = TEST_UID;
    changeInfo.sourceType = SOURCE_TYPE_CAMCORDER;
    audioPolicyClient->OnRecordControllerEnabledChange(changeInfo);
    EXPECT_TRUE(cb1->isCallbackInvoked_);
    EXPECT_TRUE(cb2->isCallbackInvoked_);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_096
* @tc.desc  : Test OnVolumeKeyEvent with expired callback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_096, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto mockCallback = std::make_shared<ConcreteVolumeKeyEventCallback>();
    EXPECT_EQ(audioPolicyClient->AddVolumeKeyEventCallback(mockCallback), SUCCESS);
    EXPECT_EQ(audioPolicyClient->volumeKeyEventCallbackList_.size(), 1);

    mockCallback.reset();
    VolumeEvent volumeEvent;
    EXPECT_EQ(audioPolicyClient->OnVolumeKeyEvent(volumeEvent), SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_097
* @tc.desc  : Test RemoveDeviceChangeCallback with non-matching callback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_097, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb0 = std::make_shared<ConcreteAudioManagerDeviceChangeCallback>();
    std::shared_ptr<AudioManagerDeviceChangeCallback> cb1 =
        std::make_shared<ConcreteAudioManagerDeviceChangeCallback>();
    EXPECT_EQ(audioPolicyClient->AddDeviceChangeCallback(DeviceFlag::ALL_DEVICES_FLAG, cb0), SUCCESS);
    EXPECT_EQ(audioPolicyClient->deviceChangeCallbackList_.size(), 1);

    EXPECT_EQ(audioPolicyClient->RemoveDeviceChangeCallback(DeviceFlag::ALL_DEVICES_FLAG, cb1), SUCCESS);
    EXPECT_EQ(audioPolicyClient->deviceChangeCallbackList_.size(), 1);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_098
* @tc.desc  : Test OnRendererDeviceChange with non-registered sessionId.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_098, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    AudioDeviceDescriptor preDeviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    AudioStreamDeviceChangeReasonExt reason(AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE);
    EXPECT_EQ(audioPolicyClient->OnRendererDeviceChange(9999, deviceInfo, reason, preDeviceInfo),
        ERR_OPERATION_FAILED);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_099
* @tc.desc  : Test OnCapturerStateChange with no registered callbacks.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_099, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    EXPECT_EQ(audioPolicyClient->OnCapturerStateChange(audioCapturerChangeInfos), SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_100
* @tc.desc  : Test AddSpatializationEnabledChangeCallback with existing callback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_100, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteAudioSpatializationEnabledChangeCallback>();
    EXPECT_EQ(audioPolicyClient->AddSpatializationEnabledChangeCallback(cb), SUCCESS);
    EXPECT_EQ(audioPolicyClient->AddSpatializationEnabledChangeCallback(cb), SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetSpatializationEnabledChangeCallbackSize(), 2);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_101
* @tc.desc  : Test RemoveSpatializationEnabledChangeCallback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_101, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteAudioSpatializationEnabledChangeCallback>();
    EXPECT_EQ(audioPolicyClient->AddSpatializationEnabledChangeCallback(cb), SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetSpatializationEnabledChangeCallbackSize(), 1);
    EXPECT_EQ(audioPolicyClient->RemoveSpatializationEnabledChangeCallback(), SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetSpatializationEnabledChangeCallbackSize(), 0);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_102
* @tc.desc  : Test OnSpatializationEnabledChange with nullptr callback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_102, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    std::shared_ptr<AudioSpatializationEnabledChangeCallback> nullCb = nullptr;
    audioPolicyClient->AddSpatializationEnabledChangeCallback(nullCb);
    bool enabled = true;
    EXPECT_EQ(audioPolicyClient->OnSpatializationEnabledChange(enabled), SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_103
* @tc.desc  : Test AddHeadTrackingEnabledChangeCallback with existing callback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_103, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteAudioHeadTrackingEnabledChangeCallback>();
    EXPECT_EQ(audioPolicyClient->AddHeadTrackingEnabledChangeCallback(cb), SUCCESS);
    EXPECT_EQ(audioPolicyClient->AddHeadTrackingEnabledChangeCallback(cb), SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetHeadTrackingEnabledChangeCallbacSize(), 2);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_104
* @tc.desc  : Test RemoveHeadTrackingEnabledChangeCallback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_104, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteAudioHeadTrackingEnabledChangeCallback>();
    EXPECT_EQ(audioPolicyClient->AddHeadTrackingEnabledChangeCallback(cb), SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetHeadTrackingEnabledChangeCallbacSize(), 1);
    EXPECT_EQ(audioPolicyClient->RemoveHeadTrackingEnabledChangeCallback(), SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetHeadTrackingEnabledChangeCallbacSize(), 0);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_105
* @tc.desc  : Test OnHeadTrackingEnabledChange with nullptr callback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_105, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    std::shared_ptr<AudioHeadTrackingEnabledChangeCallback> nullCb = nullptr;
    audioPolicyClient->AddHeadTrackingEnabledChangeCallback(nullCb);
    bool enabled = true;
    EXPECT_EQ(audioPolicyClient->OnHeadTrackingEnabledChange(enabled), SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_106
* @tc.desc  : Test AddRingerModeCallback with existing callback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_106, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteAudioRingerModeCallback>();
    EXPECT_EQ(audioPolicyClient->AddRingerModeCallback(cb), SUCCESS);
    EXPECT_EQ(audioPolicyClient->AddRingerModeCallback(cb), SUCCESS);
    EXPECT_EQ(audioPolicyClient->ringerModeCallbackList_.size(), 2);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_107
* @tc.desc  : Test AddAudioSessionCallback with existing callback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_107, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteAudioSessionCallback>();
    EXPECT_EQ(audioPolicyClient->AddAudioSessionCallback(cb), SUCCESS);
    EXPECT_EQ(audioPolicyClient->AddAudioSessionCallback(cb), SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetAudioSessionCallbackSize(), 2);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_108
* @tc.desc  : Test AddMicStateChangeCallback with existing callback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_108, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteAudioManagerMicStateChangeCallback>();
    EXPECT_EQ(audioPolicyClient->AddMicStateChangeCallback(cb), SUCCESS);
    EXPECT_EQ(audioPolicyClient->AddMicStateChangeCallback(cb), SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetMicStateChangeCallbackSize(), 2);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_109
* @tc.desc  : Test AddCapturerStateChangeCallback with existing callback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_109, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteAudioCapturerStateChangeCallback>();
    EXPECT_EQ(audioPolicyClient->AddCapturerStateChangeCallback(cb), SUCCESS);
    EXPECT_EQ(audioPolicyClient->AddCapturerStateChangeCallback(cb), SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetCapturerStateChangeCallbackSize(), 2);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_110
* @tc.desc  : Test AddDeviceChangeCallback with existing callback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_110, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteAudioManagerDeviceChangeCallback>();
    EXPECT_EQ(audioPolicyClient->AddDeviceChangeCallback(DeviceFlag::ALL_DEVICES_FLAG, cb), SUCCESS);
    EXPECT_EQ(audioPolicyClient->AddDeviceChangeCallback(DeviceFlag::ALL_DEVICES_FLAG, cb), SUCCESS);
    EXPECT_EQ(audioPolicyClient->deviceChangeCallbackList_.size(), 2);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_111
* @tc.desc  : Test AddRendererStateChangeCallback with existing callback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_111, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteAudioRendererStateChangeCallback>();
    EXPECT_EQ(audioPolicyClient->AddRendererStateChangeCallback(cb), SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetRendererStateChangeCallbackSize(), 1);
    EXPECT_EQ(audioPolicyClient->AddRendererStateChangeCallback(cb), SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetRendererStateChangeCallbackSize(), 2);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_112
* @tc.desc  : Test RemoveVolumeKeyEventCallback with non-existent callback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_112, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb0 = std::make_shared<ConcreteVolumeKeyEventCallback>();
    auto cb1 = std::make_shared<ConcreteVolumeKeyEventCallback>();
    EXPECT_EQ(audioPolicyClient->AddVolumeKeyEventCallback(cb0), SUCCESS);
    EXPECT_EQ(audioPolicyClient->volumeKeyEventCallbackList_.size(), 1);
    EXPECT_EQ(audioPolicyClient->RemoveVolumeKeyEventCallback(cb1), SUCCESS);
    EXPECT_EQ(audioPolicyClient->volumeKeyEventCallbackList_.size(), 1);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_113
* @tc.desc  : Test OnSpatializationEnabledChangeForAnyDevice with nullptr callback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_113, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    std::shared_ptr<AudioSpatializationEnabledChangeCallback> nullCb = nullptr;
    audioPolicyClient->AddSpatializationEnabledChangeCallback(nullCb);
    bool enabled = true;
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    EXPECT_EQ(audioPolicyClient->OnSpatializationEnabledChangeForAnyDevice(deviceDescriptor, enabled), SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_114
* @tc.desc  : Test OnHeadTrackingEnabledChangeForAnyDevice with nullptr callback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_114, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    std::shared_ptr<AudioHeadTrackingEnabledChangeCallback> nullCb = nullptr;
    audioPolicyClient->AddHeadTrackingEnabledChangeCallback(nullCb);
    bool enabled = true;
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    EXPECT_EQ(audioPolicyClient->OnHeadTrackingEnabledChangeForAnyDevice(deviceDescriptor, enabled), SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_115
* @tc.desc  : Test OnRingerModeUpdated with nullptr callback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_115, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    std::shared_ptr<AudioRingerModeCallback> nullCb = nullptr;
    audioPolicyClient->AddRingerModeCallback(nullCb);
    EXPECT_EQ(audioPolicyClient->ringerModeCallbackList_.size(), 1);
    audioPolicyClient->OnRingerModeUpdated(AudioRingerMode::RINGER_MODE_SILENT);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_116
* @tc.desc  : Test OnAudioSessionDeactive with nullptr callback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_116, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    std::shared_ptr<AudioSessionCallback> nullCb = nullptr;
    audioPolicyClient->AddAudioSessionCallback(nullCb);
    EXPECT_EQ(audioPolicyClient->GetAudioSessionCallbackSize(), 1);
    int32_t deactiveEvent = 0;
    EXPECT_EQ(audioPolicyClient->OnAudioSessionDeactive(deactiveEvent), SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_117
* @tc.desc  : Test OnMicStateUpdated with nullptr callback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_117, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    std::shared_ptr<AudioManagerMicStateChangeCallback> nullCb = nullptr;
    audioPolicyClient->AddMicStateChangeCallback(nullCb);
    EXPECT_EQ(audioPolicyClient->GetMicStateChangeCallbackSize(), 1);
    MicStateChangeEvent micStateChangeEvent;
    EXPECT_EQ(audioPolicyClient->OnMicStateUpdated(micStateChangeEvent), SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_118
* @tc.desc  : Test OnCapturerStateChange with expired callback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_118, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteAudioCapturerStateChangeCallback>();
    EXPECT_EQ(audioPolicyClient->AddCapturerStateChangeCallback(cb), SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetCapturerStateChangeCallbackSize(), 1);

    cb.reset();
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    EXPECT_EQ(audioPolicyClient->OnCapturerStateChange(audioCapturerChangeInfos), SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_119
* @tc.desc  : Test OnSpatializationEnabledChangeForCurrentDevice with nullptr callback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_119, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    std::shared_ptr<AudioSpatializationEnabledChangeForCurrentDeviceCallback> nullCb = nullptr;
    audioPolicyClient->AddSpatializationEnabledChangeForCurrentDeviceCallback(nullCb);
    bool enabled = true;
    EXPECT_EQ(audioPolicyClient->OnSpatializationEnabledChangeForCurrentDevice(enabled), SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_120
* @tc.desc  : Test RemoveDeviceChangeCallback with non-matching flag.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_120, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteAudioManagerDeviceChangeCallback>();
    EXPECT_EQ(audioPolicyClient->AddDeviceChangeCallback(DeviceFlag::OUTPUT_DEVICES_FLAG, cb), SUCCESS);
    EXPECT_EQ(audioPolicyClient->deviceChangeCallbackList_.size(), 1);

    std::shared_ptr<AudioManagerDeviceChangeCallback> cb2 =
        std::make_shared<ConcreteAudioManagerDeviceChangeCallback>();
    EXPECT_EQ(audioPolicyClient->RemoveDeviceChangeCallback(DeviceFlag::OUTPUT_DEVICES_FLAG, cb2), SUCCESS);
    EXPECT_EQ(audioPolicyClient->deviceChangeCallbackList_.size(), 1);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_121
* @tc.desc  : Test AddSystemVolumeChangeByFilterCallback/RemoveSystemVolumeChangeByFilterCallback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_121, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteSystemVolumeChangeByFilterCallback>();
    SystemVolumeFilter filter;
    filter.uid = TEST_UID;
    EXPECT_EQ(audioPolicyClient->AddSystemVolumeChangeByFilterCallback(filter, cb), SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetSystemVolumeChangeByFilterCallbackSize(), 1);

    auto removedFilter = audioPolicyClient->RemoveSystemVolumeChangeByFilterCallback(cb);
    EXPECT_EQ(audioPolicyClient->GetSystemVolumeChangeByFilterCallbackSize(), 0);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_122
* @tc.desc  : Test RemoveSystemVolumeChangeByFilterCallback with nullptr.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_122, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteSystemVolumeChangeByFilterCallback>();
    SystemVolumeFilter filter;
    filter.uid = TEST_UID;
    audioPolicyClient->AddSystemVolumeChangeByFilterCallback(filter, cb);
    EXPECT_EQ(audioPolicyClient->GetSystemVolumeChangeByFilterCallbackSize(), 1);

    auto removedFilter = audioPolicyClient->RemoveSystemVolumeChangeByFilterCallback(nullptr);
    EXPECT_EQ(audioPolicyClient->GetSystemVolumeChangeByFilterCallbackSize(), 0);
    EXPECT_EQ(removedFilter.uid, -1);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_123
* @tc.desc  : Test RemoveSystemVolumeChangeByFilterCallback with multiple callbacks same uid.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_123, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb1 = std::make_shared<ConcreteSystemVolumeChangeByFilterCallback>();
    auto cb2 = std::make_shared<ConcreteSystemVolumeChangeByFilterCallback>();
    SystemVolumeFilter filter;
    filter.uid = TEST_UID;
    audioPolicyClient->AddSystemVolumeChangeByFilterCallback(filter, cb1);
    audioPolicyClient->AddSystemVolumeChangeByFilterCallback(filter, cb2);
    EXPECT_EQ(audioPolicyClient->GetSystemVolumeChangeByFilterCallbackSize(), 2);

    auto removedFilter = audioPolicyClient->RemoveSystemVolumeChangeByFilterCallback(cb1);
    EXPECT_EQ(audioPolicyClient->GetSystemVolumeChangeByFilterCallbackSize(), 1);
    EXPECT_EQ(removedFilter.uid, -1);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_124
* @tc.desc  : Test GetRegisteredSystemVolumeFilters.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_124, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteSystemVolumeChangeByFilterCallback>();
    SystemVolumeFilter filter1;
    filter1.uid = 100;
    SystemVolumeFilter filter2;
    filter2.uid = 200;
    audioPolicyClient->AddSystemVolumeChangeByFilterCallback(filter1, cb);
    audioPolicyClient->AddSystemVolumeChangeByFilterCallback(filter2, cb);

    auto filters = audioPolicyClient->GetRegisteredSystemVolumeFilters();
    EXPECT_EQ(filters.size(), 2);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_125
* @tc.desc  : Test OnSystemVolumeChangeByFilter.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_125, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteSystemVolumeChangeByFilterCallback>();
    SystemVolumeFilter filter;
    filter.uid = TEST_UID;
    audioPolicyClient->AddSystemVolumeChangeByFilterCallback(filter, cb);

    VolumeEvent volumeEvent;
    EXPECT_EQ(audioPolicyClient->OnSystemVolumeChangeByFilter(filter, volumeEvent), SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_126
* @tc.desc  : Test OnSystemVolumeChangeByFilter with non-matching uid.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_126, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteSystemVolumeChangeByFilterCallback>();
    SystemVolumeFilter filter;
    filter.uid = TEST_UID;
    audioPolicyClient->AddSystemVolumeChangeByFilterCallback(filter, cb);

    SystemVolumeFilter otherFilter;
    otherFilter.uid = 9999;
    VolumeEvent volumeEvent;
    EXPECT_EQ(audioPolicyClient->OnSystemVolumeChangeByFilter(otherFilter, volumeEvent), SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_127
* @tc.desc  : Test OnAudioFocusInfoChange with valid focus info.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_127, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteAudioFocusInfoChangeCallback>();
    audioPolicyClient->AddFocusInfoChangeCallback(cb);

    std::vector<std::map<AudioInterrupt, int32_t>> focusInfoList;
    std::map<AudioInterrupt, int32_t> focusMap;
    AudioInterrupt interrupt;
    interrupt.streamId = 1;
    focusMap[interrupt] = 1;
    focusInfoList.push_back(focusMap);
    EXPECT_EQ(audioPolicyClient->OnAudioFocusInfoChange(focusInfoList), SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_128
* @tc.desc  : Test OnAudioFocusInfoChange with invalid size.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_128, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    std::vector<std::map<AudioInterrupt, int32_t>> focusInfoList(1000);
    EXPECT_EQ(audioPolicyClient->OnAudioFocusInfoChange(focusInfoList), ERR_INVALID_PARAM);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_129
* @tc.desc  : Test OnAudioFocusRequested.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_129, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteAudioFocusInfoChangeCallback>();
    audioPolicyClient->AddFocusInfoChangeCallback(cb);

    AudioInterrupt requestFocus;
    requestFocus.streamId = 1;
    EXPECT_EQ(audioPolicyClient->OnAudioFocusRequested(requestFocus), SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_130
* @tc.desc  : Test OnAudioFocusAbandoned.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_130, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteAudioFocusInfoChangeCallback>();
    audioPolicyClient->AddFocusInfoChangeCallback(cb);

    AudioInterrupt abandonFocus;
    abandonFocus.streamId = 1;
    EXPECT_EQ(audioPolicyClient->OnAudioFocusAbandoned(abandonFocus), SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_131
* @tc.desc  : Test AddDeviceInfoUpdateCallback/RemoveDeviceInfoUpdateCallback/OnDeviceInfoUpdate.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_131, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteAudioManagerDeviceInfoUpdateCallback>();
    EXPECT_EQ(audioPolicyClient->AddDeviceInfoUpdateCallback(cb), SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetDeviceInfoUpdateCallbackSize(), 1);

    DeviceChangeAction dca;
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    dca.deviceDescriptors.push_back(desc);
    EXPECT_EQ(audioPolicyClient->OnDeviceInfoUpdate(dca), SUCCESS);

    std::shared_ptr<AudioManagerDeviceInfoUpdateCallback> removeCb = cb;
    EXPECT_EQ(audioPolicyClient->RemoveDeviceInfoUpdateCallback(removeCb), SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetDeviceInfoUpdateCallbackSize(), 0);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_132
* @tc.desc  : Test RemoveDeviceInfoUpdateCallback with nullptr.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_132, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteAudioManagerDeviceInfoUpdateCallback>();
    audioPolicyClient->AddDeviceInfoUpdateCallback(cb);
    audioPolicyClient->AddDeviceInfoUpdateCallback(nullptr);
    EXPECT_EQ(audioPolicyClient->GetDeviceInfoUpdateCallbackSize(), 2);

    std::shared_ptr<AudioManagerDeviceInfoUpdateCallback> nullCb = nullptr;
    EXPECT_EQ(audioPolicyClient->RemoveDeviceInfoUpdateCallback(nullCb), SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetDeviceInfoUpdateCallbackSize(), 0);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_133
* @tc.desc  : Test OnDeviceInfoUpdate with empty device descriptors.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_133, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteAudioManagerDeviceInfoUpdateCallback>();
    audioPolicyClient->AddDeviceInfoUpdateCallback(cb);

    DeviceChangeAction dca;
    EXPECT_EQ(audioPolicyClient->OnDeviceInfoUpdate(dca), SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_134
* @tc.desc  : Test OnPreferredDeviceSet.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_134, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcretePreferredDeviceSetCallback>();
    EXPECT_EQ(audioPolicyClient->AddPreferredDeviceSetCallback(cb), SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetPreferredDeviceSetCallbackSize(), 1);

    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    EXPECT_EQ(audioPolicyClient->OnPreferredDeviceSet(0, deviceDesc, TEST_UID, "test"), SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_135
* @tc.desc  : Test OnPreferredDeviceSet with nullptr callback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_135, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    audioPolicyClient->AddPreferredDeviceSetCallback(nullptr);
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    EXPECT_EQ(audioPolicyClient->OnPreferredDeviceSet(0, deviceDesc, TEST_UID, "test"), SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_136
* @tc.desc  : Test OnAudioSceneChange with valid scene.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_136, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteAudioManagerAudioSceneChangedCallback>();
    int32_t clientId = 1;
    audioPolicyClient->AddAudioSceneChangedCallback(clientId, cb);

    EXPECT_EQ(audioPolicyClient->OnAudioSceneChange(AUDIO_SCENE_DEFAULT), SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_137
* @tc.desc  : Test OnAudioSceneChange with invalid scene.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_137, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    EXPECT_EQ(audioPolicyClient->OnAudioSceneChange(AUDIO_SCENE_INVALID), ERR_INVALID_PARAM);
    EXPECT_EQ(audioPolicyClient->OnAudioSceneChange(AUDIO_SCENE_MAX), ERR_INVALID_PARAM);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_138
* @tc.desc  : Test GetActiveVolumeTypeChangeCallbackSize.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_138, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    EXPECT_EQ(audioPolicyClient->GetActiveVolumeTypeChangeCallbackSize(), 0);

    auto cb = std::make_shared<ConcreteAudioManagerActiveVolumeTypeChangeCallback>();
    audioPolicyClient->AddActiveVolumeTypeChangeCallback(cb);
    EXPECT_EQ(audioPolicyClient->GetActiveVolumeTypeChangeCallbackSize(), 1);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_139
* @tc.desc  : Test OnSystemAppVolumeChanged.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_139, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteAudioManagerAppVolumeChangeCallback>();
    int32_t appUid = TEST_UID;
    audioPolicyClient->AddAppVolumeChangeForUidCallback(appUid, cb);

    VolumeEvent volumeEvent;
    EXPECT_EQ(audioPolicyClient->OnSystemAppVolumeChanged(appUid, volumeEvent), SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_140
* @tc.desc  : Test OnSystemAppVolumeChanged with ALL_APPS_UID.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_140, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteAudioManagerAppVolumeChangeCallback>();
    int32_t allAppsUid = -1;
    audioPolicyClient->AddAppVolumeChangeForUidCallback(allAppsUid, cb);

    VolumeEvent volumeEvent;
    EXPECT_EQ(audioPolicyClient->OnSystemAppVolumeChanged(TEST_UID, volumeEvent), SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_141
* @tc.desc  : Test RemoveSpatializationEnabledChangeForCurrentDeviceCallback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_141, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteSpatialEnabledChangeForCurrentDeviceCb>();
    EXPECT_EQ(audioPolicyClient->AddSpatializationEnabledChangeForCurrentDeviceCallback(cb), SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetSpatializationEnabledChangeForCurrentDeviceCallbackSize(), 1);

    EXPECT_EQ(audioPolicyClient->RemoveSpatializationEnabledChangeForCurrentDeviceCallback(), SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetSpatializationEnabledChangeForCurrentDeviceCallbackSize(), 0);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_142
* @tc.desc  : Test OnSpatialAudioSourceTypeChange.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_142, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteAudioSpatialAudioSourceTypeChangeCallback>();
    EXPECT_EQ(audioPolicyClient->AddSpatialAudioSourceTypeChangeCallback(cb), SUCCESS);

    int32_t mode = 0;
    EXPECT_EQ(audioPolicyClient->OnSpatialAudioSourceTypeChange(mode), SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_143
* @tc.desc  : Test RemoveSpatialAudioSourceTypeChangeCallback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_143, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteAudioSpatialAudioSourceTypeChangeCallback>();
    audioPolicyClient->AddSpatialAudioSourceTypeChangeCallback(cb);
    EXPECT_EQ(audioPolicyClient->GetCurrentSpatialAudioSourceTypeChangeCallbackSize(), 1);

    EXPECT_EQ(audioPolicyClient->RemoveSpatialAudioSourceTypeChangeCallback(), SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetCurrentSpatialAudioSourceTypeChangeCallbackSize(), 0);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_144
* @tc.desc  : Test OnAudioSeparationEffectEnabledChange.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_144, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteAudioSeparationEffectEnabledChangeCallback>();
    EXPECT_EQ(audioPolicyClient->AddAudioSeparationEffectEnabledChangeCallback(cb), SUCCESS);

    bool enabled = true;
    EXPECT_EQ(audioPolicyClient->OnAudioSeparationEffectEnabledChange(enabled), SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_145
* @tc.desc  : Test RemoveAudioSeparationEffectEnabledChangeCallback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_145, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteAudioSeparationEffectEnabledChangeCallback>();
    audioPolicyClient->AddAudioSeparationEffectEnabledChangeCallback(cb);
    EXPECT_EQ(audioPolicyClient->GetAudioSeparationEffectEnabledChangeCallbackSize(), 1);

    EXPECT_EQ(audioPolicyClient->RemoveAudioSeparationEffectEnabledChangeCallback(), SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetAudioSeparationEffectEnabledChangeCallbackSize(), 0);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_146
* @tc.desc  : Test OnNnStateChange.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_146, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteAudioNnStateChangeCallback>();
    EXPECT_EQ(audioPolicyClient->AddNnStateChangeCallback(cb), SUCCESS);

    int32_t nnState = 1;
    EXPECT_EQ(audioPolicyClient->OnNnStateChange(nnState), SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_147
* @tc.desc  : Test RemoveNnStateChangeCallback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_147, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteAudioNnStateChangeCallback>();
    audioPolicyClient->AddNnStateChangeCallback(cb);
    EXPECT_EQ(audioPolicyClient->GetNnStateChangeCallbackSize(), 1);

    EXPECT_EQ(audioPolicyClient->RemoveNnStateChangeCallback(), SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetNnStateChangeCallbackSize(), 0);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_148
* @tc.desc  : Test RemoveAdaptiveSpatialRenderingEnabledChangeCallback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_148, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteAudioAdaptiveSpatialRenderingEnabledChangeCallback>();
    EXPECT_EQ(audioPolicyClient->AddAdaptiveSpatialRenderingEnabledChangeCallback(cb), SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetAdaptiveSpatialRenderingEnabledChangeCallbackSize(), 1);

    EXPECT_EQ(audioPolicyClient->RemoveAdaptiveSpatialRenderingEnabledChangeCallback(), SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetAdaptiveSpatialRenderingEnabledChangeCallbackSize(), 0);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_149
* @tc.desc  : Test RemovePersonalizedSpatializationEnabledChangeCallback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_149, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteAudioPersonalizedSpatializationEnabledChangeCallback>();
    EXPECT_EQ(audioPolicyClient->AddPersonalizedSpatializationEnabledChangeCallback(cb), SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetPersonalizedSpatializationEnabledChangeCallbackSize(), 1);

    EXPECT_EQ(audioPolicyClient->RemovePersonalizedSpatializationEnabledChangeCallback(), SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetPersonalizedSpatializationEnabledChangeCallbackSize(), 0);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_150
* @tc.desc  : Test SetAudioRouteSelectorCallback/UnsetAudioRouteSelectorCallback/HasAudioRouteSelectorCallback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_150, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    EXPECT_FALSE(audioPolicyClient->HasAudioRouteSelectorCallback());

    auto cb = std::make_shared<ConcreteAudioRouteSelector>();
    EXPECT_EQ(audioPolicyClient->SetAudioRouteSelectorCallback(cb), SUCCESS);
    EXPECT_TRUE(audioPolicyClient->HasAudioRouteSelectorCallback());

    EXPECT_EQ(audioPolicyClient->UnsetAudioRouteSelectorCallback(), SUCCESS);
    EXPECT_FALSE(audioPolicyClient->HasAudioRouteSelectorCallback());
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_151
* @tc.desc  : Test OnAudioRouteSelectRefined.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_151, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteAudioRouteSelector>();
    audioPolicyClient->SetAudioRouteSelectorCallback(cb);

    auto routeSelectInfo = std::make_shared<AudioRouteSelectInfo>();
    int32_t selectResult = 0;
    EXPECT_EQ(audioPolicyClient->OnAudioRouteSelectRefined(routeSelectInfo, selectResult), SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStubImpl.
* @tc.number: AudioPolicyClientStubImpl_152
* @tc.desc  : Test OnAudioRouteSelectRefined without callback.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_152, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto routeSelectInfo = std::make_shared<AudioRouteSelectInfo>();
    int32_t selectResult = 0;
    EXPECT_EQ(audioPolicyClient->OnAudioRouteSelectRefined(routeSelectInfo, selectResult), ERROR);
}

/**
* @tc.name  : Test AudioDeviceDescriptor.
* @tc.number: AudioDeviceDescriptor_GetKey_001
* @tc.desc  : Test AudioDeviceDescriptor GetKey.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioDeviceDescriptor_GetKey_001, TestSize.Level1)
{
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->networkId_ = LOCAL_NETWORK_ID;
    desc->deviceType_ = DEVICE_TYPE_SPEAKER;
    std::string key = desc->GetKey();
    EXPECT_FALSE(key.empty());
    EXPECT_NE(key.find(LOCAL_NETWORK_ID), std::string::npos);
}

/**
* @tc.name  : Test AudioDeviceDescriptor.
* @tc.number: AudioDeviceDescriptor_GetKey_002
* @tc.desc  : Test AudioDeviceDescriptor GetKey with remote network id.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioDeviceDescriptor_GetKey_002, TestSize.Level1)
{
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->networkId_ = "remote_network";
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    std::string key = desc->GetKey();
    EXPECT_FALSE(key.empty());
    EXPECT_NE(key.find("remote_network"), std::string::npos);
}

/**
* @tc.name  : Test volume-limit-exceeded callback registration and delivery.
* @tc.number: AudioPolicyClientStubImpl_153
* @tc.desc  : Test add, callback delivery, specific removal, and remove all.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_153, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    auto callback = std::make_shared<ConcreteVolumeLimitExceededCallback>();
    ASSERT_EQ(audioPolicyClient->AddVolumeLimitExceededCallback(callback), SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetVolumeLimitExceededCallbackSize(), 1);

    VolumeLimitExceedEvent event;
    event.volumeType = STREAM_MUSIC;
    event.currentVolume = 75;
    event.volumeThreshold = 60;
    event.uid = TEST_UID;
    EXPECT_EQ(audioPolicyClient->OnVolumeLimitExceeded(event), SUCCESS);
    EXPECT_TRUE(callback->called_);
    EXPECT_EQ(callback->event_.volumeType, event.volumeType);
    EXPECT_EQ(callback->event_.currentVolume, event.currentVolume);
    EXPECT_EQ(callback->event_.volumeThreshold, event.volumeThreshold);
    EXPECT_EQ(callback->event_.uid, event.uid);

    EXPECT_EQ(audioPolicyClient->RemoveVolumeLimitExceededCallback(callback), SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetVolumeLimitExceededCallbackSize(), 0);
    ASSERT_EQ(audioPolicyClient->AddVolumeLimitExceededCallback(callback), SUCCESS);
    EXPECT_EQ(audioPolicyClient->RemoveVolumeLimitExceededCallback(nullptr), SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetVolumeLimitExceededCallbackSize(), 0);
}

} // namespace AudioStandard
} // namespace OHOS
