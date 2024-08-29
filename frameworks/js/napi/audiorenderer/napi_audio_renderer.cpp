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
#define LOG_TAG "NapiAudioRenderer"
#endif

#include "napi_audio_renderer.h"
#include "audio_utils.h"
#if defined(ANDROID_PLATFORM) || defined(IOS_PLATFORM)
#include "errors.h"
#else
#ifdef FEATURE_HIVIEW_ENABLE
#include "xpower_event_js.h"
#endif
#endif
#include "napi_param_utils.h"
#include "napi_audio_error.h"
#include "napi_audio_enum.h"
#include "napi_audio_renderer_callback.h"
#include "napi_renderer_position_callback.h"
#include "napi_renderer_data_request_callback.h"
#include "napi_renderer_period_position_callback.h"
#include "napi_audio_renderer_write_data_callback.h"
#include "napi_audio_renderer_device_change_callback.h"
#include "napi_audio_renderer_policy_service_died_callback.h"

namespace OHOS {
namespace AudioStandard {
static __thread napi_ref g_rendererConstructor = nullptr;
mutex NapiAudioRenderer::createMutex_;
int32_t NapiAudioRenderer::isConstructSuccess_ = SUCCESS;
std::unique_ptr<AudioRendererOptions> NapiAudioRenderer::sRendererOptions_ = nullptr;

NapiAudioRenderer::NapiAudioRenderer()
    : audioRenderer_(nullptr), contentType_(CONTENT_TYPE_MUSIC), streamUsage_(STREAM_USAGE_MEDIA), env_(nullptr) {}

void NapiAudioRenderer::Destructor(napi_env env, void *nativeObject, void *finalizeHint)
{
    if (nativeObject != nullptr) {
        auto obj = static_cast<NapiAudioRenderer *>(nativeObject);
        ObjectRefMap<NapiAudioRenderer>::DecreaseRef(obj);
    }
    AUDIO_INFO_LOG("Destructor is successful");
}

napi_status NapiAudioRenderer::InitNapiAudioRenderer(napi_env env, napi_value &constructor)
{
    napi_property_descriptor audio_renderer_properties[] = {
        DECLARE_NAPI_FUNCTION("setRenderRate", SetRenderRate),
        DECLARE_NAPI_FUNCTION("getRenderRate", GetRenderRate),
        DECLARE_NAPI_FUNCTION("getRenderRateSync", GetRenderRateSync),
        DECLARE_NAPI_FUNCTION("setRendererSamplingRate", SetRendererSamplingRate),
        DECLARE_NAPI_FUNCTION("getRendererSamplingRate", GetRendererSamplingRate),
        DECLARE_NAPI_FUNCTION("start", Start),
        DECLARE_NAPI_FUNCTION("write", Write),
        DECLARE_NAPI_FUNCTION("getAudioTime", GetAudioTime),
        DECLARE_NAPI_FUNCTION("getAudioTimeSync", GetAudioTimeSync),
        DECLARE_NAPI_FUNCTION("drain", Drain),
        DECLARE_NAPI_FUNCTION("flush", Flush),
        DECLARE_NAPI_FUNCTION("pause", Pause),
        DECLARE_NAPI_FUNCTION("stop", Stop),
        DECLARE_NAPI_FUNCTION("release", Release),
        DECLARE_NAPI_FUNCTION("getBufferSize", GetBufferSize),
        DECLARE_NAPI_FUNCTION("getBufferSizeSync", GetBufferSizeSync),
        DECLARE_NAPI_FUNCTION("getAudioStreamId", GetAudioStreamId),
        DECLARE_NAPI_FUNCTION("getAudioStreamIdSync", GetAudioStreamIdSync),
        DECLARE_NAPI_FUNCTION("setVolume", SetVolume),
        DECLARE_NAPI_FUNCTION("getVolume", GetVolume),
        DECLARE_NAPI_FUNCTION("getRendererInfo", GetRendererInfo),
        DECLARE_NAPI_FUNCTION("getRendererInfoSync", GetRendererInfoSync),
        DECLARE_NAPI_FUNCTION("getStreamInfo", GetStreamInfo),
        DECLARE_NAPI_FUNCTION("getStreamInfoSync", GetStreamInfoSync),
        DECLARE_NAPI_FUNCTION("setInterruptMode", SetInterruptMode),
        DECLARE_NAPI_FUNCTION("setInterruptModeSync", SetInterruptModeSync),
        DECLARE_NAPI_FUNCTION("getMinStreamVolume", GetMinStreamVolume),
        DECLARE_NAPI_FUNCTION("getMinStreamVolumeSync", GetMinStreamVolumeSync),
        DECLARE_NAPI_FUNCTION("getMaxStreamVolume", GetMaxStreamVolume),
        DECLARE_NAPI_FUNCTION("getMaxStreamVolumeSync", GetMaxStreamVolumeSync),
        DECLARE_NAPI_FUNCTION("getCurrentOutputDevices", GetCurrentOutputDevices),
        DECLARE_NAPI_FUNCTION("getCurrentOutputDevicesSync", GetCurrentOutputDevicesSync),
        DECLARE_NAPI_FUNCTION("getUnderflowCount", GetUnderflowCount),
        DECLARE_NAPI_FUNCTION("getUnderflowCountSync", GetUnderflowCountSync),
        DECLARE_NAPI_FUNCTION("getAudioEffectMode", GetAudioEffectMode),
        DECLARE_NAPI_FUNCTION("setAudioEffectMode", SetAudioEffectMode),
        DECLARE_NAPI_FUNCTION("setChannelBlendMode", SetChannelBlendMode),
        DECLARE_NAPI_FUNCTION("setVolumeWithRamp", SetVolumeWithRamp),
        DECLARE_NAPI_FUNCTION("setSpeed", SetSpeed),
        DECLARE_NAPI_FUNCTION("getSpeed", GetSpeed),
        DECLARE_NAPI_GETTER("state", GetState),
        DECLARE_NAPI_FUNCTION("on", On),
        DECLARE_NAPI_FUNCTION("off", Off),
        DECLARE_NAPI_FUNCTION("setSilentModeAndMixWithOthers", SetSilentModeAndMixWithOthers),
        DECLARE_NAPI_FUNCTION("getSilentModeAndMixWithOthers", GetSilentModeAndMixWithOthers),
        DECLARE_NAPI_FUNCTION("setDefaultOutputDevice", SetDefaultOutputDevice),
    };

    napi_status status = napi_define_class(env, NAPI_AUDIO_RENDERER_CLASS_NAME.c_str(),
        NAPI_AUTO_LENGTH, Construct, nullptr,
        sizeof(audio_renderer_properties) / sizeof(audio_renderer_properties[PARAM0]),
        audio_renderer_properties, &constructor);
    return status;
}

napi_value NapiAudioRenderer::Init(napi_env env, napi_value exports)
{
    napi_status status;
    napi_value constructor;
    napi_value result = nullptr;
    const int32_t refCount = 1;
    napi_get_undefined(env, &result);

    napi_property_descriptor static_prop[] = {
        DECLARE_NAPI_STATIC_FUNCTION("createAudioRenderer", CreateAudioRenderer),
        DECLARE_NAPI_STATIC_FUNCTION("createAudioRendererSync", CreateAudioRendererSync),
    };

    status = InitNapiAudioRenderer(env, constructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "InitNapiAudioRenderer fail");

    status = napi_create_reference(env, constructor, refCount, &g_rendererConstructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_create_reference fail");
    status = napi_set_named_property(env, exports, NAPI_AUDIO_RENDERER_CLASS_NAME.c_str(), constructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_set_named_property fail");
    status = napi_define_properties(env, exports,
        sizeof(static_prop) / sizeof(static_prop[PARAM0]), static_prop);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_define_properties fail");
    return exports;
}

void NapiAudioRenderer::CreateRendererFailed()
{
    NapiAudioRenderer::isConstructSuccess_ = NAPI_ERR_SYSTEM;
    if (AudioRenderer::CheckMaxRendererInstances() == ERR_OVERFLOW) {
        NapiAudioRenderer::isConstructSuccess_ = NAPI_ERR_STREAM_LIMIT;
    }
    AUDIO_ERR_LOG("Renderer Create failed %{public}d", isConstructSuccess_);
}

unique_ptr<NapiAudioRenderer> NapiAudioRenderer::CreateAudioRendererNativeObject(napi_env env)
{
    unique_ptr<NapiAudioRenderer> rendererNapi = make_unique<NapiAudioRenderer>();
    CHECK_AND_RETURN_RET_LOG(rendererNapi != nullptr, nullptr, "No memory");

    rendererNapi->env_ = env;
    rendererNapi->contentType_ = sRendererOptions_->rendererInfo.contentType;
    rendererNapi->streamUsage_ = sRendererOptions_->rendererInfo.streamUsage;

    AudioRendererOptions rendererOptions = *sRendererOptions_;
    std::string cacheDir = "";
    /* NapiAudioRenderer not support other rendererFlags, only support flag 0 */
    if (rendererOptions.rendererInfo.rendererFlags != 0) {
        rendererOptions.rendererInfo.rendererFlags = 0;
    }
    rendererNapi->audioRenderer_ = AudioRenderer::Create(cacheDir, rendererOptions);

    if (rendererNapi->audioRenderer_ == nullptr) {
        CreateRendererFailed();
        rendererNapi.release();
        return nullptr;
    }

    if (rendererNapi->audioRenderer_ != nullptr && rendererNapi->callbackNapi_ == nullptr) {
        rendererNapi->callbackNapi_ = std::make_shared<NapiAudioRendererCallback>(env);
        CHECK_AND_RETURN_RET_LOG(rendererNapi->callbackNapi_ != nullptr, nullptr, "No memory");
        int32_t ret = rendererNapi->audioRenderer_->SetRendererCallback(rendererNapi->callbackNapi_);
        CHECK_AND_RETURN_RET_LOG(!ret, rendererNapi, "Construct SetRendererCallback failed");
    }
    ObjectRefMap<NapiAudioRenderer>::Insert(rendererNapi.get());
    return rendererNapi;
}

napi_value NapiAudioRenderer::Construct(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    size_t argCount = ARGS_TWO;
    napi_value thisVar = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argCount, nullptr, &thisVar, nullptr);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "failed to napi_get_cb_info");

    unique_ptr<NapiAudioRenderer> rendererNapi = CreateAudioRendererNativeObject(env);
    CHECK_AND_RETURN_RET_LOG(rendererNapi != nullptr, result, "failed to CreateAudioRendererNativeObject");

    status = napi_wrap(env, thisVar, static_cast<void*>(rendererNapi.get()),
        NapiAudioRenderer::Destructor, nullptr, nullptr);
    if (status != napi_ok) {
        ObjectRefMap<NapiAudioRenderer>::Erase(rendererNapi.get());
        return result;
    }
    rendererNapi.release();
    return thisVar;
}

bool NapiAudioRenderer::CheckContextStatus(std::shared_ptr<AudioRendererAsyncContext> context)
{
    CHECK_AND_RETURN_RET_LOG(context != nullptr, false, "context object is nullptr.");
    if (context->native == nullptr) {
        context->SignError(NAPI_ERR_SYSTEM);
        return false;
    }
    return true;
}

bool NapiAudioRenderer::CheckAudioRendererStatus(NapiAudioRenderer *napi,
    std::shared_ptr<AudioRendererAsyncContext> context)
{
    CHECK_AND_RETURN_RET_LOG(napi != nullptr, false, "napi object is nullptr.");
    if (napi->audioRenderer_ == nullptr) {
        context->SignError(NAPI_ERR_SYSTEM);
        return false;
    }
    return true;
}

NapiAudioRenderer* NapiAudioRenderer::GetParamWithSync(const napi_env &env, napi_callback_info info,
    size_t &argc, napi_value *args)
{
    NapiAudioRenderer *napiAudioRenderer = nullptr;
    napi_value jsThis = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok && jsThis != nullptr, nullptr,
        "GetParamWithSync fail to napi_get_cb_info");

    status = napi_unwrap(env, jsThis, (void **)&napiAudioRenderer);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, nullptr, "napi_unwrap failed");
    CHECK_AND_RETURN_RET_LOG(napiAudioRenderer != nullptr && napiAudioRenderer->audioRenderer_ != nullptr,
        napiAudioRenderer, "GetParamWithSync fail to napi_unwrap");
    return napiAudioRenderer;
}

napi_value NapiAudioRenderer::CreateAudioRendererWrapper(napi_env env, const AudioRendererOptions rendererOptions)
{
    lock_guard<mutex> lock(createMutex_);
    napi_value result = nullptr;
    napi_value constructor;

    napi_status status = napi_get_reference_value(env, g_rendererConstructor, &constructor);
    if (status != napi_ok) {
        AUDIO_ERR_LOG("Failed in CreateAudioRendererWrapper, %{public}d", status);
        goto fail;
    }
    if (sRendererOptions_ != nullptr) {
        sRendererOptions_.release();
    }
    sRendererOptions_ = make_unique<AudioRendererOptions>();
    CHECK_AND_RETURN_RET_LOG(sRendererOptions_ != nullptr, result, "sRendererOptions_ create failed");
    *sRendererOptions_ = rendererOptions;
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

napi_value NapiAudioRenderer::CreateAudioRenderer(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioRendererAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("CreateAudioRenderer failed : no memory");
        NapiAudioError::ThrowError(env, "CreateAudioRenderer failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetRendererOptions(env, &context->rendererOptions, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get audioRendererRate failed",
            NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);

    auto complete = [env, context](napi_value &output) {
        output = CreateAudioRendererWrapper(env, context->rendererOptions);

        // IsConstructSuccess_ Used when creating a renderer fails.
        if (isConstructSuccess_ != SUCCESS) {
            context->SignError(isConstructSuccess_);
            isConstructSuccess_ = SUCCESS;
        }
    };

    return NapiAsyncWork::Enqueue(env, context, "CreateAudioRenderer", nullptr, complete);
}

napi_value NapiAudioRenderer::CreateAudioRendererSync(napi_env env, napi_callback_info info)
{
    AUDIO_INFO_LOG("CreateAudioRendererSync");
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = {};
    napi_status status = NapiParamUtils::GetParam(env, info, argc, argv);
    CHECK_AND_RETURN_RET_LOG((argc == ARGS_ONE) && (status == napi_ok),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID), "GetParam failed");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_object,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID), "valueType invaild");

    AudioRendererOptions rendererOptions;
    CHECK_AND_RETURN_RET_LOG(NapiParamUtils::GetRendererOptions(env, &rendererOptions, argv[PARAM0]) == napi_ok,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM), "GetRendererOptions failed");

    return NapiAudioRenderer::CreateAudioRendererWrapper(env, rendererOptions);
}

napi_value NapiAudioRenderer::SetRenderRate(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioRendererAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("SetRenderRate failed : no memory");
        NapiAudioError::ThrowError(env, "SetRenderRate failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->audioRendererRate, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get audioRendererRate failed",
            NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioRenderer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioRenderer = objectGuard.GetPtr();
        AudioRendererRate audioRenderRate = static_cast<AudioRendererRate>(context->audioRendererRate);
        CHECK_AND_RETURN_LOG(CheckAudioRendererStatus(napiAudioRenderer, context),
            "context object state is error.");
        int32_t audioClientInvalidParamsErr = -2;
        context->intValue = napiAudioRenderer->audioRenderer_->SetRenderRate(audioRenderRate);
        if (context->intValue != SUCCESS) {
            if (context->intValue == audioClientInvalidParamsErr) {
                context->SignError(NAPI_ERR_UNSUPPORTED);
            } else {
                context->SignError(NAPI_ERR_SYSTEM);
            }
        }
    };
    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SetRenderRate", executor, complete);
}

napi_value NapiAudioRenderer::GetRenderRate(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioRendererAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("GetRenderRate failed : no memory");
        NapiAudioError::ThrowError(env, "GetRenderRate failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioRenderer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioRenderer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioRendererStatus(napiAudioRenderer, context),
            "context object state is error.");
        context->intValue = napiAudioRenderer->audioRenderer_->GetRenderRate();
    };
    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueInt32(env, context->intValue, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "GetRenderRate", executor, complete);
}

napi_value NapiAudioRenderer::GetRenderRateSync(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = PARAM0;
    auto *napiAudioRenderer = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(argc == PARAM0, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID),
        "argcCount invaild");

    CHECK_AND_RETURN_RET_LOG(napiAudioRenderer != nullptr, result, "napiAudioRenderer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioRenderer->audioRenderer_ != nullptr, result, "audioRenderer_ is nullptr");
    AudioRendererRate rendererRate = napiAudioRenderer->audioRenderer_->GetRenderRate();
    NapiParamUtils::SetValueInt32(env, static_cast<int32_t>(rendererRate), result);
    return result;
}

napi_value NapiAudioRenderer::SetRendererSamplingRate(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioRendererAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("SetRendererSamplingRate failed : no memory");
        NapiAudioError::ThrowError(env, "SetRendererSamplingRate failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueUInt32(env, context->rendererSampleRate, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get RendererSamplingRate failed",
            NAPI_ERR_INVALID_PARAM);
    };

    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioRenderer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioRenderer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioRendererStatus(napiAudioRenderer, context),
            "context object state is error.");
        if (context->rendererSampleRate <= 0) {
            context->SignError(NAPI_ERR_UNSUPPORTED);
            return;
        }
        context->intValue =
            napiAudioRenderer->audioRenderer_->SetRendererSamplingRate(context->rendererSampleRate);
        if (context->intValue != SUCCESS) {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SetRendererSamplingRate", executor, complete);
}

napi_value NapiAudioRenderer::GetRendererSamplingRate(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioRendererAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("GetRendererSamplingRate failed : no memory");
        NapiAudioError::ThrowError(env, "GetRendererSamplingRate failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioRenderer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioRenderer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioRendererStatus(napiAudioRenderer, context),
            "context object state is error.");
        context->rendererSampleRate = napiAudioRenderer->audioRenderer_->GetRendererSamplingRate();
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueUInt32(env, context->rendererSampleRate, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "GetRendererSamplingRate", executor, complete);
}

napi_value NapiAudioRenderer::Start(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioRendererAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("Start failed : no memory");
        NapiAudioError::ThrowError(env, "Start failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    context->GetCbInfo(env, info);
#ifdef FEATURE_HIVIEW_ENABLE
#if !defined(ANDROID_PLATFORM) && !defined(IOS_PLATFORM)
    HiviewDFX::ReportXPowerJsStackSysEvent(env, "STREAM_CHANGE", "SRC=Audio");
#endif
#endif

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioRenderer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioRenderer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioRendererStatus(napiAudioRenderer, context),
            "context object state is error.");
        context->isTrue = napiAudioRenderer->audioRenderer_->Start();
        context->status = context->isTrue ? napi_ok : napi_generic_failure;
        if (context->status != napi_ok) {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "Start", executor, complete);
}

napi_value NapiAudioRenderer::Write(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioRendererAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("Write failed : no memory");
        NapiAudioError::ThrowError(env, "Write failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetArrayBuffer(env, context->data, context->bufferLen, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get buffer failed",
            NAPI_ERR_INVALID_PARAM);
    };

    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        context->status = WriteArrayBufferToNative(context);
        if (context->status != napi_ok) {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueUInt32(env, context->totalBytesWritten, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "Write", executor, complete);
}

napi_status NapiAudioRenderer::WriteArrayBufferToNative(std::shared_ptr<AudioRendererAsyncContext> context)
{
    CHECK_AND_RETURN_RET_LOG(CheckContextStatus(context), napi_generic_failure, "context object state is error.");
    auto obj = reinterpret_cast<NapiAudioRenderer*>(context->native);
    ObjectRefMap objectGuard(obj);
    auto *napiAudioRenderer = objectGuard.GetPtr();
    CHECK_AND_RETURN_RET_LOG(CheckAudioRendererStatus(napiAudioRenderer, context),
        napi_generic_failure, "context object state is error.");
    size_t bufferLen = context->bufferLen;
    context->status = napi_generic_failure;
    auto buffer = std::make_unique<uint8_t[]>(bufferLen);
    CHECK_AND_RETURN_RET_LOG(buffer != nullptr, napi_generic_failure, "Renderer write buffer allocation failed");
    if (memcpy_s(buffer.get(), bufferLen, context->data, bufferLen)) {
        AUDIO_ERR_LOG("Renderer mem copy failed");
        return napi_generic_failure;
    }
    int32_t bytesWritten = 0;
    size_t totalBytesWritten = 0;
    size_t minBytes = 4;
    while ((totalBytesWritten < bufferLen) && ((bufferLen - totalBytesWritten) > minBytes)) {
        bytesWritten = napiAudioRenderer->audioRenderer_->Write(buffer.get() + totalBytesWritten,
        bufferLen - totalBytesWritten);
        if (bytesWritten < 0) {
            AUDIO_ERR_LOG("Write length < 0,break.");
            break;
        }
        totalBytesWritten += static_cast<size_t>(bytesWritten);
    }
    context->status = napi_ok;
    context->totalBytesWritten = totalBytesWritten;
    return context->status;
}

napi_value NapiAudioRenderer::GetAudioTime(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioRendererAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("GetAudioTime failed : no memory");
        NapiAudioError::ThrowError(env, "GetAudioTime failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioRenderer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioRenderer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioRendererStatus(napiAudioRenderer, context),
            "context object state is error.");
        Timestamp timestamp;
        if (napiAudioRenderer->audioRenderer_->GetAudioTime(timestamp, Timestamp::Timestampbase::MONOTONIC)) {
            const uint64_t secToNanosecond = 1000000000;
            context->time = static_cast<uint64_t>(timestamp.time.tv_nsec) +
                static_cast<uint64_t>(timestamp.time.tv_sec) * secToNanosecond;
            context->status = napi_ok;
        } else {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueInt64(env, context->time, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "GetAudioTime", executor, complete);
}

napi_value NapiAudioRenderer::GetAudioTimeSync(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = PARAM0;
    auto *napiAudioRenderer = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(argc == PARAM0, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID),
        "argcCount invaild");

    CHECK_AND_RETURN_RET_LOG(napiAudioRenderer != nullptr, result, "napiAudioRenderer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioRenderer->audioRenderer_ != nullptr, result, "audioRenderer_ is nullptr");
    Timestamp timestamp;
    bool ret = napiAudioRenderer->audioRenderer_->GetAudioTime(timestamp, Timestamp::Timestampbase::MONOTONIC);
    CHECK_AND_RETURN_RET_LOG(ret, result, "GetAudioTime failure!");

    const uint64_t secToNanosecond = 1000000000;
    uint64_t time = static_cast<uint64_t>(timestamp.time.tv_nsec) +
        static_cast<uint64_t>(timestamp.time.tv_sec) * secToNanosecond;

    NapiParamUtils::SetValueInt64(env, time, result);
    return result;
}

napi_value NapiAudioRenderer::Drain(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioRendererAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("Drain failed : no memory");
        NapiAudioError::ThrowError(env, "Drain failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioRenderer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioRenderer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioRendererStatus(napiAudioRenderer, context),
            "context object state is error.");
        context->isTrue = napiAudioRenderer->audioRenderer_->Drain();
        if (!context->isTrue) {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };
    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "Drain", executor, complete);
}

napi_value NapiAudioRenderer::Flush(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioRendererAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("Flush failed : no memory");
        NapiAudioError::ThrowError(env, "Flush failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    context->GetCbInfo(env, info, nullptr, true);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioRenderer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioRenderer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioRendererStatus(napiAudioRenderer, context),
            "context object state is error.");
        context->isTrue = napiAudioRenderer->audioRenderer_->Flush();
        if (!context->isTrue) {
            context->SignError(NAPI_ERR_ILLEGAL_STATE);
        }
    };
    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "Flush", executor, complete);
}

napi_value NapiAudioRenderer::Pause(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioRendererAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("Pause failed : no memory");
        NapiAudioError::ThrowError(env, "Pause failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioRenderer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioRenderer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioRendererStatus(napiAudioRenderer, context),
            "context object state is error.");
        context->isTrue = napiAudioRenderer->audioRenderer_->Pause();
        if (!context->isTrue) {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };
    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "Pause", executor, complete);
}

napi_value NapiAudioRenderer::Stop(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioRendererAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("Stop failed : no memory");
        NapiAudioError::ThrowError(env, "Stop failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioRenderer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioRenderer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioRendererStatus(napiAudioRenderer, context),
            "context object state is error.");
        context->isTrue = napiAudioRenderer->audioRenderer_->Stop();
        if (!context->isTrue) {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };
    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "Stop", executor, complete);
}

napi_value NapiAudioRenderer::Release(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioRendererAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("Release failed : no memory");
        NapiAudioError::ThrowError(env, "Release failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioRenderer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioRenderer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioRendererStatus(napiAudioRenderer, context),
            "context object state is error.");
        context->isTrue = napiAudioRenderer->audioRenderer_->Release();
        if (!context->isTrue) {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };
    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "Release", executor, complete);
}

napi_value NapiAudioRenderer::GetBufferSize(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioRendererAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("GetBufferSize failed : no memory");
        NapiAudioError::ThrowError(env, "GetBufferSize failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioRenderer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioRenderer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioRendererStatus(napiAudioRenderer, context),
            "context object state is error.");
        size_t bufferSize;
        context->intValue = napiAudioRenderer->audioRenderer_->GetBufferSize(bufferSize);
        if (context->intValue != SUCCESS) {
            context->SignError(NAPI_ERR_SYSTEM);
        } else {
            context->bufferSize = bufferSize;
        }
    };
    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueUInt32(env, context->bufferSize, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "GetBufferSize", executor, complete);
}

napi_value NapiAudioRenderer::GetBufferSizeSync(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = PARAM0;
    auto *napiAudioRenderer = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(argc == PARAM0, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID),
        "argcCount invaild");

    CHECK_AND_RETURN_RET_LOG(napiAudioRenderer != nullptr, result, "napiAudioRenderer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioRenderer->audioRenderer_ != nullptr, result, "audioRenderer_ is nullptr");
    size_t bufferSize;
    int32_t ret = napiAudioRenderer->audioRenderer_->GetBufferSize(bufferSize);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, result, "GetBufferSize failure!");

    NapiParamUtils::SetValueUInt32(env, bufferSize, result);
    return result;
}

napi_value NapiAudioRenderer::GetAudioStreamId(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioRendererAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("GetAudioStreamId failed : no memory");
        NapiAudioError::ThrowError(env, "GetAudioStreamId failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioRenderer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioRenderer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioRendererStatus(napiAudioRenderer, context),
            "context object state is error.");
        context->intValue = napiAudioRenderer->audioRenderer_->GetAudioStreamId(context->audioStreamId);
        if (context->intValue == ERR_INVALID_INDEX) {
            context->SignError(NAPI_ERR_SYSTEM);
        } else if (context->intValue == ERR_ILLEGAL_STATE) {
            context->SignError(NAPI_ERR_ILLEGAL_STATE);
        }
    };
    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueUInt32(env, context->audioStreamId, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "GetAudioStreamId", executor, complete);
}

napi_value NapiAudioRenderer::GetAudioStreamIdSync(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = PARAM0;
    auto *napiAudioRenderer = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(argc == PARAM0, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID),
        "argcCount invaild");

    CHECK_AND_RETURN_RET_LOG(napiAudioRenderer != nullptr, result, "napiAudioRenderer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioRenderer->audioRenderer_ != nullptr, result, "audioRenderer_ is nullptr");
    uint32_t audioStreamId;
    int32_t streamIdStatus = napiAudioRenderer->audioRenderer_->GetAudioStreamId(audioStreamId);
    CHECK_AND_RETURN_RET_LOG(streamIdStatus == SUCCESS, result, "GetAudioStreamId failure!");

    NapiParamUtils::SetValueUInt32(env, audioStreamId, result);
    return result;
}

napi_value NapiAudioRenderer::SetVolume(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioRendererAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("SetVolume failed : no memory");
        NapiAudioError::ThrowError(env, "SetVolume failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueDouble(env, context->volLevel, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get RendererSamplingRate failed",
            NAPI_ERR_INVALID_PARAM);
    };

    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioRenderer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioRenderer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioRendererStatus(napiAudioRenderer, context),
            "context object state is error.");
        if (context->volLevel < MIN_VOLUME_IN_DOUBLE || context->volLevel > MAX_VOLUME_IN_DOUBLE) {
            context->SignError(NAPI_ERR_UNSUPPORTED);
            return;
        }
        context->intValue = napiAudioRenderer->audioRenderer_->SetVolume(static_cast<float>(context->volLevel));
        if (context->intValue != SUCCESS) {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SetVolume", executor, complete);
}

napi_value NapiAudioRenderer::GetVolume(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = PARAM0;
    auto *napiAudioRenderer = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(napiAudioRenderer != nullptr, result, "napiAudioRenderer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioRenderer->audioRenderer_ != nullptr, result, "audioRenderer_ is nullptr");

    double volLevel = napiAudioRenderer->audioRenderer_->GetVolume();
    NapiParamUtils::SetValueDouble(env, volLevel, result);
    return result;
}

napi_value NapiAudioRenderer::GetRendererInfo(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioRendererAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("GetRendererInfo failed : no memory");
        NapiAudioError::ThrowError(env, "GetRendererInfo failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioRenderer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioRenderer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioRendererStatus(napiAudioRenderer, context),
            "context object state is error.");
        context->intValue = napiAudioRenderer->audioRenderer_->GetRendererInfo(context->rendererInfo);
        if (context->intValue != SUCCESS) {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetRendererInfo(env, context->rendererInfo, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "GetRendererInfo", executor, complete);
}

napi_value NapiAudioRenderer::GetRendererInfoSync(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = PARAM0;
    auto *napiAudioRenderer = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(argc == PARAM0, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID),
        "argcCount invaild");

    CHECK_AND_RETURN_RET_LOG(napiAudioRenderer != nullptr, result, "napiAudioRenderer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioRenderer->audioRenderer_ != nullptr, result, "audioRenderer_ is nullptr");
    AudioRendererInfo rendererInfo = {};
    int32_t ret = napiAudioRenderer->audioRenderer_->GetRendererInfo(rendererInfo);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, result, "GetRendererInfo failure!");
    NapiParamUtils::SetRendererInfo(env, rendererInfo, result);
    return result;
}

napi_value NapiAudioRenderer::GetStreamInfo(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioRendererAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("GetStreamInfo failed : no memory");
        NapiAudioError::ThrowError(env, "GetStreamInfo failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioRenderer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioRenderer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioRendererStatus(napiAudioRenderer, context),
            "context object state is error.");
        context->intValue = napiAudioRenderer->audioRenderer_->GetStreamInfo(context->streamInfo);
        if (context->intValue != SUCCESS) {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetStreamInfo(env, context->streamInfo, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "GetStreamInfo", executor, complete);
}

napi_value NapiAudioRenderer::GetStreamInfoSync(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = PARAM0;
    auto *napiAudioRenderer = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(argc == PARAM0, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID),
        "argcCount invaild");

    CHECK_AND_RETURN_RET_LOG(napiAudioRenderer != nullptr, result, "napiAudioRenderer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioRenderer->audioRenderer_ != nullptr, result, "audioRenderer_ is nullptr");

    AudioStreamInfo streamInfo;
    int32_t ret = napiAudioRenderer->audioRenderer_->GetStreamInfo(streamInfo);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, result, "GetStreamInfo failure!");
    NapiParamUtils::SetStreamInfo(env, streamInfo, result);
    return result;
}

napi_value NapiAudioRenderer::SetInterruptMode(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioRendererAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("SetInterruptMode failed : no memory");
        NapiAudioError::ThrowError(env, "SetInterruptMode failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->interruptMode, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get interruptMode failed",
            NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioRenderer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioRenderer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioRendererStatus(napiAudioRenderer, context),
            "context object state is error.");
        InterruptMode interruptMode = NapiAudioEnum::GetNativeInterruptMode(context->interruptMode);
        napiAudioRenderer->audioRenderer_->SetInterruptMode(interruptMode);
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SetInterruptMode", executor, complete);
}

napi_value NapiAudioRenderer::SetInterruptModeSync(napi_env env, napi_callback_info info)
{
    AUDIO_INFO_LOG("SetInterruptModeSync");
    napi_value result = nullptr;
    size_t argc = ARGS_ONE;
    napi_value args[ARGS_ONE] = {};
    auto *napiAudioRenderer = GetParamWithSync(env, info, argc, args);
    CHECK_AND_RETURN_RET_LOG(argc == ARGS_ONE, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
        "mandatory parameters are left unspecified"), "argcCount invaild");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, args[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_number, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
        "incorrect parameter types: The type of mode must be number"), "valueType invaild");

    int32_t interruptMode;
    NapiParamUtils::GetValueInt32(env, interruptMode, args[PARAM0]);

    if (!NapiAudioEnum::IsLegalInputArgumentInterruptMode(interruptMode)) {
        NapiAudioError::ThrowError(env, NAPI_ERR_INVALID_PARAM,
            "parameter verification failed: The param of mode must be enum InterruptMode");
        return result;
    }
    CHECK_AND_RETURN_RET_LOG(napiAudioRenderer != nullptr, result, "napiAudioRenderer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioRenderer->audioRenderer_ != nullptr, result, "audioRenderer_ is nullptr");
    napiAudioRenderer->audioRenderer_->SetInterruptMode(NapiAudioEnum::GetNativeInterruptMode(interruptMode));

    return result;
}

napi_value NapiAudioRenderer::GetMinStreamVolume(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioRendererAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("GetMinStreamVolume failed : no memory");
        NapiAudioError::ThrowError(env, "GetMinStreamVolume failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioRenderer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioRenderer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioRendererStatus(napiAudioRenderer, context),
            "context object state is error.");
        context->volLevel = napiAudioRenderer->audioRenderer_->GetMinStreamVolume();
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueDouble(env, context->volLevel, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "GetMinStreamVolume", executor, complete);
}

napi_value NapiAudioRenderer::GetMinStreamVolumeSync(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = PARAM0;
    auto *napiAudioRenderer = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(argc == PARAM0, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID),
        "argcCount invaild");

    CHECK_AND_RETURN_RET_LOG(napiAudioRenderer != nullptr, result, "napiAudioRenderer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioRenderer->audioRenderer_ != nullptr, result, "audioRenderer_ is nullptr");

    double volLevel = napiAudioRenderer->audioRenderer_->GetMinStreamVolume();
    NapiParamUtils::SetValueDouble(env, volLevel, result);
    return result;
}

napi_value NapiAudioRenderer::GetMaxStreamVolume(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioRendererAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("GetMaxStreamVolume failed : no memory");
        NapiAudioError::ThrowError(env, "GetMaxStreamVolume failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioRenderer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioRenderer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioRendererStatus(napiAudioRenderer, context),
            "context object state is error.");
        context->volLevel = napiAudioRenderer->audioRenderer_->GetMaxStreamVolume();
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueDouble(env, context->volLevel, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "GetMaxStreamVolume", executor, complete);
}

napi_value NapiAudioRenderer::GetMaxStreamVolumeSync(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = PARAM0;
    auto *napiAudioRenderer = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(argc == PARAM0, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID),
        "argcCount invaild");

    CHECK_AND_RETURN_RET_LOG(napiAudioRenderer != nullptr, result, "napiAudioRenderer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioRenderer->audioRenderer_ != nullptr, result, "audioRenderer_ is nullptr");

    double volLevel = napiAudioRenderer->audioRenderer_->GetMaxStreamVolume();
    NapiParamUtils::SetValueDouble(env, volLevel, result);
    return result;
}

napi_value NapiAudioRenderer::GetCurrentOutputDevices(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioRendererAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("GetCurrentOutputDevices failed : no memory");
        NapiAudioError::ThrowError(env, "GetCurrentOutputDevices failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioRenderer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioRenderer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioRendererStatus(napiAudioRenderer, context),
            "context object state is error.");
        DeviceInfo deviceInfo;
        context->intValue = napiAudioRenderer->audioRenderer_->GetCurrentOutputDevices(deviceInfo);
        if (context->intValue == ERR_INVALID_PARAM) {
            context->SignError(NAPI_ERROR_INVALID_PARAM);
            return;
        }
        context->deviceInfo = deviceInfo;
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueDeviceInfo(env, context->deviceInfo, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "GetCurrentOutputDevices", executor, complete);
}

napi_value NapiAudioRenderer::GetCurrentOutputDevicesSync(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = PARAM0;
    auto *napiAudioRenderer = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(argc == PARAM0, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID),
        "argcCount invaild");

    CHECK_AND_RETURN_RET_LOG(napiAudioRenderer != nullptr, result, "napiAudioRenderer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioRenderer->audioRenderer_ != nullptr, result, "audioRenderer_ is nullptr");

    DeviceInfo deviceInfo;
    int32_t ret = napiAudioRenderer->audioRenderer_->GetCurrentOutputDevices(deviceInfo);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, result, "GetCurrentOutputDevices failure!");

    NapiParamUtils::SetValueDeviceInfo(env, deviceInfo, result);
    return result;
}

napi_value NapiAudioRenderer::GetUnderflowCount(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioRendererAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("GetUnderflowCount failed : no memory");
        NapiAudioError::ThrowError(env, "GetUnderflowCount failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioRenderer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioRenderer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioRendererStatus(napiAudioRenderer, context),
            "context object state is error.");
        context->underflowCount = napiAudioRenderer->audioRenderer_->GetUnderflowCount();
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueUInt32(env, context->underflowCount, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "GetUnderflowCount", executor, complete);
}

napi_value NapiAudioRenderer::GetUnderflowCountSync(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = PARAM0;
    auto *napiAudioRenderer = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(argc == PARAM0, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID),
        "argcCount invaild");

    CHECK_AND_RETURN_RET_LOG(napiAudioRenderer != nullptr, result, "napiAudioRenderer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioRenderer->audioRenderer_ != nullptr, result, "audioRenderer_ is nullptr");

    uint32_t underflowCount = napiAudioRenderer->audioRenderer_->GetUnderflowCount();
    NapiParamUtils::SetValueUInt32(env, underflowCount, result);
    return result;
}

napi_value NapiAudioRenderer::GetAudioEffectMode(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioRendererAsyncContext>();
    if (context == nullptr) {
        NapiAudioError::ThrowError(env, "GetAudioEffectMode failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioRenderer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioRenderer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioRendererStatus(napiAudioRenderer, context),
            "context object state is error.");
        context->intValue = napiAudioRenderer->audioRenderer_->GetAudioEffectMode();
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueInt32(env, context->intValue, output);
    };
    return NapiAsyncWork::Enqueue(env, context, "GetAudioEffectMode", executor, complete);
}

napi_value NapiAudioRenderer::SetAudioEffectMode(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioRendererAsyncContext>();
    if (context == nullptr) {
        NapiAudioError::ThrowError(env, "SetAudioEffectMode failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "mandatory parameters are left unspecified",
            NAPI_ERR_INPUT_INVALID);
        context->status = NapiParamUtils::GetValueInt32(env, context->audioEffectMode, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok,
            "incorrect parameter types: The type of mode must be number", NAPI_ERR_INPUT_INVALID);
        NAPI_CHECK_ARGS_RETURN_VOID(context,
            NapiAudioEnum::IsLegalInputArgumentAudioEffectMode(context->audioEffectMode),
            "parameter verification failed: The param of mode must be enum AudioEffectMode", NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);

    if ((context->status != napi_ok) && (context->errCode == NAPI_ERR_INPUT_INVALID)) {
        NapiAudioError::ThrowError(env, context->errCode, context->errMessage);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioRenderer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioRenderer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioRendererStatus(napiAudioRenderer, context),
            "context object state is error.");
        AudioEffectMode audioEffectMode = static_cast<AudioEffectMode>(context->audioEffectMode);
        context->intValue = napiAudioRenderer->audioRenderer_->SetAudioEffectMode(audioEffectMode);
        if (context->intValue != SUCCESS) {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SetAudioEffectMode", executor, complete);
}

napi_value NapiAudioRenderer::SetChannelBlendMode(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = {};
    auto *napiAudioRenderer = GetParamWithSync(env, info, argc, argv);
    CHECK_AND_RETURN_RET_LOG(argc == ARGS_ONE, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
        "mandatory parameters are left unspecified"), "argcCount invaild");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_number, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
        "incorrect parameter types: The type of mode must be number"), "valueType params");

    int32_t channelBlendMode;
    NapiParamUtils::GetValueInt32(env, channelBlendMode, argv[PARAM0]);
    CHECK_AND_RETURN_RET_LOG(NapiAudioEnum::IsLegalInputArgumentChannelBlendMode(channelBlendMode),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
        "parameter verification failed: The param of mode must be enum ChannelBlendMode"), "unsupport params");

    CHECK_AND_RETURN_RET_LOG(napiAudioRenderer!= nullptr, result, "napiAudioRenderer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioRenderer->audioRenderer_ != nullptr, result, "audioRenderer_ is nullptr");
    int32_t ret =
        napiAudioRenderer->audioRenderer_->SetChannelBlendMode(static_cast<ChannelBlendMode>(channelBlendMode));
    CHECK_AND_RETURN_RET_LOG(ret != ERR_ILLEGAL_STATE,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_ILLEGAL_STATE), "err illegal state");

    return result;
}

napi_value NapiAudioRenderer::SetVolumeWithRamp(napi_env env, napi_callback_info info)
{
    AUDIO_INFO_LOG("SetVolumeWithRamp");
    napi_value result = nullptr;
    size_t argc = ARGS_TWO;
    napi_value argv[ARGS_TWO] = {};
    auto *napiAudioRenderer = GetParamWithSync(env, info, argc, argv);
    CHECK_AND_RETURN_RET_LOG(argc >= ARGS_TWO, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
        "mandatory parameters are left unspecified"), "argcCount invaild");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_number, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
        "incorrect parameter types: The type of volume must be number"), "valueType param0 invaild");
    napi_typeof(env, argv[PARAM1], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_number, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
        "incorrect parameter types: The type of duration must be number"), "valueType param1 invaild");

    double volume;
    NapiParamUtils::GetValueDouble(env, volume, argv[PARAM0]);
    CHECK_AND_RETURN_RET_LOG((volume >= MIN_VOLUME_IN_DOUBLE) && (volume <= MAX_VOLUME_IN_DOUBLE),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
        "parameter verification failed: invaild volume index"), "invaild volume index");

    int32_t duration;
    NapiParamUtils::GetValueInt32(env, duration, argv[PARAM1]);
    CHECK_AND_RETURN_RET_LOG(napiAudioRenderer!= nullptr, result, "napiAudioRenderer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioRenderer->audioRenderer_ != nullptr, result, "audioRenderer_ is nullptr");
    int32_t ret =
        napiAudioRenderer->audioRenderer_->SetVolumeWithRamp(static_cast<float>(volume), duration);
    CHECK_AND_RETURN_RET_LOG(ret != ERR_ILLEGAL_STATE,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_ILLEGAL_STATE), "err illegal state");
    return result;
}

napi_value NapiAudioRenderer::SetSpeed(napi_env env, napi_callback_info info)
{
    AUDIO_INFO_LOG("SetSpeed");
    napi_value result = nullptr;
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = {};
    auto *napiAudioRenderer = GetParamWithSync(env, info, argc, argv);
    CHECK_AND_RETURN_RET_LOG(argc >= ARGS_ONE, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
        "mandatory parameters are left unspecified"), "argcCount invaild");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_number, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
        "incorrect parameter types: The type of speed must be number"), "valueType param0 invaild");

    double speed;
    NapiParamUtils::GetValueDouble(env, speed, argv[PARAM0]);
    CHECK_AND_RETURN_RET_LOG((speed >= MIN_STREAM_SPEED_LEVEL) && (speed <= MAX_STREAM_SPEED_LEVEL),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM),
        "parameter verification failed: invaild speed index");

    CHECK_AND_RETURN_RET_LOG(napiAudioRenderer!= nullptr, result, "napiAudioRenderer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioRenderer->audioRenderer_ != nullptr, result, "audioRenderer_ is nullptr");
    int32_t ret = napiAudioRenderer->audioRenderer_->SetSpeed(static_cast<float>(speed));
    CHECK_AND_RETURN_RET_LOG(ret != ERR_ILLEGAL_STATE,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_ILLEGAL_STATE), "err illegal state");
    return result;
}

napi_value NapiAudioRenderer::GetSpeed(napi_env env, napi_callback_info info)
{
    AUDIO_INFO_LOG("GetSpeed");
    napi_value result = nullptr;
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = {};
    auto *napiAudioRenderer = GetParamWithSync(env, info, argc, argv);
    CHECK_AND_RETURN_RET_LOG(napiAudioRenderer!= nullptr, result, "napiAudioRenderer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioRenderer->audioRenderer_ != nullptr, result, "audioRenderer_ is nullptr");

    double ret = napiAudioRenderer->audioRenderer_->GetSpeed();
    napi_create_double(env, ret, &result);
    return result;
}

napi_value NapiAudioRenderer::GetState(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = PARAM0;
    auto *napiAudioRenderer = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(argc == PARAM0, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID),
        "invaild params");

    CHECK_AND_RETURN_RET_LOG(napiAudioRenderer!= nullptr, result, "napiAudioRenderer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioRenderer->audioRenderer_ != nullptr, result, "audioRenderer_ is nullptr");

    uint32_t rendererState = napiAudioRenderer->audioRenderer_->GetStatus();
    napi_status status = NapiParamUtils::SetValueInt32(env, rendererState, result);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "SetValueInt32 failed");
    return result;
}

napi_value NapiAudioRenderer::On(napi_env env, napi_callback_info info)
{
    const size_t requireArgc = ARGS_TWO;
    size_t argc = ARGS_THREE;

    napi_value argv[requireArgc + 1] = {nullptr, nullptr, nullptr};
    napi_value jsThis = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &jsThis, nullptr);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_SYSTEM),
        "status error");
    CHECK_AND_RETURN_RET_LOG(argc >= requireArgc, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
        "mandatory parameters are left unspecified"), "requireArgc is invaild");

    napi_valuetype eventType = napi_undefined;
    napi_typeof(env, argv[PARAM0], &eventType);
    CHECK_AND_RETURN_RET_LOG(eventType == napi_string, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
        "incorrect parameter types: The type of eventType must be string"), "eventType is invaild");

    std::string callbackName = NapiParamUtils::GetStringArgument(env, argv[PARAM0]);
    AUDIO_DEBUG_LOG("AudioRendererNapi: On callbackName: %{public}s", callbackName.c_str());

    napi_valuetype handler = napi_undefined;
    if (argc == requireArgc) {
        napi_typeof(env, argv[PARAM1], &handler);
        CHECK_AND_RETURN_RET_LOG(handler == napi_function, NapiAudioError::ThrowErrorAndReturn(env,
            NAPI_ERR_INPUT_INVALID, "incorrect parameter types: The type of callback must be function"),
            "handler is invaild");
    } else {
        napi_valuetype paramArg1 = napi_undefined;
        napi_typeof(env, argv[PARAM1], &paramArg1);
        napi_valuetype expectedValType = napi_number;  // Default. Reset it with 'callbackName' if check, if required.
        CHECK_AND_RETURN_RET_LOG(paramArg1 == expectedValType, NapiAudioError::ThrowErrorAndReturn(env,
            NAPI_ERR_INPUT_INVALID, "incorrect parameter types: The type of frame must be number"),
            "paramArg1 is invaild");
        const int32_t arg2 = ARGS_TWO;
        napi_typeof(env, argv[arg2], &handler);
        CHECK_AND_RETURN_RET_LOG(handler == napi_function, NapiAudioError::ThrowErrorAndReturn(env,
            NAPI_ERR_INPUT_INVALID, "incorrect parameter types: The type of callback must be function"),
            "handler2 is invaild");
    }

    return RegisterCallback(env, jsThis, argv, callbackName);
}

napi_value NapiAudioRenderer::Off(napi_env env, napi_callback_info info)
{
    const size_t requireArgc = ARGS_TWO;
    size_t argc = ARGS_THREE;

    napi_value argv[requireArgc + 1] = {nullptr, nullptr, nullptr};
    napi_value jsThis = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &jsThis, nullptr);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_SYSTEM),
        "status error");
    CHECK_AND_RETURN_RET_LOG(argc <= requireArgc, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
        "mandatory parameters are left unspecified"), "argc is invaild");

    napi_valuetype eventType = napi_undefined;
    napi_typeof(env, argv[PARAM0], &eventType);
    CHECK_AND_RETURN_RET_LOG(eventType == napi_string, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
        "incorrect parameter types: The type of eventType must be string"), "eventType is invaild");

    std::string callbackName = NapiParamUtils::GetStringArgument(env, argv[PARAM0]);
    AUDIO_DEBUG_LOG("AudioRendererNapi: Off callbackName: %{public}s", callbackName.c_str());

    return UnregisterCallback(env, jsThis, argc, argv, callbackName);
}

napi_value NapiAudioRenderer::SetSilentModeAndMixWithOthers(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = {};
    auto *napiAudioRenderer = GetParamWithSync(env, info, argc, argv);
    CHECK_AND_RETURN_RET_LOG(argc >= ARGS_ONE, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
        "mandatory parameters are left unspecified"), "argcCount invalid");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_boolean, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
        "incorrect parameter types: The type of on must be bool"), "valueType param0 invalid");

    bool on;
    NapiParamUtils::GetValueBoolean(env, on, argv[PARAM0]);

    CHECK_AND_RETURN_RET_LOG(napiAudioRenderer != nullptr, result, "napiAudioRenderer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioRenderer->audioRenderer_ != nullptr, result, "audioRenderer_ is nullptr");
    napiAudioRenderer->audioRenderer_->SetSilentModeAndMixWithOthers(on);
    return result;
}

napi_value NapiAudioRenderer::GetSilentModeAndMixWithOthers(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = {};
    auto *napiAudioRenderer = GetParamWithSync(env, info, argc, argv);
    CHECK_AND_RETURN_RET_LOG(napiAudioRenderer != nullptr, result, "napiAudioRenderer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioRenderer->audioRenderer_ != nullptr, result, "audioRenderer_ is nullptr");

    bool on = napiAudioRenderer->audioRenderer_->GetSilentModeAndMixWithOthers();
    napi_get_boolean(env, on, &result);
    return result;
}

napi_value NapiAudioRenderer::SetDefaultOutputDevice(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioRendererAsyncContext>();
    if (context == nullptr) {
        NapiAudioError::ThrowError(env, "SetDefaultOutputDevice failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "mandatory parameters are left unspecified",
            NAPI_ERR_INPUT_INVALID);
        context->status = NapiParamUtils::GetValueInt32(env, context->deviceType, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok,
            "incorrect parameter types: The type of mode must be number", NAPI_ERR_INPUT_INVALID);
        NAPI_CHECK_ARGS_RETURN_VOID(context,
            NapiAudioEnum::IsLegalInputArgumentDefaultOutputDeviceType(context->deviceType),
            "parameter verification failed: The param of mode must be enum deviceType", NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);

    if ((context->status != napi_ok) && (context->errCode == NAPI_ERR_INPUT_INVALID ||
        context->errCode == NAPI_ERR_INVALID_PARAM)) {
        NapiAudioError::ThrowError(env, context->errCode, context->errMessage);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioRenderer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioRenderer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioRendererStatus(napiAudioRenderer, context),
            "context object state is error.");
        DeviceType deviceType = static_cast<DeviceType>(context->deviceType);
        context->intValue = napiAudioRenderer->audioRenderer_->SetDefaultOutputDevice(deviceType);
        if (context->intValue != SUCCESS) {
            context->SignError(NAPI_ERR_ILLEGAL_STATE);
        }
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SetDefaultOutputDevice", executor, complete);
}

napi_value NapiAudioRenderer::RegisterCallback(napi_env env, napi_value jsThis,
    napi_value *argv, const std::string &cbName)
{
    NapiAudioRenderer *napiRenderer = nullptr;
    napi_status status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napiRenderer));
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_SYSTEM),
        "status error");
    CHECK_AND_RETURN_RET_LOG(napiRenderer != nullptr, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_NO_MEMORY),
        "rendererNapi is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiRenderer->audioRenderer_ != nullptr, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_NO_MEMORY), "audioRenderer_ is nullptr");

    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    if (!cbName.compare(INTERRUPT_CALLBACK_NAME) ||
        !cbName.compare(AUDIO_INTERRUPT_CALLBACK_NAME) ||
        !cbName.compare(STATE_CHANGE_CALLBACK_NAME)) {
        result = RegisterRendererCallback(env, argv, cbName, napiRenderer);
    } else if (!cbName.compare(MARK_REACH_CALLBACK_NAME)) {
        result = RegisterPositionCallback(env, argv, cbName, napiRenderer);
    } else if (!cbName.compare(PERIOD_REACH_CALLBACK_NAME)) {
        result = RegisterPeriodPositionCallback(env, argv, cbName, napiRenderer);
    } else if (!cbName.compare(DATA_REQUEST_CALLBACK_NAME)) {
        result = RegisterDataRequestCallback(env, argv, cbName, napiRenderer);
    } else if (!cbName.compare(DEVICECHANGE_CALLBACK_NAME)) {
        RegisterRendererDeviceChangeCallback(env, argv, napiRenderer);
    } else if (cbName == OUTPUT_DEVICECHANGE_WITH_INFO) {
        RegisterRendererOutputDeviceChangeWithInfoCallback(env, argv, napiRenderer);
    } else if (!cbName.compare(WRITE_DATA_CALLBACK_NAME)) {
        RegisterRendererWriteDataCallback(env, argv, cbName, napiRenderer);
    } else {
        bool unknownCallback = true;
        CHECK_AND_RETURN_RET_LOG(!unknownCallback, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERROR_INVALID_PARAM,
            "parameter verification failed: The param of type is not supported"), "audioRenderer_ is nullptr");
    }

    return result;
}

napi_value NapiAudioRenderer::UnregisterCallback(napi_env env, napi_value jsThis, size_t argc, napi_value *argv,
    const std::string &cbName)
{
    NapiAudioRenderer *napiRenderer = nullptr;
    napi_status status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napiRenderer));
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_SYSTEM),
        "sttaus error");
    CHECK_AND_RETURN_RET_LOG(napiRenderer != nullptr, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_NO_MEMORY),
        "napiRenderer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiRenderer->audioRenderer_ != nullptr, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_NO_MEMORY), "audioRenderer_ is nullptr");

    if (!cbName.compare(MARK_REACH_CALLBACK_NAME)) {
        napiRenderer->audioRenderer_->UnsetRendererPositionCallback();
        napiRenderer->positionCbNapi_ = nullptr;
    } else if (!cbName.compare(PERIOD_REACH_CALLBACK_NAME)) {
        napiRenderer->audioRenderer_->UnsetRendererPeriodPositionCallback();
        napiRenderer->periodPositionCbNapi_ = nullptr;
    } else if (!cbName.compare(DEVICECHANGE_CALLBACK_NAME)) {
        UnregisterRendererDeviceChangeCallback(env, argc, argv, napiRenderer);
    } else if (!cbName.compare(AUDIO_INTERRUPT_CALLBACK_NAME)) {
        UnregisterRendererCallback(env, cbName, napiRenderer);
    } else if (cbName == OUTPUT_DEVICECHANGE_WITH_INFO) {
        UnregisterRendererOutputDeviceChangeWithInfoCallback(env, argc, argv, napiRenderer);
    } else if (!cbName.compare(WRITE_DATA_CALLBACK_NAME)) {
        UnregisterRendererWriteDataCallback(env, argc, argv, napiRenderer);
    } else {
        bool unknownCallback = true;
        CHECK_AND_RETURN_RET_LOG(!unknownCallback, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
            "parameter verification failed: The param of type is not supported"), "cbName is invaild");
    }

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value NapiAudioRenderer::RegisterRendererCallback(napi_env env, napi_value *argv,
    const std::string &cbName, NapiAudioRenderer *napiRenderer)
{
    CHECK_AND_RETURN_RET_LOG(napiRenderer->callbackNapi_ != nullptr, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_NO_MEMORY), "callbackNapi_ is nullptr");

    std::shared_ptr<NapiAudioRendererCallback> cb =
        std::static_pointer_cast<NapiAudioRendererCallback>(napiRenderer->callbackNapi_);
    cb->SaveCallbackReference(cbName, argv[PARAM1]);

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value NapiAudioRenderer::RegisterPositionCallback(napi_env env, napi_value *argv,
    const std::string &cbName, NapiAudioRenderer *napiRenderer)
{
    int64_t markPosition = 0;
    NapiParamUtils::GetValueInt64(env, markPosition, argv[PARAM1]);

    CHECK_AND_RETURN_RET_LOG(markPosition > 0, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
        "parameter verification failed: The param of frame is not supported"), "Mark Position value not supported!!");
    napiRenderer->positionCbNapi_ = std::make_shared<NapiRendererPositionCallback>(env);
    CHECK_AND_RETURN_RET_LOG(napiRenderer->positionCbNapi_ != nullptr, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_NO_MEMORY), "positionCbNapi_ is nullptr");
    int32_t ret = napiRenderer->audioRenderer_->SetRendererPositionCallback(markPosition,
        napiRenderer->positionCbNapi_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_SYSTEM),
        "SetRendererPositionCallback fail");

    std::shared_ptr<NapiRendererPositionCallback> cb =
        std::static_pointer_cast<NapiRendererPositionCallback>(napiRenderer->positionCbNapi_);
    cb->SaveCallbackReference(cbName, argv[PARAM2]);

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value NapiAudioRenderer::RegisterPeriodPositionCallback(napi_env env, napi_value *argv,
    const std::string &cbName, NapiAudioRenderer *napiRenderer)
{
    int64_t frameCount = 0;
    NapiParamUtils::GetValueInt64(env, frameCount, argv[PARAM1]);

    if (frameCount > 0) {
        if (napiRenderer->periodPositionCbNapi_ == nullptr) {
            napiRenderer->periodPositionCbNapi_ = std::make_shared<NapiRendererPeriodPositionCallback>(env);
            CHECK_AND_RETURN_RET_LOG(napiRenderer->periodPositionCbNapi_ != nullptr,
                NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_NO_MEMORY),
                "periodPositionCbNapi_ is nullptr, No memery");

            int32_t ret = napiRenderer->audioRenderer_->SetRendererPeriodPositionCallback(frameCount,
                napiRenderer->periodPositionCbNapi_);
            CHECK_AND_RETURN_RET_LOG(ret == SUCCESS,
                NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_SYSTEM), "SetRendererPeriodPositionCallback failed");

            std::shared_ptr<NapiRendererPeriodPositionCallback> cb =
                std::static_pointer_cast<NapiRendererPeriodPositionCallback>(napiRenderer->periodPositionCbNapi_);
            cb->SaveCallbackReference(cbName, argv[PARAM2]);
        } else {
            AUDIO_DEBUG_LOG("periodReach already subscribed.");
        }
    } else {
        AUDIO_ERR_LOG("frameCount value not supported!!");
    }

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value NapiAudioRenderer::RegisterDataRequestCallback(napi_env env, napi_value *argv,
    const std::string &cbName, NapiAudioRenderer *napiRenderer)
{
    CHECK_AND_RETURN_RET_LOG(napiRenderer->dataRequestCbNapi_ == nullptr,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_ILLEGAL_STATE), "dataRequest already subscribed.");

    napiRenderer->dataRequestCbNapi_ = std::make_shared<NapiRendererDataRequestCallback>(env, napiRenderer);
    napiRenderer->audioRenderer_->SetRenderMode(RENDER_MODE_CALLBACK);
    CHECK_AND_RETURN_RET_LOG(napiRenderer->dataRequestCbNapi_ != nullptr,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_NO_MEMORY), "dataRequestCbNapi_ is nullptr");
    int32_t ret = napiRenderer->audioRenderer_->SetRendererWriteCallback(napiRenderer->dataRequestCbNapi_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_SYSTEM), "SetRendererWriteCallback failed");
    std::shared_ptr<NapiRendererDataRequestCallback> cb =
        std::static_pointer_cast<NapiRendererDataRequestCallback>(napiRenderer->dataRequestCbNapi_);
    cb->SaveCallbackReference(cbName, argv[PARAM1]);

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

void NapiAudioRenderer::RegisterRendererDeviceChangeCallback(napi_env env, napi_value *argv,
    NapiAudioRenderer *napiRenderer)
{
    if (!napiRenderer->rendererDeviceChangeCallbackNapi_) {
        napiRenderer->rendererDeviceChangeCallbackNapi_ = std::make_shared<NapiAudioRendererDeviceChangeCallback>(env);
        CHECK_AND_RETURN_LOG(napiRenderer->rendererDeviceChangeCallbackNapi_ != nullptr,
            "rendererDeviceChangeCallbackNapi_ is nullptr, No memery");

        int32_t ret = napiRenderer->audioRenderer_->RegisterOutputDeviceChangeWithInfoCallback(
            napiRenderer->rendererDeviceChangeCallbackNapi_);
        CHECK_AND_RETURN_LOG(ret == SUCCESS,
            "Registering of Renderer Device Change Callback Failed");
    }

    if (!napiRenderer->rendererPolicyServiceDiedCallbackNapi_) {
        napiRenderer->rendererPolicyServiceDiedCallbackNapi_ =
            std::make_shared<NapiAudioRendererPolicyServiceDiedCallback>(napiRenderer);
        CHECK_AND_RETURN_LOG(napiRenderer->rendererPolicyServiceDiedCallbackNapi_ != nullptr,
            "Registering of Renderer Device Change Callback Failed");

        int32_t ret = napiRenderer->audioRenderer_->RegisterAudioPolicyServerDiedCb(getpid(),
            napiRenderer->rendererPolicyServiceDiedCallbackNapi_);
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "Registering of AudioPolicyService Died Change Callback Failed");
    }

    std::shared_ptr<NapiAudioRendererDeviceChangeCallback> cb =
        std::static_pointer_cast<NapiAudioRendererDeviceChangeCallback>(
        napiRenderer->rendererDeviceChangeCallbackNapi_);
    cb->AddCallbackReference(argv[PARAM1]);
    AUDIO_INFO_LOG("RegisterRendererStateChangeCallback is successful");
}

void NapiAudioRenderer::UnregisterRendererCallback(napi_env env,
    const std::string& cbName, NapiAudioRenderer *napiRenderer)
{
    CHECK_AND_RETURN_LOG(napiRenderer->callbackNapi_ != nullptr, "napiRendererCallback is nullptr");

    std::shared_ptr<NapiAudioRendererCallback> cb =
        std::static_pointer_cast<NapiAudioRendererCallback>(napiRenderer->callbackNapi_);
    cb->RemoveCallbackReference(cbName);
}

void NapiAudioRenderer::UnregisterRendererDeviceChangeCallback(napi_env env, size_t argc, const napi_value *argv,
    NapiAudioRenderer *napiRenderer)
{
    napi_value callback = nullptr;

    if (argc == ARGS_TWO) {
        callback = argv[PARAM1];
    }
    CHECK_AND_RETURN_LOG(napiRenderer->rendererDeviceChangeCallbackNapi_ != nullptr,
        "rendererDeviceChangeCallbackNapi_ is nullptr, return");

    CHECK_AND_RETURN_LOG(napiRenderer->rendererPolicyServiceDiedCallbackNapi_ != nullptr,
        "rendererPolicyServiceDiedCallbackNapi_ is nullptr, return");

    std::shared_ptr<NapiAudioRendererDeviceChangeCallback> cb =
        std::static_pointer_cast<NapiAudioRendererDeviceChangeCallback>(
            napiRenderer->rendererDeviceChangeCallbackNapi_);
    cb->RemoveCallbackReference(env, callback);

    if (callback == nullptr || cb->GetCallbackListSize() == 0) {
        int32_t ret = napiRenderer->audioRenderer_->UnregisterOutputDeviceChangeWithInfoCallback(cb);
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "Unregistering of Renderer devuce Change Callback Failed");

        ret = napiRenderer->audioRenderer_->UnregisterAudioPolicyServerDiedCb(getpid());
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "UnregisterAudioPolicyServerDiedCb Failed");

        napiRenderer->DestroyNAPICallbacks();
    }
    AUDIO_INFO_LOG("UnegisterRendererDeviceChangeCallback is successful");
}

void NapiAudioRenderer::RegisterRendererOutputDeviceChangeWithInfoCallback(napi_env env, napi_value *argv,
    NapiAudioRenderer *napiRenderer)
{
    if (!napiRenderer->rendererOutputDeviceChangeWithInfoCallbackNapi_) {
        napiRenderer->rendererOutputDeviceChangeWithInfoCallbackNapi_
            = std::make_shared<NapiAudioRendererOutputDeviceChangeWithInfoCallback>(env);
        CHECK_AND_RETURN_LOG(napiRenderer->rendererOutputDeviceChangeWithInfoCallbackNapi_ != nullptr,
            "rendererOutputDeviceChangeWithInfoCallbackNapi_ is nullptr, No memery");

        int32_t ret = napiRenderer->audioRenderer_->RegisterOutputDeviceChangeWithInfoCallback(
            napiRenderer->rendererOutputDeviceChangeWithInfoCallbackNapi_);
        CHECK_AND_RETURN_LOG(ret == SUCCESS,
            "Registering of Renderer Device Change Callback Failed");
    }

    if (!napiRenderer->rendererPolicyServiceDiedCallbackNapi_) {
        napiRenderer->rendererPolicyServiceDiedCallbackNapi_ =
            std::make_shared<NapiAudioRendererPolicyServiceDiedCallback>(napiRenderer);
        CHECK_AND_RETURN_LOG(napiRenderer->rendererPolicyServiceDiedCallbackNapi_ != nullptr,
            "Registering of Renderer Device Change Callback Failed");

        int32_t ret = napiRenderer->audioRenderer_->RegisterAudioPolicyServerDiedCb(getpid(),
            napiRenderer->rendererPolicyServiceDiedCallbackNapi_);
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "Registering of AudioPolicyService Died Change Callback Failed");
    }

    std::shared_ptr<NapiAudioRendererOutputDeviceChangeWithInfoCallback> cb =
        napiRenderer->rendererOutputDeviceChangeWithInfoCallbackNapi_;
    cb->AddCallbackReference(argv[PARAM1]);
    AUDIO_INFO_LOG("Register Callback is successful");
}

void NapiAudioRenderer::UnregisterRendererOutputDeviceChangeWithInfoCallback(napi_env env, size_t argc,
    const napi_value *argv, NapiAudioRenderer *napiRenderer)
{
    napi_value callback = nullptr;

    if (argc == ARGS_TWO) {
        callback = argv[PARAM1];
    }
    CHECK_AND_RETURN_LOG(napiRenderer->rendererOutputDeviceChangeWithInfoCallbackNapi_ != nullptr,
        "rendererDeviceChangeCallbackNapi_ is nullptr, return");

    CHECK_AND_RETURN_LOG(napiRenderer->rendererPolicyServiceDiedCallbackNapi_ != nullptr,
        "rendererPolicyServiceDiedCallbackNapi_ is nullptr, return");

    std::shared_ptr<NapiAudioRendererOutputDeviceChangeWithInfoCallback> cb =
        napiRenderer->rendererOutputDeviceChangeWithInfoCallbackNapi_;
    cb->RemoveCallbackReference(env, callback);

    if (callback == nullptr || cb->GetCallbackListSize() == 0) {
        int32_t ret = napiRenderer->audioRenderer_->UnregisterOutputDeviceChangeWithInfoCallback(cb);
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "Unregistering of Renderer devuce Change Callback Failed");

        ret = napiRenderer->audioRenderer_->UnregisterAudioPolicyServerDiedCb(getpid());
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "UnregisterAudioPolicyServerDiedCb Failed");

        napiRenderer->DestroyNAPICallbacks();
    }
    AUDIO_INFO_LOG("Unregister Callback is successful");
}

void NapiAudioRenderer::RegisterRendererWriteDataCallback(napi_env env, napi_value *argv,
    const std::string &cbName, NapiAudioRenderer *napiRenderer)
{
    if (napiRenderer->rendererWriteDataCallbackNapi_ != nullptr) {
        AUDIO_WARNING_LOG("writeData already subscribed. The old writeData function will be overwritten.");
    }

    napiRenderer->rendererWriteDataCallbackNapi_ = std::make_shared<NapiRendererWriteDataCallback>(env, napiRenderer);
    napiRenderer->audioRenderer_->SetRenderMode(RENDER_MODE_CALLBACK);
    CHECK_AND_RETURN_LOG(napiRenderer->rendererWriteDataCallbackNapi_ != nullptr, "writeDataCbNapi_ is nullpur");
    int32_t ret = napiRenderer->audioRenderer_->SetRendererWriteCallback(napiRenderer->rendererWriteDataCallbackNapi_);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "SetRendererWriteCallback failed");
    std::shared_ptr<NapiRendererWriteDataCallback> cb =
        std::static_pointer_cast<NapiRendererWriteDataCallback>(napiRenderer->rendererWriteDataCallbackNapi_);
    cb->AddCallbackReference(cbName, argv[PARAM1]);

    AUDIO_INFO_LOG("Register Callback is successful");
}

void NapiAudioRenderer::UnregisterRendererWriteDataCallback(napi_env env, size_t argc, const napi_value *argv,
    NapiAudioRenderer *napiRenderer)
{
    napi_value callback = nullptr;

    if (argc == ARGS_TWO) {
        callback = argv[PARAM1];
    }
    CHECK_AND_RETURN_LOG(napiRenderer->rendererWriteDataCallbackNapi_ != nullptr,
        "napiRendererWriteDataCallback is nullptr, return");

    std::shared_ptr<NapiRendererWriteDataCallback> cb =
        std::static_pointer_cast<NapiRendererWriteDataCallback>(napiRenderer->rendererWriteDataCallbackNapi_);
    cb->RemoveCallbackReference(env, callback);

    AUDIO_INFO_LOG("Unregister Callback is successful");
}

void NapiAudioRenderer::DestroyCallbacks()
{
    CHECK_AND_RETURN_LOG(rendererDeviceChangeCallbackNapi_ != nullptr, "rendererDeviceChangeCallbackNapi_ is nullptr");
    rendererDeviceChangeCallbackNapi_->RemoveAllCallbacks();
    DestroyNAPICallbacks();
}

void NapiAudioRenderer::DestroyNAPICallbacks()
{
    if (rendererDeviceChangeCallbackNapi_ != nullptr) {
        rendererDeviceChangeCallbackNapi_.reset();
        rendererDeviceChangeCallbackNapi_ = nullptr;
    }

    if (rendererPolicyServiceDiedCallbackNapi_ != nullptr) {
        rendererPolicyServiceDiedCallbackNapi_.reset();
        rendererPolicyServiceDiedCallbackNapi_ = nullptr;
    }
}
} // namespace AudioStandard
} // namespace OHOS
