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
#include <string>
#include "audio_proresampler_process.h"
#include "audio_log.h"
#include "access_token.h"
#include "../fuzz_utils.h"
#define TWO_STEPS 2
#define THREE_STEPS 3
#define MAX_RATIO_INTEGRAL_METHOD 32

using namespace std;

namespace OHOS {
namespace AudioStandard {

FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
const int32_t NUM_1 = 1;
const int32_t NUM_2 = 2;
const int32_t NUM_3 = 3;
const int32_t NUM_5 = 5;
const int32_t NUM_11 = 11;

void GetMultiplyFilterFunFuzzTest()
{
    int32_t err = RESAMPLER_ERR_SUCCESS;

    uint32_t k = g_fuzzUtils.GetData<uint32_t>() % NUM_5 + NUM_1;
    uint32_t interpolateFactor = TWO_STEPS * k;

    uint32_t decimateFactor = interpolateFactor + NUM_1;
    if (decimateFactor > MAX_RATIO_INTEGRAL_METHOD) {
        decimateFactor = MAX_RATIO_INTEGRAL_METHOD;
        interpolateFactor = decimateFactor - NUM_1;
        interpolateFactor = (interpolateFactor / TWO_STEPS) * TWO_STEPS;
        if (interpolateFactor == 0) {
            return;
        }
    }
    uint32_t numChannels = (g_fuzzUtils.GetData<uint32_t>() % NUM_2) + NUM_1;
    int32_t quality = g_fuzzUtils.GetData<int32_t>() % NUM_11;
    SingleStagePolyphaseResamplerInit(numChannels, decimateFactor, interpolateFactor, quality, &err);
}

void SetResamplerFunctionCoarseFuzzTest()
{
    int32_t err = RESAMPLER_ERR_SUCCESS;

    uint32_t interpolateFactor = NUM_1;
    uint32_t decimateFactor = THREE_STEPS * interpolateFactor;
    uint32_t numChannels = g_fuzzUtils.GetData<uint32_t>() % NUM_3 + NUM_1;
    int32_t quality = g_fuzzUtils.GetData<int32_t>() % NUM_11;

    SingleStagePolyphaseResamplerInit(numChannels, decimateFactor, interpolateFactor, quality, &err);
}

vector <TestFuncs> g_testFuncs = {
    GetMultiplyFilterFunFuzzTest,
    SetResamplerFunctionCoarseFuzzTest,
};

} // namespace AudioStandard
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::AudioStandard::g_fuzzUtils.fuzzTest(data, size, OHOS::AudioStandard::g_testFuncs);
    return 0;
}
