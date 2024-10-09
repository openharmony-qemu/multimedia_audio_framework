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

#ifndef AUDIO_CAPTURER_UNIT_TEST_H
#define AUDIO_CAPTURER_UNIT_TEST_H

#include "gtest/gtest.h"
#include "audio_capturer.h"

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

class AudioCapturerCallbackTest : public AudioCapturerCallback {
public:
    void OnInterrupt(const InterruptEvent &interruptEvent) override {}
    void OnStateChange(const CapturerState state) override {}
};

class AudioCapturerReadCallbackTest : public AudioCapturerReadCallback {
public:
    virtual ~AudioCapturerReadCallbackTest() = default;

    /**
     * Called when buffer to be enqueued.
     *
     * @param length Indicates requested buffer length.
     */
    virtual void OnReadData(size_t length) {};
};

class AudioCapturerReadCallbackMock : public AudioCapturerReadCallback {
public:
    void OnReadData(size_t length) override
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

class AudioCapturerDeviceChangeCallbackTest : public AudioCapturerDeviceChangeCallback {
public:
    virtual ~AudioCapturerDeviceChangeCallbackTest() = default;

    /**
     * Called when capturer device is updated.
     *
     * @param state Indicates updated device of the capturer.
     * since 11
     */
    virtual void OnStateChange(const DeviceInfo &deviceInfo) override {};
};

class AudioCapturerInfoChangeCallbackTest : public AudioCapturerInfoChangeCallback {
public:
    virtual ~AudioCapturerInfoChangeCallbackTest() = default;

    /**
     * Called when capturer info is updated.
     *
     * @param state Indicates info of the capturer.
     * since 11
     */
    virtual void OnStateChange(const AudioCapturerChangeInfo &capturerChangeInfo) override {};
};

class AudioCapturerUnitTest : public testing::Test {
public:
    // SetUpTestCase: Called before all test cases
    static void SetUpTestCase(void);
    // TearDownTestCase: Called after all test case
    static void TearDownTestCase(void);
    // SetUp: Called before each test cases
    void SetUp(void);
    // TearDown: Called after each test cases
    void TearDown(void);
    // Init Capturer
    static int32_t InitializeCapturer(std::unique_ptr<AudioCapturer> &audioCapturer);
    // Init Capturer Options
    static void InitializeCapturerOptions(AudioCapturerOptions &capturerOptions);
    // Init Playback Capture Options
    static void InitializePlaybackCapturerOptions(AudioCapturerOptions &capturerOptions);
};
} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_CAPTURER_UNIT_TEST_H
