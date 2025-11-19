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
#include "pa_renderer_stream_impl.h"
#include "policy_handler.h"
#include "pa_adapter_manager.h"
#include "audio_capturer_private.h"
#include "audio_system_manager.h"
#include "audio_system_manager.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {
const int32_t CAPTURER_FLAG = 10;
static std::shared_ptr<PaAdapterManager> adapterManager;

class PaRendererStreamUnitTest : public ::testing::Test {
public:
    void SetUp();
    void TearDown();
    std::shared_ptr<PaRendererStreamImpl> CreatePaRendererStreamImpl();
};
void PaRendererStreamUnitTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void PaRendererStreamUnitTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

#ifdef HAS_FEATURE_INNERCAPTURER
void LoadPaPort()
{
    AudioPlaybackCaptureConfig checkConfig;
    int32_t checkInnerCapId = 0;
    AudioSystemManager::GetInstance()->CheckCaptureLimit(checkConfig, checkInnerCapId);
}

void ReleasePaPort()
{
    AudioSystemManager::GetInstance()->ReleaseCaptureLimit(1);
}
#endif

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
    return config;
}

std::shared_ptr<PaRendererStreamImpl> PaRendererStreamUnitTest::CreatePaRendererStreamImpl()
{
    adapterManager = std::make_shared<PaAdapterManager>(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    AudioProcessConfig processConfig = GetInnerCapConfig();
    uint32_t sessionId = 123456;
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    std::shared_ptr<IRendererStream> rendererStream = adapterManager->CreateRendererStream(processConfig, stream);
    std::shared_ptr<PaRendererStreamImpl> rendererStreamImpl =
        std::static_pointer_cast<PaRendererStreamImpl>(rendererStream);
    return rendererStreamImpl;
}

/**
 * @tc.name  : Test AudioProcessProxy API
 * @tc.type  : FUNC
 * @tc.number: GetCurrentTimeStamp_001
 * @tc.desc  : Test AudioProcessProxy interface.
 */
HWTEST_F(PaRendererStreamUnitTest, GetCurrentTimeStamp_001, TestSize.Level1)
{
#ifdef HAS_FEATURE_INNERCAPTURER
    LoadPaPort();
#endif
    auto unit = CreatePaRendererStreamImpl();
    unit->paStream_ = nullptr;
    uint64_t timestamp = 0;
    int32_t ret = unit->GetCurrentTimeStamp(timestamp);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : Test GetCurrentTimeStamp
 * @tc.type  : FUNC
 * @tc.number: GetCurrentTimeStamp_002
 * @tc.desc  : Test GetCurrentTimeStamp.
 */
HWTEST_F(PaRendererStreamUnitTest, GetCurrentTimeStamp_002, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    adapterManager = std::make_shared<PaAdapterManager>(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    uint64_t timestamp = 0;
    AudioProcessConfig processConfig = GetInnerCapConfig();
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    unit->paStream_ = stream;
    int32_t ret = unit->GetCurrentTimeStamp(timestamp);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test GetEffectModeName
 * @tc.type  : FUNC
 * @tc.number: GetEffectModeName_003
 * @tc.desc  : Test GetEffectModeName.
 */
HWTEST_F(PaRendererStreamUnitTest, GetEffectModeName_003, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    int32_t effectMode = 0;
    EXPECT_EQ("EFFECT_NONE", unit->GetEffectModeName(effectMode));
}

/**
 * @tc.name  : Test GetEffectModeName
 * @tc.type  : FUNC
 * @tc.number: GetEffectModeName_004
 * @tc.desc  : Test GetEffectModeName.
 */
HWTEST_F(PaRendererStreamUnitTest, GetEffectModeName_004, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    int32_t effectMode = 1;
    EXPECT_EQ("EFFECT_DEFAULT", unit->GetEffectModeName(effectMode));
}

/**
 * @tc.name  : Test AudioProcessProxy API
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_006
 * @tc.desc  : Test AudioProcessProxy interface.
 */
HWTEST_F(PaRendererStreamUnitTest, PaRenderer_006, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    adapterManager = std::make_shared<PaAdapterManager>(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    AudioProcessConfig processConfig = GetInnerCapConfig();
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    unit->paStream_ = stream;
    int32_t rate = RENDER_RATE_NORMAL;
    EXPECT_EQ(unit->SetRate(rate), SUCCESS);
}

/**
 * @tc.name  : Test AudioProcessProxy API
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_007
 * @tc.desc  : Test AudioProcessProxy interface.
 */
HWTEST_F(PaRendererStreamUnitTest, PaRenderer_007, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    adapterManager = std::make_shared<PaAdapterManager>(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    AudioProcessConfig processConfig = GetInnerCapConfig();
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    unit->paStream_ = stream;
    int32_t rate = RENDER_RATE_DOUBLE;
    EXPECT_EQ(unit->SetRate(rate), SUCCESS);
}


/**
 * @tc.name  : Test AudioProcessProxy API
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_008
 * @tc.desc  : Test AudioProcessProxy interface.
 */
HWTEST_F(PaRendererStreamUnitTest, PaRenderer_008, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    adapterManager = std::make_shared<PaAdapterManager>(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    AudioProcessConfig processConfig = GetInnerCapConfig();
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    unit->paStream_ = stream;
    int32_t rate = RENDER_RATE_HALF;
    EXPECT_EQ(unit->SetRate(rate), SUCCESS);
}

/**
 * @tc.name  : Test AudioProcessProxy API
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_009
 * @tc.desc  : Test AudioProcessProxy interface.
 */
HWTEST_F(PaRendererStreamUnitTest, PaRenderer_009, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    adapterManager = std::make_shared<PaAdapterManager>(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    AudioProcessConfig processConfig = GetInnerCapConfig();
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    unit->paStream_ = stream;
    int32_t rate = 999;
    EXPECT_EQ(unit->SetRate(rate), ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test GetCurrentPosition
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_010
 * @tc.desc  : Test GetCurrentPosition.
 */
HWTEST_F(PaRendererStreamUnitTest, PaRenderer_010, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    unit->paStream_ = nullptr;
    uint64_t timestamp = 0;
    uint64_t framePosition = 0;
    uint64_t latency = 0;
    int32_t ret = unit->GetCurrentPosition(framePosition, timestamp, latency, Timestamp::MONOTONIC);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : Test GetCurrentPosition.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_011
 * @tc.desc  : Test GetCurrentPosition.
 */
HWTEST_F(PaRendererStreamUnitTest, PaRenderer_011, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    adapterManager = std::make_shared<PaAdapterManager>(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    AudioProcessConfig processConfig = GetInnerCapConfig();
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    unit->paStream_ = stream;
    unit->firstGetLatency_ = true;
    int32_t rate = RENDER_RATE_NORMAL;
    EXPECT_EQ(unit->SetRate(rate), SUCCESS);
}

/**
 * @tc.name  : Test InitPaStream for different adapter manager type.
 * @tc.type  : FUNC
 * @tc.number: InitPaStream_001
 * @tc.desc  : Test InitPaStream for DUP_PLAYBACK
 */
HWTEST_F(PaRendererStreamUnitTest, InitPaStream_001, TestSize.Level1)
{
    adapterManager = std::make_shared<PaAdapterManager>(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    AudioProcessConfig processConfig = GetInnerCapConfig();
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);

    std::shared_ptr<IRendererStream> rendererStream = adapterManager->CreateRendererStream(processConfig, stream);
    std::shared_ptr<PaRendererStreamImpl> rendererStreamImpl =
        std::static_pointer_cast<PaRendererStreamImpl>(rendererStream);

    int32_t rate = RENDER_RATE_NORMAL;
    EXPECT_EQ(rendererStreamImpl->SetRate(rate), SUCCESS);
}

/**
 * @tc.name  : Test InitPaStream for different adapter manager type.
 * @tc.type  : FUNC
 * @tc.number: InitPaStream_002
 * @tc.desc  : Test InitPaStream for DUAL_PLAYBACK
 */
HWTEST_F(PaRendererStreamUnitTest, InitPaStream_002, TestSize.Level1)
{
    adapterManager = std::make_shared<PaAdapterManager>(DUAL_PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    AudioProcessConfig processConfig = GetInnerCapConfig();
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);

    std::shared_ptr<IRendererStream> rendererStream = adapterManager->CreateRendererStream(processConfig, stream);
    std::shared_ptr<PaRendererStreamImpl> rendererStreamImpl =
        std::static_pointer_cast<PaRendererStreamImpl>(rendererStream);

    int32_t rate = RENDER_RATE_NORMAL;
    EXPECT_EQ(rendererStreamImpl->SetRate(rate), SUCCESS);
}

/**
 * @tc.name  : Test InitPaStream for different adapter manager type.
 * @tc.type  : FUNC
 * @tc.number: InitPaStream_003
 * @tc.desc  : Test InitPaStream for PLAYBACK
 */
HWTEST_F(PaRendererStreamUnitTest, InitPaStream_003, TestSize.Level1)
{
    adapterManager = std::make_shared<PaAdapterManager>(PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    AudioProcessConfig processConfig = GetInnerCapConfig();
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);

    std::shared_ptr<IRendererStream> rendererStream = adapterManager->CreateRendererStream(processConfig, stream);
    std::shared_ptr<PaRendererStreamImpl> rendererStreamImpl =
        std::static_pointer_cast<PaRendererStreamImpl>(rendererStream);

    int32_t rate = RENDER_RATE_NORMAL;
    EXPECT_EQ(rendererStreamImpl->SetRate(rate), SUCCESS);
}

/**
 * @tc.name  : Test OffloadSetVolume.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_013
 * @tc.desc  : Test OffloadSetVolume.
 */
HWTEST_F(PaRendererStreamUnitTest, PaRenderer_013, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    unit->offloadEnable_ = false;
    auto ret = unit->OffloadSetVolume();
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test UpdateSpatializationState.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_015
 * @tc.desc  : Test UpdateSpatializationState.
 */
HWTEST_F(PaRendererStreamUnitTest, PaRenderer_015, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    unit->paStream_ = nullptr;
    bool spatializationEnabled = false;
    bool headTrackingEnabled = false;
    int32_t ret = unit->UpdateSpatializationState(spatializationEnabled, headTrackingEnabled);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : Test UpdateSpatializationState.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_016
 * @tc.desc  : Test UpdateSpatializationState.
 */
HWTEST_F(PaRendererStreamUnitTest, PaRenderer_016, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    adapterManager = std::make_shared<PaAdapterManager>(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    AudioProcessConfig processConfig = GetInnerCapConfig();
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    unit->paStream_ = stream;
    bool spatializationEnabled = false;
    bool headTrackingEnabled = false;
    int32_t ret = unit->UpdateSpatializationState(spatializationEnabled, headTrackingEnabled);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test GetOffloadApproximatelyCacheTime.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_017
 * @tc.desc  : Test GetOffloadApproximatelyCacheTime.
 */
HWTEST_F(PaRendererStreamUnitTest, PaRenderer_017, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    adapterManager = std::make_shared<PaAdapterManager>(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    AudioProcessConfig processConfig = GetInnerCapConfig();
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    unit->paStream_ = stream;
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
 * @tc.number: PaRenderer_018
 * @tc.desc  : Test GetOffloadApproximatelyCacheTime.
 */
HWTEST_F(PaRendererStreamUnitTest, PaRenderer_018, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    unit->paStream_ = nullptr;
    unit->offloadEnable_ = true;
    uint64_t timestamp = 0;
    uint64_t paWriteIndex = 0;
    uint64_t cacheTimeDsp = 0;
    uint64_t cacheTimePa = 0;
    int32_t ret = unit->GetOffloadApproximatelyCacheTime(timestamp, paWriteIndex, cacheTimeDsp, cacheTimePa);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : Test GetOffloadApproximatelyCacheTime.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_019
 * @tc.desc  : Test GetOffloadApproximatelyCacheTime.
 */
HWTEST_F(PaRendererStreamUnitTest, PaRenderer_019, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    adapterManager = std::make_shared<PaAdapterManager>(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    AudioProcessConfig processConfig = GetInnerCapConfig();
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    unit->paStream_ = stream;
    unit->offloadEnable_ = true;
    uint64_t timestamp = 0;
    uint64_t paWriteIndex = 0;
    uint64_t cacheTimeDsp = 0;
    uint64_t cacheTimePa = 0;
    int32_t ret = unit->GetOffloadApproximatelyCacheTime(timestamp, paWriteIndex, cacheTimeDsp, cacheTimePa);
    EXPECT_EQ(ret,  SUCCESS);
}

/**
 * @tc.name  : Test OffloadUpdatePolicyInWrite.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_021
 * @tc.desc  : Test OffloadUpdatePolicyInWrite.
 */
HWTEST_F(PaRendererStreamUnitTest, PaRenderer_021, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    adapterManager = std::make_shared<PaAdapterManager>(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    AudioProcessConfig processConfig = GetInnerCapConfig();
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    unit->paStream_ = stream;
    unit->offloadEnable_ = true;
    unit->lastOffloadUpdateFinishTime_ = 0;
    int32_t ret = unit->OffloadUpdatePolicyInWrite();
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name  : Test OffloadUpdatePolicy.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_022
 * @tc.desc  : Test OffloadUpdatePolicy.
 */
HWTEST_F(PaRendererStreamUnitTest, PaRenderer_022, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    AudioOffloadType statePolicy = OFFLOAD_ACTIVE_FOREGROUND;
    bool force = true;
    unit->offloadStatePolicy_ = OFFLOAD_ACTIVE_FOREGROUND;
    unit->lastOffloadUpdateFinishTime_ = 1;
    unit->OffloadUpdatePolicy(statePolicy, force);
    EXPECT_EQ(0, unit->lastOffloadUpdateFinishTime_);
}

/**
 * @tc.name  : Test OffloadUpdatePolicy.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_023
 * @tc.desc  : Test OffloadUpdatePolicy.
 */
HWTEST_F(PaRendererStreamUnitTest, PaRenderer_023, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    AudioOffloadType statePolicy = OFFLOAD_ACTIVE_FOREGROUND;
    bool force = false;
    unit->offloadStatePolicy_ = OFFLOAD_DEFAULT;
    unit->lastOffloadUpdateFinishTime_ = 1;
    unit->OffloadUpdatePolicy(statePolicy, force);
    EXPECT_EQ(0, unit->lastOffloadUpdateFinishTime_);
}

/**
 * @tc.name  : Test OffloadUpdatePolicy.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_024
 * @tc.desc  : Test OffloadUpdatePolicy.
 */
HWTEST_F(PaRendererStreamUnitTest, PaRenderer_024, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    AudioOffloadType statePolicy = OFFLOAD_DEFAULT;
    bool force = false;
    unit->offloadStatePolicy_ = OFFLOAD_ACTIVE_FOREGROUND;
    unit->lastOffloadUpdateFinishTime_ = 1;
    unit->OffloadUpdatePolicy(statePolicy, force);
    EXPECT_EQ(0, unit->lastOffloadUpdateFinishTime_);
}

/**
 * @tc.name  : Test OffloadUpdatePolicy.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_025
 * @tc.desc  : Test OffloadUpdatePolicy.
 */
HWTEST_F(PaRendererStreamUnitTest, PaRenderer_025, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    AudioOffloadType statePolicy = OFFLOAD_ACTIVE_FOREGROUND;
    bool force = true;
    unit->offloadStatePolicy_ = OFFLOAD_ACTIVE_FOREGROUND;
    unit->paStream_ = nullptr;
    int32_t ret = unit->OffloadUpdatePolicy(statePolicy, force);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : Test SetClientVolume.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_026
 * @tc.desc  : Test SetClientVolume.
 */
HWTEST_F(PaRendererStreamUnitTest, PaRenderer_026, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    float clientVolume = -1;
    int32_t ret = unit->SetClientVolume(clientVolume);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test SetClientVolume.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_027
 * @tc.desc  : Test SetClientVolume.
 */
HWTEST_F(PaRendererStreamUnitTest, PaRenderer_027, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    float clientVolume = 1.5;
    int32_t ret = unit->SetClientVolume(clientVolume);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test SetClientVolume.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_028
 * @tc.desc  : Test SetClientVolume.
 */
HWTEST_F(PaRendererStreamUnitTest, PaRenderer_028, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    float clientVolume = 0.5;
    int32_t ret = unit->SetClientVolume(clientVolume);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test GetWritableSize.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_029
 * @tc.desc  : Test GetWritableSize.
 */
HWTEST_F(PaRendererStreamUnitTest, PaRenderer_029, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    unit->paStream_ = nullptr;
    int32_t ret = unit->GetWritableSize();
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name  : Test GetWritableSize.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_030
 * @tc.desc  : Test GetWritableSize.
 */
HWTEST_F(PaRendererStreamUnitTest, PaRenderer_030, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    adapterManager = std::make_shared<PaAdapterManager>(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    AudioProcessConfig processConfig = GetInnerCapConfig();
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    unit->paStream_ = stream;
    int32_t ret = unit->GetWritableSize();
    EXPECT_EQ(ret, pa_stream_writable_size(unit->paStream_));
}

/**
 * @tc.name  : Test EnqueueBuffer.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_031
 * @tc.desc  : Test EnqueueBuffer.
 */
HWTEST_F(PaRendererStreamUnitTest, PaRenderer_031, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    BufferDesc bufferDesc = {
        .buffer = nullptr,
        .bufLength = 0,
        .dataLength = 0,
        .metaBuffer = nullptr,
        .metaLength = 0
    };
    unit->paStream_ = nullptr;
    int32_t ret = unit->EnqueueBuffer(bufferDesc);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : Test EnqueueBuffer.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_032
 * @tc.desc  : Test EnqueueBuffer.
 */
HWTEST_F(PaRendererStreamUnitTest, PaRenderer_032, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    adapterManager = std::make_shared<PaAdapterManager>(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    AudioProcessConfig processConfig = GetInnerCapConfig();
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    unit->paStream_ = stream;
    BufferDesc bufferDesc = {
        .buffer = nullptr,
        .bufLength = 0,
        .dataLength = 0,
        .metaBuffer = nullptr,
        .metaLength = 0
    };
    int32_t ret = unit->EnqueueBuffer(bufferDesc);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetAudioEffectMode.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_033
 * @tc.desc  : Test SetAudioEffectMode.
 */
HWTEST_F(PaRendererStreamUnitTest, PaRenderer_033, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    unit->paStream_ = nullptr;
    int32_t effectMode = 0;
    int32_t ret = unit->SetAudioEffectMode(effectMode);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : Test SetAudioEffectMode.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_034
 * @tc.desc  : Test SetAudioEffectMode.
 */
HWTEST_F(PaRendererStreamUnitTest, PaRenderer_034, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    adapterManager = std::make_shared<PaAdapterManager>(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    AudioProcessConfig processConfig = GetInnerCapConfig();
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    unit->paStream_ = stream;
    int32_t effectMode = 0;
    int32_t ret = unit->SetAudioEffectMode(effectMode);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test Start.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_036
 * @tc.desc  : Test Start.
 */
HWTEST_F(PaRendererStreamUnitTest, PaRenderer_036, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    unit->paStream_ = nullptr;
    int32_t ret = unit->Start();
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : Test Pause.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_037
 * @tc.desc  : Test Pause.
 */
HWTEST_F(PaRendererStreamUnitTest, PaRenderer_037, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    unit->paStream_ = nullptr;
    bool isStandby = false;
    int32_t ret = unit->Pause(isStandby);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : Test Pause.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_038
 * @tc.desc  : Test Pause.
 */
HWTEST_F(PaRendererStreamUnitTest, PaRenderer_038, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    adapterManager = std::make_shared<PaAdapterManager>(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    AudioProcessConfig processConfig = GetInnerCapConfig();
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    unit->paStream_ = stream;
    bool isStandby = false;
    int32_t ret = unit->Pause(isStandby);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test Flush.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_039
 * @tc.desc  : Test Flush.
 */
HWTEST_F(PaRendererStreamUnitTest, PaRenderer_039, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    unit->paStream_ = nullptr;
    int32_t ret = unit->Flush();
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : Test Flush.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_040
 * @tc.desc  : Test Flush.
 */
HWTEST_F(PaRendererStreamUnitTest, PaRenderer_040, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    adapterManager = std::make_shared<PaAdapterManager>(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    AudioProcessConfig processConfig = GetInnerCapConfig();
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    unit->paStream_ = stream;
    int32_t ret = unit->Flush();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test Stop.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_041
 * @tc.desc  : Test Stop.
 */
HWTEST_F(PaRendererStreamUnitTest, PaRenderer_041, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    unit->paStream_ = nullptr;
    int32_t ret = unit->Stop();
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : Test Stop.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_042
 * @tc.desc  : Test Stop.
 */
HWTEST_F(PaRendererStreamUnitTest, PaRenderer_042, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    adapterManager = std::make_shared<PaAdapterManager>(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    AudioProcessConfig processConfig = GetInnerCapConfig();
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    unit->paStream_ = stream;
    int32_t ret = unit->Stop();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test Release.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_043
 * @tc.desc  : Test Release.
 */
HWTEST_F(PaRendererStreamUnitTest, PaRenderer_043, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    unit->paStream_ = nullptr;
    unit->state_ = RUNNING;
    int32_t ret = unit->Release();
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : Test Release.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_044
 * @tc.desc  : Test Release.
 */
HWTEST_F(PaRendererStreamUnitTest, PaRenderer_044, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    adapterManager = std::make_shared<PaAdapterManager>(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    AudioProcessConfig processConfig = GetInnerCapConfig();
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    unit->paStream_ = stream;
    unit->state_ = RUNNING;
    int32_t ret = unit->Release();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test GetLatency.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_048
 * @tc.desc  : Test GetLatency.
 */
HWTEST_F(PaRendererStreamUnitTest, PaRenderer_048, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    uint64_t latency = 0;
    unit->firstGetLatency_= true;
    unit->paStream_ = nullptr;
    int32_t ret = unit->GetLatency(latency);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : Test GetLatency.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_049
 * @tc.desc  : Test GetLatency.
 */
HWTEST_F(PaRendererStreamUnitTest, PaRenderer_049, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    adapterManager = std::make_shared<PaAdapterManager>(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    AudioProcessConfig processConfig = GetInnerCapConfig();
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    unit->paStream_ = stream;
    uint64_t latency = 0;
    unit->firstGetLatency_= true;
    int32_t ret = unit->GetLatency(latency);
    EXPECT_EQ(ret, SUCCESS);
}
/**
 * @tc.name  : Test Drain.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_050
 * @tc.desc  : Test Drain.
 */
HWTEST_F(PaRendererStreamUnitTest, PaRenderer_050, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    unit->paStream_ = nullptr;
    int32_t ret = unit->Drain();
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : Test SetRate.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_051
 * @tc.desc  : Test SetRate.
 */
HWTEST_F(PaRendererStreamUnitTest, PaRenderer_051, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    unit->paStream_ = nullptr;
    int32_t rate = RENDER_RATE_NORMAL;
    EXPECT_EQ(unit->SetRate(rate), ERR_ILLEGAL_STATE);
#ifdef HAS_FEATURE_INNERCAPTURER
    ReleasePaPort();
#endif
}

/**
 * @tc.name  : Test Pause.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_052
 * @tc.desc  : Test Pause.
 */
HWTEST_F(PaRendererStreamUnitTest, PaRenderer_052, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    unit->paStream_ = nullptr;
    unit->offloadEnable_ = false;
    bool isStandby = false;
    int32_t ret = unit->Pause(isStandby);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}
}
}