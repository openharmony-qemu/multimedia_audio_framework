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

#ifndef VA_INPUT_STREAM_STUB_IMPL_TEST_H
#define VA_INPUT_STREAM_STUB_IMPL_TEST_H

#include <gtest/gtest.h>
#include "va_input_stream_stub_impl.h"


namespace OHOS {
namespace AudioStandard {

class VAInputStreamStubImplTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp(void);
    void TearDown(void);
};
using namespace OHOS::AudioStandard;

class HelloInputStream : public VAInputStreamCallback {
public:
    HelloInputStream() = default;
    virtual ~HelloInputStream() = default;

    int32_t Start() override
    {
        return 0;
    }
    int32_t Stop() override
    {
        return 0;
    }
    int32_t Close() override
    {
        return 0;
    }
    int32_t GetStreamProperty(VAAudioStreamProperty& streamProp) override
    {
        return 0;
    }
    int32_t RequestSharedMem(const VASharedMemInfo& memInfo) override
    {
        return 0;
    }
    int32_t GetCapturePosition(uint64_t& attr_1, uint64_t& attr_2) override
    {
        return 0;
    }
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif // VA_INPUT_STREAM_STUB_IMPL_TEST_H