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
#include <thread>
#include "safe_map.h"
#include "audio_errors.h"
#include "audio_utils.h"
#include "i_hpae_manager.h"
#include "audio_stream_info.h"
#include "audio_effect_map.h"
#include "down_mixer.h"
#include "policy_handler.h"
#include "audio_engine_log.h"

using namespace OHOS::AudioStandard::HPAE;
namespace OHOS {
namespace AudioStandard {

static constexpr int32_t MIN_BUFFER_SIZE = 2;
static constexpr uint64_t FRAME_LEN_10MS = 10;
static constexpr uint64_t FRAME_LEN_20MS = 20;
static constexpr uint64_t FRAME_LEN_40MS = 40;
static const std::string DEVICE_CLASS_OFFLOAD = "offload";
static const std::string DEVICE_CLASS_REMOTE_OFFLOAD = "remote_offload";
static std::shared_ptr<IAudioRenderSink> GetRenderSinkInstance(std::string deviceClass, std::string deviceNetId);
static inline FadeType GetFadeType(uint64_t expectedPlaybackDurationMs);
HpaeRendererStreamImpl::HpaeRendererStreamImpl(AudioProcessConfig processConfig, bool isMoveAble, bool isCallbackMode)
{
    processConfig_ = processConfig;
    spanSizeInFrame_ = processConfig.streamInfo.samplingRate == SAMPLE_RATE_11025 ?
        FRAME_LEN_40MS * static_cast<uint32_t>(processConfig.streamInfo.samplingRate) / AUDIO_MS_PER_S :
        FRAME_LEN_20MS * static_cast<uint32_t>(processConfig.streamInfo.samplingRate) / AUDIO_MS_PER_S;
    byteSizePerFrame_ = (processConfig.streamInfo.channels *
        static_cast<size_t>(GetSizeFromFormat(processConfig.streamInfo.format)));
    minBufferSize_ = MIN_BUFFER_SIZE * byteSizePerFrame_ * spanSizeInFrame_;
    if (byteSizePerFrame_ == 0 || processConfig.streamInfo.samplingRate == 0) {
        expectedPlaybackDurationMs_ = 0;
    } else {
        expectedPlaybackDurationMs_ =
            (processConfig.rendererInfo.expectedPlaybackDurationBytes * AUDIO_MS_PER_S / byteSizePerFrame_) /
                processConfig.streamInfo.samplingRate;
    }
    isCallbackMode_ = isCallbackMode;
    isMoveAble_ = isMoveAble;
    if (!isCallbackMode_) {
        InitRingBuffer();
    }
}
HpaeRendererStreamImpl::~HpaeRendererStreamImpl()
{
    AUDIO_INFO_LOG("destructor [%{public}u]", streamIndex_);
    if (dumpEnqueueIn_ != nullptr) {
        DumpFileUtil::CloseDumpFile(&dumpEnqueueIn_);
    }
}

int32_t HpaeRendererStreamImpl::InitParams(const std::string &deviceName)
{
    HpaeStreamInfo streamInfo;
    streamInfo.channels = processConfig_.streamInfo.channels;
    streamInfo.samplingRate = processConfig_.streamInfo.samplingRate;
    streamInfo.format = processConfig_.streamInfo.format;
    streamInfo.channelLayout = processConfig_.streamInfo.channelLayout;
    if (streamInfo.channelLayout == CH_LAYOUT_UNKNOWN) {
        streamInfo.channelLayout = DownMixer::SetDefaultChannelLayout((AudioChannel)streamInfo.channels);
    }
    streamInfo.frameLen = spanSizeInFrame_;
    streamInfo.sessionId = processConfig_.originalSessionId;
    streamInfo.streamType = processConfig_.streamType;
    streamInfo.fadeType = GetFadeType(expectedPlaybackDurationMs_);
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    streamInfo.uid = processConfig_.appInfo.appUid;
    streamInfo.pid = processConfig_.appInfo.appPid;
    streamInfo.tokenId = processConfig_.appInfo.appTokenId;
    effectMode_ = processConfig_.rendererInfo.effectMode;
    streamInfo.effectInfo.effectMode = (effectMode_ != EFFECT_DEFAULT && effectMode_ != EFFECT_NONE) ? EFFECT_DEFAULT :
        static_cast<AudioEffectMode>(effectMode_);
    const std::unordered_map<AudioEffectScene, std::string> &audioSupportedSceneTypes = GetSupportedSceneType();
    streamInfo.effectInfo.effectScene = static_cast<AudioEffectScene>(GetKeyFromValue(
        audioSupportedSceneTypes, processConfig_.rendererInfo.sceneType));
    streamInfo.effectInfo.systemVolumeType = VolumeUtils::GetVolumeTypeFromStreamType(processConfig_.streamType);
    streamInfo.effectInfo.streamUsage = processConfig_.rendererInfo.streamUsage;
    streamInfo.sourceType = processConfig_.isInnerCapturer == true ? SOURCE_TYPE_PLAYBACK_CAPTURE : SOURCE_TYPE_INVALID;
    streamInfo.deviceName = deviceName;
    streamInfo.isMoveAble = isMoveAble_;
    streamInfo.privacyType = processConfig_.privacyType;
    AUDIO_INFO_LOG("channels %{public}u channelLayout %{public}" PRIu64 " samplingRate %{public}u format %{public}u "
        "frameLen %{public}zu streamType %{public}u sessionId %{public}u streamClassType %{public}u "
        "sourceType %{public}d fadeType %{public}d", streamInfo.channels, streamInfo.channelLayout,
        streamInfo.samplingRate, streamInfo.format, streamInfo.frameLen, streamInfo.streamType,
        streamInfo.sessionId, streamInfo.streamClassType, streamInfo.sourceType, streamInfo.fadeType);
    auto &hpaeManager = IHpaeManager::GetHpaeManager();
    int32_t ret = hpaeManager.CreateStream(streamInfo);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_INVALID_PARAM, "CreateStream is error");

    // Register Callback
    ret = hpaeManager.RegisterStatusCallback(HPAE_STREAM_CLASS_TYPE_PLAY, streamInfo.sessionId, shared_from_this());
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_INVALID_PARAM, "RegisterStatusCallback is error");
    ret = hpaeManager.RegisterWriteCallback(streamInfo.sessionId, shared_from_this());
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_INVALID_PARAM, "RegisterWriteCallback is error");
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::Start()
{
    AUDIO_INFO_LOG("[%{public}u] Enter", streamIndex_);
    ClockTime::GetAllTimeStamp(timestamp_);
    int32_t ret = IHpaeManager::GetHpaeManager().Start(HPAE_STREAM_CLASS_TYPE_PLAY, processConfig_.originalSessionId);
    std::string tempStringSessionId = std::to_string(streamIndex_);
    IHpaeManager::GetHpaeManager().AddStreamVolumeToEffect(tempStringSessionId, clientVolume_);
    if (ret != 0) {
        AUDIO_ERR_LOG("ErrorCode: %{public}d", ret);
        return ERR_INVALID_PARAM;
    }
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::StartWithSyncId(const int32_t &syncId)
{
    AUDIO_INFO_LOG("[%{public}u] Enter syncId: %{public}d", streamIndex_, syncId);
    ClockTime::GetAllTimeStamp(timestamp_);
    int32_t ret = IHpaeManager::GetHpaeManager().StartWithSyncId(HPAE_STREAM_CLASS_TYPE_PLAY,
        processConfig_.originalSessionId, syncId);
    if (ret != 0) {
        AUDIO_ERR_LOG("ErrorCode: %{public}d", ret);
        return ERR_INVALID_PARAM;
    }
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::Pause(bool isStandby)
{
    AUDIO_INFO_LOG("[%{public}u] Enter", streamIndex_);
    int32_t ret = IHpaeManager::GetHpaeManager().Pause(HPAE_STREAM_CLASS_TYPE_PLAY, processConfig_.originalSessionId);
    if (ret != 0) {
        AUDIO_ERR_LOG("ErrorCode: %{public}d", ret);
        return ERR_INVALID_PARAM;
    }
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::Flush()
{
    AUDIO_INFO_LOG("[%{public}u] Enter", streamIndex_);
    int32_t ret = IHpaeManager::GetHpaeManager().Flush(HPAE_STREAM_CLASS_TYPE_PLAY, processConfig_.originalSessionId);
    if (ret != 0) {
        AUDIO_ERR_LOG("ErrorCode: %{public}d", ret);
        return ERR_INVALID_PARAM;
    }
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::Drain(bool stopFlag)
{
    AUDIO_INFO_LOG("[%{public}u] Enter %{public}d", streamIndex_, stopFlag);
    int32_t ret = IHpaeManager::GetHpaeManager().Drain(HPAE_STREAM_CLASS_TYPE_PLAY, processConfig_.originalSessionId);
    if (ret != 0) {
        AUDIO_ERR_LOG("ErrorCode: %{public}d", ret);
        return ERR_INVALID_PARAM;
    }
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::Stop()
{
    AUDIO_INFO_LOG("[%{public}u] Enter", streamIndex_);
    int32_t ret = IHpaeManager::GetHpaeManager().Stop(HPAE_STREAM_CLASS_TYPE_PLAY, processConfig_.originalSessionId);
    if (ret != 0) {
        AUDIO_ERR_LOG("ErrorCode: %{public}d", ret);
        return ERR_INVALID_PARAM;
    }
    state_ = STOPPING;
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::Release()
{
    if (state_ == RUNNING) {
        AUDIO_ERR_LOG("%{public}u state_ is RUNNING", processConfig_.originalSessionId);
        IHpaeManager::GetHpaeManager().Stop(HPAE_STREAM_CLASS_TYPE_PLAY, processConfig_.originalSessionId);
    }
    AUDIO_INFO_LOG("[%{public}u] Enter", streamIndex_);
    int32_t ret = IHpaeManager::GetHpaeManager().DestroyStream(HPAE_STREAM_CLASS_TYPE_PLAY,
        processConfig_.originalSessionId);
    std::string tempStringSessionId = std::to_string(streamIndex_);
    IHpaeManager::GetHpaeManager().DeleteStreamVolumeToEffect(tempStringSessionId);
    if (ret != 0) {
        AUDIO_ERR_LOG("ErrorCode: %{public}d", ret);
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
    timestamp = timestamp_[Timestamp::Timestampbase::MONOTONIC];
    return SUCCESS;
}

uint32_t HpaeRendererStreamImpl::GetA2dpOffloadLatency()
{
    Trace trace("PaRendererStreamImpl::GetA2dpOffloadLatency");
    uint32_t a2dpOffloadLatency = 0;
    uint64_t a2dpOffloadSendDataSize = 0;
    uint32_t a2dpOffloadTimestamp = 0;
    auto& handle = PolicyHandler::GetInstance();
    int32_t ret = handle.OffloadGetRenderPosition(a2dpOffloadLatency, a2dpOffloadSendDataSize, a2dpOffloadTimestamp);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("OffloadGetRenderPosition failed!");
    }
    return a2dpOffloadLatency;
}

uint32_t HpaeRendererStreamImpl::GetNearlinkLatency()
{
    Trace trace("PaRendererStreamImpl::GetNearlinkLatency");
    uint32_t nearlinkLatency = 0;
    auto &handler = PolicyHandler::GetInstance();
    int32_t ret = handler.NearlinkGetRenderPosition(nearlinkLatency);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, 0, "NearlinkGetRenderPosition failed");

    return nearlinkLatency;
}

int32_t HpaeRendererStreamImpl::GetRemoteOffloadSpeedPosition(uint64_t &framePosition, uint64_t &timestamp,
    uint64_t &latency)
{
    CHECK_AND_RETURN_RET(deviceClass_ == DEVICE_CLASS_REMOTE_OFFLOAD, ERR_NOT_SUPPORTED);

    std::shared_ptr<IAudioRenderSink> sink = GetRenderSinkInstance(deviceClass_, deviceNetId_);
    CHECK_AND_RETURN_RET_LOG(sink != nullptr, ERR_INVALID_OPERATION, "audioRendererSink is null");
    uint64_t framesUS;
    int64_t timeSec;
    int64_t timeNSec;
    int32_t ret = sink->GetHdiPresentationPosition(framesUS, timeSec, timeNSec);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "get position fail");

    uint32_t curLatencyUS = 0;
    ret = sink->GetHdiLatency(curLatencyUS);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "get latency fail");

    // Here, latency and sampling count are calculated, and latency is exposed to the client as 0.
    latency = static_cast<uint64_t>(curLatencyUS) * processConfig_.streamInfo.samplingRate / AUDIO_US_PER_S;

    uint64_t frames = framesUS * processConfig_.streamInfo.samplingRate / AUDIO_US_PER_S;
    framePosition = lastHdiFramePosition_ + frames;
    timestamp = static_cast<uint64_t>(ClockTime::GetCurNano());
    AUDIO_DEBUG_LOG("frame: %{public}" PRIu64, framePosition);
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::GetSpeedPosition(uint64_t &framePosition, uint64_t &timestamp,
    uint64_t &latency, int32_t base)
{
    std::shared_lock<std::shared_mutex> lock(latencyMutex_);

    int32_t ret = GetRemoteOffloadSpeedPosition(framePosition, timestamp, latency);
    CHECK_AND_RETURN_RET(ret == ERR_NOT_SUPPORTED, ret);

    framePosition = lastHdiFramePosition_ + framePosition_ - lastFramePosition_;

    uint64_t latencyUs = 0;
    GetLatencyInner(timestamp, latencyUs, base);
    latency = latencyUs * static_cast<uint64_t>(processConfig_.streamInfo.samplingRate) / AUDIO_US_PER_S;
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::GetCurrentPosition(uint64_t &framePosition, uint64_t &timestamp,
    uint64_t &latency, int32_t base)
{
    std::shared_lock<std::shared_mutex> lock(latencyMutex_);
    uint64_t latencyUs = 0;
    GetLatencyInner(timestamp, latencyUs, base);
    latency = latencyUs * static_cast<uint64_t>(processConfig_.streamInfo.samplingRate) / AUDIO_US_PER_S;
    framePosition = framePosition_;
    AUDIO_DEBUG_LOG("HpaeRendererStreamImpl::GetCurrentPosition Latency info: framePosition: %{public}" PRIu64
        ", latency %{public}" PRIu64, framePosition, latency);
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::GetLatency(uint64_t &latency)
{
    std::shared_lock<std::shared_mutex> lock(latencyMutex_);
    uint64_t timestamp = 0;
    int32_t base = Timestamp::Timestampbase::MONOTONIC;
    GetLatencyInner(timestamp, latency, base);
    return SUCCESS;
}
void HpaeRendererStreamImpl::GetLatencyInner(uint64_t &timestamp, uint64_t &latencyUs, int32_t base)
{
    int32_t baseUsed = base >= 0 && base < Timestamp::Timestampbase::BASESIZE ?
        base : Timestamp::Timestampbase::MONOTONIC;
    uint32_t sinkLatency = 0;
    uint32_t a2dpOffloadLatency = GetA2dpOffloadLatency();
    uint32_t nearlinkLatency = GetNearlinkLatency();
    std::shared_ptr<IAudioRenderSink> audioRendererSink = GetRenderSinkInstance(deviceClass_, deviceNetId_);
    if (audioRendererSink) {
        audioRendererSink->GetLatency(sinkLatency);
    }
    latencyUs = latency_;
    latencyUs += sinkLatency * AUDIO_US_PER_MS;
    latencyUs += a2dpOffloadLatency * AUDIO_US_PER_MS;
    latencyUs += nearlinkLatency * AUDIO_US_PER_MS;
    std::vector<uint64_t> timestampCurrent = {0};
    ClockTime::GetAllTimeStamp(timestampCurrent);
    timestamp = timestampCurrent[baseUsed];

    AUDIO_DEBUG_LOG("Latency info: framePosition: %{public}" PRIu64 ", latencyUs %{public}" PRIu64
        ", base %{public}d, timestamp %{public}" PRIu64 ", pipe latency: %{public}" PRIu64
        ", sink latency: %{public}u ms, a2dp offload latency: %{public}u ms, nearlink latency: %{public}u ms",
        framePosition_, latencyUs, base, timestamp, latency_, sinkLatency, a2dpOffloadLatency, nearlinkLatency);
}

int32_t HpaeRendererStreamImpl::SetRate(int32_t rate)
{
    AUDIO_INFO_LOG("SetRate in");
    renderRate_ = rate;
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::SetAudioEffectMode(int32_t effectMode)
{
    AUDIO_INFO_LOG("effectMode: %{public}d", effectMode);
    int32_t ret = IHpaeManager::GetHpaeManager().SetAudioEffectMode(processConfig_.originalSessionId, effectMode);
    if (ret != 0) {
        AUDIO_ERR_LOG("ErrorCode: %{public}d", ret);
        return ERR_INVALID_PARAM;
    }
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
    AUDIO_DEBUG_LOG("privacyType: %{public}d", privacyType);
    privacyType_ = privacyType;
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::GetPrivacyType(int32_t &privacyType)
{
    privacyType_ = privacyType;
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::SetSpeed(float speed)
{
    AUDIO_INFO_LOG("[%{public}u] Enter", streamIndex_);
    IHpaeManager::GetHpaeManager().SetSpeed(processConfig_.originalSessionId, speed);
    return SUCCESS;
}

void HpaeRendererStreamImpl::RegisterStatusCallback(const std::weak_ptr<IStatusCallback> &callback)
{
    AUDIO_DEBUG_LOG("RegisterStatusCallback in");
    statusCallback_ = callback;
}

void HpaeRendererStreamImpl::RegisterWriteCallback(const std::weak_ptr<IWriteCallback> &callback)
{
    AUDIO_DEBUG_LOG("RegisterWriteCallback in");
    writeCallback_ = callback;
}

void HpaeRendererStreamImpl::OnDeviceClassChange(const AudioCallBackStreamInfo &callBackStreamInfo)
{
    if (deviceClass_ != callBackStreamInfo.deviceClass) {
        uint64_t newFramePosition = callBackStreamInfo.framePosition;

        // from normal to remote offload
        if (callBackStreamInfo.deviceClass == DEVICE_CLASS_REMOTE_OFFLOAD) {
            uint64_t duration = newFramePosition > lastFramePosition_ ? newFramePosition - lastFramePosition_ :
                lastFramePosition_ - newFramePosition;
            lastHdiFramePosition_ = newFramePosition > lastFramePosition_ ? lastHdiFramePosition_ + duration :
                (lastHdiFramePosition_ > duration ? lastHdiFramePosition_ - duration : 0);
        }
        // Device type switch, replace lastFramePosition_
        lastFramePosition_ = callBackStreamInfo.framePosition;
    }

    // If hdiFramePosition has a value, it indicates that the remote offload device has performed a flush.
    // The value of hdiFramePosition needs to be accumulated into lastHdiFramePosition_
    if (callBackStreamInfo.hdiFramePosition > 0) {
        lastHdiFramePosition_ +=
            // from time (us) to sample
            callBackStreamInfo.hdiFramePosition * processConfig_.streamInfo.samplingRate / AUDIO_US_PER_S;
    }
}

int32_t HpaeRendererStreamImpl::OnStreamData(AudioCallBackStreamInfo &callBackStreamInfo)
{
    {
        std::unique_lock<std::shared_mutex> lock(latencyMutex_);
        OnDeviceClassChange(callBackStreamInfo);
        framePosition_ = callBackStreamInfo.framePosition;
        timestamp_ = callBackStreamInfo.timestamp;
        latency_ = callBackStreamInfo.latency;
        framesWritten_ = callBackStreamInfo.framesWritten;
        deviceClass_ = callBackStreamInfo.deviceClass;
        deviceNetId_ = callBackStreamInfo.deviceNetId;
    }
    if (isCallbackMode_) { // callback buffer
        auto requestDataLen = callBackStreamInfo.requestDataLen;
        auto writeCallback = writeCallback_.lock();
        if (callBackStreamInfo.needData && writeCallback) {
            writeCallback->GetAvailableSize(requestDataLen);
            requestDataLen = std::min(requestDataLen, callBackStreamInfo.requestDataLen);
            if (callBackStreamInfo.requestDataLen > requestDataLen) {
                int chToFill = (processConfig_.streamInfo.format == SAMPLE_U8) ? 0x7f : 0;
                memset_s(callBackStreamInfo.inputData + requestDataLen,
                    callBackStreamInfo.requestDataLen - requestDataLen, chToFill,
                    callBackStreamInfo.requestDataLen - requestDataLen);
                requestDataLen = callBackStreamInfo.forceData ? requestDataLen : 0;
            }
            callBackStreamInfo.requestDataLen = requestDataLen;
            return writeCallback->OnWriteData(callBackStreamInfo.inputData,
                requestDataLen);
        }
    } else { // write buffer
        return WriteDataFromRingBuffer(callBackStreamInfo.forceData,
            callBackStreamInfo.inputData, callBackStreamInfo.requestDataLen);
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
    CHECK_AND_RETURN_RET_LOG(!isCallbackMode_, ERROR, "Not write buffer mode");
    CHECK_AND_RETURN_RET_LOG(ringBuffer_ != nullptr, ERROR, "RingBuffer is nullptr");

    size_t targetSize = bufferDesc.bufLength;
    OptResult result = ringBuffer_->GetWritableSize();
    CHECK_AND_RETURN_RET_LOG(result.ret == OPERATION_SUCCESS, ERROR,
        "Get writable size failed, ret:%{public}d size:%{public}zu", result.ret, result.size);

    size_t writableSize = result.size;
    if (targetSize > writableSize) {
        AUDIO_ERR_LOG("overflow, targetSize: %{public}zu, writableSize: %{public}zu",
            targetSize, writableSize);
    }

    size_t writeSize = std::min(writableSize, targetSize);
    BufferWrap bufferWrap = {bufferDesc.buffer, writeSize};
    result = ringBuffer_->Enqueue(bufferWrap);
    if (result.ret != OPERATION_SUCCESS) {
        AUDIO_ERR_LOG("failed, ret:%{public}d size:%{public}zu", result.ret, result.size);
        return ERROR;
    }
    DumpFileUtil::WriteDumpFile(dumpEnqueueIn_, bufferDesc.buffer, writeSize);
    return writeSize; // success return written in length
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
    AUDIO_INFO_LOG("Using index/sessionId %{public}u", index);
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
    if (!offloadEnable_) {
        return ERR_OPERATION_FAILED;
    }
    std::shared_ptr<IAudioRenderSink> audioRendererSinkInstance = GetRenderSinkInstance(deviceClass_, "");
    if (audioRendererSinkInstance == nullptr) {
        AUDIO_ERR_LOG("Renderer is null.");
        return ERROR;
    }
    return audioRendererSinkInstance->SetVolume(volume, volume);
}

int32_t HpaeRendererStreamImpl::SetOffloadDataCallbackState(int32_t state)
{
    AUDIO_INFO_LOG("state: %{public}d", state);
    if (!offloadEnable_) {
        return ERR_OPERATION_FAILED;
    }
    return IHpaeManager::GetHpaeManager().SetOffloadRenderCallbackType(processConfig_.originalSessionId, state);
}

int32_t HpaeRendererStreamImpl::UpdateSpatializationState(bool spatializationEnabled, bool headTrackingEnabled)
{
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::GetOffloadApproximatelyCacheTime(uint64_t &timestamp, uint64_t &paWriteIndex,
    uint64_t &cacheTimeDsp, uint64_t &cacheTimePa)
{
    if (!offloadEnable_) {
        return ERR_OPERATION_FAILED;
    }
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

    if (offloadStatePolicy_.load() == statePolicy && offloadEnable_) {
        return SUCCESS;
    }

    offloadEnable_ = true;
    SyncOffloadMode();
    auto ret = IHpaeManager::GetHpaeManager().SetOffloadPolicy(processConfig_.originalSessionId, statePolicy);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED,
        "SetOffloadPolicy failed, errcode is %{public}d", ret);
    offloadStatePolicy_.store(statePolicy);
#else
    AUDIO_INFO_LOG("not available, FEATURE_POWER_MANAGER no define");
#endif
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::UnsetOffloadMode()
{
    offloadEnable_ = false;
    SyncOffloadMode();
    IHpaeManager::GetHpaeManager().SetOffloadPolicy(processConfig_.originalSessionId, OFFLOAD_DEFAULT);
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::UpdateMaxLength(uint32_t maxLength)
{
    size_t bufferSize = maxLength * spanSizeInFrame_ * byteSizePerFrame_;
    AUDIO_INFO_LOG("bufferSize: %{public}zu, spanSizeInFrame: %{public}zu, byteSizePerFrame: %{public}zu,"
        "maxLength:%{public}u", bufferSize, spanSizeInFrame_, byteSizePerFrame_, maxLength);
    if (ringBuffer_ != nullptr) {
        ringBuffer_->ReConfig(bufferSize, false);
    } else {
        AUDIO_ERR_LOG("ring buffer is nullptr!");
    }
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
    if (clientVolume < MIN_FLOAT_VOLUME || clientVolume > MAX_FLOAT_VOLUME) {
        AUDIO_ERR_LOG("invalid clientVolume %{public}f", clientVolume);
        return ERR_INVALID_PARAM;
    }
    AUDIO_PRERELEASE_LOGI("clientVolume %{public}f", clientVolume);
    int32_t ret = IHpaeManager::GetHpaeManager().SetClientVolume(processConfig_.originalSessionId, clientVolume);
    std::string tempStringSessionId = std::to_string(processConfig_.originalSessionId);
    IHpaeManager::GetHpaeManager().AddStreamVolumeToEffect(tempStringSessionId, clientVolume);
    if (ret != 0) {
        AUDIO_ERR_LOG("ErrorCode: %{public}d", ret);
        return ERR_INVALID_PARAM;
    }
    clientVolume_ = clientVolume;
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::SetLoudnessGain(float loudnessGain)
{
    AUDIO_INFO_LOG("loudnessGain: %{public}f", loudnessGain);
    int32_t ret = IHpaeManager::GetHpaeManager().SetLoudnessGain(processConfig_.originalSessionId, loudnessGain);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_INVALID_PARAM, "ErrorCode: %{public}d", ret);
    return SUCCESS;
}

void HpaeRendererStreamImpl::InitRingBuffer()
{
    uint32_t maxLength = 20; // 20 for dup and dual play, only for enqueue buffer
    size_t bufferSize = maxLength * spanSizeInFrame_ * byteSizePerFrame_;
    AUDIO_INFO_LOG("bufferSize: %{public}zu, spanSizeInFrame: %{public}zu, byteSizePerFrame: %{public}zu,"
        "maxLength:%{public}u", bufferSize, spanSizeInFrame_, byteSizePerFrame_, maxLength);
    // create ring buffer
    ringBuffer_ = AudioRingCache::Create(bufferSize);
    if (ringBuffer_ == nullptr) {
        AUDIO_ERR_LOG("Create ring buffer failed!");
    }

    std::string dumpEnqueueInFileName = std::to_string(processConfig_.originalSessionId) + "_dual_in_" +
        std::to_string(processConfig_.streamInfo.samplingRate) + "_" +
        std::to_string(processConfig_.streamInfo.channels) + "_" +
        std::to_string(processConfig_.streamInfo.format) + ".pcm";
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, dumpEnqueueInFileName, &dumpEnqueueIn_);
}

int32_t HpaeRendererStreamImpl::WriteDataFromRingBuffer(bool forceData, int8_t *inputData, size_t &requestDataLen)
{
    CHECK_AND_RETURN_RET_LOG(inputData != nullptr, ERROR, "inputData is nullptr");
    CHECK_AND_RETURN_RET_LOG(ringBuffer_ != nullptr, ERROR, "RingBuffer is nullptr");
    OptResult result = ringBuffer_->GetReadableSize();
    CHECK_AND_RETURN_RET_LOG(result.ret == OPERATION_SUCCESS, ERROR,
        "RingBuffer get readable size failed, size is:%{public}zu", result.size);
    CHECK_AND_RETURN_RET_LOG(result.size != 0, ERROR,
        "Readable size is invalid, result.size:%{public}zu, requestDataLen:%{public}zu, buffer underflow.",
        result.size, requestDataLen);
    if (requestDataLen > result.size) {
        CHECK_AND_RETURN_RET_LOG(forceData, ERROR, "not enough data");
        int chToFill = (processConfig_.streamInfo.format == SAMPLE_U8) ? 0x7f : 0;
        memset_s(inputData + result.size,
            requestDataLen - result.size, chToFill,
            requestDataLen - result.size);
    }
    AUDIO_DEBUG_LOG("requestDataLen is:%{public}zu readSize is:%{public}zu", requestDataLen, result.size);
    requestDataLen = std::min(requestDataLen, result.size);
    result = ringBuffer_->Dequeue({reinterpret_cast<uint8_t *>(inputData), requestDataLen});
    CHECK_AND_RETURN_RET_LOG(result.ret == OPERATION_SUCCESS, ERROR, "RingBuffer dequeue failed");
    return SUCCESS;
}

void HpaeRendererStreamImpl::OnStatusUpdate(IOperation operation, uint32_t streamIndex)
{
    auto statusCallback = statusCallback_.lock();
    if (statusCallback) {
        statusCallback->OnStatusUpdate(operation);
    }
}

static std::shared_ptr<IAudioRenderSink> GetRenderSinkInstance(std::string deviceClass, std::string deviceNetId)
{
    uint32_t renderId = HDI_INVALID_ID;
    renderId = HdiAdapterManager::GetInstance().GetRenderIdByDeviceClass(deviceClass,
        deviceNetId.empty() ? HDI_ID_INFO_DEFAULT : deviceNetId, false);
    return HdiAdapterManager::GetInstance().GetRenderSink(renderId, true);
}

static inline FadeType GetFadeType(uint64_t expectedPlaybackDurationMs)
{
    // duration <= 10 ms no fade
    if (expectedPlaybackDurationMs <= FRAME_LEN_10MS && expectedPlaybackDurationMs > 0) {
        return NONE_FADE;
    }

    // duration > 10ms && duration <= 40ms do 5ms fade
    if (expectedPlaybackDurationMs <= FRAME_LEN_40MS && expectedPlaybackDurationMs > FRAME_LEN_10MS) {
        return SHORT_FADE;
    }

    // 0 is default; duration > 40ms do default fade
    return DEFAULT_FADE;
}
} // namespace AudioStandard
} // namespace OHOS
