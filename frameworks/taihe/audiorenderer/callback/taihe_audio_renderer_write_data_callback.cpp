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
#define LOG_TAG "TaiheRendererWriteDataCallback"
#endif

#include "taihe_audio_renderer_write_data_callback.h"
#include "audio_renderer_log.h"
#include "taihe_audio_enum.h"
#include "taihe_param_utils.h"

namespace ANI::Audio {
static const int32_t WRITE_CALLBACK_TIMEOUT_IN_MS = 1000; // 1s
#if defined(ANDROID_PLATFORM) || defined(IOS_PLATFORM)
vector<AudioRendererImpl*> TaiheRendererWriteDataCallback::activeRenderers_;
#endif
TaiheRendererWriteDataCallback::TaiheRendererWriteDataCallback(AudioRendererImpl *taiheRenderer)
    : taiheRenderer_(taiheRenderer)
{
    AUDIO_DEBUG_LOG("instance create");
#if defined(ANDROID_PLATFORM) || defined(IOS_PLATFORM)
    activeRenderers_.emplace_back(taiheRenderer_);
#endif
}

TaiheRendererWriteDataCallback::~TaiheRendererWriteDataCallback()
{
    AUDIO_DEBUG_LOG("instance destroy");
#if defined(ANDROID_PLATFORM) || defined(IOS_PLATFORM)
    auto iter = std::find(activeRenderers_.begin(), activeRenderers_.end(), taiheRenderer_);
    if (iter != activeRenderers_.end()) {
        activeRenderers_.erase(iter);
    }
#endif
}

void TaiheRendererWriteDataCallback::AddCallbackReference(const std::string &callbackName,
    std::shared_ptr<uintptr_t> &callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_LOG(callback != nullptr, "creating reference for callback failed");

    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(callback);
    CHECK_AND_RETURN_LOG(cb != nullptr, "Memory allocation failed!!");
    if (callbackName == WRITE_DATA_CALLBACK_NAME) {
        rendererWriteDataCallback_ = cb;
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

void TaiheRendererWriteDataCallback::RemoveCallbackReference(std::shared_ptr<uintptr_t> &callback)
{
    CHECK_AND_RETURN_LOG(rendererWriteDataCallback_ != nullptr, "Cannot find the reference of writeData callback");
    std::lock_guard<std::mutex> lock(mutex_);
    if (callback == nullptr) {
        rendererWriteDataCallback_->cb_ = nullptr;
        AUDIO_INFO_LOG("Remove Js Callback");
        return;
    }

    CHECK_AND_RETURN_LOG(rendererWriteDataCallback_->cb_ != nullptr, "rendererWriteDataCallback_->cb_ is nullptr");
    if (TaiheParamUtils::IsSameRef(callback, rendererWriteDataCallback_->cb_)) {
        AUDIO_INFO_LOG("found Js Callback, delete it!");
        rendererWriteDataCallback_->cb_ = nullptr;
    }
}

void TaiheRendererWriteDataCallback::OnWriteData(size_t length)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_LOG(rendererWriteDataCallback_ != nullptr, "Cannot find the reference of writeData callback");

    std::unique_ptr<RendererWriteDataJsCallback> cb = std::make_unique<RendererWriteDataJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "Memory allocation failed!!");
    cb->callback = rendererWriteDataCallback_;
    cb->callbackName = WRITE_DATA_CALLBACK_NAME;
    cb->bufDesc.buffer = nullptr;
    cb->rendererTaiheObj = taiheRenderer_;

    CHECK_AND_RETURN_LOG(taiheRenderer_ != nullptr, "Cannot find the reference to audio renderer taihe");
    if (!taiheRenderer_->audioRenderer_) {
        AUDIO_INFO_LOG("OnWriteData audioRenderer_ is null.");
        return;
    }
    taiheRenderer_->audioRenderer_->GetBufferDesc(cb->bufDesc);
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

void TaiheRendererWriteDataCallback::OnJsRendererWriteDataCallback(std::unique_ptr<RendererWriteDataJsCallback> &jsCb)
{
    if (jsCb == nullptr || jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsRendererWriteDataCallback: jsCb.get() is null");
        return;
    }

    CHECK_AND_RETURN_LOG(mainHandler_ != nullptr, "mainHandler_ is nullptr");
    RendererWriteDataJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");

    auto sharePtr = shared_from_this();
    auto task = [event, sharePtr]() {
        if (sharePtr != nullptr) {
            sharePtr->SafeJsCallbackWriteDataWork(event);
        }
    };
    mainHandler_->PostTask(task, "OnWriteData", 0, OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE, {});

    if (taiheRenderer_ == nullptr) {
        return;
    }
    std::unique_lock<std::mutex> writeCallbackLock(taiheRenderer_->writeCallbackMutex_);
    bool ret = taiheRenderer_->writeCallbackCv_.wait_for(writeCallbackLock,
        std::chrono::milliseconds(WRITE_CALLBACK_TIMEOUT_IN_MS), [this] () {
            return taiheRenderer_->enqueued_;
        });
    if (!ret) {
        AUDIO_ERR_LOG("Client OnWriteData operation timed out");
    }
    taiheRenderer_->enqueued_ = false;
    writeCallbackLock.unlock();
}

void TaiheRendererWriteDataCallback::CheckWriteDataCallbackResult(OHOS::AudioStandard::BufferDesc &bufDesc,
    AudioDataCallbackResult result)
{
    int32_t resultIntValue = result.get_value();
    auto resultValue = static_cast<TaiheAudioEnum::AudioDataCallbackResult>(resultIntValue);
    if (resultValue == TaiheAudioEnum::CALLBACK_RESULT_INVALID) {
        AUDIO_DEBUG_LOG("Data callback returned invalid, data will not be used.");
        bufDesc.dataLength = 0; // Ensure that the invalid data is not used.
    }
}

void TaiheRendererWriteDataCallback::SafeJsCallbackWriteDataWork(RendererWriteDataJsCallback *event)
{
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");
    std::shared_ptr<RendererWriteDataJsCallback> safeContext(
        static_cast<RendererWriteDataJsCallback*>(event),
        [](RendererWriteDataJsCallback *ptr) {
            if (ptr != nullptr) {
                delete ptr;
            }
    });
    std::string request = event->callbackName;
    do {
        std::shared_ptr<taihe::callback<AudioDataCallbackResult(array_view<uint8_t>)>> cacheCallback =
            std::reinterpret_pointer_cast<taihe::callback<AudioDataCallbackResult(array_view<uint8_t>)>>(
                event->callback->cb_);
        CHECK_AND_BREAK_LOG(cacheCallback != nullptr, "%{public}s get reference value fail", request.c_str());
        AudioDataCallbackResult result =
            (*cacheCallback)(TaiheParamUtils::ToTaiheArrayBuffer(event->bufDesc.buffer, event->bufDesc.dataLength));
        CheckWriteDataCallbackResult(event->bufDesc, result);
        CHECK_AND_BREAK_LOG(event->rendererTaiheObj != nullptr && event->rendererTaiheObj->audioRenderer_ != nullptr,
            "audioRenderer_ is null");
#if defined(ANDROID_PLATFORM) || defined(IOS_PLATFORM)
        auto iter = std::find(activeRenderers_.begin(), activeRenderers_.end(), event->rendererTaiheObj);
        if (iter != activeRenderers_.end()) {
            if (event->rendererTaiheObj->audioRenderer_) {
                event->rendererTaiheObj->audioRenderer_->Enqueue(event->bufDesc);
            } else {
                AUDIO_INFO_LOG("WorkCallbackRendererWriteData audioRenderer_ is null");
            }
        } else {
            AUDIO_INFO_LOG("TaiheRendererWriteDataCallback is finalize.");
        }
#else
        event->rendererTaiheObj->audioRenderer_->Enqueue(event->bufDesc);
#endif
    } while (0);
    CHECK_AND_RETURN_LOG(event->rendererTaiheObj != nullptr, "TaiheAudioRenderer object is nullptr");
    std::unique_lock<std::mutex> writeCallbackLock(event->rendererTaiheObj->writeCallbackMutex_);
    event->rendererTaiheObj->enqueued_ = true;
    event->rendererTaiheObj->writeCallbackCv_.notify_all();
    writeCallbackLock.unlock();
}
} // namespace ANI::Audio