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
#define LOG_TAG "NapiAudioVolumeManager"
#endif

#include "napi_audio_volume_manager.h"
#include "napi_audio_volume_group_manager.h"
#include "napi_appvolume_change_callback.h"
#include "napi_active_volume_type_change_callback.h"
#include "napi_audio_enum.h"
#include "napi_audio_error.h"
#include "napi_param_utils.h"
#include "audio_errors.h"
#include "audio_manager_log.h"
#include "napi_dfx_utils.h"

namespace OHOS {
namespace AudioStandard {
static __thread napi_ref g_volumeManagerConstructor = nullptr;

NapiAudioVolumeManager::NapiAudioVolumeManager()
    : audioSystemMngr_(nullptr), env_(nullptr) {}

NapiAudioVolumeManager::~NapiAudioVolumeManager() = default;

bool NapiAudioVolumeManager::CheckContextStatus(std::shared_ptr<AudioVolumeManagerAsyncContext> context)
{
    CHECK_AND_RETURN_RET_LOG(context != nullptr, false, "context object is nullptr.");
    if (context->native == nullptr) {
        context->SignError(NAPI_ERR_SYSTEM);
        AUDIO_ERR_LOG("context object state is error.");
        return false;
    }
    return true;
}

bool NapiAudioVolumeManager::CheckAudioVolumeManagerStatus(NapiAudioVolumeManager *napi,
    std::shared_ptr<AudioVolumeManagerAsyncContext> context)
{
    CHECK_AND_RETURN_RET_LOG(napi != nullptr, false, "napi object is nullptr.");
    if (napi->audioSystemMngr_ == nullptr) {
        context->SignError(NAPI_ERR_SYSTEM);
        AUDIO_ERR_LOG("context object state is error.");
        return false;
    }
    return true;
}

NapiAudioVolumeManager* NapiAudioVolumeManager::GetParamWithSync(const napi_env &env, napi_callback_info info,
    size_t &argc, napi_value *args)
{
    NapiAudioVolumeManager *napiAudioVolumeManager = nullptr;
    napi_value jsThis = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok && jsThis != nullptr, nullptr,
        "GetParamWithSync fail to napi_get_cb_info");

    status = napi_unwrap(env, jsThis, (void **)&napiAudioVolumeManager);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, nullptr, "napi_unwrap failed");
    CHECK_AND_RETURN_RET_LOG(napiAudioVolumeManager != nullptr && napiAudioVolumeManager->audioSystemMngr_ != nullptr,
        napiAudioVolumeManager, "GetParamWithSync fail to napi_unwrap");
    return napiAudioVolumeManager;
}

void NapiAudioVolumeManager::Destructor(napi_env env, void *nativeObject, void *finalizeHint)
{
    if (nativeObject == nullptr) {
        AUDIO_WARNING_LOG("Native object is null");
        return;
    }
    auto obj = static_cast<NapiAudioVolumeManager *>(nativeObject);
    ObjectRefMap<NapiAudioVolumeManager>::DecreaseRef(obj);
    AUDIO_INFO_LOG("Decrease obj count");
}

napi_value NapiAudioVolumeManager::Construct(napi_env env, napi_callback_info info)
{
    AUDIO_PRERELEASE_LOGI("Construct");
    napi_status status;
    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    size_t argc = ARGS_TWO;
    napi_value argv[ARGS_TWO] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;

    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    std::unique_ptr<NapiAudioVolumeManager> napiAudioVolumeManager = std::make_unique<NapiAudioVolumeManager>();
    CHECK_AND_RETURN_RET_LOG(napiAudioVolumeManager != nullptr, result, "No memory");

    napiAudioVolumeManager->audioSystemMngr_ = AudioSystemManager::GetInstance();
    napiAudioVolumeManager->env_ = env;
    napiAudioVolumeManager->cachedClientId_ = getpid();
    ObjectRefMap<NapiAudioVolumeManager>::Insert(napiAudioVolumeManager.get());

    status = napi_wrap(env, thisVar, static_cast<void*>(napiAudioVolumeManager.get()),
        NapiAudioVolumeManager::Destructor, nullptr, nullptr);
    if (status != napi_ok) {
        ObjectRefMap<NapiAudioVolumeManager>::Erase(napiAudioVolumeManager.get());
        return result;
    }
    napiAudioVolumeManager.release();
    return thisVar;
}

napi_value NapiAudioVolumeManager::CreateVolumeManagerWrapper(napi_env env)
{
    napi_status status;
    napi_value result = nullptr;
    napi_value constructor;

    status = napi_get_reference_value(env, g_volumeManagerConstructor, &constructor);
    if (status != napi_ok) {
        AUDIO_ERR_LOG("Failed in CreateVolumeManagerWrapper, %{public}d", status);
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

napi_value NapiAudioVolumeManager::Init(napi_env env, napi_value exports)
{
    napi_status status;
    napi_value constructor;
    napi_value result = nullptr;
    const int32_t refCount = ARGS_ONE;
    napi_get_undefined(env, &result);

    napi_property_descriptor audio_volume_manager_properties[] = {
        DECLARE_NAPI_FUNCTION("getVolumeGroupInfos", GetVolumeGroupInfos),
        DECLARE_NAPI_FUNCTION("getVolumeGroupInfosSync", GetVolumeGroupInfosSync),
        DECLARE_NAPI_FUNCTION("getVolumeGroupManager", GetVolumeGroupManager),
        DECLARE_NAPI_FUNCTION("getVolumeGroupManagerSync", GetVolumeGroupManagerSync),
        DECLARE_NAPI_FUNCTION("setAppVolumePercentage", SetAppVolumePercentage),
        DECLARE_NAPI_FUNCTION("setAppVolumePercentageForUid", SetAppVolumePercentageForUid),
        DECLARE_NAPI_FUNCTION("getAppVolumePercentage", GetAppVolumePercentage),
        DECLARE_NAPI_FUNCTION("getAppVolumePercentageForUid", GetAppVolumePercentageForUid),
        DECLARE_NAPI_FUNCTION("setAppVolumeMutedForUid", SetAppVolumeMutedForUid),
        DECLARE_NAPI_FUNCTION("isAppVolumeMutedForUid", IsAppVolumeMutedForUid),
        DECLARE_NAPI_FUNCTION("getSystemVolume", GetSystemVolume),
        DECLARE_NAPI_FUNCTION("getMinSystemVolume", GetMinSystemVolume),
        DECLARE_NAPI_FUNCTION("getMaxSystemVolume", GetMaxSystemVolume),
        DECLARE_NAPI_FUNCTION("isSystemMuted", IsSystemMuted),
        DECLARE_NAPI_FUNCTION("getVolumeInUnitOfDb", GetVolumeInUnitOfDb),
        DECLARE_NAPI_FUNCTION("getVolumeByStream", GetVolumeByStream),
        DECLARE_NAPI_FUNCTION("getMinVolumeByStream", GetMinVolumeByStream),
        DECLARE_NAPI_FUNCTION("getMaxVolumeByStream", GetMaxVolumeByStream),
        DECLARE_NAPI_FUNCTION("isSystemMutedForStream", IsSystemMutedForStream),
        DECLARE_NAPI_FUNCTION("getVolumeInUnitOfDbByStream", GetVolumeInUnitOfDbByStream),
        DECLARE_NAPI_FUNCTION("getSupportedAudioVolumeTypes", GetSupportedAudioVolumeTypes),
        DECLARE_NAPI_FUNCTION("getAudioVolumeTypeByStreamUsage", GetAudioVolumeTypeByStreamUsage),
        DECLARE_NAPI_FUNCTION("getStreamUsagesByVolumeType", GetStreamUsagesByVolumeType),
        DECLARE_NAPI_FUNCTION("getSystemVolumeByUid", GetSystemVolumeByUid),
        DECLARE_NAPI_FUNCTION("setSystemVolumeByUid", SetSystemVolumeByUid),
        DECLARE_NAPI_FUNCTION("on", On),
        DECLARE_NAPI_FUNCTION("off", Off),
        DECLARE_NAPI_FUNCTION("forceVolumeKeyControlType", ForceVolumeKeyControlType),
        DECLARE_NAPI_FUNCTION("getSystemVolumePercentage", GetSystemVolumePercentage),
        DECLARE_NAPI_FUNCTION("setSystemVolumePercentage", SetSystemVolumePercentage),
        DECLARE_NAPI_FUNCTION("getMinSystemVolumePercentage", GetMinSystemVolumePercentage),
    };

    status = napi_define_class(env, AUDIO_VOLUME_MANAGER_NAPI_CLASS_NAME.c_str(), NAPI_AUTO_LENGTH, Construct, nullptr,
        sizeof(audio_volume_manager_properties) / sizeof(audio_volume_manager_properties[PARAM0]),
        audio_volume_manager_properties, &constructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_define_class fail");
    status = napi_create_reference(env, constructor, refCount, &g_volumeManagerConstructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_create_reference fail");
    status = napi_set_named_property(env, exports, AUDIO_VOLUME_MANAGER_NAPI_CLASS_NAME.c_str(), constructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_set_named_property fail");
    return exports;
}

napi_value NapiAudioVolumeManager::GetAppVolumePercentage(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioVolumeManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("GetAppVolumePercentage failed : no memory");
        NapiAudioError::ThrowError(env, "GetAppVolumePercentage failed : no memory", NAPI_ERR_SYSTEM);
        return NapiParamUtils::GetUndefinedValue(env);
    }
    size_t argNum = 0;
    auto inputParser = [context, &argNum](size_t argc, napi_value *argv) {
        argNum = argc;
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioVolumeManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioVolumeManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioVolumeManagerStatus(napiAudioVolumeManager, context),
            "audio volume group manager state is error.");
        context->intValue = napiAudioVolumeManager->audioSystemMngr_->GetSelfAppVolume(context->volLevel);
        if (context->intValue != SUCCESS) {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueInt32(env, context->volLevel, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "GetAppVolumePercentage", executor, complete);
}

napi_value NapiAudioVolumeManager::GetAppVolumePercentageForUid(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioVolumeManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("GetAppVolumePercentageForUid failed : no memory");
        NapiAudioError::ThrowError(env, "GetAppVolumePercentageForUid failed : no memory", NAPI_ERR_SYSTEM);
        return NapiParamUtils::GetUndefinedValue(env);
    }
    size_t argNum = 0;
    auto inputParser = [env, context, &argNum](size_t argc, napi_value *argv) {
        argNum = argc;
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "Invalid arguments count or types.",
            NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->appUid, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get appUid failed",
            NAPI_ERR_INPUT_INVALID);
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioVolumeManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioVolumeManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioVolumeManagerStatus(napiAudioVolumeManager, context),
            "audio volume group manager state is error.");
        context->intValue = napiAudioVolumeManager->audioSystemMngr_->GetAppVolume(context->appUid, context->volLevel);
        CHECK_AND_RETURN(context->intValue != SUCCESS);
        if (context->intValue == ERR_PERMISSION_DENIED) {
            context->SignError(NAPI_ERR_NO_PERMISSION);
        } else if (context->intValue == ERR_SYSTEM_PERMISSION_DENIED) {
            context->SignError(NAPI_ERR_PERMISSION_DENIED);
        } else {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueInt32(env, context->volLevel, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "GetAppVolumePercentageForUid", executor, complete);
}

napi_value NapiAudioVolumeManager::SetAppVolumePercentageForUid(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioVolumeManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("SetAppVolumePercentageForUid failed : no memory");
        NapiAudioError::ThrowError(env, "SetAppVolumePercentageForUid failed : no memory", NAPI_ERR_SYSTEM);
        return NapiParamUtils::GetUndefinedValue(env);
    }
    size_t argNum = 0;
    auto inputParser = [env, context, &argNum](size_t argc, napi_value *argv) {
        argNum = argc;
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_TWO, " Invalid arguments count or types.",
            NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->appUid, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get appUid failed",
            NAPI_ERR_INPUT_INVALID);
        context->status = NapiParamUtils::GetValueInt32(env, context->volLevel, argv[PARAM1]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get volLevel failed",
            NAPI_ERR_INPUT_INVALID);
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioVolumeManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioVolumeManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioVolumeManagerStatus(napiAudioVolumeManager, context),
            "audio volume group manager state is error.");
        context->intValue = napiAudioVolumeManager->audioSystemMngr_->SetAppVolume(
            context->appUid, context->volLevel);
        CHECK_AND_RETURN(context->intValue != SUCCESS);
        if (context->intValue == ERR_PERMISSION_DENIED) {
            context->SignError(NAPI_ERR_NO_PERMISSION);
        } else if (context->intValue == ERR_SYSTEM_PERMISSION_DENIED) {
            context->SignError(NAPI_ERR_PERMISSION_DENIED);
        } else if (context->intValue == ERR_INVALID_PARAM) {
            context->SignError(NAPI_ERROR_INVALID_PARAM);
        } else {
            context->SignError(NAPI_ERR_SYSTEM, "System error. Set app volume fail.");
        }
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SetAppVolumePercentageForUid", executor, complete);
}

napi_value NapiAudioVolumeManager::SetAppVolumePercentage(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioVolumeManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("SetAppVolumeDegree failed : no memory");
        NapiAudioError::ThrowError(env, "SetAppVolumeDegree failed : no memory", NAPI_ERR_SYSTEM);
        return NapiParamUtils::GetUndefinedValue(env);
    }
    size_t argNum = 0;
    auto inputParser = [env, context, &argNum](size_t argc, napi_value *argv) {
        argNum = argc;
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "Invalid arguments count or types.",
            NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->volLevel, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get appUid failed",
            NAPI_ERR_INPUT_INVALID);
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioVolumeManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioVolumeManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioVolumeManagerStatus(napiAudioVolumeManager, context),
            "audio volume group manager state is error.");
        context->intValue = napiAudioVolumeManager->audioSystemMngr_->SetSelfAppVolume(
            context->volLevel);
        CHECK_AND_RETURN(context->intValue != SUCCESS);
        if (context->intValue == ERR_INVALID_PARAM) {
            context->SignError(NAPI_ERROR_INVALID_PARAM, "Invalid arguments count or types.");
        } else {
            context->SignError(NAPI_ERR_SYSTEM, "System error. Set app volume fail.");
        }
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SetAppVolumePercentage", executor, complete);
}

napi_value NapiAudioVolumeManager::SetAppVolumeMutedForUid(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioVolumeManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("SetAppVolumeMutedForUid failed : no memory");
        NapiAudioError::ThrowError(env, "SetAppVolumeMutedForUid failed : no memory", NAPI_ERR_SYSTEM);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_TWO, "Invalid arguments count or types.",
            NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->appUid, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get appUid failed", NAPI_ERR_INPUT_INVALID);
        context->status = NapiParamUtils::GetValueBoolean(env, context->isMute, argv[PARAM1]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get isMute failed", NAPI_ERR_INPUT_INVALID);
    };
    context->GetCbInfo(env, info, inputParser);
    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioVolumeManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioVolumeManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioVolumeManagerStatus(napiAudioVolumeManager, context),
            "audio volume group manager state is error.");
        context->intValue = napiAudioVolumeManager->audioSystemMngr_->SetAppVolumeMuted(
            context->appUid, context->isMute);
        CHECK_AND_RETURN(context->intValue != SUCCESS);
        if (context->intValue == ERR_PERMISSION_DENIED) {
            context->SignError(NAPI_ERR_NO_PERMISSION);
        } else if (context->intValue == ERR_SYSTEM_PERMISSION_DENIED) {
            context->SignError(NAPI_ERR_PERMISSION_DENIED);
        } else {
            context->SignError(NAPI_ERR_SYSTEM, "System error. Set app volume fail.");
        }
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SetAppVolumeMutedForUid", executor, complete);
}

napi_value NapiAudioVolumeManager::IsAppVolumeMutedForUid(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioVolumeManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("IsAppVolumeMutedForUid failed : no memory");
        NapiAudioError::ThrowError(env, "IsAppVolumeMutedForUid failed : no memory", NAPI_ERR_SYSTEM);
        return NapiParamUtils::GetUndefinedValue(env);
    }
    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_TWO, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->appUid, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get appUid failed", NAPI_ERR_INPUT_INVALID);
        context->status = NapiParamUtils::GetValueBoolean(env, context->isOwned, argv[PARAM1]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get isOwned failed", NAPI_ERR_INPUT_INVALID);
    };
    context->GetCbInfo(env, info, inputParser);
    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioVolumeManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioVolumeManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioVolumeManagerStatus(napiAudioVolumeManager, context),
            "audio volume group manager state is error.");
        context->intValue = napiAudioVolumeManager->audioSystemMngr_->IsAppVolumeMute(
            context->appUid, context->isOwned, context->isMute);
        CHECK_AND_RETURN(context->intValue != SUCCESS);
        if (context->intValue == ERR_PERMISSION_DENIED) {
            context->SignError(NAPI_ERR_NO_PERMISSION);
        } else if (context->intValue == ERR_SYSTEM_PERMISSION_DENIED) {
            context->SignError(NAPI_ERR_PERMISSION_DENIED);
        } else {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };
    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueBoolean(env, context->isMute, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "IsAppVolumeMutedForUid", executor, complete);
}

napi_value NapiAudioVolumeManager::GetSystemVolumeByUid(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = ARGS_TWO;
    napi_value args[ARGS_TWO] = {};
    auto *napiAudioVolumeManager = GetParamWithSync(env, info, argc, args);
    CHECK_AND_RETURN_RET_LOG(argc == ARGS_TWO, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID, "mandatory parameters are left unspecified"), "invalid arguments");

    for (size_t i = PARAM0; i < argc; i++) {
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, args[i], &valueType);
        CHECK_AND_RETURN_RET_LOG(valueType == napi_number, NapiAudioError::ThrowErrorAndReturn(env,
            NAPI_ERR_INPUT_INVALID, "incorrect parameter types: The type of parameter must be number"),
            "invalid valueType");
    }
    int32_t volType;
    NapiParamUtils::GetValueInt32(env, volType, args[PARAM0]);
    CHECK_AND_RETURN_RET_LOG(NapiAudioEnum::IsLegalInputArgumentVolType(volType),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
        "parameter verification failed: The param of volumeType must be enum AudioVolumeType"), "get volType failed");

    int32_t uid;
    NapiParamUtils::GetValueInt32(env, uid, args[PARAM1]);
    CHECK_AND_RETURN_RET_LOG(uid >= 0,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
        "parameter verification failed: The param of volumeType must be greater than zero"), "get uid failed");

    CHECK_AND_RETURN_RET_LOG(napiAudioVolumeManager != nullptr, result, "napiAudioVolumeManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioVolumeManager->audioSystemMngr_ != nullptr, result,
        "audioSystemMngr_ is nullptr");
    int32_t volLevel = napiAudioVolumeManager->audioSystemMngr_->GetVolume(
        NapiAudioEnum::GetNativeAudioVolumeType(volType), uid);
    NapiParamUtils::SetValueInt32(env, volLevel, result);

    return result;
}

napi_value NapiAudioVolumeManager::SetSystemVolumeByUid(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioVolumeManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("SetSystemVolumeByUid failed : no memory");
        NapiAudioError::ThrowError(env, "SetSystemVolumeByUid failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_THREE, "invalid arguments",
            NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->volumeType, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get volumeType failed",
            NAPI_ERR_INVALID_PARAM);
        if (!NapiAudioEnum::IsLegalInputArgumentVolType(context->volumeType)) {
            context->SignError(context->errCode == NAPI_ERR_INVALID_PARAM?
                NAPI_ERR_INVALID_PARAM : NAPI_ERR_UNSUPPORTED);
        }
        context->status = NapiParamUtils::GetValueInt32(env, context->volLevel, argv[PARAM1]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get volLevel failed",
            NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->appUid, argv[PARAM2]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get appUid failed",
            NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioVolumeManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioVolumeManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioVolumeManagerStatus(napiAudioVolumeManager, context),
            "audio volume manager state is error.");
        context->intValue = napiAudioVolumeManager->audioSystemMngr_->SetVolume(
            NapiAudioEnum::GetNativeAudioVolumeType(context->volumeType), context->volLevel, context->appUid);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->intValue == SUCCESS, "SetSystemVolumeByUid failed",
            NAPI_ERR_SYSTEM);
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SetSystemVolumeByUid", executor, complete);
}

napi_value NapiAudioVolumeManager::GetVolumeGroupInfos(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioVolumeManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("GetVolumeGroupInfos failed : no memory");
        NapiAudioError::ThrowError(env, "GetVolumeGroupInfos failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments",
            NAPI_ERR_INVALID_PARAM);
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[PARAM0], &valueType);
        NAPI_CHECK_ARGS_RETURN_VOID(context, valueType == napi_string, "invaild valueType",
            NAPI_ERR_INVALID_PARAM);
        context->networkId = NapiParamUtils::GetStringArgument(env, argv[PARAM0]);
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioVolumeManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioVolumeManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioVolumeManagerStatus(napiAudioVolumeManager, context),
            "audio volume group manager state is error.");
        context->intValue = napiAudioVolumeManager->audioSystemMngr_->GetVolumeGroups(
            context->networkId, context->volumeGroupInfos);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->intValue == SUCCESS, "getvolumegroups failed",
            NAPI_ERR_SYSTEM);
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetVolumeGroupInfos(env, context->volumeGroupInfos, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "GetVolumeGroupInfos", executor, complete);
}

napi_value NapiAudioVolumeManager::GetVolumeGroupInfosSync(napi_env env, napi_callback_info info)
{
    AUDIO_INFO_LOG("GetVolumeGroupInfosSync");
    napi_value result = nullptr;
    size_t argc = ARGS_ONE;
    napi_value args[ARGS_ONE] = {};
    auto *napiAudioVolumeManager = GetParamWithSync(env, info, argc, args);
    CHECK_AND_RETURN_RET_LOG(argc >= ARGS_ONE, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID, "mandatory parameters are left unspecified"), "invalid arguments");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, args[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_string, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID, "incorrect parameter types: The type of networkId must be string"),
        "invalid valueType");

    std::string networkId = NapiParamUtils::GetStringArgument(env, args[PARAM0]);
    CHECK_AND_RETURN_RET_LOG(!networkId.empty(), NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INVALID_PARAM, "parameter verification failed: The param of networkId is empty"),
        "get networkid failed");

    std::vector<sptr<VolumeGroupInfo>> volumeGroupInfos;
    if (napiAudioVolumeManager == nullptr || napiAudioVolumeManager->audioSystemMngr_ == nullptr) {
        AUDIO_ERR_LOG("napiAudioVolumeManager or audioSystemMngr  is nullptr!");
        return nullptr;
    }
    int32_t ret = napiAudioVolumeManager->audioSystemMngr_->GetVolumeGroups(networkId, volumeGroupInfos);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, result, "GetVolumeGroups failure!");

    NapiParamUtils::SetVolumeGroupInfos(env, volumeGroupInfos, result);
    return result;
}

napi_value NapiAudioVolumeManager::GetSystemVolume(napi_env env, napi_callback_info info)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED, "No system permission"),
        "No system permission");
    napi_value result = nullptr;
    size_t argc = ARGS_ONE;
    napi_value args[ARGS_ONE] = {};
    auto *napiAudioVolumeManager = GetParamWithSync(env, info, argc, args);
    CHECK_AND_RETURN_RET_LOG(argc >= ARGS_ONE, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INVALID_PARAM, "mandatory parameters are left unspecified"), "invalid arguments");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, args[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_number, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INVALID_PARAM, "incorrect parameter types: The type of audioVolumeType must be number"),
        "invalid valueType");

    int32_t volType;
    NapiParamUtils::GetValueInt32(env, volType, args[PARAM0]);
    CHECK_AND_RETURN_RET_LOG(NapiAudioEnum::IsLegalInputArgumentVolType(volType),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
            "parameter verification failed: The param of volType must be enum AudioVolumeType"),
        "get volType failed");

    NapiDfxUtils::SendVolumeApiInvokeEvent(static_cast<int32_t>(getuid()),
        "getSystemVolume", volType);

    if (napiAudioVolumeManager == nullptr || napiAudioVolumeManager->audioSystemMngr_ == nullptr) {
        AUDIO_ERR_LOG("napiAudioVolumeManager or audioSystemMngr  is nullptr!");
        return nullptr;
    }
    int32_t systemVolume = napiAudioVolumeManager->audioSystemMngr_->GetVolume(
        NapiAudioEnum::GetNativeAudioVolumeType(volType));
    NapiParamUtils::SetValueInt32(env, systemVolume, result);
    return result;
}

napi_value NapiAudioVolumeManager::GetMinSystemVolume(napi_env env, napi_callback_info info)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED, "No system permission"),
        "No system permission");
    napi_value result = nullptr;
    size_t argc = ARGS_ONE;
    napi_value args[ARGS_ONE] = {};
    auto *napiAudioVolumeManager = GetParamWithSync(env, info, argc, args);
    CHECK_AND_RETURN_RET_LOG(argc >= ARGS_ONE, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INVALID_PARAM, "mandatory parameters are left unspecified"), "invalid arguments");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, args[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_number, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INVALID_PARAM, "incorrect parameter types: The type of audioVolumeType must be number"),
        "invalid valueType");

    int32_t volType;
    NapiParamUtils::GetValueInt32(env, volType, args[PARAM0]);
    CHECK_AND_RETURN_RET_LOG(NapiAudioEnum::IsLegalInputArgumentVolType(volType),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
            "parameter verification failed: The param of volType must be enum AudioVolumeType"),
        "get volType failed");

    NapiDfxUtils::SendVolumeApiInvokeEvent(static_cast<int32_t>(getuid()),
        "getMinSystemVolume", volType);

    if (napiAudioVolumeManager == nullptr || napiAudioVolumeManager->audioSystemMngr_ == nullptr) {
        AUDIO_ERR_LOG("napiAudioVolumeManager or audioSystemMngr  is nullptr!");
        return nullptr;
    }
    int32_t minSystemVolume = napiAudioVolumeManager->audioSystemMngr_->GetMinVolume(
        NapiAudioEnum::GetNativeAudioVolumeType(volType));
    NapiParamUtils::SetValueInt32(env, minSystemVolume, result);
    return result;
}

napi_value NapiAudioVolumeManager::GetMaxSystemVolume(napi_env env, napi_callback_info info)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED, "No system permission"),
        "No system permission");
    napi_value result = nullptr;
    size_t argc = ARGS_ONE;
    napi_value args[ARGS_ONE] = {};
    auto *napiAudioVolumeManager = GetParamWithSync(env, info, argc, args);
    CHECK_AND_RETURN_RET_LOG(argc >= ARGS_ONE, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INVALID_PARAM, "mandatory parameters are left unspecified"), "invalid arguments");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, args[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_number, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INVALID_PARAM, "incorrect parameter types: The type of audioVolumeType must be number"),
        "invalid valueType");

    int32_t volType;
    NapiParamUtils::GetValueInt32(env, volType, args[PARAM0]);
    CHECK_AND_RETURN_RET_LOG(NapiAudioEnum::IsLegalInputArgumentVolType(volType),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
            "parameter verification failed: The param of volType must be enum AudioVolumeType"),
        "get volType failed");

    NapiDfxUtils::SendVolumeApiInvokeEvent(static_cast<int32_t>(getuid()),
        "getMaxSystemVolume", volType);

    if (napiAudioVolumeManager == nullptr || napiAudioVolumeManager->audioSystemMngr_ == nullptr) {
        AUDIO_ERR_LOG("napiAudioVolumeManager or audioSystemMngr  is nullptr!");
        return nullptr;
    }
    int32_t maxSystemVolume = napiAudioVolumeManager->audioSystemMngr_->GetMaxVolume(
        NapiAudioEnum::GetNativeAudioVolumeType(volType));
    NapiParamUtils::SetValueInt32(env, maxSystemVolume, result);
    return result;
}

napi_value NapiAudioVolumeManager::IsSystemMuted(napi_env env, napi_callback_info info)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED, "No system permission"),
        "No system permission");
    napi_value result = nullptr;
    size_t argc = ARGS_ONE;
    napi_value args[ARGS_ONE] = {};
    auto *napiAudioVolumeManager = GetParamWithSync(env, info, argc, args);
    CHECK_AND_RETURN_RET_LOG(argc >= ARGS_ONE, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INVALID_PARAM, "mandatory parameters are left unspecified"), "invalid arguments");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, args[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_number, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INVALID_PARAM, "incorrect parameter types: The type of audioVolumeType must be number"),
        "invalid valueType");

    int32_t volType;
    NapiParamUtils::GetValueInt32(env, volType, args[PARAM0]);
    CHECK_AND_RETURN_RET_LOG(NapiAudioEnum::IsLegalInputArgumentVolType(volType),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
            "parameter verification failed: The param of volType must be enum AudioVolumeType"),
        "get volType failed");

    NapiDfxUtils::SendVolumeApiInvokeEvent(static_cast<int32_t>(getuid()),
        "isSystemMuted", volType);

    if (napiAudioVolumeManager == nullptr || napiAudioVolumeManager->audioSystemMngr_ == nullptr) {
        AUDIO_ERR_LOG("napiAudioVolumeManager or audioSystemMngr  is nullptr!");
        return nullptr;
    }
    bool isMuted = napiAudioVolumeManager->audioSystemMngr_->IsStreamMute(
        NapiAudioEnum::GetNativeAudioVolumeType(volType));
    NapiParamUtils::SetValueBoolean(env, isMuted, result);
    return result;
}

napi_value NapiAudioVolumeManager::GetVolumeInUnitOfDb(napi_env env, napi_callback_info info)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED, "No system permission"),
        "No system permission");
    napi_value result = nullptr;
    size_t argc = ARGS_THREE;
    napi_value args[ARGS_THREE] = {};
    auto *napiAudioVolumeManager = GetParamWithSync(env, info, argc, args);
    CHECK_AND_RETURN_RET_LOG(argc >= ARGS_THREE, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INVALID_PARAM, "mandatory parameters are left unspecified"), "invalid arguments");
    for (size_t i = 0; i < argc; i++) {
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, args[i], &valueType);
        CHECK_AND_RETURN_RET_LOG(valueType == napi_number, NapiAudioError::ThrowErrorAndReturn(env,
            NAPI_ERR_INVALID_PARAM, "incorrect parameter types: The type of parameter must be number"),
            "invalid valueType");
    }
    int32_t volType;
    int32_t volLevel;
    int32_t deviceType;
    NapiParamUtils::GetValueInt32(env, volType, args[PARAM0]);
    NapiParamUtils::GetValueInt32(env, volLevel, args[PARAM1]);
    NapiParamUtils::GetValueInt32(env, deviceType, args[PARAM2]);
    CHECK_AND_RETURN_RET_LOG(NapiAudioEnum::IsLegalInputArgumentVolType(volType),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
            "parameter verification failed: The param of volType must be enum AudioVolumeType"),
        "get volType failed");
    CHECK_AND_RETURN_RET_LOG(NapiAudioEnum::IsLegalInputArgumentDeviceType(deviceType),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
            "parameter verification failed: The param of deviceType must be enum DeviceType"),
        "get deviceType failed");

    NapiDfxUtils::SendVolumeApiInvokeEvent(static_cast<int32_t>(getuid()),
        "getVolumeInUnitOfDb", volType);

    if (napiAudioVolumeManager == nullptr) {
        AUDIO_ERR_LOG("napiAudioVolumeManager  is nullptr!");
        return nullptr;
    }
    float volumeInDb = napiAudioVolumeManager->audioSystemMngr_->GetVolumeInUnitOfDb(
        NapiAudioEnum::GetNativeAudioVolumeType(volType),
        volLevel, static_cast<DeviceType>(deviceType));
    CHECK_AND_RETURN_RET_LOG(!FLOAT_COMPARE_EQ(volumeInDb, static_cast<float>(ERR_INVALID_PARAM)),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM, "volumeInDb invalid"),
        "getsystemvolumeindb failed");
    NapiParamUtils::SetValueDouble(env, volumeInDb, result);
    return result;
}

napi_value NapiAudioVolumeManager::GetVolumeByStream(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = ARGS_ONE;
    napi_value args[ARGS_ONE] = {};
    auto *napiAudioVolumeManager = GetParamWithSync(env, info, argc, args);
    CHECK_AND_RETURN_RET_LOG(argc >= ARGS_ONE, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INVALID_PARAM, "mandatory parameters are left unspecified"), "invalid arguments");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, args[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_number, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INVALID_PARAM, "incorrect parameter types: The type of streamUsage must be number"),
        "invalid valueType");

    int32_t streamUsage;
    NapiParamUtils::GetValueInt32(env, streamUsage, args[PARAM0]);
    CHECK_AND_RETURN_RET_LOG(NapiAudioEnum::IsLegalInputArgumentStreamUsage(streamUsage),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
            "parameter verification failed: The param of streamUsage must be enum StreamUsage"),
        "get volType failed");

    NapiDfxUtils::SendVolumeApiInvokeEvent(static_cast<int32_t>(getuid()),
        "getVolumeByStream", streamUsage);

    if (napiAudioVolumeManager == nullptr || napiAudioVolumeManager->audioSystemMngr_ == nullptr) {
        AUDIO_ERR_LOG("napiAudioVolumeManager or audioSystemMngr  is nullptr!");
        return nullptr;
    }
    int32_t volume = napiAudioVolumeManager->audioSystemMngr_->GetVolumeByUsage(
        NapiAudioEnum::GetNativeStreamUsage(streamUsage));
    if (volume == ERR_PERMISSION_DENIED) {
        return NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED, "No system permission");
    }
    if (volume == ERR_NOT_SUPPORTED) {
        return NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM, "streamUsage not supported");
    }
    NapiParamUtils::SetValueInt32(env, volume, result);
    return result;
}

napi_value NapiAudioVolumeManager::GetMinVolumeByStream(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = ARGS_ONE;
    napi_value args[ARGS_ONE] = {};
    auto *napiAudioVolumeManager = GetParamWithSync(env, info, argc, args);
    CHECK_AND_RETURN_RET_LOG(argc >= ARGS_ONE, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INVALID_PARAM, "mandatory parameters are left unspecified"), "invalid arguments");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, args[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_number, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INVALID_PARAM, "incorrect parameter types: The type of streamUsage must be number"),
        "invalid valueType");

    int32_t streamUsage;
    NapiParamUtils::GetValueInt32(env, streamUsage, args[PARAM0]);
    CHECK_AND_RETURN_RET_LOG(NapiAudioEnum::IsLegalInputArgumentStreamUsage(streamUsage),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
            "parameter verification failed: The param of streamUsage must be enum StreamUsage"),
        "get volType failed");

    NapiDfxUtils::SendVolumeApiInvokeEvent(static_cast<int32_t>(getuid()),
        "getMinVolumeByStream", streamUsage);

    if (napiAudioVolumeManager == nullptr || napiAudioVolumeManager->audioSystemMngr_ == nullptr) {
        AUDIO_ERR_LOG("napiAudioVolumeManager or audioSystemMngr  is nullptr!");
        return nullptr;
    }
    int32_t minVolume = napiAudioVolumeManager->audioSystemMngr_->GetMinVolumeByUsage(
        NapiAudioEnum::GetNativeStreamUsage(streamUsage));
    if (minVolume == ERR_PERMISSION_DENIED) {
        return NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED, "No system permission");
    }
    if (minVolume == ERR_NOT_SUPPORTED) {
        return NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM, "streamUsage not supported");
    }
    NapiParamUtils::SetValueInt32(env, minVolume, result);
    return result;
}

napi_value NapiAudioVolumeManager::GetMaxVolumeByStream(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = ARGS_ONE;
    napi_value args[ARGS_ONE] = {};
    auto *napiAudioVolumeManager = GetParamWithSync(env, info, argc, args);
    CHECK_AND_RETURN_RET_LOG(argc >= ARGS_ONE, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INVALID_PARAM, "mandatory parameters are left unspecified"), "invalid arguments");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, args[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_number, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INVALID_PARAM, "incorrect parameter types: The type of streamUsage must be number"),
        "invalid valueType");

    int32_t streamUsage;
    NapiParamUtils::GetValueInt32(env, streamUsage, args[PARAM0]);
    CHECK_AND_RETURN_RET_LOG(NapiAudioEnum::IsLegalInputArgumentStreamUsage(streamUsage),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
            "parameter verification failed: The param of streamUsage must be enum StreamUsage"),
        "get volType failed");

    NapiDfxUtils::SendVolumeApiInvokeEvent(static_cast<int32_t>(getuid()),
        "getMaxVolumeByStream", streamUsage);

    if (napiAudioVolumeManager == nullptr || napiAudioVolumeManager->audioSystemMngr_ == nullptr) {
        AUDIO_ERR_LOG("napiAudioVolumeManager or audioSystemMngr  is nullptr!");
        return nullptr;
    }
    int32_t maxVolume = napiAudioVolumeManager->audioSystemMngr_->GetMaxVolumeByUsage(
        NapiAudioEnum::GetNativeStreamUsage(streamUsage));
    if (maxVolume == ERR_PERMISSION_DENIED) {
        return NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED, "No system permission");
    }
    if (maxVolume == ERR_NOT_SUPPORTED) {
        return NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM, "streamUsage not supported");
    }
    NapiParamUtils::SetValueInt32(env, maxVolume, result);
    return result;
}

napi_value NapiAudioVolumeManager::IsSystemMutedForStream(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = ARGS_ONE;
    napi_value args[ARGS_ONE] = {};
    auto *napiAudioVolumeManager = GetParamWithSync(env, info, argc, args);
    CHECK_AND_RETURN_RET_LOG(argc >= ARGS_ONE, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INVALID_PARAM, "mandatory parameters are left unspecified"), "invalid arguments");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, args[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_number, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INVALID_PARAM, "incorrect parameter types: The type of streamUsage must be number"),
        "invalid valueType");

    int32_t streamUsage;
    NapiParamUtils::GetValueInt32(env, streamUsage, args[PARAM0]);
    CHECK_AND_RETURN_RET_LOG(NapiAudioEnum::IsLegalInputArgumentStreamUsage(streamUsage),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
            "parameter verification failed: The param of streamUsage must be enum StreamUsage"),
        "get volType failed");

    NapiDfxUtils::SendVolumeApiInvokeEvent(static_cast<int32_t>(getuid()),
        "isSystemMutedForStream", streamUsage);

    if (napiAudioVolumeManager == nullptr || napiAudioVolumeManager->audioSystemMngr_ == nullptr) {
        AUDIO_ERR_LOG("napiAudioVolumeManager or audioSystemMngr  is nullptr!");
        return nullptr;
    }
    bool isMuted;
    int32_t status = napiAudioVolumeManager->audioSystemMngr_->IsStreamMuteByUsage(
        NapiAudioEnum::GetNativeStreamUsage(streamUsage), isMuted);
    if (status == ERR_PERMISSION_DENIED) {
        return NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED, "No system permission");
    }
    if (status == ERR_NOT_SUPPORTED) {
        return NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM, "streamUsage not supported");
    }
    NapiParamUtils::SetValueBoolean(env, isMuted, result);
    return result;
}

napi_value NapiAudioVolumeManager::GetVolumeInUnitOfDbByStream(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = ARGS_THREE;
    napi_value args[ARGS_THREE] = {};
    auto *napiAudioVolumeManager = GetParamWithSync(env, info, argc, args);
    CHECK_AND_RETURN_RET_LOG(argc >= ARGS_THREE, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INVALID_PARAM, "mandatory parameters are left unspecified"), "invalid arguments");
    for (size_t i = 0; i < argc; i++) {
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, args[i], &valueType);
        CHECK_AND_RETURN_RET_LOG(valueType == napi_number, NapiAudioError::ThrowErrorAndReturn(env,
            NAPI_ERR_INVALID_PARAM, "incorrect parameter types: The type of parameter must be number"),
            "invalid valueType");
    }
    int32_t streamUsage;
    int32_t volLevel;
    int32_t deviceType;
    NapiParamUtils::GetValueInt32(env, streamUsage, args[PARAM0]);
    NapiParamUtils::GetValueInt32(env, volLevel, args[PARAM1]);
    NapiParamUtils::GetValueInt32(env, deviceType, args[PARAM2]);
    CHECK_AND_RETURN_RET_LOG(NapiAudioEnum::IsLegalInputArgumentStreamUsage(streamUsage),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
            "parameter verification failed: The param of streamUsage must be enum StreamUsage"),
        "get streamUsage failed");
    CHECK_AND_RETURN_RET_LOG(NapiAudioEnum::IsLegalInputArgumentDeviceType(deviceType),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
            "parameter verification failed: The param of deviceType must be enum DeviceType"),
        "get deviceType failed");

    NapiDfxUtils::SendVolumeApiInvokeEvent(static_cast<int32_t>(getuid()),
        "getVolumeInUnitOfDbByStream", streamUsage);

    if (napiAudioVolumeManager == nullptr || napiAudioVolumeManager->audioSystemMngr_ == nullptr) {
        AUDIO_ERR_LOG("napiAudioVolumeManager or audioSystemMngr_  is nullptr!");
        return nullptr;
    }
    float volumeInDb = napiAudioVolumeManager->audioSystemMngr_->GetVolumeInDbByStream(
        NapiAudioEnum::GetNativeStreamUsage(streamUsage),
        volLevel, static_cast<DeviceType>(deviceType));
    CHECK_AND_RETURN_RET_LOG(!FLOAT_COMPARE_EQ(volumeInDb, static_cast<float>(ERR_PERMISSION_DENIED)),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED, "No system permission"),
        "getsystemvolumeindb failed");
    CHECK_AND_RETURN_RET_LOG(!FLOAT_COMPARE_EQ(volumeInDb, static_cast<float>(ERR_NOT_SUPPORTED)),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM, "streamUsage not supported"),
        "getsystemvolumeindb failed");
    NapiParamUtils::SetValueDouble(env, volumeInDb, result);
    return result;
}

napi_value NapiAudioVolumeManager::GetSupportedAudioVolumeTypes(napi_env env, napi_callback_info info)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED, "No system permission"),
        "No system permission");
    napi_value result = nullptr;
    size_t argc = ARGS_ZERO;
    napi_value args[ARGS_ONE] = {};
    auto *napiAudioVolumeManager = GetParamWithSync(env, info, argc, args);
    CHECK_AND_RETURN_RET_LOG(argc >= ARGS_ZERO, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INVALID_PARAM, "mandatory parameters are left unspecified"), "invalid arguments");

    if (napiAudioVolumeManager == nullptr || napiAudioVolumeManager->audioSystemMngr_ == nullptr) {
        AUDIO_ERR_LOG("napiAudioVolumeManager or audioSystemMngr  is nullptr!");
        return nullptr;
    }
    std::vector<AudioVolumeType> volTypes = napiAudioVolumeManager->audioSystemMngr_->GetSupportedAudioVolumeTypes();
    NapiParamUtils::SetValueAudioVolumeTypeArray(env, volTypes, result);
    return result;
}

napi_value NapiAudioVolumeManager::GetAudioVolumeTypeByStreamUsage(napi_env env, napi_callback_info info)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED, "No system permission"),
        "No system permission");
    napi_value result = nullptr;
    size_t argc = ARGS_ONE;
    napi_value args[ARGS_ONE] = {};
    auto *napiAudioVolumeManager = GetParamWithSync(env, info, argc, args);
    CHECK_AND_RETURN_RET_LOG(argc >= ARGS_ONE, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INVALID_PARAM, "mandatory parameters are left unspecified"), "invalid arguments");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, args[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_number, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INVALID_PARAM, "incorrect parameter types: The type of streamUsage must be number"),
        "invalid valueType");

    int32_t streamUsage;
    NapiParamUtils::GetValueInt32(env, streamUsage, args[PARAM0]);
    CHECK_AND_RETURN_RET_LOG(NapiAudioEnum::IsLegalInputArgumentStreamUsage(streamUsage),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
            "parameter verification failed: The param of streamUsage must be enum StreamUsage"),
        "get volType failed");

    NapiDfxUtils::SendVolumeApiInvokeEvent(static_cast<int32_t>(getuid()),
        "getAudioVolumeTypeByStreamUsage", streamUsage);

    if (napiAudioVolumeManager == nullptr || napiAudioVolumeManager->audioSystemMngr_ == nullptr) {
        AUDIO_ERR_LOG("napiAudioVolumeManager or audioSystemMngr  is nullptr!");
        return nullptr;
    }
    AudioVolumeType volType = napiAudioVolumeManager->audioSystemMngr_->GetAudioVolumeTypeByStreamUsage(
        NapiAudioEnum::GetNativeStreamUsage(streamUsage));
    if (volType == STREAM_DEFAULT) {
        return NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED, "No system permission");
    }

    NapiParamUtils::SetValueInt32(env, NapiAudioEnum::GetJsAudioVolumeType(volType), result);
    return result;
}

napi_value NapiAudioVolumeManager::GetStreamUsagesByVolumeType(napi_env env, napi_callback_info info)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED, "No system permission"),
        "No system permission");
    napi_value result = nullptr;
    size_t argc = ARGS_ONE;
    napi_value args[ARGS_ONE] = {};
    auto *napiAudioVolumeManager = GetParamWithSync(env, info, argc, args);
    CHECK_AND_RETURN_RET_LOG(argc >= ARGS_ONE, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INVALID_PARAM, "mandatory parameters are left unspecified"), "invalid arguments");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, args[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_number, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INVALID_PARAM, "incorrect parameter types: The type of audioVolumeType must be number"),
        "invalid valueType");

    int32_t volType;
    NapiParamUtils::GetValueInt32(env, volType, args[PARAM0]);
    CHECK_AND_RETURN_RET_LOG(NapiAudioEnum::IsLegalInputArgumentVolType(volType),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
            "parameter verification failed: The param of volType must be enum AudioVolumeType"),
        "get volType failed");

    NapiDfxUtils::SendVolumeApiInvokeEvent(static_cast<int32_t>(getuid()),
        "getStreamUsagesByVolumeType", volType);

    if (napiAudioVolumeManager == nullptr || napiAudioVolumeManager->audioSystemMngr_ == nullptr) {
        AUDIO_ERR_LOG("napiAudioVolumeManager or audioSystemMngr  is nullptr!");
        return nullptr;
    }
    std::vector<StreamUsage> streamUsages = napiAudioVolumeManager->audioSystemMngr_->GetStreamUsagesByVolumeType(
        NapiAudioEnum::GetNativeAudioVolumeType(volType));
    NapiParamUtils::SetValueStreamUsageArray(env, streamUsages, result);
    return result;
}


napi_value NapiAudioVolumeManager::GetVolumeGroupManager(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioVolumeManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("GetVolumeGroupManager failed : no memory");
        NapiAudioError::ThrowError(env, "GetVolumeGroupManager failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments",
            NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->groupId, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get groupId failed",
            NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);
    auto executor = [context]() {
        context->audioGroupManager = AudioSystemManager::GetInstance()->GetGroupManager(context->groupId);
    };

    auto complete = [env, context](napi_value &output) {
        if (context->audioGroupManager == nullptr) {
            AUDIO_ERR_LOG("Failed to get group manager!");
            output = NapiParamUtils::GetUndefinedValue(env);
        } else {
            output = NapiAudioVolumeGroupManager::CreateAudioVolumeGroupManagerWrapper(env, context->groupId);
        }
    };
    return NapiAsyncWork::Enqueue(env, context, "GetVolumeGroupManager", executor, complete);
}

napi_value NapiAudioVolumeManager::GetVolumeGroupManagerSync(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = ARGS_ONE;
    napi_value args[ARGS_ONE] = {};
    napi_status status = NapiParamUtils::GetParam(env, info, argc, args);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "getparam failed");
    CHECK_AND_RETURN_RET_LOG(argc == ARGS_ONE, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID, "mandatory parameters are left unspecified"), "invalid arguments");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, args[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_number, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID, "incorrect parameter types: The type of groupId must be number"),
        "invalid valueType");

    int32_t groupId;
    NapiParamUtils::GetValueInt32(env, groupId, args[PARAM0]);

    if (AudioSystemManager::GetInstance()->GetGroupManager(groupId) == nullptr) {
        AUDIO_ERR_LOG("Failed to get group manager!");
        result = NapiParamUtils::GetUndefinedValue(env);
    } else {
        result = NapiAudioVolumeGroupManager::CreateAudioVolumeGroupManagerWrapper(env, groupId);
    }

    napi_value undefinedValue = nullptr;
    napi_get_undefined(env, &undefinedValue);
    bool isEqual = false;
    napi_strict_equals(env, result, undefinedValue, &isEqual);
    if (isEqual) {
        AUDIO_ERR_LOG("The audio volume group manager is undefined!");
        NapiAudioError::ThrowError(env, "GetVolumeGroupManagerSync failed: invalid param",
            NAPI_ERR_INVALID_PARAM);
        return result;
    }

    return result;
}

napi_value NapiAudioVolumeManager::RegisterCallback(napi_env env, napi_value jsThis, size_t argc, napi_value *args,
    const std::string &cbName)
{
    napi_value undefinedResult = nullptr;
    NapiAudioVolumeManager *napiVolumeManager = nullptr;
    napi_status status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napiVolumeManager));
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_SYSTEM), "status error");
    CHECK_AND_RETURN_RET_LOG(napiVolumeManager != nullptr, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_NO_MEMORY), "napiVolumeManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiVolumeManager->audioSystemMngr_ != nullptr, NapiAudioError::ThrowErrorAndReturn(
        env, NAPI_ERR_NO_MEMORY), "audioSystemMngr_ is nullptr");

    if (!cbName.compare(VOLUME_KEY_EVENT_CALLBACK_NAME)) {
        if (napiVolumeManager->volumeKeyEventCallbackNapi_ == nullptr) {
            napiVolumeManager->volumeKeyEventCallbackNapi_ = std::make_shared<NapiAudioVolumeKeyEvent>(env);
            int32_t ret = napiVolumeManager->audioSystemMngr_->RegisterVolumeKeyEventCallback(
                napiVolumeManager->cachedClientId_, napiVolumeManager->volumeKeyEventCallbackNapi_);
            napiVolumeManager->volumeKeyEventCallbackNapiList_.push_back(
                std::static_pointer_cast<NapiAudioVolumeKeyEvent>(napiVolumeManager->volumeKeyEventCallbackNapi_));
            if (ret) {
                AUDIO_ERR_LOG("RegisterVolumeKeyEventCallback Failed");
            }
        }
        std::shared_ptr<NapiAudioVolumeKeyEvent> cb =
            std::static_pointer_cast<NapiAudioVolumeKeyEvent>(napiVolumeManager->volumeKeyEventCallbackNapi_);
        cb->SaveCallbackReference(cbName, args[PARAM1]);
        if (!cb->GetVolumeTsfnFlag()) {
            cb->CreateVolumeTsfn(env);
        }
    } else if (!cbName.compare(APP_VOLUME_CHANGE_CALLBACK_NAME)) {
        undefinedResult = RegisterSelfAppVolumeChangeCallback(env, args, cbName, napiVolumeManager);
    } else if (!cbName.compare(APP_VOLUME_CHANGE_CALLBACK_NAME_FOR_UID)) {
        undefinedResult = RegisterAppVolumeChangeForUidCallback(env, args, cbName, napiVolumeManager);
    } else if (!cbName.compare(ACTIVE_VOLUME_TYPE_CHANGE_CALLBACK_NAME)) {
        undefinedResult = RegisterActiveVolumeTypeChangeCallback(env, args, cbName, napiVolumeManager);
    } else if (!cbName.compare(AUDIO_STREAM_VOLUME_CHANGE_CALLBACK_NAME)) {
        undefinedResult = RegisterStreamVolumeChangeCallback(env, args, cbName, napiVolumeManager);
    } else if (!cbName.compare(AUDIO_SYSTEM_VOLUME_CHANGE_CALLBACK_NAME)) {
        undefinedResult = RegisterSystemVolumeChangeCallback(env, args, cbName, napiVolumeManager);
    } else if (!cbName.compare(VOLUME_DEGREE_CHANGE_EVENT_CALLBACK_NAME)) {
        undefinedResult = RegisterVolumeDegreeChangeCallback(env, args, cbName, napiVolumeManager);
    } else {
        AUDIO_ERR_LOG("No such callback supported");
        NapiAudioError::ThrowError(env, NAPI_ERR_INVALID_PARAM,
            "parameter verification failed: The param of type is not supported");
    }
    return undefinedResult;
}

napi_value NapiAudioVolumeManager::RegisterActiveVolumeTypeChangeCallback(napi_env env, napi_value *args,
    const std::string &cbName, NapiAudioVolumeManager *napiAudioVolumeManager)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    if (napiAudioVolumeManager->activeVolumeTypeChangeCallbackNapi_ == nullptr) {
        napiAudioVolumeManager->activeVolumeTypeChangeCallbackNapi_ =
            std::make_shared<NapiAudioManagerActiveVolumeTypeChangeCallback>(env);
        CHECK_AND_RETURN_RET_LOG(napiAudioVolumeManager->activeVolumeTypeChangeCallbackNapi_ != nullptr,
            NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_SYSTEM, "System error"),
            "RegisterActiveVolumeTypeChangeForUidCallback: Memory Allocation Failed !");
        int32_t ret = napiAudioVolumeManager->audioSystemMngr_->SetActiveVolumeTypeCallback(
            napiAudioVolumeManager->activeVolumeTypeChangeCallbackNapi_);
        if (ret != SUCCESS) {
            if (ret == ERROR_INVALID_PARAM) {
                NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM, "Invalid parameter");
            } else if (ret == ERR_PERMISSION_DENIED) {
                NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED, "Permission denied");
            } else {
                NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_SYSTEM, "System error");
            }
        }
    }
    std::shared_ptr<NapiAudioManagerActiveVolumeTypeChangeCallback> cb =
        std::static_pointer_cast<NapiAudioManagerActiveVolumeTypeChangeCallback>(
        napiAudioVolumeManager->activeVolumeTypeChangeCallbackNapi_);
    cb->SaveActiveVolumeTypeChangeCallbackReference(cbName, args[PARAM1]);
    if (!cb->GetManagerActiveVolumeTypeChangeTsfnFlag()) {
        cb->CreateManagerActiveVolumeTypeChangeTsfn(env);
    }
    return result;
}

napi_value NapiAudioVolumeManager::RegisterAppVolumeChangeForUidCallback(napi_env env, napi_value *args,
    const std::string &cbName, NapiAudioVolumeManager *napiAudioVolumeManager)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    int32_t appUid = 0;
    NapiParamUtils::GetValueInt32(env, appUid, args[PARAM1]);
    if (napiAudioVolumeManager->appVolumeChangeCallbackForUidNapi_ == nullptr) {
        napiAudioVolumeManager->appVolumeChangeCallbackForUidNapi_ =
            std::make_shared<NapiAudioManagerAppVolumeChangeCallback>(env);
    }
    CHECK_AND_RETURN_RET_LOG(napiAudioVolumeManager->appVolumeChangeCallbackForUidNapi_ != nullptr,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_SYSTEM, "System error"),
        "RegisterAppVolumeChangeForUidCallback: Memory Allocation Failed !");
    int32_t ret = napiAudioVolumeManager->audioSystemMngr_->SetAppVolumeCallbackForUid(appUid,
        napiAudioVolumeManager->appVolumeChangeCallbackForUidNapi_);
    if (ret != SUCCESS) {
        if (ret == ERROR_INVALID_PARAM) {
            NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
                "Parameter verification failed. Invalid callback.");
        } else if (ret == ERR_PERMISSION_DENIED) {
            NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED, "Permission denied");
        } else {
            NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_SYSTEM, "System error");
        }
    }
    std::shared_ptr<NapiAudioManagerAppVolumeChangeCallback> cb =
        std::static_pointer_cast<NapiAudioManagerAppVolumeChangeCallback>(
        napiAudioVolumeManager->appVolumeChangeCallbackForUidNapi_);
    cb->SaveVolumeChangeCallbackForUidReference(cbName, args[PARAM2], appUid);
    if (!cb->GetManagerAppVolumeChangeTsfnFlag()) {
        cb->CreateManagerAppVolumeChangeTsfn(env);
    }
    return result;
}

napi_value NapiAudioVolumeManager::RegisterSelfAppVolumeChangeCallback(napi_env env,
    napi_value *args, const std::string &cbName, NapiAudioVolumeManager *napiAudioVolumeManager)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    if (napiAudioVolumeManager->selfAppVolumeChangeCallbackNapi_ == nullptr) {
        napiAudioVolumeManager->selfAppVolumeChangeCallbackNapi_ =
            std::make_shared<NapiAudioManagerAppVolumeChangeCallback>(env);
    }
    CHECK_AND_RETURN_RET_LOG(napiAudioVolumeManager->selfAppVolumeChangeCallbackNapi_ != nullptr,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_SYSTEM, "System error"),
        "napiAudioVolumeManager: Memory Allocation Failed !");
    int32_t ret = napiAudioVolumeManager->audioSystemMngr_->SetSelfAppVolumeCallback(
        napiAudioVolumeManager->selfAppVolumeChangeCallbackNapi_);
    if (ret != SUCCESS) {
        if (ret == ERROR_INVALID_PARAM) {
            NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
                "Parameter verification failed. Invalid callback.");
        } else {
            NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_SYSTEM, "System error");
        }
    }
    std::shared_ptr<NapiAudioManagerAppVolumeChangeCallback> cb =
        std::static_pointer_cast<NapiAudioManagerAppVolumeChangeCallback>(
        napiAudioVolumeManager->selfAppVolumeChangeCallbackNapi_);
    cb->SaveSelfVolumdChangeCallbackReference(cbName, args[PARAM1]);
    if (!cb->GetManagerAppVolumeChangeTsfnFlag()) {
        cb->CreateManagerAppVolumeChangeTsfn(env);
    }
    return result;
}

napi_value NapiAudioVolumeManager::RegisterStreamVolumeChangeCallback(napi_env env, napi_value *args,
    const std::string &cbName, NapiAudioVolumeManager *napiAudioVolumeManager)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    int32_t streamUsage = 0;
    NapiParamUtils::GetValueInt32(env, streamUsage, args[PARAM1]);

    NapiDfxUtils::SendVolumeApiInvokeEvent(static_cast<int32_t>(getuid()),
        "on streamVolumeChange", streamUsage);
    
    std::lock_guard<std::mutex> lock(napiAudioVolumeManager->streamMapMutex_);
    for (auto &item : napiAudioVolumeManager->streamVolumeChangeCallbackNapiMap_) {
        std::shared_ptr<NapiAudioStreamVolumeChangeCallback> cb =
            std::static_pointer_cast<NapiAudioStreamVolumeChangeCallback>(item.second);
        if (cb->ContainSameJsCallback(args[PARAM2])) {
            if (item.first == streamUsage) {
                AUDIO_INFO_LOG("callback already exists for streamUsage %{public}d", streamUsage);
                return result;
            } else {
                AUDIO_ERR_LOG("callback exists for another streamUsage %{public}d", item.first);
                NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
                    "Parameter verification failed. Invalid callback.");
            }
        }
    }

    if (napiAudioVolumeManager->streamVolumeChangeCallbackNapiMap_.find(streamUsage) ==
        napiAudioVolumeManager->streamVolumeChangeCallbackNapiMap_.end()) {
        napiAudioVolumeManager->streamVolumeChangeCallbackNapiMap_[streamUsage] =
            std::make_shared<NapiAudioStreamVolumeChangeCallback>(env);
        int32_t ret = napiAudioVolumeManager->audioSystemMngr_->RegisterStreamVolumeChangeCallback(
            napiAudioVolumeManager->cachedClientId_,
            { NapiAudioEnum::GetNativeStreamUsage(streamUsage) },
            napiAudioVolumeManager->streamVolumeChangeCallbackNapiMap_[streamUsage]);
        if (ret != SUCCESS) {
            if (ret == ERROR_INVALID_PARAM) {
                NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
                    "Parameter verification failed. Invalid callback.");
            } else {
                NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_SYSTEM,
                    "System error.");
            }
        }
    }

    std::shared_ptr<NapiAudioStreamVolumeChangeCallback> cb =
        std::static_pointer_cast<NapiAudioStreamVolumeChangeCallback>(
            napiAudioVolumeManager->streamVolumeChangeCallbackNapiMap_[streamUsage]);
    cb->SaveCallbackReference(cbName, args[PARAM2]);
    if (!cb->GetVolumeTsfnFlag()) {
        cb->CreateStreamVolumeChangeTsfn(env);
    }
    return result;
}

napi_value NapiAudioVolumeManager::RegisterSystemVolumeChangeCallback(napi_env env, napi_value *args,
    const std::string &cbName, NapiAudioVolumeManager *napiAudioVolumeManager)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED, "No system permission"),
        "No system permission");
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    if (napiAudioVolumeManager->systemVolumeChangeCallbackNapi_ == nullptr) {
        napiAudioVolumeManager->systemVolumeChangeCallbackNapi_ = std::make_shared<
            NapiAudioSystemVolumeChangeCallback>(env);
        int32_t ret = napiAudioVolumeManager->audioSystemMngr_->RegisterSystemVolumeChangeCallback(
            napiAudioVolumeManager->cachedClientId_, napiAudioVolumeManager->systemVolumeChangeCallbackNapi_);
        if (ret != SUCCESS) {
            if (ret == ERROR_INVALID_PARAM) {
                NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
                    "Parameter verification failed. Invalid callback.");
            } else {
                NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_SYSTEM,
                    "System error.");
            }
        }
    }
    std::shared_ptr<NapiAudioSystemVolumeChangeCallback> cb =
        std::static_pointer_cast<NapiAudioSystemVolumeChangeCallback>(
            napiAudioVolumeManager->systemVolumeChangeCallbackNapi_);
    cb->SaveCallbackReference(cbName, args[PARAM1]);
    if (!cb->GetVolumeTsfnFlag()) {
        cb->CreateSystemVolumeChangeTsfn(env);
    }
    return result;
}

napi_value NapiAudioVolumeManager::RegisterVolumeDegreeChangeCallback(napi_env env, napi_value *args,
    const std::string &cbName, NapiAudioVolumeManager *napiAudioVolumeManager)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED, "No system permission"),
        "No system permission");

    CHECK_AND_RETURN_RET_LOG(napiAudioVolumeManager && napiAudioVolumeManager->audioSystemMngr_,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_NO_MEMORY), "audio volume manager is nullptr.");
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    if (napiAudioVolumeManager->volumeDegreeCallbackNapi_ == nullptr) {
        napiAudioVolumeManager->volumeDegreeCallbackNapi_ = std::make_shared<
            NapiAudioVolumeKeyEventEx>(env);
        int32_t ret = napiAudioVolumeManager->audioSystemMngr_->RegisterVolumeDegreeCallback(
            napiAudioVolumeManager->cachedClientId_, napiAudioVolumeManager->volumeDegreeCallbackNapi_);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS,
            NapiAudioError::ThrowErrorAndReturn(env, ret), "Register Failed %{public}d", ret);
    }

    std::shared_ptr<NapiAudioVolumeKeyEventEx> cb =
        std::static_pointer_cast<NapiAudioVolumeKeyEventEx>(napiAudioVolumeManager->volumeDegreeCallbackNapi_);
    CHECK_AND_RETURN_RET_LOG(cb && args,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_NO_MEMORY), "callback is nullptr");
    cb->SaveCallbackReference(cbName, args[PARAM1]);
    if (!cb->GetVolumeDegreeTsfnFlag()) {
        cb->CreateVolumeDegreeTsfn(env);
    }
    return result;
}

napi_value NapiAudioVolumeManager::On(napi_env env, napi_callback_info info)
{
    napi_value undefinedResult = nullptr;
    napi_get_undefined(env, &undefinedResult);

    const size_t minArgCount = ARGS_TWO;
    size_t argCount = ARGS_THREE;
    napi_value args[minArgCount + PARAM1] = {nullptr, nullptr, nullptr};
    napi_value jsThis = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argCount, args, &jsThis, nullptr);
    if (status != napi_ok || argCount < minArgCount) {
        AUDIO_ERR_LOG("On fail to napi_get_cb_info/Requires min 2 parameters");
        NapiAudioError::ThrowError(env, NAPI_ERR_INPUT_INVALID,
            "mandatory parameters are left unspecified");
    }

    napi_valuetype eventType = napi_undefined;
    if (napi_typeof(env, args[PARAM0], &eventType) != napi_ok || eventType != napi_string) {
        NapiAudioError::ThrowError(env, NAPI_ERR_INPUT_INVALID,
            "incorrect parameter types: The type of eventType must be string");
        return undefinedResult;
    }
    std::string callbackName = NapiParamUtils::GetStringArgument(env, args[PARAM0]);
    AUDIO_INFO_LOG("On callbackName: %{public}s", callbackName.c_str());

    napi_valuetype handler = napi_undefined;
    if (napi_typeof(env, args[argCount -1], &handler) != napi_ok || handler != napi_function) {
        AUDIO_ERR_LOG("On type mismatch for parameter 2");
        NapiAudioError::ThrowError(env, NAPI_ERR_INPUT_INVALID,
            "incorrect parameter types: The type of callback must be function");
        return undefinedResult;
    }

    return RegisterCallback(env, jsThis, argCount, args, callbackName);
}

napi_value NapiAudioVolumeManager::Off(napi_env env, napi_callback_info info)
{
    napi_value undefinedResult = nullptr;
    napi_get_undefined(env, &undefinedResult);

    const size_t minArgCount = ARGS_ONE;
    size_t argc = ARGS_TWO;
    napi_value args[minArgCount + PARAM2] = {nullptr, nullptr, nullptr};
    napi_value jsThis = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr);
    if (status != napi_ok || argc < minArgCount) {
        AUDIO_ERR_LOG("Off fail to napi_get_cb_info/Requires min 1 parameters");
        NapiAudioError::ThrowError(env, NAPI_ERR_INPUT_INVALID,
            "mandatory parameters are left unspecified");
    }
    napi_valuetype eventType = napi_undefined;
    if (napi_typeof(env, args[PARAM0], &eventType) != napi_ok || eventType != napi_string) {
        NapiAudioError::ThrowError(env, NAPI_ERR_INPUT_INVALID,
            "incorrect parameter types: The type of eventType must be string");
        return undefinedResult;
    }
    std::string callbackName = NapiParamUtils::GetStringArgument(env, args[PARAM0]);
    AUDIO_INFO_LOG("Off callbackName: %{public}s", callbackName.c_str());

    return UnregisterCallback(env, jsThis, argc, args, callbackName);
}

napi_value NapiAudioVolumeManager::UnregisterCallback(napi_env env, napi_value jsThis,
    size_t argc, napi_value *args, const std::string &cbName)
{
    napi_value undefinedResult = nullptr;
    NapiAudioVolumeManager *napiVolumeManager = nullptr;
    napi_status status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napiVolumeManager));
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_SYSTEM), "status error");
    CHECK_AND_RETURN_RET_LOG(napiVolumeManager != nullptr, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_NO_MEMORY), "napiVolumeManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiVolumeManager->audioSystemMngr_ != nullptr, NapiAudioError::ThrowErrorAndReturn(
        env, NAPI_ERR_NO_MEMORY), "audioSystemMngr_ is nullptr");

    if (!cbName.compare(VOLUME_KEY_EVENT_CALLBACK_NAME)) {
        napi_value callback = nullptr;
        if (argc == ARGS_TWO) {
            callback = args[PARAM1];
        }
        if (callback != nullptr) {
            std::shared_ptr<NapiAudioVolumeKeyEvent> cb = GetVolumeEventNapiCallback(callback, napiVolumeManager);
            CHECK_AND_RETURN_RET_LOG(cb != nullptr, undefinedResult, "NapiAudioVolumeKeyEvent is nullptr");
            int32_t ret = napiVolumeManager->audioSystemMngr_->UnregisterVolumeKeyEventCallback(
                napiVolumeManager->cachedClientId_, cb);
            CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, undefinedResult, "Unset of VolumeKeyEventCallback failed");
            napiVolumeManager->volumeKeyEventCallbackNapiList_.remove(cb);
            napiVolumeManager->volumeKeyEventCallbackNapi_.reset();
            napiVolumeManager->volumeKeyEventCallbackNapi_ = nullptr;
            return undefinedResult;
        } else {
            int32_t ret = napiVolumeManager->audioSystemMngr_->UnregisterVolumeKeyEventCallback(
                napiVolumeManager->cachedClientId_);
            CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, undefinedResult, "Unset of VolumeKeyEventCallback failed");
            napiVolumeManager->volumeKeyEventCallbackNapiList_.clear();
            napiVolumeManager->volumeKeyEventCallbackNapi_.reset();
            napiVolumeManager->volumeKeyEventCallbackNapi_ = nullptr;
            return undefinedResult;
        }
    } else if (!cbName.compare(APP_VOLUME_CHANGE_CALLBACK_NAME)) {
        UnregisterSelfAppVolumeChangeCallback(env, args[PARAM1], argc, napiVolumeManager);
    } else if (!cbName.compare(APP_VOLUME_CHANGE_CALLBACK_NAME_FOR_UID)) {
        UnregisterAppVolumeChangeForUidCallback(env, args[PARAM1], args, argc, napiVolumeManager);
    } else if (!cbName.compare(ACTIVE_VOLUME_TYPE_CHANGE_CALLBACK_NAME)) {
        UnregisterActiveVolumeTypeChangeCallback(env, args[PARAM1], args, argc, napiVolumeManager);
    } else {
        UnregisterCallbackFir(env, args, argc, cbName, napiVolumeManager);
    }
    return undefinedResult;
}

void NapiAudioVolumeManager::UnregisterCallbackFir(napi_env env, napi_value *args,
    size_t argc, const std::string &cbName, NapiAudioVolumeManager *napiVolumeManager)
{
    if (!cbName.compare(AUDIO_STREAM_VOLUME_CHANGE_CALLBACK_NAME)) {
        UnregisterStreamVolumeChangeCallback(env, args, argc, napiVolumeManager);
    } else if (!cbName.compare(AUDIO_SYSTEM_VOLUME_CHANGE_CALLBACK_NAME)) {
        UnregisterSystemVolumeChangeCallback(env, args, argc, napiVolumeManager);
    } else if (!cbName.compare(VOLUME_DEGREE_CHANGE_EVENT_CALLBACK_NAME)) {
        UnregisterVolumeDegreeChangeCallback(env, args, argc, napiVolumeManager);
    } else {
        AUDIO_ERR_LOG("No such callback supported");
        NapiAudioError::ThrowError(env, NAPI_ERR_INVALID_PARAM,
            "parameter verification failed: The param of type is not supported");
    }
}

void NapiAudioVolumeManager::UnregisterActiveVolumeTypeChangeCallback(napi_env env, napi_value callback,
    napi_value *args, size_t argc, NapiAudioVolumeManager *napiAudioVolumeManager)
{
    if (napiAudioVolumeManager == nullptr ||
        napiAudioVolumeManager->activeVolumeTypeChangeCallbackNapi_ == nullptr) {
        AUDIO_ERR_LOG("napiAudioVolumeManager is nullptr");
        NapiAudioError::ThrowError(env, "unregister activeVolumeType callback failed", NAPI_ERR_SYSTEM);
        return;
    }
    std::shared_ptr<NapiAudioManagerActiveVolumeTypeChangeCallback> cb =
        std::static_pointer_cast<NapiAudioManagerActiveVolumeTypeChangeCallback>(
        napiAudioVolumeManager->activeVolumeTypeChangeCallbackNapi_);
    if (callback != nullptr && argc == ARGS_TWO) {
        cb->RemoveSelfActiveVolumeTypeChangeCbRef(env, callback);
    }
    if (argc == ARGS_ONE || cb->GetActiveVolumeTypeChangeListSize() == 0) {
        napiAudioVolumeManager->audioSystemMngr_->UnsetActiveVolumeTypeCallback(
            napiAudioVolumeManager->activeVolumeTypeChangeCallbackNapi_);
        napiAudioVolumeManager->activeVolumeTypeChangeCallbackNapi_.reset();
        napiAudioVolumeManager->activeVolumeTypeChangeCallbackNapi_ = nullptr;
        cb->RemoveAllActiveVolumeTypeChangeCbRef();
    }
}

void NapiAudioVolumeManager::UnregisterAppVolumeChangeForUidCallback(napi_env env, napi_value callback,
    napi_value *args, size_t argc, NapiAudioVolumeManager *napiAudioVolumeManager)
{
    if (napiAudioVolumeManager == nullptr ||
        napiAudioVolumeManager->appVolumeChangeCallbackForUidNapi_ == nullptr) {
        NapiAudioError::ThrowError(env, "UnregisterAppVolumeChangeForUidCallback failed", NAPI_ERR_SYSTEM);
        AUDIO_ERR_LOG("napiAudioVolumeManager is nullptr");
        return;
    }
    std::shared_ptr<NapiAudioManagerAppVolumeChangeCallback> cb =
        std::static_pointer_cast<NapiAudioManagerAppVolumeChangeCallback>(
        napiAudioVolumeManager->appVolumeChangeCallbackForUidNapi_);
    if (callback != nullptr) {
        cb->RemoveAudioVolumeChangeForUidCbRef(env, callback);
    }

    if (argc == ARGS_ONE || cb->GetAppVolumeChangeForUidListSize() == 0) {
        napiAudioVolumeManager->audioSystemMngr_->UnsetAppVolumeCallbackForUid();
        napiAudioVolumeManager->appVolumeChangeCallbackForUidNapi_.reset();
        napiAudioVolumeManager->appVolumeChangeCallbackForUidNapi_ = nullptr;
        cb->RemoveAllAudioVolumeChangeForUidCbRef();
    }
}

void NapiAudioVolumeManager::UnregisterSelfAppVolumeChangeCallback(napi_env env, napi_value callback,
    size_t argc, NapiAudioVolumeManager *napiAudioVolumeManager)
{
    if (napiAudioVolumeManager == nullptr ||
        napiAudioVolumeManager->selfAppVolumeChangeCallbackNapi_ == nullptr) {
        AUDIO_ERR_LOG("napiAudioVolumeManager is nullptr");
        NapiAudioError::ThrowError(env, "UnregisterSelfAppVolumeChangeCallback failed", NAPI_ERR_SYSTEM);
        return;
    }
    std::shared_ptr<NapiAudioManagerAppVolumeChangeCallback> cb =
        std::static_pointer_cast<NapiAudioManagerAppVolumeChangeCallback>(
        napiAudioVolumeManager->selfAppVolumeChangeCallbackNapi_);
    if (callback != nullptr && argc == ARGS_TWO) {
        cb->RemoveSelfAudioVolumeChangeCbRef(env, callback);
    }
    if (argc == ARGS_ONE || cb->GetSelfAppVolumeChangeListSize() == 0) {
        napiAudioVolumeManager->audioSystemMngr_->UnsetSelfAppVolumeCallback();
        napiAudioVolumeManager->selfAppVolumeChangeCallbackNapi_.reset();
        napiAudioVolumeManager->selfAppVolumeChangeCallbackNapi_ = nullptr;
        cb->RemoveAllSelfAudioVolumeChangeCbRef();
    }
}

void NapiAudioVolumeManager::UnregisterStreamVolumeChangeCallback(napi_env env, napi_value *args,
    size_t argc, NapiAudioVolumeManager *napiAudioVolumeManager)
{
    std::lock_guard<std::mutex> lock(napiAudioVolumeManager->streamMapMutex_);

    CHECK_AND_RETURN_LOG(napiAudioVolumeManager != nullptr, "napiAudioVolumeManager is nullptr");
    CHECK_AND_RETURN_LOG(napiAudioVolumeManager->audioSystemMngr_ != nullptr, "audioSystemMngr_ is nullptr");
    CHECK_AND_RETURN_LOG(!napiAudioVolumeManager->streamVolumeChangeCallbackNapiMap_.empty(),
        "streamVolumeChangeCallbackNapiMap_ is empty");
    
    if (argc == ARGS_ONE) {
        for (auto &item : napiAudioVolumeManager->streamVolumeChangeCallbackNapiMap_) {
            std::shared_ptr<NapiAudioStreamVolumeChangeCallback> cb =
                std::static_pointer_cast<NapiAudioStreamVolumeChangeCallback>(item.second);
            cb->RemoveAllCallbackReference();
            int32_t ret = napiAudioVolumeManager->audioSystemMngr_->UnregisterStreamVolumeChangeCallback(
                napiAudioVolumeManager->cachedClientId_, item.second);
            if (ret != SUCCESS) {
                AUDIO_ERR_LOG("UnregisterStreamVolumeChangeCallback failed");
            }
        }
        napiAudioVolumeManager->streamVolumeChangeCallbackNapiMap_.clear();
        return;
    }

    napi_value callback = args[PARAM1];
    CHECK_AND_RETURN_LOG(callback != nullptr, "callback is nullptr");

    for (auto it = napiAudioVolumeManager->streamVolumeChangeCallbackNapiMap_.begin();
         it != napiAudioVolumeManager->streamVolumeChangeCallbackNapiMap_.end(); ++it) {
        std::shared_ptr<NapiAudioStreamVolumeChangeCallback> cb =
            std::static_pointer_cast<NapiAudioStreamVolumeChangeCallback>(it->second);
        if (!cb->ContainSameJsCallback(callback)) {
            continue;
        }

        cb->RemoveCallbackReference(env, callback);

        if (cb->GetStreamVolumeCbListSize() == 0) {
            int32_t ret = napiAudioVolumeManager->audioSystemMngr_->UnregisterStreamVolumeChangeCallback(
                napiAudioVolumeManager->cachedClientId_, it->second);
            CHECK_AND_RETURN_LOG(ret == SUCCESS, "UnregisterStreamVolumeChangeCallback failed");
            int32_t key = it->first;
            napiAudioVolumeManager->streamVolumeChangeCallbackNapiMap_.erase(key);
        }
        break;
    }
}

void NapiAudioVolumeManager::UnregisterSystemVolumeChangeCallback(napi_env env, napi_value *args,
    size_t argc, NapiAudioVolumeManager *napiAudioVolumeManager)
{
    CHECK_AND_RETURN_LOG(napiAudioVolumeManager != nullptr, "napiAudioVolumeManager is nullptr");
    CHECK_AND_RETURN_LOG(napiAudioVolumeManager->audioSystemMngr_ != nullptr, "audioSystemMngr_ is nullptr");
    CHECK_AND_RETURN_LOG(napiAudioVolumeManager->systemVolumeChangeCallbackNapi_ != nullptr,
        "systemVolumeChangeCallbackNapi_ is empty");

    std::shared_ptr<NapiAudioSystemVolumeChangeCallback> cb =
        std::static_pointer_cast<NapiAudioSystemVolumeChangeCallback>(
            napiAudioVolumeManager->systemVolumeChangeCallbackNapi_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "static_pointer_cast failed");

    if (argc == ARGS_ONE) {
        cb->RemoveAllCallbackReference();
        int32_t ret = napiAudioVolumeManager->audioSystemMngr_->UnregisterSystemVolumeChangeCallback(
            napiAudioVolumeManager->cachedClientId_, napiAudioVolumeManager->systemVolumeChangeCallbackNapi_);
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "UnregisterSystemVolumeChangeCallback failed");
        napiAudioVolumeManager->systemVolumeChangeCallbackNapi_.reset();
        return;
    }

    napi_value callback = args[PARAM1];
    if (callback != nullptr) {
        CHECK_AND_RETURN_LOG(cb->ContainSameJsCallback(callback), "callback not found");
        cb->RemoveCallbackReference(env, callback);
    }
    if (cb->GetSystemVolumeCbListSize() == 0) {
        int32_t ret = napiAudioVolumeManager->audioSystemMngr_->UnregisterSystemVolumeChangeCallback(
            napiAudioVolumeManager->cachedClientId_, napiAudioVolumeManager->systemVolumeChangeCallbackNapi_);
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "UnregisterSystemVolumeChangeCallback failed");
        napiAudioVolumeManager->systemVolumeChangeCallbackNapi_.reset();
    }
}

void NapiAudioVolumeManager::UnregisterVolumeDegreeChangeCallback(napi_env env, napi_value *args,
    size_t argc, NapiAudioVolumeManager *napiAudioVolumeManager)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(),
        NapiAudioError::ThrowError(env, NAPI_ERR_PERMISSION_DENIED, "No system permission"),
        "No system permission");

    napi_value callback = nullptr;
    CHECK_AND_RETURN_LOG(args != nullptr, "args is nullptr");
    CHECK_AND_RETURN_LOG(napiAudioVolumeManager != nullptr, "napiAudioVolumeManager is nullptr");
    CHECK_AND_RETURN_LOG(napiAudioVolumeManager->audioSystemMngr_ != nullptr, "audioSystemMngr_ is nullptr");
    if (argc == ARGS_TWO) {
        callback = args[PARAM1];
    }

    std::shared_ptr<NapiAudioVolumeKeyEventEx> cb =
        std::static_pointer_cast<NapiAudioVolumeKeyEventEx>(napiAudioVolumeManager->volumeDegreeCallbackNapi_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "static_pointer_cast failed");

    if (callback != nullptr) {
        cb->RemoveCallbackReference(env, callback);
    }
    if (callback == nullptr || cb->GetVolumeKeyEventCbListSize() == 0) {
        int32_t ret = napiAudioVolumeManager->audioSystemMngr_->UnregisterVolumeDegreeCallback(
            napiAudioVolumeManager->cachedClientId_, napiAudioVolumeManager->volumeDegreeCallbackNapi_);
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "UnsetAudioSceneChangeCallback Failed");
        napiAudioVolumeManager->volumeDegreeCallbackNapi_.reset();
        napiAudioVolumeManager->volumeDegreeCallbackNapi_ = nullptr;
        cb->RemoveAllCallbackReference();
    }
}

std::shared_ptr<NapiAudioVolumeKeyEvent> NapiAudioVolumeManager::GetVolumeEventNapiCallback(napi_value argv,
    NapiAudioVolumeManager *napiVolumeManager)
{
    std::shared_ptr<NapiAudioVolumeKeyEvent> cb = nullptr;
    for (auto &iter : napiVolumeManager->volumeKeyEventCallbackNapiList_) {
        if (iter->ContainSameJsCallback(argv)) {
            cb = iter;
        }
    }
    return cb;
}

napi_value NapiAudioVolumeManager::ForceVolumeKeyControlType(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioVolumeManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("ForceVolumeKeyControlType failed : no memory");
        NapiAudioError::ThrowError(env, "ForceVolumeKeyControlType failed : no memory", NAPI_ERR_SYSTEM);
        return NapiParamUtils::GetUndefinedValue(env);
    }
    size_t argNum = 0;
    auto inputParser = [env, context, &argNum](size_t argc, napi_value *argv) {
        argNum = argc;
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_TWO, " Invalid arguments count or types.",
            NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->volumeType, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get volumeType failed",
            NAPI_ERR_INPUT_INVALID);
        context->status = NapiParamUtils::GetValueInt32(env, context->duration, argv[PARAM1]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get duration failed",
            NAPI_ERR_INPUT_INVALID);
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioVolumeManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioVolumeManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioVolumeManagerStatus(napiAudioVolumeManager, context),
            "audio volume manager state is error.");
        context->intValue = napiAudioVolumeManager->audioSystemMngr_->ForceVolumeKeyControlType(
            NapiAudioEnum::GetNativeAudioVolumeType(context->volumeType), context->duration);
        CHECK_AND_RETURN(context->intValue != SUCCESS);
        if (context->intValue == ERR_PERMISSION_DENIED) {
            context->SignError(NAPI_ERR_NO_PERMISSION);
        } else if (context->intValue == ERR_SYSTEM_PERMISSION_DENIED) {
            context->SignError(NAPI_ERR_PERMISSION_DENIED);
        } else if (context->intValue == ERR_INVALID_PARAM) {
            context->SignError(NAPI_ERR_INVALID_PARAM);
        } else {
            context->SignError(NAPI_ERR_SYSTEM, "ForceVolumeKeyControlType fail.");
        }
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "ForceVolumeKeyControlType", executor, complete);
}

napi_value NapiAudioVolumeManager::GetSystemVolumePercentage(napi_env env, napi_callback_info info)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED, "No system permission"),
        "No system permission");
    napi_value result = nullptr;
    size_t argc = ARGS_ONE;
    napi_value args[ARGS_ONE] = {};
    auto *napiAudioVolumeManager = GetParamWithSync(env, info, argc, args);
    CHECK_AND_RETURN_RET_LOG(argc >= ARGS_ONE, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INVALID_PARAM, "mandatory parameters are left unspecified"), "invalid arguments");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, args[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_number, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INVALID_PARAM, "incorrect parameter types: The type of audioVolumeType must be number"),
        "invalid valueType");

    int32_t volType;
    NapiParamUtils::GetValueInt32(env, volType, args[PARAM0]);
    CHECK_AND_RETURN_RET_LOG(NapiAudioEnum::IsLegalInputArgumentVolType(volType),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
        "parameter verification failed: The param of volType must be enum AudioVolumeType"),
        "get volType failed");

    NapiDfxUtils::SendVolumeApiInvokeEvent(static_cast<int32_t>(getuid()),
        "GetSystemVolumePercentage", volType);

    if (napiAudioVolumeManager == nullptr || napiAudioVolumeManager->audioSystemMngr_ == nullptr) {
        AUDIO_ERR_LOG("napiAudioVolumeManager or audioSystemMngr  is nullptr!");
        return NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_NO_MEMORY,
            "napiAudioVolumeManager nullptr");
    }
    int32_t systemVolume = napiAudioVolumeManager->audioSystemMngr_->GetVolumeDegree(
        NapiAudioEnum::GetNativeAudioVolumeType(volType));
    NapiParamUtils::SetValueInt32(env, systemVolume, result);
    return result;
}

napi_value NapiAudioVolumeManager::SetSystemVolumePercentage(napi_env env, napi_callback_info info)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED, "No system permission"),
        "No system permission");
    auto context = std::make_shared<AudioVolumeManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("failed : no memory");
        NapiAudioError::ThrowError(env, "failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_TWO, "invalid arguments",
            NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->volumeType, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get volumeType failed",
            NAPI_ERR_INVALID_PARAM);
        if (!NapiAudioEnum::IsLegalInputArgumentVolType(context->volumeType)) {
            context->SignError(context->errCode == NAPI_ERR_INVALID_PARAM?
                NAPI_ERR_INVALID_PARAM : NAPI_ERR_UNSUPPORTED);
        }
        context->status = NapiParamUtils::GetValueInt32(env, context->volDegree, argv[PARAM1]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get volDegree failed",
            NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioVolumeManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioVolumeManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioVolumeManagerStatus(napiAudioVolumeManager, context),
            "audio volume manager state is error.");
        context->intValue = napiAudioVolumeManager->audioSystemMngr_->SetVolumeDegree(
            NapiAudioEnum::GetNativeAudioVolumeType(context->volumeType), context->volDegree);
        CHECK_AND_RETURN(context->intValue != SUCCESS);
        if (context->intValue == ERR_PERMISSION_DENIED) {
            context->SignError(NAPI_ERR_NO_PERMISSION);
        } else if (context->intValue == ERR_SYSTEM_PERMISSION_DENIED) {
            context->SignError(NAPI_ERR_PERMISSION_DENIED);
        } else if (context->intValue == ERR_INVALID_PARAM) {
            context->SignError(NAPI_ERR_INVALID_PARAM);
        } else {
            context->SignError(NAPI_ERR_SYSTEM, "failed");
        }
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SetSystemVolumePercentage", executor, complete);
}

napi_value NapiAudioVolumeManager::GetMinSystemVolumePercentage(napi_env env, napi_callback_info info)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED, "No system permission"),
        "No system permission");
    napi_value result = nullptr;
    size_t argc = ARGS_ONE;
    napi_value args[ARGS_ONE] = {};
    auto *napiAudioVolumeManager = GetParamWithSync(env, info, argc, args);
    CHECK_AND_RETURN_RET_LOG(argc >= ARGS_ONE, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INVALID_PARAM, "mandatory parameters are left unspecified"), "invalid arguments");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, args[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_number, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INVALID_PARAM, "incorrect parameter types: The type of audioVolumeType must be number"),
        "invalid valueType");

    int32_t volType;
    NapiParamUtils::GetValueInt32(env, volType, args[PARAM0]);
    CHECK_AND_RETURN_RET_LOG(NapiAudioEnum::IsLegalInputArgumentVolType(volType),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
            "parameter verification failed: The param of volType must be enum AudioVolumeType"),
        "get volType failed");

    NapiDfxUtils::SendVolumeApiInvokeEvent(static_cast<int32_t>(getuid()),
        "getMinSystemVolumePercentage", volType);

    if (napiAudioVolumeManager == nullptr || napiAudioVolumeManager->audioSystemMngr_ == nullptr) {
        AUDIO_ERR_LOG("napiAudioVolumeManager or audioSystemMngr  is nullptr!");
        return NapiParamUtils::GetUndefinedValue(env);
    }
    int32_t minSystemVolume = napiAudioVolumeManager->audioSystemMngr_->GetMinVolumeDegree(
        NapiAudioEnum::GetNativeAudioVolumeType(volType));
    NapiParamUtils::SetValueInt32(env, minSystemVolume, result);
    return result;
}
}  // namespace AudioStandard
}  // namespace OHOS
