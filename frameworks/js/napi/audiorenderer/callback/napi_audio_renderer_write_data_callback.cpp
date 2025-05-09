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
#define LOG_TAG "NapiRendererWriteDataCallback"
#endif

#include "js_native_api.h"
#include "napi_audio_renderer_write_data_callback.h"
#include "audio_renderer_log.h"
#include "napi_audio_enum.h"

namespace OHOS {
namespace AudioStandard {
static const int32_t WRITE_CALLBACK_TIMEOUT_IN_MS = 1000; // 1s

#if defined(ANDROID_PLATFORM) || defined(IOS_PLATFORM)
vector<NapiAudioRenderer*> NapiRendererWriteDataCallback::activeRenderers_;
#endif
NapiRendererWriteDataCallback::NapiRendererWriteDataCallback(napi_env env, NapiAudioRenderer *napiRenderer)
    : env_(env), napiRenderer_(napiRenderer)
{
    AUDIO_DEBUG_LOG("instance create");
#if defined(ANDROID_PLATFORM) || defined(IOS_PLATFORM)
    activeRenderers_.emplace_back(napiRenderer_);
#endif
}

NapiRendererWriteDataCallback::~NapiRendererWriteDataCallback()
{
    AUDIO_DEBUG_LOG("instance destroy");
#if defined(ANDROID_PLATFORM) || defined(IOS_PLATFORM)
    auto iter = std::find(activeRenderers_.begin(), activeRenderers_.end(), napiRenderer_);
    if (iter != activeRenderers_.end()) {
        activeRenderers_.erase(iter);
    }
#endif
    if (regArWriteDataTsfn_) {
        napi_release_threadsafe_function(arWriteDataTsfn_, napi_tsfn_abort);
    }
}

void NapiRendererWriteDataCallback::AddCallbackReference(const std::string &callbackName, napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callback = nullptr;
    const int32_t refCount = 1;
    napi_status status = napi_create_reference(env_, args, refCount, &callback);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr, "creating reference for callback failed");

    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback);
    if (callbackName == WRITE_DATA_CALLBACK_NAME) {
        rendererWriteDataCallback_ = cb;
    } else {
        AUDIO_ERR_LOG("Unknown callback type: %{public}s", callbackName.c_str());
    }
}

void NapiRendererWriteDataCallback::CreateWriteDTsfn(napi_env env)
{
    regArWriteDataTsfn_ = true;
    std::string callbackName = "writeData";
    napi_value cbName;
    napi_create_string_utf8(env, callbackName.c_str(), callbackName.length(), &cbName);
    napi_create_threadsafe_function(env, nullptr, nullptr, cbName, 0, 1, nullptr,
        WriteDataTsfnFinalize, nullptr, SafeJsCallbackWriteDataWork, &arWriteDataTsfn_);
}

bool NapiRendererWriteDataCallback::GetWriteDTsfnFlag()
{
    return regArWriteDataTsfn_;
}

void NapiRendererWriteDataCallback::RemoveCallbackReference(napi_env env, napi_value callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    bool isEquals = false;
    napi_value copyValue = nullptr;

    if (callback == nullptr) {
        napi_status ret = napi_delete_reference(env, rendererWriteDataCallback_->cb_);
        CHECK_AND_RETURN_LOG(napi_ok == ret, "delete callback reference failed");
        rendererWriteDataCallback_->cb_ = nullptr;
        AUDIO_INFO_LOG("Remove Js Callback");
        return;
    }

    napi_get_reference_value(env, rendererWriteDataCallback_->cb_, &copyValue);
    CHECK_AND_RETURN_LOG(copyValue != nullptr, "copyValue is nullptr");
    CHECK_AND_RETURN_LOG(napi_strict_equals(env, callback, copyValue, &isEquals) == napi_ok,
        "get napi_strict_equals failed");
    if (isEquals) {
        AUDIO_INFO_LOG("found Js Callback, delete it!");
        napi_status status = napi_delete_reference(env, rendererWriteDataCallback_->cb_);
        CHECK_AND_RETURN_LOG(status == napi_ok, "deleting reference for callback failed");
        rendererWriteDataCallback_->cb_ = nullptr;
    }
}

void NapiRendererWriteDataCallback::OnWriteData(size_t length)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_LOG(rendererWriteDataCallback_ != nullptr, "Cannot find the reference of writeData callback");

    std::unique_ptr<RendererWriteDataJsCallback> cb = std::make_unique<RendererWriteDataJsCallback>();
    cb->callback = rendererWriteDataCallback_;
    cb->callbackName = WRITE_DATA_CALLBACK_NAME;
    cb->bufDesc.buffer = nullptr;
    cb->rendererNapiObj = napiRenderer_;

    CHECK_AND_RETURN_LOG(napiRenderer_ != nullptr, "Cannot find the reference to audio renderer napi");
    if (!napiRenderer_->audioRenderer_) {
        AUDIO_INFO_LOG("OnWriteData audioRenderer_ is null.");
        return;
    }
    napiRenderer_->audioRenderer_->GetBufferDesc(cb->bufDesc);
    if (cb->bufDesc.buffer == nullptr) {
        return;
    }
    if (length > cb->bufDesc.bufLength) {
        cb->bufDesc.dataLength = cb->bufDesc.bufLength;
    } else {
        cb->bufDesc.dataLength = length;
    }

    return OnJsRendererWriteDataCallback(cb);
}

void NapiRendererWriteDataCallback::OnJsRendererWriteDataCallback(std::unique_ptr<RendererWriteDataJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsRendererWriteDataCallback: jsCb.get() is null");
        return;
    }

    RendererWriteDataJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");

    auto obj = static_cast<NapiAudioRenderer *>(napiRenderer_);
    NapiAudioRenderer *napiRenderer = ObjectRefMap<NapiAudioRenderer>::IncreaseRef(obj);
    if (napiRenderer == nullptr) {
        AUDIO_ERR_LOG("napiRenderer is null");
        delete event;
        return;
    }
    
    napi_acquire_threadsafe_function(arWriteDataTsfn_);
    napi_call_threadsafe_function(arWriteDataTsfn_, event, napi_tsfn_blocking);

    if (napiRenderer_ == nullptr) {
        return;
    }
    std::unique_lock<std::mutex> writeCallbackLock(napiRenderer_->writeCallbackMutex_);
    bool ret = napiRenderer_->writeCallbackCv_.wait_for(writeCallbackLock,
        std::chrono::milliseconds(WRITE_CALLBACK_TIMEOUT_IN_MS), [this] () {
            return napiRenderer_->enqueued_;
        });
    if (!ret) {
        AUDIO_ERR_LOG("Client OnWriteData operation timed out");
    }
    napiRenderer_->enqueued_ = false;
    writeCallbackLock.unlock();
}

void NapiRendererWriteDataCallback::CheckWriteDataCallbackResult(napi_env env, BufferDesc &bufDesc, napi_value result)
{
    napi_valuetype resultType = napi_undefined;
    napi_typeof(env, result, &resultType);
    if (resultType == napi_number) {
        int32_t resultIntValue;
        napi_get_value_int32(env, result, &resultIntValue);
        auto resultValue = static_cast<NapiAudioEnum::AudioDataCallbackResult>(resultIntValue);
        if (resultValue == NapiAudioEnum::CALLBACK_RESULT_INVALID) {
            AUDIO_DEBUG_LOG("Data callback returned invalid, data will not be used.");
            bufDesc.dataLength = 0; // Ensure that the invalid data is not used.
        }
    }
}

void NapiRendererWriteDataCallback::SafeJsCallbackWriteDataWork(
    napi_env env, napi_value js_cb, void *context, void *data)
{
    RendererWriteDataJsCallback *event = reinterpret_cast<RendererWriteDataJsCallback *>(data);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");
    std::shared_ptr<RendererWriteDataJsCallback> safeContext(
        static_cast<RendererWriteDataJsCallback*>(data),
        [](RendererWriteDataJsCallback *ptr) {
            delete ptr;
    });
    WorkCallbackRendererWriteDataInner(event);
    CHECK_AND_RETURN_LOG(event->rendererNapiObj != nullptr, "NapiAudioRenderer object is nullptr");
    std::unique_lock<std::mutex> writeCallbackLock(event->rendererNapiObj->writeCallbackMutex_);
    event->rendererNapiObj->enqueued_ = true;
    event->rendererNapiObj->writeCallbackCv_.notify_all();
    writeCallbackLock.unlock();
    auto napiObj = static_cast<NapiAudioRenderer *>(event->rendererNapiObj);
    ObjectRefMap<NapiAudioRenderer>::DecreaseRef(napiObj);
}

void NapiRendererWriteDataCallback::WriteDataTsfnFinalize(napi_env env, void *data, void *hint)
{
    AUDIO_DEBUG_LOG("WriteDataTsfnFinalize: safe thread resource release.");
}

void NapiRendererWriteDataCallback::WorkCallbackRendererWriteDataInner(RendererWriteDataJsCallback *event)
{
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsRendererWriteDataCallback: no memory");
    std::string request = event->callbackName;
    napi_ref callback = event->callback->cb_;
    napi_env env = event->callback->env_;
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
        CheckWriteDataCallbackResult(env, event->bufDesc, result);
#if defined(ANDROID_PLATFORM) || defined(IOS_PLATFORM)
        auto iter = std::find(activeRenderers_.begin(), activeRenderers_.end(), event->rendererNapiObj);
        if (iter != activeRenderers_.end()) {
            if (event->rendererNapiObj->audioRenderer_) {
                event->rendererNapiObj->audioRenderer_->Enqueue(event->bufDesc);
            } else {
                AUDIO_INFO_LOG("WorkCallbackRendererWriteData audioRenderer_ is null");
            }
        } else {
            AUDIO_INFO_LOG("NapiRendererWriteDataCallback is finalize.");
        }
#else
        event->rendererNapiObj->audioRenderer_->Enqueue(event->bufDesc);
#endif
    } while (0);
    napi_close_handle_scope(env, scope);
}
}  // namespace AudioStandard
}  // namespace OHOS
