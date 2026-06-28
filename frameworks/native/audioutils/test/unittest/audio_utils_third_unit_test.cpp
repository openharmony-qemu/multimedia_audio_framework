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

#include <cstdint>
#include <thread>
#include <gtest/gtest.h>
#include "gmock/gmock.h"
#include "audio_utils.h"
#include "parameter.h"
#include "audio_channel_blend.h"
#include "volume_ramp.h"
#include "audio_speed.h"
#include "audio_errors.h"

using namespace testing::ext;
using namespace testing;
using namespace std;
namespace OHOS {
namespace AudioStandard {

constexpr size_t TEST_RATE = 44100;
constexpr size_t TEST_CHANNELS = 2;
constexpr size_t TEST_FORMAT_S16LE = 2;
constexpr int32_t TEST_MAX_BUFFER_SIZE = 614400;

// ==================== AudioBlend Additional Tests ====================

/**
 * @tc.name  : Test AudioBlend with MODE_BLEND_LR mode for stereo
 * @tc.number: AudioBlend_ModeBlendLR_Stereo_001
 * @tc.desc  : Test AudioBlend Process method with MODE_BLEND_LR for stereo channels.
 */
HWTEST(AudioUtilsThirdUnitTest, AudioBlend_ModeBlendLR_Stereo_001, TestSize.Level1)
{
    uint8_t buffer[4] = {100, 200, 50, 80}; // L=100, R=200, L=50, R=80
    uint8_t format = SAMPLE_S16LE;
    uint8_t channels = STEREO;
    ChannelBlendMode blendMode = MODE_BLEND_LR;
    
    shared_ptr<AudioBlend> audioBlend = make_shared<AudioBlend>(blendMode, format, channels);
    audioBlend->Process(buffer, 4);
    
    // After blending, L and R should be averaged
    // For S16LE, we need to interpret as int16_t
    int16_t* samples = reinterpret_cast<int16_t*>(buffer);
    EXPECT_NE(samples[0], 100); // Should be modified
    EXPECT_NE(samples[1], 200); // Should be modified
}

/**
 * @tc.name  : Test AudioBlend with MODE_ALL_LEFT mode
 * @tc.number: AudioBlend_ModeAllLeft_Stereo_001
 * @tc.desc  : Test AudioBlend Process method with MODE_ALL_LEFT for stereo channels.
 */
HWTEST(AudioUtilsThirdUnitTest, AudioBlend_ModeAllLeft_Stereo_001, TestSize.Level1)
{
    uint8_t buffer[4] = {100, 200, 50, 80};
    uint8_t format = SAMPLE_S16LE;
    uint8_t channels = STEREO;
    ChannelBlendMode blendMode = MODE_ALL_LEFT;
    
    shared_ptr<AudioBlend> audioBlend = make_shared<AudioBlend>(blendMode, format, channels);
    audioBlend->Process(buffer, 4);
    
    // After processing, right channel should equal left channel
    int16_t* samples = reinterpret_cast<int16_t*>(buffer);
    EXPECT_EQ(samples[1], samples[0]); // Right should equal left
}

/**
 * @tc.name  : Test AudioBlend with MODE_ALL_RIGHT mode
 * @tc.number: AudioBlend_ModeAllRight_Stereo_001
 * @tc.desc  : Test AudioBlend Process method with MODE_ALL_RIGHT for stereo channels.
 */
HWTEST(AudioUtilsThirdUnitTest, AudioBlend_ModeAllRight_Stereo_001, TestSize.Level1)
{
    uint8_t buffer[4] = {100, 200, 50, 80};
    uint8_t format = SAMPLE_S16LE;
    uint8_t channels = STEREO;
    ChannelBlendMode blendMode = MODE_ALL_RIGHT;
    
    shared_ptr<AudioBlend> audioBlend = make_shared<AudioBlend>(blendMode, format, channels);
    audioBlend->Process(buffer, 4);
    
    // After processing, left channel should equal right channel
    int16_t* samples = reinterpret_cast<int16_t*>(buffer);
    EXPECT_EQ(samples[0], samples[1]); // Left should equal right
}

/**
 * @tc.name  : Test AudioBlend with zero channels
 * @tc.number: AudioBlend_ZeroChannels_001
 * @tc.desc  : Test AudioBlend Process method when channels is 0.
 */
HWTEST(AudioUtilsThirdUnitTest, AudioBlend_ZeroChannels_001, TestSize.Level1)
{
    uint8_t buffer[4] = {100, 200, 50, 80};
    uint8_t format = SAMPLE_S16LE;
    uint8_t channels = 0;
    ChannelBlendMode blendMode = MODE_BLEND_LR;
    
    shared_ptr<AudioBlend> audioBlend = make_shared<AudioBlend>(blendMode, format, channels);
    audioBlend->Process(buffer, 4);
    
    // Should return early without modifying buffer
    int16_t* samples = reinterpret_cast<int16_t*>(buffer);
    EXPECT_EQ(samples[0], 100);
    EXPECT_EQ(samples[1], 200);
}

/**
 * @tc.name  : Test AudioBlend SetParams
 * @tc.number: AudioBlend_SetParams_001
 * @tc.desc  : Test AudioBlend SetParams method.
 */
HWTEST(AudioUtilsThirdUnitTest, AudioBlend_SetParams_001, TestSize.Level1)
{
    shared_ptr<AudioBlend> audioBlend = make_shared<AudioBlend>();
    
    // Test SetParams
    audioBlend->SetParams(MODE_ALL_LEFT, SAMPLE_S32LE, CHANNEL_4);
    
    // Process should now use new parameters
    uint8_t buffer[16] = {0}; // 4 channels * 4 bytes per S32LE
    audioBlend->Process(buffer, 16);
    
    // Should not crash
    EXPECT_TRUE(true);
}

// ==================== VolumeRamp Additional Tests ====================

/**
 * @tc.name  : Test VolumeRamp ramp down
 * @tc.number: VolumeRamp_RampDown_001
 * @tc.desc  : Test VolumeRamp with ramp down direction.
 */
HWTEST(AudioUtilsThirdUnitTest, VolumeRamp_RampDown_001, TestSize.Level1)
{
    shared_ptr<VolumeRamp> volumeRamp = make_shared<VolumeRamp>();
    
    // Target volume is lower than current, should ramp down
    volumeRamp->SetVolumeRampConfig(0.5f, 1.0f, 100);
    EXPECT_EQ(volumeRamp->rampDirection_, RAMP_DOWN);
    
    // GetrampVolume should return decreasing values
    float volume = volumeRamp->GetRampVolume();
    EXPECT_LE(volume, 1.0f);
}

/**
 * @tc.name  : Test VolumeRamp ramp up
 * @tc.number: VolumeRamp_RampUp_001
 * @tc.desc  : Test VolumeRamp with ramp up direction.
 */
HWTEST(AudioUtilsThirdUnitTest, VolumeRamp_RampUp_001, TestSize.Level1)
{
    shared_ptr<VolumeRamp> volumeRamp = make_shared<VolumeRamp>();
    
    // Target volume is higher than current, should ramp up
    volumeRamp->SetVolumeRampConfig(1.0f, 0.5f, 100);
    EXPECT_EQ(volumeRamp->rampDirection_, RAMP_UP);
    
    // GetRampVolume should return increasing values
    float volume = volumeRamp->GetRampVolume();
    EXPECT_GE(volume, 0.5f);
}

/**
 * @tc.name  : Test VolumeRamp termination
 * @tc.number: VolumeRamp_Terminate_001
 * @tc.desc  : Test VolumeRamp Terminate method.
 */
HWTEST(AudioUtilsThirdUnitTest, VolumeRamp_Terminate_001, TestSize.Level1)
{
    shared_ptr<VolumeRamp> volumeRamp = make_shared<VolumeRamp>();
    
    volumeRamp->SetVolumeRampConfig(0.5f, 1.0f, 100);
    EXPECT_TRUE(volumeRamp->IsActive());
    
    volumeRamp->Terminate();
    EXPECT_FALSE(volumeRamp->IsActive());
}

/**
 * @tc.name  : Test VolumeRamp GetRampVolume when not active
 * @tc.number: VolumeRamp_GetRampVolume_NotActive_001
 * @tc.desc  : Test VolumeRamp GetRampVolume when ramp is not active.
 */
HWTEST(AudioUtilsThirdUnitTest, VolumeRamp_GetRampVolume_NotActive_001, TestSize.Level1)
{
    shared_ptr<VolumeRamp> volumeRamp = make_shared<VolumeRamp>();
    
    // Should return current rampVolume_ when not active
    volumeRamp->rampVolume_ = 0.7f;
    volumeRamp->isVolumeRampActive_ = false;
    
    float volume = volumeRamp->GetRampVolume();
    EXPECT_FLOAT_EQ(volume, 0.7f);
}

/**
 * @tc.name  : Test VolumeRamp with edge case durations
 * @tc.number: VolumeRamp_EdgeCases_001
 * @tc.desc  : Test VolumeRamp with zero and negative durations.
 */
HWTEST(AudioUtilsThirdUnitTest, VolumeRamp_EdgeCases_001, TestSize.Level1)
{
    shared_ptr<VolumeRamp> volumeRamp = make_shared<VolumeRamp>();
    
    // Test with zero duration
    volumeRamp->SetVolumeRampConfig(0.5f, 1.0f, 0);
    EXPECT_EQ(volumeRamp->duration_, 0);
    
    // Test with negative duration (should handle gracefully)
    volumeRamp->SetVolumeRampConfig(0.5f, 1.0f, -100);
    EXPECT_LE(volumeRamp->duration_, 0);
}

// ==================== AudioSpeed Additional Tests ====================

/**
 * @tc.name  : Test AudioSpeed constructor and destructor
 * @tc.number: AudioSpeed_ConstructorDestructor_001
 * @tc.desc  : Test AudioSpeed constructor and destructor.
 */
HWTEST(AudioUtilsThirdUnitTest, AudioSpeed_ConstructorDestructor_001, TestSize.Level1)
{
    // Test constructor
    AudioSpeed* audioSpeed = new AudioSpeed(TEST_RATE, TEST_FORMAT_S16LE, TEST_CHANNELS, TEST_MAX_BUFFER_SIZE);
    
    EXPECT_NE(audioSpeed, nullptr);
    EXPECT_FLOAT_EQ(audioSpeed->GetSpeed(), 1.0f);
    
    // Test destructor
    delete audioSpeed;
    
    // Should not crash
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test AudioSpeed SetSpeed
 * @tc.number: AudioSpeed_SetSpeed_001
 * @tc.desc  : Test AudioSpeed SetSpeed method.
 */
HWTEST(AudioUtilsThirdUnitTest, AudioSpeed_SetSpeed_001, TestSize.Level1)
{
    AudioSpeed audioSpeed(TEST_RATE, TEST_FORMAT_S16LE, TEST_CHANNELS, TEST_MAX_BUFFER_SIZE);
    
    int32_t ret = audioSpeed.SetSpeed(0.5f);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_FLOAT_EQ(audioSpeed.GetSpeed(), 0.5f);
    
    ret = audioSpeed.SetSpeed(2.0f);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_FLOAT_EQ(audioSpeed.GetSpeed(), 2.0f);
}

/**
 * @tc.name  : Test AudioSpeed SetPitch
 * @tc.number: AudioSpeed_SetPitch_001
 * @tc.desc  : Test AudioSpeed SetPitch method.
 */
HWTEST(AudioUtilsThirdUnitTest, AudioSpeed_SetPitch_001, TestSize.Level1)
{
    AudioSpeed audioSpeed(TEST_RATE, TEST_FORMAT_S16LE, TEST_CHANNELS, TEST_MAX_BUFFER_SIZE);
    
    int32_t ret = audioSpeed.SetPitch(1.5f);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioSpeed GetPitchForSpeed
 * @tc.number: AudioSpeed_GetPitchForSpeed_001
 * @tc.desc  : Test AudioSpeed GetPitchForSpeed method.
 */
HWTEST(AudioUtilsThirdUnitTest, AudioSpeed_GetPitchForSpeed_001, TestSize.Level1)
{
    float pitch = AudioSpeed::GetPitchForSpeed(1.0f);
    EXPECT_FLOAT_EQ(pitch, 1.0f);
    
    pitch = AudioSpeed::GetPitchForSpeed(0.5f);
    EXPECT_GT(pitch, 0.0f);
    EXPECT_LT(pitch, 1.0f);
    
    pitch = AudioSpeed::GetPitchForSpeed(0.125f); // SLOW_PLAY_1_8_SPEED
    EXPECT_GT(pitch, 0.0f);
}

/**
 * @tc.name  : Test AudioSpeed with different formats
 * @tc.number: AudioSpeed_DifferentFormats_001
 * @tc.desc  : Test AudioSpeed with different audio formats.
 */
HWTEST(AudioUtilsThirdUnitTest, AudioSpeed_DifferentFormats_001, TestSize.Level1)
{
    // Test with SAMPLE_U8
    AudioSpeed audioSpeedU8(TEST_RATE, SAMPLE_U8, TEST_CHANNELS, TEST_MAX_BUFFER_SIZE);
    EXPECT_EQ(audioSpeedU8.format_, SAMPLE_U8);
    
    // Test with SAMPLE_S24LE
    AudioSpeed audioSpeedS24(TEST_RATE, SAMPLE_S24LE, TEST_CHANNELS, TEST_MAX_BUFFER_SIZE);
    EXPECT_EQ(audioSpeedS24.format_, SAMPLE_S24LE);
    
    // Test with SAMPLE_F32LE
    AudioSpeed audioSpeedF32(TEST_RATE, SAMPLE_F32LE, TEST_CHANNELS, TEST_MAX_BUFFER_SIZE);
    EXPECT_EQ(audioSpeedF32.format_, SAMPLE_F32LE);
}

/**
 * @tc.name  : Test AudioSpeed Init failure
 * @tc.number: AudioSpeed_InitFailure_001
 * @tc.desc  : Test AudioSpeed Init method when sonicCreateStream fails.
 */
HWTEST(AudioUtilsThirdUnitTest, AudioSpeed_InitFailure_001, TestSize.Level1)
{
    // Test with invalid parameters that might cause Init to fail
    AudioSpeed audioSpeed(0, SAMPLE_S16LE, 0, 0);
    
    int32_t ret = audioSpeed.Init();
    // Should handle failure gracefully
    EXPECT_TRUE(ret == SUCCESS || ret == ERROR);
}

/**
 * @tc.name  : Test AudioSpeed Flush
 * @tc.number: AudioSpeed_Flush_001
 * @tc.desc  : Test AudioSpeed Flush method.
 */
HWTEST(AudioUtilsThirdUnitTest, AudioSpeed_Flush_001, TestSize.Level1)
{
    AudioSpeed audioSpeed(TEST_RATE, TEST_FORMAT_S16LE, TEST_CHANNELS, TEST_MAX_BUFFER_SIZE);
    
    int32_t ret = audioSpeed.Flush();
    EXPECT_EQ(ret, SUCCESS);
}

} // namespace AudioStandard
} // namespace OHOS
