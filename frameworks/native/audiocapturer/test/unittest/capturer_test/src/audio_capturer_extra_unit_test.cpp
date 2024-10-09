/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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
#include "audio_capturer_private.h"
#include "audio_system_manager.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace {
    const string AUDIO_CAPTURE_FILE1 = "/data/audiocapturetest_blocking.pcm";
    const string AUDIO_FLUSH_STABILITY_TEST_FILE = "/data/audiocapture_flush_stability_test.pcm";
    const int32_t READ_BUFFERS_COUNT = 128;
    const int32_t VALUE_NEGATIVE = -1;
    const int32_t VALUE_ZERO = 0;
    const int32_t VALUE_HUNDRED = 100;
    const int32_t VALUE_THOUSAND = 1000;
    const int32_t CAPTURER_FLAG = 0;

    constexpr uint64_t BUFFER_DURATION_FIVE = 5;
    constexpr uint64_t BUFFER_DURATION_TEN = 10;
    constexpr uint64_t BUFFER_DURATION_FIFTEEN = 15;
    constexpr uint64_t BUFFER_DURATION_TWENTY = 20;
} // namespace

static void StartCaptureThread(AudioCapturer *audioCapturer, const string filePath)
{
    ASSERT_NE(audioCapturer, nullptr);
    int32_t ret = -1;
    bool isBlockingRead = true;
    size_t bufferLen;
    ret = audioCapturer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    auto buffer = std::make_unique<uint8_t[]>(bufferLen);
    ASSERT_NE(nullptr, buffer);
    FILE *capFile = fopen(filePath.c_str(), "wb");
    ASSERT_NE(nullptr, capFile);

    size_t size = 1;
    int32_t bytesRead = 0;
    int32_t numBuffersToCapture = READ_BUFFERS_COUNT;

    while (numBuffersToCapture) {
        bytesRead = audioCapturer->Read(*(buffer.get()), bufferLen, isBlockingRead);
        if (bytesRead < 0) {
            break;
        } else if (bytesRead > 0) {
            fwrite(buffer.get(), size, bytesRead, capFile);
            numBuffersToCapture--;
        }
    }

    audioCapturer->Flush();

    (void)fclose(capFile);
}

/**
* @tc.name  : Test Flush API.
* @tc.number: Audio_Capturer_Flush_001
* @tc.desc  : Test Flush interface. Returns true, if the flush is successful.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Flush_001, TestSize.Level1)
{
    int32_t ret = -1;
    bool isBlockingRead = true;
    AudioCapturerOptions capturerOptions;

    AudioCapturerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    size_t bufferLen;
    ret = audioCapturer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);
    ASSERT_NE(nullptr, buffer);

    int32_t bytesRead = audioCapturer->Read(*buffer, bufferLen, isBlockingRead);
    EXPECT_GE(bytesRead, VALUE_ZERO);

    bool isFlushed = audioCapturer->Flush();
    EXPECT_EQ(true, isFlushed);

    audioCapturer->Stop();
    audioCapturer->Release();

    free(buffer);
}

/**
* @tc.name  : Test Flush API via illegal state, CAPTURER_NEW: Without initializing the capturer.
* @tc.number: Audio_Capturer_Flush_002
* @tc.desc  : Test Flush interface. Returns false, if the capturer state is not CAPTURER_RUNNING.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Flush_002, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    bool isFlushed = audioCapturer->Flush();
    EXPECT_EQ(false, isFlushed);

    audioCapturer->Release();
}

/**
* @tc.name  : Test Flush API via illegal state, CAPTURER_PREPARED: Without Start.
* @tc.number: Audio_Capturer_Flush_003
* @tc.desc  : Test Flush interface. Returns false, if the capturer state is not CAPTURER_RUNNING.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Flush_003, TestSize.Level1)
{
    AudioCapturerOptions capturerOptions;

    AudioCapturerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool isFlushed = audioCapturer->Flush();
    EXPECT_EQ(false, isFlushed);

    audioCapturer->Release();
}

/**
* @tc.name  : Test Flush API: call Stop before Flush.
* @tc.number: Audio_Capturer_Flush_004
* @tc.desc  : Test Flush interface. Returns true, if the capturer state is CAPTURER_STOPPED.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Flush_004, TestSize.Level1)
{
    AudioCapturerOptions capturerOptions;

    AudioCapturerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioCapturer->Stop();
    EXPECT_EQ(true, isStopped);

    bool isFlushed = audioCapturer->Flush();
    EXPECT_EQ(true, isFlushed);

    audioCapturer->Release();
}

/**
* @tc.name  : Test Flush API via illegal state, CAPTURER_RELEASED: call Release before Flush.
* @tc.number: Audio_Capturer_Flush_005
* @tc.desc  : Test Flush interface. Returns false, if the capturer state is not CAPTURER_RUNNING.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Flush_005, TestSize.Level1)
{
    AudioCapturerOptions capturerOptions;

    AudioCapturerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);

    bool isFlushed = audioCapturer->Flush();
    EXPECT_EQ(false, isFlushed);

    audioCapturer->Release();
}

/**
* @tc.name  : Test Flush API stability.
* @tc.number: Audio_Capturer_Flush_Stability_001
* @tc.desc  : Test Flush interface stability.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Flush_Stability_001, TestSize.Level1)
{
    AudioCapturerOptions capturerOptions;

    AudioCapturerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    thread captureThread(StartCaptureThread, audioCapturer.get(), AUDIO_FLUSH_STABILITY_TEST_FILE);

    for (int i = 0; i < VALUE_HUNDRED; i++) {
        bool isFlushed = audioCapturer->Flush();
        EXPECT_EQ(true, isFlushed);
    }

    captureThread.join();

    bool isStopped = audioCapturer->Stop();
    EXPECT_EQ(true, isStopped);

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
* @tc.name  : Test Stop API.
* @tc.number: Audio_Capturer_Stop_001
* @tc.desc  : Test Stop interface. Returns true, if the stop is successful.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Stop_001, TestSize.Level1)
{
    int32_t ret = -1;
    bool isBlockingRead = true;
    AudioCapturerOptions capturerOptions;

    AudioCapturerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    size_t bufferLen;
    ret = audioCapturer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);
    ASSERT_NE(nullptr, buffer);

    int32_t bytesRead = audioCapturer->Read(*buffer, bufferLen, isBlockingRead);
    EXPECT_GE(bytesRead, VALUE_ZERO);

    audioCapturer->Flush();

    bool isStopped = audioCapturer->Stop();
    EXPECT_EQ(true, isStopped);

    audioCapturer->Release();

    free(buffer);
}

/**
* @tc.name  : Test Stop API via illegal state, CAPTURER_NEW: call Stop without Initializing the capturer.
* @tc.number: Audio_Capturer_Stop_002
* @tc.desc  : Test Stop interface. Returns false, if the capturer state is not CAPTURER_RUNNING.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Stop_002, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    bool isStopped = audioCapturer->Stop();
    EXPECT_EQ(false, isStopped);
}

/**
* @tc.name  : Test Stop API via illegal state, CAPTURER_PREPARED: call Stop without Start.
* @tc.number: Audio_Capturer_Stop_003
* @tc.desc  : Test Stop interface. Returns false, if the capturer state is not CAPTURER_RUNNING.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Stop_003, TestSize.Level1)
{
    AudioCapturerOptions capturerOptions;

    AudioCapturerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool isStopped = audioCapturer->Stop();
    EXPECT_EQ(false, isStopped);

    audioCapturer->Release();
}

/**
* @tc.name  : Test Stop API via illegal state, CAPTURER_RELEASED: call Stop after Release.
* @tc.number: Audio_Capturer_Stop_004
* @tc.desc  : Test Stop interface. Returns false, if the capturer state is not CAPTURER_RUNNING.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Stop_004, TestSize.Level1)
{
    AudioCapturerOptions capturerOptions;

    AudioCapturerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);

    bool isStopped = audioCapturer->Stop();
    EXPECT_EQ(false, isStopped);
}

/**
* @tc.name  : Test Stop API via legal state. call Start, Stop, Start and Stop again
* @tc.number: Audio_Capturer_Stop_005
* @tc.desc  : Test Stop interface. Returns true , if the stop is successful.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Stop_005, TestSize.Level1)
{
    AudioCapturerOptions capturerOptions;

    AudioCapturerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioCapturer->Stop();
    EXPECT_EQ(true, isStopped);

    isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    isStopped = audioCapturer->Stop();
    EXPECT_EQ(true, isStopped);
    audioCapturer->Release();
}

/**
* @tc.name  : Test Stop API.
* @tc.number: Audio_Capturer_Stop_006
* @tc.desc  : Test Stop interface. Returns true, if the stop is successful.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Stop_006, TestSize.Level1)
{
    int32_t ret = -1;
    bool isBlockingRead = true;
    AudioCapturerOptions capturerOptions;
    AudioCapturerUnitTest::InitializePlaybackCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    size_t bufferLen;
    ret = audioCapturer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);
    ASSERT_NE(nullptr, buffer);
    FILE *capFile = fopen(AUDIO_CAPTURE_FILE1.c_str(), "wb");
    ASSERT_NE(nullptr, capFile);

    size_t size = 1;
    int32_t bytesRead = 0;
    int32_t numBuffersToCapture = READ_BUFFERS_COUNT;

    while (numBuffersToCapture) {
        bytesRead = audioCapturer->Read(*buffer, bufferLen, isBlockingRead);
        if (bytesRead <= 0) {
            break;
        } else if (bytesRead > 0) {
            fwrite(buffer, size, bytesRead, capFile);
            numBuffersToCapture--;
        }
    }

    audioCapturer->Flush();

    bool isStopped = audioCapturer->Stop();
    EXPECT_EQ(true, isStopped);

    audioCapturer->Release();

    free(buffer);
}

/**
* @tc.name  : Test Release API.
* @tc.number: Audio_Capturer_Release_001
* @tc.desc  : Test Release interface. Returns true, if the release is successful.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Release_001, TestSize.Level1)
{
    int32_t ret = -1;
    bool isBlockingRead = true;
    AudioCapturerOptions capturerOptions;

    AudioCapturerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    size_t bufferLen;
    ret = audioCapturer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);
    ASSERT_NE(nullptr, buffer);

    int32_t bytesRead = audioCapturer->Read(*buffer, bufferLen, isBlockingRead);
    EXPECT_GE(bytesRead, VALUE_ZERO);

    audioCapturer->Flush();
    audioCapturer->Stop();

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);

    free(buffer);
}

/**
* @tc.name  : Test Release API via illegal state, CAPTURER_NEW: Call Release without initializing the capturer.
* @tc.number: Audio_Capturer_Release_002
* @tc.desc  : Test Release interface, Returns true, if the state is CAPTURER_NEW.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Release_002, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
* @tc.name  : Test Release API via illegal state, CAPTURER_RELEASED: call Release repeatedly.
* @tc.number: Audio_Capturer_Release_003
* @tc.desc  : Test Release interface. Returns true, if the state is already CAPTURER_RELEASED.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Release_003, TestSize.Level1)
{
    AudioCapturerOptions capturerOptions;

    AudioCapturerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);

    isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
* @tc.name  : Test Release API via legal state, CAPTURER_RUNNING: call Release after Start
* @tc.number: Audio_Capturer_Release_004
* @tc.desc  : Test Release interface. Returns true, if the release is successful.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Release_004, TestSize.Level1)
{
    AudioCapturerOptions capturerOptions;

    AudioCapturerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
* @tc.name  : Test Release API via legal state, CAPTURER_STOPPED: call release after Start and Stop
* @tc.number: Audio_Capturer_Release_005
* @tc.desc  : Test Release interface. Returns true, if the release is successful.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Release_005, TestSize.Level1)
{
    AudioCapturerOptions capturerOptions;

    AudioCapturerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioCapturer->Stop();
    EXPECT_EQ(true, isStopped);

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
* @tc.name  : Test Release API.
* @tc.number: Audio_Capturer_Release_006
* @tc.desc  : Test Release interface. Returns true, if the release is successful.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Release_006, TestSize.Level1)
{
    int32_t ret = -1;
    bool isBlockingRead = true;
    AudioCapturerOptions capturerOptions;
    capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S32LE;
    capturerOptions.streamInfo.channels = AudioChannel::STEREO;
    capturerOptions.capturerInfo.sourceType = SourceType::SOURCE_TYPE_PLAYBACK_CAPTURE;
    capturerOptions.capturerInfo.capturerFlags = CAPTURER_FLAG;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    size_t bufferLen;
    ret = audioCapturer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);
    ASSERT_NE(nullptr, buffer);
    FILE *capFile = fopen(AUDIO_CAPTURE_FILE1.c_str(), "wb");
    ASSERT_NE(nullptr, capFile);

    size_t size = 1;
    int32_t bytesRead = 0;
    int32_t numBuffersToCapture = READ_BUFFERS_COUNT;

    while (numBuffersToCapture) {
        bytesRead = audioCapturer->Read(*buffer, bufferLen, isBlockingRead);
        if (bytesRead <= 0) {
            break;
        } else if (bytesRead > 0) {
            fwrite(buffer, size, bytesRead, capFile);
            numBuffersToCapture--;
        }
    }

    audioCapturer->Flush();
    audioCapturer->Stop();

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);

    free(buffer);
}

/**
* @tc.name  : Test GetStatus API.
* @tc.number: Audio_Capturer_GetStatus_001
* @tc.desc  : Test GetStatus interface. Returns correct state on success.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_GetStatus_001, TestSize.Level1)
{
    CapturerState state = CAPTURER_INVALID;
    AudioCapturerOptions capturerOptions;

    AudioCapturerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    state = audioCapturer->GetStatus();
    EXPECT_EQ(CAPTURER_PREPARED, state);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);
    state = audioCapturer->GetStatus();
    EXPECT_EQ(CAPTURER_RUNNING, state);

    bool isStopped = audioCapturer->Stop();
    EXPECT_EQ(true, isStopped);
    state = audioCapturer->GetStatus();
    EXPECT_EQ(CAPTURER_STOPPED, state);

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);
    state = audioCapturer->GetStatus();
    EXPECT_EQ(CAPTURER_RELEASED, state);
}

/**
* @tc.name  : Test GetStatus API, call Start without Initializing the capturer
* @tc.number: Audio_Capturer_GetStatus_002
* @tc.desc  : Test GetStatus interface. Not changes to CAPTURER_RUNNING, if the current state is CAPTURER_NEW.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_GetStatus_002, TestSize.Level1)
{
    CapturerState state = CAPTURER_INVALID;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(false, isStarted);
    state = audioCapturer->GetStatus();
    EXPECT_NE(CAPTURER_RUNNING, state);
    EXPECT_EQ(CAPTURER_NEW, state);
}

/**
* @tc.name  : Test GetStatus API, call Stop without Start
* @tc.number: Audio_Capturer_GetStatus_003
* @tc.desc  : Test GetStatus interface. Not changes to CAPTURER_STOPPED, if the current state is CAPTURER_PREPARED.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_GetStatus_003, TestSize.Level1)
{
    CapturerState state = CAPTURER_INVALID;
    AudioCapturerOptions capturerOptions;

    AudioCapturerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool isStopped = audioCapturer->Stop();
    EXPECT_EQ(false, isStopped);
    state = audioCapturer->GetStatus();
    EXPECT_NE(CAPTURER_STOPPED, state);
    EXPECT_EQ(CAPTURER_PREPARED, state);

    audioCapturer->Release();
}

/**
* @tc.name  : Test GetStatus API, call Start, Stop and then Start again
* @tc.number: Audio_Capturer_GetStatus_004
* @tc.desc  : Test GetStatus interface.  Returns correct state on success.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_GetStatus_004, TestSize.Level1)
{
    CapturerState state = CAPTURER_INVALID;
    AudioCapturerOptions capturerOptions;

    AudioCapturerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);
    state = audioCapturer->GetStatus();
    EXPECT_EQ(CAPTURER_RUNNING, state);

    bool isStopped = audioCapturer->Stop();
    EXPECT_EQ(true, isStopped);
    state = audioCapturer->GetStatus();
    EXPECT_EQ(CAPTURER_STOPPED, state);

    isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);
    state = audioCapturer->GetStatus();
    EXPECT_EQ(CAPTURER_RUNNING, state);

    audioCapturer->Release();
}

/**
* @tc.name  : Test GetStatus API, call Release without initializing
* @tc.number: Audio_Capturer_GetStatus_005
* @tc.desc  : Test GetStatus interface. Changing to CAPTURER_RELEASED, if the current state is CAPTURER_NEW.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_GetStatus_005, TestSize.Level1)
{
    CapturerState state = CAPTURER_INVALID;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);
    state = audioCapturer->GetStatus();
    EXPECT_EQ(CAPTURER_RELEASED, state);
    EXPECT_NE(CAPTURER_NEW, state);
}

/**
* @tc.name  : Test GetCapturerInfo API after calling create
* @tc.number: Audio_Capturer_GetCapturerInfo_001
* @tc.desc  : Test GetCapturerInfo interface. Check whether capturer info returns proper data
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_GetCapturerInfo_001, TestSize.Level1)
{
    AudioCapturerOptions capturerOptions;
    capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_96000;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_U8;
    capturerOptions.streamInfo.channels = AudioChannel::MONO;
    capturerOptions.capturerInfo.sourceType = SourceType::SOURCE_TYPE_MIC;
    capturerOptions.capturerInfo.capturerFlags = CAPTURER_FLAG;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    AudioCapturerInfo capturerInfo;
    audioCapturer->GetCapturerInfo(capturerInfo);

    EXPECT_EQ(SourceType::SOURCE_TYPE_MIC, capturerInfo.sourceType);
    EXPECT_EQ(CAPTURER_FLAG, capturerInfo.capturerFlags);
    audioCapturer->Release();
}

/**
* @tc.name  : Test GetCapturerInfo API after calling start
* @tc.number: Audio_Capturer_GetCapturerInfo_002
* @tc.desc  : Test GetCapturerInfo interface. Check whether capturer info returns proper data
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_GetCapturerInfo_002, TestSize.Level1)
{
    AudioCapturerOptions capturerOptions;
    capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_96000;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_U8;
    capturerOptions.streamInfo.channels = AudioChannel::MONO;
    capturerOptions.capturerInfo.sourceType = SourceType::SOURCE_TYPE_MIC;
    capturerOptions.capturerInfo.capturerFlags = CAPTURER_FLAG;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    AudioCapturerInfo capturerInfo;
    audioCapturer->GetCapturerInfo(capturerInfo);

    EXPECT_EQ(SourceType::SOURCE_TYPE_MIC, capturerInfo.sourceType);
    EXPECT_EQ(CAPTURER_FLAG, capturerInfo.capturerFlags);

    audioCapturer->Stop();
    audioCapturer->Release();
}

/**
* @tc.name  : Test GetCapturerInfo API after calling release
* @tc.number: Audio_Capturer_GetCapturerInfo_003
* @tc.desc  : Test GetCapturerInfo interface. Check whether capturer info returns proper data
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_GetCapturerInfo_003, TestSize.Level1)
{
    AudioCapturerOptions capturerOptions;
    capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_96000;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_U8;
    capturerOptions.streamInfo.channels = AudioChannel::MONO;
    capturerOptions.capturerInfo.sourceType = SourceType::SOURCE_TYPE_MIC;
    capturerOptions.capturerInfo.capturerFlags = CAPTURER_FLAG;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);

    AudioCapturerInfo capturerInfo;
    audioCapturer->GetCapturerInfo(capturerInfo);

    EXPECT_EQ(SourceType::SOURCE_TYPE_MIC, capturerInfo.sourceType);
    EXPECT_EQ(CAPTURER_FLAG, capturerInfo.capturerFlags);
}

/**
* @tc.name  : Test GetCapturerInfo API after calling stop
* @tc.number: Audio_Capturer_GetCapturerInfo_004
* @tc.desc  : Test GetCapturerInfo interface. Check whether capturer info returns proper data
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_GetCapturerInfo_004, TestSize.Level1)
{
    AudioCapturerOptions capturerOptions;
    capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_96000;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_U8;
    capturerOptions.streamInfo.channels = AudioChannel::MONO;
    capturerOptions.capturerInfo.sourceType = SourceType::SOURCE_TYPE_MIC;
    capturerOptions.capturerInfo.capturerFlags = CAPTURER_FLAG;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioCapturer->Stop();
    EXPECT_EQ(true, isStopped);

    AudioCapturerInfo capturerInfo;
    audioCapturer->GetCapturerInfo(capturerInfo);

    EXPECT_EQ(SourceType::SOURCE_TYPE_MIC, capturerInfo.sourceType);
    EXPECT_EQ(CAPTURER_FLAG, capturerInfo.capturerFlags);
    audioCapturer->Release();
}

/**
* @tc.name  : Test GetCapturerInfo API Stability
* @tc.number: Audio_Capturer_GetCapturerInfo_Stability_001
* @tc.desc  : Test GetCapturerInfo interface. Check whether capturer info returns proper data
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_GetCapturerInfo_Stability_001, TestSize.Level1)
{
    AudioCapturerOptions capturerOptions;
    capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_96000;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_U8;
    capturerOptions.streamInfo.channels = AudioChannel::MONO;
    capturerOptions.capturerInfo.sourceType = SourceType::SOURCE_TYPE_MIC;
    capturerOptions.capturerInfo.capturerFlags = CAPTURER_FLAG;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    for (int i = 0; i < VALUE_HUNDRED; i++) {

        AudioCapturerInfo capturerInfo;
        audioCapturer->GetCapturerInfo(capturerInfo);

        EXPECT_EQ(SourceType::SOURCE_TYPE_MIC, capturerInfo.sourceType);
        EXPECT_EQ(CAPTURER_FLAG, capturerInfo.capturerFlags);
    }
    audioCapturer->Release();
}

/**
* @tc.name  : Test GetStreamInfo API after calling create
* @tc.number: Audio_Capturer_GetStreamInfo_001
* @tc.desc  : Test GetStreamInfo interface. Check whether stream related data is returned correctly
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_GetStreamInfo_001, TestSize.Level1)
{
    AudioCapturerOptions capturerOptions;
    capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_96000;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_U8;
    capturerOptions.streamInfo.channels = AudioChannel::MONO;
    capturerOptions.capturerInfo.sourceType = SourceType::SOURCE_TYPE_MIC;
    capturerOptions.capturerInfo.capturerFlags = CAPTURER_FLAG;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    AudioStreamInfo streamInfo;
    audioCapturer->GetStreamInfo(streamInfo);

    EXPECT_EQ(AudioSamplingRate::SAMPLE_RATE_96000, streamInfo.samplingRate);
    EXPECT_EQ(AudioEncodingType::ENCODING_PCM, streamInfo.encoding);
    EXPECT_EQ(AudioSampleFormat::SAMPLE_U8, streamInfo.format);
    EXPECT_EQ(AudioChannel::MONO, streamInfo.channels);
    audioCapturer->Release();
}

/**
* @tc.name  : Test GetStreamInfo API after calling start
* @tc.number: Audio_Capturer_GetStreamInfo_002
* @tc.desc  : Test GetStreamInfo interface. Check whether stream related data is returned correctly
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_GetStreamInfo_002, TestSize.Level1)
{
    int32_t ret = -1;
    AudioCapturerOptions capturerOptions;

    capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_96000;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_U8;
    capturerOptions.streamInfo.channels = AudioChannel::MONO;
    capturerOptions.capturerInfo.sourceType = SourceType::SOURCE_TYPE_MIC;
    capturerOptions.capturerInfo.capturerFlags = CAPTURER_FLAG;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    AudioStreamInfo streamInfo;
    ret = audioCapturer->GetStreamInfo(streamInfo);

    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(AudioSamplingRate::SAMPLE_RATE_96000, streamInfo.samplingRate);
    EXPECT_EQ(AudioEncodingType::ENCODING_PCM, streamInfo.encoding);
    EXPECT_EQ(AudioSampleFormat::SAMPLE_U8, streamInfo.format);
    EXPECT_EQ(AudioChannel::MONO, streamInfo.channels);

    audioCapturer->Stop();
    audioCapturer->Release();
}

/**
* @tc.name  : Test GetStreamInfo API after calling stop and release
* @tc.number: Audio_Capturer_GetStreamInfo_003
* @tc.desc  : Test GetStreamInfo interface. Check whether stream related data is returned correctly
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_GetStreamInfo_003, TestSize.Level1)
{
    int32_t ret1 = -1;
    int32_t ret2 = -1;
    AudioCapturerOptions capturerOptions;

    capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_96000;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_U8;
    capturerOptions.streamInfo.channels = AudioChannel::MONO;
    capturerOptions.capturerInfo.sourceType = SourceType::SOURCE_TYPE_MIC;
    capturerOptions.capturerInfo.capturerFlags = CAPTURER_FLAG;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioCapturer->Stop();
    EXPECT_EQ(true, isStopped);

    AudioStreamInfo streamInfo1;
    ret1 = audioCapturer->GetStreamInfo(streamInfo1);

    EXPECT_EQ(SUCCESS, ret1);
    EXPECT_EQ(AudioSamplingRate::SAMPLE_RATE_96000, streamInfo1.samplingRate);
    EXPECT_EQ(AudioEncodingType::ENCODING_PCM, streamInfo1.encoding);
    EXPECT_EQ(AudioSampleFormat::SAMPLE_U8, streamInfo1.format);
    EXPECT_EQ(AudioChannel::MONO, streamInfo1.channels);

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);

    AudioStreamInfo streamInfo2;
    ret2 = audioCapturer->GetStreamInfo(streamInfo2);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret2);
}

/**
* @tc.name  : Test GetStreamInfo API after calling create
* @tc.number: Audio_Renderer_GetStreamInfo_Stability_001
* @tc.desc  : Test GetStreamInfo interface. Check whether stream related data is returned correctly
*/
HWTEST(AudioCapturerUnitTest, Audio_Renderer_GetStreamInfo_Stability_001, TestSize.Level1)
{
    int32_t ret = -1;
    AudioCapturerOptions capturerOptions;

    capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_96000;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_U8;
    capturerOptions.streamInfo.channels = AudioChannel::MONO;
    capturerOptions.capturerInfo.sourceType = SourceType::SOURCE_TYPE_MIC;
    capturerOptions.capturerInfo.capturerFlags = CAPTURER_FLAG;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    for (int i = 0; i < VALUE_HUNDRED; i++) {
        AudioStreamInfo streamInfo;
        ret = audioCapturer->GetStreamInfo(streamInfo);
        EXPECT_EQ(SUCCESS, ret);
    }
    audioCapturer->Release();
}

/**
* @tc.name  : Test SetBufferDuration API
* @tc.number: Audio_Capturer_SetBufferDuration_001
* @tc.desc  : Test SetBufferDuration interface. Check whether valid or invalid parameters are accepted or rejected.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_SetBufferDuration_001, TestSize.Level1)
{
    int32_t ret = -1;

    AudioCapturerOptions capturerOptions;
    capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_96000;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_U8;
    capturerOptions.streamInfo.channels = AudioChannel::MONO;
    capturerOptions.capturerInfo.sourceType = SourceType::SOURCE_TYPE_MIC;
    capturerOptions.capturerInfo.capturerFlags = CAPTURER_FLAG;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    ret = audioCapturer->SetBufferDuration(BUFFER_DURATION_FIVE);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioCapturer->SetBufferDuration(BUFFER_DURATION_TEN);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioCapturer->SetBufferDuration(BUFFER_DURATION_FIFTEEN);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioCapturer->SetBufferDuration(BUFFER_DURATION_TWENTY);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioCapturer->SetBufferDuration(VALUE_NEGATIVE);
    EXPECT_NE(SUCCESS, ret);

    ret = audioCapturer->SetBufferDuration(VALUE_ZERO);
    EXPECT_NE(SUCCESS, ret);

    ret = audioCapturer->SetBufferDuration(VALUE_HUNDRED);
    EXPECT_NE(SUCCESS, ret);
}

/**
* @tc.name  : Test SetCapturerPositionCallback API
* @tc.number: Audio_Capturer_SetCapturerPositionCallback_001
* @tc.desc  : Test SetCapturerPositionCallback interface to check set position callback is success for valid callback.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_SetCapturerPositionCallback_001, TestSize.Level1)
{
    int32_t ret = -1;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    shared_ptr<CapturerPositionCallbackTest> positionCB = std::make_shared<CapturerPositionCallbackTest>();
    ret = audioCapturer->SetCapturerPositionCallback(VALUE_THOUSAND, positionCB);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test SetCapturerPositionCallback API
* @tc.number: Audio_Capturer_SetCapturerPositionCallback_002
* @tc.desc  : Test SetCapturerPositionCallback interface again after unregister.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_SetCapturerPositionCallback_002, TestSize.Level1)
{
    int32_t ret = -1;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    shared_ptr<CapturerPositionCallbackTest> positionCB1 = std::make_shared<CapturerPositionCallbackTest>();
    ret = audioCapturer->SetCapturerPositionCallback(VALUE_THOUSAND, positionCB1);
    EXPECT_EQ(SUCCESS, ret);

    audioCapturer->UnsetCapturerPositionCallback();

    shared_ptr<CapturerPositionCallbackTest> positionCB2 = std::make_shared<CapturerPositionCallbackTest>();
    ret = audioCapturer->SetCapturerPositionCallback(VALUE_THOUSAND, positionCB2);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test SetCapturerPositionCallback API
* @tc.number: Audio_Capturer_SetCapturerPositionCallback_003
* @tc.desc  : Test SetCapturerPositionCallback interface with null callback.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_SetCapturerPositionCallback_003, TestSize.Level1)
{
    int32_t ret = -1;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    ret = audioCapturer->SetCapturerPositionCallback(VALUE_THOUSAND, nullptr);
    EXPECT_NE(SUCCESS, ret);
}

/**
* @tc.name  : Test SetCapturerPositionCallback API
* @tc.number: Audio_Capturer_SetCapturerPositionCallback_004
* @tc.desc  : Test SetCapturerPositionCallback interface with invalid parameter.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_SetCapturerPositionCallback_004, TestSize.Level1)
{
    int32_t ret = -1;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    shared_ptr<CapturerPositionCallbackTest> positionCB = std::make_shared<CapturerPositionCallbackTest>();
    ret = audioCapturer->SetCapturerPositionCallback(VALUE_ZERO, positionCB);
    EXPECT_NE(SUCCESS, ret);

    ret = audioCapturer->SetCapturerPositionCallback(VALUE_NEGATIVE, positionCB);
    EXPECT_NE(SUCCESS, ret);
}

/**
* @tc.name  : Test SetCapturerPeriodPositionCallback API
* @tc.number: SetCapturerPeriodPositionCallback_001
* @tc.desc  : Test SetCapturerPeriodPositionCallback interface to check set period position
*             callback is success for valid callback.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_SetCapturerPeriodPositionCallback_001, TestSize.Level1)
{
    int32_t ret = -1;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    shared_ptr<CapturerPeriodPositionCallbackTest> positionCB = std::make_shared<CapturerPeriodPositionCallbackTest>();
    ret = audioCapturer->SetCapturerPeriodPositionCallback(VALUE_THOUSAND, positionCB);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test SetCapturerPeriodPositionCallback API
* @tc.number: Audio_Capturer_SetCapturerPeriodPositionCallback_002
* @tc.desc  : Test SetCapturerPeriodPositionCallback interface again after unregister.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_SetCapturerPeriodPositionCallback_002, TestSize.Level1)
{
    int32_t ret = -1;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    shared_ptr<CapturerPeriodPositionCallbackTest> positionCB1 = std::make_shared<CapturerPeriodPositionCallbackTest>();
    ret = audioCapturer->SetCapturerPeriodPositionCallback(VALUE_THOUSAND, positionCB1);
    EXPECT_EQ(SUCCESS, ret);

    audioCapturer->UnsetCapturerPeriodPositionCallback();

    shared_ptr<CapturerPeriodPositionCallbackTest> positionCB2 = std::make_shared<CapturerPeriodPositionCallbackTest>();
    ret = audioCapturer->SetCapturerPeriodPositionCallback(VALUE_THOUSAND, positionCB2);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test SetCapturerPeriodPositionCallback API
* @tc.number: Audio_Capturer_SetCapturerPeriodPositionCallback_003
* @tc.desc  : Test SetCapturerPeriodPositionCallback interface with null callback.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_SetCapturerPeriodPositionCallback_003, TestSize.Level1)
{
    int32_t ret = -1;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    ret = audioCapturer->SetCapturerPeriodPositionCallback(VALUE_THOUSAND, nullptr);
    EXPECT_NE(SUCCESS, ret);
}

/**
* @tc.name  : Test SetCapturerPeriodPositionCallback API
* @tc.number: Audio_Capturer_SetCapturerPeriodPositionCallback_004
* @tc.desc  : Test SetCapturerPeriodPositionCallback interface with invalid parameter.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_SetCapturerPeriodPositionCallback_004, TestSize.Level1)
{
    int32_t ret = -1;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    shared_ptr<CapturerPeriodPositionCallbackTest> positionCB = std::make_shared<CapturerPeriodPositionCallbackTest>();
    ret = audioCapturer->SetCapturerPeriodPositionCallback(VALUE_ZERO, positionCB);
    EXPECT_NE(SUCCESS, ret);

    ret = audioCapturer->SetCapturerPeriodPositionCallback(VALUE_NEGATIVE, positionCB);
    EXPECT_NE(SUCCESS, ret);
}

/**
* @tc.name  : Test SetCapturerCallback with null pointer.
* @tc.number: Audio_Capturer_SetCapturerCallback_001
* @tc.desc  : Test SetCapturerCallback interface. Returns error code, if null pointer is set.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_SetCapturerCallback_001, TestSize.Level1)
{
    int32_t ret = -1;

    AudioCapturerOptions capturerOptions;
    capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    capturerOptions.streamInfo.channels = AudioChannel::MONO;
    capturerOptions.capturerInfo.sourceType = SourceType::SOURCE_TYPE_MIC;
    capturerOptions.capturerInfo.capturerFlags = CAPTURER_FLAG;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    ret = audioCapturer->SetCapturerCallback(nullptr);
    EXPECT_NE(SUCCESS, ret);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}

/**
* @tc.name  : Test SetCapturerCallback with valid callback pointer.
* @tc.number: Audio_Capturer_SetCapturerCallback_002
* @tc.desc  : Test SetCapturerCallback interface. Returns success, if valid callback is set.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_SetCapturerCallback_002, TestSize.Level1)
{
    int32_t ret = -1;

    AudioCapturerOptions capturerOptions;
    capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    capturerOptions.streamInfo.channels = AudioChannel::MONO;
    capturerOptions.capturerInfo.sourceType = SourceType::SOURCE_TYPE_MIC;
    capturerOptions.capturerInfo.capturerFlags = CAPTURER_FLAG;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    shared_ptr<AudioCapturerCallbackTest> audioCapturerCB = std::make_shared<AudioCapturerCallbackTest>();
    ret = audioCapturer->SetCapturerCallback(audioCapturerCB);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test SetCapturerCallback via illegal state, CAPTURER_RELEASED: After RELEASED
* @tc.number: Audio_Capturer_SetCapturerCallback_003
* @tc.desc  : Test SetCapturerCallback interface. Returns error, if callback is set in released state.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_SetCapturerCallback_003, TestSize.Level1)
{
    int32_t ret = -1;

    AudioCapturerOptions capturerOptions;
    capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    capturerOptions.streamInfo.channels = AudioChannel::MONO;
    capturerOptions.capturerInfo.sourceType = SourceType::SOURCE_TYPE_MIC;
    capturerOptions.capturerInfo.capturerFlags = CAPTURER_FLAG;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);

    CapturerState state = audioCapturer->GetStatus();
    EXPECT_EQ(CAPTURER_RELEASED, state);

    shared_ptr<AudioCapturerCallbackTest> audioCapturerCB = std::make_shared<AudioCapturerCallbackTest>();
    ret = audioCapturer->SetCapturerCallback(audioCapturerCB);
    EXPECT_NE(SUCCESS, ret);
    EXPECT_EQ(ERR_ILLEGAL_STATE, ret);
}

/**
* @tc.name  : Test SetCapturerCallback via legal state, CAPTURER_PREPARED: After PREPARED
* @tc.number: Audio_Capturer_SetCapturerCallback_004
* @tc.desc  : Test SetCapturerCallback interface. Returns success, if callback is set in proper state.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_SetCapturerCallback_004, TestSize.Level1)
{
    int32_t ret = -1;

    AudioCapturerOptions capturerOptions;
    capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    capturerOptions.streamInfo.channels = AudioChannel::MONO;
    capturerOptions.capturerInfo.sourceType = SourceType::SOURCE_TYPE_MIC;
    capturerOptions.capturerInfo.capturerFlags = CAPTURER_FLAG;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    CapturerState state = audioCapturer->GetStatus();
    EXPECT_EQ(CAPTURER_PREPARED, state);

    shared_ptr<AudioCapturerCallbackTest> audioCapturerCB = std::make_shared<AudioCapturerCallbackTest>();
    ret = audioCapturer->SetCapturerCallback(audioCapturerCB);
    EXPECT_EQ(SUCCESS, ret);
    audioCapturer->Release();
}

/**
 * @tc.name  : Test SetCaptureMode via legal state.
 * @tc.number: Audio_Capturer_SetCaptureMode_001
 * @tc.desc  : Test SetCaptureMode interface. Returns success, if the set capture mode is successful.
 */
HWTEST(AudioCapturerUnitTest, Audio_Capturer_SetCaptureMode_001, TestSize.Level1)
{
    int32_t ret = -1;

    AudioCapturerOptions capturerOptions;
    capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    capturerOptions.streamInfo.channels = AudioChannel::MONO;
    capturerOptions.capturerInfo.sourceType = SourceType::SOURCE_TYPE_MIC;
    capturerOptions.capturerInfo.capturerFlags = CAPTURER_FLAG;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    ret = audioCapturer->SetCaptureMode(CAPTURE_MODE_CALLBACK);
    EXPECT_EQ(SUCCESS, ret);
    audioCapturer->Release();
}

/**
 * @tc.name  : Test GetCaptureMode via legal state.
 * @tc.number: Audio_Capturer_GetCaptureMode_001
 * @tc.desc  : Test GetCaptureMode interface. Returns success, if the get capture mode is successful.
 */
HWTEST(AudioCapturerUnitTest, Audio_Capturer_GetCaptureMode, TestSize.Level1)
{
    int32_t ret = -1;

    AudioCapturerOptions capturerOptions;
    capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    capturerOptions.streamInfo.channels = AudioChannel::MONO;
    capturerOptions.capturerInfo.sourceType = SourceType::SOURCE_TYPE_MIC;
    capturerOptions.capturerInfo.capturerFlags = CAPTURER_FLAG;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    ret = audioCapturer->SetCaptureMode(CAPTURE_MODE_CALLBACK);
    EXPECT_EQ(SUCCESS, ret);

    AudioCaptureMode captureMode = audioCapturer->GetCaptureMode();
    EXPECT_EQ(CAPTURE_MODE_CALLBACK, captureMode);
    audioCapturer->Release();
}

/**
 * @tc.name  : Test AudioCapturer interface.
 * @tc.number: Audio_Capturer_AudioCapturerCallback_001
 * @tc.desc  : Test AudioCapturerReadCallbackTest interface.
 */
HWTEST(AudioCapturerUnitTest, Audio_Capturer_AudioCapturerCallback_001, TestSize.Level1)
{
    int32_t ret = -1;
    AudioCapturerOptions capturerOptions;
    AppInfo appInfo = {};
    capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_8000;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_U8;
    capturerOptions.streamInfo.channels = AudioChannel::MONO;
    capturerOptions.capturerInfo.sourceType = SourceType::SOURCE_TYPE_MIC;
    capturerOptions.capturerInfo.capturerFlags = CAPTURER_FLAG;
    appInfo.appTokenId = VALUE_THOUSAND;
    appInfo.appUid = static_cast<int32_t>(getuid());
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions, appInfo);
    ASSERT_NE(nullptr, audioCapturer);

    std::shared_ptr<AudioCapturerReadCallbackTest> callback;
    callback = std::make_shared<AudioCapturerReadCallbackTest>();

    ret = audioCapturer->SetCapturerReadCallback(callback);
    EXPECT_LT(ret, 0);

    BufferDesc bufDesc;
    ret = audioCapturer->GetBufferDesc(bufDesc);
    EXPECT_LT(ret, 0);

    ret = audioCapturer->Enqueue(bufDesc);
    EXPECT_LT(ret, 0);

    BufferQueueState bufState;
    ret = audioCapturer->GetBufQueueState(bufState);
    EXPECT_LT(ret, 0);

    ret = audioCapturer->Clear();
    EXPECT_LT(ret, 0);
    audioCapturer->Release();
}

/**
 * @tc.name  : Test GetFramesRead API stability.
 * @tc.number: Audio_Capturer_GetFramesRead_001
 * @tc.desc  : Test GetFramesRead interface stability.
 */
HWTEST(AudioCapturerUnitTest, Audio_Capturer_GetFramesRead_001, TestSize.Level1)
{
    int64_t ret = -1;
    AudioCapturerOptions capturerOptions;

    AudioCapturerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    ret = audioCapturer->GetFramesRead();
    EXPECT_EQ(SUCCESS, ret);

    audioCapturer->Release();
}

/**
 * @tc.name  : Test SetApplicationCachePath API stability.
 * @tc.number: Audio_Capturer_SetApplicationCachePath_001
 * @tc.desc  : Test SetApplicationCachePath interface stability.
 */
HWTEST(AudioCapturerUnitTest, Audio_Capturer_SetApplicationCachePath_001, TestSize.Level1)
{
    string cachePath = "/data/storage/el2/base/temp";
    AudioCapturerOptions capturerOptions;

    AudioCapturerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);

    audioCapturer->SetApplicationCachePath(cachePath.c_str());
    ASSERT_NE(nullptr, audioCapturer);

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
 * @tc.name  : Test GetCurrentInputDevices API stability.
 * @tc.number: Audio_Capturer_GetCurrentInputDevices_001
 * @tc.desc  : Test GetCurrentInputDevices interface stability.
 */
HWTEST(AudioCapturerUnitTest, Audio_Capturer_GetCurrentInputDevices_001, TestSize.Level1)
{
    int32_t ret = -1;
    AudioCapturerOptions capturerOptions;

    AudioCapturerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    DeviceInfo deviceInfo;
    ret = audioCapturer->GetCurrentInputDevices(deviceInfo);
    EXPECT_EQ(SUCCESS, ret);

    AudioCapturerChangeInfo changeInfo;
    ret = audioCapturer->GetCurrentCapturerChangeInfo(changeInfo);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioCapturer ->SetAudioCapturerDeviceChangeCallback(nullptr);
    EXPECT_EQ(ERROR, ret);
    shared_ptr<AudioCapturerDeviceChangeCallback> callback =
        make_shared<AudioCapturerDeviceChangeCallbackTest>();
    ret = audioCapturer ->SetAudioCapturerDeviceChangeCallback(callback);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioCapturer ->RemoveAudioCapturerDeviceChangeCallback(callback);
    EXPECT_EQ(SUCCESS, ret);

    AppInfo appInfo = {};
    std::unique_ptr<AudioCapturerPrivate> audioCapturerPrivate =
        std::make_unique<AudioCapturerPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);

    bool isDeviceChanged = audioCapturerPrivate->IsDeviceChanged(deviceInfo);
    EXPECT_EQ(false, isDeviceChanged);

    deviceInfo.deviceType = DEVICE_TYPE_EARPIECE;
    isDeviceChanged = audioCapturerPrivate->IsDeviceChanged(deviceInfo);
    EXPECT_EQ(false, isDeviceChanged);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    int32_t ret1 = -1;
    auto inputDeviceDescriptors = AudioSystemManager::GetInstance()->GetDevices(DeviceFlag::INPUT_DEVICES_FLAG);
    if (inputDeviceDescriptors.size() > 0) {
        auto microphoneDescriptors = audioCapturer->GetCurrentMicrophones();
        EXPECT_GT(microphoneDescriptors.size(), 0);
        auto micDescriptor = microphoneDescriptors[0];
        for (auto inputDescriptor : inputDeviceDescriptors) {
            if (micDescriptor->deviceType_ == inputDescriptor->deviceType_) {
                ret1 = SUCCESS;
            }
        }
        EXPECT_EQ(SUCCESS, ret1);
    }
    audioCapturerPrivate->Release();
    audioCapturer->Release();
}

/**
 * @tc.name  : Test RegisterAudioCapturerEventListener API stability.
 * @tc.number: Audio_Capturer_RegisterAudioCapturerEventListener_001
 * @tc.desc  : Test RegisterAudioCapturerEventListener interface stability.
 */
HWTEST(AudioCapturerUnitTest, Audio_Capturer_RegisterAudioCapturerEventListener_001, TestSize.Level1)
{
    int32_t ret = -1;
    AudioCapturerOptions capturerOptions;

    AudioCapturerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    ret = audioCapturer->RegisterAudioCapturerEventListener();
    EXPECT_EQ(SUCCESS, ret);

    ret = audioCapturer->UnregisterAudioCapturerEventListener();
    EXPECT_EQ(SUCCESS, ret);

    shared_ptr<AudioCapturerInfoChangeCallback> callback =
        make_shared<AudioCapturerInfoChangeCallbackTest>();
    ret = audioCapturer->SetAudioCapturerInfoChangeCallback(nullptr);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);

    ret = audioCapturer->SetAudioCapturerInfoChangeCallback(callback);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioCapturer->RemoveAudioCapturerInfoChangeCallback(callback);
    EXPECT_EQ(SUCCESS, ret);

    audioCapturer->SetValid(true);
    audioCapturer->Release();
}

/**
* @tc.name  : Test SetParams API via illegal input.
* @tc.number: Audio_Capturer_SetParams_008
* @tc.desc  : Test SetParams interface. Returns 0 {SUCCESS}, if the setting is successful.
*             capturerParams.audioSampleFormat = SAMPLE_S24LE;
*             capturerParams.samplingRate = SAMPLE_RATE_88200;
*             capturerParams.audioChannel = STEREO;
*             capturerParams.audioEncoding = ENCODING_PCM;
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_SetParams_008, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    AudioCapturerParams capturerParams;
    capturerParams.audioSampleFormat = SAMPLE_S24LE;
    capturerParams.samplingRate = SAMPLE_RATE_88200;
    capturerParams.audioChannel = STEREO;
    capturerParams.audioEncoding = ENCODING_PCM;

    int32_t ret = audioCapturer->SetParams(capturerParams);
    EXPECT_EQ(SUCCESS, ret);
    audioCapturer->Release();
}

/**
* @tc.name  : Test SetParams API via illegal input.
* @tc.number: Audio_Capturer_SetParams_009
* @tc.desc  : Test SetParams interface. Returns 0 {SUCCESS}, if the setting is successful.
*             capturerParams.audioSampleFormat = SAMPLE_S24LE;
*             capturerParams.samplingRate = SAMPLE_RATE_176400;
*             capturerParams.audioChannel = STEREO;
*             capturerParams.audioEncoding = ENCODING_PCM;
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_SetParams_009, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    AudioCapturerParams capturerParams;
    capturerParams.audioSampleFormat = SAMPLE_S24LE;
    capturerParams.samplingRate = SAMPLE_RATE_176400;
    capturerParams.audioChannel = STEREO;
    capturerParams.audioEncoding = ENCODING_PCM;

    int32_t ret = audioCapturer->SetParams(capturerParams);
    EXPECT_EQ(SUCCESS, ret);
    audioCapturer->Release();
}

/**
* @tc.name  : Test SetParams API via illegal input.
* @tc.number: Audio_Capturer_SetParams_010
* @tc.desc  : Test SetParams interface. Returns 0 {SUCCESS}, if the setting is successful.
*             capturerParams.audioSampleFormat = SAMPLE_S24LE;
*             capturerParams.samplingRate = SAMPLE_RATE_192000;
*             capturerParams.audioChannel = STEREO;
*             capturerParams.audioEncoding = ENCODING_PCM;
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_SetParams_010, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    AudioCapturerParams capturerParams;
    capturerParams.audioSampleFormat = SAMPLE_S24LE;
    capturerParams.samplingRate = SAMPLE_RATE_192000;
    capturerParams.audioChannel = STEREO;
    capturerParams.audioEncoding = ENCODING_PCM;

    int32_t ret = audioCapturer->SetParams(capturerParams);
    EXPECT_EQ(SUCCESS, ret);
    audioCapturer->Release();
}

/**
 * @tc.name  : Test GetOverflowCount
 * @tc.number: Audio_Capturer_GetOverflowCount_001
 * @tc.desc  : Test GetOverflowCount interface get underflow value.
 */
HWTEST(AudioCapturerUnitTest, Audio_Capturer_GetOverflowCount_001, TestSize.Level1)
{
    AudioCapturerOptions capturerOptions;
    AudioCapturerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    auto overflowClount = audioCapturer->GetOverflowCount();

    EXPECT_GE(overflowClount, 0);

    audioCapturer->Stop();
    audioCapturer->Release();
}
} // namespace AudioStandard
} // namespace OHOS
