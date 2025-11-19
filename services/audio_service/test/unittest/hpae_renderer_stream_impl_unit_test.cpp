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
#include <gmock/gmock.h>
#include "hpae_renderer_stream_impl.h"
#include "policy_handler.h"
#include "hpae_adapter_manager.h"
#include "audio_capturer_private.h"
#include "audio_system_manager.h"
#include "audio_volume.h"

using namespace testing::ext;
using namespace testing;
namespace OHOS {
namespace AudioStandard {
const int32_t CAPTURER_FLAG = 10;
static constexpr uint32_t SAMPLE_RATE_16010 = 16010;
static constexpr uint32_t SAMPLE_RATE_16050 = 16050;
static constexpr uint32_t FRAME_LEN_100MS = 100;
static constexpr uint32_t FRAME_LEN_40MS = 40;
static constexpr uint32_t FRAME_LEN_20MS = 20;
static constexpr int32_t MIN_BUFFER_SIZE = 2;
static constexpr uint32_t TEST_SESSION_ID = 123456;

static inline int32_t GetSizeFromFormat(int32_t format)
{
    return format != SAMPLE_F32LE ? ((format) + 1) : (4); // float 4
}

static std::shared_ptr<HpaeAdapterManager> adapterManager;

class MockWriteCallback : public IWriteCallback {
public:
    MockWriteCallback() = default;
    virtual ~MockWriteCallback() = default;
    MOCK_METHOD(int32_t, OnWriteData, (size_t length), (override));
    MOCK_METHOD(int32_t, OnWriteData, (int8_t *inputData, size_t requestDataLen), (override));
    MOCK_METHOD(int32_t, GetAvailableSize, (size_t &length), (override));
};

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
    config.originalSessionId = TEST_SESSION_ID;
    return config;
}

static void AddStreamVolume()
{
    StreamVolumeParams param;
    param.sessionId = TEST_SESSION_ID;
    param.streamType = AudioStreamType::STREAM_MUSIC;
    param.streamUsage = 0;
    param.uid = CAPTURER_FLAG;
    param.pid = CAPTURER_FLAG;
    param.isSystemApp = false;
    param.mode = AudioMode::AUDIO_MODE_PLAYBACK;
    param.isVKB = false;
    AudioVolume::GetInstance()->AddStreamVolume(param);
}

static void RemoveStreamVolume()
{
    AudioVolume::GetInstance()->RemoveStreamVolume(TEST_SESSION_ID);
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
    auto ret = unit->OffloadSetVolume();
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
    AddStreamVolume();
    struct VolumeValues volumes = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
    std::string volumeDeviceClass = unit->deviceClass_ == "remote_offload" ? "remote" : "offload";
    uint32_t originStreamIndex = unit->streamIndex_;
    float volume = AudioVolume::GetInstance()->GetVolume(unit->streamIndex_, unit->processConfig_.streamType,
        volumeDeviceClass, &volumes);
    AudioVolume::GetInstance()->SetHistoryVolume(unit->streamIndex_, volume);
    ret = unit->OffloadSetVolume();
    RemoveStreamVolume();
    unit->streamIndex_ = originStreamIndex;
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
    unit->lastPrintTimestamp_.store(0);
    uint64_t latency = 0;
    int32_t ret = unit->GetLatency(latency);
    EXPECT_EQ(ret, SUCCESS);
    unit->deviceClass_ = "remote_offload";
    std::vector<uint64_t> timestampCurrent = {0};
    ClockTime::GetAllTimeStamp(timestampCurrent);
    unit->lastPrintTimestamp_.store(timestampCurrent[0]);
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

/**
 * @tc.name  : Test WriteDataFromRingBuffer.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_031
 * @tc.desc  : Test WriteDataFromRingBuffer.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_031, TestSize.Level0)
{
    std::shared_ptr<HpaeRendererStreamImpl> hpaeRenderer = CreateHpaeRendererStreamImpl();
    EXPECT_NE(hpaeRenderer, nullptr);

    // 10 bytes
    constexpr size_t bufferSize = 10;

    hpaeRenderer->ringBuffer_ = AudioRingCache::Create(bufferSize);
    std::vector<int8_t> tmpBuffer(bufferSize, 1);
    hpaeRenderer->ringBuffer_->Enqueue({reinterpret_cast<uint8_t *>(tmpBuffer.data()), bufferSize});
    tmpBuffer = std::vector<int8_t>(bufferSize, 0);
    size_t requestDataLen = bufferSize;
    int32_t ret = hpaeRenderer->WriteDataFromRingBuffer(false, tmpBuffer.data(), requestDataLen);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(requestDataLen, bufferSize);
    EXPECT_THAT(tmpBuffer, Each(Eq(1)));
}

/**
 * @tc.name  : Test WriteDataFromRingBuffer.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_032
 * @tc.desc  : Test WriteDataFromRingBuffer.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_032, TestSize.Level0)
{
    std::shared_ptr<HpaeRendererStreamImpl> hpaeRenderer = CreateHpaeRendererStreamImpl();
    EXPECT_NE(hpaeRenderer, nullptr);

    // 10 bytes
    constexpr size_t bufferSize = 10;

    hpaeRenderer->ringBuffer_ = AudioRingCache::Create(bufferSize);
    std::vector<int8_t> tmpBuffer(bufferSize, 1);
    hpaeRenderer->ringBuffer_->Enqueue({reinterpret_cast<uint8_t *>(tmpBuffer.data()), bufferSize - 1});
    tmpBuffer = std::vector<int8_t>(bufferSize, 0);
    size_t requestDataLen = bufferSize;
    int32_t ret = hpaeRenderer->WriteDataFromRingBuffer(false, tmpBuffer.data(), requestDataLen);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test WriteDataFromRingBuffer.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_033
 * @tc.desc  : Test WriteDataFromRingBuffer.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_033, TestSize.Level0)
{
    std::shared_ptr<HpaeRendererStreamImpl> hpaeRenderer = CreateHpaeRendererStreamImpl();
    EXPECT_NE(hpaeRenderer, nullptr);

    // 10 bytes
    constexpr size_t bufferSize = 10;

    hpaeRenderer->ringBuffer_ = AudioRingCache::Create(bufferSize);
    std::vector<int8_t> tmpBuffer(bufferSize, 1);
    hpaeRenderer->ringBuffer_->Enqueue({reinterpret_cast<uint8_t *>(tmpBuffer.data()), bufferSize - 1});
    tmpBuffer = std::vector<int8_t>(bufferSize, 2);
    size_t requestDataLen = bufferSize;
    int32_t ret = hpaeRenderer->WriteDataFromRingBuffer(true, tmpBuffer.data(), requestDataLen);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(requestDataLen, bufferSize - 1);
    EXPECT_THAT(std::vector<int8_t>(tmpBuffer.begin(), tmpBuffer.end() - 1), Each(Eq(1)));
    EXPECT_EQ(tmpBuffer[bufferSize - 1], 0);
}

/**
 * @tc.name  : Test OnStreamData.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_034
 * @tc.desc  : Test OnStreamData.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_034, TestSize.Level1)
{
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(nullptr, unit);
 
    AudioCallBackStreamInfo info = {
        .deviceClass = "remote_offload",
        .framePosition = 10000,
    };
   
    unit->OnStreamData(info);
    EXPECT_EQ(10000, unit->lastHdiFramePosition_);
    EXPECT_EQ(10000, unit->lastFramePosition_);

    unit->isCallbackMode_ = false;
    info.needData = true;
    info.requestDataLen = 0;
    unit->OnStreamData(info);
    EXPECT_EQ(OFFLOAD_DEFAULT, unit->offloadStatePolicy_);
    EXPECT_EQ(INVALID, unit->state_);
}

/**
 * @tc.name  : Test OnStreamDataPrimary.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_035
 * @tc.desc  : Test OnStreamDataPrimary.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_035, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    auto unit = std::make_shared<HpaeRendererStreamImpl>(processConfig, 0, 1); // callback mode
 
    AudioCallBackStreamInfo info = {
        .needData = false
    };

    EXPECT_NE(unit->OnStreamData(info), SUCCESS); // writecallback nullptr

    auto mockWriteCallback = std::make_shared<MockWriteCallback>();
    unit->writeCallback_ = mockWriteCallback;
    EXPECT_EQ(unit->OnStreamData(info), SUCCESS); // needData false, noneed callback

    info.needData = true;
    size_t framesize = 0;
    EXPECT_CALL(*mockWriteCallback, GetAvailableSize(framesize))
        .WillOnce(Return(0))
        .WillOnce(Return(0));
    EXPECT_CALL(*mockWriteCallback, OnWriteData(nullptr, 0))
        .WillOnce(Return(0))
        .WillOnce(Return(-1));
    EXPECT_EQ(unit->OnStreamData(info), SUCCESS); // onwritedata success

    EXPECT_NE(unit->OnStreamData(info), SUCCESS); // onwritedata error
}

/**
 * @tc.name  : Test OnQueryUnderrun.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_036
 * @tc.desc  : Test OnQueryUnderrun.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_036, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    auto mockWriteCallback = std::make_shared<MockWriteCallback>();

    auto unit = std::make_shared<HpaeRendererStreamImpl>(processConfig, 0, 0);
    unit->writeCallback_ = mockWriteCallback;
    EXPECT_EQ(unit->OnQueryUnderrun(), false); // not callback mode, return false

    unit = std::make_shared<HpaeRendererStreamImpl>(processConfig, 0, 1);
    EXPECT_EQ(unit->OnQueryUnderrun(), false); // writecallback null, return false

    unit->writeCallback_ = mockWriteCallback;
    size_t framesize = 0;
    EXPECT_CALL(*mockWriteCallback, GetAvailableSize(framesize))
        .WillOnce(DoAll(SetArgReferee<0>(0), Return(SUCCESS)))
        .WillOnce(DoAll(SetArgReferee<0>(1), Return(SUCCESS)));
    EXPECT_EQ(unit->OnQueryUnderrun(), true); // requestDataLen == 0, return true
    EXPECT_EQ(unit->OnQueryUnderrun(), false); // requestDataLen != 0, return false
}

/**
 * @tc.name  : Test OnStreamData with noWaitDataFlag.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_037
 * @tc.desc  : Test OnStreamData with noWaitDataFlag.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_037, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    auto unit = std::make_shared<HpaeRendererStreamImpl>(processConfig, 0, 1); // callback mode
 
    std::vector<int8_t> buffer(2048);
    AudioCallBackStreamInfo info = {
        .needData = true,
        .requestDataLen = 1024,
        .inputData = buffer.data(),
    };
    auto mockWriteCallback = std::make_shared<MockWriteCallback>();
    unit->writeCallback_ = mockWriteCallback;
    unit->noWaitDataFlag_ = false; // when start
    EXPECT_CALL(*mockWriteCallback, GetAvailableSize(::testing::_))
        .WillOnce(DoAll(
            SetArgReferee<0>(1023), // not enough dataLen, noWaitDataFlag_ false, do not write
            Return(0)
        ))
        .WillOnce(DoAll(
            SetArgReferee<0>(1024), // enough dataLen, noWaitDataFlag_ false, write
            Return(0)
        ))
        .WillOnce(DoAll(
            SetArgReferee<0>(1023), // not enough dataLen, noWaitDataFlag_ true, write
            Return(0)
    ));
    EXPECT_CALL(*mockWriteCallback, OnWriteData(::testing::_, ::testing::_))
        .WillOnce(Return(-1))
        .WillOnce(Return(0))
        .WillOnce(Return(0));
    EXPECT_NE(unit->OnStreamData(info), SUCCESS); // onwritedata error
    EXPECT_FALSE(unit->noWaitDataFlag_);
    EXPECT_EQ(unit->OnStreamData(info), SUCCESS); // onwritedata success
    EXPECT_TRUE(unit->noWaitDataFlag_);
    EXPECT_EQ(unit->OnStreamData(info), SUCCESS); // onwritedata success even if not enough data
    EXPECT_TRUE(unit->noWaitDataFlag_);
}

/**
 * @tc.name  : Test Start.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_038
 * @tc.desc  : Test Start with noWaitDataFlag_.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_038, TestSize.Level1)
{
    adapterManager = std::make_shared<HpaeAdapterManager>(DUP_PLAYBACK);
    AudioProcessConfig processConfig;
    processConfig.streamInfo.customSampleRate = SAMPLE_RATE_16010;
    std::string deviceName = "";
    std::shared_ptr<IRendererStream> rendererStream = adapterManager->CreateRendererStream(processConfig, deviceName);
    std::shared_ptr<HpaeRendererStreamImpl> rendererStreamImpl =
        std::static_pointer_cast<HpaeRendererStreamImpl>(rendererStream);
    EXPECT_NE(rendererStreamImpl, nullptr);
    int32_t ret = rendererStreamImpl->Start();
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(rendererStreamImpl->noWaitDataFlag_, false);

    rendererStreamImpl->noWaitDataFlag_ = true;
    int32_t syncId = 123;
    ret = rendererStreamImpl->StartWithSyncId(syncId);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(rendererStreamImpl->noWaitDataFlag_, false);
}
}
}