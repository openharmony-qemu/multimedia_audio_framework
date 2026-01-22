/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

#ifndef AUDIO_SUITE_AUDIORECORD_H
#define AUDIO_SUITE_AUDIORECORD_H

#include "napi/native_api.h"
#include "audioEffectNode/EffectNode.h"

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

void ConvertFormat();

napi_value ClearRecordBuffer(napi_env env, napi_callback_info info);

napi_value RealPlayRecordBuffer(napi_env env, napi_callback_info info);

#endif //AUDIO_SUITE_AUDIORECORD_H
