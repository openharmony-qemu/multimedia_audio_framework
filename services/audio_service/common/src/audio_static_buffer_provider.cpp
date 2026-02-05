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
#include <cinttypes>
#include "audio_errors.h"
#include "audio_static_buffer_processor.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {
namespace {
    static const size_t MAX_STATIC_BUFFER_SIZE = 1 * 1024 * 1024; // 1M
}

std::shared_ptr<AudioStaticBufferProvider> AudioStaticBufferProvider::CreateInstance(
    AudioStreamInfo streamInfo, std::shared_ptr<OHAudioBufferBase> sharedBuffer)
{
    CHECK_AND_RETURN_RET_LOG(sharedBuffer != nullptr, nullptr, "sharedBuffer is nullptr");
    return std::make_shared<AudioStaticBufferProvider>(streamInfo, sharedBuffer);
}

AudioStaticBufferProvider::AudioStaticBufferProvider(AudioStreamInfo streamInfo,
    std::shared_ptr<OHAudioBufferBase> sharedBuffer) : sharedBuffer_(sharedBuffer), streamInfo_(streamInfo) {}

int32_t AudioStaticBufferProvider::GetDataFromStaticBuffer(int8_t *inputData, size_t requestDataLen)
{
    std::unique_lock<std::mutex> lock(eventMutex_);
    CHECK_AND_RETURN_RET_LOG(sharedBuffer_ != nullptr && processedBuffer_ != nullptr, ERR_INVALID_OPERATION,
        "sharedBuffer is nullptr or read data before processBuffer!");
    if (!NeedProvideData()) {
        memset_s(inputData, requestDataLen, 0, requestDataLen);
        return ERR_OPERATION_FAILED;
    }

    Trace traceNormal("GetDataFromStaticBuffer NormalData");
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
                needFadeOut_ = true;
                playFinished_ = true;
            }
            sharedBuffer_->WakeFutex();
        }
    }
    sharedBuffer_->SetStaticPlayPosition(currentLoopTimes_, curStaticDataPos_);
    if (sharedBuffer_->IsFirstFrame()) {
        sharedBuffer_->WakeFutex();
    }

    CHECK_AND_RETURN_RET_LOG(CheckIsValid(inputData, offset, requestDataLen, remainSize) == SUCCESS,
        ERR_OPERATION_FAILED, "GetStaticBuffer is not valid, reset buffer!");

    return ProcessFadeInOutIfNeed(inputData, requestDataLen);
}

int32_t AudioStaticBufferProvider::ProcessFadeInOutIfNeed(int8_t *inputData, size_t requestDataLen)
{
    CHECK_AND_RETURN_RET(needFadeIn_ || needFadeOut_, SUCCESS);
    Trace traceFade("CopyDataFromSharedBuffer " + std::string(needFadeOut_ ? "FadeOutData" : "FadeInData"));
    int32_t ret = AudioStaticBufferProcessor::ProcessFadeInOut(inputData, requestDataLen, streamInfo_, needFadeOut_);
    if (needFadeOut_) {
        needFadeOut_ = false;
    } else {
        needFadeIn_ = false;
    }

    // if RefreshBufferStatus before fadeout, the beginning data will be processed as fadeout
    if (delayRefreshBufferStatus_) {
        RefreshBufferStatus();
    }
    return ret;
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
    std::unique_lock<std::mutex> lock(eventMutex_);
    CHECK_AND_RETURN_LOG(curStaticDataPos_ <= MAX_STATIC_BUFFER_SIZE, "SetStaticBufferInfo invalid param");
    totalLoopTimes_ = staticBufferInfo.totalLoopTimes_;
    currentLoopTimes_ = staticBufferInfo.currentLoopTimes_;
    curStaticDataPos_ = staticBufferInfo.curStaticDataPos_;
}

int32_t AudioStaticBufferProvider::GetStaticBufferInfo(StaticBufferInfo &staticBufferInfo)
{
    std::unique_lock<std::mutex> lock(eventMutex_);
    CHECK_AND_RETURN_RET_LOG(sharedBuffer_ != nullptr, ERR_NULL_POINTER, "Not in static mode");
    staticBufferInfo.totalLoopTimes_ = totalLoopTimes_;
    staticBufferInfo.currentLoopTimes_ = currentLoopTimes_;
    staticBufferInfo.curStaticDataPos_ = curStaticDataPos_;
    staticBufferInfo.sharedMemory_ = sharedBuffer_->GetSharedMem();
    return SUCCESS;
}

void AudioStaticBufferProvider::SetProcessedBuffer(uint8_t **bufferBase, size_t bufferSize)
{
    std::unique_lock<std::mutex> lock(eventMutex_);
    CHECK_AND_RETURN_LOG(bufferBase != nullptr, "bufferBase in SetProcessedBuffer is nullptr!");
    CHECK_AND_RETURN_LOG(sharedBuffer_ != nullptr, "sharedBuffer is nullptr");

    // calculate the buffer beginning position when set renderRate
    curStaticDataPos_ = (processedBufferSize_ == 0 ? 0 : curStaticDataPos_ * 1.0 / processedBufferSize_ * bufferSize);
    uint32_t byteSizePerFrame = streamInfo_.channels * PcmFormatToBits(streamInfo_.format);
    curStaticDataPos_ = (byteSizePerFrame == 0 ? 0 : curStaticDataPos_ / byteSizePerFrame * byteSizePerFrame);

    sharedBuffer_->SetStaticPlayPosition(currentLoopTimes_, curStaticDataPos_);
    processedBuffer_ = *bufferBase;
    processedBufferSize_ = bufferSize;
}

void AudioStaticBufferProvider::SetLoopTimes(int64_t times)
{
    CHECK_AND_RETURN_LOG(sharedBuffer_ != nullptr, "sharedBuffer is nullptr");
    std::unique_lock<std::mutex> lock(eventMutex_);
    totalLoopTimes_ = times;
    currentLoopTimes_ = 0;
    curStaticDataPos_ = 0;
    sharedBuffer_->ResetBufferEndCallbackSendTimes();
    sharedBuffer_->SetIsNeedSendLoopEndCallback(false);
    sharedBuffer_->SetStaticPlayPosition(currentLoopTimes_, curStaticDataPos_);
    AUDIO_INFO_LOG("SetLoopTimes %{public}" PRId64, times);
}

void AudioStaticBufferProvider::RefreshBufferStatus()
{
    // fadeout needs to be done before resfresh bufferStatus
    if (needFadeOut_ && currentLoopTimes_ != totalLoopTimes_) {
        Trace trace1("RefreshBufferStatus need delay Refresh");
        delayRefreshBufferStatus_ = true;
        return;
    }
    Trace trace("RefreshBufferStatus");
    currentLoopTimes_ = 0;
    curStaticDataPos_ = 0;
    sharedBuffer_->SetStaticPlayPosition(currentLoopTimes_, curStaticDataPos_);
    sharedBuffer_->ResetBufferEndCallbackSendTimes();
    sharedBuffer_->SetIsNeedSendLoopEndCallback(false);
    AUDIO_INFO_LOG("RefreshBufferStatus, curTotalLoopTimes %{public}" PRId64, totalLoopTimes_);
    delayRefreshBufferStatus_ = false;
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
        AUDIO_ERR_LOG("CurrentLoopTimes Reach %{public}" PRId64, totalLoopTimes_);
        return ERROR;
    }
    return SUCCESS;
}

void AudioStaticBufferProvider::NeedProcessFadeIn()
{
    Trace trace("NeedProcessFadeIn");
    std::unique_lock<std::mutex> lock(eventMutex_);
    needFadeIn_ = true;
    playFinished_ = false;
}

void AudioStaticBufferProvider::NeedProcessFadeOut()
{
    Trace trace("NeedProcessFadeOut");
    std::unique_lock<std::mutex> lock(eventMutex_);
    needFadeOut_ = true;
}

bool AudioStaticBufferProvider::IsLoopEnd()
{
    std::unique_lock<std::mutex> lock(eventMutex_);
    return currentLoopTimes_ == totalLoopTimes_;
}

bool AudioStaticBufferProvider::NeedProvideData()
{
    if (currentLoopTimes_ == totalLoopTimes_ || playFinished_) {
        Trace tracezero("GetDataFromStaticBuffer ZeroData LoopEnd");
        return false;
    }

    if (sharedBuffer_->CheckFrozenAndSetLastProcessTime(BUFFER_IN_SERVER)) {
        Trace tracezero("GetDataFromStaticBuffer ZeroData ClientFreeze");
        return false;
    }

    CHECK_AND_RETURN_RET(sharedBuffer_->GetStreamStatus() != nullptr, false);
    if (sharedBuffer_->GetStreamStatus()->load() != STREAM_RUNNING && needFadeOut_) {
        return true;
    }

    if (sharedBuffer_->GetStreamStatus()->load() != STREAM_RUNNING &&
        sharedBuffer_->GetStreamStatus()->load() != STREAM_PAUSING &&
        sharedBuffer_->GetStreamStatus()->load() != STREAM_STOPPING) {
        Trace tracezero("GetDataFromStaticBuffer ZeroData IncorrectStreamStatus" +
            std::to_string(sharedBuffer_->GetStreamStatus()->load()));
        return false;
    }
    return true;
}

void AudioStaticBufferProvider::ResetStaticPlayPosition()
{
    std::unique_lock<std::mutex> lock(eventMutex_);
    RefreshBufferStatus();
}

} // namespace AudioStandard
} // namespace OHOS