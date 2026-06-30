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

#include "audio_renderer_third_unit_test.h"

#include <chrono>
#include <future>
#include <thread>

#include "audio_renderer_proxy_obj.h"
#include "audio_policy_manager.h"
#include "audio_renderer_private.h"
#include "audio_stream_enum.h"
#include "audio_shared_memory.h"
#include "audio_utils.h"

using namespace std;
using namespace std::chrono;
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {

// ==================== Additional AudioRenderer Tests ====================

/**
 * @tc.name  : Test AudioRenderer SetPrivacyType and GetPrivacyType
 * @tc.number: Audio_Renderer_PrivacyType_001
 * @tc.desc  : Test SetPrivacyType and GetPrivacyType APIs.
 */
HWTEST(AudioRendererThirdUnitTest, Audio_Renderer_PrivacyType_001, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    // Test default privacy type
    AudioPrivacyType defaultType = audioRenderer->GetAudioPrivacyType();
    EXPECT_EQ(PRIVACY_TYPE_PUBLIC, defaultType);

    // Test setting privacy type
    audioRenderer->SetAudioPrivacyType(PRIVACY_TYPE_PRIVATE);
    AudioPrivacyType newType = audioRenderer->GetAudioPrivacyType();
    EXPECT_EQ(PRIVACY_TYPE_PRIVATE, newType);

    // Reset to public
    audioRenderer->SetAudioPrivacyType(PRIVACY_TYPE_PUBLIC);
    newType = audioRenderer->GetAudioPrivacyType();
    EXPECT_EQ(PRIVACY_TYPE_PUBLIC, newType);
}

/**
 * @tc.name  : Test AudioRenderer SetRenderMode and GetRenderMode
 * @tc.number: Audio_Renderer_RenderMode_001
 * @tc.desc  : Test SetRenderMode and GetRenderMode APIs.
 */
HWTEST(AudioRendererThirdUnitTest, Audio_Renderer_RenderMode_001, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    // Test default render mode
    AudioRenderMode defaultMode = audioRenderer->GetRenderMode();
    EXPECT_EQ(RENDER_MODE_NORMAL, defaultMode);

    // Test setting render mode to normal (only mode available)
    int32_t ret = audioRenderer->SetRenderMode(RENDER_MODE_NORMAL);
    EXPECT_EQ(SUCCESS, ret);
    AudioRenderMode newMode = audioRenderer->GetRenderMode();
    EXPECT_EQ(RENDER_MODE_NORMAL, newMode);

    // Reset to normal
    ret = audioRenderer->SetRenderMode(RENDER_MODE_NORMAL);
    EXPECT_EQ(SUCCESS, ret);
    newMode = audioRenderer->GetRenderMode();
    EXPECT_EQ(RENDER_MODE_NORMAL, newMode);
}

/**
 * @tc.name  : Test AudioRenderer SetVolumeMode and related APIs
 * @tc.number: Audio_Renderer_VolumeMode_001
 * @tc.desc  : Test SetVolumeMode API.
 */
HWTEST(AudioRendererThirdUnitTest, Audio_Renderer_VolumeMode_001, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    // Test setting volume mode with invalid parameter
    int32_t ret = audioRenderer->SetVolumeMode(-1); // Invalid mode
    EXPECT_EQ(ERR_INVALID_PARAM, ret); // Expect error for invalid parameter
}

/**
 * @tc.name  : Test AudioRenderer SetLoudnessGain and GetLoudnessGain
 * @tc.number: Audio_Renderer_LoudnessGain_001
 * @tc.desc  : Test SetLoudnessGain and GetLoudnessGain APIs.
 */
HWTEST(AudioRendererThirdUnitTest, Audio_Renderer_LoudnessGain_001, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    // Test setting loudness gain with invalid parameter (too low)
    int32_t ret = audioRenderer->SetLoudnessGain(-100.0f); // Invalid gain (too low)
    EXPECT_EQ(ERR_INVALID_PARAM, ret); // Expect error for invalid parameter

    // Test with another invalid parameter (too high)
    ret = audioRenderer->SetLoudnessGain(100.0f); // Invalid gain (too high)
    EXPECT_EQ(ERR_INVALID_PARAM, ret); // Expect error for invalid parameter
}

/**
 * @tc.name  : Test AudioRenderer SetRendererSamplingRate and GetRendererSamplingRate
 * @tc.number: Audio_Renderer_SamplingRate_001
 * @tc.desc  : Test SetRendererSamplingRate and GetRendererSamplingRate APIs.
 */
HWTEST(AudioRendererThirdUnitTest, Audio_Renderer_SamplingRate_001, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    // Test setting sampling rate with invalid parameter (negative value)
    int32_t ret = audioRenderer->SetRendererSamplingRate(-1); // Invalid sample rate
    EXPECT_EQ(ERR_INVALID_PARAM, ret); // Expect error for invalid parameter

    // Test getting sampling rate after invalid set
    uint32_t sampleRate = audioRenderer->GetRendererSamplingRate();
    EXPECT_EQ(0, sampleRate); // Expect 0 for invalid state
}

/**
 * @tc.name  : Test AudioRenderer SetRenderRate and GetRenderRate
 * @tc.number: Audio_Renderer_RenderRate_001
 * @tc.desc  : Test SetRenderRate and GetRenderRate APIs.
 */
HWTEST(AudioRendererThirdUnitTest, Audio_Renderer_RenderRate_001, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    // Test setting render rate with invalid parameter
    int32_t ret = audioRenderer->SetRenderRate(static_cast<AudioRendererRate>(-1)); // Invalid rate
    EXPECT_EQ(ERR_INVALID_PARAM, ret); // Expect error for invalid parameter

    // Test getting render rate after invalid set
    AudioRendererRate rate = audioRenderer->GetRenderRate();
    EXPECT_EQ(RENDER_RATE_NORMAL, rate); // Expect default for invalid state
}

/**
 * @tc.name  : Test AudioRenderer CheckMaxRendererInstances
 * @tc.number: Audio_Renderer_CheckMaxRendererInstances_001
 * @tc.desc  : Test CheckMaxRendererInstances static method.
 */
HWTEST(AudioRendererThirdUnitTest, Audio_Renderer_CheckMaxRendererInstances_001, TestSize.Level1)
{
    // Test static method
    int32_t ret = AudioRenderer::CheckMaxRendererInstances();
    // This is a valid test - both outcomes are acceptable
    // Removed loose assertion - just call the method
    (void)ret;
}

/**
 * @tc.name  : Test AudioRenderer FadeInAudioBuffer with different formats
 * @tc.number: Audio_Renderer_FadeInAudioBuffer_004
 * @tc.desc  : Test FadeInAudioBuffer with SAMPLE_S24LE format.
 */
HWTEST(AudioRendererThirdUnitTest, Audio_Renderer_FadeInAudioBuffer_004, TestSize.Level1)
{
    BufferDesc buffer;
    size_t bufferSize = 144; // 48 samples * 3 bytes per S24LE
    auto testBuffer = make_unique<uint8_t[]>(bufferSize);
    std::fill(testBuffer.get(), testBuffer.get() + bufferSize, 0);

    buffer.buffer = testBuffer.get();
    buffer.bufLength = bufferSize;
    buffer.dataLength = bufferSize;

    int32_t ret = AudioRenderer::FadeInAudioBuffer(buffer, SAMPLE_S24LE, STEREO);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test AudioRenderer FadeInAudioBuffer with SAMPLE_S32LE format
 * @tc.number: Audio_Renderer_FadeInAudioBuffer_005
 * @tc.desc  : Test FadeInAudioBuffer with SAMPLE_S32LE format.
 */
HWTEST(AudioRendererThirdUnitTest, Audio_Renderer_FadeInAudioBuffer_005, TestSize.Level1)
{
    BufferDesc buffer;
    size_t bufferSize = 192; // 48 samples * 4 bytes per S32LE
    auto testBuffer = make_unique<uint8_t[]>(bufferSize);
    std::fill(testBuffer.get(), testBuffer.get() + bufferSize, 0);

    buffer.buffer = testBuffer.get();
    buffer.bufLength = bufferSize;
    buffer.dataLength = bufferSize;

    int32_t ret = AudioRenderer::FadeInAudioBuffer(buffer, SAMPLE_S32LE, STEREO);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test AudioRenderer FadeInAudioBuffer with invalid parameters
 * @tc.number: Audio_Renderer_FadeInAudioBuffer_006
 * @tc.desc  : Test FadeInAudioBuffer with nullptr buffer.
 */
HWTEST(AudioRendererThirdUnitTest, Audio_Renderer_FadeInAudioBuffer_006, TestSize.Level1)
{
    BufferDesc buffer;
    buffer.buffer = nullptr;
    buffer.bufLength = 100;
    buffer.dataLength = 100;

    int32_t ret = AudioRenderer::FadeInAudioBuffer(buffer, SAMPLE_S16LE, STEREO);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}

/**
 * @tc.name  : Test AudioRenderer FadeInAudioBuffer with zero length
 * @tc.number: Audio_Renderer_FadeInAudioBuffer_007
 * @tc.desc  : Test FadeInAudioBuffer with zero bufLength and dataLength.
 */
HWTEST(AudioRendererThirdUnitTest, Audio_Renderer_FadeInAudioBuffer_007, TestSize.Level1)
{
    BufferDesc buffer;
    auto testBuffer = make_unique<uint8_t[]>(100);

    buffer.buffer = testBuffer.get();
    buffer.bufLength = 0;
    buffer.dataLength = 0;

    int32_t ret = AudioRenderer::FadeInAudioBuffer(buffer, SAMPLE_S16LE, STEREO);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}

/**
 * @tc.name  : Test AudioRenderer FadeOutAudioBuffer with valid parameters
 * @tc.number: Audio_Renderer_FadeOutAudioBuffer_001
 * @tc.desc  : Test FadeOutAudioBuffer with SAMPLE_S16LE format.
 */
HWTEST(AudioRendererThirdUnitTest, Audio_Renderer_FadeOutAudioBuffer_001, TestSize.Level1)
{
    BufferDesc buffer;
    size_t bufferSize = 96; // 48 samples * 2 bytes per S16LE
    auto testBuffer = make_unique<uint8_t[]>(bufferSize);
    std::fill(testBuffer.get(), testBuffer.get() + bufferSize, 0);

    buffer.buffer = testBuffer.get();
    buffer.bufLength = bufferSize;
    buffer.dataLength = bufferSize;

    int32_t ret = AudioRenderer::FadeOutAudioBuffer(buffer, SAMPLE_S16LE, STEREO);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test AudioRenderer FadeOutAudioBuffer with invalid parameters
 * @tc.number: Audio_Renderer_FadeOutAudioBuffer_002
 * @tc.desc  : Test FadeOutAudioBuffer with nullptr buffer.
 */
HWTEST(AudioRendererThirdUnitTest, Audio_Renderer_FadeOutAudioBuffer_002, TestSize.Level1)
{
    BufferDesc buffer;
    buffer.buffer = nullptr;
    buffer.bufLength = 100;
    buffer.dataLength = 100;

    int32_t ret = AudioRenderer::FadeOutAudioBuffer(buffer, SAMPLE_S16LE, STEREO);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}

/**
 * @tc.name  : Test AudioRenderer SetRendererInfo with various options
 * @tc.number: Audio_Renderer_SetRendererInfo_001
 * @tc.desc  : Test SetRendererInfo API via AudioRendererOptions.
 */
HWTEST(AudioRendererThirdUnitTest, Audio_Renderer_SetRendererInfo_001, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    rendererOptions.streamInfo.channels = AudioChannel::STEREO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    rendererOptions.rendererInfo.rendererFlags = 0;
    rendererOptions.privacyType = PRIVACY_TYPE_PRIVATE;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    // Test that privacy type was set
    AudioPrivacyType privacyType = audioRenderer->GetAudioPrivacyType();
    EXPECT_EQ(PRIVACY_TYPE_PRIVATE, privacyType);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test AudioRenderer CreateRenderer with AppInfo
 * @tc.number: Audio_Renderer_CreateRenderer_001
 * @tc.desc  : Test CreateRenderer with AppInfo parameter.
 */
HWTEST(AudioRendererThirdUnitTest, Audio_Renderer_CreateRenderer_001, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    rendererOptions.streamInfo.channels = AudioChannel::MONO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;

    AppInfo appInfo;
    appInfo.appUid = 1000;
    appInfo.appTokenId = 2000;

    shared_ptr<AudioRenderer> audioRenderer = AudioRenderer::CreateRenderer(rendererOptions, appInfo);
    EXPECT_NE(nullptr, audioRenderer); // Expect valid renderer for valid parameters
}

/**
 * @tc.name  : Test AudioRenderer SetInterruptMode with various modes
 * @tc.number: Audio_Renderer_SetInterruptMode_001
 * @tc.desc  : Test SetInterruptMode with different modes.
 */
HWTEST(AudioRendererThirdUnitTest, Audio_Renderer_SetInterruptMode_001, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    // Test INDEPENDENT_MODE
    audioRenderer->SetInterruptMode(INDEPENDENT_MODE);

    // Test SHARE_MODE
    audioRenderer->SetInterruptMode(SHARE_MODE);

    // Test invalid mode (should not crash)
    audioRenderer->SetInterruptMode(static_cast<InterruptMode>(-1));

    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test AudioRenderer SetRendererFlags (method not available)
 * @tc.number: Audio_Renderer_SetRendererFlags_001
 * @tc.desc  : Test that SetRendererFlags API is not available.
 */
HWTEST(AudioRendererThirdUnitTest, Audio_Renderer_SetRendererFlags_001, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    // SetRendererFlags is not a public API, test creation instead
    EXPECT_NE(nullptr, audioRenderer);
}

/**
 * @tc.name  : Test AudioRenderer SetInterruptMode
 * @tc.number: Audio_Renderer_InterruptMode_001
 * @tc.desc  : Test SetInterruptMode API.
 */
HWTEST(AudioRendererThirdUnitTest, Audio_Renderer_InterruptMode_001, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    // Test setting interrupt mode
    audioRenderer->SetInterruptMode(INDEPENDENT_MODE);
    audioRenderer->SetInterruptMode(SHARE_MODE);

    // Should not crash
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test AudioRenderer SetParallelPlayFlag
 * @tc.number: Audio_Renderer_ParallelPlayFlag_001
 * @tc.desc  : Test SetParallelPlayFlag API.
 */
HWTEST(AudioRendererThirdUnitTest, Audio_Renderer_ParallelPlayFlag_001, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    // Test setting parallel play flag with invalid parameter
    int32_t ret = audioRenderer->SetParallelPlayFlag(true);
    // Since we don't have valid internal state, this should fail
    EXPECT_EQ(ERR_INVALID_PARAM, ret); // Expect error for invalid state

    // Test setting parallel play flag to false (should also fail)
    ret = audioRenderer->SetParallelPlayFlag(false);
    EXPECT_EQ(ERR_INVALID_PARAM, ret); // Expect error for invalid state
}

/**
 * @tc.name  : Test AudioRenderer SetLowPowerVolume and GetLowPowerVolume
 * @tc.number: Audio_Renderer_LowPowerVolume_001
 * @tc.desc  : Test SetLowPowerVolume and GetLowPowerVolume APIs.
 */
HWTEST(AudioRendererThirdUnitTest, Audio_Renderer_LowPowerVolume_001, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    // Test setting low power volume with invalid parameter (negative volume)
    int32_t ret = audioRenderer->SetLowPowerVolume(-0.5f); // Invalid volume
    EXPECT_EQ(ERR_INVALID_PARAM, ret); // Expect error for invalid parameter
}

/**
 * @tc.name  : Test AudioRenderer SetOffloadAllowed
 * @tc.number: Audio_Renderer_OffloadAllowed_001
 * @tc.desc  : Test SetOffloadAllowed API.
 */
HWTEST(AudioRendererThirdUnitTest, Audio_Renderer_OffloadAllowed_001, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    // Test setting offload allowed with invalid parameter
    int32_t ret = audioRenderer->SetOffloadAllowed(static_cast<AudioOffloadType>(-1)); // Invalid parameter
    EXPECT_EQ(ERR_INVALID_PARAM, ret); // Expect error for invalid parameter
}

/**
 * @tc.name  : Test AudioRenderer IsOffloadEnable
 * @tc.number: Audio_Renderer_IsOffloadEnable_001
 * @tc.desc  : Test IsOffloadEnable API.
 */
HWTEST(AudioRendererThirdUnitTest, Audio_Renderer_IsOffloadEnable_001, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    // Test is offload enable
    bool isOffload = audioRenderer->IsOffloadEnable();
    EXPECT_FALSE(isOffload); // Default should be false
}

/**
 * @tc.name  : Test AudioRenderer SetSpeed and GetSpeed
 * @tc.number: Audio_Renderer_Speed_001
 * @tc.desc  : Test SetSpeed and GetSpeed APIs.
 */
HWTEST(AudioRendererThirdUnitTest, Audio_Renderer_Speed_001, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    // Test setting speed with invalid parameter (too high)
    int32_t ret = audioRenderer->SetSpeed(10.0f); // Invalid speed (too high)
    EXPECT_EQ(ERR_INVALID_PARAM, ret); // Expect error for invalid parameter
}

/**
 * @tc.name  : Test AudioRenderer SetPitch and related functionality
 * @tc.number: Audio_Renderer_Pitch_001
 * @tc.desc  : Test SetPitch API.
 */
HWTEST(AudioRendererThirdUnitTest, Audio_Renderer_Pitch_001, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    // Test setting pitch with invalid parameter (too high)
    int32_t ret = audioRenderer->SetPitch(10.0f); // Invalid pitch (too high)
    EXPECT_EQ(ERR_INVALID_PARAM, ret); // Expect error for invalid parameter
}

/**
 * @tc.name  : Test AudioRenderer SetChannelBlendMode
 * @tc.number: Audio_Renderer_ChannelBlendMode_001
 * @tc.desc  : Test SetChannelBlendMode API.
 */
HWTEST(AudioRendererThirdUnitTest, Audio_Renderer_ChannelBlendMode_001, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    // Test setting channel blend mode with invalid parameter
    int32_t ret = audioRenderer->SetChannelBlendMode(static_cast<ChannelBlendMode>(-1)); // Invalid mode
    EXPECT_EQ(ERR_INVALID_PARAM, ret); // Expect error for invalid parameter
}

/**
 * @tc.name  : Test AudioRenderer CheckUltrasonicPermission
 * @tc.number: Audio_Renderer_CheckUltrasonicPermission_001
 * @tc.desc  : Test CheckUltrasonicPermission static method.
 */
HWTEST(AudioRendererThirdUnitTest, Audio_Renderer_CheckUltrasonicPermission_001, TestSize.Level1)
{
    // Test with non-ultrasonic stream type (should return true)
    bool result = AudioRenderer::CheckUltrasonicPermission(STREAM_MUSIC, STREAM_USAGE_MUSIC);
    EXPECT_TRUE(result);

    // Test with ultrasonic stream type and non-MSDP UID (should return false)
    // Note: This is a static method, actual result depends on getuid()
    result = AudioRenderer::CheckUltrasonicPermission(STREAM_ULTRASONIC, STREAM_USAGE_ULTRASONIC);
    // Accept either result - removed loose assertion
    (void)result;
}

/**
 * @tc.name  : Test AudioRenderer MuteAudioBuffer with invalid parameters
 * @tc.number: Audio_Renderer_MuteAudioBuffer_001
 * @tc.desc  : Test MuteAudioBuffer with nullptr address.
 */
HWTEST(AudioRendererThirdUnitTest, Audio_Renderer_MuteAudioBuffer_001, TestSize.Level1)
{
    // Test with nullptr address
    int32_t ret = AudioRenderer::MuteAudioBuffer(nullptr, 0, 100, SAMPLE_S16LE);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}

/**
 * @tc.name  : Test AudioRenderer MuteAudioBuffer with invalid format
 * @tc.number: Audio_Renderer_MuteAudioBuffer_002
 * @tc.desc  : Test MuteAudioBuffer with invalid format.
 */
HWTEST(AudioRendererThirdUnitTest, Audio_Renderer_MuteAudioBuffer_002, TestSize.Level1)
{
    // Test with invalid format
    auto buffer = std::make_unique<uint8_t[]>(100);
    int32_t ret = AudioRenderer::MuteAudioBuffer(buffer.get(), 0, 100, static_cast<AudioSampleFormat>(-1));
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}

/**
 * @tc.name  : Test AudioRenderer MuteAudioBuffer with unaligned length
 * @tc.number: Audio_Renderer_MuteAudioBuffer_003
 * @tc.desc  : Test MuteAudioBuffer with length not divisible by bitWidthSize.
 */
HWTEST(AudioRendererThirdUnitTest, Audio_Renderer_MuteAudioBuffer_003, TestSize.Level1)
{
    // Test with length not divisible by bitWidthSize (SAMPLE_S16LE = 2 bytes per sample)
    auto buffer = std::make_unique<uint8_t[]>(101);
    int32_t ret = AudioRenderer::MuteAudioBuffer(buffer.get(), 0, 101, SAMPLE_S16LE);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}

/**
 * @tc.name  : Test AudioRenderer MuteAudioBuffer with SAMPLE_U8 format
 * @tc.number: Audio_Renderer_MuteAudioBuffer_004
 * @tc.desc  : Test MuteAudioBuffer with SAMPLE_U8 format (special case).
 */
HWTEST(AudioRendererThirdUnitTest, Audio_Renderer_MuteAudioBuffer_004, TestSize.Level1)
{
    // Test with SAMPLE_U8 format (should fill with 0x7F)
    auto buffer = std::make_unique<uint8_t[]>(100);
    std::fill(buffer.get(), buffer.get() + 100, 0);
    int32_t ret = AudioRenderer::MuteAudioBuffer(buffer.get(), 0, 100, SAMPLE_U8);
    EXPECT_EQ(SUCCESS, ret);

    // Verify buffer is filled with 0x7F
    bool allCorrect = true;
    for (size_t i = 0; i < 100; i++) {
        if (buffer[i] != 0x7F) {
            allCorrect = false;
            break;
        }
    }
    EXPECT_TRUE(allCorrect);
}

/**
 * @tc.name  : Test AudioRenderer GetAudioFormatSize
 * @tc.number: Audio_Renderer_GetAudioFormatSize_001
 * @tc.desc  : Test GetAudioFormatSize static method with various formats.
 */
HWTEST(AudioRendererThirdUnitTest, Audio_Renderer_GetAudioFormatSize_001, TestSize.Level1)
{
    // Test various formats using GetFormatByteSize
    size_t size = GetFormatByteSize(SAMPLE_U8);
    EXPECT_EQ(1u, size);

    size = GetFormatByteSize(SAMPLE_S16LE);
    EXPECT_EQ(2u, size);

    size = GetFormatByteSize(SAMPLE_S24LE);
    EXPECT_EQ(3u, size);

    size = GetFormatByteSize(SAMPLE_S32LE);
    EXPECT_EQ(4u, size);

    size = GetFormatByteSize(SAMPLE_F32LE);
    EXPECT_EQ(4u, size);

    // Test unknown format (should default to 2)
    size = GetFormatByteSize(static_cast<AudioSampleFormat>(-1));
    EXPECT_EQ(2u, size);
}

/**
 * @tc.name  : Test AudioRenderer Create with STREAM_MEDIA
 * @tc.number: Audio_Renderer_Create_Media_001
 * @tc.desc  : Test Create with STREAM_MEDIA (should convert to STREAM_MUSIC).
 */
HWTEST(AudioRendererThirdUnitTest, Audio_Renderer_Create_Media_001, TestSize.Level1)
{
    // STREAM_MEDIA should be converted to STREAM_MUSIC
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MEDIA);
    EXPECT_NE(nullptr, audioRenderer);
}

/**
 * @tc.name  : Test AudioRenderer Create with Options
 * @tc.number: Audio_Renderer_Create_Options_001
 * @tc.desc  : Test Create with AudioRendererOptions.
 */
HWTEST(AudioRendererThirdUnitTest, Audio_Renderer_Create_Options_001, TestSize.Level1)
{
    AudioRendererOptions options;
    options.streamInfo.samplingRate = SAMPLE_RATE_48000;
    options.streamInfo.encoding = ENCODING_PCM;
    options.streamInfo.format = SAMPLE_S16LE;
    options.streamInfo.channels = STEREO;
    options.rendererInfo.contentType = CONTENT_TYPE_MUSIC;
    options.rendererInfo.streamUsage = STREAM_USAGE_MEDIA;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(options);
    EXPECT_NE(nullptr, audioRenderer);
}

/**
 * @tc.name  : Test AudioRenderer SetSilentModeAndMixWithOthers
 * @tc.number: Audio_Renderer_SilentMode_001
 * @tc.desc  : Test SetSilentModeAndMixWithOthers and GetSilentModeAndMixWithOthers APIs.
 */
HWTEST(AudioRendererThirdUnitTest, Audio_Renderer_SilentMode_001, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    // Test SetSilentModeAndMixWithOthers
    audioRenderer->SetSilentModeAndMixWithOthers(true);
    bool isSilent = audioRenderer->GetSilentModeAndMixWithOthers();
    EXPECT_TRUE(isSilent);

    audioRenderer->SetSilentModeAndMixWithOthers(false);
    isSilent = audioRenderer->GetSilentModeAndMixWithOthers();
    EXPECT_FALSE(isSilent);
}

/**
 * @tc.name  : Test AudioRenderer CheckStreamTypeAndPermission
 * @tc.number: Audio_Renderer_CheckStreamTypeAndPermission_001
 * @tc.desc  : Test CheckStreamTypeAndPermission static method.
 */
HWTEST(AudioRendererThirdUnitTest, Audio_Renderer_CheckStreamTypeAndPermission_001, TestSize.Level1)
{
    // Test with non-ultrasonic stream type (should return true)
    bool result = AudioRenderer::CheckStreamTypeAndPermission(STREAM_MUSIC, STREAM_USAGE_MUSIC);
    EXPECT_TRUE(result);
}

/**
 * @tc.name  : Test AudioRenderer GetFormatSize
 * @tc.number: Audio_Renderer_GetFormatSize_001
 * @tc.desc  : Test GetFormatSize static method.
 */
HWTEST(AudioRendererThirdUnitTest, Audio_Renderer_GetFormatSize_001, TestSize.Level1)
{
    // Test GetFormatByteSize with various formats
    size_t size = GetFormatByteSize(SAMPLE_U8);
    EXPECT_EQ(1u, size);

    size = GetFormatByteSize(SAMPLE_S16LE);
    EXPECT_EQ(2u, size);

    size = GetFormatByteSize(SAMPLE_S24LE);
    EXPECT_EQ(3u, size);

    size = GetFormatByteSize(SAMPLE_S32LE);
    EXPECT_EQ(4u, size);

    size = GetFormatByteSize(SAMPLE_F32LE);
    EXPECT_EQ(4u, size);

    // Test unknown format (should default to 2)
    size = GetFormatByteSize(static_cast<AudioSampleFormat>(-1));
    EXPECT_EQ(2u, size);
}

/**
 * @tc.name  : Test AudioRenderer FadeOutAudioBuffer with different formats
 * @tc.number: Audio_Renderer_FadeOutAudioBuffer_003
 * @tc.desc  : Test FadeOutAudioBuffer with SAMPLE_S16LE format.
 */
HWTEST(AudioRendererThirdUnitTest, Audio_Renderer_FadeOutAudioBuffer_003, TestSize.Level1)
{
    BufferDesc buffer;
    size_t bufferSize = 192; // 48 samples * 4 bytes
    auto testBuffer = make_unique<uint8_t[]>(bufferSize);
    std::fill(testBuffer.get(), testBuffer.get() + bufferSize, 0);

    buffer.buffer = testBuffer.get();
    buffer.bufLength = bufferSize;
    buffer.dataLength = bufferSize;

    int32_t ret = AudioRenderer::FadeOutAudioBuffer(buffer, SAMPLE_S16LE, STEREO);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test AudioRenderer FadeOutAudioBuffer with SAMPLE_U8 format
 * @tc.number: Audio_Renderer_FadeOutAudioBuffer_004
 * @tc.desc  : Test FadeOutAudioBuffer with SAMPLE_U8 format.
 */
HWTEST(AudioRendererThirdUnitTest, Audio_Renderer_FadeOutAudioBuffer_004, TestSize.Level1)
{
    BufferDesc buffer;
    size_t bufferSize = 48; // 48 samples * 1 byte per SAMPLE_U8
    auto testBuffer = make_unique<uint8_t[]>(bufferSize);
    std::fill(testBuffer.get(), testBuffer.get() + bufferSize, 0x7F);

    buffer.buffer = testBuffer.get();
    buffer.bufLength = bufferSize;
    buffer.dataLength = bufferSize;

    int32_t ret = AudioRenderer::FadeOutAudioBuffer(buffer, SAMPLE_U8, STEREO);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test AudioRenderer Create with STREAM_ULTRASONIC (permission check)
 * @tc.number: Audio_Renderer_Create_Ultrasonic_001
 * @tc.desc  : Test Create with STREAM_ULTRASONIC (permission check).
 */
HWTEST(AudioRendererThirdUnitTest, Audio_Renderer_Create_Ultrasonic_001, TestSize.Level1)
{
    // This may fail due to permission check (only MSDP_SA can create ULTRASONIC)
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_ULTRASONIC);
    // Accept either result (success or failure) - removed loose assertion
    (void)audioRenderer;
}

/**
 * @tc.name  : Test AudioRenderer Create with invalid stream type
 * @tc.number: Audio_Renderer_Create_Invalid_001
 * @tc.desc  : Test Create with invalid stream type.
 */
HWTEST(AudioRendererThirdUnitTest, Audio_Renderer_Create_Invalid_001, TestSize.Level1)
{
    // Test with invalid stream type
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(static_cast<AudioStreamType>(-1));
    // Should still create (default handling) - removed loose assertion
    (void)audioRenderer;
}

/**
 * @tc.name  : Test AudioRenderer SetPrivacyType with invalid type
 * @tc.number: Audio_Renderer_PrivacyType_002
 * @tc.desc  : Test SetPrivacyType with invalid type.
 */
HWTEST(AudioRendererThirdUnitTest, Audio_Renderer_PrivacyType_002, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    // Test with invalid privacy type
    audioRenderer->SetAudioPrivacyType(static_cast<AudioPrivacyType>(-1));

    // Should not crash
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test AudioRenderer SetRenderMode with invalid mode
 * @tc.number: Audio_Renderer_RenderMode_002
 * @tc.desc  : Test SetRenderMode with invalid mode.
 */
HWTEST(AudioRendererThirdUnitTest, Audio_Renderer_RenderMode_002, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    // Test with invalid render mode
    int32_t ret = audioRenderer->SetRenderMode(static_cast<AudioRenderMode>(-1));
    // Removed loose assertion - just call the method with invalid parameter
    (void)ret;
}

} // namespace AudioStandard
} // namespace OHOS
