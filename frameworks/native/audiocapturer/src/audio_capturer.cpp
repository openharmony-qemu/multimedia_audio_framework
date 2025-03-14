/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#ifndef LOG_TAG
#define LOG_TAG "AudioCapturer"
#endif

#include "audio_capturer.h"

#include <cinttypes>

#include "audio_capturer_private.h"
#include "audio_errors.h"
#include "audio_utils.h"
#include "audio_capturer_log.h"
#include "audio_policy_manager.h"

#include "media_monitor_manager.h"

namespace OHOS {
namespace AudioStandard {
static constexpr uid_t UID_MSDP_SA = 6699;
static constexpr int32_t WRITE_OVERFLOW_NUM = 100;
static constexpr int32_t AUDIO_SOURCE_TYPE_INVALID_5 = 5;

std::map<AudioStreamType, SourceType> AudioCapturerPrivate::streamToSource_ = {
    {AudioStreamType::STREAM_MUSIC, SourceType::SOURCE_TYPE_MIC},
    {AudioStreamType::STREAM_MEDIA, SourceType::SOURCE_TYPE_MIC},
    {AudioStreamType::STREAM_MUSIC, SourceType::SOURCE_TYPE_UNPROCESSED},
    {AudioStreamType::STREAM_CAMCORDER, SourceType::SOURCE_TYPE_CAMCORDER},
    {AudioStreamType::STREAM_VOICE_CALL, SourceType::SOURCE_TYPE_VOICE_COMMUNICATION},
    {AudioStreamType::STREAM_ULTRASONIC, SourceType::SOURCE_TYPE_ULTRASONIC},
    {AudioStreamType::STREAM_WAKEUP, SourceType::SOURCE_TYPE_WAKEUP},
    {AudioStreamType::STREAM_SOURCE_VOICE_CALL, SourceType::SOURCE_TYPE_VOICE_CALL},
};

AudioCapturer::~AudioCapturer() = default;

AudioCapturerPrivate::~AudioCapturerPrivate()
{
    AUDIO_INFO_LOG("~AudioCapturerPrivate");
    std::shared_ptr<InputDeviceChangeWithInfoCallbackImpl> inputDeviceChangeCallback = inputDeviceChangeCallback_;
    if (inputDeviceChangeCallback != nullptr) {
        inputDeviceChangeCallback->UnsetAudioCapturerObj();
    }
    AudioPolicyManager::GetInstance().UnregisterDeviceChangeWithInfoCallback(sessionID_);
    if (audioStream_ != nullptr) {
        audioStream_->GetAudioSessionID(sessionID_);
        audioStream_->ReleaseAudioStream(true);
        audioStream_ = nullptr;
        AudioPolicyManager::GetInstance().RemoveClientTrackerStub(sessionID_);
    }
    if (audioStateChangeCallback_ != nullptr) {
        audioStateChangeCallback_->HandleCapturerDestructor();
    }
    DumpFileUtil::CloseDumpFile(&dumpFile_);
}

std::unique_ptr<AudioCapturer> AudioCapturer::Create(AudioStreamType audioStreamType)
{
    AppInfo appInfo = {};
    return Create(audioStreamType, appInfo);
}

std::unique_ptr<AudioCapturer> AudioCapturer::Create(AudioStreamType audioStreamType, const AppInfo &appInfo)
{
    return std::make_unique<AudioCapturerPrivate>(audioStreamType, appInfo, true);
}

std::unique_ptr<AudioCapturer> AudioCapturer::Create(const AudioCapturerOptions &options)
{
    AppInfo appInfo = {};
    return Create(options, "", appInfo);
}

std::unique_ptr<AudioCapturer> AudioCapturer::Create(const AudioCapturerOptions &options, const AppInfo &appInfo)
{
    return Create(options, "", appInfo);
}

std::unique_ptr<AudioCapturer> AudioCapturer::Create(const AudioCapturerOptions &options, const std::string cachePath)
{
    AppInfo appInfo = {};
    return Create(options, cachePath, appInfo);
}

std::unique_ptr<AudioCapturer> AudioCapturer::Create(const AudioCapturerOptions &capturerOptions,
    const std::string cachePath, const AppInfo &appInfo)
{
    Trace trace("AudioCapturer::Create");
    auto sourceType = capturerOptions.capturerInfo.sourceType;
    if (sourceType < SOURCE_TYPE_MIC || sourceType > SOURCE_TYPE_MAX || sourceType == AUDIO_SOURCE_TYPE_INVALID_5) {
        AudioCapturer::SendCapturerCreateError(sourceType, ERR_INVALID_PARAM);
        AUDIO_ERR_LOG("Invalid source type %{public}d!", sourceType);
        return nullptr;
    }
    if (sourceType == SOURCE_TYPE_ULTRASONIC && getuid() != UID_MSDP_SA) {
        AudioCapturer::SendCapturerCreateError(sourceType, ERR_INVALID_PARAM);
    }
    CHECK_AND_RETURN_RET_LOG(sourceType != SOURCE_TYPE_ULTRASONIC || getuid() == UID_MSDP_SA, nullptr,
        "Create failed: SOURCE_TYPE_ULTRASONIC can only be used by MSDP");
    AudioStreamType audioStreamType = FindStreamTypeBySourceType(sourceType);
    AudioCapturerParams params;
    params.audioSampleFormat = capturerOptions.streamInfo.format;
    params.samplingRate = capturerOptions.streamInfo.samplingRate;
    bool isChange = false;
    if (AudioChannel::CHANNEL_3 == capturerOptions.streamInfo.channels) {
        params.audioChannel = AudioChannel::STEREO;
        isChange = true;
    } else {
        params.audioChannel = capturerOptions.streamInfo.channels;
    }
    params.audioEncoding = capturerOptions.streamInfo.encoding;
    params.channelLayout = capturerOptions.streamInfo.channelLayout;
    auto capturer = std::make_unique<AudioCapturerPrivate>(audioStreamType, appInfo, false);
    if (capturer == nullptr) {
        AudioCapturer::SendCapturerCreateError(sourceType, ERR_OPERATION_FAILED);
        AUDIO_ERR_LOG("Failed to create capturer object");
        return capturer;
    }
    if (!cachePath.empty()) {
        capturer->cachePath_ = cachePath;
    }
    AUDIO_INFO_LOG("Capturer::Create sourceType: %{public}d, uid: %{public}d", sourceType, appInfo.appUid);
    // InitPlaybackCapturer will be replaced by UpdatePlaybackCaptureConfig.
    capturer->capturerInfo_.sourceType = sourceType;
    capturer->capturerInfo_.capturerFlags = capturerOptions.capturerInfo.capturerFlags;
    capturer->capturerInfo_.originalFlag = capturerOptions.capturerInfo.capturerFlags;
    capturer->capturerInfo_.samplingRate = capturerOptions.streamInfo.samplingRate;
    capturer->filterConfig_ = capturerOptions.playbackCaptureConfig;
    capturer->strategy_ = capturerOptions.strategy;
    if (capturer->SetParams(params) != SUCCESS) {
        AudioCapturer::SendCapturerCreateError(sourceType, ERR_OPERATION_FAILED);
        capturer = nullptr;
    }
    if (capturer != nullptr && isChange) {
        capturer->isChannelChange_ = true;
    }
    return capturer;
}

// This will be called in Create and after Create.
int32_t AudioCapturerPrivate::UpdatePlaybackCaptureConfig(const AudioPlaybackCaptureConfig &config)
{
    // UpdatePlaybackCaptureConfig will only work for InnerCap streams.
    if (capturerInfo_.sourceType != SOURCE_TYPE_PLAYBACK_CAPTURE) {
        AUDIO_WARNING_LOG("This is not a PLAYBACK_CAPTURE stream.");
        return ERR_INVALID_OPERATION;
    }

    if (config.filterOptions.usages.size() == 0 && config.filterOptions.pids.size() == 0) {
        AUDIO_WARNING_LOG("Both usages and pids are empty!");
    }

    CHECK_AND_RETURN_RET_LOG(audioStream_ != nullptr, ERR_OPERATION_FAILED, "Failed with null audioStream_");

    return audioStream_->UpdatePlaybackCaptureConfig(config);
}

void AudioCapturer::SendCapturerCreateError(const SourceType &sourceType,
    const int32_t &errorCode)
{
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::ModuleId::AUDIO, Media::MediaMonitor::EventId::AUDIO_STREAM_CREATE_ERROR_STATS,
        Media::MediaMonitor::EventType::FREQUENCY_AGGREGATION_EVENT);
    bean->Add("IS_PLAYBACK", 0);
    bean->Add("CLIENT_UID", static_cast<int32_t>(getuid()));
    bean->Add("STREAM_TYPE", sourceType);
    bean->Add("ERROR_CODE", errorCode);
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

AudioCapturerPrivate::AudioCapturerPrivate(AudioStreamType audioStreamType, const AppInfo &appInfo, bool createStream)
{
    if (audioStreamType < STREAM_VOICE_CALL || audioStreamType > STREAM_ALL) {
        AUDIO_WARNING_LOG("audioStreamType is invalid!");
    }
    audioStreamType_ = audioStreamType;
    auto iter = streamToSource_.find(audioStreamType);
    if (iter != streamToSource_.end()) {
        capturerInfo_.sourceType = iter->second;
    }
    appInfo_ = appInfo;
    if (!(appInfo_.appPid)) {
        appInfo_.appPid = getpid();
    }

    if (appInfo_.appUid < 0) {
        appInfo_.appUid = static_cast<int32_t>(getuid());
    }
    if (createStream) {
        AudioStreamParams tempParams = {};
        audioStream_ = IAudioStream::GetRecordStream(IAudioStream::PA_STREAM, tempParams, audioStreamType_,
            appInfo_.appUid);
        AUDIO_INFO_LOG("create normal stream for old mode.");
    }

    capturerProxyObj_ = std::make_shared<AudioCapturerProxyObj>();
    if (!capturerProxyObj_) {
        AUDIO_WARNING_LOG("AudioCapturerProxyObj Memory Allocation Failed !!");
    }
}

int32_t AudioCapturerPrivate::GetFrameCount(uint32_t &frameCount) const
{
    return audioStream_->GetFrameCount(frameCount);
}

IAudioStream::StreamClass AudioCapturerPrivate::GetPreferredStreamClass(AudioStreamParams audioStreamParams)
{
    int32_t flag = AudioPolicyManager::GetInstance().GetPreferredInputStreamType(capturerInfo_);
    AUDIO_INFO_LOG("Preferred capturer flag: %{public}d", flag);
    if (flag == AUDIO_FLAG_MMAP && IAudioStream::IsStreamSupported(capturerInfo_.originalFlag, audioStreamParams)) {
        capturerInfo_.capturerFlags = AUDIO_FLAG_MMAP;
        return IAudioStream::FAST_STREAM;
    }
    if (flag == AUDIO_FLAG_VOIP_FAST) {
        // It is not possible to directly create a fast VoIP stream
        isFastVoipSupported_ = true;
    }

    capturerInfo_.capturerFlags = AUDIO_FLAG_NORMAL;
    return IAudioStream::PA_STREAM;
}

int32_t AudioCapturerPrivate::SetParams(const AudioCapturerParams params)
{
    Trace trace("AudioCapturer::SetParams");
    AUDIO_INFO_LOG("StreamClientState for Capturer::SetParams.");

    std::shared_lock<std::shared_mutex> lockShared(switchStreamMutex_);
    std::lock_guard<std::mutex> lock(setParamsMutex_);

    AudioStreamParams audioStreamParams = ConvertToAudioStreamParams(params);

    IAudioStream::StreamClass streamClass = IAudioStream::PA_STREAM;
    if (capturerInfo_.sourceType != SOURCE_TYPE_PLAYBACK_CAPTURE) {
        streamClass = GetPreferredStreamClass(audioStreamParams);
    }
    ActivateAudioConcurrency(streamClass);

    // check AudioStreamParams for fast stream
    if (audioStream_ == nullptr) {
        audioStream_ = IAudioStream::GetRecordStream(streamClass, audioStreamParams, audioStreamType_,
            appInfo_.appUid);
        CHECK_AND_RETURN_RET_LOG(audioStream_ != nullptr, ERR_INVALID_PARAM, "SetParams GetRecordStream faied.");
        AUDIO_INFO_LOG("IAudioStream::GetStream success");
        audioStream_->SetApplicationCachePath(cachePath_);
    }
    int32_t ret = InitAudioStream(audioStreamParams);
    // When the fast stream creation fails, a normal stream is created
    if (ret != SUCCESS && streamClass == IAudioStream::FAST_STREAM) {
        AUDIO_INFO_LOG("Create fast Stream fail, record by normal stream");
        streamClass = IAudioStream::PA_STREAM;
        audioStream_ = IAudioStream::GetRecordStream(streamClass, audioStreamParams, audioStreamType_, appInfo_.appUid);
        CHECK_AND_RETURN_RET_LOG(audioStream_ != nullptr, ERR_INVALID_PARAM, "Get normal record stream failed");
        ret = InitAudioStream(audioStreamParams);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Init normal audio stream failed");
        audioStream_->SetCaptureMode(CAPTURE_MODE_CALLBACK);
    }
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "InitAudioStream failed");

    RegisterCapturerPolicyServiceDiedCallback();

    if (audioStream_->GetAudioSessionID(sessionID_) != 0) {
        AUDIO_ERR_LOG("GetAudioSessionID failed!");
        return ERR_INVALID_INDEX;
    }
    // eg: 100009_44100_2_1_cap_client_out.pcm
    std::string dumpFileName = std::to_string(sessionID_) + "_" + std::to_string(params.samplingRate) + "_" +
        std::to_string(params.audioChannel) + "_" + std::to_string(params.audioSampleFormat) + "_cap_client_out.pcm";
    DumpFileUtil::OpenDumpFile(DUMP_CLIENT_PARA, dumpFileName, &dumpFile_);

    ret = InitInputDeviceChangeCallback();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Init input device change callback failed");

    return InitAudioInterruptCallback();
}

int32_t AudioCapturerPrivate::InitInputDeviceChangeCallback()
{
    CHECK_AND_RETURN_RET_LOG(GetCurrentInputDevices(currentDeviceInfo_) == SUCCESS, ERROR,
        "Get current device info failed");

    if (!inputDeviceChangeCallback_) {
        inputDeviceChangeCallback_ = std::make_shared<InputDeviceChangeWithInfoCallbackImpl>();
        CHECK_AND_RETURN_RET_LOG(inputDeviceChangeCallback_ != nullptr, ERROR, "Memory allocation failed");
    }

    inputDeviceChangeCallback_->SetAudioCapturerObj(this);

    uint32_t sessionId;
    int32_t ret = GetAudioStreamId(sessionId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Get sessionId failed");

    ret = AudioPolicyManager::GetInstance().RegisterDeviceChangeWithInfoCallback(sessionId,
        inputDeviceChangeCallback_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Register failed");

    return SUCCESS;
}

int32_t AudioCapturerPrivate::InitAudioStream(const AudioStreamParams &audioStreamParams)
{
    Trace trace("AudioCapturer::InitAudioStream");
    const AudioCapturer *capturer = this;
    capturerProxyObj_->SaveCapturerObj(capturer);

    audioStream_->SetCapturerInfo(capturerInfo_);

    audioStream_->SetClientID(appInfo_.appPid, appInfo_.appUid, appInfo_.appTokenId, appInfo_.appFullTokenId);

    if (capturerInfo_.sourceType == SOURCE_TYPE_PLAYBACK_CAPTURE) {
        audioStream_->SetInnerCapturerState(true);
    } else if (capturerInfo_.sourceType == SourceType::SOURCE_TYPE_WAKEUP) {
        audioStream_->SetWakeupCapturerState(true);
    }

    audioStream_->SetCapturerSource(capturerInfo_.sourceType);

    int32_t ret = audioStream_->SetAudioStreamInfo(audioStreamParams, capturerProxyObj_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetAudioStreamInfo failed");
    // for inner-capturer
    if (capturerInfo_.sourceType == SOURCE_TYPE_PLAYBACK_CAPTURE) {
        ret = UpdatePlaybackCaptureConfig(filterConfig_);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "UpdatePlaybackCaptureConfig Failed");
    }
    InitLatencyMeasurement(audioStreamParams);
    InitAudioConcurrencyCallback();
    return ret;
}

void AudioCapturerPrivate::CheckSignalData(uint8_t *buffer, size_t bufferSize) const
{
    std::lock_guard lock(signalDetectAgentMutex_);
    if (!latencyMeasEnabled_) {
        return;
    }
    CHECK_AND_RETURN_LOG(signalDetectAgent_ != nullptr, "LatencyMeas signalDetectAgent_ is nullptr");
    bool detected = signalDetectAgent_->CheckAudioData(buffer, bufferSize);
    if (detected) {
        if (capturerInfo_.capturerFlags == IAudioStream::FAST_STREAM) {
            AUDIO_INFO_LOG("LatencyMeas fast capturer signal detected");
        } else {
            AUDIO_INFO_LOG("LatencyMeas normal capturer signal detected");
        }
        audioStream_->UpdateLatencyTimestamp(signalDetectAgent_->lastPeakBufferTime_, false);
    }
}

void AudioCapturerPrivate::InitLatencyMeasurement(const AudioStreamParams &audioStreamParams)
{
    std::lock_guard lock(signalDetectAgentMutex_);
    latencyMeasEnabled_ = AudioLatencyMeasurement::CheckIfEnabled();
    AUDIO_INFO_LOG("LatencyMeas enabled in capturer:%{public}d", latencyMeasEnabled_);
    if (!latencyMeasEnabled_) {
        return;
    }
    signalDetectAgent_ = std::make_shared<SignalDetectAgent>();
    CHECK_AND_RETURN_LOG(signalDetectAgent_ != nullptr, "LatencyMeas signalDetectAgent_ is nullptr");
    signalDetectAgent_->sampleFormat_ = audioStreamParams.format;
    signalDetectAgent_->formatByteSize_ = GetFormatByteSize(audioStreamParams.format);
}

int32_t AudioCapturerPrivate::InitAudioInterruptCallback()
{
    if (audioInterrupt_.sessionId != 0) {
        AUDIO_INFO_LOG("old session already has interrupt, need to reset");
        (void)AudioPolicyManager::GetInstance().DeactivateAudioInterrupt(audioInterrupt_);
        (void)AudioPolicyManager::GetInstance().UnsetAudioInterruptCallback(audioInterrupt_.sessionId);
    }

    if (audioStream_->GetAudioSessionID(sessionID_) != 0) {
        AUDIO_ERR_LOG("GetAudioSessionID failed for INDEPENDENT_MODE");
        return ERR_INVALID_INDEX;
    }
    audioInterrupt_.sessionId = sessionID_;
    audioInterrupt_.pid = appInfo_.appPid;
    audioInterrupt_.audioFocusType.sourceType = capturerInfo_.sourceType;
    audioInterrupt_.sessionStrategy = strategy_;
    if (audioInterrupt_.audioFocusType.sourceType == SOURCE_TYPE_VIRTUAL_CAPTURE) {
        isVoiceCallCapturer_ = true;
        audioInterrupt_.audioFocusType.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    }
    if (audioInterruptCallback_ == nullptr) {
        audioInterruptCallback_ = std::make_shared<AudioCapturerInterruptCallbackImpl>(audioStream_);
        CHECK_AND_RETURN_RET_LOG(audioInterruptCallback_ != nullptr, ERROR,
            "Failed to allocate memory for audioInterruptCallback_");
    }
    return AudioPolicyManager::GetInstance().SetAudioInterruptCallback(sessionID_, audioInterruptCallback_,
        appInfo_.appUid);
}

int32_t AudioCapturerPrivate::SetCapturerCallback(const std::shared_ptr<AudioCapturerCallback> &callback)
{
    std::lock_guard<std::mutex> lock(setCapturerCbMutex_);
    // If the client is using the deprecated SetParams API. SetCapturerCallback must be invoked, after SetParams.
    // In general, callbacks can only be set after the capturer state is  PREPARED.
    CapturerState state = GetStatus();
    CHECK_AND_RETURN_RET_LOG(state != CAPTURER_NEW && state != CAPTURER_RELEASED, ERR_ILLEGAL_STATE,
        "SetCapturerCallback ncorrect state:%{public}d to register cb", state);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM,
        "SetCapturerCallback callback param is null");

    // Save reference for interrupt callback
    CHECK_AND_RETURN_RET_LOG(audioInterruptCallback_ != nullptr, ERROR,
        "SetCapturerCallback audioInterruptCallback_ == nullptr");
    std::shared_ptr<AudioCapturerInterruptCallbackImpl> cbInterrupt =
        std::static_pointer_cast<AudioCapturerInterruptCallbackImpl>(audioInterruptCallback_);
    cbInterrupt->SaveCallback(callback);

    // Save and Set reference for stream callback. Order is important here.
    if (audioStreamCallback_ == nullptr) {
        audioStreamCallback_ = std::make_shared<AudioStreamCallbackCapturer>();
        CHECK_AND_RETURN_RET_LOG(audioStreamCallback_ != nullptr, ERROR,
            "Failed to allocate memory for audioStreamCallback_");
    }
    std::shared_ptr<AudioStreamCallbackCapturer> cbStream =
        std::static_pointer_cast<AudioStreamCallbackCapturer>(audioStreamCallback_);
    cbStream->SaveCallback(callback);
    (void)audioStream_->SetStreamCallback(audioStreamCallback_);

    return SUCCESS;
}

int32_t AudioCapturerPrivate::GetParams(AudioCapturerParams &params) const
{
    AudioStreamParams audioStreamParams;
    int32_t result = audioStream_->GetAudioStreamInfo(audioStreamParams);
    if (SUCCESS == result) {
        params.audioSampleFormat = static_cast<AudioSampleFormat>(audioStreamParams.format);
        params.samplingRate = static_cast<AudioSamplingRate>(audioStreamParams.samplingRate);
        params.audioChannel = static_cast<AudioChannel>(audioStreamParams.channels);
        params.audioEncoding = static_cast<AudioEncodingType>(audioStreamParams.encoding);
    }

    return result;
}

int32_t AudioCapturerPrivate::GetCapturerInfo(AudioCapturerInfo &capturerInfo) const
{
    capturerInfo = capturerInfo_;

    return SUCCESS;
}

int32_t AudioCapturerPrivate::GetStreamInfo(AudioStreamInfo &streamInfo) const
{
    AudioStreamParams audioStreamParams;
    int32_t result = audioStream_->GetAudioStreamInfo(audioStreamParams);
    if (SUCCESS == result) {
        streamInfo.format = static_cast<AudioSampleFormat>(audioStreamParams.format);
        streamInfo.samplingRate = static_cast<AudioSamplingRate>(audioStreamParams.samplingRate);
        if (this->isChannelChange_) {
            streamInfo.channels = AudioChannel::CHANNEL_3;
        } else {
            streamInfo.channels = static_cast<AudioChannel>(audioStreamParams.channels);
        }
        streamInfo.encoding = static_cast<AudioEncodingType>(audioStreamParams.encoding);
    }

    return result;
}

int32_t AudioCapturerPrivate::SetCapturerPositionCallback(int64_t markPosition,
    const std::shared_ptr<CapturerPositionCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG((callback != nullptr) && (markPosition > 0), ERR_INVALID_PARAM,
        "input param is invalid");

    audioStream_->SetCapturerPositionCallback(markPosition, callback);

    return SUCCESS;
}

void AudioCapturerPrivate::UnsetCapturerPositionCallback()
{
    audioStream_->UnsetCapturerPositionCallback();
}

int32_t AudioCapturerPrivate::SetCapturerPeriodPositionCallback(int64_t frameNumber,
    const std::shared_ptr<CapturerPeriodPositionCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG((callback != nullptr) && (frameNumber > 0), ERR_INVALID_PARAM,
        "input param is invalid");

    audioStream_->SetCapturerPeriodPositionCallback(frameNumber, callback);

    return SUCCESS;
}

void AudioCapturerPrivate::UnsetCapturerPeriodPositionCallback()
{
    audioStream_->UnsetCapturerPeriodPositionCallback();
}

bool AudioCapturerPrivate::Start() const
{
    Trace trace("AudioCapturer::Start" + std::to_string(sessionID_));
    std::lock_guard lock(switchStreamMutex_);
    AUDIO_INFO_LOG("StreamClientState for Capturer::Start. id %{public}u, sourceType: %{public}d",
        sessionID_, audioInterrupt_.audioFocusType.sourceType);

    CapturerState state = GetStatus();
    CHECK_AND_RETURN_RET_LOG((state == CAPTURER_PREPARED) || (state == CAPTURER_STOPPED) || (state == CAPTURER_PAUSED),
        false, "Start failed. Illegal state %{public}u.", state);

    CHECK_AND_RETURN_RET_LOG(!isSwitching_, false, "Operation failed, in switching");

    CHECK_AND_RETURN_RET(audioInterrupt_.audioFocusType.sourceType != SOURCE_TYPE_INVALID &&
        audioInterrupt_.sessionId != INVALID_SESSION_ID, false);

    int32_t ret = AudioPolicyManager::GetInstance().ActivateAudioInterrupt(audioInterrupt_);
    CHECK_AND_RETURN_RET_LOG(ret == 0, false, "ActivateAudioInterrupt Failed");

    // When the cellular call stream is starting, only need to activate audio interrupt.
    CHECK_AND_RETURN_RET(!isVoiceCallCapturer_, true);
    CHECK_AND_RETURN_RET(audioStream_ != nullptr, false, "audioStream_ is null");
    bool result = audioStream_->StartAudioStream();
    if (!result) {
        AUDIO_ERR_LOG("Start audio stream failed");
        ret = AudioPolicyManager::GetInstance().DeactivateAudioInterrupt(audioInterrupt_);
        if (ret != 0) {
            AUDIO_WARNING_LOG("DeactivateAudioInterrupt Failed");
        }
    }

    return result;
}

int32_t AudioCapturerPrivate::Read(uint8_t &buffer, size_t userSize, bool isBlockingRead) const
{
    Trace trace("AudioCapturer::Read");
    CheckSignalData(&buffer, userSize);
    int size = audioStream_->Read(buffer, userSize, isBlockingRead);
    if (size > 0) {
        DumpFileUtil::WriteDumpFile(dumpFile_, static_cast<void *>(&buffer), size);
    }
    return size;
}

CapturerState AudioCapturerPrivate::GetStatus() const
{
    return (CapturerState)audioStream_->GetState();
}

bool AudioCapturerPrivate::GetAudioTime(Timestamp &timestamp, Timestamp::Timestampbase base) const
{
    return audioStream_->GetAudioTime(timestamp, base);
}

bool AudioCapturerPrivate::Pause() const
{
    Trace trace("AudioCapturer::Pause" + std::to_string(sessionID_));
    std::lock_guard lock(switchStreamMutex_);
    AUDIO_INFO_LOG("StreamClientState for Capturer::Pause. id %{public}u", sessionID_);
    CHECK_AND_RETURN_RET_LOG(!isSwitching_, false, "Operation failed, in switching");

    // When user is intentionally pausing , Deactivate to remove from audio focus info list
    int32_t ret = AudioPolicyManager::GetInstance().DeactivateAudioInterrupt(audioInterrupt_);
    if (ret != 0) {
        AUDIO_WARNING_LOG("AudioRenderer: DeactivateAudioInterrupt Failed");
    }

    // When the cellular call stream is pausing, only need to deactivate audio interrupt.
    CHECK_AND_RETURN_RET(!isVoiceCallCapturer_, true);
    return audioStream_->PauseAudioStream();
}

bool AudioCapturerPrivate::Stop() const
{
    Trace trace("AudioCapturer::Stop" + std::to_string(sessionID_));
    std::lock_guard lock(switchStreamMutex_);
    AUDIO_INFO_LOG("StreamClientState for Capturer::Stop. id %{public}u", sessionID_);
    CHECK_AND_RETURN_RET_LOG(!isSwitching_, false, "Operation failed, in switching");

    WriteOverflowEvent();
    int32_t ret = AudioPolicyManager::GetInstance().DeactivateAudioInterrupt(audioInterrupt_);
    if (ret != 0) {
        AUDIO_WARNING_LOG("AudioCapturer: DeactivateAudioInterrupt Failed");
    }

    CHECK_AND_RETURN_RET(isVoiceCallCapturer_ != true, true);

    return audioStream_->StopAudioStream();
}

bool AudioCapturerPrivate::Flush() const
{
    Trace trace("AudioCapturer::Flush");
    AUDIO_INFO_LOG("StreamClientState for Capturer::Flush. id %{public}u", sessionID_);
    return audioStream_->FlushAudioStream();
}

bool AudioCapturerPrivate::Release()
{
    AUDIO_INFO_LOG("StreamClientState for Capturer::Release. id %{public}u", sessionID_);

    abortRestore_ = true;
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(isValid_, false, "Release when capturer invalid");

    (void)AudioPolicyManager::GetInstance().DeactivateAudioInterrupt(audioInterrupt_);

    // Unregister the callaback in policy server
    (void)AudioPolicyManager::GetInstance().UnsetAudioInterruptCallback(sessionID_);

    std::shared_ptr<AudioCapturerConcurrencyCallbackImpl> cb = audioConcurrencyCallback_;
    if (cb != nullptr) {
        cb->UnsetAudioCapturerObj();
        AudioPolicyManager::GetInstance().UnsetAudioConcurrencyCallback(sessionID_);
    }

    RemoveCapturerPolicyServiceDiedCallback();

    return audioStream_->ReleaseAudioStream();
}

int32_t AudioCapturerPrivate::GetBufferSize(size_t &bufferSize) const
{
    Trace trace("AudioCapturer::GetBufferSize");
    return audioStream_->GetBufferSize(bufferSize);
}

int32_t AudioCapturerPrivate::GetAudioStreamId(uint32_t &sessionID) const
{
    CHECK_AND_RETURN_RET_LOG(audioStream_ != nullptr, ERR_INVALID_HANDLE, "GetAudioStreamId faied.");
    return audioStream_->GetAudioSessionID(sessionID);
}

int32_t AudioCapturerPrivate::SetBufferDuration(uint64_t bufferDuration) const
{
    CHECK_AND_RETURN_RET_LOG(bufferDuration >= MINIMUM_BUFFER_SIZE_MSEC && bufferDuration <= MAXIMUM_BUFFER_SIZE_MSEC,
        ERR_INVALID_PARAM, "Error: Please set the buffer duration between 5ms ~ 20ms");
    return audioStream_->SetBufferSizeInMsec(bufferDuration);
}

void AudioCapturerPrivate::SetApplicationCachePath(const std::string cachePath)
{
    cachePath_ = cachePath;
    if (audioStream_ != nullptr) {
        audioStream_->SetApplicationCachePath(cachePath_);
    } else {
        AUDIO_WARNING_LOG("AudioCapturer SetApplicationCachePath while stream is null");
    }
}

AudioCapturerInterruptCallbackImpl::AudioCapturerInterruptCallbackImpl(const std::shared_ptr<IAudioStream> &audioStream)
    : audioStream_(audioStream)
{
    AUDIO_DEBUG_LOG("AudioCapturerInterruptCallbackImpl constructor");
}

AudioCapturerInterruptCallbackImpl::~AudioCapturerInterruptCallbackImpl()
{
    AUDIO_DEBUG_LOG("AudioCapturerInterruptCallbackImpl: instance destroy");
}

void AudioCapturerInterruptCallbackImpl::SaveCallback(const std::weak_ptr<AudioCapturerCallback> &callback)
{
    callback_ = callback;
}

void AudioCapturerInterruptCallbackImpl::UpdateAudioStream(const std::shared_ptr<IAudioStream> &audioStream)
{
    std::lock_guard<std::mutex> lock(mutex_);
    audioStream_ = audioStream;
}

void AudioCapturerInterruptCallbackImpl::NotifyEvent(const InterruptEvent &interruptEvent)
{
    AUDIO_INFO_LOG("NotifyEvent: Hint: %{public}d, eventType: %{public}d",
        interruptEvent.hintType, interruptEvent.eventType);

    if (cb_ != nullptr) {
        cb_->OnInterrupt(interruptEvent);
        AUDIO_DEBUG_LOG("OnInterrupt : NotifyEvent to app complete");
    } else {
        AUDIO_DEBUG_LOG("cb_ == nullptr cannont NotifyEvent to app");
    }
}

void AudioCapturerInterruptCallbackImpl::NotifyForcePausedToResume(const InterruptEventInternal &interruptEvent)
{
    // Change InterruptForceType to Share, Since app will take care of resuming
    InterruptEvent interruptEventResume {interruptEvent.eventType, INTERRUPT_SHARE,
                                         interruptEvent.hintType};
    NotifyEvent(interruptEventResume);
}

void AudioCapturerInterruptCallbackImpl::HandleAndNotifyForcedEvent(const InterruptEventInternal &interruptEvent)
{
    State currentState = audioStream_->GetState();
    switch (interruptEvent.hintType) {
        case INTERRUPT_HINT_RESUME:
            CHECK_AND_RETURN_LOG((currentState == PAUSED || currentState == PREPARED) && isForcePaused_ == true,
                "OnInterrupt state %{public}d or not forced pause %{public}d before", currentState, isForcePaused_);
            AUDIO_INFO_LOG("set force pause false");
            isForcePaused_ = false;
            NotifyForcePausedToResume(interruptEvent);
            return;
        case INTERRUPT_HINT_PAUSE:
            CHECK_AND_RETURN_LOG(currentState == RUNNING || currentState == PREPARED,
                "OnInterrupt state %{public}d, no need to pause", currentState);
            (void)audioStream_->PauseAudioStream(); // Just Pause, do not deactivate here
            AUDIO_INFO_LOG("set force pause true");
            isForcePaused_ = true;
            break;
        case INTERRUPT_HINT_STOP:
            (void)audioStream_->StopAudioStream();
            break;
        default:
            break;
    }
    // Notify valid forced event callbacks to app
    InterruptEvent interruptEventForced {interruptEvent.eventType, interruptEvent.forceType, interruptEvent.hintType};
    NotifyEvent(interruptEventForced);
}

void AudioCapturerInterruptCallbackImpl::OnInterrupt(const InterruptEventInternal &interruptEvent)
{
    std::lock_guard<std::mutex> lock(mutex_);

    cb_ = callback_.lock();
    InterruptForceType forceType = interruptEvent.forceType;
    AUDIO_INFO_LOG("InterruptForceType: %{public}d", forceType);

    if (forceType != INTERRUPT_FORCE) { // INTERRUPT_SHARE
        AUDIO_DEBUG_LOG("AudioCapturerPrivate ForceType: INTERRUPT_SHARE. Let app handle the event");
        InterruptEvent interruptEventShared {interruptEvent.eventType, interruptEvent.forceType,
                                             interruptEvent.hintType};
        NotifyEvent(interruptEventShared);
        return;
    }

    CHECK_AND_RETURN_LOG(audioStream_ != nullptr,
        "Stream is not alive. No need to take forced action");

    HandleAndNotifyForcedEvent(interruptEvent);
}

void AudioStreamCallbackCapturer::SaveCallback(const std::weak_ptr<AudioCapturerCallback> &callback)
{
    callback_ = callback;
}

void AudioStreamCallbackCapturer::OnStateChange(const State state,
    const StateChangeCmdType __attribute__((unused)) cmdType)
{
    std::shared_ptr<AudioCapturerCallback> cb = callback_.lock();

    CHECK_AND_RETURN_LOG(cb != nullptr, "AudioStreamCallbackCapturer::OnStateChange cb == nullptr.");

    cb->OnStateChange(static_cast<CapturerState>(state));
}

std::vector<AudioSampleFormat> AudioCapturer::GetSupportedFormats()
{
    return AUDIO_SUPPORTED_FORMATS;
}

std::vector<AudioChannel> AudioCapturer::GetSupportedChannels()
{
    return CAPTURER_SUPPORTED_CHANNELS;
}

std::vector<AudioEncodingType> AudioCapturer::GetSupportedEncodingTypes()
{
    return AUDIO_SUPPORTED_ENCODING_TYPES;
}

std::vector<AudioSamplingRate> AudioCapturer::GetSupportedSamplingRates()
{
    return AUDIO_SUPPORTED_SAMPLING_RATES;
}

AudioStreamType AudioCapturer::FindStreamTypeBySourceType(SourceType sourceType)
{
    switch (sourceType) {
        case SOURCE_TYPE_VOICE_COMMUNICATION:
        case SOURCE_TYPE_VIRTUAL_CAPTURE:
            return STREAM_VOICE_CALL;
        case SOURCE_TYPE_WAKEUP:
            return STREAM_WAKEUP;
        case SOURCE_TYPE_VOICE_CALL:
            return STREAM_SOURCE_VOICE_CALL;
        case SOURCE_TYPE_CAMCORDER:
            return STREAM_CAMCORDER;
        default:
            return STREAM_MUSIC;
    }
}

int32_t AudioCapturerPrivate::SetAudioSourceConcurrency(const std::vector<SourceType> &targetSources)
{
    if (targetSources.size() <= 0) {
        AUDIO_ERR_LOG("TargetSources size is 0, set audio source concurrency failed.");
        return ERR_INVALID_PARAM;
    }
    AUDIO_INFO_LOG("Set audio source concurrency success.");
    audioInterrupt_.currencySources.sourcesTypes = targetSources;
    return SUCCESS;
}

int32_t AudioCapturerPrivate::SetCaptureMode(AudioCaptureMode captureMode)
{
    AUDIO_INFO_LOG("Capture mode: %{public}d", captureMode);
    audioCaptureMode_ = captureMode;

    if (capturerInfo_.sourceType == SOURCE_TYPE_VOICE_COMMUNICATION && captureMode == CAPTURE_MODE_CALLBACK &&
        AudioPolicyManager::GetInstance().GetPreferredInputStreamType(capturerInfo_) == AUDIO_FLAG_VOIP_FAST &&
        firstConcurrencyResult_ == SUCCESS) {
        AUDIO_INFO_LOG("Switch to fast voip stream");
        uint32_t sessionId = 0;
        int32_t ret = audioStream_->GetAudioSessionID(sessionId);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Get audio session Id failed");
        uint32_t newSessionId = 0;
        if (!SwitchToTargetStream(IAudioStream::VOIP_STREAM, newSessionId)) {
            AUDIO_ERR_LOG("Switch to target stream failed");
            return ERROR;
        }
        ret = AudioPolicyManager::GetInstance().RegisterDeviceChangeWithInfoCallback(newSessionId,
            inputDeviceChangeCallback_);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Register device change callback for new session failed");
        ret = AudioPolicyManager::GetInstance().UnregisterDeviceChangeWithInfoCallback(sessionId);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Unregister device change callback for old session failed");
    }

    return audioStream_->SetCaptureMode(captureMode);
}

AudioCaptureMode AudioCapturerPrivate::GetCaptureMode() const
{
    return audioStream_->GetCaptureMode();
}

int32_t AudioCapturerPrivate::SetCapturerReadCallback(const std::shared_ptr<AudioCapturerReadCallback> &callback)
{
    return audioStream_->SetCapturerReadCallback(callback);
}

int32_t AudioCapturerPrivate::GetBufferDesc(BufferDesc &bufDesc) const
{
    int32_t ret = audioStream_->GetBufferDesc(bufDesc);
    DumpFileUtil::WriteDumpFile(dumpFile_, static_cast<void *>(bufDesc.buffer), bufDesc.bufLength);
    return ret;
}

int32_t AudioCapturerPrivate::Enqueue(const BufferDesc &bufDesc) const
{
    CheckSignalData(bufDesc.buffer, bufDesc.bufLength);
    return audioStream_->Enqueue(bufDesc);
}

int32_t AudioCapturerPrivate::Clear() const
{
    return audioStream_->Clear();
}

int32_t AudioCapturerPrivate::GetBufQueueState(BufferQueueState &bufState) const
{
    return audioStream_->GetBufQueueState(bufState);
}

void AudioCapturerPrivate::SetValid(bool valid)
{
    std::lock_guard<std::mutex> lock(lock_);
    isValid_ = valid;
}

int64_t AudioCapturerPrivate::GetFramesRead() const
{
    return audioStream_->GetFramesRead();
}

int32_t AudioCapturerPrivate::GetCurrentInputDevices(AudioDeviceDescriptor &deviceInfo) const
{
    std::vector<std::unique_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    uint32_t sessionId = static_cast<uint32_t>(-1);
    int32_t ret = GetAudioStreamId(sessionId);
    CHECK_AND_RETURN_RET_LOG(!ret, ret, "Get sessionId failed");

    ret = AudioPolicyManager::GetInstance().GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
    CHECK_AND_RETURN_RET_LOG(!ret, ret, "Get current capturer devices failed");

    for (auto it = audioCapturerChangeInfos.begin(); it != audioCapturerChangeInfos.end(); it++) {
        if ((*it)->sessionId == static_cast<int32_t>(sessionId)) {
            deviceInfo = (*it)->inputDeviceInfo;
        }
    }
    return SUCCESS;
}

int32_t AudioCapturerPrivate::GetCurrentCapturerChangeInfo(AudioCapturerChangeInfo &changeInfo) const
{
    std::vector<std::unique_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    uint32_t sessionId = static_cast<uint32_t>(-1);
    int32_t ret = GetAudioStreamId(sessionId);
    CHECK_AND_RETURN_RET_LOG(!ret, ret, "Get sessionId failed");

    ret = AudioPolicyManager::GetInstance().GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
    CHECK_AND_RETURN_RET_LOG(!ret, ret, "Get current capturer devices failed");

    for (auto it = audioCapturerChangeInfos.begin(); it != audioCapturerChangeInfos.end(); it++) {
        if ((*it)->sessionId == static_cast<int32_t>(sessionId)) {
            changeInfo = *(*it);
        }
    }
    return SUCCESS;
}

std::vector<sptr<MicrophoneDescriptor>> AudioCapturerPrivate::GetCurrentMicrophones() const
{
    uint32_t sessionId = static_cast<uint32_t>(-1);
    GetAudioStreamId(sessionId);
    return AudioPolicyManager::GetInstance().GetAudioCapturerMicrophoneDescriptors(sessionId);
}

int32_t AudioCapturerPrivate::SetAudioCapturerDeviceChangeCallback(
    const std::shared_ptr<AudioCapturerDeviceChangeCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERROR, "Callback is null");

    if (RegisterAudioCapturerEventListener() != SUCCESS) {
        return ERROR;
    }

    CHECK_AND_RETURN_RET_LOG(audioStateChangeCallback_ != nullptr, ERROR, "audioStateChangeCallback_ is null");
    audioStateChangeCallback_->SaveDeviceChangeCallback(callback);
    return SUCCESS;
}

int32_t AudioCapturerPrivate::RemoveAudioCapturerDeviceChangeCallback(
    const std::shared_ptr<AudioCapturerDeviceChangeCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(audioStateChangeCallback_ != nullptr, ERROR, "audioStateChangeCallback_ is null");

    audioStateChangeCallback_->RemoveDeviceChangeCallback(callback);
    if (UnregisterAudioCapturerEventListener() != SUCCESS) {
        return ERROR;
    }
    return SUCCESS;
}

bool AudioCapturerPrivate::IsDeviceChanged(AudioDeviceDescriptor &newDeviceInfo)
{
    bool deviceUpdated = false;
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);

    CHECK_AND_RETURN_RET_LOG(GetCurrentInputDevices(deviceInfo) == SUCCESS, deviceUpdated,
        "GetCurrentInputDevices failed");

    if (currentDeviceInfo_.deviceType_ != deviceInfo.deviceType_) {
        currentDeviceInfo_ = deviceInfo;
        newDeviceInfo = currentDeviceInfo_;
        deviceUpdated = true;
    }
    return deviceUpdated;
}

void AudioCapturerPrivate::GetAudioInterrupt(AudioInterrupt &audioInterrupt)
{
    audioInterrupt = audioInterrupt_;
}

void AudioCapturerPrivate::WriteOverflowEvent() const
{
    AUDIO_INFO_LOG("Write overflowEvent to media monitor");
    if (GetOverflowCount() < WRITE_OVERFLOW_NUM) {
        return;
    }
    AudioPipeType pipeType = PIPE_TYPE_NORMAL_IN;
    IAudioStream::StreamClass streamClass = audioStream_->GetStreamClass();
    if (streamClass == IAudioStream::FAST_STREAM) {
        pipeType = PIPE_TYPE_LOWLATENCY_IN;
    }
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::ModuleId::AUDIO, Media::MediaMonitor::EventId::PERFORMANCE_UNDER_OVERRUN_STATS,
        Media::MediaMonitor::EventType::FREQUENCY_AGGREGATION_EVENT);
    bean->Add("IS_PLAYBACK", 0);
    bean->Add("CLIENT_UID", appInfo_.appUid);
    bean->Add("PIPE_TYPE", pipeType);
    bean->Add("STREAM_TYPE", capturerInfo_.sourceType);
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

int32_t AudioCapturerPrivate::RegisterAudioCapturerEventListener()
{
    if (!audioStateChangeCallback_) {
        audioStateChangeCallback_ = std::make_shared<AudioCapturerStateChangeCallbackImpl>();
        CHECK_AND_RETURN_RET_LOG(audioStateChangeCallback_, ERROR, "Memory allocation failed!!");

        int32_t ret =
            AudioPolicyManager::GetInstance().RegisterAudioCapturerEventListener(getpid(), audioStateChangeCallback_);
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "RegisterAudioCapturerEventListener failed");
        audioStateChangeCallback_->setAudioCapturerObj(this);
    }
    return SUCCESS;
}

int32_t AudioCapturerPrivate::UnregisterAudioCapturerEventListener()
{
    CHECK_AND_RETURN_RET_LOG(audioStateChangeCallback_ != nullptr, ERROR, "audioStateChangeCallback_ is null");
    if (audioStateChangeCallback_->DeviceChangeCallbackArraySize() == 0 &&
        audioStateChangeCallback_->GetCapturerInfoChangeCallbackArraySize() == 0) {
        int32_t ret =
            AudioPolicyManager::GetInstance().UnregisterAudioCapturerEventListener(getpid());
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "failed");
        audioStateChangeCallback_->HandleCapturerDestructor();
        audioStateChangeCallback_ = nullptr;
    }
    return SUCCESS;
}

int32_t AudioCapturerPrivate::SetAudioCapturerInfoChangeCallback(
    const std::shared_ptr<AudioCapturerInfoChangeCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "Callback is null");

    CHECK_AND_RETURN_RET(RegisterAudioCapturerEventListener() == SUCCESS, ERROR);

    CHECK_AND_RETURN_RET_LOG(audioStateChangeCallback_ != nullptr, ERROR, "audioStateChangeCallback_ is null");
    audioStateChangeCallback_->SaveCapturerInfoChangeCallback(callback);
    return SUCCESS;
}

int32_t AudioCapturerPrivate::RemoveAudioCapturerInfoChangeCallback(
    const std::shared_ptr<AudioCapturerInfoChangeCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(audioStateChangeCallback_ != nullptr, ERROR, "audioStateChangeCallback_ is null");
    audioStateChangeCallback_->RemoveCapturerInfoChangeCallback(callback);
    CHECK_AND_RETURN_RET(UnregisterAudioCapturerEventListener() == SUCCESS, ERROR);
    return SUCCESS;
}

int32_t AudioCapturerPrivate::RegisterCapturerPolicyServiceDiedCallback()
{
    std::lock_guard<std::mutex> lock(capturerPolicyServiceDiedCbMutex_);
    AUDIO_DEBUG_LOG("AudioCapturerPrivate::SetCapturerPolicyServiceDiedCallback");
    if (!audioPolicyServiceDiedCallback_) {
        audioPolicyServiceDiedCallback_ = std::make_shared<CapturerPolicyServiceDiedCallback>();
        if (!audioPolicyServiceDiedCallback_) {
            AUDIO_ERR_LOG("Memory allocation failed!!");
            return ERROR;
        }
        AudioPolicyManager::GetInstance().RegisterAudioStreamPolicyServerDiedCb(audioPolicyServiceDiedCallback_);
        audioPolicyServiceDiedCallback_->SetAudioCapturerObj(this);
        audioPolicyServiceDiedCallback_->SetAudioInterrupt(audioInterrupt_);
    }
    return SUCCESS;
}

int32_t AudioCapturerPrivate::RemoveCapturerPolicyServiceDiedCallback()
{
    std::lock_guard<std::mutex> lock(capturerPolicyServiceDiedCbMutex_);
    AUDIO_DEBUG_LOG("AudioCapturerPrivate::RemoveCapturerPolicyServiceDiedCallback");
    if (audioPolicyServiceDiedCallback_) {
        int32_t ret = AudioPolicyManager::GetInstance().UnregisterAudioStreamPolicyServerDiedCb(
            audioPolicyServiceDiedCallback_);
        if (ret != 0) {
            AUDIO_ERR_LOG("RemoveCapturerPolicyServiceDiedCallback failed");
            audioPolicyServiceDiedCallback_ = nullptr;
            return ERROR;
        }
    }
    audioPolicyServiceDiedCallback_ = nullptr;
    return SUCCESS;
}

uint32_t AudioCapturerPrivate::GetOverflowCount() const
{
    return audioStream_->GetOverflowCount();
}

void AudioCapturerPrivate::SetSwitchInfo(IAudioStream::SwitchInfo info, std::shared_ptr<IAudioStream> audioStream)
{
    CHECK_AND_RETURN_LOG(audioStream, "stream is nullptr");

    audioStream->SetStreamTrackerState(false);
    audioStream->SetApplicationCachePath(info.cachePath);
    audioStream->SetClientID(info.clientPid, info.clientUid, appInfo_.appTokenId, appInfo_.appFullTokenId);
    audioStream->SetCapturerInfo(info.capturerInfo);
    audioStream->SetAudioStreamInfo(info.params, capturerProxyObj_);
    audioStream->SetCaptureMode(info.captureMode);

    // set callback
    if ((info.renderPositionCb != nullptr) && (info.frameMarkPosition > 0)) {
        audioStream->SetRendererPositionCallback(info.frameMarkPosition, info.renderPositionCb);
    }

    if ((info.capturePositionCb != nullptr) && (info.frameMarkPosition > 0)) {
        audioStream->SetCapturerPositionCallback(info.frameMarkPosition, info.capturePositionCb);
    }

    if ((info.renderPeriodPositionCb != nullptr) && (info.framePeriodNumber > 0)) {
        audioStream->SetRendererPeriodPositionCallback(info.framePeriodNumber, info.renderPeriodPositionCb);
    }

    if ((info.capturePeriodPositionCb != nullptr) && (info.framePeriodNumber > 0)) {
        audioStream->SetCapturerPeriodPositionCallback(info.framePeriodNumber, info.capturePeriodPositionCb);
    }

    audioStream->SetCapturerReadCallback(info.capturerReadCallback);

    audioStream->SetStreamCallback(info.audioStreamCallback);
}

bool AudioCapturerPrivate::SwitchToTargetStream(IAudioStream::StreamClass targetClass, uint32_t &newSessionId)
{
    bool switchResult = false;
    if (audioStream_) {
        Trace trace("SwitchToTargetStream");
        isSwitching_ = true;
        CapturerState previousState = GetStatus();
        AUDIO_INFO_LOG("Previous stream state: %{public}d, original sessionId: %{public}u", previousState, sessionID_);
        if (previousState == CAPTURER_RUNNING) {
            // stop old stream
            switchResult = audioStream_->StopAudioStream();
            CHECK_AND_RETURN_RET_LOG(switchResult, false, "StopAudioStream failed.");
        }
        std::lock_guard lock(switchStreamMutex_);
        // switch new stream
        IAudioStream::SwitchInfo info;
        audioStream_->GetSwitchInfo(info);
        info.params.originalSessionId = sessionID_;

        // release old stream and restart audio stream
        switchResult = audioStream_->ReleaseAudioStream();
        CHECK_AND_RETURN_RET_LOG(switchResult, false, "release old stream failed.");

        if (targetClass == IAudioStream::VOIP_STREAM) {
            info.capturerInfo.originalFlag = AUDIO_FLAG_VOIP_FAST;
        }
        std::shared_ptr<IAudioStream> newAudioStream = IAudioStream::GetRecordStream(targetClass, info.params,
            info.eStreamType, appInfo_.appPid);
        CHECK_AND_RETURN_RET_LOG(newAudioStream != nullptr, false, "GetRecordStream failed.");
        AUDIO_INFO_LOG("Get new stream success!");

        // set new stream info
        SetSwitchInfo(info, newAudioStream);

        if (previousState == CAPTURER_RUNNING) {
            // restart audio stream
            switchResult = newAudioStream->StartAudioStream();
            CHECK_AND_RETURN_RET_LOG(switchResult, false, "start new stream failed.");
        }
        audioStream_ = newAudioStream;
        if (audioInterruptCallback_ != nullptr) {
            std::shared_ptr<AudioCapturerInterruptCallbackImpl> interruptCbImpl =
                std::static_pointer_cast<AudioCapturerInterruptCallbackImpl>(audioInterruptCallback_);
            interruptCbImpl->UpdateAudioStream(audioStream_);
        }
        isSwitching_ = false;
        audioStream_->GetAudioSessionID(newSessionId);
        switchResult = true;
    }
    return switchResult;
}

void AudioCapturerPrivate::SwitchStream(const uint32_t sessionId, const int32_t streamFlag,
    const AudioStreamDeviceChangeReasonExt reason)
{
    IAudioStream::StreamClass targetClass = IAudioStream::PA_STREAM;
    switch (streamFlag) {
        case AUDIO_FLAG_NORMAL:
            capturerInfo_.capturerFlags = AUDIO_FLAG_NORMAL;
            targetClass = IAudioStream::PA_STREAM;
            break;
        case AUDIO_FLAG_MMAP:
            capturerInfo_.capturerFlags = AUDIO_FLAG_MMAP;
            targetClass = IAudioStream::FAST_STREAM;
            break;
        case AUDIO_FLAG_VOIP_FAST:
            capturerInfo_.capturerFlags = AUDIO_FLAG_VOIP_FAST;
            targetClass = IAudioStream::VOIP_STREAM;
            break;
    }

    uint32_t newSessionId = 0;
    if (!SwitchToTargetStream(targetClass, newSessionId)) {
        int32_t ret = AudioPolicyManager::GetInstance().DeactivateAudioInterrupt(audioInterrupt_);
        CHECK_AND_RETURN_LOG(ret == 0, "DeactivateAudioInterrupt Failed");
        AUDIO_ERR_LOG("Switch to target stream failed");
    }
    int32_t ret = AudioPolicyManager::GetInstance().RegisterDeviceChangeWithInfoCallback(newSessionId,
        inputDeviceChangeCallback_);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "Register device change callback for new session failed");
    ret = AudioPolicyManager::GetInstance().UnregisterDeviceChangeWithInfoCallback(sessionId);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "Unregister device change callback for old session failed");
}

void AudioCapturerPrivate::ActivateAudioConcurrency(IAudioStream::StreamClass &streamClass)
{
    capturerInfo_.pipeType = PIPE_TYPE_NORMAL_IN;
    if (capturerInfo_.sourceType == SOURCE_TYPE_VOICE_COMMUNICATION) {
        capturerInfo_.pipeType = PIPE_TYPE_CALL_IN;
    } else if (streamClass == IAudioStream::FAST_STREAM) {
        capturerInfo_.pipeType = PIPE_TYPE_LOWLATENCY_IN;
    }
    int32_t ret = AudioPolicyManager::GetInstance().ActivateAudioConcurrency(capturerInfo_.pipeType);
    if (ret != SUCCESS && streamClass == IAudioStream::FAST_STREAM) {
        streamClass = IAudioStream::PA_STREAM;
        capturerInfo_.pipeType = PIPE_TYPE_NORMAL_IN;
    }
    firstConcurrencyResult_ = ret;
    return;
}

int32_t AudioCapturerPrivate::InitAudioConcurrencyCallback()
{
    if (audioConcurrencyCallback_ == nullptr) {
        audioConcurrencyCallback_ = std::make_shared<AudioCapturerConcurrencyCallbackImpl>();
        CHECK_AND_RETURN_RET_LOG(audioConcurrencyCallback_ != nullptr, ERROR, "Memory Allocation Failed !!");
    }
    CHECK_AND_RETURN_RET_LOG(audioStream_->GetAudioSessionID(sessionID_) == SUCCESS, ERR_INVALID_INDEX,
        "Get session id failed!");
    audioConcurrencyCallback_->SetAudioCapturerObj(this);
    return AudioPolicyManager::GetInstance().SetAudioConcurrencyCallback(sessionID_, audioConcurrencyCallback_);
}

void AudioCapturerPrivate::ConcedeStream()
{
    AUDIO_INFO_LOG("session %{public}u concede from pipeType %{public}d", sessionID_, capturerInfo_.pipeType);
    AudioPipeType pipeType = PIPE_TYPE_NORMAL_IN;
    audioStream_->GetAudioPipeType(pipeType);
    if (pipeType == PIPE_TYPE_LOWLATENCY_IN || pipeType == PIPE_TYPE_CALL_IN) {
        SwitchStream(sessionID_, IAudioStream::PA_STREAM, AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN);
    }
}

AudioCapturerConcurrencyCallbackImpl::AudioCapturerConcurrencyCallbackImpl()
{
    AUDIO_INFO_LOG("AudioCapturerConcurrencyCallbackImpl ctor");
}

AudioCapturerConcurrencyCallbackImpl::~AudioCapturerConcurrencyCallbackImpl()
{
    AUDIO_INFO_LOG("AudioCapturerConcurrencyCallbackImpl dtor");
}

void AudioCapturerConcurrencyCallbackImpl::OnConcedeStream()
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_LOG(capturer_ != nullptr, "capturer is nullptr");
    capturer_->ConcedeStream();
}

AudioCapturerStateChangeCallbackImpl::AudioCapturerStateChangeCallbackImpl()
{
    AUDIO_DEBUG_LOG("AudioCapturerStateChangeCallbackImpl instance create");
}

AudioCapturerStateChangeCallbackImpl::~AudioCapturerStateChangeCallbackImpl()
{
    AUDIO_DEBUG_LOG("AudioCapturerStateChangeCallbackImpl instance destory");
}

void AudioCapturerStateChangeCallbackImpl::SaveCapturerInfoChangeCallback(
    const std::shared_ptr<AudioCapturerInfoChangeCallback> &callback)
{
    std::lock_guard<std::mutex> lock(capturerMutex_);
    auto iter = find(capturerInfoChangeCallbacklist_.begin(), capturerInfoChangeCallbacklist_.end(), callback);
    if (iter == capturerInfoChangeCallbacklist_.end()) {
        capturerInfoChangeCallbacklist_.emplace_back(callback);
    }
}

void AudioCapturerStateChangeCallbackImpl::RemoveCapturerInfoChangeCallback(
    const std::shared_ptr<AudioCapturerInfoChangeCallback> &callback)
{
    std::lock_guard<std::mutex> lock(capturerMutex_);
    if (callback == nullptr) {
        capturerInfoChangeCallbacklist_.clear();
        return;
    }

    auto iter = find(capturerInfoChangeCallbacklist_.begin(), capturerInfoChangeCallbacklist_.end(), callback);
    if (iter != capturerInfoChangeCallbacklist_.end()) {
        capturerInfoChangeCallbacklist_.erase(iter);
    }
}

int32_t AudioCapturerStateChangeCallbackImpl::GetCapturerInfoChangeCallbackArraySize()
{
    std::lock_guard<std::mutex> lock(capturerMutex_);
    return capturerInfoChangeCallbacklist_.size();
}

void AudioCapturerStateChangeCallbackImpl::SaveDeviceChangeCallback(
    const std::shared_ptr<AudioCapturerDeviceChangeCallback> &callback)
{
    auto iter = find(deviceChangeCallbacklist_.begin(), deviceChangeCallbacklist_.end(), callback);
    if (iter == deviceChangeCallbacklist_.end()) {
        deviceChangeCallbacklist_.emplace_back(callback);
    }
}

void AudioCapturerStateChangeCallbackImpl::RemoveDeviceChangeCallback(
    const std::shared_ptr<AudioCapturerDeviceChangeCallback> &callback)
{
    if (callback == nullptr) {
        deviceChangeCallbacklist_.clear();
        return;
    }

    auto iter = find(deviceChangeCallbacklist_.begin(), deviceChangeCallbacklist_.end(), callback);
    if (iter != deviceChangeCallbacklist_.end()) {
        deviceChangeCallbacklist_.erase(iter);
    }
}

int32_t AudioCapturerStateChangeCallbackImpl::DeviceChangeCallbackArraySize()
{
    return deviceChangeCallbacklist_.size();
}

void AudioCapturerStateChangeCallbackImpl::setAudioCapturerObj(AudioCapturerPrivate *capturerObj)
{
    std::lock_guard<std::mutex> lock(capturerMutex_);
    capturer_ = capturerObj;
}

void AudioCapturerStateChangeCallbackImpl::NotifyAudioCapturerInfoChange(
    const std::vector<std::unique_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos)
{
    uint32_t sessionId = static_cast<uint32_t>(-1);
    bool found = false;
    AudioCapturerChangeInfo capturerChangeInfo;
    std::vector<std::shared_ptr<AudioCapturerInfoChangeCallback>> capturerInfoChangeCallbacklist;

    {
        std::lock_guard<std::mutex> lock(capturerMutex_);
        CHECK_AND_RETURN_LOG(capturer_ != nullptr, "Bare pointer capturer_ is nullptr");
        int32_t ret = capturer_->GetAudioStreamId(sessionId);
        CHECK_AND_RETURN_LOG(!ret, "Get sessionId failed");
    }

    for (auto it = audioCapturerChangeInfos.begin(); it != audioCapturerChangeInfos.end(); it++) {
        if ((*it)->sessionId == static_cast<int32_t>(sessionId)) {
            capturerChangeInfo = *(*it);
            found = true;
        }
    }

    {
        std::lock_guard<std::mutex> lock(capturerMutex_);
        capturerInfoChangeCallbacklist = capturerInfoChangeCallbacklist_;
    }
    if (found) {
        for (auto it = capturerInfoChangeCallbacklist.begin(); it != capturerInfoChangeCallbacklist.end(); ++it) {
            if (*it != nullptr) {
                (*it)->OnStateChange(capturerChangeInfo);
            }
        }
    }
}

void AudioCapturerStateChangeCallbackImpl::NotifyAudioCapturerDeviceChange(
    const std::vector<std::unique_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos)
{
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    {
        std::lock_guard<std::mutex> lock(capturerMutex_);
        CHECK_AND_RETURN_LOG(capturer_ != nullptr, "Bare pointer capturer_ is nullptr");
        CHECK_AND_RETURN_LOG(capturer_->IsDeviceChanged(deviceInfo), "Device not change, no need callback.");
    }

    for (auto it = deviceChangeCallbacklist_.begin(); it != deviceChangeCallbacklist_.end(); ++it) {
        if (*it != nullptr) {
            (*it)->OnStateChange(deviceInfo);
        }
    }
}

void AudioCapturerStateChangeCallbackImpl::OnCapturerStateChange(
    const std::vector<std::unique_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos)
{
    if (deviceChangeCallbacklist_.size() != 0) {
        NotifyAudioCapturerDeviceChange(audioCapturerChangeInfos);
    }

    if (capturerInfoChangeCallbacklist_.size() != 0) {
        NotifyAudioCapturerInfoChange(audioCapturerChangeInfos);
    }
}

void AudioCapturerStateChangeCallbackImpl::HandleCapturerDestructor()
{
    std::lock_guard<std::mutex> lock(capturerMutex_);
    capturer_ = nullptr;
}

void InputDeviceChangeWithInfoCallbackImpl::OnDeviceChangeWithInfo(
    const uint32_t sessionId, const AudioDeviceDescriptor &deviceInfo, const AudioStreamDeviceChangeReasonExt reason)
{
    AUDIO_INFO_LOG("For capturer, OnDeviceChangeWithInfo callback is not support");
}

void InputDeviceChangeWithInfoCallbackImpl::OnRecreateStreamEvent(const uint32_t sessionId, const int32_t streamFlag,
    const AudioStreamDeviceChangeReasonExt reason)
{
    AUDIO_INFO_LOG("Enter");
    capturer_->SwitchStream(sessionId, streamFlag, reason);
}

CapturerPolicyServiceDiedCallback::CapturerPolicyServiceDiedCallback()
{
    AUDIO_DEBUG_LOG("CapturerPolicyServiceDiedCallback create");
}

CapturerPolicyServiceDiedCallback::~CapturerPolicyServiceDiedCallback()
{
    AUDIO_DEBUG_LOG("CapturerPolicyServiceDiedCallback destroy");
    if (restoreThread_ != nullptr && restoreThread_->joinable()) {
        restoreThread_->join();
        restoreThread_.reset();
        restoreThread_ = nullptr;
    }
}

void CapturerPolicyServiceDiedCallback::SetAudioCapturerObj(AudioCapturerPrivate *capturerObj)
{
    capturer_ = capturerObj;
}

void CapturerPolicyServiceDiedCallback::SetAudioInterrupt(AudioInterrupt &audioInterrupt)
{
    audioInterrupt_ = audioInterrupt;
}

void CapturerPolicyServiceDiedCallback::OnAudioPolicyServiceDied()
{
    AUDIO_INFO_LOG("CapturerPolicyServiceDiedCallback OnAudioPolicyServiceDied");
    if (restoreThread_ != nullptr) {
        restoreThread_->detach();
    }
    restoreThread_ = std::make_unique<std::thread>([this] { this->RestoreTheadLoop(); });
    pthread_setname_np(restoreThread_->native_handle(), "OS_ACPSRestore");
}

void CapturerPolicyServiceDiedCallback::RestoreTheadLoop()
{
    int32_t tryCounter = 5;
    uint32_t sleepTime = 500000;
    bool result = false;
    int32_t ret = -1;
    while (!result && tryCounter > 0) {
        tryCounter--;
        usleep(sleepTime);
        if (capturer_== nullptr || capturer_->audioStream_== nullptr ||
            capturer_->abortRestore_) {
            AUDIO_INFO_LOG("CapturerPolicyServiceDiedCallback RestoreTheadLoop abort restore");
            break;
        }
        result = capturer_->audioStream_->RestoreAudioStream();
        if (!result) {
            AUDIO_ERR_LOG("RestoreAudioStream Failed, %{public}d attempts remaining", tryCounter);
            continue;
        } else {
            capturer_->abortRestore_ = false;
        }

        if (capturer_->GetStatus() == CAPTURER_RUNNING) {
            capturer_->GetAudioInterrupt(audioInterrupt_);
            ret = AudioPolicyManager::GetInstance().ActivateAudioInterrupt(audioInterrupt_);
            if (ret != SUCCESS) {
                AUDIO_ERR_LOG("RestoreTheadLoop ActivateAudioInterrupt Failed");
            }
        }
    }
}
}  // namespace AudioStandard
}  // namespace OHOS
