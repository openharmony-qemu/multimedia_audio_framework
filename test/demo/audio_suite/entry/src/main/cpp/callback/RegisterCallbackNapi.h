/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

#ifndef AUDIOEDITTESTAPP_REGISTERCALLBACK_NAPI_H
#define AUDIOEDITTESTAPP_REGISTERCALLBACK_NAPI_H

#include <js_native_api_types.h>

// Audio playback
napi_ref callbackAudioRendererRef = nullptr;

// Obtains the finished value of audio playback.
napi_ref callbackStringRef = nullptr;

// Determines whether the audio is in the cache.
napi_ref callbackAudioCacheRef = nullptr;

// Obtain the WAV format.
napi_ref callbackStringArrayRef = nullptr;

napi_value RegisterFinishedCallback(napi_env env, napi_callback_info info);

napi_value RegisterStringCallback(napi_env env, napi_callback_info info);

napi_value RegisterAudioCacheCallback(napi_env env, napi_callback_info info);

napi_value RegisterAudioFormatCallback(napi_env env, napi_callback_info info);

napi_value UnregisterFinishedCallback(napi_env env, napi_callback_info info);

napi_value UnregisterAudioFormatCallback(napi_env env, napi_callback_info info);

napi_value UnregisterStringCallback(napi_env env, napi_callback_info info);

napi_value UnregisterAudioCacheCallback(napi_env env, napi_callback_info info);

#endif //AUDIOEDITTESTAPP_REGISTERCALLBACK_NAPI_H
