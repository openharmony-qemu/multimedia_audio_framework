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

#include "audio_errors.h"
#include <gtest/gtest.h>
#include "pa_renderer_stream_impl.h"
#include "pa_adapter_manager.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
const int32_t CAPTURER_FLAG = 10;

class PaAdapterManagerUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void PaAdapterManagerUnitTest::SetUpTestCase() {}
void PaAdapterManagerUnitTest::TearDownTestCase() {}
void PaAdapterManagerUnitTest::SetUp() {}
void PaAdapterManagerUnitTest::TearDown() {}

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
    config.capturerInfo.sourceType = SOURCE_TYPE_WAKEUP;
    return config;
}

/**
 * @tc.name  : Test PaAdapterManager.
 * @tc.type  : FUNC
 * @tc.number: PaAdapterManagerUnitTest_001.
 * @tc.desc  : Test CreateRender interface.
 */
HWTEST_F(PaAdapterManagerUnitTest, PaAdapterManagerUnitTest_001, TestSize.Level1)
{
    std::string packageName = "";
    AudioProcessConfig processConfig;
    std::shared_ptr<IRendererStream> stream;
    processConfig.originalSessionId = 10;
    processConfig.originalSessionId = MAX_SESSIONID + 1;
    PaAdapterManager* paAdapterManager_ = new PaAdapterManager(PLAYBACK);
    paAdapterManager_->mainLoop_ = pa_threaded_mainloop_new();
    paAdapterManager_->api_ = pa_threaded_mainloop_get_api(paAdapterManager_->mainLoop_);
    paAdapterManager_->context_ = pa_context_new(paAdapterManager_->api_, packageName.c_str());
    paAdapterManager_->CreateRender(processConfig, stream);
    EXPECT_NE(paAdapterManager_, nullptr);
}

/**
 * @tc.name  : Test PaAdapterManager.
 * @tc.type  : FUNC
 * @tc.number: PaAdapterManagerUnitTest_002.
 * @tc.desc  : Test CreateRender interface.
 */
HWTEST_F(PaAdapterManagerUnitTest, PaAdapterManagerUnitTest_002, TestSize.Level1)
{
    std::string packageName = "";
    AudioProcessConfig processConfig;
    std::shared_ptr<IRendererStream> stream;
    processConfig.originalSessionId = MIN_SESSIONID + 1;
    processConfig.originalSessionId = MAX_SESSIONID - 1;
    PaAdapterManager* paAdapterManager_ = new PaAdapterManager(PLAYBACK);
    paAdapterManager_->mainLoop_ = pa_threaded_mainloop_new();
    paAdapterManager_->api_ = pa_threaded_mainloop_get_api(paAdapterManager_->mainLoop_);
    paAdapterManager_->context_ = pa_context_new(paAdapterManager_->api_, packageName.c_str());
    paAdapterManager_->CreateRender(processConfig, stream);
    EXPECT_NE(paAdapterManager_, nullptr);
}

/**
 * @tc.name  : Test PaAdapterManager.
 * @tc.type  : FUNC
 * @tc.number: PaAdapterManagerUnitTest_003.
 * @tc.desc  : Test CreateRender interface.
 */
HWTEST_F(PaAdapterManagerUnitTest, PaAdapterManagerUnitTest_003, TestSize.Level1)
{
    std::string packageName = "";
    AudioProcessConfig processConfig;
    std::shared_ptr<IRendererStream> stream;
    processConfig.originalSessionId = MIN_SESSIONID + 1;
    processConfig.originalSessionId = MAX_SESSIONID + 1;
    PaAdapterManager* paAdapterManager_ = new PaAdapterManager(PLAYBACK);
    paAdapterManager_->mainLoop_ = pa_threaded_mainloop_new();
    paAdapterManager_->api_ = pa_threaded_mainloop_get_api(paAdapterManager_->mainLoop_);
    paAdapterManager_->context_ = pa_context_new(paAdapterManager_->api_, packageName.c_str());
    paAdapterManager_->CreateRender(processConfig, stream);
    EXPECT_NE(paAdapterManager_, nullptr);
}

/**
 * @tc.name  : Test PaAdapterManager.
 * @tc.type  : FUNC
 * @tc.number: PaAdapterManagerUnitTest_004.
 * @tc.desc  : Test ReleaseRender interface.
 */
HWTEST_F(PaAdapterManagerUnitTest, PaAdapterManagerUnitTest_004, TestSize.Level1)
{
    int32_t streamIndex = 1;
    AudioProcessConfig processConfig;
    PaAdapterManager* paAdapterManager_ = new PaAdapterManager(PLAYBACK);
    pa_stream *paStream = nullptr;
    pa_threaded_mainloop *mainloop = pa_threaded_mainloop_new();
    std::shared_ptr<IRendererStream> iRendererStream_ = std::make_shared<PaRendererStreamImpl>(
        paStream, processConfig, mainloop);
    paAdapterManager_->rendererStreamMap_.insert({streamIndex, iRendererStream_});
    paAdapterManager_->isHighResolutionExist_ = true;
    paAdapterManager_->highResolutionIndex_ = streamIndex;
    int32_t result = paAdapterManager_->ReleaseRender(streamIndex);
    EXPECT_EQ(result, SUCCESS);

    paAdapterManager_->isHighResolutionExist_ = false;
    paAdapterManager_->highResolutionIndex_ = 0;
    result = paAdapterManager_->ReleaseRender(streamIndex);
    EXPECT_EQ(result, SUCCESS);

    paAdapterManager_->isHighResolutionExist_ = false;
    paAdapterManager_->highResolutionIndex_ = streamIndex;
    result = paAdapterManager_->ReleaseRender(streamIndex);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : Test PaAdapterManager.
 * @tc.type  : FUNC
 * @tc.number: PaAdapterManagerUnitTest_005.
 * @tc.desc  : Test StartRender/PauseRender/StopRender interface.
 */
HWTEST_F(PaAdapterManagerUnitTest, PaAdapterManagerUnitTest_005, TestSize.Level1)
{
    uint32_t streamIndex = 1;
    pa_stream *paStream = nullptr;
    AudioProcessConfig processConfig;
    pa_threaded_mainloop *mainloop = pa_threaded_mainloop_new();
    std::shared_ptr<IRendererStream> iRendererStream_ = std::make_shared<PaRendererStreamImpl>(
        paStream, processConfig, mainloop);
    PaAdapterManager* paAdapterManager_ = new PaAdapterManager(PLAYBACK);
    int32_t result = paAdapterManager_->StartRender(streamIndex);
    EXPECT_EQ(result, SUCCESS);

    result = paAdapterManager_->PauseRender(streamIndex);
    EXPECT_EQ(result, SUCCESS);

    result = paAdapterManager_->StopRender(streamIndex);
    EXPECT_EQ(result, SUCCESS);

    paAdapterManager_->rendererStreamMap_.insert({streamIndex, iRendererStream_});
    paAdapterManager_->StartRender(streamIndex);
    EXPECT_NE(paAdapterManager_, nullptr);

    result = paAdapterManager_->PauseRender(streamIndex);
    EXPECT_NE(paAdapterManager_, nullptr);

    result = paAdapterManager_->StopRender(streamIndex);
    EXPECT_NE(paAdapterManager_, nullptr);
}

/**
 * @tc.name  : Test PaAdapterManager.
 * @tc.type  : FUNC
 * @tc.number: PaAdapterManagerUnitTest_006.
 * @tc.desc  : Test GetStreamCount interface.
 */
HWTEST_F(PaAdapterManagerUnitTest, PaAdapterManagerUnitTest_006, TestSize.Level1)
{
    PaAdapterManager* paAdapterManager_ = new PaAdapterManager(PLAYBACK);
    int32_t result = paAdapterManager_->GetStreamCount();
    EXPECT_EQ(result, 0);

    paAdapterManager_->managerType_ = RECORDER;
    result = paAdapterManager_->GetStreamCount();
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name  : Test PaAdapterManager.
 * @tc.type  : FUNC
 * @tc.number: PaAdapterManagerUnitTest_007.
 * @tc.desc  : Test CreateCapturer interface.
 */
HWTEST_F(PaAdapterManagerUnitTest, PaAdapterManagerUnitTest_007, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    std::shared_ptr<ICapturerStream> stream;
    PaAdapterManager* paAdapterManager_ = new PaAdapterManager(PLAYBACK);
    processConfig.originalSessionId = MIN_SESSIONID + 1;
    paAdapterManager_->CreateCapturer(processConfig, stream);
    EXPECT_NE(paAdapterManager_, nullptr);

    processConfig.originalSessionId = MAX_SESSIONID + 1;
    paAdapterManager_->CreateCapturer(processConfig, stream);
    EXPECT_NE(paAdapterManager_, nullptr);

    processConfig.originalSessionId = MIN_SESSIONID - 1;
    paAdapterManager_->CreateCapturer(processConfig, stream);
    EXPECT_NE(paAdapterManager_, nullptr);
}

/**
 * @tc.name  : Test PaAdapterManager.
 * @tc.type  : FUNC
 * @tc.number: PaAdapterManagerUnitTest_008.
 * @tc.desc  : Test ResetPaContext interface.
 */
HWTEST_F(PaAdapterManagerUnitTest, PaAdapterManagerUnitTest_008, TestSize.Level1)
{
    std::string packageName = "";
    PaAdapterManager* paAdapterManager_ = new PaAdapterManager(PLAYBACK);
    paAdapterManager_->context_ = nullptr;
    paAdapterManager_->ResetPaContext();
    EXPECT_NE(paAdapterManager_, nullptr);

    paAdapterManager_->mainLoop_ = pa_threaded_mainloop_new();
    paAdapterManager_->api_ = pa_threaded_mainloop_get_api(paAdapterManager_->mainLoop_);
    paAdapterManager_->context_ = pa_context_new(paAdapterManager_->api_, packageName.c_str());
    paAdapterManager_->isContextConnected_ = true;
    paAdapterManager_->ResetPaContext();
    EXPECT_NE(paAdapterManager_, nullptr);

    paAdapterManager_->isContextConnected_ = true;
    paAdapterManager_->ResetPaContext();
    EXPECT_NE(paAdapterManager_, nullptr);
}

/**
 * @tc.name  : Test PaAdapterManager.
 * @tc.type  : FUNC
 * @tc.number: PaAdapterManagerUnitTest_009.
 * @tc.desc  : Test GetDeviceNameForConnect interface.
 */
HWTEST_F(PaAdapterManagerUnitTest, PaAdapterManagerUnitTest_009, TestSize.Level1)
{
    uint32_t sessionId = 123;
    AudioProcessConfig processConfig;
    std::string deviceName;
    processConfig.audioMode = AUDIO_MODE_RECORD;
    PaAdapterManager* paAdapterManager_ = new PaAdapterManager(PLAYBACK);
    paAdapterManager_->GetDeviceNameForConnect(processConfig, sessionId, deviceName);
    EXPECT_NE(paAdapterManager_, nullptr);

    processConfig.isWakeupCapturer = true;
    paAdapterManager_->GetDeviceNameForConnect(processConfig, sessionId, deviceName);
    EXPECT_NE(paAdapterManager_, nullptr);
}

/**
 * @tc.name  : Test PaAdapterManager.
 * @tc.type  : FUNC
 * @tc.number: PaAdapterManagerUnitTest_010.
 * @tc.desc  : Test GetDeviceNameForConnect interface.
 */
HWTEST_F(PaAdapterManagerUnitTest, PaAdapterManagerUnitTest_010, TestSize.Level1)
{
    uint32_t sessionId = 123;
    AudioProcessConfig processConfig;
    std::string deviceName;
    processConfig.audioMode = AUDIO_MODE_RECORD;
    PaAdapterManager* paAdapterManager_ = new PaAdapterManager(PLAYBACK);
    processConfig.capturerInfo.sourceType = SOURCE_TYPE_REMOTE_CAST;
    paAdapterManager_->GetDeviceNameForConnect(processConfig, sessionId, deviceName);
    EXPECT_NE(paAdapterManager_, nullptr);

    processConfig.isInnerCapturer = true;
    paAdapterManager_->GetDeviceNameForConnect(processConfig, sessionId, deviceName);
    EXPECT_NE(paAdapterManager_, nullptr);

    processConfig.innerCapMode = MODERN_INNER_CAP;
    paAdapterManager_->GetDeviceNameForConnect(processConfig, sessionId, deviceName);
    EXPECT_NE(paAdapterManager_, nullptr);
}

/**
 * @tc.name  : Test PaAdapterManager.
 * @tc.type  : FUNC
 * @tc.number: PaAdapterManagerUnitTest_011.
 * @tc.desc  : Test ReleasePaStream interface.
 */
HWTEST_F(PaAdapterManagerUnitTest, PaAdapterManagerUnitTest_011, TestSize.Level1)
{
    pa_stream* paStream = nullptr;
    PaAdapterManager* paAdapterManager_ = new PaAdapterManager(PLAYBACK);
    paAdapterManager_->mainLoop_ = nullptr;
    paAdapterManager_->ReleasePaStream(paStream);
    EXPECT_NE(paAdapterManager_, nullptr);

    paAdapterManager_->mainLoop_ = pa_threaded_mainloop_new();
    paAdapterManager_->ReleasePaStream(paStream);
    EXPECT_NE(paAdapterManager_, nullptr);
}

/**
 * @tc.name  : Test PaAdapterManager.
 * @tc.type  : FUNC
 * @tc.number: PaAdapterManagerUnitTest_012.
 * @tc.desc  : Test CheckHighResolution interface.
 */
HWTEST_F(PaAdapterManagerUnitTest, PaAdapterManagerUnitTest_012, TestSize.Level1)
{
    AudioProcessConfig config;
    config.streamType = STREAM_MUSIC;
    config.streamInfo.format = SAMPLE_S24LE;
    config.deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    PaAdapterManager* paAdapterManager_ = new PaAdapterManager(DUP_PLAYBACK);

    bool result = paAdapterManager_->CheckHighResolution(config);
    EXPECT_EQ(result, true);

    config.streamInfo.format = SAMPLE_S16LE;
    result = paAdapterManager_->CheckHighResolution(config);
    EXPECT_EQ(result, false);

    config.deviceType = DEVICE_TYPE_BLUETOOTH_A2DP_IN;
    result = paAdapterManager_->CheckHighResolution(config);
    EXPECT_EQ(result, false);

    config.streamInfo.samplingRate = SAMPLE_RATE_64000;
    result = paAdapterManager_->CheckHighResolution(config);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test PaAdapterManager.
 * @tc.type  : FUNC
 * @tc.number: PaAdapterManagerUnitTest_013.
 * @tc.desc  : Test CheckHighResolution interface.
 */
HWTEST_F(PaAdapterManagerUnitTest, PaAdapterManagerUnitTest_013, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.streamType = STREAM_VOICE_CALL;
    processConfig.streamInfo.format = SAMPLE_S24LE;
    processConfig.deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    processConfig.streamInfo.samplingRate = SAMPLE_RATE_48000;
    PaAdapterManager* paAdapterManager_ = new PaAdapterManager(DUP_PLAYBACK);
    bool result = paAdapterManager_->CheckHighResolution(processConfig);
    EXPECT_EQ(result, false);

    processConfig.deviceType = DEVICE_TYPE_BLUETOOTH_A2DP_IN;
    result = paAdapterManager_->CheckHighResolution(processConfig);
    EXPECT_EQ(result, false);

    processConfig.streamInfo.samplingRate = SAMPLE_RATE_64000;
    result = paAdapterManager_->CheckHighResolution(processConfig);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test PaAdapterManager.
 * @tc.type  : FUNC
 * @tc.number: PaAdapterManagerUnitTest_014.
 * @tc.desc  : Test CheckHighResolution interface.
 */
HWTEST_F(PaAdapterManagerUnitTest, PaAdapterManagerUnitTest_014, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.streamType = STREAM_VOICE_CALL;
    processConfig.streamInfo.format = SAMPLE_S24LE;
    processConfig.deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    processConfig.streamInfo.samplingRate = SAMPLE_RATE_48000;
    PaAdapterManager* paAdapterManager_ = new PaAdapterManager(DUP_PLAYBACK);
    bool result = paAdapterManager_->CheckHighResolution(processConfig);
    EXPECT_EQ(result, false);

    processConfig.streamInfo.format = SAMPLE_S32LE;
    result = paAdapterManager_->CheckHighResolution(processConfig);
    EXPECT_EQ(result, false);

    processConfig.streamInfo.samplingRate = SAMPLE_RATE_64000;
    result = paAdapterManager_->CheckHighResolution(processConfig);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test PaAdapterManager.
 * @tc.type  : FUNC
 * @tc.number: PaAdapterManagerUnitTest_015.
 * @tc.desc  : Test SetHighResolution interface.
 */
HWTEST_F(PaAdapterManagerUnitTest, PaAdapterManagerUnitTest_015, TestSize.Level1)
{
    uint32_t sessionId = 123;
    pa_proplist* propList = pa_proplist_new();
    AudioProcessConfig processConfig;
    processConfig.streamType = STREAM_MUSIC;
    processConfig.streamInfo.format = SAMPLE_S24LE;
    processConfig.deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    processConfig.streamInfo.samplingRate = SAMPLE_RATE_48000;
    processConfig.rendererInfo.spatializationEnabled = false;
    PaAdapterManager* paAdapterManager_ = new PaAdapterManager(PLAYBACK);
    paAdapterManager_->SetHighResolution(propList, processConfig, sessionId);
    EXPECT_NE(paAdapterManager_, nullptr);

    processConfig.rendererInfo.spatializationEnabled = true;
    paAdapterManager_->SetHighResolution(propList, processConfig, sessionId);
    EXPECT_NE(paAdapterManager_, nullptr);

    paAdapterManager_->isHighResolutionExist_ = true;
    paAdapterManager_->SetHighResolution(propList, processConfig, sessionId);
    EXPECT_NE(paAdapterManager_, nullptr);

    processConfig.rendererInfo.spatializationEnabled = false;
    paAdapterManager_->SetHighResolution(propList, processConfig, sessionId);
    EXPECT_NE(paAdapterManager_, nullptr);
}

/**
 * @tc.name  : Test PaAdapterManager.
 * @tc.type  : FUNC
 * @tc.number: PaAdapterManagerUnitTest_016.
 * @tc.desc  : Test SetHighResolution interface.
 */
HWTEST_F(PaAdapterManagerUnitTest, PaAdapterManagerUnitTest_016, TestSize.Level1)
{
    uint32_t sessionId = 123;
    pa_proplist* propList = pa_proplist_new();
    AudioProcessConfig processConfig;
    processConfig.streamType = STREAM_RING;
    processConfig.streamInfo.format = SAMPLE_S24LE;
    processConfig.deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    processConfig.streamInfo.samplingRate = SAMPLE_RATE_48000;
    processConfig.rendererInfo.spatializationEnabled = false;
    PaAdapterManager* paAdapterManager_ = new PaAdapterManager(PLAYBACK);
    paAdapterManager_->SetHighResolution(propList, processConfig, sessionId);
    EXPECT_NE(paAdapterManager_, nullptr);

    processConfig.rendererInfo.spatializationEnabled = true;
    paAdapterManager_->SetHighResolution(propList, processConfig, sessionId);
    EXPECT_NE(paAdapterManager_, nullptr);

    paAdapterManager_->isHighResolutionExist_ = true;
    paAdapterManager_->SetHighResolution(propList, processConfig, sessionId);
    EXPECT_NE(paAdapterManager_, nullptr);

    processConfig.rendererInfo.spatializationEnabled = false;
    paAdapterManager_->SetHighResolution(propList, processConfig, sessionId);
    EXPECT_NE(paAdapterManager_, nullptr);
}

/**
 * @tc.name  : Test PaAdapterManager.
 * @tc.type  : FUNC
 * @tc.number: PaAdapterManagerUnitTest_017.
 * @tc.desc  : Test SetHighResolution interface.
 */
HWTEST_F(PaAdapterManagerUnitTest, PaAdapterManagerUnitTest_017, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    pa_proplist* propList = pa_proplist_new();
    PaAdapterManager* paAdapterManager_ = new PaAdapterManager(PLAYBACK);
    paAdapterManager_->SetRecordProplist(propList, processConfig);
    EXPECT_NE(paAdapterManager_, nullptr);

    processConfig.appInfo.appUid = 1;
    paAdapterManager_->unprocessAppUidSet_.emplace(processConfig.appInfo.appUid);
    paAdapterManager_->SetRecordProplist(propList, processConfig);
    EXPECT_NE(paAdapterManager_, nullptr);
}

/**
 * @tc.name  : Test PaAdapterManager.
 * @tc.type  : FUNC
 * @tc.number: PaAdapterManagerUnitTest_018.
 * @tc.desc  : Test SetStreamAudioEnhanceMode interface.
 */
HWTEST_F(PaAdapterManagerUnitTest, PaAdapterManagerUnitTest_018, TestSize.Level1)
{
    AudioProcessConfig processConfig = GetInnerCapConfig();
    PaAdapterManager* paAdapterManager_ = new PaAdapterManager(DUP_PLAYBACK);
    paAdapterManager_->mainLoop_ = pa_threaded_mainloop_new();
    paAdapterManager_->InitPaContext();
    uint32_t sessionId = 123456;
    pa_stream *stream = paAdapterManager_->InitPaStream(processConfig, sessionId, false);
    paAdapterManager_->SetStreamAudioEnhanceMode(stream, AudioEnhanceMode::ENHANCE_DEFAULT);
    EXPECT_NE(paAdapterManager_, nullptr);
}

/**
 * @tc.name  : Test PaAdapterManager.
 * @tc.type  : FUNC
 * @tc.number: PaAdapterManagerUnitTest_019.
 * @tc.desc  : Test ConvertChLayoutToPaChMap interface.
 */
HWTEST_F(PaAdapterManagerUnitTest, PaAdapterManagerUnitTest_019, TestSize.Level1)
{
    pa_channel_map paMap;
    uint64_t channelLayout = 17592186044416;
    PaAdapterManager* paAdapterManager_ = new PaAdapterManager(PLAYBACK);
    paAdapterManager_->ConvertChLayoutToPaChMap(channelLayout, paMap);
    EXPECT_NE(paAdapterManager_, nullptr);

    channelLayout = 35184372088832;
    paAdapterManager_->ConvertChLayoutToPaChMap(channelLayout, paMap);
    EXPECT_NE(paAdapterManager_, nullptr);
}

/**
 * @tc.name  : Test PaAdapterManager.
 * @tc.type  : FUNC
 * @tc.number: PaAdapterManagerUnitTest_021.
 * @tc.desc  : Test GetEnhanceSceneName interface.
 */
HWTEST_F(PaAdapterManagerUnitTest, PaAdapterManagerUnitTest_020, TestSize.Level1)
{
    PaAdapterManager* paAdapterManager_ = new PaAdapterManager(PLAYBACK);
    std::string result = paAdapterManager_->GetEnhanceSceneName(SourceType::SOURCE_TYPE_MIC);
    EXPECT_EQ(result, "SCENE_RECORD");

    result = paAdapterManager_->GetEnhanceSceneName(SourceType::SOURCE_TYPE_CAMCORDER);
    EXPECT_EQ(result, "SCENE_RECORD");

    result = paAdapterManager_->GetEnhanceSceneName(SourceType::SOURCE_TYPE_VOICE_CALL);
    EXPECT_EQ(result, "SCENE_VOIP_UP");

    result = paAdapterManager_->GetEnhanceSceneName(SourceType::SOURCE_TYPE_VOICE_COMMUNICATION);
    EXPECT_EQ(result, "SCENE_VOIP_UP");

    result = paAdapterManager_->GetEnhanceSceneName(SourceType::SOURCE_TYPE_VOICE_TRANSCRIPTION);
    EXPECT_EQ(result, "SCENE_PRE_ENHANCE");

    result = paAdapterManager_->GetEnhanceSceneName(SourceType::SOURCE_TYPE_VOICE_MESSAGE);
    EXPECT_EQ(result, "SCENE_VOICE_MESSAGE");

    result = paAdapterManager_->GetEnhanceSceneName(SourceType::SOURCE_TYPE_MAX);
    EXPECT_EQ(result, "SCENE_OTHERS");
}
} // namespace AudioStandard
} // namespace OHOS
