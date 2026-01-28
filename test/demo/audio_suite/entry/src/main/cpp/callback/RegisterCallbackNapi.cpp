/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

#include "callback/RegisterCallback.h"
#include <js_native_api.h>
#include <js_native_api_types.h>

// Register a callback to obtain the finished value of audio playback.
napi_value RegisterFinishedCallback(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1];
    napi_value callback;
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    // Creating a Global Reference
    napi_create_reference(env, args[0], 1, &callbackAudioRendererRef);

    // Creating Thread-Safe Functions
    napi_value global;
    napi_get_global(env, &global);
    napi_get_reference_value(env, callbackAudioRendererRef, &callback);
    napi_value name;
    napi_status status = napi_create_string_utf8(env, "CallBooleanCallback", NAPI_AUTO_LENGTH, &name);
    napi_create_threadsafe_function(env, callback, NULL, name, 1, 1, NULL, NULL, NULL, CallBoolThread, &tsfnBoolean);

    napi_value result;
    napi_get_undefined(env, &result);
    return result;
}

napi_value RegisterStringCallback(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1];
    napi_value callback;
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    // Creating a Global Reference
    napi_create_reference(env, args[0], 1, &callbackStringRef);

    // Creating Thread-Safe Functions
    napi_value global;
    napi_get_global(env, &global);
    napi_get_reference_value(env, callbackStringRef, &callback);
    napi_value name;
    napi_status status = napi_create_string_utf8(env, "callStringCallback", NAPI_AUTO_LENGTH, &name);
    napi_create_threadsafe_function(env, callback, NULL, name, 1, 1, NULL, NULL, NULL, CallStringThread, &tsfnString);

    napi_value result;
    napi_get_undefined(env, &result);
    return result;
}

napi_value RegisterAudioCacheCallback(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1];
    napi_value callback;
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    // Creating a Global Reference
    napi_create_reference(env, args[0], 1, &callbackAudioCacheRef);

    // Creating Thread-Safe Functions
    napi_value global;
    napi_get_global(env, &global);
    napi_get_reference_value(env, callbackAudioCacheRef, &callback);
    napi_value name;
    napi_status status = napi_create_string_utf8(env, "CallAudioCacheCallback", NAPI_AUTO_LENGTH, &name);
    napi_create_threadsafe_function(env, callback, NULL, name, 1, 1, NULL, NULL, NULL, CallBoolThread, &tsfnAudioCache);

    napi_value result;
    napi_get_undefined(env, &result);
    return result;
}

napi_value RegisterAudioFormatCallback(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1];
    napi_value callback;
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    // Creating a Global Reference
    napi_create_reference(env, args[0], 1, &callbackStringArrayRef);

    // Creating Thread-Safe Functions
    napi_value global;
    napi_get_global(env, &global);
    napi_get_reference_value(env, callbackStringArrayRef, &callback);
    napi_value name;
    napi_status status = napi_create_string_utf8(env, "CallStringArrayCallback", NAPI_AUTO_LENGTH, &name);
    napi_create_threadsafe_function(env, callback, NULL, name, 1, 1, NULL, NULL, NULL, CallStringArrayThread,
                                    &tsfnStringArray);

    napi_value result;
    napi_get_undefined(env, &result);
    return result;
}

napi_value UnregisterFinishedCallback(napi_env env, napi_callback_info info)
{
    if (tsfnBoolean != nullptr) {
        // Releasing Thread-Safe Functions
        napi_release_threadsafe_function(tsfnBoolean, napi_tsfn_release);
        tsfnBoolean = nullptr;
    }
    if (callbackAudioRendererRef != nullptr) {
        napi_delete_reference(env, callbackAudioRendererRef);
        callbackAudioRendererRef = nullptr;
    }
    return nullptr;
}

napi_value UnregisterAudioFormatCallback(napi_env env, napi_callback_info info)
{
    if (tsfnStringArray != nullptr) {
        // Releasing Thread-Safe Functions
        napi_release_threadsafe_function(tsfnStringArray, napi_tsfn_release);
        tsfnStringArray = nullptr;
    }
    if (callbackStringArrayRef != nullptr) {
        napi_delete_reference(env, callbackStringArrayRef);
        callbackStringArrayRef = nullptr;
    }
    return nullptr;
}

napi_value UnregisterStringCallback(napi_env env, napi_callback_info info)
{
    if (tsfnString != nullptr) {
        // Releasing Thread-Safe Functions
        napi_release_threadsafe_function(tsfnString, napi_tsfn_release);
        tsfnString = nullptr;
    }
    if (callbackStringRef != nullptr) {
        napi_delete_reference(env, callbackStringRef);
        callbackStringRef = nullptr;
    }
    return nullptr;
}

napi_value UnregisterAudioCacheCallback(napi_env env, napi_callback_info info)
{
    if (tsfnAudioCache != nullptr) {
        // Releasing Thread-Safe Functions
        napi_release_threadsafe_function(tsfnAudioCache, napi_tsfn_release);
        tsfnAudioCache = nullptr;
    }
    if (callbackAudioCacheRef != nullptr) {
        napi_delete_reference(env, callbackAudioCacheRef);
        callbackAudioCacheRef = nullptr;
    }
    return nullptr;
}