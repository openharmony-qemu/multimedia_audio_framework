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
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <stdint.h>
#include "bitdepth_converter.h"

#ifndef NULL
#define NULL 0
#endif

#define INDEX_TWO 2
#define INDEX_THREE 3
#define SHIFTS_8BIT 8
#define SHIFTS_16BIT 16
#define SHIFTS_24BIT 24

template <typename T>
inline T CLAMP(T x, T low, T high)
{
    return ((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x));
}

static inline uint32_t READ24LE(const uint8_t *p)
{
    return ((uint32_t)p[INDEX_TWO] << SHIFTS_16BIT) |
        ((uint32_t)p[1] << SHIFTS_8BIT) |
        ((uint32_t)p[0]);
}

static inline void WRITE24LE(uint8_t *p, uint32_t u)
{
    p[INDEX_TWO] = (uint8_t)(u >> SHIFTS_16BIT);
    p[1] = (uint8_t)(u >> SHIFTS_8BIT);
    p[0] = (uint8_t)u;
}

// SAMPLE_U8
// U8ToS16Le
static void U8ToS16Le(unsigned n, const uint8_t* a, int16_t* b)
{
    for (; n > 0; n--, a++, b++) {
        *b = (int16_t)(*a - (uint8_t)0x80U) << SHIFTS_8BIT;
    }
}
// U8ToS24Le
static void U8ToS24Le(unsigned n, const uint8_t* a, uint8_t* b)
{
    for (; n > 0; n--) {
        WRITE24LE(b, (uint32_t)(*a - (uint8_t)0x80U) << SHIFTS_16BIT);
        a++;
        b += INDEX_THREE;
    }
}
// U8ToS32Le
static void U8ToS32Le(unsigned n, const uint8_t* a, int32_t* b)
{
    for (; n > 0; n--, a++, b++) {
        *b = (int32_t)(*a - (uint8_t)0x80U) << SHIFTS_24BIT;
    }
}
// U8ToF32Le
static void U8ToF32Le(unsigned n, const uint8_t* a, float* b)
{
    for (; n > 0; n--, a++, b++) {
        *b = (float)(*a - (uint8_t)0x80U) * (1.0 / 0x80U);
    }
}

// SAMPLE_S16LE
// S16LeToU8
static void S16LeToU8(unsigned n, const int16_t* a, uint8_t* b)
{
    for (; n > 0; n--, a++, b++) {
        *b = (uint8_t)((uint16_t)(*a) >> SHIFTS_8BIT) + (uint8_t)0x80U;
    }
}
// S16LeToS24Le
static void S16LeToS24Le(unsigned n, const int16_t* a, uint8_t* b)
{
    for (; n > 0; n--) {
        WRITE24LE(b, ((uint32_t)*a) << SHIFTS_8BIT);
        a++;
        b += INDEX_THREE;
    }
}
// S16LeToS32Le
static void S16LeToS32Le(unsigned n, const int16_t* a, int32_t* b)
{
    for (; n > 0; n--, a++, b++) {
        *b = ((int32_t)*a) << SHIFTS_16BIT;
    }
}
// S16LeToF32Le
static void S16LeToF32Le(unsigned n, const int16_t* a, float* b)
{
    for (; n > 0; n--) {
        *(b++) = *(a++) * (1.0f / (1 << 0x0F));
    }
}

// SAMPLE_S24LE
// S24LeToU8
static void S24LeToU8(unsigned n, const uint8_t* a, uint8_t* b)
{
    for (; n > 0; n--) {
        *b = (uint8_t)(READ24LE(a) >> SHIFTS_16BIT) + (uint8_t)0x80U;
        a += INDEX_THREE;
        b++;
    }
}
// S24LeToS16Le
static void S24LeToS16Le(unsigned n, const uint8_t* a, int16_t* b)
{
    for (; n > 0; n--) {
        *b = (int16_t)(READ24LE(a) >> SHIFTS_8BIT);
        a += INDEX_THREE;
        b++;
    }
}
// S24LeToS32Le
static void S24LeToS32Le(unsigned n, const uint8_t* a, int32_t* b)
{
    for (; n > 0; n--) {
        *b = (int32_t)(READ24LE(a) << SHIFTS_8BIT);
        a += INDEX_THREE;
        b++;
    }
}
// S24LeToF32Le
static void S24LeToF32Le(unsigned n, const uint8_t* a, float* b)
{
    for (; n > 0; n--) {
        int32_t s = (int32_t)READ24LE(a) << SHIFTS_8BIT;
        *b = s * (1.0f / (1U << 0x1F));
        a += INDEX_THREE;
        b++;
    }
}

// SAMPLE_S32LE
// S32LeToU8
static void S32LeToU8(unsigned n, const int32_t* a, uint8_t* b)
{
    for (; n > 0; n--, a++, b++) {
        *b = (uint8_t)((int32_t)(*a) >> SHIFTS_24BIT) + (uint8_t)0x80U;
    }
}
// S32LeToS16Le
static void S32LeToS16Le(unsigned n, const int32_t* a, int16_t* b)
{
    for (; n > 0; n--, a++, b++) {
        *b = (int16_t)((int32_t)(*a) >> SHIFTS_16BIT);
    }
}
// S32LeToS24Le
static void S32LeToS24Le(unsigned n, const int32_t* a, uint8_t* b)
{
    for (; n > 0; n--) {
        WRITE24LE(b, ((uint32_t)*a) >> SHIFTS_8BIT);
        a++;
        b += INDEX_THREE;
    }
}
// S32LeToF32Le
static void S32LeToF32Le(unsigned n, const int32_t* a, float* b)
{
    for (; n > 0; n--) {
        *(b++) = *(a++) * (1.0f / (1U << 0x1F));
    }
}

// SAMPLE_F32LE
// F32LeToU8
static void F32LeToU8(unsigned n, const float* a, uint8_t* b)
{
    for (; n > 0; n--) {
        float v = *(a++);
        *(b++) = (uint8_t)(CLAMP(v, -1.0, 1.0) * 127.0f + 128.0f);
    }
}
// F32LeToS16Le
static void F32LeToS16Le(unsigned n, const float* a, int16_t* b)
{
    for (; n > 0; n--) {
        float v = *(a++) * (1 << 15);
        *(b++) = (int16_t)CLAMP((int32_t)v, -0x8000, 0x7FFF);
    }
}
// F32LeToS24Le
static void F32LeToS24Le(unsigned n, const float* a, uint8_t* b)
{
    for (; n > 0; n--) {
        float v = *(a++) * (1 << 23);
        WRITE24LE(b, (uint32_t)CLAMP((int32_t)v, -0x800000LL, 0x7FFFFFLL));
        b += INDEX_THREE;
    }
}
// F32LeToS32Le
static void F32LeToS32Le(unsigned n, const float* a, int32_t* b)
{
    for (; n > 0; n--) {
        float v = *(a++) * (1U << 31);
        *(b++) = (int32_t)CLAMP((int64_t)v, -0x80000000LL, 0x7FFFFFFFLL);
    }
}

// function table
static FmtConversionFunction g_sampleU8Table[] = {
    [SAMPLE_S16LE] = (FmtConversionFunction)S16LeToU8,
    [SAMPLE_S24LE] = (FmtConversionFunction)S24LeToU8,
    [SAMPLE_S32LE] = (FmtConversionFunction)S32LeToU8,
    [SAMPLE_F32LE] = (FmtConversionFunction)F32LeToU8,
};

static FmtConversionFunction g_sampleS16leTable[] = {
    [SAMPLE_U8] = (FmtConversionFunction)U8ToS16Le,
    [SAMPLE_S24LE] = (FmtConversionFunction)S24LeToS16Le,
    [SAMPLE_S32LE] = (FmtConversionFunction)S32LeToS16Le,
    [SAMPLE_F32LE] = (FmtConversionFunction)F32LeToS16Le,
};

static FmtConversionFunction g_sampleS24leTable[] = {
    [SAMPLE_U8] = (FmtConversionFunction)U8ToS24Le,
    [SAMPLE_S16LE] = (FmtConversionFunction)S16LeToS24Le,
    [SAMPLE_S32LE] = (FmtConversionFunction)S32LeToS24Le,
    [SAMPLE_F32LE] = (FmtConversionFunction)F32LeToS24Le,
};

static FmtConversionFunction g_sampleS32leTable[] = {
    [SAMPLE_U8] = (FmtConversionFunction)U8ToS32Le,
    [SAMPLE_S16LE] = (FmtConversionFunction)S16LeToS32Le,
    [SAMPLE_S24LE] = (FmtConversionFunction)S24LeToS32Le,
    [SAMPLE_F32LE] = (FmtConversionFunction)F32LeToS32Le,
};

static FmtConversionFunction g_sampleF32leTable[] = {
    [SAMPLE_U8] = (FmtConversionFunction)U8ToF32Le,
    [SAMPLE_S16LE] = (FmtConversionFunction)S16LeToF32Le,
    [SAMPLE_S24LE] = (FmtConversionFunction)S24LeToF32Le,
    [SAMPLE_S32LE] = (FmtConversionFunction)S32LeToF32Le,
};

// choose function from function table
FmtConversionFunction GetFmtConversionU8(AudioSampleFormat fmt)
{
    return g_sampleU8Table[fmt];
}

FmtConversionFunction GetFmtConversionS16Le(AudioSampleFormat fmt)
{
    return g_sampleS16leTable[fmt];
}

FmtConversionFunction GetFmtConversionS24Le(AudioSampleFormat fmt)
{
    return g_sampleS24leTable[fmt];
}

FmtConversionFunction GetFmtConversionS32Le(AudioSampleFormat fmt)
{
    return g_sampleS32leTable[fmt];
}

FmtConversionFunction GetFmtConversionF32Le(AudioSampleFormat fmt)
{
    return g_sampleF32leTable[fmt];
}

// BitDepthConversion implementation
BitDepthConversionState* FmtConversionInit(uint32_t inputFormat, uint32_t outputFormat,
    uint32_t numChannels, int* err)
{
    BitDepthConversionState* state;

    if (numChannels == 0 || inputFormat == outputFormat) {
        if (err) {
            *err = FMTCONV_ERR_INVALID_ARG;
        }
        return NULL;
    }
    state = (BitDepthConversionState*)calloc(sizeof(BitDepthConversionState), 1);
    if (!state) {
        if (err) {
            *err = FMTCONV_ERR_ALLOC_FAILED;
        }
        return NULL;
    }
    state->inputFormat = inputFormat;
    state->outputFormat = outputFormat;
    state->numChannels = numChannels;

    if (inputFormat != outputFormat) {
        switch (outputFormat) {
            case SAMPLE_U8:
                state->fmtConversionProcess = GetFmtConversionU8(inputFormat);
                break;
            case SAMPLE_S16LE:
                state->fmtConversionProcess = GetFmtConversionS16Le(inputFormat);
                break;
            case SAMPLE_S24LE:
                state->fmtConversionProcess = GetFmtConversionS24Le(inputFormat);
                break;
            case SAMPLE_S32LE:
                state->fmtConversionProcess = GetFmtConversionS32Le(inputFormat);
                break;
            case SAMPLE_F32LE:
            default:
                state->fmtConversionProcess = GetFmtConversionF32Le(inputFormat);
                break;
        }
    }

    return state;
}
void FmtConversionStateFree(BitDepthConversionState* state)
{
    free(state);
    state = NULL;
}