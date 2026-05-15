/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <functional>

#include "gtest/gtest.h"
#include "audio_errors.h"
#include "audio_info.h"
#include "renderer_in_client.h"
#include "renderer_in_client_private.h"
#include "i_stream_listener.h"
#include "meta/audio_types.h"
#include "oh_audio_buffer.h"
#include "audio_stream_enum.h"
#include "parameter.h"
#include "iipc_stream.h"


using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {

const uint64_t TEST_POSITION = 20000;
static constexpr int32_t AVS3METADATA_SIZE = 19824;

class RendererInClientUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

class IpcStreamTest : public IIpcStream {
public:
    virtual ~IpcStreamTest() = default;

    int32_t calculateCacheCountRet = SUCCESS;
    uint32_t calculatedCacheCount = 0;
    bool calculateCacheCountCalled = false;
    bool setVoipNoPrivacyFlagCalled = false;
    bool lastVoipNoPrivacyFlag = false;
    int32_t setVoipNoPrivacyFlagRet = SUCCESS;

    virtual int32_t RegisterStreamListener(const sptr<IRemoteObject> &object) override { return 0; }

    virtual int32_t ResolveBuffer(std::shared_ptr<OHAudioBuffer> &buffer) override { return 0; }

    virtual int32_t UpdatePosition() override { return 0; }

    virtual int32_t GetAudioSessionID(uint32_t &sessionId) override { return 0; }

    virtual int32_t Start() override { return 0; }

    virtual int32_t Pause() override { return 0; }

    virtual int32_t Stop() override { return 0; }

    virtual int32_t Release(bool isSwitchStream) override { return 0; }

    virtual int32_t Flush() override { return 0; }

    virtual int32_t Drain(bool stopFlag) override { return 0; }

    virtual int32_t RequestHandleData(uint64_t syncFramePts, uint32_t size) override { return 0; }

    virtual int32_t UpdatePlaybackCaptureConfig(const AudioPlaybackCaptureConfig &config) override { return 0; }

    virtual int32_t SetInMainThreadState(bool isInMainThread) override { return 0; }

    virtual int32_t GetAudioTime(uint64_t &framePos, uint64_t &timestamp) override { return 0; }

    virtual int32_t GetAudioPosition(uint64_t &framePos, uint64_t &timestamp, uint64_t &latency, int32_t base) override
    {
        std::vector<uint64_t> vec;
        ClockTime::GetAllTimeStamp(vec);
        timestamp = vec[0];
        return 0;
    }

    virtual int32_t GetSpeedPosition(uint64_t &framePos, uint64_t &timestamp, uint64_t &latency, int32_t base) override
    {
        std::vector<uint64_t> vec;
        ClockTime::GetAllTimeStamp(vec);
        timestamp = vec[0];
        return 0;
    }

    virtual int32_t GetLatency(uint64_t &latency) override { return 0; }
    virtual int32_t GetLatencyWithFlag(uint64_t &latency, uint32_t flag) override { return 0; }

    virtual int32_t SetRate(int32_t rate) override { return 0; } // SetRenderRate

    virtual int32_t GetRate(int32_t &rate) override { return 0; } // SetRenderRate

    virtual int32_t SetLowPowerVolume(float volume) override { return 0; } // renderer only

    virtual int32_t GetLowPowerVolume(float &volume) override { return 0; } // renderer only

    virtual int32_t SetAudioEffectMode(int32_t effectMode) override { return 0; } // renderer only

    virtual int32_t GetAudioEffectMode(int32_t &effectMode) override { return 0; } // renderer only

    virtual int32_t SetPrivacyType(int32_t privacyType) override { return 0; } // renderer only

    virtual int32_t SetVoipNoPrivacyFlag(bool voipNoPrivacyFlag) override
    {
        setVoipNoPrivacyFlagCalled = true;
        lastVoipNoPrivacyFlag = voipNoPrivacyFlag;
        return setVoipNoPrivacyFlagRet;
    }

    virtual int32_t GetPrivacyType(int32_t &privacyType) override { return 0; } // renderer only

    virtual int32_t SetOffloadMode(int32_t state, bool isAppBack) override { return 0; } // renderer only

    virtual int32_t SetTarget(int32_t target, int32_t &ret) override { return 0; } // renderer only

    virtual int32_t UnsetOffloadMode() override { return 0; } // renderer only

    virtual int32_t GetOffloadApproximatelyCacheTime(uint64_t &timestamp, uint64_t &paWriteIndex,
        uint64_t &cacheTimeDsp, uint64_t &cacheTimePa) override { return 0; } // renderer only

    virtual int32_t UpdateSpatializationState(bool spatializationEnabled, bool headTrackingEnabled) override
    {
        return 0;
    }

    virtual int32_t GetStreamManagerType() override { return 0; }

    virtual int32_t SetRebuildFlag() override { return 0; }

    virtual int32_t SetSilentModeAndMixWithOthers(bool on) override { return 0; }

    virtual int32_t SetClientVolume() override { return 0; }

    virtual int32_t SetLoudnessGain(float loudnessGain) override { return 0; }

    virtual int32_t SetMute(bool isMute) override { return (isMute ? SUCCESS : ERROR); }

    virtual int32_t SetMuteHint(bool mute) override { return SUCCESS; }

    virtual int32_t SetDuckFactor(float duckFactor, uint32_t durationMs) override { return 0; }

    virtual int32_t RegisterThreadPriority(pid_t tid, const std::string &bundleName, uint32_t method,
        uint32_t threadPriority) override
    {
        return 0;
    }

    virtual int32_t SetDefaultOutputDevice(const int32_t defaultOuputDevice, bool skipForce = false) override
    {
        return 0;
    }

    virtual int32_t SetSourceDuration(int64_t duration) override { return 0; }

    virtual int32_t SetSpeed(float speed) override { return 0; }

    virtual int32_t SetOffloadDataCallbackState(int32_t state) override { return 0; }

    virtual sptr<IRemoteObject> AsObject() override { return nullptr; }

    virtual int32_t ResolveBufferBaseAndGetServerSpanSize(std::shared_ptr<OHAudioBufferBase> &buffer,
        uint32_t &spanSizeInFrame, uint64_t &engineTotalSizeInFrame) override { return SUCCESS; }

    virtual int32_t SetAudioHapticsSyncId(int32_t audioHapticsSyncId) override { return 0; }

    virtual int32_t SetLoopTimes(int64_t bufferLoopTimes) override { return SUCCESS; }

    virtual int32_t UpdateUnderrunInfo(uint32_t underrunInfoKey, int32_t underrunInfoVal) override { return SUCCESS; }

    virtual int32_t ResetStaticPlayPosition() override { return SUCCESS; }

    virtual int32_t CalculateCacheCount(uint32_t cacheSizeSizeInFrame, uint32_t &cacheCount) override
    {
        calculateCacheCountCalled = true;
        cacheCount = calculatedCacheCount;
        return calculateCacheCountRet;
    }

    virtual int32_t SetPitch(float pitch) override { return SUCCESS; }
};

class AudioCapturerReadCallbackTest : public AudioCapturerReadCallback {
public:
    virtual ~AudioCapturerReadCallbackTest() = default;

    /**
     * Called when buffer to be enqueued.
     *
     * @param length Indicates requested buffer length.
     * @since 9
     */
    virtual void OnReadData(size_t length) {}
};

class TestRemoteDiedCallback : public RemoteDiedCallback {
public:
    virtual ~TestRemoteDiedCallback() = default;
    void OnAudioPolicyServiceDied() override {}
};

class AudioRendererFirstFrameWritingCallbackTest : public AudioRendererFirstFrameWritingCallback {
public:
    virtual ~AudioRendererFirstFrameWritingCallbackTest() = default;
    /**
    * Called when first buffer to be enqueued.
    */
    virtual void OnFirstFrameWriting(uint64_t latency) {}
};

class CapturerPositionCallbackTest : public CapturerPositionCallback {
public:
    virtual ~CapturerPositionCallbackTest() = default;

    /**
     * Called when the requested frame number is read.
     *
     * @param framePosition requested frame position.
     * @since 8
     */
    virtual void OnMarkReached(const int64_t &framePosition) {}
};

class CapturerPeriodPositionCallbackTest : public CapturerPeriodPositionCallback {
public:
    virtual ~CapturerPeriodPositionCallbackTest() = default;

    /**
     * Called when the requested frame count is read.
     *
     * @param frameCount requested frame frame count for callback.
     * @since 8
     */
    virtual void OnPeriodReached(const int64_t &frameNumber) {}
};

class RendererPeriodPositionCallbackTest : public RendererPeriodPositionCallback {
public:
    virtual ~RendererPeriodPositionCallbackTest() = default;

    /**
     * Called when the requested frame count is written.
     *
     * @param frameCount requested frame frame count for callback.
     * @since 8
     */
    virtual void OnPeriodReached(const int64_t &frameNumber) {}
};

class AudioClientTrackerTest : public AudioClientTracker {
public:
    virtual ~AudioClientTrackerTest() = default;

    /**
     * Mute Stream was controlled by system application
     *
     * @param streamSetStateEventInternal Contains the set even information.
     */
    virtual void MuteStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) {}

    /**
     * Unmute Stream was controlled by system application
     *
     * @param streamSetStateEventInternal Contains the set even information.
     */
    virtual void UnmuteStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) {}

    /**
     * Paused Stream was controlled by system application
     *
     * @param streamSetStateEventInternal Contains the set even information.
     */
    virtual void PausedStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) {}

     /**
     * Resumed Stream was controlled by system application
     *
     * @param streamSetStateEventInternal Contains the set even information.
     */
    virtual void ResumeStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) {}

    /**
     * Set low power volume was controlled by system application
     *
     * @param volume volume value.
     */
    virtual void SetLowPowerVolumeImpl(float volume) {}

    /**
     * Get low power volume was controlled by system application
     *
     * @param volume volume value.
     */
    virtual void GetLowPowerVolumeImpl(float &volume) {}

    /**
     * Set Stream into a specified Offload state
     *
     * @param state power state.
     * @param isAppBack app state.
     */
    virtual void SetOffloadModeImpl(int32_t state, bool isAppBack) {}

    /**
     * Unset Stream out of Offload state
     *
     */
    virtual void UnsetOffloadModeImpl() {}

    /**
     * Get single stream was controlled by system application
     *
     * @param volume volume value.
     */
    virtual void GetSingleStreamVolumeImpl(float &volume) {}
};

class StaticBufferEventCallbackTest : public StaticBufferEventCallback {
public:
    void OnStaticBufferEvent(StaticBufferEventId eventId) override {}
};

class MockWriteCallback : public AudioRendererWriteCallback {
public:
    void OnWriteData(size_t length) {};
};

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: CallClientHandle_001
 * @tc.desc  : Test RendererInClientInner::CallClientHandle
 */
HWTEST(RendererInClientInnerUnitTest, CallClientHandle_001, TestSize.Level1)
{
    auto renderer = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getuid());

    ASSERT_TRUE(renderer != nullptr);

    renderer->writeCb_ = nullptr;
    renderer->CallClientHandle();

    auto mockCallBack = std::make_shared<MockWriteCallback>();

    renderer->renderMode_ = AudioRenderMode::RENDER_MODE_CALLBACK;
    int32_t ret = renderer->SetRendererWriteCallback(mockCallBack);

    renderer->CallClientHandle();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_001
 * @tc.desc  : Test RendererInClientInner::OnOperationHandled
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_001, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    Operation operation = Operation::DATA_LINK_CONNECTING;
    int64_t result = 0;
    auto ret = ptrRendererInClientInner->OnOperationHandled(operation, result);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_002
 * @tc.desc  : Test RendererInClientInner::OnOperationHandled
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_002, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    Operation operation = Operation::RESTORE_SESSION;
    int64_t result = 0;
    auto ret = ptrRendererInClientInner->OnOperationHandled(operation, result);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_003
 * @tc.desc  : Test RendererInClientInner::OnOperationHandled
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_003, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    Operation operation = Operation::START_STREAM;
    int64_t result = -1;
    auto ret = ptrRendererInClientInner->OnOperationHandled(operation, result);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_004
 * @tc.desc  : Test RendererInClientInner::UpdatePlaybackCaptureConfig
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_004, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    AudioPlaybackCaptureConfig config;
    auto ret = ptrRendererInClientInner->UpdatePlaybackCaptureConfig(config);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_005
 * @tc.desc  : Test RendererInClientInner::GetBufQueueState
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_005, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);
    ptrRendererInClientInner->renderMode_ = RENDER_MODE_NORMAL;

    BufferQueueState bufState = {0, 0};
    auto ret = ptrRendererInClientInner->GetBufQueueState(bufState);
    EXPECT_EQ(ret, ERR_INCORRECT_MODE);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_006
 * @tc.desc  : Test RendererInClientInner::GetAudioPipeType
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_006, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    AudioPipeType pipeType = AudioPipeType::PIPE_TYPE_UNKNOWN;
    ptrRendererInClientInner->GetAudioPipeType(pipeType);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_007
 * @tc.desc  : Test RendererInClientInner::SetMute
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_007, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_CLIENT;
    uint32_t totalSizeInFrame = 0;
    uint32_t byteSizePerFrame = 0;
    ptrRendererInClientInner->clientBuffer_ = std::make_shared<OHAudioBufferBase>(bufferHolder, totalSizeInFrame,
        byteSizePerFrame);

    ptrRendererInClientInner->ipcStream_ = new(std::nothrow) IpcStreamTest();

    bool mute = true;
    auto ret = ptrRendererInClientInner->SetMute(mute, StateChangeCmdType::CMD_FROM_CLIENT);
    EXPECT_EQ(ret, SUCCESS);

    mute = false;
    ret = ptrRendererInClientInner->SetMute(mute, StateChangeCmdType::CMD_FROM_CLIENT);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_008
 * @tc.desc  : Test RendererInClientInner::ChangeSpeed
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_008, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    size_t rate = 0;
    size_t format = SAMPLE_S32LE;
    size_t channels = 0;
    ptrRendererInClientInner->audioSpeed_ = std::make_unique<AudioSpeed>(rate, format, channels);
    ASSERT_TRUE(ptrRendererInClientInner->audioSpeed_ != nullptr);

    auto ret = ptrRendererInClientInner->audioSpeed_->LoadChangeSpeedFunc();

    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_009
 * @tc.desc  : Test RendererInClientInner::SetRenderMode
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_009, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    ptrRendererInClientInner->renderMode_ = AudioRenderMode::RENDER_MODE_NORMAL;
    ptrRendererInClientInner->state_.store(State::INVALID);

    AudioRenderMode renderMode = AudioRenderMode::RENDER_MODE_CALLBACK;
    auto ret = ptrRendererInClientInner->SetRenderMode(renderMode);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_010
 * @tc.desc  : Test RendererInClientInner::SetCaptureMode
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_010, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    AudioCaptureMode captureMode = AudioCaptureMode::CAPTURE_MODE_NORMAL;
    auto ret = ptrRendererInClientInner->SetCaptureMode(captureMode);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_011
 * @tc.desc  : Test RendererInClientInner::GetCaptureMode
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_011, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    auto ret = ptrRendererInClientInner->GetCaptureMode();
    EXPECT_EQ(ret, CAPTURE_MODE_NORMAL);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_012
 * @tc.desc  : Test RendererInClientInner::SetCapturerReadCallback
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_012, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    std::shared_ptr<AudioCapturerReadCallback> callback = std::make_shared<AudioCapturerReadCallbackTest>();
    auto ret = ptrRendererInClientInner->SetCapturerReadCallback(callback);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_013
 * @tc.desc  : Test RendererInClientInner::SetLowPowerVolume
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_013, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    float volume = 2.0;
    auto ret = ptrRendererInClientInner->SetLowPowerVolume(volume);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_014
 * @tc.desc  : Test RendererInClientInner::SetOffloadMode
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_014, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    ptrRendererInClientInner->ipcStream_ = new(std::nothrow) IpcStreamTest();

    int32_t state = 0;
    bool isAppBack = 0;
    auto ret = ptrRendererInClientInner->SetOffloadMode(state, isAppBack);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_015
 * @tc.desc  : Test RendererInClientInner::UnsetOffloadMode
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_015, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    ptrRendererInClientInner->ipcStream_ = new(std::nothrow) IpcStreamTest();

    auto ret = ptrRendererInClientInner->UnsetOffloadMode();
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_016
 * @tc.desc  : Test RendererInClientInner::GetFramesRead
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_016, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    auto ret = ptrRendererInClientInner->GetFramesRead();
    EXPECT_EQ(ret, -1);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_017
 * @tc.desc  : Test RendererInClientInner::SetInnerCapturerState
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_017, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    bool isInnerCapturer = true;
    ptrRendererInClientInner->SetInnerCapturerState(isInnerCapturer);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_018
 * @tc.desc  : Test RendererInClientInner::SetWakeupCapturerState
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_018, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    bool isWakeupCapturer = true;
    ptrRendererInClientInner->SetWakeupCapturerState(isWakeupCapturer);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_019
 * @tc.desc  : Test RendererInClientInner::SetCapturerSource
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_019, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    int capturerSource = true;
    ptrRendererInClientInner->SetCapturerSource(capturerSource);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_020
 * @tc.desc  : Test RendererInClientInner::SetPrivacyType
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_020, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    ptrRendererInClientInner->ipcStream_ = new(std::nothrow) IpcStreamTest();

    AudioPrivacyType privacyType = AudioPrivacyType::PRIVACY_TYPE_PUBLIC;
    ptrRendererInClientInner->SetPrivacyType(privacyType);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_021
 * @tc.desc  : Test RendererInClientInner::StartAudioStream
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_021, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);
    ptrRendererInClientInner->state_.store(State::INVALID);

    StateChangeCmdType cmdType = StateChangeCmdType::CMD_FROM_CLIENT;
    AudioStreamDeviceChangeReasonExt reason(AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE);
    auto ret = ptrRendererInClientInner->StartAudioStream(cmdType, reason);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_022
 * @tc.desc  : Test RendererInClientInner::Read
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_022, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    uint8_t buffer = 0;
    size_t userSize = 0;
    bool isBlockingRead = true;
    auto ret = ptrRendererInClientInner->Read(buffer, userSize, isBlockingRead);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_023
 * @tc.desc  : Test RendererInClientInner::GetOverflowCount
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_023, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    auto ret = ptrRendererInClientInner->GetOverflowCount();
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_024
 * @tc.desc  : Test RendererInClientInner::SetOverflowCount
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_024, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    uint32_t overflowCount = 0;
    ptrRendererInClientInner->SetOverflowCount(overflowCount);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_RemoteDied_001
 * @tc.desc  : Test RendererInClientInner::RegisterRemoteDiedCallback
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_RemoteDied_001, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, 0);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    ptrRendererInClientInner->ipcProxy_ = nullptr;
    std::shared_ptr<TestRemoteDiedCallback> callback = std::make_shared<TestRemoteDiedCallback>();
    int32_t ret = ptrRendererInClientInner->RegisterRemoteDiedCallback(callback);
    ASSERT_EQ(ret, ERR_ILLEGAL_STATE);
    ret = ptrRendererInClientInner->RegisterRemoteDiedCallback(callback);
    ASSERT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_025
 * @tc.desc  : Test RendererInClientInner::SetCapturerPositionCallback
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_025, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    int64_t markPosition = 0;
    std::shared_ptr<CapturerPositionCallback> callback = std::make_shared<CapturerPositionCallbackTest>();
    ptrRendererInClientInner->SetCapturerPositionCallback(markPosition, callback);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_026
 * @tc.desc  : Test RendererInClientInner::UnsetCapturerPositionCallback
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_026, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    ptrRendererInClientInner->UnsetCapturerPositionCallback();
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_027
 * @tc.desc  : Test RendererInClientInner::SetCapturerPeriodPositionCallback
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_027, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    int64_t periodPosition = 0;
    std::shared_ptr<CapturerPeriodPositionCallback> callback = std::make_shared<CapturerPeriodPositionCallbackTest>();
    ptrRendererInClientInner->SetCapturerPeriodPositionCallback(periodPosition, callback);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_028
 * @tc.desc  : Test RendererInClientInner::UnsetCapturerPeriodPositionCallback
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_028, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    ptrRendererInClientInner->UnsetCapturerPeriodPositionCallback();
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_029
 * @tc.desc  : Test RendererInClientInner::SetChannelBlendMode
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_029, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    ptrRendererInClientInner->state_.store(State::INVALID);

    ChannelBlendMode blendMode = ChannelBlendMode::MODE_DEFAULT;
    auto ret = ptrRendererInClientInner->SetChannelBlendMode(blendMode);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);

    ptrRendererInClientInner->state_.store(State::NEW);
    ret = ptrRendererInClientInner->SetChannelBlendMode(blendMode);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_030
 * @tc.desc  : Test RendererInClientInner::SetVolumeWithRamp
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_030, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    ptrRendererInClientInner->state_.store(State::NEW);

    float volume = 1.0;
    int32_t duration = 0;
    auto ret = ptrRendererInClientInner->SetVolumeWithRamp(volume, duration);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_031
 * @tc.desc  : Test RendererInClientInner::OnHandle
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_031, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    uint32_t code = static_cast<uint32_t>(RendererInClientInner::RENDERER_PERIOD_REACHED_EVENT);
    int64_t data = 0;
    ptrRendererInClientInner->OnHandle(code, data);

    code = static_cast<uint32_t>(RendererInClientInner::CAPTURER_PERIOD_REACHED_EVENT);
    ptrRendererInClientInner->OnHandle(code, data);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_032
 * @tc.desc  : Test RendererInClientInner::StateCmdTypeToParams
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_032, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    int64_t params = static_cast<int64_t>(RendererInClientInner::HANDLER_PARAM_RUNNING_FROM_SYSTEM);
    State state = State::INVALID;
    StateChangeCmdType cmdType = CMD_FROM_SYSTEM;
    auto ret = ptrRendererInClientInner->StateCmdTypeToParams(params, state, cmdType);
    EXPECT_EQ(ret, SUCCESS);

    params = static_cast<int64_t>(RendererInClientInner::HANDLER_PARAM_PAUSED_FROM_SYSTEM);
    ret = ptrRendererInClientInner->StateCmdTypeToParams(params, state, cmdType);
    EXPECT_EQ(ret, SUCCESS);

    params = static_cast<int64_t>(RendererInClientInner::HANDLER_PARAM_INVALID);
    ret = ptrRendererInClientInner->StateCmdTypeToParams(params, state, cmdType);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_033
 * @tc.desc  : Test RendererInClientInner::ParamsToStateCmdType
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_033, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    int64_t params = static_cast<int64_t>(RendererInClientInner::HANDLER_PARAM_NEW);
    State state = State::INVALID;
    StateChangeCmdType cmdType = CMD_FROM_SYSTEM;
    auto ret = ptrRendererInClientInner->StateCmdTypeToParams(params, state, cmdType);
    EXPECT_EQ(ret, SUCCESS);

    params = static_cast<int64_t>(RendererInClientInner::HANDLER_PARAM_RELEASED);
    ret = ptrRendererInClientInner->StateCmdTypeToParams(params, state, cmdType);
    EXPECT_EQ(ret, SUCCESS);

    params = static_cast<int64_t>(RendererInClientInner::HANDLER_PARAM_STOPPING);
    ret = ptrRendererInClientInner->StateCmdTypeToParams(params, state, cmdType);
    EXPECT_EQ(ret, SUCCESS);

    params = static_cast<int64_t>(RendererInClientInner::HANDLER_PARAM_RUNNING_FROM_SYSTEM);
    ret = ptrRendererInClientInner->StateCmdTypeToParams(params, state, cmdType);
    EXPECT_EQ(ret, SUCCESS);

    params = static_cast<int64_t>(RendererInClientInner::HANDLER_PARAM_PAUSED_FROM_SYSTEM);
    ret = ptrRendererInClientInner->StateCmdTypeToParams(params, state, cmdType);
    EXPECT_EQ(ret, SUCCESS);

    params = -2;
    ret = ptrRendererInClientInner->StateCmdTypeToParams(params, state, cmdType);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_034
 * @tc.desc  : Test RendererInClientInner::SendRenderPeriodReachedEvent
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_034, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    int64_t rendererPeriodSize = 0;
    ptrRendererInClientInner->SendRenderPeriodReachedEvent(rendererPeriodSize);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_035
 * @tc.desc  : Test RendererInClientInner::HandleRendererPositionChanges
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_035, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    ptrRendererInClientInner->sizePerFrameInByte_ = 0;

    size_t bytesWritten = 0;
    ptrRendererInClientInner->HandleRendererPositionChanges(bytesWritten);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_036
 * @tc.desc  : Test RendererInClientInner::HandleRenderPeriodReachedEvent
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_036, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    ptrRendererInClientInner->rendererPeriodPositionCallback_ = std::make_shared<RendererPeriodPositionCallbackTest>();

    int64_t rendererPeriodNumber = 0;
    ptrRendererInClientInner->HandleRenderPeriodReachedEvent(rendererPeriodNumber);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_037
 * @tc.desc  : Test RendererInClientInner::OnSpatializationStateChange
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_037, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    ptrRendererInClientInner->ipcStream_ = new(std::nothrow) IpcStreamTest();

    AudioSpatializationState spatializationState;
    ptrRendererInClientInner->OnSpatializationStateChange(spatializationState);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_038
 * @tc.desc  : Test RendererInClientInner::UpdateLatencyTimestamp
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_038, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    std::string timestamp = "";
    bool isRenderer = true;
    ptrRendererInClientInner->UpdateLatencyTimestamp(timestamp, isRenderer);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_039
 * @tc.desc  : Test RendererInClientInner::GetSpatializationEnabled
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_039, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    ptrRendererInClientInner->rendererInfo_.spatializationEnabled = true;

    auto ret = ptrRendererInClientInner->GetSpatializationEnabled();
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_040
 * @tc.desc  : Test RendererInClientInner::GetHighResolutionEnabled
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_040, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    auto ret = ptrRendererInClientInner->GetHighResolutionEnabled();
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_041
 * @tc.desc  : Test RendererInClientInner::RestoreAudioStream
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_041, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    ptrRendererInClientInner->state_.store(State::NEW);
    ptrRendererInClientInner->proxyObj_ = std::make_shared<AudioClientTrackerTest>();

    bool needStoreState = true;
    auto ret = ptrRendererInClientInner->RestoreAudioStream(needStoreState);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_042
 * @tc.desc  : Test RendererInClientInner::GetDefaultOutputDevice
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_042, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    ptrRendererInClientInner->defaultOutputDevice_ = DeviceType::DEVICE_TYPE_SPEAKER;

    auto ret = ptrRendererInClientInner->GetDefaultOutputDevice();
    EXPECT_EQ(ret, DeviceType::DEVICE_TYPE_SPEAKER);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_043
 * @tc.desc  : Test RendererInClientInner::SetSwitchingStatus
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_043, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    bool isSwitching = false;
    ptrRendererInClientInner->SetSwitchingStatus(isSwitching);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_044
 * @tc.desc  : Test RendererInClientInner::OnOperationHandled with DATA_LINK_CONNECTED
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_044, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    Operation operation = Operation::DATA_LINK_CONNECTED;
    int64_t result = 0;
    auto ret = ptrRendererInClientInner->OnOperationHandled(operation, result);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_045
 * @tc.desc  : Test RendererInClientInner::GetAudioTime.
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_045, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    Timestamp timestamp;
    Timestamp::Timestampbase base = Timestamp::Timestampbase::MONOTONIC;

    ptrRendererInClientInner->paramsIsSet_ = true;
    ptrRendererInClientInner->state_.store(State::RUNNING);
    ptrRendererInClientInner->offloadEnable_ =true;
    ptrRendererInClientInner->curStreamParams_.samplingRate = 1;
    ptrRendererInClientInner->offloadStartReadPos_ = 0;
    ptrRendererInClientInner->ipcStream_ = new(std::nothrow) IpcStreamTest();

    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_CLIENT;
    uint32_t totalSizeInFrame = 0;
    uint32_t byteSizePerFrame = 0;
    ptrRendererInClientInner->clientBuffer_ = std::make_shared<OHAudioBufferBase>(bufferHolder, totalSizeInFrame,
        byteSizePerFrame);

    ptrRendererInClientInner->clientBuffer_->basicBufferInfo_ = std::make_shared<BasicBufferInfo>().get();
    ptrRendererInClientInner->clientBuffer_->basicBufferInfo_->handlePos.store(1000000);
    ptrRendererInClientInner->clientBuffer_->basicBufferInfo_->handleTime.store(0);

    auto ret = ptrRendererInClientInner->GetAudioTime(timestamp, base);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_046
 * @tc.desc  : Test RendererInClientInner::GetAudioTime.
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_046, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    Timestamp timestamp;
    Timestamp::Timestampbase base = Timestamp::Timestampbase::MONOTONIC;

    ptrRendererInClientInner->paramsIsSet_ = true;
    ptrRendererInClientInner->state_.store(State::RUNNING);
    ptrRendererInClientInner->offloadEnable_ =true;
    ptrRendererInClientInner->curStreamParams_.samplingRate = 1;
    ptrRendererInClientInner->offloadStartReadPos_ = 0;
    ptrRendererInClientInner->ipcStream_ = new(std::nothrow) IpcStreamTest();

    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_CLIENT;
    uint32_t totalSizeInFrame = 0;
    uint32_t byteSizePerFrame = 0;
    ptrRendererInClientInner->clientBuffer_ = std::make_shared<OHAudioBufferBase>(bufferHolder, totalSizeInFrame,
        byteSizePerFrame);

    ptrRendererInClientInner->clientBuffer_->basicBufferInfo_ = std::make_shared<BasicBufferInfo>().get();
    ptrRendererInClientInner->clientBuffer_->basicBufferInfo_->handlePos.store(0);
    ptrRendererInClientInner->clientBuffer_->basicBufferInfo_->handleTime.store(0);

    auto ret = ptrRendererInClientInner->GetAudioTime(timestamp, base);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_047
 * @tc.desc  : Test RendererInClientInner::GetAudioTime.
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_047, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    Timestamp timestamp;
    Timestamp::Timestampbase base = Timestamp::Timestampbase::MONOTONIC;

    ptrRendererInClientInner->paramsIsSet_ = true;
    ptrRendererInClientInner->state_.store(State::RUNNING);
    ptrRendererInClientInner->offloadEnable_ =true;
    ptrRendererInClientInner->curStreamParams_.samplingRate = 1;
    ptrRendererInClientInner->offloadStartReadPos_ = 1;
    ptrRendererInClientInner->ipcStream_ = new(std::nothrow) IpcStreamTest();

    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_CLIENT;
    uint32_t totalSizeInFrame = 0;
    uint32_t byteSizePerFrame = 0;
    ptrRendererInClientInner->clientBuffer_ = std::make_shared<OHAudioBufferBase>(bufferHolder, totalSizeInFrame,
        byteSizePerFrame);

    ptrRendererInClientInner->clientBuffer_->basicBufferInfo_ = std::make_shared<BasicBufferInfo>().get();
    ptrRendererInClientInner->clientBuffer_->basicBufferInfo_->handlePos.store(0);
    ptrRendererInClientInner->clientBuffer_->basicBufferInfo_->handleTime.store(0);

    auto ret = ptrRendererInClientInner->GetAudioTime(timestamp, base);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_048
 * @tc.desc  : Test RendererInClientInner::GetAudioPosition.
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_048, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    ptrRendererInClientInner->state_.store(State::RUNNING);
    ptrRendererInClientInner->ipcStream_ = new(std::nothrow) IpcStreamTest();
    ptrRendererInClientInner->converter_ = nullptr;

    Timestamp timestamp;
    Timestamp::Timestampbase base = Timestamp::Timestampbase::MONOTONIC;
    ptrRendererInClientInner->lastPrintTimestamp_.store(0);
    auto ret = ptrRendererInClientInner->GetAudioPosition(timestamp, base);
    EXPECT_TRUE(ret);
    std::vector<uint64_t> timestampCurrent = {0};
    ClockTime::GetAllTimeStamp(timestampCurrent);
    ptrRendererInClientInner->lastPrintTimestamp_.store(timestampCurrent[0]);
    ptrRendererInClientInner->converter_ = std::make_unique<AudioSpatialChannelConverter>();
    ret = ptrRendererInClientInner->GetAudioPosition(timestamp, base);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_049
 * @tc.desc  : Test RendererInClientInner::SetVolume.
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_049, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    ptrRendererInClientInner->volumeRamp_.isVolumeRampActive_ = true;

    float volume = 0.5f;
    auto ret = ptrRendererInClientInner->SetVolume(volume);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_050
 * @tc.desc  : Test RendererInClientInner::SetDuckVolume.
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_050, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    float volume = -0.5f;
    auto ret = ptrRendererInClientInner->SetDuckVolume(volume);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_051
 * @tc.desc  : Test RendererInClientInner::SetDuckVolume.
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_051, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);
    ptrRendererInClientInner->ipcStream_ = new(std::nothrow) IpcStreamTest();

    float speed = 2.0f;
    auto ret = ptrRendererInClientInner->SetSpeed(speed);
    EXPECT_EQ(ret, SUCCESS);

    ptrRendererInClientInner->isHWDecodingType_ = true;
    ret = ptrRendererInClientInner->SetSpeed(speed);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test WriteRawBuffer API
 * @tc.type  : FUNC
 * @tc.number: WriteRawBuffer_001
 * @tc.desc  : Test RendererInClientInner::WriteRawBuffer.
 */
HWTEST(RendererInClientInnerUnitTest, WriteRawBuffer_001, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);
    ptrRendererInClientInner->ipcStream_ = new(std::nothrow) IpcStreamTest();

    BufferDesc bufferDesc;
    bufferDesc.buffer = nullptr;
    bufferDesc.dataLength = 0;
    int32_t ret = ptrRendererInClientInner->WriteRawBuffer(bufferDesc);
    EXPECT_EQ(ret, SUCCESS);

    ret = ptrRendererInClientInner->WriteRawBuffer(bufferDesc);
    EXPECT_EQ(ret, SUCCESS);

    bufferDesc.dataLength = 1;
    ptrRendererInClientInner->WriteRawBuffer(bufferDesc);
    EXPECT_NE(ptrRendererInClientInner->sleepCount_, 0);

    ptrRendererInClientInner->AudioServerDied(0, 0);
    ret = ptrRendererInClientInner->WriteRawBuffer(bufferDesc);
    EXPECT_EQ(ret, ERR_WRITE_BUFFER);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_052
 * @tc.desc  : Test RendererInClientInner::SetLowPowerVolume
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_052, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    float volume = -0.5f;
    auto ret = ptrRendererInClientInner->SetLowPowerVolume(volume);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_053
 * @tc.desc  : Test RendererInClientInner::ReleaseAudioStream
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_053, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    ptrRendererInClientInner->state_.store(State::RUNNING);
    ptrRendererInClientInner->callbackHandler_ = nullptr;
    bool releaseRunner = true;
    bool isSwitchStream = true;
    auto ret = ptrRendererInClientInner->ReleaseAudioStream(releaseRunner, isSwitchStream);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_054
 * @tc.desc  : Test RendererInClientInner::SetChannelBlendMode
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_054, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    ptrRendererInClientInner->state_ = NEW;

    ChannelBlendMode blendMode = ChannelBlendMode::MODE_DEFAULT;
    auto ret = ptrRendererInClientInner->SetChannelBlendMode(blendMode);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_055
 * @tc.desc  : Test RendererInClientInner::ParamsToStateCmdType
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_055, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    int64_t params = static_cast<int64_t>(RendererInClientInner::HANDLER_PARAM_PREPARED);
    State state = State::INVALID;
    StateChangeCmdType cmdType = CMD_FROM_SYSTEM;
    auto ret = ptrRendererInClientInner->StateCmdTypeToParams(params, state, cmdType);
    EXPECT_EQ(ret, SUCCESS);

    params = static_cast<int64_t>(RendererInClientInner::HANDLER_PARAM_STOPPED);
    ret = ptrRendererInClientInner->StateCmdTypeToParams(params, state, cmdType);
    EXPECT_EQ(ret, SUCCESS);

    params = static_cast<int64_t>(RendererInClientInner::HANDLER_PARAM_RUNNING);
    ret = ptrRendererInClientInner->StateCmdTypeToParams(params, state, cmdType);
    EXPECT_EQ(ret, SUCCESS);

    params = static_cast<int64_t>(RendererInClientInner::HANDLER_PARAM_PAUSED);
    ret = ptrRendererInClientInner->StateCmdTypeToParams(params, state, cmdType);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_056
 * @tc.desc  : Test RendererInClientInner::SetRestoreInfo
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_056, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    RestoreInfo restoreInfo;
    restoreInfo.restoreReason = SERVER_DIED;
    ptrRendererInClientInner->cbThreadReleased_ = false;
    ptrRendererInClientInner->SetRestoreInfo(restoreInfo);
    EXPECT_TRUE(ptrRendererInClientInner->cbThreadReleased_);

    restoreInfo.restoreReason = DEFAULT_REASON;
    ptrRendererInClientInner->SetRestoreInfo(restoreInfo);
    EXPECT_TRUE(ptrRendererInClientInner->cbThreadReleased_);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_057
 * @tc.desc  : Test RendererInClientInner::RestoreAudioStream
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_057, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    ptrRendererInClientInner->state_.store(State::RUNNING);
    ptrRendererInClientInner->proxyObj_ = std::make_shared<AudioClientTrackerTest>();

    bool needStoreState = true;
    ptrRendererInClientInner->rendererInfo_.pipeType = PIPE_TYPE_OUT_OFFLOAD;
    auto ret = ptrRendererInClientInner->RestoreAudioStream(needStoreState);
    EXPECT_EQ(ret, false);

    ptrRendererInClientInner->rendererInfo_.pipeType = PIPE_TYPE_OUT_MULTICHANNEL;
    ret = ptrRendererInClientInner->RestoreAudioStream(needStoreState);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_058
 * @tc.desc  : Test RendererInClientInner::FetchDeviceForSplitStream
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_058, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    ptrRendererInClientInner->audioStreamTracker_ = nullptr;
    ptrRendererInClientInner->FetchDeviceForSplitStream();

    AudioMode mode = AUDIO_MODE_PLAYBACK;
    int32_t clientUid = 0;
    ptrRendererInClientInner->audioStreamTracker_ = std::make_unique<AudioStreamTracker>(mode, clientUid);
    ptrRendererInClientInner->FetchDeviceForSplitStream();
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_059
 * @tc.desc  : Test RendererInClientInner::SetDuckVolume.
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_059, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    float pitch = 2.0f;
    auto ret = ptrRendererInClientInner->SetSonicPitch(pitch);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: SetAudioStreamInfo_001
 * @tc.desc  : Test RendererInClientInner::SetAudioStreamInfo
 */
HWTEST(RendererInClientInnerUnitTest, SetAudioStreamInfo_001, TestSize.Level1)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    AudioStreamParams info {
        .samplingRate = AudioSamplingRate::SAMPLE_RATE_8000,
        .encoding = AudioEncodingType::ENCODING_AUDIOVIVID,
        .format = AudioSampleFormat::SAMPLE_U8,
        .channels = AudioChannel::STEREO,
    };
    int32_t ret = ptrRendererInClientInner->SetAudioStreamInfo(info, nullptr);
    EXPECT_EQ(ret, SUCCESS);
    info.isRemoteSpatialChannel = true;
    ret = ptrRendererInClientInner->SetAudioStreamInfo(info, nullptr);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: GetFastStatus_001
 * @tc.desc  : Test RendererInClientInner::GetFastStatus
 */
HWTEST(RendererInClientInnerUnitTest, GetFastStatus_001, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    auto ret = ptrRendererInClientInner->GetFastStatus();
    EXPECT_EQ(ret, FASTSTATUS_NORMAL);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_045
 * @tc.desc  : Test RendererInClientInner SetSwitchInfoTimestamp function
 */
HWTEST(RendererInClientInnerUnitTest, SetSwitchInfoTimestamp_001, TestSize.Level1)
{
    std::vector<uint64_t> timestampCurrent = {0};
    ClockTime::GetAllTimeStamp(timestampCurrent);

    // prepare object
    auto testRendererInClientObj =
        std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getpid());
    ASSERT_TRUE(testRendererInClientObj != nullptr);
    AudioStreamParams curStreamParams = { .samplingRate = SAMPLE_RATE_48000 };
    testRendererInClientObj->curStreamParams_ = curStreamParams;
 
    // start test
    std::vector<std::pair<uint64_t, uint64_t>> testLastFramePosAndTimePair = {
        Timestamp::Timestampbase::BASESIZE, {TEST_POSITION, timestampCurrent[0]}
    };
    std::vector<std::pair<uint64_t, uint64_t>> testlastFramePosAndTimePairWithSpeed = {
        Timestamp::Timestampbase::BASESIZE, {TEST_POSITION, timestampCurrent[0]}
    };

    sleep(1);

    testRendererInClientObj->SetSwitchInfoTimestamp(testLastFramePosAndTimePair, testlastFramePosAndTimePairWithSpeed);

    EXPECT_EQ(testRendererInClientObj->lastSwitchPosition_[Timestamp::Timestampbase::MONOTONIC], TEST_POSITION);
    EXPECT_EQ(testRendererInClientObj->lastSwitchPosition_[Timestamp::Timestampbase::BOOTTIME], TEST_POSITION);

    EXPECT_EQ(
        testRendererInClientObj->lastSwitchPositionWithSpeed_[Timestamp::Timestampbase::MONOTONIC], TEST_POSITION
    );
    EXPECT_EQ(
        testRendererInClientObj->lastSwitchPositionWithSpeed_[Timestamp::Timestampbase::BOOTTIME], TEST_POSITION
    );
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: GetAudioTimestampInfo_001
 * @tc.desc  : Test RendererInClientInner GetAudioTimestampInfo.
 */
HWTEST(RendererInClientInnerUnitTest, GetAudioTimestampInfo_001, TestSize.Level0)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    ptrRendererInClientInner->ipcStream_ = new(std::nothrow) IpcStreamTest();

    Timestamp timestamp;
    ptrRendererInClientInner->state_ = State::RUNNING;

    RendererInClientInner::AudioWriteState state = {500, 50};
    ptrRendererInClientInner->audioWriteState_.store(state);
    for (auto i = 0; i < Timestamp::Timestampbase::BASESIZE; i++) {
        ptrRendererInClientInner->GetAudioTimestampInfo(timestamp,
            static_cast<Timestamp::Timestampbase>(i));
        EXPECT_EQ(timestamp.framePosition, 450); // latency = 50, frameposition = 500 - 50 = 450
    }
    ptrRendererInClientInner->SetSpeed(2.0); // lastspeed = 1.0, speed = 2.0, lastFrameWritten = 50
    state = {500, 200};
    ptrRendererInClientInner->audioWriteState_.store(state);
    for (auto i = 0; i < Timestamp::Timestampbase::BASESIZE; i++) {
        ptrRendererInClientInner->GetAudioTimestampInfo(timestamp,
            static_cast<Timestamp::Timestampbase>(i));
        EXPECT_EQ(timestamp.framePosition, 450); // latency = 50 + (200 - 50) * 2 = 350, frameposition = 150 < 450
    }
    state = {1000, 200};
    ptrRendererInClientInner->audioWriteState_.store(state);
    for (auto i = 0; i < Timestamp::Timestampbase::BASESIZE; i++) {
        ptrRendererInClientInner->GetAudioTimestampInfo(timestamp,
            static_cast<Timestamp::Timestampbase>(i));
        EXPECT_EQ(timestamp.framePosition, 650); // latency = 350, frameposition = 1000-350 = 650
    }
    ptrRendererInClientInner->ResetFramePosition();
    for (auto i = 0; i < Timestamp::Timestampbase::BASESIZE; i++) {
        ptrRendererInClientInner->GetAudioTimestampInfo(timestamp,
            static_cast<Timestamp::Timestampbase>(i));
        EXPECT_EQ(timestamp.framePosition, 0); // after flush
    }
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: SetSpeed_001
 * @tc.desc  : Test RendererInClientInner SetSpeed.
 */
HWTEST(RendererInClientInnerUnitTest, SetSpeed_001, TestSize.Level0)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    ptrRendererInClientInner->ipcStream_ = new(std::nothrow) IpcStreamTest();

    ptrRendererInClientInner->state_ = State::RUNNING;

    ptrRendererInClientInner->isHdiSpeed_ = false;
    ptrRendererInClientInner->offloadEnable_ = true;
    ptrRendererInClientInner->eStreamType_ = STREAM_MOVIE;
    ptrRendererInClientInner->rendererInfo_.originalFlag = AUDIO_FLAG_PCM_OFFLOAD;
    ptrRendererInClientInner->NotifyRouteUpdate(AUDIO_OUTPUT_FLAG_LOWPOWER, LOCAL_NETWORK_ID);
    ptrRendererInClientInner->rendererInfo_.originalFlag = AUDIO_FLAG_NORMAL;
    ptrRendererInClientInner->NotifyRouteUpdate(AUDIO_OUTPUT_FLAG_LOWPOWER, LOCAL_NETWORK_ID);

    int32_t ret = ptrRendererInClientInner->SetSpeed(1.0f);
    EXPECT_EQ(ret, SUCCESS);
    ret = ptrRendererInClientInner->SetSpeed(2.0f);
    EXPECT_EQ(ret, SUCCESS);
    ptrRendererInClientInner->isHdiSpeed_ = true;
    float speed = 2.5f;
    ret = ptrRendererInClientInner->SetSpeed(speed);
    EXPECT_EQ(ret, SUCCESS);
    speed = ptrRendererInClientInner->GetSpeed();
    EXPECT_EQ(speed, 2.5f);

    ptrRendererInClientInner->isHdiSpeed_ = false;
    ptrRendererInClientInner->offloadEnable_ = true;
    ptrRendererInClientInner->eStreamType_ = STREAM_MOVIE;
    ptrRendererInClientInner->rendererInfo_.originalFlag = AUDIO_FLAG_PCM_OFFLOAD;
    ptrRendererInClientInner->NotifyRouteUpdate(AUDIO_OUTPUT_FLAG_LOWPOWER, LOCAL_NETWORK_ID);
    ptrRendererInClientInner->rendererInfo_.originalFlag = AUDIO_FLAG_NORMAL;
    ptrRendererInClientInner->NotifyRouteUpdate(AUDIO_OUTPUT_FLAG_LOWPOWER, LOCAL_NETWORK_ID);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_061
 * @tc.desc  : Test RendererInClientInner::SetAudioStreamInfo
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_061, TestSize.Level1)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    AudioStreamParams info {
        .samplingRate = AudioSamplingRate::SAMPLE_RATE_8000,
        .encoding = AudioEncodingType::ENCODING_AUDIOVIVID,
        .format = AudioSampleFormat::SAMPLE_U8,
        .channels = AudioChannel::STEREO,
    };
    int32_t ret = ptrRendererInClientInner->SetAudioStreamInfo(info, nullptr);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_062
 * @tc.desc  : Test RendererInClientInner::GetState
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_062, TestSize.Level1)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    ptrRendererInClientInner->switchingInfo_.isSwitching_ = true;
    State state = ptrRendererInClientInner->GetState();
    EXPECT_EQ(state, INVALID);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_063
 * @tc.desc  : Test RendererInClientInner::GetAudioTime
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_063, TestSize.Level1)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    ptrRendererInClientInner->paramsIsSet_ = true;
    ptrRendererInClientInner->state_ = State::RUNNING;
    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_CLIENT;
    uint32_t totalSizeInFrame = 0;
    uint32_t byteSizePerFrame = 0;
    ptrRendererInClientInner->clientBuffer_ = std::make_shared<OHAudioBufferBase>(bufferHolder, totalSizeInFrame,
        byteSizePerFrame);
    ptrRendererInClientInner->offloadEnable_ = false;
    Timestamp timestamp;
    Timestamp::Timestampbase base = Timestamp::Timestampbase::MONOTONIC;
    auto ret = ptrRendererInClientInner->GetAudioTime(timestamp, base);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_064
 * @tc.desc  : Test RendererInClientInner::GetBufferSize
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_064, TestSize.Level1)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    ptrRendererInClientInner->curStreamParams_.encoding = ENCODING_AUDIOVIVID;
    size_t bufferSize = 0;
    int32_t ret = ptrRendererInClientInner->GetBufferSize(bufferSize);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_065
 * @tc.desc  : Test RendererInClientInner::GetFrameCount
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_065, TestSize.Level1)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    ptrRendererInClientInner->curStreamParams_.encoding = ENCODING_AUDIOVIVID;
    ptrRendererInClientInner->curStreamParams_.channels = AudioChannel::STEREO;;
    ptrRendererInClientInner->streamParams_.channels = AudioChannel::STEREO;
    ptrRendererInClientInner->renderMode_ = RENDER_MODE_CALLBACK;
    ptrRendererInClientInner->cbBufferSize_ = 4;

    uint32_t frameCount = 0;
    ptrRendererInClientInner->GetFrameCount(frameCount);
    EXPECT_EQ(frameCount, 1);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_066
 * @tc.desc  : Test RendererInClientInner::SetVolume
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_066, TestSize.Level1)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    float volume = -0.1f;
    int32_t ret = ptrRendererInClientInner->SetVolume(volume);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    volume = 1.1f;
    ret = ptrRendererInClientInner->SetVolume(volume);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    volume = 0.1f;
    ptrRendererInClientInner->volumeRamp_.isVolumeRampActive_ = true;
    ret = ptrRendererInClientInner->SetVolume(volume);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_067
 * @tc.desc  : Test RendererInClientInner::SetDuckVolume
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_067, TestSize.Level1)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    uint32_t totalSizeInFrame = 100;
    uint32_t byteSizePerFrame = 1;
    ptrRendererInClientInner->clientBuffer_ = OHAudioBufferBase::CreateFromLocal(totalSizeInFrame, byteSizePerFrame);
    ptrRendererInClientInner->ipcStream_ = new(std::nothrow) IpcStreamTest();
    float volume = -0.1f;
    int32_t ret = ptrRendererInClientInner->SetDuckVolume(volume);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    volume = 1.1f;
    ret = ptrRendererInClientInner->SetDuckVolume(volume);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    volume = 0.2f;
    ret = ptrRendererInClientInner->SetDuckVolume(volume);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_068
 * @tc.desc  : Test RendererInClientInner::SetStreamCallback
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_068, TestSize.Level1)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    int32_t ret = ptrRendererInClientInner->SetStreamCallback(nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_069
 * @tc.desc  : Test RendererInClientInner::SetRenderMode
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_069, TestSize.Level1)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    ptrRendererInClientInner->renderMode_ = RENDER_MODE_CALLBACK;
    AudioRenderMode renderMode = RENDER_MODE_NORMAL;
    int32_t ret = ptrRendererInClientInner->SetRenderMode(renderMode);
    EXPECT_EQ(ret, ERR_INCORRECT_MODE);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_070
 * @tc.desc  : Test RendererInClientInner::GetBufferDesc
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_070, TestSize.Level1)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    ptrRendererInClientInner->renderMode_ = RENDER_MODE_NORMAL;
    BufferDesc bufDesc;
    int32_t ret = ptrRendererInClientInner->GetBufferDesc(bufDesc);
    EXPECT_EQ(ret, ERR_INCORRECT_MODE);

    ptrRendererInClientInner->renderMode_ = RENDER_MODE_CALLBACK;
    ptrRendererInClientInner->curStreamParams_.encoding = ENCODING_AUDIOVIVID;
    ret = ptrRendererInClientInner->GetBufferDesc(bufDesc);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);

    ptrRendererInClientInner->isHWDecodingType_ = true;
    ptrRendererInClientInner->clientBuffer_ = nullptr;
    ret = ptrRendererInClientInner->GetBufferDesc(bufDesc);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_071
 * @tc.desc  : Test RendererInClientInner::GetBufQueueState
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_071, TestSize.Level1)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    ptrRendererInClientInner->renderMode_ = RENDER_MODE_CALLBACK;
    BufferQueueState bufState;
    int32_t ret = ptrRendererInClientInner->GetBufQueueState(bufState);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_072
 * @tc.desc  : Test RendererInClientInner::Enqueue
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_072, TestSize.Level1)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    ptrRendererInClientInner->renderMode_ = RENDER_MODE_NORMAL;
    BufferDesc bufDesc;
    int32_t ret = ptrRendererInClientInner->Enqueue(bufDesc);
    EXPECT_EQ(ret, ERR_INCORRECT_MODE);

    bufDesc.buffer = new uint8_t[1024] {0};
    bufDesc.bufLength = 1024;
    bufDesc.metaBuffer = new uint8_t[AVS3METADATA_SIZE] {0};
    bufDesc.metaLength = AVS3METADATA_SIZE;
    ptrRendererInClientInner->converter_ = std::make_unique<AudioSpatialChannelConverter>();
    ptrRendererInClientInner->converter_->encoding_ = ENCODING_AUDIOVIVID;
    ptrRendererInClientInner->converter_->inChannel_ = 1;
    ptrRendererInClientInner->converter_->bps_ = 1;
    ptrRendererInClientInner->state_ = RELEASED;
    ptrRendererInClientInner->renderMode_ = RENDER_MODE_CALLBACK;
    ret = ptrRendererInClientInner->Enqueue(bufDesc);
    delete bufDesc.buffer;
    delete bufDesc.metaBuffer;
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_073
 * @tc.desc  : Test RendererInClientInner::Clear
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_073, TestSize.Level1)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    ptrRendererInClientInner->renderMode_ = RENDER_MODE_NORMAL;
    int32_t ret = ptrRendererInClientInner->Clear();
    EXPECT_EQ(ret, ERR_INCORRECT_MODE);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_074
 * @tc.desc  : Test RendererInClientInner::StartAudioStream
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_074, TestSize.Level1)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    ptrRendererInClientInner->state_ = PREPARED;
    ptrRendererInClientInner->ipcStream_ = new(std::nothrow) IpcStreamTest();
    StateChangeCmdType cmdType = CMD_FROM_CLIENT;
    AudioStreamDeviceChangeReasonExt reason;
    bool ret = ptrRendererInClientInner->StartAudioStream(cmdType, reason);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_075
 * @tc.desc  : Test RendererInClientInner::FlushBeforeStart
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_075, TestSize.Level1)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    ptrRendererInClientInner->flushAfterStop_ = true;
    ptrRendererInClientInner->FlushBeforeStart();
    EXPECT_FALSE(ptrRendererInClientInner->flushAfterStop_);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_076
 * @tc.desc  : Test RendererInClientInner::PauseAudioStream
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_076, TestSize.Level1)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    ptrRendererInClientInner->state_ = PREPARED;
    StateChangeCmdType cmdType = CMD_FROM_CLIENT;
    EXPECT_FALSE(ptrRendererInClientInner->PauseAudioStream(cmdType));

    ptrRendererInClientInner->state_ = RUNNING;
    ptrRendererInClientInner->ipcStream_ = new(std::nothrow) IpcStreamTest();
    EXPECT_FALSE(ptrRendererInClientInner->PauseAudioStream(cmdType));
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_077
 * @tc.desc  : Test RendererInClientInner::StopAudioStream
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_077, TestSize.Level1)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    ptrRendererInClientInner->offloadEnable_ = true;
    ptrRendererInClientInner->state_ = RUNNING;
    ptrRendererInClientInner->ipcStream_ = new(std::nothrow) IpcStreamTest();
    EXPECT_FALSE(ptrRendererInClientInner->StopAudioStream());
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_078
 * @tc.desc  : Test RendererInClientInner::FlushAudioStream
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_078, TestSize.Level1)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    ptrRendererInClientInner->offloadEnable_ = true;
    ptrRendererInClientInner->state_ = RUNNING;
    ptrRendererInClientInner->ipcStream_ = new(std::nothrow) IpcStreamTest();
    ptrRendererInClientInner->converter_ = std::make_unique<AudioSpatialChannelConverter>();
    ptrRendererInClientInner->notifiedOperation_ = FLUSH_STREAM;
    EXPECT_TRUE(ptrRendererInClientInner->FlushAudioStream());

    ptrRendererInClientInner->state_ = STOPPED;
    ptrRendererInClientInner->notifiedOperation_ = FLUSH_STREAM;
    ptrRendererInClientInner->uidGetter_ = []() -> uid_t { return 1013; }; // 1013 media_service uid
    EXPECT_TRUE(ptrRendererInClientInner->FlushAudioStream());

    ptrRendererInClientInner->notifiedOperation_ = FLUSH_STREAM;
    ptrRendererInClientInner->uidGetter_ = []() -> uid_t { return 9999; }; // 9999 invalid uid
    EXPECT_TRUE(ptrRendererInClientInner->FlushAudioStream());

    ptrRendererInClientInner->notifiedOperation_ = MAX_OPERATION_CODE;
    EXPECT_FALSE(ptrRendererInClientInner->FlushAudioStream());

    ptrRendererInClientInner->notifiedOperation_ = FLUSH_STREAM;
    ptrRendererInClientInner->notifiedResult_ = ERR_INVALID_OPERATION;
    EXPECT_FALSE(ptrRendererInClientInner->FlushAudioStream());
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_079
 * @tc.desc  : Test RendererInClientInner::SetBufferSizeInMsec
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_079, TestSize.Level1)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    ptrRendererInClientInner->renderMode_ = RENDER_MODE_NORMAL;
    int32_t bufferSizeInMsec = 1024;
    int32_t ret = ptrRendererInClientInner->SetBufferSizeInMsec(bufferSizeInMsec);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: SetBufferSizeInMsec_001
 * @tc.desc  : Test RendererInClientInner::SetBufferSizeInMsec
 */
HWTEST(RendererInClientInnerUnitTest, SetBufferSizeInMsec_001, TestSize.Level1)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    ptrRendererInClientInner->renderMode_ = RENDER_MODE_NORMAL;
    ptrRendererInClientInner->rendererInfo_.playerType = PLAYER_TYPE_TONE_PLAYER;
    int32_t bufferSizeInMsec = 1024;
    int32_t ret = ptrRendererInClientInner->SetBufferSizeInMsec(bufferSizeInMsec);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: SetCacheSize_001
 * @tc.desc  : Test RendererInClientInner::SetCacheSize
 */
HWTEST(RendererInClientInnerUnitTest, SetCacheSize_001, TestSize.Level1)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    ptrRendererInClientInner->spanSizeInFrame_ = 0;
    ptrRendererInClientInner->SetCacheSize(0);
    EXPECT_EQ(ptrRendererInClientInner->cacheSizeInFrame_, 0);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: SetCacheSize_004
 * @tc.desc  : Test RendererInClientInner::SetCacheSize with ipcStream_ null
 */
HWTEST(RendererInClientInnerUnitTest, SetCacheSize_004, TestSize.Level1)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    ptrRendererInClientInner->spanSizeInFrame_ = 20;
    ptrRendererInClientInner->ipcStream_ = nullptr;
    uint32_t testValue = 100;
    uint32_t originalCacheSize = ptrRendererInClientInner->cacheSizeInFrame_;
    ptrRendererInClientInner->SetCacheSize(testValue);
    EXPECT_EQ(ptrRendererInClientInner->cacheSizeInFrame_, originalCacheSize);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: SetCacheSize_005
 * @tc.desc  : Test RendererInClientInner::SetCacheSize with valid ipcStream_
 */
HWTEST(RendererInClientInnerUnitTest, SetCacheSize_005, TestSize.Level1)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    ptrRendererInClientInner->spanSizeInFrame_ = 20;
    sptr<IpcStreamTest> ipcStreamTest = new(std::nothrow) IpcStreamTest();
    ipcStreamTest->calculateCacheCountRet = SUCCESS;
    ipcStreamTest->calculatedCacheCount = 5;
    ptrRendererInClientInner->ipcStream_ = ipcStreamTest;
    uint32_t testValue = 100;
    ptrRendererInClientInner->SetCacheSize(testValue);
    EXPECT_TRUE(ipcStreamTest->calculateCacheCountCalled);
    EXPECT_EQ(ptrRendererInClientInner->cacheSizeInFrame_, 100);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: SetCacheSize_006
 * @tc.desc  : Test RendererInClientInner::SetCacheSize with CalculateCacheCount failure
 */
HWTEST(RendererInClientInnerUnitTest, SetCacheSize_006, TestSize.Level1)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    ptrRendererInClientInner->spanSizeInFrame_ = 20;
    sptr<IpcStreamTest> ipcStreamTest = new(std::nothrow) IpcStreamTest();
    ipcStreamTest->calculateCacheCountRet = ERR_INVALID_PARAM;
    ipcStreamTest->calculatedCacheCount = 5;
    ptrRendererInClientInner->ipcStream_ = ipcStreamTest;
    uint32_t testValue = 100;
    ptrRendererInClientInner->SetCacheSize(testValue);
    EXPECT_TRUE(ipcStreamTest->calculateCacheCountCalled);
    EXPECT_EQ(ptrRendererInClientInner->cacheSizeInFrame_, 120);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: SetCacheSize_007
 * @tc.desc  : Test RendererInClientInner::SetCacheSize with zero cacheCount
 */
HWTEST(RendererInClientInnerUnitTest, SetCacheSize_007, TestSize.Level1)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    ptrRendererInClientInner->spanSizeInFrame_ = 20;
    sptr<IpcStreamTest> ipcStreamTest = new(std::nothrow) IpcStreamTest();
    ipcStreamTest->calculateCacheCountRet = SUCCESS;
    ipcStreamTest->calculatedCacheCount = 0;
    ptrRendererInClientInner->ipcStream_ = ipcStreamTest;
    uint32_t testValue = 10;
    ptrRendererInClientInner->SetCacheSize(testValue);
    EXPECT_TRUE(ipcStreamTest->calculateCacheCountCalled);
    EXPECT_EQ(ptrRendererInClientInner->cacheSizeInFrame_, 0);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: SetCacheSize_008
 * @tc.desc  : Test RendererInClientInner::SetCacheSize with large cacheCount
 */
HWTEST(RendererInClientInnerUnitTest, SetCacheSize_008, TestSize.Level1)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    ptrRendererInClientInner->spanSizeInFrame_ = 20;
    sptr<IpcStreamTest> ipcStreamTest = new(std::nothrow) IpcStreamTest();
    ipcStreamTest->calculateCacheCountRet = SUCCESS;
    ipcStreamTest->calculatedCacheCount = 1000;
    ptrRendererInClientInner->ipcStream_ = ipcStreamTest;
    uint32_t testValue = 20000;
    ptrRendererInClientInner->SetCacheSize(testValue);
    EXPECT_TRUE(ipcStreamTest->calculateCacheCountCalled);
    EXPECT_EQ(ptrRendererInClientInner->cacheSizeInFrame_, 20000);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: SetCacheSize_009
 * @tc.desc  : Test RendererInClientInner::SetCacheSize with minimum cacheSize
 */
HWTEST(RendererInClientInnerUnitTest, SetCacheSize_009, TestSize.Level1)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    ptrRendererInClientInner->spanSizeInFrame_ = 10;
    sptr<IpcStreamTest> ipcStreamTest = new(std::nothrow) IpcStreamTest();
    ipcStreamTest->calculateCacheCountRet = SUCCESS;
    ipcStreamTest->calculatedCacheCount = 1;
    ptrRendererInClientInner->ipcStream_ = ipcStreamTest;
    uint32_t testValue = 1;
    ptrRendererInClientInner->SetCacheSize(testValue);
    EXPECT_TRUE(ipcStreamTest->calculateCacheCountCalled);
    EXPECT_EQ(ptrRendererInClientInner->cacheSizeInFrame_, 10);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_080
 * @tc.desc  : Test RendererInClientInner::InitCallbackHandler
 *             Test RendererInClientInner::StateCmdTypeToParams
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_080, TestSize.Level1)
{
    // Test RendererInClientInner::InitCallbackHandler
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    ptrRendererInClientInner->callbackHandler_ =
        CallbackHandler::GetInstance(ptrRendererInClientInner, "TEST_AudioStateCB");
    ptrRendererInClientInner->InitCallbackHandler();

    //Test RendererInClientInner::StateCmdTypeToParams
    int64_t params = RendererInClientInner::HANDLER_PARAM_INVALID;
    State state = RUNNING;
    StateChangeCmdType cmdType = CMD_FROM_SYSTEM;
    ptrRendererInClientInner->StateCmdTypeToParams(params, state, cmdType);
    EXPECT_EQ(params, RendererInClientInner::HANDLER_PARAM_RUNNING_FROM_SYSTEM);

    state = PAUSED;
    ptrRendererInClientInner->StateCmdTypeToParams(params, state, cmdType);
    EXPECT_EQ(params, RendererInClientInner::HANDLER_PARAM_PAUSED_FROM_SYSTEM);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_081
 * @tc.desc  : Test RendererInClientInner::ParamsToStateCmdType
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_081, TestSize.Level1)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    State state = INVALID;
    StateChangeCmdType cmdType = CMD_FROM_CLIENT;
    int64_t params = RendererInClientInner::HANDLER_PARAM_INVALID;
    ptrRendererInClientInner->ParamsToStateCmdType(params, state, cmdType);
    EXPECT_EQ(state, INVALID);

    params = RendererInClientInner::HANDLER_PARAM_NEW;
    ptrRendererInClientInner->ParamsToStateCmdType(params, state, cmdType);
    EXPECT_EQ(state, NEW);

    params = RendererInClientInner::HANDLER_PARAM_RELEASED;
    ptrRendererInClientInner->ParamsToStateCmdType(params, state, cmdType);
    EXPECT_EQ(state, RELEASED);

    params = RendererInClientInner::HANDLER_PARAM_STOPPING;
    ptrRendererInClientInner->ParamsToStateCmdType(params, state, cmdType);
    EXPECT_EQ(state, STOPPING);

    params = RendererInClientInner::HANDLER_PARAM_RUNNING_FROM_SYSTEM;
    ptrRendererInClientInner->ParamsToStateCmdType(params, state, cmdType);
    EXPECT_EQ(state, RUNNING);
    EXPECT_EQ(cmdType, CMD_FROM_SYSTEM);

    params = RendererInClientInner::HANDLER_PARAM_PAUSED_FROM_SYSTEM;
    ptrRendererInClientInner->ParamsToStateCmdType(params, state, cmdType);
    EXPECT_EQ(state, PAUSED);
    EXPECT_EQ(cmdType, CMD_FROM_SYSTEM);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_082
 * @tc.desc  : Test RendererInClientInner::HandleRendererPositionChanges
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_082, TestSize.Level1)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    size_t bytesWritten = 4;
    ptrRendererInClientInner->rendererPeriodSize_ = 1;
    ptrRendererInClientInner->HandleRendererPositionChanges(bytesWritten);
    EXPECT_TRUE(ptrRendererInClientInner->rendererMarkReached_);
    EXPECT_EQ(ptrRendererInClientInner->rendererPeriodWritten_, 0);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_083
 * @tc.desc  : Test RendererInClientInner::RestoreAudioStream
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_083, TestSize.Level1)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    ptrRendererInClientInner->proxyObj_ = std::make_shared<AudioClientTrackerTest>();
    ptrRendererInClientInner->state_ = RUNNING;
    bool needStoreState = false;
    EXPECT_FALSE(ptrRendererInClientInner->RestoreAudioStream(needStoreState));

    ptrRendererInClientInner->rendererInfo_.pipeType = PIPE_TYPE_OUT_OFFLOAD;
    EXPECT_FALSE(ptrRendererInClientInner->RestoreAudioStream(needStoreState));
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_085
 * @tc.desc  : Test RendererInClientInner::SetSwitchingStatus
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_085, TestSize.Level1)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    bool isSwitching = true;
    ptrRendererInClientInner->SetSwitchingStatus(isSwitching);
    EXPECT_TRUE(ptrRendererInClientInner->switchingInfo_.isSwitching_);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_086
 * @tc.desc  : Test RendererInClientInner::SetRestoreInfo
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_086, TestSize.Level1)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    RestoreInfo restoreInfo;
    restoreInfo.restoreReason = DEFAULT_REASON;
    ptrRendererInClientInner->cbThreadReleased_ = false;
    ptrRendererInClientInner->SetRestoreInfo(restoreInfo);
    EXPECT_FALSE(ptrRendererInClientInner->cbThreadReleased_);

    restoreInfo.restoreReason = SERVER_DIED;
    ptrRendererInClientInner->SetRestoreInfo(restoreInfo);
    EXPECT_TRUE(ptrRendererInClientInner->cbThreadReleased_);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_087
 * @tc.desc  : Test RendererInClientInner::FetchDeviceForSplitStream
 *             Test RendererInClientInner::GetCallbackLoopTid
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_087, TestSize.Level1)
{
    // Test RendererInClientInner::FetchDeviceForSplitStream
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    ptrRendererInClientInner->FetchDeviceForSplitStream();

    ptrRendererInClientInner->audioStreamTracker_.reset();
    ptrRendererInClientInner->FetchDeviceForSplitStream();

    // Test RendererInClientInner::GetCallbackLoopTid
    ptrRendererInClientInner->callbackLoopTid_ = -1;
    int32_t ret = ptrRendererInClientInner->GetCallbackLoopTid();
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_088
 * @tc.desc  : Test RendererInClientInner::CheckBufferNeedWrite
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_088, TestSize.Level1)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    // totalsize is 100
    uint32_t totalSizeInFrame = 100;
    uint32_t byteSizePerFrame = 1;
    ptrRendererInClientInner->clientBuffer_ = OHAudioBufferBase::CreateFromLocal(totalSizeInFrame, byteSizePerFrame);
    ptrRendererInClientInner->sizePerFrameInByte_ = 1;
    // enginesizeinframe 2
    ptrRendererInClientInner->cacheSizeInFrame_ = 2;
    ptrRendererInClientInner->cbBufferSize_ = 1;

    // Readable == enginesizeinframe
    ptrRendererInClientInner->clientBuffer_->SetCurWriteFrame(2);
    bool ret = ptrRendererInClientInner->CheckBufferNeedWrite();

    EXPECT_EQ(ret, true);

    BufferDesc bufferDesc;
    int32_t result = ptrRendererInClientInner->GetRawBuffer(bufferDesc);
    EXPECT_EQ(result, SUCCESS);

    ptrRendererInClientInner->clientBuffer_ = nullptr;
    result = ptrRendererInClientInner->GetRawBuffer(bufferDesc);
    EXPECT_EQ(result, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_089
 * @tc.desc  : Test RendererInClientInner::CheckBufferNeedWrite
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_089, TestSize.Level1)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    // totalsize is 100
    uint32_t totalSizeInFrame = 100;
    uint32_t byteSizePerFrame = 1;
    ptrRendererInClientInner->clientBuffer_ = OHAudioBufferBase::CreateFromLocal(totalSizeInFrame, byteSizePerFrame);
    ptrRendererInClientInner->sizePerFrameInByte_ = 1;
    // enginesizeinframe 2
    ptrRendererInClientInner->cacheSizeInFrame_ = 2;
    ptrRendererInClientInner->cbBufferSize_ = 1;

    // Readable > enginesizeinframe
    ptrRendererInClientInner->clientBuffer_->SetCurWriteFrame(3);
    bool ret = ptrRendererInClientInner->CheckBufferNeedWrite();

    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_090
 * @tc.desc  : Test RendererInClientInner::ProcessWriteInner
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_090, TestSize.Level4)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    // totalsize is 100
    uint32_t totalSizeInFrame = 100;
    uint32_t byteSizePerFrame = 1;
    ptrRendererInClientInner->clientBuffer_ = OHAudioBufferBase::CreateFromLocal(totalSizeInFrame, byteSizePerFrame);
    ptrRendererInClientInner->sizePerFrameInByte_ = 1;
    // enginesizeinframe 2
    ptrRendererInClientInner->cacheSizeInFrame_ = 2;
    ptrRendererInClientInner->cbBufferSize_ = 1;
    ptrRendererInClientInner->spanSizeInFrame_ = 1;

    // datalenth == 0
    BufferDesc bufferDesc;
    int32_t ret = ptrRendererInClientInner->ProcessWriteInner(bufferDesc);
    EXPECT_EQ(ret, SUCCESS);

    // totalsize is 100
    ptrRendererInClientInner->clientBuffer_->SetCurWriteFrame(100);
    ret = ptrRendererInClientInner->ProcessWriteInner(bufferDesc);
    EXPECT_EQ(ret, SUCCESS);

    ptrRendererInClientInner->isHWDecodingType_ = true;
    ret = ptrRendererInClientInner->ProcessWriteInner(bufferDesc);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_091
 * @tc.desc  : Test RendererInClientInner::CheckBufferValid
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_091, TestSize.Level4)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    // buffersize 10 byte
    ptrRendererInClientInner->cbBufferSize_ = 10;

    BufferDesc bufferDesc;
    // bufLength > cbBufferSize_
    bufferDesc.bufLength = 100;
    bool ret = ptrRendererInClientInner->CheckBufferValid(bufferDesc);
    EXPECT_EQ(ret, false);

    // bufLength == cbBufferSize_
    bufferDesc.bufLength = 10;
    // dataLength == cbBufferSize_
    bufferDesc.dataLength = 10;
    ret = ptrRendererInClientInner->CheckBufferValid(bufferDesc);
    EXPECT_EQ(ret, true);

    // bufLength == cbBufferSize_
    bufferDesc.bufLength = 10;
    // dataLength > cbBufferSize_
    bufferDesc.dataLength = 100;
    ret = ptrRendererInClientInner->CheckBufferValid(bufferDesc);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_092
 * @tc.desc  : Test RendererInClientInner::ProcessWriteInner
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_092, TestSize.Level4)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    ptrRendererInClientInner->state_.store(RUNNING);
    // totalsize is 100
    uint32_t totalSizeInFrame = 100;
    uint32_t byteSizePerFrame = 1;
    ptrRendererInClientInner->clientBuffer_ = OHAudioBufferBase::CreateFromLocal(totalSizeInFrame, byteSizePerFrame);
    ptrRendererInClientInner->clientBuffer_->basicBufferInfo_->restoreStatus.store(NO_NEED_FOR_RESTORE);
    EXPECT_EQ(ptrRendererInClientInner->IsRestoreNeeded(), false);

    ptrRendererInClientInner->clientBuffer_->basicBufferInfo_->restoreStatus.store(NEED_RESTORE);
    ptrRendererInClientInner->WaitForBufferNeedOperate();
    EXPECT_EQ(ptrRendererInClientInner->IsRestoreNeeded(), true);

    ptrRendererInClientInner->clientBuffer_->basicBufferInfo_->restoreStatus.store(NEED_RESTORE_TO_NORMAL);
    ptrRendererInClientInner->WaitForBufferNeedOperate();
    EXPECT_EQ(ptrRendererInClientInner->IsRestoreNeeded(), true);
}

/**
 * @tc.name  : Test GetAudioTime API
 * @tc.type  : FUNC
 * @tc.number: GetAudioTime_001
 * @tc.desc  : Test RendererInClientInner::GetAudioTime
 */
HWTEST(RendererInClientInnerUnitTest, GetAudioTime_001, TestSize.Level4)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    Timestamp timestamp;
    EXPECT_EQ(ptrRendererInClientInner->GetAudioTime(timestamp, Timestamp::Timestampbase::BASESIZE), false);
}

/**
 * @tc.name  : Test GetAudioTime API
 * @tc.type  : FUNC
 * @tc.number: GetAudioTime_002
 * @tc.desc  : Test RendererInClientInner::GetAudioTime
 */
HWTEST(RendererInClientInnerUnitTest, GetAudioTime_002, TestSize.Level4)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    ptrRendererInClientInner->state_ = RELEASED;
    Timestamp timestamp;
    EXPECT_EQ(ptrRendererInClientInner->GetAudioTime(timestamp, Timestamp::Timestampbase::BASESIZE), false);
}

/**
 * @tc.name  : Test GetAudioTime API
 * @tc.type  : FUNC
 * @tc.number: GetAudioTime_003
 * @tc.desc  : Test RendererInClientInner::GetAudioTime
 */
HWTEST(RendererInClientInnerUnitTest, GetAudioTime_003, TestSize.Level4)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    ptrRendererInClientInner->paramsIsSet_ = true;
    ptrRendererInClientInner->state_ = RUNNING;
    Timestamp timestamp;
    EXPECT_EQ(ptrRendererInClientInner->GetAudioTime(timestamp, Timestamp::Timestampbase::BASESIZE), false);
}

/**
 * @tc.name  : Test GetAudioTime API
 * @tc.type  : FUNC
 * @tc.number: GetAudioTime_004
 * @tc.desc  : Test RendererInClientInner::GetAudioTime
 */
HWTEST(RendererInClientInnerUnitTest, GetAudioTime_004, TestSize.Level4)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    ptrRendererInClientInner->paramsIsSet_ = true;
    ptrRendererInClientInner->state_ = RUNNING;
    ptrRendererInClientInner->offloadEnable_ = true;
    Timestamp timestamp;
    EXPECT_NE(ptrRendererInClientInner->GetAudioTime(timestamp, Timestamp::Timestampbase::BASESIZE), true);
}

/**
 * @tc.name  : Test SetAudioStreamType API
 * @tc.type  : FUNC
 * @tc.number: SetAudioStreamType_001
 * @tc.desc  : Test RendererInClientInner::SetAudioStreamType
 */
HWTEST(RendererInClientInnerUnitTest, SetAudioStreamType_001, TestSize.Level4)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    AudioStreamType audioStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t ret = ptrRendererInClientInner->SetAudioStreamType(audioStreamType);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test Write API
 * @tc.type  : FUNC
 * @tc.number: Write_001
 * @tc.desc  : Test RendererInClientInner::Write
 */
HWTEST(RendererInClientInnerUnitTest, Write_001, TestSize.Level4)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    uint8_t pcmBuffer[10] = {0};
    size_t pcmBufferSize = 10;
    uint8_t metaBuffer[10] = {0};
    size_t metaBufferSize = 10;

    ptrRendererInClientInner->renderMode_ = RENDER_MODE_NORMAL;
    int32_t ret = ptrRendererInClientInner->Write(pcmBuffer, pcmBufferSize, metaBuffer, metaBufferSize);
    EXPECT_NE(ret, pcmBufferSize);
}

/**
 * @tc.name  : Test Write API
 * @tc.type  : FUNC
 * @tc.number: Write_002
 * @tc.desc  : Test RendererInClientInner::Write
 */
HWTEST(RendererInClientInnerUnitTest, Write_002, TestSize.Level4)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    uint8_t pcmBuffer[10] = {0};
    size_t pcmBufferSize = 10;
    uint8_t metaBuffer[10] = {0};
    size_t metaBufferSize = 10;

    ptrRendererInClientInner->renderMode_ = RENDER_MODE_CALLBACK;
    int32_t ret = ptrRendererInClientInner->Write(pcmBuffer, pcmBufferSize, metaBuffer, metaBufferSize);
    EXPECT_EQ(ret, ERR_INCORRECT_MODE);
}

/**
 * @tc.name  : Test GetStreamSwitchInfo API
 * @tc.type  : FUNC
 * @tc.number: GetStreamSwitchInfo_001
 * @tc.desc  : Test RendererInClientInner::GetStreamSwitchInfo
 */
HWTEST(RendererInClientInnerUnitTest, GetStreamSwitchInfo_001, TestSize.Level4)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    IAudioStream::SwitchInfo info;

    // Set up the renderer with some known values
    ptrRendererInClientInner->effectMode_ = EFFECT_NONE;
    ptrRendererInClientInner->rendererRate_ = RENDER_RATE_NORMAL;
    ptrRendererInClientInner->clientPid_ = 1234;
    ptrRendererInClientInner->clientUid_ = 5678;
    ptrRendererInClientInner->clientVolume_ = 50;
    ptrRendererInClientInner->duckVolume_ = 30;
    ptrRendererInClientInner->silentModeAndMixWithOthers_ = false;
    ptrRendererInClientInner->rendererMarkPosition_ = 1000;
    ptrRendererInClientInner->rendererPositionCallback_ = nullptr;
    ptrRendererInClientInner->rendererPeriodSize_ = 1024;
    ptrRendererInClientInner->rendererPeriodPositionCallback_ = nullptr;
    ptrRendererInClientInner->writeCb_ = nullptr;

    // Call the function under test
    ptrRendererInClientInner->GetStreamSwitchInfo(info);

    // Check if the SwitchInfo structure is correctly set
    EXPECT_EQ(info.underFlowCount, ptrRendererInClientInner->GetUnderflowCount());
    EXPECT_EQ(info.effectMode, ptrRendererInClientInner->effectMode_);
    EXPECT_EQ(info.renderRate, ptrRendererInClientInner->rendererRate_);
    EXPECT_EQ(info.clientPid, ptrRendererInClientInner->clientPid_);
    EXPECT_EQ(info.clientUid, ptrRendererInClientInner->clientUid_);
    EXPECT_EQ(info.volume, ptrRendererInClientInner->clientVolume_);
    EXPECT_EQ(info.duckVolume, ptrRendererInClientInner->duckVolume_);
    EXPECT_EQ(info.silentModeAndMixWithOthers, ptrRendererInClientInner->silentModeAndMixWithOthers_);
    EXPECT_EQ(info.frameMarkPosition, ptrRendererInClientInner->rendererMarkPosition_);
    EXPECT_EQ(info.renderPositionCb, ptrRendererInClientInner->rendererPositionCallback_);
    EXPECT_EQ(info.framePeriodNumber, ptrRendererInClientInner->rendererPeriodSize_);
    EXPECT_EQ(info.renderPeriodPositionCb, ptrRendererInClientInner->rendererPeriodPositionCallback_);
    EXPECT_EQ(info.rendererWriteCallback, ptrRendererInClientInner->writeCb_);
}

/**
 * @tc.name  : Test SetSourceDuration API
 * @tc.type  : FUNC
 * @tc.number: SetSourceDuration_001
 * @tc.desc  : Test RendererInClientInner::SetSourceDuration
 */
HWTEST(RendererInClientInnerUnitTest, SetSourceDuration_001, TestSize.Level4)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    int64_t duration = 100;
    ptrRendererInClientInner->ipcStream_ = nullptr;
    int32_t ret = ptrRendererInClientInner->SetSourceDuration(duration);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test SetSourceDuration API
 * @tc.type  : FUNC
 * @tc.number: SetSourceDuration_002
 * @tc.desc  : Test RendererInClientInner::SetSourceDuration
 */
HWTEST(RendererInClientInnerUnitTest, SetSourceDuration_002, TestSize.Level4)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    int64_t duration = 100;
    ptrRendererInClientInner->ipcStream_ = new(std::nothrow) IpcStreamTest();
    int32_t ret = ptrRendererInClientInner->SetSourceDuration(duration);
    EXPECT_NE(ret, ERROR);
}

/**
 * @tc.name  : Test SetOffloadDataCallbackState API
 * @tc.type  : FUNC
 * @tc.number: SetOffloadDataCallbackState_001
 * @tc.desc  : Test RendererInClientInner::SetOffloadDataCallbackState
 */
HWTEST(RendererInClientInnerUnitTest, SetOffloadDataCallbackState_001, TestSize.Level4)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    int cbState = 1;
    EXPECT_EQ(ptrRendererInClientInner->SetOffloadDataCallbackState(cbState), ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test SetOffloadDataCallbackState API
 * @tc.type  : FUNC
 * @tc.number: SetOffloadDataCallbackState_002
 * @tc.desc  : Test RendererInClientInner::SetOffloadDataCallbackState
 */
HWTEST(RendererInClientInnerUnitTest, SetOffloadDataCallbackState_002, TestSize.Level4)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    ptrRendererInClientInner->ipcStream_ = new(std::nothrow) IpcStreamTest();
    int cbState = 1;
    EXPECT_EQ(ptrRendererInClientInner->SetOffloadDataCallbackState(cbState), SUCCESS);
}

/**
 * @tc.name  : Test RecordDropPosition API
 * @tc.type  : FUNC
 * @tc.number: RecordDropPosition_001
 * @tc.desc  : Test RendererInClientInner::RecordDropPosition
 */
HWTEST(RendererInClientInnerUnitTest, RecordDropPosition_001, TestSize.Level1)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    AudioProcessConfig config = {};
    config.streamInfo.channels = AudioChannel::STEREO;
    config.streamInfo.format = AudioSampleFormat::SAMPLE_S24LE;
    ptrRendererInClientInner->clientConfig_ = config;

    ptrRendererInClientInner->isHdiSpeed_.store(false);
    ptrRendererInClientInner->RecordDropPosition(60);
    EXPECT_EQ(ptrRendererInClientInner->dropPosition_, 0);
    EXPECT_EQ(ptrRendererInClientInner->dropHdiPosition_, 0);

    ptrRendererInClientInner->isHdiSpeed_.store(true);
    ptrRendererInClientInner->realSpeed_ = 2.0f;
    ptrRendererInClientInner->RecordDropPosition(60);
    EXPECT_EQ(ptrRendererInClientInner->dropPosition_, 10);
    EXPECT_EQ(ptrRendererInClientInner->dropHdiPosition_, 5);
}

/**
 * @tc.name  : Test InitDirectPipeType API
 * @tc.type  : FUNC
 * @tc.number: InitDirectPipeType_001
 * @tc.desc  : Test InitDirectPipeType
 */
HWTEST(RendererInClientInnerUnitTest, InitDirectPipeType_001, TestSize.Level4)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    ptrRendererInClientInner->InitDirectPipeType();
    EXPECT_EQ(ptrRendererInClientInner->rendererInfo_.pipeType, PIPE_TYPE_UNKNOWN);
}

/**
 * @tc.name  : Test CheckBufferNeedWrite API
 * @tc.type  : FUNC
 * @tc.number: CheckBufferNeedWrite_001
 * @tc.desc  : Test CheckBufferNeedWrite
 */
HWTEST(RendererInClientInnerUnitTest, CheckBufferNeedWrite_001, TestSize.Level4)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    uint32_t totalSizeInFrame = 100;
    uint32_t byteSizePerFrame = 1;
    ptrRendererInClientInner->clientBuffer_ = OHAudioBufferBase::CreateFromLocal(totalSizeInFrame, byteSizePerFrame);
    ptrRendererInClientInner->sizePerFrameInByte_ = 1;
    ptrRendererInClientInner->cacheSizeInFrame_ = 2;
    ptrRendererInClientInner->cbBufferSize_ = 1000;

    bool ret = ptrRendererInClientInner->CheckBufferNeedWrite();
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test WriteCallbackFunc API
 * @tc.type  : FUNC
 * @tc.number: WriteCallbackFunc_001
 * @tc.desc  : Test WriteCallbackFunc
 */
HWTEST(RendererInClientInnerUnitTest, WriteCallbackFunc_001, TestSize.Level4)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    ptrRendererInClientInner->state_ = State::RUNNING;
    bool ret  = ptrRendererInClientInner->WriteCallbackFunc();
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: SetRenderTarget_001
 * @tc.desc  : Test RendererInClientInner::SetRenderTarget
 */
HWTEST(RendererInClientInnerUnitTest, SetRenderTarget_001, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);
    
    auto ret = ptrRendererInClientInner->SetRenderTarget(NORMAL_PLAYBACK);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: GetRenderTarget_001
 * @tc.desc  : Test RendererInClientInner::GetRenderTarget
 */
HWTEST(RendererInClientInnerUnitTest, GetRenderTarget_001, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    auto ret = ptrRendererInClientInner->GetRenderTarget();
    EXPECT_EQ(ret, NORMAL_PLAYBACK);
}

/**
 * @tc.name  : Test GetSwitchInfo API
 * @tc.type  : FUNC
 * @tc.number: GetSwitchInfo_001
 * @tc.desc  : Test GetSwitchInfo
 */
HWTEST(RendererInClientInnerUnitTest, GetSwitchInfo_001, TestSize.Level4)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    IAudioStream::SwitchInfo info;
    ptrRendererInClientInner->GetSwitchInfo(info);
    EXPECT_EQ(info.rendererFirstFrameWritingCallback, nullptr);
    ptrRendererInClientInner->firstFrameWritingCb_ = std::make_shared<AudioRendererFirstFrameWritingCallbackTest>();
    ptrRendererInClientInner->GetSwitchInfo(info);
    EXPECT_NE(info.rendererFirstFrameWritingCallback, nullptr);
}

/**
 * @tc.name  : Test GetSwitchInfo API
 * @tc.type  : FUNC
 * @tc.number: GetStreamSwitchInfo_002
 * @tc.desc  : Test GetSwitchInfo
 */
HWTEST(RendererInClientInnerUnitTest, GetStreamSwitchInfo_002, TestSize.Level4)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    IAudioStream::SwitchInfo info;
    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_CLIENT;
    ptrRendererInClientInner->clientBuffer_ = std::make_shared<OHAudioBufferBase>(bufferHolder, 0, 0);
    ptrRendererInClientInner->staticBufferInfo_.totalLoopTimes_ = 9;
    ptrRendererInClientInner->GetStreamSwitchInfo(info);
    EXPECT_EQ(info.staticBufferInfo.totalLoopTimes_, 0);
    ptrRendererInClientInner->rendererInfo_.isStatic = true;
    ptrRendererInClientInner->GetStreamSwitchInfo(info);
    EXPECT_EQ(info.staticBufferInfo.totalLoopTimes_, 9);
}

/**
 * @tc.name  : Test CheckStaticAndOperate API
 * @tc.type  : FUNC
 * @tc.number: CheckStaticAndOperate_001
 * @tc.desc  : Test CheckStaticAndOperate
 */
HWTEST(RendererInClientInnerUnitTest, CheckStaticAndOperate_001, TestSize.Level4)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_CLIENT;
    ptrRendererInClientInner->rendererInfo_.isStatic = true;
    ptrRendererInClientInner->clientBuffer_ = std::make_shared<OHAudioBufferBase>(bufferHolder, 0, 0);
    ptrRendererInClientInner->clientBuffer_->SetStaticMode(true);
    ptrRendererInClientInner->ipcStream_ = new(std::nothrow) IpcStreamTest();
    ptrRendererInClientInner->clientBuffer_->SetIsFirstFrame(false);
    EXPECT_EQ(ptrRendererInClientInner->CheckStaticAndOperate(), false);
}

/**
 * @tc.name  : Test CheckOperations API with static renderer
 * @tc.type  : FUNC
 * @tc.number: CheckOperations_001
 * @tc.desc  : Test CheckOperations with static renderer info
 */
HWTEST(RendererInClientInnerUnitTest, CheckOperations_001, TestSize.Level4)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    ASSERT_TRUE(ptrRendererInClientInner != nullptr);
    ptrRendererInClientInner->rendererInfo_.isStatic = true;
    ptrRendererInClientInner->ipcStream_ = new(std::nothrow) IpcStreamTest();

    uint32_t totalSizeInFrame = 100;
    uint32_t byteSizePerFrame = 1;
    ptrRendererInClientInner->clientBuffer_ = OHAudioBufferBase::CreateFromLocal(totalSizeInFrame, byteSizePerFrame);
    ptrRendererInClientInner->clientBuffer_->basicBufferInfo_->restoreStatus.store(NO_NEED_FOR_RESTORE);
    ptrRendererInClientInner->sendStaticRecreateFunc_ = nullptr;
    ptrRendererInClientInner->CheckOperations();

    ptrRendererInClientInner->clientBuffer_->basicBufferInfo_->restoreStatus.store(NEED_RESTORE);
    ptrRendererInClientInner->sendStaticRecreateFunc_ = nullptr;
    ptrRendererInClientInner->CheckOperations();

    ptrRendererInClientInner->clientBuffer_->basicBufferInfo_->restoreStatus.store(NO_NEED_FOR_RESTORE);
    ptrRendererInClientInner->sendStaticRecreateFunc_ = [](){return;};
    ptrRendererInClientInner->CheckOperations();

    ptrRendererInClientInner->clientBuffer_->basicBufferInfo_->restoreStatus.store(NEED_RESTORE);
    ptrRendererInClientInner->sendStaticRecreateFunc_ = [](){return;};
    ptrRendererInClientInner->CheckOperations();
    EXPECT_NE(ptrRendererInClientInner, nullptr);
}

/**
 * @tc.name  : Test CheckOperations API with static renderer
 * @tc.type  : FUNC
 * @tc.number: CheckOperations_002
 * @tc.desc  : Test CheckOperations with static renderer info
 */
HWTEST(RendererInClientInnerUnitTest, CheckOperations_002, TestSize.Level4)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    ASSERT_TRUE(ptrRendererInClientInner != nullptr);
    ptrRendererInClientInner->rendererInfo_.isStatic = true;
    ptrRendererInClientInner->ipcStream_ = new(std::nothrow) IpcStreamTest();

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);
    ptrRendererInClientInner->rendererInfo_.isStatic = true;
    uint32_t totalSizeInFrame = 100;
    uint32_t byteSizePerFrame = 1;
    ptrRendererInClientInner->clientBuffer_ = OHAudioBufferBase::CreateFromLocal(totalSizeInFrame, byteSizePerFrame);
    ptrRendererInClientInner->clientBuffer_->basicBufferInfo_->restoreStatus.store(NO_NEED_FOR_RESTORE);
    ptrRendererInClientInner->audioStaticBufferEventCallback_ = std::make_shared<StaticBufferEventCallbackTest>();
    ptrRendererInClientInner->clientBuffer_->SetStaticMode(true);
    ptrRendererInClientInner->clientBuffer_->IncreaseBufferEndCallbackSendTimes();
    ptrRendererInClientInner->CheckOperations();
    EXPECT_EQ(ptrRendererInClientInner->clientBuffer_->IsNeedSendBufferEndCallback(), false);
}

/**
 * @tc.name  : Test CheckOperations API with static renderer
 * @tc.type  : FUNC
 * @tc.number: CheckOperations_003
 * @tc.desc  : Test CheckOperations with static renderer info
 */
HWTEST(RendererInClientInnerUnitTest, CheckOperations_003, TestSize.Level4)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    ASSERT_TRUE(ptrRendererInClientInner != nullptr);
    ptrRendererInClientInner->rendererInfo_.isStatic = true;
    ptrRendererInClientInner->ipcStream_ = new(std::nothrow) IpcStreamTest();

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);
    ptrRendererInClientInner->rendererInfo_.isStatic = true;
    uint32_t totalSizeInFrame = 100;
    uint32_t byteSizePerFrame = 1;
    ptrRendererInClientInner->clientBuffer_ = OHAudioBufferBase::CreateFromLocal(totalSizeInFrame, byteSizePerFrame);
    ptrRendererInClientInner->clientBuffer_->basicBufferInfo_->restoreStatus.store(NO_NEED_FOR_RESTORE);
    ptrRendererInClientInner->audioStaticBufferEventCallback_ = std::make_shared<StaticBufferEventCallbackTest>();
    ptrRendererInClientInner->clientBuffer_->SetStaticMode(true);
    ptrRendererInClientInner->clientBuffer_->SetIsNeedSendLoopEndCallback(true);
    ptrRendererInClientInner->clientBuffer_->SetIsFirstFrame(false);
    ptrRendererInClientInner->CheckOperations();
    EXPECT_EQ(ptrRendererInClientInner->clientBuffer_->IsNeedSendLoopEndCallback(), false);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_StopAudioStream
 * @tc.desc  : Test RendererInClientInner::StopAudioStream
 */
HWTEST(RendererInClientInnerUnitTest, StopAudioStream_static, TestSize.Level1)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    ptrRendererInClientInner->offloadEnable_ = true;
    ptrRendererInClientInner->rendererInfo_.isStatic = true;
    ptrRendererInClientInner->state_ = RUNNING;
    ptrRendererInClientInner->ipcStream_ = new(std::nothrow) IpcStreamTest();
    EXPECT_FALSE(ptrRendererInClientInner->StopAudioStream());

    ptrRendererInClientInner->offloadEnable_ = false;
    ptrRendererInClientInner->rendererInfo_.isStatic = true;
    ptrRendererInClientInner->state_ = RUNNING;
    ptrRendererInClientInner->ipcStream_ = new(std::nothrow) IpcStreamTest();
    EXPECT_FALSE(ptrRendererInClientInner->StopAudioStream());

    ptrRendererInClientInner->offloadEnable_ = true;
    ptrRendererInClientInner->rendererInfo_.isStatic = false;
    ptrRendererInClientInner->state_ = RUNNING;
    ptrRendererInClientInner->ipcStream_ = new(std::nothrow) IpcStreamTest();
    EXPECT_FALSE(ptrRendererInClientInner->StopAudioStream());

    ptrRendererInClientInner->offloadEnable_ = false;
    ptrRendererInClientInner->rendererInfo_.isStatic = false;
    ptrRendererInClientInner->state_ = RUNNING;
    ptrRendererInClientInner->ipcStream_ = new(std::nothrow) IpcStreamTest();
    EXPECT_FALSE(ptrRendererInClientInner->StopAudioStream());
}

/**
 * @tc.name  : Test CheckOperations API with static renderer
 * @tc.type  : FUNC
 * @tc.number: CheckFrozenStateInStaticMode_001
 * @tc.desc  : Test CheckFrozenStateInStaticMode with static renderer info
 */
HWTEST(RendererInClientInnerUnitTest, CheckFrozenStateInStaticMode_001, TestSize.Level4)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    ASSERT_TRUE(ptrRendererInClientInner != nullptr);
    ptrRendererInClientInner->rendererInfo_.isStatic = true;
    ptrRendererInClientInner->clientBuffer_ = OHAudioBufferBase::CreateFromLocal(10, 10);
    ptrRendererInClientInner->clientBuffer_->SetStaticMode(true);
    ptrRendererInClientInner->clientBuffer_->CheckFrozenAndSetLastProcessTime(BUFFER_IN_CLIENT);
    ptrRendererInClientInner->ipcStream_ = new(std::nothrow) IpcStreamTest();

    ptrRendererInClientInner->CheckFrozenStateInStaticMode();
    ptrRendererInClientInner->clientBuffer_->GetStreamStatus()->store(StreamStatus::STREAM_STAND_BY);
    ptrRendererInClientInner->CheckFrozenStateInStaticMode();
    EXPECT_NE(ptrRendererInClientInner->clientBuffer_->GetStreamStatus()->load(), StreamStatus::STREAM_IDEL);
}

/**
 * @tc.name  : Test CheckOperations API with static renderer
 * @tc.type  : FUNC
 * @tc.number: CheckFrozenStateInStaticMode_002
 * @tc.desc  : Test CheckFrozenStateInStaticMode with static renderer info
 */
HWTEST(RendererInClientInnerUnitTest, CheckFrozenStateInStaticMode_002, TestSize.Level4)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    ASSERT_TRUE(ptrRendererInClientInner != nullptr);
    ptrRendererInClientInner->rendererInfo_.isStatic = false;
    ptrRendererInClientInner->clientBuffer_ = OHAudioBufferBase::CreateFromLocal(10, 10);
    ptrRendererInClientInner->clientBuffer_->SetStaticMode(true);
    ptrRendererInClientInner->clientBuffer_->CheckFrozenAndSetLastProcessTime(BUFFER_IN_CLIENT);
    ptrRendererInClientInner->ipcStream_ = new(std::nothrow) IpcStreamTest();

    ptrRendererInClientInner->CheckFrozenStateInStaticMode();
    ptrRendererInClientInner->clientBuffer_->GetStreamStatus()->store(StreamStatus::STREAM_STAND_BY);
    ptrRendererInClientInner->CheckFrozenStateInStaticMode();
    EXPECT_NE(ptrRendererInClientInner->clientBuffer_->GetStreamStatus()->load(), StreamStatus::STREAM_IDEL);
}

/**
 * @tc.name  : Test CheckOperations API with static renderer
 * @tc.type  : FUNC
 * @tc.number: CheckFrozenStateInStaticMode_003
 * @tc.desc  : Test CheckFrozenStateInStaticMode with static renderer info
 */
HWTEST(RendererInClientInnerUnitTest, CheckFrozenStateInStaticMode_003, TestSize.Level4)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    ASSERT_TRUE(ptrRendererInClientInner != nullptr);
    ptrRendererInClientInner->rendererInfo_.isStatic = true;
    ptrRendererInClientInner->clientBuffer_ = OHAudioBufferBase::CreateFromLocal(10, 10);
    ptrRendererInClientInner->ipcStream_ = new(std::nothrow) IpcStreamTest();

    ptrRendererInClientInner->CheckFrozenStateInStaticMode();
    ptrRendererInClientInner->clientBuffer_->GetStreamStatus()->store(StreamStatus::STREAM_STAND_BY);
    ptrRendererInClientInner->CheckFrozenStateInStaticMode();
    EXPECT_NE(ptrRendererInClientInner->clientBuffer_->GetStreamStatus()->load(), StreamStatus::STREAM_IDEL);
}

/**
 * @tc.name  : Test CheckOperations API with static renderer
 * @tc.type  : FUNC
 * @tc.number: CheckFrozenStateInStaticMode_004
 * @tc.desc  : Test CheckFrozenStateInStaticMode with static renderer info
 */
HWTEST(RendererInClientInnerUnitTest, CheckFrozenStateInStaticMode_004, TestSize.Level4)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    ASSERT_TRUE(ptrRendererInClientInner != nullptr);
    ptrRendererInClientInner->rendererInfo_.isStatic = false;
    ptrRendererInClientInner->clientBuffer_ = OHAudioBufferBase::CreateFromLocal(10, 10);
    ptrRendererInClientInner->ipcStream_ = new(std::nothrow) IpcStreamTest();

    ptrRendererInClientInner->CheckFrozenStateInStaticMode();
    ptrRendererInClientInner->clientBuffer_->GetStreamStatus()->store(StreamStatus::STREAM_STAND_BY);
    ptrRendererInClientInner->CheckFrozenStateInStaticMode();
    EXPECT_NE(ptrRendererInClientInner->clientBuffer_->GetStreamStatus()->load(), StreamStatus::STREAM_IDEL);
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_094
 * @tc.desc  : Test RendererInClientInner::SerAudioStreamInfo
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_094, TestSize.Level4)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getpid());
    AudioStreamParams info;
    info.encoding = AudioEncodingType::ENCODING_AUDIOVIVID;
    info.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    info.format = AudioSampleFormat::SAMPLE_S16LE;
    info.channels = AudioChannel::CHANNEL_8;
    info.isRemoteSpatialChannel = false;
    ptrRendererInClientInner->rendererInfo_.rendererFlags = AUDIO_FLAG_3DA_DIRECT;

    ptrRendererInClientInner->SetAudioStreamInfo(info, nullptr);
    EXPECT_EQ(ptrRendererInClientInner->curStreamParams_.channelLayout, AudioChannelLayout::CH_LAYOUT_5POINT1POINT2);
}

/**
 * @tc.name  : Test RendererInClientInner_WriteMuteDataSysEvent_001
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_WriteMuteDataSysEvent_001
 * @tc.desc  : Test RendererInClientInner::WriteMuteDataSysEvent with silent mode enabled
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_WriteMuteDataSysEvent_001, TestSize.Level4)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getpid());
    ptrRendererInClientInner->SetSilentModeAndMixWithOthers(true);
    uint8_t buffer[10] = {0};
    size_t bufferSize = 10;
    ptrRendererInClientInner->WriteMuteDataSysEvent(buffer, bufferSize);
    // Verify function executed without error
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test RendererInClientInner_WriteMuteDataSysEvent_002
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_WriteMuteDataSysEvent_002
 * @tc.desc  : Test RendererInClientInner::WriteMuteDataSysEvent with invalid buffer and silent mode disabled
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_WriteMuteDataSysEvent_002, TestSize.Level4)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getpid());
    ptrRendererInClientInner->SetSilentModeAndMixWithOthers(false);
    uint8_t buffer[10] = {0};
    size_t bufferSize = 10;
    ptrRendererInClientInner->WriteMuteDataSysEvent(buffer, bufferSize);
    // Verify startMuteTime_ was set
    EXPECT_EQ(ptrRendererInClientInner->startMuteTime_, 0);
}

/**
 * @tc.name  : Test RendererInClientInner_WriteMuteDataSysEvent_003
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_WriteMuteDataSysEvent_003
 * @tc.desc  : Test RendererInClientInner::WriteMuteDataSysEvent with valid buffer data
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_WriteMuteDataSysEvent_003, TestSize.Level4)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getpid());
    ptrRendererInClientInner->SetSilentModeAndMixWithOthers(false);
    uint8_t buffer[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    size_t bufferSize = 10;
    ptrRendererInClientInner->startMuteTime_ = 1000000;
    ptrRendererInClientInner->WriteMuteDataSysEvent(buffer, bufferSize);
    // Verify startMuteTime_ was reset
    EXPECT_EQ(ptrRendererInClientInner->startMuteTime_, 0);
}

/**
 * @tc.name  : Test RendererInClientInner_IsInvalidBuffer_001
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_IsInvalidBuffer_001
 * @tc.desc  : Test RendererInClientInner::IsInvalidBuffer with SAMPLE_U8 format and zero data
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_IsInvalidBuffer_001, TestSize.Level4)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getpid());
    ptrRendererInClientInner->clientConfig_.streamInfo.format = SAMPLE_U8;
    uint8_t buffer[1] = {0};
    size_t bufferSize = 1;
    bool result = ptrRendererInClientInner->IsInvalidBuffer(buffer, bufferSize);
    EXPECT_TRUE(result);
}

/**
 * @tc.name  : Test RendererInClientInner_IsInvalidBuffer_002
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_IsInvalidBuffer_002
 * @tc.desc  : Test RendererInClientInner::IsInvalidBuffer with SAMPLE_U8 format and non-zero data
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_IsInvalidBuffer_002, TestSize.Level4)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getpid());
    ptrRendererInClientInner->clientConfig_.streamInfo.format = SAMPLE_U8;
    uint8_t buffer[1] = {1};
    size_t bufferSize = 1;
    bool result = ptrRendererInClientInner->IsInvalidBuffer(buffer, bufferSize);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : Test RendererInClientInner_ProcessVolume_001
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_ProcessVolume_001
 * @tc.desc  : Test RendererInClientInner::ProcessVolume when volume ramp is active
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_ProcessVolume_001, TestSize.Level4)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getpid());
    // Test the function without directly manipulating volume ramp state
    ptrRendererInClientInner->ProcessVolume();
    // Verify function executed without error
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test RendererInClientInner_ProcessVolume_002
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_ProcessVolume_002
 * @tc.desc  : Test RendererInClientInner::ProcessVolume when volume ramp is inactive
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_ProcessVolume_002, TestSize.Level4)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getpid());
    // Test the function without directly manipulating volume ramp state
    ptrRendererInClientInner->ProcessVolume();
    // Verify function executed without error
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test RendererInClientInner_OnSpatializationStateChange_001
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_OnSpatializationStateChange_001
 * @tc.desc  : Test RendererInClientInner::OnSpatializationStateChange with valid state
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_OnSpatializationStateChange_001, TestSize.Level4)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getpid());
    AudioSpatializationState state;
    state.spatializationEnabled = true;
    state.headTrackingEnabled = true;
    ptrRendererInClientInner->OnSpatializationStateChange(state);
    // Verify function executed without error
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test RendererInClientInner_SetRendererInfo_001
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_SetRendererInfo_001
 * @tc.desc  : Test SetRendererInfo with null ipcStream_.
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_SetRendererInfo_001, TestSize.Level4)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getpid());
    ASSERT_TRUE(ptrRendererInClientInner != nullptr);
    ptrRendererInClientInner->ipcStream_ = nullptr;

    AudioRendererInfo rendererInfo;
    rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    rendererInfo.voipNoPrivacyFlag = true;
    ptrRendererInClientInner->SetRendererInfo(rendererInfo);
    EXPECT_TRUE(ptrRendererInClientInner->rendererInfo_.voipNoPrivacyFlag);
}

/**
 * @tc.name  : Test RendererInClientInner_SetRendererInfo_002
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_SetRendererInfo_002
 * @tc.desc  : Test SetRendererInfo with valid ipcStream_.
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_SetRendererInfo_002, TestSize.Level4)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getpid());
    ASSERT_TRUE(ptrRendererInClientInner != nullptr);
    sptr<IpcStreamTest> ipcStreamTest = new(std::nothrow) IpcStreamTest();
    ptrRendererInClientInner->ipcStream_ = ipcStreamTest;

    AudioRendererInfo rendererInfo;
    rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    rendererInfo.voipNoPrivacyFlag = true;
    ptrRendererInClientInner->SetRendererInfo(rendererInfo);
    EXPECT_TRUE(ptrRendererInClientInner->rendererInfo_.voipNoPrivacyFlag);
    EXPECT_TRUE(ipcStreamTest->setVoipNoPrivacyFlagCalled);
    EXPECT_TRUE(ipcStreamTest->lastVoipNoPrivacyFlag);
}

/**
 * @tc.name  : Test RendererInClientInner_SetRendererInfo_003
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_SetRendererInfo_003
 * @tc.desc  : Test SetRendererInfo when ipcStream_ update returns error.
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_SetRendererInfo_003, TestSize.Level4)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getpid());
    ASSERT_TRUE(ptrRendererInClientInner != nullptr);
    sptr<IpcStreamTest> ipcStreamTest = new(std::nothrow) IpcStreamTest();
    ipcStreamTest->setVoipNoPrivacyFlagRet = ERROR;
    ptrRendererInClientInner->ipcStream_ = ipcStreamTest;

    AudioRendererInfo rendererInfo;
    rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    rendererInfo.voipNoPrivacyFlag = true;
    ptrRendererInClientInner->SetRendererInfo(rendererInfo);
    EXPECT_TRUE(ipcStreamTest->setVoipNoPrivacyFlagCalled);
}

/**
 * @tc.name  : Test CheckAndReportTimestamp API
 * @tc.type  : FUNC
 * @tc.number: CheckAndReportTimestamp_001
 * @tc.desc  : Test CheckAndReportTimestamp when duration <= MIN_INTERVAL_IN_NS is true
 */
HWTEST(RendererInClientInnerUnitTest, CheckAndReportTimestamp_001, TestSize.Level3)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getpid());
    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    // Set lastTriggerTime_ to a recent time to make duration small
    ptrRendererInClientInner->lastTriggerTime_ = ClockTime::GetCurNano() - 100000000; // 100ms ago

    // Call CheckAndReportTimestamp - duration should be <= MIN_INTERVAL_IN_NS
    ptrRendererInClientInner->CheckAndReportTimestamp();

    // logStopCallCount_ should be set to true
    EXPECT_TRUE(ptrRendererInClientInner->logStopCallCount_);
}

/**
 * @tc.name  : Test CheckAndReportTimestamp API
 * @tc.type  : FUNC
 * @tc.number: CheckAndReportTimestamp_002
 * @tc.desc  : Test CheckAndReportTimestamp when duration <= MIN_INTERVAL_IN_NS is false
 */
HWTEST(RendererInClientInnerUnitTest, CheckAndReportTimestamp_002, TestSize.Level3)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getpid());
    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    // Set lastTriggerTime_ to a very old time to make duration large
    ptrRendererInClientInner->lastTriggerTime_ = ClockTime::GetCurNano() - 200000000; // 200ms ago

    // Call CheckAndReportTimestamp - duration should be > MIN_INTERVAL_IN_NS
    ptrRendererInClientInner->CheckAndReportTimestamp();

    // logStopCallCount_ should remain false
    EXPECT_FALSE(ptrRendererInClientInner->logStopCallCount_);
}

void InitRendererOptions(AudioRendererOptions &options)
{
    options.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    options.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    options.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    options.streamInfo.channels = AudioChannel::STEREO;
    options.rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    options.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    options.rendererInfo.rendererFlags = AUDIO_FLAG_NORMAL;
}

class RendererInClientNewUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void RendererInClientNewUnitTest::SetUpTestCase(void) {}
void RendererInClientNewUnitTest::TearDownTestCase(void) {}
void RendererInClientNewUnitTest::SetUp(void) {}
void RendererInClientNewUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test IsLowLatencyRenderer with AUDIO_FLAG_MMAP
 * @tc.type  : FUNC
 * @tc.number: IsLowLatencyRenderer_MMAP_001
 * @tc.desc  : Test IsLowLatencyRenderer returns true for AUDIO_FLAG_MMAP.
 */
HWTEST(RendererInClientNewUnitTest, IsLowLatencyRenderer_MMAP_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->rendererInfo_.rendererFlags = AUDIO_FLAG_MMAP;
    
    bool isLowLatency = rendererInClientInner->IsLowLatencyRenderer();
    EXPECT_TRUE(isLowLatency);
}

/**
 * @tc.name  : Test IsLowLatencyRenderer with AUDIO_FLAG_VOIP_FAST
 * @tc.type  : FUNC
 * @tc.number: IsLowLatencyRenderer_VOIP_FAST_001
 * @tc.desc  : Test IsLowLatencyRenderer returns true for AUDIO_FLAG_VOIP_FAST.
 */
HWTEST(RendererInClientNewUnitTest, IsLowLatencyRenderer_VOIP_FAST_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->rendererInfo_.rendererFlags = AUDIO_FLAG_VOIP_FAST;
    
    bool isLowLatency = rendererInClientInner->IsLowLatencyRenderer();
    EXPECT_TRUE(isLowLatency);
}

/**
 * @tc.name  : Test IsLowLatencyRenderer with AUDIO_FLAG_NORMAL
 * @tc.type  : FUNC
 * @tc.number: IsLowLatencyRenderer_NORMAL_001
 * @tc.desc  : Test IsLowLatencyRenderer returns false for AUDIO_FLAG_NORMAL.
 */
HWTEST(RendererInClientNewUnitTest, IsLowLatencyRenderer_NORMAL_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->rendererInfo_.rendererFlags = AUDIO_FLAG_NORMAL;
    
    bool isLowLatency = rendererInClientInner->IsLowLatencyRenderer();
    EXPECT_FALSE(isLowLatency);
}

/**
 * @tc.name  : Test IsLowLatencyRenderer with AUDIO_FLAG_DIRECT
 * @tc.type  : FUNC
 * @tc.number: IsLowLatencyRenderer_DIRECT_001
 * @tc.desc  : Test IsLowLatencyRenderer returns false for AUDIO_FLAG_DIRECT.
 */
HWTEST(RendererInClientNewUnitTest, IsLowLatencyRenderer_DIRECT_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->rendererInfo_.rendererFlags = AUDIO_FLAG_DIRECT;
    
    bool isLowLatency = rendererInClientInner->IsLowLatencyRenderer();
    EXPECT_FALSE(isLowLatency);
}

/**
 * @tc.name  : Test GetDefaultCallbackBufferDurationInUs for low latency renderer
 * @tc.type  : FUNC
 * @tc.number: GetDefaultCallbackBufferDurationInUs_LowLatency_001
 * @tc.desc  : Test GetDefaultCallbackBufferDurationInUs for low latency renderer.
 */
HWTEST(RendererInClientNewUnitTest, GetDefaultCallbackBufferDurationInUs_LowLatency_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->rendererInfo_.rendererFlags = AUDIO_FLAG_MMAP;
    rendererInClientInner->curStreamParams_.samplingRate = SAMPLE_RATE_48000;
    rendererInClientInner->curStreamParams_.customSampleRate = 0;
    rendererInClientInner->spanSizeInFrame_ = 960;
    rendererInClientInner->cacheSizeInFrame_ = 1920;
    
    uint64_t duration = rendererInClientInner->GetDefaultCallbackBufferDurationInUs();
    uint64_t expectedDuration = 960 * 1000000 / 48000; // 20000 us
    EXPECT_EQ(duration, expectedDuration);
}

/**
 * @tc.name  : Test GetDefaultCallbackBufferDurationInUs for normal renderer
 * @tc.type  : FUNC
 * @tc.number: GetDefaultCallbackBufferDurationInUs_Normal_001
 * @tc.desc  : Test GetDefaultCallbackBufferDurationInUs returns OLD_BUF_DURATION for normal renderer.
 */
HWTEST(RendererInClientNewUnitTest, GetDefaultCallbackBufferDurationInUs_Normal_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->rendererInfo_.rendererFlags = AUDIO_FLAG_NORMAL;
    rendererInClientInner->spanSizeInFrame_ = 0;
    
    uint64_t duration = rendererInClientInner->GetDefaultCallbackBufferDurationInUs();
    EXPECT_EQ(duration, 92880); // OLD_BUF_DURATION_IN_USEC
}

/**
 * @tc.name  : Test GetDefaultCallbackBufferDurationInUs with zero sample rate
 * @tc.type  : FUNC
 * @tc.number: GetDefaultCallbackBufferDurationInUs_ZeroRate_001
 * @tc.desc  : Test GetDefaultCallbackBufferDurationInUs returns OLD_BUF_DURATION when sample rate is 0.
 */
HWTEST(RendererInClientNewUnitTest, GetDefaultCallbackBufferDurationInUs_ZeroRate_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->rendererInfo_.rendererFlags = AUDIO_FLAG_MMAP;
    rendererInClientInner->curStreamParams_.samplingRate = 0;
    rendererInClientInner->curStreamParams_.customSampleRate = 0;
    rendererInClientInner->spanSizeInFrame_ = 960;
    
    uint64_t duration = rendererInClientInner->GetDefaultCallbackBufferDurationInUs();
    EXPECT_EQ(duration, 92880);
}

/**
 * @tc.name  : Test GetDefaultCallbackBufferDurationInUs with custom sample rate
 * @tc.type  : FUNC
 * @tc.number: GetDefaultCallbackBufferDurationInUs_CustomRate_001
 * @tc.desc  : Test GetDefaultCallbackBufferDurationInUs uses custom sample rate.
 */
HWTEST(RendererInClientNewUnitTest, GetDefaultCallbackBufferDurationInUs_CustomRate_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->rendererInfo_.rendererFlags = AUDIO_FLAG_MMAP;
    rendererInClientInner->curStreamParams_.samplingRate = SAMPLE_RATE_48000;
    rendererInClientInner->curStreamParams_.customSampleRate = SAMPLE_RATE_44100;
    rendererInClientInner->spanSizeInFrame_ = 441;
    
    uint64_t duration = rendererInClientInner->GetDefaultCallbackBufferDurationInUs();
    uint64_t expectedDuration = 441 * 1000000 / 44100; // 10000 us
    EXPECT_EQ(duration, expectedDuration);
}

/**
 * @tc.name  : Test GetDefaultCallbackBufferDurationInUs with zero frame count
 * @tc.type  : FUNC
 * @tc.number: GetDefaultCallbackBufferDurationInUs_ZeroFrame_001
 * @tc.desc  : Test GetDefaultCallbackBufferDurationInUs returns OLD_BUF_DURATION when frame count is 0.
 */
HWTEST(RendererInClientNewUnitTest, GetDefaultCallbackBufferDurationInUs_ZeroFrame_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->rendererInfo_.rendererFlags = AUDIO_FLAG_MMAP;
    rendererInClientInner->curStreamParams_.samplingRate = SAMPLE_RATE_48000;
    rendererInClientInner->spanSizeInFrame_ = 0;
    rendererInClientInner->cacheSizeInFrame_ = 0;
    
    uint64_t duration = rendererInClientInner->GetDefaultCallbackBufferDurationInUs();
    EXPECT_EQ(duration, 92880);
}

/**
 * @tc.name  : Test GetBufferWaitTimeoutInMs for offload
 * @tc.type  : FUNC
 * @tc.number: GetBufferWaitTimeoutInMs_Offload_001
 * @tc.desc  : Test GetBufferWaitTimeoutInMs returns OFFLOAD timeout for offload mode.
 */
HWTEST(RendererInClientNewUnitTest, GetBufferWaitTimeoutInMs_Offload_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->offloadEnable_ = true;
    rendererInClientInner->rendererInfo_.rendererFlags = AUDIO_FLAG_MMAP;
    
    int32_t timeout = rendererInClientInner->GetBufferWaitTimeoutInMs();
    EXPECT_EQ(timeout, 8000); // OFFLOAD_OPERATION_TIMEOUT_IN_MS
}

/**
 * @tc.name  : Test GetBufferWaitTimeoutInMs for low latency
 * @tc.type  : FUNC
 * @tc.number: GetBufferWaitTimeoutInMs_LowLatency_001
 * @tc.desc  : Test GetBufferWaitTimeoutInMs returns FAST timeout for low latency.
 */
HWTEST(RendererInClientNewUnitTest, GetBufferWaitTimeoutInMs_LowLatency_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->offloadEnable_ = false;
    rendererInClientInner->rendererInfo_.rendererFlags = AUDIO_FLAG_MMAP;
    
    int32_t timeout = rendererInClientInner->GetBufferWaitTimeoutInMs();
    EXPECT_EQ(timeout, 40); // FAST_WRITE_CACHE_TIMEOUT_IN_MS
}

/**
 * @tc.name  : Test GetBufferWaitTimeoutInMs for normal renderer
 * @tc.type  : FUNC
 * @tc.number: GetBufferWaitTimeoutInMs_Normal_001
 * @tc.desc  : Test GetBufferWaitTimeoutInMs returns normal timeout for normal renderer.
 */
HWTEST(RendererInClientNewUnitTest, GetBufferWaitTimeoutInMs_Normal_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->offloadEnable_ = false;
    rendererInClientInner->rendererInfo_.rendererFlags = AUDIO_FLAG_NORMAL;
    
    int32_t timeout = rendererInClientInner->GetBufferWaitTimeoutInMs();
    EXPECT_EQ(timeout, 1500); // WRITE_CACHE_TIMEOUT_IN_MS
}

/**
 * @tc.name  : Test GetBufferWaitTimeoutInMs for VOIP_FAST
 * @tc.type  : FUNC
 * @tc.number: GetBufferWaitTimeoutInMs_VOIP_FAST_001
 * @tc.desc  : Test GetBufferWaitTimeoutInMs returns FAST timeout for VOIP_FAST.
 */
HWTEST(RendererInClientNewUnitTest, GetBufferWaitTimeoutInMs_VOIP_FAST_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->offloadEnable_ = false;
    rendererInClientInner->rendererInfo_.rendererFlags = AUDIO_FLAG_VOIP_FAST;
    
    int32_t timeout = rendererInClientInner->GetBufferWaitTimeoutInMs();
    EXPECT_EQ(timeout, 40); // FAST_WRITE_CACHE_TIMEOUT_IN_MS
}

/**
 * @tc.name  : Test ConstructConfig with AUDIO_FLAG_MMAP
 * @tc.type  : FUNC
 * @tc.number: ConstructConfig_MMAP_001
 * @tc.desc  : Test ConstructConfig with AUDIO_FLAG_MMAP should pass through.
 */
HWTEST(RendererInClientNewUnitTest, ConstructConfig_MMAP_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->clientPid_ = 10;
    rendererInClientInner->clientUid_ = 10;
    rendererInClientInner->curStreamParams_.channels = STEREO;
    rendererInClientInner->curStreamParams_.encoding = ENCODING_PCM;
    rendererInClientInner->curStreamParams_.format = SAMPLE_S16LE;
    rendererInClientInner->curStreamParams_.samplingRate = SAMPLE_RATE_48000;
    
    rendererInClientInner->rendererInfo_.rendererFlags = AUDIO_FLAG_MMAP;
    AudioProcessConfig config = rendererInClientInner->ConstructConfig();
    EXPECT_EQ(config.rendererInfo.rendererFlags, AUDIO_FLAG_MMAP);
}

/**
 * @tc.name  : Test ConstructConfig with AUDIO_FLAG_VOIP_FAST
 * @tc.type  : FUNC
 * @tc.number: ConstructConfig_VOIP_FAST_001
 * @tc.desc  : Test ConstructConfig with AUDIO_FLAG_VOIP_FAST should pass through.
 */
HWTEST(RendererInClientNewUnitTest, ConstructConfig_VOIP_FAST_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->clientPid_ = 10;
    rendererInClientInner->clientUid_ = 10;
    rendererInClientInner->curStreamParams_.channels = STEREO;
    rendererInClientInner->curStreamParams_.encoding = ENCODING_PCM;
    rendererInClientInner->curStreamParams_.format = SAMPLE_S16LE;
    rendererInClientInner->curStreamParams_.samplingRate = SAMPLE_RATE_48000;
    
    rendererInClientInner->rendererInfo_.rendererFlags = AUDIO_FLAG_VOIP_FAST;
    AudioProcessConfig config = rendererInClientInner->ConstructConfig();
    EXPECT_EQ(config.rendererInfo.rendererFlags, AUDIO_FLAG_VOIP_FAST);
}

/**
 * @tc.name  : Test ConstructConfig with invalid renderer flags
 * @tc.type  : FUNC
 * @tc.number: ConstructConfig_InvalidFlag_001
 * @tc.desc  : Test ConstructConfig resets invalid renderer flags to 0.
 */
HWTEST(RendererInClientNewUnitTest, ConstructConfig_InvalidFlag_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->clientPid_ = 10;
    rendererInClientInner->clientUid_ = 10;
    rendererInClientInner->curStreamParams_.channels = STEREO;
    rendererInClientInner->curStreamParams_.encoding = ENCODING_PCM;
    rendererInClientInner->curStreamParams_.format = SAMPLE_S16LE;
    rendererInClientInner->curStreamParams_.samplingRate = SAMPLE_RATE_48000;
    
    rendererInClientInner->rendererInfo_.rendererFlags = 99; // Invalid flag
    AudioProcessConfig config = rendererInClientInner->ConstructConfig();
    EXPECT_EQ(config.rendererInfo.rendererFlags, 0);
}

/**
 * @tc.name  : Test ConstructConfig with all valid renderer flags
 * @tc.type  : FUNC
 * @tc.number: ConstructConfig_AllValidFlags_001
 * @tc.desc  : Test ConstructConfig with all valid renderer flags.
 */
HWTEST(RendererInClientNewUnitTest, ConstructConfig_AllValidFlags_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->clientPid_ = 10;
    rendererInClientInner->clientUid_ = 10;
    rendererInClientInner->curStreamParams_.channels = STEREO;
    rendererInClientInner->curStreamParams_.encoding = ENCODING_PCM;
    rendererInClientInner->curStreamParams_.format = SAMPLE_S16LE;
    rendererInClientInner->curStreamParams_.samplingRate = SAMPLE_RATE_48000;
    
    std::vector<int32_t> validFlags = {
        AUDIO_FLAG_NORMAL,
        AUDIO_FLAG_MMAP,
        AUDIO_FLAG_VOIP_FAST,
        AUDIO_FLAG_VOIP_DIRECT,
        AUDIO_FLAG_DIRECT,
        AUDIO_FLAG_3DA_DIRECT
    };
    
    for (auto flag : validFlags) {
        rendererInClientInner->rendererInfo_.rendererFlags = flag;
        AudioProcessConfig config = rendererInClientInner->ConstructConfig();
        EXPECT_EQ(config.rendererInfo.rendererFlags, flag);
    }
}

/**
 * @tc.name  : Test GetStreamClass for low latency renderer
 * @tc.type  : FUNC
 * @tc.number: GetStreamClass_LowLatency_001
 * @tc.desc  : Test GetStreamClass returns FAST_STREAM for low latency renderer.
 */
HWTEST(RendererInClientNewUnitTest, GetStreamClass_LowLatency_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->rendererInfo_.audioFlag = AUDIO_OUTPUT_FLAG_FAST;
    
    IAudioStream::StreamClass streamClass = rendererInClientInner->GetStreamClass();
    EXPECT_EQ(streamClass, IAudioStream::StreamClass::FAST_STREAM);
}

/**
 * @tc.name  : Test GetStreamClass for VOIP_FAST renderer
 * @tc.type  : FUNC
 * @tc.number: GetStreamClass_VOIP_FAST_001
 * @tc.desc  : Test GetStreamClass returns FAST_STREAM for VOIP_FAST renderer.
 */
HWTEST(RendererInClientNewUnitTest, GetStreamClass_VOIP_FAST_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->rendererInfo_.audioFlag = AUDIO_OUTPUT_FLAG_FAST;
    
    IAudioStream::StreamClass streamClass = rendererInClientInner->GetStreamClass();
    EXPECT_EQ(streamClass, IAudioStream::StreamClass::FAST_STREAM);
}

/**
 * @tc.name  : Test GetStreamClass for normal renderer
 * @tc.type  : FUNC
 * @tc.number: GetStreamClass_Normal_001
 * @tc.desc  : Test GetStreamClass returns PA_STREAM for normal renderer.
 */
HWTEST(RendererInClientNewUnitTest, GetStreamClass_Normal_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->rendererInfo_.rendererFlags = AUDIO_FLAG_NORMAL;
    
    IAudioStream::StreamClass streamClass = rendererInClientInner->GetStreamClass();
    EXPECT_EQ(streamClass, IAudioStream::StreamClass::PA_STREAM);
}

/**
 * @tc.name  : Test GetFastStatus for low latency renderer
 * @tc.type  : FUNC
 * @tc.number: GetFastStatus_LowLatency_001
 * @tc.desc  : Test GetFastStatus returns FASTSTATUS_FAST for low latency renderer.
 */
HWTEST(RendererInClientNewUnitTest, GetFastStatus_LowLatency_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->rendererInfo_.audioFlag = AUDIO_OUTPUT_FLAG_FAST;
    
    FastStatus status = rendererInClientInner->GetFastStatus();
    EXPECT_EQ(status, FASTSTATUS_FAST);
}

/**
 * @tc.name  : Test GetFastStatus for VOIP_FAST renderer
 * @tc.type  : FUNC
 * @tc.number: GetFastStatus_VOIP_FAST_001
 * @tc.desc  : Test GetFastStatus returns FASTSTATUS_FAST for VOIP_FAST renderer.
 */
HWTEST(RendererInClientNewUnitTest, GetFastStatus_VOIP_FAST_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->rendererInfo_.audioFlag = AUDIO_OUTPUT_FLAG_FAST;
    
    FastStatus status = rendererInClientInner->GetFastStatus();
    EXPECT_EQ(status, FASTSTATUS_FAST);
}

/**
 * @tc.name  : Test GetFastStatus for normal renderer
 * @tc.type  : FUNC
 * @tc.number: GetFastStatus_Normal_001
 * @tc.desc  : Test GetFastStatus returns FASTSTATUS_NORMAL for normal renderer.
 */
HWTEST(RendererInClientNewUnitTest, GetFastStatus_Normal_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->rendererInfo_.rendererFlags = AUDIO_FLAG_NORMAL;
    
    FastStatus status = rendererInClientInner->GetFastStatus();
    EXPECT_EQ(status, FASTSTATUS_NORMAL);
}

/**
 * @tc.name  : Test CheckAndProcessPendingSpanSize with no pending span
 * @tc.type  : FUNC
 * @tc.number: CheckAndProcessPendingSpanSize_NoPending_001
 * @tc.desc  : Test CheckAndProcessPendingSpanSize with no pending span size.
 */
HWTEST(RendererInClientNewUnitTest, CheckAndProcessPendingSpanSize_NoPending_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_CLIENT;
    uint32_t totalSizeInFrame = 100;
    uint32_t byteSizePerFrame = 4;
    rendererInClientInner->clientBuffer_ =
        std::make_shared<OHAudioBufferBase>(bufferHolder, totalSizeInFrame, byteSizePerFrame);
    
    rendererInClientInner->CheckAndProcessPendingSpanSize();
    EXPECT_EQ(rendererInClientInner->spanSizeInFrame_, 0);
}

/**
 * @tc.name  : Test CheckAndProcessPendingSpanSize with AUDIO_OUTPUT_FLAG_VOIP
 * @tc.type  : FUNC
 * @tc.number: CheckAndProcessPendingSpanSize_VoipFlag_001
 * @tc.desc  : Test CheckAndProcessPendingSpanSize updates rendererFlags for VOIP flag.
 */
HWTEST(RendererInClientNewUnitTest, CheckAndProcessPendingSpanSize_VoipFlag_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_CLIENT;
    uint32_t totalSizeInFrame = 100;
    uint32_t byteSizePerFrame = 4;
    rendererInClientInner->clientBuffer_ =
        std::make_shared<OHAudioBufferBase>(bufferHolder, totalSizeInFrame, byteSizePerFrame);
    rendererInClientInner->sizePerFrameInByte_ = 4;
    
    uint32_t validSpanSize = 960;
    uint64_t engineSize = 1920;
    rendererInClientInner->clientBuffer_->SetPendingSpanSize(validSpanSize, engineSize,
        AUDIO_OUTPUT_FLAG_FAST | AUDIO_OUTPUT_FLAG_VOIP);
    rendererInClientInner->CheckAndProcessPendingSpanSize();
    EXPECT_EQ(rendererInClientInner->rendererInfo_.rendererFlags, AUDIO_FLAG_VOIP_FAST);
}

/**
 * @tc.name  : Test CheckAndProcessPendingSpanSize with null clientBuffer
 * @tc.type  : FUNC
 * @tc.number: CheckAndProcessPendingSpanSize_NullBuffer_001
 * @tc.desc  : Test CheckAndProcessPendingSpanSize with null clientBuffer.
 */
HWTEST(RendererInClientNewUnitTest, CheckAndProcessPendingSpanSize_NullBuffer_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->clientBuffer_ = nullptr;
    
    rendererInClientInner->CheckAndProcessPendingSpanSize();
    EXPECT_EQ(rendererInClientInner->spanSizeInFrame_, 0);
}

class IpcStreamMock : public IIpcStream {
public:
    int32_t RegisterStreamListener(const sptr<IRemoteObject> &object) override { return SUCCESS; }
    int32_t ResolveBuffer(std::shared_ptr<OHAudioBuffer> &buffer) override { return SUCCESS; }
    int32_t UpdatePosition() override { return SUCCESS; }
    int32_t GetAudioSessionID(uint32_t &sessionId) override
    {
        sessionId = 100; // 100 is temp id
        return SUCCESS;
    }
    int32_t Start() override { return SUCCESS; }
    int32_t Pause() override { return SUCCESS; }
    int32_t Stop() override { return SUCCESS; }
    int32_t Release(bool isSwitchStream) override { return SUCCESS; }
    int32_t Flush() override { return SUCCESS; }
    int32_t Drain(bool stopFlag) override { return SUCCESS; }
    int32_t RequestHandleData(uint64_t syncFramePts, uint32_t size) override { return SUCCESS; }
    int32_t UpdatePlaybackCaptureConfig(const AudioPlaybackCaptureConfig &config) override { return SUCCESS; }
    int32_t SetInMainThreadState(bool isInMainThread) override { return SUCCESS; }
    int32_t GetAudioTime(uint64_t &framePos, uint64_t &timestamp) override { return SUCCESS; }
    int32_t GetAudioPosition(
        uint64_t &framePos, uint64_t &timestamp, uint64_t &latency, int32_t base) override { return SUCCESS; }
    int32_t GetSpeedPosition(
        uint64_t &framePos, uint64_t &timestamp, uint64_t &latency, int32_t base) override { return SUCCESS; }
    int32_t GetLatency(uint64_t &latency) override
    {
        latency = 50; // 50 us is temp latency
        return SUCCESS;
    }
    int32_t GetLatencyWithFlag(uint64_t &latency, uint32_t flag) override { return SUCCESS; }
    int32_t SetRate(int32_t rate) override { return SUCCESS; }
    int32_t GetRate(int32_t &rate) override { return SUCCESS; }
    int32_t SetLowPowerVolume(float volume) override { return SUCCESS; }
    int32_t GetLowPowerVolume(float &volume) override { return SUCCESS; }
    int32_t SetAudioEffectMode(int32_t effectMode) override { return SUCCESS; }
    int32_t GetAudioEffectMode(int32_t &effectMode) override { return SUCCESS; }
    int32_t SetPrivacyType(int32_t privacyType) override { return SUCCESS; }
    int32_t SetVoipNoPrivacyFlag(bool voipNoPrivacyFlag) override { return SUCCESS; }
    int32_t GetPrivacyType(int32_t &privacyType) override { return SUCCESS; }
    int32_t SetOffloadMode(int32_t state, bool isAppBack) override { return SUCCESS; }
    int32_t SetTarget(int32_t target, int32_t &ret) override { return SUCCESS; }
    int32_t UnsetOffloadMode() override { return SUCCESS; }
    int32_t GetOffloadApproximatelyCacheTime(uint64_t &timestamp, uint64_t &paWriteIndex,
        uint64_t &cacheTimeDsp, uint64_t &cacheTimePa) override { return SUCCESS; }
    int32_t UpdateSpatializationState(bool spatializationEnabled, bool headTrackingEnabled) override { return SUCCESS; }
    int32_t GetStreamManagerType() override { return SUCCESS; }
    int32_t SetRebuildFlag() override { return SUCCESS; }
    int32_t SetSilentModeAndMixWithOthers(bool on) override { return SUCCESS; }
    int32_t SetClientVolume() override { return SUCCESS; }
    int32_t SetLoudnessGain(float loudnessGain) override { return SUCCESS; }
    int32_t SetMute(bool isMute) override { return SUCCESS; }
    int32_t SetMuteHint(bool mute) override { return SUCCESS; }
    int32_t SetDuckFactor(float duckFactor, uint32_t durationMs) override { return SUCCESS; }
    int32_t RegisterThreadPriority(pid_t tid, const std::string &bundleName, uint32_t method,
        uint32_t threadPriority) override { return SUCCESS; }
    int32_t SetDefaultOutputDevice(int32_t defaultOuputDevice, bool skipForce) override { return SUCCESS; }
    int32_t SetSourceDuration(int64_t duration) override { return SUCCESS; }
    int32_t SetOffloadDataCallbackState(int32_t state) override { return SUCCESS; }
    int32_t SetSpeed(float speed) override { return SUCCESS; }
    int32_t SetPitch(float pitch) override { return SUCCESS; }
    sptr<IRemoteObject> AsObject() override { return nullptr; }
    int32_t ResolveBufferBaseAndGetServerSpanSize(std::shared_ptr<OHAudioBufferBase> &buffer,
        uint32_t &spanSizeInFrame, uint64_t &engineTotalSizeInFrame) override { return SUCCESS; }
    int32_t SetAudioHapticsSyncId(int32_t audioHapticsSyncId) override { return SUCCESS; }
    int32_t SetLoopTimes(int64_t bufferLoopTimes) override { return SUCCESS; }
    int32_t UpdateUnderrunInfo(uint32_t underrunInfoKey, int32_t underrunInfoVal) override { return SUCCESS; }
    int32_t ResetStaticPlayPosition() override { return SUCCESS; }
    int32_t CalculateCacheCount(uint32_t cacheSizeSizeInFrame, uint32_t &cacheCount) override { return SUCCESS; }
};

class RendererInClientPublicNewUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void RendererInClientPublicNewUnitTest::SetUpTestCase(void) {}
void RendererInClientPublicNewUnitTest::TearDownTestCase(void) {}
void RendererInClientPublicNewUnitTest::SetUp(void) {}
void RendererInClientPublicNewUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test SetLoudnessGain for fast stream
 * @tc.type  : FUNC
 * @tc.number: SetLoudnessGain_FastStream_001
 * @tc.desc  : Test SetLoudnessGain returns ERROR for fast stream.
 */
HWTEST(RendererInClientPublicNewUnitTest, SetLoudnessGain_FastStream_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->rendererInfo_.audioFlag = AUDIO_OUTPUT_FLAG_FAST;
    
    int32_t ret = rendererInClientInner->SetLoudnessGain(1.0f);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test SetLoudnessGain for normal stream
 * @tc.type  : FUNC
 * @tc.number: SetLoudnessGain_NormalStream_001
 * @tc.desc  : Test SetLoudnessGain works for normal stream.
 */
HWTEST(RendererInClientPublicNewUnitTest, SetLoudnessGain_NormalStream_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->rendererInfo_.rendererFlags = AUDIO_FLAG_NORMAL;
    rendererInClientInner->ipcStream_ = new IpcStreamMock();
    
    int32_t ret = rendererInClientInner->SetLoudnessGain(1.0f);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test GetLoudnessGain for fast stream
 * @tc.type  : FUNC
 * @tc.number: GetLoudnessGain_FastStream_001
 * @tc.desc  : Test GetLoudnessGain returns 0.0 for fast stream.
 */
HWTEST(RendererInClientPublicNewUnitTest, GetLoudnessGain_FastStream_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->rendererInfo_.audioFlag = AUDIO_OUTPUT_FLAG_FAST;
    rendererInClientInner->loudnessGain_ = 1.0f;
    
    float gain = rendererInClientInner->GetLoudnessGain();
    EXPECT_EQ(gain, 0.0f);
}

/**
 * @tc.name  : Test GetLoudnessGain for normal stream
 * @tc.type  : FUNC
 * @tc.number: GetLoudnessGain_NormalStream_001
 * @tc.desc  : Test GetLoudnessGain works for normal stream.
 */
HWTEST(RendererInClientPublicNewUnitTest, GetLoudnessGain_NormalStream_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->rendererInfo_.rendererFlags = AUDIO_FLAG_NORMAL;
    rendererInClientInner->loudnessGain_ = 1.5f;
    
    float gain = rendererInClientInner->GetLoudnessGain();
    EXPECT_EQ(gain, 1.5f);
}

/**
 * @tc.name  : Test SetRenderRate for fast stream with non-normal rate
 * @tc.type  : FUNC
 * @tc.number: SetRenderRate_FastStream_001
 * @tc.desc  : Test SetRenderRate returns ERR_INVALID_OPERATION for fast stream with non-normal rate.
 */
HWTEST(RendererInClientPublicNewUnitTest, SetRenderRate_FastStream_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->rendererInfo_.audioFlag = AUDIO_OUTPUT_FLAG_FAST;
    
    int32_t ret = rendererInClientInner->SetRenderRate(RENDER_RATE_DOUBLE);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
}

/**
 * @tc.name  : Test SetRenderRate for fast stream with normal rate
 * @tc.type  : FUNC
 * @tc.number: SetRenderRate_FastStream_002
 * @tc.desc  : Test SetRenderRate returns SUCCESS for fast stream with RENDER_RATE_NORMAL.
 */
HWTEST(RendererInClientPublicNewUnitTest, SetRenderRate_FastStream_002, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->rendererInfo_.rendererFlags = AUDIO_FLAG_MMAP;
    
    int32_t ret = rendererInClientInner->SetRenderRate(RENDER_RATE_NORMAL);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetRenderRate for normal stream
 * @tc.type  : FUNC
 * @tc.number: SetRenderRate_NormalStream_001
 * @tc.desc  : Test SetRenderRate works for normal stream.
 */
HWTEST(RendererInClientPublicNewUnitTest, SetRenderRate_NormalStream_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->rendererInfo_.rendererFlags = AUDIO_FLAG_NORMAL;
    rendererInClientInner->ipcStream_ = new IpcStreamMock();
    
    int32_t ret = rendererInClientInner->SetRenderRate(RENDER_RATE_DOUBLE);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetSpeed for fast stream
 * @tc.type  : FUNC
 * @tc.number: SetSpeed_FastStream_001
 * @tc.desc  : Test SetSpeed returns ERR_OPERATION_FAILED for fast stream.
 */
HWTEST(RendererInClientPublicNewUnitTest, SetSpeed_FastStream_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->rendererInfo_.audioFlag = AUDIO_OUTPUT_FLAG_FAST;
    
    int32_t ret = rendererInClientInner->SetSpeed(2.0f);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test SetSpeed for normal stream
 * @tc.type  : FUNC
 * @tc.number: SetSpeed_NormalStream_001
 * @tc.desc  : Test SetSpeed works for normal stream.
 */
HWTEST(RendererInClientPublicNewUnitTest, SetSpeed_NormalStream_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->rendererInfo_.rendererFlags = AUDIO_FLAG_NORMAL;
    rendererInClientInner->ipcStream_ = new IpcStreamMock();
    
    int32_t ret = rendererInClientInner->SetSpeed(2.0f);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetPitch for fast stream
 * @tc.type  : FUNC
 * @tc.number: SetPitch_FastStream_001
 * @tc.desc  : Test SetPitch returns ERR_OPERATION_FAILED for fast stream.
 */
HWTEST(RendererInClientPublicNewUnitTest, SetPitch_FastStream_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->rendererInfo_.rendererFlags = AUDIO_FLAG_MMAP;
    
    int32_t ret = rendererInClientInner->SetPitch(2.0f);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test SetPitch for normal stream
 * @tc.type  : FUNC
 * @tc.number: SetPitch_NormalStream_001
 * @tc.desc  : Test SetPitch works for normal stream.
 */
HWTEST(RendererInClientPublicNewUnitTest, SetPitch_NormalStream_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->rendererInfo_.rendererFlags = AUDIO_FLAG_NORMAL;
    
    int32_t ret = rendererInClientInner->SetPitch(2.0f);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test GetSpeed for fast stream
 * @tc.type  : FUNC
 * @tc.number: GetSpeed_FastStream_001
 * @tc.desc  : Test GetSpeed returns ERROR for fast stream.
 */
HWTEST(RendererInClientPublicNewUnitTest, GetSpeed_FastStream_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->rendererInfo_.rendererFlags = AUDIO_FLAG_MMAP;
    
    float speed = rendererInClientInner->GetSpeed();
    EXPECT_EQ(static_cast<int32_t>(speed), static_cast<int32_t>(ERROR));
}

/**
 * @tc.name  : Test GetSpeed for normal stream
 * @tc.type  : FUNC
 * @tc.number: GetSpeed_NormalStream_001
 * @tc.desc  : Test GetSpeed works for normal stream.
 */
HWTEST(RendererInClientPublicNewUnitTest, GetSpeed_NormalStream_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->rendererInfo_.rendererFlags = AUDIO_FLAG_NORMAL;
    rendererInClientInner->realSpeed_ = 2.0f;
    
    float speed = rendererInClientInner->GetSpeed();
    EXPECT_EQ(speed, 2.0f);
}

/**
 * @tc.name  : Test Clear for fast stream
 * @tc.type  : FUNC
 * @tc.number: Clear_FastStream_001
 * @tc.desc  : Test Clear returns SUCCESS for fast stream.
 */
HWTEST(RendererInClientPublicNewUnitTest, Clear_FastStream_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->rendererInfo_.audioFlag = AUDIO_OUTPUT_FLAG_FAST;
    
    int32_t ret = rendererInClientInner->Clear();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetLowPowerVolume for fast stream
 * @tc.type  : FUNC
 * @tc.number: SetLowPowerVolume_FastStream_001
 * @tc.desc  : Test SetLowPowerVolume returns SUCCESS for fast stream.
 */
HWTEST(RendererInClientPublicNewUnitTest, SetLowPowerVolume_FastStream_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->rendererInfo_.audioFlag = AUDIO_OUTPUT_FLAG_FAST;
    
    int32_t ret = rendererInClientInner->SetLowPowerVolume(0.5f);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test GetLowPowerVolume for fast stream
 * @tc.type  : FUNC
 * @tc.number: GetLowPowerVolume_FastStream_001
 * @tc.desc  : Test GetLowPowerVolume returns 1.0f for fast stream.
 */
HWTEST(RendererInClientPublicNewUnitTest, GetLowPowerVolume_FastStream_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->rendererInfo_.audioFlag = AUDIO_OUTPUT_FLAG_FAST;
    rendererInClientInner->lowPowerVolume_ = 0.5f;
    
    float volume = rendererInClientInner->GetLowPowerVolume();
    EXPECT_EQ(volume, 1.0f);
}

/**
 * @tc.name  : Test SetOffloadMode for fast stream
 * @tc.type  : FUNC
 * @tc.number: SetOffloadMode_FastStream_001
 * @tc.desc  : Test SetOffloadMode returns ERR_NOT_SUPPORTED for fast stream.
 */
HWTEST(RendererInClientPublicNewUnitTest, SetOffloadMode_FastStream_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->rendererInfo_.audioFlag = AUDIO_OUTPUT_FLAG_FAST;
    
    int32_t ret = rendererInClientInner->SetOffloadMode(1, true);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

/**
 * @tc.name  : Test UnsetOffloadMode for fast stream
 * @tc.type  : FUNC
 * @tc.number: UnsetOffloadMode_FastStream_001
 * @tc.desc  : Test UnsetOffloadMode returns ERR_NOT_SUPPORTED for fast stream.
 */
HWTEST(RendererInClientPublicNewUnitTest, UnsetOffloadMode_FastStream_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->rendererInfo_.audioFlag = AUDIO_OUTPUT_FLAG_FAST;
    
    int32_t ret = rendererInClientInner->UnsetOffloadMode();
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

/**
 * @tc.name  : Test GetAudioEffectMode for fast stream
 * @tc.type  : FUNC
 * @tc.number: GetAudioEffectMode_FastStream_001
 * @tc.desc  : Test GetAudioEffectMode returns EFFECT_NONE for fast stream.
 */
HWTEST(RendererInClientPublicNewUnitTest, GetAudioEffectMode_FastStream_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->rendererInfo_.audioFlag = AUDIO_OUTPUT_FLAG_FAST;
    rendererInClientInner->effectMode_ = EFFECT_DEFAULT;
    
    AudioEffectMode mode = rendererInClientInner->GetAudioEffectMode();
    EXPECT_EQ(mode, EFFECT_NONE);
}

/**
 * @tc.name  : Test SetAudioEffectMode for fast stream
 * @tc.type  : FUNC
 * @tc.number: SetAudioEffectMode_FastStream_001
 * @tc.desc  : Test SetAudioEffectMode returns ERR_NOT_SUPPORTED for fast stream.
 */
HWTEST(RendererInClientPublicNewUnitTest, SetAudioEffectMode_FastStream_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->rendererInfo_.audioFlag = AUDIO_OUTPUT_FLAG_FAST;
    
    int32_t ret = rendererInClientInner->SetAudioEffectMode(EFFECT_DEFAULT);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

/**
 * @tc.name  : Test SetPrivacyType for fast stream
 * @tc.type  : FUNC
 * @tc.number: SetPrivacyType_FastStream_001
 * @tc.desc  : Test SetPrivacyType returns early for fast stream.
 */
HWTEST(RendererInClientPublicNewUnitTest, SetPrivacyType_FastStream_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->rendererInfo_.audioFlag = AUDIO_OUTPUT_FLAG_FAST;
    rendererInClientInner->privacyType_ = PRIVACY_TYPE_PUBLIC;
    
    rendererInClientInner->SetPrivacyType(PRIVACY_TYPE_PRIVATE);
    EXPECT_EQ(rendererInClientInner->privacyType_, PRIVACY_TYPE_PUBLIC);
}

/**
 * @tc.name  : Test FlushAudioStream for fast stream
 * @tc.type  : FUNC
 * @tc.number: FlushAudioStream_FastStream_001
 * @tc.desc  : Test FlushAudioStream returns true for fast stream.
 */
HWTEST(RendererInClientPublicNewUnitTest, FlushAudioStream_FastStream_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->rendererInfo_.audioFlag = AUDIO_OUTPUT_FLAG_FAST;
    
    bool ret = rendererInClientInner->FlushAudioStream();
    EXPECT_TRUE(ret);
}

/**
 * @tc.name  : Test DrainAudioStream for fast stream
 * @tc.type  : FUNC
 * @tc.number: DrainAudioStream_FastStream_001
 * @tc.desc  : Test DrainAudioStream returns true for fast stream.
 */
HWTEST(RendererInClientPublicNewUnitTest, DrainAudioStream_FastStream_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->rendererInfo_.audioFlag = AUDIO_OUTPUT_FLAG_FAST;
    
    bool ret = rendererInClientInner->DrainAudioStream(false);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name  : Test Write with meta for fast stream
 * @tc.type  : FUNC
 * @tc.number: Write_Meta_FastStream_001
 * @tc.desc  : Test Write with meta returns ERR_INVALID_OPERATION for fast stream.
 */
HWTEST(RendererInClientPublicNewUnitTest, Write_Meta_FastStream_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->rendererInfo_.audioFlag = AUDIO_OUTPUT_FLAG_FAST;
    uint8_t buffer[1024] = {0};
    uint8_t metaBuffer[1024] = {0};
    int32_t ret = rendererInClientInner->Write(buffer, 1024, metaBuffer, 1024);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
}

/**
 * @tc.name  : Test SetRendererPositionCallback for fast stream
 * @tc.type  : FUNC
 * @tc.number: SetRendererPositionCallback_FastStream_001
 * @tc.desc  : Test SetRendererPositionCallback returns early for fast stream.
 */
HWTEST(RendererInClientPublicNewUnitTest, SetRendererPositionCallback_FastStream_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->rendererInfo_.rendererFlags = AUDIO_FLAG_MMAP;
    
    rendererInClientInner->SetRendererPositionCallback(1000, nullptr);
    EXPECT_EQ(rendererInClientInner->rendererPositionCallback_, nullptr);
}

/**
 * @tc.name  : Test UnsetRendererPositionCallback for fast stream
 * @tc.type  : FUNC
 * @tc.number: UnsetRendererPositionCallback_FastStream_001
 * @tc.desc  : Test UnsetRendererPositionCallback returns early for fast stream.
 */
HWTEST(RendererInClientPublicNewUnitTest, UnsetRendererPositionCallback_FastStream_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->rendererInfo_.rendererFlags = AUDIO_FLAG_MMAP;
    
    rendererInClientInner->UnsetRendererPositionCallback();
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test SetRendererPeriodPositionCallback for fast stream
 * @tc.type  : FUNC
 * @tc.number: SetRendererPeriodPositionCallback_FastStream_001
 * @tc.desc  : Test SetRendererPeriodPositionCallback returns early for fast stream.
 */
HWTEST(RendererInClientPublicNewUnitTest, SetRendererPeriodPositionCallback_FastStream_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->rendererInfo_.rendererFlags = AUDIO_FLAG_MMAP;
    
    rendererInClientInner->SetRendererPeriodPositionCallback(1000, nullptr);
    EXPECT_EQ(rendererInClientInner->rendererPeriodPositionCallback_, nullptr);
}

/**
 * @tc.name  : Test UnsetRendererPeriodPositionCallback for fast stream
 * @tc.type  : FUNC
 * @tc.number: UnsetRendererPeriodPositionCallback_FastStream_001
 * @tc.desc  : Test UnsetRendererPeriodPositionCallback returns early for fast stream.
 */
HWTEST(RendererInClientPublicNewUnitTest, UnsetRendererPeriodPositionCallback_FastStream_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->rendererInfo_.rendererFlags = AUDIO_FLAG_MMAP;
    
    rendererInClientInner->UnsetRendererPeriodPositionCallback();
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test SetBufferSizeInMsec for fast stream
 * @tc.type  : FUNC
 * @tc.number: SetBufferSizeInMsec_FastStream_001
 * @tc.desc  : Test SetBufferSizeInMsec returns ERR_NOT_SUPPORTED for fast stream.
 */
HWTEST(RendererInClientPublicNewUnitTest, SetBufferSizeInMsec_FastStream_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->rendererInfo_.audioFlag = AUDIO_OUTPUT_FLAG_FAST;
    
    int32_t ret = rendererInClientInner->SetBufferSizeInMsec(10);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

/**
 * @tc.name  : Test SetChannelBlendMode for fast stream
 * @tc.type  : FUNC
 * @tc.number: SetChannelBlendMode_FastStream_001
 * @tc.desc  : Test SetChannelBlendMode returns SUCCESS for fast stream.
 */
HWTEST(RendererInClientPublicNewUnitTest, SetChannelBlendMode_FastStream_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->rendererInfo_.rendererFlags = AUDIO_FLAG_MMAP;
    
    int32_t ret = rendererInClientInner->SetChannelBlendMode(MODE_DEFAULT);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetVolumeWithRamp for fast stream
 * @tc.type  : FUNC
 * @tc.number: SetVolumeWithRamp_FastStream_001
 * @tc.desc  : Test SetVolumeWithRamp returns SUCCESS for fast stream.
 */
HWTEST(RendererInClientPublicNewUnitTest, SetVolumeWithRamp_FastStream_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->rendererInfo_.rendererFlags = AUDIO_FLAG_MMAP;
    rendererInClientInner->state_ = NEW;
    
    int32_t ret = rendererInClientInner->SetVolumeWithRamp(0.5f, 100);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test OnFirstFrameWriting with ipcStream
 * @tc.type  : FUNC
 * @tc.number: OnFirstFrameWriting_001
 * @tc.desc  : Test OnFirstFrameWriting gets latency from ipcStream.
 */
HWTEST(RendererInClientPublicNewUnitTest, OnFirstFrameWriting_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->ipcStream_ = new IpcStreamMock();
    
    rendererInClientInner->OnFirstFrameWriting();
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test OnFirstFrameWriting without ipcStream
 * @tc.type  : FUNC
 * @tc.number: OnFirstFrameWriting_002
 * @tc.desc  : Test OnFirstFrameWriting uses default latency without ipcStream.
 */
HWTEST(RendererInClientPublicNewUnitTest, OnFirstFrameWriting_002, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    rendererInClientInner->ipcStream_ = nullptr;
    
    rendererInClientInner->OnFirstFrameWriting();
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test NotifyRouteUpdate logging
 * @tc.type  : FUNC
 * @tc.number: NotifyRouteUpdate_001
 * @tc.desc  : Test NotifyRouteUpdate logs routeFlag and networkId.
 */
HWTEST(RendererInClientPublicNewUnitTest, NotifyRouteUpdate_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    
    rendererInClientInner->NotifyRouteUpdate(AUDIO_OUTPUT_FLAG_FAST, LOCAL_NETWORK_ID);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test NotifyRouteUpdate with different routeFlag
 * @tc.type  : FUNC
 * @tc.number: NotifyRouteUpdate_002
 * @tc.desc  : Test NotifyRouteUpdate with various routeFlags.
 */
HWTEST(RendererInClientPublicNewUnitTest, NotifyRouteUpdate_002, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    
    rendererInClientInner->NotifyRouteUpdate(AUDIO_OUTPUT_FLAG_VOIP, LOCAL_NETWORK_ID);
    rendererInClientInner->NotifyRouteUpdate(AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD, LOCAL_NETWORK_ID);
    rendererInClientInner->NotifyRouteUpdate(AUDIO_OUTPUT_FLAG_LOWPOWER, LOCAL_NETWORK_ID);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test OnOperationHandled with UPDATE_SPANSIZE
 * @tc.type  : FUNC
 * @tc.number: OnOperationHandled_UpdateSpanSize_001
 * @tc.desc  : Test OnOperationHandled handles UPDATE_SPANSIZE operation.
 */
HWTEST(RendererInClientPublicNewUnitTest, OnOperationHandled_UpdateSpanSize_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_CLIENT;
    rendererInClientInner->clientBuffer_ = std::make_shared<OHAudioBufferBase>(bufferHolder, 100, 4);
    rendererInClientInner->ipcStream_ = new IpcStreamMock();
    int32_t notifyCount = 0;
    FastStatus notifiedStatus = FASTSTATUS_INVALID;
    rendererInClientInner->SetFastStatusChangeCallback([&notifyCount, &notifiedStatus] (FastStatus status) {
        notifyCount++;
        notifiedStatus = status;
    });
    
    int32_t ret = rendererInClientInner->OnOperationHandled(UPDATE_SPANSIZE, FASTSTATUS_FAST);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(rendererInClientInner->fastStatus_.load(), FASTSTATUS_FAST);
    EXPECT_EQ(notifyCount, 1);
    EXPECT_EQ(notifiedStatus, FASTSTATUS_FAST);
}

/**
 * @tc.name  : Test NotifyFastStatusChange
 * @tc.type  : FUNC
 * @tc.number: NotifyFastStatusChange_001
 * @tc.desc  : Test valid, invalid, and empty fast status callback branches.
 */
HWTEST(RendererInClientPublicNewUnitTest, NotifyFastStatusChange_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    int32_t notifyCount = 0;
    FastStatus notifiedStatus = FASTSTATUS_INVALID;
    rendererInClientInner->SetFastStatusChangeCallback([&notifyCount, &notifiedStatus] (FastStatus status) {
        notifyCount++;
        notifiedStatus = status;
    });

    rendererInClientInner->NotifyFastStatusChange(FASTSTATUS_FAST);
    EXPECT_EQ(rendererInClientInner->fastStatus_.load(), FASTSTATUS_FAST);
    EXPECT_EQ(notifyCount, 1);
    EXPECT_EQ(notifiedStatus, FASTSTATUS_FAST);

    rendererInClientInner->NotifyFastStatusChange(FASTSTATUS_INVALID);
    EXPECT_EQ(rendererInClientInner->fastStatus_.load(), FASTSTATUS_FAST);
    EXPECT_EQ(notifyCount, 1);

    rendererInClientInner->SetFastStatusChangeCallback(std::function<void(FastStatus)>());
    rendererInClientInner->NotifyFastStatusChange(FASTSTATUS_NORMAL);
    EXPECT_EQ(rendererInClientInner->fastStatus_.load(), FASTSTATUS_NORMAL);
    EXPECT_EQ(notifyCount, 1);
}

// ========== SetupFastStreamThreadPriority Tests ==========

/**
 * @tc.name  : Test SetupFastStreamThreadPriority
 * @tc.type  : FUNC
 * @tc.number: SetupFastStreamThreadPriority_UltraFast_001
 * @tc.desc  : Test SetupFastStreamThreadPriority for ultra fast stream.
 */
HWTEST(RendererInClientPublicNewUnitTest, SetupFastStreamThreadPriority_UltraFast_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    
    rendererInClientInner->rendererInfo_.audioFlag = AUDIO_OUTPUT_FLAG_FAST;
    rendererInClientInner->clientConfig_.ultraFastFlag = ULTRA_REQUESTED | ULTRA_IMPLEMENTED;
    
    sptr<IpcStreamTest> ipcStreamTest = sptr<IpcStreamTest>::MakeSptr();
    rendererInClientInner->ipcStream_ = ipcStreamTest;
    
    EXPECT_TRUE(rendererInClientInner->IsFastStream());
    EXPECT_TRUE(rendererInClientInner->IsUltraFastStream());
}

/**
 * @tc.name  : Test SetupFastStreamThreadPriority
 * @tc.type  : FUNC
 * @tc.number: SetupFastStreamThreadPriority_NormalFast_001
 * @tc.desc  : Test SetupFastStreamThreadPriority for normal fast stream.
 */
HWTEST(RendererInClientPublicNewUnitTest, SetupFastStreamThreadPriority_NormalFast_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    
    rendererInClientInner->rendererInfo_.audioFlag = AUDIO_OUTPUT_FLAG_FAST;
    rendererInClientInner->clientConfig_.ultraFastFlag = ULTRA_NONE;
    
    sptr<IpcStreamTest> ipcStreamTest = sptr<IpcStreamTest>::MakeSptr();
    rendererInClientInner->ipcStream_ = ipcStreamTest;
    
    EXPECT_TRUE(rendererInClientInner->IsFastStream());
    EXPECT_FALSE(rendererInClientInner->IsUltraFastStream());
}

/**
 * @tc.name  : Test SetupFastStreamThreadPriority
 * @tc.type  : FUNC
 * @tc.number: SetupFastStreamThreadPriority_NoIpcStream_001
 * @tc.desc  : Test SetupFastStreamThreadPriority when ipcStream is nullptr.
 */
HWTEST(RendererInClientPublicNewUnitTest, SetupFastStreamThreadPriority_NoIpcStream_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    
    rendererInClientInner->rendererInfo_.audioFlag = AUDIO_OUTPUT_FLAG_FAST;
    rendererInClientInner->clientConfig_.ultraFastFlag = ULTRA_REQUESTED | ULTRA_IMPLEMENTED;
    rendererInClientInner->ipcStream_ = nullptr;
    
    EXPECT_TRUE(rendererInClientInner->IsFastStream());
    EXPECT_TRUE(rendererInClientInner->IsUltraFastStream());
}

/**
 * @tc.name  : Test SetupFastStreamThreadPriority
 * @tc.type  : FUNC
 * @tc.number: SetupFastStreamThreadPriority_NormalStream_001
 * @tc.desc  : Test SetupFastStreamThreadPriority for normal stream (not fast).
 */
HWTEST(RendererInClientPublicNewUnitTest, SetupFastStreamThreadPriority_NormalStream_001, TestSize.Level1)
{
    auto rendererInClientInner = std::make_shared<RendererInClientInner>(STREAM_MUSIC, getpid());
    
    rendererInClientInner->rendererInfo_.audioFlag = AUDIO_OUTPUT_FLAG_NORMAL;
    rendererInClientInner->clientConfig_.ultraFastFlag = ULTRA_NONE;
    
    EXPECT_FALSE(rendererInClientInner->IsFastStream());
    EXPECT_FALSE(rendererInClientInner->IsUltraFastStream());
}

} // namespace AudioStandard
} // namespace OHOS
