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

#include <mutex>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "audio_errors.h"
#include "audio_info.h"
#include "audio_renderer.h"
#include "fast_audio_stream.h"
#include "audio_renderer_proxy_obj.h"
#include "audio_policy_manager.h"
#include "audio_renderer_private.h"
#include "audio_renderer_unit_test.h"

using namespace std;
using namespace std::chrono;
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
namespace {
constexpr uint32_t STREAM_FAST = 1;
bool g_isFastRenderer = true;
bool g_isInit = false;
} // namespace

class AudioFastRendererUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void InitializeFastRendererOptions(AudioRendererOptions &rendererOptions)
{
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    rendererOptions.streamInfo.channels = AudioChannel::MONO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    rendererOptions.rendererInfo.rendererFlags = STREAM_FAST;
}

shared_ptr<AudioRenderer> GetRenderPtr()
{
    static shared_ptr<AudioRenderer> audioRenderer;
    if (!g_isInit) {
        AudioRendererOptions rendererOptions;
        InitializeFastRendererOptions(rendererOptions);
        audioRenderer = AudioRenderer::Create(rendererOptions);
        g_isInit = true;
    }
    return audioRenderer;
}

void AudioFastRendererUnitTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
    bool isFast = false;
    if (GetRenderPtr() != nullptr) {
        isFast = GetRenderPtr()->IsFastRenderer();
    }

    if (GetRenderPtr() == nullptr || !isFast) {
        g_isFastRenderer = false;
    }
}

InterruptEvent AudioRendererUnitTest::interruptEventTest_ = {};

void AudioFastRendererUnitTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
    g_isInit = false;
    GetRenderPtr()->Stop();
    GetRenderPtr()->Release();
    GetRenderPtr()->Stop();
}

void AudioFastRendererUnitTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void AudioFastRendererUnitTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

/**
 * @tc.name  : Test Fast Renderer
 * @tc.number: Audio_Fast_Renderer_001
 * @tc.desc  : Audio_Fast_Renderer_001
 */
HWTEST_F(AudioFastRendererUnitTest, Audio_Fast_Renderer_001, TestSize.Level0)
{
    if (!g_isFastRenderer) {
        return;
    }
    int32_t ret = -1;
    ASSERT_NE(nullptr, GetRenderPtr());

    float volume = GetRenderPtr()->GetVolume();
    EXPECT_EQ(1.0, volume);

    ret = GetRenderPtr()->SetVolume(0.5);
    EXPECT_EQ(SUCCESS, ret);

    float volume1 = GetRenderPtr()->GetVolume();
    EXPECT_EQ(0.5, volume1);
}

/**
 * @tc.name  : Test Fast Renderer
 * @tc.number: Audio_Fast_Renderer_002
 * @tc.desc  : Audio_Fast_Renderer_002
 */
HWTEST_F(AudioFastRendererUnitTest, Audio_Fast_Renderer_002, TestSize.Level0)
{
    if (!g_isFastRenderer) {
        return;
    }
    int32_t ret = -1;
    ASSERT_NE(nullptr, GetRenderPtr());

    shared_ptr<RendererPositionCallbackTest> positionCB = std::make_shared<RendererPositionCallbackTest>();
    ret = GetRenderPtr()->SetRendererPositionCallback(RenderUT::VALUE_THOUSAND, positionCB);
    EXPECT_EQ(SUCCESS, ret);

    GetRenderPtr()->UnsetRendererPositionCallback();

    shared_ptr<RendererPositionCallbackTest> positionCB1 = std::make_shared<RendererPositionCallbackTest>();
    ret = GetRenderPtr()->SetRendererPositionCallback(RenderUT::VALUE_THOUSAND, positionCB1);
    EXPECT_EQ(SUCCESS, ret);

    AudioRendererParams getRendererParams;
    ret = GetRenderPtr()->GetParams(getRendererParams);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Fast Renderer
 * @tc.number: Audio_Fast_Renderer_003
 * @tc.desc  : Audio_Fast_Renderer_003
 */
HWTEST_F(AudioFastRendererUnitTest, Audio_Fast_Renderer_003, TestSize.Level1)
{
    if (!g_isFastRenderer) {
        return;
    }
    int32_t ret = -1;
    FILE *wavFile = fopen(RenderUT::AUDIORENDER_TEST_FILE_PATH.c_str(), "rb");
    ASSERT_NE(nullptr, wavFile);
    ASSERT_NE(nullptr, GetRenderPtr());

    uint64_t latency;
    ret = GetRenderPtr()->GetLatency(latency);
    EXPECT_EQ(SUCCESS, ret);

    uint32_t frameCount;
    ret = GetRenderPtr()->GetFrameCount(frameCount);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = GetRenderPtr()->Start();
    EXPECT_EQ(true, isStarted);

    Timestamp timestamp;
    bool getAudioTime = GetRenderPtr()->GetAudioTime(timestamp, Timestamp::Timestampbase::MONOTONIC);
    EXPECT_EQ(true, getAudioTime);

    size_t bufferLen;
    ret = GetRenderPtr()->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);
    ASSERT_NE(nullptr, buffer);

    size_t bytesToWrite = fread(buffer, 1, bufferLen, wavFile);
    int32_t written = GetRenderPtr()->Write(buffer, bytesToWrite);
    EXPECT_GE(written, ERR_INVALID_OPERATION);
}

/**
 * @tc.name  : Test Fast Renderer
 * @tc.number: Audio_Fast_Renderer_004
 * @tc.desc  : Audio_Fast_Renderer_004
 */
HWTEST_F(AudioFastRendererUnitTest, Audio_Fast_Renderer_004, TestSize.Level1)
{
    if (!g_isFastRenderer) {
        return;
    }
    ASSERT_NE(nullptr, GetRenderPtr());

    int32_t ret = GetRenderPtr()->GetUnderflowCount();
    EXPECT_GE(ret, SUCCESS);

    Timestamp timestamp;
    bool getAudioTime = GetRenderPtr()->GetAudioTime(timestamp, Timestamp::Timestampbase::MONOTONIC);
    EXPECT_EQ(true, getAudioTime);
}

/**
 * @tc.name  : Test Fast Renderer
 * @tc.number: Audio_Fast_Renderer_005
 * @tc.desc  : Audio_Fast_Renderer_005
 */
HWTEST_F(AudioFastRendererUnitTest, Audio_Fast_Renderer_005, TestSize.Level1)
{
    if (!g_isFastRenderer) {
        return;
    }
    int32_t ret = -1;
    ASSERT_NE(nullptr, GetRenderPtr());

    AudioRenderMode renderMode = GetRenderPtr()->GetRenderMode();
    EXPECT_EQ(RENDER_MODE_NORMAL, renderMode);

    shared_ptr<AudioRendererWriteCallback> cb = make_shared<AudioRenderModeCallbackTest>();
    ret = GetRenderPtr()->SetRendererWriteCallback(cb);
    EXPECT_EQ(SUCCESS, ret);

    BufferQueueState bQueueSate {};
    bQueueSate.currentIndex = 1;
    bQueueSate.numBuffers = 1;

    ret = GetRenderPtr()->GetBufQueueState(bQueueSate);
    EXPECT_EQ(ERR_INVALID_HANDLE, ret);

    ret = GetRenderPtr()->SetRenderRate(RENDER_RATE_DOUBLE);
    EXPECT_EQ(SUCCESS, ret);

    ret = GetRenderPtr()->SetRenderRate(RENDER_RATE_NORMAL);
    EXPECT_EQ(SUCCESS, ret);

    AudioRendererRate renderRate = GetRenderPtr()->GetRenderRate();
    EXPECT_EQ(RENDER_RATE_NORMAL, renderRate);

    shared_ptr<AudioRendererCallbackTest> audioRendererCB = make_shared<AudioRendererCallbackTest>();
    ret = GetRenderPtr()->SetRendererCallback(audioRendererCB);
    EXPECT_EQ(SUCCESS, ret);

    BufferDesc bufDesc {};
    bufDesc.buffer = nullptr;
    bufDesc.dataLength = RenderUT::g_reqBufLen;
    ret = GetRenderPtr()->GetBufferDesc(bufDesc);
    EXPECT_EQ(ERR_INVALID_HANDLE, ret);
    EXPECT_EQ(nullptr, bufDesc.buffer);

    ret = GetRenderPtr()->Enqueue(bufDesc);
    EXPECT_EQ(ERR_INVALID_HANDLE, ret);

    ret = GetRenderPtr()->Clear();
    EXPECT_EQ(ERR_INVALID_HANDLE, ret);
}

/**
 * @tc.name  : Test Fast Renderer
 * @tc.number: Audio_Fast_Renderer_006
 * @tc.desc  : Audio_Fast_Renderer_006
 */
HWTEST_F(AudioFastRendererUnitTest, Audio_Fast_Renderer_006, TestSize.Level1)
{
    if (!g_isFastRenderer) {
        return;
    }
    int32_t ret = -1;
    ASSERT_NE(nullptr, GetRenderPtr());

    ret = GetRenderPtr()->SetRenderMode(RENDER_MODE_NORMAL);
    // If the audiorenderer does not enter low-latency mode but enters normal mode, the err code is ERR_INCORRECT_MODE.
    EXPECT_EQ(ret, SUCCESS);

    ret = GetRenderPtr()->SetRenderMode(RENDER_MODE_CALLBACK);
    EXPECT_EQ(ERR_ILLEGAL_STATE, ret);

    AudioRenderMode renderMode = GetRenderPtr()->GetRenderMode();
    EXPECT_EQ(RENDER_MODE_NORMAL, renderMode);

    shared_ptr<AudioRendererWriteCallback> cb = make_shared<AudioRenderModeCallbackTest>();
    ret = GetRenderPtr()->SetRendererWriteCallback(cb);
    EXPECT_EQ(ERR_INVALID_HANDLE, ret);
}

/**
 * @tc.name  : Test GetRenderRate
 * @tc.number: Audio_Fast_Renderer_007
 * @tc.desc  : Audio_Fast_Renderer_007
 */
HWTEST_F(AudioFastRendererUnitTest, Audio_Fast_Renderer_007, TestSize.Level1)
{
    if (!g_isFastRenderer) {
        return;
    }
    int32_t ret = -1;
    ASSERT_NE(nullptr, GetRenderPtr());

    int32_t setLowPowerVolume = GetRenderPtr()->SetLowPowerVolume(1.0f);
    EXPECT_EQ(setLowPowerVolume, SUCCESS);

    float getLowPowerVolume = GetRenderPtr()->GetLowPowerVolume();
    EXPECT_EQ(getLowPowerVolume, 1.0f);

    float getSingleStreamVolume = GetRenderPtr()->GetSingleStreamVolume();
    EXPECT_EQ(getSingleStreamVolume, 1.0f);

    ret = GetRenderPtr()->SetAudioEffectMode(EFFECT_NONE);
    // If the audiorenderer does not enter low-latency mode but enters normal mode, the err code is ERR_INCORRECT_MODE.
    EXPECT_THAT(ret, AnyOf(Eq(ERR_NOT_SUPPORTED), Eq(SUCCESS)));

    AudioEffectMode effectMode = GetRenderPtr()->GetAudioEffectMode();
    EXPECT_EQ(EFFECT_NONE, effectMode);

    bool isPaused = GetRenderPtr()->Pause();
    EXPECT_EQ(true, isPaused);
}

/**
 * @tc.name  : Test GetRenderRate
 * @tc.number: Audio_Fast_Renderer_008
 * @tc.desc  : Audio_Fast_Renderer_008
 */
HWTEST_F(AudioFastRendererUnitTest, Audio_Fast_Renderer_008, TestSize.Level1)
{
    if (!g_isFastRenderer) {
        return;
    }
    ASSERT_NE(nullptr, GetRenderPtr());

    // If the audiorenderer does not enter low-latency mode but enters normal mode, flush will return false in prepare.
    GetRenderPtr()->Flush();

    bool isStarted = GetRenderPtr()->Start();
    EXPECT_EQ(true, isStarted);

    bool isDrained = GetRenderPtr()->Drain();
    EXPECT_EQ(true, isDrained);

    bool isStopped = GetRenderPtr()->Stop();
    EXPECT_EQ(true, isStopped);

    bool isFlushed1 = GetRenderPtr()->Flush();
    EXPECT_EQ(true, isFlushed1);
}

/**
 * @tc.name  : Test GetRenderRate
 * @tc.number: Audio_Fast_Renderer_009
 * @tc.desc  : Audio_Fast_Renderer_009
 */
HWTEST_F(AudioFastRendererUnitTest, Audio_Fast_Renderer_009, TestSize.Level1)
{
    if (!g_isFastRenderer) {
        return;
    }
    int32_t ret = -1;
    ASSERT_NE(nullptr, GetRenderPtr());

    uint32_t sampleRate = AudioSamplingRate::SAMPLE_RATE_96000;
    ret = GetRenderPtr()->SetRendererSamplingRate(sampleRate);
    EXPECT_NE(SUCCESS, ret);

    uint32_t getSampleRateRet = GetRenderPtr()->GetRendererSamplingRate();
    EXPECT_EQ(getSampleRateRet, 48000);

    shared_ptr<RendererPeriodPositionCallbackTest> positionCB1 = std::make_shared<RendererPeriodPositionCallbackTest>();
    ret = GetRenderPtr()->SetRendererPeriodPositionCallback(RenderUT::VALUE_ZERO, positionCB1);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);

    ret = GetRenderPtr()->SetRendererPeriodPositionCallback(RenderUT::VALUE_THOUSAND, positionCB1);
    EXPECT_EQ(SUCCESS, ret);

    GetRenderPtr()->UnsetRendererPeriodPositionCallback();

    shared_ptr<RendererPeriodPositionCallbackTest> positionCB2 = std::make_shared<RendererPeriodPositionCallbackTest>();
    ret = GetRenderPtr()->SetRendererPeriodPositionCallback(RenderUT::VALUE_THOUSAND, positionCB2);
    EXPECT_EQ(SUCCESS, ret);

    bool isStopped = GetRenderPtr()->Stop();
    EXPECT_EQ(true, isStopped);
}

/**
 * @tc.name  : Test Fast Renderer
 * @tc.number: Audio_Fast_Renderer_010
 * @tc.desc  : Audio_Fast_Renderer_010
 */
HWTEST_F(AudioFastRendererUnitTest, Audio_Fast_Renderer_010, TestSize.Level1)
{
    if (!g_isFastRenderer) {
        return;
    }
    int32_t setFrameSize = 960;
    uint32_t getFrameSize = 0;
    ASSERT_NE(nullptr, GetRenderPtr());

    GetRenderPtr()->SetPreferredFrameSize(setFrameSize);
    bool gotFrameSize = GetRenderPtr()->GetFrameCount(getFrameSize);
    EXPECT_EQ(SUCCESS, gotFrameSize);
    EXPECT_EQ(static_cast<uint32_t>(setFrameSize), getFrameSize);

    bool isStarted = GetRenderPtr()->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = GetRenderPtr()->Stop();
    EXPECT_EQ(true, isStopped);
}

/**
 * @tc.name  : Test Fast Renderer
 * @tc.number: Audio_Fast_Renderer_011
 * @tc.desc  : Audio_Fast_Renderer_011
 */
#ifdef ENABLE_FAST_RENDERER_CALLBACK_TEST
HWTEST_F(AudioFastRendererUnitTest, Audio_Fast_Renderer_011, TestSize.Level1)
{
    if (!g_isFastRenderer) {
        return;
    }
    AudioRendererOptions rendererOptions;

    InitializeFastRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    int32_t ret = audioRenderer->SetRenderMode(RENDER_MODE_CALLBACK);
    EXPECT_EQ(SUCCESS, ret);

    shared_ptr<AudioRendererWriteCallbackMock> cb = make_shared<AudioRendererWriteCallbackMock>();

    ret = audioRenderer->SetRendererWriteCallback(cb);
    EXPECT_EQ(SUCCESS, ret);

    bool isFastRendererMode = audioRenderer->IsFastRenderer();
    const auto sleepTime = isFastRendererMode ? 20ms : 200ms;

    std::mutex mutex;
    std::condition_variable cv;
    int32_t count = 0;
    cb->Install([&count, &audioRenderer, &mutex, &cv, sleepTime](size_t length) {
                std::lock_guard lock(mutex);
                cv.notify_one();
                // only execute twice
                if (count > 1) {
                    return;
                }
                // sleep time trigger underflow
                if (count == 1) {
                    std::this_thread::sleep_for(sleepTime);
                }
                count++;
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

    std::unique_lock lock(mutex);
    cv.wait_for(lock, 1s, [&count] {
        // count > 1 ensure sleeped
        return count > 1;
    });
    lock.unlock();

    // Verify that the callback is invoked at least once
    EXPECT_GE(cb->GetExeCount(), 1);

    auto underFlowCount = audioRenderer->GetUnderflowCount();
    // Ensure the underflowCount is at least 1
    EXPECT_GE(underFlowCount, 1);

    audioRenderer->Stop();
    audioRenderer->Release();
}
#endif
} // namespace AudioStandard
} // namespace OHOS