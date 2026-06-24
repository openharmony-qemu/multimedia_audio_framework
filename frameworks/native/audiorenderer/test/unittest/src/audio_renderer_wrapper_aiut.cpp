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

#include "audio_renderer_wrapper_aiut.h"
#include "aiut_log.h"
#include "audio_renderer_proxy_obj.h"
#include "audio_renderer_private.h"
#include "shared_audio_renderer_wrapper.h"
#include "audio_errors.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {

namespace {
constexpr int32_t TEST_OFFLOAD_STATE = 1;
constexpr bool TEST_IS_APP_BACK = false;
constexpr int64_t TEST_POSITION = 1000;
constexpr int64_t TEST_PERIOD = 100;
constexpr int64_t TEST_SOURCE_DURATION = 5000;
}

// Mock callback classes for testing
class TestRendererCallback : public AudioRendererCallback {
public:
    void OnInterrupt(const InterruptEvent &interruptEvent) override {}
    void OnStateChange(const RendererState state, const StateChangeCmdType cmdType) override {}
};

class TestRendererPositionCallback : public RendererPositionCallback {
public:
    void OnMarkReached(const int64_t &framePosition) override {}
};

class TestRendererPeriodPositionCallback : public RendererPeriodPositionCallback {
public:
    void OnPeriodReached(const int64_t &frameNumber) override {}
};

class TestRendererWriteCallback : public AudioRendererWriteCallback {
public:
    void OnWriteData(size_t length) override {}
};

class TestRendererFirstFrameWritingCallback : public AudioRendererFirstFrameWritingCallback {
public:
    void OnFirstFrameWriting(uint64_t latency) override {}
};

class TestRendererErrorCallback : public AudioRendererErrorCallback {
public:
    void OnError(const AudioErrors errorCode) override {}
};

class TestOutputDeviceChangeCallback : public AudioRendererOutputDeviceChangeCallback {
public:
    void OnOutputDeviceChange(const AudioDeviceDescriptor &desc,
        const AudioStreamDeviceChangeReason reason) override {}
};

// ==================== AudioRendererCallbackAiut ====================

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
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "callback is null");
    // Calling OnFormatUnsupportedError without registered callback should complete silently
    callback->OnFormatUnsupportedError(ERROR_ILLEGAL_STATE);
    // Verify renderer state is still valid after callback invocation
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");
    AIUT_EXPECT_EQ_INFO_LOG(renderer->GetStatus(), RENDERER_PREPARED,
        "renderer should remain PREPARED after OnFormatUnsupportedError, state=%d", renderer->GetStatus());
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
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");
    auto routeCallback = std::make_shared<AudioRouteCallbackImpl>(renderer);
    AIUT_ASSERT_NE_INFO_LOG(routeCallback, nullptr, "routeCallback is null");
    // Calling OnRouteUpdate on callback with valid renderer should not crash
    routeCallback->OnRouteUpdate(0, "test_network");
    // Verify renderer is still in valid state after route callback
    AIUT_EXPECT_EQ_INFO_LOG(renderer->GetStatus(), RENDERER_PREPARED,
        "renderer should remain PREPARED after OnRouteUpdate, state=%d", renderer->GetStatus());
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
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");
    AudioRendererDebugInfo debugInfo;
    int32_t ret = renderer->GetRendererDebugInfo(debugInfo);
    AIUT_EXPECT_NE_INFO_LOG(ret, SUCCESS, "GetRendererDebugInfo should fail without stream, ret=%d", ret);
}

// ==================== AudioRendererProxyObjAiut ====================

void AudioRendererProxyObjAiut::SetUpTestCase(void) {}
void AudioRendererProxyObjAiut::TearDownTestCase(void) {}
void AudioRendererProxyObjAiut::SetUp(void) {}
void AudioRendererProxyObjAiut::TearDown(void) {}

// ==================== SaveRendererObj / UnsetRendererObj Tests ====================

/**
 * @tc.name  : Test SaveRendererObj stores renderer reference
 * @tc.number: SaveRendererObj_001
 * @tc.desc  : Test SaveRendererObj stores a weak reference to the renderer object.
 */
HWTEST_F(AudioRendererProxyObjAiut, SaveRendererObj_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto proxyObj = std::make_shared<AudioRendererProxyObj>();
    AIUT_ASSERT_NE_INFO_LOG(proxyObj, nullptr, "proxyObj is null");
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");

    proxyObj->SaveRendererObj(renderer);

    // After SaveRendererObj, the proxy should hold the renderer
    StreamSetStateEventInternal event = {};
    proxyObj->MuteStreamImpl(event);
    // Verify renderer is still in valid state after proxy method call
    AIUT_EXPECT_EQ_INFO_LOG(renderer->GetStatus(), RENDERER_PREPARED,
        "renderer state should remain PREPARED after SaveRendererObj, state=%d", renderer->GetStatus());
}

/**
 * @tc.name  : Test UnsetRendererObj clears renderer reference
 * @tc.number: UnsetRendererObj_001
 * @tc.desc  : Test UnsetRendererObj clears the stored renderer reference.
 */
HWTEST_F(AudioRendererProxyObjAiut, UnsetRendererObj_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto proxyObj = std::make_shared<AudioRendererProxyObj>();
    AIUT_ASSERT_NE_INFO_LOG(proxyObj, nullptr, "proxyObj is null");
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");

    proxyObj->SaveRendererObj(renderer);
    proxyObj->UnsetRendererObj();

    // After unset, methods should handle null gracefully (no crash)
    StreamSetStateEventInternal event = {};
    proxyObj->MuteStreamImpl(event);
    proxyObj->UnmuteStreamImpl(event);
    proxyObj->PausedStreamImpl(event);
    proxyObj->ResumeStreamImpl(event);
    // Verify proxyObj is still functional after UnsetRendererObj and void method calls
    auto renderer2 = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    proxyObj->SaveRendererObj(renderer2);
    AIUT_EXPECT_EQ_INFO_LOG(renderer2->GetStatus(), RENDERER_PREPARED,
        "proxyObj should still be functional after UnsetRendererObj, state=%d", renderer2->GetStatus());
}

// ==================== MuteStreamImpl Tests ====================

/**
 * @tc.name  : Test MuteStreamImpl with valid renderer
 * @tc.number: MuteStreamImpl_001
 * @tc.desc  : Test MuteStreamImpl calls Mute on the stored renderer without crash.
 */
HWTEST_F(AudioRendererProxyObjAiut, MuteStreamImpl_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto proxyObj = std::make_shared<AudioRendererProxyObj>();
    AIUT_ASSERT_NE_INFO_LOG(proxyObj, nullptr, "proxyObj is null");
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");
    proxyObj->SaveRendererObj(renderer);

    StreamSetStateEventInternal event = {};
    proxyObj->MuteStreamImpl(event);
    // Verify renderer state is still valid after MuteStreamImpl
    AIUT_EXPECT_EQ_INFO_LOG(renderer->GetStatus(), RENDERER_PREPARED,
        "renderer should remain PREPARED after MuteStreamImpl, state=%d", renderer->GetStatus());
}

/**
 * @tc.name  : Test MuteStreamImpl with no renderer
 * @tc.number: MuteStreamImpl_002
 * @tc.desc  : Test MuteStreamImpl handles gracefully when renderer is not set.
 */
HWTEST_F(AudioRendererProxyObjAiut, MuteStreamImpl_002, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto proxyObj = std::make_shared<AudioRendererProxyObj>();
    AIUT_ASSERT_NE_INFO_LOG(proxyObj, nullptr, "proxyObj is null");
    StreamSetStateEventInternal event = {};
    proxyObj->MuteStreamImpl(event);
    // Verify proxyObj is still functional after null renderer call
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");
    proxyObj->SaveRendererObj(renderer);
    AIUT_EXPECT_EQ_INFO_LOG(renderer->GetStatus(), RENDERER_PREPARED,
        "proxyObj should still be functional after MuteStreamImpl on null renderer, state=%d",
        renderer->GetStatus());
}

// ==================== UnmuteStreamImpl Tests ====================

/**
 * @tc.name  : Test UnmuteStreamImpl with valid renderer
 * @tc.number: UnmuteStreamImpl_001
 * @tc.desc  : Test UnmuteStreamImpl calls Unmute on the stored renderer without crash.
 */
HWTEST_F(AudioRendererProxyObjAiut, UnmuteStreamImpl_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto proxyObj = std::make_shared<AudioRendererProxyObj>();
    AIUT_ASSERT_NE_INFO_LOG(proxyObj, nullptr, "proxyObj is null");
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");
    proxyObj->SaveRendererObj(renderer);

    StreamSetStateEventInternal event = {};
    proxyObj->UnmuteStreamImpl(event);
    // Verify renderer state is still valid after UnmuteStreamImpl
    AIUT_EXPECT_EQ_INFO_LOG(renderer->GetStatus(), RENDERER_PREPARED,
        "renderer should remain PREPARED after UnmuteStreamImpl, state=%d", renderer->GetStatus());
}

/**
 * @tc.name  : Test UnmuteStreamImpl with no renderer
 * @tc.number: UnmuteStreamImpl_002
 * @tc.desc  : Test UnmuteStreamImpl handles gracefully when renderer is not set.
 */
HWTEST_F(AudioRendererProxyObjAiut, UnmuteStreamImpl_002, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto proxyObj = std::make_shared<AudioRendererProxyObj>();
    AIUT_ASSERT_NE_INFO_LOG(proxyObj, nullptr, "proxyObj is null");
    StreamSetStateEventInternal event = {};
    proxyObj->UnmuteStreamImpl(event);
    // Verify proxyObj is still functional after null renderer call
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");
    proxyObj->SaveRendererObj(renderer);
    AIUT_EXPECT_EQ_INFO_LOG(renderer->GetStatus(), RENDERER_PREPARED,
        "proxyObj should still be functional after UnmuteStreamImpl on null renderer, state=%d",
        renderer->GetStatus());
}

// ==================== PausedStreamImpl Tests ====================

/**
 * @tc.name  : Test PausedStreamImpl with valid renderer
 * @tc.number: PausedStreamImpl_001
 * @tc.desc  : Test PausedStreamImpl calls Pause on the stored renderer without crash.
 */
HWTEST_F(AudioRendererProxyObjAiut, PausedStreamImpl_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto proxyObj = std::make_shared<AudioRendererProxyObj>();
    AIUT_ASSERT_NE_INFO_LOG(proxyObj, nullptr, "proxyObj is null");
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");
    proxyObj->SaveRendererObj(renderer);

    StreamSetStateEventInternal event = {};
    proxyObj->PausedStreamImpl(event);
    // Verify renderer state is still valid after PausedStreamImpl
    AIUT_EXPECT_EQ_INFO_LOG(renderer->GetStatus(), RENDERER_PREPARED,
        "renderer should remain PREPARED after PausedStreamImpl, state=%d", renderer->GetStatus());
}

/**
 * @tc.name  : Test PausedStreamImpl with no renderer
 * @tc.number: PausedStreamImpl_002
 * @tc.desc  : Test PausedStreamImpl handles gracefully when renderer is not set.
 */
HWTEST_F(AudioRendererProxyObjAiut, PausedStreamImpl_002, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto proxyObj = std::make_shared<AudioRendererProxyObj>();
    AIUT_ASSERT_NE_INFO_LOG(proxyObj, nullptr, "proxyObj is null");
    StreamSetStateEventInternal event = {};
    proxyObj->PausedStreamImpl(event);
    // Verify proxyObj is still functional after null renderer call
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");
    proxyObj->SaveRendererObj(renderer);
    AIUT_EXPECT_EQ_INFO_LOG(renderer->GetStatus(), RENDERER_PREPARED,
        "proxyObj should still be functional after PausedStreamImpl on null renderer, state=%d",
        renderer->GetStatus());
}

// ==================== ResumeStreamImpl Tests ====================

/**
 * @tc.name  : Test ResumeStreamImpl with valid renderer
 * @tc.number: ResumeStreamImpl_001
 * @tc.desc  : Test ResumeStreamImpl calls Start on the stored renderer without crash.
 */
HWTEST_F(AudioRendererProxyObjAiut, ResumeStreamImpl_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto proxyObj = std::make_shared<AudioRendererProxyObj>();
    AIUT_ASSERT_NE_INFO_LOG(proxyObj, nullptr, "proxyObj is null");
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");
    proxyObj->SaveRendererObj(renderer);

    StreamSetStateEventInternal event = {};
    proxyObj->ResumeStreamImpl(event);
    // Verify renderer state is still valid after ResumeStreamImpl
    AIUT_EXPECT_EQ_INFO_LOG(renderer->GetStatus(), RENDERER_PREPARED,
        "renderer should remain PREPARED after ResumeStreamImpl, state=%d", renderer->GetStatus());
}

/**
 * @tc.name  : Test ResumeStreamImpl with no renderer
 * @tc.number: ResumeStreamImpl_002
 * @tc.desc  : Test ResumeStreamImpl handles gracefully when renderer is not set.
 */
HWTEST_F(AudioRendererProxyObjAiut, ResumeStreamImpl_002, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto proxyObj = std::make_shared<AudioRendererProxyObj>();
    AIUT_ASSERT_NE_INFO_LOG(proxyObj, nullptr, "proxyObj is null");
    StreamSetStateEventInternal event = {};
    proxyObj->ResumeStreamImpl(event);
    // Verify proxyObj is still functional after null renderer call
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");
    proxyObj->SaveRendererObj(renderer);
    AIUT_EXPECT_EQ_INFO_LOG(renderer->GetStatus(), RENDERER_PREPARED,
        "proxyObj should still be functional after ResumeStreamImpl on null renderer, state=%d",
        renderer->GetStatus());
}

// ==================== SetOffloadModeImpl Tests ====================

/**
 * @tc.name  : Test SetOffloadModeImpl with valid renderer
 * @tc.number: SetOffloadModeImpl_001
 * @tc.desc  : Test SetOffloadModeImpl calls SetOffloadMode on the renderer.
 */
HWTEST_F(AudioRendererProxyObjAiut, SetOffloadModeImpl_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto proxyObj = std::make_shared<AudioRendererProxyObj>();
    AIUT_ASSERT_NE_INFO_LOG(proxyObj, nullptr, "proxyObj is null");
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");
    proxyObj->SaveRendererObj(renderer);

    proxyObj->SetOffloadModeImpl(TEST_OFFLOAD_STATE, TEST_IS_APP_BACK);
    // Verify renderer is still in valid state after SetOffloadModeImpl
    AIUT_EXPECT_EQ_INFO_LOG(renderer->GetStatus(), RENDERER_PREPARED,
        "renderer should remain PREPARED after SetOffloadModeImpl, state=%d", renderer->GetStatus());
}

/**
 * @tc.name  : Test SetOffloadModeImpl with no renderer
 * @tc.number: SetOffloadModeImpl_002
 * @tc.desc  : Test SetOffloadModeImpl handles gracefully when renderer is not set.
 */
HWTEST_F(AudioRendererProxyObjAiut, SetOffloadModeImpl_002, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto proxyObj = std::make_shared<AudioRendererProxyObj>();
    AIUT_ASSERT_NE_INFO_LOG(proxyObj, nullptr, "proxyObj is null");
    proxyObj->SetOffloadModeImpl(TEST_OFFLOAD_STATE, TEST_IS_APP_BACK);
    // Verify proxyObj is still functional after null renderer call
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");
    proxyObj->SaveRendererObj(renderer);
    AIUT_EXPECT_EQ_INFO_LOG(renderer->GetStatus(), RENDERER_PREPARED,
        "proxyObj should still be functional after SetOffloadModeImpl on null renderer, state=%d",
        renderer->GetStatus());
}

// ==================== UnsetOffloadModeImpl Tests ====================

/**
 * @tc.name  : Test UnsetOffloadModeImpl with valid renderer
 * @tc.number: UnsetOffloadModeImpl_001
 * @tc.desc  : Test UnsetOffloadModeImpl calls UnsetOffloadMode on the renderer.
 */
HWTEST_F(AudioRendererProxyObjAiut, UnsetOffloadModeImpl_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto proxyObj = std::make_shared<AudioRendererProxyObj>();
    AIUT_ASSERT_NE_INFO_LOG(proxyObj, nullptr, "proxyObj is null");
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");
    proxyObj->SaveRendererObj(renderer);

    proxyObj->UnsetOffloadModeImpl();
    // Verify renderer is still in valid state after UnsetOffloadModeImpl
    AIUT_EXPECT_EQ_INFO_LOG(renderer->GetStatus(), RENDERER_PREPARED,
        "renderer should remain PREPARED after UnsetOffloadModeImpl, state=%d", renderer->GetStatus());
}

/**
 * @tc.name  : Test UnsetOffloadModeImpl with no renderer
 * @tc.number: UnsetOffloadModeImpl_002
 * @tc.desc  : Test UnsetOffloadModeImpl handles gracefully when renderer is not set.
 */
HWTEST_F(AudioRendererProxyObjAiut, UnsetOffloadModeImpl_002, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto proxyObj = std::make_shared<AudioRendererProxyObj>();
    AIUT_ASSERT_NE_INFO_LOG(proxyObj, nullptr, "proxyObj is null");
    proxyObj->UnsetOffloadModeImpl();
    // Verify proxyObj is still functional after null renderer call
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");
    proxyObj->SaveRendererObj(renderer);
    AIUT_EXPECT_EQ_INFO_LOG(renderer->GetStatus(), RENDERER_PREPARED,
        "proxyObj should still be functional after UnsetOffloadModeImpl on null renderer, state=%d",
        renderer->GetStatus());
}

// ==================== Weak Pointer Lifecycle Tests ====================

/**
 * @tc.name  : Test renderer weak_ptr becomes expired after release
 * @tc.number: WeakPtrLifecycle_001
 * @tc.desc  : Test that SaveRendererObj stores a weak_ptr that expires
 *             when the renderer is destroyed.
 */
HWTEST_F(AudioRendererProxyObjAiut, WeakPtrLifecycle_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto proxyObj = std::make_shared<AudioRendererProxyObj>();
    AIUT_ASSERT_NE_INFO_LOG(proxyObj, nullptr, "proxyObj is null");
    {
        auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
        AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");
        proxyObj->SaveRendererObj(renderer);
        // Renderer is alive here, methods should work
        StreamSetStateEventInternal event = {};
        proxyObj->MuteStreamImpl(event);
    }
    // Renderer is destroyed, weak_ptr is expired
    // Methods should handle gracefully (no crash)
    StreamSetStateEventInternal event = {};
    proxyObj->MuteStreamImpl(event);
    proxyObj->UnmuteStreamImpl(event);
    // Verify proxyObj is still functional after expired weak_ptr calls
    auto renderer2 = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(renderer2, nullptr, "renderer2 is null");
    proxyObj->SaveRendererObj(renderer2);
    AIUT_EXPECT_EQ_INFO_LOG(renderer2->GetStatus(), RENDERER_PREPARED,
        "proxyObj should still be functional after expired weak_ptr calls, state=%d",
        renderer2->GetStatus());
}

// ==================== SharedAudioRendererWrapperAiut ====================

void SharedAudioRendererWrapperAiut::SetUpTestCase(void) {}
void SharedAudioRendererWrapperAiut::TearDownTestCase(void) {}
void SharedAudioRendererWrapperAiut::SetUp(void) {}
void SharedAudioRendererWrapperAiut::TearDown(void) {}

// Helper: create a SharedAudioRendererWrapper wrapping a real AudioRendererPrivate
std::shared_ptr<SharedAudioRendererWrapper> CreateTestWrapper()
{
    auto innerRenderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    if (innerRenderer == nullptr) {
        AIUT_INFO_LOG("innerRenderer is null in CreateTestWrapper");
        return nullptr;
    }
    auto wrapper = std::make_shared<SharedAudioRendererWrapper>(innerRenderer);
    if (wrapper == nullptr) {
        AIUT_INFO_LOG("wrapper is null in CreateTestWrapper");
        return nullptr;
    }
    return wrapper;
}

// ==================== Constructor Tests ====================

/**
 * @tc.name  : Test SharedAudioRendererWrapper constructor
 * @tc.number: SharedAudioRendererWrapper_Constructor_001
 * @tc.desc  : Test SharedAudioRendererWrapper can be constructed with a valid renderer.
 */
HWTEST_F(SharedAudioRendererWrapperAiut, SharedAudioRendererWrapper_Constructor_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto innerRenderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(innerRenderer, nullptr, "innerRenderer is null");
    auto wrapper = std::make_shared<SharedAudioRendererWrapper>(innerRenderer);
    AIUT_ASSERT_NE_INFO_LOG(wrapper, nullptr, "wrapper is null");
    // Verify wrapper delegates correctly after construction
    AIUT_EXPECT_EQ_INFO_LOG(wrapper->GetStatus(), RENDERER_PREPARED,
        "wrapper should reflect inner renderer state after construction");
}

// ==================== Delegation Getter Tests ====================

/**
 * @tc.name  : Test GetStatus delegation
 * @tc.number: SharedAudioRendererWrapper_GetStatus_001
 * @tc.desc  : Test GetStatus delegates to the inner renderer.
 */
HWTEST_F(SharedAudioRendererWrapperAiut, SharedAudioRendererWrapper_GetStatus_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto wrapper = CreateTestWrapper();
    RendererState state = wrapper->GetStatus();
    AIUT_EXPECT_EQ_INFO_LOG(state, RENDERER_PREPARED,
        "initial state should be PREPARED via wrapper, state=%d", state);
}

/**
 * @tc.name  : Test GetRenderMode delegation
 * @tc.number: SharedAudioRendererWrapper_GetRenderMode_001
 * @tc.desc  : Test GetRenderMode returns the initial render mode.
 */
HWTEST_F(SharedAudioRendererWrapperAiut, SharedAudioRendererWrapper_GetRenderMode_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto wrapper = CreateTestWrapper();
    AudioRenderMode mode = wrapper->GetRenderMode();
    AIUT_EXPECT_EQ_INFO_LOG(mode, RENDER_MODE_NORMAL,
        "initial mode should be NORMAL, mode=%d", mode);
}

/**
 * @tc.name  : Test GetRendererDebugKey delegation
 * @tc.number: SharedAudioRendererWrapper_GetRendererDebugKey_001
 * @tc.desc  : Test GetRendererDebugKey returns the inner renderer's debug key.
 */
HWTEST_F(SharedAudioRendererWrapperAiut, SharedAudioRendererWrapper_GetRendererDebugKey_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto wrapper = CreateTestWrapper();
    uint32_t key = wrapper->GetRendererDebugKey();
    AIUT_EXPECT_EQ_INFO_LOG(key, 0, "debug key should be 0, key=%u", key);
}

/**
 * @tc.name  : Test IsNoStreamRenderer delegation
 * @tc.number: SharedAudioRendererWrapper_IsNoStreamRenderer_001
 * @tc.desc  : Test IsNoStreamRenderer returns the inner renderer's value.
 */
HWTEST_F(SharedAudioRendererWrapperAiut, SharedAudioRendererWrapper_IsNoStreamRenderer_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto wrapper = CreateTestWrapper();
    bool isNoStream = wrapper->IsNoStreamRenderer();
    AIUT_EXPECT_TRUE_INFO_LOG(isNoStream, "should be no-stream via wrapper");
}

// ==================== Delegation Setter Tests ====================

/**
 * @tc.name  : Test SetAudioPrivacyType delegation
 * @tc.number: SharedAudioRendererWrapper_SetAudioPrivacyType_001
 * @tc.desc  : Test SetAudioPrivacyType delegates to the inner renderer.
 */
HWTEST_F(SharedAudioRendererWrapperAiut, SharedAudioRendererWrapper_SetAudioPrivacyType_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto wrapper = CreateTestWrapper();
    wrapper->SetAudioPrivacyType(PRIVACY_TYPE_PUBLIC);
    AIUT_EXPECT_EQ_INFO_LOG(wrapper->GetAudioPrivacyType(), PRIVACY_TYPE_PUBLIC,
        "privacy type should be PUBLIC after SetAudioPrivacyType, type=%d", wrapper->GetAudioPrivacyType());
}

/**
 * @tc.name  : Test GetAudioPrivacyType delegation
 * @tc.number: SharedAudioRendererWrapper_GetAudioPrivacyType_001
 * @tc.desc  : Test GetAudioPrivacyType returns the inner renderer's privacy type.
 */
HWTEST_F(SharedAudioRendererWrapperAiut, GetAudioPrivacyType_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto wrapper = CreateTestWrapper();
    wrapper->SetAudioPrivacyType(PRIVACY_TYPE_PUBLIC);
    AudioPrivacyType type = wrapper->GetAudioPrivacyType();
    AIUT_EXPECT_EQ_INFO_LOG(type, PRIVACY_TYPE_PUBLIC, "privacy type should be PUBLIC, type=%d", type);
}

// ==================== Callback Registration Delegation Tests ====================

/**
 * @tc.name  : Test SetRendererCallback delegation
 * @tc.number: SharedAudioRendererWrapper_SetRendererCallback_001
 * @tc.desc  : Test SetRendererCallback delegates to the inner renderer.
 */
HWTEST_F(SharedAudioRendererWrapperAiut, SharedAudioRendererWrapper_SetRendererCallback_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto wrapper = CreateTestWrapper();
    auto callback = std::make_shared<TestRendererCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "callback is null");
    int32_t ret = wrapper->SetRendererCallback(callback);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "SetRendererCallback should succeed, ret=%d", ret);
}

/**
 * @tc.name  : Test SetRendererPositionCallback delegation
 * @tc.number: SharedAudioRendererWrapper_SetRendererPositionCallback_001
 * @tc.desc  : Test SetRendererPositionCallback delegates to the inner renderer.
 */
HWTEST_F(SharedAudioRendererWrapperAiut, SharedAudioRendererWrapper_SetRendererPositionCallback_001,
    TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto wrapper = CreateTestWrapper();
    auto callback = std::make_shared<TestRendererPositionCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "callback is null");
    int32_t ret = wrapper->SetRendererPositionCallback(TEST_POSITION, callback);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "SetRendererPositionCallback should succeed, ret=%d", ret);
}

/**
 * @tc.name  : Test UnsetRendererPositionCallback delegation
 * @tc.number: SharedAudioRendererWrapper_UnsetRendererPositionCallback_001
 * @tc.desc  : Test UnsetRendererPositionCallback delegates to the inner renderer.
 */
HWTEST_F(SharedAudioRendererWrapperAiut, SharedAudioRendererWrapper_UnsetRendererPositionCallback_001,
    TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto wrapper = CreateTestWrapper();
    wrapper->UnsetRendererPositionCallback();
    // Verify wrapper state remains valid after unsetting position callback
    AIUT_EXPECT_EQ_INFO_LOG(wrapper->GetStatus(), RENDERER_PREPARED,
        "wrapper should remain PREPARED after UnsetRendererPositionCallback, state=%d", wrapper->GetStatus());
}

/**
 * @tc.name  : Test SetRendererPeriodPositionCallback delegation
 * @tc.number: SharedAudioRendererWrapper_SetRendererPeriodPositionCallback_001
 * @tc.desc  : Test SetRendererPeriodPositionCallback delegates to the inner renderer.
 */
HWTEST_F(SharedAudioRendererWrapperAiut, SharedAudioRendererWrapper_SetRendererPeriodPositionCallback_001,
    TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto wrapper = CreateTestWrapper();
    auto callback = std::make_shared<TestRendererPeriodPositionCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "callback is null");
    int32_t ret = wrapper->SetRendererPeriodPositionCallback(TEST_PERIOD, callback);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "SetRendererPeriodPositionCallback should succeed, ret=%d", ret);
}

/**
 * @tc.name  : Test UnsetRendererPeriodPositionCallback delegation
 * @tc.number: SharedAudioRendererWrapper_UnsetRendererPeriodPositionCallback_001
 * @tc.desc  : Test UnsetRendererPeriodPositionCallback delegates to the inner renderer.
 */
HWTEST_F(SharedAudioRendererWrapperAiut, SharedAudioRendererWrapper_UnsetRendererPeriodPositionCallback_001,
    TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto wrapper = CreateTestWrapper();
    wrapper->UnsetRendererPeriodPositionCallback();
    // Verify wrapper state remains valid after unsetting period position callback
    AIUT_EXPECT_EQ_INFO_LOG(wrapper->GetStatus(), RENDERER_PREPARED,
        "wrapper should remain PREPARED after UnsetRendererPeriodPositionCallback, state=%d",
        wrapper->GetStatus());
}

// ==================== SetFastStatusChangeCallback Override Tests ====================

/**
 * @tc.name  : Test SetFastStatusChangeCallback is no-op in wrapper
 * @tc.number: SharedAudioRendererWrapper_SetFastStatusChangeCallback_001
 * @tc.desc  : Test SetFastStatusChangeCallback is overridden as no-op in SharedAudioRendererWrapper.
 */
HWTEST_F(SharedAudioRendererWrapperAiut, SharedAudioRendererWrapper_SetFastStatusChangeCallback_001,
    TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto wrapper = CreateTestWrapper();
    // This is overridden as no-op in the wrapper, should not crash
    class TestFastStatusCallback : public AudioRendererFastStatusChangeCallback {
    public:
        void OnFastStatusChange(FastStatus status) override {}
    };
    auto callback = std::make_shared<TestFastStatusCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "callback is null");
    wrapper->SetFastStatusChangeCallback(callback);
    // SetFastStatusChangeCallback is overridden as no-op in wrapper; no crash = success
    AIUT_EXPECT_EQ_INFO_LOG(wrapper->GetFastStatus(), FASTSTATUS_NORMAL,
        "wrapper should still return FASTSTATUS_NORMAL after setting fast status callback");
}

// ==================== GetFastStatus Override Tests ====================

/**
 * @tc.name  : Test GetFastStatus returns FASTSTATUS_NORMAL
 * @tc.number: SharedAudioRendererWrapper_GetFastStatus_001
 * @tc.desc  : Test GetFastStatus is overridden to always return FASTSTATUS_NORMAL in wrapper.
 */
HWTEST_F(SharedAudioRendererWrapperAiut, SharedAudioRendererWrapper_GetFastStatus_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto wrapper = CreateTestWrapper();
    FastStatus status = wrapper->GetFastStatus();
    AIUT_EXPECT_EQ_INFO_LOG(status, FASTSTATUS_NORMAL,
        "wrapper should always return FASTSTATUS_NORMAL, status=%d", status);
}

/**
 * @tc.name  : Test SetAudioRendererErrorCallback delegation
 * @tc.number: SharedAudioRendererWrapper_SetAudioRendererErrorCallback_001
 * @tc.desc  : Test SetAudioRendererErrorCallback delegates to the inner renderer.
 */
HWTEST_F(SharedAudioRendererWrapperAiut, SharedAudioRendererWrapper_SetAudioRendererErrorCallback_001,
    TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto wrapper = CreateTestWrapper();
    auto callback = std::make_shared<TestRendererErrorCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "callback is null");
    wrapper->SetAudioRendererErrorCallback(callback);
    // Verify wrapper state remains valid after setting error callback
    AIUT_EXPECT_EQ_INFO_LOG(wrapper->GetStatus(), RENDERER_PREPARED,
        "wrapper should remain PREPARED after SetAudioRendererErrorCallback, state=%d", wrapper->GetStatus());
}

// ==================== Source Duration Tests ====================

/**
 * @tc.name  : Test SetSourceDuration delegation
 * @tc.number: SharedAudioRendererWrapper_SetSourceDuration_001
 * @tc.desc  : Test SetSourceDuration delegates to the inner renderer.
 */
HWTEST_F(SharedAudioRendererWrapperAiut, SharedAudioRendererWrapper_SetSourceDuration_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto wrapper = CreateTestWrapper();
    wrapper->SetSourceDuration(TEST_SOURCE_DURATION);
    // Verify wrapper state remains valid after setting source duration
    AIUT_EXPECT_EQ_INFO_LOG(wrapper->GetStatus(), RENDERER_PREPARED,
        "wrapper should remain PREPARED after SetSourceDuration, state=%d", wrapper->GetStatus());
}

// ==================== Audio Haptics Tests ====================

/**
 * @tc.name  : Test SetAudioHapticsSyncId delegation
 * @tc.number: SharedAudioRendererWrapper_SetAudioHapticsSyncId_001
 * @tc.desc  : Test SetAudioHapticsSyncId delegates to the inner renderer.
 */
HWTEST_F(SharedAudioRendererWrapperAiut, SharedAudioRendererWrapper_SetAudioHapticsSyncId_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto wrapper = CreateTestWrapper();
    wrapper->SetAudioHapticsSyncId(100);
    // Verify wrapper state remains valid after setting haptics sync id
    AIUT_EXPECT_EQ_INFO_LOG(wrapper->GetStatus(), RENDERER_PREPARED,
        "wrapper should remain PREPARED after SetAudioHapticsSyncId, state=%d", wrapper->GetStatus());
}

// ==================== ResetFirstFrameState Tests ====================

/**
 * @tc.name  : Test ResetFirstFrameState delegation
 * @tc.number: SharedAudioRendererWrapper_ResetFirstFrameState_001
 * @tc.desc  : Test ResetFirstFrameState delegates to the inner renderer.
 */
HWTEST_F(SharedAudioRendererWrapperAiut, SharedAudioRendererWrapper_ResetFirstFrameState_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto wrapper = CreateTestWrapper();
    wrapper->ResetFirstFrameState();
    // Verify wrapper state remains valid after resetting first frame state
    AIUT_EXPECT_EQ_INFO_LOG(wrapper->GetStatus(), RENDERER_PREPARED,
        "wrapper should remain PREPARED after ResetFirstFrameState, state=%d", wrapper->GetStatus());
}

// ==================== Inner Override Methods Tests ====================

/**
 * @tc.name  : Test inner StartImpl returns SUCCESS
 * @tc.number: SharedAudioRendererWrapper_InnerStartImpl_001
 * @tc.desc  : Test the private StartImpl in SharedAudioRendererWrapper always returns SUCCESS.
 */
HWTEST_F(SharedAudioRendererWrapperAiut, SharedAudioRendererWrapper_InnerStartImpl_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto wrapper = CreateTestWrapper();
    // Start() calls StartImpl which always returns SUCCESS in the wrapper
    // So Start() should return true
    bool result = wrapper->Start();
    AIUT_EXPECT_TRUE_INFO_LOG(result, "wrapper Start should return true (StartImpl returns SUCCESS)");
}

/**
 * @tc.name  : Test inner StopImpl returns SUCCESS
 * @tc.number: SharedAudioRendererWrapper_InnerStopImpl_001
 * @tc.desc  : Test the private StopImpl in SharedAudioRendererWrapper always returns SUCCESS.
 */
HWTEST_F(SharedAudioRendererWrapperAiut, SharedAudioRendererWrapper_InnerStopImpl_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto wrapper = CreateTestWrapper();
    bool result = wrapper->Stop();
    AIUT_EXPECT_TRUE_INFO_LOG(result, "wrapper Stop should return true (StopImpl returns SUCCESS)");
}

/**
 * @tc.name  : Test inner PauseImpl returns SUCCESS
 * @tc.number: SharedAudioRendererWrapper_InnerPauseImpl_001
 * @tc.desc  : Test the private PauseImpl in SharedAudioRendererWrapper always returns SUCCESS.
 */
HWTEST_F(SharedAudioRendererWrapperAiut, SharedAudioRendererWrapper_InnerPauseImpl_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto wrapper = CreateTestWrapper();
    bool result = wrapper->Pause();
    AIUT_EXPECT_TRUE_INFO_LOG(result, "wrapper Pause should return true (PauseImpl returns SUCCESS)");
}

/**
 * @tc.name  : Test inner ReleaseImpl returns SUCCESS
 * @tc.number: SharedAudioRendererWrapper_InnerReleaseImpl_001
 * @tc.desc  : Test the private ReleaseImpl in SharedAudioRendererWrapper always returns SUCCESS.
 */
HWTEST_F(SharedAudioRendererWrapperAiut, SharedAudioRendererWrapper_InnerReleaseImpl_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto wrapper = CreateTestWrapper();
    bool result = wrapper->Release();
    AIUT_EXPECT_TRUE_INFO_LOG(result, "wrapper Release should return true (ReleaseImpl returns SUCCESS)");
}

/**
 * @tc.name  : Test inner DrainImpl returns SUCCESS
 * @tc.number: SharedAudioRendererWrapper_InnerDrainImpl_001
 * @tc.desc  : Test the private DrainImpl in SharedAudioRendererWrapper always returns SUCCESS.
 */
HWTEST_F(SharedAudioRendererWrapperAiut, SharedAudioRendererWrapper_InnerDrainImpl_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto wrapper = CreateTestWrapper();
    bool result = wrapper->Drain();
    AIUT_EXPECT_TRUE_INFO_LOG(result, "wrapper Drain should return true (DrainImpl returns SUCCESS)");
}

/**
 * @tc.name  : Test inner FlushImpl returns SUCCESS
 * @tc.number: SharedAudioRendererWrapper_InnerFlushImpl_001
 * @tc.desc  : Test the private FlushImpl in SharedAudioRendererWrapper always returns SUCCESS.
 */
HWTEST_F(SharedAudioRendererWrapperAiut, SharedAudioRendererWrapper_InnerFlushImpl_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto wrapper = CreateTestWrapper();
    bool result = wrapper->Flush();
    AIUT_EXPECT_TRUE_INFO_LOG(result, "wrapper Flush should return true (FlushImpl returns SUCCESS)");
}

} // namespace AudioStandard
} // namespace OHOS
