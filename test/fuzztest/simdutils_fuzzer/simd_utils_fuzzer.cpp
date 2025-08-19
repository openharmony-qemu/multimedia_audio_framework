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

#include <iostream>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include "audio_device_info.h"
#include "simd_utils.h"
#include "audio_effect_volume.h"
#include "../fuzz_utils.h"

using namespace std;
using namespace OHOS::AudioStandard::HPAE;
namespace OHOS {
namespace AudioStandard {
FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
typedef void (*TestFuncs)();
const size_t FUZZ_INPUT_SIZE_THRESHOLD = 10;
const size_t ROW_LENGTH = 5;

void SimdUtilsSimdPointByPointAddFuzzTest()
{
    const uint8_t randomLength = (g_fuzzUtils.GetData<uint8_t>() % ROW_LENGTH) + 1;

    std::vector<float> inputLeft(randomLength);
    std::vector<float> inputRight(randomLength);
    std::vector<float> output(randomLength);

    for (size_t i = 0; i < randomLength; ++i) {
        inputLeft[i] = g_fuzzUtils.GetData<float>();
        inputRight[i] = g_fuzzUtils.GetData<float>();
    }

    SimdPointByPointAdd(randomLength, inputLeft.data(), inputRight.data(), output.data());
}

void SimdUtilsSimdPointByPointSubFuzzTest()
{
    const uint8_t randomLength = (g_fuzzUtils.GetData<uint8_t>() % ROW_LENGTH) + 1;

    std::vector<float> inputLeft(randomLength);
    std::vector<float> inputRight(randomLength);
    std::vector<float> output(randomLength);

    for (size_t i = 0; i < randomLength; ++i) {
        inputLeft[i] = g_fuzzUtils.GetData<float>();
        inputRight[i] = g_fuzzUtils.GetData<float>();
    }

    SimdPointByPointSub(randomLength, inputLeft.data(), inputRight.data(), output.data());
}

void SimdUtilsSimdPointByPointMulFuzzTest()
{
    const uint8_t randomLength = (g_fuzzUtils.GetData<uint8_t>() % ROW_LENGTH) + 1;

    std::vector<float> inputLeft(randomLength);
    std::vector<float> inputRight(randomLength);
    std::vector<float> output(randomLength);

    for (size_t i = 0; i < randomLength; ++i) {
        inputLeft[i] = g_fuzzUtils.GetData<float>();
        inputRight[i] = g_fuzzUtils.GetData<float>();
    }

    SimdPointByPointMul(randomLength, inputLeft.data(), inputRight.data(), output.data());
}

vector<TestFuncs> g_testFuncs = {
    SimdUtilsSimdPointByPointAddFuzzTest,
    SimdUtilsSimdPointByPointSubFuzzTest,
    SimdUtilsSimdPointByPointMulFuzzTest,
};

} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::AudioStandard::FUZZ_INPUT_SIZE_THRESHOLD) {
        return 0;
    }
    OHOS::AudioStandard::g_fuzzUtils.fuzzTest(data, size, OHOS::AudioStandard::g_testFuncs);
    return 0;
}
