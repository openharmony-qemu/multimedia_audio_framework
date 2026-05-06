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

#ifndef AUDIO_PCM_PROCESS_H
#define AUDIO_PCM_PROCESS_H

#include <cstdint>
#ifdef __cplusplus
extern "C" {
#endif

enum AudioBitDepth {
    AUDIO_BIT16_INT = 16,
    AUDIO_BIT24_INT = 24,
    AUDIO_BIT32_INT = 32,
    AUDIO_BIT32_FLOAT = 33
};

enum CAudioChannel {
    AUDIO_CH_MONO = 1,
    AUDIO_CH_STEREO = 2
};

struct AudioFormatConfig {
    int sampleRate;
    CAudioChannel channel;
    AudioBitDepth bitDepth;
};

typedef void* AudioConverterHandle;

struct CAudioConvertResult {
    uint8_t* outData;
    int outSampleCount;
    int errCode;
};

AudioConverterHandle AudioConverterCreate(const AudioFormatConfig* inCfg, const AudioFormatConfig* outCfg);

CAudioConvertResult AudioConverterProcess(AudioConverterHandle handle, const uint8_t* inData, int inSampleCount);

void AudioConverterDestroy(AudioConverterHandle handle);

typedef void* AudioMixHandle;

struct CAudioMixStream {
    const uint8_t* data;
    int sampleCount;
    float volume;
};

struct AudioMixResult {
    uint8_t* outData;
    int outSamplerCount;
    int errCode;
};

AudioMixHandle AudioMixerCreate(const AudioFormatConfig* inCfg);

AudioMixResult AudioMixerProcess(AudioMixHandle handle, const CAudioMixStream* streams, int streamCount);

void AudioMixerDestroy(AudioMixHandle hanlde);

#ifdef __cplusplus
}
#endif
#endif // AUDIO_PCM_PROCESS_H