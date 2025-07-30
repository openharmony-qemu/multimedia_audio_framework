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
#define LOG_TAG "NapiAudioCapturer"
#endif

#include "napi_audio_capturer.h"
#ifdef FEATURE_HIVIEW_ENABLE
#if !defined(ANDROID_PLATFORM) && !defined(IOS_PLATFORM)
#include "xpower_event_js.h"
#endif
#endif
#include "audio_errors.h"
#include "napi_audio_error.h"
#include "napi_param_utils.h"
#include "napi_audio_enum.h"
#include "napi_audio_capturer_callbacks.h"
#include "napi_capturer_position_callback.h"
#include "napi_capturer_period_position_callback.h"
#include "napi_audio_capturer_read_data_callback.h"
#include "napi_audio_capturer_device_change_callback.h"
#include "napi_audio_capturer_info_change_callback.h"

namespace OHOS {
namespace AudioStandard {
static __thread napi_ref g_capturerConstructor = nullptr;
std::unique_ptr<AudioCapturerOptions> NapiAudioCapturer::sCapturerOptions_ = nullptr;
mutex NapiAudioCapturer::createMutex_;
int32_t NapiAudioCapturer::isConstructSuccess_ = SUCCESS;

NapiAudioCapturer::NapiAudioCapturer()
    : audioCapturer_(nullptr),  sourceType_(SOURCE_TYPE_MIC), env_(nullptr) {}

NapiAudioCapturer::~NapiAudioCapturer()
{
    if (audioCapturer_ != nullptr) {
        bool ret = audioCapturer_->Release();
        CHECK_AND_RETURN_LOG(ret, "AudioCapturer release fail");
        audioCapturer_ = nullptr;
        AUDIO_INFO_LOG("Proactively release audioCapturer");
    }
}

void NapiAudioCapturer::Destructor(napi_env env, void *nativeObject, void *finalizeHint)
{
    if (nativeObject == nullptr) {
        AUDIO_WARNING_LOG("Native object is null");
        return;
    }
    auto obj = static_cast<NapiAudioCapturer *>(nativeObject);
    if (obj != nullptr && obj->capturerReadDataCallbackNapi_ != nullptr) {
        std::shared_ptr<NapiCapturerReadDataCallback> cb =
            std::static_pointer_cast<NapiCapturerReadDataCallback>(obj->capturerReadDataCallbackNapi_);
        cb->RemoveNapiCapturer();
    }
    ObjectRefMap<NapiAudioCapturer>::DecreaseRef(obj);
    AUDIO_INFO_LOG("Decrease obj count");
}

napi_status NapiAudioCapturer::InitAudioCapturer(napi_env env, napi_value &constructor)
{
    AUDIO_DEBUG_LOG("NapiAudioCapturer::InitAudioCapturer()");
    napi_property_descriptor audio_capturer_properties[] = {
        DECLARE_NAPI_FUNCTION("getCapturerInfo", GetCapturerInfo),
        DECLARE_NAPI_FUNCTION("getCapturerInfoSync", GetCapturerInfoSync),
        DECLARE_NAPI_FUNCTION("getStreamInfo", GetStreamInfo),
        DECLARE_NAPI_FUNCTION("getStreamInfoSync", GetStreamInfoSync),
        DECLARE_NAPI_FUNCTION("start", Start),
        DECLARE_NAPI_FUNCTION("read", Read),
        DECLARE_NAPI_FUNCTION("getAudioTime", GetAudioTime),
        DECLARE_NAPI_FUNCTION("getAudioTimeSync", GetAudioTimeSync),
        DECLARE_NAPI_FUNCTION("stop", Stop),
        DECLARE_NAPI_FUNCTION("release", Release),
        DECLARE_NAPI_FUNCTION("getBufferSize", GetBufferSize),
        DECLARE_NAPI_FUNCTION("getBufferSizeSync", GetBufferSizeSync),
        DECLARE_NAPI_FUNCTION("getAudioStreamId", GetAudioStreamId),
        DECLARE_NAPI_FUNCTION("getAudioStreamIdSync", GetAudioStreamIdSync),
        DECLARE_NAPI_FUNCTION("getCurrentInputDevices", GetCurrentInputDevices),
        DECLARE_NAPI_FUNCTION("getCurrentAudioCapturerChangeInfo", GetCurrentAudioCapturerChangeInfo),
        DECLARE_NAPI_FUNCTION("getCurrentMicrophones", GetCurrentMicrophones),
        DECLARE_NAPI_FUNCTION("getOverflowCount", GetOverflowCount),
        DECLARE_NAPI_FUNCTION("setInputDeviceToAccessory", SetInputDeviceToAccessory),
        DECLARE_NAPI_FUNCTION("getOverflowCountSync", GetOverflowCountSync),
        DECLARE_NAPI_FUNCTION("getAudioTimestampInfo", GetAudioTimestampInfo),
        DECLARE_NAPI_FUNCTION("getAudioTimestampInfoSync", GetAudioTimestampInfoSync),
        DECLARE_NAPI_FUNCTION("setWillMuteWhenInterrupted", SetWillMuteWhenInterrupted),
        DECLARE_NAPI_FUNCTION("on", On),
        DECLARE_NAPI_FUNCTION("off", Off),
        DECLARE_NAPI_GETTER("state", GetState),
    };

    napi_status status = napi_define_class(env, NAPI_AUDIO_CAPTURER_CLASS_NAME.c_str(), NAPI_AUTO_LENGTH, Construct,
        nullptr, sizeof(audio_capturer_properties) / sizeof(audio_capturer_properties[PARAM0]),
        audio_capturer_properties, &constructor);
    return status;
}

napi_value NapiAudioCapturer::Init(napi_env env, napi_value exports)
{
    napi_value constructor;
    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    napi_property_descriptor static_prop[] = {
        DECLARE_NAPI_STATIC_FUNCTION("createAudioCapturer", CreateAudioCapturer),
        DECLARE_NAPI_STATIC_FUNCTION("createAudioCapturerSync", CreateAudioCapturerSync)
    };

    napi_status status = InitAudioCapturer(env, constructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_define_class fail");
    status = napi_create_reference(env, constructor, REFERENCE_CREATION_COUNT, &g_capturerConstructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_create_reference fail");
    status = napi_set_named_property(env, exports, NAPI_AUDIO_CAPTURER_CLASS_NAME.c_str(), constructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_set_named_property fail");
    status = napi_define_properties(env, exports, sizeof(static_prop) / sizeof(static_prop[0]), static_prop);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "Failure in NapiAudioCapturer::Init()");

    return exports;
}

unique_ptr<NapiAudioCapturer> NapiAudioCapturer::CreateAudioCapturerNativeObject(napi_env env)
{
    unique_ptr<NapiAudioCapturer> napiCapturer = make_unique<NapiAudioCapturer>();
    CHECK_AND_RETURN_RET_LOG(napiCapturer != nullptr, nullptr, "No memory");

    napiCapturer->env_ = env;
    napiCapturer->sourceType_ = sCapturerOptions_->capturerInfo.sourceType;

    AudioCapturerOptions capturerOptions = *sCapturerOptions_;

    /* NapiAudioCapturer not support other capturerFlags, only support flag 0 */
    if (capturerOptions.capturerInfo.capturerFlags != 0) {
        capturerOptions.capturerInfo.capturerFlags = 0;
    }
    capturerOptions.capturerInfo.recorderType = RECORDER_TYPE_ARKTS_AUDIO_RECORDER;
#if !defined(ANDROID_PLATFORM) && !defined(IOS_PLATFORM)
    napiCapturer->audioCapturer_ = AudioCapturer::CreateCapturer(capturerOptions);
#else
    std::string cacheDir = "/data/storage/el2/base/temp";
    napiCapturer->audioCapturer_ = AudioCapturer::Create(capturerOptions, cacheDir);
#endif
    if (napiCapturer->audioCapturer_ == nullptr) {
        AUDIO_ERR_LOG("Capturer Create failed");
        NapiAudioCapturer::isConstructSuccess_ = NAPI_ERR_SYSTEM;
        napiCapturer.release();
        return nullptr;
    }
    ObjectRefMap<NapiAudioCapturer>::Insert(napiCapturer.get());

    if (napiCapturer->audioCapturer_ != nullptr && napiCapturer->callbackNapi_ == nullptr) {
        napiCapturer->callbackNapi_ = std::make_shared<NapiAudioCapturerCallback>(env);
        CHECK_AND_RETURN_RET_LOG(napiCapturer->callbackNapi_ != nullptr, napiCapturer, "No memory");
        int32_t ret = napiCapturer->audioCapturer_->SetCapturerCallback(napiCapturer->callbackNapi_);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, napiCapturer, "Construct SetCapturerCallback failed");
    }
    return napiCapturer;
}

napi_value NapiAudioCapturer::GetCallback(size_t argc, napi_value *argv)
{
    napi_value callback = nullptr;

    if (argc == ARGS_TWO) {
        callback = argv[PARAM1];
    }
    return callback;
}

template <typename T>
static void GetCapturerNapiCallback(napi_value callback, const std::string &cbName,
    std::list<std::shared_ptr<NapiAudioCapturerCallbackInner>> audioCapturerCallbacks, std::shared_ptr<T> *cb)
{
    if (audioCapturerCallbacks.size() == 0) {
        AUDIO_ERR_LOG("no callback to get");
        return;
    }
    for (auto &iter:audioCapturerCallbacks) {
        if (!iter->CheckIfTargetCallbackName(cbName)) {
            continue;
        }
        std::shared_ptr<T> temp = std::static_pointer_cast<T>(iter);
        if (temp->ContainSameJsCallbackInner(cbName, callback)) {
            *cb = temp;
            return;
        }
    }
}

template <typename T>
static void UnregisterAudioCapturerSingletonCallbackTemplate(napi_env env, napi_value callback,
    const std::string &cbName, std::shared_ptr<T> cb,
    std::function<int32_t(std::shared_ptr<T> callbackPtr, napi_value callback)> removeFunction = nullptr)
{
    if (callback != nullptr) {
        CHECK_AND_RETURN_LOG(cb->ContainSameJsCallbackInner(cbName, callback), "napi_get_cb_info failed");
    }

    cb->RemoveCallbackReference(cbName, env, callback);

    if (removeFunction == nullptr) {
        return;
    }
    int32_t ret = removeFunction(cb, callback);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "Unset of Capturer callback failed");
    return;
}

template<typename T>
static void UnregisterAudioCapturerCallbackTemplate(napi_env env, napi_value callback,
    NapiAudioCapturer *napiCapturer, const std::string &cbName,
    std::function<int32_t(std::shared_ptr<T> callbackPtr, napi_value callback)> removeFunction)
{
    if (callback != nullptr) {
        std::shared_ptr<T> cb = nullptr;
        GetCapturerNapiCallback(callback, cbName, napiCapturer->audioCapturerCallbacks_, &cb);
        CHECK_AND_RETURN_LOG(cb != nullptr, "CapturerNapiCallback is null");
        UnregisterAudioCapturerSingletonCallbackTemplate(env, callback, cbName, cb, removeFunction);
        return;
    }
    
    auto isPresent = [&env, &callback, &cbName, &removeFunction]
        (std::shared_ptr<NapiAudioCapturerCallbackInner> &iter) {
            if (!iter->CheckIfTargetCallbackName(cbName)) {
                return false;
            }
            std::shared_ptr<T> cb = std::static_pointer_cast<T>(iter);
            UnregisterAudioCapturerSingletonCallbackTemplate(env, callback, cbName, cb, removeFunction);
            return true;
        };
    napiCapturer->audioCapturerCallbacks_.remove_if(isPresent);
    AUDIO_DEBUG_LOG("UnregisterAudioCapturerCallback success");
}

napi_value NapiAudioCapturer::Construct(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    size_t argCount = ARGS_TWO;
    napi_value thisVar = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argCount, nullptr, &thisVar, nullptr);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "failed to napi_get_cb_info");

    unique_ptr<NapiAudioCapturer> napiCapturer = CreateAudioCapturerNativeObject(env);
    CHECK_AND_RETURN_RET_LOG(napiCapturer != nullptr, result, "failed to CreateAudioCapturerNativeObject");
    status = napi_wrap(env, thisVar, static_cast<void*>(napiCapturer.get()),
        NapiAudioCapturer::Destructor, nullptr, nullptr);
    if (status != napi_ok) {
        ObjectRefMap<NapiAudioCapturer>::Erase(napiCapturer.get());
        return result;
    }

    napiCapturer.release();
    return thisVar;
}

bool NapiAudioCapturer::CheckContextStatus(std::shared_ptr<AudioCapturerAsyncContext> context)
{
    CHECK_AND_RETURN_RET_LOG(context != nullptr, false, "CHECK_AND_RETURN_RET_LOG");
    if (context->native == nullptr) {
        context->SignError(NAPI_ERR_SYSTEM);
        return false;
    }
    return true;
}

bool NapiAudioCapturer::CheckAudioCapturerStatus(NapiAudioCapturer *napi,
    std::shared_ptr<AudioCapturerAsyncContext> context)
{
    CHECK_AND_RETURN_RET_LOG(napi != nullptr, false, "CHECK_AND_RETURN_RET_LOG");
    if (napi->audioCapturer_ == nullptr) {
        context->SignError(NAPI_ERR_SYSTEM);
        return false;
    }
    return true;
}

NapiAudioCapturer* NapiAudioCapturer::GetParamWithSync(const napi_env &env, napi_callback_info info,
    size_t &argc, napi_value *args)
{
    napi_status status;
    NapiAudioCapturer *napiAudioCapturer = nullptr;
    napi_value jsThis = nullptr;

    status = napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok && jsThis != nullptr, nullptr,
        "GetParamWithSync fail to napi_get_cb_info");

    status = napi_unwrap(env, jsThis, (void **)&napiAudioCapturer);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, nullptr, "napi_unwrap failed");
    CHECK_AND_RETURN_RET_LOG((napiAudioCapturer != nullptr) && (napiAudioCapturer->audioCapturer_ != nullptr),
        napiAudioCapturer, "GetParamWithSync fail to napi_unwrap");

    return napiAudioCapturer;
}

napi_value NapiAudioCapturer::CreateAudioCapturerWrapper(napi_env env, const AudioCapturerOptions capturerOptions)
{
    lock_guard<mutex> lock(createMutex_);
    napi_value result = nullptr;
    napi_value constructor = nullptr;

    napi_status status = napi_get_reference_value(env, g_capturerConstructor, &constructor);
    if (status != napi_ok) {
        AUDIO_ERR_LOG("napi_get_reference_value failed");
        goto fail;
    }
    if (sCapturerOptions_ != nullptr) {
        sCapturerOptions_.release();
    }
    sCapturerOptions_ = make_unique<AudioCapturerOptions>();
    CHECK_AND_RETURN_RET_LOG(sCapturerOptions_ != nullptr, result, "sCapturerOptions create failed");
    *sCapturerOptions_ = capturerOptions;
    status = napi_new_instance(env, constructor, 0, nullptr, &result);
    if (status != napi_ok) {
        AUDIO_ERR_LOG("napi_new_instance failed");
        goto fail;
    }
    return result;

fail:
    napi_get_undefined(env, &result);
    return result;
}

napi_value NapiAudioCapturer::CreateAudioCapturer(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioCapturerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("CreateAudioCapturer failed : no memory");
        NapiAudioError::ThrowError(env, "CreateAudioCapturer failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments",
            NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetCapturerOptions(env, &context->capturerOptions, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "CreateAudioCapturer failed",
            NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);

    auto complete = [env, context](napi_value &output) {
        output = CreateAudioCapturerWrapper(env, context->capturerOptions);
        if (NapiAudioCapturer::isConstructSuccess_ != SUCCESS) {
            context->SignError(NapiAudioCapturer::isConstructSuccess_);
            NapiAudioCapturer::isConstructSuccess_ = SUCCESS;
        }
    };

    return NapiAsyncWork::Enqueue(env, context, "CreateAudioCapturer", nullptr, complete);
}

napi_value NapiAudioCapturer::CreateAudioCapturerSync(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = {};
    napi_status status = NapiParamUtils::GetParam(env, info, argc, argv);
    CHECK_AND_RETURN_RET_LOG((status == napi_ok) && (argc == ARGS_ONE),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
        "mandatory parameters are left unspecified"), "invaild param");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_object, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID, "incorrect parameter types: The type of options must be number"),
        "invaild valueType");

    AudioCapturerOptions capturerOptions;
    CHECK_AND_RETURN_RET_LOG(NapiParamUtils::GetCapturerOptions(env, &capturerOptions, argv[PARAM0]) == napi_ok,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
        "parameter verification failed: The param of options must be interface AudioCapturerOptions"),
        "get captureOptions failed");

    return NapiAudioCapturer::CreateAudioCapturerWrapper(env, capturerOptions);
}

napi_value NapiAudioCapturer::GetCapturerInfo(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioCapturerAsyncContext>();
    if (context == nullptr) {
        NapiAudioError::ThrowError(env, NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioCapturer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioCapturer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioCapturerStatus(napiAudioCapturer, context),
            "context object state is error.");
        context->intValue = napiAudioCapturer->audioCapturer_->GetCapturerInfo(context->capturerInfo);
        if (context->intValue != SUCCESS) {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetCapturerInfo(env, context->capturerInfo, output);
    };

    return NapiAsyncWork::Enqueue(env, context, "GetCapturerInfo", executor, complete);
}

napi_value NapiAudioCapturer::GetCapturerInfoSync(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = PARAM0;

    auto *napiAudioCapturer = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(argc == PARAM0, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID), "argsCount invaild");

    AudioCapturerInfo capturerInfo;
    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer != nullptr, result, "napiAudioCapturer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer->audioCapturer_ != nullptr, result, "audioCapturer_ is nullptr");
    int32_t ret = napiAudioCapturer->audioCapturer_->GetCapturerInfo(capturerInfo);

    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, result, "GetCapturerInfoSync failure!");
    NapiParamUtils::SetCapturerInfo(env, capturerInfo, result);

    return result;
}

napi_value NapiAudioCapturer::GetStreamInfo(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioCapturerAsyncContext>();
    if (context == nullptr) {
        NapiAudioError::ThrowError(env, NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }
    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioCapturer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioCapturer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioCapturerStatus(napiAudioCapturer, context),
            "context object state is error.");
        context->intValue = napiAudioCapturer->audioCapturer_->GetStreamInfo(context->streamInfo);
        if (context->intValue != SUCCESS) {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetStreamInfo(env, context->streamInfo, output);
    };

    return NapiAsyncWork::Enqueue(env, context, "GetStreamInfo", executor, complete);
}

napi_value NapiAudioCapturer::GetStreamInfoSync(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = PARAM0;

    auto *napiAudioCapturer = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(argc == PARAM0,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID), "invaild param");

    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer != nullptr, result, "napiAudioCapturer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer->audioCapturer_ != nullptr, result, "audioCapturer_ is nullptr");

    AudioStreamInfo streamInfo;
    int32_t ret = napiAudioCapturer->audioCapturer_->GetStreamInfo(streamInfo);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, result, "GetStreamInfo failure!");

    NapiParamUtils::SetStreamInfo(env, streamInfo, result);
    return result;
}

napi_value NapiAudioCapturer::Start(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioCapturerAsyncContext>();
    if (context == nullptr) {
        NapiAudioError::ThrowError(env, NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }
    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioCapturer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioCapturer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioCapturerStatus(napiAudioCapturer, context),
            "context object state is error.");
        context->isTrue = napiAudioCapturer->audioCapturer_->Start();
        if (!context->isTrue) {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
#ifdef FEATURE_HIVIEW_ENABLE
#if !defined(ANDROID_PLATFORM) && !defined(IOS_PLATFORM)
    HiviewDFX::ReportXPowerJsStackSysEvent(env, "STREAM_CHANGE", "SRC=Audio");
#endif
#endif
    return NapiAsyncWork::Enqueue(env, context, "Start", executor, complete);
}

napi_status NapiAudioCapturer::ReadFromNative(shared_ptr<AudioCapturerAsyncContext> context)
{
    napi_status status = napi_generic_failure;
    CHECK_AND_RETURN_RET_LOG(CheckContextStatus(context), napi_generic_failure, "context object state is error.");
    auto obj = reinterpret_cast<NapiAudioCapturer*>(context->native);
    ObjectRefMap objectGuard(obj);
    auto *napiAudioCapturer = objectGuard.GetPtr();
    CHECK_AND_RETURN_RET_LOG(CheckAudioCapturerStatus(napiAudioCapturer, context), napi_generic_failure,
        "context object state is error.");
    uint32_t userSize = context->userSize;
    uint8_t *buffer = new uint8_t[userSize];
    CHECK_AND_RETURN_RET_LOG(buffer != nullptr, status, "buffer malloc failed,no memery");
    int32_t bytesRead = 0;
    while (static_cast<uint32_t>(bytesRead) < context->userSize) {
        int32_t len = napiAudioCapturer->audioCapturer_->Read(*(buffer + bytesRead),
            userSize - bytesRead, context->isBlocking);
        if (len >= 0) {
            bytesRead += len;
        } else {
            bytesRead = len;
            break;
        }
    }
    if (bytesRead <= 0) {
        delete [] buffer;
        return status;
    }
    context->bytesRead = static_cast<size_t>(bytesRead);
    context->buffer = buffer;
    status = napi_ok;
    return status;
}

napi_value NapiAudioCapturer::SetInputDeviceToAccessory(napi_env env, napi_callback_info info)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED), "No system permission");
    auto context = std::make_shared<AudioCapturerAsyncContext>();
    if (context == nullptr) {
        NapiAudioError::ThrowError(env, "SetInputDeviceToAccessory failed : no memory",
            NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioCapturer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioCapturer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioCapturerStatus(napiAudioCapturer, context),
            "context object state is error.");
        context->intValue = napiAudioCapturer->audioCapturer_->SetInputDevice(DEVICE_TYPE_ACCESSORY);
        if (context->intValue != SUCCESS) {
            context->SignError(NAPI_ERR_ILLEGAL_STATE);
        }
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SetInputDeviceToAccessory", executor, complete);
}

napi_value NapiAudioCapturer::Read(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioCapturerAsyncContext>();
    if (context == nullptr) {
        NapiAudioError::ThrowError(env, NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }
    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments",
            NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueUInt32(env, context->userSize, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "GetValueUInt32 userSize failed",
            NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueBoolean(env, context->isBlocking, argv[PARAM1]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "GetValueUInt32 userSize failed",
            NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        context->status = ReadFromNative(context);
        if (context->status != napi_ok) {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::CreateArrayBuffer(env, context->bytesRead, context->buffer, output);
        delete [] context->buffer;
        context->buffer = nullptr;
    };

    return NapiAsyncWork::Enqueue(env, context, "Read", executor, complete);
}

napi_value NapiAudioCapturer::GetAudioTime(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioCapturerAsyncContext>();
    if (context == nullptr) {
        NapiAudioError::ThrowError(env, NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }
    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioCapturer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioCapturer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioCapturerStatus(napiAudioCapturer, context),
            "context object state is error.");
        Timestamp timestamp;
        if (napiAudioCapturer->audioCapturer_->GetAudioTime(timestamp, Timestamp::Timestampbase::MONOTONIC)) {
            const uint64_t secToNanosecond = 1000000000;
            context->time = static_cast<uint64_t>(timestamp.time.tv_nsec) +
                static_cast<uint64_t>(timestamp.time.tv_sec) * secToNanosecond;
        } else {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueInt64(env, static_cast<int64_t>(context->time), output);
    };

    return NapiAsyncWork::Enqueue(env, context, "GetAudioTime", executor, complete);
}

napi_value NapiAudioCapturer::GetAudioTimeSync(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = PARAM0;

    auto *napiAudioCapturer = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(argc == PARAM0,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID), "invaild param");

    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer != nullptr, result, "napiAudioCapturer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer->audioCapturer_ != nullptr, result, "audioCapturer_ is nullptr");

    Timestamp timestamp;
    bool ret = napiAudioCapturer->audioCapturer_->GetAudioTime(timestamp, Timestamp::Timestampbase::MONOTONIC);
    CHECK_AND_RETURN_RET_LOG(ret, result, "GetAudioTime failure!");
    const uint64_t secToNanosecond = 1000000000;
    uint64_t time = static_cast<uint64_t>(timestamp.time.tv_nsec) +
        static_cast<uint64_t>(timestamp.time.tv_sec) * secToNanosecond;

    NapiParamUtils::SetValueInt64(env, static_cast<int64_t>(time), result);
    return result;
}

napi_value NapiAudioCapturer::Stop(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioCapturerAsyncContext>();
    if (context == nullptr) {
        NapiAudioError::ThrowError(env, NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }
    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioCapturer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioCapturer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioCapturerStatus(napiAudioCapturer, context),
            "context object state is error.");
        context->isTrue = napiAudioCapturer->audioCapturer_->Stop();
        if (!context->isTrue) {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };

    return NapiAsyncWork::Enqueue(env, context, "Stop", executor, complete);
}

napi_value NapiAudioCapturer::Release(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioCapturerAsyncContext>();
    if (context == nullptr) {
        NapiAudioError::ThrowError(env, NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }
    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioCapturer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioCapturer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioCapturerStatus(napiAudioCapturer, context),
            "context object state is error.");
        context->isTrue = napiAudioCapturer->audioCapturer_->Release();
        if (!context->isTrue) {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };

    return NapiAsyncWork::Enqueue(env, context, "Release", executor, complete);
}

napi_value NapiAudioCapturer::GetBufferSize(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioCapturerAsyncContext>();
    if (context == nullptr) {
        NapiAudioError::ThrowError(env, NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }
    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioCapturer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioCapturer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioCapturerStatus(napiAudioCapturer, context),
            "context object state is error.");
        context->intValue = napiAudioCapturer->audioCapturer_->GetBufferSize(context->bufferSize);
        if (context->intValue != SUCCESS) {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueUInt32(env, context->bufferSize, output);
    };

    return NapiAsyncWork::Enqueue(env, context, "GetBufferSize", executor, complete);
}

napi_value NapiAudioCapturer::GetBufferSizeSync(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = PARAM0;

    auto *napiAudioCapturer = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(argc == PARAM0,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID), "invaild param");

    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer != nullptr, result, "napiAudioCapturer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer->audioCapturer_ != nullptr, result, "audioCapturer_ is nullptr");

    size_t bufferSize;
    int32_t ret = napiAudioCapturer->audioCapturer_->GetBufferSize(bufferSize);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, result, "GetBufferSize failure!");

    NapiParamUtils::SetValueUInt32(env, bufferSize, result);
    return result;
}

napi_value NapiAudioCapturer::GetAudioStreamId(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioCapturerAsyncContext>();
    if (context == nullptr) {
        NapiAudioError::ThrowError(env, NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }
    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioCapturer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioCapturer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioCapturerStatus(napiAudioCapturer, context),
            "context object state is error.");
        context->intValue = napiAudioCapturer->audioCapturer_->GetAudioStreamId(context->audioStreamId);
        if (context->intValue  == ERR_ILLEGAL_STATE) {
            context->SignError(NAPI_ERR_ILLEGAL_STATE);
        } else if (context->intValue  == ERR_INVALID_INDEX) {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueUInt32(env, context->audioStreamId, output);
    };

    return NapiAsyncWork::Enqueue(env, context, "GetAudioStreamId", executor, complete);
}

napi_value NapiAudioCapturer::GetAudioStreamIdSync(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = PARAM0;

    auto *napiAudioCapturer = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(argc == PARAM0,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID), "invaild param");

    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer != nullptr, result, "napiAudioCapturer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer->audioCapturer_ != nullptr, result, "audioCapturer_ is nullptr");

    uint32_t audioStreamId;
    int32_t ret = napiAudioCapturer->audioCapturer_->GetAudioStreamId(audioStreamId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, result, "GetAudioStreamId failure!");

    NapiParamUtils::SetValueUInt32(env, audioStreamId, result);
    return result;
}

napi_value NapiAudioCapturer::GetCurrentInputDevices(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = PARAM0;

    auto *napiAudioCapturer = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(argc == PARAM0,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID), "invaild param");

    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer != nullptr, result, "napiAudioCapturer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer->audioCapturer_ != nullptr, result, "audioCapturer_ is nullptr");

    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    int32_t ret = napiAudioCapturer->audioCapturer_->GetCurrentInputDevices(deviceInfo);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, result, "GetCurrentInputDevices failure!");

    NapiParamUtils::SetValueDeviceInfo(env, deviceInfo, result);
    return result;
}

napi_value NapiAudioCapturer::GetCurrentAudioCapturerChangeInfo(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = PARAM0;

    auto *napiAudioCapturer = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(argc == PARAM0,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID), "invaild param");

    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer != nullptr, result, "napiAudioCapturer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer->audioCapturer_ != nullptr, result, "audioCapturer_ is nullptr");

    AudioCapturerChangeInfo capturerInfo;
    int32_t ret = napiAudioCapturer->audioCapturer_->GetCurrentCapturerChangeInfo(capturerInfo);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, result, "GetCurrentCapturerChangeInfo failure!");

    NapiParamUtils::SetAudioCapturerChangeInfoDescriptors(env, capturerInfo, result);
    return result;
}

napi_value NapiAudioCapturer::GetCurrentMicrophones(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = PARAM0;

    auto *napiAudioCapturer = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(argc == PARAM0,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID), "invaild param");

    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer != nullptr, result, "napiAudioCapturer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer->audioCapturer_ != nullptr, result, "audioCapturer_ is nullptr");

    vector<sptr<MicrophoneDescriptor>> micDescs;
    micDescs = napiAudioCapturer->audioCapturer_->GetCurrentMicrophones();

    NapiParamUtils::SetMicrophoneDescriptors(env, micDescs, result);
    return result;
}

napi_value NapiAudioCapturer::GetState(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = PARAM0;

    auto *napiAudioCapturer = GetParamWithSync(env, info, argc, nullptr);
    if (argc > PARAM0) {
        NapiAudioError::ThrowError(env, NAPI_ERR_INPUT_INVALID);
        napi_get_undefined(env, &result);
        return result;
    }

    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer != nullptr, result, "napiAudioCapturer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer->audioCapturer_ != nullptr, result, "audioCapturer_ is nullptr");

    int32_t capturerState = napiAudioCapturer->audioCapturer_->GetStatus();

    napi_status status = NapiParamUtils::SetValueInt32(env, capturerState, result);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "SetValueInt32 capturerState fail");
    return result;
}

napi_value NapiAudioCapturer::GetOverflowCount(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioCapturerAsyncContext>();
    if (context == nullptr) {
        NapiAudioError::ThrowError(env, NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }
    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioCapturer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioCapturer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioCapturerStatus(napiAudioCapturer, context),
            "context object state is error.");
        context->overflowCount = napiAudioCapturer->audioCapturer_->GetOverflowCount();
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueUInt32(env, context->overflowCount, output);
    };

    return NapiAsyncWork::Enqueue(env, context, "GetOverflowCount", executor, complete);
}

napi_value NapiAudioCapturer::SetWillMuteWhenInterrupted(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioCapturerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("failed: no memory");
        NapiAudioError::ThrowError(env, "SetWillMuteWhenInterrupted failed : no memory", NAPI_ERR_ILLEGAL_STATE);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments",
            NAPI_ERR_INPUT_INVALID);
        context->status = NapiParamUtils::GetValueBoolean(env, context->muteWhenInterrupted, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get muteWhenInterrupted failed",
            NAPI_ERR_INPUT_INVALID);
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioCapturer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioCapturer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioCapturerStatus(napiAudioCapturer, context),
            "AudioCapturer state is error.");
        CHECK_AND_RETURN_LOG(napiAudioCapturer->audioCapturer_ != nullptr, "audioCapturer_ is nullptr");
        context->intValue = napiAudioCapturer->audioCapturer_->SetInterruptStrategy(context->muteWhenInterrupted ?
                InterruptStrategy::MUTE : InterruptStrategy::DEFAULT);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->intValue == SUCCESS, "SetWillMuteWhenInterrupted failed",
            NAPI_ERR_SYSTEM);
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SetWillMuteWhenInterrupted", executor, complete);
}

napi_value NapiAudioCapturer::GetOverflowCountSync(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = PARAM0;

    auto *napiAudioCapturer = GetParamWithSync(env, info, argc, nullptr);

    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer != nullptr, result, "napiAudioCapturer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer->audioCapturer_ != nullptr, result, "audioCapturer_ is nullptr");

    uint32_t overflowCount = napiAudioCapturer->audioCapturer_->GetOverflowCount();

    NapiParamUtils::SetValueUInt32(env, overflowCount, result);
    return result;
}

napi_value NapiAudioCapturer::GetAudioTimestampInfo(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioCapturerAsyncContext>();
    if (context == nullptr) {
        NapiAudioError::ThrowError(env, NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }
    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioCapturer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioCapturer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioCapturerStatus(napiAudioCapturer, context),
            "context object state is error.");
        int32_t ret = napiAudioCapturer->audioCapturer_->GetAudioTimestampInfo(context->timeStamp,
            Timestamp::Timestampbase::MONOTONIC);
        if (ret != SUCCESS) {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetTimeStampInfo(env, context->timeStamp, output);
    };

    return NapiAsyncWork::Enqueue(env, context, "GetAudioTimestampInfo", executor, complete);
}

napi_value NapiAudioCapturer::GetAudioTimestampInfoSync(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = PARAM0;

    auto *napiAudioCapturer = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(argc == PARAM0,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID), "invaild param");

    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer != nullptr, result, "napiAudioCapturer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer->audioCapturer_ != nullptr, result, "audioCapturer_ is nullptr");

    Timestamp timeStamp;
    int32_t ret = napiAudioCapturer->audioCapturer_->GetAudioTimestampInfo(timeStamp,
        Timestamp::Timestampbase::MONOTONIC);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, result, "GetAudioTimestampInfo failure!");
    NapiParamUtils::SetTimeStampInfo(env, timeStamp, result);
    return result;
}

napi_value NapiAudioCapturer::On(napi_env env, napi_callback_info info)
{
    const size_t requireArgc = ARGS_TWO;
    size_t argc = ARGS_THREE;

    napi_value argv[requireArgc + PARAM1] = {nullptr, nullptr, nullptr};
    napi_value jsThis = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &jsThis, nullptr);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_SYSTEM), "napi_get_cb_info failed");
    CHECK_AND_RETURN_RET_LOG(argc >= requireArgc, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID, "mandatory parameters are left unspecified"), "argc requeset failed");

    napi_valuetype eventType = napi_undefined;
    napi_typeof(env, argv[PARAM0], &eventType);
    CHECK_AND_RETURN_RET_LOG(eventType == napi_string, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID, "incorrect parameter types: The type of eventType must be string"),
        "eventType invaild");

    std::string callbackName = NapiParamUtils::GetStringArgument(env, argv[PARAM0]);
    AUDIO_DEBUG_LOG("NapiAudioCapturer: On callbackName: %{public}s", callbackName.c_str());

    napi_valuetype handler = napi_undefined;
    if (argc == requireArgc) {
        napi_typeof(env, argv[PARAM1], &handler);
        CHECK_AND_RETURN_RET_LOG(handler == napi_function,
            NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
            "incorrect parameter types: The type of callback must be function"), "handler invaild");
    } else {
        napi_valuetype paramArg1 = napi_undefined;
        napi_typeof(env, argv[PARAM1], &paramArg1);
        napi_valuetype expectedValType = napi_number;  // Default. Reset it with 'callbackName' if check, if required.
        CHECK_AND_RETURN_RET_LOG(paramArg1 == expectedValType,
            NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
            "incorrect parameter types: The type of frame must be number"), "paramArg1 invaild");

        napi_typeof(env, argv[PARAM2], &handler);
        CHECK_AND_RETURN_RET_LOG(handler == napi_function,
            NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
            "incorrect parameter types: The type of callback must be function"), "handler invaild");
    }

    return RegisterCallback(env, jsThis, argv, callbackName);
}

napi_value NapiAudioCapturer::RegisterCallback(napi_env env, napi_value jsThis,
    napi_value *argv, const std::string &cbName)
{
    NapiAudioCapturer *napiCapturer = nullptr;
    napi_status status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napiCapturer));

    CHECK_AND_RETURN_RET_LOG(status == napi_ok, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_SYSTEM), "napi_unwrap failed");
    CHECK_AND_RETURN_RET_LOG(napiCapturer != nullptr, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_NO_MEMORY), "napiCapturer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiCapturer->audioCapturer_ != nullptr, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_NO_MEMORY), "audioCapturer_ is nullptr");

    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    if (!cbName.compare(STATE_CHANGE_CALLBACK_NAME) ||
        !cbName.compare(AUDIO_INTERRUPT_CALLBACK_NAME)) {
        result = RegisterCapturerCallback(env, argv, cbName, napiCapturer);
    } else if (!cbName.compare(MARK_REACH_CALLBACK_NAME)) {
        result = RegisterPositionCallback(env, argv, cbName, napiCapturer);
    } else if (!cbName.compare(PERIOD_REACH_CALLBACK_NAME)) {
        result = RegisterPeriodPositionCallback(env, argv, cbName, napiCapturer);
    } else if (!cbName.compare(INPUTDEVICE_CHANGE_CALLBACK_NAME)) {
        RegisterAudioCapturerDeviceChangeCallback(env, argv, cbName, napiCapturer);
    } else if (!cbName.compare(AUDIO_CAPTURER_CHANGE_CALLBACK_NAME)) {
        RegisterAudioCapturerInfoChangeCallback(env, argv, cbName, napiCapturer);
    } else if (!cbName.compare(READ_DATA_CALLBACK_NAME)) {
        RegisterCapturerReadDataCallback(env, argv, cbName, napiCapturer);
    } else {
        bool unknownCallback = true;
        CHECK_AND_RETURN_RET_LOG(!unknownCallback, NapiAudioError::ThrowErrorAndReturn(env,
            NAPI_ERROR_INVALID_PARAM,
            "parameter verification failed: The param of type is not supported"), "audioCapturer_ is nullptr");
    }

    return result;
}

napi_value NapiAudioCapturer::RegisterCapturerCallback(napi_env env, napi_value *argv,
    const std::string &cbName, NapiAudioCapturer *napiCapturer)
{
    CHECK_AND_RETURN_RET_LOG(napiCapturer->callbackNapi_ != nullptr,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_NO_MEMORY), "napiCapturer is nullptr");

    std::shared_ptr<NapiAudioCapturerCallback> cb =
        std::static_pointer_cast<NapiAudioCapturerCallback>(napiCapturer->callbackNapi_);
    cb->SaveCallbackReference(cbName, argv[PARAM1]);
    if (cbName == INTERRUPT_CALLBACK_NAME || cbName == AUDIO_INTERRUPT_CALLBACK_NAME) {
        if (!cb->GetInterruptTsfnFlag()) {
            cb->CreateInterruptTsfn(env);
        }
    } else if (cbName == STATE_CHANGE_CALLBACK_NAME) {
        if (!cb->GetStateChangeTsfnFlag()) {
            cb->CreateStateChangeTsfn(env);
        }
    }

    if (!cbName.compare(STATE_CHANGE_CALLBACK_NAME)) {
        CapturerState state = napiCapturer->audioCapturer_->GetStatus();
        if (state == CAPTURER_PREPARED) {
            napiCapturer->callbackNapi_->OnStateChange(state);
        }
    }

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value NapiAudioCapturer::RegisterPositionCallback(napi_env env, napi_value *argv,
    const std::string &cbName, NapiAudioCapturer *napiCapturer)
{
    int64_t markPosition = 0;
    NapiParamUtils::GetValueInt64(env, markPosition, argv[PARAM1]);

    AUDIO_INFO_LOG("NapiAudioCapturer:RegisterPositionCallback start!");
    if (markPosition > 0) {
        napiCapturer->positionCbNapi_ = std::make_shared<NapiCapturerPositionCallback>(env);
        CHECK_AND_RETURN_RET_LOG(napiCapturer->positionCbNapi_ != nullptr,
            NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_NO_MEMORY), "positionCbNapi_ is nullptr");
        int32_t ret = napiCapturer->audioCapturer_->SetCapturerPositionCallback(markPosition,
            napiCapturer->positionCbNapi_);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS,
            NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_SYSTEM),
            "SetCapturerPositionCallback failed");

        std::shared_ptr<NapiCapturerPositionCallback> cb =
            std::static_pointer_cast<NapiCapturerPositionCallback>(napiCapturer->positionCbNapi_);
        cb->SaveCallbackReference(cbName, argv[PARAM2]);
        if (!cb->GetCapturePositionFlag()) {
            cb->CreateCapturePositionTsfn(env);
        }
    } else {
        AUDIO_ERR_LOG("NapiAudioCapturer: Mark Position value not supported!!");
        CHECK_AND_RETURN_RET_LOG(false, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
            "parameter verification failed: The param of frame is not supported"), "invailed callback");
    }

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value NapiAudioCapturer::RegisterPeriodPositionCallback(napi_env env, napi_value *argv, const std::string &cbName,
    NapiAudioCapturer *napiCapturer)
{
    int64_t frameCount = 0;
    napi_get_value_int64(env, argv[PARAM1], &frameCount);

    CHECK_AND_RETURN_RET_LOG(frameCount > 0, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
        "parameter verification failed: The param of frame is not supported"), "frameCount value not supported");
    
    CHECK_AND_RETURN_RET_LOG(napiCapturer->periodPositionCbNapi_ == nullptr,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_ILLEGAL_STATE),
        "periodReach already subscribed.");
    
    std::shared_ptr<NapiCapturerPeriodPositionCallback> cb = std::make_shared<NapiCapturerPeriodPositionCallback>(env);
    CHECK_AND_RETURN_RET_LOG(cb != nullptr,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_NO_MEMORY), "periodPositionCbNapi_ id nullptr");
    napiCapturer->periodPositionCbNapi_ = cb;

    int32_t ret = napiCapturer->audioCapturer_->SetCapturerPeriodPositionCallback(frameCount, cb);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_SYSTEM),
        "SetCapturerPeriodPositionCallback failed");
    
    cb->SaveCallbackReference(cbName, argv[PARAM2]);
    cb->CreatePeriodPositionTsfn(env);

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

void NapiAudioCapturer::RegisterAudioCapturerDeviceChangeCallback(napi_env env, napi_value *argv,
    const std::string &cbName, NapiAudioCapturer *napiCapturer)
{
    std::shared_ptr<NapiAudioCapturerDeviceChangeCallback> cb = nullptr;
    GetCapturerNapiCallback(argv[PARAM1], cbName, napiCapturer->audioCapturerCallbacks_, &cb);
    CHECK_AND_RETURN_LOG(cb == nullptr, "Do not register same capturer device callback!");

    cb = std::make_shared<NapiAudioCapturerDeviceChangeCallback>(env);
    CHECK_AND_RETURN_LOG(cb != nullptr, "Memory allocation failed!!");

    cb->SaveCallbackReference(cbName, argv[PARAM1]);
    cb->CreateCaptureDeviceChangeTsfn(env);
    int32_t ret =
        napiCapturer->audioCapturer_->SetAudioCapturerDeviceChangeCallback(cb);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "Registering of capturer device change callback failed");

    napiCapturer->audioCapturerCallbacks_.push_back(cb);

    AUDIO_DEBUG_LOG("RegisterAudioCapturerDeviceChangeCallback is successful");
}

void NapiAudioCapturer::RegisterAudioCapturerInfoChangeCallback(napi_env env, napi_value *argv,
    const std::string &cbName, NapiAudioCapturer *napiCapturer)
{
    std::shared_ptr<NapiAudioCapturerInfoChangeCallback> cb = nullptr;
    GetCapturerNapiCallback(argv[PARAM1], cbName, napiCapturer->audioCapturerCallbacks_, &cb);
    CHECK_AND_RETURN_LOG(cb == nullptr, "Do not register same capturer info change callback!");
    cb = std::make_shared<NapiAudioCapturerInfoChangeCallback>(env);

    CHECK_AND_RETURN_LOG(cb != nullptr, "Memory allocation failed!!");

    cb->SaveCallbackReference(cbName, argv[PARAM1]);
    cb->CreateCaptureInfoChangeTsfn(env);
    int32_t ret =
        napiCapturer->audioCapturer_->SetAudioCapturerInfoChangeCallback(cb);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "Registering of capturer info change callback failed");

    napiCapturer->audioCapturerCallbacks_.push_back(cb);

    AUDIO_DEBUG_LOG("RegisterAudioCapturerInfoChangeCallback is successful");
}

void NapiAudioCapturer::RegisterCapturerReadDataCallback(napi_env env, napi_value *argv, const std::string &cbName,
    NapiAudioCapturer *napiCapturer)
{
    CHECK_AND_RETURN_LOG(napiCapturer->capturerReadDataCallbackNapi_ == nullptr, "readData already subscribed.");

    napiCapturer->capturerReadDataCallbackNapi_ = std::make_shared<NapiCapturerReadDataCallback>(env, napiCapturer);
    napiCapturer->audioCapturer_->SetCaptureMode(CAPTURE_MODE_CALLBACK);
    CHECK_AND_RETURN_LOG(napiCapturer->capturerReadDataCallbackNapi_ != nullptr, "readDataNapi_ is nullptr");
    int32_t ret = napiCapturer->audioCapturer_->SetCapturerReadCallback(napiCapturer->capturerReadDataCallbackNapi_);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "SetCapturerCallback failed");
    std::shared_ptr<NapiCapturerReadDataCallback> cb =
        std::static_pointer_cast<NapiCapturerReadDataCallback>(napiCapturer->capturerReadDataCallbackNapi_);
    cb->AddCallbackReference(cbName, argv[PARAM1]);
    cb->CreateReadDataTsfn(env);

    AUDIO_INFO_LOG("Register Callback is successful");
}

napi_value NapiAudioCapturer::Off(napi_env env, napi_callback_info info)
{
    const size_t minArgCount = ARGS_ONE;
    size_t argc = ARGS_TWO;

    napi_value argv[minArgCount + 1] = {nullptr, nullptr};
    napi_value jsThis = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &jsThis, nullptr);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_SYSTEM), "napi_get_cb_info failed");
    CHECK_AND_RETURN_RET_LOG(argc >= minArgCount, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID, "mandatory parameters are left unspecified"), "argc invaild");

    napi_valuetype eventType = napi_undefined;
    napi_typeof(env, argv[PARAM0], &eventType);
    CHECK_AND_RETURN_RET_LOG(eventType == napi_string, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID, "incorrect parameter types: The type of eventType must be string"),
        "eventType invaild");

    std::string callbackName = NapiParamUtils::GetStringArgument(env, argv[PARAM0]);
    AUDIO_DEBUG_LOG("NapiAudioCapturer: Off callbackName: %{public}s", callbackName.c_str());

    return UnregisterCallback(env, jsThis, argc, argv, callbackName);
}

napi_value NapiAudioCapturer::UnregisterCallback(napi_env env, napi_value jsThis, size_t argc, napi_value *argv,
    const std::string &cbName)
{
    NapiAudioCapturer *napiCapturer = nullptr;
    napi_status status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napiCapturer));
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_SYSTEM), "napi_unwrap failed");
    CHECK_AND_RETURN_RET_LOG(napiCapturer != nullptr, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_NO_MEMORY), "napiCapturer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiCapturer->audioCapturer_ != nullptr, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_NO_MEMORY), "audioCapturer_ is nullptr");

    if (!cbName.compare(MARK_REACH_CALLBACK_NAME)) {
        UnregisterCapturerPositionCallback(env, argc, cbName, argv, napiCapturer);
    } else if (!cbName.compare(PERIOD_REACH_CALLBACK_NAME)) {
        UnregisterCapturerPeriodPositionCallback(env, argc, cbName, argv, napiCapturer);
    } else if (!cbName.compare(STATE_CHANGE_CALLBACK_NAME) ||
        !cbName.compare(AUDIO_INTERRUPT_CALLBACK_NAME)) {
        UnregisterCapturerCallback(env, argc, cbName, argv, napiCapturer);
    } else if (!cbName.compare(INPUTDEVICE_CHANGE_CALLBACK_NAME)) {
        UnregisterAudioCapturerDeviceChangeCallback(env, argc, cbName, argv, napiCapturer);
    } else if (!cbName.compare(AUDIO_CAPTURER_CHANGE_CALLBACK_NAME)) {
        UnregisterAudioCapturerInfoChangeCallback(env, argc, cbName, argv, napiCapturer);
    } else if (!cbName.compare(READ_DATA_CALLBACK_NAME)) {
        UnregisterCapturerReadDataCallback(env, argc, argv, napiCapturer);
    } else {
        bool unknownCallback = true;
        CHECK_AND_RETURN_RET_LOG(!unknownCallback, NapiAudioError::ThrowErrorAndReturn(env,
            NAPI_ERR_INVALID_PARAM,
            "parameter verification failed: The param of type is not supported"),
            "NAPI_ERR_UNSUPPORTED cbName");
    }

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

void NapiAudioCapturer::UnregisterCapturerPeriodPositionCallback(napi_env env, size_t argc,
    const std::string &cbName, napi_value *argv, NapiAudioCapturer *napiCapturer)
{
    CHECK_AND_RETURN_LOG(napiCapturer->periodPositionCbNapi_ != nullptr, "capturerCallbackNapi is nullptr");

    std::shared_ptr<NapiCapturerPeriodPositionCallback> cb =
        std::static_pointer_cast<NapiCapturerPeriodPositionCallback>(napiCapturer->periodPositionCbNapi_);
    std::function<int32_t(std::shared_ptr<NapiCapturerPeriodPositionCallback> callbackPtr,
        napi_value callback)> removeFunction =
        [&napiCapturer] (std::shared_ptr<NapiCapturerPeriodPositionCallback> callbackPtr, napi_value callback) {
            napiCapturer->audioCapturer_->UnsetCapturerPeriodPositionCallback();
            napiCapturer->periodPositionCbNapi_ = nullptr;
            return SUCCESS;
        };
    auto callback = GetCallback(argc, argv);
    UnregisterAudioCapturerSingletonCallbackTemplate(env, callback, cbName, cb, removeFunction);
    AUDIO_INFO_LOG("UnregisterCapturerPeriodPositionCallback is successful");
}

void NapiAudioCapturer::UnregisterCapturerPositionCallback(napi_env env, size_t argc,
    const std::string &cbName, napi_value *argv, NapiAudioCapturer *napiCapturer)
{
    CHECK_AND_RETURN_LOG(napiCapturer->positionCbNapi_ != nullptr, "capturerCallbackNapi is nullptr");

    std::shared_ptr<NapiCapturerPositionCallback> cb =
        std::static_pointer_cast<NapiCapturerPositionCallback>(napiCapturer->positionCbNapi_);
    std::function<int32_t(std::shared_ptr<NapiCapturerPositionCallback> callbackPtr,
        napi_value callback)> removeFunction =
        [&napiCapturer] (std::shared_ptr<NapiCapturerPositionCallback> callbackPtr, napi_value callback) {
            napiCapturer->audioCapturer_->UnsetCapturerPositionCallback();
            napiCapturer->positionCbNapi_ = nullptr;
            return SUCCESS;
        };
    auto callback = GetCallback(argc, argv);
    UnregisterAudioCapturerSingletonCallbackTemplate(env, callback, cbName, cb, removeFunction);
    AUDIO_INFO_LOG("UnregisterCapturerPositionCallback is successful");
}

void NapiAudioCapturer::UnregisterCapturerCallback(napi_env env, size_t argc,
    const std::string &cbName, napi_value *argv, NapiAudioCapturer *napiCapturer)
{
    CHECK_AND_RETURN_LOG(napiCapturer->callbackNapi_ != nullptr, "capturerCallbackNapi is nullptr");
    std::shared_ptr<NapiAudioCapturerCallback> cb =
        std::static_pointer_cast<NapiAudioCapturerCallback>(napiCapturer->callbackNapi_);
    auto callback = GetCallback(argc, argv);
    UnregisterAudioCapturerSingletonCallbackTemplate(env, callback, cbName, cb);
    AUDIO_INFO_LOG("Unregister CapturerCallback is successful");
}

void NapiAudioCapturer::UnregisterAudioCapturerDeviceChangeCallback(napi_env env, size_t argc,
    const std::string &cbName, napi_value *argv, NapiAudioCapturer *napiCapturer)
{
    std::function<int32_t(std::shared_ptr<NapiAudioCapturerDeviceChangeCallback> callbackptr,
        napi_value callback)> removeFunction =
        [&napiCapturer] (std::shared_ptr<NapiAudioCapturerDeviceChangeCallback> callbackPtr, napi_value callback) {
            return napiCapturer->audioCapturer_->RemoveAudioCapturerDeviceChangeCallback(callbackPtr);
        };
    auto callback = GetCallback(argc, argv);
    UnregisterAudioCapturerCallbackTemplate(env, callback, napiCapturer, cbName, removeFunction);
    
    AUDIO_INFO_LOG("UnregisterCapturerDeviceChangeCallback is successful");
}

void NapiAudioCapturer::UnregisterAudioCapturerInfoChangeCallback(napi_env env, size_t argc,
    const std::string &cbName, napi_value *argv, NapiAudioCapturer *napiCapturer)
{
    std::function<int32_t(std::shared_ptr<NapiAudioCapturerInfoChangeCallback> callbackPtr,
        napi_value callback)> removeFunction =
        [&napiCapturer] (std::shared_ptr<NapiAudioCapturerInfoChangeCallback> callbackPtr, napi_value callback) {
            return napiCapturer->audioCapturer_->RemoveAudioCapturerInfoChangeCallback(callbackPtr);
        };
    auto callback = GetCallback(argc, argv);
    UnregisterAudioCapturerCallbackTemplate(env, callback, napiCapturer, cbName, removeFunction);
    AUDIO_DEBUG_LOG("UnregisterAudioCapturerInfoChangeCallback is successful");
}

void NapiAudioCapturer::UnregisterCapturerReadDataCallback(napi_env env, size_t argc, napi_value *argv,
    NapiAudioCapturer *napiCapturer)
{
    napi_value callback = nullptr;

    if (argc == ARGS_TWO) {
        callback = argv[PARAM1];
    }
    CHECK_AND_RETURN_LOG(napiCapturer->capturerReadDataCallbackNapi_ != nullptr,
        "napiCapturerReadDataCallback is nullptr, return");

    std::shared_ptr<NapiCapturerReadDataCallback> cb =
        std::static_pointer_cast<NapiCapturerReadDataCallback>(napiCapturer->capturerReadDataCallbackNapi_);
    cb->RemoveCallbackReference(env, callback);

    AUDIO_INFO_LOG("Unregister Callback is successful");
}
} // namespace AudioStandard
} // namespace OHOS
