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

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
class RendererInClientUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

class IpcStreamTest : public IpcStream {
public:
    virtual ~IpcStreamTest() = default;

    virtual int32_t RegisterStreamListener(sptr<IRemoteObject> object) { return 0; }

    virtual int32_t ResolveBuffer(std::shared_ptr<OHAudioBuffer> &buffer) { return 0; }

    virtual int32_t UpdatePosition() { return 0; }

    virtual int32_t GetAudioSessionID(uint32_t &sessionId) { return 0; }

    virtual int32_t Start() { return 0; }

    virtual int32_t Pause() { return 0; }

    virtual int32_t Stop() { return 0; }

    virtual int32_t Release() { return 0; }

    virtual int32_t Flush() { return 0; }

    virtual int32_t Drain(bool stopFlag = false) { return 0; }

    virtual int32_t UpdatePlaybackCaptureConfig(const AudioPlaybackCaptureConfig &config) { return 0; }

    virtual int32_t GetAudioTime(uint64_t &framePos, uint64_t &timestamp) { return 0; }

    virtual int32_t GetAudioPosition(uint64_t &framePos, uint64_t &timestamp, uint64_t &latency, int32_t base)
    {
        return 0;
    }

    virtual int32_t GetLatency(uint64_t &latency) { return 0; }

    virtual int32_t SetRate(int32_t rate) { return 0; } // SetRenderRate

    virtual int32_t GetRate(int32_t &rate) { return 0; } // SetRenderRate

    virtual int32_t SetLowPowerVolume(float volume) { return 0; } // renderer only

    virtual int32_t GetLowPowerVolume(float &volume) { return 0; } // renderer only

    virtual int32_t SetAudioEffectMode(int32_t effectMode) { return 0; } // renderer only

    virtual int32_t GetAudioEffectMode(int32_t &effectMode) { return 0; } // renderer only

    virtual int32_t SetPrivacyType(int32_t privacyType) { return 0; } // renderer only

    virtual int32_t GetPrivacyType(int32_t &privacyType) { return 0; } // renderer only

    virtual int32_t SetOffloadMode(int32_t state, bool isAppBack) { return 0; } // renderer only

    virtual int32_t UnsetOffloadMode() { return 0; } // renderer only

    virtual int32_t GetOffloadApproximatelyCacheTime(uint64_t &timestamp, uint64_t &paWriteIndex,
        uint64_t &cacheTimeDsp, uint64_t &cacheTimePa) { return 0; } // renderer only

    virtual int32_t UpdateSpatializationState(bool spatializationEnabled, bool headTrackingEnabled) { return 0; }

    virtual int32_t GetStreamManagerType() { return 0; }

    virtual int32_t SetSilentModeAndMixWithOthers(bool on) { return 0; }

    virtual int32_t SetClientVolume() { return 0; }

    virtual int32_t SetLoudnessGain(float loudnessGain) { return 0; }

    virtual int32_t SetMute(bool isMute) { return (isMute ? SUCCESS : ERROR); }

    virtual int32_t SetDuckFactor(float duckFactor) { return 0; }

    virtual int32_t RegisterThreadPriority(pid_t tid, const std::string &bundleName, BoostTriggerMethod method)
    {
        return 0;
    }

    virtual int32_t SetDefaultOutputDevice(const DeviceType defaultOuputDevice) { return 0; }

    virtual int32_t SetSourceDuration(int64_t duration) { return 0; }

    virtual int32_t SetOffloadDataCallbackState(int32_t state) { return 0; }

    virtual sptr<IRemoteObject> AsObject() { return nullptr; }
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
    uint32_t spanSizeInFrame = 0;
    uint32_t byteSizePerFrame = 0;
    ptrRendererInClientInner->clientBuffer_ = std::make_shared<OHAudioBuffer>(bufferHolder, totalSizeInFrame,
        spanSizeInFrame, byteSizePerFrame);

    ptrRendererInClientInner->ipcStream_ = new(std::nothrow) IpcStreamTest();

    bool mute = true;
    auto ret = ptrRendererInClientInner->SetMute(mute);
    EXPECT_EQ(ret, SUCCESS);

    mute = false;
    ret = ptrRendererInClientInner->SetMute(mute);
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
    uint32_t spanSizeInFrame = 0;
    uint32_t byteSizePerFrame = 0;
    ptrRendererInClientInner->clientBuffer_ = std::make_shared<OHAudioBuffer>(bufferHolder, totalSizeInFrame,
        spanSizeInFrame, byteSizePerFrame);

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
    uint32_t spanSizeInFrame = 0;
    uint32_t byteSizePerFrame = 0;
    ptrRendererInClientInner->clientBuffer_ = std::make_shared<OHAudioBuffer>(bufferHolder, totalSizeInFrame,
        spanSizeInFrame, byteSizePerFrame);

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
    uint32_t spanSizeInFrame = 0;
    uint32_t byteSizePerFrame = 0;
    ptrRendererInClientInner->clientBuffer_ = std::make_shared<OHAudioBuffer>(bufferHolder, totalSizeInFrame,
        spanSizeInFrame, byteSizePerFrame);

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
 * @tc.number: RendererInClientInner_060
 * @tc.desc  : Test RendererInClientInner::GetFastStatus
 */
HWTEST(RendererInClientInnerUnitTest, RendererInClientInner_060, TestSize.Level1)
{
    AudioStreamType eStreamType = AudioStreamType::STREAM_DEFAULT;
    int32_t appUid = 1;
    auto ptrRendererInClientInner = std::make_shared<RendererInClientInner>(eStreamType, appUid);

    ASSERT_TRUE(ptrRendererInClientInner != nullptr);

    auto ret = ptrRendererInClientInner->GetFastStatus();
    EXPECT_EQ(ret, FASTSTATUS_NORMAL);
}
} // namespace AudioStandard
} // namespace OHOS
