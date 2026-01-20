//
// Created on 2025/12/2.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".
#include "napi/native_api.h"
#include "audioEffectNode/EffectNode.h"

#ifndef AUDIO_SUITE_AUDIORECORD_H
#define AUDIO_SUITE_AUDIORECORD_H

extern int32_t g_samplingRate;
extern int32_t g_channelCount;
extern int32_t g_bitsPerSample;

napi_value AudioCapturerInit(napi_env env, napi_callback_info info);

napi_value AudioCapturerStart(napi_env env, napi_callback_info info);

napi_value AudioCapturerStop(napi_env env, napi_callback_info info);

napi_value AudioCapturerRelease(napi_env env, napi_callback_info info);

napi_value GetAudioFrames(napi_env env, napi_callback_info info);

napi_value MixRecordBuffer(napi_env env, napi_callback_info info);

napi_value AudioCapturerPause(napi_env env, napi_callback_info info);

napi_value MixPlayInitBuffer(napi_env env, napi_callback_info info);

void convertFormat();

napi_value ClearRecordBuffer(napi_env env, napi_callback_info info);

napi_value RealPlayRecordBuffer(napi_env env, napi_callback_info info);

#endif //AUDIO_SUITE_AUDIORECORD_H
