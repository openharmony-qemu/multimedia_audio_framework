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

#include "gtest/gtest.h"
#include "audio_errors.h"
#include "audio_info.h"
#include "renderer_in_client.h"
#include "renderer_in_client_private.h"
#include "i_stream_listener.h"
#include "meta/audio_types.h"
#include "oh_audio_buffer.h"


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

    virtual int32_t UpdatePlaybackCaptureConfig(const AudioPlaybackCaptureConfig &config) override { return 0; }

    virtual int32_t GetAudioTime(uint64_t &framePos, uint64_t &timestamp) override { return 0; }

    virtual int32_t GetAudioPosition(uint64_t &framePos, uint64_t &timestamp, uint64_t &latency, int32_t base) override
    {
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

    virtual int32_t SetRate(int32_t rate) override { return 0; } // SetRenderRate

    virtual int32_t GetRate(int32_t &rate) override { return 0; } // SetRenderRate

    virtual int32_t SetLowPowerVolume(float volume) override { return 0; } // renderer only

    virtual int32_t GetLowPowerVolume(float &volume) override { return 0; } // renderer only

    virtual int32_t SetAudioEffectMode(int32_t effectMode) override { return 0; } // renderer only

    virtual int32_t GetAudioEffectMode(int32_t &effectMode) override { return 0; } // renderer only

    virtual int32_t SetPrivacyType(int32_t privacyType) override { return 0; } // renderer only

    virtual int32_t GetPrivacyType(int32_t &privacyType) override { return 0; } // renderer only

    virtual int32_t SetOffloadMode(int32_t state, bool isAppBack) override { return 0; } // renderer only

    virtual int32_t UnsetOffloadMode() override { return 0; } // renderer only

    virtual int32_t GetOffloadApproximatelyCacheTime(uint64_t &timestamp, uint64_t &paWriteIndex,
        uint64_t &cacheTimeDsp, uint64_t &cacheTimePa) override { return 0; } // renderer only

    virtual int32_t UpdateSpatializationState(bool spatializationEnabled, bool headTrackingEnabled) override
    {
        return 0;
    }

    virtual int32_t GetStreamManagerType() override { return 0; }

    virtual int32_t SetSilentModeAndMixWithOthers(bool on) override { return 0; }

    virtual int32_t SetClientVolume() override { return 0; }

    virtual int32_t SetLoudnessGain(float loudnessGain) override { return 0; }

    virtual int32_t SetMute(bool isMute) override { return (isMute ? SUCCESS : ERROR); }

    virtual int32_t SetDuckFactor(float duckFactor) override { return 0; }

    virtual int32_t RegisterThreadPriority(pid_t tid, const std::string &bundleName, uint32_t method) override
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
    auto ret = ptrRendererInClientInner->GetAudioPosition(timestamp, base);
    EXPECT_TRUE(ret);

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

    float speed = 2.0f;
    auto ret = ptrRendererInClientInner->SetSpeed(speed);
    EXPECT_EQ(ret, SUCCESS);
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
    ptrRendererInClientInner->rendererInfo_.pipeType = PIPE_TYPE_OFFLOAD;
    auto ret = ptrRendererInClientInner->RestoreAudioStream(needStoreState);
    EXPECT_EQ(ret, false);

    ptrRendererInClientInner->rendererInfo_.pipeType = PIPE_TYPE_MULTICHANNEL;
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
    auto ret = ptrRendererInClientInner->SetPitch(pitch);
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

    EXPECT_GT(testRendererInClientObj->lastSwitchPosition_[Timestamp::Timestampbase::MONOTONIC], TEST_POSITION);
    EXPECT_GT(testRendererInClientObj->lastSwitchPosition_[Timestamp::Timestampbase::BOOTTIME], TEST_POSITION);

    EXPECT_GT(
        testRendererInClientObj->lastSwitchPositionWithSpeed_[Timestamp::Timestampbase::MONOTONIC], TEST_POSITION
    );
    EXPECT_GT(
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
    ptrRendererInClientInner->unprocessedFramesBytes_.store(500);
    ptrRendererInClientInner->totalBytesWrittenAfterFlush_.store(50);
    for (auto i = 0; i < Timestamp::Timestampbase::BASESIZE; i++) {
        ptrRendererInClientInner->GetAudioTimestampInfo(timestamp,
            static_cast<Timestamp::Timestampbase>(i));
        EXPECT_EQ(timestamp.framePosition, 450); // latency = 50, frameposition = 500 - 50 = 450
    }
    ptrRendererInClientInner->SetSpeed(2.0); // lastspeed = 1.0, speed = 2.0, lastFrameWritten = 50
    ptrRendererInClientInner->totalBytesWrittenAfterFlush_.store(200);
    for (auto i = 0; i < Timestamp::Timestampbase::BASESIZE; i++) {
        ptrRendererInClientInner->GetAudioTimestampInfo(timestamp,
            static_cast<Timestamp::Timestampbase>(i));
        EXPECT_EQ(timestamp.framePosition, 150); // latency = 50 + (200 - 50) * 2 = 350, frameposition = 150 < 450
    }
    ptrRendererInClientInner->unprocessedFramesBytes_.store(1000);
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

    info.format = OHOS::Media::Plugins::AudioSampleFormat::SAMPLE_U8P;
    info.channels = AudioChannel::STEREO;
    ret = ptrRendererInClientInner->SetAudioStreamInfo(info, nullptr);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);

    info.format = AudioSampleFormat::SAMPLE_U8;
    info.channels = AudioChannel::CHANNEL_UNKNOW;
    ret = ptrRendererInClientInner->SetAudioStreamInfo(info, nullptr);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);

    info.encoding = AudioEncodingType::ENCODING_PCM;
    ret = ptrRendererInClientInner->SetAudioStreamInfo(info, nullptr);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);

    info.channels = AudioChannel::STEREO;
    ptrRendererInClientInner->state_ = State::INVALID;
    ret = ptrRendererInClientInner->SetAudioStreamInfo(info, nullptr);
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
    float volume = -0.1f;
    int32_t ret = ptrRendererInClientInner->SetDuckVolume(volume);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    volume = 1.1f;
    ret = ptrRendererInClientInner->SetDuckVolume(volume);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
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

    ptrRendererInClientInner->rendererInfo_.pipeType = PIPE_TYPE_OFFLOAD;
    EXPECT_FALSE(ptrRendererInClientInner->RestoreAudioStream(needStoreState));
}

/**
 * @tc.name  : Test RendererInClientInner API
 * @tc.type  : FUNC
 * @tc.number: RendererInClientInner_084
 * @tc.desc  : Test RendererInClientInner::RestoreAudioStream
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_084, TestSize.Level1)
{
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_DEFAULT, getpid());
    ptrRendererInClientInner->proxyObj_ = std::make_shared<AudioClientTrackerTest>();
    ptrRendererInClientInner->streamParams_.samplingRate = AudioSamplingRate::SAMPLE_RATE_8000,
    ptrRendererInClientInner->streamParams_.encoding = AudioEncodingType::ENCODING_AUDIOVIVID,
    ptrRendererInClientInner->streamParams_.format = AudioSampleFormat::SAMPLE_U8,
    ptrRendererInClientInner->streamParams_.channels = AudioChannel::STEREO,
    ptrRendererInClientInner->state_ = RUNNING;
    bool needStoreState = false;
    EXPECT_TRUE(ptrRendererInClientInner->RestoreAudioStream(needStoreState));

    needStoreState = true;
    ptrRendererInClientInner->isDataLinkConnected_ = true;
    ptrRendererInClientInner->ipcStream_ = new(std::nothrow) IpcStreamTest();
    EXPECT_TRUE(ptrRendererInClientInner->RestoreAudioStream(needStoreState));

    ptrRendererInClientInner->state_ = PAUSED;
    EXPECT_FALSE(ptrRendererInClientInner->RestoreAudioStream(needStoreState));

    ptrRendererInClientInner->state_ = STOPPED;
    EXPECT_FALSE(ptrRendererInClientInner->RestoreAudioStream(needStoreState));

    ptrRendererInClientInner->state_ = STOPPING;
    EXPECT_FALSE(ptrRendererInClientInner->RestoreAudioStream(needStoreState));

    ptrRendererInClientInner->state_ = INVALID;
    EXPECT_TRUE(ptrRendererInClientInner->RestoreAudioStream(needStoreState));
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
    ptrRendererInClientInner->engineTotalSizeInFrame_ = 2;
    ptrRendererInClientInner->cbBufferSize_ = 1;

    // Readable == enginesizeinframe
    ptrRendererInClientInner->clientBuffer_->SetCurWriteFrame(2);
    bool ret = ptrRendererInClientInner->CheckBufferNeedWrite();

    EXPECT_EQ(ret, true);
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
    ptrRendererInClientInner->engineTotalSizeInFrame_ = 2;
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
    ptrRendererInClientInner->engineTotalSizeInFrame_ = 2;
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
    ptrRendererInClientInner->WaitForBufferNeedWrite();
    EXPECT_EQ(ptrRendererInClientInner->IsRestoreNeeded(), true);

    ptrRendererInClientInner->clientBuffer_->basicBufferInfo_->restoreStatus.store(NEED_RESTORE_TO_NORMAL);
    ptrRendererInClientInner->WaitForBufferNeedWrite();
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
} // namespace AudioStandard
} // namespace OHOS
