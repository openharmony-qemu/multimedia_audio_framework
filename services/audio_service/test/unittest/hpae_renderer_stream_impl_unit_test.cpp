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
#include "audio_service_log.h"
#include "audio_info.h"
#include "audio_ring_cache.h"
#include "audio_process_config.h"
#include "linear_pos_time_model.h"
#include "oh_audio_buffer.h"
#include <gtest/gtest.h>
#include "hpae_renderer_stream_impl.h"
#include "policy_handler.h"
#include "hpae_adapter_manager.h"
#include "audio_capturer_private.h"
#include "audio_system_manager.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {
const int32_t CAPTURER_FLAG = 10;
static constexpr uint32_t SAMPLE_RATE_16010 = 16010;
static constexpr uint32_t SAMPLE_RATE_16050 = 16050;
static constexpr uint32_t FRAME_LEN_100MS = 100;
static constexpr uint32_t FRAME_LEN_40MS = 40;
static constexpr uint32_t FRAME_LEN_20MS = 20;
static constexpr int32_t MIN_BUFFER_SIZE = 2;

static inline int32_t GetSizeFromFormat(int32_t format)
{
    return format != SAMPLE_F32LE ? ((format) + 1) : (4); // float 4
}

static std::shared_ptr<HpaeAdapterManager> adapterManager;

class HpaeRendererStreamUnitTest : public ::testing::Test {
public:
    void SetUp();
    void TearDown();
    std::shared_ptr<HpaeRendererStreamImpl> CreateHpaeRendererStreamImpl();
};
void HpaeRendererStreamUnitTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void HpaeRendererStreamUnitTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

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
    config.originalSessionId = 123456; // 123456: session id
    return config;
}

std::shared_ptr<HpaeRendererStreamImpl> HpaeRendererStreamUnitTest::CreateHpaeRendererStreamImpl()
{
    adapterManager = std::make_shared<HpaeAdapterManager>(DUP_PLAYBACK);
    AudioProcessConfig processConfig = GetInnerCapConfig();
    std::string deviceName = "";
    std::shared_ptr<IRendererStream> rendererStream = adapterManager->CreateRendererStream(processConfig, deviceName);
    std::shared_ptr<HpaeRendererStreamImpl> rendererStreamImpl =
        std::static_pointer_cast<HpaeRendererStreamImpl>(rendererStream);
    return rendererStreamImpl;
}

/**
 * @tc.name  : Test HpaeRendererStreamImpl Construct
 * @tc.type  : FUNC
 * @tc.number: HpaeRendererStreamImplConstruct_001
 * @tc.desc  : Test branch when samplingRate = 11025
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRendererStreamUnitConstruct_001, TestSize.Level0)
{
    AudioProcessConfig processConfig = GetInnerCapConfig();
    processConfig.streamInfo.samplingRate = SAMPLE_RATE_11025;
    processConfig.rendererInfo.expectedPlaybackDurationBytes = 1024;

    HpaeRendererStreamImpl rendererStreamImpl(processConfig, true, false);
    EXPECT_EQ(rendererStreamImpl.spanSizeInFrame_, FRAME_LEN_40MS *
        static_cast<uint32_t>(processConfig.streamInfo.samplingRate) / AUDIO_MS_PER_S);
    EXPECT_EQ(rendererStreamImpl.byteSizePerFrame_, processConfig.streamInfo.channels *
        static_cast<size_t>(GetSizeFromFormat(processConfig.streamInfo.format)));
    EXPECT_EQ(rendererStreamImpl.minBufferSize_, MIN_BUFFER_SIZE * rendererStreamImpl.byteSizePerFrame_ *
        rendererStreamImpl.spanSizeInFrame_);
    EXPECT_EQ(rendererStreamImpl.expectedPlaybackDurationMs_, processConfig.rendererInfo.expectedPlaybackDurationBytes *
        AUDIO_MS_PER_S / rendererStreamImpl.byteSizePerFrame_ / processConfig.streamInfo.samplingRate);
    EXPECT_TRUE(rendererStreamImpl.isMoveAble_);
    EXPECT_FALSE(rendererStreamImpl.isCallbackMode_);
}

/**
 * @tc.name  : Test HpaeRendererStreamImpl Construct
 * @tc.type  : FUNC
 * @tc.number: HpaeRendererStreamImplConstruct_002
 * @tc.desc  : Test branch when customSampleRate = 16010
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRendererStreamUnitConstruct_002, TestSize.Level0)
{
    AudioProcessConfig processConfig = GetInnerCapConfig();
    processConfig.streamInfo.customSampleRate = SAMPLE_RATE_16010;
    processConfig.rendererInfo.expectedPlaybackDurationBytes = 1024;

    HpaeRendererStreamImpl rendererStreamImpl(processConfig, true, false);
    EXPECT_EQ(rendererStreamImpl.spanSizeInFrame_, FRAME_LEN_100MS *
        static_cast<uint32_t>(processConfig.streamInfo.customSampleRate) / AUDIO_MS_PER_S);
    EXPECT_EQ(rendererStreamImpl.byteSizePerFrame_, processConfig.streamInfo.channels *
        static_cast<size_t>(GetSizeFromFormat(processConfig.streamInfo.format)));
    EXPECT_EQ(rendererStreamImpl.minBufferSize_, MIN_BUFFER_SIZE * rendererStreamImpl.byteSizePerFrame_ *
        rendererStreamImpl.spanSizeInFrame_);
    EXPECT_EQ(rendererStreamImpl.expectedPlaybackDurationMs_, processConfig.rendererInfo.expectedPlaybackDurationBytes *
        AUDIO_MS_PER_S / rendererStreamImpl.byteSizePerFrame_ / processConfig.streamInfo.customSampleRate);
    EXPECT_TRUE(rendererStreamImpl.isMoveAble_);
    EXPECT_FALSE(rendererStreamImpl.isCallbackMode_);
}

/**
 * @tc.name  : Test HpaeRendererStreamImpl Construct
 * @tc.type  : FUNC
 * @tc.number: HpaeRendererStreamImplConstruct_003
 * @tc.desc  : Test branch when customSampleRate = 16050
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRendererStreamUnitConstruct_003, TestSize.Level0)
{
    AudioProcessConfig processConfig = GetInnerCapConfig();
    processConfig.streamInfo.customSampleRate = SAMPLE_RATE_16050;
    processConfig.rendererInfo.expectedPlaybackDurationBytes = 1024;

    HpaeRendererStreamImpl rendererStreamImpl(processConfig, true, false);
    EXPECT_EQ(rendererStreamImpl.spanSizeInFrame_, FRAME_LEN_20MS *
        static_cast<uint32_t>(processConfig.streamInfo.customSampleRate) / AUDIO_MS_PER_S);
    EXPECT_EQ(rendererStreamImpl.byteSizePerFrame_, processConfig.streamInfo.channels *
        static_cast<size_t>(GetSizeFromFormat(processConfig.streamInfo.format)));
    EXPECT_EQ(rendererStreamImpl.minBufferSize_, MIN_BUFFER_SIZE * rendererStreamImpl.byteSizePerFrame_ *
        rendererStreamImpl.spanSizeInFrame_);
    EXPECT_EQ(rendererStreamImpl.expectedPlaybackDurationMs_, processConfig.rendererInfo.expectedPlaybackDurationBytes *
        AUDIO_MS_PER_S / rendererStreamImpl.byteSizePerFrame_ / processConfig.streamInfo.customSampleRate);
    EXPECT_TRUE(rendererStreamImpl.isMoveAble_);
    EXPECT_FALSE(rendererStreamImpl.isCallbackMode_);
}

/**
 * @tc.name  : Test HpaeRendererStreamImpl Construct
 * @tc.type  : FUNC
 * @tc.number: HpaeRendererStreamImplConstruct_004
 * @tc.desc  : Test branch when channels = 0, byteSizePerFrame_ = 0
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRendererStreamUnitConstruct_004, TestSize.Level0)
{
    AudioProcessConfig processConfig = GetInnerCapConfig();
    processConfig.streamInfo.channels = CHANNEL_UNKNOW;

    HpaeRendererStreamImpl rendererStreamImpl(processConfig, true, false);
    EXPECT_EQ(rendererStreamImpl.spanSizeInFrame_, FRAME_LEN_20MS *
        static_cast<uint32_t>(processConfig.streamInfo.samplingRate) / AUDIO_MS_PER_S);
    EXPECT_EQ(rendererStreamImpl.byteSizePerFrame_, 0);
    EXPECT_EQ(rendererStreamImpl.minBufferSize_, 0);
    EXPECT_EQ(rendererStreamImpl.expectedPlaybackDurationMs_, 0);
    EXPECT_TRUE(rendererStreamImpl.isMoveAble_);
    EXPECT_FALSE(rendererStreamImpl.isCallbackMode_);
}

/**
 * @tc.name  : Test HpaeRendererStreamImpl Construct
 * @tc.type  : FUNC
 * @tc.number: HpaeRendererStreamImplConstruct_005
 * @tc.desc  : Test branch when customSampleRate = 11025
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRendererStreamUnitConstruct_005, TestSize.Level0)
{
    AudioProcessConfig processConfig = GetInnerCapConfig();
    processConfig.streamInfo.customSampleRate = SAMPLE_RATE_11025;
    processConfig.rendererInfo.expectedPlaybackDurationBytes = 1024;

    HpaeRendererStreamImpl rendererStreamImpl(processConfig, true, false);
    EXPECT_EQ(rendererStreamImpl.spanSizeInFrame_, FRAME_LEN_40MS *
        static_cast<uint32_t>(processConfig.streamInfo.customSampleRate) / AUDIO_MS_PER_S);
    EXPECT_EQ(rendererStreamImpl.byteSizePerFrame_, processConfig.streamInfo.channels *
        static_cast<size_t>(GetSizeFromFormat(processConfig.streamInfo.format)));
    EXPECT_EQ(rendererStreamImpl.minBufferSize_, MIN_BUFFER_SIZE * rendererStreamImpl.byteSizePerFrame_ *
        rendererStreamImpl.spanSizeInFrame_);
    EXPECT_EQ(rendererStreamImpl.expectedPlaybackDurationMs_, processConfig.rendererInfo.expectedPlaybackDurationBytes *
        AUDIO_MS_PER_S / rendererStreamImpl.byteSizePerFrame_ / processConfig.streamInfo.customSampleRate);
    EXPECT_TRUE(rendererStreamImpl.isMoveAble_);
    EXPECT_FALSE(rendererStreamImpl.isCallbackMode_);
}

/**
 * @tc.name  : Test AudioProcessProxy API
 * @tc.type  : FUNC
 * @tc.number: GetCurrentTimeStamp_001
 * @tc.desc  : Test AudioProcessProxy interface.
 */
HWTEST_F(HpaeRendererStreamUnitTest, GetCurrentTimeStamp_001, TestSize.Level1)
{
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);
    uint64_t timestamp = 0;
    int32_t ret = unit->GetCurrentTimeStamp(timestamp);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test GetCurrentTimeStamp
 * @tc.type  : FUNC
 * @tc.number: GetCurrentTimeStamp_002
 * @tc.desc  : Test GetCurrentTimeStamp.
 */
HWTEST_F(HpaeRendererStreamUnitTest, GetCurrentTimeStamp_002, TestSize.Level1)
{
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);
    uint64_t timestamp = 0;
    int32_t ret = unit->GetCurrentTimeStamp(timestamp);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioProcessProxy API
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_001
 * @tc.desc  : Test AudioProcessProxy interface.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_001, TestSize.Level1)
{
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);
    int32_t rate = RENDER_RATE_NORMAL;
    EXPECT_EQ(unit->SetRate(rate), SUCCESS);
}

/**
 * @tc.name  : Test AudioProcessProxy API
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_002
 * @tc.desc  : Test AudioProcessProxy interface.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_002, TestSize.Level1)
{
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);
    int32_t rate = RENDER_RATE_DOUBLE;
    EXPECT_EQ(unit->SetRate(rate), SUCCESS);
}


/**
 * @tc.name  : Test AudioProcessProxy API
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_003
 * @tc.desc  : Test AudioProcessProxy interface.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_003, TestSize.Level1)
{
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);
    int32_t rate = RENDER_RATE_HALF;
    EXPECT_EQ(unit->SetRate(rate), SUCCESS);
}

/**
 * @tc.name  : Test GetCurrentPosition
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_004
 * @tc.desc  : Test GetCurrentPosition.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_004, TestSize.Level1)
{
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);
    uint64_t timestamp = 0;
    uint64_t framePosition = 0;
    uint64_t latency = 0;
    int32_t ret = unit->GetCurrentPosition(framePosition, timestamp, latency, Timestamp::MONOTONIC);
    EXPECT_EQ(ret, SUCCESS);
    unit->deviceClass_ = "remote_offload";
    ret = unit->GetCurrentPosition(framePosition, timestamp, latency, Timestamp::MONOTONIC);
    EXPECT_EQ(ret, SUCCESS);
    unit->deviceClass_ = "offload";
    ret = unit->GetCurrentPosition(framePosition, timestamp, latency, Timestamp::MONOTONIC);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test GetCurrentPosition.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_005
 * @tc.desc  : Test GetCurrentPosition.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_005, TestSize.Level1)
{
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);
    int32_t rate = RENDER_RATE_NORMAL;
    EXPECT_EQ(unit->SetRate(rate), SUCCESS);
}

/**
 * @tc.name  : Test OffloadSetVolume.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_006
 * @tc.desc  : Test OffloadSetVolume.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_006, TestSize.Level1)
{
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);
    unit->offloadEnable_ = false;
    float volume = 0.0f;
    auto ret = unit->OffloadSetVolume(volume);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test UpdateSpatializationState.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_007
 * @tc.desc  : Test UpdateSpatializationState.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_007, TestSize.Level1)
{
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);
    bool spatializationEnabled = false;
    bool headTrackingEnabled = false;
    int32_t ret = unit->UpdateSpatializationState(spatializationEnabled, headTrackingEnabled);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test UpdateSpatializationState.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_008
 * @tc.desc  : Test UpdateSpatializationState.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_008, TestSize.Level1)
{
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);
    bool spatializationEnabled = false;
    bool headTrackingEnabled = false;
    int32_t ret = unit->UpdateSpatializationState(spatializationEnabled, headTrackingEnabled);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test GetOffloadApproximatelyCacheTime.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_009
 * @tc.desc  : Test GetOffloadApproximatelyCacheTime.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_009, TestSize.Level1)
{
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);
    unit->offloadEnable_ = false;
    uint64_t timestamp = 0;
    uint64_t paWriteIndex = 0;
    uint64_t cacheTimeDsp = 0;
    uint64_t cacheTimePa = 0;
    int32_t result = unit->GetOffloadApproximatelyCacheTime(timestamp, paWriteIndex, cacheTimeDsp, cacheTimePa);
    EXPECT_EQ(result, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test GetOffloadApproximatelyCacheTime.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_010
 * @tc.desc  : Test GetOffloadApproximatelyCacheTime.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_010, TestSize.Level1)
{
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);
    unit->offloadEnable_ = true;
    uint64_t timestamp = 0;
    uint64_t paWriteIndex = 0;
    uint64_t cacheTimeDsp = 0;
    uint64_t cacheTimePa = 0;
    int32_t ret = unit->GetOffloadApproximatelyCacheTime(timestamp, paWriteIndex, cacheTimeDsp, cacheTimePa);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test GetOffloadApproximatelyCacheTime.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_011
 * @tc.desc  : Test GetOffloadApproximatelyCacheTime.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_011, TestSize.Level1)
{
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);
    unit->offloadEnable_ = true;
    uint64_t timestamp = 0;
    uint64_t paWriteIndex = 0;
    uint64_t cacheTimeDsp = 0;
    uint64_t cacheTimePa = 0;
    int32_t ret = unit->GetOffloadApproximatelyCacheTime(timestamp, paWriteIndex, cacheTimeDsp, cacheTimePa);
    EXPECT_EQ(ret,  SUCCESS);
}

/**
 * @tc.name  : Test SetClientVolume.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_012
 * @tc.desc  : Test SetClientVolume.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_012, TestSize.Level1)
{
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);
    float clientVolume = -1;
    int32_t ret = unit->SetClientVolume(clientVolume);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test SetClientVolume.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_013
 * @tc.desc  : Test SetClientVolume.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_013, TestSize.Level1)
{
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);
    float clientVolume = 1.5;
    int32_t ret = unit->SetClientVolume(clientVolume);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test SetClientVolume.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_014
 * @tc.desc  : Test SetClientVolume.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_014, TestSize.Level1)
{
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);
    float clientVolume = 0.5;
    int32_t ret = unit->SetClientVolume(clientVolume);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test GetWritableSize.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_015
 * @tc.desc  : Test GetWritableSize.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_015, TestSize.Level1)
{
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);
    int32_t ret = unit->GetWritableSize();
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name  : Test EnqueueBuffer.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_016
 * @tc.desc  : Test EnqueueBuffer.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_016, TestSize.Level1)
{
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);
    BufferDesc bufferDesc = {
        .buffer = nullptr,
        .bufLength = 0,
        .dataLength = 0,
        .metaBuffer = nullptr,
        .metaLength = 0
    };
    int32_t ret = unit->EnqueueBuffer(bufferDesc);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test SetAudioEffectMode.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_017
 * @tc.desc  : Test SetAudioEffectMode.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_017, TestSize.Level1)
{
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);
    int32_t effectMode = 0;
    int32_t ret = unit->SetAudioEffectMode(effectMode);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetAudioEffectMode.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_018
 * @tc.desc  : Test SetAudioEffectMode.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_018, TestSize.Level1)
{
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);
    int32_t effectMode = 0;
    int32_t ret = unit->SetAudioEffectMode(effectMode);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test Start.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_019
 * @tc.desc  : Test Start.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_019, TestSize.Level1)
{
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);
    int32_t ret = unit->Start();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test Pause.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_020
 * @tc.desc  : Test Pause.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_020, TestSize.Level1)
{
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);
    bool isStandby = false;
    int32_t ret = unit->Pause(isStandby);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test Pause.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_021
 * @tc.desc  : Test Pause.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_021, TestSize.Level1)
{
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);
    bool isStandby = false;
    int32_t ret = unit->Pause(isStandby);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test Flush.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_022
 * @tc.desc  : Test Flush.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_022, TestSize.Level1)
{
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);
    int32_t ret = unit->Flush();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test Flush.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_023
 * @tc.desc  : Test Flush.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_023, TestSize.Level1)
{
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);
    int32_t ret = unit->Flush();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test Stop.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_024
 * @tc.desc  : Test Stop.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_024, TestSize.Level1)
{
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);
    int32_t ret = unit->Stop();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test Release.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_025
 * @tc.desc  : Test Release.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_025, TestSize.Level1)
{
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);
    unit->state_ = RUNNING;
    int32_t ret = unit->Release();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test GetLatency.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_026
 * @tc.desc  : Test GetLatency.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_026, TestSize.Level1)
{
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);
    uint64_t latency = 0;
    int32_t ret = unit->GetLatency(latency);
    EXPECT_EQ(ret, SUCCESS);
    unit->deviceClass_ = "remote_offload";
    ret = unit->GetLatency(latency);
    EXPECT_EQ(ret, SUCCESS);
    unit->deviceClass_ = "offload";
    ret = unit->GetLatency(latency);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test Drain.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_027
 * @tc.desc  : Test Drain.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_027, TestSize.Level1)
{
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);
    int32_t ret = unit->Drain();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetRate.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_028
 * @tc.desc  : Test SetRate.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_028, TestSize.Level1)
{
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);
    int32_t rate = RENDER_RATE_NORMAL;
    EXPECT_EQ(unit->SetRate(rate), SUCCESS);
}

/**
 * @tc.name  : Test StartWithSyncId.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_029
 * @tc.desc  : Test StartWithSyncId.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_029, TestSize.Level1)
{
    int32_t syncId = 123;
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);
    int32_t ret = unit->StartWithSyncId(syncId);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test OnDeviceClassChange.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_030
 * @tc.desc  : Test OnDeviceClassChange.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_030, TestSize.Level1)
{
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);

    AudioCallBackStreamInfo info = {
        .deviceClass = "remote_offload",
        .framePosition = 10000
    };
    unit->deviceClass_ = "primary";
    unit->OnDeviceClassChange(info);
    EXPECT_EQ(unit->lastHdiFramePosition_, 10000);
    EXPECT_EQ(unit->lastFramePosition_, 10000);

    info.deviceClass = "offload";
    unit->OnDeviceClassChange(info);
    EXPECT_EQ(unit->lastHdiFramePosition_, 10000);
    EXPECT_EQ(unit->lastFramePosition_, 10000);

    info.hdiFramePosition = 10000;
    unit->OnDeviceClassChange(info);
    EXPECT_GT(unit->lastHdiFramePosition_, 10000);
}
}
}