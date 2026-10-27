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
#ifndef MIXER_UTILS_H
#define MIXER_UTILS_H
#include <stdint.h>
#include <vector>
#include "audio_proresampler_process.h"
#include "audio_stream_info.h"
namespace OHOS {
namespace AudioStandard {
namespace HPAE {
constexpr uint32_t MAX_CHANNELS = 16;
// max framelength is sample rate 192000, 10s
constexpr uint32_t MAX_CHANNEL_CONVERT_FRAME_LENGTH = SAMPLE_RATE_192000 * 10;
constexpr float COEF_ZERO_F = 0.0f;
constexpr float COEF_0DB_F = 1.0f;
constexpr float COEF_M3DB_F = 0.7071f;
constexpr float COEF_M6DB_F = 0.5f;
constexpr float COEF_M435DB_F = 0.6057f;
constexpr float COEF_M45DB_F = 0.5946f;
constexpr float COEF_M9DB_F = 0.3544f;
constexpr float COEF_M899DB_F = 0.3552f;
constexpr float COEF_M12DB_F = 0.2509f;

enum {
    MIX_ERR_SUCCESS = 0,
    MIX_ERR_ALLOC_FAILED = -1,
    MIX_ERR_INVALID_ARG = -2
};

// used for setting up general mixing table for downmixing and upmixing
int32_t SetUpGeneralMixingTable(float (&coeffTable)[MAX_CHANNELS][MAX_CHANNELS], AudioChannelInfo inChannelInfo,
    AudioChannelInfo outChannelInfo, bool mixLfe);

bool SetDefaultChannelLayout(AudioChannel channels, AudioChannelLayout &channelLayout);

bool IsValidChLayout(AudioChannelLayout &chLayout, uint32_t chCounts);

uint32_t BitCounts(uint64_t bits);

bool CheckIsHOA(AudioChannelLayout layout);
} // HPAE
} // AudioStandard
} // OHOS

// C-linkage mixer facade for C++ callers (e.g. chipset SDK consumers): mixes equal-length
// same-format streams in the float domain with a limiter (anti-overflow), and returns
// an internally allocated buffer which stays valid until the next call or Destroy.
// Note: this header is not includable from a pure C translation unit (C++ constructs above);
// include it at file scope and do not wrap this include in an extern "C" block.
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

// Convert interleaved mono <-> stereo PCM in the float domain: mono duplicates each
// sample to L/R; stereo averages each L/R pair. srcChannels/dstChannels: 1 or 2 and
// must differ. sampleCount is frames per channel; outData must hold
// sampleCount * dstChannels samples. Returns AUDIO_PCM_ALGO_* codes.
int32_t AudioPcmConvertChannels(AudioPcmFormat format, int32_t srcChannels, int32_t dstChannels,
    const void* inData, int32_t sampleCount, void* outData);

#ifdef __cplusplus
}
#endif
#endif