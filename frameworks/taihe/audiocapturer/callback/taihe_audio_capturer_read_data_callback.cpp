/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "TaiheCapturerReadDataCallback"
#endif

#include "taihe_audio_capturer_read_data_callback.h"
#include <mutex>
#include <thread>
#include "audio_capturer_log.h"
#include "taihe_audio_capturer_callbacks.h"
#include "taihe_param_utils.h"

namespace ANI::Audio {
static bool g_taiheAudioCapturerIsNullptr = true;
static std::mutex g_asynccallbackMutex;
static const int32_t READ_CALLBACK_TIMEOUT_IN_MS = 1000; // 1s

TaiheCapturerReadDataCallback::TaiheCapturerReadDataCallback(AudioCapturerImpl *taiheCapturer)
    : taiheCapturer_(taiheCapturer)
{
    AUDIO_DEBUG_LOG("instance create");
    g_taiheAudioCapturerIsNullptr = false;
}

TaiheCapturerReadDataCallback::~TaiheCapturerReadDataCallback()
{
    AUDIO_DEBUG_LOG("instance destroy");
}

void TaiheCapturerReadDataCallback::AddCallbackReference(const std::string &callbackName,
    std::shared_ptr<uintptr_t> &callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_LOG(callback != nullptr, "creating reference for callback failed");

    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(callback);
    CHECK_AND_RETURN_LOG(cb != nullptr, "Memory allocation failed!!");
    if (callbackName == READ_DATA_CALLBACK_NAME) {
        capturerReadDataCallback_ = cb;
        isCallbackInited_ = true;
    } else {
        AUDIO_ERR_LOG("Unknown callback type: %{public}s", callbackName.c_str());
    }
    if (!mainHandler_) {
        std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
        CHECK_AND_RETURN_LOG(runner != nullptr, "runner is null");
        mainHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    } else {
        AUDIO_DEBUG_LOG("mainHandler_ is not nullptr");
    }
}

void TaiheCapturerReadDataCallback::RemoveCallbackReference(std::shared_ptr<uintptr_t> &callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_LOG(capturerReadDataCallback_ != nullptr, "capturerReadDataCallback_ is null");
    isCallbackInited_ = false;

    if (callback == nullptr) {
        AUDIO_INFO_LOG("Remove Js Callback");
        capturerReadDataCallback_->cb_ = nullptr;
        return;
    }

    if (TaiheParamUtils::IsSameRef(callback, capturerReadDataCallback_->cb_)) {
        AUDIO_INFO_LOG("found JS Callback, delete it!");
        capturerReadDataCallback_->cb_ = nullptr;
    }
}

void TaiheCapturerReadDataCallback::RemoveTaiheCapturer()
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::lock_guard<std::mutex> asyncLock(g_asynccallbackMutex);
    taiheCapturer_ = nullptr;
    g_taiheAudioCapturerIsNullptr = true;
}

void TaiheCapturerReadDataCallback::OnReadData(size_t length)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_LOG(capturerReadDataCallback_ != nullptr, "Cannot find the reference of readData callback");

    std::unique_ptr<CapturerReadDataJsCallback> cb = std::make_unique<CapturerReadDataJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "Memory allocation failed!!");
    cb->callback = capturerReadDataCallback_;
    cb->callbackName = READ_DATA_CALLBACK_NAME;
    cb->bufDesc.buffer = nullptr;
    cb->capturerTaiheObj = taiheCapturer_;
    cb->readDataCallbackPtr = this;

    CHECK_AND_RETURN_LOG(taiheCapturer_ != nullptr, "Cannot find the reference to audio capturer taihe");
    CHECK_AND_RETURN_LOG(taiheCapturer_->audioCapturer_ != nullptr, "audioCapturer is null");
    taiheCapturer_->audioCapturer_->GetBufferDesc(cb->bufDesc);
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

void TaiheCapturerReadDataCallback::OnJsCapturerReadDataCallback(std::unique_ptr<CapturerReadDataJsCallback> &jsCb)
{
    if (jsCb == nullptr || jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCapturerReadDataCallback: jsCb.get() is null");
        return;
    }
    CHECK_AND_RETURN_LOG(mainHandler_ != nullptr, "mainHandler_ is nullptr");
    CapturerReadDataJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCapturerReadDataCallback: event is nullptr.");
    if (taiheCapturer_ == nullptr) {
        return;
    }
    taiheCapturer_->isFrameCallbackDone_.store(false);

    auto sharePtr = shared_from_this();
    auto task = [event, sharePtr]() {
        if (sharePtr != nullptr) {
            sharePtr->SafeJsCallbackCapturerReadDataWork(event);
        }
    };
    mainHandler_->PostTask(task, "OnReadData", 0, OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE, {});

    std::unique_lock<std::mutex> readCallbackLock(taiheCapturer_->readCallbackMutex_);
    bool isTimeout = !taiheCapturer_->readCallbackCv_.wait_for(readCallbackLock,
        std::chrono::milliseconds(READ_CALLBACK_TIMEOUT_IN_MS), [this] {
            return taiheCapturer_->isFrameCallbackDone_.load();
        });
    if (isTimeout) {
        AUDIO_ERR_LOG("Client OnReadData operation timed out");
    }
    readCallbackLock.unlock();
}

void TaiheCapturerReadDataCallback::SafeJsCallbackCapturerReadDataWork(CapturerReadDataJsCallback *event)
{
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "SafeJsCallbackCapturerReadDataWork: no memory");
    std::shared_ptr<CapturerReadDataJsCallback> safeContext(
        static_cast<CapturerReadDataJsCallback*>(event),
        [](CapturerReadDataJsCallback *ptr) {
            if (ptr != nullptr) {
                delete ptr;
            }
    });
    CHECK_AND_RETURN_LOG(event->readDataCallbackPtr != nullptr, "CapturerReadDataCallback is already released");
    CHECK_AND_RETURN_LOG(event->readDataCallbackPtr->isCallbackInited_, "the callback has been dereferenced");
    std::string request = event->callbackName;
    do {
        std::shared_ptr<taihe::callback<void(array_view<uint8_t>)>> cacheCallback =
            std::reinterpret_pointer_cast<taihe::callback<void(array_view<uint8_t>)>>(event->callback->cb_);
        CHECK_AND_BREAK_LOG(cacheCallback != nullptr, "%{public}s get reference value fail", request.c_str());
        (*cacheCallback)(TaiheParamUtils::ToTaiheArrayBuffer(event->bufDesc.buffer, event->bufDesc.dataLength));
        CHECK_AND_BREAK_LOG(event->capturerTaiheObj != nullptr && event->capturerTaiheObj->audioCapturer_ != nullptr,
            "audioCapturer_ is null");
        event->capturerTaiheObj->audioCapturer_->Enqueue(event->bufDesc);
    } while (0);
    CHECK_AND_RETURN_LOG(event->capturerTaiheObj != nullptr, "TaiheAudioCapturer object is nullptr");
    event->capturerTaiheObj->isFrameCallbackDone_.store(true);
    event->capturerTaiheObj->readCallbackCv_.notify_all();
}
} // namespace ANI::Audio
