/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#ifndef LOG_TAG
#define LOG_TAG "AudioRendererMultiTest"
#endif

#include "audio_renderer_multithread_test.h"

#include <chrono>
#include <thread>

#include "audio_errors.h"
#include "audio_info.h"
#include "audio_renderer_log.h"
#include "audio_renderer.h"
#include "audio_renderer_proxy_obj.h"
#include "audio_policy_manager.h"
#include "audio_renderer_private.h"

using namespace std;
using namespace std::chrono;
using namespace testing::ext;
using namespace testing::mt;
namespace OHOS {
namespace AudioStandard {
namespace {
    const string AUDIORENDER_TEST_FILE_PATH = "/data/test_44100_2.wav";
    const int32_t RENDERER_FLAG = 0;
    const int32_t WRITE_BUFFERS_COUNT = 1000;
    const int32_t VALUE_ZERO = 0;
    const int32_t MAX_INSTANCE_NUM = 16;
} // namespace

void AudioRendererMultithreadTest::SetUpTestCase(void) {}
void AudioRendererMultithreadTest::TearDownTestCase(void) {}
void AudioRendererMultithreadTest::SetUp(void) {}
void AudioRendererMultithreadTest::TearDown(void) {}

void AudioRendererMultithreadTest::InitializeRendererOptions(AudioRendererOptions &rendererOptions)
{
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    rendererOptions.streamInfo.channels = AudioChannel::STEREO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    rendererOptions.rendererInfo.rendererFlags = RENDERER_FLAG;

    return;
}

void AudioRendererMultithreadTest::Write(unique_ptr<AudioRenderer> &audioRenderer)
{
    AUDIO_INFO_LOG("RendererMultiTest Write");
    FILE *wavFile = fopen(AUDIORENDER_TEST_FILE_PATH.c_str(), "rb");
    ASSERT_NE(nullptr, wavFile);
    int32_t ret = -1;
    size_t bufferLen;
    ret = audioRenderer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);
    ASSERT_NE(nullptr, buffer);

    size_t bytesToWrite = 0;
    int32_t bytesWritten = 0;
    size_t minBytes = 4;
    int32_t numBuffersToRender = WRITE_BUFFERS_COUNT;

    int32_t failCount = 10;
    while (numBuffersToRender) {
        bytesToWrite = fread(buffer, 1, bufferLen, wavFile);
        bytesWritten = 0;
        while ((static_cast<size_t>(bytesWritten) < bytesToWrite) &&
            ((static_cast<size_t>(bytesToWrite) - bytesWritten) > minBytes)) {
            bytesWritten += audioRenderer->Write(buffer + static_cast<size_t>(bytesWritten),
                                                 bytesToWrite - static_cast<size_t>(bytesWritten));
            EXPECT_GE(bytesWritten, VALUE_ZERO);
            if (bytesWritten < 0) {
                failCount--;
                break;
            }
        }
        if (failCount < 0) {
            break;
        }

        numBuffersToRender--;
    }

    free(buffer);
    fclose(wavFile);
}

void RendererMultiTest()
{
    AudioRendererOptions rendererOptions;
    AudioRendererMultithreadTest::InitializeRendererOptions(rendererOptions);

    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    EXPECT_NE(nullptr, audioRenderer);
    if (audioRenderer == nullptr) {
        AUDIO_ERR_LOG("RendererMultiTest AudioRenderer::Create Faild");
        return;
    }

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    AudioRendererMultithreadTest::Write(audioRenderer);
    bool isPaused = audioRenderer->Pause();
    EXPECT_EQ(true, isPaused);

    isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    AudioRendererMultithreadTest::Write(audioRenderer);
    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    AudioRendererMultithreadTest::Write(audioRenderer);

    isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    audioRenderer->Drain();
    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);

    AUDIO_INFO_LOG("RendererMultiTest is called");
}

HWTEST(AudioRendererMultithreadTest, Audio_Renderer_001, TestSize.Level1)
{
    SET_THREAD_NUM(MAX_INSTANCE_NUM);
    GTEST_RUN_TASK(RendererMultiTest);
}
} // namespace AudioStandard
} // namespace OHOS