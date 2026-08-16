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

#include <gtest/gtest.h>
#include "direct_playback_engine.h"
#include "sink/i_audio_render_sink.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"
#include "pro_renderer_stream_impl.h"
#include "audio_errors.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {
class DirectPlayBackEngineUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

protected:
    AudioProcessConfig InitProcessConfig();

protected:
    std::unique_ptr<AudioPlaybackEngine> playbackEngine_;
};

void DirectPlayBackEngineUnitTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void DirectPlayBackEngineUnitTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void DirectPlayBackEngineUnitTest::SetUp(void)
{
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    deviceInfo.deviceType_ = DEVICE_TYPE_SPEAKER;
    playbackEngine_ = std::make_unique<DirectPlayBackEngine>();
    if (playbackEngine_) {
        playbackEngine_->Init(deviceInfo, false);
    }
}

void DirectPlayBackEngineUnitTest::TearDown(void)
{
    if (playbackEngine_) {
        playbackEngine_->Stop();
        playbackEngine_ = nullptr;
    }
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_001
 * @tc.desc  : Test DirectPlayBackEngine::Init().
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_001, TestSize.Level2)
{
    DirectPlayBackEngine engine;
    AudioDeviceDescriptor deviceDesc(AudioDeviceDescriptor::DEVICE_INFO);
    bool isVoip = true;
    engine.isInit_ = true;
    deviceDesc.deviceType_ = DEVICE_TYPE_INVALID;
    engine.device_.deviceType_ = DEVICE_TYPE_NONE;
    engine.renderId_ = HDI_INVALID_ID;

    auto ret = engine.Init(deviceDesc, isVoip);
    EXPECT_EQ(ret, SUCCESS);

    engine.isInit_ = false;
    ret = engine.Init(deviceDesc, isVoip);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_002
 * @tc.desc  : Test DirectPlayBackEngine::Init().
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_002, TestSize.Level2)
{
    DirectPlayBackEngine engine;
    AudioDeviceDescriptor deviceDesc(AudioDeviceDescriptor::DEVICE_INFO);
    bool isVoip = true;
    engine.isInit_ = true;
    deviceDesc.deviceType_ = DEVICE_TYPE_INVALID;
    engine.device_.deviceType_ = DEVICE_TYPE_INVALID;
    engine.renderId_ = HDI_INVALID_ID;

    auto ret = engine.Init(deviceDesc, isVoip);
    EXPECT_EQ(ret, SUCCESS);

    deviceDesc.deviceType_ = DEVICE_TYPE_NONE;
    ret = engine.Init(deviceDesc, isVoip);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_003
 * @tc.desc  : Test DirectPlayBackEngine::AddRenderer().
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_003, TestSize.Level2)
{
    DirectPlayBackEngine engine;
    engine.isStart_ = true;
    uint32_t idx1 = 0;
    uint32_t idx2 = 1;
    AudioProcessConfig config;
    std::shared_ptr<ProRendererStreamImpl> rendererStream1 = std::make_shared<ProRendererStreamImpl>(config, true);
    if (rendererStream1) {
        rendererStream1->SetStreamIndex(idx1);
    }
    engine.stream_ = rendererStream1;
    auto ret = engine.AddRenderer(rendererStream1);
    EXPECT_EQ(ret, SUCCESS);

    std::shared_ptr<ProRendererStreamImpl> rendererStream2 = std::make_shared<ProRendererStreamImpl>(config, true);
    if (rendererStream2) {
        rendererStream2->SetStreamIndex(idx2);
    }
    ret = engine.AddRenderer(rendererStream2);
    EXPECT_EQ(ret, ERROR_UNSUPPORTED);
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_004
 * @tc.desc  : Test DirectPlayBackEngine::RemoveRenderer().
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_004, TestSize.Level2)
{
    DirectPlayBackEngine engine;
    engine.isStart_ = true;
    uint32_t idx1 = 0;
    uint32_t idx2 = 1;
    AudioProcessConfig config;
    std::shared_ptr<ProRendererStreamImpl> rendererStream1 = std::make_shared<ProRendererStreamImpl>(config, true);
    if (rendererStream1) {
        rendererStream1->SetStreamIndex(idx1);
    }
    engine.stream_ = nullptr;
    engine.RemoveRenderer(rendererStream1);

    engine.stream_ = rendererStream1;
    engine.RemoveRenderer(rendererStream1);
    EXPECT_EQ(engine.stream_, nullptr);

    std::shared_ptr<ProRendererStreamImpl> rendererStream2 = std::make_shared<ProRendererStreamImpl>(config, true);
    if (rendererStream2) {
        rendererStream2->SetStreamIndex(idx2);
    }
    engine.stream_ = rendererStream1;
    EXPECT_NE(engine.stream_->GetStreamIndex(), rendererStream2->GetStreamIndex());
    engine.RemoveRenderer(rendererStream2);
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_005
 * @tc.desc  : Test DirectPlayBackEngine::MixStreams()
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_005, TestSize.Level2)
{
    auto enginePtr = std::make_shared<DirectPlayBackEngine>();
    EXPECT_NE(enginePtr, nullptr);

    AudioProcessConfig configRet;
    enginePtr->stream_ = std::make_shared<ProRendererStreamImpl>(configRet, true);
    EXPECT_NE(enginePtr->stream_, nullptr);
    enginePtr->MixStreams();

    enginePtr->stream_ = nullptr;
    enginePtr->MixStreams();
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_006
 * @tc.desc  : Test DirectPlayBackEngine::MixStreams()
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_006, TestSize.Level2)
{
    auto enginePtr = std::make_shared<DirectPlayBackEngine>();
    EXPECT_NE(enginePtr, nullptr);

    AudioProcessConfig config;
    enginePtr->stream_ = std::make_shared<ProRendererStreamImpl>(config, true);
    EXPECT_NE(enginePtr->stream_, nullptr);
    enginePtr->MixStreams();

    enginePtr->MixStreams();
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_007
 * @tc.desc  : Test DirectPlayBackEngine::GetDirectFormatByteSize().
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_007, TestSize.Level2)
{
    DirectPlayBackEngine engine;
    AudioSampleFormat format = SAMPLE_S16LE;
    auto ret = engine.GetDirectFormatByteSize(format);
    EXPECT_EQ(ret, sizeof(int16_t));

    format = SAMPLE_S32LE;
    ret = engine.GetDirectFormatByteSize(format);
    EXPECT_EQ(ret, sizeof(int32_t));

    format = SAMPLE_F32LE;
    ret = engine.GetDirectFormatByteSize(format);
    EXPECT_EQ(ret, sizeof(int32_t));

    format = INVALID_WIDTH;
    ret = engine.GetDirectFormatByteSize(format);
    EXPECT_EQ(ret, sizeof(int32_t));
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_008
 * @tc.desc  : Test DirectPlayBackEngine::InitSink()
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_008, TestSize.Level2)
{
    auto enginePtr = std::make_shared<DirectPlayBackEngine>();
    EXPECT_NE(enginePtr, nullptr);

    AudioStreamInfo streamInfo;
    enginePtr->isInit_ = false;
    enginePtr->renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_HWDECODE,
        HDI_ID_INFO_DEFAULT, true);
    EXPECT_NE(enginePtr->renderId_, HDI_INVALID_ID);

    auto ret = enginePtr->InitSink(streamInfo);
    EXPECT_NE(ret, SUCCESS);
}


/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_009
 * @tc.desc  : Test DirectPlayBackEngine::InitSink()
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_009, TestSize.Level2)
{
    auto enginePtr = std::make_shared<DirectPlayBackEngine>();
    EXPECT_NE(enginePtr, nullptr);

    AudioStreamInfo streamInfo;
    enginePtr->isInit_ = true;
    enginePtr->renderId_ = HDI_INVALID_ID;

    auto ret = enginePtr->InitSink(streamInfo);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_010
 * @tc.desc  : Test DirectPlayBackEngine::InitSink()
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_010, TestSize.Level2)
{
    auto enginePtr = std::make_shared<DirectPlayBackEngine>();
    EXPECT_NE(enginePtr, nullptr);

    AudioStreamInfo streamInfo;
    streamInfo.channels = AudioChannel::STEREO;
    streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_S32LE;
    enginePtr->isInit_ = true;
    enginePtr->renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_HWDECODE,
        HDI_ID_INFO_DEFAULT, true);
    EXPECT_NE(enginePtr->renderId_, HDI_INVALID_ID);
    auto ret = enginePtr->InitSink(streamInfo);
    EXPECT_NE(ret, SUCCESS);

    enginePtr->uChannel_ = AudioChannel::CHANNEL_UNKNOW;
    ret = enginePtr->InitSink(streamInfo);
    EXPECT_NE(ret, SUCCESS);

    enginePtr->uChannel_ = AudioChannel::STEREO;
    ret = enginePtr->InitSink(streamInfo);
    EXPECT_NE(ret, SUCCESS);

    enginePtr->format_ = SAMPLE_S16LE;
    ret = enginePtr->InitSink(streamInfo);
    EXPECT_NE(ret, SUCCESS);

    enginePtr->format_ = SAMPLE_S32LE;
    ret = enginePtr->InitSink(streamInfo);
    EXPECT_NE(ret, SUCCESS);

    enginePtr->uSampleRate_ = AudioSamplingRate::SAMPLE_RATE_32000;
    ret = enginePtr->InitSink(streamInfo);
    EXPECT_NE(ret, SUCCESS);

    enginePtr->uSampleRate_ = AudioSamplingRate::SAMPLE_RATE_48000;
    ret = enginePtr->InitSink(streamInfo);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_011
 * @tc.desc  : Test DirectPlayBackEngine::StopAudioSink()
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_011, TestSize.Level2)
{
    auto enginePtr = std::make_shared<DirectPlayBackEngine>();
    ASSERT_TRUE(enginePtr != nullptr);

    enginePtr->renderId_ = -1;
    auto ret = enginePtr->StopAudioSink();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_012
 * @tc.desc  : Test DirectPlayBackEngine::StopAudioSink()
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_012, TestSize.Level2)
{
    auto enginePtr = std::make_shared<DirectPlayBackEngine>();
    ASSERT_TRUE(enginePtr != nullptr);

    enginePtr->renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_HWDECODE,
        HDI_ID_INFO_DEFAULT, true);
    auto sink = HdiAdapterManager::GetInstance().GetRenderSink(enginePtr->renderId_, true);
    ASSERT_TRUE(sink != nullptr);

    auto ret = enginePtr->StopAudioSink();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_013
 * @tc.desc  : Test DirectPlayBackEngine::Start()
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_013, TestSize.Level2)
{
    auto enginePtr = std::make_shared<DirectPlayBackEngine>();
    EXPECT_NE(enginePtr, nullptr);

    enginePtr->renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_HWDECODE,
        HDI_ID_INFO_DEFAULT, true);
    EXPECT_NE(enginePtr->renderId_, HDI_INVALID_ID);

    enginePtr->isStart_ = true;
    auto ret = enginePtr->Start();
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);

    enginePtr->isStart_ = false;
    ret = enginePtr->Start();
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_014
 * @tc.desc  : Test DirectPlayBackEngine::Stop()
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_014, TestSize.Level2)
{
    auto enginePtr = std::make_shared<DirectPlayBackEngine>();
    ASSERT_TRUE(enginePtr != nullptr);

    enginePtr->isStart_ = true;
    auto ret = enginePtr->Stop();
    EXPECT_EQ(ret, SUCCESS);

    enginePtr->isStart_ = false;
    ret = enginePtr->Stop();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_015
 * @tc.desc  : Test DirectPlayBackEngine::Pause()
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_015, TestSize.Level2)
{
    auto enginePtr = std::make_shared<DirectPlayBackEngine>();
    ASSERT_TRUE(enginePtr != nullptr);

    enginePtr->isStart_ = true;
    auto ret = enginePtr->Pause();
    EXPECT_EQ(ret, SUCCESS);

    enginePtr->isStart_ = false;
    ret = enginePtr->Pause();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_016
 * @tc.desc  : Test DirectPlayBackEngine::DirectCallback()
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_016, TestSize.Level2)
{
    auto enginePtr = std::make_shared<DirectPlayBackEngine>();
    ASSERT_TRUE(enginePtr != nullptr);
    enginePtr->DirectCallback(CB_NONBLOCK_WRITE_COMPLETED);
    enginePtr->DirectCallback(CB_DRAIN_COMPLETED);
    enginePtr->DirectCallback(CB_FLUSH_COMPLETED);
    enginePtr->DirectCallback(CB_RENDER_FULL);
    enginePtr->DirectCallback(CB_ERROR_OCCUR);
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_017
 * @tc.desc  : Test DirectPlayBackEngine::Flush().
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_017, TestSize.Level2)
{
    auto enginePtr = std::make_shared<DirectPlayBackEngine>();
    ASSERT_TRUE(enginePtr != nullptr);

    // Flush() unconditionally returns SUCCESS (direct_playback_engine.cpp:142).
    auto ret = enginePtr->Flush();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_018
 * @tc.desc  : Test DirectPlayBackEngine::GetLatency() default value.
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_018, TestSize.Level2)
{
    auto enginePtr = std::make_shared<DirectPlayBackEngine>();
    ASSERT_TRUE(enginePtr != nullptr);

    // latency_ is zero-initialized in the constructor; GetLatency returns it directly.
    EXPECT_EQ(enginePtr->GetLatency(), 0ULL);
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_019
 * @tc.desc  : Test DirectPlayBackEngine::IsPlaybackEngineRunning() reflects isStart_.
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_019, TestSize.Level2)
{
    auto enginePtr = std::make_shared<DirectPlayBackEngine>();
    ASSERT_TRUE(enginePtr != nullptr);

    enginePtr->isStart_ = false;
    EXPECT_FALSE(enginePtr->IsPlaybackEngineRunning());

    enginePtr->isStart_ = true;
    EXPECT_TRUE(enginePtr->IsPlaybackEngineRunning());
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_020
 * @tc.desc  : Test DirectPlayBackEngine::DoRenderFrame() with null sink.
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_020, TestSize.Level2)
{
    auto enginePtr = std::make_shared<DirectPlayBackEngine>();
    ASSERT_TRUE(enginePtr != nullptr);

    // renderId_ = HDI_INVALID_ID makes GetRenderSink return nullptr, hitting the early return.
    enginePtr->renderId_ = HDI_INVALID_ID;
    enginePtr->stream_ = nullptr;
    constexpr int32_t testAppUid = 0;
    std::vector<char> audioBuffer;
    // No crash; stream_->ReturnIndex is not reached.
    enginePtr->DoRenderFrame(audioBuffer, 0, testAppUid);
    EXPECT_EQ(enginePtr->stream_, nullptr);
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_021
 * @tc.desc  : Test DirectPlayBackEngine::DoRenderFrame() null sink with non-empty buffer.
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_021, TestSize.Level2)
{
    auto enginePtr = std::make_shared<DirectPlayBackEngine>();
    ASSERT_TRUE(enginePtr != nullptr);

    enginePtr->renderId_ = HDI_INVALID_ID;
    enginePtr->stream_ = nullptr;
    constexpr int32_t testAppUid = 0;
    std::vector<char> audioBuffer(sizeof(int32_t), 0);
    // Clean early-return on null sink; stream_ stays null, no deref occurs.
    enginePtr->DoRenderFrame(audioBuffer, 0, testAppUid);
    EXPECT_EQ(enginePtr->stream_, nullptr);
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_022
 * @tc.desc  : Test DirectPlayBackEngine::RegisterWriteCallback() with null sink.
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_022, TestSize.Level2)
{
    auto enginePtr = std::make_shared<DirectPlayBackEngine>();
    ASSERT_TRUE(enginePtr != nullptr);

    // null sink guard returns ERR_INVALID_HANDLE (direct_playback_engine.cpp:165).
    enginePtr->renderId_ = HDI_INVALID_ID;
    auto ret = enginePtr->RegisterWriteCallback();
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_023
 * @tc.desc  : Test DirectPlayBackEngine::AddRenderer() with null stream.
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_023, TestSize.Level2)
{
    auto enginePtr = std::make_shared<DirectPlayBackEngine>();
    ASSERT_TRUE(enginePtr != nullptr);

    // null stream guard returns ERR_INVALID_PARAM (direct_playback_engine.cpp:207).
    auto ret = enginePtr->AddRenderer(nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_024
 * @tc.desc  : Test DirectPlayBackEngine::RemoveRenderer() with null stream.
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_024, TestSize.Level2)
{
    auto enginePtr = std::make_shared<DirectPlayBackEngine>();
    ASSERT_TRUE(enginePtr != nullptr);

    enginePtr->stream_ = nullptr;
    // null stream guard returns early; no-op, no crash (direct_playback_engine.cpp:226).
    enginePtr->RemoveRenderer(nullptr);
    EXPECT_EQ(enginePtr->stream_, nullptr);
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_025
 * @tc.desc  : Test DirectPlayBackEngine::DirectCallback() default arm.
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_025, TestSize.Level2)
{
    auto enginePtr = std::make_shared<DirectPlayBackEngine>();
    ASSERT_TRUE(enginePtr != nullptr);

    enginePtr->stream_ = nullptr;
    // Value outside the named enumerators (0..4) hits the default arm; no MixStreams side effect.
    constexpr int32_t unknownCallbackType = 999;
    enginePtr->DirectCallback(static_cast<RenderCallbackType>(unknownCallbackType));
    EXPECT_EQ(enginePtr->stream_, nullptr);
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_026
 * @tc.desc  : Test DirectPlayBackEngine::Start() with non-inited sink.
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_026, TestSize.Level2)
{
    auto enginePtr = std::make_shared<DirectPlayBackEngine>();
    ASSERT_TRUE(enginePtr != nullptr);

    enginePtr->renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_HWDECODE,
        HDI_ID_INFO_DEFAULT, true);
    EXPECT_NE(enginePtr->renderId_, HDI_INVALID_ID);
    // Prime the cache so the internal GetRenderSink(renderId_) returns a non-null, not-inited sink.
    auto sink = HdiAdapterManager::GetInstance().GetRenderSink(enginePtr->renderId_, true);
    ASSERT_TRUE(sink != nullptr);
    EXPECT_FALSE(sink->IsInited());

    enginePtr->isStart_ = false;
    // sink->IsInited() is false -> returns ERR_NOT_STARTED (direct_playback_engine.cpp:85).
    auto ret = enginePtr->Start();
    EXPECT_EQ(ret, ERR_NOT_STARTED);
}

// ===== Phase 2: sink-dependent branch coverage =====
// The direct_playback_engine_unit_test target does NOT link gmock, so a
// hand-rolled fake sink is used (self-contained, no gmock dependency). It
// implements every pure virtual of IAudioRenderSink that DirectPlayBackEngine
// calls and records each call so tests can observe the sink side effects of
// every engine branch. Direct map access is legal via -fno-access-control.
class FakeAudioRenderSink : public IAudioRenderSink {
public:
    FakeAudioRenderSink() = default;
    ~FakeAudioRenderSink() override = default;

    int32_t Init(const IAudioSinkAttr &attr) override
    {
        initCalls++;
        lastAttr = attr;
        return mockInitRet;
    }
    void DeInit(void) override { deInitCalls++; }
    bool IsInited(void) override { return inited; }

    int32_t Start(void) override
    {
        startCalls++;
        return mockStartRet;
    }
    int32_t Stop(void) override
    {
        stopCalls++;
        return mockStopRet;
    }
    int32_t Resume(void) override { return SUCCESS; }
    int32_t Pause(void) override { return SUCCESS; }
    int32_t Flush(void) override { return SUCCESS; }
    int32_t Reset(void) override { return SUCCESS; }
    int32_t RenderFrame(char &data, uint64_t len, uint64_t &writeLen) override
    {
        renderFrameCalls++;
        writeLen = len;
        return SUCCESS;
    }
    int32_t GetVolumeDataCount(int64_t &volumeData) override { return SUCCESS; }

    int32_t SetVolume(float left, float right) override
    {
        setVolumeCalls++;
        return mockSetVolumeRet;
    }
    int32_t SetVolumeWithRamp(float left, float right, uint32_t durationMs) override { return SUCCESS; }
    int32_t GetVolume(float &left, float &right) override { return SUCCESS; }

    int32_t GetLatency(uint32_t &latency) override { return SUCCESS; }
    int32_t GetPresentationPosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec) override
    {
        return SUCCESS;
    }
    float GetMaxAmplitude(void) override { return 0.0f; }
    void SetAudioMonoState(bool audioMono) override {}
    void SetAudioBalanceValue(float audioBalance) override {}

    int32_t UpdateAppsUid(const int32_t appsUid[MAX_MIX_CHANNELS], const size_t size) override { return SUCCESS; }
    int32_t UpdateAppsUid(const std::vector<int32_t> &appsUid) override { return SUCCESS; }

    void DumpInfo(std::string &dumpString) override {}

    // Non-pure in the base (default returns SUCCESS); overridden so the
    // RegisterWriteCallback path can be observed.
    int32_t RegistDirectHdiCallback(std::function<void(const RenderCallbackType type)> callback) override
    {
        registerCbCalls++;
        storedCb = std::move(callback);
        return mockRegisterCbRet;
    }

    // Observable state (defaults exercise the happy path).
    bool inited = true;
    int32_t mockInitRet = SUCCESS;
    int32_t mockStartRet = SUCCESS;
    int32_t mockStopRet = SUCCESS;
    int32_t mockSetVolumeRet = SUCCESS;
    int32_t mockRegisterCbRet = SUCCESS;

    // Call counters.
    int32_t initCalls = 0;
    int32_t deInitCalls = 0;
    int32_t startCalls = 0;
    int32_t stopCalls = 0;
    int32_t renderFrameCalls = 0;
    int32_t setVolumeCalls = 0;
    int32_t registerCbCalls = 0;
    std::function<void(const RenderCallbackType)> storedCb;
    IAudioSinkAttr lastAttr = {};
};

// RAII seeder: installs a FakeAudioRenderSink into the HdiAdapterManager
// singleton for the id bound to (info), and deterministically erases the
// entry on scope exit so the singleton map stays clean for sibling cases.
// Direct erase is used (rather than ReleaseId) so cleanup is independent of
// refcount drift caused by the engine's own GetId/ReleaseId calls.
struct SeededRenderSink {
    uint32_t id = HDI_INVALID_ID;
    std::shared_ptr<FakeAudioRenderSink> fake;

    explicit SeededRenderSink(const std::string &info)
    {
        id = HdiAdapterManager::GetInstance().GetId(
            HDI_ID_BASE_RENDER, HDI_ID_TYPE_HWDECODE, info, true);
        fake = std::make_shared<FakeAudioRenderSink>();
        HdiAdapterManager::GetInstance().renderSinks_[id].sink_ = fake;
        HdiAdapterManager::GetInstance().renderSinks_[id].refCount_ = 1;
    }
    ~SeededRenderSink()
    {
        HdiAdapterManager::GetInstance().renderSinks_.erase(id);
    }
};

// Named constants for Phase 2 cases (no magic numbers).
// NOTE: must equal EAC3_SINK_NAME in direct_playback_engine.cpp.
const std::string DIRECT_PHASE2_SINK_NAME = "eac3";
constexpr AudioChannel DIRECT_PHASE2_CH_STEREO = AudioChannel::STEREO;
constexpr AudioChannel DIRECT_PHASE2_CH_MONO = AudioChannel::MONO;
// F32LE is the only format whose enum value (4) equals its byte size
// (sizeof(int32_t) == 4), which is required to reach the cache-hit branch.
constexpr AudioSampleFormat DIRECT_PHASE2_FMT_F32LE = SAMPLE_F32LE;
constexpr AudioSamplingRate DIRECT_PHASE2_RATE_48K = AudioSamplingRate::SAMPLE_RATE_48000;
constexpr AudioChannelLayout DIRECT_PHASE2_LAYOUT_STEREO = AudioChannelLayout::CH_LAYOUT_STEREO;
constexpr uint64_t DIRECT_PHASE2_ZERO_LATENCY = 0ULL;

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_027
 * @tc.desc  : Test DirectPlayBackEngine::InitSink(4-arg) happy path with seeded sink.
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_027, TestSize.Level2)
{
    SeededRenderSink seed(DIRECT_PHASE2_SINK_NAME);
    auto enginePtr = std::make_shared<DirectPlayBackEngine>();
    ASSERT_TRUE(enginePtr != nullptr);

    enginePtr->isInit_ = false;
    auto ret = enginePtr->InitSink(static_cast<uint32_t>(DIRECT_PHASE2_CH_STEREO), DIRECT_PHASE2_FMT_F32LE,
        static_cast<uint32_t>(DIRECT_PHASE2_RATE_48K), DIRECT_PHASE2_LAYOUT_STEREO);
    // sink->Init succeeds (direct_playback_engine.cpp:299-303), SetVolume runs
    // (line 305), and uChannel_/uSampleRate_/format_ are stored (lines 306-308).
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(seed.fake->initCalls, 1);
    EXPECT_EQ(seed.fake->setVolumeCalls, 1);
    EXPECT_EQ(enginePtr->uChannel_, static_cast<uint32_t>(DIRECT_PHASE2_CH_STEREO));
    EXPECT_EQ(enginePtr->uSampleRate_, static_cast<uint32_t>(DIRECT_PHASE2_RATE_48K));
    EXPECT_EQ(enginePtr->format_, enginePtr->GetDirectFormatByteSize(DIRECT_PHASE2_FMT_F32LE));

    enginePtr->renderId_ = HDI_INVALID_ID; // neutralize destructor's ReleaseId before seed erase
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_028
 * @tc.desc  : Test DirectPlayBackEngine::AddRenderer() first-stream happy path.
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_028, TestSize.Level2)
{
    SeededRenderSink seed(DIRECT_PHASE2_SINK_NAME);
    auto enginePtr = std::make_shared<DirectPlayBackEngine>();
    ASSERT_TRUE(enginePtr != nullptr);
    enginePtr->isInit_ = false;
    enginePtr->stream_ = nullptr;
    // renderId_ starts uninitialized; force it to HDI_INVALID_ID so the
    // ReleaseId inside InitSink(streamInfo) is a no-op (cpp line 273).
    enginePtr->renderId_ = HDI_INVALID_ID;

    AudioProcessConfig config;
    config.streamInfo.channels = DIRECT_PHASE2_CH_STEREO;
    config.streamInfo.samplingRate = DIRECT_PHASE2_RATE_48K;
    config.streamInfo.format = DIRECT_PHASE2_FMT_F32LE;
    config.streamInfo.channelLayout = DIRECT_PHASE2_LAYOUT_STEREO;
    auto stream = std::make_shared<ProRendererStreamImpl>(config, true);
    ASSERT_TRUE(stream != nullptr);

    auto ret = enginePtr->AddRenderer(stream);
    // !stream_ -> InitSink(streamInfo) SUCCESS via 4-arg overload using the
    // seeded fake -> stream_=stream, isInit_=true, RegisterWriteCallback bound
    // (direct_playback_engine.cpp:208-216).
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(enginePtr->isInit_);
    EXPECT_EQ(enginePtr->stream_, stream);
    EXPECT_EQ(seed.fake->initCalls, 1);       // InitSink(4-arg) called Init
    EXPECT_EQ(seed.fake->registerCbCalls, 1); // RegisterWriteCallback registered

    enginePtr->stream_ = nullptr; // avoid destructor touching the stream
    enginePtr->renderId_ = HDI_INVALID_ID;
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_029
 * @tc.desc  : Test DirectPlayBackEngine::Start() with a real (seeded) inited sink.
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_029, TestSize.Level2)
{
    SeededRenderSink seed(DIRECT_PHASE2_SINK_NAME);
    auto enginePtr = std::make_shared<DirectPlayBackEngine>();
    ASSERT_TRUE(enginePtr != nullptr);
    enginePtr->renderId_ = seed.id;
    enginePtr->isStart_ = false;

    auto ret = enginePtr->Start();
    // sink non-null and IsInited -> latency_=0, sink->Start(), isStart_=true
    // (direct_playback_engine.cpp:83-91).
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(enginePtr->isStart_);
    EXPECT_EQ(enginePtr->latency_, DIRECT_PHASE2_ZERO_LATENCY);
    EXPECT_EQ(seed.fake->startCalls, 1);

    enginePtr->renderId_ = HDI_INVALID_ID;
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_030
 * @tc.desc  : Test DirectPlayBackEngine::Stop() drives sink->Stop() via StopAudioSink.
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_030, TestSize.Level2)
{
    SeededRenderSink seed(DIRECT_PHASE2_SINK_NAME);
    auto enginePtr = std::make_shared<DirectPlayBackEngine>();
    ASSERT_TRUE(enginePtr != nullptr);
    enginePtr->renderId_ = seed.id;
    enginePtr->isStart_ = true;

    auto ret = enginePtr->Stop();
    // isStart_ true -> StopAudioSink -> sink non-null && IsInited -> sink->Stop()
    // (direct_playback_engine.cpp:98-108, 111-121).
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_FALSE(enginePtr->isStart_);
    EXPECT_EQ(seed.fake->stopCalls, 1);

    enginePtr->renderId_ = HDI_INVALID_ID;
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_031
 * @tc.desc  : Test DirectPlayBackEngine::Pause() drives sink->Stop() via StopAudioSink.
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_031, TestSize.Level2)
{
    SeededRenderSink seed(DIRECT_PHASE2_SINK_NAME);
    auto enginePtr = std::make_shared<DirectPlayBackEngine>();
    ASSERT_TRUE(enginePtr != nullptr);
    enginePtr->renderId_ = seed.id;
    enginePtr->isStart_ = true;

    auto ret = enginePtr->Pause();
    // isStart_ true -> StopAudioSink -> sink->Stop() (direct_playback_engine.cpp:126-136).
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_FALSE(enginePtr->isStart_);
    EXPECT_EQ(seed.fake->stopCalls, 1);

    enginePtr->renderId_ = HDI_INVALID_ID;
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_032
 * @tc.desc  : Test DirectPlayBackEngine::StopAudioSink() with an inited seeded sink.
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_032, TestSize.Level2)
{
    SeededRenderSink seed(DIRECT_PHASE2_SINK_NAME);
    auto enginePtr = std::make_shared<DirectPlayBackEngine>();
    ASSERT_TRUE(enginePtr != nullptr);
    enginePtr->renderId_ = seed.id;

    auto ret = enginePtr->StopAudioSink();
    // sink non-null && IsInited -> sink->Stop() (direct_playback_engine.cpp:113-116).
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(seed.fake->stopCalls, 1);

    enginePtr->renderId_ = HDI_INVALID_ID;
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_033
 * @tc.desc  : Test DirectPlayBackEngine::RegisterWriteCallback() binds the direct callback.
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_033, TestSize.Level2)
{
    SeededRenderSink seed(DIRECT_PHASE2_SINK_NAME);
    auto enginePtr = std::make_shared<DirectPlayBackEngine>();
    ASSERT_TRUE(enginePtr != nullptr);
    enginePtr->renderId_ = seed.id;

    auto ret = enginePtr->RegisterWriteCallback();
    // sink non-null && IsInited -> RegistDirectHdiCallback bound
    // (direct_playback_engine.cpp:162-171).
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(seed.fake->registerCbCalls, 1);
    EXPECT_TRUE(seed.fake->storedCb != nullptr);

    enginePtr->renderId_ = HDI_INVALID_ID;
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_034
 * @tc.desc  : Test ~DirectPlayBackEngine stops and de-inits an inited sink.
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_034, TestSize.Level2)
{
    SeededRenderSink seed(DIRECT_PHASE2_SINK_NAME);
    auto fakePtr = seed.fake; // keep an owning handle; the map may drop its copy.
    {
        auto enginePtr = std::make_shared<DirectPlayBackEngine>();
        ASSERT_TRUE(enginePtr != nullptr);
        enginePtr->renderId_ = seed.id;
        enginePtr->isInit_ = true;
        enginePtr.reset(); // trigger ~DirectPlayBackEngine
    }
    // Destructor: sink non-null && IsInited -> sink->Stop() + sink->DeInit()
    // (direct_playback_engine.cpp:52-56).
    EXPECT_EQ(fakePtr->stopCalls, 1);
    EXPECT_EQ(fakePtr->deInitCalls, 1);
    // seed destructor erases the (already-released) map entry.
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_035
 * @tc.desc  : Test DirectPlayBackEngine::InitSink(streamInfo) cache-hit fast path.
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_035, TestSize.Level2)
{
    SeededRenderSink seed(DIRECT_PHASE2_SINK_NAME);
    auto enginePtr = std::make_shared<DirectPlayBackEngine>();
    ASSERT_TRUE(enginePtr != nullptr);
    enginePtr->renderId_ = seed.id;
    enginePtr->isInit_ = true;
    // Pre-store values that MATCH the incoming streamInfo to take the cache-hit
    // fast path. format_ uses F32LE so byte-size(4) == enum(4) (cpp line 264).
    enginePtr->uChannel_ = static_cast<uint32_t>(DIRECT_PHASE2_CH_STEREO);
    enginePtr->uSampleRate_ = static_cast<uint32_t>(DIRECT_PHASE2_RATE_48K);
    enginePtr->format_ = enginePtr->GetDirectFormatByteSize(DIRECT_PHASE2_FMT_F32LE);

    AudioStreamInfo streamInfo;
    streamInfo.channels = DIRECT_PHASE2_CH_STEREO;
    streamInfo.samplingRate = DIRECT_PHASE2_RATE_48K;
    streamInfo.format = DIRECT_PHASE2_FMT_F32LE;
    streamInfo.channelLayout = DIRECT_PHASE2_LAYOUT_STEREO;

    auto ret = enginePtr->InitSink(streamInfo);
    // isInit_ && sink && matching uChannel_/format_/uSampleRate_ -> return SUCCESS
    // without re-create (direct_playback_engine.cpp:263-271).
    EXPECT_EQ(ret, SUCCESS);
    // The cache-hit path must NOT tear the sink down.
    EXPECT_EQ(seed.fake->stopCalls, 0);
    EXPECT_EQ(seed.fake->deInitCalls, 0);

    enginePtr->renderId_ = HDI_INVALID_ID;
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_036
 * @tc.desc  : Test DirectPlayBackEngine::InitSink(streamInfo) mismatch tears down and re-inits.
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_036, TestSize.Level2)
{
    // Stale sink is what renderId_ initially points at (it must be tom down);
    // the eac3 sink is what the 4-arg re-init path picks up via GetId("eac3").
    SeededRenderSink staleSeed("direct_ut_stale");
    SeededRenderSink eac3Seed(DIRECT_PHASE2_SINK_NAME);
    auto enginePtr = std::make_shared<DirectPlayBackEngine>();
    ASSERT_TRUE(enginePtr != nullptr);
    enginePtr->renderId_ = staleSeed.id;
    enginePtr->isInit_ = true;
    // Channel intentionally mismatches the incoming streamInfo to force the
    // teardown branch (format/rate match so the mismatch is unambiguous).
    enginePtr->uChannel_ = static_cast<uint32_t>(DIRECT_PHASE2_CH_MONO);
    enginePtr->uSampleRate_ = static_cast<uint32_t>(DIRECT_PHASE2_RATE_48K);
    enginePtr->format_ = enginePtr->GetDirectFormatByteSize(DIRECT_PHASE2_FMT_F32LE);

    AudioStreamInfo streamInfo;
    streamInfo.channels = DIRECT_PHASE2_CH_STEREO; // mismatch vs uChannel_(MONO)
    streamInfo.samplingRate = DIRECT_PHASE2_RATE_48K;
    streamInfo.format = DIRECT_PHASE2_FMT_F32LE;
    streamInfo.channelLayout = DIRECT_PHASE2_LAYOUT_STEREO;

    auto ret = enginePtr->InitSink(streamInfo);
    // Mismatch -> stale sink Stop()+DeInit() (cpp lines 265-268), ReleaseId
    // (line 273), then InitSink(4-arg) re-inits via the eac3 seed and returns
    // SUCCESS (lines 277-310).
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(staleSeed.fake->stopCalls, 1);
    EXPECT_EQ(staleSeed.fake->deInitCalls, 1);
    EXPECT_EQ(eac3Seed.fake->initCalls, 1);
    EXPECT_EQ(eac3Seed.fake->setVolumeCalls, 1);
    EXPECT_EQ(enginePtr->uChannel_, static_cast<uint32_t>(DIRECT_PHASE2_CH_STEREO));

    enginePtr->renderId_ = HDI_INVALID_ID;
}
} // namespace AudioStandard
} // namespace OHOS
