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
#define LOG_TAG "AudioRenderer"
#endif

#include <sstream>
#include "securec.h"
#include <atomic>
#include <cinttypes>
#include <memory>

#include "audio_renderer.h"
#include "audio_renderer_private.h"

#include "audio_renderer_log.h"
#include "audio_errors.h"
#include "audio_policy_manager.h"
#include "audio_utils.h"

#include "media_monitor_manager.h"
#include "audio_scope_exit.h"

namespace OHOS {
namespace AudioStandard {

static const std::vector<StreamUsage> NEED_VERIFY_PERMISSION_STREAMS = {
    STREAM_USAGE_SYSTEM,
    STREAM_USAGE_DTMF,
    STREAM_USAGE_ENFORCED_TONE,
    STREAM_USAGE_ULTRASONIC,
    STREAM_USAGE_VOICE_MODEM_COMMUNICATION
};
static constexpr uid_t UID_MSDP_SA = 6699;
static constexpr int32_t WRITE_UNDERRUN_NUM = 100;
constexpr int32_t TIME_OUT_SECONDS = 10;

static AudioRendererParams SetStreamInfoToParams(const AudioStreamInfo &streamInfo)
{
    AudioRendererParams params;
    params.sampleFormat = streamInfo.format;
    params.sampleRate = streamInfo.samplingRate;
    params.channelCount = streamInfo.channels;
    params.encodingType = streamInfo.encoding;
    params.channelLayout = streamInfo.channelLayout;
    return params;
}

std::mutex AudioRenderer::createRendererMutex_;

AudioRenderer::~AudioRenderer() = default;
AudioRendererPrivate::~AudioRendererPrivate()
{
    AUDIO_INFO_LOG("Destruct in");
    abortRestore_ = true;

    std::shared_ptr<OutputDeviceChangeWithInfoCallbackImpl> outputDeviceChangeCallback = outputDeviceChangeCallback_;
    if (outputDeviceChangeCallback != nullptr) {
        outputDeviceChangeCallback->RemoveCallback();
        outputDeviceChangeCallback->UnsetAudioRendererObj();
    }
    std::shared_ptr<AudioRendererConcurrencyCallbackImpl> cb = audioConcurrencyCallback_;
    if (cb != nullptr) {
        cb->UnsetAudioRendererObj();
        AudioPolicyManager::GetInstance().UnsetAudioConcurrencyCallback(sessionID_);
    }
    for (auto id : usedSessionId_) {
        AudioPolicyManager::GetInstance().UnregisterDeviceChangeWithInfoCallback(id);
    }

    RendererState state = GetStatus();
    if (state != RENDERER_RELEASED && state != RENDERER_NEW) {
        Release();
    }

    if (rendererProxyObj_ != nullptr) {
        rendererProxyObj_->UnsetRendererObj();
        AudioPolicyManager::GetInstance().RemoveClientTrackerStub(sessionID_);
    }

    RemoveRendererPolicyServiceDiedCallback();
    DumpFileUtil::CloseDumpFile(&dumpFile_);
}

int32_t AudioRenderer::CheckMaxRendererInstances()
{
    int32_t ret = AudioPolicyManager::GetInstance().CheckMaxRendererInstances();
    if (ret == ERR_OVERFLOW) {
        return ret;
    }
    return SUCCESS;
}

size_t GetAudioFormatSize(AudioSampleFormat format)
{
    size_t bitWidthSize = 2;
    switch (format) {
        case SAMPLE_U8:
            bitWidthSize = 1; // size is 1
            break;
        case SAMPLE_S16LE:
            bitWidthSize = 2; // size is 2
            break;
        case SAMPLE_S24LE:
            bitWidthSize = 3; // size is 3
            break;
        case SAMPLE_S32LE:
            bitWidthSize = 4; // size is 4
            break;
        default:
            bitWidthSize = 2; // size is 2
            break;
    }
    return bitWidthSize;
}

size_t GetFormatSize(const AudioStreamParams& info)
{
    return GetAudioFormatSize(static_cast<AudioSampleFormat>(info.format));
}
 
int32_t AudioRenderer::MuteAudioBuffer(uint8_t *addr, size_t offset, size_t length, AudioSampleFormat format)
{
    CHECK_AND_RETURN_RET_LOG(addr != nullptr && length != 0, ERR_INVALID_PARAM, "Invalid addr or length");
 
    bool formatValid = std::find(AUDIO_SUPPORTED_FORMATS.begin(), AUDIO_SUPPORTED_FORMATS.end(), format)
        != AUDIO_SUPPORTED_FORMATS.end();
    CHECK_AND_RETURN_RET_LOG(formatValid, ERR_INVALID_PARAM, "Invalid AudioSampleFormat");
 
    size_t bitWidthSize = GetAudioFormatSize(format);
    if (bitWidthSize != 0 && length % bitWidthSize != 0) {
        AUDIO_ERR_LOG("length is %{public}zu, can not be divided by %{public}zu", length, bitWidthSize);
        return ERR_INVALID_PARAM;
    }
 
    int32_t ret = 0;
    if (format == SAMPLE_U8) {
        ret = memset_s(addr + offset, length, 0X7F, length);
    } else {
        ret = memset_s(addr + offset, length, 0, length);
    }
    CHECK_AND_RETURN_RET_LOG(ret == EOK, ERR_OPERATION_FAILED, "Mute failed!");
    return SUCCESS;
}

std::unique_ptr<AudioRenderer> AudioRenderer::Create(AudioStreamType audioStreamType)
{
    AppInfo appInfo = {};
    return Create(audioStreamType, appInfo);
}

std::unique_ptr<AudioRenderer> AudioRenderer::Create(AudioStreamType audioStreamType, const AppInfo &appInfo)
{
    if (audioStreamType == STREAM_MEDIA) {
        audioStreamType = STREAM_MUSIC;
    }

    return std::make_unique<AudioRendererPrivate>(audioStreamType, appInfo, true);
}

std::unique_ptr<AudioRenderer> AudioRenderer::Create(const AudioRendererOptions &rendererOptions)
{
    AppInfo appInfo = {};
    return Create("", rendererOptions, appInfo);
}

std::unique_ptr<AudioRenderer> AudioRenderer::Create(const AudioRendererOptions &rendererOptions,
    const AppInfo &appInfo)
{
    return Create("", rendererOptions, appInfo);
}

std::unique_ptr<AudioRenderer> AudioRenderer::Create(const std::string cachePath,
    const AudioRendererOptions &rendererOptions)
{
    AppInfo appInfo = {};
    return Create(cachePath, rendererOptions, appInfo);
}

std::unique_ptr<AudioRenderer> AudioRenderer::Create(const std::string cachePath,
    const AudioRendererOptions &rendererOptions, const AppInfo &appInfo)
{
    Trace trace("AudioRenderer::Create");
    std::lock_guard<std::mutex> lock(createRendererMutex_);
    CHECK_AND_RETURN_RET_LOG(AudioPolicyManager::GetInstance().GetAudioPolicyManagerProxy() != nullptr,
        nullptr, "sa not start");
    AudioStreamType audioStreamType = IAudioStream::GetStreamType(rendererOptions.rendererInfo.contentType,
        rendererOptions.rendererInfo.streamUsage);
    if (audioStreamType == STREAM_ULTRASONIC && getuid() != UID_MSDP_SA) {
        AudioRenderer::SendRendererCreateError(rendererOptions.rendererInfo.streamUsage,
            ERR_INVALID_PARAM);
        AUDIO_ERR_LOG("ULTRASONIC can only create by MSDP");
        return nullptr;
    }

    auto audioRenderer = std::make_unique<AudioRendererPrivate>(audioStreamType, appInfo, false);
    if (audioRenderer == nullptr) {
        AudioRenderer::SendRendererCreateError(rendererOptions.rendererInfo.streamUsage,
            ERR_OPERATION_FAILED);
    }
    CHECK_AND_RETURN_RET_LOG(audioRenderer != nullptr, nullptr, "Failed to create renderer object");
    if (!cachePath.empty()) {
        AUDIO_DEBUG_LOG("Set application cache path");
        audioRenderer->cachePath_ = cachePath;
    }

    int32_t rendererFlags = rendererOptions.rendererInfo.rendererFlags;
    AUDIO_INFO_LOG("StreamClientState for Renderer::Create. content: %{public}d, usage: %{public}d, "\
        "flags: %{public}d, uid: %{public}d", rendererOptions.rendererInfo.contentType,
        rendererOptions.rendererInfo.streamUsage, rendererFlags, appInfo.appUid);

    audioRenderer->rendererInfo_.contentType = rendererOptions.rendererInfo.contentType;
    audioRenderer->rendererInfo_.streamUsage = rendererOptions.rendererInfo.streamUsage;
    audioRenderer->rendererInfo_.playerType = rendererOptions.rendererInfo.playerType;
    audioRenderer->rendererInfo_.expectedPlaybackDurationBytes
        = rendererOptions.rendererInfo.expectedPlaybackDurationBytes;
    audioRenderer->rendererInfo_.samplingRate = rendererOptions.streamInfo.samplingRate;
    audioRenderer->rendererInfo_.rendererFlags = rendererFlags;
    audioRenderer->rendererInfo_.originalFlag = rendererFlags;
    audioRenderer->privacyType_ = rendererOptions.privacyType;
    audioRenderer->strategy_ = rendererOptions.strategy;
    audioRenderer->originalStrategy_ = rendererOptions.strategy;
    AudioRendererParams params = SetStreamInfoToParams(rendererOptions.streamInfo);
    if (audioRenderer->SetParams(params) != SUCCESS) {
        AUDIO_ERR_LOG("SetParams failed in renderer");
        audioRenderer = nullptr;
        AudioRenderer::SendRendererCreateError(rendererOptions.rendererInfo.streamUsage,
            ERR_OPERATION_FAILED);
    }

    return audioRenderer;
}

void AudioRenderer::SendRendererCreateError(const StreamUsage &sreamUsage,
    const int32_t &errorCode)
{
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::AUDIO, Media::MediaMonitor::AUDIO_STREAM_CREATE_ERROR_STATS,
        Media::MediaMonitor::FREQUENCY_AGGREGATION_EVENT);
    bean->Add("IS_PLAYBACK", 1);
    bean->Add("CLIENT_UID", static_cast<int32_t>(getuid()));
    bean->Add("STREAM_TYPE", sreamUsage);
    bean->Add("ERROR_CODE", errorCode);
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

AudioRendererPrivate::AudioRendererPrivate(AudioStreamType audioStreamType, const AppInfo &appInfo, bool createStream)
{
    appInfo_ = appInfo;
    if (!(appInfo_.appPid)) {
        appInfo_.appPid = getpid();
    }

    if (appInfo_.appUid < 0) {
        appInfo_.appUid = static_cast<int32_t>(getuid());
    }

    if (createStream) {
        AudioStreamParams tempParams = {};
        audioStream_ = IAudioStream::GetPlaybackStream(IAudioStream::PA_STREAM, tempParams, audioStreamType,
            appInfo_.appUid);
        if (audioStream_ && STREAM_TYPE_USAGE_MAP.count(audioStreamType) != 0) {
            // Initialize the streamUsage based on the streamType
            rendererInfo_.streamUsage = STREAM_TYPE_USAGE_MAP.at(audioStreamType);
        }
        AUDIO_INFO_LOG("AudioRendererPrivate create normal stream for old mode.");
    }

    rendererProxyObj_ = std::make_shared<AudioRendererProxyObj>();
    if (!rendererProxyObj_) {
        AUDIO_WARNING_LOG("AudioRendererProxyObj Memory Allocation Failed !!");
    }

    audioInterrupt_.audioFocusType.streamType = audioStreamType;
    audioInterrupt_.pid = appInfo_.appPid;
    audioInterrupt_.mode = SHARE_MODE;
    audioInterrupt_.parallelPlayFlag = false;

    state_ = RENDERER_PREPARED;
}

int32_t AudioRendererPrivate::InitAudioInterruptCallback()
{
    AUDIO_DEBUG_LOG("in");

    if (audioInterrupt_.sessionId != 0) {
        AUDIO_INFO_LOG("old session already has interrupt, need to reset");
        (void)AudioPolicyManager::GetInstance().DeactivateAudioInterrupt(audioInterrupt_);
        (void)AudioPolicyManager::GetInstance().UnsetAudioInterruptCallback(audioInterrupt_.sessionId);
    }

    CHECK_AND_RETURN_RET_LOG(audioInterrupt_.mode == SHARE_MODE || audioInterrupt_.mode == INDEPENDENT_MODE,
        ERR_INVALID_PARAM, "Invalid interrupt mode!");
    CHECK_AND_RETURN_RET_LOG(audioStream_->GetAudioSessionID(audioInterrupt_.sessionId) == 0, ERR_INVALID_INDEX,
        "GetAudioSessionID failed");
    sessionID_ = audioInterrupt_.sessionId;
    audioInterrupt_.streamUsage = rendererInfo_.streamUsage;
    audioInterrupt_.contentType = rendererInfo_.contentType;
    audioInterrupt_.sessionStrategy = strategy_;

    AUDIO_INFO_LOG("interruptMode %{public}d, streamType %{public}d, sessionID %{public}d",
        audioInterrupt_.mode, audioInterrupt_.audioFocusType.streamType, audioInterrupt_.sessionId);

    if (audioInterruptCallback_ == nullptr) {
        audioInterruptCallback_ = std::make_shared<AudioRendererInterruptCallbackImpl>(audioStream_, audioInterrupt_);
        CHECK_AND_RETURN_RET_LOG(audioInterruptCallback_ != nullptr, ERROR,
            "Failed to allocate memory for audioInterruptCallback_");
    }
    return AudioPolicyManager::GetInstance().SetAudioInterruptCallback(sessionID_, audioInterruptCallback_,
        appInfo_.appUid);
}

int32_t AudioRendererPrivate::InitOutputDeviceChangeCallback()
{
    if (!outputDeviceChangeCallback_) {
        outputDeviceChangeCallback_ = std::make_shared<OutputDeviceChangeWithInfoCallbackImpl>();
        CHECK_AND_RETURN_RET_LOG(outputDeviceChangeCallback_ != nullptr, ERROR, "Memory allocation failed");
    }

    outputDeviceChangeCallback_->SetAudioRendererObj(this);

    uint32_t sessionId;
    int32_t ret = GetAudioStreamId(sessionId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Get sessionId failed");

    usedSessionId_.push_back(sessionId);
    ret = AudioPolicyManager::GetInstance().RegisterDeviceChangeWithInfoCallback(sessionId,
        outputDeviceChangeCallback_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Register failed");

    return SUCCESS;
}

int32_t AudioRendererPrivate::InitAudioStream(AudioStreamParams audioStreamParams)
{
    Trace trace("AudioRenderer::InitAudioStream");
    AudioRenderer *renderer = this;
    rendererProxyObj_->SaveRendererObj(renderer);
    audioStream_->SetRendererInfo(rendererInfo_);
    audioStream_->SetClientID(appInfo_.appPid, appInfo_.appUid, appInfo_.appTokenId, appInfo_.appFullTokenId);

    SetAudioPrivacyType(privacyType_);
    audioStream_->SetStreamTrackerState(false);

    int32_t ret = audioStream_->SetAudioStreamInfo(audioStreamParams, rendererProxyObj_);
    CHECK_AND_RETURN_RET_LOG(!ret, ret, "SetParams SetAudioStreamInfo Failed");

    ret = GetAudioStreamId(sessionID_);
    CHECK_AND_RETURN_RET_LOG(!ret, ret, "GetAudioStreamId err");
    InitLatencyMeasurement(audioStreamParams);
    InitAudioConcurrencyCallback();

    return SUCCESS;
}

int32_t AudioRendererPrivate::GetFrameCount(uint32_t &frameCount) const
{
    return audioStream_->GetFrameCount(frameCount);
}

int32_t AudioRendererPrivate::GetLatency(uint64_t &latency) const
{
    return audioStream_->GetLatency(latency);
}

void AudioRendererPrivate::SetAudioPrivacyType(AudioPrivacyType privacyType)
{
    privacyType_ = privacyType;
    if (audioStream_ == nullptr) {
        return;
    }
    audioStream_->SetPrivacyType(privacyType);
}

AudioPrivacyType AudioRendererPrivate::GetAudioPrivacyType()
{
    return privacyType_;
}

IAudioStream::StreamClass AudioRendererPrivate::GetPreferredStreamClass(AudioStreamParams audioStreamParams)
{
    if (rendererInfo_.originalFlag == AUDIO_FLAG_FORCED_NORMAL) {
        return IAudioStream::PA_STREAM;
    }
    if (rendererInfo_.originalFlag == AUDIO_FLAG_MMAP &&
        !IAudioStream::IsStreamSupported(rendererInfo_.originalFlag, audioStreamParams)) {
        AUDIO_WARNING_LOG("Unsupported stream params, will create normal stream");
        rendererInfo_.originalFlag = AUDIO_FLAG_NORMAL;
        rendererInfo_.rendererFlags = AUDIO_FLAG_NORMAL;
    }
    int32_t flag = AudioPolicyManager::GetInstance().GetPreferredOutputStreamType(rendererInfo_);
    AUDIO_INFO_LOG("Preferred renderer flag: %{public}d", flag);
    if (flag == AUDIO_FLAG_MMAP) {
        rendererInfo_.rendererFlags = AUDIO_FLAG_MMAP;
        isFastRenderer_ = true;
        return IAudioStream::FAST_STREAM;
    }
    if (flag == AUDIO_FLAG_VOIP_FAST) {
        // It is not possible to directly create a fast VoIP stream
        isFastVoipSupported_ = true;
    } else if (flag == AUDIO_FLAG_VOIP_DIRECT) {
        isDirectVoipSupported_ = IsDirectVoipParams(audioStreamParams);
        rendererInfo_.originalFlag = isDirectVoipSupported_ ? AUDIO_FLAG_VOIP_DIRECT : AUDIO_FLAG_NORMAL;
        // The VoIP direct mode can only be used for RENDER_MODE_CALLBACK
        rendererInfo_.rendererFlags = (isDirectVoipSupported_ && audioRenderMode_ == RENDER_MODE_CALLBACK) ?
            AUDIO_FLAG_VOIP_DIRECT : AUDIO_FLAG_NORMAL;
        AUDIO_INFO_LOG("Preferred renderer flag is VOIP_DIRECT. Actual flag: %{public}d", rendererInfo_.rendererFlags);
        return IAudioStream::PA_STREAM;
    }

    rendererInfo_.rendererFlags = AUDIO_FLAG_NORMAL;
    return IAudioStream::PA_STREAM;
}

bool AudioRendererPrivate::IsDirectVoipParams(const AudioStreamParams &audioStreamParams)
{
    // VoIP derect only supports 8K, 16K and 48K sampling rate.
    if (!(audioStreamParams.samplingRate == SAMPLE_RATE_8000 ||
        audioStreamParams.samplingRate == SAMPLE_RATE_16000 ||
        audioStreamParams.samplingRate == SAMPLE_RATE_48000)) {
        AUDIO_ERR_LOG("The sampling rate %{public}d is not supported for direct VoIP mode",
            audioStreamParams.samplingRate);
        return false;
    }

    // VoIP derect only supports MONO and STEREO.
    if (!(audioStreamParams.channels == MONO || audioStreamParams.channels == STEREO)) {
        AUDIO_ERR_LOG("The channels %{public}d is not supported for direct VoIP mode",
            audioStreamParams.channels);
        return false;
    }

    // VoIP derect only supports 16bit and 32bit.
    if (!(audioStreamParams.format == SAMPLE_S16LE || audioStreamParams.format == SAMPLE_S32LE)) {
        AUDIO_ERR_LOG("The format %{public}d is not supported for direct VoIP mode",
            audioStreamParams.format);
        return false;
    }

    AUDIO_INFO_LOG("Valid params for direct VoIP: sampling rate %{public}d, format %{public}d, channels %{public}d",
        audioStreamParams.samplingRate, audioStreamParams.format, audioStreamParams.channels);
    return true;
}

int32_t AudioRendererPrivate::SetParams(const AudioRendererParams params)
{
    Trace trace("AudioRenderer::SetParams");
    AUDIO_INFO_LOG("StreamClientState for Renderer::SetParams.");

    std::shared_lock<std::shared_mutex> lockShared(rendererMutex_);
    std::lock_guard<std::mutex> lock(setParamsMutex_);
    AudioStreamParams audioStreamParams = ConvertToAudioStreamParams(params);

    AudioStreamType audioStreamType = IAudioStream::GetStreamType(rendererInfo_.contentType, rendererInfo_.streamUsage);
    IAudioStream::StreamClass streamClass = GetPreferredStreamClass(audioStreamParams);
    int32_t ret = PrepareAudioStream(audioStreamParams, audioStreamType, streamClass);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_INVALID_PARAM, "PrepareAudioStream failed");

    ret = InitAudioStream(audioStreamParams);
    // When the fast stream creation fails, a normal stream is created
    if (ret != SUCCESS && streamClass == IAudioStream::FAST_STREAM) {
        AUDIO_INFO_LOG("Create fast Stream fail, play by normal stream.");
        streamClass = IAudioStream::PA_STREAM;
        isFastRenderer_ = false;
        audioStream_ = IAudioStream::GetPlaybackStream(streamClass, audioStreamParams, audioStreamType,
            appInfo_.appUid);
        CHECK_AND_RETURN_RET_LOG(audioStream_ != nullptr,
            ERR_INVALID_PARAM, "SetParams GetPlayBackStream failed when create normal stream.");
        ret = InitAudioStream(audioStreamParams);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "InitAudioStream failed");
        audioStream_->SetRenderMode(RENDER_MODE_CALLBACK);
    }

    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetAudioStreamInfo Failed");
    AUDIO_INFO_LOG("SetAudioStreamInfo Succeeded");

    RegisterRendererPolicyServiceDiedCallback();
    // eg: 100005_44100_2_1_client_in.pcm
    std::string dumpFileName = std::to_string(sessionID_) + "_" + std::to_string(params.sampleRate) + "_" +
        std::to_string(params.channelCount) + "_" + std::to_string(params.sampleFormat) + "_client_in.pcm";
    DumpFileUtil::OpenDumpFile(DUMP_CLIENT_PARA, dumpFileName, &dumpFile_);

    ret = InitOutputDeviceChangeCallback();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "InitOutputDeviceChangeCallback Failed");

    return InitAudioInterruptCallback();
}

int32_t AudioRendererPrivate::PrepareAudioStream(const AudioStreamParams &audioStreamParams,
    const AudioStreamType &audioStreamType, IAudioStream::StreamClass &streamClass)
{
    AUDIO_INFO_LOG("Create stream with flag: %{public}d, original flag: %{public}d, streamClass: %{public}d",
        rendererInfo_.rendererFlags, rendererInfo_.originalFlag, streamClass);

    // check AudioStreamParams for fast stream
    // As fast stream only support specified audio format, we should call GetPlaybackStream with audioStreamParams.
    ActivateAudioConcurrency(audioStreamParams, audioStreamType, streamClass);
    if (audioStream_ == nullptr) {
        audioStream_ = IAudioStream::GetPlaybackStream(streamClass, audioStreamParams, audioStreamType,
            appInfo_.appUid);
        CHECK_AND_RETURN_RET_LOG(audioStream_ != nullptr, ERR_INVALID_PARAM, "SetParams GetPlayBackStream failed.");
        AUDIO_INFO_LOG("IAudioStream::GetStream success");
        audioStream_->SetApplicationCachePath(cachePath_);
        isFastRenderer_ = IAudioStream::IsFastStreamClass(streamClass);
    }
    return SUCCESS;
}

int32_t AudioRendererPrivate::GetParams(AudioRendererParams &params) const
{
    AudioStreamParams audioStreamParams;
    int32_t result = audioStream_->GetAudioStreamInfo(audioStreamParams);
    if (!result) {
        params.sampleFormat = static_cast<AudioSampleFormat>(audioStreamParams.format);
        params.sampleRate = static_cast<AudioSamplingRate>(audioStreamParams.samplingRate);
        params.channelCount = static_cast<AudioChannel>(audioStreamParams.channels);
        params.encodingType = static_cast<AudioEncodingType>(audioStreamParams.encoding);
        params.channelLayout = static_cast<AudioChannelLayout>(audioStreamParams.channelLayout);
    }

    return result;
}

int32_t AudioRendererPrivate::GetRendererInfo(AudioRendererInfo &rendererInfo) const
{
    rendererInfo = rendererInfo_;

    return SUCCESS;
}

int32_t AudioRendererPrivate::GetStreamInfo(AudioStreamInfo &streamInfo) const
{
    AudioStreamParams audioStreamParams;
    int32_t result = audioStream_->GetAudioStreamInfo(audioStreamParams);
    if (!result) {
        streamInfo.format = static_cast<AudioSampleFormat>(audioStreamParams.format);
        streamInfo.samplingRate = static_cast<AudioSamplingRate>(audioStreamParams.samplingRate);
        streamInfo.channels = static_cast<AudioChannel>(audioStreamParams.channels);
        streamInfo.encoding = static_cast<AudioEncodingType>(audioStreamParams.encoding);
        streamInfo.channelLayout = static_cast<AudioChannelLayout>(audioStreamParams.channelLayout);
    }

    return result;
}

int32_t AudioRendererPrivate::SetRendererCallback(const std::shared_ptr<AudioRendererCallback> &callback)
{
    std::shared_lock<std::shared_mutex> lockShared(rendererMutex_);
    std::lock_guard<std::mutex> lock(setStreamCallbackMutex_);
    // If the client is using the deprecated SetParams API. SetRendererCallback must be invoked, after SetParams.
    // In general, callbacks can only be set after the renderer state is PREPARED.
    RendererState state = GetStatus();
    CHECK_AND_RETURN_RET_LOG(state != RENDERER_NEW && state != RENDERER_RELEASED, ERR_ILLEGAL_STATE,
        "incorrect state:%{public}d to register cb", state);

    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM,
        "callback param is null");

    // Save reference for interrupt callback
    CHECK_AND_RETURN_RET_LOG(audioInterruptCallback_ != nullptr, ERROR,
        "audioInterruptCallback_ == nullptr");
    std::shared_ptr<AudioRendererInterruptCallbackImpl> cbInterrupt =
        std::static_pointer_cast<AudioRendererInterruptCallbackImpl>(audioInterruptCallback_);
    cbInterrupt->SaveCallback(callback);

    // Save and Set reference for stream callback. Order is important here.
    if (audioStreamCallback_ == nullptr) {
        audioStreamCallback_ = std::make_shared<AudioStreamCallbackRenderer>();
        CHECK_AND_RETURN_RET_LOG(audioStreamCallback_ != nullptr, ERROR,
            "Failed to allocate memory for audioStreamCallback_");
    }
    std::shared_ptr<AudioStreamCallbackRenderer> cbStream =
        std::static_pointer_cast<AudioStreamCallbackRenderer>(audioStreamCallback_);
    cbStream->SaveCallback(callback);
    (void)audioStream_->SetStreamCallback(audioStreamCallback_);

    return SUCCESS;
}

int32_t AudioRendererPrivate::SetRendererPositionCallback(int64_t markPosition,
    const std::shared_ptr<RendererPositionCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG((callback != nullptr) && (markPosition > 0), ERR_INVALID_PARAM,
        "input param is invalid");

    audioStream_->SetRendererPositionCallback(markPosition, callback);

    return SUCCESS;
}

void AudioRendererPrivate::UnsetRendererPositionCallback()
{
    audioStream_->UnsetRendererPositionCallback();
}

int32_t AudioRendererPrivate::SetRendererPeriodPositionCallback(int64_t frameNumber,
    const std::shared_ptr<RendererPeriodPositionCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG((callback != nullptr) && (frameNumber > 0), ERR_INVALID_PARAM,
        "input param is invalid");

    audioStream_->SetRendererPeriodPositionCallback(frameNumber, callback);

    return SUCCESS;
}

void AudioRendererPrivate::UnsetRendererPeriodPositionCallback()
{
    audioStream_->UnsetRendererPeriodPositionCallback();
}

bool AudioRendererPrivate::Start(StateChangeCmdType cmdType)
{
    Trace trace("AudioRenderer::Start");
    std::lock_guard<std::shared_mutex> lock(rendererMutex_);
    AUDIO_INFO_LOG("StreamClientState for Renderer::Start. id: %{public}u, streamType: %{public}d, "\
        "interruptMode: %{public}d", sessionID_, audioInterrupt_.audioFocusType.streamType, audioInterrupt_.mode);

    RendererState state = GetStatus();
    CHECK_AND_RETURN_RET_LOG((state == RENDERER_PREPARED) || (state == RENDERER_STOPPED) || (state == RENDERER_PAUSED),
        false, "Start failed. Illegal state:%{public}u", state);

    CHECK_AND_RETURN_RET_LOG(!isSwitching_, false,
        "Start failed. Switching state: %{public}d", isSwitching_);

    if (audioInterrupt_.audioFocusType.streamType == STREAM_DEFAULT ||
        audioInterrupt_.sessionId == INVALID_SESSION_ID) {
        return false;
    }

    CHECK_AND_RETURN_RET_LOG(audioStream_ != nullptr, false, "audio stream is null");

    if (GetVolume() == 0 && isStillMuted_) {
        AUDIO_INFO_LOG("StreamClientState for Renderer::Start. volume=%{public}f, isStillMuted_=%{public}d",
            GetVolume(), isStillMuted_);
        audioInterrupt_.sessionStrategy.concurrencyMode = AudioConcurrencyMode::SLIENT;
    } else {
        isStillMuted_ = false;
    }

    {
        std::lock_guard<std::mutex> lock(silentModeAndMixWithOthersMutex_);
        if (!audioStream_->GetSilentModeAndMixWithOthers()) {
            int32_t ret = AudioPolicyManager::GetInstance().ActivateAudioInterrupt(audioInterrupt_);
            CHECK_AND_RETURN_RET_LOG(ret == 0, false, "ActivateAudioInterrupt Failed");
        }
    }

    if (IsNoStreamRenderer()) {
        // no stream renderer only need to activate audio interrupt
        state_ = RENDERER_RUNNING;
        return true;
    }

    bool result = audioStream_->StartAudioStream(cmdType);
    if (!result) {
        AUDIO_ERR_LOG("Start audio stream failed");
        std::lock_guard<std::mutex> lock(silentModeAndMixWithOthersMutex_);
        if (!audioStream_->GetSilentModeAndMixWithOthers()) {
            int32_t ret = AudioPolicyManager::GetInstance().DeactivateAudioInterrupt(audioInterrupt_);
            if (ret != 0) {
                AUDIO_WARNING_LOG("DeactivateAudioInterrupt Failed");
            }
        }
    }

    state_ = RENDERER_RUNNING;

    return result;
}

int32_t AudioRendererPrivate::Write(uint8_t *buffer, size_t bufferSize)
{
    Trace trace("AudioRenderer::Write");
    MockPcmData(buffer, bufferSize);
    int32_t size = audioStream_->Write(buffer, bufferSize);
    if (size > 0) {
        DumpFileUtil::WriteDumpFile(dumpFile_, static_cast<void *>(buffer), size);
    }
    return size;
}

int32_t AudioRendererPrivate::Write(uint8_t *pcmBuffer, size_t pcmSize, uint8_t *metaBuffer, size_t metaSize)
{
    Trace trace("Write");
    int32_t size = audioStream_->Write(pcmBuffer, pcmSize, metaBuffer, metaSize);
    if (size > 0) {
        DumpFileUtil::WriteDumpFile(dumpFile_, static_cast<void *>(pcmBuffer), size);
    }
    return size;
}

RendererState AudioRendererPrivate::GetStatus() const
{
    if (IsNoStreamRenderer()) {
        return state_;
    }
    return static_cast<RendererState>(audioStream_->GetState());
}

bool AudioRendererPrivate::GetAudioTime(Timestamp &timestamp, Timestamp::Timestampbase base) const
{
    return audioStream_->GetAudioTime(timestamp, base);
}

bool AudioRendererPrivate::GetAudioPosition(Timestamp &timestamp, Timestamp::Timestampbase base) const
{
    CHECK_AND_RETURN_RET_LOG(audioStream_ != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    return audioStream_->GetAudioPosition(timestamp, base);
}

bool AudioRendererPrivate::Drain() const
{
    Trace trace("AudioRenderer::Drain");
    return audioStream_->DrainAudioStream();
}

bool AudioRendererPrivate::Flush() const
{
    Trace trace("AudioRenderer::Flush");
    return audioStream_->FlushAudioStream();
}

bool AudioRendererPrivate::PauseTransitent(StateChangeCmdType cmdType)
{
    Trace trace("AudioRenderer::PauseTransitent");
    std::lock_guard<std::shared_mutex> lock(rendererMutex_);
    AUDIO_INFO_LOG("StreamClientState for Renderer::PauseTransitent. id: %{public}u", sessionID_);
    if (isSwitching_) {
        AUDIO_ERR_LOG("failed. Switching state: %{public}d", isSwitching_);
        return false;
    }

    if (IsNoStreamRenderer()) {
        // no stream renderer don't need to change audio stream state
        state_ = RENDERER_PAUSED;
        return true;
    }

    RendererState state = GetStatus();
    if (state != RENDERER_RUNNING) {
        // If the stream is not running, there is no need to pause and deactive audio interrupt
        AUDIO_ERR_LOG("State of stream is not running. Illegal state:%{public}u", state);
        return false;
    }
    bool result = audioStream_->PauseAudioStream(cmdType);
    if (result) {
        state_ = RENDERER_PAUSED;
    }

    return result;
}

bool AudioRendererPrivate::Pause(StateChangeCmdType cmdType)
{
    Trace trace("AudioRenderer::Pause");
    AudioXCollie audioXCollie("AudioRenderer::Pause", TIME_OUT_SECONDS);
    std::lock_guard<std::shared_mutex> lock(rendererMutex_);

    AUDIO_INFO_LOG("StreamClientState for Renderer::Pause. id: %{public}u", sessionID_);

    CHECK_AND_RETURN_RET_LOG(!isSwitching_, false, "Pause failed. Switching state: %{public}d", isSwitching_);

    if (IsNoStreamRenderer()) {
        // When the cellular call stream is pausing, only need to deactivate audio interrupt.
        if (AudioPolicyManager::GetInstance().DeactivateAudioInterrupt(audioInterrupt_) != 0) {
            AUDIO_ERR_LOG("DeactivateAudioInterrupt Failed");
        }
        state_ = RENDERER_PAUSED;
        return true;
    }

    RendererState state = GetStatus();
    CHECK_AND_RETURN_RET_LOG(state == RENDERER_RUNNING, false,
        "State of stream is not running. Illegal state:%{public}u", state);
    bool result = audioStream_->PauseAudioStream(cmdType);
    if (result) {
        state_ = RENDERER_PAUSED;
    }

    // When user is intentionally pausing, deactivate to remove from audioFocusInfoList_
    int32_t ret = AudioPolicyManager::GetInstance().DeactivateAudioInterrupt(audioInterrupt_);
    if (ret != 0) {
        AUDIO_ERR_LOG("DeactivateAudioInterrupt Failed");
    }
    (void)audioStream_->SetDuckVolume(1.0f);

    return result;
}

bool AudioRendererPrivate::Stop()
{
    AUDIO_INFO_LOG("StreamClientState for Renderer::Stop. id: %{public}u", sessionID_);
    std::lock_guard<std::shared_mutex> lock(rendererMutex_);
    CHECK_AND_RETURN_RET_LOG(!isSwitching_, false,
        "AudioRenderer::Stop failed. Switching state: %{public}d", isSwitching_);
    if (IsNoStreamRenderer()) {
        // When the cellular call stream is stopping, only need to deactivate audio interrupt.
        if (AudioPolicyManager::GetInstance().DeactivateAudioInterrupt(audioInterrupt_) != 0) {
            AUDIO_WARNING_LOG("DeactivateAudioInterrupt Failed");
        }
        state_ = RENDERER_STOPPED;
        return true;
    }

    WriteUnderrunEvent();
    bool result = audioStream_->StopAudioStream();
    if (result) {
        state_ = RENDERER_STOPPED;
    }
    int32_t ret = AudioPolicyManager::GetInstance().DeactivateAudioInterrupt(audioInterrupt_);
    if (ret != 0) {
        AUDIO_WARNING_LOG("DeactivateAudioInterrupt Failed");
    }
    (void)audioStream_->SetDuckVolume(1.0f);

    return result;
}

bool AudioRendererPrivate::Release()
{
    std::lock_guard<std::shared_mutex> lock(rendererMutex_);
    AUDIO_INFO_LOG("StreamClientState for Renderer::Release. id: %{public}u", sessionID_);

    bool result = audioStream_->ReleaseAudioStream();

    // If Stop call was skipped, Release to take care of Deactivation
    (void)AudioPolicyManager::GetInstance().DeactivateAudioInterrupt(audioInterrupt_);

    // Unregister the callaback in policy server
    (void)AudioPolicyManager::GetInstance().UnsetAudioInterruptCallback(sessionID_);

    for (auto id : usedSessionId_) {
        AudioPolicyManager::GetInstance().UnregisterDeviceChangeWithInfoCallback(id);
    }
    RemoveRendererPolicyServiceDiedCallback();

    return result;
}

int32_t AudioRendererPrivate::GetBufferSize(size_t &bufferSize) const
{
    Trace trace("AudioRenderer::GetBufferSize");
    return audioStream_->GetBufferSize(bufferSize);
}

int32_t AudioRendererPrivate::GetAudioStreamId(uint32_t &sessionID) const
{
    return audioStream_->GetAudioSessionID(sessionID);
}

int32_t AudioRendererPrivate::SetAudioRendererDesc(AudioRendererDesc audioRendererDesc)
{
    ContentType contentType = audioRendererDesc.contentType;
    StreamUsage streamUsage = audioRendererDesc.streamUsage;
    AudioStreamType audioStreamType = IAudioStream::GetStreamType(contentType, streamUsage);
    audioInterrupt_.audioFocusType.streamType = audioStreamType;
    return audioStream_->SetAudioStreamType(audioStreamType);
}

int32_t AudioRendererPrivate::SetStreamType(AudioStreamType audioStreamType)
{
    audioInterrupt_.audioFocusType.streamType = audioStreamType;
    return audioStream_->SetAudioStreamType(audioStreamType);
}

int32_t AudioRendererPrivate::SetVolume(float volume) const
{
    UpdateAudioInterruptStrategy(volume);
    return audioStream_->SetVolume(volume);
}

void AudioRendererPrivate::UpdateAudioInterruptStrategy(float volume) const
{
    State currentState = audioStream_->GetState();
    if (currentState == NEW || currentState == PREPARED) {
        AUDIO_INFO_LOG("UpdateAudioInterruptStrategy for set volume before RUNNING,  volume=%{public}f", volume);
        isStillMuted_ = (volume == 0);
    } else if (isStillMuted_ && volume > 0) {
        isStillMuted_ = false;
        audioInterrupt_.sessionStrategy.concurrencyMode =
            (originalStrategy_.concurrencyMode == AudioConcurrencyMode::INVALID ?
            AudioConcurrencyMode::DEFAULT : originalStrategy_.concurrencyMode);
        if (currentState == RUNNING) {
            AUDIO_INFO_LOG("UpdateAudioInterruptStrategy for set volume,  volume=%{public}f", volume);
            int ret = AudioPolicyManager::GetInstance().ActivateAudioInterrupt(audioInterrupt_, 0, true);
            CHECK_AND_RETURN_LOG(ret == 0, "ActivateAudioInterrupt Failed at SetVolume");
        }
    }
}

float AudioRendererPrivate::GetVolume() const
{
    return audioStream_->GetVolume();
}

int32_t AudioRendererPrivate::SetRenderRate(AudioRendererRate renderRate) const
{
    return audioStream_->SetRenderRate(renderRate);
}

AudioRendererRate AudioRendererPrivate::GetRenderRate() const
{
    return audioStream_->GetRenderRate();
}

int32_t AudioRendererPrivate::SetRendererSamplingRate(uint32_t sampleRate) const
{
    return audioStream_->SetRendererSamplingRate(sampleRate);
}

uint32_t AudioRendererPrivate::GetRendererSamplingRate() const
{
    return audioStream_->GetRendererSamplingRate();
}

int32_t AudioRendererPrivate::SetBufferDuration(uint64_t bufferDuration) const
{
    CHECK_AND_RETURN_RET_LOG(bufferDuration >= MINIMUM_BUFFER_SIZE_MSEC && bufferDuration <= MAXIMUM_BUFFER_SIZE_MSEC,
        ERR_INVALID_PARAM, "Error: Please set the buffer duration between 5ms ~ 20ms");

    return audioStream_->SetBufferSizeInMsec(bufferDuration);
}

int32_t AudioRendererPrivate::SetChannelBlendMode(ChannelBlendMode blendMode)
{
    return audioStream_->SetChannelBlendMode(blendMode);
}

AudioRendererInterruptCallbackImpl::AudioRendererInterruptCallbackImpl(const std::shared_ptr<IAudioStream> &audioStream,
    const AudioInterrupt &audioInterrupt)
    : audioStream_(audioStream), audioInterrupt_(audioInterrupt)
{
    AUDIO_DEBUG_LOG("AudioRendererInterruptCallbackImpl constructor");
}

AudioRendererInterruptCallbackImpl::~AudioRendererInterruptCallbackImpl()
{
    AUDIO_DEBUG_LOG("AudioRendererInterruptCallbackImpl: instance destroy");
}

void AudioRendererInterruptCallbackImpl::SaveCallback(const std::weak_ptr<AudioRendererCallback> &callback)
{
    callback_ = callback;
}

void AudioRendererInterruptCallbackImpl::UpdateAudioStream(const std::shared_ptr<IAudioStream> &audioStream)
{
    std::lock_guard<std::mutex> lock(mutex_);
    audioStream_ = audioStream;
}

void AudioRendererInterruptCallbackImpl::NotifyEvent(const InterruptEvent &interruptEvent)
{
    if (cb_ != nullptr && interruptEvent.callbackToApp) {
        cb_->OnInterrupt(interruptEvent);
        AUDIO_DEBUG_LOG("Send interruptEvent to app successfully");
    } else if (cb_ == nullptr) {
        AUDIO_WARNING_LOG("cb_==nullptr, failed to send interruptEvent");
    } else {
        AUDIO_INFO_LOG("callbackToApp is %{public}d", interruptEvent.callbackToApp);
    }
}

bool AudioRendererInterruptCallbackImpl::HandleForceDucking(const InterruptEventInternal &interruptEvent)
{
    float duckVolumeFactor = interruptEvent.duckVolume;
    int32_t ret = audioStream_->SetDuckVolume(duckVolumeFactor);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, false, "Failed to set duckVolumeFactor(instance) %{public}f",
        duckVolumeFactor);

    AUDIO_INFO_LOG("Set duckVolumeFactor %{public}f successfully.", duckVolumeFactor);
    return true;
}

void AudioRendererInterruptCallbackImpl::NotifyForcePausedToResume(const InterruptEventInternal &interruptEvent)
{
    // Change InterruptForceType to Share, Since app will take care of resuming
    InterruptEvent interruptEventResume {interruptEvent.eventType, INTERRUPT_SHARE,
                                         interruptEvent.hintType};
    NotifyEvent(interruptEventResume);
}

InterruptCallbackEvent AudioRendererInterruptCallbackImpl::HandleAndNotifyForcedEvent(
    const InterruptEventInternal &interruptEvent)
{
    State currentState = audioStream_->GetState();
    audioStream_->GetAudioSessionID(sessionID_);
    switch (interruptEvent.hintType) {
        case INTERRUPT_HINT_PAUSE:
            if (currentState == RUNNING || currentState == PREPARED) {
                (void)audioStream_->PauseAudioStream(); // Just Pause, do not deactivate here
                (void)audioStream_->SetDuckVolume(1.0f);
                isForcePaused_ = true;
            } else {
                AUDIO_WARNING_LOG("sessionId: %{public}u, state: %{public}d. No need to pause",
                    sessionID_, static_cast<int32_t>(currentState));
                return NO_EVENT;
            }
            break;
        case INTERRUPT_HINT_RESUME:
            if ((currentState != PAUSED && currentState != PREPARED) || !isForcePaused_) {
                AUDIO_WARNING_LOG("sessionId: %{public}u, State: %{public}d or not force pause before",
                    sessionID_, static_cast<int32_t>(currentState));
                return NO_EVENT;
            }
            isForcePaused_ = false;
            return FORCE_PAUSED_TO_RESUME_EVENT;
        case INTERRUPT_HINT_STOP:
            (void)audioStream_->StopAudioStream();
            (void)audioStream_->SetDuckVolume(1.0f);
            break;
        case INTERRUPT_HINT_DUCK:
            if (!HandleForceDucking(interruptEvent)) {
                AUDIO_WARNING_LOG("Failed to duck forcely, don't notify app");
                return NO_EVENT;
            }
            isForceDucked_ = true;
            break;
        case INTERRUPT_HINT_UNDUCK:
            CHECK_AND_RETURN_RET_LOG(isForceDucked_, NO_EVENT, "It is not forced ducked, don't unduck or notify app");
            (void)audioStream_->SetDuckVolume(1.0f);
            AUDIO_INFO_LOG("Unduck Volume successfully");
            isForceDucked_ = NO_EVENT;
            break;
        default: // If the hintType is NONE, don't need to send callbacks
            return NO_EVENT;
    }
    return FORCE_EVENT;
}

void AudioRendererInterruptCallbackImpl::NotifyForcedEvent(const InterruptEventInternal &interruptEvent)
{
    InterruptEvent interruptEventForced {interruptEvent.eventType, interruptEvent.forceType, interruptEvent.hintType,
        interruptEvent.callbackToApp};
    NotifyEvent(interruptEventForced);
}

void AudioRendererInterruptCallbackImpl::OnInterrupt(const InterruptEventInternal &interruptEvent)
{
    std::unique_lock<std::mutex> lock(mutex_);

    cb_ = callback_.lock();
    InterruptForceType forceType = interruptEvent.forceType;

    if (audioStream_ != nullptr) {
        audioStream_->GetAudioSessionID(sessionID_);
    }
    AUDIO_INFO_LOG("sessionId: %{public}u, forceType: %{public}d, hintType: %{public}d",
        sessionID_, forceType, interruptEvent.hintType);

    if (forceType != INTERRUPT_FORCE) { // INTERRUPT_SHARE
        AUDIO_DEBUG_LOG("INTERRUPT_SHARE. Let app handle the event");
        InterruptEvent interruptEventShared {interruptEvent.eventType, interruptEvent.forceType,
            interruptEvent.hintType, interruptEvent.callbackToApp};
        NotifyEvent(interruptEventShared);
        return;
    }

    CHECK_AND_RETURN_LOG(audioStream_ != nullptr,
        "Stream is not alive. No need to take forced action");

    auto ret = HandleAndNotifyForcedEvent(interruptEvent);
    lock.unlock();
    if (ret == FORCE_EVENT) {
        // Notify valid forced event callbacks to app
        NotifyForcedEvent(interruptEvent);
    } else if (ret == FORCE_PAUSED_TO_RESUME_EVENT) {
        // sending callback is taken care in NotifyForcePausedToResume
        NotifyForcePausedToResume(interruptEvent);
    }
}

AudioRendererConcurrencyCallbackImpl::AudioRendererConcurrencyCallbackImpl()
{
    AUDIO_INFO_LOG("AudioRendererConcurrencyCallbackImpl ctor");
}

AudioRendererConcurrencyCallbackImpl::~AudioRendererConcurrencyCallbackImpl()
{
    AUDIO_INFO_LOG("AudioRendererConcurrencyCallbackImpl dtor");
}

void AudioRendererConcurrencyCallbackImpl::OnConcedeStream()
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_LOG(renderer_ != nullptr, "renderer is nullptr");
    renderer_->ConcedeStream();
}

int32_t AudioRendererPrivate::InitAudioConcurrencyCallback()
{
    if (audioConcurrencyCallback_ == nullptr) {
        audioConcurrencyCallback_ = std::make_shared<AudioRendererConcurrencyCallbackImpl>();
        CHECK_AND_RETURN_RET_LOG(audioConcurrencyCallback_ != nullptr, ERROR, "Memory Allocation Failed !!");
    }
    audioConcurrencyCallback_->SetAudioRendererObj(this);
    return AudioPolicyManager::GetInstance().SetAudioConcurrencyCallback(sessionID_, audioConcurrencyCallback_);
}

void AudioStreamCallbackRenderer::SaveCallback(const std::weak_ptr<AudioRendererCallback> &callback)
{
    callback_ = callback;
}

void AudioStreamCallbackRenderer::OnStateChange(const State state, const StateChangeCmdType cmdType)
{
    std::shared_ptr<AudioRendererCallback> cb = callback_.lock();
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb == nullptr.");

    cb->OnStateChange(static_cast<RendererState>(state), cmdType);
}

std::vector<AudioSampleFormat> AudioRenderer::GetSupportedFormats()
{
    return AUDIO_SUPPORTED_FORMATS;
}

std::vector<AudioSamplingRate> AudioRenderer::GetSupportedSamplingRates()
{
    return AUDIO_SUPPORTED_SAMPLING_RATES;
}

std::vector<AudioChannel> AudioRenderer::GetSupportedChannels()
{
    return RENDERER_SUPPORTED_CHANNELS;
}

std::vector<AudioEncodingType> AudioRenderer::GetSupportedEncodingTypes()
{
    return AUDIO_SUPPORTED_ENCODING_TYPES;
}

int32_t AudioRendererPrivate::SetRenderMode(AudioRenderMode renderMode)
{
    AUDIO_INFO_LOG("Render mode: %{public}d", renderMode);
    audioRenderMode_ = renderMode;
    if (renderMode == RENDER_MODE_CALLBACK && rendererInfo_.originalFlag != AUDIO_FLAG_FORCED_NORMAL &&
        (rendererInfo_.streamUsage == STREAM_USAGE_VOICE_COMMUNICATION ||
        rendererInfo_.streamUsage == STREAM_USAGE_VIDEO_COMMUNICATION)) {
        // both fast and direct VoIP renderer can only use RENDER_MODE_CALLBACK;
        int32_t flags = AudioPolicyManager::GetInstance().GetPreferredOutputStreamType(rendererInfo_);
        uint32_t sessionId = 0;
        int32_t ret = audioStream_->GetAudioSessionID(sessionId);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Get audio session Id failed");
        uint32_t newSessionId = 0;
        IAudioStream::StreamClass streamClass = IAudioStream::PA_STREAM;
        if (flags == AUDIO_FLAG_VOIP_FAST) {
            AUDIO_INFO_LOG("Switch to fast voip stream");
            streamClass = IAudioStream::VOIP_STREAM;
        } else if (flags == AUDIO_FLAG_VOIP_DIRECT && isDirectVoipSupported_) {
            AUDIO_INFO_LOG("Switch to direct voip stream");
            rendererInfo_.rendererFlags = AUDIO_FLAG_VOIP_DIRECT;
            streamClass = IAudioStream::PA_STREAM;
        }
        if (!SwitchToTargetStream(streamClass, newSessionId, AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN)) {
            AUDIO_ERR_LOG("Switch to target stream failed");
            return ERROR;
        }
        usedSessionId_.push_back(newSessionId);
        ret = AudioPolicyManager::GetInstance().RegisterDeviceChangeWithInfoCallback(newSessionId,
            outputDeviceChangeCallback_);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Register device change callback for new session failed");
    }

    return audioStream_->SetRenderMode(renderMode);
}

AudioRenderMode AudioRendererPrivate::GetRenderMode() const
{
    return audioStream_->GetRenderMode();
}

int32_t AudioRendererPrivate::GetBufferDesc(BufferDesc &bufDesc) const
{
    int32_t ret = audioStream_->GetBufferDesc(bufDesc);
    return ret;
}

int32_t AudioRendererPrivate::Enqueue(const BufferDesc &bufDesc) const
{
    Trace trace("AudioRenderer::Enqueue");
    MockPcmData(bufDesc.buffer, bufDesc.bufLength);
    DumpFileUtil::WriteDumpFile(dumpFile_, static_cast<void *>(bufDesc.buffer), bufDesc.bufLength);
    int32_t ret = audioStream_->Enqueue(bufDesc);
    return ret;
}

int32_t AudioRendererPrivate::Clear() const
{
    return audioStream_->Clear();
}

int32_t AudioRendererPrivate::GetBufQueueState(BufferQueueState &bufState) const
{
    return audioStream_->GetBufQueueState(bufState);
}

void AudioRendererPrivate::SetApplicationCachePath(const std::string cachePath)
{
    cachePath_ = cachePath;
    if (audioStream_ != nullptr) {
        audioStream_->SetApplicationCachePath(cachePath);
    } else {
        AUDIO_WARNING_LOG("while stream is null");
    }
}

int32_t AudioRendererPrivate::SetRendererWriteCallback(const std::shared_ptr<AudioRendererWriteCallback> &callback)
{
    return audioStream_->SetRendererWriteCallback(callback);
}

int32_t AudioRendererPrivate::SetRendererFirstFrameWritingCallback(
    const std::shared_ptr<AudioRendererFirstFrameWritingCallback> &callback)
{
    return audioStream_->SetRendererFirstFrameWritingCallback(callback);
}

void AudioRendererPrivate::SetInterruptMode(InterruptMode mode)
{
    AUDIO_INFO_LOG("InterruptMode %{public}d", mode);
    if (audioInterrupt_.mode == mode) {
        return;
    } else if (mode != SHARE_MODE && mode != INDEPENDENT_MODE) {
        AUDIO_ERR_LOG("Invalid interrupt mode!");
        return;
    }
    audioInterrupt_.mode = mode;
}

void AudioRendererPrivate::SetSilentModeAndMixWithOthers(bool on)
{
    Trace trace(std::string("AudioRenderer::SetSilentModeAndMixWithOthers:") + (on ? "on" : "off"));
    std::shared_lock<std::shared_mutex> sharedLockSwitch(rendererMutex_);
    std::lock_guard<std::mutex> lock(silentModeAndMixWithOthersMutex_);
    if (static_cast<RendererState>(audioStream_->GetState()) == RENDERER_RUNNING) {
        if (audioStream_->GetSilentModeAndMixWithOthers() && !on) {
            int32_t ret = AudioPolicyManager::GetInstance().ActivateAudioInterrupt(audioInterrupt_);
            CHECK_AND_RETURN_LOG(ret == 0, "ActivateAudioInterrupt Failed");
            audioStream_->SetSilentModeAndMixWithOthers(on);
            return;
        } else if (!audioStream_->GetSilentModeAndMixWithOthers() && on) {
            audioStream_->SetSilentModeAndMixWithOthers(on);
            int32_t ret = AudioPolicyManager::GetInstance().DeactivateAudioInterrupt(audioInterrupt_);
            CHECK_AND_RETURN_LOG(ret == 0, "DeactivateAudioInterrupt Failed");
            return;
        }
    }
    audioStream_->SetSilentModeAndMixWithOthers(on);
}

bool AudioRendererPrivate::GetSilentModeAndMixWithOthers()
{
    std::lock_guard<std::mutex> lock(silentModeAndMixWithOthersMutex_);
    return audioStream_->GetSilentModeAndMixWithOthers();
}

int32_t AudioRendererPrivate::SetParallelPlayFlag(bool parallelPlayFlag)
{
    AUDIO_PRERELEASE_LOGI("parallelPlayFlag %{public}d", parallelPlayFlag);
    audioInterrupt_.parallelPlayFlag = parallelPlayFlag;
    return SUCCESS;
}

int32_t AudioRendererPrivate::SetLowPowerVolume(float volume) const
{
    return audioStream_->SetLowPowerVolume(volume);
}

float AudioRendererPrivate::GetLowPowerVolume() const
{
    return audioStream_->GetLowPowerVolume();
}

int32_t AudioRendererPrivate::SetOffloadAllowed(bool isAllowed)
{
    AUDIO_PRERELEASE_LOGI("offload allowed: %{public}d", isAllowed);
    rendererInfo_.isOffloadAllowed = isAllowed;
    audioStream_->SetRendererInfo(rendererInfo_);
    return SUCCESS;
}

int32_t AudioRendererPrivate::SetOffloadMode(int32_t state, bool isAppBack) const
{
    AUDIO_INFO_LOG("set offload mode for session %{public}u", sessionID_);
    return audioStream_->SetOffloadMode(state, isAppBack);
}

int32_t AudioRendererPrivate::UnsetOffloadMode() const
{
    AUDIO_INFO_LOG("session %{public}u session unset offload", sessionID_);
    int32_t ret = audioStream_->UnsetOffloadMode();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "unset offload failed");
    return SUCCESS;
}

float AudioRendererPrivate::GetSingleStreamVolume() const
{
    return audioStream_->GetSingleStreamVolume();
}

float AudioRendererPrivate::GetMinStreamVolume() const
{
    return AudioPolicyManager::GetInstance().GetMinStreamVolume();
}

float AudioRendererPrivate::GetMaxStreamVolume() const
{
    return AudioPolicyManager::GetInstance().GetMaxStreamVolume();
}

int32_t AudioRendererPrivate::GetCurrentOutputDevices(AudioDeviceDescriptor &deviceInfo) const
{
    std::vector<std::unique_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    uint32_t sessionId = static_cast<uint32_t>(-1);
    int32_t ret = GetAudioStreamId(sessionId);
    CHECK_AND_RETURN_RET_LOG(!ret, ret, " Get sessionId failed");

    ret = AudioPolicyManager::GetInstance().GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    CHECK_AND_RETURN_RET_LOG(!ret, ret, "Get Current Renderer devices failed");

    for (auto it = audioRendererChangeInfos.begin(); it != audioRendererChangeInfos.end(); it++) {
        if ((*it)->sessionId == static_cast<int32_t>(sessionId)) {
            deviceInfo = (*it)->outputDeviceInfo;
        }
    }
    return SUCCESS;
}

uint32_t AudioRendererPrivate::GetUnderflowCount() const
{
    return audioStream_->GetUnderflowCount();
}


void AudioRendererPrivate::SetAudioRendererErrorCallback(std::shared_ptr<AudioRendererErrorCallback> errorCallback)
{
    std::shared_lock sharedLock(rendererMutex_);
    std::lock_guard lock(audioRendererErrCallbackMutex_);
    audioRendererErrorCallback_ = errorCallback;
}

int32_t AudioRendererPrivate::RegisterAudioPolicyServerDiedCb(const int32_t clientPid,
    const std::shared_ptr<AudioRendererPolicyServiceDiedCallback> &callback)
{
    AUDIO_INFO_LOG("RegisterAudioPolicyServerDiedCb client id: %{public}d", clientPid);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is null");

    std::lock_guard<std::mutex> lock(policyServiceDiedCallbackMutex_);

    policyServiceDiedCallback_ = callback;
    return AudioPolicyManager::GetInstance().RegisterAudioPolicyServerDiedCb(clientPid, callback);
}

int32_t AudioRendererPrivate::UnregisterAudioPolicyServerDiedCb(const int32_t clientPid)
{
    AUDIO_INFO_LOG("UnregisterAudioPolicyServerDiedCb client id: %{public}d", clientPid);
    return AudioPolicyManager::GetInstance().UnregisterAudioPolicyServerDiedCb(clientPid);
}

int32_t AudioRendererPrivate::RegisterOutputDeviceChangeWithInfoCallback(
    const std::shared_ptr<AudioRendererOutputDeviceChangeCallback> &callback)
{
    AUDIO_INFO_LOG("in");
    if (callback == nullptr) {
        AUDIO_ERR_LOG("callback is null");
        return ERR_INVALID_PARAM;
    }

    outputDeviceChangeCallback_->SaveCallback(callback);
    AUDIO_DEBUG_LOG("successful!");
    return SUCCESS;
}

int32_t AudioRendererPrivate::UnregisterOutputDeviceChangeWithInfoCallback()
{
    AUDIO_INFO_LOG("Unregister all");

    outputDeviceChangeCallback_->RemoveCallback();
    return SUCCESS;
}

int32_t AudioRendererPrivate::UnregisterOutputDeviceChangeWithInfoCallback(
    const std::shared_ptr<AudioRendererOutputDeviceChangeCallback> &callback)
{
    AUDIO_INFO_LOG("in");

    outputDeviceChangeCallback_->RemoveCallback(callback);
    return SUCCESS;
}

void AudioRendererPrivate::SetSwitchInfo(IAudioStream::SwitchInfo info, std::shared_ptr<IAudioStream> audioStream)
{
    CHECK_AND_RETURN_LOG(audioStream, "stream is nullptr");

    audioStream->SetStreamTrackerState(false);
    audioStream->SetApplicationCachePath(info.cachePath);
    audioStream->SetClientID(info.clientPid, info.clientUid, appInfo_.appTokenId, appInfo_.appFullTokenId);
    audioStream->SetPrivacyType(info.privacyType);
    audioStream->SetRendererInfo(info.rendererInfo);
    audioStream->SetCapturerInfo(info.capturerInfo);
    audioStream->SetAudioStreamInfo(info.params, rendererProxyObj_);
    audioStream->SetRenderMode(info.renderMode);
    audioStream->SetAudioEffectMode(info.effectMode);
    audioStream->SetVolume(info.volume);
    audioStream->SetUnderflowCount(info.underFlowCount);

    if (info.userSettedPreferredFrameSize.has_value()) {
        audioStream->SetPreferredFrameSize(info.userSettedPreferredFrameSize.value());
    }

    audioStream->SetSilentModeAndMixWithOthers(info.silentModeAndMixWithOthers);

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

    audioStream->SetStreamCallback(info.audioStreamCallback);
    audioStream->SetRendererWriteCallback(info.rendererWriteCallback);

    audioStream->SetRendererFirstFrameWritingCallback(info.rendererFirstFrameWritingCallback);
}

void AudioRendererPrivate::UpdateRendererAudioStream(const std::shared_ptr<IAudioStream> &audioStream)
{
    if (audioInterruptCallback_ != nullptr) {
        std::shared_ptr<AudioRendererInterruptCallbackImpl> interruptCbImpl =
            std::static_pointer_cast<AudioRendererInterruptCallbackImpl>(audioInterruptCallback_);
        interruptCbImpl->UpdateAudioStream(audioStream_);
    }
}

void AudioRendererPrivate::InitSwitchInfo(IAudioStream::StreamClass targetClass, IAudioStream::SwitchInfo &info)
{
    audioStream_->GetSwitchInfo(info);
    if (targetClass == IAudioStream::VOIP_STREAM) {
        info.rendererInfo.originalFlag = AUDIO_FLAG_VOIP_FAST;
    }

    if (rendererInfo_.rendererFlags == AUDIO_FLAG_VOIP_DIRECT) {
        info.rendererInfo.originalFlag = AUDIO_FLAG_VOIP_DIRECT;
        info.rendererInfo.rendererFlags = AUDIO_FLAG_VOIP_DIRECT;
        info.rendererFlags = AUDIO_FLAG_VOIP_DIRECT;
    } else if (rendererInfo_.rendererFlags == AUDIO_FLAG_DIRECT) {
        info.rendererInfo.pipeType = PIPE_TYPE_DIRECT_MUSIC;
        info.rendererFlags = AUDIO_FLAG_DIRECT;
    }
    info.params.originalSessionId = sessionID_;
    return;
}

bool AudioRendererPrivate::SwitchToTargetStream(IAudioStream::StreamClass targetClass, uint32_t &newSessionId,
    const AudioStreamDeviceChangeReasonExt reason)
{
    bool switchResult = false;
    if (audioStream_) {
        Trace trace("SwitchToTargetStream");
        std::shared_ptr<IAudioStream> oldAudioStream = nullptr;
        std::lock_guard<std::shared_mutex> lock(rendererMutex_);
        isSwitching_ = true;
        audioStream_->SetSwitchingStatus(true);
        AudioScopeExit scopeExit([this] () {
            audioStream_->SetSwitchingStatus(false);
        });
        RendererState previousState = GetStatus();
        AUDIO_INFO_LOG("Previous stream state: %{public}d, original sessionId: %{public}u", previousState, sessionID_);
        if (previousState == RENDERER_RUNNING) {
            CHECK_AND_RETURN_RET_LOG(audioStream_->StopAudioStream(), false, "StopAudioStream failed.");
        }
        IAudioStream::SwitchInfo info;
        InitSwitchInfo(targetClass, info);
        int64_t framesWritten = audioStream_->GetFramesWritten();
        if (framesWritten > 0) {
            framesAlreadyWritten_ += framesWritten;
            AUDIO_INFO_LOG("Frames already written: %{public}" PRId64 ", current stream value: %{public}" PRId64 ".",
                framesAlreadyWritten_, framesWritten);
        }

        switchResult = audioStream_->ReleaseAudioStream(true, true);
        std::shared_ptr<IAudioStream> newAudioStream = IAudioStream::GetPlaybackStream(targetClass, info.params,
            info.eStreamType, appInfo_.appPid);
        CHECK_AND_RETURN_RET_LOG(newAudioStream != nullptr, false, "SetParams GetPlayBackStream failed.");
        AUDIO_INFO_LOG("Get new stream success!");

        // set new stream info
        SetSwitchInfo(info, newAudioStream);

        CHECK_AND_RETURN_RET_LOG(switchResult, false, "release old stream failed.");

        if (previousState == RENDERER_RUNNING) {
            // restart audio stream
            switchResult = newAudioStream->StartAudioStream(CMD_FROM_CLIENT, reason);
            CHECK_AND_RETURN_RET_LOG(switchResult, false, "start new stream failed.");
        }
        oldAudioStream = audioStream_;
        audioStream_ = newAudioStream;
        UpdateRendererAudioStream(audioStream_);
        isFastRenderer_ = IAudioStream::IsFastStreamClass(targetClass);
        isSwitching_ = false;
        audioStream_->GetAudioSessionID(newSessionId);
        switchResult = true;
        SetDefaultOutputDevice(selectedDefaultOutputDevice_);
        scopeExit.Relase();
    }
    WriteSwitchStreamLogMsg();
    return switchResult;
}

void AudioRendererPrivate::WriteSwitchStreamLogMsg()
{
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::ModuleId::AUDIO, Media::MediaMonitor::EventId::AUDIO_PIPE_CHANGE,
        Media::MediaMonitor::EventType::BEHAVIOR_EVENT);
    bean->Add("CLIENT_UID", appInfo_.appUid);
    bean->Add("IS_PLAYBACK", 1);
    bean->Add("STREAM_TYPE", rendererInfo_.streamUsage);
    bean->Add("PIPE_TYPE_BEFORE_CHANGE", PIPE_TYPE_LOWLATENCY_OUT);
    bean->Add("PIPE_TYPE_AFTER_CHANGE", PIPE_TYPE_NORMAL_OUT);
    bean->Add("REASON", Media::MediaMonitor::DEVICE_CHANGE_FROM_FAST);
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

void AudioRendererPrivate::SwitchStream(const uint32_t sessionId, const int32_t streamFlag,
    const AudioStreamDeviceChangeReasonExt reason)
{
    IAudioStream::StreamClass targetClass = IAudioStream::PA_STREAM;
    switch (streamFlag) {
        case AUDIO_FLAG_NORMAL:
            rendererInfo_.rendererFlags = AUDIO_FLAG_NORMAL;
            targetClass = IAudioStream::PA_STREAM;
            break;
        case AUDIO_FLAG_MMAP:
            rendererInfo_.rendererFlags = AUDIO_FLAG_MMAP;
            targetClass = IAudioStream::FAST_STREAM;
            break;
        case AUDIO_FLAG_VOIP_FAST:
            rendererInfo_.rendererFlags = AUDIO_FLAG_VOIP_FAST;
            targetClass = IAudioStream::VOIP_STREAM;
            break;
        case AUDIO_FLAG_VOIP_DIRECT:
            rendererInfo_.rendererFlags = (isDirectVoipSupported_ && audioRenderMode_ == RENDER_MODE_CALLBACK) ?
                AUDIO_FLAG_VOIP_DIRECT : AUDIO_FLAG_NORMAL;
            targetClass = IAudioStream::PA_STREAM;
            break;
        case AUDIO_FLAG_DIRECT:
            rendererInfo_.rendererFlags = AUDIO_FLAG_DIRECT;
            break;
    }
    if (rendererInfo_.originalFlag == AUDIO_FLAG_FORCED_NORMAL) {
        rendererInfo_.rendererFlags = AUDIO_FLAG_NORMAL;
        targetClass = IAudioStream::PA_STREAM;
    }

    uint32_t newSessionId = 0;
    if (!SwitchToTargetStream(targetClass, newSessionId, reason)) {
        int32_t ret = AudioPolicyManager::GetInstance().DeactivateAudioInterrupt(audioInterrupt_);
        CHECK_AND_RETURN_LOG(ret == 0, "DeactivateAudioInterrupt Failed");
        if (audioRendererErrorCallback_) {
            audioRendererErrorCallback_->OnError(ERROR_SYSTEM);
        }
    }
    usedSessionId_.push_back(newSessionId);
    int32_t ret = AudioPolicyManager::GetInstance().RegisterDeviceChangeWithInfoCallback(newSessionId,
        outputDeviceChangeCallback_);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "Register device change callback for new session failed");
}

void OutputDeviceChangeWithInfoCallbackImpl::OnDeviceChangeWithInfo(
    const uint32_t sessionId, const AudioDeviceDescriptor &deviceInfo, const AudioStreamDeviceChangeReasonExt reason)
{
    AUDIO_INFO_LOG("OnRendererStateChange");
    std::vector<std::shared_ptr<AudioRendererOutputDeviceChangeCallback>> callbacks;

    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        callbacks = callbacks_;
    }

    for (auto &cb : callbacks) {
        if (cb != nullptr) {
            cb->OnOutputDeviceChange(deviceInfo, reason);
        }
    }

    AUDIO_INFO_LOG("sessionId: %{public}u, deviceType: %{public}d reason: %{public}d size: %{public}zu",
        sessionId, static_cast<int>(deviceInfo.deviceType_), static_cast<int>(reason), callbacks.size());
}

void OutputDeviceChangeWithInfoCallbackImpl::OnRecreateStreamEvent(const uint32_t sessionId, const int32_t streamFlag,
    const AudioStreamDeviceChangeReasonExt reason)
{
    std::lock_guard<std::mutex> lock(audioRendererObjMutex_);
    AUDIO_INFO_LOG("Enter, session id: %{public}d, stream flag: %{public}d", sessionId, streamFlag);
    CHECK_AND_RETURN_LOG(renderer_ != nullptr, "renderer_ is nullptr");
    renderer_->SwitchStream(sessionId, streamFlag, reason);
}

AudioEffectMode AudioRendererPrivate::GetAudioEffectMode() const
{
    return audioStream_->GetAudioEffectMode();
}

int64_t AudioRendererPrivate::GetFramesWritten() const
{
    return framesAlreadyWritten_ + audioStream_->GetFramesWritten();
}

int32_t AudioRendererPrivate::SetAudioEffectMode(AudioEffectMode effectMode) const
{
    return audioStream_->SetAudioEffectMode(effectMode);
}

int32_t AudioRendererPrivate::SetVolumeWithRamp(float volume, int32_t duration)
{
    AUDIO_INFO_LOG("volume:%{public}f duration:%{public}d", volume, duration);
    CHECK_AND_RETURN_RET(audioStream_ != nullptr, ERR_INVALID_PARAM, "Error status");
    return audioStream_->SetVolumeWithRamp(volume, duration);
}

void AudioRendererPrivate::SetPreferredFrameSize(int32_t frameSize)
{
    std::shared_lock<std::shared_mutex> lock(rendererMutex_);
    audioStream_->SetPreferredFrameSize(frameSize);
}

void AudioRendererPrivate::GetAudioInterrupt(AudioInterrupt &audioInterrupt)
{
    audioInterrupt = audioInterrupt_;
}

void AudioRendererPrivate::WriteUnderrunEvent() const
{
    AUDIO_INFO_LOG("AudioRendererPrivate WriteUnderrunEvent!");
    if (GetUnderflowCount() < WRITE_UNDERRUN_NUM) {
        return;
    }
    AudioPipeType pipeType = PIPE_TYPE_NORMAL_OUT;
    IAudioStream::StreamClass streamClass = audioStream_->GetStreamClass();
    if (streamClass == IAudioStream::FAST_STREAM) {
        pipeType = PIPE_TYPE_LOWLATENCY_OUT;
    } else if (streamClass == IAudioStream::PA_STREAM) {
        if (audioStream_->GetOffloadEnable()) {
            pipeType = PIPE_TYPE_OFFLOAD;
        } else if (audioStream_->GetSpatializationEnabled()) {
            pipeType = PIPE_TYPE_SPATIALIZATION;
        } else if (audioStream_->GetHighResolutionEnabled()) {
            pipeType = PIPE_TYPE_HIGHRESOLUTION;
        }
    }
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::ModuleId::AUDIO, Media::MediaMonitor::EventId::PERFORMANCE_UNDER_OVERRUN_STATS,
        Media::MediaMonitor::EventType::FREQUENCY_AGGREGATION_EVENT);
    bean->Add("IS_PLAYBACK", 1);
    bean->Add("CLIENT_UID", appInfo_.appUid);
    bean->Add("PIPE_TYPE", pipeType);
    bean->Add("STREAM_TYPE", rendererInfo_.streamUsage);
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

int32_t AudioRendererPrivate::RegisterRendererPolicyServiceDiedCallback()
{
    std::lock_guard<std::mutex> lock(rendererPolicyServiceDiedCbMutex_);
    AUDIO_DEBUG_LOG("RegisterRendererPolicyServiceDiedCallback");
    if (!audioPolicyServiceDiedCallback_) {
        audioPolicyServiceDiedCallback_ = std::make_shared<RendererPolicyServiceDiedCallback>();
        if (!audioPolicyServiceDiedCallback_) {
            AUDIO_ERR_LOG("Memory allocation failed!!");
            return ERROR;
        }
        AudioPolicyManager::GetInstance().RegisterAudioStreamPolicyServerDiedCb(audioPolicyServiceDiedCallback_);
        audioPolicyServiceDiedCallback_->SetAudioRendererObj(this);
        audioPolicyServiceDiedCallback_->SetAudioInterrupt(audioInterrupt_);
    }
    return SUCCESS;
}

int32_t AudioRendererPrivate::RemoveRendererPolicyServiceDiedCallback()
{
    std::lock_guard<std::mutex> lock(rendererPolicyServiceDiedCbMutex_);
    AUDIO_DEBUG_LOG("RemoveRendererPolicyServiceDiedCallback");
    if (audioPolicyServiceDiedCallback_) {
        int32_t ret = AudioPolicyManager::GetInstance().UnregisterAudioStreamPolicyServerDiedCb(
            audioPolicyServiceDiedCallback_);
        if (ret != 0) {
            AUDIO_ERR_LOG("RemoveRendererPolicyServiceDiedCallback failed");
            audioPolicyServiceDiedCallback_ = nullptr;
            return ERROR;
        }
    }
    audioPolicyServiceDiedCallback_ = nullptr;
    return SUCCESS;
}

RendererPolicyServiceDiedCallback::RendererPolicyServiceDiedCallback()
{
    AUDIO_DEBUG_LOG("RendererPolicyServiceDiedCallback create");
}

RendererPolicyServiceDiedCallback::~RendererPolicyServiceDiedCallback()
{
    AUDIO_DEBUG_LOG("RendererPolicyServiceDiedCallback destroy");
    if (restoreThread_ != nullptr && restoreThread_->joinable()) {
        restoreThread_->join();
        restoreThread_.reset();
        restoreThread_ = nullptr;
    }
}

void RendererPolicyServiceDiedCallback::SetAudioRendererObj(AudioRendererPrivate *rendererObj)
{
    renderer_ = rendererObj;
}

void RendererPolicyServiceDiedCallback::SetAudioInterrupt(AudioInterrupt &audioInterrupt)
{
    audioInterrupt_ = audioInterrupt;
}

void RendererPolicyServiceDiedCallback::OnAudioPolicyServiceDied()
{
    AUDIO_INFO_LOG("RendererPolicyServiceDiedCallback::OnAudioPolicyServiceDied");
    if (restoreThread_ != nullptr) {
        restoreThread_->detach();
    }
    restoreThread_ = std::make_unique<std::thread>([this] { this->RestoreTheadLoop(); });
    pthread_setname_np(restoreThread_->native_handle(), "OS_ARPSRestore");
}

void RendererPolicyServiceDiedCallback::RestoreTheadLoop()
{
    int32_t tryCounter = 10;
    uint32_t sleepTime = 300000;
    bool restoreResult = false;
    while (!restoreResult && tryCounter > 0) {
        tryCounter--;
        usleep(sleepTime);
        if (renderer_ == nullptr || renderer_->audioStream_ == nullptr || renderer_->abortRestore_) {
            AUDIO_INFO_LOG("abort restore");
            break;
        }
        renderer_->RestoreAudioInLoop(restoreResult, tryCounter);
    }
}

void AudioRendererPrivate::RestoreAudioInLoop(bool &restoreResult, int32_t &tryCounter)
{
    std::lock_guard<std::shared_mutex> lock(rendererMutex_);
    if (IsNoStreamRenderer()) {
        // no stream renderer don't need to restore stream
        restoreResult = audioStream_->RestoreAudioStream(false);
    } else {
        restoreResult = audioStream_->RestoreAudioStream();
        if (!restoreResult) {
            AUDIO_ERR_LOG("restore audio stream failed, %{public}d attempts remaining", tryCounter);
            return;
        }
        abortRestore_ = false;
    }

    if (GetStatus() == RENDERER_RUNNING) {
        GetAudioInterrupt(audioInterrupt_);
        int32_t ret = AudioPolicyManager::GetInstance().ActivateAudioInterrupt(audioInterrupt_);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("active audio interrupt failed");
        }
    }
    return;
}

int32_t AudioRendererPrivate::SetSpeed(float speed)
{
    AUDIO_INFO_LOG("set speed %{public}f", speed);
    CHECK_AND_RETURN_RET_LOG((speed >= MIN_STREAM_SPEED_LEVEL) && (speed <= MAX_STREAM_SPEED_LEVEL),
        ERR_INVALID_PARAM, "invaild speed index");
#ifdef SONIC_ENABLE
    audioStream_->SetSpeed(speed);
#endif
    speed_ = speed;
    return SUCCESS;
}

float AudioRendererPrivate::GetSpeed()
{
#ifdef SONIC_ENABLE
    return audioStream_->GetSpeed();
#endif
    return speed_;
}

bool AudioRendererPrivate::IsOffloadEnable()
{
    CHECK_AND_RETURN_RET_LOG(audioStream_ != nullptr, false, "audioStream_ is nullptr");
    bool enable = audioStream_->GetOffloadEnable();
    AUDIO_INFO_LOG("GetOffloadEnable is [%{public}s]", (enable ? "true" : "false"));
    return enable;
}

bool AudioRendererPrivate::IsFastRenderer()
{
    return isFastRenderer_;
}

void AudioRendererPrivate::InitLatencyMeasurement(const AudioStreamParams &audioStreamParams)
{
    latencyMeasEnabled_ = AudioLatencyMeasurement::CheckIfEnabled();
    AUDIO_INFO_LOG("LatencyMeas enabled in renderer:%{public}d", latencyMeasEnabled_);
    if (!latencyMeasEnabled_) {
        return;
    }
    std::string bundleName = AudioSystemManager::GetInstance()->GetSelfBundleName(appInfo_.appUid);
    uint32_t sessionId = 0;
    audioStream_->GetAudioSessionID(sessionId);
    latencyMeasurement_ = std::make_shared<AudioLatencyMeasurement>(audioStreamParams.samplingRate,
        audioStreamParams.channels, audioStreamParams.format, bundleName, sessionId);
}

void AudioRendererPrivate::MockPcmData(uint8_t *buffer, size_t bufferSize) const
{
    if (!latencyMeasEnabled_) {
        return;
    }
    if (latencyMeasurement_->MockPcmData(buffer, bufferSize)) {
        std::string timestamp = GetTime();
        audioStream_->UpdateLatencyTimestamp(timestamp, true);
    }
}

void AudioRendererPrivate::ActivateAudioConcurrency(const AudioStreamParams &audioStreamParams,
    const AudioStreamType &streamType, IAudioStream::StreamClass &streamClass)
{
    rendererInfo_.pipeType = PIPE_TYPE_NORMAL_OUT;
    if (rendererInfo_.streamUsage == STREAM_USAGE_VOICE_COMMUNICATION ||
        rendererInfo_.streamUsage == STREAM_USAGE_VOICE_MODEM_COMMUNICATION ||
        rendererInfo_.streamUsage == STREAM_USAGE_VIDEO_COMMUNICATION) {
        rendererInfo_.pipeType = PIPE_TYPE_CALL_OUT;
    } else if (streamClass == IAudioStream::FAST_STREAM) {
        rendererInfo_.pipeType = PIPE_TYPE_LOWLATENCY_OUT;
    } else {
        std::vector<sptr<AudioDeviceDescriptor>> deviceDescriptors =
            AudioPolicyManager::GetInstance().GetPreferredOutputDeviceDescriptors(rendererInfo_);
        if (!deviceDescriptors.empty() && deviceDescriptors[0] != nullptr) {
            if ((deviceDescriptors[0]->deviceType_ == DEVICE_TYPE_USB_HEADSET ||
                deviceDescriptors[0]->deviceType_ == DEVICE_TYPE_WIRED_HEADSET) &&
                streamType == STREAM_MUSIC && audioStreamParams.samplingRate >= SAMPLE_RATE_48000 &&
                audioStreamParams.format >= SAMPLE_S24LE) {
                rendererInfo_.pipeType = PIPE_TYPE_DIRECT_MUSIC;
            }
        }
    }
    int32_t ret = AudioPolicyManager::GetInstance().ActivateAudioConcurrency(rendererInfo_.pipeType);
    if (ret != SUCCESS) {
        if (streamClass == IAudioStream::FAST_STREAM) {
            streamClass = IAudioStream::PA_STREAM;
        }
        rendererInfo_.pipeType = PIPE_TYPE_NORMAL_OUT;
    }
    return;
}

void AudioRendererPrivate::ConcedeStream()
{
    AUDIO_INFO_LOG("session %{public}u concede from pipeType %{public}d", sessionID_, rendererInfo_.pipeType);
    uint32_t sessionId = static_cast<uint32_t>(-1);
    int32_t ret = GetAudioStreamId(sessionId);
    CHECK_AND_RETURN_LOG(!ret, "Get sessionId failed");

    AudioPipeType pipeType = PIPE_TYPE_NORMAL_OUT;
    audioStream_->GetAudioPipeType(pipeType);
    rendererInfo_.pipeType = PIPE_TYPE_NORMAL_OUT;
    rendererInfo_.isOffloadAllowed = false;
    audioStream_->SetRendererInfo(rendererInfo_);
    switch (pipeType) {
        case PIPE_TYPE_LOWLATENCY_OUT:
            // todo: fix wrong pipe type in server
            CHECK_AND_RETURN_LOG(audioStream_->GetStreamClass() != IAudioStream::PA_STREAM,
                "Session %{public}u is pa stream, no need for concede", sessionId);
        case PIPE_TYPE_DIRECT_MUSIC:
            SwitchStream(sessionId, IAudioStream::PA_STREAM, AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN);
            break;
        case PIPE_TYPE_OFFLOAD:
            UnsetOffloadMode();
            AudioPolicyManager::GetInstance().MoveToNewPipe(sessionId, PIPE_TYPE_NORMAL_OUT);
            break;
        default:
            break;
    }
}

void AudioRendererPrivate::EnableVoiceModemCommunicationStartStream(bool enable)
{
    isEnableVoiceModemCommunicationStartStream_ = enable;
}

bool AudioRendererPrivate::IsNoStreamRenderer() const
{
    return rendererInfo_.streamUsage == STREAM_USAGE_VOICE_MODEM_COMMUNICATION &&
        !isEnableVoiceModemCommunicationStartStream_;
}

int32_t AudioRendererPrivate::SetDefaultOutputDevice(DeviceType deviceType)
{
    if (deviceType != DEVICE_TYPE_EARPIECE && deviceType != DEVICE_TYPE_SPEAKER &&
        deviceType != DEVICE_TYPE_DEFAULT) {
        return ERR_NOT_SUPPORTED;
    }
    bool isSupportedStreamUsage = (find(AUDIO_DEFAULT_OUTPUT_DEVICE_SUPPORTED_STREAM_USAGES.begin(),
        AUDIO_DEFAULT_OUTPUT_DEVICE_SUPPORTED_STREAM_USAGES.end(), rendererInfo_.streamUsage) !=
        AUDIO_DEFAULT_OUTPUT_DEVICE_SUPPORTED_STREAM_USAGES.end());
    CHECK_AND_RETURN_RET_LOG(isSupportedStreamUsage, ERR_NOT_SUPPORTED, "stream usage not supported");
    selectedDefaultOutputDevice_ = deviceType;
    uint32_t currentSessionID = 0;
    audioStream_->GetAudioSessionID(currentSessionID);
    int32_t ret = AudioPolicyManager::GetInstance().SetDefaultOutputDevice(deviceType, currentSessionID,
        rendererInfo_.streamUsage, GetStatus() == RENDERER_RUNNING);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "select default output device failed");
    return SUCCESS;
}

// diffrence from GetAudioPosition only when set speed
int32_t AudioRendererPrivate::GetAudioTimestampInfo(Timestamp &timestamp, Timestamp::Timestampbase base) const
{
    CHECK_AND_RETURN_RET_LOG(audioStream_ != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    return audioStream_->GetAudioTimestampInfo(timestamp, base);
}
}  // namespace AudioStandard
}  // namespace OHOS
