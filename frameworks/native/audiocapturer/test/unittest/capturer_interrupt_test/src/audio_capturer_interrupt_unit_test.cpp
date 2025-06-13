/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "audio_capturer_interrupt_unit_test.h"

#include <thread>

#include "audio_capturer.h"
#include "audio_errors.h"
#include "audio_info.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace {
    enum Index {
        MUSIC = 0,
        VOICE_CALL,
        RING,
        VOICE_ASSISTANT,
        ULTRASONIC,
        ALARM,
        ACCESSIBILITY,
        SPEECH,
        MOVIE,
        UNKNOW,
    };
    const int32_t TOTAL_RENDER_INFO = 10;
    AudioRendererInfo dRenderInfo = {CONTENT_TYPE_UNKNOWN, STREAM_USAGE_UNKNOWN, 0};

    AudioRendererInfo renderInfo[TOTAL_RENDER_INFO] = {
        {ContentType::CONTENT_TYPE_MUSIC, StreamUsage::STREAM_USAGE_MEDIA},                 // 0: music
        {ContentType::CONTENT_TYPE_SPEECH, StreamUsage::STREAM_USAGE_VOICE_COMMUNICATION},  // 1: VoiceCall
        {ContentType::CONTENT_TYPE_MUSIC, StreamUsage::STREAM_USAGE_NOTIFICATION_RINGTONE}, // 2: ring
        {ContentType::CONTENT_TYPE_SPEECH, StreamUsage::STREAM_USAGE_VOICE_ASSISTANT},      // 3: VoiceAssistant
        {ContentType::CONTENT_TYPE_ULTRASONIC, StreamUsage::STREAM_USAGE_SYSTEM},           // 4: Ultrasonic
        {ContentType::CONTENT_TYPE_MUSIC, StreamUsage::STREAM_USAGE_ALARM},                 // 5: Alarm
        {ContentType::CONTENT_TYPE_SPEECH, StreamUsage::STREAM_USAGE_ACCESSIBILITY},        // 6: Accessibility
        {ContentType::CONTENT_TYPE_SPEECH, StreamUsage::STREAM_USAGE_MEDIA},                // 7: Speech
        {ContentType::CONTENT_TYPE_MOVIE, StreamUsage::STREAM_USAGE_MEDIA},                 // 8: Movie
        {ContentType::CONTENT_TYPE_UNKNOWN, StreamUsage::STREAM_USAGE_UNKNOWN}              // 9: Unknow
    };
} // namespace

void AudioCapturerInterruptUnitTest::SetUpTestCase(void) {}
void AudioCapturerInterruptUnitTest::TearDownTestCase(void) {}
void AudioCapturerInterruptUnitTest::SetUp(void) {}
void AudioCapturerInterruptUnitTest::TearDown(void) {}

void AudioCapturerInterruptUnitTest::UTCreateAudioCapture(unique_ptr<AudioCapturer> &audioCapturer,
    SourceType sourceType)
{
    AudioCapturerOptions capturerOptions;
    capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_U8;
    capturerOptions.streamInfo.channels = AudioChannel::MONO;
    capturerOptions.capturerInfo.sourceType = sourceType;
    audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    shared_ptr<AudioCapturerCallback> cb = make_shared<AudioCapturerInterruptCallbackTest>();
    int32_t ret = audioCapturer->SetCapturerCallback(cb);
    EXPECT_EQ(SUCCESS, ret);
    return ;
}

void AudioCapturerInterruptUnitTest::UTCreateAudioRender(unique_ptr<AudioRenderer> &audioRenderer,
    AudioRendererInfo renderInfo)
{
    AudioRendererOptions rendererOptions;
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    rendererOptions.streamInfo.channels = AudioChannel::STEREO;
    rendererOptions.rendererInfo = renderInfo;
    audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    shared_ptr<AudioRendererCallback> cb = make_shared<AudioRendererInterruptCallbackTest>();
    int32_t ret = audioRenderer->SetRendererCallback(cb);
    EXPECT_EQ(SUCCESS, ret);
    return;
}

void AudioCapturerInterruptUnitTest::AudioInterruptUnitTestFunc(TestType tType, bool bothRun, SourceType sourceTypte1,
    SourceType sourceTypte2, AudioRendererInfo renderInfo)
{
    unique_ptr<AudioCapturer> audioCapturer = nullptr;
    unique_ptr<AudioCapturer> audioCapturerSec = nullptr;
    unique_ptr<AudioRenderer> audioRenderer = nullptr;

    UTCreateAudioCapture(audioCapturer, sourceTypte1);
    ASSERT_NE(nullptr, audioCapturer);

    if (tType == CAPTURE_CAPTURE) {
        bool isStarted1 = audioCapturer->Start();
        EXPECT_EQ(true, isStarted1);

        UTCreateAudioCapture(audioCapturerSec, sourceTypte2);
        ASSERT_NE(nullptr, audioCapturerSec);
        bool isStarted2 = audioCapturerSec->Start();
        if (bothRun == false) {
            EXPECT_EQ(false, isStarted2);
        } else {
            EXPECT_EQ(true, isStarted2);
            EXPECT_EQ(CAPTURER_RUNNING, audioCapturer->GetStatus());
            EXPECT_EQ(CAPTURER_RUNNING, audioCapturerSec->GetStatus());
        }
        audioCapturer->Release();
        audioCapturerSec->Release();
    } else if (tType == CAPTURE_RANDER) {
        bool isStarted1 = audioCapturer->Start();
        EXPECT_EQ(true, isStarted1);

        UTCreateAudioRender(audioRenderer, renderInfo);
        bool isStarted2 = audioRenderer->Start();
        EXPECT_EQ(true, isStarted2);

        EXPECT_EQ(CAPTURER_RUNNING, audioCapturer->GetStatus());
        EXPECT_EQ(RENDERER_RUNNING, audioRenderer->GetStatus());
        audioCapturer->Release();
        audioRenderer->Release();
    } else {
        UTCreateAudioRender(audioRenderer, renderInfo);
        bool isStarted1 = audioRenderer->Start();
        EXPECT_EQ(true, isStarted1);

        bool isStarted2 = audioCapturer->Start();
        EXPECT_EQ(true, isStarted2);
        EXPECT_EQ(CAPTURER_RUNNING, audioCapturer->GetStatus());
        EXPECT_EQ(RENDERER_RUNNING, audioRenderer->GetStatus());
        audioCapturer->Release();
        audioRenderer->Release();
    }
    return;
}

#ifdef TEMP_DISABLE
/**
* @tc.name  : Test AudioCapturer Interrupt.
* @tc.number: Audio_Capturer_Interrupt_001
* @tc.desc  : Create a MIC source recording first, and then create a ULTRASONIC source recording.
*             It is expected that the tow source both running.
*/
HWTEST(AudioCapturerInterruptUnitTest, Audio_Capturer_Interrupt_001, TestSize.Level1)
{
    AudioCapturerInterruptUnitTest::AudioInterruptUnitTestFunc(TestType::CAPTURE_CAPTURE, true,
        SOURCE_TYPE_MIC, SOURCE_TYPE_ULTRASONIC, dRenderInfo);
}
#endif

/**
* @tc.name  : Test AudioCapturer Interrupt.
* @tc.number: Audio_Capturer_Interrupt_002
* @tc.desc  : Create a ULTRASONIC source recording first, and then create a MIC source recording.
*             It is expected that the tow source both running.
*/
HWTEST(AudioCapturerInterruptUnitTest, Audio_Capturer_Interrupt_002, TestSize.Level1)
{
    AudioCapturerInterruptUnitTest::AudioInterruptUnitTestFunc(TestType::CAPTURE_CAPTURE, true,
        SOURCE_TYPE_ULTRASONIC, SOURCE_TYPE_MIC, dRenderInfo);
}

/**
* @tc.name  : Test AudioCapturer Interrupt.
* @tc.number: Audio_Capturer_Interrupt_003
* @tc.desc  : Create a VOICE_RECOGNITION source recording first, and then create a ULTRASONIC source recording.
*             It is expected that the tow source both running.
*/
HWTEST(AudioCapturerInterruptUnitTest, Audio_Capturer_Interrupt_003, TestSize.Level1)
{
    AudioCapturerInterruptUnitTest::AudioInterruptUnitTestFunc(TestType::CAPTURE_CAPTURE, true,
        SOURCE_TYPE_VOICE_RECOGNITION, SOURCE_TYPE_ULTRASONIC, dRenderInfo);
}

/**
* @tc.name  : Test AudioCapturer Interrupt.
* @tc.number: Audio_Capturer_Interrupt_004
* @tc.desc  : Create a ULTRASONIC source recording first, and then create a VOICE_RECOGNITION source recording.
*             It is expected that the tow source both running.
*/
HWTEST(AudioCapturerInterruptUnitTest, Audio_Capturer_Interrupt_004, TestSize.Level1)
{
    AudioCapturerInterruptUnitTest::AudioInterruptUnitTestFunc(TestType::CAPTURE_CAPTURE, true,
        SOURCE_TYPE_ULTRASONIC, SOURCE_TYPE_VOICE_RECOGNITION, dRenderInfo);
}

#ifdef TEMP_DISABLE
/**
* @tc.name  : Test AudioCapturer Interrupt.
* @tc.number: Audio_Capturer_Interrupt_005
* @tc.desc  : Create a VOICE_COMMUNICATION source recording first, and then create a ULTRASONIC source recording.
*             It is expected that the tow source both running.
*/
HWTEST(AudioCapturerInterruptUnitTest, Audio_Capturer_Interrupt_005, TestSize.Level1)
{
    AudioCapturerInterruptUnitTest::AudioInterruptUnitTestFunc(TestType::CAPTURE_CAPTURE, true,
        SOURCE_TYPE_VOICE_COMMUNICATION, SOURCE_TYPE_ULTRASONIC, dRenderInfo);
}

/**
* @tc.name  : Test AudioCapturer Interrupt.
* @tc.number: Audio_Capturer_Interrupt_006
* @tc.desc  : Create a ULTRASONIC source recording first, and then create a VOICE_COMMUNICATION source recording.
*             It is expected that the tow source both running.
*/
HWTEST(AudioCapturerInterruptUnitTest, Audio_Capturer_Interrupt_006, TestSize.Level1)
{
    AudioCapturerInterruptUnitTest::AudioInterruptUnitTestFunc(TestType::CAPTURE_CAPTURE, true,
        SOURCE_TYPE_ULTRASONIC, SOURCE_TYPE_VOICE_COMMUNICATION, dRenderInfo);
}

/**
* @tc.name  : Test AudioCapturer Interrupt.
* @tc.number: Audio_Capturer_Interrupt_007
* @tc.desc  : Create a ULTRASONIC source recording first, and then create a ULTRASONIC source recording.
*             It is expected that the second ULTRASONIC source will be rejected.
*/
HWTEST(AudioCapturerInterruptUnitTest, Audio_Capturer_Interrupt_007, TestSize.Level1)
{
    AudioCapturerInterruptUnitTest::AudioInterruptUnitTestFunc(TestType::CAPTURE_CAPTURE, false,
        SOURCE_TYPE_ULTRASONIC, SOURCE_TYPE_ULTRASONIC, dRenderInfo);
}

/**
* @tc.name  : Test AudioCapturer Interrupt.
* @tc.number: Audio_Capturer_Interrupt_008
* @tc.desc  : Create a ULTRASONIC source recording first, then create a Music stream type playback.
*             It is expected that capturer and renderer both running at the same time.
*/
HWTEST(AudioCapturerInterruptUnitTest, Audio_Capturer_Interrupt_008, TestSize.Level1)
{
    AudioCapturerInterruptUnitTest::AudioInterruptUnitTestFunc(TestType::CAPTURE_RANDER, true,
        SOURCE_TYPE_ULTRASONIC, SOURCE_TYPE_INVALID, renderInfo[MUSIC]);
}

/**
* @tc.name  : Test AudioCapturer Interrupt.
* @tc.number: Audio_Capturer_Interrupt_009
* @tc.desc  : Create a Music stream type playback first, then create a ULTRASONIC source recording .
*             It is expected that renderer and capturer both running at the same time.
*/
HWTEST(AudioCapturerInterruptUnitTest, Audio_Capturer_Interrupt_009, TestSize.Level1)
{
    AudioCapturerInterruptUnitTest::AudioInterruptUnitTestFunc(TestType::RANDER_CAPTURE, true,
        SOURCE_TYPE_ULTRASONIC, SOURCE_TYPE_INVALID, renderInfo[MUSIC]);
}

/**
* @tc.name  : Test AudioCapturer Interrupt.
* @tc.number: Audio_Capturer_Interrupt_010
* @tc.desc  : Create a ULTRASONIC source recording first, then create a VoiceCall stream type playback.
*             It is expected that capturer and renderer both running at the same time.
*/
HWTEST(AudioCapturerInterruptUnitTest, Audio_Capturer_Interrupt_010, TestSize.Level1)
{
    AudioCapturerInterruptUnitTest::AudioInterruptUnitTestFunc(TestType::CAPTURE_RANDER, true,
        SOURCE_TYPE_ULTRASONIC, SOURCE_TYPE_INVALID, renderInfo[VOICE_CALL]);
}

/**
* @tc.name  : Test AudioCapturer Interrupt.
* @tc.number: Audio_Capturer_Interrupt_011
* @tc.desc  : Create a VoiceCall stream type playback first, then create a ULTRASONIC source recording .
*             It is expected that renderer and capturer both running at the same time.
*/
HWTEST(AudioCapturerInterruptUnitTest, Audio_Capturer_Interrupt_011, TestSize.Level1)
{
    AudioCapturerInterruptUnitTest::AudioInterruptUnitTestFunc(TestType::RANDER_CAPTURE, true,
        SOURCE_TYPE_ULTRASONIC, SOURCE_TYPE_INVALID, renderInfo[VOICE_CALL]);
}

/**
* @tc.name  : Test AudioCapturer Interrupt.
* @tc.number: Audio_Capturer_Interrupt_012
* @tc.desc  : Create a ULTRASONIC source recording first, then create a Ring stream type playback.
*             It is expected that capturer and renderer both running at the same time.
*/
HWTEST(AudioCapturerInterruptUnitTest, Audio_Capturer_Interrupt_012, TestSize.Level1)
{
    AudioCapturerInterruptUnitTest::AudioInterruptUnitTestFunc(TestType::CAPTURE_RANDER, true,
        SOURCE_TYPE_ULTRASONIC, SOURCE_TYPE_INVALID, renderInfo[RING]);
}

/**
* @tc.name  : Test AudioCapturer Interrupt.
* @tc.number: Audio_Capturer_Interrupt_013
* @tc.desc  : Create a Ring stream type playback first, then create a ULTRASONIC source recording.
*             It is expected that renderer and capturer both running at the same time.
*/
HWTEST(AudioCapturerInterruptUnitTest, Audio_Capturer_Interrupt_013, TestSize.Level1)
{
    AudioCapturerInterruptUnitTest::AudioInterruptUnitTestFunc(TestType::RANDER_CAPTURE, true,
        SOURCE_TYPE_ULTRASONIC, SOURCE_TYPE_INVALID, renderInfo[RING]);
}

/**
* @tc.name  : Test AudioCapturer Interrupt.
* @tc.number: Audio_Capturer_Interrupt_014
* @tc.desc  : Create a ULTRASONIC source recording first, then create a VoiceAssistant stream type playback.
*             It is expected that capturer and renderer both running at the same time.
*/
HWTEST(AudioCapturerInterruptUnitTest, Audio_Capturer_Interrupt_014, TestSize.Level1)
{
    AudioCapturerInterruptUnitTest::AudioInterruptUnitTestFunc(TestType::CAPTURE_RANDER, true,
        SOURCE_TYPE_ULTRASONIC, SOURCE_TYPE_INVALID, renderInfo[VOICE_ASSISTANT]);
}

/**
* @tc.name  : Test AudioCapturer Interrupt.
* @tc.number: Audio_Capturer_Interrupt_015
* @tc.desc  : Create a VoiceAssistant stream type playback first, then create a ULTRASONIC source recording .
*             It is expected that renderer and capturer both running at the same time.
*/
HWTEST(AudioCapturerInterruptUnitTest, Audio_Capturer_Interrupt_015, TestSize.Level1)
{
    AudioCapturerInterruptUnitTest::AudioInterruptUnitTestFunc(TestType::RANDER_CAPTURE, true,
        SOURCE_TYPE_ULTRASONIC, SOURCE_TYPE_INVALID, renderInfo[VOICE_ASSISTANT]);
}

/**
* @tc.name  : Test AudioCapturer Interrupt.
* @tc.number: Audio_Capturer_Interrupt_018
* @tc.desc  : Create a ULTRASONIC source recording first, then create a Alarm stream type playback.
*             It is expected that capturer and renderer both running at the same time.
*/
HWTEST(AudioCapturerInterruptUnitTest, Audio_Capturer_Interrupt_018, TestSize.Level1)
{
    AudioCapturerInterruptUnitTest::AudioInterruptUnitTestFunc(TestType::CAPTURE_RANDER, true,
        SOURCE_TYPE_ULTRASONIC, SOURCE_TYPE_INVALID, renderInfo[ALARM]);
}

/**
* @tc.name  : Test AudioCapturer Interrupt.
* @tc.number: Audio_Capturer_Interrupt_019
* @tc.desc  : Create a Alarm stream type playback first, then create a ULTRASONIC source recording.
*             It is expected that renderer and capturer both running at the same time.
*/
HWTEST(AudioCapturerInterruptUnitTest, Audio_Capturer_Interrupt_019, TestSize.Level1)
{
    AudioCapturerInterruptUnitTest::AudioInterruptUnitTestFunc(TestType::RANDER_CAPTURE, true,
        SOURCE_TYPE_ULTRASONIC, SOURCE_TYPE_INVALID, renderInfo[ALARM]);
}

/**
* @tc.name  : Test AudioCapturer Interrupt.
* @tc.number: Audio_Capturer_Interrupt_014
* @tc.desc  : Create a ULTRASONIC source recording first, then create a Accessibility stream type playback.
*             It is expected that capturer and renderer both running at the same time.
*/
HWTEST(AudioCapturerInterruptUnitTest, Audio_Capturer_Interrupt_020, TestSize.Level1)
{
    AudioCapturerInterruptUnitTest::AudioInterruptUnitTestFunc(TestType::CAPTURE_RANDER, true,
        SOURCE_TYPE_ULTRASONIC, SOURCE_TYPE_INVALID, renderInfo[ACCESSIBILITY]);
}
#endif

/**
* @tc.name  : Test AudioCapturer Interrupt.
* @tc.number: Audio_Capturer_Interrupt_021
* @tc.desc  : Create a Accessibility stream type playback first, then create a ULTRASONIC source recording.
*             It is expected that renderer and capturer both running at the same time.
*/
HWTEST(AudioCapturerInterruptUnitTest, Audio_Capturer_Interrupt_021, TestSize.Level1)
{
    AudioCapturerInterruptUnitTest::AudioInterruptUnitTestFunc(TestType::RANDER_CAPTURE, true,
        SOURCE_TYPE_ULTRASONIC, SOURCE_TYPE_INVALID, renderInfo[ACCESSIBILITY]);
}

/**
* @tc.name  : Test AudioCapturer Interrupt.
* @tc.number: Audio_Capturer_Interrupt_022
* @tc.desc  : Create a ULTRASONIC source recording first, then create a Speech stream type playback.
*             It is expected that capturer and renderer both running at the same time.
*/
HWTEST(AudioCapturerInterruptUnitTest, Audio_Capturer_Interrupt_022, TestSize.Level1)
{
    AudioCapturerInterruptUnitTest::AudioInterruptUnitTestFunc(TestType::CAPTURE_RANDER, true,
        SOURCE_TYPE_ULTRASONIC, SOURCE_TYPE_INVALID, renderInfo[SPEECH]);
}

/**
* @tc.name  : Test AudioCapturer Interrupt.
* @tc.number: Audio_Capturer_Interrupt_023
* @tc.desc  : Create a Speech stream type playback first, then create a ULTRASONIC source recording.
*             It is expected that renderer and capturer both running at the same time.
*/
HWTEST(AudioCapturerInterruptUnitTest, Audio_Capturer_Interrupt_023, TestSize.Level1)
{
    AudioCapturerInterruptUnitTest::AudioInterruptUnitTestFunc(TestType::RANDER_CAPTURE, true,
        SOURCE_TYPE_ULTRASONIC, SOURCE_TYPE_INVALID, renderInfo[SPEECH]);
}

/**
* @tc.name  : Test AudioCapturer Interrupt.
* @tc.number: Audio_Capturer_Interrupt_024
* @tc.desc  : Create a ULTRASONIC source recording first, then create a Movie stream type playback.
*             It is expected that capturer and renderer both running at the same time.
*/
HWTEST(AudioCapturerInterruptUnitTest, Audio_Capturer_Interrupt_024, TestSize.Level1)
{
    AudioCapturerInterruptUnitTest::AudioInterruptUnitTestFunc(TestType::CAPTURE_RANDER, true,
        SOURCE_TYPE_ULTRASONIC, SOURCE_TYPE_INVALID, renderInfo[MOVIE]);
}

/**
* @tc.name  : Test AudioCapturer Interrupt.
* @tc.number: Audio_Capturer_Interrupt_025
* @tc.desc  : Create a Movie stream type playback first, then create a ULTRASONIC source recording.
*             It is expected that renderer and capturer both running at the same time.
*/
HWTEST(AudioCapturerInterruptUnitTest, Audio_Capturer_Interrupt_025, TestSize.Level1)
{
    AudioCapturerInterruptUnitTest::AudioInterruptUnitTestFunc(TestType::RANDER_CAPTURE, true,
        SOURCE_TYPE_ULTRASONIC, SOURCE_TYPE_INVALID, renderInfo[MOVIE]);
}

/**
* @tc.name  : Test AudioCapturer Interrupt.
* @tc.number: Audio_Capturer_Interrupt_026
* @tc.desc  : Create a ULTRASONIC source recording first, then create a Unknow stream type playback.
*             It is expected that capturer and renderer both running at the same time.
*/
HWTEST(AudioCapturerInterruptUnitTest, Audio_Capturer_Interrupt_026, TestSize.Level1)
{
    AudioCapturerInterruptUnitTest::AudioInterruptUnitTestFunc(TestType::CAPTURE_RANDER, true,
        SOURCE_TYPE_ULTRASONIC, SOURCE_TYPE_INVALID, renderInfo[UNKNOW]);
}

/**
* @tc.name  : Test AudioCapturer Interrupt.
* @tc.number: Audio_Capturer_Interrupt_027
* @tc.desc  : Create a Unknow stream type playback first, then create a ULTRASONIC source recording.
*             It is expected that renderer and capturer both running at the same time.
*/
HWTEST(AudioCapturerInterruptUnitTest, Audio_Capturer_Interrupt_027, TestSize.Level1)
{
    AudioCapturerInterruptUnitTest::AudioInterruptUnitTestFunc(TestType::RANDER_CAPTURE, true,
        SOURCE_TYPE_ULTRASONIC, SOURCE_TYPE_INVALID, renderInfo[UNKNOW]);
}
} // namespace AudioStandard
} // namespace OHOS
