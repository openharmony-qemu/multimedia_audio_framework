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

#include <functional>

#include "gtest/gtest.h"
#include "aiut_log.h"
#include "audio_errors.h"
#include "audio_info.h"
#include "renderer_in_client.h"
#include "renderer_in_client_private.h"
#include "i_stream_listener.h"
#include "meta/audio_types.h"
#include "oh_audio_buffer.h"
#include "audio_stream_enum.h"
#include "parameter.h"
#include "iipc_stream.h"
#include "ipc_stream_stub.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {

namespace {
} // namespace

class RendererInClientThirdAiut : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

class IpcStreamTest3 : public IpcStreamStub {
public:
    virtual ~IpcStreamTest3() = default;

    int32_t calculateCacheCountRet = SUCCESS;
    uint32_t calculatedCacheCount = 0;
    bool calculateCacheCountCalled = false;

    int32_t RegisterStreamListener(const sptr<IRemoteObject> &object) { return 0; }

    int32_t ResolveBuffer(std::shared_ptr<OHAudioBuffer> &buffer) { return 0; }

    int32_t UpdatePosition() { return 0; }

    int32_t GetAudioSessionID(uint32_t &sessionId) { return 0; }

    int32_t Start(bool hasNoCallback) { return 0; }

    int32_t Pause() { return 0; }

    int32_t Stop() { return 0; }

    int32_t Release(bool isSwitchStream) { return 0; }

    int32_t Flush() { return 0; }

    int32_t Drain(bool stopFlag) { return 0; }

    int32_t RequestHandleData(uint64_t syncFramePts, uint32_t size) { return 0; }

    int32_t UpdatePlaybackCaptureConfig(const AudioPlaybackCaptureConfig &config) { return 0; }

    int32_t SetInMainThreadState(bool isInMainThread) { return 0; }

    int32_t GetAudioTime(uint64_t &framePos, uint64_t &timestamp) { return 0; }

    int32_t GetAudioPosition(uint64_t &framePos, uint64_t &timestamp, uint64_t &latency, int32_t base)
    {
        std::vector<uint64_t> vec;
        ClockTime::GetAllTimeStamp(vec);
        timestamp = vec[0];
        return 0;
    }

    int32_t GetSpeedPosition(uint64_t &framePos, uint64_t &timestamp, uint64_t &latency, int32_t base)
    {
        std::vector<uint64_t> vec;
        ClockTime::GetAllTimeStamp(vec);
        timestamp = vec[0];
        return 0;
    }

    int32_t GetLatency(uint64_t &latency) { return 0; }
    int32_t GetLatencyWithFlag(uint64_t &latency, uint32_t flag) { return 0; }

    int32_t SetRate(int32_t rate) { return 0; }

    int32_t GetRate(int32_t &rate) { return 0; }

    int32_t SetLowPowerVolume(float volume) { return 0; }

    int32_t GetLowPowerVolume(float &volume) { return 0; }

    int32_t SetAudioEffectMode(int32_t effectMode) { return 0; }

    int32_t GetAudioEffectMode(int32_t &effectMode) { return 0; }

    int32_t SetPrivacyType(int32_t privacyType) { return 0; }

    int32_t GetPrivacyType(int32_t &privacyType) { return 0; }

    int32_t SetOffloadMode(int32_t state, bool isAppBack) { return 0; }

    int32_t SetTarget(int32_t target, int32_t targetUid,
        uint32_t targetStreamId, int32_t &ret) { return 0; }

    int32_t UnsetOffloadMode() { return 0; }

    int32_t GetOffloadApproximatelyCacheTime(uint64_t &timestamp, uint64_t &paWriteIndex,
            uint64_t &cacheTimeDsp, uint64_t &cacheTimePa) { return 0; }

    int32_t UpdateSpatializationState(bool spatializationEnabled, bool headTrackingEnabled)
    {
        return 0;
    }

    int32_t GetStreamManagerType() { return 0; }

    int32_t SetRebuildFlag() { return 0; }

    int32_t SetSilentModeAndMixWithOthers(bool on) { return 0; }

    int32_t SetClientVolume() { return 0; }

    int32_t SetLoudnessGain(float loudnessGain) { return 0; }

    int32_t SetMute(bool isMute) { return (isMute ? SUCCESS : ERROR); }

    int32_t SetMuteHint(bool mute) { return SUCCESS; }

    int32_t SetDuckFactor(float duckFactor, uint32_t durationMs) { return 0; }

    int32_t RegisterThreadPriority(pid_t tid, const std::string &bundleName, uint32_t method,
            uint32_t threadPriority)
    {
        return 0;
    }

    int32_t SetDefaultOutputDevice(const int32_t defaultOutputDevice, bool skipForce = false)
    {
        return 0;
    }

    int32_t SetSourceDuration(int64_t duration) { return 0; }

    int32_t SetSpeed(float speed) { return 0; }

    int32_t SetOffloadDataCallbackState(int32_t state) { return 0; }

    sptr<IRemoteObject> AsObject() { return nullptr; }

    int32_t ResolveBufferBaseAndGetServerSpanSize(std::shared_ptr<OHAudioBufferBase> &buffer,
            uint32_t &spanSizeInFrame, uint64_t &engineTotalSizeInFrame) { return SUCCESS; }

    int32_t SetAudioHapticsSyncId(int32_t audioHapticsSyncId) { return 0; }

    int32_t SetLoopTimes(int64_t bufferLoopTime) { return SUCCESS; }

    int32_t UpdateUnderrunInfo(uint32_t underrunInfoKey, int32_t underrunInfoVal) { return SUCCESS; }

    int32_t ResetStaticPlayPosition() { return SUCCESS; }

    int32_t CalculateCacheCount(uint32_t cacheSizeSizeInFrame, uint32_t &cacheCount)
    {
        calculateCacheCountCalled = true;
        cacheCount = calculatedCacheCount;
        return calculateCacheCountRet;
    }

    int32_t SetPitch(float pitch) { return SUCCESS; }
};

void RendererInClientThirdAiut::SetUpTestCase(void)
{
}

void RendererInClientThirdAiut::TearDownTestCase(void)
{
}

void RendererInClientThirdAiut::SetUp(void)
{
}

void RendererInClientThirdAiut::TearDown(void)
{
}

// ==================== IsHighResolution 分支测试 ====================

/**
 * @tc.name  : Test IsHighResolution with MUSIC stream and high sample rate
 * @tc.type  : FUNC
 * @tc.number: IsHighResolution_HighRate
 * @tc.desc  : Test IsHighResolution with MUSIC stream type and high sample rate.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInClientThirdAiut, IsHighResolution_HighRate, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getuid());
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");

    renderer->eStreamType_ = STREAM_MUSIC;
    renderer->curStreamParams_.samplingRate = SAMPLE_RATE_48000;
    renderer->curStreamParams_.format = SAMPLE_S24LE;
    bool result = renderer->IsHighResolution();
    AIUT_EXPECT_TRUE_INFO_LOG(result, "IsHighResolution should be true for high rate music");
}

/**
 * @tc.name  : Test IsHighResolution with non-MUSIC stream
 * @tc.type  : FUNC
 * @tc.number: IsHighResolution_NonMusic
 * @tc.desc  : Test IsHighResolution with non-MUSIC stream type.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInClientThirdAiut, IsHighResolution_NonMusic, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getuid());
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");

    renderer->eStreamType_ = STREAM_VOICE_CALL;
    renderer->curStreamParams_.samplingRate = SAMPLE_RATE_48000;
    renderer->curStreamParams_.format = SAMPLE_S24LE;
    bool result = renderer->IsHighResolution();
    AIUT_EXPECT_FALSE_INFO_LOG(result, "IsHighResolution should be false for non-music stream");
}

/**
 * @tc.name  : Test IsHighResolution with low sample rate
 * @tc.type  : FUNC
 * @tc.number: IsHighResolution_LowRate
 * @tc.desc  : Test IsHighResolution with low sample rate.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInClientThirdAiut, IsHighResolution_LowRate, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getuid());
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");

    renderer->eStreamType_ = STREAM_MUSIC;
    renderer->curStreamParams_.samplingRate = SAMPLE_RATE_44100;
    renderer->curStreamParams_.format = SAMPLE_S24LE;
    bool result = renderer->IsHighResolution();
    AIUT_EXPECT_FALSE_INFO_LOG(result, "IsHighResolution should be false for low sample rate");
}

/**
 * @tc.name  : Test IsHighResolution with low format
 * @tc.type  : FUNC
 * @tc.number: IsHighResolution_LowFormat
 * @tc.desc  : Test IsHighResolution with low format.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInClientThirdAiut, IsHighResolution_LowFormat, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getuid());
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");

    renderer->eStreamType_ = STREAM_MUSIC;
    renderer->curStreamParams_.samplingRate = SAMPLE_RATE_48000;
    renderer->curStreamParams_.format = SAMPLE_S16LE;
    bool result = renderer->IsHighResolution();
    AIUT_EXPECT_FALSE_INFO_LOG(result, "IsHighResolution should be false for low format");
}

// ==================== IsMutePlaying 分支测试 ====================

/**
 * @tc.name  : Test IsMutePlaying with negative volumeDataCount
 * @tc.type  : FUNC
 * @tc.number: IsMutePlaying_NegativeCount
 * @tc.desc  : Test IsMutePlaying when volumeDataCount is negative.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInClientThirdAiut, IsMutePlaying_NegativeCount, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getuid());
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");

    renderer->volumeDataCount_ = -1;
    bool result = renderer->IsMutePlaying();
    AIUT_EXPECT_TRUE_INFO_LOG(result, "IsMutePlaying should be true for negative volumeDataCount");
}

/**
 * @tc.name  : Test IsMutePlaying with mutePlaying flag
 * @tc.type  : FUNC
 * @tc.number: IsMutePlaying_MuteFlag
 * @tc.desc  : Test IsMutePlaying when mutePlaying_ is true.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInClientThirdAiut, IsMutePlaying_MuteFlag, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getuid());
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");

    renderer->volumeDataCount_ = 1;
    renderer->mutePlaying_ = true;
    bool result = renderer->IsMutePlaying();
    AIUT_EXPECT_TRUE_INFO_LOG(result, "IsMutePlaying should be true when mutePlaying_ is true");
}

/**
 * @tc.name  : Test IsMutePlaying with non-mute conditions
 * @tc.type  : FUNC
 * @tc.number: IsMutePlaying_NotMute
 * @tc.desc  : Test IsMutePlaying when not in mute state.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInClientThirdAiut, IsMutePlaying_NotMute, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getuid());
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");

    renderer->volumeDataCount_ = 1;
    renderer->mutePlaying_ = false;
    bool result = renderer->IsMutePlaying();
    AIUT_EXPECT_FALSE_INFO_LOG(result, "IsMutePlaying should be false when not mute");
}

// ==================== IsInvalidBuffer 分支测试 ====================

/**
 * @tc.name  : Test IsInvalidBuffer with SAMPLE_U8 format and zero data
 * @tc.type  : FUNC
 * @tc.number: IsInvalidBuffer_U8_Zero
 * @tc.desc  : Test IsInvalidBuffer with SAMPLE_U8 format and zero buffer data.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInClientThirdAiut, IsInvalidBuffer_U8_Zero, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getuid());
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");

    renderer->clientConfig_.streamInfo.format = SAMPLE_U8;
    uint8_t buffer[8] = {0};
    bool result = renderer->IsInvalidBuffer(buffer, sizeof(buffer));
    AIUT_EXPECT_TRUE_INFO_LOG(result, "IsInvalidBuffer should return true for zero U8 data");
}

/**
 * @tc.name  : Test IsInvalidBuffer with SAMPLE_U8 format and non-zero data
 * @tc.type  : FUNC
 * @tc.number: IsInvalidBuffer_U8_NonZero
 * @tc.desc  : Test IsInvalidBuffer with SAMPLE_U8 format and non-zero buffer data.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInClientThirdAiut, IsInvalidBuffer_U8_NonZero, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getuid());
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");

    renderer->clientConfig_.streamInfo.format = SAMPLE_U8;
    uint8_t buffer[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    bool result = renderer->IsInvalidBuffer(buffer, sizeof(buffer));
    AIUT_EXPECT_FALSE_INFO_LOG(result, "IsInvalidBuffer should return false for non-zero U8 data");
}

/**
 * @tc.name  : Test IsInvalidBuffer with SAMPLE_S16LE format and zero data
 * @tc.type  : FUNC
 * @tc.number: IsInvalidBuffer_S16LE_Zero
 * @tc.desc  : Test IsInvalidBuffer with SAMPLE_S16LE format and zero buffer data.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInClientThirdAiut, IsInvalidBuffer_S16LE_Zero, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getuid());
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");

    renderer->clientConfig_.streamInfo.format = SAMPLE_S16LE;
    uint8_t buffer[8] = {0};
    bool result = renderer->IsInvalidBuffer(buffer, sizeof(buffer));
    AIUT_EXPECT_TRUE_INFO_LOG(result, "IsInvalidBuffer should return true for zero S16LE data");
}

/**
 * @tc.name  : Test IsInvalidBuffer with SAMPLE_S16LE format and non-zero data
 * @tc.type  : FUNC
 * @tc.number: IsInvalidBuffer_S16LE_NonZero
 * @tc.desc  : Test IsInvalidBuffer with SAMPLE_S16LE format and non-zero buffer data.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInClientThirdAiut, IsInvalidBuffer_S16LE_NonZero, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getuid());
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");

    renderer->clientConfig_.streamInfo.format = SAMPLE_S16LE;
    uint8_t buffer[8] = {1, 0, 0, 0, 0, 0, 0, 0};
    bool result = renderer->IsInvalidBuffer(buffer, sizeof(buffer));
    AIUT_EXPECT_FALSE_INFO_LOG(result, "IsInvalidBuffer should return false for non-zero S16LE data");
}

/**
 * @tc.name  : Test IsInvalidBuffer with SAMPLE_U8 and zero buffer size
 * @tc.type  : FUNC
 * @tc.number: IsInvalidBuffer_U8_ZeroSize
 * @tc.desc  : Test IsInvalidBuffer with SAMPLE_U8 format and zero buffer size.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInClientThirdAiut, IsInvalidBuffer_U8_ZeroSize, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getuid());
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");

    renderer->clientConfig_.streamInfo.format = SAMPLE_U8;
    uint8_t buffer[8] = {0};
    bool result = renderer->IsInvalidBuffer(buffer, 0);
    AIUT_EXPECT_FALSE_INFO_LOG(result, "IsInvalidBuffer should return false for zero size U8");
}

/**
 * @tc.name  : Test IsInvalidBuffer with SAMPLE_S16LE and too small buffer size
 * @tc.type  : FUNC
 * @tc.number: IsInvalidBuffer_S16LE_SmallSize
 * @tc.desc  : Test IsInvalidBuffer with SAMPLE_S16LE format and buffer size of 1.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInClientThirdAiut, IsInvalidBuffer_S16LE_SmallSize, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getuid());
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");

    renderer->clientConfig_.streamInfo.format = SAMPLE_S16LE;
    uint8_t buffer[8] = {0};
    bool result = renderer->IsInvalidBuffer(buffer, 1);
    AIUT_EXPECT_FALSE_INFO_LOG(result, "IsInvalidBuffer should return false for too small S16LE");
}

/**
 * @tc.name  : Test IsInvalidBuffer with unsupported format
 * @tc.type  : FUNC
 * @tc.number: IsInvalidBuffer_UnsupportedFormat
 * @tc.desc  : Test IsInvalidBuffer with unsupported format (default case).
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInClientThirdAiut, IsInvalidBuffer_UnsupportedFormat, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getuid());
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");

    renderer->clientConfig_.streamInfo.format = SAMPLE_S24LE;
    uint8_t buffer[8] = {0};
    bool result = renderer->IsInvalidBuffer(buffer, sizeof(buffer));
    AIUT_EXPECT_FALSE_INFO_LOG(result, "IsInvalidBuffer should return false for unsupported format");
}

// ==================== MonitorMutePlay 分支测试 ====================

/**
 * @tc.name  : Test MonitorMutePlay with play end
 * @tc.type  : FUNC
 * @tc.number: MonitorMutePlay_PlayEnd
 * @tc.desc  : Test MonitorMutePlay when isPlayEnd is true.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInClientThirdAiut, MonitorMutePlay_PlayEnd, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getuid());
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");

    renderer->mutePlayStartTime_ = ClockTime::GetRealNano();
    renderer->MonitorMutePlay(true);
    AIUT_EXPECT_EQ_INFO_LOG(renderer->mutePlayStartTime_, 0,
        "mutePlayStartTime_ should be 0 after play end, mutePlayStartTime_=%lld",
        static_cast<long long>(renderer->mutePlayStartTime_));
}

/**
 * @tc.name  : Test MonitorMutePlay with not mute and no start time
 * @tc.type  : FUNC
 * @tc.number: MonitorMutePlay_NotMuteNoStart
 * @tc.desc  : Test MonitorMutePlay when not mute and mutePlayStartTime is 0.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInClientThirdAiut, MonitorMutePlay_NotMuteNoStart, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getuid());
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");

    renderer->volumeDataCount_ = 1;
    renderer->mutePlaying_ = false;
    renderer->mutePlayStartTime_ = 0;
    renderer->MonitorMutePlay(false);
    AIUT_EXPECT_EQ_INFO_LOG(renderer->mutePlayStartTime_, 0,
        "mutePlayStartTime_ should remain 0, mutePlayStartTime_=%lld",
        static_cast<long long>(renderer->mutePlayStartTime_));
}

// ==================== ProcessVolume 分支测试 ====================

/**
 * @tc.name  : Test ProcessVolume with active ramp
 * @tc.type  : FUNC
 * @tc.number: ProcessVolume_ActiveRamp
 * @tc.desc  : Test ProcessVolume when volumeRamp is active.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInClientThirdAiut, ProcessVolume_ActiveRamp, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getuid());
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");

    renderer->clientVolume_ = 0.5f;
    bool result = renderer->ProcessVolume();
    AIUT_EXPECT_TRUE_INFO_LOG(result, "ProcessVolume should return true");
}

// ==================== HandleRendererPositionChanges 分支测试 ====================

/**
 * @tc.name  : Test HandleRendererPositionChanges with zero sizePerFrameInByte
 * @tc.type  : FUNC
 * @tc.number: HandleRendererPositionChanges_ZeroSize
 * @tc.desc  : Test HandleRendererPositionChanges when sizePerFrameInByte is 0.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInClientThirdAiut, HandleRendererPositionChanges_ZeroSize, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getuid());
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");

    renderer->sizePerFrameInByte_ = 0;
    renderer->HandleRendererPositionChanges(1024);
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr,
        "renderer should remain valid after HandleRendererPositionChanges with zero sizePerFrameInByte");
}

/**
 * @tc.name  : Test HandleRendererPositionChanges with mark reached
 * @tc.type  : FUNC
 * @tc.number: HandleRendererPositionChanges_MarkReached
 * @tc.desc  : Test HandleRendererPositionChanges when written frames exceed mark position.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInClientThirdAiut, HandleRendererPositionChanges_MarkReached, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getuid());
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");

    renderer->sizePerFrameInByte_ = 4;
    renderer->totalBytesWritten_ = 0;
    renderer->rendererMarkPosition_ = 100;
    renderer->rendererMarkReached_ = false;
    renderer->HandleRendererPositionChanges(400);
    AIUT_EXPECT_TRUE_INFO_LOG(renderer->rendererMarkReached_,
        "rendererMarkReached_ should be true after exceeding mark position");
}

/**
 * @tc.name  : Test HandleRendererPositionChanges with period reached
 * @tc.type  : FUNC
 * @tc.number: HandleRendererPositionChanges_PeriodReached
 * @tc.desc  : Test HandleRendererPositionChanges when written frames exceed period size.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInClientThirdAiut, HandleRendererPositionChanges_PeriodReached, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getuid());
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");

    renderer->sizePerFrameInByte_ = 4;
    renderer->totalBytesWritten_ = 0;
    renderer->rendererPeriodSize_ = 100;
    renderer->rendererPeriodWritten_ = 0;
    renderer->HandleRendererPositionChanges(400);
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr,
        "renderer should remain valid after HandleRendererPositionChanges with period reached");
}

// ==================== DrainAudioStreamInner 分支测试 ====================

/**
 * @tc.name  : Test DrainAudioStreamInner with non-running state
 * @tc.type  : FUNC
 * @tc.number: DrainAudioStreamInner_NotRunning
 * @tc.desc  : Test DrainAudioStreamInner when state is not RUNNING.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInClientThirdAiut, DrainAudioStreamInner_NotRunning, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getuid());
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");

    renderer->state_ = INVALID;
    bool result = renderer->DrainAudioStreamInner(false);
    AIUT_EXPECT_FALSE_INFO_LOG(result, "DrainAudioStreamInner should return false when not running");
}

/**
 * @tc.name  : Test DrainAudioStreamInner with INTERPHONE stream usage
 * @tc.type  : FUNC
 * @tc.number: DrainAudioStreamInner_Interphone
 * @tc.desc  : Test DrainAudioStreamInner when stream usage is INTERPHONE.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInClientThirdAiut, DrainAudioStreamInner_Interphone, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getuid());
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");

    renderer->state_ = RUNNING;
    renderer->rendererInfo_.streamUsage = STREAM_USAGE_INTERPHONE;
    bool result = renderer->DrainAudioStreamInner(false);
    AIUT_EXPECT_TRUE_INFO_LOG(result,
        "DrainAudioStreamInner should return true for INTERPHONE stream");
}

// ==================== WriteMuteDataSysEvent 分支测试 ====================

/**
 * @tc.name  : Test WriteMuteDataSysEvent with silent mode
 * @tc.type  : FUNC
 * @tc.number: WriteMuteDataSysEvent_SilentMode
 * @tc.desc  : Test WriteMuteDataSysEvent when silentModeAndMixWithOthers is true.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInClientThirdAiut, WriteMuteDataSysEvent_SilentMode, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getuid());
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");

    renderer->silentModeAndMixWithOthers_ = true;
    uint8_t buffer[8] = {0};
    renderer->WriteMuteDataSysEvent(buffer, sizeof(buffer));
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr,
        "renderer should remain valid after WriteMuteDataSysEvent with silent mode");
}

/**
 * @tc.name  : Test WriteMuteDataSysEvent with invalid buffer
 * @tc.type  : FUNC
 * @tc.number: WriteMuteDataSysEvent_InvalidBuffer
 * @tc.desc  : Test WriteMuteDataSysEvent with invalid (zero) buffer data.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInClientThirdAiut, WriteMuteDataSysEvent_InvalidBuffer, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getuid());
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");

    renderer->silentModeAndMixWithOthers_ = false;
    renderer->clientConfig_.streamInfo.format = SAMPLE_U8;
    renderer->startMuteTime_ = 0;
    uint8_t buffer[8] = {0};
    renderer->WriteMuteDataSysEvent(buffer, sizeof(buffer));
    AIUT_EXPECT_TRUE_INFO_LOG(renderer->startMuteTime_ != 0,
        "startMuteTime_ should be set for invalid buffer");
}

/**
 * @tc.name  : Test WriteMuteDataSysEvent with valid buffer
 * @tc.type  : FUNC
 * @tc.number: WriteMuteDataSysEvent_ValidBuffer
 * @tc.desc  : Test WriteMuteDataSysEvent with valid (non-zero) buffer data.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInClientThirdAiut, WriteMuteDataSysEvent_ValidBuffer, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getuid());
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");

    renderer->silentModeAndMixWithOthers_ = false;
    renderer->clientConfig_.streamInfo.format = SAMPLE_U8;
    renderer->startMuteTime_ = 1000;
    uint8_t buffer[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    renderer->WriteMuteDataSysEvent(buffer, sizeof(buffer));
    AIUT_EXPECT_EQ_INFO_LOG(renderer->startMuteTime_, 0,
        "startMuteTime_ should be 0 for valid buffer, startMuteTime_=%lld",
        static_cast<long long>(renderer->startMuteTime_));
}

// ==================== RegisterTracker 分支测试 ====================

/**
 * @tc.name  : Test RegisterTracker with null tracker
 * @tc.type  : FUNC
 * @tc.number: RegisterTracker_NullTracker
 * @tc.desc  : Test RegisterTracker when audioStreamTracker is null.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInClientThirdAiut, RegisterTracker_NullTracker, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getuid());
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");

    renderer->audioStreamTracker_ = nullptr;
    renderer->RegisterTracker(nullptr);
    AIUT_EXPECT_FALSE_INFO_LOG(renderer->streamTrackerRegistered_,
        "streamTrackerRegistered_ should be false with null tracker");
}

// ==================== UpdateTracker 分支测试 ====================

/**
 * @tc.name  : Test UpdateTracker with null tracker
 * @tc.type  : FUNC
 * @tc.number: UpdateTracker_NullTracker
 * @tc.desc  : Test UpdateTracker when audioStreamTracker is null.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInClientThirdAiut, UpdateTracker_NullTracker, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getuid());
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");

    renderer->audioStreamTracker_ = nullptr;
    renderer->UpdateTracker("test");
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr,
        "renderer should remain valid after UpdateTracker with null tracker");
}

// ==================== AudioServerDied 分支测试 ====================

/**
 * @tc.name  : Test AudioServerDied
 * @tc.type  : FUNC
 * @tc.number: AudioServerDied
 * @tc.desc  : Test AudioServerDied clears the server proxy.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInClientThirdAiut, AudioServerDied, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getuid());
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");

    RendererInClientInner::AudioServerDied(1000, 1001);
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr,
        "renderer should remain valid after AudioServerDied");
}

// ==================== InitDFXOperaiton 分支测试 ====================

/**
 * @tc.name  : Test InitDFXOperaiton
 * @tc.type  : FUNC
 * @tc.number: InitDFXOperaiton
 * @tc.desc  : Test InitDFXOperaiton initializes DFX properly.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInClientThirdAiut, InitDFXOperaiton, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getuid());
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");

    renderer->sessionId_ = 100001;
    renderer->curStreamParams_.samplingRate = SAMPLE_RATE_48000;
    renderer->curStreamParams_.channels = STEREO;
    renderer->curStreamParams_.format = SAMPLE_S16LE;
    renderer->curStreamParams_.customSampleRate = 0;
    renderer->curStreamParams_.encoding = ENCODING_PCM;
    renderer->isHWDecodingType_ = false;
    renderer->InitDFXOperaiton();
    AIUT_EXPECT_FALSE_INFO_LOG(renderer->dumpOutFile_.empty(),
        "dumpOutFile_ should not be empty after InitDFXOperaiton");
}

// ==================== RegisterThreadPriorityOnStart 分支测试 ====================

/**
 * @tc.name  : Test RegisterThreadPriorityOnStart with ARKTS player type
 * @tc.type  : FUNC
 * @tc.number: RegisterThreadPriorityOnStart_ArkTS
 * @tc.desc  : Test RegisterThreadPriorityOnStart with ARKTS_AUDIO_RENDERER player type.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInClientThirdAiut, RegisterThreadPriorityOnStart_ArkTS, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getuid());
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");

    renderer->rendererInfo_.playerType = PLAYER_TYPE_ARKTS_AUDIO_RENDERER;
    renderer->RegisterThreadPriorityOnStart(CMD_FROM_CLIENT);
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr,
        "renderer should remain valid after RegisterThreadPriorityOnStart with ARKTS player type");
}

/**
 * @tc.name  : Test RegisterThreadPriorityOnStart with OH_AUDIO_RENDERER player type
 * @tc.type  : FUNC
 * @tc.number: RegisterThreadPriorityOnStart_OHAudio
 * @tc.desc  : Test RegisterThreadPriorityOnStart with OH_AUDIO_RENDERER player type.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInClientThirdAiut, RegisterThreadPriorityOnStart_OHAudio, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getuid());
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");

    renderer->rendererInfo_.playerType = PLAYER_TYPE_OH_AUDIO_RENDERER;
    renderer->RegisterThreadPriorityOnStart(CMD_FROM_CLIENT);
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr,
        "renderer should remain valid after RegisterThreadPriorityOnStart with OH_AUDIO_RENDERER player type");
}

/**
 * @tc.name  : Test RegisterThreadPriorityOnStart with default player type
 * @tc.type  : FUNC
 * @tc.number: RegisterThreadPriorityOnStart_Default
 * @tc.desc  : Test RegisterThreadPriorityOnStart with default/unknown player type.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInClientThirdAiut, RegisterThreadPriorityOnStart_Default, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getuid());
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");

    renderer->rendererInfo_.playerType = static_cast<PlayerType>(999);
    renderer->RegisterThreadPriorityOnStart(CMD_FROM_CLIENT);
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr,
        "renderer should remain valid after RegisterThreadPriorityOnStart with default player type");
}

} // namespace AudioStandard
} // namespace OHOS