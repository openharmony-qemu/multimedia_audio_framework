/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "audio_renderer_unit_test.h"

#include <chrono>
#include <thread>

#include "audio_errors.h"
#include "audio_info.h"
#include "audio_renderer_proxy_obj.h"
#include "audio_policy_manager.h"
#include "audio_renderer_private.h"
#include "fast_audio_stream.h"
#include "audio_stream_enum.h"

using namespace std;
using namespace std::chrono;
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {

/**
 * @tc.name  : Test SetDefaultOutputDevice API
 * @tc.number: Audio_Renderer_SetDefaultOutputDevice_001
 * @tc.desc  : Test SetDefaultOutputDevice interface. Returns true, if check Unmute is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetDefaultOutputDevice_001, TestSize.Level0)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MEDIA);
    EXPECT_NE(nullptr, audioRenderer);
    bool result = audioRenderer->SetDefaultOutputDevice(DEVICE_TYPE_INVALID);
    EXPECT_TRUE(result);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetParams API via legal input
 * @tc.number: Audio_Renderer_SetParams_001
 * @tc.desc  : Test SetParams interface. Returns 0 {SUCCESS}, if the setting is successful.
 *             rendererParams.sampleFormat = SAMPLE_S16LE;
 *             rendererParams.sampleRate = SAMPLE_RATE_44100;
 *             rendererParams.channelCount = STEREO;
 *             rendererParams.encodingType = ENCODING_PCM;
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetParams_001, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    AudioRendererParams rendererParams;
    rendererParams.sampleFormat = SAMPLE_S16LE;
    rendererParams.sampleRate = SAMPLE_RATE_44100;
    rendererParams.channelCount = STEREO;
    rendererParams.encodingType = ENCODING_PCM;

    int32_t ret = audioRenderer->SetParams(rendererParams);
    EXPECT_EQ(SUCCESS, ret);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetParams API via legal input.
 * @tc.number: Audio_Renderer_SetParams_002
 * @tc.desc  : Test SetParams interface. Returns 0 {SUCCESS}, if the setting is successful.
 *             rendererParams.sampleFormat = SAMPLE_S16LE;
 *             rendererParams.sampleRate = SAMPLE_RATE_8000;
 *             rendererParams.channelCount = MONO;
 *             rendererParams.encodingType = ENCODING_PCM;
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetParams_002, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    AudioRendererParams rendererParams;
    rendererParams.sampleFormat = SAMPLE_S16LE;
    rendererParams.sampleRate = SAMPLE_RATE_8000;
    rendererParams.channelCount = MONO;
    rendererParams.encodingType = ENCODING_PCM;

    int32_t ret = audioRenderer->SetParams(rendererParams);
    EXPECT_EQ(SUCCESS, ret);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetParams API via legal input.
 * @tc.number: Audio_Renderer_SetParams_003
 * @tc.desc  : Test SetParams interface. Returns 0 {SUCCESS}, if the setting is successful.
 *             rendererParams.sampleFormat = SAMPLE_S16LE;
 *             rendererParams.sampleRate = SAMPLE_RATE_11025;
 *             rendererParams.channelCount = STEREO;
 *             rendererParams.encodingType = ENCODING_PCM;
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetParams_003, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    AudioRendererParams rendererParams;
    rendererParams.sampleFormat = SAMPLE_S16LE;
    rendererParams.sampleRate = SAMPLE_RATE_11025;
    rendererParams.channelCount = STEREO;
    rendererParams.encodingType = ENCODING_PCM;

    int32_t ret = audioRenderer->SetParams(rendererParams);
    EXPECT_EQ(SUCCESS, ret);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetParams API via legal input.
 * @tc.number: Audio_Renderer_SetParams_004
 * @tc.desc  : Test SetParams interface. Returns 0 {SUCCESS}, if the setting is successful.
 *             rendererParams.sampleFormat = SAMPLE_S16LE;
 *             rendererParams.sampleRate = SAMPLE_RATE_22050;
 *             rendererParams.channelCount = MONO;
 *             rendererParams.encodingType = ENCODING_PCM;
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetParams_004, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    AudioRendererParams rendererParams;
    rendererParams.sampleFormat = SAMPLE_S16LE;
    rendererParams.sampleRate = SAMPLE_RATE_22050;
    rendererParams.channelCount = MONO;
    rendererParams.encodingType = ENCODING_PCM;

    int32_t ret = audioRenderer->SetParams(rendererParams);
    EXPECT_EQ(SUCCESS, ret);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetParams API via legal input.
 * @tc.number: Audio_Renderer_SetParams_005
 * @tc.desc  : Test SetParams interface. Returns 0 {SUCCESS}, if the setting is successful.
 *             rendererParams.sampleFormat = SAMPLE_S16LE;
 *             rendererParams.sampleRate = SAMPLE_RATE_96000;
 *             rendererParams.channelCount = MONO;
 *             rendererParams.encodingType = ENCODING_PCM;
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetParams_005, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    AudioRendererParams rendererParams;
    rendererParams.sampleFormat = SAMPLE_S16LE;
    rendererParams.sampleRate = SAMPLE_RATE_96000;
    rendererParams.channelCount = MONO;
    rendererParams.encodingType = ENCODING_PCM;

    int32_t ret = audioRenderer->SetParams(rendererParams);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetParams API via legal input.
 * @tc.number: Audio_Renderer_SetParams_006
 * @tc.desc  : Test SetParams interface. Returns 0 {SUCCESS}, if the setting is successful.
 *             rendererParams.sampleFormat = SAMPLE_S24LE;
 *             rendererParams.sampleRate = SAMPLE_RATE_64000;
 *             rendererParams.channelCount = MONO;
 *             rendererParams.encodingType = ENCODING_PCM;
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetParams_006, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    AudioRendererParams rendererParams;
    rendererParams.sampleFormat = SAMPLE_S24LE;
    rendererParams.sampleRate = SAMPLE_RATE_64000;
    rendererParams.channelCount = MONO;
    rendererParams.encodingType = ENCODING_PCM;

    int32_t ret = audioRenderer->SetParams(rendererParams);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetParams API via illegal input.
 * @tc.number: Audio_Renderer_SetParams_007
 * @tc.desc  : Test SetParams interface. Returns 0 {SUCCESS}, if the setting is successful.
 *             rendererParams.sampleFormat = SAMPLE_S16LE;
 *             rendererParams.sampleRate = SAMPLE_RATE_16000;
 *             rendererParams.channelCount = STEREO;
 *             rendererParams.encodingType = ENCODING_PCM;
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetParams_007, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    AudioRendererParams rendererParams;
    rendererParams.sampleFormat = SAMPLE_S16LE;
    rendererParams.sampleRate = SAMPLE_RATE_16000;
    rendererParams.channelCount = STEREO;
    rendererParams.encodingType = ENCODING_PCM;

    int32_t ret = audioRenderer->SetParams(rendererParams);
    EXPECT_EQ(SUCCESS, ret);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetParams API via legal input.
 * @tc.number: Audio_Renderer_SetParams_008
 * @tc.desc  : Test SetParams interface. Returns 0 {SUCCESS}, if the setting is successful.
 *             rendererParams.sampleFormat = SAMPLE_F32LE;
 *             rendererParams.sampleRate = SAMPLE_RATE_44100;
 *             rendererParams.channelCount = STEREO;
 *             rendererParams.encodingType = ENCODING_PCM;
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetParams_008, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    AudioRendererParams rendererParams;
    rendererParams.sampleFormat = SAMPLE_F32LE;
    rendererParams.sampleRate = SAMPLE_RATE_44100;
    rendererParams.channelCount = STEREO;
    rendererParams.encodingType = ENCODING_PCM;

    int32_t ret = audioRenderer->SetParams(rendererParams);
    EXPECT_EQ(SUCCESS, ret);
    audioRenderer->Release();
}

#ifdef TEMP_DISABLE
/**
 * @tc.name  : Test SetParams API stability.
 * @tc.number: Audio_Renderer_SetParams_Stability_001
 * @tc.desc  : Test SetParams interface stability.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetParams_Stability_001, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    AudioRendererParams rendererParams;
    rendererParams.sampleFormat = SAMPLE_S16LE;
    rendererParams.sampleRate = SAMPLE_RATE_44100;
    rendererParams.channelCount = STEREO;
    rendererParams.encodingType = ENCODING_PCM;

    for (int i = 0; i < RenderUT::VALUE_HUNDRED; i++) {
        ret = audioRenderer->SetParams(rendererParams);
        EXPECT_EQ(SUCCESS, ret);

        AudioRendererParams getRendererParams;
        ret = audioRenderer->GetParams(getRendererParams);
        EXPECT_EQ(SUCCESS, ret);
    }

    audioRenderer->Release();
}
#endif

/**
 * @tc.name  : Test SetInterruptMode API via legal input
 * @tc.number: Audio_Renderer_SetInterruptMode_001
 * @tc.desc  : Test SetInterruptMode interface. Returns 0 {SUCCESS}, if the setting is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetInterruptMode_001, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererUnitTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    audioRenderer->SetInterruptMode(SHARE_MODE);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetInterruptMode API via legal input
 * @tc.number: Audio_Renderer_SetInterruptMode_002
 * @tc.desc  : Test SetInterruptMode interface. Returns 0 {SUCCESS}, if the setting is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetInterruptMode_002, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererUnitTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    audioRenderer->SetInterruptMode(INDEPENDENT_MODE);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetAudioRendererDesc API stability.
 * @tc.number: Audio_Renderer_SetAudioRendererDesc_001
 * @tc.desc  : Test SetAudioRendererDesc interface stability.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetAudioRendererDesc_001, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    AudioRendererDesc audioRD = {CONTENT_TYPE_MUSIC, STREAM_USAGE_VOICE_COMMUNICATION};
    ret = audioRenderer->SetAudioRendererDesc(audioRD);
    EXPECT_EQ(SUCCESS, ret);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetStreamType API stability.
 * @tc.number: Audio_Renderer_SetStreamType_001
 * @tc.desc  : Test SetStreamType interface stability.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetStreamType_001, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    AudioStreamType audioStreamType = STREAM_MUSIC;
    ret = audioRenderer->SetStreamType(audioStreamType);
    EXPECT_EQ(SUCCESS, ret);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetVolume
 * @tc.number: Audio_Renderer_SetVolume_001
 * @tc.desc  : Test SetVolume interface, Returns 0 {SUCCESS}, if the track volume is set.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetVolume_001, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    ret = audioRenderer->SetVolume(0.5);
    EXPECT_EQ(SUCCESS, ret);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
 * @tc.name  : Test SetVolume
 * @tc.number: Audio_Renderer_SetVolume_002
 * @tc.desc  : Test SetVolume interface for minimum and maximum volumes.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetVolume_002, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    ret = audioRenderer->SetVolume(0);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioRenderer->SetVolume(1.0);
    EXPECT_EQ(SUCCESS, ret);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
 * @tc.name  : Test SetVolume
 * @tc.number: Audio_Renderer_SetVolume_003
 * @tc.desc  : Test SetVolume interface for out of range values.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetVolume_003, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    ret = audioRenderer->SetVolume(-0.5);
    EXPECT_NE(SUCCESS, ret);

    ret = audioRenderer->SetVolume(1.5);
    EXPECT_NE(SUCCESS, ret);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
 * @tc.name  : Test SetVolume
 * @tc.number: Audio_Renderer_SetVolume_Stability_001
 * @tc.desc  : Test SetVolume interface stability.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetVolume_Stability_001, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    thread renderThread(RenderUT::StartRenderThread, audioRenderer.get(), RenderUT::PLAYBACK_DURATION);

    for (int i = 0; i < RenderUT::VALUE_HUNDRED; i++) {
        audioRenderer->SetVolume(0.1);
        audioRenderer->SetVolume(1.0);
    }

    renderThread.join();

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
 * @tc.name  : Test SetRenderRate
 * @tc.number: Audio_Renderer_SetRenderRate_001
 * @tc.desc  : Test SetRenderRate interface after set volume fails.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetRenderRate_001, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    AudioRendererRate renderRate = RENDER_RATE_NORMAL;
    ret = audioRenderer->SetRenderRate(renderRate);
    EXPECT_EQ(SUCCESS, ret);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetRendererCallback with null pointer.
 * @tc.number: Audio_Renderer_SetRendererCallback_001
 * @tc.desc  : Test SetRendererCallback interface. Returns error code, if null pointer is set.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetRendererCallback_001, TestSize.Level1)
{
    int32_t ret = -1;

    AudioRendererOptions rendererOptions;
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    rendererOptions.streamInfo.channels = AudioChannel::STEREO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    rendererOptions.rendererInfo.rendererFlags = RenderUT::RENDERER_FLAG;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    ret = audioRenderer->SetRendererCallback(nullptr);
    EXPECT_NE(SUCCESS, ret);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}

/**
 * @tc.name  : Test SetRendererCallback with valid callback pointer.
 * @tc.number: Audio_Renderer_SetRendererCallback_002
 * @tc.desc  : Test SetRendererCallback interface. Returns success, if valid callback is set.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetRendererCallback_002, TestSize.Level1)
{
    int32_t ret = -1;

    AudioRendererOptions rendererOptions;
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    rendererOptions.streamInfo.channels = AudioChannel::STEREO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    rendererOptions.rendererInfo.rendererFlags = RenderUT::RENDERER_FLAG;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    shared_ptr<AudioRendererCallbackTest> audioRendererCB = make_shared<AudioRendererCallbackTest>();
    ret = audioRenderer->SetRendererCallback(audioRendererCB);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetRendererCallback via illegal state, RENDERER_RELEASED: After RELEASED
 * @tc.number: Audio_Renderer_SetRendererCallback_003
 * @tc.desc  : Test SetRendererCallback interface. Returns error, if callback is set in released state.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetRendererCallback_003, TestSize.Level1)
{
    int32_t ret = -1;

    AudioRendererOptions rendererOptions;
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    rendererOptions.streamInfo.channels = AudioChannel::STEREO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    rendererOptions.rendererInfo.rendererFlags = RenderUT::RENDERER_FLAG;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);

    RendererState state = audioRenderer->GetStatus();
    EXPECT_EQ(RENDERER_RELEASED, state);

    shared_ptr<AudioRendererCallbackTest> audioRendererCB = make_shared<AudioRendererCallbackTest>();
    ret = audioRenderer->SetRendererCallback(audioRendererCB);
    EXPECT_NE(SUCCESS, ret);
    EXPECT_EQ(ERR_ILLEGAL_STATE, ret);
}

/**
 * @tc.name  : Test SetRendererCallback via legal state, RENDERER_PREPARED: After PREPARED
 * @tc.number: Audio_Renderer_SetRendererCallback_004
 * @tc.desc  : Test SetRendererCallback interface. Returns success, if callback is set in proper state.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetRendererCallback_004, TestSize.Level1)
{
    int32_t ret = -1;

    AudioRendererOptions rendererOptions;
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    rendererOptions.streamInfo.channels = AudioChannel::STEREO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    rendererOptions.rendererInfo.rendererFlags = RenderUT::RENDERER_FLAG;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    RendererState state = audioRenderer->GetStatus();
    EXPECT_EQ(RENDERER_PREPARED, state);

    shared_ptr<AudioRendererCallbackTest> audioRendererCB = make_shared<AudioRendererCallbackTest>();
    ret = audioRenderer->SetRendererCallback(audioRendererCB);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetRenderMode via legal input, RENDER_MODE_CALLBACK
 * @tc.number: Audio_Renderer_SetRenderMode_001
 * @tc.desc  : Test SetRenderMode interface. Returns SUCCESS, if the render mode is successfully set.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetRenderMode_001, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    ret = audioRenderer->SetRenderMode(RENDER_MODE_CALLBACK);
    EXPECT_EQ(SUCCESS, ret);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetRenderMode via legal input, RENDER_MODE_NORMAL
 * @tc.number: Audio_Renderer_SetRenderMode_002
 * @tc.desc  : Test SetRenderMode interface. Returns SUCCESS, if the render mode is successfully set.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetRenderMode_002, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    ret = audioRenderer->SetRenderMode(RENDER_MODE_NORMAL);
    EXPECT_EQ(SUCCESS, ret);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetRendererWriteCallback via legal render mode, RENDER_MODE_CALLBACK
 * @tc.number: Audio_Renderer_SetRendererWriteCallback_001
 * @tc.desc  : Test SetRendererWriteCallback interface. Returns SUCCESS, if the callback is successfully set.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetRendererWriteCallback_001, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    ret = audioRenderer->SetRenderMode(RENDER_MODE_CALLBACK);
    EXPECT_EQ(SUCCESS, ret);
    AudioRenderMode renderMode = audioRenderer->GetRenderMode();
    EXPECT_EQ(RENDER_MODE_CALLBACK, renderMode);

    shared_ptr<AudioRendererWriteCallback> cb = make_shared<AudioRenderModeCallbackTest>();

    ret = audioRenderer->SetRendererWriteCallback(cb);
    EXPECT_EQ(SUCCESS, ret);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetRendererWriteCallback via illegal render mode, RENDER_MODE_NORMAL
 * @tc.number: Audio_Renderer_SetRendererWriteCallback_002
 * @tc.desc  : Test SetRendererWriteCallback interface. Returns error code, if the render mode is not callback.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetRendererWriteCallback_002, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    ret = audioRenderer->SetRenderMode(RENDER_MODE_NORMAL);
    EXPECT_EQ(SUCCESS, ret);
    AudioRenderMode renderMode = audioRenderer->GetRenderMode();
    EXPECT_EQ(RENDER_MODE_NORMAL, renderMode);

    shared_ptr<AudioRendererWriteCallback> cb = make_shared<AudioRenderModeCallbackTest>();

    ret = audioRenderer->SetRendererWriteCallback(cb);
    EXPECT_EQ(ERR_INCORRECT_MODE, ret);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetRendererWriteCallback via illegal render mode, default render mode RENDER_MODE_NORMAL
 * @tc.number: Audio_Renderer_SetRendererWriteCallback_003
 * @tc.desc  : Test SetRendererWriteCallback interface. Returns error code, if the render mode is not callback.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetRendererWriteCallback_003, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    shared_ptr<AudioRendererWriteCallback> cb = make_shared<AudioRenderModeCallbackTest>();

    ret = audioRenderer->SetRendererWriteCallback(cb);
    EXPECT_EQ(ERR_INCORRECT_MODE, ret);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetRendererWriteCallback via illegal input, nullptr
 * @tc.number: Audio_Renderer_SetRendererWriteCallback_004
 * @tc.desc  : Test SetRendererWriteCallback interface. Returns error code, if the callback reference is nullptr.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetRendererWriteCallback_004, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    ret = audioRenderer->SetRenderMode(RENDER_MODE_CALLBACK);
    EXPECT_EQ(SUCCESS, ret);
    AudioRenderMode renderMode = audioRenderer->GetRenderMode();
    EXPECT_EQ(RENDER_MODE_CALLBACK, renderMode);

    ret = audioRenderer->SetRendererWriteCallback(nullptr);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetBufferDuration API
 * @tc.number: Audio_Renderer_SetBufferDuration_001
 * @tc.desc  : Test SetBufferDuration interface. Check whether valid parameters are accepted.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetBufferDuration_001, TestSize.Level1)
{
    int32_t ret = -1;

    AudioRendererOptions rendererOptions;
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_96000;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_U8;
    rendererOptions.streamInfo.channels = AudioChannel::MONO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    rendererOptions.rendererInfo.rendererFlags = RenderUT::RENDERER_FLAG;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    EXPECT_NE(nullptr, audioRenderer);

    ret = audioRenderer->SetBufferDuration(RenderUT::BUFFER_DURATION_FIVE);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioRenderer->SetBufferDuration(RenderUT::BUFFER_DURATION_TEN);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioRenderer->SetBufferDuration(RenderUT::BUFFER_DURATION_FIFTEEN);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioRenderer->SetBufferDuration(RenderUT::BUFFER_DURATION_TWENTY);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetBufferDuration API
 * @tc.number: Audio_Renderer_SetBufferDuration_002
 * @tc.desc  : Test SetBufferDuration interface. Check whether invalid parameters are rejected.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetBufferDuration_002, TestSize.Level1)
{
    int32_t ret = -1;

    AudioRendererOptions rendererOptions;
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_96000;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_U8;
    rendererOptions.streamInfo.channels = AudioChannel::MONO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    rendererOptions.rendererInfo.rendererFlags = RenderUT::RENDERER_FLAG;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    EXPECT_NE(nullptr, audioRenderer);

    ret = audioRenderer->SetBufferDuration(RenderUT::VALUE_NEGATIVE);
    EXPECT_NE(SUCCESS, ret);

    ret = audioRenderer->SetBufferDuration(RenderUT::VALUE_ZERO);
    EXPECT_NE(SUCCESS, ret);

    ret = audioRenderer->SetBufferDuration(RenderUT::VALUE_HUNDRED);
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetRendererPositionCallback API
 * @tc.number: Audio_Renderer_SetRendererPositionCallback_001
 * @tc.desc  : Test SetRendererPositionCallback interface to check set position callback is success for valid callback.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetRendererPositionCallback_001, TestSize.Level1)
{
    int32_t ret = -1;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    shared_ptr<RendererPositionCallbackTest> positionCB = std::make_shared<RendererPositionCallbackTest>();
    ret = audioRenderer->SetRendererPositionCallback(RenderUT::VALUE_THOUSAND, positionCB);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetRendererPositionCallback API
 * @tc.number: Audio_Renderer_SetRendererPositionCallback_002
 * @tc.desc  : Test SetRendererPositionCallback interface again after unregister.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetRendererPositionCallback_002, TestSize.Level1)
{
    int32_t ret = -1;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    shared_ptr<RendererPositionCallbackTest> positionCB1 = std::make_shared<RendererPositionCallbackTest>();
    ret = audioRenderer->SetRendererPositionCallback(RenderUT::VALUE_THOUSAND, positionCB1);
    EXPECT_EQ(SUCCESS, ret);

    audioRenderer->UnsetRendererPositionCallback();

    shared_ptr<RendererPositionCallbackTest> positionCB2 = std::make_shared<RendererPositionCallbackTest>();
    ret = audioRenderer->SetRendererPositionCallback(RenderUT::VALUE_THOUSAND, positionCB2);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetRendererPositionCallback API
 * @tc.number: Audio_Renderer_SetRendererPositionCallback_003
 * @tc.desc  : Test SetRendererPositionCallback interface with null callback.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetRendererPositionCallback_003, TestSize.Level1)
{
    int32_t ret = -1;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = audioRenderer->SetRendererPositionCallback(RenderUT::VALUE_THOUSAND, nullptr);
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetRendererPositionCallback API
 * @tc.number: Audio_Renderer_SetRendererPositionCallback_004
 * @tc.desc  : Test SetRendererPositionCallback interface with invalid parameter.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetRendererPositionCallback_004, TestSize.Level1)
{
    int32_t ret = -1;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    shared_ptr<RendererPositionCallbackTest> positionCB = std::make_shared<RendererPositionCallbackTest>();
    ret = audioRenderer->SetRendererPositionCallback(RenderUT::VALUE_ZERO, positionCB);
    EXPECT_NE(SUCCESS, ret);

    ret = audioRenderer->SetRendererPositionCallback(RenderUT::VALUE_NEGATIVE, positionCB);
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetRendererPeriodPositionCallback API
 * @tc.number: Audio_Renderer_SetRendererPeriodPositionCallback_001
 * @tc.desc  : Test SetRendererPeriodPositionCallback interface to check set period position
 *             callback is success for valid callback.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetRendererPeriodPositionCallback_001, TestSize.Level1)
{
    int32_t ret = -1;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    shared_ptr<RendererPeriodPositionCallbackTest> positionCB = std::make_shared<RendererPeriodPositionCallbackTest>();
    ret = audioRenderer->SetRendererPeriodPositionCallback(RenderUT::VALUE_THOUSAND, positionCB);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetRendererPeriodPositionCallback API
 * @tc.number: Audio_Renderer_SetRendererPeriodPositionCallback_002
 * @tc.desc  : Test SetRendererPeriodPositionCallback interface again after unregister.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetRendererPeriodPositionCallback_002, TestSize.Level1)
{
    int32_t ret = -1;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    shared_ptr<RendererPeriodPositionCallbackTest> positionCB1 = std::make_shared<RendererPeriodPositionCallbackTest>();
    ret = audioRenderer->SetRendererPeriodPositionCallback(RenderUT::VALUE_THOUSAND, positionCB1);
    EXPECT_EQ(SUCCESS, ret);

    audioRenderer->UnsetRendererPeriodPositionCallback();

    shared_ptr<RendererPeriodPositionCallbackTest> positionCB2 = std::make_shared<RendererPeriodPositionCallbackTest>();
    ret = audioRenderer->SetRendererPeriodPositionCallback(RenderUT::VALUE_THOUSAND, positionCB2);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetRendererPeriodPositionCallback API
 * @tc.number: Audio_Renderer_SetRendererPeriodPositionCallback_003
 * @tc.desc  : Test SetRendererPeriodPositionCallback interface with null callback.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetRendererPeriodPositionCallback_003, TestSize.Level1)
{
    int32_t ret = -1;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = audioRenderer->SetRendererPeriodPositionCallback(RenderUT::VALUE_THOUSAND, nullptr);
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetRendererPeriodPositionCallback API
 * @tc.number: Audio_Renderer_SetRendererPeriodPositionCallback_004
 * @tc.desc  : Test SetRendererPeriodPositionCallback interface with invalid parameter.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetRendererPeriodPositionCallback_004, TestSize.Level1)
{
    int32_t ret = -1;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    shared_ptr<RendererPeriodPositionCallbackTest> positionCB =
        std::make_shared<RendererPeriodPositionCallbackTest>();
    ret = audioRenderer->SetRendererPeriodPositionCallback(RenderUT::VALUE_ZERO, positionCB);
    EXPECT_NE(SUCCESS, ret);

    ret = audioRenderer->SetRendererPeriodPositionCallback(RenderUT::VALUE_NEGATIVE, positionCB);
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test set renderer samplingrate.
 * @tc.number: Audio_Renderer_Set_Renderer_SamplingRate_001
 * @tc.desc  : Test SetRendererSamplingRate and GetRendererSamplingRate.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Set_Renderer_SamplingRate_001, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    rendererOptions.streamInfo.channels = AudioChannel::MONO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_SONIFICATION;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_VOICE_ASSISTANT;
    rendererOptions.rendererInfo.rendererFlags = RenderUT::RENDERER_FLAG;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    uint32_t sampleRate = AudioSamplingRate::SAMPLE_RATE_48000;
    ret = audioRenderer->SetRendererSamplingRate(sampleRate);
    EXPECT_EQ(ERROR, ret);

    uint32_t sampleRateRet = audioRenderer->GetRendererSamplingRate();
    EXPECT_EQ(AudioSamplingRate::SAMPLE_RATE_44100, sampleRateRet);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test set renderer instance.
 * @tc.number: Audio_Renderer_Set_Renderer_Instance_001
 * @tc.desc  : Test renderer instance GetMinStreamVolume,GetMaxStreamVolume,GetCurrentOutputDevices,GetUnderflowCount
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Set_Renderer_Instance_001, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    rendererOptions.streamInfo.channels = AudioChannel::MONO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_SONIFICATION;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_VOICE_ASSISTANT;
    rendererOptions.rendererInfo.rendererFlags = RenderUT::RENDERER_FLAG;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    float minVolume = audioRenderer->GetMinStreamVolume();
    float maxVolume = audioRenderer->GetMaxStreamVolume();
    EXPECT_LT(minVolume, maxVolume);

    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    ret = audioRenderer->GetCurrentOutputDevices(deviceInfo);
    EXPECT_EQ(SUCCESS, ret);

    float count = audioRenderer->GetUnderflowCount();
    EXPECT_GE(count, 0);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test set renderer instance.
 * @tc.number: Audio_Renderer_Set_Renderer_Instance_003
 * @tc.desc  : Test renderer instance RegisterAudioRendererEventListener,DestroyAudioRendererStateCallback
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Set_Renderer_Instance_003, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    rendererOptions.streamInfo.channels = AudioChannel::STEREO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_SONIFICATION;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_VOICE_ASSISTANT;
    rendererOptions.rendererInfo.rendererFlags = RenderUT::RENDERER_FLAG;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    int32_t clientPid = getpid();
    const std::shared_ptr<AudioRendererPolicyServiceDiedCallback> serviceCallback =
        std::make_shared<AudioRendererPolicyServiceDiedCallbackTest>();
    ret = audioRenderer->RegisterAudioPolicyServerDiedCb(clientPid, serviceCallback);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioRenderer->RegisterAudioPolicyServerDiedCb(clientPid, nullptr);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);

    ret = audioRenderer->UnregisterAudioPolicyServerDiedCb(clientPid);
    EXPECT_EQ(SUCCESS, ret);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test set renderer instance.
 * @tc.number: Audio_Renderer_Set_Renderer_Instance_005
 * @tc.desc  : Test ResumeStreamImpl and PausedStreamImpl on AudioRendererProxyObj
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Set_Renderer_Instance_005, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    unique_ptr<AudioRendererProxyObj> audioRendererProxyObj = std::make_unique<AudioRendererProxyObj>();

    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);
    std::shared_ptr<AudioRenderer> sharedRenderer = std::move(audioRenderer);
    std::weak_ptr<AudioRenderer> weakRenderer = sharedRenderer;

    audioRendererProxyObj->SaveRendererObj(weakRenderer);
    const StreamSetStateEventInternal streamSetStateEventInternal = {};
    audioRendererProxyObj->ResumeStreamImpl(streamSetStateEventInternal);
    audioRendererProxyObj->PausedStreamImpl(streamSetStateEventInternal);
    ASSERT_NE(nullptr, audioRendererPrivate);
}

/**
 * @tc.name  : Test set renderer instance.
 * @tc.number: Audio_Renderer_Set_Renderer_Instance_006
 * @tc.desc  : Test ResumeStreamImpl and PausedStreamImpl on AudioRendererProxyObj when rederer is nullptr
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Set_Renderer_Instance_006, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);

    unique_ptr<AudioRendererProxyObj> audioRendererProxyObj = std::make_unique<AudioRendererProxyObj>();

    audioRendererProxyObj->SaveRendererObj(std::weak_ptr<AudioRendererPrivate>());
    const StreamSetStateEventInternal streamSetStateEventInternal = {};
    audioRendererProxyObj->ResumeStreamImpl(streamSetStateEventInternal);
    audioRendererProxyObj->PausedStreamImpl(streamSetStateEventInternal);
    ASSERT_NE(nullptr, audioRendererPrivate);
}

/**
 * @tc.name  : Test SetAudioEffectMode via legal input, EFFECT_NONE
 * @tc.number: Audio_Renderer_SetAudioEffectMode_001
 * @tc.desc  : Test SetAudioEffectMode interface. Returns SUCCESS, if the effect mode is successfully set.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetAudioEffectMode_001, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    ret = audioRenderer->SetAudioEffectMode(EFFECT_NONE);
    EXPECT_EQ(SUCCESS, ret);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetAudioEffectMode via legal input, EFFECT_DEFAULT
 * @tc.number: Audio_Renderer_SetAudioEffectMode_002
 * @tc.desc  : Test SetAudioEffectMode interface. Returns SUCCESS, if the effect mode is successfully set.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetAudioEffectMode_002, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    ret = audioRenderer->SetAudioEffectMode(EFFECT_DEFAULT);
    EXPECT_EQ(SUCCESS, ret);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetRendererSamplingRate
 * @tc.number: Audio_Renderer_SetRendererSamplingRate_001
 * @tc.desc  : Test SetRendererSamplingRate interface for valid samplingRate.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetRendererSamplingRate_001, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    uint32_t samplingRate = 44100;
    ret = audioRenderer->SetRendererSamplingRate(samplingRate);
    EXPECT_EQ(ERROR, ret);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetRendererSamplingRate
 * @tc.number: Audio_Renderer_SetRendererSamplingRate_002
 * @tc.desc  : Test SetRendererSamplingRate interface for invalid samplingRate.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetRendererSamplingRate_002, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    uint32_t invalidRate_1 = 0;
    ret = audioRenderer->SetRendererSamplingRate(invalidRate_1);
    EXPECT_EQ(ERROR, ret);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetSpeed
 * @tc.number: Audio_Renderer_SetSpeed_001
 * @tc.desc  : Test SetSpeed interface.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetSpeed_001, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    ret = audioRenderer->SetSpeed(0.5);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioRenderer->SetSpeed(0.25); // 0.25 min speed
    EXPECT_EQ(SUCCESS, ret);

    ret = audioRenderer->SetSpeed(4); // 4 max speed
    EXPECT_EQ(SUCCESS, ret);

    ret = audioRenderer->SetSpeed(0.124); // 0.124 lower
    EXPECT_EQ(ERR_INVALID_PARAM, ret);

    ret = audioRenderer->SetSpeed(4.01); // 4.01 upper
    EXPECT_EQ(ERR_INVALID_PARAM, ret);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
 * @tc.name  : Test SetSpeed and Write API.
 * @tc.number: Audio_Renderer_SetSpeed_Write_001
 * @tc.desc  : Test SetSpeed and Write interface.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetSpeed_Write_001, TestSize.Level1)
{
    int32_t ret = -1;
    FILE *wavFile = fopen(RenderUT::AUDIORENDER_TEST_FILE_PATH.c_str(), "rb");
    ASSERT_NE(nullptr, wavFile);

    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    ret = audioRenderer->SetSpeed(1.0); // 1.0 speed
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    size_t bufferLen;
    ret = audioRenderer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);

    size_t bytesToWrite = 0;
    int32_t bytesWritten = 0;
    size_t minBytes = 4; // 4 min bytes
    int32_t numBuffersToRender = RenderUT::WRITE_BUFFERS_COUNT;

    while (numBuffersToRender) {
        if (numBuffersToRender == RenderUT::WRITE_BUFFERS_COUNT / 2) { // 2 half count
            ret = audioRenderer->SetSpeed(2.0); // 2.0 speed
            EXPECT_EQ(SUCCESS, ret);
        }
        bytesToWrite = fread(buffer, 1, bufferLen, wavFile);
        bytesWritten = 0;
        while ((static_cast<size_t>(bytesWritten) < bytesToWrite) &&
            ((static_cast<size_t>(bytesToWrite) - bytesWritten) > minBytes)) {
            bytesWritten += audioRenderer->Write(buffer + static_cast<size_t>(bytesWritten),
                                                 bytesToWrite - static_cast<size_t>(bytesWritten));
            EXPECT_GE(bytesWritten, RenderUT::VALUE_ZERO);
            if (bytesWritten < 0) {
                break;
            }
        }
        numBuffersToRender--;
    }

    audioRenderer->Drain();
    audioRenderer->Stop();
    audioRenderer->Release();

    free(buffer);
    fclose(wavFile);
}

/**
 * @tc.name  : Test SetSpeed and Write with meta API.
 * @tc.number: Audio_Renderer_SetSpeed_Write_002
 * @tc.desc  : Test SetSpeed and Write with meta interface.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetSpeed_Write_002, TestSize.Level1)
{
    int32_t ret = -1;
    FILE *wavFile = fopen(RenderUT::AUDIORENDER_TEST_PCMFILE_PATH.c_str(), "rb");
    FILE *metaFile = fopen(RenderUT::AUDIORENDER_TEST_METAFILE_PATH.c_str(), "rb");
    ASSERT_NE(nullptr, wavFile);
    ASSERT_NE(nullptr, metaFile);

    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererSpatialOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    ret = audioRenderer->SetSpeed(1.0); // 1.0 speed
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    size_t bufferLen;
    uint8_t *buffer = nullptr;
    uint8_t *metaBuffer = nullptr;
    AudioRendererUnitTest::GetBuffersAndLen(audioRenderer, buffer, metaBuffer, bufferLen);

    size_t bytesToWrite = 0;
    int32_t bytesWritten = 0;
    size_t minBytes = 4; // 4 min bytes
    int32_t numBuffersToRender = RenderUT::WRITE_BUFFERS_COUNT;

    while (numBuffersToRender) {
        if (numBuffersToRender == RenderUT::WRITE_BUFFERS_COUNT / 2) { // 2 half count
            ret = audioRenderer->SetSpeed(2.0);              // 2.0 speed
            EXPECT_EQ(SUCCESS, ret);
        }
        bytesToWrite = fread(buffer, 1, bufferLen, wavFile);
        fread(metaBuffer, 1, RenderUT::AVS3METADATA_SIZE, metaFile);
        bytesWritten = 0;
        while ((static_cast<size_t>(bytesWritten) < bytesToWrite) &&
            ((static_cast<size_t>(bytesToWrite) - bytesWritten) > minBytes)) {
            bytesWritten += audioRenderer->Write(buffer + static_cast<size_t>(bytesWritten),
                bytesToWrite - static_cast<size_t>(bytesWritten), metaBuffer, RenderUT::AVS3METADATA_SIZE);
            EXPECT_GE(bytesWritten, RenderUT::VALUE_ZERO);
        }
        numBuffersToRender--;
    }

    audioRenderer->Drain();
    audioRenderer->Stop();
    audioRenderer->Release();
    AudioRendererUnitTest::ReleaseBufferAndFiles(buffer, metaBuffer, wavFile, metaFile);
}

/**
 * @tc.name  : Test SetOffloadAllowed API.
 * @tc.number: Audio_Renderer_SetOffloadAllowed_001
 * @tc.desc  : Test SetOffloadAllowed interface.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetOffloadAllowed_001, TestSize.Level1)
{
    int32_t ret = -1;
    FILE *wavFile = fopen(RenderUT::AUDIORENDER_TEST_FILE_PATH.c_str(), "rb");
    ASSERT_NE(nullptr, wavFile);

    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    rendererOptions.rendererInfo.isOffloadAllowed = false;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    size_t bufferLen;
    ret = audioRenderer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);
    ASSERT_NE(nullptr, buffer);

    size_t bytesToWrite = 0;
    int32_t bytesWritten = 0;
    size_t minBytes = 4; // 4 min bytes
    int32_t numBuffersToRender = RenderUT::WRITE_BUFFERS_COUNT;

    while (numBuffersToRender) {
        bytesToWrite = fread(buffer, 1, bufferLen, wavFile);
        bytesWritten = 0;
        while ((static_cast<size_t>(bytesWritten) < bytesToWrite) &&
            ((static_cast<size_t>(bytesToWrite) - bytesWritten) > minBytes)) {
            bytesWritten += audioRenderer->Write(buffer + static_cast<size_t>(bytesWritten),
                                                 bytesToWrite - static_cast<size_t>(bytesWritten));
            EXPECT_GE(bytesWritten, RenderUT::VALUE_ZERO);
            if (bytesWritten < 0) {
                break;
            }
        }
        numBuffersToRender--;
    }

    audioRenderer->Drain();
    audioRenderer->Stop();
    audioRenderer->Release();

    free(buffer);
    fclose(wavFile);
}

/**
 * @tc.name  : Test voip can not interrupt voiceCall
 * @tc.number: SetVoipInterruptVoiceCall_001
 * @tc.desc  : When voip comes after voiceCall, voip will be deny by voiceCall
 */
HWTEST(AudioRendererUnitTest, SetVoipInterruptVoiceCall_001, TestSize.Level1)
{
    AudioRendererOptions rendererOptionsForVoip;
    rendererOptionsForVoip.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    rendererOptionsForVoip.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptionsForVoip.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    rendererOptionsForVoip.streamInfo.channels = AudioChannel::STEREO;
    rendererOptionsForVoip.rendererInfo.contentType = ContentType::CONTENT_TYPE_UNKNOWN;
    rendererOptionsForVoip.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_VOICE_COMMUNICATION;
    rendererOptionsForVoip.rendererInfo.rendererFlags = RenderUT::RENDERER_FLAG;

    unique_ptr<AudioRenderer> audioRendererForVoip = AudioRenderer::Create(rendererOptionsForVoip);
    if (audioRendererForVoip == nullptr) {
        return ;
    }
    shared_ptr<AudioRendererCallbackTest> audioRendererCB = make_shared<AudioRendererCallbackTest>();
    int32_t ret = audioRendererForVoip->SetRendererCallback(audioRendererCB);
    EXPECT_EQ(SUCCESS, ret);

    audioRendererForVoip->SetInterruptMode(INDEPENDENT_MODE);
    bool isStartedforVoip = audioRendererForVoip->Start();
    EXPECT_EQ(true, isStartedforVoip);

    AudioRendererOptions rendererOptionsForVoice;
    rendererOptionsForVoice.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    rendererOptionsForVoice.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptionsForVoice.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    rendererOptionsForVoice.streamInfo.channels = AudioChannel::STEREO;
    rendererOptionsForVoice.rendererInfo.contentType = ContentType::CONTENT_TYPE_UNKNOWN;
    rendererOptionsForVoice.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_VOICE_MODEM_COMMUNICATION;
    rendererOptionsForVoice.rendererInfo.rendererFlags = RenderUT::RENDERER_FLAG;

    unique_ptr<AudioRenderer> audioRendererForVoiceCall = AudioRenderer::Create(rendererOptionsForVoice);
    if (audioRendererForVoiceCall == nullptr) {
        return ;
    }
    audioRendererForVoiceCall->SetInterruptMode(INDEPENDENT_MODE);
    bool isStartedforVoiceCall = audioRendererForVoiceCall->Start();
    EXPECT_EQ(true, isStartedforVoiceCall);

    std::this_thread::sleep_for(std::chrono::seconds(3));
    EXPECT_EQ(AudioRendererUnitTest::interruptEventTest_.hintType, INTERRUPT_HINT_PAUSE);

    audioRendererForVoiceCall->Stop();
    audioRendererForVoiceCall->Release();
    audioRendererForVoip->Stop();
    audioRendererForVoip->Release();
}

/**
 * @tc.name  : Test voiceCall can interrupt voip
 * @tc.number: SetVoiceCallInterruptVoip_001
 * @tc.desc  : When voiceCall comes after voip, voip will be stopped by voiceCall
 */
HWTEST(AudioRendererUnitTest, SetVoiceCallInterruptVoip_001, TestSize.Level1)
{
    AudioRendererOptions rendererOptionsForVoice;
    rendererOptionsForVoice.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    rendererOptionsForVoice.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptionsForVoice.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    rendererOptionsForVoice.streamInfo.channels = AudioChannel::STEREO;
    rendererOptionsForVoice.rendererInfo.contentType = ContentType::CONTENT_TYPE_UNKNOWN;
    rendererOptionsForVoice.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_VOICE_MODEM_COMMUNICATION;
    rendererOptionsForVoice.rendererInfo.rendererFlags = RenderUT::RENDERER_FLAG;

    unique_ptr<AudioRenderer> audioRendererForVoiceCall = AudioRenderer::Create(rendererOptionsForVoice);
    if (audioRendererForVoiceCall == nullptr) {
        return ;
    }
    audioRendererForVoiceCall->SetInterruptMode(INDEPENDENT_MODE);
    bool isStartedforVoiceCall = audioRendererForVoiceCall->Start();
    EXPECT_EQ(true, isStartedforVoiceCall);

    AudioRendererOptions rendererOptionsForVoip;
    rendererOptionsForVoip.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    rendererOptionsForVoip.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptionsForVoip.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    rendererOptionsForVoip.streamInfo.channels = AudioChannel::STEREO;
    rendererOptionsForVoip.rendererInfo.contentType = ContentType::CONTENT_TYPE_UNKNOWN;
    rendererOptionsForVoip.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_VOICE_COMMUNICATION;
    rendererOptionsForVoip.rendererInfo.rendererFlags = RenderUT::RENDERER_FLAG;

    unique_ptr<AudioRenderer> audioRendererForVoip = AudioRenderer::Create(rendererOptionsForVoip);
    if (audioRendererForVoip == nullptr) {
        return ;
    }
    audioRendererForVoip->SetInterruptMode(INDEPENDENT_MODE);
    bool isStartedforVoip = audioRendererForVoip->Start();
    EXPECT_EQ(false, isStartedforVoip);

    audioRendererForVoip->Stop();
    audioRendererForVoip->Release();

    audioRendererForVoip->Stop();
    audioRendererForVoip->Release();
}

/**
 * @tc.name  : Test SetLowPowerVolumeImpl
 * @tc.number: Audio_Renderer_Set_Low_Power_Volume_001
 * @tc.desc  : Test SetLowPowerVolume interface.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Set_Low_Power_Volume_001, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    unique_ptr<AudioRendererProxyObj> audioRendererProxyObj = std::make_unique<AudioRendererProxyObj>();
    audioRendererProxyObj->SaveRendererObj(audioRendererPrivate);
    audioRendererProxyObj->SetOffloadModeImpl(0, true);
    audioRendererProxyObj->UnsetOffloadModeImpl();
    float ret = -1.0f;
    audioRendererProxyObj->SetLowPowerVolumeImpl(1.0f);
    audioRendererProxyObj->GetLowPowerVolumeImpl(ret);
    EXPECT_EQ(1.0f, ret);
}

/**
 * @tc.name  : Test SetLowPowerVolumeImpl
 * @tc.number: Audio_Renderer_Set_Low_Power_Volume_001
 * @tc.desc  : Test SetLowPowerVolume interface. if the renderer is nullptr.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Set_Low_Power_Volume_002, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    unique_ptr<AudioRendererProxyObj> audioRendererProxyObj = std::make_unique<AudioRendererProxyObj>();
    ASSERT_TRUE(audioRendererPrivate != nullptr);
    audioRendererProxyObj->SaveRendererObj(std::weak_ptr<AudioRendererPrivate>());
    audioRendererProxyObj->SetOffloadModeImpl(0, true);
    audioRendererProxyObj->UnsetOffloadModeImpl();
    float ret = -1.0f;
    audioRendererProxyObj->SetLowPowerVolumeImpl(1.0f);
    audioRendererProxyObj->GetLowPowerVolumeImpl(ret);
}

/**
 * @tc.name  : Test SetSwitchInfo
 * @tc.number: Audio_Renderer_SetSwitchInfo_001
 * @tc.desc  : Test SetSwitchInfo interface. if the renderPositionCb is nullptr, frameMarkPosition is 0.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetSwitchInfo_001, TestSize.Level1)
{
    AppInfo appInfo = {};
    AudioStreamParams audioStreamParams;
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    IAudioStream::SwitchInfo switchInfo;
    switchInfo.renderPositionCb = nullptr;
    switchInfo.renderPeriodPositionCb = nullptr;
    switchInfo.capturePeriodPositionCb = nullptr;
    switchInfo.capturePositionCb = nullptr;
    switchInfo.frameMarkPosition = 0;
    switchInfo.framePeriodNumber = 0;
    std::shared_ptr<IAudioStream> audioStream = IAudioStream::GetPlaybackStream(IAudioStream::FAST_STREAM,
        audioStreamParams, STREAM_DEFAULT, appInfo.appPid);
    audioRendererPrivate->SetSwitchInfo(switchInfo, audioStream);
    ASSERT_NE(nullptr, audioRendererPrivate);
}

/**
 * @tc.name  : Test SetSwitchInfo
 * @tc.number: Audio_Renderer_SetSwitchInfo_002
 * @tc.desc  : Test SetSwitchInfo interface. if the renderPositionCb is nullptr, frameMarkPosition is 1.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetSwitchInfo_002, TestSize.Level1)
{
    AppInfo appInfo = {};
    AudioStreamParams audioStreamParams;
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    IAudioStream::SwitchInfo switchInfo;
    switchInfo.renderPositionCb = nullptr;
    switchInfo.renderPeriodPositionCb = nullptr;
    switchInfo.capturePeriodPositionCb = nullptr;
    switchInfo.capturePositionCb = nullptr;
    switchInfo.frameMarkPosition = 1;
    switchInfo.framePeriodNumber = 1;
    std::shared_ptr<IAudioStream> audioStream = IAudioStream::GetPlaybackStream(IAudioStream::FAST_STREAM,
        audioStreamParams, STREAM_DEFAULT, appInfo.appPid);
    audioRendererPrivate->SetSwitchInfo(switchInfo, audioStream);
    ASSERT_NE(nullptr, audioRendererPrivate);
}

/**
 * @tc.name  : Test SetSwitchInfo
 * @tc.number: Audio_Renderer_SetSwitchInfo_003
 * @tc.desc  : Test SetSwitchInfo interface. if the renderPositionCb is not nullptr, frameMarkPosition is 0.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetSwitchInfo_003, TestSize.Level1)
{
    AppInfo appInfo = {};
    AudioStreamParams audioStreamParams;
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    IAudioStream::SwitchInfo switchInfo;
    shared_ptr<RendererPositionCallbackTest> positionCB = std::make_shared<RendererPositionCallbackTest>();
    shared_ptr<RendererPeriodPositionCallbackTest> periodPositionCB =
        std::make_shared<RendererPeriodPositionCallbackTest>();
    shared_ptr<CapturerPeriodPositionCallbackTest> capturerPeriodPositionCB =
        std::make_shared<CapturerPeriodPositionCallbackTest>();
    shared_ptr<CapturerPositionCallbackTest> capturerPositionCB = std::make_shared<CapturerPositionCallbackTest>();
    switchInfo.renderPositionCb = positionCB;
    switchInfo.renderPeriodPositionCb = periodPositionCB;
    switchInfo.capturePeriodPositionCb = capturerPeriodPositionCB;
    switchInfo.capturePositionCb = capturerPositionCB;
    switchInfo.frameMarkPosition = 0;
    switchInfo.framePeriodNumber = 0;
    std::shared_ptr<IAudioStream> audioStream = IAudioStream::GetPlaybackStream(IAudioStream::FAST_STREAM,
        audioStreamParams, STREAM_DEFAULT, appInfo.appPid);
    audioRendererPrivate->SetSwitchInfo(switchInfo, audioStream);
    ASSERT_NE(nullptr, audioRendererPrivate);
}

/**
 * @tc.name  : Test SetSwitchInfo
 * @tc.number: Audio_Renderer_SetSwitchInfo_004
 * @tc.desc  : Test SetSwitchInfo interface. if the renderPositionCb is not nullptr, frameMarkPosition is 1.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetSwitchInfo_004, TestSize.Level1)
{
    AppInfo appInfo = {};
    AudioStreamParams audioStreamParams;
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    IAudioStream::SwitchInfo switchInfo;
    shared_ptr<RendererPositionCallbackTest> positionCB = std::make_shared<RendererPositionCallbackTest>();
    shared_ptr<RendererPeriodPositionCallbackTest> periodPositionCB =
        std::make_shared<RendererPeriodPositionCallbackTest>();
    shared_ptr<CapturerPeriodPositionCallbackTest> capturerPeriodPositionCB =
        std::make_shared<CapturerPeriodPositionCallbackTest>();
    shared_ptr<CapturerPositionCallbackTest> capturerPositionCB = std::make_shared<CapturerPositionCallbackTest>();
    switchInfo.renderPositionCb = positionCB;
    switchInfo.renderPeriodPositionCb = periodPositionCB;
    switchInfo.capturePeriodPositionCb = capturerPeriodPositionCB;
    switchInfo.capturePositionCb = capturerPositionCB;
    switchInfo.frameMarkPosition = 1;
    switchInfo.framePeriodNumber = 1;
    std::shared_ptr<IAudioStream> audioStream = IAudioStream::GetPlaybackStream(IAudioStream::FAST_STREAM,
        audioStreamParams, STREAM_DEFAULT, appInfo.appPid);
    audioRendererPrivate->SetSwitchInfo(switchInfo, audioStream);
    ASSERT_NE(nullptr, audioRendererPrivate);
}

/**
 * @tc.name  : Test AudioRendererPrivate
 * @tc.number: SetAudioInterrupt
 * @tc.desc  : Test SetAudioInterrupt API
 */
HWTEST(AudioRendererUnitTest, SetAudioInterrupt_001, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    AudioInterrupt audioInterrupt;

    audioRendererPrivate->SetAudioInterrupt(audioInterrupt);
    EXPECT_EQ(audioRendererPrivate->audioInterrupt_.streamId, 0);
}

/**
 * @tc.name  : Test AudioRendererPrivate
 * @tc.number: SetSourceDuration
 * @tc.desc  : Test SetSourceDuration API
 */
HWTEST(AudioRendererUnitTest, SetSourceDuration_001, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    int64_t duration = 1;
    audioRendererPrivate->audioStream_ = nullptr;
    AudioStreamParams audioStreamParams;
    const AudioStreamType audioStreamType = STREAM_VOICE_CALL;
    IAudioStream::StreamClass streamClass;
    uint32_t flag = AUDIO_OUTPUT_FLAG_NORMAL;

    int32_t ret = audioRendererPrivate->PrepareAudioStream(audioStreamParams, audioStreamType, streamClass, flag);
    EXPECT_EQ(ret, SUCCESS);

    audioRendererPrivate->SetSourceDuration(duration);
    EXPECT_EQ(audioRendererPrivate->sourceDuration_, 1);
}

/**
 * @tc.name  : Test AudioRendererPrivate
 * @tc.number: SetAudioPrivacyType_001
 * @tc.desc  : Test SetAudioPrivacyType API
 */
HWTEST(AudioRendererUnitTest, SetAudioPrivacyType_001, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    AudioPrivacyType privacyType = PRIVACY_TYPE_PUBLIC;

    audioRendererPrivate->audioStream_ = nullptr;
    audioRendererPrivate->SetAudioPrivacyType(privacyType);
    EXPECT_EQ(audioRendererPrivate->audioStream_, nullptr);

    std::shared_ptr<TestAudioStremStub> testAudioStremStub = std::make_shared<TestAudioStremStub>();
    audioRendererPrivate->audioStream_ = testAudioStremStub;
    audioRendererPrivate->SetAudioPrivacyType(privacyType);
    EXPECT_NE(audioRendererPrivate->audioStream_, nullptr);
}

/**
 * @tc.name  : Test AudioRendererPrivate
 * @tc.number: SetClientInfo_001
 * @tc.desc  : Test SetClientInfo API
 */
HWTEST(AudioRendererUnitTest, SetClientInfo_001, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    uint32_t flag = AUDIO_OUTPUT_FLAG_FAST;
    IAudioStream::StreamClass streamClass;

    audioRendererPrivate->SetClientInfo(flag, streamClass);
    EXPECT_EQ(streamClass, IAudioStream::StreamClass::FAST_STREAM);

    flag = AUDIO_OUTPUT_FLAG_FAST | AUDIO_OUTPUT_FLAG_VOIP;
    audioRendererPrivate->SetClientInfo(flag, streamClass);
    EXPECT_EQ(streamClass, IAudioStream::StreamClass::VOIP_STREAM);

    flag = AUDIO_OUTPUT_FLAG_DIRECT;
    audioRendererPrivate->SetClientInfo(flag, streamClass);
    EXPECT_EQ(streamClass, IAudioStream::StreamClass::PA_STREAM);

    flag = AUDIO_OUTPUT_FLAG_MULTICHANNEL;
    audioRendererPrivate->SetClientInfo(flag, streamClass);
    EXPECT_EQ(streamClass, IAudioStream::StreamClass::PA_STREAM);

    flag = AUDIO_FLAG_NONE;
    audioRendererPrivate->SetClientInfo(flag, streamClass);
    EXPECT_EQ(streamClass, IAudioStream::StreamClass::PA_STREAM);
}

/**
 * @tc.name  : Test AudioRendererPrivate
 * @tc.number: SetVolumeMode_001
 * @tc.desc  : Test SetVolumeMode API
 */
HWTEST(AudioRendererUnitTest, SetVolumeMode_001, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    int32_t mode = 0;

    audioRendererPrivate->audioStream_ = nullptr;
    int32_t ret = audioRendererPrivate->SetVolumeMode(mode);
    EXPECT_EQ(ret, ERROR_ILLEGAL_STATE);

    std::shared_ptr<TestAudioStremStub> testAudioStremStub = std::make_shared<TestAudioStremStub>();
    audioRendererPrivate->audioStream_ = testAudioStremStub;
    ret = audioRendererPrivate->SetVolumeMode(mode);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioRendererPrivate
 * @tc.number: SetChannelBlendMode_001
 * @tc.desc  : Test SetChannelBlendMode API
 */
HWTEST(AudioRendererUnitTest, SetChannelBlendMode_001, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    ChannelBlendMode blendMode = MODE_DEFAULT;

    audioRendererPrivate->audioStream_ = nullptr;
    int32_t ret = audioRendererPrivate->SetChannelBlendMode(blendMode);
    EXPECT_EQ(ret, ERROR_ILLEGAL_STATE);

    std::shared_ptr<TestAudioStremStub> testAudioStremStub = std::make_shared<TestAudioStremStub>();
    audioRendererPrivate->audioStream_ = testAudioStremStub;
    ret = audioRendererPrivate->SetChannelBlendMode(blendMode);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : InitSwitchInfo_ShouldSetRendererFlags_WhenRendererFlagsIsNormal
 * @tc.number: InitSwitchInfoTest_002
 * @tc.desc  : Test when rendererFlags is AUDIO_FLAG_NORMAL then rendererFlags is set to AUDIO_FLAG_NORMAL
 */
HWTEST(AudioRendererUnitTest, InitSwitchInfo_ShouldSetRendererFlags_WhenRendererFlagsIsNormal, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    IAudioStream::StreamClass targetClass = IAudioStream::PA_STREAM;
    IAudioStream::SwitchInfo info;
    audioRendererPrivate->rendererInfo_.rendererFlags = AUDIO_FLAG_NORMAL;
    audioRendererPrivate->InitSwitchInfo(targetClass, info);
    EXPECT_EQ(info.rendererInfo.rendererFlags, AUDIO_FLAG_NORMAL);
}

/**
 * @tc.name  : InitSwitchInfo_ShouldSetRendererFlags_WhenRendererFlagsIsMMAP
 * @tc.number: InitSwitchInfoTest_003
 * @tc.desc  : Test when rendererFlags is AUDIO_FLAG_MMAP then rendererFlags is set to AUDIO_FLAG_MMAP
 */
HWTEST(AudioRendererUnitTest, InitSwitchInfo_ShouldSetRendererFlags_WhenRendererFlagsIsMMAP, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    IAudioStream::StreamClass targetClass = IAudioStream::FAST_STREAM;
    IAudioStream::SwitchInfo info;
    audioRendererPrivate->rendererInfo_.rendererFlags = AUDIO_FLAG_MMAP;
    audioRendererPrivate->InitSwitchInfo(targetClass, info);
    EXPECT_EQ(info.rendererInfo.rendererFlags, AUDIO_FLAG_MMAP);
}

/**
 * @tc.name  : Test SetFastStatusChangeCallback API.
 * @tc.number: SetFastStatusChangeCallback_001
 * @tc.desc  : Test SetFastStatusChangeCallback interface.
 */
HWTEST(AudioRendererUnitTest, SetFastStatusChangeCallback_001, TestSize.Level2)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    ASSERT_TRUE(audioRendererPrivate != nullptr);

    std::shared_ptr<RendererFastStatusChangeCallbackTest> fastStatusChangeCallback =
        std::make_shared<RendererFastStatusChangeCallbackTest>();

    audioRendererPrivate->SetFastStatusChangeCallback(fastStatusChangeCallback);
    EXPECT_NE(audioRendererPrivate->fastStatusChangeCallback_, nullptr);
}

/**
 * @tc.name  : Test SetAudioHapticsSyncId API.
 * @tc.number: SetAudioHapticsSyncId_001
 * @tc.desc  : Test SetAudioHapticsSyncId interface.
 */
HWTEST(AudioRendererUnitTest, SetAudioHapticsSyncId_001, TestSize.Level0)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    ASSERT_TRUE(audioRendererPrivate != nullptr);

    std::shared_ptr<IAudioStream> testAudioStreamStub = std::make_shared<TestAudioStremStub>();
    audioRendererPrivate->audioStream_ = testAudioStreamStub;

    int32_t syncId = 100000;
    audioRendererPrivate->SetAudioHapticsSyncId(syncId);
    EXPECT_EQ(audioRendererPrivate->audioHapticsSyncId_, syncId);

    int32_t syncId2 = -100000;
    audioRendererPrivate->SetAudioHapticsSyncId(syncId2);
    EXPECT_EQ(audioRendererPrivate->audioHapticsSyncId_, syncId);
}

/**
* @tc.name  : Test IsAllowedStartBackgroud.
* @tc.number: Audio_Renderer_IsAllowedStartBackgroud_001
* @tc.desc  : Test IsAllowedStartBackgroud interface, IsAllowedPlayback is false.
*/
HWTEST(AudioRendererUnitTest, Audio_Renderer_IsAllowedStartBackgroud_001, TestSize.Level1)
{
    AppInfo appInfo = {};
    shared_ptr<AudioRendererPrivate> audioRenderer =
        std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, appInfo, true);
    EXPECT_NE(nullptr, audioRenderer);

    audioRenderer->appInfo_.appUid = -1;
    audioRenderer->appInfo_.appPid = -1;
    audioRenderer->rendererInfo_.streamUsage = STREAM_USAGE_MOVIE;
    auto ret = audioRenderer->IsAllowedStartBackgroud();
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test IsAllowedStartBackgroud.
* @tc.number: Audio_Renderer_IsAllowedStartBackgroud_002
* @tc.desc  : Test IsAllowedStartBackgroud interface, IsAllowedPlayback is false.
*/
HWTEST(AudioRendererUnitTest, Audio_Renderer_IsAllowedStartBackgroud_002, TestSize.Level1)
{
    AppInfo appInfo = {};
    shared_ptr<AudioRendererPrivate> audioRenderer =
        std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, appInfo, true);
    EXPECT_NE(nullptr, audioRenderer);

    audioRenderer->appInfo_.appUid = -1;
    audioRenderer->appInfo_.appPid = -1;
    audioRenderer->rendererInfo_.streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;
    auto ret = audioRenderer->IsAllowedStartBackgroud();
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test CheckAndRestoreAudioRenderer.
* @tc.number: CheckAndRestoreAudioRenderer_001
* @tc.desc  : Test CheckAndRestoreAudioRenderer interface.
*/
HWTEST(AudioRendererUnitTest, CheckAndRestoreAudioRenderer_001, TestSize.Level1)
{
    AppInfo appInfo = {};
    shared_ptr<AudioRendererPrivate> audioRenderer =
        std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, appInfo, true);
    EXPECT_NE(nullptr, audioRenderer);

    std::shared_ptr<IAudioStream> testAudioStreamStub = std::make_shared<TestAudioStremStub>();
    audioRenderer->audioStream_ = testAudioStreamStub;

    std::string callingFunc = "test";
    audioRenderer->abortRestore_ = false;
    EXPECT_EQ(audioRenderer->audioStream_->SetRestoreStatus(NO_NEED_FOR_RESTORE), NO_NEED_FOR_RESTORE);
    auto ret = audioRenderer->CheckAndRestoreAudioRenderer(callingFunc);
    EXPECT_EQ(ret, SUCCESS);

    EXPECT_EQ(audioRenderer->audioStream_->SetRestoreStatus(RESTORING), RESTORING);
    ret = audioRenderer->CheckAndRestoreAudioRenderer(callingFunc);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
* @tc.name  : Test CheckAndRestoreAudioRenderer.
* @tc.number: CheckAndRestoreAudioRenderer_002
* @tc.desc  : Test CheckAndRestoreAudioRenderer interface.
*/
HWTEST(AudioRendererUnitTest, CheckAndRestoreAudioRenderer_002, TestSize.Level1)
{
    AppInfo appInfo = {};
    shared_ptr<AudioRendererPrivate> audioRenderer =
        std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, appInfo, true);
    EXPECT_NE(nullptr, audioRenderer);

    std::shared_ptr<IAudioStream> testAudioStreamStub = std::make_shared<TestAudioStremStub>();
    audioRenderer->audioStream_ = testAudioStreamStub;

    AudioInterrupt audioInterrupt;
    audioRenderer->audioInterruptCallback_ = std::make_shared<AudioRendererInterruptCallbackImpl>(
        testAudioStreamStub, audioInterrupt);

    std::string callingFunc = "test";
    audioRenderer->abortRestore_ = false;
    EXPECT_EQ(audioRenderer->audioStream_->SetRestoreStatus(NEED_RESTORE_TO_NORMAL), NEED_RESTORE_TO_NORMAL);
    auto ret = audioRenderer->CheckAndRestoreAudioRenderer(callingFunc);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test CheckAndRestoreAudioRenderer.
* @tc.number: CheckAndRestoreAudioRenderer_003
* @tc.desc  : Test CheckAndRestoreAudioRenderer interface.
*/
HWTEST(AudioRendererUnitTest, CheckAndRestoreAudioRenderer_003, TestSize.Level1)
{
    AppInfo appInfo = {};
    shared_ptr<AudioRendererPrivate> audioRenderer =
        std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, appInfo, true);
    EXPECT_NE(nullptr, audioRenderer);

    std::shared_ptr<IAudioStream> testAudioStreamStub = std::make_shared<TestAudioStremStub>();
    audioRenderer->audioStream_ = testAudioStreamStub;

    std::string callingFunc = "test";
    audioRenderer->abortRestore_ = false;
    audioRenderer->audioInterruptCallback_ = nullptr;
    EXPECT_EQ(audioRenderer->audioStream_->SetRestoreStatus(NEED_RESTORE), NEED_RESTORE);
    auto ret = audioRenderer->CheckAndRestoreAudioRenderer(callingFunc);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test CheckAndRestoreAudioRenderer.
* @tc.number: CheckAndRestoreAudioRenderer_004
* @tc.desc  : Test CheckAndRestoreAudioRenderer when audioStream_ is nullptr.
*/
HWTEST(AudioRendererUnitTest, CheckAndRestoreAudioRenderer_004, TestSize.Level1)
{
    AppInfo appInfo = {};
    shared_ptr<AudioRendererPrivate> audioRenderer =
        std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, appInfo, true);
    EXPECT_NE(nullptr, audioRenderer);

    audioRenderer->audioStream_ = nullptr;
    std::string callingFunc = "test";
    auto ret = audioRenderer->CheckAndRestoreAudioRenderer(callingFunc);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
* @tc.name  : Test CheckAndRestoreAudioRenderer.
* @tc.number: CheckAndRestoreAudioRenderer_005
* @tc.desc  : Test CheckAndRestoreAudioRenderer when abortRestore_ is true.
*/
HWTEST(AudioRendererUnitTest, CheckAndRestoreAudioRenderer_005, TestSize.Level1)
{
    AppInfo appInfo = {};
    shared_ptr<AudioRendererPrivate> audioRenderer =
        std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, appInfo, true);
    EXPECT_NE(nullptr, audioRenderer);

    std::shared_ptr<IAudioStream> testAudioStreamStub = std::make_shared<TestAudioStremStub>();
    audioRenderer->audioStream_ = testAudioStreamStub;

    AudioInterrupt audioInterrupt;
    audioRenderer->audioInterruptCallback_ = std::make_shared<AudioRendererInterruptCallbackImpl>(
        testAudioStreamStub, audioInterrupt);

    std::string callingFunc = "test";
    audioRenderer->abortRestore_ = true;
    EXPECT_EQ(audioRenderer->audioStream_->SetRestoreStatus(NEED_RESTORE), NEED_RESTORE);
    auto ret = audioRenderer->CheckAndRestoreAudioRenderer(callingFunc);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test StartSwitchProcess.
* @tc.number: StartSwitchProcess_001
* @tc.desc  : Test StartSwitchProcess when audioInterruprCallback_ is nullptr.
*/
HWTEST(AudioRendererUnitTest, StartSwitchProcess_001, TestSize.Level1)
{
    AppInfo appInfo = {};
    shared_ptr<AudioRendererPrivate> audioRenderer =
        std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, appInfo, true);
    EXPECT_NE(nullptr, audioRenderer);

    std::shared_ptr<IAudioStream> testAudioStreamStub = std::make_shared<TestAudioStremStub>();
    audioRenderer->audioStream_ = testAudioStreamStub;

    RestoreInfo restoreInfo;
    std::string callingFunc = "test";
    audioRenderer->abortRestore_ = false;
    audioRenderer->audioInterruptCallback_ = nullptr;
    IAudioStream::StreamClass targetClass = IAudioStream::StreamClass::FAST_STREAM;
    EXPECT_EQ(audioRenderer->audioStream_->SetRestoreStatus(NEED_RESTORE), NEED_RESTORE);
    int32_t ret = audioRenderer->StartSwitchProcess(restoreInfo, targetClass, callingFunc);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test StartSwitchProcess.
* @tc.number: StartSwitchProcess_002
* @tc.desc  : Test StartSwitchProcess when switchtotargetstream is false.
*/
HWTEST(AudioRendererUnitTest, StartSwitchProcess_002, TestSize.Level1)
{
    AppInfo appInfo = {};
    shared_ptr<AudioRendererPrivate> audioRenderer =
        std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, appInfo, true);
    EXPECT_NE(nullptr, audioRenderer);

    std::shared_ptr<IAudioStream> testAudioStreamStub = std::make_shared<TestAudioStremStub>();
    audioRenderer->audioStream_ = testAudioStreamStub;

    AudioInterrupt audioInterrupt;
    audioRenderer->audioInterruptCallback_ = std::make_shared<AudioRendererInterruptCallbackImpl>(
        testAudioStreamStub, audioInterrupt);

    RestoreInfo restoreInfo;
    std::string callingFunc = "test";
    audioRenderer->abortRestore_ = false;
    IAudioStream::StreamClass targetClass = IAudioStream::StreamClass::FAST_STREAM;
    EXPECT_EQ(audioRenderer->audioStream_->SetRestoreStatus(NEED_RESTORE), NEED_RESTORE);
    int32_t ret = audioRenderer->StartSwitchProcess(restoreInfo, targetClass, callingFunc);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test StartSwitchProcess.
* @tc.number: StartSwitchProcess_003
* @tc.desc  : Test StartSwitchProcess when switchtotargetstream is false.
*/
HWTEST(AudioRendererUnitTest, StartSwitchProcess_003, TestSize.Level1)
{
    AppInfo appInfo = {};
    shared_ptr<AudioRendererPrivate> audioRenderer =
        std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, appInfo, true);
    EXPECT_NE(nullptr, audioRenderer);

    std::shared_ptr<IAudioStream> testAudioStreamStub = std::make_shared<TestAudioStremStub>();
    audioRenderer->audioStream_ = testAudioStreamStub;

    AudioInterrupt audioInterrupt;
    audioRenderer->audioInterruptCallback_ = std::make_shared<AudioRendererInterruptCallbackImpl>(
        testAudioStreamStub, audioInterrupt);

    RestoreInfo restoreInfo;
    restoreInfo.restoreReason = DEFAULT_REASON;
    std::string callingFunc = "test";
    audioRenderer->abortRestore_ = false;
    IAudioStream::StreamClass targetClass = IAudioStream::StreamClass::FAST_STREAM;
    EXPECT_EQ(audioRenderer->audioStream_->SetRestoreStatus(NEED_RESTORE), NEED_RESTORE);
    int32_t ret = audioRenderer->StartSwitchProcess(restoreInfo, targetClass, callingFunc);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test StartSwitchProcess.
* @tc.number: StartSwitchProcess_004
* @tc.desc  : Test StartSwitchProcess when switchtotargetstream is false.
*/
HWTEST(AudioRendererUnitTest, StartSwitchProcess_004, TestSize.Level1)
{
    AppInfo appInfo = {};
    shared_ptr<AudioRendererPrivate> audioRenderer =
        std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, appInfo, true);
    EXPECT_NE(nullptr, audioRenderer);

    std::shared_ptr<IAudioStream> testAudioStreamStub = std::make_shared<TestAudioStremStub>();
    audioRenderer->audioStream_ = testAudioStreamStub;

    AudioInterrupt audioInterrupt;
    audioRenderer->audioInterruptCallback_ = std::make_shared<AudioRendererInterruptCallbackImpl>(
        testAudioStreamStub, audioInterrupt);

    RestoreInfo restoreInfo;
    restoreInfo.restoreReason = SERVER_DIED;
    std::string callingFunc = "test";
    audioRenderer->abortRestore_ = false;
    IAudioStream::StreamClass targetClass = IAudioStream::StreamClass::FAST_STREAM;
    EXPECT_EQ(audioRenderer->audioStream_->SetRestoreStatus(NEED_RESTORE), NEED_RESTORE);
    int32_t ret = audioRenderer->StartSwitchProcess(restoreInfo, targetClass, callingFunc);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test SetSwitchInfo.
* @tc.number: Audio_Renderer_SetSwitchInfo_005
* @tc.desc  : Test SetSwitchInfo interface.
*/
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetSwitchInfo_005, TestSize.Level1)
{
    AppInfo appInfo = {};
    IAudioStream::SwitchInfo info;
    shared_ptr<AudioRendererPrivate> audioRenderer =
        std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, appInfo, true);
    EXPECT_NE(nullptr, audioRenderer);

    std::shared_ptr<IAudioStream> audioStream = std::make_shared<TestAudioStremStub>();
    bool ret = audioRenderer->SetSwitchInfo(info, audioStream);
    EXPECT_TRUE(ret);
}

/**
* @tc.name  : Test SetSwitchInfo.
* @tc.number: Audio_Renderer_SetSwitchInfo_006
* @tc.desc  : Test SetSwitchInfo interface.
*/
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetSwitchInfo_006, TestSize.Level1)
{
    AppInfo appInfo = {};
    IAudioStream::SwitchInfo info;
    info.userSettedPreferredFrameSize = 0;
    info.lastCallStartByUserTid = 0;
    info.frameMarkPosition = 1;
    info.framePeriodNumber = 1;
    shared_ptr<AudioRendererPrivate> audioRenderer =
        std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, appInfo, true);
    EXPECT_NE(nullptr, audioRenderer);

    audioRenderer->speed_ = 0;
    audioRenderer->pitch_ = 0;
    std::shared_ptr<IAudioStream> audioStream = std::make_shared<TestAudioStremStub>();
    bool ret = audioRenderer->SetSwitchInfo(info, audioStream);
    EXPECT_TRUE(ret);
}

/**
* @tc.name  : Test SetSwitchInfo.
* @tc.number: Audio_Renderer_SetSwitchInfo_007
* @tc.desc  : Test SetSwitchInfo interface.
*/
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetSwitchInfo_007, TestSize.Level1)
{
    AppInfo appInfo = {};
    IAudioStream::SwitchInfo info;
    info.userSettedPreferredFrameSize = 0;
    info.lastCallStartByUserTid = 0;
    info.frameMarkPosition = 1;
    info.framePeriodNumber = 1;
    info.renderPositionCb = std::make_shared<RendererPositionCallbackTest>();
    info.capturePositionCb = std::make_shared<CapturerPositionCallbackTest>();
    info.renderPeriodPositionCb = std::make_shared<RendererPeriodPositionCallbackTest>();
    info.capturePeriodPositionCb = std::make_shared<CapturerPeriodPositionCallbackTest>();
    shared_ptr<AudioRendererPrivate> audioRenderer =
        std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, appInfo, true);
    EXPECT_NE(nullptr, audioRenderer);

    audioRenderer->speed_ = 0;
    audioRenderer->pitch_ = 0;
    std::shared_ptr<IAudioStream> audioStream = std::make_shared<TestAudioStremStub>();
    bool ret = audioRenderer->SetSwitchInfo(info, audioStream);
    EXPECT_TRUE(ret);
}

/**
* @tc.name  : Test SetSwitchInfo.
* @tc.number: Audio_Renderer_SetSwitchInfo_008
* @tc.desc  : Test SetSwitchInfo interface.
*/
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetSwitchInfo_008, TestSize.Level1)
{
    AppInfo appInfo = {};
    IAudioStream::SwitchInfo info;
    info.renderPositionCb = std::make_shared<RendererPositionCallbackTest>();
    info.capturePositionCb = std::make_shared<CapturerPositionCallbackTest>();
    info.renderPeriodPositionCb = std::make_shared<RendererPeriodPositionCallbackTest>();
    info.capturePeriodPositionCb = std::make_shared<CapturerPeriodPositionCallbackTest>();
    shared_ptr<AudioRendererPrivate> audioRenderer =
        std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, appInfo, true);
    EXPECT_NE(nullptr, audioRenderer);

    std::shared_ptr<IAudioStream> audioStream = std::make_shared<TestAudioStremStub>();
    bool ret = audioRenderer->SetSwitchInfo(info, audioStream);
    EXPECT_TRUE(ret);
}

/**
* @tc.name  : Test InitTargetStream.
* @tc.number: Audio_Renderer_InitTargetStream_003
* @tc.desc  : Test InitTargetStream interface.
*/
HWTEST(AudioRendererUnitTest, Audio_Renderer_InitTargetStream_003, TestSize.Level1)
{
    AppInfo appInfo = {};
    shared_ptr<AudioRendererPrivate> audioRenderer =
        std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, appInfo, true);
    EXPECT_NE(nullptr, audioRenderer);

    IAudioStream::SwitchInfo info;
    info.rendererInfo.originalFlag = AUDIO_FLAG_INVALID;
    std::shared_ptr<IAudioStream> newAudioStream = nullptr;
    auto ret = audioRenderer->InitTargetStream(info, newAudioStream);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test InitTargetStream.
* @tc.number: Audio_Renderer_InitTargetStream_004
* @tc.desc  : Test InitTargetStream interface.
*/
HWTEST(AudioRendererUnitTest, Audio_Renderer_InitTargetStream_004, TestSize.Level1)
{
    AppInfo appInfo = {};
    shared_ptr<AudioRendererPrivate> audioRenderer =
        std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, appInfo, true);
    EXPECT_NE(nullptr, audioRenderer);

    IAudioStream::SwitchInfo info;
    info.rendererInfo.originalFlag = AUDIO_FLAG_INVALID;
    std::shared_ptr<IAudioStream> newAudioStream = std::make_shared<TestAudioStremStub>();
    auto ret = audioRenderer->InitTargetStream(info, newAudioStream);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test InitTargetStream.
* @tc.number: Audio_Renderer_InitTargetStream_005
* @tc.desc  : Test InitTargetStream interface.
*/
HWTEST(AudioRendererUnitTest, Audio_Renderer_InitTargetStream_005, TestSize.Level1)
{
    AppInfo appInfo = {};
    shared_ptr<AudioRendererPrivate> audioRenderer =
        std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, appInfo, true);
    EXPECT_NE(nullptr, audioRenderer);

    IAudioStream::SwitchInfo info;
    info.rendererInfo.originalFlag = AUDIO_FLAG_NORMAL;
    std::shared_ptr<IAudioStream> newAudioStream = std::make_shared<TestAudioStremStub>();
    auto ret = audioRenderer->InitTargetStream(info, newAudioStream);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test GenerateNewStream.
* @tc.number: Audio_Renderer_GenerateNewStream_001
* @tc.desc  : Test GenerateNewStream interface.
*/
HWTEST(AudioRendererUnitTest, Audio_Renderer_GenerateNewStream_001, TestSize.Level1)
{
    AppInfo appInfo = {};
    shared_ptr<AudioRendererPrivate> audioRenderer =
        std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, appInfo, true);
    EXPECT_NE(nullptr, audioRenderer);

    RestoreInfo restoreInfo;
    RendererState previousState = RENDERER_NEW;
    IAudioStream::SwitchInfo switchInfo;
    switchInfo.eStreamType = STREAM_MUSIC;

    auto ret = audioRenderer->GenerateNewStream(IAudioStream::StreamClass::FAST_STREAM, restoreInfo,
        previousState, switchInfo);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test SwitchToTargetStream.
* @tc.number: Audio_Renderer_SwitchToTargetStream_001
* @tc.desc  : Test SwitchToTargetStream interface.
*/
HWTEST(AudioRendererUnitTest, Audio_Renderer_SwitchToTargetStream_001, TestSize.Level1)
{
    AppInfo appInfo = {};
    shared_ptr<AudioRendererPrivate> audioRenderer =
        std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, appInfo, true);
    EXPECT_NE(nullptr, audioRenderer);

    RestoreInfo restoreInfo;
    restoreInfo.restoreReason = DEFAULT_REASON;
    auto ret = audioRenderer->SwitchToTargetStream(IAudioStream::StreamClass::FAST_STREAM, restoreInfo);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test RestoreAudioInLoop.
* @tc.number: Audio_Renderer_RestoreAudioInLoop_002.
* @tc.desc  : Test RestoreAudioInLoop interface.
*/
HWTEST(AudioRendererUnitTest, Audio_Renderer_RestoreAudioInLoop_002, TestSize.Level1)
{
    AppInfo appInfo = {};
    shared_ptr<AudioRendererPrivate> audioRenderer =
        std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, appInfo, true);
    EXPECT_NE(nullptr, audioRenderer);

    bool restoreResult = true;
    int32_t tryCounter = 0;
    audioRenderer->callbackLoopTid_ = gettid();
    audioRenderer->RestoreAudioInLoop(restoreResult, tryCounter);

    audioRenderer->callbackLoopTid_ = gettid() + 1;
    audioRenderer->RestoreAudioInLoop(restoreResult, tryCounter);
}

/**
* @tc.name  : Test FastStatusChangeCallback.
* @tc.number: Audio_Renderer_FastStatusChangeCallback_001.
* @tc.desc  : Test FastStatusChangeCallback interface.
*/
HWTEST(AudioRendererUnitTest, Audio_Renderer_FastStatusChangeCallback_001, TestSize.Level1)
{
    AppInfo appInfo = {};
    shared_ptr<AudioRendererPrivate> audioRenderer =
        std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, appInfo, true);
    EXPECT_NE(nullptr, audioRenderer);

    FastStatus status = FASTSTATUS_NORMAL;
    audioRenderer->FastStatusChangeCallback(status);
}

/**
* @tc.name  : Test SetRenderRate.
* @tc.number: SetRenderRate_002.
* @tc.desc  : Test SetRenderRate interface.
*/
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetRenderRate_002, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    AudioRendererRate renderRate = static_cast<AudioRendererRate>(100);
    ret = audioRenderer->SetRenderRate(renderRate);
    EXPECT_EQ(SUCCESS, ret);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetInterruptMode API via legal input
 * @tc.number: Audio_Renderer_SetInterruptMode_003
 * @tc.desc  : Test SetInterruptMode interface. Returns 0 {SUCCESS}, if the setting is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetInterruptMode_003, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererUnitTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    audioRenderer->SetInterruptMode(static_cast<InterruptMode>(100));

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);
    audioRenderer->Release();
}

/**
* @tc.name  : Test GenerateNewStream.
* @tc.number: Audio_Renderer_GenerateNewStream_002
* @tc.desc  : Test GenerateNewStream interface.
*/
HWTEST(AudioRendererUnitTest, Audio_Renderer_GenerateNewStream_002, TestSize.Level1)
{
    AppInfo appInfo = {};
    shared_ptr<AudioRendererPrivate> audioRenderer =
        std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, appInfo, true);
    EXPECT_NE(nullptr, audioRenderer);

    RestoreInfo restoreInfo;
    restoreInfo.restoreReason = SERVER_DIED;
    RendererState previousState = RENDERER_NEW;
    IAudioStream::SwitchInfo switchInfo;
    switchInfo.eStreamType = STREAM_MUSIC;

    auto ret = audioRenderer->GenerateNewStream(IAudioStream::StreamClass::FAST_STREAM, restoreInfo,
        previousState, switchInfo);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test SwitchToTargetStream.
* @tc.number: Audio_Renderer_SwitchToTargetStream_002
* @tc.desc  : Test SwitchToTargetStream interface.
*/
HWTEST(AudioRendererUnitTest, Audio_Renderer_SwitchToTargetStream_002, TestSize.Level1)
{
    AppInfo appInfo = {};
    shared_ptr<AudioRendererPrivate> audioRenderer =
        std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, appInfo, true);
    EXPECT_NE(nullptr, audioRenderer);

    RestoreInfo restoreInfo;
    restoreInfo.restoreReason = SERVER_DIED;
    auto ret = audioRenderer->SwitchToTargetStream(IAudioStream::StreamClass::FAST_STREAM, restoreInfo);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioRendererPrivate
 * @tc.number: SetClientInfo_002
 * @tc.desc  : Test SetClientInfo API
 */
HWTEST(AudioRendererUnitTest, SetClientInfo_002, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    uint32_t flag = AUDIO_OUTPUT_FLAG_DIRECT;
    IAudioStream::StreamClass streamClass;

    audioRendererPrivate->SetClientInfo(flag, streamClass);
    EXPECT_EQ(streamClass, IAudioStream::StreamClass::PA_STREAM);

    flag = AUDIO_OUTPUT_FLAG_DIRECT | AUDIO_OUTPUT_FLAG_VOIP;
    audioRendererPrivate->SetClientInfo(flag, streamClass);
    EXPECT_EQ(streamClass, IAudioStream::StreamClass::PA_STREAM);
}
} // namespace AudioStandard
} // namespace OHOS