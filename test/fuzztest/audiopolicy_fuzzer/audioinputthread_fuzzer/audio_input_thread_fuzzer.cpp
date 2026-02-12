/*
* Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <iostream>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <linux/input.h>
#include "audio_input_thread.h"
#include "audio_errors.h"
#include <fuzzer/FuzzedDataProvider.h>

namespace OHOS {
namespace AudioStandard {
using namespace std;

const size_t THRESHOLD = 10;
static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;

template<class T>
T GetData()
{
    T object {};
    size_t objectSize = sizeof(object);
    if (RAW_DATA == nullptr || objectSize > g_dataSize - g_pos) {
        return object;
    }
    errno_t ret = memcpy_s(&object, objectSize, RAW_DATA + g_pos, objectSize);
    if (ret != EOK) {
        return {};
    }
    g_pos += objectSize;
    return object;
}

void AudioPnpInputOpenFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInputThread::AudioPnpInputOpen();
}

void AudioPnpInputPollAndReadFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInputThread::AudioPnpInputPollAndRead();
}

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
        AudioPnpInputOpenFuzzTest,
        AudioPnpInputPollAndReadFuzzTest,
    });
    func(fdp);
}

void Init(const uint8_t* data, size_t size)
{
    if (data == nullptr) {
        return;
    }
    RAW_DATA = data;
    g_dataSize = size;
    g_pos = 0;
}

void Init()
{
}

} // namespace AudioStandard
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }
    OHOS::AudioStandard::Init(data, size);
    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::Test(fdp);
    return 0;
}

extern "C" int LLVMFuzzerInitialize(const uint8_t* data, size_t size)
{
    OHOS::AudioStandard::Init();
    return 0;
}
