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
#define LOG_TAG "NapiAsrProcessingController"
#endif

#include "napi_asr_processing_controller.h"

#include <map>
#include <string>
#include "napi_audio_enum.h"
#include "napi_audio_error.h"
#include "napi_param_utils.h"
#include "audio_errors.h"
#include "audio_log.h"
#include "audio_utils.h"
#include "napi_audio_capturer.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
using namespace HiviewDFX;
static __thread napi_ref g_asrConstructor = nullptr;

static const std::map<int32_t, int32_t> ERR_MAP = {
    {ERR_SYSTEM_PERMISSION_DENIED, NAPI_ERR_PERMISSION_DENIED},
    {ERR_INVALID_PARAM, NAPI_ERR_INVALID_PARAM},
    {ERROR, NAPI_ERR_UNSUPPORTED},
    {-1, NAPI_ERR_UNSUPPORTED},
};

static const std::map<int32_t, std::string> ERR_INFO_MAP = {
    {ERR_SYSTEM_PERMISSION_DENIED, "Caller is not a system application."},
    {ERR_INVALID_PARAM, "Parameter verification failed. : The param of mode must be mode enum"},
    {ERROR, "Operation not allowed."},
    {-1, "Operation not allowed."},
};

static int32_t GetResInt(int32_t errNum)
{
    auto it = ERR_MAP.find(errNum);
    int32_t errInt = NAPI_ERR_UNSUPPORTED;
    if (it != ERR_MAP.end()) {
        errInt = it->second;
    } else {
        AUDIO_ERR_LOG("err not found.");
    }
    return errInt;
}

static std::string GetResStr(int32_t errNum)
{
    auto it = ERR_INFO_MAP.find(errNum);
    std::string errStr = "Operation not allowed.";
    if (it != ERR_INFO_MAP.end()) {
        errStr = it->second;
    } else {
        AUDIO_ERR_LOG("err not found.");
    }
    return errStr;
}

static bool CheckCapturerValid(napi_env env, napi_value capturer)
{
    NapiAudioCapturer *napiCapturer = nullptr;

    napi_status status = napi_unwrap(env, capturer, reinterpret_cast<void**>(&napiCapturer));
    if (status != napi_ok) {
        AUDIO_ERR_LOG("napi unwrap failed");
        return false;
    }

    if (napiCapturer == nullptr) {
        AUDIO_ERR_LOG("napi capturer is nullptr");
        return false;
    }

    AudioCapturerInfo capturerInfo;
    napiCapturer->audioCapturer_->GetCapturerInfo(capturerInfo);
    if ((capturerInfo.sourceType != SourceType::SOURCE_TYPE_VOICE_RECOGNITION) &&
        (capturerInfo.sourceType != SourceType::SOURCE_TYPE_WAKEUP) &&
        (capturerInfo.sourceType != SourceType::SOURCE_TYPE_VOICE_CALL)) {
        AUDIO_ERR_LOG("sourceType not valid. type : %{public}d", capturerInfo.sourceType);
        return false;
    }
    return true;
}

NapiAsrProcessingController::NapiAsrProcessingController()
    : audioMngr_(nullptr), env_(nullptr) {}

NapiAsrProcessingController::~NapiAsrProcessingController()
{
}

NapiAsrProcessingController* NapiAsrProcessingController::GetParamWithSync(const napi_env& env,
    napi_callback_info info, size_t& argc, napi_value* args)
{
    napi_status status;
    NapiAsrProcessingController* napiAsrProcessingController = nullptr;
    napi_value jsThis = nullptr;

    status = napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok && jsThis != nullptr, nullptr,
        "GetParamWithSync fail to napi_get_cb_info");

    status = napi_unwrap(env, jsThis, (void**)&napiAsrProcessingController);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, nullptr, "napi_unwrap failed");
    CHECK_AND_RETURN_RET_LOG(napiAsrProcessingController != nullptr &&
        napiAsrProcessingController->audioMngr_ != nullptr, napiAsrProcessingController,
        "GetParamWithSync fail to napi_unwrap");
    return napiAsrProcessingController;
}

napi_status NapiAsrProcessingController::InitNapiAsrProcessingController(napi_env env, napi_value& constructor)
{
    napi_property_descriptor audio_svc_mngr_properties[] = {
        DECLARE_NAPI_FUNCTION("setAsrAecMode", SetAsrAecMode),
        DECLARE_NAPI_FUNCTION("getAsrAecMode", GetAsrAecMode),
        DECLARE_NAPI_FUNCTION("setAsrNoiseSuppressionMode", SetAsrNoiseSuppressionMode),
        DECLARE_NAPI_FUNCTION("getAsrNoiseSuppressionMode", GetAsrNoiseSuppressionMode),
        DECLARE_NAPI_FUNCTION("setAsrWhisperDetectionMode", SetAsrWhisperDetectionMode),
        DECLARE_NAPI_FUNCTION("getAsrWhisperDetectionMode", GetAsrWhisperDetectionMode),
        DECLARE_NAPI_FUNCTION("setAsrVoiceControlMode", SetAsrVoiceControlMode),
        DECLARE_NAPI_FUNCTION("setAsrVoiceMuteMode", SetAsrVoiceMuteMode),
        DECLARE_NAPI_FUNCTION("isWhispering", IsWhispering),
    };

    napi_status status = napi_define_class(env, NAPI_ASR_PROCESSING_CONTROLLER_CLASS_NAME.c_str(),
        NAPI_AUTO_LENGTH, Construct, nullptr,
        sizeof(audio_svc_mngr_properties) / sizeof(audio_svc_mngr_properties[PARAM0]),
        audio_svc_mngr_properties, &constructor);
    return status;
}

napi_value NapiAsrProcessingController::Init(napi_env env, napi_value exports)
{
    AUDIO_DEBUG_LOG("Init");
    napi_status status;
    napi_value constructor;
    napi_value result = nullptr;
    const int32_t refCount = 1;

    napi_property_descriptor static_prop[] = {
        DECLARE_NAPI_STATIC_FUNCTION("createAsrProcessingController", CreateAsrProcessingController),
    };

    status = InitNapiAsrProcessingController(env, constructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "InitNapiAudioRenderer fail");

    status = napi_create_reference(env, constructor, refCount, &g_asrConstructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_create_reference fail");
    status = napi_set_named_property(env, exports, NAPI_ASR_PROCESSING_CONTROLLER_CLASS_NAME.c_str(),
        constructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_set_named_property fail");
    status = napi_define_properties(env, exports, sizeof(static_prop) / sizeof(static_prop[PARAM0]),
        static_prop);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_define_properties fail");
    return exports;
}

void NapiAsrProcessingController::Destructor(napi_env env, void* nativeObject, void* finalizeHint)
{
    if (nativeObject != nullptr) {
        auto obj = static_cast<NapiAsrProcessingController*>(nativeObject);
        ObjectRefMap<NapiAsrProcessingController>::DecreaseRef(obj);
        AUDIO_DEBUG_LOG("delete obj done");
    }
}

napi_value NapiAsrProcessingController::Construct(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value jsThis;
    napi_value undefinedResult = nullptr;
    NapiParamUtils::GetUndefinedValue(env);
    size_t argCount = PARAM0;

    status = napi_get_cb_info(env, info, &argCount, nullptr, &jsThis, nullptr);
    if (status == napi_ok) {
        unique_ptr<NapiAsrProcessingController> managerNapi = make_unique<NapiAsrProcessingController>();
        if (managerNapi != nullptr) {
            ObjectRefMap<NapiAsrProcessingController>::Insert(managerNapi.get());
            managerNapi->env_ = env;
            managerNapi->audioMngr_ = AudioSystemManager::GetInstance();

            status = napi_wrap(env, jsThis, static_cast<void*>(managerNapi.get()),
                NapiAsrProcessingController::Destructor, nullptr, nullptr);
            if (status != napi_ok) {
                ObjectRefMap<NapiAsrProcessingController>::Erase(managerNapi.get());
                return undefinedResult;
            }
            managerNapi.release();
            return jsThis;
        }
    }
    return undefinedResult;
}

napi_value NapiAsrProcessingController::CreateAsrProcessingControllerWrapper(napi_env env)
{
    napi_status status;
    napi_value result = nullptr;
    napi_value constructor;

    status = napi_get_reference_value(env, g_asrConstructor, &constructor);
    if (status != napi_ok) {
        AUDIO_ERR_LOG("napi get ref failed,  status:%{public}d", status);
        goto fail;
    }
    status = napi_new_instance(env, constructor, 0, nullptr, &result);
    if (status != napi_ok) {
        AUDIO_ERR_LOG("napi_new_instance failed, status:%{public}d", status);
        goto fail;
    }
    return result;

fail:
    napi_get_undefined(env, &result);
    return result;
}

napi_value NapiAsrProcessingController::CreateAsrProcessingController(napi_env env, napi_callback_info info)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED), "No system permission");
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = {};
    napi_value thisVar = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    CHECK_AND_RETURN_RET_LOG(argc >= ARGS_ONE, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
        "mandatory parameters are left unspecified"), "argCount invaild");
    bool isCapturerValid = CheckCapturerValid(env, argv[PARAM0]);
    CHECK_AND_RETURN_RET_LOG(isCapturerValid,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_UNSUPPORTED), "Operation not allowed. ");
    return NapiAsrProcessingController::CreateAsrProcessingControllerWrapper(env);
}

napi_value NapiAsrProcessingController::SetAsrAecMode(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = {};
    auto *napiAsrController = GetParamWithSync(env, info, argc, argv);
    CHECK_AND_RETURN_RET_LOG(argc >= ARGS_ONE, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
        "mandatory parameters are left unspecified"), "argCount invaild");

    int32_t asrAecMode = 0;
    int32_t retMode = NapiParamUtils::GetValueInt32(env, asrAecMode, argv[PARAM0]);
    CHECK_AND_RETURN_RET_LOG(retMode == 0, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
        "parameter verification failed: The param of mode must be mode enum"), "Input parameter value error. ");
    CHECK_AND_RETURN_RET_LOG(asrAecMode == 0 || asrAecMode == 1, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INVALID_PARAM, "parameter verification failed: The param of mode must be enum AsrAecMode"),
        "Input parameter value error. ");
    CHECK_AND_RETURN_RET_LOG(napiAsrController != nullptr, result, "napiAsrController is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAsrController->audioMngr_ != nullptr, result, "audioMngr_ is nullptr");
    int32_t res = napiAsrController->audioMngr_->SetAsrAecMode(static_cast<AsrAecMode>(asrAecMode));
    CHECK_AND_RETURN_RET_LOG(res == 0, NapiAudioError::ThrowErrorAndReturn(env,
        GetResInt(res), GetResStr(res)), "SetAsrAecMode fail");
    bool setSuc = ((res == 0) ? true : false);
    NapiParamUtils::SetValueBoolean(env, setSuc, result);
    return result;
}

napi_value NapiAsrProcessingController::GetAsrAecMode(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = PARAM0;
    auto *napiAsrController = GetParamWithSync(env, info, argc, nullptr);
    AsrAecMode asrAecMode;
    CHECK_AND_RETURN_RET_LOG(napiAsrController != nullptr, result, "napiAsrController is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAsrController->audioMngr_ != nullptr, result, "audioMngr_ is nullptr");
    int32_t res = napiAsrController->audioMngr_->GetAsrAecMode(asrAecMode);
    CHECK_AND_RETURN_RET_LOG(res == 0, NapiAudioError::ThrowErrorAndReturn(env,
        GetResInt(res), GetResStr(res)), "GetAsrAecMode fail");
    NapiParamUtils::SetValueInt32(env, int32_t(asrAecMode), result);
    return result;
}

napi_value NapiAsrProcessingController::SetAsrNoiseSuppressionMode(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = {};
    auto *napiAsrController = GetParamWithSync(env, info, argc, argv);
    CHECK_AND_RETURN_RET_LOG(argc >= ARGS_ONE, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
        "mandatory parameters are left unspecified"), "argCount invaild");

    int32_t asrNoiseSuppressionMode = 0;
    int32_t asrVoiceControlModeMax = static_cast<int32_t>(AsrNoiseSuppressionMode::FAR_FIELD);
    int32_t retMode = NapiParamUtils::GetValueInt32(env, asrNoiseSuppressionMode, argv[PARAM0]);
    CHECK_AND_RETURN_RET_LOG(retMode == 0, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
        "parameter verification failed: The param of mode must be mode enum"), "Input parameter value error. ");
    CHECK_AND_RETURN_RET_LOG(asrNoiseSuppressionMode >= 0 && asrNoiseSuppressionMode <= asrVoiceControlModeMax,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
        "parameter verification failed: The param of mode must be enum AsrNoiseSuppressionMode"),
        "Input parameter value error. ");
    CHECK_AND_RETURN_RET_LOG(napiAsrController != nullptr, result, "napiAsrController is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAsrController->audioMngr_ != nullptr, result, "audioMngr_ is nullptr");
    int32_t res = napiAsrController->audioMngr_->SetAsrNoiseSuppressionMode(
        static_cast<AsrNoiseSuppressionMode>(asrNoiseSuppressionMode));
    CHECK_AND_RETURN_RET_LOG(res == 0, NapiAudioError::ThrowErrorAndReturn(env,
        GetResInt(res), GetResStr(res)), "SetNSMode fail");
    bool setSuc = ((res == 0) ? true : false);
    NapiParamUtils::SetValueBoolean(env, setSuc, result);
    return result;
}

napi_value NapiAsrProcessingController::GetAsrNoiseSuppressionMode(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = PARAM0;
    auto *napiAsrController = GetParamWithSync(env, info, argc, nullptr);
    AsrNoiseSuppressionMode asrNoiseSuppressionMode;
    CHECK_AND_RETURN_RET_LOG(napiAsrController != nullptr, result, "napiAsrController is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAsrController->audioMngr_ != nullptr, result, "audioMngr_ is nullptr");
    int32_t res = napiAsrController->audioMngr_->GetAsrNoiseSuppressionMode(asrNoiseSuppressionMode);
    CHECK_AND_RETURN_RET_LOG(res == 0, NapiAudioError::ThrowErrorAndReturn(env,
        GetResInt(res), GetResStr(res)), "GetNSMode fail");
    NapiParamUtils::SetValueInt32(env, int32_t(asrNoiseSuppressionMode), result);
    return result;
}

napi_value NapiAsrProcessingController::SetAsrWhisperDetectionMode(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = {};
    auto *napiAsrController = GetParamWithSync(env, info, argc, argv);
    CHECK_AND_RETURN_RET_LOG(argc >= ARGS_ONE, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID),
        "argCount invaild");

    int32_t asrWhisperDetectionMode = 0;
    int32_t retMode = NapiParamUtils::GetValueInt32(env, asrWhisperDetectionMode, argv[PARAM0]);
    CHECK_AND_RETURN_RET_LOG(retMode == 0, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
        "parameter verification failed: The param of mode must be mode enum"), "Input parameter value error. ");
    CHECK_AND_RETURN_RET_LOG(asrWhisperDetectionMode == 0 || asrWhisperDetectionMode == 1,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM), "Input parameter value error. ");
    CHECK_AND_RETURN_RET_LOG(napiAsrController != nullptr, result, "napiAsrController is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAsrController->audioMngr_ != nullptr, result, "audioMngr_ is nullptr");
    int32_t res = napiAsrController->audioMngr_->SetAsrWhisperDetectionMode(
        static_cast<AsrWhisperDetectionMode>(asrWhisperDetectionMode));
    CHECK_AND_RETURN_RET_LOG(res == 0, NapiAudioError::ThrowErrorAndReturn(env,
        GetResInt(res), GetResStr(res)), "SetAsrWhisperDetectionMode fail");
    bool setSuc = ((res == 0) ? true : false);
    NapiParamUtils::SetValueBoolean(env, setSuc, result);
    return result;
}

napi_value NapiAsrProcessingController::GetAsrWhisperDetectionMode(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = PARAM0;
    auto *napiAsrController = GetParamWithSync(env, info, argc, nullptr);
    AsrWhisperDetectionMode asrWhisperDetectionMode;
    CHECK_AND_RETURN_RET_LOG(napiAsrController != nullptr, result, "napiAsrController is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAsrController->audioMngr_ != nullptr, result, "audioMngr_ is nullptr");
    int32_t res = napiAsrController->audioMngr_->GetAsrWhisperDetectionMode(asrWhisperDetectionMode);
    CHECK_AND_RETURN_RET_LOG(res == 0, NapiAudioError::ThrowErrorAndReturn(env,
        GetResInt(res), GetResStr(res)), "GetAsrWhisperDetectionMode fail");
    NapiParamUtils::SetValueInt32(env, int32_t(asrWhisperDetectionMode), result);
    return result;
}

napi_value NapiAsrProcessingController::SetAsrVoiceControlMode(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = ARGS_TWO;
    napi_value argv[ARGS_TWO] = {};
    auto *napiAsrController = GetParamWithSync(env, info, argc, argv);
    CHECK_AND_RETURN_RET_LOG(argc >= ARGS_TWO, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
        "mandatory parameters are left unspecified"), "argCount invaild");

    int32_t asrVoiceControlMode = 0;
    bool on = false;
    int32_t asrVoiceControlModeMax = static_cast<int32_t>(AsrVoiceControlMode::AUDIO_MIX_2_VOICE_TX_EX);
    int32_t retMode = NapiParamUtils::GetValueInt32(env, asrVoiceControlMode, argv[PARAM0]);
    int32_t retBool = NapiParamUtils::GetValueBoolean(env, on, argv[PARAM1]);
    CHECK_AND_RETURN_RET_LOG(retMode == 0, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
        "parameter verification failed: The param of mode must be mode enum"), "Input parameter value error. ");
    CHECK_AND_RETURN_RET_LOG(retBool == 0, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
        "parameter verification failed: The param of mode must be bool"), "Input parameter value error. ");
    CHECK_AND_RETURN_RET_LOG(asrVoiceControlMode >= 0 && asrVoiceControlMode <= asrVoiceControlModeMax,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
        "parameter verification failed: The param of mode must be enum AsrVoiceControlMode"),
        "Input parameter value error. ");
    CHECK_AND_RETURN_RET_LOG(napiAsrController != nullptr, result, "napiAsrController is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAsrController->audioMngr_ != nullptr, result, "audioMngr_ is nullptr");
    int32_t res = napiAsrController->audioMngr_->SetAsrVoiceControlMode(
        static_cast<AsrVoiceControlMode>(asrVoiceControlMode), on);
    CHECK_AND_RETURN_RET_LOG(res == 0, NapiAudioError::ThrowErrorAndReturn(env,
        GetResInt(res), GetResStr(res)), "SetVCMode fail");
    bool setSuc = ((res == 0) ? true : false);
    NapiParamUtils::SetValueBoolean(env, setSuc, result);
    return result;
}

napi_value NapiAsrProcessingController::SetAsrVoiceMuteMode(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = ARGS_TWO;
    napi_value argv[ARGS_TWO] = {};
    auto *napiAsrController = GetParamWithSync(env, info, argc, argv);
    CHECK_AND_RETURN_RET_LOG(argc >= ARGS_TWO, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
        "mandatory parameters are left unspecified"), "argCount invaild");

    int32_t asrVoiceMuteMode = 0;
    bool on = false;
    int32_t asrVoiceMuteModeMax = static_cast<int32_t>(AsrVoiceMuteMode::OUTPUT_MUTE_EX);
    int32_t retMode = NapiParamUtils::GetValueInt32(env, asrVoiceMuteMode, argv[PARAM0]);
    int32_t retBool = NapiParamUtils::GetValueBoolean(env, on, argv[PARAM1]);
    CHECK_AND_RETURN_RET_LOG(retMode == 0, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
        "parameter verification failed: The param of mode must be mode enum"), "Input parameter value error. ");
    CHECK_AND_RETURN_RET_LOG(retBool == 0, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
        "parameter verification failed: The param of mode must be bool"), "Input parameter value error. ");
    CHECK_AND_RETURN_RET_LOG(asrVoiceMuteMode >= 0 && asrVoiceMuteMode <= asrVoiceMuteModeMax,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
        "parameter verification failed: The param of mode must be enum AsrVoiceMuteMode"),
        "Input parameter value error. ");
    CHECK_AND_RETURN_RET_LOG(napiAsrController != nullptr, result, "napiAsrController is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAsrController->audioMngr_ != nullptr, result, "audioMngr_ is nullptr");
    int32_t res = napiAsrController->audioMngr_->SetAsrVoiceMuteMode(
        static_cast<AsrVoiceMuteMode>(asrVoiceMuteMode), on);
    CHECK_AND_RETURN_RET_LOG(res == 0, NapiAudioError::ThrowErrorAndReturn(env,
        GetResInt(res), GetResStr(res)), "SetVMMode fail");
    bool setSuc = ((res == 0) ? true : false);
    NapiParamUtils::SetValueBoolean(env, setSuc, result);
    return result;
}

napi_value NapiAsrProcessingController::IsWhispering(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = PARAM0;
    auto *napiAsrController = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(napiAsrController != nullptr, result, "napiAsrController is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAsrController->audioMngr_ != nullptr, result, "audioMngr_ is nullptr");
    int32_t res = napiAsrController->audioMngr_->IsWhispering();
    CHECK_AND_RETURN_RET_LOG(res == 0 || res == 1, NapiAudioError::ThrowErrorAndReturn(env,
        GetResInt(res), GetResStr(res)), "IsWhispering fail");
    bool setSuc = ((res == 0) ? true : false);
    NapiParamUtils::SetValueBoolean(env, setSuc, result);
    return result;
}
}  // namespace AudioStandard
}  // namespace OHOS