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
#define LOG_TAG "NapiRecordControllerChangeCallback"
#endif

#include "js_native_api.h"
#include "napi_record_controller_change_callback.h"
#include "napi_param_utils.h"
#include "napi_audio_error.h"

namespace OHOS {
namespace AudioStandard {
NapiRecordControllerChangeCallback::NapiRecordControllerChangeCallback(napi_env env)
    : env_(env)
{
    AUDIO_DEBUG_LOG("NapiRecordControllerChangeCallback: instance create");
}

NapiRecordControllerChangeCallback::~NapiRecordControllerChangeCallback()
{
    if (regRecordControllerChgTsfn_) {
        napi_release_threadsafe_function(recordControllerChgTsfn_, napi_tsfn_abort);
    }
    AUDIO_DEBUG_LOG("NapiRecordControllerChangeCallback: instance destroy");
}

void NapiRecordControllerChangeCallback::SaveCallbackReference(const std::string &callbackName, napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callback = nullptr;
    std::string taskName = "NapiRecordControllerChangeCallback::destroy";
    const int32_t refCount = ARGS_ONE;
    napi_status status = napi_create_reference(env_, args, refCount, &callback);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
        "NapiRecordControllerChangeCallback: creating reference for callback fail");
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback, taskName);
    CHECK_AND_RETURN_LOG(callbackName == RECORD_CONTROLLER_ENABLED_CHANGE_CALLBACK_NAME,
        "NapiRecordControllerChangeCallback: Unknown callback type: %{public}s", callbackName.c_str());
    recordControllerChangeCallback_ = cb;
}

void NapiRecordControllerChangeCallback::CreateRecordControllerChangeTsfn(napi_env env)
{
    regRecordControllerChgTsfn_ = true;
    napi_value cbName;
    std::string callbackName = "RecordControllerEnabledChange";
    napi_create_string_utf8(env, callbackName.c_str(), callbackName.length(), &cbName);
    napi_create_threadsafe_function(env, nullptr, nullptr, cbName, 0, 1, nullptr,
        RecordControllerChangeTsfnFinalize, nullptr, SafeJsCallbackRecordControllerChangeWork,
        &recordControllerChgTsfn_);
}

bool NapiRecordControllerChangeCallback::GetRecordControllerChangeTsfnFlag()
{
    return regRecordControllerChgTsfn_;
}

void NapiRecordControllerChangeCallback::RemoveCallbackReference(const napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (args != nullptr) {
        napi_value callbackValue = nullptr;
        napi_status status = napi_get_reference_value(env_, recordControllerChangeCallback_->cb_, &callbackValue);
        bool isEquals = false;
        if (status != napi_ok || napi_strict_equals(env_, args, callbackValue, &isEquals) != napi_ok || !isEquals) {
            AUDIO_ERR_LOG("RemoveCallbackReference: callback not the same");
            return;
        }
    }
    recordControllerChangeCallback_ = nullptr;
    AUDIO_INFO_LOG("Remove callback reference successful.");
}

bool NapiRecordControllerChangeCallback::IsSameCallback(const napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (recordControllerChangeCallback_ == nullptr) {
        return false;
    }
    if (args == nullptr) {
        return true;
    }
    napi_value callbackValue = nullptr;
    napi_get_reference_value(env_, recordControllerChangeCallback_->cb_, &callbackValue);
    bool isEquals = false;
    CHECK_AND_RETURN_RET_LOG(napi_strict_equals(env_, args, callbackValue, &isEquals) == napi_ok, false,
        "get napi_strict_equals failed");
    return isEquals;
}

void NapiRecordControllerChangeCallback::OnRecordControllerEnabledChange(
    const SystemRecordControllerChangeInfo &changeInfo)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_LOG(recordControllerChangeCallback_ != nullptr, "callback not registered by JS client");

    std::unique_ptr<RecordControllerChangeJsCallback> cb = std::make_unique<RecordControllerChangeJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");

    cb->callback = recordControllerChangeCallback_;
    cb->callbackName = RECORD_CONTROLLER_ENABLED_CHANGE_CALLBACK_NAME;
    cb->changeInfo = changeInfo;
    return OnJsCallbackRecordControllerChange(cb);
}

void NapiRecordControllerChangeCallback::SafeJsCallbackRecordControllerChangeWork(
    napi_env env, napi_value js_cb, void *context, void *data)
{
    std::shared_ptr<RecordControllerChangeJsCallback> safeContext(
        static_cast<RecordControllerChangeJsCallback *>(data),
        [](RecordControllerChangeJsCallback *ptr) {
            delete ptr;
            ptr = nullptr;
    });
    CHECK_AND_RETURN_LOG(safeContext != nullptr && safeContext->callback != nullptr,
        "OnJsCallbackRecordControllerChange: no memory");
    std::string request = safeContext->callbackName;
    napi_ref callback = safeContext->callback->cb_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");

    do {
        napi_value jsCallback = nullptr;
        napi_status nstatus = napi_get_reference_value(env, callback, &jsCallback);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && jsCallback != nullptr, "%{public}s get reference value fail",
            request.c_str());
        napi_value args[ARGS_ONE] = { nullptr };
        nstatus = NapiParamUtils::SetValueSystemRecordControllerChangeInfo(env, safeContext->changeInfo, args[PARAM0]);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[0] != nullptr,
            "%{public}s fail to create RecordControllerChange callback", request.c_str());
        const size_t argCount = ARGS_ONE;
        napi_value result = nullptr;
        nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok, "%{public}s fail to call RecordControllerChange callback",
            request.c_str());
    } while (0);
    napi_close_handle_scope(env, scope);
}

void NapiRecordControllerChangeCallback::RecordControllerChangeTsfnFinalize(napi_env env, void *data, void *hint)
{
    AUDIO_INFO_LOG("RecordControllerChangeTsfnFinalize: safe thread resource release.");
}

void NapiRecordControllerChangeCallback::OnJsCallbackRecordControllerChange(
    std::unique_ptr<RecordControllerChangeJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackRecordControllerChange: jsCb.get() is null");
        return;
    }
    RecordControllerChangeJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");

    if (recordControllerChgTsfn_ == nullptr) {
        AUDIO_ERR_LOG("recordControllerChgTsfn_ is nullptr");
        delete event;
        return;
    }
    napi_acquire_threadsafe_function(recordControllerChgTsfn_);
    napi_call_threadsafe_function(recordControllerChgTsfn_, event, napi_tsfn_blocking);
}
} // namespace AudioStandard
} // namespace OHOS