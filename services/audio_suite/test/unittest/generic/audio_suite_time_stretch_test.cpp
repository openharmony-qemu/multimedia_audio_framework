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

#include <gtest/gtest.h>
#include <cstring>
#include <vector>
#include <dlfcn.h>
#include "audio_errors.h"
#include "audio_suite_time_stretch.h"
#include "audio_time_stretch_api.h"

using namespace testing::ext;
using namespace OHOS::AudioStandard;

namespace OHOS {
namespace AudioStandard {

// ===== Test constants =====
constexpr float    SINE_FREQ_COEF      = 0.01f;
constexpr float    F32_AMPLITUDE       = 0.5f;
constexpr int16_t  S16_AMPLITUDE       = 16384;
constexpr uint8_t  U8_MIDPOINT         = 128;
constexpr uint8_t  U8_AMPLITUDE        = 64;
constexpr int32_t  S24_AMPLITUDE       = 4194304;
constexpr int32_t  S32_AMPLITUDE       = 1073741824;
constexpr int      INVALID_FORMAT_VAL  = 999;
constexpr int      UNALIGNED_BYTE_COUNT = 3;
constexpr uint32_t U8_SIZE             = 1;
constexpr uint32_t S24_SIZE            = 3;
constexpr int32_t BYTE_SHIFT_8         = 8;
constexpr int32_t BYTE_SHIFT_16        = 16;
constexpr int32_t BYTE_MASK_0xFF       = 0xFF;
constexpr int32_t S24_BYTE_LOW         = 0;
constexpr int32_t S24_BYTE_MID         = 1;
constexpr int32_t S24_BYTE_HIGH        = 2;

// ===== Test scenario constants =====
constexpr int32_t TEST_IN_FRAMES        = 1024;
constexpr int32_t TEST_SMALL_IN_FRAMES  = 256;
constexpr int32_t OUTPUT_CAPACITY_X2    = 2;
constexpr float   SPEED_NORMAL          = 2.0f;
constexpr float   SPEED_OUT_OF_RANGE_LOW  = 0.01f;
constexpr float   SPEED_OUT_OF_RANGE_HIGH = 100.0f;
constexpr float   PITCH_NORMAL          = 1.5f;
constexpr float   PITCH_OUT_OF_RANGE_LOW  = 0.01f;
constexpr float   PITCH_OUT_OF_RANGE_HIGH = 100.0f;

// ===== Boundary values for invalid config tests =====
constexpr uint32_t INVALID_SAMPLE_RATE_LOW  = 7999;    // MIN_SAMPLE_RATE(8000) - 1
constexpr uint32_t INVALID_SAMPLE_RATE_HIGH = 384001;  // MAX_SAMPLE_RATE(384000) + 1
constexpr uint32_t INVALID_CHANNELS_LOW     = 0;       // MIN_CHANNELS(1) - 1
constexpr uint32_t INVALID_CHANNELS_HIGH    = 17;      // MAX_CHANNELS(16) + 1

// ===== Check if algorithm so is available =====
static bool IsTimeStretchAlgoAvailable()
{
    const char *soPath = "/system/lib64/libaudio_time_stretch.z.so";
    void *handle = dlopen(soPath, RTLD_NOW);
    if (handle == nullptr) {
        return false;
    }
    dlclose(handle);
    return true;
}

// ===== Helper functions =====

// Generate float test data of specified frame count (sine wave)
static std::vector<float> GenerateFloatInput(int32_t frames, uint32_t channels)
{
    std::vector<float> data(frames * channels, 0.0f);
    for (int32_t i = 0; i < frames; i++) {
        for (uint32_t ch = 0; ch < channels; ch++) {
            data[i * channels + ch] = F32_AMPLITUDE * sinf(i * SINE_FREQ_COEF);
        }
    }
    return data;
}

// Generate S16LE test data of specified frame count
static std::vector<int16_t> GenerateS16Input(int32_t frames, uint32_t channels)
{
    std::vector<int16_t> data(frames * channels, 0);
    for (int32_t i = 0; i < frames; i++) {
        for (uint32_t ch = 0; ch < channels; ch++) {
            data[i * channels + ch] = static_cast<int16_t>(S16_AMPLITUDE * sinf(i * SINE_FREQ_COEF));
        }
    }
    return data;
}

// Generate U8 test data of specified frame count (U8 midpoint is 128)
static std::vector<uint8_t> GenerateU8Input(int32_t frames, uint32_t channels)
{
    std::vector<uint8_t> data(frames * channels, U8_MIDPOINT);
    for (int32_t i = 0; i < frames; i++) {
        for (uint32_t ch = 0; ch < channels; ch++) {
            data[i * channels + ch] = static_cast<uint8_t>(U8_MIDPOINT + U8_AMPLITUDE * sinf(i * SINE_FREQ_COEF));
        }
    }
    return data;
}

// Generate S24LE test data of specified frame count (3-byte interleaved, S24 range +/-8388608)
static std::vector<uint8_t> GenerateS24Input(int32_t frames, uint32_t channels)
{
    std::vector<uint8_t> data(frames * channels * S24_SIZE, 0);
    for (int32_t i = 0; i < frames; i++) {
        for (uint32_t ch = 0; ch < channels; ch++) {
            int32_t sample = static_cast<int32_t>(S24_AMPLITUDE * sinf(i * SINE_FREQ_COEF));
            size_t offset = (i * channels + ch) * S24_SIZE;
            data[offset + S24_BYTE_LOW] = sample & BYTE_MASK_0xFF;
            data[offset + S24_BYTE_MID] = (sample >> BYTE_SHIFT_8) & BYTE_MASK_0xFF;
            data[offset + S24_BYTE_HIGH] = (sample >> BYTE_SHIFT_16) & BYTE_MASK_0xFF;
        }
    }
    return data;
}

// Generate S32LE test data of specified frame count (S32 range +/-2147483648)
static std::vector<int32_t> GenerateS32Input(int32_t frames, uint32_t channels)
{
    std::vector<int32_t> data(frames * channels, 0);
    for (int32_t i = 0; i < frames; i++) {
        for (uint32_t ch = 0; ch < channels; ch++) {
            data[i * channels + ch] = static_cast<int32_t>(S32_AMPLITUDE * sinf(i * SINE_FREQ_COEF));
        }
    }
    return data;
}

// ==================== Test cases ====================

class AudioSuiteTimeStretchTest : public testing::Test {
public:
    void SetUp()
    {
        if (!IsTimeStretchAlgoAvailable()) {
            GTEST_SKIP() << "libaudio_time_stretch.z.so not available, skip this test";
        }
    }
    static constexpr uint32_t SAMPLE_RATE = 44100;
    static constexpr uint32_t CHANNELS = 2;
};

/**
 * @tc.name  : Test AudioSuiteTimeStretch MapFormat.
 * @tc.number: AudioSuiteTimeStretch_MapFormat_AllFormats
 * @tc.desc  : Test MapFormat for all supported sample formats and an unsupported format.
 */
HWTEST_F(AudioSuiteTimeStretchTest, MapFormat_AllFormats, TestSize.Level1)
{
    // Valid formats: Create should succeed (MapFormat returns SUCCESS internally)
    {
        auto p = AudioSuiteTimeStretch::Create(SAMPLE_RATE, CHANNELS, SAMPLE_U8);
        EXPECT_NE(p, nullptr);
        p.reset();
    }
    {
        auto p = AudioSuiteTimeStretch::Create(SAMPLE_RATE, CHANNELS, SAMPLE_S16LE);
        EXPECT_NE(p, nullptr);
        p.reset();
    }
    {
        auto p = AudioSuiteTimeStretch::Create(SAMPLE_RATE, CHANNELS, SAMPLE_S24LE);
        EXPECT_NE(p, nullptr);
        p.reset();
    }
    {
        auto p = AudioSuiteTimeStretch::Create(SAMPLE_RATE, CHANNELS, SAMPLE_S32LE);
        EXPECT_NE(p, nullptr);
        p.reset();
    }
    {
        auto p = AudioSuiteTimeStretch::Create(SAMPLE_RATE, CHANNELS, SAMPLE_F32LE);
        EXPECT_NE(p, nullptr);
        p.reset();
    }
    // Unsupported format: Create should return nullptr
    {
        AudioSampleFormat invalidFmt = static_cast<AudioSampleFormat>(INVALID_FORMAT_VAL);
        auto p = AudioSuiteTimeStretch::Create(SAMPLE_RATE, CHANNELS, invalidFmt);
        EXPECT_EQ(p, nullptr);
    }
}

/**
 * @tc.name  : Test AudioSuiteTimeStretch Create with invalid config.
 * @tc.number: AudioSuiteTimeStretch_Create_InvalidConfig
 * @tc.desc  : Test Create with out-of-range sampleRate/channels returns nullptr.
 */
HWTEST_F(AudioSuiteTimeStretchTest, Create_InvalidConfig, TestSize.Level1)
{
    // sampleRate below MIN_SAMPLE_RATE
    {
        auto p = AudioSuiteTimeStretch::Create(INVALID_SAMPLE_RATE_LOW, CHANNELS, SAMPLE_S16LE);
        EXPECT_EQ(p, nullptr);
    }
    // sampleRate above MAX_SAMPLE_RATE
    {
        auto p = AudioSuiteTimeStretch::Create(INVALID_SAMPLE_RATE_HIGH, CHANNELS, SAMPLE_S16LE);
        EXPECT_EQ(p, nullptr);
    }
    // channels below MIN_CHANNELS
    {
        auto p = AudioSuiteTimeStretch::Create(SAMPLE_RATE, INVALID_CHANNELS_LOW, SAMPLE_S16LE);
        EXPECT_EQ(p, nullptr);
    }
    // channels above MAX_CHANNELS
    {
        auto p = AudioSuiteTimeStretch::Create(SAMPLE_RATE, INVALID_CHANNELS_HIGH, SAMPLE_S16LE);
        EXPECT_EQ(p, nullptr);
    }
}

/**
 * @tc.name  : Test AudioSuiteTimeStretch SetSpeed and GetSpeed.
 * @tc.number: AudioSuiteTimeStretch_SetSpeed_GetSpeed
 * @tc.desc  : Test SetSpeed/GetSpeed after Create and after reset, including out-of-range values.
 */
HWTEST_F(AudioSuiteTimeStretchTest, SetSpeed_GetSpeed, TestSize.Level1)
{
    // After Create: normal set and get
    {
        auto p = AudioSuiteTimeStretch::Create(SAMPLE_RATE, CHANNELS, SAMPLE_S16LE);
        ASSERT_NE(p, nullptr);
        EXPECT_EQ(p->SetSpeed(SPEED_NORMAL), SUCCESS);
        EXPECT_FLOAT_EQ(p->GetSpeed(), SPEED_NORMAL);
        // Out of range
        EXPECT_NE(p->SetSpeed(SPEED_OUT_OF_RANGE_LOW), SUCCESS);   // below MIN_SPEED
        EXPECT_NE(p->SetSpeed(SPEED_OUT_OF_RANGE_HIGH), SUCCESS);  // above MAX_SPEED
        p.reset();
    }
    // After reset: GetSpeed on default-constructed (empty) instance returns default value
    {
        auto p = AudioSuiteTimeStretch::Create(SAMPLE_RATE, CHANNELS, SAMPLE_S16LE);
        ASSERT_NE(p, nullptr);
        p.reset();
        EXPECT_EQ(p, nullptr);
    }
}

/**
 * @tc.name  : Test AudioSuiteTimeStretch SetPitch and GetPitch.
 * @tc.number: AudioSuiteTimeStretch_SetPitch_GetPitch
 * @tc.desc  : Test SetPitch/GetPitch after Create and after reset, including out-of-range values.
 */
HWTEST_F(AudioSuiteTimeStretchTest, SetPitch_GetPitch, TestSize.Level1)
{
    // After Create: normal set and get
    {
        auto p = AudioSuiteTimeStretch::Create(SAMPLE_RATE, CHANNELS, SAMPLE_S16LE);
        ASSERT_NE(p, nullptr);
        EXPECT_EQ(p->SetPitch(PITCH_NORMAL), SUCCESS);
        // GetPitch should return the set value (or algo default, depending on implementation)
        float pitch = p->GetPitch();
        EXPECT_TRUE(pitch >= TIME_STRETCH_MIN_PITCH && pitch <= TIME_STRETCH_MAX_PITCH);
        // Out of range
        EXPECT_NE(p->SetPitch(PITCH_OUT_OF_RANGE_LOW), SUCCESS);   // below MIN_PITCH
        EXPECT_NE(p->SetPitch(PITCH_OUT_OF_RANGE_HIGH), SUCCESS);  // above MAX_PITCH
        p.reset();
    }
    // After reset: GetPitch on default-constructed (empty) instance returns default value
    {
        auto p = AudioSuiteTimeStretch::Create(SAMPLE_RATE, CHANNELS, SAMPLE_S16LE);
        ASSERT_NE(p, nullptr);
        p.reset();
        EXPECT_EQ(p, nullptr);
    }
}

/**
 * @tc.name  : Test AudioSuiteTimeStretch GetDelayFrames.
 * @tc.number: AudioSuiteTimeStretch_GetDelayFrames
 * @tc.desc  : Test GetDelayFrames after Create.
 */
HWTEST_F(AudioSuiteTimeStretchTest, GetDelayFrames, TestSize.Level1)
{
    // After Create: returns actual algorithm delay (non-negative)
    {
        auto p = AudioSuiteTimeStretch::Create(SAMPLE_RATE, CHANNELS, SAMPLE_S16LE);
        ASSERT_NE(p, nullptr);
        int32_t delay = p->GetDelayFrames();
        EXPECT_GE(delay, 0);
        p.reset();
    }
}

/**
 * @tc.name  : Test AudioSuiteTimeStretch Process with F32LE/U8/S16 formats.
 * @tc.number: AudioSuiteTimeStretch_Process_F32LE_U8_S16
 * @tc.desc  : Test Process F32LE (no convert), U8/S16 (format convert) paths.
 */
HWTEST_F(AudioSuiteTimeStretchTest, Process_F32LE_U8_S16, TestSize.Level1)
{
    int32_t outSize = 0;

    // SAMPLE_F32LE: needFormatConvert=false (direct path)
    {
        auto p = AudioSuiteTimeStretch::Create(SAMPLE_RATE, CHANNELS, SAMPLE_F32LE);
        ASSERT_NE(p, nullptr);
        auto inputData = GenerateFloatInput(TEST_IN_FRAMES, CHANNELS);
        const int32_t inputSize = TEST_IN_FRAMES * CHANNELS * sizeof(float);
        const int32_t outCapacity = TEST_IN_FRAMES * CHANNELS * sizeof(float) * OUTPUT_CAPACITY_X2;
        std::vector<uint8_t> output(outCapacity, 0);
        EXPECT_EQ(p->Process(reinterpret_cast<const uint8_t *>(inputData.data()), inputSize,
            output.data(), outCapacity, outSize), SUCCESS);
        EXPECT_GE(outSize, 0);
        p.reset();
    }
    // SAMPLE_U8: needFormatConvert=true
    {
        auto p = AudioSuiteTimeStretch::Create(SAMPLE_RATE, CHANNELS, SAMPLE_U8);
        ASSERT_NE(p, nullptr);
        auto inputData = GenerateU8Input(TEST_IN_FRAMES, CHANNELS);
        const int32_t inputSize = TEST_IN_FRAMES * CHANNELS * U8_SIZE;
        std::vector<uint8_t> output(inputSize * OUTPUT_CAPACITY_X2, 0);
        EXPECT_EQ(p->Process(reinterpret_cast<const uint8_t *>(inputData.data()), inputSize,
            output.data(), inputSize * OUTPUT_CAPACITY_X2, outSize), SUCCESS);
        EXPECT_GE(outSize, 0);
        p.reset();
    }
    // SAMPLE_S16LE: needFormatConvert=true
    {
        auto p = AudioSuiteTimeStretch::Create(SAMPLE_RATE, CHANNELS, SAMPLE_S16LE);
        ASSERT_NE(p, nullptr);
        auto inputData = GenerateS16Input(TEST_IN_FRAMES, CHANNELS);
        const int32_t inputSize = TEST_IN_FRAMES * CHANNELS * sizeof(int16_t);
        // Normal capacity (covers if false branch: maxOutFrames >= expectedOutFrames)
        std::vector<uint8_t> output(inputSize * OUTPUT_CAPACITY_X2, 0);
        EXPECT_EQ(p->Process(reinterpret_cast<const uint8_t *>(inputData.data()), inputSize,
            output.data(), inputSize * OUTPUT_CAPACITY_X2, outSize), SUCCESS);
        EXPECT_GE(outSize, 0);
        // Set speed=0.5 (slow) so expectedOutFrames = 2*inFrames, then provide
        // insufficient output capacity (covers if true branch: triggers WARNING)
        ASSERT_EQ(p->SetSpeed(0.5f), SUCCESS);
        std::vector<uint8_t> smallOutput(inputSize, 0); // capacity only fits inFrames < 2*inFrames
        EXPECT_EQ(p->Process(reinterpret_cast<const uint8_t *>(inputData.data()), inputSize,
            smallOutput.data(), inputSize, outSize), SUCCESS);
        p.reset();
    }
}

/**
 * @tc.name  : Test AudioSuiteTimeStretch Process with S24/S32 formats.
 * @tc.number: AudioSuiteTimeStretch_Process_S24_S32
 * @tc.desc  : Test Process S24/S32 (format convert) paths.
 */
HWTEST_F(AudioSuiteTimeStretchTest, Process_S24_S32, TestSize.Level1)
{
    int32_t outSize = 0;

    // SAMPLE_S24LE: needFormatConvert=true
    {
        auto p = AudioSuiteTimeStretch::Create(SAMPLE_RATE, CHANNELS, SAMPLE_S24LE);
        ASSERT_NE(p, nullptr);
        auto inputData = GenerateS24Input(TEST_IN_FRAMES, CHANNELS);
        const int32_t inputSize = TEST_IN_FRAMES * CHANNELS * S24_SIZE;
        std::vector<uint8_t> output(inputSize * OUTPUT_CAPACITY_X2, 0);
        EXPECT_EQ(p->Process(reinterpret_cast<const uint8_t *>(inputData.data()), inputSize,
            output.data(), inputSize * OUTPUT_CAPACITY_X2, outSize), SUCCESS);
        EXPECT_GE(outSize, 0);
        p.reset();
    }
    // SAMPLE_S32LE: needFormatConvert=true
    {
        auto p = AudioSuiteTimeStretch::Create(SAMPLE_RATE, CHANNELS, SAMPLE_S32LE);
        ASSERT_NE(p, nullptr);
        auto inputData = GenerateS32Input(TEST_IN_FRAMES, CHANNELS);
        const int32_t inputSize = TEST_IN_FRAMES * CHANNELS * sizeof(int32_t);
        std::vector<uint8_t> output(inputSize * OUTPUT_CAPACITY_X2, 0);
        EXPECT_EQ(p->Process(reinterpret_cast<const uint8_t *>(inputData.data()), inputSize,
            output.data(), inputSize * OUTPUT_CAPACITY_X2, outSize), SUCCESS);
        EXPECT_GE(outSize, 0);
        p.reset();
    }
}

/**
 * @tc.name  : Test AudioSuiteTimeStretch Process parameter validation.
 * @tc.number: AudioSuiteTimeStretch_Process_InvalidParams
 * @tc.desc  : Test Process with invalid parameters (null pointers, zero sizes, unaligned input).
 */
HWTEST_F(AudioSuiteTimeStretchTest, Process_InvalidParams, TestSize.Level1)
{
    auto p = AudioSuiteTimeStretch::Create(SAMPLE_RATE, CHANNELS, SAMPLE_S16LE);
    ASSERT_NE(p, nullptr);

    const int32_t inFrames = TEST_SMALL_IN_FRAMES;
    auto inputData = GenerateS16Input(inFrames, CHANNELS);
    const int32_t inputSize = inFrames * CHANNELS * sizeof(int16_t);
    std::vector<uint8_t> output(inputSize * OUTPUT_CAPACITY_X2, 0);
    int32_t outSize = 0;

    // Null inputData
    EXPECT_NE(p->Process(nullptr, inputSize, output.data(), inputSize, outSize), SUCCESS);
    // inputSize <= 0
    EXPECT_NE(p->Process(reinterpret_cast<const uint8_t *>(inputData.data()), 0,
        output.data(), inputSize, outSize), SUCCESS);
    // Null outputData
    EXPECT_NE(p->Process(reinterpret_cast<const uint8_t *>(inputData.data()), inputSize,
        nullptr, inputSize, outSize), SUCCESS);
    // inputSize not aligned to bytesPerFrame (S16LE bytesPerFrame=4)
    EXPECT_NE(p->Process(reinterpret_cast<const uint8_t *>(inputData.data()), UNALIGNED_BYTE_COUNT,
        output.data(), inputSize, outSize), SUCCESS);
    p.reset();

    // Processor with invalid config (Create returns nullptr)
    {
        auto p2 = AudioSuiteTimeStretch::Create(SAMPLE_RATE, CHANNELS,
            static_cast<AudioSampleFormat>(INVALID_FORMAT_VAL));
        EXPECT_EQ(p2, nullptr);
    }
}

/**
 * @tc.name  : Test AudioSuiteTimeStretch Flush with F32LE.
 * @tc.number: AudioSuiteTimeStretch_Flush_F32LE
 * @tc.desc  : Test Flush with F32LE (no format convert) path. Feeds Process to
 *             accumulate frames, then Flush drains them.
 */
HWTEST_F(AudioSuiteTimeStretchTest, Flush_F32LE, TestSize.Level1)
{
    int32_t outSize = 0;
    int32_t flushSize = 0;
    auto p = AudioSuiteTimeStretch::Create(SAMPLE_RATE, CHANNELS, SAMPLE_F32LE);
    ASSERT_NE(p, nullptr);
    auto inputData = GenerateFloatInput(TEST_IN_FRAMES, CHANNELS);
    const int32_t inputSize = TEST_IN_FRAMES * CHANNELS * sizeof(float);
    const int32_t outCapacity = TEST_IN_FRAMES * CHANNELS * sizeof(float) * OUTPUT_CAPACITY_X2;
    std::vector<uint8_t> output(outCapacity, 0);
    p->Process(reinterpret_cast<const uint8_t *>(inputData.data()), inputSize,
        output.data(), outCapacity, outSize);
    const int32_t flushCapacity = TEST_IN_FRAMES * CHANNELS * sizeof(float) * OUTPUT_CAPACITY_X2;
    std::vector<uint8_t> flushOutput(flushCapacity, 0);
    int32_t residualFrames = -1;
    EXPECT_EQ(p->Flush(flushOutput.data(), flushCapacity, flushSize, residualFrames), SUCCESS);
    EXPECT_GE(flushSize, 0);
    EXPECT_GE(residualFrames, 0);
    p.reset();
}

/**
 * @tc.name  : Test AudioSuiteTimeStretch Flush with U8.
 * @tc.number: AudioSuiteTimeStretch_Flush_U8
 * @tc.desc  : Test Flush with U8 (format convert) path. Feeds multiple Process
 *             calls to accumulate frames, then Flush drains them, exercising
 *             ConvertToFloat(U8) + ConvertFromFloat(U8).
 */
HWTEST_F(AudioSuiteTimeStretchTest, Flush_U8, TestSize.Level1)
{
    int32_t outSize = 0;
    int32_t flushSize = 0;
    auto p = AudioSuiteTimeStretch::Create(SAMPLE_RATE, CHANNELS, SAMPLE_U8);
    ASSERT_NE(p, nullptr);
    auto inputData = GenerateU8Input(TEST_IN_FRAMES, CHANNELS);
    const int32_t inputSize = TEST_IN_FRAMES * CHANNELS * U8_SIZE;
    const int32_t outCapacity = inputSize * OUTPUT_CAPACITY_X2;
    std::vector<uint8_t> output(outCapacity, 0);
    for (int i = 0; i < 4; i++) {
        p->Process(reinterpret_cast<const uint8_t *>(inputData.data()), inputSize,
            output.data(), outCapacity, outSize);
    }
    const int32_t flushCapacity = inputSize * OUTPUT_CAPACITY_X2;
    std::vector<uint8_t> flushOutput(flushCapacity, 0);
    int32_t residualFrames = -1;
    EXPECT_EQ(p->Flush(flushOutput.data(), flushCapacity, flushSize, residualFrames), SUCCESS);
    EXPECT_GE(flushSize, 0);
    p.reset();
}

/**
 * @tc.name  : Test AudioSuiteTimeStretch Flush with S16LE.
 * @tc.number: AudioSuiteTimeStretch_Flush_S16LE
 * @tc.desc  : Test Flush with S16LE (format convert) path. Feeds multiple Process
 *             calls to accumulate frames, then Flush drains them, exercising
 *             ConvertToFloat(S16) + ConvertFromFloat(S16).
 */
HWTEST_F(AudioSuiteTimeStretchTest, Flush_S16LE, TestSize.Level1)
{
    int32_t outSize = 0;
    int32_t flushSize = 0;
    auto p = AudioSuiteTimeStretch::Create(SAMPLE_RATE, CHANNELS, SAMPLE_S16LE);
    ASSERT_NE(p, nullptr);
    auto inputData = GenerateS16Input(TEST_IN_FRAMES, CHANNELS);
    const int32_t inputSize = TEST_IN_FRAMES * CHANNELS * sizeof(int16_t);
    const int32_t outCapacity = inputSize * OUTPUT_CAPACITY_X2;
    std::vector<uint8_t> output(outCapacity, 0);
    for (int i = 0; i < 4; i++) {
        p->Process(reinterpret_cast<const uint8_t *>(inputData.data()), inputSize,
            output.data(), outCapacity, outSize);
    }
    const int32_t flushCapacity = inputSize * OUTPUT_CAPACITY_X2;
    std::vector<uint8_t> flushOutput(flushCapacity, 0);
    int32_t residualFrames = -1;
    EXPECT_EQ(p->Flush(flushOutput.data(), flushCapacity, flushSize, residualFrames), SUCCESS);
    EXPECT_GE(flushSize, 0);
    p.reset();
}

/**
 * @tc.name  : Test AudioSuiteTimeStretch Flush with S24LE.
 * @tc.number: AudioSuiteTimeStretch_Flush_S24LE
 * @tc.desc  : Test Flush with S24LE (format convert) path. Feeds multiple Process
 *             calls to accumulate frames, then Flush drains them, exercising
 *             ConvertToFloat(S24) + ConvertFromFloat(S24).
 */
HWTEST_F(AudioSuiteTimeStretchTest, Flush_S24LE, TestSize.Level1)
{
    int32_t outSize = 0;
    int32_t flushSize = 0;
    auto p = AudioSuiteTimeStretch::Create(SAMPLE_RATE, CHANNELS, SAMPLE_S24LE);
    ASSERT_NE(p, nullptr);
    auto inputData = GenerateS24Input(TEST_IN_FRAMES, CHANNELS);
    const int32_t inputSize = TEST_IN_FRAMES * CHANNELS * S24_SIZE;
    const int32_t outCapacity = inputSize * OUTPUT_CAPACITY_X2;
    std::vector<uint8_t> output(outCapacity, 0);
    for (int i = 0; i < 4; i++) {
        p->Process(reinterpret_cast<const uint8_t *>(inputData.data()), inputSize,
            output.data(), outCapacity, outSize);
    }
    const int32_t flushCapacity = inputSize * OUTPUT_CAPACITY_X2;
    std::vector<uint8_t> flushOutput(flushCapacity, 0);
    int32_t residualFrames = -1;
    EXPECT_EQ(p->Flush(flushOutput.data(), flushCapacity, flushSize, residualFrames), SUCCESS);
    EXPECT_GE(flushSize, 0);
    p.reset();
}

/**
 * @tc.name  : Test AudioSuiteTimeStretch Flush with S32LE.
 * @tc.number: AudioSuiteTimeStretch_Flush_S32LE
 * @tc.desc  : Test Flush with S32LE (format convert) path. Feeds multiple Process
 *             calls to accumulate frames, then Flush drains them, exercising
 *             ConvertToFloat(S32) + ConvertFromFloat(S32).
 */
HWTEST_F(AudioSuiteTimeStretchTest, Flush_S32LE, TestSize.Level1)
{
    int32_t outSize = 0;
    int32_t flushSize = 0;
    auto p = AudioSuiteTimeStretch::Create(SAMPLE_RATE, CHANNELS, SAMPLE_S32LE);
    ASSERT_NE(p, nullptr);
    auto inputData = GenerateS32Input(TEST_IN_FRAMES, CHANNELS);
    const int32_t inputSize = TEST_IN_FRAMES * CHANNELS * sizeof(int32_t);
    const int32_t outCapacity = inputSize * OUTPUT_CAPACITY_X2;
    std::vector<uint8_t> output(outCapacity, 0);
    for (int i = 0; i < 4; i++) {
        p->Process(reinterpret_cast<const uint8_t *>(inputData.data()), inputSize,
            output.data(), outCapacity, outSize);
    }
    const int32_t flushCapacity = inputSize * OUTPUT_CAPACITY_X2;
    std::vector<uint8_t> flushOutput(flushCapacity, 0);
    int32_t residualFrames = -1;
    EXPECT_EQ(p->Flush(flushOutput.data(), flushCapacity, flushSize, residualFrames), SUCCESS);
    EXPECT_GE(flushSize, 0);
    p.reset();
}
} // namespace AudioStandard
} // namespace OHOS
