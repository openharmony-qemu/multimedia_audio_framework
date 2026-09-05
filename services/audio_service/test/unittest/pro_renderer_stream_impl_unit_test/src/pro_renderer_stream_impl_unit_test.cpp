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

#include <gtest/gtest.h>
#include "audio_info.h"
#include "audio_errors.h"
#include "pro_renderer_stream_impl_unit_test.h"
#include "pro_renderer_stream_impl.h"
#include "renderer_in_server.h"
#include "ipc_stream_in_server.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
constexpr int32_t DEFAULT_STREAM_ID = 10;
// 3DA HDI whole-frame contract, keep in sync with the constants in pro_renderer_stream_impl.cpp
constexpr uint32_t AVS3METADATA_SIZE = 19824;
constexpr uint32_t AUDIO_VIVID_SAMPLES = 1024;
constexpr uint32_t THREE_DA_HDI_HEADER_FRAMES = 3;
void ProRendererStreamImplUnitTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void ProRendererStreamImplUnitTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void ProRendererStreamImplUnitTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void ProRendererStreamImplUnitTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

static AudioProcessConfig InitProcessConfig()
{
    AudioProcessConfig config;
    config.appInfo.appUid = DEFAULT_STREAM_ID;
    config.appInfo.appPid = DEFAULT_STREAM_ID;
    config.streamInfo.format = SAMPLE_S32LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    return config;
}

/**
 * @tc.name  : Test GetDirectSampleRate API
 * @tc.type  : FUNC
 * @tc.number: GetDirectSampleRate_001
 */
HWTEST(ProRendererStreamImplUnitTest, GetDirectSampleRate_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    rendererStreamImpl->processConfig_.streamType = STREAM_VOICE_COMMUNICATION;
    AudioSamplingRate sampleRate = SAMPLE_RATE_16000;

    AudioSamplingRate ret = rendererStreamImpl->GetDirectSampleRate(sampleRate);
    EXPECT_EQ(ret, SAMPLE_RATE_16000);

    sampleRate = SAMPLE_RATE_48000;
    ret = rendererStreamImpl->GetDirectSampleRate(sampleRate);
    EXPECT_EQ(ret, SAMPLE_RATE_48000);
}

/**
 * @tc.name  : Test GetDirectSampleRate API
 * @tc.type  : FUNC
 * @tc.number: GetDirectSampleRate_002
 */
HWTEST(ProRendererStreamImplUnitTest, GetDirectSampleRate_002, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    rendererStreamImpl->processConfig_.streamType = STREAM_MUSIC;
    AudioSamplingRate sampleRate = SAMPLE_RATE_44100;

    AudioSamplingRate ret = rendererStreamImpl->GetDirectSampleRate(sampleRate);
    EXPECT_EQ(ret, SAMPLE_RATE_48000);

    sampleRate = SAMPLE_RATE_88200;
    ret = rendererStreamImpl->GetDirectSampleRate(sampleRate);
    EXPECT_EQ(ret, SAMPLE_RATE_96000);

    sampleRate = SAMPLE_RATE_176400;
    ret = rendererStreamImpl->GetDirectSampleRate(sampleRate);
    EXPECT_EQ(ret, SAMPLE_RATE_192000);

    sampleRate = SAMPLE_RATE_96000;
    ret = rendererStreamImpl->GetDirectSampleRate(sampleRate);
    EXPECT_EQ(ret, SAMPLE_RATE_96000);
}

/**
 * @tc.name  : Test GetDirectFormat API
 * @tc.type  : FUNC
 * @tc.number: GetDirectFormat_001
 */
HWTEST(ProRendererStreamImplUnitTest, GetDirectFormat_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    bool isDirect = false;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    AudioSampleFormat format1 = SAMPLE_S16LE;
    AudioSampleFormat format2 = SAMPLE_S32LE;
    AudioSampleFormat ret;

    ret = rendererStreamImpl->GetDirectFormat(format1);
    EXPECT_EQ(ret, SAMPLE_S16LE);

    ret = rendererStreamImpl->GetDirectFormat(format2);
    EXPECT_EQ(ret, SAMPLE_S32LE);
}

/**
 * @tc.name  : Test GetDirectFormat API
 * @tc.type  : FUNC
 * @tc.number: GetDirectFormat_002
 */
HWTEST(ProRendererStreamImplUnitTest, GetDirectFormat_002, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    AudioSampleFormat format = SAMPLE_S32LE;
    AudioSampleFormat ret;

    rendererStreamImpl->isDirect_ = true;
    ret = rendererStreamImpl->GetDirectFormat(format);
    EXPECT_EQ(ret, SAMPLE_S32LE);

    rendererStreamImpl->isDirect_ = false;
    ret = rendererStreamImpl->GetDirectFormat(format);
    EXPECT_EQ(ret, SAMPLE_S32LE);

    format = SAMPLE_S16LE;
    ret = rendererStreamImpl->GetDirectFormat(format);
    EXPECT_EQ(ret, SAMPLE_S16LE);

    format = SAMPLE_F32LE;
    ret = rendererStreamImpl->GetDirectFormat(format);
    EXPECT_EQ(ret, SAMPLE_S16LE);

    format = SAMPLE_S24LE;
    ret = rendererStreamImpl->GetDirectFormat(format);
    EXPECT_EQ(ret, SAMPLE_S32LE);
}

/**
 * @tc.name  : Test InitParams API
 * @tc.type  : FUNC
 * @tc.number: InitParams_001
 */
HWTEST(ProRendererStreamImplUnitTest, InitParams_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.streamInfo.samplingRate = SAMPLE_RATE_8000;
    processConfig.streamType = STREAM_VOICE_CALL;
    bool isDirect = false;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    rendererStreamImpl->resample_.reset();

    int32_t ret = rendererStreamImpl->InitParams();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test InitParams API
 * @tc.type  : FUNC
 * @tc.number: InitParams_002
 */
HWTEST(ProRendererStreamImplUnitTest, InitParams_002, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.streamInfo.channels = CHANNEL_4;
    processConfig.streamInfo.samplingRate = SAMPLE_RATE_8000;
    processConfig.streamType = STREAM_VOICE_CALL;
    bool isDirect = false;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);

    int32_t ret = rendererStreamImpl->InitParams();
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test InitParams API
 * @tc.type  : FUNC
 * @tc.number: InitParams_003
 */
HWTEST(ProRendererStreamImplUnitTest, InitParams_003, TestSize.Level1)
{
    AudioProcessConfig processConfig = InitProcessConfig();
    bool isDirect = false;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);

    rendererStreamImpl->status_ = I_STATUS_STARTING;
    int32_t ret = rendererStreamImpl->InitParams();
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);

    rendererStreamImpl->status_ = I_STATUS_INVALID;
    rendererStreamImpl->desSamplingRate_ = SAMPLE_RATE_16000;
    ret = rendererStreamImpl->InitParams();
    EXPECT_EQ(ret, SUCCESS);

    rendererStreamImpl->status_ = I_STATUS_INVALID;
    rendererStreamImpl->desSamplingRate_ = SAMPLE_RATE_48000;
    processConfig.streamInfo.channels = CHANNEL_4;
    ret = rendererStreamImpl->InitParams();
    EXPECT_EQ(ret, SUCCESS);

    rendererStreamImpl->status_ = I_STATUS_INVALID;
    processConfig.streamInfo.channels = STEREO;
    ret = rendererStreamImpl->InitParams();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : InitParams_Direct3DA_001
 * @tc.type  : FUNC
 * @tc.number: Test InitParams 3DA direct stream to verify bypass logic and whole-frame buffer allocation
 */
HWTEST(ProRendererStreamImplUnitTest, InitParams_Direct3DA_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.streamInfo.channels = CHANNEL_8;
    processConfig.streamInfo.samplingRate = SAMPLE_RATE_48000;
    processConfig.streamInfo.format = SAMPLE_S16LE;
    processConfig.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_5POINT1POINT2;
    processConfig.streamInfo.encoding = ENCODING_AUDIOVIVID;
    processConfig.rendererInfo.rendererFlags = AUDIO_FLAG_3DA_DIRECT;

    auto rendererStreamImpl = std::make_shared<ProRendererStreamImpl>(processConfig, false);
    rendererStreamImpl->status_ = I_STATUS_INVALID;

    int32_t ret = rendererStreamImpl->InitParams();
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_FALSE(rendererStreamImpl->isNeedResample_);
    EXPECT_FALSE(rendererStreamImpl->isNeedMcr_);
    // Each sinkBuffer slot holds one whole 3DA HDI frame: (PCM + header) frames + aligned metadata
    uint32_t frameSize = 2 * 8; // S16LE 2 bytes per sample * 8 channels
    uint32_t metaAligned = (AVS3METADATA_SIZE + frameSize - 1) / frameSize * frameSize;
    size_t expectedSlotSize = (AUDIO_VIVID_SAMPLES + THREE_DA_HDI_HEADER_FRAMES) * frameSize + metaAligned;
    EXPECT_EQ(rendererStreamImpl->sinkBuffer_[0].size(), expectedSlotSize);
}

/**
 * @tc.name  : InitParams_Direct3DA_002
 * @tc.type  : FUNC
 * @tc.number: Test InitParams AUDIOVIVID stream without 3DA direct flag goes the resample and mixer path
 */
HWTEST(ProRendererStreamImplUnitTest, InitParams_Direct3DA_002, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.streamInfo.channels = CHANNEL_10;
    processConfig.streamInfo.samplingRate = SAMPLE_RATE_44100;
    processConfig.streamInfo.format = SAMPLE_S16LE;
    processConfig.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_7POINT1POINT2;
    processConfig.streamType = STREAM_MUSIC;
    processConfig.streamInfo.encoding = ENCODING_AUDIOVIVID;
    processConfig.rendererInfo.rendererFlags = AUDIO_FLAG_NORMAL;

    auto rendererStreamImpl = std::make_shared<ProRendererStreamImpl>(processConfig, false);
    rendererStreamImpl->status_ = I_STATUS_INVALID;

    int32_t ret = rendererStreamImpl->InitParams();
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(rendererStreamImpl->isNeedResample_);
    EXPECT_TRUE(rendererStreamImpl->isNeedMcr_);
}

/**
 * @tc.name  : InitParams_Direct3DA_003
 * @tc.type  : FUNC
 * @tc.number: Test InitParams 3DA direct stream with zero channels returns ERR_INVALID_PARAM
 */
HWTEST(ProRendererStreamImplUnitTest, InitParams_Direct3DA_003, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.streamInfo.channels = CHANNEL_UNKNOW; // zero byteSizePerFrame_
    processConfig.streamInfo.samplingRate = SAMPLE_RATE_48000;
    processConfig.streamInfo.format = SAMPLE_S16LE;
    processConfig.streamInfo.encoding = ENCODING_AUDIOVIVID;
    processConfig.rendererInfo.rendererFlags = AUDIO_FLAG_3DA_DIRECT;

    auto rendererStreamImpl = std::make_shared<ProRendererStreamImpl>(processConfig, false);
    rendererStreamImpl->status_ = I_STATUS_INVALID;

    // zero channels -> zero byteSizePerFrame_ -> guard rejects before any division
    int32_t ret = rendererStreamImpl->InitParams();
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test Start API
 * @tc.type  : FUNC
 * @tc.number:Start_001
 */
HWTEST(ProRendererStreamImplUnitTest, Start_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    rendererStreamImpl->status_ = I_STATUS_STARTED;
    int32_t ret = rendererStreamImpl->Start();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test Start API
 * @tc.type  : FUNC
 * @tc.number: Start_002
 */
HWTEST(ProRendererStreamImplUnitTest, Start_002, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    rendererStreamImpl->status_ = I_STATUS_PAUSED;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = nullptr;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListenerHolder);
    rendererStreamImpl->RegisterStatusCallback(rendererInServer);
    int32_t ret = rendererStreamImpl->Start();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test Pause API
 * @tc.type  : FUNC
 * @tc.number: Pause_001
 */
HWTEST(ProRendererStreamImplUnitTest, Pause_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    rendererStreamImpl->status_ = I_STATUS_PAUSED;
    rendererStreamImpl->isFirstFrame_ = false;
    int32_t ret = rendererStreamImpl->Pause();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test Pause API
 * @tc.type  : FUNC
 * @tc.number: Pause_002
 */
HWTEST(ProRendererStreamImplUnitTest, Pause_002, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    rendererStreamImpl->status_ = I_STATUS_STARTED;
    rendererStreamImpl->isFirstFrame_ = true;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = nullptr;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListenerHolder);
    rendererStreamImpl->RegisterStatusCallback(rendererInServer);
    int32_t ret = rendererStreamImpl->Pause();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test Flush API
 * @tc.type  : FUNC
 * @tc.number: Flush
 */
HWTEST(ProRendererStreamImplUnitTest, Flush_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    rendererStreamImpl->readQueue_.push(1);
    rendererStreamImpl->isDrain_ = true;
    int32_t ret = rendererStreamImpl->Flush();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test Flush API
 * @tc.type  : FUNC
 * @tc.number: Flush
 */
HWTEST(ProRendererStreamImplUnitTest, Flush_002, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    rendererStreamImpl->readQueue_.push(1);
    rendererStreamImpl->isDrain_ = true;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = nullptr;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListenerHolder);
    rendererStreamImpl->RegisterStatusCallback(rendererInServer);
    int32_t ret = rendererStreamImpl->Flush();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test Drain API
 * @tc.type  : FUNC
 * @tc.number: Drain
 */
HWTEST(ProRendererStreamImplUnitTest, Drain_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    rendererStreamImpl->readQueue_.push(1);
    int32_t ret = rendererStreamImpl->Drain();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test Drain API
 * @tc.type  : FUNC
 * @tc.number: Drain
 */
HWTEST(ProRendererStreamImplUnitTest, Drain_002, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = nullptr;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListenerHolder);
    rendererStreamImpl->RegisterStatusCallback(rendererInServer);
    rendererStreamImpl->readQueue_.push(1);
    int32_t ret = rendererStreamImpl->Drain();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test Drain API
 * @tc.type  : FUNC
 * @tc.number: Stop
 */
HWTEST(ProRendererStreamImplUnitTest, Stop_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    rendererStreamImpl->isFirstFrame_ = false;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = nullptr;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListenerHolder);
    rendererStreamImpl->RegisterStatusCallback(rendererInServer);

    int32_t ret = rendererStreamImpl->Stop();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test Drain API
 * @tc.type  : FUNC
 * @tc.number: Stop
 */
HWTEST(ProRendererStreamImplUnitTest, Stop_002, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    rendererStreamImpl->isFirstFrame_ = true;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = nullptr;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListenerHolder);
    rendererStreamImpl->RegisterStatusCallback(rendererInServer);

    int32_t ret = rendererStreamImpl->Stop();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test Drain API
 * @tc.type  : FUNC
 * @tc.number: Release
 */
HWTEST(ProRendererStreamImplUnitTest, Release_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = nullptr;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListenerHolder);
    rendererStreamImpl->RegisterStatusCallback(rendererInServer);

    int32_t ret = rendererStreamImpl->Release();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetRate API
 * @tc.type  : FUNC
 * @tc.number: SetRate
 */
HWTEST(ProRendererStreamImplUnitTest, SetRate_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);

    int32_t rate = RENDER_RATE_NORMAL;
    int32_t ret = rendererStreamImpl->SetRate(rate);
    EXPECT_EQ(ret, SUCCESS);

    rate = RENDER_RATE_DOUBLE;
    ret = rendererStreamImpl->SetRate(rate);
    EXPECT_EQ(ret, SUCCESS);

    rate = RENDER_RATE_HALF;
    ret = rendererStreamImpl->SetRate(rate);
    EXPECT_EQ(ret, SUCCESS);

    rate = static_cast<AudioRendererRate>(3);;
    ret = rendererStreamImpl->SetRate(rate);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test DequeueBuffer API
 * @tc.type  : FUNC
 * @tc.number: DequeueBuffer
 */
HWTEST(ProRendererStreamImplUnitTest, DequeueBuffer_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);

    rendererStreamImpl->status_ = I_STATUS_IDLE;
    size_t length = 10;
    BufferDesc bufferDesc = rendererStreamImpl->DequeueBuffer(length);
    EXPECT_EQ(bufferDesc.buffer, nullptr);
}

/**
 * @tc.name  : Test DequeueBuffer API
 * @tc.type  : FUNC
 * @tc.number: DequeueBuffer
 */
HWTEST(ProRendererStreamImplUnitTest, DequeueBuffer_002, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);

    std::vector<char> buffer(10, 'a');
    rendererStreamImpl->sinkBuffer_.push_back(buffer);
    rendererStreamImpl->status_ = I_STATUS_STARTED;
    size_t length = 10;
    BufferDesc bufferDesc = rendererStreamImpl->DequeueBuffer(length);
    EXPECT_NE(bufferDesc.buffer, nullptr);
}

/**
 * @tc.name  : Test EnqueueBuffer API
 * @tc.type  : FUNC
 * @tc.number: EnqueueBuffer
 */
HWTEST(ProRendererStreamImplUnitTest, EnqueueBuffer_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);

    const BufferDesc bufferDesc = {nullptr, 0, 0};
    int32_t ret = rendererStreamImpl->EnqueueBuffer(bufferDesc);
    EXPECT_EQ(ret, ERR_WRITE_BUFFER);
}


/**
* @tc.name  : Test EnqueueBuffer API
 * @tc.type  : FUNC
 * @tc.number: EnqueueBuffer
 */
HWTEST(ProRendererStreamImplUnitTest, EnqueueBuffer_002, TestSize.Level1)
{
    AudioProcessConfig processConfig = InitProcessConfig();
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);

    rendererStreamImpl->InitParams();
    const BufferDesc bufferDesc = {nullptr, 0, 0};
    int32_t ret = rendererStreamImpl->EnqueueBuffer(bufferDesc);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test EnqueueBuffer API
 * @tc.type  : FUNC
 * @tc.number: EnqueueBuffer
 */
HWTEST(ProRendererStreamImplUnitTest, EnqueueBuffer_003, TestSize.Level1)
{
    AudioProcessConfig processConfig = InitProcessConfig();
    processConfig.streamType = STREAM_VOICE_CALL;
    processConfig.streamInfo.channels = CHANNEL_4;
    processConfig.streamInfo.samplingRate = SAMPLE_RATE_16000;
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);

    rendererStreamImpl->InitParams();
    rendererStreamImpl->isNeedMcr_ = true;
    rendererStreamImpl->isNeedResample_ = false;
    const BufferDesc bufferDesc = {nullptr, 0, 0};
    int32_t ret = rendererStreamImpl->EnqueueBuffer(bufferDesc);
    EXPECT_NE(ret, SUCCESS);
}

/**
* @tc.name  : Test EnqueueBuffer API
 * @tc.type  : FUNC
 * @tc.number: EnqueueBuffer
 */
HWTEST(ProRendererStreamImplUnitTest, EnqueueBuffer_004, TestSize.Level1)
{
    AudioProcessConfig processConfig = InitProcessConfig();
    processConfig.streamType = STREAM_VOICE_CALL;
    processConfig.streamInfo.channels = CHANNEL_4;
    processConfig.streamInfo.samplingRate = SAMPLE_RATE_8000;
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);

    rendererStreamImpl->InitParams();
    rendererStreamImpl->isNeedMcr_ = true;
    rendererStreamImpl->isNeedResample_ = true;
    const BufferDesc bufferDesc = {nullptr, 0, 0};
    int32_t ret = rendererStreamImpl->EnqueueBuffer(bufferDesc);
    EXPECT_NE(ret, SUCCESS);
}

/**
* @tc.name  : Test EnqueueBuffer API
 * @tc.type  : FUNC
 * @tc.number: EnqueueBuffer
 */
HWTEST(ProRendererStreamImplUnitTest, EnqueueBuffer_005, TestSize.Level1)
{
    AudioProcessConfig processConfig = InitProcessConfig();
    processConfig.streamType = STREAM_VOICE_CALL;
    processConfig.streamInfo.channels = STEREO;
    processConfig.streamInfo.samplingRate = SAMPLE_RATE_8000;
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);

    rendererStreamImpl->InitParams();
    rendererStreamImpl->isNeedMcr_ = false;
    rendererStreamImpl->isNeedResample_ = true;

    const BufferDesc bufferDesc = {nullptr, 0, 0};
    int32_t ret = rendererStreamImpl->EnqueueBuffer(bufferDesc);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test EnqueueBuffer API
 * @tc.type  : FUNC
 * @tc.number: EnqueueBuffer
 */
HWTEST(ProRendererStreamImplUnitTest, EnqueueBuffer_006, TestSize.Level1)
{
    AudioProcessConfig processConfig = InitProcessConfig();
    processConfig.streamType = STREAM_VOICE_CALL;
    processConfig.streamInfo.format = SAMPLE_S16LE;
    processConfig.streamInfo.channels = STEREO;
    processConfig.streamInfo.samplingRate = SAMPLE_RATE_16000;
    bool isDirect = false;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);

    rendererStreamImpl->InitParams();
    rendererStreamImpl->desFormat_ = SAMPLE_S16LE;
    const BufferDesc bufferDesc = {nullptr, 0, 0};
    int32_t ret = rendererStreamImpl->EnqueueBuffer(bufferDesc);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test EnqueueBuffer API
 * @tc.type  : FUNC
 * @tc.number: EnqueueBuffer
 */
HWTEST(ProRendererStreamImplUnitTest, EnqueueBuffer_007, TestSize.Level3)
{
    AudioProcessConfig processConfig = InitProcessConfig();
    processConfig.streamType = STREAM_VOICE_CALL;
    processConfig.streamInfo.format = SAMPLE_S16LE;
    processConfig.streamInfo.channels = STEREO;
    processConfig.streamInfo.samplingRate = SAMPLE_RATE_16000;
    bool isDirect = false;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    rendererStreamImpl->InitParams();
    rendererStreamImpl->isNeedMcr_ = false;
    rendererStreamImpl->isNeedResample_ = false;
    rendererStreamImpl->desFormat_ = SAMPLE_U8;
    const BufferDesc bufferDesc = {nullptr, 1, 0};

    int32_t writeIndex = rendererStreamImpl->PopWriteBufferIndex();
    rendererStreamImpl->bufferInfo_.format = 0;
    rendererStreamImpl->bufferInfo_.frameSize =
                    rendererStreamImpl->sinkBuffer_[writeIndex].size() / sizeof(int32_t);
    int32_t ret = rendererStreamImpl->EnqueueBuffer(bufferDesc);
    EXPECT_EQ(rendererStreamImpl->totalBytesWritten_, 1);

    rendererStreamImpl->bufferInfo_.format = 1;
    ret = rendererStreamImpl->EnqueueBuffer(bufferDesc);
    EXPECT_EQ(rendererStreamImpl->totalBytesWritten_, 2);

    rendererStreamImpl->bufferInfo_.format = 2;
    ret = rendererStreamImpl->EnqueueBuffer(bufferDesc);
    EXPECT_EQ(rendererStreamImpl->totalBytesWritten_, 3);

    ret = rendererStreamImpl->Flush();
    EXPECT_EQ(ret, SUCCESS);

    rendererStreamImpl->bufferInfo_.format = 3;
    ret = rendererStreamImpl->EnqueueBuffer(bufferDesc);
    EXPECT_EQ(rendererStreamImpl->totalBytesWritten_, 4);

    rendererStreamImpl->bufferInfo_.format = 4;
    ret = rendererStreamImpl->EnqueueBuffer(bufferDesc);
    EXPECT_EQ(rendererStreamImpl->totalBytesWritten_, 5);

    rendererStreamImpl->bufferInfo_.format = 24;
    rendererStreamImpl->EnqueueBuffer(bufferDesc);
    EXPECT_EQ(rendererStreamImpl->totalBytesWritten_, 6);
}


/**
* @tc.name  : Test EnqueueBuffer API
 * @tc.type  : FUNC
 * @tc.number: EnqueueBuffer
 */
HWTEST(ProRendererStreamImplUnitTest, EnqueueBuffer_008, TestSize.Level3)
{
    AudioProcessConfig processConfig = InitProcessConfig();
    processConfig.streamType = STREAM_VOICE_CALL;
    processConfig.streamInfo.format = SAMPLE_S16LE;
    processConfig.streamInfo.channels = STEREO;
    processConfig.streamInfo.samplingRate = SAMPLE_RATE_16000;
    bool isDirect = false;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    rendererStreamImpl->InitParams();
    rendererStreamImpl->isNeedMcr_ = false;
    rendererStreamImpl->isNeedResample_ = false;
    rendererStreamImpl->desFormat_ = SAMPLE_S16LE;
    const BufferDesc bufferDesc = {nullptr, 1, 0};

    int32_t writeIndex = rendererStreamImpl->PopWriteBufferIndex();
    rendererStreamImpl->bufferInfo_.format = 0;
    rendererStreamImpl->bufferInfo_.frameSize =
                    rendererStreamImpl->sinkBuffer_[writeIndex].size() / sizeof(int32_t);
    int32_t ret = rendererStreamImpl->EnqueueBuffer(bufferDesc);
    EXPECT_EQ(rendererStreamImpl->totalBytesWritten_, 1);

    rendererStreamImpl->bufferInfo_.format = 1;
    ret = rendererStreamImpl->EnqueueBuffer(bufferDesc);
    EXPECT_EQ(rendererStreamImpl->totalBytesWritten_, 2);

    rendererStreamImpl->bufferInfo_.format = 2;
    ret = rendererStreamImpl->EnqueueBuffer(bufferDesc);
    EXPECT_EQ(rendererStreamImpl->totalBytesWritten_, 3);

    ret = rendererStreamImpl->Flush();
    EXPECT_EQ(ret, SUCCESS);

    rendererStreamImpl->bufferInfo_.format = 3;
    ret = rendererStreamImpl->EnqueueBuffer(bufferDesc);
    EXPECT_EQ(rendererStreamImpl->totalBytesWritten_, 4);

    rendererStreamImpl->bufferInfo_.format = 4;
    ret = rendererStreamImpl->EnqueueBuffer(bufferDesc);
    EXPECT_EQ(rendererStreamImpl->totalBytesWritten_, 5);

    rendererStreamImpl->bufferInfo_.format = 24;
    rendererStreamImpl->EnqueueBuffer(bufferDesc);
    EXPECT_EQ(rendererStreamImpl->totalBytesWritten_, 6);
}

/**
* @tc.name  : Test EnqueueBuffer API
 * @tc.type  : FUNC
 * @tc.number: EnqueueBuffer
 */
HWTEST(ProRendererStreamImplUnitTest, EnqueueBuffer_009, TestSize.Level3)
{
    AudioProcessConfig processConfig = InitProcessConfig();
    processConfig.streamType = STREAM_MOVIE;
    processConfig.streamInfo.channels = STEREO;
    processConfig.streamInfo.samplingRate = SAMPLE_RATE_48000;
    processConfig.streamInfo.encoding = ENCODING_AUDIOVIVID;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, false);
    rendererStreamImpl->InitParams();
    rendererStreamImpl->isNeedMcr_ = false;
    rendererStreamImpl->isNeedResample_ = false;

    uint32_t pcmLen = 1024 * 2 * 2;
    uint32_t metaLen = 19824;
    uint8_t *pcmBuffer = new uint8_t[pcmLen] {0};
    uint8_t *metaBuffer = new uint8_t[metaLen] {0};
    BufferDesc bufferDesc;
    bufferDesc.buffer = pcmBuffer;
    bufferDesc.bufLength = pcmLen;
    bufferDesc.metaBuffer = metaBuffer;
    bufferDesc.metaLength = metaLen;

    int32_t ret = rendererStreamImpl->EnqueueBuffer(bufferDesc);
    EXPECT_EQ(ret, SUCCESS);

    delete[] pcmBuffer;
    delete[] metaBuffer;
}

/**
* @tc.name  : Test EnqueueBuffer API
 * @tc.type  : FUNC
 * @tc.number: EnqueueBuffer
 */
HWTEST(ProRendererStreamImplUnitTest, EnqueueBuffer_010, TestSize.Level3)
{
    AudioProcessConfig processConfig = InitProcessConfig();
    processConfig.streamType = STREAM_VOICE_CALL;
    processConfig.streamInfo.format = SAMPLE_S16LE;
    processConfig.streamInfo.channels = STEREO;
    processConfig.streamInfo.samplingRate = SAMPLE_RATE_16000;
    bool isDirect = false;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    rendererStreamImpl->InitParams();
    rendererStreamImpl->isNeedMcr_ = false;
    rendererStreamImpl->isNeedResample_ = false;
    rendererStreamImpl->desFormat_ = SAMPLE_S32LE;
    const BufferDesc bufferDesc = {nullptr, 1, 0};

    int32_t writeIndex = rendererStreamImpl->PopWriteBufferIndex();
    rendererStreamImpl->bufferInfo_.format = 0;
    rendererStreamImpl->bufferInfo_.frameSize =
                    rendererStreamImpl->sinkBuffer_[writeIndex].size() / sizeof(int32_t);
    int32_t ret = rendererStreamImpl->EnqueueBuffer(bufferDesc);
    EXPECT_EQ(rendererStreamImpl->totalBytesWritten_, 1);

    rendererStreamImpl->bufferInfo_.format = 1;
    ret = rendererStreamImpl->EnqueueBuffer(bufferDesc);
    EXPECT_EQ(rendererStreamImpl->totalBytesWritten_, 2);

    rendererStreamImpl->bufferInfo_.format = 2;
    ret = rendererStreamImpl->EnqueueBuffer(bufferDesc);
    EXPECT_EQ(rendererStreamImpl->totalBytesWritten_, 3);

    ret = rendererStreamImpl->Flush();
    EXPECT_EQ(ret, SUCCESS);

    rendererStreamImpl->bufferInfo_.format = 3;
    ret = rendererStreamImpl->EnqueueBuffer(bufferDesc);
    EXPECT_EQ(rendererStreamImpl->totalBytesWritten_, 4);

    rendererStreamImpl->bufferInfo_.format = 4;
    ret = rendererStreamImpl->EnqueueBuffer(bufferDesc);
    EXPECT_EQ(rendererStreamImpl->totalBytesWritten_, 5);

    rendererStreamImpl->bufferInfo_.format = 24;
    rendererStreamImpl->EnqueueBuffer(bufferDesc);
    EXPECT_EQ(rendererStreamImpl->totalBytesWritten_, 6);
}

/**
* @tc.name  : EnqueueBuffer_Direct3DA_011
* @tc.type  : FUNC
* @tc.number: Test EnqueueBuffer 3DA direct passthrough: whole frame copied byte-identical,
*             oversize buffer dropped without overflowing the slot
*/
HWTEST(ProRendererStreamImplUnitTest, EnqueueBuffer_Direct3DA_011, TestSize.Level3)
{
    AudioProcessConfig processConfig;
    processConfig.streamInfo.channels = CHANNEL_8;
    processConfig.streamInfo.samplingRate = SAMPLE_RATE_48000;
    processConfig.streamInfo.format = SAMPLE_S16LE;
    processConfig.streamInfo.encoding = ENCODING_AUDIOVIVID;
    processConfig.rendererInfo.rendererFlags = AUDIO_FLAG_3DA_DIRECT;
    auto rendererStreamImpl = std::make_shared<ProRendererStreamImpl>(processConfig, false);
    rendererStreamImpl->status_ = I_STATUS_INVALID;
    ASSERT_EQ(rendererStreamImpl->InitParams(), SUCCESS);

    uint32_t frameSize = 2 * 8; // S16LE 2 bytes per sample * 8 channels
    uint32_t metaAligned = (AVS3METADATA_SIZE + frameSize - 1) / frameSize * frameSize;
    size_t slotSize = static_cast<size_t>((AUDIO_VIVID_SAMPLES + THREE_DA_HDI_HEADER_FRAMES) * frameSize
        + metaAligned);
    ASSERT_EQ(rendererStreamImpl->sinkBuffer_[0].size(), slotSize);

    // one whole frame passes through byte-identical
    std::vector<uint8_t> wholeFrame(slotSize, 0x5A);
    BufferDesc bufferDesc;
    bufferDesc.buffer = wholeFrame.data();
    bufferDesc.bufLength = slotSize;
    EXPECT_EQ(rendererStreamImpl->EnqueueBuffer(bufferDesc), SUCCESS);
    int32_t writeIndex = rendererStreamImpl->readQueue_.front();
    bool identical = true;
    for (size_t i = 0; i < slotSize; i++) {
        if (static_cast<uint8_t>(rendererStreamImpl->sinkBuffer_[writeIndex][i]) != wholeFrame[i]) {
            identical = false;
            break;
        }
    }
    EXPECT_TRUE(identical);

    // oversize buffer is dropped, never overflows the slot
    std::vector<uint8_t> oversize(slotSize + 1, 0xA5);
    bufferDesc.buffer = oversize.data();
    bufferDesc.bufLength = oversize.size();
    EXPECT_EQ(rendererStreamImpl->EnqueueBuffer(bufferDesc), SUCCESS);
    writeIndex = rendererStreamImpl->readQueue_.back();
    bool slotUntouched = true;
    for (size_t i = 0; i < slotSize; i++) {
        if (rendererStreamImpl->sinkBuffer_[writeIndex][i] != 0) {
            slotUntouched = false;
            break;
        }
    }
    EXPECT_TRUE(slotUntouched);
}

/**
* @tc.name  : Test WriteToSinkBuffer passes the 3DA whole frame through untouched
* @tc.type  : FUNC
* @tc.number: WriteToSinkBuffer_3DA_Direct_Passthrough_001
* @tc.desc  : The client assembles the whole 3DA HDI frame; the server side copies it into
*             the sink slot byte-identically and never assembles PCM/metadata itself.
*/
HWTEST(ProRendererStreamImplUnitTest, WriteToSinkBuffer_3DA_Direct_Passthrough_001, TestSize.Level3)
{
    constexpr uint8_t FRAME_SENTINEL = 0x5A;

    AudioProcessConfig processConfig;
    processConfig.streamInfo.channels = CHANNEL_8;
    processConfig.streamInfo.samplingRate = SAMPLE_RATE_48000;
    processConfig.streamInfo.format = SAMPLE_S16LE;
    processConfig.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_5POINT1POINT2;
    processConfig.streamInfo.encoding = ENCODING_AUDIOVIVID;
    processConfig.rendererInfo.rendererFlags = AUDIO_FLAG_3DA_DIRECT;

    auto rendererStreamImpl = std::make_shared<ProRendererStreamImpl>(processConfig, false);
    rendererStreamImpl->status_ = I_STATUS_INVALID;
    ASSERT_EQ(rendererStreamImpl->InitParams(), SUCCESS);
    EXPECT_FALSE(rendererStreamImpl->isNeedResample_);
    EXPECT_FALSE(rendererStreamImpl->isNeedMcr_);

    const uint32_t frameSize = static_cast<uint32_t>(processConfig.streamInfo.channels) * sizeof(int16_t);
    const uint32_t metaAligned = (AVS3METADATA_SIZE + frameSize - 1) / frameSize * frameSize;
    const uint32_t wholeFrameLen = (AUDIO_VIVID_SAMPLES + THREE_DA_HDI_HEADER_FRAMES) * frameSize
        + metaAligned; // one whole HDI frame, 36256 bytes for 8ch S16LE

    // The client (Write3DADirectBuffer) assembles magic|size1|metadata|align|size2|pcm; the
    // server side only passes it through, so the slot must be a byte-identical copy.
    std::vector<uint8_t> wholeFrame(wholeFrameLen, FRAME_SENTINEL);
    BufferDesc bufferDesc;
    bufferDesc.buffer = wholeFrame.data();
    bufferDesc.bufLength = wholeFrameLen;

    int32_t writeIndex = rendererStreamImpl->PopWriteBufferIndex();
    ASSERT_GE(writeIndex, 0);
    rendererStreamImpl->WriteToSinkBuffer(bufferDesc, static_cast<uint32_t>(writeIndex));

    const auto &sinkSlot = rendererStreamImpl->sinkBuffer_[static_cast<size_t>(writeIndex)];
    ASSERT_EQ(sinkSlot.size(), wholeFrameLen);
    bool identical = true;
    for (uint32_t i = 0; i < wholeFrameLen; i++) {
        if (static_cast<uint8_t>(sinkSlot[i]) != FRAME_SENTINEL) {
            identical = false;
            break;
        }
    }
    EXPECT_TRUE(identical);
}

/**
 * @tc.name  : Test GetMinimumBufferSize API
 * @tc.type  : FUNC
 * @tc.number: GetMinimumBufferSize
 */
HWTEST(ProRendererStreamImplUnitTest, GetMinimumBufferSize_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    size_t minBufferSize;
    int32_t ret = rendererStreamImpl->GetMinimumBufferSize(minBufferSize);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetOffloadMode API
 * @tc.type  : FUNC
 * @tc.number: SetOffloadMode
 */
HWTEST(ProRendererStreamImplUnitTest, SetOffloadMode_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    int32_t state = 0;
    bool isAppBack = true;
    int32_t ret = rendererStreamImpl->SetOffloadMode(state, isAppBack);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 *@tc.name  : Test UnsetOffloadMode API
 *@tc.type  : FUNC
 *@tc.number: UnsetOffloadMode
 */
HWTEST(ProRendererStreamImplUnitTest, UnsetOffloadMode_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);

    int32_t ret = rendererStreamImpl->UnsetOffloadMode();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 *@tc.name  : Test OffloadSetVolume API
 *@tc.type  : FUNC
 *@tc.number: OffloadSetVolume
 */
HWTEST(ProRendererStreamImplUnitTest, OffloadSetVolume_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);

    int32_t ret = rendererStreamImpl->OffloadSetVolume();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 *@tc.name  : Test GetAudioTime API
 *@tc.type  : FUNC
 *@tc.number: GetAudioTime
 */
HWTEST(ProRendererStreamImplUnitTest, GetAudioTime_001, TestSize.Level1)
{
    AudioProcessConfig processConfig = InitProcessConfig();
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    rendererStreamImpl->InitParams();

    uint64_t framePos;
    int64_t sec, nanoSec;
    bool ret = rendererStreamImpl->GetAudioTime(framePos, sec, nanoSec);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test ReturnIndex API
 * @tc.type  : FUNC
 * @tc.number: ReturnIndex
 */
HWTEST(ProRendererStreamImplUnitTest, ReturnIndex_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);

    int32_t index = 10;
    int32_t ret = rendererStreamImpl->ReturnIndex(index);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test PopSinkBuffer API
 * @tc.type  : FUNC
 * @tc.number: PopSinkBuffer
 */
HWTEST(ProRendererStreamImplUnitTest, PopSinkBuffer_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);

    std::vector<char> audioBuffer;
    int32_t index;
    rendererStreamImpl->isFirstFrame_ = true;
    rendererStreamImpl->PopSinkBuffer(&audioBuffer, index);
    EXPECT_EQ(rendererStreamImpl->isFirstFrame_, true);
}

/**
 * @tc.name  : Test PopSinkBuffer API
 * @tc.type  : FUNC
 * @tc.number: PopSinkBuffer
 */
HWTEST(ProRendererStreamImplUnitTest, PopSinkBuffer_002, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);

    std::vector<char> audioBuffer;
    int32_t index;
    rendererStreamImpl->isFirstFrame_ = false;
    rendererStreamImpl->isDrain_ = true;
    rendererStreamImpl->PopSinkBuffer(&audioBuffer, index);
    EXPECT_EQ(rendererStreamImpl->isFirstFrame_, false);
}

/**
 * @tc.name  : Test PopSinkBuffer API
 * @tc.type  : FUNC
 * @tc.number: PopSinkBuffer
 */
HWTEST(ProRendererStreamImplUnitTest, PopSinkBuffer_003, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);

    std::vector<char> audioBuffer;
    std::vector<char> tmp = {'1', '2', '3'};
    int32_t index = 0;
    rendererStreamImpl->isFirstFrame_ = true;
    rendererStreamImpl->isBlock_ = true;
    rendererStreamImpl->isDrain_ = true;
    rendererStreamImpl->PopSinkBuffer(&audioBuffer, index);
    EXPECT_EQ(rendererStreamImpl->isFirstFrame_, true);

    rendererStreamImpl->readQueue_.push(1);
    rendererStreamImpl->sinkBuffer_.push_back(tmp);
    rendererStreamImpl->sinkBuffer_.push_back(tmp);
    rendererStreamImpl->PopSinkBuffer(&audioBuffer, index);
    EXPECT_EQ(rendererStreamImpl->isDrain_, true);
}

/**
 * @tc.name  : Test ConvertSrcToFloat API
 * @tc.type  : FUNC
 * @tc.number: ConvertSrcToFloat
 */
HWTEST(ProRendererStreamImplUnitTest, ConvertSrcToFloat_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    uint8_t buf[8] = {0};
    BufferDesc bufferDesc = {0};

    bufferDesc.buffer = buf;
    bufferDesc.bufLength = 8;
    bufferDesc.dataLength = 4;
    rendererStreamImpl->bufferInfo_.samplePerFrame = 2;
    rendererStreamImpl->bufferInfo_.format = AudioSampleFormat::SAMPLE_F32LE;
    rendererStreamImpl->ConvertSrcToFloat(bufferDesc);
    EXPECT_EQ(rendererStreamImpl != nullptr, true);
}

/**
 * @tc.name  : Test ConvertSrcToFloat API
 * @tc.type  : FUNC
 * @tc.number: ConvertSrcToFloat
 */
HWTEST(ProRendererStreamImplUnitTest, ConvertSrcToFloat_002, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    uint8_t buf[8] = {0};
    BufferDesc bufferDesc = {0};

    bufferDesc.buffer = buf;
    bufferDesc.bufLength = 8;
    bufferDesc.dataLength = 4;
    rendererStreamImpl->bufferInfo_.samplePerFrame = 2;
    rendererStreamImpl->bufferInfo_.format = AudioSampleFormat::SAMPLE_S16LE;
    rendererStreamImpl->ConvertSrcToFloat(bufferDesc);
    EXPECT_EQ(rendererStreamImpl != nullptr, true);
}
/**
 *@tc.name  : Test SetClientVolume API
 *@tc.type  : FUNC
 *@tc.number: SetClientVolume
 */
HWTEST(ProRendererStreamImplUnitTest, SetClientVolume_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);

    float clientVolume = 0;
    int32_t ret = rendererStreamImpl->SetClientVolume(clientVolume);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 *@tc.name  : Test UpdateMaxLength API
 *@tc.type  : FUNC
 *@tc.number: UpdateMaxLength
 */
HWTEST(ProRendererStreamImplUnitTest, UpdateMaxLength_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);

    uint32_t maxLength = 0;
    int32_t ret = rendererStreamImpl->UpdateMaxLength(maxLength);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 *@tc.name  : Test GetCurrentTimeStamp API
 *@tc.type  : FUNC
 *@tc.number: GetCurrentTimeStamp
 */
HWTEST(ProRendererStreamImplUnitTest, GetCurrentTimeStamp_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);

    uint64_t timestamp;
    int32_t ret = rendererStreamImpl->GetCurrentTimeStamp(timestamp);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 *@tc.name  : Test GetStreamFramesWritten API
 *@tc.type  : FUNC
 *@tc.number: GetStreamFramesWritten
 */
HWTEST(ProRendererStreamImplUnitTest, GetStreamFramesWritte_001, TestSize.Level1)
{
    AudioProcessConfig processConfig = InitProcessConfig();
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    rendererStreamImpl->InitParams();

    uint64_t framesWritten;
    int32_t ret = rendererStreamImpl->GetStreamFramesWritten(framesWritten);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 *@tc.name  : Test GetCurrentPosition API
 *@tc.type  : FUNC
 *@tc.number: GetCurrentPosition
 */
HWTEST(ProRendererStreamImplUnitTest, GetCurrentPosition_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);

    uint64_t framePosition, timestamp, latency;
    int32_t ret = rendererStreamImpl->GetCurrentPosition(framePosition, timestamp, latency,
        Timestamp::MONOTONIC);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 *@tc.name  : Test GetLatency API
 *@tc.type  : FUNC
 *@tc.number: GetLatency
*/
HWTEST(ProRendererStreamImplUnitTest, GetLatency_001, TestSize.Level1)
{
    AudioProcessConfig processConfig = InitProcessConfig();
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    rendererStreamImpl->InitParams();

    uint64_t latency;
    int32_t ret = rendererStreamImpl->GetLatency(latency);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 *@tc.name  : Test SetAudioEffectMode API
 *@tc.type  : FUNC
 *@tc.number: SetAudioEffectMode
*/
HWTEST(ProRendererStreamImplUnitTest, SetAudioEffectMode_001, TestSize.Level1)
{
    AudioProcessConfig processConfig = InitProcessConfig();
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    rendererStreamImpl->InitParams();

    int32_t effectMode = 1;
    int32_t result = rendererStreamImpl->SetAudioEffectMode(effectMode);
    EXPECT_EQ(result, SUCCESS);
}

/**
 *@tc.name  : Test GetAudioEffectMode API
 *@tc.type  : FUNC
 *@tc.number: GetAudioEffectMode
*/
HWTEST(ProRendererStreamImplUnitTest, GetAudioEffectMode_001, TestSize.Level1)
{
    AudioProcessConfig processConfig = InitProcessConfig();
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    rendererStreamImpl->InitParams();

    int32_t effectMode;
    int32_t result = rendererStreamImpl->GetAudioEffectMode(effectMode);
    EXPECT_EQ(result, SUCCESS);
}

/**
 *@tc.name  : Test SetPrivacyType API
 *@tc.type  : FUNC
 *@tc.number: SetPrivacyType
*/
HWTEST(ProRendererStreamImplUnitTest, SetPrivacyType_001, TestSize.Level1)
{
    AudioProcessConfig processConfig = InitProcessConfig();
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    rendererStreamImpl->InitParams();

    int32_t privacyType = 1;
    int32_t result = rendererStreamImpl->SetPrivacyType(privacyType);
    EXPECT_EQ(result, SUCCESS);
    int32_t rePrivacyType;
    result = rendererStreamImpl->GetPrivacyType(rePrivacyType);

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(privacyType, rePrivacyType);
}
/**
 *@tc.name  : Test GetOffloadApproximatelyCacheTime API
 *@tc.type  : FUNC
 *@tc.number: GetOffloadApproximatelyCacheTime
*/
HWTEST(ProRendererStreamImplUnitTest, GetOffloadApproximatelyCacheTime_001, TestSize.Level1)
{
    AudioProcessConfig processConfig = InitProcessConfig();
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    rendererStreamImpl->InitParams();

    uint64_t timestamp = 0;
    uint64_t paWriteIndex = 0;
    uint64_t cacheTimeDsp = 0;
    uint64_t cacheTimePa = 0;
    int32_t result = rendererStreamImpl->GetOffloadApproximatelyCacheTime(timestamp,
        paWriteIndex, cacheTimeDsp, cacheTimePa);
    ASSERT_EQ(result, SUCCESS);
}

/**
 *@tc.name  : Test UpdateSpatializationState API
 *@tc.type  : FUNC
 *@tc.number: UpdateSpatializationState
*/
HWTEST(ProRendererStreamImplUnitTest, UpdateSpatializationState_001, TestSize.Level1)
{
    AudioProcessConfig processConfig = InitProcessConfig();
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    rendererStreamImpl->InitParams();
    int32_t result = rendererStreamImpl->UpdateSpatializationState(true, true);
    ASSERT_EQ(result, SUCCESS);
}

/**
 *@tc.name  : Test BlockStream API
 *@tc.type  : FUNC
 *@tc.number: BlockStream
*/
HWTEST(ProRendererStreamImplUnitTest, BlockStream_001, TestSize.Level1)
{
    AudioProcessConfig processConfig = InitProcessConfig();
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    rendererStreamImpl->BlockStream();
    ASSERT_EQ(rendererStreamImpl->isBlock_, true);
}

/**
 *@tc.name  : Test PeekAPI
 *@tc.type  : FUNC
 *@tc.number: Peek
*/
HWTEST(ProRendererStreamImplUnitTest, Peek_001, TestSize.Level0)
{
    AudioProcessConfig processConfig = InitProcessConfig();
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    std::vector<char> audioBuffer;
    int32_t index = 0;

    rendererStreamImpl->isBlock_ = false;
    EXPECT_NE(rendererStreamImpl->Peek(&audioBuffer, index), SUCCESS);
}

/**
 *@tc.name  : Test PeekAPI
 *@tc.type  : FUNC
 *@tc.number: Peek
*/
HWTEST(ProRendererStreamImplUnitTest, Peek_002, TestSize.Level0)
{
    AudioProcessConfig processConfig = InitProcessConfig();
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = nullptr;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListenerHolder);
    rendererStreamImpl->RegisterWriteCallback(rendererInServer);
    std::vector<char> audioBuffer;
    int32_t index = 0;

    rendererStreamImpl->isBlock_ = true;
    EXPECT_EQ(rendererStreamImpl->Peek(&audioBuffer, index), ERR_WRITE_BUFFER);
}

/**
 *@tc.name  : Test PeekAPI
 *@tc.type  : FUNC
 *@tc.number: Peek
*/
HWTEST(ProRendererStreamImplUnitTest, Peek_003, TestSize.Level0)
{
    AudioProcessConfig processConfig = InitProcessConfig();
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    std::vector<char> audioBuffer;
    int32_t index = 0;
    int32_t ret = 0;

    rendererStreamImpl->isBlock_ = false;
    ret = rendererStreamImpl->Peek(&audioBuffer, index);
    EXPECT_EQ(ret, ERR_WRITE_BUFFER);
}

/**
 *@tc.name  : Test GetByteSizePerFrame API
 *@tc.type  : FUNC
 *@tc.number: GetByteSizePerFrame
*/
HWTEST(ProRendererStreamImplUnitTest, GetByteSizePerFrame_001, TestSize.Level0)
{
    AudioProcessConfig processConfig = InitProcessConfig();
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);

    rendererStreamImpl->byteSizePerFrame_= 10;
    size_t byteSizePerFrame = 0;
    rendererStreamImpl->GetByteSizePerFrame(byteSizePerFrame);
    EXPECT_EQ(byteSizePerFrame, 10);
}

/**
 *@tc.name  : Test GetSpanSizePerFrame API
 *@tc.type  : FUNC
 *@tc.number: GetSpanSizePerFrame
*/
HWTEST(ProRendererStreamImplUnitTest, GetSpanSizePerFrame_001, TestSize.Level0)
{
    AudioProcessConfig processConfig = InitProcessConfig();
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);

    rendererStreamImpl->spanSizeInFrame_= 10;
    size_t spanSizeInFrame = 0;
    rendererStreamImpl->GetSpanSizePerFrame(spanSizeInFrame);
    EXPECT_EQ(spanSizeInFrame, 10);
}

/**
 *@tc.name  : Test RegisterStatusCallback API
 *@tc.type  : FUNC
 *@tc.number: RegisterStatusCallback
*/
HWTEST(ProRendererStreamImplUnitTest, RegisterStatusCallback_001, TestSize.Level0)
{
    AudioProcessConfig processConfig = InitProcessConfig();
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = nullptr;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListenerHolder);
    rendererStreamImpl->RegisterStatusCallback(rendererInServer);
    EXPECT_NE(rendererStreamImpl->statusCallback_.lock(), nullptr);
}

/**
 *@tc.name  : Test RegisterWriteCallback API
 *@tc.type  : FUNC
 *@tc.number: RegisterWriteCallback
*/
HWTEST(ProRendererStreamImplUnitTest, RegisterWriteCallback_001, TestSize.Level0)
{
    AudioProcessConfig processConfig = InitProcessConfig();
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = nullptr;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListenerHolder);
    rendererStreamImpl->RegisterWriteCallback(rendererInServer);
    EXPECT_NE(rendererStreamImpl->writeCallback_.lock(), nullptr);
}

/**
 *@tc.name  : Test GetWritableSize API
 *@tc.type  : FUNC
 *@tc.number: GetWritableSize
*/

HWTEST(ProRendererStreamImplUnitTest, GetWritableSize_001, TestSize.Level0)
{
    AudioProcessConfig processConfig = InitProcessConfig();
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);

    size_t writableSize = rendererStreamImpl->GetWritableSize();

    EXPECT_EQ(writableSize, 0);
}

/**
 *@tc.name  : Test ConvertFloatToDes API
 *@tc.type  : FUNC
 *@tc.number: ConvertFloatToDes
*/
HWTEST(ProRendererStreamImplUnitTest, ConvertFloatToDes_001, TestSize.Level0)
{
    AudioProcessConfig processConfig = InitProcessConfig();
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    rendererStreamImpl->InitParams();
    int32_t writeIndex = 0;
    rendererStreamImpl->desFormat_ = SAMPLE_F32LE;

    rendererStreamImpl->ConvertFloatToDes(writeIndex);
}

/**
 *@tc.name  : Test ConvertFloatToDes API
 *@tc.type  : FUNC
 *@tc.number: ConvertFloatToDes
*/
HWTEST(ProRendererStreamImplUnitTest, ConvertFloatToDes_002, TestSize.Level0)
{
    AudioProcessConfig processConfig = InitProcessConfig();
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    rendererStreamImpl->InitParams();
    int32_t writeIndex = 0;
    std::vector<char> buffer(10, 'a');

    rendererStreamImpl->desFormat_ = SAMPLE_S32LE;
    rendererStreamImpl->sinkBuffer_.push_back(buffer);
    rendererStreamImpl->ConvertFloatToDes(writeIndex);
    EXPECT_EQ(rendererStreamImpl->resampleDesBuffer.size(), 0);
}

/**
 *@tc.name  : Test SetOffloadDisable API
 *@tc.type  : FUNC
 *@tc.number: SetOffloadDisable_001
*/
HWTEST(ProRendererStreamImplUnitTest, SetOffloadDisable_001, TestSize.Level0)
{
    AudioProcessConfig processConfig = InitProcessConfig();
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder = nullptr;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListenerHolder);

    rendererStreamImpl->RegisterStatusCallback(rendererInServer);
    rendererStreamImpl->SetOffloadDisable();
    EXPECT_NE(rendererStreamImpl->statusCallback_.lock(), nullptr);
}

/**
 *@tc.name  : Test GetStreamVolume API
 *@tc.type  : FUNC
 *@tc.number: GetStreamVolume_001
*/
HWTEST(ProRendererStreamImplUnitTest, GetStreamVolume_001, TestSize.Level0)
{
    AudioProcessConfig processConfig = InitProcessConfig();
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);

    rendererStreamImpl->processConfig_.streamType = STREAM_VOICE_COMMUNICATION;
    rendererStreamImpl->GetStreamVolume();
    EXPECT_EQ(rendererStreamImpl->bufferInfo_.volumeBg, 1);

    rendererStreamImpl->processConfig_.streamType = STREAM_SOURCE_VOICE_CALL;
    rendererStreamImpl->GetStreamVolume();
    EXPECT_EQ(rendererStreamImpl->bufferInfo_.volumeBg, 0.0);
    EXPECT_EQ(rendererStreamImpl->bufferInfo_.volumeEd, 1.0);
}

/**
 *@tc.name  : Test InitBasicInfo API
 *@tc.type  : FUNC
 *@tc.number: InitBasicInfo_001
*/
HWTEST(ProRendererStreamImplUnitTest, InitBasicInfo_001, TestSize.Level0)
{
    AudioStreamInfo streamInfo;
    AudioProcessConfig processConfig = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, false);

    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_S16LE;
    streamInfo.channels = STEREO;
    rendererStreamImpl->InitBasicInfo(streamInfo);
    EXPECT_EQ(rendererStreamImpl->currentRate_, SAMPLE_RATE_48000);
}

/**
 *@tc.name  : Test InitBasicInfo API
 *@tc.type  : FUNC
 *@tc.number: InitBasicInfo_3DA_DIRECT_002
*/
HWTEST(ProRendererStreamImplUnitTest, InitBasicInfo_002, TestSize.Level0)
{
    AudioProcessConfig processConfig;
    processConfig.streamInfo.channels = CHANNEL_8;
    processConfig.streamInfo.samplingRate = SAMPLE_RATE_48000;
    processConfig.streamInfo.format = SAMPLE_S16LE;
    processConfig.streamInfo.encoding = ENCODING_AUDIOVIVID;
    processConfig.rendererInfo.rendererFlags = AUDIO_FLAG_3DA_DIRECT;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, false);

    rendererStreamImpl->InitBasicInfo(processConfig.streamInfo);
    // Whole-frame geometry: frameSize = 2 bytes * 8 ch = 16; metadata 19824 is already 16-aligned,
    // span = PCM frames + header frames + metadata frames, minBufferSize = one whole frame.
    uint32_t frameSize = 2 * 8; // S16LE 2 bytes per sample * 8 channels
    uint32_t metaAligned = (AVS3METADATA_SIZE + frameSize - 1) / frameSize * frameSize;
    uint32_t expectedSpanSize = AUDIO_VIVID_SAMPLES + THREE_DA_HDI_HEADER_FRAMES + metaAligned / frameSize;
    uint32_t expectedMinBufSize = (AUDIO_VIVID_SAMPLES + THREE_DA_HDI_HEADER_FRAMES) * frameSize + metaAligned;
    EXPECT_EQ(rendererStreamImpl->spanSizeInFrame_, expectedSpanSize);
    EXPECT_EQ(rendererStreamImpl->minBufferSize_, expectedMinBufSize);
}

/**
 *@tc.name  : Test InitBasicInfo API
 *@tc.type  : FUNC
 *@tc.number: InitBasicInfo_3DA_DIRECT_003
*/
HWTEST(ProRendererStreamImplUnitTest, InitBasicInfo_003, TestSize.Level0)
{
    AudioProcessConfig processConfig;
    processConfig.streamInfo.channels = CHANNEL_8;
    processConfig.streamInfo.samplingRate = SAMPLE_RATE_48000;
    processConfig.streamInfo.format = SAMPLE_S16LE;
    processConfig.streamInfo.encoding = ENCODING_AUDIOVIVID;
    processConfig.rendererInfo.rendererFlags = AUDIO_FLAG_NORMAL;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, false);

    rendererStreamImpl->InitBasicInfo(processConfig.streamInfo);
    EXPECT_EQ(rendererStreamImpl->spanSizeInFrame_, 960); // 48kHz smaplingRate 20ms span
    EXPECT_EQ(rendererStreamImpl->minBufferSize_, 15360); // 960 * 2 * 8
}

/**
 *@tc.name  : Test PopWriteBufferIndex API
 *@tc.type  : FUNC
 *@tc.number: PopWriteBufferIndex_001
*/
HWTEST(ProRendererStreamImplUnitTest, PopWriteBufferIndex_001, TestSize.Level0)
{
    int32_t ret;
    AudioProcessConfig processConfig = InitProcessConfig();
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);

    rendererStreamImpl->writeQueue_.push(1);
    ret = rendererStreamImpl->PopWriteBufferIndex();
    EXPECT_EQ(ret, 1);
}

/**
 *@tc.name  : Test GetAudioProcessConfig API
 *@tc.type  : FUNC
 *@tc.number: GetAudioProcessConfig_001
*/
HWTEST(ProRendererStreamImplUnitTest, GetAudioProcessConfig_001, TestSize.Level0)
{
    AudioProcessConfig ret;
    AudioProcessConfig processConfig = InitProcessConfig();
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);

    ret = rendererStreamImpl->GetAudioProcessConfig();
    EXPECT_EQ(ret.deviceType, processConfig.deviceType);
}

/**
 *@tc.name  : Test SetStreamIndex API
 *@tc.type  : FUNC
 *@tc.number: SetStreamIndex_001
*/
HWTEST(ProRendererStreamImplUnitTest, SetStreamIndex_001, TestSize.Level0)
{
    uint32_t index = 1;
    AudioProcessConfig processConfig = InitProcessConfig();
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);

    rendererStreamImpl->SetStreamIndex(index);
    EXPECT_EQ(rendererStreamImpl->streamIndex_, index);
}

/**
 * @tc.name  : Test SetSpeed API is unsupported on ProRendererStreamImpl
 * @tc.type  : FUNC
 * @tc.number: SetSpeed_NotSupported_001
 */
HWTEST(ProRendererStreamImplUnitTest, SetSpeed_NotSupported_001, TestSize.Level1)
{
    AudioProcessConfig processConfig = InitProcessConfig();
    auto rendererStreamImpl = std::make_shared<ProRendererStreamImpl>(processConfig, true);
    // SetSpeed is intentionally unsupported on the pro renderer stream.
    constexpr float NORMAL_PLAYBACK_SPEED = 1.0f;
    int32_t ret = rendererStreamImpl->SetSpeed(NORMAL_PLAYBACK_SPEED);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

/**
 * @tc.name  : Test SetLoudnessGain API is unsupported on ProRendererStreamImpl
 * @tc.type  : FUNC
 * @tc.number: SetLoudnessGain_NotSupported_001
 */
HWTEST(ProRendererStreamImplUnitTest, SetLoudnessGain_NotSupported_001, TestSize.Level1)
{
    AudioProcessConfig processConfig = InitProcessConfig();
    auto rendererStreamImpl = std::make_shared<ProRendererStreamImpl>(processConfig, true);
    // SetLoudnessGain is reserved for the hpae renderer stream and is rejected here.
    constexpr float TYPICAL_LOUDNESS_GAIN = 0.5f;
    int32_t ret = rendererStreamImpl->SetLoudnessGain(TYPICAL_LOUDNESS_GAIN);
    EXPECT_EQ(ret, ERR_PRO_STREAM_NOT_SUPPORTED);
}

/**
 * @tc.name  : Test SetOffloadDataCallbackState API is unsupported on ProRendererStreamImpl
 * @tc.type  : FUNC
 * @tc.number: SetOffloadDataCallbackState_NotSupported_001
 */
HWTEST(ProRendererStreamImplUnitTest, SetOffloadDataCallbackState_NotSupported_001, TestSize.Level1)
{
    AudioProcessConfig processConfig = InitProcessConfig();
    auto rendererStreamImpl = std::make_shared<ProRendererStreamImpl>(processConfig, true);
    // The offload data-callback state is not supported on this stream type.
    constexpr int32_t OFFLOAD_CALLBACK_STATE_INACTIVE = 0;
    int32_t ret = rendererStreamImpl->SetOffloadDataCallbackState(OFFLOAD_CALLBACK_STATE_INACTIVE);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

/**
 * @tc.name  : Test GetStreamFramesWritten guards divide-by-zero when frame size is unset
 * @tc.type  : FUNC
 * @tc.number: GetStreamFramesWritten_ZeroFrameSize_001
 */
HWTEST(ProRendererStreamImplUnitTest, GetStreamFramesWritten_ZeroFrameSize_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    // No InitParams(): byteSizePerFrame_ stays 0 and triggers the div-by-zero guard.
    auto rendererStreamImpl = std::make_shared<ProRendererStreamImpl>(processConfig, true);
    EXPECT_EQ(rendererStreamImpl->byteSizePerFrame_, 0u);

    uint64_t framesWritten = 0;
    int32_t ret = rendererStreamImpl->GetStreamFramesWritten(framesWritten);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : Test GetLatency guards divide-by-zero when frame size is unset
 * @tc.type  : FUNC
 * @tc.number: GetLatency_ZeroFrameSize_001
 */
HWTEST(ProRendererStreamImplUnitTest, GetLatency_ZeroFrameSize_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    // No InitParams(): byteSizePerFrame_ stays 0 and triggers the div-by-zero guard.
    auto rendererStreamImpl = std::make_shared<ProRendererStreamImpl>(processConfig, true);
    EXPECT_EQ(rendererStreamImpl->byteSizePerFrame_, 0u);

    uint64_t latency = 0;
    int32_t ret = rendererStreamImpl->GetLatency(latency);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : Test Start rejects the default invalid status and leaves status unchanged
 * @tc.type  : FUNC
 * @tc.number: Start_InvalidStatus_001
 */
HWTEST(ProRendererStreamImplUnitTest, Start_InvalidStatus_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    auto rendererStreamImpl = std::make_shared<ProRendererStreamImpl>(processConfig, true);
    // Default status is I_STATUS_INVALID; Start() must reject and leave status unchanged.
    EXPECT_EQ(rendererStreamImpl->status_, I_STATUS_INVALID);

    int32_t ret = rendererStreamImpl->Start();
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
    EXPECT_EQ(rendererStreamImpl->status_, I_STATUS_INVALID);
}

/**
 * @tc.name  : Test ReturnIndex ignores a negative index and leaves the write queue unchanged
 * @tc.type  : FUNC
 * @tc.number: ReturnIndex_NegativeIndex_001
 */
HWTEST(ProRendererStreamImplUnitTest, ReturnIndex_NegativeIndex_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    auto rendererStreamImpl = std::make_shared<ProRendererStreamImpl>(processConfig, true);
    // Seed writeQueue_ with a known set of entries (it is empty until InitParams runs).
    rendererStreamImpl->writeQueue_.push(0);
    rendererStreamImpl->writeQueue_.push(1);
    const size_t expectedSize = rendererStreamImpl->writeQueue_.size();
    EXPECT_EQ(expectedSize, 2u);

    constexpr int32_t NEGATIVE_INDEX = -1;
    int32_t ret = rendererStreamImpl->ReturnIndex(NEGATIVE_INDEX);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(rendererStreamImpl->writeQueue_.size(), expectedSize);
}

/**
 * @tc.name  : Test RegisterSinkLatencyFetcher rejects a null fetcher
 * @tc.type  : FUNC
 * @tc.number: RegisterSinkLatencyFetcher_Null_001
 */
HWTEST(ProRendererStreamImplUnitTest, RegisterSinkLatencyFetcher_Null_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    auto rendererStreamImpl = std::make_shared<ProRendererStreamImpl>(processConfig, true);
    // A null fetcher must be rejected and sinkLatencyFetcher_ must remain unset.
    EXPECT_FALSE(rendererStreamImpl->sinkLatencyFetcher_);

    int32_t ret = rendererStreamImpl->RegisterSinkLatencyFetcher(nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
    EXPECT_FALSE(rendererStreamImpl->sinkLatencyFetcher_);
}

/**
 * @tc.name  : Test GetLatencyWithFlag with a software-only flag returns success with zero latency
 * @tc.type  : FUNC
 * @tc.number: GetLatencyWithFlag_SoftwareOnly_001
 */
HWTEST(ProRendererStreamImplUnitTest, GetLatencyWithFlag_SoftwareOnly_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    auto rendererStreamImpl = std::make_shared<ProRendererStreamImpl>(processConfig, true);
    // LATENCY_FLAG_ENGINE carries no HARDWARE bit, so the hardware fetcher path is skipped.
    constexpr uint64_t NON_ZERO_SEED = 1;
    uint64_t latency = NON_ZERO_SEED;
    int32_t ret = rendererStreamImpl->GetLatencyWithFlag(latency, LATENCY_FLAG_ENGINE);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(latency, 0ULL);
}

/**
 * @tc.name  : Test GetLatencyWithFlag with hardware flag but no fetcher fails
 * @tc.type  : FUNC
 * @tc.number: GetLatencyWithFlag_HardwareNoFetcher_001
 */
HWTEST(ProRendererStreamImplUnitTest, GetLatencyWithFlag_HardwareNoFetcher_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    auto rendererStreamImpl = std::make_shared<ProRendererStreamImpl>(processConfig, true);
    // No fetcher registered: requesting HARDWARE latency must fail with ERR_OPERATION_FAILED.
    EXPECT_FALSE(rendererStreamImpl->sinkLatencyFetcher_);

    uint64_t latency = 0;
    int32_t ret = rendererStreamImpl->GetLatencyWithFlag(latency, LATENCY_FLAG_HARDWARE);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test GetLatencyWithFlag propagates the fetcher error code
 * @tc.type  : FUNC
 * @tc.number: GetLatencyWithFlag_HardwareFetcherError_001
 */
HWTEST(ProRendererStreamImplUnitTest, GetLatencyWithFlag_HardwareFetcherError_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    auto rendererStreamImpl = std::make_shared<ProRendererStreamImpl>(processConfig, true);
    // Register a lambda fetcher (no mock class) that always fails.
    auto failingFetcher = [](uint32_t &latencyMs) -> int32_t {
        latencyMs = 0;
        return ERR_OPERATION_FAILED;
    };
    int32_t regRet = rendererStreamImpl->RegisterSinkLatencyFetcher(failingFetcher);
    EXPECT_EQ(regRet, SUCCESS);

    uint64_t latency = 0;
    int32_t ret = rendererStreamImpl->GetLatencyWithFlag(latency, LATENCY_FLAG_HARDWARE);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test GetLatencyWithFlag scales fetcher latency (ms) into microseconds
 * @tc.type  : FUNC
 * @tc.number: GetLatencyWithFlag_HardwareFetcherSuccess_001
 */
HWTEST(ProRendererStreamImplUnitTest, GetLatencyWithFlag_HardwareFetcherSuccess_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    auto rendererStreamImpl = std::make_shared<ProRendererStreamImpl>(processConfig, true);
    // Fetcher reports 5 ms; implementation scales by AUDIO_US_PER_MS (1000) -> 5000 us.
    constexpr uint32_t FETCHER_LATENCY_MS = 5;
    auto okFetcher = [FETCHER_LATENCY_MS](uint32_t &latencyMs) -> int32_t {
        latencyMs = FETCHER_LATENCY_MS;
        return SUCCESS;
    };
    int32_t regRet = rendererStreamImpl->RegisterSinkLatencyFetcher(okFetcher);
    EXPECT_EQ(regRet, SUCCESS);

    uint64_t latency = 0;
    int32_t ret = rendererStreamImpl->GetLatencyWithFlag(latency, LATENCY_FLAG_HARDWARE);
    EXPECT_EQ(ret, SUCCESS);
    const uint64_t expectedLatency = static_cast<uint64_t>(FETCHER_LATENCY_MS) * AUDIO_US_PER_MS;
    EXPECT_EQ(latency, expectedLatency);
}

/**
 * @tc.name  : Test AbortCallback accumulates every abort signal into abortFlag_
 * @tc.type  : FUNC
 * @tc.number: AbortCallback_AccumulatesFlag_001
 */
HWTEST(ProRendererStreamImplUnitTest, AbortCallback_AccumulatesFlag_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    auto rendererStreamImpl = std::make_shared<ProRendererStreamImpl>(processConfig, true);
    rendererStreamImpl->abortFlag_ = 0;

    constexpr int32_t FIRST_ABORT_TIMES = 1;
    constexpr int32_t SECOND_ABORT_TIMES = 2;
    rendererStreamImpl->AbortCallback(FIRST_ABORT_TIMES);
    rendererStreamImpl->AbortCallback(SECOND_ABORT_TIMES);
    const int32_t expectedAbortFlag = FIRST_ABORT_TIMES + SECOND_ABORT_TIMES;
    EXPECT_EQ(rendererStreamImpl->abortFlag_, expectedAbortFlag);
}

/**
 * @tc.name  : Test GetStreamIndex returns the value set by SetStreamIndex
 * @tc.type  : FUNC
 * @tc.number: GetStreamIndex_AfterSet_001
 */
HWTEST(ProRendererStreamImplUnitTest, GetStreamIndex_AfterSet_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    auto rendererStreamImpl = std::make_shared<ProRendererStreamImpl>(processConfig, true);
    constexpr uint32_t EXPECTED_STREAM_INDEX = 7;
    rendererStreamImpl->SetStreamIndex(EXPECTED_STREAM_INDEX);
    EXPECT_EQ(rendererStreamImpl->GetStreamIndex(), EXPECTED_STREAM_INDEX);
}

// ---------------------------------------------------------------------------
// Phase 2 helpers: hand-rolled fakes for Peek's writeCallback switch
// (pro_renderer_stream_impl.cpp:603-632). Held alive via shared_ptr in test
// scope so the weak_ptr registered with the SUT does not expire before Peek.
// ---------------------------------------------------------------------------

// Named constants for Phase 2 Peek branch tests (no magic numbers).
constexpr int32_t INVALID_INDEX_BEFORE_PEEK = -1;
constexpr int32_t SINK_BUFFER_SLOT_ZERO = 0;
constexpr size_t FAKE_MIN_BUFFER_SIZE = 1024;
const std::vector<char> SENTINEL_AUDIO_DATA = {'P', 'E', 'E', 'K'};

// Fake IWriteCallback for testing Peek's writeCallback switch. Only
// OnWriteData(size_t) is exercised by Peek (pro_renderer_stream_impl.cpp:605);
// the other two pure virtuals return SUCCESS and are not invoked here.
class FakeWriteCallback : public IWriteCallback {
public:
    explicit FakeWriteCallback(int32_t code) : returnCode_(code), writeDataSizeCallCount_(0), lastLength_(0) {}
    virtual ~FakeWriteCallback() = default;

    int32_t OnWriteData(size_t length) override
    {
        writeDataSizeCallCount_++;
        lastLength_ = length;
        return returnCode_;
    }
    int32_t OnWriteData(int8_t *inputData, size_t requestDataLen) override { return SUCCESS; }
    int32_t GetAvailableSize(size_t &length) override { return SUCCESS; }

    int32_t GetWriteDataSizeCallCount() const { return writeDataSizeCallCount_; }
    size_t GetLastLength() const { return lastLength_; }

private:
    int32_t returnCode_;
    int32_t writeDataSizeCallCount_;
    size_t lastLength_;
};

// Fake IStatusCallback that records the last observed operation so Peek's
// OPERATION_UNDERFLOW branch (pro_renderer_stream_impl.cpp:611-613) can be
// verified without depending on RendererInServer.
class FakeStatusCallback : public IStatusCallback {
public:
    FakeStatusCallback() : lastOperation_(INVALID_INDEX_BEFORE_PEEK), updateCount_(0) {}
    virtual ~FakeStatusCallback() = default;

    void OnStatusUpdate(IOperation operation) override
    {
        lastOperation_ = static_cast<int32_t>(operation);
        updateCount_++;
    }

    int32_t GetLastOperation() const { return lastOperation_; }
    int32_t GetUpdateCount() const { return updateCount_; }

private:
    int32_t lastOperation_;
    int32_t updateCount_;
};

/**
 * @tc.name  : Test Peek writeCallback SUCCESS branch
 * @tc.type  : FUNC
 * @tc.number: Peek_WriteCallbackSuccess_001
 * @tc.desc  : OnWriteData returns SUCCESS; Peek enters the SUCCESS case
 *             (pro_renderer_stream_impl.cpp:616-622), sets
 *             isFirstNoUnderrunFrame_ and returns SUCCESS.
 */
HWTEST(ProRendererStreamImplUnitTest, Peek_WriteCallbackSuccess_001, TestSize.Level1)
{
    AudioProcessConfig processConfig = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, true);

    // Preconditions to reach the writeCallback switch: data enabled (default),
    // not blocked, empty readQueue (skip short-circuit at line 597-601), and
    // not first frame (avoid 500ms wait inside PopSinkBuffer at line 676-682).
    rendererStreamImpl->isBlock_ = false;
    rendererStreamImpl->isFirstFrame_ = false;
    rendererStreamImpl->minBufferSize_ = FAKE_MIN_BUFFER_SIZE;
    ASSERT_EQ(rendererStreamImpl->readQueue_.empty(), true);

    auto fakeWriteCallback = std::make_shared<FakeWriteCallback>(SUCCESS);
    rendererStreamImpl->RegisterWriteCallback(fakeWriteCallback);

    std::vector<char> audioBuffer;
    int32_t index = INVALID_INDEX_BEFORE_PEEK;
    int32_t result = rendererStreamImpl->Peek(&audioBuffer, index);

    // SUCCESS case (line 616-622): result reassigned to SUCCESS at line 620.
    EXPECT_EQ(result, SUCCESS);
    // OnWriteData(size_t) invoked exactly once with minBufferSize_ (line 605).
    EXPECT_EQ(fakeWriteCallback->GetWriteDataSizeCallCount(), 1);
    EXPECT_EQ(fakeWriteCallback->GetLastLength(), FAKE_MIN_BUFFER_SIZE);
    // isFirstNoUnderrunFrame_ flipped to true by line 619.
    EXPECT_EQ(rendererStreamImpl->isFirstNoUnderrunFrame_.load(), true);
}

/**
 * @tc.name  : Test Peek writeCallback UNDERRUN with first underrun frame
 * @tc.type  : FUNC
 * @tc.number: Peek_WriteCallbackUnderrun_FirstFrame_FiresUnderflow_001
 * @tc.desc  : OnWriteData returns ERR_RENDERER_IN_SERVER_UNDERRUN and
 *             isFirstNoUnderrunFrame_ is true; the status callback receives
 *             OPERATION_UNDERFLOW (pro_renderer_stream_impl.cpp:609-614).
 *             Note: Peek still returns ERR_RENDERER_IN_SERVER_UNDERRUN at
 *             line 633 because result is never reassigned in this case.
 */
HWTEST(ProRendererStreamImplUnitTest, Peek_WriteCallbackUnderrun_FirstFrame_FiresUnderflow_001, TestSize.Level1)
{
    AudioProcessConfig processConfig = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, true);

    rendererStreamImpl->isBlock_ = false;
    rendererStreamImpl->isFirstFrame_ = false;
    ASSERT_EQ(rendererStreamImpl->readQueue_.empty(), true);
    // isFirstNoUnderrunFrame_ gates the OPERATION_UNDERFLOW callback (line 611).
    rendererStreamImpl->isFirstNoUnderrunFrame_ = true;

    auto fakeWriteCallback = std::make_shared<FakeWriteCallback>(ERR_RENDERER_IN_SERVER_UNDERRUN);
    rendererStreamImpl->RegisterWriteCallback(fakeWriteCallback);

    auto fakeStatusCallback = std::make_shared<FakeStatusCallback>();
    rendererStreamImpl->RegisterStatusCallback(fakeStatusCallback);

    std::vector<char> audioBuffer;
    int32_t index = INVALID_INDEX_BEFORE_PEEK;
    int32_t result = rendererStreamImpl->Peek(&audioBuffer, index);

    // UNDERRUN case: result NOT reassigned, returned as-is at line 633.
    EXPECT_EQ(result, ERR_RENDERER_IN_SERVER_UNDERRUN);
    // statusCallback non-null + isFirstNoUnderrunFrame_ true -> fired (line 611-612).
    EXPECT_EQ(fakeStatusCallback->GetUpdateCount(), 1);
    EXPECT_EQ(fakeStatusCallback->GetLastOperation(), static_cast<int32_t>(OPERATION_UNDERFLOW));
}

/**
 * @tc.name  : Test Peek writeCallback UNDERRUN with isFirstNoUnderrunFrame_ false
 * @tc.type  : FUNC
 * @tc.number: Peek_WriteCallbackUnderrun_NotFirstFrame_NoCallback_001
 * @tc.desc  : OnWriteData returns ERR_RENDERER_IN_SERVER_UNDERRUN and
 *             isFirstNoUnderrunFrame_ is false; the OPERATION_UNDERFLOW
 *             callback is suppressed (pro_renderer_stream_impl.cpp:611).
 */
HWTEST(ProRendererStreamImplUnitTest, Peek_WriteCallbackUnderrun_NotFirstFrame_NoCallback_001, TestSize.Level1)
{
    AudioProcessConfig processConfig = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, true);

    rendererStreamImpl->isBlock_ = false;
    rendererStreamImpl->isFirstFrame_ = false;
    ASSERT_EQ(rendererStreamImpl->readQueue_.empty(), true);
    rendererStreamImpl->isFirstNoUnderrunFrame_ = false; // suppresses callback (line 611)

    auto fakeWriteCallback = std::make_shared<FakeWriteCallback>(ERR_RENDERER_IN_SERVER_UNDERRUN);
    rendererStreamImpl->RegisterWriteCallback(fakeWriteCallback);

    auto fakeStatusCallback = std::make_shared<FakeStatusCallback>();
    rendererStreamImpl->RegisterStatusCallback(fakeStatusCallback);

    std::vector<char> audioBuffer;
    int32_t index = INVALID_INDEX_BEFORE_PEEK;
    int32_t result = rendererStreamImpl->Peek(&audioBuffer, index);

    // Returned as-is at line 633 (result never reassigned in UNDERRUN case).
    EXPECT_EQ(result, ERR_RENDERER_IN_SERVER_UNDERRUN);
    EXPECT_EQ(fakeStatusCallback->GetUpdateCount(), 0); // callback suppressed
}

/**
 * @tc.name  : Test Peek writeCallback non-SUCCESS/non-UNDERRUN error
 * @tc.type  : FUNC
 * @tc.number: Peek_WriteCallbackError_Propagated_001
 * @tc.desc  : OnWriteData returns a generic error code; Peek hits the default
 *             case and returns that code early
 *             (pro_renderer_stream_impl.cpp:624-627).
 */
HWTEST(ProRendererStreamImplUnitTest, Peek_WriteCallbackError_Propagated_001, TestSize.Level1)
{
    AudioProcessConfig processConfig = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, true);

    rendererStreamImpl->isBlock_ = false;
    rendererStreamImpl->isFirstFrame_ = false;
    ASSERT_EQ(rendererStreamImpl->readQueue_.empty(), true);

    // Any error code that is neither SUCCESS nor ERR_RENDERER_IN_SERVER_UNDERRUN
    // falls into the default branch (line 624) and is returned at line 626.
    auto fakeWriteCallback = std::make_shared<FakeWriteCallback>(ERR_OPERATION_FAILED);
    rendererStreamImpl->RegisterWriteCallback(fakeWriteCallback);

    std::vector<char> audioBuffer;
    int32_t index = INVALID_INDEX_BEFORE_PEEK;
    int32_t result = rendererStreamImpl->Peek(&audioBuffer, index);

    EXPECT_EQ(result, ERR_OPERATION_FAILED); // returned early at line 626
    EXPECT_EQ(fakeWriteCallback->GetWriteDataSizeCallCount(), 1);
}

/**
 * @tc.name  : Test Peek short-circuits when readQueue_ is non-empty
 * @tc.type  : FUNC
 * @tc.number: Peek_ReadQueueNonEmpty_ShortCircuits_001
 * @tc.desc  : A non-empty readQueue_ makes Peek return SUCCESS via
 *             PopSinkBuffer (pro_renderer_stream_impl.cpp:597-601) without
 *             invoking the write callback at line 603-605.
 */
HWTEST(ProRendererStreamImplUnitTest, Peek_ReadQueueNonEmpty_ShortCircuits_001, TestSize.Level1)
{
    AudioProcessConfig processConfig = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, true);

    rendererStreamImpl->isBlock_ = false;
    rendererStreamImpl->isFirstFrame_ = false;

    // Provide a sinkBuffer_ slot and enqueue its index so readQueue_ is non-empty
    // and PopSinkBuffer (line 685-689) can copy the sentinel through.
    rendererStreamImpl->sinkBuffer_.resize(1);
    rendererStreamImpl->sinkBuffer_[SINK_BUFFER_SLOT_ZERO] = SENTINEL_AUDIO_DATA;
    rendererStreamImpl->readQueue_.push(SINK_BUFFER_SLOT_ZERO);
    ASSERT_EQ(rendererStreamImpl->readQueue_.empty(), false);

    // Register a callback whose OnWriteData counts calls; it must NOT be invoked
    // because the short-circuit at line 597-601 skips the writeCallback entirely.
    auto fakeWriteCallback = std::make_shared<FakeWriteCallback>(SUCCESS);
    rendererStreamImpl->RegisterWriteCallback(fakeWriteCallback);

    std::vector<char> audioBuffer;
    int32_t index = INVALID_INDEX_BEFORE_PEEK;
    int32_t result = rendererStreamImpl->Peek(&audioBuffer, index);

    EXPECT_EQ(result, SUCCESS); // result initialized to SUCCESS, returned at line 600
    EXPECT_EQ(fakeWriteCallback->GetWriteDataSizeCallCount(), 0); // short-circuit, no callback
    EXPECT_EQ(audioBuffer, SENTINEL_AUDIO_DATA); // PopSinkBuffer copied the slot (line 688)
    EXPECT_EQ(index, SINK_BUFFER_SLOT_ZERO); // readQueue_.front() per line 686
}

/**
 * @tc.name  : Test EnqueueBuffer copies an EAC3 passthrough buffer directly
 * @tc.type  : FUNC
 * @tc.number: EnqueueBuffer_EAC3_001
 * @tc.desc  : With streamInfo.encoding == ENCODING_EAC3, EnqueueBuffer takes the
 *             direct memcpy passthrough path (pro_renderer_stream_impl.cpp:462-467)
 *             instead of the mcr/resample/3DA conversion branches.
 */
HWTEST(ProRendererStreamImplUnitTest, EnqueueBuffer_EAC3_001, TestSize.Level1)
{
    AudioProcessConfig processConfig = InitProcessConfig();
    processConfig.streamInfo.encoding = ENCODING_EAC3;
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);

    ASSERT_EQ(rendererStreamImpl->InitParams(), SUCCESS);

    // A modest passthrough buffer (well within one sinkBuffer_ slot) carrying a sentinel.
    constexpr uint32_t EAC3_BUFFER_LEN = 256;
    const uint8_t EAC3_SENTINEL = 0xE3;
    std::vector<uint8_t> eac3Buffer(EAC3_BUFFER_LEN, EAC3_SENTINEL);

    BufferDesc bufferDesc;
    bufferDesc.buffer = eac3Buffer.data();
    bufferDesc.bufLength = EAC3_BUFFER_LEN;

    int32_t ret = rendererStreamImpl->EnqueueBuffer(bufferDesc);
    EXPECT_EQ(ret, SUCCESS);

    // The EAC3 path copies bufLength bytes verbatim into the first write slot (line 463-464).
    const auto &sinkSlot = rendererStreamImpl->sinkBuffer_[SINK_BUFFER_SLOT_ZERO];
    ASSERT_GE(sinkSlot.size(), static_cast<size_t>(EAC3_BUFFER_LEN));
    EXPECT_EQ(static_cast<uint8_t>(sinkSlot[0]), EAC3_SENTINEL);
    EXPECT_EQ(static_cast<uint8_t>(sinkSlot[EAC3_BUFFER_LEN - 1]), EAC3_SENTINEL);
}

/**
 * @tc.name  : Test Peek returns ERR_OPERATION_FAILED when data sending is disabled
 * @tc.type  : FUNC
 * @tc.number: Peek_SendDataDisabled_001
 * @tc.desc  : SetSendDataEnabled(false) flips sendDataEnabled_ to false; Peek then
 *             hits the early-return guard at pro_renderer_stream_impl.cpp:593-594
 *             and returns ERR_OPERATION_FAILED. Re-enabling restores the path.
 */
HWTEST(ProRendererStreamImplUnitTest, Peek_SendDataDisabled_001, TestSize.Level1)
{
    AudioProcessConfig processConfig = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, true);

    // Default sendDataEnabled_ is true; disable it to exercise the guard at line 593-594.
    rendererStreamImpl->SetSendDataEnabled(false);
    rendererStreamImpl->isBlock_ = false;

    std::vector<char> audioBuffer;
    int32_t index = INVALID_INDEX_BEFORE_PEEK;
    int32_t result = rendererStreamImpl->Peek(&audioBuffer, index);
    EXPECT_EQ(result, ERR_OPERATION_FAILED); // guard at line 593-594

    // Re-enabling restores the data-enabled path: with no callback registered, Peek
    // now proceeds past the guard to the writeCallback==nullptr else branch (line 629-631).
    rendererStreamImpl->SetSendDataEnabled(true);
    int32_t restored = rendererStreamImpl->Peek(&audioBuffer, index);
    EXPECT_EQ(restored, ERR_WRITE_BUFFER);
}

/**
 * @tc.name  : Test GetCurrentPosition with BOOTTIME clock base
 * @tc.type  : FUNC
 * @tc.number: GetCurrentPosition_Boottime_001
 * @tc.desc  : With base == Timestamp::BOOTTIME, GetCurrentPosition takes the
 *             GetBootNano() branch (pro_renderer_stream_impl.cpp:325) and returns SUCCESS.
 */
HWTEST(ProRendererStreamImplUnitTest, GetCurrentPosition_Boottime_001, TestSize.Level1)
{
    AudioProcessConfig processConfig = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, true);
    ASSERT_EQ(rendererStreamImpl->InitParams(), SUCCESS);

    uint64_t framePosition = 0;
    uint64_t timestamp = 0;
    uint64_t latency = 0;
    int32_t ret = rendererStreamImpl->GetCurrentPosition(framePosition, timestamp, latency,
        Timestamp::BOOTTIME);
    EXPECT_EQ(ret, SUCCESS); // BOOTTIME branch at line 325
}

/**
 * @tc.name  : Test Drain returns immediately when the read queue is already empty
 * @tc.type  : FUNC
 * @tc.number: Drain_EmptyReadQueue_001
 * @tc.desc  : With an empty readQueue_, Drain skips the drainSync_.wait_for at
 *             pro_renderer_stream_impl.cpp:257-261 and completes immediately.
 */
HWTEST(ProRendererStreamImplUnitTest, Drain_EmptyReadQueue_001, TestSize.Level1)
{
    AudioProcessConfig processConfig = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, true);
    ASSERT_EQ(rendererStreamImpl->readQueue_.empty(), true); // empty -> skip wait at line 257

    int32_t ret = rendererStreamImpl->Drain();
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(rendererStreamImpl->status_, I_STATUS_DRAINED); // status set at line 267
}
} // namespace AudioStandard
} // namespace OHOS