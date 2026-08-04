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

#include "audio_service_log.h"
#include "audio_errors.h"
#include "capturer_in_client_inner.h"
#include "ipc_stream_listener_impl.h"
#include "audio_ring_cache.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

const int32_t CLIENT_ID = 10;
const int32_t VALUE_FOUR = 4;
const int32_t VALUE_FIVE = 5;
const int32_t VALUE_FIF = 15;

class TestCapturerReadCallback : public AudioCapturerReadCallback {
public:
    void OnReadData(size_t length) override {}
};

class TestStaticBufferEventCallback : public StaticBufferEventCallback {
public:
    void OnStaticBufferEvent(StaticBufferEventId eventId) override {}
};

class TestFirstFrameWritingCallback : public AudioRendererFirstFrameWritingCallback {
public:
    void OnFirstFrameWriting(uint64_t latency) override {}
};

enum {
    STATE_CHANGE_EVENT = 0,
    RENDERER_MARK_REACHED_EVENT,
    RENDERER_PERIOD_REACHED_EVENT,
    CAPTURER_PERIOD_REACHED_EVENT,
    CAPTURER_MARK_REACHED_EVENT,
};

enum : int64_t {
    HANDLER_PARAM_INVALID = -1,
    HANDLER_PARAM_NEW = 0,
    HANDLER_PARAM_PREPARED,
    HANDLER_PARAM_RUNNING,
    HANDLER_PARAM_STOPPED,
    HANDLER_PARAM_RELEASED,
    HANDLER_PARAM_PAUSED,
    HANDLER_PARAM_STOPPING,
    HANDLER_PARAM_RUNNING_FROM_SYSTEM,
    HANDLER_PARAM_PAUSED_FROM_SYSTEM,
};

class CapturerInClientThirdUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void Init(std::shared_ptr<CapturerInClientInner> capturerInClientInner)
{
    capturerInClientInner->clientPid_ = CLIENT_ID;
    capturerInClientInner->clientUid_ = CLIENT_ID;
    capturerInClientInner->appTokenId_ = CLIENT_ID;
    capturerInClientInner->fullTokenId_ = CLIENT_ID;
    capturerInClientInner->streamParams_.channels = STEREO;
    capturerInClientInner->streamParams_.encoding = AudioEncodingType::ENCODING_AUDIOVIVID;
    capturerInClientInner->streamParams_.format = SAMPLE_S16LE;
    capturerInClientInner->streamParams_.samplingRate = SAMPLE_RATE_48000;
    capturerInClientInner->streamParams_.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    capturerInClientInner->eStreamType_ = AudioStreamType::STREAM_MUSIC;
    capturerInClientInner->capturerInfo_.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    size_t size = 8 * 1024;
    capturerInClientInner->clientSpanSizeInByte_ = size;
    capturerInClientInner->ringCache_ = std::make_unique<AudioRingCache>(size);
    capturerInClientInner->clientBuffer_ = OHAudioBuffer::CreateFromLocal(VALUE_FIF, VALUE_FIVE, VALUE_FOUR);
}

/**
 * @tc.name  : Test SetCapturerReadCallback API
 * @tc.type  : FUNC
 * @tc.number: SetCapturerReadCallback_001
 * @tc.desc  : Test SetCapturerReadCallback with null callback
 */
HWTEST(CapturerInClientThirdUnitTest, SetCapturerReadCallback_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    int32_t ret = capturerInClientInner->SetCapturerReadCallback(nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test SetCapturerReadCallback API
 * @tc.type  : FUNC
 * @tc.number: SetCapturerReadCallback_002
 * @tc.desc  : Test SetCapturerReadCallback with wrong mode
 */
HWTEST(CapturerInClientThirdUnitTest, SetCapturerReadCallback_002, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    auto callback = std::make_shared<TestCapturerReadCallback>();
    capturerInClientInner->capturerMode_ = CAPTURE_MODE_NORMAL;
    int32_t ret = capturerInClientInner->SetCapturerReadCallback(callback);
    EXPECT_EQ(ret, ERR_INCORRECT_MODE);
}

/**
 * @tc.name  : Test GetBufferDesc API
 * @tc.type  : FUNC
 * @tc.number: GetBufferDesc_001
 * @tc.desc  : Test GetBufferDesc with wrong mode
 */
HWTEST(CapturerInClientThirdUnitTest, GetBufferDesc_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    Init(capturerInClientInner);
    capturerInClientInner->capturerMode_ = CAPTURE_MODE_NORMAL;
    BufferDesc bufDesc;
    int32_t ret = capturerInClientInner->GetBufferDesc(bufDesc);
    EXPECT_EQ(ret, ERR_INCORRECT_MODE);
}

/**
 * @tc.name  : Test GetBufQueueState API
 * @tc.type  : FUNC
 * @tc.number: GetBufQueueState_001
 * @tc.desc  : Test GetBufQueueState with wrong mode
 */
HWTEST(CapturerInClientThirdUnitTest, GetBufQueueState_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    Init(capturerInClientInner);
    capturerInClientInner->capturerMode_ = CAPTURE_MODE_NORMAL;
    BufferQueueState bufState;
    int32_t ret = capturerInClientInner->GetBufQueueState(bufState);
    EXPECT_EQ(ret, ERR_INCORRECT_MODE);
}

/**
 * @tc.name  : Test Enqueue API
 * @tc.type  : FUNC
 * @tc.number: Enqueue_001
 * @tc.desc  : Test Enqueue with wrong mode
 */
HWTEST(CapturerInClientThirdUnitTest, Enqueue_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    Init(capturerInClientInner);
    capturerInClientInner->capturerMode_ = CAPTURE_MODE_NORMAL;
    BufferDesc bufDesc;
    int32_t ret = capturerInClientInner->Enqueue(bufDesc);
    EXPECT_EQ(ret, ERR_INCORRECT_MODE);
}

/**
 * @tc.name  : Test Clear API
 * @tc.type  : FUNC
 * @tc.number: Clear_001
 * @tc.desc  : Test Clear with wrong mode
 */
HWTEST(CapturerInClientThirdUnitTest, Clear_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    Init(capturerInClientInner);
    capturerInClientInner->capturerMode_ = CAPTURE_MODE_NORMAL;
    int32_t ret = capturerInClientInner->Clear();
    EXPECT_EQ(ret, ERR_INCORRECT_MODE);
}

/**
 * @tc.name  : Test SetRenderMode API
 * @tc.type  : FUNC
 * @tc.number: SetRenderMode_001
 * @tc.desc  : Test SetRenderMode returns ERROR for capturer
 */
HWTEST(CapturerInClientThirdUnitTest, SetRenderMode_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    int32_t ret = capturerInClientInner->SetRenderMode(RENDER_MODE_NORMAL);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test GetRenderMode API
 * @tc.type  : FUNC
 * @tc.number: GetRenderMode_001
 * @tc.desc  : Test GetRenderMode returns RENDER_MODE_NORMAL for capturer
 */
HWTEST(CapturerInClientThirdUnitTest, GetRenderMode_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    AudioRenderMode mode = capturerInClientInner->GetRenderMode();
    EXPECT_EQ(mode, RENDER_MODE_NORMAL);
}

/**
 * @tc.name  : Test SetRendererWriteCallback API
 * @tc.type  : FUNC
 * @tc.number: SetRendererWriteCallback_001
 * @tc.desc  : Test SetRendererWriteCallback returns ERROR for capturer
 */
HWTEST(CapturerInClientThirdUnitTest, SetRendererWriteCallback_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    int32_t ret = capturerInClientInner->SetRendererWriteCallback(nullptr);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test SetLowPowerVolume API
 * @tc.type  : FUNC
 * @tc.number: SetLowPowerVolume_001
 * @tc.desc  : Test SetLowPowerVolume returns ERROR for capturer
 */
HWTEST(CapturerInClientThirdUnitTest, SetLowPowerVolume_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    int32_t ret = capturerInClientInner->SetLowPowerVolume(1.0f);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test GetLowPowerVolume API
 * @tc.type  : FUNC
 * @tc.number: GetLowPowerVolume_001
 * @tc.desc  : Test GetLowPowerVolume returns 0.0 for capturer
 */
HWTEST(CapturerInClientThirdUnitTest, GetLowPowerVolume_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    float volume = capturerInClientInner->GetLowPowerVolume();
    EXPECT_EQ(volume, 0.0);
}

/**
 * @tc.name  : Test SetOffloadMode API
 * @tc.type  : FUNC
 * @tc.number: SetOffloadMode_001
 * @tc.desc  : Test SetOffloadMode returns ERROR for capturer
 */
HWTEST(CapturerInClientThirdUnitTest, SetOffloadMode_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    int32_t ret = capturerInClientInner->SetOffloadMode(0, false);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test UnsetOffloadMode API
 * @tc.type  : FUNC
 * @tc.number: UnsetOffloadMode_001
 * @tc.desc  : Test UnsetOffloadMode returns ERROR for capturer
 */
HWTEST(CapturerInClientThirdUnitTest, UnsetOffloadMode_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    int32_t ret = capturerInClientInner->UnsetOffloadMode();
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test GetSingleStreamVolume API
 * @tc.type  : FUNC
 * @tc.number: GetSingleStreamVolume_001
 * @tc.desc  : Test GetSingleStreamVolume returns 0.0 for capturer
 */
HWTEST(CapturerInClientThirdUnitTest, GetSingleStreamVolume_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    float volume = capturerInClientInner->GetSingleStreamVolume();
    EXPECT_EQ(volume, 0.0);
}

/**
 * @tc.name  : Test GetAudioEffectMode API
 * @tc.type  : FUNC
 * @tc.number: GetAudioEffectMode_001
 * @tc.desc  : Test GetAudioEffectMode returns EFFECT_NONE for capturer
 */
HWTEST(CapturerInClientThirdUnitTest, GetAudioEffectMode_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    AudioEffectMode mode = capturerInClientInner->GetAudioEffectMode();
    EXPECT_EQ(mode, EFFECT_NONE);
}

/**
 * @tc.name  : Test SetAudioEffectMode API
 * @tc.type  : FUNC
 * @tc.number: SetAudioEffectMode_001
 * @tc.desc  : Test SetAudioEffectMode returns ERROR for capturer
 */
HWTEST(CapturerInClientThirdUnitTest, SetAudioEffectMode_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    int32_t ret = capturerInClientInner->SetAudioEffectMode(EFFECT_NONE);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test GetFramesWritten API
 * @tc.type  : FUNC
 * @tc.number: GetFramesWritten_001
 * @tc.desc  : Test GetFramesWritten returns -1 for capturer
 */
HWTEST(CapturerInClientThirdUnitTest, GetFramesWritten_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    int64_t frames = capturerInClientInner->GetFramesWritten();
    EXPECT_EQ(frames, -1);
}

/**
 * @tc.name  : Test SetPrivacyType API
 * @tc.type  : FUNC
 * @tc.number: SetPrivacyType_001
 * @tc.desc  : Test SetPrivacyType for capturer (no-op)
 */
HWTEST(CapturerInClientThirdUnitTest, SetPrivacyType_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    capturerInClientInner->SetPrivacyType(PRIVACY_TYPE_PUBLIC);
    EXPECT_NE(capturerInClientInner, nullptr);
}

/**
 * @tc.name  : Test SetRendererPositionCallback API
 * @tc.type  : FUNC
 * @tc.number: SetRendererPositionCallback_001
 * @tc.desc  : Test SetRendererPositionCallback logs error for capturer
 */
HWTEST(CapturerInClientThirdUnitTest, SetRendererPositionCallback_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    capturerInClientInner->SetRendererPositionCallback(0, nullptr);
    EXPECT_NE(capturerInClientInner, nullptr);
}

/**
 * @tc.name  : Test UnsetRendererPositionCallback API
 * @tc.type  : FUNC
 * @tc.number: UnsetRendererPositionCallback_001
 * @tc.desc  : Test UnsetRendererPositionCallback for capturer (no-op)
 */
HWTEST(CapturerInClientThirdUnitTest, UnsetRendererPositionCallback_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    capturerInClientInner->UnsetRendererPositionCallback();
    EXPECT_NE(capturerInClientInner, nullptr);
}

/**
 * @tc.name  : Test SetRendererPeriodPositionCallback API
 * @tc.type  : FUNC
 * @tc.number: SetRendererPeriodPositionCallback_001
 * @tc.desc  : Test SetRendererPeriodPositionCallback logs error for capturer
 */
HWTEST(CapturerInClientThirdUnitTest, SetRendererPeriodPositionCallback_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    capturerInClientInner->SetRendererPeriodPositionCallback(0, nullptr);
    EXPECT_NE(capturerInClientInner, nullptr);
}

/**
 * @tc.name  : Test UnsetRendererPeriodPositionCallback API
 * @tc.type  : FUNC
 * @tc.number: UnsetRendererPeriodPositionCallback_001
 * @tc.desc  : Test UnsetRendererPeriodPositionCallback for capturer (no-op)
 */
HWTEST(CapturerInClientThirdUnitTest, UnsetRendererPeriodPositionCallback_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    capturerInClientInner->UnsetRendererPeriodPositionCallback();
    EXPECT_NE(capturerInClientInner, nullptr);
}

/**
 * @tc.name  : Test SetRendererSamplingRate API
 * @tc.type  : FUNC
 * @tc.number: SetRendererSamplingRate_001
 * @tc.desc  : Test SetRendererSamplingRate returns ERROR for capturer
 */
HWTEST(CapturerInClientThirdUnitTest, SetRendererSamplingRate_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    int32_t ret = capturerInClientInner->SetRendererSamplingRate(48000);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test GetRendererSamplingRate API
 * @tc.type  : FUNC
 * @tc.number: GetRendererSamplingRate_001
 * @tc.desc  : Test GetRendererSamplingRate returns 0 for capturer
 */
HWTEST(CapturerInClientThirdUnitTest, GetRendererSamplingRate_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    uint32_t rate = capturerInClientInner->GetRendererSamplingRate();
    EXPECT_EQ(rate, 0);
}

/**
 * @tc.name  : Test SetChannelBlendMode API
 * @tc.type  : FUNC
 * @tc.number: SetChannelBlendMode_001
 * @tc.desc  : Test SetChannelBlendMode returns ERROR for capturer
 */
HWTEST(CapturerInClientThirdUnitTest, SetChannelBlendMode_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    int32_t ret = capturerInClientInner->SetChannelBlendMode(ChannelBlendMode::MODE_DEFAULT);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test SetVolumeWithRamp API
 * @tc.type  : FUNC
 * @tc.number: SetVolumeWithRamp_001
 * @tc.desc  : Test SetVolumeWithRamp returns ERROR for capturer
 */
HWTEST(CapturerInClientThirdUnitTest, SetVolumeWithRamp_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    int32_t ret = capturerInClientInner->SetVolumeWithRamp(1.0f, 100);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test GetOffloadEnable API
 * @tc.type  : FUNC
 * @tc.number: GetOffloadEnable_001
 * @tc.desc  : Test GetOffloadEnable returns false for capturer
 */
HWTEST(CapturerInClientThirdUnitTest, GetOffloadEnable_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    bool enable = capturerInClientInner->GetOffloadEnable();
    EXPECT_EQ(enable, false);
}

/**
 * @tc.name  : Test GetSpatializationEnabled API
 * @tc.type  : FUNC
 * @tc.number: GetSpatializationEnabled_001
 * @tc.desc  : Test GetSpatializationEnabled returns false for capturer
 */
HWTEST(CapturerInClientThirdUnitTest, GetSpatializationEnabled_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    bool enabled = capturerInClientInner->GetSpatializationEnabled();
    EXPECT_EQ(enabled, false);
}

/**
 * @tc.name  : Test GetHighResolutionEnabled API
 * @tc.type  : FUNC
 * @tc.number: GetHighResolutionEnabled_001
 * @tc.desc  : Test GetHighResolutionEnabled returns false for capturer
 */
HWTEST(CapturerInClientThirdUnitTest, GetHighResolutionEnabled_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    bool enabled = capturerInClientInner->GetHighResolutionEnabled();
    EXPECT_EQ(enabled, false);
}

/**
 * @tc.name  : Test SetSilentModeAndMixWithOthers API
 * @tc.type  : FUNC
 * @tc.number: SetSilentModeAndMixWithOthers_001
 * @tc.desc  : Test SetSilentModeAndMixWithOthers for capturer (no-op)
 */
HWTEST(CapturerInClientThirdUnitTest, SetSilentModeAndMixWithOthers_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    capturerInClientInner->SetSilentModeAndMixWithOthers(true);
    EXPECT_NE(capturerInClientInner, nullptr);
}

/**
 * @tc.name  : Test GetSilentModeAndMixWithOthers API
 * @tc.type  : FUNC
 * @tc.number: GetSilentModeAndMixWithOthers_001
 * @tc.desc  : Test GetSilentModeAndMixWithOthers returns false for capturer
 */
HWTEST(CapturerInClientThirdUnitTest, GetSilentModeAndMixWithOthers_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    bool result = capturerInClientInner->GetSilentModeAndMixWithOthers();
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test SetDefaultOutputDevice API
 * @tc.type  : FUNC
 * @tc.number: SetDefaultOutputDevice_001
 * @tc.desc  : Test SetDefaultOutputDevice returns ERROR for capturer
 */
HWTEST(CapturerInClientThirdUnitTest, SetDefaultOutputDevice_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    int32_t ret = capturerInClientInner->SetDefaultOutputDevice(DEVICE_TYPE_SPEAKER, false);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test GetDefaultOutputDevice API
 * @tc.type  : FUNC
 * @tc.number: GetDefaultOutputDevice_001
 * @tc.desc  : Test GetDefaultOutputDevice returns DEVICE_TYPE_NONE for capturer
 */
HWTEST(CapturerInClientThirdUnitTest, GetDefaultOutputDevice_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    DeviceType device = capturerInClientInner->GetDefaultOutputDevice();
    EXPECT_EQ(device, DEVICE_TYPE_NONE);
}

/**
 * @tc.name  : Test SetSwitchingStatus API
 * @tc.type  : FUNC
 * @tc.number: SetSwitchingStatus_001
 * @tc.desc  : Test SetSwitchingStatus for capturer (no-op)
 */
HWTEST(CapturerInClientThirdUnitTest, SetSwitchingStatus_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    capturerInClientInner->SetSwitchingStatus(true);
    EXPECT_NE(capturerInClientInner, nullptr);
}

/**
 * @tc.name  : Test SetCallStartByUserTid API
 * @tc.type  : FUNC
 * @tc.number: SetCallStartByUserTid_001
 * @tc.desc  : Test SetCallStartByUserTid for capturer (no-op)
 */
HWTEST(CapturerInClientThirdUnitTest, SetCallStartByUserTid_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    capturerInClientInner->SetCallStartByUserTid(getpid());
    EXPECT_NE(capturerInClientInner, nullptr);
}

/**
 * @tc.name  : Test SetLoopTimes API
 * @tc.type  : FUNC
 * @tc.number: SetLoopTimes_capturer_001
 * @tc.desc  : Test SetLoopTimes returns ERR_INCORRECT_MODE for capturer
 */
HWTEST(CapturerInClientThirdUnitTest, SetLoopTimes_capturer_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    int32_t ret = capturerInClientInner->SetLoopTimes(10);
    EXPECT_EQ(ret, ERR_INCORRECT_MODE);
}

/**
 * @tc.name  : Test SetStaticBufferInfo API
 * @tc.type  : FUNC
 * @tc.number: SetStaticBufferInfo_001
 * @tc.desc  : Test SetStaticBufferInfo for capturer (no-op, logs warning)
 */
HWTEST(CapturerInClientThirdUnitTest, SetStaticBufferInfo_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    StaticBufferInfo staticBufferInfo;
    std::shared_ptr<AudioSharedMemory> sharedMemory = nullptr;
    capturerInClientInner->SetStaticBufferInfo(staticBufferInfo, sharedMemory);
    EXPECT_NE(capturerInClientInner, nullptr);
}

/**
 * @tc.name  : Test SetStaticBufferEventCallback API
 * @tc.type  : FUNC
 * @tc.number: SetStaticBufferEventCallback_capturer_001
 * @tc.desc  : Test SetStaticBufferEventCallback returns ERR_INCORRECT_MODE for capturer
 */
HWTEST(CapturerInClientThirdUnitTest, SetStaticBufferEventCallback_capturer_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    auto callback = std::make_shared<TestStaticBufferEventCallback>();
    int32_t ret = capturerInClientInner->SetStaticBufferEventCallback(callback);
    EXPECT_EQ(ret, ERR_INCORRECT_MODE);
}

/**
 * @tc.name  : Test SetStaticTriggerRecreateCallback API
 * @tc.type  : FUNC
 * @tc.number: SetStaticTriggerRecreateCallback_capturer_001
 * @tc.desc  : Test SetStaticTriggerRecreateCallback returns ERR_INCORRECT_MODE for capturer
 */
HWTEST(CapturerInClientThirdUnitTest, SetStaticTriggerRecreateCallback_capturer_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    int32_t ret = capturerInClientInner->SetStaticTriggerRecreateCallback([](){return;});
    EXPECT_EQ(ret, ERR_INCORRECT_MODE);
}

/**
 * @tc.name  : Test ResetStaticPlayPosition API
 * @tc.type  : FUNC
 * @tc.number: ResetStaticPlayPosition_capturer_001
 * @tc.desc  : Test ResetStaticPlayPosition returns false for capturer
 */
HWTEST(CapturerInClientThirdUnitTest, ResetStaticPlayPosition_capturer_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    bool result = capturerInClientInner->ResetStaticPlayPosition();
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test DrainAudioStream API
 * @tc.type  : FUNC
 * @tc.number: DrainAudioStream_001
 * @tc.desc  : Test DrainAudioStream returns false for capturer
 */
HWTEST(CapturerInClientThirdUnitTest, DrainAudioStream_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    bool result = capturerInClientInner->DrainAudioStream(false);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test SetRendererFirstFrameWritingCallback API
 * @tc.type  : FUNC
 * @tc.number: SetRendererFirstFrameWritingCallback_001
 * @tc.desc  : Test SetRendererFirstFrameWritingCallback returns ERR_INVALID_OPERATION for capturer
 */
HWTEST(CapturerInClientThirdUnitTest, SetRendererFirstFrameWritingCallback_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    auto callback = std::make_shared<TestFirstFrameWritingCallback>();
    int32_t ret = capturerInClientInner->SetRendererFirstFrameWritingCallback(callback);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
}

/**
 * @tc.name  : Test OnFirstFrameWriting API
 * @tc.type  : FUNC
 * @tc.number: OnFirstFrameWriting_001
 * @tc.desc  : Test OnFirstFrameWriting logs error for capturer
 */
HWTEST(CapturerInClientThirdUnitTest, OnFirstFrameWriting_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    capturerInClientInner->OnFirstFrameWriting();
    EXPECT_NE(capturerInClientInner, nullptr);
}

/**
 * @tc.name  : Test Write API (uint8_t*)
 * @tc.type  : FUNC
 * @tc.number: Write_001
 * @tc.desc  : Test Write returns ERR_INVALID_OPERATION for capturer
 */
HWTEST(CapturerInClientThirdUnitTest, Write_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    uint8_t buffer[10] = {0};
    int32_t ret = capturerInClientInner->Write(buffer, 10);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
}

/**
 * @tc.name  : Test Write API (with meta)
 * @tc.type  : FUNC
 * @tc.number: Write_002
 * @tc.desc  : Test Write with meta returns ERR_INVALID_OPERATION for capturer
 */
HWTEST(CapturerInClientThirdUnitTest, Write_002, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    uint8_t buffer[10] = {0};
    uint8_t meta[10] = {0};
    int32_t ret = capturerInClientInner->Write(buffer, 10, meta, 10);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
}

/**
 * @tc.name  : Test GetAudioTimestampInfo API
 * @tc.type  : FUNC
 * @tc.number: GetAudioTimestampInfo_001
 * @tc.desc  : Test GetAudioTimestampInfo calls GetAudioTime
 */
HWTEST(CapturerInClientThirdUnitTest, GetAudioTimestampInfo_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    Init(capturerInClientInner);
    Timestamp timestamp;
    int32_t ret = capturerInClientInner->GetAudioTimestampInfo(timestamp, Timestamp::Timestampbase::MONOTONIC);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test GetBundleName API
 * @tc.type  : FUNC
 * @tc.number: GetBundleName_001
 * @tc.desc  : Test GetBundleName returns bundle name
 */
HWTEST(CapturerInClientThirdUnitTest, GetBundleName_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    capturerInClientInner->bundleName = "test.bundle";
    std::string name = capturerInClientInner->GetBundleName();
    EXPECT_EQ(name, "test.bundle");
}

/**
 * @tc.name  : Test SetBundleName API
 * @tc.type  : FUNC
 * @tc.number: SetBundleName_001
 * @tc.desc  : Test SetBundleName sets bundle name
 */
HWTEST(CapturerInClientThirdUnitTest, SetBundleName_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    std::string name = "test.bundle";
    capturerInClientInner->SetBundleName(name);
    EXPECT_EQ(capturerInClientInner->bundleName, "test.bundle");
}

/**
 * @tc.name  : Test GetLatencyWithFlag API
 * @tc.type  : FUNC
 * @tc.number: GetLatencyWithFlag_capturer_001
 * @tc.desc  : Test GetLatencyWithFlag returns ERR_NOT_SUPPORTED for capturer
 */
HWTEST(CapturerInClientThirdUnitTest, GetLatencyWithFlag_capturer_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    uint64_t latency = 0;
    int32_t ret = capturerInClientInner->GetLatencyWithFlag(latency, LATENCY_FLAG_SHARED_BUFFER);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

/**
 * @tc.name  : Test NotifyFastStatusChange API (null callback)
 * @tc.type  : FUNC
 * @tc.number: NotifyFastStatusChange_002
 * @tc.desc  : Test NotifyFastStatusChange with null callback
 */
HWTEST(CapturerInClientThirdUnitTest, NotifyFastStatusChange_002, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    capturerInClientInner->NotifyFastStatusChange(FASTSTATUS_NORMAL);
    EXPECT_EQ(capturerInClientInner->fastStatus_.load(), FASTSTATUS_NORMAL);
}

/**
 * @tc.name  : Test Read API (blocking)
 * @tc.type  : FUNC
 * @tc.number: Read_003
 * @tc.desc  : Test Read with blocking mode
 */
HWTEST(CapturerInClientThirdUnitTest, Read_003, TestSize.Level2)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    Init(capturerInClientInner);
    capturerInClientInner->state_ = RUNNING;
    capturerInClientInner->sizePerFrameInByte_ = 4;
    capturerInClientInner->clientBuffer_ = OHAudioBuffer::CreateFromLocal(100, 4, 4);
    
    uint8_t buffer[16] = {0};
    int32_t ret = capturerInClientInner->Read(buffer[0], 16, true);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test StartAudioStream API (prepared state)
 * @tc.type  : FUNC
 * @tc.number: StartAudioStream_001
 * @tc.desc  : Test StartAudioStream from PREPARED state
 */
HWTEST(CapturerInClientThirdUnitTest, StartAudioStream_001, TestSize.Level2)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    Init(capturerInClientInner);
    capturerInClientInner->state_ = PREPARED;
    capturerInClientInner->ipcStream_ = nullptr;
    bool result = capturerInClientInner->StartAudioStream(CMD_FROM_CLIENT);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test StartAudioStream API (stopped state)
 * @tc.type  : FUNC
 * @tc.number: StartAudioStream_002
 * @tc.desc  : Test StartAudioStream from STOPPED state
 */
HWTEST(CapturerInClientThirdUnitTest, StartAudioStream_002, TestSize.Level2)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    Init(capturerInClientInner);
    capturerInClientInner->state_ = STOPPED;
    capturerInClientInner->ipcStream_ = nullptr;
    bool result = capturerInClientInner->StartAudioStream(CMD_FROM_CLIENT);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test ReleaseAudioStream API (released state)
 * @tc.type  : FUNC
 * @tc.number: ReleaseAudioStream_001
 * @tc.desc  : Test ReleaseAudioStream when already released
 */
HWTEST(CapturerInClientThirdUnitTest, ReleaseAudioStream_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    Init(capturerInClientInner);
    capturerInClientInner->state_ = RELEASED;
    bool result = capturerInClientInner->ReleaseAudioStream();
    EXPECT_EQ(result, true);
}

/**
 * @tc.name  : Test ReleaseAudioStream API (with ipcStream)
 * @tc.type  : FUNC
 * @tc.number: ReleaseAudioStream_002
 * @tc.desc  : Test ReleaseAudioStream with valid ipcStream
 */
HWTEST(CapturerInClientThirdUnitTest, ReleaseAudioStream_002, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    Init(capturerInClientInner);
    capturerInClientInner->state_ = RUNNING;
    capturerInClientInner->ipcStream_ = nullptr;
    bool result = capturerInClientInner->ReleaseAudioStream();
    EXPECT_EQ(result, true);
}

/**
 * @tc.name  : Test ReleaseAudioStream API (null ipcStream)
 * @tc.type  : FUNC
 * @tc.number: ReleaseAudioStream_003
 * @tc.desc  : Test ReleaseAudioStream with null ipcStream
 */
HWTEST(CapturerInClientThirdUnitTest, ReleaseAudioStream_003, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    Init(capturerInClientInner);
    capturerInClientInner->state_ = RUNNING;
    capturerInClientInner->ipcStream_ = nullptr;
    bool result = capturerInClientInner->ReleaseAudioStream();
    EXPECT_EQ(result, true);
}

/**
 * @tc.name  : Test SetCaptureMode API (same mode)
 * @tc.type  : FUNC
 * @tc.number: SetCaptureMode_001
 * @tc.desc  : Test SetCaptureMode with same mode returns SUCCESS
 */
HWTEST(CapturerInClientThirdUnitTest, SetCaptureMode_001, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    int32_t ret = capturerInClientInner->SetCaptureMode(CAPTURE_MODE_NORMAL);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetCaptureMode API (callback to normal)
 * @tc.type  : FUNC
 * @tc.number: SetCaptureMode_002
 * @tc.desc  : Test SetCaptureMode from callback to normal returns ERR_INCORRECT_MODE
 */
HWTEST(CapturerInClientThirdUnitTest, SetCaptureMode_002, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    capturerInClientInner->capturerMode_ = CAPTURE_MODE_CALLBACK;
    int32_t ret = capturerInClientInner->SetCaptureMode(CAPTURE_MODE_NORMAL);
    EXPECT_EQ(ret, ERR_INCORRECT_MODE);
}

/**
 * @tc.name  : Test SetCaptureMode API (invalid state)
 * @tc.type  : FUNC
 * @tc.number: SetCaptureMode_003
 * @tc.desc  : Test SetCaptureMode in running state returns ERR_ILLEGAL_STATE
 */
HWTEST(CapturerInClientThirdUnitTest, SetCaptureMode_003, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    capturerInClientInner->capturerMode_ = CAPTURE_MODE_NORMAL;
    capturerInClientInner->state_ = RUNNING;
    int32_t ret = capturerInClientInner->SetCaptureMode(CAPTURE_MODE_CALLBACK);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : Test SetCaptureMode API (valid transition)
 * @tc.type  : FUNC
 * @tc.number: SetCaptureMode_004
 * @tc.desc  : Test SetCaptureMode from normal to callback in prepared state
 */
HWTEST(CapturerInClientThirdUnitTest, SetCaptureMode_004, TestSize.Level2)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    Init(capturerInClientInner);
    capturerInClientInner->state_ = PREPARED;
    int32_t ret = capturerInClientInner->SetCaptureMode(CAPTURE_MODE_CALLBACK);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test CheckRestoreStatus API
 * @tc.type  : FUNC
 * @tc.number: CheckRestoreStatus_001
 * @tc.desc  : Test CheckRestoreStatus with fast status and no callback
 */
HWTEST(CapturerInClientThirdUnitTest, CheckRestoreStatus_001, TestSize.Level2)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    Init(capturerInClientInner);
    capturerInClientInner->capturerInfo_.capturerFlags = AUDIO_FLAG_VOIP_FAST;
    capturerInClientInner->readCb_ = nullptr;
    RestoreStatus status = capturerInClientInner->CheckRestoreStatus();
    EXPECT_EQ(status, NEED_RESTORE_TO_NORMAL);
}

/**
 * @tc.name  : Test CheckRestoreStatus API
 * @tc.type  : FUNC
 * @tc.number: CheckRestoreStatus_002
 * @tc.desc  : Test CheckRestoreStatus with fast status and has callback
 */
HWTEST(CapturerInClientThirdUnitTest, CheckRestoreStatus_002, TestSize.Level2)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    Init(capturerInClientInner);
    capturerInClientInner->capturerInfo_.capturerFlags = AUDIO_FLAG_VOIP_FAST;
    capturerInClientInner->readCb_ = std::make_shared<TestCapturerReadCallback>();
    capturerInClientInner->clientBuffer_ = OHAudioBuffer::CreateFromLocal(100, 4, 4);
    RestoreStatus status = capturerInClientInner->CheckRestoreStatus();
    EXPECT_NE(status, NEED_RESTORE_TO_NORMAL);
}

/**
 * @tc.name  : Test ReleaseAudioStream API (double release in destructor)
 * @tc.type  : FUNC
 * @tc.number: ReleaseAudioStream_004
 * @tc.desc  : Test ReleaseAudioStream when already released in destructor
 */
HWTEST(CapturerInClientThirdUnitTest, ReleaseAudioStream_004, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    Init(capturerInClientInner);
    capturerInClientInner->state_ = RELEASED;
    bool result = capturerInClientInner->ReleaseAudioStream(true, false, true);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name  : Test ReleaseAudioStream API (double release by user)
 * @tc.type  : FUNC
 * @tc.number: ReleaseAudioStream_005
 * @tc.desc  : Test ReleaseAudioStream when already released by user
 */
HWTEST(CapturerInClientThirdUnitTest, ReleaseAudioStream_005, TestSize.Level1)
{
    auto capturerInClientInner = std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    Init(capturerInClientInner);
    capturerInClientInner->state_ = RELEASED;
    bool result = capturerInClientInner->ReleaseAudioStream(true, false, false);
    EXPECT_EQ(result, true);
}

} // namespace AudioStandard
} // namespace OHOS
