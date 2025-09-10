/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <OpenSLES_OpenHarmony.h>
#include <OpenSLES_Platform.h>
#include "fast_audio_stream.h"
#include "audiocapturer_adapter.h"
#include "audio_capturer_private.h"
#include "audiocapturer_adapter_unit_test.h"
#include "common.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioCapturerAdapterUnitTest::SetUpTestCase(void) { }

void AudioCapturerAdapterUnitTest::TearDownTestCase(void) { }

void AudioCapturerAdapterUnitTest::SetUp(void) { }

void AudioCapturerAdapterUnitTest::TearDown(void) { }

/**
* @tc.name  : Test AudioCapturerAdapter API
* @tc.type  : FUNC
* @tc.number: GetAudioCapturerById_001
* @tc.desc  : Test GetAudioCapturerById interface.
*/
HWTEST(AudioCapturerAdapterUnitTest, GetAudioCapturerById_001, TestSize.Level0)
{
    SLuint32 id = 0;
    AudioCapturerAdapter::GetInstance()->captureMap_.clear();
    auto ret = AudioCapturerAdapter::GetInstance()->GetAudioCapturerById(id);
    EXPECT_EQ(ret, nullptr);

    AppInfo appInfo = {};
    AudioStreamType audioStreamType = STREAM_MUSIC;
    auto audioCapturer = std::make_shared<AudioCapturerPrivate>(audioStreamType, appInfo, false);
    AudioCapturerAdapter::GetInstance()->captureMap_[id] = audioCapturer;
    ret = AudioCapturerAdapter::GetInstance()->GetAudioCapturerById(id);
    EXPECT_NE(ret, nullptr);
}

/**
* @tc.name  : Test AudioCapturerAdapter API
* @tc.type  : FUNC
* @tc.number: SetCaptureStateAdapter_001
* @tc.desc  : Test SetCaptureStateAdapter interface.
*/
HWTEST(AudioCapturerAdapterUnitTest, SetCaptureStateAdapter_001, TestSize.Level0)
{
    SLuint32 id = 10;
    SLuint32 state = SL_RECORDSTATE_RECORDING;
    AudioMode eMode = AUDIO_MODE_PLAYBACK;
    AudioStreamType eStreamType = STREAM_MUSIC;
    int32_t appUid = 0;

    AppInfo appInfo = {};
    AudioStreamType audioStreamType = STREAM_MUSIC;
    AudioCapturerAdapter::GetInstance()->captureMap_.clear();
    auto audioCapturer = std::make_shared<AudioCapturerPrivate>(audioStreamType, appInfo, false);
    ASSERT_TRUE(audioCapturer != nullptr);
    auto fastAudioStream = std::make_shared<FastAudioStream>(eStreamType, eMode, appUid);
    ASSERT_TRUE(fastAudioStream != nullptr);

    audioCapturer->audioStream_ = fastAudioStream;
    AudioCapturerAdapter::GetInstance()->captureMap_[id] = audioCapturer;

    auto ret = AudioCapturerAdapter::GetInstance()->SetCaptureStateAdapter(id, state);
    EXPECT_EQ(ret, SL_RESULT_RESOURCE_ERROR);
}

/**
* @tc.name  : Test AudioCapturerAdapter API
* @tc.type  : FUNC
* @tc.number: SetCaptureStateAdapter_002
* @tc.desc  : Test SetCaptureStateAdapter interface.
*/
HWTEST(AudioCapturerAdapterUnitTest, SetCaptureStateAdapter_002, TestSize.Level0)
{
    SLuint32 id = 10;
    SLuint32 state = SL_RECORDSTATE_STOPPED;
    AudioMode eMode = AUDIO_MODE_PLAYBACK;
    AudioStreamType eStreamType = STREAM_MUSIC;
    int32_t appUid = 0;

    AppInfo appInfo = {};
    AudioStreamType audioStreamType = STREAM_MUSIC;
    AudioCapturerAdapter::GetInstance()->captureMap_.clear();
    auto audioCapturer = std::make_shared<AudioCapturerPrivate>(audioStreamType, appInfo, false);
    ASSERT_TRUE(audioCapturer != nullptr);
    auto fastAudioStream = std::make_shared<FastAudioStream>(eStreamType, eMode, appUid);
    ASSERT_TRUE(fastAudioStream != nullptr);

    audioCapturer->audioStream_ = fastAudioStream;
    AudioCapturerAdapter::GetInstance()->captureMap_[id] = audioCapturer;
    auto ret = AudioCapturerAdapter::GetInstance()->SetCaptureStateAdapter(id, state);
    EXPECT_EQ(ret, SL_RESULT_RESOURCE_ERROR);

    state = -1;
    ret = AudioCapturerAdapter::GetInstance()->SetCaptureStateAdapter(id, state);
    EXPECT_EQ(ret, SL_RESULT_RESOURCE_ERROR);
}

/**
* @tc.name  : Test AudioCapturerAdapter APIPf
* @tc.number: GetCaptureStateAdapter_001
* @tc.desc  : Test GetCaptureStateAdapter interface.
*/
HWTEST(AudioCapturerAdapterUnitTest, GetCaptureStateAdapter_001, TestSize.Level0)
{
    SLuint32 id = 1;
    SLuint32 stateTest;

    AppInfo appInfo = {};
    AudioStreamType audioStreamType = STREAM_MUSIC;
    AudioCapturerAdapter::GetInstance()->captureMap_.clear();
    auto audioCapturer = std::make_shared<AudioCapturerPrivate>(audioStreamType, appInfo, false);
    ASSERT_TRUE(audioCapturer != nullptr);
    audioCapturer->audioStream_ = std::make_shared<TestAudioStreamStub>();
    ASSERT_TRUE(audioCapturer->audioStream_ != nullptr);
    AudioCapturerAdapter::GetInstance()->captureMap_[id] = audioCapturer;
    AudioCapturerAdapter::GetInstance()->GetCaptureStateAdapter(id, &stateTest);
    EXPECT_TRUE(stateTest == SL_RECORDSTATE_RECORDING);
}

/**
* @tc.name  : Test AudioCapturerAdapter APIPf
* @tc.number: GetCaptureStateAdapter_002
* @tc.desc  : Test GetCaptureStateAdapter interface.
*/
HWTEST(AudioCapturerAdapterUnitTest, GetCaptureStateAdapter_002, TestSize.Level0)
{
    SLuint32 id = 1;
    SLuint32 stateTest;

    AppInfo appInfo = {};
    AudioStreamType audioStreamType = STREAM_MUSIC;
    AudioCapturerAdapter::GetInstance()->captureMap_.clear();
    auto audioCapturer = std::make_shared<AudioCapturerPrivate>(audioStreamType, appInfo, false);
    ASSERT_TRUE(audioCapturer != nullptr);
    audioCapturer->audioStream_ = std::make_shared<TestAudioStreamStub1>();
    ASSERT_TRUE(audioCapturer->audioStream_ != nullptr);
    AudioCapturerAdapter::GetInstance()->captureMap_[id] = audioCapturer;
    AudioCapturerAdapter::GetInstance()->GetCaptureStateAdapter(id, &stateTest);
    EXPECT_TRUE(stateTest == SL_RECORDSTATE_PAUSED);
}

/**
* @tc.name  : Test AudioCapturerAdapter APIPf
* @tc.number: GetCaptureStateAdapter_003
* @tc.desc  : Test GetCaptureStateAdapter interface.
*/
HWTEST(AudioCapturerAdapterUnitTest, GetCaptureStateAdapter_003, TestSize.Level0)
{
    SLuint32 id = 1;
    SLuint32 stateTest;

    AppInfo appInfo = {};
    AudioStreamType audioStreamType = STREAM_MUSIC;
    AudioCapturerAdapter::GetInstance()->captureMap_.clear();
    auto audioCapturer = std::make_shared<AudioCapturerPrivate>(audioStreamType, appInfo, false);
    ASSERT_TRUE(audioCapturer != nullptr);
    audioCapturer->audioStream_ = std::make_shared<TestAudioStreamStub2>();
    ASSERT_TRUE(audioCapturer->audioStream_ != nullptr);
    AudioCapturerAdapter::GetInstance()->captureMap_[id] = audioCapturer;
    AudioCapturerAdapter::GetInstance()->GetCaptureStateAdapter(id, &stateTest);
    EXPECT_TRUE(stateTest == SL_RECORDSTATE_STOPPED);
}

/**
* @tc.name  : Test AudioCapturerAdapter APIPf
* @tc.number: GetCaptureStateAdapter_004
* @tc.desc  : Test GetCaptureStateAdapter interface.
*/
HWTEST(AudioCapturerAdapterUnitTest, GetCaptureStateAdapter_004, TestSize.Level0)
{
    SLuint32 id = 1;
    SLuint32 stateTest;
    AudioMode eMode = AUDIO_MODE_PLAYBACK;
    AudioStreamType eStreamType = STREAM_MUSIC;
    int32_t appUid = 0;

    AppInfo appInfo = {};
    AudioStreamType audioStreamType = STREAM_MUSIC;
    AudioCapturerAdapter::GetInstance()->captureMap_.clear();
    auto audioCapturer = std::make_shared<AudioCapturerPrivate>(audioStreamType, appInfo, false);
    ASSERT_TRUE(audioCapturer != nullptr);
    auto fastAudioStream = std::make_shared<FastAudioStream>(eStreamType, eMode, appUid);
    ASSERT_TRUE(fastAudioStream != nullptr);
    fastAudioStream->state_ = State::NEW;
    audioCapturer->audioStream_ = fastAudioStream;
    AudioCapturerAdapter::GetInstance()->captureMap_[id] = audioCapturer;
    AudioCapturerAdapter::GetInstance()->GetCaptureStateAdapter(id, &stateTest);
    EXPECT_TRUE(stateTest == -1);
}

/**
* @tc.name  : Test AudioCapturerAdapter APIPf
* @tc.number: SlToOhosSampelFormat_001
* @tc.desc  : Test SlToOhosSampelFormat interface.
*/
HWTEST(AudioCapturerAdapterUnitTest, SlToOhosSampelFormat_001, TestSize.Level0)
{
    SLDataFormat_PCM pcm = {
        SL_DATAFORMAT_PCM,
        AudioChannel::MONO,
        SL_SAMPLINGRATE_8,
        SL_PCMSAMPLEFORMAT_FIXED_8,
        0,
        0,
        0
    };

    SLDataFormat_PCM *pcmFormat = &pcm;
    pcmFormat->bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_8;
    auto ret = AudioCapturerAdapter::GetInstance()->SlToOhosSampelFormat(pcmFormat);
    EXPECT_EQ(SAMPLE_U8, ret);

    pcmFormat->bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
    ret = AudioCapturerAdapter::GetInstance()->SlToOhosSampelFormat(pcmFormat);
    EXPECT_EQ(SAMPLE_S16LE, ret);

    pcmFormat->bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_20;
    ret = AudioCapturerAdapter::GetInstance()->SlToOhosSampelFormat(pcmFormat);
    EXPECT_EQ(INVALID_WIDTH, ret);

    pcmFormat->bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_24;
    ret = AudioCapturerAdapter::GetInstance()->SlToOhosSampelFormat(pcmFormat);
    EXPECT_EQ(SAMPLE_S24LE, ret);

    pcmFormat->bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_28;
    ret = AudioCapturerAdapter::GetInstance()->SlToOhosSampelFormat(pcmFormat);
    EXPECT_EQ(INVALID_WIDTH, ret);

    pcmFormat->bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_32;
    ret = AudioCapturerAdapter::GetInstance()->SlToOhosSampelFormat(pcmFormat);
    EXPECT_EQ(SAMPLE_S32LE, ret);

    pcmFormat->bitsPerSample = -1;
    ret = AudioCapturerAdapter::GetInstance()->SlToOhosSampelFormat(pcmFormat);
    EXPECT_EQ(INVALID_WIDTH, ret);
}

/**
* @tc.name  : Test AudioCapturerAdapter APIPf
* @tc.number: SlToOhosSamplingRate_001
* @tc.desc  : Test SlToOhosSamplingRate interface.
*/
HWTEST(AudioCapturerAdapterUnitTest, SlToOhosSamplingRate_001, TestSize.Level0)
{
    SLDataFormat_PCM pcm = {
        SL_DATAFORMAT_PCM,
        AudioChannel::MONO,
        SL_SAMPLINGRATE_8,
        SL_PCMSAMPLEFORMAT_FIXED_8,
        0,
        0,
        0
    };

    SLDataFormat_PCM *pcmFormat = &pcm;
    pcmFormat->samplesPerSec = SL_SAMPLINGRATE_8;
    auto ret = AudioCapturerAdapter::GetInstance()->SlToOhosSamplingRate(pcmFormat);
    EXPECT_EQ(SAMPLE_RATE_8000, ret);

    pcmFormat->samplesPerSec = SL_SAMPLINGRATE_11_025;
    ret = AudioCapturerAdapter::GetInstance()->SlToOhosSamplingRate(pcmFormat);
    EXPECT_EQ(SAMPLE_RATE_11025, ret);

    pcmFormat->samplesPerSec = SL_SAMPLINGRATE_12;
    ret = AudioCapturerAdapter::GetInstance()->SlToOhosSamplingRate(pcmFormat);
    EXPECT_EQ(SAMPLE_RATE_12000, ret);

    pcmFormat->samplesPerSec = SL_SAMPLINGRATE_16;
    ret = AudioCapturerAdapter::GetInstance()->SlToOhosSamplingRate(pcmFormat);
    EXPECT_EQ(SAMPLE_RATE_16000, ret);

    pcmFormat->samplesPerSec = SL_SAMPLINGRATE_22_05;
    ret = AudioCapturerAdapter::GetInstance()->SlToOhosSamplingRate(pcmFormat);
    EXPECT_EQ(SAMPLE_RATE_22050, ret);

    pcmFormat->samplesPerSec = SL_SAMPLINGRATE_24;
    ret = AudioCapturerAdapter::GetInstance()->SlToOhosSamplingRate(pcmFormat);
    EXPECT_EQ(SAMPLE_RATE_24000, ret);

    pcmFormat->samplesPerSec = -1;
    ret = AudioCapturerAdapter::GetInstance()->SlToOhosSamplingRate(pcmFormat);
    EXPECT_EQ(SAMPLE_RATE_44100, ret);
}

/**
* @tc.name  : Test AudioCapturerAdapter APIPf
* @tc.number: SlToOhosSamplingRate_002
* @tc.desc  : Test SlToOhosSamplingRate interface.
*/
HWTEST(AudioCapturerAdapterUnitTest, SlToOhosSamplingRate_002, TestSize.Level0)
{
    SLDataFormat_PCM pcm = {
        SL_DATAFORMAT_PCM,
        AudioChannel::MONO,
        SL_SAMPLINGRATE_8,
        SL_PCMSAMPLEFORMAT_FIXED_8,
        0,
        0,
        0
    };
    SLDataFormat_PCM *pcmFormat = &pcm;
    pcmFormat->samplesPerSec = SL_SAMPLINGRATE_32;
    auto ret = AudioCapturerAdapter::GetInstance()->SlToOhosSamplingRate(pcmFormat);
    EXPECT_EQ(SAMPLE_RATE_32000, ret);

    pcmFormat->samplesPerSec = SL_SAMPLINGRATE_44_1;
    ret = AudioCapturerAdapter::GetInstance()->SlToOhosSamplingRate(pcmFormat);
    EXPECT_EQ(SAMPLE_RATE_44100, ret);

    pcmFormat->samplesPerSec = SL_SAMPLINGRATE_48;
    ret = AudioCapturerAdapter::GetInstance()->SlToOhosSamplingRate(pcmFormat);
    EXPECT_EQ(SAMPLE_RATE_48000, ret);

    pcmFormat->samplesPerSec = SL_SAMPLINGRATE_64;
    ret = AudioCapturerAdapter::GetInstance()->SlToOhosSamplingRate(pcmFormat);
    EXPECT_EQ(SAMPLE_RATE_64000, ret);

    pcmFormat->samplesPerSec = SL_SAMPLINGRATE_88_2;
    ret = AudioCapturerAdapter::GetInstance()->SlToOhosSamplingRate(pcmFormat);
    EXPECT_EQ(SAMPLE_RATE_44100, ret);

    pcmFormat->samplesPerSec = SL_SAMPLINGRATE_96;
    ret = AudioCapturerAdapter::GetInstance()->SlToOhosSamplingRate(pcmFormat);
    EXPECT_EQ(SAMPLE_RATE_96000, ret);

    pcmFormat->samplesPerSec = SL_SAMPLINGRATE_192;
    ret = AudioCapturerAdapter::GetInstance()->SlToOhosSamplingRate(pcmFormat);
    EXPECT_EQ(SAMPLE_RATE_44100, ret);
}

/**
* @tc.name  : Test AudioCapturerAdapter APIPf
* @tc.number: SlToOhosChannel_001
* @tc.desc  : Test SlToOhosChannel interface.
*/
HWTEST(AudioCapturerAdapterUnitTest, SlToOhosChannel_001, TestSize.Level0)
{
    SLDataFormat_PCM pcm = {
        SL_DATAFORMAT_PCM,
        AudioChannel::MONO,
        SL_SAMPLINGRATE_8,
        SL_PCMSAMPLEFORMAT_FIXED_8,
        0,
        0,
        0
    };
    SLDataFormat_PCM *pcmFormat = &pcm;

    pcmFormat->numChannels = MONO;
    auto ret = AudioCapturerAdapter::GetInstance()->SlToOhosChannel(pcmFormat);
    EXPECT_EQ(ret, MONO);

    pcmFormat->numChannels = STEREO;
    ret = AudioCapturerAdapter::GetInstance()->SlToOhosChannel(pcmFormat);
    EXPECT_EQ(ret, STEREO);

    pcmFormat->numChannels = CHANNEL_3;
    ret = AudioCapturerAdapter::GetInstance()->SlToOhosChannel(pcmFormat);
    EXPECT_EQ(ret, MONO);
}

/**
* @tc.name  : Test AudioRecorderDestroy
* @tc.number: AudioRecorderDestroy_001
* @tc.desc  : Test AudioRecorderDestroy interface.
*/
HWTEST(AudioCapturerAdapterUnitTest, AudioRecorderDestroy_001, TestSize.Level0)
{
    void *self = new CAudioRecorder();
    SLresult result = AudioRecorderDestroy(nullptr);
    EXPECT_TRUE(result == SL_RESULT_PARAMETER_INVALID);

    result = AudioRecorderDestroy(self);
    EXPECT_EQ(result, SL_RESULT_SUCCESS);
}

/**
* @tc.name  : Test AudioRecorderDestroy
* @tc.number: AudioPlayerDestroy_001
* @tc.desc  : Test AudioRecorderDestroy interface.
*/
HWTEST(AudioCapturerAdapterUnitTest, AudioPlayerDestroy_001, TestSize.Level3)
{
    void *self = new CAudioPlayer();
    SLresult result = AudioPlayerDestroy(nullptr);
    EXPECT_TRUE(result == SL_RESULT_PARAMETER_INVALID);
 
    result = AudioPlayerDestroy(self);
    EXPECT_EQ(result, SL_RESULT_SUCCESS);
}

/**
* @tc.name  : Test AudioCapturerAdapter API
* @tc.type  : FUNC
* @tc.number: SetCaptureStateAdapter_003
* @tc.desc  : Test SetCaptureStateAdapter interface.
*/
HWTEST(AudioCapturerAdapterUnitTest, SetCaptureStateAdapter_003, TestSize.Level0)
{
    SLuint32 id = 10;
    SLuint32 state = SL_RECORDSTATE_PAUSED;
    AudioMode eMode = AUDIO_MODE_PLAYBACK;
    AudioStreamType eStreamType = STREAM_MUSIC;
    int32_t appUid = 0;

    AppInfo appInfo = {};
    AudioStreamType audioStreamType = STREAM_MUSIC;
    AudioCapturerAdapter::GetInstance()->captureMap_.clear();
    auto audioCapturer = std::make_shared<AudioCapturerPrivate>(audioStreamType, appInfo, false);
    ASSERT_TRUE(audioCapturer != nullptr);
    auto fastAudioStream = std::make_shared<FastAudioStream>(eStreamType, eMode, appUid);
    ASSERT_TRUE(fastAudioStream != nullptr);

    audioCapturer->audioStream_ = fastAudioStream;
    AudioCapturerAdapter::GetInstance()->captureMap_[id] = audioCapturer;
    auto ret = AudioCapturerAdapter::GetInstance()->SetCaptureStateAdapter(id, state);
    EXPECT_EQ(ret, SL_RESULT_RESOURCE_ERROR);

    state = -1;
    ret = AudioCapturerAdapter::GetInstance()->SetCaptureStateAdapter(id, state);
    EXPECT_EQ(ret, SL_RESULT_RESOURCE_ERROR);
}
} // namespace AudioStandard
} // namespace OHOS
