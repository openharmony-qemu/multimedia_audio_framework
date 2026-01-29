/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

#ifndef AUDIOEDITTESTAPP_UTILS_H
#define AUDIOEDITTESTAPP_UTILS_H

#include <string>
#include "audioSuiteError/AudioSuiteError.h"
#include "napi/native_api.h"
#include "ohaudiosuite/native_audio_suite_base.h"
#include "ohaudio/native_audiostream_base.h"

extern const char* STR_0;
extern const char* STR_1;
extern const char* STR_2;
extern const char* STR_3;
extern const char* STR_4;
extern const char* STR_5;
extern const char* STR_6;

enum {
    ARG_0 = 0,
    ARG_1 = 1,
    ARG_2 = 2,
    ARG_3 = 3,
    ARG_4 = 4,
    ARG_5 = 5,
    ARG_6 = 6,
    ARG_7 = 7,
    ARG_8 = 8,
};

enum DemoBitsPerSample {
    DEMO_SAMPLE_U8 = 0,
    DEMO_SAMPLE_S16LE = 1,
    DEMO_SAMPLE_S246E = 2,
    DEMO_SAMPLE_S32LE = 3,
    DEMO_SAMPLE_F32LE = 4,

    DEMO_BITSPERSAMPLE_8 = 8,
    DEMO_BITSPERSAMPLE_16 = 16,
    DEMO_BITSPERSAMPLE_24 = 24,
    DEMO_BITSPERSAMPLE_32 = 32,
};

enum DemoChannels {
    DEMO_CHANNELS_1 = 1,
    DEMO_CHANNELS_2 = 2,
};

struct FieldEffectParams {
    std::string inputId;
    unsigned int mode = 0;
    std::string fieldEffectId;
    std::string selectedNodeId;
};

struct AudioFormat {
    int32_t sampleRate = 0;
    int32_t channels = 0;
    int32_t bitsPerSample = 0;
    long startTime = 0;
};

napi_status ParseNapiString(napi_env env, napi_value value, std::string &result);

void GetBitsPerSampleAndStreamFormat(const OH_AudioFormat& g_audioFormatOutput,
    int32_t* bitsPerSample, OH_AudioStream_SampleFormat* streamSampleFormat);

// Set Sampling Rate
OH_Audio_SampleRate SetSamplingRate(int32_t sampleRate);

// Set audio channels
OH_AudioChannelLayout SetChannelLayout(int32_t channels);

// Set bit depth
OH_Audio_SampleFormat SetSampleFormat(int32_t bitsPerSample);

// Bit depth conversion
void ConvertBitsPerSample(unsigned int& bitsPerSample, const unsigned int& bitsPerSampleMode);

int32_t GetBitsPerSample(OH_Audio_SampleFormat sampleFormat);

OH_EnvironmentType GetEnvEnumByNumber(int num);

napi_value ReturnResult(napi_env env, AudioSuiteResult result);

void FreeBuffer(char **buffer);

void FreeBufferOfVoid(void **buffer);

void ConvertToFloat(int format, unsigned inputSampleCount, void *src, float *dst);

void ConvertFromU8ToFloat(unsigned n, const uint8_t *a, float *b);

void ConvertFrom16BitToFloat(unsigned n, const int16_t *a, float *b);

void ConvertFrom24BitToFloat(unsigned n, const uint8_t *a, float *b);

void ConvertFrom32BitToFloat(unsigned n, const int32_t *a, float *b);

uint32_t Read24Bit(const uint8_t *p);

void SetAudioFormat(const int sampleRate, const int channels, const int bitsPerSample);

long GetAudioDuration(long pcmDataLength, int sampleRate, int channels, int bitsPerSample);

long GetAudioSize(int sampleRate, int channels, int bitsPerSample);

int GetBit(int bitsPerSample);

bool AddWriteDataBuffer(const std::string inputId, const long oldStartTime, const long newStartTime,
                        std::vector<long> indexs, bool isCopyMultiple);

bool UpdateWriteDataBuffer(const std::string inputId, const long startTime, long startIndex, long endIndex);

bool DeleteWriteDataBuffer(const std::string inputId, const long originStartTime);

bool SetWriteDataBuffer(const std::string inputId, const long originStartTime, const long newStartTime);

OH_AudioStream_SampleFormat ConvertInt2AudioStream(const int32_t sampleFormat);

#endif //#define AUDIOEDITTESTAPP_UTILS_H