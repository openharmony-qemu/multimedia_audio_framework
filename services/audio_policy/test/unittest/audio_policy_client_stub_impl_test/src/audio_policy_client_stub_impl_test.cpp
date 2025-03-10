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
* @tc.number: AudioPolicyClientStubImpl_003
* @tc.desc  : Test OnAudioFocusInfoChange/OnAudioFocusRequested/OnAudioFocusAbandoned.
*/
HWTEST(AudioPolicyClientStubImplTest, AudioPolicyClientStubImpl_003, TestSize.Level1)
{
    auto audioPolicyClient = std::make_shared<AudioPolicyClientStubImpl>();
    std::list<std::pair<AudioInterrupt, AudioFocuState>> focusInfoList;
    std::shared_ptr<AudioFocusInfoChangeCallback> callback0 =
        std::make_shared<ConcreteAudioFocusInfoChangeCallback>();
    std::shared_ptr<AudioFocusInfoChangeCallback> callback1 =
        std::make_shared<ConcreteAudioFocusInfoChangeCallback>();
    int32_t result = audioPolicyClient->AddFocusInfoChangeCallback(callback0);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(audioPolicyClient->focusInfoChangeCallbackList_.size(), 1);

    result = audioPolicyClient->AddFocusInfoChangeCallback(callback1);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(audioPolicyClient->focusInfoChangeCallbackList_.size(), 2);

    audioPolicyClient->OnAudioFocusInfoChange(focusInfoList);
    EXPECT_NE(audioPolicyClient, nullptr);

    AudioInterrupt requestFocus;
    audioPolicyClient->OnAudioFocusRequested(requestFocus);
    EXPECT_NE(audioPolicyClient, nullptr);

    AudioInterrupt abandonFocus;
    audioPolicyClient->OnAudioFocusAbandoned(abandonFocus);
    EXPECT_NE(audioPolicyClient, nullptr);
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

    AudioSessionDeactiveEvent deactiveEvent;
    audioPolicyClient->OnAudioSessionDeactive(deactiveEvent);
    EXPECT_NE(audioPolicyClient, nullptr);

    int32_t result = audioPolicyClient->RemoveAudioSessionCallback(cb1);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetAudioSessionCallbackSize(), 1);

    result = audioPolicyClient->RemoveAudioSessionCallback();
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(audioPolicyClient->GetAudioSessionCallbackSize(), 0);

    auto cb2 = std::make_shared<ConcreteAudioDistribuitedOutputChangeCallback>();
    result = audioPolicyClient->SetDistribuitedOutputChangeCallback(cb2);
    EXPECT_EQ(result, SUCCESS);
    AudioDeviceDescriptor desc;
    audioPolicyClient->OnDistribuitedOutputChange(desc, true);
    EXPECT_NE(audioPolicyClient, nullptr);
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
    EXPECT_EQ(deviceDescriptor.MapInternalToExternalDeviceType(), DEVICE_TYPE_USB_HEADSET);
    deviceDescriptor.deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;
    EXPECT_EQ(deviceDescriptor.MapInternalToExternalDeviceType(), DEVICE_TYPE_USB_HEADSET);
    deviceDescriptor.hasPair_ = false;
    deviceDescriptor.deviceRole_ = OUTPUT_DEVICE;
    deviceDescriptor.MapInternalToExternalDeviceType();
    deviceDescriptor.deviceRole_ = INPUT_DEVICE;
    EXPECT_EQ(deviceDescriptor.MapInternalToExternalDeviceType(), DEVICE_TYPE_USB_DEVICE);
    deviceDescriptor.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP_IN;
    EXPECT_EQ(deviceDescriptor.MapInternalToExternalDeviceType(), DEVICE_TYPE_BLUETOOTH_A2DP);
    deviceDescriptor.deviceType_ = DEVICE_TYPE_SPEAKER;
    EXPECT_EQ(deviceDescriptor.MapInternalToExternalDeviceType(), DEVICE_TYPE_SPEAKER);
}
} // namespace AudioStandard
} // namespace OHOS
