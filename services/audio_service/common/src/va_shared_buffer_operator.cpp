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
#define LOG_TAG "VASharedBufferOperator"
#endif

#include "va_shared_buffer_operator.h"
#include "audio_errors.h"
#include "audio_policy_log.h"

namespace OHOS {
namespace AudioStandard {

VASharedBufferOperator::VASharedBufferOperator(const VASharedBuffer &buffer) : buffer_(buffer)
{
    dataAshmem_ = buffer_.GetDataAshmem();
    CHECK_AND_RETURN_LOG(dataAshmem_ != nullptr, "dataAshmem_ is nullptr");
    capacity = buffer_.GetDataSize();
    InitVASharedStatusInfo();
}

void VASharedBufferOperator::InitVASharedStatusInfo()
{
    statusInfo_ = reinterpret_cast<VASharedStatusInfo *>(buffer_.GetStatusInfoBase());
    CHECK_AND_RETURN_LOG(statusInfo_ != nullptr, "statusInfo_ is null");
}

VASharedBufferOperator::~VASharedBufferOperator()
{}

void VASharedBufferOperator::SetMinReadSize(const size_t minReadSize)
{
    minReadSize_ = minReadSize;
}

void VASharedBufferOperator::Reset()
{
    CHECK_AND_RETURN_LOG(statusInfo_ != nullptr, "statusInfo_ is null");
    FutexCode retCode = WaitForFutex(timeoutInNano_, []() { return true; });
    CHECK_AND_RETURN_LOG(retCode == FUTEX_SUCCESS, "wait futex failed");
    statusInfo_->readPos = 0;
    statusInfo_->writePos = 0;
    WakeFutex();
}

void VASharedBufferOperator::SetReadPosToWritePos()
{
    CHECK_AND_RETURN_LOG(statusInfo_ != nullptr, "statusInfo_ is null");
    FutexCode retCode = WaitForFutex(timeoutInNano_, []() { return true; });
    CHECK_AND_RETURN_LOG(retCode == FUTEX_SUCCESS, "wait futex failed");
    statusInfo_->readPos = statusInfo_->writePos;
    WakeFutex();
}

size_t VASharedBufferOperator::GetReadableSize()
{
    CHECK_AND_RETURN_RET_LOG(statusInfo_ != nullptr, 0, "statusInfo_ is null");
    FutexCode retCode = WaitForFutex(timeoutInNano_, []() { return true; });
    CHECK_AND_RETURN_RET_LOG(retCode == FUTEX_SUCCESS, 0, "wait futex failed");
    size_t readableSize = statusInfo_->writePos - statusInfo_->readPos;
    WakeFutex();
    return readableSize;
}

size_t VASharedBufferOperator::GetReadableSizeNoLock()
{
    CHECK_AND_RETURN_RET_LOG(statusInfo_ != nullptr, 0, "statusInfo_ is nullptr");
    return statusInfo_->writePos - statusInfo_->readPos;
}

size_t VASharedBufferOperator::GetWritableSizeNoLock()
{
    CHECK_AND_RETURN_RET_LOG(statusInfo_ != nullptr, 0, "statusInfo_ is nullptr");
    return capacity - (statusInfo_->writePos - statusInfo_->readPos);
}

size_t VASharedBufferOperator::Read(uint8_t *data, size_t dataSize)
{
    CHECK_AND_RETURN_RET_LOG(statusInfo_ != nullptr, 0, "statusInfo_ is nullptr");
    CHECK_AND_RETURN_RET_LOG(data != nullptr, 0, "input data is nullptr");
    CHECK_AND_RETURN_RET_LOG(dataAshmem_ != nullptr, 0, "dataAshmem is nullptr");

    FutexCode retCode = WaitForFutex(timeoutInNano_, [this]() { return HasEnoughReadableData(); });
    CHECK_AND_RETURN_RET_LOG(retCode == FUTEX_SUCCESS, 0, "wait futex failed");

    const size_t readSize = std::min(dataSize, GetReadableSizeNoLock());
    if (readSize == 0) {
        AUDIO_INFO_LOG("wrong readSize: %{public}zu", readSize);
        WakeFutex();
        return 0;
    }

    int ret;
    size_t readIndex = statusInfo_->readPos % capacity;
    if (readIndex + readSize <= capacity) {
        const void *read_ptr = dataAshmem_->ReadFromAshmem(readSize, readIndex);
        ret = memcpy_s(data, dataSize, read_ptr, readSize);
        if (ret) {
            AUDIO_INFO_LOG("[1] Read failed");
            WakeFutex();
            return 0;
        }
    } else {
        size_t firstReadSize = capacity - readIndex;
        const void *first_read_ptr = dataAshmem_->ReadFromAshmem(firstReadSize, readIndex);
        ret = memcpy_s(data, dataSize, first_read_ptr, firstReadSize);
        if (ret) {
            AUDIO_INFO_LOG("[2] Read failed");
            WakeFutex();
            return 0;
        }
        size_t secondReadSize = readSize - firstReadSize;
        const void *second_read_ptr = dataAshmem_->ReadFromAshmem(secondReadSize, 0);
        ret = memcpy_s(data + firstReadSize, dataSize - firstReadSize, second_read_ptr, secondReadSize);
        if (ret) {
            AUDIO_INFO_LOG("[3] Read failed");
            WakeFutex();
            return 0;
        }
    }
    statusInfo_->readPos += readSize;

    WakeFutex();

    AUDIO_DEBUG_LOG("request readsize:%{public}zu, actual:%{public}zu", dataSize, readSize);

    return readSize;
}

size_t VASharedBufferOperator::Write(uint8_t *data, size_t dataSize)
{
    CHECK_AND_RETURN_RET_LOG(statusInfo_ != nullptr, 0, "statusInfo_ is nullptr");
    CHECK_AND_RETURN_RET_LOG(data != nullptr, 0, "data pointer is null");
    CHECK_AND_RETURN_RET_LOG(dataAshmem_ != nullptr, 0, "dataAshmem is nullptr");

    FutexCode retCode = WaitForFutex(timeoutInNano_, []() { return true; });
    CHECK_AND_RETURN_RET_LOG(retCode == FUTEX_SUCCESS, 0, "wait futex failed");

    size_t writeSize = std::min(dataSize, GetWritableSizeNoLock());
    if (writeSize == 0) {
        AUDIO_INFO_LOG("wrong writeSize: %{public}zu", writeSize);
        WakeFutex();
        return 0;
    }

    AUDIO_INFO_LOG("Write dataSize: %{public}zu. Actual writeSize: %{public}zu", dataSize, writeSize);

    bool success;
    size_t writeIndex = statusInfo_->writePos % capacity;

    if (writeIndex + writeSize <= capacity) {
        success = dataAshmem_->WriteToAshmem(data, writeSize, writeIndex);
        if (!success) {
            AUDIO_INFO_LOG("[1] Write failed");
            WakeFutex();
            return 0;
        }
    } else {
        size_t firstWriteSize = capacity - writeIndex;
        success = dataAshmem_->WriteToAshmem(data, firstWriteSize, writeIndex);
        if (!success) {
            AUDIO_INFO_LOG("[2] Write failed");
            WakeFutex();
            return 0;
        }
        size_t secondWriteSize = writeSize - firstWriteSize;
        success = dataAshmem_->WriteToAshmem(data + firstWriteSize, secondWriteSize, 0);
        if (!success) {
            AUDIO_INFO_LOG("[3] Write failed");
            WakeFutex();
            return 0;
        }
    }
    statusInfo_->writePos += writeSize;

    WakeFutex();

    return writeSize;
}

void VASharedBufferOperator::GetVASharedMemInfo(VASharedMemInfo &memInfo)
{
    buffer_.GetVASharedMemInfo(memInfo);
}

std::atomic<uint32_t>* VASharedBufferOperator::GetFutex()
{
    CHECK_AND_RETURN_RET_LOG(statusInfo_ != nullptr, nullptr, "statusInfo is null");
    return &statusInfo_->futex;
}

FutexCode VASharedBufferOperator::WaitForFutex(uint64_t timeoutInNs, const std::function<bool(void)> &pred)
{
    auto futex = GetFutex();
    CHECK_AND_RETURN_RET_LOG(futex != nullptr, FUTEX_INVALID_PARAMS, "futex is null");
    uint64_t startTime = ClockTime::GetCurNano();
    uint64_t curTime = ClockTime::GetCurNano();
    while (curTime - startTime < timeoutInNs) {
        uint32_t expected = LOCK_RELEASED;
        if (futex->compare_exchange_weak(expected, LOCK_OWNED) && pred()) {
            return FUTEX_SUCCESS;
        }
        std::this_thread::yield();
        curTime = ClockTime::GetCurNano();
    }
    if (curTime - startTime >= timeoutInNs) {
        return FUTEX_TIMEOUT;
    } else {
        return FUTEX_PRE_EXIT;
    }
}

void VASharedBufferOperator::WakeFutex()
{
    auto futex = GetFutex();
    CHECK_AND_RETURN_LOG(futex != nullptr, "futex is null");
    futex->store(LOCK_RELEASED);
}

void VASharedBufferOperator::WakeFutexIfNeed()
{
    if (statusInfo_) {
        FutexTool::FutexWake(&(statusInfo_->futex));
    }
}

bool VASharedBufferOperator::HasEnoughReadableData()
{
    return GetReadableSizeNoLock() > minReadSize_;
}
}  // namespace AudioStandard
}  // namespace OHOS