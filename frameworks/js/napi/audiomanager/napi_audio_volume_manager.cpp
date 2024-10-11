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
#define LOG_TAG "NapiAudioVolumeManager"
#endif

#include "napi_audio_volume_manager.h"
#include "napi_audio_volume_group_manager.h"
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
    if (nativeObject != nullptr) {
        auto obj = static_cast<NapiAudioVolumeManager *>(nativeObject);
        ObjectRefMap<NapiAudioVolumeManager>::DecreaseRef(obj);
    }
    AUDIO_INFO_LOG("Destructor is successful");
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

napi_value NapiAudioVolumeManager::GetVolumeGroupInfos(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioVolumeManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("GetVolumeGroupInfos failed : no memory");
        NapiAudioError::ThrowError(env, "GetVolumeGroupInfos failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[PARAM0], &valueType);
        NAPI_CHECK_ARGS_RETURN_VOID(context, valueType == napi_string, "invaild valueType", NAPI_ERR_INVALID_PARAM);
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
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->intValue == SUCCESS, "getvolumegroups failed", NAPI_ERR_SYSTEM);
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
    CHECK_AND_RETURN_RET_LOG(argc >= ARGS_ONE, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
        "mandatory parameters are left unspecified"), "invalid arguments");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, args[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_string, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
        "incorrect parameter types: The type of networkId must be string"), "invalid valueType");

    std::string networkId = NapiParamUtils::GetStringArgument(env, args[PARAM0]);
    CHECK_AND_RETURN_RET_LOG(!networkId.empty(), NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
        "parameter verification failed: The param of networkId is empty"), "get networkid failed");

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
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->groupId, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get groupId failed", NAPI_ERR_INVALID_PARAM);
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
    CHECK_AND_RETURN_RET_LOG(argc == ARGS_ONE, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
        "mandatory parameters are left unspecified"), "invalid arguments");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, args[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_number, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
        "incorrect parameter types: The type of groupId must be number"), "invalid valueType");

    int32_t groupId;
    NapiParamUtils::GetValueInt32(env, groupId, args[PARAM0]);

    result = NapiAudioVolumeGroupManager::CreateAudioVolumeGroupManagerWrapper(env, groupId);

    napi_value undefinedValue = nullptr;
    napi_get_undefined(env, &undefinedValue);
    bool isEqual = false;
    napi_strict_equals(env, result, undefinedValue, &isEqual);
    if (isEqual) {
        AUDIO_ERR_LOG("The audio volume group manager is undefined!");
        NapiAudioError::ThrowError(env, "GetVolumeGroupManagerSync failed: invalid param", NAPI_ERR_INVALID_PARAM);
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
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_SYSTEM),
        "status error");
    CHECK_AND_RETURN_RET_LOG(napiVolumeManager != nullptr, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_NO_MEMORY),
        "napiVolumeManager is nullptr");
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
    } else {
        AUDIO_ERR_LOG("No such callback supported");
        NapiAudioError::ThrowError(env, NAPI_ERR_INVALID_PARAM,
            "parameter verification failed: The param of type is not supported");
    }
    return undefinedResult;
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
        NapiAudioError::ThrowError(env, NAPI_ERR_INPUT_INVALID, "mandatory parameters are left unspecified");
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
    if (napi_typeof(env, args[PARAM1], &handler) != napi_ok || handler != napi_function) {
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
        NapiAudioError::ThrowError(env, NAPI_ERR_INPUT_INVALID, "mandatory parameters are left unspecified");
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
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_SYSTEM),
        "status error");
    CHECK_AND_RETURN_RET_LOG(napiVolumeManager != nullptr, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_NO_MEMORY),
        "napiVolumeManager is nullptr");
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
    } else {
        AUDIO_ERR_LOG("No such callback supported");
        NapiAudioError::ThrowError(env, NAPI_ERR_INVALID_PARAM,
            "parameter verification failed: The param of type is not supported");
    }
    return undefinedResult;
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