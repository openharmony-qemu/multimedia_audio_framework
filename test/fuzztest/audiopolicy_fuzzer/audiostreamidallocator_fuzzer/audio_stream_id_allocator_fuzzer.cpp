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
#include "audio_stream_id_allocator.h"
#include <fuzzer/FuzzedDataProvider.h>

namespace OHOS {
namespace AudioStandard {
using namespace std;

const size_t THRESHOLD = 10;

void GenerateStreamIdFuzzTest(FuzzedDataProvider& fdp)
{
    AudioStreamIdAllocator::GetAudioStreamIdAllocator().GenerateStreamId();
}

void MultipleGenerateStreamIdFuzzTest(FuzzedDataProvider& fdp)
{
    uint32_t count = fdp.ConsumeIntegralInRange<uint32_t>(1, 100);
    for (uint32_t i = 0; i < count; i++) {
        AudioStreamIdAllocator::GetAudioStreamIdAllocator().GenerateStreamId();
    }
}

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
        GenerateStreamIdFuzzTest,
        MultipleGenerateStreamIdFuzzTest,
    });
    func(fdp);
}

} // namespace AudioStandard
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }
    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::Test(fdp);
    return 0;
}
