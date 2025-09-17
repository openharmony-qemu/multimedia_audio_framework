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


#include "audio_suite_common.h"
#include "audio_suite_log.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

static constexpr uint32_t MAX_CACHE = std::numeric_limits<uint32_t>::max() - 1; // 最大分配容量

int32_t AudioSuiteRingBuffer::PushData(uint8_t* byteData, uint32_t size)
{
    if (size == 0) {
        return ERR_INVALID_OPERATION;
    }
    // 分两段复制：从tail到缓冲区末尾，然后从缓冲区头继续
    size_t firstChunk = std::min(size, capacity_ - tail_);
    errno_t err = memcpy_s(buffer_ + tail_, capacity_ - tail_, byteData, firstChunk);
    if (err != 0) {
        AUDIO_INFO_LOG("AudioSuiteRingBuffer::PushData error capacity_:%{public}u,"
            "tail_:%{public}u, size:%{public}u", capacity_, tail_, size);
        return ERR_INVALID_OPERATION;
    }

    if (size > firstChunk) {
        err = memcpy_s(buffer_, capacity_, byteData + firstChunk, size - firstChunk);
        if (err != 0) {
            AUDIO_INFO_LOG("AudioSuiteRingBuffer::PushData error capacity_:%{public}u,"
                "tail_:%{public}u, size:%{public}u, firstChunk:%{public}zu", capacity_, tail_, size, firstChunk);
            return ERR_INVALID_OPERATION;
        }
    }

    tail_ = (tail_ + size) % capacity_;
    size_ += size;
    return SUCCESS;
}
int32_t AudioSuiteRingBuffer::GetData(uint8_t* byteData, uint32_t size)
{
    if (size == 0) {
        return ERR_INVALID_OPERATION;
    }
    size_t firstChunk = std::min(size, capacity_ - head_);
    errno_t err = memcpy_s(byteData, size, buffer_ + head_, firstChunk);
    if (err != 0) {
        AUDIO_INFO_LOG("AudioSuiteRingBuffer::PushData error capacity_:%{public}u,"
            "head_:%{public}u, size:%{public}u", capacity_, head_, size);
        return ERR_INVALID_OPERATION;
    }

    if (size > firstChunk) {
        err = memcpy_s(byteData + firstChunk, size - firstChunk, buffer_, size - firstChunk);
        if (err != 0) {
            AUDIO_INFO_LOG("AudioSuiteRingBuffer::PushData error capacity_:%{public}u,"
                "head_:%{public}u, size:%{public}u, firstChunk:%{public}zu", capacity_, head_, size, firstChunk);
            return ERR_INVALID_OPERATION;
        }
    }

    head_ = (head_ + size) % capacity_;
    size_ -= size;
    return SUCCESS;
}

int32_t AudioSuiteRingBuffer::ResizeBuffer(uint32_t size)
{
    delete[] buffer_;
    if (size <= 0 || size > MAX_CACHE) {
        return ERROR;
    }
    buffer_ = new uint8_t[size];
    capacity_ = size;
    head_ = 0;
    tail_ = 0;
    size_ = 0;
    return 0;
}

int32_t AudioSuiteRingBuffer::ClearBuffer()
{
    head_ = 0;
    tail_ = 0;
    size_ = 0;
    return 0;
}
}
}
}