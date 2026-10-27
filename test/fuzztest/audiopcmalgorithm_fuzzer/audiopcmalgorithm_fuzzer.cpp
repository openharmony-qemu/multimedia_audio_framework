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

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>
#include "audio_proresampler_process.h"
#include "mixer_utils.h"
#include "securec.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;

// Curated format set: the four supported values plus one invalid value.
static const vector<int32_t> TEST_FORMATS = {16, 24, 32, 33, 20};
// Curated rate set inside the mixer gate [8000, 384000] plus invalid bounds.
static const vector<int32_t> TEST_RATES = {8000, 16000, 44100, 48000, 96000, 4000, 400000};
// Curated channel set: supported 1/2 plus invalid 0/3.
static const vector<int32_t> TEST_CHANNELS = {1, 2, 0, 3};
// Cap fuzz-derived sample counts so buffers stay bounded (20ms@48k stereo = 960).
constexpr int32_t MAX_FUZZ_SAMPLE_COUNT = 2000;
constexpr int32_t MAX_FUZZ_STREAM_COUNT = 8;
constexpr int32_t MIXER_LOOP_COUNT = 4;
constexpr int64_t MAX_FUZZ_BUFFER_BYTES = 4096 * 1024; // keep fuzz allocations bounded
constexpr int32_t FUZZ_BRANCH_MOD = 2; // splits fuzz inputs between two branches
constexpr int32_t MIXER_MAX_BLOCK_MS = 60; // process contract: block no longer than 60ms
constexpr int32_t MS_PER_SECOND = 1000;
constexpr int32_t MIXER_ALIGN_DIVISOR = 4; // steers toward the 4-chunk limiter path; other
                                            // block lengths fall back to 3 / 2 / 1 chunks
constexpr int32_t MIXER_CHANNEL_MONO_FUZZ = 1;
constexpr int32_t MIXER_CHANNEL_STEREO_FUZZ = 2;

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

// Fill dst with fuzz bytes (cyclically when the input is shorter than dst).
static void FillBuffer(uint8_t *dst, size_t dstSize)
{
    if (dst == nullptr || dstSize == 0 || RAW_DATA == nullptr || g_dataSize == 0) {
        return;
    }
    size_t avail = g_dataSize - (g_pos < g_dataSize ? g_pos : g_dataSize);
    if (avail == 0) {
        return;
    }
    for (size_t i = 0; i < dstSize; ++i) {
        dst[i] = RAW_DATA[g_pos + (i % avail)];
    }
}

static void ConvertFuzz(const uint8_t *data, size_t size)
{
    int32_t fmt = TEST_FORMATS[GetData<uint32_t>() % TEST_FORMATS.size()];
    int32_t channels = TEST_CHANNELS[GetData<uint32_t>() % TEST_CHANNELS.size()];
    int32_t sampleCount = static_cast<int32_t>(GetData<uint32_t>() % MAX_FUZZ_SAMPLE_COUNT);
    AudioPcmFormat format = static_cast<AudioPcmFormat>(fmt);
    int32_t bytesPerSample = AudioFormatCalcMemLength(format, 1, 1); // 1 frame * 1 channel = bytes per sample
    if (bytesPerSample <= 0 || channels <= 0 || sampleCount <= 0) {
        (void)ConvertPcmToFloat(format, data, sampleCount, nullptr);
        (void)AudioFormatCalcMemLength(format, channels, sampleCount);
        return;
    }
    int64_t byteLen = static_cast<int64_t>(sampleCount) * bytesPerSample;
    if (byteLen > MAX_FUZZ_BUFFER_BYTES) {
        return;
    }
    vector<uint8_t> pcm(static_cast<size_t>(byteLen), 0);
    FillBuffer(pcm.data(), pcm.size());
    vector<float> floats(static_cast<size_t>(sampleCount), 0.0f);
    int32_t ret = ConvertPcmToFloat(format, pcm.data(), sampleCount, floats.data());
    if (ret != AUDIO_PCM_ALGO_SUCCESS) {
        return;
    }
    vector<uint8_t> back(static_cast<size_t>(byteLen), 0);
    (void)ConvertFloatToPcm(format, floats.data(), sampleCount, back.data());
    (void)AudioFormatCalcMemLength(format, channels, sampleCount);

    // mono <-> stereo conversion: exercise both directions plus the reject path.
    // pcm holds sampleCount interleaved samples, so the stereo source has only
    // sampleCount / channels frames available.
    int32_t frameCount = sampleCount / channels;
    if (frameCount > 0) {
        int32_t dstChannels = (channels == 1) ? MIXER_CHANNEL_STEREO_FUZZ : MIXER_CHANNEL_MONO_FUZZ;
        int32_t outSamples = frameCount * dstChannels;
        vector<uint8_t> converted(static_cast<size_t>(outSamples) * bytesPerSample, 0);
        (void)AudioPcmConvertChannels(format, channels, dstChannels, pcm.data(), frameCount,
            converted.data());
        (void)AudioPcmConvertChannels(format, channels, channels, pcm.data(), frameCount,
            converted.data()); // same channels must be rejected
    }
}

// Prepares fuzz streams and drives AudioMixerProcess; extracted to keep the nesting
// of MixerFuzz within the codecheck limit. Consumes fuzz data in a fixed order.
static void MixerProcessFuzz(AudioMixHandle handle, int32_t rate, int32_t channels,
    int32_t bytesPerSample, int32_t sampleCount)
{
    if (bytesPerSample <= 0 || channels <= 0) {
        return;
    }
    // half of the cases align to the process contract to reach the mix+limiter path
    if (GetData<uint8_t>() % FUZZ_BRANCH_MOD == 0) {
        int32_t maxCount = rate * MIXER_MAX_BLOCK_MS / MS_PER_SECOND;
        if (sampleCount > maxCount) {
            sampleCount = maxCount;
        }
        // round down to a multiple of 4; channels is 1 or 2 here (mixer gate), so
        // sampleCount * channels stays divisible by 4 without further adjustment
        sampleCount = (sampleCount / MIXER_ALIGN_DIVISOR) * MIXER_ALIGN_DIVISOR;
    }
    int64_t byteLen = static_cast<int64_t>(sampleCount) * channels * bytesPerSample;
    if (sampleCount <= 0 || byteLen > MAX_FUZZ_BUFFER_BYTES) {
        return;
    }
    vector<uint8_t> pcm(static_cast<size_t>(byteLen), 0);
    FillBuffer(pcm.data(), pcm.size());
    int32_t streamCount = static_cast<int32_t>(GetData<uint8_t>() % MAX_FUZZ_STREAM_COUNT) + 1;
    vector<AudioPcmMixStream> streams(static_cast<size_t>(streamCount));
    for (int32_t i = 0; i < streamCount; ++i) {
        streams[i].data = pcm.data();
        streams[i].sampleCount = sampleCount;
        streams[i].volume = GetData<float>();
    }
    for (int32_t i = 0; i < MIXER_LOOP_COUNT; ++i) {
        struct AudioMixResult result = AudioMixerProcess(handle, streams.data(), streamCount);
        if (result.errCode != AUDIO_PCM_ALGO_SUCCESS) {
            break;
        }
    }
}

static void MixerFuzz(const uint8_t *data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }
    int32_t rate = TEST_RATES[GetData<uint32_t>() % TEST_RATES.size()];
    int32_t channels = TEST_CHANNELS[GetData<uint32_t>() % TEST_CHANNELS.size()];
    AudioPcmFormat format = static_cast<AudioPcmFormat>(TEST_FORMATS[GetData<uint32_t>() % TEST_FORMATS.size()]);

    AudioMixHandle handle = AudioMixerCreate(rate, channels, format);
    if (handle == nullptr) {
        return;
    }

    int32_t bytesPerSample = AudioFormatCalcMemLength(format, 1, 1); // 1 frame * 1 channel = bytes per sample
    int32_t sampleCount = static_cast<int32_t>(GetData<uint32_t>() % MAX_FUZZ_SAMPLE_COUNT);
    MixerProcessFuzz(handle, rate, channels, bytesPerSample, sampleCount);
    (void)AudioMixerProcess(handle, nullptr, 1);
    AudioMixerDestroy(handle);
    AudioMixerDestroy(nullptr);
}

void FuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr || size < sizeof(uint32_t)) {
        return;
    }
    RAW_DATA = data;
    g_dataSize = size;
    g_pos = 0;
    uint32_t select = GetData<uint32_t>();
    switch (select % FUZZ_BRANCH_MOD) {
        case 0:
            ConvertFuzz(data, size);
            break;
        default:
            MixerFuzz(data, size);
            break;
    }
}
} // AudioStandard
} // OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::AudioStandard::FuzzTest(data, size);
    return 0;
}
