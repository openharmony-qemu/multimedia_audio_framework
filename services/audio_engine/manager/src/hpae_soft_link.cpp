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
#define LOG_TAG "HpaeSoftLink"
#endif

#include "hpae_soft_link.h"
#ifdef ENABLE_HOOK_PCM
#include <thread>
#endif
#include "audio_errors.h"
#include "audio_utils.h"
#include "audio_volume.h"
#include "i_hpae_manager.h"
#include "audio_engine_log.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
static constexpr uint32_t FIRST_SESSIONID = 90000;
static constexpr uint32_t MAX_VALID_SESSIONID = 99999;
static constexpr uint32_t OPERATION_TIMEOUT_IN_MS = 1000; // 1000ms
static constexpr uint32_t MS_PER_SECOND = 1000;
static constexpr uint32_t DEFAULT_RING_BUFFER_NUM = 4;
static constexpr int32_t MAX_OVERFLOW_UNDERRUN_COUNT = 50; // 1s
uint32_t HpaeSoftLink::g_sessionId = FIRST_SESSIONID; // begin at 90000
std::shared_ptr<IHpaeSoftLink> IHpaeSoftLink::CreateSoftLink(uint32_t sinkIdx, uint32_t sourceIdx, SoftLinkMode mode)
{
    std::shared_ptr<HpaeSoftLink> softLink = std::make_shared<HpaeSoftLink>(sinkIdx, sourceIdx, mode);
    CHECK_AND_RETURN_RET_LOG(softLink != nullptr, nullptr, "new HpaeSoftLink failed");
    CHECK_AND_RETURN_RET_LOG(softLink->Init() == SUCCESS, nullptr, "HpaeSoftLink init failed");
    return softLink;
}

uint32_t HpaeSoftLink::GenerateSessionId()
{
    std::lock_guard<std::mutex> lock(sessionIdMutex_);
    uint32_t sessionId = g_sessionId++;
    AUDIO_INFO_LOG("hpae softlink sessionId: %{public}u", sessionId);
    if (g_sessionId > MAX_VALID_SESSIONID) {
        AUDIO_WARNING_LOG("sessionId is too large, reset it!");
        g_sessionId = FIRST_SESSIONID;
    }
    return sessionId;
}

HpaeSoftLink::HpaeSoftLink(uint32_t sinkIdx, uint32_t sourceIdx, SoftLinkMode mode)
    : sinkIdx_(sinkIdx), sourceIdx_(sourceIdx), linkMode_(mode), state_(HpaeSoftLinkState::NEW)
{
    sinkInfo_.sinkId = sinkIdx;
    sourceInfo_.sourceId = sourceIdx;
}

HpaeSoftLink::~HpaeSoftLink()
{
    AUDIO_INFO_LOG("~HpaeSoftLink");
    Release();
}

int32_t HpaeSoftLink::Init()
{
    Trace trace("HpaeSoftLink::Init");
    AUDIO_INFO_LOG("init in");
    CHECK_AND_RETURN_RET_LOG(state_.load() != HpaeSoftLinkState::PREPARED, SUCCESS, "softlink already inited");
    CHECK_AND_RETURN_RET_LOG(state_.load() == HpaeSoftLinkState::NEW, ERR_ILLEGAL_STATE, "init error state");
    CHECK_AND_RETURN_RET_LOG(sinkIdx_ != HDI_INVALID_ID && sourceIdx_ != HDI_INVALID_ID, ERR_INVALID_PARAM,
        "invalid sinkIdx or capturerIdx");

    int32_t ret = GetDeviceInfo();
    CHECK_AND_RETURN_RET(ret == SUCCESS, ERR_OPERATION_FAILED);

    size_t frameBytes = sinkInfo_.channels * static_cast<size_t>(GetSizeFromFormat(sinkInfo_.format)) *
        FRAME_LEN_20MS * sinkInfo_.samplingRate / MS_PER_SECOND;
    size_t size = DEFAULT_RING_BUFFER_NUM * frameBytes;
    bufferQueue_ = AudioRingCache::Create(size);
    CHECK_AND_RETURN_RET_LOG(bufferQueue_ != nullptr, ERR_OPERATION_FAILED, "bufferQueue create error");
    tempBuffer_.resize(frameBytes);
    ret = CreateStream();
    if (ret == SUCCESS) {
        state_ = HpaeSoftLinkState::PREPARED;
    }
    return ret;
}

int32_t HpaeSoftLink::GetDeviceInfo()
{
    Trace trace("HpaeSoftLink::GetDeviceInfo");
    AUDIO_INFO_LOG("get device info");
    std::unique_lock<std::mutex> lock(callbackMutex_);
    isDeviceOperationFinish_ = 0;
    int32_t retGetSink = ERROR;
    IHpaeManager::GetHpaeManager().GetSinkInfoByIdx(sinkIdx_,
        [this, &retGetSink](const HpaeSinkInfo &sinkInfo, int32_t result) {
            sinkInfo_ = sinkInfo;
            retGetSink = result;
            OnDeviceInfoReceived(SOFTLINK_SINK_OPERATION);
    });
    int32_t retGetSource = ERROR;
    IHpaeManager::GetHpaeManager().GetSourceInfoByIdx(sourceIdx_,
        [this, &retGetSource](const HpaeSourceInfo &sourceInfo, int32_t result) {
            sourceInfo_ = sourceInfo;
            retGetSource = result;
            OnDeviceInfoReceived(SOFTLINK_SOURCE_OPERATION);
    });
    bool stopWaiting = callbackCV_.wait_for(lock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return (isDeviceOperationFinish_ & SOFTLINK_SINK_OPERATION) &&
            (isDeviceOperationFinish_ & SOFTLINK_SOURCE_OPERATION);
    });
    CHECK_AND_RETURN_RET_LOG(stopWaiting, ERROR, "get device info timeout");
    CHECK_AND_RETURN_RET_LOG(retGetSink == SUCCESS, ERROR, "get sink info failed");
    CHECK_AND_RETURN_RET_LOG(retGetSource == SUCCESS, ERROR, "get source info failed");
    return SUCCESS;
}

void HpaeSoftLink::OnDeviceInfoReceived(const HpaeSoftLinkDeviceOperation &operation)
{
    std::unique_lock<std::mutex> lock(callbackMutex_);
    isDeviceOperationFinish_ |= operation;
    AUDIO_INFO_LOG("GetDeviceInfo callback, operation: %{public}d", operation);
    callbackCV_.notify_all();
}

void HpaeSoftLink::TransSinkInfoToStreamInfo(HpaeStreamInfo &info, const HpaeStreamClassType &streamClassType)
{
    info.channels = sinkInfo_.channels;
    info.samplingRate = sinkInfo_.samplingRate;
    info.format = sinkInfo_.format;
    info.channelLayout = sinkInfo_.channelLayout;
    info.frameLen = FRAME_LEN_20MS * static_cast<uint32_t>(sinkInfo_.samplingRate) / MS_PER_SECOND;
    info.streamClassType = streamClassType;
    info.isMoveAble = false;
    info.sessionId = GenerateSessionId();
    if (streamClassType == HPAE_STREAM_CLASS_TYPE_PLAY) {
        info.streamType = STREAM_VOICE_CALL;
        info.deviceName = sinkInfo_.deviceName;
        info.sourceType = SOURCE_TYPE_INVALID;
        info.fadeType = DEFAULT_FADE;
        info.effectInfo.effectMode = EFFECT_DEFAULT;
        info.effectInfo.effectScene = SCENE_VOIP_DOWN;
        info.effectInfo.systemVolumeType = STREAM_VOICE_CALL;
        info.effectInfo.streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;
    } else {
        info.streamType = STREAM_SOURCE_VOICE_CALL;
        info.deviceName = sourceInfo_.deviceName;
        if (linkMode_ == SoftLinkMode::OFFLOADINNERCAP_AID) {
            info.sourceType = SOURCE_TYPE_OFFLOAD_CAPTURE;
        } else {
            info.sourceType = SOURCE_TYPE_MIC;
        }
    }
}

int32_t HpaeSoftLink::CreateStream()
{
    TransSinkInfoToStreamInfo(rendererStreamInfo_, HPAE_STREAM_CLASS_TYPE_PLAY);
    TransSinkInfoToStreamInfo(capturerStreamInfo_, HPAE_STREAM_CLASS_TYPE_RECORD);

    uint32_t &rendererSessionId = rendererStreamInfo_.sessionId;
    IHpaeManager::GetHpaeManager().CreateStream(rendererStreamInfo_);
    IHpaeManager::GetHpaeManager().RegisterStatusCallback(HPAE_STREAM_CLASS_TYPE_PLAY, rendererSessionId,
        shared_from_this());
    IHpaeManager::GetHpaeManager().RegisterWriteCallback(rendererSessionId, shared_from_this());
    StreamVolumeParams streamVolumeParams = {
        rendererSessionId, rendererStreamInfo_.streamType,
        rendererStreamInfo_.effectInfo.streamUsage, -1, -1,
        false, AUDIOSTREAM_VOLUMEMODE_SYSTEM_GLOBAL, false
    };
    AudioVolume::GetInstance()->AddStreamVolume(streamVolumeParams);
    streamStateMap_[rendererStreamInfo_.sessionId] = HpaeSoftLinkState::PREPARED;

    uint32_t &capturerSessionId = capturerStreamInfo_.sessionId;
    IHpaeManager::GetHpaeManager().CreateStream(capturerStreamInfo_);
    IHpaeManager::GetHpaeManager().RegisterStatusCallback(HPAE_STREAM_CLASS_TYPE_RECORD, capturerSessionId,
        shared_from_this());
    IHpaeManager::GetHpaeManager().RegisterReadCallback(capturerSessionId, shared_from_this());
    streamStateMap_[capturerStreamInfo_.sessionId] = HpaeSoftLinkState::PREPARED;
    return SUCCESS;
}

int32_t HpaeSoftLink::SetVolume(float volume)
{
    std::lock_guard<std::mutex> stateLock(stateMutex_);
    CHECK_AND_RETURN_RET_LOG(state_.load() != HpaeSoftLinkState::NEW, ERR_ILLEGAL_STATE, "softlink not prepared");
    CHECK_AND_RETURN_RET_LOG(state_.load() != HpaeSoftLinkState::RELEASED, ERR_ILLEGAL_STATE,
        "softlink already release");
    AudioVolume::GetInstance()->SetStreamVolume(rendererStreamInfo_.sessionId, volume);
    return SUCCESS;
}

int32_t HpaeSoftLink::SetVolumeDuckFactor(float duckFactor)
{
    std::lock_guard<std::mutex> stateLock(stateMutex_);
    CHECK_AND_RETURN_RET_LOG(state_ != HpaeSoftLinkState::NEW, ERR_ILLEGAL_STATE, "softlink not prepared");
    CHECK_AND_RETURN_RET_LOG(state_ != HpaeSoftLinkState::RELEASED, ERR_ILLEGAL_STATE,
        "softlink already release");
    AudioVolume::GetInstance()->SetStreamVolumeDuckFactor(rendererStreamInfo_.sessionId, duckFactor);
    return SUCCESS;
}

int32_t HpaeSoftLink::SetVolumeMute(bool isMute)
{
    std::lock_guard<std::mutex> stateLock(stateMutex_);
    CHECK_AND_RETURN_RET_LOG(state_ != HpaeSoftLinkState::NEW, ERR_ILLEGAL_STATE, "softlink not prepared");
    CHECK_AND_RETURN_RET_LOG(state_ != HpaeSoftLinkState::RELEASED, ERR_ILLEGAL_STATE,
        "softlink already release");
    AudioVolume::GetInstance()->SetStreamVolumeMute(rendererStreamInfo_.sessionId, isMute);
    return SUCCESS;
}

int32_t HpaeSoftLink::SetVolumeLowPowerFactor(float lowPowerFactor)
{
    std::lock_guard<std::mutex> stateLock(stateMutex_);
    CHECK_AND_RETURN_RET_LOG(state_ != HpaeSoftLinkState::NEW, ERR_ILLEGAL_STATE, "softlink not prepared");
    CHECK_AND_RETURN_RET_LOG(state_ != HpaeSoftLinkState::RELEASED, ERR_ILLEGAL_STATE,
        "softlink already release");
    AudioVolume::GetInstance()->SetStreamVolumeLowPowerFactor(rendererStreamInfo_.sessionId, lowPowerFactor);
    return SUCCESS;
}

int32_t HpaeSoftLink::Start()
{
    Trace trace("HpaeSoftLink::Start");
    AUDIO_INFO_LOG("Start in");
    std::lock_guard<std::mutex> stateLock(stateMutex_);
    CHECK_AND_RETURN_RET_LOG(state_.load() != HpaeSoftLinkState::RUNNING, SUCCESS, "softlink already start");
    CHECK_AND_RETURN_RET_LOG(
        state_.load() == HpaeSoftLinkState::PREPARED || state_.load() == HpaeSoftLinkState::STOPPED,
        ERR_ILLEGAL_STATE, "softlink not init");
    std::unique_lock<std::mutex> lock(callbackMutex_);
    isStreamOperationFinish_ = 0;
    IHpaeManager::GetHpaeManager().Start(HPAE_STREAM_CLASS_TYPE_PLAY, rendererStreamInfo_.sessionId);
    IHpaeManager::GetHpaeManager().Start(HPAE_STREAM_CLASS_TYPE_RECORD, capturerStreamInfo_.sessionId);
    bool stopWaiting = callbackCV_.wait_for(lock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return (isStreamOperationFinish_ & SOFTLINK_RENDERER_OPERATION) &&
            (isStreamOperationFinish_ & SOFTLINK_CAPTURER_OPERATION);
    });

    CHECK_AND_RETURN_RET_LOG(stopWaiting, ERROR, "start timeout");

    if (streamStateMap_[rendererStreamInfo_.sessionId] != HpaeSoftLinkState::RUNNING) {
        AUDIO_ERR_LOG("start renderer[%{public}d] failed", rendererStreamInfo_.sessionId);
        CHECK_AND_RETURN_RET(streamStateMap_[capturerStreamInfo_.sessionId] == HpaeSoftLinkState::RUNNING, ERROR,
            "start capturer[%{public}d] failed", capturerStreamInfo_.sessionId);
        IHpaeManager::GetHpaeManager().Stop(HPAE_STREAM_CLASS_TYPE_RECORD, capturerStreamInfo_.sessionId);
        return ERROR;
    }

    if (streamStateMap_[capturerStreamInfo_.sessionId] != HpaeSoftLinkState::RUNNING) {
        AUDIO_ERR_LOG("start capturer[%{public}d] failed", capturerStreamInfo_.sessionId);
        IHpaeManager::GetHpaeManager().Stop(HPAE_STREAM_CLASS_TYPE_PLAY, rendererStreamInfo_.sessionId);
        return ERROR;
    }
    state_ = HpaeSoftLinkState::RUNNING;
    return SUCCESS;
}

int32_t HpaeSoftLink::Stop()
{
    Trace trace("HpaeSoftLink::Stop");
    AUDIO_INFO_LOG("Stop in");
    std::lock_guard<std::mutex> stateLock(stateMutex_);
    CHECK_AND_RETURN_RET_LOG(state_.load() != HpaeSoftLinkState::STOPPED, SUCCESS, "softlink already stop");
    CHECK_AND_RETURN_RET_LOG(state_.load() == HpaeSoftLinkState::RUNNING, ERR_ILLEGAL_STATE, "softlink not init");
    StopInner();
    return SUCCESS;
}

void HpaeSoftLink::StopInner()
{
    IHpaeManager::GetHpaeManager().Stop(HPAE_STREAM_CLASS_TYPE_PLAY, rendererStreamInfo_.sessionId);
    IHpaeManager::GetHpaeManager().Stop(HPAE_STREAM_CLASS_TYPE_RECORD, capturerStreamInfo_.sessionId);
    state_ = HpaeSoftLinkState::STOPPED;
}

int32_t HpaeSoftLink::Release()
{
    Trace trace("HpaeSoftLink::Release");
    AUDIO_INFO_LOG("Release in");
    std::lock_guard<std::mutex> stateLock(stateMutex_);
    CHECK_AND_RETURN_RET_LOG(state_.load() != HpaeSoftLinkState::RELEASED, SUCCESS, "softlink already release");
    if (state_.load() == HpaeSoftLinkState::RUNNING) {
        AUDIO_INFO_LOG("softlink not stop, stop before release");
        StopInner();
    }
    IHpaeManager::GetHpaeManager().Release(HPAE_STREAM_CLASS_TYPE_PLAY, rendererStreamInfo_.sessionId);
    AudioVolume::GetInstance()->RemoveStreamVolume(rendererStreamInfo_.sessionId);
    IHpaeManager::GetHpaeManager().Release(HPAE_STREAM_CLASS_TYPE_RECORD, capturerStreamInfo_.sessionId);
    state_ = HpaeSoftLinkState::RELEASED;
    return SUCCESS;
}

void HpaeSoftLink::FlushRingCache()
{
    CHECK_AND_RETURN_LOG(bufferQueue_ != nullptr, "ring cache is null");
    bufferQueue_->ResetBuffer();
}

void HpaeSoftLink::OnStatusUpdate(IOperation operation, uint32_t streamIndex)
{
    AUDIO_INFO_LOG("stream %{public}u recv operation:%{public}d", streamIndex, operation);
    CHECK_AND_RETURN_LOG(operation != OPERATION_RELEASED, "stream already released");
    CHECK_AND_RETURN_LOG(streamIndex == rendererStreamInfo_.sessionId || streamIndex == capturerStreamInfo_.sessionId,
        "invalid streamIndex");
    if (operation == OPERATION_STARTED) {
        streamStateMap_[streamIndex] = HpaeSoftLinkState::RUNNING;
    } else if (operation == OPERATION_STOPPED) {
        streamStateMap_[streamIndex] = HpaeSoftLinkState::STOPPED;
        if (streamIndex == capturerStreamInfo_.sessionId) {
            FlushRingCache();
        }
    } else if (operation == OPERATION_RELEASED) {
        streamStateMap_[streamIndex] = HpaeSoftLinkState::RELEASED;
    } else {
        return;
    }
    std::lock_guard<std::mutex> lock(callbackMutex_);
    isStreamOperationFinish_ |=
        (streamIndex == rendererStreamInfo_.sessionId ? SOFTLINK_RENDERER_OPERATION : SOFTLINK_CAPTURER_OPERATION);
    callbackCV_.notify_all();
}

int32_t HpaeSoftLink::OnStreamData(AudioCallBackStreamInfo& callbackStreamInfo)
{
    Trace trace("HpaeSoftLink::OnStreamData, [" +std::to_string(rendererStreamInfo_.sessionId) + "]OnWriteData");
#ifdef ENABLE_HOOK_PCM
    if (sinkInfo_.adapterName == "file_io") {
        std::this_thread::sleep_for(std::chrono::milliseconds(20)); // 20s for file_io sleep
    }
#endif
    int8_t *inputData = callbackStreamInfo.inputData;
    size_t requestDataLen = callbackStreamInfo.requestDataLen;
    OptResult result = bufferQueue_->GetReadableSize();
    CHECK_AND_RETURN_RET_LOG(result.ret == OPERATION_SUCCESS, ERROR,
        "ringBuffer get readable invalid size: %{public}zu", result.size);
    if (result.size == 0 || result.size < requestDataLen) {
        ++underRunCount_;
        AUDIO_DEBUG_LOG("underrun[%{public}d]!, readable size is invalid, result.size[%{public}zu],"
            "requestDataLen[%{public}zu]", underRunCount_, result.size, requestDataLen);
    } else {
        underRunCount_ = 0;
    }
    if (underRunCount_ >= MAX_OVERFLOW_UNDERRUN_COUNT) {
        AUDIO_WARNING_LOG("renderer[%{public}u] has underrun 1s", rendererStreamInfo_.sessionId);
        underRunCount_ = 0;
        return ERROR;
    }
    AUDIO_DEBUG_LOG("readable size: %{public}zu, requestDataLen: %{public}zu", result.size, requestDataLen);
    result = bufferQueue_->Dequeue({reinterpret_cast<uint8_t *>(inputData), requestDataLen});
    CHECK_AND_RETURN_RET_LOG(result.ret == OPERATION_SUCCESS, ERROR, "ringBuffer dequeue failed");
    return SUCCESS;
}

static void CopyRightToLeft(uint8_t *data, size_t size, const AudioSampleFormat &format)
{
    CHECK_AND_RETURN_LOG(data != nullptr && size > 0, "error param");
    const size_t bytesPerSample = static_cast<size_t>(GetSizeFromFormat(format));
    const size_t frameSize = bytesPerSample * 2;
    CHECK_AND_RETURN_LOG(size > frameSize, "invalid data size");
    uint8_t *left = nullptr;
    uint8_t *right = nullptr;
    for (size_t i = 0; i < size - frameSize + 1; i += frameSize) {
        left = data + i;
        right = left + bytesPerSample;
        CHECK_AND_RETURN_LOG(memcpy_s(left, bytesPerSample, right, bytesPerSample) == 0, "memcpy_s failed");
    }
}

int32_t HpaeSoftLink::OnStreamData(AudioCallBackCapturerStreamInfo& callbackStreamInfo)
{
    Trace trace("HpaeSoftLink::OnStreamData, [" + std::to_string(capturerStreamInfo_.sessionId) + "]OnReadData");
#ifdef ENABLE_HOOK_PCM
    if (sourceInfo_.adapterName == "file_io") {
        std::this_thread::sleep_for(std::chrono::milliseconds(20)); // 20s for file_io sleep
    }
#endif
    int8_t *outputData = callbackStreamInfo.outputData;
    size_t requestDataLen = callbackStreamInfo.requestDataLen;
    // todo : channel select
    if (linkMode_ == SoftLinkMode::HEARING_AID && sinkInfo_.channels == STEREO) {
        CopyRightToLeft(reinterpret_cast<uint8_t *>(outputData), requestDataLen, sinkInfo_.format);
    }
    OptResult result = bufferQueue_->GetWritableSize();
    CHECK_AND_RETURN_RET_LOG(result.ret == OPERATION_SUCCESS, ERR_READ_FAILED,
        "ringBuffer get writeable invalid size: %{public}zu", result.size);
    if (result.size == 0 || result.size < requestDataLen) {
        ++overFlowCount_;
        AUDIO_DEBUG_LOG("overflow[%{public}d]!, writable size is invalid, result.size[%{public}zu],"
            "requestDataLen[%{public}zu]", overFlowCount_, result.size, requestDataLen);
        bufferQueue_->Dequeue({reinterpret_cast<uint8_t *>(tempBuffer_.data()), requestDataLen});
    } else {
        overFlowCount_ = 0;
    }
    if (overFlowCount_ >= MAX_OVERFLOW_UNDERRUN_COUNT) {
        AUDIO_WARNING_LOG("capturer[%{public}u] has overflow 1s", capturerStreamInfo_.sessionId);
        overFlowCount_ = 0;
        return ERROR;
    }
    AUDIO_DEBUG_LOG("writable size: %{public}zu, requestDataLen: %{public}zu", result.size, requestDataLen);
    result = bufferQueue_->Enqueue({reinterpret_cast<uint8_t *>(outputData), requestDataLen});
    CHECK_AND_RETURN_RET_LOG(result.ret == OPERATION_SUCCESS, ERROR, "ringBuffer enqueue failed");
    return SUCCESS;
}

// for test
HpaeSoftLinkState HpaeSoftLink::GetStreamStateById(uint32_t sessionId)
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    CHECK_AND_RETURN_RET_LOG(streamStateMap_.find(sessionId) != streamStateMap_.end(), HpaeSoftLinkState::INVALID,
        "invalid param");
    return streamStateMap_[sessionId];
}

int32_t HpaeSoftLink::SetLoudnessGain(float loudnessGain)
{
    std::lock_guard<std::mutex> stateLock(stateMutex_);
    int32_t ret = IHpaeManager::GetHpaeManager().SetLoudnessGain(rendererStreamInfo_.sessionId, loudnessGain);
    CHECK_AND_RETURN_RET_LOG(ret == OPERATION_SUCCESS, ERROR, "SetLoudnessGain is error");
    return SUCCESS;
}
} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS
