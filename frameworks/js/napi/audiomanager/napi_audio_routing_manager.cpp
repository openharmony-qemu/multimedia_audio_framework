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
#include "napi_audio_routing_manager.h"
#include "napi_audio_error.h"
#include "napi_param_utils.h"
#include "napi_audio_enum.h"
#include "audio_errors.h"
#include "audio_log.h"
#include "napi_audio_manager_callbacks.h"
#include "napi_audio_rounting_available_devicechange_callback.h"
#include "napi_audio_routing_manager_callbacks.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
using namespace HiviewDFX;
static __thread napi_ref g_routingManagerConstructor = nullptr;

NapiAudioRoutingManager::NapiAudioRoutingManager()
    : audioMngr_(nullptr), env_(nullptr) {}

NapiAudioRoutingManager::~NapiAudioRoutingManager() = default;

static napi_value ThrowErrorAndReturn(napi_env env, int32_t errCode)
{
    NapiAudioError::ThrowError(env, errCode);
    return nullptr;
}

void NapiAudioRoutingManager::Destructor(napi_env env, void *nativeObject, void *finalizeHint)
{
    if (nativeObject != nullptr) {
        auto obj = static_cast<NapiAudioRoutingManager *>(nativeObject);
        ObjectRefMap<NapiAudioRoutingManager>::DecreaseRef(obj);
    }
    AUDIO_INFO_LOG("Destructor is successful");
}

napi_value NapiAudioRoutingManager::Construct(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    size_t argc = ARGS_TWO;
    napi_value argv[ARGS_TWO] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    unique_ptr<NapiAudioRoutingManager> napiAudioRoutingManager = make_unique<NapiAudioRoutingManager>();
    CHECK_AND_RETURN_RET_LOG(napiAudioRoutingManager != nullptr, result, "No memory");

    napiAudioRoutingManager->audioMngr_ = AudioSystemManager::GetInstance();
    napiAudioRoutingManager->audioRoutingMngr_ = AudioRoutingManager::GetInstance();
    napiAudioRoutingManager->env_ = env;
    ObjectRefMap<NapiAudioRoutingManager>::Insert(napiAudioRoutingManager.get());

    status = napi_wrap(env, thisVar, static_cast<void*>(napiAudioRoutingManager.get()),
        NapiAudioRoutingManager::Destructor, nullptr, nullptr);
    if (status != napi_ok) {
        ObjectRefMap<NapiAudioRoutingManager>::Erase(napiAudioRoutingManager.get());
        return result;
    }
    napiAudioRoutingManager.release();
    return thisVar;
}

napi_value NapiAudioRoutingManager::Init(napi_env env, napi_value exports)
{
    napi_status status;
    napi_value constructor;
    napi_value result = nullptr;
    const int32_t refCount = ARGS_ONE;
    napi_get_undefined(env, &result);

    napi_property_descriptor audio_routing_manager_properties[] = {
        DECLARE_NAPI_FUNCTION("getDevices", GetDevices),
        DECLARE_NAPI_FUNCTION("getDevicesSync", GetDevicesSync),
        DECLARE_NAPI_FUNCTION("selectOutputDevice", SelectOutputDevice),
        DECLARE_NAPI_FUNCTION("selectOutputDeviceByFilter", SelectOutputDeviceByFilter),
        DECLARE_NAPI_FUNCTION("selectInputDevice", SelectInputDevice),
        DECLARE_NAPI_FUNCTION("selectInputDeviceByFilter", SelectInputDeviceByFilter),
        DECLARE_NAPI_FUNCTION("setCommunicationDevice", SetCommunicationDevice),
        DECLARE_NAPI_FUNCTION("isCommunicationDeviceActive", IsCommunicationDeviceActive),
        DECLARE_NAPI_FUNCTION("isCommunicationDeviceActiveSync", IsCommunicationDeviceActiveSync),
        DECLARE_NAPI_FUNCTION("getActiveOutputDeviceDescriptors", GetActiveOutputDeviceDescriptors),
        DECLARE_NAPI_FUNCTION("getPreferredOutputDeviceForRendererInfo", GetPreferredOutputDeviceForRendererInfo),
        DECLARE_NAPI_FUNCTION("getPreferOutputDeviceForRendererInfo", GetPreferOutputDeviceForRendererInfo),
        DECLARE_NAPI_FUNCTION("getPreferredOutputDeviceForRendererInfoSync",
            GetPreferredOutputDeviceForRendererInfoSync),
        DECLARE_NAPI_FUNCTION("getPreferredInputDeviceForCapturerInfo", GetPreferredInputDeviceForCapturerInfo),
        DECLARE_NAPI_FUNCTION("getPreferredInputDeviceForCapturerInfoSync", GetPreferredInputDeviceForCapturerInfoSync),
        DECLARE_NAPI_FUNCTION("getAvailableMicrophones", GetAvailableMicrophones),
        DECLARE_NAPI_FUNCTION("getAvailableDevices", GetAvailableDevices),
        DECLARE_NAPI_FUNCTION("on", On),
        DECLARE_NAPI_FUNCTION("off", Off),
    };

    status = napi_define_class(env, NAPI_AUDIO_ROUTING_MANAGER_CLASS_NAME.c_str(), NAPI_AUTO_LENGTH, Construct,
        nullptr, sizeof(audio_routing_manager_properties) / sizeof(audio_routing_manager_properties[PARAM0]),
        audio_routing_manager_properties, &constructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_define_class fail");

    status = napi_create_reference(env, constructor, refCount, &g_routingManagerConstructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_create_reference fail");
    status = napi_set_named_property(env, exports, NAPI_AUDIO_ROUTING_MANAGER_CLASS_NAME.c_str(), constructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_set_named_property fail");
    return exports;
}

bool NapiAudioRoutingManager::CheckContextStatus(std::shared_ptr<AudioRoutingManagerAsyncContext> context)
{
    CHECK_AND_RETURN_RET_LOG(context != nullptr, false, "context object is nullptr.");
    if (context->native == nullptr) {
        context->SignError(NAPI_ERR_SYSTEM);
        AUDIO_ERR_LOG("context object state is error.");
        return false;
    }
    return true;
}

bool NapiAudioRoutingManager::CheckAudioRoutingManagerStatus(NapiAudioRoutingManager *napi,
    std::shared_ptr<AudioRoutingManagerAsyncContext> context)
{
    CHECK_AND_RETURN_RET_LOG(napi != nullptr, false, "napi object is nullptr.");
    if ((napi->audioMngr_ == nullptr) || (napi->audioRoutingMngr_ == nullptr)) {
        context->SignError(NAPI_ERR_SYSTEM);
        AUDIO_ERR_LOG("audioMngr_ is nullptr");
        return false;
    }

    return true;
}

NapiAudioRoutingManager* NapiAudioRoutingManager::GetParamWithSync(const napi_env &env, napi_callback_info info,
    size_t &argc, napi_value *args)
{
    napi_status status;
    NapiAudioRoutingManager *napiAudioRoutingManager = nullptr;
    napi_value jsThis = nullptr;
    status = napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok && jsThis != nullptr, nullptr,
        "GetParamWithSync fail to napi_get_cb_info");

    status = napi_unwrap(env, jsThis, (void **)&napiAudioRoutingManager);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, nullptr, "napi_unwrap failed");
    CHECK_AND_RETURN_RET_LOG(napiAudioRoutingManager != nullptr && napiAudioRoutingManager->audioMngr_  !=
        nullptr, napiAudioRoutingManager, "GetParamWithSync fail to napi_unwrap");
    return napiAudioRoutingManager;
}

napi_value NapiAudioRoutingManager::CreateRoutingManagerWrapper(napi_env env)
{
    napi_status status;
    napi_value result = nullptr;
    napi_value constructor;

    status = napi_get_reference_value(env, g_routingManagerConstructor, &constructor);
    if (status != napi_ok) {
        AUDIO_ERR_LOG("Failed in CreateRoutingManagerWrapper, %{public}d", status);
        goto fail;
    }
    status = napi_new_instance(env, constructor, PARAM0, nullptr, &result);
    if (status != napi_ok) {
        AUDIO_ERR_LOG("napi_new_instance failed, sttaus:%{public}d", status);
        goto fail;
    }
    return result;

fail:
    napi_get_undefined(env, &result);
    return result;
}

napi_value NapiAudioRoutingManager::GetDevices(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioRoutingManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("GetDevices failed : no memory");
        NapiAudioError::ThrowError(env, NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->deviceFlag, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get deviceFlag failed",
            NAPI_ERR_INVALID_PARAM);
        if (!NapiAudioEnum::IsLegalInputArgumentDeviceFlag(context->deviceFlag)) {
            context->SignError(context->errCode ==
                NAPI_ERR_INVALID_PARAM? NAPI_ERR_INVALID_PARAM : NAPI_ERR_UNSUPPORTED);
        }
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioRoutingManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioRoutingManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioRoutingManagerStatus(napiAudioRoutingManager, context),
            "context object state is error.");
        context->deviceDescriptors = napiAudioRoutingManager->audioMngr_->GetDevices(
            static_cast<DeviceFlag>(context->deviceFlag));
    };
    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetDeviceDescriptors(env, context->deviceDescriptors, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "GetDevices", executor, complete);
}

napi_value NapiAudioRoutingManager::GetDevicesSync(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = {};
    auto *napiAudioRoutingManager = GetParamWithSync(env, info, argc, argv);
    CHECK_AND_RETURN_RET_LOG(argc == ARGS_ONE, ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID), "argCount invaild");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_number,
        ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID), "valueType invaild");

    int32_t deviceFlag;
    napi_status status = NapiParamUtils::GetValueInt32(env, deviceFlag, argv[PARAM0]);
    CHECK_AND_RETURN_RET_LOG(NapiAudioEnum::IsLegalInputArgumentDeviceFlag(deviceFlag) && (status == napi_ok),
        ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM), "deviceFlag invaild");

    CHECK_AND_RETURN_RET_LOG(napiAudioRoutingManager != nullptr, result, "napiAudioRoutingManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioRoutingManager->audioMngr_ != nullptr, result, "audioMngr_ nullptr");
    vector<sptr<AudioDeviceDescriptor>> deviceDescriptors = napiAudioRoutingManager->audioMngr_->GetDevices(
        static_cast<DeviceFlag>(deviceFlag));

    NapiParamUtils::SetDeviceDescriptors(env, deviceDescriptors, result);

    return result;
}

napi_value NapiAudioRoutingManager::SelectOutputDevice(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioRoutingManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("SelectOutputDevice failed : no memory");
        NapiAudioError::ThrowError(env, NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        NapiParamUtils::GetAudioDeviceDescriptorVector(env, context->deviceDescriptors,
            context->bArgTransFlag, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->bArgTransFlag, "select output device failed",
            NAPI_ERR_UNSUPPORTED);
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioRoutingManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioRoutingManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioRoutingManagerStatus(napiAudioRoutingManager, context),
            "context object state is error.");
        context->intValue = napiAudioRoutingManager->audioMngr_->SelectOutputDevice(context->deviceDescriptors);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->intValue == SUCCESS, "SelectOutputDevice failed",
            NAPI_ERR_SYSTEM);
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SelectOutputDevice", executor, complete);
}

napi_value NapiAudioRoutingManager::SelectOutputDeviceByFilter(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioRoutingManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("SelectOutputDeviceByFilter failed : no memory");
        NapiAudioError::ThrowError(env, NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_TWO, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        NapiParamUtils::GetAudioRendererFilter(env, context->audioRendererFilter,
            context->bArgTransFlag, argv[PARAM0]);
        NapiParamUtils::GetAudioDeviceDescriptorVector(env, context->deviceDescriptors,
            context->bArgTransFlag, argv[PARAM1]);
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioRoutingManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioRoutingManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioRoutingManagerStatus(napiAudioRoutingManager, context),
            "context object state is error.");
        if (!context->bArgTransFlag) {
            context->SignError(NAPI_ERR_UNSUPPORTED);
        }
        context->intValue = napiAudioRoutingManager->audioMngr_->SelectOutputDevice(context->audioRendererFilter,
            context->deviceDescriptors);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->intValue == SUCCESS, "SelectOutputDeviceByFilter failed",
            NAPI_ERR_SYSTEM);
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SelectOutputDeviceByFilter", executor, complete);
}

napi_value NapiAudioRoutingManager::SelectInputDevice(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioRoutingManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("SelectInputDevice failed : no memory");
        NapiAudioError::ThrowError(env, NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        NapiParamUtils::GetAudioDeviceDescriptorVector(env, context->deviceDescriptors,
            context->bArgTransFlag, argv[PARAM0]);
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioRoutingManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioRoutingManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioRoutingManagerStatus(napiAudioRoutingManager, context),
            "context object state is error.");
        if (!context->bArgTransFlag) {
            context->SignError(NAPI_ERR_INVALID_PARAM);
        }
        context->intValue = napiAudioRoutingManager->audioMngr_->SelectInputDevice(context->deviceDescriptors);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->intValue == SUCCESS, "SelectInputDevice failed", NAPI_ERR_SYSTEM);
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SelectInputDevice", executor, complete);
}

napi_value NapiAudioRoutingManager::SelectInputDeviceByFilter(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioRoutingManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("SelectInputDeviceByFilter failed : no memory");
        NapiAudioError::ThrowError(env, NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_TWO, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetAudioCapturerFilter(env, context->audioCapturerFilter, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "select input device by filter failed",
            NAPI_ERR_INVALID_PARAM);
        NapiParamUtils::GetAudioDeviceDescriptorVector(env, context->deviceDescriptors,
            context->bArgTransFlag, argv[PARAM1]);
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioRoutingManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioRoutingManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioRoutingManagerStatus(napiAudioRoutingManager, context),
            "context object state is error.");
        if (!context->bArgTransFlag) {
            context->SignError(NAPI_ERR_UNSUPPORTED);
        }
        context->intValue = napiAudioRoutingManager->audioMngr_->SelectInputDevice(context->audioCapturerFilter,
            context->deviceDescriptors);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->intValue == SUCCESS, "SelectInputDevice failed", NAPI_ERR_SYSTEM);
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SelectInputDeviceByFilter", executor, complete);
}

napi_value NapiAudioRoutingManager::SetCommunicationDevice(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioRoutingManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("SetCommunicationDevice failed : no memory");
        NapiAudioError::ThrowError(env, NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_TWO, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->deviceType, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "set communication device failed",
            NAPI_ERR_INVALID_PARAM);
        if (!NapiAudioEnum::IsLegalInputArgumentCommunicationDeviceType(context->deviceType)) {
            context->SignError(context->errCode ==
                NAPI_ERR_INVALID_PARAM? NAPI_ERR_INVALID_PARAM : NAPI_ERR_UNSUPPORTED);
        }
        context->status = NapiParamUtils::GetValueBoolean(env, context->isActive, argv[PARAM1]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "set communication device failed",
            NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioRoutingManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioRoutingManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioRoutingManagerStatus(napiAudioRoutingManager, context),
            "context object state is error.");
        context->intValue = napiAudioRoutingManager->audioMngr_->SetDeviceActive(
            static_cast<ActiveDeviceType>(context->deviceType), context->isActive);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->intValue == SUCCESS, "SelectInputDevice failed", NAPI_ERR_SYSTEM);
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SetCommunicationDevice", executor, complete);
}

napi_value NapiAudioRoutingManager::IsCommunicationDeviceActive(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioRoutingManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("IsCommunicationDeviceActive failed : no memory");
        NapiAudioError::ThrowError(env, NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->deviceType, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "IsCommunicationDeviceActive failed",
            NAPI_ERR_INVALID_PARAM);
        if (!NapiAudioEnum::IsLegalInputArgumentActiveDeviceType(context->deviceType)) {
            context->SignError(context->errCode ==
                NAPI_ERR_INVALID_PARAM? NAPI_ERR_INVALID_PARAM : NAPI_ERR_UNSUPPORTED);
        }
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioRoutingManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioRoutingManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioRoutingManagerStatus(napiAudioRoutingManager, context),
            "context object state is error.");
        context->isActive = napiAudioRoutingManager->audioMngr_->IsDeviceActive(
            static_cast<ActiveDeviceType>(context->deviceType));
        context->isTrue = context->isActive;
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueBoolean(env, context->isTrue, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "IsCommunicationDeviceActive", executor, complete);
}

napi_value NapiAudioRoutingManager::IsCommunicationDeviceActiveSync(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = {};
    auto *napiAudioRoutingManager = GetParamWithSync(env, info, argc, argv);
    CHECK_AND_RETURN_RET_LOG(argc == ARGS_ONE, ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID), "argCount invaild");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_number, ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID),
        "valueType invaild");

    int32_t deviceType;
    napi_status status = NapiParamUtils::GetValueInt32(env, deviceType, argv[PARAM0]);
    CHECK_AND_RETURN_RET_LOG(NapiAudioEnum::IsLegalInputArgumentActiveDeviceType(deviceType) && status == napi_ok,
        ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM), "valueType invaild");

    CHECK_AND_RETURN_RET_LOG(napiAudioRoutingManager != nullptr, result, "napiAudioRoutingManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioRoutingManager->audioMngr_ != nullptr, result, "audioMngr_ nullptr");
    bool isActive = napiAudioRoutingManager->audioMngr_->IsDeviceActive(static_cast<ActiveDeviceType>(deviceType));

    NapiParamUtils::SetValueBoolean(env, isActive, result);
    return result;
}

napi_value NapiAudioRoutingManager::GetActiveOutputDeviceDescriptors(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioRoutingManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("GetActiveOutputDeviceDescriptors failed : no memory");
        NapiAudioError::ThrowError(env, NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioRoutingManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioRoutingManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioRoutingManagerStatus(napiAudioRoutingManager, context),
            "context object state is error.");
        context->outDeviceDescriptors = napiAudioRoutingManager->audioMngr_->GetActiveOutputDeviceDescriptors();
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetDeviceDescriptors(env, context->outDeviceDescriptors, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "GetActiveOutputDeviceDescriptors", executor, complete);
}

napi_value NapiAudioRoutingManager::GetPreferredOutputDeviceForRendererInfo(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioRoutingManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("GetPreferredOutputDeviceForRendererInfo failed : no memory");
        NapiAudioError::ThrowError(env, NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments", NAPI_ERR_INPUT_INVALID);
        context->status = NapiParamUtils::GetRendererInfo(env, &(context->rendererInfo), argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok,
            "get preferred output device for rendererInfo failed", NAPI_ERR_INPUT_INVALID);
    };
    context->GetCbInfo(env, info, inputParser);
    if (context->status != napi_ok) {
        NapiAudioError::ThrowError(env, context->errCode);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioRoutingManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioRoutingManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioRoutingManagerStatus(napiAudioRoutingManager, context),
            "context object state is error.");
        if (context->rendererInfo.streamUsage == StreamUsage::STREAM_USAGE_INVALID) {
            context->SignError(NAPI_ERR_INVALID_PARAM);
        } else {
            context->intValue = napiAudioRoutingManager->audioRoutingMngr_->GetPreferredOutputDeviceForRendererInfo(
                context->rendererInfo, context->outDeviceDescriptors);
            NAPI_CHECK_ARGS_RETURN_VOID(context, context->intValue == SUCCESS,
                "GetPreferredOutputDeviceForRendererInfo failed", NAPI_ERR_SYSTEM);
        }
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetDeviceDescriptors(env, context->outDeviceDescriptors, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "GetPreferredOutputDeviceForRendererInfo", executor, complete);
}

napi_value NapiAudioRoutingManager::GetPreferOutputDeviceForRendererInfo(napi_env env, napi_callback_info info)
{
    // for api compatibility, leave some time for applications to adapt to new one
    return GetPreferredOutputDeviceForRendererInfo(env, info);
}

napi_value NapiAudioRoutingManager::GetPreferredOutputDeviceForRendererInfoSync(napi_env env, napi_callback_info info)
{
    AUDIO_INFO_LOG("GetPreferredOutputDeviceForRendererInfoSync");
    napi_value result = nullptr;
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = {};
    auto *napiAudioRoutingManager = GetParamWithSync(env, info, argc, argv);
    CHECK_AND_RETURN_RET_LOG(argc == ARGS_ONE, ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID), "argCount invaild");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_object, ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID),
        "valueType invaild");

    AudioRendererInfo rendererInfo;
    if (NapiParamUtils::GetRendererInfo(env, &rendererInfo, argv[PARAM0]) != napi_ok) {
        NapiAudioError::ThrowError(env, NAPI_ERR_INPUT_INVALID);
        return result;
    } else if (rendererInfo.streamUsage == StreamUsage::STREAM_USAGE_INVALID) {
        NapiAudioError::ThrowError(env, NAPI_ERR_INVALID_PARAM);
        return result;
    }

    vector<sptr<AudioDeviceDescriptor>> outDeviceDescriptors;
    CHECK_AND_RETURN_RET_LOG(napiAudioRoutingManager != nullptr, result, "napiAudioRoutingManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioRoutingManager->audioRoutingMngr_ != nullptr, result,
        "audioRoutingMngr_ nullptr");
    napiAudioRoutingManager->audioRoutingMngr_->GetPreferredOutputDeviceForRendererInfo(
        rendererInfo, outDeviceDescriptors);

    NapiParamUtils::SetDeviceDescriptors(env, outDeviceDescriptors, result);

    return result;
}

napi_value NapiAudioRoutingManager::GetPreferredInputDeviceForCapturerInfo(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioRoutingManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("GetPreferredInputDeviceForCapturerInfo failed : no memory");
        NapiAudioError::ThrowError(env, NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments", NAPI_ERR_INPUT_INVALID);
        context->status = NapiParamUtils::GetAudioCapturerInfo(env, &context->captureInfo, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok,
            "get preferred input device for capturerInfo failed", NAPI_ERR_INPUT_INVALID);
    };
    context->GetCbInfo(env, info, inputParser);

    if (context->status != napi_ok) {
        NapiAudioError::ThrowError(env, context->errCode);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioRoutingManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioRoutingManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioRoutingManagerStatus(napiAudioRoutingManager, context),
            "context object state is error.");
        if (context->captureInfo.sourceType == SourceType::SOURCE_TYPE_INVALID) {
            context->SignError(NAPI_ERR_INVALID_PARAM);
        } else {
            context->intValue = napiAudioRoutingManager->audioRoutingMngr_->GetPreferredInputDeviceForCapturerInfo(
                context->captureInfo, context->inputDeviceDescriptors);
            NAPI_CHECK_ARGS_RETURN_VOID(context, context->intValue == SUCCESS,
                "GetPreferredInputDeviceForCapturerInfo failed", NAPI_ERR_SYSTEM);
        }
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetDeviceDescriptors(env, context->inputDeviceDescriptors, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "GetPreferredInputDeviceForCapturerInfo", executor, complete);
}

napi_value NapiAudioRoutingManager::GetPreferredInputDeviceForCapturerInfoSync(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = {};
    auto *napiAudioRoutingManager = GetParamWithSync(env, info, argc, argv);
    CHECK_AND_RETURN_RET_LOG(argc == ARGS_ONE, ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID), "argCount invaild");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_object,
        ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID), "valueType invaild");

    AudioCapturerInfo capturerInfo;
    napi_status status = NapiParamUtils::GetAudioCapturerInfo(env, &capturerInfo, argv[PARAM0]);
    CHECK_AND_RETURN_RET_LOG((capturerInfo.sourceType != SourceType::SOURCE_TYPE_INVALID) && (status == napi_ok),
        ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM), "sourceType invaild");

    vector<sptr<AudioDeviceDescriptor>> outDeviceDescriptors;
    CHECK_AND_RETURN_RET_LOG(napiAudioRoutingManager != nullptr, result, "napiAudioRoutingManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioRoutingManager->audioRoutingMngr_ != nullptr, result,
        "audioRoutingMngr_ nullptr");
    napiAudioRoutingManager->audioRoutingMngr_->GetPreferredInputDeviceForCapturerInfo(
        capturerInfo, outDeviceDescriptors);

    NapiParamUtils::SetDeviceDescriptors(env, outDeviceDescriptors, result);

    return result;
}

napi_value NapiAudioRoutingManager::GetAvailableMicrophones(napi_env env, napi_callback_info info)
{
    AUDIO_INFO_LOG("GetAvailableMicrophones");
    napi_value result = nullptr;
    size_t argc = PARAM0;
    auto *napiAudioRoutingManager = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(argc == PARAM0, ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID), "argCount invaild");
    CHECK_AND_RETURN_RET_LOG(napiAudioRoutingManager != nullptr, result, "napiAudioRoutingManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioRoutingManager->audioRoutingMngr_ != nullptr, result,
        "audioRoutingMngr_ is nullptr");

    vector<sptr<MicrophoneDescriptor>> micDescs =
        napiAudioRoutingManager->audioRoutingMngr_->GetAvailableMicrophones();

    NapiParamUtils::SetMicrophoneDescriptors(env, micDescs, result);

    return result;
}

napi_value NapiAudioRoutingManager::GetAvailableDevices(napi_env env, napi_callback_info info)
{
    AUDIO_INFO_LOG("GetAvailableDevices");

    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(),
        ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED), "No system permission");

    napi_value result = nullptr;
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = {};
    auto *napiAudioRoutingManager = GetParamWithSync(env, info, argc, argv);
    CHECK_AND_RETURN_RET_LOG(argc == ARGS_ONE, ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID),
        "argcCount invaild");

    napi_valuetype valueType = napi_undefined;
    napi_status status = napi_typeof(env, argv[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok && valueType == napi_number,
        ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID), "valueType invaild");

    int32_t intValue = 0;
    status = napi_get_value_int32(env, argv[PARAM0], &intValue);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok && NapiAudioEnum::IsLegalDeviceUsage(intValue),
        ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM), "invaild deviceusage");

    CHECK_AND_RETURN_RET_LOG(napiAudioRoutingManager != nullptr, result, "napiAudioRoutingManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioRoutingManager->audioRoutingMngr_ != nullptr, result,
        "audioRoutingMngr_ is nullptr");
    AudioDeviceUsage usage = static_cast<AudioDeviceUsage>(intValue);

    vector<std::unique_ptr<AudioDeviceDescriptor>> availableDescs =
        napiAudioRoutingManager->audioRoutingMngr_->GetAvailableDevices(usage);

    vector<sptr<AudioDeviceDescriptor>> availableSptrDescs;
    for (const auto &availableDesc : availableDescs) {
        sptr<AudioDeviceDescriptor> dec = new(std::nothrow) AudioDeviceDescriptor(*availableDesc);
        CHECK_AND_BREAK_LOG(dec != nullptr, "dec mallac failed,no memery.");
        availableSptrDescs.push_back(dec);
    }
    NapiParamUtils::SetDeviceDescriptors(env, availableSptrDescs, result);
    return result;
}

napi_value NapiAudioRoutingManager::RegisterCallback(napi_env env, napi_value jsThis, size_t argc,
    napi_value *args, const std::string &cbName)
{
    napi_value undefinedResult = nullptr;
    NapiAudioRoutingManager *napiRoutingMgr = nullptr;
    napi_status status = napi_unwrap(env, jsThis, reinterpret_cast<void**>(&napiRoutingMgr));
    if ((status != napi_ok) || (napiRoutingMgr == nullptr) || (napiRoutingMgr->audioMngr_ == nullptr) ||
        (napiRoutingMgr->audioRoutingMngr_ == nullptr)) {
        AUDIO_ERR_LOG("NapiAudioRoutingManager::Failed to retrieve stream mgr napi instance.");
        return undefinedResult;
    }

    if (!cbName.compare(DEVICE_CHANGE_CALLBACK_NAME)) {
        RegisterDeviceChangeCallback(env, argc, args, cbName, napiRoutingMgr);
    } else if (!cbName.compare(PREFERRED_OUTPUT_DEVICE_CALLBACK_NAME) ||
        !cbName.compare(PREFER_OUTPUT_DEVICE_CALLBACK_NAME)) {
        RegisterPreferredOutputDeviceChangeCallback(env, argc, args, cbName, napiRoutingMgr);
    } else if (!cbName.compare(PREFERRED_INPUT_DEVICE_CALLBACK_NAME)) {
        RegisterPreferredInputDeviceChangeCallback(env, argc, args, cbName, napiRoutingMgr);
    } else if (!cbName.compare(AVAILABLE_DEVICE_CHANGE_CALLBACK_NAME)) {
        CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(),
            ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED), "No system permission");
        RegisterAvaiableDeviceChangeCallback(env, argc, args, cbName, napiRoutingMgr);
    } else {
        AUDIO_ERR_LOG("NapiAudioRoutingManager::No such supported");
        NapiAudioError::ThrowError(env, NAPI_ERR_INVALID_PARAM);
    }
    return undefinedResult;
}

void NapiAudioRoutingManager::RegisterDeviceChangeCallback(napi_env env, size_t argc, napi_value *args,
    const std::string &cbName, NapiAudioRoutingManager *napiRoutingMgr)
{
    int32_t flag = ARGS_THREE;
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, args[PARAM1], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_number, NapiAudioError::ThrowError(env, NAPI_ERR_INPUT_INVALID),
        "invalid valueType");
    if (valueType == napi_number) {
        NapiParamUtils::GetValueInt32(env, flag, args[PARAM1]);
        AUDIO_INFO_LOG("RegisterDeviceChangeCallback:On deviceFlag: %{public}d", flag);
        if (!NapiAudioEnum::IsLegalInputArgumentDeviceFlag(flag)) {
            NapiAudioError::ThrowError(env, NAPI_ERR_INVALID_PARAM);
        }
    }

    napi_valuetype handler = napi_undefined;
    napi_typeof(env, args[PARAM2], &handler);

    if (handler != napi_function) {
        NapiAudioError::ThrowError(env, NAPI_ERR_INPUT_INVALID);
    }
    DeviceFlag deviceFlag = DeviceFlag(flag);
    if (!napiRoutingMgr->deviceChangeCallbackNapi_) {
        napiRoutingMgr->deviceChangeCallbackNapi_ = std::make_shared<NapiAudioManagerCallback>(env);
    }
    CHECK_AND_RETURN_LOG(napiRoutingMgr->deviceChangeCallbackNapi_,
        "RegisterDeviceChangeCallback: Memory Allocation Failed !");

    int32_t ret = napiRoutingMgr->audioMngr_->SetDeviceChangeCallback(deviceFlag,
        napiRoutingMgr->deviceChangeCallbackNapi_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, NapiAudioError::ThrowError(env, ret),
        "RegisterDeviceChangeCallback: Registering Device Change Callback Failed %{public}d", ret);

    std::shared_ptr<NapiAudioManagerCallback> cb =
        std::static_pointer_cast<NapiAudioManagerCallback>(napiRoutingMgr->deviceChangeCallbackNapi_);
    cb->SaveRoutingManagerDeviceChangeCbRef(deviceFlag, args[PARAM2]);
}

void NapiAudioRoutingManager::RegisterPreferredOutputDeviceChangeCallback(napi_env env, size_t argc, napi_value *args,
    const std::string &cbName, NapiAudioRoutingManager *napiRoutingMgr)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, args[PARAM1], &valueType);
    if (valueType != napi_object) {
        NapiAudioError::ThrowError(env, NAPI_ERR_INPUT_INVALID);
    }

    AudioRendererInfo rendererInfo;
    NapiParamUtils::GetRendererInfo(env, &rendererInfo, args[PARAM1]);
    CHECK_AND_RETURN_RET_LOG(rendererInfo.streamUsage != StreamUsage::STREAM_USAGE_INVALID,
        NapiAudioError::ThrowError(env, NAPI_ERR_INVALID_PARAM), "invalid streamUsage");
    AudioStreamType streamType = AudioSystemManager::GetStreamType(rendererInfo.contentType,
        rendererInfo.streamUsage);

    if (!napiRoutingMgr->preferredOutputDeviceCallbackNapi_) {
        napiRoutingMgr->preferredOutputDeviceCallbackNapi_ =
            std::make_shared<NapiAudioPreferredOutputDeviceChangeCallback>(env);
        CHECK_AND_RETURN_LOG(napiRoutingMgr->preferredOutputDeviceCallbackNapi_ != nullptr,
            "RegisterPreferredOutputDeviceChangeCallback: Memory Allocation Failed !!");

        int32_t ret = napiRoutingMgr->audioRoutingMngr_->SetPreferredOutputDeviceChangeCallback(
            rendererInfo, napiRoutingMgr->preferredOutputDeviceCallbackNapi_);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, NapiAudioError::ThrowError(env, ret),
            "Registering Active Output DeviceChange Callback Failed %{public}d", ret);
    }

    std::shared_ptr<NapiAudioPreferredOutputDeviceChangeCallback> cb =
        std::static_pointer_cast<NapiAudioPreferredOutputDeviceChangeCallback>(
        napiRoutingMgr->preferredOutputDeviceCallbackNapi_);
    cb->SaveCallbackReference(streamType, args[PARAM2]);
}

void NapiAudioRoutingManager::RegisterPreferredInputDeviceChangeCallback(napi_env env, size_t argc, napi_value *args,
    const std::string &cbName, NapiAudioRoutingManager *napiRoutingMgr)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, args[PARAM1], &valueType);
    if (valueType != napi_object) {
        NapiAudioError::ThrowError(env, NAPI_ERR_INPUT_INVALID);
    }

    AudioCapturerInfo captureInfo;
    NapiParamUtils::GetAudioCapturerInfo(env, &captureInfo, args[PARAM1]);

    CHECK_AND_RETURN_RET_LOG(captureInfo.sourceType != SourceType::SOURCE_TYPE_INVALID,
        NapiAudioError::ThrowError(env, NAPI_ERR_INVALID_PARAM), "invaild sourceType");

    if (!napiRoutingMgr->preferredInputDeviceCallbackNapi_) {
        napiRoutingMgr->preferredInputDeviceCallbackNapi_ =
            std::make_shared<NapiAudioPreferredInputDeviceChangeCallback>(env);
        CHECK_AND_RETURN_LOG(napiRoutingMgr->preferredInputDeviceCallbackNapi_ != nullptr,
            "RegisterPreferredInputDeviceChangeCallback: Memory Allocation Failed !!");

        int32_t ret = napiRoutingMgr->audioRoutingMngr_->SetPreferredInputDeviceChangeCallback(
            captureInfo, napiRoutingMgr->preferredInputDeviceCallbackNapi_);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, NapiAudioError::ThrowError(env, ret),
            "Registering Active Input DeviceChange Callback Failed %{public}d", ret);
    }

    std::shared_ptr<NapiAudioPreferredInputDeviceChangeCallback> cb =
        std::static_pointer_cast<NapiAudioPreferredInputDeviceChangeCallback>(
        napiRoutingMgr->preferredInputDeviceCallbackNapi_);
    cb->SaveCallbackReference(captureInfo.sourceType, args[PARAM2]);
}

void NapiAudioRoutingManager::RegisterAvaiableDeviceChangeCallback(napi_env env, size_t argc, napi_value *args,
    const std::string &cbName, NapiAudioRoutingManager *napiRoutingMgr)
{
    int32_t flag = 0;
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, args[PARAM1], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_number, NapiAudioError::ThrowError(env, NAPI_ERR_INPUT_INVALID),
        "invaild Type");

    NapiParamUtils::GetValueInt32(env, flag, args[PARAM1]);
    AUDIO_INFO_LOG("RegisterDeviceChangeCallback:On deviceFlag: %{public}d", flag);
    if (!NapiAudioEnum::IsLegalDeviceUsage(flag)) {
        NapiAudioError::ThrowError(env, NAPI_ERR_INVALID_PARAM);
    }

    napi_valuetype handler = napi_undefined;
    napi_typeof(env, args[PARAM2], &handler);
    if (handler != napi_function) {
        NapiAudioError::ThrowError(env, NAPI_ERR_INPUT_INVALID);
    }
    AudioDeviceUsage usage = static_cast<AudioDeviceUsage>(flag);
    if (!napiRoutingMgr->availableDeviceChangeCallbackNapi_) {
        napiRoutingMgr->availableDeviceChangeCallbackNapi_ =
            std::make_shared<NapiAudioRountingAvailableDeviceChangeCallback>(env);
    }
    CHECK_AND_RETURN_LOG(napiRoutingMgr->availableDeviceChangeCallbackNapi_ != nullptr,
        "RegisterDeviceChangeCallback: Memory Allocation Failed !");

    int32_t ret = napiRoutingMgr->audioMngr_->SetAvailableDeviceChangeCallback(usage,
        napiRoutingMgr->availableDeviceChangeCallbackNapi_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, NapiAudioError::ThrowError(env, ret),
        "RegisterDeviceChangeCallback: Registering Device Change Callback Failed %{public}d", ret);

    std::shared_ptr<NapiAudioRountingAvailableDeviceChangeCallback> cb =
        std::static_pointer_cast<NapiAudioRountingAvailableDeviceChangeCallback>(
        napiRoutingMgr->availableDeviceChangeCallbackNapi_);
    cb->SaveRoutingAvailbleDeviceChangeCbRef(usage, args[PARAM2]);
}

napi_value NapiAudioRoutingManager::On(napi_env env, napi_callback_info info)
{
    const size_t requireArgc = ARGS_TWO;
    const size_t maxArgc = ARGS_THREE;
    size_t argc = ARGS_THREE;

    napi_value undefinedResult = nullptr;
    napi_get_undefined(env, &undefinedResult);

    napi_value args[requireArgc + PARAM1] = { nullptr, nullptr, nullptr };
    napi_value jsThis = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr);
    bool isArgcCountRight = argc == requireArgc || argc == maxArgc;
    CHECK_AND_RETURN_RET_LOG(status == napi_ok && isArgcCountRight, ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID),
        "status or isArgcCountRight error");

    napi_valuetype eventType = napi_undefined;
    napi_typeof(env, args[PARAM0], &eventType);
    CHECK_AND_RETURN_RET_LOG(eventType == napi_string, ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID),
        "eventType is invaild");
    std::string callbackName = NapiParamUtils::GetStringArgument(env, args[PARAM0]);
    AUDIO_INFO_LOG("On callbackName: %{public}s", callbackName.c_str());

    if (argc == requireArgc) {
        napi_valuetype handler = napi_undefined;
        napi_typeof(env, args[PARAM1], &handler);
        CHECK_AND_RETURN_RET_LOG(handler == napi_function, ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID),
            "handler is invaild");
    }

    return RegisterCallback(env, jsThis, argc, args, callbackName);
}

napi_value NapiAudioRoutingManager::UnregisterCallback(napi_env env, napi_value jsThis,
    const std::string &callbackName, napi_value callback)
{
    napi_value undefinedResult = nullptr;
    napi_get_undefined(env, &undefinedResult);
    NapiAudioRoutingManager *napiRoutingMgr = nullptr;
    napi_status status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napiRoutingMgr));
    CHECK_AND_RETURN_RET_LOG(status == napi_ok && napiRoutingMgr != nullptr, undefinedResult,
        "Failed to retrieve audio mgr napi instance.");
    CHECK_AND_RETURN_RET_LOG(napiRoutingMgr->audioMngr_ != nullptr, undefinedResult,
        "audio system mgr instance is null.");
    if (!callbackName.compare(DEVICE_CHANGE_CALLBACK_NAME)) {
        UnregisterDeviceChangeCallback(env, callback, napiRoutingMgr);
    } else if (!callbackName.compare(PREFERRED_OUTPUT_DEVICE_CALLBACK_NAME) ||
        !callbackName.compare(PREFER_OUTPUT_DEVICE_CALLBACK_NAME)) {
        UnregisterPreferredOutputDeviceChangeCallback(env, callback, napiRoutingMgr);
    } else if (!callbackName.compare(PREFERRED_INPUT_DEVICE_CALLBACK_NAME)) {
        UnregisterPreferredInputDeviceChangeCallback(env, callback, napiRoutingMgr);
    } else if (!callbackName.compare(AVAILABLE_DEVICE_CHANGE_CALLBACK_NAME)) {
        CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(),
            ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED), "No system permission");
        UnregisterAvailableDeviceChangeCallback(env, callback, napiRoutingMgr);
    } else {
        AUDIO_ERR_LOG("off no such supported");
        NapiAudioError::ThrowError(env, NAPI_ERR_INVALID_PARAM);
    }

    return undefinedResult;
}

void NapiAudioRoutingManager::UnregisterDeviceChangeCallback(napi_env env, napi_value callback,
    NapiAudioRoutingManager *napiRoutingMgr)
{
    if (napiRoutingMgr->deviceChangeCallbackNapi_ != nullptr) {
        std::shared_ptr<NapiAudioManagerCallback> cb =
            std::static_pointer_cast<NapiAudioManagerCallback>(
            napiRoutingMgr->deviceChangeCallbackNapi_);
        if (callback != nullptr) {
            cb->RemoveRoutingManagerDeviceChangeCbRef(env, callback);
        }
        if (callback == nullptr || cb->GetRoutingManagerDeviceChangeCbListSize() == 0) {
            int32_t ret = napiRoutingMgr->audioMngr_->UnsetDeviceChangeCallback(DeviceFlag::ALL_L_D_DEVICES_FLAG);
            CHECK_AND_RETURN_LOG(ret == SUCCESS, "UnsetDeviceChangeCallback Failed");
            napiRoutingMgr->deviceChangeCallbackNapi_.reset();
            napiRoutingMgr->deviceChangeCallbackNapi_ = nullptr;

            cb->RemoveAllRoutingManagerDeviceChangeCb();
        }
    } else {
        AUDIO_ERR_LOG("UnregisterDeviceChangeCallback: deviceChangeCallbackNapi_ is null");
    }
}

void NapiAudioRoutingManager::UnregisterPreferredOutputDeviceChangeCallback(napi_env env, napi_value callback,
    NapiAudioRoutingManager *napiRoutingMgr)
{
    if (napiRoutingMgr->preferredOutputDeviceCallbackNapi_ != nullptr) {
        std::shared_ptr<NapiAudioPreferredOutputDeviceChangeCallback> cb =
            std::static_pointer_cast<NapiAudioPreferredOutputDeviceChangeCallback>(
            napiRoutingMgr->preferredOutputDeviceCallbackNapi_);
        if (callback == nullptr) {
            int32_t ret = napiRoutingMgr->audioRoutingMngr_->UnsetPreferredOutputDeviceChangeCallback();
            CHECK_AND_RETURN_LOG(ret == SUCCESS, "UnsetPreferredOutputDeviceChangeCallback Failed");

            napiRoutingMgr->preferredOutputDeviceCallbackNapi_.reset();
            napiRoutingMgr->preferredOutputDeviceCallbackNapi_ = nullptr;
            cb->RemoveAllCallbacks();
            return;
        }
        cb->RemoveCallbackReference(env, callback);
    } else {
        AUDIO_ERR_LOG("UnregisterPreferredOutputDeviceChangeCallback: preferredOutputDeviceCallbackNapi_ is null");
    }
}

void NapiAudioRoutingManager::UnregisterPreferredInputDeviceChangeCallback(napi_env env, napi_value callback,
    NapiAudioRoutingManager *napiRoutingMgr)
{
    if (napiRoutingMgr->preferredInputDeviceCallbackNapi_ != nullptr) {
        std::shared_ptr<NapiAudioPreferredInputDeviceChangeCallback> cb =
            std::static_pointer_cast<NapiAudioPreferredInputDeviceChangeCallback>(
            napiRoutingMgr->preferredInputDeviceCallbackNapi_);
        if (callback == nullptr) {
            int32_t ret = napiRoutingMgr->audioRoutingMngr_->UnsetPreferredInputDeviceChangeCallback();
            CHECK_AND_RETURN_LOG(ret == SUCCESS, "UnsetPreferredInputDeviceChangeCallback Failed");

            napiRoutingMgr->preferredInputDeviceCallbackNapi_.reset();
            napiRoutingMgr->preferredInputDeviceCallbackNapi_ = nullptr;
            cb->RemoveAllCallbacks();
            return;
        }
        cb->RemoveCallbackReference(env, callback);
    } else {
        AUDIO_ERR_LOG("UnregisterPreferredInputDeviceChangeCallback: preferredInputDeviceCallbackNapi_ is null");
    }
}

void NapiAudioRoutingManager::UnregisterAvailableDeviceChangeCallback(napi_env env, napi_value callback,
    NapiAudioRoutingManager *napiRoutingMgr)
{
    if (napiRoutingMgr->availableDeviceChangeCallbackNapi_ != nullptr) {
        std::shared_ptr<NapiAudioRountingAvailableDeviceChangeCallback> cb =
            std::static_pointer_cast<NapiAudioRountingAvailableDeviceChangeCallback>(
            napiRoutingMgr->availableDeviceChangeCallbackNapi_);
        if (callback == nullptr || cb->GetRoutingAvailbleDeviceChangeCbListSize() == 0) {
            int32_t ret = napiRoutingMgr->audioMngr_->UnsetAvailableDeviceChangeCallback(D_ALL_DEVICES);
            CHECK_AND_RETURN_LOG(ret == SUCCESS, "UnsetAvailableDeviceChangeCallback Failed");

            napiRoutingMgr->availableDeviceChangeCallbackNapi_.reset();
            napiRoutingMgr->availableDeviceChangeCallbackNapi_ = nullptr;
            cb->RemoveAllRoutinAvailbleDeviceChangeCb();
            return;
        }
        cb->RemoveRoutingAvailbleDeviceChangeCbRef(env, callback);
    } else {
        AUDIO_ERR_LOG("UnregisterAvailableDeviceChangeCallback: availableDeviceChangeCallbackNapi_ is null");
    }
}

napi_value NapiAudioRoutingManager::Off(napi_env env, napi_callback_info info)
{
    napi_value undefinedResult = nullptr;
    napi_get_undefined(env, &undefinedResult);

    const size_t minArgCount = ARGS_ONE;
    size_t argCount = ARGS_TWO;
    napi_value args[minArgCount + PARAM1] = {nullptr, nullptr};
    napi_value jsThis = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argCount, args, &jsThis, nullptr);
    if (status != napi_ok || argCount < minArgCount) {
        AUDIO_ERR_LOG("Off fail to napi_get_cb_info/Requires min 1 parameters");
        NapiAudioError::ThrowError(env, NAPI_ERR_INPUT_INVALID);
        return undefinedResult;
    }

    napi_valuetype eventType = napi_undefined;
    if (napi_typeof(env, args[PARAM0], &eventType) != napi_ok || eventType != napi_string) {
        NapiAudioError::ThrowError(env, NAPI_ERR_INPUT_INVALID);
        return undefinedResult;
    }

    napi_valuetype secondArgsType = napi_undefined;
    if (argCount > minArgCount &&
        (napi_typeof(env, args[PARAM1], &secondArgsType) != napi_ok || secondArgsType != napi_function)) {
        NapiAudioError::ThrowError(env, NAPI_ERR_INPUT_INVALID);
        return undefinedResult;
    }
    std::string callbackName = NapiParamUtils::GetStringArgument(env, args[PARAM0]);

    if (argCount == minArgCount) {
        args[PARAM1] = nullptr;
    }
    AUDIO_INFO_LOG("Off callbackName: %{public}s", callbackName.c_str());

    return UnregisterCallback(env, jsThis, callbackName, args[PARAM1]);
}
}  // namespace AudioStandard
}  // namespace OHOS
