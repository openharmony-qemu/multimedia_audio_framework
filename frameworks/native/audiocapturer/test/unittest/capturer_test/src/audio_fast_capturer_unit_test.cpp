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

} // namespace AudioStandard
} // namespace OHOS
