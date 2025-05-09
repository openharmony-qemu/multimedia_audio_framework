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
#define LOG_TAG "NapiCapturerReadDataCallback"
#endif

#include "js_native_api.h"
#include "napi_audio_capturer_read_data_callback.h"
#include "audio_capturer_log.h"

namespace OHOS {
namespace AudioStandard {
static bool g_napiAudioCapturerIsNullptr = true;
static std::mutex g_asynccallbackMutex;
static const int32_t READ_CALLBACK_TIMEOUT_IN_MS = 1000; // 1s

NapiCapturerReadDataCallback::NapiCapturerReadDataCallback(napi_env env, NapiAudioCapturer *napiCapturer)
    : env_(env), napiCapturer_(napiCapturer)
{
    AUDIO_DEBUG_LOG("instance create");
    g_napiAudioCapturerIsNullptr = false;
}

NapiCapturerReadDataCallback::~NapiCapturerReadDataCallback()
{
    if (regAcReadDataTsfn_) {
        napi_release_threadsafe_function(acReadDataTsfn_, napi_tsfn_abort);
    }
    AUDIO_DEBUG_LOG("instance destroy");
}

void NapiCapturerReadDataCallback::AddCallbackReference(const std::string &callbackName, napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callback = nullptr;
    const int32_t refCount = 1;
    napi_status status = napi_create_reference(env_, args, refCount, &callback);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr, "creating reference for callback failed");

    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback);
    if (callbackName == READ_DATA_CALLBACK_NAME) {
        capturerReadDataCallback_ = cb;
        isCallbackInited_ = true;
    } else {
        AUDIO_ERR_LOG("Unknown callback type: %{public}s", callbackName.c_str());
    }
}

void NapiCapturerReadDataCallback::CreateReadDataTsfn(napi_env env)
{
    regAcReadDataTsfn_ = true;
    napi_value cbName;
    std::string callbackName = "CapturerReadData";
    napi_create_string_utf8(env, callbackName.c_str(), callbackName.length(), &cbName);
    napi_create_threadsafe_function(env, nullptr, nullptr, cbName, 0, 1, nullptr,
        CaptureReadDataTsfnFinalize, nullptr, SafeJsCallbackCapturerReadDataWork, &acReadDataTsfn_);
}

void NapiCapturerReadDataCallback::RemoveCallbackReference(napi_env env, napi_value callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    isCallbackInited_ = false;
    bool isEquals = false;
    napi_value copyValue = nullptr;

    if (callback == nullptr) {
        napi_status ret = napi_delete_reference(env, capturerReadDataCallback_->cb_);
        CHECK_AND_RETURN_LOG(napi_ok == ret, "delete callback reference failed");
        AUDIO_INFO_LOG("Remove Js Callback");
        capturerReadDataCallback_->cb_ = nullptr;
        return;
    }

    napi_get_reference_value(env, capturerReadDataCallback_->cb_, &copyValue);
    CHECK_AND_RETURN_LOG(copyValue != nullptr, "copyValue is nullptr");
    CHECK_AND_RETURN_LOG(napi_strict_equals(env, callback, copyValue, &isEquals) == napi_ok,
        "get napi_strict_equals failed");
    if (isEquals) {
        AUDIO_INFO_LOG("found JS Callback, delete it!");
        napi_status status = napi_delete_reference(env, capturerReadDataCallback_->cb_);
        CHECK_AND_RETURN_LOG(status == napi_ok, "deleting reference for callback failed");
        capturerReadDataCallback_->cb_ = nullptr;
    }
}

void NapiCapturerReadDataCallback::RemoveNapiCapturer()
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::lock_guard<std::mutex> asyncLock(g_asynccallbackMutex);
    napiCapturer_ = nullptr;
    g_napiAudioCapturerIsNullptr = true;
}

void NapiCapturerReadDataCallback::OnReadData(size_t length)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_LOG(capturerReadDataCallback_ != nullptr, "Cannot find the reference of readData callback");

    std::unique_ptr<CapturerReadDataJsCallback> cb = std::make_unique<CapturerReadDataJsCallback>();
    cb->callback = capturerReadDataCallback_;
    cb->callbackName = READ_DATA_CALLBACK_NAME;
    cb->bufDesc.buffer = nullptr;
    cb->capturerNapiObj = napiCapturer_;
    cb->readDataCallbackPtr = this;

    CHECK_AND_RETURN_LOG(napiCapturer_ != nullptr, "Cannot find the reference to audio capturer napi");
    CHECK_AND_RETURN_LOG(napiCapturer_->audioCapturer_ != nullptr, "audioCapturer is null");
    napiCapturer_->audioCapturer_->GetBufferDesc(cb->bufDesc);
    if (cb->bufDesc.buffer == nullptr) {
        return;
    }
    if (length > cb->bufDesc.bufLength) {
        cb->bufDesc.dataLength = cb->bufDesc.bufLength;
    } else {
        cb->bufDesc.dataLength = length;
    }

    return OnJsCapturerReadDataCallback(cb);
}

void NapiCapturerReadDataCallback::OnJsCapturerReadDataCallback(std::unique_ptr<CapturerReadDataJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCapturerReadDataCallback: jsCb.get() is null");
        return;
    }

    auto obj = static_cast<NapiAudioCapturer *>(napiCapturer_);
    NapiAudioCapturer *napiCapturer = ObjectRefMap<NapiAudioCapturer>::IncreaseRef(obj);
    if (napiCapturer == nullptr) {
        AUDIO_ERR_LOG("napiCapturer is null");
        return;
    }

    CapturerReadDataJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCapturerReadDataCallback: event is nullptr.");

    if (napiCapturer_ == nullptr) {
        return;
    }
    napiCapturer_->isFrameCallbackDone_.store(false);

    napi_acquire_threadsafe_function(acReadDataTsfn_);
    napi_call_threadsafe_function(acReadDataTsfn_, event, napi_tsfn_blocking);

    std::unique_lock<std::mutex> readCallbackLock(napiCapturer_->readCallbackMutex_);
    bool isTimeout = !napiCapturer_->readCallbackCv_.wait_for(readCallbackLock,
        std::chrono::milliseconds(READ_CALLBACK_TIMEOUT_IN_MS), [this] {
            return napiCapturer_->isFrameCallbackDone_.load();
        });
    if (isTimeout) {
        AUDIO_ERR_LOG("Client OnReadData operation timed out");
    }
    readCallbackLock.unlock();
}

void NapiCapturerReadDataCallback::CaptureReadDataTsfnFinalize(napi_env env, void *data, void *hint)
{
    AUDIO_DEBUG_LOG("CaptureReadDataTsfnFinalize: safe thread resource release.");
}

void NapiCapturerReadDataCallback::SafeJsCallbackCapturerReadDataWork(
    napi_env env, napi_value js_cb, void *context, void *data)
{
    CapturerReadDataJsCallback *event = reinterpret_cast<CapturerReadDataJsCallback *>(data);
    CHECK_AND_RETURN_LOG(event != nullptr, "capturer read data event is nullptr");
    std::shared_ptr<CapturerReadDataJsCallback> safeContext(
        static_cast<CapturerReadDataJsCallback*>(data),
        [](CapturerReadDataJsCallback *ptr) {
            delete ptr;
    });
    SafeJsCallbackCapturerReadDataWorkInner(event);

    CHECK_AND_RETURN_LOG(event->capturerNapiObj != nullptr, "NapiAudioCapturer object is nullptr");
    event->capturerNapiObj->isFrameCallbackDone_.store(true);
    event->capturerNapiObj->readCallbackCv_.notify_all();
    auto napiObj = static_cast<NapiAudioCapturer *>(event->capturerNapiObj);
    ObjectRefMap<NapiAudioCapturer>::DecreaseRef(napiObj);
}

void NapiCapturerReadDataCallback::SafeJsCallbackCapturerReadDataWorkInner(CapturerReadDataJsCallback *event)
{
    CHECK_AND_RETURN_LOG(event != nullptr, "capture read data event is nullptr");
    CHECK_AND_RETURN_LOG(event->readDataCallbackPtr != nullptr, "CapturerReadDataCallback is already released");
    CHECK_AND_RETURN_LOG(event->readDataCallbackPtr->isCallbackInited_, "the callback has been dereferenced");
    std::string request = event->callbackName;
    CHECK_AND_RETURN_LOG(event->callback != nullptr, "event is nullptr");
    napi_env env = event->callback->env_;
    napi_ref callback = event->callback->cb_;

    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "%{public}s scope is nullptr", request.c_str());
    do {
        napi_value jsCallback = nullptr;
        napi_status nstatus = napi_get_reference_value(env, callback, &jsCallback);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && jsCallback != nullptr, "%{public}s get reference value failed",
            request.c_str());
        napi_value args[ARGS_ONE] = { nullptr };
        nstatus = napi_create_external_arraybuffer(env, event->bufDesc.buffer, event->bufDesc.dataLength,
            [](napi_env env, void *data, void *hint) {}, nullptr, &args[PARAM0]);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[PARAM0] != nullptr,
            "%{public}s callback fail to create buffer", request.c_str());
        const size_t argCount = 1;
        napi_value result = nullptr;
        nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok, "fail to call %{public}s callback", request.c_str());

        CHECK_AND_BREAK_LOG(event->capturerNapiObj != nullptr && event->capturerNapiObj->audioCapturer_ != nullptr,
            "audioCapturer_ is null");
        event->capturerNapiObj->audioCapturer_->Enqueue(event->bufDesc);
    } while (0);
    napi_close_handle_scope(env, scope);
}
} // namespace AudioStandard
} // namespace OHOS
