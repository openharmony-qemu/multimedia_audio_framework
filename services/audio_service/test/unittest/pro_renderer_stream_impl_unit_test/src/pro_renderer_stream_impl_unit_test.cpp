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
 * @tc.number: GetDirectSampleRate
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
 * @tc.name  : Test OnAddSystemAbility API
 * @tc.type  : FUNC
 * @tc.number: AudioServerOnAddSystemAbility_001
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
 * @tc.name  : Test Pause API
 * @tc.type  : FUNC
 * @tc.number: Pause
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
    rendererStreamImpl->DequeueBuffer(length);
    EXPECT_EQ(rendererStreamImpl != nullptr, true);
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

    size_t length = 10;
    rendererStreamImpl->DequeueBuffer(length);
    EXPECT_EQ(rendererStreamImpl != nullptr, true);
}

/**
 * @tc.name  : Test DequeueBuffer API
 * @tc.type  : FUNC
 * @tc.number: DequeueBuffer
 */
HWTEST(ProRendererStreamImplUnitTest, DequeueBuffer_003, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    rendererStreamImpl->status_ = I_STATUS_STARTED;
    size_t length = 10;
    rendererStreamImpl->DequeueBuffer(length);
    EXPECT_EQ(rendererStreamImpl != nullptr, true);
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

    const BufferDesc bufferDesc = { nullptr, 0, 0};
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
    const BufferDesc bufferDesc = { nullptr, 0, 0};
    int32_t ret = rendererStreamImpl->EnqueueBuffer(bufferDesc);
    EXPECT_EQ(ret, SUCCESS);
}


/**
 * @tc.name  : Test EnqueueBuffer API
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
HWTEST(ProRendererStreamImplUnitTest, ffloadSetVolume_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    
    float volume = 0;
    int32_t ret = rendererStreamImpl->OffloadSetVolume(volume);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 *@tc.name  : Test GetAudioTime API
 *@tc.type  : FUNC
 *@tc.number: GetAudioTime
 */
HWTEST(ProRendererStreamImplUnitTest, GetAudioTime_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    uint64_t framePos;
    int64_t sec, nanoSec;
    int32_t ret = rendererStreamImpl->GetAudioTime(framePos, sec, nanoSec);
    EXPECT_EQ(ret, SUCCESS);
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

    const BufferDesc bufferDesc = { nullptr, 0, 0};
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

    const BufferDesc bufferDesc = { nullptr, 0, 0};
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
    AudioProcessConfig processConfig;
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);

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
    int32_t ret = rendererStreamImpl->GetCurrentPosition(framePosition, timestamp, latency);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 *@tc.name  : Test GetLatency API
 *@tc.type  : FUNC
 *@tc.number: GetLatency
*/
HWTEST(ProRendererStreamImplUnitTest, GetLatency_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> rendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);

    uint64_t latency;
    int32_t ret = rendererStreamImpl->GetLatency(latency);
    EXPECT_EQ(ret, SUCCESS);
} // namespace AudioStandard
} // namespace OHOS
