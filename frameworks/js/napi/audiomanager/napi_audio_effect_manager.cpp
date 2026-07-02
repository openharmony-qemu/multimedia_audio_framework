/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "NapiAudioEffectMgr"
#endif

#include "napi_audio_effect_manager.h"
#include "napi_audio_error.h"
#include "napi_param_utils.h"
#include "napi_audio_enum.h"
#include "audio_errors.h"
#include "audio_manager_log.h"
#include "napi_audio_effect_manager_callback.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
using namespace HiviewDFX;
static __thread napi_ref g_effectMgrConstructor = nullptr;

NapiAudioEffectMgr::NapiAudioEffectMgr()
    : env_(nullptr), audioEffectMngr_(nullptr) {}

NapiAudioEffectMgr::~NapiAudioEffectMgr() = default;

void NapiAudioEffectMgr::Destructor(napi_env env, void *nativeObject, void *finalizeHint)
{
    if (nativeObject != nullptr) {
        auto obj = static_cast<NapiAudioEffectMgr *>(nativeObject);
        ObjectRefMap<NapiAudioEffectMgr>::DecreaseRef(obj);
    }
    AUDIO_INFO_LOG("Destructor is successful");
}

napi_value NapiAudioEffectMgr::Construct(napi_env env, napi_callback_info info)
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
    unique_ptr<NapiAudioEffectMgr> napiEffectMgr = make_unique<NapiAudioEffectMgr>();
    CHECK_AND_RETURN_RET_LOG(napiEffectMgr != nullptr, result, "No memory");

    napiEffectMgr->env_ = env;
    napiEffectMgr->audioEffectMngr_ = AudioEffectManager::GetInstance();
    napiEffectMgr->cachedClientId_ = getpid();
    ObjectRefMap<NapiAudioEffectMgr>::Insert(napiEffectMgr.get());

    status = napi_wrap(env, thisVar, static_cast<void*>(napiEffectMgr.get()),
        NapiAudioEffectMgr::Destructor, nullptr, nullptr);
    if (status != napi_ok) {
        ObjectRefMap<NapiAudioEffectMgr>::Erase(napiEffectMgr.get());
        return result;
    }
    napiEffectMgr.release();
    return thisVar;
}

napi_value NapiAudioEffectMgr::Init(napi_env env, napi_value exports)
{
    napi_status status;
    napi_value constructor;
    napi_value result = nullptr;
    const int32_t refCount = ARGS_ONE;
    napi_get_undefined(env, &result);

    napi_property_descriptor audio_effect_mgr_properties[] = {
        
        DECLARE_NAPI_FUNCTION("getSupportedAudioEffectProperty", GetSupportedAudioEffectProperty),
        DECLARE_NAPI_FUNCTION("getAudioEffectProperty", GetAudioEffectProperty),
        DECLARE_NAPI_FUNCTION("setAudioEffectProperty", SetAudioEffectProperty),
        DECLARE_NAPI_FUNCTION("isAudioSeparationEffectSupported", IsAudioSeparationEffectSupported),
        DECLARE_NAPI_FUNCTION("setAudioSeparationEffectEnabled", SetAudioSeparationEffectEnabled),
        DECLARE_NAPI_FUNCTION("setAudioSeparationEffectVolume", SetAudioSeparationEffectVolume),
        DECLARE_NAPI_FUNCTION("onAudioSeparationEffectEnabledChange", OnAudioSeparationEffectEnabledChange),
        DECLARE_NAPI_FUNCTION("offAudioSeparationEffectEnabledChange", OffAudioSeparationEffectEnabledChange),
       
    };

    status = napi_define_class(env, AUDIO_EFFECT_MGR_NAPI_CLASS_NAME.c_str(), NAPI_AUTO_LENGTH, Construct, nullptr,
        sizeof(audio_effect_mgr_properties) / sizeof(audio_effect_mgr_properties[PARAM0]),
        audio_effect_mgr_properties, &constructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_define_class fail");

    status = napi_create_reference(env, constructor, refCount, &g_effectMgrConstructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_create_reference fail");
    status = napi_set_named_property(env, exports, AUDIO_EFFECT_MGR_NAPI_CLASS_NAME.c_str(), constructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_set_named_property fail");
    return exports;
}

napi_value NapiAudioEffectMgr::CreateEffectManagerWrapper(napi_env env)
{
    napi_status status;
    napi_value result = nullptr;
    napi_value constructor;

    status = napi_get_reference_value(env, g_effectMgrConstructor, &constructor);
    if (status != napi_ok) {
        AUDIO_ERR_LOG("Failed in CreateEffectManagerWrapper, %{public}d", status);
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

NapiAudioEffectMgr* NapiAudioEffectMgr::GetParamWithSync(const napi_env &env, napi_callback_info info,
    size_t &argc, napi_value *args)
{
    napi_status status;
    NapiAudioEffectMgr *napiEffectMgr = nullptr;
    napi_value jsThis = nullptr;
    status = napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok && jsThis != nullptr, nullptr,
        "GetParamWithSync fail to napi_get_cb_info");

    status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napiEffectMgr));
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, nullptr, "napi_unwrap failed");
    CHECK_AND_RETURN_RET_LOG(napiEffectMgr != nullptr && napiEffectMgr->audioEffectMngr_  !=
        nullptr, napiEffectMgr, "GetParamWithSync fail to napi_unwrap");
    return napiEffectMgr;
}

napi_value NapiAudioEffectMgr::GetSupportedAudioEffectProperty(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = PARAM0;
    auto *napiEffectMgr = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(argc == PARAM0 && napiEffectMgr != nullptr && napiEffectMgr->audioEffectMngr_ != nullptr,
        NapiAudioError::ThrowErrorAndReturnDoNotUse(env, NAPI_ERR_SYSTEM,
        "incorrect parameter types: The type of options must be empty"), "argcCount invalid");

    AudioEffectPropertyArray propertyArray = {};
    int32_t ret = napiEffectMgr->audioEffectMngr_->GetSupportedAudioEffectProperty(propertyArray);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK,  NapiAudioError::ThrowErrorAndReturnDoNotUse(env, ret,
        "interface operation failed"), "get support audio effect property failure!");

    napi_status status = NapiParamUtils::SetEffectProperty(env, propertyArray, result);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, NapiAudioError::ThrowErrorAndReturnDoNotUse(env,
        NAPI_ERR_SYSTEM, "Combining property data fail"), "fill support effect property failed");

    return result;
}

napi_value NapiAudioEffectMgr::GetAudioEffectProperty(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = PARAM0;
    auto *napiEffectMgr = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(argc == PARAM0 && napiEffectMgr != nullptr && napiEffectMgr->audioEffectMngr_ != nullptr,
        NapiAudioError::ThrowErrorAndReturnDoNotUse(env, NAPI_ERR_SYSTEM,
        "incorrect parameter types: The type of options must be empty"), "argcCount invalid");

    AudioEffectPropertyArray propertyArray = {};
    int32_t ret = napiEffectMgr->audioEffectMngr_->GetAudioEffectProperty(propertyArray);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK,  NapiAudioError::ThrowErrorAndReturnDoNotUse(env, ret,
        "interface operation failed"), "get audio enhance property failure!");

    napi_status status = NapiParamUtils::SetEffectProperty(env, propertyArray, result);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, NapiAudioError::ThrowErrorAndReturnDoNotUse(env,
        NAPI_ERR_SYSTEM, "combining property data fail"), "fill effect property failed");

    return result;
}

napi_value NapiAudioEffectMgr::SetAudioEffectProperty(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = ARGS_ONE;
    napi_value args[ARGS_ONE] = {};
    auto *napiEffectMgr = GetParamWithSync(env, info, argc, args);
    CHECK_AND_RETURN_RET_LOG(argc == ARGS_ONE && napiEffectMgr != nullptr &&
        napiEffectMgr->audioEffectMngr_ != nullptr, NapiAudioError::ThrowErrorAndReturnDoNotUse(env,
        NAPI_ERR_INPUT_INVALID,
        "parameter verification failed: mandatory parameters are left unspecified"), "argcCount invalid");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, args[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_object, NapiAudioError::ThrowErrorAndReturnDoNotUse(env,
        NAPI_ERR_INPUT_INVALID,
        "incorrect parameter types: The type of options must be array"), "invaild valueType");

    AudioEffectPropertyArray propertyArray = {};
    napi_status status = NapiParamUtils::GetEffectPropertyArray(env, propertyArray, args[PARAM0]);
    CHECK_AND_RETURN_RET_LOG(propertyArray.property.size() > 0,
        NapiAudioError::ThrowErrorAndReturnDoNotUse(env, NAPI_ERR_INVALID_PARAM,
        "parameter verification failed: mandatory parameters are left unspecified"), "status or arguments error");

    CHECK_AND_RETURN_RET_LOG(status == napi_ok,
        NapiAudioError::ThrowErrorAndReturnDoNotUse(env, NAPI_ERR_INVALID_PARAM,
        "parameter verification failed: mandatory parameters are left unspecified"), "status or arguments error");

    int32_t ret = napiEffectMgr->audioEffectMngr_->SetAudioEffectProperty(propertyArray);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK,  NapiAudioError::ThrowErrorAndReturnDoNotUse(env, ret,
        "interface operation failed"), "set audio effect property failure!");

    return result;
}

napi_value NapiAudioEffectMgr::IsAudioSeparationEffectSupported(napi_env env, napi_callback_info info)
{
    AUDIO_INFO_LOG("IsAudioSeparationEffectSupported");
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(),
        NapiAudioError::ThrowErrorAndReturnDoNotUse(env, NAPI_ERR_PERMISSION_DENIED), "No system permission");

    napi_value result = nullptr;
    size_t argc = PARAM0;
    auto *napiEffectMgr = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(argc == PARAM0 && napiEffectMgr != nullptr,
        NapiAudioError::ThrowErrorAndReturnDoNotUse(env, NAPI_ERR_INPUT_INVALID, "invalid arguments"), "argc invalid");
    CHECK_AND_RETURN_RET_LOG(napiEffectMgr->audioEffectMngr_ != nullptr, result, "audioEffectMngr is nullptr");

    bool isSupported = napiEffectMgr->audioEffectMngr_->IsAudioSeparationEffectSupported();
    NapiParamUtils::SetValueBoolean(env, isSupported, result);

    return result;
}

napi_value NapiAudioEffectMgr::SetAudioSeparationEffectEnabled(napi_env env, napi_callback_info info)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(),
        NapiAudioError::ThrowErrorAndReturnDoNotUse(env, NAPI_ERR_PERMISSION_DENIED), "No system permission");

    auto context = std::make_shared<AudioEffectManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("SetAudioSeparationEffectEnabled failed : no memory");
        NapiAudioError::ThrowError(env, "SetAudioSeparationEffectEnabled failed : no memory",
            NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_TWO,
            "parameter verification failed: mandatory parameters are left unspecified",
            NAPI_ERR_INVALID_PARAM);
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[PARAM0], &valueType);
        NAPI_CHECK_ARGS_RETURN_VOID(context, valueType == napi_boolean,
            "incorrect parameter types: The type of enabled must be boolean",
            NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueBoolean(env, context->enabled, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok,
            "parameter verification failed", NAPI_ERR_INVALID_PARAM);
        napi_typeof(env, argv[PARAM1], &valueType);
        NAPI_CHECK_ARGS_RETURN_VOID(context, valueType == napi_number,
            "incorrect parameter types: The type of uid must be number",
            NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->uid, argv[PARAM1]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok,
            "parameter verification failed", NAPI_ERR_INVALID_PARAM);
        if (argc == ARGS_THREE) {
            context->status = NapiParamUtils::GetValueInt64(env, context->streamId, argv[PARAM2]);
            NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok,
                "parameter verification failed", NAPI_ERR_INVALID_PARAM);
        }
    };
    context->GetCbInfo(env, info, inputParser);
    if (context->status != napi_ok) {
        NapiAudioError::ThrowError(env, context->errMessage.c_str(), context->errCode);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    return UpdateAudioSeparationEffectEnabled(env, context);
}

napi_value NapiAudioEffectMgr::UpdateAudioSeparationEffectEnabled(napi_env env,
    std::shared_ptr<AudioEffectManagerAsyncContext> &context)
{
    auto executor = [context]() {
        auto obj = reinterpret_cast<NapiAudioEffectMgr*>(context->native);
        CHECK_AND_RETURN_LOG(obj != nullptr, "napiEffectMgr is nullptr");
        auto *napiEffectMgr = obj;
        CHECK_AND_RETURN_LOG(napiEffectMgr->audioEffectMngr_ != nullptr, "audioEffectMngr_ is nullptr");
        context->intValue = napiEffectMgr->audioEffectMngr_->SetAudioSeparationEffectEnabled(
            context->enabled, context->uid, context->streamId);
        if (context->intValue != SUCCESS) {
            if (context->intValue == ERR_NOT_SUPPORTED) {
                context->SignError(NAPI_ERR_UNSUPPORTED);
            } else {
                context->SignError(NAPI_ERR_SYSTEM);
            }
        }
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SetAudioSeparationEffectEnabled", executor, complete);
}

napi_value NapiAudioEffectMgr::SetAudioSeparationEffectVolume(napi_env env, napi_callback_info info)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(),
        NapiAudioError::ThrowErrorAndReturnDoNotUse(env, NAPI_ERR_PERMISSION_DENIED), "No system permission");

    auto context = std::make_shared<AudioEffectManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("SetAudioSeparationEffectVolume failed : no memory");
        NapiAudioError::ThrowError(env, "SetAudioSeparationEffectVolume failed : no memory",
            NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc == ARGS_TWO,
            "parameter verification failed: mandatory parameters are left unspecified",
            NAPI_ERR_INVALID_PARAM);
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[PARAM0], &valueType);
        NAPI_CHECK_ARGS_RETURN_VOID(context, valueType == napi_number,
            "incorrect parameter types: The type of type must be number",
            NAPI_ERR_INVALID_PARAM);
        int32_t typeValue = 0;
        context->status = NapiParamUtils::GetValueInt32(env, typeValue, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok && typeValue == VOLUME_TYPE_VOCAL,
            "parameter verification failed: type value must be VOLUME_TYPE_VOCAL",
            NAPI_ERR_INVALID_PARAM);
        context->volumeType = static_cast<AudioSeparationVolumeType>(typeValue);
        napi_typeof(env, argv[PARAM1], &valueType);
        NAPI_CHECK_ARGS_RETURN_VOID(context, valueType == napi_number,
            "incorrect parameter types: The type of volume must be number",
            NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueDouble(env, context->volume, argv[PARAM1]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok && context->volume >= 0.0 &&
            context->volume <= 1.0, "parameter verification failed: volume range is [0.0, 1.0]",
            NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);
    if (context->status != napi_ok) {
        NapiAudioError::ThrowError(env, context->errMessage.c_str(), context->errCode);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    return UpdateAudioSeparationEffectVolume(env, context);
}

napi_value NapiAudioEffectMgr::UpdateAudioSeparationEffectVolume(napi_env env,
    std::shared_ptr<AudioEffectManagerAsyncContext> &context)
{
    auto executor = [context]() {
        auto obj = reinterpret_cast<NapiAudioEffectMgr*>(context->native);
        CHECK_AND_RETURN_LOG(obj != nullptr, "napiEffectMgr is nullptr");
        auto *napiEffectMgr = obj;
        CHECK_AND_RETURN_LOG(napiEffectMgr->audioEffectMngr_ != nullptr, "audioEffectMngr_ is nullptr");
        context->intValue = napiEffectMgr->audioEffectMngr_->SetAudioSeparationEffectVolume(
            context->volumeType, context->volume);
        if (context->intValue != SUCCESS) {
            if (context->intValue == ERR_NOT_SUPPORTED) {
                context->SignError(NAPI_ERR_UNSUPPORTED);
            } else {
                context->SignError(NAPI_ERR_SYSTEM);
            }
        }
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SetAudioSeparationEffectVolume", executor, complete);
}

napi_value NapiAudioEffectMgr::OnAudioSeparationEffectEnabledChange(napi_env env, napi_callback_info info)
{
    const size_t requireArgc = ARGS_ONE;
    size_t argc = requireArgc + 1;
    napi_value undefinedResult = nullptr;
    napi_get_undefined(env, &undefinedResult);

    napi_value args[requireArgc + 1] = {nullptr};
    napi_value jsThis = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr);
    if (status != napi_ok || argc < requireArgc) {
        AUDIO_ERR_LOG("On fail to napi_get_cb_info/Requires min 1 parameters");
        NapiAudioError::ThrowErrorDoNotUse(env, NAPI_ERR_INPUT_INVALID, "mandatory parameters are left unspecified");
        return undefinedResult;
    }

    napi_valuetype handler = napi_undefined;
    if (napi_typeof(env, args[PARAM0], &handler) != napi_ok || handler != napi_function) {
        AUDIO_ERR_LOG("On type mismatch for parameter 1");
        NapiAudioError::ThrowErrorDoNotUse(env, NAPI_ERR_INPUT_INVALID,
            "incorrect parameter types: The type of callback must be function");
        return undefinedResult;
    }

    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(),
        NapiAudioError::ThrowErrorAndReturnDoNotUse(env, NAPI_ERR_PERMISSION_DENIED), "No system permission");

    NapiAudioEffectMgr *napiEffectMgr = nullptr;
    status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napiEffectMgr));
    if ((status != napi_ok) || (napiEffectMgr == nullptr) ||
        (napiEffectMgr->audioEffectMngr_ == nullptr)) {
        AUDIO_ERR_LOG("Failed to retrieve audio effect manager napi instance.");
        return undefinedResult;
    }

    if (!napiEffectMgr->audioSeparationEffectEnabledChangeCallbackNapi_) {
        napiEffectMgr->audioSeparationEffectEnabledChangeCallbackNapi_ =
            std::make_shared<NapiAudioSeparationEffectEnabledChangeCallback>(env);
        CHECK_AND_RETURN_RET_LOG(napiEffectMgr->audioSeparationEffectEnabledChangeCallbackNapi_ != nullptr,
            undefinedResult, "NapiAudioEffectMgr: Memory Allocation Failed !!");

        int32_t ret = napiEffectMgr->audioEffectMngr_->OnAudioSeparationEffectEnabledChange(
            napiEffectMgr->audioSeparationEffectEnabledChangeCallbackNapi_);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, undefinedResult,
            "NapiAudioEffectMgr: Registering of Audio Separation Effect Enabled Change Callback Failed");
    }

    std::shared_ptr<NapiAudioSeparationEffectEnabledChangeCallback> cb =
        std::static_pointer_cast<NapiAudioSeparationEffectEnabledChangeCallback>
        (napiEffectMgr->audioSeparationEffectEnabledChangeCallbackNapi_);
    cb->SaveAudioSeparationEffectEnabledChangeCallbackReference(args[PARAM0]);
    if (!cb->GetSeparationEffectEnableTsfnFlag()) {
        cb->CreateSeparationEffectEnableTsfn(env);
    }

    AUDIO_INFO_LOG("Register audio separation effect enabled callback is successful");
    return undefinedResult;
}

napi_value NapiAudioEffectMgr::OffAudioSeparationEffectEnabledChange(napi_env env, napi_callback_info info)
{
    const size_t minArgc = PARAM0;
    size_t argc = ARGS_ONE;
    napi_value undefinedResult = nullptr;
    napi_get_undefined(env, &undefinedResult);

    napi_value args[ARGS_ONE] = {nullptr};
    napi_value jsThis = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok,
        NapiAudioError::ThrowErrorAndReturnDoNotUse(env, NAPI_ERR_INPUT_INVALID, "invalid arguments"),
        "Off fail to napi_get_cb_info");

    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(),
        NapiAudioError::ThrowErrorAndReturnDoNotUse(env, NAPI_ERR_PERMISSION_DENIED), "No system permission");

    NapiAudioEffectMgr *napiEffectMgr = nullptr;
    status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napiEffectMgr));
    CHECK_AND_RETURN_RET_LOG((status == napi_ok) && (napiEffectMgr != nullptr) &&
        (napiEffectMgr->audioEffectMngr_ != nullptr),
        undefinedResult, "Failed to retrieve audio effect manager napi instance.");

    CHECK_AND_RETURN_RET_LOG(napiEffectMgr->audioSeparationEffectEnabledChangeCallbackNapi_ != nullptr,
        undefinedResult, "audioSeparationEffectEnabledChangeCallbackNapi_ is nullptr");

    std::shared_ptr<NapiAudioSeparationEffectEnabledChangeCallback> cb =
        std::static_pointer_cast<NapiAudioSeparationEffectEnabledChangeCallback>
        (napiEffectMgr->audioSeparationEffectEnabledChangeCallbackNapi_);

    if (argc == minArgc) {
        cb->RemoveAllAudioSeparationEffectEnabledChangeCallbackReference();
        int32_t ret = napiEffectMgr->audioEffectMngr_->OffAudioSeparationEffectEnabledChange();
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, undefinedResult,
            "NapiAudioEffectMgr: Unregistering of Audio Separation Effect Enabled Change Callback Failed");
        napiEffectMgr->audioSeparationEffectEnabledChangeCallbackNapi_ = nullptr;
        AUDIO_INFO_LOG("Unregister all audio separation effect enabled callbacks successful");
        return undefinedResult;
    }

    napi_valuetype valueType = napi_undefined;
    status = napi_typeof(env, args[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok && valueType == napi_function,
        NapiAudioError::ThrowErrorAndReturnDoNotUse(env, NAPI_ERR_INPUT_INVALID,
        "incorrect parameter types: The type of callback must be function"),
        "Type mismatch for callback");

    cb->RemoveAudioSeparationEffectEnabledChangeCallbackReference(env, args[PARAM0]);
    if (cb->GetAudioSeparationEffectEnabledChangeCbListSize() == 0) {
        int32_t ret = napiEffectMgr->audioEffectMngr_->OffAudioSeparationEffectEnabledChange();
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, undefinedResult,
            "NapiAudioEffectMgr: Unregistering of Audio Separation Effect Enabled Change Callback Failed");
        napiEffectMgr->audioSeparationEffectEnabledChangeCallbackNapi_ = nullptr;
    }
    AUDIO_INFO_LOG("Unregister specific audio separation effect enabled callback successful");

    return undefinedResult;
}

}  // namespace AudioStandard
}  // namespace OHOS
