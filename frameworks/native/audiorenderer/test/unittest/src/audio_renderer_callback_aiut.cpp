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

#include "audio_renderer_callback_aiut.h"
#include "aiut_log.h"
#include "audio_renderer_private.h"
#include "audio_errors.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {

namespace {
constexpr uint32_t TEST_SESSION_ID = 100;
constexpr int32_t TEST_CLIENT_UID = 1234;
constexpr int32_t TEST_STREAM_ID = 200;
}

void AudioRendererCallbackAiut::SetUpTestCase(void) {}
void AudioRendererCallbackAiut::TearDownTestCase(void) {}
void AudioRendererCallbackAiut::SetUp(void) {}
void AudioRendererCallbackAiut::TearDown(void) {}

// ==================== FormatUnsupportedErrorCallbackImpl Tests ====================

/**
 * @tc.name  : Test FormatUnsupportedErrorCallbackImpl construction
 * @tc.number: FormatUnsupportedCallback_Construct_001
 * @tc.desc  : Test FormatUnsupportedErrorCallbackImpl can be constructed.
 */
HWTEST_F(AudioRendererCallbackAiut, FormatUnsupportedCallback_Construct_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto callback = std::make_shared<FormatUnsupportedErrorCallbackImpl>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "FormatUnsupportedErrorCallbackImpl should be created");
}

// ==================== AudioRouteCallbackImpl Tests ====================

/**
 * @tc.name  : Test AudioRouteCallbackImpl construction
 * @tc.number: RouteCallback_Construct_001
 * @tc.desc  : Test AudioRouteCallbackImpl can be constructed with a valid renderer.
 */
HWTEST_F(AudioRendererCallbackAiut, RouteCallback_Construct_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    auto routeCallback = std::make_shared<AudioRouteCallbackImpl>(renderer);
    AIUT_ASSERT_NE_INFO_LOG(routeCallback, nullptr, "AudioRouteCallbackImpl should be created");
}

// ==================== AudioRendererPrivate DebugInfo Tests ====================

/**
 * @tc.name  : Test GetRendererDebugInfo with no stream
 * @tc.number: GetRendererDebugInfo_001
 * @tc.desc  : Test GetRendererDebugInfo returns error when audioStream_ is null.
 */
HWTEST_F(AudioRendererCallbackAiut, GetRendererDebugInfo_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AudioRendererDebugInfo debugInfo;
    int32_t ret = renderer->GetRendererDebugInfo(debugInfo);
    AIUT_EXPECT_NE_INFO_LOG(ret, SUCCESS, "GetRendererDebugInfo should fail without stream, ret=%d", ret);
}

} // namespace AudioStandard
} // namespace OHOS
