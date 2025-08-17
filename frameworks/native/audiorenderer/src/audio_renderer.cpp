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
#define LOG_TAG "AudioRenderer"
#endif

#include <sstream>
#include <atomic>
#include <cinttypes>

#include "audio_renderer_private.h"
#include "shared_audio_renderer_wrapper.h"

#include "audio_renderer_log.h"
#include "audio_errors.h"
#include "audio_policy_manager.h"
#include "audio_speed.h"

#include "media_monitor_manager.h"
#include "audio_stream_descriptor.h"
#include "audio_scope_exit.h"
#include "volume_tools.h"

namespace OHOS {
namespace AudioStandard {

static const std::vector<StreamUsage> NEED_VERIFY_PERMISSION_STREAMS = {
    STREAM_USAGE_SYSTEM,
    STREAM_USAGE_DTMF,
    STREAM_USAGE_ENFORCED_TONE,
    STREAM_USAGE_ULTRASONIC,
    STREAM_USAGE_VOICE_MODEM_COMMUNICATION
};

const std::vector<StreamUsage> BACKGROUND_NOSTART_STREAM_USAGE {
    STREAM_USAGE_MUSIC,
    STREAM_USAGE_MOVIE,
    STREAM_USAGE_AUDIOBOOK
};
static constexpr uid_t UID_MSDP_SA = 6699;
static constexpr int32_t WRITE_UNDERRUN_NUM = 100;
static constexpr int32_t MINIMUM_BUFFER_SIZE_MSEC = 5;
static constexpr int32_t MAXIMUM_BUFFER_SIZE_MSEC = 60;
constexpr int32_t TIME_OUT_SECONDS = 10;
constexpr int32_t START_TIME_OUT_SECONDS = 15;
static constexpr uint32_t BLOCK_INTERRUPT_CALLBACK_IN_MS = 1000; // 1000ms
static constexpr float MIN_LOUDNESS_GAIN = -90.0;
static constexpr float MAX_LOUDNESS_GAIN = 24.0;
static constexpr int32_t UID_MEDIA = 1013;

static const std::map<AudioStreamType, StreamUsage> STREAM_TYPE_USAGE_MAP = {
    {STREAM_MUSIC, STREAM_USAGE_MUSIC},
    {STREAM_VOICE_CALL, STREAM_USAGE_VOICE_COMMUNICATION},
    {STREAM_VOICE_CALL_ASSISTANT, STREAM_USAGE_VOICE_CALL_ASSISTANT},
    {STREAM_VOICE_ASSISTANT, STREAM_USAGE_VOICE_ASSISTANT},
    {STREAM_ALARM, STREAM_USAGE_ALARM},
    {STREAM_VOICE_MESSAGE, STREAM_USAGE_VOICE_MESSAGE},
    {STREAM_RING, STREAM_USAGE_RINGTONE},
    {STREAM_NOTIFICATION, STREAM_USAGE_NOTIFICATION},
    {STREAM_ACCESSIBILITY, STREAM_USAGE_ACCESSIBILITY},
    {STREAM_SYSTEM, STREAM_USAGE_SYSTEM},
    {STREAM_MOVIE, STREAM_USAGE_MOVIE},
    {STREAM_GAME, STREAM_USAGE_GAME},
    {STREAM_SPEECH, STREAM_USAGE_AUDIOBOOK},
    {STREAM_NAVIGATION, STREAM_USAGE_NAVIGATION},
    {STREAM_DTMF, STREAM_USAGE_DTMF},
    {STREAM_SYSTEM_ENFORCED, STREAM_USAGE_ENFORCED_TONE},
    {STREAM_ULTRASONIC, STREAM_USAGE_ULTRASONIC},
    {STREAM_VOICE_RING, STREAM_USAGE_VOICE_RINGTONE},
};

static const std::map<AudioFlag, int32_t> OUTPUT_ROUTE_TO_STREAM_MAP = {
    {AUDIO_OUTPUT_FLAG_NORMAL, AUDIO_FLAG_NORMAL},
    {AUDIO_OUTPUT_FLAG_DIRECT, AUDIO_FLAG_DIRECT},
    {AUDIO_OUTPUT_FLAG_FAST, AUDIO_FLAG_MMAP},
};

static const std::map<uint32_t, IAudioStream::StreamClass> AUDIO_OUTPUT_FLAG_GROUP_MAP = {
    {AUDIO_OUTPUT_FLAG_NORMAL, IAudioStream::StreamClass::PA_STREAM},
    {AUDIO_OUTPUT_FLAG_DIRECT, IAudioStream::StreamClass::PA_STREAM},
    {AUDIO_OUTPUT_FLAG_MULTICHANNEL, IAudioStream::StreamClass::PA_STREAM},
    {AUDIO_OUTPUT_FLAG_LOWPOWER, IAudioStream::StreamClass::PA_STREAM},
    {AUDIO_OUTPUT_FLAG_FAST, IAudioStream::StreamClass::FAST_STREAM},
    {AUDIO_OUTPUT_FLAG_HWDECODING, IAudioStream::StreamClass::PA_STREAM},
};

static const std::vector<StreamUsage> AUDIO_DEFAULT_OUTPUT_DEVICE_SUPPORTED_STREAM_USAGES {
    STREAM_USAGE_VOICE_COMMUNICATION,
    STREAM_USAGE_VOICE_MESSAGE,
    STREAM_USAGE_VIDEO_COMMUNICATION,
    STREAM_USAGE_VOICE_MODEM_COMMUNICATION,
};

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

    std::shared_ptr<OutputDeviceChangeWithInfoCallbackImpl> outputDeviceChangeCallback = outputDeviceChangeCallback_;
    if (outputDeviceChangeCallback != nullptr) {
        outputDeviceChangeCallback->RemoveCallback();
        outputDeviceChangeCallback->UnsetAudioRendererObj();
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
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    AudioPolicyManager::GetInstance().GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    AUDIO_INFO_LOG("Audio current renderer change infos size: %{public}zu", audioRendererChangeInfos.size());
    int32_t maxRendererInstances = AudioPolicyManager::GetInstance().GetMaxRendererInstances();
    if (audioRendererChangeInfos.size() >= static_cast<size_t>(maxRendererInstances)) {
        std::map<int32_t, int32_t> appUseNumMap;
        int32_t INITIAL_VALUE = 1;
        int32_t mostAppUid = -1;
        int32_t mostAppNum = -1;
        for (auto it = audioRendererChangeInfos.begin(); it != audioRendererChangeInfos.end(); it++) {
            auto appUseNum = appUseNumMap.find((*it)->clientUID);
            if (appUseNum != appUseNumMap.end()) {
                appUseNumMap[(*it)->clientUID] = ++appUseNum->second;
            } else {
                appUseNumMap.emplace((*it)->clientUID, INITIAL_VALUE);
            }
        }
        for (auto iter = appUseNumMap.begin(); iter != appUseNumMap.end(); iter++) {
            if (iter->second > mostAppNum) {
                mostAppNum = iter->second;
                mostAppUid = iter->first;
            }
        }
        std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
            Media::MediaMonitor::ModuleId::AUDIO, Media::MediaMonitor::EventId::AUDIO_STREAM_EXHAUSTED_STATS,
            Media::MediaMonitor::EventType::FREQUENCY_AGGREGATION_EVENT);
        bean->Add("CLIENT_UID", mostAppUid);
        bean->Add("TIMES", mostAppNum);
        Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
    }

    CHECK_AND_RETURN_RET_LOG(audioRendererChangeInfos.size() < static_cast<size_t>(maxRendererInstances), ERR_OVERFLOW,
        "The current number of audio renderer streams is greater than the maximum number of configured instances");

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
        case SAMPLE_F32LE:
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

int32_t AudioRenderer::FadeInAudioBuffer(const BufferDesc &buffer, AudioSampleFormat format, AudioChannel channel)
{
    CHECK_AND_RETURN_RET_LOG(buffer.buffer != nullptr && buffer.bufLength != 0 && buffer.dataLength != 0,
        ERR_INVALID_PARAM, "Invalid buffer or length");
    BufferDesc tempBuffer = buffer;
    if (tempBuffer.bufLength > tempBuffer.dataLength) {
        AUDIO_INFO_LOG("less buffer case: bufLength: %{public}zu, dataLength : %{public}zu", tempBuffer.bufLength,
            tempBuffer.dataLength);
        tempBuffer.bufLength = tempBuffer.dataLength;
    }
    ChannelVolumes mapVols = VolumeTools::GetChannelVolumes(channel, 0.0f, 1.0f);
    int32_t volRet = VolumeTools::Process(tempBuffer, format, mapVols);
    CHECK_AND_RETURN_RET_LOG(volRet == SUCCESS, volRet, "Process Volume failed: %{public}d", volRet);
    return volRet;
}

int32_t AudioRenderer::FadeOutAudioBuffer(const BufferDesc &buffer, AudioSampleFormat format, AudioChannel channel)
{
    CHECK_AND_RETURN_RET_LOG(buffer.buffer != nullptr && buffer.bufLength != 0 && buffer.dataLength != 0,
        ERR_INVALID_PARAM, "Invalid buffer or length");
    BufferDesc tempBuffer = buffer;
    if (tempBuffer.bufLength > tempBuffer.dataLength) {
        AUDIO_INFO_LOG("less buffer case: bufLength: %{public}zu, dataLength : %{public}zu", tempBuffer.bufLength,
            tempBuffer.dataLength);
        tempBuffer.bufLength = tempBuffer.dataLength;
    }
    ChannelVolumes mapVols = VolumeTools::GetChannelVolumes(channel, 1.0f, 0.0f);
    int32_t volRet = VolumeTools::Process(tempBuffer, format, mapVols);
    CHECK_AND_RETURN_RET_LOG(volRet == SUCCESS, volRet, "Process Volume failed: %{public}d", volRet);
    return volRet;
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

    auto sharedRenderer = std::make_shared<AudioRendererPrivate>(audioStreamType, appInfo, true);
    CHECK_AND_RETURN_RET_LOG(sharedRenderer != nullptr, nullptr, "renderer is null");

    return std::make_unique<SharedAudioRendererWrapper>(sharedRenderer);
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
    auto sharedRenderer = CreateRenderer(rendererOptions, appInfo);
    CHECK_AND_RETURN_RET_LOG(sharedRenderer != nullptr, nullptr, "renderer is null");

    return std::make_unique<SharedAudioRendererWrapper>(sharedRenderer);
}

void AudioRendererPrivate::HandleSetRendererInfoByOptions(const AudioRendererOptions &rendererOptions,
    const AppInfo &appInfo)
{
    rendererInfo_.contentType = rendererOptions.rendererInfo.contentType;
    rendererInfo_.streamUsage = rendererOptions.rendererInfo.streamUsage;
    rendererInfo_.isSatellite = rendererOptions.rendererInfo.isSatellite;
    /* Set isOffloadAllowed during renderer creation when setOffloadAllowed is disabled. */
    rendererInfo_.isOffloadAllowed = GetFinalOffloadAllowed(rendererOptions.rendererInfo.isOffloadAllowed);
    rendererInfo_.playerType = rendererOptions.rendererInfo.playerType;
    rendererInfo_.expectedPlaybackDurationBytes
        = rendererOptions.rendererInfo.expectedPlaybackDurationBytes;
    rendererInfo_.samplingRate = rendererOptions.streamInfo.samplingRate;
    rendererInfo_.volumeMode = rendererOptions.rendererInfo.volumeMode;
    rendererInfo_.isLoopback = rendererOptions.rendererInfo.isLoopback;
    rendererInfo_.loopbackMode = rendererOptions.rendererInfo.loopbackMode;

    privacyType_ = rendererOptions.privacyType;
    strategy_ = rendererOptions.strategy;
    originalStrategy_ = rendererOptions.strategy;
}

bool AudioRendererPrivate::GetFinalOffloadAllowed(bool originalAllowed)
{
    if (getuid() == UID_MEDIA) {
        // Boot animation use avplayer, do not get bundle name to avoid increasing boot duration.
        std::string bundleName = AudioSystemManager::GetInstance()->GetSelfBundleName(appInfo_.appUid);
        if (bundleName == "mockNotOffloadHap") {
            AUDIO_INFO_LOG("Force set offload allowed to false for this stream");
            return false;
        }
    }
    return originalAllowed;
}

std::shared_ptr<AudioRenderer> AudioRenderer::CreateRenderer(const AudioRendererOptions &rendererOptions,
    const AppInfo &appInfo)
{
    Trace trace("KeyAction AudioRenderer::Create");
    std::lock_guard<std::mutex> lock(createRendererMutex_);
    CHECK_AND_RETURN_RET_LOG(AudioPolicyManager::GetInstance().GetAudioPolicyManagerProxy() != nullptr,
        nullptr, "sa not start");
    AudioStreamType audioStreamType = IAudioStream::GetStreamType(rendererOptions.rendererInfo.contentType,
        rendererOptions.rendererInfo.streamUsage);
    if (audioStreamType == STREAM_ULTRASONIC && getuid() != UID_MSDP_SA) {
        AudioRenderer::SendRendererCreateError(rendererOptions.rendererInfo.streamUsage, ERR_INVALID_PARAM);
        AUDIO_ERR_LOG("ULTRASONIC can only create by MSDP");
        return nullptr;
    }

    auto audioRenderer = std::make_shared<AudioRendererPrivate>(audioStreamType, appInfo, false);
    if (audioRenderer == nullptr) {
        AudioRenderer::SendRendererCreateError(rendererOptions.rendererInfo.streamUsage,
            ERR_OPERATION_FAILED);
    }
    CHECK_AND_RETURN_RET_LOG(audioRenderer != nullptr, nullptr, "Failed to create renderer object");

    int32_t rendererFlags = rendererOptions.rendererInfo.rendererFlags;
    bool isVirtualKeyboard = audioRenderer->IsVirtualKeyboard(rendererFlags);
    rendererFlags = rendererFlags == AUDIO_FLAG_VKB_NORMAL ? AUDIO_FLAG_NORMAL : rendererFlags;
    rendererFlags = rendererFlags == AUDIO_FLAG_VKB_FAST ? AUDIO_FLAG_MMAP : rendererFlags;

    AUDIO_INFO_LOG("StreamClientState for Renderer::Create. content: %{public}d, usage: %{public}d, "\
        "isOffloadAllowed: %{public}s, isVKB: %{public}s, flags: %{public}d, uid: %{public}d",
        rendererOptions.rendererInfo.contentType, rendererOptions.rendererInfo.streamUsage,
        rendererOptions.rendererInfo.isOffloadAllowed ? "T" : "F",
        isVirtualKeyboard ? "T" : "F", rendererFlags, appInfo.appUid);
    
    audioRenderer->rendererInfo_.isVirtualKeyboard = isVirtualKeyboard;
    audioRenderer->rendererInfo_.rendererFlags = rendererFlags;
    audioRenderer->rendererInfo_.originalFlag = rendererFlags;
    audioRenderer->HandleSetRendererInfoByOptions(rendererOptions, appInfo);
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
    audioInterrupt_.uid = appInfo_.appUid;
    audioInterrupt_.mode = SHARE_MODE;

    state_ = RENDERER_PREPARED;
}

// Inner function. Must be called with AudioRendererPrivate::rendererMutex_
// or AudioRendererPrivate::streamMutex_ held.
int32_t AudioRendererPrivate::InitAudioInterruptCallback(bool isRestoreAudio)
{
    AUDIO_DEBUG_LOG("in");

    if (audioInterrupt_.streamId != 0 && !isRestoreAudio) {
        AUDIO_INFO_LOG("old session already has interrupt, need to reset");
        (void)AudioPolicyManager::GetInstance().DeactivateAudioInterrupt(audioInterrupt_);
        (void)AudioPolicyManager::GetInstance().UnsetAudioInterruptCallback(audioInterrupt_.streamId);
    }

    CHECK_AND_RETURN_RET_LOG(audioInterrupt_.mode == SHARE_MODE || audioInterrupt_.mode == INDEPENDENT_MODE,
        ERR_INVALID_PARAM, "Invalid interrupt mode!");
    CHECK_AND_RETURN_RET_LOG(audioStream_->GetAudioSessionID(audioInterrupt_.streamId) == 0, ERR_INVALID_INDEX,
        "GetAudioSessionID failed");
    sessionID_ = audioInterrupt_.streamId;
    audioInterrupt_.streamUsage = rendererInfo_.streamUsage;
    audioInterrupt_.contentType = rendererInfo_.contentType;
    audioInterrupt_.sessionStrategy = strategy_;
    audioInterrupt_.api = rendererInfo_.playerType;

    AUDIO_INFO_LOG("interruptMode %{public}d, streamType %{public}d, sessionID %{public}d",
        audioInterrupt_.mode, audioInterrupt_.audioFocusType.streamType, audioInterrupt_.streamId);

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

    outputDeviceChangeCallback_->SetAudioRendererObj(weak_from_this());

    uint32_t sessionId;
    int32_t ret = GetAudioStreamIdInner(sessionId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Get sessionId failed");

    usedSessionId_.push_back(sessionId);
    ret = AudioPolicyManager::GetInstance().RegisterDeviceChangeWithInfoCallback(sessionId,
        outputDeviceChangeCallback_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Register failed");

    return SUCCESS;
}

void AudioRendererPrivate::InitAudioRouteCallback()
{
    audioRouteCallback_ = audioRouteCallback_ == nullptr ? std::make_shared<AudioRouteCallbackImpl>(weak_from_this()) :
        audioRouteCallback_;
    AUDIO_INFO_LOG("set audio route callback, sessionId: %{public}u", sessionID_);
    AudioPolicyManager::GetInstance().SetAudioRouteCallback(sessionID_, audioRouteCallback_, appInfo_.appUid);
}

// Inner function. Must be called with AudioRendererPrivate::rendererMutex_
// or AudioRendererPrivate::streamMutex_ held.
int32_t AudioRendererPrivate::InitAudioStream(AudioStreamParams audioStreamParams)
{
    Trace trace("AudioRenderer::InitAudioStream");
    rendererProxyObj_->SaveRendererObj(weak_from_this());
    audioStream_->SetRendererInfo(rendererInfo_);
    audioStream_->SetClientID(appInfo_.appPid, appInfo_.appUid, appInfo_.appTokenId, appInfo_.appFullTokenId);

    SetAudioPrivacyTypeInner(privacyType_);
    audioStream_->SetStreamTrackerState(false);

    int32_t ret = audioStream_->SetAudioStreamInfo(audioStreamParams, rendererProxyObj_);
    CHECK_AND_RETURN_RET_LOG(!ret, ret, "SetParams SetAudioStreamInfo Failed");

    ret = GetAudioStreamIdInner(sessionID_);
    CHECK_AND_RETURN_RET_LOG(!ret, ret, "GetAudioStreamId err");
    InitLatencyMeasurement(audioStreamParams);

    return SUCCESS;
}

int32_t AudioRendererPrivate::GetFrameCount(uint32_t &frameCount) const
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    return currentStream->GetFrameCount(frameCount);
}

int32_t AudioRendererPrivate::GetLatency(uint64_t &latency) const
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    return currentStream->GetLatency(latency);
}

void AudioRendererPrivate::SetAudioPrivacyType(AudioPrivacyType privacyType)
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_LOG(currentStream != nullptr, "audioStream_ is nullptr");
    privacyType_ = privacyType;
    currentStream->SetPrivacyType(privacyType);
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

    AUDIO_INFO_LOG("Preferred renderer flag: AUDIO_FLAG_NORMAL");
    rendererInfo_.rendererFlags = AUDIO_FLAG_NORMAL;
    return IAudioStream::PA_STREAM;
}

bool AudioRendererPrivate::IsDirectVoipParams(const AudioStreamParams &audioStreamParams)
{
    // VoIP derect only supports 16K and 48K sampling rate.
    if (!(audioStreamParams.samplingRate == SAMPLE_RATE_16000 ||
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

    // VoIP derect only supports 16bit, 32bit, 32float.
    if (!(audioStreamParams.format == SAMPLE_S16LE || audioStreamParams.format == SAMPLE_S32LE ||
        audioStreamParams.format == SAMPLE_F32LE)) {
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
    std::shared_lock<std::shared_mutex> lockShared;
    if (callbackLoopTid_ != gettid()) { // No need to add lock in callback thread to prevent deadlocks
        lockShared = std::shared_lock<std::shared_mutex>(rendererMutex_);
    }
    std::lock_guard<std::mutex> lock(setParamsMutex_);
    AudioStreamParams audioStreamParams = ConvertToAudioStreamParams(params);

    AudioStreamType audioStreamType = IAudioStream::GetStreamType(rendererInfo_.contentType, rendererInfo_.streamUsage);
#ifdef SUPPORT_LOW_LATENCY
    IAudioStream::StreamClass streamClass = GetPreferredStreamClass(audioStreamParams);
#else
    if (rendererInfo_.originalFlag != AUDIO_FLAG_PCM_OFFLOAD) {
        rendererInfo_.originalFlag = AUDIO_FLAG_NORMAL;
    }
    rendererInfo_.rendererFlags = AUDIO_FLAG_NORMAL;
    IAudioStream::StreamClass streamClass = IAudioStream::PA_STREAM;
#endif
    rendererInfo_.audioFlag = AUDIO_OUTPUT_FLAG_NORMAL;
    int32_t ret = PrepareAudioStream(audioStreamParams, audioStreamType, streamClass, rendererInfo_.audioFlag);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_INVALID_PARAM, "PrepareAudioStream failed");

    ret = InitAudioStream(audioStreamParams);
    if (ret != SUCCESS) {
        // if the normal stream creation fails, return fail, other try create normal stream
        CHECK_AND_RETURN_RET_LOG(streamClass != IAudioStream::PA_STREAM, ret, "Normal Stream Init Failed");
        ret = HandleCreateFastStreamError(audioStreamParams, audioStreamType);
    }

    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetAudioStreamInfo Failed");
    AUDIO_INFO_LOG("SetAudioStreamInfo Succeeded");

    RegisterRendererPolicyServiceDiedCallback();
    // eg: 100005_44100_2_1_client_in.pcm
    std::string dumpFileName = std::to_string(sessionID_) + "_" + std::to_string(params.sampleRate) + "_" +
        std::to_string(params.channelCount) + "_" + std::to_string(params.sampleFormat) + "_client_in.pcm";
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_CLIENT_PARA, dumpFileName, &dumpFile_);

    ret = InitOutputDeviceChangeCallback();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "InitOutputDeviceChangeCallback Failed");

    ret = InitFormatUnsupportedErrorCallback();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "InitFormatUnsupportedErrorCallback Failed");

    InitAudioRouteCallback();

    return InitAudioInterruptCallback();
}

int32_t AudioRendererPrivate::PrepareAudioStream(AudioStreamParams &audioStreamParams,
    const AudioStreamType &audioStreamType, IAudioStream::StreamClass &streamClass, uint32_t &flag)
{
    AUDIO_INFO_LOG("Create stream with flag: %{public}d, original flag: %{public}d, streamClass: %{public}d",
        rendererInfo_.rendererFlags, rendererInfo_.originalFlag, streamClass);

    // Create Client
    std::shared_ptr<AudioStreamDescriptor> streamDesc = ConvertToStreamDescriptor(audioStreamParams);
    flag = AUDIO_OUTPUT_FLAG_NORMAL;

    std::string networkId = LOCAL_NETWORK_ID;
    int32_t ret = AudioPolicyManager::GetInstance().CreateRendererClient(
        streamDesc, flag, audioStreamParams.originalSessionId, networkId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "CreateRendererClient failed");
    AUDIO_INFO_LOG("StreamClientState for Renderer::CreateClient. id %{public}u, flag: %{public}u",
        audioStreamParams.originalSessionId, flag);

    SetClientInfo(flag, streamClass);

    if (audioStream_ == nullptr) {
        audioStream_ = IAudioStream::GetPlaybackStream(streamClass, audioStreamParams, audioStreamType,
            appInfo_.appUid);
        CHECK_AND_RETURN_RET_LOG(audioStream_ != nullptr, ERR_INVALID_PARAM, "SetParams GetPlayBackStream failed.");
        AUDIO_INFO_LOG("IAudioStream::GetStream success");
        isFastRenderer_ = IAudioStream::IsFastStreamClass(streamClass);
        audioStream_->NotifyRouteUpdate(flag, networkId);
    }
    return SUCCESS;
}

std::shared_ptr<AudioStreamDescriptor> AudioRendererPrivate::ConvertToStreamDescriptor(
    const AudioStreamParams &audioStreamParams)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->streamInfo_.format = static_cast<AudioSampleFormat>(audioStreamParams.format);
    streamDesc->streamInfo_.samplingRate = static_cast<AudioSamplingRate>(audioStreamParams.samplingRate);
    streamDesc->streamInfo_.channels = static_cast<AudioChannel>(audioStreamParams.channels);
    streamDesc->streamInfo_.encoding = static_cast<AudioEncodingType>(audioStreamParams.encoding);
    streamDesc->streamInfo_.channelLayout = static_cast<AudioChannelLayout>(audioStreamParams.channelLayout);
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->createTimeStamp_ = ClockTime::GetCurNano();
    streamDesc->rendererInfo_ = rendererInfo_;
    streamDesc->appInfo_ = appInfo_;
    streamDesc->callerUid_ = static_cast<int32_t>(getuid());
    streamDesc->callerPid_ = static_cast<int32_t>(getpid());
    streamDesc->sessionId_ = audioStreamParams.originalSessionId;
    return streamDesc;
}

void AudioRendererPrivate::SetClientInfo(uint32_t flag, IAudioStream::StreamClass &streamClass)
{
    if (flag & AUDIO_OUTPUT_FLAG_FAST) {
        if (flag & AUDIO_OUTPUT_FLAG_VOIP) {
            streamClass = IAudioStream::StreamClass::VOIP_STREAM;
            rendererInfo_.originalFlag = AUDIO_FLAG_VOIP_FAST;
            rendererInfo_.rendererFlags = AUDIO_FLAG_VOIP_FAST;
            rendererInfo_.pipeType = PIPE_TYPE_LOWLATENCY_OUT;
        } else {
            streamClass = IAudioStream::StreamClass::FAST_STREAM;
            rendererInfo_.rendererFlags = AUDIO_FLAG_VOIP_FAST;
            rendererInfo_.pipeType = PIPE_TYPE_LOWLATENCY_OUT;
        }
    } else if (flag & AUDIO_OUTPUT_FLAG_DIRECT) {
        if (flag & AUDIO_OUTPUT_FLAG_VOIP) {
            streamClass = IAudioStream::StreamClass::PA_STREAM;
            rendererInfo_.originalFlag = AUDIO_FLAG_VOIP_DIRECT;
            rendererInfo_.rendererFlags = AUDIO_FLAG_VOIP_DIRECT;
            rendererInfo_.pipeType = PIPE_TYPE_CALL_OUT;
        } else {
            streamClass = IAudioStream::StreamClass::PA_STREAM;
            rendererInfo_.rendererFlags = AUDIO_FLAG_DIRECT;
            rendererInfo_.pipeType = PIPE_TYPE_DIRECT_OUT;
        }
    } else if (flag & AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD) {
        streamClass = IAudioStream::StreamClass::PA_STREAM;
        rendererInfo_.rendererFlags = AUDIO_FLAG_NORMAL;
        rendererInfo_.pipeType = PIPE_TYPE_OFFLOAD;
    } else if (flag & AUDIO_OUTPUT_FLAG_MULTICHANNEL) {
        streamClass = IAudioStream::StreamClass::PA_STREAM;
        rendererInfo_.rendererFlags = AUDIO_FLAG_NORMAL;
        rendererInfo_.pipeType = PIPE_TYPE_MULTICHANNEL;
    } else {
        streamClass = IAudioStream::StreamClass::PA_STREAM;
        rendererInfo_.rendererFlags = AUDIO_FLAG_NORMAL;
        rendererInfo_.pipeType = PIPE_TYPE_NORMAL_OUT;
    }
    AUDIO_INFO_LOG("Route flag: %{public}u, streamClass: %{public}d, rendererFlag: %{public}d, pipeType: %{public}d",
        flag, streamClass, rendererInfo_.rendererFlags, rendererInfo_.pipeType);
}

int32_t AudioRendererPrivate::GetParams(AudioRendererParams &params) const
{
    AudioStreamParams audioStreamParams;
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    int32_t result = currentStream->GetAudioStreamInfo(audioStreamParams);
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
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    int32_t result = currentStream->GetAudioStreamInfo(audioStreamParams);
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
    std::shared_lock<std::shared_mutex> lockShared;
    if (callbackLoopTid_ != gettid()) { // No need to add lock in callback thread to prevent deadlocks
        lockShared = std::shared_lock<std::shared_mutex>(rendererMutex_);
    }
    std::lock_guard<std::mutex> lock(setStreamCallbackMutex_);
    // If the client is using the deprecated SetParams API. SetRendererCallback must be invoked, after SetParams.
    // In general, callbacks can only be set after the renderer state is PREPARED.
    RendererState state = GetStatusInner();
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
        audioStreamCallback_ = std::make_shared<AudioStreamCallbackRenderer>(weak_from_this());
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
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    currentStream->SetRendererPositionCallback(markPosition, callback);

    return SUCCESS;
}

void AudioRendererPrivate::UnsetRendererPositionCallback()
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_LOG(currentStream != nullptr, "audioStream_ is nullptr");
    currentStream->UnsetRendererPositionCallback();
}

int32_t AudioRendererPrivate::SetRendererPeriodPositionCallback(int64_t frameNumber,
    const std::shared_ptr<RendererPeriodPositionCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG((callback != nullptr) && (frameNumber > 0), ERR_INVALID_PARAM,
        "input param is invalid");
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    currentStream->SetRendererPeriodPositionCallback(frameNumber, callback);

    return SUCCESS;
}

void AudioRendererPrivate::UnsetRendererPeriodPositionCallback()
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_LOG(currentStream != nullptr, "audioStream_ is nullptr");
    currentStream->UnsetRendererPeriodPositionCallback();
}

bool AudioRendererPrivate::IsAllowedStartBackgroud()
{
    bool ret = AudioPolicyManager::GetInstance().IsAllowedPlayback(appInfo_.appUid, appInfo_.appPid);
    if (ret) {
        AUDIO_INFO_LOG("AVSession IsAudioPlaybackAllowed is: %{public}d", ret);
        return ret;
    } else {
        if (std::count(BACKGROUND_NOSTART_STREAM_USAGE.begin(), BACKGROUND_NOSTART_STREAM_USAGE.end(),
            rendererInfo_.streamUsage) == 0) {
            AUDIO_INFO_LOG("%{public}d is BACKGROUND_NOSTART_STREAM_USAGE", rendererInfo_.streamUsage);
            return true;
        }
    }
    return ret;
}

// Must be called with rendererMutex_ or streamMutex_ held.
bool AudioRendererPrivate::GetStartStreamResult(StateChangeCmdType cmdType)
{
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

std::shared_ptr<IAudioStream> AudioRendererPrivate::GetInnerStream() const
{
    std::shared_lock<std::shared_mutex> lockShared;
    if (callbackLoopTid_ != gettid()) { // No need to add lock in callback thread to prevent deadlocks
        lockShared = std::shared_lock<std::shared_mutex>(rendererMutex_);
    }
    return audioStream_;
}

int32_t AudioRendererPrivate::StartSwitchProcess(RestoreInfo &restoreInfo, IAudioStream::StreamClass &targetClass,
    std::string callingFunc)
{
    // hold rendererMutex_ to avoid render control and switch process called in concurrency
    std::unique_lock<std::shared_mutex> lock;
    if (callbackLoopTid_ != gettid()) { // No need to add lock in callback thread to prevent deadlocks
        lock = std::unique_lock<std::shared_mutex>(rendererMutex_);
        CHECK_AND_RETURN_RET_LOG(releaseFlag_ == false, SUCCESS, "In renderer release, return");
    }

    // Block interrupt calback, avoid pausing wrong stream.
    std::shared_ptr<AudioRendererInterruptCallbackImpl> interruptCbImpl = nullptr;
    if (audioInterruptCallback_ != nullptr) {
        interruptCbImpl = std::static_pointer_cast<AudioRendererInterruptCallbackImpl>(audioInterruptCallback_);
        interruptCbImpl->StartSwitch();
    }

    FastStatus fastStatus = GetFastStatusInner();
    // Switch to target audio stream. Deactivate audio interrupt if switch failed.
    AUDIO_INFO_LOG("Before %{public}s, restore audiorenderer %{public}u", callingFunc.c_str(), sessionID_);
    if (!SwitchToTargetStream(targetClass, restoreInfo)) {
        if (audioRendererErrorCallback_) {
            audioRendererErrorCallback_->OnError(ERROR_SYSTEM); // Notify app if switch failed.
        }
        AUDIO_INFO_LOG("Deactivate audio interrupt after switch to target stream");
        AudioInterrupt audioInterrupt = audioInterrupt_;
        int32_t ret = AudioPolicyManager::GetInstance().DeactivateAudioInterrupt(audioInterrupt);
        if (ret != SUCCESS) {
            if (interruptCbImpl) {
                interruptCbImpl->FinishSwitch();
            }
            AUDIO_ERR_LOG("DeactivateAudioInterrupt Failed");
            return ERR_OPERATION_FAILED;
        }
    } else {
        FastStatusChangeCallback(fastStatus);
    }

    // Unblock interrupt callback.
    if (interruptCbImpl) {
        interruptCbImpl->FinishSwitch();
    }
    return SUCCESS;
}

int32_t AudioRendererPrivate::CheckAndRestoreAudioRenderer(std::string callingFunc)
{
    RestoreInfo restoreInfo;
    std::shared_ptr<IAudioStream> oldStream = nullptr;
    IAudioStream::StreamClass targetClass = IAudioStream::PA_STREAM;
    {
        std::unique_lock<std::shared_mutex> lock;
        if (callbackLoopTid_ != gettid()) { // No need to add lock in callback thread to prevent deadlocks
            lock = std::unique_lock<std::shared_mutex>(rendererMutex_);
            CHECK_AND_RETURN_RET_LOG(releaseFlag_ == false, SUCCESS, "In renderer release, return");
        }

        // Return in advance if there's no need for restore.
        CHECK_AND_RETURN_RET_LOG(audioStream_, ERR_ILLEGAL_STATE, "audioStream_ is nullptr");
        RestoreStatus restoreStatus = audioStream_->CheckRestoreStatus();
        if (abortRestore_ || restoreStatus == NO_NEED_FOR_RESTORE) {
            return SUCCESS;
        }
        if (restoreStatus == RESTORING) {
            AUDIO_WARNING_LOG("%{public}s when restoring, return", callingFunc.c_str());
            return ERR_ILLEGAL_STATE;
        }

        // Get restore info and target stream class for switching.
        audioStream_->GetRestoreInfo(restoreInfo);
        SetClientInfo(restoreInfo.routeFlag, targetClass);
        if (restoreStatus == NEED_RESTORE_TO_NORMAL) {
            restoreInfo.targetStreamFlag = AUDIO_FLAG_FORCED_NORMAL;
        }
        // Check if split stream. If true, fetch output device and return.
        CHECK_AND_RETURN_RET(ContinueAfterSplit(restoreInfo), true, "Stream split");
        // Check if continue to switch after some concede operation.
        CHECK_AND_RETURN_RET_LOG(ContinueAfterConcede(targetClass, restoreInfo),
            true, "No need for switch");
        oldStream = audioStream_;
    }
    // ahead join callbackLoop and do not hold rendererMutex_ when waiting for callback
    oldStream->JoinCallbackLoop();

    return StartSwitchProcess(restoreInfo, targetClass, callingFunc);
}

int32_t AudioRendererPrivate::AsyncCheckAudioRenderer(std::string callingFunc)
{
    if (switchStreamInNewThreadTaskCount_.fetch_add(1) > 0) {
        return SUCCESS;
    }
    auto weakRenderer = weak_from_this();
    taskLoop_.PostTask([weakRenderer, callingFunc] () {
        auto sharedRenderer = weakRenderer.lock();
        CHECK_AND_RETURN_LOG(sharedRenderer, "render is null");
        uint32_t taskCount;
        do {
            taskCount = sharedRenderer->switchStreamInNewThreadTaskCount_.load();
            sharedRenderer->CheckAudioRenderer(callingFunc + "withNewThread");
        } while (sharedRenderer->switchStreamInNewThreadTaskCount_.fetch_sub(taskCount) > taskCount);
    });
    return SUCCESS;
}

bool AudioRendererPrivate::Start(StateChangeCmdType cmdType)
{
    Trace trace("KeyAction AudioRenderer::Start " + std::to_string(sessionID_));
    AsyncCheckAudioRenderer("Start");
    AudioXCollie audioXCollie("AudioRendererPrivate::Start", START_TIME_OUT_SECONDS,
        [](void *) { AUDIO_ERR_LOG("Start timeout"); }, nullptr, AUDIO_XCOLLIE_FLAG_LOG);

    std::unique_lock<std::shared_mutex> lock;
    if (callbackLoopTid_ != gettid()) { // No need to add lock in callback thread to prevent deadlocks
        lock = std::unique_lock<std::shared_mutex>(rendererMutex_);
    }
    AUDIO_WARNING_LOG("StreamClientState for Renderer::Start. id: %{public}u, streamType: %{public}d, "\
        "volume: %{public}f, interruptMode: %{public}d, isVKB: %{public}s",
        sessionID_, audioInterrupt_.audioFocusType.streamType,
        GetVolumeInner(), audioInterrupt_.mode, rendererInfo_.isVirtualKeyboard ? "T" : "F");
    CHECK_AND_RETURN_RET_LOG(IsAllowedStartBackgroud(), false, "Start failed. IsAllowedStartBackgroud is false");
    RendererState state = GetStatusInner();
    CHECK_AND_RETURN_RET_LOG((state == RENDERER_PREPARED) || (state == RENDERER_STOPPED) || (state == RENDERER_PAUSED),
        false, "Start failed. Illegal state:%{public}u", state);

    CHECK_AND_RETURN_RET_LOG(!isSwitching_, false, "Start failed. Switching state: %{public}d", isSwitching_);

    if (audioInterrupt_.audioFocusType.streamType == STREAM_DEFAULT || audioInterrupt_.streamId == INVALID_SESSION_ID) {
        return false;
    }

    CHECK_AND_RETURN_RET_LOG(audioStream_ != nullptr, false, "audio stream is null");

    float duckVolume = audioStream_->GetDuckVolume();
    bool isMute = audioStream_->GetMute();
    AUDIO_WARNING_LOG("VolumeInfo for Renderer::Start. duckVolume: %{public}f, isMute: %{public}d, MinStreamVolume:"\
        "MinStreamVolume: %{public}f, MaxStreamVolume: %{public}f",
        duckVolume, isMute, GetMinStreamVolume(), GetMaxStreamVolume());

    if ((GetVolumeInner() == 0 && isStillZeroStreamVolume_) || isMute) {
        AUDIO_INFO_LOG("StreamClientState for Renderer::Start. volume=%{public}f, isStillZeroStreamVolume_=%{public}d"
            " isMute=%{public}d", GetVolumeInner(), isStillZeroStreamVolume_, isMute);
        audioInterrupt_.sessionStrategy.concurrencyMode = AudioConcurrencyMode::SILENT;
    } else {
        isStillZeroStreamVolume_ = false;
    }

    {
        std::lock_guard<std::mutex> lockSilentMode(silentModeAndMixWithOthersMutex_);
        if (audioStream_->GetSilentModeAndMixWithOthers()) {
            audioInterrupt_.sessionStrategy.concurrencyMode = AudioConcurrencyMode::SILENT;
        }
        int32_t ret = AudioPolicyManager::GetInstance().ActivateAudioInterrupt(audioInterrupt_);
        CHECK_AND_RETURN_RET_LOG(ret == 0, false, "ActivateAudioInterrupt Failed");
    }

    if (IsNoStreamRenderer()) {
        // no stream renderer only need to activate audio interrupt
        state_ = RENDERER_RUNNING;
        return true;
    }

    return GetStartStreamResult(cmdType);
}

int32_t AudioRendererPrivate::Write(uint8_t *buffer, size_t bufferSize)
{
    Trace trace("AudioRenderer::Write");
    AsyncCheckAudioRenderer("Write");
    MockPcmData(buffer, bufferSize);
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    int32_t size = currentStream->Write(buffer, bufferSize);
    if (size > 0) {
        DumpFileUtil::WriteDumpFile(dumpFile_, static_cast<void *>(buffer), size);
    }
    return size;
}

int32_t AudioRendererPrivate::Write(uint8_t *pcmBuffer, size_t pcmSize, uint8_t *metaBuffer, size_t metaSize)
{
    Trace trace("Write");
    AsyncCheckAudioRenderer("Write");
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    int32_t size = currentStream->Write(pcmBuffer, pcmSize, metaBuffer, metaSize);
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
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, RENDERER_INVALID, "audioStream_ is nullptr");
    return static_cast<RendererState>(currentStream->GetState());
}

bool AudioRendererPrivate::GetAudioTime(Timestamp &timestamp, Timestamp::Timestampbase base) const
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    return currentStream->GetAudioTime(timestamp, base);
}

bool AudioRendererPrivate::GetAudioPosition(Timestamp &timestamp, Timestamp::Timestampbase base)
{
    AsyncCheckAudioRenderer("GetAudioPosition");
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    return currentStream->GetAudioPosition(timestamp, base);
}

bool AudioRendererPrivate::Drain() const
{
    Trace trace("KeyAction AudioRenderer::Drain");
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    return currentStream->DrainAudioStream();
}

bool AudioRendererPrivate::Flush() const
{
    Trace trace("KeyAction AudioRenderer::Flush " + std::to_string(sessionID_));
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    return currentStream->FlushAudioStream();
}

bool AudioRendererPrivate::PauseTransitent(StateChangeCmdType cmdType)
{
    Trace trace("KeyAction AudioRenderer::PauseTransitent " + std::to_string(sessionID_));
    std::unique_lock<std::shared_mutex> lock;
    if (callbackLoopTid_ != gettid()) { // No need to add lock in callback thread to prevent deadlocks
        lock = std::unique_lock<std::shared_mutex>(rendererMutex_);
    }
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

    RendererState state = GetStatusInner();
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

bool AudioRendererPrivate::Mute(StateChangeCmdType cmdType) const
{
    Trace trace("AudioRenderer::Mute");
    std::shared_lock<std::shared_mutex> lock;
    if (callbackLoopTid_ != gettid()) { // No need to add lock in callback thread to prevent deadlocks
        lock = std::shared_lock<std::shared_mutex>(rendererMutex_);
    }
    AUDIO_INFO_LOG("StreamClientState for Renderer::Mute. id: %{public}u", sessionID_);
    (void)audioStream_->SetMute(true, cmdType);
    return true;
}

bool AudioRendererPrivate::Unmute(StateChangeCmdType cmdType) const
{
    Trace trace("AudioRenderer::Unmute");
    std::shared_lock<std::shared_mutex> lock;
    if (callbackLoopTid_ != gettid()) { // No need to add lock in callback thread to prevent deadlocks
        lock = std::shared_lock<std::shared_mutex>(rendererMutex_);
    }
    AUDIO_INFO_LOG("StreamClientState for Renderer::Unmute. id: %{public}u", sessionID_);
    (void)audioStream_->SetMute(false, cmdType);
    UpdateAudioInterruptStrategy(GetVolumeInner(), false);
    return true;
}

bool AudioRendererPrivate::Pause(StateChangeCmdType cmdType)
{
    Trace trace("KeyAction AudioRenderer::Pause " + std::to_string(sessionID_));
    AudioXCollie audioXCollie("AudioRenderer::Pause", TIME_OUT_SECONDS,
        [](void *) {
            AUDIO_ERR_LOG("Pause timeout");
        }, nullptr, AUDIO_XCOLLIE_FLAG_LOG);
    std::unique_lock<std::shared_mutex> lock;
    if (callbackLoopTid_ != gettid()) { // No need to add lock in callback thread to prevent deadlocks
        lock = std::unique_lock<std::shared_mutex>(rendererMutex_);
    }
    AUDIO_WARNING_LOG("StreamClientState for Renderer::Pause. id: %{public}u", sessionID_);

    CHECK_AND_RETURN_RET_LOG(!isSwitching_, false, "Pause failed. Switching state: %{public}d", isSwitching_);

    if (IsNoStreamRenderer()) {
        // When the cellular call stream is pausing, only need to deactivate audio interrupt.
        if (AudioPolicyManager::GetInstance().DeactivateAudioInterrupt(audioInterrupt_) != 0) {
            AUDIO_ERR_LOG("DeactivateAudioInterrupt Failed");
        }
        state_ = RENDERER_PAUSED;
        return true;
    }

    RendererState state = GetStatusInner();
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
    Trace trace("KeyAction AudioRenderer::Stop " + std::to_string(sessionID_));
    AUDIO_WARNING_LOG("StreamClientState for Renderer::Stop. id: %{public}u", sessionID_);
    std::unique_lock<std::shared_mutex> lock;
    if (callbackLoopTid_ != gettid()) { // No need to add lock in callback thread to prevent deadlocks
        lock = std::unique_lock<std::shared_mutex>(rendererMutex_);
    }
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

void AudioRendererPrivate::SetReleaseFlagWithLock(bool releaseFlag)
{
    std::unique_lock<std::shared_mutex> lock(rendererMutex_);
    releaseFlag_ = releaseFlag;
}

void AudioRendererPrivate::SetReleaseFlagNoLock(bool releaseFlag)
{
    releaseFlag_ = releaseFlag;
}

bool AudioRendererPrivate::Release()
{
    Trace trace("KeyAction AudioRenderer::Release " + std::to_string(sessionID_));

    std::unique_lock<std::shared_mutex> lock;
    if (callbackLoopTid_ != gettid()) { // No need to add lock in callback thread to prevent deadlocks
        SetReleaseFlagWithLock(true);
        auto audioStreamInner = GetInnerStream();
        if (audioStreamInner != nullptr) {
            audioStreamInner->JoinCallbackLoop();
        }
        lock = std::unique_lock<std::shared_mutex>(rendererMutex_);
    }
    AUDIO_WARNING_LOG("StreamClientState for Renderer::Release. id: %{public}u", sessionID_);

    abortRestore_ = true;
    if (audioStream_ == nullptr) {
        AUDIO_ERR_LOG("audioStream is null");
        SetReleaseFlagNoLock(false);
        return true;
    }
    bool result = audioStream_->ReleaseAudioStream();

    // If Stop call was skipped, Release to take care of Deactivation
    (void)AudioPolicyManager::GetInstance().DeactivateAudioInterrupt(audioInterrupt_);

    // Unregister the callaback in policy server
    (void)AudioPolicyManager::GetInstance().UnsetAudioInterruptCallback(sessionID_);

    (void)AudioPolicyManager::GetInstance().UnsetAudioFormatUnsupportedErrorCallback();

    (void)AudioPolicyManager::GetInstance().UnsetAudioRouteCallback(sessionID_);

    for (auto id : usedSessionId_) {
        AudioPolicyManager::GetInstance().UnregisterDeviceChangeWithInfoCallback(id);
    }
    SetReleaseFlagNoLock(false);
    lock.unlock();
    RemoveRendererPolicyServiceDiedCallback();

    return result;
}

int32_t AudioRendererPrivate::GetBufferSize(size_t &bufferSize) const
{
    Trace trace("AudioRenderer::GetBufferSize");
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    return currentStream->GetBufferSize(bufferSize);
}

int32_t AudioRendererPrivate::GetAudioStreamId(uint32_t &sessionID) const
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    return currentStream->GetAudioSessionID(sessionID);
}

int32_t AudioRendererPrivate::SetAudioRendererDesc(AudioRendererDesc audioRendererDesc)
{
    ContentType contentType = audioRendererDesc.contentType;
    StreamUsage streamUsage = audioRendererDesc.streamUsage;
    AudioStreamType audioStreamType = IAudioStream::GetStreamType(contentType, streamUsage);
    audioInterrupt_.audioFocusType.streamType = audioStreamType;
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    return currentStream->SetAudioStreamType(audioStreamType);
}

int32_t AudioRendererPrivate::SetStreamType(AudioStreamType audioStreamType)
{
    audioInterrupt_.audioFocusType.streamType = audioStreamType;
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    return currentStream->SetAudioStreamType(audioStreamType);
}

int32_t AudioRendererPrivate::SetVolumeMode(int32_t mode)
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    AUDIO_INFO_LOG("SetVolumeMode mode = %{public}d", mode);
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    rendererInfo_.volumeMode = static_cast<AudioVolumeMode>(mode);
    return SUCCESS;
}

int32_t AudioRendererPrivate::SetVolume(float volume) const
{
    UpdateAudioInterruptStrategy(volume, true);
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    return currentStream->SetVolume(volume);
}

void AudioRendererPrivate::UpdateAudioInterruptStrategy(float volume, bool setVolume) const
{
    CHECK_AND_RETURN_LOG(audioStream_ != nullptr, "audioStream_ is nullptr");
    State currentState = audioStream_->GetState();
    bool isMute = audioStream_->GetMute();
    bool noNeedActive = setVolume && (audioStream_->GetVolume() > 0) && (volume > 0);
    if (currentState == NEW || currentState == PREPARED) {
        AUDIO_INFO_LOG("UpdateAudioInterruptStrategy for set volume before RUNNING,  volume=%{public}f", volume);
        isStillZeroStreamVolume_ = (volume == 0);
    } else if ((isStillZeroStreamVolume_ || !isMute) && volume > 0) {
        isStillZeroStreamVolume_ = false;
        audioInterrupt_.sessionStrategy.concurrencyMode =
            (originalStrategy_.concurrencyMode == AudioConcurrencyMode::INVALID ?
            AudioConcurrencyMode::DEFAULT : originalStrategy_.concurrencyMode);
        if (currentState == RUNNING && !noNeedActive) {
            AudioInterrupt audioInterrupt = audioInterrupt_;
            AUDIO_INFO_LOG("UpdateAudioInterruptStrategy for set volume,  volume=%{public}f", volume);
            int ret = AudioPolicyManager::GetInstance().ActivateAudioInterrupt(audioInterrupt, 0, true);
            CHECK_AND_RETURN_LOG(ret == 0, "ActivateAudioInterrupt Failed at SetVolume");
        }
    }
}

float AudioRendererPrivate::GetVolume() const
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    return currentStream->GetVolume();
}

int32_t AudioRendererPrivate::SetLoudnessGain(float loudnessGain) const
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    CHECK_AND_RETURN_RET_LOG(rendererInfo_.streamUsage == STREAM_USAGE_MUSIC ||
        rendererInfo_.streamUsage == STREAM_USAGE_MOVIE ||
        rendererInfo_.streamUsage == STREAM_USAGE_AUDIOBOOK, ERROR_UNSUPPORTED, "audio stream type not supported");
    CHECK_AND_RETURN_RET_LOG(((loudnessGain >= MIN_LOUDNESS_GAIN) && (loudnessGain <= MAX_LOUDNESS_GAIN)),
        ERROR_INVALID_PARAM, "loudnessGain set invalid");
    CHECK_AND_RETURN_RET_LOG(rendererInfo_.rendererFlags != AUDIO_FLAG_MMAP &&
        rendererInfo_.rendererFlags != AUDIO_FLAG_VOIP_FAST &&
        rendererInfo_.rendererFlags != AUDIO_FLAG_DIRECT &&
        rendererInfo_.rendererFlags != AUDIO_FLAG_VOIP_DIRECT,
        ERROR_UNSUPPORTED, "low latency mode not supported");
    return currentStream->SetLoudnessGain(loudnessGain);
}

float AudioRendererPrivate::GetLoudnessGain() const
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, 0.0f, "audioStream_ is nullptr");

    CHECK_AND_RETURN_RET_LOG(rendererInfo_.streamUsage == STREAM_USAGE_MUSIC ||
        rendererInfo_.streamUsage == STREAM_USAGE_MOVIE ||
        rendererInfo_.streamUsage == STREAM_USAGE_AUDIOBOOK, 0.0f, "audio stream type not supported");

    CHECK_AND_RETURN_RET_LOG(rendererInfo_.rendererFlags != AUDIO_FLAG_MMAP &&
        rendererInfo_.rendererFlags != AUDIO_FLAG_VOIP_FAST &&
        rendererInfo_.rendererFlags != AUDIO_FLAG_DIRECT &&
        rendererInfo_.rendererFlags != AUDIO_FLAG_VOIP_DIRECT,
        0.0f, "low latency mode not supported");

    return currentStream->GetLoudnessGain();
}

int32_t AudioRendererPrivate::SetRenderRate(AudioRendererRate renderRate) const
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    int32_t ret = currentStream->SetRenderRate(renderRate);
    CHECK_AND_RETURN_RET(ret == SUCCESS, ret);
    float speed = 1.0f;
    switch (renderRate) {
        case RENDER_RATE_NORMAL:
            speed = 1.0f;
            break;
        case RENDER_RATE_DOUBLE:
            speed = 2.0f;
            break;
        case RENDER_RATE_HALF:
            speed = 0.5f;
            break;
        default:
            speed = 1.0f;
    }
    ret = currentStream->SetSpeed(speed);
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("SetSpeed Failed, error: %{public}d", ret);
    }
    ret = currentStream->SetPitch(speed);
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("SetPitch Failed, error: %{public}d", ret);
    }
    return SUCCESS;
}

AudioRendererRate AudioRendererPrivate::GetRenderRate() const
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, RENDER_RATE_NORMAL, "audioStream_ is nullptr");
    return currentStream->GetRenderRate();
}

int32_t AudioRendererPrivate::SetRendererSamplingRate(uint32_t sampleRate) const
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    return currentStream->SetRendererSamplingRate(sampleRate);
}

uint32_t AudioRendererPrivate::GetRendererSamplingRate() const
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    return currentStream->GetRendererSamplingRate();
}

int32_t AudioRendererPrivate::SetBufferDuration(uint64_t bufferDuration) const
{
    CHECK_AND_RETURN_RET_LOG(bufferDuration >= MINIMUM_BUFFER_SIZE_MSEC && bufferDuration <= MAXIMUM_BUFFER_SIZE_MSEC,
        ERR_INVALID_PARAM, "Error: Please set the buffer duration between 5ms ~ 60ms");
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    return currentStream->SetBufferSizeInMsec(bufferDuration);
}

int32_t AudioRendererPrivate::SetChannelBlendMode(ChannelBlendMode blendMode)
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    return currentStream->SetChannelBlendMode(blendMode);
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

void AudioRendererInterruptCallbackImpl::StartSwitch()
{
    std::lock_guard<std::mutex> lock(mutex_);
    switching_ = true;
    AUDIO_INFO_LOG("SwitchStream start, block interrupt callback");
}

void AudioRendererInterruptCallbackImpl::FinishSwitch()
{
    std::lock_guard<std::mutex> lock(mutex_);
    switching_ = false;
    switchStreamCv_.notify_all();
    AUDIO_INFO_LOG("SwitchStream finish, notify interrupt callback");
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
    if (interruptEventForced.hintType == INTERRUPT_HINT_RESUME) {
        // Reusme event should be INTERRUPT_SHARE type. Change the force type before sending the interrupt event.
        interruptEventForced.forceType = INTERRUPT_SHARE;
    }
    NotifyEvent(interruptEventForced);
}

void AudioRendererInterruptCallbackImpl::OnInterrupt(const InterruptEventInternal &interruptEvent)
{
    std::unique_lock<std::mutex> lock(mutex_);

    if (interruptEvent.hintType == InterruptHint::INTERRUPT_HINT_EXIT_STANDALONE) {
        int32_t ret = AudioPolicyManager::GetInstance().ActivateAudioInterrupt(audioInterrupt_);
        CHECK_AND_RETURN_LOG(ret == 0, "resume ActivateAudioInterrupt Failed");
        return;
    }
    if (switching_) {
        AUDIO_INFO_LOG("Wait for SwitchStream");
        bool res = switchStreamCv_.wait_for(lock, std::chrono::milliseconds(BLOCK_INTERRUPT_CALLBACK_IN_MS),
            [this] {return !switching_;});
        if (!res) {
            switching_ = false;
            AUDIO_WARNING_LOG("Wait for SwitchStream time out, could handle interrupt event with old stream");
        }
    }
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

AudioStreamCallbackRenderer::AudioStreamCallbackRenderer(std::weak_ptr<AudioRendererPrivate> renderer)
    : renderer_(renderer)
{
}

void AudioStreamCallbackRenderer::SaveCallback(const std::weak_ptr<AudioRendererCallback> &callback)
{
    callback_ = callback;
}

void AudioStreamCallbackRenderer::OnStateChange(const State state, const StateChangeCmdType cmdType)
{
    std::shared_ptr<AudioRendererPrivate> rendererObj = renderer_.lock();
    CHECK_AND_RETURN_LOG(rendererObj != nullptr, "rendererObj is nullptr");
    std::shared_ptr<AudioRendererCallback> cb = callback_.lock();
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb == nullptr.");

    auto renderState = static_cast<RendererState>(state);
    cb->OnStateChange(renderState, cmdType);

    AudioInterrupt audioInterrupt;
    rendererObj->GetAudioInterrupt(audioInterrupt);
    audioInterrupt.state = state;
    rendererObj->SetAudioInterrupt(audioInterrupt);
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
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    int32_t ret = currentStream->SetRenderMode(renderMode);
    callbackLoopTid_ = audioStream_->GetCallbackLoopTid();
    return ret;
}

AudioRenderMode AudioRendererPrivate::GetRenderMode() const
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, RENDER_MODE_NORMAL, "audioStream_ is nullptr");
    return currentStream->GetRenderMode();
}

int32_t AudioRendererPrivate::GetBufferDesc(BufferDesc &bufDesc)
{
    AsyncCheckAudioRenderer("GetBufferDesc");
    std::shared_ptr<IAudioStream> currentStream = audioStream_;
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    int32_t ret = currentStream->GetBufferDesc(bufDesc);
    return ret;
}

int32_t AudioRendererPrivate::Enqueue(const BufferDesc &bufDesc)
{
    Trace trace("AudioRenderer::Enqueue");
    AsyncCheckAudioRenderer("Enqueue");
    MockPcmData(bufDesc.buffer, bufDesc.bufLength);
    DumpFileUtil::WriteDumpFile(dumpFile_, static_cast<void *>(bufDesc.buffer), bufDesc.bufLength);
    std::shared_ptr<IAudioStream> currentStream = audioStream_;
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    int32_t ret = currentStream->Enqueue(bufDesc);
    return ret;
}

int32_t AudioRendererPrivate::Clear() const
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    return currentStream->Clear();
}

int32_t AudioRendererPrivate::GetBufQueueState(BufferQueueState &bufState) const
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    return currentStream->GetBufQueueState(bufState);
}

int32_t AudioRendererPrivate::SetRendererWriteCallback(const std::shared_ptr<AudioRendererWriteCallback> &callback)
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    return currentStream->SetRendererWriteCallback(callback);
}

int32_t AudioRendererPrivate::SetRendererFirstFrameWritingCallback(
    const std::shared_ptr<AudioRendererFirstFrameWritingCallback> &callback)
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    return currentStream->SetRendererFirstFrameWritingCallback(callback);
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
    std::shared_lock<std::shared_mutex> sharedLockSwitch;
    if (callbackLoopTid_ != gettid()) { // No need to add lock in callback thread to prevent deadlocks
        sharedLockSwitch = std::shared_lock<std::shared_mutex>(rendererMutex_);
    }
    std::lock_guard<std::mutex> lock(silentModeAndMixWithOthersMutex_);
    if (audioStream_->GetSilentModeAndMixWithOthers() && !on) {
        audioInterrupt_.sessionStrategy.concurrencyMode = originalStrategy_.concurrencyMode;
        if (static_cast<RendererState>(audioStream_->GetState()) == RENDERER_RUNNING) {
            int32_t ret = AudioPolicyManager::GetInstance().ActivateAudioInterrupt(audioInterrupt_, 0, true);
            CHECK_AND_RETURN_LOG(ret == SUCCESS, "ActivateAudioInterrupt Failed");
        }
        audioStream_->SetSilentModeAndMixWithOthers(on);
        return;
    } else if (!audioStream_->GetSilentModeAndMixWithOthers() && on) {
        audioStream_->SetSilentModeAndMixWithOthers(on);
        audioInterrupt_.sessionStrategy.concurrencyMode = AudioConcurrencyMode::SILENT;
        if (static_cast<RendererState>(audioStream_->GetState()) == RENDERER_RUNNING) {
            int32_t ret = AudioPolicyManager::GetInstance().ActivateAudioInterrupt(audioInterrupt_, 0, true);
            CHECK_AND_RETURN_LOG(ret == SUCCESS, "ActivateAudioInterrupt Failed");
        }
        return;
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
    if (parallelPlayFlag) {
        audioInterrupt_.sessionStrategy.concurrencyMode = AudioConcurrencyMode::MIX_WITH_OTHERS;
    } else {
        audioInterrupt_.sessionStrategy.concurrencyMode = originalStrategy_.concurrencyMode;
    }
    return SUCCESS;
}

int32_t AudioRendererPrivate::SetLowPowerVolume(float volume) const
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    return currentStream->SetLowPowerVolume(volume);
}

float AudioRendererPrivate::GetLowPowerVolume() const
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    return currentStream->GetLowPowerVolume();
}

// in plan: need remove
int32_t AudioRendererPrivate::SetOffloadAllowed(bool isAllowed)
{
    AUDIO_PRERELEASE_LOGI("offload allowed: %{public}d", isAllowed);
    rendererInfo_.isOffloadAllowed = isAllowed;
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    currentStream->SetRendererInfo(rendererInfo_);
    return SUCCESS;
}

int32_t AudioRendererPrivate::SetOffloadMode(int32_t state, bool isAppBack) const
{
    AUDIO_INFO_LOG("set offload mode for session %{public}u", sessionID_);
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    return currentStream->SetOffloadMode(state, isAppBack);
}

int32_t AudioRendererPrivate::UnsetOffloadMode() const
{
    AUDIO_INFO_LOG("session %{public}u session unset offload", sessionID_);
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    int32_t ret = currentStream->UnsetOffloadMode();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "unset offload failed");
    return SUCCESS;
}

float AudioRendererPrivate::GetSingleStreamVolume() const
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    return currentStream->GetSingleStreamVolume();
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
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
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
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    return currentStream->GetUnderflowCount();
}


void AudioRendererPrivate::SetAudioRendererErrorCallback(std::shared_ptr<AudioRendererErrorCallback> errorCallback)
{
    std::shared_lock<std::shared_mutex> sharedLock;
    if (callbackLoopTid_ != gettid()) { // No need to add lock in callback thread to prevent deadlocks
        sharedLock = std::shared_lock<std::shared_mutex>(rendererMutex_);
    }
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

void AudioRendererPrivate::SetFastStatusChangeCallback(
    const std::shared_ptr<AudioRendererFastStatusChangeCallback> &callback)
{
    std::lock_guard lock(fastStatusChangeCallbackMutex_);
    fastStatusChangeCallback_ = callback;
}

bool AudioRendererPrivate::SetSwitchInfo(IAudioStream::SwitchInfo info, std::shared_ptr<IAudioStream> audioStream)
{
    CHECK_AND_RETURN_RET_LOG(audioStream, false, "stream is nullptr");

    audioStream->SetStreamTrackerState(false);
    audioStream->SetClientID(info.clientPid, info.clientUid, appInfo_.appTokenId, appInfo_.appFullTokenId);
    audioStream->SetPrivacyType(info.privacyType);
    audioStream->SetRendererInfo(info.rendererInfo);
    audioStream->SetCapturerInfo(info.capturerInfo);
    int32_t res = audioStream->SetAudioStreamInfo(info.params, rendererProxyObj_);
    CHECK_AND_RETURN_RET_LOG(res == SUCCESS, false, "SetAudioStreamInfo failed");
    audioStream->SetDefaultOutputDevice(info.defaultOutputDevice, true);
    audioStream->SetRenderMode(info.renderMode);
    callbackLoopTid_ = audioStream->GetCallbackLoopTid();
    audioStream->SetAudioEffectMode(info.effectMode);
    audioStream->SetVolume(info.volume);
    res = audioStream->SetDuckVolume(info.duckVolume);
    CHECK_AND_RETURN_RET_LOG(res == SUCCESS, false, "SetDuckVolume failed");
    audioStream->SetUnderflowCount(info.underFlowCount);

    if (info.userSettedPreferredFrameSize.has_value()) {
        audioStream->SetPreferredFrameSize(info.userSettedPreferredFrameSize.value());
    }

    audioStream->SetSilentModeAndMixWithOthers(info.silentModeAndMixWithOthers);

    if (speed_.has_value()) {
        audioStream->SetSpeed(speed_.value());
    }

    if (pitch_.has_value()) {
        audioStream->SetPitch(pitch_.value());
    }

    if (info.lastCallStartByUserTid.has_value()) {
        audioStream->SetCallStartByUserTid(info.lastCallStartByUserTid.value());
    }

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
    audioStream->SetSwitchInfoTimestamp(info.lastFramePosAndTimePair, info.lastFramePosAndTimePairWithSpeed);
    return true;
}

// The only function that updates AudioRendererPrivate::audioStream_ in its life cycle.
void AudioRendererPrivate::UpdateRendererAudioStream(const std::shared_ptr<IAudioStream> &newAudioStream)
{
    audioStream_ = newAudioStream;
    audioStream_->GetRendererInfo(rendererInfo_);
    if (audioInterruptCallback_ != nullptr) {
        std::shared_ptr<AudioRendererInterruptCallbackImpl> interruptCbImpl =
            std::static_pointer_cast<AudioRendererInterruptCallbackImpl>(audioInterruptCallback_);
        interruptCbImpl->UpdateAudioStream(audioStream_);
    }
}

// Inner function. Must be called with AudioRendererPrivate::rendererMutex_ held;
int32_t AudioRendererPrivate::UnsetOffloadModeInner() const
{
    AUDIO_INFO_LOG("session %{public}u session unset offload", sessionID_);
    int32_t ret = audioStream_->UnsetOffloadMode();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "unset offload failed");
    return SUCCESS;
}

// Inner function. Must be called with AudioRendererPrivate::rendererMutex_
// or AudioRendererPrivate::streamMutex_ held.
RendererState AudioRendererPrivate::GetStatusInner()
{
    if (IsNoStreamRenderer()) {
        return state_;
    }
    return static_cast<RendererState>(audioStream_->GetState());
}

// Inner function. Must be called with AudioRendererPrivate::rendererMutex_
// or AudioRendererPrivate::streamMutex_ held.
void AudioRendererPrivate::SetAudioPrivacyTypeInner(AudioPrivacyType privacyType)
{
    CHECK_AND_RETURN_LOG(audioStream_ != nullptr, "audioStream_ is nullptr");
    privacyType_ = privacyType;
    audioStream_->SetPrivacyType(privacyType);
}

// Inner function. Must be called with AudioRendererPrivate::rendererMutex_
// or AudioRendererPrivate::streamMutex_ held.
int32_t AudioRendererPrivate::GetAudioStreamIdInner(uint32_t &sessionID) const
{
    return audioStream_->GetAudioSessionID(sessionID);
}

// Inner function. Must be called with AudioRendererPrivate::rendererMutex_
// or AudioRendererPrivate::streamMutex_ held.
float AudioRendererPrivate::GetVolumeInner() const
{
    return audioStream_->GetVolume();
}

// Inner function. Must be called with AudioRendererPrivate::rendererMutex_
// or AudioRendererPrivate::streamMutex_ held.
uint32_t AudioRendererPrivate::GetUnderflowCountInner() const
{
    return audioStream_->GetUnderflowCount();
}

// Only called in SwitchToTargetStream, with AudioRendererPrivate::rendererMutex_ held.
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
    } else if (rendererInfo_.rendererFlags == AUDIO_FLAG_NORMAL) {
        info.rendererInfo.rendererFlags = AUDIO_FLAG_NORMAL;
    } else if (rendererInfo_.rendererFlags == AUDIO_FLAG_MMAP) {
        info.rendererInfo.rendererFlags = AUDIO_FLAG_MMAP;
    }
    info.params.originalSessionId = sessionID_;
    return;
}

// Only called in SwitchToTargetStream, with AudioRendererPrivate::rendererMutex_ held.
void AudioRendererPrivate::UpdateFramesWritten()
{
    int64_t framesWritten = audioStream_->GetFramesWritten();
    if (framesWritten > 0) {
        framesAlreadyWritten_ += framesWritten;
        AUDIO_INFO_LOG("Frames already written: %{public}" PRId64 ", current stream value: %{public}" PRId64 ".",
            framesAlreadyWritten_, framesWritten);
    }
}

// Set new stream info before switching to new stream.
bool AudioRendererPrivate::InitTargetStream(IAudioStream::SwitchInfo &info,
    std::shared_ptr<IAudioStream> &newAudioStream)
{
    bool initResult = SetSwitchInfo(info, newAudioStream);
    if (initResult != SUCCESS && info.rendererInfo.originalFlag != AUDIO_FLAG_NORMAL) {
        AUDIO_ERR_LOG("Re-create stream failed, crate normal ipc stream");
        isFastRenderer_ = false;
        newAudioStream = IAudioStream::GetPlaybackStream(IAudioStream::PA_STREAM, info.params,
            info.eStreamType, appInfo_.appUid);
        CHECK_AND_RETURN_RET_LOG(newAudioStream != nullptr, false, "Get ipc stream failed");
        initResult = SetSwitchInfo(info, newAudioStream);
        CHECK_AND_RETURN_RET_LOG(initResult, false, "Init ipc strean failed");
    }
    return initResult;
}

bool AudioRendererPrivate::FinishOldStream(IAudioStream::StreamClass targetClass, RestoreInfo restoreInfo,
    RendererState previousState, IAudioStream::SwitchInfo &switchInfo)
{
    audioStream_->SetMute(true, CMD_FROM_SYSTEM); // Do not record this status in recover(InitSwitchInfo)
    bool switchResult = false;
    if (previousState == RENDERER_RUNNING) {
        switchResult = audioStream_->StopAudioStream();
        if (restoreInfo.restoreReason != SERVER_DIED) {
            CHECK_AND_RETURN_RET_LOG(switchResult, false, "StopAudioStream failed.");
        } else {
            switchResult = true;
        }
    }
    InitSwitchInfo(targetClass, switchInfo);
    if (restoreInfo.restoreReason == SERVER_DIED) {
        AUDIO_INFO_LOG("Server died, reset session id: %{public}d", switchInfo.params.originalSessionId);
        switchInfo.params.originalSessionId = 0;
        switchInfo.sessionId = 0;
    }
    UpdateFramesWritten();
    switchResult = audioStream_->ReleaseAudioStream(true, true);
    if (restoreInfo.restoreReason != SERVER_DIED) {
        CHECK_AND_RETURN_RET_LOG(switchResult, false, "release old stream failed.");
    } else {
        switchResult = true;
    }
    return switchResult;
}

bool AudioRendererPrivate::GenerateNewStream(IAudioStream::StreamClass targetClass, RestoreInfo restoreInfo,
    RendererState previousState, IAudioStream::SwitchInfo &switchInfo)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = GetStreamDescBySwitchInfo(switchInfo, restoreInfo);
    uint32_t flag = AUDIO_OUTPUT_FLAG_NORMAL;
    std::string networkId = LOCAL_NETWORK_ID;
    int32_t ret = AudioPolicyManager::GetInstance().CreateRendererClient(
        streamDesc, flag, switchInfo.params.originalSessionId, networkId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, false, "CreateRendererClient failed");

    bool switchResult = false;
    std::shared_ptr<IAudioStream> oldAudioStream = nullptr;
    // create new IAudioStream
    std::shared_ptr<IAudioStream> newAudioStream = IAudioStream::GetPlaybackStream(targetClass, switchInfo.params,
        switchInfo.eStreamType, appInfo_.appUid);
    CHECK_AND_RETURN_RET_LOG(newAudioStream != nullptr, false, "SetParams GetPlayBackStream failed.");
    AUDIO_INFO_LOG("Get new stream success!");

    // set new stream info. When switch to fast stream failed, call SetSwitchInfo again
    // and switch to normal ipc stream to avoid silence.
    switchResult = SetSwitchInfo(switchInfo, newAudioStream);
    if (!switchResult && switchInfo.rendererInfo.originalFlag != AUDIO_FLAG_NORMAL) {
        AUDIO_ERR_LOG("Re-create stream failed, create normal ipc stream");
        if (restoreInfo.restoreReason == SERVER_DIED) {
            switchInfo.sessionId = switchInfo.params.originalSessionId;
            streamDesc->sessionId_ = switchInfo.params.originalSessionId;
        }
        streamDesc->rendererInfo_.rendererFlags = AUDIO_FLAG_FORCED_NORMAL;
        streamDesc->routeFlag_ = AUDIO_FLAG_NONE;
        int32_t ret = AudioPolicyManager::GetInstance().CreateRendererClient(streamDesc, flag,
            switchInfo.params.originalSessionId, networkId);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, false, "CreateRendererClient failed");

        newAudioStream = IAudioStream::GetPlaybackStream(IAudioStream::PA_STREAM, switchInfo.params,
            switchInfo.eStreamType, appInfo_.appUid);
        targetClass = IAudioStream::PA_STREAM;
        CHECK_AND_RETURN_RET_LOG(newAudioStream != nullptr, false, "Get ipc stream failed");
        switchResult = SetSwitchInfo(switchInfo, newAudioStream);
        CHECK_AND_RETURN_RET_LOG(switchResult, false, "Init ipc stream failed");
    }
    oldAudioStream = audioStream_;
    // Update audioStream_ to newAudioStream in both AudioRendererPrivate and AudioInterruptCallbackImpl.
    // Operation of replace audioStream_ must be performed before StartAudioStream.
    // Otherwise GetBufferDesc will return the buffer pointer of oldStream (causing Use-After-Free).
    UpdateRendererAudioStream(newAudioStream);
    newAudioStream->NotifyRouteUpdate(flag, networkId);

    // Start new stream if old stream was in running state.
    // When restoring for audio server died, no need for restart.
    if (restoreInfo.restoreReason == SERVER_DIED && IsNoStreamRenderer()) {
        AUDIO_INFO_LOG("Telephony scene , no need for start");
    } else if (previousState == RENDERER_RUNNING) {
        // restart audio stream
        switchResult = newAudioStream->StartAudioStream(CMD_FROM_CLIENT,
            static_cast<AudioStreamDeviceChangeReasonExt::ExtEnum>(restoreInfo.deviceChangeReason));
        CHECK_AND_RETURN_RET_LOG(switchResult, false, "start new stream failed.");
    }

    isFastRenderer_ = IAudioStream::IsFastStreamClass(targetClass);
    return switchResult;
}

bool AudioRendererPrivate::ContinueAfterConcede(IAudioStream::StreamClass &targetClass, RestoreInfo restoreInfo)
{
    CHECK_AND_RETURN_RET(restoreInfo.restoreReason == STREAM_CONCEDED, true);
    targetClass = IAudioStream::PA_STREAM;
    uint32_t sessionId = sessionID_;
    GetAudioStreamIdInner(sessionId);
    AudioPipeType pipeType = PIPE_TYPE_NORMAL_OUT;
    audioStream_->GetAudioPipeType(pipeType);
    AUDIO_INFO_LOG("session %{public}u concede from pipeType %{public}d", sessionID_, rendererInfo_.pipeType);
    rendererInfo_.pipeType = PIPE_TYPE_NORMAL_OUT;
    rendererInfo_.isOffloadAllowed = false;
    audioStream_->SetRendererInfo(rendererInfo_);
    if (pipeType == PIPE_TYPE_OFFLOAD) {
        UnsetOffloadModeInner();
        AudioPolicyManager::GetInstance().MoveToNewPipe(sessionId, PIPE_TYPE_NORMAL_OUT);
        audioStream_->SetRestoreStatus(NO_NEED_FOR_RESTORE);
        return false;
    }
    if ((pipeType == PIPE_TYPE_LOWLATENCY_OUT && audioStream_->GetStreamClass() != IAudioStream::PA_STREAM) ||
        pipeType == PIPE_TYPE_DIRECT_MUSIC) {
        return true;
    }
    audioStream_->SetRestoreStatus(NO_NEED_FOR_RESTORE);
    return false;
}

bool AudioRendererPrivate::ContinueAfterSplit(RestoreInfo restoreInfo)
{
    CHECK_AND_RETURN_RET(restoreInfo.restoreReason == STREAM_SPLIT, true);
    audioStream_->FetchDeviceForSplitStream();
    return false;
}

bool AudioRendererPrivate::SwitchToTargetStream(IAudioStream::StreamClass targetClass, RestoreInfo restoreInfo)
{
    bool switchResult = false;
    Trace trace("KeyAction AudioRenderer::SwitchToTargetStream " + std::to_string(sessionID_)
        + ", target class " + std::to_string(targetClass) + ", reason " + std::to_string(restoreInfo.restoreReason)
        + ", device change reason " + std::to_string(restoreInfo.deviceChangeReason)
        + ", target flag " + std::to_string(restoreInfo.targetStreamFlag));
    AUDIO_INFO_LOG("Restore AudioRenderer %{public}u, target class %{public}d, reason: %{public}d, "
        "device change reason %{public}d, target flag %{public}d", sessionID_, targetClass,
        restoreInfo.restoreReason, restoreInfo.deviceChangeReason, restoreInfo.targetStreamFlag);

    isSwitching_ = true;
    audioStream_->SetSwitchingStatus(true);
    AudioScopeExit scopeExit([this] () {
        audioStream_->SetSwitchingStatus(false);
    });
    RendererState previousState = GetStatusInner();
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
    InitAudioRouteCallback();
    isSwitching_ = false;
    switchResult = true;
    scopeExit.Relase();
    WriteSwitchStreamLogMsg();
    return switchResult;
}

std::shared_ptr<AudioStreamDescriptor> AudioRendererPrivate::GetStreamDescBySwitchInfo(
    const IAudioStream::SwitchInfo &switchInfo, const RestoreInfo &restoreInfo)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->streamInfo_.format = static_cast<AudioSampleFormat>(switchInfo.params.format);
    streamDesc->streamInfo_.samplingRate = static_cast<AudioSamplingRate>(switchInfo.params.samplingRate);
    streamDesc->streamInfo_.channels = static_cast<AudioChannel>(switchInfo.params.channels);
    streamDesc->streamInfo_.encoding = static_cast<AudioEncodingType>(switchInfo.params.encoding);
    streamDesc->streamInfo_.channelLayout = static_cast<AudioChannelLayout>(switchInfo.params.channelLayout);

    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->createTimeStamp_ = ClockTime::GetCurNano();
    streamDesc->rendererInfo_ = switchInfo.rendererInfo;
    streamDesc->appInfo_ = AppInfo{switchInfo.appUid, 0, switchInfo.clientPid, 0};
    streamDesc->callerUid_ = static_cast<int32_t>(getuid());
    streamDesc->callerPid_ = static_cast<int32_t>(getpid());
    streamDesc->sessionId_ = switchInfo.sessionId;
    streamDesc->routeFlag_ = restoreInfo.routeFlag;
    if (restoreInfo.targetStreamFlag == AUDIO_FLAG_FORCED_NORMAL) {
        streamDesc->rendererInfo_.originalFlag = AUDIO_FLAG_FORCED_NORMAL;
    }
    return streamDesc;
}

void AudioRendererPrivate::HandleAudioInterruptWhenServerDied()
{
    InitAudioInterruptCallback(true); // Register audio interrupt callback again.
    if (GetStatusInner() == RENDERER_RUNNING) {
        int32_t ret = AudioPolicyManager::GetInstance().ActivateAudioInterrupt(audioInterrupt_);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("active audio interrupt failed");
        }
    }
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

// NOTIFY: Possible audioRendererPrivate destruction here.
void OutputDeviceChangeWithInfoCallbackImpl::OnRecreateStreamEvent(const uint32_t sessionId, const int32_t streamFlag,
    const AudioStreamDeviceChangeReasonExt reason)
{
    std::unique_lock<std::mutex> lock(audioRendererObjMutex_);
    AUDIO_INFO_LOG("Enter, session id: %{public}d, stream flag: %{public}d", sessionId, streamFlag);
    auto sharedptrRenderer = renderer_.lock();
    CHECK_AND_RETURN_LOG(sharedptrRenderer != nullptr, "renderer_ is nullptr");
    lock.unlock();
}

AudioEffectMode AudioRendererPrivate::GetAudioEffectMode() const
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, EFFECT_NONE, "audioStream_ is nullptr");
    return currentStream->GetAudioEffectMode();
}

int64_t AudioRendererPrivate::GetFramesWritten() const
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    return framesAlreadyWritten_ + currentStream->GetFramesWritten();
}

int32_t AudioRendererPrivate::SetAudioEffectMode(AudioEffectMode effectMode) const
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    return currentStream->SetAudioEffectMode(effectMode);
}

int32_t AudioRendererPrivate::SetVolumeWithRamp(float volume, int32_t duration)
{
    AUDIO_INFO_LOG("volume:%{public}f duration:%{public}d", volume, duration);
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    return currentStream->SetVolumeWithRamp(volume, duration);
}

void AudioRendererPrivate::SetPreferredFrameSize(int32_t frameSize)
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_LOG(currentStream != nullptr, "audioStream_ is nullptr");
    currentStream->SetPreferredFrameSize(frameSize);
}

void AudioRendererPrivate::GetAudioInterrupt(AudioInterrupt &audioInterrupt)
{
    audioInterrupt = audioInterrupt_;
}

void AudioRendererPrivate::SetAudioInterrupt(const AudioInterrupt &audioInterrupt)
{
    audioInterrupt_ = audioInterrupt;
}

// Only called AudioRendererPrivate::Stop(), with AudioRendererPrivate::rendererMutex_ held.
void AudioRendererPrivate::WriteUnderrunEvent() const
{
    AUDIO_INFO_LOG("AudioRendererPrivate WriteUnderrunEvent!");
    if (GetUnderflowCountInner() < WRITE_UNDERRUN_NUM) {
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
        audioPolicyServiceDiedCallback_->SetAudioRendererObj(weak_from_this());
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
}

void RendererPolicyServiceDiedCallback::SetAudioRendererObj(std::weak_ptr<AudioRendererPrivate> rendererObj)
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

    if (taskCount_.fetch_add(1) > 0) {
        AUDIO_INFO_LOG("direct ret");
        return;
    }

    std::weak_ptr<RendererPolicyServiceDiedCallback> weakRefCb = weak_from_this();

    std::thread restoreThread ([weakRefCb] {
        std::shared_ptr<RendererPolicyServiceDiedCallback> strongRefCb = weakRefCb.lock();
        CHECK_AND_RETURN_LOG(strongRefCb != nullptr, "strongRef is nullptr");
        int32_t count;
        do {
            count = strongRefCb->taskCount_.load();
            strongRefCb->RestoreTheadLoop();
        } while (strongRefCb->taskCount_.fetch_sub(count) > count);
    });
    pthread_setname_np(restoreThread.native_handle(), "OS_ARPSRestore");
    restoreThread.detach();
}

void RendererPolicyServiceDiedCallback::RestoreTheadLoop()
{
    int32_t tryCounter = 10;
    uint32_t sleepTime = 300000;
    bool restoreResult = false;
    while (!restoreResult && tryCounter > 0) {
        tryCounter--;
        usleep(sleepTime);
        std::shared_ptr<AudioRendererPrivate> sharedRenderer = renderer_.lock();
        CHECK_AND_RETURN_LOG(sharedRenderer != nullptr, "sharedRenderer is nullptr");
        if (sharedRenderer->audioStream_ == nullptr || sharedRenderer->abortRestore_) {
            AUDIO_INFO_LOG("abort restore");
            break;
        }
        sharedRenderer->RestoreAudioInLoop(restoreResult, tryCounter);
    }
}

void AudioRendererPrivate::RestoreAudioInLoop(bool &restoreResult, int32_t &tryCounter)
{
    std::unique_lock<std::shared_mutex> lock;
    if (callbackLoopTid_ != gettid()) { // No need to add lock in callback thread to prevent deadlocks
        lock = std::unique_lock<std::shared_mutex>(rendererMutex_);
    }
    CHECK_AND_RETURN_LOG(audioStream_, "audioStream_ is nullptr, no need for restore");
    AUDIO_INFO_LOG("Restore audio renderer when server died, session %{public}u", sessionID_);
    RestoreInfo restoreInfo;
    restoreInfo.restoreReason = SERVER_DIED;
    // When server died, restore client stream by SwitchToTargetStream. Target stream class is
    // the stream class of the old stream.
    restoreResult = SwitchToTargetStream(audioStream_->GetStreamClass(), restoreInfo);
    AUDIO_INFO_LOG("Set restore status when server died, restore result %{public}d", restoreResult);
    return;
}

int32_t AudioRendererPrivate::SetSpeed(float speed)
{
    AUDIO_INFO_LOG("set speed %{public}f", speed);
    CHECK_AND_RETURN_RET_LOG((speed >= MIN_STREAM_SPEED_LEVEL) && (speed <= MAX_STREAM_SPEED_LEVEL),
        ERR_INVALID_PARAM, "invaild speed index");
    std::unique_lock<std::shared_mutex> lock;
    if (callbackLoopTid_ != gettid()) { // No need to add lock in callback thread to prevent deadlocks
        lock = std::unique_lock<std::shared_mutex>(rendererMutex_);
    }
#ifdef SONIC_ENABLE
    CHECK_AND_RETURN_RET_LOG(audioStream_ != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    audioStream_->SetSpeed(speed);
#endif
    speed_ = speed;

    if (lock.owns_lock()) {
        lock.unlock();
    }

    SetPitch(AudioSpeed::GetPitchForSpeed(speed));
    return SUCCESS;
}

int32_t AudioRendererPrivate::SetPitch(float pitch)
{
    AUDIO_INFO_LOG("set pitch %{public}f", pitch);
    CHECK_AND_RETURN_RET_LOG((pitch >= MIN_STREAM_SPEED_LEVEL) && (pitch <= MAX_STREAM_SPEED_LEVEL),
        ERR_INVALID_PARAM, "invaild pitch index");
    std::unique_lock<std::shared_mutex> lock;
    if (callbackLoopTid_ != gettid()) { // No need to add lock in callback thread to prevent deadlocks
        lock = std::unique_lock<std::shared_mutex>(rendererMutex_);
    }
#ifdef SONIC_ENABLE
    CHECK_AND_RETURN_RET_LOG(audioStream_ != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    audioStream_->SetPitch(pitch);
#endif
    pitch_ = pitch;
    return SUCCESS;
}

float AudioRendererPrivate::GetSpeed()
{
    std::shared_lock<std::shared_mutex> lock;
    if (callbackLoopTid_ != gettid()) { // No need to add lock in callback thread to prevent deadlocks
        lock = std::shared_lock<std::shared_mutex>(rendererMutex_);
    }
#ifdef SONIC_ENABLE
    CHECK_AND_RETURN_RET_LOG(audioStream_ != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    return audioStream_->GetSpeed();
#endif
    return speed_.value_or(1.0f);
}

bool AudioRendererPrivate::IsOffloadEnable()
{
    std::shared_ptr currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, false, "audioStream_ is nullptr");
    bool enable = currentStream->GetOffloadEnable();
    AUDIO_INFO_LOG("GetOffloadEnable is [%{public}s]", (enable ? "true" : "false"));
    return enable;
}

bool AudioRendererPrivate::IsFastRenderer()
{
    return isFastRenderer_;
}

// Inner function. Must be called with AudioRendererPrivate::rendererMutex_
// or AudioRendererPrivate::streamMutex_ held.
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

// Inner function. Must be called with AudioRendererPrivate::rendererMutex_
// or AudioRendererPrivate::streamMutex_ held.
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

void AudioRendererPrivate::EnableVoiceModemCommunicationStartStream(bool enable)
{
    isEnableVoiceModemCommunicationStartStream_ = enable;
}

bool AudioRendererPrivate::IsNoStreamRenderer() const
{
    return rendererInfo_.streamUsage == STREAM_USAGE_VOICE_MODEM_COMMUNICATION &&
        !isEnableVoiceModemCommunicationStartStream_;
}

int64_t AudioRendererPrivate::GetSourceDuration() const
{
    return sourceDuration_;
}

void AudioRendererPrivate::SetSourceDuration(int64_t duration)
{
    sourceDuration_ = duration;
    audioStream_->SetSourceDuration(sourceDuration_);
}

int32_t AudioRendererPrivate::SetDefaultOutputDevice(DeviceType deviceType)
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    if (deviceType != DEVICE_TYPE_EARPIECE && deviceType != DEVICE_TYPE_SPEAKER &&
        deviceType != DEVICE_TYPE_DEFAULT) {
        return ERR_NOT_SUPPORTED;
    }
    bool isSupportedStreamUsage = (find(AUDIO_DEFAULT_OUTPUT_DEVICE_SUPPORTED_STREAM_USAGES.begin(),
        AUDIO_DEFAULT_OUTPUT_DEVICE_SUPPORTED_STREAM_USAGES.end(), rendererInfo_.streamUsage) !=
        AUDIO_DEFAULT_OUTPUT_DEVICE_SUPPORTED_STREAM_USAGES.end());
    CHECK_AND_RETURN_RET_LOG(isSupportedStreamUsage, ERR_NOT_SUPPORTED, "stream usage not supported");
    AUDIO_INFO_LOG("set to %{public}d", deviceType);
    return currentStream->SetDefaultOutputDevice(deviceType);
}

FastStatus AudioRendererPrivate::GetFastStatus()
{
    std::unique_lock<std::shared_mutex> lock(rendererMutex_, std::defer_lock);
    if (callbackLoopTid_ != gettid()) {
        lock.lock();
    }

    return GetFastStatusInner();
}

FastStatus AudioRendererPrivate::GetFastStatusInner()
{
    // inner function. Must be called with AudioRendererPrivate::rendererMutex_ held.
    CHECK_AND_RETURN_RET_LOG(audioStream_ != nullptr, FASTSTATUS_INVALID, "audioStream_ is nullptr");
    return audioStream_->GetFastStatus();
}

// diffrence from GetAudioPosition only when set speed
int32_t AudioRendererPrivate::GetAudioTimestampInfo(Timestamp &timestamp, Timestamp::Timestampbase base) const
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");
    return currentStream->GetAudioTimestampInfo(timestamp, base);
}

int32_t AudioRendererPrivate::InitFormatUnsupportedErrorCallback()
{
    if (!formatUnsupportedErrorCallback_) {
        formatUnsupportedErrorCallback_ = std::make_shared<FormatUnsupportedErrorCallbackImpl>();
        CHECK_AND_RETURN_RET_LOG(formatUnsupportedErrorCallback_ != nullptr, ERROR, "Memory allocation failed");
    }
    int32_t ret = AudioPolicyManager::GetInstance().SetAudioFormatUnsupportedErrorCallback(
        formatUnsupportedErrorCallback_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Register failed");
    return SUCCESS;
}

void AudioRendererPrivate::FastStatusChangeCallback(FastStatus status)
{
    FastStatus newStatus = GetFastStatusInner();
    if (newStatus != status) {
        if (fastStatusChangeCallback_ != nullptr) {
            fastStatusChangeCallback_->OnFastStatusChange(newStatus);
        }
    }
}

void FormatUnsupportedErrorCallbackImpl::OnFormatUnsupportedError(const AudioErrors &errorCode)
{
    std::shared_ptr<AudioRendererErrorCallback> cb = callback_.lock();
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    cb->OnError(errorCode);
}

int32_t AudioRendererPrivate::StartDataCallback()
{
    std::lock_guard<std::shared_mutex> lock(rendererMutex_);
    RendererState state = GetStatusInner();
    CHECK_AND_RETURN_RET_LOG(state == RENDERER_RUNNING, ERROR_ILLEGAL_STATE,
        "StartDataCallback failed. Illegal state:%{public}u", state);
    return audioStream_->SetOffloadDataCallbackState(0); // 0 hdi state need data
}

void AudioRouteCallbackImpl::OnRouteUpdate(uint32_t routeFlag, const std::string &networkId)
{
    std::shared_ptr<AudioRendererPrivate> sharedRenderer = renderer_.lock();
    CHECK_AND_RETURN_LOG(sharedRenderer != nullptr, "renderer is nullptr");
    std::shared_ptr<IAudioStream> currentStream = sharedRenderer->GetInnerStream();
    CHECK_AND_RETURN_LOG(currentStream != nullptr, "audioStream is nullptr");
    currentStream->NotifyRouteUpdate(routeFlag, networkId);
}

void AudioRendererPrivate::SetAudioHapticsSyncId(int32_t audioHapticsSyncId)
{
    AUDIO_PRERELEASE_LOGI("AudioRendererPrivate::SetAudioHapticsSyncId %{public}d", audioHapticsSyncId);
    std::unique_lock<std::shared_mutex> lock;
    if (callbackLoopTid_ != gettid()) { // No need to add lock in callback thread to prevent deadlocks
        lock = std::unique_lock<std::shared_mutex>(rendererMutex_);
    }

    CHECK_AND_RETURN_LOG(audioStream_ != nullptr, "audio stream is null");

    if (audioHapticsSyncId > 0) {
        audioHapticsSyncId_ = audioHapticsSyncId;
        audioStream_->SetAudioHapticsSyncId(audioHapticsSyncId);
    }
}

void AudioRendererPrivate::ResetFirstFrameState()
{
    AUDIO_PRERELEASE_LOGI("AudioRendererPrivate::ResetFirstFrameState");
    std::unique_lock<std::shared_mutex> lock;
    if (callbackLoopTid_ != gettid()) { // No need to add lock in callback thread to prevent deadlocks
        lock = std::unique_lock<std::shared_mutex>(rendererMutex_);
    }

    CHECK_AND_RETURN_LOG(audioStream_ != nullptr, "audio stream is null");

    audioStream_->ResetFirstFrameState();
}

int32_t AudioRendererPrivate::StopDataCallback()
{
    std::lock_guard<std::shared_mutex> lock(rendererMutex_);
    RendererState state = GetStatusInner();
    CHECK_AND_RETURN_RET_LOG(state == RENDERER_RUNNING, ERROR_ILLEGAL_STATE,
        "StopDataCallback failed. Illegal state:%{public}u", state);
    return audioStream_->SetOffloadDataCallbackState(3); // 3 hdi state full
}

void AudioRendererPrivate::SetInterruptEventCallbackType(InterruptEventCallbackType callbackType)
{
    audioInterrupt_.callbackType = callbackType;
}

bool AudioRendererPrivate::IsVirtualKeyboard(const int32_t flags)
{
    bool isBundleNameValid = false;
    std::string bundleName = AudioSystemManager::GetInstance()->GetSelfBundleName(getuid());
    int32_t ret = AudioSystemManager::GetInstance()->CheckVKBInfo(bundleName, isBundleNameValid);
    bool isVirtualKeyboard = (flags == AUDIO_FLAG_VKB_NORMAL || flags == AUDIO_FLAG_VKB_FAST)
        && isBundleNameValid;
    AUDIO_INFO_LOG("Check VKB ret:%{public}d, flags:%{public}d, isVKB:%{public}s", ret, flags,
        isVirtualKeyboard ? "T" : "F");
    return isVirtualKeyboard;
}

int32_t AudioRendererPrivate::CheckAudioRenderer(std::string callingFunc)
{
    CheckAndStopAudioRenderer(callingFunc);
    return CheckAndRestoreAudioRenderer(callingFunc);
}

int32_t AudioRendererPrivate::CheckAndStopAudioRenderer(std::string callingFunc)
{
    std::unique_lock<std::shared_mutex> lock(rendererMutex_, std::defer_lock);
    if (callbackLoopTid_ != gettid()) {
        lock.lock();
    }
    CHECK_AND_RETURN_RET_LOG(audioStream_, ERR_INVALID_PARAM, "audioStream_ is nullptr");

    bool isNeedStop = audioStream_->GetStopFlag();
    if (!isNeedStop) {
        return SUCCESS;
    }

    AUDIO_INFO_LOG("Before %{public}s, stop audio renderer %{public}u", callingFunc.c_str(), sessionID_);
    if (lock.owns_lock()) {
        lock.unlock();
    }
    Stop();
    return SUCCESS;
}

int32_t AudioRendererPrivate::HandleCreateFastStreamError(AudioStreamParams &audioStreamParams,
    AudioStreamType audioStreamType)
{
    AUDIO_INFO_LOG("Create fast Stream fail, play by normal stream.");
    IAudioStream::StreamClass streamClass = IAudioStream::PA_STREAM;
    isFastRenderer_ = false;
    rendererInfo_.rendererFlags = AUDIO_FLAG_FORCED_NORMAL;

    // Create stream desc and pipe
    std::shared_ptr<AudioStreamDescriptor> streamDesc = ConvertToStreamDescriptor(audioStreamParams);
    uint32_t flag = AUDIO_OUTPUT_FLAG_NORMAL;
    std::string networkId = LOCAL_NETWORK_ID;
    int32_t ret = AudioPolicyManager::GetInstance().CreateRendererClient(streamDesc, flag,
        audioStreamParams.originalSessionId, networkId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "CreateRendererClient failed");
    AUDIO_INFO_LOG("Create normal renderer, id: %{public}u", audioStreamParams.originalSessionId);

    audioStream_ = IAudioStream::GetPlaybackStream(streamClass, audioStreamParams, audioStreamType, appInfo_.appUid);
    CHECK_AND_RETURN_RET_LOG(audioStream_ != nullptr, ERR_INVALID_PARAM, "Re-create normal stream failed.");
    ret = InitAudioStream(audioStreamParams);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "InitAudioStream failed");
    audioStream_->SetRenderMode(RENDER_MODE_CALLBACK);
    callbackLoopTid_ = audioStream_->GetCallbackLoopTid();
    audioStream_->NotifyRouteUpdate(flag, networkId);
    return ret;
}
}  // namespace AudioStandard
}  // namespace OHOS
