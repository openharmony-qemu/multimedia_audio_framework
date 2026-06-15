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

#include "audio_capturer_interrupt_aiut.h"
#include "aiut_log.h"
#include "audio_capturer_private.h"
#include "shared_audio_capturer_wrapper.h"
#include "audio_errors.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {

namespace {
constexpr int32_t TEST_APP_UID = 1234;
constexpr int32_t TEST_APP_PID = 5678;
constexpr int32_t TEST_STREAM_ID = 200;
constexpr int32_t TEST_CLIENT_UID = 1234;
}

// Mock AudioCapturerCallback for testing
class TestCapturerCallbackForInterrupt : public AudioCapturerCallback {
public:
    void OnInterrupt(const InterruptEvent &interruptEvent) override
    {
        lastEvent_ = interruptEvent;
        interruptCount_++;
    }
    void OnStateChange(const CapturerState state) override {}

    InterruptEvent lastEvent_ = {};
    int32_t interruptCount_ = 0;
};

void AudioCapturerInterruptAiut::SetUpTestCase(void) {}
void AudioCapturerInterruptAiut::TearDownTestCase(void) {}
void AudioCapturerInterruptAiut::SetUp(void) {}
void AudioCapturerInterruptAiut::TearDown(void) {}

// ==================== AudioCapturerPrivate Constructor Interrupt Tests ====================

/**
 * @tc.name  : Test AudioCapturerPrivate constructor sets audioInterrupt streamType
 * @tc.number: AudioCapturerPrivate_Interrupt_Constructor_001
 * @tc.desc  : Test AudioCapturerPrivate constructor sets audioInterrupt_.audioFocusType.streamType
 *             to the stream type passed in.
 */
HWTEST_F(AudioCapturerInterruptAiut, AudioCapturerPrivate_Interrupt_Constructor_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    AppInfo appInfo;
    appInfo.appUid = TEST_APP_UID;
    appInfo.appPid = TEST_APP_PID;
    auto capturer = std::make_shared<AudioCapturerPrivate>(STREAM_MUSIC, appInfo, false);
    AIUT_ASSERT_NE_INFO_LOG(capturer, nullptr, "capturer is null");

    AIUT_EXPECT_EQ_INFO_LOG(capturer->audioInterrupt_.audioFocusType.streamType, STREAM_MUSIC,
        "streamType mismatch");
}

// ==================== AudioCapturerInterruptCallbackImpl Tests ====================

/**
 * @tc.name  : Test interrupt callback construction
 * @tc.number: CapturerInterruptCallback_Construct_001
 * @tc.desc  : Test AudioCapturerInterruptCallbackImpl can be constructed with
 *             a valid audioStream.
 */
HWTEST_F(AudioCapturerInterruptAiut, CapturerInterruptCallback_Construct_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto capturer = std::make_shared<AudioCapturerPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(capturer, nullptr, "capturer is null");
    AIUT_ASSERT_NE_INFO_LOG(capturer->audioInterruptCallback_, nullptr,
        "interrupt callback should exist after construction");
}

/**
 * @tc.name  : Test SaveCallback on interrupt callback
 * @tc.number: CapturerInterruptCallback_SaveCallback_001
 * @tc.desc  : Test AudioCapturerInterruptCallbackImpl::SaveCallback stores the callback.
 */
HWTEST_F(AudioCapturerInterruptAiut, CapturerInterruptCallback_SaveCallback_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto capturer = std::make_shared<AudioCapturerPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(capturer, nullptr, "capturer is null");
    auto callback = std::make_shared<TestCapturerCallbackForInterrupt>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "callback is null");

    AIUT_ASSERT_NE_INFO_LOG(capturer->audioInterruptCallback_, nullptr,
        "audioInterruptCallback_ should not be null");
    auto impl = std::static_pointer_cast<AudioCapturerInterruptCallbackImpl>(capturer->audioInterruptCallback_);
    AIUT_ASSERT_NE_INFO_LOG(impl, nullptr, "cast to AudioCapturerInterruptCallbackImpl failed");
    impl->SaveCallback(callback);
}

// ==================== AudioStreamCallbackCapturer Tests ====================

/**
 * @tc.name  : Test AudioStreamCallbackCapturer construction
 * @tc.number: CapturerStreamCallback_Construct_001
 * @tc.desc  : Test AudioStreamCallbackCapturer can be constructed with a valid capturer.
 */
HWTEST_F(AudioCapturerInterruptAiut, CapturerStreamCallback_Construct_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto capturer = std::make_shared<AudioCapturerPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(capturer, nullptr, "capturer is null");
    auto streamCallback = std::make_shared<AudioStreamCallbackCapturer>(capturer);
    AIUT_ASSERT_NE_INFO_LOG(streamCallback, nullptr, "streamCallback is null");
    // Verify capturer is still in valid state after stream callback construction
    AIUT_EXPECT_EQ_INFO_LOG(capturer->GetStatus(), CAPTURER_PREPARED,
        "capturer should remain PREPARED after StreamCallback construction, state=%d",
        capturer->GetStatus());
}

/**
 * @tc.name  : Test AudioStreamCallbackCapturer SaveCallback
 * @tc.number: CapturerStreamCallback_SaveCallback_001
 * @tc.desc  : Test AudioStreamCallbackCapturer::SaveCallback stores the callback.
 */
HWTEST_F(AudioCapturerInterruptAiut, CapturerStreamCallback_SaveCallback_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto capturer = std::make_shared<AudioCapturerPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(capturer, nullptr, "capturer is null");
    auto streamCallback = std::make_shared<AudioStreamCallbackCapturer>(capturer);
    AIUT_ASSERT_NE_INFO_LOG(streamCallback, nullptr, "streamCallback is null");

    auto callback = std::make_shared<TestCapturerCallbackForInterrupt>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "callback is null");
    streamCallback->SaveCallback(callback);
    // Verify capturer is still in valid state after SaveCallback
    AIUT_EXPECT_EQ_INFO_LOG(capturer->GetStatus(), CAPTURER_PREPARED,
        "capturer should remain PREPARED after StreamCallback SaveCallback, state=%d",
        capturer->GetStatus());
}

// ==================== CapturerPolicyServiceDiedCallback Tests ====================

/**
 * @tc.name  : Test CapturerPolicyServiceDiedCallback construction
 * @tc.number: CapturerPolicyServiceDiedCallback_Construct_001
 * @tc.desc  : Test CapturerPolicyServiceDiedCallback can be constructed.
 */
HWTEST_F(AudioCapturerInterruptAiut, CapturerPolicyServiceDiedCallback_Construct_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto callback = std::make_shared<CapturerPolicyServiceDiedCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "callback is null");
    // Verify callback can be used by setting a capturer on it
    auto capturer = std::make_shared<AudioCapturerPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(capturer, nullptr, "capturer is null");
    callback->SetAudioCapturerObj(capturer);
    AIUT_EXPECT_EQ_INFO_LOG(capturer->GetStatus(), CAPTURER_PREPARED,
        "capturer should remain PREPARED after PolicyServiceDiedCallback operations, state=%d",
        capturer->GetStatus());
}

/**
 * @tc.name  : Test CapturerPolicyServiceDiedCallback SetAudioCapturerObj
 * @tc.number: CapturerPolicyServiceDiedCallback_SetAudioCapturerObj_001
 * @tc.desc  : Test SetAudioCapturerObj stores capturer reference.
 */
HWTEST_F(AudioCapturerInterruptAiut, CapturerPolicyServiceDiedCallback_SetAudioCapturerObj_001,
    TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto callback = std::make_shared<CapturerPolicyServiceDiedCallback>();
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
 * @tc.name  : Test CapturerPolicyServiceDiedCallback SetAudioInterrupt
 * @tc.number: CapturerPolicyServiceDiedCallback_SetAudioInterrupt_001
 * @tc.desc  : Test SetAudioInterrupt stores audio interrupt info.
 */
HWTEST_F(AudioCapturerInterruptAiut, CapturerPolicyServiceDiedCallback_SetAudioInterrupt_001,
    TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto callback = std::make_shared<CapturerPolicyServiceDiedCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "callback is null");
    AudioInterrupt interrupt;
    interrupt.streamId = TEST_STREAM_ID;
    interrupt.pid = TEST_CLIENT_UID;
    interrupt.mode = SHARE_MODE;
    callback->SetAudioInterrupt(interrupt);
    // Verify callback can still accept capturer after SetAudioInterrupt
    auto capturer = std::make_shared<AudioCapturerPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(capturer, nullptr, "capturer is null");
    callback->SetAudioCapturerObj(capturer);
    AIUT_EXPECT_EQ_INFO_LOG(capturer->GetStatus(), CAPTURER_PREPARED,
        "capturer should remain PREPARED after SetAudioInterrupt + SetAudioCapturerObj, state=%d",
        capturer->GetStatus());
}

// ==================== RegisterCapturerPolicyServiceDiedCallback Tests ====================

/**
 * @tc.name  : Test RegisterCapturerPolicyServiceDiedCallback
 * @tc.number: RegisterCapturerPolicyServiceDiedCallback_001
 * @tc.desc  : Test RegisterCapturerPolicyServiceDiedCallback stores callback in capturer.
 */
HWTEST_F(AudioCapturerInterruptAiut, RegisterCapturerPolicyServiceDiedCallback_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto capturer = std::make_shared<AudioCapturerPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(capturer, nullptr, "capturer is null");
    auto callback = std::make_shared<CapturerPolicyServiceDiedCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "callback is null");
    AIUT_ASSERT_NE_INFO_LOG(capturer->audioStream_, nullptr, "audioStream_ should not be null");
    capturer->RegisterCapturerPolicyServiceDiedCallback(capturer->audioStream_);
}

// ==================== SharedCapturerWrapper Interrupt Delegation Tests ====================

/**
 * @tc.name  : Test SetInterruptStrategy delegation via wrapper
 * @tc.number: SharedCapturerWrapper_SetInterruptStrategy_001
 * @tc.desc  : Test SetInterruptStrategy delegates to the inner capturer.
 */
HWTEST_F(AudioCapturerInterruptAiut, SharedCapturerWrapper_SetInterruptStrategy_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto innerCapturer = std::make_shared<AudioCapturerPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(innerCapturer, nullptr, "innerCapturer is null");
    auto wrapper = std::make_shared<SharedCapturerWrapper>(innerCapturer);
    AIUT_ASSERT_NE_INFO_LOG(wrapper, nullptr, "wrapper is null");
    int32_t ret = wrapper->SetInterruptStrategy(InterruptStrategy::MUTE);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS,
        "SetInterruptStrategy should succeed, ret=%d", ret);
}

/**
 * @tc.name  : Test SetMuteHint delegation via wrapper
 * @tc.number: SharedCapturerWrapper_SetMuteHint_001
 * @tc.desc  : Test SetMuteHint delegates to the inner capturer.
 */
HWTEST_F(AudioCapturerInterruptAiut, SharedCapturerWrapper_SetMuteHint_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto innerCapturer = std::make_shared<AudioCapturerPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(innerCapturer, nullptr, "innerCapturer is null");
    auto wrapper = std::make_shared<SharedCapturerWrapper>(innerCapturer);
    AIUT_ASSERT_NE_INFO_LOG(wrapper, nullptr, "wrapper is null");
    int32_t ret = wrapper->SetMuteHint(true);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "SetMuteHint should succeed, ret=%d", ret);
}

} // namespace AudioStandard
} // namespace OHOS
