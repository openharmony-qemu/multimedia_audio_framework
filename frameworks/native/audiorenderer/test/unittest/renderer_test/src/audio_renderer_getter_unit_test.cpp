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
#include "audio_renderer.cpp"
#include "fast_audio_stream.h"

using namespace std;
using namespace std::chrono;
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {

/**
 * @tc.name  : Test GetSupportedFormats API
 * @tc.number: Audio_Renderer_GetSupportedFormats_001
 * @tc.desc  : Test GetSupportedFormats interface. Returns supported Formats on success.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetSupportedFormats_001, TestSize.Level0)
{
    vector<AudioSampleFormat> supportedFormatList = AudioRenderer::GetSupportedFormats();
    EXPECT_EQ(AUDIO_SUPPORTED_FORMATS.size(), supportedFormatList.size());
}

/**
 * @tc.name  : Test GetSupportedChannels API
 * @tc.number: Audio_Renderer_GetSupportedChannels_001
 * @tc.desc  : Test GetSupportedChannels interface. Returns supported Channels on success.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetSupportedChannels_001, TestSize.Level0)
{
    vector<AudioChannel> supportedChannelList = AudioRenderer::GetSupportedChannels();
    EXPECT_EQ(RENDERER_SUPPORTED_CHANNELS.size(), supportedChannelList.size());
}

/**
 * @tc.name  : Test GetSupportedEncodingTypes API
 * @tc.number: Audio_Renderer_GetSupportedEncodingTypes_001
 * @tc.desc  : Test GetSupportedEncodingTypes interface. Returns supported Encoding types on success.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetSupportedEncodingTypes_001, TestSize.Level0)
{
    vector<AudioEncodingType> supportedEncodingTypes
                                        = AudioRenderer::GetSupportedEncodingTypes();
    EXPECT_EQ(AUDIO_SUPPORTED_ENCODING_TYPES.size(), supportedEncodingTypes.size());
}

/**
 * @tc.name  : Test GetSupportedSamplingRates API
 * @tc.number: Audio_Renderer_GetSupportedSamplingRates_001
 * @tc.desc  : Test GetSupportedSamplingRates interface. Returns supported Sampling rates on success.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetSupportedSamplingRates_001, TestSize.Level0)
{
    vector<AudioSamplingRate> supportedSamplingRates = AudioRenderer::GetSupportedSamplingRates();
    EXPECT_EQ(AUDIO_SUPPORTED_SAMPLING_RATES.size(), supportedSamplingRates.size());
}

/**
 * @tc.name  : Test GetParams API via legal input.
 * @tc.number: Audio_Renderer_GetParams_001
 * @tc.desc  : Test GetParams interface. Returns 0 {SUCCESS}, if the getting is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetParams_001, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    AudioRendererParams rendererParams;
    rendererParams.sampleFormat = SAMPLE_S16LE;
    rendererParams.sampleRate = SAMPLE_RATE_44100;
    rendererParams.channelCount = STEREO;
    rendererParams.encodingType = ENCODING_PCM;
    ret = audioRenderer->SetParams(rendererParams);
    EXPECT_EQ(SUCCESS, ret);

    AudioRendererParams getRendererParams;
    ret = audioRenderer->GetParams(getRendererParams);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(rendererParams.sampleFormat, getRendererParams.sampleFormat);
    EXPECT_EQ(rendererParams.sampleRate, getRendererParams.sampleRate);
    EXPECT_EQ(rendererParams.channelCount, getRendererParams.channelCount);
    EXPECT_EQ(rendererParams.encodingType, getRendererParams.encodingType);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetParams API via legal state, RENDERER_RUNNING: GetParams after Start.
 * @tc.number: Audio_Renderer_GetParams_002
 * @tc.desc  : Test GetParams interface. Returns 0 {SUCCESS} if the getting is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetParams_002, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    AudioRendererParams rendererParams;
    rendererParams.sampleFormat = SAMPLE_S16LE;
    rendererParams.sampleRate = SAMPLE_RATE_44100;
    rendererParams.channelCount = MONO;
    rendererParams.encodingType = ENCODING_PCM;
    ret = audioRenderer->SetParams(rendererParams);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    AudioRendererParams getRendererParams;
    ret = audioRenderer->GetParams(getRendererParams);
    EXPECT_EQ(SUCCESS, ret);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetParams API via illegal state, RENDERER_NEW: Call GetParams without SetParams.
 * @tc.number: Audio_Renderer_GetParams_003
 * @tc.desc  : Test GetParams interface. Returns error code, if the renderer state is RENDERER_NEW.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetParams_003, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    AudioRendererParams rendererParams;
    rendererParams.sampleFormat = SAMPLE_S16LE;
    rendererParams.sampleRate = SAMPLE_RATE_44100;
    rendererParams.channelCount = MONO;
    rendererParams.encodingType = ENCODING_PCM;

    AudioRendererParams getRendererParams;
    ret = audioRenderer->GetParams(getRendererParams);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);
}

/**
 * @tc.name  : Test GetParams API via illegal state, RENDERER_RELEASED: Call GetParams after Release.
 * @tc.number: Audio_Renderer_GetParams_004
 * @tc.desc  : Test GetParams interface. Returns error code, if the renderer state is RENDERER_RELEASED.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetParams_004, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererUnitTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);

    AudioRendererParams getRendererParams;
    ret = audioRenderer->GetParams(getRendererParams);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);
}

/**
 * @tc.name  : Test GetParams API via legal state, RENDERER_STOPPED: GetParams after Stop.
 * @tc.number: Audio_Renderer_GetParams_005
 * @tc.desc  : Test GetParams interface. Returns 0 {SUCCESS}, if the getting is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetParams_005, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererUnitTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    AudioRendererParams getRendererParams;
    ret = audioRenderer->GetParams(getRendererParams);
    EXPECT_EQ(SUCCESS, ret);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetParams API via legal input.
 * @tc.number: Audio_Renderer_GetParams_006
 * @tc.desc  : Test GetParams interface. Returns 0 {SUCCESS}, if the getting is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetParams_006, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    AudioRendererParams rendererParams;
    rendererParams.sampleFormat = SAMPLE_S24LE;
    rendererParams.sampleRate = SAMPLE_RATE_44100;
    rendererParams.channelCount = STEREO;
    rendererParams.encodingType = ENCODING_PCM;
    ret = audioRenderer->SetParams(rendererParams);
    EXPECT_EQ(SUCCESS, ret);

    AudioRendererParams getRendererParams;
    ret = audioRenderer->GetParams(getRendererParams);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(rendererParams.sampleFormat, getRendererParams.sampleFormat);
    EXPECT_EQ(rendererParams.sampleRate, getRendererParams.sampleRate);
    EXPECT_EQ(rendererParams.channelCount, getRendererParams.channelCount);
    EXPECT_EQ(rendererParams.encodingType, getRendererParams.encodingType);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetParams API via legal input.
 * @tc.number: Audio_Renderer_GetParams_007
 * @tc.desc  : Test GetParams interface. Returns 0 {SUCCESS}, if the getting is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetParams_007, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    AudioRendererParams rendererParams;
    rendererParams.sampleFormat = SAMPLE_S32LE;
    rendererParams.sampleRate = SAMPLE_RATE_44100;
    rendererParams.channelCount = STEREO;
    rendererParams.encodingType = ENCODING_PCM;
    ret = audioRenderer->SetParams(rendererParams);
    EXPECT_EQ(SUCCESS, ret);

    AudioRendererParams getRendererParams;
    ret = audioRenderer->GetParams(getRendererParams);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(rendererParams.sampleFormat, getRendererParams.sampleFormat);
    EXPECT_EQ(rendererParams.sampleRate, getRendererParams.sampleRate);
    EXPECT_EQ(rendererParams.channelCount, getRendererParams.channelCount);
    EXPECT_EQ(rendererParams.encodingType, getRendererParams.encodingType);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetParams API via legal input.
 * @tc.number: Audio_Renderer_GetParams_008
 * @tc.desc  : Test GetParams interface. Returns 0 {SUCCESS}, if the getting is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetParams_008, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    AudioRendererParams getRendererParams;
    getRendererParams.sampleFormat = AudioSampleFormat::INVALID_WIDTH;
    ret = audioRenderer->GetParams(getRendererParams);
    EXPECT_EQ(true, ret < 0);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetBufQueueState
 * @tc.number: Audio_Renderer_GetBufQueueState_001
 * @tc.desc  : Test GetBufQueueState interface. Returns BufferQueueState, if obtained successfully.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetBufQueueState_001, TestSize.Level1)
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

    BufferQueueState bQueueSate {};
    bQueueSate.currentIndex = 1;
    bQueueSate.numBuffers = 1;

    ret = audioRenderer->GetBufQueueState(bQueueSate);
    EXPECT_EQ(SUCCESS, ret);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetParams API stability.
 * @tc.number: Audio_Renderer_GetParams_Stability_001
 * @tc.desc  : Test GetParams interface stability.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetParams_Stability_001, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    AudioRendererParams rendererParams;
    rendererParams.sampleFormat = SAMPLE_S16LE;
    rendererParams.sampleRate = SAMPLE_RATE_44100;
    rendererParams.channelCount = STEREO;
    rendererParams.encodingType = ENCODING_PCM;

    ret = audioRenderer->SetParams(rendererParams);
    EXPECT_EQ(SUCCESS, ret);

    for (int i = 0; i < RenderUT::VALUE_THOUSAND; i++) {
        AudioRendererParams getRendererParams;
        ret = audioRenderer->GetParams(getRendererParams);
        EXPECT_EQ(SUCCESS, ret);
    }

    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetBufferSize API via legal input.
 * @tc.number: Audio_Renderer_GetBufferSize_001
 * @tc.desc  : Test GetBufferSize interface. Returns 0 {SUCCESS}, if the getting is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetBufferSize_001, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    size_t bufferLen;
    ret = audioRenderer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetBufferSize API via illegal state, RENDERER_NEW: without initializing the renderer.
 * @tc.number: Audio_Renderer_GetBufferSize_002
 * @tc.desc  : Test GetBufferSize interface. Returns error code, if the renderer state is RENDERER_NEW.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetBufferSize_002, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    size_t bufferLen;
    ret = audioRenderer->GetBufferSize(bufferLen);
    EXPECT_EQ(RenderUT::VALUE_ZERO, ret);
}

/**
 * @tc.name  : Test GetBufferSize API via illegal state, RENDERER_RELEASED: call Release before GetBufferSize
 * @tc.number: Audio_Renderer_GetBufferSize_003
 * @tc.desc  : Test GetBufferSize interface. Returns error code, if the renderer state is RENDERER_RELEASED.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetBufferSize_003, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);

    size_t bufferLen;
    ret = audioRenderer->GetBufferSize(bufferLen);
    EXPECT_EQ(ERR_ILLEGAL_STATE, ret);
}

/**
 * @tc.name  : Test GetBufferSize API via legal state, RENDERER_STOPPED: call Stop before GetBufferSize
 * @tc.number: Audio_Renderer_GetBufferSize_004
 * @tc.desc  : Test GetBufferSize interface. Returns 0 {SUCCESS}, if the getting is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetBufferSize_004, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    size_t bufferLen;
    ret = audioRenderer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetBufferSize API via legal state, RENDERER_RUNNING: call Start before GetBufferSize
 * @tc.number: Audio_Renderer_GetBufferSize_005
 * @tc.desc  : test GetBufferSize interface. Returns 0 {SUCCESS}, if the getting is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetBufferSize_005, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    size_t bufferLen;
    ret = audioRenderer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetAudioStreamId API stability.
 * @tc.number: Audio_Renderer_GetAudioStreamId_001
 * @tc.desc  : Test GetAudioStreamId interface stability.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetAudioStreamId_001, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    uint32_t sessionID;
    ret = audioRenderer->GetAudioStreamId(sessionID);
    EXPECT_EQ(SUCCESS, ret);

    audioRenderer->Release();
}


/**
 * @tc.name  : Test GetFrameCount API via legal input.
 * @tc.number: Audio_Renderer_GetFrameCount_001
 * @tc.desc  : test GetFrameCount interface, Returns 0 {SUCCESS}, if the getting is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetFrameCount_001, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    uint32_t frameCount;
    ret = audioRenderer->GetFrameCount(frameCount);
    EXPECT_EQ(SUCCESS, ret);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetFrameCount API via illegal state, RENDERER_NEW: without initialiing the renderer.
 * @tc.number: Audio_Renderer_GetFrameCount_002
 * @tc.desc  : Test GetFrameCount interface. Returns error code, if the renderer state is RENDERER_NEW.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetFrameCount_002, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    uint32_t frameCount;
    ret = audioRenderer->GetFrameCount(frameCount);
    EXPECT_EQ(RenderUT::VALUE_ZERO, ret);
}

/**
 * @tc.name  : Test GetFrameCount API via legal state, RENDERER_RUNNING: call Start before GetFrameCount.
 * @tc.number: Audio_Renderer_GetFrameCount_003
 * @tc.desc  : Test GetFrameCount interface. Returns 0 {SUCCESS}, if the getting is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetFrameCount_003, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    uint32_t frameCount;
    ret = audioRenderer->GetFrameCount(frameCount);
    EXPECT_EQ(SUCCESS, ret);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetFrameCount API via legal state, RENDERER_STOPPED: call Stop before GetFrameCount
 * @tc.number: Audio_Renderer_GetFrameCount_004
 * @tc.desc  : Test GetFrameCount interface. Returns 0 {SUCCESS}, if the getting is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetFrameCount_004, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    uint32_t frameCount;
    ret = audioRenderer->GetFrameCount(frameCount);
    EXPECT_EQ(SUCCESS, ret);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetFrameCount API via illegal state, RENDERER_RELEASED: call Release before GetFrameCount
 * @tc.number: Audio_Renderer_GetFrameCount_005
 * @tc.desc  : Test GetFrameCount interface.  Returns error code, if the state is RENDERER_RELEASED.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetFrameCount_005, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);

    uint32_t frameCount;
    ret = audioRenderer->GetFrameCount(frameCount);
    EXPECT_EQ(ERR_ILLEGAL_STATE, ret);
}

/**
 * @tc.name  : Test GetFrameCount API via legal state, RENDERER_PAUSED: call Pause before GetFrameCount
 * @tc.number: Audio_Renderer_GetFrameCount_006
 * @tc.desc  : Test GetFrameCount interface. Returns 0 {SUCCESS}, if the getting is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetFrameCount_006, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isPaused = audioRenderer->Pause();
    EXPECT_EQ(true, isPaused);

    uint32_t frameCount;
    ret = audioRenderer->GetFrameCount(frameCount);
    EXPECT_EQ(SUCCESS, ret);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetFrameCount API via legal input when playing audiovivid in callback mode.
 * @tc.number: Audio_Renderer_GetFrameCount_007
 * @tc.desc  : Test GetFrameCount interface, Returns 0 {SUCCESS}, if the getting is successful.
 *             The frame count should be const 1024 in this situation.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetFrameCount_007, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererSpatialOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);
    ret = audioRenderer->SetRenderMode(RENDER_MODE_CALLBACK);
    EXPECT_EQ(SUCCESS, ret);

    uint32_t frameCount;
    ret = audioRenderer->GetFrameCount(frameCount);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(RenderUT::AUDIOVIVID_FRAME_COUNT, frameCount);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetVolume
 * @tc.number: Audio_Renderer_GetVolume_001
 * @tc.desc  : Test GetVolume interface to get the default value.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetVolume_001, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    float volume = audioRenderer->GetVolume();
    EXPECT_EQ(1.0, volume);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
 * @tc.name  : Test GetVolume
 * @tc.number: Audio_Renderer_GetVolume_002
 * @tc.desc  : Test GetVolume interface after set volume call.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetVolume_002, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    ret = audioRenderer->SetVolume(0.5);
    EXPECT_EQ(SUCCESS, ret);

    float volume = audioRenderer->GetVolume();
    EXPECT_EQ(0.5, volume);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
 * @tc.name  : Test GetVolume
 * @tc.number: Audio_Renderer_GetVolume_003
 * @tc.desc  : Test GetVolume interface after set volume fails.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetVolume_003, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    ret = audioRenderer->SetVolume(0.5);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioRenderer->SetVolume(1.5);
    EXPECT_NE(SUCCESS, ret);

    float volume = audioRenderer->GetVolume();
    EXPECT_EQ(0.5, volume);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
 * @tc.name  : Test GetAudioTime API via legal input.
 * @tc.number: Audio_Renderer_GetAudioTime_001
 * @tc.desc  : Test GetAudioTime interface. Returns true, if the getting is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetAudioTime_001, TestSize.Level1)
{
    int32_t ret = -1;
    FILE *wavFile = fopen(RenderUT::AUDIORENDER_TEST_FILE_PATH.c_str(), "rb");
    ASSERT_NE(nullptr, wavFile);

    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    size_t bufferLen;
    ret = audioRenderer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);
    ASSERT_NE(nullptr, buffer);

    size_t bytesToWrite = fread(buffer, 1, bufferLen, wavFile);
    int32_t bytesWritten = audioRenderer->Write(buffer, bytesToWrite);
    EXPECT_GE(bytesWritten, RenderUT::VALUE_ZERO);

    Timestamp timestamp;
    bool getAudioTime = audioRenderer->GetAudioTime(timestamp, Timestamp::Timestampbase::MONOTONIC);
    EXPECT_EQ(true, getAudioTime);
    EXPECT_GE(timestamp.time.tv_sec, (const long)RenderUT::VALUE_ZERO);
    EXPECT_GE(timestamp.time.tv_nsec, (const long)RenderUT::VALUE_ZERO);

    audioRenderer->Drain();
    audioRenderer->Stop();
    audioRenderer->Release();

    free(buffer);
    fclose(wavFile);
}

/**
 * @tc.name  : Test GetAudioTime API via illegal state, RENDERER_NEW: GetAudioTime without initializing the renderer.
 * @tc.number: Audio_Renderer_GetAudioTime_002
 * @tc.desc  : Test GetAudioTime interface. Returns false, if the renderer state is RENDERER_NEW
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetAudioTime_002, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    Timestamp timestamp;
    bool getAudioTime = audioRenderer->GetAudioTime(timestamp, Timestamp::Timestampbase::MONOTONIC);
    EXPECT_EQ(false, getAudioTime);
}

/**
 * @tc.name  : Test GetAudioTime API via legal state, RENDERER_RUNNING.
 * @tc.number: Audio_Renderer_GetAudioTime_003
 * @tc.desc  : test GetAudioTime interface. Returns true, if the getting is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetAudioTime_003, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    Timestamp timestamp;
    bool getAudioTime = audioRenderer->GetAudioTime(timestamp, Timestamp::Timestampbase::MONOTONIC);
    EXPECT_EQ(true, getAudioTime);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetAudioTime API via legal state, RENDERER_STOPPED.
 * @tc.number: Audio_Renderer_GetAudioTime_004
 * @tc.desc  : Test GetAudioTime interface. Returns true, if the getting is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetAudioTime_004, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    Timestamp timestamp;
    bool getAudioTime = audioRenderer->GetAudioTime(timestamp, Timestamp::Timestampbase::MONOTONIC);
    EXPECT_EQ(false, getAudioTime);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetAudioTime API via illegal state, RENDERER_RELEASED: GetAudioTime after Release.
 * @tc.number: Audio_Renderer_GetAudioTime_005
 * @tc.desc  : Test GetAudioTime interface. Returns false, if the renderer state is RENDERER_RELEASED
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetAudioTime_005, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);

    Timestamp timestamp;
    bool getAudioTime = audioRenderer->GetAudioTime(timestamp, Timestamp::Timestampbase::MONOTONIC);
    EXPECT_EQ(false, getAudioTime);
}

/**
 * @tc.name  : Test GetAudioTime API via legal state, RENDERER_PAUSED.
 * @tc.number: Audio_Renderer_GetAudioTime_006
 * @tc.desc  : Test GetAudioTime interface. Returns true, if the getting is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetAudioTime_006, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isPaused = audioRenderer->Pause();
    EXPECT_EQ(true, isPaused);

    Timestamp timestamp;
    bool getAudioTime = audioRenderer->GetAudioTime(timestamp, Timestamp::Timestampbase::MONOTONIC);
    EXPECT_EQ(true, getAudioTime);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetAudioTime API via legal state, RENDERER_PAUSED.
 * @tc.number: Audio_Renderer_GetAudioTime_007
 * @tc.desc  : Test GetAudioTime interface. Timestamp should be larger after pause 1s.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetAudioTime_007, TestSize.Level2)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    size_t bufferSize = 3528; // 44.1 khz, 20ms
    std::unique_ptr<uint8_t[]> tempBuffer = std::make_unique<uint8_t[]>(bufferSize);
    int loopCount = 20; // 400ms
    while (loopCount-- > 0) {
        audioRenderer->Write(tempBuffer.get(), bufferSize);
    }
    Timestamp timestamp1;
    audioRenderer->GetAudioTime(timestamp1, Timestamp::Timestampbase::MONOTONIC);

    bool isPaused = audioRenderer->Pause();
    EXPECT_EQ(true, isPaused);

    size_t sleepTime = 1000000; // sleep 1s
    usleep(sleepTime);

    isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    loopCount = 10; // 200ms
    while (loopCount-- > 0) {
        audioRenderer->Write(tempBuffer.get(), bufferSize);
    }
    Timestamp timestamp2;
    audioRenderer->GetAudioTime(timestamp2, Timestamp::Timestampbase::MONOTONIC);

    int64_t duration = (timestamp2.time.tv_sec - timestamp1.time.tv_sec) * 1000000 + (timestamp2.time.tv_nsec -
        timestamp1.time.tv_nsec) / RenderUT::VALUE_THOUSAND; // ns -> us
    EXPECT_GE(duration, sleepTime);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetRenderRate
 * @tc.number: Audio_Renderer_GetRenderRate_001
 * @tc.desc  : Test GetRenderRate interface after set volume fails.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetRenderRate_001, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    ret = audioRenderer->SetRenderRate(RENDER_RATE_DOUBLE);
    EXPECT_EQ(SUCCESS, ret);

    AudioRendererRate renderRate = audioRenderer->GetRenderRate();
    EXPECT_EQ(RENDER_RATE_DOUBLE, renderRate);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetStatus API.
 * @tc.number: Audio_Renderer_GetStatus_001
 * @tc.desc  : Test GetStatus interface. Returns correct state on success.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetStatus_001, TestSize.Level1)
{
    RendererState state = RENDERER_INVALID;

    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    state = audioRenderer->GetStatus();
    EXPECT_EQ(RENDERER_PREPARED, state);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);
    state = audioRenderer->GetStatus();
    EXPECT_EQ(RENDERER_RUNNING, state);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);
    state = audioRenderer->GetStatus();
    EXPECT_EQ(RENDERER_STOPPED, state);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
    state = audioRenderer->GetStatus();
    EXPECT_EQ(RENDERER_RELEASED, state);
}

/**
 * @tc.name  : Test GetStatus API, call Start without Initializing the renderer
 * @tc.number: Audio_Renderer_GetStatus_002
 * @tc.desc  : Test GetStatus interface. Not changes to RENDERER_RUNNING, if the current state is RENDERER_NEW.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetStatus_002, TestSize.Level1)
{
    RendererState state = RENDERER_INVALID;

    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);
    state = audioRenderer->GetStatus();
    EXPECT_EQ(RENDERER_RUNNING, state);
}

/**
 * @tc.name  : Test GetStatus API, call Stop without Start
 * @tc.number: Audio_Renderer_GetStatus_003
 * @tc.desc  : Test GetStatus interface. Not changes to RENDERER_STOPPED, if the current state is RENDERER_PREPARED.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetStatus_003, TestSize.Level1)
{
    RendererState state = RENDERER_INVALID;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(false, isStopped);
    state = audioRenderer->GetStatus();
    EXPECT_NE(RENDERER_STOPPED, state);
    EXPECT_EQ(RENDERER_PREPARED, state);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetStatus API, call Start, Stop and then Start again
 * @tc.number: Audio_Renderer_GetStatus_004
 * @tc.desc  : Test GetStatus interface.  Returns correct state on success.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetStatus_004, TestSize.Level1)
{
    RendererState state = RENDERER_INVALID;

    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);
    state = audioRenderer->GetStatus();
    EXPECT_EQ(RENDERER_RUNNING, state);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);
    state = audioRenderer->GetStatus();
    EXPECT_EQ(RENDERER_STOPPED, state);

    isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);
    state = audioRenderer->GetStatus();
    EXPECT_EQ(RENDERER_RUNNING, state);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetStatus API, call Release without initializing
 * @tc.number: Audio_Renderer_GetStatus_005
 * @tc.desc  : Test GetStatus interface. Not changes to RENDERER_RELEASED, if the current state is RENDERER_NEW.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetStatus_005, TestSize.Level1)
{
    RendererState state = RENDERER_INVALID;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
    state = audioRenderer->GetStatus();
    EXPECT_EQ(RENDERER_RELEASED, state);
}

/**
 * @tc.name  : Test GetLatency API.
 * @tc.number: Audio_Renderer_GetLatency_001
 * @tc.desc  : Test GetLatency interface. Returns 0 {SUCCESS}, if the getting is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetLatency_001, TestSize.Level1)
{
    int32_t ret = -1;
    FILE *wavFile = fopen(RenderUT::AUDIORENDER_TEST_FILE_PATH.c_str(), "rb");
    ASSERT_NE(nullptr, wavFile);

    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
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
    size_t minBytes = 4;
    int32_t numBuffersToRender = RenderUT::WRITE_BUFFERS_COUNT;

    while (numBuffersToRender) {
        bytesToWrite = fread(buffer, 1, bufferLen, wavFile);
        bytesWritten = 0;
        uint64_t latency;
        ret = audioRenderer->GetLatency(latency);
        EXPECT_EQ(SUCCESS, ret);
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
    audioRenderer->Release();

    free(buffer);
    fclose(wavFile);
}

/**
 * @tc.name  : Test GetLatency API via illegal state, RENDERER_NEW: without initializing the renderer
 * @tc.number: Audio_Renderer_GetLatency_002
 * @tc.desc  : Test GetLatency interface. Returns error code, if the renderer state is RENDERER_NEW.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetLatency_002, TestSize.Level1)
{
    int32_t ret = -1;

    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    uint64_t latency;
    ret = audioRenderer->GetLatency(latency);
    EXPECT_EQ(RenderUT::VALUE_ZERO, ret);
}

/**
 * @tc.name  : Test GetLatency API via legal state, RENDERER_PREPARED
 * @tc.number: Audio_Renderer_GetLatency_003
 * @tc.desc  : Test GetLatency interface. Returns 0 {SUCCESS}, if the getting is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetLatency_003, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    uint64_t latency;
    ret = audioRenderer->GetLatency(latency);
    EXPECT_EQ(SUCCESS, ret);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetLatency API via legal state, RENDERER_STOPPED: After Stop
 * @tc.number: Audio_Renderer_GetLatency_004
 * @tc.desc  : Test GetLatency interface. Returns 0 {SUCCESS}, if the getting is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetLatency_004, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    uint64_t latency;
    ret = audioRenderer->GetLatency(latency);
    EXPECT_EQ(SUCCESS, ret);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetLatency API via illegal state, RENDERER_RELEASED: After Release
 * @tc.number: Audio_Renderer_GetLatency_005
 * @tc.desc  : Test GetLatency interface. Returns error code, if the renderer state is RENDERER_RELEASED.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetLatency_005, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);

    uint64_t latency;
    ret = audioRenderer->GetLatency(latency);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test GetRenderMode with, RENDER_MODE_CALLBACK
 * @tc.number: Audio_Renderer_GetRenderMode_001
 * @tc.desc  : Test GetRenderMode interface. Returns the current render mode.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetRenderMode_001, TestSize.Level1)
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

    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetRenderMode with, RENDER_MODE_NORMAL
 * @tc.number: Audio_Renderer_GetRenderMode_002
 * @tc.desc  : Test GetRenderMode interface. Returns the current render mode.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetRenderMode_002, TestSize.Level1)
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

    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetRenderMode with, default renderMode
 * @tc.number: Audio_Renderer_GetRenderMode_003
 * @tc.desc  : Test GetRenderMode interface. Returns the default render mode RENDER_MODE_NORMAL.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetRenderMode_003, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    AudioRenderMode renderMode = audioRenderer->GetRenderMode();
    EXPECT_EQ(RENDER_MODE_NORMAL, renderMode);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetBufferDesc via legal render mode, RENDER_MODE_CALLBACK
 * @tc.number: Audio_Renderer_GetBufferDesc_001
 * @tc.desc  : Test GetBufferDesc interface. Returns SUCCESS, if BufferDesc obtained successfully.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetBufferDesc_001, TestSize.Level1)
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

    BufferDesc bufDesc {};
    bufDesc.buffer = nullptr;
    bufDesc.dataLength = RenderUT::g_reqBufLen;
    ret = audioRenderer->GetBufferDesc(bufDesc);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_NE(nullptr, bufDesc.buffer);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetBufferDesc via illegal render mode, RENDER_MODE_NORMAL
 * @tc.number: Audio_Renderer_GetBufferDesc_002
 * @tc.desc  : Test GetBufferDesc interface. Returns errorcode, if render mode is not callback.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetBufferDesc_002, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    shared_ptr<AudioRendererWriteCallback> cb = make_shared<AudioRenderModeCallbackTest>();

    ret = audioRenderer->SetRendererWriteCallback(cb);
    EXPECT_EQ(ERR_INCORRECT_MODE, ret);

    BufferDesc bufDesc {};
    bufDesc.buffer = nullptr;
    bufDesc.dataLength = RenderUT::g_reqBufLen;
    ret = audioRenderer->GetBufferDesc(bufDesc);
    EXPECT_EQ(ERR_INCORRECT_MODE, ret);
    EXPECT_EQ(nullptr, bufDesc.buffer);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetRendererInfo API after calling create
 * @tc.number: Audio_Renderer_GetRendererInfo_001
 * @tc.desc  : Test GetRendererInfo interface. Check whether renderer info returns proper data
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetRendererInfo_001, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_96000;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_U8;
    rendererOptions.streamInfo.channels = AudioChannel::MONO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    rendererOptions.rendererInfo.rendererFlags = RenderUT::RENDERER_FLAG;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_TRUE(audioRenderer != nullptr);

    AudioRendererInfo rendererInfo;
    audioRenderer->GetRendererInfo(rendererInfo);

    EXPECT_EQ(ContentType::CONTENT_TYPE_MUSIC, rendererInfo.contentType);
    EXPECT_EQ(StreamUsage::STREAM_USAGE_MEDIA, rendererInfo.streamUsage);
    EXPECT_EQ(RenderUT::RENDERER_FLAG, rendererInfo.rendererFlags);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetRendererInfo API via legal state, RENDERER_RUNNING: GetRendererInfo after Start.
 * @tc.number: Audio_Renderer_GetRendererInfo_002
 * @tc.desc  : Test GetRendererInfo interface. Check whether renderer info returns proper data
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetRendererInfo_002, TestSize.Level1)
{
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

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    AudioRendererInfo rendererInfo;
    audioRenderer->GetRendererInfo(rendererInfo);

    EXPECT_EQ(ContentType::CONTENT_TYPE_MUSIC, rendererInfo.contentType);
    EXPECT_EQ(StreamUsage::STREAM_USAGE_MEDIA, rendererInfo.streamUsage);
    EXPECT_EQ(RenderUT::RENDERER_FLAG, rendererInfo.rendererFlags);

    audioRenderer->Stop();
    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetRendererInfo API via legal state, RENDERER_RELEASED: Call GetRendererInfo after Release.
 * @tc.number: Audio_Renderer_GetRendererInfo_003
 * @tc.desc  : Test GetRendererInfo interface. Check whether renderer info returns proper data
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetRendererInfo_003, TestSize.Level1)
{
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

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);

    AudioRendererInfo rendererInfo;
    audioRenderer->GetRendererInfo(rendererInfo);

    EXPECT_EQ(ContentType::CONTENT_TYPE_MUSIC, rendererInfo.contentType);
    EXPECT_EQ(StreamUsage::STREAM_USAGE_MEDIA, rendererInfo.streamUsage);
    EXPECT_EQ(RenderUT::RENDERER_FLAG, rendererInfo.rendererFlags);
}

/**
 * @tc.name  : Test GetRendererInfo API via legal state, RENDERER_STOPPED: Call GetRendererInfo after Stop.
 * @tc.number: Audio_Renderer_GetRendererInfo_004
 * @tc.desc  : Test GetRendererInfo interface. Check whether renderer info returns proper data
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetRendererInfo_004, TestSize.Level1)
{
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

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    AudioRendererInfo rendererInfo;
    audioRenderer->GetRendererInfo(rendererInfo);

    EXPECT_EQ(ContentType::CONTENT_TYPE_MUSIC, rendererInfo.contentType);
    EXPECT_EQ(StreamUsage::STREAM_USAGE_MEDIA, rendererInfo.streamUsage);
    EXPECT_EQ(RenderUT::RENDERER_FLAG, rendererInfo.rendererFlags);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetRendererInfo API Stability
 * @tc.number: Audio_Renderer_GetRendererInfo_Stability_001
 * @tc.desc  : Test GetRendererInfo interface Stability
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetRendererInfo_Stability_001, TestSize.Level1)
{
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

    for (int i = 0; i < RenderUT::VALUE_THOUSAND; i++) {

        AudioRendererInfo rendererInfo;
        audioRenderer->GetRendererInfo(rendererInfo);

        EXPECT_EQ(ContentType::CONTENT_TYPE_MUSIC, rendererInfo.contentType);
        EXPECT_EQ(StreamUsage::STREAM_USAGE_MEDIA, rendererInfo.streamUsage);
        EXPECT_EQ(RenderUT::RENDERER_FLAG, rendererInfo.rendererFlags);
    }
    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetStreamInfo API after calling create
 * @tc.number: Audio_Renderer_GetStreamInfo_001
 * @tc.desc  : Test GetStreamInfo interface. Check whether stream related data is returned correctly
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetStreamInfo_001, TestSize.Level1)
{
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

    AudioStreamInfo streamInfo;
    audioRenderer->GetStreamInfo(streamInfo);

    EXPECT_EQ(AudioSamplingRate::SAMPLE_RATE_96000, streamInfo.samplingRate);
    EXPECT_EQ(AudioEncodingType::ENCODING_PCM, streamInfo.encoding);
    EXPECT_EQ(AudioSampleFormat::SAMPLE_U8, streamInfo.format);
    EXPECT_EQ(AudioChannel::MONO, streamInfo.channels);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetStreamInfo via legal state,  RENDERER_RUNNING: GetStreamInfo after Start.
 * @tc.number: Audio_Renderer_GetStreamInfo_002
 * @tc.desc  : Test GetStreamInfo interface. Check whether stream related data is returned correctly
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetStreamInfo_002, TestSize.Level1)
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

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    AudioStreamInfo streamInfo;
    ret = audioRenderer->GetStreamInfo(streamInfo);

    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(AudioSamplingRate::SAMPLE_RATE_96000, streamInfo.samplingRate);
    EXPECT_EQ(AudioEncodingType::ENCODING_PCM, streamInfo.encoding);
    EXPECT_EQ(AudioSampleFormat::SAMPLE_U8, streamInfo.format);
    EXPECT_EQ(AudioChannel::MONO, streamInfo.channels);

    audioRenderer->Stop();
    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetStreamInfo via illegal state, RENDERER_RELEASED: GetStreamInfo after Release.
 * @tc.number: Audio_Renderer_GetStreamInfo_003
 * @tc.desc  : Test GetStreamInfo interface. Returns error code, if the renderer state is RENDERER_RELEASED.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetStreamInfo_003, TestSize.Level1)
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

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);

    AudioStreamInfo streamInfo;
    ret = audioRenderer->GetStreamInfo(streamInfo);

    EXPECT_EQ(ERR_OPERATION_FAILED, ret);
}

/**
 * @tc.name  : Test GetStreamInfo via legal state, RENDERER_STOPPED: GetStreamInfo after Stop.
 * @tc.number: Audio_Renderer_GetStreamInfo_004
 * @tc.desc  : Test GetStreamInfo interface. Check whether stream related data is returned correctly
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetStreamInfo_004, TestSize.Level1)
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

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    AudioStreamInfo streamInfo;
    ret = audioRenderer->GetStreamInfo(streamInfo);

    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(AudioSamplingRate::SAMPLE_RATE_96000, streamInfo.samplingRate);
    EXPECT_EQ(AudioEncodingType::ENCODING_PCM, streamInfo.encoding);
    EXPECT_EQ(AudioSampleFormat::SAMPLE_U8, streamInfo.format);
    EXPECT_EQ(AudioChannel::MONO, streamInfo.channels);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetStreamInfo via legal state, RENDERER_PAUSED: GetStreamInfo after Pause.
 * @tc.number: Audio_Renderer_GetStreamInfo_005
 * @tc.desc  : Test GetStreamInfo interface. Check whether stream related data is returned correctly
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetStreamInfo_005, TestSize.Level1)
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

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isPaused = audioRenderer->Pause();
    EXPECT_EQ(true, isPaused);

    AudioStreamInfo streamInfo;
    ret = audioRenderer->GetStreamInfo(streamInfo);

    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(AudioSamplingRate::SAMPLE_RATE_96000, streamInfo.samplingRate);
    EXPECT_EQ(AudioEncodingType::ENCODING_PCM, streamInfo.encoding);
    EXPECT_EQ(AudioSampleFormat::SAMPLE_U8, streamInfo.format);
    EXPECT_EQ(AudioChannel::MONO, streamInfo.channels);

    audioRenderer->Stop();
    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetAudioEffectMode with, EFFECT_NONE
 * @tc.number: Audio_Renderer_GetAudioEffectMode_001
 * @tc.desc  : Test GetAudioEffectMode interface. Returns the current effect mode.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetAudioEffectMode_001, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    ret = audioRenderer->SetAudioEffectMode(EFFECT_NONE);
    EXPECT_EQ(SUCCESS, ret);

    AudioEffectMode effectMode = audioRenderer->GetAudioEffectMode();
    EXPECT_EQ(EFFECT_NONE, effectMode);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetAudioEffectMode with, EFFECT_DEFAULT
 * @tc.number: Audio_Renderer_GetAudioEffectMode_002
 * @tc.desc  : Test GetAudioEffectMode interface. Returns the current effect mode.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetAudioEffectMode_002, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    ret = audioRenderer->SetAudioEffectMode(EFFECT_DEFAULT);
    EXPECT_EQ(SUCCESS, ret);

    AudioEffectMode effectMode = audioRenderer->GetAudioEffectMode();
    EXPECT_EQ(EFFECT_DEFAULT, effectMode);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetAudioEffectMode with, default effectMode
 * @tc.number: Audio_Renderer_GetAudioEffectMode_003
 * @tc.desc  : Test GetAudioEffectMode interface. Returns the default effect mode EFFECT_DEFAULT.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetAudioEffectMode_003, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    AudioEffectMode effectMode = audioRenderer->GetAudioEffectMode();
    EXPECT_EQ(EFFECT_DEFAULT, effectMode);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetMinStreamVolume
 * @tc.number: Audio_Renderer_GetMinStreamVolume_001
 * @tc.desc  : Test GetMinStreamVolume interface to get the min volume value.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetMinStreamVolume_001, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    float volume = audioRenderer->GetMinStreamVolume();
    EXPECT_EQ(0.0, volume);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
 * @tc.name  : Test GetMinStreamVolume
 * @tc.number: Audio_Renderer_GetMinStreamVolume_Stability_001
 * @tc.desc  : Test GetMinStreamVolume interface to get the min volume value for 1000 times.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetMinStreamVolume_Stability_001, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    for (int i = 0; i < RenderUT::VALUE_THOUSAND; i++) {
        float volume = audioRenderer->GetMinStreamVolume();
        EXPECT_EQ(0.0, volume);
    }

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
 * @tc.name  : Test GetMaxStreamVolume
 * @tc.number: Audio_Renderer_GetMaxStreamVolume_001
 * @tc.desc  : Test GetMaxStreamVolume interface to get the max volume value.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetMaxStreamVolume_001, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    float volume = audioRenderer->GetMaxStreamVolume();
    EXPECT_EQ(1.0, volume);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
 * @tc.name  : Test GetMaxStreamVolume
 * @tc.number: Audio_Renderer_GetMaxStreamVolume_Stability_001
 * @tc.desc  : Test GetMaxStreamVolume interface to get the max volume value for 1000 times.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetMaxStreamVolume_Stability_001, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    for (int i = 0; i < RenderUT::VALUE_THOUSAND; i++) {
        float volume = audioRenderer->GetMaxStreamVolume();
        EXPECT_EQ(1.0, volume);
    }

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
 * @tc.name  : Test GetUnderflowCount
 * @tc.number: Audio_Renderer_GetUnderflowCount_001
 * @tc.desc  : Test GetUnderflowCount interface get underflow value.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetUnderflowCount_001, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    ret = audioRenderer->GetUnderflowCount();
    EXPECT_GE(ret, SUCCESS);

    audioRenderer->Release();
}


/**
 * @tc.name  : Test GetUnderflowCount
 * @tc.number: Audio_Renderer_GetUnderflowCount_002
 * @tc.desc  : Test GetUnderflowCount interface get underflow value.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetUnderflowCount_002, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    // Use the STREAM_USAGE_VOICE_COMMUNICATION to prevent entering offload mode, as offload does not support underflow.
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_UNKNOWN;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_VOICE_COMMUNICATION;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    size_t bufferSize;
    int32_t ret = audioRenderer->GetBufferSize(bufferSize);
    EXPECT_EQ(ret, SUCCESS);

    auto buffer = std::make_unique<uint8_t[]>(bufferSize);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    ret = audioRenderer->Write(buffer.get(), bufferSize);

    std::this_thread::sleep_for(1s);
    auto underFlowCount = audioRenderer->GetUnderflowCount();

    // Ensure the underflowCount is at least 1
    EXPECT_GE(underFlowCount, 1);

    audioRenderer->Stop();
    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetUnderflowCount
 * @tc.number: Audio_Renderer_GetUnderflowCount_004
 * @tc.desc  : Test GetUnderflowCount interface get underflow value.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetUnderflowCount_004, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    // Use the STREAM_USAGE_VOICE_COMMUNICATION to prevent entering offload mode, as offload does not support underflow.
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_UNKNOWN;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_VOICE_COMMUNICATION;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    ret = audioRenderer->SetRenderMode(RENDER_MODE_CALLBACK);
    EXPECT_EQ(SUCCESS, ret);

    shared_ptr<AudioRendererWriteCallbackMock> cb = make_shared<AudioRendererWriteCallbackMock>();

    ret = audioRenderer->SetRendererWriteCallback(cb);
    EXPECT_EQ(SUCCESS, ret);

    int32_t count = 0;
    cb->Install([&count, &audioRenderer](size_t length) {
                // only execute once
                if (count++ > 0) {
                    return;
                }
                BufferDesc bufDesc {};
                bufDesc.buffer = nullptr;
                bufDesc.dataLength = RenderUT::g_reqBufLen;
                auto ret = audioRenderer->GetBufferDesc(bufDesc);
                EXPECT_EQ(SUCCESS, ret);
                EXPECT_NE(nullptr, bufDesc.buffer);
                audioRenderer->Enqueue(bufDesc);
                });

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    std::this_thread::sleep_for(1s);

    // Verify that the callback is invoked at least once
    EXPECT_GE(cb->GetExeCount(), 1);

    auto underFlowCount = audioRenderer->GetUnderflowCount();

    // Ensure the underflowCount is at least 1
    EXPECT_GE(underFlowCount, 1);

    audioRenderer->Stop();
    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetStreamInfo API stability.
 * @tc.number: Audio_Renderer_GetStreamInfo_Stability_001
 * @tc.desc  : Test GetStreamInfo interface stability
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetStreamInfo_Stability_001, TestSize.Level1)
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

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);


    for (int i = 0; i < RenderUT::VALUE_THOUSAND; i++) {
        AudioStreamInfo streamInfo;
        ret = audioRenderer->GetStreamInfo(streamInfo);
        EXPECT_EQ(SUCCESS, ret);
    }

    audioRenderer->Stop();
    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetUnderflowCount
 * @tc.number: Audio_Renderer_GetUnderflowCount_Stability_001
 * @tc.desc  : Test GetUnderflowCount interface get underflow value for 1000 times.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetUnderflowCount_Stability_001, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    for (int i = 0; i < RenderUT::VALUE_THOUSAND; i++) {
        ret = audioRenderer->GetUnderflowCount();
        EXPECT_GE(ret, SUCCESS);
    }

    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetRendererSamplingRate
 * @tc.number: Audio_Renderer_GetRendererSamplingRate_001
 * @tc.desc  : Test GetRendererSamplingRate get default samplingRate.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetRendererSamplingRate_001, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    uint32_t ret = audioRenderer->GetRendererSamplingRate();
    EXPECT_EQ(SAMPLE_RATE_44100, ret);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetRendererSamplingRate
 * @tc.number: Audio_Renderer_GetRendererSamplingRate_002
 * @tc.desc  : Test GetRendererSamplingRate get valid samplingRate after set valid samplingRate.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetRendererSamplingRate_002, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    uint32_t samplingRate = 48000;
    ret = audioRenderer->SetRendererSamplingRate(samplingRate);
    EXPECT_EQ(ERROR, ret);

    uint32_t retSamplerate = audioRenderer->GetRendererSamplingRate();
    EXPECT_EQ(SAMPLE_RATE_44100, retSamplerate);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetRendererSamplingRate
 * @tc.number: Audio_Renderer_GetRendererSamplingRate_003
 * @tc.desc  : Test GetRendererSamplingRate get default samplingRate after set invalid samplingRate.
 */

HWTEST(AudioRendererUnitTest, Audio_Renderer_GetRendererSamplingRate_003, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    uint32_t samplingRate = 0;
    ret = audioRenderer->SetRendererSamplingRate(samplingRate);
    EXPECT_EQ(ERROR, ret);

    uint32_t retSamplerate = audioRenderer->GetRendererSamplingRate();
    EXPECT_EQ(SAMPLE_RATE_44100, retSamplerate);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetRendererSamplingRate
 * @tc.number: Audio_Renderer_GetRendererSamplingRate_004
 * @tc.desc  : Test GetRendererSamplingRate get valid samplingRate after set invalid samplingRate.
 */

HWTEST(AudioRendererUnitTest, Audio_Renderer_GetRendererSamplingRate_004, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    uint32_t validRate = 48000;
    ret = audioRenderer->SetRendererSamplingRate(validRate);
    EXPECT_EQ(ERROR, ret);

    uint32_t invalidRate = 0;
    ret = audioRenderer->SetRendererSamplingRate(invalidRate);
    EXPECT_EQ(ERROR, ret);

    uint32_t retSampleRate = audioRenderer->GetRendererSamplingRate();
    EXPECT_EQ(SAMPLE_RATE_44100, retSampleRate);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetRendererSamplingRate
 * @tc.number: Audio_Renderer_GetRendererSamplingRate_Stability_001
 * @tc.desc  : Test GetRendererSamplingRate get valid samplingRate 1000 times after set valid samplingRate.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetRendererSamplingRate_Stability_001, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    for (int i = 0; i < RenderUT::VALUE_THOUSAND; i++) {
        uint32_t samplingRate = 48000;
        ret = audioRenderer->SetRendererSamplingRate(samplingRate);
        EXPECT_EQ(ERROR, ret);

        uint32_t retSampleRate = audioRenderer->GetRendererSamplingRate();
        EXPECT_EQ(SAMPLE_RATE_44100, retSampleRate);
    }

    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetCurrentOutputDevices API after calling create
 * @tc.number: Audio_Renderer_GetCurrentOutputDevices_001
 * @tc.desc  : Test GetCurrentOutputDevices interface. Check whether renderer info returns proper data
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetCurrentOutputDevices_001, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    EXPECT_NE(nullptr, audioRenderer);

    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    ret = audioRenderer->GetCurrentOutputDevices(deviceInfo);
    EXPECT_EQ(SUCCESS, ret);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetCurrentOutputDevices API after calling create
 * @tc.number: Audio_Renderer_GetCurrentOutputDevices_002
 * @tc.desc  : Test GetCurrentOutputDevices interface.Check the deviceinfo is proper data when using speaker.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetCurrentOutputDevices_002, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    EXPECT_NE(nullptr, audioRenderer);

    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    audioRenderer->GetCurrentOutputDevices(deviceInfo);

    EXPECT_EQ(OUTPUT_DEVICE, deviceInfo.deviceRole_);
    EXPECT_EQ(DEVICE_TYPE_SPEAKER, deviceInfo.deviceType_);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetCurrentOutputDevices API after calling create
 * @tc.number: Audio_Renderer_GetCurrentOutputDevices_001
 * @tc.desc  : Test GetCurrentOutputDevices interface check if it is success for 1000 times
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetCurrentOutputDevices_Stability_001, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    EXPECT_NE(nullptr, audioRenderer);

    for (int i = 0; i < RenderUT::VALUE_THOUSAND; i++) {
        AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
        audioRenderer->GetCurrentOutputDevices(deviceInfo);

        EXPECT_EQ(OUTPUT_DEVICE, deviceInfo.deviceRole_);
        EXPECT_EQ(DEVICE_TYPE_SPEAKER, deviceInfo.deviceType_);
    }

    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetCurrentOutputDevices API after calling create
 * @tc.number: Audio_Renderer_GetCurrentOutputDevices_001
 * @tc.desc  : Test GetCurrentOutputDevices interface check proper data when using speaker for 1000 times
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetCurrentOutputDevices_Stability_002, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    EXPECT_NE(nullptr, audioRenderer);

    for (int i = 0; i < RenderUT::VALUE_THOUSAND; i++) {
        AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
        audioRenderer->GetCurrentOutputDevices(deviceInfo);

        EXPECT_EQ(OUTPUT_DEVICE, deviceInfo.deviceRole_);
        EXPECT_EQ(DEVICE_TYPE_SPEAKER, deviceInfo.deviceType_);
    }

    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetFramesWritten API after calling create
 * @tc.number: Audio_Renderer_GetFramesWritten_001
 * @tc.desc  : Test GetFramesWritten interface.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetFramesWritten_001, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    EXPECT_NE(nullptr, audioRenderer);

    ret = audioRenderer->GetFramesWritten();
    EXPECT_EQ(SUCCESS, ret);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetSpeed
 * @tc.number: Audio_Renderer_GetSpeed_001
 * @tc.desc  : Test GetSpeed interface.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetSpeed_001, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    float speed = audioRenderer->GetSpeed();
    EXPECT_EQ(1.0, speed);

    ret = audioRenderer->SetSpeed(4.0);
    EXPECT_EQ(SUCCESS, ret);

    speed = audioRenderer->GetSpeed();
    EXPECT_EQ(4.0, speed);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
}

/*
 * @tc.name  : Test GetAudioPosition API via legal input.
 * @tc.number: Audio_Renderer_GetAudioPosition_001
 * @tc.desc  : Test GetAudioPosition interface. Returns true, if the getting is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetAudioPosition_001, TestSize.Level1)
{
    int32_t ret = -1;
    FILE *wavFile = fopen(RenderUT::AUDIORENDER_TEST_FILE_PATH.c_str(), "rb");
    ASSERT_NE(nullptr, wavFile);

    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    size_t bufferLen;
    ret = audioRenderer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);
    ASSERT_NE(nullptr, buffer);

    size_t bytesToWrite = fread(buffer, 1, bufferLen, wavFile);
    int32_t bytesWritten = audioRenderer->Write(buffer, bytesToWrite);
    EXPECT_GE(bytesWritten, RenderUT::VALUE_ZERO);

    Timestamp timestamp;
    bool getAudioPositionRet = audioRenderer->GetAudioPosition(timestamp, Timestamp::Timestampbase::MONOTONIC);
    EXPECT_EQ(true, getAudioPositionRet);
    EXPECT_GE(timestamp.time.tv_sec, (const long)RenderUT::VALUE_ZERO);
    EXPECT_GE(timestamp.time.tv_nsec, (const long)RenderUT::VALUE_ZERO);
    getAudioPositionRet = audioRenderer->GetAudioPosition(timestamp, Timestamp::Timestampbase::BOOTTIME);
    EXPECT_EQ(true, getAudioPositionRet);
    EXPECT_GE(timestamp.time.tv_sec, (const long)RenderUT::VALUE_ZERO);
    EXPECT_GE(timestamp.time.tv_nsec, (const long)RenderUT::VALUE_ZERO);

    audioRenderer->Drain();
    audioRenderer->Stop();
    audioRenderer->Release();

    free(buffer);
    fclose(wavFile);
}

/**
 * @tc.name  : Test GetAudioPosition API via illegal state, RENDERER_NEW: GetAudioPosition without initializing
 * the renderer.
 * @tc.number: Audio_Renderer_GetAudioPosition_002
 * @tc.desc  : Test GetAudioPosition interface. Returns false, if the renderer state is RENDERER_NEW
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetAudioPosition_002, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    Timestamp timestamp;
    bool ret = audioRenderer->GetAudioPosition(timestamp, Timestamp::Timestampbase::MONOTONIC);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name  : Test GetAudioPosition API via legal state, RENDERER_RUNNING.
 * @tc.number: Audio_Renderer_GetAudioPosition_003
 * @tc.desc  : test GetAudioPosition interface. Returns true, if the getting is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetAudioPosition_003, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    Timestamp timestamp;
    bool ret = audioRenderer->GetAudioPosition(timestamp, Timestamp::Timestampbase::MONOTONIC);
    EXPECT_EQ(true, ret);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetAudioPosition API via illegal state, RENDERER_STOPPED: GetAudioPosition after Stop.
 * @tc.number: Audio_Renderer_GetAudioPosition_004
 * @tc.desc  : Test GetAudioPosition interface. Returns false, if the renderer state is RENDERER_STOPPED.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetAudioPosition_004, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    Timestamp timestamp;
    bool ret = audioRenderer->GetAudioPosition(timestamp, Timestamp::Timestampbase::MONOTONIC);
    EXPECT_EQ(false, ret);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetAudioPosition API via illegal state, RENDERER_RELEASED: GetAudioPosition after Release.
 * @tc.number: Audio_Renderer_GetAudioPosition_005
 * @tc.desc  : Test GetAudioPosition interface. Returns false, if the renderer state is RENDERER_RELEASED
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetAudioPosition_005, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);

    Timestamp timestamp;
    bool ret = audioRenderer->GetAudioPosition(timestamp, Timestamp::Timestampbase::MONOTONIC);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name  : Test GetAudioPosition API via illegal state, RENDERER_PAUSED: GetAudioPosition after Stop.
 * @tc.number: Audio_Renderer_GetAudioPosition_006
 * @tc.desc  : Test GetAudioPosition interface. Returns false, if the renderer state is RENDERER_PAUSED.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetAudioPosition_006, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isPaused = audioRenderer->Pause();
    EXPECT_EQ(true, isPaused);

    Timestamp timestamp;
    bool ret = audioRenderer->GetAudioPosition(timestamp, Timestamp::Timestampbase::MONOTONIC);
    EXPECT_EQ(false, ret);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetAudioPosition API via legal state, RENDERER_PAUSED.
 * @tc.number: Audio_Renderer_GetAudioPosition_007
 * @tc.desc  : Test GetAudioPosition interface. Timestamp should be larger after pause 1s.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetAudioPosition_007, TestSize.Level2)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    size_t bufferSize = 3528; // 44.1 khz, 20ms
    std::unique_ptr<uint8_t[]> tempBuffer = std::make_unique<uint8_t[]>(bufferSize);
    int loopCount = 20; // 400ms
    while (loopCount-- > 0) {
        audioRenderer->Write(tempBuffer.get(), bufferSize);
    }
    Timestamp timestamp1;
    audioRenderer->GetAudioPosition(timestamp1, Timestamp::Timestampbase::MONOTONIC);

    bool isPaused = audioRenderer->Pause();
    EXPECT_EQ(true, isPaused);

    size_t sleepTime = 1000000; // sleep 1s
    usleep(sleepTime);

    isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    loopCount = 10; // 200ms
    while (loopCount-- > 0) {
        audioRenderer->Write(tempBuffer.get(), bufferSize);
    }
    Timestamp timestamp2;
    audioRenderer->GetAudioPosition(timestamp2, Timestamp::Timestampbase::MONOTONIC);

    int64_t duration = (timestamp2.time.tv_sec - timestamp1.time.tv_sec) * 1000000 + (timestamp2.time.tv_nsec -
        timestamp1.time.tv_nsec) / RenderUT::VALUE_THOUSAND; // ns -> us
    EXPECT_GE(duration, sleepTime);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetSilentModeAndMixWithOthers
 * @tc.number: Audio_Renderer_GetSilentModeAndMixWithOthers_001
 * @tc.desc  : Test GetSpeed interface.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetSilentModeAndMixWithOthers_001, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool on = audioRenderer->GetSilentModeAndMixWithOthers();
    EXPECT_EQ(false, on);

    audioRenderer->SetSilentModeAndMixWithOthers(true);

    on = audioRenderer->GetSilentModeAndMixWithOthers();
    EXPECT_EQ(true, on);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
 * @tc.name  : Test GetFormatSize
 * @tc.number: GetFormatSize
 * @tc.desc  : Test GetFormatSize
 */
HWTEST(AudioRendererUnitTest, GetFormatSize_001, TestSize.Level1)
{
    AudioStreamParams params;
    params.format = SAMPLE_U8;
    const AudioStreamParams info_1 = params;
    size_t ret = GetFormatSize(info_1);
    EXPECT_EQ(ret, 1);

    params.format = SAMPLE_S16LE;
    const AudioStreamParams info_2 = params;
    ret = GetFormatSize(info_2);
    EXPECT_EQ(ret, 2);

    params.format = SAMPLE_S24LE;
    const AudioStreamParams info_3 = params;
    ret = GetFormatSize(info_3);
    EXPECT_EQ(ret, 3);

    params.format = SAMPLE_S32LE;
    const AudioStreamParams info_4 = params;
    ret = GetFormatSize(info_4);
    EXPECT_EQ(ret, 4);

    params.format = INVALID_WIDTH;
    const AudioStreamParams info_5 = params;
    ret = GetFormatSize(info_5);
    EXPECT_EQ(ret, 2);

    params.format = SAMPLE_F32LE;
    const AudioStreamParams info_6 = params;
    ret = GetFormatSize(info_6);
    EXPECT_EQ(ret, 4);
}

/**
 * @tc.name  : Test GetPreferredStreamClass
 * @tc.number: GetPreferredStreamClass
 * @tc.desc  : Test GetPreferredStreamClass
 */
HWTEST(AudioRendererUnitTest, GetPreferredStreamClass_001, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    AudioStreamParams audioStreamParams;
    audioStreamParams.samplingRate = SAMPLE_RATE_64000;
    audioRendererPrivate->rendererInfo_.originalFlag = AUDIO_FLAG_MMAP;

    audioRendererPrivate->GetPreferredStreamClass(audioStreamParams);
    EXPECT_EQ(audioRendererPrivate->rendererInfo_.rendererFlags, AUDIO_FLAG_NORMAL);
}

/**
 * @tc.name  : Test GetStreamInfo
 * @tc.number: GetStreamInfo
 * @tc.desc  : Test GetStreamInfo
 */
HWTEST(AudioRendererUnitTest, GetStreamInfo_001, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    AudioStreamInfo streamInfo;

    int32_t ret = audioRendererPrivate->GetStreamInfo(streamInfo);
    EXPECT_EQ(ret, -62980101);
}

/**
 * @tc.name  : Test AudioRendererPrivate
 * @tc.number: GetSourceDuration
 * @tc.desc  : Test GetSourceDuration API
 */
HWTEST(AudioRendererUnitTest, GetSourceDuration_001, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);

    audioRendererPrivate->sourceDuration_ = 1;
    EXPECT_EQ(audioRendererPrivate->GetSourceDuration(), 1);
}

/*
 * @tc.name  : Test GetAudioTimestampInfo API via legal input.
 * @tc.number: Audio_Renderer_GetAudioTimestampInfo_001
 * @tc.desc  : Test GetAudioTimestampInfo interface. Returns true, if the getting is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetAudioTimestampInfo_001, TestSize.Level1)
{
    int32_t ret = -1;
    FILE *wavFile = fopen(RenderUT::AUDIORENDER_TEST_FILE_PATH.c_str(), "rb");
    ASSERT_NE(nullptr, wavFile);

    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    size_t bufferLen;
    ret = audioRenderer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);
    ASSERT_NE(nullptr, buffer);

    EXPECT_EQ(SUCCESS, audioRenderer->SetSpeed(2.0));

    size_t bytesToWrite = fread(buffer, 1, bufferLen, wavFile);
    int32_t bytesWritten = audioRenderer->Write(buffer, bytesToWrite);
    EXPECT_GE(bytesWritten, RenderUT::VALUE_ZERO);

    Timestamp timestamp;
    int32_t getAudioTimestampInfoRet =
        audioRenderer->GetAudioTimestampInfo(timestamp, Timestamp::Timestampbase::MONOTONIC);
    EXPECT_EQ(SUCCESS, getAudioTimestampInfoRet);
    EXPECT_GE(timestamp.time.tv_sec, (const long)RenderUT::VALUE_ZERO);
    EXPECT_GE(timestamp.time.tv_nsec, (const long)RenderUT::VALUE_ZERO);
    getAudioTimestampInfoRet =
        audioRenderer->GetAudioTimestampInfo(timestamp, Timestamp::Timestampbase::BOOTTIME);
    EXPECT_EQ(SUCCESS, getAudioTimestampInfoRet);
    EXPECT_GE(timestamp.time.tv_sec, (const long)RenderUT::VALUE_ZERO);
    EXPECT_GE(timestamp.time.tv_nsec, (const long)RenderUT::VALUE_ZERO);

    audioRenderer->Drain();
    audioRenderer->Stop();
    audioRenderer->Release();

    free(buffer);
    fclose(wavFile);
}

/**
 * @tc.name  : Test GetAudioTimestampInfo API via illegal state, RENDERER_NEW: GetAudioTimestampInfo without
 *             initializing the renderer.
 * @tc.number: Audio_Renderer_GetAudioTimestampInfo_002
 * @tc.desc  : Test GetAudioTimestampInfo interface. Returns false, if the renderer state is RENDERER_NEW
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetAudioTimestampInfo_002, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    Timestamp timestamp;
    int32_t ret = audioRenderer->GetAudioTimestampInfo(timestamp, Timestamp::Timestampbase::MONOTONIC);
    EXPECT_EQ(ERR_ILLEGAL_STATE, ret);
}

/**
 * @tc.name  : Test GetAudioTimestampInfo API via legal state, RENDERER_RUNNING.
 * @tc.number: Audio_Renderer_GetAudioTimestampInfo_003
 * @tc.desc  : test GetAudioTimestampInfo interface. Returns true, if the getting is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetAudioTimestampInfo_003, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    Timestamp timestamp;
    int32_t ret = audioRenderer->GetAudioTimestampInfo(timestamp, Timestamp::Timestampbase::MONOTONIC);
    EXPECT_EQ(SUCCESS, ret);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetAudioTimestampInfo API via illegal state, RENDERER_STOPPED: GetAudioTimestampInfo after Stop.
 * @tc.number: Audio_Renderer_GetAudioTimestampInfo_004
 * @tc.desc  : Test GetAudioTimestampInfo interface. Returns false, if the renderer state is RENDERER_STOPPED.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetAudioTimestampInfo_004, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    Timestamp timestamp;
    int32_t ret = audioRenderer->GetAudioTimestampInfo(timestamp, Timestamp::Timestampbase::MONOTONIC);
    EXPECT_EQ(ERR_ILLEGAL_STATE, ret);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetAudioTimestampInfo API via illegal state, RENDERER_RELEASED: GetAudioTimestampInfo after Release.
 * @tc.number: Audio_Renderer_GetAudioTimestampInfo_005
 * @tc.desc  : Test GetAudioTimestampInfo interface. Returns false, if the renderer state is RENDERER_RELEASED
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetAudioTimestampInfo_005, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);

    Timestamp timestamp;
    int32_t ret = audioRenderer->GetAudioTimestampInfo(timestamp, Timestamp::Timestampbase::MONOTONIC);
    EXPECT_EQ(ERR_ILLEGAL_STATE, ret);
}

/**
 * @tc.name  : Test GetAudioTimestampInfo API via illegal state, RENDERER_PAUSED: GetAudioTimestampInfo after Stop.
 * @tc.number: Audio_Renderer_GetAudioTimestampInfo_006
 * @tc.desc  : Test GetAudioTimestampInfo interface. Returns false, if the renderer state is RENDERER_PAUSED.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetAudioTimestampInfo_006, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isPaused = audioRenderer->Pause();
    EXPECT_EQ(true, isPaused);

    Timestamp timestamp;
    int32_t ret = audioRenderer->GetAudioTimestampInfo(timestamp, Timestamp::Timestampbase::MONOTONIC);
    EXPECT_EQ(ERR_ILLEGAL_STATE, ret);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetAudioTimestampInfo API via legal state, RENDERER_PAUSED.
 * @tc.number: Audio_Renderer_GetAudioTimestampInfo_007
 * @tc.desc  : Test GetAudioTimestampInfo interface. Timestamp should be larger after pause 1s.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetAudioTimestampInfo_007, TestSize.Level2)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    size_t bufferSize = 3528; // 44.1 khz, 20ms
    std::unique_ptr<uint8_t[]> tempBuffer = std::make_unique<uint8_t[]>(bufferSize);
    int loopCount = 20; // 400ms
    while (loopCount-- > 0) {
        audioRenderer->Write(tempBuffer.get(), bufferSize);
    }
    Timestamp timestamp1;
    Timestamp timeStampBoot1;
    audioRenderer->GetAudioTimestampInfo(timestamp1, Timestamp::Timestampbase::MONOTONIC);
    audioRenderer->GetAudioTimestampInfo(timeStampBoot1, Timestamp::Timestampbase::BOOTTIME);

    bool isPaused = audioRenderer->Pause();
    EXPECT_EQ(true, isPaused);

    size_t sleepTime = 1000000; // sleep 1s
    usleep(sleepTime);

    isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    loopCount = 10; // 200ms
    while (loopCount-- > 0) {
        audioRenderer->Write(tempBuffer.get(), bufferSize);
    }
    Timestamp timestamp2;
    Timestamp timeStampBoot2;
    audioRenderer->GetAudioTimestampInfo(timestamp2, Timestamp::Timestampbase::MONOTONIC);
    audioRenderer->GetAudioTimestampInfo(timeStampBoot2, Timestamp::Timestampbase::BOOTTIME);

    int64_t duration = (timestamp2.time.tv_sec - timestamp1.time.tv_sec) * 1000000 + (timestamp2.time.tv_nsec -
        timestamp1.time.tv_nsec) / RenderUT::VALUE_THOUSAND; // ns -> us
    EXPECT_GE(duration, sleepTime);
    duration = (timeStampBoot2.time.tv_sec - timeStampBoot1.time.tv_sec) * 1000000 + (timeStampBoot2.time.tv_nsec -
        timeStampBoot1.time.tv_nsec) / RenderUT::VALUE_THOUSAND; // ns -> us
    EXPECT_GE(duration, 0);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetStartStreamResult API.
 * @tc.number: Audio_Renderer_GetStartStreamResult_001
 * @tc.desc  : Test GetStartStreamResult interface.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetStartStreamResult_001, TestSize.Level2)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    ASSERT_TRUE(audioRendererPrivate != nullptr);

    int32_t ret = audioRendererPrivate->GetStartStreamResult(StateChangeCmdType::CMD_FROM_CLIENT);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test GetFastStatus API.
 * @tc.number: GetFastStatus_001
 * @tc.desc  : Test GetFastStatus interface.
 */
HWTEST(AudioRendererUnitTest, GetFastStatus_001, TestSize.Level2)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    ASSERT_TRUE(audioRendererPrivate != nullptr);

    auto ret = audioRendererPrivate->GetFastStatus();
    EXPECT_EQ(ret, FASTSTATUS_NORMAL);
}

/**
 * @tc.name  : Test GetFinalOffloadAllowed API.
 * @tc.number: GetFinalOffloadAllowed_001
 * @tc.desc  : Test GetFinalOffloadAllowed interface with different if cases.
 */
HWTEST(AudioRendererUnitTest, GetFinalOffloadAllowed_001, TestSize.Level3)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> renderer =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    ASSERT_TRUE(renderer != nullptr);
 
    bool allowed = renderer->GetFinalOffloadAllowed(true);
    EXPECT_EQ(allowed, true);
    allowed = renderer->GetFinalOffloadAllowed(false);
    EXPECT_EQ(allowed, false);
 
    setuid(UID_MEDIA);
    allowed = renderer->GetFinalOffloadAllowed(true);
    EXPECT_EQ(allowed, true);
    allowed = renderer->GetFinalOffloadAllowed(false);
    EXPECT_EQ(allowed, false);
}
 
/**
 * @tc.name  : Test HandleSetRendererInfoByOptions API.
 * @tc.number: HandleSetRendererInfoByOptions_001
 * @tc.desc  : Test HandleSetRendererInfoByOptions interface.
 */
HWTEST(AudioRendererUnitTest, HandleSetRendererInfoByOptions_001, TestSize.Level3)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> renderer =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    ASSERT_TRUE(renderer != nullptr);
 
    AudioRendererOptions rendererOpts;
    rendererOpts.rendererInfo.isOffloadAllowed = false;
    renderer->HandleSetRendererInfoByOptions(rendererOpts, appInfo);
    EXPECT_EQ(renderer->rendererInfo_.isOffloadAllowed, false);
}
} // namespace AudioStandard
} // namespace OHOS