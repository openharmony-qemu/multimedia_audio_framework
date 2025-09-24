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

#include "audio_capturer_unit_test.h"

#include <thread>
#include <memory>
#include <vector>

#include "audio_capturer.h"
#include "audio_capturer_private.h"
#include "audio_errors.h"
#include "audio_info.h"
#include "audio_system_manager.h"
#include "fast_audio_stream.h"
#include "audio_client_tracker_callback_service.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace {
    constexpr float DUCK_VOLUME = 0.2f;
} // namespace

class CapturerPositionCallbackTestStub : public CapturerPositionCallback {
public:
    void OnMarkReached(const int64_t &framePosition) override {}
};

class CapturerPeriodPositionCallbackTestStub : public CapturerPeriodPositionCallback {
public:
    void OnPeriodReached(const int64_t &frameNumber) override {}
};

class AudioCapturerCallbackTestStub : public AudioCapturerCallback {
public:
    void OnInterrupt(const InterruptEvent &interruptEvent) override
    {
        state_ = interruptEvent.hintType;
    }
    void OnStateChange(const CapturerState state) override {}
public:
    InterruptHint state_;
};

class RendererPositionCallbackTestStub : public RendererPositionCallback {
public:
    void OnMarkReached(const int64_t &framePosition) override {}
};

class RendererPeriodPositionCallbackTestStub : public RendererPeriodPositionCallback {
public:
    void OnPeriodReached(const int64_t &frameNumber) override {}
};

class CapturerPolicyServiceDiedCallbackTestStub : public CapturerPolicyServiceDiedCallback {
};

static int g_writeOverflowNum = 1000;
class TestAudioStremStub : public FastAudioStream {
public:
    TestAudioStremStub() : FastAudioStream(AudioStreamType::STREAM_MUSIC,
        AudioMode::AUDIO_MODE_RECORD, 0) {}
    uint32_t GetOverflowCount() override { return g_writeOverflowNum; }
    State GetState() override { return state_; }
    bool StopAudioStream() override { return true; }
    bool StartAudioStream(StateChangeCmdType cmdType,
        AudioStreamDeviceChangeReasonExt reason) override { return true; }
    bool ReleaseAudioStream(bool releaseRunner, bool isSwitchStream) override { return true; }
    float GetDuckVolume() override { return DUCK_VOLUME; }

    State state_ = State::RUNNING;
};

class FastAudioStreamFork : public FastAudioStream {
public:
    FastAudioStreamFork() : FastAudioStream(AudioStreamType::STREAM_MUSIC,
        AudioMode::AUDIO_MODE_RECORD, 0) {}
    uint32_t GetOverflowCount() override { return g_writeOverflowNum; }
    State GetState() override { return state_; }
    int32_t SetAudioStreamInfo(const AudioStreamParams info,
        const std::shared_ptr<AudioClientTracker> &proxyObj,
        const AudioPlaybackCaptureConfig &config = AudioPlaybackCaptureConfig()) override { return SUCCESS; }
    bool StopAudioStream() override { return true; }
    bool StartAudioStream(StateChangeCmdType cmdType,
        AudioStreamDeviceChangeReasonExt reason) override { return true; }
    IAudioStream::StreamClass GetStreamClass() override { return IAudioStream::PA_STREAM; }
    RestoreStatus SetRestoreStatus(RestoreStatus restoreStatus) override { return restoreStatus; }

    State state_ = State::RUNNING;
};


class FastAudioStreamFork2 : public FastAudioStream {
public:
    FastAudioStreamFork2() : FastAudioStream(AudioStreamType::STREAM_MUSIC,
        AudioMode::AUDIO_MODE_RECORD, 0) {}
    uint32_t GetOverflowCount() override { return g_writeOverflowNum; }
    State GetState() override { return RUNNING; }
    int32_t SetAudioStreamInfo(const AudioStreamParams info,
        const std::shared_ptr<AudioClientTracker> &proxyObj,
        const AudioPlaybackCaptureConfig &config = AudioPlaybackCaptureConfig()) override { return SUCCESS; }
    bool StopAudioStream() override { return true; }
    bool StartAudioStream(StateChangeCmdType cmdType,
        AudioStreamDeviceChangeReasonExt reason) override { return true; }
    IAudioStream::StreamClass GetStreamClass() override { return IAudioStream::FAST_STREAM; }
    void GetAudioPipeType(AudioPipeType &pipeType) override { pipeType = PIPE_TYPE_LOWLATENCY_IN; }
    RestoreStatus SetRestoreStatus(RestoreStatus restoreStatus) override { return restoreStatus; }
};

class FastAudioStreamFork3 : public FastAudioStream {
public:
    FastAudioStreamFork3() : FastAudioStream(AudioStreamType::STREAM_MUSIC,
        AudioMode::AUDIO_MODE_RECORD, 0) {}
    uint32_t GetOverflowCount() override { return g_writeOverflowNum; }
    State GetState() override { return NEW; }
    int32_t SetAudioStreamInfo(const AudioStreamParams info,
        const std::shared_ptr<AudioClientTracker> &proxyObj,
        const AudioPlaybackCaptureConfig &config = AudioPlaybackCaptureConfig()) override { return SUCCESS; }
    bool StopAudioStream() override { return true; }
    bool StartAudioStream(StateChangeCmdType cmdType,
        AudioStreamDeviceChangeReasonExt reason) override { return true; }
    IAudioStream::StreamClass GetStreamClass() override { return IAudioStream::FAST_STREAM; }
    void GetAudioPipeType(AudioPipeType &pipeType) override { pipeType = PIPE_TYPE_CALL_IN; }
    RestoreStatus SetRestoreStatus(RestoreStatus restoreStatus) override { return restoreStatus; }
};

class FastAudioStreamFork4 : public FastAudioStream {
public:
    FastAudioStreamFork4() : FastAudioStream(AudioStreamType::STREAM_MUSIC,
        AudioMode::AUDIO_MODE_RECORD, 0) {}
    uint32_t GetOverflowCount() override { return g_writeOverflowNum; }
    State GetState() override { return state_; }
    int32_t SetAudioStreamInfo(const AudioStreamParams info,
        const std::shared_ptr<AudioClientTracker> &proxyObj,
        const AudioPlaybackCaptureConfig &config = AudioPlaybackCaptureConfig()) override { return SUCCESS; }
    bool StopAudioStream() override { return true; }
    bool StartAudioStream(StateChangeCmdType cmdType,
        AudioStreamDeviceChangeReasonExt reason) override { return true; }
    IAudioStream::StreamClass GetStreamClass() override { return IAudioStream::FAST_STREAM; }
    void GetAudioPipeType(AudioPipeType &pipeType) override { pipeType = PIPE_TYPE_UNKNOWN; }
    RestoreStatus SetRestoreStatus(RestoreStatus restoreStatus) override { return restoreStatus; }

    State state_ = State::RUNNING;
};

class CapturerFastStatusChangeCallbackTest : public AudioCapturerFastStatusChangeCallback {
public:
    void OnFastStatusChange(FastStatus status) override { return; }
};

/**
* @tc.name  : Test MISCELLANEOUS classes of module audio capturer.
* @tc.number: Audio_Capturer_MISCELLANEOUS_001.
* @tc.desc  : Test AudioCapturerInterruptCallbackImpl. Functions should
*             work without throwing any exception.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_MISCELLANEOUS_001, TestSize.Level1)
{
    AppInfo appInfo = {};
    auto capturer = std::make_shared<AudioCapturerPrivate>(STREAM_MUSIC, appInfo, false);

    AudioStreamParams tempParams = {};

    auto audioStream = IAudioStream::GetRecordStream(IAudioStream::FAST_STREAM, tempParams,
        STREAM_MUSIC, getpid());

    auto interruptCallback = std::make_unique<AudioCapturerInterruptCallbackImpl>(nullptr);
    interruptCallback->UpdateAudioStream(audioStream);
    EXPECT_EQ(interruptCallback->audioStream_, audioStream);

    capturer->audioStream_ = audioStream;

    InterruptEventInternal interruptEventResume {InterruptType::INTERRUPT_TYPE_BEGIN,
        InterruptForceType::INTERRUPT_SHARE, InterruptHint::INTERRUPT_HINT_RESUME, 0.0};
    InterruptEventInternal interruptEventInternalResume {InterruptType::INTERRUPT_TYPE_BEGIN,
        InterruptForceType::INTERRUPT_FORCE, InterruptHint::INTERRUPT_HINT_RESUME, 0.0};
    InterruptEventInternal interruptEventInternalPause {InterruptType::INTERRUPT_TYPE_BEGIN,
        InterruptForceType::INTERRUPT_FORCE, InterruptHint::INTERRUPT_HINT_PAUSE, 0.0};
    InterruptEventInternal interruptEventInternalStop {InterruptType::INTERRUPT_TYPE_BEGIN,
        InterruptForceType::INTERRUPT_FORCE, InterruptHint::INTERRUPT_HINT_STOP, 0.0};
    InterruptEventInternal interruptEventInternalNone {InterruptType::INTERRUPT_TYPE_BEGIN,
        InterruptForceType::INTERRUPT_FORCE, InterruptHint::INTERRUPT_HINT_NONE, 0.0};

    shared_ptr<AudioCapturerCallbackTestStub> cb = make_shared<AudioCapturerCallbackTestStub>();
    interruptCallback->SaveCallback(cb);
    interruptCallback->OnInterrupt(interruptEventResume);
    EXPECT_EQ(cb->state_, InterruptHint::INTERRUPT_HINT_RESUME);

    auto testAudioStremStub = std::make_shared<TestAudioStremStub>();
    interruptCallback->audioStream_ = testAudioStremStub;
    testAudioStremStub->state_ = State::PAUSED;
    interruptCallback->isForcePaused_ = true;
    interruptCallback->OnInterrupt(interruptEventInternalResume);
    EXPECT_EQ(cb->state_, InterruptHint::INTERRUPT_HINT_RESUME);

    testAudioStremStub->state_ = State::RUNNING;
    interruptCallback->OnInterrupt(interruptEventInternalPause);
    EXPECT_EQ(cb->state_, InterruptHint::INTERRUPT_HINT_PAUSE);

    interruptCallback->OnInterrupt(interruptEventInternalStop);
    EXPECT_EQ(cb->state_, InterruptHint::INTERRUPT_HINT_STOP);

    interruptCallback->OnInterrupt(interruptEventInternalNone);
    EXPECT_EQ(cb->state_, InterruptHint::INTERRUPT_HINT_NONE);
}

/**
* @tc.name  : Test MISCELLANEOUS classes of module audio capturer.
* @tc.number: Audio_Capturer_MISCELLANEOUS_002.
* @tc.desc  : Test AudioCapturerPrivate. Functions should
*             work without throwing any exception.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_MISCELLANEOUS_002, TestSize.Level1)
{
    AppInfo appInfo = {};
    auto capturer = std::make_shared<AudioCapturerPrivate>(STREAM_MUSIC, appInfo, false);

    AudioStreamParams tempParams = {};
    capturer->audioStream_ = IAudioStream::GetRecordStream(IAudioStream::PA_STREAM,
        tempParams, STREAM_MUSIC, getpid());

    capturer->FindStreamTypeBySourceType(SourceType::SOURCE_TYPE_VOICE_COMMUNICATION);
    capturer->FindStreamTypeBySourceType(SourceType::SOURCE_TYPE_VIRTUAL_CAPTURE);
    capturer->FindStreamTypeBySourceType(SourceType::SOURCE_TYPE_WAKEUP);
    capturer->FindStreamTypeBySourceType(SourceType::SOURCE_TYPE_VOICE_CALL);
    capturer->FindStreamTypeBySourceType(SourceType::SOURCE_TYPE_INVALID);

    std::vector<SourceType> targetSources;
    capturer->SetAudioSourceConcurrency(targetSources);
    targetSources.push_back(SourceType::SOURCE_TYPE_VOICE_COMMUNICATION);
    auto status = capturer->SetAudioSourceConcurrency(targetSources);
    EXPECT_EQ(status, SUCCESS);

    AudioInterrupt audioInterrupt;
    capturer->GetAudioInterrupt(audioInterrupt);

    capturer->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    status = capturer->SetCaptureMode(AudioCaptureMode::CAPTURE_MODE_CALLBACK);

    EXPECT_EQ(status, ERR_ILLEGAL_STATE);
}

/**
* @tc.name  : Test MISCELLANEOUS classes of module audio capturer.
* @tc.number: Audio_Capturer_MISCELLANEOUS_003.
* @tc.desc  : Test AudioCapturerPrivate. Functions should
*             work without throwing any exception.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_MISCELLANEOUS_003, TestSize.Level1)
{
    AppInfo appInfo = {};
    auto capturer = std::make_shared<AudioCapturerPrivate>(STREAM_MUSIC, appInfo, false);

    // AudioStreamParams tempParams = {};
    capturer->audioStream_ = std::make_shared<TestAudioStremStub>();
    auto audioStream2 = std::make_shared<TestAudioStremStub>();

    capturer->WriteOverflowEvent();

    capturer->audioPolicyServiceDiedCallback_ = std::make_shared<CapturerPolicyServiceDiedCallbackTestStub>();
    auto status = capturer->RemoveCapturerPolicyServiceDiedCallback();
    EXPECT_EQ(status, 0);

    IAudioStream::SwitchInfo info;
    info.eStreamType = STREAM_MUSIC;
    info.capturerInfo.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    info.frameMarkPosition = 1;
    info.framePeriodNumber = 1;
    info.state = State::PREPARED;
    info.renderPositionCb = std::make_shared<RendererPositionCallbackTestStub>();
    info.capturePositionCb  = std::make_shared<CapturerPositionCallbackTestStub>();
    info.renderPeriodPositionCb = std::make_shared<RendererPeriodPositionCallbackTestStub>();
    info.capturePeriodPositionCb = std::make_shared<CapturerPeriodPositionCallbackTestStub>();
    capturer->SetSwitchInfo(info, audioStream2);

    AUDIO_INFO_LOG("SetSwitchInfo");
    capturer->SetSwitchInfo(info, capturer->audioStream_);

    RestoreInfo restoreInfo;
    capturer->audioInterruptCallback_ = std::make_unique<AudioCapturerInterruptCallbackImpl>(nullptr);
    auto switchResult = capturer->SwitchToTargetStream(IAudioStream::StreamClass::VOIP_STREAM, restoreInfo);
    EXPECT_EQ(switchResult, true);
}

/**
 * @tc.name  : Test AudioCapturerPrivate API
 * @tc.type  : FUNC
 * @tc.number: AudioCapturerPrivate_004
 * @tc.desc  : Test AudioCapturerPrivate::SetSwitchInfo
 */
HWTEST(AudioCapturerUnitTest, AudioCapturerPrivate_004, TestSize.Level1)
{
    AppInfo appInfo = {};
    auto capturer = std::make_shared<AudioCapturerPrivate>(AudioStreamType::STREAM_VOICE_CALL, appInfo, true);

    IAudioStream::SwitchInfo info;
    auto audioStream = std::make_shared<TestAudioStremStub>();

    info.renderPositionCb = std::make_shared<RendererPositionCallbackTestStub>();
    info.framePeriodNumber = 1;
    info.capturePositionCb  = std::make_shared<CapturerPositionCallbackTestStub>();
    info.renderPeriodPositionCb = std::make_shared<RendererPeriodPositionCallbackTestStub>();
    info.framePeriodNumber = 1;
    info.capturePeriodPositionCb = std::make_shared<CapturerPeriodPositionCallbackTestStub>();

    capturer->SetSwitchInfo(info, audioStream);
}

/**
 * @tc.name  : Test AudioCapturerPrivate API
 * @tc.type  : FUNC
 * @tc.number: AudioCapturerPrivate_005
 * @tc.desc  : Test AudioCapturerPrivate::SetSwitchInfo
 */
HWTEST(AudioCapturerUnitTest, AudioCapturerPrivate_005, TestSize.Level1)
{
    AppInfo appInfo = {};
    auto capturer = std::make_shared<AudioCapturerPrivate>(AudioStreamType::STREAM_VOICE_CALL, appInfo, true);

    IAudioStream::SwitchInfo info;
    auto audioStream = std::make_shared<TestAudioStremStub>();

    info.renderPositionCb = std::make_shared<RendererPositionCallbackTestStub>();
    info.framePeriodNumber = 0;
    info.capturePositionCb  = std::make_shared<CapturerPositionCallbackTestStub>();
    info.renderPeriodPositionCb = std::make_shared<RendererPeriodPositionCallbackTestStub>();
    info.framePeriodNumber = 0;
    info.capturePeriodPositionCb = std::make_shared<CapturerPeriodPositionCallbackTestStub>();

    capturer->SetSwitchInfo(info, audioStream);
}

/**
 * @tc.name  : Test AudioCapturerPrivate API
 * @tc.type  : FUNC
 * @tc.number: AudioCapturerPrivate_006
 * @tc.desc  : Test AudioCapturerPrivate::SetSwitchInfo
 */
HWTEST(AudioCapturerUnitTest, AudioCapturerPrivate_006, TestSize.Level1)
{
    AppInfo appInfo = {};
    auto capturer = std::make_shared<AudioCapturerPrivate>(AudioStreamType::STREAM_VOICE_CALL, appInfo, true);

    IAudioStream::SwitchInfo info;
    auto audioStream = std::make_shared<TestAudioStremStub>();

    capturer->SetSwitchInfo(info, audioStream);
}

/**
 * @tc.name  : Test AudioCapturerPrivate API
 * @tc.type  : FUNC
 * @tc.number: AudioCapturerPrivate_007
 * @tc.desc  : Test AudioCapturerPrivate::SetSwitchInfo
 */
HWTEST(AudioCapturerUnitTest, AudioCapturerPrivate_007, TestSize.Level1)
{
    AppInfo appInfo = {};
    auto capturer = std::make_shared<AudioCapturerPrivate>(AudioStreamType::STREAM_VOICE_CALL, appInfo, true);
    ASSERT_NE(capturer, nullptr);

    IAudioStream::SwitchInfo info;
    auto audioStream = std::make_shared<FastAudioStreamFork>();
    ASSERT_NE(audioStream, nullptr);

    info.renderPositionCb = std::make_shared<RendererPositionCallbackTestStub>();
    info.framePeriodNumber = 1;
    info.capturePositionCb  = std::make_shared<CapturerPositionCallbackTestStub>();
    info.renderPeriodPositionCb = std::make_shared<RendererPeriodPositionCallbackTestStub>();
    info.framePeriodNumber = 1;
    info.capturePeriodPositionCb = std::make_shared<CapturerPeriodPositionCallbackTestStub>();

    auto ret = capturer->SetSwitchInfo(info, audioStream);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCapturerPrivate API
 * @tc.type  : FUNC
 * @tc.number: AudioCapturerPrivate_008
 * @tc.desc  : Test AudioCapturerPrivate::SetSwitchInfo
 */
HWTEST(AudioCapturerUnitTest, AudioCapturerPrivate_008, TestSize.Level1)
{
    AppInfo appInfo = {};
    auto capturer = std::make_shared<AudioCapturerPrivate>(AudioStreamType::STREAM_VOICE_CALL, appInfo, true);
    ASSERT_NE(capturer, nullptr);

    IAudioStream::SwitchInfo info;
    auto audioStream = std::make_shared<FastAudioStreamFork>();
    ASSERT_NE(audioStream, nullptr);

    info.renderPositionCb = std::make_shared<RendererPositionCallbackTestStub>();
    info.framePeriodNumber = 0;
    info.capturePositionCb  = std::make_shared<CapturerPositionCallbackTestStub>();
    info.renderPeriodPositionCb = std::make_shared<RendererPeriodPositionCallbackTestStub>();
    info.framePeriodNumber = 0;
    info.capturePeriodPositionCb = std::make_shared<CapturerPeriodPositionCallbackTestStub>();

    auto ret = capturer->SetSwitchInfo(info, audioStream);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCapturerPrivate API
 * @tc.type  : FUNC
 * @tc.number: AudioCapturerPrivate_009
 * @tc.desc  : Test AudioCapturerPrivate::SetSwitchInfo
 */
HWTEST(AudioCapturerUnitTest, AudioCapturerPrivate_009, TestSize.Level1)
{
    AppInfo appInfo = {};
    auto capturer = std::make_shared<AudioCapturerPrivate>(AudioStreamType::STREAM_VOICE_CALL, appInfo, true);
    ASSERT_NE(capturer, nullptr);

    IAudioStream::SwitchInfo info;
    auto audioStream = std::make_shared<FastAudioStreamFork>();
    ASSERT_NE(audioStream, nullptr);

    info.renderPositionCb = nullptr;
    info.framePeriodNumber = 0;
    info.capturePositionCb  = nullptr;
    info.renderPeriodPositionCb = nullptr;
    info.framePeriodNumber = 0;
    info.capturePeriodPositionCb = nullptr;

    auto ret = capturer->SetSwitchInfo(info, audioStream);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCapturerPrivate API
 * @tc.type  : FUNC
 * @tc.number: AudioCapturerPrivate_018
 * @tc.desc  : Test AudioCapturerPrivate::HandleAudioInterruptWhenServerDied
 */
HWTEST(AudioCapturerUnitTest, AudioCapturerPrivate_018, TestSize.Level1)
{
    AppInfo appInfo = {};
    auto capturer = std::make_shared<AudioCapturerPrivate>(AudioStreamType::STREAM_VOICE_CALL, appInfo, true);
    ASSERT_NE(capturer, nullptr);

    capturer->audioStream_ = std::make_shared<FastAudioStreamFork2>();

    capturer->HandleAudioInterruptWhenServerDied();
    EXPECT_EQ(capturer->GetStatusInner(), CapturerState::CAPTURER_RUNNING);
}

/**
 * @tc.name  : Test AudioCapturerPrivate API
 * @tc.type  : FUNC
 * @tc.number: AudioCapturerPrivate_019
 * @tc.desc  : Test AudioCapturerPrivate::HandleAudioInterruptWhenServerDied
 */
HWTEST(AudioCapturerUnitTest, AudioCapturerPrivate_019, TestSize.Level1)
{
    AppInfo appInfo = {};
    auto capturer = std::make_shared<AudioCapturerPrivate>(AudioStreamType::STREAM_VOICE_CALL, appInfo, true);
    ASSERT_NE(capturer, nullptr);

    capturer->audioStream_ = std::make_shared<FastAudioStreamFork3>();

    capturer->HandleAudioInterruptWhenServerDied();
    EXPECT_EQ(capturer->GetStatusInner(), CapturerState::CAPTURER_NEW);
}

/**
 * @tc.name  : Test AudioCapturerStateChangeCallbackImpl API
 * @tc.type  : FUNC
 * @tc.number: AudioCapturerPrivate_023
 * @tc.desc  : Test AudioCapturerStateChangeCallbackImpl::RemoveCapturerInfoChangeCallback
 */
HWTEST(AudioCapturerUnitTest, AudioCapturerPrivate_023, TestSize.Level1)
{
    auto audioCapturerStateChangeCallbackImpl = std::make_shared<AudioCapturerStateChangeCallbackImpl>();
    ASSERT_NE(audioCapturerStateChangeCallbackImpl, nullptr);

    std::shared_ptr<AudioCapturerInfoChangeCallback> callback = nullptr;

    audioCapturerStateChangeCallbackImpl->
        capturerInfoChangeCallbacklist_.push_back(std::make_shared<AudioCapturerInfoChangeCallbackTest>());

    audioCapturerStateChangeCallbackImpl->RemoveCapturerInfoChangeCallback(callback);
    EXPECT_EQ(audioCapturerStateChangeCallbackImpl->capturerInfoChangeCallbacklist_.empty(), true);
}

/**
 * @tc.name  : Test AudioCapturerPrivate API
 * @tc.type  : FUNC
 * @tc.number: AudioCapturerPrivate_024
 * @tc.desc  : Test AudioCapturerPrivate::UpdatePlaybackCaptureConfig
 */
HWTEST(AudioCapturerUnitTest, AudioCapturerPrivate_024, TestSize.Level1)
{
    AppInfo appInfo = {};
    auto capturer = std::make_shared<AudioCapturerPrivate>(AudioStreamType::STREAM_VOICE_CALL, appInfo, true);
    ASSERT_NE(capturer, nullptr);

    AudioPlaybackCaptureConfig config;
    capturer->capturerInfo_.sourceType = SOURCE_TYPE_MIC;

    auto ret = capturer->UpdatePlaybackCaptureConfig(config);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
}

/**
 * @tc.name  : Test AudioCapturerPrivate API
 * @tc.type  : FUNC
 * @tc.number: AudioCapturerPrivate_025
 * @tc.desc  : Test AudioCapturerPrivate::DecideStreamClassAndUpdateCapturerInfo
 */
HWTEST(AudioCapturerUnitTest, AudioCapturerPrivate_025, TestSize.Level1)
{
    AppInfo appInfo = {};
    auto capturer = std::make_shared<AudioCapturerPrivate>(AudioStreamType::STREAM_VOICE_CALL, appInfo, true);
    ASSERT_NE(capturer, nullptr);

    uint32_t flag = 0x6000;
    IAudioStream::StreamClass streamClass = capturer->DecideStreamClassAndUpdateCapturerInfo(flag);
    EXPECT_EQ(streamClass, IAudioStream::StreamClass::VOIP_STREAM);
}

/**
 * @tc.name  : Test AudioCapturerPrivate API
 * @tc.type  : FUNC
 * @tc.number: AudioCapturerPrivate_026
 * @tc.desc  : Test AudioCapturerPrivate::SetInputDevice
 */
HWTEST(AudioCapturerUnitTest, AudioCapturerPrivate_026, TestSize.Level1)
{
    AppInfo appInfo = {};
    auto capturer = std::make_shared<AudioCapturerPrivate>(AudioStreamType::STREAM_VOICE_CALL, appInfo, true);
    ASSERT_NE(capturer, nullptr);

    DeviceType deviceType = DEVICE_TYPE_INVALID;
    capturer->audioStream_ = NULL;

    auto ret = capturer->SetInputDevice(deviceType);
    capturer->audioStream_ = std::make_shared<FastAudioStreamFork2>();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCapturerInterruptCallbackImpl API
 * @tc.type  : FUNC
 * @tc.number: AudioCapturerPrivate_027
 * @tc.desc  : Test AudioCapturerInterruptCallbackImpl::OnInterrupt
 */
HWTEST(AudioCapturerUnitTest, AudioCapturerPrivate_027, TestSize.Level1)
{
    std::shared_ptr<IAudioStream> audioStream = nullptr;
    auto capturer = std::make_shared<AudioCapturerInterruptCallbackImpl>(audioStream);
    ASSERT_NE(capturer, nullptr);

    InterruptEventInternal interruptEvent;
    capturer->switching_ = true;

    capturer->OnInterrupt(interruptEvent);
    EXPECT_EQ(capturer->switching_, false);
}

/**
 * @tc.name  : Test GetFastStatus API
 * @tc.type  : FUNC
 * @tc.number: AudioCapturerPrivate_028
 * @tc.desc  : Test AudioCapturerPrivate::GetFastStatus
 */
HWTEST(AudioCapturerUnitTest, AudioCapturerPrivate_028, TestSize.Level1)
{
    AppInfo appInfo = {};
    shared_ptr<AudioCapturerPrivate> audioCapturer =
        std::make_shared<AudioCapturerPrivate>(STREAM_MUSIC, appInfo, true);
    ASSERT_NE(audioCapturer, nullptr);

    auto ret = audioCapturer->GetFastStatus();
    EXPECT_EQ(ret, FASTSTATUS_NORMAL);

    AudioPlaybackCaptureConfig playbackCaptureConfig;
    audioCapturer->capturerInfo_.sourceType = SOURCE_TYPE_MIC;
    audioCapturer->capturerInfo_.capturerFlags = 0;
    audioCapturer->capturerInfo_.originalFlag = 0;
    audioCapturer->filterConfig_ = playbackCaptureConfig;
    AudioCapturerParams capturerParams;
    capturerParams.audioSampleFormat = SAMPLE_S16LE;
    capturerParams.samplingRate = SAMPLE_RATE_44100;
    capturerParams.audioChannel = STEREO;
    capturerParams.audioEncoding = ENCODING_PCM;

    audioCapturer->SetParams(capturerParams);
    ret = audioCapturer->GetFastStatus();
    EXPECT_EQ(ret, FASTSTATUS_NORMAL);
}

/**
 * @tc.name  : Test SetFastStatusChangeCallback API
 * @tc.type  : FUNC
 * @tc.number: AudioCapturerPrivate_029
 * @tc.desc  : Test AudioCapturerPrivate::SetFastStatusChangeCallback
 */
HWTEST(AudioCapturerUnitTest, AudioCapturerPrivate_029, TestSize.Level1)
{
    AppInfo appInfo = {};
    auto capturer = std::make_shared<AudioCapturerPrivate>(AudioStreamType::STREAM_VOICE_CALL, appInfo, true);
    ASSERT_NE(capturer, nullptr);

    std::shared_ptr<CapturerFastStatusChangeCallbackTest> fastStatusChangeCallback =
        std::make_shared<CapturerFastStatusChangeCallbackTest>();

    capturer->SetFastStatusChangeCallback(fastStatusChangeCallback);
    EXPECT_NE(capturer->fastStatusChangeCallback_, nullptr);
}

/**
 * @tc.name  : Test ParamsToStateCmdType API
 * @tc.type  : FUNC
 * @tc.number: ParamsToStateCmdType_001
 * @tc.desc  : Test ParamsToStateCmdType API
 */
HWTEST(AudioCapturerUnitTest, ParamsToStateCmdType_001, TestSize.Level1)
{
    FastAudioStream fastAudioStream(AudioStreamType::STREAM_MUSIC,
        AudioMode::AUDIO_MODE_RECORD, 0);
    State state;
    StateChangeCmdType cmdType;
    int32_t ret = fastAudioStream.ParamsToStateCmdType(FastAudioStreamFork2::HANDLER_PARAM_NEW, state, cmdType);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(state, NEW);
    EXPECT_EQ(cmdType, CMD_FROM_CLIENT);
}

/**
 * @tc.name  : Test ParamsToStateCmdType API
 * @tc.type  : FUNC
 * @tc.number: ParamsToStateCmdType_002
 * @tc.desc  : Test ParamsToStateCmdType
 */
HWTEST(AudioCapturerUnitTest, ParamsToStateCmdType_002, TestSize.Level1)
{
    FastAudioStream fastAudioStream(AudioStreamType::STREAM_MUSIC,
        AudioMode::AUDIO_MODE_RECORD, 0);
    State state;
    StateChangeCmdType cmdType;
    int32_t ret = fastAudioStream.ParamsToStateCmdType(FastAudioStreamFork2::HANDLER_PARAM_RELEASED, state, cmdType);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(state, RELEASED);
    EXPECT_EQ(cmdType, CMD_FROM_CLIENT);
}

/**
 * @tc.name  : Test ParamsToStateCmdType API
 * @tc.type  : FUNC
 * @tc.number: ParamsToStateCmdType_003
 * @tc.desc  : Test ParamsToStateCmdType
 */
HWTEST(AudioCapturerUnitTest, ParamsToStateCmdType_003, TestSize.Level1)
{
    FastAudioStream fastAudioStream(AudioStreamType::STREAM_MUSIC,
        AudioMode::AUDIO_MODE_RECORD, 0);
    State state;
    StateChangeCmdType cmdType;
    int32_t ret = fastAudioStream.ParamsToStateCmdType(
        FastAudioStreamFork2::HANDLER_PARAM_PAUSED, state, cmdType);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(state, PAUSED);
    EXPECT_EQ(cmdType, CMD_FROM_CLIENT);
}

/**
 * @tc.name  : Test ParamsToStateCmdType API
 * @tc.type  : FUNC
 * @tc.number: ParamsToStateCmdType_004
 * @tc.desc  : Test ParamsToStateCmdType
 */
HWTEST(AudioCapturerUnitTest, ParamsToStateCmdType_004, TestSize.Level1)
{
    FastAudioStream fastAudioStream(AudioStreamType::STREAM_MUSIC,
        AudioMode::AUDIO_MODE_RECORD, 0);
    State state;
    StateChangeCmdType cmdType;
    int32_t ret = fastAudioStream.ParamsToStateCmdType(
        FastAudioStreamFork2::HANDLER_PARAM_RUNNING_FROM_SYSTEM, state, cmdType);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(state, RUNNING);
    EXPECT_EQ(cmdType, CMD_FROM_SYSTEM);
}

/**
 * @tc.name  : Test ParamsToStateCmdType API
 * @tc.type  : FUNC
 * @tc.number: ParamsToStateCmdType_005
 * @tc.desc  : Test ParamsToStateCmdType
 */
HWTEST(AudioCapturerUnitTest, ParamsToStateCmdType_005, TestSize.Level1)
{
    FastAudioStream fastAudioStream(AudioStreamType::STREAM_MUSIC,
        AudioMode::AUDIO_MODE_RECORD, 0);
    State state;
    StateChangeCmdType cmdType;
    int32_t ret = fastAudioStream.ParamsToStateCmdType(
        FastAudioStreamFork2::HANDLER_PARAM_PAUSED_FROM_SYSTEM, state, cmdType);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(state, PAUSED);
    EXPECT_EQ(cmdType, CMD_FROM_SYSTEM);
}

/**
 * @tc.name  : Test SetStreamCallback API
 * @tc.type  : FUNC
 * @tc.number: SetStreamCallback_001
 * @tc.desc  : Test SetStreamCallback_001
 */
HWTEST(AudioCapturerUnitTest, SetStreamCallback_001, TestSize.Level1)
{
    FastAudioStream fastAudioStream(AudioStreamType::STREAM_MUSIC,
        AudioMode::AUDIO_MODE_RECORD, 0);
    std::shared_ptr<AudioStreamCallback> callback = nullptr;
    int32_t ret = fastAudioStream.SetStreamCallback(callback);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}
} // namespace AudioStandard
} // namespace OHOS
