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
#include "audio_utils.h"
#include <algorithm>
#include <cinttypes>
#include "audio_proresampler_process.h"
#include "audio_engine_log.h"
#include "audio_proresampler.h"
#include "audio_stream_info.h"
#include "securec.h"
namespace OHOS {
namespace AudioStandard {
using namespace std;
using namespace HPAE;
static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
const static std::vector<uint32_t>  TEST_CHANNELS = {MONO, STEREO, CHANNEL_6};

constexpr uint32_t QUALITY_ONE = 1;
constexpr uint32_t FRAME_LEN_20MS = 20;
constexpr uint32_t MS_PER_SECOND = 1000;
constexpr uint32_t PROCESS_LOOP_COUNT = 10;
// Moduli used to clamp fuzz-derived channel/quality into valid ranges.
constexpr uint32_t CHANNEL_RANGE_MOD = 2; // % N + MONO -> 1 or 2 channels
constexpr uint32_t QUALITY_RANGE = 11;    // % N -> 0..10 (valid quality levels)
constexpr uint32_t QUALITY_LEVEL_TEN = 10; // top quality -> longest filter (256 taps)
// Small frame size (samples per channel) for zero-frames / zombie-process fuzz cases.
constexpr uint32_t SMALL_FRAME_SIZE = 100;
// Out-of-range args for ResampleInvalidConstructorTest (constructor bounds: rate in
// [8000, 384000], channels in [1, 16], quality <= 10 -- see audio_proresampler.cpp L42-53).
constexpr uint32_t INVALID_IN_RATE = 100;     // below SAMPLE_RATE_8000
constexpr uint32_t INVALID_OUT_RATE = 999999; // above SAMPLE_RATE_384000
constexpr uint32_t INVALID_CHANNELS = 20;     // above MAX_CHANNELS (16)
constexpr uint32_t INVALID_QUALITY = 11;      // above MAX_QUALITY (10)
// Curated rate set for ResampleProcessTest: bounded ratios (no pathological coprime
// pairs inflating the filter) while spanning coarse-up/down, halfband and fine paths.
static const std::vector<uint32_t> TEST_RATES = {
    SAMPLE_RATE_8000, SAMPLE_RATE_11025, SAMPLE_RATE_16000, SAMPLE_RATE_22050,
    SAMPLE_RATE_24000, SAMPLE_RATE_32000, SAMPLE_RATE_44100, SAMPLE_RATE_48000,
};
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

template<class T>
uint32_t GetArrLength(T& arr)
{
    if (arr == nullptr) {
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
        return 0;
    }
    return sizeof(arr) / sizeof(arr[0]);
}

// test initialize resampler, update resampler
void ResampleBaseTest()
{
    uint32_t inRate = GetData<uint32_t>();
    uint32_t outRate = GetData<uint32_t>();
    uint32_t quality = GetData<uint32_t>();
    uint32_t channels = GetData<uint32_t>();

    ProResampler resampler(inRate, outRate, channels, quality);

    inRate = GetData<uint32_t>();
    outRate = GetData<uint32_t>();
    resampler.UpdateRates(inRate, outRate);
    
    channels = GetData<uint32_t>();
    resampler.UpdateChannels(channels);
}

void ResampleProcessTest()
{
    // Pick rates from a curated set so ratios stay bounded (no pathological coprime
    // pairs inflating the filter). Size both buffers from the real sample rates so
    // Process runs its full body -- the old outFrameLen = inFrameLen / inRate * outRate
    // truncated to 0 and early-returned at the outBuffer null check.
    uint32_t inRate = TEST_RATES[GetData<uint32_t>() % TEST_RATES.size()];
    uint32_t outRate = TEST_RATES[GetData<uint32_t>() % TEST_RATES.size()];
    if (inRate == outRate) {
        return; // equal rates -> constructor rejects (state_ null) -> Process no-op; skip
    }
    uint32_t inFrameLen = inRate * FRAME_LEN_20MS / MS_PER_SECOND;
    uint32_t outFrameLen = outRate * FRAME_LEN_20MS / MS_PER_SECOND;

    for (uint32_t channels: TEST_CHANNELS) {
        ProResampler resampler(inRate, outRate, channels, QUALITY_ONE);
        // Buffer must hold inFrameLen/outFrameLen PER CHANNEL -- the C core reads
        // processInputLength * numChannels floats (audio_proresampler_process.c).
        vector<float> in(inFrameLen * channels, 0.0f);
        vector<float> out(outFrameLen * channels, 0.0f);
        for (uint32_t i = 0; i < inFrameLen * channels; i++) {
            in[i] = GetData<float>();
        }
        for (uint32_t i = 0; i < PROCESS_LOOP_COUNT; i++) {
            resampler.Process(in.data(), inFrameLen, out.data(), outFrameLen);
        }
    }
}

// === Incremental: cover ProResampler::Process full body (was L71-only) ===
// Root cause: ResampleProcessTest computes outFrameLen = inFrameLen / inRate * outRate,
// and inFrameLen / inRate truncates to 0 for every valid rate, so the out vector is
// always empty and out.data() == nullptr -> Process early-returns at the outBuffer
// null check (L71). Here both buffers are sized from the real sample rates so
// outFrameLen is non-zero and the whole body runs (state/inBuffer checks, tmpOutBuf_
// resize, core SingleStagePolyphaseResamplerProcess, fillSize memset + memcpy).
void ResampleProcessFullBodyTest()
{
    uint32_t inRate = SAMPLE_RATE_48000;
    uint32_t outRate = SAMPLE_RATE_44100;
    uint32_t channels = GetData<uint32_t>() % CHANNEL_RANGE_MOD + MONO; // 1 or 2
    ProResampler resampler(inRate, outRate, channels, QUALITY_ONE);
    uint32_t inFrameLen = inRate * FRAME_LEN_20MS / MS_PER_SECOND;   // 960
    uint32_t outFrameLen = outRate * FRAME_LEN_20MS / MS_PER_SECOND; // 882, non-zero
    vector<float> in(inFrameLen * channels, 0.0f);
    vector<float> out(outFrameLen * channels, 0.0f);
    for (uint32_t i = 0; i < inFrameLen * channels; i++) {
        in[i] = GetData<float>();
    }
    resampler.Process(in.data(), inFrameLen, out.data(), outFrameLen);
}

// === operator=(ProResampler&&) move assignment (L180-L195), incl. self-assign guard ===
void ResampleMoveAssignTest()
{
    ProResampler src(SAMPLE_RATE_48000, SAMPLE_RATE_44100, STEREO, QUALITY_ONE);
    ProResampler dst(SAMPLE_RATE_48000, SAMPLE_RATE_44100, STEREO, QUALITY_ONE);
    dst = std::move(src);     // this != &other: full move, old dst state_ freed
    ProResampler self(SAMPLE_RATE_48000, SAMPLE_RATE_44100, STEREO, QUALITY_ONE);
    self = std::move(self);   // this == &other: guard skips the body
}

// === GetQuality (L220-L223) ===
void ResampleGetQualityTest()
{
    uint32_t quality = GetData<uint32_t>() % QUALITY_RANGE; // 0..10
    ProResampler resampler(SAMPLE_RATE_48000, SAMPLE_RATE_44100, STEREO, quality);
    (void)resampler.GetQuality();
}

// === GetDelayFrames (L225-L228): before and after Process ===
void ResampleGetDelayFramesTest()
{
    ProResampler resampler(SAMPLE_RATE_48000, SAMPLE_RATE_44100, STEREO, QUALITY_ONE);
    (void)resampler.GetDelayFrames(); // lastDelayFrames_ == 0 (default-init)
    uint32_t inFrameLen = SAMPLE_RATE_48000 * FRAME_LEN_20MS / MS_PER_SECOND;
    uint32_t outFrameLen = SAMPLE_RATE_44100 * FRAME_LEN_20MS / MS_PER_SECOND;
    vector<float> in(inFrameLen * STEREO, 0.5f);
    vector<float> out(outFrameLen * STEREO, 0.0f);
    resampler.Process(in.data(), inFrameLen, out.data(), outFrameLen);
    (void)resampler.GetDelayFrames(); // lastDelayFrames_ == fillSize (set in Process)
}

// === ErrCodeToString (L236-L259): every switch case + default ===
void ResampleErrCodeToStringTest()
{
    ProResampler resampler(SAMPLE_RATE_48000, SAMPLE_RATE_44100, STEREO, QUALITY_ONE);
    (void)resampler.ErrCodeToString(RESAMPLER_ERR_SUCCESS);
    (void)resampler.ErrCodeToString(RESAMPLER_ERR_ALLOC_FAILED);
    (void)resampler.ErrCodeToString(RESAMPLER_ERR_INVALID_ARG);
    (void)resampler.ErrCodeToString(RESAMPLER_ERR_OVERFLOW);
    (void)resampler.ErrCodeToString(GetData<int32_t>()); // default: "Unknown Error Code"
}

// === gainCorrection == 1 branch in the C core PolyphaseResampler{Mono,Stereo,Multichannel}
// and inside GainCompensation (audio_proresampler_process.c) ===
// gainCorrection flips to 1 only when interpolateFactor > polyphaseFactor, where
// polyphaseFactor = MAX_LIMIT_POLYNUM / filterLength. SetRate gcd-reduces the rates, so
// the reduced interp must exceed 1280 (at quality 10, filterLength=256 -> polyphaseFactor
// = 327680/256 = 1280). 11025 -> 192000 reduces (gcd 75) to decim=147 / interp=2560, and
// 2560 > 1280 -> gainCorrection=1. (The earlier 8000->44100 ratio was wrong: gcd is 100,
// giving interp=441 < 1280, so it never triggered this path.) The else (gainCorrection==0)
// is already hit by every normal case; this closes the indexPhase = subfilterNum *
// scalerPhase branch for mono / stereo / multichannel, plus the GainCompensation loop body.
void ResampleGainCorrectionPathTest()
{
    static const uint32_t GAIN_CHANNELS[] = {MONO, STEREO, CHANNEL_6};
    for (uint32_t ch : GAIN_CHANNELS) {
        ProResampler resampler(SAMPLE_RATE_11025, SAMPLE_RATE_192000, ch, QUALITY_LEVEL_TEN);
        uint32_t inFrameLen = SAMPLE_RATE_11025 * FRAME_LEN_20MS / MS_PER_SECOND;   // 220
        uint32_t outFrameLen = SAMPLE_RATE_192000 * FRAME_LEN_20MS / MS_PER_SECOND; // 3840
        vector<float> in(inFrameLen * ch, 0.5f);
        vector<float> out(outFrameLen * ch, 0.0f);
        resampler.Process(in.data(), inFrameLen, out.data(), outFrameLen);
    }
}

// === CalculateFilter / UpdateFilterMemory realloc-else branches
// (audio_proresampler_process.c) ===
// Both helpers take the realloc path only when the filter GROWS after Init
// (filterCoefficients / inputMemory already non-NULL). Init at a small-filter ratio,
// then UpdateRates to a larger-filter ratio: 48000->24000 gives filterLength 32, then
// 48000->8000 grows it to 96, forcing both realloc branches in one transition.
void ResampleFilterReallocPathTest()
{
    uint32_t ch = GetData<uint32_t>() % CHANNEL_RANGE_MOD + MONO; // 1 or 2
    ProResampler resampler(SAMPLE_RATE_48000, SAMPLE_RATE_24000, ch, QUALITY_ONE);
    resampler.UpdateRates(SAMPLE_RATE_48000, SAMPLE_RATE_8000); // grow filter -> realloc
    uint32_t inFrameLen = SAMPLE_RATE_48000 * FRAME_LEN_20MS / MS_PER_SECOND; // 960
    uint32_t outFrameLen = SAMPLE_RATE_8000 * FRAME_LEN_20MS / MS_PER_SECOND; // 160
    vector<float> in(inFrameLen * ch, 0.5f);
    vector<float> out(outFrameLen * ch, 0.0f);
    resampler.Process(in.data(), inFrameLen, out.data(), outFrameLen);
}

// === Reset (L197-203): ResetMem + SkipHalfTaps on a valid state ===
void ResampleResetTest()
{
    ProResampler resampler(SAMPLE_RATE_48000, SAMPLE_RATE_44100, STEREO, QUALITY_ONE);
    resampler.Reset();
}

// === GetChannels (L215-218); GetQuality is already covered by ResampleGetQualityTest ===
void ResampleGetChannelsTest()
{
    uint32_t channels = GetData<uint32_t>() % CHANNEL_RANGE_MOD + MONO; // 1 or 2
    ProResampler resampler(SAMPLE_RATE_48000, SAMPLE_RATE_44100, channels, QUALITY_ONE);
    (void)resampler.GetChannels();
}

// === GetInRate / GetOutRate (L205-213) ===
void ResampleGetRatesTest()
{
    ProResampler resampler(SAMPLE_RATE_48000, SAMPLE_RATE_44100, STEREO, QUALITY_ONE);
    (void)resampler.GetInRate();
    (void)resampler.GetOutRate();
}

// === Process early-return branches (L71, L73): null inBuffer / zero outFrameSize ===
void ResampleProcessZeroFramesTest()
{
    ProResampler resampler(SAMPLE_RATE_48000, SAMPLE_RATE_44100, STEREO, QUALITY_ONE);
    uint32_t channels = resampler.GetChannels();
    vector<float> out(SMALL_FRAME_SIZE * channels);
    // null inBuffer -> L73 returns INVALID_ARG
    resampler.Process(nullptr, 0, out.data(), SMALL_FRAME_SIZE);
    // zero outFrameSize -> proceeds into SingleStagePolyphaseResamplerProcess with outLen=0
    vector<float> in(SMALL_FRAME_SIZE * channels, 0.5f);
    resampler.Process(in.data(), SMALL_FRAME_SIZE, out.data(), 0);
}

// === Multichannel + coarse-up ratio (16000->48000 reduces to decim=1, interp=3) ===
void ResampleProcessMultiChTest()
{
    ProResampler resampler(SAMPLE_RATE_16000, SAMPLE_RATE_48000, CHANNEL_6, QUALITY_ONE);
    uint32_t channels = resampler.GetChannels();
    uint32_t inFrameLen = SAMPLE_RATE_16000 * FRAME_LEN_20MS / MS_PER_SECOND;  // 320
    uint32_t outFrameLen = SAMPLE_RATE_48000 * FRAME_LEN_20MS / MS_PER_SECOND; // 960
    vector<float> in(inFrameLen * channels, 0.5f);
    vector<float> out(outFrameLen * channels, 0.0f);
    resampler.Process(in.data(), inFrameLen, out.data(), outFrameLen);
}

// === Process across quality 0..10: drives CalculateFilter with filterLength 8..256 ===
// (ResampleGetQualityTest only calls GetQuality; this runs Process at each level.)
void ResampleQualityVariationTest()
{
    uint32_t quality = GetData<uint32_t>() % QUALITY_RANGE; // 0..10
    uint32_t channels = GetData<uint32_t>() % CHANNEL_RANGE_MOD + MONO; // 1 or 2
    ProResampler resampler(SAMPLE_RATE_48000, SAMPLE_RATE_44100, channels, quality);
    uint32_t inFrameLen = SAMPLE_RATE_48000 * FRAME_LEN_20MS / MS_PER_SECOND;  // 960
    uint32_t outFrameLen = SAMPLE_RATE_44100 * FRAME_LEN_20MS / MS_PER_SECOND; // 882
    vector<float> in(inFrameLen * channels, 0.5f);
    vector<float> out(outFrameLen * channels, 0.0f);
    resampler.Process(in.data(), inFrameLen, out.data(), outFrameLen);
}

// === Constructor validation early-returns (L42-53): invalid args leave state_ nullptr ===
// Safe: state_ has a default member initializer (nullptr), so zombie objects destruct and
// process without UB. Process on a zombie hits L72 (state_ == nullptr) -> ALLOC_FAILED.
void ResampleInvalidConstructorTest()
{
    ProResampler r1(SAMPLE_RATE_48000, SAMPLE_RATE_48000, STEREO, QUALITY_ONE);  // equal rates
    ProResampler r2(INVALID_IN_RATE, SAMPLE_RATE_48000, STEREO, QUALITY_ONE);    // rate < 8000
    ProResampler r3(SAMPLE_RATE_48000, INVALID_OUT_RATE, STEREO, QUALITY_ONE);   // rate > 384000
    ProResampler r4(SAMPLE_RATE_48000, SAMPLE_RATE_24000, INVALID_CHANNELS, QUALITY_ONE); // ch > 16
    ProResampler r5(SAMPLE_RATE_48000, SAMPLE_RATE_24000, STEREO, INVALID_QUALITY);       // q > 10
    vector<float> in(SMALL_FRAME_SIZE, 0.0f);
    vector<float> out(SMALL_FRAME_SIZE, 0.0f);
    r5.Process(in.data(), SMALL_FRAME_SIZE, out.data(), SMALL_FRAME_SIZE); // zombie -> ALLOC_FAILED
}

// === UpdateRates to an invalid (equal) pair -> CheckRateAndChannelParam frees state_ ===
// (L113-117); the subsequent Process hits state_ == nullptr -> ALLOC_FAILED (L72).
void ResampleNullStateProcessTest()
{
    ProResampler resampler(SAMPLE_RATE_48000, SAMPLE_RATE_24000, STEREO, QUALITY_ONE);
    resampler.UpdateRates(SAMPLE_RATE_48000, SAMPLE_RATE_48000); // equal -> state_ freed
    uint32_t channels = resampler.GetChannels();
    vector<float> in(SMALL_FRAME_SIZE * channels, 0.0f);
    vector<float> out(SMALL_FRAME_SIZE * channels, 0.0f);
    resampler.Process(in.data(), SMALL_FRAME_SIZE, out.data(), SMALL_FRAME_SIZE); // ALLOC_FAILED
}

// === Move constructor (L171-178): steals state_, source left null ===
void ResampleMoveCtorTest()
{
    ProResampler source(SAMPLE_RATE_11025, SAMPLE_RATE_48000, STEREO, QUALITY_ONE);
    ProResampler target(std::move(source)); // target gets state_, source.state_ = null
    // move-construct from a moved-from source (state_ == null): no UB
    ProResampler second(std::move(source));
}

typedef void (*TestFuncs)();
TestFuncs g_testFuncs[] = {
    ResampleBaseTest,
    ResampleProcessTest,
    ResampleProcessFullBodyTest,   // Process full body (was L71-only)
    ResampleMoveAssignTest,        // operator=(ProResampler&&)
    ResampleGetQualityTest,        // GetQuality
    ResampleGetDelayFramesTest,    // GetDelayFrames
    ResampleErrCodeToStringTest,   // ErrCodeToString
    ResampleGainCorrectionPathTest, // C-core gainCorrection==1 branch
    ResampleFilterReallocPathTest,  // C-core CalculateFilter/UpdateFilterMemory realloc
    ResampleResetTest,             // Reset (ResetMem + SkipHalfTaps)
    ResampleGetChannelsTest,       // GetChannels
    ResampleGetRatesTest,          // GetInRate / GetOutRate
    ResampleProcessZeroFramesTest, // null inBuffer / zero outFrameSize early-return
    ResampleProcessMultiChTest,    // multichannel + coarse-up ratio (16000->48000)
    ResampleQualityVariationTest,  // Process across quality 0..10 (filterLength sweep)
    ResampleInvalidConstructorTest, // constructor validation early-returns (state_ null)
    ResampleNullStateProcessTest,  // UpdateRates->equal frees state_ -> Process ALLOC_FAILED
    ResampleMoveCtorTest,          // move constructor
};

bool FuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr) {
        return false;
    }

    // initialize data
    RAW_DATA = rawData;
    g_dataSize = size;
    g_pos = 0;

    uint32_t code = GetData<uint32_t>();
    uint32_t len = GetArrLength(g_testFuncs);
    if (len > 0) {
        g_testFuncs[code % len]();
    } else {
        AUDIO_INFO_LOG("%{public}s: The len length is equal to 0", __func__);
    }

    return true;
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::FuzzTest(data, size);
    return 0;
}
