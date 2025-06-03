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

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {
const int32_t CAPTURER_FLAG = 10;

class PaRendererStreamUnitTestP2 : public ::testing::Test {
public:
    void SetUp();
    void TearDown();
    std::shared_ptr<PaRendererStreamImpl> CreatePaRendererStreamImpl();
};
void PaRendererStreamUnitTestP2::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void PaRendererStreamUnitTestP2::TearDown(void)
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

std::shared_ptr<PaRendererStreamImpl> PaRendererStreamUnitTestP2::CreatePaRendererStreamImpl()
{
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
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
 * @tc.name  : Test GetCurrentPosition.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_004
 * @tc.desc  : Test GetCurrentPosition.
 */
HWTEST_F(PaRendererStreamUnitTestP2, PaRenderer_004, TestSize.Level1)
{
#ifdef HAS_FEATURE_INNERCAPTURER
    LoadPaPort();
#endif
    auto unit = CreatePaRendererStreamImpl();
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    AudioProcessConfig processConfig = GetInnerCapConfig();
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    unit->paStream_ = stream;
    if (stream == nullptr) {
        std::cout << "stream is nullptr" << std::endl;
    }
    uint64_t framePosition = 0;
    uint64_t timestamp = 0;
    uint64_t latency = 0;
    unit->firstGetPaLatency_ = true;
    unit->GetCurrentPosition(framePosition, timestamp, latency, Timestamp::MONOTONIC);
    EXPECT_EQ(false, unit->firstGetPaLatency_);
}

/**
 * @tc.name  : Test GetCurrentPosition.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_005
 * @tc.desc  : Test GetCurrentPosition.
 */
HWTEST_F(PaRendererStreamUnitTestP2, PaRenderer_005, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    AudioProcessConfig processConfig = GetInnerCapConfig();
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    unit->paStream_ = stream;
    if (stream == nullptr) {
        std::cout << "stream is nullptr" << std::endl;
    }
    uint64_t framePosition = 0;
    uint64_t timestamp = 0;
    uint64_t latency = 0;
    unit->offloadEnable_ = true;
    int32_t ret = unit->GetCurrentPosition(framePosition, timestamp, latency, Timestamp::MONOTONIC);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test GetLatency.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_006
 * @tc.desc  : Test GetLatency.
 */
HWTEST_F(PaRendererStreamUnitTestP2, PaRenderer_006, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    AudioProcessConfig processConfig = GetInnerCapConfig();
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    unit->paStream_ = stream;
    if (stream == nullptr) {
        std::cout << "stream is nullptr" << std::endl;
    }
    uint64_t latency = 0;
    unit->firstGetLatency_= false;
    int32_t ret = unit->GetLatency(latency);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test InitParams.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_007
 * @tc.desc  : Test InitParams.
 */
HWTEST_F(PaRendererStreamUnitTestP2, PaRenderer_007, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    AudioProcessConfig processConfig = GetInnerCapConfig();
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    unit->paStream_ = stream;
    int32_t ret = unit->InitParams();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test OffloadSetVolume.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_008
 * @tc.desc  : Test OffloadSetVolume.
 */
HWTEST_F(PaRendererStreamUnitTestP2, PaRenderer_008, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    AudioProcessConfig processConfig = GetInnerCapConfig();
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    unit->paStream_ = stream;
    unit->offloadEnable_ = true;
    float volume = 0.0f;
    EXPECT_NE(unit->OffloadSetVolume(volume), SUCCESS);
}

/**
 * @tc.name  : Test SetOffloadMode.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_009
 * @tc.desc  : Test SetOffloadMode.
 */
HWTEST_F(PaRendererStreamUnitTestP2, PaRenderer_009, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    AudioProcessConfig processConfig = GetInnerCapConfig();
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    unit->paStream_ = stream;
    if (stream == nullptr) {
        std::cout << "stream is nullptr" << std::endl;
    }
    int32_t state = 0;
    bool isAppBack = false;
    EXPECT_EQ(unit->SetOffloadMode(state, isAppBack), SUCCESS);
}

/**
 * @tc.name  : Test SetOffloadMode.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_010
 * @tc.desc  : Test SetOffloadMode.
 */
HWTEST_F(PaRendererStreamUnitTestP2, PaRenderer_010, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    AudioProcessConfig processConfig = GetInnerCapConfig();
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    unit->paStream_ = stream;
    if (stream == nullptr) {
        std::cout << "stream is nullptr" << std::endl;
    }
    unit->offloadNextStateTargetPolicy_ = OFFLOAD_ACTIVE_FOREGROUND;
    int32_t state = 0;
    bool isAppBack = false;
    EXPECT_EQ(unit->SetOffloadMode(state, isAppBack), SUCCESS);
}

/**
 * @tc.name  : Test SetOffloadMode.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_011
 * @tc.desc  : Test SetOffloadMode.
 */
HWTEST_F(PaRendererStreamUnitTestP2, PaRenderer_011, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    unit->paStream_ = nullptr;
    unit->offloadNextStateTargetPolicy_ = OFFLOAD_DEFAULT;
    int32_t state = 0;
    bool isAppBack = false;
    EXPECT_EQ(unit->SetOffloadMode(state, isAppBack), ERR_OPERATION_FAILED);
}
/**
 * @tc.name  : Test OffloadUpdatePolicyInWrite.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_012
 * @tc.desc  : Test OffloadUpdatePolicyInWrite.
 */
HWTEST_F(PaRendererStreamUnitTestP2, PaRenderer_012, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    AudioProcessConfig processConfig = GetInnerCapConfig();
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    unit->paStream_ = stream;
    unit->offloadEnable_ = true;
    unit->lastOffloadUpdateFinishTime_ = -1;
    int32_t ret = unit->OffloadUpdatePolicyInWrite();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test GetCurrentPosition.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_013
 * @tc.desc  : Test GetCurrentPosition.
 */
HWTEST_F(PaRendererStreamUnitTestP2, PaRenderer_013, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    AudioProcessConfig processConfig = GetInnerCapConfig();
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    unit->paStream_ = stream;
    if (stream == nullptr) {
        std::cout << "stream is nullptr" << std::endl;
    }
    uint64_t framePosition = 0;
    uint64_t timestamp = 0;
    uint64_t latency = 0;
    unit->firstGetPaLatency_ = true;
    unit->GetCurrentPosition(framePosition, timestamp, latency, Timestamp::MONOTONIC);
    EXPECT_EQ(false, unit->firstGetPaLatency_);
}

/**
 * @tc.name  : Test GetCurrentPosition.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_014
 * @tc.desc  : Test GetCurrentPosition.
 */
HWTEST_F(PaRendererStreamUnitTestP2, PaRenderer_014, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    AudioProcessConfig processConfig = GetInnerCapConfig();
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    unit->paStream_ = stream;
    if (stream == nullptr) {
        std::cout << "stream is nullptr" << std::endl;
    }
    uint64_t framePosition = 0;
    uint64_t timestamp = 0;
    uint64_t latency = 0;
    unit->offloadEnable_ = true;
    int32_t ret = unit->GetCurrentPosition(framePosition, timestamp, latency, Timestamp::MONOTONIC);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test GetLatency.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_015
 * @tc.desc  : Test GetLatency.
 */
HWTEST_F(PaRendererStreamUnitTestP2, PaRenderer_015, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    AudioProcessConfig processConfig = GetInnerCapConfig();
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    unit->paStream_ = stream;
    uint64_t latency = 0;
    unit->firstGetLatency_= false;
    int32_t ret = unit->GetLatency(latency);
    EXPECT_EQ(ret, SUCCESS);
#ifdef HAS_FEATURE_INNERCAPTURER
    ReleasePaPort();
#endif
}

/**
 * @tc.name  : Test
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_016
 * @tc.desc  : Test PAStreamMovedCb.
 */
HWTEST_F(PaRendererStreamUnitTestP2, PaRenderer_016, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    std::shared_ptr<PaAdapterManager> adapterManager = std::make_shared<PaAdapterManager>(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    AudioProcessConfig processConfig = GetInnerCapConfig();
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    unit->paStream_ = stream;
    void *userdataRet = nullptr;

    EXPECT_EQ(userdataRet, nullptr);
    unit->PAStreamMovedCb(stream, userdataRet);
    unit->PAStreamMovedCb(stream, (void *)1);
}

/**
 * @tc.name  : Test
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_017
 * @tc.desc  : Test UpdateBufferSize.
 */
HWTEST_F(PaRendererStreamUnitTestP2, PaRenderer_017, TestSize.Level1)
{
    auto unit = CreatePaRendererStreamImpl();
    uint32_t bufferLength = 10;
    int32_t ret = unit->UpdateBufferSize(bufferLength);
    EXPECT_EQ(ret, SUCCESS);
}
}
}