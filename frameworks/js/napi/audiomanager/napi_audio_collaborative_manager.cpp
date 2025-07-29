/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "NapiAudioCollaborativeManager"
#endif

#include "napi_audio_collaborative_manager.h"

#include <vector>
#include "napi_audio_error.h"
#include "napi_param_utils.h"
#include "napi_audio_enum.h"
#include "audio_errors.h"
#include "audio_manager_log.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
using namespace HiviewDFX;
static __thread napi_ref g_collaborativeManagerConstructor = nullptr;
NapiAudioCollaborativeManager::NapiAudioCollaborativeManager()
    : audioCollaborativeMngr_(nullptr), env_(nullptr) {}
NapiAudioCollaborativeManager::~NapiAudioCollaborativeManager() = default;

bool NapiAudioCollaborativeManager::CheckContextStatus(std::shared_ptr<AudioCollaborativeManagerAsyncContext> context)
{
    CHECK_AND_RETURN_RET_LOG(context != nullptr, false, "context object is nullptr.");
    if (context->native == nullptr) {
        context->SignError(NAPI_ERR_SYSTEM);
        AUDIO_ERR_LOG("context object state is error.");
        return false;
    }
    return true;
}

bool NapiAudioCollaborativeManager::CheckAudioCollaborativeManagerStatus(NapiAudioCollaborativeManager *napi,
    std::shared_ptr<AudioCollaborativeManagerAsyncContext> context)
{
    CHECK_AND_RETURN_RET_LOG(napi != nullptr, false, "napi object is nullptr.");
    if (napi->audioCollaborativeMngr_ == nullptr) {
        context->SignError(NAPI_ERR_SYSTEM);
        AUDIO_ERR_LOG("audioCollaborativeMngr is nullptr.");
        return false;
    }
    return true;
}

void NapiAudioCollaborativeManager::Destructor(napi_env env, void *nativeObject, void *finalizeHint)
{
    CHECK_AND_RETURN_LOG(nativeObject, "Native object is null");
    auto obj = static_cast<NapiAudioCollaborativeManager *>(nativeObject);
    ObjectRefMap<NapiAudioCollaborativeManager>::DecreaseRef(obj);
    AUDIO_INFO_LOG("Decrease obj count");
}

napi_value NapiAudioCollaborativeManager::Construct(napi_env env, napi_callback_info info)
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
    unique_ptr<NapiAudioCollaborativeManager> napiAudioCollaborativeManager =
        make_unique<NapiAudioCollaborativeManager>();
    CHECK_AND_RETURN_RET_LOG(napiAudioCollaborativeManager != nullptr, result, "No memory");

    napiAudioCollaborativeManager->audioCollaborativeMngr_ = AudioCollaborativeManager::GetInstance();
    napiAudioCollaborativeManager->env_ = env;

    ObjectRefMap<NapiAudioCollaborativeManager>::Insert(napiAudioCollaborativeManager.get());
    status = napi_wrap(env, thisVar, static_cast<void*>(napiAudioCollaborativeManager.get()),
        NapiAudioCollaborativeManager::Destructor, nullptr, nullptr);
    if (status != napi_ok) {
        ObjectRefMap<NapiAudioCollaborativeManager>::Erase(napiAudioCollaborativeManager.get());
        return result;
    }
    napiAudioCollaborativeManager.release();
    return thisVar;
}

NapiAudioCollaborativeManager* NapiAudioCollaborativeManager::GetParamWithSync(const napi_env &env,
    napi_callback_info info, size_t &argc, napi_value *args)
{
    napi_status status;
    NapiAudioCollaborativeManager *napiAudioCollaborativeManager = nullptr;
    napi_value jsThis = nullptr;

    status = napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok && jsThis != nullptr, nullptr,
        "GetParamWithSync fail to napi_get_cb_info");

    status = napi_unwrap(env, jsThis, (void **)&napiAudioCollaborativeManager);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, nullptr, "napi_unwrap failed");
    CHECK_AND_RETURN_RET_LOG(napiAudioCollaborativeManager != nullptr &&
        napiAudioCollaborativeManager->audioCollaborativeMngr_ !=
        nullptr, napiAudioCollaborativeManager, "GetParamWithSync fail to napi_unwrap");
    return napiAudioCollaborativeManager;
}

napi_value NapiAudioCollaborativeManager::CreateCollaborativeManagerWrapper(napi_env env)
{
    napi_status status;
    napi_value result = nullptr;
    napi_value constructor;

    auto HandleNapiError = [env](const char *logMsg, napi_status status, napi_value &result) {
        AUDIO_INFO_LOG("%s, status %{public}d", logMsg, status);
        napi_get_undefined(env, &result);
        return result;
    };
    status = napi_get_reference_value(env, g_collaborativeManagerConstructor, &constructor);
    CHECK_AND_RETURN_RET(status == napi_ok,
        HandleNapiError("napi_get_reference_value fail!", status, result));
    
    status = napi_new_instance(env, constructor, 0, nullptr, &result);
    CHECK_AND_RETURN_RET(status == napi_ok,
        HandleNapiError("napi_new_instance fail!", status, result));

    return result;
}

napi_value NapiAudioCollaborativeManager::Init(napi_env env, napi_value exports)
{
    AUDIO_DEBUG_LOG("Init");

    napi_status status;
    napi_value constructor;
    napi_value result = nullptr;
    const int32_t refCount = 1;
    napi_get_undefined(env, &result);

    napi_property_descriptor audio_collaborative_manager_properties[] = {
        DECLARE_NAPI_FUNCTION("isCollaborativePlaybackSupported", IsCollaborativePlaybackSupported),
        DECLARE_NAPI_FUNCTION("isCollaborativePlaybackEnabledForDevice", IsCollaborativePlaybackEnabledForDevice),
        DECLARE_NAPI_FUNCTION("setCollaborativePlaybackEnabledForDevice", SetCollaborativePlaybackEnabledForDevice),
    };

    status = napi_define_class(env, AUDIO_COLLABORATIVE_MANAGER_NAPI_CLASS_NAME.c_str(), NAPI_AUTO_LENGTH, Construct,
        nullptr,
        sizeof(audio_collaborative_manager_properties) / sizeof(audio_collaborative_manager_properties[PARAM0]),
        audio_collaborative_manager_properties, &constructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_define_class fail");

    status = napi_create_reference(env, constructor, refCount, &g_collaborativeManagerConstructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_create_reference fail");
    status = napi_set_named_property(env, exports, AUDIO_COLLABORATIVE_MANAGER_NAPI_CLASS_NAME.c_str(),
        constructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_set_named_property fail");
    return exports;
}

napi_value NapiAudioCollaborativeManager::IsCollaborativePlaybackSupported(napi_env env, napi_callback_info info)
{
    AUDIO_DEBUG_LOG("in");
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED), "No system permission");

    napi_value result = nullptr;
    size_t argc = PARAM0;
    auto *napiAudioCollaborativeManager = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(argc == PARAM0, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID), "invalid arguments");
    CHECK_AND_RETURN_RET_LOG(napiAudioCollaborativeManager != nullptr, result,
        "napiAudioCollaborativeManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioCollaborativeManager->audioCollaborativeMngr_ != nullptr, result,
        "audioCollaborativeMngr_ is nullptr");
    bool isCollaborativeSupported =
        napiAudioCollaborativeManager->audioCollaborativeMngr_->IsCollaborativePlaybackSupported();
    NapiParamUtils::SetValueBoolean(env, isCollaborativeSupported, result);
    return result;
}

napi_value NapiAudioCollaborativeManager::IsCollaborativePlaybackEnabledForDevice(
    napi_env env, napi_callback_info info)
{
    AUDIO_INFO_LOG("in");
    napi_value result = nullptr;
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED), "No system permission");

    bool isCollaborativeEnabled = false;
    const size_t requireArgc = ARGS_ONE;
    size_t argc = PARAM1;
    napi_value args[ARGS_ONE] = {};
    auto *napiAudioCollaborativeManager = GetParamWithSync(env, info, argc, args);
    CHECK_AND_RETURN_RET_LOG(napiAudioCollaborativeManager != nullptr, result,
        "napiAudioCollaborativeManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioCollaborativeManager->audioCollaborativeMngr_ != nullptr, result,
        "audioCollaborativeMngr is nullptr");

    if (argc == requireArgc) {
        bool argTransFlag = true;
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, args[PARAM0], &valueType);
        CHECK_AND_RETURN_RET_LOG(valueType == napi_object, NapiAudioError::ThrowErrorAndReturn(env,
            NAPI_ERR_INPUT_INVALID,
            "incorrect parameter types: The type of deviceDescriptor must be object"), "invalid valueType");

        std::shared_ptr<AudioDeviceDescriptor> selectedAudioDevice = std::make_shared<AudioDeviceDescriptor>();
        NapiParamUtils::GetAudioDeviceDescriptor(env, selectedAudioDevice, argTransFlag, args[PARAM0]);
        CHECK_AND_RETURN_RET_LOG(argTransFlag == true, NapiAudioError::ThrowErrorAndReturn(env,
            NAPI_ERR_INPUT_INVALID,
            "parameter verification failed: The param of deviceDescriptor must be interface AudioDeviceDescriptor"),
            "invalid parameter");
        if ((selectedAudioDevice->deviceType_ != DEVICE_TYPE_BLUETOOTH_A2DP) ||
            (selectedAudioDevice->connectState_ != CONNECTED)) {
            NapiAudioError::ThrowError(env, NAPI_ERR_INVALID_PARAM,
                "invalid arguments, device is not A2DP or device is connected");
            return NapiParamUtils::GetUndefinedValue(env);
        }
        
        isCollaborativeEnabled = napiAudioCollaborativeManager->audioCollaborativeMngr_
            ->IsCollaborativePlaybackEnabledForDevice(selectedAudioDevice);
    } else {
        NapiAudioError::ThrowError(env, NAPI_ERR_INPUT_INVALID, "invalid arguments");
        return NapiParamUtils::GetUndefinedValue(env);
    }
    NapiParamUtils::SetValueBoolean(env, isCollaborativeEnabled, result);
    return result;
}

napi_value NapiAudioCollaborativeManager::SetCollaborativePlaybackEnabledForDevice(
    napi_env env, napi_callback_info info)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED), "No system permission.");

    auto context = std::make_shared<AudioCollaborativeManagerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("SetCollaborativePlaybackEnabledForDevice failed : no memory");
        NapiAudioError::ThrowError(env, "SetCollaborativePlaybackEnabledForDevice failed : no memory",
            NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc == ARGS_TWO, "mandatory parameters are left unspecified",
            NAPI_ERR_INPUT_INVALID);
        
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[PARAM0], &valueType);
        NAPI_CHECK_ARGS_RETURN_VOID(context, valueType == napi_object,
            "incorrect parameter types: The type of deviceDescriptor must be object", NAPI_ERR_INPUT_INVALID);
        
        bool argTransFlag = true;
        context->status = NapiParamUtils::GetAudioDeviceDescriptor(env, context->deviceDescriptor, argTransFlag,
            argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok,
            "incorrect parameter types: The param of deviceDescriptor must be interface AudioDeviceDescriptor",
            NAPI_ERR_INPUT_INVALID);

        context->status = NapiParamUtils::GetValueBoolean(env, context->collaborativeEnable, argv[PARAM1]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok,
            "incorrect parameter types: The type of enable must be boolean",
            NAPI_ERR_INPUT_INVALID);
    };

    context->GetCbInfo(env, info, inputParser);
    if (context->status != napi_ok) {
        NapiAudioError::ThrowError(env, context->errCode, context->errMessage);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    return UpdateCollaborativeEnabled(env, context);
}

napi_value NapiAudioCollaborativeManager::UpdateCollaborativeEnabled(napi_env env,
    std::shared_ptr<AudioCollaborativeManagerAsyncContext> &context)
{
    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioCollaborativeManager*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioCollaborativeManager = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioCollaborativeManagerStatus(napiAudioCollaborativeManager, context),
            "audio collaborative manager state is error.");
        
        if (!napiAudioCollaborativeManager->audioCollaborativeMngr_->IsCollaborativePlaybackSupported()) {
            context->SignError(NAPI_ERR_UNAVAILABLE_ON_DEVICE);
            return;
        }
        if ((context->deviceDescriptor->deviceType_ != DEVICE_TYPE_BLUETOOTH_A2DP) ||
            (context->deviceDescriptor->connectState_ != CONNECTED)) {
            context->SignError(NAPI_ERR_INVALID_PARAM);
            return;
        }
        context->intValue =
            napiAudioCollaborativeManager->audioCollaborativeMngr_->SetCollaborativePlaybackEnabledForDevice(
                context->deviceDescriptor, context->collaborativeEnable);

        if (context->intValue == ERR_PERMISSION_DENIED) {
            context->SignError(NAPI_ERR_NO_PERMISSION);
        } else if (context->intValue != SUCCESS) {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SetCollaborativeEnabled", executor, complete);
}

}
}