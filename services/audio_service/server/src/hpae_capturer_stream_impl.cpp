/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "audio_capturer_log.h"
#include "audio_utils.h"
#include "policy_handler.h"
#include <iostream>
#include <cinttypes>

namespace OHOS {
namespace AudioStandard {
static SafeMap<void *, std::weak_ptr<HpaeCapturerStreamImpl>> paCapturerMap_;
const int32_t FRAME_LEN_ON_MS = 20;
const int32_t MSEC_PER_SEC = 1000;

static inline int32_t GetSizeFromFormat(int32_t format)
{
    return format != SAMPLE_F32LE ? ((format) + 1) : (4);
}

HpaeCapturerStreamImpl::HpaeCapturerStreamImpl(AudioProcessConfig processConfig)
{
    processConfig_ = processConfig;
    spanSizeInFrame_ = static_cast<size_t>(FRAME_LEN_ON_MS *
        (static_cast<float>(streamInfo.samplingRate) / MSEC_PER_SEC));
    byteSizePerFrame_ = (processConfig.streamInfo.channels * GetSizeFromFormat(processConfig.streamInfo.format));
    minBufferSize_ = MIN_BUFFER_SIZE * byteSizePerFrame_ * spanSizeInFrame_;
}

HpaeCapturerStreamImpl::~HpaeCapturerStreamImpl()
{
    AUDIO_DEBUG_LOG("~HpaeCapturerStreamImpl");
    if (capturerServerDumpFile_) {
        fclose(capturerServerDumpFile_);
        capturerServerDumpFile_ = nullptr;
    }
    paCapturerMap_.Erase(this);
}

int32_t HpaeCapturerStreamImpl::InitParams(const std::string &deviceName)
{
    paCapturerMap_.Insert(this, weak_from_this());

    return SUCCESS;
}

int32_t HpaeCapturerStreamImpl::Start()
{
    AUDIO_INFO_LOG("Start");
    state_ = RUNNING;
    return SUCCESS;
}

int32_t HpaeCapturerStreamImpl::Pause(bool isStandby)
{
    AUDIO_INFO_LOG("Pause");
    return SUCCESS;
}

int32_t HpaeCapturerStreamImpl::GetStreamFramesRead(uint64_t &framesRead)
{
    // to do callback data report
    framesRead = 0;
    return SUCCESS;
}

int32_t HpaeCapturerStreamImpl::GetCurrentTimeStamp(uint64_t &timestamp)
{
    timestamp = 0;
    return SUCCESS;
}

int32_t HpaeCapturerStreamImpl::GetLatency(uint64_t &latency)
{
    latency = 0;
    AUDIO_DEBUG_LOG("total latency:  %{public}" PRIu64 "ms", latency);
    return SUCCESS;
}

int32_t HpaeCapturerStreamImpl::Flush()
{
    AUDIO_INFO_LOG("Flush");
    return SUCCESS;
}

int32_t HpaeCapturerStreamImpl::Stop()
{
    AUDIO_INFO_LOG("Stop");
    state_ = STOPPING;
    return SUCCESS;
}

int32_t HpaeCapturerStreamImpl::Release()
{
    if (state_ == RUNNING) {
        AUDIO_ERR_LOG("%{public}u Release state_ is RUNNING", processConfig_.originalSessionId);
    }
    AUDIO_INFO_LOG("Release Enter");
    state_ = RELEASED;
    // to do check closeaudioport
    if (processConfig_.capturerInfo.sourceType == SOURCE_TYPE_WAKEUP) {
        PolicyHandler::GetInstance().NotifyWakeUpCapturerRemoved();
    }
    return SUCCESS;
}

// to do callback data report
void HpaeCapturerStreamImpl::RegisterStatusCallback(const std::weak_ptr<IStatusCallback> &callback)
{
    statusCallback_ = callback;
}

void HpaeCapturerStreamImpl::RegisterReadCallback(const std::weak_ptr<IReadCallback> &callback)
{
    AUDIO_INFO_LOG("RegisterReadCallback start");
    readCallback_ = callback;
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
