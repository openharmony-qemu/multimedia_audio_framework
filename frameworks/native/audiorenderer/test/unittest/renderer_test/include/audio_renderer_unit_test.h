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
#include <string>
#include "audio_info.h"
#include "audio_errors.h"
#include "gtest/gtest.h"
#include "audio_renderer.h"
#include "fast_audio_stream.h"


namespace OHOS {
namespace AudioStandard {

namespace RenderUT {
const std::string AUDIORENDER_TEST_FILE_PATH = "/data/test_44100_2.wav";
const std::string AUDIORENDER_TEST_PCMFILE_PATH = "/data/avs3_16.wav";
const std::string AUDIORENDER_TEST_METAFILE_PATH = "/data/avs3_bitstream.bin";
constexpr int32_t VALUE_NEGATIVE = -1;
constexpr int32_t VALUE_ZERO = 0;
constexpr int32_t VALUE_HUNDRED = 100;
constexpr int32_t VALUE_THOUSAND = 1000;
constexpr int32_t VALUE_ERROR = -62980098;
constexpr int32_t RENDERER_FLAG = 0;
// Writing only 500 buffers of data for test
constexpr int32_t WRITE_BUFFERS_COUNT = 500;
constexpr int32_t MAX_BUFFER_SIZE = 20000;
constexpr int32_t PAUSE_BUFFER_POSITION = 400000;
constexpr int32_t PAUSE_RENDER_TIME_SECONDS = 1;

constexpr uint64_t BUFFER_DURATION_FIVE = 5;
constexpr uint64_t BUFFER_DURATION_TEN = 10;
constexpr uint64_t BUFFER_DURATION_FIFTEEN = 15;
constexpr uint64_t BUFFER_DURATION_TWENTY = 20;
constexpr uint32_t PLAYBACK_DURATION = 2;
constexpr size_t MAX_RENDERER_INSTANCES = 16;

constexpr size_t AVS3METADATA_SIZE = 19824;
constexpr size_t AUDIOVIVID_FRAME_COUNT = 1024;
constexpr int32_t MAX_CACHE_SIZE = 16384;
constexpr int32_t MIN_CACHE_SIZE = 3528;

inline size_t g_reqBufLen = 0;

constexpr int g_writeOverflowNum = 1000;

void StartRenderThread(AudioRenderer *audioRenderer, uint32_t limit);
}

class AudioRendererDeviceChangeCallbackTest : public AudioRendererDeviceChangeCallback {
public:
    virtual void OnStateChange(const AudioDeviceDescriptor &deviceInfo) override {}
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
    void OnWriteData(size_t length) override { RenderUT::g_reqBufLen = length; }
};

class CapturerPositionCallbackTest : public CapturerPositionCallback {
public:
    void OnMarkReached(const int64_t &framePosition) override {}
};

class CapturerPeriodPositionCallbackTest : public CapturerPeriodPositionCallback {
public:
    void OnPeriodReached(const int64_t &frameNumber) override {}
};

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
    RestoreStatus CheckRestoreStatus() override { return restoreStatus_; };
    RestoreStatus SetRestoreStatus(RestoreStatus status) override
    {
        restoreStatus_ = status;
        return restoreStatus_;
    };
    int32_t SetAudioStreamInfo(const AudioStreamParams info,
        const std::shared_ptr<AudioClientTracker> &proxyObj,
        const AudioPlaybackCaptureConfig &config = AudioPlaybackCaptureConfig()) override { return SUCCESS; }
    void GetRestoreInfo(RestoreInfo &restoreInfo) override {};

    State state_ = State::RUNNING;
    RestoreStatus restoreStatus_ = NO_NEED_FOR_RESTORE;
};

class RendererFastStatusChangeCallbackTest : public AudioRendererFastStatusChangeCallback {
public:
    void OnFastStatusChange(FastStatus status) override { return; }
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
    static void SetUpTestCase(void) {}
    // TearDownTestCase: Called after all test case
    static void TearDownTestCase(void) {}
    // SetUp: Called before each test cases
    void SetUp(void) {};
    // TearDown: Called after each test cases
    void TearDown(void) {};
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

class AudioRendererCallbackTest : public AudioRendererCallback {
public:
    void OnInterrupt(const InterruptEvent &interruptEvent) override {
        AudioRendererUnitTest::interruptEventTest_.hintType = interruptEvent.hintType;
    }
    void OnStateChange(const RendererState state, const StateChangeCmdType cmdType) override {}
};

} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_RENDERER_UNIT_TEST_H
