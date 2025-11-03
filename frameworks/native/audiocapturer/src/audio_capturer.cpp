/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#include "shared_audio_capturer_wrapper.h"

#include <cinttypes>

#include "audio_capturer_private.h"
#include "audio_errors.h"
#include "audio_capturer_log.h"
#include "audio_policy_manager.h"
#include "audio_system_manager.h"

#include "media_monitor_manager.h"
#include "audio_stream_descriptor.h"

namespace OHOS {
namespace AudioStandard {
static constexpr uid_t UID_MSDP_SA = 6699;
static constexpr int32_t WRITE_OVERFLOW_NUM = 100;
static constexpr int32_t AUDIO_SOURCE_TYPE_INVALID_5 = 5;
static constexpr uint32_t BLOCK_INTERRUPT_CALLBACK_IN_MS = 1000; // 1000ms
static constexpr uint32_t BLOCK_INTERRUPT_OVERTIMES_IN_MS = 3000; // 3s
static constexpr int32_t MINIMUM_BUFFER_SIZE_MSEC = 5;
static constexpr int32_t MAXIMUM_BUFFER_SIZE_MSEC = 20;
static constexpr uint32_t DECIMAL_BASE = 10;
static constexpr int32_t UID_MEDIA_SA = 1013;

std::map<AudioStreamType, SourceType> AudioCapturerPrivate::streamToSource_ = {
    {AudioStreamType::STREAM_MUSIC, SourceType::SOURCE_TYPE_MIC},
    {AudioStreamType::STREAM_MEDIA, SourceType::SOURCE_TYPE_MIC},
    {AudioStreamType::STREAM_MUSIC, SourceType::SOURCE_TYPE_UNPROCESSED},
    {AudioStreamType::STREAM_CAMCORDER, SourceType::SOURCE_TYPE_CAMCORDER},
    {AudioStreamType::STREAM_VOICE_CALL, SourceType::SOURCE_TYPE_VOICE_COMMUNICATION},
    {AudioStreamType::STREAM_ULTRASONIC, SourceType::SOURCE_TYPE_ULTRASONIC},
    {AudioStreamType::STREAM_WAKEUP, SourceType::SOURCE_TYPE_WAKEUP},
    {AudioStreamType::STREAM_SOURCE_VOICE_CALL, SourceType::SOURCE_TYPE_VOICE_CALL},
    {AudioStreamType::STREAM_MUSIC, SourceType::SOURCE_TYPE_LIVE},
    {AudioStreamType::STREAM_MEDIA, SourceType::SOURCE_TYPE_LIVE},
};

static const std::map<uint32_t, IAudioStream::StreamClass> AUDIO_INPUT_FLAG_GROUP_MAP = {
    {AUDIO_INPUT_FLAG_NORMAL, IAudioStream::StreamClass::PA_STREAM},
    {AUDIO_INPUT_FLAG_FAST, IAudioStream::StreamClass::FAST_STREAM},
    {AUDIO_INPUT_FLAG_VOIP_FAST, IAudioStream::StreamClass::VOIP_STREAM},
    {AUDIO_INPUT_FLAG_WAKEUP, IAudioStream::StreamClass::PA_STREAM},
};

static const std::map<AudioFlag, int32_t> INPUT_ROUTE_TO_STREAM_MAP = {
    {AUDIO_OUTPUT_FLAG_NORMAL, AUDIO_FLAG_NORMAL},
    {AUDIO_OUTPUT_FLAG_DIRECT, AUDIO_FLAG_DIRECT},
    {AUDIO_OUTPUT_FLAG_FAST, AUDIO_FLAG_MMAP},
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
    CapturerState state = GetStatus();
    if (state != CAPTURER_RELEASED && state != CAPTURER_NEW) {
        Release();
    }
    AudioPolicyManager::GetInstance().RemoveClientTrackerStub(sessionID_);
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
    std::shared_ptr<AudioCapturer> sharedCapturer = std::make_shared<AudioCapturerPrivate>(audioStreamType,
        appInfo, true);
    CHECK_AND_RETURN_RET_LOG(sharedCapturer != nullptr, nullptr, "capturer is nullptr");

    return std::make_unique<SharedCapturerWrapper>(sharedCapturer);
}

std::unique_ptr<AudioCapturer> AudioCapturer::Create(const AudioCapturerOptions &options)
{
    AppInfo appInfo = {};
    return Create(options, appInfo);
}

std::unique_ptr<AudioCapturer> AudioCapturer::Create(const AudioCapturerOptions &options, const std::string cachePath)
{
    AppInfo appInfo = {};
    return Create(options, appInfo);
}

std::unique_ptr<AudioCapturer> AudioCapturer::Create(const AudioCapturerOptions &options,
    const std::string cachePath, const AppInfo &appInfo)
{
    return Create(options, appInfo);
}

std::unique_ptr<AudioCapturer> AudioCapturer::Create(const AudioCapturerOptions &options,
    const AppInfo &appInfo)
{
    auto tempSharedPtr = CreateCapturer(options, appInfo);
    CHECK_AND_RETURN_RET_LOG(tempSharedPtr != nullptr, nullptr, "capturer is nullptr");

    return std::make_unique<SharedCapturerWrapper>(tempSharedPtr);
}

void AudioCapturerPrivate::HandleSetCapturerInfoByOptions(const AudioCapturerOptions &capturerOptions,
    const AppInfo &appInfo)
{
    capturerInfo_.sourceType = capturerOptions.capturerInfo.sourceType;
    capturerInfo_.capturerFlags = capturerOptions.capturerInfo.capturerFlags;
    capturerInfo_.originalFlag = ((capturerOptions.capturerInfo.sourceType == SOURCE_TYPE_VOICE_COMMUNICATION) &&
        (capturerOptions.capturerInfo.capturerFlags == AUDIO_FLAG_MMAP)) ?
        AUDIO_FLAG_NORMAL : capturerOptions.capturerInfo.capturerFlags;
    capturerInfo_.samplingRate = capturerOptions.streamInfo.samplingRate;
    capturerInfo_.recorderType = capturerOptions.capturerInfo.recorderType;
    capturerInfo_.isLoopback = capturerOptions.capturerInfo.isLoopback;
    capturerInfo_.loopbackMode = capturerOptions.capturerInfo.loopbackMode;
    // InitPlaybackCapturer will be replaced by UpdatePlaybackCaptureConfig.
    filterConfig_ = capturerOptions.playbackCaptureConfig;
    strategy_ = capturerOptions.strategy;
}

// LCOV_EXCL_START
std::shared_ptr<AudioCapturer> AudioCapturer::CreateCapturer(const AudioCapturerOptions &capturerOptions,
    const AppInfo &appInfo)
{
    Trace trace("KeyAction AudioCapturer::Create");
    auto sourceType = capturerOptions.capturerInfo.sourceType;
    if (sourceType == SOURCE_TYPE_VIRTUAL_CAPTURE) {
        AUDIO_ERR_LOG("Invalid sourceType %{public}d!", sourceType);
        return nullptr;
    }
    if (sourceType < SOURCE_TYPE_MIC || sourceType > SOURCE_TYPE_MAX ||
        sourceType == AUDIO_SOURCE_TYPE_INVALID_5) {
        AudioCapturer::SendCapturerCreateError(sourceType, ERR_INVALID_PARAM);
        AUDIO_ERR_LOG("Invalid sourceType %{public}d!", sourceType);
        return nullptr;
    }
    if (sourceType == SOURCE_TYPE_ULTRASONIC && getuid() != UID_MSDP_SA) {
        AudioCapturer::SendCapturerCreateError(sourceType, ERR_INVALID_PARAM);
        AUDIO_ERR_LOG("Create failed: SOURCE_TYPE_ULTRASONIC can only be used by MSDP");
        return nullptr;
    }
    
    AUDIO_INFO_LOG("StreamClientState for Capturer::CreateCapturer sourceType:%{public}d, capturerFlags:%{public}d, "
        "AppInfo:[%{public}d] [%{public}s] [%{public}s], ", sourceType, capturerOptions.capturerInfo.capturerFlags,
        appInfo.appUid, appInfo.appTokenId == 0 ? "T" : "F", appInfo.appFullTokenId == 0 ? "T" : "F");

    AudioStreamType audioStreamType = FindStreamTypeBySourceType(sourceType);
    AudioCapturerParams params;
    params.preferredInputDevice = capturerOptions.preferredInputDevice;
    params.audioSampleFormat = capturerOptions.streamInfo.format;
    params.samplingRate = capturerOptions.streamInfo.samplingRate;
    params.audioChannel = AudioChannel::CHANNEL_3 == capturerOptions.streamInfo.channels ? AudioChannel::STEREO :
        capturerOptions.streamInfo.channels;
    params.audioEncoding = capturerOptions.streamInfo.encoding;
    params.channelLayout = capturerOptions.streamInfo.channelLayout;
    auto capturer = std::make_shared<AudioCapturerPrivate>(audioStreamType, appInfo, false);

    if (capturer == nullptr) {
        AudioCapturer::SendCapturerCreateError(sourceType, ERR_OPERATION_FAILED);
        AUDIO_ERR_LOG("Failed to create capturer object");
        return nullptr;
    }
    capturer->HandleSetCapturerInfoByOptions(capturerOptions, appInfo);
    if (capturer->SetParams(params) != SUCCESS) {
        AudioCapturer::SendCapturerCreateError(sourceType, ERR_OPERATION_FAILED);
        capturer = nullptr;
    }
    if (capturer != nullptr && AudioChannel::CHANNEL_3 == capturerOptions.streamInfo.channels) {
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

#ifdef HAS_FEATURE_INNERCAPTURER
    if (config.filterOptions.usages.size() == 0 && config.filterOptions.pids.size() == 0) {
        AUDIO_WARNING_LOG("Both usages and pids are empty!");
    }

    CHECK_AND_RETURN_RET_LOG(audioStream_ != nullptr, ERR_OPERATION_FAILED, "Failed with null audioStream_");

    return audioStream_->UpdatePlaybackCaptureConfig(config);
#else
    AUDIO_WARNING_LOG("Inner capture is not supported.");
    return ERR_NOT_SUPPORTED;
#endif
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
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    return currentStream->GetFrameCount(frameCount);
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
    AUDIO_INFO_LOG("enter");
    std::shared_lock<std::shared_mutex> lockShared;
    if (callbackLoopTid_ != gettid()) { // No need to add lock in callback thread to prevent deadlocks
        lockShared = std::shared_lock<std::shared_mutex>(capturerMutex_);
    }
    AudioStreamParams audioStreamParams = ConvertToAudioStreamParams(params);
    IAudioStream::StreamClass streamClass = SetCaptureInfo(audioStreamParams);

    // Create Client
    std::shared_ptr<AudioStreamDescriptor> streamDesc = ConvertToStreamDescriptor(audioStreamParams);
    streamDesc->preferredInputDevice = AudioDeviceDescriptor(params.preferredInputDevice);
    int32_t ret = IAudioStream::CheckCapturerAudioStreamInfo(audioStreamParams);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "CheckCapturerAudioStreamInfo fail!");

    uint32_t flag = AUDIO_INPUT_FLAG_NORMAL;
    ret = AudioPolicyManager::GetInstance().CreateCapturerClient(streamDesc, flag, audioStreamParams.originalSessionId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "CreateCapturerClient failed");
    HILOG_COMM_INFO("StreamClientState for Capturer::CreateClient. id %{public}u, flag :%{public}u",
        audioStreamParams.originalSessionId, flag);

    streamClass = DecideStreamClassAndUpdateCapturerInfo(flag);
    // check AudioStreamParams for fast stream
    if (audioStream_ == nullptr) {
        audioStream_ = IAudioStream::GetRecordStream(streamClass, audioStreamParams, audioStreamType_,
            appInfo_.appUid);
        CHECK_AND_RETURN_RET_LOG(audioStream_ != nullptr, ERR_INVALID_PARAM, "SetParams GetRecordStream faied.");
        AUDIO_INFO_LOG("IAudioStream::GetStream success");
    }
    ret = InitAudioStream(audioStreamParams);
    if (ret != SUCCESS) {
        // if the normal stream creation fails, return fail, other try create normal stream
        CHECK_AND_RETURN_RET_LOG(streamClass != IAudioStream::PA_STREAM, ret, "Normal Stream Init Failed");
        ret = HandleCreateFastStreamError(audioStreamParams);
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
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_CLIENT_PARA, dumpFileName, &dumpFile_);

    ret = InitInputDeviceChangeCallback();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Init input device change callback failed");

    return InitAudioInterruptCallback();
}

IAudioStream::StreamClass AudioCapturerPrivate::SetCaptureInfo(AudioStreamParams &audioStreamParams)
{
    IAudioStream::StreamClass streamClass = IAudioStream::PA_STREAM;
    if (capturerInfo_.sourceType != SOURCE_TYPE_PLAYBACK_CAPTURE) {
#ifdef SUPPORT_LOW_LATENCY
        streamClass = GetPreferredStreamClass(audioStreamParams);
#else
        capturerInfo_.originalFlag = AUDIO_FLAG_FORCED_NORMAL;
        capturerInfo_.capturerFlags = AUDIO_FLAG_NORMAL;
        streamClass = IAudioStream::PA_STREAM;
#endif
    }
    return streamClass;
}

std::shared_ptr<AudioStreamDescriptor> AudioCapturerPrivate::ConvertToStreamDescriptor(
    const AudioStreamParams &audioStreamParams)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->streamInfo_.format = static_cast<AudioSampleFormat>(audioStreamParams.format);
    streamDesc->streamInfo_.samplingRate = static_cast<AudioSamplingRate>(audioStreamParams.samplingRate);
    streamDesc->streamInfo_.channels = static_cast<AudioChannel>(audioStreamParams.channels);
    streamDesc->streamInfo_.encoding = static_cast<AudioEncodingType>(audioStreamParams.encoding);
    streamDesc->streamInfo_.channelLayout = static_cast<AudioChannelLayout>(audioStreamParams.channelLayout);
    streamDesc->audioMode_ = AUDIO_MODE_RECORD;
    streamDesc->createTimeStamp_ = ClockTime::GetCurNano();
    streamDesc->capturerInfo_ = capturerInfo_;
    streamDesc->appInfo_ = appInfo_;
    streamDesc->callerUid_ = static_cast<int32_t>(getuid());
    streamDesc->callerPid_ = static_cast<int32_t>(getpid());
    streamDesc->sessionId_ = audioStreamParams.originalSessionId;
    return streamDesc;
}

IAudioStream::StreamClass AudioCapturerPrivate::DecideStreamClassAndUpdateCapturerInfo(uint32_t flag)
{
    IAudioStream::StreamClass ret = IAudioStream::StreamClass::PA_STREAM;
    if (flag & AUDIO_INPUT_FLAG_FAST) {
        if (flag & AUDIO_INPUT_FLAG_VOIP) {
            capturerInfo_.originalFlag = AUDIO_FLAG_VOIP_FAST;
            capturerInfo_.capturerFlags = AUDIO_FLAG_VOIP_FAST;
            capturerInfo_.pipeType = PIPE_TYPE_CALL_IN;
            ret = IAudioStream::StreamClass::VOIP_STREAM;
        } else {
            capturerInfo_.capturerFlags = AUDIO_FLAG_MMAP;
            capturerInfo_.pipeType = PIPE_TYPE_LOWLATENCY_IN;
            ret = IAudioStream::StreamClass::FAST_STREAM;
        }
    } else {
        capturerInfo_.capturerFlags = AUDIO_FLAG_NORMAL;
        capturerInfo_.pipeType = PIPE_TYPE_NORMAL_IN;
    }
    AUDIO_INFO_LOG("Route flag: %{public}u, streamClass: %{public}d, capturerFlags: %{public}d, pipeType: %{public}d",
        flag, ret, capturerInfo_.capturerFlags, capturerInfo_.pipeType);
    return ret;
}

int32_t AudioCapturerPrivate::InitInputDeviceChangeCallback()
{
    CHECK_AND_RETURN_RET_LOG(GetCurrentInputDevicesInner(currentDeviceInfo_) == SUCCESS, ERROR,
        "Get current device info failed");

    if (!inputDeviceChangeCallback_) {
        inputDeviceChangeCallback_ = std::make_shared<InputDeviceChangeWithInfoCallbackImpl>();
        CHECK_AND_RETURN_RET_LOG(inputDeviceChangeCallback_ != nullptr, ERROR, "Memory allocation failed");
    }

    inputDeviceChangeCallback_->SetAudioCapturerObj(weak_from_this());

    uint32_t sessionId;
    int32_t ret = GetAudioStreamId(sessionId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Get sessionId failed");

    ret = AudioPolicyManager::GetInstance().RegisterDeviceChangeWithInfoCallback(sessionId,
        inputDeviceChangeCallback_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Register failed");

    return SUCCESS;
}

int32_t AudioCapturerPrivate::SetInputDevice(DeviceType deviceType) const
{
    AUDIO_INFO_LOG("AudioCapturerPrivate::SetInputDevice %{public}d", deviceType);
    if (audioStream_ == NULL) {
        return SUCCESS;
    }
    uint32_t currentSessionID = 0;
    audioStream_->GetAudioSessionID(currentSessionID);
    int32_t ret = AudioPolicyManager::GetInstance().SetInputDevice(deviceType, currentSessionID,
        capturerInfo_.sourceType, GetStatus() == CAPTURER_RUNNING);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "select input device failed");
    return SUCCESS;
}

FastStatus AudioCapturerPrivate::GetFastStatus()
{
    std::unique_lock<std::shared_mutex> lock(capturerMutex_, std::defer_lock);
    if (callbackLoopTid_ != gettid()) {
        lock.lock();
    }

    return GetFastStatusInner();
}

FastStatus AudioCapturerPrivate::GetFastStatusInner()
{
    // inner function. Must be called with AudioCapturerPrivate::capturerMutex_ held.
    CHECK_AND_RETURN_RET_LOG(audioStream_ != nullptr, FASTSTATUS_INVALID, "audioStream_ is nullptr");
    return audioStream_->GetFastStatus();
}

int32_t AudioCapturerPrivate::InitAudioStream(const AudioStreamParams &audioStreamParams)
{
    Trace trace("AudioCapturer::InitAudioStream");
    capturerProxyObj_->SaveCapturerObj(weak_from_this());

    audioStream_->SetCapturerInfo(capturerInfo_);

    audioStream_->SetClientID(appInfo_.appPid, appInfo_.appUid, appInfo_.appTokenId, appInfo_.appFullTokenId);

    if (capturerInfo_.sourceType == SOURCE_TYPE_PLAYBACK_CAPTURE) {
        audioStream_->SetInnerCapturerState(true);
    } else if (capturerInfo_.sourceType == SourceType::SOURCE_TYPE_WAKEUP) {
        audioStream_->SetWakeupCapturerState(true);
    }

    audioStream_->SetCapturerSource(capturerInfo_.sourceType);
    int32_t ret = audioStream_->SetAudioStreamInfo(audioStreamParams, capturerProxyObj_, filterConfig_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetAudioStreamInfo failed");
    // for inner-capturer
    if (capturerInfo_.sourceType == SOURCE_TYPE_PLAYBACK_CAPTURE) {
        ret = UpdatePlaybackCaptureConfig(filterConfig_);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "UpdatePlaybackCaptureConfig Failed");
    }
    InitLatencyMeasurement(audioStreamParams);
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
        std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
        CHECK_AND_RETURN_LOG(currentStream != nullptr, "audioStream_ is nullptr");
        if (capturerInfo_.capturerFlags == IAudioStream::FAST_STREAM) {
            AUDIO_INFO_LOG("LatencyMeas fast capturer signal detected");
        } else {
            AUDIO_INFO_LOG("LatencyMeas normal capturer signal detected");
        }
        currentStream->UpdateLatencyTimestamp(signalDetectAgent_->lastPeakBufferTime_, false);
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
    if (audioInterrupt_.streamId != 0) {
        AUDIO_INFO_LOG("old session already has interrupt, need to reset");
        (void)AudioPolicyManager::GetInstance().DeactivateAudioInterrupt(audioInterrupt_);
        (void)AudioPolicyManager::GetInstance().UnsetAudioInterruptCallback(audioInterrupt_.streamId);
    }

    if (audioStream_->GetAudioSessionID(sessionID_) != 0) {
        AUDIO_ERR_LOG("GetAudioSessionID failed for INDEPENDENT_MODE");
        return ERR_INVALID_INDEX;
    }
    audioInterrupt_.streamId = sessionID_;
    audioInterrupt_.pid = appInfo_.appPid;
    audioInterrupt_.uid = appInfo_.appUid;
    audioInterrupt_.audioFocusType.sourceType = capturerInfo_.sourceType;
    audioInterrupt_.sessionStrategy = strategy_;
    audioInterrupt_.bundleName = AudioSystemManager::GetInstance()->GetSelfBundleName(appInfo_.appUid);
    if (audioInterrupt_.bundleName.empty()) {
        audioInterrupt_.bundleName = AudioSystemManager::GetInstance()->GetSelfBundleName();
    }
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
        audioStreamCallback_ = std::make_shared<AudioStreamCallbackCapturer>(weak_from_this());
        CHECK_AND_RETURN_RET_LOG(audioStreamCallback_ != nullptr, ERROR,
            "Failed to allocate memory for audioStreamCallback_");
    }
    std::shared_ptr<AudioStreamCallbackCapturer> cbStream =
        std::static_pointer_cast<AudioStreamCallbackCapturer>(audioStreamCallback_);
    cbStream->SaveCallback(callback);
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    (void)currentStream->SetStreamCallback(audioStreamCallback_);

    return SUCCESS;
}

void AudioCapturerPrivate::SetAudioCapturerErrorCallback(std::shared_ptr<AudioCapturerErrorCallback> errorCallback)
{
    std::shared_lock<std::shared_mutex> sharedLock;
    if (callbackLoopTid_ != gettid()) { // No need to add lock in callback thread to prevent deadlocks
        sharedLock = std::shared_lock<std::shared_mutex>(capturerMutex_);
    }
    std::lock_guard lock(audioCapturerErrCallbackMutex_);
    audioCapturerErrorCallback_ = errorCallback;
}

int32_t AudioCapturerPrivate::RegisterAudioPolicyServerDiedCb(const int32_t clientPid,
    const std::shared_ptr<AudioCapturerPolicyServiceDiedCallback> &callback)
{
    AUDIO_INFO_LOG("RegisterAudioPolicyServerDiedCb client id: %{public}d", clientPid);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is null");

    std::lock_guard<std::mutex> lock(policyServiceDiedCallbackMutex_);

    policyServiceDiedCallback_ = callback;
    return AudioPolicyManager::GetInstance().RegisterAudioPolicyServerDiedCb(clientPid, callback);
}

void AudioCapturerPrivate::SetFastStatusChangeCallback(
    const std::shared_ptr<AudioCapturerFastStatusChangeCallback> &callback)
{
    std::lock_guard lock(fastStatusChangeCallbackMutex_);
    fastStatusChangeCallback_ = callback;
}

int32_t AudioCapturerPrivate::GetParams(AudioCapturerParams &params) const
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    AudioStreamParams audioStreamParams;
    int32_t result = currentStream->GetAudioStreamInfo(audioStreamParams);
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
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    AudioStreamParams audioStreamParams;
    int32_t result = currentStream->GetAudioStreamInfo(audioStreamParams);
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
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    currentStream->SetCapturerPositionCallback(markPosition, callback);

    return SUCCESS;
}

void AudioCapturerPrivate::UnsetCapturerPositionCallback()
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_LOG(currentStream != nullptr, "audioStream_ is nullptr");
    currentStream->UnsetCapturerPositionCallback();
}

int32_t AudioCapturerPrivate::SetCapturerPeriodPositionCallback(int64_t frameNumber,
    const std::shared_ptr<CapturerPeriodPositionCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG((callback != nullptr) && (frameNumber > 0), ERR_INVALID_PARAM,
        "input param is invalid");
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    currentStream->SetCapturerPeriodPositionCallback(frameNumber, callback);

    return SUCCESS;
}

void AudioCapturerPrivate::UnsetCapturerPeriodPositionCallback()
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_LOG(currentStream != nullptr, "audioStream_ is nullptr");
    currentStream->UnsetCapturerPeriodPositionCallback();
}

int32_t AudioCapturerPrivate::CheckAndRestoreAudioCapturer(std::string callingFunc)
{
    std::unique_lock<std::shared_mutex> lock;
    if (callbackLoopTid_ != gettid()) { // No need to add lock in callback thread to prevent deadlocks
        lock = std::unique_lock<std::shared_mutex>(capturerMutex_);
    }

    if (abortRestore_) {
        return SUCCESS;
    }
    // Return in advance if there's no need for restore.
    CHECK_AND_RETURN_RET_LOG(audioStream_, ERR_ILLEGAL_STATE, "audioStream_ is nullptr");
    RestoreStatus restoreStatus = audioStream_->CheckRestoreStatus();
    if (restoreStatus == NO_NEED_FOR_RESTORE) {
        return SUCCESS;
    }
    if (restoreStatus == RESTORING) {
        AUDIO_WARNING_LOG("%{public}s when restoring, return", callingFunc.c_str());
        return ERR_ILLEGAL_STATE;
    }

    // Get restore info and target stream class for switching.
    RestoreInfo restoreInfo;
    audioStream_->GetRestoreInfo(restoreInfo);
    IAudioStream::StreamClass targetClass = DecideStreamClassAndUpdateCapturerInfo(restoreInfo.routeFlag);
    if (restoreStatus == NEED_RESTORE_TO_NORMAL) {
        restoreInfo.targetStreamFlag = AUDIO_FLAG_FORCED_NORMAL;
    }

    // Block interrupt calback, avoid pausing wrong stream.
    std::shared_ptr<AudioCapturerInterruptCallbackImpl> interruptCbImpl = nullptr;
    if (audioInterruptCallback_ != nullptr) {
        interruptCbImpl = std::static_pointer_cast<AudioCapturerInterruptCallbackImpl>(audioInterruptCallback_);
        interruptCbImpl->StartSwitch();
    }

    FastStatus fastStatus = GetFastStatusInner();
    // Switch to target audio stream. Deactivate audio interrupt if switch failed.
    AUDIO_INFO_LOG("Before %{public}s, restore audio capturer %{public}u", callingFunc.c_str(), sessionID_);
    if (!SwitchToTargetStream(targetClass, restoreInfo)) {
        AudioInterrupt audioInterrupt = audioInterrupt_;
        int32_t ret = AudioPolicyManager::GetInstance().DeactivateAudioInterrupt(audioInterrupt);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "DeactivateAudioInterrupt Failed");
    } else {
        FastStatusChangeCallback(fastStatus);
    }

    // Unblock interrupt callback.
    if (interruptCbImpl) {
        interruptCbImpl->FinishSwitch();
    }
    return SUCCESS;
}

bool AudioCapturerPrivate::IsRestoreOrStopNeeded()
{
    std::unique_lock<std::shared_mutex> lock;
    if (callbackLoopTid_ != gettid()) { // No need to add lock in callback thread to prevent deadlocks
        lock = std::unique_lock<std::shared_mutex>(capturerMutex_);
    }
    CHECK_AND_RETURN_RET_LOG(audioStream_ != nullptr, false, "audio stream is null");
    return audioStream_->IsRestoreNeeded() || audioStream_->GetStopFlag();
}

void AudioCapturerPrivate::SetInSwitchingFlag(bool inSwitchingFlag)
{
    std::unique_lock<std::mutex> lock(inSwitchingMtx_);
    inSwitchingFlag_ = inSwitchingFlag;
    if (!inSwitchingFlag_) {
        taskLoopCv_.notify_all();
    }
}

int32_t AudioCapturerPrivate::AsyncCheckAudioCapturer(std::string callingFunc)
{
    if (switchStreamInNewThreadTaskCount_.fetch_add(1) > 0) {
        return SUCCESS;
    }
    auto weakCapturer = weak_from_this();
    taskLoop_.PostTask([weakCapturer, callingFunc] () {
        auto sharedCapturer = weakCapturer.lock();
        CHECK_AND_RETURN_LOG(sharedCapturer, "capturer is null");
        uint32_t taskCount;
        do {
            taskCount = sharedCapturer->switchStreamInNewThreadTaskCount_.load();
            sharedCapturer->SetInSwitchingFlag(true);
            sharedCapturer->CheckAudioCapturer(callingFunc + "withNewThread");
            sharedCapturer->SetInSwitchingFlag(false);
        } while (sharedCapturer->switchStreamInNewThreadTaskCount_.fetch_sub(taskCount) > taskCount);
    });
    return SUCCESS;
}

bool AudioCapturerPrivate::Start()
{
    AsyncCheckAudioCapturer("Start");
    std::unique_lock<std::shared_mutex> lock;
    if (callbackLoopTid_ != gettid()) { // No need to add lock in callback thread to prevent deadlocks
        lock = std::unique_lock<std::shared_mutex>(capturerMutex_);
    }
    Trace trace("KeyAction AudioCapturer::Start " + std::to_string(sessionID_));
    HILOG_COMM_INFO("StreamClientState for Capturer::Start. id %{public}u, sourceType: %{public}d",
        sessionID_, audioInterrupt_.audioFocusType.sourceType);

    CapturerState state = GetStatusInner();
    CHECK_AND_RETURN_RET_LOG((state == CAPTURER_PREPARED) || (state == CAPTURER_STOPPED) || (state == CAPTURER_PAUSED),
        false, "Start failed. Illegal state %{public}u.", state);

    CHECK_AND_RETURN_RET_LOG(!isSwitching_, false, "Operation failed, in switching");

    CHECK_AND_RETURN_RET(audioInterrupt_.audioFocusType.sourceType != SOURCE_TYPE_INVALID &&
        audioInterrupt_.streamId != INVALID_SESSION_ID, false);
    std::unique_lock<std::mutex> audioInterruptLock(audioInterruptMutex_);
    AudioInterrupt audioInterrupt = audioInterrupt_;
    audioInterruptLock.unlock();
    int32_t ret = AudioPolicyManager::GetInstance().ActivateAudioInterrupt(audioInterrupt);
    CHECK_AND_RETURN_RET_LOG(ret == 0, false, "ActivateAudioInterrupt Failed");

    // When the cellular call stream is starting, only need to activate audio interrupt.
    CHECK_AND_RETURN_RET(!isVoiceCallCapturer_, true);
    CHECK_AND_RETURN_RET(audioStream_ != nullptr, false, "audioStream_ is null");
    if (state == CAPTURER_STOPPED && getuid() == UID_MEDIA_SA) {
        AUDIO_INFO_LOG("Media SA is startting, flush data.");
        audioStream_->FlushAudioStream();
    }
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

int32_t AudioCapturerPrivate::Read(uint8_t &buffer, size_t userSize, bool isBlockingRead)
{
    Trace trace("AudioCapturer::Read");
    CheckSignalData(&buffer, userSize);
    AsyncCheckAudioCapturer("Read");

    std::unique_lock<std::mutex> lock(inSwitchingMtx_);
    taskLoopCv_.wait_for(lock, std::chrono::milliseconds(BLOCK_INTERRUPT_OVERTIMES_IN_MS), [this] {
        return inSwitchingFlag_ == false;
    });
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    int size = currentStream->Read(buffer, userSize, isBlockingRead);
    if (size > 0) {
        DumpFileUtil::WriteDumpFile(dumpFile_, static_cast<void *>(&buffer), size);
    }
    return size;
}

CapturerState AudioCapturerPrivate::GetStatus() const
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, CAPTURER_INVALID, "audioStream_ is nullptr");
    return static_cast<CapturerState>(currentStream->GetState());
}

bool AudioCapturerPrivate::GetAudioTime(Timestamp &timestamp, Timestamp::Timestampbase base) const
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, false, "audioStream_ is nullptr");
    return currentStream->GetAudioTime(timestamp, base);
}

bool AudioCapturerPrivate::GetFirstPkgTimeStampInfo(int64_t &firstTs) const
{
    AUDIO_INFO_LOG("StreamClient for Capturer::Get first pkg timestamp info.");
    const std::string subKeyLatency = "record_algo_first_ts";
    std::vector<std::pair<std::string, std::string>> results;

    int32_t ret =
        AudioSystemManager::GetInstance()->GetExtraParameters("audio_effect", { subKeyLatency }, results);
    if (ret != 0) {
        AUDIO_WARNING_LOG("AudioCapturerPrivate GetExtraParameters fail! %{public}d", ret);
        return false;
    }

    auto iter = std::find_if(results.begin(), results.end(),
        [&](const std::pair<std::string, std::string> &result) {
            return result.first == subKeyLatency;
        });
    if (iter == results.end() || iter->second.empty()) {
        AUDIO_WARNING_LOG(
            "AudioCapturerPrivate GetExtraParameters fail! cannot find result or is empty string");
        return false;
    }

    firstTs = std::strtol(iter->second.c_str(), nullptr, DECIMAL_BASE);
    AUDIO_INFO_LOG("StreamClient for Capturer::first ts is raw %{public}s and value %{public}lld",
        iter->second.c_str(), static_cast<long long>(firstTs));
    return true;
}

bool AudioCapturerPrivate::Pause() const
{
    std::unique_lock<std::shared_mutex> lock;
    if (callbackLoopTid_ != gettid()) { // No need to add lock in callback thread to prevent deadlocks
        lock = std::unique_lock<std::shared_mutex>(capturerMutex_);
    }
    Trace trace("KeyAction AudioCapturer::Pause " + std::to_string(sessionID_));
    HILOG_COMM_INFO("StreamClientState for Capturer::Pause. id %{public}u", sessionID_);
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
    std::unique_lock<std::shared_mutex> lock;
    if (callbackLoopTid_ != gettid()) { // No need to add lock in callback thread to prevent deadlocks
        lock = std::unique_lock<std::shared_mutex>(capturerMutex_);
    }
    Trace trace("KeyAction AudioCapturer::Stop " + std::to_string(sessionID_));
    HILOG_COMM_INFO("StreamClientState for Capturer::Stop. id %{public}u", sessionID_);
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
    Trace trace("KeyAction AudioCapturer::Flush " + std::to_string(sessionID_));
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    AUDIO_INFO_LOG("StreamClientState for Capturer::Flush. id %{public}u", sessionID_);
    return currentStream->FlushAudioStream();
}

bool AudioCapturerPrivate::Release()
{
    Trace trace("KeyAction AudioCapturer::Release" + std::to_string(sessionID_));
    HILOG_COMM_INFO("StreamClientState for Capturer::Release. id %{public}u", sessionID_);
    std::unique_lock<std::shared_mutex> releaseLock;
    if (callbackLoopTid_ != gettid()) { // No need to add lock in callback thread to prevent deadlocks
        releaseLock = std::unique_lock<std::shared_mutex>(capturerMutex_);
    }
    abortRestore_ = true;
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(isValid_, false, "Release when capturer invalid");

    CHECK_AND_RETURN_RET_LOG(audioStream_ != nullptr, false, "audioStream_ is nullptr");
    audioInterrupt_.state = State::RELEASED;
    (void)AudioPolicyManager::GetInstance().DeactivateAudioInterrupt(audioInterrupt_);

    // Unregister the callaback in policy server
    (void)AudioPolicyManager::GetInstance().UnsetAudioInterruptCallback(sessionID_);

    RemoveCapturerPolicyServiceDiedCallback();

    return audioStream_->ReleaseAudioStream();
}

int32_t AudioCapturerPrivate::GetBufferSize(size_t &bufferSize) const
{
    Trace trace("AudioCapturer::GetBufferSize");
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    return currentStream->GetBufferSize(bufferSize);
}

int32_t AudioCapturerPrivate::GetAudioStreamId(uint32_t &sessionID) const
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERR_INVALID_HANDLE, "GetAudioStreamId faied.");
    return currentStream->GetAudioSessionID(sessionID);
}

int32_t AudioCapturerPrivate::SetBufferDuration(uint64_t bufferDuration) const
{
    CHECK_AND_RETURN_RET_LOG(bufferDuration >= MINIMUM_BUFFER_SIZE_MSEC && bufferDuration <= MAXIMUM_BUFFER_SIZE_MSEC,
        ERR_INVALID_PARAM, "Error: Please set the buffer duration between 5ms ~ 20ms");
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    return currentStream->SetBufferSizeInMsec(bufferDuration);
}

bool AudioCapturerPrivate::GetTimeStampInfo(Timestamp &timestamp, Timestamp::Timestampbase base) const
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, false, "audioStream_ is nullptr");
    return currentStream->GetTimeStampInfo(timestamp, base);
}

// diffrence from GetAudioPosition only when set speed
int32_t AudioCapturerPrivate::GetAudioTimestampInfo(Timestamp &timestamp, Timestamp::Timestampbase base) const
{
    CHECK_AND_RETURN_RET_LOG(audioStream_ != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    return audioStream_->GetAudioTimestampInfo(timestamp, base);
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

void AudioCapturerInterruptCallbackImpl::StartSwitch()
{
    std::lock_guard<std::mutex> lock(mutex_);
    switching_ = true;
    AUDIO_INFO_LOG("SwitchStream start, block interrupt callback");
}

void AudioCapturerInterruptCallbackImpl::FinishSwitch()
{
    std::lock_guard<std::mutex> lock(mutex_);
    switching_ = false;
    switchStreamCv_.notify_all();
    AUDIO_INFO_LOG("SwitchStream finish, notify interrupt callback");
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

void AudioCapturerInterruptCallbackImpl::OnInterrupt(const InterruptEventInternal &interruptEvent)
{
    std::unique_lock<std::mutex> lock(mutex_);

    if (switching_) {
        AUDIO_INFO_LOG("Wait for SwitchStream");
        bool ret = switchStreamCv_.wait_for(lock, std::chrono::milliseconds(BLOCK_INTERRUPT_CALLBACK_IN_MS),
            [this] {return !switching_;});
        if (!ret) {
            switching_ = false;
            AUDIO_WARNING_LOG("Wait for SwitchStream time out, could handle interrupt event with old stream");
        }
    }
    cb_ = callback_.lock();
    InterruptForceType forceType = interruptEvent.forceType;
    AUDIO_INFO_LOG("InterruptForceType: %{public}d", forceType);
    InterruptEvent event;

    if (forceType == INTERRUPT_SHARE) { // INTERRUPT_SHARE
        AUDIO_DEBUG_LOG("AudioCapturerPrivate ForceType: INTERRUPT_SHARE. Let app handle the event");
        event = InterruptEvent {interruptEvent.eventType, interruptEvent.forceType, interruptEvent.hintType};
    } else {
        CHECK_AND_RETURN_LOG(audioStream_ != nullptr, "Stream is not alive. No need to take forced action");
        State currentState = audioStream_->GetState();

        switch (interruptEvent.hintType) {
            case INTERRUPT_HINT_RESUME:
                CHECK_AND_RETURN_LOG((currentState == PAUSED || currentState == PREPARED) && isForcePaused_ == true,
                    "OnInterrupt state %{public}d or not forced pause %{public}d before", currentState, isForcePaused_);
                AUDIO_INFO_LOG("set force pause false");
                isForcePaused_ = false;
                event = InterruptEvent {interruptEvent.eventType, INTERRUPT_SHARE, interruptEvent.hintType};
                lock.unlock();
                NotifyEvent(event);
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
        event = InterruptEvent {interruptEvent.eventType, interruptEvent.forceType, interruptEvent.hintType};
    }

    lock.unlock();
    NotifyEvent(event);
}

AudioStreamCallbackCapturer::AudioStreamCallbackCapturer(std::weak_ptr<AudioCapturerPrivate> capturer)
    : capturer_(capturer)
{
}

void AudioStreamCallbackCapturer::SaveCallback(const std::weak_ptr<AudioCapturerCallback> &callback)
{
    callback_ = callback;
}

void AudioStreamCallbackCapturer::OnStateChange(const State state,
    const StateChangeCmdType __attribute__((unused)) cmdType)
{
    std::shared_ptr<AudioCapturerPrivate> capturerObj = capturer_.lock();
    CHECK_AND_RETURN_LOG(capturerObj != nullptr, "capturerObj is nullptr");
    std::shared_ptr<AudioCapturerCallback> cb = callback_.lock();
    CHECK_AND_RETURN_LOG(cb != nullptr, "AudioStreamCallbackCapturer::OnStateChange cb == nullptr.");

    auto captureState = static_cast<CapturerState>(state);
    cb->OnStateChange(captureState);

    AudioInterrupt audioInterrupt;
    capturerObj->GetAudioInterrupt(audioInterrupt);
    audioInterrupt.state = state;
    capturerObj->SetAudioInterrupt(audioInterrupt);
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
    std::lock_guard<std::mutex> lock(audioInterruptMutex_);
    if (targetSources.size() <= 0) {
        AUDIO_ERR_LOG("TargetSources size is 0, set audio source concurrency failed.");
        return ERR_INVALID_PARAM;
    }
    AUDIO_INFO_LOG("Set audio source concurrency success.");
    audioInterrupt_.currencySources.sourcesTypes = targetSources;
    return SUCCESS;
}

int32_t AudioCapturerPrivate::SetInterruptStrategy(InterruptStrategy strategy)
{
    CapturerState state = GetStatusInner();
    CHECK_AND_RETURN_RET_LOG(state == CAPTURER_PREPARED, ERR_ILLEGAL_STATE,
        "incorrect state:%{public}d", state);
    audioInterrupt_.strategy = strategy;
    AUDIO_INFO_LOG("set InterruptStrategy to %{public}d", static_cast<int32_t>(strategy));
    return SUCCESS;
}

int32_t AudioCapturerPrivate::SetCaptureMode(AudioCaptureMode captureMode)
{
    AUDIO_INFO_LOG("Capture mode: %{public}d", captureMode);
    audioCaptureMode_ = captureMode;
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    int32_t ret = currentStream->SetCaptureMode(captureMode);
    callbackLoopTid_ = audioStream_->GetCallbackLoopTid();
    return ret;
}

AudioCaptureMode AudioCapturerPrivate::GetCaptureMode() const
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, CAPTURE_MODE_NORMAL, "audioStream_ is nullptr");
    return currentStream->GetCaptureMode();
}

int32_t AudioCapturerPrivate::SetCapturerReadCallback(const std::shared_ptr<AudioCapturerReadCallback> &callback)
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    return currentStream->SetCapturerReadCallback(callback);
}

int32_t AudioCapturerPrivate::GetBufferDesc(BufferDesc &bufDesc)
{
    AsyncCheckAudioCapturer("GetBufferDesc");
    std::shared_ptr<IAudioStream> currentStream = audioStream_;
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    int32_t ret = currentStream->GetBufferDesc(bufDesc);
    DumpFileUtil::WriteDumpFile(dumpFile_, static_cast<void *>(bufDesc.buffer), bufDesc.bufLength);
    return ret;
}

int32_t AudioCapturerPrivate::Enqueue(const BufferDesc &bufDesc)
{
    AsyncCheckAudioCapturer("Enqueue");
    std::shared_ptr<IAudioStream> currentStream = audioStream_;
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    CheckSignalData(bufDesc.buffer, bufDesc.bufLength);
    return currentStream->Enqueue(bufDesc);
}

int32_t AudioCapturerPrivate::Clear() const
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    return currentStream->Clear();
}

int32_t AudioCapturerPrivate::GetBufQueueState(BufferQueueState &bufState) const
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    return currentStream->GetBufQueueState(bufState);
}

void AudioCapturerPrivate::SetValid(bool valid)
{
    std::lock_guard<std::mutex> lock(lock_);
    isValid_ = valid;
}

int64_t AudioCapturerPrivate::GetFramesRead() const
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    return currentStream->GetFramesRead();
}

int32_t AudioCapturerPrivate::GetCurrentInputDevices(AudioDeviceDescriptor &deviceInfo) const
{
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
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
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
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
    return AudioPolicyManager::GetInstance().GetAudioCapturerMicrophoneDescriptors(static_cast<int32_t>(sessionId));
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

    CHECK_AND_RETURN_RET_LOG(GetCurrentInputDevicesInner(deviceInfo) == SUCCESS, deviceUpdated,
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
    std::lock_guard<std::mutex> lock(audioInterruptMutex_);
    audioInterrupt = audioInterrupt_;
}

void AudioCapturerPrivate::SetAudioInterrupt(const AudioInterrupt &audioInterrupt)
{
    std::lock_guard<std::mutex> lock(audioInterruptMutex_);
    audioInterrupt_ = audioInterrupt;
}

// Inner function. Must be called with AudioCapturerPrivate::capturerMutex_ held
void AudioCapturerPrivate::WriteOverflowEvent() const
{
    AUDIO_INFO_LOG("Write overflowEvent to media monitor");
    if (GetOverflowCountInner() < WRITE_OVERFLOW_NUM) {
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
        audioStateChangeCallback_->SetAudioCapturerObj(weak_from_this());
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
        audioPolicyServiceDiedCallback_->SetAudioCapturerObj(weak_from_this());
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
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    return currentStream->GetOverflowCount();
}

int32_t AudioCapturerPrivate::SetSwitchInfo(IAudioStream::SwitchInfo info, std::shared_ptr<IAudioStream> audioStream)
{
    CHECK_AND_RETURN_RET_LOG(audioStream, ERROR, "stream is nullptr");

    audioStream->SetStreamTrackerState(false);
    audioStream->SetClientID(info.clientPid, info.clientUid, appInfo_.appTokenId, appInfo_.appFullTokenId);
    audioStream->SetCapturerInfo(info.capturerInfo);
    int32_t res = audioStream->SetAudioStreamInfo(info.params, capturerProxyObj_);
    CHECK_AND_RETURN_RET_LOG(res == SUCCESS, ERROR, "SetAudioStreamInfo failed");
    audioStream->SetCaptureMode(info.captureMode);
    callbackLoopTid_ = audioStream_->GetCallbackLoopTid();

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
    return SUCCESS;
}

void AudioCapturerPrivate::InitSwitchInfo(IAudioStream::StreamClass targetClass, IAudioStream::SwitchInfo &switchInfo)
{
    audioStream_->GetSwitchInfo(switchInfo);

    if (targetClass == IAudioStream::VOIP_STREAM) {
        switchInfo.capturerInfo.originalFlag = AUDIO_FLAG_VOIP_FAST;
    }
    switchInfo.captureMode = audioCaptureMode_;
    switchInfo.params.originalSessionId = sessionID_;
    return;
}

bool AudioCapturerPrivate::FinishOldStream(IAudioStream::StreamClass targetClass, RestoreInfo restoreInfo,
    CapturerState previousState, IAudioStream::SwitchInfo &switchInfo)
{
    bool switchResult = false;
    if (previousState == CAPTURER_RUNNING) {
        // stop old stream
        switchResult = audioStream_->StopAudioStream();
        if (restoreInfo.restoreReason != SERVER_DIED) {
            JUDGE_AND_ERR_LOG(!switchResult, "StopAudioStream failed.");
        }
    }
    // switch new stream
    InitSwitchInfo(targetClass, switchInfo);
    if (restoreInfo.restoreReason == SERVER_DIED) {
        AUDIO_INFO_LOG("Server died, reset session id: %{public}d", switchInfo.params.originalSessionId);
        switchInfo.params.originalSessionId = 0;
        switchInfo.sessionId = 0;
    }

    // release old stream and restart audio stream
    switchResult = audioStream_->ReleaseAudioStream(true, true);
    if (restoreInfo.restoreReason != SERVER_DIED) {
        CHECK_AND_RETURN_RET_LOG(switchResult, false, "release old stream failed.");
    }
    return true;
}

bool AudioCapturerPrivate::GenerateNewStream(IAudioStream::StreamClass targetClass, RestoreInfo restoreInfo,
    CapturerState previousState, IAudioStream::SwitchInfo &switchInfo)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = GenerateStreamDesc(switchInfo, restoreInfo);

    int32_t ret = IAudioStream::CheckCapturerAudioStreamInfo(switchInfo.params);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "CheckCapturerAudioStreamInfo fail!");
    uint32_t flag = AUDIO_INPUT_FLAG_NORMAL;
    ret = AudioPolicyManager::GetInstance().CreateCapturerClient(
        streamDesc, flag, switchInfo.params.originalSessionId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, false, "CreateCapturerClient failed");

    bool switchResult = false;
    targetClass = DecideStreamClassAndUpdateCapturerInfo(flag);
    std::shared_ptr<IAudioStream> newAudioStream = IAudioStream::GetRecordStream(targetClass, switchInfo.params,
        switchInfo.eStreamType, appInfo_.appUid);
    CHECK_AND_RETURN_RET_LOG(newAudioStream != nullptr, false, "GetRecordStream failed.");
    AUDIO_INFO_LOG("Get new stream success!");

    // set new stream info
    int32_t initResult = SetSwitchInfo(switchInfo, newAudioStream);
    if (initResult != SUCCESS && switchInfo.capturerInfo.originalFlag != AUDIO_FLAG_NORMAL) {
        AUDIO_ERR_LOG("Re-create stream failed, crate normal ipc stream");
        if (restoreInfo.restoreReason == SERVER_DIED) {
            switchInfo.sessionId = switchInfo.params.originalSessionId;
            streamDesc->sessionId_ = switchInfo.params.originalSessionId;
        }
        streamDesc->capturerInfo_.capturerFlags = AUDIO_FLAG_FORCED_NORMAL;
        streamDesc->routeFlag_ = AUDIO_FLAG_NONE;
        int32_t ret = AudioPolicyManager::GetInstance().CreateCapturerClient(
            streamDesc, flag, switchInfo.params.originalSessionId);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, false, "CreateRendererClient failed");

        newAudioStream = IAudioStream::GetRecordStream(IAudioStream::PA_STREAM, switchInfo.params,
            switchInfo.eStreamType, appInfo_.appUid);
        CHECK_AND_RETURN_RET_LOG(newAudioStream != nullptr, false, "Get ipc stream failed");
        initResult = SetSwitchInfo(switchInfo, newAudioStream);
        CHECK_AND_RETURN_RET_LOG(initResult == SUCCESS, false, "Init ipc strean failed");
    }

    std::shared_ptr<IAudioStream> oldAudioStream = audioStream_;
    // Operation of replace audioStream_ must be performed before StartAudioStream.
    // Otherwise GetBufferDesc will return the buffer pointer of oldStream (causing Use-After-Free).
    audioStream_ = newAudioStream;
    if (capturerInfo_.sourceType == SOURCE_TYPE_PLAYBACK_CAPTURE) {
        auto ret = UpdatePlaybackCaptureConfig(filterConfig_);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, false, "UpdatePlaybackCaptureConfig Failed!");
    }
    if (audioInterruptCallback_ != nullptr) {
        std::shared_ptr<AudioCapturerInterruptCallbackImpl> interruptCbImpl =
            std::static_pointer_cast<AudioCapturerInterruptCallbackImpl>(audioInterruptCallback_);
        interruptCbImpl->UpdateAudioStream(audioStream_);
    }

    if (previousState == CAPTURER_RUNNING) {
        // restart audio stream
        newAudioStream->SetRebuildFlag();
        switchResult = newAudioStream->StartAudioStream();
        CHECK_AND_RETURN_RET_LOG(switchResult, false, "start new stream failed.");
    }
    return true;
}

bool AudioCapturerPrivate::ContinueAfterSplit(RestoreInfo restoreInfo)
{
    CHECK_AND_RETURN_RET(restoreInfo.restoreReason == STREAM_SPLIT, true);
    audioStream_->FetchDeviceForSplitStream();
    return false;
}

bool AudioCapturerPrivate::SwitchToTargetStream(IAudioStream::StreamClass targetClass, RestoreInfo restoreInfo)
{
    bool switchResult = false;

    Trace trace("KeyAction AudioCapturer::SwitchToTargetStream " + std::to_string(sessionID_)
        + ", target class " + std::to_string(targetClass) + ", reason " + std::to_string(restoreInfo.restoreReason)
        + ", device change reason " + std::to_string(restoreInfo.deviceChangeReason)
        + ", target flag " + std::to_string(restoreInfo.targetStreamFlag));
    AUDIO_INFO_LOG("Restore AudioCapturer %{public}u, target class %{public}d, reason: %{public}d, "
        "device change reason %{public}d, target flag %{public}d", sessionID_, targetClass,
        restoreInfo.restoreReason, restoreInfo.deviceChangeReason, restoreInfo.targetStreamFlag);

    // Check if continue to switch after some concede operation.
    CHECK_AND_RETURN_RET(ContinueAfterSplit(restoreInfo), true, "Stream split");

    isSwitching_ = true;
    CapturerState previousState = GetStatusInner();
    IAudioStream::SwitchInfo switchInfo;

    // Stop old stream, get stream info and frames written for new stream, and release old stream.
    switchResult = FinishOldStream(targetClass, restoreInfo, previousState, switchInfo);
    CHECK_AND_RETURN_RET_LOG(switchResult, false, "Finish old stream failed");

    // Create and start new stream.
    switchResult = GenerateNewStream(targetClass, restoreInfo, previousState, switchInfo);
    CHECK_AND_RETURN_RET_LOG(switchResult, false, "Generate new stream failed");

    // Activate audio interrupt again when restoring for audio server died.
    if (restoreInfo.restoreReason == SERVER_DIED) {
        HandleAudioInterruptWhenServerDied();
    }

    isSwitching_ = false;
    switchResult = true;

    return switchResult;
}

void AudioCapturerPrivate::HandleAudioInterruptWhenServerDied()
{
    if (GetStatusInner() == CAPTURER_RUNNING) {
        AudioInterrupt audioInterrupt = audioInterrupt_;
        int32_t ret = AudioPolicyManager::GetInstance().ActivateAudioInterrupt(audioInterrupt);
        if (ret != SUCCESS) {
            AUDIO_WARNING_LOG("Activate audio interrupt failed when restoring from server died");
        }
    }
}

void AudioCapturerPrivate::FastStatusChangeCallback(FastStatus status)
{
    FastStatus newStatus = GetFastStatusInner();
    if (newStatus != status) {
        if (fastStatusChangeCallback_ != nullptr) {
            fastStatusChangeCallback_->OnFastStatusChange(newStatus);
        }
    }
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
    std::lock_guard<std::mutex> lock(deviceChangeCallbackMutex_);
    auto iter = find(deviceChangeCallbacklist_.begin(), deviceChangeCallbacklist_.end(), callback);
    if (iter == deviceChangeCallbacklist_.end()) {
        deviceChangeCallbacklist_.emplace_back(callback);
    }
}

void AudioCapturerStateChangeCallbackImpl::RemoveDeviceChangeCallback(
    const std::shared_ptr<AudioCapturerDeviceChangeCallback> &callback)
{
    std::lock_guard<std::mutex> lock(deviceChangeCallbackMutex_);
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
    std::lock_guard<std::mutex> lock(deviceChangeCallbackMutex_);
    return deviceChangeCallbacklist_.size();
}

void AudioCapturerStateChangeCallbackImpl::SetAudioCapturerObj(
    std::weak_ptr<AudioCapturerPrivate> capturerObj)
{
    std::lock_guard<std::mutex> lock(capturerMutex_);
    capturer_ = capturerObj;
}

void AudioCapturerStateChangeCallbackImpl::NotifyAudioCapturerInfoChange(
    const std::vector<std::shared_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos)
{
    uint32_t sessionId = static_cast<uint32_t>(-1);
    bool found = false;
    AudioCapturerChangeInfo capturerChangeInfo;
    std::vector<std::shared_ptr<AudioCapturerInfoChangeCallback>> capturerInfoChangeCallbacklist;

    {
        std::unique_lock lock(capturerMutex_);
        auto sharedCapturer = capturer_.lock();
        lock.unlock();
        CHECK_AND_RETURN_LOG(sharedCapturer != nullptr, "sharedCapturer is nullptr");
        int32_t ret = sharedCapturer->GetAudioStreamId(sessionId);
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
    const std::vector<std::shared_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos)
{
    std::vector<std::shared_ptr<AudioCapturerDeviceChangeCallback>> deviceChangeCallbacklist;
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    {
        std::unique_lock lock(capturerMutex_);
        auto sharedCapturer = capturer_.lock();
        lock.unlock();
        CHECK_AND_RETURN_LOG(sharedCapturer != nullptr, "sharedCapturer is nullptr");
        CHECK_AND_RETURN_LOG(sharedCapturer->IsDeviceChanged(deviceInfo), "Device not change, no need callback.");
    }

    {
        std::lock_guard<std::mutex> lock(deviceChangeCallbackMutex_);
        deviceChangeCallbacklist = deviceChangeCallbacklist_;
    }
    for (auto it = deviceChangeCallbacklist.begin(); it != deviceChangeCallbacklist.end(); ++it) {
        if (*it != nullptr) {
            (*it)->OnStateChange(deviceInfo);
        }
    }
}

void AudioCapturerStateChangeCallbackImpl::OnCapturerStateChange(
    const std::vector<std::shared_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos)
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
    capturer_.reset();
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
}

CapturerPolicyServiceDiedCallback::CapturerPolicyServiceDiedCallback()
{
    AUDIO_DEBUG_LOG("CapturerPolicyServiceDiedCallback create");
}

CapturerPolicyServiceDiedCallback::~CapturerPolicyServiceDiedCallback()
{
    AUDIO_DEBUG_LOG("CapturerPolicyServiceDiedCallback destroy");
}

void CapturerPolicyServiceDiedCallback::SetAudioCapturerObj(
    std::weak_ptr<AudioCapturerPrivate> capturerObj)
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
    if (taskCount_.fetch_add(1) > 0) {
        AUDIO_INFO_LOG("direct ret");
        return;
    }

    auto weakRefCb = weak_from_this();
    std::thread restoreThread ([weakRefCb] {
        auto strongRefCb = weakRefCb.lock();
        CHECK_AND_RETURN_LOG(strongRefCb != nullptr, "strongRef is nullptr");
        int32_t count;
        do {
            count = strongRefCb->taskCount_.load();
            strongRefCb->RestoreTheadLoop();
        } while (strongRefCb->taskCount_.fetch_sub(count) > count);
    });
    pthread_setname_np(restoreThread.native_handle(), "OS_ACPSRestore");
    restoreThread.detach();
}

void CapturerPolicyServiceDiedCallback::RestoreTheadLoop()
{
    int32_t tryCounter = 10;
    uint32_t sleepTime = 300000;
    bool restoreResult = false;
    while (!restoreResult && tryCounter > 0) {
        tryCounter--;
        usleep(sleepTime);
        auto sharedCapturer = capturer_.lock();
        CHECK_AND_RETURN_LOG(sharedCapturer != nullptr, "sharedRenderer is nullptr");
        if (sharedCapturer->audioStream_ == nullptr || sharedCapturer->abortRestore_) {
            AUDIO_INFO_LOG("abort restore");
            break;
        }
        sharedCapturer->RestoreAudioInLoop(restoreResult, tryCounter);
    }
}

void AudioCapturerPrivate::RestoreAudioInLoop(bool &restoreResult, int32_t &tryCounter)
{
    std::unique_lock<std::shared_mutex> lock;
    if (callbackLoopTid_ != gettid()) { // No need to add lock in callback thread to prevent deadlocks
        lock = std::unique_lock<std::shared_mutex>(capturerMutex_);
    }
    CHECK_AND_RETURN_LOG(audioStream_, "audioStream_ is nullptr, no need for restore");
    AUDIO_INFO_LOG("Restore AudioCapturer %{public}u when server died", sessionID_);
    RestoreInfo restoreInfo;
    restoreInfo.restoreReason = SERVER_DIED;
    restoreResult = SwitchToTargetStream(audioStream_->GetStreamClass(), restoreInfo);
    AUDIO_INFO_LOG("Set restore status when server died, restore result %{public}d", restoreResult);
    return;
}

// Inner function. Must be called with AudioCapturerPrivate::capturerMutex_ held
int32_t AudioCapturerPrivate::GetCurrentInputDevicesInner(AudioDeviceDescriptor &deviceInfo) const
{
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    uint32_t sessionId = static_cast<uint32_t>(-1);
    int32_t ret = GetAudioStreamIdInner(sessionId);
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

// Inner function. Must be called with AudioCapturerPrivate::capturerMutex_ held
int32_t AudioCapturerPrivate::GetAudioStreamIdInner(uint32_t &sessionID) const
{
    CHECK_AND_RETURN_RET_LOG(audioStream_ != nullptr, ERR_INVALID_HANDLE, "GetAudioStreamId faied.");
    return audioStream_->GetAudioSessionID(sessionID);
}

// Inner function. Must be called with AudioCapturerPrivate::capturerMutex_ held
uint32_t AudioCapturerPrivate::GetOverflowCountInner() const
{
    return audioStream_->GetOverflowCount();
}

// Inner function. Must be called with AudioCapturerPrivate::capturerMutex_ held
CapturerState AudioCapturerPrivate::GetStatusInner() const
{
    return static_cast<CapturerState>(audioStream_->GetState());
}

std::shared_ptr<IAudioStream> AudioCapturerPrivate::GetInnerStream() const
{
    std::shared_lock<std::shared_mutex> lock;
    if (callbackLoopTid_ != gettid()) { // No need to add lock in callback thread to prevent deadlocks
        lock = std::shared_lock<std::shared_mutex>(capturerMutex_);
    }
    return audioStream_;
}
// LCOV_EXCL_STOP

std::shared_ptr<AudioStreamDescriptor> AudioCapturerPrivate::GenerateStreamDesc(
    const IAudioStream::SwitchInfo &switchInfo, const RestoreInfo &restoreInfo)
{
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();

    streamDesc->audioMode_ = AUDIO_MODE_RECORD;
    streamDesc->createTimeStamp_ = ClockTime::GetCurNano();
    streamDesc->appInfo_ = appInfo_;
    streamDesc->callerUid_ = static_cast<int32_t>(getuid());
    streamDesc->callerPid_ = static_cast<int32_t>(getpid());

    // update with switchInfo
    AudioStreamInfo &streamInfo = streamDesc->streamInfo_;
    streamInfo.format = static_cast<AudioSampleFormat>(switchInfo.params.format);
    streamInfo.samplingRate = static_cast<AudioSamplingRate>(switchInfo.params.samplingRate);
    streamInfo.channels = static_cast<AudioChannel>(switchInfo.params.channels);
    streamInfo.encoding = static_cast<AudioEncodingType>(switchInfo.params.encoding);
    streamInfo.channelLayout = static_cast<AudioChannelLayout>(switchInfo.params.channelLayout);
    streamDesc->capturerInfo_= switchInfo.capturerInfo;
    streamDesc->sessionId_ = switchInfo.sessionId;

    // update with restoreInfo
    streamDesc->routeFlag_ = restoreInfo.routeFlag;
    if (restoreInfo.targetStreamFlag == AUDIO_FLAG_FORCED_NORMAL) {
        streamDesc->capturerInfo_.originalFlag = AUDIO_FLAG_FORCED_NORMAL;
    }

    return streamDesc;
}

void AudioCapturerPrivate::SetInterruptEventCallbackType(InterruptEventCallbackType callbackType)
{
    audioInterrupt_.callbackType = callbackType;
}

int32_t AudioCapturerPrivate::CheckAudioCapturer(std::string callingFunc)
{
    CheckAndStopAudioCapturer(callingFunc);
    return CheckAndRestoreAudioCapturer(callingFunc);
}

int32_t AudioCapturerPrivate::CheckAndStopAudioCapturer(std::string callingFunc)
{
    std::unique_lock<std::shared_mutex> lock(capturerMutex_, std::defer_lock);
    if (callbackLoopTid_ != gettid()) {
        lock.lock();
    }
    CHECK_AND_RETURN_RET_LOG(audioStream_, ERR_INVALID_PARAM, "audioStream_ is nullptr");
    bool isNeedStop = audioStream_->GetStopFlag();
    if (!isNeedStop) {
        return SUCCESS;
    }

    AUDIO_INFO_LOG("Before %{public}s, stop audio capturer %{public}u", callingFunc.c_str(), sessionID_);
    if (lock.owns_lock()) {
        lock.unlock();
    }
    Stop();
    return SUCCESS;
}

int32_t AudioCapturerPrivate::HandleCreateFastStreamError(AudioStreamParams &audioStreamParams)
{
    AUDIO_INFO_LOG("Create fast Stream fail, record by normal stream");
    IAudioStream::StreamClass streamClass = IAudioStream::PA_STREAM;
    capturerInfo_.capturerFlags = AUDIO_FLAG_FORCED_NORMAL;

    // Create stream desc and pipe
    std::shared_ptr<AudioStreamDescriptor> streamDesc = ConvertToStreamDescriptor(audioStreamParams);
    uint32_t flag = AUDIO_INPUT_FLAG_NORMAL;
    int32_t ret = AudioPolicyManager::GetInstance().CreateCapturerClient(streamDesc, flag,
        audioStreamParams.originalSessionId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "CreateCapturerClient failed");
    AUDIO_INFO_LOG("Create normal capturer, id: %{public}u", audioStreamParams.originalSessionId);

    audioStream_ = IAudioStream::GetRecordStream(streamClass, audioStreamParams, audioStreamType_, appInfo_.appUid);
    CHECK_AND_RETURN_RET_LOG(audioStream_ != nullptr, ERR_INVALID_PARAM, "Get normal record stream failed");
    ret = InitAudioStream(audioStreamParams);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Init normal audio stream failed");
    audioStream_->SetCaptureMode(CAPTURE_MODE_CALLBACK);
    callbackLoopTid_ = audioStream_->GetCallbackLoopTid();
    return ret;
}
}  // namespace AudioStandard
}  // namespace OHOS
