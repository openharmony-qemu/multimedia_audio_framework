/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#include "audio_utils.h"
#include "audio_proresampler.h"
#include "audio_proresampler_process.h"
#include "../fuzz_utils.h"

using namespace std;
namespace OHOS {
namespace AudioStandard {
FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
typedef void (*TestFuncs)();
const size_t FUZZ_INPUT_SIZE_THRESHOLD = 10;
namespace HPAE {
const uint32_t FRAME_SIZE = 1000 ;
const uint32_t QUARTER_FRAME_DIVISOR = 4;
// Test frame sizes (samples per channel) and quality used by the process-path fuzz cases.
const uint32_t SMALL_FRAME_SIZE = 100;
const uint32_t IN_FRAME_SIZE_480 = 480;
const uint32_t OUT_FRAME_SIZE_512 = 512;
const uint32_t IN_FRAME_SIZE_320 = 320;
const uint32_t OUT_FRAME_SIZE_1024 = 1024;
const int32_t DEFAULT_RESAMPLE_QUALITY = 5;
// Out-of-range arguments for AudioProresamplerInvalidConstructorFuzzTest.
const int32_t INVALID_IN_RATE = 100;     // below SAMPLE_RATE_8000
const int32_t INVALID_OUT_RATE = 999999; // above max supported rate
const int32_t INVALID_CHANNELS = 20;     // above max channel count
const int32_t INVALID_QUALITY = 11;      // above max quality
// Quality (0..10) and channel (1..2) variation moduli for AudioProresamplerQualityVariationFuzzTest.
const uint32_t QUALITY_RANGE_MOD = 11;
const uint32_t CHANNEL_RANGE_MOD = 2;

std::shared_ptr<ProResampler> CreateProResampler(bool force)
{
    AudioSamplingRate inRate;
    if (force) {
        inRate = SAMPLE_RATE_11025;
    } else {
        inRate = g_fuzzUtils.GetData<AudioSamplingRate>();
        inRate = (inRate < SAMPLE_RATE_8000) ? SAMPLE_RATE_8000 : inRate;
        inRate = (inRate > SAMPLE_RATE_192000) ? SAMPLE_RATE_192000 : inRate;
    }

    AudioSamplingRate outRate = g_fuzzUtils.GetData<AudioSamplingRate>();
    outRate = (outRate < SAMPLE_RATE_8000) ? SAMPLE_RATE_8000 : outRate;
    outRate = (outRate > SAMPLE_RATE_192000) ? SAMPLE_RATE_192000 : outRate;

    AudioChannel channels = g_fuzzUtils.GetData<AudioChannel>();
    channels = (channels < MONO) ? MONO : channels;
    channels = (channels > CHANNEL_8) ? CHANNEL_8 : channels;

    uint32_t quality = 1;
    return std::make_shared<ProResampler>(inRate, outRate, channels, quality);
}

void AudioProresamplerProcessFuzzTest()
{
    auto proResampler = CreateProResampler(g_fuzzUtils.GetData<bool>());
    if (proResampler == nullptr) {
        return;
    }

    uint32_t inFrameSize = g_fuzzUtils.GetData<uint32_t>() % FRAME_SIZE;
    uint32_t outFrameSize = g_fuzzUtils.GetData<uint32_t>() % FRAME_SIZE;
    uint32_t channels = proResampler->GetChannels();

    std::vector<float> inBuffer(inFrameSize * channels);
    for (auto& val : inBuffer)  {
        val = g_fuzzUtils.GetData<float>();
    }
    std::vector<float> outBuffer(outFrameSize * channels);

    proResampler->Process(inFrameSize > 0 ? inBuffer.data() : nullptr, inFrameSize, outBuffer.data(), outFrameSize);
}

void AudioProresamplerUpdateRatesFuzzTest()
{
    auto proResampler = CreateProResampler(g_fuzzUtils.GetData<bool>());
    if (proResampler == nullptr) {
        return;
    }

    AudioSamplingRate newInRate = g_fuzzUtils.GetData<AudioSamplingRate>();
    AudioSamplingRate newOutRate = g_fuzzUtils.GetData<AudioSamplingRate>();

    proResampler->UpdateRates(newInRate, newOutRate);
}

void AudioProresamplerUpdateChannelsFuzzTest()
{
    bool ret = g_fuzzUtils.GetData<bool>();
    auto proResampler = CreateProResampler(ret);
    if (proResampler == nullptr) {
        return;
    }

    uint32_t currentchannels = proResampler->GetChannels();
    proResampler->UpdateChannels(currentchannels);
}

void AudioProresampleroperatorFuzzTest()
{
    ProResampler source(SAMPLE_RATE_11025, SAMPLE_RATE_48000, STEREO, 1);
    ProResampler target(std::move(source));
    ProResampler movedProResampler = std::move(source);
}

void AudioProresamplerResetFuzzTest()
{
    auto proResampler = CreateProResampler(g_fuzzUtils.GetData<bool>());
    if (proResampler == nullptr) {
        return;
    }

    proResampler->Reset();
}

void AudioProresamplerGetChannelsFuzzTest()
{
    auto proResampler = CreateProResampler(g_fuzzUtils.GetData<bool>());
    if (proResampler == nullptr) {
        return;
    }

    proResampler->GetChannels();
    proResampler->GetQuality();
}

void AudioProresamplerErrCodeToStringFuzzTest()
{
    auto proResampler = CreateProResampler(g_fuzzUtils.GetData<bool>());
    if (proResampler == nullptr) {
        return;
    }

    int32_t errCode = g_fuzzUtils.GetData<int32_t>();
    proResampler->ErrCodeToString(errCode);
}

void AudioProresamplerProcessZeroFramesFuzzTest()
{
    auto proResampler = CreateProResampler(true);
    if (proResampler == nullptr) {
        return;
    }
    uint32_t channels = proResampler->GetChannels();
    std::vector<float> outBuffer(SMALL_FRAME_SIZE * channels);
    // Process with zero input frames (null inBuffer)
    proResampler->Process(nullptr, 0, outBuffer.data(), SMALL_FRAME_SIZE);
    // Process with zero output frames
    std::vector<float> inBuffer(SMALL_FRAME_SIZE * channels, 0.5f);
    proResampler->Process(inBuffer.data(), SMALL_FRAME_SIZE, outBuffer.data(), 0);
}

void AudioProresamplerProcessMonoFuzzTest()
{
    AudioSamplingRate inRate = SAMPLE_RATE_48000;
    AudioSamplingRate outRate = SAMPLE_RATE_44100;
    auto proResampler = std::make_shared<ProResampler>(inRate, outRate, MONO, 1);
    if (proResampler == nullptr) {
        return;
    }
    uint32_t channels = proResampler->GetChannels();
    uint32_t inFrameSize = IN_FRAME_SIZE_480;
    uint32_t outFrameSize = OUT_FRAME_SIZE_512;
    std::vector<float> inBuffer(inFrameSize * channels, 0.5f);
    std::vector<float> outBuffer(outFrameSize * channels, 0.0f);
    proResampler->Process(inBuffer.data(), inFrameSize, outBuffer.data(), outFrameSize);
}

void AudioProresamplerProcessMultiChFuzzTest()
{
    AudioSamplingRate inRate = SAMPLE_RATE_16000;
    AudioSamplingRate outRate = SAMPLE_RATE_48000;
    auto proResampler = std::make_shared<ProResampler>(inRate, outRate, CHANNEL_6, 1);
    if (proResampler == nullptr) {
        return;
    }
    uint32_t channels = proResampler->GetChannels();
    uint32_t inFrameSize = IN_FRAME_SIZE_320;
    uint32_t outFrameSize = OUT_FRAME_SIZE_1024;
    std::vector<float> inBuffer(inFrameSize * channels, 0.5f);
    std::vector<float> outBuffer(outFrameSize * channels, 0.0f);
    proResampler->Process(inBuffer.data(), inFrameSize, outBuffer.data(), outFrameSize);
}

// === Incremental supplement ===
// QUALITY_TABLE entry / filterLength path in the C core is exercised.
void AudioProresamplerQualityVariationFuzzTest()
{
    uint32_t quality = g_fuzzUtils.GetData<uint32_t>() % QUALITY_RANGE_MOD; // 0..10
    uint32_t channels = (g_fuzzUtils.GetData<uint32_t>() % CHANNEL_RANGE_MOD) + 1; // 1 or 2
    ProResampler resampler(SAMPLE_RATE_48000, SAMPLE_RATE_44100, channels, quality);
    uint32_t inFrameSize = FRAME_SIZE / QUARTER_FRAME_DIVISOR;
    uint32_t outFrameSize = FRAME_SIZE / QUARTER_FRAME_DIVISOR;
    std::vector<float> inBuffer(inFrameSize * channels, 0.5f);
    std::vector<float> outBuffer(outFrameSize * channels, 0.0f);
    resampler.Process(inBuffer.data(), inFrameSize, outBuffer.data(), outFrameSize);
}

// Getters not exercised by the existing GetChannelsFuzzTest.
void AudioProresamplerRemainingGettersFuzzTest()
{
    auto proResampler = CreateProResampler(g_fuzzUtils.GetData<bool>());
    if (proResampler == nullptr) {
        return;
    }
    proResampler->GetInRate();
    proResampler->GetOutRate();
}

// Constructor validation early-return paths. Safe: state_ has a default member
// initializer (nullptr), so zombie objects destruct/process without UB.
void AudioProresamplerInvalidConstructorFuzzTest()
{
    ProResampler r1(SAMPLE_RATE_48000, SAMPLE_RATE_48000, STEREO, DEFAULT_RESAMPLE_QUALITY); // inRate == outRate
    ProResampler r2(INVALID_IN_RATE, SAMPLE_RATE_48000, STEREO, DEFAULT_RESAMPLE_QUALITY);
    ProResampler r3(SAMPLE_RATE_48000, INVALID_OUT_RATE, STEREO, DEFAULT_RESAMPLE_QUALITY);
    ProResampler r4(SAMPLE_RATE_48000, SAMPLE_RATE_24000, INVALID_CHANNELS, DEFAULT_RESAMPLE_QUALITY);
    ProResampler r5(SAMPLE_RATE_48000, SAMPLE_RATE_24000, STEREO, INVALID_QUALITY);
    // Process on a zombie (state_ == nullptr) -> returns ALLOC_FAILED, no crash
    std::vector<float> inBuffer(FRAME_SIZE / QUARTER_FRAME_DIVISOR, 0.0f);
    std::vector<float> outBuffer(FRAME_SIZE / QUARTER_FRAME_DIVISOR, 0.0f);
    r5.Process(inBuffer.data(), FRAME_SIZE / QUARTER_FRAME_DIVISOR, outBuffer.data(),
        FRAME_SIZE / QUARTER_FRAME_DIVISOR);
}

// UpdateRates to an invalid rate pair -> CheckRateAndChannelParam frees state_ ->
// subsequent Process hits the state_ == nullptr (ALLOC_FAILED) branch.
void AudioProresamplerNullStateProcessFuzzTest()
{
    ProResampler resampler(SAMPLE_RATE_48000, SAMPLE_RATE_24000, STEREO, DEFAULT_RESAMPLE_QUALITY);
    resampler.UpdateRates(SAMPLE_RATE_48000, SAMPLE_RATE_48000); // invalid (equal) -> state_ freed
    std::vector<float> inBuffer(FRAME_SIZE / QUARTER_FRAME_DIVISOR * STEREO, 0.0f);
    std::vector<float> outBuffer(FRAME_SIZE / QUARTER_FRAME_DIVISOR * STEREO, 0.0f);
    resampler.Process(inBuffer.data(), FRAME_SIZE / QUARTER_FRAME_DIVISOR, outBuffer.data(),
        FRAME_SIZE / QUARTER_FRAME_DIVISOR); // expect ALLOC_FAILED
}

vector<TestFuncs> g_testFuncs = {
    AudioProresamplerProcessFuzzTest,
    AudioProresamplerUpdateRatesFuzzTest,
    AudioProresamplerUpdateChannelsFuzzTest,
    AudioProresampleroperatorFuzzTest,
    AudioProresamplerResetFuzzTest,
    AudioProresamplerGetChannelsFuzzTest,
    AudioProresamplerErrCodeToStringFuzzTest,
    AudioProresamplerProcessZeroFramesFuzzTest,
    AudioProresamplerProcessMonoFuzzTest,
    AudioProresamplerProcessMultiChFuzzTest,
    AudioProresamplerQualityVariationFuzzTest,
    AudioProresamplerRemainingGettersFuzzTest,
    AudioProresamplerInvalidConstructorFuzzTest,
    AudioProresamplerNullStateProcessFuzzTest,
};
} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::AudioStandard::FUZZ_INPUT_SIZE_THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::g_fuzzUtils.fuzzTest(data, size, OHOS::AudioStandard::HPAE::g_testFuncs);
    return 0;
}