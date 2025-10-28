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

#include "renderer_in_server_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

const AudioSamplingRate SAMPLING_RATE_ERROR_0 = static_cast<AudioSamplingRate>(1);
const AudioSamplingRate SAMPLING_RATE_ERROR_OVER = static_cast<AudioSamplingRate>(132000000);
const AudioChannel AUDIO_CHANNEL_ERROR_0 = static_cast<AudioChannel>(0);
const AudioChannel AUDIO_CHANNEL_ERROR_1 = static_cast<AudioChannel>(1);
const AudioSampleFormat AUDIO_SAMPLE_FORMAT_ERROR = static_cast<AudioSampleFormat>(5);
const uint32_t TEST_STREAMINDEX = 64;
const uint64_t TEST_LATENCY = 123456;
const uint64_t TEST_FRAMEPOS = 123456;
const uint64_t TEST_TIMESTAMP = 111111;
const int32_t TEST_STATE = 2;
const int32_t TEST_RATE = 2;
const int32_t TEST_EFFECTMODE = 3;
const int32_t TEST_PRIVACYTYPE = 1;
const int32_t OUT_OF_STATUS = 14;
const int32_t NO_FADING = 0;
const int32_t DO_FADINGOUT = 1;
const int32_t FADING_OUT_DONE = 2;
const size_t TEST_LENGTH = 512;
const float OUT_OF_MAX_FLOAT_VOLUME = 2.0f;
const float IN_VOLUME_RANGE = 0.5f;
const float OUT_OF_MIN_FLOAT_VOLUME = -2.0f;
const bool TEST_ISAPPBACK = true;
const uint32_t SAMPLE_RATE_383840 = 383840;
constexpr int32_t DEFAULT_SPAN_SIZE = 2;

static std::shared_ptr<IStreamListener> stateListener;
static std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
static std::shared_ptr<RendererInServer> rendererInServer;
static std::shared_ptr<OHAudioBuffer> buffer;
static std::weak_ptr<IStreamListener> streamListener;

static AudioProcessConfig processConfig;
static BufferDesc bufferDesc;

static AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
    AudioChannelLayout::CH_LAYOUT_UNKNOWN);

void RendererInServerUnitTest::SetUpTestCase(void) {}

void RendererInServerUnitTest::TearDownTestCase(void)
{
    stateListener.reset();
    streamListenerHolder.reset();
    rendererInServer.reset();
    buffer.reset();
    streamListener.reset();
}

void RendererInServerUnitTest::SetUp(void)
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

void RendererInServerUnitTest::TearDown(void) {}

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
 * @tc.name  : Test Init API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerInit_001
 * @tc.desc  : Test Init API when managerType_ is VOIP_PLAYBACK.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerInit_001, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_FLAG_VOIP_DIRECT);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Init API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerInit_002
 * @tc.desc  : Test Init API when managerType_ is DIRECT_PLAYBACK.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerInit_002, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Init API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerInit_003
 * @tc.desc  : Test Init API when managerType_ is PLAYBACK.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerInit_003, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_FLAG_VOIP_DIRECT);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    ret = rendererInServer->Init();
    EXPECT_EQ(PLAYBACK, rendererInServer->managerType_);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Init API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerInit_004
 * @tc.desc  : Test Init API when ConfigServerBuffer return Error with spanSizeInFrame_ is 0.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerInit_004, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLING_RATE_ERROR_0, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);
}

/**
 * @tc.name  : Test ConfigServerBuffer API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerInit_005
 * @tc.desc  : Test Init API when ConfigServerBuffer return Error with byteSizePerFrame_ is 0.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerInit_005, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, AUDIO_CHANNEL_ERROR_0,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test ConfigServerBuffer API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerInit_006
 * @tc.desc  : Test ConfigServerBuffer when audioServerBuffer_ create failed.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerInit_006, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLING_RATE_ERROR_OVER, ENCODING_INVALID, SAMPLE_S24LE, AUDIO_CHANNEL_ERROR_1,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);

    int32_t ret = rendererInServer->Init();
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);
}

/**
 * @tc.name  : Test ConfigServerBuffer API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerConfigServerBuffer_001
 * @tc.desc  : Test ConfigServerBuffer when audioServerBuffer_ is not nullptr.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerConfigServerBuffer_001, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    ret = rendererInServer->ConfigServerBuffer();
    EXPECT_EQ(rendererInServer->bufferTotalSizeInFrame_, (MAX_CBBUF_IN_USEC * DEFAULT_SPAN_SIZE + MIN_CBBUF_IN_USEC) *
        testStreamInfo.samplingRate / AUDIO_US_PER_S);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test ConfigServerBuffer API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerConfigServerBuffer_002
 * @tc.desc  : Test ConfigServerBuffer when using customSampleRate.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerConfigServerBuffer_002, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_8000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    testStreamInfo.customSampleRate = SAMPLE_RATE_383840;
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    ret = rendererInServer->ConfigServerBuffer();
    EXPECT_EQ(rendererInServer->bufferTotalSizeInFrame_, (MAX_CBBUF_IN_USEC * DEFAULT_SPAN_SIZE + MIN_CBBUF_IN_USEC) *
        testStreamInfo.customSampleRate / AUDIO_US_PER_S);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test InitBufferStatus API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerInitBufferStatus_001
 * @tc.desc  : Test ConfigServerBuffer when audioServerBuffer_ is not nullptr.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerInitBufferStatus_001, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->audioServerBuffer_ = nullptr;
    int32_t ret = rendererInServer->InitBufferStatus();
    EXPECT_EQ(ERR_ILLEGAL_STATE, ret);
}

/**
 * @tc.name  : Test OnStatusUpdate API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerOnStatusUpdate_001
 * @tc.desc  : Test OnStatusUpdate when operation is OPERATION_FLUSHED.
 *             Test HandleOperationFlushed when status_ is I_STATUS_IDLE.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerOnStatusUpdate_001, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);
    rendererInServer->OnStatusUpdate(OPERATION_FLUSHED);
    EXPECT_EQ(I_STATUS_IDLE, rendererInServer->status_);
}

/**
 * @tc.name  : Test OnStatusUpdate API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerOnStatusUpdate_002
 * @tc.desc  : Test OnStatusUpdate when operation is OPERATION_DRAINED but status_ is not I_STATUS_DRAINING.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerOnStatusUpdate_002, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);
    rendererInServer->OnStatusUpdate(OPERATION_DRAINED);
    EXPECT_TRUE(rendererInServer->afterDrain);
}

/**
 * @tc.name  : Test OnStatusUpdate API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerOnStatusUpdate_003
 * @tc.desc  : Test OnStatusUpdate when operation is OPERATION_DRAINED but status_ is I_STATUS_DRAINING.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerOnStatusUpdate_003, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);
    rendererInServer->status_ = I_STATUS_DRAINING;
    rendererInServer->OnStatusUpdate(OPERATION_DRAINED);
    EXPECT_EQ(I_STATUS_STARTED, rendererInServer->status_);
    EXPECT_TRUE(rendererInServer->afterDrain);
}

/**
 * @tc.name  : Test OnStatusUpdate API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerOnStatusUpdate_004
 * @tc.desc  : Test OnStatusUpdate when operation is OPERATION_SET_OFFLOAD_ENABLE.
 *             Test OnStatusUpdateSub when operation is OPERATION_SET_OFFLOAD_ENABLE.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerOnStatusUpdate_004, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);
    rendererInServer->OnStatusUpdate(OPERATION_SET_OFFLOAD_ENABLE);
    EXPECT_TRUE(rendererInServer->offloadEnable_);
}

/**
 * @tc.name  : Test OnStatusUpdateSub API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerOnStatusUpdateSub_001
 * @tc.desc  : Test OnStatusUpdateSub when operation is OPERATION_RELEASED.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerOnStatusUpdateSub_001, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);
    rendererInServer->OnStatusUpdateSub(OPERATION_RELEASED);
    EXPECT_EQ(I_STATUS_RELEASED, rendererInServer->status_);
}

/**
 * @tc.name  : Test OnStatusUpdateSub API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerOnStatusUpdateSub_002
 * @tc.desc  : Test OnStatusUpdateSub when operation is OPERATION_UNDERRUN and buffer is empty.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerOnStatusUpdateSub_002, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);
    rendererInServer->audioServerBuffer_->basicBufferInfo_->curWriteFrame.store(0);
    rendererInServer->audioServerBuffer_->basicBufferInfo_->curReadFrame.store(0);
    rendererInServer->audioServerBuffer_->totalSizeInFrame_ = 4;
    rendererInServer->spanSizeInFrame_ = 1;
    rendererInServer->OnStatusUpdateSub(OPERATION_UNDERRUN);
    EXPECT_EQ(0, rendererInServer->needForceWrite_);
}

/**
 * @tc.name  : Test OnStatusUpdateSub API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerOnStatusUpdateSub_003
 * @tc.desc  : Test OnStatusUpdateSub when operation is OPERATION_UNDERRUN and buffer is not empty.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerOnStatusUpdateSub_003, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);
    rendererInServer->audioServerBuffer_->basicBufferInfo_->curWriteFrame.store(4);
    rendererInServer->audioServerBuffer_->basicBufferInfo_->curReadFrame.store(0);
    rendererInServer->audioServerBuffer_->basicBufferInfo_->basePosInFrame.store(4);
    rendererInServer->audioServerBuffer_->totalSizeInFrame_ = 8;
    rendererInServer->spanSizeInFrame_ = 1;

    rendererInServer->standByCounter_ = 1;
    rendererInServer->OnStatusUpdateSub(OPERATION_UNDERRUN);
    EXPECT_EQ(0, rendererInServer->standByCounter_);
}

/**
 * @tc.name  : Test OnStatusUpdateSub API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerOnStatusUpdateSub_004
 * @tc.desc  : Test OnStatusUpdateSub when operation is OPERATION_UNSET_OFFLOAD_ENABLE.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerOnStatusUpdateSub_004, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);
    rendererInServer->OnStatusUpdateSub(OPERATION_UNSET_OFFLOAD_ENABLE);
    EXPECT_FALSE(rendererInServer->offloadEnable_);
}

/**
 * @tc.name  : Test OnStatusUpdateSub API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerOnStatusUpdateSub_005
 * @tc.desc  : Test OnStatusUpdateSub when operation is OPERATION_UNDERFLOW and startTime_ is 0.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerOnStatusUpdateSub_005, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);
    rendererInServer->startedTime_ = 0;
    rendererInServer->OnStatusUpdateSub(OPERATION_UNDERFLOW);
    EXPECT_EQ(1, rendererInServer->underrunCount_);
}

/**
 * @tc.name  : Test OnStatusUpdateSub API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerOnStatusUpdateSub_006
 * @tc.desc  : Test OnStatusUpdateSub when operation is OPERATION_UNDERFLOW and result not over START_MIN_COST.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerOnStatusUpdateSub_006, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();

    struct timespec time;
    ret = clock_gettime(CLOCK_MONOTONIC, &time);
    rendererInServer->startedTime_ = (time.tv_sec * AUDIO_NS_PER_SECOND) + time.tv_nsec;
    rendererInServer->OnStatusUpdateSub(OPERATION_UNDERFLOW);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(0, rendererInServer->underrunCount_);
}

/**
 * @tc.name  : Test OnStatusUpdateSub API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerOnStatusUpdateSub_007
 * @tc.desc  : Test OnStatusUpdateSub when operation is OPERATION_STARTED.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerOnStatusUpdateSub_007, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);
    rendererInServer->OnStatusUpdateSub(OPERATION_STARTED);
    EXPECT_EQ(I_STATUS_INVALID, rendererInServer->status_);
}

/**
 * @tc.name  : Test HandleOperationFlushed API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerHandleOperationFlushed_001
 * @tc.desc  : Test OnStatusUpdate when status_ is I_STATUS_FLUSHING_WHEN_STARTED.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerHandleOperationFlushed_001, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);
    rendererInServer->status_ = I_STATUS_FLUSHING_WHEN_STARTED;
    rendererInServer->HandleOperationFlushed();
    EXPECT_EQ(I_STATUS_STARTED, rendererInServer->status_);
}

/**
 * @tc.name  : Test HandleOperationFlushed API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerHandleOperationFlushed_002
 * @tc.desc  : Test OnStatusUpdate when status_ is I_STATUS_FLUSHING_WHEN_PAUSED.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerHandleOperationFlushed_002, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);
    rendererInServer->status_ = I_STATUS_FLUSHING_WHEN_PAUSED;
    rendererInServer->HandleOperationFlushed();
    EXPECT_EQ(I_STATUS_PAUSED, rendererInServer->status_);
}

/**
 * @tc.name  : Test HandleOperationFlushed API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerHandleOperationFlushed_003
 * @tc.desc  : Test OnStatusUpdate when status_ is I_STATUS_FLUSHING_WHEN_STOPPED.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerHandleOperationFlushed_003, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);
    rendererInServer->status_ = I_STATUS_FLUSHING_WHEN_STOPPED;
    rendererInServer->HandleOperationFlushed();
    EXPECT_EQ(I_STATUS_STOPPED, rendererInServer->status_);
}

/**
 * @tc.name  : Test DequeueBuffer API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerDequeueBuffer_001
 * @tc.desc  : Test normal DequeueBuffer.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerDequeueBuffer_001, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    size_t length = 10;
    bufferDesc = rendererInServer->DequeueBuffer(length);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test DoFadingOut API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerDoFadingOut_001
 * @tc.desc  : Test DoFadingOut when fadeoutFlag_ is not DO_FADINGOUT.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerDoFadingOut_001, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);
    rendererInServer->fadeoutFlag_ = NO_FADING;
    RingBufferWrapper bufferWrapper = {
        .basicBufferDescs = {{
            {bufferDesc.buffer, bufferDesc.bufLength},
            {}
        }},
        .dataLength = bufferDesc.dataLength
    };
    rendererInServer->DoFadingOut(bufferWrapper);
    EXPECT_NE(FADING_OUT_DONE, rendererInServer->fadeoutFlag_);
}

/**
 * @tc.name  : Test DoFadingOut API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerDoFadingOut_002
 * @tc.desc  : Test DoFadingOut when fadeoutFlag_ is DO_FADINGOUT and Process failed.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerDoFadingOut_002, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, AUDIO_SAMPLE_FORMAT_ERROR, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->fadeoutFlag_ = DO_FADINGOUT;
    RingBufferWrapper bufferWrapper = {
        .basicBufferDescs = {{
            {bufferDesc.buffer, bufferDesc.bufLength},
            {}
        }},
        .dataLength = bufferDesc.dataLength
    };
    rendererInServer->DoFadingOut(bufferWrapper);
    EXPECT_EQ(FADING_OUT_DONE, rendererInServer->fadeoutFlag_);
}

/**
 * @tc.name  : Test DoFadingOut API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerDoFadingOut_003
 * @tc.desc  : Test DoFadingOut when fadeoutFlag_ is DO_FADINGOUT and Process success.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerDoFadingOut_003, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->fadeoutFlag_ = DO_FADINGOUT;
        RingBufferWrapper bufferWrapper = {
        .basicBufferDescs = {{
            {bufferDesc.buffer, bufferDesc.bufLength},
            {}
        }},
        .dataLength = bufferDesc.dataLength
    };
    rendererInServer->DoFadingOut(bufferWrapper);
    EXPECT_EQ(FADING_OUT_DONE, rendererInServer->fadeoutFlag_);
}

/**
 * @tc.name  : Test VolumeHandle API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerVolumeHandle_001
 * @tc.desc  : Test VolumeHandle interface.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerVolumeHandle_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->VolumeHandle(bufferDesc);

    EXPECT_EQ(nullptr, rendererInServer->audioServerBuffer_);
}

/**
 * @tc.name  : Test VolumeHandle API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerVolumeHandle_002
 * @tc.desc  : Test VolumeHandle when condition all true.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerVolumeHandle_002, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);
    rendererInServer->lowPowerVolume_ = 0.0f;
    rendererInServer->audioServerBuffer_->basicBufferInfo_->duckFactor.store(0.0f);
    rendererInServer->silentModeAndMixWithOthers_ = 0;

    rendererInServer->VolumeHandle(bufferDesc);
    EXPECT_EQ(0.0f, rendererInServer->oldAppliedVolume_);
}

/**
 * @tc.name  : Test VolumeHandle API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerVolumeHandle_003
 * @tc.desc  : Test VolumeHandle when oldAppliedVolume_ is different from applyVolume.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerVolumeHandle_003, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);
    rendererInServer->oldAppliedVolume_ = 0.0f;

    rendererInServer->VolumeHandle(bufferDesc);
    EXPECT_EQ(1.0f, rendererInServer->oldAppliedVolume_);
}

/**
 * @tc.name  : Test VolumeHandle API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerVolumeHandle_004
 * @tc.desc  : Test VolumeHandle when Process failed.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerVolumeHandle_004, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);
    rendererInServer->oldAppliedVolume_ = 0.0f;
    bufferDesc.buffer = nullptr;

    rendererInServer->VolumeHandle(bufferDesc);
    EXPECT_EQ(1.0f, rendererInServer->oldAppliedVolume_);
}

/**
 * @tc.name  : Test WriteData API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerWriteData_001
 * @tc.desc  : Test WriteData when return error message.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerWriteData_001, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    rendererInServer->audioServerBuffer_->basicBufferInfo_->curWriteFrame.store(0);
    rendererInServer->audioServerBuffer_->basicBufferInfo_->curReadFrame.store(4);
    rendererInServer->spanSizeInFrame_ = 4;

    ret = rendererInServer->WriteData();
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);
}

/**
 * @tc.name  : Test WriteData API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerWriteData_002
 * @tc.desc  : Test WriteData when currentReadFrame add spanSizeInFrame_ is same with currentWriteFrame.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerWriteData_002, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    rendererInServer->audioServerBuffer_->basicBufferInfo_->curWriteFrame.store(8);
    rendererInServer->audioServerBuffer_->basicBufferInfo_->curReadFrame.store(4);
    rendererInServer->audioServerBuffer_->basicBufferInfo_->totalSizeInFrame = 16;
    rendererInServer->spanSizeInFrame_ = 4;

    ret = rendererInServer->WriteData();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test WriteData API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerWriteData_003
 * @tc.desc  : Test WriteData when currentReadFrame add spanSizeInFrame_ is different from currentWriteFrame.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerWriteData_003, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    rendererInServer->audioServerBuffer_->basicBufferInfo_->curWriteFrame.store(12);
    rendererInServer->audioServerBuffer_->basicBufferInfo_->curReadFrame.store(4);
    rendererInServer->audioServerBuffer_->basicBufferInfo_->totalSizeInFrame = 16;
    rendererInServer->spanSizeInFrame_ = 4;

    ret = rendererInServer->WriteData();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test WriteData API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerWriteData_004
 * @tc.desc  : Test WriteData when currentReadFrame add spanSizeInFrame_ is same with currentWriteFrame.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerWriteData_004, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_WIRED_HEADSET, AUDIO_FLAG_NORMAL, STREAM_ULTRASONIC);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    rendererInServer->audioServerBuffer_->basicBufferInfo_->curWriteFrame.store(8);
    rendererInServer->audioServerBuffer_->basicBufferInfo_->curReadFrame.store(4);
    rendererInServer->audioServerBuffer_->basicBufferInfo_->totalSizeInFrame = 16;
    rendererInServer->spanSizeInFrame_ = 4;

    ret = rendererInServer->WriteData();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test WriteData API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerWriteData_005
 * @tc.desc  : Test WriteData when currentReadFrame add spanSizeInFrame_ is same with currentWriteFrame.
 *             Test normal OtherStreamEnqueue.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerWriteData_005, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_WIRED_HEADSET, AUDIO_FLAG_NORMAL, STREAM_ULTRASONIC);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    rendererInServer->audioServerBuffer_->basicBufferInfo_->curWriteFrame.store(8);
    rendererInServer->audioServerBuffer_->basicBufferInfo_->curReadFrame.store(4);
    rendererInServer->audioServerBuffer_->basicBufferInfo_->totalSizeInFrame = 16;
    rendererInServer->spanSizeInFrame_ = 4;
    ret = rendererInServer->InitDupStream(1);
    ret = rendererInServer->EnableDualTone("Speaker");

    ret = rendererInServer->WriteData();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test WriteData API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerWriteData_006
 * @tc.desc  : Test WriteData when currentReadFrame add spanSizeInFrame_ is same with currentWriteFrame.
 *             Test OtherStreamEnqueue but dupStream_ and dualToneStream_ are nullptr.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerWriteData_006, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_WIRED_HEADSET, AUDIO_FLAG_NORMAL, STREAM_ULTRASONIC);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    rendererInServer->audioServerBuffer_->basicBufferInfo_->curWriteFrame.store(8);
    rendererInServer->audioServerBuffer_->basicBufferInfo_->curReadFrame.store(4);
    rendererInServer->audioServerBuffer_->basicBufferInfo_->totalSizeInFrame = 16;
    rendererInServer->spanSizeInFrame_ = 4;
    ret = rendererInServer->InitDupStream(1);
    ret = rendererInServer->EnableDualTone("Speaker");
    if (rendererInServer->captureInfos_.count(1)) {
        rendererInServer->captureInfos_[1].dupStream = nullptr;
        rendererInServer->captureInfos_.erase(1);
    }
    rendererInServer->dualToneStream_ = nullptr;

    ret = rendererInServer->WriteData();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test WriteData API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerWriteData_007
 * @tc.desc  : Test WriteData when currentReadFrame add spanSizeInFrame_ is same with currentWriteFrame.
 *             Test WriteMuteDataSysEvent when silentModeAndMixWithOthers_ is true.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerWriteData_007, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    rendererInServer->audioServerBuffer_->basicBufferInfo_->curWriteFrame.store(8);
    rendererInServer->audioServerBuffer_->basicBufferInfo_->curReadFrame.store(4);
    rendererInServer->audioServerBuffer_->basicBufferInfo_->totalSizeInFrame = 16;
    rendererInServer->spanSizeInFrame_ = 4;
    rendererInServer->silentModeAndMixWithOthers_ = true;

    ret = rendererInServer->WriteData();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test ResolveBuffer API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerResolveBuffer_001
 * @tc.desc  : Test normal ResolveBuffer.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerResolveBuffer_001, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->ResolveBuffer(buffer);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test GetSessionId API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerGetSessionId_001
 * @tc.desc  : Test normal GetSessionId.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerGetSessionId_001, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    uint32_t sessionId = 0;
    int32_t ret = rendererInServer->Init();
    ret = rendererInServer->GetSessionId(sessionId);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test GetSessionId API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerGetSessionId_002
 * @tc.desc  : Test GetSessionId when stream_ is nullptr.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerGetSessionId_002, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    uint32_t sessionId = 0;
    int32_t ret = rendererInServer->GetSessionId(sessionId);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);
}

/**
 * @tc.name  : Test Start API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerStart_001
 * @tc.desc  : Test Start when standByEnable_ false and isInnerCapEnabled_ true and isDualToneEnabled_ true.
 *             Test OnStatusUpdate when operation is OPERATION_RELEASED.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerStart_001, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    ret = rendererInServer->InitDupStream(1);
    ret = rendererInServer->EnableDualTone("Speaker");
    rendererInServer->OnStatusUpdate(OPERATION_RELEASED);

    ret = rendererInServer->Start();
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test Start API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerStart_002
 * @tc.desc  : Test Start when standByEnable_ false and isInnerCapEnabled_ false and isDualToneEnabled_ false.
 *             Test OnStatusUpdate when operation is OPERATION_RELEASED.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerStart_002, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    rendererInServer->OnStatusUpdate(OPERATION_RELEASED);

    ret = rendererInServer->Start();
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test Start API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerStart_003
 * @tc.desc  : Test Start when standByEnable_ false.
 *             Test OnStatusUpdate when operation is OPERATION_PAUSED.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerStart_003, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    rendererInServer->OnStatusUpdate(OPERATION_PAUSED);

    ret = rendererInServer->Start();
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test Start API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerStart_004
 * @tc.desc  : Test Start when standByEnable_ false.
 *             Test OnStatusUpdate when operation is OPERATION_STOPPED.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerStart_004, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    rendererInServer->OnStatusUpdate(OPERATION_STOPPED);

    ret = rendererInServer->Start();
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test Start API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerStart_005
 * @tc.desc  : Test Start when standByEnable_ false.
 *             Test OnStatusUpdate when operation is OPERATION_PAUSED and streamListener_ is nullptr.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerStart_005, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    streamListener.reset();
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    ret = rendererInServer->InitDupStream(1);
    ret = rendererInServer->EnableDualTone("Speaker");
    if (rendererInServer->captureInfos_.count(1)) {
        rendererInServer->captureInfos_[1].dupStream = nullptr;
        rendererInServer->captureInfos_.erase(1);
    }
    rendererInServer->dualToneStream_ = nullptr;
    rendererInServer->OnStatusUpdate(OPERATION_PAUSED);

    ret = rendererInServer->Start();
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test Start API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerStart_006
 * @tc.desc  : Test Start when status_ illegal.
 *             Test OnStatusUpdate when operation is OPERATION_STARTED.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerStart_006, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    rendererInServer->OnStatusUpdate(OPERATION_STARTED);

    ret = rendererInServer->Start();
    EXPECT_NE(true, ret);
}

/**
 * @tc.name  : Test Start API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerStart_007
 * @tc.desc  : Test Start and OnStatusUpdate in OPERATION_STARTED when standByEnable_ true.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerStart_007, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    rendererInServer->standByEnable_ = true;
    rendererInServer->OnStatusUpdate(OPERATION_STARTED);
    rendererInServer->standByEnable_ = true;

    ret = rendererInServer->Start();
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test Start API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerStart_008
 * @tc.desc  : Test Start and OnStatusUpdate in OPERATION_PAUSED when standByEnable_ true with managerType is
 * DIRECT_PLAYBACK.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerStart_008, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    rendererInServer->standByEnable_ = true;
    rendererInServer->OnStatusUpdate(OPERATION_PAUSED);

    ret = rendererInServer->Start();
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test Start API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerStart_009
 * @tc.desc  : Test Start and OnStatusUpdate in OPERATION_PAUSED when standByEnable_ true with managerType is
 * VOIP_PLAYBACK.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerStart_009, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_FLAG_VOIP_DIRECT);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    rendererInServer->standByEnable_ = true;
    rendererInServer->OnStatusUpdate(OPERATION_PAUSED);

    ret = rendererInServer->Start();
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test Pause API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerPause_001
 * @tc.desc  : Test Pause when status_ illegal.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerPause_001, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    rendererInServer->OnStatusUpdate(OPERATION_RELEASED);

    ret = rendererInServer->Pause();
    EXPECT_EQ(ERR_ILLEGAL_STATE, ret);
}

/**
 * @tc.name  : Test Pause API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerPause_002
 * @tc.desc  : Test Pause when isInnerCapEnabled_ and isDualToneEnabled_ are false.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerPause_002, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    rendererInServer->OnStatusUpdate(OPERATION_STARTED);

    ret = rendererInServer->Pause();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Pause API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerPause_003
 * @tc.desc  : Test Pause when isInnerCapEnabled_ and isDualToneEnabled_ are true.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerPause_003, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    ret = rendererInServer->InitDupStream(1);
    ret = rendererInServer->EnableDualTone("Speaker");
    rendererInServer->OnStatusUpdate(OPERATION_STARTED);

    ret = rendererInServer->Pause();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Pause API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerPause_004
 * @tc.desc  : Test Pause when isInnerCapEnabled_ and isDualToneEnabled_ are true but dupStream_ and
 * dualToneStream_ are nullptr.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerPause_004, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    ret = rendererInServer->InitDupStream(1);
    ret = rendererInServer->EnableDualTone("Speaker");
    if (rendererInServer->captureInfos_.count(1)) {
        rendererInServer->captureInfos_[1].dupStream = nullptr;
        rendererInServer->captureInfos_.erase(1);
    }
    rendererInServer->dualToneStream_ = nullptr;
    rendererInServer->OnStatusUpdate(OPERATION_STARTED);

    ret = rendererInServer->Pause();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Pause API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerPause_005
 * @tc.desc  : Test Pause when standByEnable_ is true.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerPause_005, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    rendererInServer->standByEnable_ = true;
    rendererInServer->OnStatusUpdate(OPERATION_PAUSED);
    rendererInServer->standByEnable_ = true;

    ret = rendererInServer->Pause();
    EXPECT_EQ(ERR_ILLEGAL_STATE, ret);
}

/**
 * @tc.name  : Test Flush API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerFlush_001
 * @tc.desc  : Test Flush when status_ illegal.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerFlush_001, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    rendererInServer->OnStatusUpdate(OPERATION_RELEASED);

    ret = rendererInServer->Flush();
    EXPECT_EQ(ERR_ILLEGAL_STATE, ret);
}

/**
 * @tc.name  : Test Flush API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerFlush_002
 * @tc.desc  : Test Flush when status_ is I_STATUS_STARTED.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerFlush_002, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    rendererInServer->OnStatusUpdate(OPERATION_STARTED);

    ret = rendererInServer->Flush();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Flush API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerFlush_003
 * @tc.desc  : Test Flush when status_ is I_STATUS_PAUSED.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerFlush_003, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    rendererInServer->OnStatusUpdate(OPERATION_PAUSED);

    ret = rendererInServer->Flush();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Flush API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerFlush_004
 * @tc.desc  : Test Flush when status_ is I_STATUS_STOPPED.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerFlush_004, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    rendererInServer->OnStatusUpdate(OPERATION_STOPPED);

    ret = rendererInServer->Flush();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Flush API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerFlush_005
 * @tc.desc  : Test Flush when isInnerCapEnabled_ and isDualToneEnabled_ are true.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerFlush_005, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    ret = rendererInServer->InitDupStream(1);
    ret = rendererInServer->EnableDualTone("Speaker");
    rendererInServer->OnStatusUpdate(OPERATION_STARTED);

    ret = rendererInServer->Flush();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Flush API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerFlush_006
 * @tc.desc  : Test Flush when isInnerCapEnabled_ and isDualToneEnabled_ are true but dupStream_ and
 * dualToneStream_ are nullptr.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerFlush_006, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    ret = rendererInServer->InitDupStream(1);
    ret = rendererInServer->EnableDualTone("Speaker");
    if (rendererInServer->captureInfos_.count(1)) {
        rendererInServer->captureInfos_[1].dupStream = nullptr;
        rendererInServer->captureInfos_.erase(1);
    }
    rendererInServer->dualToneStream_ = nullptr;
    rendererInServer->OnStatusUpdate(OPERATION_STARTED);

    ret = rendererInServer->Flush();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Flush API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerFlush_007
 * @tc.desc  : Test Flush when audioServerBuffer_ GetReadbuffer failed
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerFlush_007, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    rendererInServer->OnStatusUpdate(OPERATION_STARTED);
    rendererInServer->audioServerBuffer_->basicBufferInfo_->curWriteFrame.store(10);
    rendererInServer->audioServerBuffer_->basicBufferInfo_->curReadFrame.store(5);
    rendererInServer->audioServerBuffer_->basicBufferInfo_->basePosInFrame.store(5);

    ret = rendererInServer->Flush();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Flush API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerFlush_008
 * @tc.desc  : Test Flush when audioServerBuffer_ GetReadbuffer failed
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerFlush_008, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    rendererInServer->OnStatusUpdate(OPERATION_STARTED);
    rendererInServer->audioServerBuffer_->basicBufferInfo_->curWriteFrame.store(10);
    rendererInServer->audioServerBuffer_->basicBufferInfo_->curReadFrame.store(5);
    rendererInServer->audioServerBuffer_->basicBufferInfo_->basePosInFrame.store(10);

    ret = rendererInServer->Flush();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test DrainAudioBuffer API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerDrainAudioBuffer_001
 * @tc.desc  : Test normal DrainAudioBuffer.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerDrainAudioBuffer_001, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->DrainAudioBuffer();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Drain API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerDrain_001
 * @tc.desc  : Test Drain when status_ illegal.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerDrain_001, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    rendererInServer->OnStatusUpdate(OPERATION_RELEASED);

    bool stopFlag = true;
    ret = rendererInServer->Drain(stopFlag);
    EXPECT_EQ(ERR_ILLEGAL_STATE, ret);
}

/**
 * @tc.name  : Test Drain API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerDrain_002
 * @tc.desc  : Test Drain when stopFlag and isInnerCapEnabled_ and isDualToneEnabled_ are false.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerDrain_002, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    rendererInServer->OnStatusUpdate(OPERATION_STARTED);

    bool stopFlag = false;
    ret = rendererInServer->Drain(stopFlag);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Drain API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerDrain_003
 * @tc.desc  : Test Drain when stopFlag and isInnerCapEnabled_ and isDualToneEnabled_ are true.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerDrain_003, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    ret = rendererInServer->InitDupStream(1);
    ret = rendererInServer->EnableDualTone("Speaker");
    rendererInServer->OnStatusUpdate(OPERATION_STARTED);

    bool stopFlag = true;
    ret = rendererInServer->Drain(stopFlag);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Drain API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerDrain_004
 * @tc.desc  : Test Drain when stopFlag and isInnerCapEnabled_ and isDualToneEnabled_ are true but dupStream_ and
 * dualToneStream_ are nullptr.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerDrain_004, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    ret = rendererInServer->InitDupStream(1);
    ret = rendererInServer->EnableDualTone("Speaker");
    if (rendererInServer->captureInfos_.count(1)) {
        rendererInServer->captureInfos_[1].dupStream = nullptr;
        rendererInServer->captureInfos_.erase(1);
    }
    rendererInServer->dualToneStream_ = nullptr;
    rendererInServer->OnStatusUpdate(OPERATION_STARTED);
    bool stopFlag = true;

    ret = rendererInServer->Drain(stopFlag);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test OnStatusUpdateExt API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerOnStatusUpdateExt_001
 * @tc.desc  : Test Init API when status is not I_STATUS_DRAINING.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerOnStatusUpdateExt_001, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);
    rendererInServer->status_ = I_STATUS_STARTED;
    rendererInServer->OnStatusUpdateExt(OPERATION_STARTED, stateListener);
    EXPECT_TRUE(rendererInServer->afterDrain);
}

/**
 * @tc.name  : Test Stop API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerStop_001
 * @tc.desc  : Test Stop interface.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerStop_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Stop();

    EXPECT_EQ(ERR_ILLEGAL_STATE, ret);
}

/**
 * @tc.name  : Test Stop API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerStop_002
 * @tc.desc  : Test Stop interface. Set status_ is I_STATUS_STARTED.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerStop_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->status_ = I_STATUS_STARTED;
    rendererInServer->Init();
    rendererInServer->standByEnable_ = true;
    auto &info = rendererInServer->captureInfos_[1];
    info.isInnerCapEnabled = true;
    rendererInServer->isDualToneEnabled_ = true;
    int32_t ret = rendererInServer->Stop();

    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Stop API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerStop_003
 * @tc.desc  : Test Stop interface. Set status_ is I_STATUS_PAUSED.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerStop_003, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->status_ = I_STATUS_PAUSED;
    rendererInServer->Init();
    rendererInServer->standByEnable_ = true;
    rendererInServer->InitDupStream(1);
    rendererInServer->EnableDualTone("Speaker");
    int32_t ret = rendererInServer->Stop();

    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Stop API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerStop_004
 * @tc.desc  : Test Stop interface. Set status_ is I_STATUS_DRAINING.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerStop_004, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->status_ = I_STATUS_DRAINING;
    rendererInServer->Init();
    rendererInServer->standByEnable_ = true;
    auto &info = rendererInServer->captureInfos_[1];
    info.isInnerCapEnabled = true;
    rendererInServer->isDualToneEnabled_ = true;
    int32_t ret = rendererInServer->Stop();

    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Stop API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerStop_005
 * @tc.desc  : Test Stop interface. Set status_ is I_STATUS_STARTING.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerStop_005, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->status_ = I_STATUS_STARTING;
    rendererInServer->Init();
    rendererInServer->standByEnable_ = true;
    auto &info = rendererInServer->captureInfos_[1];
    info.isInnerCapEnabled = true;
    rendererInServer->isDualToneEnabled_ = true;
    int32_t ret = rendererInServer->Stop();

    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Stop API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerStop_006
 * @tc.desc  : Test Stop interface. standByEnable_, isInnerCapEnabled_ and isInnerCapEnabled_ is false.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerStop_006, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->status_ = I_STATUS_STARTING;
    rendererInServer->Init();
    rendererInServer->standByEnable_ = false;
    auto &info = rendererInServer->captureInfos_[1];
    info.isInnerCapEnabled = false;
    rendererInServer->isDualToneEnabled_ = false;
    int32_t ret = rendererInServer->Stop();

    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Release API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerRelease_001
 * @tc.desc  : Test Release interface, Set status_ is I_STATUS_RELEASED.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerRelease_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->status_ = I_STATUS_RELEASED;
    int32_t ret = rendererInServer->Release();

    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Release API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerRelease_003
 * @tc.desc  : Test Release interface, without init.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerRelease_003, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    auto &info = rendererInServer->captureInfos_[1];
    info.isInnerCapEnabled = true;
    rendererInServer->isDualToneEnabled_ = true;
    int32_t ret = rendererInServer->Release();

    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test Release API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerRelease_004
 * @tc.desc  : Test Release interface, without init.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerRelease_004, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Release();

    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test GetAudioTime API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerGetAudioTime_001
 * @tc.desc  : Test GetAudioTime interface, status_ is not I_STATUS_STOPPED.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerGetAudioTime_001, TestSize.Level1)
{
    processConfig.rendererInfo.rendererFlags = AUDIO_FLAG_NORMAL;
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->Init();
    rendererInServer->Start();
    uint64_t framePos = TEST_FRAMEPOS;
    uint64_t timestamp = TEST_TIMESTAMP;
    int32_t ret = rendererInServer->GetAudioTime(framePos, timestamp);

    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test GetAudioTime API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerGetAudioTime_002
 * @tc.desc  : Test GetAudioTime interface, status_ is I_STATUS_STOPPED.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerGetAudioTime_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->status_ = I_STATUS_STOPPED;
    uint64_t framePos = TEST_FRAMEPOS;
    uint64_t timestamp = TEST_TIMESTAMP;
    int32_t ret = rendererInServer->GetAudioTime(framePos, timestamp);

    EXPECT_EQ(ERR_ILLEGAL_STATE, ret);
}

/**
 * @tc.name  : Test GetAudioTime API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerGetAudioTime_003
 * @tc.desc  : Test GetAudioTime interface, status_ is not I_STATUS_STOPPED, resetTime_ is false.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerGetAudioTime_003, TestSize.Level1)
{
    processConfig.rendererInfo.rendererFlags = AUDIO_FLAG_NORMAL;
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->Init();
    rendererInServer->Start();
    rendererInServer->resetTime_ = false;
    uint64_t framePos = TEST_FRAMEPOS;
    uint64_t timestamp = TEST_TIMESTAMP;
    int32_t ret = rendererInServer->GetAudioTime(framePos, timestamp);

    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test GetAudioPosition API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerGetAudioPosition_001
 * @tc.desc  : Test GetAudioPosition interface, status_ is not I_STATUS_STOPPED.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerGetAudioPosition_001, TestSize.Level1)
{
    processConfig.deviceType = DEVICE_TYPE_INVALID;
    processConfig.rendererInfo.rendererFlags = AUDIO_FLAG_MMAP;
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->managerType_ = DIRECT_PLAYBACK;
    rendererInServer->Init();
    uint64_t framePos = TEST_FRAMEPOS;
    uint64_t timestamp = TEST_TIMESTAMP;
    uint64_t latency = 0;
    int32_t ret = rendererInServer->GetAudioPosition(framePos, timestamp, latency, Timestamp::MONOTONIC);

    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test GetAudioPosition API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerGetAudioPosition_002
 * @tc.desc  : Test GetAudioPosition interface, status_ is I_STATUS_STOPPED.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerGetAudioPosition_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->status_ = I_STATUS_STOPPED;
    uint64_t framePos = TEST_FRAMEPOS;
    uint64_t timestamp = TEST_TIMESTAMP;
    uint64_t latency = 0;
    int32_t ret = rendererInServer->GetAudioPosition(framePos, timestamp, latency, Timestamp::MONOTONIC);

    EXPECT_EQ(ERR_ILLEGAL_STATE, ret);
}

/**
 * @tc.name  : Test GetLatency API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerGetLatency_001
 * @tc.desc  : Test GetLatency interface.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerGetLatency_001, TestSize.Level1)
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
 * @tc.name  : Test SetRate API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetRate_001
 * @tc.desc  : Test SetRate interface.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerSetRate_001, TestSize.Level1)
{
    processConfig.deviceType = DEVICE_TYPE_INVALID;
    processConfig.rendererInfo.rendererFlags = AUDIO_FLAG_MMAP;
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->managerType_ = DIRECT_PLAYBACK;
    rendererInServer->Init();
    int32_t rate = TEST_RATE;
    int32_t ret = rendererInServer->SetRate(rate);

    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetLowPowerVolume API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetLowPowerVolume_001
 * @tc.desc  : Test SetLowPowerVolume interface, Set volume in the range.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerSetLowPowerVolume_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    float volume = IN_VOLUME_RANGE;
    int32_t ret = rendererInServer->SetLowPowerVolume(volume);

    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetLowPowerVolume API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetLowPowerVolume_002
 * @tc.desc  : Test SetLowPowerVolume interface, Set volume out of the range.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerSetLowPowerVolume_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    float volume = OUT_OF_MAX_FLOAT_VOLUME;
    int32_t ret = rendererInServer->SetLowPowerVolume(volume);

    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}

/**
 * @tc.name  : Test SetLowPowerVolume API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetLowPowerVolume_003
 * @tc.desc  : Test SetLowPowerVolume interface, Set volume is OUT_OF_MIN_FLOAT_VOLUME.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerSetLowPowerVolume_003, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    float volume = OUT_OF_MIN_FLOAT_VOLUME;
    int32_t ret = rendererInServer->SetLowPowerVolume(volume);

    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}

/**
 * @tc.name  : Test GetLowPowerVolume API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerGetLowPowerVolume_001
 * @tc.desc  : Test GetLowPowerVolume interface.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerGetLowPowerVolume_001, TestSize.Level1)
{
    processConfig.deviceType = DEVICE_TYPE_INVALID;
    processConfig.rendererInfo.rendererFlags = AUDIO_FLAG_MMAP;
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->managerType_ = DIRECT_PLAYBACK;
    rendererInServer->Init();
    float volume = IN_VOLUME_RANGE;
    int32_t ret = rendererInServer->GetLowPowerVolume(volume);

    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetAudioEffectMode API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetAudioEffectMode_001
 * @tc.desc  : Test SetAudioEffectMode interface.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerSetAudioEffectMode_001, TestSize.Level1)
{
    processConfig.deviceType = DEVICE_TYPE_INVALID;
    processConfig.rendererInfo.rendererFlags = AUDIO_FLAG_MMAP;
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->managerType_ = DIRECT_PLAYBACK;
    rendererInServer->Init();
    int32_t effectMode = TEST_EFFECTMODE;
    int32_t ret = rendererInServer->SetAudioEffectMode(effectMode);

    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test GetAudioEffectMode API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerGetAudioEffectMode_001
 * @tc.desc  : Test GetAudioEffectMode interface.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerGetAudioEffectMode_001, TestSize.Level1)
{
    processConfig.deviceType = DEVICE_TYPE_INVALID;
    processConfig.rendererInfo.rendererFlags = AUDIO_FLAG_MMAP;
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->managerType_ = DIRECT_PLAYBACK;
    rendererInServer->Init();
    int32_t effectMode = TEST_EFFECTMODE;
    int32_t ret = rendererInServer->GetAudioEffectMode(effectMode);

    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetPrivacyType API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetPrivacyType_001
 * @tc.desc  : Test SetPrivacyType interface.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerSetPrivacyType_001, TestSize.Level1)
{
    processConfig.deviceType = DEVICE_TYPE_INVALID;
    processConfig.rendererInfo.rendererFlags = AUDIO_FLAG_MMAP;
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->managerType_ = DIRECT_PLAYBACK;
    rendererInServer->Init();
    int32_t privacyType = TEST_PRIVACYTYPE;
    int32_t ret = rendererInServer->SetPrivacyType(privacyType);

    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test GetPrivacyType API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerGetPrivacyType_001
 * @tc.desc  : Test GetPrivacyType interface.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerGetPrivacyType_001, TestSize.Level1)
{
    processConfig.deviceType = DEVICE_TYPE_INVALID;
    processConfig.rendererInfo.rendererFlags = AUDIO_FLAG_MMAP;
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->managerType_ = DIRECT_PLAYBACK;
    rendererInServer->Init();
    int32_t privacyType = TEST_PRIVACYTYPE;
    int32_t ret = rendererInServer->GetPrivacyType(privacyType);

    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test EnableInnerCap API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerEnableInnerCap_001
 * @tc.desc  : Test EnableInnerCap interface, Set isInnerCapEnabled_ is false.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerEnableInnerCap_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->EnableInnerCap(1);

    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test EnableInnerCap API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerEnableInnerCap_002
 * @tc.desc  : Test EnableInnerCap interface, Set isInnerCapEnabled_ is true.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerEnableInnerCap_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->InitDupStream(1);
    int32_t ret = rendererInServer->EnableInnerCap(1);

    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test DisableInnerCap API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerDisableInnerCap_001
 * @tc.desc  : Test DisableInnerCap interface, Set isInnerCapEnabled_ is false.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerDisableInnerCap_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->DisableInnerCap(0);

    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test DisableInnerCap API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerDisableInnerCap_002
 * @tc.desc  : Test DisableInnerCap interface, Set isInnerCapEnabled_ is true.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerDisableInnerCap_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->InitDupStream(1);
    int32_t ret = rendererInServer->DisableInnerCap(1);

    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test InitDupStream API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerInitDupStream_001
 * @tc.desc  : Test InitDupStream interface, Set dupStream_ is nullptr.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerInitDupStream_001, TestSize.Level1)
{
    AudioProcessConfig tempProcessConfig;
    tempProcessConfig.streamInfo = testStreamInfo;
    tempProcessConfig.streamType = STREAM_MUSIC;
    tempProcessConfig.rendererInfo.pipeType = PIPE_TYPE_DIRECT_MUSIC;
    tempProcessConfig.deviceType = DEVICE_TYPE_INVALID;
    tempProcessConfig.rendererInfo.rendererFlags = AUDIO_FLAG_MMAP;
    std::shared_ptr<RendererInServer> tempRendererInServer;
    tempRendererInServer = std::make_shared<RendererInServer>(tempProcessConfig, streamListener);
    EXPECT_NE(nullptr, tempRendererInServer);

    tempRendererInServer->dualToneStream_ = nullptr;
    int32_t ret = tempRendererInServer->InitDupStream(1);

    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test InitDupStream API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerInitDupStream_002
 * @tc.desc  : Test InitDupStream interface.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerInitDupStream_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->status_ = I_STATUS_STARTED;
    int32_t ret = rendererInServer->InitDupStream(1);

    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test EnableDualTone API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerEnableDualTone_001
 * @tc.desc  : Test EnableDualTone interface, Set isDualToneEnabled_ is false.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerEnableDualTone_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->EnableDualTone("Speaker");

    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test EnableDualTone API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerEnableDualTone_002
 * @tc.desc  : Test EnableDualTone interface, Set isDualToneEnabled_ is true.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerEnableDualTone_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->EnableDualTone("Speaker");
    int32_t ret = rendererInServer->EnableDualTone("Speaker");

    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test DisableDualTone API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerDisableDualTone_001
 * @tc.desc  : Test DisableDualTone interface, Set isDualToneEnabled_ is false.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerDisableDualTone_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->DisableDualTone();

    EXPECT_EQ(ERR_INVALID_OPERATION, ret);
}

/**
 * @tc.name  : Test DisableDualTone API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerDisableDualTone_002
 * @tc.desc  : Test DisableDualTone interface, Set isDualToneEnabled_ is true.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerDisableDualTone_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->EnableDualTone("Speaker");
    int32_t ret = rendererInServer->DisableDualTone();

    EXPECT_EQ(ERROR, ret);
}

/**
 * @tc.name  : Test EnableDualTone API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerEnableDualTone_003
 * @tc.desc  : Test EnableDualTone interface, Set dualToneStream_ is nullptr.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerEnableDualTone_003, TestSize.Level1)
{
    AudioProcessConfig tempProcessConfig;
    tempProcessConfig.streamInfo = testStreamInfo;
    tempProcessConfig.streamType = STREAM_MUSIC;
    tempProcessConfig.rendererInfo.pipeType = PIPE_TYPE_DIRECT_MUSIC;
    tempProcessConfig.deviceType = DEVICE_TYPE_INVALID;
    tempProcessConfig.rendererInfo.rendererFlags = AUDIO_FLAG_MMAP;
    std::shared_ptr<RendererInServer> tempRendererInServer;
    tempRendererInServer = std::make_shared<RendererInServer>(tempProcessConfig, streamListener);
    EXPECT_NE(nullptr, tempRendererInServer);

    tempRendererInServer->dualToneStream_ = nullptr;
    int32_t ret = tempRendererInServer->EnableDualTone("Speaker");

    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test EnableDualTone API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerEnableDualTone_004
 * @tc.desc  : Test EnableDualTone interface.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerEnableDualTone_004, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->status_ = I_STATUS_STARTED;
    rendererInServer->Init();
    int32_t ret = rendererInServer->EnableDualTone("Speaker");

    EXPECT_EQ(SUCCESS, ret);

    ret = rendererInServer->EnableDualTone("Test");
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test EnableDualTone API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerEnableDualTone_005
 * @tc.desc  : Test EnableDualTone interface.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerEnableDualTone_005, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->status_ = I_STATUS_STARTED;
    rendererInServer->Init();
    rendererInServer->offloadEnable_ = true;
    int32_t ret = rendererInServer->EnableDualTone("Speaker");

    EXPECT_EQ(SUCCESS, ret);

    ret = rendererInServer->EnableDualTone("Test");
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test OnWriteData API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerOnWriteData_001
 * @tc.desc  : Test OnWriteData interface.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerOnWriteData_001, TestSize.Level1)
{
    std::shared_ptr<StreamCallbacks> streamCallbacks;
    streamCallbacks = std::make_shared<StreamCallbacks>(TEST_STREAMINDEX);
    EXPECT_NE(nullptr, streamCallbacks);

    int32_t ret = streamCallbacks->OnWriteData(TEST_LENGTH);

    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetOffloadMode API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetOffloadMode_001
 * @tc.desc  : Test SetOffloadMode interface.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerSetOffloadMode_001, TestSize.Level1)
{
    AudioProcessConfig tempProcessConfig;
    std::shared_ptr<RendererInServer> tempRendererInServer;

    tempProcessConfig.streamInfo = testStreamInfo;
    tempProcessConfig.streamType = STREAM_MUSIC;
    tempProcessConfig.rendererInfo.pipeType = PIPE_TYPE_DIRECT_MUSIC;
    tempProcessConfig.deviceType = DEVICE_TYPE_INVALID;
    tempProcessConfig.rendererInfo.rendererFlags = AUDIO_FLAG_MMAP;
    tempRendererInServer = std::make_shared<RendererInServer>(tempProcessConfig, streamListener);
    EXPECT_NE(nullptr, tempRendererInServer);

    tempRendererInServer->managerType_ = DIRECT_PLAYBACK;
    tempRendererInServer->Init();
    int32_t ret = tempRendererInServer->SetOffloadMode(TEST_STATE, TEST_ISAPPBACK);

    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetOffloadMode API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetOffloadMode_002
 * @tc.desc  : Test SetOffloadMode interface, dupStream_ and dualToneStream_ is not nullptr.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerSetOffloadMode_002, TestSize.Level1)
{
    AudioProcessConfig tempProcessConfig;
    std::shared_ptr<RendererInServer> tempRendererInServer;

    tempProcessConfig.streamInfo = testStreamInfo;
    tempProcessConfig.streamType = STREAM_MUSIC;
    tempProcessConfig.rendererInfo.pipeType = PIPE_TYPE_DIRECT_MUSIC;
    tempProcessConfig.deviceType = DEVICE_TYPE_INVALID;
    tempProcessConfig.rendererInfo.rendererFlags = AUDIO_FLAG_MMAP;
    tempRendererInServer = std::make_shared<RendererInServer>(tempProcessConfig, streamListener);
    EXPECT_NE(nullptr, tempRendererInServer);

    tempRendererInServer->managerType_ = DIRECT_PLAYBACK;
    tempRendererInServer->Init();
    tempRendererInServer->InitDupStream(1);
    tempRendererInServer->EnableDualTone("Speaker");
    int32_t ret = tempRendererInServer->SetOffloadMode(TEST_STATE, TEST_ISAPPBACK);

    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetOffloadMode API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetOffloadMode_003
 * @tc.desc  : Test SetOffloadMode interface, dupStream_ and dualToneStream_ is nullptr.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerSetOffloadMode_003, TestSize.Level1)
{
    AudioProcessConfig tempProcessConfig;
    std::shared_ptr<RendererInServer> tempRendererInServer;

    tempProcessConfig.streamInfo = testStreamInfo;
    tempProcessConfig.streamType = STREAM_MUSIC;
    tempProcessConfig.rendererInfo.pipeType = PIPE_TYPE_DIRECT_MUSIC;
    tempProcessConfig.deviceType = DEVICE_TYPE_INVALID;
    tempProcessConfig.rendererInfo.rendererFlags = AUDIO_FLAG_MMAP;
    tempRendererInServer = std::make_shared<RendererInServer>(tempProcessConfig, streamListener);
    EXPECT_NE(nullptr, tempRendererInServer);

    tempRendererInServer->managerType_ = DIRECT_PLAYBACK;
    tempRendererInServer->Init();
    tempRendererInServer->InitDupStream(1);
    tempRendererInServer->EnableDualTone("Speaker");
    if (rendererInServer->captureInfos_.count(1)) {
        rendererInServer->captureInfos_[1].dupStream = nullptr;
        rendererInServer->captureInfos_.erase(1);
    }
    rendererInServer->dualToneStream_ = nullptr;
    int32_t ret = tempRendererInServer->SetOffloadMode(TEST_STATE, TEST_ISAPPBACK);

    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test UnsetOffloadMode API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerUnsetOffloadMode_001
 * @tc.desc  : Test UnsetOffloadMode interface.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerUnsetOffloadMode_001, TestSize.Level1)
{
    AudioProcessConfig tempProcessConfig;
    std::shared_ptr<RendererInServer> tempRendererInServer;

    tempProcessConfig.streamInfo = testStreamInfo;
    tempProcessConfig.streamType = STREAM_MUSIC;
    tempProcessConfig.rendererInfo.pipeType = PIPE_TYPE_DIRECT_MUSIC;
    tempProcessConfig.deviceType = DEVICE_TYPE_INVALID;
    tempProcessConfig.rendererInfo.rendererFlags = AUDIO_FLAG_MMAP;
    tempRendererInServer = std::make_shared<RendererInServer>(tempProcessConfig, streamListener);
    EXPECT_NE(nullptr, tempRendererInServer);

    tempRendererInServer->managerType_ = DIRECT_PLAYBACK;
    tempRendererInServer->Init();
    int32_t ret = tempRendererInServer->UnsetOffloadMode();

    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test UnsetOffloadMode API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerUnsetOffloadMode_002
 * @tc.desc  : Test UnsetOffloadMode interface, dupStream_ and dualToneStream_ is not nullptr.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerUnsetOffloadMode_002, TestSize.Level1)
{
    AudioProcessConfig tempProcessConfig;
    std::shared_ptr<RendererInServer> tempRendererInServer;

    tempProcessConfig.streamInfo = testStreamInfo;
    tempProcessConfig.streamType = STREAM_MUSIC;
    tempProcessConfig.rendererInfo.pipeType = PIPE_TYPE_DIRECT_MUSIC;
    tempProcessConfig.deviceType = DEVICE_TYPE_INVALID;
    tempProcessConfig.rendererInfo.rendererFlags = AUDIO_FLAG_MMAP;
    tempRendererInServer = std::make_shared<RendererInServer>(tempProcessConfig, streamListener);
    EXPECT_NE(nullptr, tempRendererInServer);

    tempRendererInServer->managerType_ = DIRECT_PLAYBACK;
    tempRendererInServer->Init();
    tempRendererInServer->InitDupStream(1);
    tempRendererInServer->EnableDualTone("Speaker");
    int32_t ret = tempRendererInServer->UnsetOffloadMode();

    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test UnsetOffloadMode API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerUnsetOffloadMode_003
 * @tc.desc  : Test UnsetOffloadMode interface, dupStream_ and dualToneStream_ is nullptr.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerUnsetOffloadMode_003, TestSize.Level1)
{
    AudioProcessConfig tempProcessConfig;
    std::shared_ptr<RendererInServer> tempRendererInServer;

    tempProcessConfig.streamInfo = testStreamInfo;
    tempProcessConfig.streamType = STREAM_MUSIC;
    tempProcessConfig.rendererInfo.pipeType = PIPE_TYPE_DIRECT_MUSIC;
    tempProcessConfig.deviceType = DEVICE_TYPE_INVALID;
    tempProcessConfig.rendererInfo.rendererFlags = AUDIO_FLAG_MMAP;
    tempRendererInServer = std::make_shared<RendererInServer>(tempProcessConfig, streamListener);
    EXPECT_NE(nullptr, tempRendererInServer);

    tempRendererInServer->managerType_ = DIRECT_PLAYBACK;
    tempRendererInServer->Init();
    tempRendererInServer->InitDupStream(1);
    tempRendererInServer->EnableDualTone("Speaker");
    if (rendererInServer->captureInfos_.count(1)) {
        rendererInServer->captureInfos_[1].dupStream = nullptr;
        rendererInServer->captureInfos_.erase(1);
    }
    rendererInServer->dualToneStream_ = nullptr;
    int32_t ret = tempRendererInServer->UnsetOffloadMode();

    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test UpdateSpatializationState API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerUpdateSpatializationState_001
 * @tc.desc  : Test UpdateSpatializationState interface, Set managerType_ is DIRECT_PLAYBACK.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerUpdateSpatializationState_001, TestSize.Level1)
{
    AudioProcessConfig tempProcessConfig;
    std::shared_ptr<RendererInServer> tempRendererInServer;

    tempProcessConfig.streamInfo = testStreamInfo;
    tempProcessConfig.streamType = STREAM_MUSIC;
    tempProcessConfig.rendererInfo.pipeType = PIPE_TYPE_DIRECT_MUSIC;
    tempProcessConfig.deviceType = DEVICE_TYPE_INVALID;
    tempProcessConfig.rendererInfo.rendererFlags = AUDIO_FLAG_MMAP;
    tempRendererInServer = std::make_shared<RendererInServer>(tempProcessConfig, streamListener);
    EXPECT_NE(nullptr, tempRendererInServer);

    tempRendererInServer->managerType_ = DIRECT_PLAYBACK;
    tempRendererInServer->Init();
    bool spatializationEnabled = true;
    bool headTrackingEnabled = true;
    int32_t ret = tempRendererInServer->UpdateSpatializationState(spatializationEnabled, headTrackingEnabled);

    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test GetStreamManagerType API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerGetStreamManagerType_001
 * @tc.desc  : Test GetStreamManagerType interface, Set managerType_ is DIRECT_PLAYBACK.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerGetStreamManagerType_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->managerType_ = DIRECT_PLAYBACK;
    int32_t ret = rendererInServer->GetStreamManagerType();

    EXPECT_EQ(AUDIO_DIRECT_MANAGER_TYPE, ret);
}

/**
 * @tc.name  : Test GetStreamManagerType API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerGetStreamManagerType_002
 * @tc.desc  : Test GetStreamManagerType interface, Set managerType_ is VOIP_PLAYBACK.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerGetStreamManagerType_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->managerType_ = VOIP_PLAYBACK;
    int32_t ret = rendererInServer->GetStreamManagerType();

    EXPECT_EQ(AUDIO_NORMAL_MANAGER_TYPE, ret);
}

/**
 * @tc.name  : Test SetClientVolume API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetClientVolume_001
 * @tc.desc  : Test SetClientVolume interface, Set audioServerBuffer_ is nullptr.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerSetClientVolume_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->SetClientVolume();

    EXPECT_EQ(ERROR, ret);
}

/**
 * @tc.name  : Test SetClientVolume API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetClientVolume_002
 * @tc.desc  : Test SetClientVolume interface. Set audioServerBuffer_ is not nullptr.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerSetClientVolume_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->managerType_ = DIRECT_PLAYBACK;
    rendererInServer->Init();

    int32_t ret = rendererInServer->SetClientVolume();

    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetMute API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetMute_001
 * @tc.desc  : Test SetMute interface.
*/
HWTEST_F(RendererInServerUnitTest, RendererInServerSetMute_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    bool mute = true;
    int32_t ret = rendererInServer->SetMute(mute);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetDuckFactor API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetDuckFactor_001
 * @tc.desc  : Test SetDuckFactor interface.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerSetDuckFactor_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    float duck = 0.2f;
    int32_t ret = rendererInServer->SetDuckFactor(duck);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetSilentModeAndMixWithOthers API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerSetSilentModeAndMixWithOthers_001
 * @tc.desc  : Test SetSilentModeAndMixWithOthers interface.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerSetSilentModeAndMixWithOthers_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    bool on = true;
    int32_t ret = rendererInServer->SetSilentModeAndMixWithOthers(on);

    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test OnDataLinkConnectionUpdate API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerOnDataLinkConnectionUpdate_001
 * @tc.desc  : Test OnDataLinkConnectionUpdate interface. Set operation is OPERATION_DATA_LINK_CONNECTING.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerOnDataLinkConnectionUpdate_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);
    IOperation operation = OPERATION_DATA_LINK_CONNECTING;
    rendererInServer->OnDataLinkConnectionUpdate(operation);
}

/**
 * @tc.name  : Test OnDataLinkConnectionUpdate API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerOnDataLinkConnectionUpdate_002
 * @tc.desc  : Test OnDataLinkConnectionUpdate interface. Set operation is OPERATION_DATA_LINK_CONNECTED.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerOnDataLinkConnectionUpdate_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);
    IOperation operation = OPERATION_DATA_LINK_CONNECTED;
    rendererInServer->OnDataLinkConnectionUpdate(operation);
}

/**
 * @tc.name  : Test OnDataLinkConnectionUpdate API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerOnDataLinkConnectionUpdate_003
 * @tc.desc  : Test OnDataLinkConnectionUpdate interface. Set operation is OPERATION_UNSET_OFFLOAD_ENABLE.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerOnDataLinkConnectionUpdate_003, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);
    IOperation operation = OPERATION_UNSET_OFFLOAD_ENABLE;
    rendererInServer->OnDataLinkConnectionUpdate(operation);
}

/**
 * @tc.name  : Test Dump API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerDump_001
 * @tc.desc  : Test Dump interface. Set dumpString is "", managerType_ is PLAYBACK.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerDump_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    std::string dump="";
    rendererInServer->managerType_ = PLAYBACK;

    EXPECT_TRUE(rendererInServer->Dump(dump));
}

/**
 * @tc.name  : Test Dump API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerDump_002
 * @tc.desc  : Test Dump interface. Set dumpString is "", managerType_ is DIRECT_PLAYBACK, status_ is I_STATUS_INVALID.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerDump_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    std::string dump="";
    rendererInServer->managerType_ = DIRECT_PLAYBACK;
    rendererInServer->status_ = I_STATUS_INVALID;

    EXPECT_TRUE(rendererInServer->Dump(dump));
}

/**
 * @tc.name  : Test Dump API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerDump_003
 * @tc.desc  : Test Dump interface. Set dumpString is "", managerType_ is DIRECT_PLAYBACK, status_ is I_STATUS_IDLE.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerDump_003, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    std::string dump="";
    rendererInServer->managerType_ = DIRECT_PLAYBACK;
    rendererInServer->status_ = I_STATUS_IDLE;

    EXPECT_TRUE(rendererInServer->Dump(dump));
}

/**
 * @tc.name  : Test Dump API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerDump_004
 * @tc.desc  : Test Dump interface. Set dumpString is "", managerType_ is DIRECT_PLAYBACK, status_ is I_STATUS_STARTING.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerDump_004, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    std::string dump="";
    rendererInServer->managerType_ = DIRECT_PLAYBACK;
    rendererInServer->status_ = I_STATUS_STARTING;

    EXPECT_TRUE(rendererInServer->Dump(dump));
}

/**
 * @tc.name  : Test Dump API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerDump_005
 * @tc.desc  : Test Dump interface. Set dumpString is "", managerType_ is DIRECT_PLAYBACK, status_ is I_STATUS_STARTED.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerDump_005, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    std::string dump="";
    rendererInServer->managerType_ = DIRECT_PLAYBACK;
    rendererInServer->status_ = I_STATUS_STARTED;

    EXPECT_TRUE(rendererInServer->Dump(dump));
}

/**
 * @tc.name  : Test Dump API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerDump_006
 * @tc.desc  : Test Dump interface. Set dumpString is "", managerType_ is DIRECT_PLAYBACK, status_ is I_STATUS_PAUSING.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerDump_006, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    std::string dump="";
    rendererInServer->managerType_ = DIRECT_PLAYBACK;
    rendererInServer->status_ = I_STATUS_PAUSING;

    EXPECT_TRUE(rendererInServer->Dump(dump));
}

/**
 * @tc.name  : Test Dump API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerDump_007
 * @tc.desc  : Test Dump interface. Set dumpString is "", managerType_ is DIRECT_PLAYBACK, status_ is I_STATUS_PAUSED.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerDump_007, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    std::string dump="";
    rendererInServer->managerType_ = DIRECT_PLAYBACK;
    rendererInServer->status_ = I_STATUS_PAUSED;

    EXPECT_TRUE(rendererInServer->Dump(dump));
}

/**
 * @tc.name  : Test Dump API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerDump_008
 * @tc.desc  : Test Dump interface. Set dumpString is "", managerType_ is DIRECT_PLAYBACK,
 *             status_ is I_STATUS_FLUSHING_WHEN_STARTED.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerDump_008, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    std::string dump="";
    rendererInServer->managerType_ = DIRECT_PLAYBACK;
    rendererInServer->status_ = I_STATUS_FLUSHING_WHEN_STARTED;

    EXPECT_TRUE(rendererInServer->Dump(dump));
}

/**
 * @tc.name  : Test Dump API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerDump_009
 * @tc.desc  : Test Dump interface. Set dumpString is "", managerType_ is DIRECT_PLAYBACK,
 *             status_ is I_STATUS_FLUSHING_WHEN_PAUSED.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerDump_009, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    std::string dump="";
    rendererInServer->managerType_ = DIRECT_PLAYBACK;
    rendererInServer->status_ = I_STATUS_FLUSHING_WHEN_PAUSED;

    EXPECT_TRUE(rendererInServer->Dump(dump));
}

/**
 * @tc.name  : Test Dump API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerDump_010
 * @tc.desc  : Test Dump interface. Set dumpString is "", managerType_ is VOIP_PLAYBACK,
 *             status_ is I_STATUS_FLUSHING_WHEN_STOPPED.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerDump_010, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    std::string dump="";
    rendererInServer->managerType_ = VOIP_PLAYBACK;
    rendererInServer->status_ = I_STATUS_FLUSHING_WHEN_STOPPED;

    EXPECT_TRUE(rendererInServer->Dump(dump));
}

/**
 * @tc.name  : Test Dump API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerDump_011
 * @tc.desc  : Test Dump interface. Set dumpString is "", managerType_ is VOIP_PLAYBACK, status_ is I_STATUS_DRAINING.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerDump_011, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    std::string dump="";
    rendererInServer->managerType_ = VOIP_PLAYBACK;
    rendererInServer->status_ = I_STATUS_DRAINING;

    EXPECT_TRUE(rendererInServer->Dump(dump));
}

/**
 * @tc.name  : Test Dump API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerDump_012
 * @tc.desc  : Test Dump interface. Set dumpString is "", managerType_ is VOIP_PLAYBACK, status_ is I_STATUS_DRAINED.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerDump_012, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    std::string dump="";
    rendererInServer->managerType_ = VOIP_PLAYBACK;
    rendererInServer->status_ = I_STATUS_DRAINED;

    EXPECT_TRUE(rendererInServer->Dump(dump));
}

/**
 * @tc.name  : Test Dump API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerDump_013
 * @tc.desc  : Test Dump interface. Set dumpString is "", managerType_ is VOIP_PLAYBACK, status_ is I_STATUS_STOPPING.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerDump_013, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    std::string dump="";
    rendererInServer->managerType_ = VOIP_PLAYBACK;
    rendererInServer->status_ = I_STATUS_STOPPING;

    EXPECT_TRUE(rendererInServer->Dump(dump));
}

/**
 * @tc.name  : Test Dump API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerDump_014
 * @tc.desc  : Test Dump interface. Set dumpString is "", managerType_ is VOIP_PLAYBACK, status_ is I_STATUS_STOPPED.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerDump_014, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    std::string dump="";
    rendererInServer->managerType_ = VOIP_PLAYBACK;
    rendererInServer->status_ = I_STATUS_STOPPED;

    EXPECT_TRUE(rendererInServer->Dump(dump));
}

/**
 * @tc.name  : Test Dump API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerDump_015
 * @tc.desc  : Test Dump interface. Set dumpString is "", managerType_ is VOIP_PLAYBACK, status_ is I_STATUS_RELEASING.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerDump_015, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    std::string dump="";
    rendererInServer->managerType_ = VOIP_PLAYBACK;
    rendererInServer->status_ = I_STATUS_RELEASING;

    EXPECT_TRUE(rendererInServer->Dump(dump));
}

/**
 * @tc.name  : Test Dump API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerDump_016
 * @tc.desc  : Test Dump interface. Set dumpString is "", managerType_ is VOIP_PLAYBACK, status_ is I_STATUS_RELEASED.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerDump_016, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    std::string dump="";
    rendererInServer->managerType_ = VOIP_PLAYBACK;
    rendererInServer->status_ = I_STATUS_RELEASED;

    EXPECT_TRUE(rendererInServer->Dump(dump));
}

/**
 * @tc.name  : Test Dump API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerDump_017
 * @tc.desc  : Test Dump interface. Set dumpString is "", managerType_ is VOIP_PLAYBACK, status_ is OUT_OF_STATUS.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerDump_017, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    std::string dump="";
    rendererInServer->managerType_ = VOIP_PLAYBACK;
    rendererInServer->status_ = static_cast<IStatus>(OUT_OF_STATUS);

    EXPECT_TRUE(rendererInServer->Dump(dump));
}

/**
 * @tc.name  : Test Dump API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerDump_018
 * @tc.desc  : Test Dump interface. audioServerBuffer_ is not nullptr.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerDump_018, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    std::string dump="";
    rendererInServer->Init();
    rendererInServer->managerType_ = VOIP_PLAYBACK;
    rendererInServer->status_ = I_STATUS_RELEASED;

    EXPECT_TRUE(rendererInServer->Dump(dump));
}

/**
 * @tc.name  : Test DumpNormal API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerDumpNormal_001
 * @tc.desc  : Test DumpNormal interface.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerDumpNormal_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, rendererInServer);

    std::string dump="";
    rendererInServer->Init();
    rendererInServer->managerType_ = VOIP_PLAYBACK;

    EXPECT_FALSE(rendererInServer->DumpNormal(dump));
}

/**
 * @tc.name  : Test StandByCheck API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerStandByCheck_001
 * @tc.desc  : Test StandByCheck API when managerType_ is PLAYBACK.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerStandByCheck_001, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_FLAG_VOIP_DIRECT);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    ret = rendererInServer->Init();
    EXPECT_EQ(PLAYBACK, rendererInServer->managerType_);
    EXPECT_EQ(SUCCESS, ret);

    rendererInServer->standByEnable_ = true;
    rendererInServer->StandByCheck();
    EXPECT_EQ(true, rendererInServer->standByEnable_);
    rendererInServer->standByEnable_ = false;
    rendererInServer->StandByCheck();
    rendererInServer->standByEnable_ = false;
    rendererInServer->standByCounter_ = 60;
    rendererInServer->lastWriteTime_ = 1000000000;
    rendererInServer->StandByCheck();
    rendererInServer->standByEnable_ = false;
    rendererInServer->managerType_ = DIRECT_PLAYBACK;
    rendererInServer->standByCounter_ = 0;
    rendererInServer->lastWriteTime_ = 0;
    rendererInServer->StandByCheck();

    rendererInServer->standByCounter_ = 400;
    rendererInServer->lastWriteTime_ = 8000000000;
    rendererInServer->offloadEnable_ = true;
    ret = rendererInServer->ShouldEnableStandBy();
    EXPECT_EQ(true, ret);

    rendererInServer->standByCounter_ = 10;
    rendererInServer->lastWriteTime_ = 8000000000;
    rendererInServer->offloadEnable_ = false;
    ret = rendererInServer->ShouldEnableStandBy();
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name  : Test UpdateWriteIndex API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerUpdateWriteIndex_001
 * @tc.desc  : Test UpdateWriteIndex API when managerType_ is not PLAYBACK.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerUpdateWriteIndex_001, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_FLAG_VOIP_DIRECT);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    ret = rendererInServer->Init();
    EXPECT_EQ(PLAYBACK, rendererInServer->managerType_);
    EXPECT_EQ(SUCCESS, ret);
    rendererInServer->managerType_ = DIRECT_PLAYBACK;
    rendererInServer->needForceWrite_ = 1;
    rendererInServer->afterDrain = true;
    ret = rendererInServer->UpdateWriteIndex();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test OnWriteData API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerOnWriteData_002
 * @tc.desc  : Test OnWriteData API when managerType_ is not PLAYBACK.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerOnWriteData_002, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_FLAG_VOIP_DIRECT);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    ret = rendererInServer->Init();
    EXPECT_EQ(PLAYBACK, rendererInServer->managerType_);
    EXPECT_EQ(SUCCESS, ret);
    ret = rendererInServer->OnWriteData(4);
    EXPECT_EQ(SUCCESS, ret);
}
/**
 * @tc.name  : Test OnWriteData API
 * @tc.type  : FUNC
 * @tc.number: RendererInServer_001
 * @tc.desc  : Test OnWriteData API when managerType_ is not PLAYBACK.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServer_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.callerUid = 1013;

    RendererInServer rendererInServer(processConfig, stateListener);
    int ret = rendererInServer.InitBufferStatus();
    ASSERT_EQ(ERR_ILLEGAL_STATE, ret);
}
/**
 * @tc.name  : Test OnWriteData API
 * @tc.type  : FUNC
 * @tc.number: RendererInServerStandByCheck_002
 * @tc.desc  : Test OnWriteData API when managerType_ is not PLAYBACK.
 */
HWTEST_F(RendererInServerUnitTest, RendererInServerStandByCheck_002, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_ULTRASONIC;

    RendererInServer rendererInServer(processConfig, stateListener);
    rendererInServer.standByEnable_ = true;
    rendererInServer.StandByCheck();
    EXPECT_EQ(true, rendererInServer.standByEnable_);
}
/**
 * @tc.name  : Test OnWriteData API
 * @tc.type  : FUNC
 * @tc.number: GetStandbyStatus_001
 * @tc.desc  : Test OnWriteData API when managerType_ is not PLAYBACK.
 */
HWTEST_F(RendererInServerUnitTest, GetStandbyStatus_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    RendererInServer rendererInServer(processConfig, stateListener);
    rendererInServer.standByEnable_ = true;
    bool isStandby = false;
    int64_t enterStandbyTime = 0;
    int ret = rendererInServer.GetStandbyStatus(isStandby, enterStandbyTime);
    EXPECT_EQ(ret, SUCCESS);
}
/**
 * @tc.name  : Test OnWriteData API
 * @tc.type  : FUNC
 * @tc.number: GetStandbyStatus_002
 * @tc.desc  : Test OnWriteData API when managerType_ is not PLAYBACK.
 */
HWTEST_F(RendererInServerUnitTest, GetStandbyStatus_002, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    RendererInServer rendererInServer(processConfig, stateListener);
    rendererInServer.standByEnable_ = false;
    bool isStandby = false;
    int64_t enterStandbyTime = 0;
    int ret = rendererInServer.GetStandbyStatus(isStandby, enterStandbyTime);
    EXPECT_EQ(ret, SUCCESS);
}
/**
 * @tc.name  : Test OnWriteData API
 * @tc.type  : FUNC
 * @tc.number: IsInvalidBuffer_001
 * @tc.desc  : Test OnWriteData API when managerType_ is not PLAYBACK.
 */
HWTEST_F(RendererInServerUnitTest, IsInvalidBuffer_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.streamInfo.format = SAMPLE_U8;
    RendererInServer rendererInServer(processConfig, stateListener);
    uint8_t *buffer = nullptr;
    size_t bufferSize = 0;
    int ret = rendererInServer.IsInvalidBuffer(buffer, bufferSize);
    EXPECT_EQ(ret, false);
}
/**
 * @tc.name  : Test OnWriteData API
 * @tc.type  : FUNC
 * @tc.number: IsInvalidBuffer_002
 * @tc.desc  : Test OnWriteData API when managerType_ is not PLAYBACK.
 */
HWTEST_F(RendererInServerUnitTest, IsInvalidBuffer_002, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.streamInfo.format = SAMPLE_S16LE;
    RendererInServer rendererInServer(processConfig, stateListener);
    uint8_t *buffer = nullptr;
    size_t bufferSize = 0;
    int ret = rendererInServer.IsInvalidBuffer(buffer, bufferSize);
    EXPECT_EQ(ret, false);
}
/**
 * @tc.name  : Test OnWriteData API
 * @tc.type  : FUNC
 * @tc.number: IsInvalidBuffer_003
 * @tc.desc  : Test OnWriteData API when managerType_ is not PLAYBACK.
 */
HWTEST_F(RendererInServerUnitTest, IsInvalidBuffer_003, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.streamInfo.format = SAMPLE_S24LE;
    RendererInServer rendererInServer(processConfig, stateListener);
    uint8_t *buffer = nullptr;
    size_t bufferSize = 0;
    int ret = rendererInServer.IsInvalidBuffer(buffer, bufferSize);
    EXPECT_EQ(ret, false);
}
/**
 * @tc.name  : Test OnWriteData API
 * @tc.type  : FUNC
 * @tc.number: WriteMuteDataSysEvent_001
 * @tc.desc  : Test OnWriteData API when managerType_ is not PLAYBACK.
 */
HWTEST_F(RendererInServerUnitTest, WriteMuteDataSysEvent_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.streamInfo.format = SAMPLE_U8;
    RendererInServer rendererInServer(processConfig, stateListener);
    rendererInServer.isInSilentState_ = 0;
    uint8_t buffer[10] = {0};
    size_t bufferSize = 10;
    bufferDesc.buffer = buffer;
    bufferDesc.bufLength = bufferSize;

    int ret = rendererInServer.IsInvalidBuffer(buffer, bufferSize);
    rendererInServer.WriteMuteDataSysEvent(bufferDesc);
    EXPECT_EQ(ret, true);
}
/**
 * @tc.name  : Test OnWriteData API
 * @tc.type  : FUNC
 * @tc.number: WriteMuteDataSysEvent_002
 * @tc.desc  : Test OnWriteData API when managerType_ is not PLAYBACK.
 */
HWTEST_F(RendererInServerUnitTest, WriteMuteDataSysEvent_002, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.streamInfo.format = SAMPLE_U8;
    RendererInServer rendererInServer(processConfig, stateListener);
    rendererInServer.startMuteTime_ = 1;
    rendererInServer.isInSilentState_ = 1;
    uint8_t buffer[10] = {0};
    size_t bufferSize = 10;
    bufferDesc.buffer = buffer;
    bufferDesc.bufLength = bufferSize;
    int ret = rendererInServer.IsInvalidBuffer(buffer, bufferSize);
    rendererInServer.WriteMuteDataSysEvent(bufferDesc);
    EXPECT_EQ(ret, true);
}
/**
 * @tc.name  : Test OnWriteData API
 * @tc.type  : FUNC
 * @tc.number: WriteMuteDataSysEvent_003
 * @tc.desc  : Test OnWriteData API when managerType_ is not PLAYBACK.
 */
HWTEST_F(RendererInServerUnitTest, WriteMuteDataSysEvent_003, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.streamInfo.format = SAMPLE_U8;
    RendererInServer rendererInServer(processConfig, stateListener);
    rendererInServer.startMuteTime_ = 1;
    rendererInServer.isInSilentState_ = 1;
    uint8_t buffer[10] = {0};
    size_t bufferSize = 10;
    bufferDesc.buffer = buffer;
    bufferDesc.bufLength = bufferSize;
    int ret = rendererInServer.IsInvalidBuffer(buffer, bufferSize);
    rendererInServer.WriteMuteDataSysEvent(bufferDesc);
    EXPECT_EQ(ret, true);
}
/**
 * @tc.name  : Test OnWriteData API
 * @tc.type  : FUNC
 * @tc.number: GetLatency_002
 * @tc.desc  : Test OnWriteData API when managerType_ is not PLAYBACK.
 */
HWTEST_F(RendererInServerUnitTest, GetLatency_002, TestSize.Level1)
{
    processConfig.deviceType = DEVICE_TYPE_INVALID;
    processConfig.rendererInfo.rendererFlags = AUDIO_FLAG_MMAP;
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);
    rendererInServer->managerType_ = PLAYBACK;
    rendererInServer->Init();
    uint64_t latency = TEST_LATENCY;
    int32_t ret = rendererInServer->GetLatency(latency);

    EXPECT_EQ(SUCCESS, ret);
}
/**
 * @tc.name  : Test OnWriteData API
 * @tc.type  : FUNC
 * @tc.number: SetAudioEffectMode_002
 * @tc.desc  : Test OnWriteData API when managerType_ is not PLAYBACK.
 */
HWTEST_F(RendererInServerUnitTest, SetAudioEffectMode_002, TestSize.Level1)
{
    processConfig.deviceType = DEVICE_TYPE_INVALID;
    processConfig.rendererInfo.rendererFlags = AUDIO_FLAG_MMAP;
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);
    rendererInServer->isDualToneEnabled_ = true;

    rendererInServer->managerType_ = DIRECT_PLAYBACK;
    rendererInServer->Init();
    int32_t effectMode = TEST_EFFECTMODE;
    int32_t ret = rendererInServer->SetAudioEffectMode(effectMode);

    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test RendererInServer
 * @tc.type  : FUNC
 * @tc.number: GetLastAudioDuration_001
 * @tc.desc  : Test GetLastAudioDuration API
 */
HWTEST_F(RendererInServerUnitTest, GetLastAudioDuration_001, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, MONO,
        AudioChannelLayout::CH_LAYOUT_MONO);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_USAGE_NORMAL);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    rendererInServer->lastStopTime_ = 1;
    rendererInServer->lastStartTime_ = 2;
    int64_t result = rendererInServer->GetLastAudioDuration();
    EXPECT_EQ(result, -1);

    rendererInServer->lastStopTime_ = 3;
    result = rendererInServer->GetLastAudioDuration();
    EXPECT_EQ(result, 1);
}

/**
 * @tc.name  : Test RendererInServer
 * @tc.type  : FUNC
 * @tc.number: HandleOperationStarted_001
 * @tc.desc  : Test HandleOperationStarted API
 */
HWTEST_F(RendererInServerUnitTest, HandleOperationStarted_001, TestSize.Level1)
{
    AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, MONO,
        AudioChannelLayout::CH_LAYOUT_MONO);
    InitAudioProcessConfig(testStreamInfo, DEVICE_TYPE_USB_HEADSET, AUDIO_USAGE_NORMAL);
    rendererInServer = std::make_shared<RendererInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, rendererInServer);

    int32_t ret = rendererInServer->Init();
    EXPECT_EQ(ret, SUCCESS);

    ret = rendererInServer->ConfigServerBuffer();
    EXPECT_EQ(ret, SUCCESS);

    rendererInServer->standByEnable_ = true;
    rendererInServer->HandleOperationStarted();
    EXPECT_EQ(rendererInServer->status_, I_STATUS_STARTED);
}

/**
 * @tc.name  : Test RendererInServer
 * @tc.type  : FUNC
 * @tc.number: SetSourceDuration_001
 * @tc.desc  : Test SetSourceDuration API
 */
HWTEST_F(RendererInServerUnitTest, SetSourceDuration_001, TestSize.Level1)
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
} // namespace AudioStandard
} // namespace OHOS