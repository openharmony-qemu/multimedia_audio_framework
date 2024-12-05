/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

    status = napi_unwrap(env, jsThis, (void **)&napiEffectMgr);
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
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_SYSTEM,
        "incorrect parameter types: The type of options must be empty"), "argcCount invalid");

    AudioEffectPropertyArrayV3 propertyArray = {};
    int32_t ret = napiEffectMgr->audioEffectMngr_->GetSupportedAudioEffectProperty(propertyArray);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK,  NapiAudioError::ThrowErrorAndReturn(env, ret,
        "interface operation failed"), "get support audio effect property failure!");

    napi_status status = NapiParamUtils::SetEffectProperty(env, propertyArray, result);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_SYSTEM,
        "Combining property data fail"), "fill support effect property failed");

    return result;
}

napi_value NapiAudioEffectMgr::GetAudioEffectProperty(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = PARAM0;
    auto *napiEffectMgr = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(argc == PARAM0 && napiEffectMgr != nullptr && napiEffectMgr->audioEffectMngr_ != nullptr,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_SYSTEM,
        "incorrect parameter types: The type of options must be empty"), "argcCount invalid");

    AudioEffectPropertyArrayV3 propertyArray = {};
    int32_t ret = napiEffectMgr->audioEffectMngr_->GetAudioEffectProperty(propertyArray);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK,  NapiAudioError::ThrowErrorAndReturn(env, ret,
        "interface operation failed"), "get audio enhance property failure!");

    napi_status status = NapiParamUtils::SetEffectProperty(env, propertyArray, result);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_SYSTEM,
        "combining property data fail"), "fill effect property failed");

    return result;
}

napi_value NapiAudioEffectMgr::SetAudioEffectProperty(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = ARGS_ONE;
    napi_value args[ARGS_ONE] = {};
    auto *napiEffectMgr = GetParamWithSync(env, info, argc, args);
    CHECK_AND_RETURN_RET_LOG(argc == ARGS_ONE && napiEffectMgr != nullptr &&
        napiEffectMgr->audioEffectMngr_ != nullptr, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
        "parameter verification failed: mandatory parameters are left unspecified"), "argcCount invalid");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, args[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_object, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
        "incorrect parameter types: The type of options must be array"), "invaild valueType");

    AudioEffectPropertyArrayV3 propertyArray = {};
    napi_status status = NapiParamUtils::GetEffectPropertyArray(env, propertyArray, args[PARAM0]);
    CHECK_AND_RETURN_RET_LOG(propertyArray.property.size() > 0,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
        "parameter verification failed: mandatory parameters are left unspecified"), "status or arguments error");

    CHECK_AND_RETURN_RET_LOG(status == napi_ok,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
        "parameter verification failed: mandatory parameters are left unspecified"), "status or arguments error");

    int32_t ret = napiEffectMgr->audioEffectMngr_->SetAudioEffectProperty(propertyArray);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK,  NapiAudioError::ThrowErrorAndReturn(env, ret,
        "interface operation failed"), "set audio effect property failure!");

    return result;
}

}  // namespace AudioStandard
}  // namespace OHOS
