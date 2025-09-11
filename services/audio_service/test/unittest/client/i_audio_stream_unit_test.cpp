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
} // namespace AudioStandard
} // namespace OHOS
