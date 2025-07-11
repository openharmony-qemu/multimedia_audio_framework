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
#define LOG_TAG "NapiAudioVolumeGroupManager"
#endif

#include "napi_audio_volume_group_manager.h"

#include "napi_audio_error.h"
#include "napi_param_utils.h"
#include "napi_audio_enum.h"
#include "napi_audio_ringermode_callback.h"
#include "napi_audio_micstatechange_callback.h"
#include "audio_errors.h"
#include "audio_manager_log.h"
#ifdef FEATURE_HIVIEW_ENABLE
#if !defined(ANDROID_PLATFORM) && !defined(IOS_PLATFORM)
#include "xpower_event_js.h"
#endif
#endif

namespace OHOS {
namespace AudioStandard {
using namespace std;
using namespace HiviewDFX;
const std::string AUDIO_VOLUME_GROUP_MNGR_NAPI_CLASS_NAME = "AudioVolumeGroupManager";
static __thread napi_ref g_groupmanagerConstructor = nullptr;
int32_t NapiAudioVolumeGroupManager::isConstructSuccess_ = SUCCESS;
std::mutex NapiAudioVolumeGroupManager::volumeGroupManagerMutex_;

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
        DECLARE_NAPI_FUNCTION("getActiveVolumeTypeSync", GetActiveVolumeTypeSync),
        DECLARE_NAPI_FUNCTION("getVolume", GetVolume),
        DECLARE_NAPI_FUNCTION("getVolumeSync", GetVolumeSync),
        DECLARE_NAPI_FUNCTION("getSystemVolumeByUid", GetSystemVolumeByUid),
        DECLARE_NAPI_FUNCTION("setVolume", SetVolume),
        DECLARE_NAPI_FUNCTION("setVolumeWithFlag", SetVolumeWithFlag),
        DECLARE_NAPI_FUNCTION("setSystemVolumeByUid", SetSystemVolumeByUid),
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
        DECLARE_NAPI_FUNCTION("setMicMutePersistent", SetMicMutePersistent),
        DECLARE_NAPI_FUNCTION("isPersistentMicMute", GetPersistentMicMuteState),
        DECLARE_NAPI_FUNCTION("isVolumeUnadjustable", IsVolumeUnadjustable),
        DECLARE_NAPI_FUNCTION("adjustVolumeByStep", AdjustVolumeByStep),
        DECLARE_NAPI_FUNCTION("adjustSystemVolumeByStep", AdjustSystemVolumeByStep),
        DECLARE_NAPI_FUNCTION("getSystemVolumeInDb", GetSystemVolumeInDb),
        DECLARE_NAPI_FUNCTION("getSystemVolumeInDbSync", GetSystemVolumeInDbSync),
        DECLARE_NAPI_FUNCTION("on", On),
        DECLARE_NAPI_FUNCTION("off", Off),
        DECLARE_NAPI_FUNCTION("getMaxAmplitudeForOutputDevice", GetMaxAmplitudeForOutputDevice),
        DECLARE_NAPI_FUNCTION("getMaxAmplitudeForInputDevice", GetMaxAmplitudeForInputDevice),
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
        AUDIO_ERR_LOG("napi_new_instance failed, status:%{public}d", status);
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

    if (nativeObject == nullptr) {
        AUDIO_WARNING_LOG("Native object is null");
        return;
    }
    auto obj = static_cast<NapiAudioVolumeGroupManager*>(nativeObject);
    ObjectRefMap<NapiAudioVolumeGroupManager>::DecreaseRef(obj);
    AUDIO_PRERELEASE_LOGI("Decrease obj count");
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
    AUDIO_PRERELEASE_LOGI("Construct() %{public}d", groupId);

    CHECK_AND_RETURN_RET_LOG(status == napi_ok, undefinedResult, "Failed in NapiAudioVolumeGroupManager::Construct()!");
    auto groupManager = AudioSystemManager::GetInstance()->GetGroupManager(groupId);
    if (groupManager == nullptr) {
        AUDIO_ERR_LOG("Failed to get group manager!");
        NapiAudioVolumeGroupManager::isConstructSuccess_ = NAPI_ERR_INVALID_PARAM;
        return undefinedResult;
    }

    unique_ptr<NapiAudioVolumeGroupManager> napiAudioVolumeGroupManager = make_unique<NapiAudioVolumeGroupManager>();
    CHECK_AND_RETURN_RET_LOG(napiAudioVolumeGroupManager != nullptr, undefinedResult, "groupmanagerNapi is nullptr");

    napiAudioVolumeGroupManager->audioGroupMngr_ = groupManager;
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

napi_value NapiAudioVolumeGroupManager::GetActiveVolumeTypeSync(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = ARGS_ONE;
    napi_value args[ARGS_ONE] = {};
    auto *napiAudioVolumeGroupManager = GetParamWithSync(env, info, argc, args);
    CHECK_AND_RETURN_RET_LOG(argc == ARGS_ONE, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID, "mandatory parameters are left unspecified"), "invalid arguments");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, args[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_number, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID, "incorrect parameter types: The type of uid must be number"),
        "invalid uid");

    int32_t clientUid;
    NapiParamUtils::GetValueInt32(env, clientUid, args[PARAM0]);

    CHECK_AND_RETURN_RET_LOG(napiAudioVolumeGroupManager != nullptr, result, "napiAduioVolumeGroupManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioVolumeGroupManager->audioGroupMngr_ != nullptr, result,
        "audioGroupMngr_ is nullptr");
    AudioStreamType volType = napiAudioVolumeGroupManager->audioGroupMngr_->GetActiveVolumeType(clientUid);
    int32_t jsVolType = NapiAudioEnum::GetJsAudioVolumeType(volType);
    NapiParamUtils::SetValueInt32(env, jsVolType, result);

    return result;
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
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments",
            NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->volType, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get volType failed",
            NAPI_ERR_INVALID_PARAM);
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
    CHECK_AND_RETURN_RET_LOG(argc == ARGS_ONE, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID, "mandatory parameters are left unspecified"), "invalid arguments");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, args[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_number, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID, "incorrect parameter types: The type of volumeType must be number"),
        "invalid valueType");

    int32_t volType;
    NapiParamUtils::GetValueInt32(env, volType, args[PARAM0]);
    CHECK_AND_RETURN_RET_LOG(NapiAudioEnum::IsLegalInputArgumentVolType(volType),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
        "parameter verification failed: The param of volumeType must be enum AudioVolumeType"), "get volType failed");

    CHECK_AND_RETURN_RET_LOG(napiAudioVolumeGroupManager != nullptr, result, "napiAudioVolumeGroupManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioVolumeGroupManager->audioGroupMngr_ != nullptr, result,
        "audioGroupMngr_ is nullptr");
    int32_t volLevel = napiAudioVolumeGroupManager->audioGroupMngr_->GetVolume(
        NapiAudioEnum::GetNativeAudioVolumeType(volType));
    NapiParamUtils::SetValueInt32(env, volLevel, result);

    return result;
}

napi_value NapiAudioVolumeGroupManager::GetSystemVolumeByUid(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = ARGS_TWO;
    napi_value args[ARGS_TWO] = {};
    auto *napiAudioVolumeGroupManager = GetParamWithSync(env, info, argc, args);
    CHECK_AND_RETURN_RET_LOG(argc == ARGS_TWO, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID, "mandatory parameters are left unspecified"), "invalid arguments");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, args[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_number, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID, "incorrect parameter types: The type of volumeType must be number"),
        "invalid valueType");

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

    CHECK_AND_RETURN_RET_LOG(napiAudioVolumeGroupManager != nullptr, result, "napiAudioVolumeGroupManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioVolumeGroupManager->audioGroupMngr_ != nullptr, result,
        "audioGroupMngr_ is nullptr");
    int32_t volLevel = napiAudioVolumeGroupManager->audioGroupMngr_->GetVolume(
        NapiAudioEnum::GetNativeAudioVolumeType(volType), uid);
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
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_TWO, "invalid arguments",
            NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->volType, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get volType failed",
            NAPI_ERR_INVALID_PARAM);
        if (!NapiAudioEnum::IsLegalInputArgumentVolType(context->volType)) {
            context->SignError(context->errCode == NAPI_ERR_INVALID_PARAM?
            NAPI_ERR_INVALID_PARAM : NAPI_ERR_UNSUPPORTED);
        }
        context->status = NapiParamUtils::GetValueInt32(env, context->volLevel, argv[PARAM1]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get volLevel failed",
            NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);
#ifdef FEATURE_HIVIEW_ENABLE
#if !defined(ANDROID_PLATFORM) && !defined(IOS_PLATFORM)
    HiviewDFX::ReportXPowerJsStackSysEvent(env, "VOLUME_CHANGE", "SRC=Audio");
#endif
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
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->intValue == SUCCESS, "setvolume failed",
            NAPI_ERR_SYSTEM);
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SetVolume", executor, complete);
}

napi_value NapiAudioVolumeGroupManager::SetVolumeWithFlag(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioVolumeGroupManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("SetVolumeiWithFlag failed : no memory");
        NapiAudioError::ThrowError(env, "SetVolumeWithFlag failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_THREE, "invalid arguments",
            NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->volType, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get volType failed",
            NAPI_ERR_INVALID_PARAM);
        if (!NapiAudioEnum::IsLegalInputArgumentVolType(context->volType)) {
            context->SignError(context->errCode == NAPI_ERR_INVALID_PARAM?
                NAPI_ERR_INVALID_PARAM : NAPI_ERR_UNSUPPORTED);
        }
        context->status = NapiParamUtils::GetValueInt32(env, context->volLevel, argv[PARAM1]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get volLevel failed",
            NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->volFlag, argv[PARAM2]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get volFlag failed",
            NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);
#ifdef FEATURE_HIVIEW_ENABLE
#if !defined(ANDROID_PLATFORM) && !defined(IOS_PLATFORM)
    HiviewDFX::ReportXPowerJsStackSysEvent(env, "VOLUME_CHANGE", "SRC=Audio");
#endif
#endif

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioVolumeGroupManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioVolumeGroupManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioVolumeGroupManagerStatus(napiAudioVolumeGroupManager, context),
            "audio volume group manager state is error.");
        context->intValue = napiAudioVolumeGroupManager->audioGroupMngr_->SetVolume(
            NapiAudioEnum::GetNativeAudioVolumeType(context->volType), context->volLevel, context->volFlag);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->intValue == SUCCESS, "setvolumeWithFlag failed",
            NAPI_ERR_SYSTEM);
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SetVolumeWithFlag", executor, complete);
}

napi_value NapiAudioVolumeGroupManager::SetSystemVolumeByUid(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioVolumeGroupManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("SetSystemVolumeByUid failed : no memory");
        NapiAudioError::ThrowError(env, "SetVolumeWithFlag failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_THREE, "invalid arguments",
            NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->volType, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get volType failed",
            NAPI_ERR_INVALID_PARAM);
        if (!NapiAudioEnum::IsLegalInputArgumentVolType(context->volType)) {
            context->SignError(context->errCode == NAPI_ERR_INVALID_PARAM?
                NAPI_ERR_INVALID_PARAM : NAPI_ERR_UNSUPPORTED);
        }
        context->status = NapiParamUtils::GetValueInt32(env, context->volLevel, argv[PARAM1]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get volLevel failed",
            NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->uid, argv[PARAM2]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get uid failed",
            NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioVolumeGroupManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioVolumeGroupManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioVolumeGroupManagerStatus(napiAudioVolumeGroupManager, context),
            "audio volume group manager state is error.");
        context->intValue = napiAudioVolumeGroupManager->audioGroupMngr_->SetVolume(
            NapiAudioEnum::GetNativeAudioVolumeType(context->volType), context->volLevel, false, context->uid);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->intValue == SUCCESS, "SetSystemVolumeByUid failed",
            NAPI_ERR_SYSTEM);
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SetSystemVolumeByUid", executor, complete);
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
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments",
            NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->volType, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get volType failed",
            NAPI_ERR_INVALID_PARAM);
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
    napi_value result = nullptr;
    size_t argc = ARGS_ONE;
    napi_value args[ARGS_ONE] = {};
    auto *napiAudioVolumeGroupManager = GetParamWithSync(env, info, argc, args);
    CHECK_AND_RETURN_RET_LOG(argc >= ARGS_ONE, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID, "mandatory parameters are left unspecified"), "invalid arguments");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, args[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_number, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID, "incorrect parameter types: The type of volumeType must be number"),
        "invalid valueType");

    int32_t volType;
    NapiParamUtils::GetValueInt32(env, volType, args[PARAM0]);
    CHECK_AND_RETURN_RET_LOG(NapiAudioEnum::IsLegalInputArgumentVolType(volType),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
        "parameter verification failed: The param of volumeType must be enum AudioVolumeType"), "get volType failed");

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
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments",
            NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->volType, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get volType failed",
            NAPI_ERR_INVALID_PARAM);
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
    CHECK_AND_RETURN_RET_LOG(argc >= ARGS_ONE, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID, "mandatory parameters are left unspecified"), "invalid arguments");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, args[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_number, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID, "incorrect parameter types: The type of volumeType must be number"),
        "invalid valueType");

    int32_t volType;
    NapiParamUtils::GetValueInt32(env, volType, args[PARAM0]);
    CHECK_AND_RETURN_RET_LOG(NapiAudioEnum::IsLegalInputArgumentVolType(volType),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
        "parameter verification failed: The param of volumeType must be enum AudioVolumeType"), "get volType failed");

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
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_TWO, "invalid arguments",
            NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->volType, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get volType failed",
            NAPI_ERR_INVALID_PARAM);
        if (!NapiAudioEnum::IsLegalInputArgumentVolType(context->volType)) {
            context->SignError(context->errCode == NAPI_ERR_INVALID_PARAM?
                NAPI_ERR_INVALID_PARAM : NAPI_ERR_UNSUPPORTED);
        }
        context->status = NapiParamUtils::GetValueBoolean(env, context->isMute, argv[PARAM1]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get isMute failed",
            NAPI_ERR_INVALID_PARAM);
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
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->intValue == SUCCESS, "setmute failed",
            NAPI_ERR_SYSTEM);
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
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments",
            NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->volType, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get volType failed",
            NAPI_ERR_INVALID_PARAM);
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
    CHECK_AND_RETURN_RET_LOG(argc >= ARGS_ONE, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID, "mandatory parameters are left unspecified"), "invalid arguments");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, args[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_number, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID, "incorrect parameter types: The type of volumeType must be number"),
        "invalid valueType");

    int32_t volType;
    NapiParamUtils::GetValueInt32(env, volType, args[PARAM0]);
    CHECK_AND_RETURN_RET_LOG(NapiAudioEnum::IsLegalInputArgumentVolType(volType),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
        "parameter verification failed: The param of volumeType must be enum AudioVolumeType"), "get volType failed");

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
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments",
            NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->ringMode, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get ringMode failed",
            NAPI_ERR_INVALID_PARAM);
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
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments",
            NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueBoolean(env, context->isMute, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get ringMode failed",
            NAPI_ERR_INVALID_PARAM);
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
    CHECK_AND_RETURN_RET_LOG(argc < ARGS_ONE, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID), "invalid arguments");
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
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED), "No system permission");

    auto context = std::make_shared<AudioVolumeGroupManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("no memory failed");
        NapiAudioError::ThrowError(env, "failed no memory", NAPI_ERR_SYSTEM);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "mandatory parameters are left unspecified",
            NAPI_ERR_INPUT_INVALID);
        context->status = NapiParamUtils::GetValueBoolean(env, context->isMute, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok,
            "incorrect parameter types: The type of mute must be boolean", NAPI_ERR_INPUT_INVALID);
    };
    context->GetCbInfo(env, info, inputParser);
    if (context->status != napi_ok) {
        NapiAudioError::ThrowError(env, context->errCode, context->errMessage);
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

napi_value NapiAudioVolumeGroupManager::SetMicMutePersistent(napi_env env, napi_callback_info info)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED), "No system permission");

    auto context = std::make_shared<AudioVolumeGroupManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("no memory failed");
        NapiAudioError::ThrowError(env, "failed no memory", NAPI_ERR_SYSTEM);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_TWO, "mandatory parameters are left unspecified",
            NAPI_ERR_INPUT_INVALID);
        context->status = NapiParamUtils::GetValueBoolean(env, context->isMute, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok,
            "incorrect parameter types: The type of mute must be boolean", NAPI_ERR_INPUT_INVALID);
        context->status = NapiParamUtils::GetValueInt32(env, context->policyType, argv[PARAM1]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok,
            "incorrect parameter types: get policyType failed", NAPI_ERR_INPUT_INVALID);
    };
    context->GetCbInfo(env, info, inputParser);
    if (context->status != napi_ok) {
        NapiAudioError::ThrowError(env, context->errCode, context->errMessage);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioVolumeGroupManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioVolumeGroupManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioVolumeGroupManagerStatus(napiAudioVolumeGroupManager, context),
            "audio volume group manager state is error.");
        context->intValue = napiAudioVolumeGroupManager->audioGroupMngr_->SetMicrophoneMutePersistent(context->isMute,
            static_cast<PolicyType>(context->policyType));
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
    return NapiAsyncWork::Enqueue(env, context, "SetMicMutePersistent", executor, complete);
}

napi_value NapiAudioVolumeGroupManager::GetPersistentMicMuteState(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = ARGS_ONE;
    napi_value args[ARGS_ONE] = {};
    auto *napiAudioVolumeGroupManager = GetParamWithSync(env, info, argc, args);
    CHECK_AND_RETURN_RET_LOG(napiAudioVolumeGroupManager != nullptr, result, "napiAudioVolumeGroupManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioVolumeGroupManager->audioGroupMngr_ != nullptr, result,
        "audioGroupMngr_ is nullptr");
    bool isPersistentMicMute = napiAudioVolumeGroupManager->audioGroupMngr_->GetPersistentMicMuteState();
    NapiParamUtils::SetValueBoolean(env, isPersistentMicMute, result);

    return result;
}

napi_value NapiAudioVolumeGroupManager::IsVolumeUnadjustable(napi_env env, napi_callback_info info)
{
    AUDIO_INFO_LOG("IsVolumeUnadjustable");
    napi_value result = nullptr;
    size_t argc = PARAM0;
    auto *napiAudioVolumeGroupManager = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(argc < ARGS_ONE, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID), "invalid arguments");
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
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "mandatory parameters are left unspecified",
            NAPI_ERR_INPUT_INVALID);
        context->status = NapiParamUtils::GetValueInt32(env, context->adjustType, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok,
            "incorrect parameter types: The type of adjustType must be number", NAPI_ERR_INPUT_INVALID);
        NAPI_CHECK_ARGS_RETURN_VOID(context, NapiAudioEnum::IsLegalInputArgumentVolumeAdjustType(context->adjustType),
            "parameter verification failed: The param of adjustType must be enum VolumeAdjustType",
            NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);

    if ((context->status != napi_ok) && (context->errCode == NAPI_ERR_INPUT_INVALID)) {
        NapiAudioError::ThrowError(env, context->errCode, context->errMessage);
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
                context->SignError(NAPI_ERR_SYSTEM, "System error. Set app volume fail.");
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
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_TWO, "mandatory parameters are left unspecified",
            NAPI_ERR_INPUT_INVALID);
        context->status = NapiParamUtils::GetValueInt32(env, context->volType, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok,
            "incorrect parameter types: The type of volumeType must be number", NAPI_ERR_INPUT_INVALID);
        NAPI_CHECK_ARGS_RETURN_VOID(context, NapiAudioEnum::IsLegalInputArgumentVolType(context->volType) &&
            context->volType != NapiAudioEnum::ALL,
            "The param of volumeType must be enum AudioVolumeType", NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->adjustType, argv[PARAM1]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok,
            "incorrect parameter types: The type of adjustType must be number", NAPI_ERR_INPUT_INVALID);
        NAPI_CHECK_ARGS_RETURN_VOID(context, NapiAudioEnum::IsLegalInputArgumentVolumeAdjustType(context->adjustType),
            "The param of adjustType must be enum VolumeAdjustType", NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);

    if ((context->status != napi_ok) && (context->errCode == NAPI_ERR_INPUT_INVALID)) {
        NapiAudioError::ThrowError(env, context->errCode, context->errMessage);
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
                context->SignError(NAPI_ERR_SYSTEM, "System error. Set app volume fail.");
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
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_THREE, "mandatory parameters are left unspecified",
            NAPI_ERR_INPUT_INVALID);
        context->status = NapiParamUtils::GetValueInt32(env, context->volType, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok,
            "incorrect parameter types: The type of volumeType must be number", NAPI_ERR_INPUT_INVALID);
        NAPI_CHECK_ARGS_RETURN_VOID(context, NapiAudioEnum::IsLegalInputArgumentVolType(context->volType),
            "parameter verification failed: The param of volumeType must be enum AudioVolumeType",
            NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->volLevel, argv[PARAM1]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok,
        "incorrect parameter types: The type of volumeLevel must be number", NAPI_ERR_INPUT_INVALID);
        context->status = NapiParamUtils::GetValueInt32(env, context->deviceType, argv[PARAM2]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, NapiAudioEnum::IsLegalInputArgumentDeviceType(context->deviceType) &&
            (context->status == napi_ok), "parameter verification failed: The param of device must be enum DeviceType",
            NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);

    if ((context->status != napi_ok) && (context->errCode == NAPI_ERR_INPUT_INVALID)) {
        NapiAudioError::ThrowError(env, context->errCode, context->errMessage);
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
            context->SignError(NAPI_ERR_INVALID_PARAM, "volumeInDb invalid");
        } else if (context->volumeInDb < 0) {
            context->SignError(NAPI_ERR_SYSTEM, "System error. Set app volume fail.");
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
    CHECK_AND_RETURN_RET_LOG(argc >= ARGS_THREE, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID, "mandatory parameters are left unspecified"), "invalid arguments");

    int32_t volType;
    int32_t volLevel;
    int32_t deviceType;
    for (size_t i = PARAM0; i < argc; i++) {
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, args[i], &valueType);
        CHECK_AND_RETURN_RET_LOG(valueType == napi_number, NapiAudioError::ThrowErrorAndReturn(env,
            NAPI_ERR_INPUT_INVALID, "incorrect parameter types: The type of parameter must be number"),
            "invalid valueType");
    }
    NapiParamUtils::GetValueInt32(env, volType, args[PARAM0]);
    CHECK_AND_RETURN_RET_LOG(NapiAudioEnum::IsLegalInputArgumentVolType(volType),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
        "parameter verification failed: The param of volumeType must be enum AudioVolumeType"), "get volType failed");
    NapiParamUtils::GetValueInt32(env, volLevel, args[PARAM1]);
    NapiParamUtils::GetValueInt32(env, deviceType, args[PARAM2]);
    CHECK_AND_RETURN_RET_LOG(NapiAudioEnum::IsLegalInputArgumentDeviceType(deviceType),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
        "parameter verification failed: The param of device must be enum DeviceType"), "get deviceType failed");

    CHECK_AND_RETURN_RET_LOG(napiAudioVolumeGroupManager != nullptr, result, "napiAudioVolumeGroupManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioVolumeGroupManager->audioGroupMngr_ != nullptr, result,
        "audioGroupMngr_ is nullptr");
    double volumeInDb = napiAudioVolumeGroupManager->audioGroupMngr_->GetSystemVolumeInDb(
        NapiAudioEnum::GetNativeAudioVolumeType(volType), volLevel, static_cast<DeviceType>(deviceType));
    CHECK_AND_RETURN_RET_LOG(!FLOAT_COMPARE_EQ(static_cast<float>(volumeInDb), static_cast<float>(ERR_INVALID_PARAM)),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM, "volumeInDb invalid"),
        "getsystemvolumeindb failed");
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
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_SYSTEM), "status error");
    CHECK_AND_RETURN_RET_LOG(napiAudioVolumeGroupManager != nullptr, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_NO_MEMORY), "napiAudioVolumeGroupManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioVolumeGroupManager->audioGroupMngr_ != nullptr,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_NO_MEMORY), "audioGroupMngr_ is nullptr");
    if (!cbName.compare(RINGERMODE_CALLBACK_NAME)) {
        undefinedResult = RegisterRingModeCallback(env, args, cbName, napiAudioVolumeGroupManager);
    } else if (!cbName.compare(MIC_STATE_CHANGE_CALLBACK_NAME)) {
        undefinedResult = RegisterMicStateChangeCallback(env, args, cbName, napiAudioVolumeGroupManager);
    } else {
        AUDIO_ERR_LOG("No such callback supported");
        NapiAudioError::ThrowError(env, NAPI_ERR_INVALID_PARAM,
            "parameter verification failed: The param of type is not supported");
    }
    return undefinedResult;
}

napi_value NapiAudioVolumeGroupManager::RegisterRingModeCallback(napi_env env, napi_value *args,
    const std::string &cbName, NapiAudioVolumeGroupManager *napiAudioVolumeGroupManager)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    if (napiAudioVolumeGroupManager->ringerModecallbackNapi_ == nullptr) {
        napiAudioVolumeGroupManager->ringerModecallbackNapi_ = std::make_shared<NapiAudioRingerModeCallback>(env);
        int32_t ret = napiAudioVolumeGroupManager->audioGroupMngr_->SetRingerModeCallback(
            napiAudioVolumeGroupManager->cachedClientId_, napiAudioVolumeGroupManager->ringerModecallbackNapi_);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, result, "SetRingerModeCallback Failed");
    }
    std::shared_ptr<NapiAudioRingerModeCallback> cb =
        std::static_pointer_cast<NapiAudioRingerModeCallback>(napiAudioVolumeGroupManager->ringerModecallbackNapi_);
    cb->SaveCallbackReference(cbName, args[PARAM1]);
    if (!cb->GetRingModeTsfnFlag()) {
        cb->CreateRingModeTsfn(env);
    }
    return result;
}

napi_value NapiAudioVolumeGroupManager::RegisterMicStateChangeCallback(napi_env env, napi_value *args,
    const std::string &cbName, NapiAudioVolumeGroupManager *napiAudioVolumeGroupManager)
{
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
    if (!cb->GetManagerMicStateChangeTsfnFlag()) {
        cb->CreateManagerMicStateChangeTsfn(env);
    }
    AUDIO_DEBUG_LOG("On SetMicStateChangeCallback is successful");
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value NapiAudioVolumeGroupManager::UnregisterCallback(napi_env env, napi_value jsThis,
    size_t argc, napi_value *args, const std::string &cbName)
{
    napi_value undefinedResult = nullptr;
    napi_get_undefined(env, &undefinedResult);

    NapiAudioVolumeGroupManager *napiAudioVolumeGroupManager = nullptr;
    napi_status status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napiAudioVolumeGroupManager));
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_SYSTEM), "status error");
    CHECK_AND_RETURN_RET_LOG(napiAudioVolumeGroupManager != nullptr, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_NO_MEMORY), "napiAudioVolumeGroupManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioVolumeGroupManager->audioGroupMngr_ != nullptr,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_NO_MEMORY), "audioGroupMngr_ is nullptr");
    if (!cbName.compare(RINGERMODE_CALLBACK_NAME)) {
        UnregisterRingerModeCallback(napiAudioVolumeGroupManager, argc, args);
    } else if (!cbName.compare(MIC_STATE_CHANGE_CALLBACK_NAME)) {
        UnregisterMicStateChangeCallback(napiAudioVolumeGroupManager, argc, args);
    } else {
        AUDIO_ERR_LOG("No such callback supported");
        NapiAudioError::ThrowError(env, NAPI_ERR_INVALID_PARAM,
            "parameter verification failed: The param of type is not supported");
    }
    return undefinedResult;
}

void NapiAudioVolumeGroupManager::UnregisterRingerModeCallback(
    NapiAudioVolumeGroupManager *napiAudioVolumeGroupManager, size_t argc, napi_value *args)
{
    CHECK_AND_RETURN_LOG(napiAudioVolumeGroupManager->ringerModecallbackNapi_ != nullptr,
        "ringerModecallbackNapi is null");
    std::shared_ptr<NapiAudioRingerModeCallback> cb = std::static_pointer_cast<NapiAudioRingerModeCallback>(
        napiAudioVolumeGroupManager->ringerModecallbackNapi_);
    
    napi_value callback = nullptr;
    if (argc == ARGS_TWO) {
        callback = args[PARAM1];
        CHECK_AND_RETURN_LOG(cb->IsSameCallback(callback),
            "The callback need to be unregistered is not the same as the registered callback");
    }
    int32_t ret = napiAudioVolumeGroupManager->audioGroupMngr_->UnsetRingerModeCallback(
        napiAudioVolumeGroupManager->cachedClientId_, cb);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "UnsetRingerModeCallback failed");
    cb->RemoveCallbackReference(callback);
    napiAudioVolumeGroupManager->ringerModecallbackNapi_ = nullptr;
    AUDIO_INFO_LOG("UnregisterRingerModeCallback success");
}

void NapiAudioVolumeGroupManager::UnregisterMicStateChangeCallback(
    NapiAudioVolumeGroupManager *napiAudioVolumeGroupManager, size_t argc, napi_value *args)
{
    CHECK_AND_RETURN_LOG(napiAudioVolumeGroupManager->micStateChangeCallbackNapi_ != nullptr,
        "micStateChangeCallbackNapi is null");
    std::shared_ptr<NapiAudioManagerMicStateChangeCallback> cb =
        std::static_pointer_cast<NapiAudioManagerMicStateChangeCallback>(
        napiAudioVolumeGroupManager->micStateChangeCallbackNapi_);
    
    napi_value callback = nullptr;
    if (argc == ARGS_TWO) {
        callback = args[PARAM1];
        CHECK_AND_RETURN_LOG(cb->IsSameCallback(callback),
            "The callback need to be unregistered is not the same as the registered callback");
    }
    int32_t ret = napiAudioVolumeGroupManager->audioGroupMngr_->UnsetMicStateChangeCallback(
        napiAudioVolumeGroupManager->micStateChangeCallbackNapi_);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "UnregisterMicStateChangeCallback failed");
    cb->RemoveCallbackReference(callback);
    napiAudioVolumeGroupManager->micStateChangeCallbackNapi_ = nullptr;
    AUDIO_INFO_LOG("UnregisterMicStateChangeCallback success");
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
    if (napi_typeof(env, args[PARAM1], &handler) != napi_ok || handler != napi_function) {
        AUDIO_ERR_LOG("On type mismatch for parameter 2");
        NapiAudioError::ThrowError(env, NAPI_ERR_INPUT_INVALID,
            "incorrect parameter types: The type of callback must be function");
        return undefinedResult;
    }

    return RegisterCallback(env, jsThis, argc, args, callbackName);
}

napi_value NapiAudioVolumeGroupManager::Off(napi_env env, napi_callback_info info)
{
    AUDIO_DEBUG_LOG("On inter");
    napi_value undefinedResult = nullptr;
    NapiParamUtils::GetUndefinedValue(env);

    const size_t minArgc = ARGS_ONE;
    size_t argc = ARGS_THREE;
    napi_value args[minArgc + PARAM2] = {nullptr, nullptr, nullptr};
    napi_value jsThis = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr);
    if (status != napi_ok || argc < minArgc) {
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

    napi_valuetype handler = napi_undefined;
    if (napi_typeof(env, args[PARAM1], &handler) != napi_ok || handler != napi_function) {
        AUDIO_INFO_LOG("Off type has no parameter 2");
    }
    return UnregisterCallback(env, jsThis, argc, args, callbackName);
}

napi_value NapiAudioVolumeGroupManager::GetMaxAmplitudeForOutputDevice(napi_env env, napi_callback_info info)
{
    AUDIO_DEBUG_LOG("GetMaxAmplitude enter");
    auto context = std::make_shared<AudioVolumeGroupManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("GetMaxAmplitude failed : no memory");
        NapiAudioError::ThrowError(env, "GetMaxAmplitude failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments",
            NAPI_ERR_INPUT_INVALID);
        NapiParamUtils::GetAudioDeviceDescriptor(env, context->outputDeviceDescriptor,
            context->outputBArgTransFlag, argv[PARAM0]);
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioVolumeGroupManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioVolumeGroupManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioVolumeGroupManagerStatus(napiAudioVolumeGroupManager, context),
            "audio volume group manager state is error.");
        context->outputMaxAmplitude = napiAudioVolumeGroupManager->audioGroupMngr_->GetMaxAmplitude(
            context->outputDeviceDescriptor->deviceId_);
        if (FLOAT_COMPARE_EQ(context->outputMaxAmplitude, static_cast<float>(ERR_INVALID_PARAM))) {
            context->SignError(NAPI_ERR_INVALID_PARAM, "Parmeter verification faild. OutputDevice not exist.");
        } else if (context->outputMaxAmplitude < 0) {
            context->SignError(NAPI_ERR_SYSTEM, "System error. Internal variable exception.");
        } else if (!context->outputBArgTransFlag) {
            context->SignError(NAPI_ERR_SYSTEM, "System error. Set app volume fail.");
        }
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueDouble(env, context->outputMaxAmplitude, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "GetMaxAmplitudeForOutputDevice", executor, complete);
}

napi_value NapiAudioVolumeGroupManager::GetMaxAmplitudeForInputDevice(napi_env env, napi_callback_info info)
{
    AUDIO_DEBUG_LOG("GetMaxAmplitude enter");
    auto context = std::make_shared<AudioVolumeGroupManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("GetMaxAmplitude failed : no memory");
        NapiAudioError::ThrowError(env, "GetMaxAmplitude failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "mandatory parameters are left unspecified",
            NAPI_ERR_INPUT_INVALID);
        NapiParamUtils::GetAudioDeviceDescriptor(env, context->inputDeviceDescriptor,
            context->inputBArgTransFlag, argv[PARAM0]);
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioVolumeGroupManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioVolumeGroupManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioVolumeGroupManagerStatus(napiAudioVolumeGroupManager, context),
            "audio volume group manager state is error.");
        context->inputMaxAmplitude = napiAudioVolumeGroupManager->audioGroupMngr_->GetMaxAmplitude(
            context->inputDeviceDescriptor->deviceId_);
        if (FLOAT_COMPARE_EQ(context->inputMaxAmplitude, static_cast<float>(ERR_INVALID_PARAM))) {
            context->SignError(NAPI_ERR_INVALID_PARAM, "maxAmplitude invalid");
        } else if (context->inputMaxAmplitude < 0) {
            context->SignError(NAPI_ERR_SYSTEM, "System error. Internal variable exception.");
        } else if (!context->inputBArgTransFlag) {
            context->SignError(NAPI_ERR_SYSTEM, "System error. Set app volume fail.");
        }
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueDouble(env, context->inputMaxAmplitude, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "GetMaxAmplitudeForInputDevice", executor, complete);
}
}  // namespace AudioStandard
}  // namespace OHOS
