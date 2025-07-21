/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#include "napi_audio_session_callback.h"
#include "napi_audio_session_state_callback.h"
#include "napi_audio_session_device_callback.h"
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
    if (nativeObject == nullptr) {
        AUDIO_WARNING_LOG("Native object is null");
        return;
    }
    auto obj = static_cast<NapiAudioSessionMgr *>(nativeObject);
    ObjectRefMap<NapiAudioSessionMgr>::DecreaseRef(obj);
    AUDIO_INFO_LOG("Decrease obj count");
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
        DECLARE_NAPI_FUNCTION("setAudioSessionScene", SetAudioSessionScene),
        DECLARE_NAPI_FUNCTION("getDefaultOutputDevice", GetDefaultOutputDevice),
        DECLARE_NAPI_FUNCTION("setDefaultOutputDevice", SetDefaultOutputDevice),
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

bool NapiAudioSessionMgr::CheckAudioSessionStatus(NapiAudioSessionMgr *napi,
    std::shared_ptr<AudioSessionMgrAsyncContext> context)
{
    CHECK_AND_RETURN_RET_LOG(napi != nullptr, false, "napi object is nullptr.");
    if (napi->audioSessionMngr_ == nullptr) {
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
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments",
            NAPI_ERR_INPUT_INVALID);
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
        if (napiSessionMgr == nullptr || napiSessionMgr->audioSessionMngr_ == nullptr) {
            context->SignError(NAPI_ERR_SYSTEM, "System error. Internal variable exception.");
            AUDIO_ERR_LOG("The napiSessionMgr or audioSessionMngr is nullptr");
            return;
        }
        context->intValue = napiSessionMgr->audioSessionMngr_->ActivateAudioSession(context->audioSessionStrategy);
        if (context->intValue != SUCCESS) {
            context->SignError(NAPI_ERR_SYSTEM, "System error. Set app volume fail.");
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
        NapiAudioError::ThrowError(env, "DeactivateAudioSession failed : no memory",
            NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }
    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioSessionMgr*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiSessionMgr = objectGuard.GetPtr();
        if (napiSessionMgr == nullptr || napiSessionMgr->audioSessionMngr_ == nullptr) {
            context->SignError(NAPI_ERR_SYSTEM, "System error. Internal variable exception.");
            AUDIO_ERR_LOG("The napiSessionMgr or audioSessionMngr is nullptr");
            return;
        }
        context->intValue = napiSessionMgr->audioSessionMngr_->DeactivateAudioSession();
        if (context->intValue != SUCCESS) {
            context->SignError(NAPI_ERR_SYSTEM, "System error. Set app volume fail.");
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
        NapiAudioError::ThrowError(env, "IsAudioSessionActivated failed : no memory",
            NAPI_ERR_NO_MEMORY);
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
    } else if (!cbName.compare(AUDIOSESSION_STATE_CALLBACK_NAME)) {
        NapiAudioSessionMgr *napiSessionMgr = nullptr;
        napi_status status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napiSessionMgr));
        if ((status != napi_ok) || (napiSessionMgr == nullptr) || (napiSessionMgr->audioSessionMngr_ == nullptr)) {
            AUDIO_ERR_LOG("NapiAudioSessionMgr can not get session mgr napi instance");
            NapiAudioError::ThrowError(env, NAPI_ERR_INVALID_PARAM, "can not get session mgr napi instance");
            return;
        }
        RegisterAudioSessionStateCallback(env, args, cbName, napiSessionMgr);
    } else if (!cbName.compare(AUDIOSESSION_DEVICE_CALLBACK_NAME)) {
        NapiAudioSessionMgr *napiSessionMgr = nullptr;
        napi_status status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napiSessionMgr));
        if ((status != napi_ok) || (napiSessionMgr == nullptr) || (napiSessionMgr->audioSessionMngr_ == nullptr)) {
            AUDIO_ERR_LOG("NapiAudioSessionMgr can not get session mgr napi instance");
            NapiAudioError::ThrowError(env, NAPI_ERR_INVALID_PARAM, "can not get session mgr napi instance");
            return;
        }
        RegisterAudioSessionDeviceCallback(env, args, cbName, napiSessionMgr);
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
    if (!cb->GetAudioSessionTsfnFlag()) {
        cb->CreateAudioSessionTsfn(env);
    }

    AUDIO_INFO_LOG("OnRendererStateChangeCallback is successful");
}

void NapiAudioSessionMgr::RegisterAudioSessionStateCallback(napi_env env, napi_value *args,
    const std::string &cbName, NapiAudioSessionMgr *napiSessionMgr)
{
    if (args[PARAM1] == nullptr) {
        AUDIO_ERR_LOG("OnAudioSessionStateChangeCallback failed, callback function is nullptr");
        NapiAudioError::ThrowError(env, NAPI_ERR_INVALID_PARAM, "callback function is nullptr");
        return;
    }

    std::lock_guard<std::mutex> lock(napiSessionMgr->sessionStateCbMutex_);
    CHECK_AND_RETURN_LOG(GetAudioSessionStateCallback(args[PARAM1], napiSessionMgr) == nullptr,
        "The callback function already registered.");

    std::shared_ptr<AudioSessionStateChangedCallback> stateChangedCallback =
        std::make_shared<NapiAudioSessionStateCallback>(env);
    if (stateChangedCallback == nullptr) {
        AUDIO_ERR_LOG("NapiAudioSessionMgr: Memory Allocation Failed!");
        NapiAudioError::ThrowError(env, NAPI_ERR_NO_MEMORY, "Memory Allocation Failed!");
        return;
    }

    int32_t ret = napiSessionMgr->audioSessionMngr_->SetAudioSessionStateChangeCallback(stateChangedCallback);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("SetAudioSessionStateChangeCallback is failed, ret = %{public}d", ret);
        NapiAudioError::ThrowError(env, NAPI_ERR_SYSTEM);
        return;
    }

    std::shared_ptr<NapiAudioSessionStateCallback> cb =
        std::static_pointer_cast<NapiAudioSessionStateCallback>(stateChangedCallback);
    napiSessionMgr->sessionStateCallbackList_.push_back(cb);
    cb->SaveCallbackReference(args[PARAM1]);
    if (!cb->GetAudioSessionStateTsfnFlag()) {
        cb->CreateAudioSessionStateTsfn(env);
    }

    AUDIO_INFO_LOG("OnAudioSessionStateChangeCallback is successful");
}

void NapiAudioSessionMgr::RegisterAudioSessionDeviceCallback(napi_env env, napi_value *args,
    const std::string &cbName, NapiAudioSessionMgr *napiSessionMgr)
{
    if (args[PARAM1] == nullptr) {
        AUDIO_ERR_LOG("OnAudioSessionDeviceCallback failed, callback function is nullptr");
        NapiAudioError::ThrowError(env, NAPI_ERR_INVALID_PARAM, "callback function is nullptr");
        return;
    }

    std::lock_guard<std::mutex> lock(napiSessionMgr->sessionDeviceCbMutex_);
    CHECK_AND_RETURN_LOG(GetAudioSessionDeviceCallback(args[PARAM1], napiSessionMgr) == nullptr,
        "The callback function already registered.");

    std::shared_ptr<AudioSessionCurrentDeviceChangedCallback> deviceChangedCallback =
        std::make_shared<NapiAudioSessionDeviceCallback>(env);
    if (deviceChangedCallback == nullptr) {
        AUDIO_ERR_LOG("NapiAudioSessionMgr: Memory Allocation Failed!");
        NapiAudioError::ThrowError(env, NAPI_ERR_NO_MEMORY, "Memory Allocation Failed!");
        return;
    }

    int32_t ret = napiSessionMgr->audioSessionMngr_->SetAudioSessionCurrentDeviceChangeCallback(deviceChangedCallback);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("RegisterAudioSessionDeviceCallback is failed, ret = %{public}d", ret);
        NapiAudioError::ThrowError(env, NAPI_ERR_SYSTEM);
        return;
    }

    std::shared_ptr<NapiAudioSessionDeviceCallback> cb =
        std::static_pointer_cast<NapiAudioSessionDeviceCallback>(deviceChangedCallback);
    napiSessionMgr->sessionDeviceCallbackList_.push_back(cb);
    cb->SaveCallbackReference(args[PARAM1]);
    if (!cb->GetAudioSessionDeviceTsfnFlag()) {
        cb->CreateAudioSessionDeviceTsfn(env);
    }

    AUDIO_INFO_LOG("RegisterAudioSessionDeviceCallback is successful");
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
        NAPI_ERR_INPUT_INVALID, "mandatory parameters are left unspecified"),
        "status for arguments error");

    napi_valuetype eventType = napi_undefined;
    napi_typeof(env, args[PARAM0], &eventType);
    CHECK_AND_RETURN_RET_LOG(eventType == napi_string, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID, "incorrect parameter types: The type of eventType must be string"),
        "eventType error");
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

void NapiAudioSessionMgr::UnregisterSessionStateCallback(napi_env env, napi_value jsThis)
{
    AUDIO_INFO_LOG("UnregisterCallback state");
    NapiAudioSessionMgr *napiSessionMgr = nullptr;
    napi_status status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napiSessionMgr));
    if ((status != napi_ok) || (napiSessionMgr == nullptr) || (napiSessionMgr->audioSessionMngr_ == nullptr)) {
        AUDIO_ERR_LOG("NapiAudioSessionMgr can not get session mgr napi instance");
        NapiAudioError::ThrowError(env, NAPI_ERR_INVALID_PARAM, "can not get session mgr napi instance");
        return;
    }

    std::lock_guard<std::mutex> lock(napiSessionMgr->sessionStateCbMutex_);
    CHECK_AND_RETURN_LOG(!napiSessionMgr->sessionStateCallbackList_.empty(),
        "Not register callback function, no need unregister.");

    int32_t ret = napiSessionMgr->audioSessionMngr_->UnsetAudioSessionStateChangeCallback();
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("UnsetAudioSessionStateChangeCallback Failed, ret = %{public}d", ret);
        NapiAudioError::ThrowError(env, NAPI_ERR_SYSTEM);
        return;
    }

    for (auto it = napiSessionMgr->sessionStateCallbackList_.rbegin();
        it != napiSessionMgr->sessionStateCallbackList_.rend(); ++it) {
        std::shared_ptr<NapiAudioSessionStateCallback> cb =
            std::static_pointer_cast<NapiAudioSessionStateCallback>(*it);
        cb.reset();
    }
    napiSessionMgr->sessionStateCallbackList_.clear();

    AUDIO_ERR_LOG("UnregisterSessionStateCallback Success");
}

void NapiAudioSessionMgr::UnregisterSessionStateCallbackCarryParam(
    napi_env env, napi_value jsThis, napi_value *args, size_t len)
{
    AUDIO_INFO_LOG("UnregisterCallback StateChanged.");
    if (args[PARAM1] == nullptr) {
        AUDIO_ERR_LOG("UnregisterSessionStateCallbackCarryParam failed, callback function is nullptr");
        NapiAudioError::ThrowError(env, NAPI_ERR_INVALID_PARAM, "callback function is nullptr");
        return;
    }

    NapiAudioSessionMgr *napiSessionMgr = nullptr;
    napi_status status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napiSessionMgr));
    if ((status != napi_ok) || (napiSessionMgr == nullptr) || (napiSessionMgr->audioSessionMngr_ == nullptr)) {
        AUDIO_ERR_LOG("NapiAudioSessionMgr can not get session mgr napi instance");
        NapiAudioError::ThrowError(env, NAPI_ERR_INVALID_PARAM, "can not get session mgr napi instance");
        return;
    }

    std::lock_guard<std::mutex> lock(napiSessionMgr->sessionStateCbMutex_);
    std::shared_ptr<NapiAudioSessionStateCallback> cb = GetAudioSessionStateCallback(args[PARAM1], napiSessionMgr);
    CHECK_AND_RETURN_LOG(cb != nullptr, "The callback function not registered.");
    std::shared_ptr<AudioSessionStateChangedCallback> stateChangedCallback =
        std::static_pointer_cast<AudioSessionStateChangedCallback>(cb);

    int32_t ret = napiSessionMgr->audioSessionMngr_->UnsetAudioSessionStateChangeCallback(stateChangedCallback);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("UnregisterSessionStateCallbackCarryParam Failed, ret = %{public}d", ret);
        NapiAudioError::ThrowError(env, NAPI_ERR_SYSTEM);
        return;
    }

    napiSessionMgr->sessionStateCallbackList_.remove(cb);
    cb.reset();

    AUDIO_ERR_LOG("UnregisterSessionStateCallbackCarryParam Success");
}

void NapiAudioSessionMgr::UnregisterSessionDeviceCallback(napi_env env, napi_value jsThis)
{
    AUDIO_INFO_LOG("UnregisterCallback device");
    NapiAudioSessionMgr *napiSessionMgr = nullptr;
    napi_status status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napiSessionMgr));
    if ((status != napi_ok) || (napiSessionMgr == nullptr) || (napiSessionMgr->audioSessionMngr_ == nullptr)) {
        AUDIO_ERR_LOG("NapiAudioSessionMgr can not get session mgr napi instance");
        NapiAudioError::ThrowError(env, NAPI_ERR_INVALID_PARAM, "can not get session mgr napi instance");
        return;
    }

    std::lock_guard<std::mutex> lock(napiSessionMgr->sessionDeviceCbMutex_);
    CHECK_AND_RETURN_LOG(!napiSessionMgr->sessionDeviceCallbackList_.empty(),
        "Not register callback function, no need unregister.");

    int32_t ret = napiSessionMgr->audioSessionMngr_->UnsetAudioSessionCurrentDeviceChangeCallback();
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("UnregisterSessionDeviceCallback is failed, ret = %{public}d", ret);
        NapiAudioError::ThrowError(env, NAPI_ERR_SYSTEM);
        return;
    }

    for (auto it = napiSessionMgr->sessionDeviceCallbackList_.rbegin();
        it != napiSessionMgr->sessionDeviceCallbackList_.rend(); ++it) {
        std::shared_ptr<NapiAudioSessionDeviceCallback> cb =
            std::static_pointer_cast<NapiAudioSessionDeviceCallback>(*it);
        cb.reset();
    }
    napiSessionMgr->sessionDeviceCallbackList_.clear();

    AUDIO_ERR_LOG("UnregisterSessionDeviceCallback Success");
}

void NapiAudioSessionMgr::UnregisterSessionDeviceCallbackCarryParam(
    napi_env env, napi_value jsThis, napi_value *args, size_t len)
{
    AUDIO_INFO_LOG("UnregisterCallback device changed.");
    if (args[PARAM1] == nullptr) {
        AUDIO_ERR_LOG("UnregisterSessionDeviceCallbackCarryParam failed, callback function is nullptr");
        NapiAudioError::ThrowError(env, NAPI_ERR_INVALID_PARAM, "callback function is nullptr");
        return;
    }

    NapiAudioSessionMgr *napiSessionMgr = nullptr;
    napi_status status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napiSessionMgr));
    if ((status != napi_ok) || (napiSessionMgr == nullptr) || (napiSessionMgr->audioSessionMngr_ == nullptr)) {
        AUDIO_ERR_LOG("NapiAudioSessionMgr can not get session mgr napi instance");
        NapiAudioError::ThrowError(env, NAPI_ERR_INVALID_PARAM, "can not get session mgr napi instance");
        return;
    }

    std::lock_guard<std::mutex> lock(napiSessionMgr->sessionDeviceCbMutex_);
    std::shared_ptr<NapiAudioSessionDeviceCallback> cb = GetAudioSessionDeviceCallback(args[PARAM1], napiSessionMgr);
    CHECK_AND_RETURN_LOG(cb != nullptr, "The callback function not registered.");
    std::shared_ptr<AudioSessionCurrentDeviceChangedCallback> deviceCallback =
        std::static_pointer_cast<AudioSessionCurrentDeviceChangedCallback>(cb);

    int32_t ret = napiSessionMgr->audioSessionMngr_->UnsetAudioSessionCurrentDeviceChangeCallback(deviceCallback);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("UnsetAudioSessionCurrentDeviceChangeCallback is failed, ret = %{public}d", ret);
        NapiAudioError::ThrowError(env, NAPI_ERR_SYSTEM);
        return;
    }

    napiSessionMgr->sessionDeviceCallbackList_.remove(cb);
    cb.reset();

    AUDIO_ERR_LOG("UnregisterSessionDeviceCallbackCarryParam Success");
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
    CHECK_AND_RETURN_RET_LOG(eventType == napi_string, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID, "incorrect parameter types: The type of eventType must be string"), "event error");
    std::string callbackName = NapiParamUtils::GetStringArgument(env, args[PARAM0]);
    if (!callbackName.compare(AUDIOSESSION_CALLBACK_NAME)) {
        napi_valuetype handler = napi_undefined;
        napi_typeof(env, args[PARAM1], &handler);
        if (handler == napi_function) {
            UnregisterCallbackCarryParam(env, jsThis, args, sizeof(args));
        } else {
            UnregisterCallback(env, jsThis);
        }
    } else if (!callbackName.compare(AUDIOSESSION_STATE_CALLBACK_NAME)) {
        napi_valuetype handler = napi_undefined;
        napi_typeof(env, args[PARAM1], &handler);
        if (handler == napi_function) {
            UnregisterSessionStateCallbackCarryParam(env, jsThis, args, sizeof(args));
        } else {
            UnregisterSessionStateCallback(env, jsThis);
        }
    } else if (!callbackName.compare(AUDIOSESSION_DEVICE_CALLBACK_NAME)) {
        napi_valuetype handler = napi_undefined;
        napi_typeof(env, args[PARAM1], &handler);
        if (handler == napi_function) {
            UnregisterSessionDeviceCallbackCarryParam(env, jsThis, args, sizeof(args));
        } else {
            UnregisterSessionDeviceCallback(env, jsThis);
        }
    } else {
        AUDIO_ERR_LOG("NapiAudioSessionMgr::No such callback supported");
        NapiAudioError::ThrowError(env, NAPI_ERR_INVALID_PARAM,
            "parameter verification failed: The param of type is not supported");
    }
    return undefinedResult;
}

napi_value NapiAudioSessionMgr::SetAudioSessionScene(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = ARGS_ONE;
    napi_value args[ARGS_ONE] = {};
    auto *napiSessionMgr = GetParamWithSync(env, info, argc, args);
    CHECK_AND_RETURN_RET_LOG(argc >= ARGS_ONE, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INVALID_PARAM, "argcCount invalid"), "argcCount invalid");

    int32_t scene;
    napi_status status = NapiParamUtils::GetValueInt32(env, scene, args[PARAM0]);
    CHECK_AND_RETURN_RET_LOG((status == napi_ok) && NapiAudioEnum::IsLegalInputArgumentSessionScene(scene),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
        "parameter verification failed: The param of scene must be enum AudioSessionScene"),
        "valueType invalid");

    CHECK_AND_RETURN_RET_LOG(napiSessionMgr != nullptr, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INVALID_PARAM, "can not get session"), "napiSessionMgr is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiSessionMgr->audioSessionMngr_ != nullptr, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INVALID_PARAM, "can not get session"), "audioSessionMngr_ is nullptr");

    int32_t ret = napiSessionMgr->audioSessionMngr_->SetAudioSessionScene(static_cast<AudioSessionScene>(scene));
    if (ret == ERR_NOT_SUPPORTED) {
        AUDIO_ERR_LOG("SetAudioSessionScene Failed, not supported ret = %{public}d", ret);
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_ILLEGAL_STATE);
    } else if (ret != SUCCESS) {
        AUDIO_ERR_LOG("SetAudioSessionScene Failed, ret = %{public}d", ret);
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_SYSTEM);
    }

    return result;
}

NapiAudioSessionMgr *NapiAudioSessionMgr::GetParamWithSync(const napi_env &env, napi_callback_info info,
    size_t &argc, napi_value *args)
{
    NapiAudioSessionMgr *napiSessionMgr = nullptr;
    napi_value jsThis = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr);
    CHECK_AND_RETURN_RET_LOG((status == napi_ok) && jsThis != nullptr, nullptr,
        "GetParamWithSync fail to napi_get_cb_info");

    status = napi_unwrap(env, jsThis, (void **)&napiSessionMgr);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, nullptr, "napi_unwrap failed");
    CHECK_AND_RETURN_RET_LOG(napiSessionMgr != nullptr && napiSessionMgr->audioSessionMngr_ != nullptr,
        napiSessionMgr, "GetParamWithSync fail to napi_unwrap");
    return napiSessionMgr;
}

napi_value NapiAudioSessionMgr::GetDefaultOutputDevice(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = PARAM0;
    auto *napiSessionMgr = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(argc == PARAM0, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INVALID_PARAM, "argcCount invalid"), "argcCount invalid");

    CHECK_AND_RETURN_RET_LOG(napiSessionMgr != nullptr, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INVALID_PARAM, "can not get session"), "napiSessionMgr is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiSessionMgr->audioSessionMngr_ != nullptr, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INVALID_PARAM, "can not get session"), "audioSessionMngr_ is nullptr");

    DeviceType deviceType = DeviceType::DEVICE_TYPE_INVALID;
    int32_t ret = napiSessionMgr->audioSessionMngr_->GetDefaultOutputDevice(deviceType);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("GetDefaultOutputDevice Failed, ret = %{public}d", ret);
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_ILLEGAL_STATE, "get deviceType state error");
        return result;
    }
    AUDIO_INFO_LOG("GetDefaultOutputDevice successful, deviceType = %{public}d.", static_cast<int32_t>(deviceType));

    NapiParamUtils::SetValueInt32(env, deviceType, result);
    return result;
}

napi_value NapiAudioSessionMgr::SetDefaultOutputDevice(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioSessionMgrAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("SetDefaultOutputDevice failed : no memory");
        NapiAudioError::ThrowError(env, "SetDefaultOutputDevice failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc == ARGS_ONE, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->deviceType, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok,
            "incorrect parameter types: The type of mode must be number", NAPI_ERR_INVALID_PARAM);
        NAPI_CHECK_ARGS_RETURN_VOID(context,
            NapiAudioEnum::IsLegalInputArgumentDefaultOutputDeviceType(context->deviceType),
            "parameter verification failed: The param of mode must be enum deviceType",
            NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);

    if ((context->status != napi_ok) && (context->errCode == NAPI_ERR_INVALID_PARAM)) {
        NapiAudioError::ThrowError(env, context->errCode, context->errMessage);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioSessionMgr*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiSessionMgr = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioSessionStatus(napiSessionMgr, context),
            "context object state is error.");
        DeviceType deviceType = static_cast<DeviceType>(context->deviceType);
        context->intValue = napiSessionMgr->audioSessionMngr_->SetDefaultOutputDevice(deviceType);
        if (context->intValue != SUCCESS) {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SetDefaultOutputDevice", executor, complete);
}

std::shared_ptr<NapiAudioSessionStateCallback> NapiAudioSessionMgr::GetAudioSessionStateCallback(
    napi_value argv, NapiAudioSessionMgr *napiSessionMgr)
{
    std::shared_ptr<NapiAudioSessionStateCallback> cb = nullptr;
    for (auto &iter : napiSessionMgr->sessionStateCallbackList_) {
        if (iter == nullptr) {
            continue;
        }

        if (iter->ContainSameJsCallback(argv)) {
            cb = iter;
        }
    }
    return cb;
}

std::shared_ptr<NapiAudioSessionDeviceCallback> NapiAudioSessionMgr::GetAudioSessionDeviceCallback(
    napi_value argv, NapiAudioSessionMgr *napiSessionMgr)
{
    std::shared_ptr<NapiAudioSessionDeviceCallback> cb = nullptr;
    for (auto &iter : napiSessionMgr->sessionDeviceCallbackList_) {
        if (iter == nullptr) {
            continue;
        }

        if (iter->ContainSameJsCallback(argv)) {
            cb = iter;
        }
    }
    return cb;
}

}  // namespace AudioStandard
}  // namespace OHOS
