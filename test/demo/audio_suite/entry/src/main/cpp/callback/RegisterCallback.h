/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2026. ALL rights reserved.
 */

#ifndef REGISTERCALLBACK_H
#define REGISTERCALLBACK_H

#include <vector>
#include <string>
#include "napi/native_api.h"

extern napi_threadsafe_function tsfnStringArray;

extern napi_threadsafe_function tsfnBoolean;

extern napi_threadsafe_function tsfnString;

extern napi_threadsafe_function tsfnAudioCache;

extern napi_ref callbackAudioRendererRef;

extern napi_ref callbackStringRef;

extern napi_ref callbackAudioCacheRef;

extern napi_ref callbackStringArrayRef;

void CallStringArrayThread(napi_env env, napi_value js_callback, void *context, void *data);
void CallStringArrayCallback(const std::vector<std::string> &strings);

void CallBoolThread(napi_env env, napi_value js_callback, void *context, void *data);
void CallBooleanCallback(int result);

void CallStringThread(napi_env env, napi_value js_callback, void *context, void *data);

void CallStringCallback(const std::string &result);

void CallAudioCacheThread(napi_env env, napi_value js_callback, void *data);

void CallAudioCacheCallback(int result);

#endif // REGISTERCALLBACK_H
