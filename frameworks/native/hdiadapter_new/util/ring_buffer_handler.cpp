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
#define LOG_TAG "RingBufferHandler"
#endif

#include "util/ring_buffer_handler.h"
#include "securec.h"
#include "audio_hdi_log.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
RingBufferHandler::~RingBufferHandler()
{
    if (buffer_ != nullptr) {
        delete[] buffer_;
        buffer_ = nullptr;
    }
}

void RingBufferHandler::Init(const uint32_t sampleRate, const uint32_t channelCount, const uint32_t formatBytes,
    const uint32_t onceFrameNum, const uint32_t maxFrameNum)
{
    std::lock_guard<std::mutex> lock(mutex_);
    perFrameLength_ = ((sampleRate * onceFrameNum) / PER_FRAME_LENGTH_RATE) * channelCount * formatBytes;
    maxBufferSize_ = perFrameLength_ * maxFrameNum;
    CHECK_AND_RETURN_LOG(maxBufferSize_ > 0, "invalid param, maxBufferSize: %{public}u", maxBufferSize_);
    maxFrameNum_ = maxFrameNum;

    buffer_ = new uint8_t[maxBufferSize_];
    CHECK_AND_RETURN_LOG(buffer_ != nullptr, "alloc buffer fail");
    memset_s(static_cast<void *>(buffer_), maxBufferSize_, 0, maxBufferSize_);
}

int32_t RingBufferHandler::WriteDataToRingBuffer(uint8_t *data, uint32_t dataLen)
{
    CHECK_AND_RETURN_RET_LOG(data != nullptr, ERR_INVALID_PARAM, "data is nullptr");
    CHECK_AND_RETURN_RET_LOG(dataLen == perFrameLength_, ERR_INVALID_PARAM, "dataLen not equal perFrameLength");

    std::lock_guard<std::mutex> lock(mutex_);
    uint32_t offset = (writeIdx_ % maxFrameNum_) * perFrameLength_;
    CHECK_AND_RETURN_RET_LOG(buffer_ != nullptr, ERR_INVALID_PARAM, "buffer is nullptr");
    auto ret = memcpy_s(buffer_ + offset, maxBufferSize_ - offset, data, dataLen);
    CHECK_AND_RETURN_RET_LOG(ret == EOK, ERR_WRITE_BUFFER, "write ring buffer fail");
    AddWriteIndex();

    return SUCCESS;
}

int32_t RingBufferHandler::ReadDataFromRingBuffer(uint8_t *data, uint32_t dataLen)
{
    CHECK_AND_RETURN_RET_LOG(data != nullptr, ERR_INVALID_PARAM, "data is nullptr");
    CHECK_AND_RETURN_RET_LOG(dataLen == perFrameLength_, ERR_INVALID_PARAM, "dataLen not equal perFrameLength");

    std::lock_guard<std::mutex> lock(mutex_);
    if (readIdx_ >= writeIdx_) {
        static_cast<void>(memset_s(data, dataLen, 0, dataLen));
        return SUCCESS;
    }
    uint32_t offset = (readIdx_ % maxFrameNum_) * perFrameLength_;
    CHECK_AND_RETURN_RET_LOG(buffer_ != nullptr, ERR_INVALID_PARAM, "buffer is nullptr");
    auto ret = memcpy_s(data, dataLen, buffer_ + offset, perFrameLength_);
    CHECK_AND_RETURN_RET_LOG(ret == EOK, ERR_READ_BUFFER, "read ring buffer fail");
    AddReadIndex();

    return SUCCESS;
}

void RingBufferHandler::AddWriteIndex(void)
{
    if (writeIdx_ < UINT64_MAX) {
        ++writeIdx_;
        return;
    }

    uint64_t diff = (writeIdx_ - readIdx_) % maxFrameNum_;
    writeIdx_ = writeIdx_ % maxFrameNum_;
    readIdx_ = writeIdx_ - diff;
    ++writeIdx_;
}

void RingBufferHandler::AddReadIndex(void)
{
    if (readIdx_ < UINT64_MAX) {
        ++readIdx_;
        return;
    }

    readIdx_ = 0;
}

} // namespace AudioStandard
} // namespace OHOS
