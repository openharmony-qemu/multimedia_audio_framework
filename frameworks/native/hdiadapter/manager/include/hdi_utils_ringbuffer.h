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

#ifndef HDI_UTILS_RINGBUFFER_H
#define HDI_UTILS_RINGBUFFER_H

#include <cstdint>
#include <mutex>
#include <string>

namespace OHOS {
namespace AudioStandard {

class HdiRingBuffer {
public:
    HdiRingBuffer() = default;
    ~HdiRingBuffer();

    void Init(const uint32_t sampleRate, const uint32_t channelCount, const uint32_t formatBytes,
        const uint32_t onceFrameNum = 2, const uint32_t maxFrameNum = 5);

    int32_t WriteDataToRingBuffer(uint8_t *data, uint32_t dataLen);
    int32_t ReadDataFromRingBuffer(uint8_t *data, uint32_t dataLen);

private:
    void AddWriteIndex(void);
    void AddReadIndex(void);
    uint8_t *buffer_ { nullptr };
    uint32_t maxBufferSize_ { 0 };
    uint32_t perFrameLength_ { 0 };
    uint32_t maxFrameNum_ { 0 };
    uint64_t readIdx_ { 0 };
    uint64_t writeIdx_ { 0 };
    std::mutex mtx_;
};

} // namespace AudioStandard
} // namespace OHOS
#endif // HDI_UTILS_RINGBUFFER_H