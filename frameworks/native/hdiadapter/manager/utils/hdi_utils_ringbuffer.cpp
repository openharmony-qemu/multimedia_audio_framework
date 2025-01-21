/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#undef LOG_TAG
#define LOG_TAG "HdiUtilsRingBuffer"

#include "hdi_utils_ringbuffer.h"

#include "securec.h"
#include "audio_hdi_log.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
namespace {
const int32_t PER_FRAME_LENGTH_RATE = 100;
}

HdiRingBuffer::~HdiRingBuffer()
{
    if (buffer_ != nullptr) {
        delete [] buffer_;
        buffer_ = nullptr;
    }
}

void HdiRingBuffer::Init(const uint32_t sampleRate, const uint32_t channelCount, const uint32_t formatBytes,
    const uint32_t onceFrameNum, const uint32_t maxFrameNum)
{
    perFrameLength_ = ((sampleRate * onceFrameNum) / PER_FRAME_LENGTH_RATE) * channelCount * formatBytes;
    maxBufferSize_ = perFrameLength_ * maxFrameNum;
    maxFrameNum_ = maxFrameNum;
    if (maxBufferSize_ <= 0) {
        AUDIO_ERR_LOG("maxBufferSize_: %{public}u is error", maxBufferSize_);
        return;
    }

    buffer_ = new uint8_t[maxBufferSize_];
    if (buffer_ != nullptr) {
        memset_s(static_cast<void *>(buffer_), maxBufferSize_, 0, maxBufferSize_);
    } else {
        AUDIO_ERR_LOG("error: new ringBuffer data failed.");
    }
}

void HdiRingBuffer::AddWriteIndex(void)
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

void HdiRingBuffer::AddReadIndex(void)
{
    if (readIdx_ < UINT64_MAX) {
        ++readIdx_;
        return;
    }

    readIdx_ = 0;
}

int32_t HdiRingBuffer::WriteDataToRingBuffer(uint8_t *data, uint32_t dataLen)
{
    CHECK_AND_RETURN_RET_LOG(data != nullptr, ERR_INVALID_PARAM, "data is null");
    CHECK_AND_RETURN_RET_LOG(dataLen == perFrameLength_, ERR_INVALID_PARAM, "dataLen != perFrameLength_");

    std::lock_guard<std::mutex> lock(mtx_);
    uint32_t offset = (writeIdx_ % maxFrameNum_) * perFrameLength_;
    CHECK_AND_RETURN_RET_LOG(buffer_ != nullptr, ERR_INVALID_PARAM, "buffer_ is null");
    auto memcpyRet = memcpy_s(buffer_ + offset, maxBufferSize_ - offset, data, dataLen);
    CHECK_AND_RETURN_RET_LOG(memcpyRet == EOK, ERR_READ_BUFFER, "write ringbuffer fail");
    AddWriteIndex();

    return SUCCESS;
}

int32_t HdiRingBuffer::ReadDataFromRingBuffer(uint8_t *data, uint32_t dataLen)
{
    CHECK_AND_RETURN_RET_LOG(data != nullptr, ERR_INVALID_PARAM, "data is null");
    CHECK_AND_RETURN_RET_LOG(dataLen == perFrameLength_, ERR_INVALID_PARAM, "dataLen != perFrameLength_");

    std::lock_guard<std::mutex> lock(mtx_);
    if (readIdx_ >= writeIdx_) {
        static_cast<void>(memset_s(data, dataLen, 0, dataLen));
        return SUCCESS;
    }

    uint32_t offset = (readIdx_ % maxFrameNum_) * perFrameLength_;
    CHECK_AND_RETURN_RET_LOG(buffer_ != nullptr, ERR_INVALID_PARAM, "buffer_ is null");
    auto memcpyRet = memcpy_s(data, dataLen, buffer_ + offset, perFrameLength_);
    CHECK_AND_RETURN_RET_LOG(memcpyRet == EOK, ERR_READ_BUFFER, "read ringbuffer fail");
    AddReadIndex();

    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS