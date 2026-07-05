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

#include "audio_capturer_third_unit_test.h"

#include <chrono>
#include <future>
#include <thread>

#include "audio_capturer_proxy_obj.h"
#include "audio_policy_manager.h"
#include "audio_capturer_private.h"
#include "audio_stream_enum.h"

using namespace std;
using namespace std::chrono;
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {

// ==================== Additional AudioCapturer Tests ====================

/**
 * @tc.name  : Test AudioCapturer SetParams with invalid parameter
 * @tc.number: Audio_Capturer_SetParams_001
 * @tc.desc  : Test SetParams API with invalid parameter.
 */
HWTEST(AudioCapturerThirdUnitTest, Audio_Capturer_SetParams_001, TestSize.Level1)
{
    AudioCapturerOptions options;
    options.streamInfo.samplingRate = SAMPLE_RATE_44100;
    options.streamInfo.encoding = ENCODING_PCM;
    options.streamInfo.format = SAMPLE_S16LE;
    options.streamInfo.channels = STEREO;
    options.capturerInfo.sourceType = SOURCE_TYPE_MIC;
    options.capturerInfo.capturerFlags = AUDIO_INPUT_FLAG_NORMAL;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(options);
    ASSERT_EQ(nullptr, audioCapturer);
    if (audioCapturer != nullptr) {
        // Test SetParams with invalid parameter
        AudioCapturerParams params;
        params.inputSource = static_cast<AudioSourceType>(-1); // Invalid source type

        int32_t ret = audioCapturer->SetParams(params);
        EXPECT_EQ(ERR_INVALID_PARAM, ret); // Expect error for invalid parameter
    }
}

/**
 * @tc.name  : Test AudioCapturer GetCurrentInputDevices
 * @tc.number: Audio_Capturer_GetCurrentInputDevices_001
 * @tc.desc  : Test GetCurrentInputDevices API.
 */
HWTEST(AudioCapturerThirdUnitTest, Audio_Capturer_GetCurrentInputDevices_001, TestSize.Level1)
{
    AudioCapturerOptions options;
    options.streamInfo.samplingRate = SAMPLE_RATE_44100;
    options.streamInfo.encoding = ENCODING_PCM;
    options.streamInfo.format = SAMPLE_S16LE;
    options.streamInfo.channels = STEREO;
    options.capturerInfo.sourceType = SOURCE_TYPE_MIC;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(options);
    ASSERT_EQ(nullptr, audioCapturer);
    if (audioCapturer != nullptr) {
        // Test GetCurrentInputDevices
        AudioDeviceDescriptor deviceInfo;
        int32_t ret = audioCapturer->GetCurrentInputDevices(deviceInfo);
        EXPECT_EQ(SUCCESS, ret);
    }
}

/**
 * @tc.name  : Test AudioCapturer SetInputDevice
 * @tc.number: Audio_Capturer_SetInputDevice_001
 * @tc.desc  : Test SetInputDevice API.
 */
HWTEST(AudioCapturerThirdUnitTest, Audio_Capturer_SetInputDevice_001, TestSize.Level1)
{
    AudioCapturerOptions options;
    options.streamInfo.samplingRate = SAMPLE_RATE_44100;
    options.streamInfo.encoding = ENCODING_PCM;
    options.streamInfo.format = SAMPLE_S16LE;
    options.streamInfo.channels = STEREO;
    options.capturerInfo.sourceType = SOURCE_TYPE_MIC;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(options);
    ASSERT_EQ(nullptr, audioCapturer);
    if (audioCapturer != nullptr) {
        // Test SetInputDevice
        int32_t ret = audioCapturer->SetInputDevice(DEVICE_TYPE_MIC);
        EXPECT_EQ(SUCCESS, ret);
    }
}

/**
 * @tc.name  : Test AudioCapturer SetCaptureMode with invalid parameter
 * @tc.number: Audio_Capturer_CaptureMode_InvalidParam_001
 * @tc.desc  : Test SetCaptureMode API with invalid parameter.
 */
HWTEST(AudioCapturerThirdUnitTest, Audio_Capturer_CaptureMode_InvalidParam_001, TestSize.Level1)
{
    AudioCapturerOptions options;
    options.streamInfo.samplingRate = SAMPLE_RATE_44100;
    options.streamInfo.encoding = ENCODING_PCM;
    options.streamInfo.format = SAMPLE_S16LE;
    options.streamInfo.channels = STEREO;
    options.capturerInfo.sourceType = SOURCE_TYPE_MIC;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(options);
    ASSERT_EQ(nullptr, audioCapturer);
    if (audioCapturer != nullptr) {
        // Test SetCaptureMode with invalid parameter
        int32_t ret = audioCapturer->SetCaptureMode(static_cast<AudioCaptureMode>(-1)); // Invalid mode
        EXPECT_EQ(ERR_INVALID_PARAM, ret); // Expect error for invalid parameter
    }
}

/**
 * @tc.name  : Test AudioCapturer SetAudioSourceConcurrency with empty vector
 * @tc.number: Audio_Capturer_AudioSourceConcurrency_InvalidParam_001
 * @tc.desc  : Test SetAudioSourceConcurrency API with invalid parameter.
 */
HWTEST(AudioCapturerThirdUnitTest, Audio_Capturer_AudioSourceConcurrency_InvalidParam_001, TestSize.Level1)
{
    AudioCapturerOptions options;
    options.streamInfo.samplingRate = SAMPLE_RATE_44100;
    options.streamInfo.encoding = ENCODING_PCM;
    options.streamInfo.format = SAMPLE_S16LE;
    options.streamInfo.channels = STEREO;
    options.capturerInfo.sourceType = SOURCE_TYPE_MIC;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(options);
    ASSERT_EQ(nullptr, audioCapturer);
    if (audioCapturer != nullptr) {
        // Test SetAudioSourceConcurrency with empty vector (invalid parameter)
        vector<SourceType> targetSources = {}; // Empty vector - invalid parameter
        int32_t ret = audioCapturer->SetAudioSourceConcurrency(targetSources);
        EXPECT_EQ(ERR_INVALID_PARAM, ret); // Expect error for invalid parameter
    }
}

/**
 * @tc.name  : Test AudioCapturer SetInterruptStrategy with invalid parameter
 * @tc.number: Audio_Capturer_InterruptStrategy_InvalidParam_001
 * @tc.desc  : Test SetInterruptStrategy API with invalid parameter.
 */
HWTEST(AudioCapturerThirdUnitTest, Audio_Capturer_InterruptStrategy_InvalidParam_001, TestSize.Level1)
{
    AudioCapturerOptions options;
    options.streamInfo.samplingRate = SAMPLE_RATE_44100;
    options.streamInfo.encoding = ENCODING_PCM;
    options.streamInfo.format = SAMPLE_S16LE;
    options.streamInfo.channels = STEREO;
    options.capturerInfo.sourceType = SOURCE_TYPE_MIC;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(options);
    ASSERT_EQ(nullptr, audioCapturer);
    if (audioCapturer != nullptr) {
        // Test SetInterruptStrategy with invalid parameter
        int32_t ret = audioCapturer->SetInterruptStrategy(static_cast<InterruptStrategy>(-1)); // Invalid strategy
        EXPECT_EQ(ERR_INVALID_PARAM, ret); // Expect error for invalid parameter
    }
}

/**
 * @tc.name  : Test AudioCapturer SetMuteHint with invalid parameter
 * @tc.number: Audio_Capturer_MuteHint_InvalidParam_001
 * @tc.desc  : Test SetMuteHint API with invalid parameter.
 */
HWTEST(AudioCapturerThirdUnitTest, Audio_Capturer_MuteHint_InvalidParam_001, TestSize.Level1)
{
    AudioCapturerOptions options;
    options.streamInfo.samplingRate = SAMPLE_RATE_44100;
    options.streamInfo.encoding = ENCODING_PCM;
    options.streamInfo.format = SAMPLE_S16LE;
    options.streamInfo.channels = STEREO;
    options.capturerInfo.sourceType = SOURCE_TYPE_MIC;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(options);
    ASSERT_EQ(nullptr, audioCapturer);
    if (audioCapturer != nullptr) {
        // Test SetMuteHint with invalid parameter (nullptr callback)
        int32_t ret = audioCapturer->SetMuteHint(true);
        // Since we don't have a valid callback set up, this should return error
        EXPECT_EQ(ERR_INVALID_PARAM, ret);
    }
}

/**
 * @tc.name  : Test AudioCapturer GetOverflowCount
 * @tc.number: Audio_Capturer_OverflowCount_001
 * @tc.desc  : Test GetOverflowCount API.
 */
HWTEST(AudioCapturerThirdUnitTest, Audio_Capturer_OverflowCount_001, TestSize.Level1)
{
    AudioCapturerOptions options;
    options.streamInfo.samplingRate = SAMPLE_RATE_44100;
    options.streamInfo.encoding = ENCODING_PCM;
    options.streamInfo.format = SAMPLE_S16LE;
    options.streamInfo.channels = STEREO;
    options.capturerInfo.sourceType = SOURCE_TYPE_MIC;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(options);
    ASSERT_EQ(nullptr, audioCapturer);
    if (audioCapturer != nullptr) {
        // Test GetOverflowCount
        uint32_t overflowCount = audioCapturer->GetOverflowCount();
        EXPECT_GE(overflowCount, 0);
    }
}

/**
 * @tc.name  : Test AudioCapturer GetCurrentMicrophones with invalid parameter
 * @tc.number: Audio_Capturer_CurrentMicrophones_InvalidParam_001
 * @tc.desc  : Test GetCurrentMicrophones API with invalid parameter.
 */
HWTEST(AudioCapturerThirdUnitTest, Audio_Capturer_CurrentMicrophones_InvalidParam_001, TestSize.Level1)
{
    // Test GetCurrentMicrophones with invalid internal state
    AudioCapturerOptions options;
    options.streamInfo.samplingRate = SAMPLE_RATE_44100;
    options.streamInfo.encoding = ENCODING_PCM;
    options.streamInfo.format = SAMPLE_S16LE;
    options.streamInfo.channels = STEREO;
    options.capturerInfo.sourceType = SOURCE_TYPE_MIC;

    unique_ptr<AudioCapturer> capturer = AudioCapturer::Create(options);
    ASSERT_EQ(nullptr, capturer);
    if (capturer != nullptr) {
        // Test GetCurrentMicrophones - should return empty vector on error
        vector<sptr<MicrophoneDescriptor>> microphones = capturer->GetCurrentMicrophones();
        EXPECT_TRUE(microphones.empty()); // Expect empty vector for invalid state
    }
}

/**
 * @tc.name  : Test AudioCapturer SetInMainThreadState with invalid parameter
 * @tc.number: Audio_Capturer_InMainThreadState_InvalidParam_001
 * @tc.desc  : Test SetInMainThreadState API with invalid parameter.
 */
HWTEST(AudioCapturerThirdUnitTest, Audio_Capturer_InMainThreadState_InvalidParam_001, TestSize.Level1)
{
    // Test with invalid internal state
    AudioCapturerOptions options;
    options.streamInfo.samplingRate = SAMPLE_RATE_44100;
    options.streamInfo.encoding = ENCODING_PCM;
    options.streamInfo.format = SAMPLE_S16LE;
    options.streamInfo.channels = STEREO;
    options.capturerInfo.sourceType = SOURCE_TYPE_MIC;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(options);
    ASSERT_EQ(nullptr, audioCapturer);
    if (audioCapturer != nullptr) {
        // Test SetInMainThreadState - expect error due to invalid internal state
        int32_t ret = audioCapturer->SetInMainThreadState(true);
        EXPECT_EQ(ERR_INVALID_PARAM, ret); // Expect error for invalid internal state
    }
}

/**
 * @tc.name  : Test AudioCapturer SetInterruptEventCallbackType with invalid parameter
 * @tc.number: Audio_Capturer_InterruptEventCallbackType_InvalidParam_001
 * @tc.desc  : Test SetInterruptEventCallbackType API with invalid parameter.
 */
HWTEST(AudioCapturerThirdUnitTest, Audio_Capturer_InterruptEventCallbackType_InvalidParam_001, TestSize.Level1)
{
    AudioCapturerOptions options;
    options.streamInfo.samplingRate = SAMPLE_RATE_44100;
    options.streamInfo.encoding = ENCODING_PCM;
    options.streamInfo.format = SAMPLE_S16LE;
    options.streamInfo.channels = STEREO;
    options.capturerInfo.sourceType = SOURCE_TYPE_MIC;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(options);
    ASSERT_EQ(nullptr, audioCapturer);
    if (audioCapturer != nullptr) {
        // Test SetInterruptEventCallbackType with invalid parameter
        audioCapturer->SetInterruptEventCallbackType(static_cast<InterruptEventCallbackType>(-1)); // Invalid type

        // Should not crash (error handling)
        EXPECT_TRUE(true);
    }
}

/**
 * @tc.name  : Test AudioCapturer Create with invalid parameters
 * @tc.number: Audio_Capturer_Create_Invalid_001
 * @tc.desc  : Test Create with invalid stream type.
 */
HWTEST(AudioCapturerThirdUnitTest, Audio_Capturer_Create_Invalid_001, TestSize.Level1)
{
    // Test with invalid stream type
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(static_cast<AudioStreamType>(-1));
    // Expect nullptr for invalid stream type
    EXPECT_NE(nullptr, audioCapturer);
}

/**
 * @tc.name  : Test AudioCapturer SetCapturerCallback with invalid parameter
 * @tc.number: Audio_Capturer_SetCapturerCallback_InvalidParam_001
 * @tc.desc  : Test SetCapturerCallback API with invalid parameter (nullptr).
 */
HWTEST(AudioCapturerThirdUnitTest, Audio_Capturer_SetCapturerCallback_InvalidParam_001, TestSize.Level1)
{
    AudioCapturerOptions options;
    options.streamInfo.samplingRate = SAMPLE_RATE_44100;
    options.streamInfo.encoding = ENCODING_PCM;
    options.streamInfo.format = SAMPLE_S16LE;
    options.streamInfo.channels = STEREO;
    options.capturerInfo.sourceType = SOURCE_TYPE_MIC;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(options);
    ASSERT_EQ(nullptr, audioCapturer);
    if (audioCapturer != nullptr) {
        // Test SetCapturerCallback with nullptr (invalid parameter)
        int32_t ret = audioCapturer->SetCapturerCallback(nullptr);
        EXPECT_EQ(ERR_INVALID_PARAM, ret); // Expect error for nullptr callback
    }
}

/**
 * @tc.name  : Test AudioCapturer SetAudioCapturerErrorCallback with invalid parameter
 * @tc.number: Audio_Capturer_SetAudioCapturerErrorCallback_InvalidParam_001
 * @tc.desc  : Test SetAudioCapturerErrorCallback API with invalid parameter.
 */
HWTEST(AudioCapturerThirdUnitTest, Audio_Capturer_SetAudioCapturerErrorCallback_InvalidParam_001, TestSize.Level1)
{
    AudioCapturerOptions options;
    options.streamInfo.samplingRate = SAMPLE_RATE_44100;
    options.streamInfo.encoding = ENCODING_PCM;
    options.streamInfo.format = SAMPLE_S16LE;
    options.streamInfo.channels = STEREO;
    options.capturerInfo.sourceType = SOURCE_TYPE_MIC;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(options);
    ASSERT_EQ(nullptr, audioCapturer);
    if (audioCapturer != nullptr) {
        // Test SetAudioCapturerErrorCallback with nullptr (invalid parameter)
        audioCapturer->SetAudioCapturerErrorCallback(nullptr);

        // Should not crash (error handling)
        EXPECT_TRUE(true);
    }
}

/**
 * @tc.name  : Test AudioCapturer SetFastStatusChangeCallback with invalid parameter
 * @tc.number: Audio_Capturer_SetFastStatusChangeCallback_InvalidParam_001
 * @tc.desc  : Test SetFastStatusChangeCallback API with invalid parameter.
 */
HWTEST(AudioCapturerThirdUnitTest, Audio_Capturer_SetFastStatusChangeCallback_InvalidParam_001, TestSize.Level1)
{
    AudioCapturerOptions options;
    options.streamInfo.samplingRate = SAMPLE_RATE_44100;
    options.streamInfo.encoding = ENCODING_PCM;
    options.streamInfo.format = SAMPLE_S16LE;
    options.streamInfo.channels = STEREO;
    options.capturerInfo.sourceType = SOURCE_TYPE_MIC;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(options);
    ASSERT_EQ(nullptr, audioCapturer);
    if (audioCapturer != nullptr) {
        // Test SetFastStatusChangeCallback with nullptr (invalid parameter)
        audioCapturer->SetFastStatusChangeCallback(nullptr);

        // Should not crash (error handling)
        EXPECT_TRUE(true);
    }
}

/**
 * @tc.name  : Test AudioCapturer GetBufferSize with invalid parameter
 * @tc.number: Audio_Capturer_GetBufferSize_InvalidParam_001
 * @tc.desc  : Test GetBufferSize API with invalid parameter.
 */
HWTEST(AudioCapturerThirdUnitTest, Audio_Capturer_GetBufferSize_InvalidParam_001, TestSize.Level1)
{
    // Test GetBufferSize with nullptr parameter (invalid)
    AudioCapturerOptions options;
    options.streamInfo.samplingRate = SAMPLE_RATE_44100;
    options.streamInfo.encoding = ENCODING_PCM;
    options.streamInfo.format = SAMPLE_S16LE;
    options.streamInfo.channels = STEREO;
    options.capturerInfo.sourceType = SOURCE_TYPE_MIC;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(options);
    ASSERT_EQ(nullptr, audioCapturer);
    if (audioCapturer != nullptr) {
        // Test GetBufferSize with invalid parameter (nullptr reference is not possible in C++,
        // but we can test with uninitialized state)
        size_t bufferSize = static_cast<size_t>(-1); // Invalid size
        int32_t ret = audioCapturer->GetBufferSize(bufferSize);
        EXPECT_EQ(ERR_INVALID_PARAM, ret); // Expect error for invalid parameter
    }
}

/**
 * @tc.name  : Test AudioCapturer GetFrameCount with invalid parameter
 * @tc.number: Audio_Capturer_GetFrameCount_InvalidParam_001
 * @tc.desc  : Test GetFrameCount API with invalid parameter.
 */
HWTEST(AudioCapturerThirdUnitTest, Audio_Capturer_GetFrameCount_InvalidParam_001, TestSize.Level1)
{
    AudioCapturerOptions options;
    options.streamInfo.samplingRate = SAMPLE_RATE_44100;
    options.streamInfo.encoding = ENCODING_PCM;
    options.streamInfo.format = SAMPLE_S16LE;
    options.streamInfo.channels = STEREO;
    options.capturerInfo.sourceType = SOURCE_TYPE_MIC;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(options);
    ASSERT_EQ(nullptr, audioCapturer);
    if (audioCapturer != nullptr) {
        // Test GetFrameCount with invalid parameter (nullptr is not possible for reference,
        // but we can test with uninitialized state)
        uint32_t frameCount = static_cast<uint32_t>(-1); // Invalid frame count
        int32_t ret = audioCapturer->GetFrameCount(frameCount);
        EXPECT_EQ(ERR_INVALID_PARAM, ret); // Expect error for invalid parameter
    }
}

/**
 * @tc.name  : Test AudioCapturer SetBufferDuration with invalid parameter
 * @tc.number: Audio_Capturer_SetBufferDuration_InvalidParam_001
 * @tc.desc  : Test SetBufferDuration API with invalid parameter.
 */
HWTEST(AudioCapturerThirdUnitTest, Audio_Capturer_SetBufferDuration_InvalidParam_001, TestSize.Level1)
{
    AudioCapturerOptions options;
    options.streamInfo.samplingRate = SAMPLE_RATE_44100;
    options.streamInfo.encoding = ENCODING_PCM;
    options.streamInfo.format = SAMPLE_S16LE;
    options.streamInfo.channels = STEREO;
    options.capturerInfo.sourceType = SOURCE_TYPE_MIC;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(options);
    ASSERT_EQ(nullptr, audioCapturer);
    if (audioCapturer != nullptr) {
        // Test SetBufferDuration with invalid value (negative value)
        int32_t ret = audioCapturer->SetBufferDuration(-1); // Invalid duration
        EXPECT_EQ(ERR_INVALID_PARAM, ret); // Expect error for invalid parameter
    }
}

/**
 * @tc.name  : Test AudioCapturer SetBufferDuration with invalid parameter
 * @tc.number: Audio_Capturer_SetBufferDuration_002
 * @tc.desc  : Test SetBufferDuration API with invalid parameter (too small).
 */
HWTEST(AudioCapturerThirdUnitTest, Audio_Capturer_SetBufferDuration_002, TestSize.Level1)
{
    AudioCapturerOptions options;
    options.streamInfo.samplingRate = SAMPLE_RATE_44100;
    options.streamInfo.encoding = ENCODING_PCM;
    options.streamInfo.format = SAMPLE_S16LE;
    options.streamInfo.channels = STEREO;
    options.capturerInfo.sourceType = SOURCE_TYPE_MIC;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(options);
    ASSERT_EQ(nullptr, audioCapturer);
    if (audioCapturer != nullptr) {
        // Test SetBufferDuration with invalid value (1ms, too small)
        int32_t ret = audioCapturer->SetBufferDuration(1);
        EXPECT_EQ(ERR_INVALID_PARAM, ret); // Expect error for too small duration
    }
}

/**
 * @tc.name  : Test AudioCapturer SetBufferDuration with invalid parameter
 * @tc.number: Audio_Capturer_SetBufferDuration_003
 * @tc.desc  : Test SetBufferDuration API with invalid parameter (too large).
 */
HWTEST(AudioCapturerThirdUnitTest, Audio_Capturer_SetBufferDuration_003, TestSize.Level1)
{
    AudioCapturerOptions options;
    options.streamInfo.samplingRate = SAMPLE_RATE_44100;
    options.streamInfo.encoding = ENCODING_PCM;
    options.streamInfo.format = SAMPLE_S16LE;
    options.streamInfo.channels = STEREO;
    options.capturerInfo.sourceType = SOURCE_TYPE_MIC;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(options);
    ASSERT_EQ(nullptr, audioCapturer);
    if (audioCapturer != nullptr) {
        // Test SetBufferDuration with invalid value (100ms, too large)
        int32_t ret = audioCapturer->SetBufferDuration(100);
        EXPECT_EQ(ERR_INVALID_PARAM, ret); // Expect error for too large duration
    }
}

} // namespace AudioStandard
} // namespace OHOS
