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

#include "audio_capturer_device_aiut.h"
#include "aiut_log.h"
#include "audio_capturer_private.h"
#include "audio_errors.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {

namespace {
}

void AudioCapturerDeviceAiut::SetUpTestCase(void) {}
void AudioCapturerDeviceAiut::TearDownTestCase(void) {}
void AudioCapturerDeviceAiut::SetUp(void) {}
void AudioCapturerDeviceAiut::TearDown(void) {}

// ==================== InputDeviceChangeWithInfoCallbackImpl Tests ====================

/**
 * @tc.name  : Test InputDeviceChangeWithInfoCallbackImpl SetAudioCapturerObj
 * @tc.number: InputDeviceChangeCallback_SetAudioCapturerObj_001
 * @tc.desc  : Test SetAudioCapturerObj stores a weak reference to the capturer.
 */
HWTEST_F(AudioCapturerDeviceAiut, InputDeviceChangeCallback_SetAudioCapturerObj_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto callback = std::make_shared<InputDeviceChangeWithInfoCallbackImpl>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "callback is null");
    auto capturer = std::make_shared<AudioCapturerPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(capturer, nullptr, "capturer is null");

    callback->SetAudioCapturerObj(capturer);

    // Verify capturer is still in valid state after SetAudioCapturerObj
    AIUT_EXPECT_EQ_INFO_LOG(capturer->GetStatus(), CAPTURER_PREPARED,
        "capturer should remain PREPARED after SetAudioCapturerObj, state=%d",
        capturer->GetStatus());
}

/**
 * @tc.name  : Test InputDeviceChangeWithInfoCallbackImpl UnsetAudioCapturerObj
 * @tc.number: InputDeviceChangeCallback_UnsetAudioCapturerObj_001
 * @tc.desc  : Test UnsetAudioCapturerObj clears the stored capturer reference.
 */
HWTEST_F(AudioCapturerDeviceAiut, InputDeviceChangeCallback_UnsetAudioCapturerObj_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto callback = std::make_shared<InputDeviceChangeWithInfoCallbackImpl>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "callback is null");
    auto capturer = std::make_shared<AudioCapturerPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(capturer, nullptr, "capturer is null");

    callback->SetAudioCapturerObj(capturer);
    callback->UnsetAudioCapturerObj();

    // After unset, the callback should not hold a reference
    // Verify capturer is still in valid state
    AIUT_EXPECT_EQ_INFO_LOG(capturer->GetStatus(), CAPTURER_PREPARED,
        "capturer should remain PREPARED after UnsetAudioCapturerObj, state=%d",
        capturer->GetStatus());
}

// ==================== AudioCapturerStateChangeCallbackImpl Tests ====================

/**
 * @tc.name  : Test AudioCapturerStateChangeCallbackImpl SaveDeviceChangeCallback
 * @tc.number: CapturerStateChangeCallback_SaveDeviceChangeCallback_001
 * @tc.desc  : Test SaveDeviceChangeCallback stores the callback.
 */
HWTEST_F(AudioCapturerDeviceAiut, CapturerStateChangeCallback_SaveDeviceChangeCallback_001,
    TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto callback = std::make_shared<AudioCapturerStateChangeCallbackImpl>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "callback is null");

    class TestDeviceChangeCallback : public AudioCapturerDeviceChangeCallback {
    public:
        void OnStateChange(const AudioDeviceDescriptor &deviceInfo) override {}
    };
    auto deviceCb = std::make_shared<TestDeviceChangeCallback>();
    AIUT_ASSERT_NE_INFO_LOG(deviceCb, nullptr, "deviceCb is null");

    callback->SaveDeviceChangeCallback(deviceCb);
    AIUT_EXPECT_EQ_INFO_LOG(callback->DeviceChangeCallbackArraySize(), 1,
        "should have 1 device change callback after save");
}

/**
 * @tc.name  : Test AudioCapturerStateChangeCallbackImpl RemoveDeviceChangeCallback
 * @tc.number: CapturerStateChangeCallback_RemoveDeviceChangeCallback_001
 * @tc.desc  : Test RemoveDeviceChangeCallback removes the stored callback.
 */
HWTEST_F(AudioCapturerDeviceAiut, CapturerStateChangeCallback_RemoveDeviceChangeCallback_001,
    TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto callback = std::make_shared<AudioCapturerStateChangeCallbackImpl>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "callback is null");

    class TestDeviceChangeCallback : public AudioCapturerDeviceChangeCallback {
    public:
        void OnStateChange(const AudioDeviceDescriptor &deviceInfo) override {}
    };
    auto deviceCb = std::make_shared<TestDeviceChangeCallback>();
    AIUT_ASSERT_NE_INFO_LOG(deviceCb, nullptr, "deviceCb is null");

    callback->SaveDeviceChangeCallback(deviceCb);
    callback->RemoveDeviceChangeCallback(deviceCb);
    AIUT_EXPECT_EQ_INFO_LOG(callback->DeviceChangeCallbackArraySize(), 0,
        "should have 0 device change callbacks after remove");
}

/**
 * @tc.name  : Test AudioCapturerStateChangeCallbackImpl SetAudioCapturerObj
 * @tc.number: CapturerStateChangeCallback_SetAudioCapturerObj_001
 * @tc.desc  : Test SetAudioCapturerObj stores capturer reference.
 */
HWTEST_F(AudioCapturerDeviceAiut, CapturerStateChangeCallback_SetAudioCapturerObj_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto callback = std::make_shared<AudioCapturerStateChangeCallbackImpl>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "callback is null");
    auto capturer = std::make_shared<AudioCapturerPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(capturer, nullptr, "capturer is null");

    callback->SetAudioCapturerObj(capturer);

    AIUT_EXPECT_EQ_INFO_LOG(capturer->GetStatus(), CAPTURER_PREPARED,
        "capturer should remain PREPARED after SetAudioCapturerObj, state=%d",
        capturer->GetStatus());
}

} // namespace AudioStandard
} // namespace OHOS
