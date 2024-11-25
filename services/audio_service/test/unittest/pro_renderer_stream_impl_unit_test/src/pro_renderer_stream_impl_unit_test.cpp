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
/**
 * @tc.name  : Test GetDirectSampleRate API
 * @tc.type  : FUNC
 * @tc.number: GetDirectSampleRate
 */
HWTEST(ProRendererStreamImplUnitTest, GetDirectSampleRate_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    bool isDirect = true;
    std::shared_ptr<ProRendererStreamImpl> RendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    RendererStreamImpl->processConfig_.streamType = STREAM_VOICE_COMMUNICATION;
    AudioSamplingRate sampleRate = SAMPLE_RATE_16000;

    AudioSamplingRate ret = RendererStreamImpl->GetDirectSampleRate(sampleRate);
    EXPECT_EQ(ret, SAMPLE_RATE_16000);

    sampleRate = SAMPLE_RATE_48000;
    ret = RendererStreamImpl->GetDirectSampleRate(sampleRate);
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
    std::shared_ptr<ProRendererStreamImpl> RendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    RendererStreamImpl->status_ = I_STATUS_STARTED;
    int32_t ret = RendererStreamImpl->Start();
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
    std::shared_ptr<ProRendererStreamImpl> RendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    RendererStreamImpl->status_ = I_STATUS_PAUSED;
    RendererStreamImpl->isFirstFrame_ = false;
    int32_t ret = RendererStreamImpl->Pause();
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
    std::shared_ptr<ProRendererStreamImpl> RendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    RendererStreamImpl->readQueue_.push(1);
    RendererStreamImpl->isDrain_ = true;
    int32_t ret = RendererStreamImpl->Flush();
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
    std::shared_ptr<ProRendererStreamImpl> RendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);
    RendererStreamImpl->readQueue_.push(1);
    int32_t ret = RendererStreamImpl->Drain();
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
    std::shared_ptr<ProRendererStreamImpl> RendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);

    int32_t rate = RENDER_RATE_NORMAL;
    int32_t ret = RendererStreamImpl->SetRate(rate);
    EXPECT_EQ(ret, SUCCESS);

    rate = RENDER_RATE_DOUBLE;
    ret = RendererStreamImpl->SetRate(rate);
    EXPECT_EQ(ret, SUCCESS);

    rate = RENDER_RATE_HALF;
    ret = RendererStreamImpl->SetRate(rate);
    EXPECT_EQ(ret, SUCCESS);

    rate = static_cast<AudioRendererRate>(3);;
    ret = RendererStreamImpl->SetRate(rate);
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
    std::shared_ptr<ProRendererStreamImpl> RendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);

    RendererStreamImpl->status_ = I_STATUS_IDLE;
    size_t length = 10;
    RendererStreamImpl->DequeueBuffer(length);
    EXPECT_EQ(RendererStreamImpl != nullptr, true);
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
    std::shared_ptr<ProRendererStreamImpl> RendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);

    size_t length = 10;
    RendererStreamImpl->DequeueBuffer(length);
    EXPECT_EQ(RendererStreamImpl != nullptr, true);
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
    std::shared_ptr<ProRendererStreamImpl> RendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);

    const BufferDesc bufferDesc = { nullptr, 0, 0};
    int32_t ret = RendererStreamImpl->EnqueueBuffer(bufferDesc);
    EXPECT_EQ(ret, ERR_WRITE_BUFFER);
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
    std::shared_ptr<ProRendererStreamImpl> RendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);

    int32_t index = 10;
    int32_t ret = RendererStreamImpl->ReturnIndex(index);
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
    std::shared_ptr<ProRendererStreamImpl> RendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);

    std::vector<char> audioBuffer;
    int32_t index;
    RendererStreamImpl->isFirstFrame_ = true;
    RendererStreamImpl->PopSinkBuffer(&audioBuffer, index);
    EXPECT_EQ(RendererStreamImpl->isFirstFrame_, true);
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
    std::shared_ptr<ProRendererStreamImpl> RendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);

    std::vector<char> audioBuffer;
    int32_t index;
    RendererStreamImpl->isFirstFrame_ = false;
    RendererStreamImpl->isDrain_ = true;
    RendererStreamImpl->PopSinkBuffer(&audioBuffer, index);
    EXPECT_EQ(RendererStreamImpl->isFirstFrame_, false);
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
    std::shared_ptr<ProRendererStreamImpl> RendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);

    const BufferDesc bufferDesc = { nullptr, 0, 0};
    RendererStreamImpl->bufferInfo_.format = AudioSampleFormat::SAMPLE_F32LE;
    RendererStreamImpl->ConvertSrcToFloat(bufferDesc);
    EXPECT_EQ(RendererStreamImpl != nullptr, true);
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
    std::shared_ptr<ProRendererStreamImpl> RendererStreamImpl =
        std::make_shared<ProRendererStreamImpl>(processConfig, isDirect);

    const BufferDesc bufferDesc = { nullptr, 0, 0};
    RendererStreamImpl->ConvertSrcToFloat(bufferDesc);
    EXPECT_EQ(RendererStreamImpl != nullptr, true);
}
} // namespace AudioStandard
} // namespace OHOS
