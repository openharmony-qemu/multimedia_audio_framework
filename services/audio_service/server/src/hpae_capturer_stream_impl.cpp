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
#define LOG_TAG "HpaeCapturerStreamImpl"
#endif

#include "safe_map.h"
#include "hpae_capturer_stream_impl.h"
#include "audio_errors.h"
#include "audio_utils.h"
#include "policy_handler.h"
#include <iostream>
#include <cinttypes>
#include "i_hpae_manager.h"
#include "audio_engine_log.h"
using namespace OHOS::AudioStandard::HPAE;
namespace OHOS {
namespace AudioStandard {
static SafeMap<void *, std::weak_ptr<HpaeCapturerStreamImpl>> paCapturerMap_;
const int32_t MIN_BUFFER_SIZE = 2;
const int32_t FRAME_LEN_10MS = 2;
const int32_t TENMS_PER_SEC = 100;

HpaeCapturerStreamImpl::HpaeCapturerStreamImpl(AudioProcessConfig processConfig)
{
    processConfig_ = processConfig;
    spanSizeInFrame_ = FRAME_LEN_10MS * processConfig.streamInfo.samplingRate / TENMS_PER_SEC;
    byteSizePerFrame_ = (processConfig.streamInfo.channels *
        static_cast<size_t>(GetSizeFromFormat(processConfig.streamInfo.format)));
    minBufferSize_ = MIN_BUFFER_SIZE * byteSizePerFrame_ * spanSizeInFrame_;
}

HpaeCapturerStreamImpl::~HpaeCapturerStreamImpl()
{
    AUDIO_INFO_LOG("~HpaeCapturerStreamImpl [%{public}u]", streamIndex_);
    if (capturerServerDumpFile_) {
        fclose(capturerServerDumpFile_);
        capturerServerDumpFile_ = nullptr;
    }
    paCapturerMap_.Erase(this);
}

int32_t HpaeCapturerStreamImpl::InitParams(const std::string &deviceName)
{
    paCapturerMap_.Insert(this, weak_from_this());

    HpaeStreamInfo streamInfo;
    streamInfo.channels = processConfig_.streamInfo.channels;
    streamInfo.samplingRate = processConfig_.streamInfo.samplingRate;
    streamInfo.format = processConfig_.streamInfo.format;
    streamInfo.frameLen = spanSizeInFrame_;
    streamInfo.sessionId = processConfig_.originalSessionId;
    streamInfo.streamType = processConfig_.streamType;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_RECORD;
    streamInfo.sourceType = processConfig_.capturerInfo.sourceType;
    streamInfo.uid = processConfig_.appInfo.appUid;
    streamInfo.pid = processConfig_.appInfo.appPid;
    streamInfo.tokenId = processConfig_.appInfo.appTokenId;
    streamInfo.deviceName = deviceName;
    streamInfo.isMoveAble = true;
    streamInfo.privacyType = processConfig_.privacyType;
    auto &hpaeManager = IHpaeManager::GetHpaeManager();
    int32_t ret = hpaeManager.CreateStream(streamInfo);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR_INVALID_PARAM, "CreateStream is error");

    // Register Callback
    ret = hpaeManager.RegisterStatusCallback(HPAE_STREAM_CLASS_TYPE_RECORD, streamInfo.sessionId, shared_from_this());
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR_INVALID_PARAM, "RegisterStatusCallback is error!");
    ret = hpaeManager.RegisterReadCallback(streamInfo.sessionId, shared_from_this());
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR_INVALID_PARAM, "RegisterReadCallback is error!");
    return SUCCESS;
}

int32_t HpaeCapturerStreamImpl::Start()
{
    AUDIO_INFO_LOG("[%{public}u] Enter", streamIndex_);
    int32_t ret = IHpaeManager::GetHpaeManager().Start(HPAE_STREAM_CLASS_TYPE_RECORD, processConfig_.originalSessionId);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_INVALID_PARAM, "ErrorCode: %{public}d", ret);
    state_ = RUNNING;
    return SUCCESS;
}

int32_t HpaeCapturerStreamImpl::Pause(bool isStandby)
{
    AUDIO_INFO_LOG("[%{public}u] Enter", streamIndex_);
    int32_t ret = IHpaeManager::GetHpaeManager().Pause(HPAE_STREAM_CLASS_TYPE_RECORD, processConfig_.originalSessionId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_INVALID_PARAM, "ErrorCode: %{public}d", ret);
    return SUCCESS;
}

int32_t HpaeCapturerStreamImpl::GetStreamFramesRead(uint64_t &framesRead)
{
    framesRead = framesRead_;
    return SUCCESS;
}

int32_t HpaeCapturerStreamImpl::GetCurrentTimeStamp(uint64_t &timestamp)
{
    std::shared_lock<std::shared_mutex> lock(latencyMutex_);
    timestamp = timestamp_;
    return SUCCESS;
}

int32_t HpaeCapturerStreamImpl::GetLatency(uint64_t &latency)
{
    std::shared_lock<std::shared_mutex> lock(latencyMutex_);
    latency = latency_;
    AUDIO_DEBUG_LOG("total latency:  %{public}" PRIu64 "ms", latency);
    return SUCCESS;
}

int32_t HpaeCapturerStreamImpl::Flush()
{
    AUDIO_INFO_LOG("[%{public}u] Enter", streamIndex_);
    int32_t ret = IHpaeManager::GetHpaeManager().Flush(HPAE_STREAM_CLASS_TYPE_RECORD, processConfig_.originalSessionId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_INVALID_PARAM, "ErrorCode: %{public}d", ret);
    return SUCCESS;
}

int32_t HpaeCapturerStreamImpl::Stop()
{
    AUDIO_INFO_LOG("[%{public}u] Enter", streamIndex_);
    int32_t ret = IHpaeManager::GetHpaeManager().Stop(HPAE_STREAM_CLASS_TYPE_RECORD, processConfig_.originalSessionId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_INVALID_PARAM, "ErrorCode: %{public}d", ret);
    state_ = STOPPING;
    return SUCCESS;
}

int32_t HpaeCapturerStreamImpl::Release()
{
    if (state_ == RUNNING) {
        AUDIO_ERR_LOG("%{public}u state_ is RUNNING", processConfig_.originalSessionId);
        IHpaeManager::GetHpaeManager().Stop(HPAE_STREAM_CLASS_TYPE_RECORD, processConfig_.originalSessionId);
    }
    AUDIO_INFO_LOG("[%{public}u] Enter", streamIndex_);
    int32_t ret = IHpaeManager::GetHpaeManager().Release(HPAE_STREAM_CLASS_TYPE_RECORD,
        processConfig_.originalSessionId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_INVALID_PARAM, "ErrorCode: %{public}d", ret);
    state_ = RELEASED;
    // to do check closeaudioport
    if (processConfig_.capturerInfo.sourceType == SOURCE_TYPE_WAKEUP) {
        PolicyHandler::GetInstance().NotifyWakeUpCapturerRemoved();
    }
    return SUCCESS;
}

void HpaeCapturerStreamImpl::RegisterStatusCallback(const std::weak_ptr<IStatusCallback> &callback)
{
    AUDIO_DEBUG_LOG("RegisterStatusCallback in");
    statusCallback_ = callback;
}

void HpaeCapturerStreamImpl::RegisterReadCallback(const std::weak_ptr<IReadCallback> &callback)
{
    AUDIO_INFO_LOG("RegisterReadCallback start");
    readCallback_ = callback;
}

int32_t HpaeCapturerStreamImpl::OnStreamData(AudioCallBackCapturerStreamInfo &callBackStreamInfo)
{
    {
        std::unique_lock<std::shared_mutex> lock(latencyMutex_);
        timestamp_ = callBackStreamInfo.timestamp;
        latency_ = callBackStreamInfo.latency;
        framesRead_ = callBackStreamInfo.framesRead;
    }
    if (auto callback = readCallback_.lock()) {
        return callback->OnReadData(callBackStreamInfo.outputData, callBackStreamInfo.requestDataLen);
    }
    return SUCCESS;
}

void HpaeCapturerStreamImpl::OnStatusUpdate(IOperation operation, uint32_t streamIndex)
{
    auto statusCallback = statusCallback_.lock();
    if (statusCallback) {
        statusCallback->OnStatusUpdate(operation);
    }
}

BufferDesc HpaeCapturerStreamImpl::DequeueBuffer(size_t length)
{
    BufferDesc bufferDesc;
    return bufferDesc;
}

int32_t HpaeCapturerStreamImpl::EnqueueBuffer(const BufferDesc &bufferDesc)
{
    AUDIO_DEBUG_LOG("After capturere EnqueueBuffer");
    return SUCCESS;
}

int32_t HpaeCapturerStreamImpl::DropBuffer()
{
    AUDIO_DEBUG_LOG("After capturere DropBuffer");
    return SUCCESS;
}

int32_t HpaeCapturerStreamImpl::GetMinimumBufferSize(size_t &minBufferSize) const
{
    minBufferSize = minBufferSize_;
    return SUCCESS;
}

void HpaeCapturerStreamImpl::GetByteSizePerFrame(size_t &byteSizePerFrame) const
{
    byteSizePerFrame = byteSizePerFrame_;
}

void HpaeCapturerStreamImpl::GetSpanSizePerFrame(size_t &spanSizeInFrame) const
{
    spanSizeInFrame = spanSizeInFrame_;
}

void HpaeCapturerStreamImpl::SetStreamIndex(uint32_t index)
{
    AUDIO_INFO_LOG("Using index/sessionId %{public}u", index);
    streamIndex_ = index;
}

uint32_t HpaeCapturerStreamImpl::GetStreamIndex()
{
    return streamIndex_;
}

void HpaeCapturerStreamImpl::AbortCallback(int32_t abortTimes)
{
    abortFlag_ += abortTimes;
}
} // namespace AudioStandard
} // namespace OHOS
