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

} // namespace AudioStandard
} // namespace OHOS
