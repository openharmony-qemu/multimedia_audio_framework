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
#include <gmock/gmock.h>

#include "audio_service_log.h"
#include "audio_errors.h"
#include "fast_audio_stream.h"

using namespace testing::ext;
using namespace testing;
using namespace std;

namespace OHOS {
namespace AudioStandard {
class MockAudioProcessInClient : public AudioProcessInClient {
public:
    MOCK_METHOD(int32_t, SaveDataCallback, (const std::shared_ptr<AudioDataCallback> &dataCallback), (override));
    MOCK_METHOD(int32_t, SaveUnderrunCallback,
        (const std::shared_ptr<ClientUnderrunCallBack> &underrunCallback), (override));

    MOCK_METHOD(int32_t, GetBufferDesc, (BufferDesc &bufDesc), (const, override));
    MOCK_METHOD(int32_t, Enqueue, (const BufferDesc &bufDesc), (override));

    MOCK_METHOD(int32_t, SetVolume, (int32_t vol), (override));
    MOCK_METHOD(int32_t, SetSourceDuration, (int64_t duration), (override));

    MOCK_METHOD(int32_t, Start, (), (override));
    MOCK_METHOD(int32_t, Pause, (bool isFlush), (override));
    MOCK_METHOD(int32_t, Resume, (), (override));
    MOCK_METHOD(int32_t, Stop, (AudioProcessStage stage), (override));
    MOCK_METHOD(int32_t, Release, (bool isSwitchStream), (override));

    MOCK_METHOD(int32_t, GetSessionID, (uint32_t &sessionID), (override));
    MOCK_METHOD(bool, GetAudioTime, (uint32_t &framePos, int64_t &sec, int64_t &nanoSec), (override));
    MOCK_METHOD(int32_t, GetBufferSize, (size_t &bufferSize), (override));
    MOCK_METHOD(int32_t, GetFrameCount, (uint32_t &frameCount), (override));
    MOCK_METHOD(int32_t, GetLatency, (uint64_t &latency), (override));

    MOCK_METHOD(int32_t, SetVolume, (float vol), (override));
    MOCK_METHOD(float, GetVolume, (), (override));

    MOCK_METHOD(int32_t, SetDuckVolume, (float vol), (override));
    MOCK_METHOD(float, GetDuckVolume, (), (override));

    MOCK_METHOD(int32_t, SetMute, (bool mute), (override));
    MOCK_METHOD(bool, GetMute, (), (override));

    MOCK_METHOD(uint32_t, GetUnderflowCount, (), (override));
    MOCK_METHOD(uint32_t, GetOverflowCount, (), (override));
    MOCK_METHOD(void, SetUnderflowCount, (uint32_t underflowCount), (override));
    MOCK_METHOD(void, SetOverflowCount, (uint32_t overflowCount), (override));

    MOCK_METHOD(int64_t, GetFramesWritten, (), (override));
    MOCK_METHOD(int64_t, GetFramesRead, (), (override));

    MOCK_METHOD(void, SetPreferredFrameSize, (int32_t frameSize), (override));
    MOCK_METHOD(void, UpdateLatencyTimestamp, (std::string &timestamp, bool isRenderer), (override));

    MOCK_METHOD(int32_t, SetDefaultOutputDevice, (const DeviceType defaultOutputDevice, bool skipForce), (override));
    MOCK_METHOD(int32_t, SetSilentModeAndMixWithOthers, (bool on), (override));

    MOCK_METHOD(void, GetRestoreInfo, (RestoreInfo &restoreInfo), (override));
    MOCK_METHOD(void, SetRestoreInfo, (RestoreInfo &restoreInfo), (override));
    MOCK_METHOD(RestoreStatus, CheckRestoreStatus, (), (override));
    MOCK_METHOD(RestoreStatus, SetRestoreStatus, (RestoreStatus restoreStatus), (override));

    MOCK_METHOD(void, SaveAdjustStreamVolumeInfo,
        (float volume, uint32_t sessionId, std::string adjustTime, uint32_t code), (override));

    MOCK_METHOD(int32_t, RegisterThreadPriority,
        (pid_t tid, const std::string &bundleName, BoostTriggerMethod method), (override));

    MOCK_METHOD(bool, GetStopFlag, (), (const, override));
    MOCK_METHOD(void, JoinCallbackLoop, (), (override));
    MOCK_METHOD(void, SetAudioHapticsSyncId, (const int32_t &audioHapticsSyncId), (override));
    MOCK_METHOD(bool, IsRestoreNeeded, (), (override));
    MOCK_METHOD(void, SetRebuildFlag, (), (override));
};

class FastSystemStreamUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}

    void SetUp() override
    {
        int32_t appUid = static_cast<int32_t>(getuid());
        stream_ = make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
        mockProcessClient_ = make_shared<MockAudioProcessInClient>();
        stream_->processClient_ = mockProcessClient_;
    }

    void TearDown() override
    {
        stream_.reset();
        mockProcessClient_.reset();
    }

    shared_ptr<FastAudioStream> stream_ = nullptr;
    shared_ptr<MockAudioProcessInClient> mockProcessClient_ = nullptr;
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

class AudioRendererFirstFrameWritingCallbackTest : public AudioRendererFirstFrameWritingCallback {
public:
    virtual ~AudioRendererFirstFrameWritingCallbackTest() = default;
    /**
     * Called when first buffer to be enqueued.
     */
    virtual void OnFirstFrameWriting(uint64_t latency) {}
};

class AudioRendererWriteCallbackTest : public AudioRendererWriteCallback {
public:
    virtual ~AudioRendererWriteCallbackTest() = default;

    /**
     * Called when buffer to be enqueued.
     *
     * @param length Indicates requested buffer length.
     * @since 8
     */
    virtual void OnWriteData(size_t length) {}
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

class MockFastAudioStream : public FastAudioStream {
public:
    using FastAudioStream::FastAudioStream;
    MOCK_METHOD(float, GetDuckVolume, (),  (override));
};

/**
 * @tc.name  : Test GetVolume API
 * @tc.type  : FUNC
 * @tc.number: GetVolume_001
 * @tc.desc  : Test GetVolume interface.
 */
HWTEST_F(FastSystemStreamUnitTest, GetVolume_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);

    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetVolume_001 start");
    fastAudioStream->silentModeAndMixWithOthers_ = true;
    float result = fastAudioStream->GetVolume();
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetVolume_001 result:%{public}f", result);
    EXPECT_GT(result, 0);
}

/**
 * @tc.name  : Test SetVolume API
 * @tc.type  : FUNC
 * @tc.number: SetVolume_001
 * @tc.desc  : Test SetVolume interface.
 */
HWTEST_F(FastSystemStreamUnitTest, SetVolume_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);

    float volume = 0.5f;
    fastAudioStream->silentModeAndMixWithOthers_ = true;
    int32_t result = fastAudioStream->SetVolume(volume);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetVolume_001 result:%{public}d", result);
    EXPECT_NE(result, ERROR);
}

/**
 * @tc.name  : Test SetSilentModeAndMixWithOthers API
 * @tc.type  : FUNC
 * @tc.number: SetSilentModeAndMixWithOthers_001
 * @tc.desc  : Test SetSilentModeAndMixWithOthers interface.
 */
HWTEST_F(FastSystemStreamUnitTest, SetSilentModeAndMixWithOthers_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetSilentModeAndMixWithOthers_001 start");
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);

    bool on = false;
    fastAudioStream->silentModeAndMixWithOthers_ = false;
    fastAudioStream->SetSilentModeAndMixWithOthers(on);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetSilentModeAndMixWithOthers_001 -1");
    fastAudioStream->silentModeAndMixWithOthers_ = true;
    fastAudioStream->SetSilentModeAndMixWithOthers(on);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetSilentModeAndMixWithOthers_001 -2");

    on = true;
    fastAudioStream->silentModeAndMixWithOthers_ = false;
    fastAudioStream->SetSilentModeAndMixWithOthers(on);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetSilentModeAndMixWithOthers_001 -3");
    fastAudioStream->silentModeAndMixWithOthers_ = true;
    fastAudioStream->SetSilentModeAndMixWithOthers(on);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetSilentModeAndMixWithOthers_001 -4");
}

/**
 * @tc.name  : Test GetSwitchInfo API
 * @tc.type  : FUNC
 * @tc.number: GetSwitchInfo_001
 * @tc.desc  : Test GetSwitchInfo interface.
 */
HWTEST_F(FastSystemStreamUnitTest, GetSwitchInfo_001, TestSize.Level4)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetSwitchInfo_001 start");
    int32_t appUid = static_cast<int32_t>(getuid());
    auto fastAudioStream = std::make_shared<MockFastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    EXPECT_NE(fastAudioStream, nullptr);
    EXPECT_CALL(*fastAudioStream, GetDuckVolume()).WillRepeatedly(testing::Return(0.2));

    IAudioStream::SwitchInfo info;
    fastAudioStream->GetSwitchInfo(info);
    EXPECT_EQ(info.renderMode, fastAudioStream->renderMode_);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetSwitchInfo_001 end");
}

/**
 * @tc.name  : Test GetSwitchInfo API
 * @tc.type  : FUNC
 * @tc.number: GetSwitchInfo_002
 * @tc.desc  : Test GetSwitchInfo interface.
 */
HWTEST_F(FastSystemStreamUnitTest, GetSwitchInfo_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetSwitchInfo_002 start");
    int32_t appUid = static_cast<int32_t>(getuid());
    auto fastAudioStream = std::make_shared<MockFastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    EXPECT_NE(fastAudioStream, nullptr);
    EXPECT_CALL(*fastAudioStream, GetDuckVolume()).WillRepeatedly(testing::Return(0.2));

    std::shared_ptr<AudioRendererWriteCallback> spkCallback = std::make_shared<AudioRendererWriteCallbackTest>();
    AudioStreamParams tempParams = {};
    auto audioStream = IAudioStream::GetRecordStream(IAudioStream::PA_STREAM, tempParams, STREAM_MUSIC, getpid());
    fastAudioStream->spkProcClientCb_ = std::make_shared<FastAudioStreamRenderCallback>(spkCallback, *audioStream);

    std::shared_ptr<AudioCapturerReadCallback> micCallback = std::make_shared<AudioCapturerReadCallbackTest>();
    fastAudioStream->micProcClientCb_ = std::make_shared<FastAudioStreamCaptureCallback>(micCallback);

    fastAudioStream->firstFrameWritingCb_ = std::make_shared<AudioRendererFirstFrameWritingCallbackTest>();

    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetSwitchInfo_002 start");

    IAudioStream::SwitchInfo info;
    fastAudioStream->GetSwitchInfo(info);
    EXPECT_EQ(info.renderMode, fastAudioStream->renderMode_);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetSwitchInfo_002 end");
}

/**
 * @tc.name  : Test UpdatePlaybackCaptureConfig API
 * @tc.type  : FUNC
 * @tc.number: UpdatePlaybackCaptureConfig_001
 * @tc.desc  : Test UpdatePlaybackCaptureConfig interface.
 */
HWTEST_F(FastSystemStreamUnitTest, UpdatePlaybackCaptureConfig_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest UpdatePlaybackCaptureConfig_001 start");
    int32_t appUid = static_cast<int32_t>(getuid());
    AudioPlaybackCaptureConfig config;
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    int32_t res = 0;
    res = fastAudioStream->UpdatePlaybackCaptureConfig(config);
    EXPECT_EQ(res, ERR_NOT_SUPPORTED);
}

/**
 * @tc.name  : Test GetAudioPipeType and SetAudioStreamType API
 * @tc.type  : FUNC
 * @tc.number: GetAudioPipeType_001
 * @tc.desc  : Test GetAudioPipeType and SetAudioStreamType interface.
 */
HWTEST_F(FastSystemStreamUnitTest, GetAudioPipeType_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetAudioPipeType_001 start");
    int32_t appUid = static_cast<int32_t>(getuid());
    AudioPipeType pipeType;
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    fastAudioStream->GetAudioPipeType(pipeType);
    AudioStreamType audioStreamType = STREAM_DEFAULT;
    int32_t res = fastAudioStream->SetAudioStreamType(audioStreamType);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/**
 * @tc.name  : Test GetFastStatus API
 * @tc.type  : FUNC
 * @tc.number: GetFastStatus_001
 * @tc.desc  : Test GetFastStatus and SetAudioStreamType interface.
 */
HWTEST_F(FastSystemStreamUnitTest, GetFastStatus_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    ASSERT_NE(fastAudioStream, nullptr);

    auto ret = fastAudioStream->GetFastStatus();
    EXPECT_EQ(ret, FASTSTATUS_FAST);
}

/**
 * @tc.name  : Test SetMute API
 * @tc.type  : FUNC
 * @tc.number: SetMute_001
 * @tc.desc  : Test SetMute interface.
 */
HWTEST_F(FastSystemStreamUnitTest, SetMute_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetMute_001 start");
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    int32_t res = fastAudioStream->SetMute(false, StateChangeCmdType::CMD_FROM_CLIENT);
    EXPECT_EQ(res, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test SetRenderMode and GetCaptureMode API
 * @tc.type  : FUNC
 * @tc.number: SetRenderMode_001
 * @tc.desc  : Test SetRenderMode and GetCaptureMode interface.
 */
HWTEST_F(FastSystemStreamUnitTest, SetRenderMode_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetRenderMode_001 start");
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    AudioRenderMode renderMode = RENDER_MODE_CALLBACK;
    int32_t res = fastAudioStream->SetRenderMode(renderMode);
    EXPECT_EQ(res, SUCCESS);
}

/**
 * @tc.name  : Test GetCaptureMode API
 * @tc.type  : FUNC
 * @tc.number: GetCaptureMode_001
 * @tc.desc  : Test GetCaptureMode interface.
 */
HWTEST_F(FastSystemStreamUnitTest, GetCaptureMode_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetCaptureMode_001 start");
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    AudioCaptureMode captureMode;
    captureMode = fastAudioStream->GetCaptureMode();
    EXPECT_EQ(captureMode, CAPTURE_MODE_CALLBACK);
}

/**
 * @tc.name  : Test SetLowPowerVolume, GetLowPowerVolume and GetSingleStreamVolume API
 * @tc.type  : FUNC
 * @tc.number: SetLowPowerVolume_001
 * @tc.desc  : Test SetLowPowerVolume, GetLowPowerVolume and GetSingleStreamVolume interface.
 */
HWTEST_F(FastSystemStreamUnitTest, SetLowPowerVolume_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetLowPowerVolume_001 start");
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    int32_t res = fastAudioStream->SetLowPowerVolume(1.0f);
    EXPECT_EQ(res, SUCCESS);
    float volume = fastAudioStream->GetLowPowerVolume();
    EXPECT_EQ(volume, 1.0f);
    volume = fastAudioStream->GetSingleStreamVolume();
    EXPECT_EQ(volume, 1.0f);
}

/**
 * @tc.name  : Test SetOffloadMode and UnsetOffloadMode API
 * @tc.type  : FUNC
 * @tc.number: SetOffloadMode_001
 * @tc.desc  : Test SetOffloadMode and UnsetOffloadMode interface.
 */
HWTEST_F(FastSystemStreamUnitTest, SetOffloadMode_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetOffloadMode_001 start");
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    int32_t res = fastAudioStream->SetOffloadMode(0, true);
    EXPECT_EQ(res, ERR_NOT_SUPPORTED);
    res = fastAudioStream->UnsetOffloadMode();
    EXPECT_EQ(res, ERR_NOT_SUPPORTED);
}

/**
 * @tc.name  : Test SetAudioEffectMode API
 * @tc.type  : FUNC
 * @tc.number: SetAudioEffectMode_001
 * @tc.desc  : Test SetAudioEffectMode interface.
 */
HWTEST_F(FastSystemStreamUnitTest, SetAudioEffectMode_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetAudioEffectMode_001 start");
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    AudioEffectMode effectMode = EFFECT_NONE;
    int32_t res = fastAudioStream->SetAudioEffectMode(effectMode);
    EXPECT_EQ(res, ERR_NOT_SUPPORTED);
}

/**
 * @tc.name  : Test GetFramesWritten and GetFramesRead API
 * @tc.type  : FUNC
 * @tc.number: GetFramesWritten_001
 * @tc.desc  : Test GetFramesWritten and GetFramesRead interface.
 */
HWTEST_F(FastSystemStreamUnitTest, GetFramesWritten_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetFramesWritten_001 start");
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    int32_t res = fastAudioStream->GetFramesWritten();
    EXPECT_EQ(res, -1);
    res = fastAudioStream->GetFramesRead();
    EXPECT_EQ(res, -1);
}

/**
 * @tc.name  : Test SetSpeed and GetSpeed API
 * @tc.type  : FUNC
 * @tc.number: SetSpeed_001
 * @tc.desc  : Test SetSpeed and GetSpeed interface.
 */
HWTEST_F(FastSystemStreamUnitTest, SetSpeed_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetSpeed_001 start");
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    int32_t res = fastAudioStream->SetSpeed(1.0f);
    EXPECT_EQ(res, ERR_OPERATION_FAILED);
    float ret = fastAudioStream->GetSpeed();
    EXPECT_EQ(ret, static_cast<float>(ERROR));
}

/**
 * @tc.name  : Test SetPitch API
 * @tc.type  : FUNC
 * @tc.number: SetPitch_001
 * @tc.desc  : Test SetPitch interface.
 */
HWTEST_F(FastSystemStreamUnitTest, SetPitch_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetPitch_001 start");
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    int32_t res = fastAudioStream->SetPitch(1.0f);
    EXPECT_EQ(res, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test FlushAudioStream and DrainAudioStream API
 * @tc.type  : FUNC
 * @tc.number: SetSpeed_001
 * @tc.desc  : Test FlushAudioStream and DrainAudioStream interface.
 */
HWTEST_F(FastSystemStreamUnitTest, FlushAudioStream_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest FlushAudioStream_001 start");
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    bool res = fastAudioStream->FlushAudioStream();
    EXPECT_EQ(res, true);
    res = fastAudioStream->DrainAudioStream(true);
    EXPECT_EQ(res, true);
}

/**
 * @tc.name  : Test callbacks and samplingrate API
 * @tc.type  : FUNC
 * @tc.number: SetAndGetCallback_001
 * @tc.desc  : Test callbacks and samplingrate interface.
 */
HWTEST_F(FastSystemStreamUnitTest, SetAndGetCallback_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetAndGetCallback_001 start");
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);

    std::shared_ptr<RendererPeriodPositionCallback> rendererPeriodPositionCallback = nullptr;
    fastAudioStream->SetRendererPeriodPositionCallback(0, rendererPeriodPositionCallback);
    fastAudioStream->UnsetRendererPeriodPositionCallback();

    std::shared_ptr<CapturerPositionCallback> capturerPositionCallback = nullptr;
    fastAudioStream->SetCapturerPositionCallback(0, capturerPositionCallback);
    fastAudioStream->UnsetCapturerPositionCallback();

    std::shared_ptr<CapturerPeriodPositionCallback> capturerPeriodPositionCallback = nullptr;
    fastAudioStream->SetCapturerPeriodPositionCallback(0, capturerPeriodPositionCallback);
    fastAudioStream->UnsetCapturerPeriodPositionCallback();

    int32_t res = fastAudioStream->SetRendererSamplingRate(0);
    EXPECT_EQ(res, ERR_OPERATION_FAILED);

    uint32_t samplingRate = fastAudioStream->streamInfo_.samplingRate;
    uint32_t rate = fastAudioStream->GetRendererSamplingRate();
    EXPECT_EQ(rate, samplingRate);
}

/**
 * @tc.name  : Test SetAudioStreamInfo API
 * @tc.type  : FUNC
 * @tc.number: SetAudioStreamInfo_001
 * @tc.desc  : Test SetAudioStreamInfo interface.
 */
HWTEST_F(FastSystemStreamUnitTest, SetAudioStreamInfo_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetAudioStreamInfo_001 start");
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    std::shared_ptr<AudioClientTracker> proxyObj;
    fastAudioStream->state_ = PREPARED;
    AudioStreamParams info;
    int32_t res = fastAudioStream->SetAudioStreamInfo(info, proxyObj);
    EXPECT_EQ(res, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test SetAudioStreamInfo API
 * @tc.type  : FUNC
 * @tc.number: SetAudioStreamInfo_002
 * @tc.desc  : Test SetAudioStreamInfo interface.
 */
HWTEST_F(FastSystemStreamUnitTest, SetAudioStreamInfo_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetAudioStreamInfo_002 start");
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    std::shared_ptr<AudioClientTracker> proxyObj;
    AudioStreamParams info;
    info.format = AudioSampleFormat::SAMPLE_S16LE;
    info.encoding = AudioEncodingType::ENCODING_PCM;
    info.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    info.channels = AudioChannel::MONO;
    info.channelLayout = AudioChannelLayout::CH_LAYOUT_MONO;
    int32_t res = fastAudioStream->SetAudioStreamInfo(info, proxyObj);
    EXPECT_NE(res, SUCCESS);
    auto ret = fastAudioStream->RestoreAudioStream(true);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test RestoreAudioStream API
 * @tc.type  : FUNC
 * @tc.number: RestoreAudioStream_001
 * @tc.desc  : Test RestoreAudioStream interface.
 */
HWTEST_F(FastSystemStreamUnitTest, RestoreAudioStream_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest RestoreAudioStream_001 start");
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    bool result = false;
    AudioStreamParams info;
    info.format = AudioSampleFormat::SAMPLE_S16LE;
    info.encoding = AudioEncodingType::ENCODING_PCM;
    info.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    info.channels = AudioChannel::MONO;
    info.channelLayout = AudioChannelLayout::CH_LAYOUT_MONO;
    std::shared_ptr<AudioClientTracker> proxyObj = std::make_shared<AudioClientTrackerTest>();
    fastAudioStream->proxyObj_ = proxyObj;
    fastAudioStream->streamInfo_ = info;
    fastAudioStream->state_ = RUNNING;
    result = fastAudioStream->SetAudioStreamInfo(info, proxyObj);
    EXPECT_EQ(result, true);
    fastAudioStream->state_ = PAUSED;
    result = fastAudioStream->SetAudioStreamInfo(info, proxyObj);
    EXPECT_EQ(result, true);
    fastAudioStream->state_ = STOPPED;
    result = fastAudioStream->SetAudioStreamInfo(info, proxyObj);
    EXPECT_EQ(result, true);
    fastAudioStream->state_ = STOPPING;
    result = fastAudioStream->SetAudioStreamInfo(info, proxyObj);
    EXPECT_EQ(result, true);
    fastAudioStream->state_ = INVALID;
    result = fastAudioStream->SetAudioStreamInfo(info, proxyObj);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name  : Test InitializeAudioProcessConfig API
 * @tc.type  : FUNC
 * @tc.number: InitializeAudioProcessConfig_001
 * @tc.desc  : Test InitializeAudioProcessConfig interface.
 */
HWTEST_F(FastSystemStreamUnitTest, InitializeAudioProcessConfig_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);

    AUDIO_INFO_LOG("AudioSystemManagerUnitTest InitializeAudioProcessConfig_001 start");
    AudioProcessConfig config;
    AudioStreamParams info;
    fastAudioStream->eMode_ = static_cast<AudioMode>(-1);
    auto result = fastAudioStream->InitializeAudioProcessConfig(config, info);
    EXPECT_EQ(result, ERR_INVALID_OPERATION);
}

/**
 * @tc.name  : Test InitializeAudioProcessConfig API
 * @tc.type  : FUNC
 * @tc.number: InitializeAudioProcessConfig_002
 * @tc.desc  : Test InitializeAudioProcessConfig interface.
 */
HWTEST_F(FastSystemStreamUnitTest, InitializeAudioProcessConfig_002, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    AudioRendererInfo rendererInfo;
    rendererInfo.isVirtualKeyboard = true;
    fastAudioStream->SetRendererInfo(rendererInfo);

    AUDIO_INFO_LOG("AudioSystemManagerUnitTest InitializeAudioProcessConfig_002 start");
    AudioProcessConfig config;
    AudioStreamParams info;
    auto result = fastAudioStream->InitializeAudioProcessConfig(config, info);
    EXPECT_TRUE(config.rendererInfo.isVirtualKeyboard);
    EXPECT_NE(result, ERR_INVALID_OPERATION);
}

/**
 * @tc.name  : Test GetState API
 * @tc.type  : FUNC
 * @tc.number: GetState_001
 * @tc.desc  : Test GetState interface.
 */
HWTEST_F(FastSystemStreamUnitTest, GetState_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);

    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetState_001 start");
    fastAudioStream->switchingInfo_.isSwitching_ = true;
    auto result = fastAudioStream->GetState();
    EXPECT_EQ(result, State::INVALID);
}

/**
 * @tc.name  : Test GetAudioPosition API
 * @tc.type  : FUNC
 * @tc.number: GetAudioPosition_001
 * @tc.desc  : Test GetAudioPosition interface.
 */
HWTEST_F(FastSystemStreamUnitTest, GetAudioPosition_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);

    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetAudioPosition_001 start");
    Timestamp timestamp;
    Timestamp::Timestampbase base = Timestamp::Timestampbase::MONOTONIC;
    auto result = fastAudioStream->GetAudioPosition(timestamp, base);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test SetRendererFirstFrameWritingCallback API
 * @tc.type  : FUNC
 * @tc.number: SetRendererFirstFrameWritingCallback_001
 * @tc.desc  : Test SetRendererFirstFrameWritingCallback interface.
 */
HWTEST_F(FastSystemStreamUnitTest, SetRendererFirstFrameWritingCallback_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);

    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetRendererFirstFrameWritingCallback_001 start");
    std::shared_ptr<AudioRendererFirstFrameWritingCallback> callback =
        std::make_shared<AudioRendererFirstFrameWritingCallbackTest>();
    auto result = fastAudioStream->SetRendererFirstFrameWritingCallback(callback);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : Test SetPreferredFrameSize API
 * @tc.type  : FUNC
 * @tc.number: SetPreferredFrameSize_001
 * @tc.desc  : Test SetPreferredFrameSize interface.
 */
HWTEST_F(FastSystemStreamUnitTest, SetPreferredFrameSize_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);

    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetPreferredFrameSize_001 start");
    int32_t frameSize = 0;
    fastAudioStream->SetPreferredFrameSize(frameSize);
}

/**
 * @tc.name  : Test UpdateLatencyTimestamp API
 * @tc.type  : FUNC
 * @tc.number: UpdateLatencyTimestamp_001
 * @tc.desc  : Test UpdateLatencyTimestamp interface.
 */
HWTEST_F(FastSystemStreamUnitTest, UpdateLatencyTimestamp_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);

    AUDIO_INFO_LOG("AudioSystemManagerUnitTest UpdateLatencyTimestamp_001 start");
    std::string timestamp = "";
    bool isRenderer = true;
    fastAudioStream->UpdateLatencyTimestamp(timestamp, isRenderer);
}

/**
 * @tc.name  : Test Read API
 * @tc.type  : FUNC
 * @tc.number: Read_001
 * @tc.desc  : Test Read interface.
 */
HWTEST_F(FastSystemStreamUnitTest, Read_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);

    AUDIO_INFO_LOG("AudioSystemManagerUnitTest Read_001 start");
    uint8_t buffer = 0XFF;
    size_t userSize = 0;
    bool isBlockingRead = true;
    auto result = fastAudioStream->Read(buffer, userSize, isBlockingRead);
    EXPECT_EQ(result, ERR_INVALID_OPERATION);
}

/**
 * @tc.name  : Test Write API
 * @tc.type  : FUNC
 * @tc.number: Write_001
 * @tc.desc  : Test Write interface.
 */
HWTEST_F(FastSystemStreamUnitTest, Write_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);

    AUDIO_INFO_LOG("AudioSystemManagerUnitTest Write_001 start");
    uint8_t *pcmBuffer = new uint8_t[2];
    size_t pcmBufferSize = 0;
    uint8_t *metaBuffer = new uint8_t[2];
    size_t metaBufferSize = 0;
    auto result = fastAudioStream->Write(pcmBuffer, pcmBufferSize, metaBuffer, metaBufferSize);
    EXPECT_EQ(result, ERR_INVALID_OPERATION);
}

/**
 * @tc.name  : Test SetUnderflowCount API
 * @tc.type  : FUNC
 * @tc.number: SetUnderflowCount_001
 * @tc.desc  : Test SetUnderflowCount interface.
 */
HWTEST_F(FastSystemStreamUnitTest, SetUnderflowCount_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);

    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetUnderflowCount_001 start");
    uint32_t underflowCount = 0;
    fastAudioStream->SetUnderflowCount(underflowCount);
}

/**
 * @tc.name  : Test SetOverflowCount API
 * @tc.type  : FUNC
 * @tc.number: SetOverflowCount_001
 * @tc.desc  : Test SetOverflowCount interface.
 */
HWTEST_F(FastSystemStreamUnitTest, SetOverflowCount_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);

    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetOverflowCount_001 start");
    uint32_t overflowCount = 0;
    fastAudioStream->SetOverflowCount(overflowCount);
}

/**
 * @tc.name  : Test SetBufferSizeInMsec API
 * @tc.type  : FUNC
 * @tc.number: SetBufferSizeInMsec_001
 * @tc.desc  : Test SetBufferSizeInMsec interface.
 */
HWTEST_F(FastSystemStreamUnitTest, SetBufferSizeInMsec_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);

    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetBufferSizeInMsec_001 start");
    int32_t bufferSizeInMsec = 0;
    auto result = fastAudioStream->SetBufferSizeInMsec(bufferSizeInMsec);
    EXPECT_EQ(result, ERR_NOT_SUPPORTED);
}

/**
 * @tc.name  : Test SetInnerCapturerState API
 * @tc.type  : FUNC
 * @tc.number: SetInnerCapturerState_001
 * @tc.desc  : Test SetInnerCapturerState interface.
 */
HWTEST_F(FastSystemStreamUnitTest, SetInnerCapturerState_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);

    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetInnerCapturerState_001 start");
    bool isInnerCapturer = true;
    fastAudioStream->SetInnerCapturerState(isInnerCapturer);
}

/**
 * @tc.name  : Test SetWakeupCapturerState API
 * @tc.type  : FUNC
 * @tc.number: SetWakeupCapturerState_001
 * @tc.desc  : Test SetWakeupCapturerState interface.
 */
HWTEST_F(FastSystemStreamUnitTest, SetWakeupCapturerState_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);

    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetWakeupCapturerState_001 start");
    bool isWakeupCapturer = true;
    fastAudioStream->SetWakeupCapturerState(isWakeupCapturer);
}

/**
 * @tc.name  : Test OnFirstFrameWriting API
 * @tc.type  : FUNC
 * @tc.number: OnFirstFrameWriting_001
 * @tc.desc  : Test OnFirstFrameWriting interface.
 */
HWTEST_F(FastSystemStreamUnitTest, OnFirstFrameWriting_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);

    AUDIO_INFO_LOG("AudioSystemManagerUnitTest OnFirstFrameWriting_001 start");
    fastAudioStream->OnFirstFrameWriting();
}

/**
 * @tc.name  : Test OnHandleData API
 * @tc.type  : FUNC
 * @tc.number: OnHandleData_001
 * @tc.desc  : Test OnHandleData interface.
 */
HWTEST_F(FastSystemStreamUnitTest, OnHandleData_001, TestSize.Level1)
{
    std::shared_ptr<AudioRendererWriteCallback> callback = std::make_shared<AudioRendererWriteCallbackTest>();
    AudioStreamParams tempParams = {};
    auto audioStream = IAudioStream::GetRecordStream(IAudioStream::PA_STREAM, tempParams, STREAM_MUSIC, getpid());
    std::shared_ptr<FastAudioStreamRenderCallback> fastAudioStreamRenderCallback;
    fastAudioStreamRenderCallback = std::make_shared<FastAudioStreamRenderCallback>(callback, *audioStream);

    AUDIO_INFO_LOG("AudioSystemManagerUnitTest OnHandleData_001 start");
    size_t length = 0;
    fastAudioStreamRenderCallback->OnHandleData(length);
}

/**
 * @tc.name  : Test ResetFirstFrameState API
 * @tc.type  : FUNC
 * @tc.number: ResetFirstFrameState_001
 * @tc.desc  : Test ResetFirstFrameState interface.
 */
HWTEST_F(FastSystemStreamUnitTest, ResetFirstFrameState_001, TestSize.Level1)
{
    std::shared_ptr<AudioRendererWriteCallback> callback = std::make_shared<AudioRendererWriteCallbackTest>();
    AudioStreamParams tempParams = {};
    auto audioStream = IAudioStream::GetRecordStream(IAudioStream::PA_STREAM, tempParams, STREAM_MUSIC, getpid());
    std::shared_ptr<FastAudioStreamRenderCallback> fastAudioStreamRenderCallback;
    fastAudioStreamRenderCallback = std::make_shared<FastAudioStreamRenderCallback>(callback, *audioStream);

    AUDIO_INFO_LOG("AudioSystemManagerUnitTest ResetFirstFrameState_001 start");
    fastAudioStreamRenderCallback->ResetFirstFrameState();
}

/**
 * @tc.name  : Test ResetFirstFrameState API
 * @tc.type  : FUNC
 * @tc.number: ResetFirstFrameState_002
 * @tc.desc  : Test ResetFirstFrameState interface. - do nothing when spkProcClientCb_ is null
 */
HWTEST_F(FastSystemStreamUnitTest, ResetFirstFrameState_002, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream =
        std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    EXPECT_NE(fastAudioStream, nullptr);

    std::shared_ptr<AudioRendererWriteCallback> spkCallback = std::make_shared<AudioRendererWriteCallbackTest>();
    AudioStreamParams tempParams = {};
    auto audioStream = IAudioStream::GetRecordStream(IAudioStream::PA_STREAM, tempParams, STREAM_MUSIC, getpid());
    auto spkProcClientCb = std::make_shared<FastAudioStreamRenderCallback>(spkCallback, *audioStream);
    spkProcClientCb->hasFirstFrameWrited_.store(true);

    fastAudioStream->spkProcClientCb_ = spkProcClientCb;
    fastAudioStream->ResetFirstFrameState();

    fastAudioStream->spkProcClientCb_ = nullptr;
    fastAudioStream->ResetFirstFrameState();
    
    EXPECT_EQ(spkProcClientCb->hasFirstFrameWrited_.load(), false);
}

/**
 * @tc.name  : Test GetRendererWriteCallback API
 * @tc.type  : FUNC
 * @tc.number: GetRendererWriteCallback_001
 * @tc.desc  : Test GetRendererWriteCallback interface.
 */
HWTEST_F(FastSystemStreamUnitTest, GetRendererWriteCallback_001, TestSize.Level1)
{
    std::shared_ptr<AudioRendererWriteCallback> callback = std::make_shared<AudioRendererWriteCallbackTest>();
    AudioStreamParams tempParams = {};
    auto audioStream = IAudioStream::GetRecordStream(IAudioStream::PA_STREAM, tempParams, STREAM_MUSIC, getpid());
    std::shared_ptr<FastAudioStreamRenderCallback> fastAudioStreamRenderCallback;
    fastAudioStreamRenderCallback = std::make_shared<FastAudioStreamRenderCallback>(callback, *audioStream);

    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetRendererWriteCallback_001 start");
    auto result = fastAudioStreamRenderCallback->GetRendererWriteCallback();
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name  : Test GetCapturerReadCallback API
 * @tc.type  : FUNC
 * @tc.number: GetCapturerReadCallback_001
 * @tc.desc  : Test GetCapturerReadCallback interface.
 */
HWTEST_F(FastSystemStreamUnitTest, GetCapturerReadCallback_001, TestSize.Level1)
{
    std::shared_ptr<AudioCapturerReadCallback> callback = std::make_shared<AudioCapturerReadCallbackTest>();
    std::shared_ptr<FastAudioStreamCaptureCallback> fastAudioStreamCaptureCallback;
    fastAudioStreamCaptureCallback = std::make_shared<FastAudioStreamCaptureCallback>(callback);

    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetCapturerReadCallback_001 start");
    auto result = fastAudioStreamCaptureCallback->GetCapturerReadCallback();
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name  : Test SetChannelBlendMode API
 * @tc.type  : FUNC
 * @tc.number: SetChannelBlendMode_001
 * @tc.desc  : Test SetChannelBlendMode interface.
 */
HWTEST_F(FastSystemStreamUnitTest, SetChannelBlendMode_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);

    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetChannelBlendMode_001 start");
    ChannelBlendMode blendMode = ChannelBlendMode::MODE_DEFAULT;
    auto result = fastAudioStream->SetChannelBlendMode(blendMode);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : Test SetVolumeWithRamp API
 * @tc.type  : FUNC
 * @tc.number: SetVolumeWithRamp_001
 * @tc.desc  : Test SetVolumeWithRamp interface.
 */
HWTEST_F(FastSystemStreamUnitTest, SetVolumeWithRamp_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);

    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetVolumeWithRamp_001 start");
    float volume = 0;
    int32_t duration = 0;
    auto result = fastAudioStream->SetVolumeWithRamp(volume, duration);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : Test GetOffloadEnable API
 * @tc.type  : FUNC
 * @tc.number: GetOffloadEnable_001
 * @tc.desc  : Test GetOffloadEnable interface.
 */
HWTEST_F(FastSystemStreamUnitTest, GetOffloadEnable_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);

    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetOffloadEnable_001 start");
    auto result = fastAudioStream->GetOffloadEnable();
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test GetSpatializationEnabled API
 * @tc.type  : FUNC
 * @tc.number: GetSpatializationEnabled_001
 * @tc.desc  : Test GetSpatializationEnabled interface.
 */
HWTEST_F(FastSystemStreamUnitTest, GetSpatializationEnabled_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);

    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetSpatializationEnabled_001 start");
    auto result = fastAudioStream->GetSpatializationEnabled();
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test GetHighResolutionEnabled API
 * @tc.type  : FUNC
 * @tc.number: GetHighResolutionEnabled_001
 * @tc.desc  : Test GetHighResolutionEnabled interface.
 */
HWTEST_F(FastSystemStreamUnitTest, GetHighResolutionEnabled_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);

    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetHighResolutionEnabled_001 start");
    auto result = fastAudioStream->GetHighResolutionEnabled();
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test SetDefaultOutputDevice API
 * @tc.type  : FUNC
 * @tc.number: SetDefaultOutputDevice_001
 * @tc.desc  : Test SetDefaultOutputDevice interface.
 */
HWTEST_F(FastSystemStreamUnitTest, SetDefaultOutputDevice_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);

    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetDefaultOutputDevice_001 start");
    DeviceType defaultOuputDevice = DeviceType::DEVICE_TYPE_INVALID;
    auto result = fastAudioStream->SetDefaultOutputDevice(defaultOuputDevice);
    EXPECT_EQ(result, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test GetAudioTimestampInfo with null process client
 * @tc.type  : FUNC
 * @tc.number: GetAudioTimestampInfo_001
 * @tc.desc  : Test GetAudioTimestampInfo returns error when process client is null
 */
HWTEST_F(FastSystemStreamUnitTest, GetAudioTimestampInfo_001, TestSize.Level0)
{
    Timestamp timestamp;
    stream_->processClient_ = nullptr;

    int32_t result = stream_->GetAudioTimestampInfo(timestamp, Timestamp::MONOTONIC);

    EXPECT_EQ(result, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test GetAudioTimestampInfo with invalid timestamp base
 * @tc.type  : FUNC
 * @tc.number: GetAudioTimestampInfo_002
 * @tc.desc  : Test GetAudioTimestampInfo returns error when timestamp base is not MONOTONIC
 */
HWTEST_F(FastSystemStreamUnitTest, GetAudioTimestampInfo_002, TestSize.Level0)
{
    Timestamp timestamp;

    int32_t result = stream_->GetAudioTimestampInfo(timestamp, Timestamp::BOOTTIME);

    EXPECT_EQ(result, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test GetAudioTimestampInfo when GetAudioTime returns false
 * @tc.type  : FUNC
 * @tc.number: GetAudioTimestampInfo_003
 * @tc.desc  : Test GetAudioTimestampInfo returns error when GetAudioTime returns false
 */
HWTEST_F(FastSystemStreamUnitTest, GetAudioTimestampInfo_003, TestSize.Level0)
{
    Timestamp timestamp;

    // Set expectation: mock process client's GetAudioTime returns false
    EXPECT_CALL(*mockProcessClient_, GetAudioTime(_, _, _))
        .WillOnce(Return(false));

    int32_t result = stream_->GetAudioTimestampInfo(timestamp, Timestamp::MONOTONIC);

    EXPECT_EQ(result, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test GetAudioTimestampInfo success case
 * @tc.type  : FUNC
 * @tc.number: GetAudioTimestampInfo_004
 * @tc.desc  : Test GetAudioTimestampInfo returns success and correctly populates timestamp
 */
HWTEST_F(FastSystemStreamUnitTest, GetAudioTimestampInfo_004, TestSize.Level0)
{
    Timestamp timestamp;

    uint32_t expectedFramePos = 100;
    int64_t expectedSec = 12345;
    int64_t expectedNsec = 67890;

    // Set expectation: mock process client's GetAudioTime returns true with test values
    EXPECT_CALL(*mockProcessClient_, GetAudioTime(_, _, _))
        .WillOnce(DoAll(
            SetArgReferee<0>(expectedFramePos),
            SetArgReferee<1>(expectedSec),
            SetArgReferee<2>(expectedNsec),
            Return(true)
        ));

    int32_t result = stream_->GetAudioTimestampInfo(timestamp, Timestamp::MONOTONIC);

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(timestamp.framePosition, expectedFramePos);
    EXPECT_EQ(timestamp.time.tv_sec, expectedSec);
    EXPECT_EQ(timestamp.time.tv_nsec, expectedNsec);
}

/**
 * @tc.name  : Test SetSwitchingStatus API
 * @tc.type  : FUNC
 * @tc.number: SetSwitchingStatus_001
 * @tc.desc  : Test SetSwitchingStatus interface.
 */
HWTEST_F(FastSystemStreamUnitTest, SetSwitchingStatus_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);

    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetSwitchingStatus_001 start");
    bool isSwitching = true;
    fastAudioStream->SetSwitchingStatus(isSwitching);
}

/**
 * @tc.name  : Test SetSwitchingStatus API
 * @tc.type  : FUNC
 * @tc.number: SetSwitchingStatus_002
 * @tc.desc  : Test SetSwitchingStatus interface.
 */
HWTEST_F(FastSystemStreamUnitTest, SetSwitchingStatus_002, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);

    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetSwitchingStatus_002 start");
    bool isSwitching = false;
    fastAudioStream->SetSwitchingStatus(isSwitching);
}

/**
 * @tc.name  : Test InitCallbackHandler API
 * @tc.type  : FUNC
 * @tc.number: InitCallbackHandler_001
 * @tc.desc  : Test InitCallbackHandler interface.
 */
HWTEST_F(FastSystemStreamUnitTest, InitCallbackHandler_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream =
        std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);

    fastAudioStream->InitCallbackHandler();
    EXPECT_NE(fastAudioStream->callbackHandler_, nullptr);
}

/**
 * @tc.name  : Test SafeSendCallbackEvent API
 * @tc.type  : FUNC
 * @tc.number: SafeSendCallbackEvent_001
 * @tc.desc  : Test SafeSendCallbackEvent interface.
 */
HWTEST_F(FastSystemStreamUnitTest, SafeSendCallbackEvent_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream =
        std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    uint32_t eventCode = 1;
    int64_t data = 0;

    fastAudioStream->SafeSendCallbackEvent(eventCode, data);
    EXPECT_EQ(fastAudioStream->callbackHandler_, nullptr);

    fastAudioStream->runnerReleased_ = false;
    fastAudioStream->InitCallbackHandler();
    fastAudioStream->SafeSendCallbackEvent(eventCode, data);
    EXPECT_NE(fastAudioStream->callbackHandler_, nullptr);

    fastAudioStream->runnerReleased_ = true;
    fastAudioStream->SafeSendCallbackEvent(eventCode, data);
    EXPECT_NE(fastAudioStream->callbackHandler_, nullptr);
}

/**
 * @tc.name  : Test HandleStateChangeEvent API
 * @tc.type  : FUNC
 * @tc.number: HandleStateChangeEvent_001
 * @tc.desc  : Test HandleStateChangeEvent interface.
 */
HWTEST_F(FastSystemStreamUnitTest, HandleStateChangeEvent_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream =
        std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    int64_t data = 0;
    auto callback = std::make_shared<AudioStreamCallbackTest>();

    data = FastAudioStream::HANDLER_PARAM_NEW;
    fastAudioStream->SetStreamCallback(callback);
    fastAudioStream->HandleStateChangeEvent(data);
    EXPECT_NE(fastAudioStream->streamCallback_.lock(), nullptr);

    data = FastAudioStream::HANDLER_PARAM_STOPPING;
    fastAudioStream->HandleStateChangeEvent(data);
    EXPECT_NE(fastAudioStream->streamCallback_.lock(), nullptr);
}

/**
 * @tc.name  : Test ParamsToStateCmdType API
 * @tc.type  : FUNC
 * @tc.number: ParamsToStateCmdType_001
 * @tc.desc  : Test ParamsToStateCmdType interface.
 */
HWTEST_F(FastSystemStreamUnitTest, ParamsToStateCmdType_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream =
        std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    int64_t params;
    State state;
    StateChangeCmdType cmdType;

    params = FastAudioStream::HANDLER_PARAM_NEW;
    fastAudioStream->ParamsToStateCmdType(params, state, cmdType);
    EXPECT_EQ(state, NEW);

    params = FastAudioStream::HANDLER_PARAM_PREPARED;
    fastAudioStream->ParamsToStateCmdType(params, state, cmdType);
    EXPECT_EQ(state, PREPARED);

    params = FastAudioStream::HANDLER_PARAM_RUNNING;
    fastAudioStream->ParamsToStateCmdType(params, state, cmdType);
    EXPECT_EQ(state, RUNNING);

    params = FastAudioStream::HANDLER_PARAM_STOPPED;
    fastAudioStream->ParamsToStateCmdType(params, state, cmdType);
    EXPECT_EQ(state, STOPPED);

    params = FastAudioStream::HANDLER_PARAM_RELEASED;
    fastAudioStream->ParamsToStateCmdType(params, state, cmdType);
    EXPECT_EQ(state, RELEASED);

    params = FastAudioStream::HANDLER_PARAM_PAUSED;
    fastAudioStream->ParamsToStateCmdType(params, state, cmdType);
    EXPECT_EQ(state, PAUSED);

    params = FastAudioStream::HANDLER_PARAM_STOPPING;
    fastAudioStream->ParamsToStateCmdType(params, state, cmdType);
    EXPECT_EQ(state, STOPPING);

    params = FastAudioStream::HANDLER_PARAM_RUNNING_FROM_SYSTEM;
    fastAudioStream->ParamsToStateCmdType(params, state, cmdType);
    EXPECT_EQ(state, RUNNING);

    params = FastAudioStream::HANDLER_PARAM_PAUSED_FROM_SYSTEM;
    fastAudioStream->ParamsToStateCmdType(params, state, cmdType);
    EXPECT_EQ(state, PAUSED);

    params = FastAudioStream::HANDLER_PARAM_INVALID;
    fastAudioStream->ParamsToStateCmdType(params, state, cmdType);
    EXPECT_EQ(state, INVALID);
}

/**
 * @tc.name  : Test SetSourceDuration API
 * @tc.type  : FUNC
 * @tc.number: SetSourceDuration_001
 * @tc.desc  : Test SetSourceDuration interface.
 */
HWTEST_F(FastSystemStreamUnitTest, SetSourceDuration_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_RECORD, appUid);
    int64_t duration = 0;
    int32_t res;
    AudioStreamParams info;
    info.format = AudioSampleFormat::SAMPLE_S16LE;
    info.encoding = AudioEncodingType::ENCODING_PCM;
    info.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    info.channels = AudioChannel::MONO;
    info.channelLayout = AudioChannelLayout::CH_LAYOUT_MONO;
    std::shared_ptr<AudioClientTracker> proxyObj = std::make_shared<AudioClientTrackerTest>();
    fastAudioStream->proxyObj_ = proxyObj;
    fastAudioStream->streamInfo_ = info;

    res = fastAudioStream->SetSourceDuration(duration);
    EXPECT_NE(res, SUCCESS);

    fastAudioStream->eMode_ = AUDIO_MODE_RECORD;
    fastAudioStream->state_ = NEW;
    res = fastAudioStream->SetAudioStreamInfo(info, proxyObj);
    res = fastAudioStream->SetSourceDuration(duration);
    EXPECT_NE(res, SUCCESS);
}

/**
 * @tc.name  : Test SetStreamCallback API
 * @tc.type  : FUNC
 * @tc.number: SetStreamCallback_001
 * @tc.desc  : Test SetStreamCallback interface.
 */
HWTEST_F(FastSystemStreamUnitTest, SetStreamCallback_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream =
        std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    int32_t ret;
    std::shared_ptr<AudioStreamCallback> callback = nullptr;

    ret = fastAudioStream->SetStreamCallback(callback);
    EXPECT_NE(ret, SUCCESS);

    fastAudioStream->state_ = NEW;
    callback = std::make_shared<AudioStreamCallbackTest>();
    ret = fastAudioStream->SetStreamCallback(callback);
    EXPECT_EQ(ret, SUCCESS);

    fastAudioStream->state_ = PREPARED;
    ret = fastAudioStream->SetStreamCallback(callback);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test GetAudioStreamInfo API
 * @tc.type  : FUNC
 * @tc.number: GetAudioStreamInfo_001
 * @tc.desc  : Test GetAudioStreamInfo interface.
 */
HWTEST_F(FastSystemStreamUnitTest, GetAudioStreamInfo_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    auto fastAudioStream =
        std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    EXPECT_NE(fastAudioStream, nullptr);

    AudioStreamParams audioStreamInfo;
    auto ret = fastAudioStream->GetAudioStreamInfo(audioStreamInfo);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test GetAudioSessionID API
 * @tc.type  : FUNC
 * @tc.number: GetAudioSessionID_001
 * @tc.desc  : Test GetAudioSessionID interface.
 */
HWTEST_F(FastSystemStreamUnitTest, GetAudioSessionID_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    auto fastAudioStream =
        std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    EXPECT_NE(fastAudioStream, nullptr);

    uint32_t sessionId = 0;
    auto ret = fastAudioStream->GetAudioSessionID(sessionId);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test GetAudioTime API
 * @tc.type  : FUNC
 * @tc.number: GetAudioTime_001
 * @tc.desc  : Test GetAudioTime interface.
 */
HWTEST_F(FastSystemStreamUnitTest, GetAudioTime_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    auto fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    EXPECT_NE(fastAudioStream, nullptr);

    Timestamp timestamp;
    auto base = static_cast<Timestamp::Timestampbase>(1);
    auto ret = fastAudioStream->GetAudioTime(timestamp, base);
    EXPECT_EQ(ret, false);

    ret = fastAudioStream->GetAudioTime(timestamp, Timestamp::Timestampbase::MONOTONIC);
    EXPECT_EQ(ret, false);

    AudioStreamParams info;
    info.format = AudioSampleFormat::SAMPLE_S16LE;
    info.encoding = AudioEncodingType::ENCODING_PCM;
    info.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    info.channels = AudioChannel::STEREO;
    info.channelLayout = AudioChannelLayout::CH_LAYOUT_MONO;
    std::shared_ptr<AudioClientTracker> proxyObj = std::make_shared<AudioClientTrackerTest>();
    fastAudioStream->SetAudioStreamInfo(info, proxyObj);

    ret = fastAudioStream->GetAudioTime(timestamp, Timestamp::Timestampbase::MONOTONIC);
    EXPECT_NE(ret, true);
}

/**
 * @tc.name  : Test GetBufferSize API
 * @tc.type  : FUNC
 * @tc.number: GetBufferSize_001
 * @tc.desc  : Test GetBufferSize interface.
 */
HWTEST_F(FastSystemStreamUnitTest, GetBufferSize_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream
        = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    EXPECT_NE(fastAudioStream, nullptr);

    size_t bufferSize = 0;
    auto ret = fastAudioStream->GetBufferSize(bufferSize);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test GetFrameCount API
 * @tc.type  : FUNC
 * @tc.number: GetFrameCount_001
 * @tc.desc  : Test GetFrameCount interface.
 */
HWTEST_F(FastSystemStreamUnitTest, GetFrameCount_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream
        = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    EXPECT_NE(fastAudioStream, nullptr);

    size_t frameCount = 0;
    auto ret = fastAudioStream->GetFrameCount(frameCount);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test GetLatency API
 * @tc.type  : FUNC
 * @tc.number: GetLatency_001
 * @tc.desc  : Test GetLatency interface.
 */
HWTEST_F(FastSystemStreamUnitTest, GetLatency_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream
        = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    EXPECT_NE(fastAudioStream, nullptr);

    uint64_t latency = 0;
    auto ret = fastAudioStream->GetLatency(latency);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test SetDuckVolume API
 * @tc.type  : FUNC
 * @tc.number: SetDuckVolume_001
 * @tc.desc  : Test SetDuckVolume interface.
 */
HWTEST_F(FastSystemStreamUnitTest, SetDuckVolume_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream
        = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    EXPECT_NE(fastAudioStream, nullptr);

    float duckVolume = 0;
    auto ret = fastAudioStream->SetDuckVolume(duckVolume);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test SetRenderRate API
 * @tc.type  : FUNC
 * @tc.number: SetRenderRate_001
 * @tc.desc  : Test SetRenderRate interface.
 */
HWTEST_F(FastSystemStreamUnitTest, SetRenderRate_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream
        = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    EXPECT_NE(fastAudioStream, nullptr);

    AudioRendererRate renderRate = AudioRendererRate::RENDER_RATE_NORMAL;
    auto ret = fastAudioStream->SetRenderRate(renderRate);
    EXPECT_EQ(ret, SUCCESS);

    renderRate = AudioRendererRate::RENDER_RATE_DOUBLE;
    ret = fastAudioStream->SetRenderRate(renderRate);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
}

/**
 * @tc.name  : Test SetRendererWriteCallback API
 * @tc.type  : FUNC
 * @tc.number: SetRendererWriteCallback_001
 * @tc.desc  : Test SetRendererWriteCallback interface.
 */
HWTEST_F(FastSystemStreamUnitTest, SetRendererWriteCallback_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream
        = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    EXPECT_NE(fastAudioStream, nullptr);

    auto callback = std::make_shared<AudioRendererWriteCallbackTest>();
    auto ret = fastAudioStream->SetRendererWriteCallback(callback);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test SetCapturerReadCallback API
 * @tc.type  : FUNC
 * @tc.number: SetCapturerReadCallback_001
 * @tc.desc  : Test SetCapturerReadCallback interface.
 */
HWTEST_F(FastSystemStreamUnitTest, SetCapturerReadCallback_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream
        = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_RECORD, appUid);
    EXPECT_NE(fastAudioStream, nullptr);

    auto callback = std::make_shared<AudioCapturerReadCallbackTest>();
    auto ret = fastAudioStream->SetCapturerReadCallback(callback);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test GetBufferDesc API
 * @tc.type  : FUNC
 * @tc.number: GetBufferDesc_001
 * @tc.desc  : Test GetBufferDesc interface.
 */
HWTEST_F(FastSystemStreamUnitTest, GetBufferDesc_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream
        = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    EXPECT_NE(fastAudioStream, nullptr);

    BufferDesc bufferDesc;
    auto ret = fastAudioStream->GetBufferDesc(bufferDesc);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);

    AudioStreamParams info;
    info.format = AudioSampleFormat::SAMPLE_S16LE;
    info.encoding = AudioEncodingType::ENCODING_PCM;
    info.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    info.channels = AudioChannel::STEREO;
    info.channelLayout = AudioChannelLayout::CH_LAYOUT_MONO;
    std::shared_ptr<AudioClientTracker> proxyObj = std::make_shared<AudioClientTrackerTest>();
    fastAudioStream->SetAudioStreamInfo(info, proxyObj);

    ret = fastAudioStream->GetBufferDesc(bufferDesc);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test GetBufQueueState API
 * @tc.type  : FUNC
 * @tc.number: GetBufQueueState_001
 * @tc.desc  : Test GetBufQueueState interface.
 */
HWTEST_F(FastSystemStreamUnitTest, GetBufQueueState_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream
        = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    EXPECT_NE(fastAudioStream, nullptr);

    BufferQueueState bufState;
    auto ret = fastAudioStream->GetBufQueueState(bufState);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test InitCallbackHandler API
 * @tc.type  : FUNC
 * @tc.number: InitCallbackHandler_002
 * @tc.desc  : Test InitCallbackHandler interface.
 */
HWTEST_F(FastSystemStreamUnitTest, InitCallbackHandler_002, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream =
        std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    ASSERT_TRUE(fastAudioStream != nullptr);

    fastAudioStream->InitCallbackHandler();
    EXPECT_NE(fastAudioStream->callbackHandler_, nullptr);
    fastAudioStream->InitCallbackHandler();
}

/**
 * @tc.name  : Test OnHandleData API
 * @tc.type  : FUNC
 * @tc.number: OnHandleData_002
 * @tc.desc  : Test OnHandleData interface.
 */
HWTEST_F(FastSystemStreamUnitTest, OnHandleData_002, TestSize.Level1)
{
    std::shared_ptr<AudioRendererWriteCallback> callback = std::make_shared<AudioRendererWriteCallbackTest>();
    AudioStreamParams tempParams = {};
    auto audioStream = IAudioStream::GetRecordStream(IAudioStream::PA_STREAM, tempParams, STREAM_MUSIC, getpid());
    std::shared_ptr<FastAudioStreamRenderCallback> fastAudioStreamRenderCallback;
    fastAudioStreamRenderCallback = std::make_shared<FastAudioStreamRenderCallback>(callback, *audioStream);
    ASSERT_TRUE(fastAudioStreamRenderCallback != nullptr);

    fastAudioStreamRenderCallback->rendererWriteCallback_ = std::make_shared<AudioRendererWriteCallbackTest>();
    size_t length = 0;
    fastAudioStreamRenderCallback->OnHandleData(length);
    fastAudioStreamRenderCallback->OnHandleData(length);
    EXPECT_TRUE(fastAudioStreamRenderCallback->hasFirstFrameWrited_);
}

/**
 * @tc.name  : Test InitCallbackHandler API
 * @tc.type  : FUNC
 * @tc.number: FetchDeviceForSplitStream_001
 * @tc.desc  : Test InitCallbackHandler interface.
 */
HWTEST_F(FastSystemStreamUnitTest, FetchDeviceForSplitStream_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream =
        std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    ASSERT_TRUE(fastAudioStream != nullptr);

    fastAudioStream->FetchDeviceForSplitStream();
}

/**
 * @tc.name  : Test InitCallbackHandler API
 * @tc.type  : FUNC
 * @tc.number: RestoreAudioStream_002
 * @tc.desc  : Test InitCallbackHandler interface.
 */
HWTEST_F(FastSystemStreamUnitTest, RestoreAudioStream_002, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream =
        std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    ASSERT_TRUE(fastAudioStream != nullptr);

    std::shared_ptr<AudioClientTracker> proxyObj = std::make_shared<AudioClientTrackerTest>();
    fastAudioStream->proxyObj_ = proxyObj;
    fastAudioStream->state_ = RUNNING;
    fastAudioStream->RestoreAudioStream();
}

/**
 * @tc.name  : Test CheckRestoreStatus API
 * @tc.type  : FUNC
 * @tc.number: CheckRestoreStatus_001
 * @tc.desc  : Test CheckRestoreStatus interface. - return RESTORE_TO_NORMAL when spkProcClientCb_ is null
 */
HWTEST_F(FastSystemStreamUnitTest, CheckRestoreStatus_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream =
        std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    EXPECT_NE(fastAudioStream, nullptr);

    fastAudioStream->spkProcClientCb_ = nullptr;
    fastAudioStream->micProcClientCb_ = nullptr;
    RestoreStatus status = fastAudioStream->CheckRestoreStatus();
    EXPECT_EQ(status, NEED_RESTORE_TO_NORMAL);
}

/**
 * @tc.name  : Test SetCallbacksWhenRestore API
 * @tc.type  : FUNC
 * @tc.number: SetCallbacksWhenRestore_001
 * @tc.desc  : Test SetCallbacksWhenRestore interface using unsupported parameters.
 */
HWTEST_F(FastSystemStreamUnitTest, SetCallbacksWhenRestore_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream =
        std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    fastAudioStream->processClient_ = nullptr;
    fastAudioStream->eMode_ = AUDIO_MODE_PLAYBACK;
    int ret = fastAudioStream->SetCallbacksWhenRestore();
    EXPECT_EQ(ERROR_INVALID_PARAM, ret);
}

/**
 * @tc.name  : Test SetCallbacksWhenRestore API
 * @tc.type  : FUNC
 * @tc.number: SetCallbacksWhenRestore_002
 * @tc.desc  : Test SetCallbacksWhenRestore interface using unsupported parameters.
 */
HWTEST_F(FastSystemStreamUnitTest, SetCallbacksWhenRestore_002, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream =
        std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    fastAudioStream->processClient_ = nullptr;
    fastAudioStream->eMode_ = AUDIO_MODE_RECORD;
    int ret = fastAudioStream->SetCallbacksWhenRestore();
    EXPECT_EQ(ERROR_INVALID_PARAM, ret);
}

/**
 * @tc.name  : Test RestoreAudioStream API
 * @tc.type  : FUNC
 * @tc.number: RestoreAudioStream_003
 * @tc.desc  : Test RestoreAudioStream interface using unsupported parameters.
 */
HWTEST_F(FastSystemStreamUnitTest, RestoreAudioStream_003, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream =
        std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    bool needStoreState = true;
    fastAudioStream->proxyObj_ = nullptr;
    fastAudioStream->state_ = RELEASED;
    int ret = fastAudioStream->RestoreAudioStream(needStoreState);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test RestoreAudioStream API
 * @tc.type  : FUNC
 * @tc.number: RestoreAudioStream_004
 * @tc.desc  : Test RestoreAudioStream interface using unsupported parameters.
 */
HWTEST_F(FastSystemStreamUnitTest, RestoreAudioStream_004, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream =
        std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    bool needStoreState = true;
    std::shared_ptr<AudioClientTracker> proxyObj = std::make_shared<AudioClientTrackerTest>();
    fastAudioStream->proxyObj_ = proxyObj;
    fastAudioStream->state_ = NEW;
    int ret = fastAudioStream->RestoreAudioStream(needStoreState);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name  : Test RestoreAudioStream API
 * @tc.type  : FUNC
 * @tc.number: RestoreAudioStream_005
 * @tc.desc  : Test RestoreAudioStream interface using unsupported parameters.
 */
HWTEST_F(FastSystemStreamUnitTest, RestoreAudioStream_005, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream =
        std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    bool needStoreState = true;
    std::shared_ptr<AudioClientTracker> proxyObj = std::make_shared<AudioClientTrackerTest>();
    fastAudioStream->proxyObj_ = proxyObj;
    fastAudioStream->state_ = INVALID;
    int ret = fastAudioStream->RestoreAudioStream(needStoreState);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name  : Test RestoreAudioStream API
 * @tc.type  : FUNC
 * @tc.number: RestoreAudioStream_006
 * @tc.desc  : Test RestoreAudioStream interface using unsupported parameters.
 */
HWTEST_F(FastSystemStreamUnitTest, RestoreAudioStream_006, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream =
        std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    bool needStoreState = true;
    std::shared_ptr<AudioClientTracker> proxyObj = std::make_shared<AudioClientTrackerTest>();
    fastAudioStream->proxyObj_ = proxyObj;
    fastAudioStream->state_ = RELEASED;
    int ret = fastAudioStream->RestoreAudioStream(needStoreState);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name  : Test RestoreAudioStream API
 * @tc.type  : FUNC
 * @tc.number: RestoreAudioStream_007
 * @tc.desc  : Test RestoreAudioStream interface using unsupported parameters.
 */
HWTEST_F(FastSystemStreamUnitTest, RestoreAudioStream_007, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream =
        std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    EXPECT_EQ(fastAudioStream->RestoreAudioStream(true), false);
}

/**
 * @tc.name  : Test RestoreAudioStream API
 * @tc.type  : FUNC
 * @tc.number: RestoreAudioStream_008
 * @tc.desc  : Test RestoreAudioStream interface using unsupported parameters.
 */
HWTEST_F(FastSystemStreamUnitTest, RestoreAudioStream_008, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream =
        std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    fastAudioStream->state_ = NEW;
    EXPECT_NE(fastAudioStream->RestoreAudioStream(true), true);

    fastAudioStream->state_ = RUNNING;
    EXPECT_EQ(fastAudioStream->RestoreAudioStream(true), false);

    fastAudioStream->state_ = PAUSED;
    EXPECT_EQ(fastAudioStream->RestoreAudioStream(true), false);

    fastAudioStream->state_ = STOPPED;
    EXPECT_EQ(fastAudioStream->RestoreAudioStream(true), false);

    fastAudioStream->state_ = STOPPING;
    EXPECT_EQ(fastAudioStream->RestoreAudioStream(true), false);
}
/**
 * @tc.name  : Test GetDefaultOutputDevice API
 * @tc.type  : FUNC
 * @tc.number: GetDefaultOutputDevice_001
 * @tc.desc  : Test GetDefaultOutputDevice interface using unsupported parameters.
 */
HWTEST_F(FastSystemStreamUnitTest, GetDefaultOutputDevice_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream =
        std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    fastAudioStream->defaultOutputDevice_ = DeviceType::DEVICE_TYPE_SPEAKER;

    DeviceType result = fastAudioStream->GetDefaultOutputDevice();

    EXPECT_EQ(result, DeviceType::DEVICE_TYPE_SPEAKER);
}

/**
 * @tc.name  : FastAudioStream_IsRestoreNeeded_001
 * @tc.type  : FUNC
 * @tc.number: IsRestoreNeeded_001
 * @tc.desc  : Test FastAudioStream IsRestoreNeeded() when processClient_ is null
 */
HWTEST(FastAudioStreamUnitTest, IsRestoreNeeded_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream =
        std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);

    fastAudioStream->processClient_ = nullptr;
    EXPECT_EQ(fastAudioStream->IsRestoreNeeded(), false);
}

/**
 * @tc.name  : FastAudioStream_IsRestoreNeeded_002
 * @tc.type  : FUNC
 * @tc.number: IsRestoreNeeded_002
 * @tc.desc  : Test FastAudioStream IsRestoreNeeded() when processClient_ not null but no callback set
 */
HWTEST(FastAudioStreamUnitTest, IsRestoreNeeded_002, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream =
        std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);

    auto mockProcessClient = std::make_shared<MockAudioProcessInClient>();
    fastAudioStream->processClient_ = mockProcessClient;
    fastAudioStream->spkProcClientCb_ = nullptr;
    fastAudioStream->micProcClientCb_ = nullptr;

    EXPECT_EQ(fastAudioStream->IsRestoreNeeded(), true);
}

/**
 * @tc.name  : FastAudioStream_IsRestoreNeeded_003
 * @tc.type  : FUNC
 * @tc.number: IsRestoreNeeded_003
 * @tc.desc  : Test FastAudioStream IsRestoreNeeded() when spk callback set and processClient return false
 */
HWTEST(FastAudioStreamUnitTest, IsRestoreNeeded_003, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream =
        std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);

    auto mockProcessClient = std::make_shared<MockAudioProcessInClient>();
    EXPECT_CALL(*mockProcessClient, IsRestoreNeeded()).WillOnce(Return(false));
    fastAudioStream->processClient_ = mockProcessClient;
    std::shared_ptr<AudioRendererWriteCallback> spkCallback = std::make_shared<AudioRendererWriteCallbackTest>();
    AudioStreamParams tempParams = {};
    auto audioStream = IAudioStream::GetRecordStream(IAudioStream::PA_STREAM, tempParams, STREAM_MUSIC, getpid());
    fastAudioStream->spkProcClientCb_ = std::make_shared<FastAudioStreamRenderCallback>(spkCallback, *audioStream);
    fastAudioStream->micProcClientCb_ = nullptr;

    EXPECT_EQ(fastAudioStream->IsRestoreNeeded(), false);
}

/**
 * @tc.name  : FastAudioStream_IsRestoreNeeded_004
 * @tc.type  : FUNC
 * @tc.number: IsRestoreNeeded_004
 * @tc.desc  : Test FastAudioStream IsRestoreNeeded() when mic callback set and processClient return true
 */
HWTEST(FastAudioStreamUnitTest, IsRestoreNeeded_004, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream =
        std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);

    auto mockProcessClient = std::make_shared<MockAudioProcessInClient>();
    EXPECT_CALL(*mockProcessClient, IsRestoreNeeded()).WillOnce(Return(true));
    fastAudioStream->processClient_ = mockProcessClient;
    fastAudioStream->spkProcClientCb_ = nullptr;
    std::shared_ptr<AudioCapturerReadCallback> micCallback = std::make_shared<AudioCapturerReadCallbackTest>();
    fastAudioStream->micProcClientCb_ = std::make_shared<FastAudioStreamCaptureCallback>(micCallback);

    EXPECT_EQ(fastAudioStream->IsRestoreNeeded(), true);
}

/**
 * @tc.name  : FastAudioStream_IsRestoreNeeded_005
 * @tc.type  : FUNC
 * @tc.number: IsRestoreNeeded_005
 * @tc.desc  : Test FastAudioStream IsRestoreNeeded() when both callbacks set and processClient return false
 */
HWTEST(FastAudioStreamUnitTest, IsRestoreNeeded_005, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream =
        std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);

    auto mockProcessClient = std::make_shared<MockAudioProcessInClient>();
    EXPECT_CALL(*mockProcessClient, IsRestoreNeeded()).WillOnce(Return(false));
    fastAudioStream->processClient_ = mockProcessClient;

    std::shared_ptr<AudioRendererWriteCallback> spkCallback = std::make_shared<AudioRendererWriteCallbackTest>();
    AudioStreamParams tempParams = {};
    auto audioStream = IAudioStream::GetRecordStream(IAudioStream::PA_STREAM, tempParams, STREAM_MUSIC, getpid());
    fastAudioStream->spkProcClientCb_ = std::make_shared<FastAudioStreamRenderCallback>(spkCallback, *audioStream);

    std::shared_ptr<AudioCapturerReadCallback> micCallback = std::make_shared<AudioCapturerReadCallbackTest>();
    fastAudioStream->micProcClientCb_ = std::make_shared<FastAudioStreamCaptureCallback>(micCallback);

    EXPECT_EQ(fastAudioStream->IsRestoreNeeded(), false);
}
} // namespace AudioStandard
} // namespace OHOS
