/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "audio_stream_collector_unit_test.h"
#include "audio_system_manager.h"
#include "audio_client_tracker_callback_proxy.h"
#include "audio_spatialization_service.h"
#include "audio_policy_log.h"
#include "audio_errors.h"
#include <thread>
#include <string>
#include <memory>
#include <vector>
#include <sys/socket.h>
#include <cerrno>
#include <fstream>
#include <algorithm>
using namespace std;
using namespace std::chrono;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioStreamCollectorUnitTest::SetUpTestCase(void) {}
void AudioStreamCollectorUnitTest::TearDownTestCase(void) {}
void AudioStreamCollectorUnitTest::SetUp(void) {}
void AudioStreamCollectorUnitTest::TearDown(void) {}


#define PRINT_LINE printf("debug __LINE__:%d\n", __LINE__)

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: AudioStreamCollector_001
* @tc.desc  : Test CheckRendererStateInfoChanged.
*/
HWTEST_F(AudioStreamCollectorUnitTest, AudioStreamCollector_001, TestSize.Level1)
{
    AudioStreamCollector audioStreamCollector_;
    AudioStreamChangeInfo streamChangeInfo;
    streamChangeInfo.audioRendererChangeInfo.clientUID = 1001;
    streamChangeInfo.audioRendererChangeInfo.sessionId = 2001;
    streamChangeInfo.audioRendererChangeInfo.rendererState = RendererState::RENDERER_RUNNING;
    // Test case 1: New client (not in the queue)
    EXPECT_TRUE(audioStreamCollector_.CheckRendererStateInfoChanged(streamChangeInfo));
    // Add the client to the queue
    audioStreamCollector_.rendererStatequeue_[std::make_pair(1001, 2001)] = 1;
    // Test case 2: Existing client with same state
    EXPECT_TRUE(audioStreamCollector_.CheckRendererStateInfoChanged(streamChangeInfo));
    // Test case 3: Existing client with different state
    streamChangeInfo.audioRendererChangeInfo.rendererState = RendererState::RENDERER_PAUSED;
    EXPECT_TRUE(audioStreamCollector_.CheckRendererStateInfoChanged(streamChangeInfo));
    // Test case 4: Different client
    streamChangeInfo.audioRendererChangeInfo.clientUID = 1002;
    EXPECT_TRUE(audioStreamCollector_.CheckRendererStateInfoChanged(streamChangeInfo));
    // Test case 5: Different session ID
    streamChangeInfo.audioRendererChangeInfo.clientUID = 1001;
    streamChangeInfo.audioRendererChangeInfo.sessionId = 2002;
    EXPECT_TRUE(audioStreamCollector_.CheckRendererStateInfoChanged(streamChangeInfo));
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: AudioStreamCollector_002
* @tc.desc  : Test UpdateRendererDeviceInfo.
*/
HWTEST_F(AudioStreamCollectorUnitTest, AudioStreamCollector_002, TestSize.Level1)
{
    AudioStreamCollector audioStreamCollector_;
    AudioDeviceDescriptor outputDeviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    outputDeviceInfo.deviceType_ = DeviceType::DEVICE_TYPE_SPEAKER;
    auto info1 = std::make_unique<AudioRendererChangeInfo>();
    info1->outputDeviceInfo.deviceType_ = DeviceType::DEVICE_TYPE_EARPIECE;
    audioStreamCollector_.audioRendererChangeInfos_.push_back(std::move(info1));
    auto info2 = std::make_unique<AudioRendererChangeInfo>();
    info2->outputDeviceInfo.deviceType_ = DeviceType::DEVICE_TYPE_SPEAKER;
    audioStreamCollector_.audioRendererChangeInfos_.push_back(std::move(info2));
    int32_t result = audioStreamCollector_.UpdateRendererDeviceInfo(outputDeviceInfo);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(audioStreamCollector_.audioRendererChangeInfos_.size(), 2);
    EXPECT_EQ(
        audioStreamCollector_.audioRendererChangeInfos_[0]->outputDeviceInfo.deviceType_,
        DeviceType::DEVICE_TYPE_SPEAKER
    );
    EXPECT_EQ(
        audioStreamCollector_.audioRendererChangeInfos_[1]->outputDeviceInfo.deviceType_,
        DeviceType::DEVICE_TYPE_SPEAKER
    );
    AudioDeviceDescriptor sameDeviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    sameDeviceInfo.deviceType_ = DeviceType::DEVICE_TYPE_SPEAKER;
    result = audioStreamCollector_.UpdateRendererDeviceInfo(sameDeviceInfo);

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(
        audioStreamCollector_.audioRendererChangeInfos_[0]->outputDeviceInfo.deviceType_,
        DeviceType::DEVICE_TYPE_SPEAKER
    );
    EXPECT_EQ(
        audioStreamCollector_.audioRendererChangeInfos_[1]->outputDeviceInfo.deviceType_,
        DeviceType::DEVICE_TYPE_SPEAKER
    );
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: AudioStreamCollector_003
* @tc.desc  : Test UpdateCapturerDeviceInfo.
*/
HWTEST_F(AudioStreamCollectorUnitTest, AudioStreamCollector_003, TestSize.Level1)
{
    AudioStreamCollector audioStreamCollector_;
    AudioDeviceDescriptor inputDeviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    inputDeviceInfo.deviceType_ = DeviceType::DEVICE_TYPE_MIC;
    auto info1 = std::make_unique<AudioCapturerChangeInfo>();
    info1->inputDeviceInfo.deviceType_ = DeviceType::DEVICE_TYPE_WIRED_HEADSET;
    audioStreamCollector_.audioCapturerChangeInfos_.push_back(std::move(info1));
    auto info2 = std::make_unique<AudioCapturerChangeInfo>();
    info2->inputDeviceInfo.deviceType_ = DeviceType::DEVICE_TYPE_MIC;
    audioStreamCollector_.audioCapturerChangeInfos_.push_back(std::move(info2));
    int32_t result = audioStreamCollector_.UpdateCapturerDeviceInfo(inputDeviceInfo);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(audioStreamCollector_.audioCapturerChangeInfos_.size(), 2);
    EXPECT_EQ(
        audioStreamCollector_.audioCapturerChangeInfos_[0]->inputDeviceInfo.deviceType_,
        DeviceType::DEVICE_TYPE_MIC);
    EXPECT_EQ(
        audioStreamCollector_.audioCapturerChangeInfos_[1]->inputDeviceInfo.deviceType_,
        DeviceType::DEVICE_TYPE_MIC
    );
    AudioDeviceDescriptor sameDeviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    sameDeviceInfo.deviceType_ = DeviceType::DEVICE_TYPE_MIC;
    result = audioStreamCollector_.UpdateCapturerDeviceInfo(sameDeviceInfo);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(
        audioStreamCollector_.audioCapturerChangeInfos_[0]->inputDeviceInfo.deviceType_,
        DeviceType::DEVICE_TYPE_MIC
    );
    EXPECT_EQ(
        audioStreamCollector_.audioCapturerChangeInfos_[1]->inputDeviceInfo.deviceType_,
        DeviceType::DEVICE_TYPE_MIC
    );
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: AudioStreamCollector_004
* @tc.desc  : Test GetAndCompareStreamType.
*/
HWTEST_F(AudioStreamCollectorUnitTest, AudioStreamCollector_004, TestSize.Level1)
{
    AudioStreamCollector audioStreamCollector_;
    // Test case 1: Match between target usage and renderer info
    {
        StreamUsage targetUsage = STREAM_USAGE_MEDIA;
        AudioRendererInfo rendererInfo;
        rendererInfo.contentType = CONTENT_TYPE_UNKNOWN;
        rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
        bool result = audioStreamCollector_.GetAndCompareStreamType(targetUsage, rendererInfo);
        EXPECT_TRUE(result);
    }
    // Test case 2: Mismatch between target usage and renderer info
    {
        StreamUsage targetUsage = STREAM_USAGE_ALARM;
        AudioRendererInfo rendererInfo;
        rendererInfo.contentType = CONTENT_TYPE_UNKNOWN;
        rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
        bool result = audioStreamCollector_.GetAndCompareStreamType(targetUsage, rendererInfo);
        EXPECT_FALSE(result);
    }
    // Test case 3: Unknown content type and stream usage
    {
        StreamUsage targetUsage = STREAM_USAGE_UNKNOWN;
        AudioRendererInfo rendererInfo;
        rendererInfo.contentType = CONTENT_TYPE_UNKNOWN;
        rendererInfo.streamUsage = STREAM_USAGE_UNKNOWN;
        bool result = audioStreamCollector_.GetAndCompareStreamType(targetUsage, rendererInfo);
        EXPECT_TRUE(result);
    }
    // Test case 4: Voice communication
    {
        StreamUsage targetUsage = STREAM_USAGE_VOICE_COMMUNICATION;
        AudioRendererInfo rendererInfo;
        rendererInfo.contentType = CONTENT_TYPE_UNKNOWN;
        rendererInfo.streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;
        bool result = audioStreamCollector_.GetAndCompareStreamType(targetUsage, rendererInfo);
        EXPECT_TRUE(result);
    }
    // Test case 5: Media stream type (should be converted to STREAM_MUSIC)
    {
        StreamUsage targetUsage = STREAM_USAGE_MEDIA;
        AudioRendererInfo rendererInfo;
        rendererInfo.contentType = CONTENT_TYPE_UNKNOWN;
        rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
        bool result = audioStreamCollector_.GetAndCompareStreamType(targetUsage, rendererInfo);
        EXPECT_TRUE(result);
    }
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: AudioStreamCollector_005
* @tc.desc  : Test UpdateStreamState.
*/
HWTEST_F(AudioStreamCollectorUnitTest, AudioStreamCollector_005, TestSize.Level1)
{
    AudioStreamCollector audioStreamCollector_;
    int32_t clientUid = 1001;
    StreamSetStateEventInternal event;
    event.streamSetState = StreamSetState::STREAM_PAUSE;
    event.streamUsage = STREAM_USAGE_MEDIA;
    // Create and add AudioRendererChangeInfo
    auto changeInfo = std::make_unique<AudioRendererChangeInfo>();
    changeInfo->clientUID = clientUid;
    changeInfo->rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
    changeInfo->sessionId = 1;
    audioStreamCollector_.audioRendererChangeInfos_.push_back(std::move(changeInfo));
    int32_t result = audioStreamCollector_.UpdateStreamState(clientUid, event);
    EXPECT_EQ(result, SUCCESS);
    event.streamSetState = StreamSetState::STREAM_RESUME;
    result = audioStreamCollector_.UpdateStreamState(clientUid, event);
    EXPECT_EQ(result, SUCCESS);
    event.streamSetState = StreamSetState::STREAM_MUTE;
    result = audioStreamCollector_.UpdateStreamState(clientUid, event);
    EXPECT_EQ(result, SUCCESS);
    event.streamSetState = StreamSetState::STREAM_UNMUTE;
    result = audioStreamCollector_.UpdateStreamState(clientUid, event);
    EXPECT_EQ(result, SUCCESS);
    event.streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;
    event.streamSetState = StreamSetState::STREAM_MUTE;
    result = audioStreamCollector_.UpdateStreamState(clientUid, event);
    EXPECT_EQ(result, SUCCESS);
    // The tests do not match clientUid
    result = audioStreamCollector_.UpdateStreamState(9999, event);
    //The function should still return SUCCESS, but no callbacks should be called
    EXPECT_EQ(result, SUCCESS);
    // Test empty callback
    audioStreamCollector_.clientTracker_.clear();
    result = audioStreamCollector_.UpdateStreamState(clientUid, event);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: AudioStreamCollector_006
* @tc.desc  : Test GetRunningStream.
*/
HWTEST_F(AudioStreamCollectorUnitTest, AudioStreamCollector_006, TestSize.Level1)
{
    std::unique_ptr<AudioStreamCollector> collector = std::make_unique<AudioStreamCollector>();
    AudioRendererInfo rendererInfo1 = {
        .contentType = CONTENT_TYPE_MUSIC, .streamUsage = STREAM_USAGE_MEDIA
    };
    AudioRendererInfo rendererInfo2 = {
        .contentType = CONTENT_TYPE_SPEECH, .streamUsage = STREAM_USAGE_VOICE_COMMUNICATION
    };
    AudioRendererInfo rendererInfo3 = {
        .contentType = CONTENT_TYPE_MOVIE, .streamUsage = STREAM_USAGE_MEDIA
    };
    std::unique_ptr<AudioRendererChangeInfo> info1 = std::make_unique<AudioRendererChangeInfo>();
    info1->sessionId = 1;
    info1->rendererState = RENDERER_RUNNING;
    info1->rendererInfo = rendererInfo1;
    info1->channelCount = 2;
    std::unique_ptr<AudioRendererChangeInfo> info2 = std::make_unique<AudioRendererChangeInfo>();
    info2->sessionId = 2;
    info2->rendererState = RENDERER_RUNNING;
    info2->rendererInfo = rendererInfo2;
    info2->channelCount = 1;
    std::unique_ptr<AudioRendererChangeInfo> info3 = std::make_unique<AudioRendererChangeInfo>();
    info3->sessionId = 3;
    info3->rendererState = RENDERER_PAUSED;
    info3->rendererInfo = rendererInfo3;
    info3->channelCount = 2;
    collector->audioRendererChangeInfos_.push_back(std::move(info1));
    collector->audioRendererChangeInfos_.push_back(std::move(info2));
    collector->audioRendererChangeInfos_.push_back(std::move(info3));
    // Test case 1: Default stream type and channel count
    EXPECT_EQ(collector->GetRunningStream(STREAM_DEFAULT, 0), 1);
    // Test case 2: Specific stream type (STREAM_MUSIC) and default channel count
    EXPECT_EQ(collector->GetRunningStream(STREAM_MUSIC, 0), 1);
    // Test case 3: Specific stream type (STREAM_VOICE_COMMUNICATION) and default channel count
    EXPECT_EQ(collector->GetRunningStream(STREAM_VOICE_COMMUNICATION, 0), 2);
    // Test case 4: Specific stream type (STREAM_MUSIC) and specific channel count (2)
    EXPECT_EQ(collector->GetRunningStream(STREAM_MUSIC, 2), 1);
    // Test case 5: Specific stream type (STREAM_VOICE_COMMUNICATION) and specific channel count (1)
    EXPECT_EQ(collector->GetRunningStream(STREAM_VOICE_COMMUNICATION, 1), 2);
    // Test case 6: Non-existent stream type
    EXPECT_EQ(collector->GetRunningStream(STREAM_ALARM, 0), -1);
    // Test case 7: Existing stream type but non-matching channel count
    EXPECT_EQ(collector->GetRunningStream(STREAM_MUSIC, 1), -1);
    // Test case 8: Existing stream type and matching channel count, but not in RUNNING state
    EXPECT_EQ(collector->GetRunningStream(STREAM_MUSIC, 2), 1);  // Should still return 1, not 3
}

std::unique_ptr<AudioRendererChangeInfo> CreateChangeInfo(
    int32_t sessionId,
    RendererState state,
    ContentType contentType,
    StreamUsage streamUsage,
    int32_t channelCount)
{
    auto info = std::make_unique<AudioRendererChangeInfo>();
    info->sessionId = sessionId;
    info->rendererState = state;
    info->rendererInfo.contentType = contentType;
    info->rendererInfo.streamUsage = streamUsage;
    info->channelCount = channelCount;
    return info;
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: AudioStreamCollector_007
* @tc.desc  : Test GetRunningStream.
*/
HWTEST_F(AudioStreamCollectorUnitTest, AudioStreamCollector_007, TestSize.Level1)
{
    std::unique_ptr<AudioStreamCollector> collector = std::make_unique<AudioStreamCollector>();
    std::vector<std::unique_ptr<AudioRendererChangeInfo>> testInfos;
    testInfos.emplace_back(
        CreateChangeInfo(1, RendererState::RENDERER_RUNNING, ContentType::CONTENT_TYPE_MUSIC,
                        StreamUsage::STREAM_USAGE_MEDIA, 2));
    testInfos.emplace_back(
        CreateChangeInfo(2, RendererState::RENDERER_RUNNING, ContentType::CONTENT_TYPE_SPEECH,
                        StreamUsage::STREAM_USAGE_VOICE_COMMUNICATION, 1));
    testInfos.emplace_back(
        CreateChangeInfo(3, RendererState::RENDERER_PAUSED, ContentType::CONTENT_TYPE_MOVIE,
                        StreamUsage::STREAM_USAGE_MEDIA, 2));
    testInfos.emplace_back(
        CreateChangeInfo(4, RendererState::RENDERER_RUNNING, ContentType::CONTENT_TYPE_PROMPT,
                        StreamUsage::STREAM_USAGE_NOTIFICATION, 1));
    testInfos.emplace_back(
        CreateChangeInfo(5, RendererState::RENDERER_RUNNING, ContentType::CONTENT_TYPE_DTMF,
                        StreamUsage::STREAM_USAGE_VIDEO_COMMUNICATION, 1));
    testInfos.emplace_back(
        CreateChangeInfo(6, RendererState::RENDERER_RUNNING, ContentType::CONTENT_TYPE_UNKNOWN,
                        StreamUsage::STREAM_USAGE_ALARM, 2));
    for (auto& info : testInfos) {
        collector->audioRendererChangeInfos_.push_back(std::move(info));
    }

    // Test case 1: Default stream type and channel count
    EXPECT_EQ(collector->GetRunningStream(STREAM_DEFAULT, 0), 1);
    // Test case 2: Specific stream type (STREAM_MUSIC) and default channel count
    EXPECT_EQ(collector->GetRunningStream(STREAM_MUSIC, 0), 1);
    // Test case 3: Specific stream type (STREAM_VOICE_COMMUNICATION) and default channel count
    EXPECT_EQ(collector->GetRunningStream(STREAM_VOICE_COMMUNICATION, 0), 2);
    // Test case 4: Specific stream type (STREAM_MUSIC) and specific channel count (2)
    EXPECT_EQ(collector->GetRunningStream(STREAM_MUSIC, 2), 1);
    // Test case 5: Specific stream type (STREAM_VOICE_COMMUNICATION) and specific channel count (1)
    EXPECT_EQ(collector->GetRunningStream(STREAM_VOICE_COMMUNICATION, 1), 2);
    // Test case 6: Non-existent stream type
    EXPECT_EQ(collector->GetRunningStream(STREAM_VOICE_CALL, 0), -1);
    // Test case 7: Existing stream type but non-matching channel count
    EXPECT_NE(collector->GetRunningStream(STREAM_MUSIC, 1), -1);
    // Test case 8: Existing stream type and matching channel count, but not in RUNNING state
    EXPECT_EQ(collector->GetRunningStream(STREAM_MOVIE, 2), -1);
    // Test case 9: STREAM_NOTIFICATION
    EXPECT_EQ(collector->GetRunningStream(STREAM_NOTIFICATION, 0), 4);
    // Test case 10: STREAM_DTMF
    EXPECT_NE(collector->GetRunningStream(STREAM_DTMF, 0), 5);
    // Test case 11: STREAM_ALARM (mapped from UNKNOWN content type)
    EXPECT_EQ(collector->GetRunningStream(STREAM_ALARM, 0), 6);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: AudioStreamCollector_008
* @tc.desc  : Test GetRunningStream.
*/
HWTEST_F(AudioStreamCollectorUnitTest, AudioStreamCollector_008, TestSize.Level1)
{
    AudioStreamCollector collector;
    // Test SOURCE_TYPE_MIC
    EXPECT_EQ(STREAM_MUSIC, collector.GetStreamTypeFromSourceType(SOURCE_TYPE_MIC));
    // Test SOURCE_TYPE_VOICE_COMMUNICATION and SOURCE_TYPE_VOICE_CALL
    EXPECT_EQ(STREAM_VOICE_CALL, collector.GetStreamTypeFromSourceType(SOURCE_TYPE_VOICE_COMMUNICATION));
    EXPECT_EQ(STREAM_VOICE_CALL, collector.GetStreamTypeFromSourceType(SOURCE_TYPE_VOICE_CALL));
    // Test SOURCE_TYPE_ULTRASONIC
    EXPECT_EQ(STREAM_ULTRASONIC, collector.GetStreamTypeFromSourceType(SOURCE_TYPE_ULTRASONIC));
    // Test SOURCE_TYPE_WAKEUP
    EXPECT_EQ(STREAM_WAKEUP, collector.GetStreamTypeFromSourceType(SOURCE_TYPE_WAKEUP));
    // Test SOURCE_TYPE_CAMCORDER
    EXPECT_EQ(STREAM_CAMCORDER, collector.GetStreamTypeFromSourceType(SOURCE_TYPE_CAMCORDER));
    // Test cases where sourceType is directly cast to AudioStreamType
    EXPECT_EQ(static_cast<AudioStreamType>(SOURCE_TYPE_VOICE_RECOGNITION),
              collector.GetStreamTypeFromSourceType(SOURCE_TYPE_VOICE_RECOGNITION));
    EXPECT_EQ(static_cast<AudioStreamType>(SOURCE_TYPE_PLAYBACK_CAPTURE),
              collector.GetStreamTypeFromSourceType(SOURCE_TYPE_PLAYBACK_CAPTURE));
    EXPECT_EQ(static_cast<AudioStreamType>(SOURCE_TYPE_REMOTE_CAST),
              collector.GetStreamTypeFromSourceType(SOURCE_TYPE_REMOTE_CAST));
    EXPECT_EQ(static_cast<AudioStreamType>(SOURCE_TYPE_VIRTUAL_CAPTURE),
              collector.GetStreamTypeFromSourceType(SOURCE_TYPE_VIRTUAL_CAPTURE));
    EXPECT_EQ(static_cast<AudioStreamType>(SOURCE_TYPE_VOICE_MESSAGE),
              collector.GetStreamTypeFromSourceType(SOURCE_TYPE_VOICE_MESSAGE));
    // Test an invalid source type (should fall into default case)
    EXPECT_EQ(static_cast<AudioStreamType>(SOURCE_TYPE_INVALID),
              collector.GetStreamTypeFromSourceType(SOURCE_TYPE_INVALID));
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: AudioStreamCollector_009
* @tc.desc  : Test UpdateCapturerInfoMuteStatus.
*/
HWTEST_F(AudioStreamCollectorUnitTest, AudioStreamCollector_009, TestSize.Level1)
{
    AudioStreamCollector collector;
    auto changeInfo1 = std::make_unique<AudioCapturerChangeInfo>();
    changeInfo1->clientUID = 1000;
    changeInfo1->muted = false;
    changeInfo1->sessionId = 1;
    changeInfo1->capturerInfo.sourceType = SOURCE_TYPE_MIC;
    changeInfo1->inputDeviceInfo.deviceType_ = DeviceType::DEVICE_TYPE_MIC;
    auto changeInfo2 = std::make_unique<AudioCapturerChangeInfo>();
    changeInfo2->clientUID = 2000;
    changeInfo2->muted = false;
    changeInfo2->sessionId = 2;
    changeInfo2->capturerInfo.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    changeInfo2->inputDeviceInfo.deviceType_ = DeviceType::DEVICE_TYPE_MIC;
    collector.audioCapturerChangeInfos_.push_back(std::move(changeInfo1));
    collector.audioCapturerChangeInfos_.push_back(std::move(changeInfo2));
    // Create a mock AudioPolicyServerHandler
    collector.audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    // Test updating mute status for a specific UID
    EXPECT_EQ(SUCCESS, collector.UpdateCapturerInfoMuteStatus(1000, true));
    EXPECT_TRUE(collector.audioCapturerChangeInfos_[0]->muted);
    EXPECT_FALSE(collector.audioCapturerChangeInfos_[1]->muted);
    // Test updating mute status for all UIDs (uid = 0)
    EXPECT_EQ(SUCCESS, collector.UpdateCapturerInfoMuteStatus(0, false));
    EXPECT_FALSE(collector.audioCapturerChangeInfos_[0]->muted);
    EXPECT_FALSE(collector.audioCapturerChangeInfos_[1]->muted);
    // Test updating mute status for a non-existent UID
    EXPECT_EQ(SUCCESS, collector.UpdateCapturerInfoMuteStatus(3000, true));
    EXPECT_FALSE(collector.audioCapturerChangeInfos_[0]->muted);
    EXPECT_FALSE(collector.audioCapturerChangeInfos_[1]->muted);
    // Test updating mute status with nullptr audioPolicyServerHandler_
    collector.audioPolicyServerHandler_ = nullptr;
    EXPECT_EQ(SUCCESS, collector.UpdateCapturerInfoMuteStatus(1000, true));
    EXPECT_TRUE(collector.audioCapturerChangeInfos_[0]->muted);
    EXPECT_FALSE(collector.audioCapturerChangeInfos_[1]->muted);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: AudioStreamCollector_010
* @tc.desc  : Test UpdateTracker.
*/
HWTEST_F(AudioStreamCollectorUnitTest, AudioStreamCollector_010, TestSize.Level1)
{
    AudioMode audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    AudioDeviceDescriptor audioDev(AudioDeviceDescriptor::DEVICE_INFO);
    AudioStreamCollector collector;

    int32_t ret = collector.UpdateTracker(audioMode, audioDev);
    EXPECT_EQ(SUCCESS, ret);

    audioMode = AudioMode::AUDIO_MODE_RECORD;
    ret = collector.UpdateTracker(audioMode, audioDev);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: AudioStreamCollector_011
* @tc.desc  : Test UpdateTracker.
*/
HWTEST_F(AudioStreamCollectorUnitTest, AudioStreamCollector_011, TestSize.Level1)
{
    AudioMode audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    AudioStreamChangeInfo streamChangeInfo;
    streamChangeInfo.audioRendererChangeInfo.clientUID = 1001;
    streamChangeInfo.audioRendererChangeInfo.sessionId = 2001;
    streamChangeInfo.audioRendererChangeInfo.rendererState = RendererState::RENDERER_RUNNING;
    sptr<IRemoteObject> clientTrackerObj = nullptr;
    AudioStreamCollector collector;

    int32_t ret = collector.RegisterTracker(audioMode, streamChangeInfo, clientTrackerObj);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
    ret = collector.UpdateTracker(audioMode, streamChangeInfo);
    EXPECT_EQ(SUCCESS, ret);

    audioMode = AudioMode::AUDIO_MODE_RECORD;
    ret = collector.RegisterTracker(audioMode, streamChangeInfo, clientTrackerObj);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
    ret = collector.UpdateTracker(audioMode, streamChangeInfo);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: AudioStreamCollector_012
* @tc.desc  : Test UnsetOffloadMode.
*/
HWTEST_F(AudioStreamCollectorUnitTest, AudioStreamCollector_012, TestSize.Level1)
{
    AudioStreamCollector collector;
    int32_t streamId = -1;
    AudioStreamChangeInfo streamChangeInfo;
    streamChangeInfo.audioRendererChangeInfo.clientUID = 1001;
    streamChangeInfo.audioRendererChangeInfo.sessionId = 2001;
    streamChangeInfo.audioRendererChangeInfo.rendererState = RendererState::RENDERER_RUNNING;
    sptr<IRemoteObject> object;
    sptr<IStandardClientTracker> listener = iface_cast<IStandardClientTracker>(object);
    std::shared_ptr<AudioClientTracker> callback = std::make_shared<ClientTrackerCallbackListener>(listener);
    int32_t clientId = streamChangeInfo.audioRendererChangeInfo.sessionId;

    collector.clientTracker_[clientId] = callback;

    int32_t ret = collector.UnsetOffloadMode(streamId);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);

    streamId = 2001;
    ret = collector.UnsetOffloadMode(streamId);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: AudioStreamCollector_013
* @tc.desc  : Test ExistStreamForPipe.
*/
HWTEST_F(AudioStreamCollectorUnitTest, AudioStreamCollector_013, TestSize.Level1)
{
    AudioStreamCollector collector;
    AudioPipeType pipeType = PIPE_TYPE_UNKNOWN;
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    AudioStreamChangeInfo streamChangeInfo;
    streamChangeInfo.audioRendererChangeInfo.clientUID = 1001;
    streamChangeInfo.audioRendererChangeInfo.sessionId = 2001;
    streamChangeInfo.audioRendererChangeInfo.rendererState = RendererState::RENDERER_RUNNING;

    bool result = collector.ExistStreamForPipe(pipeType);
    EXPECT_FALSE(result);

    rendererChangeInfo->createrUID = streamChangeInfo.audioRendererChangeInfo.createrUID;
    rendererChangeInfo->clientUID = streamChangeInfo.audioRendererChangeInfo.clientUID;
    collector.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));
    collector.audioRendererChangeInfos_[0]->rendererInfo.pipeType = PIPE_TYPE_MULTICHANNEL;

    pipeType = PIPE_TYPE_MULTICHANNEL;
    result = collector.ExistStreamForPipe(pipeType);
    EXPECT_TRUE(result);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: AudioStreamCollector_014
* @tc.desc  : Test GetLastestRunningCallStreamUsage.
*/
HWTEST_F(AudioStreamCollectorUnitTest, AudioStreamCollector_014, TestSize.Level1)
{
    AudioStreamCollector collector;
    StreamUsage callStreamUsage = collector.GetLastestRunningCallStreamUsage();
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();

    EXPECT_EQ(STREAM_USAGE_UNKNOWN, callStreamUsage);

    rendererChangeInfo->rendererInfo.streamUsage = STREAM_USAGE_VOICE_MODEM_COMMUNICATION;
    rendererChangeInfo->rendererState = RENDERER_PREPARED;
    collector.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));

    callStreamUsage = collector.GetLastestRunningCallStreamUsage();
    EXPECT_EQ(STREAM_USAGE_VOICE_MODEM_COMMUNICATION, callStreamUsage);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: AudioStreamCollector_015
* @tc.desc  : Test RegisteredTrackerClientDied.
*/
HWTEST_F(AudioStreamCollectorUnitTest, AudioStreamCollector_015, TestSize.Level1)
{
    AudioStreamCollector collector;
    int32_t uid = 1001;
    StreamUsage callStreamUsage = collector.GetLastestRunningCallStreamUsage();
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();

    rendererChangeInfo->clientUID = 1001;
    rendererChangeInfo->createrUID = 1001;
    rendererChangeInfo->sessionId = 2001;
    collector.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));
    EXPECT_NO_THROW(
        collector.RegisteredTrackerClientDied(uid);
    );
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: AudioStreamCollector_016
* @tc.desc  : Test GetCapturerStreamInfo.
*/
HWTEST_F(AudioStreamCollectorUnitTest, AudioStreamCollector_016, TestSize.Level1)
{
    AudioStreamCollector collector;
    AudioStreamChangeInfo streamChangeInfo;
    streamChangeInfo.audioCapturerChangeInfo.clientUID = 1001;
    streamChangeInfo.audioCapturerChangeInfo.sessionId = 2001;
    AudioCapturerChangeInfo capturerChangeInfo;
    shared_ptr<AudioCapturerChangeInfo> rendererChangeInfo = make_shared<AudioCapturerChangeInfo>();

    rendererChangeInfo->clientUID = 1001;
    rendererChangeInfo->createrUID = 1001;
    rendererChangeInfo->sessionId = 2001;
    collector.audioCapturerChangeInfos_.push_back(move(rendererChangeInfo));

    EXPECT_NO_THROW(
        collector.GetCapturerStreamInfo(streamChangeInfo, capturerChangeInfo);
    );
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: AudioStreamCollector_017
* @tc.desc  : Test GetRendererStreamInfo.
*/
HWTEST_F(AudioStreamCollectorUnitTest, AudioStreamCollector_017, TestSize.Level1)
{
    AudioStreamCollector collector;
    AudioStreamChangeInfo streamChangeInfo;
    streamChangeInfo.audioCapturerChangeInfo.clientUID = 1001;
    streamChangeInfo.audioCapturerChangeInfo.sessionId = 2001;
    AudioRendererChangeInfo rendererInfo;
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();

    rendererChangeInfo->clientUID = 1001;
    rendererChangeInfo->createrUID = 1001;
    rendererChangeInfo->sessionId = 2001;
    collector.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));
    EXPECT_NO_THROW(
        collector.GetRendererStreamInfo(streamChangeInfo, rendererInfo);
    );
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: AudioStreamCollector_018
* @tc.desc  : Test CheckRendererInfoChanged.
*/
HWTEST_F(AudioStreamCollectorUnitTest, AudioStreamCollector_018, TestSize.Level1)
{
    AudioStreamCollector collector;
    AudioStreamChangeInfo streamChangeInfo;
    streamChangeInfo.audioRendererChangeInfo.clientUID = 1001;
    streamChangeInfo.audioRendererChangeInfo.sessionId = 2001;
    streamChangeInfo.audioRendererChangeInfo.rendererState = RendererState::RENDERER_RUNNING;
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();

    bool result = collector.CheckRendererInfoChanged(streamChangeInfo);
    EXPECT_TRUE(result);

    rendererChangeInfo->clientUID = 1001;
    rendererChangeInfo->createrUID = 1001;
    rendererChangeInfo->sessionId = 2001;
    rendererChangeInfo->rendererInfo.pipeType = PIPE_TYPE_MULTICHANNEL;
    collector.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));

    result = collector.CheckRendererInfoChanged(streamChangeInfo);
    EXPECT_TRUE(result);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: AudioStreamCollector_019
* @tc.desc  : Test SetRendererStreamParam.
*/
HWTEST_F(AudioStreamCollectorUnitTest, AudioStreamCollector_019, TestSize.Level1)
{
    AudioStreamCollector collector;
    AudioStreamChangeInfo streamChangeInfo;
    streamChangeInfo.audioRendererChangeInfo.clientUID = 1001;
    streamChangeInfo.audioRendererChangeInfo.sessionId = 2001;
    streamChangeInfo.audioRendererChangeInfo.rendererState = RendererState::RENDERER_RUNNING;
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();

    EXPECT_NO_THROW(
        collector.SetRendererStreamParam(streamChangeInfo, rendererChangeInfo);
    );
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: AudioStreamCollector_020
* @tc.desc  : Test UpdateRendererStreamInternal.
*/
HWTEST_F(AudioStreamCollectorUnitTest, AudioStreamCollector_020, TestSize.Level1)
{
    AudioStreamCollector collector;
    AudioStreamChangeInfo streamChangeInfo;
    streamChangeInfo.audioRendererChangeInfo.clientUID = 1001;
    streamChangeInfo.audioRendererChangeInfo.sessionId = 2001;
    streamChangeInfo.audioRendererChangeInfo.rendererState = RendererState::RENDERER_RUNNING;
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();

    int32_t ret = collector.UpdateRendererStreamInternal(streamChangeInfo);
    EXPECT_EQ(ERROR, ret);

    rendererChangeInfo->clientUID = 1001;
    rendererChangeInfo->createrUID = 1001;
    rendererChangeInfo->sessionId = 2001;
    rendererChangeInfo->rendererInfo.pipeType = PIPE_TYPE_MULTICHANNEL;
    collector.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));

    ret = collector.UpdateRendererStreamInternal(streamChangeInfo);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: AudioStreamCollector_021
* @tc.desc  : Test GetPipeType.
*/
HWTEST_F(AudioStreamCollectorUnitTest, AudioStreamCollector_021, TestSize.Level1)
{
    AudioStreamCollector collector;
    int32_t sessionId = -1;
    AudioPipeType pipeType = PIPE_TYPE_UNKNOWN;
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();

    int32_t ret = collector.GetPipeType(sessionId, pipeType);
    EXPECT_EQ(ERROR, ret);

    sessionId = 2001;
    rendererChangeInfo->clientUID = 1001;
    rendererChangeInfo->createrUID = 1001;
    rendererChangeInfo->sessionId = 2001;
    rendererChangeInfo->rendererInfo.pipeType = PIPE_TYPE_MULTICHANNEL;
    collector.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));

    ret = collector.GetPipeType(sessionId, pipeType);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: AudioStreamCollector_022
* @tc.desc  : Test GetRendererDeviceInfo.
*/
HWTEST_F(AudioStreamCollectorUnitTest, AudioStreamCollector_022, TestSize.Level1)
{
    AudioStreamCollector collector;
    int32_t sessionId = -1;
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();

    int32_t ret = collector.GetRendererDeviceInfo(sessionId, deviceInfo);
    EXPECT_EQ(ERROR, ret);

    sessionId = 2001;
    rendererChangeInfo->clientUID = 1001;
    rendererChangeInfo->createrUID = 1001;
    rendererChangeInfo->sessionId = 2001;
    rendererChangeInfo->rendererInfo.pipeType = PIPE_TYPE_MULTICHANNEL;
    collector.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));

    ret = collector.GetRendererDeviceInfo(sessionId, deviceInfo);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: AudioStreamCollector_023
* @tc.desc  : Test UpdateRendererPipeInfo.
*/
HWTEST_F(AudioStreamCollectorUnitTest, AudioStreamCollector_023, TestSize.Level1)
{
    AudioStreamCollector collector;
    int32_t sessionId = 2001;
    AudioPipeType normalPipe = PIPE_TYPE_NORMAL_OUT;
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();

    rendererChangeInfo->clientUID = 1001;
    rendererChangeInfo->createrUID = 1001;
    rendererChangeInfo->sessionId = 2001;
    rendererChangeInfo->rendererInfo.pipeType = PIPE_TYPE_MULTICHANNEL;
    collector.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));

    int32_t ret = collector.UpdateRendererPipeInfo(sessionId, normalPipe);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: AudioStreamCollector_024
* @tc.desc  : Test GetLowPowerVolume.
*/
HWTEST_F(AudioStreamCollectorUnitTest, AudioStreamCollector_024, TestSize.Level1)
{
    int32_t streamId = -1;
    AudioStreamCollector collector;

    float ret = collector.GetLowPowerVolume(streamId);
    EXPECT_EQ(1.0, ret);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: AudioStreamCollector_025
* @tc.desc  : Test SetOffloadMode.
*/
HWTEST_F(AudioStreamCollectorUnitTest, AudioStreamCollector_025, TestSize.Level1)
{
    int32_t streamId = -1;
    int32_t state = 0;
    bool isAppBack = false;
    AudioStreamCollector collector;

    int32_t ret = collector.SetOffloadMode(streamId, state, isAppBack);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: AudioStreamCollector_026
* @tc.desc  : Test GetSingleStreamVolume.
*/
HWTEST_F(AudioStreamCollectorUnitTest, AudioStreamCollector_026, TestSize.Level1)
{
    int32_t streamId = -1;
    AudioStreamCollector collector;

    float ret = collector.GetSingleStreamVolume(streamId);
    EXPECT_EQ(1.0, ret);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: AudioStreamCollector_027
* @tc.desc  : Test IsCallStreamUsage.
*/
HWTEST_F(AudioStreamCollectorUnitTest, AudioStreamCollector_027, TestSize.Level1)
{
    StreamUsage usage = STREAM_USAGE_VOICE_COMMUNICATION;
    AudioStreamCollector collector;

    bool ret = collector.IsCallStreamUsage(usage);
    EXPECT_TRUE(ret);

    usage = STREAM_USAGE_UNKNOWN;
    ret = collector.IsCallStreamUsage(usage);
    EXPECT_FALSE(ret);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: AudioStreamCollector_028
* @tc.desc  : Test SetCapturerStreamParam.
*/
HWTEST_F(AudioStreamCollectorUnitTest, AudioStreamCollector_028, TestSize.Level1)
{
    AudioStreamCollector collector;
    AudioStreamChangeInfo streamChangeInfo;
    streamChangeInfo.audioRendererChangeInfo.clientUID = 1001;
    streamChangeInfo.audioRendererChangeInfo.sessionId = 2001;
    streamChangeInfo.audioRendererChangeInfo.rendererState = RendererState::RENDERER_RUNNING;
    shared_ptr<AudioCapturerChangeInfo> rendererChangeInfo = make_shared<AudioCapturerChangeInfo>();

    EXPECT_NO_THROW(
        collector.SetCapturerStreamParam(streamChangeInfo, rendererChangeInfo);
    );
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: AudioStreamCollector_029
* @tc.desc  : Test ResetRendererStreamDeviceInfo.
*/
HWTEST_F(AudioStreamCollectorUnitTest, AudioStreamCollector_029, TestSize.Level1)
{
    AudioStreamCollector collector;
    AudioDeviceDescriptor outputDeviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();

    rendererChangeInfo->clientUID = 1001;
    rendererChangeInfo->createrUID = 1001;
    rendererChangeInfo->sessionId = 2001;
    rendererChangeInfo->rendererInfo.pipeType = PIPE_TYPE_MULTICHANNEL;
    collector.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));

    EXPECT_NO_THROW(
        collector.ResetRendererStreamDeviceInfo(outputDeviceInfo);
    );
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: AudioStreamCollector_030
* @tc.desc  : Test ResetCapturerStreamDeviceInfo.
*/
HWTEST_F(AudioStreamCollectorUnitTest, AudioStreamCollector_030, TestSize.Level1)
{
    AudioStreamCollector collector;
    AudioDeviceDescriptor outputDeviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    shared_ptr<AudioCapturerChangeInfo> rendererChangeInfo = make_shared<AudioCapturerChangeInfo>();

    rendererChangeInfo->clientUID = 1001;
    rendererChangeInfo->createrUID = 1001;
    rendererChangeInfo->sessionId = 2001;
    collector.audioCapturerChangeInfos_.push_back(move(rendererChangeInfo));

    EXPECT_NO_THROW(
        collector.ResetCapturerStreamDeviceInfo(outputDeviceInfo);
    );
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: AudioStreamCollector_031
* @tc.desc  : Test UpdateRendererDeviceInfo.
*/
HWTEST_F(AudioStreamCollectorUnitTest, AudioStreamCollector_031, TestSize.Level1)
{
    AudioStreamCollector collector;
    AudioDeviceDescriptor outputDeviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    int32_t clientUID = 1001;
    int32_t sessionId = 2001;

    rendererChangeInfo->clientUID = 1001;
    rendererChangeInfo->createrUID = 1001;
    rendererChangeInfo->sessionId = 2001;
    rendererChangeInfo->outputDeviceInfo = outputDeviceInfo;
    collector.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));

    int32_t ret = collector.UpdateRendererDeviceInfo(clientUID, sessionId, outputDeviceInfo);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: AudioStreamCollector_032
* @tc.desc  : Test UpdateCapturerDeviceInfo.
*/
HWTEST_F(AudioStreamCollectorUnitTest, AudioStreamCollector_032, TestSize.Level1)
{
    AudioStreamCollector collector;
    AudioDeviceDescriptor outputDeviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    shared_ptr<AudioCapturerChangeInfo> rendererChangeInfo = make_shared<AudioCapturerChangeInfo>();
    int32_t clientUID = 1001;
    int32_t sessionId = 2001;

    rendererChangeInfo->clientUID = 1001;
    rendererChangeInfo->createrUID = 1001;
    rendererChangeInfo->sessionId = 2001;
    rendererChangeInfo->inputDeviceInfo = outputDeviceInfo;
    collector.audioCapturerChangeInfos_.push_back(move(rendererChangeInfo));

    int32_t ret = collector.UpdateCapturerDeviceInfo(clientUID, sessionId, outputDeviceInfo);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: AudioStreamCollector_033
* @tc.desc  : Test UpdateTrackerInternal.
*/
HWTEST_F(AudioStreamCollectorUnitTest, AudioStreamCollector_033, TestSize.Level1)
{
    AudioStreamCollector audioStreamCollector;
    AudioMode mode = AUDIO_MODE_PLAYBACK;
    AudioStreamChangeInfo streamChangeInfo;
    streamChangeInfo.audioRendererChangeInfo.clientUID = 1001;
    streamChangeInfo.audioRendererChangeInfo.sessionId = 2001;

    int32_t ret = audioStreamCollector.UpdateTrackerInternal(mode, streamChangeInfo);
    EXPECT_EQ(ERROR, ret);

    mode = AUDIO_MODE_RECORD;
    ret = audioStreamCollector.UpdateTrackerInternal(mode, streamChangeInfo);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: AudioStreamCollector_034
* @tc.desc  : Test GetStreamType.
*/
HWTEST_F(AudioStreamCollectorUnitTest, AudioStreamCollector_034, TestSize.Level1)
{
    AudioStreamCollector audioStreamCollector;
    ContentType contentType = CONTENT_TYPE_UNKNOWN;
    StreamUsage streamUsage = StreamUsage::STREAM_USAGE_MEDIA;

    AudioStreamType streamType = audioStreamCollector.GetStreamType(contentType, streamUsage);
    EXPECT_EQ(STREAM_MUSIC, streamType);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: AudioStreamCollector_035
* @tc.desc  : Test IsOffloadAllowed.
*/
HWTEST_F(AudioStreamCollectorUnitTest, AudioStreamCollector_035, TestSize.Level1)
{
    AudioStreamCollector collector;
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    int32_t sessionId = -1;

    bool ret = collector.IsOffloadAllowed(sessionId);
    EXPECT_FALSE(ret);

    sessionId = 2001;
    rendererChangeInfo->createrUID = 1001;
    rendererChangeInfo->clientUID = 1001;
    rendererChangeInfo->sessionId = 2001;
    collector.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));

    ret = collector.IsOffloadAllowed(sessionId);
    EXPECT_TRUE(ret);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: AudioStreamCollector_036
* @tc.desc  : Test GetChannelCount.
*/
HWTEST_F(AudioStreamCollectorUnitTest, AudioStreamCollector_036, TestSize.Level1)
{
    AudioStreamCollector collector;
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    int32_t sessionId = -1;

    int32_t ret = collector.GetChannelCount(sessionId);
    EXPECT_EQ(0, ret);

    sessionId = 2001;
    rendererChangeInfo->createrUID = 1001;
    rendererChangeInfo->clientUID = 1001;
    rendererChangeInfo->sessionId = 2001;
    collector.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));

    ret = collector.GetChannelCount(sessionId);
    EXPECT_EQ(0, ret);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: AudioStreamCollector_037
* @tc.desc  : Test GetCurrentRendererChangeInfos.
*/
HWTEST_F(AudioStreamCollectorUnitTest, AudioStreamCollector_037, TestSize.Level1)
{
    AudioStreamCollector collector;
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    std::vector<shared_ptr<AudioRendererChangeInfo>> rendererChangeInfos;

    rendererChangeInfo->createrUID = 1001;
    rendererChangeInfo->clientUID = 1001;
    rendererChangeInfo->sessionId = 2001;
    collector.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));

    int32_t ret = collector.GetCurrentRendererChangeInfos(rendererChangeInfos);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: AudioStreamCollector_038
* @tc.desc  : Test GetCurrentCapturerChangeInfos.
*/
HWTEST_F(AudioStreamCollectorUnitTest, AudioStreamCollector_038, TestSize.Level1)
{
    AudioStreamCollector collector;
    shared_ptr<AudioCapturerChangeInfo> rendererChangeInfo = make_shared<AudioCapturerChangeInfo>();
    std::vector<shared_ptr<AudioCapturerChangeInfo>> rendererChangeInfos;

    rendererChangeInfo->createrUID = 1001;
    rendererChangeInfo->clientUID = 1001;
    rendererChangeInfo->sessionId = 2001;
    collector.audioCapturerChangeInfos_.push_back(move(rendererChangeInfo));

    int32_t ret = collector.GetCurrentCapturerChangeInfos(rendererChangeInfos);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: AudioStreamCollector_039
* @tc.desc  : Test GetUid.
*/
HWTEST_F(AudioStreamCollectorUnitTest, AudioStreamCollector_039, TestSize.Level1)
{
    AudioStreamCollector collector;
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    int32_t sessionId = -1;

    rendererChangeInfo->createrUID = 1001;
    rendererChangeInfo->clientUID = 1001;
    rendererChangeInfo->sessionId = 2001;
    collector.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));

    int32_t ret = collector.GetUid(sessionId);
    EXPECT_EQ(-1, ret);

    sessionId = 2001;
    ret = collector.GetUid(sessionId);
    EXPECT_EQ(1001, ret);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: AudioStreamCollector_040
* @tc.desc  : Test IsStreamActive.
*/
HWTEST_F(AudioStreamCollectorUnitTest, AudioStreamCollector_040, TestSize.Level1)
{
    AudioStreamCollector collector;
    AudioStreamType volumeType = STREAM_MUSIC;
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();

    rendererChangeInfo->createrUID = 1001;
    rendererChangeInfo->clientUID = 1001;
    rendererChangeInfo->sessionId = 2001;
    rendererChangeInfo->rendererState = RENDERER_RUNNING;
    collector.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));

    bool ret = collector.IsStreamActive(volumeType);
    EXPECT_TRUE(ret);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: AudioStreamCollector_041
* @tc.desc  : Test ActivateAudioConcurrency.
*/
HWTEST_F(AudioStreamCollectorUnitTest, AudioStreamCollector_041, TestSize.Level1)
{
    AudioStreamCollector collector;
    AudioPipeType pipeType = PIPE_TYPE_UNKNOWN;

    int32_t ret = collector.ActivateAudioConcurrency(pipeType);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: AudioStreamCollector_042
* @tc.desc  : Test GetAllRendererSessionIDForUID.
*/
HWTEST_F(AudioStreamCollectorUnitTest, AudioStreamCollector_042, TestSize.Level1)
{
    AudioStreamCollector collector;
    int32_t uid = 1001;
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();

    rendererChangeInfo->clientUID = 1001;
    rendererChangeInfo->createrUID = 1001;
    rendererChangeInfo->sessionId = 2001;
    collector.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));

    EXPECT_NO_THROW(
        collector.GetAllRendererSessionIDForUID(uid);
    );
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: AudioStreamCollector_043
* @tc.desc  : Test ChangeVoipCapturerStreamToNormal.
*/
HWTEST_F(AudioStreamCollectorUnitTest, AudioStreamCollector_043, TestSize.Level1)
{
    AudioStreamCollector collector;
    shared_ptr<AudioCapturerChangeInfo> rendererChangeInfo = make_shared<AudioCapturerChangeInfo>();
    shared_ptr<AudioCapturerChangeInfo> rendererChangeInfo1 = make_shared<AudioCapturerChangeInfo>();

    rendererChangeInfo->clientUID = 1001;
    rendererChangeInfo->createrUID = 1001;
    rendererChangeInfo->sessionId = 2001;
    rendererChangeInfo->capturerInfo.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    collector.audioCapturerChangeInfos_.push_back(move(rendererChangeInfo));

    rendererChangeInfo1->clientUID = 1001;
    rendererChangeInfo1->createrUID = 1001;
    rendererChangeInfo1->sessionId = 2001;
    rendererChangeInfo1->capturerInfo.sourceType = SOURCE_TYPE_VOICE_MESSAGE;
    collector.audioCapturerChangeInfos_.push_back(move(rendererChangeInfo1));

    bool ret = collector.ChangeVoipCapturerStreamToNormal();
    EXPECT_TRUE(ret);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: AudioStreamCollector_044
* @tc.desc  : Test HasVoipRendererStream.
*/
HWTEST_F(AudioStreamCollectorUnitTest, AudioStreamCollector_044, TestSize.Level1)
{
    AudioStreamCollector collector;
    int32_t uid = 1001;
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();

    rendererChangeInfo->clientUID = 1001;
    rendererChangeInfo->createrUID = 1001;
    rendererChangeInfo->sessionId = 2001;
    rendererChangeInfo->rendererInfo.originalFlag = AUDIO_FLAG_VOIP_FAST;
    collector.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));

    bool ret = collector.HasVoipRendererStream();
    EXPECT_TRUE(ret);
}

/**
* @tc.name  : IsMediaPlaying_Test01
* @tc.number: AudioStreamCollectorUnitTest_IsMediaPlaying_Test01
* @tc.desc  : Test IsMediaPlaying function when there is at least one media renderer running.
*/
HWTEST_F(AudioStreamCollectorUnitTest, IsMediaPlaying_Test01, TestSize.Level1)
{
    std::unique_ptr<AudioStreamCollector> collector = std::make_unique<AudioStreamCollector>();
    AudioRendererInfo rendererInfo1 = {
        .contentType = CONTENT_TYPE_MUSIC, .streamUsage = STREAM_USAGE_MEDIA
    };
    std::unique_ptr<AudioRendererChangeInfo> info1 = std::make_unique<AudioRendererChangeInfo>();
    info1->sessionId = 1;
    info1->rendererState = RENDERER_PAUSED;
    info1->rendererInfo = rendererInfo1;
    info1->channelCount = 2;
    collector->audioRendererChangeInfos_.push_back(std::move(info1));
    bool result = collector->IsMediaPlaying();
    EXPECT_FALSE(result);

    AudioRendererInfo rendererInfo2 = {
        .contentType = CONTENT_TYPE_SPEECH, .streamUsage = STREAM_USAGE_VOICE_COMMUNICATION
    };
    std::unique_ptr<AudioRendererChangeInfo> info2 = std::make_unique<AudioRendererChangeInfo>();
    info2->sessionId = 2;
    info2->rendererState = RENDERER_RUNNING;
    info2->rendererInfo = rendererInfo2;
    info2->channelCount = 1;
    collector->audioRendererChangeInfos_.push_back(std::move(info2));
    result = collector->IsMediaPlaying();
    EXPECT_FALSE(result);

    AudioRendererInfo rendererInfo3 = {
        .contentType = CONTENT_TYPE_MOVIE, .streamUsage = STREAM_USAGE_MEDIA
    };
    std::unique_ptr<AudioRendererChangeInfo> info3 = std::make_unique<AudioRendererChangeInfo>();
    info3->sessionId = 3;
    info3->rendererState = RENDERER_RUNNING;
    info3->rendererInfo = rendererInfo3;
    info3->channelCount = 2;
    collector->audioRendererChangeInfos_.push_back(std::move(info3));
    result = collector->IsMediaPlaying();
    EXPECT_TRUE(result);
}
} // namespace AudioStandard
} // namespace OHOS
