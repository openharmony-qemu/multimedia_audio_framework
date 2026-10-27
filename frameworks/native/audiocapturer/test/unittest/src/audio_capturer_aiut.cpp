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

#include "audio_capturer_aiut.h"
#include "aiut_log.h"
#include "audio_capturer_private.h"
#include "audio_errors.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {

namespace {
constexpr uint32_t TEST_SAMPLE_RATE_44100 = 44100;
constexpr int32_t TEST_CAPTURER_FLAGS_NORMAL = 0;
constexpr int32_t TEST_APP_UID = 1234;
constexpr int32_t TEST_APP_PID = 5678;
}

// ==================== AudioCapturerStaticAiut ====================

void AudioCapturerStaticAiut::SetUpTestCase(void) {}
void AudioCapturerStaticAiut::TearDownTestCase(void) {}
void AudioCapturerStaticAiut::SetUp(void) {}
void AudioCapturerStaticAiut::TearDown(void) {}

// ==================== ConvertToAudioStreamParams Tests ====================

/**
 * @tc.name  : Test ConvertToAudioStreamParams with valid S16LE params
 * @tc.number: CapturerConvertToAudioStreamParams_001
 * @tc.desc  : Test static inline ConvertToAudioStreamParams converts AudioCapturerParams to
 *             AudioStreamParams correctly for S16LE/44100/Stereo/PCM.
 */
HWTEST_F(AudioCapturerStaticAiut, CapturerConvertToAudioStreamParams_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    AudioCapturerParams capturerParams;
    capturerParams.audioSampleFormat = SAMPLE_S16LE;
    capturerParams.samplingRate = SAMPLE_RATE_44100;
    capturerParams.audioChannel = STEREO;
    capturerParams.audioEncoding = ENCODING_PCM;
    capturerParams.channelLayout = CH_LAYOUT_STEREO;

    AudioStreamParams streamParams = AudioCapturerPrivate::ConvertToAudioStreamParams(capturerParams);

    AIUT_EXPECT_EQ_INFO_LOG(streamParams.format, SAMPLE_S16LE, "format mismatch");
    AIUT_EXPECT_EQ_INFO_LOG(streamParams.samplingRate, SAMPLE_RATE_44100, "samplingRate mismatch");
    AIUT_EXPECT_EQ_INFO_LOG(streamParams.channels, STEREO, "channels mismatch");
    AIUT_EXPECT_EQ_INFO_LOG(streamParams.encoding, ENCODING_PCM, "encoding mismatch");
}

/**
 * @tc.name  : Test ConvertToAudioStreamParams with different format
 * @tc.number: CapturerConvertToAudioStreamParams_002
 * @tc.desc  : Test ConvertToAudioStreamParams converts SAMPLE_U8/MONO correctly.
 */
HWTEST_F(AudioCapturerStaticAiut, CapturerConvertToAudioStreamParams_002, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    AudioCapturerParams capturerParams;
    capturerParams.audioSampleFormat = SAMPLE_U8;
    capturerParams.samplingRate = SAMPLE_RATE_8000;
    capturerParams.audioChannel = MONO;
    capturerParams.audioEncoding = ENCODING_PCM;

    AudioStreamParams streamParams = AudioCapturerPrivate::ConvertToAudioStreamParams(capturerParams);

    AIUT_EXPECT_EQ_INFO_LOG(streamParams.format, SAMPLE_U8, "format mismatch");
    AIUT_EXPECT_EQ_INFO_LOG(streamParams.samplingRate, SAMPLE_RATE_8000, "samplingRate mismatch");
    AIUT_EXPECT_EQ_INFO_LOG(streamParams.channels, MONO, "channels mismatch");
    AIUT_EXPECT_EQ_INFO_LOG(streamParams.encoding, ENCODING_PCM, "encoding mismatch");
}

/**
 * @tc.name  : Test ConvertToAudioStreamParams preserves channelLayout
 * @tc.number: CapturerConvertToAudioStreamParams_003
 * @tc.desc  : Test ConvertToAudioStreamParams preserves channelLayout field.
 */
HWTEST_F(AudioCapturerStaticAiut, CapturerConvertToAudioStreamParams_003, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    AudioCapturerParams capturerParams;
    capturerParams.audioSampleFormat = SAMPLE_S32LE;
    capturerParams.samplingRate = SAMPLE_RATE_48000;
    capturerParams.audioChannel = STEREO;
    capturerParams.audioEncoding = ENCODING_PCM;
    capturerParams.channelLayout = CH_LAYOUT_STEREO;

    AudioStreamParams streamParams = AudioCapturerPrivate::ConvertToAudioStreamParams(capturerParams);

    AIUT_EXPECT_EQ_INFO_LOG(streamParams.channelLayout, CH_LAYOUT_STEREO, "channelLayout mismatch");
}

/**
 * @tc.name  : Test ConvertToAudioStreamParams with EC fields
 * @tc.number: CapturerConvertToAudioStreamParams_004
 * @tc.desc  : Test ConvertToAudioStreamParams populates EC (echo cancellation) stream params.
 */
HWTEST_F(AudioCapturerStaticAiut, CapturerConvertToAudioStreamParams_004, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    AudioCapturerParams capturerParams;
    capturerParams.audioSampleFormat = SAMPLE_S16LE;
    capturerParams.samplingRate = SAMPLE_RATE_48000;
    capturerParams.audioChannel = STEREO;
    capturerParams.audioEncoding = ENCODING_PCM;
    capturerParams.audioEcSampleFormat = SAMPLE_S16LE;
    capturerParams.ecSamplingRate = SAMPLE_RATE_16000;
    capturerParams.audioEcChannel = MONO;
    capturerParams.audioEcEncoding = ENCODING_PCM;
    capturerParams.ecChannelLayout = CH_LAYOUT_MONO;

    AudioStreamParams streamParams = AudioCapturerPrivate::ConvertToAudioStreamParams(capturerParams);

    AIUT_EXPECT_EQ_INFO_LOG(streamParams.ecFormat, SAMPLE_S16LE, "ecFormat mismatch");
    AIUT_EXPECT_EQ_INFO_LOG(streamParams.ecSamplingRate, SAMPLE_RATE_16000, "ecSamplingRate mismatch");
    AIUT_EXPECT_EQ_INFO_LOG(streamParams.ecChannels, MONO, "ecChannels mismatch");
    AIUT_EXPECT_EQ_INFO_LOG(streamParams.ecEncoding, ENCODING_PCM, "ecEncoding mismatch");
    AIUT_EXPECT_EQ_INFO_LOG(streamParams.ecChannelLayout, CH_LAYOUT_MONO, "ecChannelLayout mismatch");
}

/**
 * @tc.name  : Test ConvertToAudioStreamParams with MicIn fields
 * @tc.number: CapturerConvertToAudioStreamParams_005
 * @tc.desc  : Test ConvertToAudioStreamParams populates MicIn stream params.
 */
HWTEST_F(AudioCapturerStaticAiut, CapturerConvertToAudioStreamParams_005, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    AudioCapturerParams capturerParams;
    capturerParams.audioSampleFormat = SAMPLE_S16LE;
    capturerParams.samplingRate = SAMPLE_RATE_48000;
    capturerParams.audioChannel = STEREO;
    capturerParams.audioEncoding = ENCODING_PCM;
    capturerParams.audioMicInSampleFormat = SAMPLE_S16LE;
    capturerParams.micInSamplingRate = SAMPLE_RATE_16000;
    capturerParams.audioMicInChannel = MONO;
    capturerParams.audioMicInEncoding = ENCODING_PCM;
    capturerParams.micInChannelLayout = CH_LAYOUT_MONO;

    AudioStreamParams streamParams = AudioCapturerPrivate::ConvertToAudioStreamParams(capturerParams);

    AIUT_EXPECT_EQ_INFO_LOG(streamParams.micInFormat, SAMPLE_S16LE, "micInFormat mismatch");
    AIUT_EXPECT_EQ_INFO_LOG(streamParams.micInSamplingRate, SAMPLE_RATE_16000,
        "micInSamplingRate mismatch");
    AIUT_EXPECT_EQ_INFO_LOG(streamParams.micInChannels, MONO, "micInChannels mismatch");
    AIUT_EXPECT_EQ_INFO_LOG(streamParams.micInEncoding, ENCODING_PCM, "micInEncoding mismatch");
    AIUT_EXPECT_EQ_INFO_LOG(streamParams.micInChannelLayout, CH_LAYOUT_MONO,
        "micInChannelLayout mismatch");
}

// ==================== GetSupportedFormats/Channels/EncodingTypes/SamplingRates Tests ====================

/**
 * @tc.name  : Test GetSupportedFormats returns non-empty vector
 * @tc.number: CapturerGetSupportedFormats_001
 * @tc.desc  : Verify GetSupportedFormats returns a non-empty list of supported sample formats.
 */
HWTEST_F(AudioCapturerStaticAiut, CapturerGetSupportedFormats_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto formats = AudioCapturer::GetSupportedFormats();
    AIUT_EXPECT_FALSE_INFO_LOG(formats.empty(), "supported formats should not be empty");
}

/**
 * @tc.name  : Test GetSupportedChannels returns non-empty vector
 * @tc.number: CapturerGetSupportedChannels_001
 * @tc.desc  : Verify GetSupportedChannels returns a non-empty list of supported channels.
 */
HWTEST_F(AudioCapturerStaticAiut, CapturerGetSupportedChannels_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto channels = AudioCapturer::GetSupportedChannels();
    AIUT_EXPECT_FALSE_INFO_LOG(channels.empty(), "supported channels should not be empty");
}

/**
 * @tc.name  : Test GetSupportedEncodingTypes returns non-empty vector
 * @tc.number: CapturerGetSupportedEncodingTypes_001
 * @tc.desc  : Verify GetSupportedEncodingTypes returns a non-empty list.
 */
HWTEST_F(AudioCapturerStaticAiut, CapturerGetSupportedEncodingTypes_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto encodings = AudioCapturer::GetSupportedEncodingTypes();
    AIUT_EXPECT_FALSE_INFO_LOG(encodings.empty(), "supported encoding types should not be empty");
}

/**
 * @tc.name  : Test GetSupportedSamplingRates returns non-empty vector
 * @tc.number: CapturerGetSupportedSamplingRates_001
 * @tc.desc  : Verify GetSupportedSamplingRates returns a non-empty list.
 */
HWTEST_F(AudioCapturerStaticAiut, CapturerGetSupportedSamplingRates_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto rates = AudioCapturer::GetSupportedSamplingRates();
    AIUT_EXPECT_FALSE_INFO_LOG(rates.empty(), "supported sampling rates should not be empty");
}

// ==================== AudioCapturerPrivate Construction Tests ====================

/**
 * @tc.name  : Test AudioCapturerPrivate constructor without stream creation
 * @tc.number: AudioCapturerPrivate_Constructor_001
 * @tc.desc  : Test AudioCapturerPrivate constructor with createStream=false creates proxy obj.
 */
HWTEST_F(AudioCapturerStaticAiut, AudioCapturerPrivate_Constructor_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    AppInfo appInfo;
    appInfo.appUid = TEST_APP_UID;
    appInfo.appPid = TEST_APP_PID;
    auto capturer = std::make_shared<AudioCapturerPrivate>(STREAM_MUSIC, appInfo, false);
    AIUT_ASSERT_NE_INFO_LOG(capturer, nullptr, "capturer is null");

    AIUT_ASSERT_NE_INFO_LOG(capturer->capturerProxyObj_, nullptr, "proxy obj should be created");
}

// ==================== GetStatus Tests ====================

/**
 * @tc.name  : Test GetStatus returns initial state
 * @tc.number: CapturerGetStatus_001
 * @tc.desc  : Test GetStatus returns CAPTURER_PREPARED after construction without stream.
 */
HWTEST_F(AudioCapturerStaticAiut, CapturerGetStatus_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto capturer = std::make_shared<AudioCapturerPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(capturer, nullptr, "capturer is null");
    CapturerState state = capturer->GetStatus();
    AIUT_EXPECT_EQ_INFO_LOG(state, CAPTURER_PREPARED,
        "initial state should be PREPARED, state=%d", state);
}

// ==================== GetCapturerDebugKey Tests ====================

/**
 * @tc.name  : Test GetCapturerDebugKey returns initial value
 * @tc.number: CapturerGetCapturerDebugKey_001
 * @tc.desc  : Test GetCapturerDebugKey returns 0 for newly created capturer.
 */
HWTEST_F(AudioCapturerStaticAiut, CapturerGetCapturerDebugKey_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto capturer = std::make_shared<AudioCapturerPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(capturer, nullptr, "capturer is null");
    uint32_t key = capturer->GetCapturerDebugKey();
    AIUT_EXPECT_EQ_INFO_LOG(key, 0, "initial debug key should be 0, key=%u", key);
}

// ==================== GetCapturerDebugInfo Tests ====================

/**
 * @tc.name  : Test GetCapturerDebugInfo with no stream
 * @tc.number: CapturerGetCapturerDebugInfo_001
 * @tc.desc  : Test GetCapturerDebugInfo returns error when audioStream_ is null.
 */
HWTEST_F(AudioCapturerStaticAiut, CapturerGetCapturerDebugInfo_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto capturer = std::make_shared<AudioCapturerPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(capturer, nullptr, "capturer is null");
    AudioCapturerDebugInfo debugInfo;
    int32_t ret = capturer->GetCapturerDebugInfo(debugInfo);
    AIUT_EXPECT_NE_INFO_LOG(ret, SUCCESS,
        "GetCapturerDebugInfo should fail without stream, ret=%d", ret);
}

// ==================== SetCapturerInfoByOptions Tests ====================

/**
 * @tc.name  : Test SetCapturerInfoByOptions sets capturerInfo correctly
 * @tc.number: CapturerSetCapturerInfoByOptions_001
 * @tc.desc  : Test SetCapturerInfoByOptions copies fields from capturerOptions to capturerInfo_.
 */
HWTEST_F(AudioCapturerStaticAiut, CapturerSetCapturerInfoByOptions_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto capturer = std::make_shared<AudioCapturerPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(capturer, nullptr, "capturer is null");
    AudioCapturerOptions capturerOptions;
    capturerOptions.capturerInfo.sourceType = SOURCE_TYPE_MIC;
    capturerOptions.capturerInfo.capturerFlags = TEST_CAPTURER_FLAGS_NORMAL;
    capturerOptions.streamInfo.samplingRate = static_cast<AudioSamplingRate>(TEST_SAMPLE_RATE_44100);
    AppInfo appInfo;

    capturer->SetCapturerInfoByOptions(capturerOptions, appInfo);

    AIUT_EXPECT_EQ_INFO_LOG(capturer->capturerInfo_.sourceType, SOURCE_TYPE_MIC,
        "sourceType mismatch");
}

/**
 * @tc.name  : Test AudioCapturerOptions actualUsingTokenId default value
 * @tc.number: CapturerOptionsActualUsingTokenId_001
 * @tc.desc  : AudioCapturerOptions.actualUsingTokenId defaults to 0 (compat behavior).
 */
HWTEST_F(AudioCapturerStaticAiut, CapturerOptionsActualUsingTokenId_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    AudioCapturerOptions capturerOptions;
    AIUT_EXPECT_EQ_INFO_LOG(capturerOptions.actualUsingTokenId, 0u,
        "actualUsingTokenId should default to 0");
}

// ==================== AudioCapturerFastAiut ====================

void AudioCapturerFastAiut::SetUpTestCase(void) {}
void AudioCapturerFastAiut::TearDownTestCase(void) {}
void AudioCapturerFastAiut::SetUp(void) {}
void AudioCapturerFastAiut::TearDown(void) {}

// ==================== GetFastStatus Tests ====================

/**
 * @tc.name  : Test GetFastStatus returns initial value
 * @tc.number: CapturerGetFastStatus_001
 * @tc.desc  : Test GetFastStatus returns FASTSTATUS_NORMAL for non-fast capturer.
 */
HWTEST_F(AudioCapturerFastAiut, CapturerGetFastStatus_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto capturer = std::make_shared<AudioCapturerPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(capturer, nullptr, "capturer is null");
    FastStatus status = capturer->GetFastStatus();
    AIUT_EXPECT_EQ_INFO_LOG(status, FASTSTATUS_NORMAL,
        "non-fast capturer should return FASTSTATUS_NORMAL, status=%d", status);
}

} // namespace AudioStandard
} // namespace OHOS
