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
#define LOG_TAG "audioStaticBufferProvider"
#endif

#include "audio_static_buffer_provider.h"

#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
namespace {
    static const size_t MAX_STATIC_BUFFER_SIZE = 1 * 1024 * 1024; // 1M
}

std::shared_ptr<AudioStaticBufferProvider> AudioStaticBufferProvider::CreateInstance(
    std::shared_ptr<OHAudioBufferBase> sharedBuffer)
{
    CHECK_AND_RETURN_RET_LOG(sharedBuffer != nullptr, nullptr, "sharedBuffer is nullptr");
    return std::make_shared<AudioStaticBufferProvider>(sharedBuffer);
}

AudioStaticBufferProvider::AudioStaticBufferProvider(std::shared_ptr<OHAudioBufferBase> sharedBuffer)
    : sharedBuffer_(sharedBuffer) {}

int32_t AudioStaticBufferProvider::GetDataFromStaticBuffer(int8_t *inputData, size_t requestDataLen)
{
    CHECK_AND_RETURN_RET_LOG(sharedBuffer_ != nullptr, ERR_NULL_POINTER, "Not in static mode");
    if (currentLoopTimes_ == totalLoopTimes_ ||
        sharedBuffer_->CheckFrozenAndSetLastProcessTime(BUFFER_IN_SERVER) ||
        sharedBuffer_->GetStreamStatus()->load() != STREAM_RUNNING) {
        memset_s(inputData, requestDataLen, 0, requestDataLen);
        AUDIO_WARNING_LOG("GetDataFromStaticBuffer fail, isReachLoopTimes %{public}d, isStatusRunning %{public}d",
            currentLoopTimes_ == totalLoopTimes_, sharedBuffer_->GetStreamStatus()->load() != STREAM_RUNNING);
        return ERR_OPERATION_FAILED;
    }

    size_t offset = 0;
    size_t remainSize = requestDataLen;
    while (remainSize > 0) {
        Trace loopTrace("CopyDataFromSharedBuffer " + std::to_string(remainSize));
        size_t copySize =
            std::min({remainSize, processedBufferSize_, processedBufferSize_ - curStaticDataPos_});
        CHECK_AND_RETURN_RET_LOG(curStaticDataPos_ + copySize <= processedBufferSize_, ERROR_INVALID_PARAM,
            "copySize exeeds totalSizeInByte");
        int32_t ret = memcpy_s(inputData + offset, copySize, processedBuffer_ + curStaticDataPos_, copySize);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR_INVALID_PARAM, "memcpy to inputData failed!");
        curStaticDataPos_ += copySize;
        remainSize -= copySize;
        offset += copySize;

        if (curStaticDataPos_ == processedBufferSize_) {
            // buffer copy finished once
            IncreaseCurrentLoopTimes();
            sharedBuffer_->IncreaseBufferEndCallbackSendTimes();
            curStaticDataPos_ = 0;
            if (currentLoopTimes_ == totalLoopTimes_) {
                sharedBuffer_->SetIsNeedSendLoopEndCallback(true);
                memset_s(inputData + offset, remainSize, 0, remainSize);
                offset += remainSize;
                remainSize = 0;
            }
            sharedBuffer_->WakeFutex();
        }
    }

    return CheckIsValid(inputData, offset, requestDataLen, remainSize);
}

int32_t AudioStaticBufferProvider::CheckIsValid(int8_t *inputData,
    size_t offset, size_t requestDataLen, size_t remainSize)
{
    if (offset != requestDataLen || remainSize != 0) {
        AUDIO_ERR_LOG("GetDataFromStaticBuffer failed");
        memset_s(inputData, requestDataLen, 0, requestDataLen);
        return ERR_OPERATION_FAILED;
    }
    return SUCCESS;
}

void AudioStaticBufferProvider::SetStaticBufferInfo(const StaticBufferInfo &staticBufferInfo)
{
    CHECK_AND_RETURN_RET_LOG(curStaticDataPos_ <= MAX_STATIC_BUFFER_SIZE, "SetStaticBufferInfo invalid param");
    preSetTotalLoopTimes_ = staticBufferInfo.preSetTotalLoopTimes_;
    totalLoopTimes_ = staticBufferInfo.totalLoopTimes_;
    currentLoopTimes_ = staticBufferInfo.currentLoopTimes_;
    curStaticDataPos_ = staticBufferInfo.curStaticDataPos_;
}

int32_t AudioStaticBufferProvider::GetStaticBufferInfo(StaticBufferInfo &staticBufferInfo)
{
    CHECK_AND_RETURN_RET_LOG(sharedBuffer_ != nullptr, ERR_NULL_POINTER, "Not in static mode");
    staticBufferInfo.totalLoopTimes_ = totalLoopTimes_;
    staticBufferInfo.currentLoopTimes_ = currentLoopTimes_;
    staticBufferInfo.curStaticDataPos_ = curStaticDataPos_;
    staticBufferInfo.preSetTotalLoopTimes_ = preSetTotalLoopTimes_;
    staticBufferInfo.sharedMemory_ = sharedBuffer_->GetSharedMem();
    return SUCCESS;
}

void AudioStaticBufferProvider::SetProcessedBuffer(uint8_t **bufferBase, size_t bufferSize)
{
    processedBuffer_ = *bufferBase;
    processedBufferSize_ = bufferSize;
}

void AudioStaticBufferProvider::PreSetLoopTimes(int64_t times)
{
    needRefreshLoopTimes_ = true;
    preSetTotalLoopTimes_ = times;
}

void AudioStaticBufferProvider::RefreshLoopTimes()
{
    totalLoopTimes_ = preSetTotalLoopTimes_;
    needRefreshLoopTimes_ = false;
    AUDIO_INFO_LOG("RefreshLoopTimes, curTotalLoopTimes %{public}ld", totalLoopTimes_);
}

bool AudioStaticBufferProvider::NeedRefreshLoopTimes()
{
    return needRefreshLoopTimes_;
}

int32_t AudioStaticBufferProvider::ResetLoopStatus()
{
    CHECK_AND_RETURN_RET_LOG(sharedBuffer_ != nullptr, ERR_NULL_POINTER, "Not in static mode");
    currentLoopTimes_ = 0;
    curStaticDataPos_ = 0;
    sharedBuffer_->ResetBufferEndCallbackSendTimes();
    sharedBuffer_->SetIsNeedSendLoopEndCallback(false);
    return SUCCESS;
}

int32_t AudioStaticBufferProvider::IncreaseCurrentLoopTimes()
{
    if (currentLoopTimes_ > LLONG_MAX - 1) {
        AUDIO_ERR_LOG("bufferEndCallbackSendTimes increase will overflow");
        return ERROR_INVALID_PARAM;
    }
    currentLoopTimes_ += 1;
    if (totalLoopTimes_ == -1) {
        return SUCCESS;
    }
    if (currentLoopTimes_ > totalLoopTimes_) {
        AUDIO_ERR_LOG("CurrentLoopTimes Reach %{public}ld", totalLoopTimes_);
        return ERROR;
    }
    return SUCCESS;
}


} // namespace AudioStandard
} // namespace OHOS