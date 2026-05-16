/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#ifndef LOG_TAG
#define LOG_TAG "NapiAudioDebuggingManager"
#endif

#include "napi_audio_debugging_manager.h"
#include "napi_audio_renderer.h"
#include "napi_audio_capturer.h"
#include "napi_audio_loopback.h"
#include "napi_audio_error.h"
#include "napi_param_utils.h"
#include "audio_errors.h"
#include "audio_manager_log.h"

namespace OHOS {
namespace AudioStandard {
namespace {
constexpr size_t ARG_COUNT_TWO = 2;
}
NapiAudioDebuggingManager::NapiAudioDebuggingManager()
    : env_(nullptr) {}

NapiAudioDebuggingManager::~NapiAudioDebuggingManager() = default;

napi_value NapiAudioDebuggingManager::Init(napi_env env, napi_value exports)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("printAppInfo", PrintAppInfo),
        DECLARE_NAPI_FUNCTION("printRendererInfo", PrintRendererInfo),
        DECLARE_NAPI_FUNCTION("printCapturerInfo", PrintCapturerInfo),
        DECLARE_NAPI_FUNCTION("printLoopbackInfo", PrintLoopbackInfo),
        DECLARE_NAPI_FUNCTION("printSessionInfo", PrintSessionInfo),
    };

    napi_value constructor = nullptr;
    napi_status status = napi_define_class(env, AUDIO_DEBUGGING_MGR_NAPI_CLASS_NAME.c_str(),
        NAPI_AUTO_LENGTH, Construct, nullptr,
        sizeof(properties) / sizeof(properties[0]), properties, &constructor);
    if (status != napi_ok) {
        AUDIO_ERR_LOG("Define class failed");
        return nullptr;
    }

    return constructor;
}

napi_value NapiAudioDebuggingManager::Construct(napi_env env, napi_callback_info info)
{
    napi_value jsThis = nullptr;
    napi_status status = napi_get_cb_info(env, info, nullptr, nullptr, &jsThis, nullptr);
    if (status != napi_ok) {
        return nullptr;
    }

    auto *napiDebuggingManager = new (std::nothrow) NapiAudioDebuggingManager();
    if (napiDebuggingManager == nullptr) {
        AUDIO_ERR_LOG("No memory");
        return nullptr;
    }
    napiDebuggingManager->env_ = env;

    status = napi_wrap(env, jsThis, static_cast<void *>(napiDebuggingManager),
        Destructor, nullptr, nullptr);
    if (status != napi_ok) {
        delete napiDebuggingManager;
        return nullptr;
    }

    return jsThis;
}

void NapiAudioDebuggingManager::Destructor(napi_env env, void *nativeObject, void *finalizeHint)
{
    if (nativeObject != nullptr) {
        auto *napiDebuggingManager = static_cast<NapiAudioDebuggingManager *>(nativeObject);
        delete napiDebuggingManager;
    }
}

napi_value NapiAudioDebuggingManager::CreateDebuggingManagerWrapper(napi_env env)
{
    napi_value constructor = Init(env, nullptr);
    if (constructor == nullptr) {
        AUDIO_ERR_LOG("Init failed");
        return nullptr;
    }

    napi_value result = nullptr;
    napi_status status = napi_new_instance(env, constructor, 0, nullptr, &result);
    if (status != napi_ok) {
        AUDIO_ERR_LOG("New instance failed");
        return nullptr;
    }

    return result;
}

NapiAudioDebuggingManager *NapiAudioDebuggingManager::GetParamWithSync(const napi_env &env,
    napi_callback_info info, size_t &argc, napi_value *args)
{
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        AUDIO_ERR_LOG("Get cb info failed");
        return nullptr;
    }

    NapiAudioDebuggingManager *napiDebuggingManager = nullptr;
    status = napi_unwrap(env, args[0], reinterpret_cast<void **>(&napiDebuggingManager));
    if (status != napi_ok || napiDebuggingManager == nullptr) {
        AUDIO_ERR_LOG("Unwrap failed");
        return nullptr;
    }

    return napiDebuggingManager;
}

napi_value NapiAudioDebuggingManager::PrintAppInfo(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok || argc != 1) {
        NapiAudioError::ThrowError(env, "PrintAppInfo invalid arguments", NAPI_ERR_INVALID_PARAM);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    int32_t fd = -1;
    if (NapiParamUtils::GetValueInt32(env, fd, args[0]) != napi_ok) {
        NapiAudioError::ThrowError(env, "PrintAppInfo fd type invalid", NAPI_ERR_INVALID_PARAM);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    if (AudioDebugManager::GetInstance().PrintAppAudioDebugInfo(fd) != SUCCESS) {
        NapiAudioError::ThrowError(env, "PrintAppInfo failed", NAPI_ERR_SYSTEM);
    }
    return NapiParamUtils::GetUndefinedValue(env);
}

napi_value NapiAudioDebuggingManager::PrintRendererInfo(napi_env env, napi_callback_info info)
{
    size_t argc = ARG_COUNT_TWO;
    napi_value args[ARG_COUNT_TWO] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok || argc != ARG_COUNT_TWO) {
        NapiAudioError::ThrowError(env, "PrintRendererInfo invalid arguments", NAPI_ERR_INVALID_PARAM);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    NapiAudioRenderer *napiRenderer = nullptr;
    status = napi_unwrap(env, args[0], reinterpret_cast<void **>(&napiRenderer));
    if (status != napi_ok || napiRenderer == nullptr || napiRenderer->audioRenderer_ == nullptr) {
        NapiAudioError::ThrowError(env, "PrintRendererInfo renderer invalid", NAPI_ERR_INVALID_PARAM);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    int32_t fd = -1;
    if (NapiParamUtils::GetValueInt32(env, fd, args[1]) != napi_ok) {
        NapiAudioError::ThrowError(env, "PrintRendererInfo fd type invalid", NAPI_ERR_INVALID_PARAM);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    uintptr_t rendererKey = reinterpret_cast<uintptr_t>(napiRenderer->audioRenderer_.get());
    if (AudioDebugManager::GetInstance().PrintAudioRendererDebugInfo(rendererKey, fd) != SUCCESS) {
        NapiAudioError::ThrowError(env, "PrintRendererInfo failed", NAPI_ERR_SYSTEM);
    }
    return NapiParamUtils::GetUndefinedValue(env);
}

napi_value NapiAudioDebuggingManager::PrintCapturerInfo(napi_env env, napi_callback_info info)
{
    size_t argc = ARG_COUNT_TWO;
    napi_value args[ARG_COUNT_TWO] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok || argc != ARG_COUNT_TWO) {
        NapiAudioError::ThrowError(env, "PrintCapturerInfo invalid arguments", NAPI_ERR_INVALID_PARAM);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    NapiAudioCapturer *napiCapturer = nullptr;
    status = napi_unwrap(env, args[0], reinterpret_cast<void **>(&napiCapturer));
    if (status != napi_ok || napiCapturer == nullptr || napiCapturer->audioCapturer_ == nullptr) {
        NapiAudioError::ThrowError(env, "PrintCapturerInfo capturer invalid", NAPI_ERR_INVALID_PARAM);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    int32_t fd = -1;
    if (NapiParamUtils::GetValueInt32(env, fd, args[1]) != napi_ok) {
        NapiAudioError::ThrowError(env, "PrintCapturerInfo fd type invalid", NAPI_ERR_INVALID_PARAM);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    uintptr_t capturerKey = reinterpret_cast<uintptr_t>(napiCapturer->audioCapturer_.get());
    if (AudioDebugManager::GetInstance().PrintAudioCapturerDebugInfo(capturerKey, fd) != SUCCESS) {
        NapiAudioError::ThrowError(env, "PrintCapturerInfo failed", NAPI_ERR_SYSTEM);
    }
    return NapiParamUtils::GetUndefinedValue(env);
}

napi_value NapiAudioDebuggingManager::PrintLoopbackInfo(napi_env env, napi_callback_info info)
{
    size_t argc = ARG_COUNT_TWO;
    napi_value args[ARG_COUNT_TWO] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok || argc != ARG_COUNT_TWO) {
        NapiAudioError::ThrowError(env, "PrintLoopbackInfo invalid arguments", NAPI_ERR_INVALID_PARAM);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    NapiAudioLoopback *napiLoopback = nullptr;
    status = napi_unwrap(env, args[0], reinterpret_cast<void **>(&napiLoopback));
    if (status != napi_ok || napiLoopback == nullptr || napiLoopback->loopback_ == nullptr) {
        NapiAudioError::ThrowError(env, "PrintLoopbackInfo loopback invalid", NAPI_ERR_INVALID_PARAM);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    int32_t fd = -1;
    if (NapiParamUtils::GetValueInt32(env, fd, args[1]) != napi_ok) {
        NapiAudioError::ThrowError(env, "PrintLoopbackInfo fd type invalid", NAPI_ERR_INVALID_PARAM);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    uint32_t loopbackKey = napiLoopback->loopback_->GetDebugKey();
    if (AudioDebugManager::GetInstance().PrintAudioLoopbackDebugInfo(loopbackKey, fd) != SUCCESS) {
        NapiAudioError::ThrowError(env, "PrintLoopbackInfo failed", NAPI_ERR_SYSTEM);
    }
    return NapiParamUtils::GetUndefinedValue(env);
}

napi_value NapiAudioDebuggingManager::PrintSessionInfo(napi_env env, napi_callback_info info)
{
    size_t argc = ARG_COUNT_TWO;
    napi_value args[ARG_COUNT_TWO] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok || argc != ARG_COUNT_TWO) {
        NapiAudioError::ThrowError(env, "PrintSessionInfo invalid arguments", NAPI_ERR_INVALID_PARAM);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    NapiAudioError::ThrowError(env, "PrintSessionInfo is unsupported now", NAPI_ERR_UNSUPPORTED);
    return NapiParamUtils::GetUndefinedValue(env);
}
} // namespace AudioStandard
} // namespace OHOS
