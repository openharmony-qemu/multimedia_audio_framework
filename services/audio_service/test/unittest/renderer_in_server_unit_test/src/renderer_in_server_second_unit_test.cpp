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

#include "renderer_in_server_second_unit_test.h"
#include "accesstoken_kit.h"
#include "audio_device_info.h"
#include "audio_errors.h"
#include "audio_info.h"
#include "audio_process_config.h"
#include "audio_server.h"
#include "audio_service.h"
#include "audio_stream_info.h"
#include "audio_utils.h"
#include "policy_handler.h"
#include "renderer_in_server.h"
#include "pro_audio_stream_manager.h"
#include "i_renderer_stream.h"
#include "audio_service_log.h"
#include "ipc_stream_in_server.h"
#include "pro_renderer_stream_impl.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

static std::shared_ptr<IStreamListener> stateListener;
static std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
static std::shared_ptr<RendererInServer> rendererInServer;
static std::shared_ptr<OHAudioBuffer> buffer;
static std::weak_ptr<IStreamListener> streamListener;

static AudioProcessConfig processConfig;
static BufferDesc bufferDesc;

static AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
    AudioChannelLayout::CH_LAYOUT_UNKNOWN);

void RendererInServerExtUnitTest::SetUpTestCase(void) {}

void RendererInServerExtUnitTest::TearDownTestCase(void)
{
    stateListener.reset();
    streamListenerHolder.reset();
    rendererInServer.reset();
    buffer.reset();
    streamListener.reset();
}

void RendererInServerExtUnitTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
    processConfig.deviceType = DEVICE_TYPE_WIRED_HEADSET;
    processConfig.streamInfo = testStreamInfo;
    processConfig.streamType = STREAM_MUSIC;
    processConfig.rendererInfo.pipeType = PIPE_TYPE_DIRECT_MUSIC;
    processConfig.rendererInfo.rendererFlags = AUDIO_FLAG_VOIP_DIRECT;
    streamListener = streamListenerHolder;
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
}

void RendererInServerExtUnitTest::TearDown(void) {}

void InitAudioProcessConfig(AudioStreamInfo streamInfo, DeviceType deviceType = DEVICE_TYPE_WIRED_HEADSET,
    int32_t rendererFlags = AUDIO_FLAG_NORMAL, AudioStreamType streamType = STREAM_DEFAULT)
{
    processConfig.streamInfo = streamInfo;
    processConfig.deviceType = deviceType;
    processConfig.rendererInfo.rendererFlags = rendererFlags;
    processConfig.streamType = streamType;
}

/**
 * @tc.name  : Test OnWriteData API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerStandByCheck_001
 * @tc.desc  : Test StandByCheck API.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerStandByCheck_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_GAME;
    auto server = std::make_shared<RendererInServer>(processConfig, stateListener);
    ASSERT_TRUE(server != nullptr);

    server->managerType_ = VOIP_PLAYBACK;
    server->StandByCheck();
}

/**
 * @tc.name  : Test OnWriteData API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerStandByCheck_002
 * @tc.desc  : Test StandByCheck API.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerStandByCheck_002, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_GAME;
    auto server = std::make_shared<RendererInServer>(processConfig, stateListener);
    ASSERT_TRUE(server != nullptr);

    server->managerType_ = DUP_PLAYBACK;
    server->standByEnable_ = false;
    server->playerDfx_ = nullptr;
    server->standByCounter_ = 100;
    server->StandByCheck();
    EXPECT_TRUE(server->standByEnable_);
}

/**
 * @tc.name  : Test VolumeHandle API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerVolumeHandle_001
 * @tc.desc  : Test VolumeHandle interface.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerVolumeHandle_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_GAME;
    auto server = std::make_shared<RendererInServer>(processConfig, stateListener);
    ASSERT_TRUE(server != nullptr);

    AudioBufferHolder bufferHolder;
    uint32_t totalSizeInFrame = 10;
    uint32_t byteSizePerFrame = 10;
    server->muteFlag_ = true;
    server->audioServerBuffer_ = std::make_shared<OHAudioBufferBase>(bufferHolder, totalSizeInFrame,
        byteSizePerFrame);
    server->VolumeHandle(bufferDesc);
}

/**
 * @tc.name  : Test VolumeHandle API
 * @tc.type  : FUNC
 * @tc.number: InnerCaptureOtherStream_001
 * @tc.desc  : Test InnerCaptureOtherStream interface.
 */
HWTEST_F(RendererInServerExtUnitTest, InnerCaptureOtherStream_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_GAME;
    auto server = std::make_shared<RendererInServer>(processConfig, stateListener);
    ASSERT_TRUE(server != nullptr);

    CaptureInfo captureInfo;
    captureInfo.isInnerCapEnabled = true;
    captureInfo.dupStream = nullptr;
    int32_t innerCapId = 1;
    server->InnerCaptureOtherStream(bufferDesc, captureInfo, innerCapId);

    captureInfo.dupStream = std::make_shared<ProRendererStreamImpl>(processConfig, true);
    server->InnerCaptureOtherStream(bufferDesc, captureInfo, innerCapId);

    server->renderEmptyCountForInnerCap_ = 1;
    server->InnerCaptureOtherStream(bufferDesc, captureInfo, innerCapId);
}

/**
 * @tc.name  : Test VolumeHandle API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerUpdateWriteIndex_001
 * @tc.desc  : Test UpdateWriteIndex interface.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerUpdateWriteIndex_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_GAME;
    auto server = std::make_shared<RendererInServer>(processConfig, stateListener);
    ASSERT_TRUE(server != nullptr);

    server->managerType_ = PLAYBACK;
    server->needForceWrite_ = 1;
    server->spanSizeInByte_ = 10;
    server->stream_ = std::make_shared<ProRendererStreamImpl>(processConfig, true);
    auto ret = server->UpdateWriteIndex();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test VolumeHandle API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerUpdateWriteIndex_002
 * @tc.desc  : Test UpdateWriteIndex interface.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerUpdateWriteIndex_002, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_GAME;
    auto server = std::make_shared<RendererInServer>(processConfig, stateListener);
    ASSERT_TRUE(server != nullptr);

    server->managerType_ = PLAYBACK;
    server->needForceWrite_ = 5;
    server->spanSizeInByte_ = -1;
    server->stream_ = std::make_shared<ProRendererStreamImpl>(processConfig, true);
    auto ret = server->UpdateWriteIndex();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Start API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerStart_001
 * @tc.desc  : Test Start.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerStart_001, TestSize.Level1)
{
    auto server = std::make_shared<RendererInServer>(processConfig, streamListener);
    ASSERT_TRUE(server != nullptr);

    int32_t ret = server->Init();
    server->standByEnable_ = true;
    server->OnStatusUpdate(OPERATION_PAUSED);
    server->playerDfx_ = nullptr;

    ret = server->Start();
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test VolumeHandle API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerdualToneStreamInStart_001
 * @tc.desc  : Test dualToneStreamInStart interface.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerdualToneStreamInStart_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_ULTRASONIC;
    auto server = std::make_shared<RendererInServer>(processConfig, stateListener);
    ASSERT_TRUE(server != nullptr);

    server->dualToneStreamInStart();

    server->isDualToneEnabled_ = true;
    server->dualToneStreamInStart();

    server->isDualToneEnabled_ = false;
    server->dualToneStream_ = std::make_shared<ProRendererStreamImpl>(processConfig, true);
    server->dualToneStreamInStart();
}

/**
 * @tc.name  : Test VolumeHandle API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerdualToneStreamInStart_002
 * @tc.desc  : Test dualToneStreamInStart interface.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerdualToneStreamInStart_002, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_ULTRASONIC;
    auto server = std::make_shared<RendererInServer>(processConfig, stateListener);
    ASSERT_TRUE(server != nullptr);

    server->isDualToneEnabled_ = true;
    server->stream_ = std::make_shared<ProRendererStreamImpl>(processConfig, true);
    server->dualToneStream_ = std::make_shared<ProRendererStreamImpl>(processConfig, true);
    server->dualToneStreamInStart();
}

/**
 * @tc.name  : Test RendererInServer API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerPause_001
 * @tc.desc  : Test Pause interface.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerPause_001, TestSize.Level1)
{
    AudioBufferHolder bufferHolder;
    uint32_t totalSizeInFrame = 10;
    uint32_t byteSizePerFrame = 10;
    AudioProcessConfig processConfig;
    processConfig.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_ULTRASONIC;
    auto server = std::make_shared<RendererInServer>(processConfig, stateListener);
    ASSERT_TRUE(server != nullptr);

    server->status_ = I_STATUS_STARTED;
    server->standByEnable_ = true;
    server->playerDfx_ = nullptr;
    server->audioServerBuffer_ = std::make_shared<OHAudioBufferBase>(bufferHolder, totalSizeInFrame,
        byteSizePerFrame);
    server->audioServerBuffer_->basicBufferInfo_ = std::make_shared<BasicBufferInfo>().get();
    server->audioServerBuffer_->basicBufferInfo_->streamStatus = STREAM_IDEL;
    server->stream_ = std::make_shared<ProRendererStreamImpl>(processConfig, true);
    auto ret = server->Pause();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test RendererInServer API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerPause_002
 * @tc.desc  : Test Pause interface.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerPause_002, TestSize.Level1)
{
    AudioBufferHolder bufferHolder;
    uint32_t totalSizeInFrame = 10;
    uint32_t byteSizePerFrame = 10;
    AudioProcessConfig processConfig;
    processConfig.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_ULTRASONIC;
    auto server = std::make_shared<RendererInServer>(processConfig, stateListener);
    ASSERT_TRUE(server != nullptr);

    server->status_ = I_STATUS_STARTED;
    server->standByEnable_ = true;
    server->audioServerBuffer_ = std::make_shared<OHAudioBufferBase>(bufferHolder, totalSizeInFrame,
        byteSizePerFrame);
    server->audioServerBuffer_->basicBufferInfo_ = std::make_shared<BasicBufferInfo>().get();
    server->audioServerBuffer_->basicBufferInfo_->streamStatus = STREAM_IDEL;
    server->stream_ = std::make_shared<ProRendererStreamImpl>(processConfig, true);

    AppInfo appInfo;
    uint32_t index = 0;
    server->playerDfx_ = std::make_unique<PlayerDfxWriter>(appInfo, index);
    auto ret = server->Pause();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test RendererInServer API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerRelease_001
 * @tc.desc  : Test Release interface.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerRelease_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.audioMode = AUDIO_MODE_RECORD;
    processConfig.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_ULTRASONIC;
    auto server = std::make_shared<RendererInServer>(processConfig, stateListener);
    ASSERT_TRUE(server != nullptr);

    server->status_ = I_STATUS_STARTING;
    server->Release();
}

/**
 * @tc.name  : Test RendererInServer API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetLowPowerVolume_001
 * @tc.desc  : Test SetLowPowerVolume interface.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerSetLowPowerVolume_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.audioMode = AUDIO_MODE_RECORD;
    processConfig.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_ULTRASONIC;
    auto server = std::make_shared<RendererInServer>(processConfig, stateListener);
    ASSERT_TRUE(server != nullptr);

    server->captureInfos_[0].isInnerCapEnabled = false;
    server->captureInfos_[1].isInnerCapEnabled = true;
    server->captureInfos_[1].dupStream = std::make_shared<ProRendererStreamImpl>(processConfig, true);
    server->stream_ = std::make_shared<ProRendererStreamImpl>(processConfig, true);

    server->isDualToneEnabled_ = true;
    server->offloadEnable_ = true;

    float volume = 0.5f;
    auto ret = server->SetLowPowerVolume(volume);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test RendererInServer API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerDisableInnerCap_001
 * @tc.desc  : Test DisableInnerCap interface.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerDisableInnerCap_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_ULTRASONIC;
    auto server = std::make_shared<RendererInServer>(processConfig, stateListener);
    ASSERT_TRUE(server != nullptr);

    int32_t innerCapId = 0;
    server->captureInfos_[innerCapId].isInnerCapEnabled = true;
    auto ret = server->DisableInnerCap(innerCapId);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test RendererInServer API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetOffloadMode_001
 * @tc.desc  : Test SetOffloadMode interface.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerSetOffloadMode_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_ULTRASONIC;
    auto server = std::make_shared<RendererInServer>(processConfig, stateListener);
    ASSERT_TRUE(server != nullptr);

    server->captureInfos_[0].isInnerCapEnabled = false;
    server->captureInfos_[0].dupStream = std::make_shared<ProRendererStreamImpl>(processConfig, true);
    server->isDualToneEnabled_ = true;
    server->dualToneStream_ = std::make_shared<ProRendererStreamImpl>(processConfig, true);
    server->stream_ = std::make_shared<ProRendererStreamImpl>(processConfig, true);

    int32_t state = 0;
    bool isAppBack = false;
    auto ret = server->SetOffloadMode(state, isAppBack);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test RendererInServer API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerOffloadSetVolumeInner_001
 * @tc.desc  : Test OffloadSetVolumeInner interface.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerOffloadSetVolumeInner_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_ULTRASONIC;
    auto server = std::make_shared<RendererInServer>(processConfig, stateListener);
    ASSERT_TRUE(server != nullptr);

    server->stream_ = std::make_shared<ProRendererStreamImpl>(processConfig, true);
    auto ret = server->OffloadSetVolumeInner();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test RendererInServer API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerIsHighResolution_001
 * @tc.desc  : Test IsHighResolution interface.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerIsHighResolution_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.deviceType = DEVICE_TYPE_WIRED_HEADSET;
    processConfig.streamType = STREAM_MUSIC;
    processConfig.streamInfo.samplingRate = SAMPLE_RATE_44100;
    processConfig.streamInfo.format = SAMPLE_S16LE;
    processConfig.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_ULTRASONIC;
    auto server = std::make_shared<RendererInServer>(processConfig, stateListener);
    ASSERT_TRUE(server != nullptr);

    auto ret = server->IsHighResolution();
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test RendererInServer API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerIsHighResolution_002
 * @tc.desc  : Test IsHighResolution interface.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerIsHighResolution_002, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.deviceType = DEVICE_TYPE_WIRED_HEADSET;
    processConfig.streamType = STREAM_MUSIC;
    processConfig.streamInfo.samplingRate = SAMPLE_RATE_44100;
    processConfig.streamInfo.format = SAMPLE_F32LE;
    processConfig.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_ULTRASONIC;
    auto server = std::make_shared<RendererInServer>(processConfig, stateListener);
    ASSERT_TRUE(server != nullptr);

    auto ret = server->IsHighResolution();
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test RendererInServer API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerIsHighResolution_003
 * @tc.desc  : Test IsHighResolution interface.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerIsHighResolution_003, TestSize.Level1)
{
    int rate = 193000;
    AudioProcessConfig processConfig;
    processConfig.deviceType = DEVICE_TYPE_WIRED_HEADSET;
    processConfig.streamType = STREAM_MUSIC;
    processConfig.streamInfo.samplingRate = static_cast<AudioSamplingRate>(rate);
    processConfig.streamInfo.format = SAMPLE_F32LE;
    processConfig.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_ULTRASONIC;
    auto server = std::make_shared<RendererInServer>(processConfig, stateListener);
    ASSERT_TRUE(server != nullptr);

    auto ret = server->IsHighResolution();
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test RendererInServer API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetSilentModeAndMixWithOthers_001
 * @tc.desc  : Test SetSilentModeAndMixWithOthers interface.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerSetSilentModeAndMixWithOthers_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_ULTRASONIC;
    auto server = std::make_shared<RendererInServer>(processConfig, stateListener);
    ASSERT_TRUE(server != nullptr);

    server->captureInfos_[0].isInnerCapEnabled = false;
    server->captureInfos_[0].dupStream = nullptr;
    server->captureInfos_[1].isInnerCapEnabled = true;
    server->captureInfos_[1].dupStream = nullptr;
    server->captureInfos_[2].isInnerCapEnabled = false;
    server->captureInfos_[2].dupStream = std::make_shared<ProRendererStreamImpl>(processConfig, true);
    server->captureInfos_[3].isInnerCapEnabled = true;
    server->captureInfos_[3].dupStream = std::make_shared<ProRendererStreamImpl>(processConfig, true);

    server->isDualToneEnabled_ = true;
    server->offloadEnable_ = true;
    server->stream_ = std::make_shared<ProRendererStreamImpl>(processConfig, true);

    auto ret = server->SetSilentModeAndMixWithOthers(false);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test RendererInServer API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetClientVolume_001
 * @tc.desc  : Test SetClientVolume interface.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerSetClientVolume_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_ULTRASONIC;
    auto server = std::make_shared<RendererInServer>(processConfig, stateListener);
    ASSERT_TRUE(server != nullptr);

    AudioBufferHolder bufferHolder;
    uint32_t totalSizeInFrame = 10;
    uint32_t byteSizePerFrame = 10;
    server->audioServerBuffer_ = std::make_shared<OHAudioBufferBase>(bufferHolder, totalSizeInFrame,
        byteSizePerFrame);
    server->playerDfx_ = nullptr;

    auto ret = server->SetClientVolume();
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test RendererInServer API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetClientVolume_002
 * @tc.desc  : Test SetClientVolume interface.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerSetClientVolume_002, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_ULTRASONIC;
    auto server = std::make_shared<RendererInServer>(processConfig, stateListener);
    ASSERT_TRUE(server != nullptr);

    AudioBufferHolder bufferHolder;
    uint32_t totalSizeInFrame = 10;
    uint32_t byteSizePerFrame = 10;
    server->audioServerBuffer_ = std::make_shared<OHAudioBufferBase>(bufferHolder, totalSizeInFrame,
        byteSizePerFrame);

    AppInfo appInfo;
    uint32_t index = 0;
    server->playerDfx_ = std::make_unique<PlayerDfxWriter>(appInfo, index);
    ASSERT_TRUE(server->playerDfx_ != nullptr);

    server->captureInfos_[0].isInnerCapEnabled = false;
    server->captureInfos_[0].dupStream = nullptr;
    server->captureInfos_[1].isInnerCapEnabled = true;
    server->captureInfos_[1].dupStream = nullptr;
    server->captureInfos_[2].isInnerCapEnabled = false;
    server->captureInfos_[2].dupStream = std::make_shared<ProRendererStreamImpl>(processConfig, true);
    server->captureInfos_[3].isInnerCapEnabled = true;
    server->captureInfos_[3].dupStream = std::make_shared<ProRendererStreamImpl>(processConfig, true);

    server->isDualToneEnabled_ = true;
    server->offloadEnable_ = true;
    server->stream_ = std::make_shared<ProRendererStreamImpl>(processConfig, true);

    auto ret = server->SetClientVolume();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test RendererInServer API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetSetMute_001
 * @tc.desc  : Test SetMute interface.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerSetSetMute_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_ULTRASONIC;
    auto server = std::make_shared<RendererInServer>(processConfig, stateListener);
    ASSERT_TRUE(server != nullptr);

    server->captureInfos_[0].isInnerCapEnabled = false;
    server->captureInfos_[0].dupStream = nullptr;
    server->captureInfos_[1].isInnerCapEnabled = true;
    server->captureInfos_[1].dupStream = nullptr;
    server->captureInfos_[2].isInnerCapEnabled = false;
    server->captureInfos_[2].dupStream = std::make_shared<ProRendererStreamImpl>(processConfig, true);
    server->captureInfos_[3].isInnerCapEnabled = true;
    server->captureInfos_[3].dupStream = std::make_shared<ProRendererStreamImpl>(processConfig, true);

    server->isDualToneEnabled_ = true;
    server->offloadEnable_ = true;
    server->stream_ = std::make_shared<ProRendererStreamImpl>(processConfig, true);

    auto ret = server->SetMute(true);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test RendererInServer API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetDuckFactor_001
 * @tc.desc  : Test SetDuckFactor interface.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerSetDuckFactor_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_ULTRASONIC;
    auto server = std::make_shared<RendererInServer>(processConfig, stateListener);
    ASSERT_TRUE(server != nullptr);

    float duckFactor = -0.5f;
    auto ret = server->SetDuckFactor(duckFactor);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    duckFactor = 2.0f;
    ret = server->SetDuckFactor(duckFactor);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test RendererInServer API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetDuckFactor_002
 * @tc.desc  : Test SetDuckFactor interface.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerSetDuckFactor_002, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_ULTRASONIC;
    auto server = std::make_shared<RendererInServer>(processConfig, stateListener);
    ASSERT_TRUE(server != nullptr);

    server->captureInfos_[0].isInnerCapEnabled = false;
    server->captureInfos_[0].dupStream = nullptr;
    server->captureInfos_[1].isInnerCapEnabled = true;
    server->captureInfos_[1].dupStream = nullptr;
    server->captureInfos_[2].isInnerCapEnabled = false;
    server->captureInfos_[2].dupStream = std::make_shared<ProRendererStreamImpl>(processConfig, true);
    server->captureInfos_[3].isInnerCapEnabled = true;
    server->captureInfos_[3].dupStream = std::make_shared<ProRendererStreamImpl>(processConfig, true);

    server->isDualToneEnabled_ = true;
    server->offloadEnable_ = true;
    server->stream_ = std::make_shared<ProRendererStreamImpl>(processConfig, true);

    float duckFactor = 0.5f;
    auto ret = server->SetDuckFactor(duckFactor);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test RendererInServer API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetSetNonInterruptMute_001
 * @tc.desc  : Test SetDuckFactor interface.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerSetSetNonInterruptMute_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_ULTRASONIC;
    auto server = std::make_shared<RendererInServer>(processConfig, stateListener);
    ASSERT_TRUE(server != nullptr);

    server->captureInfos_[0].isInnerCapEnabled = false;
    server->captureInfos_[0].dupStream = nullptr;
    server->captureInfos_[1].isInnerCapEnabled = true;
    server->captureInfos_[1].dupStream = nullptr;
    server->captureInfos_[2].isInnerCapEnabled = false;
    server->captureInfos_[2].dupStream = std::make_shared<ProRendererStreamImpl>(processConfig, true);
    server->captureInfos_[3].isInnerCapEnabled = true;
    server->captureInfos_[3].dupStream = std::make_shared<ProRendererStreamImpl>(processConfig, true);

    server->isDualToneEnabled_ = true;
    server->offloadEnable_ = true;
    server->stream_ = std::make_shared<ProRendererStreamImpl>(processConfig, true);

    bool muteFlag = false;
    server->SetNonInterruptMute(muteFlag);
}

/**
 * @tc.name  : Test RendererInServer API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerRestoreSession_001
 * @tc.desc  : Test SetDuckFactor interface.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerRestoreSession_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_ULTRASONIC;
    auto server = std::make_shared<RendererInServer>(processConfig, stateListener);
    ASSERT_TRUE(server != nullptr);

    AudioBufferHolder bufferHolder;
    uint32_t totalSizeInFrame = 10;
    uint32_t byteSizePerFrame = 10;
    server->muteFlag_ = true;
    server->audioServerBuffer_ = std::make_shared<OHAudioBufferBase>(bufferHolder, totalSizeInFrame,
        byteSizePerFrame);

    RestoreInfo restoreInfo;
    auto ret = server->RestoreSession(restoreInfo);
    EXPECT_EQ(ret, RESTORE_ERROR);

    server->audioServerBuffer_->basicBufferInfo_ = std::make_shared<BasicBufferInfo>().get();
    ret = server->RestoreSession(restoreInfo);
    EXPECT_EQ(ret, NO_NEED_FOR_RESTORE);
}

/**
 * @tc.name  : Test RendererInServer API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetAudioHapticsSyncId_001
 * @tc.desc  : Test SetAudioHapticsSyncId interface.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerSetAudioHapticsSyncId_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_ULTRASONIC;
    auto server = std::make_shared<RendererInServer>(processConfig, stateListener);
    ASSERT_TRUE(server != nullptr);

    int32_t syncId = 100;
    server->SetAudioHapticsSyncId(syncId);
    EXPECT_EQ(server->audioHapticsSyncId_, syncId);
}
} // namespace AudioStandard
} // namespace OHOS
