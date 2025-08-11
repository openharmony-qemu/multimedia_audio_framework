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

#ifndef LOG_TAGS
#define LOG_TAGS "ProAudioStreamManagerUnitTest"
#endif

#include "pro_audio_stream_manager_unit_test.h"

#include <chrono>
#include <thread>

#include "audio_errors.h"
#include "pro_audio_stream_manager.h"
#include "policy_handler.h"

using namespace std;
using namespace std::chrono;
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
constexpr uint32_t MIDDLE_SESSIONID = 100001;
constexpr uint32_t MORE_SESSIONID = MAX_STREAMID + 1;
const int32_t MAP_NUM = 1;
const int32_t CAPTURER_FLAG = 10;
const uint32_t SESSIONID = 123456;
const uint32_t STREAMINDEX_ONE = 1;

void ProAudioStreamManagerUnitTest::SetUpTestCase(void) {}
void ProAudioStreamManagerUnitTest::TearDownTestCase(void) {}
void ProAudioStreamManagerUnitTest::SetUp(void) {}
void ProAudioStreamManagerUnitTest::TearDown(void) {}

static AudioProcessConfig GetInnerCapConfig()
{
    AudioProcessConfig config;
    config.appInfo.appUid = CAPTURER_FLAG;
    config.appInfo.appPid = CAPTURER_FLAG;
    config.streamInfo.format = SAMPLE_S32LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    return config;
}

/**
* @tc.name   : Test CreateRender API
* @tc.number : ProAudioStreamManager_001
* @tc.desc   : Test CreateRender interface.
*/
HWTEST(ProAudioStreamManagerUnitTest, ProAudioStreamManager_001, TestSize.Level1)
{
    shared_ptr<ProAudioStreamManager> audioStreamManager = make_shared<ProAudioStreamManager>(DIRECT_PLAYBACK);
    ASSERT_TRUE(audioStreamManager != nullptr);

    AudioProcessConfig config = GetInnerCapConfig();
    shared_ptr<IRendererStream> rendererStream = audioStreamManager->CreateRendererStream(config);
    ASSERT_TRUE(rendererStream != nullptr);

    int result = audioStreamManager->CreateRender(config, rendererStream);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test CreateRender API
* @tc.number : ProAudioStreamManager_002
* @tc.desc   : Test CreateRender interface.
*/
HWTEST(ProAudioStreamManagerUnitTest, ProAudioStreamManager_002, TestSize.Level1)
{
    shared_ptr<ProAudioStreamManager> audioStreamManager = make_shared<ProAudioStreamManager>(DIRECT_PLAYBACK);
    ASSERT_TRUE(audioStreamManager != nullptr);

    AudioProcessConfig config;
    config.appInfo.appUid = CAPTURER_FLAG;
    config.appInfo.appPid = CAPTURER_FLAG;
    config.streamInfo.format = SAMPLE_S32LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    config.originalSessionId = MORE_SESSIONID;
    shared_ptr<IRendererStream> rendererStream = audioStreamManager->CreateRendererStream(config);
    ASSERT_TRUE(rendererStream != nullptr);

    int result = audioStreamManager->CreateRender(config, rendererStream);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test CreateRender API
* @tc.number : ProAudioStreamManager_003
* @tc.desc   : Test CreateRender interface.
*/
HWTEST(ProAudioStreamManagerUnitTest, ProAudioStreamManager_003, TestSize.Level1)
{
    shared_ptr<ProAudioStreamManager> audioStreamManager = make_shared<ProAudioStreamManager>(DIRECT_PLAYBACK);
    ASSERT_TRUE(audioStreamManager != nullptr);

    AudioProcessConfig config;
    config.appInfo.appUid = CAPTURER_FLAG;
    config.appInfo.appPid = CAPTURER_FLAG;
    config.streamInfo.format = SAMPLE_S32LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    config.originalSessionId = MIDDLE_SESSIONID;
    shared_ptr<IRendererStream> rendererStream = audioStreamManager->CreateRendererStream(config);
    ASSERT_TRUE(rendererStream != nullptr);

    int result = audioStreamManager->CreateRender(config, rendererStream);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test ReleaseRender API
* @tc.number : ProAudioStreamManager_004
* @tc.desc   : Test ReleaseRender interface.
*/
HWTEST(ProAudioStreamManagerUnitTest, ProAudioStreamManager_004, TestSize.Level1)
{
    shared_ptr<ProAudioStreamManager> audioStreamManager = make_shared<ProAudioStreamManager>(DIRECT_PLAYBACK);
    ASSERT_TRUE(audioStreamManager != nullptr);

    AudioProcessConfig config;
    config.appInfo.appUid = CAPTURER_FLAG;
    config.appInfo.appPid = CAPTURER_FLAG;
    config.streamInfo.format = SAMPLE_S32LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    shared_ptr<IRendererStream> rendererStream = audioStreamManager->CreateRendererStream(config);
    ASSERT_TRUE(rendererStream != nullptr);

    int32_t rendererStreamMap = MAP_NUM;
    audioStreamManager->rendererStreamMap_.emplace(rendererStreamMap, rendererStream);

    uint32_t streamIndex = STREAMINDEX_ONE;
    int result = audioStreamManager->ReleaseRender(streamIndex);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test ReleaseRender API
* @tc.number : ProAudioStreamManager_005
* @tc.desc   : Test ReleaseRender interface.
*/
HWTEST(ProAudioStreamManagerUnitTest, ProAudioStreamManager_005, TestSize.Level1)
{
    shared_ptr<ProAudioStreamManager> audioStreamManager = make_shared<ProAudioStreamManager>(DIRECT_PLAYBACK);
    ASSERT_TRUE(audioStreamManager != nullptr);

    AudioProcessConfig config;
    config.appInfo.appUid = CAPTURER_FLAG;
    config.appInfo.appPid = CAPTURER_FLAG;
    config.streamInfo.format = SAMPLE_S32LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    shared_ptr<IRendererStream> rendererStream = audioStreamManager->CreateRendererStream(config);
    ASSERT_TRUE(rendererStream != nullptr);

    int32_t rendererStreamMap = 0;
    audioStreamManager->rendererStreamMap_.emplace(rendererStreamMap, rendererStream);

    uint32_t streamIndex = 0;
    int result = audioStreamManager->ReleaseRender(streamIndex);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test ReleaseRender API
* @tc.number : ProAudioStreamManager_006
* @tc.desc   : Test Start interface.
*/
HWTEST(ProAudioStreamManagerUnitTest, ProAudioStreamManager_006, TestSize.Level1)
{
    shared_ptr<ProAudioStreamManager> audioStreamManager = make_shared<ProAudioStreamManager>(DIRECT_PLAYBACK);
    ASSERT_TRUE(audioStreamManager != nullptr);

    AudioProcessConfig config;
    config.appInfo.appUid = CAPTURER_FLAG;
    config.appInfo.appPid = CAPTURER_FLAG;
    config.streamInfo.format = SAMPLE_S32LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    shared_ptr<IRendererStream> rendererStream = audioStreamManager->CreateRendererStream(config);
    ASSERT_TRUE(rendererStream != nullptr);

    int32_t rendererStreamMap = MAP_NUM;
    audioStreamManager->rendererStreamMap_.emplace(rendererStreamMap, rendererStream);
    audioStreamManager->playbackEngine_.reset();

    uint32_t streamIndex = STREAMINDEX_ONE;
    int result = audioStreamManager->ReleaseRender(streamIndex);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test ReleaseRender API
* @tc.number : ProAudioStreamManager_007
* @tc.desc   : Test ReleaseRender interface.
*/
HWTEST(ProAudioStreamManagerUnitTest, ProAudioStreamManager_007, TestSize.Level1)
{
    shared_ptr<ProAudioStreamManager> audioStreamManager = make_shared<ProAudioStreamManager>(DIRECT_PLAYBACK);
    ASSERT_TRUE(audioStreamManager != nullptr);

    AudioProcessConfig config;
    config.appInfo.appUid = CAPTURER_FLAG;
    config.appInfo.appPid = CAPTURER_FLAG;
    config.streamInfo.format = SAMPLE_S32LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    shared_ptr<IRendererStream> rendererStream = audioStreamManager->CreateRendererStream(config);
    ASSERT_TRUE(rendererStream != nullptr);

    int32_t rendererStreamMap = 0;
    audioStreamManager->rendererStreamMap_.emplace(rendererStreamMap, rendererStream);
    audioStreamManager->playbackEngine_.reset();

    uint32_t streamIndex = 0;
    int result = audioStreamManager->ReleaseRender(streamIndex);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test StartRender API
* @tc.number : ProAudioStreamManager_008
* @tc.desc   : Test StartRender interface.
*/
HWTEST(ProAudioStreamManagerUnitTest, ProAudioStreamManager_008, TestSize.Level1)
{
    shared_ptr<ProAudioStreamManager> audioStreamManager = make_shared<ProAudioStreamManager>(DIRECT_PLAYBACK);
    ASSERT_TRUE(audioStreamManager != nullptr);

    uint32_t streamIndex = 0;
    int result = audioStreamManager->StartRender(streamIndex);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test StartRender API
* @tc.number : ProAudioStreamManager_009
* @tc.desc   : Test StartRender interface.
*/
HWTEST(ProAudioStreamManagerUnitTest, ProAudioStreamManager_009, TestSize.Level1)
{
    shared_ptr<ProAudioStreamManager> audioStreamManager = make_shared<ProAudioStreamManager>(DIRECT_PLAYBACK);
    ASSERT_TRUE(audioStreamManager != nullptr);

    AudioProcessConfig config;
    config.appInfo.appUid = CAPTURER_FLAG;
    config.appInfo.appPid = CAPTURER_FLAG;
    config.streamInfo.format = SAMPLE_S32LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    shared_ptr<IRendererStream> rendererStream = audioStreamManager->CreateRendererStream(config);

    int32_t rendererStreamMap = MAP_NUM;
    audioStreamManager->rendererStreamMap_.emplace(rendererStreamMap, rendererStream);

    uint32_t streamIndex = STREAMINDEX_ONE;
    int result = audioStreamManager->StartRender(streamIndex);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test StartRender API
* @tc.number : ProAudioStreamManager_010
* @tc.desc   : Test StartRender interface.
*/
HWTEST(ProAudioStreamManagerUnitTest, ProAudioStreamManager_010, TestSize.Level1)
{
    shared_ptr<ProAudioStreamManager> audioStreamManager = make_shared<ProAudioStreamManager>(DIRECT_PLAYBACK);
    ASSERT_TRUE(audioStreamManager != nullptr);

    AudioProcessConfig config;
    config.appInfo.appUid = CAPTURER_FLAG;
    config.appInfo.appPid = CAPTURER_FLAG;
    config.streamInfo.format = SAMPLE_S32LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    shared_ptr<IRendererStream> rendererStream = audioStreamManager->CreateRendererStream(config);

    int32_t rendererStreamMap = MAP_NUM;
    audioStreamManager->rendererStreamMap_.emplace(rendererStreamMap, rendererStream);
    audioStreamManager->playbackEngine_.reset();

    uint32_t streamIndex = STREAMINDEX_ONE;
    int result = audioStreamManager->StartRender(streamIndex);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test StopRender API
* @tc.number : ProAudioStreamManager_011
* @tc.desc   : Test StopRender interface.
*/
HWTEST(ProAudioStreamManagerUnitTest, ProAudioStreamManager_011, TestSize.Level1)
{
    shared_ptr<ProAudioStreamManager> audioStreamManager = make_shared<ProAudioStreamManager>(DIRECT_PLAYBACK);
    ASSERT_TRUE(audioStreamManager != nullptr);

    uint32_t streamIndex = 0;
    int result = audioStreamManager->StopRender(streamIndex);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test StopRender API
* @tc.number : ProAudioStreamManager_012
* @tc.desc   : Test StopRender interface.
*/
HWTEST(ProAudioStreamManagerUnitTest, ProAudioStreamManager_012, TestSize.Level1)
{
    shared_ptr<ProAudioStreamManager> audioStreamManager = make_shared<ProAudioStreamManager>(DIRECT_PLAYBACK);
    ASSERT_TRUE(audioStreamManager != nullptr);

    uint64_t latency = audioStreamManager->GetLatency();
    EXPECT_EQ(latency, 0);

    AudioProcessConfig config;
    config.appInfo.appUid = CAPTURER_FLAG;
    config.appInfo.appPid = CAPTURER_FLAG;
    config.streamInfo.format = SAMPLE_S32LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    shared_ptr<IRendererStream> rendererStream = audioStreamManager->CreateRendererStream(config);

    int32_t rendererStreamMap = MAP_NUM;
    audioStreamManager->rendererStreamMap_.emplace(rendererStreamMap, rendererStream);

    uint32_t streamIndex = STREAMINDEX_ONE;
    int result = audioStreamManager->StopRender(streamIndex);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test StopRender API
* @tc.number : ProAudioStreamManager_013
* @tc.desc   : Test StopRender interface.
*/
HWTEST(ProAudioStreamManagerUnitTest, ProAudioStreamManager_013, TestSize.Level1)
{
    shared_ptr<ProAudioStreamManager> audioStreamManager = make_shared<ProAudioStreamManager>(DIRECT_PLAYBACK);
    ASSERT_TRUE(audioStreamManager != nullptr);

    AudioProcessConfig config;
    config.appInfo.appUid = CAPTURER_FLAG;
    config.appInfo.appPid = CAPTURER_FLAG;
    config.streamInfo.format = SAMPLE_S32LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    shared_ptr<IRendererStream> rendererStream = audioStreamManager->CreateRendererStream(config);

    int32_t rendererStreamMap = MAP_NUM;
    audioStreamManager->rendererStreamMap_.emplace(rendererStreamMap, rendererStream);
    audioStreamManager->playbackEngine_.reset();

    uint32_t streamIndex = STREAMINDEX_ONE;
    int result = audioStreamManager->StopRender(streamIndex);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test PauseRender API
* @tc.number : ProAudioStreamManager_014
* @tc.desc   : Test PauseRender interface.
*/
HWTEST(ProAudioStreamManagerUnitTest, ProAudioStreamManager_014, TestSize.Level1)
{
    shared_ptr<ProAudioStreamManager> audioStreamManager = make_shared<ProAudioStreamManager>(DIRECT_PLAYBACK);
    ASSERT_TRUE(audioStreamManager != nullptr);

    uint32_t streamIndex = 0;
    int result = audioStreamManager->PauseRender(streamIndex);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test PauseRender API
* @tc.number : ProAudioStreamManager_015
* @tc.desc   : Test PauseRender interface.
*/
HWTEST(ProAudioStreamManagerUnitTest, ProAudioStreamManager_015, TestSize.Level1)
{
    shared_ptr<ProAudioStreamManager> audioStreamManager = make_shared<ProAudioStreamManager>(DIRECT_PLAYBACK);
    ASSERT_TRUE(audioStreamManager != nullptr);

    AudioProcessConfig config;
    config.appInfo.appUid = CAPTURER_FLAG;
    config.appInfo.appPid = CAPTURER_FLAG;
    config.streamInfo.format = SAMPLE_S32LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    shared_ptr<IRendererStream> rendererStream = audioStreamManager->CreateRendererStream(config);

    int32_t rendererStreamMap = MAP_NUM;
    audioStreamManager->rendererStreamMap_.emplace(rendererStreamMap, rendererStream);

    uint32_t streamIndex = STREAMINDEX_ONE;
    int result = audioStreamManager->PauseRender(streamIndex);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test PauseRender API
* @tc.number : ProAudioStreamManager_016
* @tc.desc   : Test PauseRender interface.
*/
HWTEST(ProAudioStreamManagerUnitTest, ProAudioStreamManager_016, TestSize.Level1)
{
    shared_ptr<ProAudioStreamManager> audioStreamManager = make_shared<ProAudioStreamManager>(DIRECT_PLAYBACK);
    ASSERT_TRUE(audioStreamManager != nullptr);

    AudioProcessConfig config;
    config.appInfo.appUid = CAPTURER_FLAG;
    config.appInfo.appPid = CAPTURER_FLAG;
    config.streamInfo.format = SAMPLE_S32LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    shared_ptr<IRendererStream> rendererStream = audioStreamManager->CreateRendererStream(config);

    int32_t rendererStreamMap = MAP_NUM;
    audioStreamManager->rendererStreamMap_.emplace(rendererStreamMap, rendererStream);
    audioStreamManager->playbackEngine_.reset();

    uint32_t streamIndex = STREAMINDEX_ONE;
    int result = audioStreamManager->PauseRender(streamIndex);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test CreateCapturer API
* @tc.number : ProAudioStreamManager_017
* @tc.desc   : Test CreateCapturer interface.
*/
HWTEST(ProAudioStreamManagerUnitTest, ProAudioStreamManager_017, TestSize.Level1)
{
    shared_ptr<ProAudioStreamManager> audioStreamManager = make_shared<ProAudioStreamManager>(DIRECT_PLAYBACK);
    ASSERT_TRUE(audioStreamManager != nullptr);

    AudioProcessConfig config;
    config.appInfo.appUid = CAPTURER_FLAG;
    config.appInfo.appPid = CAPTURER_FLAG;
    config.streamInfo.format = SAMPLE_S32LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;

    shared_ptr<ICapturerStream> capturerStream = nullptr;
    int32_t result = audioStreamManager->CreateCapturer(config, capturerStream);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name   : Test ReleaseCapturer API
* @tc.number : ProAudioStreamManager_018
* @tc.desc   : Test ReleaseCapturer interface.
*/
HWTEST(ProAudioStreamManagerUnitTest, ProAudioStreamManager_018, TestSize.Level1)
{
    shared_ptr<ProAudioStreamManager> audioStreamManager = make_shared<ProAudioStreamManager>(DIRECT_PLAYBACK);
    ASSERT_TRUE(audioStreamManager != nullptr);

    uint32_t streamIndex = SESSIONID;
    int32_t result = audioStreamManager->ReleaseCapturer(streamIndex);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name   : Test AddUnprocessStream API
* @tc.number : ProAudioStreamManager_019
* @tc.desc   : Test AddUnprocessStream interface.
*/
HWTEST(ProAudioStreamManagerUnitTest, ProAudioStreamManager_019, TestSize.Level1)
{
    shared_ptr<ProAudioStreamManager> audioStreamManager = make_shared<ProAudioStreamManager>(DIRECT_PLAYBACK);
    ASSERT_TRUE(audioStreamManager != nullptr);

    int32_t appUid = CAPTURER_FLAG;
    int32_t result = audioStreamManager->AddUnprocessStream(appUid);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name   : Test CreatePlayBackEngine API
* @tc.number : ProAudioStreamManager_020
* @tc.desc   : Test CreatePlayBackEngine interface.
*/
HWTEST(ProAudioStreamManagerUnitTest, ProAudioStreamManager_020, TestSize.Level1)
{
    shared_ptr<ProAudioStreamManager> audioStreamManager = make_shared<ProAudioStreamManager>(DIRECT_PLAYBACK);
    ASSERT_TRUE(audioStreamManager != nullptr);

    AudioProcessConfig config;
    config.appInfo.appUid = CAPTURER_FLAG;
    config.appInfo.appPid = CAPTURER_FLAG;
    config.streamInfo.format = SAMPLE_S32LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    shared_ptr<IRendererStream> rendererStream = audioStreamManager->CreateRendererStream(config);

    int32_t result = audioStreamManager->CreatePlayBackEngine(rendererStream);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test CreatePlayBackEngine API
* @tc.number : ProAudioStreamManager_021
* @tc.desc   : Test TriggerStartIfNecessary interface.
*/
HWTEST(ProAudioStreamManagerUnitTest, ProAudioStreamManager_021, TestSize.Level1)
{
    shared_ptr<ProAudioStreamManager> audioStreamManager = make_shared<ProAudioStreamManager>(DIRECT_PLAYBACK);
    ASSERT_TRUE(audioStreamManager != nullptr);

    AudioProcessConfig config;
    config.appInfo.appUid = CAPTURER_FLAG;
    config.appInfo.appPid = CAPTURER_FLAG;
    config.streamInfo.format = SAMPLE_S32LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    shared_ptr<IRendererStream> rendererStream = audioStreamManager->CreateRendererStream(config);
    audioStreamManager->playbackEngine_.reset();

    int32_t result = audioStreamManager->TriggerStartIfNecessary();
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test CreatePlayBackEngine API
* @tc.number : ProAudioStreamManager_022
* @tc.desc   : Test TriggerStartIfNecessary interface.
*/
HWTEST(ProAudioStreamManagerUnitTest, ProAudioStreamManager_022, TestSize.Level1)
{
    shared_ptr<ProAudioStreamManager> audioStreamManager = make_shared<ProAudioStreamManager>(DIRECT_PLAYBACK);
    ASSERT_TRUE(audioStreamManager != nullptr);

    AudioProcessConfig config;
    config.appInfo.appUid = CAPTURER_FLAG;
    config.appInfo.appPid = CAPTURER_FLAG;
    config.streamInfo.format = SAMPLE_S32LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    shared_ptr<IRendererStream> rendererStream = audioStreamManager->CreateRendererStream(config);
    audioStreamManager->playbackEngine_->Stop();

    int32_t result = audioStreamManager->TriggerStartIfNecessary();
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test CreateRendererStream API
* @tc.number : CreateRendererStream_001
* @tc.desc   : Test CreateRendererStream interface.
*/
HWTEST(ProAudioStreamManagerUnitTest, CreateRendererStream_001, TestSize.Level1)
{
    shared_ptr<ProAudioStreamManager> audioStreamManager = make_shared<ProAudioStreamManager>(DIRECT_PLAYBACK);
    ASSERT_TRUE(audioStreamManager != nullptr);
    AudioProcessConfig config;
    // Mock InitParams to return failure
    auto stream = audioStreamManager->CreateRendererStream(config);
    EXPECT_NE(stream, nullptr);
}

/**
* @tc.name   : Test ReleaseRender API
* @tc.number : ReleaseRender_001
* @tc.desc   : Test ReleaseRender interface.
*/
HWTEST(ProAudioStreamManagerUnitTest, ReleaseRender_001, TestSize.Level1)
{
    shared_ptr<ProAudioStreamManager> audioStreamManager = make_shared<ProAudioStreamManager>(DIRECT_PLAYBACK);
    ASSERT_TRUE(audioStreamManager != nullptr);

    int32_t result = audioStreamManager->ReleaseRender(1);

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(audioStreamManager->rendererStreamMap_.size(), 0);
}
}
}
