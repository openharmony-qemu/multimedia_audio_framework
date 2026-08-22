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
#define LOG_TAG "NapiAudioSuiteDownloadCallback"
#endif

#include <memory>
#include "js_native_api.h"
#include "napi_audio_suite_download_callback.h"
#include "napi_param_utils.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace AudioStandard {
NapiAudioSuiteDownloadCallback::NapiAudioSuiteDownloadCallback(napi_env env)
    : env_(env)
{
    AUDIO_DEBUG_LOG("NapiAudioSuiteDownloadCallback constructor");
}

NapiAudioSuiteDownloadCallback::~NapiAudioSuiteDownloadCallback()
{
    if (regDownloadStatusTsfn_) {
        napi_release_threadsafe_function(downloadStatusTsfn_, napi_tsfn_abort);
        regDownloadStatusTsfn_ = false;
    }
    jsCallbacks_.clear();
    AUDIO_DEBUG_LOG("NapiAudioSuiteDownloadCallback destructor");
}

void NapiAudioSuiteDownloadCallback::OnDownloadStatusChanged(
    const std::vector<AudioSuite::AudioSuiteDownloadStatusInfo> &status)
{
    AUDIO_INFO_LOG("OnDownloadStatusChanged called, status size=%{public}zu", status.size());
    std::vector<std::shared_ptr<AutoRef>> callbacksCopy;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        callbacksCopy = jsCallbacks_;
    }
    if (callbacksCopy.empty()) {
        AUDIO_WARNING_LOG("OnDownloadStatusChanged: no JS callback registered");
        return;
    }

    auto jsCb = std::make_unique<DownloadStatusJsCallback>();
    CHECK_AND_RETURN_LOG(jsCb != nullptr, "OnDownloadStatusChanged: no memory");
    jsCb->callbacks = std::move(callbacksCopy);
    jsCb->callbackName = DOWNLOAD_STATUS_CHANGE_CALLBACK_NAME;
    jsCb->statusInfo = status;

    OnJsCallbackDownloadStatus(jsCb);
}

void NapiAudioSuiteDownloadCallback::SaveCallbackReference(napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callback = nullptr;
    const int32_t refCount = 1;
    std::string taskName = "NapiAudioSuiteDownloadCallback::destroy";
    napi_status status = napi_create_reference(env_, args, refCount, &callback);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
        "NapiAudioSuiteDownloadCallback: creating reference for callback fail");
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback, taskName);
    CHECK_AND_RETURN_LOG(cb != nullptr, "NapiAudioSuiteDownloadCallback: AutoRef is nullptr");
    jsCallbacks_.push_back(cb);
    AUDIO_INFO_LOG("SaveCallbackReference success, total callbacks=%{public}zu", jsCallbacks_.size());
}

bool NapiAudioSuiteDownloadCallback::RemoveCallbackReference(napi_env env, napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    bool isEquals = false;
    for (auto it = jsCallbacks_.begin(); it != jsCallbacks_.end(); ++it) {
        napi_value copyValue = nullptr;
        napi_status status = napi_get_reference_value(env, (*it)->GetRef(), &copyValue);
        if (status != napi_ok || copyValue == nullptr) {
            continue;
        }
        if (napi_strict_equals(env, copyValue, args, &isEquals) == napi_ok && isEquals) {
            jsCallbacks_.erase(it);
            AUDIO_INFO_LOG("RemoveCallbackReference success, remaining callbacks=%{public}zu", jsCallbacks_.size());
            return true;
        }
    }
    AUDIO_WARNING_LOG("RemoveCallbackReference: callback not found");
    return false;
}

void NapiAudioSuiteDownloadCallback::ClearCallbacks()
{
    std::lock_guard<std::mutex> lock(mutex_);
    jsCallbacks_.clear();
    AUDIO_INFO_LOG("ClearCallbacks success");
}

bool NapiAudioSuiteDownloadCallback::IsEmpty() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return jsCallbacks_.empty();
}

void NapiAudioSuiteDownloadCallback::CreateDownloadStatusTsfn(napi_env env)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (regDownloadStatusTsfn_) {
        return;
    }
    std::string callbackName = DOWNLOAD_STATUS_CHANGE_CALLBACK_NAME;
    napi_value cbName;
    napi_create_string_utf8(env, callbackName.c_str(), callbackName.length(), &cbName);
    napi_threadsafe_function tsfn = nullptr;
    napi_status status = napi_create_threadsafe_function(env, nullptr, nullptr, cbName, 0, 1, nullptr,
        DownloadStatusTsfnFinalize, nullptr, SafeJsCallbackDownloadStatusWork, &tsfn);
    if (status != napi_ok || tsfn == nullptr) {
        AUDIO_ERR_LOG("CreateDownloadStatusTsfn: napi_create_threadsafe_function failed, status=%{public}d", status);
        return;
    }
    downloadStatusTsfn_ = tsfn;
    regDownloadStatusTsfn_ = true;
    AUDIO_INFO_LOG("CreateDownloadStatusTsfn success");
}

bool NapiAudioSuiteDownloadCallback::GetDownloadStatusTsfnFlag() const
{
    return regDownloadStatusTsfn_;
}

bool NapiAudioSuiteDownloadCallback::ContainSameJsCallback(napi_env env, napi_value args) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    bool isEquals = false;
    for (const auto &cb : jsCallbacks_) {
        napi_value copyValue = nullptr;
        napi_status status = napi_get_reference_value(env, cb->GetRef(), &copyValue);
        if (status != napi_ok || copyValue == nullptr) {
            continue;
        }
        if (napi_strict_equals(env, copyValue, args, &isEquals) == napi_ok && isEquals) {
            return true;
        }
    }
    return false;
}

void NapiAudioSuiteDownloadCallback::SafeJsCallbackDownloadStatusWork(
    napi_env env, napi_value js_cb, void *context, void *data)
{
    std::unique_ptr<DownloadStatusJsCallback> eventGuard(
        reinterpret_cast<DownloadStatusJsCallback *>(data));
    DownloadStatusJsCallback *event = eventGuard.get();
    CHECK_AND_RETURN_LOG(event != nullptr, "SafeJsCallbackDownloadStatusWork: event is nullptr");
    CHECK_AND_RETURN_LOG(!event->callbacks.empty(), "SafeJsCallbackDownloadStatusWork: no callbacks");

    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "SafeJsCallbackDownloadStatusWork: scope is nullptr");

    napi_value statusArray = nullptr;
    napi_status nstatus = napi_create_array(env, &statusArray);
    CHECK_AND_RETURN_LOG(nstatus == napi_ok && statusArray != nullptr,
        "SafeJsCallbackDownloadStatusWork: create array failed");

    for (size_t i = 0; i < event->statusInfo.size(); i++) {
        napi_value statusObj = nullptr;
        nstatus = NapiParamUtils::SetValueAudioSuiteDownloadStatusInfo(env, event->statusInfo[i], statusObj);
        if (nstatus != napi_ok || statusObj == nullptr) {
            AUDIO_ERR_LOG("SafeJsCallbackDownloadStatusWork: "
                "SetValueAudioSuiteDownloadStatusInfo failed at index=%{public}zu", i);
            continue;
        }
        napi_set_element(env, statusArray, static_cast<uint32_t>(i), statusObj);
    }

    const size_t argCount = ARGS_ONE;
    for (const auto &cb : event->callbacks) {
        napi_value jsCallback = nullptr;
        napi_ref callbackRef = cb->GetRef();
        nstatus = napi_get_reference_value(env, callbackRef, &jsCallback);
        if (nstatus != napi_ok || jsCallback == nullptr) {
            AUDIO_ERR_LOG("SafeJsCallbackDownloadStatusWork: get reference value fail");
            continue;
        }
        napi_value result = nullptr;
        nstatus = napi_call_function(env, nullptr, jsCallback, argCount, &statusArray, &result);
        if (nstatus != napi_ok) {
            AUDIO_ERR_LOG("SafeJsCallbackDownloadStatusWork: call callback fail");
        }
    }
    napi_close_handle_scope(env, scope);
}

void NapiAudioSuiteDownloadCallback::DownloadStatusTsfnFinalize(napi_env env, void *data, void *hint)
{
    AUDIO_INFO_LOG("DownloadStatusTsfnFinalize: safe thread resource release.");
}

void NapiAudioSuiteDownloadCallback::OnJsCallbackDownloadStatus(
    std::unique_ptr<DownloadStatusJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackDownloadStatus: jsCb is nullptr");
        return;
    }
    if (!regDownloadStatusTsfn_ || downloadStatusTsfn_ == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackDownloadStatus: tsfn not created");
        return;
    }
    DownloadStatusJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG(event != nullptr, "OnJsCallbackDownloadStatus: event is nullptr");
    napi_acquire_threadsafe_function(downloadStatusTsfn_);
    napi_call_threadsafe_function(downloadStatusTsfn_, event, napi_tsfn_blocking);
}

}  // namespace AudioStandard
}  // namespace OHOS
