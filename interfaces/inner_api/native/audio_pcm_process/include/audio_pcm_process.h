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

// BitDepth CAPI
enum AudioBitDepth {
    AudioBit16Int = 16, // 16bit
    AudioBit24Int = 24, // 24bit
    AudioBit32Int = 32, // 32bit
    AudioBit32Float = 33 // 32 float
};

// channel
enum AudioChannel {
    AudioChMono = 1,
    AudioChStereo = 2
};

struct AudioFormatConfig {
    int sampleRate;
    AudioChannel channel;
    AudioBitDepth bitDepth;
};

typedef void* AudioConverterHandle;

struct AudioConvertResult {
    uint8_t* outData;
    int outSampleCount; // output sample count.
    int errCode; // 0 for success, non-0 for fail.
};

AudioConverterHandle AudioConverterCreate(const AudioFormatConfig* inCfg, const AudioFormatConfig* outCfg);

AudioConvertResult AudioConverterProcess(AudioConverterHandle handle, const uint8_t* inData, int inSampleCount);

void AudioConverterDestroy(AudioConverterHandle handle);

// mix c-apis
typedef void* AudioMixHandle;

struct AudioMixStream {
    const uint8_t* data;
    int sampleCount;
    float volume; // 0.0~1.0
};

struct AudioMixResult {
    uint8_t* outData;
    int outSampleCount;
    int errCode; // 0 for success, non-0 for fail.
};

AudioMixHandle AudioMixerCreate(const AudioFormatConfig* inCfg);

AudioMixResult AudioMixerProcess(AudioMixHandle handle, const AudioMixStream* streams, int streamCount);

void AudioMixerDestroy(AudioMixHandle hanlde);

#ifdef __cplusplus
}
#endif
#endif // AUDIO_PCM_PROCESS_H