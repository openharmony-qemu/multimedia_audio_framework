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
#include <cstring>
#include <functional>

#include "audio_capturer.h"
#include "audio_capturer_private.h"
#include "audio_errors.h"
#include "audio_info.h"
#include "audio_system_manager.h"
#include "audio_client_tracker_callback_service.h"
#include "capturer_in_client.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
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

class CapturerFastStatusChangeCallbackTest : public AudioCapturerFastStatusChangeCallback {
public:
    void OnFastStatusChange(FastStatus status) override
    {
        notifyCount_++;
        status_ = status;
    }

    int32_t notifyCount_ = 0;
    FastStatus status_ = FASTSTATUS_INVALID;
};

class AudioCapturerMetaDataCallbackTestStub : public AudioCapturerMetaDataCallback {
public:
    void OnMetaData(const CaptureMetaDataType type, const std::vector<uint8_t> &data) override
    {
        (void)type;
        (void)data;
    }
};

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
 * @tc.name  : Test AudioCapturerPrivate API
 * @tc.type  : FUNC
 * @tc.number: AudioCapturerPrivate_033
 * @tc.desc  : Test AudioCapturerPrivate::ConvertToStreamDescriptor for voice recognition micIn/ec request.
 */
HWTEST(AudioCapturerUnitTest, AudioCapturerPrivate_033, TestSize.Level1)
{
    AppInfo appInfo = {};
    auto capturer = std::make_shared<AudioCapturerPrivate>(AudioStreamType::STREAM_MUSIC, appInfo, false);
    ASSERT_NE(capturer, nullptr);
    capturer->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;

    AudioStreamParams streamParams = {};
    streamParams.format = SAMPLE_S16LE;
    streamParams.encoding = ENCODING_PCM;
    streamParams.samplingRate = SAMPLE_RATE_48000;
    streamParams.channels = MONO;
    streamParams.channelLayout = CH_LAYOUT_MONO;
    streamParams.micInFormat = SAMPLE_S16LE;
    streamParams.micInEncoding = ENCODING_PCM;
    streamParams.micInSamplingRate = SAMPLE_RATE_48000;
    streamParams.micInChannels = CHANNEL_4;
    streamParams.micInChannelLayout = CH_LAYOUT_UNKNOWN;
    streamParams.ecFormat = SAMPLE_S16LE;
    streamParams.ecEncoding = ENCODING_PCM;
    streamParams.ecSamplingRate = SAMPLE_RATE_48000;
    streamParams.ecChannels = STEREO;
    streamParams.ecChannelLayout = CH_LAYOUT_STEREO;

    auto streamDesc = capturer->ConvertToStreamDescriptor(streamParams);
    ASSERT_NE(streamDesc, nullptr);
    EXPECT_EQ(streamDesc->streamInfo_.channels, MONO);
    if (streamDesc->micInStreamInfo_.channels == CHANNEL_UNKNOW) {
        EXPECT_EQ(streamDesc->ecStreamInfo_.channels, CHANNEL_UNKNOW);
        EXPECT_EQ(streamDesc->micInStreamInfo_.format, INVALID_WIDTH);
        EXPECT_EQ(streamDesc->ecStreamInfo_.format, INVALID_WIDTH);
    } else {
        EXPECT_EQ(streamDesc->micInStreamInfo_.channels, CHANNEL_4);
        EXPECT_EQ(streamDesc->ecStreamInfo_.channels, STEREO);
    }
}

/**
 * @tc.name  : Test AudioCapturerPrivate API
 * @tc.type  : FUNC
 * @tc.number: AudioCapturerPrivate_034
 * @tc.desc  : Test AudioCapturerPrivate::GenerateStreamDesc for voice recognition micIn/ec request.
 */
HWTEST(AudioCapturerUnitTest, AudioCapturerPrivate_034, TestSize.Level1)
{
    AppInfo appInfo = {};
    auto capturer = std::make_shared<AudioCapturerPrivate>(AudioStreamType::STREAM_MUSIC, appInfo, false);
    ASSERT_NE(capturer, nullptr);

    IAudioStream::SwitchInfo switchInfo = {};
    switchInfo.capturerInfo.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    switchInfo.sessionId = 1001;
    switchInfo.params.format = SAMPLE_S16LE;
    switchInfo.params.encoding = ENCODING_PCM;
    switchInfo.params.samplingRate = SAMPLE_RATE_48000;
    switchInfo.params.channels = MONO;
    switchInfo.params.channelLayout = CH_LAYOUT_MONO;
    switchInfo.params.micInFormat = SAMPLE_S16LE;
    switchInfo.params.micInEncoding = ENCODING_PCM;
    switchInfo.params.micInSamplingRate = SAMPLE_RATE_48000;
    switchInfo.params.micInChannels = CHANNEL_4;
    switchInfo.params.micInChannelLayout = CH_LAYOUT_UNKNOWN;
    switchInfo.params.ecFormat = SAMPLE_S16LE;
    switchInfo.params.ecEncoding = ENCODING_PCM;
    switchInfo.params.ecSamplingRate = SAMPLE_RATE_48000;
    switchInfo.params.ecChannels = STEREO;
    switchInfo.params.ecChannelLayout = CH_LAYOUT_STEREO;

    RestoreInfo restoreInfo = {};
    auto streamDesc = capturer->GenerateStreamDesc(switchInfo, restoreInfo);
    ASSERT_NE(streamDesc, nullptr);
    EXPECT_EQ(streamDesc->sessionId_, switchInfo.sessionId);
    EXPECT_EQ(streamDesc->streamInfo_.channels, MONO);
    if (streamDesc->micInStreamInfo_.channels == CHANNEL_UNKNOW) {
        EXPECT_EQ(streamDesc->ecStreamInfo_.channels, CHANNEL_UNKNOW);
    } else {
        EXPECT_EQ(streamDesc->micInStreamInfo_.channels, CHANNEL_4);
        EXPECT_EQ(streamDesc->ecStreamInfo_.channels, STEREO);
    }
}

/**
 * @tc.name  : Test NoNeedNotifyEvent
 * @tc.type  : FUNC
 * @tc.number: NoNeedNotifyEvent_001
 * @tc.desc  : Test NoNeedNotifyEvent
 */
HWTEST(AudioCapturerUnitTest, NoNeedNotifyEvent_001, TestSize.Level1)
{
    AppInfo appInfo = {};
    auto capturer = std::make_shared<AudioCapturerPrivate>(STREAM_MUSIC, appInfo, false);
 
    AudioStreamParams tempParams = {};
 
    auto audioStream = IAudioStream::GetRecordStream(IAudioStream::FAST_STREAM, tempParams,
        STREAM_MUSIC, getpid());
    
    audioStream->SetFocusState(FOCUS_RUNNING);
    auto interruptCallback = std::make_unique<AudioCapturerInterruptCallbackImpl>(nullptr);
    interruptCallback->UpdateAudioStream(audioStream);
    EXPECT_EQ(interruptCallback->audioStream_, audioStream);
 
    capturer->audioStream_ = audioStream;
    InterruptEventInternal interruptEventInternal {InterruptType::INTERRUPT_TYPE_BEGIN,
        InterruptForceType::INTERRUPT_FORCE, InterruptHint::INTERRUPT_HINT_PAUSE, 0.0};
    InterruptEvent event = InterruptEvent {interruptEventInternal.eventType, interruptEventInternal.forceType,
        interruptEventInternal.hintType};
 
    shared_ptr<AudioCapturerCallbackTestStub> cb = make_shared<AudioCapturerCallbackTestStub>();
    interruptCallback->SaveCallback(cb);
    EXPECT_EQ(true, interruptCallback->NoNeedNotifyEvent(event));
    event.hintType = INTERRUPT_HINT_STOP;
    EXPECT_EQ(true, interruptCallback->NoNeedNotifyEvent(event));
    event.hintType = INTERRUPT_HINT_RESUME;
    EXPECT_EQ(true, interruptCallback->NoNeedNotifyEvent(event));
    event.hintType = INTERRUPT_HINT_DUCK;
    EXPECT_NE(true, interruptCallback->NoNeedNotifyEvent(event));
}

/** @tc.desc: Runtime fallbacks preserve the ultra fast request for device migration and service recovery. */
HWTEST(AudioCapturerUnitTest, UltraFastRequestPreserved_001, TestSize.Level1)
{
    AudioCapturerPrivate capturer(STREAM_MUSIC, AppInfo{}, false);
    capturer.capturerInfo_.originalFlag = AUDIO_FLAG_ULTRA_FAST;
    EXPECT_EQ(capturer.DecideStreamClassAndUpdateCapturerInfo(AUDIO_INPUT_FLAG_ULTRA_FAST), IAudioStream::FAST_STREAM);
    EXPECT_EQ(capturer.capturerInfo_.capturerFlags, AUDIO_FLAG_ULTRA_FAST);
    EXPECT_EQ(capturer.capturerInfo_.pipeType, PIPE_TYPE_IN_ULTRA_FAST);
    EXPECT_EQ(capturer.DecideStreamClassAndUpdateCapturerInfo(AUDIO_INPUT_FLAG_FAST), IAudioStream::FAST_STREAM);
    EXPECT_EQ(capturer.capturerInfo_.originalFlag, AUDIO_FLAG_ULTRA_FAST);
    EXPECT_EQ(capturer.capturerInfo_.capturerFlags, AUDIO_FLAG_MMAP);
    EXPECT_EQ(capturer.DecideStreamClassAndUpdateCapturerInfo(AUDIO_INPUT_FLAG_NORMAL), IAudioStream::PA_STREAM);
    EXPECT_EQ(capturer.capturerInfo_.originalFlag, AUDIO_FLAG_ULTRA_FAST);
}

} // namespace AudioStandard
} // namespace OHOS
