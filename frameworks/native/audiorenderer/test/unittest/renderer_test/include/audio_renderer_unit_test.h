/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef AUDIO_RENDERER_UNIT_TEST_H
#define AUDIO_RENDERER_UNIT_TEST_H

#include <functional>
#include "gtest/gtest.h"
#include "audio_renderer.h"

namespace OHOS {
namespace AudioStandard {
class AudioRendererCallbackTest : public AudioRendererCallback {
public:
    void OnInterrupt(const InterruptEvent &interruptEvent) override;
    void OnStateChange(const RendererState state, const StateChangeCmdType cmdType) override {}
};

class AudioRendererDeviceChangeCallbackTest : public AudioRendererDeviceChangeCallback {
public:
    virtual void OnStateChange(const DeviceInfo &deviceInfo) override {}
    virtual void RemoveAllCallbacks() override {}
};

class AudioRendererPolicyServiceDiedCallbackTest : public AudioRendererPolicyServiceDiedCallback {
public:
    virtual void OnAudioPolicyServiceDied() override {}
};

class RendererPositionCallbackTest : public RendererPositionCallback {
public:
    void OnMarkReached(const int64_t &framePosition) override {}
};

class RendererPeriodPositionCallbackTest : public RendererPeriodPositionCallback {
public:
    void OnPeriodReached(const int64_t &frameNumber) override {}
};

class AudioRenderModeCallbackTest : public AudioRendererWriteCallback {
public:
    void OnWriteData(size_t length) override;
};

class AudioRendererWriteCallbackMock : public AudioRendererWriteCallback {
public:
    void OnWriteData(size_t length)
    {
        exeCount_++;
        if (executor_) {
            executor_(length);
        }
    }

    void Install(std::function<void(size_t)> executor)
    {
        executor_ = executor;
    }

    uint32_t GetExeCount()
    {
        return exeCount_;
    }
private:
    std::function<void(size_t)> executor_;
    std::atomic<uint32_t> exeCount_ = 0;
};

class AudioRendererUnitTest : public testing::Test {
public:
    // SetUpTestCase: Called before all test cases
    static void SetUpTestCase(void);
    // TearDownTestCase: Called after all test case
    static void TearDownTestCase(void);
    // SetUp: Called before each test cases
    void SetUp(void);
    // TearDown: Called after each test cases
    void TearDown(void);
    // Init Renderer
    static int32_t InitializeRenderer(std::unique_ptr<AudioRenderer> &audioRenderer);
    // Init Renderer Options
    static void InitializeRendererOptions(AudioRendererOptions &rendererOptions);
    // Init 3DRenderer Options
    static void InitializeRendererSpatialOptions(AudioRendererOptions &rendererOptions);
    // Allocate memory
    static void GetBuffersAndLen(std::unique_ptr<AudioRenderer> &audioRenderer,
        uint8_t *&buffer, uint8_t *&metaBuffer, size_t &bufferLen);
    // Release memory
    static void ReleaseBufferAndFiles(uint8_t *&buffer, uint8_t *&metaBuffer,
        FILE *&wavFile, FILE *&metaFile);
    static InterruptEvent interruptEventTest_;
};
} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_RENDERER_UNIT_TEST_H
