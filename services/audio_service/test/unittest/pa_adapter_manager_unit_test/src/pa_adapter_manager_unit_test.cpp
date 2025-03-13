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

#ifndef LOG_TAG
#define LOG_TAG "AudioDeviceAdapterImplUnitTest"
#endif

#include "pa_adapter_manager_unit_test.h"

#include <chrono>
#include <thread>

#include "audio_errors.h"
#include "pa_adapter_manager.h"
#include "policy_handler.h"
#include "audio_system_manager.h"

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

void PaAdapterManagerUnitTest::SetUpTestCase(void) {}
void PaAdapterManagerUnitTest::TearDownTestCase(void) {}
void PaAdapterManagerUnitTest::SetUp(void) {}
void PaAdapterManagerUnitTest::TearDown(void) {}

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
    config.innerCapId = 1;
    return config;
}

#ifdef HAS_FEATURE_INNERCAPTURER
void LoadPaPort()
{
    AudioPlaybackCaptureConfig checkConfig;
    int32_t checkInnerCapId = 0;
    AudioSystemManager::GetInstance()->CheckCaptureLimit(checkConfig, checkInnerCapId);
}

void ReleasePaPort()
{
    AudioSystemManager::GetInstance()->ReleaseCaptureLimit(1);
}
#endif

/**
* @tc.name   : Test CreateRender API
* @tc.number : PaAdapterManager_001
* @tc.desc   : Test CreateRender interface.
*/
HWTEST(PaAdapterManagerUnitTest, PaAdapterManager_001, TestSize.Level1)
{
#ifdef HAS_FEATURE_INNERCAPTURER
    LoadPaPort();
#endif
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);
    adapterManager->InitPaContext();

    AudioProcessConfig processConfig = GetInnerCapConfig();
    uint32_t sessionId = SESSIONID;
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    std::shared_ptr<IRendererStream> rendererStream = adapterManager->CreateRendererStream(processConfig, stream);
    ASSERT_TRUE(rendererStream != nullptr);

    int result = adapterManager->CreateRender(processConfig, rendererStream);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test CreateRender API
* @tc.number : PaAdapterManager_002
* @tc.desc   : Test CreateRender interface.
*/
HWTEST(PaAdapterManagerUnitTest, PaAdapterManager_002, TestSize.Level1)
{
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);
    adapterManager->InitPaContext();

    AudioProcessConfig config = GetInnerCapConfig();
    config.originalSessionId = MORE_SESSIONID;
    uint32_t sessionId = SESSIONID;
    pa_stream *stream = adapterManager->InitPaStream(config, sessionId, false);
    std::shared_ptr<IRendererStream> rendererStream = adapterManager->CreateRendererStream(config, stream);
    ASSERT_TRUE(rendererStream != nullptr);

    int result = adapterManager->CreateRender(config, rendererStream);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test CreateRender API
* @tc.number : PaAdapterManager_003
* @tc.desc   : Test CreateRender interface.
*/
HWTEST(PaAdapterManagerUnitTest, PaAdapterManager_003, TestSize.Level1)
{
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);
    adapterManager->InitPaContext();

    AudioProcessConfig config = GetInnerCapConfig();
    config.originalSessionId = MIDDLE_SESSIONID;
    uint32_t sessionId = SESSIONID;
    pa_stream *stream = adapterManager->InitPaStream(config, sessionId, false);
    std::shared_ptr<IRendererStream> rendererStream = adapterManager->CreateRendererStream(config, stream);
    ASSERT_TRUE(rendererStream != nullptr);

    int result = adapterManager->CreateRender(config, rendererStream);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test ReleaseRender API
* @tc.number : PaAdapterManager_004
* @tc.desc   : Test ReleaseRender interface.
*/
HWTEST(PaAdapterManagerUnitTest, PaAdapterManager_004, TestSize.Level1)
{
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);
    adapterManager->InitPaContext();

    AudioProcessConfig config = GetInnerCapConfig();
    uint32_t sessionId = SESSIONID;
    pa_stream *stream = adapterManager->InitPaStream(config, sessionId, false);
    std::shared_ptr<IRendererStream> rendererStream = adapterManager->CreateRendererStream(config, stream);
    ASSERT_TRUE(rendererStream != nullptr);

    int32_t rendererStreamMap = MAP_NUM;
    adapterManager->rendererStreamMap_.emplace(rendererStreamMap, rendererStream);

    uint32_t streamIndex = STREAMINDEX_ONE;
    int result = adapterManager->ReleaseRender(streamIndex);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test ReleaseRender API
* @tc.number : PaAdapterManager_005
* @tc.desc   : Test ReleaseRender interface.
*/
HWTEST(PaAdapterManagerUnitTest, PaAdapterManager_005, TestSize.Level1)
{
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);
    adapterManager->InitPaContext();

    AudioProcessConfig config = GetInnerCapConfig();
    uint32_t sessionId = SESSIONID;
    pa_stream *stream = adapterManager->InitPaStream(config, sessionId, false);
    std::shared_ptr<IRendererStream> rendererStream = adapterManager->CreateRendererStream(config, stream);
    ASSERT_TRUE(rendererStream != nullptr);

    int32_t rendererStreamMap = 0;
    adapterManager->rendererStreamMap_.emplace(rendererStreamMap, rendererStream);

    uint32_t streamIndex = 0;
    int result = adapterManager->ReleaseRender(streamIndex);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test ReleaseRender API
* @tc.number : PaAdapterManager_006
* @tc.desc   : Test ReleaseRender interface.
*/
HWTEST(PaAdapterManagerUnitTest, PaAdapterManager_006, TestSize.Level1)
{
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);
    adapterManager->InitPaContext();

    AudioProcessConfig config = GetInnerCapConfig();
    uint32_t sessionId = SESSIONID;
    pa_stream *stream = adapterManager->InitPaStream(config, sessionId, false);
    std::shared_ptr<IRendererStream> rendererStream = adapterManager->CreateRendererStream(config, stream);
    ASSERT_TRUE(rendererStream != nullptr);

    int32_t rendererStreamMap = MAP_NUM;
    adapterManager->rendererStreamMap_.emplace(rendererStreamMap, rendererStream);
    adapterManager->isHighResolutionExist_ = true;

    uint32_t streamIndex = STREAMINDEX_ONE;
    int result = adapterManager->ReleaseRender(streamIndex);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test ReleaseRender API
* @tc.number : PaAdapterManager_007
* @tc.desc   : Test ReleaseRender interface.
*/
HWTEST(PaAdapterManagerUnitTest, PaAdapterManager_007, TestSize.Level1)
{
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);
    adapterManager->InitPaContext();

    AudioProcessConfig config = GetInnerCapConfig();
    uint32_t sessionId = SESSIONID;
    pa_stream *stream = adapterManager->InitPaStream(config, sessionId, false);
    std::shared_ptr<IRendererStream> rendererStream = adapterManager->CreateRendererStream(config, stream);
    ASSERT_TRUE(rendererStream != nullptr);

    int32_t rendererStreamMap = 0;
    adapterManager->rendererStreamMap_.emplace(rendererStreamMap, rendererStream);
    adapterManager->isHighResolutionExist_ = true;

    uint32_t streamIndex = 0;
    int result = adapterManager->ReleaseRender(streamIndex);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test StartRender API
* @tc.number : PaAdapterManager_008
* @tc.desc   : Test StartRender interface.
*/
HWTEST(PaAdapterManagerUnitTest, PaAdapterManager_008, TestSize.Level1)
{
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);

    uint32_t streamIndex = 0;
    int result = adapterManager->StartRender(streamIndex);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test StartRender API
* @tc.number : PaAdapterManager_009
* @tc.desc   : Test StartRender interface.
*/
HWTEST(PaAdapterManagerUnitTest, PaAdapterManager_009, TestSize.Level1)
{
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);
    adapterManager->InitPaContext();

    AudioProcessConfig config = GetInnerCapConfig();
    uint32_t sessionId = SESSIONID;
    pa_stream *stream = adapterManager->InitPaStream(config, sessionId, false);
    std::shared_ptr<IRendererStream> rendererStream = adapterManager->CreateRendererStream(config, stream);
    ASSERT_TRUE(rendererStream != nullptr);

    int32_t rendererStreamMap = MAP_NUM;
    adapterManager->rendererStreamMap_.emplace(rendererStreamMap, rendererStream);

    uint32_t streamIndex = STREAMINDEX_ONE;
    int result = adapterManager->StartRender(streamIndex);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test StartRender API
* @tc.number : PaAdapterManager_010
* @tc.desc   : Test StartRender interface.
*/
HWTEST(PaAdapterManagerUnitTest, PaAdapterManager_010, TestSize.Level1)
{
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);

    uint32_t streamIndex = 0;
    int result = adapterManager->StopRender(streamIndex);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test StopRender API
* @tc.number : PaAdapterManager_011
* @tc.desc   : Test StopRender interface.
*/
HWTEST(PaAdapterManagerUnitTest, PaAdapterManager_011, TestSize.Level1)
{
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);
    adapterManager->InitPaContext();

    AudioProcessConfig config = GetInnerCapConfig();
    uint32_t sessionId = SESSIONID;
    pa_stream *stream = adapterManager->InitPaStream(config, sessionId, false);
    std::shared_ptr<IRendererStream> rendererStream = adapterManager->CreateRendererStream(config, stream);
    ASSERT_TRUE(rendererStream != nullptr);

    int32_t rendererStreamMap = MAP_NUM;
    adapterManager->rendererStreamMap_.emplace(rendererStreamMap, rendererStream);

    uint32_t streamIndex = STREAMINDEX_ONE;
    int result = adapterManager->StopRender(streamIndex);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test PauseRender API
* @tc.number : PaAdapterManager_012
* @tc.desc   : Test PauseRender interface.
*/
HWTEST(PaAdapterManagerUnitTest, PaAdapterManager_012, TestSize.Level1)
{
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);

    uint32_t streamIndex = 0;
    int result = adapterManager->PauseRender(streamIndex);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test PauseRender API
* @tc.number : PaAdapterManager_013
* @tc.desc   : Test PauseRender interface.
*/
HWTEST(PaAdapterManagerUnitTest, PaAdapterManager_013, TestSize.Level1)
{
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);
    adapterManager->InitPaContext();

    AudioProcessConfig config = GetInnerCapConfig();
    uint32_t sessionId = SESSIONID;
    pa_stream *stream = adapterManager->InitPaStream(config, sessionId, false);
    std::shared_ptr<IRendererStream> rendererStream = adapterManager->CreateRendererStream(config, stream);
    ASSERT_TRUE(rendererStream != nullptr);

    int32_t rendererStreamMap = MAP_NUM;
    adapterManager->rendererStreamMap_.emplace(rendererStreamMap, rendererStream);

    uint32_t streamIndex = STREAMINDEX_ONE;
    int result = adapterManager->PauseRender(streamIndex);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test GetStreamCount API
* @tc.number : PaAdapterManager_014
* @tc.desc   : Test GetStreamCount interface.
*/
HWTEST(PaAdapterManagerUnitTest, PaAdapterManager_014, TestSize.Level1)
{
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);

    adapterManager->GetStreamCount();
    EXPECT_EQ(true, adapterManager->rendererStreamMap_.size() == 0);
}

/**
* @tc.name   : Test GetStreamCount API
* @tc.number : PaAdapterManager_015
* @tc.desc   : Test GetStreamCount interface.
*/
HWTEST(PaAdapterManagerUnitTest, PaAdapterManager_015, TestSize.Level1)
{
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);

    adapterManager->managerType_ = RECORDER;
    adapterManager->GetStreamCount();
    EXPECT_EQ(true, adapterManager->capturerStreamMap_.size() == 0);
}

/**
* @tc.name   : Test CreateCapturer API
* @tc.number : PaAdapterManager_016
* @tc.desc   : Test CreateCapturer interface.
*/
HWTEST(PaAdapterManagerUnitTest, PaAdapterManager_016, TestSize.Level1)
{
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);
    adapterManager->InitPaContext();

    AudioProcessConfig processConfig = GetInnerCapConfig();
    uint32_t sessionId = SESSIONID;
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    std::shared_ptr<ICapturerStream> capturerStream = adapterManager->CreateCapturerStream(processConfig, stream);
    ASSERT_TRUE(capturerStream != nullptr);

    adapterManager->managerType_ = RECORDER;
    int result = adapterManager->CreateCapturer(processConfig, capturerStream);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test CreateCapturer API
* @tc.number : PaAdapterManager_017
* @tc.desc   : Test CreateCapturer interface.
*/
HWTEST(PaAdapterManagerUnitTest, PaAdapterManager_017, TestSize.Level1)
{
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);
    adapterManager->InitPaContext();

    AudioProcessConfig config = GetInnerCapConfig();
    config.originalSessionId = MORE_SESSIONID;
    uint32_t sessionId = SESSIONID;
    pa_stream *stream = adapterManager->InitPaStream(config, sessionId, false);
    std::shared_ptr<ICapturerStream> capturerStream = adapterManager->CreateCapturerStream(config, stream);
    ASSERT_TRUE(capturerStream != nullptr);

    adapterManager->managerType_ = RECORDER;
    int result = adapterManager->CreateCapturer(config, capturerStream);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test CreateCapturer API
* @tc.number : PaAdapterManager_018
* @tc.desc   : Test CreateCapturer interface.
*/
HWTEST(PaAdapterManagerUnitTest, PaAdapterManager_018, TestSize.Level1)
{
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);
    adapterManager->InitPaContext();

    AudioProcessConfig config = GetInnerCapConfig();
    config.originalSessionId = MIDDLE_SESSIONID;
    uint32_t sessionId = SESSIONID;
    pa_stream *stream = adapterManager->InitPaStream(config, sessionId, false);
    std::shared_ptr<ICapturerStream> capturerStream = adapterManager->CreateCapturerStream(config, stream);
    ASSERT_TRUE(capturerStream != nullptr);

    adapterManager->managerType_ = RECORDER;
    int result = adapterManager->CreateCapturer(config, capturerStream);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test ResetPaContext API
* @tc.number : PaAdapterManager_019
* @tc.desc   : Test ResetPaContext interface.
*/
HWTEST(PaAdapterManagerUnitTest, PaAdapterManager_019, TestSize.Level1)
{
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);

    adapterManager->context_ = nullptr;
    adapterManager->mainLoop_ = nullptr;
    int result = adapterManager->ResetPaContext();
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name   : Test ResetPaContext API
* @tc.number : PaAdapterManager_020
* @tc.desc   : Test ResetPaContext interface.
*/
HWTEST(PaAdapterManagerUnitTest, PaAdapterManager_020, TestSize.Level1)
{
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);

    adapterManager->context_ = nullptr;
    adapterManager->mainLoop_ = pa_threaded_mainloop_new();
    int result = adapterManager->ResetPaContext();
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name   : Test ResetPaContext API
* @tc.number : PaAdapterManager_021
* @tc.desc   : Test ResetPaContext interface.
*/
HWTEST(PaAdapterManagerUnitTest, PaAdapterManager_021, TestSize.Level1)
{
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);

    pa_mainloop_api *mainloop = new pa_mainloop_api();
    char *name = nullptr;
    adapterManager->context_ = pa_context_new(mainloop, name);
    adapterManager->mainLoop_ = nullptr;

    int result = adapterManager->ResetPaContext();
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name   : Test HandleMainLoopStart API
* @tc.number : PaAdapterManager_022
* @tc.desc   : Test HandleMainLoopStart interface.
*/
HWTEST(PaAdapterManagerUnitTest, PaAdapterManager_022, TestSize.Level1)
{
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);

    pa_mainloop_api *mainloop = new pa_mainloop_api();
    char *name = nullptr;
    adapterManager->context_ = pa_context_new(mainloop, name);
    adapterManager->mainLoop_ = pa_threaded_mainloop_new();

    int result = adapterManager->HandleMainLoopStart();
    EXPECT_NE(SUCCESS, result);
}

/**
* @tc.name   : Test GetDeviceNameForConnect API
* @tc.number : PaAdapterManager_023
* @tc.desc   : Test GetDeviceNameForConnect interface.
*/
HWTEST(PaAdapterManagerUnitTest, PaAdapterManager_023, TestSize.Level1)
{
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);

    AudioProcessConfig config = GetInnerCapConfig();
    config.audioMode = AudioMode::AUDIO_MODE_RECORD;
    config.isWakeupCapturer = true;
    config.isInnerCapturer = true;
    config.innerCapMode = InnerCapMode::LEGACY_MUTE_CAP;
    uint32_t sessionId = SESSIONID;
    std::string deviceName = "deviceName";
    int result = adapterManager->GetDeviceNameForConnect(config, sessionId, deviceName);
    EXPECT_NE(SUCCESS, result);
}

/**
* @tc.name   : Test GetDeviceNameForConnect API
* @tc.number : PaAdapterManager_024
* @tc.desc   : Test GetDeviceNameForConnect interface.
*/
HWTEST(PaAdapterManagerUnitTest, PaAdapterManager_024, TestSize.Level1)
{
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);

    AudioProcessConfig config = GetInnerCapConfig();
    config.audioMode = AudioMode::AUDIO_MODE_RECORD;
    config.isInnerCapturer = true;
    config.innerCapMode = InnerCapMode::LEGACY_MUTE_CAP;
    uint32_t sessionId = SESSIONID;
    std::string deviceName = "deviceName";
    int result = adapterManager->GetDeviceNameForConnect(config, sessionId, deviceName);
    EXPECT_NE(SUCCESS, result);
}

/**
* @tc.name   : Test ReleasePaStream API
* @tc.number : PaAdapterManager_025
* @tc.desc   : Test ReleasePaStream interface.
*/
HWTEST(PaAdapterManagerUnitTest, PaAdapterManager_025, TestSize.Level1)
{
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);
    adapterManager->InitPaContext();

    AudioProcessConfig processConfig = GetInnerCapConfig();
    uint32_t sessionId = SESSIONID;
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    adapterManager->mainLoop_ = pa_threaded_mainloop_new();
    adapterManager->ReleasePaStream(stream);
    EXPECT_NE(nullptr, adapterManager->mainLoop_);
}

/**
* @tc.name   : Test ReleasePaStream API
* @tc.number : PaAdapterManager_026
* @tc.desc   : Test ReleasePaStream interface.
*/
HWTEST(PaAdapterManagerUnitTest, PaAdapterManager_026, TestSize.Level1)
{
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);
    adapterManager->InitPaContext();

    AudioProcessConfig processConfig = GetInnerCapConfig();
    uint32_t sessionId = SESSIONID;
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    adapterManager->mainLoop_ = nullptr;
    adapterManager->ReleasePaStream(stream);
    EXPECT_NE(nullptr, stream);
}

/**
* @tc.name   : Test ReleasePaStream API
* @tc.number : PaAdapterManager_027
* @tc.desc   : Test ReleasePaStream interface.
*/
HWTEST(PaAdapterManagerUnitTest, PaAdapterManager_027, TestSize.Level1)
{
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);

    pa_stream *stream = nullptr;
    adapterManager->mainLoop_ = nullptr;
    adapterManager->ReleasePaStream(stream);
    EXPECT_NE(nullptr, adapterManager);
}

/**
* @tc.name   : Test CheckHighResolution API
* @tc.number : PaAdapterManager_028
* @tc.desc   : Test CheckHighResolution interface.
*/
HWTEST(PaAdapterManagerUnitTest, PaAdapterManager_035, TestSize.Level1)
{
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);

    AudioProcessConfig config;
    config.appInfo.appUid = CAPTURER_FLAG;
    config.appInfo.appPid = CAPTURER_FLAG;
    config.streamInfo.format = SAMPLE_S16LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_32000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.streamType = AudioStreamType::STREAM_VOICE_CALL;
    config.deviceType = DEVICE_TYPE_BLUETOOTH_SCO;
    bool result = adapterManager->CheckHighResolution(config);
    EXPECT_NE(true, result);
}

/**
* @tc.name   : Test CheckHighResolution API
* @tc.number : PaAdapterManager_029
* @tc.desc   : Test CheckHighResolution interface.
*/
HWTEST(PaAdapterManagerUnitTest, PaAdapterManager_036, TestSize.Level1)
{
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);

    AudioProcessConfig config;
    config.appInfo.appUid = CAPTURER_FLAG;
    config.appInfo.appPid = CAPTURER_FLAG;
    config.streamInfo.format = SAMPLE_S16LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_32000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.streamType = AudioStreamType::STREAM_VOICE_CALL;
    config.deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    bool result = adapterManager->CheckHighResolution(config);
    EXPECT_NE(true, result);
}

/**
* @tc.name   : Test CheckHighResolution API
* @tc.number : PaAdapterManager_030
* @tc.desc   : Test CheckHighResolution interface.
*/
HWTEST(PaAdapterManagerUnitTest, PaAdapterManager_037, TestSize.Level1)
{
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);

    AudioProcessConfig config;
    config.appInfo.appUid = CAPTURER_FLAG;
    config.appInfo.appPid = CAPTURER_FLAG;
    config.streamInfo.format = SAMPLE_S16LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_32000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    bool result = adapterManager->CheckHighResolution(config);
    EXPECT_NE(true, result);
}

/**
* @tc.name   : Test CheckHighResolution API
* @tc.number : PaAdapterManager_031
* @tc.desc   : Test CheckHighResolution interface.
*/
HWTEST(PaAdapterManagerUnitTest, PaAdapterManager_038, TestSize.Level1)
{
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);

    AudioProcessConfig config;
    config.appInfo.appUid = CAPTURER_FLAG;
    config.appInfo.appPid = CAPTURER_FLAG;
    config.streamInfo.format = SAMPLE_S16LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    bool result = adapterManager->CheckHighResolution(config);
    EXPECT_NE(true, result);
}

/**
* @tc.name   : Test CheckHighResolution API
* @tc.number : PaAdapterManager_032
* @tc.desc   : Test CheckHighResolution interface.
*/
HWTEST(PaAdapterManagerUnitTest, PaAdapterManager_039, TestSize.Level1)
{
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);

    AudioProcessConfig config;
    config.appInfo.appUid = CAPTURER_FLAG;
    config.appInfo.appPid = CAPTURER_FLAG;
    config.streamInfo.format = SAMPLE_S24LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    bool result = adapterManager->CheckHighResolution(config);
    EXPECT_NE(false, result);
}

/**
* @tc.name   : Test CheckHighResolution API
* @tc.number : PaAdapterManager_033
* @tc.desc   : Test CheckHighResolution interface.
*/
HWTEST(PaAdapterManagerUnitTest, PaAdapterManager_040, TestSize.Level1)
{
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);

    AudioProcessConfig config;
    config.appInfo.appUid = CAPTURER_FLAG;
    config.appInfo.appPid = CAPTURER_FLAG;
    config.streamInfo.format = SAMPLE_S24LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    bool result = adapterManager->CheckHighResolution(config);
    EXPECT_EQ(true, result);
}

/**
* @tc.name   : Test SetHighResolution API
* @tc.number : PaAdapterManager_034
* @tc.desc   : Test SetHighResolution interface.
*/
HWTEST(PaAdapterManagerUnitTest, PaAdapterManager_041, TestSize.Level1)
{
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);

    AudioProcessConfig config;
    config.appInfo.appUid = CAPTURER_FLAG;
    config.appInfo.appPid = CAPTURER_FLAG;
    config.streamInfo.format = SAMPLE_S16LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_32000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.streamType = AudioStreamType::STREAM_VOICE_CALL;
    config.deviceType = DEVICE_TYPE_BLUETOOTH_SCO;
    config.rendererInfo.spatializationEnabled = true;
    pa_proplist *propList = pa_proplist_new();
    uint32_t sessionId = SESSIONID;
    adapterManager->isHighResolutionExist_ = true;
    adapterManager->SetHighResolution(propList, config, sessionId);
    EXPECT_NE(nullptr, adapterManager);
}

/**
* @tc.name   : Test SetHighResolution API
* @tc.number : PaAdapterManager_035
* @tc.desc   : Test SetHighResolution interface.
*/
HWTEST(PaAdapterManagerUnitTest, PaAdapterManager_042, TestSize.Level1)
{
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);

    AudioProcessConfig config;
    config.appInfo.appUid = CAPTURER_FLAG;
    config.appInfo.appPid = CAPTURER_FLAG;
    config.streamInfo.format = SAMPLE_S16LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_32000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.streamType = AudioStreamType::STREAM_VOICE_CALL;
    config.deviceType = DEVICE_TYPE_BLUETOOTH_SCO;
    config.rendererInfo.spatializationEnabled = false;
    pa_proplist *propList = pa_proplist_new();
    uint32_t sessionId = SESSIONID;
    adapterManager->isHighResolutionExist_ = true;
    adapterManager->SetHighResolution(propList, config, sessionId);
    EXPECT_NE(nullptr, adapterManager);
}

/**
* @tc.name   : Test SetHighResolution API
* @tc.number : PaAdapterManager_036
* @tc.desc   : Test SetHighResolution interface.
*/
HWTEST(PaAdapterManagerUnitTest, PaAdapterManager_043, TestSize.Level1)
{
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);

    AudioProcessConfig config;
    config.appInfo.appUid = CAPTURER_FLAG;
    config.appInfo.appPid = CAPTURER_FLAG;
    config.streamInfo.format = SAMPLE_S16LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_32000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.streamType = AudioStreamType::STREAM_VOICE_CALL;
    config.deviceType = DEVICE_TYPE_BLUETOOTH_SCO;
    config.rendererInfo.spatializationEnabled = false;
    pa_proplist *propList = pa_proplist_new();
    uint32_t sessionId = SESSIONID;
    adapterManager->isHighResolutionExist_ = false;
    adapterManager->SetHighResolution(propList, config, sessionId);
    EXPECT_NE(nullptr, adapterManager);
}

/**
* @tc.name   : Test SetHighResolution API
* @tc.number : PaAdapterManager_037
* @tc.desc   : Test SetHighResolution interface.
*/
HWTEST(PaAdapterManagerUnitTest, PaAdapterManager_044, TestSize.Level1)
{
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);

    AudioProcessConfig config;
    config.appInfo.appUid = CAPTURER_FLAG;
    config.appInfo.appPid = CAPTURER_FLAG;
    config.streamInfo.format = SAMPLE_S24LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    config.rendererInfo.spatializationEnabled = false;
    pa_proplist *propList = pa_proplist_new();
    uint32_t sessionId = SESSIONID;
    adapterManager->isHighResolutionExist_ = false;
    adapterManager->SetHighResolution(propList, config, sessionId);
    EXPECT_EQ(true, adapterManager->isHighResolutionExist_);
}

/**
* @tc.name   : Test SetRecordProplist API
* @tc.number : PaAdapterManager_038
* @tc.desc   : Test SetRecordProplist interface.
*/
HWTEST(PaAdapterManagerUnitTest, PaAdapterManager_045, TestSize.Level1)
{
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);
    adapterManager->unprocessAppUidSet_.emplace(CAPTURER_FLAG);

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
    pa_proplist *propList = pa_proplist_new();
    adapterManager->SetRecordProplist(propList, config);
    EXPECT_NE(nullptr, adapterManager);
}

/**
* @tc.name   : Test CreateRendererStream API
* @tc.number : PaAdapterManager_039
* @tc.desc   : Test CreateRendererStream interface.
*/
HWTEST(PaAdapterManagerUnitTest, PaAdapterManager_046, TestSize.Level1)
{
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);
    adapterManager->InitPaContext();

    AudioProcessConfig processConfig = GetInnerCapConfig();
    uint32_t sessionId = SESSIONID;
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    adapterManager->CreateRendererStream(processConfig, stream);
    EXPECT_NE(nullptr, adapterManager);
}

/**
* @tc.name   : Test CreateCapturerStream API
* @tc.number : PaAdapterManager_040
* @tc.desc   : Test CreateCapturerStream interface.
*/
HWTEST(PaAdapterManagerUnitTest, PaAdapterManager_047, TestSize.Level1)
{
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    AudioProcessConfig processConfig = GetInnerCapConfig();
    uint32_t sessionId = SESSIONID;
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    adapterManager->CreateCapturerStream(processConfig, stream);
    EXPECT_NE(nullptr, adapterManager);
}

/**
* @tc.name   : Test PAStreamStateCb API
* @tc.number : PaAdapterManager_042
* @tc.desc   : Test PAStreamStateCb interface.
*/
HWTEST(PaAdapterManagerUnitTest, PaAdapterManager_048, TestSize.Level1)
{
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    AudioProcessConfig processConfig = GetInnerCapConfig();
    uint32_t sessionId = SESSIONID;
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    void *userdata = nullptr;
    adapterManager->PAStreamStateCb(stream, userdata);
    EXPECT_NE(nullptr, adapterManager);
}

/**
* @tc.name   : Test ConvertChLayoutToPaChMap API
* @tc.number : PaAdapterManager_043
* @tc.desc   : Test ConvertChLayoutToPaChMap interface.
*/
HWTEST(PaAdapterManagerUnitTest, PaAdapterManager_049, TestSize.Level1)
{
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    const uint64_t channelLayout = CH_LAYOUT_HOA_ORDER1_ACN_N3D;
    pa_channel_map processCm;
    adapterManager->ConvertChLayoutToPaChMap(channelLayout, processCm);
    EXPECT_NE(nullptr, adapterManager);
}

/**
* @tc.name   : Test GetEnhanceSceneName API
* @tc.number : PaAdapterManager_044
* @tc.desc   : Test GetEnhanceSceneName interface.
*/
HWTEST(PaAdapterManagerUnitTest, PaAdapterManager_050, TestSize.Level1)
{
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    adapterManager->GetEnhanceSceneName(SOURCE_TYPE_VOICE_TRANSCRIPTION);
    EXPECT_NE(nullptr, adapterManager);
}

/**
* @tc.name   : Test GetEnhanceSceneName API
* @tc.number : PaAdapterManager_045
* @tc.desc   : Test GetEnhanceSceneName interface.
*/
HWTEST(PaAdapterManagerUnitTest, PaAdapterManager_051, TestSize.Level1)
{
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    adapterManager->GetEnhanceSceneName(SOURCE_TYPE_VOICE_MESSAGE);
    EXPECT_NE(nullptr, adapterManager);
#ifdef HAS_FEATURE_INNERCAPTURER
    ReleasePaPort();
#endif
}
} // namespace AudioStandard
} // namespace OHOS