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
#include <thread>

#include <memory>

#include <securec.h>
#include "gtest/gtest.h"

#include "audio_capturer.h"
#include "audio_errors.h"
#include "audio_info.h"
#include "audio_capturer_log.h"
#include "audio_renderer.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace {
const int32_t READ_BUFFERS_MAX_COUNT = 100;
const int32_t VALID_DATA_COUNT = 20;
const size_t SHORT_SLEEP_TIME = 100000; // us 100ms
} // namespace

class InnerCapturerUnitTest : public testing::Test {
public:
    // SetUpTestCase: Called before all test cases
    static void SetUpTestCase(void);
    // TearDownTestCase: Called after all test case
    static void TearDownTestCase(void);
    // SetUp: Called before each test cases
    void SetUp(void);
    // TearDown: Called after each test cases
    void TearDown(void);
    // Init Capturer Options
    static AudioCapturerOptions GetCapturerOptions(AudioPlaybackCaptureConfig config);
};

void InnerCapturerUnitTest::SetUpTestCase(void) {}
void InnerCapturerUnitTest::TearDownTestCase(void) {}
void InnerCapturerUnitTest::SetUp(void) {}
void InnerCapturerUnitTest::TearDown(void) {}

class MockRenderer : public AudioRendererWriteCallback, public std::enable_shared_from_this<MockRenderer> {
public:
    MockRenderer() {};

    ~MockRenderer();

    void OnWriteData(size_t length) override;

    bool InitRenderer(StreamUsage usage, AudioPrivacyType type);

    bool Start();

    bool Stop();

private:
    void InitBuffer();
private:
    std::unique_ptr<AudioRenderer> audioRenderer_ = nullptr;
    std::unique_ptr<uint8_t []> cacheBuffer_ = nullptr;
    size_t cacheBufferSize_ = 0;
    size_t bytesAlreadyWrite_ = 0;
};

MockRenderer::~MockRenderer()
{
    if (audioRenderer_ != nullptr) {
        audioRenderer_->Release();
    }
}

void MockRenderer::OnWriteData(size_t length)
{
    if (audioRenderer_ == nullptr) {
        return;
    }
    BufferDesc buffer = { nullptr, 0, 0};
    audioRenderer_->GetBufferDesc(buffer);
    if (buffer.buffer == nullptr) {
        return  ;
    }
    if (length > buffer.bufLength) {
        buffer.dataLength = buffer.bufLength;
    } else {
        buffer.dataLength = length;
    }

    int ret = memcpy_s(static_cast<void *>(buffer.buffer), buffer.dataLength,
        static_cast<void *>(cacheBuffer_.get()), cacheBufferSize_);
    if (ret != EOK) {
        AUDIO_ERR_LOG("OnWriteData failed");
    }

    bytesAlreadyWrite_ += buffer.dataLength;
    audioRenderer_->Enqueue(buffer);
}

void MockRenderer::InitBuffer()
{
    cacheBuffer_ = std::make_unique<uint8_t []>(cacheBufferSize_);
    const int channels = 2; // 2 channels
    const int samplePerChannel = cacheBufferSize_ / channels; // 1920 for 20ms

    int16_t *signalData = reinterpret_cast<int16_t *>(cacheBuffer_.get());
    int16_t bound = 10;
    for (int idx = 0; idx < samplePerChannel; idx++) {
        signalData[channels * idx] = bound + static_cast<int16_t>(sinf(2.0f * static_cast<float>(M_PI) * idx /
            samplePerChannel) * (SHRT_MAX - bound));
        for (int c = 1; c < channels; c++) {
            signalData[channels * idx + c] = signalData[channels * idx];
        }
    }
}

bool MockRenderer::InitRenderer(StreamUsage usage, AudioPrivacyType type)
{
    AudioRendererOptions rendererOptions = {};
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    rendererOptions.streamInfo.channels = AudioChannel::STEREO;

    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_UNKNOWN;
    rendererOptions.rendererInfo.streamUsage = usage;
    rendererOptions.rendererInfo.rendererFlags = 0;

    rendererOptions.privacyType = type;

    audioRenderer_ = AudioRenderer::Create(rendererOptions);
    if (audioRenderer_ == nullptr) {
        AUDIO_ERR_LOG("RenderCallbackTest: Renderer create failed");
        return false;
    }

    size_t targetSize = 0;
    int32_t ret = audioRenderer_->GetBufferSize(targetSize);

    AUDIO_INFO_LOG("RenderCallbackTest: Playback renderer created");
    if (audioRenderer_->SetRenderMode(RENDER_MODE_CALLBACK)) {
        AUDIO_ERR_LOG("RenderCallbackTest: SetRenderMode failed");
        return false;
    }

    if (ret == 0 && targetSize != 0) {
        size_t bufferDuration = 20; // 20 -> 20ms
        audioRenderer_->SetBufferDuration(bufferDuration);
        cacheBufferSize_ = targetSize;
        InitBuffer();
    } else {
        AUDIO_ERR_LOG("Init renderer failed size:%{public}zu, ret:%{public}d", targetSize, ret);
        return false;
    }

    if (audioRenderer_->SetRendererWriteCallback(shared_from_this())) {
        AUDIO_ERR_LOG("RenderCallbackTest: SetRendererWriteCallback failed");
        return false;
    }
    return true;
}

bool MockRenderer::Start()
{
    if (audioRenderer_ == nullptr) {
        return false;
    }

    return audioRenderer_->Start();
}


bool MockRenderer::Stop()
{
    if (audioRenderer_ == nullptr) {
        return false;
    }

    return audioRenderer_->Stop();
}

AudioCapturerOptions InnerCapturerUnitTest::GetCapturerOptions(AudioPlaybackCaptureConfig config)
{
    AudioCapturerOptions capturerOptions;

    capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    capturerOptions.streamInfo.channels = AudioChannel::STEREO;

    capturerOptions.capturerInfo.sourceType = SourceType::SOURCE_TYPE_PLAYBACK_CAPTURE;
    capturerOptions.capturerInfo.capturerFlags = 0;

    capturerOptions.playbackCaptureConfig = config;

    return capturerOptions;
}

/**
* @tc.name  : Test Create.
* @tc.number: Inner_Capturer_Basic_001
* @tc.desc  : Test PLAYBACK_CAPTURE
*/
HWTEST(InnerCapturerUnitTest, Inner_Capturer_Basic_001, TestSize.Level1)
{
    AudioPlaybackCaptureConfig config = {{{}, FilterMode::INCLUDE, {}, FilterMode::INCLUDE}, false};

    config.filterOptions.usages.emplace_back(STREAM_USAGE_MEDIA);
    config.filterOptions.usages.emplace_back(STREAM_USAGE_ALARM);
    
    AudioCapturerOptions capturerOptions = InnerCapturerUnitTest::GetCapturerOptions(config);

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer) << "Create failed!";
    audioCapturer->Release();
}

/**
* @tc.name  : Test Create and start.
* @tc.number: Inner_Capturer_Basic_002
* @tc.desc  : Test PLAYBACK_CAPTURE
*/
HWTEST(InnerCapturerUnitTest, Inner_Capturer_Basic_002, TestSize.Level1)
{
    AudioPlaybackCaptureConfig config = {{{}, FilterMode::INCLUDE, {}, FilterMode::INCLUDE}, false};

    config.filterOptions.usages.emplace_back(STREAM_USAGE_MEDIA);
    config.filterOptions.usages.emplace_back(STREAM_USAGE_ALARM);
    
    AudioCapturerOptions capturerOptions = InnerCapturerUnitTest::GetCapturerOptions(config);

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool res = audioCapturer->Start();
    ASSERT_EQ(res, true) << "Start failed!";

    usleep(SHORT_SLEEP_TIME);
    
    res = audioCapturer->Stop();
    ASSERT_EQ(res, true) << "Stop failed!";

    audioCapturer->Release();
}

/**
* @tc.name  : Test Create and start.
* @tc.number: Inner_Capturer_Basic_003
* @tc.desc  : Test PLAYBACK_CAPTURE
*/
HWTEST(InnerCapturerUnitTest, Inner_Capturer_Basic_003, TestSize.Level1)
{
    AudioPlaybackCaptureConfig config = {{{}, FilterMode::INCLUDE, {}, FilterMode::INCLUDE}, false};

    config.filterOptions.usages.emplace_back(STREAM_USAGE_MEDIA);
    config.filterOptions.usages.emplace_back(STREAM_USAGE_ALARM);
    
    AudioCapturerOptions capturerOptions = InnerCapturerUnitTest::GetCapturerOptions(config);

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool res = audioCapturer->Start();
    ASSERT_EQ(res, true) << "Start failed!";

    usleep(SHORT_SLEEP_TIME);

    config.filterOptions.usages = {STREAM_USAGE_MUSIC};
    int32_t ret = audioCapturer->UpdatePlaybackCaptureConfig(config);
    ASSERT_EQ(SUCCESS, ret) << "UpdatePlaybackCaptureConfig failed!";

    usleep(SHORT_SLEEP_TIME);
    
    res = audioCapturer->Stop();
    ASSERT_EQ(res, true) << "Stop failed!";

    audioCapturer->Release();
}

/**
* @tc.name  : Test Create and start and write.
* @tc.number: Inner_Capturer_Basic_004
* @tc.desc  : Test PLAYBACK_CAPTURE
*/
HWTEST(InnerCapturerUnitTest, Inner_Capturer_Basic_004, TestSize.Level1)
{
    AudioPlaybackCaptureConfig config = {{{}, FilterMode::INCLUDE, {}, FilterMode::INCLUDE}, false};

    config.filterOptions.usages.emplace_back(STREAM_USAGE_MEDIA);
    config.filterOptions.usages.emplace_back(STREAM_USAGE_ALARM);
    
    AudioCapturerOptions capturerOptions = InnerCapturerUnitTest::GetCapturerOptions(config);

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool res = audioCapturer->Start();
    ASSERT_EQ(res, true) << "Start failed!";

    size_t bufferLen;
    int32_t ret = audioCapturer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret) << "GetBufferSize failed!";

    auto buffer = std::make_unique<uint8_t[]>(bufferLen);

    int32_t bytesRead = 0;
    int32_t numBuffersToCapture = READ_BUFFERS_MAX_COUNT;

    while (numBuffersToCapture) {
        bytesRead = audioCapturer->Read(*(buffer.get()), bufferLen, true);
        if (bytesRead < 0) {
            break;
        }
        numBuffersToCapture--;
    }

    res = audioCapturer->Stop();
    ASSERT_EQ(res, true) << "Stop failed!";

    audioCapturer->Release();
}

// Test play target usage and start inner-cap the target usage, check data not empty.
static void TestInnerCapturer(StreamUsage targetUsage)
{
    AudioPlaybackCaptureConfig config = {{{}, FilterMode::INCLUDE, {}, FilterMode::INCLUDE}, false};

    config.filterOptions.usages.emplace_back(targetUsage);
    
    AudioCapturerOptions capturerOptions = InnerCapturerUnitTest::GetCapturerOptions(config);

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    std::shared_ptr<MockRenderer> renderer = std::make_shared<MockRenderer>();
    renderer->InitRenderer(targetUsage, PRIVACY_TYPE_PUBLIC);
    renderer->Start();
    usleep(SHORT_SLEEP_TIME);

    ASSERT_EQ(audioCapturer->Start(), true) << "Start failed!";

    size_t bufferLen;
    int32_t ret = audioCapturer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret) << "GetBufferSize failed!";

    auto buffer = std::make_unique<uint8_t[]>(bufferLen);
    ASSERT_NE(nullptr, buffer);
    int32_t bytesRead = 0;
    int32_t numBuffersToCapture = READ_BUFFERS_MAX_COUNT;

    int32_t notEmptyCount = 0;
    while (numBuffersToCapture) {
        bytesRead = audioCapturer->Read(*(buffer.get()), bufferLen, true);
        if (bytesRead < 0) {
            break;
        }
        if (*(buffer.get()) != 0 && notEmptyCount++ > VALID_DATA_COUNT) {
            break;
        }
        numBuffersToCapture--;
    }

    ASSERT_GE(notEmptyCount, VALID_DATA_COUNT) << "too less empty data";

    renderer->Stop();
    ASSERT_EQ(audioCapturer->Stop(), true) << "Stop failed!";

    audioCapturer->Release();
}

/**
* @tc.name  : Test capturer UNKNOWN.
* @tc.number: Inner_Capturer_Basic_005
* @tc.desc  : Test PLAYBACK_CAPTURE
*/
HWTEST(InnerCapturerUnitTest, Inner_Capturer_Basic_005, TestSize.Level1)
{
    TestInnerCapturer(STREAM_USAGE_UNKNOWN);
}

/**
* @tc.name  : Test capturer MEDIA.
* @tc.number: Inner_Capturer_Basic_006
* @tc.desc  : Test PLAYBACK_CAPTURE
*/
HWTEST(InnerCapturerUnitTest, Inner_Capturer_Basic_006, TestSize.Level1)
{
    TestInnerCapturer(STREAM_USAGE_MEDIA);
}

/**
* @tc.name  : Test capturer MUSIC.
* @tc.number: Inner_Capturer_Basic_007
* @tc.desc  : Test PLAYBACK_CAPTURE
*/
HWTEST(InnerCapturerUnitTest, Inner_Capturer_Basic_007, TestSize.Level1)
{
    TestInnerCapturer(STREAM_USAGE_MUSIC);
}

/**
* @tc.name  : Test capturer ALARM.
* @tc.number: Inner_Capturer_Basic_008
* @tc.desc  : Test PLAYBACK_CAPTURE
*/
HWTEST(InnerCapturerUnitTest, Inner_Capturer_Basic_008, TestSize.Level1)
{
    TestInnerCapturer(STREAM_USAGE_ALARM);
}

/**
* @tc.name  : Test capturer RINGTONE.
* @tc.number: Inner_Capturer_Basic_009
* @tc.desc  : Test PLAYBACK_CAPTURE
*/
HWTEST(InnerCapturerUnitTest, Inner_Capturer_Basic_009, TestSize.Level1)
{
    TestInnerCapturer(STREAM_USAGE_RINGTONE);
}

/**
* @tc.name  : Test capturer NOTIFICATION.
* @tc.number: Inner_Capturer_Basic_0010
* @tc.desc  : Test PLAYBACK_CAPTURE
*/
HWTEST(InnerCapturerUnitTest, Inner_Capturer_Basic_0010, TestSize.Level1)
{
    TestInnerCapturer(STREAM_USAGE_NOTIFICATION);
}

/**
* @tc.name  : Test capturer SYSTEM.
* @tc.number: Inner_Capturer_Basic_0011
* @tc.desc  : Test PLAYBACK_CAPTURE
*/
HWTEST(InnerCapturerUnitTest, Inner_Capturer_Basic_0011, TestSize.Level1)
{
    TestInnerCapturer(STREAM_USAGE_SYSTEM);
}

/**
* @tc.name  : Test capturer MOVIE.
* @tc.number: Inner_Capturer_Basic_0012
* @tc.desc  : Test PLAYBACK_CAPTURE
*/
HWTEST(InnerCapturerUnitTest, Inner_Capturer_Basic_0012, TestSize.Level1)
{
    TestInnerCapturer(STREAM_USAGE_MOVIE);
}

/**
* @tc.name  : Test capturer GAME.
* @tc.number: Inner_Capturer_Basic_0013
* @tc.desc  : Test PLAYBACK_CAPTURE
*/
HWTEST(InnerCapturerUnitTest, Inner_Capturer_Basic_0013, TestSize.Level1)
{
    TestInnerCapturer(STREAM_USAGE_GAME);
}

/**
* @tc.name  : Test capturer AUDIOBOOK.
* @tc.number: Inner_Capturer_Basic_0014
* @tc.desc  : Test PLAYBACK_CAPTURE
*/
HWTEST(InnerCapturerUnitTest, Inner_Capturer_Basic_0014, TestSize.Level1)
{
    TestInnerCapturer(STREAM_USAGE_AUDIOBOOK);
}

/**
* @tc.name  : Test capturer NAVIGATION.
* @tc.number: Inner_Capturer_Basic_0015
* @tc.desc  : Test PLAYBACK_CAPTURE
*/
HWTEST(InnerCapturerUnitTest, Inner_Capturer_Basic_0015, TestSize.Level1)
{
    TestInnerCapturer(STREAM_USAGE_NAVIGATION);
}

/**
* @tc.name  : Test capturer EXCLUDE self.
* @tc.number: Inner_Capturer_Basic_0016
* @tc.desc  : Test PLAYBACK_CAPTURE
*/
HWTEST(InnerCapturerUnitTest, Inner_Capturer_Basic_0016, TestSize.Level1)
{
    AudioPlaybackCaptureConfig config = {{{}, FilterMode::INCLUDE, {}, FilterMode::INCLUDE}, false};

    config.filterOptions.usages.emplace_back(STREAM_USAGE_MUSIC);
    config.filterOptions.pids.emplace_back(getpid());
    config.filterOptions.pidFilterMode = FilterMode::EXCLUDE;
    
    AudioCapturerOptions capturerOptions = InnerCapturerUnitTest::GetCapturerOptions(config);

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    std::shared_ptr<MockRenderer> renderer = std::make_shared<MockRenderer>();
    renderer->InitRenderer(STREAM_USAGE_MUSIC, PRIVACY_TYPE_PUBLIC);
    renderer->Start();
    usleep(SHORT_SLEEP_TIME);

    ASSERT_EQ(audioCapturer->Start(), true) << "Start failed!";

    size_t bufferLen;
    int32_t ret = audioCapturer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret) << "GetBufferSize failed!";

    auto buffer = std::make_unique<uint8_t[]>(bufferLen);

    int32_t numBuffersToCapture = READ_BUFFERS_MAX_COUNT;

    int32_t notEmptyCount = 0;
    while (numBuffersToCapture) {
        if (audioCapturer->Read(*(buffer.get()), bufferLen, true) < 0) {
            break;
        }
        if (*(buffer.get()) != 0 && notEmptyCount++ > VALID_DATA_COUNT) {
            break;
        }
        numBuffersToCapture--;
    }

    ASSERT_EQ(notEmptyCount, 0) << "should get only empty data";

    renderer->Stop();
    ASSERT_EQ(audioCapturer->Stop(), true) << "Stop failed!";

    audioCapturer->Release();
}
} // namespace AudioStandard
} // namespace OHOS
