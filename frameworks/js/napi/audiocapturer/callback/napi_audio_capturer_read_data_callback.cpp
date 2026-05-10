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
#include "audio_errors.h"
#include "napi_param_utils.h"
#include <cstring>
#include <memory>
#include <new>

namespace OHOS {
namespace AudioStandard {
static const int32_t READ_CALLBACK_TIMEOUT_IN_MS = 1000; // 1s

static napi_status CreateExternalArrayBufferProperty(const napi_env &env, const char *fieldStr, size_t bufferLen,
    const uint8_t *bufferData, napi_value &result)
{
    if (bufferLen > 0 && bufferData == nullptr) {
        AUDIO_ERR_LOG("CreateExternalArrayBufferProperty invalid input buffer");
        return napi_invalid_arg;
    }
    size_t allocLen = (bufferLen > 0) ? bufferLen : 1;
    std::unique_ptr<uint8_t[]> externalData = std::make_unique<uint8_t[]>(allocLen);
    CHECK_AND_RETURN_RET_LOG(externalData != nullptr, napi_generic_failure,
        "CreateExternalArrayBufferProperty alloc failed");
    if (bufferLen > 0) {
        errno_t copyRet = memcpy_s(externalData.get(), allocLen, bufferData, bufferLen);
        if (copyRet != EOK) {
            AUDIO_ERR_LOG("CreateExternalArrayBufferProperty memcpy_s failed, ret:%{public}d", copyRet);
            return napi_generic_failure;
        }
    }

    napi_value value = nullptr;
    napi_status status = napi_create_external_arraybuffer(env, externalData.get(), bufferLen,
        [](napi_env env, void *data, void *hint) {
            delete[] static_cast<uint8_t *>(data);
        }, nullptr, &value);
    if (status != napi_ok) {
        return status;
    }
    // Ownership has been transferred to Node.js via the external ArrayBuffer finalizer.
    (void)externalData.release();

    status = napi_set_named_property(env, result, fieldStr, value);
    if (status != napi_ok) {
        AUDIO_ERR_LOG("napi_set_named_property failed");
        return status;
    }
    return status;
}

NapiCapturerReadDataCallback::NapiCapturerReadDataCallback(napi_env env, NapiAudioCapturer *napiCapturer)
    : env_(env), napiCapturer_(napiCapturer)
{
    AUDIO_DEBUG_LOG("instance create");
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
    std::string taskName = "NapiCapturerReadDataCallback::destroy";
    napi_ref callback = nullptr;
    const int32_t refCount = 1;
    napi_status status = napi_create_reference(env_, args, refCount, &callback);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr, "creating reference for callback failed");

    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback, taskName);
    if (callbackName == READ_DATA_CALLBACK_NAME) {
        capturerReadDataCallback_ = cb;
        isCallbackInited_ = true;
    } else if (callbackName == READ_MICIN_DATA_CALLBACK_NAME) {
        capturerReadMicInDataCallback_ = cb;
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

bool NapiCapturerReadDataCallback::HasCallbackReference(const std::string &callbackName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (callbackName == READ_DATA_CALLBACK_NAME) {
        return capturerReadDataCallback_ != nullptr;
    }
    if (callbackName == READ_MICIN_DATA_CALLBACK_NAME) {
        return capturerReadMicInDataCallback_ != nullptr;
    }
    return false;
}

bool NapiCapturerReadDataCallback::HasAnyCallbackReference()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return capturerReadDataCallback_ != nullptr || capturerReadMicInDataCallback_ != nullptr;
}

void NapiCapturerReadDataCallback::RemoveCallbackReference(napi_env env, napi_value callback,
    const std::string &callbackName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::shared_ptr<AutoRef> targetCallback = nullptr;
    if (callbackName == READ_DATA_CALLBACK_NAME) {
        targetCallback = capturerReadDataCallback_;
    } else if (callbackName == READ_MICIN_DATA_CALLBACK_NAME) {
        targetCallback = capturerReadMicInDataCallback_;
    } else {
        AUDIO_ERR_LOG("Unknown callback type: %{public}s", callbackName.c_str());
        return;
    }

    CHECK_AND_RETURN_LOG(targetCallback != nullptr, "target callback is nullptr");
    bool isEquals = false;
    napi_value copyValue = nullptr;

    if (callback == nullptr) {
        napi_status ret = napi_delete_reference(env, targetCallback->cb_);
        CHECK_AND_RETURN_LOG(napi_ok == ret, "delete callback reference failed");
        AUDIO_INFO_LOG("Remove Js Callback");
        targetCallback->cb_ = nullptr;
        if (callbackName == READ_DATA_CALLBACK_NAME) {
            capturerReadDataCallback_ = nullptr;
        } else {
            capturerReadMicInDataCallback_ = nullptr;
        }
        isCallbackInited_ = (capturerReadDataCallback_ != nullptr || capturerReadMicInDataCallback_ != nullptr);
        return;
    }

    napi_get_reference_value(env, targetCallback->cb_, &copyValue);
    CHECK_AND_RETURN_LOG(copyValue != nullptr, "copyValue is nullptr");
    CHECK_AND_RETURN_LOG(napi_strict_equals(env, callback, copyValue, &isEquals) == napi_ok,
        "get napi_strict_equals failed");
    if (isEquals) {
        AUDIO_INFO_LOG("found JS Callback, delete it!");
        napi_status status = napi_delete_reference(env, targetCallback->cb_);
        CHECK_AND_RETURN_LOG(status == napi_ok, "deleting reference for callback failed");
        targetCallback->cb_ = nullptr;
        if (callbackName == READ_DATA_CALLBACK_NAME) {
            capturerReadDataCallback_ = nullptr;
        } else {
            capturerReadMicInDataCallback_ = nullptr;
        }
        isCallbackInited_ = (capturerReadDataCallback_ != nullptr || capturerReadMicInDataCallback_ != nullptr);
    }
}

bool NapiCapturerReadDataCallback::PrepareMicInReadData(CapturerReadDataJsCallback &jsCb)
{
    size_t processBufSize = 0;
    size_t micInBufSize = 0;
    size_t ecBufSize = 0;
    int32_t ret = napiCapturer_->audioCapturer_->GetMicInBufferSize(jsCb.bufDesc,
        processBufSize, micInBufSize, ecBufSize);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("GetMicInBufferSize failed");
        napiCapturer_->audioCapturer_->Enqueue(jsCb.bufDesc);
        return false;
    }
    jsCb.processDataLength = processBufSize;
    jsCb.micInDataLength = micInBufSize;
    jsCb.ecDataLength = ecBufSize;
    jsCb.processData.resize(processBufSize);
    jsCb.micInData.resize(micInBufSize);
    jsCb.ecData.resize(ecBufSize);

    BufferDesc processBufDesc {};
    BufferDesc micInBufDesc {};
    BufferDesc ecBufDesc {};
    processBufDesc.buffer = jsCb.processData.data();
    processBufDesc.bufLength = jsCb.processData.size();
    processBufDesc.dataLength = 0;
    micInBufDesc.buffer = jsCb.micInData.data();
    micInBufDesc.bufLength = jsCb.micInData.size();
    micInBufDesc.dataLength = 0;
    ecBufDesc.buffer = jsCb.ecData.data();
    ecBufDesc.bufLength = jsCb.ecData.size();
    ecBufDesc.dataLength = 0;
    ret = napiCapturer_->audioCapturer_->DeinterleaveBuffer(jsCb.bufDesc, processBufDesc, micInBufDesc, ecBufDesc);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("DeinterleaveBuffer failed");
        napiCapturer_->audioCapturer_->Enqueue(jsCb.bufDesc);
        return false;
    }
    return true;
}

void NapiCapturerReadDataCallback::OnReadData(size_t length)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_LOG(napiCapturer_ != nullptr, "Cannot find the reference to audio capturer napi");
    CHECK_AND_RETURN_LOG(napiCapturer_->audioCapturer_ != nullptr, "audioCapturer is null");
    if (!isCallbackInited_) {
        BufferDesc emptyDesc {};
        napiCapturer_->audioCapturer_->GetBufferDesc(emptyDesc);
        if (emptyDesc.buffer != nullptr) {
            napiCapturer_->audioCapturer_->Enqueue(emptyDesc);
        }
        return;
    }

    std::shared_ptr<AutoRef> targetCallback = nullptr;
    std::string targetCallbackName = READ_DATA_CALLBACK_NAME;
    if (capturerReadMicInDataCallback_ != nullptr) {
        targetCallback = capturerReadMicInDataCallback_;
        targetCallbackName = READ_MICIN_DATA_CALLBACK_NAME;
    } else if (capturerReadDataCallback_ != nullptr) {
        targetCallback = capturerReadDataCallback_;
    }
    if (targetCallback == nullptr) {
        BufferDesc emptyDesc {};
        napiCapturer_->audioCapturer_->GetBufferDesc(emptyDesc);
        if (emptyDesc.buffer != nullptr) {
            napiCapturer_->audioCapturer_->Enqueue(emptyDesc);
        }
        return;
    }

    std::unique_ptr<CapturerReadDataJsCallback> cb = std::make_unique<CapturerReadDataJsCallback>();
    cb->callback = targetCallback;
    cb->callbackName = targetCallbackName;
    cb->bufDesc.buffer = nullptr;
    cb->capturerNapiObj = napiCapturer_;
    cb->readDataCallbackPtr = this;
    napiCapturer_->audioCapturer_->GetBufferDesc(cb->bufDesc);
    if (cb->bufDesc.buffer == nullptr) {
        return;
    }
    cb->bufDesc.dataLength = (length > cb->bufDesc.bufLength) ? cb->bufDesc.bufLength : length;
    if (targetCallbackName == READ_MICIN_DATA_CALLBACK_NAME && !PrepareMicInReadData(*cb)) {
        return;
    }

    return OnJsCapturerReadDataCallback(cb);
}

void NapiCapturerReadDataCallback::OnJsCapturerReadDataCallback(std::unique_ptr<CapturerReadDataJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("readData Js Callback is null");
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
    AUDIO_INFO_LOG("CaptureReadDataTsfnFinalize: safe thread resource release.");
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
            ptr = nullptr;
    });
    SafeJsCallbackCapturerReadDataWorkInner(event);

    CHECK_AND_RETURN_LOG(event->capturerNapiObj != nullptr, "NapiAudioCapturer object is nullptr");
    std::unique_lock<std::mutex> readCallbackLock(event->capturerNapiObj->readCallbackMutex_);
    event->capturerNapiObj->isFrameCallbackDone_.store(true);
    event->capturerNapiObj->readCallbackCv_.notify_all();
    readCallbackLock.unlock();
    auto napiObj = static_cast<NapiAudioCapturer *>(event->capturerNapiObj);
    ObjectRefMap<NapiAudioCapturer>::DecreaseRef(napiObj);
}

void NapiCapturerReadDataCallback::SafeJsCallbackCapturerReadDataWorkInner(CapturerReadDataJsCallback *event)
{
    CHECK_AND_RETURN_LOG(event != nullptr, "capture read data event is nullptr");
    CHECK_AND_RETURN_LOG(event->readDataCallbackPtr != nullptr, "CapturerReadDataCallback is already released");
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
        if (request == READ_MICIN_DATA_CALLBACK_NAME) {
            nstatus = napi_create_object(env, &args[PARAM0]);
            CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[PARAM0] != nullptr,
                "%{public}s callback fail to create object", request.c_str());
            nstatus = CreateExternalArrayBufferProperty(env, "data", event->processDataLength,
                event->processData.data(), args[PARAM0]);
            CHECK_AND_BREAK_LOG(nstatus == napi_ok, "%{public}s callback fail to create data buffer",
                request.c_str());
            nstatus = CreateExternalArrayBufferProperty(env, "micInData", event->micInDataLength,
                event->micInData.data(), args[PARAM0]);
            CHECK_AND_BREAK_LOG(nstatus == napi_ok, "%{public}s callback fail to create micInData buffer",
                request.c_str());
            if (event->ecDataLength > 0) {
                nstatus = CreateExternalArrayBufferProperty(env, "ecData", event->ecDataLength,
                    event->ecData.data(), args[PARAM0]);
                CHECK_AND_BREAK_LOG(nstatus == napi_ok, "%{public}s callback fail to create ecData buffer",
                    request.c_str());
            }
        } else {
            nstatus = napi_create_external_arraybuffer(env, event->bufDesc.buffer, event->bufDesc.dataLength,
                [](napi_env env, void *data, void *hint) {}, nullptr, &args[PARAM0]);
            CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[PARAM0] != nullptr,
                "%{public}s callback fail to create buffer", request.c_str());
        }
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
