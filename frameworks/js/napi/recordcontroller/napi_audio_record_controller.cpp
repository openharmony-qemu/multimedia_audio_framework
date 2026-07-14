/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LOG_TAG
#define LOG_TAG "NapiAudioRecordController"
#endif

#include "napi_audio_record_controller.h"
#include "napi_audio_enum.h"
#include "napi_audio_error.h"
#include "napi_param_utils.h"
#include "audio_errors.h"
#include "audio_manager_log.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {
static napi_ref g_recordControllerConstructor = nullptr;

NapiAudioRecordController::NapiAudioRecordController()
{
    AUDIO_DEBUG_LOG("NapiAudioRecordController: instance create");
}

NapiAudioRecordController::~NapiAudioRecordController()
{
    AUDIO_DEBUG_LOG("NapiAudioRecordController: instance destroy");
}

napi_value NapiAudioRecordController::Init(napi_env env, napi_value exports)
{
    napi_status status;
    napi_value constructor;
    napi_value result = nullptr;
    const int32_t refCount = 1;

    napi_property_descriptor static_prop[] = {
        DECLARE_NAPI_STATIC_FUNCTION("createAudioRecordControlManager", CreateAudioRecordController),
    };

    status = InitNapiAudioRecordController(env, constructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "InitNapiAudioRecordController fail");

    status = napi_create_reference(env, constructor, refCount, &g_recordControllerConstructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_create_reference fail");
    status = napi_set_named_property(env, exports, "AudioRecordingManager", constructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_set_named_property fail");
    status = napi_define_properties(env, exports, sizeof(static_prop) / sizeof(static_prop[0]), static_prop);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_define_properties fail");

    return exports;
}

napi_status NapiAudioRecordController::InitNapiAudioRecordController(napi_env env, napi_value &constructor)
{
    napi_property_descriptor audio_record_controller_properties[] = {
        DECLARE_NAPI_FUNCTION("enableSystemRecordController", EnableSystemRecordController),
        DECLARE_NAPI_FUNCTION("onSystemRecordControllerEnabledChange", OnSystemRecordControllerEnabledChange),
        DECLARE_NAPI_FUNCTION("offSystemRecordControllerEnabledChange", OffSystemRecordControllerEnabledChange),
    };

    napi_status status = napi_define_class(env, "AudioRecordingManager", NAPI_AUTO_LENGTH,
        Construct, nullptr,
        sizeof(audio_record_controller_properties) / sizeof(audio_record_controller_properties[0]),
        audio_record_controller_properties, &constructor);
    return status;
}

NapiAudioRecordController* NapiAudioRecordController::GetParamWithSync(const napi_env &env,
    napi_callback_info info, size_t &argc, napi_value *args)
{
    napi_value jsThis = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok && jsThis != nullptr, nullptr, "napi_get_cb_info fail");

    NapiAudioRecordController *napiRecordController = nullptr;
    status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napiRecordController));
    CHECK_AND_RETURN_RET_LOG(status == napi_ok && napiRecordController != nullptr, nullptr, "napi_unwrap fail");
    return napiRecordController;
}

void NapiAudioRecordController::Destructor(napi_env env, void *nativeObject, void *finalizeHint)
{
    auto *napiRecordController = static_cast<NapiAudioRecordController *>(nativeObject);
    ObjectRefMap<NapiAudioRecordController>::Erase(napiRecordController);
    delete napiRecordController;
    napiRecordController = nullptr;
}

napi_value NapiAudioRecordController::Construct(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value jsThis;
    size_t argCount = 0;

    status = napi_get_cb_info(env, info, &argCount, nullptr, &jsThis, nullptr);
    if (status == napi_ok) {
        std::unique_ptr<NapiAudioRecordController> napiRecordController = std::make_unique<NapiAudioRecordController>();
        if (napiRecordController != nullptr) {
            ObjectRefMap<NapiAudioRecordController>::Insert(napiRecordController.get());
            napiRecordController->audioMngr_ = AudioSystemManager::GetInstance();

            status = napi_wrap(env, jsThis, static_cast<void *>(napiRecordController.get()),
                NapiAudioRecordController::Destructor, nullptr, nullptr);
            if (status == napi_ok) {
                (void)napiRecordController.release();
                return jsThis;
            }
            ObjectRefMap<NapiAudioRecordController>::Erase(napiRecordController.get());
        }
    }
    napi_value undefinedResult = nullptr;
    napi_get_undefined(env, &undefinedResult);
    return undefinedResult;
}

napi_value NapiAudioRecordController::CreateAudioRecordControllerWrapper(napi_env env)
{
    napi_status status;
    napi_value result = nullptr;
    napi_value constructor;

    status = napi_get_reference_value(env, g_recordControllerConstructor, &constructor);
    if (status != napi_ok) {
        napi_get_undefined(env, &result);
        return result;
    }

    status = napi_new_instance(env, constructor, 0, nullptr, &result);
    if (status != napi_ok) {
        napi_get_undefined(env, &result);
        return result;
    }

    return result;
}

napi_value NapiAudioRecordController::CreateAudioRecordController(napi_env env, napi_callback_info info)
{
    AUDIO_DEBUG_LOG("CreateAudioRecordController");
    return CreateAudioRecordControllerWrapper(env);
}

bool NapiAudioRecordController::CheckContextStatus(std::shared_ptr<RecordControllerAsyncContext> context)
{
    CHECK_AND_RETURN_RET_LOG(context != nullptr, false, "context object is nullptr.");
    if (context->native == nullptr) {
        context->SignError(NAPI_ERR_SYSTEM);
        AUDIO_ERR_LOG("context object state is error.");
        return false;
    }
    return true;
}

bool NapiAudioRecordController::CheckAudioRecordControllerStatus(NapiAudioRecordController *napi,
    std::shared_ptr<RecordControllerAsyncContext> context)
{
    CHECK_AND_RETURN_RET_LOG(napi != nullptr, false, "napi object is nullptr.");
    if (napi->audioMngr_ == nullptr) {
        context->SignError(NAPI_ERR_SYSTEM);
        AUDIO_ERR_LOG("audioMngr_ is nullptr.");
        return false;
    }
    return true;
}

napi_value NapiAudioRecordController::EnableSystemRecordController(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<RecordControllerAsyncContext>();
    if (context == nullptr) {
        NapiAudioError::ThrowError(env, "EnableSystemRecordController failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_TWO, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueBoolean(env, context->show, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get show failed", NAPI_ERR_INVALID_PARAM);

        SystemRecordControllerConfig config;
        napi_value optionsObj = argv[PARAM1];
        napi_value sourceTypeValue = nullptr;
        napi_status status = napi_get_named_property(env, optionsObj, "sourceType", &sourceTypeValue);
        NAPI_CHECK_ARGS_RETURN_VOID(context, status == napi_ok, "get sourceType failed", NAPI_ERR_INVALID_PARAM);
        int32_t sourceTypeInt = 0;
        status = NapiParamUtils::GetValueInt32(env, sourceTypeInt, sourceTypeValue);
        NAPI_CHECK_ARGS_RETURN_VOID(context, status == napi_ok, "get sourceType value failed", NAPI_ERR_INVALID_PARAM);
        if (sourceTypeInt != SOURCE_TYPE_MIC && sourceTypeInt != SOURCE_TYPE_CAMCORDER &&
            sourceTypeInt != SOURCE_TYPE_LIVE) {
            NAPI_CHECK_ARGS_RETURN_VOID(context, false, "invalid sourceType", NAPI_ERR_INVALID_PARAM);
        }
        config.sourceType = static_cast<SourceType>(sourceTypeInt);
        context->config = config;
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioRecordController *>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiRecordController = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioRecordControllerStatus(napiRecordController, context),
            "record controller object state is error.");

        int32_t ret = napiRecordController->audioMngr_->EnableSystemRecordController(
            context->show, context->config);
        if (ret != SUCCESS) {
            context->SignError(ret);
        }
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };

    return NapiAsyncWork::Enqueue(env, context, "EnableSystemRecordController", executor, complete);
}

napi_value NapiAudioRecordController::OnSystemRecordControllerEnabledChange(napi_env env, napi_callback_info info)
{
    AUDIO_DEBUG_LOG("OnSystemRecordControllerEnabledChange");
    napi_value undefinedResult = nullptr;
    napi_get_undefined(env, &undefinedResult);

    CHECK_AND_RETURN_RET(PermissionUtil::VerifySelfPermission(),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED, "No system permission"));

    const size_t minArgc = ARGS_ONE;
    size_t argc = ARGS_TWO;
    napi_value args[ARGS_TWO] = {nullptr, nullptr};
    napi_value jsThis = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr);
    if (status != napi_ok || argc < minArgc) {
        NapiAudioError::ThrowError(env, NAPI_ERR_INPUT_INVALID, "mandatory parameters are left unspecified");
        return undefinedResult;
    }

    napi_valuetype handler = napi_undefined;
    if (napi_typeof(env, args[PARAM0], &handler) != napi_ok || handler != napi_function) {
        NapiAudioError::ThrowError(env, NAPI_ERR_INPUT_INVALID,
            "incorrect parameter types: The type of callback must be function");
        return undefinedResult;
    }

    NapiAudioRecordController *napiRecordController = nullptr;
    status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napiRecordController));
    CHECK_AND_RETURN_RET_LOG(status == napi_ok && napiRecordController != nullptr, undefinedResult,
        "napi_unwrap fail");

    return RegisterRecordControllerEnabledChangeCallback(env, args, napiRecordController);
}

napi_value NapiAudioRecordController::RegisterRecordControllerEnabledChangeCallback(napi_env env,
    napi_value *args, NapiAudioRecordController *napiRecordController)
{
    if (!napiRecordController->recordControllerChangeCallbackNapi_) {
        napiRecordController->recordControllerChangeCallbackNapi_ =
            std::make_shared<NapiRecordControllerChangeCallback>(env);
        if (!napiRecordController->recordControllerChangeCallbackNapi_) {
            AUDIO_ERR_LOG("Memory Allocation Failed !!");
            NapiAudioError::ThrowError(env, "Memory Allocation Failed", NAPI_ERR_NO_MEMORY);
            napi_value result = nullptr;
            napi_get_undefined(env, &result);
            return result;
        }
        int32_t ret = napiRecordController->audioMngr_->SetRecordControllerEnabledChangeCallback(
            napiRecordController->recordControllerChangeCallbackNapi_);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("Registering RecordController Enabled Change Callback Failed");
            napiRecordController->recordControllerChangeCallbackNapi_ = nullptr;
            NapiAudioError::ThrowError(env, NAPI_ERR_SYSTEM, "Registering callback failed");
            napi_value result = nullptr;
            napi_get_undefined(env, &result);
            return result;
        }
    }
    std::shared_ptr<NapiRecordControllerChangeCallback> cb =
        std::static_pointer_cast<NapiRecordControllerChangeCallback>(
            napiRecordController->recordControllerChangeCallbackNapi_);
    cb->SaveCallbackReference(RECORD_CONTROLLER_ENABLED_CHANGE_CALLBACK_NAME, args[PARAM0]);
    if (!cb->GetRecordControllerChangeTsfnFlag()) {
        cb->CreateRecordControllerChangeTsfn(env);
    }
    AUDIO_DEBUG_LOG("On SetRecordControllerEnabledChangeCallback is successful");
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value NapiAudioRecordController::OffSystemRecordControllerEnabledChange(napi_env env, napi_callback_info info)
{
    AUDIO_DEBUG_LOG("OffSystemRecordControllerEnabledChange");
    napi_value undefinedResult = nullptr;
    napi_get_undefined(env, &undefinedResult);

    CHECK_AND_RETURN_RET(PermissionUtil::VerifySelfPermission(),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED, "No system permission"));

    size_t argc = ARGS_TWO;
    napi_value args[ARGS_TWO] = {nullptr, nullptr};
    napi_value jsThis = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr);
    if (status != napi_ok) {
        NapiAudioError::ThrowError(env, NAPI_ERR_INPUT_INVALID, "mandatory parameters are left unspecified");
        return undefinedResult;
    }

    if (argc > ARGS_ONE) {
        NapiAudioError::ThrowError(env, NAPI_ERR_INPUT_INVALID, "too many parameters");
        return undefinedResult;
    }

    if (argc == ARGS_ONE) {
        napi_valuetype handler = napi_undefined;
        if (napi_typeof(env, args[PARAM0], &handler) != napi_ok || handler != napi_function) {
            NapiAudioError::ThrowError(env, NAPI_ERR_INPUT_INVALID,
                "incorrect parameter types: The type of callback must be function");
            return undefinedResult;
        }
    }

    NapiAudioRecordController *napiRecordController = nullptr;
    status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napiRecordController));
    CHECK_AND_RETURN_RET_LOG(status == napi_ok && napiRecordController != nullptr, undefinedResult,
        "napi_unwrap fail");

    UnregisterRecordControllerEnabledChangeCallback(env, napiRecordController, argc, args);
    return undefinedResult;
}

void NapiAudioRecordController::UnregisterRecordControllerEnabledChangeCallback(napi_env env,
    NapiAudioRecordController *napiRecordController, size_t argc, napi_value *args)
{
    CHECK_AND_RETURN_LOG(napiRecordController->recordControllerChangeCallbackNapi_ != nullptr,
        "recordControllerChangeCallbackNapi is null");
    std::shared_ptr<NapiRecordControllerChangeCallback> cb =
        std::static_pointer_cast<NapiRecordControllerChangeCallback>(
            napiRecordController->recordControllerChangeCallbackNapi_);

    napi_value callback = nullptr;
    if (argc == ARGS_ONE) {
        callback = args[PARAM0];
        CHECK_AND_RETURN_LOG(cb->IsSameCallback(callback),
            "The callback need to be unregistered is not the same as the registered callback");
    }
    int32_t ret = napiRecordController->audioMngr_->UnsetRecordControllerEnabledChangeCallback(
        napiRecordController->recordControllerChangeCallbackNapi_);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "UnregisterRecordControllerEnabledChangeCallback failed");
    cb->RemoveCallbackReference(callback);
    napiRecordController->recordControllerChangeCallbackNapi_ = nullptr;
    AUDIO_INFO_LOG("UnregisterRecordControllerEnabledChangeCallback success");
}
} // namespace AudioStandard
} // namespace OHOS