/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "audio_renderer_engine_aiut.h"
#include "aiut_log.h"
#include "audio_renderer_private.h"
#include "audio_errors.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {

namespace {
constexpr size_t TEST_BUFFER_SIZE = 4096;
}

void AudioRendererEngineAiut::SetUpTestCase(void) {}
void AudioRendererEngineAiut::TearDownTestCase(void) {}
void AudioRendererEngineAiut::SetUp(void) {}
void AudioRendererEngineAiut::TearDown(void) {}

// ==================== FadeInAudioBuffer Tests ====================

/**
 * @tc.name  : Test FadeInAudioBuffer with nullptr buffer
 * @tc.number: FadeInAudioBuffer_001
 * @tc.desc  : Test FadeInAudioBuffer returns ERR_INVALID_PARAM when buffer.buffer is nullptr.
 */
HWTEST_F(AudioRendererEngineAiut, FadeInAudioBuffer_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    BufferDesc bufferDesc = {nullptr, 0, 0};
    int32_t ret = AudioRenderer::FadeInAudioBuffer(bufferDesc, SAMPLE_S16LE, STEREO);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM,
        "nullptr buffer should return ERR_INVALID_PARAM, ret=%d", ret);
}

/**
 * @tc.name  : Test FadeInAudioBuffer with zero bufLength
 * @tc.number: FadeInAudioBuffer_002
 * @tc.desc  : Test FadeInAudioBuffer returns ERR_INVALID_PARAM when bufLength is 0.
 */
HWTEST_F(AudioRendererEngineAiut, FadeInAudioBuffer_002, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    uint8_t buffer[TEST_BUFFER_SIZE];
    BufferDesc bufferDesc = {buffer, 0, TEST_BUFFER_SIZE};
    int32_t ret = AudioRenderer::FadeInAudioBuffer(bufferDesc, SAMPLE_S16LE, STEREO);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM,
        "zero bufLength should return ERR_INVALID_PARAM, ret=%d", ret);
}

/**
 * @tc.name  : Test FadeInAudioBuffer with zero dataLength
 * @tc.number: FadeInAudioBuffer_003
 * @tc.desc  : Test FadeInAudioBuffer returns ERR_INVALID_PARAM when dataLength is 0.
 */
HWTEST_F(AudioRendererEngineAiut, FadeInAudioBuffer_003, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    uint8_t buffer[TEST_BUFFER_SIZE];
    BufferDesc bufferDesc = {buffer, TEST_BUFFER_SIZE, 0};
    int32_t ret = AudioRenderer::FadeInAudioBuffer(bufferDesc, SAMPLE_S16LE, STEREO);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM,
        "zero dataLength should return ERR_INVALID_PARAM, ret=%d", ret);
}

// ==================== FadeOutAudioBuffer Tests ====================

/**
 * @tc.name  : Test FadeOutAudioBuffer with nullptr buffer
 * @tc.number: FadeOutAudioBuffer_001
 * @tc.desc  : Test FadeOutAudioBuffer returns ERR_INVALID_PARAM when buffer.buffer is nullptr.
 */
HWTEST_F(AudioRendererEngineAiut, FadeOutAudioBuffer_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    BufferDesc bufferDesc = {nullptr, 0, 0};
    int32_t ret = AudioRenderer::FadeOutAudioBuffer(bufferDesc, SAMPLE_S16LE, STEREO);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM,
        "nullptr buffer should return ERR_INVALID_PARAM, ret=%d", ret);
}

/**
 * @tc.name  : Test FadeOutAudioBuffer with zero bufLength
 * @tc.number: FadeOutAudioBuffer_002
 * @tc.desc  : Test FadeOutAudioBuffer returns ERR_INVALID_PARAM when bufLength is 0.
 */
HWTEST_F(AudioRendererEngineAiut, FadeOutAudioBuffer_002, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    uint8_t buffer[TEST_BUFFER_SIZE];
    BufferDesc bufferDesc = {buffer, 0, TEST_BUFFER_SIZE};
    int32_t ret = AudioRenderer::FadeOutAudioBuffer(bufferDesc, SAMPLE_S16LE, STEREO);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM,
        "zero bufLength should return ERR_INVALID_PARAM, ret=%d", ret);
}

/**
 * @tc.name  : Test FadeOutAudioBuffer with zero dataLength
 * @tc.number: FadeOutAudioBuffer_003
 * @tc.desc  : Test FadeOutAudioBuffer returns ERR_INVALID_PARAM when dataLength is 0.
 */
HWTEST_F(AudioRendererEngineAiut, FadeOutAudioBuffer_003, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    uint8_t buffer[TEST_BUFFER_SIZE];
    BufferDesc bufferDesc = {buffer, TEST_BUFFER_SIZE, 0};
    int32_t ret = AudioRenderer::FadeOutAudioBuffer(bufferDesc, SAMPLE_S16LE, STEREO);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM,
        "zero dataLength should return ERR_INVALID_PARAM, ret=%d", ret);
}

} // namespace AudioStandard
} // namespace OHOS
