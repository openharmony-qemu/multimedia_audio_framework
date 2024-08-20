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
#define LOG_TAG "NapiAudioSpatializationManager"
#endif

#include "napi_audio_spatialization_manager.h"

#include <vector>
#include "napi_audio_error.h"
#include "napi_param_utils.h"
#include "napi_audio_enum.h"
#include "audio_errors.h"
#include "audio_manager_log.h"
#include "audio_utils.h"
#include "napi_audio_spatialization_manager_callback.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
using namespace HiviewDFX;
static __thread napi_ref g_spatializationManagerConstructor = nullptr;
NapiAudioSpatializationManager::NapiAudioSpatializationManager()
    : audioSpatializationMngr_(nullptr), env_(nullptr) {}

NapiAudioSpatializationManager::~NapiAudioSpatializationManager() = default;

bool NapiAudioSpatializationManager::CheckContextStatus(std::shared_ptr<AudioSpatializationManagerAsyncContext> context)
{
    CHECK_AND_RETURN_RET_LOG(context != nullptr, false, "context object is nullptr.");
    if (context->native == nullptr) {
        context->SignError(NAPI_ERR_SYSTEM);
        AUDIO_ERR_LOG("context object state is error.");
        return false;
    }
    return true;
}

bool NapiAudioSpatializationManager::CheckAudioSpatializationManagerStatus(NapiAudioSpatializationManager *napi,
    std::shared_ptr<AudioSpatializationManagerAsyncContext> context)
{
    CHECK_AND_RETURN_RET_LOG(napi != nullptr, false, "napi object is nullptr.");
    if (napi->audioSpatializationMngr_ == nullptr) {
        context->SignError(NAPI_ERR_SYSTEM);
        AUDIO_ERR_LOG("audioSpatializationMngr_ is nullptr.");
        return false;
    }
    return true;
}

void NapiAudioSpatializationManager::Destructor(napi_env env, void *nativeObject, void *finalizeHint)
{
    if (nativeObject != nullptr) {
        auto obj = static_cast<NapiAudioSpatializationManager *>(nativeObject);
        ObjectRefMap<NapiAudioSpatializationManager>::DecreaseRef(obj);
    }
    AUDIO_INFO_LOG("Destructor is successful");
}

napi_value NapiAudioSpatializationManager::Construct(napi_env env, napi_callback_info info)
{
    AUDIO_INFO_LOG("Construct");
    napi_status status;
    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    size_t argc = ARGS_TWO;
    napi_value argv[ARGS_TWO] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    unique_ptr<NapiAudioSpatializationManager> napiAudioSpatializationManager =
        make_unique<NapiAudioSpatializationManager>();
    CHECK_AND_RETURN_RET_LOG(napiAudioSpatializationManager != nullptr, result, "No memory");

    napiAudioSpatializationManager->audioSpatializationMngr_ = AudioSpatializationManager::GetInstance();
    napiAudioSpatializationManager->env_ = env;

    ObjectRefMap<NapiAudioSpatializationManager>::Insert(napiAudioSpatializationManager.get());
    status = napi_wrap(env, thisVar, static_cast<void*>(napiAudioSpatializationManager.get()),
        NapiAudioSpatializationManager::Destructor, nullptr, nullptr);
    if (status != napi_ok) {
        ObjectRefMap<NapiAudioSpatializationManager>::Erase(napiAudioSpatializationManager.get());
        return result;
    }
    napiAudioSpatializationManager.release();
    return thisVar;
}

NapiAudioSpatializationManager* NapiAudioSpatializationManager::GetParamWithSync(const napi_env &env,
    napi_callback_info info, size_t &argc, napi_value *args)
{
    napi_status status;
    NapiAudioSpatializationManager *napiAudioSpatializationManager = nullptr;
    napi_value jsThis = nullptr;

    status = napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok && jsThis != nullptr, nullptr,
        "GetParamWithSync fail to napi_get_cb_info");

    status = napi_unwrap(env, jsThis, (void **)&napiAudioSpatializationManager);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, nullptr, "napi_unwrap failed");
    CHECK_AND_RETURN_RET_LOG(napiAudioSpatializationManager != nullptr &&
        napiAudioSpatializationManager->audioSpatializationMngr_ !=
        nullptr, napiAudioSpatializationManager, "GetParamWithSync fail to napi_unwrap");
    return napiAudioSpatializationManager;
}

napi_value NapiAudioSpatializationManager::CreateSpatializationManagerWrapper(napi_env env)
{
    napi_status status;
    napi_value result = nullptr;
    napi_value constructor;

    status = napi_get_reference_value(env, g_spatializationManagerConstructor, &constructor);
    if (status != napi_ok) {
        AUDIO_ERR_LOG("Failed in CreateSpatializationManagerWrapper, %{public}d", status);
        goto fail;
    }
    status = napi_new_instance(env, constructor, 0, nullptr, &result);
    if (status != napi_ok) {
        AUDIO_ERR_LOG("napi_new_instance failed, sttaus:%{public}d", status);
        goto fail;
    }
    return result;

fail:
    napi_get_undefined(env, &result);
    return result;
}

napi_value NapiAudioSpatializationManager::Init(napi_env env, napi_value exports)
{
    AUDIO_DEBUG_LOG("Init");

    napi_status status;
    napi_value constructor;
    napi_value result = nullptr;
    const int32_t refCount = 1;
    napi_get_undefined(env, &result);

    napi_property_descriptor audio_spatialization_manager_properties[] = {
        DECLARE_NAPI_FUNCTION("isSpatializationEnabled", IsSpatializationEnabled),
        DECLARE_NAPI_FUNCTION("setSpatializationEnabled", SetSpatializationEnabled),
        DECLARE_NAPI_FUNCTION("isHeadTrackingEnabled", IsHeadTrackingEnabled),
        DECLARE_NAPI_FUNCTION("setHeadTrackingEnabled", SetHeadTrackingEnabled),
        DECLARE_NAPI_FUNCTION("isSpatializationSupported", IsSpatializationSupported),
        DECLARE_NAPI_FUNCTION("isSpatializationSupportedForDevice", IsSpatializationSupportedForDevice),
        DECLARE_NAPI_FUNCTION("isHeadTrackingSupported", IsHeadTrackingSupported),
        DECLARE_NAPI_FUNCTION("isHeadTrackingSupportedForDevice", IsHeadTrackingSupportedForDevice),
        DECLARE_NAPI_FUNCTION("updateSpatialDeviceState", UpdateSpatialDeviceState),
        DECLARE_NAPI_FUNCTION("getSpatializationSceneType", GetSpatializationSceneType),
        DECLARE_NAPI_FUNCTION("setSpatializationSceneType", SetSpatializationSceneType),
        DECLARE_NAPI_FUNCTION("on", On),
        DECLARE_NAPI_FUNCTION("off", Off),
    };

    status = napi_define_class(env, AUDIO_SPATIALIZATION_MANAGER_NAPI_CLASS_NAME.c_str(), NAPI_AUTO_LENGTH, Construct,
        nullptr,
        sizeof(audio_spatialization_manager_properties) / sizeof(audio_spatialization_manager_properties[PARAM0]),
        audio_spatialization_manager_properties, &constructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_define_class fail");

    status = napi_create_reference(env, constructor, refCount, &g_spatializationManagerConstructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_create_reference fail");
    status = napi_set_named_property(env, exports, AUDIO_SPATIALIZATION_MANAGER_NAPI_CLASS_NAME.c_str(),
        constructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_set_named_property fail");
    return exports;
}

napi_value NapiAudioSpatializationManager::IsSpatializationEnabled(napi_env env, napi_callback_info info)
{
    AUDIO_INFO_LOG("IsSpatializationEnabled in");
    napi_value result = nullptr;
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED), "No system permission");

    bool isSpatializationEnabled = false;
    const size_t requireArgc = ARGS_ONE;
    size_t argc = PARAM1;
    napi_value args[ARGS_ONE] = {};
    auto *napiAudioSpatializationManager = GetParamWithSync(env, info, argc, args);
    CHECK_AND_RETURN_RET_LOG(napiAudioSpatializationManager != nullptr, result,
        "napiAudioSpatializationManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioSpatializationManager->audioSpatializationMngr_ != nullptr, result,
        "audioSpatializationMngr_ is nullptr");

    if (argc == requireArgc) {
        bool argTransFlag = true;
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, args[PARAM0], &valueType);
        CHECK_AND_RETURN_RET_LOG(valueType == napi_object, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID, "incorrect parameter types: The type of deviceDescriptor must be object"),
            "invalid valueType");

        sptr<AudioDeviceDescriptor> selectedAudioDevice = new (std::nothrow) AudioDeviceDescriptor();
        NapiParamUtils::GetAudioDeviceDescriptor(env, selectedAudioDevice, argTransFlag, args[PARAM0]);
        CHECK_AND_RETURN_RET_LOG(argTransFlag == true, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
            "parameter verification failed: The param of deviceDescriptor must be interface AudioDeviceDescriptor"),
            "invalid parameter");

        isSpatializationEnabled = napiAudioSpatializationManager->audioSpatializationMngr_
            ->IsSpatializationEnabled(selectedAudioDevice);
    } else if (argc < requireArgc) {
        isSpatializationEnabled = napiAudioSpatializationManager->audioSpatializationMngr_->IsSpatializationEnabled();
    } else {
        NapiAudioError::ThrowError(env, NAPI_ERR_INPUT_INVALID, "invalid arguments");
        return NapiParamUtils::GetUndefinedValue(env);
    }
    NapiParamUtils::SetValueBoolean(env, isSpatializationEnabled, result);
    return result;
}

napi_value NapiAudioSpatializationManager::SetSpatializationEnabled(napi_env env, napi_callback_info info)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED), "No system permission");

    size_t requireArgc = ARGS_ONE;
    auto context = std::make_shared<AudioSpatializationManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("SetSpatializationEnabled failed : no memory");
        NapiAudioError::ThrowError(env, "SetSpatializationEnabled failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context, &requireArgc](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "mandatory parameters are left unspecified",
            NAPI_ERR_INPUT_INVALID);
            bool argTransFlag = true;
        if (argc == ARGS_ONE) {
            context->status = NapiParamUtils::GetValueBoolean(env, context->spatializationEnable, argv[PARAM0]);
            NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok,
                "incorrect parameter types: The type of enable must be boolean", NAPI_ERR_INPUT_INVALID);
        } else if (argc == ARGS_TWO) {
            context->status = NapiParamUtils::GetValueBoolean(env, context->spatializationEnable, argv[PARAM0]);
            if (context->status == napi_ok) {
                requireArgc = ARGS_ONE;
            } else {
                requireArgc = ARGS_TWO;
                context->status = NapiParamUtils::GetAudioDeviceDescriptor(env, context->deviceDescriptor, argTransFlag,
                    argv[PARAM0]);
                NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok,
                    "incorrect parameter types: The param of deviceDescriptor must be interface AudioDeviceDescriptor",
                    NAPI_ERR_INPUT_INVALID);
                context->status = NapiParamUtils::GetValueBoolean(env, context->spatializationEnable, argv[PARAM1]);
                NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok,
                    "incorrect parameter types: The type of enable must be boolean", NAPI_ERR_INPUT_INVALID);
            }
        }
    };
    context->GetCbInfo(env, info, inputParser);
    if (context->status != napi_ok) {
        NapiAudioError::ThrowError(env, context->errCode, context->errMessage);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    return updateSpatializationEnabled(env, requireArgc, context);
}

napi_value NapiAudioSpatializationManager::updateSpatializationEnabled(napi_env env, const std::size_t argc,
    std::shared_ptr<AudioSpatializationManagerAsyncContext> &context)
{
    auto executor = [context, argc]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioSpatializationManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioSpatializationManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioSpatializationManagerStatus(napiAudioSpatializationManager, context),
            "audio spatialization manager state is error.");
        if (argc == ARGS_ONE) {
            context->intValue = napiAudioSpatializationManager->audioSpatializationMngr_->SetSpatializationEnabled(
                context->spatializationEnable);
        } else if (argc == ARGS_TWO) {
            context->intValue = napiAudioSpatializationManager->audioSpatializationMngr_->SetSpatializationEnabled(
                context->deviceDescriptor, context->spatializationEnable);
        }

        if (context->intValue == ERR_PERMISSION_DENIED) {
            context->SignError(NAPI_ERR_NO_PERMISSION);
        } else if (context->intValue != SUCCESS) {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SetSpatializationEnabled", executor, complete);
}

napi_value NapiAudioSpatializationManager::IsHeadTrackingEnabled(napi_env env, napi_callback_info info)
{
    AUDIO_INFO_LOG("IsHeadTrackingEnabled in");
    napi_value result = nullptr;
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED), "No system permission");

    bool isHeadTrackingEnabled = false;
    const size_t requireArgc = ARGS_ONE;
    size_t argc = PARAM1;
    napi_value args[ARGS_ONE] = {};
    auto *napiAudioSpatializationManager = GetParamWithSync(env, info, argc, args);
    CHECK_AND_RETURN_RET_LOG(napiAudioSpatializationManager != nullptr, result,
        "napiAudioSpatializationManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioSpatializationManager->audioSpatializationMngr_ != nullptr, result,
        "audioSpatializationMngr_ is nullptr");

    if (argc == requireArgc) {
        bool argTransFlag = true;
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, args[PARAM0], &valueType);
        CHECK_AND_RETURN_RET_LOG(valueType == napi_object, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID, "incorrect parameter types: The type of deviceDescriptor must be object"),
            "invalid valueType");

        sptr<AudioDeviceDescriptor> selectedAudioDevice = new (std::nothrow) AudioDeviceDescriptor();
        NapiParamUtils::GetAudioDeviceDescriptor(env, selectedAudioDevice, argTransFlag, args[PARAM0]);
        CHECK_AND_RETURN_RET_LOG(argTransFlag == true, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
            "parameter verification failed: The param of deviceDescriptor must be interface AudioDeviceDescriptor"),
            "invalid parameter");

        isHeadTrackingEnabled = napiAudioSpatializationManager->audioSpatializationMngr_
            ->IsHeadTrackingEnabled(selectedAudioDevice);
    } else if (argc < requireArgc) {
        isHeadTrackingEnabled = napiAudioSpatializationManager->audioSpatializationMngr_->IsHeadTrackingEnabled();
    } else {
        NapiAudioError::ThrowError(env, NAPI_ERR_INPUT_INVALID, "invalid arguments");
        return NapiParamUtils::GetUndefinedValue(env);
    }
    NapiParamUtils::SetValueBoolean(env, isHeadTrackingEnabled, result);
    return result;
}

napi_value NapiAudioSpatializationManager::SetHeadTrackingEnabled(napi_env env, napi_callback_info info)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED), "No system permission");

    size_t requireArgc = ARGS_ONE;
    auto context = std::make_shared<AudioSpatializationManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("SetHeadTrackingEnabled failed : no memory");
        NapiAudioError::ThrowError(env, "SetHeadTrackingEnabled failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context, &requireArgc](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "mandatory parameters are left unspecified",
            NAPI_ERR_INPUT_INVALID);
        bool argTransFlag = true;
        if (argc == ARGS_ONE) {
            context->status = NapiParamUtils::GetValueBoolean(env, context->headTrackingEnable, argv[PARAM0]);
            NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok,
                "incorrect parameter types: The type of enable must be boolean", NAPI_ERR_INPUT_INVALID);
        } else if (argc == ARGS_TWO) {
            context->status = NapiParamUtils::GetValueBoolean(env, context->headTrackingEnable, argv[PARAM0]);
            if (context->status == napi_ok) {
                requireArgc = ARGS_ONE;
            } else {
                requireArgc = ARGS_TWO;
                context->status = NapiParamUtils::GetAudioDeviceDescriptor(env, context->deviceDescriptor, argTransFlag,
                    argv[PARAM0]);
                NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok,
                    "incorrect parameter types: The param of deviceDescriptor must be interface AudioDeviceDescriptor",
                    NAPI_ERR_INPUT_INVALID);
                context->status = NapiParamUtils::GetValueBoolean(env, context->headTrackingEnable, argv[PARAM1]);
                NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok,
                    "incorrect parameter types: The type of enable must be boolean", NAPI_ERR_INPUT_INVALID);
            }
        }
    };
    context->GetCbInfo(env, info, inputParser);
    if (context->status != napi_ok) {
        NapiAudioError::ThrowError(env, context->errCode);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    return updateHeadTrackingEnabled(env, requireArgc, context);
}

napi_value NapiAudioSpatializationManager::updateHeadTrackingEnabled(napi_env env, const std::size_t argc,
    std::shared_ptr<AudioSpatializationManagerAsyncContext> &context)
{
    auto executor = [context, argc]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioSpatializationManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioSpatializationManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioSpatializationManagerStatus(napiAudioSpatializationManager, context),
            "audio spatialization manager state is error.");
        if (argc == ARGS_ONE) {
            context->intValue = napiAudioSpatializationManager->audioSpatializationMngr_->SetHeadTrackingEnabled(
                context->headTrackingEnable);
        } else if (argc == ARGS_TWO) {
            context->intValue = napiAudioSpatializationManager->audioSpatializationMngr_->SetHeadTrackingEnabled(
                context->deviceDescriptor, context->headTrackingEnable);
        }

        if (context->intValue == ERR_PERMISSION_DENIED) {
            context->SignError(NAPI_ERR_NO_PERMISSION);
        } else if (context->intValue != SUCCESS) {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SetHeadTrackingEnabled", executor, complete);
}

napi_value NapiAudioSpatializationManager::IsSpatializationSupported(napi_env env, napi_callback_info info)
{
    AUDIO_DEBUG_LOG("IsSpatializationSupported in");
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED), "No system permission");

    napi_value result = nullptr;
    size_t argc = PARAM0;
    auto *napiAudioSpatializationManager = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(argc == PARAM0, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID),
        "invalid arguments");
    CHECK_AND_RETURN_RET_LOG(napiAudioSpatializationManager != nullptr, result,
        "napiAudioSpatializationManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioSpatializationManager->audioSpatializationMngr_ != nullptr, result,
        "audioSpatializationMngr_ is nullptr");
    bool isSpatializationSupported =
        napiAudioSpatializationManager->audioSpatializationMngr_->IsSpatializationSupported();
    NapiParamUtils::SetValueBoolean(env, isSpatializationSupported, result);

    return result;
}

napi_value NapiAudioSpatializationManager::IsSpatializationSupportedForDevice(napi_env env, napi_callback_info info)
{
    AUDIO_DEBUG_LOG("IsSpatializationSupportedForDevice");
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED), "No system permission");

    napi_value result = nullptr;
    bool argTransFlag = true;
    size_t argc = ARGS_ONE;
    napi_value args[ARGS_ONE] = {};
    auto *napiAudioSpatializationManager = GetParamWithSync(env, info, argc, args);
    CHECK_AND_RETURN_RET_LOG(argc == ARGS_ONE, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
        "mandatory parameters are left unspecified"), "invalid arguments");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, args[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_object, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
        "incorrect parameter types: The type of deviceDescriptor must be object"), "invalid valueType");

    sptr<AudioDeviceDescriptor> selectedAudioDevice = new (std::nothrow) AudioDeviceDescriptor();
    NapiParamUtils::GetAudioDeviceDescriptor(env, selectedAudioDevice, argTransFlag, args[PARAM0]);
    CHECK_AND_RETURN_RET_LOG(argTransFlag == true, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
        "parameter verification failed: The param of deviceDescriptor must be interface AudioDeviceDescriptor"),
        "invalid parameter");

    bool isSpatializationSupportedForDevice = napiAudioSpatializationManager
        ->audioSpatializationMngr_->IsSpatializationSupportedForDevice(selectedAudioDevice);
    NapiParamUtils::SetValueBoolean(env, isSpatializationSupportedForDevice, result);
    return result;
}

napi_value NapiAudioSpatializationManager::IsHeadTrackingSupported(napi_env env, napi_callback_info info)
{
    AUDIO_DEBUG_LOG("IsHeadTrackingSupported in");
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED), "No system permission");

    napi_value result = nullptr;
    size_t argc = PARAM0;
    auto *napiAudioSpatializationManager = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(argc == PARAM0, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID),
        "invalid arguments");
    CHECK_AND_RETURN_RET_LOG(napiAudioSpatializationManager != nullptr, result,
        "napiAudioSpatializationManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioSpatializationManager->audioSpatializationMngr_ != nullptr, result,
        "audioSpatializationMngr_ is nullptr");
    bool isHeadTrackingSupported =
        napiAudioSpatializationManager->audioSpatializationMngr_->IsHeadTrackingSupported();
    NapiParamUtils::SetValueBoolean(env, isHeadTrackingSupported, result);
    return result;
}

napi_value NapiAudioSpatializationManager::IsHeadTrackingSupportedForDevice(napi_env env, napi_callback_info info)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED), "No system permission");

    napi_value result = nullptr;
    bool argTransFlag = true;
    size_t argc = ARGS_ONE;
    napi_value args[ARGS_ONE] = {};
    auto *napiAudioSpatializationManager = GetParamWithSync(env, info, argc, args);
    CHECK_AND_RETURN_RET_LOG(argc == ARGS_ONE, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
        "mandatory parameters are left unspecified"), "invalid arguments");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, args[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_object, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
        "incorrect parameter types: The type of deviceDescriptor must be object"), "invalid valueType");

    sptr<AudioDeviceDescriptor> selectedAudioDevice = new (std::nothrow) AudioDeviceDescriptor();
    NapiParamUtils::GetAudioDeviceDescriptor(env, selectedAudioDevice, argTransFlag, args[PARAM0]);
    CHECK_AND_RETURN_RET_LOG(argTransFlag == true, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
        "parameter verification failed: The param of deviceDescriptor must be interface AudioDeviceDescriptor"),
        "invalid parameter");

    bool isHeadTrackingSupportedForDevice = napiAudioSpatializationManager
        ->audioSpatializationMngr_->IsHeadTrackingSupportedForDevice(selectedAudioDevice);
    NapiParamUtils::SetValueBoolean(env, isHeadTrackingSupportedForDevice, result);
    return result;
}

napi_value NapiAudioSpatializationManager::UpdateSpatialDeviceState(napi_env env, napi_callback_info info)
{
    AUDIO_INFO_LOG("UpdateSpatialDeviceState");
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED), "No system permission");

    napi_value result = nullptr;
    size_t argc = ARGS_ONE;
    napi_value args[ARGS_ONE] = {};
    auto *napiAudioSpatializationManager = GetParamWithSync(env, info, argc, args);
    CHECK_AND_RETURN_RET_LOG(argc >= ARGS_ONE, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
        "mandatory parameters are left unspecified"), "invalid arguments");

    AudioSpatialDeviceState audioSpatialDeviceState;
    if (NapiParamUtils::GetSpatialDeviceState(env, &audioSpatialDeviceState, args[PARAM0]) != napi_ok) {
        NapiAudioError::ThrowError(env, NAPI_ERR_INVALID_PARAM,
            "parameter verification failed: The param of spatialDeviceState must be interface AudioSpatialDeviceState");
    }
    int32_t ret = napiAudioSpatializationManager->audioSpatializationMngr_->UpdateSpatialDeviceState(
        audioSpatialDeviceState);
    if (ret == ERR_PERMISSION_DENIED) {
        NapiAudioError::ThrowError(env, NAPI_ERR_NO_PERMISSION);
    }
    return result;
}

napi_value NapiAudioSpatializationManager::GetSpatializationSceneType(napi_env env, napi_callback_info info)
{
    AUDIO_INFO_LOG("Start to get current spatialization rendering scene type");
    napi_value result = nullptr;
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED), "No system permission");

    size_t argc = PARAM0;
    auto *napiAudioSpatializationManager = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(argc == PARAM0, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID),
        "invalid arguments");
    CHECK_AND_RETURN_RET_LOG(napiAudioSpatializationManager != nullptr, result,
        "napiAudioSpatializationManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioSpatializationManager->audioSpatializationMngr_ != nullptr, result,
        "audioSpatializationMngr_ is nullptr");
    AudioSpatializationSceneType sceneType =
        napiAudioSpatializationManager->audioSpatializationMngr_->GetSpatializationSceneType();
    NapiParamUtils::SetValueInt32(env, static_cast<int32_t>(sceneType), result);

    return result;
}

napi_value NapiAudioSpatializationManager::SetSpatializationSceneType(napi_env env, napi_callback_info info)
{
    AUDIO_INFO_LOG("Start to set spatialization rendering scene type");
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED), "No system permission");

    napi_value result = nullptr;
    size_t argc = ARGS_ONE;
    napi_value args[ARGS_ONE] = {};
    auto *napiAudioSpatializationManager = GetParamWithSync(env, info, argc, args);
    CHECK_AND_RETURN_RET_LOG(argc >= ARGS_ONE, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
        "mandatory parameters are left unspecified"), "invalid arguments");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, args[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_number, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
        "incorrect parameter types: The type of spatializationSceneType must be number"), "invalid valueType");

    int32_t sceneType;
    NapiParamUtils::GetValueInt32(env, sceneType, args[PARAM0]);
    CHECK_AND_RETURN_RET_LOG(NapiAudioEnum::IsLegalInputArgumentSpatializationSceneType(sceneType),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
        "parameter verification failed: The param of spatializationSceneType must be \
        enum AudioSpatializationSceneType"), "get sceneType failed");

    CHECK_AND_RETURN_RET_LOG(napiAudioSpatializationManager != nullptr, result,
        "napiAudioSpatializationManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioSpatializationManager->audioSpatializationMngr_ != nullptr, result,
        "audioSpatializationMngr_ is nullptr");
    int32_t ret = napiAudioSpatializationManager->audioSpatializationMngr_->SetSpatializationSceneType(
        static_cast<AudioSpatializationSceneType>(sceneType));
    if (ret == ERR_PERMISSION_DENIED) {
        NapiAudioError::ThrowError(env, NAPI_ERR_NO_PERMISSION);
    }
    return result;
}

void NapiAudioSpatializationManager::RegisterCallback(napi_env env, napi_value jsThis,
    napi_value *args, const std::string &cbName)
{
    NapiAudioSpatializationManager *napiAudioSpatializationManager = nullptr;
    napi_status status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napiAudioSpatializationManager));
    if ((status != napi_ok) || (napiAudioSpatializationManager == nullptr) ||
        (napiAudioSpatializationManager->audioSpatializationMngr_ == nullptr)) {
        AUDIO_ERR_LOG("NapiAudioSpatializationManager::Failed to retrieve audio spatialization manager napi instance.");
        return;
    }

    if (!cbName.compare(SPATIALIZATION_ENABLED_CHANGE_CALLBACK_NAME)) {
        RegisterSpatializationEnabledChangeCallback(env, args, cbName, napiAudioSpatializationManager);
    } else if (!cbName.compare(SPATIALIZATION_ENABLED_CHANGE_FOR_ANY_DEVICES_CALLBACK_NAME)) {
        RegisterSpatializationEnabledChangeCallback(env, args, cbName, napiAudioSpatializationManager);
    } else if (!cbName.compare(HEAD_TRACKING_ENABLED_CHANGE_CALLBACK_NAME)) {
        RegisterHeadTrackingEnabledChangeCallback(env, args, cbName, napiAudioSpatializationManager);
    } else if (!cbName.compare(HEAD_TRACKING_ENABLED_CHANGE_FOR_ANY_DEVICES_CALLBACK_NAME)) {
        RegisterHeadTrackingEnabledChangeCallback(env, args, cbName, napiAudioSpatializationManager);
    } else {
        AUDIO_ERR_LOG("NapiAudioSpatializationManager::No such callback supported");
        NapiAudioError::ThrowError(env, NAPI_ERR_INVALID_PARAM,
            "parameter verification failed: The param of type is not supported");
    }
}

void NapiAudioSpatializationManager::RegisterSpatializationEnabledChangeCallback(napi_env env, napi_value *args,
    const std::string &cbName, NapiAudioSpatializationManager *napiAudioSpatializationManager)
{
    if (!napiAudioSpatializationManager->spatializationEnabledChangeCallbackNapi_) {
        napiAudioSpatializationManager->spatializationEnabledChangeCallbackNapi_ =
            std::make_shared<NapiAudioSpatializationEnabledChangeCallback>(env);
        CHECK_AND_RETURN_LOG(napiAudioSpatializationManager->spatializationEnabledChangeCallbackNapi_ != nullptr,
            "NapiAudioSpatializationManager: Memory Allocation Failed !!");

        int32_t ret = napiAudioSpatializationManager->audioSpatializationMngr_->
            RegisterSpatializationEnabledEventListener(
            napiAudioSpatializationManager->spatializationEnabledChangeCallbackNapi_);
        CHECK_AND_RETURN_LOG(ret == SUCCESS,
            "NapiAudioSpatializationManager: Registering of Spatialization Enabled Change Callback Failed");
    }

    std::shared_ptr<NapiAudioSpatializationEnabledChangeCallback> cb =
        std::static_pointer_cast<NapiAudioSpatializationEnabledChangeCallback>
        (napiAudioSpatializationManager->spatializationEnabledChangeCallbackNapi_);
    cb->SaveSpatializationEnabledChangeCallbackReference(args[PARAM1], cbName);

    AUDIO_INFO_LOG("Register spatialization enabled callback is successful");
}

void NapiAudioSpatializationManager::RegisterHeadTrackingEnabledChangeCallback(napi_env env, napi_value *args,
    const std::string &cbName, NapiAudioSpatializationManager *napiAudioSpatializationManager)
{
    if (!napiAudioSpatializationManager->headTrackingEnabledChangeCallbackNapi_) {
        napiAudioSpatializationManager->headTrackingEnabledChangeCallbackNapi_ =
            std::make_shared<NapiAudioHeadTrackingEnabledChangeCallback>(env);
        CHECK_AND_RETURN_LOG(napiAudioSpatializationManager->headTrackingEnabledChangeCallbackNapi_ != nullptr,
            "NapiAudioSpatializationManager: Memory Allocation Failed !!");

        int32_t ret = napiAudioSpatializationManager->audioSpatializationMngr_->
            RegisterHeadTrackingEnabledEventListener(
            napiAudioSpatializationManager->headTrackingEnabledChangeCallbackNapi_);
        CHECK_AND_RETURN_LOG(ret == SUCCESS,
            "NapiAudioSpatializationManager: Registering of Head Tracking Enabled Change Callback Failed");
    }

    std::shared_ptr<NapiAudioHeadTrackingEnabledChangeCallback> cb =
        std::static_pointer_cast<NapiAudioHeadTrackingEnabledChangeCallback>
        (napiAudioSpatializationManager->headTrackingEnabledChangeCallbackNapi_);
    cb->SaveHeadTrackingEnabledChangeCallbackReference(args[PARAM1], cbName);

    AUDIO_INFO_LOG("Register head tracking enabled callback is successful");
}

napi_value NapiAudioSpatializationManager::On(napi_env env, napi_callback_info info)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED), "No system permission");

    const size_t requireArgc = ARGS_TWO;
    size_t argc = ARGS_THREE;
    napi_value undefinedResult = nullptr;
    napi_get_undefined(env, &undefinedResult);

    napi_value args[requireArgc + 1] = {nullptr, nullptr, nullptr};
    napi_value jsThis = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr);
    if (status != napi_ok || argc < requireArgc) {
        AUDIO_ERR_LOG("On fail to napi_get_cb_info/Requires min 2 parameters");
        NapiAudioError::ThrowError(env, NAPI_ERR_INPUT_INVALID, "mandatory parameters are left unspecified");
    }

    napi_valuetype eventType = napi_undefined;
    if (napi_typeof(env, args[PARAM0], &eventType) != napi_ok || eventType != napi_string) {
        NapiAudioError::ThrowError(env, NAPI_ERR_INPUT_INVALID,
            "incorrect parameter types: The type of eventType must be string");
        return undefinedResult;
    }
    std::string callbackName = NapiParamUtils::GetStringArgument(env, args[PARAM0]);
    AUDIO_DEBUG_LOG("On callbackName: %{public}s", callbackName.c_str());

    napi_valuetype handler = napi_undefined;
    if (napi_typeof(env, args[PARAM1], &handler) != napi_ok || handler != napi_function) {
        AUDIO_ERR_LOG("On type mismatch for parameter 2");
        NapiAudioError::ThrowError(env, NAPI_ERR_INPUT_INVALID,
            "incorrect parameter types: The type of callback must be function");
        return undefinedResult;
    }

    RegisterCallback(env, jsThis, args, callbackName);

    return undefinedResult;
}

void NapiAudioSpatializationManager::UnRegisterCallback(napi_env env, napi_value jsThis,
    napi_value *args, const std::string &cbName)
{
    NapiAudioSpatializationManager *napiAudioSpatializationManager = nullptr;
    napi_status status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napiAudioSpatializationManager));
    CHECK_AND_RETURN_LOG(status == napi_ok && napiAudioSpatializationManager != nullptr,
        "Failed to retrieve napi instance.");
    CHECK_AND_RETURN_LOG(napiAudioSpatializationManager->audioSpatializationMngr_ != nullptr,
        "spatialization instance null.");

    if (!cbName.compare(SPATIALIZATION_ENABLED_CHANGE_CALLBACK_NAME)) {
        UnregisterSpatializationEnabledChangeCallback(env, args[PARAM1], cbName, napiAudioSpatializationManager);
    } else if (!cbName.compare(SPATIALIZATION_ENABLED_CHANGE_FOR_ANY_DEVICES_CALLBACK_NAME)) {
        UnregisterSpatializationEnabledChangeCallback(env, args[PARAM1], cbName, napiAudioSpatializationManager);
    } else if (!cbName.compare(HEAD_TRACKING_ENABLED_CHANGE_CALLBACK_NAME)) {
        UnregisterHeadTrackingEnabledChangeCallback(env, args[PARAM1], cbName, napiAudioSpatializationManager);
    } else if (!cbName.compare(HEAD_TRACKING_ENABLED_CHANGE_FOR_ANY_DEVICES_CALLBACK_NAME)) {
        UnregisterHeadTrackingEnabledChangeCallback(env, args[PARAM1], cbName, napiAudioSpatializationManager);
    } else {
        NapiAudioError::ThrowError(env, NAPI_ERR_INVALID_PARAM,
            "parameter verification failed: The param of type is not supported");
    }
}

void NapiAudioSpatializationManager::UnregisterSpatializationEnabledChangeCallback(napi_env env, napi_value callback,
    const std::string &cbName, NapiAudioSpatializationManager *napiAudioSpatializationManager)
{
    if (napiAudioSpatializationManager->spatializationEnabledChangeCallbackNapi_ != nullptr) {
        std::shared_ptr<NapiAudioSpatializationEnabledChangeCallback> cb =
            std::static_pointer_cast<NapiAudioSpatializationEnabledChangeCallback>(
            napiAudioSpatializationManager->spatializationEnabledChangeCallbackNapi_);
        if (callback != nullptr) {
            cb->RemoveSpatializationEnabledChangeCallbackReference(env, callback, cbName);
        }
        if (callback == nullptr || cb->GetSpatializationEnabledChangeCbListSize(cbName) == 0) {
            int32_t ret = napiAudioSpatializationManager->audioSpatializationMngr_->
                UnregisterSpatializationEnabledEventListener();
            CHECK_AND_RETURN_LOG(ret == SUCCESS, "UnregisterSpatializationEnabledEventListener Failed");
            napiAudioSpatializationManager->spatializationEnabledChangeCallbackNapi_.reset();
            napiAudioSpatializationManager->spatializationEnabledChangeCallbackNapi_ = nullptr;
            cb->RemoveAllSpatializationEnabledChangeCallbackReference(cbName);
        }
    } else {
        AUDIO_ERR_LOG("UnregisterSpatializationEnabledChangeCb: spatializationEnabledChangeCallbackNapi_ is null");
    }
}

void NapiAudioSpatializationManager::UnregisterHeadTrackingEnabledChangeCallback(napi_env env, napi_value callback,
    const std::string &cbName, NapiAudioSpatializationManager *napiAudioSpatializationManager)
{
    if (napiAudioSpatializationManager->headTrackingEnabledChangeCallbackNapi_ != nullptr) {
        std::shared_ptr<NapiAudioHeadTrackingEnabledChangeCallback> cb =
            std::static_pointer_cast<NapiAudioHeadTrackingEnabledChangeCallback>(
            napiAudioSpatializationManager->headTrackingEnabledChangeCallbackNapi_);
        if (callback != nullptr) {
            cb->RemoveHeadTrackingEnabledChangeCallbackReference(env, callback, cbName);
        }
        if (callback == nullptr || cb->GetHeadTrackingEnabledChangeCbListSize(cbName) == 0) {
            int32_t ret = napiAudioSpatializationManager->audioSpatializationMngr_->
                UnregisterHeadTrackingEnabledEventListener();
            CHECK_AND_RETURN_LOG(ret == SUCCESS, "UnregisterHeadTrackingEnabledEventListener Failed");
            napiAudioSpatializationManager->headTrackingEnabledChangeCallbackNapi_.reset();
            napiAudioSpatializationManager->headTrackingEnabledChangeCallbackNapi_ = nullptr;
            cb->RemoveAllHeadTrackingEnabledChangeCallbackReference(cbName);
        }
    } else {
        AUDIO_ERR_LOG("UnregisterHeadTrackingEnabledChangeCb: headTrackingEnabledChangeCallbackNapi_ is null");
    }
}

napi_value NapiAudioSpatializationManager::Off(napi_env env, napi_callback_info info)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED), "No system permission");
    const size_t requireArgc = ARGS_ONE;
    size_t argc = PARAM2;

    napi_value undefinedResult = nullptr;
    napi_get_undefined(env, &undefinedResult);

    napi_value args[requireArgc + 1] = {nullptr, nullptr};
    napi_value jsThis = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr);
    if (status != napi_ok || argc < requireArgc) {
        AUDIO_ERR_LOG("Off fail to napi_get_cb_info/Requires min 1 parameters");
        NapiAudioError::ThrowError(env, NAPI_ERR_INPUT_INVALID, "mandatory parameters are left unspecified");
        return undefinedResult;
    }

    napi_valuetype eventType = napi_undefined;
    if (napi_typeof(env, args[PARAM0], &eventType) != napi_ok || eventType != napi_string) {
        NapiAudioError::ThrowError(env, NAPI_ERR_INPUT_INVALID,
            "incorrect parameter types: The type of eventType must be string");
        return undefinedResult;
    }

    napi_valuetype secondArgsType = napi_undefined;
    if (argc > requireArgc &&
        (napi_typeof(env, args[PARAM1], &secondArgsType) != napi_ok || secondArgsType != napi_function)) {
        NapiAudioError::ThrowError(env, NAPI_ERR_INPUT_INVALID,
            "incorrect parameter types: The type of callback must be function");
        return undefinedResult;
    }
    std::string callbackName = NapiParamUtils::GetStringArgument(env, args[PARAM0]);

    if (argc == requireArgc) {
        args[PARAM1] = nullptr;
    }
    AUDIO_DEBUG_LOG("AudioSpatializationManagerNapi: Off callbackName: %{public}s", callbackName.c_str());

    UnRegisterCallback(env, jsThis, args, callbackName);

    return undefinedResult;
}
} // namespace AudioStandard
} // namespace OHOS
