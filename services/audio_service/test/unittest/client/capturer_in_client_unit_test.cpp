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

#include <gtest/gtest.h>

#include "audio_service_log.h"
#include "audio_errors.h"
#include "iremote_broker.h"
#include "audio_policy_manager.h"
#include "system_ability_definition.h"
#include "audio_service_types.h"
#include "capturer_in_client_inner.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
const int32_t CLIENT_ID = 10;
const int32_t VALUE_FOUR = 4;
const int32_t VALUE_FIVE = 5;
const int32_t VALUE_SIX = 6;
const int32_t VALUE_EIGHT = 8;
const int32_t VALUE_TEN = 10;
const int32_t VALUE_FIF = 15;
const int32_t VALUE_INVALID = -1;
const int32_t MAX_TIMES = 21;
const size_t SHORT_SLEEP_TIME = 200000; // us 200ms
const uint64_t TEST_POSITION = 20000;
const uint64_t TEST_TIMESTAMP_NS = 20000;

enum {
    STATE_CHANGE_EVENT = 0,
    RENDERER_MARK_REACHED_EVENT,
    RENDERER_PERIOD_REACHED_EVENT,
    CAPTURER_PERIOD_REACHED_EVENT,
    CAPTURER_MARK_REACHED_EVENT,
};

enum : int64_t {
    HANDLER_PARAM_INVALID = -1,
    HANDLER_PARAM_NEW = 0,
    HANDLER_PARAM_PREPARED,
    HANDLER_PARAM_RUNNING,
    HANDLER_PARAM_STOPPED,
    HANDLER_PARAM_RELEASED,
    HANDLER_PARAM_PAUSED,
    HANDLER_PARAM_STOPPING,
    HANDLER_PARAM_RUNNING_FROM_SYSTEM,
    HANDLER_PARAM_PAUSED_FROM_SYSTEM,
};

class CapturerInClientUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
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
    virtual void OnPeriodReached(const int64_t &frameNumber) {};
};

class AudioStreamCallbackTest : public AudioStreamCallback {
public:
    virtual ~AudioStreamCallbackTest() = default;
    /**
     * Called when stream state is updated.
     *
     * @param state Indicates the InterruptEvent information needed by client.
     * For details, refer InterruptEvent struct in audio_info.h
     */
    virtual void OnStateChange(const State state, const StateChangeCmdType cmdType = CMD_FROM_CLIENT) {};
};

class CapturerPositionCallbackTest : public CapturerPositionCallback {
public:
    void OnMarkReached(const int64_t &framePosition) override {};
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

    virtual int32_t GetSpeedPosition(uint64_t &framePos, uint64_t &timestamp, uint64_t &latency,
        int32_t base) override
    {
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

    virtual int32_t RegisterThreadPriority(int32_t tid, const std::string &bundleName, uint32_t method) override
    {
        return 0;
    }

    virtual int32_t SetDefaultOutputDevice(int32_t defaultOuputDevice, bool skipForce = false) override { return 0; }

    virtual int32_t SetSourceDuration(int64_t duration) override { return 0; }

    virtual int32_t SetOffloadDataCallbackState(int32_t state) override { return 0; }

    virtual int32_t SetSpeed(float speed) override { return 0; }

    sptr<IRemoteObject> AsObject() override { return nullptr; }

    virtual int32_t ResolveBufferBaseAndGetServerSpanSize(std::shared_ptr<OHAudioBufferBase> &buffer,
        uint32_t &spanSizeInFrame, uint64_t &engineTotalSizeInFrame) override { return SUCCESS; }

    virtual int32_t SetAudioHapticsSyncId(int32_t audioHapticsSyncId) override { return 0; }
};

class AudioClientTrackerTest : public AudioClientTracker {
public:
    virtual ~AudioClientTrackerTest() = default;
    /**
     * Mute Stream was controlled by system application
     *
     * @param streamSetStateEventInternal Contains the set even information.
     */
    virtual void MuteStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) {};
     /**
     * Unmute Stream was controlled by system application
     *
     * @param streamSetStateEventInternal Contains the set even information.
     */
    virtual void UnmuteStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) {};
    /**
     * Paused Stream was controlled by system application
     *
     * @param streamSetStateEventInternal Contains the set even information.
     */
    virtual void PausedStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) {};
     /**
     * Resumed Stream was controlled by system application
     *
     * @param streamSetStateEventInternal Contains the set even information.
     */
    virtual void ResumeStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) {};
    virtual void SetLowPowerVolumeImpl(float volume) {};
    virtual void GetLowPowerVolumeImpl(float &volume) {};
    virtual void GetSingleStreamVolumeImpl(float &volume) {};
    virtual void SetOffloadModeImpl(int32_t state, bool isAppBack) {};
    virtual void UnsetOffloadModeImpl() {};
};

void Init(std::shared_ptr<CapturerInClientInner> capturerInClientInner)
{
    capturerInClientInner->clientPid_ = CLIENT_ID;
    capturerInClientInner->clientUid_ = CLIENT_ID;
    capturerInClientInner->appTokenId_ = CLIENT_ID;
    capturerInClientInner->fullTokenId_ = CLIENT_ID;
    capturerInClientInner->streamParams_.channels = STEREO;
    capturerInClientInner->streamParams_.encoding = AudioEncodingType::ENCODING_AUDIOVIVID;
    capturerInClientInner->streamParams_.format = SAMPLE_S32LE;
    capturerInClientInner->streamParams_.samplingRate = SAMPLE_RATE_48000;
    capturerInClientInner->streamParams_.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    capturerInClientInner->eStreamType_ = AudioStreamType::STREAM_MUSIC;
    capturerInClientInner->capturerInfo_.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    size_t size = 8 * 1024;
    capturerInClientInner->clientSpanSizeInByte_ = size;
    capturerInClientInner->ringCache_ = std::make_unique<AudioRingCache>(size);
}

/**
 * @tc.name  : Test OnOperationHandled API
 * @tc.type  : FUNC
 * @tc.number: OnOperationHandled_001
 * @tc.desc  : Test OnOperationHandled interface.
 */
HWTEST(CapturerInClientUnitTest, OnOperationHandled_001, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    Operation operation = Operation::UPDATE_STREAM;
    int64_t result = 1;
    int32_t ret = capturerInClientInner_->OnOperationHandled(operation, result);
    EXPECT_EQ(ret, SUCCESS);

    operation = Operation::BUFFER_OVERFLOW;
    ret = capturerInClientInner_->OnOperationHandled(operation, result);
    EXPECT_EQ(ret, SUCCESS);

    operation = Operation::RESTORE_SESSION;
    ret = capturerInClientInner_->OnOperationHandled(operation, result);
    EXPECT_EQ(ret, SUCCESS);

    operation = Operation::START_STREAM;
    ret = capturerInClientInner_->OnOperationHandled(operation, result);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test RegisterTracker API
 * @tc.type  : FUNC
 * @tc.number: RegisterTracker_001
 * @tc.desc  : Test RegisterTracker interface.
 */
HWTEST(CapturerInClientUnitTest, RegisterTracker_001, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    capturerInClientInner_->streamTrackerRegistered_ = false;
    std::shared_ptr<AudioClientTracker> proxyObj;
    capturerInClientInner_->RegisterTracker(proxyObj);
    EXPECT_NE(capturerInClientInner_, nullptr);
}

/**
 * @tc.name  : Test RegisterTracker API
 * @tc.type  : FUNC
 * @tc.number: RegisterTracker_002
 * @tc.desc  : Test RegisterTracker interface.
 */
HWTEST(CapturerInClientUnitTest, RegisterTracker_002, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    capturerInClientInner_->audioStreamTracker_ = nullptr;
    capturerInClientInner_->streamTrackerRegistered_ = true;
    std::shared_ptr<AudioClientTracker> proxyObj;
    capturerInClientInner_->RegisterTracker(proxyObj);
    EXPECT_NE(capturerInClientInner_, nullptr);
}

/**
 * @tc.name  : Test RegisterTracker API
 * @tc.type  : FUNC
 * @tc.number: RegisterTracker_003
 * @tc.desc  : Test RegisterTracker interface.
 */
HWTEST(CapturerInClientUnitTest, RegisterTracker_003, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    capturerInClientInner_->audioStreamTracker_ = nullptr;
    capturerInClientInner_->streamTrackerRegistered_ = false;
    std::shared_ptr<AudioClientTracker> proxyObj;
    capturerInClientInner_->RegisterTracker(proxyObj);
    EXPECT_NE(capturerInClientInner_, nullptr);
}

/**
 * @tc.name  : Test RegisterTracker API
 * @tc.type  : FUNC
 * @tc.number: RegisterTracker_004
 * @tc.desc  : Test RegisterTracker interface.
 */
HWTEST(CapturerInClientUnitTest, RegisterTracker_004, TestSize.Level1)
{
    int32_t clientUid = 0;
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    capturerInClientInner_->audioStreamTracker_ = std::make_unique<AudioStreamTracker>(AudioMode::AUDIO_MODE_PLAYBACK,
        clientUid);
    capturerInClientInner_->streamTrackerRegistered_ = true;
    std::shared_ptr<AudioClientTracker> proxyObj;
    capturerInClientInner_->RegisterTracker(proxyObj);
    EXPECT_NE(capturerInClientInner_, nullptr);
}

/**
 * @tc.name  : Test RegisterTracker API
 * @tc.type  : FUNC
 * @tc.number: RegisterTracker_005
 * @tc.desc  : Test RegisterTracker interface.
 */
HWTEST(CapturerInClientUnitTest, RegisterTracker_005, TestSize.Level1)
{
    int32_t clientUid = 0;
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    capturerInClientInner_->audioStreamTracker_ = std::make_unique<AudioStreamTracker>(AudioMode::AUDIO_MODE_PLAYBACK,
        clientUid);
    capturerInClientInner_->streamTrackerRegistered_ = false;
    std::shared_ptr<AudioClientTracker> proxyObj;
    capturerInClientInner_->RegisterTracker(proxyObj);
    EXPECT_NE(capturerInClientInner_, nullptr);
}

/**
 * @tc.name  : Test UpdateTracker API
 * @tc.type  : FUNC
 * @tc.number: UpdateTracker_001
 * @tc.desc  : Test UpdateTracker interface.
 */
HWTEST(CapturerInClientUnitTest, UpdateTracker_001, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    capturerInClientInner_->audioStreamTracker_ = nullptr;
    std::string updateCase = " ";
    capturerInClientInner_->UpdateTracker(updateCase);
    EXPECT_NE(capturerInClientInner_, nullptr);
}

/**
 * @tc.name  : Test SetAudioStreamInfo API
 * @tc.type  : FUNC
 * @tc.number: SetAudioStreamInfo_001
 * @tc.desc  : Test SetAudioStreamInfo interface.
 */
HWTEST(CapturerInClientUnitTest, SetAudioStreamInfo_001, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    Init(capturerInClientInner_);
    std::shared_ptr<AudioClientTracker> proxyObj;
    AudioStreamParams info;
    info.format = AudioSampleFormat::SAMPLE_U8;
    info.encoding = AudioEncodingType::ENCODING_AUDIOVIVID;
    info.samplingRate = AudioSamplingRate::SAMPLE_RATE_8000;
    info.channels = AudioChannel::MONO;
    info.channelLayout = AudioChannelLayout::CH_LAYOUT_MONO;
    capturerInClientInner_->state_ = NEW;
    int32_t ret = capturerInClientInner_->SetAudioStreamInfo(info, proxyObj);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetAudioStreamInfo API
 * @tc.type  : FUNC
 * @tc.number: SetAudioStreamInfo_002
 * @tc.desc  : Test SetAudioStreamInfo interface.
 */
HWTEST(CapturerInClientUnitTest, SetAudioStreamInfo_002, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    std::shared_ptr<AudioClientTracker> proxyObj;
    AudioStreamParams info;
    info.format = AudioSampleFormat::SAMPLE_F32LE;
    info.encoding = AudioEncodingType::ENCODING_AUDIOVIVID;
    info.samplingRate = AudioSamplingRate::SAMPLE_RATE_8000;
    info.channels = AudioChannel::MONO;
    info.channelLayout = AudioChannelLayout::CH_LAYOUT_MONO;
    capturerInClientInner_->state_ = NEW;
    int32_t ret = capturerInClientInner_->SetAudioStreamInfo(info, proxyObj);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetAudioStreamInfo API
 * @tc.type  : FUNC
 * @tc.number: SetAudioStreamInfo_003
 * @tc.desc  : Test SetAudioStreamInfo interface.
 */
HWTEST(CapturerInClientUnitTest, SetAudioStreamInfo_003, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    std::shared_ptr<AudioClientTracker> proxyObj;
    AudioStreamParams info;
    info.format = AudioSampleFormat::SAMPLE_F32LE;
    info.encoding = AudioEncodingType::ENCODING_INVALID;
    info.samplingRate = AudioSamplingRate::SAMPLE_RATE_8000;
    info.channels = AudioChannel::MONO;
    info.channelLayout = AudioChannelLayout::CH_LAYOUT_MONO;
    capturerInClientInner_->state_ = NEW;
    int32_t ret = capturerInClientInner_->SetAudioStreamInfo(info, proxyObj);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

/**
 * @tc.name  : Test SetAudioStreamInfo API
 * @tc.type  : FUNC
 * @tc.number: SetAudioStreamInfo_004
 * @tc.desc  : Test SetAudioStreamInfo interface.
 */
HWTEST(CapturerInClientUnitTest, SetAudioStreamInfo_004, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    std::shared_ptr<AudioClientTracker> proxyObj;
    AudioStreamParams info;
    info.format = AudioSampleFormat::SAMPLE_F32LE;
    info.encoding = AudioEncodingType::ENCODING_INVALID;
    info.samplingRate = static_cast<AudioSamplingRate>(600);
    info.channels = AudioChannel::MONO;
    info.channelLayout = AudioChannelLayout::CH_LAYOUT_MONO;
    capturerInClientInner_->state_ = NEW;
    int32_t ret = capturerInClientInner_->SetAudioStreamInfo(info, proxyObj);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

/**
 * @tc.name  : Test SetAudioStreamInfo API
 * @tc.type  : FUNC
 * @tc.number: SetAudioStreamInfo_005
 * @tc.desc  : Test SetAudioStreamInfo interface.
 */
HWTEST(CapturerInClientUnitTest, SetAudioStreamInfo_005, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    std::shared_ptr<AudioClientTracker> proxyObj;
    AudioStreamParams info;
    info.format = AudioSampleFormat::SAMPLE_F32LE;
    info.encoding = AudioEncodingType::ENCODING_AUDIOVIVID;
    info.samplingRate = static_cast<AudioSamplingRate>(600);
    info.channels = AudioChannel::MONO;
    info.channelLayout = AudioChannelLayout::CH_LAYOUT_MONO;
    capturerInClientInner_->state_ = NEW;
    int32_t ret = capturerInClientInner_->SetAudioStreamInfo(info, proxyObj);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

/**
 * @tc.name  : Test SetAudioStreamInfo API
 * @tc.type  : FUNC
 * @tc.number: SetAudioStreamInfo_006
 * @tc.desc  : Test SetAudioStreamInfo interface.
 */
HWTEST(CapturerInClientUnitTest, SetAudioStreamInfo_006, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    std::shared_ptr<AudioClientTracker> proxyObj;
    AudioStreamParams info;
    info.format = AudioSampleFormat::SAMPLE_U8;
    info.encoding = AudioEncodingType::ENCODING_INVALID;
    info.samplingRate = static_cast<AudioSamplingRate>(600);
    info.channels = AudioChannel::MONO;
    info.channelLayout = AudioChannelLayout::CH_LAYOUT_MONO;
    capturerInClientInner_->state_ = NEW;
    int32_t ret = capturerInClientInner_->SetAudioStreamInfo(info, proxyObj);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

/**
 * @tc.name  : Test SetAudioStreamInfo API
 * @tc.type  : FUNC
 * @tc.number: SetAudioStreamInfo_007
 * @tc.desc  : Test SetAudioStreamInfo interface.
 */
HWTEST(CapturerInClientUnitTest, SetAudioStreamInfo_007, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    std::shared_ptr<AudioClientTracker> proxyObj;
    AudioStreamParams info;
    info.format = AudioSampleFormat::SAMPLE_U8;
    info.encoding = AudioEncodingType::ENCODING_INVALID;
    info.samplingRate = AudioSamplingRate::SAMPLE_RATE_8000;
    info.channels = AudioChannel::MONO;
    info.channelLayout = AudioChannelLayout::CH_LAYOUT_MONO;
    capturerInClientInner_->state_ = NEW;
    int32_t ret = capturerInClientInner_->SetAudioStreamInfo(info, proxyObj);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

/**
 * @tc.name  : Test SetAudioStreamInfo API
 * @tc.type  : FUNC
 * @tc.number: SetAudioStreamInfo_008
 * @tc.desc  : Test SetAudioStreamInfo interface.
 */
HWTEST(CapturerInClientUnitTest, SetAudioStreamInfo_008, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    std::shared_ptr<AudioClientTracker> proxyObj;
    AudioStreamParams info;
    info.format = AudioSampleFormat::SAMPLE_U8;
    info.encoding = AudioEncodingType::ENCODING_AUDIOVIVID;
    info.samplingRate = static_cast<AudioSamplingRate>(600);
    info.channels = AudioChannel::MONO;
    info.channelLayout = AudioChannelLayout::CH_LAYOUT_MONO;
    capturerInClientInner_->state_ = NEW;
    int32_t ret = capturerInClientInner_->SetAudioStreamInfo(info, proxyObj);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

/**
 * @tc.name  : Test SetAudioStreamInfo API
 * @tc.type  : FUNC
 * @tc.number: SetAudioStreamInfo_009
 * @tc.desc  : Test SetAudioStreamInfo interface.
 */
HWTEST(CapturerInClientUnitTest, SetAudioStreamInfo_009, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    std::shared_ptr<AudioClientTracker> proxyObj;
    AudioStreamParams info;
    info.format = AudioSampleFormat::SAMPLE_U8;
    info.encoding = AudioEncodingType::ENCODING_AUDIOVIVID;
    info.samplingRate = AudioSamplingRate::SAMPLE_RATE_8000;
    info.channels = AudioChannel::CHANNEL_7;
    info.channelLayout = static_cast<AudioChannelLayout>(99999);
    capturerInClientInner_->state_ = NEW;
    int32_t ret = capturerInClientInner_->SetAudioStreamInfo(info, proxyObj);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

/**
 * @tc.name  : Test SetAudioStreamInfo API
 * @tc.type  : FUNC
 * @tc.number: SetAudioStreamInfo_010
 * @tc.desc  : Test SetAudioStreamInfo interface.
 */
HWTEST(CapturerInClientUnitTest, SetAudioStreamInfo_010, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    Init(capturerInClientInner_);
    std::shared_ptr<AudioClientTracker> proxyObj;
    AudioStreamParams info;
    info.format = AudioSampleFormat::SAMPLE_U8;
    info.encoding = AudioEncodingType::ENCODING_AUDIOVIVID;
    info.samplingRate = AudioSamplingRate::SAMPLE_RATE_8000;
    info.channels = AudioChannel::MONO;
    info.channelLayout = AudioChannelLayout::CH_LAYOUT_MONO;
    capturerInClientInner_->state_ = INVALID;
    int32_t ret = capturerInClientInner_->SetAudioStreamInfo(info, proxyObj);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test GetAudioServerProxy API
 * @tc.type  : FUNC
 * @tc.number: GetAudioServerProxy_001
 * @tc.desc  : Test GetAudioServerProxy interface.
 */
HWTEST(CapturerInClientUnitTest, GetAudioServerProxy_001, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    Init(capturerInClientInner_);
    capturerInClientInner_->GetAudioServerProxy();
    capturerInClientInner_->GetAudioServerProxy();
    EXPECT_NE(capturerInClientInner_, nullptr);
}

/**
 * @tc.name  : Test OnHandle API
 * @tc.type  : FUNC
 * @tc.number: OnHandle_001
 * @tc.desc  : Test OnHandle interface.
 */
HWTEST(CapturerInClientUnitTest, OnHandle_001, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    Init(capturerInClientInner_);
    uint32_t code = RENDERER_PERIOD_REACHED_EVENT;
    int64_t data = 0;
    capturerInClientInner_->OnHandle(code, data);
    EXPECT_NE(capturerInClientInner_, nullptr);

    code = CAPTURER_PERIOD_REACHED_EVENT;
    capturerInClientInner_->OnHandle(code, data);
    EXPECT_NE(capturerInClientInner_, nullptr);
}

/**
 * @tc.name  : Test HandleCapturerMarkReachedEvent API
 * @tc.type  : FUNC
 * @tc.number: HandleCapturerMarkReachedEvent_001
 * @tc.desc  : Test HandleCapturerMarkReachedEvent interface.
 */
HWTEST(CapturerInClientUnitTest, HandleCapturerMarkReachedEvent_001, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    Init(capturerInClientInner_);
    capturerInClientInner_->capturerPositionCallback_ = nullptr;
    int64_t data = 16;
    capturerInClientInner_->HandleCapturerMarkReachedEvent(data);
    EXPECT_NE(capturerInClientInner_, nullptr);
}

/**
 * @tc.name  : Test HandleCapturerMarkReachedEvent API
 * @tc.type  : FUNC
 * @tc.number: HandleCapturerMarkReachedEvent_002
 * @tc.desc  : Test HandleCapturerMarkReachedEvent interface.
 */
HWTEST(CapturerInClientUnitTest, HandleCapturerMarkReachedEvent_002, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    Init(capturerInClientInner_);
    capturerInClientInner_->capturerPositionCallback_ = std::make_shared<CapturerPositionCallbackTest>();
    ASSERT_TRUE(capturerInClientInner_->capturerPositionCallback_ != nullptr);
    int64_t data = 16;
    capturerInClientInner_->HandleCapturerMarkReachedEvent(data);
    EXPECT_NE(capturerInClientInner_, nullptr);
}

/**
 * @tc.name  : Test HandleCapturerPeriodReachedEvent API
 * @tc.type  : FUNC
 * @tc.number: HandleCapturerPeriodReachedEvent_001
 * @tc.desc  : Test HandleCapturerPeriodReachedEvent interface.
 */
HWTEST(CapturerInClientUnitTest, HandleCapturerPeriodReachedEvent_001, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    Init(capturerInClientInner_);
    capturerInClientInner_->capturerPeriodPositionCallback_ = nullptr;
    int64_t data = 16;
    capturerInClientInner_->HandleCapturerPeriodReachedEvent(data);
    EXPECT_NE(capturerInClientInner_, nullptr);
}

/**
 * @tc.name  : Test HandleCapturerPeriodReachedEvent API
 * @tc.type  : FUNC
 * @tc.number: HandleCapturerPeriodReachedEvent_002
 * @tc.desc  : Test HandleCapturerPeriodReachedEvent interface.
 */
HWTEST(CapturerInClientUnitTest, HandleCapturerPeriodReachedEvent_002, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    Init(capturerInClientInner_);
    auto callback = std::make_shared<CapturerPeriodPositionCallbackTest>();
    int64_t data = 16;
    capturerInClientInner_->SetCapturerPeriodPositionCallback(data, callback);
    capturerInClientInner_->HandleCapturerPeriodReachedEvent(data);
    EXPECT_NE(capturerInClientInner_, nullptr);
}

/**
 * @tc.name  : Test ParamsToStateCmdType API
 * @tc.type  : FUNC
 * @tc.number: ParamsToStateCmdType_001
 * @tc.desc  : Test ParamsToStateCmdType interface.
 */
HWTEST(CapturerInClientUnitTest, ParamsToStateCmdType_001, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    Init(capturerInClientInner_);
    int64_t params = HANDLER_PARAM_NEW;
    State state;
    StateChangeCmdType cmdType;
    int32_t ret = capturerInClientInner_->ParamsToStateCmdType(params, state, cmdType);
    EXPECT_EQ(state, NEW);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test ParamsToStateCmdType API
 * @tc.type  : FUNC
 * @tc.number: ParamsToStateCmdType_002
 * @tc.desc  : Test ParamsToStateCmdType interface.
 */
HWTEST(CapturerInClientUnitTest, ParamsToStateCmdType_002, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    Init(capturerInClientInner_);
    int64_t params = HANDLER_PARAM_RELEASED;
    State state;
    StateChangeCmdType cmdType;
    int32_t ret = capturerInClientInner_->ParamsToStateCmdType(params, state, cmdType);
    EXPECT_EQ(state, RELEASED);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test ParamsToStateCmdType API
 * @tc.type  : FUNC
 * @tc.number: ParamsToStateCmdType_003
 * @tc.desc  : Test ParamsToStateCmdType interface.
 */
HWTEST(CapturerInClientUnitTest, ParamsToStateCmdType_003, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    Init(capturerInClientInner_);
    int64_t params = HANDLER_PARAM_STOPPING;
    State state;
    StateChangeCmdType cmdType;
    int32_t ret = capturerInClientInner_->ParamsToStateCmdType(params, state, cmdType);
    EXPECT_EQ(state, STOPPING);
    EXPECT_EQ(ret, SUCCESS);

    params = HANDLER_PARAM_RUNNING_FROM_SYSTEM;
    ret = capturerInClientInner_->ParamsToStateCmdType(params, state, cmdType);
    EXPECT_EQ(cmdType, CMD_FROM_SYSTEM);
    EXPECT_EQ(ret, SUCCESS);

    params = HANDLER_PARAM_PAUSED_FROM_SYSTEM;
    ret = capturerInClientInner_->ParamsToStateCmdType(params, state, cmdType);
    EXPECT_EQ(cmdType, CMD_FROM_SYSTEM);
    EXPECT_EQ(ret, SUCCESS);

    params = HANDLER_PARAM_INVALID;
    ret = capturerInClientInner_->ParamsToStateCmdType(params, state, cmdType);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test ParamsToStateCmdType API
 * @tc.type  : FUNC
 * @tc.number: ParamsToStateCmdType_004
 * @tc.desc  : Test ParamsToStateCmdType interface.
 */
HWTEST(CapturerInClientUnitTest, ParamsToStateCmdType_004, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    Init(capturerInClientInner_);
    int64_t params = HANDLER_PARAM_PAUSED;
    State state;
    StateChangeCmdType cmdType;
    int32_t ret = capturerInClientInner_->ParamsToStateCmdType(params, state, cmdType);
    EXPECT_EQ(state, PAUSED);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test StateCmdTypeToParams API
 * @tc.type  : FUNC
 * @tc.number: StateCmdTypeToParams_001
 * @tc.desc  : Test StateCmdTypeToParams interface.
 */
HWTEST(CapturerInClientUnitTest, StateCmdTypeToParams_001, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    Init(capturerInClientInner_);
    int64_t params;
    State state = State::RUNNING;
    StateChangeCmdType cmdType = CMD_FROM_SYSTEM;
    int32_t ret = capturerInClientInner_->StateCmdTypeToParams(params, state, cmdType);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(params, HANDLER_PARAM_RUNNING_FROM_SYSTEM);
}

/**
 * @tc.name  : Test StateCmdTypeToParams API
 * @tc.type  : FUNC
 * @tc.number: StateCmdTypeToParams_001
 * @tc.desc  : Test StateCmdTypeToParams interface.
 */
HWTEST(CapturerInClientUnitTest, StateCmdTypeToParams_002, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    Init(capturerInClientInner_);
    int64_t params;
    State state = State::PAUSED;
    StateChangeCmdType cmdType = CMD_FROM_SYSTEM;
    int32_t ret = capturerInClientInner_->StateCmdTypeToParams(params, state, cmdType);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(params, HANDLER_PARAM_PAUSED_FROM_SYSTEM);
}

/**
 * @tc.name  : Test StateCmdTypeToParams API
 * @tc.type  : FUNC
 * @tc.number: StateCmdTypeToParams_001
 * @tc.desc  : Test StateCmdTypeToParams interface.
 */
HWTEST(CapturerInClientUnitTest, StateCmdTypeToParams_003, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    Init(capturerInClientInner_);
    int64_t params;
    State state = State::NEW;
    StateChangeCmdType cmdType = CMD_FROM_SYSTEM;
    int32_t ret = capturerInClientInner_->StateCmdTypeToParams(params, state, cmdType);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(params, HANDLER_PARAM_INVALID);
}

/**
 * @tc.name  : Test ConstructConfig API
 * @tc.type  : FUNC
 * @tc.number: ConstructConfig_001
 * @tc.desc  : Test ConstructConfig interface.
 */
HWTEST(CapturerInClientUnitTest, ConstructConfig_001, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    Init(capturerInClientInner_);
    capturerInClientInner_->capturerInfo_.capturerFlags = 1;
    AudioProcessConfig ret = capturerInClientInner_->ConstructConfig();
    EXPECT_NE(capturerInClientInner_, nullptr);
}

/**
 * @tc.name  : Test InitCacheBuffer API
 * @tc.type  : FUNC
 * @tc.number: InitCacheBuffer_001
 * @tc.desc  : Test InitCacheBuffer interface.
 */
HWTEST(CapturerInClientUnitTest, InitCacheBuffer_001, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    Init(capturerInClientInner_);
    size_t size = 32 * 1024 * 1024;
    int32_t ret = capturerInClientInner_->InitCacheBuffer(size);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}


/**
 * @tc.name  : Test GetBufferSize API
 * @tc.type  : FUNC
 * @tc.number: GetBufferSize_001
 * @tc.desc  : Test GetBufferSize interface.
 */
HWTEST(CapturerInClientUnitTest, GetBufferSize_001, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    Init(capturerInClientInner_);
    capturerInClientInner_->state_ = State::NEW;
    capturerInClientInner_->capturerMode_ = CAPTURE_MODE_CALLBACK;
    size_t bufferSize = 0;
    int32_t ret = capturerInClientInner_->GetBufferSize(bufferSize);
    EXPECT_EQ(bufferSize, capturerInClientInner_->cbBufferSize_);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetStreamCallback API
 * @tc.type  : FUNC
 * @tc.number: SetStreamCallback_001
 * @tc.desc  : Test SetStreamCallback interface.
 */
HWTEST(CapturerInClientUnitTest, SetStreamCallback_001, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    auto callback = std::make_shared<AudioStreamCallbackTest>();
    callback = nullptr;
    int32_t ret = capturerInClientInner_->SetStreamCallback(callback);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test SetStreamCallback API
 * @tc.type  : FUNC
 * @tc.number: SetStreamCallback_002
 * @tc.desc  : Test SetStreamCallback interface.
 */
HWTEST(CapturerInClientUnitTest, SetStreamCallback_002, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    auto callback = std::make_shared<AudioStreamCallbackTest>();
    capturerInClientInner_->state_ = State::RUNNING;
    int32_t ret = capturerInClientInner_->SetStreamCallback(callback);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test InitCallbackBuffer API
 * @tc.type  : FUNC
 * @tc.number: InitCallbackBuffer_001
 * @tc.desc  : Test InitCallbackBuffer interface.
 */
HWTEST(CapturerInClientUnitTest, InitCallbackBuffer_001, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    u_int64_t bufferDurationInUs = 3000000;
    capturerInClientInner_->InitCallbackBuffer(bufferDurationInUs);
    EXPECT_NE(capturerInClientInner_, nullptr);
}

/**
 * @tc.name  : Test SetCaptureMode API
 * @tc.type  : FUNC
 * @tc.number: SetCaptureMode_001
 * @tc.desc  : Test SetCaptureMode interface.
 */
HWTEST(CapturerInClientUnitTest, SetCaptureMode_001, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    capturerInClientInner_->capturerMode_ = AudioCaptureMode::CAPTURE_MODE_NORMAL;
    AudioCaptureMode captureMode = AudioCaptureMode::CAPTURE_MODE_NORMAL;
    int32_t ret = capturerInClientInner_->SetCaptureMode(captureMode);
    usleep(SHORT_SLEEP_TIME);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetCaptureMode API
 * @tc.type  : FUNC
 * @tc.number: SetCaptureMode_002
 * @tc.desc  : Test SetCaptureMode interface.
 */
HWTEST(CapturerInClientUnitTest, SetCaptureMode_002, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    capturerInClientInner_->state_ = State::INVALID;
    capturerInClientInner_->capturerMode_ = AudioCaptureMode::CAPTURE_MODE_NORMAL;
    AudioCaptureMode captureMode = AudioCaptureMode::CAPTURE_MODE_CALLBACK;
    int32_t ret = capturerInClientInner_->SetCaptureMode(captureMode);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);

    capturerInClientInner_->capturerMode_ = AudioCaptureMode::CAPTURE_MODE_CALLBACK;
    captureMode = AudioCaptureMode::CAPTURE_MODE_NORMAL;
    ret = capturerInClientInner_->SetCaptureMode(captureMode);
    usleep(SHORT_SLEEP_TIME);
    EXPECT_EQ(ret, ERR_INCORRECT_MODE);
}

/**
 * @tc.name  : Test SetCaptureMode API
 * @tc.type  : FUNC
 * @tc.number: SetCaptureMode_003
 * @tc.desc  : Test SetCaptureMode interface.
 */
HWTEST(CapturerInClientUnitTest, SetCaptureMode_003, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    capturerInClientInner_->capturerMode_ = AudioCaptureMode::CAPTURE_MODE_NORMAL;
    AudioCaptureMode captureMode = AudioCaptureMode::CAPTURE_MODE_CALLBACK;
    capturerInClientInner_->state_ = State::NEW;
    capturerInClientInner_->streamParams_.samplingRate = SAMPLE_RATE_11025;
    capturerInClientInner_->spanSizeInFrame_ = 1;
    int32_t ret = capturerInClientInner_->SetCaptureMode(captureMode);
    EXPECT_EQ(ret, SUCCESS);

    capturerInClientInner_->state_ = State::PREPARED;
    ret = capturerInClientInner_->SetCaptureMode(captureMode);
    usleep(SHORT_SLEEP_TIME);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetCaptureMode API
 * @tc.type  : FUNC
 * @tc.number: SetCaptureMode_004
 * @tc.desc  : Test SetCaptureMode interface.
 */
HWTEST(CapturerInClientUnitTest, SetCaptureMode_004, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    capturerInClientInner_->capturerMode_ = AudioCaptureMode::CAPTURE_MODE_NORMAL;
    AudioCaptureMode captureMode = AudioCaptureMode::CAPTURE_MODE_CALLBACK;
    capturerInClientInner_->state_ = State::PREPARED;
    capturerInClientInner_->streamParams_.samplingRate = SAMPLE_RATE_11025;
    capturerInClientInner_->spanSizeInFrame_ = 1;
    int32_t ret = capturerInClientInner_->SetCaptureMode(captureMode);
    usleep(SHORT_SLEEP_TIME);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetCaptureMode API
 * @tc.type  : FUNC
 * @tc.number: SetCaptureMode_005
 * @tc.desc  : Test SetCaptureMode interface.
 */
HWTEST(CapturerInClientUnitTest, SetCaptureMode_005, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    capturerInClientInner_->capturerMode_ = AudioCaptureMode::CAPTURE_MODE_NORMAL;
    AudioCaptureMode captureMode = AudioCaptureMode::CAPTURE_MODE_CALLBACK;
    capturerInClientInner_->state_ = State::NEW;
    capturerInClientInner_->streamParams_.samplingRate = SAMPLE_RATE_11025;
    capturerInClientInner_->cbThreadReleased_ = true;
    capturerInClientInner_->spanSizeInFrame_ = 1;
    int32_t ret = capturerInClientInner_->SetCaptureMode(captureMode);
    usleep(SHORT_SLEEP_TIME);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test ReadCallbackFunc API
 * @tc.type  : FUNC
 * @tc.number: ReadCallbackFunc_001
 * @tc.desc  : Test ReadCallbackFunc interface.
 */
HWTEST(CapturerInClientUnitTest, ReadCallbackFunc_001, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    BufferDesc temp;
    temp.buffer = nullptr;
    capturerInClientInner_->cbBufferQueue_.Push(temp);
    capturerInClientInner_->state_ = State::RUNNING;
    auto ret = capturerInClientInner_->ReadCallbackFunc();
    EXPECT_NE(capturerInClientInner_, nullptr);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test ReadCallbackFunc API
 * @tc.type  : FUNC
 * @tc.number: ReadCallbackFunc_002
 * @tc.desc  : Test ReadCallbackFunc interface.
 */
HWTEST(CapturerInClientUnitTest, ReadCallbackFunc_002, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    BufferDesc temp;
    temp.buffer = nullptr;
    capturerInClientInner_->cbBufferQueue_.Push(temp);
    capturerInClientInner_->state_ = State::RUNNING;
    capturerInClientInner_->cbThreadReleased_ = false;
    auto ret = capturerInClientInner_->ReadCallbackFunc();
    EXPECT_NE(capturerInClientInner_, nullptr);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test ReadCallbackFunc API
 * @tc.type  : FUNC
 * @tc.number: ReadCallbackFunc_003
 * @tc.desc  : Test ReadCallbackFunc interface.
 */
HWTEST(CapturerInClientUnitTest, ReadCallbackFunc_003, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    capturerInClientInner_->state_ = State::STOPPED;
    capturerInClientInner_->cbThreadReleased_ = false;
    auto ret = capturerInClientInner_->ReadCallbackFunc();
    EXPECT_NE(capturerInClientInner_, nullptr);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test ReadCallbackFunc API
 * @tc.type  : FUNC
 * @tc.number: ReadCallbackFunc_004
 * @tc.desc  : Test ReadCallbackFunc interface.
 */
HWTEST(CapturerInClientUnitTest, ReadCallbackFunc_004, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    BufferDesc temp;
    uint8_t bufferTest = 10;
    temp.buffer = &bufferTest;
    capturerInClientInner_->cbBufferQueue_.Push(temp);
    capturerInClientInner_->state_ = State::RUNNING;
    capturerInClientInner_->cbThreadReleased_ = false;
    auto ret = capturerInClientInner_->ReadCallbackFunc();
    EXPECT_NE(capturerInClientInner_, nullptr);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test GetBufferDesc API
 * @tc.type  : FUNC
 * @tc.number: GetBufferDesc_001
 * @tc.desc  : Test GetBufferDesc interface.
 */
HWTEST(CapturerInClientUnitTest, GetBufferDesc_001, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    capturerInClientInner_->capturerMode_ = CAPTURE_MODE_CALLBACK;
    BufferDesc bufferDesc;
    int32_t ret = capturerInClientInner_->GetBufferDesc(bufferDesc);
    EXPECT_EQ(ret, SUCCESS);

    capturerInClientInner_->capturerMode_ = CAPTURE_MODE_NORMAL;
    ret = capturerInClientInner_->GetBufferDesc(bufferDesc);
    EXPECT_EQ(ret, ERR_INCORRECT_MODE);
}

/**
 * @tc.name  : Test GetBufQueueState API
 * @tc.type  : FUNC
 * @tc.number: GetBufQueueState_001
 * @tc.desc  : Test GetBufQueueState interface.
 */
HWTEST(CapturerInClientUnitTest, GetBufQueueState_001, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    capturerInClientInner_->capturerMode_ = CAPTURE_MODE_CALLBACK;
    BufferQueueState bufState;
    int32_t ret = capturerInClientInner_->GetBufQueueState(bufState);
    EXPECT_EQ(ret, SUCCESS);

    capturerInClientInner_->capturerMode_ = CAPTURE_MODE_NORMAL;
    ret = capturerInClientInner_->GetBufQueueState(bufState);
    EXPECT_EQ(ret, ERR_INCORRECT_MODE);
}

/**
 * @tc.name  : Test Enqueue API
 * @tc.type  : FUNC
 * @tc.number: Enqueue_001
 * @tc.desc  : Test Enqueue interface.
 */
HWTEST(CapturerInClientUnitTest, Enqueue_001, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    capturerInClientInner_->capturerMode_ = CAPTURE_MODE_CALLBACK;
    BufferDesc bufferDesc;
    int32_t ret = capturerInClientInner_->Enqueue(bufferDesc);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test Enqueue API
 * @tc.type  : FUNC
 * @tc.number: Enqueue_002
 * @tc.desc  : Test Enqueue interface.
 */
HWTEST(CapturerInClientUnitTest, Enqueue_002, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    capturerInClientInner_->capturerMode_ = CAPTURE_MODE_CALLBACK;
    capturerInClientInner_->cbBufferSize_ = 16;
    BufferDesc bufferDesc;
    bufferDesc.bufLength = 16;
    bufferDesc.dataLength = 16;
    int32_t ret = capturerInClientInner_->Enqueue(bufferDesc);
    EXPECT_EQ(ret, SUCCESS);

    bufferDesc.bufLength = VALUE_FIF;
    ret = capturerInClientInner_->Enqueue(bufferDesc);
    EXPECT_EQ(ret, ERR_INVALID_INDEX);

    bufferDesc.dataLength = VALUE_FIF;
    ret = capturerInClientInner_->Enqueue(bufferDesc);
    EXPECT_EQ(ret, ERR_INVALID_INDEX);

    bufferDesc.bufLength = 16;
    ret = capturerInClientInner_->Enqueue(bufferDesc);
    EXPECT_EQ(ret, ERR_INVALID_INDEX);
}

/**
 * @tc.name  : Test Enqueue API
 * @tc.type  : FUNC
 * @tc.number: Enqueue_003
 * @tc.desc  : Test Enqueue interface.
 */
HWTEST(CapturerInClientUnitTest, Enqueue_003, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    capturerInClientInner_->capturerMode_ = CAPTURE_MODE_CALLBACK;
    capturerInClientInner_->cbBufferSize_ = 16;
    BufferDesc bufferDesc;
    bufferDesc.bufLength = 16;
    bufferDesc.dataLength = 16;
    uint8_t buffer_ = 1;
    bufferDesc.buffer = &buffer_;
    int32_t ret = capturerInClientInner_->Enqueue(bufferDesc);
    EXPECT_EQ(ret, SUCCESS);

    capturerInClientInner_->cbBuffer_.reset(new uint8_t[VALUE_TEN]);
    bufferDesc.buffer = capturerInClientInner_->cbBuffer_.get();
    ret = capturerInClientInner_->Enqueue(bufferDesc);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test Clear API
 * @tc.type  : FUNC
 * @tc.number: Clear_001
 * @tc.desc  : Test Clear interface.
 */
HWTEST(CapturerInClientUnitTest, Clear_001, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    capturerInClientInner_->capturerMode_ = CAPTURE_MODE_CALLBACK;
    capturerInClientInner_->cbBuffer_.reset(new uint8_t[VALUE_TEN]);
    int32_t ret = capturerInClientInner_->Clear();
    EXPECT_NE(ret, ERR_INCORRECT_MODE);
}

/**
 * @tc.name  : Test StartAudioStream API
 * @tc.type  : FUNC
 * @tc.number: StartAudioStream_001
 * @tc.desc  : Test StartAudioStream interface.
 */
HWTEST(CapturerInClientUnitTest, StartAudioStream_001, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    capturerInClientInner_->state_ = State::RUNNING;
    StateChangeCmdType cmdType = CMD_FROM_CLIENT;
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    bool ret = capturerInClientInner_->StartAudioStream(cmdType, reason);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test StartAudioStream API
 * @tc.type  : FUNC
 * @tc.number: StartAudioStream_002
 * @tc.desc  : Test StartAudioStream interface. Need IpcStreamInServer Object.
 */
HWTEST(CapturerInClientUnitTest, StartAudioStream_002, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    capturerInClientInner_->audioStreamTracker_ = nullptr;
    capturerInClientInner_->state_ = State::PREPARED;
    capturerInClientInner_->notifiedOperation_ = Operation::PAUSE_STREAM;
    capturerInClientInner_->notifiedResult_ = ERR_ILLEGAL_STATE;
    StateChangeCmdType cmdType = CMD_FROM_CLIENT;
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    bool ret = capturerInClientInner_->StartAudioStream(cmdType, reason);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test StartAudioStream API
 * @tc.type  : FUNC
 * @tc.number: StartAudioStream_003
 * @tc.desc  : Test StartAudioStream interface. Need IpcStreamInServer Object.
 */
HWTEST(CapturerInClientUnitTest, StartAudioStream_003, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    capturerInClientInner_->audioStreamTracker_ = nullptr;
    capturerInClientInner_->state_ = State::PREPARED;
    capturerInClientInner_->notifiedOperation_ = Operation::PAUSE_STREAM;
    capturerInClientInner_->notifiedResult_ = SUCCESS;
    StateChangeCmdType cmdType = CMD_FROM_CLIENT;
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    bool ret = capturerInClientInner_->StartAudioStream(cmdType, reason);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test StopAudioStream API
 * @tc.type  : FUNC
 * @tc.number: StopAudioStream_001
 * @tc.desc  : Test StopAudioStream interface.
 */
HWTEST(CapturerInClientUnitTest, StopAudioStream_001, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    capturerInClientInner_->state_ = State::STOPPED;
    bool ret = capturerInClientInner_->StopAudioStream();
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test UpdateLatencyTimestamp API
 * @tc.type  : FUNC
 * @tc.number: UpdateLatencyTimestamp_001
 * @tc.desc  : Test UpdateLatencyTimestamp interface.
 */
HWTEST(CapturerInClientUnitTest, UpdateLatencyTimestamp_001, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    Init(capturerInClientInner_);
    std::string timestamp = "";
    bool isRenderer = false;
    capturerInClientInner_->UpdateLatencyTimestamp(timestamp, isRenderer);
    EXPECT_NE(capturerInClientInner_, nullptr);
}

/**
 * @tc.name  : Test UpdateLatencyTimestamp API
 * @tc.type  : FUNC
 * @tc.number: UpdateLatencyTimestamp_002
 * @tc.desc  : Test UpdateLatencyTimestamp interface.
 */
HWTEST(CapturerInClientUnitTest, UpdateLatencyTimestamp_002, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    std::string timestamp = "";
    bool isRenderer = false;
    capturerInClientInner_->UpdateLatencyTimestamp(timestamp, isRenderer);
    EXPECT_NE(capturerInClientInner_, nullptr);
}

/**
 * @tc.name  : Test HandleCapturerRead API
 * @tc.type  : FUNC
 * @tc.number: HandleCapturerRead_001
 * @tc.desc  : Test HandleCapturerRead interface.
 */
HWTEST(CapturerInClientUnitTest, HandleCapturerRead_001, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    size_t readSize = VALUE_FIVE;
    size_t userSize = 0;
    uint8_t buffer = 0;
    bool isBlockingRead = false;
    int32_t ret = capturerInClientInner_->HandleCapturerRead(readSize, userSize, buffer, isBlockingRead);
    EXPECT_EQ(ret, 5);
}

/**
 * @tc.name  : Test HandleCapturerRead API
 * @tc.type  : FUNC
 * @tc.number: HandleCapturerRead_002
 * @tc.desc  : Test HandleCapturerRead interface.
 */
HWTEST(CapturerInClientUnitTest, HandleCapturerRead_002, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    Init(capturerInClientInner_);
    size_t readSize = VALUE_FIVE;
    size_t userSize = 16;
    uint8_t buffer = 0;
    bool isBlockingRead = false;
    capturerInClientInner_->ringCache_->writeIndex_ = VALUE_FIVE;
    capturerInClientInner_->ringCache_->readIndex_ = 0;
    capturerInClientInner_->ringCache_->cacheTotalSize_ = VALUE_SIX;
    int32_t ret = capturerInClientInner_->HandleCapturerRead(readSize, userSize, buffer, isBlockingRead);
    EXPECT_NE(ret, OPERATION_FAILED);
}

/**
 * @tc.name  : Test Read API
 * @tc.type  : FUNC
 * @tc.number: Read_001
 * @tc.desc  : Test Read interface.
 */
HWTEST(CapturerInClientUnitTest, Read_001, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    capturerInClientInner_->state_ = State::NEW;
    size_t userSize = 16;
    uint8_t buffer = 0;
    bool isBlockingRead = false;
    capturerInClientInner_->readLogTimes_ = MAX_TIMES;
    int32_t ret = capturerInClientInner_->Read(buffer, userSize, isBlockingRead);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : Test HandleCapturerPositionChanges API
 * @tc.type  : FUNC
 * @tc.number: HandleCapturerPositionChanges_001
 * @tc.desc  : Test HandleCapturerPositionChanges interface.
 */
HWTEST(CapturerInClientUnitTest, HandleCapturerPositionChanges_001, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    size_t byteRead = 1;
    capturerInClientInner_->sizePerFrameInByte_ = 0;
    capturerInClientInner_->HandleCapturerPositionChanges(byteRead);
    EXPECT_NE(capturerInClientInner_, nullptr);
}

/**
 * @tc.name  : Test HandleCapturerPositionChanges API
 * @tc.type  : FUNC
 * @tc.number: HandleCapturerPositionChanges_002
 * @tc.desc  : Test HandleCapturerPositionChanges interface.
 */
HWTEST(CapturerInClientUnitTest, HandleCapturerPositionChanges_002, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    size_t byteRead = 1;
    capturerInClientInner_->capturerMarkPosition_ = false;
    capturerInClientInner_->totalBytesRead_ = VALUE_EIGHT;
    capturerInClientInner_->sizePerFrameInByte_ = VALUE_FOUR;
    capturerInClientInner_->capturerMarkPosition_ = VALUE_FOUR;
    capturerInClientInner_->HandleCapturerPositionChanges(byteRead);
    EXPECT_NE(capturerInClientInner_, nullptr);
}

/**
 * @tc.name  : Test HandleCapturerPositionChanges API
 * @tc.type  : FUNC
 * @tc.number: HandleCapturerPositionChanges_003
 * @tc.desc  : Test HandleCapturerPositionChanges interface.
 */
HWTEST(CapturerInClientUnitTest, HandleCapturerPositionChanges_003, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    size_t byteRead = 1;
    capturerInClientInner_->capturerMarkPosition_ = true;
    capturerInClientInner_->capturerPeriodRead_ = VALUE_EIGHT;
    capturerInClientInner_->capturerPeriodSize_ = VALUE_FOUR;
    capturerInClientInner_->HandleCapturerPositionChanges(byteRead);
    EXPECT_NE(capturerInClientInner_, nullptr);

    capturerInClientInner_->capturerPeriodRead_ = VALUE_EIGHT;
    capturerInClientInner_->capturerPeriodSize_ = 0;
    capturerInClientInner_->HandleCapturerPositionChanges(byteRead);
    EXPECT_NE(capturerInClientInner_, nullptr);

    capturerInClientInner_->capturerPeriodRead_ = VALUE_FOUR;
    capturerInClientInner_->capturerPeriodSize_ = VALUE_EIGHT;
    capturerInClientInner_->HandleCapturerPositionChanges(byteRead);
    EXPECT_NE(capturerInClientInner_, nullptr);

    capturerInClientInner_->capturerPeriodRead_ = VALUE_INVALID;
    capturerInClientInner_->capturerPeriodSize_ = 0;
    capturerInClientInner_->HandleCapturerPositionChanges(byteRead);
    EXPECT_NE(capturerInClientInner_, nullptr);
}

/**
 * @tc.name  : Test SetBufferSizeInMsec API
 * @tc.type  : FUNC
 * @tc.number: SetBufferSizeInMsec_001
 * @tc.desc  : Test SetBufferSizeInMsec interface.
 */
HWTEST(CapturerInClientUnitTest, SetBufferSizeInMsec_001, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    int32_t bufferSizeInMsec = 16;
    capturerInClientInner_->capturerMode_ = CAPTURE_MODE_CALLBACK;
    int32_t ret = capturerInClientInner_->SetBufferSizeInMsec(bufferSizeInMsec);
    EXPECT_EQ(ret, SUCCESS);

    capturerInClientInner_->capturerMode_ = CAPTURE_MODE_NORMAL;
    ret = capturerInClientInner_->SetBufferSizeInMsec(bufferSizeInMsec);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test GetFastStatus API
 * @tc.type  : FUNC
 * @tc.number: GetFastStatus_001
 * @tc.desc  : Test GetFastStatus interface.
 */
HWTEST(CapturerInClientUnitTest, GetFastStatus_001, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    ASSERT_NE(capturerInClientInner_, nullptr);

    auto ret = capturerInClientInner_->GetFastStatus();
    EXPECT_EQ(ret, FASTSTATUS_NORMAL);
}

/**
 * @tc.name  : Test RestoreAudioStream API
 * @tc.type  : FUNC
 * @tc.number: RestoreAudioStream_001
 * @tc.desc  : Test RestoreAudioStream interface.
 */
HWTEST(CapturerInClientUnitTest, RestoreAudioStream_001, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    Init(capturerInClientInner_);
    std::shared_ptr<AudioClientTracker> proxyObj = std::make_shared<AudioClientTrackerTest>();
    AudioStreamParams info;
    info.format = AudioSampleFormat::SAMPLE_U8;
    info.encoding = AudioEncodingType::ENCODING_AUDIOVIVID;
    info.samplingRate = AudioSamplingRate::SAMPLE_RATE_8000;
    info.channels = AudioChannel::MONO;
    info.channelLayout = AudioChannelLayout::CH_LAYOUT_MONO;
    capturerInClientInner_->streamParams_ = info;
    capturerInClientInner_->proxyObj_ = std::make_shared<AudioClientTrackerTest>();
    capturerInClientInner_->state_ = State::RUNNING;
    bool ret = capturerInClientInner_->RestoreAudioStream();
    EXPECT_NE(ret, VALUE_EIGHT);

    capturerInClientInner_->state_ = State::PAUSED;
    ret = capturerInClientInner_->RestoreAudioStream();
    EXPECT_NE(ret, VALUE_EIGHT);

    capturerInClientInner_->state_ = State::STOPPED;
    ret = capturerInClientInner_->RestoreAudioStream();
    EXPECT_NE(ret, VALUE_EIGHT);

    capturerInClientInner_->state_ = State::STOPPING;
    ret = capturerInClientInner_->RestoreAudioStream();
    EXPECT_NE(ret, VALUE_EIGHT);

    capturerInClientInner_->state_ = State::PREPARED;
    ret = capturerInClientInner_->RestoreAudioStream();
    EXPECT_NE(ret, VALUE_EIGHT);
}

/**
 * @tc.name  : Test RestoreAudioStream API
 * @tc.type  : FUNC
 * @tc.number: RestoreAudioStream_002
 * @tc.desc  : Test RestoreAudioStream interface.
 */
HWTEST(CapturerInClientUnitTest, RestoreAudioStream_002, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    AudioStreamParams info;
    info.format = AudioSampleFormat::SAMPLE_U8;
    info.encoding = AudioEncodingType::ENCODING_AUDIOVIVID;
    info.samplingRate = AudioSamplingRate::SAMPLE_RATE_8000;
    info.channels = AudioChannel::MONO;
    info.channelLayout = AudioChannelLayout::CH_LAYOUT_MONO;
    capturerInClientInner_->streamParams_ = info;
    capturerInClientInner_->proxyObj_ = std::make_shared<AudioClientTrackerTest>();
    capturerInClientInner_->state_ = State::RUNNING;
    bool ret = capturerInClientInner_->RestoreAudioStream();
    EXPECT_NE(ret, VALUE_EIGHT);
}

/**
 * @tc.name  : Test PauseAudioStream API
 * @tc.type  : FUNC
 * @tc.number: PauseAudioStream_001
 * @tc.desc  : Test PauseAudioStream interface.
 */
HWTEST(CapturerInClientUnitTest, PauseAudioStream_001, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());

    StateChangeCmdType cmdType = CMD_FROM_CLIENT;
    capturerInClientInner->state_ = PREPARED;
    auto ret = capturerInClientInner->PauseAudioStream(cmdType);
    EXPECT_EQ(ret, false);

    capturerInClientInner->state_ = RUNNING;
    capturerInClientInner->ipcStream_ = std::make_shared<IpcStreamTest>().get();
    ret = capturerInClientInner->PauseAudioStream(cmdType);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test PauseAudioStream API
 * @tc.type  : FUNC
 * @tc.number: PauseAudioStream_002
 * @tc.desc  : Test PauseAudioStream interface.
 */
HWTEST(CapturerInClientUnitTest, PauseAudioStream_002, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());

    StateChangeCmdType cmdType = CMD_FROM_CLIENT;
    capturerInClientInner->state_ = RUNNING;
    capturerInClientInner->ipcStream_ = std::make_shared<IpcStreamTest>().get();
    capturerInClientInner->notifiedOperation_ = PAUSE_STREAM;
    auto ret = capturerInClientInner->PauseAudioStream(cmdType);
    EXPECT_EQ(ret, true);

    ret = capturerInClientInner->PauseAudioStream(cmdType);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test PauseAudioStream API
 * @tc.type  : FUNC
 * @tc.number: PauseAudioStream_003
 * @tc.desc  : Test PauseAudioStream interface.
 */
HWTEST(CapturerInClientUnitTest, PauseAudioStream_003, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());

    StateChangeCmdType cmdType = CMD_FROM_CLIENT;
    capturerInClientInner->state_ = RUNNING;
    capturerInClientInner->ipcStream_ = std::make_shared<IpcStreamTest>().get();
    capturerInClientInner->notifiedResult_ = 1;

    auto ret = capturerInClientInner->PauseAudioStream(cmdType);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test FlushAudioStream API
 * @tc.type  : FUNC
 * @tc.number: FlushAudioStream_001
 * @tc.desc  : Test FlushAudioStream interface.
 */
HWTEST(CapturerInClientUnitTest, FlushAudioStream_001, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    size_t size = 8 * 1024;
    capturerInClientInner->ringCache_ = std::make_unique<AudioRingCache>(size);

    capturerInClientInner->state_ = PREPARED;
    auto ret = capturerInClientInner->FlushAudioStream();
    EXPECT_EQ(ret, false);

    capturerInClientInner->state_ = RUNNING;
    capturerInClientInner->ipcStream_ = std::make_shared<IpcStreamTest>().get();
    capturerInClientInner->notifiedOperation_ = FLUSH_STREAM;
    capturerInClientInner->notifiedResult_ = SUCCESS;
    ret = capturerInClientInner->FlushAudioStream();
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test FlushAudioStream API
 * @tc.type  : FUNC
 * @tc.number: FlushAudioStream_002
 * @tc.desc  : Test FlushAudioStream interface.
 */
HWTEST(CapturerInClientUnitTest, FlushAudioStream_002, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    size_t size = 8 * 1024;
    capturerInClientInner->ringCache_ = std::make_unique<AudioRingCache>(size);

    capturerInClientInner->state_ = PAUSED;
    capturerInClientInner->ipcStream_ = std::make_shared<IpcStreamTest>().get();
    capturerInClientInner->notifiedOperation_ = RELEASE_STREAM;
    capturerInClientInner->notifiedResult_ = SUCCESS;
    auto ret = capturerInClientInner->FlushAudioStream();
    EXPECT_EQ(ret, false);

    capturerInClientInner->notifiedResult_ = 10;
    ret = capturerInClientInner->FlushAudioStream();
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test FlushAudioStream API
 * @tc.type  : FUNC
 * @tc.number: FlushAudioStream_003
 * @tc.desc  : Test FlushAudioStream interface.
 */
HWTEST(CapturerInClientUnitTest, FlushAudioStream_003, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    size_t size = 8 * 1024;
    capturerInClientInner->ringCache_ = std::make_unique<AudioRingCache>(size);

    capturerInClientInner->state_ = STOPPED;
    capturerInClientInner->ipcStream_ = std::make_shared<IpcStreamTest>().get();
    capturerInClientInner->notifiedOperation_ = RELEASE_STREAM;
    capturerInClientInner->notifiedResult_ = 10;
    auto ret = capturerInClientInner->FlushAudioStream();
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test FlushCbBuffer API
 * @tc.type  : FUNC
 * @tc.number: FlushCbBuffer_001
 * @tc.desc  : Test FlushCbBuffer interface.
 */
HWTEST(CapturerInClientUnitTest, FlushCbBuffer_001, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());

    capturerInClientInner->capturerMode_ = CAPTURE_MODE_CALLBACK;
    auto ret = capturerInClientInner->FlushCbBuffer();
    EXPECT_EQ(ret, SUCCESS);

    capturerInClientInner->capturerMode_ = CAPTURE_MODE_NORMAL;
    ret = capturerInClientInner->FlushCbBuffer();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test FlushCbBuffer API
 * @tc.type  : FUNC
 * @tc.number: FlushCbBuffer_002
 * @tc.desc  : Test FlushCbBuffer interface.
 */
HWTEST(CapturerInClientUnitTest, FlushCbBuffer_002, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());

    capturerInClientInner->cbBuffer_.reset(new uint8_t[VALUE_TEN]);
    capturerInClientInner->capturerMode_ = CAPTURE_MODE_CALLBACK;
    auto ret = capturerInClientInner->FlushCbBuffer();
    EXPECT_EQ(ret, SUCCESS);

    capturerInClientInner->capturerMode_ = CAPTURE_MODE_NORMAL;
    ret = capturerInClientInner->FlushCbBuffer();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test FetchDeviceForSplitStream API
 * @tc.type  : FUNC
 * @tc.number: FetchDeviceForSplitStream_001
 * @tc.desc  : Test FetchDeviceForSplitStream interface.
 */
HWTEST(CapturerInClientUnitTest, FetchDeviceForSplitStream_001, TestSize.Level1)
{
    int32_t clientUid = 0;
    std::shared_ptr<CapturerInClientInner> capturerInClientInner =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    ASSERT_TRUE(capturerInClientInner != nullptr);

    capturerInClientInner->audioStreamTracker_ = nullptr;
    capturerInClientInner->FetchDeviceForSplitStream();

    capturerInClientInner->audioStreamTracker_ = std::make_unique<AudioStreamTracker>(AudioMode::AUDIO_MODE_PLAYBACK,
        clientUid);
    capturerInClientInner->FetchDeviceForSplitStream();
}

/**
 * @tc.name  : Test CapturerInClient  API
 * @tc.type  : FUNC
 * @tc.number: SetSwitchInfoTimestamp_001
 * @tc.desc  : Test CapturerInClient SetSwitchInfoTimestamp function
 */
HWTEST(CapturerInClientUnitTest, SetSwitchInfoTimestamp_001, TestSize.Level1)
{
    // prepare object
    std::shared_ptr<CapturerInClientInner> testCapturerInClientObj =
        std::make_shared<CapturerInClientInner>(STREAM_DEFAULT, getpid());
    ASSERT_TRUE(testCapturerInClientObj != nullptr);
 
    // start test
    std::vector<std::pair<uint64_t, uint64_t>> testLastFramePosAndTimePair = {
        Timestamp::Timestampbase::BASESIZE, {TEST_POSITION, TEST_TIMESTAMP_NS}
    };
    testCapturerInClientObj->SetSwitchInfoTimestamp(testLastFramePosAndTimePair, testLastFramePosAndTimePair);
    Timestamp testTimestamp;
    testCapturerInClientObj->GetAudioPosition(testTimestamp, Timestamp::Timestampbase::MONOTONIC);
    EXPECT_NE(testTimestamp.framePosition, TEST_POSITION);
}

/**
 * @tc.name  : Test GetFrameCount API
 * @tc.type  : FUNC
 * @tc.number: GetFrameCount_001
 * @tc.desc  : Test GetFrameCount interface.
 */
HWTEST(CapturerInClientUnitTest, GetFrameCount_001, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    uint32_t frameCount = -111;

    int32_t result = capturerInClientInner->GetFrameCount(frameCount);

    frameCount = 10;
    result = capturerInClientInner->GetFrameCount(frameCount);
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name  : Test WaitForRunning API
 * @tc.type  : FUNC
 * @tc.number: WaitForRunning_001
 * @tc.desc  : Test WaitForRunning interface.
 */
HWTEST(CapturerInClientUnitTest, WaitForRunning_001, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    capturerInClientInner->state_ = RUNNING;
    bool result = capturerInClientInner->WaitForRunning();
    EXPECT_TRUE(result);
}

/**
 * @tc.name  : Test WaitForRunning API
 * @tc.type  : FUNC
 * @tc.number: WaitForRunning_002
 * @tc.desc  : Test WaitForRunning interface.
 */
HWTEST(CapturerInClientUnitTest, WaitForRunning_002, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    capturerInClientInner->state_ = PAUSED;
    capturerInClientInner->cbThreadReleased_ = true;
    bool result = capturerInClientInner->WaitForRunning();
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : Test StopAudioStream API
 * @tc.type  : FUNC
 * @tc.number: StopAudioStream_002
 * @tc.desc  : Test StopAudioStream interface.
 */
HWTEST(CapturerInClientUnitTest, StopAudioStream_002, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    capturerInClientInner->state_ = State::INVALID;
    bool result = capturerInClientInner->StopAudioStream();
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : Test StopAudioStream API
 * @tc.type  : FUNC
 * @tc.number: StopAudioStream_003
 * @tc.desc  : Test StopAudioStream interface.
 */
HWTEST(CapturerInClientUnitTest, StopAudioStream_003, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    capturerInClientInner->ipcStream_ = nullptr;
    bool result = capturerInClientInner->StopAudioStream();
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : Test StopAudioStream API
 * @tc.type  : FUNC
 * @tc.number: StopAudioStream_003
 * @tc.desc  : Test StopAudioStream interface.
 */
HWTEST(CapturerInClientUnitTest, FlushCbBuffer_003, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    capturerInClientInner->cbBuffer_ = std::make_unique<uint8_t[]>(10);
    capturerInClientInner->cbBufferSize_ = 10;
    auto ret = capturerInClientInner->FlushCbBuffer();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test FetchDeviceForSplitStream API
 * @tc.type  : FUNC
 * @tc.number: FetchDeviceForSplitStream_002
 * @tc.desc  : Test FetchDeviceForSplitStream interface.
 */
HWTEST(CapturerInClientUnitTest, FetchDeviceForSplitStream_002, TestSize.Level2)
{
    int32_t clientUid = 0;
    std::shared_ptr<CapturerInClientInner> capturerInClientInner =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    capturerInClientInner->SetRestoreStatus(NO_NEED_FOR_RESTORE);
    capturerInClientInner->FetchDeviceForSplitStream();

    EXPECT_NE(capturerInClientInner->CheckRestoreStatus(), NEED_RESTORE);
}
 
/**
 * @tc.name  : Test FetchDeviceForSplitStream API
 * @tc.type  : FUNC
 * @tc.number: FetchDeviceForSplitStream_003
 * @tc.desc  : Test FetchDeviceForSplitStream interface.
 */
HWTEST(CapturerInClientUnitTest, FetchDeviceForSplitStream_003, TestSize.Level2)
{
    int32_t clientUid = 0;
    std::shared_ptr<CapturerInClientInner> capturerInClientInner =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    capturerInClientInner->SetRestoreStatus(NEED_RESTORE);
    capturerInClientInner->audioStreamTracker_ = nullptr;
    capturerInClientInner->FetchDeviceForSplitStream();

    EXPECT_NE(capturerInClientInner->CheckRestoreStatus(), NO_NEED_FOR_RESTORE);
}
} // namespace AudioStandard
} // namespace OHOS
