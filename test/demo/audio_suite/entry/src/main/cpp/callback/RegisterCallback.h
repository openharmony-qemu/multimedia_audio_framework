/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

#ifndef REGISTERCALLBACK_H
#define REGISTERCALLBACK_H

#include <vector>
#include <string>
#include "napi/native_api.h"

extern napi_threadsafe_function tsfnStringArray;
extern napi_threadsafe_function tsfnBoolean;

void callStringArrayThread(napi_env env, napi_value js_callback, void *context, void *data);
void callStringArrayCallback(const std::vector<std::string>& strings);

void callBoolThread(napi_env env, napi_value js_callback, void *context, void *data);
void callBooleanCallback(int result);

#endif //REGISTERCALLBACK_H
