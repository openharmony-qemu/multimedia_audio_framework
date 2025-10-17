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

#ifndef VA_INPUT_STREAM_STUB_IMPL_H
#define VA_INPUT_STREAM_STUB_IMPL_H

#include "virtual_audio_interface.h"
#include "va_input_stream_stub.h"

namespace OHOS {
namespace AudioStandard {

class VAInputStreamStubImpl : public VAInputStreamStub {
public:
    VAInputStreamStubImpl();
    virtual ~VAInputStreamStubImpl();

    int32_t SetVAInputStreamCallback(const std::shared_ptr<VAInputStreamCallback> &callback);

    int32_t GetStreamProperty(VAAudioStreamProperty& streamProp) override;
    int32_t RequestSharedMem(const VASharedMemInfo& memInfo) override;
    int32_t Start() override;
    int32_t Stop() override;
    int32_t Close() override;
    int32_t GetCapturePosition(uint64_t& attr_1, uint64_t& attr_2) override;

private:
    std::shared_ptr<VAInputStreamCallback> vaInputStreamCallback_;
};

} // namespace AudioStandard
} // namespace OHOS
#endif // VA_INPUT_STREAM_STUB_IMPL_H