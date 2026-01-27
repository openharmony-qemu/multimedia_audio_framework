/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

#ifndef AUDIOEDITTESTAPP_REGISTERCALLBACK_NAPI_H
#define AUDIOEDITTESTAPP_REGISTERCALLBACK_NAPI_H

#include <js_native_api_types.h>

// 音频播放
napi_ref callbackAudioRendererRef = nullptr;

// 获取音频播放的finished的值
napi_ref callbackStringRef = nullptr;

// 判断音频是否在缓存中
napi_ref callbackAudioCacheRef = nullptr;

// 获取wav格式
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
