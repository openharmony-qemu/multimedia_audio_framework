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

#ifndef AUDIO_RENDERER_MULTITHREAD_TEST_H
#define AUDIO_RENDERER_MULTITHREAD_TEST_H

#include "gtest/gtest.h"
#include "gtest/hwext/gtest-multithread.h"
#include "unistd.h"
#include "audio_renderer.h"

namespace OHOS {
namespace AudioStandard {
class AudioRendererMultithreadTest : public testing::Test {
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

    static void Write(std::unique_ptr<AudioRenderer> &audioRenderer);
};
} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_RENDERER_MULTITHREAD_TEST_H
