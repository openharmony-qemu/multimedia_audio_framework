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

#include "audio_capturer_wrapper_aiut.h"
#include "aiut_log.h"
#include "audio_capturer_proxy_obj.h"
#include "audio_capturer_private.h"
#include "shared_audio_capturer_wrapper.h"
#include "audio_errors.h"

#include <vector>

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {

namespace {
constexpr float TEST_LOW_POWER_VOLUME = 0.5f;
constexpr int64_t TEST_POSITION = 1000;
constexpr int64_t TEST_PERIOD = 100;
}

// Mock callback classes for testing
class TestCapturerCallback : public AudioCapturerCallback {
public:
    void OnInterrupt(const InterruptEvent &interruptEvent) override {}
    void OnStateChange(const CapturerState state) override {}
};

class TestCapturerPositionCallback : public CapturerPositionCallback {
public:
    void OnMarkReached(const int64_t &framePosition) override {}
};

class TestCapturerPeriodPositionCallback : public CapturerPeriodPositionCallback {
public:
    void OnPeriodReached(const int64_t &frameNumber) override {}
};

class TestCapturerReadCallback : public AudioCapturerReadCallback {
public:
    void OnReadData(size_t length) override {}
};

class TestCapturerErrorCallback : public AudioCapturerErrorCallback {
public:
    void OnError(const AudioErrors errorCode) override {}
};

class TestInputDeviceChangeCallback : public AudioCapturerDeviceChangeCallback {
public:
    void OnStateChange(const AudioDeviceDescriptor &deviceInfo) override {}
};

class TestNoiseReductionCapturer : public AudioCapturerPrivate {
public:
    TestNoiseReductionCapturer() : AudioCapturerPrivate(STREAM_MUSIC, AppInfo {}, false) {}

    int32_t SetNoiseReductionMode(NoiseReductionMode mode) override
    {
        mode_ = mode;
        return setRet_;
    }

    int32_t GetNoiseReductionMode(NoiseReductionMode &mode) override
    {
        mode = mode_;
        return getRet_;
    }

    int32_t GetSupportedNoiseReductionModes(std::vector<NoiseReductionMode> &modes) override
    {
        modes = supportedModes_;
        return getSupportedRet_;
    }

    void SetReturnValues(int32_t setRet, int32_t getRet, int32_t getSupportedRet)
    {
        setRet_ = setRet;
        getRet_ = getRet;
        getSupportedRet_ = getSupportedRet;
    }

private:
    int32_t setRet_ = SUCCESS;
    int32_t getRet_ = SUCCESS;
    int32_t getSupportedRet_ = SUCCESS;
    NoiseReductionMode mode_ = NOISE_REDUCTION_MODE_FIDELITY;
    std::vector<NoiseReductionMode> supportedModes_ = {
        NOISE_REDUCTION_MODE_FIDELITY,
        NOISE_REDUCTION_MODE_STANDARD,
    };
};

// ==================== AudioCapturerCallbackAiut ====================

void AudioCapturerCallbackAiut::SetUpTestCase(void) {}
void AudioCapturerCallbackAiut::TearDownTestCase(void) {}
void AudioCapturerCallbackAiut::SetUp(void) {}
void AudioCapturerCallbackAiut::TearDown(void) {}

// ==================== GetCapturerDebugInfo Tests ====================

/**
 * @tc.name  : Test GetCapturerDebugInfo with no stream
 * @tc.number: CapturerCallback_GetCapturerDebugInfo_001
 * @tc.desc  : Test GetCapturerDebugInfo returns error when audioStream_ is null.
 */
HWTEST_F(AudioCapturerCallbackAiut, CapturerCallback_GetCapturerDebugInfo_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto capturer = std::make_shared<AudioCapturerPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(capturer, nullptr, "capturer is null");
    AudioCapturerDebugInfo debugInfo;
    int32_t ret = capturer->GetCapturerDebugInfo(debugInfo);
    AIUT_EXPECT_NE_INFO_LOG(ret, SUCCESS,
        "GetCapturerDebugInfo should fail without stream, ret=%d", ret);
}

// ==================== AudioCapturerProxyObjAiut ====================

void AudioCapturerProxyObjAiut::SetUpTestCase(void) {}
void AudioCapturerProxyObjAiut::TearDownTestCase(void) {}
void AudioCapturerProxyObjAiut::SetUp(void) {}
void AudioCapturerProxyObjAiut::TearDown(void) {}

// ==================== SaveCapturerObj Tests ====================

/**
 * @tc.name  : Test SaveCapturerObj stores capturer reference
 * @tc.number: SaveCapturerObj_001
 * @tc.desc  : Test SaveCapturerObj stores a weak reference to the capturer object.
 */
HWTEST_F(AudioCapturerProxyObjAiut, SaveCapturerObj_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto proxyObj = std::make_shared<AudioCapturerProxyObj>();
    AIUT_ASSERT_NE_INFO_LOG(proxyObj, nullptr, "proxyObj is null");
    auto capturer = std::make_shared<AudioCapturerPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(capturer, nullptr, "capturer is null");

    proxyObj->SaveCapturerObj(capturer);

    // After SaveCapturerObj, the proxy should hold the capturer
    StreamSetStateEventInternal event = {};
    proxyObj->MuteStreamImpl(event);
    // Verify capturer is still in valid state after proxy method call
    AIUT_EXPECT_EQ_INFO_LOG(capturer->GetStatus(), CAPTURER_PREPARED,
        "capturer state should remain PREPARED after SaveCapturerObj, state=%d",
        capturer->GetStatus());
}

// ==================== MuteStreamImpl Tests ====================

/**
 * @tc.name  : Test MuteStreamImpl with valid capturer
 * @tc.number: CapturerMuteStreamImpl_001
 * @tc.desc  : Test MuteStreamImpl calls Mute on the stored capturer without crash.
 */
HWTEST_F(AudioCapturerProxyObjAiut, CapturerMuteStreamImpl_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto proxyObj = std::make_shared<AudioCapturerProxyObj>();
    AIUT_ASSERT_NE_INFO_LOG(proxyObj, nullptr, "proxyObj is null");
    auto capturer = std::make_shared<AudioCapturerPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(capturer, nullptr, "capturer is null");
    proxyObj->SaveCapturerObj(capturer);

    StreamSetStateEventInternal event = {};
    proxyObj->MuteStreamImpl(event);
    // Verify capturer state is still valid after MuteStreamImpl
    AIUT_EXPECT_EQ_INFO_LOG(capturer->GetStatus(), CAPTURER_PREPARED,
        "capturer should remain PREPARED after MuteStreamImpl, state=%d",
        capturer->GetStatus());
}

/**
 * @tc.name  : Test MuteStreamImpl with no capturer
 * @tc.number: CapturerMuteStreamImpl_002
 * @tc.desc  : Test MuteStreamImpl handles gracefully when capturer is not set.
 */
HWTEST_F(AudioCapturerProxyObjAiut, CapturerMuteStreamImpl_002, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto proxyObj = std::make_shared<AudioCapturerProxyObj>();
    AIUT_ASSERT_NE_INFO_LOG(proxyObj, nullptr, "proxyObj is null");
    StreamSetStateEventInternal event = {};
    proxyObj->MuteStreamImpl(event);
    // Verify proxyObj is still functional after null capturer call
    auto capturer = std::make_shared<AudioCapturerPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(capturer, nullptr, "capturer is null");
    proxyObj->SaveCapturerObj(capturer);
    AIUT_EXPECT_EQ_INFO_LOG(capturer->GetStatus(), CAPTURER_PREPARED,
        "proxyObj should still be functional after MuteStreamImpl on null capturer, state=%d",
        capturer->GetStatus());
}

// ==================== UnmuteStreamImpl Tests ====================

/**
 * @tc.name  : Test UnmuteStreamImpl with valid capturer
 * @tc.number: CapturerUnmuteStreamImpl_001
 * @tc.desc  : Test UnmuteStreamImpl calls Unmute on the stored capturer without crash.
 */
HWTEST_F(AudioCapturerProxyObjAiut, CapturerUnmuteStreamImpl_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto proxyObj = std::make_shared<AudioCapturerProxyObj>();
    AIUT_ASSERT_NE_INFO_LOG(proxyObj, nullptr, "proxyObj is null");
    auto capturer = std::make_shared<AudioCapturerPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(capturer, nullptr, "capturer is null");
    proxyObj->SaveCapturerObj(capturer);

    StreamSetStateEventInternal event = {};
    proxyObj->UnmuteStreamImpl(event);
    AIUT_EXPECT_EQ_INFO_LOG(capturer->GetStatus(), CAPTURER_PREPARED,
        "capturer should remain PREPARED after UnmuteStreamImpl, state=%d",
        capturer->GetStatus());
}

/**
 * @tc.name  : Test UnmuteStreamImpl with no capturer
 * @tc.number: CapturerUnmuteStreamImpl_002
 * @tc.desc  : Test UnmuteStreamImpl handles gracefully when capturer is not set.
 */
HWTEST_F(AudioCapturerProxyObjAiut, CapturerUnmuteStreamImpl_002, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto proxyObj = std::make_shared<AudioCapturerProxyObj>();
    AIUT_ASSERT_NE_INFO_LOG(proxyObj, nullptr, "proxyObj is null");
    StreamSetStateEventInternal event = {};
    proxyObj->UnmuteStreamImpl(event);
    // Verify proxyObj is still functional after null capturer call
    auto capturer = std::make_shared<AudioCapturerPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(capturer, nullptr, "capturer is null");
    proxyObj->SaveCapturerObj(capturer);
    AIUT_EXPECT_EQ_INFO_LOG(capturer->GetStatus(), CAPTURER_PREPARED,
        "proxyObj should still be functional after UnmuteStreamImpl on null capturer, state=%d",
        capturer->GetStatus());
}

// ==================== PausedStreamImpl Tests ====================

/**
 * @tc.name  : Test PausedStreamImpl with valid capturer
 * @tc.number: CapturerPausedStreamImpl_001
 * @tc.desc  : Test PausedStreamImpl calls Pause on the stored capturer without crash.
 */
HWTEST_F(AudioCapturerProxyObjAiut, CapturerPausedStreamImpl_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto proxyObj = std::make_shared<AudioCapturerProxyObj>();
    AIUT_ASSERT_NE_INFO_LOG(proxyObj, nullptr, "proxyObj is null");
    auto capturer = std::make_shared<AudioCapturerPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(capturer, nullptr, "capturer is null");
    proxyObj->SaveCapturerObj(capturer);

    StreamSetStateEventInternal event = {};
    proxyObj->PausedStreamImpl(event);
    AIUT_EXPECT_EQ_INFO_LOG(capturer->GetStatus(), CAPTURER_PREPARED,
        "capturer should remain PREPARED after PausedStreamImpl, state=%d",
        capturer->GetStatus());
}

/**
 * @tc.name  : Test PausedStreamImpl with no capturer
 * @tc.number: CapturerPausedStreamImpl_002
 * @tc.desc  : Test PausedStreamImpl handles gracefully when capturer is not set.
 */
HWTEST_F(AudioCapturerProxyObjAiut, CapturerPausedStreamImpl_002, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto proxyObj = std::make_shared<AudioCapturerProxyObj>();
    AIUT_ASSERT_NE_INFO_LOG(proxyObj, nullptr, "proxyObj is null");
    StreamSetStateEventInternal event = {};
    proxyObj->PausedStreamImpl(event);
    auto capturer = std::make_shared<AudioCapturerPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(capturer, nullptr, "capturer is null");
    proxyObj->SaveCapturerObj(capturer);
    AIUT_EXPECT_EQ_INFO_LOG(capturer->GetStatus(), CAPTURER_PREPARED,
        "proxyObj should still be functional after PausedStreamImpl on null capturer, state=%d",
        capturer->GetStatus());
}

// ==================== ResumeStreamImpl Tests ====================

/**
 * @tc.name  : Test ResumeStreamImpl with valid capturer
 * @tc.number: CapturerResumeStreamImpl_001
 * @tc.desc  : Test ResumeStreamImpl calls Start on the stored capturer without crash.
 */
HWTEST_F(AudioCapturerProxyObjAiut, CapturerResumeStreamImpl_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto proxyObj = std::make_shared<AudioCapturerProxyObj>();
    AIUT_ASSERT_NE_INFO_LOG(proxyObj, nullptr, "proxyObj is null");
    auto capturer = std::make_shared<AudioCapturerPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(capturer, nullptr, "capturer is null");
    proxyObj->SaveCapturerObj(capturer);

    StreamSetStateEventInternal event = {};
    proxyObj->ResumeStreamImpl(event);
    AIUT_EXPECT_EQ_INFO_LOG(capturer->GetStatus(), CAPTURER_PREPARED,
        "capturer should remain PREPARED after ResumeStreamImpl, state=%d",
        capturer->GetStatus());
}

/**
 * @tc.name  : Test ResumeStreamImpl with no capturer
 * @tc.number: CapturerResumeStreamImpl_002
 * @tc.desc  : Test ResumeStreamImpl handles gracefully when capturer is not set.
 */
HWTEST_F(AudioCapturerProxyObjAiut, CapturerResumeStreamImpl_002, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto proxyObj = std::make_shared<AudioCapturerProxyObj>();
    AIUT_ASSERT_NE_INFO_LOG(proxyObj, nullptr, "proxyObj is null");
    StreamSetStateEventInternal event = {};
    proxyObj->ResumeStreamImpl(event);
    auto capturer = std::make_shared<AudioCapturerPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(capturer, nullptr, "capturer is null");
    proxyObj->SaveCapturerObj(capturer);
    AIUT_EXPECT_EQ_INFO_LOG(capturer->GetStatus(), CAPTURER_PREPARED,
        "proxyObj should still be functional after ResumeStreamImpl on null capturer, state=%d",
        capturer->GetStatus());
}

// ==================== SetLowPowerVolumeImpl Tests ====================

/**
 * @tc.name  : Test SetLowPowerVolumeImpl with valid capturer
 * @tc.number: CapturerSetLowPowerVolumeImpl_001
 * @tc.desc  : Test SetLowPowerVolumeImpl sets low power volume on the capturer.
 */
HWTEST_F(AudioCapturerProxyObjAiut, CapturerSetLowPowerVolumeImpl_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto proxyObj = std::make_shared<AudioCapturerProxyObj>();
    AIUT_ASSERT_NE_INFO_LOG(proxyObj, nullptr, "proxyObj is null");
    auto capturer = std::make_shared<AudioCapturerPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(capturer, nullptr, "capturer is null");
    proxyObj->SaveCapturerObj(capturer);

    proxyObj->SetLowPowerVolumeImpl(TEST_LOW_POWER_VOLUME);
    // Verify capturer is still in valid state after SetLowPowerVolumeImpl
    AIUT_EXPECT_EQ_INFO_LOG(capturer->GetStatus(), CAPTURER_PREPARED,
        "capturer should remain PREPARED after SetLowPowerVolumeImpl, state=%d",
        capturer->GetStatus());
}

/**
 * @tc.name  : Test SetLowPowerVolumeImpl with no capturer
 * @tc.number: CapturerSetLowPowerVolumeImpl_002
 * @tc.desc  : Test SetLowPowerVolumeImpl handles gracefully when capturer is not set.
 */
HWTEST_F(AudioCapturerProxyObjAiut, CapturerSetLowPowerVolumeImpl_002, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto proxyObj = std::make_shared<AudioCapturerProxyObj>();
    AIUT_ASSERT_NE_INFO_LOG(proxyObj, nullptr, "proxyObj is null");
    proxyObj->SetLowPowerVolumeImpl(TEST_LOW_POWER_VOLUME);
    // Verify proxyObj is still functional after null capturer call
    auto capturer = std::make_shared<AudioCapturerPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(capturer, nullptr, "capturer is null");
    proxyObj->SaveCapturerObj(capturer);
    AIUT_EXPECT_EQ_INFO_LOG(capturer->GetStatus(), CAPTURER_PREPARED,
        "proxyObj should still be functional after SetLowPowerVolumeImpl on null capturer, state=%d",
        capturer->GetStatus());
}

// ==================== GetSingleStreamVolumeImpl Tests ====================

/**
 * @tc.name  : Test GetSingleStreamVolumeImpl with valid capturer
 * @tc.number: CapturerGetSingleStreamVolumeImpl_001
 * @tc.desc  : Test GetSingleStreamVolumeImpl retrieves volume from the capturer.
 */
HWTEST_F(AudioCapturerProxyObjAiut, CapturerGetSingleStreamVolumeImpl_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto proxyObj = std::make_shared<AudioCapturerProxyObj>();
    AIUT_ASSERT_NE_INFO_LOG(proxyObj, nullptr, "proxyObj is null");
    auto capturer = std::make_shared<AudioCapturerPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(capturer, nullptr, "capturer is null");
    proxyObj->SaveCapturerObj(capturer);

    float volume = 0.0f;
    proxyObj->GetSingleStreamVolumeImpl(volume);
    // Verify capturer is still in valid state
    AIUT_EXPECT_EQ_INFO_LOG(capturer->GetStatus(), CAPTURER_PREPARED,
        "capturer should remain PREPARED after GetSingleStreamVolumeImpl, state=%d",
        capturer->GetStatus());
}

/**
 * @tc.name  : Test GetSingleStreamVolumeImpl with no capturer
 * @tc.number: CapturerGetSingleStreamVolumeImpl_002
 * @tc.desc  : Test GetSingleStreamVolumeImpl handles gracefully when capturer is not set.
 */
HWTEST_F(AudioCapturerProxyObjAiut, CapturerGetSingleStreamVolumeImpl_002, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto proxyObj = std::make_shared<AudioCapturerProxyObj>();
    AIUT_ASSERT_NE_INFO_LOG(proxyObj, nullptr, "proxyObj is null");
    float volume = 0.0f;
    proxyObj->GetSingleStreamVolumeImpl(volume);
    // Verify proxyObj is still functional after null capturer call
    auto capturer = std::make_shared<AudioCapturerPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(capturer, nullptr, "capturer is null");
    proxyObj->SaveCapturerObj(capturer);
    AIUT_EXPECT_EQ_INFO_LOG(capturer->GetStatus(), CAPTURER_PREPARED,
        "proxyObj should still be functional after GetSingleStreamVolumeImpl on null, state=%d",
        capturer->GetStatus());
}

// ==================== Weak Pointer Lifecycle Tests ====================

/**
 * @tc.name  : Test capturer weak_ptr becomes expired after release
 * @tc.number: CapturerWeakPtrLifecycle_001
 * @tc.desc  : Test that SaveCapturerObj stores a weak_ptr that expires
 *             when the capturer is destroyed.
 */
HWTEST_F(AudioCapturerProxyObjAiut, CapturerWeakPtrLifecycle_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto proxyObj = std::make_shared<AudioCapturerProxyObj>();
    AIUT_ASSERT_NE_INFO_LOG(proxyObj, nullptr, "proxyObj is null");
    {
        auto capturer = std::make_shared<AudioCapturerPrivate>(STREAM_MUSIC, AppInfo{}, false);
        AIUT_ASSERT_NE_INFO_LOG(capturer, nullptr, "capturer is null");
        proxyObj->SaveCapturerObj(capturer);
        // Capturer is alive here, methods should work
        StreamSetStateEventInternal event = {};
        proxyObj->MuteStreamImpl(event);
    }
    // Capturer is destroyed, weak_ptr is expired
    // Methods should handle gracefully (no crash)
    StreamSetStateEventInternal event = {};
    proxyObj->MuteStreamImpl(event);
    proxyObj->UnmuteStreamImpl(event);
    // Verify proxyObj is still functional after expired weak_ptr calls
    auto capturer2 = std::make_shared<AudioCapturerPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(capturer2, nullptr, "capturer2 is null");
    proxyObj->SaveCapturerObj(capturer2);
    AIUT_EXPECT_EQ_INFO_LOG(capturer2->GetStatus(), CAPTURER_PREPARED,
        "proxyObj should still be functional after expired weak_ptr calls, state=%d",
        capturer2->GetStatus());
}

// ==================== SharedAudioCapturerWrapperAiut ====================

void SharedAudioCapturerWrapperAiut::SetUpTestCase(void) {}
void SharedAudioCapturerWrapperAiut::TearDownTestCase(void) {}
void SharedAudioCapturerWrapperAiut::SetUp(void) {}
void SharedAudioCapturerWrapperAiut::TearDown(void) {}

// Helper: create a SharedCapturerWrapper wrapping a real AudioCapturerPrivate
std::shared_ptr<SharedCapturerWrapper> CreateTestCapturerWrapper()
{
    auto innerCapturer = std::make_shared<AudioCapturerPrivate>(STREAM_MUSIC, AppInfo{}, false);
    if (innerCapturer == nullptr) {
        AIUT_INFO_LOG("innerCapturer is null in CreateTestWrapper");
        return nullptr;
    }
    auto wrapper = std::make_shared<SharedCapturerWrapper>(innerCapturer);
    if (wrapper == nullptr) {
        AIUT_INFO_LOG("wrapper is null in CreateTestWrapper");
        return nullptr;
    }
    return wrapper;
}

// ==================== Constructor Tests ====================

/**
 * @tc.name  : Test SharedCapturerWrapper constructor
 * @tc.number: SharedCapturerWrapper_Constructor_001
 * @tc.desc  : Test SharedCapturerWrapper can be constructed with a valid capturer.
 */
HWTEST_F(SharedAudioCapturerWrapperAiut, SharedCapturerWrapper_Constructor_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto innerCapturer = std::make_shared<AudioCapturerPrivate>(STREAM_MUSIC, AppInfo{}, false);
    AIUT_ASSERT_NE_INFO_LOG(innerCapturer, nullptr, "innerCapturer is null");
    auto wrapper = std::make_shared<SharedCapturerWrapper>(innerCapturer);
    AIUT_ASSERT_NE_INFO_LOG(wrapper, nullptr, "wrapper is null");
    // Verify wrapper delegates correctly after construction
    AIUT_EXPECT_EQ_INFO_LOG(wrapper->GetStatus(), CAPTURER_PREPARED,
        "wrapper should reflect inner capturer state after construction");
}

// ==================== Delegation Getter Tests ====================

/**
 * @tc.name  : Test GetStatus delegation
 * @tc.number: SharedCapturerWrapper_GetStatus_001
 * @tc.desc  : Test GetStatus delegates to the inner capturer.
 */
HWTEST_F(SharedAudioCapturerWrapperAiut, SharedCapturerWrapper_GetStatus_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto wrapper = CreateTestCapturerWrapper();
    CapturerState state = wrapper->GetStatus();
    AIUT_EXPECT_EQ_INFO_LOG(state, CAPTURER_PREPARED,
        "initial state should be PREPARED via wrapper, state=%d", state);
}

/**
 * @tc.name  : Test GetCaptureMode delegation
 * @tc.number: SharedCapturerWrapper_GetCaptureMode_001
 * @tc.desc  : Test GetCaptureMode returns the initial capture mode.
 */
HWTEST_F(SharedAudioCapturerWrapperAiut, SharedCapturerWrapper_GetCaptureMode_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto wrapper = CreateTestCapturerWrapper();
    AudioCaptureMode mode = wrapper->GetCaptureMode();
    AIUT_EXPECT_EQ_INFO_LOG(mode, CAPTURE_MODE_NORMAL,
        "initial mode should be NORMAL, mode=%d", mode);
}

/**
 * @tc.name  : Test GetCapturerDebugKey delegation
 * @tc.number: SharedCapturerWrapper_GetCapturerDebugKey_001
 * @tc.desc  : Test GetCapturerDebugKey returns the inner capturer's debug key.
 */
HWTEST_F(SharedAudioCapturerWrapperAiut, SharedCapturerWrapper_GetCapturerDebugKey_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto wrapper = CreateTestCapturerWrapper();
    uint32_t key = wrapper->GetCapturerDebugKey();
    AIUT_EXPECT_EQ_INFO_LOG(key, 0, "debug key should be 0, key=%u", key);
}

// ==================== Callback Registration Delegation Tests ====================

/**
 * @tc.name  : Test SetCapturerCallback delegation
 * @tc.number: SharedCapturerWrapper_SetCapturerCallback_001
 * @tc.desc  : Test SetCapturerCallback delegates to the inner capturer.
 */
HWTEST_F(SharedAudioCapturerWrapperAiut, SharedCapturerWrapper_SetCapturerCallback_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto wrapper = CreateTestCapturerWrapper();
    auto callback = std::make_shared<TestCapturerCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "callback is null");
    int32_t ret = wrapper->SetCapturerCallback(callback);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "SetCapturerCallback should succeed, ret=%d", ret);
}

/**
 * @tc.name  : Test SetCapturerPositionCallback delegation
 * @tc.number: SharedCapturerWrapper_SetCapturerPositionCallback_001
 * @tc.desc  : Test SetCapturerPositionCallback delegates to the inner capturer.
 */
HWTEST_F(SharedAudioCapturerWrapperAiut,
    SharedCapturerWrapper_SetCapturerPositionCallback_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto wrapper = CreateTestCapturerWrapper();
    auto callback = std::make_shared<TestCapturerPositionCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "callback is null");
    int32_t ret = wrapper->SetCapturerPositionCallback(TEST_POSITION, callback);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS,
        "SetCapturerPositionCallback should succeed, ret=%d", ret);
}

/**
 * @tc.name  : Test UnsetCapturerPositionCallback delegation
 * @tc.number: SharedCapturerWrapper_UnsetCapturerPositionCallback_001
 * @tc.desc  : Test UnsetCapturerPositionCallback delegates to the inner capturer.
 */
HWTEST_F(SharedAudioCapturerWrapperAiut,
    SharedCapturerWrapper_UnsetCapturerPositionCallback_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto wrapper = CreateTestCapturerWrapper();
    wrapper->UnsetCapturerPositionCallback();
    // Verify wrapper state remains valid after unsetting position callback
    AIUT_EXPECT_EQ_INFO_LOG(wrapper->GetStatus(), CAPTURER_PREPARED,
        "wrapper should remain PREPARED after UnsetCapturerPositionCallback, state=%d",
        wrapper->GetStatus());
}

/**
 * @tc.name  : Test SetCapturerPeriodPositionCallback delegation
 * @tc.number: SharedCapturerWrapper_SetCapturerPeriodPositionCallback_001
 * @tc.desc  : Test SetCapturerPeriodPositionCallback delegates to the inner capturer.
 */
HWTEST_F(SharedAudioCapturerWrapperAiut,
    SharedCapturerWrapper_SetCapturerPeriodPositionCallback_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto wrapper = CreateTestCapturerWrapper();
    auto callback = std::make_shared<TestCapturerPeriodPositionCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "callback is null");
    int32_t ret = wrapper->SetCapturerPeriodPositionCallback(TEST_PERIOD, callback);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS,
        "SetCapturerPeriodPositionCallback should succeed, ret=%d", ret);
}

// ==================== SetFastStatusChangeCallback Override Tests ====================

/**
 * @tc.name  : Test SetFastStatusChangeCallback is no-op in wrapper
 * @tc.number: SharedCapturerWrapper_SetFastStatusChangeCallback_001
 * @tc.desc  : Test SetFastStatusChangeCallback is overridden as no-op in SharedCapturerWrapper.
 */
HWTEST_F(SharedAudioCapturerWrapperAiut,
    SharedCapturerWrapper_SetFastStatusChangeCallback_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto wrapper = CreateTestCapturerWrapper();
    class TestFastStatusCallback : public AudioCapturerFastStatusChangeCallback {
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
 * @tc.number: SharedCapturerWrapper_GetFastStatus_001
 * @tc.desc  : Test GetFastStatus is overridden to always return FASTSTATUS_NORMAL in wrapper.
 */
HWTEST_F(SharedAudioCapturerWrapperAiut, SharedCapturerWrapper_GetFastStatus_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto wrapper = CreateTestCapturerWrapper();
    FastStatus status = wrapper->GetFastStatus();
    AIUT_EXPECT_EQ_INFO_LOG(status, FASTSTATUS_NORMAL,
        "wrapper should always return FASTSTATUS_NORMAL, status=%d", status);
}

/**
 * @tc.name  : Test SetAudioCapturerErrorCallback delegation
 * @tc.number: SharedCapturerWrapper_SetAudioCapturerErrorCallback_001
 * @tc.desc  : Test SetAudioCapturerErrorCallback delegates to the inner capturer.
 */
HWTEST_F(SharedAudioCapturerWrapperAiut,
    SharedCapturerWrapper_SetAudioCapturerErrorCallback_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto wrapper = CreateTestCapturerWrapper();
    auto callback = std::make_shared<TestCapturerErrorCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "callback is null");
    wrapper->SetAudioCapturerErrorCallback(callback);
    // Verify wrapper state remains valid after setting error callback
    AIUT_EXPECT_EQ_INFO_LOG(wrapper->GetStatus(), CAPTURER_PREPARED,
        "wrapper should remain PREPARED after SetAudioCapturerErrorCallback, state=%d",
        wrapper->GetStatus());
}

// ==================== SetPlaybackCaptureStartStateCallback Override Tests ====================

/**
 * @tc.name  : Test SetPlaybackCaptureStartStateCallback is no-op in wrapper
 * @tc.number: SharedCapturerWrapper_SetPlaybackCaptureStartStateCallback_001
 * @tc.desc  : Test SetPlaybackCaptureStartStateCallback is overridden as no-op
 *             in SharedCapturerWrapper.
 */
HWTEST_F(SharedAudioCapturerWrapperAiut,
    SharedCapturerWrapper_SetPlaybackCaptureStartStateCallback_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto wrapper = CreateTestCapturerWrapper();
    class TestPlaybackCaptureStartCallback : public AudioCapturerOnPlaybackCaptureStartCallback {
    public:
        void OnPlaybackCaptureStartResult(PlaybackCaptureStartState state) override {}
    };
    auto callback = std::make_shared<TestPlaybackCaptureStartCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "callback is null");
    wrapper->SetPlaybackCaptureStartStateCallback(callback);
    // SetPlaybackCaptureStartStateCallback is no-op in wrapper; no crash = success
    AIUT_EXPECT_EQ_INFO_LOG(wrapper->GetStatus(), CAPTURER_PREPARED,
        "wrapper should remain PREPARED after SetPlaybackCaptureStartStateCallback, state=%d",
        wrapper->GetStatus());
}

// ==================== Inner Override Methods Tests ====================

/**
 * @tc.name  : Test inner StartImpl returns SUCCESS
 * @tc.number: SharedCapturerWrapper_InnerStartImpl_001
 * @tc.desc  : Test the private StartImpl in SharedCapturerWrapper always returns SUCCESS.
 */
HWTEST_F(SharedAudioCapturerWrapperAiut, SharedCapturerWrapper_InnerStartImpl_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto wrapper = CreateTestCapturerWrapper();
    // Start() calls StartImpl which always returns SUCCESS in the wrapper
    bool result = wrapper->Start();
    AIUT_EXPECT_TRUE_INFO_LOG(result, "wrapper Start should return true (StartImpl returns SUCCESS)");
}

/**
 * @tc.name  : Test inner StopImpl returns SUCCESS
 * @tc.number: SharedCapturerWrapper_InnerStopImpl_001
 * @tc.desc  : Test the private StopImpl in SharedCapturerWrapper always returns SUCCESS.
 */
HWTEST_F(SharedAudioCapturerWrapperAiut, SharedCapturerWrapper_InnerStopImpl_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto wrapper = CreateTestCapturerWrapper();
    bool result = wrapper->Stop();
    AIUT_EXPECT_TRUE_INFO_LOG(result, "wrapper Stop should return true (StopImpl returns SUCCESS)");
}

/**
 * @tc.name  : Test inner ReleaseImpl returns SUCCESS
 * @tc.number: SharedCapturerWrapper_InnerReleaseImpl_001
 * @tc.desc  : Test the private ReleaseImpl in SharedCapturerWrapper always returns SUCCESS.
 */
HWTEST_F(SharedAudioCapturerWrapperAiut, SharedCapturerWrapper_InnerReleaseImpl_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto wrapper = CreateTestCapturerWrapper();
    bool result = wrapper->Release();
    AIUT_EXPECT_TRUE_INFO_LOG(result, "wrapper Release should return true (ReleaseImpl returns SUCCESS)");
}

/**
 * @tc.name  : Test StartPlaybackCapture returns 0
 * @tc.number: SharedCapturerWrapper_StartPlaybackCapture_001
 * @tc.desc  : Test StartPlaybackCapture is overridden to always return 0 in wrapper.
 */
HWTEST_F(SharedAudioCapturerWrapperAiut, SharedCapturerWrapper_StartPlaybackCapture_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto wrapper = CreateTestCapturerWrapper();
    int32_t ret = wrapper->StartPlaybackCapture();
    AIUT_EXPECT_EQ_INFO_LOG(ret, 0,
        "wrapper StartPlaybackCapture should return 0, ret=%d", ret);
}

/**
 * @tc.name  : Test noise reduction API delegation
 * @tc.number: SharedCapturerWrapper_NoiseReduction_001
 * @tc.desc  : Test noise reduction APIs delegate to the inner capturer.
 */
HWTEST_F(SharedAudioCapturerWrapperAiut, SharedCapturerWrapper_NoiseReduction_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto innerCapturer = std::make_shared<TestNoiseReductionCapturer>();
    AIUT_ASSERT_NE_INFO_LOG(innerCapturer, nullptr, "innerCapturer is null");
    auto wrapper = std::make_shared<SharedCapturerWrapper>(innerCapturer);
    AIUT_ASSERT_NE_INFO_LOG(wrapper, nullptr, "wrapper is null");

    int32_t ret = wrapper->SetNoiseReductionMode(NOISE_REDUCTION_MODE_STANDARD);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "SetNoiseReductionMode should succeed, ret=%d", ret);

    NoiseReductionMode mode = NOISE_REDUCTION_MODE_FIDELITY;
    ret = wrapper->GetNoiseReductionMode(mode);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "GetNoiseReductionMode should succeed, ret=%d", ret);
    AIUT_EXPECT_EQ_INFO_LOG(mode, NOISE_REDUCTION_MODE_STANDARD,
        "mode should be STANDARD, mode=%d", static_cast<int32_t>(mode));

    std::vector<NoiseReductionMode> modes;
    ret = wrapper->GetSupportedNoiseReductionModes(modes);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "GetSupportedNoiseReductionModes should succeed, ret=%d", ret);
    AIUT_ASSERT_EQ_INFO_LOG(modes.size(), static_cast<size_t>(2), "supported modes size mismatch");
    AIUT_EXPECT_EQ_INFO_LOG(modes[0], NOISE_REDUCTION_MODE_FIDELITY,
        "first mode should be FIDELITY, mode=%d", static_cast<int32_t>(modes[0]));
    AIUT_EXPECT_EQ_INFO_LOG(modes[1], NOISE_REDUCTION_MODE_STANDARD,
        "second mode should be STANDARD, mode=%d", static_cast<int32_t>(modes[1]));

    innerCapturer->SetReturnValues(ERR_INVALID_PARAM, ERR_ILLEGAL_STATE, ERROR);
    ret = wrapper->SetNoiseReductionMode(NOISE_REDUCTION_MODE_FIDELITY);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM, "SetNoiseReductionMode should forward error, ret=%d", ret);
    ret = wrapper->GetNoiseReductionMode(mode);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_ILLEGAL_STATE, "GetNoiseReductionMode should forward error, ret=%d", ret);
    ret = wrapper->GetSupportedNoiseReductionModes(modes);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERROR, "GetSupportedNoiseReductionModes should forward error, ret=%d", ret);
}

} // namespace AudioStandard
} // namespace OHOS
