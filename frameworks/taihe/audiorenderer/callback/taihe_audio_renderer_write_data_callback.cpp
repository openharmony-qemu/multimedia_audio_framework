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
#include <algorithm>
#include <limits>
#include <optional>
#include "audio_errors.h"
#include "audio_renderer_log.h"
#include "taihe_audio_enum.h"
#include "taihe_param_utils.h"

namespace ANI::Audio {
static const int32_t WRITE_CALLBACK_TIMEOUT_IN_MS = 1000; // 1s
#if defined(ANDROID_PLATFORM) || defined(IOS_PLATFORM)
vector<AudioRendererImpl*> TaiheRendererWriteDataCallback::activeRenderers_;
#endif

TaiheRendererWriteDataCallback::TaiheRendererWriteDataCallback(AudioRendererImpl *taiheRenderer, size_t bufferSize)
    : taiheRenderer_(taiheRenderer), bufferSize_(bufferSize), callbackBuffer_(std::make_unique<uint8_t[]>(bufferSize))
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

    std::shared_ptr<RendererWriteDataJsCallback> cb = std::make_shared<RendererWriteDataJsCallback>();
    cb->callback = rendererWriteDataCallback_;
    cb->callbackName = WRITE_DATA_CALLBACK_NAME;
    cb->bufDesc.buffer = nullptr;
    cb->rendererTaiheObj = taiheRenderer_;

    CHECK_AND_RETURN_LOG(taiheRenderer_ != nullptr, "Cannot find the reference to audio renderer taihe");
    CHECK_AND_RETURN_LOG(taiheRenderer_->audioRenderer_ != nullptr, "OnWriteData audioRenderer_ is null.");
    CHECK_AND_RETURN_LOG(bufferSize_ >= length, "buffersize: %{public}zu, length: %{public}zu", bufferSize_, length);

    cb->bufDesc.buffer = callbackBuffer_.get();
    cb->bufDesc.bufLength = length;
    cb->bufDesc.dataLength = length;

    return OnJsRendererWriteDataCallback(cb);
}

void TaiheRendererWriteDataCallback::OnJsRendererWriteDataCallback(
    const std::shared_ptr<RendererWriteDataJsCallback> &jsCb)
{
    CHECK_AND_RETURN_LOG(jsCb != nullptr, "OnJsRendererWriteDataCallback: jsCb is null");
    CHECK_AND_RETURN_LOG(mainHandler_ != nullptr, "mainHandler_ is nullptr");
    CHECK_AND_RETURN_LOG(jsCb->callback != nullptr, "event is nullptr.");

    auto sharePtr = shared_from_this();
    auto task = [jsCb, sharePtr]() {
        if (sharePtr != nullptr) {
            sharePtr->SafeJsCallbackWriteDataWork(*jsCb);
        }
    };
    mainHandler_->PostTask(task, "OnWriteData", 0, OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE, {});

    CHECK_AND_RETURN_LOG(WaitForWriteDataCallbackComplete(jsCb), "Wait write data callback complete failed");

#if defined(ANDROID_PLATFORM) || defined(IOS_PLATFORM)
    auto iter = std::find(activeRenderers_.begin(), activeRenderers_.end(), jsCb->rendererTaiheObj);
    if (iter == activeRenderers_.end()) {
        AUDIO_INFO_LOG("TaiheRendererWriteDataCallback is finalize.");
        return;
    }
#endif
    CHECK_AND_RETURN_LOG(jsCb->rendererTaiheObj != nullptr && jsCb->rendererTaiheObj->audioRenderer_ != nullptr,
        "audioRenderer_ is null");
    int32_t ret = jsCb->rendererTaiheObj->audioRenderer_->Enqueue(jsCb->bufDesc);
    CHECK_AND_RETURN_LOG(ret == OHOS::AudioStandard::SUCCESS, "Enqueue failed, ret: %{public}d", ret);
}

bool TaiheRendererWriteDataCallback::WaitForWriteDataCallbackComplete(
    const std::shared_ptr<RendererWriteDataJsCallback> &jsCb)
{
    CHECK_AND_RETURN_RET_LOG(taiheRenderer_ != nullptr, false, "TaiheAudioRenderer object is nullptr");
    std::unique_lock<std::mutex> writeCallbackLock(taiheRenderer_->writeCallbackMutex_);
    bool ret = taiheRenderer_->writeCallbackCv_.wait_for(writeCallbackLock,
        std::chrono::milliseconds(WRITE_CALLBACK_TIMEOUT_IN_MS), [jsCb] () {
            return jsCb->enqueued;
        });
    CHECK_AND_RETURN_RET_LOG(ret, false, "Client OnWriteData operation timed out");
    return true;
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

void TaiheRendererWriteDataCallback::CopyWriteDataCallbackBuffer(OHOS::AudioStandard::BufferDesc &bufDesc,
    const taihe::array<uint8_t> &data)
{
    CHECK_AND_RETURN_LOG(bufDesc.buffer != nullptr, "bufDesc buffer is nullptr");
    size_t writeLength = std::min(data.size(), bufDesc.bufLength);
    std::copy_n(data.begin(), writeLength, bufDesc.buffer);
    bufDesc.dataLength = writeLength;
}

void TaiheRendererWriteDataCallback::SafeJsCallbackWriteDataWork(RendererWriteDataJsCallback &event)
{
    std::optional<AudioRendererWriteDataCallbackResult> callbackResult = CallWriteDataCallback(event);
    CHECK_AND_RETURN_LOG(event.rendererTaiheObj != nullptr, "TaiheAudioRenderer object is nullptr");
    std::lock_guard<std::mutex> writeCallbackLock(event.rendererTaiheObj->writeCallbackMutex_);
    if (callbackResult.has_value()) {
        CheckWriteDataCallbackResult(event.bufDesc, callbackResult->result);
        if (event.bufDesc.dataLength > 0) {
            CopyWriteDataCallbackBuffer(event.bufDesc, callbackResult->data);
        }
    } else {
        event.bufDesc.dataLength = 0;
    }
    event.enqueued = true;
    event.rendererTaiheObj->writeCallbackCv_.notify_all();
}

std::optional<AudioRendererWriteDataCallbackResult> TaiheRendererWriteDataCallback::CallWriteDataCallback(
    RendererWriteDataJsCallback &event)
{
    std::string request = event.callbackName;
    CHECK_AND_RETURN_RET_LOG(event.callback != nullptr, std::nullopt, "event is nullptr.");
    std::shared_ptr<taihe::callback<AudioRendererWriteDataCallbackResult(int32_t)>> cacheCallback =
        std::reinterpret_pointer_cast<taihe::callback<AudioRendererWriteDataCallbackResult(int32_t)>>(
            event.callback->cb_);
    CHECK_AND_RETURN_RET_LOG(cacheCallback != nullptr, std::nullopt, "%{public}s get reference value fail",
        request.c_str());
    CHECK_AND_RETURN_RET_LOG(event.bufDesc.bufLength <= static_cast<size_t>(std::numeric_limits<int32_t>::max()),
        std::nullopt, "bufLength is too large");
    AudioRendererWriteDataCallbackResult result = (*cacheCallback)(static_cast<int32_t>(event.bufDesc.bufLength));
    return result;
}
} // namespace ANI::Audio
