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
#define LOG_TAG "NapiTonePlayer"
#endif

#include "napi_toneplayer.h"

#include "audio_utils.h"
#include "napi_param_utils.h"
#include "napi_audio_error.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {
static __thread napi_ref g_tonePlayerConstructor = nullptr;
std::unique_ptr<AudioRendererInfo> NapiTonePlayer::sRendererInfo_ = nullptr;
mutex NapiTonePlayer::createMutex_;
int32_t NapiTonePlayer::isConstructSuccess_ = SUCCESS;

static napi_value ThrowErrorAndReturn(napi_env env, int32_t errCode)
{
    NapiAudioError::ThrowError(env, errCode);
    return nullptr;
}

NapiTonePlayer::NapiTonePlayer()
    : tonePlayer_(nullptr), env_(nullptr) {}

NapiTonePlayer::~NapiTonePlayer() = default;

bool NapiTonePlayer::CheckTonePlayerStatus(NapiTonePlayer *napi,
    std::shared_ptr<TonePlayerAsyncContext> context)
{
    CHECK_AND_RETURN_RET_LOG(napi != nullptr, false, "napi object is nullptr.");
    if (napi->tonePlayer_ == nullptr) {
        context->SignError(NAPI_ERR_SYSTEM);
        return false;
    }
    return true;
}

void NapiTonePlayer::Destructor(napi_env env, void *nativeObject, void *finalizeHint)
{
    if (nativeObject != nullptr) {
        auto obj = static_cast<NapiTonePlayer *>(nativeObject);
        ObjectRefMap<NapiTonePlayer>::DecreaseRef(obj);
    }
}

napi_value NapiTonePlayer::Init(napi_env env, napi_value exports)
{
    napi_status status;
    napi_value constructor;
    napi_value result = nullptr;
    const int32_t refCount = 1;
    napi_get_undefined(env, &result);
    AUDIO_DEBUG_LOG("NapiTonePlayer::Init");
    napi_property_descriptor audio_toneplayer_properties[] = {
        DECLARE_NAPI_FUNCTION("load", Load),
        DECLARE_NAPI_FUNCTION("start", Start),
        DECLARE_NAPI_FUNCTION("stop", Stop),
        DECLARE_NAPI_FUNCTION("release", Release),
    };

    napi_property_descriptor static_prop[] = {
        DECLARE_NAPI_STATIC_FUNCTION("createTonePlayer", CreateTonePlayer),
        DECLARE_NAPI_STATIC_FUNCTION("createTonePlayerSync", CreateTonePlayerSync),
    };

    status = napi_define_class(env, NAPI_TONE_PLAYER_CLASS_NAME.c_str(), NAPI_AUTO_LENGTH, Construct, nullptr,
        sizeof(audio_toneplayer_properties) / sizeof(audio_toneplayer_properties[PARAM0]),
        audio_toneplayer_properties, &constructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_define_class failed");

    status = napi_create_reference(env, constructor, refCount, &g_tonePlayerConstructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_create_reference failed");

    status = napi_set_named_property(env, exports, NAPI_TONE_PLAYER_CLASS_NAME.c_str(), constructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_set_named_property failed");

    status = napi_define_properties(env, exports,
        sizeof(static_prop) / sizeof(static_prop[PARAM0]), static_prop);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_define_properties failed");

    return exports;
}

napi_value NapiTonePlayer::Construct(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    size_t argCount = ARGS_TWO;
    napi_value thisVar = nullptr;
    status = napi_get_cb_info(env, info, &argCount, nullptr, &thisVar, nullptr);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "failed to napi_get_cb_info");

    unique_ptr<NapiTonePlayer> napiTonePlayer = make_unique<NapiTonePlayer>();
    CHECK_AND_RETURN_RET_LOG(napiTonePlayer != nullptr, result, "No memory");
    ObjectRefMap<NapiTonePlayer>::Insert(napiTonePlayer.get());

    napiTonePlayer->env_ = env;
    AudioRendererInfo rendererInfo = {};
    CHECK_AND_RETURN_RET_LOG(sRendererInfo_ != nullptr, result, "Construct sRendererInfo_ is null");
    rendererInfo = *sRendererInfo_;
    std::string cacheDir = "/data/storage/el2/base/cache";
    /* NapiTonePlayer not support other rendererFlags, only support flag 0 */
    if (rendererInfo.rendererFlags != 0) {
        rendererInfo.rendererFlags = 0;
    }
    napiTonePlayer->tonePlayer_ = TonePlayer::Create(cacheDir, rendererInfo);

    if (napiTonePlayer->tonePlayer_  == nullptr) {
        AUDIO_ERR_LOG("Toneplayer Create failed");
        NapiTonePlayer::isConstructSuccess_ = NAPI_ERR_PERMISSION_DENIED;
    }

    status = napi_wrap(env, thisVar, static_cast<void*>(napiTonePlayer.get()),
        NapiTonePlayer::Destructor, nullptr, nullptr);
    if (status != napi_ok) {
        ObjectRefMap<NapiTonePlayer>::Erase(napiTonePlayer.get());
        return result;
    }
    napiTonePlayer.release();
    return thisVar;
}

napi_value NapiTonePlayer::CreateTonePlayerWrapper(napi_env env, unique_ptr<AudioRendererInfo> &rendererInfo)
{
    lock_guard<mutex> lock(createMutex_);
    napi_status status = napi_invalid_arg;
    napi_value result = nullptr;
    napi_value constructor;

    if (rendererInfo == nullptr) {
        goto fail;
    }
    status = napi_get_reference_value(env, g_tonePlayerConstructor, &constructor);
    if (status != napi_ok) {
        goto fail;
    }
    sRendererInfo_ = move(rendererInfo);
    status = napi_new_instance(env, constructor, 0, nullptr, &result);
    sRendererInfo_.release();
    if (status != napi_ok) {
        goto fail;
    }
    return result;

fail:
    AUDIO_ERR_LOG("Failed in CreateTonePlayerWrapper, %{public}d", status);
    napi_get_undefined(env, &result);
    return result;
}

napi_value NapiTonePlayer::CreateTonePlayer(napi_env env, napi_callback_info info)
{
    AUDIO_INFO_LOG("CreateTonePlayer");
    auto context = std::make_shared<TonePlayerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("CreateTonePlayer failed : no memory");
        NapiAudioError::ThrowError(env, "CreateTonePlayer failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetRendererInfo(env, &context->rendererInfo, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "GetRendererInfo failed",
            NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);

    auto complete = [env, context](napi_value &output) {
        unique_ptr<AudioRendererInfo> audioRendererInfo = make_unique<AudioRendererInfo>();
        *audioRendererInfo = context->rendererInfo;
        output = CreateTonePlayerWrapper(env, audioRendererInfo);
    };

    return NapiAsyncWork::Enqueue(env, context, "CreateTonePlayer", nullptr, complete);
}

napi_value NapiTonePlayer::CreateTonePlayerSync(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = {};
    napi_status status = NapiParamUtils::GetParam(env, info, argc, argv);
    CHECK_AND_RETURN_RET_LOG((argc == ARGS_ONE) && (status == napi_ok),
        ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID), "GetParam failed");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_object,
        ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID), "valueType invaild");

    AudioRendererInfo rendererInfo;

    CHECK_AND_RETURN_RET_LOG(NapiParamUtils::GetRendererInfo(env, &rendererInfo, argv[PARAM0]) == napi_ok,
        ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM), "GetRendererInfo failed");

    unique_ptr<AudioRendererInfo> audioRendererInfo = make_unique<AudioRendererInfo>();
    CHECK_AND_RETURN_RET_LOG(audioRendererInfo != nullptr,
        ThrowErrorAndReturn(env, NAPI_ERR_NO_MEMORY), "audioRendererInfo create failed,no memery.");
    *audioRendererInfo = rendererInfo;

    return NapiTonePlayer::CreateTonePlayerWrapper(env, audioRendererInfo);
}

napi_value NapiTonePlayer::Load(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<TonePlayerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("Load failed : no memory");
        NapiAudioError::ThrowError(env, "Load failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->toneType, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get toneType failed",
            NAPI_ERR_INVALID_PARAM);
        NAPI_CHECK_ARGS_RETURN_VOID(context, ToneTypeCheck(env, context->toneType), "toneType invaild",
            NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        auto obj = reinterpret_cast<NapiTonePlayer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiTonePlayer = objectGuard.GetPtr();
        ToneType toneType = static_cast<ToneType>(context->toneType);
        CHECK_AND_RETURN_LOG(CheckTonePlayerStatus(napiTonePlayer, context),
            "context object state is error.");
        context->isTrue = napiTonePlayer->tonePlayer_->LoadTone(toneType);
        if (!context->isTrue) {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };
    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "Load", executor, complete);
}

napi_value NapiTonePlayer::Start(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<TonePlayerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("Start failed : no memory");
        NapiAudioError::ThrowError(env, "Start failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    context->GetCbInfo(env, info);

    auto executor = [context]() {
        auto obj = reinterpret_cast<NapiTonePlayer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiTonePlayer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckTonePlayerStatus(napiTonePlayer, context),
            "context object state is error.");
        context->isTrue = napiTonePlayer->tonePlayer_->StartTone();
        if (!context->isTrue) {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };
    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "Start", executor, complete);
}

napi_value NapiTonePlayer::Stop(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<TonePlayerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("Stop failed : no memory");
        NapiAudioError::ThrowError(env, "Stop failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    context->GetCbInfo(env, info);

    auto executor = [context]() {
        auto obj = reinterpret_cast<NapiTonePlayer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiTonePlayer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckTonePlayerStatus(napiTonePlayer, context),
            "context object state is error.");
        context->isTrue = napiTonePlayer->tonePlayer_->StopTone();
        if (!context->isTrue) {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };
    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "Stop", executor, complete);
}

napi_value NapiTonePlayer::Release(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<TonePlayerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("Release failed : no memory");
        NapiAudioError::ThrowError(env, "Release failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    context->GetCbInfo(env, info);

    auto executor = [context]() {
        auto obj = reinterpret_cast<NapiTonePlayer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiTonePlayer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckTonePlayerStatus(napiTonePlayer, context),
            "context object state is error.");
        context->isTrue = napiTonePlayer->tonePlayer_->Release();
        if (!context->isTrue) {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };
    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "Release", executor, complete);
}

bool NapiTonePlayer::ToneTypeCheck(napi_env env, int32_t type)
{
    int32_t len = sizeof(TONE_TYPE_ARR) / sizeof(TONE_TYPE_ARR[PARAM0]);
    for (int32_t i = 0; i < len; i++) {
        if (TONE_TYPE_ARR[i] == type) {
            return true;
        }
    }
    return false;
}
} // namespace AudioStandard
} // namespace OHOS