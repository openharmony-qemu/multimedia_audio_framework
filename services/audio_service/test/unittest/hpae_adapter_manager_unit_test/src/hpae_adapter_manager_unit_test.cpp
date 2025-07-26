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

#ifndef LOG_TAG
#define LOG_TAG "HpaeAudioDeviceAdapterImplUnitTest"
#endif

#include "hpae_adapter_manager_unit_test.h"

#include <chrono>
#include <thread>

#include "audio_errors.h"
#include "hpae_adapter_manager.h"
#include "policy_handler.h"
#include "audio_system_manager.h"

using namespace std;
using namespace std::chrono;
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
constexpr uint32_t MIDDLE_SESSIONID = 100001;
constexpr uint32_t MAX_SESSIONID = UINT32_MAX - MIDDLE_SESSIONID;
constexpr uint32_t MORE_SESSIONID = MAX_SESSIONID + 1;
const int32_t MAP_NUM = 1;
const int32_t CAPTURER_FLAG = 10;
const uint32_t SESSIONID = 123456;
const uint32_t STREAMINDEX_ONE = 1;

void HpaeAdapterManagerUnitTest::SetUpTestCase(void) {}
void HpaeAdapterManagerUnitTest::TearDownTestCase(void) {}
void HpaeAdapterManagerUnitTest::SetUp(void) {}
void HpaeAdapterManagerUnitTest::TearDown(void) {}

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
* @tc.number : HpaeAdapterManager_001
* @tc.desc   : Test CreateRender interface.
*/
HWTEST(HpaeAdapterManagerUnitTest, HpaeAdapterManager_001, TestSize.Level1)
{
#ifdef HAS_FEATURE_INNERCAPTURER
    LoadPaPort();
#endif
    HpaeAdapterManager *adapterManager = new HpaeAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);
    AudioProcessConfig processConfig = GetInnerCapConfig();
    std::string deviceName = "Speaker";
    std::shared_ptr<IRendererStream> rendererStream = adapterManager->CreateRendererStream(processConfig, deviceName);
    ASSERT_TRUE(rendererStream != nullptr);

    int result = adapterManager->CreateRender(processConfig, rendererStream);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test CreateRender API
* @tc.number : HpaeAdapterManager_002
* @tc.desc   : Test CreateRender interface.
*/
HWTEST(HpaeAdapterManagerUnitTest, HpaeAdapterManager_002, TestSize.Level1)
{
    HpaeAdapterManager *adapterManager = new HpaeAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);
    AudioProcessConfig config = GetInnerCapConfig();
    config.originalSessionId = MORE_SESSIONID;
    std::string deviceName = "Speaker";
    std::shared_ptr<IRendererStream> rendererStream = adapterManager->CreateRendererStream(config, deviceName);
    ASSERT_TRUE(rendererStream != nullptr);

    int result = adapterManager->CreateRender(config, rendererStream);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test CreateRender API
* @tc.number : HpaeAdapterManager_003
* @tc.desc   : Test CreateRender interface.
*/
HWTEST(HpaeAdapterManagerUnitTest, HpaeAdapterManager_003, TestSize.Level1)
{
    HpaeAdapterManager *adapterManager = new HpaeAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);

    AudioProcessConfig config = GetInnerCapConfig();
    config.originalSessionId = MIDDLE_SESSIONID;
    std::string deviceName = "Speaker";
    std::shared_ptr<IRendererStream> rendererStream = adapterManager->CreateRendererStream(config, deviceName);
    ASSERT_TRUE(rendererStream != nullptr);

    int result = adapterManager->CreateRender(config, rendererStream);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test ReleaseRender API
* @tc.number : HpaeAdapterManager_004
* @tc.desc   : Test ReleaseRender interface.
*/
HWTEST(HpaeAdapterManagerUnitTest, HpaeAdapterManager_004, TestSize.Level1)
{
    HpaeAdapterManager *adapterManager = new HpaeAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);
    AudioProcessConfig config = GetInnerCapConfig();
    std::string deviceName = "Speaker";
    std::shared_ptr<IRendererStream> rendererStream = adapterManager->CreateRendererStream(config, deviceName);
    ASSERT_TRUE(rendererStream != nullptr);

    int32_t rendererStreamMap = MAP_NUM;
    adapterManager->rendererStreamMap_.emplace(rendererStreamMap, rendererStream);

    uint32_t streamIndex = STREAMINDEX_ONE;
    int result = adapterManager->ReleaseRender(streamIndex);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test ReleaseRender API
* @tc.number : HpaeAdapterManager_005
* @tc.desc   : Test ReleaseRender interface.
*/
HWTEST(HpaeAdapterManagerUnitTest, HpaeAdapterManager_005, TestSize.Level1)
{
    HpaeAdapterManager *adapterManager = new HpaeAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);
    AudioProcessConfig config = GetInnerCapConfig();
    std::string deviceName = "Speaker";
    std::shared_ptr<IRendererStream> rendererStream = adapterManager->CreateRendererStream(config, deviceName);
    ASSERT_TRUE(rendererStream != nullptr);

    int32_t rendererStreamMap = 0;
    adapterManager->rendererStreamMap_.emplace(rendererStreamMap, rendererStream);

    uint32_t streamIndex = 0;
    int result = adapterManager->ReleaseRender(streamIndex);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test ReleaseRender API
* @tc.number : HpaeAdapterManager_006
* @tc.desc   : Test ReleaseRender interface.
*/
HWTEST(HpaeAdapterManagerUnitTest, HpaeAdapterManager_006, TestSize.Level1)
{
    HpaeAdapterManager *adapterManager = new HpaeAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);

    AudioProcessConfig config = GetInnerCapConfig();
    std::string deviceName = "Speaker";
    std::shared_ptr<IRendererStream> rendererStream = adapterManager->CreateRendererStream(config, deviceName);
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
* @tc.number : HpaeAdapterManager_007
* @tc.desc   : Test ReleaseRender interface.
*/
HWTEST(HpaeAdapterManagerUnitTest, HpaeAdapterManager_007, TestSize.Level1)
{
    HpaeAdapterManager *adapterManager = new HpaeAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);

    AudioProcessConfig config = GetInnerCapConfig();
    std::string deviceName = "Speaker";
    std::shared_ptr<IRendererStream> rendererStream = adapterManager->CreateRendererStream(config, deviceName);
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
* @tc.number : HpaeAdapterManager_008
* @tc.desc   : Test StartRender interface.
*/
HWTEST(HpaeAdapterManagerUnitTest, HpaeAdapterManager_008, TestSize.Level1)
{
    HpaeAdapterManager *adapterManager = new HpaeAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);

    uint32_t streamIndex = 0;
    int result = adapterManager->StartRender(streamIndex);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test StartRender API
* @tc.number : HpaeAdapterManager_009
* @tc.desc   : Test StartRender interface.
*/
HWTEST(HpaeAdapterManagerUnitTest, HpaeAdapterManager_009, TestSize.Level1)
{
    HpaeAdapterManager *adapterManager = new HpaeAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);

    AudioProcessConfig config = GetInnerCapConfig();
    std::string deviceName = "Speaker";
    std::shared_ptr<IRendererStream> rendererStream = adapterManager->CreateRendererStream(config, deviceName);
    ASSERT_TRUE(rendererStream != nullptr);

    int32_t rendererStreamMap = MAP_NUM;
    adapterManager->rendererStreamMap_.emplace(rendererStreamMap, rendererStream);

    uint32_t streamIndex = STREAMINDEX_ONE;
    int result = adapterManager->StartRender(streamIndex);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test StartRender API
* @tc.number : HpaeAdapterManager_010
* @tc.desc   : Test StartRender interface.
*/
HWTEST(HpaeAdapterManagerUnitTest, HpaeAdapterManager_010, TestSize.Level1)
{
    HpaeAdapterManager *adapterManager = new HpaeAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);

    uint32_t streamIndex = 0;
    int result = adapterManager->StopRender(streamIndex);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test StopRender API
* @tc.number : HpaeAdapterManager_011
* @tc.desc   : Test StopRender interface.
*/
HWTEST(HpaeAdapterManagerUnitTest, HpaeAdapterManager_011, TestSize.Level1)
{
    HpaeAdapterManager *adapterManager = new HpaeAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);

    AudioProcessConfig config = GetInnerCapConfig();
    std::string deviceName = "Speaker";
    std::shared_ptr<IRendererStream> rendererStream = adapterManager->CreateRendererStream(config, deviceName);
    ASSERT_TRUE(rendererStream != nullptr);

    int32_t rendererStreamMap = MAP_NUM;
    adapterManager->rendererStreamMap_.emplace(rendererStreamMap, rendererStream);

    uint32_t streamIndex = STREAMINDEX_ONE;
    int result = adapterManager->StopRender(streamIndex);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test PauseRender API
* @tc.number : HpaeAdapterManager_012
* @tc.desc   : Test PauseRender interface.
*/
HWTEST(HpaeAdapterManagerUnitTest, HpaeAdapterManager_012, TestSize.Level1)
{
    HpaeAdapterManager *adapterManager = new HpaeAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);

    uint32_t streamIndex = 0;
    int result = adapterManager->PauseRender(streamIndex);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test PauseRender API
* @tc.number : HpaeAdapterManager_013
* @tc.desc   : Test PauseRender interface.
*/
HWTEST(HpaeAdapterManagerUnitTest, HpaeAdapterManager_013, TestSize.Level1)
{
    HpaeAdapterManager *adapterManager = new HpaeAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);

    AudioProcessConfig config = GetInnerCapConfig();
    std::string deviceName = "Speaker";
    std::shared_ptr<IRendererStream> rendererStream = adapterManager->CreateRendererStream(config, deviceName);
    ASSERT_TRUE(rendererStream != nullptr);

    int32_t rendererStreamMap = MAP_NUM;
    adapterManager->rendererStreamMap_.emplace(rendererStreamMap, rendererStream);

    uint32_t streamIndex = STREAMINDEX_ONE;
    int result = adapterManager->PauseRender(streamIndex);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test GetStreamCount API
* @tc.number : HpaeAdapterManager_014
* @tc.desc   : Test GetStreamCount interface.
*/
HWTEST(HpaeAdapterManagerUnitTest, HpaeAdapterManager_014, TestSize.Level1)
{
    HpaeAdapterManager *adapterManager = new HpaeAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);

    adapterManager->GetStreamCount();
    EXPECT_EQ(true, adapterManager->rendererStreamMap_.size() == 0);
}

/**
* @tc.name   : Test GetStreamCount API
* @tc.number : HpaeAdapterManager_015
* @tc.desc   : Test GetStreamCount interface.
*/
HWTEST(HpaeAdapterManagerUnitTest, HpaeAdapterManager_015, TestSize.Level1)
{
    HpaeAdapterManager *adapterManager = new HpaeAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);

    adapterManager->managerType_ = RECORDER;
    adapterManager->GetStreamCount();
    EXPECT_EQ(true, adapterManager->capturerStreamMap_.size() == 0);
}

/**
* @tc.name   : Test CreateCapturer API
* @tc.number : HpaeAdapterManager_016
* @tc.desc   : Test CreateCapturer interface.
*/
HWTEST(HpaeAdapterManagerUnitTest, HpaeAdapterManager_016, TestSize.Level1)
{
    HpaeAdapterManager *adapterManager = new HpaeAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);

    AudioProcessConfig processConfig = GetInnerCapConfig();
    std::string deviceName = "";

    std::shared_ptr<ICapturerStream> capturerStream = adapterManager->CreateCapturerStream(processConfig, deviceName);
    ASSERT_TRUE(capturerStream != nullptr);

    adapterManager->managerType_ = RECORDER;
    int result = adapterManager->CreateCapturer(processConfig, capturerStream);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test CreateCapturer API
* @tc.number : HpaeAdapterManager_017
* @tc.desc   : Test CreateCapturer interface.
*/
HWTEST(HpaeAdapterManagerUnitTest, HpaeAdapterManager_017, TestSize.Level1)
{
    HpaeAdapterManager *adapterManager = new HpaeAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);

    AudioProcessConfig config = GetInnerCapConfig();
    config.originalSessionId = MORE_SESSIONID;
    std::string deviceName = "";
    std::shared_ptr<ICapturerStream> capturerStream = adapterManager->CreateCapturerStream(config, deviceName);
    ASSERT_TRUE(capturerStream != nullptr);

    adapterManager->managerType_ = RECORDER;
    int result = adapterManager->CreateCapturer(config, capturerStream);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test CreateCapturer API
* @tc.number : HpaeAdapterManager_018
* @tc.desc   : Test CreateCapturer interface.
*/
HWTEST(HpaeAdapterManagerUnitTest, HpaeAdapterManager_018, TestSize.Level1)
{
    HpaeAdapterManager *adapterManager = new HpaeAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);

    AudioProcessConfig config = GetInnerCapConfig();
    config.originalSessionId = MIDDLE_SESSIONID;
    std::string deviceName = "Speaker";
    std::shared_ptr<ICapturerStream> capturerStream = adapterManager->CreateCapturerStream(config, deviceName);
    ASSERT_TRUE(capturerStream != nullptr);

    adapterManager->managerType_ = RECORDER;
    int result = adapterManager->CreateCapturer(config, capturerStream);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test GetDeviceNameForConnect API
* @tc.number : HpaeAdapterManager_019
* @tc.desc   : Test GetDeviceNameForConnect interface.
*/
HWTEST(HpaeAdapterManagerUnitTest, HpaeAdapterManager_019, TestSize.Level1)
{
    HpaeAdapterManager *adapterManager = new HpaeAdapterManager(DUP_PLAYBACK);
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
* @tc.number : HpaeAdapterManager_020
* @tc.desc   : Test GetDeviceNameForConnect interface.
*/
HWTEST(HpaeAdapterManagerUnitTest, HpaeAdapterManager_020, TestSize.Level1)
{
    HpaeAdapterManager *adapterManager = new HpaeAdapterManager(DUP_PLAYBACK);
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
* @tc.name   : Test CheckHighResolution API
* @tc.number : HpaeAdapterManager_021
* @tc.desc   : Test CheckHighResolution interface.
*/
HWTEST(HpaeAdapterManagerUnitTest, HpaeAdapterManager_021, TestSize.Level1)
{
    HpaeAdapterManager *adapterManager = new HpaeAdapterManager(DUP_PLAYBACK);
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
* @tc.number : HpaeAdapterManager_022
* @tc.desc   : Test CheckHighResolution interface.
*/
HWTEST(HpaeAdapterManagerUnitTest, HpaeAdapterManager_022, TestSize.Level1)
{
    HpaeAdapterManager *adapterManager = new HpaeAdapterManager(DUP_PLAYBACK);
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
* @tc.number : HpaeAdapterManager_023
* @tc.desc   : Test CheckHighResolution interface.
*/
HWTEST(HpaeAdapterManagerUnitTest, HpaeAdapterManager_023, TestSize.Level1)
{
    HpaeAdapterManager *adapterManager = new HpaeAdapterManager(DUP_PLAYBACK);
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
* @tc.number : HpaeAdapterManager_024
* @tc.desc   : Test CheckHighResolution interface.
*/
HWTEST(HpaeAdapterManagerUnitTest, HpaeAdapterManager_024, TestSize.Level1)
{
    HpaeAdapterManager *adapterManager = new HpaeAdapterManager(DUP_PLAYBACK);
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
* @tc.number : HpaeAdapterManager_025
* @tc.desc   : Test CheckHighResolution interface.
*/
HWTEST(HpaeAdapterManagerUnitTest, HpaeAdapterManager_025, TestSize.Level1)
{
    HpaeAdapterManager *adapterManager = new HpaeAdapterManager(DUP_PLAYBACK);
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
* @tc.number : HpaeAdapterManager_026
* @tc.desc   : Test CheckHighResolution interface.
*/
HWTEST(HpaeAdapterManagerUnitTest, HpaeAdapterManager_026, TestSize.Level1)
{
    HpaeAdapterManager *adapterManager = new HpaeAdapterManager(DUP_PLAYBACK);
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
* @tc.number : HpaeAdapterManager_027
* @tc.desc   : Test SetHighResolution interface.
*/
HWTEST(HpaeAdapterManagerUnitTest, HpaeAdapterManager_027, TestSize.Level1)
{
    HpaeAdapterManager *adapterManager = new HpaeAdapterManager(DUP_PLAYBACK);
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
    uint32_t sessionId = SESSIONID;
    adapterManager->isHighResolutionExist_ = true;
    adapterManager->SetHighResolution(config, sessionId);
    EXPECT_NE(nullptr, adapterManager);
}

/**
* @tc.name   : Test SetHighResolution API
* @tc.number : HpaeAdapterManager_028
* @tc.desc   : Test SetHighResolution interface.
*/
HWTEST(HpaeAdapterManagerUnitTest, HpaeAdapterManager_028, TestSize.Level1)
{
    HpaeAdapterManager *adapterManager = new HpaeAdapterManager(DUP_PLAYBACK);
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
    uint32_t sessionId = SESSIONID;
    adapterManager->isHighResolutionExist_ = true;
    adapterManager->SetHighResolution(config, sessionId);
    EXPECT_NE(nullptr, adapterManager);
}

/**
* @tc.name   : Test SetHighResolution API
* @tc.number : HpaeAdapterManager_029
* @tc.desc   : Test SetHighResolution interface.
*/
HWTEST(HpaeAdapterManagerUnitTest, HpaeAdapterManager_029, TestSize.Level1)
{
    HpaeAdapterManager *adapterManager = new HpaeAdapterManager(DUP_PLAYBACK);
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
    uint32_t sessionId = SESSIONID;
    adapterManager->isHighResolutionExist_ = false;
    adapterManager->SetHighResolution(config, sessionId);
    EXPECT_NE(nullptr, adapterManager);
}

/**
* @tc.name   : Test SetHighResolution API
* @tc.number : HpaeAdapterManager_030
* @tc.desc   : Test SetHighResolution interface.
*/
HWTEST(HpaeAdapterManagerUnitTest, HpaeAdapterManager_030, TestSize.Level1)
{
    HpaeAdapterManager *adapterManager = new HpaeAdapterManager(DUP_PLAYBACK);
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
    uint32_t sessionId = SESSIONID;
    adapterManager->isHighResolutionExist_ = false;
    adapterManager->SetHighResolution(config, sessionId);
    EXPECT_EQ(true, adapterManager->isHighResolutionExist_);
}

/**
* @tc.name   : Test CreateRendererStream API
* @tc.number : HpaeAdapterManager_031
* @tc.desc   : Test CreateRendererStream interface.
*/
HWTEST(HpaeAdapterManagerUnitTest, HpaeAdapterManager_031, TestSize.Level1)
{
    HpaeAdapterManager *adapterManager = new HpaeAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);

    AudioProcessConfig processConfig = GetInnerCapConfig();
    std::string deviceName = "";
    adapterManager->CreateRendererStream(processConfig, deviceName);
    EXPECT_NE(nullptr, adapterManager);
}

/**
* @tc.name   : Test CreateCapturerStream API
* @tc.number : HpaeAdapterManager_032
* @tc.desc   : Test CreateCapturerStream interface.
*/
HWTEST(HpaeAdapterManagerUnitTest, HpaeAdapterManager_032, TestSize.Level1)
{
    HpaeAdapterManager *adapterManager = new HpaeAdapterManager(DUP_PLAYBACK);

    AudioProcessConfig processConfig = GetInnerCapConfig();
    std::string deviceName = "";
    adapterManager->CreateCapturerStream(processConfig, deviceName);
    EXPECT_NE(nullptr, adapterManager);
}

/**
* @tc.name   : Test ReleaseCapturer API
* @tc.number : HpaeAdapterManager_033
* @tc.desc   : Test ReleaseCapturer interface.
*/
HWTEST(HpaeAdapterManagerUnitTest, HpaeAdapterManager_033, TestSize.Level1)
{
    uint32_t streamIndex0 = 0;
    HpaeAdapterManager *adapterManager = new HpaeAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);

    AudioProcessConfig processConfig = GetInnerCapConfig();
    std::string deviceName = "";
    std::shared_ptr<ICapturerStream> capturerStream = adapterManager->CreateCapturerStream(processConfig, deviceName);
    ASSERT_TRUE(capturerStream != nullptr);
    adapterManager->capturerStreamMap_.insert({streamIndex0, capturerStream});

    auto ret = adapterManager->ReleaseCapturer(streamIndex0);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name   : Test CreateRender API
* @tc.number : HpaeAdapterManager_034
* @tc.desc   : Test CreateRender interface.
*/
HWTEST(HpaeAdapterManagerUnitTest, HpaeAdapterManager_034, TestSize.Level1)
{
    HpaeAdapterManager *adapterManager = new HpaeAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);

    AudioProcessConfig config = GetInnerCapConfig();
    config.originalSessionId = MORE_SESSIONID;
    uint32_t sessionId = SESSIONID;
    std::shared_ptr<IRendererStream> stream = nullptr;
    std::shared_ptr<IRendererStream> rendererStream = adapterManager->CreateRendererStream(config, "");
    ASSERT_TRUE(rendererStream != nullptr);

    int result = adapterManager->CreateRender(config, rendererStream);
    EXPECT_NE(sessionId, result);
}

/**
* @tc.name   : Test StartRenderWithSyncId API
* @tc.number : HpaeAdapterManager_035
* @tc.desc   : Test StartRenderWithSyncId interface.
*/
HWTEST(HpaeAdapterManagerUnitTest, HpaeAdapterManager_035, TestSize.Level1)
{
    HpaeAdapterManager *adapterManager = new HpaeAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);

    uint32_t streamIndex = 0;
    int32_t syncId = -1;
    int result = adapterManager->StartRenderWithSyncId(streamIndex, syncId);
    EXPECT_NE(ERROR, result);
}

/**
* @tc.name   : Test StartRenderWithSyncId API
* @tc.number : HpaeAdapterManager_036
* @tc.desc   : Test StartRenderWithSyncId interface.
*/
HWTEST(HpaeAdapterManagerUnitTest, HpaeAdapterManager_036, TestSize.Level1)
{
    HpaeAdapterManager *adapterManager = new HpaeAdapterManager(DUP_PLAYBACK);
    ASSERT_TRUE(adapterManager != nullptr);

    AudioProcessConfig config = GetInnerCapConfig();
    std::string deviceName = "Speaker";
    std::shared_ptr<IRendererStream> rendererStream = adapterManager->CreateRendererStream(config, deviceName);
    ASSERT_TRUE(rendererStream != nullptr);

    int32_t rendererStreamMap = MAP_NUM;
    adapterManager->rendererStreamMap_.emplace(rendererStreamMap, rendererStream);

    uint32_t streamIndex = STREAMINDEX_ONE;
    int32_t syncId = 123;
    int result = adapterManager->StartRenderWithSyncId(streamIndex, syncId);
    EXPECT_NE(ERROR, result);
}
} // namespace AudioStandard
} // namespace OHOS