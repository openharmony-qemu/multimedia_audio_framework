/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "HpaeRendererStreamImpl"
#endif

#ifdef FEATURE_POWER_MANAGER
#include "power_mgr_client.h"
#endif

#include "hpae_renderer_stream_impl.h"
#include "sink/i_audio_render_sink.h"
#include "manager/hdi_adapter_manager.h"
#include <chrono>
#include "safe_map.h"
#include "audio_errors.h"
#include "audio_service_log.h"
#include "audio_utils.h"
#include "i_hpae_manager.h"
#include "audio_stream_info.h"
#include "audio_effect_map.h"

using namespace OHOS::AudioStandard::HPAE;
namespace OHOS {
namespace AudioStandard {

const int32_t MIN_BUFFER_SIZE = 2;
const int32_t FRAME_LEN_10MS = 2;
static AudioChannelLayout SetDefaultChannelLayout(AudioChannel channels);
static std::shared_ptr<IAudioRenderSink> GetRenderSinkInstance(std::string deviceClass, std::string deviceNetId);
HpaeRendererStreamImpl::HpaeRendererStreamImpl(AudioProcessConfig processConfig)
{
    processConfig_ = processConfig;
    spanSizeInFrame_ = FRAME_LEN_10MS * (processConfig.streamInfo.samplingRate / 100);
    byteSizePerFrame_ = (processConfig.streamInfo.channels * GetSizeFromFormat(processConfig.streamInfo.format));
    minBufferSize_ = MIN_BUFFER_SIZE * byteSizePerFrame_ * spanSizeInFrame_;
}
HpaeRendererStreamImpl::~HpaeRendererStreamImpl()
{
    AUDIO_DEBUG_LOG("~HpaeRendererStreamImpl");
}

int32_t HpaeRendererStreamImpl::InitParams(const std::string &deviceName)
{
    HpaeStreamInfo streamInfo;
    streamInfo.channels = processConfig_.streamInfo.channels;
    streamInfo.samplingRate = processConfig_.streamInfo.samplingRate;
    streamInfo.format = processConfig_.streamInfo.format;
    if (processConfig_.streamInfo.channelLayout == CH_LAYOUT_UNKNOWN) {
        streamInfo.channelLayout = SetDefaultChannelLayout(streamInfo.channels);
    }
    streamInfo.frameLen = spanSizeInFrame_;
    streamInfo.sessionId = processConfig_.originalSessionId;
    streamInfo.streamType = processConfig_.streamType;
    streamInfo.fadeType = FadeType::DEFAULT_FADE; // to be passed from processConfig
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    streamInfo.uid = processConfig_.appInfo.appUid;
    streamInfo.pid = processConfig_.appInfo.appPid;
    streamInfo.effectInfo.effectMode = (effectMode_ != EFFECT_DEFAULT && effectMode_ != EFFECT_NONE) ? EFFECT_DEFAULT :
        static_cast<AudioEffectMode>(effectMode_);
    const std::unordered_map<AudioEffectScene, std::string> &audioSupportedSceneTypes = GetSupportedSceneType();
    streamInfo.effectInfo.effectScene = static_cast<AudioEffectScene>(GetKeyFromValue(
        audioSupportedSceneTypes, processConfig_.rendererInfo.sceneType));
    streamInfo.effectInfo.volumeType = STREAM_MUSIC;
    streamInfo.effectInfo.streamUsage = processConfig_.rendererInfo.streamUsage;
    streamInfo.sourceType = processConfig_.isInnerCapturer == true ? SOURCE_TYPE_PLAYBACK_CAPTURE : SOURCE_TYPE_INVALID;
    streamInfo.deviceName = deviceName;
    AUDIO_INFO_LOG("InitParams channels %{public}u  end", streamInfo.channels);
    AUDIO_INFO_LOG("InitParams channelLayout %{public}" PRIu64 " end", streamInfo.channelLayout);
    AUDIO_INFO_LOG("InitParams samplingRate %{public}u  end", streamInfo.samplingRate);
    AUDIO_INFO_LOG("InitParams format %{public}u  end", streamInfo.format);
    AUDIO_INFO_LOG("InitParams frameLen %{public}zu  end", streamInfo.frameLen);
    AUDIO_INFO_LOG("InitParams streamType %{public}u  end", streamInfo.streamType);
    AUDIO_INFO_LOG("InitParams sessionId %{public}u  end", streamInfo.sessionId);
    AUDIO_INFO_LOG("InitParams streamClassType %{public}u  end", streamInfo.streamClassType);
    AUDIO_INFO_LOG("InitParams sourceType %{public}d  end", streamInfo.sourceType);
    int32_t ret = IHpaeManager::GetHpaeManager().CreateStream(streamInfo);
    if (ret != 0) {
        AUDIO_ERR_LOG("CreateStream is error");
        return ERR_INVALID_PARAM;
    }
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::Start()
{
    AUDIO_INFO_LOG("Start");
    int32_t ret = IHpaeManager::GetHpaeManager().Start(HPAE_STREAM_CLASS_TYPE_PLAY, processConfig_.originalSessionId);
    if (ret != 0) {
        AUDIO_ERR_LOG("Start is error");
        return ERR_INVALID_PARAM;
    }
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::Pause(bool isStandby)
{
    AUDIO_INFO_LOG("Pause");
    int32_t ret = IHpaeManager::GetHpaeManager().Pause(HPAE_STREAM_CLASS_TYPE_PLAY, processConfig_.originalSessionId);
    if (ret != 0) {
        AUDIO_ERR_LOG("Pause is error");
        return ERR_INVALID_PARAM;
    }
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::Flush()
{
    AUDIO_PRERELEASE_LOGI("Flush Enter");
    int32_t ret = IHpaeManager::GetHpaeManager().Flush(HPAE_STREAM_CLASS_TYPE_PLAY, processConfig_.originalSessionId);
    if (ret != 0) {
        AUDIO_ERR_LOG("Flush is error");
        return ERR_INVALID_PARAM;
    }
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::Drain(bool stopFlag)
{
    AUDIO_INFO_LOG("Drain Enter %{public}d", stopFlag);
    int32_t ret = IHpaeManager::GetHpaeManager().Drain(HPAE_STREAM_CLASS_TYPE_PLAY, processConfig_.originalSessionId);
    if (ret != 0) {
        AUDIO_ERR_LOG("Drain is error");
        return ERR_INVALID_PARAM;
    }
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::Stop()
{
    AUDIO_INFO_LOG("Stop Enter");
    int32_t ret = IHpaeManager::GetHpaeManager().Stop(HPAE_STREAM_CLASS_TYPE_PLAY, processConfig_.originalSessionId);
    if (ret != 0) {
        AUDIO_ERR_LOG("Stop is error");
        return ERR_INVALID_PARAM;
    }
    state_ = STOPPING;
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::Release()
{
    if (state_ == RUNNING) {
        AUDIO_ERR_LOG("%{public}u Release state_ is RUNNING", processConfig_.originalSessionId);
        IHpaeManager::GetHpaeManager().Stop(HPAE_STREAM_CLASS_TYPE_PLAY, processConfig_.originalSessionId);
    }
    AUDIO_INFO_LOG("Release Enter");
    int32_t ret = IHpaeManager::GetHpaeManager().DestroyStream(HPAE_STREAM_CLASS_TYPE_PLAY, processConfig_.originalSessionId);
    if (ret != 0) {
        AUDIO_ERR_LOG("Release is error");
        return ERR_INVALID_PARAM;
    }
    state_ = RELEASED;
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::GetStreamFramesWritten(uint64_t &framesWritten)
{
    framesWritten = framesWritten_;
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::GetCurrentTimeStamp(uint64_t &timestamp)
{
    std::shared_lock<std::shared_mutex> lock(latencyMutex_);
    timestamp = timestamp_;
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::GetCurrentPosition(uint64_t &framePosition, uint64_t &timestamp, uint64_t &latency)
{
    std::shared_lock<std::shared_mutex> lock(latencyMutex_);
    framePosition = framePosition_;
    timestamp = timestamp_;
    latency = latency_;
    return SUCCESS;
}


int32_t HpaeRendererStreamImpl::GetLatency(uint64_t &latency)
{
    std::shared_lock<std::shared_mutex> lock(latencyMutex_);
    if (deviceClass_ != "offload") {
        uint32_t SinkLatency = 0;
        std::shared_ptr<IAudioRenderSink> audioRendererSink = GetRenderSinkInstance(deviceClass_, deviceNetId_);
        if (audioRendererSink) {
            audioRendererSink->GetLatency(SinkLatency);
        }
        latency = SinkLatency + latency_;
        return SUCCESS;
    }
    timespec tm {};
    clock_gettime(CLOCK_MONOTONIC, &tm);
    auto timestamp = static_cast<uint64_t>(tm.tv_sec) * 1000000000ll + static_cast<uint64_t>(tm.tv_nsec);
    auto interval = (timestamp - timestamp_) / 1000;
    latency = latency_ > interval ? latency_ - interval : 0;
    AUDIO_DEBUG_LOG("HpaeRendererStreamImpl::GetLatency latency_ %{public}" PRIu64 ", \
        interval %{public}llu latency %{public}" PRIu64, latency_, interval, latency);
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::SetRate(int32_t rate)
{
    AUDIO_INFO_LOG("SetRate in");
    renderRate_ = rate;
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::SetAudioEffectMode(int32_t effectMode)
{
    AUDIO_INFO_LOG("SetAudioEffectMode: %d", effectMode);
   
    effectMode_ = effectMode;
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::GetAudioEffectMode(int32_t &effectMode)
{
    effectMode = effectMode_;
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::SetPrivacyType(int32_t privacyType)
{
    AUDIO_DEBUG_LOG("SetInnerCapturerState: %d", privacyType);
    privacyType_ = privacyType;
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::GetPrivacyType(int32_t &privacyType)
{
    privacyType_ = privacyType;
    return SUCCESS;
}


void HpaeRendererStreamImpl::RegisterStatusCallback(const std::weak_ptr<IStatusCallback> &callback)
{
    AUDIO_DEBUG_LOG("RegisterStatusCallback in");
    int32_t ret = IHpaeManager::GetHpaeManager().RegisterStatusCallback(HPAE_STREAM_CLASS_TYPE_PLAY, processConfig_.originalSessionId, callback);
    if (ret != 0) {
        AUDIO_ERR_LOG("RegisterStatusCallback is error");
        return;
    }
    statusCallback_ = callback;
}

void HpaeRendererStreamImpl::RegisterWriteCallback(const std::weak_ptr<IWriteCallback> &callback)
{
    AUDIO_DEBUG_LOG("RegisterWriteCallback in");
    int32_t ret = IHpaeManager::GetHpaeManager().RegisterWriteCallback(processConfig_.originalSessionId, shared_from_this());
    if (ret != 0) {
        AUDIO_ERR_LOG("RegisterStatusCallback is error");
        return;
    }
    writeCallback_ = callback;
}

int32_t HpaeRendererStreamImpl::OnStreamData(AudioCallBackStreamInfo& callBackStreamInfo)
{
    {
        std::unique_lock<std::shared_mutex> lock(latencyMutex_);
        framePosition_ = callBackStreamInfo.framePosition;
        timestamp_ = callBackStreamInfo.timestamp;
        latency_ = callBackStreamInfo.latency;
        framesWritten_ = callBackStreamInfo.framesWritten;
        deviceClass_ = callBackStreamInfo.deviceClass;
        deviceNetId_ = callBackStreamInfo.deviceNetId;
    }
    if (callBackStreamInfo.needData && writeCallback_.lock()) {
        return writeCallback_.lock()->OnWriteData(callBackStreamInfo.inputData, callBackStreamInfo.requestDataLen);
    }
    return SUCCESS;
}

BufferDesc HpaeRendererStreamImpl::DequeueBuffer(size_t length)
{
    BufferDesc bufferDesc;
    return bufferDesc;
}

int32_t HpaeRendererStreamImpl::EnqueueBuffer(const BufferDesc &bufferDesc)
{
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::GetMinimumBufferSize(size_t &minBufferSize) const
{
    minBufferSize = minBufferSize_;
    return SUCCESS;
}

void HpaeRendererStreamImpl::GetByteSizePerFrame(size_t &byteSizePerFrame) const
{
    byteSizePerFrame = byteSizePerFrame_;
}

void HpaeRendererStreamImpl::GetSpanSizePerFrame(size_t &spanSizeInFrame) const
{
    spanSizeInFrame = spanSizeInFrame_;
}

void HpaeRendererStreamImpl::SetStreamIndex(uint32_t index)
{
    AUDIO_INFO_LOG("Using index/sessionId %d", index);
    streamIndex_ = index;
}

uint32_t HpaeRendererStreamImpl::GetStreamIndex()
{
    return streamIndex_;
}

void HpaeRendererStreamImpl::AbortCallback(int32_t abortTimes)
{
    abortFlag_ += abortTimes;
}

// offload

size_t HpaeRendererStreamImpl::GetWritableSize()
{
    return 0;
}

int32_t HpaeRendererStreamImpl::OffloadSetVolume(float volume)
{
    std::shared_ptr<IAudioRenderSink> audioRendererSinkInstance = GetRenderSinkInstance("offload", "");
    if (audioRendererSinkInstance == nullptr) {
        AUDIO_ERR_LOG("Renderer is null.");
        return ERROR;
    }
    return audioRendererSinkInstance->SetVolume(volume, volume);
}

int32_t HpaeRendererStreamImpl::UpdateSpatializationState(bool spatializationEnabled, bool headTrackingEnabled)
{
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::GetOffloadApproximatelyCacheTime(uint64_t &timestamp, uint64_t &paWriteIndex,
    uint64_t &cacheTimeDsp, uint64_t &cacheTimePa)
{
    return SUCCESS;
}

void HpaeRendererStreamImpl::SyncOffloadMode()
{
    std::shared_ptr<IStatusCallback> statusCallback = statusCallback_.lock();
    if (statusCallback != nullptr) {
        if (offloadEnable_) {
            statusCallback->OnStatusUpdate(OPERATION_SET_OFFLOAD_ENABLE);
        } else {
            statusCallback->OnStatusUpdate(OPERATION_UNSET_OFFLOAD_ENABLE);
        }
    }
}

int32_t HpaeRendererStreamImpl::SetOffloadMode(int32_t state, bool isAppBack)
{
#ifdef FEATURE_POWER_MANAGER
    static const std::set<PowerMgr::PowerState> screenOffTable = {
        PowerMgr::PowerState::INACTIVE, PowerMgr::PowerState::STAND_BY,
        PowerMgr::PowerState::DOZE, PowerMgr::PowerState::SLEEP,
        PowerMgr::PowerState::HIBERNATE,
    };
    AudioOffloadType statePolicy = OFFLOAD_DEFAULT;
    statePolicy = screenOffTable.count(static_cast<PowerMgr::PowerState>(state)) ?
        OFFLOAD_INACTIVE_BACKGROUND : OFFLOAD_ACTIVE_FOREGROUND;

    AUDIO_INFO_LOG("calling set stream offloadMode PowerState: %{public}d, isAppBack: %{public}d", state, isAppBack);

    if (offloadStatePolicy_.load() == statePolicy) {
        return SUCCESS;
    }

    offloadEnable_ = true;
    SyncOffloadMode();
    auto ret = IHpaeManager::GetHpaeManager().SetOffloadPolicy(processConfig_.originalSessionId, statePolicy);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED,
        "SetOffloadPolicy failed, errcode is %{public}d", ret);
    offloadStatePolicy_.store(statePolicy);
#else
    AUDIO_INFO_LOG("SetStreamOffloadMode not available, FEATURE_POWER_MANAGER no define");
#endif
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::UnsetOffloadMode()
{
    offloadEnable_ = false;
    SyncOffloadMode();
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::UpdateMaxLength(uint32_t maxLength)
{
    return SUCCESS;
}

AudioProcessConfig HpaeRendererStreamImpl::GetAudioProcessConfig() const noexcept
{
    return processConfig_;
}

int32_t HpaeRendererStreamImpl::Peek(std::vector<char> *audioBuffer, int32_t &index)
{
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::ReturnIndex(int32_t index)
{
    return SUCCESS;
}

void HpaeRendererStreamImpl::BlockStream() noexcept
{
    return;
}
// offload end

int32_t HpaeRendererStreamImpl::SetClientVolume(float clientVolume)
{
    
    AUDIO_PRERELEASE_LOGI("set client volume success");
    int32_t ret = IHpaeManager::GetHpaeManager().SetClientVolume(processConfig_.originalSessionId, clientVolume);
    if (ret != 0) {
        AUDIO_ERR_LOG("SetClientVolume is error");
        return ERR_INVALID_PARAM;
    }
    clientVolume_ = clientVolume;
    return SUCCESS;
}

static AudioChannelLayout SetDefaultChannelLayout(AudioChannel channels)
{
    if (channels < MONO || channels > CHANNEL_16) {
        return CH_LAYOUT_UNKNOWN;
    }
    switch (channels) {
        case MONO:
            return CH_LAYOUT_MONO;
        case STEREO:
            return CH_LAYOUT_STEREO;
        case CHANNEL_3:
            return CH_LAYOUT_SURROUND;
        case CHANNEL_4:
            return CH_LAYOUT_3POINT1;
        case CHANNEL_5:
            return CH_LAYOUT_4POINT1;
        case CHANNEL_6:
            return CH_LAYOUT_5POINT1;
        case CHANNEL_7:
            return CH_LAYOUT_6POINT1;
        case CHANNEL_8:
            return CH_LAYOUT_5POINT1POINT2;
        case CHANNEL_10:
            return CH_LAYOUT_7POINT1POINT2;
        case CHANNEL_12:
            return CH_LAYOUT_7POINT1POINT4;
        case CHANNEL_14:
            return CH_LAYOUT_9POINT1POINT4;
        case CHANNEL_16:
            return CH_LAYOUT_9POINT1POINT6;
        default:
            return CH_LAYOUT_UNKNOWN;
    }
}

static std::shared_ptr<IAudioRenderSink> GetRenderSinkInstance(std::string deviceClass, std::string deviceNetId)
{
    uint32_t renderId = HDI_INVALID_ID;
    renderId = HdiAdapterManager::GetInstance().GetRenderIdByDeviceClass(deviceClass,
        deviceNetId.empty() ? HDI_ID_INFO_DEFAULT : deviceNetId, false);
    return HdiAdapterManager::GetInstance().GetRenderSink(renderId, true);
}
} // namespace AudioStandard
} // namespace OHOS
