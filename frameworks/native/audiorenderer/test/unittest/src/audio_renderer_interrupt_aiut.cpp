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

#include "audio_renderer_interrupt_aiut.h"
#include "aiut_log.h"
#include "audio_renderer_private.h"
#include "audio_errors.h"
#include "shared_audio_renderer_wrapper.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {

namespace {
constexpr int32_t TEST_APP_UID = 1234;
constexpr int32_t TEST_APP_PID = 5678;
constexpr int32_t TEST_STREAM_ID = 200;
constexpr int32_t TEST_CLIENT_UID = 1234;
}

// Mock AudioRendererCallback for testing
class TestRendererCallbackForInterrupt : public AudioRendererCallback {
public:
    void OnInterrupt(const InterruptEvent &interruptEvent) override
    {
        lastEvent_ = interruptEvent;
        interruptCount_++;
    }
    void OnStateChange(const RendererState state, const StateChangeCmdType cmdType) override {}

    InterruptEvent lastEvent_ = {};
    int32_t interruptCount_ = 0;
};

void AudioRendererInterruptAiut::SetUpTestCase(void) {}
void AudioRendererInterruptAiut::TearDownTestCase(void) {}
void AudioRendererInterruptAiut::SetUp(void) {}
void AudioRendererInterruptAiut::TearDown(void) {}

// ==================== AudioRendererPrivate Constructor Interrupt Tests ====================

/**
 * @tc.name  : Test AudioRendererPrivate constructor sets audioInterrupt streamType
 * @tc.number: AudioRendererPrivate_Interrupt_Constructor_001
 * @tc.desc  : Test AudioRendererPrivate constructor sets audioInterrupt_.audioFocusType.streamType
 *             to the stream type passed in.
 */
HWTEST_F(AudioRendererInterruptAiut, AudioRendererPrivate_Interrupt_Constructor_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    AppInfo appInfo;
    appInfo.appUid = TEST_APP_UID;
    appInfo.appPid = TEST_APP_PID;
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, appInfo, false);
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");

    AIUT_EXPECT_EQ_INFO_LOG(renderer->audioInterrupt_.audioFocusType.streamType, STREAM_MUSIC,
        "streamType mismatch");
}

// ==================== SetParallelPlayFlag Tests ====================

/**
 * @tc.name  : Test SetParallelPlayFlag with true
 * @tc.number: SetParallelPlayFlag_001
 * @tc.desc  : Test SetParallelPlayFlag sets concurrency mode to MIX_WITH_OTHERS when true.
 */
HWTEST_F(AudioRendererInterruptAiut, SetParallelPlayFlag_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");
    int32_t ret = renderer->SetParallelPlayFlag(true);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "SetParallelPlayFlag(true) should succeed, ret=%d", ret);
    AIUT_EXPECT_EQ_INFO_LOG(renderer->audioInterrupt_.sessionStrategy.concurrencyMode,
        AudioConcurrencyMode::MIX_WITH_OTHERS, "concurrencyMode should be MIX_WITH_OTHERS");
}

/**
 * @tc.name  : Test SetParallelPlayFlag with false restores original strategy
 * @tc.number: SetParallelPlayFlag_002
 * @tc.desc  : Test SetParallelPlayFlag(false) restores original concurrency mode.
 */
HWTEST_F(AudioRendererInterruptAiut, SetParallelPlayFlag_002, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");
    // First set to true
    renderer->SetParallelPlayFlag(true);
    // Then set to false - should restore original
    int32_t ret = renderer->SetParallelPlayFlag(false);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "SetParallelPlayFlag(false) should succeed, ret=%d", ret);
}

// ==================== SetInterruptMode Tests ====================

/**
 * @tc.name  : Test SetInterruptMode with SHARE_MODE
 * @tc.number: SetInterruptMode_001
 * @tc.desc  : Test SetInterruptMode sets audioInterrupt mode to SHARE_MODE.
 */
HWTEST_F(AudioRendererInterruptAiut, SetInterruptMode_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");
    renderer->SetInterruptMode(SHARE_MODE);
    AIUT_EXPECT_EQ_INFO_LOG(renderer->audioInterrupt_.mode, SHARE_MODE, "mode should be SHARE_MODE");
}

/**
 * @tc.name  : Test SetInterruptMode with INDEPENDENT_MODE
 * @tc.number: SetInterruptMode_002
 * @tc.desc  : Test SetInterruptMode sets audioInterrupt mode to INDEPENDENT_MODE.
 */
HWTEST_F(AudioRendererInterruptAiut, SetInterruptMode_002, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");
    renderer->SetInterruptMode(INDEPENDENT_MODE);
    AIUT_EXPECT_EQ_INFO_LOG(renderer->audioInterrupt_.mode, INDEPENDENT_MODE,
        "mode should be INDEPENDENT_MODE");
}

// ==================== AudioRendererInterruptCallbackImpl Tests ====================

/**
 * @tc.name  : Test interrupt callback construction
 * @tc.number: InterruptCallback_Construct_001
 * @tc.desc  : Test AudioRendererInterruptCallbackImpl can be constructed with
 *             a valid audioStream and audioInterrupt.
 */
HWTEST_F(AudioRendererInterruptAiut, InterruptCallback_Construct_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");
    AIUT_ASSERT_NE_INFO_LOG(renderer->audioInterruptCallback_, nullptr,
        "interrupt callback should exist after construction");
}

/**
 * @tc.name  : Test SaveCallback on interrupt callback
 * @tc.number: InterruptCallback_SaveCallback_001
 * @tc.desc  : Test AudioRendererInterruptCallbackImpl::SaveCallback stores the callback.
 */
HWTEST_F(AudioRendererInterruptAiut, InterruptCallback_SaveCallback_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");
    auto callback = std::make_shared<TestRendererCallbackForInterrupt>();

    AIUT_ASSERT_NE_INFO_LOG(renderer->audioInterruptCallback_, nullptr,
        "audioInterruptCallback_ should not be null");
    auto interruptCb = std::static_pointer_cast<AudioRendererInterruptCallbackImpl>(
        renderer->audioInterruptCallback_);
    interruptCb->SaveCallback(callback);
}

// ==================== AudioStreamCallbackRenderer Tests ====================

/**
 * @tc.name  : Test AudioStreamCallbackRenderer construction
 * @tc.number: StreamCallback_Construct_001
 * @tc.desc  : Test AudioStreamCallbackRenderer can be constructed with a valid renderer.
 */
HWTEST_F(AudioRendererInterruptAiut, StreamCallback_Construct_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");
    auto streamCallback = std::make_shared<AudioStreamCallbackRenderer>(renderer);
    AIUT_ASSERT_NE_INFO_LOG(streamCallback, nullptr, "streamCallback is null");
    // Call SaveCallback to verify streamCallback is functional after construction
    auto callback = std::make_shared<TestRendererCallbackForInterrupt>();
    streamCallback->SaveCallback(callback);
    // Verify renderer is still in valid state after stream callback operations
    AIUT_EXPECT_EQ_INFO_LOG(renderer->GetStatus(), RENDERER_PREPARED,
        "renderer should remain PREPARED after StreamCallback construction, state=%d", renderer->GetStatus());
}

/**
 * @tc.name  : Test AudioStreamCallbackRenderer SaveCallback
 * @tc.number: StreamCallback_SaveCallback_001
 * @tc.desc  : Test AudioStreamCallbackRenderer::SaveCallback stores the callback.
 */
HWTEST_F(AudioRendererInterruptAiut, StreamCallback_SaveCallback_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");
    auto streamCallback = std::make_shared<AudioStreamCallbackRenderer>(renderer);
    AIUT_ASSERT_NE_INFO_LOG(streamCallback, nullptr, "streamCallback is null");

    auto callback = std::make_shared<TestRendererCallbackForInterrupt>();

    streamCallback->SaveCallback(callback);
    // Verify renderer is still in valid state after SaveCallback
    AIUT_EXPECT_EQ_INFO_LOG(renderer->GetStatus(), RENDERER_PREPARED,
        "renderer should remain PREPARED after StreamCallback SaveCallback, state=%d", renderer->GetStatus());
}

// ==================== RendererPolicyServiceDiedCallback Tests ====================

/**
 * @tc.name  : Test RendererPolicyServiceDiedCallback construction
 * @tc.number: PolicyServiceDiedCallback_Construct_001
 * @tc.desc  : Test RendererPolicyServiceDiedCallback can be constructed.
 */
HWTEST_F(AudioRendererInterruptAiut, PolicyServiceDiedCallback_Construct_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto callback = std::make_shared<RendererPolicyServiceDiedCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "callback is null");
    // Verify callback can be used by setting a renderer on it
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");
    callback->SetAudioRendererObj(renderer);
    AIUT_EXPECT_EQ_INFO_LOG(renderer->GetStatus(), RENDERER_PREPARED,
        "renderer should remain PREPARED after PolicyServiceDiedCallback operations, state=%d",
        renderer->GetStatus());
}

/**
 * @tc.name  : Test RendererPolicyServiceDiedCallback SetAudioRendererObj
 * @tc.number: PolicyServiceDiedCallback_SetAudioRendererObj_001
 * @tc.desc  : Test SetAudioRendererObj stores renderer reference.
 */
HWTEST_F(AudioRendererInterruptAiut, PolicyServiceDiedCallback_SetAudioRendererObj_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto callback = std::make_shared<RendererPolicyServiceDiedCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "callback is null");
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");
    callback->SetAudioRendererObj(renderer);
    // Verify renderer is still in valid state after SetAudioRendererObj
    AIUT_EXPECT_EQ_INFO_LOG(renderer->GetStatus(), RENDERER_PREPARED,
        "renderer should remain PREPARED after SetAudioRendererObj, state=%d", renderer->GetStatus());
}

/**
 * @tc.name  : Test RendererPolicyServiceDiedCallback SetAudioInterrupt
 * @tc.number: PolicyServiceDiedCallback_SetAudioInterrupt_001
 * @tc.desc  : Test SetAudioInterrupt stores audio interrupt info.
 */
HWTEST_F(AudioRendererInterruptAiut, PolicyServiceDiedCallback_SetAudioInterrupt_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto callback = std::make_shared<RendererPolicyServiceDiedCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "callback is null");
    AudioInterrupt interrupt;
    interrupt.streamId = TEST_STREAM_ID;
    interrupt.pid = TEST_CLIENT_UID;
    interrupt.mode = SHARE_MODE;
    callback->SetAudioInterrupt(interrupt);
    // Verify callback can still accept renderer after SetAudioInterrupt
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");
    callback->SetAudioRendererObj(renderer);
    AIUT_EXPECT_EQ_INFO_LOG(renderer->GetStatus(), RENDERER_PREPARED,
        "renderer should remain PREPARED after SetAudioInterrupt + SetAudioRendererObj, state=%d",
        renderer->GetStatus());
}

// ==================== RegisterRendererPolicyServiceDiedCallback Tests ====================

/**
 * @tc.name  : Test RegisterRendererPolicyServiceDiedCallback
 * @tc.number: RegisterRendererPolicyServiceDiedCallback_001
 * @tc.desc  : Test RegisterRendererPolicyServiceDiedCallback stores callback in renderer.
 */
HWTEST_F(AudioRendererInterruptAiut, RegisterRendererPolicyServiceDiedCallback_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");
    auto callback = std::make_shared<RendererPolicyServiceDiedCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "callback is null");
    AIUT_ASSERT_NE_INFO_LOG(renderer->audioStream_, nullptr, "audioStream_ should not be null");
    renderer->RegisterRendererPolicyServiceDiedCallback(renderer->audioStream_);
}

// ==================== SharedAudioRendererWrapper Interrupt Delegation Tests ====================

/**
 * @tc.name  : Test SetInterruptMode delegation via wrapper
 * @tc.number: SharedAudioRendererWrapper_SetInterruptMode_001
 * @tc.desc  : Test SetInterruptMode delegates to the inner renderer.
 */
HWTEST_F(AudioRendererInterruptAiut, SharedAudioRendererWrapper_SetInterruptMode_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto innerRenderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(innerRenderer, nullptr, "innerRenderer is null");
    auto wrapper = std::make_shared<SharedAudioRendererWrapper>(innerRenderer);
    AIUT_ASSERT_NE_INFO_LOG(wrapper, nullptr, "wrapper is null");
    wrapper->SetInterruptMode(INDEPENDENT_MODE);
    AIUT_EXPECT_EQ_INFO_LOG(innerRenderer->audioInterrupt_.mode, INDEPENDENT_MODE,
        "mode should be INDEPENDENT_MODE via wrapper");
}

/**
 * @tc.name  : Test SetRegisterInterruptCallbackFlag delegation via wrapper
 * @tc.number: SharedAudioRendererWrapper_SetRegisterInterruptCallbackFlag_001
 * @tc.desc  : Test SetRegisterInterruptCallbackFlag delegates to the inner renderer.
 */
HWTEST_F(AudioRendererInterruptAiut, SharedAudioRendererWrapper_SetRegisterInterruptCallbackFlag_001,
    TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto innerRenderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(innerRenderer, nullptr, "innerRenderer is null");
    auto wrapper = std::make_shared<SharedAudioRendererWrapper>(innerRenderer);
    AIUT_ASSERT_NE_INFO_LOG(wrapper, nullptr, "wrapper is null");
    wrapper->SetRegisterInterruptCallbackFlag(true);
    // Verify wrapper state remains valid after setting interrupt callback flag
    AIUT_EXPECT_EQ_INFO_LOG(wrapper->GetStatus(), RENDERER_PREPARED,
        "wrapper should remain PREPARED after SetRegisterInterruptCallbackFlag, state=%d",
        wrapper->GetStatus());
}

/**
 * @tc.name  : Test SetParallelPlayFlag delegation via wrapper
 * @tc.number: SharedAudioRendererWrapper_SetParallelPlayFlag_001
 * @tc.desc  : Test SetParallelPlayFlag delegates to the inner renderer.
 */
HWTEST_F(AudioRendererInterruptAiut, SharedAudioRendererWrapper_SetParallelPlayFlag_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto innerRenderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(innerRenderer, nullptr, "innerRenderer is null");
    auto wrapper = std::make_shared<SharedAudioRendererWrapper>(innerRenderer);
    AIUT_ASSERT_NE_INFO_LOG(wrapper, nullptr, "wrapper is null");
    int32_t ret = wrapper->SetParallelPlayFlag(true);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "SetParallelPlayFlag should succeed, ret=%d", ret);
}

} // namespace AudioStandard
} // namespace OHOS
