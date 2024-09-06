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

#include "renderer_capture_interrupt_unit_test.h"

#include <chrono>
#include <thread>

#include "audio_errors.h"
#include "audio_info.h"
#include "audio_renderer.h"
#include "audio_capturer.h"
#include "audio_capturer_private.h"
#include "audio_renderer_proxy_obj.h"

using namespace std;
using namespace std::chrono;
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
namespace {
constexpr int32_t SLEEP_TIME = 2;
}

InterruptEvent AudioRendererCapturerUnitTest::CaptureinterruptEventTest_ = {};
void AudioRendererCapturerUnitTest::SetUpTestCase(void) {}
void AudioRendererCapturerUnitTest::TearDownTestCase(void) {}
void AudioRendererCapturerUnitTest::SetUp(void) {}
void AudioRendererCapturerUnitTest::TearDown(void) {}

void AudioCapturerCallbackTest::OnInterrupt(const InterruptEvent &interruptEvent)
{
    AudioRendererCapturerUnitTest::CaptureinterruptEventTest_.hintType = interruptEvent.hintType;
}

AudioRendererOptions AudioRendererCapturerUnitTest::UTCreateAudioRenderer(StreamUsage streamUsage)
{
    AudioRendererOptions rendererOptions;
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    rendererOptions.streamInfo.channels = AudioChannel::STEREO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_UNKNOWN;
    rendererOptions.rendererInfo.streamUsage = streamUsage;
    rendererOptions.rendererInfo.rendererFlags = 0;
    return rendererOptions;
}

AudioCapturerOptions AudioRendererCapturerUnitTest::UTCreateAudioCapturer(SourceType sourceType)
{
    AudioCapturerOptions capturerOptions;
    capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_U8;
    capturerOptions.streamInfo.channels = AudioChannel::MONO;
    capturerOptions.capturerInfo.sourceType = sourceType;
    return capturerOptions;
}

void AudioRendererCapturerUnitTest::AudioInterruptUnitTestFunc(StreamUsage streamUsage, SourceType sourceType)
{
    unique_ptr<AudioRenderer> audioRenderer = nullptr;
    unique_ptr<AudioCapturer> audioCapturer = nullptr;

    AudioRendererOptions rendererOptions = UTCreateAudioRenderer(streamUsage);
    audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);
    audioRenderer->SetInterruptMode(INDEPENDENT_MODE);
    bool isRendererStarted = audioRenderer->Start();
    EXPECT_EQ(true, isRendererStarted);

    AudioCapturerOptions capturerOptions = UTCreateAudioCapturer(sourceType);
    audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);
    shared_ptr<AudioCapturerCallbackTest> audioCapturerCB = make_shared<AudioCapturerCallbackTest>();
    int32_t ret = audioCapturer->SetCapturerCallback(audioCapturerCB);
    EXPECT_EQ(SUCCESS, ret);
    bool isCapturerStarted = audioCapturer->Start();
    EXPECT_EQ(true, isCapturerStarted);

    std::this_thread::sleep_for(std::chrono::seconds(SLEEP_TIME));
    EXPECT_EQ(AudioRendererCapturerUnitTest::CaptureinterruptEventTest_.hintType, INTERRUPT_HINT_NONE);
    AudioRendererCapturerUnitTest::CaptureinterruptEventTest_.hintType = INTERRUPT_HINT_NONE;

    audioCapturer->Release();
    audioRenderer->Release();
}

void AudioRendererCapturerUnitTest::AudioInterruptDenyIncomingUnitTestFunc(StreamUsage streamUsage,
    SourceType sourceType)
{
    unique_ptr<AudioRenderer> audioRenderer = nullptr;
    unique_ptr<AudioCapturer> audioCapturer = nullptr;

    AudioRendererOptions rendererOptions = UTCreateAudioRenderer(streamUsage);
    audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);
    audioRenderer->SetInterruptMode(INDEPENDENT_MODE);
    bool isRendererStarted = audioRenderer->Start();
    EXPECT_EQ(true, isRendererStarted);

    AudioCapturerOptions capturerOptions = UTCreateAudioCapturer(sourceType);
    audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);
    shared_ptr<AudioCapturerCallbackTest> audioCapturerCB = make_shared<AudioCapturerCallbackTest>();
    int32_t ret = audioCapturer->SetCapturerCallback(audioCapturerCB);
    EXPECT_EQ(SUCCESS, ret);
    bool isCapturerStarted = audioCapturer->Start();
    EXPECT_EQ(false, isCapturerStarted);

    std::this_thread::sleep_for(std::chrono::seconds(SLEEP_TIME));
    EXPECT_EQ(AudioRendererCapturerUnitTest::CaptureinterruptEventTest_.hintType, INTERRUPT_HINT_STOP);
    AudioRendererCapturerUnitTest::CaptureinterruptEventTest_.hintType = INTERRUPT_HINT_NONE;

    audioCapturer->Release();
    audioRenderer->Release();
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetRendererCaptureInterrupt_001
 * @tc.desc  : When voice message capturer comes after media renderer, play both
 */
HWTEST(AudioRendererCapturerUnitTest, SetRendererCaptureInterrupt_001, TestSize.Level1)
{
    AudioRendererCapturerUnitTest::AudioInterruptUnitTestFunc(STREAM_USAGE_MEDIA, SOURCE_TYPE_VOICE_MESSAGE);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetRendererCaptureInterrupt_002
 * @tc.desc  : When voice message capturer comes after music renderer, play both
 */
HWTEST(AudioRendererCapturerUnitTest, SetRendererCaptureInterrupt_002, TestSize.Level1)
{
    AudioRendererCapturerUnitTest::AudioInterruptUnitTestFunc(STREAM_USAGE_MUSIC, SOURCE_TYPE_VOICE_MESSAGE);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetRendererCaptureInterrupt_003
 * @tc.desc  : When voice message capturer comes after voip renderer, play both
 */
HWTEST(AudioRendererCapturerUnitTest, SetRendererCaptureInterrupt_003, TestSize.Level1)
{
    AudioRendererCapturerUnitTest::AudioInterruptUnitTestFunc(STREAM_USAGE_VOICE_COMMUNICATION,
        SOURCE_TYPE_VOICE_MESSAGE);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetRendererCaptureInterrupt_004
 * @tc.desc  : When voice message capturer comes after voice assistant renderer, play both
 */
HWTEST(AudioRendererCapturerUnitTest, SetRendererCaptureInterrupt_004, TestSize.Level1)
{
    AudioRendererCapturerUnitTest::AudioInterruptUnitTestFunc(STREAM_USAGE_VOICE_ASSISTANT,
        SOURCE_TYPE_VOICE_MESSAGE);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetRendererCaptureInterrupt_005
 * @tc.desc  : When voice message capturer comes after alarm renderer, play both
 */
HWTEST(AudioRendererCapturerUnitTest, SetRendererCaptureInterrupt_005, TestSize.Level1)
{
    AudioRendererCapturerUnitTest::AudioInterruptUnitTestFunc(STREAM_USAGE_ALARM, SOURCE_TYPE_VOICE_MESSAGE);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetRendererCaptureInterrupt_006
 * @tc.desc  : When voice message capturer comes after voice message renderer, play both
 */
HWTEST(AudioRendererCapturerUnitTest, SetRendererCaptureInterrupt_006, TestSize.Level1)
{
    AudioRendererCapturerUnitTest::AudioInterruptUnitTestFunc(STREAM_USAGE_VOICE_MESSAGE, SOURCE_TYPE_VOICE_MESSAGE);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetRendererCaptureInterrupt_007
 * @tc.desc  : When voice message capturer comes after notification ringtone renderer, play both
 */
HWTEST(AudioRendererCapturerUnitTest, SetRendererCaptureInterrupt_007, TestSize.Level1)
{
    AudioRendererCapturerUnitTest::AudioInterruptUnitTestFunc(STREAM_USAGE_NOTIFICATION_RINGTONE,
        SOURCE_TYPE_VOICE_MESSAGE);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetRendererCaptureInterrupt_008
 * @tc.desc  : When voice message capturer comes after ringtone renderer, play both
 */
HWTEST(AudioRendererCapturerUnitTest, SetRendererCaptureInterrupt_008, TestSize.Level1)
{
    AudioRendererCapturerUnitTest::AudioInterruptUnitTestFunc(STREAM_USAGE_RINGTONE, SOURCE_TYPE_VOICE_MESSAGE);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetRendererCaptureInterrupt_009
 * @tc.desc  : When voice message capturer comes after notification renderer, play both
 */
HWTEST(AudioRendererCapturerUnitTest, SetRendererCaptureInterrupt_009, TestSize.Level1)
{
    AudioRendererCapturerUnitTest::AudioInterruptUnitTestFunc(STREAM_USAGE_NOTIFICATION, SOURCE_TYPE_VOICE_MESSAGE);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetRendererCaptureInterrupt_010
 * @tc.desc  : When voice message capturer comes after accessibility renderer, play both
 */
HWTEST(AudioRendererCapturerUnitTest, SetRendererCaptureInterrupt_010, TestSize.Level1)
{
    AudioRendererCapturerUnitTest::AudioInterruptUnitTestFunc(STREAM_USAGE_ACCESSIBILITY, SOURCE_TYPE_VOICE_MESSAGE);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetRendererCaptureInterrupt_011
 * @tc.desc  : When voice message capturer comes after system renderer, play both
 */
HWTEST(AudioRendererCapturerUnitTest, SetRendererCaptureInterrupt_011, TestSize.Level1)
{
    AudioRendererCapturerUnitTest::AudioInterruptUnitTestFunc(STREAM_USAGE_SYSTEM, SOURCE_TYPE_VOICE_MESSAGE);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetRendererCaptureInterrupt_012
 * @tc.desc  : When voice message capturer comes after movie renderer, play both
 */
HWTEST(AudioRendererCapturerUnitTest, SetRendererCaptureInterrupt_012, TestSize.Level1)
{
    AudioRendererCapturerUnitTest::AudioInterruptUnitTestFunc(STREAM_USAGE_MOVIE, SOURCE_TYPE_VOICE_MESSAGE);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetRendererCaptureInterrupt_013
 * @tc.desc  : When voice message capturer comes after game renderer, play both
 */
HWTEST(AudioRendererCapturerUnitTest, SetRendererCaptureInterrupt_013, TestSize.Level1)
{
    AudioRendererCapturerUnitTest::AudioInterruptUnitTestFunc(STREAM_USAGE_GAME, SOURCE_TYPE_VOICE_MESSAGE);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetRendererCaptureInterrupt_014
 * @tc.desc  : When voice message capturer comes after audiobook renderer, play both
 */
HWTEST(AudioRendererCapturerUnitTest, SetRendererCaptureInterrupt_014, TestSize.Level1)
{
    AudioRendererCapturerUnitTest::AudioInterruptUnitTestFunc(STREAM_USAGE_AUDIOBOOK, SOURCE_TYPE_VOICE_MESSAGE);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetRendererCaptureInterrupt_015
 * @tc.desc  : When voice message capturer comes after navigation renderer, play both
 */
HWTEST(AudioRendererCapturerUnitTest, SetRendererCaptureInterrupt_015, TestSize.Level1)
{
    AudioRendererCapturerUnitTest::AudioInterruptUnitTestFunc(STREAM_USAGE_NAVIGATION, SOURCE_TYPE_VOICE_MESSAGE);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetRendererCaptureInterrupt_016
 * @tc.desc  : When voice message capturer comes after dtmf renderer, play both
 */
HWTEST(AudioRendererCapturerUnitTest, SetRendererCaptureInterrupt_016, TestSize.Level1)
{
    AudioRendererCapturerUnitTest::AudioInterruptUnitTestFunc(STREAM_USAGE_DTMF, SOURCE_TYPE_VOICE_MESSAGE);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetRendererCaptureInterrupt_017
 * @tc.desc  : When voice message capturer comes after enforced tone renderer, play both
 */
HWTEST(AudioRendererCapturerUnitTest, SetRendererCaptureInterrupt_017, TestSize.Level1)
{
    AudioRendererCapturerUnitTest::AudioInterruptUnitTestFunc(STREAM_USAGE_ENFORCED_TONE, SOURCE_TYPE_VOICE_MESSAGE);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetRendererCaptureInterrupt_018
 * @tc.desc  : When voice message capturer comes after video communication renderer, play both
 */
HWTEST(AudioRendererCapturerUnitTest, SetRendererCaptureInterrupt_018, TestSize.Level1)
{
    AudioRendererCapturerUnitTest::AudioInterruptUnitTestFunc(STREAM_USAGE_VIDEO_COMMUNICATION,
        SOURCE_TYPE_VOICE_MESSAGE);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetRendererCaptureInterrupt_019
 * @tc.desc  : When voice message capturer comes after ringing renderer, play both
 */
HWTEST(AudioRendererCapturerUnitTest, SetRendererCaptureInterrupt_019, TestSize.Level1)
{
    AudioRendererCapturerUnitTest::AudioInterruptUnitTestFunc(STREAM_USAGE_RANGING, SOURCE_TYPE_VOICE_MESSAGE);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetRendererCaptureInterrupt_020
 * @tc.desc  : When voice message capturer comes after voice call renderer, play both
 */
HWTEST(AudioRendererCapturerUnitTest, SetRendererCaptureInterrupt_020, TestSize.Level1)
{
    AudioRendererCapturerUnitTest::AudioInterruptDenyIncomingUnitTestFunc(STREAM_USAGE_VOICE_MODEM_COMMUNICATION,
        SOURCE_TYPE_VOICE_MESSAGE);
}
} // namespace AudioStandard
} // namespace OHOS