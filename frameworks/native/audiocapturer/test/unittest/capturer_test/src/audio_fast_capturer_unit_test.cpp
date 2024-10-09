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

#include "audio_capturer_unit_test.h"

#include <thread>

#include "audio_capturer.h"
#include "audio_errors.h"
#include "audio_info.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace {
bool g_isFastCapturer = true;
const uint32_t STREAM_FAST = 1;
} // namespace

class AudioFastCapturerUnitTest : public testing::Test {
public:
    // SetUpTestCase: Called before all test cases
    static void SetUpTestCase(void);
    // TearDownTestCase: Called after all test case
    static void TearDownTestCase(void);
    // SetUp: Called before each test cases
    void SetUp(void);
    // TearDown: Called after each test cases
    void TearDown(void);
    // Init Capturer Options
    static void InitializeFastCapturerOptions(AudioCapturerOptions &capturerOptions);
};

void AudioFastCapturerUnitTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
    AudioCapturerOptions capturerOptions;
    InitializeFastCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    if (audioCapturer == nullptr) {
        g_isFastCapturer = false;
    }
}
void AudioFastCapturerUnitTest::TearDownTestCase(void) {}
void AudioFastCapturerUnitTest::SetUp(void) {}
void AudioFastCapturerUnitTest::TearDown(void) {}

void AudioFastCapturerUnitTest::InitializeFastCapturerOptions(AudioCapturerOptions &capturerOptions)
{
    capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    capturerOptions.streamInfo.channels = AudioChannel::MONO;
    capturerOptions.capturerInfo.sourceType = SourceType::SOURCE_TYPE_MIC;
    capturerOptions.capturerInfo.capturerFlags = STREAM_FAST;
    return;
}

/**
 * @tc.name  : Test GetOverflowCount
 * @tc.number: Audio_Fast_Capturer_GetOverflowCount_001
 * @tc.desc  : Test GetOverflowCount interface get overflow value for fastaudiocapturer.
 */
HWTEST_F(AudioFastCapturerUnitTest, Audio_Fast_Capturer_GetOverflowCount_001, TestSize.Level1)
{
    if (!g_isFastCapturer) {
        return;
    }
    AudioCapturerOptions CapturerOptions;

    InitializeFastCapturerOptions(CapturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(CapturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    int32_t ret = audioCapturer->SetCaptureMode(CAPTURE_MODE_CALLBACK);
    EXPECT_EQ(SUCCESS, ret);

    shared_ptr<AudioCapturerReadCallbackMock> cb = make_shared<AudioCapturerReadCallbackMock>();

    ret = audioCapturer->SetCapturerReadCallback(cb);
    EXPECT_EQ(SUCCESS, ret);

    std::mutex mutex;
    std::condition_variable cv;
    int32_t count = 0;
    cb->Install([&count, &audioCapturer, &mutex, &cv](size_t length) {
                std::lock_guard lock(mutex);
                cv.notify_one();
                // only execute twice
                if (count > 1) {
                    return;
                }
                // sleep time trigger underflow
                if (count == 1) {
                    std::this_thread::sleep_for(20ms);
                }
                count++;
                BufferDesc bufDesc {};
                bufDesc.buffer = nullptr;
                auto ret = audioCapturer->GetBufferDesc(bufDesc);
                EXPECT_EQ(SUCCESS, ret);
                EXPECT_NE(nullptr, bufDesc.buffer);
                audioCapturer->Enqueue(bufDesc);
                });

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    std::unique_lock lock(mutex);
    cv.wait_for(lock, 1s, [&count] {
        // count > 1 ensure sleeped
        return count > 1;
    });

    // Verify that the callback is invoked at least once
    EXPECT_GE(cb->GetExeCount(), 1);

    auto overFlowCount = audioCapturer->GetOverflowCount();
    // Ensure the underflowCount is at least 1
    EXPECT_GE(overFlowCount, 1);

    audioCapturer->Stop();
    audioCapturer->Release();
}
} // namespace AudioStandard
} // namespace OHOS
