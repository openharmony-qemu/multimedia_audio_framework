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

using namespace std;
using namespace std::chrono;
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {

class CapturerPositionCallbackTest : public CapturerPositionCallback {
public:
    void OnMarkReached(const int64_t &framePosition) override {}
};

class CapturerPeriodPositionCallbackTest : public CapturerPeriodPositionCallback {
public:
    void OnPeriodReached(const int64_t &frameNumber) override {}
};

InterruptEvent AudioRendererUnitTest::interruptEventTest_ = {};
void AudioRendererUnitTest::SetUpTestCase(void) {}
void AudioRendererUnitTest::TearDownTestCase(void) {}
void AudioRendererUnitTest::SetUp(void) {}
void AudioRendererUnitTest::TearDown(void) {}

void AudioRenderModeCallbackTest::OnWriteData(size_t length)
{
    RenderUT::g_reqBufLen = length;
}

class TestAudioStremStub : public FastAudioStream {
public:
    TestAudioStremStub() : FastAudioStream(AudioStreamType::STREAM_MUSIC,
        AudioMode::AUDIO_MODE_RECORD, 0) {}
    uint32_t GetOverflowCount() override { return RenderUT::g_writeOverflowNum; }
    State GetState() override { return state_; }
    bool StopAudioStream() override { return true; }
    bool StartAudioStream(StateChangeCmdType cmdType,
        AudioStreamDeviceChangeReasonExt reason) override { return true; }
    bool ReleaseAudioStream(bool releaseRunner, bool destoryAtOnce) override { return true; }

    State state_ = State::RUNNING;
};

void AudioRendererCallbackTest::OnInterrupt(const InterruptEvent &interruptEvent)
{
    AudioRendererUnitTest::interruptEventTest_.hintType = interruptEvent.hintType;
}

int32_t AudioRendererUnitTest::InitializeRenderer(unique_ptr<AudioRenderer> &audioRenderer)
{
    AudioRendererParams rendererParams;
    rendererParams.sampleFormat = SAMPLE_S16LE;
    rendererParams.sampleRate = SAMPLE_RATE_44100;
    rendererParams.channelCount = STEREO;
    rendererParams.encodingType = ENCODING_PCM;

    return audioRenderer->SetParams(rendererParams);
}

void AudioRendererUnitTest::InitializeRendererOptions(AudioRendererOptions &rendererOptions)
{
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    rendererOptions.streamInfo.channels = AudioChannel::STEREO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_MOVIE;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MOVIE;
    rendererOptions.rendererInfo.rendererFlags = RenderUT::RENDERER_FLAG;

    return;
}

void AudioRendererUnitTest::InitializeRendererSpatialOptions(AudioRendererOptions &rendererOptions)
{
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_AUDIOVIVID;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    rendererOptions.streamInfo.channels = AudioChannel::CHANNEL_8;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    rendererOptions.rendererInfo.rendererFlags = RenderUT::RENDERER_FLAG;
    return;
}

void AudioRendererUnitTest::GetBuffersAndLen(unique_ptr<AudioRenderer> &audioRenderer,
    uint8_t *&buffer, uint8_t *&metaBuffer, size_t &bufferLen)
{
    uint32_t ret = audioRenderer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);
    buffer = new uint8_t[bufferLen];
    ASSERT_NE(nullptr, buffer);
    EXPECT_GE(RenderUT::MAX_BUFFER_SIZE, bufferLen);
    metaBuffer = new uint8_t[RenderUT::AVS3METADATA_SIZE];
    ASSERT_NE(nullptr, metaBuffer);
}

void AudioRendererUnitTest::ReleaseBufferAndFiles(uint8_t* &buffer, uint8_t* &metaBuffer,
    FILE* &wavFile, FILE* &metaFile)
{
    delete []buffer;
    delete []metaBuffer;
    (void)fclose(wavFile);
    fclose(metaFile);
}

void StartRenderThread(AudioRenderer *audioRenderer, uint32_t limit)
{
    int32_t ret = -1;
    FILE *wavFile = fopen(RenderUT::AUDIORENDER_TEST_FILE_PATH.c_str(), "rb");
    ASSERT_NE(nullptr, wavFile);

    size_t bufferLen;
    ret = audioRenderer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    auto buffer = std::make_unique<uint8_t[]>(bufferLen);
    ASSERT_NE(nullptr, buffer);

    size_t bytesToWrite = 0;
    int32_t bytesWritten = 0;
    size_t minBytes = 4;
    int32_t numBuffersToRender = RenderUT::WRITE_BUFFERS_COUNT;
    auto start = chrono::system_clock::now();

    while (numBuffersToRender) {
        bytesToWrite = fread(buffer.get(), 1, bufferLen, wavFile);
        bytesWritten = 0;
        while ((static_cast<size_t>(bytesWritten) < bytesToWrite) &&
            ((static_cast<size_t>(bytesToWrite) - bytesWritten) > minBytes)) {
            bytesWritten += audioRenderer->Write(buffer.get() + static_cast<size_t>(bytesWritten),
                                                 bytesToWrite - static_cast<size_t>(bytesWritten));
            if (bytesWritten < 0) {
                break;
            }
        }
        numBuffersToRender--;

        if ((limit > 0) && (duration_cast<seconds>(system_clock::now() - start).count() > limit)) {
            break;
        }
    }

    audioRenderer->Drain();

    fclose(wavFile);
}


/**
 * @tc.name  : Test Create API via legal input.
 * @tc.number: Audio_Renderer_Create_001
 * @tc.desc  : Test Create interface with STREAM_MUSIC. Returns audioRenderer instance, if create is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Create_001, TestSize.Level0)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    EXPECT_NE(nullptr, audioRenderer);
}

/**
 * @tc.name  : Test Create API via legal input.
 * @tc.number: Audio_Renderer_Create_002
 * @tc.desc  : Test Create interface with STREAM_RING. Returns audioRenderer instance, if create is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Create_002, TestSize.Level0)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_RING);
    EXPECT_NE(nullptr, audioRenderer);
}

/**
 * @tc.name  : Test Create API via legal input.
 * @tc.number: Audio_Renderer_Create_003
 * @tc.desc  : Test Create interface with STREAM_VOICE_CALL. Returns audioRenderer instance if create is successful.
 *             Note: instance will be created but functional support for STREAM_VOICE_CALL not available yet.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Create_003, TestSize.Level0)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_VOICE_CALL);
    EXPECT_NE(nullptr, audioRenderer);
}

/**
 * @tc.name  : Test Create API via legal input.
 * @tc.number: Audio_Renderer_Create_004
 * @tc.desc  : Test Create interface with STREAM_SYSTEM. Returns audioRenderer instance, if create is successful.
 *             Note: instance will be created but functional support for STREAM_SYSTEM not available yet.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Create_004, TestSize.Level0)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_SYSTEM);
    EXPECT_NE(nullptr, audioRenderer);
}

/**
 * @tc.name  : Test Create API via legal input.
 * @tc.number: Audio_Renderer_Create_005
 * @tc.desc  : Test Create interface with STREAM_BLUETOOTH_SCO. Returns audioRenderer instance, if create is successful.
 *             Note: instance will be created but functional support for STREAM_BLUETOOTH_SCO not available yet
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Create_005, TestSize.Level0)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_BLUETOOTH_SCO);
    EXPECT_NE(nullptr, audioRenderer);
}

/**
 * @tc.name  : Test Create API via legal input.
 * @tc.number: Audio_Renderer_Create_006
 * @tc.desc  : Test Create interface with STREAM_ALARM. Returns audioRenderer instance, if create is successful.
 *             Note: instance will be created but functional support for STREAM_ALARM not available yet.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Create_006, TestSize.Level0)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_ALARM);
    EXPECT_NE(nullptr, audioRenderer);
}

/**
 * @tc.name  : Test Create API via legal input.
 * @tc.number: Audio_Renderer_Create_007
 * @tc.desc  : Test Create interface with STREAM_NOTIFICATION. Returns audioRenderer instance, if create is successful.
 *             Note: instance will be created but functional support for STREAM_NOTIFICATION not available yet.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Create_007, TestSize.Level0)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_NOTIFICATION);
    EXPECT_NE(nullptr, audioRenderer);
}

/**
 * @tc.name  : Test Create API via legal input.
 * @tc.number: Audio_Renderer_Create_008
 * @tc.desc  : Test Create interface with AudioRendererOptions below.
 *             Returns audioRenderer instance, if create is successful.
 *             rendererOptions.streamInfo.samplingRate = SAMPLE_RATE_96000;
 *             rendererOptions.streamInfo.encoding = ENCODING_PCM;
 *             rendererOptions.streamInfo.format = SAMPLE_U8;
 *             rendererOptions.streamInfo.channels = MONO;
 *             rendererOptions.rendererInfo.contentType = CONTENT_TYPE_MUSIC;
 *             rendererOptions.rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
 *             rendererOptions.rendererInfo.rendererFlags = RenderUT::RENDERER_FLAG;
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Create_008, TestSize.Level0)
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
    ASSERT_NE(nullptr, audioRenderer);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test Create API via legal input.
 * @tc.number: Audio_Renderer_Create_009
 * @tc.desc  : Test Create interface with AudioRendererOptions below.
 *             Returns audioRenderer instance, if create is successful.
 *             rendererOptions.streamInfo.samplingRate = SAMPLE_RATE_96000;
 *             rendererOptions.streamInfo.encoding = ENCODING_PCM;
 *             rendererOptions.streamInfo.format = SAMPLE_U8;
 *             rendererOptions.streamInfo.channels = STEREO;
 *             rendererOptions.rendererInfo.contentType = CONTENT_TYPE_MOVIE;
 *             rendererOptions.rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
 *             rendererOptions.rendererInfo.rendererFlags = RenderUT::RENDERER_FLAG;
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Create_009, TestSize.Level0)
{
    AudioRendererOptions rendererOptions;
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_8000;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_U8;
    rendererOptions.streamInfo.channels = AudioChannel::STEREO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_MOVIE;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    rendererOptions.rendererInfo.rendererFlags = RenderUT::RENDERER_FLAG;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test Create API via legal input.
 * @tc.number: Audio_Renderer_Create_010
 * @tc.desc  : Test Create interface with AudioRendererOptions below.
 *             Returns audioRenderer instance, if create is successful.
 *             rendererOptions.streamInfo.samplingRate = SAMPLE_RATE_64000;
 *             rendererOptions.streamInfo.encoding = ENCODING_PCM;
 *             rendererOptions.streamInfo.format = SAMPLE_S32LE;
 *             rendererOptions.streamInfo.channels = MONO;
 *             rendererOptions.rendererInfo.contentType = CONTENT_TYPE_RINGTONE;
 *             rendererOptions.rendererInfo.streamUsage = STREAM_USAGE_NOTIFICATION_RINGTONE;
 *             rendererOptions.rendererInfo.rendererFlags = RenderUT::RENDERER_FLAG;
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Create_010, TestSize.Level0)
{
    AudioRendererOptions rendererOptions;
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_64000;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S32LE;
    rendererOptions.streamInfo.channels = AudioChannel::MONO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_RINGTONE;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_NOTIFICATION_RINGTONE;
    rendererOptions.rendererInfo.rendererFlags = RenderUT::RENDERER_FLAG;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test Create API via legal input.
 * @tc.number: Audio_Renderer_Create_011
 * @tc.desc  : Test Create interface with AudioRendererOptions below.
 *             Returns audioRenderer instance, if create is successful.
 *             rendererOptions.streamInfo.samplingRate = SAMPLE_RATE_48000;
 *             rendererOptions.streamInfo.encoding = ENCODING_PCM;
 *             rendererOptions.streamInfo.format = SAMPLE_S24LE;
 *             rendererOptions.streamInfo.channels = STEREO;
 *             rendererOptions.rendererInfo.contentType = CONTENT_TYPE_MOVIE;
 *             rendererOptions.rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
 *             rendererOptions.rendererInfo.rendererFlags = RenderUT::RENDERER_FLAG;
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Create_011, TestSize.Level0)
{
    AudioRendererOptions rendererOptions;
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S24LE;
    rendererOptions.streamInfo.channels = AudioChannel::STEREO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_MOVIE;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    rendererOptions.rendererInfo.rendererFlags = RenderUT::RENDERER_FLAG;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test Create API via legal input.
 * @tc.number: Audio_Renderer_Create_012
 * @tc.desc  : Test Create interface with AudioRendererOptions below.
 *             Returns audioRenderer instance, if create is successful.
 *             rendererOptions.streamInfo.samplingRate = SAMPLE_RATE_44100;
 *             rendererOptions.streamInfo.encoding = ENCODING_PCM;
 *             rendererOptions.streamInfo.format = SAMPLE_S16LE;
 *             rendererOptions.streamInfo.channels = MONO;
 *             rendererOptions.rendererInfo.contentType = CONTENT_TYPE_SONIFICATION;
 *             rendererOptions.rendererInfo.streamUsage = STREAM_USAGE_VOICE_ASSISTANT;
 *             rendererOptions.rendererInfo.rendererFlags = RenderUT::RENDERER_FLAG;
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Create_012, TestSize.Level0)
{
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
    audioRenderer->Release();
}

/**
 * @tc.name  : Test Create API via legal input.
 * @tc.number: Audio_Renderer_Create_013
 * @tc.desc  : Test Create interface with AudioRendererOptions below.
 *             Returns audioRenderer instance, if create is successful.
 *             rendererOptions.streamInfo.samplingRate = SAMPLE_RATE_22050;
 *             rendererOptions.streamInfo.encoding = ENCODING_PCM;
 *             rendererOptions.streamInfo.format = SAMPLE_S24LE;
 *             rendererOptions.streamInfo.channels = STEREO;
 *             rendererOptions.rendererInfo.contentType = CONTENT_TYPE_SPEECH;
 *             rendererOptions.rendererInfo.streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;
 *             rendererOptions.rendererInfo.rendererFlags = RenderUT::RENDERER_FLAG;
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Create_013, TestSize.Level0)
{
    AudioRendererOptions rendererOptions;
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_22050;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S24LE;
    rendererOptions.streamInfo.channels = AudioChannel::STEREO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_SPEECH;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_VOICE_COMMUNICATION;
    rendererOptions.rendererInfo.rendererFlags = RenderUT::RENDERER_FLAG;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test Create API via legal input.
 * @tc.number: Audio_Renderer_Create_014
 * @tc.desc  : Test Create interface with AudioRendererOptions below.
 *             Returns audioRenderer instance, if create is successful.
 *             rendererOptions.streamInfo.samplingRate = SAMPLE_RATE_12000;
 *             rendererOptions.streamInfo.encoding = ENCODING_PCM;
 *             rendererOptions.streamInfo.format = SAMPLE_S24LE;
 *             rendererOptions.streamInfo.channels = MONO;
 *             rendererOptions.rendererInfo.contentType = CONTENT_TYPE_MUSIC;
 *             rendererOptions.rendererInfo.streamUsage = STREAM_USAGE_VOICE_ASSISTANT;
 *             rendererOptions.rendererInfo.rendererFlags = RenderUT::RENDERER_FLAG;
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Create_014, TestSize.Level0)
{
    AudioRendererOptions rendererOptions;
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_12000;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S24LE;
    rendererOptions.streamInfo.channels = AudioChannel::MONO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_VOICE_ASSISTANT;
    rendererOptions.rendererInfo.rendererFlags = RenderUT::RENDERER_FLAG;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test Create API via legal input.
 * @tc.number: Audio_Renderer_Create_015
 * @tc.desc  : Test Create interface with STREAM_MUSIC. Returns audioRenderer instance, if create is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Create_015, TestSize.Level0)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MEDIA);
    EXPECT_NE(nullptr, audioRenderer);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test CheckMaxRendererInstances API
 * @tc.number: Audio_Renderer_CheckMaxRendererInstances_001
 * @tc.desc  : Test CheckMaxRendererInstances interface. Returns SUCCESS, if check max renderer instances is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_CheckMaxRendererInstances_001, TestSize.Level0)
{
    int32_t result = AudioRenderer::CheckMaxRendererInstances();
    EXPECT_EQ(SUCCESS, result);
}

/**
 * @tc.name  : Test Mute API
 * @tc.number: Audio_Renderer_Mute_001
 * @tc.desc  : Test Mute interface. Returns true, if check Mute is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Mute_001, TestSize.Level0)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MEDIA);
    EXPECT_NE(nullptr, audioRenderer);
    bool result = audioRenderer->Mute();
    EXPECT_TRUE(result);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test Unmute API
 * @tc.number: Audio_Renderer_Unmute_001
 * @tc.desc  : Test Unmute interface. Returns true, if check Unmute is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Unmute_001, TestSize.Level0)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MEDIA);
    EXPECT_NE(nullptr, audioRenderer);
    bool result = audioRenderer->Unmute();
    EXPECT_TRUE(result);
    audioRenderer->Release();
}

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
 * @tc.name  : Test Renderer playback
 * @tc.number: Audio_Renderer_Playback_001
 * @tc.desc  : Test normal playback for 2 sec
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Playback_001, TestSize.Level0)
{
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    thread renderThread(StartRenderThread, audioRenderer.get(), RenderUT::PLAYBACK_DURATION);

    renderThread.join();

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
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

    thread renderThread(StartRenderThread, audioRenderer.get(), RenderUT::PLAYBACK_DURATION);

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
 * @tc.name  : Test Start API via legal state, RENDERER_PREPARED.
 * @tc.number: Audio_Renderer_Start_001
 * @tc.desc  : Test Start interface. Returns true if start is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Start_001, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test Start API via illegal state, RENDERER_NEW: without initializing the renderer.
 * @tc.number: Audio_Renderer_Start_002
 * @tc.desc  : Test Start interface. Returns false, if the renderer state is RENDERER_NEW.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Start_002, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);
}

/**
 * @tc.name  : Test Start API via illegal state, RENDERER_RELEASED: call Start after Release
 * @tc.number: Audio_Renderer_Start_003
 * @tc.desc  : Test Start interface. Returns false, if the renderer state is RENDERER_RELEASED.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Start_003, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(false, isStarted);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test Start API via legal state, RENDERER_STOPPED: Start Stop and then Start again
 * @tc.number: Audio_Renderer_Start_004
 * @tc.desc  : Test Start interface. Returns true, if the start is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Start_004, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test Start API via illegal state, RENDERER_RUNNING : call Start repeatedly
 * @tc.number: Audio_Renderer_Start_005
 * @tc.desc  : Test Start interface. Returns false, if the renderer state is RENDERER_RUNNING.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Start_005, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    isStarted = audioRenderer->Start();
    EXPECT_EQ(false, isStarted);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test Start API via legal state, RENDERER_PAUSED : call Start after pause
 * @tc.number: Audio_Renderer_Start_005
 * @tc.desc  : Test Start interface. Returns false, if the renderer state is RENDERER_PAUSED.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Start_006, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isPaused = audioRenderer->Pause();
    EXPECT_EQ(true, isPaused);

    isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test Write API.
 * @tc.number: Audio_Renderer_Write_001
 * @tc.desc  : Test Write interface. Returns number of bytes written, if the write is successful.
 */

HWTEST(AudioRendererUnitTest, Audio_Renderer_Write_001, TestSize.Level1)
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
 * @tc.name  : Test Write API via illegl state, RENDERER_NEW : without Initializing the renderer.
 * @tc.number: Audio_Renderer_Write_002
 * @tc.desc  : Test Write interface. Returns error code, if the renderer state is RENDERER_NEW.
 *           : bufferLen is invalid here, firstly bufferLen is validated in Write. So it returns ERR_INVALID_PARAM.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Write_002, TestSize.Level1)
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
    EXPECT_EQ(RenderUT::MIN_CACHE_SIZE, bytesWritten);

    free(buffer);
    fclose(wavFile);
}

/**
 * @tc.name  : Test Write API via illegl state, RENDERER_PREPARED : Write without Start.
 * @tc.number: Audio_Renderer_Write_003
 * @tc.desc  : Test Write interface. Returns error code, if the renderer state is not RENDERER_RUNNING.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Write_003, TestSize.Level1)
{
    int32_t ret = -1;
    FILE *wavFile = fopen(RenderUT::AUDIORENDER_TEST_FILE_PATH.c_str(), "rb");
    ASSERT_NE(nullptr, wavFile);

    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    size_t bufferLen;
    ret = audioRenderer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);
    ASSERT_NE(nullptr, buffer);

    size_t bytesToWrite = fread(buffer, 1, bufferLen, wavFile);
    int32_t bytesWritten = audioRenderer->Write(buffer, bytesToWrite);
    EXPECT_EQ(ERR_ILLEGAL_STATE, bytesWritten);

    audioRenderer->Release();

    free(buffer);
    fclose(wavFile);
}

/**
 * @tc.name  : Test Write API via illegal input, bufferLength = 0.
 * @tc.number: Audio_Renderer_Write_004
 * @tc.desc  : Test Write interface. Returns error code, if the bufferLength <= 0.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Write_004, TestSize.Level1)
{
    FILE *wavFile = fopen(RenderUT::AUDIORENDER_TEST_FILE_PATH.c_str(), "rb");
    ASSERT_NE(nullptr, wavFile);

    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    size_t bufferLen = 0;

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);
    ASSERT_NE(nullptr, buffer);

    size_t bytesToWrite = fread(buffer, 1, bufferLen, wavFile);
    int32_t bytesWritten = audioRenderer->Write(buffer, bytesToWrite);
    EXPECT_EQ(ERR_INVALID_PARAM, bytesWritten);

    audioRenderer->Stop();
    audioRenderer->Release();

    free(buffer);
    fclose(wavFile);
}

/**
 * @tc.name  : Test Write API via illegal input, buffer = nullptr.
 * @tc.number: Audio_Renderer_Write_005
 * @tc.desc  : Test Write interface. Returns error code, if the buffer = nullptr.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Write_005, TestSize.Level1)
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
    uint8_t *buffer_null = nullptr;
    int32_t bytesWritten = audioRenderer->Write(buffer_null, bytesToWrite);
    EXPECT_EQ(ERR_INVALID_PARAM, bytesWritten);

    audioRenderer->Stop();
    audioRenderer->Release();

    free(buffer);
    fclose(wavFile);
}

/**
 * @tc.name  : Test Write API via illegal state, RENDERER_STOPPED: Write after Stop
 * @tc.number: Audio_Renderer_Write_006
 * @tc.desc  : Test Write interface. Returns error code, if the renderer state is not RENDERER_RUNNING
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Write_006, TestSize.Level1)
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

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    size_t bytesToWrite = fread(buffer, 1, bufferLen, wavFile);
    int32_t bytesWritten = audioRenderer->Write(buffer, bytesToWrite);
    EXPECT_EQ(ERR_ILLEGAL_STATE, bytesWritten);

    audioRenderer->Release();

    free(buffer);
    fclose(wavFile);
}

/**
 * @tc.name  : Test Write API via illegal state, RENDERER_RELEASED: Write after Release
 * @tc.number: Audio_Renderer_Write_007
 * @tc.desc  : Test Write interface. Returns error code, if the renderer state is not RENDERER_RUNNING
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Write_007, TestSize.Level1)
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

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);

    size_t bytesToWrite = fread(buffer, 1, bufferLen, wavFile);
    int32_t bytesWritten = audioRenderer->Write(buffer, bytesToWrite);
    EXPECT_EQ(ERR_ILLEGAL_STATE, bytesWritten);

    free(buffer);
    fclose(wavFile);
}

/**
 * @tc.name  : Test Write API.
 * @tc.number: Audio_Renderer_Write_008
 * @tc.desc  : Test Write interface after pause and resume. Returns number of bytes written, if the write is successful.
 */

HWTEST(AudioRendererUnitTest, Audio_Renderer_Write_008, TestSize.Level1)
{
    FILE *wavFile = fopen(RenderUT::AUDIORENDER_TEST_FILE_PATH.c_str(), "rb");
    ASSERT_NE(nullptr, wavFile);

    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    size_t bufferLen;
    int32_t ret = audioRenderer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);
    ASSERT_NE(nullptr, buffer);

    size_t bytesToWrite = 0;
    int32_t bytesWritten = 0;
    size_t minBytes = 4;
    int32_t numBuffersToRender = RenderUT::WRITE_BUFFERS_COUNT;
    bool pauseTested = false;

    while (numBuffersToRender) {
        bytesToWrite = fread(buffer, 1, bufferLen, wavFile);
        bytesWritten = 0;
        uint64_t currFilePos = ftell(wavFile);
        if (!pauseTested && (currFilePos > RenderUT::PAUSE_BUFFER_POSITION) && audioRenderer->Pause()) {
            pauseTested = true;
            sleep(RenderUT::PAUSE_RENDER_TIME_SECONDS);
            isStarted = audioRenderer->Start();
            EXPECT_EQ(true, isStarted);

            ret = audioRenderer->SetVolume(0.5);
            EXPECT_EQ(SUCCESS, ret);
            float volume = audioRenderer->GetVolume();
            EXPECT_EQ(0.5, volume);
        }

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
 * @tc.name  : Test Write API via illegl render mode, RENDER_MODE_CALLBACK.
 * @tc.number: Audio_Renderer_Write_009
 * @tc.desc  : Test Write interface. Returns error code, if the render mode is RENDER_MODE_CALLBACK.
 *           : In RENDER_MODE_CALLBACK Write API call not supported. By default render mode is RENDER_MODE_NORMAL.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Write_009, TestSize.Level1)
{
    int32_t ret = -1;
    FILE *wavFile = fopen(RenderUT::AUDIORENDER_TEST_FILE_PATH.c_str(), "rb");
    ASSERT_NE(nullptr, wavFile);

    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    ret = audioRenderer->SetRenderMode(RENDER_MODE_CALLBACK);
    EXPECT_EQ(SUCCESS, ret);

    size_t bufferLen;
    ret = audioRenderer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);
    ASSERT_NE(nullptr, buffer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    size_t bytesToWrite = fread(buffer, 1, bufferLen, wavFile);
    int32_t bytesWritten = audioRenderer->Write(buffer, bytesToWrite);
    EXPECT_EQ(ERR_INCORRECT_MODE, bytesWritten);

    audioRenderer->Release();

    free(buffer);
    fclose(wavFile);
}

/**
   * @tc.name  : Test Write API.
   * @tc.number: Audio_Renderer_Write_With_Meta_001
   * @tc.desc  : Test Write interface. Returns number of bytes written, if the write is successful.
   */

HWTEST(AudioRendererUnitTest, Audio_Renderer_Write_With_Meta_001, TestSize.Level1)
{
    FILE *wavFile = fopen(RenderUT::AUDIORENDER_TEST_PCMFILE_PATH.c_str(), "rb");
    FILE *metaFile = fopen(RenderUT::AUDIORENDER_TEST_METAFILE_PATH.c_str(), "rb");
    if (wavFile != nullptr) {
        ASSERT_NE(nullptr, wavFile);
        ASSERT_NE(nullptr, metaFile);

        AudioRendererOptions rendererOptions;
        AudioRendererUnitTest::InitializeRendererSpatialOptions(rendererOptions);
        unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
        ASSERT_NE(nullptr, audioRenderer);

        bool isStarted = audioRenderer->Start();
        EXPECT_EQ(true, isStarted);

        size_t bufferLen;
        uint8_t *buffer;
        uint8_t *metaBuffer;

        AudioRendererUnitTest::GetBuffersAndLen(audioRenderer, buffer, metaBuffer, bufferLen);

        size_t bytesToWrite = 0;
        size_t bytesWritten = 0;
        int32_t numBuffersToRender = RenderUT::WRITE_BUFFERS_COUNT;

        while (numBuffersToRender) {
            bytesToWrite = fread(buffer, 1, bufferLen, wavFile);
            fread(metaBuffer, 1, RenderUT::AVS3METADATA_SIZE, metaFile);
            std::fill(buffer + bytesToWrite, buffer + bufferLen, 0);
            bytesWritten = audioRenderer->Write(buffer, bufferLen, metaBuffer, RenderUT::AVS3METADATA_SIZE);
            EXPECT_GE(bytesWritten, RenderUT::VALUE_ZERO);
            numBuffersToRender--;
        }

        audioRenderer->Drain();
        audioRenderer->Stop();
        audioRenderer->Release();

        AudioRendererUnitTest::ReleaseBufferAndFiles(buffer, metaBuffer, wavFile, metaFile);
    }
}

/**
 * @tc.name  : Test Write API via illegl state, RENDERER_NEW : without Initializing the renderer.
 * @tc.number: Audio_Renderer_Write_With_Meta_002
 * @tc.desc  : Test Write interface. Returns error code, if the renderer state is RENDERER_NEW.
 *           : encodingType is not initialized here, so it returns ERR_NOT_SUPPORTED for encodingType that is wrong.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Write_With_Meta_002, TestSize.Level1)
{
    FILE *wavFile = fopen(RenderUT::AUDIORENDER_TEST_PCMFILE_PATH.c_str(), "rb");
    FILE *metaFile = fopen(RenderUT::AUDIORENDER_TEST_METAFILE_PATH.c_str(), "rb");
    if (wavFile != nullptr) {
        ASSERT_NE(nullptr, wavFile);
        ASSERT_NE(nullptr, metaFile);

        AudioRendererOptions rendererOptions;
        AudioRendererUnitTest::InitializeRendererSpatialOptions(rendererOptions);
        unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
        ASSERT_NE(nullptr, audioRenderer);

        bool isStarted = audioRenderer->Start();
        EXPECT_EQ(true, isStarted);

        size_t bufferLen;
        uint8_t *buffer;
        uint8_t *metaBuffer;

        int32_t ret = audioRenderer->GetBufferSize(bufferLen);
        EXPECT_EQ(SUCCESS, ret);

        buffer = new uint8_t[bufferLen];
        ASSERT_NE(nullptr, buffer);
        metaBuffer = new uint8_t[RenderUT::AVS3METADATA_SIZE];
        ASSERT_NE(nullptr, metaBuffer);

        fread(buffer, 1, bufferLen, wavFile);
        fread(metaBuffer, 1, RenderUT::AVS3METADATA_SIZE, metaFile);
        int32_t bytesWritten = audioRenderer->Write(buffer, bufferLen, metaBuffer, RenderUT::AVS3METADATA_SIZE);
        EXPECT_EQ(RenderUT::MAX_CACHE_SIZE, bytesWritten);

        AudioRendererUnitTest::ReleaseBufferAndFiles(buffer, metaBuffer, wavFile, metaFile);
    }
}

/**
 * @tc.name  : Test Write API via illegl state, RENDERER_PREPARED : Write without Start.
 * @tc.number: Audio_Renderer_Write_With_Meta_003
 * @tc.desc  : Test Write interface. Returns error code, if the renderer state is not RENDERER_RUNNING.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Write_With_Meta_003, TestSize.Level1)
{
    FILE *wavFile = fopen(RenderUT::AUDIORENDER_TEST_PCMFILE_PATH.c_str(), "rb");
    FILE *metaFile = fopen(RenderUT::AUDIORENDER_TEST_METAFILE_PATH.c_str(), "rb");
    if (wavFile != nullptr) {
        ASSERT_NE(nullptr, wavFile);
        ASSERT_NE(nullptr, metaFile);

        AudioRendererOptions rendererOptions;

        AudioRendererUnitTest::InitializeRendererSpatialOptions(rendererOptions);
        unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
        ASSERT_NE(nullptr, audioRenderer);

        size_t bufferLen;
        uint8_t *buffer;
        uint8_t *metaBuffer;

        AudioRendererUnitTest::GetBuffersAndLen(audioRenderer, buffer, metaBuffer, bufferLen);

        fread(buffer, 1, bufferLen, wavFile);
        fread(metaBuffer, 1, RenderUT::AVS3METADATA_SIZE, metaFile);
        int32_t bytesWritten = audioRenderer->Write(buffer, bufferLen, metaBuffer, RenderUT::AVS3METADATA_SIZE);
        EXPECT_EQ(ERR_ILLEGAL_STATE, bytesWritten);

        audioRenderer->Release();

        AudioRendererUnitTest::ReleaseBufferAndFiles(buffer, metaBuffer, wavFile, metaFile);
    }
}

/**
 * @tc.name  : Test Write API via illegal input, bufferLength = 0.
 * @tc.number: Audio_Renderer_Write_With_Meta_004
 * @tc.desc  : Test Write interface. Returns error code, if the bufferLength != samples * inchs * bps.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Write_With_Meta_004, TestSize.Level1)
{
    FILE *wavFile = fopen(RenderUT::AUDIORENDER_TEST_PCMFILE_PATH.c_str(), "rb");
    FILE *metaFile = fopen(RenderUT::AUDIORENDER_TEST_METAFILE_PATH.c_str(), "rb");
    if (wavFile != nullptr) {
        ASSERT_NE(nullptr, wavFile);
        ASSERT_NE(nullptr, metaFile);

        AudioRendererOptions rendererOptions;

        AudioRendererUnitTest::InitializeRendererSpatialOptions(rendererOptions);
        unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
        ASSERT_NE(nullptr, audioRenderer);

        bool isStarted = audioRenderer->Start();
        EXPECT_EQ(true, isStarted);

        size_t bufferLen = 0;

        uint8_t *buffer = new uint8_t[bufferLen];
        ASSERT_NE(nullptr, buffer);
        uint8_t *metaBuffer = new uint8_t[RenderUT::AVS3METADATA_SIZE];
        ASSERT_NE(nullptr, metaBuffer);

        fread(buffer, 1, bufferLen, wavFile);
        fread(metaBuffer, 1, RenderUT::AVS3METADATA_SIZE, metaFile);

        int32_t bytesWritten = audioRenderer->Write(buffer, bufferLen, metaBuffer, RenderUT::AVS3METADATA_SIZE);
        EXPECT_EQ(ERR_INVALID_PARAM, bytesWritten);

        audioRenderer->Stop();
        audioRenderer->Release();

        AudioRendererUnitTest::ReleaseBufferAndFiles(buffer, metaBuffer, wavFile, metaFile);
    }
}

/**
 * @tc.name  : Test Write API via illegal input, metaLength = 0.
 * @tc.number: Audio_Renderer_Write_With_Meta_005
 * @tc.desc  : Test Write interface. Returns error code, if the metaLength != sizeoof(avs3metadata).
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Write_With_Meta_005, TestSize.Level1)
{
    FILE *wavFile = fopen(RenderUT::AUDIORENDER_TEST_PCMFILE_PATH.c_str(), "rb");
    FILE *metaFile = fopen(RenderUT::AUDIORENDER_TEST_METAFILE_PATH.c_str(), "rb");
    if (wavFile != nullptr) {
        ASSERT_NE(nullptr, wavFile);
        ASSERT_NE(nullptr, metaFile);

        AudioRendererOptions rendererOptions;

        AudioRendererUnitTest::InitializeRendererSpatialOptions(rendererOptions);
        unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
        ASSERT_NE(nullptr, audioRenderer);

        bool isStarted = audioRenderer->Start();
        EXPECT_EQ(true, isStarted);

        size_t bufferLen;
        int32_t ret = audioRenderer->GetBufferSize(bufferLen);
        EXPECT_EQ(SUCCESS, ret);

        uint8_t *buffer = new uint8_t[bufferLen];
        ASSERT_NE(nullptr, buffer);
        uint8_t *metaBuffer = new uint8_t[0];
        ASSERT_NE(nullptr, metaBuffer);

        fread(buffer, 1, bufferLen, wavFile);
        fread(metaBuffer, 1, 0, metaFile);

        int32_t bytesWritten = audioRenderer->Write(buffer, bufferLen, metaBuffer, 0);
        EXPECT_EQ(ERR_INVALID_PARAM, bytesWritten);

        audioRenderer->Stop();
        audioRenderer->Release();

        AudioRendererUnitTest::ReleaseBufferAndFiles(buffer, metaBuffer, wavFile, metaFile);
    }
}

/**
 * @tc.name  : Test Write API via illegal input, buffer = nullptr.
 * @tc.number: Audio_Renderer_Write_With_Meta_006
 * @tc.desc  : Test Write interface. Returns error code, if the buffer = nullptr.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Write_With_Meta_006, TestSize.Level1)
{
    FILE *wavFile = fopen(RenderUT::AUDIORENDER_TEST_PCMFILE_PATH.c_str(), "rb");
    FILE *metaFile = fopen(RenderUT::AUDIORENDER_TEST_METAFILE_PATH.c_str(), "rb");
    if (wavFile != nullptr) {
        ASSERT_NE(nullptr, wavFile);
        ASSERT_NE(nullptr, metaFile);

        AudioRendererOptions rendererOptions;

        AudioRendererUnitTest::InitializeRendererSpatialOptions(rendererOptions);
        unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
        ASSERT_NE(nullptr, audioRenderer);

        bool isStarted = audioRenderer->Start();
        EXPECT_EQ(true, isStarted);

        size_t bufferLen;
        uint8_t *buffer;
        uint8_t *metaBuffer;

        AudioRendererUnitTest::GetBuffersAndLen(audioRenderer, buffer, metaBuffer, bufferLen);

        fread(buffer, 1, bufferLen, wavFile);
        fread(metaBuffer, 1, RenderUT::AVS3METADATA_SIZE, metaFile);
        uint8_t *buffer_null = nullptr;
        int32_t bytesWritten = audioRenderer->Write(buffer_null, bufferLen, metaBuffer, RenderUT::AVS3METADATA_SIZE);
        EXPECT_EQ(ERR_INVALID_PARAM, bytesWritten);

        audioRenderer->Stop();
        audioRenderer->Release();

        AudioRendererUnitTest::ReleaseBufferAndFiles(buffer, metaBuffer, wavFile, metaFile);
    }
}

/**
 * @tc.name  : Test Write API via illegal input, metaBuffer = nullptr.
 * @tc.number: Audio_Renderer_Write_With_Meta_007
 * @tc.desc  : Test Write interface. Returns error code, if the metaBuffer = nullptr.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Write_With_Meta_007, TestSize.Level1)
{
    FILE *wavFile = fopen(RenderUT::AUDIORENDER_TEST_PCMFILE_PATH.c_str(), "rb");
    FILE *metaFile = fopen(RenderUT::AUDIORENDER_TEST_METAFILE_PATH.c_str(), "rb");
    if (wavFile != nullptr) {
        ASSERT_NE(nullptr, wavFile);
        ASSERT_NE(nullptr, metaFile);

        AudioRendererOptions rendererOptions;

        AudioRendererUnitTest::InitializeRendererSpatialOptions(rendererOptions);
        unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
        ASSERT_NE(nullptr, audioRenderer);

        bool isStarted = audioRenderer->Start();
        EXPECT_EQ(true, isStarted);

        size_t bufferLen;
        uint8_t *buffer;
        uint8_t *metaBuffer;

        AudioRendererUnitTest::GetBuffersAndLen(audioRenderer, buffer, metaBuffer, bufferLen);

        fread(buffer, 1, bufferLen, wavFile);
        fread(metaBuffer, 1, RenderUT::AVS3METADATA_SIZE, metaFile);
        uint8_t *buffer_null = nullptr;
        int32_t bytesWritten = audioRenderer->Write(buffer, bufferLen, buffer_null, RenderUT::AVS3METADATA_SIZE);
        EXPECT_EQ(ERR_INVALID_PARAM, bytesWritten);

        audioRenderer->Stop();
        audioRenderer->Release();

        AudioRendererUnitTest::ReleaseBufferAndFiles(buffer, metaBuffer, wavFile, metaFile);
    }
}

/**
 * @tc.name  : Test Write API via illegal state, RENDERER_STOPPED: Write after Stop
 * @tc.number: Audio_Renderer_Write_With_Meta_008
 * @tc.desc  : Test Write interface. Returns error code, if the renderer state is not RENDERER_RUNNING
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Write_With_Meta_008, TestSize.Level1)
{
    FILE *wavFile = fopen(RenderUT::AUDIORENDER_TEST_PCMFILE_PATH.c_str(), "rb");
    FILE *metaFile = fopen(RenderUT::AUDIORENDER_TEST_METAFILE_PATH.c_str(), "rb");
    if (wavFile != nullptr) {
        ASSERT_NE(nullptr, wavFile);
        ASSERT_NE(nullptr, metaFile);

        AudioRendererOptions rendererOptions;

        AudioRendererUnitTest::InitializeRendererSpatialOptions(rendererOptions);
        unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
        ASSERT_NE(nullptr, audioRenderer);

        bool isStarted = audioRenderer->Start();
        EXPECT_EQ(true, isStarted);

        size_t bufferLen;
        uint8_t *buffer;
        uint8_t *metaBuffer;

        AudioRendererUnitTest::GetBuffersAndLen(audioRenderer, buffer, metaBuffer, bufferLen);

        bool isStopped = audioRenderer->Stop();
        EXPECT_EQ(true, isStopped);

        fread(buffer, 1, bufferLen, wavFile);
        fread(metaBuffer, 1, RenderUT::AVS3METADATA_SIZE, metaFile);
        int32_t bytesWritten = audioRenderer->Write(buffer, bufferLen, metaBuffer, RenderUT::AVS3METADATA_SIZE);
        EXPECT_EQ(ERR_ILLEGAL_STATE, bytesWritten);

        audioRenderer->Release();

        AudioRendererUnitTest::ReleaseBufferAndFiles(buffer, metaBuffer, wavFile, metaFile);
    }
}

/**
 * @tc.name  : Test Write API via illegal state, RENDERER_RELEASED: Write after Release
 * @tc.number: Audio_Renderer_Write_With_Meta_009
 * @tc.desc  : Test Write interface. Returns error code, if the renderer state is not RENDERER_RUNNING
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Write_With_Meta_009, TestSize.Level1)
{
    FILE *wavFile = fopen(RenderUT::AUDIORENDER_TEST_PCMFILE_PATH.c_str(), "rb");
    FILE *metaFile = fopen(RenderUT::AUDIORENDER_TEST_METAFILE_PATH.c_str(), "rb");
    if (wavFile != nullptr) {
        ASSERT_NE(nullptr, wavFile);
        ASSERT_NE(nullptr, metaFile);

        AudioRendererOptions rendererOptions;

        AudioRendererUnitTest::InitializeRendererSpatialOptions(rendererOptions);
        unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
        ASSERT_NE(nullptr, audioRenderer);

        bool isStarted = audioRenderer->Start();
        EXPECT_EQ(true, isStarted);

        size_t bufferLen;
        uint8_t *buffer;
        uint8_t *metaBuffer;

        AudioRendererUnitTest::GetBuffersAndLen(audioRenderer, buffer, metaBuffer, bufferLen);

        bool isReleased = audioRenderer->Release();
        EXPECT_EQ(true, isReleased);


        fread(buffer, 1, bufferLen, wavFile);
        fread(metaBuffer, 1, RenderUT::AVS3METADATA_SIZE, metaFile);
        int32_t bytesWritten = audioRenderer->Write(buffer, bufferLen, metaBuffer, RenderUT::AVS3METADATA_SIZE);
        EXPECT_EQ(ERR_ILLEGAL_STATE, bytesWritten);

        AudioRendererUnitTest::ReleaseBufferAndFiles(buffer, metaBuffer, wavFile, metaFile);
    }
}

/**
 * @tc.name  : Test Write API.
 * @tc.number: Audio_Renderer_Write_With_Meta_010
 * @tc.desc  : Test Write interface after pause and resume. Returns number of bytes written, if the write is successful.
 */

HWTEST(AudioRendererUnitTest, Audio_Renderer_Write_With_Meta_010, TestSize.Level1)
{
    FILE *wavFile = fopen(RenderUT::AUDIORENDER_TEST_PCMFILE_PATH.c_str(), "rb");
    FILE *metaFile = fopen(RenderUT::AUDIORENDER_TEST_METAFILE_PATH.c_str(), "rb");
    if (wavFile != nullptr) {
        ASSERT_NE(nullptr, wavFile);
        ASSERT_NE(nullptr, metaFile);

        AudioRendererOptions rendererOptions;

        AudioRendererUnitTest::InitializeRendererSpatialOptions(rendererOptions);
        unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
        ASSERT_NE(nullptr, audioRenderer);

        bool isStarted = audioRenderer->Start();
        EXPECT_EQ(true, isStarted);

        size_t bufferLen;
        uint8_t *buffer;
        uint8_t *metaBuffer;

        AudioRendererUnitTest::GetBuffersAndLen(audioRenderer, buffer, metaBuffer, bufferLen);

        size_t bytesToWrite = 0;
        int32_t bytesWritten = 0;
        int32_t numBuffersToRender = RenderUT::WRITE_BUFFERS_COUNT;
        bool pauseTested = false;

        while (numBuffersToRender) {
            bytesToWrite = fread(buffer, 1, bufferLen, wavFile);
            fread(metaBuffer, 1, RenderUT::AVS3METADATA_SIZE, metaFile);

            std::fill(buffer + bytesToWrite, buffer + bufferLen, 0);

            bytesWritten = 0;
            uint64_t currFilePos = ftell(wavFile);
            if (!pauseTested && (currFilePos > RenderUT::PAUSE_BUFFER_POSITION) && audioRenderer->Pause()) {
                pauseTested = true;
                sleep(RenderUT::PAUSE_RENDER_TIME_SECONDS);
                isStarted = audioRenderer->Start();
                EXPECT_EQ(true, isStarted);

                int32_t ret = audioRenderer->SetVolume(0.5);
                EXPECT_EQ(SUCCESS, ret);
                float volume = audioRenderer->GetVolume();
                EXPECT_EQ(0.5, volume);
            }

            bytesWritten = audioRenderer->Write(buffer, bufferLen, metaBuffer, RenderUT::AVS3METADATA_SIZE);
            EXPECT_GE(bytesWritten, RenderUT::VALUE_ZERO);
            numBuffersToRender--;
        }

        audioRenderer->Drain();
        audioRenderer->Stop();
        audioRenderer->Release();

        AudioRendererUnitTest::ReleaseBufferAndFiles(buffer, metaBuffer, wavFile, metaFile);
    }
}

/**
 * @tc.name  : Test Write API via illegl render mode, RENDER_MODE_CALLBACK.
 * @tc.number: Audio_Renderer_Write_With_Meta_011
 * @tc.desc  : Test Write interface. Returns error code, if the render mode is RENDER_MODE_CALLBACK.
 *           : In RENDER_MODE_CALLBACK Write API call not supported. By default render mode is RENDER_MODE_NORMAL.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Write_With_Meta_011, TestSize.Level1)
{
    int32_t ret = -1;
    FILE *wavFile = fopen(RenderUT::AUDIORENDER_TEST_PCMFILE_PATH.c_str(), "rb");
    FILE *metaFile = fopen(RenderUT::AUDIORENDER_TEST_METAFILE_PATH.c_str(), "rb");
    if (wavFile != nullptr) {
        ASSERT_NE(nullptr, wavFile);
        ASSERT_NE(nullptr, metaFile);

        AudioRendererOptions rendererOptions;

        AudioRendererUnitTest::InitializeRendererSpatialOptions(rendererOptions);
        unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
        ASSERT_NE(nullptr, audioRenderer);

        ret = audioRenderer->SetRenderMode(RENDER_MODE_CALLBACK);
        EXPECT_EQ(SUCCESS, ret);

        size_t bufferLen;
        uint8_t *buffer;
        uint8_t *metaBuffer;

        AudioRendererUnitTest::GetBuffersAndLen(audioRenderer, buffer, metaBuffer, bufferLen);

        bool isStarted = audioRenderer->Start();
        EXPECT_EQ(true, isStarted);

        fread(buffer, 1, bufferLen, wavFile);
        fread(metaBuffer, 1, RenderUT::AVS3METADATA_SIZE, metaFile);
        int32_t bytesWritten = audioRenderer->Write(buffer, bufferLen, metaBuffer, RenderUT::AVS3METADATA_SIZE);
        EXPECT_EQ(ERR_INCORRECT_MODE, bytesWritten);

        audioRenderer->Release();

        AudioRendererUnitTest::ReleaseBufferAndFiles(buffer, metaBuffer, wavFile, metaFile);
    }
}

/**
 * @tc.name  : Test Drain API.
 * @tc.number: Audio_Renderer_Drain_001
 * @tc.desc  : Test Drain interface. Returns true, if the flush is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Drain_001, TestSize.Level1)
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

    bool isDrained = audioRenderer->Drain();
    EXPECT_EQ(true, isDrained);

    audioRenderer->Stop();
    audioRenderer->Release();

    free(buffer);
    fclose(wavFile);
}

/**
 * @tc.name  : Test Drain API via illegal state, RENDERER_NEW: Without initializing the renderer.
 * @tc.number: Audio_Renderer_Drain_002
 * @tc.desc  : Test Drain interface. Returns false, if the renderer state is not RENDERER_RUNNING.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Drain_002, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    bool isDrained = audioRenderer->Drain();
    EXPECT_EQ(false, isDrained);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test Drain API via illegal state, RENDERER_PREPARED: Without Start.
 * @tc.number: Audio_Renderer_Drain_003
 * @tc.desc  : Test Drain interface. Returns false, if the renderer state is not RENDERER_RUNNING.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Drain_003, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isDrained = audioRenderer->Drain();
    EXPECT_EQ(false, isDrained);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test Drain API via illegal state, RENDERER_STOPPED: call Stop before Drain.
 * @tc.number: Audio_Renderer_Drain_004
 * @tc.desc  : Test Drain interface. Returns false, if the renderer state is not RENDERER_RUNNING.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Drain_004, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    bool isDrained = audioRenderer->Drain();
    EXPECT_EQ(false, isDrained);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test Drain API via illegal state, RENDERER_RELEASED: call Release before Drain.
 * @tc.number: Audio_Renderer_Drain_005
 * @tc.desc  : Test Drain interface. Returns false, if the renderer state is not RENDERER_RUNNING.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Drain_005, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);

    bool isDrained = audioRenderer->Drain();
    EXPECT_EQ(false, isDrained);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test Drain API via illegal state, RENDERER_PAUSED: call Pause before Drain.
 * @tc.number: Audio_Renderer_Drain_006
 * @tc.desc  : Test Drain interface. Returns false, if the renderer state is not RENDERER_RUNNING.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Drain_006, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isPaused = audioRenderer->Pause();
    EXPECT_EQ(true, isPaused);

    bool isDrained = audioRenderer->Drain();
    EXPECT_EQ(false, isDrained);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test Drain API stability.
 * @tc.number: Audio_Renderer_Drain_Stability_001
 * @tc.desc  : Test Drain interface stability.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Drain_Stability_001, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    thread renderThread(StartRenderThread, audioRenderer.get(), RenderUT::PLAYBACK_DURATION);

    for (int i = 0; i < RenderUT::VALUE_THOUSAND; i++) {
        bool isDrained = audioRenderer->Drain();
        if (isDrained != true) {
            return ;
        }
    }

    renderThread.join();

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
 * @tc.name  : Test Flush API.
 * @tc.number: Audio_Renderer_Flush_001
 * @tc.desc  : Test Flush interface. Returns true, if the flush is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Flush_001, TestSize.Level1)
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

    bool isFlushed = audioRenderer->Flush();
    EXPECT_EQ(true, isFlushed);

    audioRenderer->Release();

    free(buffer);
    fclose(wavFile);
}

/**
 * @tc.name  : Test Flush API.
 * @tc.number: Audio_Renderer_Flush_002
 * @tc.desc  : Test Flush interface after Pause call. Returns true, if the flush is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Flush_002, TestSize.Level1)
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

    audioRenderer->Pause();

    bool isFlushed = audioRenderer->Flush();
    EXPECT_EQ(true, isFlushed);

    audioRenderer->Release();

    free(buffer);
    fclose(wavFile);
}

/**
 * @tc.name  : Test Flush API via illegal state, RENDERER_NEW: Without initializing the renderer.
 * @tc.number: Audio_Renderer_Flush_003
 * @tc.desc  : Test Flush interface. Returns false, if the renderer state is not RENDERER_RUNNING or RENDERER_PAUSED.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Flush_003, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    bool isFlushed = audioRenderer->Flush();
    EXPECT_EQ(false, isFlushed);
}

/**
 * @tc.name  : Test Flush API via illegal state, RENDERER_PREPARED: Without Start.
 * @tc.number: Audio_Renderer_Flush_004
 * @tc.desc  : Test Flush interface. Returns false, if the renderer state is not RENDERER_RUNNING or RENDERER_PAUSED.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Flush_004, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isFlushed = audioRenderer->Flush();
    EXPECT_EQ(false, isFlushed);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test Flush API: call Stop before Flush.
 * @tc.number: Audio_Renderer_Flush_005
 * @tc.desc  : Test Flush interface. Returns true, if the renderer state is RENDERER_STOPPED.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Flush_005, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    bool isFlushed = audioRenderer->Flush();
    EXPECT_EQ(true, isFlushed);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test Flush API via illegal state, RENDERER_RELEASED: call Release before Flush.
 * @tc.number: Audio_Renderer_Flush_006
 * @tc.desc  : Test Flush interface. Returns false, if the renderer state is not RENDERER_RUNNING.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Flush_006, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);

    bool isFlushed = audioRenderer->Flush();
    EXPECT_EQ(false, isFlushed);
}

/**
 * @tc.name  : Test Flush API stability.
 * @tc.number: Audio_Renderer_Flush_Stability_001
 * @tc.desc  : Test Flush interface stability.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Flush_Stability_001, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    thread renderThread(StartRenderThread, audioRenderer.get(), RenderUT::PLAYBACK_DURATION);

    for (int i = 0; i < RenderUT::VALUE_THOUSAND; i++) {
        bool isFlushed = audioRenderer->Flush();
        EXPECT_EQ(true, isFlushed);
    }

    renderThread.join();

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
 * @tc.name  : Test Pause API.
 * @tc.number: Audio_Renderer_Pause_001
 * @tc.desc  : Test Pause interface. Returns true, if the pause is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Pause_001, TestSize.Level1)
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

    audioRenderer->Drain();

    bool isPaused = audioRenderer->Pause();
    EXPECT_EQ(true, isPaused);

    audioRenderer->Release();

    free(buffer);
    fclose(wavFile);
}

/**
 * @tc.name  : Test Pause API via illegal state, RENDERER_NEW: call Pause without Initializing the renderer.
 * @tc.number: Audio_Renderer_Pause_002
 * @tc.desc  : Test Pause interface. Returns false, if the renderer state is not RENDERER_RUNNING.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Pause_002, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    bool isPaused = audioRenderer->Pause();
    EXPECT_EQ(false, isPaused);
}

/**
 * @tc.name  : Test Pause API via illegal state, RENDERER_PREPARED: call Pause without Start.
 * @tc.number: Audio_Renderer_Pause_003
 * @tc.desc  : Test Pause interface. Returns false, if the renderer state is not RENDERER_RUNNING.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Pause_003, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isPaused = audioRenderer->Pause();
    EXPECT_EQ(false, isPaused);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test Pause API via illegal state, RENDERER_RELEASED: call Pause after Release.
 * @tc.number: Audio_Renderer_Pause_004
 * @tc.desc  : Test Pause interface. Returns false, if the renderer state is not RENDERER_RUNNING.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Pause_004, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);

    bool isPaused = audioRenderer->Pause();
    EXPECT_EQ(false, isPaused);
}

/**
 * @tc.name  : Test Pause and resume
 * @tc.number: Audio_Renderer_Pause_005
 * @tc.desc  : Test Pause interface. Returns true , if the pause is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Pause_005, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isPaused = audioRenderer->Pause();
    EXPECT_EQ(true, isPaused);

    isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    audioRenderer->Stop();
    audioRenderer->Release();
}

/**
 * @tc.name  : Test Pause API via illegal state, RENDERER_STOPPED: call Pause after Stop.
 * @tc.number: Audio_Renderer_Pause_006
 * @tc.desc  : Test Pause interface. Returns false, if the renderer state is not RENDERER_RUNNING.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Pause_006, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    bool isPaused = audioRenderer->Pause();
    EXPECT_EQ(false, isPaused);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test PauseTransitent API.
 * @tc.number: Audio_Renderer_PauseTransitent_001
 * @tc.desc  : Test Pause interface. Returns true, if the pause is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_PauseTransitent_001, TestSize.Level1)
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

    audioRenderer->Drain();

    bool isPaused = audioRenderer->PauseTransitent();
    EXPECT_EQ(true, isPaused);

    audioRenderer->Release();

    free(buffer);
    fclose(wavFile);
}

/**
 * @tc.name  : Test PauseTransitent API via illegal state, RENDERER_NEW: call Pause without Initializing the renderer.
 * @tc.number: Audio_Renderer_PauseTransitent_002
 * @tc.desc  : Test Pause interface. Returns false, if the renderer state is not RENDERER_RUNNING.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_PauseTransitent_002, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    bool isPaused = audioRenderer->Pause();
    EXPECT_EQ(false, isPaused);
}

/**
 * @tc.name  : Test PauseTransitent API via illegal state, RENDERER_PREPARED: call Pause without Start.
 * @tc.number: Audio_Renderer_PauseTransitent_003
 * @tc.desc  : Test Pause interface. Returns false, if the renderer state is not RENDERER_RUNNING.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_PauseTransitent_003, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isPaused = audioRenderer->PauseTransitent();
    EXPECT_EQ(false, isPaused);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test PauseTransitent API via illegal state, RENDERER_RELEASED: call Pause after Release.
 * @tc.number: Audio_Renderer_PauseTransitent_004
 * @tc.desc  : Test Pause interface. Returns false, if the renderer state is not RENDERER_RUNNING.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_PauseTransitent_004, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);

    bool isPaused = audioRenderer->PauseTransitent();
    EXPECT_EQ(false, isPaused);
}

/**
 * @tc.name  : Test Pause and resume
 * @tc.number: Audio_Renderer_PauseTransitent_005
 * @tc.desc  : Test Pause interface. Returns true , if the pause is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_PauseTransitent_005, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isPaused = audioRenderer->PauseTransitent();
    EXPECT_EQ(true, isPaused);

    isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    audioRenderer->Stop();
    audioRenderer->Release();
}

/**
 * @tc.name  : Test PauseTransitent API via illegal state, RENDERER_STOPPED: call Pause after Stop.
 * @tc.number: Audio_Renderer_Pause_006
 * @tc.desc  : Test Pause interface. Returns false, if the renderer state is not RENDERER_RUNNING.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_PauseTransitent_006, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    bool isPaused = audioRenderer->PauseTransitent();
    EXPECT_EQ(false, isPaused);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test Pause and resume
 * @tc.number: Audio_Renderer_PauseTransitent_007
 * @tc.desc  : Test Pause interface. Returns false, if the isSwitching_ is true.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_PauseTransitent_007, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);

    audioRendererPrivate->isSwitching_ = true;
    bool ret = audioRendererPrivate->PauseTransitent(CMD_FROM_CLIENT);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test Pause and resume
 * @tc.number: Audio_Renderer_PauseTransitent_008
 * @tc.desc  : Test Pause interface. Returns true, if the streamUsage is STREAM_USAGE_VOICE_MODEM_COMMUNICATION.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_PauseTransitent_008, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);

    audioRendererPrivate->isSwitching_ = false;
    audioRendererPrivate->audioInterrupt_.streamUsage = STREAM_USAGE_VOICE_MODEM_COMMUNICATION;
    bool ret = audioRendererPrivate->PauseTransitent(CMD_FROM_SYSTEM);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test Pause and resume
 * @tc.number: Audio_Renderer_Pause_Stability_001
 * @tc.desc  : Test Pause interface for stability.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Pause_Stability_001, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    FILE *wavFile = fopen(RenderUT::AUDIORENDER_TEST_FILE_PATH.c_str(), "rb");
    ASSERT_NE(nullptr, wavFile);

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
        while ((static_cast<size_t>(bytesWritten) < bytesToWrite) &&
            ((static_cast<size_t>(bytesToWrite) - bytesWritten) > minBytes)) {
            bytesWritten += audioRenderer->Write(buffer + static_cast<size_t>(bytesWritten),
                                                 bytesToWrite - static_cast<size_t>(bytesWritten));
            EXPECT_GE(bytesWritten, RenderUT::VALUE_ZERO);
            if (bytesWritten < 0) {
                break;
            }
        }
        EXPECT_EQ(true, audioRenderer->Pause());
        EXPECT_EQ(true, audioRenderer->Start());
        numBuffersToRender--;
    }

    audioRenderer->Drain();

    free(buffer);
    fclose(wavFile);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
 * @tc.name  : Test Stop API.
 * @tc.number: Audio_Renderer_Stop_001
 * @tc.desc  : Test Stop interface. Returns true, if the stop is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Stop_001, TestSize.Level1)
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

    audioRenderer->Drain();

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    audioRenderer->Release();

    free(buffer);
    fclose(wavFile);
}

/**
 * @tc.name  : Test Stop API via illegal state, RENDERER_NEW: call Stop without Initializing the renderer.
 * @tc.number: Audio_Renderer_Stop_002
 * @tc.desc  : Test Stop interface. Returns false, if the renderer state is not RENDERER_RUNNING.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Stop_002, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(false, isStopped);
}

/**
 * @tc.name  : Test Stop API via illegal state, RENDERER_PREPARED: call Stop without Start.
 * @tc.number: Audio_Renderer_Stop_003
 * @tc.desc  : Test Stop interface. Returns false, if the renderer state is not RENDERER_RUNNING.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Stop_003, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(false, isStopped);

    audioRenderer->Release();
}

/**
 * @tc.name  : Test Stop API via illegal state, RENDERER_RELEASED: call Stop after Release.
 * @tc.number: Audio_Renderer_Stop_004
 * @tc.desc  : Test Stop interface. Returns false, if the renderer state is not RENDERER_RUNNING.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Stop_004, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(false, isStopped);
}

/**
 * @tc.name  : Test Stop API via legal state. call Start, Stop, Start and Stop again
 * @tc.number: Audio_Renderer_Stop_005
 * @tc.desc  : Test Stop interface. Returns true , if the stop is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Stop_005, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test Stop API via legal state, RENDERER_PAUSED: call Stop after Pause.
 * @tc.number: Audio_Renderer_Stop_006
 * @tc.desc  : Test Stop interface. Returns false, if the renderer state is not RENDERER_RUNNING or RENDERER_PAUSED.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Stop_006, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isPaused = audioRenderer->Pause();
    EXPECT_EQ(true, isPaused);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test Release API.
 * @tc.number: Audio_Renderer_Release_001
 * @tc.desc  : Test Release interface. Returns true, if the release is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Release_001, TestSize.Level1)
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

    audioRenderer->Drain();
    audioRenderer->Stop();

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);

    free(buffer);
    fclose(wavFile);
}

/**
 * @tc.name  : Test Release API via illegal state, RENDERER_NEW: Call Release without initializing the renderer.
 * @tc.number: Audio_Renderer_Release_002
 * @tc.desc  : Test Release interface, Returns true, if the state is RENDERER_NEW.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Release_002, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
 * @tc.name  : Test Release API via illegal state, RENDERER_RELEASED: call Release repeatedly.
 * @tc.number: Audio_Renderer_Release_003
 * @tc.desc  : Test Release interface. Returns true, if the state is already RENDERER_RELEASED.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Release_003, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);

    isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
 * @tc.name  : Test Release API via legal state, RENDERER_RUNNING: call Release after Start
 * @tc.number: Audio_Renderer_Release_004
 * @tc.desc  : Test Release interface. Returns true, if the release is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Release_004, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
 * @tc.name  : Test Release API via legal state, RENDERER_STOPPED: call release after Start and Stop
 * @tc.number: Audio_Renderer_Release_005
 * @tc.desc  : Test Release interface. Returns true, if the release is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Release_005, TestSize.Level1)
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
}

/**
 * @tc.name  : Test Release API via legal state, RENDERER_PAUSED: call release after Start and Pause
 * @tc.number: Audio_Renderer_Release_006
 * @tc.desc  : Test Release interface. Returns true, if the release is successful.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Release_006, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isPaused = audioRenderer->Pause();
    EXPECT_EQ(true, isPaused);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
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
 * @tc.name  : Test Enqueue via legal render mode, RENDER_MODE_CALLBACK
 * @tc.number: Audio_Renderer_Enqueue_001
 * @tc.desc  : Test Enqueue interface. Returns SUCCESS , if the buff desc enqueued successfully.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Enqueue_001, TestSize.Level1)
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

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    BufferDesc bufDesc {};
    bufDesc.buffer = nullptr;
    bufDesc.dataLength = RenderUT::g_reqBufLen;
    ret = audioRenderer->GetBufferDesc(bufDesc);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_NE(nullptr, bufDesc.buffer);

    ret = audioRenderer->Enqueue(bufDesc);
    EXPECT_EQ(SUCCESS, ret);

    audioRenderer->Stop();
    audioRenderer->Release();
}

/**
 * @tc.name  : Test Enqueue via illegal render mode, RENDER_MODE_NORMAL
 * @tc.number: Audio_Renderer_Enqueue_002
 * @tc.desc  : Test Enqueue interface. Returns error code, if the render mode is not callback.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Enqueue_002, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    shared_ptr<AudioRendererWriteCallback> cb = make_shared<AudioRenderModeCallbackTest>();

    ret = audioRenderer->SetRendererWriteCallback(cb);
    EXPECT_EQ(ERR_INCORRECT_MODE, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    BufferDesc bufDesc {};
    bufDesc.buffer = nullptr;
    bufDesc.dataLength = RenderUT::g_reqBufLen;
    ret = audioRenderer->GetBufferDesc(bufDesc);
    EXPECT_EQ(ERR_INCORRECT_MODE, ret);

    ret = audioRenderer->Enqueue(bufDesc);
    EXPECT_EQ(ERR_INCORRECT_MODE, ret);

    audioRenderer->Stop();
    audioRenderer->Release();
}

/**
 * @tc.name  : Test Enqueue via illegal input, buffer nullptr
 * @tc.number: Audio_Renderer_Enqueue_003
 * @tc.desc  : Test Enqueue interface. Returns error code, if the buffer nullptr
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Enqueue_003, TestSize.Level1)
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

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    BufferDesc bufDesc {};
    bufDesc.buffer = nullptr;
    bufDesc.dataLength = RenderUT::g_reqBufLen;

    ret = audioRenderer->Enqueue(bufDesc);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);

    audioRenderer->Stop();
    audioRenderer->Release();
}

/**
 * @tc.name  : Test Clear via legal render mode, RENDER_MODE_CALLBACK
 * @tc.number: Audio_Renderer_Clear_001
 * @tc.desc  : Test Clear interface. Returns SUCCESS , if the buff queue cleared successfully.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Clear_001, TestSize.Level1)
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

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    BufferDesc bufDesc {};
    bufDesc.buffer = nullptr;
    bufDesc.dataLength = RenderUT::g_reqBufLen;
    ret = audioRenderer->GetBufferDesc(bufDesc);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_NE(nullptr, bufDesc.buffer);

    ret = audioRenderer->Enqueue(bufDesc);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioRenderer->Clear();
    EXPECT_EQ(SUCCESS, ret);

    audioRenderer->Stop();
    audioRenderer->Release();
}

/**
 * @tc.name  : Test Clear via illegal render mode, RENDER_MODE_NORMAL
 * @tc.number: Audio_Renderer_Clear_002
 * @tc.desc  : Test Clear interface. Returns error code, if the render mode is not callback.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Clear_002, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    shared_ptr<AudioRendererWriteCallback> cb = make_shared<AudioRenderModeCallbackTest>();

    ret = audioRenderer->SetRendererWriteCallback(cb);
    EXPECT_EQ(ERR_INCORRECT_MODE, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    BufferDesc bufDesc {};
    bufDesc.buffer = nullptr;
    bufDesc.dataLength = RenderUT::g_reqBufLen;
    ret = audioRenderer->GetBufferDesc(bufDesc);
    EXPECT_EQ(ERR_INCORRECT_MODE, ret);

    ret = audioRenderer->Enqueue(bufDesc);
    EXPECT_EQ(ERR_INCORRECT_MODE, ret);

    ret = audioRenderer->Clear();
    EXPECT_EQ(ERR_INCORRECT_MODE, ret);

    audioRenderer->Stop();
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
 * @tc.name  : Test max renderer instances.
 * @tc.number: Audio_Renderer_Max_Renderer_Instances_001
 * @tc.desc  : Test creating maximum configured audio renderer instances.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Max_Renderer_Instances_001, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    vector<unique_ptr<AudioRenderer>> rendererList;
    vector<shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos = {};
    AudioPolicyManager::GetInstance().GetCurrentRendererChangeInfos(audioRendererChangeInfos);

    // Create renderer instance with the maximum number of configured instances
    while (audioRendererChangeInfos.size() < RenderUT::MAX_RENDERER_INSTANCES) {
        auto audioRenderer = AudioRenderer::Create(rendererOptions);
        EXPECT_NE(nullptr, audioRenderer);
        rendererList.push_back(std::move(audioRenderer));
        audioRendererChangeInfos.clear();
        AudioPolicyManager::GetInstance().GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    }

    for (auto it = rendererList.begin(); it != rendererList.end();) {
        bool isReleased = (*it)->Release();
        EXPECT_EQ(true, isReleased);
        it = rendererList.erase(it);
    }
    EXPECT_EQ(rendererList.size(), 0);
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
* @tc.name  : Test RegisterAudioPolicyServerDiedCb via legal state
* @tc.number: Audio_Renderer_RegisterAudioPolicyServerDiedCb_001
* @tc.desc  : Test registerAudioRendererEventListener interface. Returns success.
*/
HWTEST(AudioRendererUnitTest, Audio_Renderer_RegisterAudioPolicyServerDiedCb_001, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    int32_t clientId = getpid();
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    shared_ptr<AudioRendererPolicyServiceDiedCallbackTest> callback =
        make_shared<AudioRendererPolicyServiceDiedCallbackTest>();
    int32_t ret = audioRenderer->RegisterAudioPolicyServerDiedCb(clientId, callback);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioRenderer->UnregisterAudioPolicyServerDiedCb(clientId);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test RegisterAudioPolicyServerDiedCb via legal state
* @tc.number: Audio_Renderer_RegisterAudioPolicyServerDiedCb_002
* @tc.desc  : Test registerAudioRendererEventListener interface. Returns ERR_INVALID_PARAM.
*/
HWTEST(AudioRendererUnitTest, Audio_Renderer_RegisterAudioPolicyServerDiedCb_002, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    int32_t clientId = getpid();
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    int32_t ret = audioRenderer->RegisterAudioPolicyServerDiedCb(clientId, nullptr);
    EXPECT_EQ(RenderUT::VALUE_ERROR, ret);
}

/**
* @tc.name  : Test RegisterAudioPolicyServerDiedCb via legal state
* @tc.number: Audio_Renderer_RegisterAudioPolicyServerDiedCb_Stability_001
* @tc.desc  : Test registerAudioRendererEventListener interface valid callback 1000 times.
*/
HWTEST(AudioRendererUnitTest, Audio_Renderer_RegisterAudioPolicyServerDiedCb_Stability_001, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    int32_t clientId = getpid();
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);
    for (int i = 0; i < RenderUT::VALUE_THOUSAND; i++) {
        shared_ptr<AudioRendererPolicyServiceDiedCallbackTest> callback =
            make_shared<AudioRendererPolicyServiceDiedCallbackTest>();
        int32_t ret = audioRenderer->RegisterAudioPolicyServerDiedCb(clientId, callback);
        EXPECT_EQ(SUCCESS, ret);

        ret = audioRenderer->UnregisterAudioPolicyServerDiedCb(clientId);
        EXPECT_EQ(SUCCESS, ret);
    }
}

/**
* @tc.name  : Test RegisterAudioPolicyServerDiedCb via legal state
* @tc.number: Audio_Renderer_RegisterAudioPolicyServerDiedCb_002
* @tc.desc  : Test registerAudioRendererEventListener interface invalid callback 1000 times.
*/
HWTEST(AudioRendererUnitTest, Audio_Renderer_RegisterAudioPolicyServerDiedCb_Stability_002, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    int32_t clientId = getpid();
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);
    for (int i = 0; i < RenderUT::VALUE_THOUSAND; i++) {
        int32_t ret = audioRenderer->RegisterAudioPolicyServerDiedCb(clientId, nullptr);
        EXPECT_EQ(RenderUT::VALUE_ERROR, ret);
    }
}

/**
* @tc.name  : Test UnregisterAudioPolicyServerDiedCb via legal state
* @tc.number: Audio_Renderer_UnregisterAudioPolicyServerDiedCb_001
* @tc.desc  : Test UnregisterAudioPolicyServerDiedCb interface. Returns success.
*/
HWTEST(AudioRendererUnitTest, Audio_Renderer_UnregisterAudioPolicyServerDiedCb_001, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    int32_t clientId = getpid();
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    shared_ptr<AudioRendererPolicyServiceDiedCallbackTest> callback =
        make_shared<AudioRendererPolicyServiceDiedCallbackTest>();
    int32_t ret = audioRenderer->RegisterAudioPolicyServerDiedCb(clientId, callback);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioRenderer->UnregisterAudioPolicyServerDiedCb(clientId);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test UnregisterAudioPolicyServerDiedCb via legal state
* @tc.number: Audio_Renderer_UnregisterAudioPolicyServerDiedCb_Stability_001
* @tc.desc  : Test UnregisterAudioPolicyServerDiedCb interface valid callback 1000 times.
*/
HWTEST(AudioRendererUnitTest, Audio_Renderer_UnregisterAudioPolicyServerDiedCb_Stability_001, TestSize.Level1)
{
    AudioRendererOptions rendererOptions;

    int32_t clientId = getpid();
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);
    for (int i = 0; i < RenderUT::VALUE_THOUSAND; i++) {
        shared_ptr<AudioRendererPolicyServiceDiedCallbackTest> callback =
            make_shared<AudioRendererPolicyServiceDiedCallbackTest>();
        int32_t ret = audioRenderer->RegisterAudioPolicyServerDiedCb(clientId, callback);
        EXPECT_EQ(SUCCESS, ret);

        ret = audioRenderer->UnregisterAudioPolicyServerDiedCb(clientId);
        EXPECT_EQ(SUCCESS, ret);
    }
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
 * @tc.name  : Test SetPitch
 * @tc.number: Audio_Renderer_SetPitch_001
 * @tc.desc  : Test SetPitch interface.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetPitch_001, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    ret = audioRenderer->SetPitch(0.5);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioRenderer->SetPitch(0.25); // 0.25 min speed
    EXPECT_EQ(SUCCESS, ret);

    ret = audioRenderer->SetPitch(4); // 4 max speed
    EXPECT_EQ(SUCCESS, ret);

    ret = audioRenderer->SetPitch(0.124); // 0.124 lower
    EXPECT_EQ(ERR_INVALID_PARAM, ret);

    ret = audioRenderer->SetPitch(4.01); // 4.01 upper
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
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    ret = audioRenderer->SetOffloadAllowed(false);
    EXPECT_EQ(SUCCESS, ret);

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
    audioRendererProxyObj->SaveRendererObj(std::weak_ptr<AudioRendererPrivate>());
    audioRendererProxyObj->SetOffloadModeImpl(0, true);
    audioRendererProxyObj->UnsetOffloadModeImpl();
    float ret = -1.0f;
    audioRendererProxyObj->SetLowPowerVolumeImpl(1.0f);
    audioRendererProxyObj->GetLowPowerVolumeImpl(ret);
    EXPECT_EQ(-1.0f, ret);
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
 * @tc.name  : Test HandleAndNotifyForcedEvent
 * @tc.number: Audio_Renderer_HandleAndNotifyForcedEvent_001
 * @tc.desc  : Test HandleAndNotifyForcedEvent interface. if the InterruptHint is INTERRUPT_HINT_PAUSE.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_HandleAndNotifyForcedEvent_001, TestSize.Level1)
{
    AudioStreamParams audioStreamParams;
    std::shared_ptr<IAudioStream> audioStream = IAudioStream::GetPlaybackStream(IAudioStream::FAST_STREAM,
        audioStreamParams, STREAM_DEFAULT, 1);
    AudioInterrupt audioInterrupt;
    auto audioInterruptCallback = std::make_shared<AudioRendererInterruptCallbackImpl>(audioStream, audioInterrupt);
    InterruptEventInternal interruptEvent {INTERRUPT_TYPE_BEGIN, INTERRUPT_FORCE, INTERRUPT_HINT_PAUSE, 20.0f};
    audioInterruptCallback->HandleAndNotifyForcedEvent(interruptEvent);
    EXPECT_FALSE(audioInterruptCallback->isForcePaused_);
}

/**
 * @tc.name  : Test HandleAndNotifyForcedEvent
 * @tc.number: Audio_Renderer_HandleAndNotifyForcedEvent_002
 * @tc.desc  : Test HandleAndNotifyForcedEvent interface. if the InterruptHint is INTERRUPT_HINT_RESUME.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_HandleAndNotifyForcedEvent_002, TestSize.Level1)
{
    AudioStreamParams audioStreamParams;
    std::shared_ptr<IAudioStream> audioStream = IAudioStream::GetPlaybackStream(IAudioStream::PA_STREAM,
        audioStreamParams, STREAM_DEFAULT, 1);
    AudioInterrupt audioInterrupt;
    auto audioInterruptCallback = std::make_shared<AudioRendererInterruptCallbackImpl>(audioStream, audioInterrupt);
    InterruptEventInternal interruptEvent {INTERRUPT_TYPE_BEGIN, INTERRUPT_FORCE, INTERRUPT_HINT_RESUME, 20.0f};
    audioInterruptCallback->HandleAndNotifyForcedEvent(interruptEvent);
    EXPECT_FALSE(audioInterruptCallback->isForcePaused_);
}

/**
 * @tc.name  : Test HandleAndNotifyForcedEvent
 * @tc.number: Audio_Renderer_HandleAndNotifyForcedEvent_003
 * @tc.desc  : Test HandleAndNotifyForcedEvent interface. if the InterruptHint is INTERRUPT_HINT_DUCK.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_HandleAndNotifyForcedEvent_003, TestSize.Level1)
{
    AudioStreamParams audioStreamParams;
    std::shared_ptr<IAudioStream> audioStream = IAudioStream::GetPlaybackStream(IAudioStream::PA_STREAM,
        audioStreamParams, STREAM_DEFAULT, 1);
    AudioInterrupt audioInterrupt;
    auto audioInterruptCallback = std::make_shared<AudioRendererInterruptCallbackImpl>(audioStream, audioInterrupt);
    InterruptEventInternal interruptEvent {INTERRUPT_TYPE_BEGIN, INTERRUPT_FORCE, INTERRUPT_HINT_DUCK, 20.0f};
    audioInterruptCallback->HandleAndNotifyForcedEvent(interruptEvent);
    EXPECT_FALSE(audioInterruptCallback->isForcePaused_);
}

/**
 * @tc.name  : Test HandleAndNotifyForcedEvent
 * @tc.number: Audio_Renderer_HandleAndNotifyForcedEvent_004
 * @tc.desc  : Test HandleAndNotifyForcedEvent interface. if the InterruptHint is INTERRUPT_HINT_UNDUCK.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_HandleAndNotifyForcedEvent_004, TestSize.Level1)
{
    AudioStreamParams audioStreamParams;
    std::shared_ptr<IAudioStream> audioStream = IAudioStream::GetPlaybackStream(IAudioStream::PA_STREAM,
        audioStreamParams, STREAM_DEFAULT, 1);
    AudioInterrupt audioInterrupt;
    auto audioInterruptCallback = std::make_shared<AudioRendererInterruptCallbackImpl>(audioStream, audioInterrupt);
    InterruptEventInternal interruptEvent {INTERRUPT_TYPE_BEGIN, INTERRUPT_FORCE, INTERRUPT_HINT_UNDUCK, 20.0f};
    audioInterruptCallback->HandleAndNotifyForcedEvent(interruptEvent);
    EXPECT_FALSE(audioInterruptCallback->isForcePaused_);
}

/**
 * @tc.name  : Test HandleAndNotifyForcedEvent
 * @tc.number: Audio_Renderer_HandleAndNotifyForcedEvent_005
 * @tc.desc  : Test HandleAndNotifyForcedEvent interface. if the InterruptHint is INTERRUPT_HINT_NONE.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_HandleAndNotifyForcedEvent_005, TestSize.Level1)
{
    AudioStreamParams audioStreamParams;
    std::shared_ptr<IAudioStream> audioStream = IAudioStream::GetPlaybackStream(IAudioStream::PA_STREAM,
        audioStreamParams, STREAM_DEFAULT, 1);
    AudioInterrupt audioInterrupt;
    auto audioInterruptCallback = std::make_shared<AudioRendererInterruptCallbackImpl>(audioStream, audioInterrupt);
    InterruptEventInternal interruptEvent {INTERRUPT_TYPE_BEGIN, INTERRUPT_FORCE, INTERRUPT_HINT_NONE, 20.0f};
    audioInterruptCallback->HandleAndNotifyForcedEvent(interruptEvent);
    EXPECT_FALSE(audioInterruptCallback->isForcePaused_);
}

/**
 * @tc.name  : Test IsDirectVoipParams
 * @tc.number: Audio_Renderer_IsDirectVoipParams_001
 * @tc.desc  : Test Create interface with IsDirectVoipParams below.
 *              audioStreamParams.samplingRate = SAMPLE_RATE_8000;
 *              audioStreamParams.channels = MONO;
 *              audioStreamParams.format = SAMPLE_S16LE;
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_IsDirectVoipParams_001, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    AudioStreamParams audioStreamParams;
    audioStreamParams.samplingRate = SAMPLE_RATE_16000;
    audioStreamParams.channels = MONO;
    audioStreamParams.format = SAMPLE_S16LE;
    bool ret = audioRendererPrivate->IsDirectVoipParams(audioStreamParams);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name  : Test IsDirectVoipParams
 * @tc.number: Audio_Renderer_IsDirectVoipParams_002
 * @tc.desc  : Test Create interface with IsDirectVoipParams below.
 *              audioStreamParams.samplingRate = SAMPLE_RATE_16000;
 *              audioStreamParams.channels = STEREO;
 *              audioStreamParams.format = SAMPLE_S32LE;
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_IsDirectVoipParams_002, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    AudioStreamParams audioStreamParams;
    audioStreamParams.samplingRate = SAMPLE_RATE_16000;
    audioStreamParams.channels = STEREO;
    audioStreamParams.format = SAMPLE_S32LE;
    bool ret = audioRendererPrivate->IsDirectVoipParams(audioStreamParams);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name  : Test IsDirectVoipParams
 * @tc.number: Audio_Renderer_IsDirectVoipParams_003
 * @tc.desc  : Test Create interface with IsDirectVoipParams below.
 *              audioStreamParams.samplingRate = SAMPLE_RATE_48000;
 *              audioStreamParams.channels = STEREO;
 *              audioStreamParams.format = INVALID_WIDTH;
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_IsDirectVoipParams_003, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    AudioStreamParams audioStreamParams;
    audioStreamParams.samplingRate = SAMPLE_RATE_48000;
    audioStreamParams.channels = STEREO;
    audioStreamParams.format = INVALID_WIDTH;
    bool ret = audioRendererPrivate->IsDirectVoipParams(audioStreamParams);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test IsDirectVoipParams
 * @tc.number: Audio_Renderer_IsDirectVoipParams_004
 * @tc.desc  : Test Create interface with IsDirectVoipParams below.
 *              audioStreamParams.samplingRate = SAMPLE_RATE_16000;
 *              audioStreamParams.channels = CHANNEL_3;
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_IsDirectVoipParams_004, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    AudioStreamParams audioStreamParams;
    audioStreamParams.samplingRate = SAMPLE_RATE_16000;
    audioStreamParams.channels = CHANNEL_3;
    bool ret = audioRendererPrivate->IsDirectVoipParams(audioStreamParams);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test IsDirectVoipParams
 * @tc.number: Audio_Renderer_IsDirectVoipParams_005
 * @tc.desc  : Test Create interface with IsDirectVoipParams below.
 *              audioStreamParams.samplingRate = SAMPLE_RATE_192000;
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_IsDirectVoipParams_005, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    AudioStreamParams audioStreamParams;
    audioStreamParams.samplingRate = SAMPLE_RATE_192000;
    bool ret = audioRendererPrivate->IsDirectVoipParams(audioStreamParams);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test OnAudioPolicyServiceDied
 * @tc.number: Audio_Renderer_OnAudioPolicyServiceDied_001
 * @tc.desc  : Test OnAudioPolicyServiceDied interface
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_OnAudioPolicyServiceDied_001, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    audioRendererPrivate->RegisterRendererPolicyServiceDiedCallback();
    audioRendererPrivate->RegisterOutputDeviceChangeWithInfoCallback(nullptr);
    audioRendererPrivate->audioPolicyServiceDiedCallback_->OnAudioPolicyServiceDied();
    ASSERT_NE(nullptr, audioRendererPrivate);
}

/**
 * @tc.name  : Test RegisterOutputDeviceChangeWithInfoCallback
 * @tc.number: Audio_Renderer_RegisterOutputDeviceChangeWithInfoCallback_001
 * @tc.desc  : Test RegisterOutputDeviceChangeWithInfoCallback interface
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_RegisterOutputDeviceChangeWithInfoCallback_001, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    audioRendererPrivate->RegisterOutputDeviceChangeWithInfoCallback(nullptr);
    ASSERT_NE(nullptr, audioRendererPrivate);
}

/**
 * @tc.name  : Test SetSilentModeAndMixWithOthers
 * @tc.number: Audio_Renderer_WriteUnderrunEvent_001
 * @tc.desc  : Test SetSilentModeAndMixWithOthers interface
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_WriteUnderrunEvent_001, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    audioRendererPrivate->rendererInfo_.originalFlag = AUDIO_FLAG_MMAP;
    std::shared_ptr<FastAudioStream> audioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK,
        appInfo.appUid);
    audioRendererPrivate->audioStream_ = audioStream;
    audioStream->state_ = RUNNING;
    audioStream->silentModeAndMixWithOthers_ = false;
    audioRendererPrivate->SetSilentModeAndMixWithOthers(true);
    EXPECT_TRUE(audioStream->silentModeAndMixWithOthers_);
}

/**
 * @tc.name  : Test SetSilentModeAndMixWithOthers
 * @tc.number: Audio_Renderer_WriteUnderrunEvent_002
 * @tc.desc  : Test SetSilentModeAndMixWithOthers interface
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_WriteUnderrunEvent_002, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    audioRendererPrivate->rendererInfo_.originalFlag = AUDIO_FLAG_MMAP;
    std::shared_ptr<FastAudioStream> audioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK,
        appInfo.appUid);
    audioRendererPrivate->audioStream_ = audioStream;
    audioStream->state_ = RUNNING;
    audioStream->silentModeAndMixWithOthers_ = true;
    audioRendererPrivate->SetSilentModeAndMixWithOthers(true);
    EXPECT_TRUE(audioStream->silentModeAndMixWithOthers_);
}

/**
 * @tc.name  : Test SetSilentModeAndMixWithOthers
 * @tc.number: Audio_Renderer_WriteUnderrunEvent_003
 * @tc.desc  : Test SetSilentModeAndMixWithOthers interface
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_WriteUnderrunEvent_003, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    audioRendererPrivate->rendererInfo_.originalFlag = AUDIO_FLAG_MMAP;
    std::shared_ptr<FastAudioStream> audioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK,
        appInfo.appUid);
    audioRendererPrivate->audioStream_ = audioStream;
    audioStream->state_ = RUNNING;
    audioStream->silentModeAndMixWithOthers_ = false;
    audioRendererPrivate->SetSilentModeAndMixWithOthers(false);
    EXPECT_FALSE(audioStream->silentModeAndMixWithOthers_);
}

/**
 * @tc.name  : Test SetSilentModeAndMixWithOthers
 * @tc.number: Audio_Renderer_WriteUnderrunEvent_004
 * @tc.desc  : Test SetSilentModeAndMixWithOthers interface
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_WriteUnderrunEvent_004, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    audioRendererPrivate->rendererInfo_.originalFlag = AUDIO_FLAG_MMAP;
    std::shared_ptr<FastAudioStream> audioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK,
        appInfo.appUid);
    audioRendererPrivate->audioStream_ = audioStream;
    audioStream->state_ = RUNNING;
    audioStream->silentModeAndMixWithOthers_ = true;
    audioRendererPrivate->SetSilentModeAndMixWithOthers(false);
    EXPECT_FALSE(audioStream->silentModeAndMixWithOthers_);
}

/**
 * @tc.name  : Test OnInterrupt
 * @tc.number: Audio_Renderer_OnInterrupt_001
 * @tc.desc  : Test OnInterrupt interface
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_OnInterrupt_001, TestSize.Level1)
{
    AudioStreamParams audioStreamParams;
    std::shared_ptr<IAudioStream> audioStream = IAudioStream::GetPlaybackStream(IAudioStream::FAST_STREAM,
        audioStreamParams, STREAM_DEFAULT, 1);
    AudioInterrupt audioInterrupt;
    auto audioInterruptCallback = std::make_shared<AudioRendererInterruptCallbackImpl>(audioStream, audioInterrupt);
    InterruptEventInternal interruptEvent {INTERRUPT_TYPE_BEGIN, INTERRUPT_SHARE, INTERRUPT_HINT_PAUSE, 20.0f};
    audioInterruptCallback->audioStream_ = nullptr;
    audioInterruptCallback->OnInterrupt(interruptEvent);
}

/**
 * @tc.name  : Test ConcedeStream
 * @tc.number: Audio_Renderer_ConcedeStream_001
 * @tc.desc  : Test ConcedeStream interface
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_ConcedeStream_001, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    audioRendererPrivate->rendererInfo_.originalFlag = AUDIO_FLAG_MMAP;
    std::shared_ptr<FastAudioStream> audioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK,
        appInfo.appUid);
    audioStream->rendererInfo_.pipeType = PIPE_TYPE_LOWLATENCY_OUT;
    audioRendererPrivate->ConcedeStream();
    ASSERT_NE(nullptr, audioRendererPrivate);
}

/**
 * @tc.name  : Test ConcedeStream
 * @tc.number: Audio_Renderer_ConcedeStream_002
 * @tc.desc  : Test ConcedeStream interface
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_ConcedeStream_002, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    audioRendererPrivate->rendererInfo_.originalFlag = AUDIO_FLAG_MMAP;
    std::shared_ptr<FastAudioStream> audioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK,
        appInfo.appUid);
    audioStream->rendererInfo_.pipeType = PIPE_TYPE_DIRECT_MUSIC;
    audioRendererPrivate->ConcedeStream();
    ASSERT_NE(nullptr, audioRendererPrivate);
}

/**
 * @tc.name  : Test ConcedeStream
 * @tc.number: Audio_Renderer_ConcedeStream_003
 * @tc.desc  : Test ConcedeStream interface
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_ConcedeStream_003, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    audioRendererPrivate->rendererInfo_.originalFlag = AUDIO_FLAG_MMAP;
    std::shared_ptr<FastAudioStream> audioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK,
        appInfo.appUid);
    audioRendererPrivate->WriteUnderrunEvent();
    audioStream->rendererInfo_.pipeType = PIPE_TYPE_UNKNOWN;
    audioRendererPrivate->ConcedeStream();
    ASSERT_NE(nullptr, audioRendererPrivate);
}

/**
 * @tc.name  : Test direct VoIP Audio Render
 * @tc.number: Audio_Renderer_Direct_VoIP_001
 * @tc.desc  : Test the direct VoIP stream type with STREAM_USAGE_VOICE_COMMUNICATION
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Direct_VoIP_001, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_8000;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    rendererOptions.streamInfo.channels = AudioChannel::STEREO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_UNKNOWN;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_VOICE_COMMUNICATION;
    rendererOptions.rendererInfo.rendererFlags = 0;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    ret = audioRenderer->SetRenderMode(RENDER_MODE_CALLBACK);
    EXPECT_EQ(SUCCESS, ret);

    shared_ptr<AudioRendererWriteCallbackMock> cb = make_shared<AudioRendererWriteCallbackMock>();

    ret = audioRenderer->SetRendererWriteCallback(cb);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    std::this_thread::sleep_for(1s);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);
    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
 * @tc.name  : Test direct VoIP Audio Render
 * @tc.number: Audio_Renderer_Direct_VoIP_002
 * @tc.desc  : Test the direct VoIP stream type with STREAM_USAGE_VOICE_COMMUNICATION
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Direct_VoIP_002, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_16000;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    rendererOptions.streamInfo.channels = AudioChannel::STEREO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_UNKNOWN;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_VOICE_COMMUNICATION;
    rendererOptions.rendererInfo.rendererFlags = 0;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    ret = audioRenderer->SetRenderMode(RENDER_MODE_CALLBACK);
    EXPECT_EQ(SUCCESS, ret);

    shared_ptr<AudioRendererWriteCallbackMock> cb = make_shared<AudioRendererWriteCallbackMock>();

    ret = audioRenderer->SetRendererWriteCallback(cb);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    std::this_thread::sleep_for(1s);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);
    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
 * @tc.name  : Test direct VoIP Audio Render
 * @tc.number: Audio_Renderer_Direct_VoIP_003
 * @tc.desc  : Test the direct VoIP stream type with STREAM_USAGE_VOICE_COMMUNICATION
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Direct_VoIP_003, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    rendererOptions.streamInfo.channels = AudioChannel::STEREO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_UNKNOWN;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_VOICE_COMMUNICATION;
    rendererOptions.rendererInfo.rendererFlags = 0;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    ret = audioRenderer->SetRenderMode(RENDER_MODE_CALLBACK);
    EXPECT_EQ(SUCCESS, ret);

    shared_ptr<AudioRendererWriteCallbackMock> cb = make_shared<AudioRendererWriteCallbackMock>();

    ret = audioRenderer->SetRendererWriteCallback(cb);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    std::this_thread::sleep_for(1s);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);
    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
 * @tc.name  : Test direct VoIP Audio Render
 * @tc.number: Audio_Renderer_Direct_VoIP_004
 * @tc.desc  : Test the direct VoIP stream type with STREAM_USAGE_VOICE_COMMUNICATION
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Direct_VoIP_004, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S32LE;
    rendererOptions.streamInfo.channels = AudioChannel::STEREO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_UNKNOWN;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_VOICE_COMMUNICATION;
    rendererOptions.rendererInfo.rendererFlags = 0;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    ret = audioRenderer->SetRenderMode(RENDER_MODE_CALLBACK);
    EXPECT_EQ(SUCCESS, ret);

    shared_ptr<AudioRendererWriteCallbackMock> cb = make_shared<AudioRendererWriteCallbackMock>();

    ret = audioRenderer->SetRendererWriteCallback(cb);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    std::this_thread::sleep_for(1s);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);
    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
 * @tc.name  : Test direct VoIP Audio Render
 * @tc.number: Audio_Renderer_Direct_VoIP_005
 * @tc.desc  : Test the direct VoIP stream type with STREAM_USAGE_VOICE_COMMUNICATION
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Direct_VoIP_005, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    rendererOptions.streamInfo.channels = AudioChannel::MONO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_UNKNOWN;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_VOICE_COMMUNICATION;
    rendererOptions.rendererInfo.rendererFlags = 0;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    ret = audioRenderer->SetRenderMode(RENDER_MODE_CALLBACK);
    EXPECT_EQ(SUCCESS, ret);

    shared_ptr<AudioRendererWriteCallbackMock> cb = make_shared<AudioRendererWriteCallbackMock>();

    ret = audioRenderer->SetRendererWriteCallback(cb);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    std::this_thread::sleep_for(1s);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);
    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
 * @tc.name  : Test direct VoIP Audio Render
 * @tc.number: Audio_Renderer_Direct_VoIP_006
 * @tc.desc  : Test the direct VoIP stream type with STREAM_USAGE_VIDEO_COMMUNICATION
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Direct_VoIP_006, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_8000;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    rendererOptions.streamInfo.channels = AudioChannel::STEREO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_UNKNOWN;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_VIDEO_COMMUNICATION;
    rendererOptions.rendererInfo.rendererFlags = 0;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    ret = audioRenderer->SetRenderMode(RENDER_MODE_CALLBACK);
    EXPECT_EQ(SUCCESS, ret);

    shared_ptr<AudioRendererWriteCallbackMock> cb = make_shared<AudioRendererWriteCallbackMock>();

    ret = audioRenderer->SetRendererWriteCallback(cb);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    std::this_thread::sleep_for(1s);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);
    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
 * @tc.name  : Test direct VoIP Audio Render
 * @tc.number: Audio_Renderer_Direct_VoIP_007
 * @tc.desc  : Test the direct VoIP stream type with STREAM_USAGE_VIDEO_COMMUNICATION
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Direct_VoIP_007, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_16000;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    rendererOptions.streamInfo.channels = AudioChannel::STEREO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_UNKNOWN;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_VIDEO_COMMUNICATION;
    rendererOptions.rendererInfo.rendererFlags = 0;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    ret = audioRenderer->SetRenderMode(RENDER_MODE_CALLBACK);
    EXPECT_EQ(SUCCESS, ret);

    shared_ptr<AudioRendererWriteCallbackMock> cb = make_shared<AudioRendererWriteCallbackMock>();

    ret = audioRenderer->SetRendererWriteCallback(cb);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    std::this_thread::sleep_for(1s);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);
    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
 * @tc.name  : Test direct VoIP Audio Render
 * @tc.number: Audio_Renderer_Direct_VoIP_008
 * @tc.desc  : Test the direct VoIP stream type with STREAM_USAGE_VIDEO_COMMUNICATION
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Direct_VoIP_008, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    rendererOptions.streamInfo.channels = AudioChannel::STEREO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_UNKNOWN;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_VIDEO_COMMUNICATION;
    rendererOptions.rendererInfo.rendererFlags = 0;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    ret = audioRenderer->SetRenderMode(RENDER_MODE_CALLBACK);
    EXPECT_EQ(SUCCESS, ret);

    shared_ptr<AudioRendererWriteCallbackMock> cb = make_shared<AudioRendererWriteCallbackMock>();

    ret = audioRenderer->SetRendererWriteCallback(cb);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    std::this_thread::sleep_for(1s);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);
    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
 * @tc.name  : Test direct VoIP Audio Render
 * @tc.number: Audio_Renderer_Direct_VoIP_009
 * @tc.desc  : Test the direct VoIP stream type with STREAM_USAGE_VIDEO_COMMUNICATION
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Direct_VoIP_009, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S32LE;
    rendererOptions.streamInfo.channels = AudioChannel::STEREO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_UNKNOWN;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_VIDEO_COMMUNICATION;
    rendererOptions.rendererInfo.rendererFlags = 0;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    ret = audioRenderer->SetRenderMode(RENDER_MODE_CALLBACK);
    EXPECT_EQ(SUCCESS, ret);

    shared_ptr<AudioRendererWriteCallbackMock> cb = make_shared<AudioRendererWriteCallbackMock>();

    ret = audioRenderer->SetRendererWriteCallback(cb);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    std::this_thread::sleep_for(1s);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);
    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
 * @tc.name  : Test direct VoIP Audio Render
 * @tc.number: Audio_Renderer_Direct_VoIP_010
 * @tc.desc  : Test the direct VoIP stream type with STREAM_USAGE_VIDEO_COMMUNICATION
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_Direct_VoIP_010, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    rendererOptions.streamInfo.channels = AudioChannel::MONO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_UNKNOWN;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_VIDEO_COMMUNICATION;
    rendererOptions.rendererInfo.rendererFlags = 0;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    ret = audioRenderer->SetRenderMode(RENDER_MODE_CALLBACK);
    EXPECT_EQ(SUCCESS, ret);

    shared_ptr<AudioRendererWriteCallbackMock> cb = make_shared<AudioRendererWriteCallbackMock>();

    ret = audioRenderer->SetRendererWriteCallback(cb);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    std::this_thread::sleep_for(1s);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);
    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
 * @tc.name  : Test InitAudioInterruptCallback
 * @tc.number: InitAudioInterruptCallback
 * @tc.desc  : Test InitAudioInterruptCallback
 */
HWTEST(AudioRendererUnitTest, InitAudioInterruptCallback_001, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    audioRendererPrivate->audioInterrupt_.streamId = 1;
    audioRendererPrivate->InitAudioInterruptCallback();
    EXPECT_EQ(audioRendererPrivate->audioInterrupt_.streamId, 1);
}

/**
 * @tc.name  : Test IsDirectVoipParams
 * @tc.number: IsDirectVoipParams
 * @tc.desc  : Test IsDirectVoipParams
 */
HWTEST(AudioRendererUnitTest, IsDirectVoipParams_001, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    AudioStreamParams audioStreamParams_;
    audioStreamParams_.samplingRate = SAMPLE_RATE_48000;
    audioStreamParams_.channels = CHANNEL_3;
    const AudioStreamParams audioStreamParams = audioStreamParams_;

    bool ret = audioRendererPrivate->IsDirectVoipParams(audioStreamParams);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test IsDirectVoipParams
 * @tc.number: IsDirectVoipParams
 * @tc.desc  : Test IsDirectVoipParams
 */
HWTEST(AudioRendererUnitTest, IsDirectVoipParams_002, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    AudioStreamParams audioStreamParams_;
    audioStreamParams_.samplingRate = SAMPLE_RATE_48000;
    audioStreamParams_.channels = STEREO;
    audioStreamParams_.format = SAMPLE_S16LE;
    const AudioStreamParams audioStreamParams = audioStreamParams_;

    bool ret = audioRendererPrivate->IsDirectVoipParams(audioStreamParams);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test IsDirectVoipParams
 * @tc.number: IsDirectVoipParams
 * @tc.desc  : Test IsDirectVoipParams
 */
HWTEST(AudioRendererUnitTest, IsDirectVoipParams_003, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    AudioStreamParams audioStreamParams_;
    audioStreamParams_.samplingRate = SAMPLE_RATE_48000;
    audioStreamParams_.channels = STEREO;
    audioStreamParams_.format = SAMPLE_F32LE;
    const AudioStreamParams audioStreamParams = audioStreamParams_;

    bool ret = audioRendererPrivate->IsDirectVoipParams(audioStreamParams);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test PrepareAudioStream
 * @tc.number: PrepareAudioStream
 * @tc.desc  : Test PrepareAudioStream
 */
HWTEST(AudioRendererUnitTest, PrepareAudioStream_001, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    audioRendererPrivate->audioStream_ = nullptr;
    AudioStreamParams audioStreamParams;
    const AudioStreamType audioStreamType = STREAM_VOICE_CALL;
    IAudioStream::StreamClass streamClass;

    int32_t ret = audioRendererPrivate->PrepareAudioStream(audioStreamParams, audioStreamType, streamClass);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test PauseTransitent
 * @tc.number: PauseTransitent
 * @tc.desc  : Test PauseTransitent
 */
HWTEST(AudioRendererUnitTest, PauseTransitent_001, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    StateChangeCmdType cmdType = CMD_FROM_SYSTEM;
    audioRendererPrivate->isSwitching_ = true;

    bool ret = audioRendererPrivate->PauseTransitent(cmdType);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test PauseTransitent
 * @tc.number: PauseTransitent
 * @tc.desc  : Test PauseTransitent
 */
HWTEST(AudioRendererUnitTest, PauseTransitent_002, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    StateChangeCmdType cmdType = CMD_FROM_SYSTEM;
    audioRendererPrivate->rendererInfo_.streamUsage = STREAM_USAGE_VOICE_MODEM_COMMUNICATION;
    audioRendererPrivate->isEnableVoiceModemCommunicationStartStream_ = false;

    bool ret = audioRendererPrivate->PauseTransitent(cmdType);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test UpdateAudioInterruptStrategy
 * @tc.number: UpdateAudioInterruptStrategy
 * @tc.desc  : Test UpdateAudioInterruptStrategy
 */
HWTEST(AudioRendererUnitTest, UpdateAudioInterruptStrategy_001, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    audioRendererPrivate->isStillMuted_ = true;
    float volume = 1;

    audioRendererPrivate->UpdateAudioInterruptStrategy(volume);
    EXPECT_EQ(audioRendererPrivate->isStillMuted_, false);
}

/**
 * @tc.name  : Test AudioRendererPrivate
 * @tc.number: UpdateAudioInterruptStrategy_002
 * @tc.desc  : Test UpdateAudioInterruptStrategy API
 */
HWTEST(AudioRendererUnitTest, UpdateAudioInterruptStrategy_002, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    audioRendererPrivate->isStillMuted_ = true;
    float volume = 1;
    std::shared_ptr<TestAudioStremStub> testAudioStremStub = std::make_shared<TestAudioStremStub>();

    testAudioStremStub->state_ = RUNNING;
    audioRendererPrivate->audioStream_ = testAudioStremStub;
    audioRendererPrivate->UpdateAudioInterruptStrategy(volume);
    EXPECT_EQ(audioRendererPrivate->isStillMuted_, false);
}

/**
 * @tc.name  : Test HandleAndNotifyForcedEvent
 * @tc.number: HandleAndNotifyForcedEvent
 * @tc.desc  : Test HandleAndNotifyForcedEvent
 */
HWTEST(AudioRendererUnitTest, HandleAndNotifyForcedEvent_001, TestSize.Level1)
{
    AudioStreamParams audioStreamParams;
    std::shared_ptr<IAudioStream> audioStream = IAudioStream::GetPlaybackStream(IAudioStream::FAST_STREAM,
        audioStreamParams, STREAM_DEFAULT, 1);
    AudioInterrupt audioInterrupt;
    auto audioInterruptCallback = std::make_shared<AudioRendererInterruptCallbackImpl>(audioStream, audioInterrupt);
    InterruptEventInternal interruptEvent_;
    interruptEvent_.hintType = INTERRUPT_HINT_PAUSE;

    const InterruptEventInternal interruptEvent = interruptEvent_;
    std::shared_ptr<TestAudioStremStub> testAudioStremStub = std::make_shared<TestAudioStremStub>();
    testAudioStremStub->state_ = NEW;

    audioInterruptCallback->HandleAndNotifyForcedEvent(interruptEvent);
    EXPECT_EQ(testAudioStremStub->state_, NEW);
}

/**
 * @tc.name  : Test HandleAndNotifyForcedEvent
 * @tc.number: HandleAndNotifyForcedEvent
 * @tc.desc  : Test HandleAndNotifyForcedEvent
 */
HWTEST(AudioRendererUnitTest, HandleAndNotifyForcedEvent_002, TestSize.Level1)
{
    AudioStreamParams audioStreamParams;
    std::shared_ptr<IAudioStream> audioStream = IAudioStream::GetPlaybackStream(IAudioStream::FAST_STREAM,
        audioStreamParams, STREAM_DEFAULT, 1);
    AudioInterrupt audioInterrupt;
    auto audioInterruptCallback = std::make_shared<AudioRendererInterruptCallbackImpl>(audioStream, audioInterrupt);
    InterruptEventInternal interruptEvent_;
    interruptEvent_.hintType = INTERRUPT_HINT_RESUME;
    audioInterruptCallback->isForcePaused_ = true;

    const InterruptEventInternal interruptEvent = interruptEvent_;
    std::shared_ptr<TestAudioStremStub> testAudioStremStub = std::make_shared<TestAudioStremStub>();
    testAudioStremStub->state_ = PAUSED;

    audioInterruptCallback->HandleAndNotifyForcedEvent(interruptEvent);
    EXPECT_EQ(audioInterruptCallback->isForcePaused_, true);
}

/**
 * @tc.name  : Test HandleAndNotifyForcedEvent
 * @tc.number: HandleAndNotifyForcedEvent
 * @tc.desc  : Test HandleAndNotifyForcedEvent
 */
HWTEST(AudioRendererUnitTest, HandleAndNotifyForcedEvent_003, TestSize.Level1)
{
    AudioStreamParams audioStreamParams;
    std::shared_ptr<IAudioStream> audioStream = IAudioStream::GetPlaybackStream(IAudioStream::FAST_STREAM,
        audioStreamParams, STREAM_DEFAULT, 1);
    AudioInterrupt audioInterrupt;
    auto audioInterruptCallback = std::make_shared<AudioRendererInterruptCallbackImpl>(audioStream, audioInterrupt);
    InterruptEventInternal interruptEvent_;
    interruptEvent_.hintType = INTERRUPT_HINT_RESUME;
    audioInterruptCallback->isForcePaused_ = true;

    const InterruptEventInternal interruptEvent = interruptEvent_;
    std::shared_ptr<TestAudioStremStub> testAudioStremStub = std::make_shared<TestAudioStremStub>();
    testAudioStremStub->state_ = NEW;

    audioInterruptCallback->HandleAndNotifyForcedEvent(interruptEvent);
    EXPECT_EQ(testAudioStremStub->state_, NEW);
}

/**
 * @tc.name  : Test HandleAndNotifyForcedEvent
 * @tc.number: HandleAndNotifyForcedEvent
 * @tc.desc  : Test HandleAndNotifyForcedEvent
 */
HWTEST(AudioRendererUnitTest, HandleAndNotifyForcedEvent_004, TestSize.Level1)
{
    AudioStreamParams audioStreamParams;
    std::shared_ptr<IAudioStream> audioStream = IAudioStream::GetPlaybackStream(IAudioStream::FAST_STREAM,
        audioStreamParams, STREAM_DEFAULT, 1);
    AudioInterrupt audioInterrupt;
    auto audioInterruptCallback = std::make_shared<AudioRendererInterruptCallbackImpl>(audioStream, audioInterrupt);
    InterruptEventInternal interruptEvent_;
    interruptEvent_.hintType = INTERRUPT_HINT_DUCK;
    audioInterruptCallback->isForcePaused_ = true;
    interruptEvent_.duckVolume = 0.5f;

    const InterruptEventInternal interruptEvent = interruptEvent_;
    std::shared_ptr<TestAudioStremStub> testAudioStremStub = std::make_shared<TestAudioStremStub>();
    testAudioStremStub->state_ = NEW;

    audioInterruptCallback->HandleAndNotifyForcedEvent(interruptEvent);
    EXPECT_EQ(audioInterruptCallback->isForcePaused_, true);
}

/**
 * @tc.name  : Test HandleAndNotifyForcedEvent
 * @tc.number: HandleAndNotifyForcedEvent
 * @tc.desc  : Test HandleAndNotifyForcedEvent
 */
HWTEST(AudioRendererUnitTest, HandleAndNotifyForcedEvent_005, TestSize.Level1)
{
    AudioStreamParams audioStreamParams;
    std::shared_ptr<IAudioStream> audioStream = IAudioStream::GetPlaybackStream(IAudioStream::FAST_STREAM,
        audioStreamParams, STREAM_DEFAULT, 1);
    AudioInterrupt audioInterrupt;
    auto audioInterruptCallback = std::make_shared<AudioRendererInterruptCallbackImpl>(audioStream, audioInterrupt);
    InterruptEventInternal interruptEvent_;
    interruptEvent_.hintType = INTERRUPT_HINT_DUCK;
    audioInterruptCallback->isForcePaused_ = true;
    interruptEvent_.duckVolume = 1.5f;

    const InterruptEventInternal interruptEvent = interruptEvent_;
    std::shared_ptr<TestAudioStremStub> testAudioStremStub = std::make_shared<TestAudioStremStub>();
    testAudioStremStub->state_ = NEW;

    audioInterruptCallback->HandleAndNotifyForcedEvent(interruptEvent);
    EXPECT_EQ(testAudioStremStub->state_, NEW);
}

/**
 * @tc.name  : Test HandleAndNotifyForcedEvent
 * @tc.number: HandleAndNotifyForcedEvent
 * @tc.desc  : Test HandleAndNotifyForcedEvent
 */
HWTEST(AudioRendererUnitTest, HandleAndNotifyForcedEvent_006, TestSize.Level1)
{
    AudioStreamParams audioStreamParams;
    std::shared_ptr<IAudioStream> audioStream = IAudioStream::GetPlaybackStream(IAudioStream::FAST_STREAM,
        audioStreamParams, STREAM_DEFAULT, 1);
    AudioInterrupt audioInterrupt;
    auto audioInterruptCallback = std::make_shared<AudioRendererInterruptCallbackImpl>(audioStream, audioInterrupt);
    InterruptEventInternal interruptEvent_;
    interruptEvent_.hintType = INTERRUPT_HINT_DUCK;
    audioInterruptCallback->isForcePaused_ = true;
    interruptEvent_.duckVolume = 1.5f;

    const InterruptEventInternal interruptEvent = interruptEvent_;
    std::shared_ptr<TestAudioStremStub> testAudioStremStub = std::make_shared<TestAudioStremStub>();
    testAudioStremStub->state_ = NEW;

    audioInterruptCallback->HandleAndNotifyForcedEvent(interruptEvent);
    EXPECT_EQ(testAudioStremStub->state_, NEW);
}

/**
 * @tc.name  : Test RegisterOutputDeviceChangeWithInfoCallback
 * @tc.number: RegisterOutputDeviceChangeWithInfoCallback
 * @tc.desc  : Test RegisterOutputDeviceChangeWithInfoCallback
 */
HWTEST(AudioRendererUnitTest, RegisterOutputDeviceChangeWithInfoCallback_001, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    const std::shared_ptr<AudioRendererOutputDeviceChangeCallback> callback = nullptr;

    int32_t ret = audioRendererPrivate->RegisterOutputDeviceChangeWithInfoCallback(callback);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test InitSwitchInfo
 * @tc.number: InitSwitchInfo
 * @tc.desc  : Test InitSwitchInfo
 */
HWTEST(AudioRendererUnitTest, InitSwitchInfo_001, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    IAudioStream::StreamClass targetClass = IAudioStream::VOIP_STREAM;
    IAudioStream::SwitchInfo info;
    audioRendererPrivate->rendererInfo_.rendererFlags = AUDIO_FLAG_DIRECT;

    audioRendererPrivate->InitSwitchInfo(targetClass, info);
    EXPECT_EQ(info.params.originalSessionId, INVALID_SESSION_ID);
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
 * @tc.name  : Test RestoreTheadLoop
 * @tc.number: RestoreTheadLoop
 * @tc.desc  : Test RestoreTheadLoop
 */
HWTEST(AudioRendererUnitTest, RestoreTheadLoop_001, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);

    const std::shared_ptr<RendererPolicyServiceDiedCallback> serviceCallback =
        std::make_shared<RendererPolicyServiceDiedCallback>();
    std::shared_ptr<FastAudioStream> audioStream =
    std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appInfo.appUid);
    audioRendererPrivate->audioStream_ = audioStream;
    serviceCallback->renderer_ = std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    audioRendererPrivate->abortRestore_ = true;
    audioRendererPrivate->rendererInfo_.streamUsage = STREAM_USAGE_VOICE_MODEM_COMMUNICATION;
    audioRendererPrivate->isEnableVoiceModemCommunicationStartStream_ = false;
    audioRendererPrivate->state_ = RENDERER_RUNNING;

    serviceCallback->RestoreTheadLoop();

    EXPECT_EQ(audioRendererPrivate->abortRestore_, true);
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

    int32_t ret = audioRendererPrivate->PrepareAudioStream(audioStreamParams, audioStreamType, streamClass);
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
 * @tc.name  : Test AudioRendererInterruptCallbackImpl
 * @tc.number: UpdateAudioStream_001
 * @tc.desc  : Test UpdateAudioStream API
 */
HWTEST(AudioRendererUnitTest, UpdateAudioStream_001, TestSize.Level1)
{
    AudioStreamParams audioStreamParams;
    std::shared_ptr<IAudioStream> audioStream = IAudioStream::GetPlaybackStream(IAudioStream::FAST_STREAM,
        audioStreamParams, STREAM_DEFAULT, 1);
    AudioInterrupt audioInterrupt;
    auto audioInterruptCallback = std::make_shared<AudioRendererInterruptCallbackImpl>(audioStream, audioInterrupt);
    std::shared_ptr<TestAudioStremStub> testAudioStremStub = std::make_shared<TestAudioStremStub>();

    audioInterruptCallback->UpdateAudioStream(testAudioStremStub);
    EXPECT_NE(audioInterruptCallback->audioStream_, nullptr);
}

/**
 * @tc.name  : Test AudioRendererPrivate
 * @tc.number: UnregisterOutputDeviceChangeWithInfoCallback_001
 * @tc.desc  : Test UnregisterOutputDeviceChangeWithInfoCallback API
 */
HWTEST(AudioRendererUnitTest, UnregisterOutputDeviceChangeWithInfoCallback_001, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);

    audioRendererPrivate->outputDeviceChangeCallback_ = std::make_shared<OutputDeviceChangeWithInfoCallbackImpl>();
    EXPECT_EQ(audioRendererPrivate->UnregisterOutputDeviceChangeWithInfoCallback(), SUCCESS);
}

/**
 * @tc.name  : Test AudioRendererPrivate
 * @tc.number: UnregisterOutputDeviceChangeWithInfoCallback_002
 * @tc.desc  : Test UnregisterOutputDeviceChangeWithInfoCallback API
 */
HWTEST(AudioRendererUnitTest, UnregisterOutputDeviceChangeWithInfoCallback_002, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    std::shared_ptr<AudioRendererOutputDeviceChangeCallback> callback = nullptr;

    audioRendererPrivate->outputDeviceChangeCallback_ = std::make_shared<OutputDeviceChangeWithInfoCallbackImpl>();
    EXPECT_EQ(audioRendererPrivate->UnregisterOutputDeviceChangeWithInfoCallback(callback), SUCCESS);
}

/**
 * @tc.name  : Test AudioRendererPrivate
 * @tc.number: UpdateRendererAudioStream_001
 * @tc.desc  : Test UpdateRendererAudioStream API
 */
HWTEST(AudioRendererUnitTest, UpdateRendererAudioStream_001, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    std::shared_ptr<IAudioStream> testAudioStremStub = std::make_shared<TestAudioStremStub>();

    audioRendererPrivate->audioInterruptCallback_ = nullptr;
    audioRendererPrivate->UpdateRendererAudioStream(testAudioStremStub);
    EXPECT_EQ(audioRendererPrivate->audioInterruptCallback_, nullptr);

    AudioInterrupt audioInterrupt;
    audioRendererPrivate->audioInterruptCallback_ = std::make_shared<AudioRendererInterruptCallbackImpl>(
        testAudioStremStub, audioInterrupt);
    audioRendererPrivate->UpdateRendererAudioStream(testAudioStremStub);
    EXPECT_NE(audioRendererPrivate->audioInterruptCallback_, nullptr);
}

/**
 * @tc.name  : Test AudioRendererPrivate
 * @tc.number: UnsetOffloadModeInner_001
 * @tc.desc  : Test UnsetOffloadModeInner API
 */
HWTEST(AudioRendererUnitTest, UnsetOffloadModeInner_001, TestSize.Level1)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    std::shared_ptr<IAudioStream> testAudioStremStub = std::make_shared<TestAudioStremStub>();

    audioRendererPrivate->audioStream_ = testAudioStremStub;
    auto ret = audioRendererPrivate->UnsetOffloadModeInner();
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test IsFastStreamClass
 * @tc.number: IsFastStreamClass
 * @tc.desc  : Test IsFastStreamClass
 */
HWTEST(AudioRendererUnitTest, IsFastStreamClass_001, TestSize.Level1)
{
    EXPECT_EQ(IAudioStream::IsFastStreamClass(IAudioStream::PA_STREAM), false);
    EXPECT_EQ(IAudioStream::IsFastStreamClass(IAudioStream::FAST_STREAM), true);
    EXPECT_EQ(IAudioStream::IsFastStreamClass(IAudioStream::VOIP_STREAM), true);
}

/**
 * @tc.name  : Test InitFormatUnsupportedErrorCallback API.
 * @tc.number: Audio_Renderer_InitFormatUnsupportedErrorCallback_001
 * @tc.desc  : Test InitFormatUnsupportedErrorCallback interface.
 */
HWTEST(AudioRendererUnitTest, Audio_Renderer_InitFormatUnsupportedErrorCallback_001, TestSize.Level2)
{
    AppInfo appInfo = {};
    std::shared_ptr<AudioRendererPrivate> audioRendererPrivate =
        std::make_shared<AudioRendererPrivate>(AudioStreamType::STREAM_MEDIA, appInfo);
    int32_t ret = audioRendererPrivate->InitFormatUnsupportedErrorCallback();
    EXPECT_EQ(SUCCESS, ret);
}
} // namespace AudioStandard
} // namespace OHOS