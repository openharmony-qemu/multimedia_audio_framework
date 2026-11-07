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

#ifndef AUDIO_TIME_STRETCH_API_H
#define AUDIO_TIME_STRETCH_API_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TIME_STRETCH_SUCCESS            0
#define TIME_STRETCH_ERR_INVALID_PARAM  (-1)
#define TIME_STRETCH_ERR_INVALID_HANDLE (-2)
#define TIME_STRETCH_ERR_INVALID_SPEED  (-3)
#define TIME_STRETCH_ERR_INVALID_BUFFER (-4)
/* Reserved for ABI stability */
#define TIME_STRETCH_ERR_NO_MEMORY      (-5)
#define TIME_STRETCH_ERR_BUFFER_OVERFLOW (-6)

/* Supported speed range (stretch factor 1/speed is clamped inside [0.125, 8.0]). */
#define TIME_STRETCH_MIN_SPEED 0.125f
#define TIME_STRETCH_MAX_SPEED 8.0f

/* Supported pitch range (aligned with the framework stream pitch limits). */
#define TIME_STRETCH_MIN_PITCH 0.25f
#define TIME_STRETCH_MAX_PITCH 4.0f

typedef struct {
    bool isSupport;
    bool isRealTime;
    unsigned int frameLen;
    unsigned int inSampleRate;
    unsigned int inChannels;
    unsigned int inFormat;
    unsigned int outSampleRate;
    unsigned int outChannels;
    unsigned int outFormat;
    unsigned int algorithmDelayFrames;
    float minSpeed;
    float maxSpeed;
} AudioTimeStretchSpec;

struct TimeStretchStruct;
typedef struct TimeStretchStruct *TimeStretchHandle;

/* ==================== Streaming layer (sonic-like) ==================== */

TimeStretchHandle TimeStretchCreate(uint32_t sampleRate, uint32_t channels);
void TimeStretchDestroy(TimeStretchHandle handle);
int32_t TimeStretchSetSpeed(TimeStretchHandle handle, float speed);
int32_t TimeStretchSetPitch(TimeStretchHandle handle, float pitch);
int32_t TimeStretchWriteFloat(TimeStretchHandle handle, const float *data, int32_t numFrames);
int32_t TimeStretchReadFloat(TimeStretchHandle handle, float *data, int32_t maxFrames);
int32_t TimeStretchSamplesAvailable(TimeStretchHandle handle);
float TimeStretchGetSpeed(TimeStretchHandle handle);
float TimeStretchGetPitch(TimeStretchHandle handle);
int32_t TimeStretchReset(TimeStretchHandle handle);
int32_t TimeStretchFlush(TimeStretchHandle handle, int32_t *outFrames);

/* ==================== One-shot layer (PV-like) ==================== */

int32_t TimeStretchChangeSpeedFloat(TimeStretchHandle handle, const float *dataIn, float *dataOut,
    int32_t inCount, int32_t outCount);

/* ==================== Capability query ==================== */

AudioTimeStretchSpec TimeStretchGetSpec(TimeStretchHandle handle);

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_TIME_STRETCH_API_H */
