/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#ifndef AUDIO_OH_AUDIO_RENDER_UNIT_TEST_H
#define AUDIO_OH_AUDIO_RENDER_UNIT_TEST_H

#include "gtest/gtest.h"
#include "audio_errors.h"
#include "native_audiostreambuilder.h"
#include "native_audiorenderer.h"
#include "OHAudioRenderer.h"
#include "audio_renderer_private.h"

namespace OHOS {
namespace AudioStandard {
namespace OHAudioRenderUT {
constexpr int32_t SAMPLING_RATE = 48000; // 48000:SAMPLING_RATE value
constexpr int32_t CHANNEL_COUNT = 2; // 2:CHANNEL_COUNT value
constexpr int32_t LATENCY_MODE = 0;
constexpr int32_t SAMPLE_FORMAT = 1;
constexpr int32_t FORMAT_SIZE = 2; // s16le 2 bytes.
constexpr int32_t FRAME_SIZE = 240; // 240:FRAME_SIZE value
constexpr float MAX_AUDIO_VOLUME = 1.0f; // volume range is between 0 to 1.
constexpr float MIN_AUDIO_VOLUME = 0.0f; // volume range is between 0 to 1.
constexpr float VALID_LOUDNESS_GAIN = 10.0f;
constexpr float INVALID_LOUDNESS_GAIN = 25.0f;
constexpr int32_t DURATIONMS = 40; // 40:fade out latency ms
constexpr int32_t CHANNEL_2 = 2;

inline uint32_t g_flag = 0;

int32_t AudioRendererOnWriteData(OH_AudioRenderer* capturer,
    void* userData,
    void* buffer,
    int32_t bufferLen);
void AudioRendererOnMarkReachedCb(OH_AudioRenderer* renderer, uint32_t samplePos, void* userData);

int32_t AudioRendererOnWriteDataMock(OH_AudioRenderer* renderer,
    void* userData,
    void* buffer,
    int32_t bufferLen);

OH_AudioData_Callback_Result OnWriteDataCallbackWithValidData(OH_AudioRenderer* renderer,
    void* userData,
    void* buffer,
    int32_t bufferLen);

OH_AudioData_Callback_Result OnWriteDataCallbackWithInvalidData(OH_AudioRenderer* renderer,
    void* userData,
    void* buffer,
    int32_t bufferLen);

int32_t OnWriteDataCbMock(OH_AudioRenderer* renderer,
    void* userData,
    void* buffer,
    int32_t bufferLer);

OH_AudioData_Callback_Result OnWriteDataCbWithValidDataMock(OH_AudioRenderer* renderer,
    void* userData,
    void* buffer,
    int32_t bufferLen);

OH_AudioData_Callback_Result OnWriteDataCbWithInvalidDataMock(OH_AudioRenderer* renderer,
    void* userData,
    void* buffer,
    int32_t bufferLen);

OH_AudioStreamBuilder* InitRenderBuilder();

void CleanupAudioResources(OH_AudioStreamBuilder* builder, OH_AudioRenderer* audioRenderer);
} // namespace OHAudioRenderUT

class OHAudioRenderUnitTest : public testing::Test {
public:
    // SetUpTestCase: Called before all test cases
    static void SetUpTestCase(void) {}
    // TearDownTestCase: Called after all test case
    static void TearDownTestCase(void) {}
    // SetUp: Called before each test cases
    void SetUp(void) {}
    // TearDown: Called after each test cases
    void TearDown(void) {}
    // create a renderer type builder
    static OH_AudioStreamBuilder* CreateRenderBuilder()
    {
        OH_AudioStreamBuilder* builder;
        OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
        OH_AudioStreamBuilder_Create(&builder, type);
        return builder;
    }
};

class OHAudioRendererWriteCallbackMock {
public:
    void OnWriteData(OH_AudioRenderer* renderer, void* userData,
    void* buffer,
    int32_t bufferLen)
    {
        exeCount_++;
        if (executor_) {
            executor_(renderer, userData, buffer, bufferLen);
        }
    }

    void Install(std::function<void(OH_AudioRenderer*, void*, void*, int32_t)> executor)
    {
        executor_ = executor;
    }

    uint32_t GetExeCount()
    {
        return exeCount_;
    }
private:
    std::function<void(OH_AudioRenderer*, void*, void*, int32_t)> executor_;
    std::atomic<uint32_t> exeCount_ = 0;
};

struct UserData {
public:
    enum {
        WRITE_DATA_CALLBACK,
        WRITE_DATA_CALLBACK_WITH_RESULT
    } writeDataCallbackType;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_OH_AUDIO_RENDER_UNIT_TEST_H
