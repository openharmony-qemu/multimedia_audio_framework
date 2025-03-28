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

#include "audio_service_log.h"
#include "audio_errors.h"
#include "fast_audio_stream.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class FastSystemStreamUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

class AudioClientTrackerTest : public AudioClientTracker {
public:
    virtual ~AudioClientTrackerTest() = default;
    /**
     * Mute Stream was controlled by system application
     *
     * @param streamSetStateEventInternal Contains the set even information.
     */
    virtual void MuteStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) {};
     /**
     * Unmute Stream was controlled by system application
     *
     * @param streamSetStateEventInternal Contains the set even information.
     */
    virtual void UnmuteStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) {};
    /**
     * Paused Stream was controlled by system application
     *
     * @param streamSetStateEventInternal Contains the set even information.
     */
    virtual void PausedStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) {};
     /**
     * Resumed Stream was controlled by system application
     *
     * @param streamSetStateEventInternal Contains the set even information.
     */
    virtual void ResumeStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) {};
    virtual void SetLowPowerVolumeImpl(float volume) {};
    virtual void GetLowPowerVolumeImpl(float &volume) {};
    virtual void GetSingleStreamVolumeImpl(float &volume) {};
    virtual void SetOffloadModeImpl(int32_t state, bool isAppBack) {};
    virtual void UnsetOffloadModeImpl() {};
};

/**
 * @tc.name  : Test GetVolume API
 * @tc.type  : FUNC
 * @tc.number: GetVolume_001
 * @tc.desc  : Test GetVolume interface.
 */
HWTEST(FastSystemStreamUnitTest, GetVolume_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);

    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetVolume_001 start");
    fastAudioStream->silentModeAndMixWithOthers_ = true;
    float result = fastAudioStream->GetVolume();
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetVolume_001 result:%{public}f", result);
    EXPECT_GT(result, 0);
}

/**
 * @tc.name  : Test SetVolume API
 * @tc.type  : FUNC
 * @tc.number: SetVolume_001
 * @tc.desc  : Test SetVolume interface.
 */
HWTEST(FastSystemStreamUnitTest, SetVolume_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);

    float volume = 0.5f;
    fastAudioStream->silentModeAndMixWithOthers_ = true;
    int32_t result = fastAudioStream->SetVolume(volume);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetVolume_001 result:%{public}d", result);
    EXPECT_NE(result, ERROR);
}

/**
 * @tc.name  : Test SetSilentModeAndMixWithOthers API
 * @tc.type  : FUNC
 * @tc.number: SetSilentModeAndMixWithOthers_001
 * @tc.desc  : Test SetSilentModeAndMixWithOthers interface.
 */
HWTEST(FastSystemStreamUnitTest, SetSilentModeAndMixWithOthers_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetSilentModeAndMixWithOthers_001 start");
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);

    bool on = false;
    fastAudioStream->silentModeAndMixWithOthers_ = false;
    fastAudioStream->SetSilentModeAndMixWithOthers(on);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetSilentModeAndMixWithOthers_001 -1");
    fastAudioStream->silentModeAndMixWithOthers_ = true;
    fastAudioStream->SetSilentModeAndMixWithOthers(on);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetSilentModeAndMixWithOthers_001 -2");

    on = true;
    fastAudioStream->silentModeAndMixWithOthers_ = false;
    fastAudioStream->SetSilentModeAndMixWithOthers(on);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetSilentModeAndMixWithOthers_001 -3");
    fastAudioStream->silentModeAndMixWithOthers_ = true;
    fastAudioStream->SetSilentModeAndMixWithOthers(on);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetSilentModeAndMixWithOthers_001 -4");
}

/**
 * @tc.name  : Test GetSwitchInfo API
 * @tc.type  : FUNC
 * @tc.number: GetSwitchInfo_001
 * @tc.desc  : Test GetSwitchInfo interface.
 */
HWTEST(FastSystemStreamUnitTest, GetSwitchInfo_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetSwitchInfo_001 start");
    int32_t appUid = static_cast<int32_t>(getuid());
    IAudioStream::SwitchInfo info;
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    fastAudioStream->GetSwitchInfo(info);
}

/**
 * @tc.name  : Test UpdatePlaybackCaptureConfig API
 * @tc.type  : FUNC
 * @tc.number: UpdatePlaybackCaptureConfig_001
 * @tc.desc  : Test UpdatePlaybackCaptureConfig interface.
 */
HWTEST(FastSystemStreamUnitTest, UpdatePlaybackCaptureConfig_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest UpdatePlaybackCaptureConfig_001 start");
    int32_t appUid = static_cast<int32_t>(getuid());
    AudioPlaybackCaptureConfig config;
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    int32_t res = 0;
    res = fastAudioStream->UpdatePlaybackCaptureConfig(config);
    EXPECT_EQ(res, ERR_NOT_SUPPORTED);
}

/**
 * @tc.name  : Test GetAudioPipeType and SetAudioStreamType API
 * @tc.type  : FUNC
 * @tc.number: GetAudioPipeType_001
 * @tc.desc  : Test GetAudioPipeType and SetAudioStreamType interface.
 */
HWTEST(FastSystemStreamUnitTest, GetAudioPipeType_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetAudioPipeType_001 start");
    int32_t appUid = static_cast<int32_t>(getuid());
    AudioPipeType pipeType;
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    fastAudioStream->GetAudioPipeType(pipeType);
    AudioStreamType audioStreamType = STREAM_DEFAULT;
    int32_t res = fastAudioStream->SetAudioStreamType(audioStreamType);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/**
 * @tc.name  : Test SetMute API
 * @tc.type  : FUNC
 * @tc.number: SetMute_001
 * @tc.desc  : Test SetMute interface.
 */
HWTEST(FastSystemStreamUnitTest, SetMute_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetMute_001 start");
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    int32_t res = fastAudioStream->SetMute(false);
    EXPECT_EQ(res, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test SetRenderMode and GetCaptureMode API
 * @tc.type  : FUNC
 * @tc.number: SetRenderMode_001
 * @tc.desc  : Test SetRenderMode and GetCaptureMode interface.
 */
HWTEST(FastSystemStreamUnitTest, SetRenderMode_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetRenderMode_001 start");
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    AudioRenderMode renderMode = RENDER_MODE_CALLBACK;
    int32_t res = fastAudioStream->SetRenderMode(renderMode);
    EXPECT_EQ(res, SUCCESS);
}

/**
 * @tc.name  : Test GetCaptureMode API
 * @tc.type  : FUNC
 * @tc.number: GetCaptureMode_001
 * @tc.desc  : Test GetCaptureMode interface.
 */
HWTEST(FastSystemStreamUnitTest, GetCaptureMode_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetCaptureMode_001 start");
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    AudioCaptureMode captureMode;
    captureMode = fastAudioStream->GetCaptureMode();
    EXPECT_EQ(captureMode, CAPTURE_MODE_CALLBACK);
}

/**
 * @tc.name  : Test SetLowPowerVolume, GetLowPowerVolume and GetSingleStreamVolume API
 * @tc.type  : FUNC
 * @tc.number: SetLowPowerVolume_001
 * @tc.desc  : Test SetLowPowerVolume, GetLowPowerVolume and GetSingleStreamVolume interface.
 */
HWTEST(FastSystemStreamUnitTest, SetLowPowerVolume_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetLowPowerVolume_001 start");
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    int32_t res = fastAudioStream->SetLowPowerVolume(1.0f);
    EXPECT_EQ(res, SUCCESS);
    float volume = fastAudioStream->GetLowPowerVolume();
    EXPECT_EQ(volume, 1.0f);
    volume = fastAudioStream->GetSingleStreamVolume();
    EXPECT_EQ(volume, 1.0f);
}

/**
 * @tc.name  : Test SetOffloadMode and UnsetOffloadMode API
 * @tc.type  : FUNC
 * @tc.number: SetOffloadMode_001
 * @tc.desc  : Test SetOffloadMode and UnsetOffloadMode interface.
 */
HWTEST(FastSystemStreamUnitTest, SetOffloadMode_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetOffloadMode_001 start");
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    int32_t res = fastAudioStream->SetOffloadMode(0, true);
    EXPECT_EQ(res, ERR_NOT_SUPPORTED);
    res = fastAudioStream->UnsetOffloadMode();
    EXPECT_EQ(res, ERR_NOT_SUPPORTED);
}

/**
 * @tc.name  : Test SetAudioEffectMode API
 * @tc.type  : FUNC
 * @tc.number: SetAudioEffectMode_001
 * @tc.desc  : Test SetAudioEffectMode interface.
 */
HWTEST(FastSystemStreamUnitTest, SetAudioEffectMode_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetAudioEffectMode_001 start");
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    AudioEffectMode effectMode = EFFECT_NONE;
    int32_t res = fastAudioStream->SetAudioEffectMode(effectMode);
    EXPECT_EQ(res, ERR_NOT_SUPPORTED);
}

/**
 * @tc.name  : Test GetFramesWritten and GetFramesRead API
 * @tc.type  : FUNC
 * @tc.number: GetFramesWritten_001
 * @tc.desc  : Test GetFramesWritten and GetFramesRead interface.
 */
HWTEST(FastSystemStreamUnitTest, GetFramesWritten_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetFramesWritten_001 start");
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    int32_t res = fastAudioStream->GetFramesWritten();
    EXPECT_EQ(res, -1);
    res = fastAudioStream->GetFramesRead();
    EXPECT_EQ(res, -1);
}

/**
 * @tc.name  : Test SetSpeed and GetSpeed API
 * @tc.type  : FUNC
 * @tc.number: SetSpeed_001
 * @tc.desc  : Test SetSpeed and GetSpeed interface.
 */
HWTEST(FastSystemStreamUnitTest, SetSpeed_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetSpeed_001 start");
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    int32_t res = fastAudioStream->SetSpeed(1.0f);
    EXPECT_EQ(res, ERR_OPERATION_FAILED);
    float ret = fastAudioStream->GetSpeed();
    EXPECT_EQ(ret, static_cast<float>(ERROR));
}

/**
 * @tc.name  : Test FlushAudioStream and DrainAudioStream API
 * @tc.type  : FUNC
 * @tc.number: SetSpeed_001
 * @tc.desc  : Test FlushAudioStream and DrainAudioStream interface.
 */
HWTEST(FastSystemStreamUnitTest, FlushAudioStream_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest FlushAudioStream_001 start");
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    bool res = fastAudioStream->FlushAudioStream();
    EXPECT_EQ(res, true);
    res = fastAudioStream->DrainAudioStream(true);
    EXPECT_EQ(res, true);
}

/**
 * @tc.name  : Test callbacks and samplingrate API
 * @tc.type  : FUNC
 * @tc.number: SetAndGetCallback_001
 * @tc.desc  : Test callbacks and samplingrate interface.
 */
HWTEST(FastSystemStreamUnitTest, SetAndGetCallback_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetAndGetCallback_001 start");
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);

    std::shared_ptr<RendererPeriodPositionCallback> rendererPeriodPositionCallback = nullptr;
    fastAudioStream->SetRendererPeriodPositionCallback(0, rendererPeriodPositionCallback);
    fastAudioStream->UnsetRendererPeriodPositionCallback();

    std::shared_ptr<CapturerPositionCallback> capturerPositionCallback = nullptr;
    fastAudioStream->SetCapturerPositionCallback(0, capturerPositionCallback);
    fastAudioStream->UnsetCapturerPositionCallback();

    std::shared_ptr<CapturerPeriodPositionCallback> capturerPeriodPositionCallback = nullptr;
    fastAudioStream->SetCapturerPeriodPositionCallback(0, capturerPeriodPositionCallback);
    fastAudioStream->UnsetCapturerPeriodPositionCallback();

    int32_t res = fastAudioStream->SetRendererSamplingRate(0);
    EXPECT_EQ(res, ERR_OPERATION_FAILED);

    uint32_t samplingRate = fastAudioStream->streamInfo_.samplingRate;
    uint32_t rate = fastAudioStream->GetRendererSamplingRate();
    EXPECT_EQ(rate, samplingRate);
}

/**
 * @tc.name  : Test SetAudioStreamInfo API
 * @tc.type  : FUNC
 * @tc.number: SetAudioStreamInfo_001
 * @tc.desc  : Test SetAudioStreamInfo interface.
 */
HWTEST(FastSystemStreamUnitTest, SetAudioStreamInfo_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetAudioStreamInfo_001 start");
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    std::shared_ptr<AudioClientTracker> proxyObj;
    fastAudioStream->state_ = PREPARED;
    AudioStreamParams info;
    int32_t res = fastAudioStream->SetAudioStreamInfo(info, proxyObj);
    EXPECT_EQ(res, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test SetAudioStreamInfo API
 * @tc.type  : FUNC
 * @tc.number: SetAudioStreamInfo_002
 * @tc.desc  : Test SetAudioStreamInfo interface.
 */
HWTEST(FastSystemStreamUnitTest, SetAudioStreamInfo_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetAudioStreamInfo_002 start");
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    std::shared_ptr<AudioClientTracker> proxyObj;
    AudioStreamParams info;
    info.format = AudioSampleFormat::SAMPLE_S16LE;
    info.encoding = AudioEncodingType::ENCODING_PCM;
    info.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    info.channels = AudioChannel::MONO;
    info.channelLayout = AudioChannelLayout::CH_LAYOUT_MONO;
    int32_t res = fastAudioStream->SetAudioStreamInfo(info, proxyObj);
    EXPECT_NE(res, SUCCESS);
    bool result = false;
    fastAudioStream->RestoreAudioStream(true);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test RestoreAudioStream API
 * @tc.type  : FUNC
 * @tc.number: RestoreAudioStream_001
 * @tc.desc  : Test RestoreAudioStream interface.
 */
HWTEST(FastSystemStreamUnitTest, RestoreAudioStream_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest RestoreAudioStream_001 start");
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    bool result = false;
    AudioStreamParams info;
    info.format = AudioSampleFormat::SAMPLE_S16LE;
    info.encoding = AudioEncodingType::ENCODING_PCM;
    info.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    info.channels = AudioChannel::MONO;
    info.channelLayout = AudioChannelLayout::CH_LAYOUT_MONO;
    std::shared_ptr<AudioClientTracker> proxyObj = std::make_shared<AudioClientTrackerTest>();
    fastAudioStream->proxyObj_ = proxyObj;
    fastAudioStream->streamInfo_ = info;
    fastAudioStream->state_ = RUNNING;
    result = fastAudioStream->SetAudioStreamInfo(info, proxyObj);
    EXPECT_EQ(result, true);
    fastAudioStream->state_ = PAUSED;
    result = fastAudioStream->SetAudioStreamInfo(info, proxyObj);
    EXPECT_EQ(result, true);
    fastAudioStream->state_ = STOPPED;
    result = fastAudioStream->SetAudioStreamInfo(info, proxyObj);
    EXPECT_EQ(result, true);
    fastAudioStream->state_ = STOPPING;
    result = fastAudioStream->SetAudioStreamInfo(info, proxyObj);
    EXPECT_EQ(result, true);
    fastAudioStream->state_ = INVALID;
    result = fastAudioStream->SetAudioStreamInfo(info, proxyObj);
    EXPECT_EQ(result, true);
}
} // namespace AudioStandard
} // namespace OHOS