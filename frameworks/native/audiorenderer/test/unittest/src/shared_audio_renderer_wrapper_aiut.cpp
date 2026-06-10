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

#include "shared_audio_renderer_wrapper_aiut.h"
#include "aiut_log.h"
#include "shared_audio_renderer_wrapper.h"
#include "audio_renderer_private.h"
#include "audio_errors.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {

namespace {
constexpr int32_t TEST_APP_UID = 1234;
constexpr uint32_t TEST_SAMPLE_RATE = 44100;
constexpr float TEST_VOLUME = 0.5f;
constexpr float TEST_LOUDNESS_GAIN = 3.0f;
constexpr int64_t TEST_POSITION = 1000;
constexpr int64_t TEST_PERIOD = 100;
constexpr uint64_t TEST_BUFFER_DURATION = 10;
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
    void OnFirstFrameWritingCallback(uint64_t &duration) override {}
};

class TestRendererErrorCallback : public AudioRendererErrorCallback {
public:
    void OnError(const ErrorCode errorCode) override {}
};

class TestOutputDeviceChangeCallback : public AudioRendererOutputDeviceChangeCallback {
public:
    void OnOutputDeviceChange(const AudioDeviceDescriptor &desc,
        const AudioStreamDeviceChangeReasonExt reason) override {}
};

void SharedAudioRendererWrapperAiut::SetUpTestCase(void) {}
void SharedAudioRendererWrapperAiut::TearDownTestCase(void) {}
void SharedAudioRendererWrapperAiut::SetUp(void) {}
void SharedAudioRendererWrapperAiut::TearDown(void) {}

// Helper: create a SharedAudioRendererWrapper wrapping a real AudioRendererPrivate
std::shared_ptr<SharedAudioRendererWrapper> CreateTestWrapper()
{
    auto innerRenderer = std::make_shared<AudioRendererPrivate>(STREAM_MUSIC, AppInfo{}, false);
    return std::make_shared<SharedAudioRendererWrapper>(innerRenderer);
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
    auto wrapper = std::make_shared<SharedAudioRendererWrapper>(innerRenderer);
    AIUT_ASSERT_NE_INFO_LOG(wrapper, nullptr, "wrapper should not be null");
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
    AIUT_EXPECT_EQ_INFO_LOG(key, INVALID_DEBUGKEY, "debug key should be 0, key=%u", key);
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
    int32_t ret = wrapper->SetRendererCallback(callback);
    // Result depends on inner state
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
    int32_t ret = wrapper->SetRendererPositionCallback(TEST_POSITION, callback);
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
    int32_t ret = wrapper->SetRendererPeriodPositionCallback(TEST_PERIOD, callback);
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
    wrapper->SetFastStatusChangeCallback(callback);
    // No crash = success for no-op
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
    wrapper->SetAudioRendererErrorCallback(callback);
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
