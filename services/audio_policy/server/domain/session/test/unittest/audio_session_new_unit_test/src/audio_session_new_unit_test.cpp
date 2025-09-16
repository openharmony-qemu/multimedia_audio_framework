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
#include "audio_session_new_unit_test.h"
#include "audio_pipe_manager.h"
#include "audio_device_manager.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioSessionNewUnitTest::SetUpTestCase(void) {}
void AudioSessionNewUnitTest::TearDownTestCase(void) {}
void AudioSessionNewUnitTest::SetUp(void) {}
void AudioSessionNewUnitTest::TearDown(void)
{
    audioSessionService_.sessionMap_.clear();
    audioSessionService_.timeOutCallback_.reset();
}

/**
* @tc.name  : Test SetAudioSessionScene
* @tc.number: AudioSessionNewUnitTest_SetAudioSessionScene_001
* @tc.desc  : Test SetAudioSessionScene function
*/
HWTEST_F(AudioSessionNewUnitTest, SetAudioSessionScene_001, TestSize.Level4)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionService_);

    AudioSessionScene scene = AudioSessionScene::GAME;
    int32_t ret = audioSession->SetAudioSessionScene(scene);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test SetAudioSessionScene
* @tc.number: AudioSessionNewUnitTest_SetAudioSessionScene_002
* @tc.desc  : Test SetAudioSessionScene function
*/
HWTEST_F(AudioSessionNewUnitTest, SetAudioSessionScene_002, TestSize.Level4)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionService_);

    AudioSessionScene scene = AudioSessionScene::VOICE_COMMUNICATION;
    int32_t ret = audioSession->SetAudioSessionScene(scene);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioSession.
* @tc.number: AudioSessionNewUnitTest_AddStreamInfo_001.
* @tc.desc  : Test AddStreamInfo.
*/
HWTEST_F(AudioSessionNewUnitTest, AddStreamInfo_001, TestSize.Level4)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionService_);

    AudioInterrupt audioInterrupt = {};
    audioInterrupt.streamId = 1;
    audioSession->state_ = AudioSessionState::SESSION_ACTIVE;
    audioSession->audioSessionScene_ = AudioSessionScene::GAME;
    audioInterrupt.audioFocusType.sourceType = SOURCE_TYPE_MIC;
    audioSessionService_.AddStreamInfo(audioInterrupt);
    EXPECT_TRUE(audioSession->IsAudioSessionEmpty());
}

/**
* @tc.name  : Test AudioSession.
* @tc.number: AudioSessionNewUnitTest_AddStreamInfo_002.
* @tc.desc  : Test AddStreamInfo.
*/
HWTEST_F(AudioSessionNewUnitTest, AddStreamInfo_002, TestSize.Level4)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionService_);

    AudioInterrupt audioInterrupt = {};
    audioInterrupt.streamId = 1;
    audioSession->state_ = AudioSessionState::SESSION_ACTIVE;
    audioSession->defaultDeviceType_ = DEVICE_TYPE_SPEAKER;
    audioSession->audioSessionScene_ = AudioSessionScene::MEDIA;
    audioInterrupt.audioFocusType.sourceType = SOURCE_TYPE_INVALID;
    audioSession->AddStreamInfo(audioInterrupt);
    EXPECT_FALSE(audioSession->IsAudioSessionEmpty());
}

/**
* @tc.name  : Test AudioSession.
* @tc.number: AudioSessionUnitTest_RemoveStreamInfo_001.
* @tc.desc  : Test RemoveStreamInfo.
*/
HWTEST_F(AudioSessionNewUnitTest, RemoveStreamInfo_001, TestSize.Level4)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionService_);

    AudioInterrupt audioInterrupt = {};
    audioInterrupt.streamId = 1;
    audioSession->AddStreamInfo(audioInterrupt);
    audioSession->state_ = AudioSessionState::SESSION_ACTIVE;
    audioSession->defaultDeviceType_ = DEVICE_TYPE_SPEAKER;
    audioSession->RemoveStreamInfo(audioInterrupt.streamId);
    EXPECT_TRUE(audioSession->IsAudioSessionEmpty());
}

/**
* @tc.name  : Test AudioSession.
* @tc.number: AudioSessionUnitTest_UpdateSingleVoipStreamDefaultOutputDevice_001.
* @tc.desc  : Test RemoveStreamInfo.
*/
HWTEST_F(AudioSessionNewUnitTest, UpdateSingleVoipStreamDefaultOutputDevice_001, TestSize.Level4)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionService_);

    AudioInterrupt audioInterrupt = {};
    audioInterrupt.streamId = 1;
    audioInterrupt.streamUsage = STREAM_USAGE_MUSIC;
    audioSession->streamsInSession_.push_back(audioInterrupt);
    audioSession->SetAudioSessionScene(AudioSessionScene::MEDIA);
    int32_t ret = audioSession->Activate(strategy);
    EXPECT_EQ(ret, SUCCESS);
    audioSession->UpdateSingleVoipStreamDefaultOutputDevice(audioInterrupt);
}

/**
* @tc.name  : Test AudioSession.
* @tc.number: AudioSessionUnitTest_EnableSingleVoipStreamDefaultOutputDevice_001.
* @tc.desc  : Test RemoveStreamInfo.
*/
HWTEST_F(AudioSessionNewUnitTest, EnableSingleVoipStreamDefaultOutputDevice_001, TestSize.Level4)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionService_);
    EXPECT_EQ(SUCCESS, audioSession->SetAudioSessionScene(AudioSessionScene::MEDIA));

    AudioInterrupt interrupt = {};
    interrupt.streamUsage = STREAM_USAGE_MUSIC;
    interrupt.streamId = 2;
    audioSession->pipeManager_ = nullptr;

    int32_t ret = audioSession->EnableSingleVoipStreamDefaultOutputDevice(interrupt);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioSession.
* @tc.number: AudioSessionUnitTest_EnableSingleVoipStreamDefaultOutputDevice_002.
* @tc.desc  : Test RemoveStreamInfo.
*/
HWTEST_F(AudioSessionNewUnitTest, EnableSingleVoipStreamDefaultOutputDevice_002, TestSize.Level4)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionService_);
    EXPECT_EQ(SUCCESS, audioSession->SetAudioSessionScene(AudioSessionScene::MEDIA));

    AudioInterrupt interrupt = {};
    interrupt.streamUsage = STREAM_USAGE_VOICE_MESSAGE;
    interrupt.streamId = 2;
    audioSession->pipeManager_ = nullptr;

    int32_t ret = audioSession->EnableSingleVoipStreamDefaultOutputDevice(interrupt);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioSession.
* @tc.number: AudioSessionUnitTest_EnableSingleVoipStreamDefaultOutputDevice_003.
* @tc.desc  : Test RemoveStreamInfo.
*/
HWTEST_F(AudioSessionNewUnitTest, EnableSingleVoipStreamDefaultOutputDevice_003, TestSize.Level4)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionService_);
    EXPECT_EQ(SUCCESS, audioSession->SetAudioSessionScene(AudioSessionScene::MEDIA));

    AudioInterrupt interrupt = {};
    interrupt.streamUsage = STREAM_USAGE_MUSIC;
    interrupt.streamId = 2;

    int32_t ret = audioSession->EnableSingleVoipStreamDefaultOutputDevice(interrupt);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test EnableSingleVoipStreamDefaultOutputDevice: enter SetDefaultOutputDevice via real data.
 * @tc.number: AudioSessionNewUnitTest_EnableSingleVoipStreamDefaultOutputDevice_004.
 * @tc.desc  : Test that function enters SetDefaultOutputDevice by injecting real stream descriptor.
 */
HWTEST_F(AudioSessionNewUnitTest, EnableSingleVoipStreamDefaultOutputDevice_004, TestSize.Level4)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionService_);
    EXPECT_EQ(SUCCESS, audioSession->SetAudioSessionScene(AudioSessionScene::MEDIA));
    
    AudioInterrupt interrupt = {};
    interrupt.streamUsage = STREAM_USAGE_VOICE_MESSAGE;
    interrupt.streamId = 2;

    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = 2;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_VOICE_MESSAGE;
    streamDesc->streamStatus_ = STREAM_STATUS_STARTED;

    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->id_ = 1;
    pipeInfo->name_ = "TestOutputPipe";
    pipeInfo->pipeRole_ = PIPE_ROLE_OUTPUT;
    pipeInfo->InitAudioStreamInfo();

    pipeInfo->streamDescriptors_.push_back(streamDesc);

    auto pipeManager = AudioPipeManager::GetPipeManager();
    pipeManager->AddAudioPipeInfo(pipeInfo);

    int32_t ret = audioSession->EnableSingleVoipStreamDefaultOutputDevice(interrupt);
    EXPECT_EQ(ret, SUCCESS);
    pipeManager->curPipeList_.clear();
}

/**
* @tc.name  : Test IsRecommendToStopAudio.
* @tc.number: AudioSessionUnitTest_IsRecommendToStopAudio_001.
* @tc.desc  : Test IsRecommendToStopAudio function.
*/
HWTEST_F(AudioSessionNewUnitTest, IsRecommendToStopAudio_001, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionService_);
    auto validDescriptor = std::make_shared<AudioDeviceDescriptor>();
    EXPECT_FALSE(audioSession->IsRecommendToStopAudio(AudioStreamDeviceChangeReason::OVERRODE, validDescriptor));
    EXPECT_FALSE(audioSession->IsRecommendToStopAudio(AudioStreamDeviceChangeReason::UNKNOWN, nullptr));
    EXPECT_FALSE(audioSession->IsRecommendToStopAudio(
        AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE, validDescriptor));
}

} // namespace AudioStandard
} // namespace OHOS
