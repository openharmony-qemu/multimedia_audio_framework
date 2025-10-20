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
    EXPECT_NE(ret, SUCCESS);
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
    EXPECT_EQ(ret, ERR_WRITE_BUFFER);
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
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);

    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_S16LE;
    streamInfo.channels = STEREO;
    rendererStreamImpl->InitBasicInfo(streamInfo);
    EXPECT_EQ(rendererStreamImpl->currentRate_, SAMPLE_RATE_48000);
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
} // namespace AudioStandard
} // namespace OHOS