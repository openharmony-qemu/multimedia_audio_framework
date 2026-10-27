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
#include "hpae_info.h"
#include "util/id_handler.h"

using namespace testing::ext;
using namespace testing;
namespace OHOS {
namespace AudioStandard {
const int32_t CAPTURER_FLAG = 10;
static constexpr uint32_t SAMPLE_RATE_16010 = 16010;
static constexpr uint32_t SAMPLE_RATE_16050 = 16050;
static constexpr uint32_t FRAME_LEN_20MS = 20;
static constexpr int32_t MIN_BUFFER_SIZE = 2;
static constexpr uint32_t TEST_SESSION_ID = 123456;

static inline int32_t GetSizeFromFormat(int32_t format)
{
    format = format > SAMPLE_F32LE ? -1 : format;
    return format != SAMPLE_F32LE ? ((format) + 1) : (4); // float 4
}

static std::shared_ptr<HpaeAdapterManager> adapterManager;

// defined in hpae_renderer_stream_impl.cpp with external linkage for direct unit testing
HPAE::FadeType ConvertFadeStrategyToFadeType(FadeStrategy strategy);

class MockWriteCallback : public IWriteCallback {
public:
    MockWriteCallback() = default;
    virtual ~MockWriteCallback() = default;
    MOCK_METHOD(int32_t, OnWriteData, (size_t length), (override));
    MOCK_METHOD(int32_t, OnWriteData, (int8_t *inputData, size_t requestDataLen), (override));
    MOCK_METHOD(int32_t, GetAvailableSize, (size_t &length), (override));
};

class MockStatusCallback : public IStatusCallback {
public:
    MOCK_METHOD(void, OnStatusUpdate, (IOperation operation), (override));
    virtual ~MockStatusCallback() = default;
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
    param.isVKB = VKB_INVALID;
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
 * @tc.name  : Test ConvertFadeStrategyToFadeType mapping
 * @tc.type  : FUNC
 * @tc.number: ConvertFadeStrategyToFadeType_001
 * @tc.desc  : Test FadeStrategy to FadeType explicit mapping (enum orders differ).
 */
HWTEST_F(HpaeRendererStreamUnitTest, ConvertFadeStrategyToFadeType_001, TestSize.Level0)
{
    EXPECT_EQ(HPAE::DEFAULT_FADE, ConvertFadeStrategyToFadeType(FADE_STRATEGY_DEFAULT));
    EXPECT_EQ(HPAE::NONE_FADE, ConvertFadeStrategyToFadeType(FADE_STRATEGY_NONE));
    EXPECT_EQ(HPAE::SHORT_FADE, ConvertFadeStrategyToFadeType(FADE_STRATEGY_SHORTER));
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
    std::string deviceName = "temp";
 
    HpaeRendererStreamImpl rendererStreamImpl(processConfig, true, deviceName, false);
    EXPECT_EQ(rendererStreamImpl.usedSampleRate_, SAMPLE_RATE_11025);
    EXPECT_EQ(rendererStreamImpl.spanSizeInFrame_, FRAME_LEN_20MS *
        static_cast<uint32_t>(processConfig.streamInfo.samplingRate) / AUDIO_MS_PER_S);
    EXPECT_EQ(rendererStreamImpl.byteSizePerFrame_, processConfig.streamInfo.channels *
        static_cast<size_t>(GetSizeFromFormat(processConfig.streamInfo.format)));
    EXPECT_EQ(rendererStreamImpl.minBufferSize_, MIN_BUFFER_SIZE * rendererStreamImpl.byteSizePerFrame_ *
        rendererStreamImpl.spanSizeInFrame_);
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
    std::string deviceName = "temp";
 
    HpaeRendererStreamImpl rendererStreamImpl(processConfig, true, deviceName, false);
    EXPECT_EQ(rendererStreamImpl.usedSampleRate_, SAMPLE_RATE_16010);
    EXPECT_EQ(rendererStreamImpl.spanSizeInFrame_, FRAME_LEN_20MS *
        static_cast<uint32_t>(processConfig.streamInfo.customSampleRate) / AUDIO_MS_PER_S);
    EXPECT_EQ(rendererStreamImpl.byteSizePerFrame_, processConfig.streamInfo.channels *
        static_cast<size_t>(GetSizeFromFormat(processConfig.streamInfo.format)));
    EXPECT_EQ(rendererStreamImpl.minBufferSize_, MIN_BUFFER_SIZE * rendererStreamImpl.byteSizePerFrame_ *
        rendererStreamImpl.spanSizeInFrame_);
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
    std::string deviceName = "temp";
 
    HpaeRendererStreamImpl rendererStreamImpl(processConfig, true, deviceName, false);
    EXPECT_EQ(rendererStreamImpl.usedSampleRate_, SAMPLE_RATE_16050);
    EXPECT_EQ(rendererStreamImpl.spanSizeInFrame_, FRAME_LEN_20MS *
        static_cast<uint32_t>(processConfig.streamInfo.customSampleRate) / AUDIO_MS_PER_S);
    EXPECT_EQ(rendererStreamImpl.byteSizePerFrame_, processConfig.streamInfo.channels *
        static_cast<size_t>(GetSizeFromFormat(processConfig.streamInfo.format)));
    EXPECT_EQ(rendererStreamImpl.minBufferSize_, MIN_BUFFER_SIZE * rendererStreamImpl.byteSizePerFrame_ *
        rendererStreamImpl.spanSizeInFrame_);
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
    std::string deviceName = "temp";
 
    HpaeRendererStreamImpl rendererStreamImpl(processConfig, true, deviceName, false);
    EXPECT_EQ(rendererStreamImpl.usedSampleRate_, processConfig.streamInfo.samplingRate);
    EXPECT_EQ(rendererStreamImpl.spanSizeInFrame_, FRAME_LEN_20MS *
        static_cast<uint32_t>(processConfig.streamInfo.samplingRate) / AUDIO_MS_PER_S);
    EXPECT_EQ(rendererStreamImpl.byteSizePerFrame_, 0);
    EXPECT_EQ(rendererStreamImpl.minBufferSize_, 0);
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
    std::string deviceName = "temp";
 
    HpaeRendererStreamImpl rendererStreamImpl(processConfig, true, deviceName, false);
    EXPECT_EQ(rendererStreamImpl.usedSampleRate_, SAMPLE_RATE_11025);
    EXPECT_EQ(rendererStreamImpl.spanSizeInFrame_, FRAME_LEN_20MS *
        static_cast<uint32_t>(processConfig.streamInfo.customSampleRate) / AUDIO_MS_PER_S);
    EXPECT_EQ(rendererStreamImpl.byteSizePerFrame_, processConfig.streamInfo.channels *
        static_cast<size_t>(GetSizeFromFormat(processConfig.streamInfo.format)));
    EXPECT_EQ(rendererStreamImpl.minBufferSize_, MIN_BUFFER_SIZE * rendererStreamImpl.byteSizePerFrame_ *
        rendererStreamImpl.spanSizeInFrame_);
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
    ret = unit->GetOffloadLatency();
    EXPECT_EQ(ret, SUCCESS);

    unit->deviceClass_ = "remote_offload";
    ret = unit->GetCurrentPosition(framePosition, timestamp, latency, Timestamp::MONOTONIC);
    EXPECT_EQ(ret, SUCCESS);
    unit->deviceClass_ = "offload";
    ret = unit->GetCurrentPosition(framePosition, timestamp, latency, Timestamp::MONOTONIC);
    EXPECT_EQ(ret, SUCCESS);
    ret = unit->GetOffloadLatency();
    EXPECT_EQ(ret, SUCCESS);
    unit->processConfig_.streamType = STREAM_MOVIE;
    ret = unit->GetOffloadLatency();
    EXPECT_EQ(ret, SUCCESS);
    unit->isWriteFirst_ = true;
    unit->UpdateInnerCapWriteState(true);
    EXPECT_EQ(unit->isWriteFirst_, true);
    unit->UpdateInnerCapWriteState(false);
    EXPECT_EQ(unit->isWriteFirst_, false);
}

/**
 * @tc.name  : Test SetSpeed on a session outside the offload engine chain.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_SetSpeed_NoHdiCapability_001
 * @tc.desc  : Sessions without offload-engine HDI speed capability must be rejected so the
 *             client falls back to local sonic speed (e.g. HiCar MCH speed silent-drop fix).
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_SetSpeed_NoHdiCapability_001, TestSize.Level1)
{
    auto rendererStreamImpl = CreateHpaeRendererStreamImpl();
    ASSERT_NE(rendererStreamImpl, nullptr);
    // The session is not registered in HpaeManager, so the capability query returns false
    // and SetSpeed must reject instead of storing the value silently.
    EXPECT_EQ(rendererStreamImpl->SetSpeed(2.0f), ERR_NOT_SUPPORTED);
}

/**
 * @tc.name  : Test GetLatencyWithFlag with no hardware/engine flag.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_GetLatencyWithFlag_001
 * @tc.desc  : Flag 0 should return success and latency 0 without waiting.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_GetLatencyWithFlag_001, TestSize.Level1)
{
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);
    uint64_t latency = 123; // preset non-zero
    int32_t ret = unit->GetLatencyWithFlag(latency, static_cast<LatencyFlag>(0));
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(latency, 0u);
}

/**
 * @tc.name  : Test GetLatencyWithFlag timeout before first data.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_GetLatencyWithFlag_002
 * @tc.desc  : WaitFirstStreamData returns false, expect ERR_OPERATION_FAILED.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_GetLatencyWithFlag_002, TestSize.Level1)
{
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);
    unit->firstStreamDataReceived_.store(false, std::memory_order_release);
    uint64_t latency = 0;
    int32_t ret = unit->GetLatencyWithFlag(latency, LATENCY_FLAG_ENGINE);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test GetLatencyWithFlag hardware path success.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_GetLatencyWithFlag_003
 * @tc.desc  : FetchSinkLatency success should include sink latency.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_GetLatencyWithFlag_003, TestSize.Level1)
{
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);
    unit->firstStreamDataReceived_.store(true, std::memory_order_release);
    {
        std::lock_guard<std::mutex> lock(unit->sinkLatencyFetcherMutex_);
        unit->sinkLatencyFetcher_ = [] (uint32_t &latency) {
            latency = 7; // ms
            return SUCCESS;
        };
    }
    uint64_t latency = 0;
    int32_t ret = unit->GetLatencyWithFlag(latency, LATENCY_FLAG_HARDWARE);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(latency, 7u * AUDIO_US_PER_MS);
}

/**
 * @tc.name  : Test GetLatencyWithFlag hardware fetch failure.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_GetLatencyWithFlag_004
 * @tc.desc  : FetchSinkLatency failure should propagate error.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_GetLatencyWithFlag_004, TestSize.Level1)
{
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);
    unit->firstStreamDataReceived_.store(true, std::memory_order_release);
    {
        std::lock_guard<std::mutex> lock(unit->sinkLatencyFetcherMutex_);
        unit->sinkLatencyFetcher_ = [] (uint32_t &latency) {
            (void)latency;
            return ERR_INVALID_OPERATION;
        };
    }
    uint64_t latency = 0;
    int32_t ret = unit->GetLatencyWithFlag(latency, LATENCY_FLAG_HARDWARE);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
}

/**
 * @tc.name  : Test GetLatencyWithFlag engine path only.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_GetLatencyWithFlag_005
 * @tc.desc  : When only engine flag set, should add internal latency_ value.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_GetLatencyWithFlag_005, TestSize.Level1)
{
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);
    unit->firstStreamDataReceived_.store(true, std::memory_order_release);
    {
        std::unique_lock<std::shared_mutex> lock(unit->latencyMutex_);
        unit->latency_ = 1234;
    }
    uint64_t latency = 0;
    int32_t ret = unit->GetLatencyWithFlag(latency, LATENCY_FLAG_ENGINE);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(latency, 1234u);
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
    unit->deviceClass_ = "offload";
    auto ret = unit->OffloadSetVolume();
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
    AddStreamVolume();
    struct VolumeValues volumes = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
    std::string volumeDeviceClass = unit->deviceClass_ == "remote_offload" ? "remote" : "offload";
    auto renderId = IdHandler::GetInstance().GetRenderIdByDeviceClass(volumeDeviceClass);
    uint32_t originStreamIndex = unit->streamIndex_;
    float volume = AudioVolume::GetInstance()->GetVolume(unit->streamIndex_, unit->processConfig_.streamType,
        renderId, &volumes);
    AudioVolume::GetInstance()->SetHistoryVolume(unit->streamIndex_, volume);
    ret = unit->OffloadSetVolume();
    RemoveStreamVolume();
    unit->streamIndex_ = originStreamIndex;
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test OnStreamData macAddress refresh.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_044
 * @tc.desc  : Test OnStreamData refreshes macAddress from callback info.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_044, TestSize.Level1)
{
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);
    AudioCallBackStreamInfo info;
    info.deviceClass = "primary";
    info.deviceNetId = "local";
    info.macAddress = "11:22:33:44:55:66";
    info.needData = false;
    EXPECT_EQ(unit->OnStreamData(info), SUCCESS);
    EXPECT_EQ(unit->macAddress_, "11:22:33:44:55:66");
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
 * @tc.name  : Test OnDeviceClassChange no change and no hdiFramePosition.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_030a
 * @tc.desc  : Verify lastFramePosition_ and lastHdiFramePosition_ stay unchanged when device class is the same
 *             and hdiFramePosition is 0 (needLog=false path).
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_030a, TestSize.Level1)
{
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);

    unit->deviceClass_ = "primary";
    unit->lastFramePosition_ = 5000;
    unit->lastHdiFramePosition_ = 3000;

    AudioCallBackStreamInfo info = {
        .deviceClass = "primary",
        .framePosition = 9999,
        .hdiFramePosition = 0
    };
    unit->OnDeviceClassChange(info);
    EXPECT_EQ(unit->lastFramePosition_, 5000);
    EXPECT_EQ(unit->lastHdiFramePosition_, 3000);
}

/**
 * @tc.name  : Test OnDeviceClassChange normal to remote_offload forward.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_030b
 * @tc.desc  : Verify lastHdiFramePosition_ accumulates duration when switching to remote_offload with
 *             newFramePosition > lastFramePosition_.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_030b, TestSize.Level1)
{
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);

    unit->deviceClass_ = "primary";
    unit->lastFramePosition_ = 1000;
    unit->lastHdiFramePosition_ = 2000;

    AudioCallBackStreamInfo info = {
        .deviceClass = "remote_offload",
        .framePosition = 6000,
        .hdiFramePosition = 0
    };
    unit->OnDeviceClassChange(info);
    uint64_t expectedHdi = 2000 + (6000 - 1000);
    EXPECT_EQ(unit->lastHdiFramePosition_, expectedHdi);
    EXPECT_EQ(unit->lastFramePosition_, 6000);
}

/**
 * @tc.name  : Test OnDeviceClassChange normal to remote_offload backward.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_030c
 * @tc.desc  : Verify lastHdiFramePosition_ subtracts duration when switching to remote_offload with
 *             newFramePosition < lastFramePosition_ (backward position).
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_030c, TestSize.Level1)
{
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);

    unit->deviceClass_ = "primary";
    unit->lastFramePosition_ = 8000;
    unit->lastHdiFramePosition_ = 5000;

    AudioCallBackStreamInfo info = {
        .deviceClass = "remote_offload",
        .framePosition = 3000,
        .hdiFramePosition = 0
    };
    unit->OnDeviceClassChange(info);
    uint64_t duration = 8000 - 3000;
    uint64_t expectedHdi = 5000 - duration;
    EXPECT_EQ(unit->lastHdiFramePosition_, expectedHdi);
    EXPECT_EQ(unit->lastFramePosition_, 3000);
}

/**
 * @tc.name  : Test OnDeviceClassChange normal to remote_offload backward underflow.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_030d
 * @tc.desc  : Verify lastHdiFramePosition_ clamps to 0 when duration exceeds lastHdiFramePosition_.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_030d, TestSize.Level1)
{
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);

    unit->deviceClass_ = "primary";
    unit->lastFramePosition_ = 10000;
    unit->lastHdiFramePosition_ = 1000;

    AudioCallBackStreamInfo info = {
        .deviceClass = "remote_offload",
        .framePosition = 2000,
        .hdiFramePosition = 0
    };
    unit->OnDeviceClassChange(info);
    EXPECT_EQ(unit->lastHdiFramePosition_, 0);
    EXPECT_EQ(unit->lastFramePosition_, 2000);
}

/**
 * @tc.name  : Test OnDeviceClassChange remote_offload to normal.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_030e
 * @tc.desc  : Verify lastHdiFramePosition_ unchanged when switching from remote_offload to non-offload.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_030e, TestSize.Level1)
{
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);

    unit->deviceClass_ = "remote_offload";
    unit->lastFramePosition_ = 4000;
    unit->lastHdiFramePosition_ = 7000;

    AudioCallBackStreamInfo info = {
        .deviceClass = "offload",
        .framePosition = 9000,
        .hdiFramePosition = 0
    };
    unit->OnDeviceClassChange(info);
    EXPECT_EQ(unit->lastHdiFramePosition_, 7000);
    EXPECT_EQ(unit->lastFramePosition_, 9000);
}

/**
 * @tc.name  : Test OnDeviceClassChange hdiFramePosition accumulation.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_030f
 * @tc.desc  : Verify hdiFramePosition is accumulated into lastHdiFramePosition_ and
 *             rewindTime is stored in rewindFramePosition_ converted from microseconds to samples.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_030f, TestSize.Level1)
{
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);

    unit->deviceClass_ = "primary";
    unit->lastFramePosition_ = 0;
    unit->lastHdiFramePosition_ = 0;
    uint32_t sampleRate = unit->usedSampleRate_;

    AudioCallBackStreamInfo info = {
        .deviceClass = "primary",
        .framePosition = 0,
        .hdiFramePosition = 1000000, // 1 second in us
        .rewindTime = 500000         // 0.5 second in us
    };
    unit->OnDeviceClassChange(info);
    uint64_t expectedHdi = static_cast<uint64_t>(1000000) * sampleRate / AUDIO_US_PER_S;
    uint64_t expectedRewind = static_cast<uint64_t>(500000) * sampleRate / AUDIO_US_PER_S;
    EXPECT_EQ(unit->lastHdiFramePosition_, expectedHdi);
    EXPECT_EQ(unit->rewindFramePosition_, expectedRewind);
    EXPECT_EQ(unit->lastFramePosition_, 0);
}

/**
 * @tc.name  : Test OnDeviceClassChange device switch with hdiFramePosition.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_030g
 * @tc.desc  : Verify both device class switch and hdiFramePosition accumulation are applied together.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_030g, TestSize.Level1)
{
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);

    unit->deviceClass_ = "remote_offload";
    unit->lastFramePosition_ = 1000;
    unit->lastHdiFramePosition_ = 2000;
    uint32_t sampleRate = unit->usedSampleRate_;

    AudioCallBackStreamInfo info = {
        .deviceClass = "offload",
        .framePosition = 5000,
        .hdiFramePosition = 500000
    };
    unit->OnDeviceClassChange(info);
    // remote_offload -> non-offload: no duration accumulation, only framePosition replace
    // hdiFramePosition accumulation
    uint64_t expectedHdi = 2000 + static_cast<uint64_t>(500000) * sampleRate / AUDIO_US_PER_S;
    EXPECT_EQ(unit->lastHdiFramePosition_, expectedHdi);
    EXPECT_EQ(unit->lastFramePosition_, 5000);
}

/**
 * @tc.name  : Test OnDeviceClassChange remote_offload to normal with rewindFramePosition.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_030h
 * @tc.desc  : Verify rewindFramePosition_ is consumed when switching from remote_offload to non-offload.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_030h, TestSize.Level1)
{
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);

    unit->deviceClass_ = "remote_offload";
    unit->lastFramePosition_ = 2000;
    unit->lastHdiFramePosition_ = 5000;
    uint32_t sampleRate = unit->usedSampleRate_;

    AudioCallBackStreamInfo info1 = {
        .deviceClass = "remote_offload",
        .framePosition = 2000,
        .hdiFramePosition = 1000000,
        .rewindTime = 500000
    };
    unit->OnDeviceClassChange(info1);
    uint64_t rewindFrames = static_cast<uint64_t>(500000) * sampleRate / AUDIO_US_PER_S;
    uint64_t expectedHdiAfterFirst = 5000 + static_cast<uint64_t>(1000000) * sampleRate / AUDIO_US_PER_S;
    EXPECT_EQ(unit->lastHdiFramePosition_, expectedHdiAfterFirst);
    EXPECT_EQ(unit->rewindFramePosition_, rewindFrames);

    AudioCallBackStreamInfo info2 = {
        .deviceClass = "offload",
        .framePosition = 8000,
        .hdiFramePosition = 0,
        .rewindTime = 0
    };
    unit->OnDeviceClassChange(info2);
    uint64_t expectedHdiAfterSecond = expectedHdiAfterFirst + rewindFrames;
    EXPECT_EQ(unit->lastHdiFramePosition_, expectedHdiAfterSecond);
    EXPECT_EQ(unit->rewindFramePosition_, 0);
    EXPECT_EQ(unit->lastFramePosition_, 8000);
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
    std::string deviceName = "temp";

    auto unit = std::make_shared<HpaeRendererStreamImpl>(processConfig, 0, deviceName, 1); // callback mode
 
    AudioCallBackStreamInfo info = {
        .needData = true
    };

    EXPECT_NE(unit->OnStreamData(info), SUCCESS); // writecallback nullptr

    info.needData = false;
    EXPECT_EQ(unit->OnStreamData(info), SUCCESS);

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
    std::string deviceName = "temp";

    auto unit = std::make_shared<HpaeRendererStreamImpl>(processConfig, 0, deviceName, 0);
    unit->writeCallback_ = mockWriteCallback;
    EXPECT_EQ(unit->OnQueryUnderrun(), false); // not callback mode, return false

    unit = std::make_shared<HpaeRendererStreamImpl>(processConfig, 0, deviceName, 1);
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
    std::string deviceName = "temp";
    auto unit = std::make_shared<HpaeRendererStreamImpl>(processConfig, 0, deviceName, 1); // callback mode
 
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

// HPAE first-frame-consumed fallback (plan §6.2/§6.3/§8.2). Verifies the once-gate
// lifecycle and the three trigger rules under callback mode.
// Field order in AudioCallBackStreamInfo designated initializers below follows the
// struct declaration order (needData, forceData, ..., isPullData) for -Werror safety.

// Case 1: OnStreamData before STARTED must not notify (gate not armed).
HWTEST_F(HpaeRendererStreamUnitTest, FirstFrameConsumed_BeforeStarted_NoNotify, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    std::string deviceName = "temp";
    auto unit = std::make_shared<HpaeRendererStreamImpl>(processConfig, 0, deviceName, 1); // callback mode
    auto mockCb = std::make_shared<MockStatusCallback>();
    unit->statusCallback_ = mockCb;

    EXPECT_CALL(*mockCb, OnStatusUpdate(OPERATION_FIRST_FRAME_CONSUMED)).Times(0);
    AudioCallBackStreamInfo info = { .needData = false, .isPullData = true }; // history consumption
    EXPECT_EQ(unit->OnStreamData(info), SUCCESS);
}

// Case 2: after STARTED, first history-buffer consumption notifies exactly once; a
// second call does not repeat.
HWTEST_F(HpaeRendererStreamUnitTest, FirstFrameConsumed_HistoryAfterStarted_Once, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    std::string deviceName = "temp";
    auto unit = std::make_shared<HpaeRendererStreamImpl>(processConfig, 0, deviceName, 1);
    auto mockCb = std::make_shared<MockStatusCallback>();
    unit->statusCallback_ = mockCb;

    EXPECT_CALL(*mockCb, OnStatusUpdate(OPERATION_STARTED)).Times(1);
    EXPECT_CALL(*mockCb, OnStatusUpdate(OPERATION_FIRST_FRAME_CONSUMED)).Times(1); // exactly once

    AudioCallBackStreamInfo info = { .needData = false, .isPullData = true };
    unit->OnStatusUpdate(OPERATION_STARTED, 0); // arm
    EXPECT_EQ(unit->OnStreamData(info), SUCCESS); // notify
    EXPECT_EQ(unit->OnStreamData(info), SUCCESS); // pending consumed -> no repeat
}

// Case 3: after STARTED, a successful non-zero client fetch notifies once.
HWTEST_F(HpaeRendererStreamUnitTest, FirstFrameConsumed_NonZeroDataAfterStarted_Once, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    std::string deviceName = "temp";
    auto unit = std::make_shared<HpaeRendererStreamImpl>(processConfig, 0, deviceName, 1);
    auto mockCb = std::make_shared<MockStatusCallback>();
    unit->statusCallback_ = mockCb;
    auto mockWriteCb = std::make_shared<MockWriteCallback>();
    unit->writeCallback_ = mockWriteCb;

    EXPECT_CALL(*mockCb, OnStatusUpdate(OPERATION_STARTED)).Times(1);
    unit->OnStatusUpdate(OPERATION_STARTED, 0); // arm

    std::vector<int8_t> buffer(2048, 1);
    AudioCallBackStreamInfo info = {
        .inputData = buffer.data(), .requestDataLen = 1024, .needData = true, .isPullData = true,
    };
    EXPECT_CALL(*mockWriteCb, GetAvailableSize(::testing::_))
        .WillOnce(DoAll(SetArgReferee<0>(1024), Return(0)));
    EXPECT_CALL(*mockWriteCb, OnWriteData(::testing::_, ::testing::_)).WillOnce(Return(0));
    EXPECT_CALL(*mockCb, OnStatusUpdate(OPERATION_FIRST_FRAME_CONSUMED)).Times(1);
    EXPECT_EQ(unit->OnStreamData(info), SUCCESS);
}

// Case 4: fetch failure, zero-length and mute-only padding must not notify.
HWTEST_F(HpaeRendererStreamUnitTest, FirstFrameConsumed_FetchFailureAndZeroLength_NoNotify, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    std::string deviceName = "temp";
    auto unit = std::make_shared<HpaeRendererStreamImpl>(processConfig, 0, deviceName, 1);
    auto mockCb = std::make_shared<MockStatusCallback>();
    unit->statusCallback_ = mockCb;
    auto mockWriteCb = std::make_shared<MockWriteCallback>();
    unit->writeCallback_ = mockWriteCb;

    EXPECT_CALL(*mockCb, OnStatusUpdate(OPERATION_STARTED)).Times(1);
    unit->OnStatusUpdate(OPERATION_STARTED, 0); // arm

    std::vector<int8_t> buffer(2048, 1);

    // (a) OnWriteData fails -> return before the notify, no first-frame.
    AudioCallBackStreamInfo infoFail = {
        .inputData = buffer.data(), .requestDataLen = 1024, .needData = true, .isPullData = true,
    };
    EXPECT_CALL(*mockWriteCb, GetAvailableSize(::testing::_))
        .WillOnce(DoAll(SetArgReferee<0>(1024), Return(0)));
    EXPECT_CALL(*mockWriteCb, OnWriteData(::testing::_, ::testing::_)).WillOnce(Return(-1));
    EXPECT_CALL(*mockCb, OnStatusUpdate(OPERATION_FIRST_FRAME_CONSUMED)).Times(0);
    EXPECT_NE(unit->OnStreamData(infoFail), SUCCESS);

    // (b) mute-only padding (available 0, forceData false) -> zero delivered length, no notify.
    AudioCallBackStreamInfo infoZero = {
        .inputData = buffer.data(), .requestDataLen = 1024, .needData = true, .forceData = false,
        .isPullData = true,
    };
    EXPECT_CALL(*mockWriteCb, GetAvailableSize(::testing::_)).WillOnce(DoAll(SetArgReferee<0>(0), Return(0)));
    EXPECT_CALL(*mockWriteCb, OnWriteData(::testing::_, ::testing::_)).WillOnce(Return(0));
    EXPECT_CALL(*mockCb, OnStatusUpdate(OPERATION_FIRST_FRAME_CONSUMED)).Times(0);
    EXPECT_EQ(unit->OnStreamData(infoZero), SUCCESS);
}

// Case 5: isPullData == false (info-sync call) must not notify even on a successful fetch.
HWTEST_F(HpaeRendererStreamUnitTest, FirstFrameConsumed_IsPullDataFalse_NoNotify, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    std::string deviceName = "temp";
    auto unit = std::make_shared<HpaeRendererStreamImpl>(processConfig, 0, deviceName, 1);
    auto mockCb = std::make_shared<MockStatusCallback>();
    unit->statusCallback_ = mockCb;
    auto mockWriteCb = std::make_shared<MockWriteCallback>();
    unit->writeCallback_ = mockWriteCb;

    EXPECT_CALL(*mockCb, OnStatusUpdate(OPERATION_STARTED)).Times(1);
    unit->OnStatusUpdate(OPERATION_STARTED, 0); // arm

    std::vector<int8_t> buffer(2048, 1);
    AudioCallBackStreamInfo info = {
        .inputData = buffer.data(), .requestDataLen = 1024, .needData = true, .isPullData = false,
    };
    EXPECT_CALL(*mockWriteCb, GetAvailableSize(::testing::_))
        .WillOnce(DoAll(SetArgReferee<0>(1024), Return(0)));
    EXPECT_CALL(*mockWriteCb, OnWriteData(::testing::_, ::testing::_)).WillOnce(Return(0));
    EXPECT_CALL(*mockCb, OnStatusUpdate(OPERATION_FIRST_FRAME_CONSUMED)).Times(0);
    EXPECT_EQ(unit->OnStreamData(info), SUCCESS);
}

// Case 6: after STOPPED (disarm), residual OnStreamData must not notify. PAUSED / RELEASED
// share the same disarm branch in OnStatusUpdate.
HWTEST_F(HpaeRendererStreamUnitTest, FirstFrameConsumed_AfterStop_NoNotify, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    std::string deviceName = "temp";
    auto unit = std::make_shared<HpaeRendererStreamImpl>(processConfig, 0, deviceName, 1);
    auto mockCb = std::make_shared<MockStatusCallback>();
    unit->statusCallback_ = mockCb;

    EXPECT_CALL(*mockCb, OnStatusUpdate(OPERATION_STARTED)).Times(1);
    EXPECT_CALL(*mockCb, OnStatusUpdate(OPERATION_STOPPED)).Times(1);
    unit->OnStatusUpdate(OPERATION_STARTED, 0); // arm
    unit->OnStatusUpdate(OPERATION_STOPPED, 0); // disarm

    EXPECT_CALL(*mockCb, OnStatusUpdate(OPERATION_FIRST_FRAME_CONSUMED)).Times(0);
    AudioCallBackStreamInfo info = { .needData = false, .isPullData = true };
    EXPECT_EQ(unit->OnStreamData(info), SUCCESS);
}

// Case 7: after Stop, a new STARTED re-arms and the fallback can notify once again.
HWTEST_F(HpaeRendererStreamUnitTest, FirstFrameConsumed_RestartAfterStop_NotifiesAgain, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    std::string deviceName = "temp";
    auto unit = std::make_shared<HpaeRendererStreamImpl>(processConfig, 0, deviceName, 1);
    auto mockCb = std::make_shared<MockStatusCallback>();
    unit->statusCallback_ = mockCb;

    EXPECT_CALL(*mockCb, OnStatusUpdate(OPERATION_STARTED)).Times(2);
    EXPECT_CALL(*mockCb, OnStatusUpdate(OPERATION_STOPPED)).Times(1);
    EXPECT_CALL(*mockCb, OnStatusUpdate(OPERATION_FIRST_FRAME_CONSUMED)).Times(2); // one per cycle

    AudioCallBackStreamInfo info = { .needData = false, .isPullData = true };
    unit->OnStatusUpdate(OPERATION_STARTED, 0); // cycle 1 arm
    EXPECT_EQ(unit->OnStreamData(info), SUCCESS); // notify #1
    unit->OnStatusUpdate(OPERATION_STOPPED, 0); // disarm
    unit->OnStatusUpdate(OPERATION_STARTED, 0); // cycle 2 arm
    EXPECT_EQ(unit->OnStreamData(info), SUCCESS); // notify #2
}

// Case 8: an expired statusCallback_ must not crash and must not consume the pending
// gate; a fresh callback can still receive the notification afterwards.
HWTEST_F(HpaeRendererStreamUnitTest, FirstFrameConsumed_StatusCallbackExpired_NoConsume, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    std::string deviceName = "temp";
    auto unit = std::make_shared<HpaeRendererStreamImpl>(processConfig, 0, deviceName, 1);
    auto mockCb = std::make_shared<MockStatusCallback>();
    unit->statusCallback_ = mockCb;

    EXPECT_CALL(*mockCb, OnStatusUpdate(OPERATION_STARTED)).Times(1);
    unit->OnStatusUpdate(OPERATION_STARTED, 0); // arm

    unit->statusCallback_.reset(); // weak ptr now dangles
    AudioCallBackStreamInfo info = { .needData = false, .isPullData = true };
    unit->OnStreamData(info); // lock() null -> return WITHOUT consuming pending (no crash)

    // Pending survived: a newly installed callback still gets exactly one notification.
    auto mockCb2 = std::make_shared<MockStatusCallback>();
    unit->statusCallback_ = mockCb2;
    EXPECT_CALL(*mockCb2, OnStatusUpdate(OPERATION_FIRST_FRAME_CONSUMED)).Times(1);
    EXPECT_EQ(unit->OnStreamData(info), SUCCESS);
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

/**
 * @tc.name  : Test GetSpeedPosition.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_039
 * @tc.desc  : Test GetCurrentPosition with offload_inactive_background.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_039, TestSize.Level1)
{
    adapterManager = std::make_shared<HpaeAdapterManager>(PLAYBACK);
    AudioProcessConfig processConfig;
    processConfig.streamInfo.customSampleRate = SAMPLE_RATE_16010;
    std::string deviceName = "";
    std::shared_ptr<IRendererStream> rendererStream = adapterManager->CreateRendererStream(processConfig, deviceName);
    std::shared_ptr<HpaeRendererStreamImpl> rendererStreamImpl =
        std::static_pointer_cast<HpaeRendererStreamImpl>(rendererStream);
    EXPECT_NE(rendererStreamImpl, nullptr);
    rendererStreamImpl->offloadStatePolicy_.store(OFFLOAD_INACTIVE_BACKGROUND);
    uint64_t framePosition = 1;
    uint64_t timestamp = 1;
    uint64_t latency = 1;
    EXPECT_EQ(rendererStreamImpl->GetCurrentPosition(framePosition, timestamp, latency, 0), SUCCESS);
    uint64_t framePosition2 = 2;
    uint64_t timestamp2 = 2;
    uint64_t latency2 = 2;
    EXPECT_EQ(rendererStreamImpl->GetCurrentPosition(framePosition2, timestamp2, latency2, 0), SUCCESS);
    EXPECT_EQ(framePosition, framePosition2);
    EXPECT_EQ(latency, latency2);
}

/**
 * @tc.name  : Test GetSpeedPosition.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_040
 * @tc.desc  : Test GetCurrentPosition with offload_inactive_background.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_040, TestSize.Level1)
{
    adapterManager = std::make_shared<HpaeAdapterManager>(PLAYBACK);
    AudioProcessConfig processConfig;
    processConfig.streamInfo.customSampleRate = SAMPLE_RATE_16010;
    std::string deviceName = "";
    std::shared_ptr<IRendererStream> rendererStream = adapterManager->CreateRendererStream(processConfig, deviceName);
    std::shared_ptr<HpaeRendererStreamImpl> rendererStreamImpl =
        std::static_pointer_cast<HpaeRendererStreamImpl>(rendererStream);
    EXPECT_NE(rendererStreamImpl, nullptr);
    rendererStreamImpl->offloadStatePolicy_.store(OFFLOAD_INACTIVE_BACKGROUND);
    uint64_t framePosition = 1;
    uint64_t timestamp = 1;
    uint64_t latency = 1;
    EXPECT_EQ(rendererStreamImpl->GetSpeedPosition(framePosition, timestamp, latency, 0), SUCCESS);
    uint64_t framePosition2 = 2;
    uint64_t timestamp2 = 2;
    uint64_t latency2 = 2;
    EXPECT_EQ(rendererStreamImpl->GetSpeedPosition(framePosition2, timestamp2, latency2, 0), SUCCESS);
    EXPECT_EQ(framePosition, framePosition2);
    EXPECT_EQ(latency, latency2);
}

/**
 * @tc.name  : Test WriteDataFromRingBuffer preBuf not ready.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_041
 * @tc.desc  : Test WriteDataFromRingBuffer preBuf not ready.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_041, TestSize.Level1)
{
    AudioProcessConfig processConfig = GetInnerCapConfig();
    constexpr size_t preBufSize = 8;
    std::string deviceName = "temp";
    auto unit = std::make_shared<HpaeRendererStreamImpl>(processConfig, 0, deviceName, 0, preBufSize); // write mode
    EXPECT_NE(unit, nullptr);

    unit->ringBuffer_ = AudioRingCache::Create(preBufSize * 2);
    std::vector<int8_t> inBuffer(preBufSize, 1);
    unit->ringBuffer_->Enqueue({reinterpret_cast<uint8_t *>(inBuffer.data()), preBufSize - 1});

    size_t requestDataLen = preBufSize - 1;
    std::vector<int8_t> outBuffer(preBufSize, 0);
    int32_t ret = unit->WriteDataFromRingBuffer(true, outBuffer.data(), requestDataLen);
    EXPECT_EQ(ret, ERROR);
    EXPECT_FALSE(unit->preBufDone_);
}

/**
 * @tc.name  : Test WriteDataFromRingBuffer preBuf ready.
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_042
 * @tc.desc  : Test WriteDataFromRingBuffer preBuf ready.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_042, TestSize.Level1)
{
    AudioProcessConfig processConfig = GetInnerCapConfig();
    constexpr size_t preBufSize = 8;
    std::string deviceName = "temp";
    auto unit = std::make_shared<HpaeRendererStreamImpl>(processConfig, 0, deviceName, 0, preBufSize); // write mode
    EXPECT_NE(unit, nullptr);

    unit->ringBuffer_ = AudioRingCache::Create(preBufSize * 2);
    std::vector<int8_t> inBuffer(preBufSize, 1);
    unit->ringBuffer_->Enqueue({reinterpret_cast<uint8_t *>(inBuffer.data()), preBufSize});

    size_t requestDataLen = preBufSize;
    std::vector<int8_t> outBuffer(preBufSize, 0);
    int32_t ret = unit->WriteDataFromRingBuffer(true, outBuffer.data(), requestDataLen);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(unit->preBufDone_);
}

/**
 * @tc.name  : Test SetOffloadMode with statusCallback
 * @tc.type  : FUNC
 * @tc.number: SetOffloadMode_001
 * @tc.desc  : Test SetOffloadMode when statusCallback != nullptr
 */
HWTEST_F(HpaeRendererStreamUnitTest, SetOffloadMode_001, TestSize.Level1)
{
    AudioProcessConfig processConfig = GetInnerCapConfig();
    std::string deviceName = "temp";
    auto unit = std::make_shared<HpaeRendererStreamImpl>(processConfig, 0, deviceName, 0);
    EXPECT_NE(unit, nullptr);

    // Create a mock status callback
    class MockStatusCallback : public IStatusCallback {
    public:
        MOCK_METHOD(void, OnStatusUpdate, (IOperation operation), (override));
        virtual ~MockStatusCallback() = default;
    };
    auto mockCallback = std::make_shared<MockStatusCallback>();
    unit->statusCallback_ = mockCallback;

    // Set offload mode with isAppBack = true
    EXPECT_CALL(*mockCallback, OnStatusUpdate(::testing::_))
        .Times(::testing::AtLeast(1));
    int32_t ret = unit->SetOffloadMode(1, true); // state = 1, isAppBack = true
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetOffloadMode without statusCallback
 * @tc.type  : FUNC
 * @tc.number: SetOffloadMode_002
 * @tc.desc  : Test SetOffloadMode when statusCallback is nullptr
 */
HWTEST_F(HpaeRendererStreamUnitTest, SetOffloadMode_002, TestSize.Level1)
{
    AudioProcessConfig processConfig = GetInnerCapConfig();
    std::string deviceName = "temp";
    auto unit = std::make_shared<HpaeRendererStreamImpl>(processConfig, 0, deviceName, 0);
    EXPECT_NE(unit, nullptr);

    // Set statusCallback to empty weak_ptr
    unit->statusCallback_.reset();

    // Set offload mode - should not crash
    int32_t ret = unit->SetOffloadMode(1, true);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test HpaeRendererStreamImpl FAST flag span size
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_FAST_SpanSize_001
 * @tc.desc  : Test HpaeRendererStreamImpl with AUDIO_OUTPUT_FLAG_FAST for 5ms span.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_FAST_SpanSize_001, TestSize.Level1)
{
    AudioProcessConfig processConfig = GetInnerCapConfig();
    processConfig.rendererInfo.audioFlag = AUDIO_OUTPUT_FLAG_FAST;
    processConfig.isInnerCapturer = false;
    processConfig.streamInfo.samplingRate = SAMPLE_RATE_48000;
    
    std::string deviceName = "temp";
    HpaeRendererStreamImpl rendererStreamImpl(processConfig, true, deviceName);
    size_t expectedSpanSize = 5 * processConfig.streamInfo.samplingRate / AUDIO_MS_PER_S;
    EXPECT_EQ(rendererStreamImpl.spanSizeInFrame_, expectedSpanSize);
}

/**
 * @tc.name  : Test HpaeRendererStreamImpl fast stream on ultra_fast pipe span size
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_UltraFastPipe_SpanSize_001
 * @tc.desc  : Test fast stream (ULTRA_IMPLEMENTED only) on ultra_fast pipe gets 2.5ms span.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_UltraFastPipe_SpanSize_001, TestSize.Level1)
{
    AudioProcessConfig processConfig = GetInnerCapConfig();
    processConfig.rendererInfo.audioFlag = AUDIO_OUTPUT_FLAG_FAST;
    processConfig.ultraFastFlag = ULTRA_IMPLEMENTED;
    processConfig.isInnerCapturer = false;
    processConfig.streamInfo.samplingRate = SAMPLE_RATE_48000;

    std::string deviceName = "temp";
    HpaeRendererStreamImpl rendererStreamImpl(processConfig, true, deviceName);
    size_t expectedSpanSize = static_cast<size_t>(2500 * processConfig.streamInfo.samplingRate / AUDIO_MS_PER_S / 1000);
    EXPECT_EQ(rendererStreamImpl.spanSizeInFrame_, expectedSpanSize);
}

/**
 * @tc.name  : Test HpaeRendererStreamImpl ultra_fast stream span size
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_UltraFast_SpanSize_001
 * @tc.desc  : Test ultra_fast stream (ULTRA_REQUESTED | ULTRA_IMPLEMENTED) gets 2.5ms span.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_UltraFast_SpanSize_001, TestSize.Level1)
{
    AudioProcessConfig processConfig = GetInnerCapConfig();
    processConfig.rendererInfo.audioFlag = AUDIO_OUTPUT_FLAG_FAST;
    processConfig.ultraFastFlag = ULTRA_REQUESTED | ULTRA_IMPLEMENTED;
    processConfig.isInnerCapturer = false;
    processConfig.streamInfo.samplingRate = SAMPLE_RATE_48000;

    std::string deviceName = "temp";
    HpaeRendererStreamImpl rendererStreamImpl(processConfig, true, deviceName);
    size_t expectedSpanSize = static_cast<size_t>(2500 * processConfig.streamInfo.samplingRate / AUDIO_MS_PER_S / 1000);
    EXPECT_EQ(rendererStreamImpl.spanSizeInFrame_, expectedSpanSize);
}

/**
 * @tc.name  : Test HpaeRendererStreamImpl ultra requested but not implemented span size
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_UltraRequested_SpanSize_001
 * @tc.desc  : Test stream with ULTRA_REQUESTED only (not implemented, fast pipe) gets 5ms span.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_UltraRequested_SpanSize_001, TestSize.Level1)
{
    AudioProcessConfig processConfig = GetInnerCapConfig();
    processConfig.rendererInfo.audioFlag = AUDIO_OUTPUT_FLAG_FAST;
    processConfig.ultraFastFlag = ULTRA_REQUESTED;
    processConfig.isInnerCapturer = false;
    processConfig.streamInfo.samplingRate = SAMPLE_RATE_48000;

    std::string deviceName = "temp";
    HpaeRendererStreamImpl rendererStreamImpl(processConfig, true, deviceName);
    size_t expectedSpanSize = 5 * processConfig.streamInfo.samplingRate / AUDIO_MS_PER_S;
    EXPECT_EQ(rendererStreamImpl.spanSizeInFrame_, expectedSpanSize);
}

/**
 * @tc.name  : Test HpaeRendererStreamImpl VOIP_FAST delayedWake
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_VOIP_FAST_DelayedWake_001
 * @tc.desc  : Test HpaeRendererStreamImpl with VOIP_FAST flag sets delayedWake.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_VOIP_FAST_DelayedWake_001, TestSize.Level1)
{
    AudioProcessConfig processConfig = GetInnerCapConfig();
    processConfig.rendererInfo.audioFlag = AUDIO_OUTPUT_FLAG_VOIP_FAST;
    processConfig.isInnerCapturer = false;
    
    std::string deviceName = "temp";
    HpaeRendererStreamImpl rendererStreamImpl(processConfig, true, deviceName);
    EXPECT_TRUE(rendererStreamImpl.delayedWake_);
}

/**
 * @tc.name  : Test HpaeRendererStreamImpl ULTRA_FAST delayedWake
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_ULTRA_FAST_DelayedWake_001
 * @tc.desc  : Test HpaeRendererStreamImpl with ultraFastFlag sets delayedWake.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_ULTRA_FAST_DelayedWake_001, TestSize.Level1)
{
    AudioProcessConfig processConfig = GetInnerCapConfig();
    processConfig.rendererInfo.ultraFastFlag = 3;
    processConfig.isInnerCapturer = false;
    
    std::string deviceName = "temp";
    HpaeRendererStreamImpl rendererStreamImpl(processConfig, true, deviceName);
    EXPECT_TRUE(rendererStreamImpl.delayedWake_);
}

/**
 * @tc.name  : Test HpaeRendererStreamImpl inner capturer no delayedWake
 * @tc.type  : FUNC
 * @tc.number: HpaeRenderer_Inner_NoDelayedWake_001
 * @tc.desc  : Test HpaeRendererStreamImpl with inner capturer should not set delayedWake.
 */
HWTEST_F(HpaeRendererStreamUnitTest, HpaeRenderer_Inner_NoDelayedWake_001, TestSize.Level1)
{
    AudioProcessConfig processConfig = GetInnerCapConfig();
    processConfig.rendererInfo.audioFlag = AUDIO_OUTPUT_FLAG_FAST;
    processConfig.isInnerCapturer = true;
    
    std::string deviceName = "temp";
    HpaeRendererStreamImpl rendererStreamImpl(processConfig, true, deviceName);
    EXPECT_FALSE(rendererStreamImpl.delayedWake_);
}

/**
 * @tc.name  : Test GetRouteFlag API
 * @tc.type  : FUNC
 * @tc.number: GetRouteFlag_001
 * @tc.desc  : Test GetRouteFlag returns routeFlag value.
 */
HWTEST_F(HpaeRendererStreamUnitTest, GetRouteFlag_001, TestSize.Level1)
{
    AudioProcessConfig processConfig = GetInnerCapConfig();
    std::string deviceName = "temp";
    auto unit = std::make_shared<HpaeRendererStreamImpl>(processConfig, 0, deviceName, 0);
    EXPECT_NE(unit, nullptr);
    
    unit->routeFlag_.store(123);
    uint32_t routeFlag = unit->GetRouteFlag();
    EXPECT_EQ(routeFlag, 123u);
}

/**
 * @tc.name  : Test TriggerClientWake without writeCallback
 * @tc.type  : FUNC
 * @tc.number: TriggerClientWake_002
 * @tc.desc  : Test TriggerClientWake without writeCallback should not crash.
 */
HWTEST_F(HpaeRendererStreamUnitTest, TriggerClientWake_002, TestSize.Level1)
{
    AudioProcessConfig processConfig = GetInnerCapConfig();
    std::string deviceName = "temp";
    auto unit = std::make_shared<HpaeRendererStreamImpl>(processConfig, 0, deviceName, 0);
    EXPECT_NE(unit, nullptr);
    
    unit->writeCallback_.reset();
    unit->TriggerClientWake();
    EXPECT_NE(unit, nullptr);
}

/**
 * @tc.name  : Test OnStatusUpdate OPERATION_UPDATE_SPANSIZE
 * @tc.type  : FUNC
 * @tc.number: OnStatusUpdate_UpdateSpanSize_001
 * @tc.desc  : Test OnStatusUpdate with OPERATION_UPDATE_SPANSIZE operation.
 */
HWTEST_F(HpaeRendererStreamUnitTest, OnStatusUpdate_UpdateSpanSize_001, TestSize.Level1)
{
    AudioProcessConfig processConfig = GetInnerCapConfig();
    std::string deviceName = "temp";
    auto unit = std::make_shared<HpaeRendererStreamImpl>(processConfig, 0, deviceName, 0);
    EXPECT_NE(unit, nullptr);
    
    unit->OnStatusUpdate(OPERATION_UPDATE_SPANSIZE, 0);
    EXPECT_NE(unit, nullptr);
}

/**
 * @tc.name  : Test UpdateSpanSize unchanged
 * @tc.type  : FUNC
 * @tc.number: UpdateSpanSize_Unchanged_001
 * @tc.desc  : Test UpdateSpanSize when spanSize is unchanged.
 */
HWTEST_F(HpaeRendererStreamUnitTest, UpdateSpanSize_Unchanged_001, TestSize.Level1)
{
    AudioProcessConfig processConfig = GetInnerCapConfig();
    std::string deviceName = "temp";
    auto unit = std::make_shared<HpaeRendererStreamImpl>(processConfig, 0, deviceName, 0);
    EXPECT_NE(unit, nullptr);
    
    uint32_t currentSpanSize = unit->spanSizeInFrame_;
    int32_t ret = unit->UpdateSpanSize(currentSpanSize);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test UpdateSpanSize changed
 * @tc.type  : FUNC
 * @tc.number: UpdateSpanSize_Changed_001
 * @tc.desc  : Test UpdateSpanSize when spanSize changes.
 */
HWTEST_F(HpaeRendererStreamUnitTest, UpdateSpanSize_Changed_001, TestSize.Level1)
{
    AudioProcessConfig processConfig = GetInnerCapConfig();
    std::string deviceName = "temp";
    auto unit = std::make_shared<HpaeRendererStreamImpl>(processConfig, 0, deviceName, 0);
    EXPECT_NE(unit, nullptr);
    
    uint32_t newSpanSize = 960; // 20ms at 48000Hz
    int32_t ret = unit->UpdateSpanSize(newSpanSize);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(unit->spanSizeInFrame_, newSpanSize);
}

/**
 * @tc.name  : Test UpdateSpanSize zero span
 * @tc.type  : FUNC
 * @tc.number: UpdateSpanSize_Zero_001
 * @tc.desc  : Test UpdateSpanSize with zero should return error.
 */
HWTEST_F(HpaeRendererStreamUnitTest, UpdateSpanSize_Zero_001, TestSize.Level1)
{
    AudioProcessConfig processConfig = GetInnerCapConfig();
    std::string deviceName = "temp";
    auto unit = std::make_shared<HpaeRendererStreamImpl>(processConfig, 0, deviceName, 0);
    EXPECT_NE(unit, nullptr);
    
    int32_t ret = unit->UpdateSpanSize(0);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test keepRunning and isLoopback streamInfo
 * @tc.type  : FUNC
 * @tc.number: StreamInfo_LoopbackKeepRunning_001
 * @tc.desc  : Test streamInfo sets keepRunning and isLoopback from processConfig.
 */
HWTEST_F(HpaeRendererStreamUnitTest, StreamInfo_LoopbackKeepRunning_001, TestSize.Level1)
{
    AudioProcessConfig processConfig = GetInnerCapConfig();
    processConfig.rendererInfo.keepRunning = true;
    processConfig.rendererInfo.isLoopback = true;
    
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);
    
    int32_t ret = unit->InitParams("Speaker");
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test Pause with isStandby parameter
 * @tc.type  : FUNC
 * @tc.number: Pause_IsStandby_001
 * @tc.desc  : Test Pause passes isStandby to IHpaeManager.
 */
HWTEST_F(HpaeRendererStreamUnitTest, Pause_IsStandby_001, TestSize.Level1)
{
    AudioProcessConfig processConfig = GetInnerCapConfig();
    auto unit = CreateHpaeRendererStreamImpl();
    EXPECT_NE(unit, nullptr);
    
    int32_t ret = unit->Pause(true);
    EXPECT_EQ(ret, SUCCESS);
    
    ret = unit->Pause(false);
    EXPECT_EQ(ret, SUCCESS);
}
}
}
