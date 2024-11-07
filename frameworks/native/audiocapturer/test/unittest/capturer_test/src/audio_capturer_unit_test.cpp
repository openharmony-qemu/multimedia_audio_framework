/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace {
    const string AUDIO_CAPTURE_FILE1 = "/data/audiocapturetest_blocking.pcm";
    const string AUDIO_CAPTURE_FILE2 = "/data/audiocapturetest_nonblocking.pcm";
    const string AUDIO_TIME_STABILITY_TEST_FILE = "/data/audiocapture_getaudiotime_stability_test.pcm";
    const string AUDIO_PLAYBACK_CAPTURER_TEST_FILE = "/data/audiocapturer_playbackcapturer_test.pcm";
    const int32_t READ_BUFFERS_COUNT = 128;
    const int32_t VALUE_ZERO = 0;
    const int32_t VALUE_HUNDRED = 100;
    const int32_t STRESS_TEST_COUNTS = 200;
    const int32_t VALUE_THOUSAND = 1000;
    const int32_t CAPTURER_FLAG = 0;
} // namespace

void AudioCapturerUnitTest::SetUpTestCase(void) {}
void AudioCapturerUnitTest::TearDownTestCase(void) {}
void AudioCapturerUnitTest::SetUp(void) {}
void AudioCapturerUnitTest::TearDown(void) {}

int32_t AudioCapturerUnitTest::InitializeCapturer(unique_ptr<AudioCapturer> &audioCapturer)
{
    AudioCapturerParams capturerParams;
    capturerParams.audioSampleFormat = SAMPLE_S16LE;
    capturerParams.samplingRate = SAMPLE_RATE_44100;
    capturerParams.audioChannel = STEREO;
    capturerParams.audioEncoding = ENCODING_PCM;

    return audioCapturer->SetParams(capturerParams);
}

void AudioCapturerUnitTest::InitializeCapturerOptions(AudioCapturerOptions &capturerOptions)
{
    capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_96000;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_U8;
    capturerOptions.streamInfo.channels = AudioChannel::MONO;
    capturerOptions.capturerInfo.sourceType = SourceType::SOURCE_TYPE_MIC;
    capturerOptions.capturerInfo.capturerFlags = CAPTURER_FLAG;
    return;
}

void AudioCapturerUnitTest::InitializePlaybackCapturerOptions(AudioCapturerOptions &capturerOptions)
{
    capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S32LE;
    capturerOptions.streamInfo.channels = AudioChannel::STEREO;
    capturerOptions.capturerInfo.sourceType = SourceType::SOURCE_TYPE_PLAYBACK_CAPTURE;
    capturerOptions.capturerInfo.capturerFlags = CAPTURER_FLAG;
    return;
}

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

    fclose(capFile);
}

/**
* @tc.name  : Test GetSupportedFormats API
* @tc.number: Audio_Capturer_GetSupportedFormats_001
* @tc.desc  : Test GetSupportedFormats interface. Returns supported Formats on success.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_GetSupportedFormats_001, TestSize.Level0)
{
    vector<AudioSampleFormat> supportedFormatList = AudioCapturer::GetSupportedFormats();
    EXPECT_EQ(AUDIO_SUPPORTED_FORMATS.size(), supportedFormatList.size());
}

/**
* @tc.name  : Test GetSupportedChannels API
* @tc.number: Audio_Capturer_GetSupportedChannels_001
* @tc.desc  : Test GetSupportedChannels interface. Returns supported Channels on success.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_GetSupportedChannels_001, TestSize.Level0)
{
    vector<AudioChannel> supportedChannelList = AudioCapturer::GetSupportedChannels();
    EXPECT_EQ(CAPTURER_SUPPORTED_CHANNELS.size(), supportedChannelList.size());
}

/**
* @tc.name  : Test GetSupportedEncodingTypes API
* @tc.number: Audio_Capturer_GetSupportedEncodingTypes_001
* @tc.desc  : Test GetSupportedEncodingTypes interface. Returns supported Encoding types on success.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_GetSupportedEncodingTypes_001, TestSize.Level0)
{
    vector<AudioEncodingType> supportedEncodingTypes
                                        = AudioCapturer::GetSupportedEncodingTypes();
    EXPECT_EQ(AUDIO_SUPPORTED_ENCODING_TYPES.size(), supportedEncodingTypes.size());
}

/**
* @tc.name  : Test GetSupportedSamplingRates API
* @tc.number: Audio_Capturer_GetSupportedSamplingRates_001
* @tc.desc  : Test GetSupportedSamplingRates interface. Returns supported Sampling rates on success.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_GetSupportedSamplingRates_001, TestSize.Level0)
{
    vector<AudioSamplingRate> supportedSamplingRates = AudioCapturer::GetSupportedSamplingRates();
    EXPECT_EQ(AUDIO_SUPPORTED_SAMPLING_RATES.size(), supportedSamplingRates.size());
}

/**
* @tc.name  : Test Create API via legal input.
* @tc.number: Audio_Capturer_Create_001
* @tc.desc  : Test Create interface with STREAM_MUSIC. Returns audioCapturer instance, if create is successful.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Create_001, TestSize.Level0)
{
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    EXPECT_NE(nullptr, audioCapturer);
}

/**
* @tc.name  : Test Create API via legal input.
* @tc.number: Audio_Capturer_Create_002
* @tc.desc  : Test Create interface with STREAM_RING. Returns audioCapturer instance, if create is successful.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Create_002, TestSize.Level0)
{
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_RING);
    EXPECT_NE(nullptr, audioCapturer);
}

/**
* @tc.name  : Test Create API via legal input.
* @tc.number: Audio_Capturer_Create_003
* @tc.desc  : Test Create interface with STREAM_VOICE_CALL. Returns audioCapturer instance if create is successful.
*             Note: instance will be created but functional support for STREAM_VOICE_CALL not available yet.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Create_003, TestSize.Level0)
{
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_VOICE_CALL);
    EXPECT_NE(nullptr, audioCapturer);
}

/**
* @tc.name  : Test Create API via legal input.
* @tc.number: Audio_Capturer_Create_004
* @tc.desc  : Test Create interface with STREAM_SYSTEM. Returns audioCapturer instance, if create is successful.
*             Note: instance will be created but functional support for STREAM_SYSTEM not available yet.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Create_004, TestSize.Level0)
{
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_SYSTEM);
    EXPECT_NE(nullptr, audioCapturer);
}

/**
* @tc.name  : Test Create API via legal input.
* @tc.number: Audio_Capturer_Create_005
* @tc.desc  : Test Create interface with STREAM_BLUETOOTH_SCO. Returns audioCapturer instance, if create is successful.
*             Note: instance will be created but functional support for STREAM_BLUETOOTH_SCO not available yet
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Create_005, TestSize.Level0)
{
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_BLUETOOTH_SCO);
    EXPECT_NE(nullptr, audioCapturer);
}

/**
* @tc.name  : Test Create API via legal input.
* @tc.number: Audio_Capturer_Create_006
* @tc.desc  : Test Create interface with STREAM_ALARM. Returns audioCapturer instance, if create is successful.
*             Note: instance will be created but functional support for STREAM_ALARM not available yet.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Create_006, TestSize.Level0)
{
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_ALARM);
    EXPECT_NE(nullptr, audioCapturer);
}

/**
* @tc.name  : Test Create API via legal input.
* @tc.number: Audio_Capturer_Create_007
* @tc.desc  : Test Create interface with STREAM_NOTIFICATION. Returns audioCapturer instance, if create is successful.
*             Note: instance will be created but functional support for STREAM_NOTIFICATION not available yet.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Create_007, TestSize.Level0)
{
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_NOTIFICATION);
    EXPECT_NE(nullptr, audioCapturer);
}

/**
* @tc.name  : Test Create API via legal input.
* @tc.number: Audio_Capturer_Create_008
* @tc.desc  : Test Create interface with AudioCapturerOptions below.
*             Returns audioCapturer instance, if create is successful.
*             capturerOptions.streamInfo.samplingRate = SAMPLE_RATE_96000;
*             capturerOptions.streamInfo.encoding = ENCODING_PCM;
*             capturerOptions.streamInfo.format = SAMPLE_U8;
*             capturerOptions.streamInfo.channels = MONO;
*             capturerOptions.capturerInfo.sourceType = SOURCE_TYPE_MIC;
*             capturerOptions.capturerInfo.capturerFlags = CAPTURER_FLAG;
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Create_008, TestSize.Level0)
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
    audioCapturer->Release();
}

/**
* @tc.name  : Test Create API via legal input.
* @tc.number: Audio_Capturer_Create_009
* @tc.desc  : Test Create interface with AudioCapturerOptions below.
*             Returns audioCapturer instance, if create is successful.
*             capturerOptions.streamInfo.samplingRate = SAMPLE_RATE_96000;
*             capturerOptions.streamInfo.encoding = ENCODING_PCM;
*             capturerOptions.streamInfo.format = SAMPLE_U8;
*             capturerOptions.streamInfo.channels = MONO;
*             capturerOptions.capturerInfo.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
*             capturerOptions.capturerInfo.capturerFlags = CAPTURER_FLAG;
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Create_009, TestSize.Level0)
{
    AudioCapturerOptions capturerOptions;
    capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_96000;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_U8;
    capturerOptions.streamInfo.channels = AudioChannel::MONO;
    capturerOptions.capturerInfo.sourceType = SourceType::SOURCE_TYPE_VOICE_COMMUNICATION;
    capturerOptions.capturerInfo.capturerFlags = CAPTURER_FLAG;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);
    audioCapturer->Release();
}

/**
* @tc.name  : Test Create API via legal input.
* @tc.number: Audio_Capturer_Create_010
* @tc.desc  : Test Create interface with AudioCapturerOptions below.
*             Returns audioCapturer instance, if create is successful.
*             capturerOptions.streamInfo.samplingRate = SAMPLE_RATE_96000;
*             capturerOptions.streamInfo.encoding = ENCODING_PCM;
*             capturerOptions.streamInfo.format = SAMPLE_S32LE;
*             capturerOptions.streamInfo.channels = MONO;
*             capturerOptions.capturerInfo.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
*             capturerOptions.capturerInfo.capturerFlags = CAPTURER_FLAG;
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Create_010, TestSize.Level0)
{
    AudioCapturerOptions capturerOptions;
    capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_64000;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S32LE;
    capturerOptions.streamInfo.channels = AudioChannel::MONO;
    capturerOptions.capturerInfo.sourceType = SourceType::SOURCE_TYPE_VOICE_COMMUNICATION;
    capturerOptions.capturerInfo.capturerFlags = CAPTURER_FLAG;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);
    audioCapturer->Release();
}

/**
* @tc.name  : Test Create API via legal input.
* @tc.number: Audio_Capturer_Create_011
* @tc.desc  : Test Create interface with AudioCapturerOptions below.
*             Returns audioCapturer instance, if create is successful.
*             capturerOptions.streamInfo.samplingRate = SAMPLE_RATE_64000;
*             capturerOptions.streamInfo.encoding = ENCODING_PCM;
*             capturerOptions.streamInfo.format = SAMPLE_S32LE;
*             capturerOptions.streamInfo.channels = STEREO;
*             capturerOptions.capturerInfo.sourceType = SOURCE_TYPE_MIC;
*             capturerOptions.capturerInfo.capturerFlags = CAPTURER_FLAG;
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Create_011, TestSize.Level0)
{
    AudioCapturerOptions capturerOptions;
    capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_64000;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S32LE;
    capturerOptions.streamInfo.channels = AudioChannel::STEREO;
    capturerOptions.capturerInfo.sourceType = SourceType::SOURCE_TYPE_MIC;
    capturerOptions.capturerInfo.capturerFlags = CAPTURER_FLAG;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);
    audioCapturer->Release();
}

/**
* @tc.name  : Test Create API via legal input.
* @tc.number: Audio_Capturer_Create_012
* @tc.desc  : Test Create interface with AudioCapturerOptions below.
*             Returns audioCapturer instance, if create is successful.
*             capturerOptions.streamInfo.samplingRate = SAMPLE_RATE_44100;
*             capturerOptions.streamInfo.encoding = ENCODING_PCM;
*             capturerOptions.streamInfo.format = SAMPLE_S16LE;
*             capturerOptions.streamInfo.channels = STEREO;
*             capturerOptions.capturerInfo.sourceType = SOURCE_TYPE_MIC;
*             capturerOptions.capturerInfo.capturerFlags = CAPTURER_FLAG;
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Create_012, TestSize.Level0)
{
    AudioCapturerOptions capturerOptions;
    capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    capturerOptions.streamInfo.channels = AudioChannel::STEREO;
    capturerOptions.capturerInfo.sourceType = SourceType::SOURCE_TYPE_MIC;
    capturerOptions.capturerInfo.capturerFlags = CAPTURER_FLAG;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);
    audioCapturer->Release();
}

/**
* @tc.name  : Test Create API via legal input.
* @tc.number: Audio_Capturer_Create_013
* @tc.desc  : Test Create interface with STREAM_MUSIC and appInfo. Returns audioCapturer instance, if successful.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Create_013, TestSize.Level0)
{
    AppInfo appInfo = {};
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC, appInfo);
    ASSERT_NE(nullptr, audioCapturer);

    AudioCapturerParams capturerParams;
    capturerParams.audioSampleFormat = SAMPLE_S16LE;
    capturerParams.samplingRate = SAMPLE_RATE_44100;
    capturerParams.audioChannel = STEREO;
    capturerParams.audioEncoding = ENCODING_PCM;

    int32_t ret = audioCapturer->SetParams(capturerParams);
    EXPECT_EQ(SUCCESS, ret);
    audioCapturer->Release();
}

/**
* @tc.name  : Test Create API via legal input.
* @tc.number: Audio_Capturer_Create_014
* @tc.desc  : Test Create interface with STREAM_MUSIC and appInfo. Returns audioCapturer instance, if successful.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Create_014, TestSize.Level0)
{
    AppInfo appInfo = {};
    appInfo.appTokenId = VALUE_THOUSAND;
    appInfo.appUid = static_cast<int32_t>(getuid());
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC, appInfo);
    ASSERT_NE(nullptr, audioCapturer);

    AudioCapturerParams capturerParams;
    capturerParams.audioSampleFormat = SAMPLE_S16LE;
    capturerParams.samplingRate = SAMPLE_RATE_44100;
    capturerParams.audioChannel = STEREO;
    capturerParams.audioEncoding = ENCODING_PCM;

    int32_t ret = audioCapturer->SetParams(capturerParams);
    EXPECT_EQ(SUCCESS, ret);
    audioCapturer->Release();
}

/**
 * @tc.name  : Test Create API via legal input.
 * @tc.number: Audio_Capturer_Create_015
 * @tc.desc  : Test Create function with two types of parameters: AudioCapturerOptions and AppInfo.
 */
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Create_015, TestSize.Level0)
{
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
    audioCapturer->Release();
}

/**
 * @tc.name  : Test Create API via legal input.
 * @tc.number: Audio_Capturer_Create_016
 * @tc.desc  : Test Create function with two types of parameters: AudioCapturerOptions and AppInfo，
 *             and give different parameters.
 */
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Create_016, TestSize.Level0)
{
    AudioCapturerOptions capturerOptions;
    AppInfo appInfo = {};
    capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_16000;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    capturerOptions.streamInfo.channels = AudioChannel::MONO;
    capturerOptions.capturerInfo.sourceType = SourceType::SOURCE_TYPE_VOICE_RECOGNITION;
    capturerOptions.capturerInfo.capturerFlags = CAPTURER_FLAG;
    appInfo.appTokenId = VALUE_THOUSAND;
    appInfo.appUid = static_cast<int32_t>(getuid());
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions, appInfo);
    ASSERT_NE(nullptr, audioCapturer);
    audioCapturer->Release();
}

/**
 * @tc.name  : Test Create API via legal input.
 * @tc.number: Audio_Capturer_Create_017
 * @tc.desc  : Test Create function with two types of parameters: AudioCapturerOptions and string.
 */
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Create_017, TestSize.Level0)
{
    AudioCapturerOptions capturerOptions;
    string cachePath = "/data/storage/el2/base/temp";
    capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_16000;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    capturerOptions.streamInfo.channels = AudioChannel::MONO;
    capturerOptions.capturerInfo.sourceType = SourceType::SOURCE_TYPE_VOICE_RECOGNITION;
    capturerOptions.capturerInfo.capturerFlags = CAPTURER_FLAG;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions, cachePath.c_str());
    ASSERT_NE(nullptr, audioCapturer);
    audioCapturer->Release();
}

/**
 * @tc.name  : Test Create API via legal input.
 * @tc.number: Audio_Capturer_Create_018
 * @tc.desc  : Test function Create uses three types of parameters: AudioCapturerOptions, string and AppInfo.
 */
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Create_018, TestSize.Level0)
{
    AudioCapturerOptions capturerOptions;
    AppInfo appInfo = {};
    string cachePath = "/data/storage/el2/base/temp";
    capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_16000;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    capturerOptions.streamInfo.channels = AudioChannel::MONO;
    capturerOptions.capturerInfo.sourceType = SourceType::SOURCE_TYPE_VOICE_RECOGNITION;
    capturerOptions.capturerInfo.capturerFlags = CAPTURER_FLAG;
    appInfo.appTokenId = VALUE_THOUSAND;
    appInfo.appUid = static_cast<int32_t>(getuid());
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions, cachePath.c_str(), appInfo);
    ASSERT_NE(nullptr, audioCapturer);
    audioCapturer->Release();
}

/**
 * @tc.name  : Test Create API via legal input.
 * @tc.number: Audio_Capturer_Create_019
 * @tc.desc  : Test Create function with two types of parameters: AudioCapturerOptions and AppInfo，
 *             and give different parameters.
 */
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Create_019, TestSize.Level0)
{
    AudioCapturerOptions capturerOptions;
    AppInfo appInfo = {};
    capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_16000;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    capturerOptions.streamInfo.channels = AudioChannel::CHANNEL_3;
    capturerOptions.capturerInfo.sourceType = SourceType::SOURCE_TYPE_VOICE_RECOGNITION;
    capturerOptions.capturerInfo.capturerFlags = CAPTURER_FLAG;
    appInfo.appTokenId = VALUE_THOUSAND;
    appInfo.appUid = static_cast<int32_t>(getuid());
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions, appInfo);
    ASSERT_NE(nullptr, audioCapturer);
    audioCapturer->Release();
}

/**
 * @tc.name  : Test Create API via legal input.
 * @tc.number: Audio_Capturer_Create_020
 * @tc.desc  : Test Create function with two types of parameters: AudioCapturerOptions and AppInfo，
 *             and give different parameters.
 */
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Create_020, TestSize.Level0)
{
    AudioCapturerOptions capturerOptions;
    AppInfo appInfo = {};
    int32_t invalidSampleRate = -2;
    capturerOptions.streamInfo.samplingRate = static_cast<AudioSamplingRate>(invalidSampleRate);
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    capturerOptions.streamInfo.channels = AudioChannel::STEREO;
    capturerOptions.capturerInfo.sourceType = SourceType::SOURCE_TYPE_VOICE_RECOGNITION;
    capturerOptions.capturerInfo.capturerFlags = CAPTURER_FLAG;
    appInfo.appTokenId = VALUE_THOUSAND;
    appInfo.appUid = static_cast<int32_t>(getuid());
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions, appInfo);
    ASSERT_EQ(nullptr, audioCapturer);
}

/**
* @tc.name  : Test Create API via legal input.
* @tc.number: Audio_Capturer_Create_021
* @tc.desc  : Test Create interface with AudioCapturerOptions below.
*             Returns audioCapturer instance, if create is successful.
*             capturerOptions.streamInfo.samplingRate = SAMPLE_RATE_48000;
*             capturerOptions.streamInfo.encoding = ENCODING_PCM;
*             capturerOptions.streamInfo.format = SAMPLE_S32LE;
*             capturerOptions.streamInfo.channels = STEREO;
*             capturerOptions.capturerInfo.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
*             capturerOptions.capturerInfo.capturerFlags = CAPTURER_FLAG;
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Create_021, TestSize.Level0)
{
    AudioCapturerOptions capturerOptions;
    AudioCapturerUnitTest::InitializePlaybackCapturerOptions(capturerOptions);

    CaptureFilterOptions filterOptions;
    filterOptions.usages.emplace_back(StreamUsage::STREAM_USAGE_MEDIA);
    filterOptions.usages.emplace_back(StreamUsage::STREAM_USAGE_ALARM);
    capturerOptions.playbackCaptureConfig.filterOptions = filterOptions;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);
    audioCapturer->Release();
}

/**
* @tc.name  : Test Create API via legal input.
* @tc.number: Audio_Capturer_Create_022
* @tc.desc  : Test Create interface with AudioCapturerOptions below.
*             Returns audioCapturer instance, if create is successful.
*             capturerOptions.streamInfo.samplingRate = SAMPLE_RATE_48000;
*             capturerOptions.streamInfo.encoding = ENCODING_PCM;
*             capturerOptions.streamInfo.format = SAMPLE_S32LE;
*             capturerOptions.streamInfo.channels = STEREO;
*             capturerOptions.capturerInfo.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
*             capturerOptions.capturerInfo.capturerFlags = CAPTURER_FLAG;
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Create_022, TestSize.Level0)
{
    AudioCapturerOptions capturerOptions;
    capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S32LE;
    capturerOptions.streamInfo.channels = AudioChannel::STEREO;
    capturerOptions.capturerInfo.sourceType = SourceType::SOURCE_TYPE_PLAYBACK_CAPTURE;
    capturerOptions.capturerInfo.capturerFlags = CAPTURER_FLAG;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);
    audioCapturer->Release();
}

/**
* @tc.name  : Test Create API via legal input.
* @tc.number: Audio_Capturer_Create_023
* @tc.desc  : Test Create interface with parameter: silentCapture in AudioPlaybackCaptureConfig.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Create_023, TestSize.Level0)
{
    AudioCapturerOptions capturerOptions;
    AudioCapturerUnitTest::InitializePlaybackCapturerOptions(capturerOptions);
    capturerOptions.playbackCaptureConfig.silentCapture = true;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);
    audioCapturer->Release();
}

/**
 * @tc.name  : Test Create API via legal input.
 * @tc.number: Audio_Capturer_Create_024
 * @tc.desc  : Test Create function with two types of parameters: AudioCapturerOptions and AppInfo，
 *             and give different parameters.
 */
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Create_024, TestSize.Level0)
{
    AudioCapturerOptions capturerOptions;
    AppInfo appInfo = {};
    capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_16000;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    capturerOptions.streamInfo.channels = AudioChannel::MONO;
    capturerOptions.capturerInfo.sourceType = SourceType::SOURCE_TYPE_VOICE_MESSAGE;
    capturerOptions.capturerInfo.capturerFlags = CAPTURER_FLAG;
    appInfo.appTokenId = VALUE_THOUSAND;
    appInfo.appUid = static_cast<int32_t>(getuid());
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions, appInfo);
    ASSERT_NE(nullptr, audioCapturer);
    audioCapturer->Release();
}

/**
 * @tc.name  : Test Create API via legal input.
 * @tc.number: Audio_Capturer_Create_025
 * @tc.desc  : Test Create function maps sourceType to sceneType correctly.
 */
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Create_025, TestSize.Level0)
{
    AudioCapturerOptions capturerOptions;
    capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_16000;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    capturerOptions.streamInfo.channels = AudioChannel::MONO;
    capturerOptions.capturerInfo.capturerFlags = CAPTURER_FLAG;

    std::vector<SourceType> sourceTypeList = {
        SourceType::SOURCE_TYPE_MIC,
        SourceType::SOURCE_TYPE_VOICE_COMMUNICATION
    };
    for (int32_t i = 0; i < sourceTypeList.size(); i++) {
        capturerOptions.capturerInfo.sourceType = sourceTypeList[i];
        unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
        ASSERT_NE(nullptr, audioCapturer);
        audioCapturer->Release();
    }
}

/**
 * @tc.name  : Test Create API via legal input.
 * @tc.number: Audio_Capturer_Create_026
 * @tc.desc  : Test Create capture with invalid sourceType
 */
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Create_026, TestSize.Level0)
{
    AudioCapturerOptions capturerOptions;
    capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_16000;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    capturerOptions.streamInfo.channels = AudioChannel::MONO;
    capturerOptions.capturerInfo.capturerFlags = CAPTURER_FLAG;
    capturerOptions.capturerInfo.sourceType = SOURCE_TYPE_INVALID;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_EQ(nullptr, audioCapturer);
}

/**
* @tc.name  : Test SetParams API via legal input
* @tc.number: Audio_Capturer_SetParams_001
* @tc.desc  : Test SetParams interface. Returns 0 {SUCCESS}, if the setting is successful.
*             capturerParams.audioSampleFormat = SAMPLE_S16LE;
*             capturerParams.samplingRate = SAMPLE_RATE_44100;
*             capturerParams.audioChannel = STEREO;
*             capturerParams.audioEncoding = ENCODING_PCM;
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_SetParams_001, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    AudioCapturerParams capturerParams;
    capturerParams.audioSampleFormat = SAMPLE_S16LE;
    capturerParams.samplingRate = SAMPLE_RATE_44100;
    capturerParams.audioChannel = STEREO;
    capturerParams.audioEncoding = ENCODING_PCM;

    int32_t ret = audioCapturer->SetParams(capturerParams);
    EXPECT_EQ(SUCCESS, ret);
    audioCapturer->Release();
}

/**
* @tc.name  : Test SetParams API via legal input.
* @tc.number: Audio_Capturer_SetParams_002
* @tc.desc  : Test SetParams interface. Returns 0 {SUCCESS}, if the setting is successful.
*             capturerParams.audioSampleFormat = SAMPLE_S16LE;
*             capturerParams.samplingRate = SAMPLE_RATE_8000;
*             capturerParams.audioChannel = MONO;
*             capturerParams.audioEncoding = ENCODING_PCM;
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_SetParams_002, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    AudioCapturerParams capturerParams;
    capturerParams.audioSampleFormat = SAMPLE_S16LE;
    capturerParams.samplingRate = SAMPLE_RATE_8000;
    capturerParams.audioChannel = MONO;
    capturerParams.audioEncoding = ENCODING_PCM;

    int32_t ret = audioCapturer->SetParams(capturerParams);
    EXPECT_EQ(SUCCESS, ret);
    audioCapturer->Release();
}

/**
* @tc.name  : Test SetParams API via legal input.
* @tc.number: Audio_Capturer_SetParams_003
* @tc.desc  : Test SetParams interface. Returns 0 {SUCCESS}, if the setting is successful.
*             capturerParams.audioSampleFormat = SAMPLE_S16LE;
*             capturerParams.samplingRate = SAMPLE_RATE_11025;
*             capturerParams.audioChannel = STEREO;
*             capturerParams.audioEncoding = ENCODING_PCM;
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_SetParams_003, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    AudioCapturerParams capturerParams;
    capturerParams.audioSampleFormat = SAMPLE_S16LE;
    capturerParams.samplingRate = SAMPLE_RATE_11025;
    capturerParams.audioChannel = STEREO;
    capturerParams.audioEncoding = ENCODING_PCM;

    int32_t ret = audioCapturer->SetParams(capturerParams);
    EXPECT_EQ(SUCCESS, ret);
    audioCapturer->Release();
}

/**
* @tc.name  : Test SetParams API via legal input.
* @tc.number: Audio_Capturer_SetParams_004
* @tc.desc  : Test SetParams interface. Returns 0 {SUCCESS}, if the setting is successful.
*             capturerParams.audioSampleFormat = SAMPLE_S16LE;
*             capturerParams.samplingRate = SAMPLE_RATE_22050;
*             capturerParams.audioChannel = MONO;
*             capturerParams.audioEncoding = ENCODING_PCM;
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_SetParams_004, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    AudioCapturerParams capturerParams;
    capturerParams.audioSampleFormat = SAMPLE_S16LE;
    capturerParams.samplingRate = SAMPLE_RATE_22050;
    capturerParams.audioChannel = MONO;
    capturerParams.audioEncoding = ENCODING_PCM;

    int32_t ret = audioCapturer->SetParams(capturerParams);
    EXPECT_EQ(SUCCESS, ret);
    audioCapturer->Release();
}

/**
* @tc.name  : Test SetParams API via legal input.
* @tc.number: Audio_Capturer_SetParams_005
* @tc.desc  : Test SetParams interface. Returns 0 {SUCCESS}, if the setting is successful.
*             capturerParams.audioSampleFormat = SAMPLE_S16LE;
*             capturerParams.samplingRate = SAMPLE_RATE_96000;
*             capturerParams.audioChannel = MONO;
*             capturerParams.audioEncoding = ENCODING_PCM;
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_SetParams_005, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    AudioCapturerParams capturerParams;
    capturerParams.audioSampleFormat = SAMPLE_S16LE;
    capturerParams.samplingRate = SAMPLE_RATE_96000;
    capturerParams.audioChannel = MONO;
    capturerParams.audioEncoding = ENCODING_PCM;

    int32_t ret = audioCapturer->SetParams(capturerParams);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test SetParams API via legal input.
* @tc.number: Audio_Capturer_SetParams_006
* @tc.desc  : Test SetParams interface. Returns 0 {SUCCESS}, if the setting is successful.
*             capturerParams.audioSampleFormat = SAMPLE_S24LE;
*             capturerParams.samplingRate = SAMPLE_RATE_64000;
*             capturerParams.audioChannel = MONO;
*             capturerParams.audioEncoding = ENCODING_PCM;
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_SetParams_006, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    AudioCapturerParams capturerParams;
    capturerParams.audioSampleFormat = SAMPLE_S24LE;
    capturerParams.samplingRate = SAMPLE_RATE_64000;
    capturerParams.audioChannel = MONO;
    capturerParams.audioEncoding = ENCODING_PCM;

    int32_t ret = audioCapturer->SetParams(capturerParams);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test SetParams API via illegal input.
* @tc.number: Audio_Capturer_SetParams_007
* @tc.desc  : Test SetParams interface. Returns 0 {SUCCESS}, if the setting is successful.
*             capturerParams.audioSampleFormat = SAMPLE_S16LE;
*             capturerParams.samplingRate = SAMPLE_RATE_16000;
*             capturerParams.audioChannel = STEREO;
*             capturerParams.audioEncoding = ENCODING_PCM;
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_SetParams_007, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    AudioCapturerParams capturerParams;
    capturerParams.audioSampleFormat = SAMPLE_S16LE;
    capturerParams.samplingRate = SAMPLE_RATE_16000;
    capturerParams.audioChannel = STEREO;
    capturerParams.audioEncoding = ENCODING_PCM;

    int32_t ret = audioCapturer->SetParams(capturerParams);
    EXPECT_EQ(SUCCESS, ret);
    audioCapturer->Release();
}

/**
* @tc.name  : Test SetParams API stability.
* @tc.number: Audio_Capturer_SetParams_Stability_001
* @tc.desc  : Test SetParams interface stability.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_SetParams_Stability_001, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    AudioCapturerParams capturerParams;
    capturerParams.audioSampleFormat = SAMPLE_S16LE;
    capturerParams.samplingRate = SAMPLE_RATE_16000;
    capturerParams.audioChannel = STEREO;
    capturerParams.audioEncoding = ENCODING_PCM;

    for (int i = 0; i < VALUE_HUNDRED; i++) {
        ret = audioCapturer->SetParams(capturerParams);
        EXPECT_EQ(SUCCESS, ret);

        AudioCapturerParams getCapturerParams;
        ret = audioCapturer->GetParams(getCapturerParams);
        EXPECT_EQ(SUCCESS, ret);
    }

    audioCapturer->Release();
}

/**
* @tc.name  : Test GetParams API via legal input.
* @tc.number: Audio_Capturer_GetParams_001
* @tc.desc  : Test GetParams interface. Returns 0 {SUCCESS}, if the getting is successful.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_GetParams_001, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    AudioCapturerParams capturerParams;
    capturerParams.audioSampleFormat = SAMPLE_S16LE;
    capturerParams.samplingRate = SAMPLE_RATE_44100;
    capturerParams.audioChannel = STEREO;
    capturerParams.audioEncoding = ENCODING_PCM;
    ret = audioCapturer->SetParams(capturerParams);
    EXPECT_EQ(SUCCESS, ret);

    AudioCapturerParams getCapturerParams;
    ret = audioCapturer->GetParams(getCapturerParams);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(capturerParams.audioSampleFormat, getCapturerParams.audioSampleFormat);
    EXPECT_EQ(capturerParams.samplingRate, getCapturerParams.samplingRate);
    EXPECT_EQ(capturerParams.audioChannel, getCapturerParams.audioChannel);
    EXPECT_EQ(capturerParams.audioEncoding, getCapturerParams.audioEncoding);

    audioCapturer->Release();
}

/**
* @tc.name  : Test GetParams API via legal state, CAPTURER_RUNNING: GetParams after Start.
* @tc.number: Audio_Capturer_GetParams_002
* @tc.desc  : Test GetParams interface. Returns 0 {SUCCESS} if the getting is successful.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_GetParams_002, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    AudioCapturerParams capturerParams;
    capturerParams.audioSampleFormat = SAMPLE_S16LE;
    capturerParams.samplingRate = SAMPLE_RATE_44100;
    capturerParams.audioChannel = MONO;
    capturerParams.audioEncoding = ENCODING_PCM;
    ret = audioCapturer->SetParams(capturerParams);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    AudioCapturerParams getCapturerParams;
    ret = audioCapturer->GetParams(getCapturerParams);
    EXPECT_EQ(SUCCESS, ret);

    audioCapturer->Release();
}

/**
* @tc.name  : Test GetParams API via illegal state, CAPTURER_NEW: Call GetParams without SetParams.
* @tc.number: Audio_Capturer_GetParams_003
* @tc.desc  : Test GetParams interface. Returns error code, if the capturer state is CAPTURER_NEW.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_GetParams_003, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    AudioCapturerParams capturerParams;
    capturerParams.audioSampleFormat = SAMPLE_S16LE;
    capturerParams.samplingRate = SAMPLE_RATE_44100;
    capturerParams.audioChannel = MONO;
    capturerParams.audioEncoding = ENCODING_PCM;

    AudioCapturerParams getCapturerParams;
    ret = audioCapturer->GetParams(getCapturerParams);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);
}

/**
* @tc.name  : Test GetParams API via illegal state, CAPTURER_RELEASED: Call GetParams after Release.
* @tc.number: Audio_Capturer_GetParams_004
* @tc.desc  : Test GetParams interface. Returns error code, if the capturer state is CAPTURER_RELEASED.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_GetParams_004, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    ret = AudioCapturerUnitTest::InitializeCapturer(audioCapturer);
    EXPECT_EQ(SUCCESS, ret);

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);

    AudioCapturerParams getCapturerParams;
    ret = audioCapturer->GetParams(getCapturerParams);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);
}

/**
* @tc.name  : Test GetParams API via legal state, CAPTURER_STOPPED: GetParams after Stop.
* @tc.number: Audio_Capturer_GetParams_005
* @tc.desc  : Test GetParams interface. Returns 0 {SUCCESS}, if the getting is successful.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_GetParams_005, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    ret = AudioCapturerUnitTest::InitializeCapturer(audioCapturer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioCapturer->Stop();
    EXPECT_EQ(true, isStopped);

    AudioCapturerParams getCapturerParams;
    ret = audioCapturer->GetParams(getCapturerParams);
    EXPECT_EQ(SUCCESS, ret);

    audioCapturer->Release();
}

/**
* @tc.name  : Test GetParams API stability.
* @tc.number: Audio_Capturer_GetParams_Stability_001
* @tc.desc  : Test GetParams interface stability.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_GetParams_Stability_001, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    AudioCapturerParams capturerParams;
    capturerParams.audioSampleFormat = SAMPLE_S16LE;
    capturerParams.samplingRate = SAMPLE_RATE_44100;
    capturerParams.audioChannel = MONO;
    capturerParams.audioEncoding = ENCODING_PCM;

    ret = audioCapturer->SetParams(capturerParams);
    EXPECT_EQ(SUCCESS, ret);

    for (int i = 0; i < VALUE_THOUSAND; i++) {
        AudioCapturerParams getCapturerParams;
        ret = audioCapturer->GetParams(getCapturerParams);
        EXPECT_EQ(SUCCESS, ret);
    }

    audioCapturer->Release();
}

/**
 * @tc.name  : Test GetParams API stability.
 * @tc.number: Audio_Capturer_GetParams_Stability_001
 * @tc.desc  : Test GetParams interface stability.
 */
HWTEST(AudioCapturerUnitTest, Audio_Capturer_GetAudioStreamId_001, TestSize.Level1)
{
    int32_t ret = -1;
    AudioCapturerOptions capturerOptions;

    AudioCapturerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    uint32_t sessionID;
    ret = audioCapturer->GetAudioStreamId(sessionID);
    EXPECT_EQ(SUCCESS, ret);

    audioCapturer->Release();
}

/**
* @tc.name  : Test GetBufferSize API via legal input.
* @tc.number: Audio_Capturer_GetBufferSize_001
* @tc.desc  : Test GetBufferSize interface. Returns 0 {SUCCESS}, if the getting is successful.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_GetBufferSize_001, TestSize.Level1)
{
    int32_t ret = -1;
    AudioCapturerOptions capturerOptions;

    AudioCapturerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    size_t bufferLen;
    ret = audioCapturer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    audioCapturer->Release();
}

/**
* @tc.name  : Test GetBufferSize API via illegal state, CAPTURER_NEW: without initializing the capturer.
* @tc.number: Audio_Capturer_GetBufferSize_002
* @tc.desc  : Test GetBufferSize interface. Returns SUCCESS, if the capturer state is CAPTURER_NEW.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_GetBufferSize_002, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    size_t bufferLen;
    ret = audioCapturer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test GetBufferSize API via illegal state, CAPTURER_RELEASED: call Release before GetBufferSize
* @tc.number: Audio_Capturer_GetBufferSize_003
* @tc.desc  : Test GetBufferSize interface. Returns error code, if the capturer state is CAPTURER_RELEASED.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_GetBufferSize_003, TestSize.Level1)
{
    int32_t ret = -1;
    AudioCapturerOptions capturerOptions;

    AudioCapturerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);

    size_t bufferLen;
    ret = audioCapturer->GetBufferSize(bufferLen);
    EXPECT_EQ(ERR_ILLEGAL_STATE, ret);
}

/**
* @tc.name  : Test GetBufferSize API via legal state, CAPTURER_STOPPED: call Stop before GetBufferSize
* @tc.number: Audio_Capturer_GetBufferSize_004
* @tc.desc  : Test GetBufferSize interface. Returns 0 {SUCCESS}, if the getting is successful.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_GetBufferSize_004, TestSize.Level1)
{
    int32_t ret = -1;
    AudioCapturerOptions capturerOptions;

    AudioCapturerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioCapturer->Stop();
    EXPECT_EQ(true, isStopped);

    size_t bufferLen;
    ret = audioCapturer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    audioCapturer->Release();
}

/**
* @tc.name  : Test GetBufferSize API via legal state, CAPTURER_RUNNING: call Start before GetBufferSize
* @tc.number: Audio_Capturer_GetBufferSize_005
* @tc.desc  : test GetBufferSize interface. Returns 0 {SUCCESS}, if the getting is successful.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_GetBufferSize_005, TestSize.Level1)
{
    int32_t ret = -1;
    AudioCapturerOptions capturerOptions;

    AudioCapturerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    size_t bufferLen;
    ret = audioCapturer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    audioCapturer->Release();
}

/**
* @tc.name  : Test GetFrameCount API via legal input.
* @tc.number: Audio_Capturer_GetFrameCount_001
* @tc.desc  : test GetFrameCount interface, Returns 0 {SUCCESS}, if the getting is successful.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_GetFrameCount_001, TestSize.Level1)
{
    int32_t ret = -1;
    AudioCapturerOptions capturerOptions;

    AudioCapturerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    uint32_t frameCount;
    ret = audioCapturer->GetFrameCount(frameCount);
    EXPECT_EQ(SUCCESS, ret);

    audioCapturer->Release();
}

/**
* @tc.name  : Test GetFrameCount API via illegal state, CAPTURER_NEW: without initialiing the capturer.
* @tc.number: Audio_Capturer_GetFrameCount_002
* @tc.desc  : Test GetFrameCount interface. Returns SUCCESS, if the capturer state is CAPTURER_NEW.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_GetFrameCount_002, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    uint32_t frameCount;
    ret = audioCapturer->GetFrameCount(frameCount);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test GetFrameCount API via legal state, CAPTURER_RUNNING: call Start before GetFrameCount.
* @tc.number: Audio_Capturer_GetFrameCount_003
* @tc.desc  : Test GetFrameCount interface. Returns 0 {SUCCESS}, if the getting is successful.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_GetFrameCount_003, TestSize.Level1)
{
    int32_t ret = -1;
    AudioCapturerOptions capturerOptions;

    AudioCapturerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    uint32_t frameCount;
    ret = audioCapturer->GetFrameCount(frameCount);
    EXPECT_EQ(SUCCESS, ret);

    audioCapturer->Release();
}

/**
* @tc.name  : Test GetFrameCount API via legal state, CAPTURER_STOPPED: call Stop before GetFrameCount
* @tc.number: Audio_Capturer_GetFrameCount_004
* @tc.desc  : Test GetFrameCount interface. Returns 0 {SUCCESS}, if the getting is successful.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_GetFrameCount_004, TestSize.Level1)
{
    int32_t ret = -1;
    AudioCapturerOptions capturerOptions;

    AudioCapturerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioCapturer->Stop();
    EXPECT_EQ(true, isStopped);

    uint32_t frameCount;
    ret = audioCapturer->GetFrameCount(frameCount);
    EXPECT_EQ(SUCCESS, ret);

    audioCapturer->Release();
}

/**
* @tc.name  : Test GetFrameCount API via illegal state, CAPTURER_RELEASED: call Release before GetFrameCount
* @tc.number: Audio_Capturer_GetFrameCount_005
* @tc.desc  : Test GetFrameCount interface.  Returns error code, if the state is CAPTURER_RELEASED.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_GetFrameCount_005, TestSize.Level1)
{
    int32_t ret = -1;
    AudioCapturerOptions capturerOptions;

    AudioCapturerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);

    uint32_t frameCount;
    ret = audioCapturer->GetFrameCount(frameCount);
    EXPECT_EQ(ERR_ILLEGAL_STATE, ret);
}

/**
* @tc.name  : Test Start API via legal state, CAPTURER_PREPARED.
* @tc.number: Audio_Capturer_Start_001
* @tc.desc  : Test Start interface. Returns true if start is successful.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Start_001, TestSize.Level1)
{
    AudioCapturerOptions capturerOptions;

    AudioCapturerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    audioCapturer->Release();
}

/**
* @tc.name  : Test Start API via illegal state, CAPTURER_NEW: without initializing the capturer.
* @tc.number: Audio_Capturer_Start_002
* @tc.desc  : Test Start interface. Returns false, if the capturer state is CAPTURER_NEW.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Start_002, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(false, isStarted);
}

/**
* @tc.name  : Test Start API via illegal state, CAPTURER_RELEASED: call Start after Release
* @tc.number: Audio_Capturer_Start_003
* @tc.desc  : Test Start interface. Returns false, if the capturer state is CAPTURER_RELEASED.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Start_003, TestSize.Level1)
{
    AudioCapturerOptions capturerOptions;

    AudioCapturerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(false, isStarted);

    audioCapturer->Release();
}

/**
* @tc.name  : Test Start API via legal state, CAPTURER_STOPPED: Start Stop and then Start again
* @tc.number: Audio_Capturer_Start_004
* @tc.desc  : Test Start interface. Returns true, if the start is successful.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Start_004, TestSize.Level1)
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

    audioCapturer->Release();
}

/**
* @tc.name  : Test Start API via illegal state, CAPTURER_RUNNING : call Start repeatedly
* @tc.number: Audio_Capturer_Start_005
* @tc.desc  : Test Start interface. Returns false, if the capturer state is CAPTURER_RUNNING.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Start_005, TestSize.Level1)
{
    AudioCapturerOptions capturerOptions;

    AudioCapturerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    isStarted = audioCapturer->Start();
    EXPECT_EQ(false, isStarted);

    audioCapturer->Release();
}

/**
* @tc.name  : Test Start API via legal state, CAPTURER_PREPARED.
* @tc.number: Audio_Capturer_Start_006
* @tc.desc  : Test Start interface. Returns true if start is successful.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Start_006, TestSize.Level1)
{
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

    audioCapturer->Release();
}

/**
* @tc.name  : Test Read API via isBlockingRead = true.
* @tc.number: Audio_Capturer_Read_001
* @tc.desc  : Test Read interface. Returns number of bytes read, if the read is successful.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Read_001, TestSize.Level1)
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
    audioCapturer->Release();

    free(buffer);
    fclose(capFile);
}

/**
* @tc.name  : Test Read API via isBlockingRead = false.
* @tc.number: Audio_Capturer_Read_002
* @tc.desc  : Test Read interface. Returns number of bytes read, if the read is successful.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Read_002, TestSize.Level1)
{
    int32_t ret = -1;
    bool isBlockingRead = false;
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
    FILE *capFile = fopen(AUDIO_CAPTURE_FILE2.c_str(), "wb");
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
    audioCapturer->Release();

    free(buffer);
    fclose(capFile);
}

/**
* @tc.name  : Test Read API via illegl state, CAPTURER_NEW : without Initializing the capturer.
* @tc.number: Audio_Capturer_Read_003
* @tc.desc  : Test Read interface. Returns SUCCESS, if the capturer state is CAPTURER_NEW.
*           : bufferLen is 0 here, so audioCapturer->Read returns ERR_INVALID_PARAM.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Read_003, TestSize.Level1)
{
    int32_t ret = -1;
    bool isBlockingRead = true;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(false, isStarted);

    size_t bufferLen;
    ret = audioCapturer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);
    ASSERT_NE(nullptr, buffer);

    int32_t bytesRead = audioCapturer->Read(*buffer, bufferLen, isBlockingRead);
    EXPECT_EQ(ERR_INVALID_PARAM, bytesRead);

    audioCapturer->Flush();
    audioCapturer->Stop();
    audioCapturer->Release();

    free(buffer);
}

/**
* @tc.name  : Test Read API via illegl state, CAPTURER_PREPARED : Read without Start.
* @tc.number: Audio_Capturer_Read_004
* @tc.desc  : Test Read interface. Returns error code, if the capturer state is not CAPTURER_RUNNING.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Read_004, TestSize.Level1)
{
    int32_t ret = -1;
    bool isBlockingRead = true;
    AudioCapturerOptions capturerOptions;

    AudioCapturerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    size_t bufferLen;
    ret = audioCapturer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);
    ASSERT_NE(nullptr, buffer);

    int32_t bytesRead = audioCapturer->Read(*buffer, bufferLen, isBlockingRead);
    EXPECT_EQ(ERR_ILLEGAL_STATE, bytesRead);

    audioCapturer->Flush();
    audioCapturer->Stop();
    audioCapturer->Release();

    free(buffer);
}

/**
* @tc.name  : Test Read API via illegal input, bufferLength = 0.
* @tc.number: Audio_Capturer_Read_005
* @tc.desc  : Test Read interface. Returns error code, if the bufferLength <= 0.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Read_005, TestSize.Level1)
{
    bool isBlockingRead = true;
    AudioCapturerOptions capturerOptions;

    AudioCapturerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    size_t bufferLen = 0;

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);
    ASSERT_NE(nullptr, buffer);

    int32_t bytesRead = audioCapturer->Read(*buffer, bufferLen, isBlockingRead);
    EXPECT_EQ(ERR_INVALID_PARAM, bytesRead);

    audioCapturer->Flush();
    audioCapturer->Stop();
    audioCapturer->Release();

    free(buffer);
}

/**
* @tc.name  : Test Read API via illegal state, CAPTURER_STOPPED: Read after Stop.
* @tc.number: Audio_Capturer_Read_006
* @tc.desc  : Test Read interface. Returns error code, if the capturer state is not CAPTURER_RUNNING.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Read_006, TestSize.Level1)
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

    bool isStopped = audioCapturer->Stop();
    EXPECT_EQ(true, isStopped);

    int32_t bytesRead = audioCapturer->Read(*buffer, bufferLen, isBlockingRead);
    EXPECT_EQ(ERR_ILLEGAL_STATE, bytesRead);

    audioCapturer->Release();

    free(buffer);
}

/**
* @tc.name  : Test Read API via illegal state, CAPTURER_RELEASED: Read after Release.
* @tc.number: Audio_Capturer_Read_007
* @tc.desc  : Test Read interface. Returns error code, if the capturer state is not CAPTURER_RUNNING.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Read_007, TestSize.Level1)
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

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);

    int32_t bytesRead = audioCapturer->Read(*buffer, bufferLen, isBlockingRead);
    EXPECT_EQ(ERR_ILLEGAL_STATE, bytesRead);

    free(buffer);
}

/**
* @tc.name  : Test Read API via isBlockingRead = true.
* @tc.number: Audio_Capturer_Read_008
* @tc.desc  : Test Read interface. Returns number of bytes read, if the read is successful.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_Read_008, TestSize.Level1)
{
    int32_t ret = -1;
    bool isBlockingRead = true;
    AudioCapturerOptions capturerOptions;
    AudioCapturerUnitTest::InitializePlaybackCapturerOptions(capturerOptions);
    capturerOptions.playbackCaptureConfig.filterOptions.usages.push_back(StreamUsage::STREAM_USAGE_MEDIA);

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    size_t bufferLen;
    ret = audioCapturer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);
    ASSERT_NE(nullptr, buffer);
    FILE *capFile = fopen(AUDIO_PLAYBACK_CAPTURER_TEST_FILE.c_str(), "wb");
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
    audioCapturer->Release();

    free(buffer);
    fclose(capFile);
}

/**
* @tc.name  : Test GetAudioTime API via legal input.
* @tc.number: Audio_Capturer_GetAudioTime_001
* @tc.desc  : Test GetAudioTime interface. Returns true, if the getting is successful.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_GetAudioTime_001, TestSize.Level1)
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

    Timestamp timestamp;
    bool getAudioTime = audioCapturer->GetAudioTime(timestamp, Timestamp::Timestampbase::MONOTONIC);
    EXPECT_EQ(true, getAudioTime);
    EXPECT_GE(timestamp.time.tv_sec, (const long)VALUE_ZERO);
    EXPECT_GE(timestamp.time.tv_nsec, (const long)VALUE_ZERO);

    audioCapturer->Flush();
    audioCapturer->Stop();
    audioCapturer->Release();

    free(buffer);
}

/**
* @tc.name  : Test GetAudioTime API via illegal state, CAPTURER_NEW: GetAudioTime without initializing the capturer.
* @tc.number: Audio_Capturer_GetAudioTime_002
* @tc.desc  : Test GetAudioTime interface. Returns false, if the capturer state is CAPTURER_NEW.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_GetAudioTime_002, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    Timestamp timestamp;
    bool getAudioTime = audioCapturer->GetAudioTime(timestamp, Timestamp::Timestampbase::MONOTONIC);
    EXPECT_EQ(false, getAudioTime);
}

/**
* @tc.name  : Test GetAudioTime API via legal state, CAPTURER_RUNNING.
* @tc.number: Audio_Capturer_GetAudioTime_003
* @tc.desc  : test GetAudioTime interface. Returns true, if the getting is successful.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_GetAudioTime_003, TestSize.Level1)
{
    AudioCapturerOptions capturerOptions;

    AudioCapturerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    Timestamp timestamp;
    bool getAudioTime = audioCapturer->GetAudioTime(timestamp, Timestamp::Timestampbase::MONOTONIC);
    EXPECT_EQ(true, getAudioTime);

    audioCapturer->Release();
}

/**
* @tc.name  : Test GetAudioTime API via legal state, CAPTURER_STOPPED.
* @tc.number: Audio_Capturer_GetAudioTime_004
* @tc.desc  : Test GetAudioTime interface.  Returns true, if the getting is successful.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_GetAudioTime_004, TestSize.Level1)
{
    AudioCapturerOptions capturerOptions;

    AudioCapturerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioCapturer->Stop();
    EXPECT_EQ(true, isStopped);

    Timestamp timestamp;
    bool getAudioTime = audioCapturer->GetAudioTime(timestamp, Timestamp::Timestampbase::MONOTONIC);
    EXPECT_EQ(false, getAudioTime);

    audioCapturer->Release();
}

/**
* @tc.name  : Test GetAudioTime API via illegal state, CAPTURER_RELEASED: GetAudioTime after Release.
* @tc.number: Audio_Capturer_GetAudioTime_005
* @tc.desc  : Test GetAudioTime interface. Returns false, if the capturer state is CAPTURER_RELEASED
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_GetAudioTime_005, TestSize.Level1)
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

    Timestamp timestamp;
    bool getAudioTime = audioCapturer->GetAudioTime(timestamp, Timestamp::Timestampbase::MONOTONIC);
    EXPECT_EQ(false, getAudioTime);
}

/**
* @tc.name  : Test GetAudioTime API stability.
* @tc.number: Audio_Capturer_GetAudioTime_Stability_001
* @tc.desc  : Test GetAudioTime interface stability.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_GetAudioTime_Stability_001, TestSize.Level1)
{
    AudioCapturerOptions capturerOptions;

    AudioCapturerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    thread captureThread(StartCaptureThread, audioCapturer.get(), AUDIO_TIME_STABILITY_TEST_FILE);

    for (int i = 0; i < VALUE_THOUSAND; i++) {
        Timestamp timestamp;
        bool getAudioTime = audioCapturer->GetAudioTime(timestamp, Timestamp::Timestampbase::MONOTONIC);
        EXPECT_EQ(true, getAudioTime);
    }

    captureThread.join();

    bool isStopped = audioCapturer->Stop();
    EXPECT_EQ(true, isStopped);

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
* @tc.name  : Test IsDeviceChanged API via different device type.
* @tc.number: Audio_Capturer_IsDeviceChanged_001
* @tc.desc  : Test IsDeviceChanged API via device type DEVICE_TYPE_INVALID, return true because different device types.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_IsDeviceChanged_001, TestSize.Level1)
{
    AppInfo appInfo = {};
    unique_ptr<AudioCapturerPrivate> audioCapturer =
        std::make_unique<AudioCapturerPrivate>(STREAM_MUSIC, appInfo, true);
    EXPECT_NE(nullptr, audioCapturer);

    AudioPlaybackCaptureConfig playbackCaptureConfig;
    audioCapturer->capturerInfo_.sourceType = SOURCE_TYPE_MIC;
    audioCapturer->capturerInfo_.capturerFlags = 0;
    audioCapturer->capturerInfo_.originalFlag = 0;
    audioCapturer->filterConfig_ = playbackCaptureConfig;
    AudioCapturerParams capturerParams;
    capturerParams.audioSampleFormat = SAMPLE_S16LE;
    capturerParams.samplingRate = SAMPLE_RATE_44100;
    capturerParams.audioChannel = STEREO;
    capturerParams.audioEncoding = ENCODING_PCM;

    audioCapturer->SetParams(capturerParams);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    audioCapturer->currentDeviceInfo_.deviceType = DEVICE_TYPE_INVALID;

    DeviceInfo newDeviceInfo = {};
    bool isChanged = audioCapturer->IsDeviceChanged(newDeviceInfo);
    EXPECT_EQ(isChanged, true);

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
* @tc.name  : Test IsDeviceChanged API via same device type.
* @tc.number: Audio_Capturer_IsDeviceChanged_002
* @tc.desc  : Test IsDeviceChanged API via same device, return false.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_IsDeviceChanged_002, TestSize.Level1)
{
    AppInfo appInfo = {};
    unique_ptr<AudioCapturerPrivate> audioCapturer =
        std::make_unique<AudioCapturerPrivate>(STREAM_MUSIC, appInfo, true);
    EXPECT_NE(nullptr, audioCapturer);

    DeviceInfo newDeviceInfo = {};
    bool isChanged = audioCapturer->IsDeviceChanged(newDeviceInfo);
    EXPECT_EQ(isChanged, false);

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
* @tc.name  : Test IsDeviceChanged API via lots of calls.
* @tc.number: Audio_Capturer_IsDeviceChanged_003
* @tc.desc  : Test IsDeviceChanged API 200 times.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_IsDeviceChanged_003, TestSize.Level1)
{
    AppInfo appInfo = {};
    unique_ptr<AudioCapturerPrivate> audioCapturer =
        std::make_unique<AudioCapturerPrivate>(STREAM_MUSIC, appInfo, true);
    EXPECT_NE(nullptr, audioCapturer);

    DeviceInfo newDeviceInfo = {};
    for (int32_t i = 0; i < STRESS_TEST_COUNTS; i++) {
        bool isChanged = audioCapturer->IsDeviceChanged(newDeviceInfo);
        EXPECT_EQ(isChanged, false);
    }

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);
}
} // namespace AudioStandard
} // namespace OHOS
