/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

#include "RegisterCallback.h"
#include "hilog/log.h"

const int GLOBAL_RESMGR = 0xFF00;
const char *REGISTERCALLBACK_TAG = "[AudioEditTestApp_RegisterCallback_cpp]";

napi_threadsafe_function tsfnStringArray = nullptr;
napi_threadsafe_function tsfnBoolean = nullptr;
napi_threadsafe_function tsfnString = nullptr;
napi_threadsafe_function tsfnAudioCache = nullptr;

void CallStringArrayThread(napi_env env, napi_value js_callback, void *context, void *data)
{
    (void)context;
    std::vector<std::string> *strings = static_cast<std::vector<std::string> *>(data);
    size_t count = strings->size();
    napi_value resultArray;
    napi_create_array_with_length(env, count, &resultArray);
    for (size_t i = 0; i < count; ++i) {
        napi_value stringValue;
        napi_create_string_utf8(env, (*strings)[i].c_str(), NAPI_AUTO_LENGTH, &stringValue);
        napi_set_element(env, resultArray, i, stringValue);
    }

    delete strings; // Releasing String Array

    napi_call_function(env, NULL, js_callback, 1, &resultArray, NULL);
}

void CallStringArrayCallback(const std::vector<std::string> &strings)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, REGISTERCALLBACK_TAG,
                 "audioEditTest CallStringArrayCallback called");
    if (tsfnStringArray == nullptr) {
        return;
    }
    // Create a new copy
    std::vector<std::string> *stringsCopy = new std::vector<std::string>(strings);
    if (napi_call_threadsafe_function(tsfnStringArray, stringsCopy, napi_tsfn_blocking) != napi_ok) {
        delete stringsCopy;
        stringsCopy = nullptr;
    }
}

void CallBoolThread(napi_env env, napi_value js_callback, void *context, void *data)
{
    (void)context;
    int result = *(int *)data;
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, REGISTERCALLBACK_TAG,
                 "audioEditTest CallBoolThread result: %{public}d", result);
    napi_value resultValue;
    napi_get_boolean(env, result != 0, &resultValue);
    napi_call_function(env, NULL, js_callback, 1, &resultValue, NULL);
    free(data);
}

void CallBooleanCallback(int result)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, REGISTERCALLBACK_TAG,
                 "audioEditTest CallBooleanCallback result: %{public}d", result);
    if (tsfnBoolean == nullptr) {
        return;
    }

    int *data = (int *)malloc(sizeof(int));
    if (data == nullptr) {
        return;
    }
    *data = result;

    if (napi_call_threadsafe_function(tsfnBoolean, data, napi_tsfn_blocking) != napi_ok) {
        free(data);
        data = nullptr;
    }
}

void CallStringThread(napi_env env, napi_value js_callback, void *context, void *data)
{
    std::string result = *(std::string *)data;
    napi_value resultValue;
    napi_create_string_utf8(env, result.c_str(), NAPI_AUTO_LENGTH, &resultValue);
    napi_call_function(env, NULL, js_callback, 1, &resultValue, NULL);
    delete data; 
}

void CallStringCallback(const std::string &result)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, REGISTERCALLBACK_TAG,
                 "audioEditTest callStringCallback result: %{public}s", result.c_str());
    if (tsfnString == nullptr) {
        return;
    }

    std::string *data = new std::string(result);

    napi_call_threadsafe_function(tsfnString, data, napi_tsfn_blocking);
}

void CallAudioCacheThread(napi_env env, napi_value js_callback, void *context, void *data)
{
    int result = *(int *)data;
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, REGISTERCALLBACK_TAG,
                 "audioEditTest CallBoolThread result: %{public}d", result);
    napi_value resultValue;
    napi_get_boolean(env, result != 0, &resultValue);
    napi_call_function(env, NULL, js_callback, 1, &resultValue, NULL);
    free(data);
}

void CallAudioCacheCallback(int result)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, REGISTERCALLBACK_TAG,
                 "audioEditTest CallBooleanCallback result: %{public}d", result);
    if (tsfnAudioCache == nullptr) {
        return;
    }

    int *data = (int *)malloc(sizeof(int));
    if (data == nullptr) {
        return;
    }
    *data = result;

    if (napi_call_threadsafe_function(tsfnAudioCache, data, napi_tsfn_blocking) != napi_ok) {
        free(data);
        data = nullptr;
    }
}