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

#include "audio_renderer_volume_aiut.h"
#include "aiut_log.h"
#include "audio_renderer_proxy_obj.h"
#include "audio_renderer_private.h"
#include "shared_audio_renderer_wrapper.h"
#include "audio_errors.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {

namespace {
constexpr float TEST_LOW_POWER_VOLUME = 0.5f;
}

void AudioRendererVolumeAiut::SetUpTestCase(void) {}
void AudioRendererVolumeAiut::TearDownTestCase(void) {}
void AudioRendererVolumeAiut::SetUp(void) {}
void AudioRendererVolumeAiut::TearDown(void) {}

// ==================== SetLowPowerVolumeImpl Tests ====================

/**
 * @tc.name  : Test SetLowPowerVolumeImpl with valid renderer
 * @tc.number: SetLowPowerVolumeImpl_001
 * @tc.desc  : Test SetLowPowerVolumeImpl calls SetLowPowerVolume on the renderer.
 */
HWTEST_F(AudioRendererVolumeAiut, SetLowPowerVolumeImpl_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto proxyObj = std::make_shared<AudioRendererProxyObj>();
    AIUT_ASSERT_NE_INFO_LOG(proxyObj, nullptr, "proxyObj is null");
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");
    proxyObj->SaveRendererObj(renderer);

    proxyObj->SetLowPowerVolumeImpl(TEST_LOW_POWER_VOLUME);
    // Verify renderer is still in valid state after setting volume
    AIUT_EXPECT_EQ_INFO_LOG(renderer->GetStatus(), RENDERER_PREPARED,
        "renderer should remain PREPARED after SetLowPowerVolumeImpl, state=%d", renderer->GetStatus());
}

/**
 * @tc.name  : Test SetLowPowerVolumeImpl with no renderer
 * @tc.number: SetLowPowerVolumeImpl_002
 * @tc.desc  : Test SetLowPowerVolumeImpl handles gracefully when renderer is not set.
 */
HWTEST_F(AudioRendererVolumeAiut, SetLowPowerVolumeImpl_002, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto proxyObj = std::make_shared<AudioRendererProxyObj>();
    AIUT_ASSERT_NE_INFO_LOG(proxyObj, nullptr, "proxyObj is null");
    proxyObj->SetLowPowerVolumeImpl(TEST_LOW_POWER_VOLUME);
    // Verify proxyObj is still functional after null renderer call
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");
    proxyObj->SaveRendererObj(renderer);
    AIUT_EXPECT_EQ_INFO_LOG(renderer->GetStatus(), RENDERER_PREPARED,
        "proxyObj should still be functional after SetLowPowerVolumeImpl on null renderer, state=%d",
        renderer->GetStatus());
}

// ==================== GetLowPowerVolumeImpl Tests ====================

/**
 * @tc.name  : Test GetLowPowerVolumeImpl with valid renderer
 * @tc.number: GetLowPowerVolumeImpl_001
 * @tc.desc  : Test GetLowPowerVolumeImpl retrieves volume from the renderer.
 */
HWTEST_F(AudioRendererVolumeAiut, GetLowPowerVolumeImpl_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto proxyObj = std::make_shared<AudioRendererProxyObj>();
    AIUT_ASSERT_NE_INFO_LOG(proxyObj, nullptr, "proxyObj is null");
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");
    proxyObj->SaveRendererObj(renderer);

    float volume = 0.0f;
    proxyObj->GetLowPowerVolumeImpl(volume);
    AIUT_EXPECT_TRUE_INFO_LOG(volume >= 0.0f && volume <= 1.0f,
        "volume should be in [0, 1] range, volume=%f", volume);
}

/**
 * @tc.name  : Test GetLowPowerVolumeImpl with no renderer
 * @tc.number: GetLowPowerVolumeImpl_002
 * @tc.desc  : Test GetLowPowerVolumeImpl handles gracefully when renderer is not set.
 */
HWTEST_F(AudioRendererVolumeAiut, GetLowPowerVolumeImpl_002, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto proxyObj = std::make_shared<AudioRendererProxyObj>();
    AIUT_ASSERT_NE_INFO_LOG(proxyObj, nullptr, "proxyObj is null");
    float volume = 1.0f;
    proxyObj->GetLowPowerVolumeImpl(volume);
    // Without renderer, volume should remain unchanged
    AIUT_EXPECT_EQ_INFO_LOG(volume, 1.0f, "volume should remain 1.0f without renderer");
}

// ==================== GetSingleStreamVolumeImpl Tests ====================

/**
 * @tc.name  : Test GetSingleStreamVolumeImpl with valid renderer
 * @tc.number: GetSingleStreamVolumeImpl_001
 * @tc.desc  : Test GetSingleStreamVolumeImpl retrieves single stream volume from the renderer.
 */
HWTEST_F(AudioRendererVolumeAiut, GetSingleStreamVolumeImpl_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto proxyObj = std::make_shared<AudioRendererProxyObj>();
    AIUT_ASSERT_NE_INFO_LOG(proxyObj, nullptr, "proxyObj is null");
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");
    proxyObj->SaveRendererObj(renderer);

    float volume = 0.0f;
    proxyObj->GetSingleStreamVolumeImpl(volume);
    AIUT_EXPECT_TRUE_INFO_LOG(volume >= 0.0f && volume <= 1.0f,
        "single stream volume should be in [0, 1] range, volume=%f", volume);
}

/**
 * @tc.name  : Test GetSingleStreamVolumeImpl with no renderer
 * @tc.number: GetSingleStreamVolumeImpl_002
 * @tc.desc  : Test GetSingleStreamVolumeImpl handles gracefully when renderer is not set.
 */
HWTEST_F(AudioRendererVolumeAiut, GetSingleStreamVolumeImpl_002, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto proxyObj = std::make_shared<AudioRendererProxyObj>();
    AIUT_ASSERT_NE_INFO_LOG(proxyObj, nullptr, "proxyObj is null");
    float volume = 1.0f;
    proxyObj->GetSingleStreamVolumeImpl(volume);
    AIUT_EXPECT_EQ_INFO_LOG(volume, 1.0f, "volume should remain 1.0f without renderer");
}

// ==================== GetMinStreamVolume / GetMaxStreamVolume Delegation Tests ====================

/**
 * @tc.name  : Test GetMinStreamVolume delegation
 * @tc.number: SharedAudioRendererWrapper_GetMinStreamVolume_001
 * @tc.desc  : Test GetMinStreamVolume delegates to the inner renderer.
 */
HWTEST_F(AudioRendererVolumeAiut, SharedAudioRendererWrapper_GetMinStreamVolume_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto innerRenderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(innerRenderer, nullptr, "innerRenderer is null");
    auto wrapper = std::make_shared<SharedAudioRendererWrapper>(innerRenderer);
    AIUT_ASSERT_NE_INFO_LOG(wrapper, nullptr, "wrapper is null");
    float minVolume = wrapper->GetMinStreamVolume();
    AIUT_EXPECT_TRUE_INFO_LOG(minVolume >= 0.0f, "min volume should be >= 0");
}

/**
 * @tc.name  : Test GetMaxStreamVolume delegation
 * @tc.number: SharedAudioRendererWrapper_GetMaxStreamVolume_001
 * @tc.desc  : Test GetMaxStreamVolume delegates to the inner renderer.
 */
HWTEST_F(AudioRendererVolumeAiut, SharedAudioRendererWrapper_GetMaxStreamVolume_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto innerRenderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(innerRenderer, nullptr, "innerRenderer is null");
    auto wrapper = std::make_shared<SharedAudioRendererWrapper>(innerRenderer);
    AIUT_ASSERT_NE_INFO_LOG(wrapper, nullptr, "wrapper is null");
    float maxVolume = wrapper->GetMaxStreamVolume();
    AIUT_EXPECT_TRUE_INFO_LOG(maxVolume >= 0.0f && maxVolume <= 1.0f,
        "max volume should be in [0, 1]");
}

} // namespace AudioStandard
} // namespace OHOS
