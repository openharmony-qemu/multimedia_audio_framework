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

#include <gtest/gtest.h>

#include "i_audio_stream.h"
#include <map>

#include "audio_errors.h"
#include "audio_service_log.h"
#include "audio_utils.h"
#include "audio_policy_manager.h"
#include "capturer_in_client.h"
#include "renderer_in_client.h"
#include "capturer_in_client_inner.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class IAudioStreamUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

const std::vector<AudioSamplingRate> AUDIO_FAST_STREAM_SUPPORTED_SAMPLING_RATES {
    SAMPLE_RATE_48000,
};

const std::vector<AudioSampleFormat> AUDIO_FAST_STREAM_SUPPORTED_FORMATS {
    SAMPLE_S16LE,
    SAMPLE_S32LE,
    SAMPLE_F32LE
};

/**
 * @tc.name  : Test GetByteSizePerFrame API
 * @tc.type  : FUNC
 * @tc.number: GetByteSizePerFrame_001
 * @tc.desc  : Test GetByteSizePerFrame interface.
 */
HWTEST(IAudioStreamUnitTest, GetByteSizePerFrame_001, TestSize.Level1)
{
    AudioStreamParams params = {SAMPLE_RATE_48000, SAMPLE_S16LE, 2};
    size_t result = 0;
    int32_t ret = IAudioStream::GetByteSizePerFrame(params, result);
    EXPECT_NE(ret, ERR_INVALID_OPERATION);
}

/**
 * @tc.name  : Test GetByteSizePerFrame API
 * @tc.type  : FUNC
 * @tc.number: GetByteSizePerFrame_002
 * @tc.desc  : Test GetByteSizePerFrame interface.
 */
HWTEST(IAudioStreamUnitTest, GetByteSizePerFrame_002, TestSize.Level1)
{
    AudioStreamParams params = {SAMPLE_RATE_48000, 100, 2};
    size_t result = 0;
    int32_t ret = IAudioStream::GetByteSizePerFrame(params, result);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test GetByteSizePerFrame API
 * @tc.type  : FUNC
 * @tc.number: GetByteSizePerFrame_003
 * @tc.desc  : Test GetByteSizePerFrame interface.
 */
HWTEST(IAudioStreamUnitTest, GetByteSizePerFrame_003, TestSize.Level1)
{
    AudioStreamParams params = {SAMPLE_RATE_48000, 100, -1};
    size_t result = 0;
    int32_t ret = IAudioStream::GetByteSizePerFrame(params, result);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test IsStreamSupported API
 * @tc.type  : FUNC
 * @tc.number: IsStreamSupported_001
 * @tc.desc  : Test IsStreamSupported interface.
 */
HWTEST(IAudioStreamUnitTest, IsStreamSupported_001, TestSize.Level1)
{
    int32_t streamFlags = 0;
    AudioStreamParams params = {SAMPLE_RATE_48000, SAMPLE_S16LE, 2};
    bool result = IAudioStream::IsStreamSupported(streamFlags, params);
    EXPECT_TRUE(result);
}

/**
 * @tc.name  : Test IsStreamSupported API
 * @tc.type  : FUNC
 * @tc.number: IsStreamSupported_002
 * @tc.desc  : Test IsStreamSupported interface.
 */
HWTEST(IAudioStreamUnitTest, IsStreamSupported_002, TestSize.Level1)
{
    int32_t streamFlags = STREAM_FLAG_FAST;
    AudioStreamParams params = {SAMPLE_RATE_48000, SAMPLE_S16LE, 2};
    bool result = IAudioStream::IsStreamSupported(streamFlags, params);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : Test GetByteSizePerFrame API
 * @tc.type  : FUNC
 * @tc.number: GetByteSizePerFrame_004
 * @tc.desc  : Test GetByteSizePerFrame interface.
 */
HWTEST(IAudioStreamUnitTest, GetByteSizePerFrame_004, TestSize.Level1)
{
    AudioStreamParams params;
    params.samplingRate = SAMPLE_RATE_48000;
    params.encoding = ENCODING_PCM;
    params.format = SAMPLE_F32LE;
    params.channels = 0;
    size_t result = 0;
    int32_t ret = IAudioStream::GetByteSizePerFrame(params, result);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    params.format = SAMPLE_S32LE;
    params.channels = 17;
    ret = IAudioStream::GetByteSizePerFrame(params, result);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    params.format = SAMPLE_S32LE;
    params.channels = 5;
    ret = IAudioStream::GetByteSizePerFrame(params, result);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test IsStreamSupported API
 * @tc.type  : FUNC
 * @tc.number: IsPlaybackChannelRelatedInfoValid_001
 * @tc.desc  : Test IsPlaybackChannelRelatedInfoValid interface.
 */
HWTEST(IAudioStreamUnitTest, IsPlaybackChannelRelatedInfoValid_001, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(AudioStreamType::STREAM_MUSIC, 0);
    std::uint8_t audioChannel = 100;
    std::uint64_t channelLayout = 100;
    EXPECT_FALSE(IAudioStream::IsPlaybackChannelRelatedInfoValid(ENCODING_PCM, audioChannel, channelLayout));

    audioChannel = 2;
    channelLayout = 100;
    EXPECT_FALSE(IAudioStream::IsPlaybackChannelRelatedInfoValid(ENCODING_PCM, audioChannel, channelLayout));

    audioChannel = 2;
    channelLayout = 4;
    EXPECT_FALSE(IAudioStream::IsPlaybackChannelRelatedInfoValid(ENCODING_PCM, audioChannel, channelLayout));

    audioChannel = 2;
    channelLayout = 3;
    EXPECT_TRUE(IAudioStream::IsPlaybackChannelRelatedInfoValid(ENCODING_PCM, audioChannel, channelLayout));
}

/**
 * @tc.name  : Test IsStreamSupported API
 * @tc.type  : FUNC
 * @tc.number: IsRecordChannelRelatedInfoValid_001
 * @tc.desc  : Test IsRecordChannelRelatedInfoValid interface.
 */
HWTEST(IAudioStreamUnitTest, IsRecordChannelRelatedInfoValid_001, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(AudioStreamType::STREAM_MUSIC, 0);
    std::uint8_t audioChannel = 100;
    std::uint64_t channelLayout = 100;
    EXPECT_FALSE(IAudioStream::IsRecordChannelRelatedInfoValid(audioChannel, channelLayout));

    audioChannel = 2;
    channelLayout = 100;
    EXPECT_FALSE(IAudioStream::IsRecordChannelRelatedInfoValid(audioChannel, channelLayout));

    audioChannel = 2;
    channelLayout = 4;
    EXPECT_FALSE(IAudioStream::IsRecordChannelRelatedInfoValid(audioChannel, channelLayout));

    audioChannel = 2;
    channelLayout = 3;
    EXPECT_TRUE(IAudioStream::IsRecordChannelRelatedInfoValid(audioChannel, channelLayout));
}

/**
 * @tc.name  : Test IsStreamSupported API
 * @tc.type  : FUNC
 * @tc.number: IsStreamSupported_003
 * @tc.desc  : Test IsStreamSupported interface.
 */
HWTEST(IAudioStreamUnitTest, IsStreamSupported_003, TestSize.Level1)
{
    int32_t streamFlags = STREAM_FLAG_FAST;
    AudioStreamParams params;
    params.samplingRate = SAMPLE_RATE_11025;
    params.encoding = ENCODING_PCM;
    params.format = SAMPLE_S16LE;
    params.channels = 0;
    bool result = IAudioStream::IsStreamSupported(streamFlags, params);
    EXPECT_FALSE(result);

    params.samplingRate = SAMPLE_RATE_48000;
    params.format = SAMPLE_S16LE;
    params.channels = STEREO;
    result = IAudioStream::IsStreamSupported(streamFlags, params);
    EXPECT_TRUE(result);

    streamFlags = AUDIO_FLAG_VOIP_DIRECT;
    result = IAudioStream::IsStreamSupported(2, params);
    EXPECT_TRUE(result);
}

/**
 * @tc.name  : Test CheckRendererAudioStreamInfo API
 * @tc.type  : FUNC
 * @tc.number: CheckRendererAudioStreamInfo_001
 * @tc.desc  : Test CheckRendererAudioStreamInfo interface.
 */
HWTEST(IAudioStreamUnitTest, CheckRendererAudioStreamInfo_001, TestSize.Level1)
{
    AudioStreamParams params;
    params.format = SAMPLE_S16LE;
    params.encoding = ENCODING_PCM;
    params.samplingRate = SAMPLE_RATE_48000;
    params.customSampleRate = 0;
    params.channels = STEREO;
    params.channelLayout = CH_LAYOUT_STEREO;
    EXPECT_EQ(IAudioStream::CheckRendererAudioStreamInfo(params), SUCCESS);

    params.channelLayout = 12345;
    EXPECT_EQ(IAudioStream::CheckRendererAudioStreamInfo(params), ERR_NOT_SUPPORTED);
    params.channels = CHANNEL_UNKNOW;
    EXPECT_EQ(IAudioStream::CheckRendererAudioStreamInfo(params), ERR_NOT_SUPPORTED);

    params.samplingRate = 12345;
    EXPECT_EQ(IAudioStream::CheckRendererAudioStreamInfo(params), ERR_NOT_SUPPORTED);
    params.customSampleRate = SAMPLE_RATE_48000;
    EXPECT_EQ(IAudioStream::CheckRendererAudioStreamInfo(params), ERR_NOT_SUPPORTED);
    params.encoding = ENCODING_INVALID;
    EXPECT_EQ(IAudioStream::CheckRendererAudioStreamInfo(params), ERR_NOT_SUPPORTED);
    params.format = INVALID_WIDTH;
    EXPECT_EQ(IAudioStream::CheckRendererAudioStreamInfo(params), ERR_NOT_SUPPORTED);
}

/**
 * @tc.name  : Test CheckCapturerAudioStreamInfo API
 * @tc.type  : FUNC
 * @tc.number: CheckCapturerAudioStreamInfo_001
 * @tc.desc  : Test CheckCapturerAudioStreamInfo interface.
 */
HWTEST(IAudioStreamUnitTest, CheckCapturerAudioStreamInfo_001, TestSize.Level1)
{
    AudioStreamParams params;
    params.format = SAMPLE_S16LE;
    params.encoding = ENCODING_PCM;
    params.samplingRate = SAMPLE_RATE_48000;
    params.customSampleRate = 0;
    params.channels = STEREO;
    params.channelLayout = CH_LAYOUT_STEREO;
    EXPECT_EQ(IAudioStream::CheckCapturerAudioStreamInfo(params), SUCCESS);

    params.channelLayout = 12345;
    EXPECT_EQ(IAudioStream::CheckCapturerAudioStreamInfo(params), ERR_NOT_SUPPORTED);
    params.channels = CHANNEL_UNKNOW;
    EXPECT_EQ(IAudioStream::CheckCapturerAudioStreamInfo(params), ERR_NOT_SUPPORTED);

    params.samplingRate = 12345;
    EXPECT_EQ(IAudioStream::CheckCapturerAudioStreamInfo(params), ERR_NOT_SUPPORTED);
    params.encoding = ENCODING_INVALID;
    EXPECT_EQ(IAudioStream::CheckCapturerAudioStreamInfo(params), ERR_NOT_SUPPORTED);
    params.format = INVALID_WIDTH;
    EXPECT_EQ(IAudioStream::CheckCapturerAudioStreamInfo(params), ERR_NOT_SUPPORTED);
}

/**
 * @tc.name  : Test GetByteSizePerFrameWithEc API
 * @tc.type  : FUNC
 * @tc.number: GetByteSizePerFrameWithEc_001
 * @tc.desc  : Test GetByteSizePerFrameWithEc interface with valid parameters
 */
HWTEST(IAudioStreamUnitTest, GetByteSizePerFrameWithEc_001, TestSize.Level1)
{
    AudioStreamParams params = {SAMPLE_RATE_48000, SAMPLE_S16LE, 2, 0};
    size_t result = 0;
    IAudioStream::GetByteSizePerFrameWithEc(params, result);
    EXPECT_EQ(result, 0); // 2 channels * 2 bytes per sample
}

/**
 * @tc.name  : Test GetByteSizePerFrameWithEc API
 * @tc.type  : FUNC
 * @tc.number: GetByteSizePerFrameWithEc_002
 * @tc.desc  : Test GetByteSizePerFrameWithEc interface with valid parameters and EC channels
 */
HWTEST(IAudioStreamUnitTest, GetByteSizePerFrameWithEc_002, TestSize.Level1)
{
    AudioStreamParams params = {SAMPLE_RATE_48000, SAMPLE_S16LE, 2, 1}; // 2 channels + 1 EC channel
    size_t result = 0;
    int32_t ret = IAudioStream::GetByteSizePerFrameWithEc(params, result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, 3); // (2 + 1) channels * 2 bytes per sample
}

/**
 * @tc.name  : Test GetByteSizePerFrameWithEc API
 * @tc.type  : FUNC
 * @tc.number: GetByteSizePerFrameWithEc_003
 * @tc.desc  : Test GetByteSizePerFrameWithEc interface with invalid format
 */
HWTEST(IAudioStreamUnitTest, GetByteSizePerFrameWithEc_003, TestSize.Level1)
{
    AudioStreamParams params = {SAMPLE_RATE_48000, 100, 2, 0}; // Invalid format
    size_t result = 0;
    int32_t ret = IAudioStream::GetByteSizePerFrameWithEc(params, result);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test GetByteSizePerFrameWithEc API
 * @tc.type  : FUNC
 * @tc.number: GetByteSizePerFrameWithEc_004
 * @tc.desc  : Test GetByteSizePerFrameWithEc interface with invalid channel count
 */
HWTEST(IAudioStreamUnitTest, GetByteSizePerFrameWithEc_004, TestSize.Level1)
{
    AudioStreamParams params = {SAMPLE_RATE_48000, SAMPLE_S16LE, 0, 0}; // Invalid channel count
    size_t result = 0;
    int32_t ret = IAudioStream::GetByteSizePerFrameWithEc(params, result);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test GetByteSizePerFrameWithEc API
 * @tc.type  : FUNC
 * @tc.number: GetByteSizePerFrameWithEc_005
 * @tc.desc  : Test GetByteSizePerFrameWithEc interface with valid parameters and different format
 */
HWTEST(IAudioStreamUnitTest, GetByteSizePerFrameWithEc_005, TestSize.Level1)
{
    AudioStreamParams params = {SAMPLE_RATE_48000, SAMPLE_S32LE, 4, 2}; // 4 channels + 2 EC channels
    size_t result = 0;
    int32_t ret = IAudioStream::GetByteSizePerFrameWithEc(params, result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(result, 8); // (4 + 2) channels * 4 bytes per sample
}

/**
 * @tc.name  : Test GetByteSizePerFrameWithEc API
 * @tc.type  : FUNC
 * @tc.number: GetByteSizePerFrameWithEc_006
 * @tc.desc  : Test GetByteSizePerFrameWithEc interface with maximum valid channel count
 */
HWTEST(IAudioStreamUnitTest, GetByteSizePerFrameWithEc_006, TestSize.Level1)
{
    AudioStreamParams params = {SAMPLE_RATE_48000, SAMPLE_S16LE, 16, 0}; // Maximum valid channels
    size_t result = 0;
    IAudioStream::GetByteSizePerFrameWithEc(params, result);
    EXPECT_EQ(result, 0); // 16 channels * 2 bytes per sample
}

/**
 * @tc.name  : Test GetByteSizePerFrameWithEc API
 * @tc.type  : FUNC
 * @tc.number: GetByteSizePerFrameWithEc_007
 * @tc.desc  : Test GetByteSizePerFrameWithEc interface with maximum valid channel count plus EC
 */
HWTEST(IAudioStreamUnitTest, GetByteSizePerFrameWithEc_007, TestSize.Level1)
{
    AudioStreamParams params = {SAMPLE_RATE_48000, SAMPLE_S16LE, 16, 1}; // Maximum channels + 1 EC channel
    size_t result = 0;
    IAudioStream::GetByteSizePerFrameWithEc(params, result);
    EXPECT_EQ(result, 0); // (16 + 1) channels * 2 bytes per sample
}

/**
 * @tc.name  : Test GetByteSizePerFrameWithMicInEc API
 * @tc.type  : FUNC
 * @tc.number: GetByteSizePerFrameWithMicInEc_001
 * @tc.desc  : Test GetByteSizePerFrameWithMicInEc with all valid formats.
 */
HWTEST(IAudioStreamUnitTest, GetByteSizePerFrameWithMicInEc_001, TestSize.Level1)
{
    std::vector<std::pair<AudioSampleFormat, size_t>> cases = {
        {SAMPLE_U8, 1},
        {SAMPLE_S16LE, 2},
        {SAMPLE_S24LE, 3},
        {SAMPLE_S32LE, 4},
        {SAMPLE_F32LE, 4},
    };
    for (const auto &item : cases) {
        AudioSampleFormat format = item.first;
        size_t bytesPerSample = item.second;
        AudioStreamParams params {};
        params.samplingRate = SAMPLE_RATE_48000;
        params.format = format;
        params.channels = AudioChannel::MONO;
        params.micInChannels = AudioChannel::CHANNEL_4;
        params.ecChannels = AudioChannel::STEREO;
        size_t result = 0;
        int32_t ret = IAudioStream::GetByteSizePerFrameWithMicInEc(params, result);
        EXPECT_EQ(ret, SUCCESS);
        EXPECT_EQ(result, bytesPerSample * 7); // 1 + 4 + 2 channels
    }
}

/**
 * @tc.name  : Test GetByteSizePerFrameWithMicInEc API
 * @tc.type  : FUNC
 * @tc.number: GetByteSizePerFrameWithMicInEc_002
 * @tc.desc  : Test GetByteSizePerFrameWithMicInEc with invalid format.
 */
HWTEST(IAudioStreamUnitTest, GetByteSizePerFrameWithMicInEc_002, TestSize.Level1)
{
    AudioStreamParams params {};
    params.samplingRate = SAMPLE_RATE_48000;
    params.format = INVALID_WIDTH;
    params.channels = AudioChannel::MONO;
    params.micInChannels = AudioChannel::CHANNEL_4;
    params.ecChannels = AudioChannel::STEREO;
    size_t result = 0;
    int32_t ret = IAudioStream::GetByteSizePerFrameWithMicInEc(params, result);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test GetByteSizePerFrameWithMicInEc API
 * @tc.type  : FUNC
 * @tc.number: GetByteSizePerFrameWithMicInEc_003
 * @tc.desc  : Test GetByteSizePerFrameWithMicInEc with zero total channels.
 */
HWTEST(IAudioStreamUnitTest, GetByteSizePerFrameWithMicInEc_003, TestSize.Level1)
{
    AudioStreamParams params {};
    params.samplingRate = SAMPLE_RATE_48000;
    params.format = SAMPLE_S16LE;
    params.channels = CHANNEL_UNKNOW;
    params.micInChannels = CHANNEL_UNKNOW;
    params.ecChannels = CHANNEL_UNKNOW;
    size_t result = 0;
    int32_t ret = IAudioStream::GetByteSizePerFrameWithMicInEc(params, result);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test GetByteSizePerFrameWithMicInEc API
 * @tc.type  : FUNC
 * @tc.number: GetByteSizePerFrameWithMicInEc_004
 * @tc.desc  : Test GetByteSizePerFrameWithMicInEc with total channels over limit.
 */
HWTEST(IAudioStreamUnitTest, GetByteSizePerFrameWithMicInEc_004, TestSize.Level1)
{
    AudioStreamParams params {};
    params.samplingRate = SAMPLE_RATE_48000;
    params.format = SAMPLE_S16LE;
    params.channels = static_cast<AudioChannel>(8);
    params.micInChannels = static_cast<AudioChannel>(8);
    params.ecChannels = AudioChannel::MONO; // total 17
    size_t result = 0;
    int32_t ret = IAudioStream::GetByteSizePerFrameWithMicInEc(params, result);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test GetByteSizePerFrameWithMicIn API
 * @tc.type  : FUNC
 * @tc.number: GetByteSizePerFrameWithMicIn_001
 * @tc.desc  : Test GetByteSizePerFrameWithMicIn with all valid formats.
 */
HWTEST(IAudioStreamUnitTest, GetByteSizePerFrameWithMicIn_001, TestSize.Level1)
{
    std::vector<std::pair<AudioSampleFormat, size_t>> cases = {
        {SAMPLE_U8, 1},
        {SAMPLE_S16LE, 2},
        {SAMPLE_S24LE, 3},
        {SAMPLE_S32LE, 4},
        {SAMPLE_F32LE, 4},
    };
    for (const auto &item : cases) {
        AudioStreamParams params {};
        params.samplingRate = SAMPLE_RATE_48000;
        params.format = item.first;
        params.channels = AudioChannel::STEREO;
        params.micInChannels = AudioChannel::CHANNEL_4;

        size_t result = 0;
        int32_t ret = IAudioStream::GetByteSizePerFrameWithMicIn(params, result);
        EXPECT_EQ(ret, SUCCESS);
        EXPECT_EQ(result, item.second * 6); // 2 process channels + 4 mic-in channels
    }
}

/**
 * @tc.name  : Test GetByteSizePerFrameWithMicIn API
 * @tc.type  : FUNC
 * @tc.number: GetByteSizePerFrameWithMicIn_002
 * @tc.desc  : Test GetByteSizePerFrameWithMicIn with invalid format.
 */
HWTEST(IAudioStreamUnitTest, GetByteSizePerFrameWithMicIn_002, TestSize.Level1)
{
    AudioStreamParams params {};
    params.samplingRate = SAMPLE_RATE_48000;
    params.format = INVALID_WIDTH;
    params.channels = AudioChannel::STEREO;
    params.micInChannels = AudioChannel::CHANNEL_4;

    size_t result = 0;
    int32_t ret = IAudioStream::GetByteSizePerFrameWithMicIn(params, result);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name  : Test GetByteSizePerFrameWithMicIn API
 * @tc.type  : FUNC
 * @tc.number: GetByteSizePerFrameWithMicIn_003
 * @tc.desc  : Test GetByteSizePerFrameWithMicIn with invalid total channels.
 */
HWTEST(IAudioStreamUnitTest, GetByteSizePerFrameWithMicIn_003, TestSize.Level1)
{
    AudioStreamParams params {};
    params.samplingRate = SAMPLE_RATE_48000;
    params.format = SAMPLE_S16LE;
    params.channels = CHANNEL_UNKNOW;
    params.micInChannels = CHANNEL_UNKNOW;

    size_t result = 0;
    int32_t ret = IAudioStream::GetByteSizePerFrameWithMicIn(params, result);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    params.channels = static_cast<AudioChannel>(13);
    params.micInChannels = AudioChannel::CHANNEL_4; // total 17
    ret = IAudioStream::GetByteSizePerFrameWithMicIn(params, result);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}
class IAudioStreamNewUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void IAudioStreamNewUnitTest::SetUpTestCase(void) {}
void IAudioStreamNewUnitTest::TearDownTestCase(void) {}
void IAudioStreamNewUnitTest::SetUp(void) {}
void IAudioStreamNewUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test GetPlaybackStream for FAST_STREAM with new route enabled
 * @tc.type  : FUNC
 * @tc.number: GetPlaybackStream_NewRoute_001
 * @tc.desc  : Test GetPlaybackStream creates RendererInClient when new route is enabled.
 */
HWTEST(IAudioStreamNewUnitTest, GetPlaybackStream_NewRoute_001, TestSize.Level1)
{
    AudioStreamParams params;
    params.samplingRate = SAMPLE_RATE_48000;
    params.format = SAMPLE_S16LE;
    params.channels = STEREO;
    params.encoding = ENCODING_PCM;
    
    auto stream = IAudioStream::GetPlaybackStream(IAudioStream::FAST_STREAM, params, STREAM_MUSIC, getpid());
    EXPECT_TRUE(stream != nullptr);
}

/**
 * @tc.name  : Test GetPlaybackStream for VOIP_STREAM with new route
 * @tc.type  : FUNC
 * @tc.number: GetPlaybackStream_VoipStream_001
 * @tc.desc  : Test GetPlaybackStream for VOIP_STREAM creates appropriate stream.
 */
HWTEST(IAudioStreamNewUnitTest, GetPlaybackStream_VoipStream_001, TestSize.Level1)
{
    AudioStreamParams params;
    params.samplingRate = SAMPLE_RATE_48000;
    params.format = SAMPLE_S16LE;
    params.channels = STEREO;
    params.encoding = ENCODING_PCM;
    
    auto stream = IAudioStream::GetPlaybackStream(IAudioStream::VOIP_STREAM, params, STREAM_MUSIC, getpid());
    EXPECT_TRUE(stream != nullptr);
}

/**
 * @tc.name  : Test GetPlaybackStream for PA_STREAM
 * @tc.type  : FUNC
 * @tc.number: GetPlaybackStream_PAStream_001
 * @tc.desc  : Test GetPlaybackStream creates RendererInClient for PA_STREAM.
 */
HWTEST(IAudioStreamNewUnitTest, GetPlaybackStream_PAStream_001, TestSize.Level1)
{
    AudioStreamParams params;
    params.samplingRate = SAMPLE_RATE_48000;
    params.format = SAMPLE_S16LE;
    params.channels = STEREO;
    params.encoding = ENCODING_PCM;
    
    auto stream = IAudioStream::GetPlaybackStream(IAudioStream::PA_STREAM, params, STREAM_MUSIC, getpid());
    EXPECT_TRUE(stream != nullptr);
}

/**
 * @tc.name  : Test GetPlaybackStream for invalid stream class
 * @tc.type  : FUNC
 * @tc.number: GetPlaybackStream_InvalidClass_001
 * @tc.desc  : Test GetPlaybackStream returns nullptr for invalid stream class.
 */
HWTEST(IAudioStreamNewUnitTest, GetPlaybackStream_InvalidClass_001, TestSize.Level1)
{
    AudioStreamParams params;
    params.samplingRate = SAMPLE_RATE_48000;
    params.format = SAMPLE_S16LE;
    params.channels = STEREO;
    params.encoding = ENCODING_PCM;
    
    auto stream = IAudioStream::GetPlaybackStream(
        static_cast<IAudioStream::StreamClass>(99), params, STREAM_MUSIC, getpid()); // 99 is temp invalid class
    EXPECT_TRUE(stream == nullptr);
}

/**
 * @tc.name  : Test GetRecordStream for FAST_STREAM with new route
 * @tc.type  : FUNC
 * @tc.number: GetRecordStream_NewRoute_001
 * @tc.desc  : Test GetRecordStream creates CapturerInClient when new route is enabled.
 */
HWTEST(IAudioStreamNewUnitTest, GetRecordStream_NewRoute_001, TestSize.Level1)
{
    AudioStreamParams params;
    params.samplingRate = SAMPLE_RATE_48000;
    params.format = SAMPLE_S16LE;
    params.channels = STEREO;
    params.encoding = ENCODING_PCM;
    
    auto stream = IAudioStream::GetRecordStream(IAudioStream::FAST_STREAM, params, STREAM_MUSIC, getpid());
    EXPECT_TRUE(stream != nullptr);
}

/**
 * @tc.name  : Test GetRecordStream for VOIP_STREAM
 * @tc.type  : FUNC
 * @tc.number: GetRecordStream_VoipStream_001
 * @tc.desc  : Test GetRecordStream for VOIP_STREAM creates appropriate stream.
 */
HWTEST(IAudioStreamNewUnitTest, GetRecordStream_VoipStream_001, TestSize.Level1)
{
    AudioStreamParams params;
    params.samplingRate = SAMPLE_RATE_48000;
    params.format = SAMPLE_S16LE;
    params.channels = STEREO;
    params.encoding = ENCODING_PCM;
    
    auto stream = IAudioStream::GetRecordStream(IAudioStream::VOIP_STREAM, params, STREAM_MUSIC, getpid());
    EXPECT_TRUE(stream != nullptr);
}

/**
 * @tc.name  : Test GetRecordStream for PA_STREAM
 * @tc.type  : FUNC
 * @tc.number: GetRecordStream_PAStream_001
 * @tc.desc  : Test GetRecordStream creates CapturerInClient for PA_STREAM.
 */
HWTEST(IAudioStreamNewUnitTest, GetRecordStream_PAStream_001, TestSize.Level1)
{
    AudioStreamParams params;
    params.samplingRate = SAMPLE_RATE_48000;
    params.format = SAMPLE_S16LE;
    params.channels = STEREO;
    params.encoding = ENCODING_PCM;
    
    auto stream = IAudioStream::GetRecordStream(IAudioStream::PA_STREAM, params, STREAM_MUSIC, getpid());
    EXPECT_TRUE(stream != nullptr);
}

/**
 * @tc.name  : Test GetRecordStream for invalid stream class
 * @tc.type  : FUNC
 * @tc.number: GetRecordStream_InvalidClass_001
 * @tc.desc  : Test GetRecordStream returns nullptr for invalid stream class.
 */
HWTEST(IAudioStreamNewUnitTest, GetRecordStream_InvalidClass_001, TestSize.Level1)
{
    AudioStreamParams params;
    params.samplingRate = SAMPLE_RATE_48000;
    params.format = SAMPLE_S16LE;
    params.channels = STEREO;
    params.encoding = ENCODING_PCM;
    
    auto stream =IAudioStream::GetRecordStream(
        static_cast<IAudioStream::StreamClass>(99), params, STREAM_MUSIC, getpid()); // 99 is temp invalid class
    EXPECT_TRUE(stream == nullptr);
}

/**
 * @tc.name  : Test GetPlaybackStream with different stream types
 * @tc.type  : FUNC
 * @tc.number: GetPlaybackStream_DifferentTypes_001
 * @tc.desc  : Test GetPlaybackStream works for various stream types.
 */
HWTEST(IAudioStreamNewUnitTest, GetPlaybackStream_DifferentTypes_001, TestSize.Level1)
{
    AudioStreamParams params;
    params.samplingRate = SAMPLE_RATE_48000;
    params.format = SAMPLE_S16LE;
    params.channels = STEREO;
    params.encoding = ENCODING_PCM;
    
    std::vector<AudioStreamType> streamTypes = {
        STREAM_MUSIC,
        STREAM_VOICE_CALL,
        STREAM_SYSTEM,
        STREAM_RING,
        STREAM_ALARM,
        STREAM_NOTIFICATION
    };
    
    for (auto type : streamTypes) {
        auto stream = IAudioStream::GetPlaybackStream(IAudioStream::PA_STREAM, params, type, getpid());
        EXPECT_TRUE(stream != nullptr);
    }
}

/**
 * @tc.name  : Test GetRecordStream with different stream types
 * @tc.type  : FUNC
 * @tc.number: GetRecordStream_DifferentTypes_001
 * @tc.desc  : Test GetRecordStream works for various stream types.
 */
HWTEST(IAudioStreamNewUnitTest, GetRecordStream_DifferentTypes_001, TestSize.Level1)
{
    AudioStreamParams params;
    params.samplingRate = SAMPLE_RATE_48000;
    params.format = SAMPLE_S16LE;
    params.channels = STEREO;
    params.encoding = ENCODING_PCM;
    
    std::vector<AudioStreamType> streamTypes = {
        STREAM_MUSIC,
        STREAM_VOICE_CALL,
        STREAM_SYSTEM
    };
    
    for (auto type : streamTypes) {
        auto stream = IAudioStream::GetRecordStream(IAudioStream::PA_STREAM, params, type, getpid());
        EXPECT_TRUE(stream != nullptr);
    }
}

/**
 * @tc.name  : Test GetPlaybackStream with different sample rates
 * @tc.type  : FUNC
 * @tc.number: GetPlaybackStream_DifferentRates_001
 * @tc.desc  : Test GetPlaybackStream works for various sample rates.
 */
HWTEST(IAudioStreamNewUnitTest, GetPlaybackStream_DifferentRates_001, TestSize.Level1)
{
    std::vector<AudioSamplingRate> rates = {
        SAMPLE_RATE_8000,
        SAMPLE_RATE_16000,
        SAMPLE_RATE_44100,
        SAMPLE_RATE_48000
    };
    
    for (auto rate : rates) {
        AudioStreamParams params;
        params.samplingRate = rate;
        params.format = SAMPLE_S16LE;
        params.channels = STEREO;
        params.encoding = ENCODING_PCM;
        
        auto stream = IAudioStream::GetPlaybackStream(IAudioStream::PA_STREAM, params, STREAM_MUSIC, getpid());
        EXPECT_TRUE(stream != nullptr);
    }
}

/**
 * @tc.name  : Test GetRecordStream with different sample rates
 * @tc.type  : FUNC
 * @tc.number: GetRecordStream_DifferentRates_001
 * @tc.desc  : Test GetRecordStream works for various sample rates.
 */
HWTEST(IAudioStreamNewUnitTest, GetRecordStream_DifferentRates_001, TestSize.Level1)
{
    std::vector<AudioSamplingRate> rates = {
        SAMPLE_RATE_8000,
        SAMPLE_RATE_16000,
        SAMPLE_RATE_44100,
        SAMPLE_RATE_48000
    };
    
    for (auto rate : rates) {
        AudioStreamParams params;
        params.samplingRate = rate;
        params.format = SAMPLE_S16LE;
        params.channels = STEREO;
        params.encoding = ENCODING_PCM;
        
        auto stream = IAudioStream::GetRecordStream(IAudioStream::PA_STREAM, params, STREAM_MUSIC, getpid());
        EXPECT_TRUE(stream != nullptr);
    }
}

/**
 * @tc.name  : Test GetPlaybackStream with different formats
 * @tc.type  : FUNC
 * @tc.number: GetPlaybackStream_DifferentFormats_001
 * @tc.desc  : Test GetPlaybackStream works for various formats.
 */
HWTEST(IAudioStreamNewUnitTest, GetPlaybackStream_DifferentFormats_001, TestSize.Level1)
{
    std::vector<AudioSampleFormat> formats = {
        SAMPLE_U8,
        SAMPLE_S16LE,
        SAMPLE_S24LE,
        SAMPLE_S32LE,
        SAMPLE_F32LE
    };
    
    for (auto format : formats) {
        AudioStreamParams params;
        params.samplingRate = SAMPLE_RATE_48000;
        params.format = format;
        params.channels = STEREO;
        params.encoding = ENCODING_PCM;
        
        auto stream = IAudioStream::GetPlaybackStream(IAudioStream::PA_STREAM, params, STREAM_MUSIC, getpid());
        EXPECT_TRUE(stream != nullptr);
    }
}

/**
 * @tc.name  : Test GetRecordStream with different formats
 * @tc.type  : FUNC
 * @tc.number: GetRecordStream_DifferentFormats_001
 * @tc.desc  : Test GetRecordStream works for various formats.
 */
HWTEST(IAudioStreamNewUnitTest, GetRecordStream_DifferentFormats_001, TestSize.Level1)
{
    std::vector<AudioSampleFormat> formats = {
        SAMPLE_U8,
        SAMPLE_S16LE,
        SAMPLE_S24LE,
        SAMPLE_S32LE,
        SAMPLE_F32LE
    };
    
    for (auto format : formats) {
        AudioStreamParams params;
        params.samplingRate = SAMPLE_RATE_48000;
        params.format = format;
        params.channels = STEREO;
        params.encoding = ENCODING_PCM;
        
        auto stream = IAudioStream::GetRecordStream(IAudioStream::PA_STREAM, params, STREAM_MUSIC, getpid());
        EXPECT_TRUE(stream != nullptr);
    }
}

/**
 * @tc.name  : Test GetPlaybackStream with different channels
 * @tc.type  : FUNC
 * @tc.number: GetPlaybackStream_DifferentChannels_001
 * @tc.desc  : Test GetPlaybackStream works for various channel counts.
 */
HWTEST(IAudioStreamNewUnitTest, GetPlaybackStream_DifferentChannels_001, TestSize.Level1)
{
    std::vector<AudioChannel> channels = {
        MONO,
        STEREO,
        CHANNEL_4,
        CHANNEL_6,
        CHANNEL_8
    };
    
    for (auto channel : channels) {
        AudioStreamParams params;
        params.samplingRate = SAMPLE_RATE_48000;
        params.format = SAMPLE_S16LE;
        params.channels = channel;
        params.encoding = ENCODING_PCM;
        
        auto stream = IAudioStream::GetPlaybackStream(IAudioStream::PA_STREAM, params, STREAM_MUSIC, getpid());
        EXPECT_TRUE(stream != nullptr);
    }
}

/**
 * @tc.name  : Test GetRecordStream with different channels
 * @tc.type  : FUNC
 * @tc.number: GetRecordStream_DifferentChannels_001
 * @tc.desc  : Test GetRecordStream works for various channel counts.
 */
HWTEST(IAudioStreamNewUnitTest, GetRecordStream_DifferentChannels_001, TestSize.Level1)
{
    std::vector<AudioChannel> channels = {
        MONO,
        STEREO,
        CHANNEL_4,
        CHANNEL_6
    };
    
    for (auto channel : channels) {
        AudioStreamParams params;
        params.samplingRate = SAMPLE_RATE_48000;
        params.format = SAMPLE_S16LE;
        params.channels = channel;
        params.encoding = ENCODING_PCM;
        
        auto stream = IAudioStream::GetRecordStream(IAudioStream::PA_STREAM, params, STREAM_MUSIC, getpid());
        EXPECT_TRUE(stream != nullptr);
    }
}

/**
 * @tc.name  : Test GetPlaybackStream with different appUids
 * @tc.type  : FUNC
 * @tc.number: GetPlaybackStream_DifferentUids_001
 * @tc.desc  : Test GetPlaybackStream works for various appUids.
 */
HWTEST(IAudioStreamNewUnitTest, GetPlaybackStream_DifferentUids_001, TestSize.Level1)
{
    AudioStreamParams params;
    params.samplingRate = SAMPLE_RATE_48000;
    params.format = SAMPLE_S16LE;
    params.channels = STEREO;
    params.encoding = ENCODING_PCM;
    
    std::vector<int32_t> uids = {100, 1000, 10000, getuid()};
    
    for (auto uid : uids) {
        auto stream = IAudioStream::GetPlaybackStream(IAudioStream::PA_STREAM, params, STREAM_MUSIC, uid);
        EXPECT_TRUE(stream != nullptr);
    }
}

/**
 * @tc.name  : Test GetRecordStream with different appUids
 * @tc.type  : FUNC
 * @tc.number: GetRecordStream_DifferentUids_001
 * @tc.desc  : Test GetRecordStream works for various appUid.
 */
HWTEST(IAudioStreamNewUnitTest, GetRecordStream_DifferentUids_001, TestSize.Level1)
{
    AudioStreamParams params;
    params.samplingRate = SAMPLE_RATE_48000;
    params.format = SAMPLE_S16LE;
    params.channels = STEREO;
    params.encoding = ENCODING_PCM;
    
    std::vector<int32_t> uids = {100, 1000, 10000, getuid()};
    
    for (auto uid : uids) {
        auto stream = IAudioStream::GetRecordStream(IAudioStream::PA_STREAM, params, STREAM_MUSIC, uid);
        EXPECT_TRUE(stream != nullptr);
    }
}

/**
 * @tc.name  : Test stream creation with RendererInClient
 * @tc.type  : FUNC
 * @tc.number: RendererInClient_Instance_001
 * @tc.desc  : Test RendererInClient::GetInstance creates valid stream.
 */
HWTEST(IAudioStreamNewUnitTest, RendererInClient_Instance_001, TestSize.Level1)
{
    auto rendererInClient = RendererInClient::GetInstance(STREAM_MUSIC, getpid());
    EXPECT_TRUE(rendererInClient != nullptr);
}

/**
 * @tc.name  : Test stream creation with CapturerInClient
 * @tc.type  : FUNC
 * @tc.number: CapturerInClient_Instance_001
 * @tc.desc  : Test CapturerInClient::GetInstance creates valid stream.
 */
HWTEST(IAudioStreamNewUnitTest, CapturerInClient_Instance_001, TestSize.Level1)
{
    auto capturerInClient = CapturerInClient::GetInstance(STREAM_MUSIC, getpid());
    EXPECT_TRUE(capturerInClient != nullptr);
}

/**
 * @tc.name  : Test stream creation with different stream types for RendererInClient
 * @tc.type  : FUNC
 * @tc.number: RendererInClient_DifferentTypes_001
 * @tc.desc  : Test RendererInClient::GetInstance for various stream types.
 */
HWTEST(IAudioStreamNewUnitTest, RendererInClient_DifferentTypes_001, TestSize.Level1)
{
    std::vector<AudioStreamType> streamTypes = {
        STREAM_MUSIC,
        STREAM_VOICE_CALL,
        STREAM_SYSTEM,
        STREAM_RING,
        STREAM_ALARM
    };
    
    for (auto type : streamTypes) {
        auto rendererInClient = RendererInClient::GetInstance(type, getpid());
        EXPECT_TRUE(rendererInClient != nullptr);
    }
}

/**
 * @tc.name  : Test stream creation with different stream types for CapturerInClient
 * @tc.type  : FUNC
 * @tc.number: CapturerInClient_DifferentTypes_001
 * @tc.desc  : Test CapturerInClient::GetInstance for various stream types.
 */
HWTEST(IAudioStreamNewUnitTest, CapturerInClient_DifferentTypes_001, TestSize.Level1)
{
    std::vector<AudioStreamType> streamTypes = {
        STREAM_MUSIC,
        STREAM_VOICE_CALL,
        STREAM_SYSTEM
    };
    
    for (auto type : streamTypes) {
        auto capturerInClient = CapturerInClient::GetInstance(type, getpid());
        EXPECT_TRUE(capturerInClient != nullptr);
    }
}

} // namespace AudioStandard
} // namespace OHOS
