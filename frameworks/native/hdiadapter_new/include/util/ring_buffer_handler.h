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

#ifndef RING_BUFFER_HANDLER_H
#define RING_BUFFER_HANDLER_H

#include <string>
#include <mutex>
#include "common/hdi_adapter_info.h"

namespace OHOS {
namespace AudioStandard {
class RingBufferHandler {
public:
    RingBufferHandler() = default;
    ~RingBufferHandler();

    void Init(const uint32_t sampleRate, const uint32_t channelCount, const uint32_t formatBytes,
        const uint32_t onceFrameNum = DEFAULT_ONCE_FRAME_NUM, const uint32_t maxFrameNum = DEFAULT_MAX_FRAME_NUM);
    int32_t WriteDataToRingBuffer(uint8_t *data, uint32_t dataLen);
    int32_t ReadDataFromRingBuffer(uint8_t *data, uint32_t dataLen);

private:
    void AddWriteIndex(void);
    void AddReadIndex(void);

private:
    static constexpr int32_t DEFAULT_ONCE_FRAME_NUM = 2;
    static constexpr int32_t DEFAULT_MAX_FRAME_NUM = 5;
    static constexpr int32_t PER_FRAME_LENGTH_RATE = 100;

    uint8_t *buffer_ = nullptr;
    uint32_t maxBufferSize_ = 0;
    uint32_t perFrameLength_ = 0;
    uint32_t maxFrameNum_ = 0;
    uint64_t readIdx_ = 0;
    uint64_t writeIdx_ = 0;
    std::mutex mutex_;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // RING_BUFFER_HANDLER_H
