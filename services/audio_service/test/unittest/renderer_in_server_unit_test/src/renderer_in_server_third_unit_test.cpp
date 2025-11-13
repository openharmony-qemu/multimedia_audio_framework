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

#include "renderer_in_server_third_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
const uint32_t TEST_STREAMINDEX = 64;
const uint64_t TEST_LATENCY = 123456;
const uint64_t TEST_FRAMEPOS = 123456;
const uint64_t TEST_TIMESTAMP = 111111;
const float IN_VOLUME_RANGE = 0.5f;
const uint32_t TEST_BUFLENGTH = 10;

static std::shared_ptr<IStreamListener> stateListener;
static std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
static std::shared_ptr<RendererInServer> rendererInServer;
static std::shared_ptr<OHAudioBuffer> buffer;
static std::weak_ptr<IStreamListener> streamListener;

static AudioProcessConfig processConfig;
static BufferDesc bufferDesc;

static AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
    AudioChannelLayout::CH_LAYOUT_UNKNOWN);

void RendererInServerThirdUnitTest::SetUpTestCase(void) {}

void RendererInServerThirdUnitTest::TearDownTestCase(void)
{
    stateListener.reset();
    streamListenerHolder.reset();
    rendererInServer.reset();
    buffer.reset();
    streamListener.reset();
}

void RendererInServerThirdUnitTest::SetUp(void)
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

void RendererInServerThirdUnitTest::TearDown(void) {}

void InitAudioProcessConfig(AudioStreamInfo streamInfo, DeviceType deviceType = DEVICE_TYPE_WIRED_HEADSET,
    int32_t rendererFlags = AUDIO_FLAG_NORMAL, AudioStreamType streamType = STREAM_DEFAULT)
{
    processConfig = {};
    processConfig.streamInfo = streamInfo;
    processConfig.deviceType = deviceType;
    processConfig.rendererInfo = {};
    processConfig.capturerInfo = {};
    processConfig.rendererInfo.rendererFlags = rendererFlags;
    processConfig.streamType = streamType;
}

/**
 * @tc.name  : Test OnStatusUpdate API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerOnStatusUpdate_001
 * @tc.desc  : Test OnStatusUpdate when operation is OPERATION_SET_OFFLOAD_ENABLE.
 *             latestForWorkgroupInited_ is 0, status is I_STATUS_IDLE.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerOnStatusUpdate_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);
    rendererInServer->latestForWorkgroupInited_ = 0;
    rendererInServer->latestForWorkgroup_.status = I_STATUS_IDLE;
    rendererInServer->status_ = I_STATUS_IDLE;
    rendererInServer->OnStatusUpdate(OPERATION_SET_OFFLOAD_ENABLE);
    EXPECT_TRUE(rendererInServer->offloadEnable_);
}

/**
 * @tc.name  : Test OnStatusUpdate API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerOnStatusUpdate_002
 * @tc.desc  : Test OnStatusUpdate when operation is OPERATION_DRAINED.
 *             latestForWorkgroupInited_ is 0, status is I_STATUS_IDLE.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerOnStatusUpdate_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);
    rendererInServer->latestForWorkgroupInited_ = 0;
    rendererInServer->latestForWorkgroup_.status = I_STATUS_IDLE;
    rendererInServer->status_ = I_STATUS_IDLE;
    rendererInServer->OnStatusUpdate(OPERATION_DRAINED);
    EXPECT_FALSE(rendererInServer->offloadEnable_);
}

/**
 * @tc.name  : Test OnStatusUpdate API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerOnStatusUpdate_003
 * @tc.desc  : Test OnStatusUpdate when operation is OPERATION_SET_OFFLOAD_ENABLE.
 *             latestForWorkgroupInited_ is 0, status is I_STATUS_STARTING.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerOnStatusUpdate_003, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);
    rendererInServer->latestForWorkgroupInited_ = 0;
    rendererInServer->latestForWorkgroup_.status = I_STATUS_STARTING;
    rendererInServer->status_ = I_STATUS_IDLE;
    rendererInServer->OnStatusUpdate(OPERATION_SET_OFFLOAD_ENABLE);
    EXPECT_TRUE(rendererInServer->offloadEnable_);
}

/**
 * @tc.name  : Test OnStatusUpdate API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerOnStatusUpdate_004
 * @tc.desc  : Test OnStatusUpdate when operation is OPERATION_SET_OFFLOAD_ENABLE.
 *             and latestForWorkgroupInited_ is 1.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerOnStatusUpdate_004, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);
    rendererInServer->latestForWorkgroupInited_ = 1;
    rendererInServer->latestForWorkgroup_.status = I_STATUS_STARTING;
    rendererInServer->status_ = I_STATUS_IDLE;
    rendererInServer->OnStatusUpdate(OPERATION_SET_OFFLOAD_ENABLE);
    EXPECT_TRUE(rendererInServer->offloadEnable_);
}

/**
 * @tc.name  : Test OnStatusUpdate API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerOnStatusUpdate_005
 * @tc.desc  : Test OnStatusUpdate when operation is OPERATION_SET_OFFLOAD_ENABLE.
 *             and status_ is I_STATUS_IDLE.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerOnStatusUpdate_005, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);
    rendererInServer->latestForWorkgroup_.status = I_STATUS_IDLE;
    rendererInServer->status_ = I_STATUS_IDLE;
    rendererInServer->OnStatusUpdate(OPERATION_SET_OFFLOAD_ENABLE);
    EXPECT_TRUE(rendererInServer->offloadEnable_);
}

/**
 * @tc.name  : Test OnStatusUpdate API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerOnStatusUpdate_006
 * @tc.desc  : Test RendererInServerOnStatusUpdate.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerOnStatusUpdate_006, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);
    rendererInServer->OnCheckActiveMusicTime("Started");
    rendererInServer->latestForWorkgroup_.status = I_STATUS_IDLE;
    rendererInServer->status_ = I_STATUS_IDLE;
    rendererInServer->OnStatusUpdate(OPERATION_SET_OFFLOAD_ENABLE);
    EXPECT_TRUE(rendererInServer->offloadEnable_);
    rendererInServer->OnCheckActiveMusicTime("Started");
    EXPECT_TRUE(rendererInServer->offloadEnable_);
}

/**
 * @tc.name  : Test WriteMuteDataSysEvent API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerWriteMuteDataSysEvent_001
 * @tc.desc  : Test WriteMuteDataSysEvent when isInSilentState_ is true.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerWriteMuteDataSysEvent_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->isInSilentState_ = 1;
    uint8_t buffer[TEST_BUFLENGTH] = {0};
    size_t bufferSize = TEST_BUFLENGTH;
    bufferDesc.buffer = buffer;
    bufferDesc.bufLength = bufferSize;
    rendererInServer->WriteMuteDataSysEvent(bufferDesc);
    EXPECT_EQ(false, rendererInServer->isInSilentState_);
}

/**
 * @tc.name  : Test VolumeHandle API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerVolumeHandle_001
 * @tc.desc  : Test VolumeHandle when silentModeAndMixWithOthers_ is true.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerVolumeHandle_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);
    rendererInServer->lowPowerVolume_ = 0.0f;
    rendererInServer->audioServerBuffer_->basicBufferInfo_->duckFactor.store(0.0f);
    rendererInServer->silentModeAndMixWithOthers_ = true;

    rendererInServer->VolumeHandle(bufferDesc);
    EXPECT_EQ(0.0f, rendererInServer->oldAppliedVolume_);
}

/**
 * @tc.name  : Test VolumeHandle API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerVolumeHandle_002
 * @tc.desc  : Test VolumeHandle when muteFlag_ and silentModeAndMixWithOthers_ are true.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerVolumeHandle_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);
    rendererInServer->lowPowerVolume_ = 0.0f;
    rendererInServer->audioServerBuffer_->basicBufferInfo_->duckFactor.store(0.0f);
    rendererInServer->silentModeAndMixWithOthers_ = true;
    rendererInServer->muteFlag_ = true;

    rendererInServer->VolumeHandle(bufferDesc);
    EXPECT_EQ(0.0f, rendererInServer->oldAppliedVolume_);
}

/**
 * @tc.name  : Test GetAudioTime API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerGetAudioTime_001
 * @tc.desc  : Test GetAudioTime interface, set resetTime_ is true.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerGetAudioTime_001, TestSize.Level1)
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

    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetLowPowerVolume API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetLowPowerVolume_001
 * @tc.desc  : Test SetLowPowerVolume interface, Set isDualToneEnabled_ is true.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerSetLowPowerVolume_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->isDualToneEnabled_ = true;
    float volume = IN_VOLUME_RANGE;
    int32_t ret = rendererInServer->SetLowPowerVolume(volume);

    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test EnableDualTone API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerEnableDualTone_001
 * @tc.desc  : Test EnableDualTone interface, Set status_ is I_STATUS_STARTED.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerEnableDualTone_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->status_ = I_STATUS_STARTED;
    
    rendererInServer->Init();
    int32_t ret = rendererInServer->EnableDualTone("Speaker");

    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test EnableDualTone API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerEnableDualTone_002
 * @tc.desc  : Test EnableDualTone interface, Set dualToneStream_ is nullptr.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerEnableDualTone_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->status_ = I_STATUS_STARTED;
    rendererInServer->dualToneStream_ = nullptr;
    rendererInServer->Init();
    int32_t ret = rendererInServer->EnableDualTone("Speaker");

    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Dump API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerDump_001
 * @tc.desc  : Test Dump interface. Set dumpString is "", managerType_ is PLAYBACK,
 *             status_ is I_STATUS_FLUSHING_WHEN_STOPPED.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerDump_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    std::string dump="";
    rendererInServer->managerType_ = PLAYBACK;
    rendererInServer->status_ = I_STATUS_FLUSHING_WHEN_STOPPED;

    EXPECT_TRUE(rendererInServer->Dump(dump));
}

/**
 * @tc.name  : Test Dump API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerDump_002
 * @tc.desc  : Test Dump interface. Set dumpString is "", managerType_ is DUP_PLAYBACK,
 *             status_ is I_STATUS_FLUSHING_WHEN_STOPPED.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerDump_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    std::string dump="";
    rendererInServer->managerType_ = DUP_PLAYBACK;
    rendererInServer->status_ = I_STATUS_FLUSHING_WHEN_STOPPED;

    EXPECT_FALSE(rendererInServer->Dump(dump));
}

/**
 * @tc.name  : Test Dump API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerDump_003
 * @tc.desc  : Test Dump interface. Set dumpString is "", managerType_ is DUAL_PLAYBACK,
 *             status_ is I_STATUS_FLUSHING_WHEN_STOPPED.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerDump_003, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    std::string dump="";
    rendererInServer->managerType_ = DUAL_PLAYBACK;
    rendererInServer->status_ = I_STATUS_FLUSHING_WHEN_STOPPED;

    EXPECT_FALSE(rendererInServer->Dump(dump));
}

/**
 * @tc.name  : Test Dump API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerDump_004
 * @tc.desc  : Test Dump interface. Set dumpString is "", managerType_ is RECORDER,
 *             status_ is I_STATUS_FLUSHING_WHEN_STOPPED.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerDump_004, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    std::string dump="";
    rendererInServer->managerType_ = RECORDER;
    rendererInServer->status_ = I_STATUS_FLUSHING_WHEN_STOPPED;

    EXPECT_FALSE(rendererInServer->Dump(dump));
}

/**
 * @tc.name  : Test Dump API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerDump_005
 * @tc.desc  : Test Dump interface. Set dumpString is "", managerType_ is EAC3_PLAYBACK,
 *             status_ is I_STATUS_FLUSHING_WHEN_STOPPED.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerDump_005, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    std::string dump="";
    rendererInServer->managerType_ = EAC3_PLAYBACK;
    rendererInServer->status_ = I_STATUS_FLUSHING_WHEN_STOPPED;

    EXPECT_FALSE(rendererInServer->Dump(dump));
}

/**
 * @tc.name  : Test OnWriteData API
 * @tc.type  : FUNC
 * @tc.number: GetStandbyStatus_001
 * @tc.desc  : Test OnWriteData API when standByEnable_ is false.
 */
HWTEST_F(RendererInServerThirdUnitTest, GetStandbyStatus_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    RendererInServer rendererInServer(processConfig, stateListener);
    rendererInServer.standByEnable_ = false;
    bool isStandby = true;
    int64_t enterStandbyTime = 0;
    int ret = rendererInServer.GetStandbyStatus(isStandby, enterStandbyTime);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test GetLatency API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerGetLatency_001
 * @tc.desc  : Test GetLatency interface, set managerType_ is DIRECT_PLAYBACK.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerGetLatency_001, TestSize.Level1)
{
    processConfig.deviceType = DEVICE_TYPE_INVALID;
    processConfig.rendererInfo.rendererFlags = AUDIO_FLAG_MMAP;
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->managerType_ = DIRECT_PLAYBACK;
    rendererInServer->Init();
    uint64_t latency = TEST_LATENCY;
    int32_t ret = rendererInServer->GetLatency(latency);

    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test GetLatency API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerGetLatency_002
 * @tc.desc  : Test GetLatency interface, set managerType_ is VOIP_PLAYBACK.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerGetLatency_002, TestSize.Level1)
{
    processConfig.deviceType = DEVICE_TYPE_INVALID;
    processConfig.rendererInfo.rendererFlags = AUDIO_FLAG_MMAP;
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->managerType_ = VOIP_PLAYBACK;
    rendererInServer->Init();
    uint64_t latency = TEST_LATENCY;
    int32_t ret = rendererInServer->GetLatency(latency);

    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test CreateDupBufferInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerCreateDupBufferInner_001
 * @tc.desc  : Test CreateDupBufferInner interface, set innerCapId is 0.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerCreateDupBufferInner_001, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, MONO,
        AudioChannelLayout::CH_LAYOUT_MONO);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_USAGE_NORMAL);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t innerCapId = 0;
    int32_t ret = rendererInServer->CreateDupBufferInner(innerCapId);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test CreateDupBufferInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerCreateDupBufferInner_002
 * @tc.desc  : Test CreateDupBufferInner interface, set innerCapId is 1.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerCreateDupBufferInner_002, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, MONO,
        AudioChannelLayout::CH_LAYOUT_MONO);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_USAGE_NORMAL);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t innerCapId = 1;
    int32_t ret = rendererInServer->CreateDupBufferInner(innerCapId);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test WriteDupBufferInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerWriteDupBufferInner_001
 * @tc.desc  : Test WriteDupBufferInner interface, set innerCapId is 0.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerWriteDupBufferInner_001, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, MONO,
        AudioChannelLayout::CH_LAYOUT_MONO);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_USAGE_NORMAL);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t innerCapId = 0;
    uint8_t bufferTest = 0;
    bufferDesc.buffer = &bufferTest;
    bufferDesc.metaBuffer = &bufferTest;
    int32_t ret = rendererInServer->WriteDupBufferInner(bufferDesc, innerCapId);
    EXPECT_EQ(ERROR, ret);
}

/**
 * @tc.name  : Test WriteDupBufferInner API

 * @tc.type  : FUNC
 * @tc.number: RendererInServerWriteDupBufferInner_002
 * @tc.desc  : Test WriteDupBufferInner interface, set innerCapId is 1.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerWriteDupBufferInner_002, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, MONO,
        AudioChannelLayout::CH_LAYOUT_MONO);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_USAGE_NORMAL);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t innerCapId = 1;
    uint8_t bufferTest = 0;
    bufferDesc.buffer = &bufferTest;
    bufferDesc.metaBuffer = &bufferTest;
    int32_t ret = rendererInServer->WriteDupBufferInner(bufferDesc, innerCapId);
    EXPECT_EQ(ERROR, ret);
}

/**
 * @tc.name  : Test SetSpeed API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetSpeed_001
 * @tc.desc  : Test SetSpeed interface, set speed is 0.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerSetSpeed_001, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, MONO,
        AudioChannelLayout::CH_LAYOUT_MONO);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_USAGE_NORMAL);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    float speed = 0;
    rendererInServer->SetSpeed(speed);
    EXPECT_EQ(nullptr, rendererInServer->stream_);
}

/**
 * @tc.name  : Test SetSpeed API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetSpeed_002
 * @tc.desc  : Test SetSpeed interface, set speed is 1.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerSetSpeed_002, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, MONO,
        AudioChannelLayout::CH_LAYOUT_MONO);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_USAGE_NORMAL);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    float speed = 1;
    rendererInServer->SetSpeed(speed);
    EXPECT_EQ(nullptr, rendererInServer->stream_);
}

/**
 * @tc.name  : Test StopSession API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerStopSession_001
 * @tc.desc  : Test StopSession interface, set speed is 1.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerStopSession_001, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, MONO,
        AudioChannelLayout::CH_LAYOUT_MONO);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_USAGE_NORMAL);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->StopSession();
    EXPECT_EQ(nullptr, rendererInServer->audioServerBuffer_);
}

/**
 * @tc.name  : Test GetDupRingBuffer API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerGetDupRingBuffer_001
 * @tc.desc  : Test GetDupRingBuffer interface, set dupRingBuffer_ is nullptr.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerGetDupRingBuffer_001, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, MONO,
        AudioChannelLayout::CH_LAYOUT_MONO);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_USAGE_NORMAL);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->dupRingBuffer_ = nullptr;
    EXPECT_EQ(rendererInServer->GetDupRingBuffer(), nullptr);
}

/**
 * @tc.name  : Test SetSourceDuration API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetSourceDuration_001
 * @tc.desc  : Test SetSourceDuration interface, set duration is 1.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerSetSourceDuration_001, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, MONO,
        AudioChannelLayout::CH_LAYOUT_MONO);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_USAGE_NORMAL);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int64_t duration = 1;
    int32_t ret = rendererInServer->SetSourceDuration(duration);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetSourceDuration API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetSourceDuration_002
 * @tc.desc  : Test SetSourceDuration interface, set duration is 0.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerSetSourceDuration_002, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, MONO,
        AudioChannelLayout::CH_LAYOUT_MONO);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_USAGE_NORMAL);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int64_t duration = 0;
    int32_t ret = rendererInServer->SetSourceDuration(duration);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test GetActualStreamManagerType API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerGetActualStreamManagerType_001
 * @tc.desc  : Test GetActualStreamManagerType interface, set managerType_ is DIRECT_PLAYBACK.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerGetActualStreamManagerType_001, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, MONO,
        AudioChannelLayout::CH_LAYOUT_MONO);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_USAGE_NORMAL);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->GetActualStreamManagerType();

    EXPECT_EQ(rendererInServer->managerType_, PLAYBACK);
}

/**
 * @tc.name  : Test SetAudioHapticsSyncId API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetAudioHapticsSyncId_001
 * @tc.desc  : Test SetAudioHapticsSyncId interface, set managerType_ is DIRECT_PLAYBACK.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerSetAudioHapticsSyncId_001, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, MONO,
        AudioChannelLayout::CH_LAYOUT_MONO);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_USAGE_NORMAL);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t audioHapticsSyncId = 0;
    int32_t ret = rendererInServer->SetAudioHapticsSyncId(audioHapticsSyncId);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test InitDupBuffer API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerInitDupBuffer_001
 * @tc.desc  : Test InitDupBuffer interface, set innerCapId is 0.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerInitDupBuffer_001, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, MONO,
        AudioChannelLayout::CH_LAYOUT_MONO);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_USAGE_NORMAL);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t innerCapId = 0;
    rendererInServer->InitDupBuffer(innerCapId);
    EXPECT_EQ(rendererInServer->offloadEnable_, false);
}

/**
 * @tc.name  : Test InitDupBuffer API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerInitDupBuffer_002
 * @tc.desc  : Test InitDupBuffer interface, set innerCapId is 1.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerInitDupBuffer_002, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, MONO,
        AudioChannelLayout::CH_LAYOUT_MONO);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_USAGE_NORMAL);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t innerCapId = 1;
    rendererInServer->InitDupBuffer(innerCapId);
    EXPECT_EQ(rendererInServer->offloadEnable_, false);
}

/**
 * @tc.name  : Test CopyDataToInputBuffer API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerCopyDataToInputBuffer_001
 * @tc.desc  : Test CopyDataToInputBuffer interface, set buffer is nullptr.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerCopyDataToInputBuffer_001, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, MONO,
        AudioChannelLayout::CH_LAYOUT_MONO);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_USAGE_NORMAL);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int8_t inputData[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    size_t requestDataLen = 10;
    RingBufferWrapper ringBufferDesc;
    ringBufferDesc.basicBufferDescs[0].buffer = nullptr;
    ringBufferDesc.basicBufferDescs[0].bufLength = 10;
    ringBufferDesc.dataLength = 10;

    rendererInServer->CopyDataToInputBuffer(inputData, requestDataLen, ringBufferDesc);
    EXPECT_EQ(ringBufferDesc.basicBufferDescs[0].buffer, nullptr);
}

/**
 * @tc.name  : Test ProcessFadeOutIfNeeded API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerProcessFadeOutIfNeeded_001
 * @tc.desc  : Test ProcessFadeOutIfNeeded interface, set currentReadFrame + requestDataInFrameis != currentWriteFrame.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerProcessFadeOutIfNeeded_001, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, MONO,
        AudioChannelLayout::CH_LAYOUT_MONO);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_USAGE_NORMAL);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    RingBufferWrapper ringBufferDesc;
    uint64_t currentReadFrame = 10;
    uint64_t currentWriteFrame = 30;
    size_t requestDataInFrame = 10;

    rendererInServer->ProcessFadeOutIfNeeded(ringBufferDesc, currentReadFrame, currentWriteFrame, requestDataInFrame);
    EXPECT_EQ(ringBufferDesc.basicBufferDescs[0].buffer, nullptr);
}

/**
 * @tc.name  : Test ProcessFadeOutIfNeeded API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerProcessFadeOutIfNeeded_002
 * @tc.desc  : Test ProcessFadeOutIfNeeded interface, set currentReadFrame + requestDataInFrameis == currentWriteFrame.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerProcessFadeOutIfNeeded_002, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, MONO,
        AudioChannelLayout::CH_LAYOUT_MONO);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_USAGE_NORMAL);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    RingBufferWrapper ringBufferDesc;
    uint64_t currentReadFrame = 10;
    uint64_t currentWriteFrame = 20;
    size_t requestDataInFrame = 10;

    rendererInServer->ProcessFadeOutIfNeeded(ringBufferDesc, currentReadFrame, currentWriteFrame, requestDataInFrame);
    EXPECT_EQ(ringBufferDesc.basicBufferDescs[0].buffer, nullptr);
}

/**
 * @tc.name  : Test GetAvailableSize API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerGetAvailableSize_001
 * @tc.desc  : Test GetAvailableSize when curWriteFrame > curReadFrame.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerGetAvailableSize_001, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, MONO,
        AudioChannelLayout::CH_LAYOUT_MONO);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_USAGE_NORMAL);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    rendererInServer->audioServerBuffer_->basicBufferInfo_->curWriteFrame.store(8);
    rendererInServer->audioServerBuffer_->basicBufferInfo_->curReadFrame.store(4);
    rendererInServer->audioServerBuffer_->basicBufferInfo_->totalSizeInFrame = 16;
    rendererInServer->spanSizeInFrame_ = 4;
    rendererInServer->byteSizePerFrame_ = 1;
    size_t length = 0;
    ret = rendererInServer->GetAvailableSize(length);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test GetAvailableSize API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerGetAvailableSize_002
 * @tc.desc  : Test GetAvailableSize when when curWriteFrame < curReadFrame.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerGetAvailableSize_002, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, MONO,
        AudioChannelLayout::CH_LAYOUT_MONO);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_USAGE_NORMAL);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    rendererInServer->audioServerBuffer_->basicBufferInfo_->curWriteFrame.store(8);
    rendererInServer->audioServerBuffer_->basicBufferInfo_->curReadFrame.store(12);
    rendererInServer->audioServerBuffer_->basicBufferInfo_->totalSizeInFrame = 16;
    rendererInServer->spanSizeInFrame_ = 4;
    rendererInServer->byteSizePerFrame_ = 1;
    size_t length = 0;
    ret = rendererInServer->GetAvailableSize(length);
    EXPECT_EQ(ERROR, ret);
}

/**
 * @tc.name  : Test WriteData API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerWriteData_001
 * @tc.desc  : Test WriteData when currentReadFrame >= currentWriteFram.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerWriteData_001, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, MONO,
        AudioChannelLayout::CH_LAYOUT_MONO);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_USAGE_NORMAL);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    rendererInServer->audioServerBuffer_->basicBufferInfo_->curWriteFrame.store(8);
    rendererInServer->audioServerBuffer_->basicBufferInfo_->curReadFrame.store(4);
    rendererInServer->audioServerBuffer_->basicBufferInfo_->totalSizeInFrame = 16;
    rendererInServer->spanSizeInFrame_ = 4;
    rendererInServer->silentModeAndMixWithOthers_ = true;
    
    RingBufferWrapper ringBufferDesc;
    ringBufferDesc.dataLength = 0;
    ret = rendererInServer->WriteData();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test OnWriteData API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerOnWriteData_001
 * @tc.desc  : Test OnWriteData when currentReadFrame + requestDataLen > currentWriteFram.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerOnWriteData_001, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, MONO,
        AudioChannelLayout::CH_LAYOUT_MONO);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_USAGE_NORMAL);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    rendererInServer->audioServerBuffer_->basicBufferInfo_->curWriteFrame.store(8);
    rendererInServer->audioServerBuffer_->basicBufferInfo_->curReadFrame.store(4);
    rendererInServer->audioServerBuffer_->basicBufferInfo_->totalSizeInFrame = 16;
    rendererInServer->spanSizeInFrame_ = 4;
    
    int8_t inputData[10] = {0};
    size_t requestDataLen = 10;
    ret = rendererInServer->OnWriteData(inputData, requestDataLen);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);
}

/**
 * @tc.name  : Test OnWriteData API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerOnWriteData_002
 * @tc.desc  : Test OnWriteData when currentReadFrame + requestDataLen > currentWriteFram.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerOnWriteData_002, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, MONO,
        AudioChannelLayout::CH_LAYOUT_MONO);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_USAGE_NORMAL);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    rendererInServer->audioServerBuffer_->basicBufferInfo_->curWriteFrame.store(8);
    rendererInServer->audioServerBuffer_->basicBufferInfo_->curReadFrame.store(12);
    rendererInServer->audioServerBuffer_->basicBufferInfo_->totalSizeInFrame = 16;
    rendererInServer->spanSizeInFrame_ = 4;
    
    int8_t inputData[10] = {0};
    size_t requestDataLen = 0;
    ret = rendererInServer->OnWriteData(inputData, requestDataLen);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);
}

/**
 * @tc.name  : Test OnWriteData API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerOnWriteData_003
 * @tc.desc  : Test OnWriteData when currentReadFrame + requestDataLen > currentWriteFram.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerOnWriteData_003, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, MONO,
        AudioChannelLayout::CH_LAYOUT_MONO);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_USAGE_NORMAL);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    rendererInServer->audioServerBuffer_->basicBufferInfo_->curWriteFrame.store(8);
    rendererInServer->audioServerBuffer_->basicBufferInfo_->curReadFrame.store(4);
    rendererInServer->audioServerBuffer_->basicBufferInfo_->totalSizeInFrame = 16;
    rendererInServer->spanSizeInFrame_ = 4;
    rendererInServer->offloadEnable_ = 1;
    
    int8_t inputData[10] = {0};
    size_t requestDataLen = 10;
    ret = rendererInServer->OnWriteData(inputData, requestDataLen);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);
}

/**
 * @tc.name  : Test OnWriteData API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerOnWriteData_004
 * @tc.desc  : Test OnWriteData when currentReadFrame + requestDataLen < currentWriteFram.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerOnWriteData_004, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, MONO,
        AudioChannelLayout::CH_LAYOUT_MONO);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_USAGE_NORMAL);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    rendererInServer->audioServerBuffer_->basicBufferInfo_->curWriteFrame.store(12);
    rendererInServer->audioServerBuffer_->basicBufferInfo_->curReadFrame.store(4);
    rendererInServer->audioServerBuffer_->basicBufferInfo_->totalSizeInFrame = 16;
    rendererInServer->spanSizeInFrame_ = 4;
    
    int8_t inputData[10] = {0};
    size_t requestDataLen = 10;
    ret = rendererInServer->OnWriteData(inputData, requestDataLen);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test InnerCaptureEnqueueBuffer API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerInnerCaptureEnqueueBuffer_001
 * @tc.desc  : Test InnerCaptureEnqueueBuffer when when renderEmptyCountForInnerCap_ is 1.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerInnerCaptureEnqueueBuffer_001, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, MONO,
        AudioChannelLayout::CH_LAYOUT_MONO);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_USAGE_NORMAL);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    BufferDesc bufferDesc;
    CaptureInfo captureInfo;
    int32_t innerCapId = 1;
    
    rendererInServer->renderEmptyCountForInnerCapToInnerCapIdMap_[innerCapId] = 1;
    rendererInServer->spanSizeInByte_ = 10;
    rendererInServer->InnerCaptureEnqueueBuffer(bufferDesc, captureInfo, innerCapId);
    EXPECT_EQ(rendererInServer->renderEmptyCountForInnerCapToInnerCapIdMap_[innerCapId], 0);
}

/**
 * @tc.name  : Test InnerCaptureEnqueueBuffer API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerInnerCaptureEnqueueBuffer_002
 * @tc.desc  : Test InnerCaptureEnqueueBuffer when renderEmptyCountForInnerCap_ is 0.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerInnerCaptureEnqueueBuffer_002, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, MONO,
        AudioChannelLayout::CH_LAYOUT_MONO);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_USAGE_NORMAL);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    BufferDesc bufferDesc;
    CaptureInfo captureInfo;
    int32_t innerCapId = 1;
    
    rendererInServer->renderEmptyCountForInnerCapToInnerCapIdMap_[innerCapId] = 0;
    rendererInServer->spanSizeInByte_ = 10;
    rendererInServer->InnerCaptureEnqueueBuffer(bufferDesc, captureInfo, innerCapId);
    EXPECT_EQ(rendererInServer->renderEmptyCountForInnerCapToInnerCapIdMap_[innerCapId], 0);
}

/**
 * @tc.name  : Test InnerCaptureOtherStream API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerInnerCaptureOtherStream_001
 * @tc.desc  : Test InnerCaptureOtherStream when dupStream is nullptr.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerInnerCaptureOtherStream_001, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, MONO,
        AudioChannelLayout::CH_LAYOUT_MONO);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_USAGE_NORMAL);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    BufferDesc bufferDesc;
    CaptureInfo captureInfo;
    captureInfo.isInnerCapEnabled = true;
    captureInfo.dupStream = nullptr;
    int32_t innerCapId = 1;

    rendererInServer->InnerCaptureOtherStream(bufferDesc, captureInfo, innerCapId);
    EXPECT_EQ(rendererInServer->renderEmptyCountForInnerCapToInnerCapIdMap_[innerCapId], 0);
}

/**
 * @tc.name  : Test InnerCaptureOtherStream API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerInnerCaptureOtherStream_002
 * @tc.desc  : Test InnerCaptureOtherStream when isInnerCapEnabled is false.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerInnerCaptureOtherStream_002, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, MONO,
        AudioChannelLayout::CH_LAYOUT_MONO);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_USAGE_NORMAL);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    BufferDesc bufferDesc;
    CaptureInfo captureInfo;
    captureInfo.isInnerCapEnabled = false;
    int32_t innerCapId = 1;

    rendererInServer->InnerCaptureOtherStream(bufferDesc, captureInfo, innerCapId);
    EXPECT_EQ(rendererInServer->renderEmptyCountForInnerCapToInnerCapIdMap_[innerCapId], 0);
}

/**
 * @tc.name  : Test OnWriteData API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerOnWriteData_005
 * @tc.desc  : Test OnWriteData when length is 0.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerOnWriteData_005, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, MONO,
        AudioChannelLayout::CH_LAYOUT_MONO);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_USAGE_NORMAL);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    size_t length = 0;
    int32_t ret = rendererInServer->OnWriteData(length);
    EXPECT_EQ(ERR_WRITE_FAILED, ret);
}

/**
 * @tc.name  : Test OnWriteData API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerOnWriteData_006
 * @tc.desc  : Test OnWriteData when length is 10.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerOnWriteData_006, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, MONO,
        AudioChannelLayout::CH_LAYOUT_MONO);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_USAGE_NORMAL);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->spanSizeInFrame_ = 0;
    size_t length = 10;
    int32_t ret = rendererInServer->OnWriteData(length);
    EXPECT_EQ(ERR_WRITE_FAILED, ret);
}

/**
 * @tc.name  : Test GetSessionId API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerGetSessionId_001
 * @tc.desc  : Test GetSessionId when sessionId > INT32_MAX.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerGetSessionId_001, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    uint32_t sessionId = 2147483648;
    int32_t ret = rendererInServer->GetSessionId(sessionId);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);
}

/**
 * @tc.name  : Test ResolveBufferBaseAndGetServerSpanSize API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerResolveBufferBaseAndGetServerSpanSize_001
 * @tc.desc  : Test ResolveBufferBaseAndGetServerSpanSize when length is 10.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerResolveBufferBaseAndGetServerSpanSize_001, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    std::shared_ptr<OHAudioBufferBase> buffer;
    uint32_t sizeInFrame = 0;
    uint64_t engineTotal = 0;
    int32_t ret = rendererInServer->ResolveBufferBaseAndGetServerSpanSize(buffer, sizeInFrame, engineTotal);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Start API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerStart_001
 * @tc.desc  : Test Start when standByEnable_ is true and managerType_ is DIRECT_PLAYBACK.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerStart_001, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_FLAG_VOIP_DIRECT);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    rendererInServer->standByEnable_ = true;
    rendererInServer->OnStatusUpdate(OPERATION_PAUSED);
    rendererInServer->playerDfx_ = 0;
    rendererInServer->managerType_ = DIRECT_PLAYBACK;
    
    ret = rendererInServer->Start();
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test Start API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerStart_002
 * @tc.desc  : Test Start when standByEnable_ is false and managerType_ is DIRECT_PLAYBACK.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerStart_002, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_FLAG_VOIP_DIRECT);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    rendererInServer->standByEnable_ = false;
    rendererInServer->OnStatusUpdate(OPERATION_PAUSED);
    rendererInServer->playerDfx_ = 0;
    rendererInServer->managerType_ = DIRECT_PLAYBACK;
    
    ret = rendererInServer->Start();
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test Start API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerStart_003
 * @tc.desc  : Test Start when standByEnable_ is true and managerType_ is VOIP_PLAYBACK.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerStart_003, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_FLAG_VOIP_DIRECT);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    rendererInServer->standByEnable_ = true;
    rendererInServer->OnStatusUpdate(OPERATION_PAUSED);
    rendererInServer->playerDfx_ = 0;
    rendererInServer->managerType_ = VOIP_PLAYBACK;
    
    ret = rendererInServer->Start();
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test Start API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerStart_004
 * @tc.desc  : Test Start when standByEnable_ is true and managerType_ is EAC3_PLAYBACK.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerStart_004, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_FLAG_VOIP_DIRECT);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    rendererInServer->standByEnable_ = true;
    rendererInServer->OnStatusUpdate(OPERATION_PAUSED);
    rendererInServer->playerDfx_ = 0;
    rendererInServer->managerType_ = EAC3_PLAYBACK;
    
    ret = rendererInServer->Start();
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test Start API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerStart_005
 * @tc.desc  : Test Start when standByEnable_ is true and managerType_ is RECORDER.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerStart_005, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_FLAG_VOIP_DIRECT);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    rendererInServer->standByEnable_ = true;
    rendererInServer->OnStatusUpdate(OPERATION_PAUSED);
    rendererInServer->playerDfx_ = 0;
    rendererInServer->managerType_ = RECORDER;
    
    ret = rendererInServer->Start();
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test Start API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerStart_006
 * @tc.desc  : Test Start when standByEnable_ is false and managerType_ is VOIP_PLAYBACK.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerStart_006, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_FLAG_VOIP_DIRECT);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    rendererInServer->standByEnable_ = false;
    rendererInServer->OnStatusUpdate(OPERATION_PAUSED);
    rendererInServer->playerDfx_ = 0;
    rendererInServer->managerType_ = VOIP_PLAYBACK;
    
    ret = rendererInServer->Start();
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test Start API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerStart_007
 * @tc.desc  : Test Start when standByEnable_ is false and managerType_ is EAC3_PLAYBACK.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerStart_007, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_FLAG_VOIP_DIRECT);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    rendererInServer->standByEnable_ = false;
    rendererInServer->OnStatusUpdate(OPERATION_PAUSED);
    rendererInServer->playerDfx_ = 0;
    rendererInServer->managerType_ = EAC3_PLAYBACK;
    
    ret = rendererInServer->Start();
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test Start API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerStart_008
 * @tc.desc  : Test Start when standByEnable_ is false and managerType_ is RECORDER.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerStart_008, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_FLAG_VOIP_DIRECT);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    rendererInServer->standByEnable_ = false;
    rendererInServer->OnStatusUpdate(OPERATION_PAUSED);
    rendererInServer->playerDfx_ = 0;
    rendererInServer->managerType_ = RECORDER;
    
    ret = rendererInServer->Start();
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test dualToneStreamInStart API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerdualToneStreamInStart_001
 * @tc.desc  : Test dualToneStreamInStart when isDualToneEnabled_ is true.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerdualToneStreamInStart_001, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_FLAG_VOIP_DIRECT);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->Init();
    rendererInServer->OnStatusUpdate(OPERATION_PAUSED);
    rendererInServer->isDualToneEnabled_ = true;
    
    rendererInServer->dualToneStreamInStart();
    EXPECT_EQ(rendererInServer->dualToneStream_, nullptr);
}

/**
 * @tc.name  : Test dualToneStreamInStart API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerdualToneStreamInStart_002
 * @tc.desc  : Test dualToneStreamInStart when isDualToneEnabled_ is true.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerdualToneStreamInStart_002, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_FLAG_VOIP_DIRECT);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->Init();
    rendererInServer->OnStatusUpdate(OPERATION_PAUSED);
    rendererInServer->isDualToneEnabled_ = false;
    
    rendererInServer->dualToneStreamInStart();
    EXPECT_EQ(rendererInServer->dualToneStream_, nullptr);
}

/**
 * @tc.name  : Test dualToneStreamInStart API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerdualToneStreamInStart_003
 * @tc.desc  : Test dualToneStreamInStart when isDualToneEnabled_ is true.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerdualToneStreamInStart_003, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_FLAG_VOIP_DIRECT);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->Init();
    rendererInServer->OnStatusUpdate(OPERATION_PAUSED);
    rendererInServer->isDualToneEnabled_ = false;
    rendererInServer->dualToneStream_ = nullptr;
    
    rendererInServer->dualToneStreamInStart();
    EXPECT_EQ(rendererInServer->dualToneStream_, nullptr);
}

/**
 * @tc.name  : Test RecordStandbyTime API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerRecordStandbyTime_001
 * @tc.desc  : Test RecordStandbyTime when isStandby is false and isStandbyStart is true.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerRecordStandbyTime_001, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_FLAG_VOIP_DIRECT);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    bool isStandby = false;
    bool isStandbyStart = true;
    
    rendererInServer->RecordStandbyTime(isStandby, isStandbyStart);
    EXPECT_EQ(rendererInServer->managerType_, PLAYBACK);
}

/**
 * @tc.name  : Test RecordStandbyTime API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerRecordStandbyTime_002
 * @tc.desc  : Test RecordStandbyTime when isStandby and isStandbyStart are true.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerRecordStandbyTime_002, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_FLAG_VOIP_DIRECT);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    bool isStandby = true;
    bool isStandbyStart = true;
    
    rendererInServer->RecordStandbyTime(isStandby, isStandbyStart);
    EXPECT_EQ(rendererInServer->managerType_, PLAYBACK);
}

/**
 * @tc.name  : Test Pause API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerPause_001
 * @tc.desc  : Test Pause when managerType_ is DIRECT_PLAYBACK.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerPause_001, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    rendererInServer->OnStatusUpdate(OPERATION_STARTED);
    rendererInServer->standByEnable_ = true;
    rendererInServer->playerDfx_ = 0;
    rendererInServer->managerType_ = DIRECT_PLAYBACK;
    
    ret = rendererInServer->Pause();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Pause API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerPause_002
 * @tc.desc  : Test Pause when managerType_ is VOIP_PLAYBACK.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerPause_002, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    rendererInServer->OnStatusUpdate(OPERATION_STARTED);
    rendererInServer->standByEnable_ = true;
    rendererInServer->playerDfx_ = 0;
    rendererInServer->managerType_ = VOIP_PLAYBACK;
    
    ret = rendererInServer->Pause();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Pause API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerPause_003
 * @tc.desc  : Test Pause when managerType_ is EAC3_PLAYBACK.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerPause_003, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    rendererInServer->OnStatusUpdate(OPERATION_STARTED);
    rendererInServer->standByEnable_ = true;
    rendererInServer->playerDfx_ = 0;
    rendererInServer->managerType_ = EAC3_PLAYBACK;
    
    ret = rendererInServer->Pause();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Pause API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerPause_004
 * @tc.desc  : Test Pause when managerType_ is RECORDER.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerPause_004, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    rendererInServer->OnStatusUpdate(OPERATION_STARTED);
    rendererInServer->standByEnable_ = true;
    rendererInServer->playerDfx_ = 0;
    rendererInServer->managerType_ = RECORDER;
    
    ret = rendererInServer->Pause();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Stop API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerStop_001
 * @tc.desc  : Test Stop when playerDfx_ is 0.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerStop_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->status_ = I_STATUS_PAUSED;
    int32_t ret = rendererInServer->Init();
    rendererInServer->standByEnable_ = true;
    rendererInServer->InitDupStream(1);
    rendererInServer->EnableDualTone("Speaker");
    rendererInServer->playerDfx_ = 0;

    ret = rendererInServer->Stop();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Stop API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerStop_002
 * @tc.desc  : Test Stop when managerType_ is DIRECT_PLAYBACK.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerStop_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->status_ = I_STATUS_PAUSED;
    int32_t ret = rendererInServer->Init();
    rendererInServer->standByEnable_ = true;
    rendererInServer->InitDupStream(1);
    rendererInServer->EnableDualTone("Speaker");
    rendererInServer->managerType_ = DIRECT_PLAYBACK;

    ret = rendererInServer->Stop();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Stop API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerStop_003
 * @tc.desc  : Test Stop when managerType_ is EAC3_PLAYBACK.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerStop_003, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->status_ = I_STATUS_PAUSED;
    int32_t ret = rendererInServer->Init();
    rendererInServer->standByEnable_ = true;
    rendererInServer->InitDupStream(1);
    rendererInServer->EnableDualTone("Speaker");
    rendererInServer->managerType_ = EAC3_PLAYBACK;

    ret = rendererInServer->Stop();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Stop API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerStop_004
 * @tc.desc  : Test Stop when managerType_ is RECORDER.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerStop_004, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->status_ = I_STATUS_PAUSED;
    int32_t ret = rendererInServer->Init();
    rendererInServer->standByEnable_ = true;
    rendererInServer->InitDupStream(1);
    rendererInServer->EnableDualTone("Speaker");
    rendererInServer->managerType_ = RECORDER;

    ret = rendererInServer->Stop();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test DisableAllInnerCap API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerDisableAllInnerCap_001
 * @tc.desc  : Test DisableAllInnerCap.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerDisableAllInnerCap_001, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->DisableAllInnerCap();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test EnableInnerCap API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerEnableInnerCap_001
 * @tc.desc  : Test EnableInnerCap when innerCapId is 0.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerEnableInnerCap_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->InitDupStream(1);
    int32_t ret = rendererInServer->EnableInnerCap(0);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test InitDupStream API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerInitDupStream_001
 * @tc.desc  : Test InitDupStream interface, Set offloadEnable_ is true.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerInitDupStream_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->offloadEnable_ = true;
    rendererInServer->status_ = I_STATUS_STARTED;
    int32_t ret = rendererInServer->InitDupStream(1);

    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test InitDupStream API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerInitDupStream_002
 * @tc.desc  : Test InitDupStream when silentModeAndMixWithOthers_ is true.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerInitDupStream_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->offloadEnable_ = 1;
    rendererInServer->status_ = I_STATUS_STARTED;
    rendererInServer->silentModeAndMixWithOthers_ = true;
    int32_t ret = rendererInServer->InitDupStream(1);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test InitDupStream API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerInitDupStream_003
 * @tc.desc  : Test InitDupStream when isMuted_ is true.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerInitDupStream_003, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->offloadEnable_ = 1;
    rendererInServer->status_ = I_STATUS_STARTED;
    rendererInServer->isMuted_ = true;
    int32_t ret = rendererInServer->InitDupStream(1);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test InitDupStream API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerInitDupStream_004
 * @tc.desc  : Test InitDupStream when muteFlag_ is true.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerInitDupStream_004, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->offloadEnable_ = 1;
    rendererInServer->status_ = I_STATUS_STARTED;
    rendererInServer->muteFlag_ = true;
    int32_t ret = rendererInServer->InitDupStream(1);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test InitDupStream API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerInitDupStream_005
 * @tc.desc  : Test InitDupStream when muteFlag_ and isMuted_ and silentModeAndMixWithOthers_ are false.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerInitDupStream_005, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->offloadEnable_ = 1;
    rendererInServer->status_ = I_STATUS_STARTED;
    rendererInServer->muteFlag_ = false;
    rendererInServer->isMuted_ = false;
    rendererInServer->silentModeAndMixWithOthers_ = false;
    int32_t ret = rendererInServer->InitDupStream(1);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test OnStatusUpdate API
 * @tc.type  : FUNC
 * @tc.number: OnStatusUpdate_001
 * @tc.desc  : Test OnStatusUpdate when IOperation is OPERATION_STARTED.
 */
HWTEST_F(RendererInServerThirdUnitTest, OnStatusUpdate_001, TestSize.Level1)
{
    std::shared_ptr<StreamCallbacks> streamCallbacks;
    streamCallbacks = std::make_shared<StreamCallbacks>(TEST_STREAMINDEX);
    EXPECT_NE(nullptr, streamCallbacks);

    streamCallbacks->OnStatusUpdate(IOperation::OPERATION_STARTED);

    EXPECT_EQ(64, streamCallbacks->streamIndex_);
}

/**
 * @tc.name  : Test OnWriteData API
 * @tc.type  : FUNC
 * @tc.number: OnWriteData_001
 * @tc.desc  : Test OnWriteData when IOperation is OPERATION_STARTED.
 */
HWTEST_F(RendererInServerThirdUnitTest, OnWriteData_001, TestSize.Level1)
{
    std::shared_ptr<StreamCallbacks> streamCallbacks;
    streamCallbacks = std::make_shared<StreamCallbacks>(TEST_STREAMINDEX);
    EXPECT_NE(nullptr, streamCallbacks);

    streamCallbacks->OnStatusUpdate(IOperation::OPERATION_STARTED);
    int8_t inputData[10] = {0};
    size_t requestDataLen = 10;
    EXPECT_EQ(streamCallbacks->OnWriteData(inputData, requestDataLen), SUCCESS);
}

/**
 * @tc.name  : Test OnWriteData API
 * @tc.type  : FUNC
 * @tc.number: OnWriteData_002
 * @tc.desc  : Test OnWriteData when recoveryAntiShakeBufferCount_ is 1.
 */
HWTEST_F(RendererInServerThirdUnitTest, OnWriteData_002, TestSize.Level1)
{
    std::shared_ptr<StreamCallbacks> streamCallbacks;
    streamCallbacks = std::make_shared<StreamCallbacks>(TEST_STREAMINDEX);
    EXPECT_NE(nullptr, streamCallbacks);

    streamCallbacks->OnStatusUpdate(IOperation::OPERATION_STARTED);
    int8_t inputData[10] = {0};
    size_t requestDataLen = 10;
    streamCallbacks->recoveryAntiShakeBufferCount_ = 1;
    EXPECT_EQ(streamCallbacks->OnWriteData(inputData, requestDataLen), SUCCESS);
}

/**
 * @tc.name  : Test GetAvailableSize API
 * @tc.type  : FUNC
 * @tc.number: GetAvailableSize_001
 * @tc.desc  : Test GetAvailableSize when length is 10.
 */
HWTEST_F(RendererInServerThirdUnitTest, GetAvailableSize_001, TestSize.Level1)
{
    std::shared_ptr<StreamCallbacks> streamCallbacks;
    streamCallbacks = std::make_shared<StreamCallbacks>(TEST_STREAMINDEX);
    EXPECT_NE(nullptr, streamCallbacks);
    
    size_t length = 0;
    int32_t ret = streamCallbacks->GetAvailableSize(length);
    EXPECT_EQ(ret, ERROR);
}


/**
 * @tc.name  : Test IsHighResolution API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerIsHighResolution_001
 * @tc.desc  : Test IsHighResolution.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerIsHighResolution_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);
    
    int32_t ret = rendererInServer->IsHighResolution();
    EXPECT_EQ(ret, 1);
}

/**
 * @tc.name  : Test SetSilentModeAndMixWithOthers API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetSilentModeAndMixWithOthers_001
 * @tc.desc  : Test SetSilentModeAndMixWithOthers, set isMuted_ and on are true.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerSetSilentModeAndMixWithOthers_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    bool on = true;
    rendererInServer->isMuted_ = true;
    int32_t ret = rendererInServer->SetSilentModeAndMixWithOthers(on);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetSilentModeAndMixWithOthers API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetSilentModeAndMixWithOthers_002
 * @tc.desc  : Test SetSilentModeAndMixWithOthers, set isMuted_ and on and muteFlag_ are true.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerSetSilentModeAndMixWithOthers_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    bool on = true;
    rendererInServer->isMuted_ = true;
    rendererInServer->muteFlag_ = true;
    int32_t ret = rendererInServer->SetSilentModeAndMixWithOthers(on);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetSilentModeAndMixWithOthers API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetSilentModeAndMixWithOthers_003
 * @tc.desc  : Test SetSilentModeAndMixWithOthers, set on is false.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerSetSilentModeAndMixWithOthers_003, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    bool on = false;
    int32_t ret = rendererInServer->SetSilentModeAndMixWithOthers(on);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetSilentModeAndMixWithOthers API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetSilentModeAndMixWithOthers_004
 * @tc.desc  : Test SetSilentModeAndMixWithOthers, set on is true and isDualToneEnabled_ is true.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerSetSilentModeAndMixWithOthers_004, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    bool on = true;
    rendererInServer->isDualToneEnabled_ = true;
    int32_t ret = rendererInServer->SetSilentModeAndMixWithOthers(on);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetClientVolume API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetClientVolume_001
 * @tc.desc  : Test SetClientVolume, set isDualToneEnabled_ is true.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerSetClientVolume_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->managerType_ = DIRECT_PLAYBACK;
    rendererInServer->Init();
    rendererInServer->isDualToneEnabled_ = true;
    int32_t ret = rendererInServer->SetClientVolume();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetMute API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetMute_001
 * @tc.desc  : Test SetMute, set isMute and muteFlag_ are true.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerSetMute_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    bool isMute = true;
    rendererInServer->muteFlag_ = true;
    int32_t ret = rendererInServer->SetMute(isMute);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetMute API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetMute_002
 * @tc.desc  : Test SetMute, set isMute and silentModeAndMixWithOthers_ and muteFlag_ are true.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerSetMute_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    bool isMute = true;
    rendererInServer->silentModeAndMixWithOthers_ = true;
    rendererInServer->muteFlag_ = true;
    int32_t ret = rendererInServer->SetMute(isMute);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetMute API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetMute_003
 * @tc.desc  : Test SetMute, set isMute and silentModeAndMixWithOthers_ and muteFlag_ are false.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerSetMute_003, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    bool isMute = false;
    rendererInServer->silentModeAndMixWithOthers_ = false;
    rendererInServer->muteFlag_ = false;
    int32_t ret = rendererInServer->SetMute(isMute);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetMute API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetMute_004
 * @tc.desc  : Test SetMute, set isMute and isDualToneEnabled_ are true.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerSetMute_004, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    bool isMute = true;
    rendererInServer->isDualToneEnabled_ = true;
    int32_t ret = rendererInServer->SetMute(isMute);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetDuckFactor API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetDuckFactor_001
 * @tc.desc  : Test SetDuckFactor, set duckFactor < MIN_FLOAT_VOLUME and duckFactor > MAX_FLOAT_VOLUME.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerSetDuckFactor_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    float duckFactor = 0.2f;
    rendererInServer->isDualToneEnabled_ = true;
    int32_t ret = rendererInServer->SetDuckFactor(duckFactor);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetDuckFactor API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetDuckFactor_002
 * @tc.desc  : Test SetDuckFactor, set duckFactor < MIN_FLOAT_VOLUME.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerSetDuckFactor_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    float duckFactor = -0.2f;
    int32_t ret = rendererInServer->SetDuckFactor(duckFactor);
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetDuckFactor API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetDuckFactor_003
 * @tc.desc  : Test SetDuckFactor, set duckFactor > MAX_FLOAT_VOLUME.
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerSetDuckFactor_003, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    float duckFactor = 2.0f;
    int32_t ret = rendererInServer->SetDuckFactor(duckFactor);
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetTarget API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetTarget_001
 * @tc.desc  : wzwzwz
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerSetTarget_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);
    int32_t ret = rendererInServer->Init();
    rendererInServer->lastTarget_ = NORMAL_PLAYBACK;
    rendererInServer->SetTarget(NORMAL_PLAYBACK, ret);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetTarget API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetTarget_002
 * @tc.desc  : wzwzwz
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerSetTarget_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);
    int32_t ret = rendererInServer->Init();
    rendererInServer->lastTarget_ = NORMAL_PLAYBACK;
    rendererInServer->SetTarget(INJECT_TO_VOICE_COMMUNICATION_CAPTURE, ret);
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetTarget API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetTarget_003
 * @tc.desc  : wzwzwz
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerSetTarget_003, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);
    int32_t ret = rendererInServer->Init();
    rendererInServer->lastTarget_ = NORMAL_PLAYBACK;
    rendererInServer->status_ = I_STATUS_IDLE;
    rendererInServer->SetTarget(INJECT_TO_VOICE_COMMUNICATION_CAPTURE, ret);
    EXPECT_NE(SUCCESS, ret);

    rendererInServer->status_ = I_STATUS_INVALID;
    rendererInServer->SetTarget(INJECT_TO_VOICE_COMMUNICATION_CAPTURE, ret);
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetTarget API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetTarget_004
 * @tc.desc  : wzwzwz
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerSetTarget_004, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);
    int32_t ret = rendererInServer->Init();
    rendererInServer->lastTarget_ = NORMAL_PLAYBACK;
    rendererInServer->status_ = I_STATUS_PAUSED;
    rendererInServer->SetTarget(INJECT_TO_VOICE_COMMUNICATION_CAPTURE, ret);
    EXPECT_NE(SUCCESS, ret);

    rendererInServer->status_ = I_STATUS_INVALID;
    rendererInServer->SetTarget(INJECT_TO_VOICE_COMMUNICATION_CAPTURE, ret);
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetTarget API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetTarget_005
 * @tc.desc  : wzwzwz
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerSetTarget_005, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);
    int32_t ret = rendererInServer->Init();
    rendererInServer->lastTarget_ = NORMAL_PLAYBACK;
    rendererInServer->status_ = I_STATUS_STOPPED;
    rendererInServer->SetTarget(INJECT_TO_VOICE_COMMUNICATION_CAPTURE, ret);
    EXPECT_NE(SUCCESS, ret);

    rendererInServer->status_ = I_STATUS_INVALID;
    rendererInServer->SetTarget(INJECT_TO_VOICE_COMMUNICATION_CAPTURE, ret);
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetTarget API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetTarget_006
 * @tc.desc  : wzwzwz
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerSetTarget_006, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);
    int32_t ret = rendererInServer->Init();
    rendererInServer->lastTarget_ = NORMAL_PLAYBACK;
    rendererInServer->status_ = I_STATUS_INVALID;
    rendererInServer->SetTarget(INJECT_TO_VOICE_COMMUNICATION_CAPTURE, ret);
    EXPECT_NE(SUCCESS, ret);

    rendererInServer->lastTarget_ = INJECT_TO_VOICE_COMMUNICATION_CAPTURE;
    rendererInServer->status_ = I_STATUS_INVALID;
    rendererInServer->SetTarget(NORMAL_PLAYBACK, ret);
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test WriteDupBufferInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerWriteDupBufferInner_003
 * @tc.desc  : test writeDupBufferInner interface in injector mode
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerWriteDupBufferInner_003, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_GAME;
    auto server = std::make_shared<RendererInServer>(processConfig, stateListener);
    ASSERT_TRUE(server != nullptr);

    CaptureInfo captureInfo;
    captureInfo.isInnerCapEnabled = true;
    captureInfo.dupStream = nullptr;
    int32_t innerCapId = 1;
    uint32_t streamIndex = 0;
    auto streamCallbacks = std::make_shared<StreamCallbacks>(streamIndex);
    server->lastTarget_ = INJECT_TO_VOICE_COMMUNICATION_CAPTURE;
    server->innerCapIdToDupStreamCallbackMap_.insert({innerCapId, streamCallbacks});
    int32_t length = 10000;
    server->innerCapIdToDupStreamCallbackMap_[innerCapId]->GetDupRingBuffer() = AudioRingCache::Create(length);
    
    auto buffer = std::make_unique<uint8_t []>(length);
    BufferDesc emptyBufferDesc = {buffer.get(), length, length};
    memset_s(emptyBufferDesc.buffer, emptyBufferDesc.bufLength, 1, emptyBufferDesc.bufLength);
    EXPECT_EQ(uint8_t(buffer[0]), 1);
    EXPECT_EQ(server->WriteDupBufferInner(emptyBufferDesc, innerCapId), SUCCESS);

    BufferWrap bufferWrap = {buffer.get(), length};
    server->innerCapIdToDupStreamCallbackMap_[innerCapId]->GetDupRingBuffer()->Dequeue(bufferWrap);
    EXPECT_EQ(uint8_t(buffer[0]), 0);
}

/**
 * @tc.name  : Test WaitForDataConnection API
 * @tc.type  : FUNC
 * @tc.number: WaitForDataConnection
 * @tc.desc  : Test WaitForDataConnection
 */
HWTEST_F(RendererInServerThirdUnitTest, WaitForDataConnection_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->WaitForDataConnection();
    rendererInServer->OnDataLinkConnectionUpdate(IOperation::OPERATION_DATA_LINK_CONNECTING);
    EXPECT_EQ(rendererInServer->isDataLinkConnected_, false);

    rendererInServer->WaitForDataConnection();
    rendererInServer->OnDataLinkConnectionUpdate(IOperation::OPERATION_DATA_LINK_CONNECTED);
    EXPECT_EQ(rendererInServer->isDataLinkConnected_, true);
}

/**
 * @tc.name  : Test UpdateStreamInfo API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerUpdateStreamInfo_001
 * @tc.desc  : test UpdateStreamInfo
 */
HWTEST_F(RendererInServerThirdUnitTest, RendererInServerUpdateStreamInfo_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);
    rendererInServer->UpdateStreamInfo();
    EXPECT_GT(rendererInServer->checkCount_, 0);

    uint32_t checkCount = rendererInServer->checkCount_;
    rendererInServer->UpdateStreamInfo();

    EXPECT_EQ(checkCount, rendererInServer->checkCount_ + 1);
}
} // namespace AudioStandard
} // namespace OHOS