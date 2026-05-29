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
#ifndef LOG_TAG
#define LOG_TAG "MultipleAudioCapturerUnitTest"
#endif

#include "multiple_audio_capturer_unit_test.h"
#include "audio_capturer.h"
#include "audio_errors.h"
#include "audio_info.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
void MultipleAudioCapturerUnitTest::SetUpTestCase(void) {}
void MultipleAudioCapturerUnitTest::TearDownTestCase(void) {}
void MultipleAudioCapturerUnitTest::SetUp(void) {}
void MultipleAudioCapturerUnitTest::TearDown(void) {}

void MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(unique_ptr<AudioCapturer> &audioCapturer,
    shared_ptr<MultipleAudioCapturerCallbackTest> &cb, SourceType sourceType, bool isStarted)
{
    AudioCapturerOptions capturerOptions;
    capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_U8;
    capturerOptions.streamInfo.channels = AudioChannel::MONO;
    capturerOptions.capturerInfo.sourceType = sourceType;
    audioCapturer = AudioCapturer::Create(capturerOptions);
    if (audioCapturer == nullptr) {
        return ;
    }

    cb = make_shared<MultipleAudioCapturerCallbackTest>();
    ASSERT_NE(nullptr, cb);
    int32_t ret = audioCapturer->SetCapturerCallback(cb);
    EXPECT_EQ(SUCCESS, ret);

    bool startOK = audioCapturer->Start();
    EXPECT_EQ(isStarted, startOK);

    return;
}

void MultipleAudioCapturerUnitTest::TestUnitFunc(SourceType s1, SourceType s2, FOCUS_RULING exp1,
    SourceType s3, FOCUS_RULING exp2)
{
    constexpr uint32_t SLEEP_TIME = 300000;
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, s1);
    if (exp1 == START_NORMAL) {
        MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, s2);
    } else if (exp1 == START_FAIL) {
        MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, s2, false);
        usleep(SLEEP_TIME);
        EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    } else if (exp1 == STOP_CAP1) {
        MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, s2);
        usleep(SLEEP_TIME);
        EXPECT_EQ(INTERRUPT_HINT_STOP, cb1->GetInterruptEvent().hintType);
    } else {
        AUDIO_ERR_LOG("The input exp1 value exceeds the expected range.");
        EXPECT_EQ(START_NORMAL, exp1);
    }

    if (exp2 == START_NORMAL) {
        MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, s3);
    } else if (exp2 == START_FAIL) {
        MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, s3, false);
        usleep(SLEEP_TIME);
        EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);
    } else if (exp2 == STOP_CAP1) {
        MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, s3);
        usleep(SLEEP_TIME);
        EXPECT_EQ(INTERRUPT_HINT_STOP, cb1->GetInterruptEvent().hintType);
    } else if (exp2 == STOP_CAP2) {
        MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, s3);
        usleep(SLEEP_TIME);
        EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    } else {
        AUDIO_ERR_LOG("The input exp2 value exceeds the expected range.");
        EXPECT_EQ(START_NORMAL, exp1);
    }

    if (audioCapturer1 != nullptr) audioCapturer1->Release();
    if (audioCapturer2 != nullptr) audioCapturer2->Release();
    if (audioCapturer2 != nullptr) audioCapturer3->Release();
}

} // namespace AudioStandard
} // namespace OHOS
