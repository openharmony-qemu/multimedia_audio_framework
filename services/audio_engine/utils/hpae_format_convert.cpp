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
#include "securec.h"
#include "hpae_format_convert.h"


namespace OHOS {
namespace AudioStandard {
namespace HPAE {
constexpr float FLOAT_EPS  = 1e-6f;
constexpr int OFFSET_BIT_24 = 3;
constexpr int BIT_DEPTH_TWO = 2;
constexpr int BIT_8 = 8;
constexpr int BIT_16 = 16;
constexpr int  BIT_32 = 32;

static uint32_t Read24Bit(const uint8_t *p)
{
    return ((uint32_t) p[BIT_DEPTH_TWO] << BIT_16) | ((uint32_t) p[1] << BIT_8) | ((uint32_t) p[0]);
}

static void Write24Bit(uint8_t *p, uint32_t u)
{
    p[BIT_DEPTH_TWO] = (uint8_t) (u >> BIT_16);
    p[1] = (uint8_t) (u >> BIT_8);
    p[0] = (uint8_t) u;
}

static void ConvertFromU8ToFloat(unsigned n, const uint8_t *a, float *b)
{
    for (; n > 0; n--, a++, b++) {
        *b = (float)(*a - (uint8_t)0x80U) * (1.0 / 0x80U);
    }
}

static void ConvertFrom16BitToFloat(unsigned n, const int16_t *a, float *b)
{
    for (; n > 0; n--) {
        *(b++) = *(a++) * (1.0f / (1 << (BIT_16 - 1)));
    }
}

static void ConvertFrom24BitToFloat(unsigned n, const uint8_t *a, float *b)
{
    for (; n > 0; n--) {
        int32_t s = Read24Bit(a) << BIT_8;
        *b = s * (1.0f / (1U << (BIT_32 - 1)));
        a += OFFSET_BIT_24;
        b++;
    }
}

static void ConvertFrom32BitToFloat(unsigned n, const int32_t *a, float *b)
{
    for (; n > 0; n--) {
        *(b++) = *(a++) * (1.0f / (1U << (BIT_32 - 1)));
    }
}

static float CapMax(float v)
{
    float value = v;
    if (v > 1.0f) {
        value = 1.0f - FLOAT_EPS;
    } else if (v < -1.0f) {
        value = -1.0f + FLOAT_EPS;
    }
    return value;
}

static void ConvertFromFloatToU8(unsigned n, const float *a, uint8_t *b)
{
    for (; n > 0; n--) {
        float v = *(a++);
        *(b++) = (uint8_t)(CapMax(v) * 127.0f + 128.0f);
    }
}

static void ConvertFromFloatTo16Bit(unsigned n, const float *a, int16_t *b)
{
    for (; n > 0; n--) {
        float tmp = *a++;
        float v = CapMax(tmp) * (1 << (BIT_16 - 1));
        *(b++) = (int16_t) v;
    }
}

static void ConvertFromFloatTo24Bit(unsigned n, const float *a, uint8_t *b)
{
    for (; n > 0; n--) {
        float tmp = *a++;
        float v = CapMax(tmp) * (1U << (BIT_32 - 1));
        Write24Bit(b, ((int32_t) v) >> BIT_8);
        b += OFFSET_BIT_24;
    }
}

static void ConvertFromFloatTo32Bit(unsigned n, const float *a, int32_t *b)
{
    for (; n > 0; n--) {
        float tmp = *a++;
        float v = CapMax(tmp) * (1U << (BIT_32 - 1));
        *(b++) = (int32_t) v;
    }
}

void ConvertToFloat(AudioSampleFormat format, unsigned n, void *src, float *dst)
{
    int32_t ret;
    switch (format) {
        case SAMPLE_U8:
            ConvertFromU8ToFloat(n, (const uint8_t *)src, dst);
            break;
        case SAMPLE_S16LE:
            ConvertFrom16BitToFloat(n, (const int16_t *)src, dst);
            break;
        case SAMPLE_S24LE:
            ConvertFrom24BitToFloat(n, (const uint8_t *)src, dst);
            break;
        case SAMPLE_S32LE:
            ConvertFrom32BitToFloat(n, (const int32_t *)src, dst);
            break;
        default:
            ret = memcpy_s(dst, n * sizeof(float), (const float *)src, n * sizeof(float));
            if (ret != 0) {
                float *srcFloat = (float *)src;
                for (uint32_t i = 0; i < n; i++) {
                    dst[i] = srcFloat[i];
                }
            }
            break;
    }
}

void ConvertFromFloat(AudioSampleFormat format, unsigned n, float *src, void *dst)
{
    int32_t ret;
    switch (format) {
        case SAMPLE_U8:
            ConvertFromFloatToU8(n, src, (uint8_t *)dst);
            break;
        case SAMPLE_S16LE:
            ConvertFromFloatTo16Bit(n, src, (int16_t *)dst);
            break;
        case SAMPLE_S24LE:
            ConvertFromFloatTo24Bit(n, src, (uint8_t *)dst);
            break;
        case SAMPLE_S32LE:
            ConvertFromFloatTo32Bit(n, src, (int32_t *)dst);
            break;
        default:
            ret = memcpy_s(dst, n * sizeof(float), src, n * sizeof(float));
            if (ret != 0) {
                float *dstFloat = (float *)dst;
                for (uint32_t i = 0; i < n; i++) {
                    dstFloat[i] = src[i];
                }
            }
            break;
    }
}
}}}