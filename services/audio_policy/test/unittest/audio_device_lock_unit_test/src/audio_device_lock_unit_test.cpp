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

#include "audio_device_lock_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

static int32_t NUM_5 = 5;
static int32_t NUM_20 = 20;

void AudioDeviceLockUnitTest::SetUpTestCase(void) {}
void AudioDeviceLockUnitTest::TearDownTestCase(void) {}
void AudioDeviceLockUnitTest::SetUp(void) {}
void AudioDeviceLockUnitTest::TearDown(void) {}

/**
* @tc.name  : Test AudioDeviceLock.
* @tc.number: AudioDeviceLock_001
* @tc.desc  : Test GetHasDpFlag interface.
*/
HWTEST_F(AudioDeviceLockUnitTest, AudioDeviceLock_001, TestSize.Level1)
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    EXPECT_NE(audioDeviceLock, nullptr);

    AudioMode mode = AudioMode::AUDIO_MODE_RECORD;
    AudioStreamChangeInfo streamChangeInfo;
    sptr<IRemoteObject> object = nullptr;
    int32_t apiVersion = 0;

    auto ret = audioDeviceLock->RegisterTracker(mode, streamChangeInfo, object, apiVersion);
    EXPECT_NE(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioDeviceLock.
* @tc.number: AudioDeviceLock_002
* @tc.desc  : Test GetHasDpFlag interface.
*/
HWTEST_F(AudioDeviceLockUnitTest, AudioDeviceLock_002, TestSize.Level1)
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    EXPECT_NE(audioDeviceLock, nullptr);

    AudioMode mode = AudioMode::AUDIO_MODE_RECORD;
    AudioStreamChangeInfo streamChangeInfo;
    sptr<IRemoteObject> object = nullptr;
    int32_t apiVersion = NUM_5;

    auto ret = audioDeviceLock->RegisterTracker(mode, streamChangeInfo, object, apiVersion);
    EXPECT_NE(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioDeviceLock.
* @tc.number: AudioDeviceLock_003
* @tc.desc  : Test GetHasDpFlag interface.
*/
HWTEST_F(AudioDeviceLockUnitTest, AudioDeviceLock_003, TestSize.Level1)
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    EXPECT_NE(audioDeviceLock, nullptr);

    AudioMode mode = AudioMode::AUDIO_MODE_RECORD;
    AudioStreamChangeInfo streamChangeInfo;
    sptr<IRemoteObject> object = nullptr;
    int32_t apiVersion = NUM_20;

    auto ret = audioDeviceLock->RegisterTracker(mode, streamChangeInfo, object, apiVersion);
    EXPECT_NE(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioDeviceLock.
* @tc.number: AudioDeviceLock_004
* @tc.desc  : Test GetHasDpFlag interface.
*/
HWTEST_F(AudioDeviceLockUnitTest, AudioDeviceLock_004, TestSize.Level1)
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    EXPECT_NE(audioDeviceLock, nullptr);

    AudioMode mode = AudioMode::AUDIO_MODE_PLAYBACK;
    AudioStreamChangeInfo streamChangeInfo;
    sptr<IRemoteObject> object = nullptr;
    int32_t apiVersion = 0;

    auto ret = audioDeviceLock->RegisterTracker(mode, streamChangeInfo, object, apiVersion);
    EXPECT_NE(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioDeviceLock.
* @tc.number: AudioDeviceLock_005
* @tc.desc  : Test GetHasDpFlag interface.
*/
HWTEST_F(AudioDeviceLockUnitTest, AudioDeviceLock_005, TestSize.Level1)
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    EXPECT_NE(audioDeviceLock, nullptr);

    RendererState rendererState = RENDERER_RUNNING;
    uint32_t sessionId = NUM_5;

    audioDeviceLock->audioA2dpOffloadManager_ = std::make_shared<AudioA2dpOffloadManager>();
    EXPECT_NE(audioDeviceLock->audioA2dpOffloadManager_, nullptr);


    audioDeviceLock->audioA2dpOffloadManager_->audioA2dpOffloadFlag_.currentOffloadConnectionState_ =
        CONNECTION_STATUS_CONNECTING;
    audioDeviceLock->audioA2dpOffloadManager_->connectionTriggerSessionIds_ = {NUM_20};

    audioDeviceLock->SendA2dpConnectedWhileRunning(rendererState, sessionId);
}

/**
* @tc.name  : Test AudioDeviceLock.
* @tc.number: AudioDeviceLock_006
* @tc.desc  : Test GetHasDpFlag interface.
*/
HWTEST_F(AudioDeviceLockUnitTest, AudioDeviceLock_006, TestSize.Level1)
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    EXPECT_NE(audioDeviceLock, nullptr);

    RendererState rendererState = RENDERER_RUNNING;
    uint32_t sessionId = NUM_5;

    audioDeviceLock->audioA2dpOffloadManager_ = nullptr;

    audioDeviceLock->SendA2dpConnectedWhileRunning(rendererState, sessionId);
}

/**
* @tc.name  : Test AudioDeviceLock.
* @tc.number: AudioDeviceLock_007
* @tc.desc  : Test GetHasDpFlag interface.
*/
HWTEST_F(AudioDeviceLockUnitTest, AudioDeviceLock_007, TestSize.Level1)
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    EXPECT_NE(audioDeviceLock, nullptr);

    RendererState rendererState = RENDERER_NEW;
    uint32_t sessionId = NUM_5;

    audioDeviceLock->audioA2dpOffloadManager_ = nullptr;

    audioDeviceLock->SendA2dpConnectedWhileRunning(rendererState, sessionId);
}

/**
* @tc.name  : Test AudioDeviceLock.
* @tc.number: AudioDeviceLock_008
* @tc.desc  : Test GetHasDpFlag interface.
*/
HWTEST_F(AudioDeviceLockUnitTest, AudioDeviceLock_008, TestSize.Level1)
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    EXPECT_NE(audioDeviceLock, nullptr);

    AudioMode mode = AudioMode::AUDIO_MODE_RECORD;
    AudioStreamChangeInfo streamChangeInfo;
    streamChangeInfo.audioCapturerChangeInfo.capturerState = CAPTURER_RELEASED;
    streamChangeInfo.audioCapturerChangeInfo.capturerInfo.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;

    audioDeviceLock->HandleAudioCaptureState(mode, streamChangeInfo);
}

/**
* @tc.name  : Test AudioDeviceLock.
* @tc.number: AudioDeviceLock_009
* @tc.desc  : Test GetHasDpFlag interface.
*/
HWTEST_F(AudioDeviceLockUnitTest, AudioDeviceLock_009, TestSize.Level1)
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    EXPECT_NE(audioDeviceLock, nullptr);

    AudioMode mode = AudioMode::AUDIO_MODE_RECORD;
    AudioStreamChangeInfo streamChangeInfo;
    streamChangeInfo.audioCapturerChangeInfo.capturerState = CAPTURER_RELEASED;
    streamChangeInfo.audioCapturerChangeInfo.capturerInfo.sourceType = SOURCE_TYPE_MIC;

    audioDeviceLock->HandleAudioCaptureState(mode, streamChangeInfo);
}

/**
* @tc.name  : Test AudioDeviceLock.
* @tc.number: AudioDeviceLock_010
* @tc.desc  : Test GetHasDpFlag interface.
*/
HWTEST_F(AudioDeviceLockUnitTest, AudioDeviceLock_010, TestSize.Level1)
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    EXPECT_NE(audioDeviceLock, nullptr);

    AudioMode mode = AudioMode::AUDIO_MODE_RECORD;
    AudioStreamChangeInfo streamChangeInfo;
    streamChangeInfo.audioCapturerChangeInfo.capturerState = CAPTURER_STOPPED;
    streamChangeInfo.audioCapturerChangeInfo.capturerInfo.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;

    audioDeviceLock->HandleAudioCaptureState(mode, streamChangeInfo);
}

/**
* @tc.name  : Test AudioDeviceLock.
* @tc.number: AudioDeviceLock_011
* @tc.desc  : Test GetHasDpFlag interface.
*/
HWTEST_F(AudioDeviceLockUnitTest, AudioDeviceLock_011, TestSize.Level1)
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    EXPECT_NE(audioDeviceLock, nullptr);

    AudioMode mode = AudioMode::AUDIO_MODE_RECORD;
    AudioStreamChangeInfo streamChangeInfo;
    streamChangeInfo.audioCapturerChangeInfo.capturerState = CAPTURER_NEW;

    audioDeviceLock->HandleAudioCaptureState(mode, streamChangeInfo);
}

/**
* @tc.name  : Test AudioDeviceLock.
* @tc.number: AudioDeviceLock_012
* @tc.desc  : Test GetHasDpFlag interface.
*/
HWTEST_F(AudioDeviceLockUnitTest, AudioDeviceLock_012, TestSize.Level1)
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    EXPECT_NE(audioDeviceLock, nullptr);

    AudioMode mode = AudioMode::AUDIO_MODE_PLAYBACK;
    AudioStreamChangeInfo streamChangeInfo;
    streamChangeInfo.audioCapturerChangeInfo.capturerState = CAPTURER_NEW;

    audioDeviceLock->HandleAudioCaptureState(mode, streamChangeInfo);
}

/**
* @tc.name  : Test AudioDeviceLock.
* @tc.number: AudioDeviceLock_013
* @tc.desc  : Test GetHasDpFlag interface.
*/
HWTEST_F(AudioDeviceLockUnitTest, AudioDeviceLock_013, TestSize.Level1)
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    EXPECT_NE(audioDeviceLock, nullptr);

    AudioMode mode = AudioMode::AUDIO_MODE_PLAYBACK;
    AudioStreamChangeInfo streamChangeInfo;
    streamChangeInfo.audioRendererChangeInfo.rendererState = RENDERER_PREPARED;

    auto ret = audioDeviceLock->UpdateTracker(mode, streamChangeInfo);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioDeviceLock.
* @tc.number: AudioDeviceLock_014
* @tc.desc  : Test GetHasDpFlag interface.
*/
HWTEST_F(AudioDeviceLockUnitTest, AudioDeviceLock_014, TestSize.Level1)
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    EXPECT_NE(audioDeviceLock, nullptr);

    AudioMode mode = AudioMode::AUDIO_MODE_PLAYBACK;
    AudioStreamChangeInfo streamChangeInfo;
    streamChangeInfo.audioRendererChangeInfo.rendererState = RENDERER_NEW;

    auto ret = audioDeviceLock->UpdateTracker(mode, streamChangeInfo);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioDeviceLock.
* @tc.number: AudioDeviceLock_015
* @tc.desc  : Test GetHasDpFlag interface.
*/
HWTEST_F(AudioDeviceLockUnitTest, AudioDeviceLock_015, TestSize.Level1)
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    EXPECT_NE(audioDeviceLock, nullptr);

    AudioMode mode = AudioMode::AUDIO_MODE_PLAYBACK;
    AudioStreamChangeInfo streamChangeInfo;
    streamChangeInfo.audioRendererChangeInfo.rendererState = RENDERER_NEW;

    auto ret = audioDeviceLock->UpdateTracker(mode, streamChangeInfo);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioDeviceLock.
* @tc.number: AudioDeviceLock_016
* @tc.desc  : Test GetHasDpFlag interface.
*/
HWTEST_F(AudioDeviceLockUnitTest, AudioDeviceLock_016, TestSize.Level1)
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    EXPECT_NE(audioDeviceLock, nullptr);

    AudioMode mode = AudioMode::AUDIO_MODE_PLAYBACK;
    AudioStreamChangeInfo streamChangeInfo;
    streamChangeInfo.audioRendererChangeInfo.rendererState = RENDERER_INVALID;

    auto ret = audioDeviceLock->UpdateTracker(mode, streamChangeInfo);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioDeviceLock.
* @tc.number: AudioDeviceLock_017
* @tc.desc  : Test GetHasDpFlag interface.
*/
HWTEST_F(AudioDeviceLockUnitTest, AudioDeviceLock_017, TestSize.Level1)
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    EXPECT_NE(audioDeviceLock, nullptr);

    AudioMode mode = AudioMode::AUDIO_MODE_PLAYBACK;
    AudioStreamChangeInfo streamChangeInfo;
    streamChangeInfo.audioRendererChangeInfo.rendererState = RENDERER_STOPPED;

    audioDeviceLock->audioA2dpOffloadManager_ = std::make_shared<AudioA2dpOffloadManager>();
    EXPECT_NE(audioDeviceLock->audioA2dpOffloadManager_, nullptr);

    auto ret = audioDeviceLock->UpdateTracker(mode, streamChangeInfo);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioDeviceLock.
* @tc.number: AudioDeviceLock_018
* @tc.desc  : Test GetHasDpFlag interface.
*/
HWTEST_F(AudioDeviceLockUnitTest, AudioDeviceLock_018, TestSize.Level1)
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    EXPECT_NE(audioDeviceLock, nullptr);

    AudioMode mode = AudioMode::AUDIO_MODE_PLAYBACK;
    AudioStreamChangeInfo streamChangeInfo;
    streamChangeInfo.audioRendererChangeInfo.rendererState = RENDERER_STOPPED;

    audioDeviceLock->audioA2dpOffloadManager_ = nullptr;

    auto ret = audioDeviceLock->UpdateTracker(mode, streamChangeInfo);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioDeviceLock.
* @tc.number: AudioDeviceLock_019
* @tc.desc  : Test GetHasDpFlag interface.
*/
HWTEST_F(AudioDeviceLockUnitTest, AudioDeviceLock_019, TestSize.Level1)
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    EXPECT_NE(audioDeviceLock, nullptr);

    pid_t uid = PIPE_TYPE_OFFLOAD;

    audioDeviceLock->RegisteredTrackerClientDied(uid);
    EXPECT_NE(audioDeviceLock->streamCollector_.ExistStreamForPipe(PIPE_TYPE_OFFLOAD), true);
}

/**
* @tc.name  :  AudioDeviceLock_020
* @tc.desc  : Test GetHasDpFlag interface.
*/
HWTEST_F(AudioDeviceLockUnitTest, AudioDeviceLock_020, TestSize.Level1)
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    EXPECT_NE(audioDeviceLock, nullptr);

    AudioDeviceDescriptor updatedDesc;
    updatedDesc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    updatedDesc.connectState_ = CONNECTED;
    bool isConnected = true;

    audioDeviceLock->OnDeviceStatusUpdated(updatedDesc, isConnected);
    EXPECT_NE(audioDeviceLock, nullptr);
}

/**
* @tc.name  :  AudioDeviceLock_021
* @tc.desc  : Test GetHasDpFlag interface.
*/
HWTEST_F(AudioDeviceLockUnitTest, AudioDeviceLock_021, TestSize.Level1)
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    EXPECT_NE(audioDeviceLock, nullptr);

    AudioDeviceDescriptor updatedDesc;
    updatedDesc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    updatedDesc.connectState_ = CONNECTED;
    bool isConnected = false;

    audioDeviceLock->OnDeviceStatusUpdated(updatedDesc, isConnected);
    EXPECT_NE(audioDeviceLock, nullptr);
}

/**
* @tc.name  :  AudioDeviceLock_022
* @tc.desc  : Test GetHasDpFlag interface.
*/
HWTEST_F(AudioDeviceLockUnitTest, AudioDeviceLock_022, TestSize.Level1)
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    EXPECT_NE(audioDeviceLock, nullptr);

    AudioDeviceDescriptor updatedDesc;
    updatedDesc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    updatedDesc.connectState_ = VIRTUAL_CONNECTED;
    bool isConnected = false;

    audioDeviceLock->OnDeviceStatusUpdated(updatedDesc, isConnected);
    EXPECT_NE(audioDeviceLock, nullptr);
}

/**
* @tc.name  :  AudioDeviceLock_023
* @tc.desc  : Test GetHasDpFlag interface.
*/
HWTEST_F(AudioDeviceLockUnitTest, AudioDeviceLock_023, TestSize.Level1)
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    EXPECT_NE(audioDeviceLock, nullptr);

    AudioDeviceDescriptor updatedDesc;
    updatedDesc.deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    updatedDesc.connectState_ = VIRTUAL_CONNECTED;
    bool isConnected = false;

    audioDeviceLock->OnDeviceStatusUpdated(updatedDesc, isConnected);
    EXPECT_NE(audioDeviceLock, nullptr);
}

/**
* @tc.name  :  AudioDeviceLock_024
* @tc.desc  : Test GetHasDpFlag interface.
*/
HWTEST_F(AudioDeviceLockUnitTest, AudioDeviceLock_024, TestSize.Level1)
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    EXPECT_NE(audioDeviceLock, nullptr);

    AudioDeviceDescriptor updatedDesc;
    updatedDesc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP_IN;
    updatedDesc.connectState_ = CONNECTED;
    bool isConnected = true;

    audioDeviceLock->OnDeviceStatusUpdated(updatedDesc, isConnected);
    EXPECT_NE(audioDeviceLock, nullptr);
}

/**
* @tc.name  :  AudioDeviceLock_025
* @tc.desc  : Test GetHasDpFlag interface.
*/
HWTEST_F(AudioDeviceLockUnitTest, AudioDeviceLock_025, TestSize.Level1)
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    EXPECT_NE(audioDeviceLock, nullptr);

    AudioDeviceDescriptor updatedDesc;
    updatedDesc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP_IN;
    updatedDesc.connectState_ = CONNECTED;
    bool isConnected = false;

    audioDeviceLock->OnDeviceStatusUpdated(updatedDesc, isConnected);
    EXPECT_NE(audioDeviceLock, nullptr);
}

/**
* @tc.name  :  AudioDeviceLock_026
* @tc.desc  : Test GetHasDpFlag interface.
*/
HWTEST_F(AudioDeviceLockUnitTest, AudioDeviceLock_026, TestSize.Level1)
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    EXPECT_NE(audioDeviceLock, nullptr);

    AudioDeviceDescriptor updatedDesc;
    updatedDesc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP_IN;
    updatedDesc.connectState_ = VIRTUAL_CONNECTED;
    bool isConnected = false;

    audioDeviceLock->OnDeviceStatusUpdated(updatedDesc, isConnected);
    EXPECT_NE(audioDeviceLock, nullptr);
}

/**
* @tc.name  :  AudioDeviceLock_027
* @tc.desc  : Test GetHasDpFlag interface.
*/
HWTEST_F(AudioDeviceLockUnitTest, AudioDeviceLock_027, TestSize.Level1)
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    EXPECT_NE(audioDeviceLock, nullptr);

    std::vector<std::shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos =
        {std::make_shared<AudioRendererChangeInfo>()};
    bool hasBTPermission = true;
    bool hasSystemPermission = true;

    auto ret = audioDeviceLock->GetCurrentRendererChangeInfos(audioRendererChangeInfos,
        hasBTPermission, hasSystemPermission);
    EXPECT_NE(ret, true);
}
} // namespace AudioStandard
} // namespace OHOS