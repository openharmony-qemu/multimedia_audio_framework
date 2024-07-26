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
#include "napi_audio_volume_group_manager.h"

#include "napi_audio_error.h"
#include "napi_param_utils.h"
#include "napi_audio_enum.h"
#include "napi_audio_ringermode_callback.h"
#include "napi_audio_micstatechange_callback.h"
#include "audio_errors.h"
#include "audio_log.h"
#include "audio_utils.h"
#ifdef FEATURE_HIVIEW_ENABLE
#include "xpower_event_js.h"
#endif

namespace OHOS {
namespace AudioStandard {
using namespace std;
using namespace HiviewDFX;
static __thread napi_ref g_groupmanagerConstructor = nullptr;
int32_t NapiAudioVolumeGroupManager::isConstructSuccess_ = SUCCESS;
std::mutex NapiAudioVolumeGroupManager::volumeGroupManagerMutex_;

static napi_value ThrowErrorAndReturn(napi_env env, int32_t errCode)
{
    NapiAudioError::ThrowError(env, errCode);
    return nullptr;
}

bool NapiAudioVolumeGroupManager::CheckContextStatus(std::shared_ptr<AudioVolumeGroupManagerAsyncContext> context)
{
    CHECK_AND_RETURN_RET_LOG(context != nullptr, false, "context object is nullptr.");
    if (context->native == nullptr) {
        context->SignError(NAPI_ERR_SYSTEM);
        AUDIO_ERR_LOG("context object state is error.");
        return false;
    }
    return true;
}

bool NapiAudioVolumeGroupManager::CheckAudioVolumeGroupManagerStatus(NapiAudioVolumeGroupManager *napi,
    std::shared_ptr<AudioVolumeGroupManagerAsyncContext> context)
{
    CHECK_AND_RETURN_RET_LOG(napi != nullptr, false, "napi object is nullptr.");
    if (napi->audioGroupMngr_ == nullptr) {
        context->SignError(NAPI_ERR_SYSTEM);
        AUDIO_ERR_LOG("context object state is error.");
        return false;
    }
    return true;
}

NapiAudioVolumeGroupManager* NapiAudioVolumeGroupManager::GetParamWithSync(const napi_env &env, napi_callback_info info,
    size_t &argc, napi_value *args)
{
    napi_status status;
    NapiAudioVolumeGroupManager *napiAudioVolumeGroupManager = nullptr;
    napi_value jsThis = nullptr;

    status = napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok && jsThis != nullptr, nullptr,
        "GetParamWithSync fail to napi_get_cb_info");

    status = napi_unwrap(env, jsThis, (void **)&napiAudioVolumeGroupManager);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, nullptr, "napi_unwrap failed");
    CHECK_AND_RETURN_RET_LOG(napiAudioVolumeGroupManager != nullptr && napiAudioVolumeGroupManager->audioGroupMngr_ !=
        nullptr, napiAudioVolumeGroupManager, "GetParamWithSync fail to napi_unwrap");
    return napiAudioVolumeGroupManager;
}

napi_status NapiAudioVolumeGroupManager::InitNapiAudioVolumeGroupManager(napi_env env, napi_value &constructor)
{
    napi_property_descriptor audio_svc_group_mngr_properties[] = {
        DECLARE_NAPI_FUNCTION("getVolume", GetVolume),
        DECLARE_NAPI_FUNCTION("getVolumeSync", GetVolumeSync),
        DECLARE_NAPI_FUNCTION("setVolume", SetVolume),
        DECLARE_NAPI_FUNCTION("getMaxVolume", GetMaxVolume),
        DECLARE_NAPI_FUNCTION("getMaxVolumeSync", GetMaxVolumeSync),
        DECLARE_NAPI_FUNCTION("getMinVolume", GetMinVolume),
        DECLARE_NAPI_FUNCTION("getMinVolumeSync", GetMinVolumeSync),
        DECLARE_NAPI_FUNCTION("mute", SetMute),
        DECLARE_NAPI_FUNCTION("isMute", IsStreamMute),
        DECLARE_NAPI_FUNCTION("isMuteSync", IsStreamMuteSync),
        DECLARE_NAPI_FUNCTION("setRingerMode", SetRingerMode),
        DECLARE_NAPI_FUNCTION("getRingerMode", GetRingerMode),
        DECLARE_NAPI_FUNCTION("getRingerModeSync", GetRingerModeSync),
        DECLARE_NAPI_FUNCTION("setMicrophoneMute", SetMicrophoneMute),
        DECLARE_NAPI_FUNCTION("isMicrophoneMute", IsMicrophoneMute),
        DECLARE_NAPI_FUNCTION("isMicrophoneMuteSync", IsMicrophoneMuteSync),
        DECLARE_NAPI_FUNCTION("setMicMute", SetMicMute),
        DECLARE_NAPI_FUNCTION("isVolumeUnadjustable", IsVolumeUnadjustable),
        DECLARE_NAPI_FUNCTION("adjustVolumeByStep", AdjustVolumeByStep),
        DECLARE_NAPI_FUNCTION("adjustSystemVolumeByStep", AdjustSystemVolumeByStep),
        DECLARE_NAPI_FUNCTION("getSystemVolumeInDb", GetSystemVolumeInDb),
        DECLARE_NAPI_FUNCTION("getSystemVolumeInDbSync", GetSystemVolumeInDbSync),
        DECLARE_NAPI_FUNCTION("on", On),
    };

    napi_status status = napi_define_class(env, AUDIO_VOLUME_GROUP_MNGR_NAPI_CLASS_NAME.c_str(),
        NAPI_AUTO_LENGTH, Construct, nullptr,
        sizeof(audio_svc_group_mngr_properties) / sizeof(audio_svc_group_mngr_properties[PARAM0]),
        audio_svc_group_mngr_properties, &constructor);
    return status;
}

napi_value NapiAudioVolumeGroupManager::Init(napi_env env, napi_value exports)
{
    AUDIO_DEBUG_LOG("Init");
    napi_status status;
    napi_value constructor;
    napi_value result = nullptr;
    const int32_t refCount = 1;
    NapiParamUtils::GetUndefinedValue(env);

    status = InitNapiAudioVolumeGroupManager(env, constructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_define_class fail");
    status = napi_create_reference(env, constructor, refCount, &g_groupmanagerConstructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_create_reference fail");
    status = napi_set_named_property(env, exports, AUDIO_VOLUME_GROUP_MNGR_NAPI_CLASS_NAME.c_str(), constructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_set_named_property fail");
    return exports;
}

napi_value NapiAudioVolumeGroupManager::CreateAudioVolumeGroupManagerWrapper(napi_env env, int32_t groupId)
{
    napi_status status;
    napi_value result = nullptr;
    napi_value constructor;
    napi_value groupId_;
    NapiParamUtils::SetValueInt64(env, groupId, groupId_);
    napi_value args[PARAM1] = {groupId_};
    status = napi_get_reference_value(env, g_groupmanagerConstructor, &constructor);
    if (status != napi_ok) {
        AUDIO_ERR_LOG("Failed in CreateAudioVolumeGroupManagerWrapper, %{public}d", status);
        goto fail;
    }
    status = napi_new_instance(env, constructor, PARAM1, args, &result);
    if (status != napi_ok) {
        AUDIO_ERR_LOG("napi_new_instance failed, sttaus:%{public}d", status);
        goto fail;
    }
    return result;

fail:
    napi_get_undefined(env, &result);
    return result;
}

void NapiAudioVolumeGroupManager::Destructor(napi_env env, void *nativeObject, void *finalizeHint)
{
    std::lock_guard<mutex> lock(volumeGroupManagerMutex_);

    if (nativeObject != nullptr) {
        auto obj = static_cast<NapiAudioVolumeGroupManager*>(nativeObject);
        ObjectRefMap<NapiAudioVolumeGroupManager>::DecreaseRef(obj);
    }
    AUDIO_INFO_LOG("Destructor is successful");
}

napi_value NapiAudioVolumeGroupManager::Construct(napi_env env, napi_callback_info info)
{
    std::lock_guard<mutex> lock(volumeGroupManagerMutex_);

    napi_status status;
    napi_value jsThis;
    napi_value undefinedResult = nullptr;
    NapiParamUtils::GetUndefinedValue(env);
    size_t argCount = PARAM1;
    int32_t groupId = PARAM0;

    napi_value args[PARAM1] = { nullptr};
    status = napi_get_cb_info(env, info, &argCount, args, &jsThis, nullptr);
    NapiParamUtils::GetValueInt32(env, groupId, args[PARAM0]);
    AUDIO_INFO_LOG("Construct() %{public}d", groupId);

    CHECK_AND_RETURN_RET_LOG(status == napi_ok, undefinedResult, "Failed in NapiAudioVolumeGroupManager::Construct()!");
    unique_ptr<NapiAudioVolumeGroupManager> napiAudioVolumeGroupManager = make_unique<NapiAudioVolumeGroupManager>();
    CHECK_AND_RETURN_RET_LOG(napiAudioVolumeGroupManager != nullptr, undefinedResult, "groupmanagerNapi is nullptr");

    napiAudioVolumeGroupManager->audioGroupMngr_ = AudioSystemManager::GetInstance()->GetGroupManager(groupId);
    if (napiAudioVolumeGroupManager->audioGroupMngr_ == nullptr) {
        AUDIO_ERR_LOG("Failed in NapiAudioVolumeGroupManager::Construct()!");
        NapiAudioVolumeGroupManager::isConstructSuccess_ = NAPI_ERR_SYSTEM;
    }
    napiAudioVolumeGroupManager->cachedClientId_ = getpid();
    ObjectRefMap<NapiAudioVolumeGroupManager>::Insert(napiAudioVolumeGroupManager.get());
    status = napi_wrap(env, jsThis, static_cast<void*>(napiAudioVolumeGroupManager.get()),
        NapiAudioVolumeGroupManager::Destructor, nullptr, nullptr);
    if (status != napi_ok) {
        ObjectRefMap<NapiAudioVolumeGroupManager>::Erase(napiAudioVolumeGroupManager.get());
        return undefinedResult;
    }
    napiAudioVolumeGroupManager.release();
    return jsThis;
}

napi_value NapiAudioVolumeGroupManager::GetVolume(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioVolumeGroupManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("GetVolume failed : no memory");
        NapiAudioError::ThrowError(env, "GetVolume failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->volType, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get volType failed", NAPI_ERR_INVALID_PARAM);
        if (!NapiAudioEnum::IsLegalInputArgumentVolType(context->volType)) {
            context->SignError(NAPI_ERR_UNSUPPORTED);
            return;
        }
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioVolumeGroupManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioVolumeGroupManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioVolumeGroupManagerStatus(napiAudioVolumeGroupManager, context),
            "audio volume group manager state is error.");
        context->volLevel = napiAudioVolumeGroupManager->audioGroupMngr_->GetVolume(
            NapiAudioEnum::GetNativeAudioVolumeType(context->volType));
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueInt32(env, context->volLevel, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "GetVolume", executor, complete);
}

napi_value NapiAudioVolumeGroupManager::GetVolumeSync(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = ARGS_ONE;
    napi_value args[ARGS_ONE] = {};
    auto *napiAudioVolumeGroupManager = GetParamWithSync(env, info, argc, args);
    CHECK_AND_RETURN_RET_LOG(argc == ARGS_ONE, ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID), "invalid arguments");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, args[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_number, ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID),
        "invalid valueType");

    int32_t volType;
    NapiParamUtils::GetValueInt32(env, volType, args[PARAM0]);
    CHECK_AND_RETURN_RET_LOG(NapiAudioEnum::IsLegalInputArgumentVolType(volType), ThrowErrorAndReturn(env,
        NAPI_ERR_INVALID_PARAM), "get volType failed");

    CHECK_AND_RETURN_RET_LOG(napiAudioVolumeGroupManager != nullptr, result, "napiAudioVolumeGroupManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioVolumeGroupManager->audioGroupMngr_ != nullptr, result,
        "audioGroupMngr_ is nullptr");
    int32_t volLevel = napiAudioVolumeGroupManager->audioGroupMngr_->GetVolume(
        NapiAudioEnum::GetNativeAudioVolumeType(volType));
    NapiParamUtils::SetValueInt32(env, volLevel, result);

    return result;
}

napi_value NapiAudioVolumeGroupManager::SetVolume(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioVolumeGroupManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("SetVolume failed : no memory");
        NapiAudioError::ThrowError(env, "SetVolume failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_TWO, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->volType, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get volType failed", NAPI_ERR_INVALID_PARAM);
        if (!NapiAudioEnum::IsLegalInputArgumentVolType(context->volType)) {
            context->SignError(context->errCode ==
                NAPI_ERR_INVALID_PARAM? NAPI_ERR_INVALID_PARAM : NAPI_ERR_UNSUPPORTED);
        }
        context->status = NapiParamUtils::GetValueInt32(env, context->volLevel, argv[PARAM1]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get volLevel failed", NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);
#ifdef FEATURE_HIVIEW_ENABLE
    HiviewDFX::ReportXPowerJsStackSysEvent(env, "VOLUME_CHANGE", "SRC=Audio");
#endif
    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioVolumeGroupManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioVolumeGroupManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioVolumeGroupManagerStatus(napiAudioVolumeGroupManager, context),
            "audio volume group manager state is error.");
        context->intValue = napiAudioVolumeGroupManager->audioGroupMngr_->SetVolume(
            NapiAudioEnum::GetNativeAudioVolumeType(context->volType), context->volLevel);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->intValue == SUCCESS, "setvolume failed", NAPI_ERR_SYSTEM);
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SetVolume", executor, complete);
}

napi_value NapiAudioVolumeGroupManager::GetMaxVolume(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioVolumeGroupManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("GetMaxVolume failed : no memory");
        NapiAudioError::ThrowError(env, "GetMaxVolume failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->volType, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get volType failed", NAPI_ERR_INVALID_PARAM);
        if (!NapiAudioEnum::IsLegalInputArgumentVolType(context->volType)) {
            context->SignError(NAPI_ERR_UNSUPPORTED);
            return;
        }
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioVolumeGroupManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioVolumeGroupManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioVolumeGroupManagerStatus(napiAudioVolumeGroupManager, context),
            "audio volume group manager state is error.");
        context->volLevel = napiAudioVolumeGroupManager->audioGroupMngr_->GetMaxVolume(
            NapiAudioEnum::GetNativeAudioVolumeType(context->volType));
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueInt32(env, context->volLevel, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "GetMaxVolume", executor, complete);
}

napi_value NapiAudioVolumeGroupManager::GetMaxVolumeSync(napi_env env, napi_callback_info info)
{
    AUDIO_INFO_LOG("GetMaxVolumeSync");
    napi_value result = nullptr;
    size_t argc = ARGS_ONE;
    napi_value args[ARGS_ONE] = {};
    auto *napiAudioVolumeGroupManager = GetParamWithSync(env, info, argc, args);
    CHECK_AND_RETURN_RET_LOG(argc >= ARGS_ONE, ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID), "invalid arguments");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, args[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_number, ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID),
        "invalid valueType");

    int32_t volType;
    NapiParamUtils::GetValueInt32(env, volType, args[PARAM0]);
    CHECK_AND_RETURN_RET_LOG(NapiAudioEnum::IsLegalInputArgumentVolType(volType), ThrowErrorAndReturn(env,
        NAPI_ERR_INVALID_PARAM), "get volType failed");

    CHECK_AND_RETURN_RET_LOG(napiAudioVolumeGroupManager != nullptr, result, "napiAudioVolumeGroupManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioVolumeGroupManager->audioGroupMngr_ != nullptr, result,
        "audioGroupMngr_ is nullptr");
    int32_t volLevel = napiAudioVolumeGroupManager->audioGroupMngr_->GetMaxVolume(
        NapiAudioEnum::GetNativeAudioVolumeType(volType));
    NapiParamUtils::SetValueInt32(env, volLevel, result);

    return result;
}

napi_value NapiAudioVolumeGroupManager::GetMinVolume(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioVolumeGroupManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("GetMinVolume failed : no memory");
        NapiAudioError::ThrowError(env, "GetMinVolume failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->volType, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get volType failed", NAPI_ERR_INVALID_PARAM);
        if (!NapiAudioEnum::IsLegalInputArgumentVolType(context->volType)) {
            context->SignError(NAPI_ERR_UNSUPPORTED);
        }
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioVolumeGroupManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioVolumeGroupManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioVolumeGroupManagerStatus(napiAudioVolumeGroupManager, context),
            "audio volume group manager state is error.");
        context->volLevel = napiAudioVolumeGroupManager->audioGroupMngr_->GetMinVolume(
            NapiAudioEnum::GetNativeAudioVolumeType(context->volType));
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueInt32(env, context->volLevel, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "GetMinVolume", executor, complete);
}

napi_value NapiAudioVolumeGroupManager::GetMinVolumeSync(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = ARGS_ONE;
    napi_value args[ARGS_ONE] = {};
    auto *napiAudioVolumeGroupManager = GetParamWithSync(env, info, argc, args);
    CHECK_AND_RETURN_RET_LOG(argc >= ARGS_ONE, ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID), "invalid arguments");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, args[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_number, ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID),
        "invalid valueType");

    int32_t volType;
    NapiParamUtils::GetValueInt32(env, volType, args[PARAM0]);
    CHECK_AND_RETURN_RET_LOG(NapiAudioEnum::IsLegalInputArgumentVolType(volType), ThrowErrorAndReturn(env,
        NAPI_ERR_INVALID_PARAM), "get volType failed");

    CHECK_AND_RETURN_RET_LOG(napiAudioVolumeGroupManager != nullptr, result, "napiAudioVolumeGroupManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioVolumeGroupManager->audioGroupMngr_ != nullptr, result,
        "audioGroupMngr_ is nullptr");
    int32_t volLevel = napiAudioVolumeGroupManager->audioGroupMngr_->GetMinVolume(
        NapiAudioEnum::GetNativeAudioVolumeType(volType));
    NapiParamUtils::SetValueInt32(env, volLevel, result);

    return result;
}

napi_value NapiAudioVolumeGroupManager::SetMute(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioVolumeGroupManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("SetMute failed : no memory");
        NapiAudioError::ThrowError(env, "SetMute failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_TWO, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->volType, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get volType failed", NAPI_ERR_INVALID_PARAM);
        if (!NapiAudioEnum::IsLegalInputArgumentVolType(context->volType)) {
            context->SignError(context->errCode ==
                NAPI_ERR_INVALID_PARAM? NAPI_ERR_INVALID_PARAM : NAPI_ERR_UNSUPPORTED);
        }
        context->status = NapiParamUtils::GetValueBoolean(env, context->isMute, argv[PARAM1]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get isMute failed", NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioVolumeGroupManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioVolumeGroupManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioVolumeGroupManagerStatus(napiAudioVolumeGroupManager, context),
            "audio volume group manager state is error.");
        context->intValue = napiAudioVolumeGroupManager->audioGroupMngr_->SetMute(
            NapiAudioEnum::GetNativeAudioVolumeType(context->volType), context->isMute);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->intValue == SUCCESS, "setmute failed", NAPI_ERR_SYSTEM);
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SetMute", executor, complete);
}

napi_value NapiAudioVolumeGroupManager::IsStreamMute(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioVolumeGroupManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("IsStreamMute failed : no memory");
        NapiAudioError::ThrowError(env, "IsStreamMute failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->volType, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get volType failed", NAPI_ERR_INVALID_PARAM);
        if (!NapiAudioEnum::IsLegalInputArgumentVolType(context->volType)) {
            context->SignError(NAPI_ERR_UNSUPPORTED);
        }
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioVolumeGroupManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioVolumeGroupManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioVolumeGroupManagerStatus(napiAudioVolumeGroupManager, context),
            "audio volume group manager state is error.");
        context->intValue = napiAudioVolumeGroupManager->audioGroupMngr_->IsStreamMute(
            NapiAudioEnum::GetNativeAudioVolumeType(context->volType), context->isMute);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->intValue == SUCCESS, "isstreammute failed",
            NAPI_ERR_SYSTEM);
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueBoolean(env, context->isMute, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "IsStreamMute", executor, complete);
}

napi_value NapiAudioVolumeGroupManager::IsStreamMuteSync(napi_env env, napi_callback_info info)
{
    AUDIO_INFO_LOG("IsStreamMuteSync");
    napi_value result = nullptr;
    size_t argc = ARGS_ONE;
    napi_value args[ARGS_ONE] = {};
    auto *napiAudioVolumeGroupManager = GetParamWithSync(env, info, argc, args);
    CHECK_AND_RETURN_RET_LOG(argc >= ARGS_ONE, ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID), "invalid arguments");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, args[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_number, ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID),
        "invalid valueType");

    int32_t volType;
    NapiParamUtils::GetValueInt32(env, volType, args[PARAM0]);
    CHECK_AND_RETURN_RET_LOG(NapiAudioEnum::IsLegalInputArgumentVolType(volType), ThrowErrorAndReturn(env,
        NAPI_ERR_INVALID_PARAM), "get volType failed");

    CHECK_AND_RETURN_RET_LOG(napiAudioVolumeGroupManager != nullptr, result, "napiAudioVolumeGroupManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioVolumeGroupManager->audioGroupMngr_ != nullptr, result,
        "audioGroupMngr_ is nullptr");
    bool isMute;
    int32_t ret = napiAudioVolumeGroupManager->audioGroupMngr_->IsStreamMute(
        NapiAudioEnum::GetNativeAudioVolumeType(volType), isMute);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, result, "IsStreamMute failure!");
    NapiParamUtils::SetValueBoolean(env, isMute, result);

    return result;
}

napi_value NapiAudioVolumeGroupManager::SetRingerMode(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioVolumeGroupManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("SetRingerMode failed : no memory");
        NapiAudioError::ThrowError(env, "SetRingerMode failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->ringMode, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get ringMode failed", NAPI_ERR_INVALID_PARAM);
        if (!NapiAudioEnum::IsLegalInputArgumentRingMode(context->ringMode)) {
            context->SignError(NAPI_ERR_UNSUPPORTED);
        }
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioVolumeGroupManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioVolumeGroupManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioVolumeGroupManagerStatus(napiAudioVolumeGroupManager, context),
            "audio volume group manager state is error.");
        context->intValue = napiAudioVolumeGroupManager->audioGroupMngr_->SetRingerMode(
            NapiAudioEnum::GetNativeAudioRingerMode(context->ringMode));
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->intValue == SUCCESS, "setringermode failed",
            NAPI_ERR_SYSTEM);
    };

    auto complete = [env](napi_value &output) {
        NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SetRingerMode", executor, complete);
}

napi_value NapiAudioVolumeGroupManager::GetRingerMode(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioVolumeGroupManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("GetRingerMode failed : no memory");
        NapiAudioError::ThrowError(env, "GetRingerMode failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioVolumeGroupManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioVolumeGroupManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioVolumeGroupManagerStatus(napiAudioVolumeGroupManager, context),
            "audio volume group manager state is error.");
        context->ringMode = NapiAudioEnum::GetJsAudioRingMode(
            napiAudioVolumeGroupManager->audioGroupMngr_->GetRingerMode());
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueInt32(env, context->ringMode, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "GetRingerMode", executor, complete);
}

napi_value NapiAudioVolumeGroupManager::GetRingerModeSync(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = PARAM0;
    auto *napiAudioVolumeGroupManager = GetParamWithSync(env, info, argc, nullptr);

    CHECK_AND_RETURN_RET_LOG(napiAudioVolumeGroupManager != nullptr, result, "napiAudioVolumeGroupManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioVolumeGroupManager->audioGroupMngr_ != nullptr, result,
        "audioGroupMngr_ is nullptr");
    AudioRingerMode ringerMode = napiAudioVolumeGroupManager->audioGroupMngr_->GetRingerMode();
    NapiParamUtils::SetValueInt32(env, ringerMode, result);

    return result;
}

napi_value NapiAudioVolumeGroupManager::SetMicrophoneMute(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioVolumeGroupManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("SetMicrophoneMute failed : no memory");
        NapiAudioError::ThrowError(env, "SetMicrophoneMute failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueBoolean(env, context->isMute, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get ringMode failed", NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioVolumeGroupManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioVolumeGroupManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioVolumeGroupManagerStatus(napiAudioVolumeGroupManager, context),
            "audio volume group manager state is error.");
        context->intValue = napiAudioVolumeGroupManager->audioGroupMngr_->SetMicrophoneMute(
            context->isMute);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->intValue == SUCCESS, "setmicrophonemute failed",
            NAPI_ERR_SYSTEM);
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SetMicrophoneMute", executor, complete);
}

napi_value NapiAudioVolumeGroupManager::IsMicrophoneMute(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioVolumeGroupManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("IsMicrophoneMute failed : no memory");
        NapiAudioError::ThrowError(env, "IsMicrophoneMute failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioVolumeGroupManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioVolumeGroupManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioVolumeGroupManagerStatus(napiAudioVolumeGroupManager, context),
            "audio volume group manager state is error.");
        context->isMute = napiAudioVolumeGroupManager->audioGroupMngr_->IsMicrophoneMute();
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueBoolean(env, context->isMute, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "IsMicrophoneMute", executor, complete);
}

napi_value NapiAudioVolumeGroupManager::IsMicrophoneMuteSync(napi_env env, napi_callback_info info)
{
    AUDIO_INFO_LOG("IsMicrophoneMuteSync in");
    napi_value result = nullptr;
    size_t argc = PARAM0;
    auto *napiAudioVolumeGroupManager = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(argc < ARGS_ONE, ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID), "invalid arguments");
    CHECK_AND_RETURN_RET_LOG(napiAudioVolumeGroupManager != nullptr, result, "napiAudioVolumeGroupManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioVolumeGroupManager->audioGroupMngr_ != nullptr, result,
        "audioGroupMngr_ is nullptr");
    bool isMute = napiAudioVolumeGroupManager->audioGroupMngr_->IsMicrophoneMute();
    NapiParamUtils::SetValueBoolean(env, isMute, result);

    return result;
}

napi_value NapiAudioVolumeGroupManager::SetMicMute(napi_env env, napi_callback_info info)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(),
        ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED), "No system permission");

    auto context = std::make_shared<AudioVolumeGroupManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("no memory failed");
        NapiAudioError::ThrowError(env, "failed no memory", NAPI_ERR_SYSTEM);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments", NAPI_ERR_INPUT_INVALID);
        context->status = NapiParamUtils::GetValueBoolean(env, context->isMute, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "", NAPI_ERR_INPUT_INVALID);
    };
    context->GetCbInfo(env, info, inputParser);
    if (context->status != napi_ok) {
        NapiAudioError::ThrowError(env, context->errCode);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioVolumeGroupManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioVolumeGroupManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioVolumeGroupManagerStatus(napiAudioVolumeGroupManager, context),
            "audio volume group manager state is error.");
        context->intValue = napiAudioVolumeGroupManager->audioGroupMngr_->SetMicrophoneMute(context->isMute);
        if (context->intValue != SUCCESS) {
            if (context->intValue == ERR_PERMISSION_DENIED) {
                context->SignError(NAPI_ERR_NO_PERMISSION);
            } else {
                context->SignError(NAPI_ERR_SYSTEM);
            }
        }
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SetMicMute", executor, complete);
}

napi_value NapiAudioVolumeGroupManager::IsVolumeUnadjustable(napi_env env, napi_callback_info info)
{
    AUDIO_INFO_LOG("IsVolumeUnadjustable");
    napi_value result = nullptr;
    size_t argc = PARAM0;
    auto *napiAudioVolumeGroupManager = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(argc < ARGS_ONE, ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID), "invalid arguments");
    CHECK_AND_RETURN_RET_LOG(napiAudioVolumeGroupManager != nullptr, result, "napiAudioVolumeGroupManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioVolumeGroupManager->audioGroupMngr_ != nullptr, result,
        "audioGroupMngr_ is nullptr");
    bool isVolumeUnadjustable = napiAudioVolumeGroupManager->audioGroupMngr_->IsVolumeUnadjustable();
    NapiParamUtils::SetValueBoolean(env, isVolumeUnadjustable, result);

    AUDIO_INFO_LOG("IsVolumeUnadjustable is successful");
    return result;
}

napi_value NapiAudioVolumeGroupManager::AdjustVolumeByStep(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioVolumeGroupManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("AdjustVolumeByStep failed : no memory");
        NapiAudioError::ThrowError(env, "AdjustVolumeByStep failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments", NAPI_ERR_INPUT_INVALID);
        context->status = NapiParamUtils::GetValueInt32(env, context->adjustType, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get adjustType failed",
            NAPI_ERR_INPUT_INVALID);
        NAPI_CHECK_ARGS_RETURN_VOID(context, NapiAudioEnum::IsLegalInputArgumentVolumeAdjustType(context->adjustType),
            "adjustType invaild", NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);

    if ((context->status != napi_ok) && (context->errCode == NAPI_ERR_INPUT_INVALID)) {
        NapiAudioError::ThrowError(env, context->errCode);
        return NapiParamUtils::GetUndefinedValue(env);
    }
    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioVolumeGroupManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioVolumeGroupManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioVolumeGroupManagerStatus(napiAudioVolumeGroupManager, context),
            "audio volume group manager state is error.");
        context->volumeAdjustStatus = napiAudioVolumeGroupManager->audioGroupMngr_->AdjustVolumeByStep(
            static_cast<VolumeAdjustType>(context->adjustType));
        if (context->volumeAdjustStatus != SUCCESS) {
            if (context->volumeAdjustStatus == ERR_PERMISSION_DENIED) {
                context->SignError(NAPI_ERR_NO_PERMISSION);
            } else {
                context->SignError(NAPI_ERR_SYSTEM);
            }
        }
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueInt32(env, context->volumeAdjustStatus, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "AdjustVolumeByStep", executor, complete);
}

napi_value NapiAudioVolumeGroupManager::AdjustSystemVolumeByStep(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioVolumeGroupManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("AdjustSystemVolumeByStep failed : no memory");
        NapiAudioError::ThrowError(env, "AdjustSystemVolumeByStep failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_TWO, "invalid arguments", NAPI_ERR_INPUT_INVALID);
        context->status = NapiParamUtils::GetValueInt32(env, context->volType, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get volType failed", NAPI_ERR_INPUT_INVALID);
        NAPI_CHECK_ARGS_RETURN_VOID(context, NapiAudioEnum::IsLegalInputArgumentVolType(context->volType) &&
            context->volType != NapiAudioEnum::ALL, "volType invaild", NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->adjustType, argv[PARAM1]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get adjustType failed",
            NAPI_ERR_INPUT_INVALID);
        NAPI_CHECK_ARGS_RETURN_VOID(context, NapiAudioEnum::IsLegalInputArgumentVolumeAdjustType(context->adjustType),
            "adjustType invaild", NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);

    if ((context->status != napi_ok) && (context->errCode == NAPI_ERR_INPUT_INVALID)) {
        NapiAudioError::ThrowError(env, context->errCode);
        return NapiParamUtils::GetUndefinedValue(env);
    }
    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioVolumeGroupManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioVolumeGroupManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioVolumeGroupManagerStatus(napiAudioVolumeGroupManager, context),
            "audio volume group manager state is error.");
        context->volumeAdjustStatus = napiAudioVolumeGroupManager->audioGroupMngr_->AdjustSystemVolumeByStep(
            NapiAudioEnum::GetNativeAudioVolumeType(context->volType),
            static_cast<VolumeAdjustType>(context->adjustType));
        if (context->volumeAdjustStatus != SUCCESS) {
            if (context->volumeAdjustStatus == ERR_PERMISSION_DENIED) {
                context->SignError(NAPI_ERR_NO_PERMISSION);
            } else {
                context->SignError(NAPI_ERR_SYSTEM);
            }
        }
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueInt32(env, context->volumeAdjustStatus, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "AdjustSystemVolumeByStep", executor, complete);
}

napi_value NapiAudioVolumeGroupManager::GetSystemVolumeInDb(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioVolumeGroupManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("GetSystemVolumeInDb failed : no memory");
        NapiAudioError::ThrowError(env, "GetSystemVolumeInDb failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_THREE, "invalid arguments", NAPI_ERR_INPUT_INVALID);
        context->status = NapiParamUtils::GetValueInt32(env, context->volType, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "volType getfailed", NAPI_ERR_INPUT_INVALID);
        NAPI_CHECK_ARGS_RETURN_VOID(context, NapiAudioEnum::IsLegalInputArgumentVolType(context->volType),
            "volType invaild", NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->volLevel, argv[PARAM1]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "volLevel getfailed", NAPI_ERR_INPUT_INVALID);
        context->status = NapiParamUtils::GetValueInt32(env, context->deviceType, argv[PARAM2]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, NapiAudioEnum::IsLegalInputArgumentDeviceType(context->deviceType) &&
            (context->status == napi_ok), "deviceType invaild", NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);

    if ((context->status != napi_ok) && (context->errCode == NAPI_ERR_INPUT_INVALID)) {
        NapiAudioError::ThrowError(env, context->errCode);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioVolumeGroupManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioVolumeGroupManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioVolumeGroupManagerStatus(napiAudioVolumeGroupManager, context),
            "audio volume group manager state is error.");
        context->volumeInDb = napiAudioVolumeGroupManager->audioGroupMngr_->GetSystemVolumeInDb(
            NapiAudioEnum::GetNativeAudioVolumeType(context->volType), context->volLevel,
            static_cast<DeviceType>(context->deviceType));
        if (FLOAT_COMPARE_EQ(context->volumeInDb, static_cast<float>(ERR_INVALID_PARAM))) {
            context->SignError(NAPI_ERR_INVALID_PARAM);
        } else if (context->volumeInDb < 0) {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueDouble(env, context->volumeInDb, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "AdjustSystemVolumeByStep", executor, complete);
}

napi_value NapiAudioVolumeGroupManager::GetSystemVolumeInDbSync(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = ARGS_THREE;
    napi_value args[ARGS_THREE] = {};
    auto *napiAudioVolumeGroupManager = GetParamWithSync(env, info, argc, args);
    CHECK_AND_RETURN_RET_LOG(argc >= ARGS_THREE, ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID), "invalid arguments");

    int32_t volType;
    int32_t volLevel;
    int32_t deviceType;
    for (size_t i = PARAM0; i < argc; i++) {
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, args[i], &valueType);
        CHECK_AND_RETURN_RET_LOG(valueType == napi_number, ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID),
            "invalid valueType");
    }
    NapiParamUtils::GetValueInt32(env, volType, args[PARAM0]);
    CHECK_AND_RETURN_RET_LOG(NapiAudioEnum::IsLegalInputArgumentVolType(volType), ThrowErrorAndReturn(env,
        NAPI_ERR_INVALID_PARAM), "get volType failed");
    NapiParamUtils::GetValueInt32(env, volLevel, args[PARAM1]);
    NapiParamUtils::GetValueInt32(env, deviceType, args[PARAM2]);
    CHECK_AND_RETURN_RET_LOG(NapiAudioEnum::IsLegalInputArgumentDeviceType(deviceType), ThrowErrorAndReturn(env,
        NAPI_ERR_INVALID_PARAM), "get deviceType failed");

    CHECK_AND_RETURN_RET_LOG(napiAudioVolumeGroupManager != nullptr, result, "napiAudioVolumeGroupManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioVolumeGroupManager->audioGroupMngr_ != nullptr, result,
        "audioGroupMngr_ is nullptr");
    double volumeInDb = napiAudioVolumeGroupManager->audioGroupMngr_->GetSystemVolumeInDb(
        NapiAudioEnum::GetNativeAudioVolumeType(volType), volLevel, static_cast<DeviceType>(deviceType));
    CHECK_AND_RETURN_RET_LOG(!FLOAT_COMPARE_EQ(static_cast<float>(volumeInDb), static_cast<float>(ERR_INVALID_PARAM)),
        ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM), "getsystemvolumeindb failed");
    NapiParamUtils::SetValueDouble(env, volumeInDb, result);

    return result;
}

napi_value NapiAudioVolumeGroupManager::RegisterCallback(napi_env env, napi_value jsThis, size_t argc, napi_value *args,
    const std::string &cbName)
{
    napi_value undefinedResult = nullptr;
    napi_get_undefined(env, &undefinedResult);

    NapiAudioVolumeGroupManager *napiAudioVolumeGroupManager = nullptr;
    napi_status status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napiAudioVolumeGroupManager));
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, ThrowErrorAndReturn(env, NAPI_ERR_SYSTEM), "status error");
    CHECK_AND_RETURN_RET_LOG(napiAudioVolumeGroupManager != nullptr, ThrowErrorAndReturn(env, NAPI_ERR_NO_MEMORY),
        "napiAudioVolumeGroupManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioVolumeGroupManager->audioGroupMngr_ != nullptr, ThrowErrorAndReturn(
        env, NAPI_ERR_NO_MEMORY), "audioGroupMngr_ is nullptr");
    if (!cbName.compare(RINGERMODE_CALLBACK_NAME)) {
        if (napiAudioVolumeGroupManager->ringerModecallbackNapi_ == nullptr) {
            napiAudioVolumeGroupManager->ringerModecallbackNapi_ = std::make_shared<NapiAudioRingerModeCallback>(env);
            int32_t ret = napiAudioVolumeGroupManager->audioGroupMngr_->SetRingerModeCallback(
                napiAudioVolumeGroupManager->cachedClientId_, napiAudioVolumeGroupManager->ringerModecallbackNapi_);
            CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, undefinedResult, "SetRingerModeCallback Failed");
        }
        std::shared_ptr<NapiAudioRingerModeCallback> cb =
            std::static_pointer_cast<NapiAudioRingerModeCallback>(napiAudioVolumeGroupManager->ringerModecallbackNapi_);
        cb->SaveCallbackReference(cbName, args[PARAM1]);
    } else if (!cbName.compare(MIC_STATE_CHANGE_CALLBACK_NAME)) {
        if (!napiAudioVolumeGroupManager->micStateChangeCallbackNapi_) {
            napiAudioVolumeGroupManager->micStateChangeCallbackNapi_ =
                std::make_shared<NapiAudioManagerMicStateChangeCallback>(env);
            if (!napiAudioVolumeGroupManager->micStateChangeCallbackNapi_) {
                AUDIO_ERR_LOG("Memory Allocation Failed !!");
            }

            int32_t ret = napiAudioVolumeGroupManager->audioGroupMngr_->SetMicStateChangeCallback(
                napiAudioVolumeGroupManager->micStateChangeCallbackNapi_);
            if (ret) {
                AUDIO_ERR_LOG("Registering Microphone Change Callback Failed");
            }
        }
        std::shared_ptr<NapiAudioManagerMicStateChangeCallback> cb =
            std::static_pointer_cast<NapiAudioManagerMicStateChangeCallback>(napiAudioVolumeGroupManager->
                micStateChangeCallbackNapi_);
        cb->SaveCallbackReference(cbName, args[PARAM1]);
        AUDIO_DEBUG_LOG("On SetMicStateChangeCallback is successful");
    } else {
        AUDIO_ERR_LOG("No such callback supported");
        NapiAudioError::ThrowError(env, NAPI_ERR_INVALID_PARAM);
    }
    return undefinedResult;
}

napi_value NapiAudioVolumeGroupManager::On(napi_env env, napi_callback_info info)
{
    AUDIO_DEBUG_LOG("On inter");
    napi_value undefinedResult = nullptr;
    NapiParamUtils::GetUndefinedValue(env);

    const size_t minArgc = ARGS_TWO;
    size_t argc = ARGS_THREE;
    napi_value args[minArgc + PARAM1] = {nullptr, nullptr, nullptr};
    napi_value jsThis = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr);
    if (status != napi_ok || argc < minArgc) {
        AUDIO_ERR_LOG("On fail to napi_get_cb_info/Requires min 2 parameters");
        NapiAudioError::ThrowError(env, NAPI_ERR_INPUT_INVALID);
    }

    napi_valuetype eventType = napi_undefined;
    if (napi_typeof(env, args[PARAM0], &eventType) != napi_ok || eventType != napi_string) {
        NapiAudioError::ThrowError(env, NAPI_ERR_INPUT_INVALID);
        return undefinedResult;
    }
    std::string callbackName = NapiParamUtils::GetStringArgument(env, args[PARAM0]);
    AUDIO_INFO_LOG("On callbackName: %{public}s", callbackName.c_str());

    napi_valuetype handler = napi_undefined;
    if (napi_typeof(env, args[PARAM1], &handler) != napi_ok || handler != napi_function) {
        AUDIO_ERR_LOG("On type mismatch for parameter 2");
        NapiAudioError::ThrowError(env, NAPI_ERR_INPUT_INVALID);
        return undefinedResult;
    }

    return RegisterCallback(env, jsThis, argc, args, callbackName);
}
}  // namespace AudioStandard
}  // namespace OHOS