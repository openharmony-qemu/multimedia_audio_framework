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
#ifndef BITDEPTH_CONVERTER_H
#define BITDEPTH_CONVERTER_H
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
    enum {
        FMTCONV_ERR_SUCCESS = 0,
        FMTCONV_ERR_ALLOC_FAILED = 1,
        FMTCONV_ERR_INVALID_ARG = 2
    };

    /** Audio sample format */
    typedef enum AudioSampleFormat {
        SAMPLE_U8 = 0,
        SAMPLE_S16LE = 1,
        SAMPLE_S24LE = 2,
        SAMPLE_S32LE = 3,
        SAMPLE_F32LE = 4,
        INVALID_WIDTH = -1
    } AudioSampleFormat;


    typedef void (*FmtConversionFunction)(unsigned n, const void* in, const void* out);

    FmtConversionFunction GetFmtConversionU8(AudioSampleFormat fmt);
    FmtConversionFunction GetFmtConversionS16Le(AudioSampleFormat fmt);
    FmtConversionFunction GetFmtConversionS24Le(AudioSampleFormat fmt);
    FmtConversionFunction GetFmtConversionS32Le(AudioSampleFormat fmt);
    FmtConversionFunction GetFmtConversionF32Le(AudioSampleFormat fmt);

    typedef struct BitDepthConversionState BitDepthConversionState;

    struct BitDepthConversionState {
        uint32_t inputFormat;
        uint32_t outputFormat;
        uint32_t numChannels;
        uint32_t length;

        FmtConversionFunction fmtConversionProcess;
    };

    BitDepthConversionState* FmtConversionInit(uint32_t inputFormat, uint32_t outputFormat,
        uint32_t numChannels, int* err);

    void FmtConversionStateFree(BitDepthConversionState* state);

#ifdef __cplusplus
}
#endif

#endif