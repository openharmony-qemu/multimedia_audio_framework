/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "NapiAudioLoopback"
#endif

#include "napi_audio_loopback.h"

#include "napi_param_utils.h"
#include "napi_audio_error.h"
#include "napi_audio_enum.h"
#include "napi_audio_loopback_callback.h"
#include "audio_stream_manager.h"
#include "audio_manager_log.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {
static __thread napi_ref g_loopbackConstructor = nullptr;
static constexpr double MIN_VOLUME_IN_DOUBLE = 0.0;
static constexpr double MAX_VOLUME_IN_DOUBLE = 1.0;
mutex NapiAudioLoopback::createMutex_;
int32_t NapiAudioLoopback::isConstructSuccess_ = SUCCESS;
AudioLoopbackMode NapiAudioLoopback::sLoopbackMode_ = LOOPBACK_HARDWARE;

NapiAudioLoopback::NapiAudioLoopback()
    : loopback_(nullptr), env_(nullptr) {}

NapiAudioLoopback::~NapiAudioLoopback() = default;

void NapiAudioLoopback::Destructor(napi_env env, void *nativeObject, void *finalizeHint)
{
    if (nativeObject == nullptr) {
        AUDIO_WARNING_LOG("Native object is null");
        return;
    }
    auto obj = static_cast<NapiAudioLoopback *>(nativeObject);
    ObjectRefMap<NapiAudioLoopback>::DecreaseRef(obj);
    AUDIO_INFO_LOG("Decrease obj count");
}

napi_value NapiAudioLoopback::Init(napi_env env, napi_value exports)
{
    napi_status status;
    napi_value constructor;
    napi_value result = nullptr;
    const int32_t refCount = 1;
    napi_get_undefined(env, &result);
    AUDIO_DEBUG_LOG("NapiAudioLoopback::Init");
    napi_property_descriptor audio_loopback_properties[] = {
        DECLARE_NAPI_FUNCTION("getStatus", GetStatus),
        DECLARE_NAPI_FUNCTION("setVolume", SetVolume),
        DECLARE_NAPI_FUNCTION("enable", Enable),
        DECLARE_NAPI_FUNCTION("on", On),
        DECLARE_NAPI_FUNCTION("off", Off),
        DECLARE_NAPI_FUNCTION("setReverbPreset", SetReverbPreset),
        DECLARE_NAPI_FUNCTION("getReverbPreset", GetReverbPreset),
        DECLARE_NAPI_FUNCTION("setEqualizerPreset", SetEqualizerPreset),
        DECLARE_NAPI_FUNCTION("getEqualizerPreset", GetEqualizerPreset),
    };

    napi_property_descriptor static_prop[] = {
        DECLARE_NAPI_STATIC_FUNCTION("createAudioLoopback", CreateAudioLoopback),
    };

    status = napi_define_class(env, NAPI_AUDIO_LOOPBACK_CLASS_NAME.c_str(), NAPI_AUTO_LENGTH, Construct, nullptr,
        sizeof(audio_loopback_properties) / sizeof(audio_loopback_properties[PARAM0]),
        audio_loopback_properties, &constructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_define_class failed");

    status = napi_create_reference(env, constructor, refCount, &g_loopbackConstructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_create_reference failed");

    status = napi_set_named_property(env, exports, NAPI_AUDIO_LOOPBACK_CLASS_NAME.c_str(), constructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_set_named_property failed");

    status = napi_define_properties(env, exports,
        sizeof(static_prop) / sizeof(static_prop[PARAM0]), static_prop);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_define_properties failed");

    return exports;
}

napi_value NapiAudioLoopback::Construct(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    size_t argCount = ARGS_TWO;
    napi_value thisVar = nullptr;
    status = napi_get_cb_info(env, info, &argCount, nullptr, &thisVar, nullptr);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "failed to napi_get_cb_info");

    unique_ptr<NapiAudioLoopback> napiLoopback = make_unique<NapiAudioLoopback>();
    CHECK_AND_RETURN_RET_LOG(napiLoopback != nullptr, result, "No memory");
    ObjectRefMap<NapiAudioLoopback>::Insert(napiLoopback.get());

    napiLoopback->env_ = env;
    auto loopbackMode = sLoopbackMode_;
    auto streamManager = AudioStreamManager::GetInstance();
    if (streamManager != nullptr && streamManager->IsAudioLoopbackSupported(loopbackMode)) {
        napiLoopback->loopback_ = AudioLoopback::CreateAudioLoopback(loopbackMode);
        if (napiLoopback->loopback_  == nullptr) {
            AUDIO_ERR_LOG("AudioLoopback Create failed");
            NapiAudioLoopback::isConstructSuccess_ = NAPI_ERR_NO_PERMISSION;
        }
    } else {
        AUDIO_ERR_LOG("AudioLoopback not supported");
        NapiAudioLoopback::isConstructSuccess_ = NAPI_ERR_UNSUPPORTED;
    }

    if (napiLoopback->loopback_ != nullptr && napiLoopback->callbackNapi_ == nullptr) {
        napiLoopback->callbackNapi_ = std::make_shared<NapiAudioLoopbackCallback>(env);
        CHECK_AND_RETURN_RET_LOG(napiLoopback->callbackNapi_ != nullptr, nullptr, "No memory");
        int32_t ret = napiLoopback->loopback_->SetAudioLoopbackCallback(napiLoopback->callbackNapi_);
        CHECK_AND_RETURN_RET_LOG(!ret, result, "Construct SetLoopbackCallback failed");
    }

    status = napi_wrap(env, thisVar, static_cast<void*>(napiLoopback.get()),
        NapiAudioLoopback::Destructor, nullptr, nullptr);
    if (status != napi_ok) {
        ObjectRefMap<NapiAudioLoopback>::Erase(napiLoopback.get());
        return result;
    }
    napiLoopback.release();
    return thisVar;
}

napi_value NapiAudioLoopback::CreateAudioLoopbackWrapper(napi_env env, AudioLoopbackMode loopbackMode)
{
    lock_guard<mutex> lock(createMutex_);
    napi_status status = napi_invalid_arg;
    napi_value result = nullptr;
    napi_value constructor;

    status = napi_get_reference_value(env, g_loopbackConstructor, &constructor);
    if (status == napi_ok) {
        sLoopbackMode_ = loopbackMode;
        status = napi_new_instance(env, constructor, 0, nullptr, &result);
    }
    if (status != napi_ok) {
        AUDIO_ERR_LOG("Failed in CreateAudioLoopbackWrapper, %{public}d", status);
        napi_get_undefined(env, &result);
    }
    return result;
}

napi_value NapiAudioLoopback::CreateAudioLoopback(napi_env env, napi_callback_info info)
{
    AUDIO_INFO_LOG("CreateAudioLoopback");
    auto context = std::make_shared<AudioLoopbackAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("CreateAudioLoopback failed : no memory");
        NapiAudioError::ThrowError(env, "CreateAudioLoopback failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments",
            NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->loopbackMode, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "GetAudioLoopbackMode failed",
            NAPI_ERR_INVALID_PARAM);
        NAPI_CHECK_ARGS_RETURN_VOID(context,
            NapiAudioEnum::IsLegalInputArgumentAudioLoopbackMode(context->loopbackMode), "loopback mode invaild",
            NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);

    auto complete = [env, context](napi_value &output) {
        AudioLoopbackMode loopbackMode = static_cast<AudioLoopbackMode>(context->loopbackMode);
        output = CreateAudioLoopbackWrapper(env, loopbackMode);
        // IsConstructSuccess_ Used when creating a loopback fails.
        if (isConstructSuccess_ != SUCCESS) {
            context->SignError(isConstructSuccess_);
            isConstructSuccess_ = SUCCESS;
        }
    };

    return NapiAsyncWork::Enqueue(env, context, "CreateAudioLoopback", nullptr, complete);
}

napi_value NapiAudioLoopback::GetStatus(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioLoopbackAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("GetStatus failed : no memory");
        NapiAudioError::ThrowError(env, "GetStatus failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioLoopback*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioLoopback = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioLoopbackStatus(napiAudioLoopback, context),
            "context object state is error.");
        context->loopbackStatus = napiAudioLoopback->loopback_->GetStatus();
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueInt32(env, static_cast<int32_t>(context->loopbackStatus), output);
    };
    return NapiAsyncWork::Enqueue(env, context, "GetStatus", executor, complete);
}

napi_value NapiAudioLoopback::SetVolume(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioLoopbackAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("SetVolume failed : no memory");
        NapiAudioError::ThrowError(env, "SetVolume failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments",
            NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueDouble(env, context->volLevel, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "set volume failed",
            NAPI_ERR_INVALID_PARAM);
    };

    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioLoopback*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioLoopback = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioLoopbackStatus(napiAudioLoopback, context),
            "context object state is error.");
        if (context->volLevel < MIN_VOLUME_IN_DOUBLE || context->volLevel > MAX_VOLUME_IN_DOUBLE) {
            context->SignError(NAPI_ERR_INVALID_PARAM);
            return;
        }
        context->intValue = napiAudioLoopback->loopback_->SetVolume(static_cast<float>(context->volLevel));
        if (context->intValue != SUCCESS) {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SetVolume", executor, complete);
}

napi_value NapiAudioLoopback::Enable(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioLoopbackAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("Enable failed : no memory");
        NapiAudioError::ThrowError(env, "Enable failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments",
            NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueBoolean(env, context->enable, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "enable parameter failed",
            NAPI_ERR_INVALID_PARAM);
    };

    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioLoopback*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioLoopback = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioLoopbackStatus(napiAudioLoopback, context),
            "context object state is error.");
        context->isTrue = napiAudioLoopback->loopback_->Enable(context->enable);
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueBoolean(env, context->isTrue, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "Enable", executor, complete);
}

napi_value NapiAudioLoopback::On(napi_env env, napi_callback_info info)
{
    const size_t requireArgc = ARGS_TWO;
    size_t argc = ARGS_THREE;

    napi_value argv[requireArgc + 1] = {nullptr, nullptr, nullptr};
    napi_value jsThis = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &jsThis, nullptr);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_SYSTEM), "status error");
    CHECK_AND_RETURN_RET_LOG(argc >= requireArgc, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID, "mandatory parameters are left unspecified"), "requireArgc is invaild");

    napi_valuetype eventType = napi_undefined;
    napi_typeof(env, argv[PARAM0], &eventType);
    CHECK_AND_RETURN_RET_LOG(eventType == napi_string, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID, "incorrect parameter types: The type of eventType must be string"),
        "eventType is invaild");

    std::string callbackName = NapiParamUtils::GetStringArgument(env, argv[PARAM0]);
    AUDIO_DEBUG_LOG("AudioLoopbackNapi: On callbackName: %{public}s", callbackName.c_str());

    napi_valuetype handler = napi_undefined;
    if (argc == requireArgc) {
        napi_typeof(env, argv[PARAM1], &handler);
        CHECK_AND_RETURN_RET_LOG(handler == napi_function, NapiAudioError::ThrowErrorAndReturn(env,
            NAPI_ERR_INPUT_INVALID, "incorrect parameter types: The type of callback must be function"),
            "handler is invaild");
    } else {
        napi_valuetype paramArg1 = napi_undefined;
        napi_typeof(env, argv[PARAM1], &paramArg1);
        napi_valuetype expectedValType = napi_number;  // Default. Reset it with 'callbackName' if check, if required.
        CHECK_AND_RETURN_RET_LOG(paramArg1 == expectedValType, NapiAudioError::ThrowErrorAndReturn(env,
            NAPI_ERR_INPUT_INVALID, "incorrect parameter types: The type of frame must be number"),
            "paramArg1 is invaild");
        const int32_t arg2 = ARGS_TWO;
        napi_typeof(env, argv[arg2], &handler);
        CHECK_AND_RETURN_RET_LOG(handler == napi_function, NapiAudioError::ThrowErrorAndReturn(env,
            NAPI_ERR_INPUT_INVALID, "incorrect parameter types: The type of callback must be function"),
            "handler2 is invaild");
    }

    return RegisterCallback(env, jsThis, argv, callbackName);
}

napi_value NapiAudioLoopback::RegisterCallback(napi_env env, napi_value jsThis,
    napi_value *argv, const std::string &cbName)
{
    NapiAudioLoopback *napiLoopback = nullptr;
    napi_status status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napiLoopback));
    CHECK_AND_RETURN_RET_LOG(status == napi_ok,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_SYSTEM), "status error");
    CHECK_AND_RETURN_RET_LOG(napiLoopback != nullptr,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_NO_MEMORY), "napiLoopback is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiLoopback->loopback_ != nullptr, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_NO_MEMORY), "loopback_ is nullptr");

    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    if (!cbName.compare(STATUS_CHANGE_CALLBACK_NAME)) {
        result = RegisterLoopbackCallback(env, argv, cbName, napiLoopback);
    } else {
        bool unknownCallback = true;
        CHECK_AND_RETURN_RET_LOG(!unknownCallback, NapiAudioError::ThrowErrorAndReturn(env,
            NAPI_ERROR_INVALID_PARAM,
            "parameter verification failed: The param of type is not supported"), "loopback_ is nullptr");
    }

    return result;
}

napi_value NapiAudioLoopback::RegisterLoopbackCallback(napi_env env, napi_value *argv,
    const std::string &cbName, NapiAudioLoopback *napiLoopback)
{
    CHECK_AND_RETURN_RET_LOG(napiLoopback->callbackNapi_ != nullptr, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_NO_MEMORY), "callbackNapi_ is nullptr");

    std::shared_ptr<NapiAudioLoopbackCallback> cb =
        std::static_pointer_cast<NapiAudioLoopbackCallback>(napiLoopback->callbackNapi_);
    cb->SaveCallbackReference(cbName, argv[PARAM1]);
    if (cbName == STATUS_CHANGE_CALLBACK_NAME) {
        if (!cb->GetArStatusChangeTsfnFlag()) {
            cb->CreateArStatusChange(env);
        }
    }

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value NapiAudioLoopback::Off(napi_env env, napi_callback_info info)
{
    const size_t requireArgc = ARGS_TWO;
    size_t argc = ARGS_THREE;

    napi_value argv[requireArgc + 1] = {nullptr, nullptr, nullptr};
    napi_value jsThis = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &jsThis, nullptr);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_SYSTEM), "status error");
    CHECK_AND_RETURN_RET_LOG(argc <= requireArgc, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID, "mandatory parameters are left unspecified"), "argc is invaild");

    napi_valuetype eventType = napi_undefined;
    napi_typeof(env, argv[PARAM0], &eventType);
    CHECK_AND_RETURN_RET_LOG(eventType == napi_string, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID, "incorrect parameter types: The type of eventType must be string"),
        "eventType is invaild");

    std::string callbackName = NapiParamUtils::GetStringArgument(env, argv[PARAM0]);
    AUDIO_DEBUG_LOG("AudioLoopbackNapi: Off callbackName: %{public}s", callbackName.c_str());

    return UnregisterCallback(env, jsThis, argc, argv, callbackName);
}

napi_value NapiAudioLoopback::UnregisterCallback(napi_env env, napi_value jsThis, size_t argc, napi_value *argv,
    const std::string &cbName)
{
    NapiAudioLoopback *napiLoopback = nullptr;
    napi_status status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napiLoopback));
    CHECK_AND_RETURN_RET_LOG(status == napi_ok,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_SYSTEM), "status error");
    CHECK_AND_RETURN_RET_LOG(napiLoopback != nullptr,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_NO_MEMORY), "napiLoopback is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiLoopback->loopback_ != nullptr, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_NO_MEMORY), "loopback_ is nullptr");

    if (!cbName.compare(STATUS_CHANGE_CALLBACK_NAME)) {
        UnregisterLoopbackCallback(env, argc, cbName, argv, napiLoopback);
    } else {
        bool unknownCallback = true;
        CHECK_AND_RETURN_RET_LOG(!unknownCallback, NapiAudioError::ThrowErrorAndReturn(env,
            NAPI_ERR_INVALID_PARAM,
            "parameter verification failed: The param of type is not supported"), "cbName is invaild");
    }

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

template <typename T>
static void UnregisterAudioLoopbackSingletonCallbackTemplate(napi_env env, napi_value callback,
    const std::string &cbName, std::shared_ptr<T> cb,
    std::function<int32_t(std::shared_ptr<T> callbackPtr, napi_value callback)> removeFunction = nullptr)
{
    if (callback != nullptr) {
        CHECK_AND_RETURN_LOG(cb->ContainSameJsCallbackInner(cbName, callback), "callback not exists!");
    }
    cb->RemoveCallbackReference(cbName, env, callback);

    if (removeFunction == nullptr) {
        return;
    }
    int32_t ret = removeFunction(cb, callback);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "Unset of Loopback info change call failed");
    return;
}

void NapiAudioLoopback::UnregisterLoopbackCallback(napi_env env, size_t argc,
    const std::string &cbName, napi_value *argv, NapiAudioLoopback *napiLoopback)
{
    CHECK_AND_RETURN_LOG(napiLoopback->callbackNapi_ != nullptr, "napiLoopbackCallback is nullptr");

    std::shared_ptr<NapiAudioLoopbackCallback> cb =
        std::static_pointer_cast<NapiAudioLoopbackCallback>(napiLoopback->callbackNapi_);
    auto callback = GetCallback(argc, argv);
    UnregisterAudioLoopbackSingletonCallbackTemplate(env, callback, cbName, cb);
    AUDIO_DEBUG_LOG("UnregisterLoopbackCallback is successful");
}

napi_value NapiAudioLoopback::GetCallback(size_t argc, napi_value *argv)
{
    napi_value callback = nullptr;

    if (argc == ARGS_TWO) {
        callback = argv[PARAM1];
    }
    return callback;
}

bool NapiAudioLoopback::CheckContextStatus(std::shared_ptr<AudioLoopbackAsyncContext> context)
{
    CHECK_AND_RETURN_RET_LOG(context != nullptr, false, "context object is nullptr.");
    if (context->native == nullptr) {
        context->SignError(NAPI_ERR_SYSTEM);
        return false;
    }
    return true;
}

bool NapiAudioLoopback::CheckAudioLoopbackStatus(NapiAudioLoopback *napi,
    std::shared_ptr<AudioLoopbackAsyncContext> context)
{
    CHECK_AND_RETURN_RET_LOG(napi != nullptr, false, "napi object is nullptr.");
    if (napi->loopback_ == nullptr) {
        context->SignError(NAPI_ERR_SYSTEM);
        return false;
    }
    return true;
}

NapiAudioLoopback* NapiAudioLoopback::GetParamWithSync(const napi_env &env, napi_callback_info info,
    size_t &argc, napi_value *args)
{
    NapiAudioLoopback *napiLoopback = nullptr;
    napi_value jsThis = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok && jsThis != nullptr, nullptr, "status error");
    status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napiLoopback));
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, nullptr, "napi_unwrap failed");
    CHECK_AND_RETURN_RET_LOG(napiLoopback != nullptr, nullptr, "napiLoopback is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiLoopback->loopback_ != nullptr, nullptr, "loopback_ is nullptr");
    return napiLoopback;
}

napi_value NapiAudioLoopback::SetReverbPreset(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = {};
    NapiAudioLoopback *napiLoopback = GetParamWithSync(env, info, argc, argv);
    CHECK_AND_RETURN_RET_LOG(argc >= ARGS_ONE, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID, "mandatory parameters are left unspecified"), "argcCount invalid");
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_number, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID, "incorrect parameter types: The type of mode must be number"),
        "valueType invaild");
    int32_t preset;
    NapiParamUtils::GetValueInt32(env, preset, argv[PARAM0]);

    if (!NapiAudioEnum::IsLegalInputArgumentAudioLoopbackReverbPreset(preset)) {
        NapiAudioError::ThrowError(env, NAPI_ERR_INVALID_PARAM,
            "parameter verification failed: The param of mode must be enum AudioLoopbackReverbPreset");
        return result;
    }
    CHECK_AND_RETURN_RET_LOG(napiLoopback != nullptr,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_NO_MEMORY), "napiLoopback is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiLoopback->loopback_ != nullptr, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_NO_MEMORY), "loopback_ is nullptr");
    bool ret = napiLoopback->loopback_->SetReverbPreset(static_cast<AudioLoopbackReverbPreset>(preset));
    napi_get_boolean(env, ret, &result);
    return result;
}

napi_value NapiAudioLoopback::GetReverbPreset(napi_env env, napi_callback_info info)
{
    size_t argc = PARAM0;
    NapiAudioLoopback *napiLoopback = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(napiLoopback != nullptr,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_NO_MEMORY), "napiLoopback is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiLoopback->loopback_ != nullptr, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_NO_MEMORY), "loopback_ is nullptr");
    int32_t reverbPreset = static_cast<int32_t>(napiLoopback->loopback_->GetReverbPreset());
    napi_value result = nullptr;
    napi_status status = NapiParamUtils::SetValueInt32(env, reverbPreset, result);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "SetValueInt32 failed");
    return result;
}

napi_value NapiAudioLoopback::SetEqualizerPreset(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = {};
    NapiAudioLoopback *napiLoopback = GetParamWithSync(env, info, argc, argv);
    CHECK_AND_RETURN_RET_LOG(argc >= ARGS_ONE, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID, "mandatory parameters are left unspecified"), "argcCount invalid");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_number, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID, "incorrect parameter types: The type of mode must be number"),
        "valueType invaild");
    int32_t preset;
    NapiParamUtils::GetValueInt32(env, preset, argv[PARAM0]);

    if (!NapiAudioEnum::IsLegalInputArgumentAudioLoopbackEqualizerPreset(preset)) {
        NapiAudioError::ThrowError(env, NAPI_ERR_INVALID_PARAM,
            "parameter verification failed: The param of mode must be enum AudioLoopbackEqualizerPreset");
        return result;
    }
    CHECK_AND_RETURN_RET_LOG(napiLoopback != nullptr,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_NO_MEMORY), "napiLoopback is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiLoopback->loopback_ != nullptr, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_NO_MEMORY), "loopback_ is nullptr");
    bool ret = napiLoopback->loopback_->SetEqualizerPreset(static_cast<AudioLoopbackEqualizerPreset>(preset));
    napi_get_boolean(env, ret, &result);
    return result;
}

napi_value NapiAudioLoopback::GetEqualizerPreset(napi_env env, napi_callback_info info)
{
    size_t argc = PARAM0;
    NapiAudioLoopback *napiLoopback = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(napiLoopback != nullptr,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_NO_MEMORY), "napiLoopback is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiLoopback->loopback_ != nullptr, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_NO_MEMORY), "loopback_ is nullptr");
    int32_t reverbPreset = static_cast<int32_t>(napiLoopback->loopback_->GetEqualizerPreset());
    napi_value result = nullptr;
    napi_status status = NapiParamUtils::SetValueInt32(env, reverbPreset, result);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "SetValueInt32 failed");
    return result;
}
} // namespace AudioStandard
} // namespace OHOS
