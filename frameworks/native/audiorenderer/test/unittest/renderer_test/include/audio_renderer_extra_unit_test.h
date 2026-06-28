/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#ifndef AUDIO_RENDERER_EXTRA_UNIT_TEST_H
#define AUDIO_RENDERER_EXTRA_UNIT_TEST_H

#include <functional>
#include <string>
#include "audio_info.h"
#include "audio_errors.h"
#include "gtest/gtest.h"
#include "audio_renderer.h"
#include "fast_audio_stream.h"
#include "audio_renderer_private.h"

namespace OHOS {
namespace AudioStandard {

class AudioRendererExtraUnitTest : public testing::Test {
public:
    // SetUpTestCase: Called before all test cases
    static void SetUpTestCase(void) {}
    // TearDownTestCase: Called after all test case
    static void TearDownTestCase(void) {}
    // SetUp: Called before each test cases
    void SetUp(void) {};
    // TearDown: Called after each test cases
    void TearDown(void) {};
};

} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_RENDERER_EXTRA_UNIT_TEST_H
