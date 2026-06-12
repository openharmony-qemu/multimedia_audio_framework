/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef AUDIO_RENDERER_FIRSTFRAME_MOCK_TEST_H
#define AUDIO_RENDERER_FIRSTFRAME_MOCK_TEST_H

#include <functional>
#include <string>
#include "securec.h"
#include "audio_info.h"
#include "audio_errors.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "audio_renderer.h"

namespace OHOS {
namespace AudioStandard {

namespace FirstFrameMockUT {
constexpr int32_t SUCCESS = 0;
constexpr int32_t RENDERER_FLAG = 0;
constexpr uint64_t BUFFER_DURATION_FIVE = 5;
constexpr int64_t MARK_POSITION = 1000;

inline uint64_t g_firstFrameLatency = 0;
inline bool g_firstFrameCalled = false;
}

class MockAudioRendererFirstFrameWritingCallback : public AudioRendererFirstFrameWritingCallback {
public:
    MOCK_METHOD(void, OnFirstFrameWriting, (uint64_t latency), (override));
};

class AudioRendererFirstFrameMockTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp(void) {}
    void TearDown(void)
    {
        FirstFrameMockUT::g_firstFrameLatency = 0;
        FirstFrameMockUT::g_firstFrameCalled = false;
    }
    
    static void InitializeRendererOptions(AudioRendererOptions &rendererOptions)
    {
        rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
        rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
        rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
        rendererOptions.streamInfo.channels = AudioChannel::STEREO;
        rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
        rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
        rendererOptions.rendererInfo.rendererFlags = FirstFrameMockUT::RENDERER_FLAG;
    }
};

class MockWriteCallback : public AudioRendererWriteCallback {
public:
    MockWriteCallback(std::shared_ptr<AudioRenderer> renderer) : renderer_(renderer) {}
    
    void OnWriteData(size_t length) override
    {
        BufferDesc bufDesc;
        int32_t ret = renderer_->GetBufferDesc(bufDesc);
        if (ret == SUCCESS && bufDesc.buffer != nullptr && length > 0) {
            memset_s(bufDesc.buffer, length, 0, length);
            bufDesc.dataLength = length;
            renderer_->Enqueue(bufDesc);
        }
    }
private:
    std::shared_ptr<AudioRenderer> renderer_;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_RENDERER_FIRSTFRAME_MOCK_TEST_H