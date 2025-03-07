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
#include "napi_audio_enum.h"
#include "napi_audio_error.h"
#include "napi_param_utils.h"
#include "audio_errors.h"
#include "audio_manager_log.h"

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
    CHECK_AND_RETURN_RET_LOG(napiAudioVolumeManager != nullptr && napiAudioVolumeManager->audioSystemMngr_ !=
        nullptr, napiAudioVolumeManager, "GetParamWithSync fail to napi_unwrap");
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
        DECLARE_NAPI_FUNCTION("on", On),
        DECLARE_NAPI_FUNCTION("off", Off),
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
    int32_t argNum = 0;
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
        context->volLevel = napiAudioVolumeManager->audioSystemMngr_->GetSelfAppVolume();
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
    int32_t argNum = 0;
    auto inputParser = [env, context, &argNum](size_t argc, napi_value *argv) {
        argNum = argc;
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments", NAPI_ERR_INVALID_PARAM);
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
        context->volLevel = napiAudioVolumeManager->audioSystemMngr_->GetAppVolume(context->appUid);
        if (context->volLevel == ERR_PERMISSION_DENIED) {
            context->SignError(NAPI_ERR_NO_PERMISSION);
        } else if (context->volLevel == ERR_SYSTEM_PERMISSION_DENIED) {
            context->SignError(NAPI_ERR_PERMISSION_DENIED);
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
    int32_t argNum = 0;
    auto inputParser = [env, context, &argNum](size_t argc, napi_value *argv) {
        argNum = argc;
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_TWO, "invalid arguments", NAPI_ERR_INVALID_PARAM);
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
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->intValue == SUCCESS, "set appvolume failed",
            NAPI_ERR_SYSTEM);
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
    int32_t argNum = 0;
    auto inputParser = [env, context, &argNum](size_t argc, napi_value *argv) {
        argNum = argc;
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments", NAPI_ERR_INVALID_PARAM);
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
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->intValue == SUCCESS, "set appvolume failed",
            NAPI_ERR_SYSTEM);
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
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_TWO, "invalid arguments", NAPI_ERR_INVALID_PARAM);
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
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->intValue == SUCCESS, "SetAppVolumeMuted failed", NAPI_ERR_SYSTEM);
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
        context->isMute = napiAudioVolumeManager->audioSystemMngr_->IsAppVolumeMute(
            context->appUid, context->isOwned);
    };
    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueBoolean(env, context->isMute, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "IsAppVolumeMutedForUid", executor, complete);
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

    auto complete = [env, context](napi_value &output) {
        output = NapiAudioVolumeGroupManager::CreateAudioVolumeGroupManagerWrapper(env, context->groupId);
        NapiAudioVolumeGroupManager::isConstructSuccess_ = SUCCESS;
    };
    return NapiAsyncWork::Enqueue(env, context, "GetVolumeGroupManager", nullptr, complete);
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

    result = NapiAudioVolumeGroupManager::CreateAudioVolumeGroupManagerWrapper(env, groupId);

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
        undefinedResult = RegisterSelfAppVolumeChangeCallback(env, args, cbName,
            napiVolumeManager);
    } else if (!cbName.compare(APP_VOLUME_CHANGE_CALLBACK_NAME_FOR_UID)) {
        undefinedResult = RegisterAppVolumeChangeForUidCallback(env, args, cbName,
            napiVolumeManager);
    } else {
        AUDIO_ERR_LOG("No such callback supported");
        NapiAudioError::ThrowError(env, NAPI_ERR_INVALID_PARAM,
            "parameter verification failed: The param of type is not supported");
    }
    return undefinedResult;
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
    CHECK_AND_RETURN_RET_LOG(napiAudioVolumeManager->appVolumeChangeCallbackForUidNapi_, result,
        "RegisterAppVolumeChangeForUidCallback: Memory Allocation Failed !");
    int32_t ret = napiAudioVolumeManager->audioSystemMngr_->SetAppVolumeCallbackForUid(appUid,
        napiAudioVolumeManager->appVolumeChangeCallbackForUidNapi_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, result, "SetAppVolumeCallbackForUid Failed");
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
    CHECK_AND_RETURN_RET_LOG(napiAudioVolumeManager->selfAppVolumeChangeCallbackNapi_, result,
        "napiAudioVolumeManager: Memory Allocation Failed !");
    int32_t ret = napiAudioVolumeManager->audioSystemMngr_->SetSelfAppVolumeCallback(
        napiAudioVolumeManager->selfAppVolumeChangeCallbackNapi_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, result, "SetSelfAppVolumeCallback Failed");
    std::shared_ptr<NapiAudioManagerAppVolumeChangeCallback> cb =
        std::static_pointer_cast<NapiAudioManagerAppVolumeChangeCallback>(
        napiAudioVolumeManager->selfAppVolumeChangeCallbackNapi_);
    cb->SaveSelfVolumdChangeCallbackReference(cbName, args[PARAM1]);
    if (!cb->GetManagerAppVolumeChangeTsfnFlag()) {
        cb->CreateManagerAppVolumeChangeTsfn(env);
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
        CHECK_AND_RETURN_RET_LOG(argc == ARGS_TWO, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
            "INPUT ERROR PARAMETER"), "parameter error");
        UnregisterAppVolumeChangeForUidCallback(env, args[PARAM1], args, argc, napiVolumeManager);
    } else {
        AUDIO_ERR_LOG("No such callback supported");
        NapiAudioError::ThrowError(env, NAPI_ERR_INVALID_PARAM,
            "parameter verification failed: The param of type is not supported");
    }
    return undefinedResult;
}

void NapiAudioVolumeManager::UnregisterAppVolumeChangeForUidCallback(napi_env env, napi_value callback,
    napi_value *args, size_t argc, NapiAudioVolumeManager *napiAudioVolumeManager)
{
    if (napiAudioVolumeManager == nullptr) {
        AUDIO_ERR_LOG("napiAudioVolumeManager is nullptr");
        return;
    }
    CHECK_AND_RETURN_LOG(napiAudioVolumeManager->appVolumeChangeCallbackForUidNapi_ != nullptr,
        "UnregisterDeviceChangeCallback: audio manager deviceChangeCallbackNapi_ is null");
    std::shared_ptr<NapiAudioManagerAppVolumeChangeCallback> cb =
        std::static_pointer_cast<NapiAudioManagerAppVolumeChangeCallback>(
        napiAudioVolumeManager->appVolumeChangeCallbackForUidNapi_);
    if (callback != nullptr) {
        cb->RemoveAudioVolumeChangeForUidCbRef(env, callback);
    }

    if (cb->GetAppVolumeChangeForUidListSize() == 0) {
        napiAudioVolumeManager->audioSystemMngr_->UnsetAppVolumeCallbackForUid();
        napiAudioVolumeManager->appVolumeChangeCallbackForUidNapi_.reset();
        napiAudioVolumeManager->appVolumeChangeCallbackForUidNapi_ = nullptr;
        cb->RemoveAllAudioVolumeChangeForUidCbRef();
    }
}

void NapiAudioVolumeManager::UnregisterSelfAppVolumeChangeCallback(napi_env env, napi_value callback,
    size_t argc, NapiAudioVolumeManager *napiAudioVolumeManager)
{
    if (napiAudioVolumeManager == nullptr) {
        AUDIO_ERR_LOG("napiAudioVolumeManager is nullptr");
        return;
    }
    CHECK_AND_RETURN_LOG(napiAudioVolumeManager->selfAppVolumeChangeCallbackNapi_ != nullptr,
        "UnregisterDeviceChangeCallback: audio manager selfAppVolumeChangeCallbackNapi_ is null");
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
}  // namespace AudioStandard
}  // namespace OHOS