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
#include "audio_volume.h"
#include "i_renderer_stream.h"
#include "audio_utils.h"
#include "hpae_renderer_stream_impl.h"
#include "hpae_adapter_manager.h"
#include "hpae_soft_link.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

static std::shared_ptr<IStreamListener> stateListener;
static std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
static std::shared_ptr<HPAE::IHpaeSoftLink>  softLink =
    HPAE::IHpaeSoftLink::CreateSoftLink(1, 1, HPAE::SoftLinkMode::OFFLOADINNERCAP_AID);
static std::shared_ptr<RendererInServer> rendererInServer;
static std::shared_ptr<OHAudioBuffer> buffer;
static std::weak_ptr<IStreamListener> streamListener;
static constexpr int32_t ONE_MINUTE = 60;
const uint64_t TEST_FRAMEPOS = 123456;
const uint64_t TEST_TIMESTAMP = 111111;
const float IN_VOLUME_RANGE = 0.5f;
const uint32_t TEST_STREAMINDEX = 64;
const uint32_t TEST_SESSIONID = 64;
const int32_t TEST_UID = 10;

static AudioProcessConfig processConfig;
static BufferDesc bufferDesc;
static int32_t length = 10000;

static AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
    AudioChannelLayout::CH_LAYOUT_UNKNOWN);

static AudioProcessConfig GetInnerCapConfig()
{
    AudioProcessConfig config;
    config.appInfo.appUid = TEST_UID;
    config.appInfo.appPid = TEST_UID;
    config.streamInfo.format = SAMPLE_S32LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    config.innerCapId = 1;
    config.originalSessionId = TEST_SESSIONID;
    return config;
}
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

std::shared_ptr<IRendererStream> RendererInServerExtUnitTest::CreateHpaeRendererStream()
{
    std::shared_ptr<HpaeAdapterManager> adapterManager = std::make_shared<HpaeAdapterManager>(DUP_PLAYBACK);
    AudioProcessConfig processConfig = GetInnerCapConfig();
    std::shared_ptr<IRendererStream> rendererStream = adapterManager->CreateRendererStream(processConfig, "");
    return rendererStream;
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
    uint32_t streamIndex_ = 0;
    auto streamCallbacks = std::make_shared<StreamCallbacks>(streamIndex_);
    server->innerCapIdToDupStreamCallbackMap_.insert({innerCapId, streamCallbacks});
    server->innerCapIdToDupStreamCallbackMap_[innerCapId]->GetDupRingBuffer() = AudioRingCache::Create(length);
    
    auto buffer = std::make_unique<uint8_t []>(length);
    BufferDesc emptyBufferDesc = {buffer.get(), length, length};
    memset_s(emptyBufferDesc.buffer, emptyBufferDesc.bufLength, 0, emptyBufferDesc.bufLength);
    server->WriteDupBufferInner(emptyBufferDesc, innerCapId);
    int8_t inputData[length + 1];
    server->innerCapIdToDupStreamCallbackMap_[innerCapId]->OnWriteData(inputData, length + 1);
    server->innerCapIdToDupStreamCallbackMap_[innerCapId]->OnWriteData(inputData, length - 1);
    server->InnerCaptureOtherStream(bufferDesc, captureInfo, innerCapId);

    IStreamManager::GetDupPlaybackManager().CreateRender(processConfig, captureInfo.dupStream);
    server->InnerCaptureOtherStream(bufferDesc, captureInfo, innerCapId);

    server->renderEmptyCountForInnerCapToInnerCapIdMap_[innerCapId] = 1;
    server->InnerCaptureOtherStream(bufferDesc, captureInfo, innerCapId);
    server->InnerCaptureOtherStream(bufferDesc, captureInfo, innerCapId + 1);
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

    server->Init();
    server->offloadEnable_ = true;
    server->stream_ = std::make_shared<ProRendererStreamImpl>(processConfig, true);
    ret = server->stream_->SetOffloadMode(1, true);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test RendererInServer API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerDisableInnerCap_002
 * @tc.desc  : Test DisableInnerCap interface.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerDisableInnerCap_002, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_ULTRASONIC;
    processConfig.rendererInfo.originalFlag = AUDIO_FLAG_PCM_OFFLOAD;
    processConfig.streamType = STREAM_MOVIE;
    auto server = std::make_shared<RendererInServer>(processConfig, stateListener);
    ASSERT_TRUE(server != nullptr);
    server->Init();
    
    int32_t innerCapId = 0;
    server->offloadEnable_ = true;
    server->stream_->SetOffloadMode(1, true);
    auto ret = server->DisableInnerCap(innerCapId);
    EXPECT_NE(SUCCESS, ret);
    
    server->offloadEnable_ = true;
    server->stream_->UnsetOffloadMode();
    server->DisableInnerCap(innerCapId);

    server->offloadEnable_ = false;
    server->stream_->SetOffloadMode(1, true);
    server->DisableInnerCap(innerCapId);

    server->offloadEnable_ = false;
    server->stream_->UnsetOffloadMode();
    server->DisableInnerCap(innerCapId);

    EXPECT_EQ(server->softLinkInfos_[innerCapId].isSoftLinkEnabled, false);
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
 * @tc.number: RemoveIdForInjector_001
 * @tc.desc  : Test RemoveIdForInjector interface.
 */
HWTEST_F(RendererInServerExtUnitTest, RemoveIdForInjector_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_ULTRASONIC;
    auto server = std::make_shared<RendererInServer>(processConfig, stateListener);
    EXPECT_NE(nullptr, server);
    server->lastTarget_ = INJECT_TO_VOICE_COMMUNICATION_CAPTURE;
    server->RemoveIdForInjector();
}

/**
 * @tc.name  : Test RendererInServer API
 * @tc.type  : FUNC
 * @tc.number: RemoveIdForInjector_002
 * @tc.desc  : Test RemoveIdForInjector interface.
 */
HWTEST_F(RendererInServerExtUnitTest, RemoveIdForInjector_002, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_ULTRASONIC;
    auto server = std::make_shared<RendererInServer>(processConfig, stateListener);
    EXPECT_NE(nullptr, server);
    server->lastTarget_ = NORMAL_PLAYBACK;
    server->RemoveIdForInjector();
}

/**
 * @tc.name  : Test WriteMuteDataSysEvent API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerWriteMuteDataSysEvent_001
 * @tc.desc  : Test WriteMuteDataSysEvent when buffer[0] is 0 and startMuteTime_ is 0.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerWriteMuteDataSysEvent_001, TestSize.Level4)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    bufferDesc.buffer[0] = 0;
    rendererInServer->WriteMuteDataSysEvent(bufferDesc);
    EXPECT_EQ(false, rendererInServer->isInSilentState_);
}

/**
 * @tc.name  : Test WriteMuteDataSysEvent API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerWriteMuteDataSysEvent_002
 * @tc.desc  : Test WriteMuteDataSysEvent when buffer[0] is 0 and startMuteTime_ is not 0.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerWriteMuteDataSysEvent_002, TestSize.Level4)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    uint8_t bufferTest = 0;
    bufferDesc.buffer = &bufferTest;
    rendererInServer->startMuteTime_ = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    rendererInServer->WriteMuteDataSysEvent(bufferDesc);
    EXPECT_EQ(false, rendererInServer->isInSilentState_);
}

/**
 * @tc.name  : Test WriteMuteDataSysEvent API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerWriteMuteDataSysEvent_003
 * @tc.desc  : Test WriteMuteDataSysEvent when buffer[0] is 0 and isInSilentState_ is not 1.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerWriteMuteDataSysEvent_003, TestSize.Level4)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    bufferDesc.buffer[0] = 0;
    rendererInServer->isInSilentState_ = 0;
    rendererInServer->startMuteTime_ = 1;
    rendererInServer->WriteMuteDataSysEvent(bufferDesc);
    EXPECT_EQ(0, rendererInServer->startMuteTime_);
}

/**
 * @tc.name  : Test WriteMuteDataSysEvent API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerWriteMuteDataSysEvent_004
 * @tc.desc  : Test WriteMuteDataSysEvent when buffer[0] is not 0.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerWriteMuteDataSysEvent_004, TestSize.Level4)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    bufferDesc.buffer[0] = 1;
    rendererInServer->WriteMuteDataSysEvent(bufferDesc);
    EXPECT_EQ(0, rendererInServer->startMuteTime_);
    EXPECT_EQ(false, rendererInServer->isInSilentState_);
}

/**
 * @tc.name  : Test WriteMuteDataSysEvent API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerWriteMuteDataSysEvent_005
 * @tc.desc  : Test WriteMuteDataSysEvent when buffer[0] is not 0 and startMuteTime_ is not 0 and isInSilentState_ is 0.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerWriteMuteDataSysEvent_005, TestSize.Level4)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    bufferDesc.buffer[0] = 1;
    rendererInServer->startMuteTime_ = 1;
    rendererInServer->isInSilentState_ = 0;
    rendererInServer->WriteMuteDataSysEvent(bufferDesc);
    EXPECT_EQ(0, rendererInServer->startMuteTime_);
    EXPECT_EQ(false, rendererInServer->isInSilentState_);
}

/**
 * @tc.name  : Test WriteMuteDataSysEvent API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerWriteMuteDataSysEvent_006
 * @tc.desc  : Test WriteMuteDataSysEvent when buffer[0] is not 0 and startMuteTime_ is not 0 and isInSilentState_ is 0.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerWriteMuteDataSysEvent_006, TestSize.Level4)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_U8, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);
    uint8_t buffer[10] = {0};
    size_t bufferSize = 10;
    bufferDesc.buffer = buffer;
    bufferDesc.bufLength = bufferSize;

    bufferDesc.buffer[0] = 0;
    rendererInServer->startMuteTime_ =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) - ONE_MINUTE - 1;
    rendererInServer->isInSilentState_ = false;
    rendererInServer->WriteMuteDataSysEvent(bufferDesc);
    EXPECT_EQ(true, rendererInServer->isInSilentState_);
}

/**
* @tc.name  : Test WriteMuteDataSysEvent API
* @tc.type  : FUNC
* @tc.number: RendererInServerWriteMuteDataSysEvent_007
* @tc.desc  : Test WriteMuteDataSysEvent when buffer[0] is not 0 and startMuteTime_ is not 0 and isInSilentState_ is 1.
*/
HWTEST_F(RendererInServerExtUnitTest, RendererInServerWriteMuteDataSysEvent_007, TestSize.Level4)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);
    uint8_t buffer[10] = {0};
    size_t bufferSize = 10;
    bufferDesc.buffer = buffer;
    bufferDesc.bufLength = bufferSize;

    bufferDesc.buffer[0] = 1;
    rendererInServer->startMuteTime_ = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    rendererInServer->isInSilentState_ = true;
    rendererInServer->WriteMuteDataSysEvent(bufferDesc);
    EXPECT_EQ(false, rendererInServer->isInSilentState_);
}

/**
 * @tc.name  : Test GetAudioTime API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerGetAudioTime_004
 * @tc.desc  : Test GetAudioTime interface, status_ is not I_STATUS_STOPPED, resetTime_ is true.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerGetAudioTime_004, TestSize.Level1)
{
    processConfig.rendererInfo.rendererFlags = AUDIO_FLAG_NORMAL;
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->Init();
    rendererInServer->Start();
    rendererInServer->resetTime_ = true;
    uint64_t framePos = TEST_FRAMEPOS;
    uint64_t timestamp = TEST_TIMESTAMP;
    int32_t ret = rendererInServer->GetAudioTime(framePos, timestamp);
    EXPECT_EQ(false, rendererInServer->resetTime_);
    EXPECT_EQ(SUCCESS, ret);
}


/**
 * @tc.name  : Test SetLowPowerVolume API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetLowPowerVolume_004
 * @tc.desc  : Test SetLowPowerVolume interface, Set volume is IN_VOLUME_RANGE.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerSetLowPowerVolume_004, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    float volume = IN_VOLUME_RANGE;
    int32_t ret = rendererInServer->SetLowPowerVolume(volume);

    rendererInServer->isDualToneEnabled_ = true;
    rendererInServer->offloadEnable_ = true;
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetLowPowerVolume API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetLowPowerVolume_005
 * @tc.desc  : Test SetLowPowerVolume interface.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerSetLowPowerVolume_005, TestSize.Level1)
{
    rendererInServer->softLinkInfos_[0].isSoftLinkEnabled = false;
    rendererInServer->softLinkInfos_[0].softLink = nullptr;
    rendererInServer->softLinkInfos_[1].isSoftLinkEnabled = true;
    rendererInServer->softLinkInfos_[1].softLink = nullptr;
    rendererInServer->softLinkInfos_[2].isSoftLinkEnabled = false;
    rendererInServer->softLinkInfos_[2].softLink =
        std::make_shared<HPAE::HpaeSoftLink>(1, 1, HPAE::SoftLinkMode::OFFLOADINNERCAP_AID);
    rendererInServer->softLinkInfos_[3].isSoftLinkEnabled = true;
    rendererInServer->softLinkInfos_[3].softLink =
        std::make_shared<HPAE::HpaeSoftLink>(1, 1, HPAE::SoftLinkMode::OFFLOADINNERCAP_AID);

    float volume = IN_VOLUME_RANGE;
    int32_t ret = rendererInServer->SetLowPowerVolume(volume);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test OnWriteData API
 * @tc.type  : FUNC
 * @tc.number: StreamCallbacksOnWriteData_002
 * @tc.desc  : Test OnWriteData interface.
 */
HWTEST_F(RendererInServerExtUnitTest, StreamCallbacksOnWriteData_002, TestSize.Level1)
{
    std::shared_ptr<StreamCallbacks> streamCallbacks;
    streamCallbacks = std::make_shared<StreamCallbacks>(TEST_STREAMINDEX);
    EXPECT_NE(nullptr, streamCallbacks);

    auto inputData = new int8_t [10] {1, 2, 3};
    ASSERT_NE(nullptr, inputData);
    int32_t ret = streamCallbacks->OnWriteData(inputData, 3);
    EXPECT_EQ(SUCCESS, ret);

    streamCallbacks->dupRingBuffer_ = AudioRingCache::Create(10);
    ret = streamCallbacks->OnWriteData(inputData, 3);
    EXPECT_EQ(ERROR, ret);
    delete[] inputData;
}

/**
 * @tc.name  : Test OnWriteData API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerOnWriteData_003
 * @tc.desc  : Test OnWriteData API when requestDataLen is not 0,
 * currentReadFrame + requestDataInFrame > currentWriteFrame, offloadEnable_ is false.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerOnWriteData_003, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_FLAG_VOIP_DIRECT);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererInServer->ConfigServerBuffer();
    EXPECT_EQ(SUCCESS, ret);
    rendererInServer->offloadEnable_ = false;

    auto inputData = new int8_t[5] {1, 2, 3, 4, 5};
    ASSERT_NE(inputData, nullptr);
    ret = rendererInServer->OnWriteData(inputData, 5);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);
    delete[] inputData;
}

/**
* @tc.name  : Test OnWriteData API
* @tc.type  : FUNC
* @tc.number: RendererInServerOnWriteData_004
* @tc.desc  : Test OnWriteData API when requestDataLen is not 0,
* currentReadFrame + requestDataInFrame > currentWriteFrame, offloadEnable_ is true.
*/
HWTEST_F(RendererInServerExtUnitTest, RendererInServerOnWriteData_004, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_FLAG_VOIP_DIRECT);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererInServer->ConfigServerBuffer();
    EXPECT_EQ(SUCCESS, ret);
    rendererInServer->offloadEnable_ = true;

    auto inputData = new int8_t[5] {1, 2, 3, 4, 5};
    ASSERT_NE(inputData, nullptr);
    ret = rendererInServer->OnWriteData(inputData, 5);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);
    delete[] inputData;
}

/**
* @tc.name  : Test OnWriteData API
* @tc.type  : FUNC
* @tc.number: RendererInServerOnWriteData_005
* @tc.desc  : Test OnWriteData API when requestDataLen is 0,
* currentReadFrame + requestDataInFrame > currentWriteFrame, offloadEnable_ is true.
*/
HWTEST_F(RendererInServerExtUnitTest, RendererInServerOnWriteData_005, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_FLAG_VOIP_DIRECT);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererInServer->ConfigServerBuffer();
    EXPECT_EQ(SUCCESS, ret);
    rendererInServer->offloadEnable_ = true;

    auto inputData = new int8_t[5] {1, 2, 3, 4, 5};
    ASSERT_NE(inputData, nullptr);
    ret = rendererInServer->OnWriteData(inputData, 0);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);
    delete[] inputData;
}

/**
* @tc.name  : Test OnWriteData API
* @tc.type  : FUNC
* @tc.number: RendererInServerOnWriteData_006
* @tc.desc  : Test OnWriteData API when requestDataLen is 0,
* currentReadFrame + requestDataInFrame > currentWriteFrame, offloadEnable_ is false.
*/
HWTEST_F(RendererInServerExtUnitTest, RendererInServerOnWriteData_006, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_FLAG_VOIP_DIRECT);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererInServer->ConfigServerBuffer();
    EXPECT_EQ(SUCCESS, ret);
    rendererInServer->offloadEnable_ = false;

    auto inputData = new int8_t[5] {1, 2, 3, 4, 5};
    ASSERT_NE(inputData, nullptr);
    ret = rendererInServer->OnWriteData(inputData, 0);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);
    delete[] inputData;
}

/**
* @tc.name  : Test OnWriteData API
* @tc.type  : FUNC
* @tc.number: RendererInServerOnWriteData_007
* @tc.desc  : Test OnWriteData API when requestDataLen is not 0,
* currentReadFrame + requestDataInFrame <= currentWriteFrame, offloadEnable_ is false.
*/
HWTEST_F(RendererInServerExtUnitTest, RendererInServerOnWriteData_007, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_FLAG_VOIP_DIRECT);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);
    const std::string version = AudioDump::GetInstance().GetVersionType();
    AudioDump::GetInstance().SetVersionType(DumpFileUtil::BETA_VERSION);

    int32_t ret = rendererInServer->Init();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererInServer->ConfigServerBuffer();
    EXPECT_EQ(SUCCESS, ret);
    const int requestDataLen = 5;
    auto inputData = new int8_t[5] {1, 2, 3, 4, 5};
    ASSERT_NE(inputData, nullptr);
    rendererInServer->offloadEnable_ = false;
    size_t requestDataInFrame = requestDataLen / rendererInServer->byteSizePerFrame_;
    uint64_t currentReadFrame = rendererInServer->audioServerBuffer_->GetCurReadFrame();
    rendererInServer->audioServerBuffer_->SetCurWriteFrame(currentReadFrame + requestDataInFrame + 1);

    ret = rendererInServer->OnWriteData(inputData, 2);
    EXPECT_EQ(SUCCESS, ret);
    AudioDump::GetInstance().SetVersionType(version);

    ret = rendererInServer->OnWriteData(inputData, 3);
    EXPECT_EQ(SUCCESS, ret);
    delete[] inputData;
}

/**
* @tc.name  : Test OnWriteData API
* @tc.type  : FUNC
* @tc.number: RendererInServerOnWriteData_008
* @tc.desc  : Test OnWriteData API when requestDataLen is not 0,
* currentReadFrame + requestDataInFrame <= currentWriteFrame.
*/
HWTEST_F(RendererInServerExtUnitTest, RendererInServerOnWriteData_008, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_FLAG_VOIP_DIRECT);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererInServer->ConfigServerBuffer();
    EXPECT_EQ(SUCCESS, ret);
    const int requestDataLen = 2;
    auto inputData = new int8_t[2] {1, 2};
    ASSERT_NE(inputData, nullptr);
    rendererInServer->offloadEnable_ = false;

    uint64_t currentReadFrame = rendererInServer->audioServerBuffer_->GetCurReadFrame();
    rendererInServer->audioServerBuffer_->SetCurWriteFrame(currentReadFrame);
    RingBufferWrapper ringBufferDesc;
    rendererInServer->audioServerBuffer_->GetAllReadableBufferFromPosFrame(currentReadFrame, ringBufferDesc);
    ret = rendererInServer->OnWriteData(inputData, requestDataLen);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
    delete[] inputData;
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

/**
 * @tc.name  : Test RendererInServer
 * @tc.type  : FUNC
 * @tc.number: IsHighResolution_001
 * @tc.desc  : Test IsHighResolution API
 */
HWTEST_F(RendererInServerExtUnitTest, IsHighResolution_001, TestSize.Level1)
{
    AudioProcessConfig tempProcessConfig;
    tempProcessConfig.streamInfo = testStreamInfo;
    tempProcessConfig.streamType = STREAM_MUSIC;
    tempProcessConfig.rendererInfo.pipeType = PIPE_TYPE_DIRECT_MUSIC;
    tempProcessConfig.deviceType = DEVICE_TYPE_SPEAKER;
    tempProcessConfig.rendererInfo.rendererFlags = AUDIO_FLAG_MMAP;
    std::shared_ptr<RendererInServer> tmpRendererInServer;
    tmpRendererInServer = std::make_shared<RendererInServer>(tempProcessConfig, streamListener);
    EXPECT_NE(nullptr, tmpRendererInServer);
    EXPECT_FALSE(tmpRendererInServer->IsHighResolution());
}

/**
 * @tc.name  : Test RendererInServer
 * @tc.type  : FUNC
 * @tc.number: IsHighResolution_002
 * @tc.desc  : Test IsHighResolution API
 */
HWTEST_F(RendererInServerExtUnitTest, IsHighResolution_002, TestSize.Level1)
{
    AudioProcessConfig tempProcessConfig;
    tempProcessConfig.streamInfo = testStreamInfo;
    tempProcessConfig.streamType = STREAM_ALARM;
    tempProcessConfig.rendererInfo.pipeType = PIPE_TYPE_DIRECT_MUSIC;
    tempProcessConfig.deviceType = DEVICE_TYPE_WIRED_HEADSET;
    tempProcessConfig.rendererInfo.rendererFlags = AUDIO_FLAG_MMAP;
    std::shared_ptr<RendererInServer> tmpRendererInServer;
    tmpRendererInServer = std::make_shared<RendererInServer>(tempProcessConfig, streamListener);
    EXPECT_NE(nullptr, tmpRendererInServer);
    EXPECT_FALSE(tmpRendererInServer->IsHighResolution());
}

/**
 * @tc.name  : Test RendererInServer
 * @tc.type  : FUNC
 * @tc.number: IsHighResolution_003
 * @tc.desc  : Test IsHighResolution API
 */
HWTEST_F(RendererInServerExtUnitTest, IsHighResolution_003, TestSize.Level1)
{
    AudioProcessConfig tempProcessConfig;
    tempProcessConfig.streamInfo = testStreamInfo;
    tempProcessConfig.streamType = STREAM_MUSIC;
    tempProcessConfig.rendererInfo.pipeType = PIPE_TYPE_DIRECT_MUSIC;
    tempProcessConfig.deviceType = DEVICE_TYPE_WIRED_HEADSET;
    tempProcessConfig.rendererInfo.rendererFlags = AUDIO_FLAG_MMAP;
    tempProcessConfig.streamInfo.samplingRate = SAMPLE_RATE_44100;
    std::shared_ptr<RendererInServer> tmpRendererInServer;
    tmpRendererInServer = std::make_shared<RendererInServer>(tempProcessConfig, streamListener);
    EXPECT_NE(nullptr, tmpRendererInServer);
    EXPECT_FALSE(tmpRendererInServer->IsHighResolution());
}

/**
 * @tc.name  : Test RendererInServer
 * @tc.type  : FUNC
 * @tc.number: IsHighResolution_004
 * @tc.desc  : Test IsHighResolution API
 */
HWTEST_F(RendererInServerExtUnitTest, IsHighResolution_004, TestSize.Level1)
{
    AudioProcessConfig tempProcessConfig;
    tempProcessConfig.streamInfo = testStreamInfo;
    tempProcessConfig.streamType = STREAM_MUSIC;
    tempProcessConfig.rendererInfo.pipeType = PIPE_TYPE_DIRECT_MUSIC;
    tempProcessConfig.deviceType = DEVICE_TYPE_WIRED_HEADSET;
    tempProcessConfig.rendererInfo.rendererFlags = AUDIO_FLAG_MMAP;
    tempProcessConfig.streamInfo.samplingRate = SAMPLE_RATE_48000;
    tempProcessConfig.streamInfo.format = SAMPLE_S16LE;
    std::shared_ptr<RendererInServer> tmpRendererInServer;
    tmpRendererInServer = std::make_shared<RendererInServer>(tempProcessConfig, streamListener);
    EXPECT_NE(nullptr, tmpRendererInServer);
    EXPECT_TRUE(tmpRendererInServer->IsHighResolution());
}

/**
 * @tc.name  : Test RendererInServer
 * @tc.type  : FUNC
 * @tc.number: IsHighResolution_005
 * @tc.desc  : Test IsHighResolution API
 */
HWTEST_F(RendererInServerExtUnitTest, IsHighResolution_005, TestSize.Level1)
{
    AudioProcessConfig tempProcessConfig;
    tempProcessConfig.streamInfo = testStreamInfo;
    tempProcessConfig.streamType = STREAM_MUSIC;
    tempProcessConfig.rendererInfo.pipeType = PIPE_TYPE_DIRECT_MUSIC;
    tempProcessConfig.deviceType = DEVICE_TYPE_WIRED_HEADSET;
    tempProcessConfig.rendererInfo.rendererFlags = AUDIO_FLAG_MMAP;
    tempProcessConfig.streamInfo.samplingRate = SAMPLE_RATE_192000;
    std::shared_ptr<RendererInServer> tmpRendererInServer;
    tmpRendererInServer = std::make_shared<RendererInServer>(tempProcessConfig, streamListener);
    EXPECT_NE(nullptr, tmpRendererInServer);
    EXPECT_TRUE(tmpRendererInServer->IsHighResolution());
}

/**
 * @tc.name  : Test RendererInServer
 * @tc.type  : FUNC
 * @tc.number: IsHighResolution_006
 * @tc.desc  : Test IsHighResolution API
 */
HWTEST_F(RendererInServerExtUnitTest, IsHighResolution_006, TestSize.Level1)
{
    AudioProcessConfig tempProcessConfig;
    tempProcessConfig.streamInfo = testStreamInfo;
    tempProcessConfig.streamType = STREAM_MUSIC;
    tempProcessConfig.rendererInfo.pipeType = PIPE_TYPE_DIRECT_MUSIC;
    tempProcessConfig.deviceType = DEVICE_TYPE_WIRED_HEADSET;
    tempProcessConfig.rendererInfo.rendererFlags = AUDIO_FLAG_MMAP;
    tempProcessConfig.streamInfo.samplingRate = SAMPLE_RATE_48000;
    tempProcessConfig.streamInfo.format = SAMPLE_S24LE;
    std::shared_ptr<RendererInServer> tmpRendererInServer;
    tmpRendererInServer = std::make_shared<RendererInServer>(tempProcessConfig, streamListener);
    EXPECT_NE(nullptr, tmpRendererInServer);
    EXPECT_TRUE(tmpRendererInServer->IsHighResolution());
}

/**
 * @tc.name  : Test GetEAC3ControlParam
 * @tc.type  : FUNC
 * @tc.number: GetEAC3ControlParam_001
 * @tc.desc  : Test GetEAC3ControlParam API
 */
HWTEST_F(RendererInServerExtUnitTest, GetEAC3ControlParam_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);
    int32_t eac3TestFlag = 1;
    GetSysPara("persist.multimedia.eac3test", eac3TestFlag);
    rendererInServer->GetEAC3ControlParam();
    EXPECT_NE(rendererInServer->managerType_, EAC3_PLAYBACK);
}

/**
 * @tc.name  : Test GetEAC3ControlParam
 * @tc.type  : FUNC
 * @tc.number: GetEAC3ControlParam_002
 * @tc.desc  : Test GetEAC3ControlParam API
 */
HWTEST_F(RendererInServerExtUnitTest, GetEAC3ControlParam_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);
    int32_t eac3TestFlag = 0;
    GetSysPara("persist.multimedia.eac3test", eac3TestFlag);
    rendererInServer->GetEAC3ControlParam();
    EXPECT_NE(rendererInServer->managerType_, EAC3_PLAYBACK);
}

/**
 * @tc.name  : Test GetPlaybackManager
 * @tc.type  : FUNC
 * @tc.number: GetPlaybackManager_001
 * @tc.desc  : Test GetPlaybackManager API
 */
HWTEST_F(RendererInServerExtUnitTest, GetPlaybackManager_001, TestSize.Level1)
{
    IStreamManager &manager = IStreamManager::GetPlaybackManager(DIRECT_PLAYBACK);
    EXPECT_NE(&manager, nullptr);
}

/**
 * @tc.name  : Test GetPlaybackManager
 * @tc.type  : FUNC
 * @tc.number: GetPlaybackManager_002
 * @tc.desc  : Test GetPlaybackManager API
 */
HWTEST_F(RendererInServerExtUnitTest, GetPlaybackManager_002, TestSize.Level1)
{
    IStreamManager &manager = IStreamManager::GetPlaybackManager(EAC3_PLAYBACK);
    EXPECT_NE(&manager, nullptr);
}

/**
 * @tc.name  : Test GetPlaybackManager
 * @tc.type  : FUNC
 * @tc.number: GetPlaybackManager_003
 * @tc.desc  : Test GetPlaybackManager API
 */
HWTEST_F(RendererInServerExtUnitTest, GetPlaybackManager_003, TestSize.Level1)
{
    IStreamManager &manager = IStreamManager::GetPlaybackManager(VOIP_PLAYBACK);
    EXPECT_NE(&manager, nullptr);
}

/**
 * @tc.name  : Test Pause API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerPause_003
 * @tc.desc  : Test Pause interface.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerPause_003, TestSize.Level1)
{
    processConfig.streamType = STREAM_MOVIE;
    processConfig.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_ULTRASONIC;
    processConfig.rendererInfo.originalFlag = AUDIO_FLAG_PCM_OFFLOAD;
    auto dupStream = std::make_shared<ProRendererStreamImpl>(processConfig, true);
    auto server = std::make_shared<RendererInServer>(processConfig, stateListener);
    ASSERT_TRUE(server != nullptr);
    server->stream_ = CreateHpaeRendererStream();
    server->status_ = I_STATUS_STARTED;
    server->standByEnable_ = false;
    server->offloadEnable_ = true;
    server->stream_->SetOffloadMode(1, true);

    server->softLinkInfos_[0].isSoftLinkEnabled = false;
    server->softLinkInfos_[0].softLink = nullptr;
    server->softLinkInfos_[1].isSoftLinkEnabled = true;
    server->softLinkInfos_[1].softLink = nullptr;
    server->softLinkInfos_[2].isSoftLinkEnabled = false;
    server->softLinkInfos_[2].softLink =
        std::make_shared<HPAE::HpaeSoftLink>(1, 1, HPAE::SoftLinkMode::OFFLOADINNERCAP_AID);
    server->softLinkInfos_[3].isSoftLinkEnabled = true;
    server->softLinkInfos_[3].softLink =
        std::make_shared<HPAE::HpaeSoftLink>(1, 1, HPAE::SoftLinkMode::OFFLOADINNERCAP_AID);

    server->captureInfos_[0].isInnerCapEnabled = false;
    server->captureInfos_[0].dupStream = nullptr;
    server->captureInfos_[1].isInnerCapEnabled = true;
    server->captureInfos_[1].dupStream = nullptr;
    server->captureInfos_[2].isInnerCapEnabled = false;
    server->captureInfos_[2].dupStream = dupStream;
    server->captureInfos_[3].isInnerCapEnabled = true;
    server->captureInfos_[3].dupStream = dupStream;

    int32_t ret = server->Pause();
    EXPECT_EQ(SUCCESS, ret);

    server->offloadEnable_ = false;
    server->status_ = I_STATUS_STARTED;
    ret = server->Pause();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test StopInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerStopInner_001
 * @tc.desc  : Test StopInner interface.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerStopInner_001, TestSize.Level1)
{
    processConfig.streamType = STREAM_MOVIE;
    processConfig.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_ULTRASONIC;
    processConfig.rendererInfo.originalFlag = AUDIO_FLAG_PCM_OFFLOAD;
    auto dupStream = std::make_shared<ProRendererStreamImpl>(processConfig, true);
    auto server = std::make_shared<RendererInServer>(processConfig, stateListener);
    ASSERT_TRUE(server != nullptr);
    server->stream_ = CreateHpaeRendererStream();
    server->status_ = I_STATUS_STARTED;
    server->standByEnable_ = false;
    server->offloadEnable_ = true;
    server->stream_->SetOffloadMode(1, true);

    server->softLinkInfos_[0].isSoftLinkEnabled = false;
    server->softLinkInfos_[0].softLink = nullptr;
    server->softLinkInfos_[1].isSoftLinkEnabled = true;
    server->softLinkInfos_[1].softLink = nullptr;
    server->softLinkInfos_[2].isSoftLinkEnabled = false;
    server->softLinkInfos_[2].softLink =
        std::make_shared<HPAE::HpaeSoftLink>(1, 1, HPAE::SoftLinkMode::OFFLOADINNERCAP_AID);
    server->softLinkInfos_[3].isSoftLinkEnabled = true;
    server->softLinkInfos_[3].softLink =
        std::make_shared<HPAE::HpaeSoftLink>(1, 1, HPAE::SoftLinkMode::OFFLOADINNERCAP_AID);

    server->captureInfos_[0].isInnerCapEnabled = false;
    server->captureInfos_[0].dupStream = nullptr;
    server->captureInfos_[1].isInnerCapEnabled = true;
    server->captureInfos_[1].dupStream = nullptr;
    server->captureInfos_[2].isInnerCapEnabled = false;
    server->captureInfos_[2].dupStream = dupStream;
    server->captureInfos_[3].isInnerCapEnabled = true;
    server->captureInfos_[3].dupStream = dupStream;

    int32_t ret = server->StopInner();
    EXPECT_EQ(SUCCESS, ret);

    server->offloadEnable_ = false;
    ret = server->StopInner();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test EnableInnerCap API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerEnableInnerCap_001
 * @tc.desc  : Test EnableInnerCap interface.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerEnableInnerCap_001, TestSize.Level1)
{
    processConfig.streamType = STREAM_MOVIE;
    processConfig.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_ULTRASONIC;
    processConfig.rendererInfo.originalFlag = AUDIO_FLAG_PCM_OFFLOAD;
    auto dupStream = std::make_shared<ProRendererStreamImpl>(processConfig, true);
    auto server = std::make_shared<RendererInServer>(processConfig, stateListener);
    ASSERT_TRUE(server != nullptr);
    server->stream_ = CreateHpaeRendererStream();
    server->status_ = I_STATUS_STARTED;
    server->standByEnable_ = false;
    server->offloadEnable_ = true;
    server->stream_->SetOffloadMode(1, true);

    auto ret = server->EnableInnerCap(0);
    EXPECT_NE(SUCCESS, ret);

    server->offloadEnable_ = false;
    ret = server->EnableInnerCap(0);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test DisableInnerCapHandle API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerDisableInnerCapHandle_001
 * @tc.desc  : Test DisableInnerCapHandle interface.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerDisableInnerCapHandle_001, TestSize.Level1)
{
    processConfig.streamType = STREAM_MOVIE;
    processConfig.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_ULTRASONIC;
    processConfig.rendererInfo.originalFlag = AUDIO_FLAG_PCM_OFFLOAD;
    auto dupStream = std::make_shared<ProRendererStreamImpl>(processConfig, true);
    auto server = std::make_shared<RendererInServer>(processConfig, stateListener);
    ASSERT_TRUE(server != nullptr);
    server->stream_ = CreateHpaeRendererStream();
    server->status_ = I_STATUS_STARTED;
    server->standByEnable_ = false;
    server->offloadEnable_ = true;
    server->stream_->SetOffloadMode(1, true);

    server->captureInfos_[0].isInnerCapEnabled = false;
    server->captureInfos_[0].dupStream = nullptr;
    server->DisableInnerCapHandle(0);

    server->captureInfos_[1].isInnerCapEnabled = true;
    server->captureInfos_[1].dupStream = nullptr;
    server->DisableInnerCapHandle(1);
    auto ret = server->DisableInnerCapHandle(2);
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetOffloadMode API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetOffloadMode_002
 * @tc.desc  : Test SetOffloadMode interface.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerSetOffloadMode_002, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.streamType = STREAM_MOVIE;
    processConfig.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_ULTRASONIC;
    processConfig.rendererInfo.originalFlag = AUDIO_FLAG_PCM_OFFLOAD;
    auto dupStream = std::make_shared<ProRendererStreamImpl>(processConfig, true);
    auto server = std::make_shared<RendererInServer>(processConfig, stateListener);
    ASSERT_TRUE(server != nullptr);
    server->stream_ = CreateHpaeRendererStream();
    server->status_ = I_STATUS_STARTED;
    server->standByEnable_ = false;
    server->offloadEnable_ = true;
    server->Init();

    server->softLinkInfos_[0].isSoftLinkEnabled = false;
    server->softLinkInfos_[0].softLink = nullptr;
    server->softLinkInfos_[1].isSoftLinkEnabled = true;
    server->softLinkInfos_[1].softLink = nullptr;
    server->softLinkInfos_[2].isSoftLinkEnabled = false;
    server->softLinkInfos_[2].softLink = softLink;
    server->softLinkInfos_[3].isSoftLinkEnabled = true;
    server->softLinkInfos_[3].softLink = softLink;

    server->captureInfos_[0].isInnerCapEnabled = false;
    server->captureInfos_[0].dupStream = nullptr;
    server->captureInfos_[1].isInnerCapEnabled = true;
    server->captureInfos_[1].dupStream = nullptr;
    server->captureInfos_[2].isInnerCapEnabled = false;
    server->captureInfos_[2].dupStream = dupStream;
    server->captureInfos_[3].isInnerCapEnabled = true;
    server->captureInfos_[3].dupStream = dupStream;

    int32_t ret = server->SetOffloadMode(0, false);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test UnsetOffloadMode API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerUnsetOffloadMode_001
 * @tc.desc  : Test UnsetOffloadMode interface.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerUnsetOffloadMode_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.streamType = STREAM_MOVIE;
    processConfig.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_ULTRASONIC;
    processConfig.rendererInfo.originalFlag = AUDIO_FLAG_PCM_OFFLOAD;
    auto dupStream = std::make_shared<ProRendererStreamImpl>(processConfig, true);
    auto server = std::make_shared<RendererInServer>(processConfig, stateListener);
    ASSERT_TRUE(server != nullptr);
    server->standByEnable_ = false;
    server->offloadEnable_ = true;
    server->status_ = I_STATUS_IDLE;
    server->Init();
    std::shared_ptr<HPAE::IHpaeSoftLink> softLink =
        std::make_shared<HPAE::HpaeSoftLink>(1, 1, HPAE::SoftLinkMode::OFFLOADINNERCAP_AID);

    server->softLinkInfos_[0].isSoftLinkEnabled = false;
    server->softLinkInfos_[0].softLink = nullptr;
    server->softLinkInfos_[1].isSoftLinkEnabled = true;
    server->softLinkInfos_[1].softLink = nullptr;
    server->softLinkInfos_[2].isSoftLinkEnabled = false;
    server->softLinkInfos_[2].softLink = softLink;
    server->softLinkInfos_[3].isSoftLinkEnabled = true;
    server->softLinkInfos_[3].softLink = softLink;

    server->captureInfos_[0].isInnerCapEnabled = false;
    server->captureInfos_[0].dupStream = nullptr;
    server->captureInfos_[1].isInnerCapEnabled = true;
    server->captureInfos_[1].dupStream = nullptr;
    server->captureInfos_[2].isInnerCapEnabled = false;
    server->captureInfos_[2].dupStream = dupStream;
    server->captureInfos_[4].isInnerCapEnabled = true;
    server->captureInfos_[4].dupStream = dupStream;

    int32_t ret = server->UnsetOffloadMode();
    EXPECT_EQ(SUCCESS, ret);

    server->status_ = I_STATUS_STARTED;
    ret = server->UnsetOffloadMode();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetLoudnessGain API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetLoudnessGain_001
 * @tc.desc  : Test SetLoudnessGain interface.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerSetLoudnessGain_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.streamType = STREAM_MOVIE;
    processConfig.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_ULTRASONIC;
    processConfig.rendererInfo.originalFlag = AUDIO_FLAG_PCM_OFFLOAD;
    auto dupStream = std::make_shared<ProRendererStreamImpl>(processConfig, true);
    auto server = std::make_shared<RendererInServer>(processConfig, stateListener);
    ASSERT_TRUE(server != nullptr);
    server->stream_ = CreateHpaeRendererStream();
    server->status_ = I_STATUS_STARTED;
    server->standByEnable_ = false;
    server->offloadEnable_ = true;
    server->Init();

    server->softLinkInfos_[0].isSoftLinkEnabled = false;
    server->softLinkInfos_[0].softLink = nullptr;
    server->softLinkInfos_[1].isSoftLinkEnabled = true;
    server->softLinkInfos_[1].softLink = nullptr;
    server->softLinkInfos_[2].isSoftLinkEnabled = false;
    server->softLinkInfos_[2].softLink = softLink;
    server->softLinkInfos_[3].isSoftLinkEnabled = true;
    server->softLinkInfos_[3].softLink = softLink;

    int32_t ret = server->SetLoudnessGain(0.5);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test DisableAllInnerCap API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerDisableAllInnerCap_001
 * @tc.desc  : Test DisableAllInnerCap interface.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerDisableAllInnerCap_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.streamType = STREAM_MOVIE;
    processConfig.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_ULTRASONIC;
    processConfig.rendererInfo.originalFlag = AUDIO_FLAG_PCM_OFFLOAD;
    auto dupStream = std::make_shared<ProRendererStreamImpl>(processConfig, true);
    auto server = std::make_shared<RendererInServer>(processConfig, stateListener);
    ASSERT_TRUE(server != nullptr);
    server->stream_ = CreateHpaeRendererStream();
    server->status_ = I_STATUS_STARTED;
    server->standByEnable_ = false;
    server->offloadEnable_ = true;
    server->Init();

    server->softLinkInfos_[0].isSoftLinkEnabled = false;
    server->softLinkInfos_[0].softLink = nullptr;
    server->softLinkInfos_[1].isSoftLinkEnabled = true;
    server->softLinkInfos_[1].softLink = nullptr;
    server->softLinkInfos_[2].isSoftLinkEnabled = false;
    server->softLinkInfos_[2].softLink =
        std::make_shared<HPAE::HpaeSoftLink>(1, 1, HPAE::SoftLinkMode::OFFLOADINNERCAP_AID);
    server->softLinkInfos_[3].isSoftLinkEnabled = true;
    server->softLinkInfos_[3].softLink =
        std::make_shared<HPAE::HpaeSoftLink>(1, 1, HPAE::SoftLinkMode::OFFLOADINNERCAP_AID);

    server->captureInfos_[0].isInnerCapEnabled = false;
    server->captureInfos_[0].dupStream = nullptr;
    server->captureInfos_[1].isInnerCapEnabled = true;
    server->captureInfos_[1].dupStream = nullptr;
    server->captureInfos_[2].isInnerCapEnabled = false;
    server->captureInfos_[2].dupStream = dupStream;
    server->captureInfos_[3].isInnerCapEnabled = true;
    server->captureInfos_[3].dupStream = dupStream;

    int32_t ret = server->DisableAllInnerCap();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test InitSoftLinkVolume API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerInitSoftLinkVolume_001
 * @tc.desc  : Test InitSoftLinkVolume interface.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerInitSoftLinkVolume_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.streamType = STREAM_MOVIE;
    processConfig.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_ULTRASONIC;
    processConfig.rendererInfo.originalFlag = AUDIO_FLAG_PCM_OFFLOAD;
    auto server = std::make_shared<RendererInServer>(processConfig, stateListener);
    ASSERT_TRUE(server != nullptr);
    server->Init();

    server->softLinkInfos_[2].isSoftLinkEnabled = false;
    server->softLinkInfos_[2].softLink =
        std::make_shared<HPAE::HpaeSoftLink>(1, 1, HPAE::SoftLinkMode::OFFLOADINNERCAP_AID);

    int32_t ret = server->InitSoftLinkVolume(server->softLinkInfos_[2].softLink);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test InitSoftLink API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerInitSoftLink_001
 * @tc.desc  : Test InitSoftLinkVolume interface.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerInitSoftLink_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.streamType = STREAM_MOVIE;
    processConfig.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_ULTRASONIC;
    processConfig.rendererInfo.originalFlag = AUDIO_FLAG_PCM_OFFLOAD;
    auto server = std::make_shared<RendererInServer>(processConfig, stateListener);
    ASSERT_TRUE(server != nullptr);
    server->Init();
    server->InitSoftLink(30);
    server->status_ = I_STATUS_STARTED;
    server->InitSoftLink(31);

    server->softLinkInfos_[32].isSoftLinkEnabled = true;
    server->softLinkInfos_[32].softLink =
        std::make_shared<HPAE::HpaeSoftLink>(1, 1, HPAE::SoftLinkMode::OFFLOADINNERCAP_AID);

    int32_t ret = server->InitSoftLink(32);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test StartStreamByType API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerStartStreamByType_001
 * @tc.desc  : Test StartStreamByType interface.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerStartStreamByType_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.streamType = STREAM_MOVIE;
    processConfig.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_ULTRASONIC;
    processConfig.rendererInfo.originalFlag = AUDIO_FLAG_PCM_OFFLOAD;
    auto dupStream = std::make_shared<ProRendererStreamImpl>(processConfig, true);
    auto server = std::make_shared<RendererInServer>(processConfig, stateListener);
    ASSERT_TRUE(server != nullptr);
    server->stream_ = CreateHpaeRendererStream();
    server->status_ = I_STATUS_STARTED;
    server->standByEnable_ = false;
    server->offloadEnable_ = true;
    int32_t ret = server->Init();

    server->softLinkInfos_[0].isSoftLinkEnabled = false;
    server->softLinkInfos_[0].softLink = nullptr;
    server->softLinkInfos_[1].isSoftLinkEnabled = true;
    server->softLinkInfos_[1].softLink = nullptr;
    server->softLinkInfos_[2].isSoftLinkEnabled = false;
    server->softLinkInfos_[2].softLink = softLink;
    server->softLinkInfos_[3].isSoftLinkEnabled = true;
    server->softLinkInfos_[3].softLink = softLink;

    server->captureInfos_[0].isInnerCapEnabled = false;
    server->captureInfos_[0].dupStream = nullptr;
    server->captureInfos_[1].isInnerCapEnabled = true;
    server->captureInfos_[1].dupStream = nullptr;
    server->captureInfos_[2].isInnerCapEnabled = false;
    server->captureInfos_[2].dupStream = dupStream;
    server->captureInfos_[3].isInnerCapEnabled = true;
    server->captureInfos_[3].dupStream = dupStream;

    server->StartStreamByType();
    server->offloadEnable_ = false;
    server->StartStreamByType();
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test HandleOffloadStream API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerHandleOffloadStream_001
 * @tc.desc  : Test HandleOffloadStream interface.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerHandleOffloadStream_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.streamType = STREAM_MOVIE;
    processConfig.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_ULTRASONIC;
    processConfig.rendererInfo.originalFlag = AUDIO_FLAG_PCM_OFFLOAD;
    auto dupStream = std::make_shared<ProRendererStreamImpl>(processConfig, true);
    auto server = std::make_shared<RendererInServer>(processConfig, stateListener);
    ASSERT_TRUE(server != nullptr);
    server->stream_ = CreateHpaeRendererStream();
    server->standByEnable_ = false;
    server->offloadEnable_ = true;
    auto ret = server->Init();

    CaptureInfo captureInfo;
    captureInfo.isInnerCapEnabled = true;
    captureInfo.dupStream = dupStream;

    server->HandleOffloadStream(1, captureInfo);
    server->status_ = I_STATUS_STARTED;
    server->HandleOffloadStream(1, captureInfo);

    server->softLinkInfos_[1].isSoftLinkEnabled = false;
    server->softLinkInfos_[1].softLink = softLink;
    server->HandleOffloadStream(1, captureInfo);

    server->softLinkInfos_[1].isSoftLinkEnabled = true;
    server->HandleOffloadStream(1, captureInfo);

    server->status_ = I_STATUS_IDLE;
    server->HandleOffloadStream(1, captureInfo);

    server->softLinkInfos_[1].softLink = nullptr;
    server->HandleOffloadStream(1, captureInfo);

    processConfig.streamType = STREAM_RECORDING;
    auto server2 = std::make_shared<RendererInServer>(processConfig, stateListener);
    ASSERT_TRUE(server2 != nullptr);
    server2->stream_ = CreateHpaeRendererStream();
    server2->standByEnable_ = false;
    server2->offloadEnable_ = true;
    ret = server2->Init();
    server2->HandleOffloadStream(1, captureInfo);
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test DestroySoftLink API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerDestroySoftLink_001
 * @tc.desc  : Test DestorySoftLink interface.
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerDestroySoftLink_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.streamType = STREAM_MOVIE;
    processConfig.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_ULTRASONIC;
    processConfig.rendererInfo.originalFlag = AUDIO_FLAG_PCM_OFFLOAD;
    auto server = std::make_shared<RendererInServer>(processConfig, stateListener);
    ASSERT_TRUE(server != nullptr);
    server->status_ = I_STATUS_STARTED;
    server->Init();

    server->softLinkInfos_[0].isSoftLinkEnabled = false;
    server->softLinkInfos_[0].softLink = nullptr;
    server->softLinkInfos_[1].isSoftLinkEnabled = true;
    server->softLinkInfos_[1].softLink = nullptr;
    server->softLinkInfos_[3].isSoftLinkEnabled = true;
    server->softLinkInfos_[3].softLink =
        std::make_shared<HPAE::HpaeSoftLink>(1, 1, HPAE::SoftLinkMode::OFFLOADINNERCAP_AID);

    int32_t ret = server->DestroySoftLink(0);
    ret = server->DestroySoftLink(1);
    ret = server->DestroySoftLink(2);
    ret = server->DestroySoftLink(3);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test StartInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerStartInner_001
 * @tc.desc  : wzwzwz
 */
HWTEST_F(RendererInServerExtUnitTest, RendererInServerStartInner_001, TestSize.Level1)
{
    auto server = std::make_shared<RendererInServer>(processConfig, streamListener);
    ASSERT_TRUE(server != nullptr);

    int32_t ret = server->Init();
    server->standByEnable_ = true;
    server->OnStatusUpdate(OPERATION_PAUSED);
    server->playerDfx_ = nullptr;
    server->lastTarget_ = INJECT_TO_VOICE_COMMUNICATION_CAPTURE;

    ret = server->StartInner();
    EXPECT_NE(SUCCESS, ret);
}
} // namespace AudioStandard
} // namespace OHOS
