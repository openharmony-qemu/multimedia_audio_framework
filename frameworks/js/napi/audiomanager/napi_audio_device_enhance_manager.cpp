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
#define LOG_TAG "NapiAudioDeviceEnhanceManager"
#endif

#include "napi_audio_device_enhance_manager.h"

#include "audio_errors.h"
#include "audio_manager_log.h"
#include "napi_audio_error.h"
#include "napi_param_utils.h"

namespace OHOS {
namespace AudioStandard {
static __thread napi_ref g_deviceEnhanceManagerConstructor = nullptr;

namespace {
bool GetRendererFromJs(napi_env env, napi_value value, std::shared_ptr<AudioRenderer> &renderer)
{
    NapiAudioRenderer *napiRenderer = nullptr;
    napi_status status = napi_unwrap(env, value, reinterpret_cast<void **>(&napiRenderer));
    CHECK_AND_RETURN_RET_LOG(status == napi_ok && napiRenderer != nullptr, false, "unwrap renderer failed");
    renderer = napiRenderer->audioRenderer_;
    return renderer != nullptr;
}

bool GetCapturerFromJs(napi_env env, napi_value value, std::shared_ptr<AudioCapturer> &capturer)
{
    NapiAudioCapturer *napiCapturer = nullptr;
    napi_status status = napi_unwrap(env, value, reinterpret_cast<void **>(&napiCapturer));
    CHECK_AND_RETURN_RET_LOG(status == napi_ok && napiCapturer != nullptr, false, "unwrap capturer failed");
    capturer = napiCapturer->audioCapturer_;
    return capturer != nullptr;
}
}

NapiAudioDeviceEnhanceManager::NapiAudioDeviceEnhanceManager()
    : audioDeviceEnhanceMngr_(nullptr), env_(nullptr) {}

NapiAudioDeviceEnhanceManager::~NapiAudioDeviceEnhanceManager() = default;

bool NapiAudioDeviceEnhanceManager::CheckContextStatus(
    std::shared_ptr<AudioDeviceEnhanceManagerAsyncContext> context)
{
    CHECK_AND_RETURN_RET_LOG(context != nullptr, false, "context object is nullptr.");
    if (context->native == nullptr) {
        context->SignError(NAPI_ERR_SYSTEM);
        AUDIO_ERR_LOG("context object state is error.");
        return false;
    }
    return true;
}

bool NapiAudioDeviceEnhanceManager::CheckDeviceEnhanceManagerStatus(
    NapiAudioDeviceEnhanceManager *napi, std::shared_ptr<AudioDeviceEnhanceManagerAsyncContext> context)
{
    CHECK_AND_RETURN_RET_LOG(napi != nullptr, false, "napi object is nullptr.");
    if (napi->audioDeviceEnhanceMngr_ == nullptr) {
        context->SignError(NAPI_ERR_SYSTEM);
        AUDIO_ERR_LOG("audioDeviceEnhanceMngr_ is nullptr.");
        return false;
    }
    return true;
}

void NapiAudioDeviceEnhanceManager::Destructor(napi_env env, void *nativeObject, void *finalizeHint)
{
    CHECK_AND_RETURN_LOG(nativeObject, "Native object is null");
    auto obj = static_cast<NapiAudioDeviceEnhanceManager *>(nativeObject);
    ObjectRefMap<NapiAudioDeviceEnhanceManager>::DecreaseRef(obj);
}

napi_value NapiAudioDeviceEnhanceManager::Construct(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    size_t argc = 0;
    napi_value thisVar = nullptr;
    napi_get_cb_info(env, info, &argc, nullptr, &thisVar, nullptr);

    auto napiManager = std::make_unique<NapiAudioDeviceEnhanceManager>();
    CHECK_AND_RETURN_RET_LOG(napiManager != nullptr, result, "No memory");
    napiManager->audioDeviceEnhanceMngr_ = &AudioDeviceEnhanceManager::GetInstance();
    napiManager->env_ = env;

    ObjectRefMap<NapiAudioDeviceEnhanceManager>::Insert(napiManager.get());
    napi_status status = napi_wrap(env, thisVar, static_cast<void *>(napiManager.get()),
        NapiAudioDeviceEnhanceManager::Destructor, nullptr, nullptr);
    if (status != napi_ok) {
        ObjectRefMap<NapiAudioDeviceEnhanceManager>::Erase(napiManager.get());
        return result;
    }
    napiManager.release();
    return thisVar;
}

napi_value NapiAudioDeviceEnhanceManager::CreateDeviceEnhanceManagerWrapper(napi_env env)
{
    napi_value result = nullptr;
    napi_value constructor;
    napi_status status = napi_get_reference_value(env, g_deviceEnhanceManagerConstructor, &constructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, NapiParamUtils::GetUndefinedValue(env),
        "napi_get_reference_value fail");

    status = napi_new_instance(env, constructor, 0, nullptr, &result);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, NapiParamUtils::GetUndefinedValue(env),
        "napi_new_instance fail");
    return result;
}

napi_value NapiAudioDeviceEnhanceManager::IsEnhancedRoutingSupported(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = {};
    napi_value thisVar = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_SYSTEM),
        "napi_get_cb_info failed");
    CHECK_AND_RETURN_RET_LOG(argc == PARAM0, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID),
        "invalid arguments");

    NapiAudioDeviceEnhanceManager *obj = nullptr;
    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    CHECK_AND_RETURN_RET_LOG(status == napi_ok && obj != nullptr,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_SYSTEM), "napi_unwrap failed");
    ObjectRefMap objectGuard(obj);
    auto *napiManager = objectGuard.GetPtr();
    CHECK_AND_RETURN_RET_LOG(napiManager != nullptr && napiManager->audioDeviceEnhanceMngr_ != nullptr,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_SYSTEM), "device enhance manager state is error");

    bool supported = false;
    int32_t ret = napiManager->audioDeviceEnhanceMngr_->IsEnhancedRoutingSupported(supported);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_SYSTEM),
        "IsEnhancedRoutingSupported failed");
    NapiParamUtils::SetValueBoolean(env, supported, result);
    return result;
}

napi_value NapiAudioDeviceEnhanceManager::SelectOutputDevice(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioDeviceEnhanceManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("SelectOutputDevice failed : no memory");
        NapiAudioError::ThrowError(env, NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments", NAPI_ERR_INPUT_INVALID);
        context->status = NapiParamUtils::GetAudioDeviceDescriptor(
            env, context->deviceDescriptor, context->argTransFlag, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok && context->argTransFlag,
            "invalid device descriptor", NAPI_ERR_INPUT_INVALID);
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioDeviceEnhanceManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckDeviceEnhanceManagerStatus(napiManager, context),
            "device enhance manager state is error.");
        context->intValue = napiManager->audioDeviceEnhanceMngr_->SelectOutputDevice(context->deviceDescriptor);
        if (context->intValue == ERR_INVALID_PARAM) {
            context->SignError(NAPI_ERR_INVALID_PARAM);
            return;
        }
        if (context->intValue != SUCCESS) {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };
    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SelectOutputDevice", executor, complete);
}

napi_value NapiAudioDeviceEnhanceManager::SelectInputDevice(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioDeviceEnhanceManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("SelectInputDevice failed : no memory");
        NapiAudioError::ThrowError(env, NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments", NAPI_ERR_INPUT_INVALID);
        context->status = NapiParamUtils::GetAudioDeviceDescriptor(
            env, context->deviceDescriptor, context->argTransFlag, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok && context->argTransFlag,
            "invalid device descriptor", NAPI_ERR_INPUT_INVALID);
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioDeviceEnhanceManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckDeviceEnhanceManagerStatus(napiManager, context),
            "device enhance manager state is error.");
        context->intValue = napiManager->audioDeviceEnhanceMngr_->SelectInputDevice(context->deviceDescriptor);
        if (context->intValue == ERR_INVALID_PARAM) {
            context->SignError(NAPI_ERR_INVALID_PARAM);
            return;
        }
        if (context->intValue != SUCCESS) {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };
    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SelectInputDevice", executor, complete);
}

napi_value NapiAudioDeviceEnhanceManager::SelectOutputDeviceForAudioRenderer(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioDeviceEnhanceManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("SelectOutputDeviceForAudioRenderer failed : no memory");
        NapiAudioError::ThrowError(env, NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_TWO, "invalid arguments", NAPI_ERR_INPUT_INVALID);
        NAPI_CHECK_ARGS_RETURN_VOID(context, GetRendererFromJs(env, argv[PARAM0], context->audioRenderer),
            "invalid renderer", NAPI_ERR_INPUT_INVALID);
        context->status = NapiParamUtils::GetAudioDeviceDescriptor(
            env, context->deviceDescriptor, context->argTransFlag, argv[PARAM1]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok && context->argTransFlag,
            "invalid device descriptor", NAPI_ERR_INPUT_INVALID);
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioDeviceEnhanceManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckDeviceEnhanceManagerStatus(napiManager, context),
            "device enhance manager state is error.");
        context->intValue = napiManager->audioDeviceEnhanceMngr_->SelectOutputDeviceForAudioRenderer(
            context->audioRenderer, context->deviceDescriptor);
        if (context->intValue == ERR_INVALID_PARAM) {
            context->SignError(NAPI_ERR_INVALID_PARAM);
            return;
        }
        if (context->intValue != SUCCESS) {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };
    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SelectOutputDeviceForAudioRenderer", executor, complete);
}

napi_value NapiAudioDeviceEnhanceManager::SelectInputDeviceForAudioCapturer(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioDeviceEnhanceManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("SelectInputDeviceForAudioCapturer failed : no memory");
        NapiAudioError::ThrowError(env, NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_TWO, "invalid arguments", NAPI_ERR_INPUT_INVALID);
        NAPI_CHECK_ARGS_RETURN_VOID(context, GetCapturerFromJs(env, argv[PARAM0], context->audioCapturer),
            "invalid capturer", NAPI_ERR_INPUT_INVALID);
        context->status = NapiParamUtils::GetAudioDeviceDescriptor(
            env, context->deviceDescriptor, context->argTransFlag, argv[PARAM1]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok && context->argTransFlag,
            "invalid device descriptor", NAPI_ERR_INPUT_INVALID);
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioDeviceEnhanceManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckDeviceEnhanceManagerStatus(napiManager, context),
            "device enhance manager state is error.");
        context->intValue = napiManager->audioDeviceEnhanceMngr_->SelectInputDeviceForAudioCapturer(
            context->audioCapturer, context->deviceDescriptor);
        if (context->intValue == ERR_INVALID_PARAM) {
            context->SignError(NAPI_ERR_INVALID_PARAM);
            return;
        }
        if (context->intValue != SUCCESS) {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };
    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SelectInputDeviceForAudioCapturer", executor, complete);
}

napi_value NapiAudioDeviceEnhanceManager::GetSoundCardInfo(napi_env env, napi_callback_info info)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED), "No system permission");

    auto context = std::make_shared<AudioDeviceEnhanceManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("GetSoundCardInfo failed : no memory");
        NapiAudioError::ThrowError(env, NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioDeviceEnhanceManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckDeviceEnhanceManagerStatus(napiManager, context),
            "device enhance manager state is error.");
        context->soundCardInfo = napiManager->audioDeviceEnhanceMngr_->GetSoundCardInfo();
    };
    auto complete = [env, context](napi_value &output) {
        napi_value jsObj = nullptr;
        napi_status status = NapiParamUtils::ConvertSoundCardInfoToJs(env, context->soundCardInfo, jsObj);
        if (status != napi_ok) {
            NapiAudioError::ThrowError(env, "Fail to convert SoundCardInfo to Js object", NAPI_ERR_SYSTEM);
            output = NapiParamUtils::GetUndefinedValue(env);
            return;
        }
        output = jsObj;
    };
    return NapiAsyncWork::Enqueue(env, context, "GetSoundCardInfo", executor, complete);
}

napi_value NapiAudioDeviceEnhanceManager::Init(napi_env env, napi_value exports)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    napi_property_descriptor props[] = {
        DECLARE_NAPI_FUNCTION("isEnhancedRoutingSupported", IsEnhancedRoutingSupported),
        DECLARE_NAPI_FUNCTION("selectOutputDevice", SelectOutputDevice),
        DECLARE_NAPI_FUNCTION("selectInputDevice", SelectInputDevice),
        DECLARE_NAPI_FUNCTION("selectOutputDeviceForAudioRenderer", SelectOutputDeviceForAudioRenderer),
        DECLARE_NAPI_FUNCTION("selectInputDeviceForAudioCapturer", SelectInputDeviceForAudioCapturer),
        DECLARE_NAPI_FUNCTION("getSoundCardInfo", GetSoundCardInfo),
    };

    napi_value constructor = nullptr;
    napi_status status = napi_define_class(env, AUDIO_DEVICE_ENHANCE_MANAGER_NAPI_CLASS_NAME.c_str(), NAPI_AUTO_LENGTH,
        Construct, nullptr, sizeof(props) / sizeof(props[0]), props, &constructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_define_class fail");

    constexpr int32_t refCount = 1;
    status = napi_create_reference(env, constructor, refCount, &g_deviceEnhanceManagerConstructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_create_reference fail");
    status = napi_set_named_property(env, exports, AUDIO_DEVICE_ENHANCE_MANAGER_NAPI_CLASS_NAME.c_str(), constructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_set_named_property fail");
    return exports;
}
} // namespace AudioStandard
} // namespace OHOS
