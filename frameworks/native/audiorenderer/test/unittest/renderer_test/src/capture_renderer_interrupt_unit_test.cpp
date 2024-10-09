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

#include "capture_renderer_interrupt_unit_test.h"

#include <chrono>
#include <thread>

#include "audio_errors.h"
#include "audio_info.h"
#include "audio_renderer.h"
#include "audio_capturer.h"
#include "audio_renderer_private.h"
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

InterruptEvent AudioCaptureRendererUnitTest::RendererinterruptEventTest_ = {};
void AudioCaptureRendererUnitTest::SetUpTestCase(void) {}
void AudioCaptureRendererUnitTest::TearDownTestCase(void) {}
void AudioCaptureRendererUnitTest::SetUp(void) {}
void AudioCaptureRendererUnitTest::TearDown(void) {}

void AudioRendererCallbackTest::OnInterrupt(const InterruptEvent &interruptEvent)
{
    AudioCaptureRendererUnitTest::RendererinterruptEventTest_.hintType = interruptEvent.hintType;
}

AudioRendererOptions AudioCaptureRendererUnitTest::UTCreateAudioRenderer(StreamUsage streamUsage)
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

AudioCapturerOptions AudioCaptureRendererUnitTest::UTCreateAudioCapturer(SourceType sourceType)
{
    AudioCapturerOptions capturerOptions;
    capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_U8;
    capturerOptions.streamInfo.channels = AudioChannel::MONO;
    capturerOptions.capturerInfo.sourceType = sourceType;
    return capturerOptions;
}

void AudioCaptureRendererUnitTest::AudioInterruptUnitTestFunc(SourceType sourceType, StreamUsage streamUsage)
{
    unique_ptr<AudioRenderer> audioRenderer = nullptr;
    unique_ptr<AudioCapturer> audioCapturer = nullptr;

    AudioCapturerOptions capturerOptions = UTCreateAudioCapturer(sourceType);
    audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);
    bool isCapturerStarted = audioCapturer->Start();
    EXPECT_EQ(true, isCapturerStarted);

    AudioRendererOptions rendererOptions = UTCreateAudioRenderer(streamUsage);
    audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);
    audioRenderer->SetInterruptMode(INDEPENDENT_MODE);
    shared_ptr<AudioRendererCallbackTest> audioCapturerCB = make_shared<AudioRendererCallbackTest>();
    int32_t ret = audioRenderer->SetRendererCallback(audioCapturerCB);
    EXPECT_EQ(SUCCESS, ret);
    bool isRendererStarted = audioRenderer->Start();
    EXPECT_EQ(true, isRendererStarted);

    std::this_thread::sleep_for(std::chrono::seconds(SLEEP_TIME));
    EXPECT_EQ(AudioCaptureRendererUnitTest::RendererinterruptEventTest_.hintType, INTERRUPT_HINT_NONE);
    AudioCaptureRendererUnitTest::RendererinterruptEventTest_.hintType = INTERRUPT_HINT_NONE;

    audioCapturer->Release();
    audioRenderer->Release();
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetRendererCaptureInterrupt_001
 * @tc.desc  : When media renderer comes after voice message capturer, play both
 */
HWTEST(AudioCaptureRendererUnitTest, SetRendererCaptureInterrupt_001, TestSize.Level1)
{
    AudioCaptureRendererUnitTest::AudioInterruptUnitTestFunc(SOURCE_TYPE_VOICE_MESSAGE, STREAM_USAGE_MEDIA);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetRendererCaptureInterrupt_002
 * @tc.desc  : When music renderer comes after voice message capturer, play both
 */
HWTEST(AudioCaptureRendererUnitTest, SetRendererCaptureInterrupt_002, TestSize.Level1)
{
    AudioCaptureRendererUnitTest::AudioInterruptUnitTestFunc(SOURCE_TYPE_VOICE_MESSAGE, STREAM_USAGE_MUSIC);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetRendererCaptureInterrupt_003
 * @tc.desc  : When voip renderer comes after voice message capturer, play both
 */
HWTEST(AudioCaptureRendererUnitTest, SetRendererCaptureInterrupt_003, TestSize.Level1)
{
    AudioCaptureRendererUnitTest::AudioInterruptUnitTestFunc(SOURCE_TYPE_VOICE_MESSAGE,
        STREAM_USAGE_VOICE_COMMUNICATION);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetRendererCaptureInterrupt_004
 * @tc.desc  : When voice assistant renderer comes after voice message capturer, play both
 */
HWTEST(AudioCaptureRendererUnitTest, SetRendererCaptureInterrupt_004, TestSize.Level1)
{
    AudioCaptureRendererUnitTest::AudioInterruptUnitTestFunc(SOURCE_TYPE_VOICE_MESSAGE, STREAM_USAGE_VOICE_ASSISTANT);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetRendererCaptureInterrupt_005
 * @tc.desc  : When alarm renderer comes after voice message capturer, play both
 */
HWTEST(AudioCaptureRendererUnitTest, SetRendererCaptureInterrupt_005, TestSize.Level1)
{
    AudioCaptureRendererUnitTest::AudioInterruptUnitTestFunc(SOURCE_TYPE_VOICE_MESSAGE, STREAM_USAGE_ALARM);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetRendererCaptureInterrupt_006
 * @tc.desc  : When voice message renderer comes after voice message capturer, play both
 */
HWTEST(AudioCaptureRendererUnitTest, SetRendererCaptureInterrupt_006, TestSize.Level1)
{
    AudioCaptureRendererUnitTest::AudioInterruptUnitTestFunc(SOURCE_TYPE_VOICE_MESSAGE, STREAM_USAGE_VOICE_MESSAGE);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetRendererCaptureInterrupt_007
 * @tc.desc  : When notification ringtone renderer comes after voice message capturer, play both
 */
HWTEST(AudioCaptureRendererUnitTest, SetRendererCaptureInterrupt_007, TestSize.Level1)
{
    AudioCaptureRendererUnitTest::AudioInterruptUnitTestFunc(SOURCE_TYPE_VOICE_MESSAGE,
        STREAM_USAGE_NOTIFICATION_RINGTONE);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetRendererCaptureInterrupt_008
 * @tc.desc  : When ringtone renderer comes after voice message capturer, play both
 */
HWTEST(AudioCaptureRendererUnitTest, SetRendererCaptureInterrupt_008, TestSize.Level1)
{
    AudioCaptureRendererUnitTest::AudioInterruptUnitTestFunc(SOURCE_TYPE_VOICE_MESSAGE, STREAM_USAGE_RINGTONE);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetRendererCaptureInterrupt_009
 * @tc.desc  : When notification renderer comes after voice message capturer, play both
 */
HWTEST(AudioCaptureRendererUnitTest, SetRendererCaptureInterrupt_009, TestSize.Level1)
{
    AudioCaptureRendererUnitTest::AudioInterruptUnitTestFunc(SOURCE_TYPE_VOICE_MESSAGE, STREAM_USAGE_NOTIFICATION);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetRendererCaptureInterrupt_010
 * @tc.desc  : When accessibility renderer comes after voice message capturer, play both
 */
HWTEST(AudioCaptureRendererUnitTest, SetRendererCaptureInterrupt_010, TestSize.Level1)
{
    AudioCaptureRendererUnitTest::AudioInterruptUnitTestFunc(SOURCE_TYPE_VOICE_MESSAGE, STREAM_USAGE_ACCESSIBILITY);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetRendererCaptureInterrupt_011
 * @tc.desc  : When system renderer comes after voice message capturer, play both
 */
HWTEST(AudioCaptureRendererUnitTest, SetRendererCaptureInterrupt_011, TestSize.Level1)
{
    AudioCaptureRendererUnitTest::AudioInterruptUnitTestFunc(SOURCE_TYPE_VOICE_MESSAGE, STREAM_USAGE_SYSTEM);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetRendererCaptureInterrupt_012
 * @tc.desc  : When movie renderer comes after voice message capturer, play both
 */
HWTEST(AudioCaptureRendererUnitTest, SetRendererCaptureInterrupt_012, TestSize.Level1)
{
    AudioCaptureRendererUnitTest::AudioInterruptUnitTestFunc(SOURCE_TYPE_VOICE_MESSAGE, STREAM_USAGE_MOVIE);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetRendererCaptureInterrupt_013
 * @tc.desc  : When game renderer comes after voice message capturer, play both
 */
HWTEST(AudioCaptureRendererUnitTest, SetRendererCaptureInterrupt_013, TestSize.Level1)
{
    AudioCaptureRendererUnitTest::AudioInterruptUnitTestFunc(SOURCE_TYPE_VOICE_MESSAGE, STREAM_USAGE_GAME);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetRendererCaptureInterrupt_014
 * @tc.desc  : When audiobook renderer comes after voice message capturer, play both
 */
HWTEST(AudioCaptureRendererUnitTest, SetRendererCaptureInterrupt_014, TestSize.Level1)
{
    AudioCaptureRendererUnitTest::AudioInterruptUnitTestFunc(SOURCE_TYPE_VOICE_MESSAGE, STREAM_USAGE_AUDIOBOOK);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetRendererCaptureInterrupt_015
 * @tc.desc  : When navigation renderer comes after voice message capturer, play both
 */
HWTEST(AudioCaptureRendererUnitTest, SetRendererCaptureInterrupt_015, TestSize.Level1)
{
    AudioCaptureRendererUnitTest::AudioInterruptUnitTestFunc(SOURCE_TYPE_VOICE_MESSAGE, STREAM_USAGE_NAVIGATION);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetRendererCaptureInterrupt_016
 * @tc.desc  : When dtmf renderer comes after voice message capturer, play both
 */
HWTEST(AudioCaptureRendererUnitTest, SetRendererCaptureInterrupt_016, TestSize.Level1)
{
    AudioCaptureRendererUnitTest::AudioInterruptUnitTestFunc(SOURCE_TYPE_VOICE_MESSAGE, STREAM_USAGE_DTMF);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetRendererCaptureInterrupt_017
 * @tc.desc  : When enforced tone renderer comes after voice message capturer, play both
 */
HWTEST(AudioCaptureRendererUnitTest, SetRendererCaptureInterrupt_017, TestSize.Level1)
{
    AudioCaptureRendererUnitTest::AudioInterruptUnitTestFunc(SOURCE_TYPE_VOICE_MESSAGE, STREAM_USAGE_ENFORCED_TONE);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetRendererCaptureInterrupt_018
 * @tc.desc  : When video communication renderer comes after voice message capturer, play both
 */
HWTEST(AudioCaptureRendererUnitTest, SetRendererCaptureInterrupt_018, TestSize.Level1)
{
    AudioCaptureRendererUnitTest::AudioInterruptUnitTestFunc(SOURCE_TYPE_VOICE_MESSAGE,
        STREAM_USAGE_VIDEO_COMMUNICATION);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetRendererCaptureInterrupt_019
 * @tc.desc  : When ringing renderer comes after voice message capturer, play both
 */
HWTEST(AudioCaptureRendererUnitTest, SetRendererCaptureInterrupt_019, TestSize.Level1)
{
    AudioCaptureRendererUnitTest::AudioInterruptUnitTestFunc(SOURCE_TYPE_VOICE_MESSAGE, STREAM_USAGE_RANGING);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetRendererCaptureInterrupt_020
 * @tc.desc  : When voice call renderer comes after voice message capturer, play both
 */
HWTEST(AudioCaptureRendererUnitTest, SetRendererCaptureInterrupt_020, TestSize.Level1)
{
    AudioCaptureRendererUnitTest::AudioInterruptUnitTestFunc(SOURCE_TYPE_VOICE_MESSAGE,
        STREAM_USAGE_VOICE_MODEM_COMMUNICATION);
}
} // namespace AudioStandard
} // namespace OHOS