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

#include "audio_renderer_base_aiut.h"
#include "aiut_log.h"
#include "audio_renderer_private.h"
#include "audio_errors.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {

namespace {
constexpr size_t TEST_BUFFER_SIZE = 4096;
constexpr size_t TEST_META_BUFFER_SIZE = 19824;
constexpr uint32_t TEST_SAMPLE_RATE_44100 = 44100;
constexpr uint32_t TEST_SAMPLE_RATE_48000 = 48000;
constexpr uint32_t TEST_SAMPLE_RATE_8000 = 8000;
constexpr uint32_t TEST_SAMPLE_RATE_384000 = 384000;
constexpr uint32_t TEST_SAMPLE_RATE_11025 = 11025;
constexpr uint32_t TEST_SAMPLE_RATE_7999 = 7999;
constexpr uint32_t TEST_SAMPLE_RATE_16010 = 16010;
constexpr int32_t TEST_RENDERER_FLAGS_NORMAL = 0;
constexpr int32_t TEST_RENDERER_FLAGS_MMAP = 1;
constexpr int32_t TEST_RENDERER_FLAGS_FORCED_NORMAL = 4;
constexpr int32_t TEST_RENDERER_FLAGS_VKB_NORMAL = 16;
constexpr int32_t TEST_RENDERER_FLAGS_VKB_FAST = 32;
constexpr int32_t TEST_RENDERER_FLAGS_VKB_ALL_RING = 48;
constexpr int32_t TEST_RENDERER_FLAGS_INVALID = 999;
constexpr float TEST_VOLUME_HALF = 0.5f;
constexpr float TEST_SPEED_NORMAL = 1.0f;
constexpr float TEST_SPEED_MIN = 0.25f;
constexpr float TEST_PITCH_NORMAL = 1.0f;
constexpr float TEST_PITCH_MIN = 0.25f;
constexpr float TEST_PITCH_MAX = 4.0f;
constexpr int32_t TEST_APP_UID = 1234;
constexpr int32_t TEST_APP_PID = 5678;
}

void AudioRendererBaseAiut::SetUpTestCase(void) {}
void AudioRendererBaseAiut::TearDownTestCase(void) {}
void AudioRendererBaseAiut::SetUp(void) {}
void AudioRendererBaseAiut::TearDown(void) {}

// ==================== ConvertToAudioStreamParams Tests ====================

/**
 * @tc.name  : Test ConvertToAudioStreamParams with valid S16LE params
 * @tc.number: ConvertToAudioStreamParams_001
 * @tc.desc  : Test static inline ConvertToAudioStreamParams converts AudioRendererParams to
 *             AudioStreamParams correctly for S16LE/44100/Stereo/PCM.
 */
HWTEST_F(AudioRendererBaseAiut, ConvertToAudioStreamParams_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    AudioRendererParams rendererParams;
    rendererParams.sampleFormat = SAMPLE_S16LE;
    rendererParams.sampleRate = SAMPLE_RATE_44100;
    rendererParams.channelCount = CHANNEL_2;
    rendererParams.encodingType = ENCODING_PCM;
    rendererParams.channelLayout = CHANNEL_LAYOUT_STEREO;

    AudioStreamParams streamParams = AudioRendererPrivate::ConvertToAudioStreamParams(rendererParams);

    AIUT_EXPECT_EQ_INFO_LOG(streamParams.format, SAMPLE_S16LE, "format mismatch");
    AIUT_EXPECT_EQ_INFO_LOG(streamParams.samplingRate, SAMPLE_RATE_44100, "samplingRate mismatch");
    AIUT_EXPECT_EQ_INFO_LOG(streamParams.channels, CHANNEL_2, "channels mismatch");
    AIUT_EXPECT_EQ_INFO_LOG(streamParams.encoding, ENCODING_PCM, "encoding mismatch");
}

/**
 * @tc.name  : Test ConvertToAudioStreamParams with different format
 * @tc.number: ConvertToAudioStreamParams_002
 * @tc.desc  : Test ConvertToAudioStreamParams converts SAMPLE_U8/MONO correctly.
 */
HWTEST_F(AudioRendererBaseAiut, ConvertToAudioStreamParams_002, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    AudioRendererParams rendererParams;
    rendererParams.sampleFormat = SAMPLE_U8;
    rendererParams.sampleRate = SAMPLE_RATE_8000;
    rendererParams.channelCount = CHANNEL_1;
    rendererParams.encodingType = ENCODING_PCM;

    AudioStreamParams streamParams = AudioRendererPrivate::ConvertToAudioStreamParams(rendererParams);

    AIUT_EXPECT_EQ_INFO_LOG(streamParams.format, SAMPLE_U8, "format mismatch");
    AIUT_EXPECT_EQ_INFO_LOG(streamParams.samplingRate, SAMPLE_RATE_8000, "samplingRate mismatch");
    AIUT_EXPECT_EQ_INFO_LOG(streamParams.channels, CHANNEL_1, "channels mismatch");
    AIUT_EXPECT_EQ_INFO_LOG(streamParams.encoding, ENCODING_PCM, "encoding mismatch");
}

/**
 * @tc.name  : Test ConvertToAudioStreamParams preserves channelLayout
 * @tc.number: ConvertToAudioStreamParams_003
 * @tc.desc  : Test ConvertToAudioStreamParams preserves channelLayout field.
 */
HWTEST_F(AudioRendererBaseAiut, ConvertToAudioStreamParams_003, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    AudioRendererParams rendererParams;
    rendererParams.sampleFormat = SAMPLE_S32LE;
    rendererParams.sampleRate = SAMPLE_RATE_48000;
    rendererParams.channelCount = CHANNEL_2;
    rendererParams.encodingType = ENCODING_PCM;
    rendererParams.channelLayout = CHANNEL_LAYOUT_STEREO;

    AudioStreamParams streamParams = AudioRendererPrivate::ConvertToAudioStreamParams(rendererParams);

    AIUT_EXPECT_EQ_INFO_LOG(streamParams.channelLayout, CHANNEL_LAYOUT_STEREO, "channelLayout mismatch");
}

// ==================== CheckSupportedSamplingRates Tests ====================

/**
 * @tc.name  : Test CheckSupportedSamplingRates with valid rates
 * @tc.number: CheckSupportedSamplingRates_001
 * @tc.desc  : Test CheckSupportedSamplingRates returns true for standard valid rates.
 */
HWTEST_F(AudioRendererBaseAiut, CheckSupportedSamplingRates_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    AIUT_EXPECT_TRUE_INFO_LOG(AudioRenderer::CheckSupportedSamplingRates(TEST_SAMPLE_RATE_8000),
        "8000 should be valid");
    AIUT_EXPECT_TRUE_INFO_LOG(AudioRenderer::CheckSupportedSamplingRates(TEST_SAMPLE_RATE_44100),
        "44100 should be valid");
    AIUT_EXPECT_TRUE_INFO_LOG(AudioRenderer::CheckSupportedSamplingRates(TEST_SAMPLE_RATE_48000),
        "48000 should be valid");
    AIUT_EXPECT_TRUE_INFO_LOG(AudioRenderer::CheckSupportedSamplingRates(TEST_SAMPLE_RATE_384000),
        "384000 should be valid");
    AIUT_EXPECT_TRUE_INFO_LOG(AudioRenderer::CheckSupportedSamplingRates(TEST_SAMPLE_RATE_11025),
        "11025 should be valid");
}

/**
 * @tc.name  : Test CheckSupportedSamplingRates with invalid rates
 * @tc.number: CheckSupportedSamplingRates_002
 * @tc.desc  : Test CheckSupportedSamplingRates returns false for invalid rates.
 */
HWTEST_F(AudioRendererBaseAiut, CheckSupportedSamplingRates_002, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    AIUT_EXPECT_FALSE_INFO_LOG(AudioRenderer::CheckSupportedSamplingRates(TEST_SAMPLE_RATE_7999),
        "7999 should be invalid");
    AIUT_EXPECT_FALSE_INFO_LOG(AudioRenderer::CheckSupportedSamplingRates(TEST_SAMPLE_RATE_16010),
        "16010 should be invalid");
    AIUT_EXPECT_FALSE_INFO_LOG(AudioRenderer::CheckSupportedSamplingRates(0),
        "0 should be invalid");
    AIUT_EXPECT_FALSE_INFO_LOG(AudioRenderer::CheckSupportedSamplingRates(384001),
        "384001 should be invalid");
}

// ==================== GetSupportedFormats/Channels/EncodingTypes/SamplingRates Tests ====================

/**
 * @tc.name  : Test GetSupportedFormats returns non-empty vector
 * @tc.number: GetSupportedFormats_001
 * @tc.desc  : Verify GetSupportedFormats returns a non-empty list of supported sample formats.
 */
HWTEST_F(AudioRendererBaseAiut, GetSupportedFormats_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto formats = AudioRenderer::GetSupportedFormats();
    AIUT_EXPECT_FALSE_INFO_LOG(formats.empty(), "supported formats should not be empty");
}

/**
 * @tc.name  : Test GetSupportedChannels returns non-empty vector
 * @tc.number: GetSupportedChannels_001
 * @tc.desc  : Verify GetSupportedChannels returns a non-empty list of supported channels.
 */
HWTEST_F(AudioRendererBaseAiut, GetSupportedChannels_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto channels = AudioRenderer::GetSupportedChannels();
    AIUT_EXPECT_FALSE_INFO_LOG(channels.empty(), "supported channels should not be empty");
}

/**
 * @tc.name  : Test GetSupportedEncodingTypes returns non-empty vector
 * @tc.number: GetSupportedEncodingTypes_001
 * @tc.desc  : Verify GetSupportedEncodingTypes returns a non-empty list.
 */
HWTEST_F(AudioRendererBaseAiut, GetSupportedEncodingTypes_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto encodings = AudioRenderer::GetSupportedEncodingTypes();
    AIUT_EXPECT_FALSE_INFO_LOG(encodings.empty(), "supported encoding types should not be empty");
}

/**
 * @tc.name  : Test GetSupportedSamplingRates returns non-empty vector
 * @tc.number: GetSupportedSamplingRates_001
 * @tc.desc  : Verify GetSupportedSamplingRates returns a non-empty list.
 */
HWTEST_F(AudioRendererBaseAiut, GetSupportedSamplingRates_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto rates = AudioRenderer::GetSupportedSamplingRates();
    AIUT_EXPECT_FALSE_INFO_LOG(rates.empty(), "supported sampling rates should not be empty");
}

// ==================== MuteAudioBuffer Tests ====================

/**
 * @tc.name  : Test MuteAudioBuffer with valid S16LE buffer
 * @tc.number: MuteAudioBuffer_001
 * @tc.desc  : Test MuteAudioBuffer fills S16LE buffer with zeros successfully.
 */
HWTEST_F(AudioRendererBaseAiut, MuteAudioBuffer_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    uint8_t buffer[TEST_BUFFER_SIZE];
    // Fill buffer with non-zero data
    for (size_t i = 0; i < TEST_BUFFER_SIZE; i++) {
        buffer[i] = 0xAA;
    }

    int32_t ret = AudioRenderer::MuteAudioBuffer(buffer, 0, TEST_BUFFER_SIZE, SAMPLE_S16LE);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "MuteAudioBuffer S16LE should succeed, ret=%d", ret);

    // Verify buffer is zeroed
    bool allZero = true;
    for (size_t i = 0; i < TEST_BUFFER_SIZE; i++) {
        if (buffer[i] != 0) {
            allZero = false;
            break;
        }
    }
    AIUT_EXPECT_TRUE_INFO_LOG(allZero, "S16LE buffer should be zeroed after mute");
}

/**
 * @tc.name  : Test MuteAudioBuffer with valid U8 buffer
 * @tc.number: MuteAudioBuffer_002
 * @tc.desc  : Test MuteAudioBuffer fills U8 buffer with 0x7F (silence for unsigned 8-bit).
 */
HWTEST_F(AudioRendererBaseAiut, MuteAudioBuffer_002, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    uint8_t buffer[TEST_BUFFER_SIZE];
    for (size_t i = 0; i < TEST_BUFFER_SIZE; i++) {
        buffer[i] = 0x00;
    }

    int32_t ret = AudioRenderer::MuteAudioBuffer(buffer, 0, TEST_BUFFER_SIZE, SAMPLE_U8);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "MuteAudioBuffer U8 should succeed, ret=%d", ret);

    // Verify buffer is filled with 0x7F
    bool allSilent = true;
    for (size_t i = 0; i < TEST_BUFFER_SIZE; i++) {
        if (buffer[i] != 0x7F) {
            allSilent = false;
            break;
        }
    }
    AIUT_EXPECT_TRUE_INFO_LOG(allSilent, "U8 buffer should be 0x7F after mute");
}

/**
 * @tc.name  : Test MuteAudioBuffer with nullptr
 * @tc.number: MuteAudioBuffer_003
 * @tc.desc  : Test MuteAudioBuffer returns ERR_INVALID_PARAM when addr is nullptr.
 */
HWTEST_F(AudioRendererBaseAiut, MuteAudioBuffer_003, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    int32_t ret = AudioRenderer::MuteAudioBuffer(nullptr, 0, TEST_BUFFER_SIZE, SAMPLE_S16LE);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM, "nullptr should return ERR_INVALID_PARAM, ret=%d", ret);
}

/**
 * @tc.name  : Test MuteAudioBuffer with zero length
 * @tc.number: MuteAudioBuffer_004
 * @tc.desc  : Test MuteAudioBuffer returns ERR_INVALID_PARAM when length is 0.
 */
HWTEST_F(AudioRendererBaseAiut, MuteAudioBuffer_004, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    uint8_t buffer[TEST_BUFFER_SIZE];
    int32_t ret = AudioRenderer::MuteAudioBuffer(buffer, 0, 0, SAMPLE_S16LE);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM, "zero length should return ERR_INVALID_PARAM, ret=%d", ret);
}

/**
 * @tc.name  : Test MuteAudioBuffer with offset
 * @tc.number: MuteAudioBuffer_005
 * @tc.desc  : Test MuteAudioBuffer mutes only the portion starting from offset.
 */
HWTEST_F(AudioRendererBaseAiut, MuteAudioBuffer_005, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    uint8_t buffer[TEST_BUFFER_SIZE];
    size_t offset = TEST_BUFFER_SIZE / 2;
    for (size_t i = 0; i < TEST_BUFFER_SIZE; i++) {
        buffer[i] = 0xBB;
    }

    int32_t ret = AudioRenderer::MuteAudioBuffer(buffer, offset, TEST_BUFFER_SIZE - offset, SAMPLE_S16LE);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "MuteAudioBuffer with offset should succeed, ret=%d", ret);

    // First half should be untouched
    bool firstHalfUntouched = true;
    for (size_t i = 0; i < offset; i++) {
        if (buffer[i] != 0xBB) {
            firstHalfUntouched = false;
            break;
        }
    }
    AIUT_EXPECT_TRUE_INFO_LOG(firstHalfUntouched, "first half should be untouched");

    // Second half should be zeroed
    bool secondHalfZeroed = true;
    for (size_t i = offset; i < TEST_BUFFER_SIZE; i++) {
        if (buffer[i] != 0) {
            secondHalfZeroed = false;
            break;
        }
    }
    AIUT_EXPECT_TRUE_INFO_LOG(secondHalfZeroed, "second half should be zeroed");
}

// ==================== FadeInAudioBuffer Tests ====================

/**
 * @tc.name  : Test FadeInAudioBuffer with nullptr buffer
 * @tc.number: FadeInAudioBuffer_001
 * @tc.desc  : Test FadeInAudioBuffer returns ERR_INVALID_PARAM when buffer.buffer is nullptr.
 */
HWTEST_F(AudioRendererBaseAiut, FadeInAudioBuffer_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    BufferDesc bufferDesc = {nullptr, 0, 0};
    int32_t ret = AudioRenderer::FadeInAudioBuffer(bufferDesc, SAMPLE_S16LE, CHANNEL_2);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM,
        "nullptr buffer should return ERR_INVALID_PARAM, ret=%d", ret);
}

/**
 * @tc.name  : Test FadeInAudioBuffer with zero bufLength
 * @tc.number: FadeInAudioBuffer_002
 * @tc.desc  : Test FadeInAudioBuffer returns ERR_INVALID_PARAM when bufLength is 0.
 */
HWTEST_F(AudioRendererBaseAiut, FadeInAudioBuffer_002, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    uint8_t buffer[TEST_BUFFER_SIZE];
    BufferDesc bufferDesc = {buffer, 0, TEST_BUFFER_SIZE};
    int32_t ret = AudioRenderer::FadeInAudioBuffer(bufferDesc, SAMPLE_S16LE, CHANNEL_2);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM,
        "zero bufLength should return ERR_INVALID_PARAM, ret=%d", ret);
}

/**
 * @tc.name  : Test FadeInAudioBuffer with zero dataLength
 * @tc.number: FadeInAudioBuffer_003
 * @tc.desc  : Test FadeInAudioBuffer returns ERR_INVALID_PARAM when dataLength is 0.
 */
HWTEST_F(AudioRendererBaseAiut, FadeInAudioBuffer_003, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    uint8_t buffer[TEST_BUFFER_SIZE];
    BufferDesc bufferDesc = {buffer, TEST_BUFFER_SIZE, 0};
    int32_t ret = AudioRenderer::FadeInAudioBuffer(bufferDesc, SAMPLE_S16LE, CHANNEL_2);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM,
        "zero dataLength should return ERR_INVALID_PARAM, ret=%d", ret);
}

// ==================== FadeOutAudioBuffer Tests ====================

/**
 * @tc.name  : Test FadeOutAudioBuffer with nullptr buffer
 * @tc.number: FadeOutAudioBuffer_001
 * @tc.desc  : Test FadeOutAudioBuffer returns ERR_INVALID_PARAM when buffer.buffer is nullptr.
 */
HWTEST_F(AudioRendererBaseAiut, FadeOutAudioBuffer_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    BufferDesc bufferDesc = {nullptr, 0, 0};
    int32_t ret = AudioRenderer::FadeOutAudioBuffer(bufferDesc, SAMPLE_S16LE, CHANNEL_2);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM,
        "nullptr buffer should return ERR_INVALID_PARAM, ret=%d", ret);
}

/**
 * @tc.name  : Test FadeOutAudioBuffer with zero bufLength
 * @tc.number: FadeOutAudioBuffer_002
 * @tc.desc  : Test FadeOutAudioBuffer returns ERR_INVALID_PARAM when bufLength is 0.
 */
HWTEST_F(AudioRendererBaseAiut, FadeOutAudioBuffer_002, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    uint8_t buffer[TEST_BUFFER_SIZE];
    BufferDesc bufferDesc = {buffer, 0, TEST_BUFFER_SIZE};
    int32_t ret = AudioRenderer::FadeOutAudioBuffer(bufferDesc, SAMPLE_S16LE, CHANNEL_2);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM,
        "zero bufLength should return ERR_INVALID_PARAM, ret=%d", ret);
}

/**
 * @tc.name  : Test FadeOutAudioBuffer with zero dataLength
 * @tc.number: FadeOutAudioBuffer_003
 * @tc.desc  : Test FadeOutAudioBuffer returns ERR_INVALID_PARAM when dataLength is 0.
 */
HWTEST_F(AudioRendererBaseAiut, FadeOutAudioBuffer_003, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    uint8_t buffer[TEST_BUFFER_SIZE];
    BufferDesc bufferDesc = {buffer, TEST_BUFFER_SIZE, 0};
    int32_t ret = AudioRenderer::FadeOutAudioBuffer(bufferDesc, SAMPLE_S16LE, CHANNEL_2);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM,
        "zero dataLength should return ERR_INVALID_PARAM, ret=%d", ret);
}

// ==================== IsRendererFlagsSupportStatic Tests ====================

/**
 * @tc.name  : Test IsRendererFlagsSupportStatic with valid flags
 * @tc.number: IsRendererFlagsSupportStatic_001
 * @tc.desc  : Test IsRendererFlagsSupportStatic returns true for NORMAL, MMAP, FORCED_NORMAL flags.
 */
HWTEST_F(AudioRendererBaseAiut, IsRendererFlagsSupportStatic_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_EXPECT_TRUE_INFO_LOG(renderer->IsRendererFlagsSupportStatic(TEST_RENDERER_FLAGS_NORMAL),
        "NORMAL flags should be supported");
    AIUT_EXPECT_TRUE_INFO_LOG(renderer->IsRendererFlagsSupportStatic(TEST_RENDERER_FLAGS_MMAP),
        "MMAP flags should be supported");
    AIUT_EXPECT_TRUE_INFO_LOG(renderer->IsRendererFlagsSupportStatic(TEST_RENDERER_FLAGS_FORCED_NORMAL),
        "FORCED_NORMAL flags should be supported");
}

/**
 * @tc.name  : Test IsRendererFlagsSupportStatic with invalid flags
 * @tc.number: IsRendererFlagsSupportStatic_002
 * @tc.desc  : Test IsRendererFlagsSupportStatic returns false for unsupported flags.
 */
HWTEST_F(AudioRendererBaseAiut, IsRendererFlagsSupportStatic_002, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_EXPECT_FALSE_INFO_LOG(renderer->IsRendererFlagsSupportStatic(TEST_RENDERER_FLAGS_INVALID),
        "invalid flags should not be supported");
}

// ==================== HandleSetRendererInfoByOptions Tests ====================

/**
 * @tc.name  : Test HandleSetRendererInfoByOptions sets rendererInfo correctly
 * @tc.number: HandleSetRendererInfoByOptions_001
 * @tc.desc  : Test HandleSetRendererInfoByOptions copies fields from rendererOptions to rendererInfo_.
 */
HWTEST_F(AudioRendererBaseAiut, HandleSetRendererInfoByOptions_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AudioRendererOptions rendererOptions;
    rendererOptions.rendererInfo.contentType = CONTENT_TYPE_MUSIC;
    rendererOptions.rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    rendererOptions.rendererInfo.rendererFlags = TEST_RENDERER_FLAGS_NORMAL;
    rendererOptions.streamInfo.samplingRate = TEST_SAMPLE_RATE_44100;
    AppInfo appInfo;

    renderer->HandleSetRendererInfoByOptions(rendererOptions, appInfo);

    AIUT_EXPECT_EQ_INFO_LOG(renderer->rendererInfo_.contentType, CONTENT_TYPE_MUSIC,
        "contentType mismatch");
    AIUT_EXPECT_EQ_INFO_LOG(renderer->rendererInfo_.streamUsage, STREAM_USAGE_MUSIC,
        "streamUsage mismatch");
}

// ==================== AudioRendererPrivate Construction Tests ====================

/**
 * @tc.name  : Test AudioRendererPrivate constructor without stream creation
 * @tc.number: AudioRendererPrivate_Constructor_001
 * @tc.desc  : Test AudioRendererPrivate constructor with createStream=false creates proxy obj.
 */
HWTEST_F(AudioRendererBaseAiut, AudioRendererPrivate_Constructor_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    AppInfo appInfo;
    appInfo.appUid = TEST_APP_UID;
    appInfo.appPid = TEST_APP_PID;
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, appInfo, false);

    AIUT_EXPECT_NE_INFO_LOG(renderer->rendererProxyObj_, nullptr, "proxy obj should be created");
}

// ==================== GetStatus Tests ====================

/**
 * @tc.name  : Test GetStatus returns initial state
 * @tc.number: GetStatus_001
 * @tc.desc  : Test GetStatus returns RENDERER_PREPARED after construction without stream.
 */
HWTEST_F(AudioRendererBaseAiut, GetStatus_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    RendererState state = renderer->GetStatus();
    AIUT_EXPECT_EQ_INFO_LOG(state, RENDERER_PREPARED, "initial state should be PREPARED, state=%d", state);
}

// ==================== GetRendererDebugKey Tests ====================

/**
 * @tc.name  : Test GetRendererDebugKey returns initial value
 * @tc.number: GetRendererDebugKey_001
 * @tc.desc  : Test GetRendererDebugKey returns INVALID_DEBUGKEY (0) for newly created renderer.
 */
HWTEST_F(AudioRendererBaseAiut, GetRendererDebugKey_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    uint32_t key = renderer->GetRendererDebugKey();
    AIUT_EXPECT_EQ_INFO_LOG(key, INVALID_DEBUGKEY, "initial debug key should be 0, key=%u", key);
}

// ==================== IsNoStreamRenderer Tests ====================

/**
 * @tc.name  : Test IsNoStreamRenderer with null audioStream
 * @tc.number: IsNoStreamRenderer_001
 * @tc.desc  : Test IsNoStreamRenderer returns true when audioStream_ is null.
 */
HWTEST_F(AudioRendererBaseAiut, IsNoStreamRenderer_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    // With createStream=false, audioStream_ may be null
    bool isNoStream = renderer->IsNoStreamRenderer();
    AIUT_EXPECT_TRUE_INFO_LOG(isNoStream, "should be no-stream renderer when stream not created");
}

} // namespace AudioStandard
} // namespace OHOS
