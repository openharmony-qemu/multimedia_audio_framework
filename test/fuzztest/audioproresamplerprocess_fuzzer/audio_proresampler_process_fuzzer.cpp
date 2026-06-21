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
#include <vector>
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
const uint32_t FUZZ_BUF_IN_LEN = 256;
const uint32_t FUZZ_BUF_OUT_LEN = 1024;
// Channel-count / ratio moduli for fuzz clamping (base reuses NUM_3 / NUM_1 above).
const uint32_t CHANNEL_COUNT_MOD = 6;   // % N + NUM_3 -> 3..8 channels
const uint32_t RATIO_MOD = 31;          // % N + NUM_1 -> 1..31 decim/interp
// Small buffer lengths (samples) for the reset-mem and streaming Process fuzz cases.
const uint32_t PROC_BUF_IN_LEN = 128;
const uint32_t PROC_BUF_OUT_LEN = 256;
const uint32_t STREAM_BUF_IN_LEN = 64;
const uint32_t STREAM_BUF_OUT_LEN = 128;

// Helper: Init resampler, run Process, then Free
static void ProcessAndFree(uint32_t channels, uint32_t decim, uint32_t interp, int32_t quality)
{
    int32_t err = RESAMPLER_ERR_SUCCESS;
    SingleStagePolyphaseResamplerState *state =
        SingleStagePolyphaseResamplerInit(channels, decim, interp, quality, &err);
    if (state == nullptr || err != RESAMPLER_ERR_SUCCESS) {
        return;
    }

    uint32_t inLen = FUZZ_BUF_IN_LEN;
    uint32_t outLen = FUZZ_BUF_OUT_LEN;
    vector<float> inBuf(inLen * channels, 0.5f);
    vector<float> outBuf(outLen * channels, 0.0f);

    SingleStagePolyphaseResamplerProcess(state, inBuf.data(), &inLen, outBuf.data(), &outLen);
    SingleStagePolyphaseResamplerFree(state);
}

// Original test 1: GetMultiplyFilterFun with random factors
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

// Original test 2: Coarse path (downsample by 3)
void SetResamplerFunctionCoarseFuzzTest()
{
    int32_t err = RESAMPLER_ERR_SUCCESS;

    uint32_t interpolateFactor = NUM_1;
    uint32_t decimateFactor = THREE_STEPS * interpolateFactor;
    uint32_t numChannels = g_fuzzUtils.GetData<uint32_t>() % NUM_3 + NUM_1;
    int32_t quality = g_fuzzUtils.GetData<int32_t>() % NUM_11;

    SingleStagePolyphaseResamplerInit(numChannels, decimateFactor, interpolateFactor, quality, &err);
}

// === Fine path (both decim and interp > 1) ===
// Covers: PolyphaseResamplerMono/Stereo/Multichannel + MultiplyFilterMono/Stereo/Multichannel + ApplyResampler

void ProcessFineMonoFuzzTest()
{
    ProcessAndFree(1, 2, 3, g_fuzzUtils.GetData<int32_t>() % NUM_11);
}

void ProcessFineStereoFuzzTest()
{
    ProcessAndFree(2, 2, 3, g_fuzzUtils.GetData<int32_t>() % NUM_11);
}

void ProcessFineMultiChFuzzTest()
{
    uint32_t ch = g_fuzzUtils.GetData<uint32_t>() % CHANNEL_COUNT_MOD + NUM_3; // 3-8 channels
    ProcessAndFree(ch, 2, 3, g_fuzzUtils.GetData<int32_t>() % NUM_11);
}

// === Halfband path (2*interp == decim) ===
// Covers: PolyphaseDownsamplerHalfbandMono/Stereo/Multichannel

void ProcessHalfbandMonoFuzzTest()
{
    ProcessAndFree(1, 2, 1, g_fuzzUtils.GetData<int32_t>() % NUM_11);
}

void ProcessHalfbandStereoFuzzTest()
{
    ProcessAndFree(2, 2, 1, g_fuzzUtils.GetData<int32_t>() % NUM_11);
}

void ProcessHalfbandMultiChFuzzTest()
{
    uint32_t ch = g_fuzzUtils.GetData<uint32_t>() % CHANNEL_COUNT_MOD + NUM_3;
    ProcessAndFree(ch, 2, 1, g_fuzzUtils.GetData<int32_t>() % NUM_11);
}

// === Thirdband path (3*interp == decim) ===
// Covers: PolyphaseDownsamplerThirdbandMono/Stereo/Multichannel

void ProcessThirdbandMonoFuzzTest()
{
    ProcessAndFree(1, 3, 1, g_fuzzUtils.GetData<int32_t>() % NUM_11);
}

void ProcessThirdbandStereoFuzzTest()
{
    ProcessAndFree(2, 3, 1, g_fuzzUtils.GetData<int32_t>() % NUM_11);
}

void ProcessThirdbandMultiChFuzzTest()
{
    uint32_t ch = g_fuzzUtils.GetData<uint32_t>() % CHANNEL_COUNT_MOD + NUM_3;
    ProcessAndFree(ch, 3, 1, g_fuzzUtils.GetData<int32_t>() % NUM_11);
}

// === Coarse upsampling (decim=1, interp > 1) ===
// Covers: PolyphaseResamplerCoarse + SymmetricOddUp + SymmetricEvenUp + MultiplyFilterMono/Stereo/Multi

void ProcessCoarseUpMonoFuzzTest()
{
    ProcessAndFree(1, 1, 4, g_fuzzUtils.GetData<int32_t>() % NUM_11);
}

void ProcessCoarseUpStereoFuzzTest()
{
    ProcessAndFree(2, 1, 4, g_fuzzUtils.GetData<int32_t>() % NUM_11);
}

void ProcessCoarseUpMultiChFuzzTest()
{
    uint32_t ch = g_fuzzUtils.GetData<uint32_t>() % CHANNEL_COUNT_MOD + NUM_3;
    ProcessAndFree(ch, 1, 4, g_fuzzUtils.GetData<int32_t>() % NUM_11);
}

// === Coarse downsampling (decim > 1, interp=1) ===
// Covers: PolyphaseResamplerCoarse + SymmetricOddDown

void ProcessCoarseDownMonoFuzzTest()
{
    ProcessAndFree(1, 5, 1, g_fuzzUtils.GetData<int32_t>() % NUM_11);
}

void ProcessCoarseDownStereoFuzzTest()
{
    ProcessAndFree(2, 5, 1, g_fuzzUtils.GetData<int32_t>() % NUM_11);
}

void ProcessCoarseDownMultiChFuzzTest()
{
    uint32_t ch = g_fuzzUtils.GetData<uint32_t>() % CHANNEL_COUNT_MOD + NUM_3;
    ProcessAndFree(ch, 5, 1, g_fuzzUtils.GetData<int32_t>() % NUM_11);
}

// === SetRate: covers UpdateResamplerState, UpdateFilterMemory, ComputeGcd ===

void SetRateFuzzTest()
{
    int32_t err = RESAMPLER_ERR_SUCCESS;
    uint32_t ch = g_fuzzUtils.GetData<uint32_t>() % NUM_3 + NUM_1;
    SingleStagePolyphaseResamplerState *state =
        SingleStagePolyphaseResamplerInit(ch, 2, 3, g_fuzzUtils.GetData<int32_t>() % NUM_11, &err);
    if (state == nullptr) {
        return;
    }
    // Change rate: covers ComputeGcd, UpdateResamplerState, UpdateFilterMemory
    uint32_t newDecim = g_fuzzUtils.GetData<uint32_t>() % RATIO_MOD + NUM_1;
    uint32_t newInterp = g_fuzzUtils.GetData<uint32_t>() % RATIO_MOD + NUM_1;
    SingleStagePolyphaseResamplerSetRate(state, newDecim, newInterp);

    uint32_t inLen = FUZZ_BUF_IN_LEN;
    uint32_t outLen = FUZZ_BUF_OUT_LEN;
    vector<float> inBuf(inLen * ch, 0.5f);
    vector<float> outBuf(outLen * ch, 0.0f);
    SingleStagePolyphaseResamplerProcess(state, inBuf.data(), &inLen, outBuf.data(), &outLen);
    SingleStagePolyphaseResamplerFree(state);
}

// === ResetMem + SkipHalfTaps ===

void ResetMemAndSkipHalfTapsFuzzTest()
{
    int32_t err = RESAMPLER_ERR_SUCCESS;
    uint32_t ch = g_fuzzUtils.GetData<uint32_t>() % NUM_3 + NUM_1;
    SingleStagePolyphaseResamplerState *state =
        SingleStagePolyphaseResamplerInit(ch, 2, 3, g_fuzzUtils.GetData<int32_t>() % NUM_11, &err);
    if (state == nullptr) {
        return;
    }
    // Process some data first
    uint32_t inLen = PROC_BUF_IN_LEN;
    uint32_t outLen = PROC_BUF_OUT_LEN;
    vector<float> inBuf(inLen * ch, 0.3f);
    vector<float> outBuf(outLen * ch, 0.0f);
    SingleStagePolyphaseResamplerProcess(state, inBuf.data(), &inLen, outBuf.data(), &outLen);

    // Reset and skip half taps
    SingleStagePolyphaseResamplerResetMem(state);
    SingleStagePolyphaseResamplerSkipHalfTaps(state);

    // Process again after reset
    inLen = PROC_BUF_IN_LEN;
    outLen = PROC_BUF_OUT_LEN;
    SingleStagePolyphaseResamplerProcess(state, inBuf.data(), &inLen, outBuf.data(), &outLen);
    SingleStagePolyphaseResamplerFree(state);
}

// === Free: normal free + double-free safety ===

void FreeAndNullFuzzTest()
{
    int32_t err = RESAMPLER_ERR_SUCCESS;
    uint32_t ch = g_fuzzUtils.GetData<uint32_t>() % NUM_3 + NUM_1;
    SingleStagePolyphaseResamplerState *state =
        SingleStagePolyphaseResamplerInit(ch, 2, 1, g_fuzzUtils.GetData<int32_t>() % NUM_11, &err);
    if (state) {
        SingleStagePolyphaseResamplerFree(state);
    }
    // Free with null pointer (should not crash)
    SingleStagePolyphaseResamplerFree(nullptr);
}

// === Null input buffer in Process ===

void ProcessNullInputFuzzTest()
{
    int32_t err = RESAMPLER_ERR_SUCCESS;
    uint32_t ch = g_fuzzUtils.GetData<uint32_t>() % NUM_3 + NUM_1;
    SingleStagePolyphaseResamplerState *state =
        SingleStagePolyphaseResamplerInit(ch, 2, 3, g_fuzzUtils.GetData<int32_t>() % NUM_11, &err);
    if (state == nullptr) {
        return;
    }
    uint32_t inLen = FUZZ_BUF_IN_LEN;
    uint32_t outLen = FUZZ_BUF_OUT_LEN;
    vector<float> outBuf(outLen * ch, 0.0f);
    // Pass null input buffer — Process should handle it
    SingleStagePolyphaseResamplerProcess(state, nullptr, &inLen, outBuf.data(), &outLen);
    SingleStagePolyphaseResamplerFree(state);
}

// === Multiple Process calls (streaming) ===

void ProcessStreamingFuzzTest()
{
    int32_t err = RESAMPLER_ERR_SUCCESS;
    uint32_t ch = g_fuzzUtils.GetData<uint32_t>() % NUM_3 + NUM_1;
    SingleStagePolyphaseResamplerState *state =
        SingleStagePolyphaseResamplerInit(ch, 480, 441, g_fuzzUtils.GetData<int32_t>() % NUM_11, &err);
    if (state == nullptr) {
        return;
    }
    // Simulate streaming: multiple small Process calls
    for (int i = 0; i < 3; i++) {
        uint32_t inLen = STREAM_BUF_IN_LEN;
        uint32_t outLen = STREAM_BUF_OUT_LEN;
        vector<float> inBuf(inLen * ch, 0.5f);
        vector<float> outBuf(outLen * ch, 0.0f);
        SingleStagePolyphaseResamplerProcess(state, inBuf.data(), &inLen, outBuf.data(), &outLen);
    }
    SingleStagePolyphaseResamplerFree(state);
}

// === Argument validation: illegal args must return NULL/error and NOT crash ===
// Covers Init early-return validation lines (numChannels=0, factors=0, quality>10/<0)
// and SetRate(0,0) -> RESAMPLER_ERR_INVALID_ARG.
void ArgValidationFuzzTest()
{
    int32_t err = RESAMPLER_ERR_SUCCESS;
    (void)SingleStagePolyphaseResamplerInit(0, 2, 1, 5, &err);  // numChannels = 0
    (void)SingleStagePolyphaseResamplerInit(2, 0, 1, 5, &err);  // decimate = 0
    (void)SingleStagePolyphaseResamplerInit(2, 2, 0, 5, &err);  // interpolate = 0
    (void)SingleStagePolyphaseResamplerInit(2, 2, 1, NUM_11, &err);  // quality > 10
    (void)SingleStagePolyphaseResamplerInit(2, 2, 1, -1, &err); // quality < 0
    SingleStagePolyphaseResamplerState *state = SingleStagePolyphaseResamplerInit(2, 2, 1, 5, &err);
    if (state != nullptr) {
        (void)SingleStagePolyphaseResamplerSetRate(state, 0, 0); // expect INVALID_ARG
        SingleStagePolyphaseResamplerFree(state);
    }
    SingleStagePolyphaseResamplerFree(nullptr); // must not crash
}

// === Overflow guards: large factors + max quality (256 taps) ===
// Covers CalculateFilter INT_MAX/sizeof/pFactor overflow check, MAX_DWNSMPLE_FILT_LEN
// clamp, and filterLength round-up-to-8 in UpdateResamplerState.
void OverflowGuardFuzzTest()
{
    int32_t err = RESAMPLER_ERR_SUCCESS;
    uint32_t ch = g_fuzzUtils.GetData<uint32_t>() % NUM_3 + NUM_1; // 1..3 channels
    uint32_t dec = (g_fuzzUtils.GetData<uint32_t>() % 64) + 1;     // 1..64
    uint32_t interp = (g_fuzzUtils.GetData<uint32_t>() % 64) + 1;  // 1..64
    SingleStagePolyphaseResamplerState *state =
        SingleStagePolyphaseResamplerInit(ch, dec, interp, NUM_11 - 1, &err); // quality=10 -> 256 taps
    if (state == nullptr) {
        return;
    }
    uint32_t inLen = FUZZ_BUF_IN_LEN;
    uint32_t outLen = FUZZ_BUF_OUT_LEN;
    vector<float> inBuf(inLen * ch, 0.5f);
    vector<float> outBuf(outLen * ch, 0.0f);
    SingleStagePolyphaseResamplerProcess(state, inBuf.data(), &inLen, outBuf.data(), &outLen);
    SingleStagePolyphaseResamplerFree(state);
}

vector<TestFuncs> g_testFuncs = {
    GetMultiplyFilterFunFuzzTest,       // 0  - random factors (Init only)
    SetResamplerFunctionCoarseFuzzTest, // 1  - coarse downsample (Init only)
    ProcessFineMonoFuzzTest,            // 2  - Fine path mono
    ProcessFineStereoFuzzTest,          // 3  - Fine path stereo
    ProcessFineMultiChFuzzTest,         // 4  - Fine path multichannel
    ProcessHalfbandMonoFuzzTest,        // 5  - Halfband mono
    ProcessHalfbandStereoFuzzTest,      // 6  - Halfband stereo
    ProcessHalfbandMultiChFuzzTest,     // 7  - Halfband multichannel
    ProcessThirdbandMonoFuzzTest,       // 8  - Thirdband mono
    ProcessThirdbandStereoFuzzTest,     // 9  - Thirdband stereo
    ProcessThirdbandMultiChFuzzTest,    // 10 - Thirdband multichannel
    ProcessCoarseUpMonoFuzzTest,        // 11 - Coarse up mono
    ProcessCoarseUpStereoFuzzTest,      // 12 - Coarse up stereo
    ProcessCoarseUpMultiChFuzzTest,     // 13 - Coarse up multichannel
    ProcessCoarseDownMonoFuzzTest,      // 14 - Coarse down mono
    ProcessCoarseDownStereoFuzzTest,    // 15 - Coarse down stereo
    ProcessCoarseDownMultiChFuzzTest,   // 16 - Coarse down multichannel
    SetRateFuzzTest,                    // 17 - SetRate (UpdateResamplerState)
    ResetMemAndSkipHalfTapsFuzzTest,    // 18 - ResetMem + SkipHalfTaps
    FreeAndNullFuzzTest,               // 19 - Free + null free
    ProcessNullInputFuzzTest,           // 20 - Process with null input
    ProcessStreamingFuzzTest,           // 21 - Multiple Process calls (streaming)
    ArgValidationFuzzTest,              // 22 - illegal-arg validation (Init/SetRate bounds)
    OverflowGuardFuzzTest,              // 23 - large factors + max quality overflow guards
};

} // namespace AudioStandard
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::AudioStandard::g_fuzzUtils.fuzzTest(data, size, OHOS::AudioStandard::g_testFuncs);
    return 0;
}
