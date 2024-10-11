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

#ifndef I_CAPTURER_STREAM_H
#define I_CAPTURER_STREAM_H

#include "i_stream.h"
#include "audio_info.h"
#include "audio_stream_info.h"

namespace OHOS {
namespace AudioStandard {
class IReadCallback {
public:
    virtual int32_t OnReadData(size_t length) = 0;
};

class ICapturerStream : public IStream {
public:
    virtual ~ICapturerStream() = default;
    virtual int32_t GetStreamFramesRead(uint64_t &framesRead) = 0;
    virtual int32_t GetCurrentTimeStamp(uint64_t &timestamp) = 0;
    virtual int32_t GetLatency(uint64_t &latency) = 0;

    virtual void RegisterReadCallback(const std::weak_ptr<IReadCallback> &callback) = 0;
    virtual int32_t GetMinimumBufferSize(size_t &minBufferSize) const = 0;
    virtual void GetByteSizePerFrame(size_t &byteSizePerFrame) const = 0;
    virtual void GetSpanSizePerFrame(size_t &spanSizeInFrame) const = 0;
    virtual int32_t DropBuffer() = 0;
    virtual void AbortCallback(int32_t abortTimes) = 0;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // I_CAPTURER_STREAM_H
