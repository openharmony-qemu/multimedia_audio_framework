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
#ifndef PCM_MIXER_C_API_H
#define PCM_MIXER_C_API_H

// Self-contained C facade of the PCM algorithms for direct (chipset SDK) consumers.
// Keep this header dependency-free beyond stdint / audio_proresampler_process.h:
// mixer_utils.h additionally declares C++ helpers whose parameter types come from
// audio_stream_info.h, whose include closure (parcel.h, securec.h, ...) is not
// exported with this library -- consumers that only need the C API below include
// this header instead of mixer_utils.h.
#include <stdint.h>
#include "audio_proresampler_process.h"

#ifdef __cplusplus
extern "C" {
#endif

// One input stream. sampleCount is frames per channel and must be the same for all
// streams of one AudioMixerProcess call; volume is a linear gain in [0.0, 1.0]
// applied to this stream before accumulation.
// Prefixed AudioPcm* (not AudioMixStream): a bare AudioMixStream would be ambiguous
// with OHOS::AudioStandard::AudioSuite::AudioMixStream in TUs that include both headers.
struct AudioPcmMixStream {
    const void* data;
    int32_t sampleCount;
    float volume;
};

// Mixing result. outData points to an internal buffer owned by the mixer handle.
struct AudioMixResult {
    uint8_t* outData;
    int32_t outSampleCount;
    int32_t errCode; // AUDIO_PCM_ALGO_* codes from audio_proresampler_process.h
};

typedef void* AudioMixHandle;

// Create a mixer. channel: 1 = mono, 2 = stereo; the output format equals the input
// format. Constraints per AudioMixerProcess call: sampleCount > 0 and fixed after the
// first successful call; any block length works (the limiter chunks into 4 equal parts
// when possible, 3 / 2 / 1 otherwise); not longer than 60ms; stream count in [1, 1024].
AudioMixHandle AudioMixerCreate(int32_t sampleRate, int32_t channel, AudioPcmFormat format);

struct AudioMixResult AudioMixerProcess(AudioMixHandle handle, const struct AudioPcmMixStream* streams,
    int32_t streamCount);

// Destroy the mixer and release the internally allocated buffers.
void AudioMixerDestroy(AudioMixHandle handle);

// Convert interleaved mono PCM to stereo in the float domain: each input sample is
// duplicated to L/R. sampleCount is frames per channel; outData must hold
// sampleCount * 2 samples. Returns AUDIO_PCM_ALGO_* codes.
int32_t AudioPcmUpmixMonoToStereo(AudioPcmFormat format, const void* inData,
    int32_t sampleCount, void* outData);

// Convert interleaved stereo PCM to mono in the float domain: each L/R pair is
// averaged (0.5 * (L + R)). sampleCount is frames per channel; outData must hold
// sampleCount samples. Returns AUDIO_PCM_ALGO_* codes.
int32_t AudioPcmDownmixStereoToMono(AudioPcmFormat format, const void* inData,
    int32_t sampleCount, void* outData);

#ifdef __cplusplus
}
#endif
#endif
