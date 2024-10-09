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
    AUDIO_INFO_LOG("instance destroy");
}

void NapiRendererDataRequestCallback::SaveCallbackReference(const std::string &callbackName, napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callback = nullptr;
    const int32_t refCount = 1;
    napi_status status = napi_create_reference(env_, args, refCount, &callback);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr, "creating reference for callback fail");

    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback);
    if (callbackName == DATA_REQUEST_CALLBACK_NAME) {
        rendererDataRequestCallback_ = cb;
    } else {
        AUDIO_ERR_LOG("Unknown callback type: %{public}s", callbackName.c_str());
    }
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

void NapiRendererDataRequestCallback::WorkCallbackRendererDataRequest(uv_work_t *work, int status)
{
    // Js Thread
    std::shared_ptr<RendererDataRequestJsCallback> context(
        static_cast<RendererDataRequestJsCallback*>(work->data),
        [work](RendererDataRequestJsCallback* ptr) {
            delete ptr;
            delete work;
    });
    CHECK_AND_RETURN_LOG(work != nullptr, "WorkCallbackRendererDataRequest work is nullptr");
    RendererDataRequestJsCallback *event = reinterpret_cast<RendererDataRequestJsCallback *>(work->data);
    CHECK_AND_RETURN_LOG(event != nullptr, "WorkCallbackRendererDataRequest event is nullptr");
    std::string request = event->callbackName;

    CHECK_AND_RETURN_LOG(event->callback != nullptr, "event->callback is nullptr");
    napi_env env = event->callback->env_;
    napi_ref callback = event->callback->cb_;

    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    AUDIO_DEBUG_LOG("JsCallBack %{public}s, uv_queue_work_with_qos start", request.c_str());
    do {
        CHECK_AND_BREAK_LOG(status != UV_ECANCELED, "%{public}s canceled", request.c_str());

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

void NapiRendererDataRequestCallback::OnJsRendererDataRequestCallback(
    std::unique_ptr<RendererDataRequestJsCallback> &jsCb)
{
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    CHECK_AND_RETURN_LOG(loop != nullptr, "loop is nullptr fail");

    uv_work_t *work = new(std::nothrow) uv_work_t;
    CHECK_AND_RETURN_LOG(work != nullptr, "OnJsRendererPeriodPositionCallback: No memory");

    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsRendererPeriodPositionCallback: jsCb.get() is null");
        delete work;
        return;
    }
    work->data = reinterpret_cast<void *>(jsCb.get());

    int ret = uv_queue_work_with_qos(loop, work, [] (uv_work_t *work) {},
        WorkCallbackRendererDataRequest, uv_qos_default);
    if (ret != 0) {
        AUDIO_ERR_LOG("Failed to execute libuv work queue");
        delete work;
    } else {
        jsCb.release();
    }
}
}  // namespace AudioStandard
}  // namespace OHOS