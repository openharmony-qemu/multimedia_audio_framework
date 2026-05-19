/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include "audio_errors.h"
#include "audio_session.h"
#include "audio_session_service.h"
#include "audio_device_manager.h"
#include "audio_session_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioSessionUnitTest::SetUpTestCase(void) {}
void AudioSessionUnitTest::TearDownTestCase(void) {}
void AudioSessionUnitTest::SetUp(void) {}
void AudioSessionUnitTest::TearDown(void) {}

/**
* @tc.name  : Test AudioSession.
* @tc.number: AudioSessionUnitTest_001.
* @tc.desc  : Test AddStreamInfo.
*/
HWTEST_F(AudioSessionUnitTest, AudioSessionUnitTest_001, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor_);

    AudioInterrupt audioInterrupt = {};
    audioInterrupt.streamId = 1;
    audioSession->AddStreamInfo({audioInterrupt, ACTIVE});
    EXPECT_FALSE(audioSession->IsAudioSessionEmpty());
    audioSession->RemoveStreamInfo(audioInterrupt.streamId);
    EXPECT_TRUE(audioSession->IsAudioSessionEmpty());
}

/**
* @tc.name  : Test AudioSession.
* @tc.number: AudioSessionUnitTest_002.
* @tc.desc  : Test RemoveStreamInfo.
*/
HWTEST_F(AudioSessionUnitTest, AudioSessionUnitTest_002, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor_);

    AudioInterrupt audioInterrupt = {};
    audioInterrupt.streamId = 1;
    audioSession->AddStreamInfo({audioInterrupt, ACTIVE});
    audioSession->RemoveStreamInfo(0);
    EXPECT_FALSE(audioSession->IsAudioSessionEmpty());
    audioSession->RemoveStreamInfo(audioInterrupt.streamId);
}

/**
* @tc.name  : Test AudioSession.
* @tc.number: AudioSessionUnitTest_003.
* @tc.desc  : Test RemoveStreamInfo.
*/
HWTEST_F(AudioSessionUnitTest, AudioSessionUnitTest_003, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor_);

    AudioInterrupt audioInterrupt = {};
    audioInterrupt.streamId = 10;
    audioSession->AddStreamInfo({audioInterrupt, ACTIVE});
    audioSession->RemoveStreamInfo(audioInterrupt.streamId);
    EXPECT_TRUE(audioSession->IsAudioSessionEmpty());
}

/**
* @tc.name  : Test AudioSession.
* @tc.number: AudioSessionUnitTest_004.
* @tc.desc  : Test RemoveStreamInfo.
*/
HWTEST_F(AudioSessionUnitTest, AudioSessionUnitTest_004, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor_);

    AudioInterrupt audioInterrupt = {};
    audioInterrupt.streamId = 0;
    audioSession->AddStreamInfo({audioInterrupt, ACTIVE});
    audioSession->RemoveStreamInfo(audioInterrupt.streamId);
    EXPECT_TRUE(audioSession->IsAudioSessionEmpty());
}

/**
* @tc.name  : Test AudioSession.
* @tc.number: AudioSessionUnitTest_005.
* @tc.desc  : Test RemoveStreamInfo.
*/
HWTEST_F(AudioSessionUnitTest, AudioSessionUnitTest_005, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor_);

    uint32_t i = 1;
    AudioInterrupt audioInterrupt = {};
    audioInterrupt.streamId = i;
    audioSession->AddStreamInfo({audioInterrupt, ACTIVE});

    AudioInterrupt audioInterrupt2;
    audioInterrupt2.streamId = i + 1;
    audioSession->AddStreamInfo({audioInterrupt2, ACTIVE});

    audioSession->RemoveStreamInfo(audioInterrupt2.streamId);
    EXPECT_FALSE(audioSession->IsAudioSessionEmpty());
    audioSession->RemoveStreamInfo(audioInterrupt.streamId);
}

/**
* @tc.name  : Test AudioSession.
* @tc.number: AudioSessionUnitTest_006.
* @tc.desc  : Test RemoveStreamInfo.
*/
HWTEST_F(AudioSessionUnitTest, AudioSessionUnitTest_006, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor_);

    uint32_t i = 1;
    AudioInterrupt audioInterrupt = {};
    audioInterrupt.streamId = i;

    audioSession->AddStreamInfo({audioInterrupt, ACTIVE});
    EXPECT_FALSE(audioSession->IsAudioSessionEmpty());
    audioSession->RemoveStreamInfo(audioInterrupt.streamId);
}

/**
* @tc.name  : Test AudioSession.
* @tc.number: AudioSessionUnitTest_007.
* @tc.desc  : Test IsAudioRendererEmpty.
*/
HWTEST_F(AudioSessionUnitTest, AudioSessionUnitTest_007, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor_);

    uint32_t streamId = 1;
    AudioInterrupt audioInterrupt = {};
    audioInterrupt.streamId = streamId;
    audioInterrupt.audioFocusType.streamType = STREAM_DEFAULT;

    audioSession->AddStreamInfo({audioInterrupt, ACTIVE});
    EXPECT_TRUE(audioSession->IsAudioRendererEmpty());
    audioSession->RemoveStreamInfo(streamId);
}

/**
* @tc.name  : Test AudioSession.
* @tc.number: AudioSessionUnitTest_008.
* @tc.desc  : Test IsAudioRendererEmpty.
*/
HWTEST_F(AudioSessionUnitTest, AudioSessionUnitTest_008, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor_);

    uint32_t streamId = 1;
    AudioInterrupt audioInterrupt = {};
    audioInterrupt.streamId = streamId;
    audioInterrupt.audioFocusType.streamType = STREAM_VOICE_CALL;

    audioSession->AddStreamInfo({audioInterrupt, ACTIVE});
    EXPECT_FALSE(audioSession->IsAudioRendererEmpty());
}

/**
* @tc.name  : Test EnableDefaultDevice.
* @tc.number: AudioSessionUnitTest_009.
* @tc.desc  : Test EnableDefaultDevice function.
*/
HWTEST_F(AudioSessionUnitTest, AudioSessionUnitTest_009, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor_);

    audioSession->fakeStreamId_ = 0;
    audioSession->state_ = AudioSessionState::SESSION_RELEASED;
    EXPECT_EQ(audioSession->EnableDefaultDevice(), 0);

    audioSession->state_ = AudioSessionState::SESSION_ACTIVE;
    audioSession->defaultDeviceType_ = DEVICE_TYPE_INVALID;
    EXPECT_EQ(audioSession->EnableDefaultDevice(), 0);
}

/**
* @tc.name  : Test SetSessionDefaultOutputDevice
* @tc.number: AudioSessionUnitTest_011
* @tc.desc  : Test SetSessionDefaultOutputDevice function
*/
HWTEST_F(AudioSessionUnitTest, AudioSessionUnitTest_011, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor_);

    EXPECT_EQ(audioSession->SetSessionDefaultOutputDevice(DEVICE_TYPE_INVALID), ERROR_INVALID_PARAM);
 
    audioSession->state_ = AudioSessionState::SESSION_ACTIVE;
    audioSession->fakeStreamId_ = 0;
    EXPECT_EQ(audioSession->SetSessionDefaultOutputDevice(DEVICE_TYPE_EARPIECE), 0);

    audioSession->state_ = AudioSessionState::SESSION_DEACTIVE;
    EXPECT_EQ(audioSession->SetSessionDefaultOutputDevice(DEVICE_TYPE_EARPIECE), 0);

    DeviceType type;
    audioSession->GetSessionDefaultOutputDevice(type);
}

/**
* @tc.name  : Test IsStreamContainedInCurrentSession
* @tc.number: AudioSessionUnitTest_012
* @tc.desc  : Test IsStreamContainedInCurrentSession function
*/
HWTEST_F(AudioSessionUnitTest, AudioSessionUnitTest_012, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor_);

    audioSession->interruptMap_.clear();
    EXPECT_FALSE(audioSession->IsStreamContainedInCurrentSession(0));

    AudioInterrupt incomingInterrupt;
    incomingInterrupt.streamId = 0;
    audioSession->interruptMap_[incomingInterrupt.streamId] = {incomingInterrupt, ACTIVE};
    EXPECT_TRUE(audioSession->IsStreamContainedInCurrentSession(0));
}

/**
* @tc.name  : Test IsRecommendToStopAudio
* @tc.number: AudioSessionUnitTest_013
* @tc.desc  : Test IsRecommendToStopAudio function
*/
HWTEST_F(AudioSessionUnitTest, AudioSessionUnitTest_013, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor_);
    EXPECT_FALSE(audioSession->IsRecommendToStopAudio(AudioStreamDeviceChangeReason::UNKNOWN, nullptr));
}

/**
* @tc.name  : Test UpdateVoipStreamsDefaultOutputDevice
* @tc.number: AudioSessionUnitTest_014
* @tc.desc  : Test UpdateVoipStreamsDefaultOutputDevice function
*/
HWTEST_F(AudioSessionUnitTest, AudioSessionUnitTest_014, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor_);

    AudioInterrupt audioInterrupt = {};
    audioInterrupt.streamId = 1;
    audioInterrupt.streamUsage = STREAM_USAGE_VOICE_MESSAGE;
    audioSession->interruptMap_[audioInterrupt.streamId] = {audioInterrupt, ACTIVE};
    audioSession->SetAudioSessionScene(AudioSessionScene::MEDIA);
    int32_t ret = audioSession->Activate(strategy);
    EXPECT_EQ(ret, SUCCESS);
    audioSession->SetSessionDefaultOutputDevice(DEVICE_TYPE_DEFAULT);
    ret = audioSession->Deactivate();
    EXPECT_EQ(ret, SUCCESS);
}


/**
* @tc.name  : Test SetAudioSessionScene
* @tc.number: AudioSessionUnitTest_015
* @tc.desc  : Test SetAudioSessionScene function
*/
HWTEST_F(AudioSessionUnitTest, AudioSessionUnitTest_015, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor_);

    AudioSessionScene scene = AudioSessionScene::INVALID;
    int32_t ret = audioSession->SetAudioSessionScene(scene);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
* @tc.name  : Test ShouldExcludeStreamType
* @tc.number: AudioSessionUnitTest_016
* @tc.desc  : Test ShouldExcludeStreamType function
*/
HWTEST_F(AudioSessionUnitTest, AudioSessionUnitTest_016, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor_);
    audioSession->IsActivated();
    audioSession->IsSceneParameterSet();
    AudioInterrupt incomingInterrupt;

    incomingInterrupt.audioFocusType.streamType = STREAM_NOTIFICATION;
    EXPECT_NO_THROW(
        audioSession->AddStreamInfo({incomingInterrupt, ACTIVE});
    );

    incomingInterrupt.isAudioSessionInterrupt = true;
    EXPECT_NO_THROW(
        audioSession->AddStreamInfo({incomingInterrupt, ACTIVE});
    );
}

/**
* @tc.name  : Test EnableSingleVoipStreamDefaultOutputDevice
* @tc.number: AudioSessionUnitTest_017
* @tc.desc  : Test EnableSingleVoipStreamDefaultOutputDevice function
*/
HWTEST_F(AudioSessionUnitTest, AudioSessionUnitTest_017, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor_);
    
    EXPECT_EQ(SUCCESS, audioSession->SetAudioSessionScene(AudioSessionScene::MEDIA));

    AudioInterrupt interrupt = {};
    interrupt.streamUsage = STREAM_USAGE_VOICE_MESSAGE;
    interrupt.streamId = 2;
    int32_t ret = audioSession->EnableSingleVoipStreamDefaultOutputDevice(interrupt);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test EnableVoipStreamsDefaultOutputDevice
* @tc.number: AudioSessionUnitTest_018
* @tc.desc  : Test EnableVoipStreamsDefaultOutputDevice function
*/
HWTEST_F(AudioSessionUnitTest, AudioSessionUnitTest_018, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor_);
    
    EXPECT_EQ(SUCCESS, audioSession->SetAudioSessionScene(AudioSessionScene::MEDIA));

    AudioInterrupt interrupt = {};
    interrupt.streamUsage = STREAM_USAGE_VOICE_MESSAGE;
    interrupt.streamId = 2;
    audioSession->AddStreamInfo({interrupt, ACTIVE});
    int32_t ret = audioSession->EnableVoipStreamsDefaultOutputDevice();
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test IsSessionOutputDeviceChanged
* @tc.number: AudioSessionUnitTest_019
* @tc.desc  : Test IsSessionOutputDeviceChanged function
*/
HWTEST_F(AudioSessionUnitTest, AudioSessionUnitTest_019, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor_);
    std::shared_ptr<AudioDeviceDescriptor> desc =
        std::make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, DeviceRole::OUTPUT_DEVICE);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> oldDeviceDescriptors;
    bool ret = audioSession->IsSessionOutputDeviceChanged(desc, oldDeviceDescriptors);
    EXPECT_TRUE(ret);
    ret = audioSession->IsSessionOutputDeviceChanged(desc, oldDeviceDescriptors);
    EXPECT_FALSE(ret);
}

/**
* @tc.name  : Test GetAudioSessionStreamUsageForDevice
* @tc.number: AudioSessionUnitTest_020
* @tc.desc  : Test GetAudioSessionStreamUsageForDevice function
*/
HWTEST_F(AudioSessionUnitTest, AudioSessionUnitTest_020, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor_);
    StreamUsage ret = audioSession->GetAudioSessionStreamUsageForDevice();
    EXPECT_EQ(ret, StreamUsage::STREAM_USAGE_INVALID);
    audioSession->SetAudioSessionScene(AudioSessionScene::VOICE_COMMUNICATION);
    ret = audioSession->GetAudioSessionStreamUsageForDevice();
    EXPECT_EQ(ret, StreamUsage::STREAM_USAGE_VOICE_COMMUNICATION);
    AudioInterrupt interrupt;
    interrupt.pid = callerPid;
    interrupt.streamUsage = StreamUsage::STREAM_USAGE_RINGTONE;
    interrupt.audioFocusType.streamType = STREAM_RING;
    audioSession->interruptMap_[interrupt.streamId] = {interrupt, ACTIVE};
    ret = audioSession->GetAudioSessionStreamUsageForDevice();
    EXPECT_EQ(ret, StreamUsage::STREAM_USAGE_RINGTONE);
}

/**
* @tc.name  : Test GetStreams
* @tc.number: AudioSessionUnitTest_021
* @tc.desc  : Test GetStreams function returns correct state
*/
HWTEST_F(AudioSessionUnitTest, AudioSessionUnitTest_021, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor_);

    AudioInterrupt audioInterrupt = {};
    audioInterrupt.streamId = 100;
    audioSession->interruptMap_[audioInterrupt.streamId] = {audioInterrupt, AudioFocuState::ACTIVE};

    auto streams = audioSession->GetStreams();
    bool found = false;
    for (const auto &[stream, state] : streams) {
        if (stream.streamId == audioInterrupt.streamId) {
            EXPECT_EQ(state, AudioFocuState::ACTIVE);
            found = true;
        }
    }
    EXPECT_TRUE(found);

    audioSession->interruptMap_[audioInterrupt.streamId] = {audioInterrupt, AudioFocuState::STANDBY};
    streams = audioSession->GetStreams();
    for (const auto &[stream, state] : streams) {
        if (stream.streamId == audioInterrupt.streamId) {
            EXPECT_EQ(state, AudioFocuState::STANDBY);
        }
    }
}

/**
* @tc.name  : Test UpdateStreamFocusState
* @tc.number: AudioSessionUnitTest_022
* @tc.desc  : Test UpdateStreamFocusState function success case
*/
HWTEST_F(AudioSessionUnitTest, AudioSessionUnitTest_022, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor_);

    AudioInterrupt audioInterrupt = {};
    audioInterrupt.streamId = 100;
    audioSession->interruptMap_[audioInterrupt.streamId] = {audioInterrupt, AudioFocuState::ACTIVE};

    int32_t ret = audioSession->UpdateStreamFocusState(audioInterrupt.streamId, AudioFocuState::STANDBY);
    EXPECT_EQ(ret, SUCCESS);

    auto streams = audioSession->GetStreams();
    for (const auto &[stream, state] : streams) {
        if (stream.streamId == audioInterrupt.streamId) {
            EXPECT_EQ(state, AudioFocuState::STANDBY);
        }
    }
}

/**
* @tc.name  : Test UpdateStreamFocusState
* @tc.number: AudioSessionUnitTest_023
* @tc.desc  : Test UpdateStreamFocusState function failure case
*/
HWTEST_F(AudioSessionUnitTest, AudioSessionUnitTest_023, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor_);

    int32_t ret = audioSession->UpdateStreamFocusState(999, AudioFocuState::STANDBY);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
* @tc.name  : Test AudioSession.
* @tc.number: AudioSessionUnitTest_025.
* @tc.desc  : Test RemoveStreamInfo.
*/
HWTEST_F(AudioSessionUnitTest, AudioSessionUnitTest_025, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor_);

    AudioInterrupt audioInterrupt = {};
    audioInterrupt.streamId = 10;
    EXPECT_EQ(SUCCESS, audioSession->SetAudioSessionScene(AudioSessionScene::MEDIA));
    audioSession->fakeFocusInterruptEvent_.hintType = INTERRUPT_HINT_PAUSE;
    audioSession->AddStreamInfo({audioInterrupt, ACTIVE});
    audioSession->RemoveStreamInfo(audioInterrupt.streamId);
    EXPECT_TRUE(audioSession->IsAudioSessionEmpty());

    audioSession->fakeFocusInterruptEvent_.hintType = INTERRUPT_HINT_STOP;
    audioSession->AddStreamInfo({audioInterrupt, ACTIVE});
    audioSession->RemoveStreamInfo(audioInterrupt.streamId);
    EXPECT_TRUE(audioSession->IsAudioSessionEmpty());

    EXPECT_EQ(SUCCESS, audioSession->SetAudioSessionScene(AudioSessionScene::INVALID));
    audioSession->fakeFocusInterruptEvent_.hintType = INTERRUPT_HINT_PAUSE;
    audioSession->AddStreamInfo({audioInterrupt, ACTIVE});
    audioSession->RemoveStreamInfo(audioInterrupt.streamId);
    EXPECT_TRUE(audioSession->IsAudioSessionEmpty());

    audioSession->fakeFocusInterruptEvent_.hintType = INTERRUPT_HINT_STOP;
    audioSession->AddStreamInfo({audioInterrupt, ACTIVE});
    audioSession->RemoveStreamInfo(audioInterrupt.streamId);
    EXPECT_TRUE(audioSession->IsAudioSessionEmpty());
}

} // namespace AudioStandard
} // namespace OHOS
