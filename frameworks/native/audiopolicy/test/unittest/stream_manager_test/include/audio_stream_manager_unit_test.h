/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef AUDIO_STREAM_MANAGER_UNIT_TEST_H
#define AUDIO_STREAM_MANAGER_UNIT_TEST_H

#include "gtest/gtest.h"
#include "audio_capturer.h"
#include "audio_renderer.h"
#include "audio_stream_manager.h"

namespace OHOS {
namespace AudioStandard {
class AudioRendererStateChangeCallbackTest : public AudioRendererStateChangeCallback {
public:
    explicit AudioRendererStateChangeCallbackTest(const std::string &testCaseName);
    ~AudioRendererStateChangeCallbackTest() = default;

    void OnRendererStateChange(
        const std::vector<std::unique_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos) override;
private:
    std::string testCaseName_;
};

class AudioCapturerStateChangeCallbackTest : public AudioCapturerStateChangeCallback {
public:
    explicit AudioCapturerStateChangeCallbackTest(const std::string &testCaseName);
    ~AudioCapturerStateChangeCallbackTest() = default;

    void OnCapturerStateChange(
        const std::vector<std::unique_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos) override;
private:
    std::string testCaseName_;
};

class AudioStreamManagerUnitTest : public testing::Test {
public:
    // SetUpTestCase: Called before all test cases
    static void SetUpTestCase(void);
    // TearDownTestCase: Called after all test case
    static void TearDownTestCase(void);
    // SetUp: Called before each test cases
    void SetUp(void);
    // TearDown: Called after each test cases
    void TearDown(void);
    // Init Renderer Options
    static void InitializeRendererOptions(AudioRendererOptions &rendererOptions);
    // Init Capturer Options
    static void InitializeCapturerOptions(AudioCapturerOptions &capturerOptions);
    // Wait for Callback invoke
    static void WaitForCallback();
};
} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_STREAM_MANAGER_UNIT_TEST_H
