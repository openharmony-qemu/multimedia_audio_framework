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

#ifndef HPAE_CAPTURER_STREAM_IMPL_FUZZER_H
#define HPAE_CAPTURER_STREAM_IMPL_FUZZER_H

#include "hpae_capturer_stream_impl.h"
#include "i_stream.h"
#include "i_capturer_stream.h"
namespace OHOS {
namespace AudioStandard {
class IIReadCallback : public IReadCallback {
public:
    IIReadCallback() = default;

    virtual ~IIReadCallback() = default;

    int32_t OnReadData(size_t length) override
    {
        return 0;
    }

    int32_t OnReadData(int8_t *outputData, size_t requestDataLen) override
    {
        return 0;
    }
};

class IIStatusCallback : public IStatusCallback {
public:
    IIStatusCallback() = default;

    virtual ~IIStatusCallback() = default;

    void OnStatusUpdate(IOperation operation) override {}
};
}
}
#endif