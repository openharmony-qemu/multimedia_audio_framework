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

vector<TestFuncs> g_testFuncs = {
    AudioProresamplerProcessFuzzTest,
    AudioProresamplerUpdateRatesFuzzTest,
    AudioProresamplerUpdateChannelsFuzzTest,
    AudioProresampleroperatorFuzzTest,
    AudioProresamplerResetFuzzTest,
    AudioProresamplerGetChannelsFuzzTest,
    AudioProresamplerErrCodeToStringFuzzTest,
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