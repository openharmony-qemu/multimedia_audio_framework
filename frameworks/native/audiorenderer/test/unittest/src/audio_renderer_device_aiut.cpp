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

#include "audio_renderer_device_aiut.h"
#include "aiut_log.h"
#include "audio_renderer_private.h"
#include "audio_errors.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {

void AudioRendererDeviceAiut::SetUpTestCase(void) {}
void AudioRendererDeviceAiut::TearDownTestCase(void) {}
void AudioRendererDeviceAiut::SetUp(void) {}
void AudioRendererDeviceAiut::TearDown(void) {}

// ==================== OutputDeviceChangeWithInfoCallbackImpl Tests ====================

/**
 * @tc.name  : Test OutputDeviceChangeWithInfoCallbackImpl SaveCallback
 * @tc.number: OutputDeviceCallback_SaveCallback_001
 * @tc.desc  : Test OutputDeviceChangeWithInfoCallbackImpl::SaveCallback stores the callback.
 */
HWTEST_F(AudioRendererDeviceAiut, OutputDeviceCallback_SaveCallback_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");
    auto callbackImpl = renderer->outputDeviceChangeCallback_;
    AIUT_ASSERT_NE_INFO_LOG(callbackImpl, nullptr, "outputDeviceChangeCallback_ should not be null");

    class TestOutputDeviceChangeCallback : public AudioRendererOutputDeviceChangeCallback {
    public:
        void OnOutputDeviceChange(const AudioDeviceDescriptor &desc,
            const AudioStreamDeviceChangeReasonExt reason) override {}
    };
    auto testCallback = std::make_shared<TestOutputDeviceChangeCallback>();
    AIUT_ASSERT_NE_INFO_LOG(testCallback, nullptr, "testCallback is null");
    callbackImpl->SaveCallback(testCallback);
}

/**
 * @tc.name  : Test OutputDeviceChangeWithInfoCallbackImpl RemoveCallback
 * @tc.number: OutputDeviceCallback_RemoveCallback_001
 * @tc.desc  : Test OutputDeviceChangeWithInfoCallbackImpl::RemoveCallback clears all callbacks.
 */
HWTEST_F(AudioRendererDeviceAiut, OutputDeviceCallback_RemoveCallback_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");
    auto callbackImpl = renderer->outputDeviceChangeCallback_;
    AIUT_ASSERT_NE_INFO_LOG(callbackImpl, nullptr, "outputDeviceChangeCallback_ should not be null");

    callbackImpl->RemoveCallback();
}

/**
 * @tc.name  : Test OutputDeviceChangeWithInfoCallbackImpl SetAudioRendererObj
 * @tc.number: OutputDeviceCallback_SetAudioRendererObj_001
 * @tc.desc  : Test OutputDeviceChangeWithInfoCallbackImpl::SetAudioRendererObj stores renderer ref.
 */
HWTEST_F(AudioRendererDeviceAiut, OutputDeviceCallback_SetAudioRendererObj_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");
    auto callbackImpl = renderer->outputDeviceChangeCallback_;
    AIUT_ASSERT_NE_INFO_LOG(callbackImpl, nullptr, "outputDeviceChangeCallback_ should not be null");

    callbackImpl->SetAudioRendererObj(renderer);
}

/**
 * @tc.name  : Test OutputDeviceChangeWithInfoCallbackImpl UnsetAudioRendererObj
 * @tc.number: OutputDeviceCallback_UnsetAudioRendererObj_001
 * @tc.desc  : Test OutputDeviceChangeWithInfoCallbackImpl::UnsetAudioRendererObj clears renderer ref.
 */
HWTEST_F(AudioRendererDeviceAiut, OutputDeviceCallback_UnsetAudioRendererObj_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto renderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(renderer, nullptr, "renderer is null");
    auto callbackImpl = renderer->outputDeviceChangeCallback_;
    AIUT_ASSERT_NE_INFO_LOG(callbackImpl, nullptr, "outputDeviceChangeCallback_ should not be null");

    callbackImpl->SetAudioRendererObj(renderer);
    callbackImpl->UnsetAudioRendererObj();
}

} // namespace AudioStandard
} // namespace OHOS
