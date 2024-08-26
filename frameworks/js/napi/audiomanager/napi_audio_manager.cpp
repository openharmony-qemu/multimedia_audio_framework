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
#define LOG_TAG "NapiAudioManager"
#endif

#include "napi_audio_manager.h"
#include "napi_audio_routing_manager.h"
#include "napi_audio_stream_manager.h"
#include "napi_audio_volume_manager.h"
#include "napi_audio_interrupt_manager.h"
#include "napi_audio_spatialization_manager.h"
#include "napi_audio_enum.h"
#include "napi_audio_error.h"
#include "napi_param_utils.h"
#include "audio_errors.h"
#include "audio_manager_log.h"
#include "audio_utils.h"
#ifdef FEATURE_HIVIEW_ENABLE
#if !defined(ANDROID_PLATFORM) && !defined(IOS_PLATFORM)
#include "xpower_event_js.h"
#endif
#endif
#include "napi_audio_manager_callbacks.h"
#include "napi_audio_ringermode_callback.h"
#include "napi_audio_manager_interrupt_callback.h"
#include "napi_audio_volume_key_event.h"
#if !defined(ANDROID_PLATFORM) && !defined(IOS_PLATFORM)
#include "napi_audio_session_manager.h"
#endif

namespace OHOS {
namespace AudioStandard {
using namespace std;
using namespace HiviewDFX;
static __thread napi_ref g_managerConstructor = nullptr;

NapiAudioManager::NapiAudioManager()
    : audioMngr_(nullptr), env_(nullptr) {}

NapiAudioManager::~NapiAudioManager()
{
    AUDIO_DEBUG_LOG("NapiAudioManager::~NapiAudioManager()");
}

NapiAudioManager* NapiAudioManager::GetParamWithSync(const napi_env &env, napi_callback_info info,
    size_t &argc, napi_value *args)
{
    napi_status status;
    NapiAudioManager *napiAudioManager = nullptr;
    napi_value jsThis = nullptr;

    status = napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok && jsThis != nullptr, nullptr,
        "GetParamWithSync fail to napi_get_cb_info");

    status = napi_unwrap(env, jsThis, (void **)&napiAudioManager);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, nullptr, "napi_unwrap failed");
    CHECK_AND_RETURN_RET_LOG(napiAudioManager != nullptr && napiAudioManager->audioMngr_ !=
        nullptr, napiAudioManager, "GetParamWithSync fail to napi_unwrap");
    return napiAudioManager;
}

bool NapiAudioManager::CheckContextStatus(std::shared_ptr<AudioManagerAsyncContext> context)
{
    CHECK_AND_RETURN_RET_LOG(context != nullptr, false, "context object is nullptr.");
    if (context->native == nullptr) {
        context->SignError(NAPI_ERR_SYSTEM);
        AUDIO_ERR_LOG("context object state is error.");
        return false;
    }
    return true;
}

bool NapiAudioManager::CheckAudioManagerStatus(NapiAudioManager *napi,
    std::shared_ptr<AudioManagerAsyncContext> context)
{
    CHECK_AND_RETURN_RET_LOG(napi != nullptr, false, "napi object is nullptr.");
    if (napi->audioMngr_ == nullptr) {
        context->SignError(NAPI_ERR_SYSTEM);
        AUDIO_ERR_LOG("audioMngr_ is nullptr.");
        return false;
    }
    return true;
}

napi_status NapiAudioManager::InitNapiAudioManager(napi_env env, napi_value &constructor)
{
    napi_property_descriptor audio_svc_mngr_properties[] = {
        DECLARE_NAPI_FUNCTION("setVolume", SetVolume),
        DECLARE_NAPI_FUNCTION("getVolume", GetVolume),
        DECLARE_NAPI_FUNCTION("getMaxVolume", GetMaxVolume),
        DECLARE_NAPI_FUNCTION("getMinVolume", GetMinVolume),
        DECLARE_NAPI_FUNCTION("getDevices", GetDevices),
        DECLARE_NAPI_FUNCTION("mute", SetStreamMute),
        DECLARE_NAPI_FUNCTION("isMute", IsStreamMute),
        DECLARE_NAPI_FUNCTION("isActive", IsStreamActive),
        DECLARE_NAPI_FUNCTION("setRingerMode", SetRingerMode),
        DECLARE_NAPI_FUNCTION("getRingerMode", GetRingerMode),
        DECLARE_NAPI_FUNCTION("setAudioScene", SetAudioScene),
        DECLARE_NAPI_FUNCTION("getAudioScene", GetAudioScene),
        DECLARE_NAPI_FUNCTION("getAudioSceneSync", GetAudioSceneSync),
        DECLARE_NAPI_FUNCTION("setDeviceActive", SetDeviceActive),
        DECLARE_NAPI_FUNCTION("isDeviceActive", IsDeviceActive),
        DECLARE_NAPI_FUNCTION("setAudioParameter", SetAudioParameter),
        DECLARE_NAPI_FUNCTION("getAudioParameter", GetAudioParameter),
        DECLARE_NAPI_FUNCTION("setExtraParameters", SetExtraParameters),
        DECLARE_NAPI_FUNCTION("getExtraParameters", GetExtraParameters),
        DECLARE_NAPI_FUNCTION("setMicrophoneMute", SetMicrophoneMute),
        DECLARE_NAPI_FUNCTION("isMicrophoneMute", IsMicrophoneMute),
        DECLARE_NAPI_FUNCTION("requestIndependentInterrupt", RequestIndependentInterrupt),
        DECLARE_NAPI_FUNCTION("abandonIndependentInterrupt", AbandonIndependentInterrupt),
        DECLARE_NAPI_FUNCTION("on", On),
        DECLARE_NAPI_FUNCTION("off", Off),
        DECLARE_NAPI_FUNCTION("getStreamManager", GetStreamManager),
#if !defined(ANDROID_PLATFORM) && !defined(IOS_PLATFORM)
        DECLARE_NAPI_FUNCTION("getSessionManager", GetSessionManager),
#endif
        DECLARE_NAPI_FUNCTION("getRoutingManager", GetRoutingManager),
        DECLARE_NAPI_FUNCTION("getVolumeManager", GetVolumeManager),
        DECLARE_NAPI_FUNCTION("getInterruptManager", GetInterruptManager),
        DECLARE_NAPI_FUNCTION("getSpatializationManager", GetSpatializationManager),
        DECLARE_NAPI_FUNCTION("disableSafeMediaVolume", DisableSafeMediaVolume),
    };

    napi_status status = napi_define_class(env, NAPI_AUDIO_MNGR_CLASS_NAME.c_str(), NAPI_AUTO_LENGTH,
        Construct, nullptr,
        sizeof(audio_svc_mngr_properties) / sizeof(audio_svc_mngr_properties[PARAM0]),
        audio_svc_mngr_properties, &constructor);
    return status;
}

napi_value NapiAudioManager::Init(napi_env env, napi_value exports)
{
    AUDIO_DEBUG_LOG("Init");
    napi_status status;
    napi_value constructor;
    napi_value result = nullptr;
    const int32_t refCount = 1;

    napi_property_descriptor static_prop[] = {
        DECLARE_NAPI_STATIC_FUNCTION("getAudioManager", GetAudioManager),
    };

    status = InitNapiAudioManager(env, constructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "InitNapiAudioRenderer fail");

    status = napi_create_reference(env, constructor, refCount, &g_managerConstructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_create_reference fail");
    status = napi_set_named_property(env, exports, NAPI_AUDIO_MNGR_CLASS_NAME.c_str(), constructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_set_named_property fail");
    status = napi_define_properties(env, exports, sizeof(static_prop) / sizeof(static_prop[PARAM0]),
        static_prop);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_define_properties fail");
    return exports;
}

void NapiAudioManager::Destructor(napi_env env, void *nativeObject, void *finalizeHint)
{
    if (nativeObject != nullptr) {
        auto obj = static_cast<NapiAudioManager*>(nativeObject);
        ObjectRefMap<NapiAudioManager>::DecreaseRef(obj);
        AUDIO_DEBUG_LOG("NapiAudioManager::Destructor delete NapiAudioManager obj done");
    }
}

napi_value NapiAudioManager::Construct(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value jsThis;
    napi_value undefinedResult = nullptr;
    NapiParamUtils::GetUndefinedValue(env);
    size_t argCount = PARAM0;

    status = napi_get_cb_info(env, info, &argCount, nullptr, &jsThis, nullptr);
    if (status == napi_ok) {
        unique_ptr<NapiAudioManager> managerNapi = make_unique<NapiAudioManager>();
        if (managerNapi != nullptr) {
            ObjectRefMap<NapiAudioManager>::Insert(managerNapi.get());
            managerNapi->env_ = env;
            managerNapi->audioMngr_ = AudioSystemManager::GetInstance();
            managerNapi->cachedClientId_ = getpid();

            status = napi_wrap(env, jsThis, static_cast<void*>(managerNapi.get()),
                               NapiAudioManager::Destructor, nullptr, nullptr);
            if (status != napi_ok) {
                ObjectRefMap<NapiAudioManager>::Erase(managerNapi.get());
                return undefinedResult;
            }
            managerNapi.release();
            return jsThis;
        }
    }
    return undefinedResult;
}

napi_value NapiAudioManager::CreateAudioManagerWrapper(napi_env env)
{
    napi_status status;
    napi_value result = nullptr;
    napi_value constructor;

    status = napi_get_reference_value(env, g_managerConstructor, &constructor);
    if (status != napi_ok) {
        AUDIO_ERR_LOG("Failed in CreateAudioManagerWrapper, %{public}d", status);
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

napi_value NapiAudioManager::GetAudioManager(napi_env env, napi_callback_info info)
{
    napi_status status;
    size_t argCount = PARAM0;

    status = napi_get_cb_info(env, info, &argCount, nullptr, nullptr, nullptr);
    if (status != napi_ok || argCount != 0) {
        AUDIO_ERR_LOG("Invalid arguments!");
        return nullptr;
    }

    return NapiAudioManager::CreateAudioManagerWrapper(env);
}

napi_value NapiAudioManager::GetStreamManager(napi_env env, napi_callback_info info)
{
    napi_status status;
    size_t argCount = PARAM0;

    status = napi_get_cb_info(env, info, &argCount, nullptr, nullptr, nullptr);
    if (status != napi_ok || argCount != 0) {
        AUDIO_ERR_LOG("Invalid arguments!");
        return nullptr;
    }

    return NapiAudioStreamMgr::CreateStreamManagerWrapper(env);
}

#if !defined(ANDROID_PLATFORM) && !defined(IOS_PLATFORM)
napi_value NapiAudioManager::GetSessionManager(napi_env env, napi_callback_info info)
{
    napi_status status;
    size_t argCount = PARAM0;

    status = napi_get_cb_info(env, info, &argCount, nullptr, nullptr, nullptr);
    if (status != napi_ok || argCount != 0) {
        AUDIO_ERR_LOG("Invalid arguments!");
        return nullptr;
    }

    return NapiAudioSessionMgr::CreateSessionManagerWrapper(env);
}
#endif

napi_value NapiAudioManager::GetRoutingManager(napi_env env, napi_callback_info info)
{
    napi_status status;
    size_t argCount = PARAM0;

    status = napi_get_cb_info(env, info, &argCount, nullptr, nullptr, nullptr);
    if (status != napi_ok || argCount != 0) {
        AUDIO_ERR_LOG("Invalid arguments!");
        return nullptr;
    }

    return NapiAudioRoutingManager::CreateRoutingManagerWrapper(env);
}

napi_value NapiAudioManager::GetVolumeManager(napi_env env, napi_callback_info info)
{
    napi_status status;
    size_t argCount = PARAM0;

    status = napi_get_cb_info(env, info, &argCount, nullptr, nullptr, nullptr);
    if (status != napi_ok || argCount != 0) {
        AUDIO_ERR_LOG("Invalid arguments!");
        return nullptr;
    }

    return NapiAudioVolumeManager::CreateVolumeManagerWrapper(env);
}

napi_value NapiAudioManager::GetInterruptManager(napi_env env, napi_callback_info info)
{
    napi_status status;
    size_t argCount = PARAM0;

    status = napi_get_cb_info(env, info, &argCount, nullptr, nullptr, nullptr);
    if (status != napi_ok || argCount != 0) {
        AUDIO_ERR_LOG("Invalid arguments!");
        return nullptr;
    }

    return NapiAudioInterruptManager::CreateInterruptManagerWrapper(env);
}

napi_value NapiAudioManager::GetSpatializationManager(napi_env env, napi_callback_info info)
{
    napi_status status;
    size_t argCount = 0;
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED), "No system permission");

    status = napi_get_cb_info(env, info, &argCount, nullptr, nullptr, nullptr);
    if (status != napi_ok || argCount != 0) {
        AUDIO_ERR_LOG("Invalid arguments!");
        return nullptr;
    }

    return NapiAudioSpatializationManager::CreateSpatializationManagerWrapper(env);
}

napi_value NapiAudioManager::SetVolume(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("SetVolume failed : no memory");
        NapiAudioError::ThrowError(env, "SetVolume failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_TWO, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->volType, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get volType failed", NAPI_ERR_INVALID_PARAM);
        NAPI_CHECK_ARGS_RETURN_VOID(context, NapiAudioEnum::IsLegalInputArgumentVolType(context->volType),
            "get volType unsupport", NAPI_ERR_UNSUPPORTED);
        context->status = NapiParamUtils::GetValueInt32(env, context->volLevel, argv[PARAM1]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get volLevel failed", NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);
#ifdef FEATURE_HIVIEW_ENABLE
#if !defined(ANDROID_PLATFORM) && !defined(IOS_PLATFORM)
    HiviewDFX::ReportXPowerJsStackSysEvent(env, "VOLUME_CHANGE", "SRC=Audio");
#endif
#endif

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioManagerStatus(napiAudioManager, context),
            "audio manager state is error.");
        context->intValue = napiAudioManager->audioMngr_->SetVolume(
            NapiAudioEnum::GetNativeAudioVolumeType(context->volType), context->volLevel);
        if (context->intValue != SUCCESS) {
            context->SignError(context->intValue);
        }
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SetVolume", executor, complete);
}

napi_value NapiAudioManager::GetVolume(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("GetVolume failed : no memory");
        NapiAudioError::ThrowError(env, "GetVolume failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->volType, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get volType failed", NAPI_ERR_INVALID_PARAM);
        NAPI_CHECK_ARGS_RETURN_VOID(context, NapiAudioEnum::IsLegalInputArgumentVolType(context->volType),
            "get volType unsupport", NAPI_ERR_UNSUPPORTED);
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioManagerStatus(napiAudioManager, context),
            "audio manager state is error.");
        context->volLevel = napiAudioManager->audioMngr_->GetVolume(
            NapiAudioEnum::GetNativeAudioVolumeType(context->volType));
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueInt32(env, context->volLevel, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "GetVolume", executor, complete);
}

napi_value NapiAudioManager::GetMaxVolume(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("GetMaxVolume failed : no memory");
        NapiAudioError::ThrowError(env, "GetMaxVolume failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->volType, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get volType failed", NAPI_ERR_INVALID_PARAM);
        NAPI_CHECK_ARGS_RETURN_VOID(context, NapiAudioEnum::IsLegalInputArgumentVolType(context->volType),
            "get volType unsupport", NAPI_ERR_UNSUPPORTED);
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioManagerStatus(napiAudioManager, context),
            "audio manager state is error.");
        context->intValue = napiAudioManager->audioMngr_->GetMaxVolume(
            NapiAudioEnum::GetNativeAudioVolumeType(context->volType));
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueInt32(env, context->intValue, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "GetMaxVolume", executor, complete);
}

napi_value NapiAudioManager::GetMinVolume(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("GetMinVolume failed : no memory");
        NapiAudioError::ThrowError(env, "GetMinVolume failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->volType, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get volType failed", NAPI_ERR_INVALID_PARAM);
        NAPI_CHECK_ARGS_RETURN_VOID(context, NapiAudioEnum::IsLegalInputArgumentVolType(context->volType),
            "get volType unsupport", NAPI_ERR_UNSUPPORTED);
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioManagerStatus(napiAudioManager, context),
            "audio manager state is error.");
        context->intValue = napiAudioManager->audioMngr_->GetMinVolume(
            NapiAudioEnum::GetNativeAudioVolumeType(context->volType));
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueInt32(env, context->intValue, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "GetMinVolume", executor, complete);
}

napi_value NapiAudioManager::GetDevices(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("GetDevices failed : no memory");
        NapiAudioError::ThrowError(env, "GetDevices failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->deviceFlag, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get deviceFlag failed",
            NAPI_ERR_INVALID_PARAM);
        NAPI_CHECK_ARGS_RETURN_VOID(context, NapiAudioEnum::IsLegalInputArgumentDeviceFlag(context->deviceFlag),
            "get deviceFlag unsupport", NAPI_ERR_UNSUPPORTED);
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioManagerStatus(napiAudioManager, context),
            "audio manager state is error.");
        context->deviceDescriptors = napiAudioManager->audioMngr_->GetDevices(
            static_cast<DeviceFlag>(context->deviceFlag));
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetDeviceDescriptors(env, context->deviceDescriptors, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "GetDevices", executor, complete);
}

napi_value NapiAudioManager::SetStreamMute(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("SetStreamMute failed : no memory");
        NapiAudioError::ThrowError(env, "SetStreamMute failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_TWO, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->volType, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get volType failed",
            NAPI_ERR_INVALID_PARAM);
        NAPI_CHECK_ARGS_RETURN_VOID(context, NapiAudioEnum::IsLegalInputArgumentVolType(context->volType),
            "get volType unsupport", NAPI_ERR_UNSUPPORTED);
        context->status = NapiParamUtils::GetValueBoolean(env, context->isMute, argv[PARAM1]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get ismute failed", NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioManagerStatus(napiAudioManager, context),
            "audio manager state is error.");
        context->intValue = napiAudioManager->audioMngr_->SetMute(
            NapiAudioEnum::GetNativeAudioVolumeType(context->volType), context->isMute);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->intValue == SUCCESS, "SetMute failed", NAPI_ERR_SYSTEM);
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SetStreamMute", executor, complete);
}

napi_value NapiAudioManager::IsStreamMute(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("IsStreamMute failed : no memory");
        NapiAudioError::ThrowError(env, "IsStreamMute failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->volType, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get volType failed",
            NAPI_ERR_INVALID_PARAM);
        NAPI_CHECK_ARGS_RETURN_VOID(context, NapiAudioEnum::IsLegalInputArgumentVolType(context->volType),
            "get volType unsupport", NAPI_ERR_UNSUPPORTED);
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioManagerStatus(napiAudioManager, context),
            "audio manager state is error.");
        context->isMute = napiAudioManager->audioMngr_->IsStreamMute(
            NapiAudioEnum::GetNativeAudioVolumeType(context->volType));
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueBoolean(env, context->isMute, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "IsStreamMute", executor, complete);
}

napi_value NapiAudioManager::IsStreamActive(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("IsStreamActive failed : no memory");
        NapiAudioError::ThrowError(env, "IsStreamActive failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->volType, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get volType failed",
            NAPI_ERR_INVALID_PARAM);
        NAPI_CHECK_ARGS_RETURN_VOID(context, NapiAudioEnum::IsLegalInputArgumentVolType(context->volType),
            "get volType unsupport", NAPI_ERR_UNSUPPORTED);
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioManagerStatus(napiAudioManager, context),
            "audio manager state is error.");
        context->isActive = napiAudioManager->audioMngr_->IsStreamActive(
            NapiAudioEnum::GetNativeAudioVolumeType(context->volType));
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueBoolean(env, context->isActive, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "IsStreamActive", executor, complete);
}

napi_value NapiAudioManager::SetRingerMode(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("SetRingerMode failed : no memory");
        NapiAudioError::ThrowError(env, "SetRingerMode failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->ringMode, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get ringMode failed",
            NAPI_ERR_INVALID_PARAM);
        NAPI_CHECK_ARGS_RETURN_VOID(context, NapiAudioEnum::IsLegalInputArgumentRingMode(context->ringMode),
            "get ringMode unsupport", NAPI_ERR_UNSUPPORTED);
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioManagerStatus(napiAudioManager, context),
            "audio manager state is error.");
        napiAudioManager->audioMngr_->SetRingerMode(
            NapiAudioEnum::GetNativeAudioRingerMode(context->ringMode));
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SetRingerMode", executor, complete);
}

napi_value NapiAudioManager::GetRingerMode(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("SetRingerMode failed : no memory");
        NapiAudioError::ThrowError(env, "SetRingerMode failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioManagerStatus(napiAudioManager, context),
            "audio manager state is error.");
        context->ringMode = NapiAudioEnum::GetJsAudioRingMode(napiAudioManager->audioMngr_->GetRingerMode());
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueInt32(env, context->ringMode, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "GetRingerMode", executor, complete);
}

napi_value NapiAudioManager::SetAudioScene(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("SetAudioScene failed : no memory");
        NapiAudioError::ThrowError(env, "SetAudioScene failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->scene, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get scene failed",
            NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioManagerStatus(napiAudioManager, context),
            "audio manager state is error.");
        context->intValue = napiAudioManager->audioMngr_->SetAudioScene(static_cast<AudioScene>(context->scene));
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->intValue == SUCCESS, "SetAudioScene failed", NAPI_ERR_SYSTEM);
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SetAudioScene", executor, complete);
}

napi_value NapiAudioManager::GetAudioScene(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("GetAudioScene failed : no memory");
        NapiAudioError::ThrowError(env, "GetAudioScene failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioManagerStatus(napiAudioManager, context),
            "audio manager state is error.");
        AudioScene audioScene = napiAudioManager->audioMngr_->GetAudioScene();
        if (audioScene == AUDIO_SCENE_VOICE_RINGING) {
            audioScene = AUDIO_SCENE_RINGING;
        }
        context->intValue = audioScene;
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueInt32(env, context->intValue, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "GetAudioScene", executor, complete);
}

napi_value NapiAudioManager::GetAudioSceneSync(napi_env env, napi_callback_info info)
{
    AUDIO_INFO_LOG("GetRenderRateSync");
    napi_value result = nullptr;
    size_t argc = PARAM0;
    auto *napiAudioManager = GetParamWithSync(env, info, argc, nullptr);
    if (argc > PARAM0) {
        NapiAudioError::ThrowError(env, NAPI_ERROR_INVALID_PARAM);
        return result;
    }
    CHECK_AND_RETURN_RET_LOG(napiAudioManager != nullptr, result, "napiAudioManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioManager->audioMngr_ != nullptr, result, "audioMngr_ is nullptr");
    AudioScene audioScene = napiAudioManager->audioMngr_->GetAudioScene();
    if (audioScene == AUDIO_SCENE_VOICE_RINGING) {
        audioScene = AUDIO_SCENE_RINGING;
    }
    NapiParamUtils::SetValueInt32(env, audioScene, result);
    return result;
}

napi_value NapiAudioManager::SetDeviceActive(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("SetDeviceActive failed : no memory");
        NapiAudioError::ThrowError(env, "SetDeviceActive failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_TWO, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->deviceType, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get deviceType failed",
            NAPI_ERR_INVALID_PARAM);
        NAPI_CHECK_ARGS_RETURN_VOID(context, NapiAudioEnum::IsLegalInputArgumentActiveDeviceType(context->deviceType),
            "invaild deviceType", NAPI_ERR_UNSUPPORTED);
        context->status = NapiParamUtils::GetValueBoolean(env, context->isActive, argv[PARAM1]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get isActive failed",
            NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioManagerStatus(napiAudioManager, context),
            "audio manager state is error.");
        context->intValue = napiAudioManager->audioMngr_->SetDeviceActive(
            static_cast<ActiveDeviceType>(context->deviceType), context->isActive);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->intValue == SUCCESS, "SetDeviceActive failed",
            NAPI_ERR_SYSTEM);
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SetDeviceActive", executor, complete);
}

napi_value NapiAudioManager::IsDeviceActive(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("IsDeviceActive failed : no memory");
        NapiAudioError::ThrowError(env, "IsDeviceActive failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->deviceType, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get deviceType failed",
            NAPI_ERR_INVALID_PARAM);
        NAPI_CHECK_ARGS_RETURN_VOID(context, NapiAudioEnum::IsLegalInputArgumentActiveDeviceType(context->deviceType),
            "invaild deviceType", NAPI_ERR_UNSUPPORTED);
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioManagerStatus(napiAudioManager, context),
            "audio manager state is error.");
        context->isActive = napiAudioManager->audioMngr_->IsDeviceActive(
            static_cast<ActiveDeviceType>(context->deviceType));
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueBoolean(env, context->isActive, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "IsDeviceActive", executor, complete);
}

napi_value NapiAudioManager::SetAudioParameter(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("SetAudioParameter failed : no memory");
        NapiAudioError::ThrowError(env, "SetAudioParameter failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_TWO, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        context->key = NapiParamUtils::GetStringArgument(env, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, !context->key.empty(), "get key failed",
            NAPI_ERR_INVALID_PARAM);
        context->valueStr = NapiParamUtils::GetStringArgument(env, argv[PARAM1]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, !context->valueStr.empty(), "get valueStr failed",
            NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioManagerStatus(napiAudioManager, context),
            "audio manager state is error.");
        napiAudioManager->audioMngr_->SetAudioParameter(context->key, context->valueStr);
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SetAudioParameter", executor, complete);
}

napi_value NapiAudioManager::GetAudioParameter(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("GetAudioParameter failed : no memory");
        NapiAudioError::ThrowError(env, "GetAudioParameter failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        context->key = NapiParamUtils::GetStringArgument(env, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, !context->key.empty(), "get key failed",
            NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioManagerStatus(napiAudioManager, context),
            "audio manager state is error.");
        context->valueStr = napiAudioManager->audioMngr_->GetAudioParameter(context->key);
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueString(env, context->valueStr, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "GetAudioParameter", executor, complete);
}

napi_value NapiAudioManager::SetExtraParameters(napi_env env, napi_callback_info info)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED), "No system permission");

    auto context = std::make_shared<AudioManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("set extra parameters failed : no memory");
        NapiAudioError::ThrowError(env, "SetExtraParameters failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_TWO, "mandatory parameters are left unspecified",
            NAPI_ERR_INPUT_INVALID);

        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[PARAM0], &valueType);
        NAPI_CHECK_ARGS_RETURN_VOID(context, valueType == napi_string,
            "incorrect parameter types: The type of mainKey must be string", NAPI_ERR_INPUT_INVALID);
        context->key = NapiParamUtils::GetStringArgument(env, argv[PARAM0]);

        napi_typeof(env, argv[PARAM1], &valueType);
        NAPI_CHECK_ARGS_RETURN_VOID(context, valueType == napi_object,
            "incorrect parameter types: The type of kvpairs must be Record<string, string>", NAPI_ERR_INPUT_INVALID);
        context->status = NapiParamUtils::GetExtraParametersSubKV(env, context->subKvpairs, argv[PARAM1]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get sub key and value failed",
            NAPI_ERR_INPUT_INVALID);
    };
    context->GetCbInfo(env, info, inputParser);
    if (context->status != napi_ok) {
        NapiAudioError::ThrowError(env, context->errCode, context->errMessage);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioManager *>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioManagerStatus(napiAudioManager, context), "audio manager state is error.");

        NAPI_CHECK_ARGS_RETURN_VOID(context, !context->key.empty(),
            "parameter verification failed: get main key failed", NAPI_ERR_INVALID_PARAM);
        NAPI_CHECK_ARGS_RETURN_VOID(context, !context->subKvpairs.empty(),
            "parameter verification failed: sub key and value is empty", NAPI_ERR_INVALID_PARAM);
        context->intValue = napiAudioManager->audioMngr_->SetExtraParameters(context->key, context->subKvpairs);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->intValue != ERR_PERMISSION_DENIED, "permission denied",
            NAPI_ERR_NO_PERMISSION);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->intValue == SUCCESS, "SetExtraParameters failed",
            NAPI_ERR_INVALID_PARAM);
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SetExtraParameters", executor, complete);
}

napi_value NapiAudioManager::GetExtraParameters(napi_env env, napi_callback_info info)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED), "No system permission");

    auto context = std::make_shared<AudioManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("get extra parameters failed : no memory");
        NapiAudioError::ThrowError(env, "GetExtraParameters failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "mandatory parameters are left unspecified",
            NAPI_ERR_INPUT_INVALID);

        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[PARAM0], &valueType);
        NAPI_CHECK_ARGS_RETURN_VOID(context, valueType == napi_string,
            "incorrect parameter types: The type of mainKey must be string", NAPI_ERR_INPUT_INVALID);
        context->key = NapiParamUtils::GetStringArgument(env, argv[PARAM0]);
        
        if (argc > ARGS_ONE) {
            napi_typeof(env, argv[PARAM1], &valueType);
            NAPI_CHECK_ARGS_RETURN_VOID(context, valueType == napi_object,
                "incorrect parameter types: The type of kvpairs must be Record<string, string>",
                NAPI_ERR_INPUT_INVALID);
            context->status = NapiParamUtils::GetExtraParametersVector(env, context->subKeys, argv[PARAM1]);
            NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok,
                "parameter verification failed: get sub key and value failed", NAPI_ERR_INVALID_PARAM);
        }
    };
    context->GetCbInfo(env, info, inputParser);
    if (context->status != napi_ok) {
        NapiAudioError::ThrowError(env, context->errCode, context->errMessage);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioManager *>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioManagerStatus(napiAudioManager, context), "audio manager state is error.");

        NAPI_CHECK_ARGS_RETURN_VOID(context, !context->key.empty(),
            "parameter verification failed: get main key failed", NAPI_ERR_INVALID_PARAM);
        context->intValue = napiAudioManager->audioMngr_->GetExtraParameters(
            context->key, context->subKeys, context->subKvpairs);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->intValue == SUCCESS, "GetExtraParameters failed",
            NAPI_ERR_INVALID_PARAM);
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetExtraAudioParametersInfo(env, context->subKvpairs, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "GetExtraParameters", executor, complete);
}

napi_value NapiAudioManager::SetMicrophoneMute(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("SetMicrophoneMute failed : no memory");
        NapiAudioError::ThrowError(env, "SetMicrophoneMute failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueBoolean(env, context->isMute, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get isMute failed",
            NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioManagerStatus(napiAudioManager, context),
            "audio manager state is error.");
        context->intValue = napiAudioManager->audioMngr_->SetMicrophoneMute(context->isMute);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->intValue == SUCCESS, "SetMicrophoneMute failed",
            NAPI_ERR_SYSTEM);
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SetMicrophoneMute", executor, complete);
}

napi_value NapiAudioManager::IsMicrophoneMute(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("IsMicrophoneMute failed : no memory");
        NapiAudioError::ThrowError(env, "IsMicrophoneMute failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioManagerStatus(napiAudioManager, context),
            "audio manager state is error.");
        context->isMute = napiAudioManager->audioMngr_->IsMicrophoneMute();
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueBoolean(env, context->isMute, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "IsMicrophoneMute", executor, complete);
}

napi_value NapiAudioManager::RequestIndependentInterrupt(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("RequestIndependentInterrupt failed : no memory");
        NapiAudioError::ThrowError(env, "RequestIndependentInterrupt failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->focusType, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get focusType failed",
            NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioManagerStatus(napiAudioManager, context),
            "audio manager state is error.");
        context->isTrue = napiAudioManager->audioMngr_->RequestIndependentInterrupt(
            NapiAudioEnum::GetNativeFocusType(context->focusType));
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueBoolean(env, context->isTrue, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "RequestIndependentInterrupt", executor, complete);
}

napi_value NapiAudioManager::AbandonIndependentInterrupt(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("AbandonIndependentInterrupt failed : no memory");
        NapiAudioError::ThrowError(env, "AbandonIndependentInterrupt failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->focusType, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get focusType failed",
            NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioManagerStatus(napiAudioManager, context),
            "audio manager state is error.");
        context->isTrue = napiAudioManager->audioMngr_->AbandonIndependentInterrupt(
            NapiAudioEnum::GetNativeFocusType(context->focusType));
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueBoolean(env, context->isTrue, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "AbandonIndependentInterrupt", executor, complete);
}

napi_value NapiAudioManager::DisableSafeMediaVolume(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("failed : no memory");
        NapiAudioError::ThrowError(env, "DisableSafeMediaVolume failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioManagerStatus(napiAudioManager, context),
            "audio manager state is error.");
        context->intValue = napiAudioManager->audioMngr_->DisableSafeMediaVolume();
        if (context->intValue == ERR_PERMISSION_DENIED) {
            context->SignError(NAPI_ERR_NO_PERMISSION);
        } else if (context->intValue == ERR_SYSTEM_PERMISSION_DENIED) {
            context->SignError(NAPI_ERR_PERMISSION_DENIED);
        }
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "DisableSafeMediaVolume", executor, complete);
}

napi_value NapiAudioManager::RegisterCallback(napi_env env, napi_value jsThis,
    napi_value *argv, const std::string &cbName)
{
    napi_value undefinedResult = nullptr;
    NapiAudioManager *napiAudioManager = nullptr;
    napi_status status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napiAudioManager));
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_SYSTEM),
        "status error");
    CHECK_AND_RETURN_RET_LOG(napiAudioManager != nullptr, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_NO_MEMORY), "napiAudioManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioManager->audioMngr_ != nullptr, NapiAudioError::ThrowErrorAndReturn(
        env, NAPI_ERR_NO_MEMORY), "audioMngr_ is nullptr");

    if (!cbName.compare(INTERRUPT_CALLBACK_NAME)) {
        RegisterInterruptCallback(env, argv, napiAudioManager);
    } else if (!cbName.compare(RINGERMODE_CALLBACK_NAME)) {
        RegisterRingerModeCallback(env, argv, napiAudioManager);
    } else if (!cbName.compare(VOLUME_CHANGE_CALLBACK_NAME)) {
        AUDIO_INFO_LOG("enter RegisterVolumeChangeCallback");
        RegisterVolumeChangeCallback(env, argv, napiAudioManager);
    } else if (!cbName.compare(DEVICE_CHANGE_CALLBACK_NAME)) {
        RegisterDeviceChangeCallback(env, argv, napiAudioManager);
    }
    return undefinedResult;
}

template<typename T> void NapiAudioManager::RegisterInterruptCallback(napi_env env, const T &argv,
    NapiAudioManager *napiAudioManager)
{
    napi_valuetype paramArg1 = napi_undefined;
    napi_typeof(env, argv[PARAM1], &paramArg1);
    napi_valuetype handler = napi_undefined;
    if (paramArg1 != napi_object) {
        AUDIO_ERR_LOG("NapiAudioManager::On Type mismatch for parameter 2");
        NapiAudioError::ThrowError(env, NAPI_ERR_INPUT_INVALID);
        return;
    }
    if (napi_typeof(env, argv[PARAM2], &handler) != napi_ok || handler != napi_function) {
        AUDIO_ERR_LOG("NapiAudioManager::On type mismatch for parameter 3");
        NapiAudioError::ThrowError(env, NAPI_ERR_INPUT_INVALID);
        return;
    }
    if (napiAudioManager->interruptCallbackNapi_ == nullptr) {
        napiAudioManager->interruptCallbackNapi_ = std::make_shared<NapiAudioManagerInterruptCallback>(env);
        int32_t ret = napiAudioManager->audioMngr_->
            SetAudioManagerInterruptCallback(napiAudioManager->interruptCallbackNapi_);
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "SetAudioManagerInterruptCallback Failed");
    }
    std::lock_guard<std::mutex> lock(napiAudioManager->interruptCallbackNapi_->cbMutex_);
    std::shared_ptr<NapiAudioManagerInterruptCallback> cb =
        std::static_pointer_cast<NapiAudioManagerInterruptCallback>(napiAudioManager->interruptCallbackNapi_);
    cb->SaveCallbackReference(INTERRUPT_CALLBACK_NAME, argv[PARAM2]);
    AudioInterrupt audioInterrupt;
    NapiParamUtils::GetAudioInterrupt(env, audioInterrupt, argv[PARAM1]);
    int32_t ret = napiAudioManager->audioMngr_->RequestAudioFocus(audioInterrupt);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "RequestAudioFocus Failed");
    AUDIO_INFO_LOG("SetAudioManagerInterruptCallback and RequestAudioFocus is successful");
}

template<typename T> void NapiAudioManager::RegisterRingerModeCallback(napi_env env, const T &argv,
    NapiAudioManager *napiAudioManager)
{
    if (napiAudioManager->ringerModecallbackNapi_ == nullptr) {
        napiAudioManager->ringerModecallbackNapi_ = std::make_shared<NapiAudioRingerModeCallback>(env);
        int32_t ret = napiAudioManager->audioMngr_->SetRingerModeCallback(
            napiAudioManager->cachedClientId_, napiAudioManager->ringerModecallbackNapi_);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, NapiAudioError::ThrowError(env, ret),
            "SetRingerModeCallback Failed %{public}d", ret);
    }

    std::shared_ptr<NapiAudioRingerModeCallback> cb =
        std::static_pointer_cast<NapiAudioRingerModeCallback>(napiAudioManager->ringerModecallbackNapi_);
    cb->SaveCallbackReference(RINGERMODE_CALLBACK_NAME, argv[PARAM1]);
}

template<typename T> void NapiAudioManager::RegisterVolumeChangeCallback(napi_env env, const T &argv,
    NapiAudioManager *napiAudioManager)
{
    if (napiAudioManager->volumeKeyEventCallbackNapi_ == nullptr) {
        napiAudioManager->volumeKeyEventCallbackNapi_ = std::make_shared<NapiAudioVolumeKeyEvent>(env);
        int32_t ret = napiAudioManager->audioMngr_->RegisterVolumeKeyEventCallback(napiAudioManager->cachedClientId_,
            napiAudioManager->volumeKeyEventCallbackNapi_, API_8);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, NapiAudioError::ThrowError(env, ret),
            "RegisterVolumeKeyEventCallback Failed %{public}d", ret);
    }
    std::shared_ptr<NapiAudioVolumeKeyEvent> cb =
        std::static_pointer_cast<NapiAudioVolumeKeyEvent>(napiAudioManager->volumeKeyEventCallbackNapi_);
    cb->SaveCallbackReference(VOLUME_CHANGE_CALLBACK_NAME, argv[PARAM1]);
}

template<typename T> void NapiAudioManager::RegisterDeviceChangeCallback(napi_env env, const T &argv,
    NapiAudioManager *napiAudioManager)
{
    if (napiAudioManager->deviceChangeCallbackNapi_ == nullptr) {
        napiAudioManager->deviceChangeCallbackNapi_ = std::make_shared<NapiAudioManagerCallback>(env);
    }
    int32_t ret = napiAudioManager->audioMngr_->SetDeviceChangeCallback(DeviceFlag::ALL_DEVICES_FLAG,
        napiAudioManager->deviceChangeCallbackNapi_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, NapiAudioError::ThrowError(env, ret),
        "NapiAudioManager: SetDeviceChangeCallback Failed %{public}d", ret);
    std::shared_ptr<NapiAudioManagerCallback> cb =
        std::static_pointer_cast<NapiAudioManagerCallback>(napiAudioManager->deviceChangeCallbackNapi_);
    cb->SaveAudioManagerDeviceChangeCbRef(DeviceFlag::ALL_DEVICES_FLAG, argv[PARAM1]);
}

napi_value NapiAudioManager::On(napi_env env, napi_callback_info info)
{
    napi_value undefinedResult = nullptr;
    NapiParamUtils::GetUndefinedValue(env);

    constexpr size_t minArgCount = ARGS_TWO;
    size_t argCount = ARGS_THREE;
    napi_value argv[minArgCount + PARAM1] = {nullptr, nullptr, nullptr};
    napi_value jsThis = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argCount, argv, &jsThis, nullptr);
    if (status != napi_ok || argCount < minArgCount) {
        AUDIO_ERR_LOG("On fail to napi_get_cb_info/Requires min 2 parameters");
        NapiAudioError::ThrowError(env, NAPI_ERR_INPUT_INVALID);
        return undefinedResult;
    }

    napi_valuetype eventType = napi_undefined;
    if (napi_typeof(env, argv[PARAM0], &eventType) != napi_ok || eventType != napi_string) {
        NapiAudioError::ThrowError(env, NAPI_ERR_INPUT_INVALID);
        return undefinedResult;
    }
    std::string callbackName = NapiParamUtils::GetStringArgument(env, argv[PARAM0]);
    AUDIO_INFO_LOG("On callbackName: %{public}s", callbackName.c_str());

    if (argCount == minArgCount) {
        napi_valuetype handler = napi_undefined;
        if (napi_typeof(env, argv[PARAM1], &handler) != napi_ok || handler != napi_function) {
            AUDIO_ERR_LOG("NapiAudioManager::On type mismatch for parameter 2");
            NapiAudioError::ThrowError(env, NAPI_ERR_INPUT_INVALID);
            return undefinedResult;
        }
    }

    return RegisterCallback(env, jsThis, argv, callbackName);
}

template<typename T> void NapiAudioManager::UnregisterInterruptCallback(napi_env env, const T &argv,
    const size_t argCount, NapiAudioManager *napiAudioManager)
{
    napi_valuetype paramArg1 = napi_undefined;
    napi_valuetype handler = napi_undefined;
    if (napi_typeof(env, argv[PARAM1], &paramArg1) != napi_ok || paramArg1 != napi_object) {
        AUDIO_ERR_LOG("Off type mismatch for parameter 2");
        NapiAudioError::ThrowError(env, NAPI_ERR_INPUT_INVALID);
        return;
    }
    if ((argCount == ARGS_THREE) &&
        (napi_typeof(env, argv[PARAM2], &handler) != napi_ok || handler != napi_function)) {
        AUDIO_ERR_LOG("Off type mismatch for parameter 3");
        NapiAudioError::ThrowError(env, NAPI_ERR_INPUT_INVALID);
        return;
    }
    int32_t callbackCount = PARAM0;
    if (napiAudioManager->interruptCallbackNapi_ != nullptr) {
        std::shared_ptr<NapiAudioManagerInterruptCallback> cb =
            std::static_pointer_cast<NapiAudioManagerInterruptCallback>(napiAudioManager->interruptCallbackNapi_);
        if (argCount == ARGS_TWO) {
            cb->RemoveAllCallbackReferences(INTERRUPT_CALLBACK_NAME);
        } else if (argCount == ARGS_THREE) {
            cb->RemoveCallbackReference(INTERRUPT_CALLBACK_NAME, argv[PARAM2]);
        }
        callbackCount = cb->GetInterruptCallbackListSize();
    }
    AUDIO_INFO_LOG("Remove Callback Reference success");
    if (callbackCount == PARAM0) {
        AudioInterrupt audioInterrupt;
        NapiParamUtils::GetAudioInterrupt(env, audioInterrupt, argv[PARAM1]);
        int32_t ret = napiAudioManager->audioMngr_->AbandonAudioFocus(audioInterrupt);
        if (ret) {
            AUDIO_ERR_LOG("Off AbandonAudioFocus Failed");
        }
        ret = napiAudioManager->audioMngr_->UnsetAudioManagerInterruptCallback();
        CHECK_AND_RETURN_LOG(ret == SUCCESS,
            "Off UnsetAudioManagerInterruptCallback Failed");
        if (napiAudioManager->interruptCallbackNapi_ != nullptr) {
            std::lock_guard<std::mutex> lock(napiAudioManager->interruptCallbackNapi_->cbMutex_);
            napiAudioManager->interruptCallbackNapi_.reset();
            napiAudioManager->interruptCallbackNapi_ = nullptr;
        }
        AUDIO_INFO_LOG("Off Abandon Focus and UnsetAudioInterruptCallback success");
    }
}

void NapiAudioManager::UnregisterDeviceChangeCallback(napi_env env, napi_value callback,
    NapiAudioManager *audioMgrNapi)
{
    CHECK_AND_RETURN_LOG(audioMgrNapi->deviceChangeCallbackNapi_ != nullptr,
        "UnregisterDeviceChangeCallback: audio manager deviceChangeCallbackNapi_ is null");
    std::shared_ptr<NapiAudioManagerCallback> cb =
        std::static_pointer_cast<NapiAudioManagerCallback>(audioMgrNapi->deviceChangeCallbackNapi_);
    if (callback != nullptr) {
        cb->RemoveAudioManagerDeviceChangeCbRef(env, callback);
    }
    if (callback == nullptr || cb->GetAudioManagerDeviceChangeCbListSize() == 0) {
        int32_t ret = audioMgrNapi->audioMngr_->UnsetDeviceChangeCallback(DeviceFlag::ALL_DEVICES_FLAG);
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "UnsetDeviceChangeCallback Failed");
        audioMgrNapi->deviceChangeCallbackNapi_.reset();
        audioMgrNapi->deviceChangeCallbackNapi_ = nullptr;
        cb->RemoveAllAudioManagerDeviceChangeCb();
    }
}

napi_value NapiAudioManager::Off(napi_env env, napi_callback_info info)
{
    napi_value undefinedResult = nullptr;
    NapiParamUtils::GetUndefinedValue(env);

    constexpr size_t minArgCount = ARGS_ONE;
    size_t argCount = ARGS_THREE;
    napi_value argv[minArgCount + PARAM2] = {nullptr, nullptr, nullptr};
    napi_value jsThis = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argCount, argv, &jsThis, nullptr);
    if (status != napi_ok || argCount < minArgCount) {
        AUDIO_ERR_LOG("Off fail to napi_get_cb_info/Requires min 1 parameters");
        NapiAudioError::ThrowError(env, NAPI_ERR_INPUT_INVALID);
        return undefinedResult;
    }

    napi_valuetype eventType = napi_undefined;
    if (napi_typeof(env, argv[PARAM0], &eventType) != napi_ok || eventType != napi_string) {
        NapiAudioError::ThrowError(env, NAPI_ERR_INPUT_INVALID);
        return undefinedResult;
    }
    std::string callbackName = NapiParamUtils::GetStringArgument(env, argv[PARAM0]);
    AUDIO_INFO_LOG("Off callbackName: %{public}s", callbackName.c_str());

    NapiAudioManager *napiAudioManager = nullptr;
    status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napiAudioManager));

    if (!callbackName.compare(INTERRUPT_CALLBACK_NAME) && argCount > ARGS_ONE) {
        UnregisterInterruptCallback(env, argv, argCount, napiAudioManager);
    } else if (!callbackName.compare(DEVICE_CHANGE_CALLBACK_NAME)) {
        UnregisterDeviceChangeCallback(env, argv[PARAM1], napiAudioManager);
    } else {
        NapiAudioError::ThrowError(env, NAPI_ERR_INVALID_PARAM);
    }
    return undefinedResult;
}
}  // namespace AudioStandard
}  // namespace OHOS