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
#define LOG_TAG "NapiRendererDataRequestCallback"
#endif

#include "js_native_api.h"
#include "napi_renderer_data_request_callback.h"

#include "audio_errors.h"
#include "audio_renderer_log.h"

namespace OHOS {
namespace AudioStandard {
namespace {
    const std::string RENDERER_DATA_REQUEST_CALLBACK_NAME = "dataRequest";
}

NapiRendererDataRequestCallback::NapiRendererDataRequestCallback(napi_env env, NapiAudioRenderer *napiRenderer)
    : env_(env), napiRenderer_(napiRenderer)
{
    AUDIO_INFO_LOG("instance create");
}

NapiRendererDataRequestCallback::~NapiRendererDataRequestCallback()
{
    if (regArDataReqTsfn_) {
        napi_release_threadsafe_function(arDataReqTsfn_, napi_tsfn_abort);
    }
    AUDIO_INFO_LOG("instance destroy");
}

void NapiRendererDataRequestCallback::SaveCallbackReference(const std::string &callbackName, napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    // create function that will operate while save callback reference success.
    std::function<void(std::shared_ptr<AutoRef> generatedCallback)> successed =
        [this](std::shared_ptr<AutoRef> generatedCallback) {
        rendererDataRequestCallback_ = generatedCallback;
    };
    NapiAudioRendererCallbackInner::SaveCallbackReferenceInner(callbackName, args, successed);
    AUDIO_DEBUG_LOG("SaveAudioRendererDataRequestCallback sucessful");
}

std::shared_ptr<AutoRef> NapiRendererDataRequestCallback::GetCallback(const std::string &callbackName)
{
    return rendererDataRequestCallback_;
}

void NapiRendererDataRequestCallback::RemoveCallbackReference(
    const std::string &callbackName, napi_env env, napi_value callback, napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    //create function that will operate while save callback reference success.
    std::function<void()> successed = [this]() {
        rendererDataRequestCallback_ = nullptr;
    };
    RemoveCallbackReferenceInner(callbackName, env, callback, successed);
}

napi_env &NapiRendererDataRequestCallback::GetEnv()
{
    return env_;
}

bool NapiRendererDataRequestCallback::CheckIfTargetCallbackName(const std::string &callbackName)
{
    return (callbackName == DATA_REQUEST_CALLBACK_NAME);
}

void NapiRendererDataRequestCallback::CreateWriteDataTsfn(napi_env env)
{
    regArDataReqTsfn_ = true;
    std::string callbackName = "writeData";
    napi_value cbName;
    napi_create_string_utf8(env, callbackName.c_str(), callbackName.length(), &cbName);
    napi_create_threadsafe_function(env, nullptr, nullptr, cbName, 0, 1, nullptr,
        DataRequestTsfnFinalize, nullptr, SafeJsCallbackDataRequestWork, &arDataReqTsfn_);
}

void NapiRendererDataRequestCallback::OnWriteData(size_t length)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_DEBUG_LOG("onDataRequest enqueue added");
    CHECK_AND_RETURN_LOG(rendererDataRequestCallback_ != nullptr, "Cannot find the reference of dataRequest callback");
    CHECK_AND_RETURN_LOG(napiRenderer_ != nullptr, "Cannot find the reference to audio renderer napi");
    std::unique_ptr<RendererDataRequestJsCallback> cb = std::make_unique<RendererDataRequestJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
    cb->callback = rendererDataRequestCallback_;
    cb->callbackName = DATA_REQUEST_CALLBACK_NAME;
    size_t reqLen = length;
    cb->bufDesc_.buffer = nullptr;
    cb->rendererNapiObj = napiRenderer_;
    napiRenderer_->audioRenderer_->GetBufferDesc(cb->bufDesc_);
    if (cb->bufDesc_.buffer == nullptr) {
        return;
    }
    if (reqLen > cb->bufDesc_.bufLength) {
        cb->bufDesc_.dataLength = cb->bufDesc_.bufLength;
    } else {
        cb->bufDesc_.dataLength = reqLen;
    }
    AudioRendererDataInfo audioRendererDataInfo = {};
    audioRendererDataInfo.buffer = cb->bufDesc_.buffer;
    audioRendererDataInfo.flag =  cb->bufDesc_.bufLength;
    cb->audioRendererDataInfo = audioRendererDataInfo;
    return OnJsRendererDataRequestCallback(cb);
}

void NapiRendererDataRequestCallback::SafeJsCallbackDataRequestWork(
    napi_env env, napi_value js_cb, void *context, void *data)
{
    RendererDataRequestJsCallback *event = reinterpret_cast<RendererDataRequestJsCallback *>(data);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsRendererDataRequestCallback: no memory");
    std::shared_ptr<RendererDataRequestJsCallback> safeContext(
        static_cast<RendererDataRequestJsCallback*>(data),
        [](RendererDataRequestJsCallback *ptr) {
            delete ptr;
    });
    std::string request = event->callbackName;
    napi_ref callback = event->callback->cb_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    AUDIO_INFO_LOG("SafeJsCallbackDataRequestWork: safe js callback working.");

    do {
        napi_value jsCallback = nullptr;
        napi_status nstatus = napi_get_reference_value(env, callback, &jsCallback);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && jsCallback != nullptr, "%{public}s get reference value fail",
            request.c_str());

        napi_value args[ARGS_ONE] = { nullptr };
        NapiParamUtils::SetNativeAudioRendererDataInfo(env, event->audioRendererDataInfo, args[0]);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[PARAM0] != nullptr,
            "%{public}s fail to create position callback", request.c_str());
        const size_t argCount = 1;
        napi_value result = nullptr;
        nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
        event->rendererNapiObj->audioRenderer_->Enqueue(event->bufDesc_);
            CHECK_AND_BREAK_LOG(nstatus == napi_ok, "%{public}s fail to call position callback", request.c_str());
    } while (0);
    napi_close_handle_scope(env, scope);
}

void NapiRendererDataRequestCallback::DataRequestTsfnFinalize(napi_env env, void *data, void *hint)
{
    AUDIO_INFO_LOG("DataRequestTsfnFinalize: safe thread resource release.");
}

void NapiRendererDataRequestCallback::OnJsRendererDataRequestCallback(
    std::unique_ptr<RendererDataRequestJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsRendererDataRequestCallback: jsCb.get() is null");
        return;
    }

    RendererDataRequestJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");

    napi_acquire_threadsafe_function(arDataReqTsfn_);
    napi_call_threadsafe_function(arDataReqTsfn_, event, napi_tsfn_blocking);
}
}  // namespace AudioStandard
}  // namespace OHOS