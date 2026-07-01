/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "audio_service_log.h"
#include "audio_errors.h"
#include "audio_info.h"
#include "fast_audio_stream.h"

using namespace testing::ext;
using namespace testing;
using namespace std;

namespace OHOS {
namespace AudioStandard {

class TestStreamCallback : public AudioStreamCallback {
public:
    void OnStateChange(const State state, const StateChangeCmdType cmdType = CMD_FROM_CLIENT) override {}
};

class MockAudioProcessInClient : public AudioProcessInClient {
public:
    MOCK_METHOD(int32_t, SaveDataCallback, (const std::shared_ptr<AudioDataCallback> &dataCallback), (override));

    MOCK_METHOD(int32_t, SaveUnderrunCallback,
        (const std::shared_ptr<ClientUnderrunCallBack> &underrunCallback), (override));

    MOCK_METHOD(int32_t, GetBufferDesc, (BufferDesc &bufDesc), (override));
    MOCK_METHOD(int32_t, Enqueue, (const BufferDesc &bufDesc), (override));

    MOCK_METHOD(int32_t, SetVolume, (int32_t vol), (override));
    MOCK_METHOD(int32_t, SetSourceDuration, (int64_t duration), (override));

    MOCK_METHOD(int32_t, Start, (), (override));
    MOCK_METHOD(int32_t, Pause, (bool isFlush), (override));
    MOCK_METHOD(int32_t, Resume, (), (override));
    MOCK_METHOD(int32_t, Stop, (AudioProcessStage stage), (override));
    MOCK_METHOD(int32_t, Release, (bool isSwitchStream), (override));

    MOCK_METHOD(int32_t, GetSessionID, (uint32_t &sessionID), (override));
    MOCK_METHOD(bool, GetAudioTime, (int64_t &framePos, int64_t &sec, int64_t &nanoSec), (override));
    MOCK_METHOD(int32_t, GetBufferSize, (size_t &bufferSize), (override));
    MOCK_METHOD(int32_t, GetFrameCount, (uint32_t &frameCount), (override));
    MOCK_METHOD(int32_t, GetLatency, (uint64_t &latency), (override));

    MOCK_METHOD(int32_t, SetVolume, (float vol), (override));
    MOCK_METHOD(float, GetVolume, (), (override));

    MOCK_METHOD(int32_t, SetDuckVolume, (float vol), (override));
    MOCK_METHOD(float, GetDuckVolume, (), (override));

    MOCK_METHOD(int32_t, SetMute, (bool mute), (override));
    MOCK_METHOD(bool, GetMute, (), (override));
    MOCK_METHOD(int32_t, SetMuteHint, (bool mute), (override));

    MOCK_METHOD(uint32_t, GetUnderflowCount, (), (override));
    MOCK_METHOD(uint32_t, GetOverflowCount, (), (override));
    MOCK_METHOD(void, SetUnderflowCount, (uint32_t underflowCount), (override));
    MOCK_METHOD(void, SetOverflowCount, (uint32_t overflowCount), (override));
    MOCK_METHOD(int32_t, RegisterRemoteDiedCallback, (const std::shared_ptr<RemoteDiedCallback> &callback), (override));
    MOCK_METHOD(int32_t, UnregisterRemoteDiedCallback, (), (override));
    MOCK_METHOD(int64_t, GetFramesWritten, (), (override));
    MOCK_METHOD(int64_t, GetFramesRead, (), (override));

    MOCK_METHOD(void, SetPreferredFrameSize, (int32_t frameSize, bool isRecreate), (override));
    MOCK_METHOD(void, UpdateLatencyTimestamp, (std::string &timestamp, bool isRenderer), (override));

    MOCK_METHOD(int32_t, SetDefaultOutputDevice, (const DeviceType defaultOutputDevice, bool skipForce), (override));
    MOCK_METHOD(int32_t, SetSilentModeAndMixWithOthers, (bool on), (override));

    MOCK_METHOD(void, GetRestoreInfo, (RestoreInfo &restoreInfo), (override));
    MOCK_METHOD(void, SetRestoreInfo, (RestoreInfo &restoreInfo), (override));
    MOCK_METHOD(RestoreStatus, CheckRestoreStatus, (), (override));
    MOCK_METHOD(RestoreStatus, SetRestoreStatus, (RestoreStatus restoreStatus), (override));

    MOCK_METHOD(void, SaveAdjustStreamVolumeInfo,
        (float volume, uint32_t sessionId, std::string adjustTime, uint32_t code), (override));

    MOCK_METHOD(int32_t, RegisterThreadPriority,
        (pid_t tid, const std::string &bundleName, BoostTriggerMethod method, ThreadPriorityConfig threadPriority),
        (override));

    MOCK_METHOD(bool, GetStopFlag, (), (const, override));
    MOCK_METHOD(void, JoinCallbackLoop, (), (override));
    MOCK_METHOD(void, SetAudioHapticsSyncId, (const int32_t &audioHapticsSyncId), (override));
    MOCK_METHOD(bool, IsRestoreNeeded, (), (override));
    MOCK_METHOD(void, SetRebuildFlag, (), (override));
    MOCK_METHOD(void, GetKeepRunning, (bool &keepRunning), (override));

    MOCK_METHOD(int32_t, GetStaticPlayPosition, (StaticBufferInfo &staticBufferInfo), (override));
    MOCK_METHOD(int32_t, SetStaticBufferEventCallback, (std::shared_ptr<StaticBufferEventCallback>), (override));
    MOCK_METHOD(int32_t, SetStaticTriggerRecreateCallback, (std::function<void()> sendStaticRecreateFunc), (override));
    MOCK_METHOD(int32_t, SetLoopTimes, (int64_t bufferLoopTimes), (override));
    MOCK_METHOD(int32_t, SetStaticRenderRate, (AudioRendererRate renderRate), (override));

    MOCK_METHOD(int32_t, SetFirstFrameWritingCallback,
        (const std::shared_ptr<AudioFirstFrameCallback> &callback), (override));
    MOCK_METHOD(void, SetIsFirstFrame, (bool value), (override));
    MOCK_METHOD(int32_t, ResetStaticPlayPosition, (), (override));
    MOCK_METHOD(int32_t, SetPitch, (float pitch), (override));
};

class FastSystemStreamSecondUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}

    void SetUp() override
    {
        int32_t appUid = static_cast<int32_t>(getuid());
        stream_ = make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
        mockProcessClient_ = make_shared<MockAudioProcessInClient>();
        stream_->processClient_ = mockProcessClient_;
    }

    void TearDown() override
    {
        stream_.reset();
        mockProcessClient_.reset();
    }

    shared_ptr<FastAudioStream> stream_ = nullptr;
    shared_ptr<MockAudioProcessInClient> mockProcessClient_ = nullptr;
};

/**
 * @tc.name  : Test SetAudioStreamInfo API - record mode
 * @tc.type  : FUNC
 * @tc.number: SetAudioStreamInfo_003
 * @tc.desc  : Test SetAudioStreamInfo in record mode.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, SetAudioStreamInfo_003, TestSize.Level1)
{
    AUDIO_INFO_LOG("SetAudioStreamInfo_003 start");
    int32_t appUid = static_cast<int32_t>(getuid());
    auto fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_RECORD, appUid);
    std::shared_ptr<AudioClientTracker> proxyObj = nullptr;
    AudioStreamParams info;
    info.format = AudioSampleFormat::SAMPLE_S16LE;
    info.encoding = AudioEncodingType::ENCODING_PCM;
    info.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    info.channels = AudioChannel::MONO;
    info.channelLayout = AudioChannelLayout::CH_LAYOUT_MONO;
    int32_t res = fastAudioStream->SetAudioStreamInfo(info, proxyObj);
    EXPECT_NE(res, SUCCESS);
}

/**
 * @tc.name  : Test StartAudioStream API - pause state
 * @tc.type  : FUNC
 * @tc.number: StartAudioStream_001
 * @tc.desc  : Test StartAudioStream from PAUSED state.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, StartAudioStream_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("StartAudioStream_001 start");
    stream_->state_ = PAUSED;
    EXPECT_CALL(*mockProcessClient_, Resume()).WillOnce(Return(SUCCESS));
    bool result = stream_->StartAudioStream();
    EXPECT_TRUE(result);
}

/**
 * @tc.name  : Test StartAudioStream API - stopped state
 * @tc.type  : FUNC
 * @tc.number: StartAudioStream_002
 * @tc.desc  : Test StartAudioStream from STOPPED state.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, StartAudioStream_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("StartAudioStream_002 start");
    stream_->state_ = STOPPED;
    EXPECT_CALL(*mockProcessClient_, Start()).WillOnce(Return(SUCCESS));
    bool result = stream_->StartAudioStream();
    EXPECT_TRUE(result);
}

/**
 * @tc.name  : Test StartAudioStream API - invalid state
 * @tc.type  : FUNC
 * @tc.number: StartAudioStream_003
 * @tc.desc  : Test StartAudioStream from invalid state.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, StartAudioStream_003, TestSize.Level1)
{
    AUDIO_INFO_LOG("StartAudioStream_003 start");
    stream_->state_ = RUNNING;
    bool result = stream_->StartAudioStream();
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : Test StartAudioStream API - start failure
 * @tc.type  : FUNC
 * @tc.number: StartAudioStream_004
 * @tc.desc  : Test StartAudioStream when process client Start fails.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, StartAudioStream_004, TestSize.Level1)
{
    AUDIO_INFO_LOG("StartAudioStream_004 start");
    stream_->state_ = PREPARED;
    EXPECT_CALL(*mockProcessClient_, Start()).WillOnce(Return(ERROR));
    bool result = stream_->StartAudioStream();
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : Test PauseAudioStream API - failure case
 * @tc.type  : FUNC
 * @tc.number: PauseAudioStream_001
 * @tc.desc  : Test PauseAudioStream when Pause fails.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, PauseAudioStream_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("PauseAudioStream_001 start");
    stream_->state_ = RUNNING;
    EXPECT_CALL(*mockProcessClient_, Pause(false)).WillOnce(Return(ERROR));
    bool result = stream_->PauseAudioStream();
    EXPECT_FALSE(result);
    EXPECT_EQ(stream_->GetState(), RUNNING);
}

/**
 * @tc.name  : Test StopAudioStream API - not running or paused
 * @tc.type  : FUNC
 * @tc.number: StopAudioStream_001
 * @tc.desc  : Test StopAudioStream when state is not RUNNING or PAUSED.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, StopAudioStream_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("StopAudioStream_001 start");
    stream_->state_ = PREPARED;
    bool result = stream_->StopAudioStream();
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : Test StopAudioStream API - stop failure
 * @tc.type  : FUNC
 * @tc.number: StopAudioStream_002
 * @tc.desc  : Test StopAudioStream when Stop fails.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, StopAudioStream_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("StopAudioStream_002 start");
    stream_->state_ = RUNNING;
    EXPECT_CALL(*mockProcessClient_, Stop(AUDIO_PROC_STAGE_STOP)).WillOnce(Return(ERROR));
    bool result = stream_->StopAudioStream();
    EXPECT_FALSE(result);
    EXPECT_EQ(stream_->GetState(), RUNNING);
}

/**
 * @tc.name  : Test ReleaseAudioStream API - from running state
 * @tc.type  : FUNC
 * @tc.number: ReleaseAudioStream_001
 * @tc.desc  : Test ReleaseAudioStream from RUNNING state.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, ReleaseAudioStream_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("ReleaseAudioStream_001 start");
    stream_->state_ = RUNNING;
    EXPECT_CALL(*mockProcessClient_, Stop(AUDIO_PROC_STAGE_STOP)).WillOnce(Return(SUCCESS));
    EXPECT_CALL(*mockProcessClient_, Release(false)).WillOnce(Return(SUCCESS));
    bool result = stream_->ReleaseAudioStream(false);
    EXPECT_TRUE(result);
}

/**
 * @tc.name  : Test ReleaseAudioStream API - already released
 * @tc.type  : FUNC
 * @tc.number: ReleaseAudioStream_002
 * @tc.desc  : Test ReleaseAudioStream when already RELEASED.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, ReleaseAudioStream_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("ReleaseAudioStream_002 start");
    stream_->state_ = RELEASED;
    bool result = stream_->ReleaseAudioStream(false);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : Test ReleaseAudioStream API - null process client
 * @tc.type  : FUNC
 * @tc.number: ReleaseAudioStream_003
 * @tc.desc  : Test ReleaseAudioStream when process client is null.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, ReleaseAudioStream_003, TestSize.Level1)
{
    AUDIO_INFO_LOG("ReleaseAudioStream_003 start");
    stream_->state_ = PREPARED;
    stream_->processClient_ = nullptr;
    bool result = stream_->ReleaseAudioStream(false);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : Test GetAudioTime API - null process client
 * @tc.type  : FUNC
 * @tc.number: GetAudioTime_001
 * @tc.desc  : Test GetAudioTime when process client is null.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, GetAudioTime_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("GetAudioTime_001 start");
    stream_->processClient_ = nullptr;
    int64_t framePosition = 0;
    (void)framePosition;
    Timestamp timestamp;
    bool result = stream_->GetAudioTime(timestamp, Timestamp::MONOTONIC);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : Test GetAudioTime API - invalid base
 * @tc.type  : FUNC
 * @tc.number: GetAudioTime_002
 * @tc.desc  : Test GetAudioTime with invalid timestamp base.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, GetAudioTime_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("GetAudioTime_002 start");
    Timestamp timestamp;
    bool result = stream_->GetAudioTime(timestamp, Timestamp::BOOTTIME);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : Test GetAudioPosition API - success case
 * @tc.type  : FUNC
 * @tc.number: GetAudioPosition_002
 * @tc.desc  : Test GetAudioPosition success.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, GetAudioPosition_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("GetAudioPosition_002 start");
    EXPECT_CALL(*mockProcessClient_, GetAudioTime(_, _, _))
        .WillOnce(DoAll(SetArgReferee<0>(1000),
                    SetArgReferee<1>(1),
                    SetArgReferee<2>(500000000),
                    Return(true)));
    Timestamp timestamp;
    bool result = stream_->GetAudioPosition(timestamp, Timestamp::MONOTONIC);
    EXPECT_TRUE(result);
}

/**
 * @tc.name  : Test GetLatency API - null process client
 * @tc.type  : FUNC
 * @tc.number: GetLatency_001
 * @tc.desc  : Test GetLatency when process client is null.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, GetLatency_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("GetLatency_001 start");
    stream_->processClient_ = nullptr;
    uint64_t latency = 0;
    int32_t result = stream_->GetLatency(latency);
    EXPECT_EQ(result, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test GetLatencyWithFlag API
 * @tc.type  : FUNC
 * @tc.number: GetLatencyWithFlag_001
 * @tc.desc  : Test GetLatencyWithFlag with hardware flag.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, GetLatencyWithFlag_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("GetLatencyWithFlag_001 start");
    uint64_t latency = 0;
    int32_t result = stream_->GetLatencyWithFlag(latency, LATENCY_FLAG_HARDWARE);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_GT(latency, 0);
}

/**
 * @tc.name  : Test GetLatencyWithFlag API
 * @tc.type  : FUNC
 * @tc.number: GetLatencyWithFlag_002
 * @tc.desc  : Test GetLatencyWithFlag with engine flag.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, GetLatencyWithFlag_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("GetLatencyWithFlag_002 start");
    uint64_t latency = 0;
    int32_t result = stream_->GetLatencyWithFlag(latency, LATENCY_FLAG_ENGINE);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_GT(latency, 0);
}

/**
 * @tc.name  : Test GetLatencyWithFlag API
 * @tc.type  : FUNC
 * @tc.number: GetLatencyWithFlag_003
 * @tc.desc  : Test GetLatencyWithFlag with both flags.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, GetLatencyWithFlag_003, TestSize.Level1)
{
    AUDIO_INFO_LOG("GetLatencyWithFlag_003 start");
    uint64_t latency = 0;
    auto flag = static_cast<LatencyFlag>(LATENCY_FLAG_HARDWARE | LATENCY_FLAG_ENGINE);
    int32_t result = stream_->GetLatencyWithFlag(latency, flag);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_GT(latency, 20000ULL);
}

/**
 * @tc.name  : Test SetVolume API - null process client
 * @tc.type  : FUNC
 * @tc.number: SetVolume_002
 * @tc.desc  : Test SetVolume when process client is null.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, SetVolume_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("SetVolume_002 start");
    stream_->processClient_ = nullptr;
    int32_t result = stream_->SetVolume(0.5f);
    EXPECT_EQ(result, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test SetVolume API - process client returns error
 * @tc.type  : FUNC
 * @tc.number: SetVolume_003
 * @tc.desc  : Test SetVolume when process client SetVolume fails.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, SetVolume_003, TestSize.Level1)
{
    AUDIO_INFO_LOG("SetVolume_003 start");
    EXPECT_CALL(*mockProcessClient_, SetVolume(Matcher<float>(0.5f))).WillOnce(Return(ERROR));
    int32_t result = stream_->SetVolume(0.5f);
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name  : Test GetVolume API - null process client
 * @tc.type  : FUNC
 * @tc.number: GetVolume_002
 * @tc.desc  : Test GetVolume when process client is null.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, GetVolume_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("GetVolume_002 start");
    stream_->processClient_ = nullptr;
    float result = stream_->GetVolume();
    EXPECT_EQ(result, 1.0f);
}

/**
 * @tc.name  : Test SetMute API - success case
 * @tc.type  : FUNC
 * @tc.number: SetMute_002
 * @tc.desc  : Test SetMute success.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, SetMute_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("SetMute_002 start");
    EXPECT_CALL(*mockProcessClient_, SetMute(true)).WillOnce(Return(SUCCESS));
    int32_t result = stream_->SetMute(true, StateChangeCmdType::CMD_FROM_CLIENT);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : Test SetMute API - null process client
 * @tc.type  : FUNC
 * @tc.number: SetMute_003
 * @tc.desc  : Test SetMute when process client is null.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, SetMute_003, TestSize.Level1)
{
    AUDIO_INFO_LOG("SetMute_003 start");
    stream_->processClient_ = nullptr;
    int32_t result = stream_->SetMute(true, StateChangeCmdType::CMD_FROM_CLIENT);
    EXPECT_EQ(result, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test GetMute API - null process client
 * @tc.type  : FUNC
 * @tc.number: GetMute_001
 * @tc.desc  : Test GetMute when process client is null.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, GetMute_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("GetMute_001 start");
    stream_->processClient_ = nullptr;
    bool result = stream_->GetMute();
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : Test SetDuckVolume API - null process client
 * @tc.type  : FUNC
 * @tc.number: SetDuckVolume_001
 * @tc.desc  : Test SetDuckVolume when process client is null.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, SetDuckVolume_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("SetDuckVolume_001 start");
    stream_->processClient_ = nullptr;
    int32_t result = stream_->SetDuckVolume(0.5f);
    EXPECT_EQ(result, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test GetDuckVolume API - null process client
 * @tc.type  : FUNC
 * @tc.number: GetDuckVolume_001
 * @tc.desc  : Test GetDuckVolume when process client is null.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, GetDuckVolume_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("GetDuckVolume_001 start");
    stream_->processClient_ = nullptr;
    float result = stream_->GetDuckVolume();
    EXPECT_EQ(result, 1.0f);
}

/**
 * @tc.name  : Test SetRenderMode API - invalid mode
 * @tc.type  : FUNC
 * @tc.number: SetRenderMode_002
 * @tc.desc  : Test SetRenderMode with invalid mode.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, SetRenderMode_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("SetRenderMode_002 start");
    int32_t result = stream_->SetRenderMode(RENDER_MODE_NORMAL);
    EXPECT_EQ(result, ERR_INVALID_OPERATION);
}

/**
 * @tc.name  : Test SetRenderMode API - record mode
 * @tc.type  : FUNC
 * @tc.number: SetRenderMode_003
 * @tc.desc  : Test SetRenderMode in record mode.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, SetRenderMode_003, TestSize.Level1)
{
    AUDIO_INFO_LOG("SetRenderMode_003 start");
    int32_t appUid = static_cast<int32_t>(getuid());
    auto fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_RECORD, appUid);
    int32_t result = fastAudioStream->SetRenderMode(RENDER_MODE_CALLBACK);
    EXPECT_EQ(result, ERR_INVALID_OPERATION);
}

/**
 * @tc.name  : Test SetStreamCallback API - null callback
 * @tc.type  : FUNC
 * @tc.number: SetStreamCallback_001
 * @tc.desc  : Test SetStreamCallback with null callback.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, SetStreamCallback_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("SetStreamCallback_001 start");
    int32_t result = stream_->SetStreamCallback(nullptr);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test SetStreamCallback API - success case
 * @tc.type  : FUNC
 * @tc.number: SetStreamCallback_002
 * @tc.desc  : Test SetStreamCallback success.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, SetStreamCallback_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("SetStreamCallback_002 start");
    auto callback = std::make_shared<TestStreamCallback>();
    int32_t result = stream_->SetStreamCallback(callback);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : Test SetStreamCallback API - prepared state
 * @tc.type  : FUNC
 * @tc.number: SetStreamCallback_003
 * @tc.desc  : Test SetStreamCallback in PREPARED state sends callback event.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, SetStreamCallback_003, TestSize.Level1)
{
    AUDIO_INFO_LOG("SetStreamCallback_003 start");
    auto callback = std::make_shared<TestStreamCallback>();
    stream_->state_ = PREPARED;
    int32_t result = stream_->SetStreamCallback(callback);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : Test GetUnderflowCount API - null process client
 * @tc.type  : FUNC
 * @tc.number: GetUnderflowCount_001
 * @tc.desc  : Test GetUnderflowCount when process client is null.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, GetUnderflowCount_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("GetUnderflowCount_001 start");
    stream_->processClient_ = nullptr;
    uint32_t result = stream_->GetUnderflowCount();
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name  : Test GetOverflowCount API - null process client
 * @tc.type  : FUNC
 * @tc.number: GetOverflowCount_001
 * @tc.desc  : Test GetOverflowCount when process client is null.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, GetOverflowCount_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("GetOverflowCount_001 start");
    stream_->processClient_ = nullptr;
    uint32_t result = stream_->GetOverflowCount();
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name  : Test RegisterRemoteDiedCallback API - null process client
 * @tc.type  : FUNC
 * @tc.number: RegisterRemoteDiedCallback_001
 * @tc.desc  : Test RegisterRemoteDiedCallback when process client is null.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, RegisterRemoteDiedCallback_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("RegisterRemoteDiedCallback_001 start");
    stream_->processClient_ = nullptr;
    int32_t result = stream_->RegisterRemoteDiedCallback(nullptr);
    EXPECT_EQ(result, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test UnregisterRemoteDiedCallback API - null process client
 * @tc.type  : FUNC
 * @tc.number: UnregisterRemoteDiedCallback_001
 * @tc.desc  : Test UnregisterRemoteDiedCallback when process client is null.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, UnregisterRemoteDiedCallback_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("UnregisterRemoteDiedCallback_001 start");
    stream_->processClient_ = nullptr;
    int32_t result = stream_->UnregisterRemoteDiedCallback();
    EXPECT_EQ(result, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test SetDefaultOutputDevice API - success case
 * @tc.type  : FUNC
 * @tc.number: SetDefaultOutputDevice_002
 * @tc.desc  : Test SetDefaultOutputDevice success.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, SetDefaultOutputDevice_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("SetDefaultOutputDevice_002 start");
    EXPECT_CALL(*mockProcessClient_, SetDefaultOutputDevice(_, _)).WillOnce(Return(SUCCESS));
    int32_t result = stream_->SetDefaultOutputDevice(DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : Test SetDefaultOutputDevice API - failure case
 * @tc.type  : FUNC
 * @tc.number: SetDefaultOutputDevice_003
 * @tc.desc  : Test SetDefaultOutputDevice failure.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, SetDefaultOutputDevice_003, TestSize.Level1)
{
    AUDIO_INFO_LOG("SetDefaultOutputDevice_003 start");
    EXPECT_CALL(*mockProcessClient_, SetDefaultOutputDevice(_, _)).WillOnce(Return(ERROR));
    int32_t result = stream_->SetDefaultOutputDevice(DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name  : Test GetDefaultOutputDevice API
 * @tc.type  : FUNC
 * @tc.number: GetDefaultOutputDevice_001
 * @tc.desc  : Test GetDefaultOutputDevice.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, GetDefaultOutputDevice_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("GetDefaultOutputDevice_001 start");
    DeviceType result = stream_->GetDefaultOutputDevice();
    EXPECT_EQ(result, DEVICE_TYPE_INVALID);
}

/**
 * @tc.name  : Test SetGetBundleName API
 * @tc.type  : FUNC
 * @tc.number: SetGetBundleName_001
 * @tc.desc  : Test SetBundleName and GetBundleName.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, SetGetBundleName_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("SetGetBundleName_001 start");
    std::string name = "test_bundle";
    stream_->SetBundleName(name);
    std::string result = stream_->GetBundleName();
    EXPECT_EQ(result, name);
}

/**
 * @tc.name  : Test SetGetCallStartByUserTid API
 * @tc.type  : FUNC
 * @tc.number: SetGetCallStartByUserTid_001
 * @tc.desc  : Test SetCallStartByUserTid and getter.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, SetGetCallStartByUserTid_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("SetGetCallStartByUserTid_001 start");
    pid_t tid = 12345;
    stream_->SetCallStartByUserTid(tid);
    
    std::lock_guard<std::mutex> lock(stream_->lastCallStartByUserTidMutex_);
    EXPECT_EQ(stream_->lastCallStartByUserTid_.has_value(), true);
}

/**
 * @tc.name  : Test FetchDeviceForSplitStream API
 * @tc.type  : FUNC
 * @tc.number: FetchDeviceForSplitStream_001
 * @tc.desc  : Test FetchDeviceForSplitStream.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, FetchDeviceForSplitStream_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("FetchDeviceForSplitStream_001 start");
    stream_->FetchDeviceForSplitStream();
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test SetRebuildFlag API - null process client
 * @tc.type  : FUNC
 * @tc.number: SetRebuildFlag_001
 * @tc.desc  : Test SetRebuildFlag when process client is null.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, SetRebuildFlag_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("SetRebuildFlag_001 start");
    stream_->processClient_ = nullptr;
    int32_t result = stream_->SetRebuildFlag();
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name  : Test SetGetCallbackLoopTid API
 * @tc.type  : FUNC
 * @tc.number: SetGetCallbackLoopTid_001
 * @tc.desc  : Test SetCallbackLoopTid and GetCallbackLoopTid.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, SetGetCallbackLoopTid_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("SetGetCallbackLoopTid_001 start");
    int32_t tid = 54321;
    stream_->SetCallbackLoopTid(tid);
    
    // Give time for the cv to be notified
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    int32_t result = stream_->GetCallbackLoopTid();
    EXPECT_EQ(result, tid);
}

/**
 * @tc.name  : Test ResetCallbackLoopTid API
 * @tc.type  : FUNC
 * @tc.number: ResetCallbackLoopTid_001
 * @tc.desc  : Test ResetCallbackLoopTid.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, ResetCallbackLoopTid_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("ResetCallbackLoopTid_001 start");
    stream_->ResetCallbackLoopTid();
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test GetStopFlag API - null process client
 * @tc.type  : FUNC
 * @tc.number: GetStopFlag_001
 * @tc.desc  : Test GetStopFlag when process client is null.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, GetStopFlag_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("GetStopFlag_001 start");
    stream_->processClient_ = nullptr;
    bool result = stream_->GetStopFlag();
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : Test IsRestoreNeeded API - null process client
 * @tc.type  : FUNC
 * @tc.number: IsRestoreNeeded_001
 * @tc.desc  : Test IsRestoreNeeded when process client is null.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, IsRestoreNeeded_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("IsRestoreNeeded_001 start");
    stream_->processClient_ = nullptr;
    bool result = stream_->IsRestoreNeeded();
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : Test CheckRestoreStatus API - null process client
 * @tc.type  : FUNC
 * @tc.number: CheckRestoreStatus_001
 * @tc.desc  : Test CheckRestoreStatus when process client is null.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, CheckRestoreStatus_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("CheckRestoreStatus_001 start");
    stream_->processClient_ = nullptr;
    RestoreStatus result = stream_->CheckRestoreStatus();
    EXPECT_EQ(result, RESTORE_ERROR);
}

/**
 * @tc.name  : Test SetRestoreStatus API - null process client
 * @tc.type  : FUNC
 * @tc.number: SetRestoreStatus_001
 * @tc.desc  : Test SetRestoreStatus when process client is null.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, SetRestoreStatus_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("SetRestoreStatus_001 start");
    stream_->processClient_ = nullptr;
    RestoreStatus result = stream_->SetRestoreStatus(NO_NEED_FOR_RESTORE);
    EXPECT_EQ(result, RESTORE_ERROR);
}

/**
 * @tc.name  : Test GetSetStaticBufferInfo API
 * @tc.type  : FUNC
 * @tc.number: GetSetStaticBufferInfo_001
 * @tc.desc  : Test SetStaticBufferInfo and GetRestoreInfo.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, GetSetStaticBufferInfo_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("GetSetStaticBufferInfo_001 start");
    // Set static buffer info
    StaticBufferInfo staticBufferInfo;
    staticBufferInfo.bufferSize_ = 1024;
    staticBufferInfo.totalLoopTimes_ = 2;
    
    auto sharedMemory = AudioSharedMemory::CreateFromLocal(1024, "test_static_buffer");
    stream_->SetStaticBufferInfo(staticBufferInfo, sharedMemory);
    
    EXPECT_EQ(stream_->staticBufferInfo_.bufferSize_, 1024);
    EXPECT_EQ(stream_->staticBufferInfo_.totalLoopTimes_, 2);
}

/**
 * @tc.name  : Test SetGetKeepRunning API
 * @tc.type  : FUNC
 * @tc.number: SetGetKeepRunning_001
 * @tc.desc  : Test GetKeepRunning with null process client.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, SetGetKeepRunning_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("SetGetKeepRunning_001 start");
    stream_->processClient_ = nullptr;
    bool keepRunning = false;
    int32_t result = stream_->GetKeepRunning(keepRunning);
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name  : Test SetStaticTriggerRecreateCallback API - null process client
 * @tc.type  : FUNC
 * @tc.number: SetStaticTriggerRecreateCallback_001
 * @tc.desc  : Test SetStaticTriggerRecreateCallback with null process client.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, SetStaticTriggerRecreateCallback_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("SetStaticTriggerRecreateCallback_001 start");
    stream_->processClient_ = nullptr;
    int32_t result = stream_->SetStaticTriggerRecreateCallback(nullptr);
    EXPECT_EQ(result, ERR_NULL_POINTER);
}

/**
 * @tc.name  : Test SetLoopTimes API - null process client
 * @tc.type  : FUNC
 * @tc.number: SetLoopTimes_001
 * @tc.desc  : Test SetLoopTimes with null process client.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, SetLoopTimes_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("SetLoopTimes_001 start");
    stream_->processClient_ = nullptr;
    int32_t result = stream_->SetLoopTimes(3);
    EXPECT_EQ(result, ERR_NULL_POINTER);
}

/**
 * @tc.name  : Test ResetStaticPlayPosition API - not static mode
 * @tc.type  : FUNC
 * @tc.number: ResetStaticPlayPosition_001
 * @tc.desc  : Test ResetStaticPlayPosition when not in static mode.
 */
HWTEST_F(FastSystemStreamSecondUnitTest, ResetStaticPlayPosition_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("ResetStaticPlayPosition_001 start");
    bool result = stream_->ResetStaticPlayPosition();
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : Test GetSetPrivacyType API
 * @tc.type  : FUNC
 * @tc.number: SetPrivacyType_001
 * @tc.desc  : Test SetPrivacyType (not supported).
 */
HWTEST_F(FastSystemStreamSecondUnitTest, SetPrivacyType_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("SetPrivacyType_001 start");
    stream_->SetPrivacyType(PRIVACY_TYPE_PUBLIC);
    EXPECT_TRUE(true);
}

} // namespace AudioStandard
} // namespace OHOS
