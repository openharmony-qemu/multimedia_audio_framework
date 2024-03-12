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
#include "napi_audio_stream_manager.h"
#include "napi_audio_error.h"
#include "napi_param_utils.h"
#include "napi_audio_enum.h"
#include "audio_errors.h"
#include "audio_log.h"
#include "napi_audio_renderer_state_callback.h"
#include "napi_audio_capturer_state_callback.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
using namespace HiviewDFX;
static __thread napi_ref g_streamMgrConstructor = nullptr;

NapiAudioStreamMgr::NapiAudioStreamMgr()
    : env_(nullptr), audioStreamMngr_(nullptr) {}

NapiAudioStreamMgr::~NapiAudioStreamMgr() = default;

static napi_value ThrowErrorAndReturn(napi_env env, int32_t errCode)
{
    NapiAudioError::ThrowError(env, errCode);
    return nullptr;
}

void NapiAudioStreamMgr::Destructor(napi_env env, void *nativeObject, void *finalizeHint)
{
    if (nativeObject != nullptr) {
        auto obj = static_cast<NapiAudioStreamMgr *>(nativeObject);
        ObjectRefMap<NapiAudioStreamMgr>::DecreaseRef(obj);
    }
    AUDIO_INFO_LOG("Destructor is successful");
}

napi_value NapiAudioStreamMgr::Construct(napi_env env, napi_callback_info info)
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
    unique_ptr<NapiAudioStreamMgr> napiStreamMgr = make_unique<NapiAudioStreamMgr>();
    CHECK_AND_RETURN_RET_LOG(napiStreamMgr != nullptr, result, "No memory");

    napiStreamMgr->env_ = env;
    napiStreamMgr->audioStreamMngr_ = AudioStreamManager::GetInstance();
    napiStreamMgr->cachedClientId_ = getpid();
    ObjectRefMap<NapiAudioStreamMgr>::Insert(napiStreamMgr.get());

    status = napi_wrap(env, thisVar, static_cast<void*>(napiStreamMgr.get()),
        NapiAudioStreamMgr::Destructor, nullptr, nullptr);
    if (status != napi_ok) {
        ObjectRefMap<NapiAudioStreamMgr>::Erase(napiStreamMgr.get());
        return result;
    }
    napiStreamMgr.release();
    return thisVar;
}

napi_value NapiAudioStreamMgr::Init(napi_env env, napi_value exports)
{
    napi_status status;
    napi_value constructor;
    napi_value result = nullptr;
    const int32_t refCount = ARGS_ONE;
    napi_get_undefined(env, &result);

    napi_property_descriptor audio_stream_mgr_properties[] = {
        DECLARE_NAPI_FUNCTION("on", On),
        DECLARE_NAPI_FUNCTION("off", Off),
        DECLARE_NAPI_FUNCTION("getCurrentAudioRendererInfoArray", GetCurrentAudioRendererInfos),
        DECLARE_NAPI_FUNCTION("getCurrentAudioRendererInfoArraySync", GetCurrentAudioRendererInfosSync),
        DECLARE_NAPI_FUNCTION("getCurrentAudioCapturerInfoArray", GetCurrentAudioCapturerInfos),
        DECLARE_NAPI_FUNCTION("getCurrentAudioCapturerInfoArraySync", GetCurrentAudioCapturerInfosSync),
        DECLARE_NAPI_FUNCTION("isAudioRendererLowLatencySupported", IsAudioRendererLowLatencySupported),
        DECLARE_NAPI_FUNCTION("isActive", IsStreamActive),
        DECLARE_NAPI_FUNCTION("isActiveSync", IsStreamActiveSync),
        DECLARE_NAPI_FUNCTION("getAudioEffectInfoArray", GetEffectInfoArray),
        DECLARE_NAPI_FUNCTION("getAudioEffectInfoArraySync", GetEffectInfoArraySync),
        DECLARE_NAPI_FUNCTION("getHardwareOutputSamplingRate", GetHardwareOutputSamplingRate),
    };

    status = napi_define_class(env, AUDIO_STREAM_MGR_NAPI_CLASS_NAME.c_str(), NAPI_AUTO_LENGTH, Construct, nullptr,
        sizeof(audio_stream_mgr_properties) / sizeof(audio_stream_mgr_properties[PARAM0]),
        audio_stream_mgr_properties, &constructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_define_class fail");

    status = napi_create_reference(env, constructor, refCount, &g_streamMgrConstructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_create_reference fail");
    status = napi_set_named_property(env, exports, AUDIO_STREAM_MGR_NAPI_CLASS_NAME.c_str(), constructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_set_named_property fail");
    return exports;
}

napi_value NapiAudioStreamMgr::CreateStreamManagerWrapper(napi_env env)
{
    napi_status status;
    napi_value result = nullptr;
    napi_value constructor;

    status = napi_get_reference_value(env, g_streamMgrConstructor, &constructor);
    if (status != napi_ok) {
        AUDIO_ERR_LOG("Failed in CreateStreamManagerWrapper, %{public}d", status);
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

bool NapiAudioStreamMgr::CheckContextStatus(std::shared_ptr<AudioStreamMgrAsyncContext> context)
{
    CHECK_AND_RETURN_RET_LOG(context != nullptr, false, "context object is nullptr.");
    if (context->native == nullptr) {
        context->SignError(NAPI_ERR_SYSTEM);
        return false;
    }
    return true;
}

bool NapiAudioStreamMgr::CheckAudioStreamManagerStatus(NapiAudioStreamMgr *napi,
    std::shared_ptr<AudioStreamMgrAsyncContext> context)
{
    CHECK_AND_RETURN_RET_LOG(napi != nullptr, false, "napi object is nullptr.");
    if (napi->audioStreamMngr_ == nullptr) {
        context->SignError(NAPI_ERR_SYSTEM);
        return false;
    }
    return true;
}

NapiAudioStreamMgr* NapiAudioStreamMgr::GetParamWithSync(const napi_env &env, napi_callback_info info,
    size_t &argc, napi_value *args)
{
    napi_status status;
    NapiAudioStreamMgr *napiStreamMgr = nullptr;
    napi_value jsThis = nullptr;
    status = napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok && jsThis != nullptr, nullptr,
        "GetParamWithSync fail to napi_get_cb_info");

    status = napi_unwrap(env, jsThis, (void **)&napiStreamMgr);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, nullptr, "napi_unwrap failed");
    CHECK_AND_RETURN_RET_LOG(napiStreamMgr != nullptr && napiStreamMgr->audioStreamMngr_  !=
        nullptr, napiStreamMgr, "GetParamWithSync fail to napi_unwrap");
    return napiStreamMgr;
}

napi_value NapiAudioStreamMgr::GetCurrentAudioRendererInfos(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioStreamMgrAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("GetCurrentAudioRendererInfos failed : no memory");
        NapiAudioError::ThrowError(env, "GetCurrentAudioRendererInfos failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioStreamMgr*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiStreamMgr = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioStreamManagerStatus(napiStreamMgr, context),
            "context object state is error.");
        context->intValue = napiStreamMgr->audioStreamMngr_->GetCurrentRendererChangeInfos(
            context->audioRendererChangeInfos);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->intValue == SUCCESS,
            "GetCurrentAudioRendererInfos failed", NAPI_ERR_SYSTEM);
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetRendererChangeInfos(env, context->audioRendererChangeInfos, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "GetCurrentAudioRendererInfos", executor, complete);
}

napi_value NapiAudioStreamMgr::GetCurrentAudioRendererInfosSync(napi_env env, napi_callback_info info)
{
    AUDIO_INFO_LOG("GetCurrentAudioRendererInfosSync");
    napi_value result = nullptr;
    size_t argc = PARAM0;
    auto *napiStreamMgr = GetParamWithSync(env, info, argc, nullptr);
    if (argc > 0) {
        NapiAudioError::ThrowError(env, NAPI_ERROR_INVALID_PARAM);
    }
    CHECK_AND_RETURN_RET_LOG(napiStreamMgr!= nullptr, result, "napiStreamMgr is nullptr");

    vector<std::unique_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    int32_t ret = napiStreamMgr->audioStreamMngr_->GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, result, "GetCurrentRendererChangeInfos failure!");

    NapiParamUtils::SetRendererChangeInfos(env, audioRendererChangeInfos, result);

    return result;
}

napi_value NapiAudioStreamMgr::GetCurrentAudioCapturerInfos(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioStreamMgrAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("GetCurrentAudioCapturerInfos failed : no memory");
        NapiAudioError::ThrowError(env, "GetCurrentAudioCapturerInfos failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioStreamMgr*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiStreamMgr = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioStreamManagerStatus(napiStreamMgr, context),
            "context object state is error.");
        napiStreamMgr->audioStreamMngr_->GetCurrentCapturerChangeInfos(
            context->audioCapturerChangeInfos);
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetCapturerChangeInfos(env, context->audioCapturerChangeInfos, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "GetCurrentAudioCapturerInfos", executor, complete);
}

napi_value NapiAudioStreamMgr::GetCurrentAudioCapturerInfosSync(napi_env env, napi_callback_info info)
{
    AUDIO_INFO_LOG("GetCurrentAudioCapturerInfosSync");
    napi_value result = nullptr;
    size_t argc = PARAM0;
    auto *napiStreamMgr = GetParamWithSync(env, info, argc, nullptr);
    if (argc > 0) {
        NapiAudioError::ThrowError(env, NAPI_ERROR_INVALID_PARAM);
    }
    CHECK_AND_RETURN_RET_LOG(napiStreamMgr!= nullptr, result, "napiStreamMgr is nullptr");

    vector<std::unique_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    int32_t ret = napiStreamMgr->audioStreamMngr_->GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
    if (ret != AUDIO_OK) {
        AUDIO_ERR_LOG("GetCurrentCapturerChangeInfos failure!");
        return result;
    }
    NapiParamUtils::SetCapturerChangeInfos(env, audioCapturerChangeInfos, result);

    return result;
}

napi_value NapiAudioStreamMgr::IsAudioRendererLowLatencySupported(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioStreamMgrAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("IsAudioRendererLowLatencySupported failed : no memory");
        NapiAudioError::ThrowError(env, "IsAudioRendererLowLatencySupported failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetStreamInfo(env, &context->audioStreamInfo, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "getstreaminfo failed",
            NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioStreamMgr*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiStreamMgr = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioStreamManagerStatus(napiStreamMgr, context),
            "context object state is error.");
        context->isLowLatencySupported = napiStreamMgr->audioStreamMngr_->IsAudioRendererLowLatencySupported(
            context->audioStreamInfo);
        context->isTrue = context->isLowLatencySupported;
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueBoolean(env, context->isTrue, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "IsAudioRendererLowLatencySupported", executor, complete);
}

napi_value NapiAudioStreamMgr::IsStreamActive(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioStreamMgrAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("IsStreamActive failed : no memory");
        NapiAudioError::ThrowError(env, "IsStreamActive failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->volType, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "getvoltype failed",
            NAPI_ERR_INVALID_PARAM);
        if (!NapiAudioEnum::IsLegalInputArgumentVolType(context->volType)) {
            context->SignError(context->errCode ==
                NAPI_ERR_INVALID_PARAM? NAPI_ERR_INVALID_PARAM : NAPI_ERR_UNSUPPORTED);
        }
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioStreamMgr*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiStreamMgr = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioStreamManagerStatus(napiStreamMgr, context),
            "context object state is error.");
        context->isActive = napiStreamMgr->audioStreamMngr_->IsStreamActive(
            NapiAudioEnum::GetNativeAudioVolumeType(context->volType));
        context->isTrue = context->isActive;
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueBoolean(env, context->isTrue, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "IsStreamActive", executor, complete);
}

napi_value NapiAudioStreamMgr::IsStreamActiveSync(napi_env env, napi_callback_info info)
{
    AUDIO_INFO_LOG("IsStreamActiveSync");
    napi_value result = nullptr;
    size_t argc = ARGS_ONE;
    napi_value args[ARGS_ONE] = {};
    auto *napiStreamMgr = GetParamWithSync(env, info, argc, args);
    CHECK_AND_RETURN_RET_LOG(argc >= ARGS_ONE, ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID), "invalid arguments");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, args[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_number, ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID),
        "invalid valueType");

    int32_t volType;
    NapiParamUtils::GetValueInt32(env, volType, args[PARAM0]);
    CHECK_AND_RETURN_RET_LOG(NapiAudioEnum::IsLegalInputArgumentVolType(volType), ThrowErrorAndReturn(env,
        NAPI_ERR_INVALID_PARAM), "get volType failed");

    CHECK_AND_RETURN_RET_LOG(napiStreamMgr != nullptr, result, "napiStreamMgr is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiStreamMgr->audioStreamMngr_ != nullptr, result,
        "audioStreamMngr_ is nullptr");
    bool isActive = napiStreamMgr->audioStreamMngr_->
        IsStreamActive(NapiAudioEnum::GetNativeAudioVolumeType(volType));
    NapiParamUtils::SetValueBoolean(env, isActive, result);
    return result;
}

napi_value NapiAudioStreamMgr::GetEffectInfoArray(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioStreamMgrAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("GetEffectInfoArray failed : no memory");
        NapiAudioError::ThrowError(env, "GetEffectInfoArray failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments", NAPI_ERR_INPUT_INVALID);
        context->status = NapiParamUtils::GetValueInt32(env, context->streamUsage, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "getstreamUsage failed",
            NAPI_ERR_INPUT_INVALID);
        if (!NapiAudioEnum::IsLegalInputArgumentStreamUsage(context->streamUsage)) {
            context->SignError(NAPI_ERR_INVALID_PARAM);
        }
    };
    context->GetCbInfo(env, info, inputParser);

    if ((context->status != napi_ok) && (context->errCode == NAPI_ERR_INPUT_INVALID)) {
        NapiAudioError::ThrowError(env, context->errCode);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioStreamMgr*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiStreamMgr = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioStreamManagerStatus(napiStreamMgr, context),
            "context object state is error.");
        StreamUsage streamUsage = static_cast<StreamUsage>(context->streamUsage);
        context->intValue = napiStreamMgr->audioStreamMngr_->GetEffectInfoArray(
            context->audioSceneEffectInfo, streamUsage);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->intValue == SUCCESS, "GetEffectInfoArray failed",
            NAPI_ERR_SYSTEM);
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetEffectInfo(env, context->audioSceneEffectInfo, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "GetEffectInfoArray", executor, complete);
}

napi_value NapiAudioStreamMgr::GetEffectInfoArraySync(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = ARGS_ONE;
    napi_value args[ARGS_ONE] = {};
    auto *napiStreamMgr = GetParamWithSync(env, info, argc, args);
    CHECK_AND_RETURN_RET_LOG(argc >= ARGS_ONE, ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID), "invalid arguments");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, args[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_number, ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID),
        "invalid valueType");

    int32_t streamUsage;
    NapiParamUtils::GetValueInt32(env, streamUsage, args[PARAM0]);
    CHECK_AND_RETURN_RET_LOG(NapiAudioEnum::IsLegalInputArgumentStreamUsage(streamUsage), ThrowErrorAndReturn(env,
        NAPI_ERR_INVALID_PARAM), "get streamUsage failed");

    CHECK_AND_RETURN_RET_LOG(napiStreamMgr != nullptr, result, "napiStreamMgr is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiStreamMgr->audioStreamMngr_ != nullptr, result,
        "audioStreamMngr_ is nullptr");
    AudioSceneEffectInfo audioSceneEffectInfo;
    int32_t ret = napiStreamMgr->audioStreamMngr_->GetEffectInfoArray(audioSceneEffectInfo,
        static_cast<StreamUsage>(streamUsage));
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, result, "GetEffectInfoArray failure!");
    NapiParamUtils::SetEffectInfo(env, audioSceneEffectInfo, result);
    return result;
}

napi_value NapiAudioStreamMgr::GetHardwareOutputSamplingRate(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    sptr<AudioDeviceDescriptor> deviceDescriptor = nullptr;
    size_t argc = ARGS_ONE;
    napi_value args[ARGS_ONE] = {};
    auto *napiStreamMgr = GetParamWithSync(env, info, argc, args);
    CHECK_AND_RETURN_RET_LOG(napiStreamMgr != nullptr, result, "napiStreamMgr is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiStreamMgr->audioStreamMngr_ != nullptr, result,
        "audioStreamMngr_ is nullptr");

    if (argc < ARGS_ONE) {
        int32_t rate = napiStreamMgr->audioStreamMngr_->GetHardwareOutputSamplingRate(deviceDescriptor);
        NapiParamUtils::SetValueInt32(env, rate, result);
        return result;
    }

    deviceDescriptor = new (std::nothrow) AudioDeviceDescriptor();
    CHECK_AND_RETURN_RET_LOG(deviceDescriptor != nullptr, result, "AudioDeviceDescriptor alloc failed!");

    bool argTransFlag = false;
    NapiParamUtils::GetAudioDeviceDescriptor(env, deviceDescriptor, argTransFlag, args[PARAM0]);
    CHECK_AND_RETURN_RET_LOG(argTransFlag && NapiAudioEnum::IsLegalOutputDeviceType(deviceDescriptor->deviceType_) &&
        (deviceDescriptor->deviceRole_ == DeviceRole::OUTPUT_DEVICE),
        ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM), "invalid deviceDescriptor");

    int32_t rate = napiStreamMgr->audioStreamMngr_->GetHardwareOutputSamplingRate(deviceDescriptor);
    NapiParamUtils::SetValueInt32(env, rate, result);
    return result;
}

void NapiAudioStreamMgr::RegisterCallback(napi_env env, napi_value jsThis,
    napi_value *args, const std::string &cbName)
{
    NapiAudioStreamMgr *napiStreamMgr = nullptr;
    napi_status status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napiStreamMgr));
    CHECK_AND_RETURN_LOG((status == napi_ok) && (napiStreamMgr != nullptr) &&
        (napiStreamMgr->audioStreamMngr_ != nullptr), "Failed to retrieve stream mgr napi instance.");

    if (!cbName.compare(RENDERERCHANGE_CALLBACK_NAME)) {
        RegisterRendererStateChangeCallback(env, args, cbName, napiStreamMgr);
    } else if (!cbName.compare(CAPTURERCHANGE_CALLBACK_NAME)) {
        RegisterCapturerStateChangeCallback(env, args, cbName, napiStreamMgr);
    } else {
        AUDIO_ERR_LOG("NapiAudioStreamMgr::No such callback supported");
        NapiAudioError::ThrowError(env, NAPI_ERR_INVALID_PARAM);
    }
}

void NapiAudioStreamMgr::RegisterRendererStateChangeCallback(napi_env env, napi_value *args,
    const std::string &cbName, NapiAudioStreamMgr *napiStreamMgr)
{
    if (!napiStreamMgr->rendererStateChangeCallbackNapi_) {
        napiStreamMgr->rendererStateChangeCallbackNapi_ = std::make_shared<NapiAudioRendererStateCallback>(env);
        CHECK_AND_RETURN_LOG(napiStreamMgr->rendererStateChangeCallbackNapi_ != nullptr,
            "NapiAudioStreamMgr: Memory Allocation Failed !!");

        int32_t ret =
            napiStreamMgr->audioStreamMngr_->RegisterAudioRendererEventListener(napiStreamMgr->cachedClientId_,
            napiStreamMgr->rendererStateChangeCallbackNapi_);
        CHECK_AND_RETURN_LOG(ret == SUCCESS,
            "NapiAudioStreamMgr: Registering of Renderer State Change Callback Failed");
    }

    std::shared_ptr<NapiAudioRendererStateCallback> cb =
    std::static_pointer_cast<NapiAudioRendererStateCallback>(napiStreamMgr->rendererStateChangeCallbackNapi_);
    cb->SaveCallbackReference(args[PARAM1]);

    AUDIO_INFO_LOG("OnRendererStateChangeCallback is successful");
}

void NapiAudioStreamMgr::RegisterCapturerStateChangeCallback(napi_env env, napi_value *args,
    const std::string &cbName, NapiAudioStreamMgr *napiStreamMgr)
{
    if (!napiStreamMgr->capturerStateChangeCallbackNapi_) {
        napiStreamMgr->capturerStateChangeCallbackNapi_ = std::make_shared<NapiAudioCapturerStateCallback>(env);
        CHECK_AND_RETURN_LOG(napiStreamMgr->capturerStateChangeCallbackNapi_ != nullptr,
            "Memory Allocation Failed !!");

        int32_t ret =
            napiStreamMgr->audioStreamMngr_->RegisterAudioCapturerEventListener(napiStreamMgr->cachedClientId_,
            napiStreamMgr->capturerStateChangeCallbackNapi_);
        CHECK_AND_RETURN_LOG(ret == SUCCESS,
            "Registering of Capturer State Change Callback Failed");
    }
    std::lock_guard<std::mutex> lock(napiStreamMgr->capturerStateChangeCallbackNapi_->cbMutex_);

    std::shared_ptr<NapiAudioCapturerStateCallback> cb =
        std::static_pointer_cast<NapiAudioCapturerStateCallback>(napiStreamMgr->capturerStateChangeCallbackNapi_);
    cb->SaveCallbackReference(args[PARAM1]);

    AUDIO_INFO_LOG("OnCapturerStateChangeCallback is successful");
}

napi_value NapiAudioStreamMgr::On(napi_env env, napi_callback_info info)
{
    const size_t requireArgc = ARGS_TWO;
    size_t argc = ARGS_THREE;

    napi_value undefinedResult = nullptr;
    napi_get_undefined(env, &undefinedResult);

    napi_value args[requireArgc + PARAM1] = {nullptr, nullptr, nullptr};
    napi_value jsThis = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok && argc == requireArgc,
        ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID), "status or arguments error");

    napi_valuetype eventType = napi_undefined;
    napi_typeof(env, args[PARAM0], &eventType);
    CHECK_AND_RETURN_RET_LOG(eventType == napi_string,
        ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID), "eventType error");

    std::string callbackName = NapiParamUtils::GetStringArgument(env, args[PARAM0]);
    AUDIO_DEBUG_LOG("AudioStreamMgrNapi: On callbackName: %{public}s", callbackName.c_str());

    napi_valuetype handler = napi_undefined;

    napi_typeof(env, args[PARAM1], &handler);
    CHECK_AND_RETURN_RET_LOG(handler == napi_function,
        ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID), "eventType error");

    RegisterCallback(env, jsThis, args, callbackName);
    return undefinedResult;
}

void NapiAudioStreamMgr::UnregisterCallback(napi_env env, napi_value jsThis, const std::string &cbName)
{
    AUDIO_INFO_LOG("UnregisterCallback");
    NapiAudioStreamMgr *napiStreamMgr = nullptr;
    napi_status status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napiStreamMgr));
    CHECK_AND_RETURN_LOG((status == napi_ok) && (napiStreamMgr != nullptr) &&
        (napiStreamMgr->audioStreamMngr_ != nullptr), "Failed to retrieve stream mgr napi instance.");

    if (!cbName.compare(RENDERERCHANGE_CALLBACK_NAME)) {
        int32_t ret = napiStreamMgr->audioStreamMngr_->
            UnregisterAudioRendererEventListener(napiStreamMgr->cachedClientId_);
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "UnRegistering of Renderer State Change Callback Failed");

        if (napiStreamMgr->rendererStateChangeCallbackNapi_ != nullptr) {
            std::shared_ptr<NapiAudioRendererStateCallback> cb =
                std::static_pointer_cast<NapiAudioRendererStateCallback>(napiStreamMgr->
                    rendererStateChangeCallbackNapi_);
            cb->RemoveCallbackReference();
            napiStreamMgr->rendererStateChangeCallbackNapi_.reset();
        }
        AUDIO_INFO_LOG("UnRegistering of renderer State Change Callback successful");
    } else if (!cbName.compare(CAPTURERCHANGE_CALLBACK_NAME)) {
        int32_t ret = napiStreamMgr->audioStreamMngr_->
            UnregisterAudioCapturerEventListener(napiStreamMgr->cachedClientId_);
        if (ret) {
            AUDIO_ERR_LOG("UnRegistering of capturer State Change Callback Failed");
            return;
        }
        if (napiStreamMgr->capturerStateChangeCallbackNapi_ != nullptr) {
            std::lock_guard<std::mutex> lock(napiStreamMgr->capturerStateChangeCallbackNapi_->cbMutex_);
            napiStreamMgr->capturerStateChangeCallbackNapi_.reset();
            napiStreamMgr->capturerStateChangeCallbackNapi_ = nullptr;
        }
        AUDIO_INFO_LOG("UnRegistering of capturer State Change Callback successful");
    } else {
        AUDIO_ERR_LOG("No such callback supported");
        NapiAudioError::ThrowError(env, NAPI_ERR_INVALID_PARAM);
    }
}

napi_value NapiAudioStreamMgr::Off(napi_env env, napi_callback_info info)
{
    const size_t requireArgc = ARGS_ONE;
    size_t argc = ARGS_ONE;

    napi_value undefinedResult = nullptr;
    napi_get_undefined(env, &undefinedResult);

    napi_value args[requireArgc] = {nullptr};
    napi_value jsThis = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok && argc >= requireArgc,
        ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID), "status or arguments error");

    napi_valuetype eventType = napi_undefined;
    napi_typeof(env, args[PARAM0], &eventType);
    CHECK_AND_RETURN_RET_LOG(eventType == napi_string,
        ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID), "eventType error");

    std::string callbackName = NapiParamUtils::GetStringArgument(env, args[0]);
    AUDIO_DEBUG_LOG("NapiAudioStreamMgr: Off callbackName: %{public}s", callbackName.c_str());

    UnregisterCallback(env, jsThis, callbackName);
    return undefinedResult;
}
}  // namespace AudioStandard
}  // namespace OHOS
