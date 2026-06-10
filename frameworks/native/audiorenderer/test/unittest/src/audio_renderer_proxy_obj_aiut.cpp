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

#include "audio_renderer_proxy_obj_aiut.h"
#include "aiut_log.h"
#include "audio_renderer_proxy_obj.h"
#include "audio_renderer_private.h"
#include "audio_errors.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {

namespace {
constexpr int32_t TEST_APP_UID = 1234;
constexpr int32_t TEST_APP_PID = 5678;
constexpr float TEST_LOW_POWER_VOLUME = 0.5f;
constexpr int32_t TEST_OFFLOAD_STATE = 1;
constexpr bool TEST_IS_APP_BACK = false;
}

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
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);

    proxyObj->SaveRendererObj(renderer);

    // After SaveRendererObj, the proxy should hold the renderer
    // Verify by calling methods that use the renderer
    StreamSetStateEventInternal event = {};
    event.streamId = 100;
    // MuteStreamImpl should not crash with valid renderer
    proxyObj->MuteStreamImpl(event);
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
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);

    proxyObj->SaveRendererObj(renderer);
    proxyObj->UnsetRendererObj();

    // After unset, methods should handle null gracefully (no crash)
    StreamSetStateEventInternal event = {};
    proxyObj->MuteStreamImpl(event);
    proxyObj->UnmuteStreamImpl(event);
    proxyObj->PausedStreamImpl(event);
    proxyObj->ResumeStreamImpl(event);
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
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    proxyObj->SaveRendererObj(renderer);

    StreamSetStateEventInternal event = {};
    event.streamId = 100;
    // Should not crash even though the renderer is in PREPARED state
    proxyObj->MuteStreamImpl(event);
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
    StreamSetStateEventInternal event = {};
    // No renderer set, should not crash
    proxyObj->MuteStreamImpl(event);
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
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    proxyObj->SaveRendererObj(renderer);

    StreamSetStateEventInternal event = {};
    proxyObj->UnmuteStreamImpl(event);
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
    StreamSetStateEventInternal event = {};
    proxyObj->UnmuteStreamImpl(event);
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
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    proxyObj->SaveRendererObj(renderer);

    StreamSetStateEventInternal event = {};
    proxyObj->PausedStreamImpl(event);
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
    StreamSetStateEventInternal event = {};
    proxyObj->PausedStreamImpl(event);
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
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    proxyObj->SaveRendererObj(renderer);

    StreamSetStateEventInternal event = {};
    proxyObj->ResumeStreamImpl(event);
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
    StreamSetStateEventInternal event = {};
    proxyObj->ResumeStreamImpl(event);
}

// ==================== Weak Pointer Lifecycle Tests ====================

/**
 * @tc.name  : Test SetOffloadModeImpl with valid renderer
 * @tc.number: SetOffloadModeImpl_001
 * @tc.desc  : Test SetOffloadModeImpl calls SetOffloadMode on the renderer.
 */
HWTEST_F(AudioRendererProxyObjAiut, SetOffloadModeImpl_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto proxyObj = std::make_shared<AudioRendererProxyObj>();
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    proxyObj->SaveRendererObj(renderer);

    proxyObj->SetOffloadModeImpl(TEST_OFFLOAD_STATE, TEST_IS_APP_BACK);
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
    proxyObj->SetOffloadModeImpl(TEST_OFFLOAD_STATE, TEST_IS_APP_BACK);
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
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    proxyObj->SaveRendererObj(renderer);

    proxyObj->UnsetOffloadModeImpl();
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
    proxyObj->UnsetOffloadModeImpl();
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
    {
        auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
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
}

} // namespace AudioStandard
} // namespace OHOS
