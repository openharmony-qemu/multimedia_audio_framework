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
#include <gmock/gmock.h>
#include "stream_dfx_manager.h"
#include "audio_stream_info.h"
#include "audio_source_type.h"
#include "audio_info.h"
#include "audio_capturer_dfx_collector.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {

class StreamDfxManagerUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void StreamDfxManagerUnitTest::SetUpTestCase(void) {}

void StreamDfxManagerUnitTest::TearDownTestCase(void) {}

void StreamDfxManagerUnitTest::SetUp() {}

void StreamDfxManagerUnitTest::TearDown() {}

/**
 * @tc.name  : Test IsInterphoneStream
 * @tc.number: IsInterphoneStream_VOIP_Playback_001
 * @tc.desc  : Test IsInterphoneStream for VOIP playback stream (VOICE_COMMUNICATION)
 */
HWTEST(StreamDfxManagerUnitTest, IsInterphoneStream_VOIP_Playback_001, TestSize.Level1)
{
    StreamDfxManager &manager = StreamDfxManager::GetInstance();
    AudioProcessConfig processConfig;
    
    processConfig.audioMode = AUDIO_MODE_PLAYBACK;
    processConfig.rendererInfo.streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;
    
    bool result = manager.IsInterphoneStream(processConfig);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test IsInterphoneStream
 * @tc.number: IsInterphoneStream_VOIP_VideoPlayback_001
 * @tc.desc  : Test IsInterphoneStream for VOIP video playback stream (VIDEO_COMMUNICATION)
 */
HWTEST(StreamDfxManagerUnitTest, IsInterphoneStream_VOIP_VideoPlayback_001, TestSize.Level1)
{
    StreamDfxManager &manager = StreamDfxManager::GetInstance();
    AudioProcessConfig processConfig;
    
    processConfig.audioMode = AUDIO_MODE_PLAYBACK;
    processConfig.rendererInfo.streamUsage = STREAM_USAGE_VIDEO_COMMUNICATION;
    
    bool result = manager.IsInterphoneStream(processConfig);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test IsInterphoneStream
 * @tc.number: IsInterphoneStream_VOIP_Capture_001
 * @tc.desc  : Test IsInterphoneStream for VOIP capture stream (VOICE_COMMUNICATION)
 */
HWTEST(StreamDfxManagerUnitTest, IsInterphoneStream_VOIP_Capture_001, TestSize.Level1)
{
    StreamDfxManager &manager = StreamDfxManager::GetInstance();
    AudioProcessConfig processConfig;
    
    processConfig.audioMode = AUDIO_MODE_RECORD;
    processConfig.capturerInfo.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    
    bool result = manager.IsInterphoneStream(processConfig);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test IsInterphoneStream
 * @tc.number: IsInterphoneStream_Interphone_Playback_001
 * @tc.desc  : Test IsInterphoneStream for interphone playback stream
 */
HWTEST(StreamDfxManagerUnitTest, IsInterphoneStream_Interphone_Playback_001, TestSize.Level1)
{
    StreamDfxManager &manager = StreamDfxManager::GetInstance();
    AudioProcessConfig processConfig;
    
    processConfig.audioMode = AUDIO_MODE_PLAYBACK;
    processConfig.rendererInfo.streamUsage = STREAM_USAGE_INTERPHONE;
    
    bool result = manager.IsInterphoneStream(processConfig);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name  : Test IsInterphoneStream
 * @tc.number: IsInterphoneStream_Interphone_Capture_001
 * @tc.desc  : Test IsInterphoneStream for interphone capture stream
 */
HWTEST(StreamDfxManagerUnitTest, IsInterphoneStream_Interphone_Capture_001, TestSize.Level1)
{
    StreamDfxManager &manager = StreamDfxManager::GetInstance();
    AudioProcessConfig processConfig;
    
    processConfig.audioMode = AUDIO_MODE_RECORD;
    processConfig.capturerInfo.sourceType = SOURCE_TYPE_INTERPHONE;
    
    bool result = manager.IsInterphoneStream(processConfig);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name  : Test IsInterphoneStream
 * @tc.number: IsInterphoneStream_Music_001
 * @tc.desc  : Test IsInterphoneStream for music stream
 */
HWTEST(StreamDfxManagerUnitTest, IsInterphoneStream_Music_001, TestSize.Level1)
{
    StreamDfxManager &manager = StreamDfxManager::GetInstance();
    AudioProcessConfig processConfig;
    
    processConfig.audioMode = AUDIO_MODE_PLAYBACK;
    processConfig.rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    
    bool result = manager.IsInterphoneStream(processConfig);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test CheckStreamOccupancy
 * @tc.number: CheckStreamOccupancy_VOIP_Start_001
 * @tc.desc  : Test CheckStreamOccupancy for VOIP stream start
 */
HWTEST(StreamDfxManagerUnitTest, CheckStreamOccupancy_VOIP_Start_001, TestSize.Level1)
{
    StreamDfxManager &manager = StreamDfxManager::GetInstance();
    uint32_t sessionId = 1001;
    AudioProcessConfig processConfig;
    
    processConfig.audioMode = AUDIO_MODE_PLAYBACK;
    processConfig.rendererInfo.streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;
    processConfig.appInfo.appUid = 1000;
    
    manager.CheckStreamOccupancy(sessionId, processConfig, true);
    
    manager.CheckStreamOccupancy(sessionId, processConfig, false);
}

/**
 * @tc.name  : Test CheckStreamOccupancy
 * @tc.number: CheckStreamOccupancy_InterphoneCapture_Start_001
 * @tc.desc  : Test CheckStreamOccupancy for interphone capture stream start
 */
HWTEST(StreamDfxManagerUnitTest, CheckStreamOccupancy_InterphoneCapture_Start_001, TestSize.Level1)
{
    StreamDfxManager &manager = StreamDfxManager::GetInstance();
    uint32_t sessionId = 1002;
    AudioProcessConfig processConfig;
    
    processConfig.audioMode = AUDIO_MODE_RECORD;
    processConfig.capturerInfo.sourceType = SOURCE_TYPE_INTERPHONE;
    processConfig.appInfo.appUid = 1000;
    
    manager.CheckStreamOccupancy(sessionId, processConfig, true);
    
    manager.CheckStreamOccupancy(sessionId, processConfig, false);
}

/**
 * @tc.name  : Test CheckStreamOccupancy
 * @tc.number: CheckStreamOccupancy_InterphoneRender_Start_001
 * @tc.desc  : Test CheckStreamOccupancy for interphone render stream start
 */
HWTEST(StreamDfxManagerUnitTest, CheckStreamOccupancy_InterphoneRender_Start_001, TestSize.Level1)
{
    StreamDfxManager &manager = StreamDfxManager::GetInstance();
    uint32_t sessionId = 1003;
    AudioProcessConfig processConfig;
    
    processConfig.audioMode = AUDIO_MODE_PLAYBACK;
    processConfig.rendererInfo.streamUsage = STREAM_USAGE_INTERPHONE;
    processConfig.appInfo.appUid = 1000;
    
    manager.CheckStreamOccupancy(sessionId, processConfig, true);
    
    manager.CheckStreamOccupancy(sessionId, processConfig, false);
}

/**
 * @tc.name  : Test CheckStreamOccupancy
 * @tc.number: CheckStreamOccupancy_Stop_001
 * @tc.desc  : Test CheckStreamOccupancy for stream stop
 */
HWTEST(StreamDfxManagerUnitTest, CheckStreamOccupancy_Stop_001, TestSize.Level1)
{
    StreamDfxManager &manager = StreamDfxManager::GetInstance();
    uint32_t sessionId = 1004;
    AudioProcessConfig processConfig;
    
    processConfig.audioMode = AUDIO_MODE_PLAYBACK;
    processConfig.rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    
    manager.CheckStreamOccupancy(sessionId, processConfig, true);
    manager.CheckStreamOccupancy(sessionId, processConfig, false);
}

/**
 * @tc.name  : Test CheckStreamOccupancy
 * @tc.number: CheckStreamOccupancy_MultipleStreams_001
 * @tc.desc  : Test CheckStreamOccupancy for multiple streams
 */
HWTEST(StreamDfxManagerUnitTest, CheckStreamOccupancy_MultipleStreams_001, TestSize.Level1)
{
    StreamDfxManager &manager = StreamDfxManager::GetInstance();
    
    uint32_t sessionId1 = 2001;
    AudioProcessConfig config1;
    config1.audioMode = AUDIO_MODE_PLAYBACK;
    config1.rendererInfo.streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;
    
    uint32_t sessionId2 = 2002;
    AudioProcessConfig config2;
    config2.audioMode = AUDIO_MODE_RECORD;
    config2.capturerInfo.sourceType = SOURCE_TYPE_INTERPHONE;
    
    uint32_t sessionId3 = 2003;
    AudioProcessConfig config3;
    config3.audioMode = AUDIO_MODE_PLAYBACK;
    config3.rendererInfo.streamUsage = STREAM_USAGE_INTERPHONE;
    
    manager.CheckStreamOccupancy(sessionId1, config1, true);
    manager.CheckStreamOccupancy(sessionId2, config2, true);
    manager.CheckStreamOccupancy(sessionId3, config3, true);
    
    manager.CheckStreamOccupancy(sessionId1, config1, false);
    manager.CheckStreamOccupancy(sessionId2, config2, false);
    manager.CheckStreamOccupancy(sessionId3, config3, false);
}

/**
 * @tc.name  : Test GetInstance
 * @tc.number: GetInstance_001
 * @tc.desc  : Test GetInstance returns singleton
 */
HWTEST(StreamDfxManagerUnitTest, GetInstance_001, TestSize.Level1)
{
    StreamDfxManager &instance1 = StreamDfxManager::GetInstance();
    StreamDfxManager &instance2 = StreamDfxManager::GetInstance();
    
    EXPECT_EQ(&instance1, &instance2);
}

/**
 * @tc.name  : Test SendAudioErrorEvent
 * @tc.number: SendAudioErrorEvent_001
 * @tc.desc  : Test SendAudioErrorEvent method
 */
HWTEST(StreamDfxManagerUnitTest, SendAudioErrorEvent_001, TestSize.Level1)
{
    StreamDfxManager &manager = StreamDfxManager::GetInstance();
    int32_t uid = 1000;
    int32_t errorCode = 6800101;
    std::string erroDesc = "Test error";
    bool isClient = true;
    
    manager.SendAudioErrorEvent(uid, errorCode, erroDesc, isClient);
}

/**
 * @tc.name  : Test CheckStreamOccupancy timeout
 * @tc.number: CheckStreamOccupancy_InterphoneCaptureTimeout_001
 * @tc.desc  : Test CheckStreamOccupancy for interphone capture stream timeout reporting
 */
HWTEST(StreamDfxManagerUnitTest, CheckStreamOccupancy_InterphoneCaptureTimeout_001, TestSize.Level1)
{
    StreamDfxManager &manager = StreamDfxManager::GetInstance();
    uint32_t sessionId = 3001;
    AudioProcessConfig config;
    
    config.audioMode = AUDIO_MODE_RECORD;
    config.capturerInfo.sourceType = SOURCE_TYPE_INTERPHONE;
    config.appInfo.appUid = 1000;
    
    manager.CheckStreamOccupancy(sessionId, config, true);
    
    int64_t currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    auto it = manager.streamRecordMap_.find(sessionId);
    if (it != manager.streamRecordMap_.end()) {
        it->second.startTime = currentTime - 610000;
        it->second.lastUploadTime = currentTime - 610000;
    }
    
    AudioProcessConfig dummyConfig;
    dummyConfig.audioMode = AUDIO_MODE_PLAYBACK;
    dummyConfig.rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    manager.CheckStreamOccupancy(sessionId + 1, dummyConfig, true);
    
    manager.CheckStreamOccupancy(sessionId, config, false);
    manager.CheckStreamOccupancy(sessionId + 1, dummyConfig, false);
}

/**
 * @tc.name  : Test CheckStreamOccupancy timeout
 * @tc.number: CheckStreamOccupancy_InterphoneRenderTimeout_001
 * @tc.desc  : Test CheckStreamOccupancy for interphone render stream timeout reporting
 */
HWTEST(StreamDfxManagerUnitTest, CheckStreamOccupancy_InterphoneRenderTimeout_001, TestSize.Level1)
{
    StreamDfxManager &manager = StreamDfxManager::GetInstance();
    uint32_t sessionId = 3002;
    AudioProcessConfig config;
    
    config.audioMode = AUDIO_MODE_PLAYBACK;
    config.rendererInfo.streamUsage = STREAM_USAGE_INTERPHONE;
    config.appInfo.appUid = 1000;
    
    manager.CheckStreamOccupancy(sessionId, config, true);
    
    int64_t currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    auto it = manager.streamRecordMap_.find(sessionId);
    if (it != manager.streamRecordMap_.end()) {
        it->second.startTime = currentTime - 21610000;
        it->second.lastUploadTime = currentTime - 21610000;
    }
    
    AudioProcessConfig dummyConfig;
    dummyConfig.audioMode = AUDIO_MODE_PLAYBACK;
    dummyConfig.rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    manager.CheckStreamOccupancy(sessionId + 1, dummyConfig, true);
    
    manager.CheckStreamOccupancy(sessionId, config, false);
    manager.CheckStreamOccupancy(sessionId + 1, dummyConfig, false);
}

/**
 * @tc.name  : Test CheckStreamOccupancy timeout
 * @tc.number: CheckStreamOccupancy_VOIPTimeout_001
 * @tc.desc  : Test CheckStreamOccupancy for VOIP stream timeout reporting
 */
HWTEST(StreamDfxManagerUnitTest, CheckStreamOccupancy_VOIPTimeout_001, TestSize.Level1)
{
    StreamDfxManager &manager = StreamDfxManager::GetInstance();
    uint32_t sessionId = 3003;
    AudioProcessConfig config;
    
    config.audioMode = AUDIO_MODE_PLAYBACK;
    config.rendererInfo.streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;
    config.appInfo.appUid = 1000;
    
    manager.CheckStreamOccupancy(sessionId, config, true);
    
    int64_t currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    auto it = manager.streamRecordMap_.find(sessionId);
    if (it != manager.streamRecordMap_.end()) {
        it->second.startTime = currentTime - 7201000;
        it->second.lastUploadTime = currentTime - 7201000;
    }
    
    AudioProcessConfig dummyConfig;
    dummyConfig.audioMode = AUDIO_MODE_PLAYBACK;
    dummyConfig.rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    manager.CheckStreamOccupancy(sessionId + 1, dummyConfig, true);
    
    manager.CheckStreamOccupancy(sessionId, config, false);
    manager.CheckStreamOccupancy(sessionId + 1, dummyConfig, false);
}

/**
 * @tc.name  : Test CheckStreamOccupancy no timeout
 * @tc.number: CheckStreamOccupancy_NoTimeout_001
 * @tc.desc  : Test CheckStreamOccupancy for stream not timeout
 */
HWTEST(StreamDfxManagerUnitTest, CheckStreamOccupancy_NoTimeout_001, TestSize.Level1)
{
    StreamDfxManager &manager = StreamDfxManager::GetInstance();
    uint32_t sessionId = 3004;
    AudioProcessConfig config;
    
    config.audioMode = AUDIO_MODE_PLAYBACK;
    config.rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    config.appInfo.appUid = 1000;
    
    manager.CheckStreamOccupancy(sessionId, config, true);
    
    int64_t currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    auto it = manager.streamRecordMap_.find(sessionId);
    if (it != manager.streamRecordMap_.end()) {
        it->second.startTime = currentTime - 3600000;
        it->second.lastUploadTime = currentTime - 3600000;
    }
    
    AudioProcessConfig dummyConfig;
    dummyConfig.audioMode = AUDIO_MODE_PLAYBACK;
    dummyConfig.rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    manager.CheckStreamOccupancy(sessionId + 1, dummyConfig, true);
    
    manager.CheckStreamOccupancy(sessionId, config, false);
    manager.CheckStreamOccupancy(sessionId + 1, dummyConfig, false);
}

/**
 * @tc.name  : Test AudioCapturerDfxCollector FlushDfxMsg via invalid appUid.
 * @tc.number: AudioCapturerDfxCollector_FlushDfxMsg_001
 * @tc.desc  : Test FlushDfxMsg returns early when appUid is DFX_INVALID_APP_UID.
 */
HWTEST(StreamDfxManagerUnitTest, AudioCapturerDfxCollector_FlushDfxMsg_001, TestSize.Level1)
{
    AudioCapturerDfxCollector dfxCollector;
    uint32_t index = 0;
    int32_t invalidAppUid = -1;

    dfxCollector.FlushDfxMsg(index, invalidAppUid);
}

/**
 * @tc.name  : Test AudioCapturerDfxCollector FlushDfxMsg via valid appUid.
 * @tc.number: AudioCapturerDfxCollector_FlushDfxMsg_002
 * @tc.desc  : Test FlushDfxMsg processes messages when appUid is valid.
 */
HWTEST(StreamDfxManagerUnitTest, AudioCapturerDfxCollector_FlushDfxMsg_002, TestSize.Level1)
{
    AudioCapturerDfxCollector dfxCollector;
    uint32_t index = 0;
    int32_t validAppUid = 1000;

    dfxCollector.FlushDfxMsg(index, validAppUid);
}

} // AudioStandard
} // OHOS