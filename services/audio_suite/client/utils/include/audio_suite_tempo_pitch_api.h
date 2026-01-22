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

#ifndef AUDIO_SUITE_TEMPO_PITCH_API_H
#define AUDIO_SUITE_TEMPO_PITCH_API_H

#define FFT_LENGTH 1024
#define FFT_FRAME_LEN 1024
#define PV_MAX_BUFFER 200000
#ifdef __cplusplus
extern "C" {
#endif

struct PVStruct;
typedef struct PVStruct *PVParam;

typedef struct {
    bool isSupport;
    bool isRealTime;
    uint32_t frameLen;
    uint32_t inSampleRate;
    uint32_t inChannels;
    uint32_t inFormat;
    uint32_t outSampleRate;
    uint32_t outChannels;
    uint32_t outFormat;
} AudioPVSpec;

#ifdef __cplusplus
}
#endif

#endif