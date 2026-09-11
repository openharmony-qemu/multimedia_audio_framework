/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "NapiAudioSuiteDownloadManager"
#endif

#include <memory>
#include "napi_audio_suite_download_manager.h"
#include "napi_audio_error.h"
#include "napi_param_utils.h"
#include "napi_audio_enum.h"
#include "audio_errors.h"
#include "audio_common_log.h"
#include "napi_audio_suite_download_callback.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

static constexpr int32_t NETWORK_TYPE_CELLULAR = 1;
static constexpr int32_t NETWORK_TYPE_CELLULAR_AND_WIFI = 7;

static __thread napi_ref g_suiteDownloadMgrConstructor = nullptr;

NapiAudioSuiteDownloadManager::NapiAudioSuiteDownloadManager()
    : audioSuiteDownloadMgr_(nullptr), env_(nullptr) {}

NapiAudioSuiteDownloadManager::~NapiAudioSuiteDownloadManager()
{
    AUDIO_DEBUG_LOG("NapiAudioSuiteDownloadManager destructor");
}

void NapiAudioSuiteDownloadManager::Destructor(napi_env env, void *nativeObject, void *finalizeHint)
{
    if (nativeObject == nullptr) {
        AUDIO_WARNING_LOG("Destructor: nativeObject is null");
        return;
    }
    auto obj = static_cast<NapiAudioSuiteDownloadManager *>(nativeObject);
    ObjectRefMap<NapiAudioSuiteDownloadManager>::DecreaseRef(obj);
    AUDIO_INFO_LOG("Destructor: decrease obj count");
}

napi_value NapiAudioSuiteDownloadManager::Construct(napi_env env, napi_callback_info info)
{
    AUDIO_DEBUG_LOG("Construct");
    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    size_t argc = ARGS_TWO;
    napi_value argv[ARGS_TWO] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "Construct: napi_get_cb_info failed");

    auto napiSuiteDownloadMgr = std::make_unique<NapiAudioSuiteDownloadManager>();
    CHECK_AND_RETURN_RET_LOG(napiSuiteDownloadMgr != nullptr, result, "No memory");

    napiSuiteDownloadMgr->env_ = env;
    napiSuiteDownloadMgr->audioSuiteDownloadMgr_ = &AudioSuite::AudioSuiteDownloadManager::GetInstance();
    ObjectRefMap<NapiAudioSuiteDownloadManager>::Insert(napiSuiteDownloadMgr.get());

    status = napi_wrap(env, thisVar, static_cast<void *>(napiSuiteDownloadMgr.get()),
        NapiAudioSuiteDownloadManager::Destructor, nullptr, nullptr);
    if (status != napi_ok) {
        ObjectRefMap<NapiAudioSuiteDownloadManager>::Erase(napiSuiteDownloadMgr.get());
        return result;
    }
    napiSuiteDownloadMgr.release();
    return thisVar;
}

napi_value NapiAudioSuiteDownloadManager::Init(napi_env env, napi_value exports)
{
    napi_status status;
    napi_value constructor;
    napi_value result = nullptr;
    const int32_t refCount = ARGS_ONE;
    napi_get_undefined(env, &result);

    napi_property_descriptor audio_suite_download_mgr_properties[] = {
        DECLARE_NAPI_FUNCTION("onDownloadStatusChange", OnDownloadStatusChange),
        DECLARE_NAPI_FUNCTION("offDownloadStatusChange", OffDownloadStatusChange),
        DECLARE_NAPI_FUNCTION("startDownload", StartDownload),
        DECLARE_NAPI_FUNCTION("pauseDownload", PauseDownload),
        DECLARE_NAPI_FUNCTION("cancelDownload", CancelDownload),
        DECLARE_NAPI_FUNCTION("startBackgroundDownload", StartBackgroundDownload),
        DECLARE_NAPI_FUNCTION("getDownloadStatus", GetDownloadStatus),
        DECLARE_NAPI_FUNCTION("uninstallCloudRom", UninstallCloudRom),
    };

    status = napi_define_class(env, NAPI_AUDIO_SUITE_DOWNLOAD_MGR_CLASS_NAME.c_str(), NAPI_AUTO_LENGTH,
        Construct, nullptr,
        sizeof(audio_suite_download_mgr_properties) / sizeof(audio_suite_download_mgr_properties[PARAM0]),
        audio_suite_download_mgr_properties, &constructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "Init: napi_define_class fail");

    status = napi_create_reference(env, constructor, refCount, &g_suiteDownloadMgrConstructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "Init: napi_create_reference fail");
    status = napi_set_named_property(env, exports, NAPI_AUDIO_SUITE_DOWNLOAD_MGR_CLASS_NAME.c_str(), constructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "Init: napi_set_named_property fail");
    return exports;
}

napi_value NapiAudioSuiteDownloadManager::CreateSuiteDownloadManagerWrapper(napi_env env)
{
    napi_status status;
    napi_value result = nullptr;
    napi_value constructor = nullptr;

    status = napi_get_reference_value(env, g_suiteDownloadMgrConstructor, &constructor);
    if (status != napi_ok) {
        AUDIO_ERR_LOG("CreateSuiteDownloadManagerWrapper: get reference failed, status:%{public}d", status);
        napi_get_undefined(env, &result);
        return result;
    }
    status = napi_new_instance(env, constructor, ARGS_ZERO, nullptr, &result);
    if (status != napi_ok) {
        AUDIO_ERR_LOG("CreateSuiteDownloadManagerWrapper: napi_new_instance failed, status:%{public}d", status);
        napi_get_undefined(env, &result);
        return result;
    }
    return result;
}

bool NapiAudioSuiteDownloadManager::CheckContextStatus(
    std::shared_ptr<AudioSuiteDownloadMgrAsyncContext> context)
{
    CHECK_AND_RETURN_RET_LOG(context != nullptr, false, "context object is nullptr.");
    if (context->native == nullptr) {
        context->SignError(NAPI_ERR_SYSTEM);
        return false;
    }
    return true;
}

bool NapiAudioSuiteDownloadManager::CheckSuiteDownloadManagerStatus(
    NapiAudioSuiteDownloadManager *napi, std::shared_ptr<AudioSuiteDownloadMgrAsyncContext> context)
{
    CHECK_AND_RETURN_RET_LOG(napi != nullptr, false, "napi object is nullptr.");
    if (napi->audioSuiteDownloadMgr_ == nullptr) {
        context->SignError(NAPI_ERR_SYSTEM);
        AUDIO_ERR_LOG("audioSuiteDownloadMgr_ is nullptr.");
        return false;
    }
    return true;
}

int32_t NapiAudioSuiteDownloadManager::ConvertNativeErrorToNapi(int32_t nativeError)
{
    if (nativeError == SUCCESS) {
        return 0;
    }
    if (nativeError == ERR_INVALID_PARAM) {
        return NAPI_ERR_INVALID_PARAM;
    }
    if (nativeError == ERR_ILLEGAL_STATE || nativeError == ERR_NOT_STARTED) {
        return NAPI_ERR_ILLEGAL_STATE;
    }
    if (nativeError == ERR_SYSTEM_PERMISSION_DENIED) {
        return NAPI_ERR_PERMISSION_DENIED;
    }
    return NAPI_ERR_SYSTEM;
}

napi_value NapiAudioSuiteDownloadManager::StartDownload(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioSuiteDownloadMgrAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("StartDownload failed: no memory");
        NapiAudioError::ThrowError(env, "StartDownload failed: no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_TWO, "invalid arguments",
            NAPI_ERR_INPUT_INVALID);
        context->featureName = NapiParamUtils::GetStringArgument(env, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, !context->featureName.empty(),
            "featureName is empty", NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->networkType, argv[PARAM1]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok,
            "get networkType failed", NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);

    if ((context->status != napi_ok) && (context->errCode == NAPI_ERR_INPUT_INVALID)) {
        NapiAudioError::ThrowError(env, context->errMessage.c_str(), context->errCode);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioSuiteDownloadManager *>(context->native);
        ObjectRefMap<NapiAudioSuiteDownloadManager> objectGuard(obj);
        auto *napi = objectGuard.GetPtr();
        if (!CheckSuiteDownloadManagerStatus(napi, context)) {
            return;
        }
        int32_t ret = napi->audioSuiteDownloadMgr_->StartDownloadDirect(
            context->featureName,
            context->networkType == NETWORK_TYPE_CELLULAR ||
            context->networkType == NETWORK_TYPE_CELLULAR_AND_WIFI);
        if (ret != SUCCESS) {
            int32_t errCode = ConvertNativeErrorToNapi(ret);
            context->SignError(errCode, "StartDownload failed");
        }
    };

    auto complete = [env, context](napi_value &output) {
        napi_get_undefined(env, &output);
    };
    return NapiAsyncWork::Enqueue(env, context, "OHAudioSuite_StartDownload", executor, complete);
}

napi_value NapiAudioSuiteDownloadManager::PauseDownload(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioSuiteDownloadMgrAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("PauseDownload failed: no memory");
        NapiAudioError::ThrowError(env, "PauseDownload failed: no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments",
            NAPI_ERR_INPUT_INVALID);
        context->featureName = NapiParamUtils::GetStringArgument(env, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, !context->featureName.empty(),
            "featureName is empty", NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);

    if ((context->status != napi_ok) && (context->errCode == NAPI_ERR_INPUT_INVALID)) {
        NapiAudioError::ThrowError(env, context->errMessage.c_str(), context->errCode);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioSuiteDownloadManager *>(context->native);
        ObjectRefMap<NapiAudioSuiteDownloadManager> objectGuard(obj);
        auto *napi = objectGuard.GetPtr();
        if (!CheckSuiteDownloadManagerStatus(napi, context)) {
            return;
        }
        int32_t ret = napi->audioSuiteDownloadMgr_->PauseDownloadDirect(context->featureName);
        if (ret != SUCCESS) {
            int32_t errCode = ConvertNativeErrorToNapi(ret);
            context->SignError(errCode, "PauseDownload failed");
        }
    };

    auto complete = [env, context](napi_value &output) {
        napi_get_undefined(env, &output);
    };
    return NapiAsyncWork::Enqueue(env, context, "OHAudioSuite_PauseDownload", executor, complete);
}

napi_value NapiAudioSuiteDownloadManager::CancelDownload(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioSuiteDownloadMgrAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("CancelDownload failed: no memory");
        NapiAudioError::ThrowError(env, "CancelDownload failed: no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments",
            NAPI_ERR_INPUT_INVALID);
        context->featureName = NapiParamUtils::GetStringArgument(env, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, !context->featureName.empty(),
            "featureName is empty", NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);

    if ((context->status != napi_ok) && (context->errCode == NAPI_ERR_INPUT_INVALID)) {
        NapiAudioError::ThrowError(env, context->errMessage.c_str(), context->errCode);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioSuiteDownloadManager *>(context->native);
        ObjectRefMap<NapiAudioSuiteDownloadManager> objectGuard(obj);
        auto *napi = objectGuard.GetPtr();
        if (!CheckSuiteDownloadManagerStatus(napi, context)) {
            return;
        }
        int32_t ret = napi->audioSuiteDownloadMgr_->CancelDownloadDirect(context->featureName);
        if (ret != SUCCESS) {
            int32_t errCode = ConvertNativeErrorToNapi(ret);
            context->SignError(errCode, "CancelDownload failed");
        }
    };

    auto complete = [env, context](napi_value &output) {
        napi_get_undefined(env, &output);
    };
    return NapiAsyncWork::Enqueue(env, context, "OHAudioSuite_CancelDownload", executor, complete);
}

napi_value NapiAudioSuiteDownloadManager::StartBackgroundDownload(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioSuiteDownloadMgrAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("StartBackgroundDownload failed: no memory");
        NapiAudioError::ThrowError(env, "StartBackgroundDownload failed: no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments",
            NAPI_ERR_INPUT_INVALID);
        context->featureName = NapiParamUtils::GetStringArgument(env, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, !context->featureName.empty(),
            "featureName is empty", NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);

    if ((context->status != napi_ok) && (context->errCode == NAPI_ERR_INPUT_INVALID)) {
        NapiAudioError::ThrowError(env, context->errMessage.c_str(), context->errCode);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioSuiteDownloadManager *>(context->native);
        ObjectRefMap<NapiAudioSuiteDownloadManager> objectGuard(obj);
        auto *napi = objectGuard.GetPtr();
        if (!CheckSuiteDownloadManagerStatus(napi, context)) {
            return;
        }
        int32_t ret = napi->audioSuiteDownloadMgr_->StartBackgroundDownloadDirect(context->featureName);
        if (ret != SUCCESS) {
            int32_t errCode = ConvertNativeErrorToNapi(ret);
            context->SignError(errCode, "StartBackgroundDownload failed");
        }
    };

    auto complete = [env, context](napi_value &output) {
        napi_get_undefined(env, &output);
    };
    return NapiAsyncWork::Enqueue(env, context, "OHAudioSuite_StartBackgroundDownload", executor, complete);
}

napi_value NapiAudioSuiteDownloadManager::GetDownloadStatus(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioSuiteDownloadMgrAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("GetDownloadStatus failed: no memory");
        NapiAudioError::ThrowError(env, "GetDownloadStatus failed: no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments",
            NAPI_ERR_INPUT_INVALID);
        context->featureName = NapiParamUtils::GetStringArgument(env, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, !context->featureName.empty(),
            "featureName is empty", NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);

    if ((context->status != napi_ok) && (context->errCode == NAPI_ERR_INPUT_INVALID)) {
        NapiAudioError::ThrowError(env, context->errMessage.c_str(), context->errCode);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioSuiteDownloadManager *>(context->native);
        ObjectRefMap<NapiAudioSuiteDownloadManager> objectGuard(obj);
        auto *napi = objectGuard.GetPtr();
        if (!CheckSuiteDownloadManagerStatus(napi, context)) {
            return;
        }
        int32_t ret = napi->audioSuiteDownloadMgr_->GetDownloadStatusDirect(
            context->featureName, context->statusInfo);
        if (ret != SUCCESS) {
            int32_t errCode = ConvertNativeErrorToNapi(ret);
            context->SignError(errCode, "GetDownloadStatus failed");
        }
    };

    auto complete = [env, context](napi_value &output) {
        napi_value statusObj = nullptr;
        napi_status nstatus = NapiParamUtils::SetValueAudioSuiteDownloadStatusInfo(
            env, context->statusInfo, statusObj);
        if (nstatus != napi_ok || statusObj == nullptr) {
            AUDIO_ERR_LOG("GetDownloadStatus: SetValueAudioSuiteDownloadStatusInfo failed");
            napi_get_undefined(env, &output);
            return;
        }
        napi_create_array(env, &output);
        napi_set_element(env, output, PARAM0, statusObj);
    };
    return NapiAsyncWork::Enqueue(env, context, "OHAudioSuite_GetDownloadStatus", executor, complete);
}

napi_value NapiAudioSuiteDownloadManager::UninstallCloudRom(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioSuiteDownloadMgrAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("UninstallCloudRom failed: no memory");
        NapiAudioError::ThrowError(env, "UninstallCloudRom failed: no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments",
            NAPI_ERR_INPUT_INVALID);
        context->featureName = NapiParamUtils::GetStringArgument(env, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, !context->featureName.empty(),
            "featureName is empty", NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);

    if ((context->status != napi_ok) && (context->errCode == NAPI_ERR_INPUT_INVALID)) {
        NapiAudioError::ThrowError(env, context->errMessage.c_str(), context->errCode);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioSuiteDownloadManager *>(context->native);
        ObjectRefMap<NapiAudioSuiteDownloadManager> objectGuard(obj);
        auto *napi = objectGuard.GetPtr();
        if (!CheckSuiteDownloadManagerStatus(napi, context)) {
            return;
        }
        int32_t ret = napi->audioSuiteDownloadMgr_->UninstallCloudRomDirect(context->featureName);
        if (ret != SUCCESS) {
            int32_t errCode = ConvertNativeErrorToNapi(ret);
            context->SignError(errCode, "UninstallCloudRom failed");
        }
    };

    auto complete = [env, context](napi_value &output) {
        napi_get_undefined(env, &output);
    };
    return NapiAsyncWork::Enqueue(env, context, "OHAudioSuite_UninstallCloudRom", executor, complete);
}

void NapiAudioSuiteDownloadManager::RegisterDownloadStatusCallback(napi_env env, napi_value *args,
    NapiAudioSuiteDownloadManager *napi)
{
    std::lock_guard<std::mutex> lock(napi->downloadCbMutex_);
    if (args[PARAM0] == nullptr) {
        AUDIO_ERR_LOG("RegisterDownloadStatusCallback: callback is nullptr");
        NapiAudioError::ThrowError(env, "callback function is nullptr", NAPI_ERR_INVALID_PARAM);
        return;
    }

    if (napi->downloadCallback_ == nullptr) {
        napi->downloadCallback_ = std::make_shared<NapiAudioSuiteDownloadCallback>(env);
        if (napi->downloadCallback_ == nullptr) {
            AUDIO_ERR_LOG("RegisterDownloadStatusCallback: Memory Allocation Failed");
            NapiAudioError::ThrowError(env, "Memory Allocation Failed", NAPI_ERR_NO_MEMORY);
            return;
        }
        int32_t ret = napi->audioSuiteDownloadMgr_->RegisterDownloadStatusCallback(napi->downloadCallback_);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("RegisterDownloadStatusCallback failed, ret=%{public}d", ret);
            NapiAudioError::ThrowError(env, "RegisterDownloadStatusCallback failed", NAPI_ERR_SYSTEM);
            napi->downloadCallback_ = nullptr;
            return;
        }
    }

    if (napi->downloadCallback_->ContainSameJsCallback(env, args[PARAM0])) {
        AUDIO_WARNING_LOG("RegisterDownloadStatusCallback: callback already registered");
        return;
    }

    napi->downloadCallback_->SaveCallbackReference(args[PARAM0]);
    if (!napi->downloadCallback_->GetDownloadStatusTsfnFlag()) {
        napi->downloadCallback_->CreateDownloadStatusTsfn(env);
    }
    AUDIO_INFO_LOG("RegisterDownloadStatusCallback success");
}

void NapiAudioSuiteDownloadManager::UnregisterDownloadStatusCallback(napi_env env, napi_value *args,
    NapiAudioSuiteDownloadManager *napi)
{
    std::lock_guard<std::mutex> lock(napi->downloadCbMutex_);
    if (napi->downloadCallback_ == nullptr) {
        AUDIO_WARNING_LOG("UnregisterDownloadStatusCallback: no callback registered");
        return;
    }

    if (args[PARAM0] != nullptr) {
        napi->downloadCallback_->RemoveCallbackReference(env, args[PARAM0]);
    } else {
        napi->downloadCallback_->ClearCallbacks();
    }

    if (napi->downloadCallback_->IsEmpty()) {
        int32_t ret = napi->audioSuiteDownloadMgr_->UnregisterDownloadStatusCallback();
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("UnregisterDownloadStatusCallback failed, ret=%{public}d", ret);
        }
        napi->downloadCallback_ = nullptr;
    }
    AUDIO_INFO_LOG("UnregisterDownloadStatusCallback success");
}

napi_value NapiAudioSuiteDownloadManager::OnDownloadStatusChange(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = {0};
    napi_value jsThis = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &jsThis, nullptr);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok && jsThis != nullptr, nullptr,
        "OnDownloadStatusChange: napi_get_cb_info failed");
    CHECK_AND_RETURN_RET_LOG(argc >= ARGS_ONE, nullptr,
        "OnDownloadStatusChange: invalid argument count");

    NapiAudioSuiteDownloadManager *napi = nullptr;
    status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napi));
    CHECK_AND_RETURN_RET_LOG(status == napi_ok && napi != nullptr, nullptr,
        "OnDownloadStatusChange: napi_unwrap failed");
    CHECK_AND_RETURN_RET_LOG(napi->audioSuiteDownloadMgr_ != nullptr, nullptr,
        "OnDownloadStatusChange: audioSuiteDownloadMgr_ is nullptr");

    RegisterDownloadStatusCallback(env, argv, napi);
    return NapiParamUtils::GetUndefinedValue(env);
}

napi_value NapiAudioSuiteDownloadManager::OffDownloadStatusChange(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = {0};
    napi_value jsThis = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &jsThis, nullptr);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok && jsThis != nullptr, nullptr,
        "OffDownloadStatusChange: napi_get_cb_info failed");

    NapiAudioSuiteDownloadManager *napi = nullptr;
    status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napi));
    CHECK_AND_RETURN_RET_LOG(status == napi_ok && napi != nullptr, nullptr,
        "OffDownloadStatusChange: napi_unwrap failed");
    CHECK_AND_RETURN_RET_LOG(napi->audioSuiteDownloadMgr_ != nullptr, nullptr,
        "OffDownloadStatusChange: audioSuiteDownloadMgr_ is nullptr");

    UnregisterDownloadStatusCallback(env, argv, napi);
    return NapiParamUtils::GetUndefinedValue(env);
}

}  // namespace AudioStandard
}  // namespace OHOS
