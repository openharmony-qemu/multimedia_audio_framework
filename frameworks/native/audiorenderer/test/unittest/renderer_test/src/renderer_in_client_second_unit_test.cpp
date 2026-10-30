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

#include <functional>

#include "gtest/gtest.h"
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

const uint64_t TEST_POSITION = 20000;

class RendererInClientSecondUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

class IpcStreamTest : public IpcStreamStub {
public:
    virtual ~IpcStreamTest() = default;

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

    int32_t SetRate(int32_t rate) { return 0; } // SetRenderRate

    int32_t GetRate(int32_t &rate) { return 0; } // SetRenderRate

    int32_t SetLowPowerVolume(float volume) { return 0; } // renderer only

    int32_t GetLowPowerVolume(float &volume) { return 0; } // renderer only

    int32_t SetAudioEffectMode(int32_t effectMode) { return 0; } // renderer only

    int32_t GetAudioEffectMode(int32_t &effectMode) { return 0; } // renderer only

    int32_t SetPrivacyType(int32_t privacyType) { return 0; } // renderer only

    int32_t GetPrivacyType(int32_t &privacyType) { return 0; } // renderer only

    int32_t SetOffloadMode(int32_t state, bool isAppBack) { return 0; } // renderer only

    int32_t SetTarget(int32_t target, int32_t targetUid,
        uint32_t targetStreamId, int32_t &ret) { return 0; } // renderer only

    int32_t UnsetOffloadMode() { return 0; } // renderer only

    int32_t GetOffloadApproximatelyCacheTime(uint64_t &timestamp, uint64_t &paWriteIndex,
            uint64_t &cacheTimeDsp, uint64_t &cacheTimePa) { return 0; } // renderer only

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

    ErrCode SetStandbyState(bool on) { return SUCCESS; }

    sptr<IRemoteObject> AsObject() { return nullptr; }

    int32_t ResolveBufferBaseAndGetServerSpanSize(std::shared_ptr<OHAudioBufferBase> &buffer,
            uint32_t &spanSizeInFrame, uint64_t &engineTotalSizeInFrame) { return SUCCESS; }

    int32_t SetAudioHapticsSyncId(int32_t audioHapticsSyncId) { return 0; }

    int32_t SetLoopTimes(int64_t bufferLoopTimes) { return SUCCESS; }

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

void RendererInClientSecondUnitTest::SetUpTestCase(void)
{
}

void RendererInClientSecondUnitTest::TearDownTestCase(void)
{
}

void RendererInClientSecondUnitTest::SetUp(void)
{
}

void RendererInClientSecondUnitTest::TearDown(void)
{
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: SetAudioStreamInfo_001
 * @tc.desc  : Test SetAudioStreamInfo with invalid params
 */
HWTEST(RendererInClientSecondUnitTest, SetAudioStreamInfo_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    AudioStreamParams info;
    info.samplingRate = 0;
    info.channels = 0;
    info.format = 0;
    info.encoding = 0;

    std::shared_ptr<AudioClientTracker> proxyObj = nullptr;
    auto ret = renderer->SetAudioStreamInfo(info, proxyObj);
    EXPECT_NE(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: SetAudioStreamInfo_002
 * @tc.desc  : Test SetAudioStreamInfo with valid params but null process
 */
HWTEST(RendererInClientSecondUnitTest, SetAudioStreamInfo_002, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getuid());

    ASSERT_TRUE(renderer != nullptr);

    AudioStreamParams info;
    info.samplingRate = static_cast<int32_t>(AudioSamplingRate::SAMPLE_RATE_48000);
    info.channels = static_cast<int32_t>(AudioChannel::STEREO);
    info.format = static_cast<int32_t>(AudioSampleFormat::SAMPLE_S16LE);
    info.encoding = static_cast<int32_t>(AudioEncodingType::ENCODING_PCM);

    std::shared_ptr<AudioClientTracker> proxyObj = nullptr;
    auto ret = renderer->SetAudioStreamInfo(info, proxyObj);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: GetAudioStreamInfo_001
 * @tc.desc  : Test GetAudioStreamInfo
 */
HWTEST(RendererInClientSecondUnitTest, GetAudioStreamInfo_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    AudioStreamParams info;
    auto ret = renderer->GetAudioStreamInfo(info);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: GetAudioSessionID_001
 * @tc.desc  : Test GetAudioSessionID with null ipcStream
 */
HWTEST(RendererInClientSecondUnitTest, GetAudioSessionID_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    uint32_t sessionID = 0;
    auto ret = renderer->GetAudioSessionID(sessionID);
    EXPECT_NE(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: GetBufferSize_001
 * @tc.desc  : Test GetBufferSize with null clientBuffer
 */
HWTEST(RendererInClientSecondUnitTest, GetBufferSize_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    size_t bufferSize = 0;
    auto ret = renderer->GetBufferSize(bufferSize);
    EXPECT_NE(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: GetFrameCount_001
 * @tc.desc  : Test GetFrameCount with null clientBuffer
 */
HWTEST(RendererInClientSecondUnitTest, GetFrameCount_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    uint32_t frameCount = 0;
    auto ret = renderer->GetFrameCount(frameCount);
    EXPECT_NE(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: GetLatency_001
 * @tc.desc  : Test GetLatency with null ipcStream
 */
HWTEST(RendererInClientSecondUnitTest, GetLatency_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    uint64_t latency = 0;
    auto ret = renderer->GetLatency(latency);
    EXPECT_NE(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: SetVolume_001
 * @tc.desc  : Test SetVolume
 */
HWTEST(RendererInClientSecondUnitTest, SetVolume_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    float volume = 0.5f;
    auto ret = renderer->SetVolume(volume);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: GetVolume_001
 * @tc.desc  : Test GetVolume
 */
HWTEST(RendererInClientSecondUnitTest, GetVolume_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    float volume = renderer->GetVolume();
    EXPECT_GE(volume, 0.0f);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: SetLoudnessGain_001
 * @tc.desc  : Test SetLoudnessGain
 */
HWTEST(RendererInClientSecondUnitTest, SetLoudnessGain_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    float loudnessGain = 1.0f;
    auto ret = renderer->SetLoudnessGain(loudnessGain);
    EXPECT_NE(ret, ERROR);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: GetLoudnessGain_001
 * @tc.desc  : Test GetLoudnessGain
 */
HWTEST(RendererInClientSecondUnitTest, GetLoudnessGain_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    float loudnessGain = renderer->GetLoudnessGain();
    EXPECT_EQ(loudnessGain, 0.0f);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: SetDuckVolume_001
 * @tc.desc  : Test SetDuckVolume
 */
HWTEST(RendererInClientSecondUnitTest, SetDuckVolume_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    float volume = 0.5f;
    auto ret = renderer->SetDuckVolume(volume);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: GetDuckVolume_001
 * @tc.desc  : Test GetDuckVolume
 */
HWTEST(RendererInClientSecondUnitTest, GetDuckVolume_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    float volume = renderer->GetDuckVolume();
    EXPECT_GE(volume, 0.0f);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: SetBackMute_001
 * @tc.desc  : Test SetBackMute
 */
HWTEST(RendererInClientSecondUnitTest, SetBackMute_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    bool backMute = true;
    auto ret = renderer->SetBackMute(backMute);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: GetMute_001
 * @tc.desc  : Test GetMute with null ipcStream
 */
HWTEST(RendererInClientSecondUnitTest, GetMute_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    bool mute = renderer->GetMute();
    EXPECT_FALSE(mute);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: SetMuteHint_001
 * @tc.desc  : Test SetMuteHint
 */
HWTEST(RendererInClientSecondUnitTest, SetMuteHint_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    bool mute = true;
    auto ret = renderer->SetMuteHint(mute);
    EXPECT_NE(ret, ERROR);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: SetRenderRate_001
 * @tc.desc  : Test SetRenderRate
 */
HWTEST(RendererInClientSecondUnitTest, SetRenderRate_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    AudioRendererRate renderRate = RENDER_RATE_NORMAL;
    auto ret = renderer->SetRenderRate(renderRate);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: GetRenderRate_001
 * @tc.desc  : Test GetRenderRate
 */
HWTEST(RendererInClientSecondUnitTest, GetRenderRate_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    AudioRendererRate renderRate = renderer->GetRenderRate();
    EXPECT_EQ(renderRate, RENDER_RATE_NORMAL);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: SetAudioStreamType_001
 * @tc.desc  : Test SetAudioStreamType
 */
HWTEST(RendererInClientSecondUnitTest, SetAudioStreamType_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    AudioStreamType audioStreamType = STREAM_DEFAULT;
    auto ret = renderer->SetAudioStreamType(audioStreamType);
    EXPECT_NE(ret, ERR_INVALID_OPERATION);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: SetStreamCallback_001
 * @tc.desc  : Test SetStreamCallback
 */
HWTEST(RendererInClientSecondUnitTest, SetStreamCallback_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    std::shared_ptr<AudioStreamCallback> callback = nullptr;
    auto ret = renderer->SetStreamCallback(callback);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: SetRendererFirstFrameWritingCallback_001
 * @tc.desc  : Test SetRendererFirstFrameWritingCallback with null callback
 */
HWTEST(RendererInClientSecondUnitTest, SetRendererFirstFrameWritingCallback_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    std::shared_ptr<AudioRendererFirstFrameWritingCallback> callback = nullptr;
    auto ret = renderer->SetRendererFirstFrameWritingCallback(callback);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: SetSpeed_001
 * @tc.desc  : Test SetSpeed
 */
HWTEST(RendererInClientSecondUnitTest, SetSpeed_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    float speed = 1.0f;
    auto ret = renderer->SetSpeed(speed);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: SetPitch_001
 * @tc.desc  : Test SetPitch
 */
HWTEST(RendererInClientSecondUnitTest, SetPitch_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    float pitch = 1.0f;
    auto ret = renderer->SetPitch(pitch);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: SetSonicPitch_001
 * @tc.desc  : Test SetSonicPitch
 */
HWTEST(RendererInClientSecondUnitTest, SetSonicPitch_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    float pitch = 1.0f;
    auto ret = renderer->SetSonicPitch(pitch);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: GetSpeed_001
 * @tc.desc  : Test GetSpeed
 */
HWTEST(RendererInClientSecondUnitTest, GetSpeed_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    float speed = renderer->GetSpeed();
    EXPECT_EQ(speed, 1.0f);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: GetAudioDebugInfo_001
 * @tc.desc  : Test GetAudioDebugInfo
 */
HWTEST(RendererInClientSecondUnitTest, GetAudioDebugInfo_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    AudioDebugInfo debugInfo;
    auto ret = renderer->GetAudioDebugInfo(debugInfo);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: SetRendererWriteCallback_001
 * @tc.desc  : Test SetRendererWriteCallback with null callback
 */
HWTEST(RendererInClientSecondUnitTest, SetRendererWriteCallback_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    std::shared_ptr<AudioRendererWriteCallback> callback = nullptr;
    auto ret = renderer->SetRendererWriteCallback(callback);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: SetCapturerMetaDataCallback_001
 * @tc.desc  : Test SetCapturerMetaDataCallback
 */
HWTEST(RendererInClientSecondUnitTest, SetCapturerMetaDataCallback_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    std::shared_ptr<AudioCapturerMetaDataCallback> callback = nullptr;
    auto ret = renderer->SetCapturerMetaDataCallback(callback);
    EXPECT_NE(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: GetBufferDesc_001
 * @tc.desc  : Test GetBufferDesc with null ipcStream
 */
HWTEST(RendererInClientSecondUnitTest, GetBufferDesc_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    BufferDesc bufDesc;
    auto ret = renderer->GetBufferDesc(bufDesc);
    EXPECT_NE(ret, ERR_INVALID_OPERATION);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: Enqueue_001
 * @tc.desc  : Test Enqueue with null ipcStream
 */
HWTEST(RendererInClientSecondUnitTest, Enqueue_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    BufferDesc bufDesc;
    auto ret = renderer->Enqueue(bufDesc);
    EXPECT_NE(ret, ERR_INVALID_OPERATION);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: Clear_001
 * @tc.desc  : Test Clear
 */
HWTEST(RendererInClientSecondUnitTest, Clear_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    auto ret = renderer->Clear();
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: SetLowPowerVolume_001
 * @tc.desc  : Test SetLowPowerVolume with valid volume
 */
HWTEST(RendererInClientSecondUnitTest, SetLowPowerVolume_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    float volume = 0.5f;
    auto ret = renderer->SetLowPowerVolume(volume);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: GetLowPowerVolume_001
 * @tc.desc  : Test GetLowPowerVolume
 */
HWTEST(RendererInClientSecondUnitTest, GetLowPowerVolume_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    float volume = renderer->GetLowPowerVolume();
    EXPECT_EQ(volume, 1.0f);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: SetAudioEffectMode_001
 * @tc.desc  : Test SetAudioEffectMode
 */
HWTEST(RendererInClientSecondUnitTest, SetAudioEffectMode_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    AudioEffectMode effectMode = EFFECT_NONE;
    auto ret = renderer->SetAudioEffectMode(effectMode);
    EXPECT_NE(ret, ERR_NOT_SUPPORTED);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: GetAudioEffectMode_001
 * @tc.desc  : Test GetAudioEffectMode
 */
HWTEST(RendererInClientSecondUnitTest, GetAudioEffectMode_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    AudioEffectMode effectMode = renderer->GetAudioEffectMode();
    EXPECT_NE(effectMode, EFFECT_NONE);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: GetSingleStreamVolume_001
 * @tc.desc  : Test GetSingleStreamVolume
 */
HWTEST(RendererInClientSecondUnitTest, GetSingleStreamVolume_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    float volume = renderer->GetSingleStreamVolume();
    EXPECT_EQ(volume, 1.0f);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: SetSourceDuration_001
 * @tc.desc  : Test SetSourceDuration
 */
HWTEST(RendererInClientSecondUnitTest, SetSourceDuration_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    int64_t duration = 1000;
    auto ret = renderer->SetSourceDuration(duration);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: SetInMainThreadState_001
 * @tc.desc  : Test SetInMainThreadState
 */
HWTEST(RendererInClientSecondUnitTest, SetInMainThreadState_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    bool isInMainThread = true;
    auto ret = renderer->SetInMainThreadState(isInMainThread);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RequestUserPrivacyAuthority_001
 * @tc.desc  : Test RequestUserPrivacyAuthority
 */
HWTEST(RendererInClientSecondUnitTest, RequestUserPrivacyAuthority_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    uint32_t sessionId = 0;
    auto ret = renderer->RequestUserPrivacyAuthority(sessionId);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: SetPlaybackCaptureStartStateCallback_001
 * @tc.desc  : Test SetPlaybackCaptureStartStateCallback
 */
HWTEST(RendererInClientSecondUnitTest, SetPlaybackCaptureStartStateCallback_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    std::shared_ptr<AudioCapturerOnPlaybackCaptureStartCallback> callback = nullptr;
    renderer->SetPlaybackCaptureStartStateCallback(callback);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: SetClientDeviceId_001
 * @tc.desc  : Test SetClientDeviceId
 */
HWTEST(RendererInClientSecondUnitTest, SetClientDeviceId_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    std::string deviceId = "test_device";
    renderer->SetClientDeviceId(deviceId);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: SetRendererInfo_001
 * @tc.desc  : Test SetRendererInfo
 */
HWTEST(RendererInClientSecondUnitTest, SetRendererInfo_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    AudioRendererInfo rendererInfo;
    rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
    rendererInfo.contentType = CONTENT_TYPE_MUSIC;
    renderer->SetRendererInfo(rendererInfo);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: GetRendererInfo_001
 * @tc.desc  : Test GetRendererInfo
 */
HWTEST(RendererInClientSecondUnitTest, GetRendererInfo_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    AudioRendererInfo rendererInfo;
    renderer->GetRendererInfo(rendererInfo);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: SetCapturerInfo_001
 * @tc.desc  : Test SetCapturerInfo
 */
HWTEST(RendererInClientSecondUnitTest, SetCapturerInfo_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    AudioCapturerInfo capturerInfo;
    capturerInfo.sourceType = SOURCE_TYPE_MIC;
    renderer->SetCapturerInfo(capturerInfo);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: GetRenderMode_001
 * @tc.desc  : Test GetRenderMode
 */
HWTEST(RendererInClientSecondUnitTest, GetRenderMode_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    AudioRenderMode renderMode = renderer->GetRenderMode();
    EXPECT_EQ(renderMode, RENDER_MODE_NORMAL);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: SetPreferredFrameSize_001
 * @tc.desc  : Test SetPreferredFrameSize
 */
HWTEST(RendererInClientSecondUnitTest, SetPreferredFrameSize_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    int32_t frameSize = 1024;
    renderer->SetPreferredFrameSize(frameSize);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: SetPreferredFrameSize_002
 * @tc.desc  : Test SetPreferredFrameSize with isRecreate
 */
HWTEST(RendererInClientSecondUnitTest, SetPreferredFrameSize_002, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    int32_t frameSize = 2048;
    bool isRecreate = true;
    renderer->SetPreferredFrameSize(frameSize, isRecreate);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: UpdateLatencyTimestamp_001
 * @tc.desc  : Test UpdateLatencyTimestamp
 */
HWTEST(RendererInClientSecondUnitTest, UpdateLatencyTimestamp_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    std::string timestamp = "test_timestamp";
    bool isRenderer = true;
    renderer->UpdateLatencyTimestamp(timestamp, isRenderer);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: GetFramesWritten_001
 * @tc.desc  : Test GetFramesWritten with null ipcStream
 */
HWTEST(RendererInClientSecondUnitTest, GetFramesWritten_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    int64_t framesWritten = renderer->GetFramesWritten();
    EXPECT_NE(framesWritten, -1);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RegisterRemoteDiedCallback_001
 * @tc.desc  : Test RegisterRemoteDiedCallback with null ipcStream
 */
HWTEST(RendererInClientSecondUnitTest, RegisterRemoteDiedCallback_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    std::shared_ptr<RemoteDiedCallback> callback = nullptr;
    auto ret = renderer->RegisterRemoteDiedCallback(callback);
    EXPECT_NE(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: UnregisterRemoteDiedCallback_001
 * @tc.desc  : Test UnregisterRemoteDiedCallback with null ipcStream
 */
HWTEST(RendererInClientSecondUnitTest, UnregisterRemoteDiedCallback_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    auto ret = renderer->UnregisterRemoteDiedCallback();
    EXPECT_NE(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: SetRendererPositionCallback_001
 * @tc.desc  : Test SetRendererPositionCallback
 */
HWTEST(RendererInClientSecondUnitTest, SetRendererPositionCallback_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    int64_t markPosition = TEST_POSITION;
    std::shared_ptr<RendererPositionCallback> callback = nullptr;
    renderer->SetRendererPositionCallback(markPosition, callback);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: UnsetRendererPositionCallback_001
 * @tc.desc  : Test UnsetRendererPositionCallback
 */
HWTEST(RendererInClientSecondUnitTest, UnsetRendererPositionCallback_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    renderer->UnsetRendererPositionCallback();
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: SetRendererPeriodPositionCallback_001
 * @tc.desc  : Test SetRendererPeriodPositionCallback
 */
HWTEST(RendererInClientSecondUnitTest, SetRendererPeriodPositionCallback_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    int64_t periodPosition = TEST_POSITION;
    std::shared_ptr<RendererPeriodPositionCallback> callback = nullptr;
    renderer->SetRendererPeriodPositionCallback(periodPosition, callback);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: UnsetRendererPeriodPositionCallback_001
 * @tc.desc  : Test UnsetRendererPeriodPositionCallback
 */
HWTEST(RendererInClientSecondUnitTest, UnsetRendererPeriodPositionCallback_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    renderer->UnsetRendererPeriodPositionCallback();
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: SetCapturerPositionCallback_001
 * @tc.desc  : Test SetCapturerPositionCallback
 */
HWTEST(RendererInClientSecondUnitTest, SetCapturerPositionCallback_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    int64_t markPosition = TEST_POSITION;
    std::shared_ptr<CapturerPositionCallback> callback = nullptr;
    renderer->SetCapturerPositionCallback(markPosition, callback);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: UnsetCapturerPositionCallback_001
 * @tc.desc  : Test UnsetCapturerPositionCallback
 */
HWTEST(RendererInClientSecondUnitTest, UnsetCapturerPositionCallback_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    renderer->UnsetCapturerPositionCallback();
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: SetCapturerPeriodPositionCallback_001
 * @tc.desc  : Test SetCapturerPeriodPositionCallback
 */
HWTEST(RendererInClientSecondUnitTest, SetCapturerPeriodPositionCallback_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    int64_t periodPosition = TEST_POSITION;
    std::shared_ptr<CapturerPeriodPositionCallback> callback = nullptr;
    renderer->SetCapturerPeriodPositionCallback(periodPosition, callback);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: UnsetCapturerPeriodPositionCallback_001
 * @tc.desc  : Test UnsetCapturerPeriodPositionCallback
 */
HWTEST(RendererInClientSecondUnitTest, UnsetCapturerPeriodPositionCallback_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    renderer->UnsetCapturerPeriodPositionCallback();
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: SetRendererSamplingRate_001
 * @tc.desc  : Test SetRendererSamplingRate
 */
HWTEST(RendererInClientSecondUnitTest, SetRendererSamplingRate_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    uint32_t sampleRate = 48000;
    auto ret = renderer->SetRendererSamplingRate(sampleRate);
    EXPECT_NE(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: GetRendererSamplingRate_001
 * @tc.desc  : Test GetRendererSamplingRate
 */
HWTEST(RendererInClientSecondUnitTest, GetRendererSamplingRate_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    uint32_t sampleRate = renderer->GetRendererSamplingRate();
    EXPECT_EQ(sampleRate, 0);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: SetBufferSizeInMsec_001
 * @tc.desc  : Test SetBufferSizeInMsec
 */
HWTEST(RendererInClientSecondUnitTest, SetBufferSizeInMsec_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    int32_t bufferSizeInMsec = 20;
    auto ret = renderer->SetBufferSizeInMsec(bufferSizeInMsec);
    EXPECT_NE(ret, ERR_NOT_SUPPORTED);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: SetChannelBlendMode_001
 * @tc.desc  : Test SetChannelBlendMode
 */
HWTEST(RendererInClientSecondUnitTest, SetChannelBlendMode_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    ChannelBlendMode blendMode = MODE_DEFAULT;
    auto ret = renderer->SetChannelBlendMode(blendMode);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: SetVolumeWithRamp_001
 * @tc.desc  : Test SetVolumeWithRamp
 */
HWTEST(RendererInClientSecondUnitTest, SetVolumeWithRamp_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    float volume = 0.5f;
    int32_t duration = 100;
    auto ret = renderer->SetVolumeWithRamp(volume, duration);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: SetStreamTrackerState_001
 * @tc.desc  : Test SetStreamTrackerState
 */
HWTEST(RendererInClientSecondUnitTest, SetStreamTrackerState_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    bool trackerRegisteredState = true;
    renderer->SetStreamTrackerState(trackerRegisteredState);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: GetStreamClass_001
 * @tc.desc  : Test GetStreamClass
 */
HWTEST(RendererInClientSecondUnitTest, GetStreamClass_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    IAudioStream::StreamClass streamClass = renderer->GetStreamClass();
    EXPECT_EQ(streamClass, IAudioStream::StreamClass::PA_STREAM);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: GetFocusState_001
 * @tc.desc  : Test GetFocusState
 */
HWTEST(RendererInClientSecondUnitTest, GetFocusState_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    StreamFocusState focusState = renderer->GetFocusState();
    EXPECT_NE(focusState, StreamFocusState::FOCUS_INVALID);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: SetFocusState_001
 * @tc.desc  : Test SetFocusState
 */
HWTEST(RendererInClientSecondUnitTest, SetFocusState_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    StreamFocusState focusState = StreamFocusState::FOCUS_RUNNING;
    renderer->SetFocusState(focusState);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: UpdateStreamState_001
 * @tc.desc  : Test UpdateStreamState
 */
HWTEST(RendererInClientSecondUnitTest, UpdateStreamState_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    State state = State::RUNNING;
    renderer->UpdateStreamState(state);
    EXPECT_EQ(renderer->GetState(), State::RUNNING);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: InitCallbackHandler_001
 * @tc.desc  : Test InitCallbackHandler
 */
HWTEST(RendererInClientSecondUnitTest, InitCallbackHandler_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    renderer->InitCallbackHandler();
    EXPECT_TRUE(true);
}
} // namespace AudioStandard
} // namespace OHOS
