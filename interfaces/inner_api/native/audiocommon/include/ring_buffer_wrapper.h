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
#ifndef RING_BUFFER_WRAPPER_H
#define RING_BUFFER_WRAPPER_H

#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <parcel.h>

#include "audio_buffer_desc.h"
#include "audio_errors.h"
#include "securec.h"

namespace OHOS {
namespace AudioStandard {
/* If buflength is 0, buffer must be nullptr;
   conversely, if buffer is nullptr, buflength must be 0.
   Otherwise, the behavior is undefined. */
struct BasicBufferDesc {
    uint8_t *buffer = nullptr;
    size_t bufLength = 0;

    int32_t SeekFromStart(size_t offset)
    {
        if (offset > bufLength) {
            return ERR_INVALID_PARAM;
        }

        bufLength -= offset;

        if (bufLength == 0) {
            buffer = nullptr;
        } else {
            buffer += offset;
        }
        return SUCCESS;
    }
};

/**
 * Precondition: dataLength must not exceed the total buffer capacity
 * (sum of all bufLength).
 * The two memory blocks must not overlap in any part of their address ranges.
 * If first buffer is null, second is null.
 * Violation results in undefined behavior.
 */
struct RingBufferWrapper {
    static inline constexpr size_t DESC_SIZE = 2;

    std::array<BasicBufferDesc, DESC_SIZE> basicBufferDescs = {};
    size_t dataLenth = 0;

    size_t GetBufferSize()
    {
        size_t size = 0;
        for (const auto &[buffer, bufLength] : basicBufferDescs) {
            size += bufLength;
        }
        return size;
    }

    void Reset()
    {
        dataLenth = 0;
        for (auto &[buffer, bufLength] : basicBufferDescs) {
            buffer = nullptr;
            bufLength = 0;
        }
    }

// use int because memset_s parm is int
    void SetMemTo(int ch)
    {
        for (auto &[buffer, bufLength] : basicBufferDescs) {
            if (buffer != nullptr && bufLength != 0) {
                memset_s(buffer, bufLength, ch, bufLength);
            }
        }
    }

    void SetDataTo(int ch)
    {
        size_t remainSize = dataLenth;
        for (auto &[buffer, bufLength] : basicBufferDescs) {
            size_t setSize = std::min(remainSize, bufLength);
            remainSize -= setSize;
            if (buffer != nullptr && bufLength != 0) {
                memset_s(buffer, bufLength, ch, setSize);
            }
        }
    }

    int32_t SeekFromStart(size_t offset)
    {
        if (offset > GetBufferSize()) {
            return ERR_INVALID_PARAM;
        }

        size_t remainSeek = offset;
        dataLenth <= offset ? dataLenth = 0 : dataLenth -= offset;

        for (auto &basicBuffer : basicBufferDescs) {
            size_t seekSize = std::min(remainSeek, basicBuffer.bufLength);
            basicBuffer.SeekFromStart(seekSize);
            remainSeek -= seekSize;
        }

        if (basicBufferDescs[0].bufLength == 0) {
            std::swap(basicBufferDescs[0], basicBufferDescs[1]);
        }
        return SUCCESS;
    }

    int32_t MemCopyFrom(const RingBufferWrapper &buffer)
    {
        if (GetBufferSize() < buffer.dataLenth) {
            return ERR_INVALID_PARAM;
        }

        RingBufferWrapper dstBuffer(*this);
        RingBufferWrapper srcBuffer(buffer);
        dstBuffer.dataLenth = buffer.dataLenth;

        size_t remainSize = buffer.dataLenth;
        while (remainSize > 0) {
            auto copySize = std::min({srcBuffer.basicBufferDescs[0].bufLength, dstBuffer.basicBufferDescs[0].bufLength,
                remainSize});
            remainSize -= copySize;

            if (copySize == 0) {
                // This branch should never be executed under any valid conditions. Consider let it crash?
                return ERR_INVALID_PARAM;
            }
            memcpy_s(dstBuffer.basicBufferDescs[0].buffer, dstBuffer.basicBufferDescs[0].bufLength,
                srcBuffer.basicBufferDescs[0].buffer, copySize);
            dstBuffer.SeekFromStart(copySize);
            srcBuffer.SeekFromStart(copySize);
        }

        dataLenth = buffer.dataLenth;
        return SUCCESS;
    }
};
} // namespace AudioStandard
} // namespace OHOS
#endif // RING_BUFFER_WRAPPER_H
