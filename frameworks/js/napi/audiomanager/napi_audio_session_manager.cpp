/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "NapiAudioSessionMgr"
#endif

#include "napi_audio_error.h"
#include "napi_param_utils.h"
#include "napi_audio_enum.h"
#include "audio_errors.h"
#include "audio_info.h"
#include "napi_audio_session_callback.h"
#include "napi_audio_session_manager.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
using namespace HiviewDFX;
static __thread napi_ref g_sessionMgrConstructor = nullptr;

const std::string AUDIO_SESSION_MGR_NAPI_CLASS_NAME = "AudioSessionManager";

NapiAudioSessionMgr::NapiAudioSessionMgr()
    : env_(nullptr), audioSessionMngr_(nullptr) {}

NapiAudioSessionMgr::~NapiAudioSessionMgr() = default;

void NapiAudioSessionMgr::Destructor(napi_env env, void *nativeObject, void *finalizeHint)
{
    if (nativeObject != nullptr) {
        auto obj = static_cast<NapiAudioSessionMgr *>(nativeObject);
        ObjectRefMap<NapiAudioSessionMgr>::DecreaseRef(obj);
    }
    AUDIO_INFO_LOG("Destructor is successful");
}

napi_value NapiAudioSessionMgr::Construct(napi_env env, napi_callback_info info)
{
    AUDIO_DEBUG_LOG("Construct");
    napi_status status;
    napi_value result = nullptr;
    NapiParamUtils::GetUndefinedValue(env);

    size_t argc = ARGS_TWO;
    napi_value argv[ARGS_TWO] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    unique_ptr<NapiAudioSessionMgr> napiSessionMgr = make_unique<NapiAudioSessionMgr>();
    CHECK_AND_RETURN_RET_LOG(napiSessionMgr != nullptr, result, "No memory");

    napiSessionMgr->env_ = env;
    napiSessionMgr->audioSessionMngr_ = AudioSessionManager::GetInstance();
    ObjectRefMap<NapiAudioSessionMgr>::Insert(napiSessionMgr.get());

    status = napi_wrap(env, thisVar, static_cast<void*>(napiSessionMgr.get()),
        NapiAudioSessionMgr::Destructor, nullptr, nullptr);
    if (status != napi_ok) {
        ObjectRefMap<NapiAudioSessionMgr>::Erase(napiSessionMgr.get());
        return result;
    }
    napiSessionMgr.release();
    return thisVar;
}

napi_value NapiAudioSessionMgr::Init(napi_env env, napi_value exports)
{
    napi_status status;
    napi_value constructor;
    napi_value result = nullptr;
    const int32_t refCount = ARGS_ONE;
    napi_get_undefined(env, &result);

    napi_property_descriptor audio_session_mgr_properties[] = {
        DECLARE_NAPI_FUNCTION("on", On),
        DECLARE_NAPI_FUNCTION("off", Off),
        DECLARE_NAPI_FUNCTION("activateAudioSession", ActivateAudioSession),
        DECLARE_NAPI_FUNCTION("deactivateAudioSession", DeactivateAudioSession),
        DECLARE_NAPI_FUNCTION("isAudioSessionActivated", IsAudioSessionActivated),
    };

    status = napi_define_class(env, AUDIO_SESSION_MGR_NAPI_CLASS_NAME.c_str(), NAPI_AUTO_LENGTH, Construct, nullptr,
        sizeof(audio_session_mgr_properties) / sizeof(audio_session_mgr_properties[PARAM0]),
        audio_session_mgr_properties, &constructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_define_class fail");

    status = napi_create_reference(env, constructor, refCount, &g_sessionMgrConstructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_create_reference fail");
    status = napi_set_named_property(env, exports, AUDIO_SESSION_MGR_NAPI_CLASS_NAME.c_str(), constructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_set_named_property fail");
    return exports;
}

napi_value NapiAudioSessionMgr::CreateSessionManagerWrapper(napi_env env)
{
    napi_status status;
    napi_value result = nullptr;
    napi_value constructor;

    status = napi_get_reference_value(env, g_sessionMgrConstructor, &constructor);
    if (status != napi_ok) {
        AUDIO_ERR_LOG("Failed in CreateSessionManagerWrapper, %{public}d", status);
        goto fail;
    }
    status = napi_new_instance(env, constructor, PARAM0, nullptr, &result);
    if (status != napi_ok) {
        AUDIO_ERR_LOG("napi_new_instance failed, status:%{public}d", status);
        goto fail;
    }
    return result;

fail:
    napi_get_undefined(env, &result);
    return result;
}

bool NapiAudioSessionMgr::CheckContextStatus(std::shared_ptr<AudioSessionMgrAsyncContext> context)
{
    CHECK_AND_RETURN_RET_LOG(context != nullptr, false, "context object is nullptr.");
    if (context->native == nullptr) {
        context->SignError(NAPI_ERR_SYSTEM);
        return false;
    }
    return true;
}

napi_value NapiAudioSessionMgr::ActivateAudioSession(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioSessionMgrAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("ActivateAudioSession failed : no memory");
        NapiAudioError::ThrowError(env, "ActivateAudioSession failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetAudioSessionStrategy(env, context->audioSessionStrategy, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "getAudioSessionStrategy failed",
            NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);

    if ((context->status != napi_ok) && (context->errCode == NAPI_ERR_INPUT_INVALID)) {
        NapiAudioError::ThrowError(env, context->errCode, context->errMessage);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioSessionMgr*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiSessionMgr = objectGuard.GetPtr();
        context->intValue = napiSessionMgr->audioSessionMngr_->ActivateAudioSession(context->audioSessionStrategy);
        if (context->intValue != SUCCESS) {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueInt32(env, context->intValue, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "ActivateAudioSession", executor, complete);
}

napi_value NapiAudioSessionMgr::DeactivateAudioSession(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioSessionMgrAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("DeactivateAudioSession failed : no memory");
        NapiAudioError::ThrowError(env, "DeactivateAudioSession failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }
    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioSessionMgr*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiSessionMgr = objectGuard.GetPtr();
        context->intValue = napiSessionMgr->audioSessionMngr_->DeactivateAudioSession();
        if (context->intValue != SUCCESS) {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueInt32(env, context->intValue, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "DeactivateAudioSession", executor, complete);
}

napi_value NapiAudioSessionMgr::IsAudioSessionActivated(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    auto context = std::make_shared<AudioSessionMgrAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("IsAudioSessionActivated failed : no memory");
        NapiAudioError::ThrowError(env, "IsAudioSessionActivated failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }
    context->GetCbInfo(env, info);

    CHECK_AND_RETURN_RET_LOG(CheckContextStatus(context), result,  "context object state is error.");
    auto obj = reinterpret_cast<NapiAudioSessionMgr*>(context->native);
    ObjectRefMap objectGuard(obj);
    auto *napiSessionMgr = objectGuard.GetPtr();
    if (napiSessionMgr == nullptr || napiSessionMgr->audioSessionMngr_ == nullptr) {
        AUDIO_ERR_LOG("The napiSessionMgr or audioSessionMngr is nullptr");
        return nullptr;
    }
    context->isActive = napiSessionMgr->audioSessionMngr_->IsAudioSessionActivated();
    NapiParamUtils::SetValueBoolean(env, context->isActive, result);
    return result;
}

void NapiAudioSessionMgr::RegisterCallback(napi_env env, napi_value jsThis,
    napi_value *args, const std::string &cbName)
{
    if (!cbName.compare(AUDIOSESSION_CALLBACK_NAME)) {
        NapiAudioSessionMgr *napiSessionMgr = nullptr;
        napi_status status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napiSessionMgr));
        CHECK_AND_RETURN_LOG((status == napi_ok) && (napiSessionMgr != nullptr) &&
            (napiSessionMgr->audioSessionMngr_ != nullptr), "Failed to retrieve session mgr napi instance.");
        RegisterAudioSessionCallback(env, args, cbName, napiSessionMgr);
    } else {
        AUDIO_ERR_LOG("NapiAudioSessionMgr::No such callback supported");
        NapiAudioError::ThrowError(env, NAPI_ERR_INVALID_PARAM,
            "parameter verification failed: The param of type is not supported");
    }
}

void NapiAudioSessionMgr::RegisterAudioSessionCallback(napi_env env, napi_value *args,
    const std::string &cbName, NapiAudioSessionMgr *napiSessionMgr)
{
    if (!napiSessionMgr->audioSessionCallbackNapi_) {
        napiSessionMgr->audioSessionCallbackNapi_ = std::make_shared<NapiAudioSessionCallback>(env);
        CHECK_AND_RETURN_LOG(napiSessionMgr->audioSessionCallbackNapi_ != nullptr,
            "NapiAudioSessionMgr: Memory Allocation Failed !!");

        int32_t ret =
            napiSessionMgr->audioSessionMngr_->SetAudioSessionCallback(napiSessionMgr->audioSessionCallbackNapi_);
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "Registering of AudioSessionDeactiveEvent Callback Failed");
    }

    std::shared_ptr<NapiAudioSessionCallback> cb =
        std::static_pointer_cast<NapiAudioSessionCallback>(napiSessionMgr->audioSessionCallbackNapi_);
    cb->SaveCallbackReference(args[PARAM1]);

    AUDIO_INFO_LOG("OnRendererStateChangeCallback is successful");
}

napi_value NapiAudioSessionMgr::On(napi_env env, napi_callback_info info)
{
    const size_t requireArgc = ARGS_TWO;
    size_t argc = ARGS_THREE;

    napi_value undefinedResult = nullptr;
    napi_get_undefined(env, &undefinedResult);

    napi_value args[requireArgc + PARAM1] = {nullptr, nullptr, nullptr};
    napi_value jsThis = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok && argc == requireArgc, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID, "mandatory parameters are left unspecified"), "status for arguments error");

    napi_valuetype eventType = napi_undefined;
    napi_typeof(env, args[PARAM0], &eventType);
    CHECK_AND_RETURN_RET_LOG(eventType == napi_string, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
        "incorrect parameter types: The type of eventType must be string"), "eventType error");
    std::string callbackName = NapiParamUtils::GetStringArgument(env, args[PARAM0]);
    AUDIO_DEBUG_LOG("AudioStreamMgrNapi: On callbackName: %{public}s", callbackName.c_str());

    napi_valuetype handler = napi_undefined;
    napi_typeof(env, args[PARAM1], &handler);
    CHECK_AND_RETURN_RET_LOG(
        handler == napi_function, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
        "incorrect parameter types: The type of handler must be function"), "handler is invalid");
    RegisterCallback(env, jsThis, args, callbackName);
    return undefinedResult;
}

void NapiAudioSessionMgr::UnregisterCallback(napi_env env, napi_value jsThis)
{
    AUDIO_INFO_LOG("UnregisterCallback");
    NapiAudioSessionMgr *napiSessionMgr = nullptr;
    napi_status status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napiSessionMgr));
    CHECK_AND_RETURN_LOG((status == napi_ok) && (napiSessionMgr != nullptr) &&
        (napiSessionMgr->audioSessionMngr_ != nullptr), "Failed to retrieve session mgr  instance.");

    int32_t ret = napiSessionMgr->audioSessionMngr_->UnsetAudioSessionCallback();
    if (ret) {
        AUDIO_ERR_LOG("Unset AudioSessionCallback Failed");
        return;
    }
    if (napiSessionMgr->audioSessionCallbackNapi_ != nullptr) {
        napiSessionMgr->audioSessionCallbackNapi_.reset();
        napiSessionMgr->audioSessionCallbackNapi_ = nullptr;
    }
    AUDIO_ERR_LOG("Unset AudioSessionCallback Success");
}

void NapiAudioSessionMgr::UnregisterCallbackCarryParam(napi_env env, napi_value jsThis, napi_value *args, size_t len)
{
    AUDIO_INFO_LOG("UnregisterCallback");
    NapiAudioSessionMgr *napiSessionMgr = nullptr;
    napi_status status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napiSessionMgr));
    CHECK_AND_RETURN_LOG((status == napi_ok) && (napiSessionMgr != nullptr) &&
        (napiSessionMgr->audioSessionMngr_ != nullptr), "Failed to retrieve session mgr  instance.");
    if (!napiSessionMgr->audioSessionCallbackNapi_) {
        napiSessionMgr->audioSessionCallbackNapi_ = std::make_shared<NapiAudioSessionCallback>(env);
        CHECK_AND_RETURN_LOG(napiSessionMgr->audioSessionCallbackNapi_ != nullptr,
            "Memory Allocation Failed !!");
        int32_t ret =
            napiSessionMgr->audioSessionMngr_->UnsetAudioSessionCallback(napiSessionMgr->audioSessionCallbackNapi_);
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "Unregister Callback CarryParam Failed");
    }
    std::shared_ptr<NapiAudioSessionCallback> cb =
        std::static_pointer_cast<NapiAudioSessionCallback>(napiSessionMgr->audioSessionCallbackNapi_);
    cb->SaveCallbackReference(args[PARAM0]);
    AUDIO_ERR_LOG("Unset AudioSessionCallback Success");
}

napi_value NapiAudioSessionMgr::Off(napi_env env, napi_callback_info info)
{
    const size_t requireArgc = ARGS_ONE;
    size_t argc = PARAM2;

    napi_value undefinedResult = nullptr;
    napi_get_undefined(env, &undefinedResult);

    napi_value args[requireArgc + PARAM1] = {nullptr, nullptr};
    napi_value jsThis = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr);
    if (status != napi_ok || argc < requireArgc) {
        AUDIO_ERR_LOG("Off fail to napi_get_cb_info/Requires min 1 parameters");
        NapiAudioError::ThrowError(env, NAPI_ERR_INPUT_INVALID, "mandatory parameters are left unspecified");
        return undefinedResult;
    }

    napi_valuetype eventType = napi_undefined;
    napi_typeof(env, args[PARAM0], &eventType);
    CHECK_AND_RETURN_RET_LOG(eventType == napi_string, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
        "incorrect parameter types: The type of eventType must be string"), "event error");
    std::string callbackName = NapiParamUtils::GetStringArgument(env, args[PARAM0]);
    if (!callbackName.compare(AUDIOSESSION_CALLBACK_NAME)) {
        napi_valuetype handler = napi_undefined;
        napi_typeof(env, args[PARAM1], &handler);
        if (handler == napi_function) {
            UnregisterCallbackCarryParam(env, jsThis, args, sizeof(args));
        } else {
            UnregisterCallback(env, jsThis);
        }
    } else {
        AUDIO_ERR_LOG("NapiAudioSessionMgr::No such callback supported");
        NapiAudioError::ThrowError(env, NAPI_ERR_INVALID_PARAM,
            "parameter verification failed: The param of type is not supported");
    }
    return undefinedResult;
}
}  // namespace AudioStandard
}  // namespace OHOS
