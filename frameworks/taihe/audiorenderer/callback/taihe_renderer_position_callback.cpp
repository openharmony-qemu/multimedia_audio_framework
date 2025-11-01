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
#define LOG_TAG "TaiheRendererPositionCallback"
#endif

#include "taihe_renderer_position_callback.h"
#include <thread>
#include "taihe_audio_renderer_callback.h"
#include "taihe_param_utils.h"

namespace ANI::Audio {
TaiheRendererPositionCallback::TaiheRendererPositionCallback()
{
    AUDIO_DEBUG_LOG("instance create");
}
TaiheRendererPositionCallback::~TaiheRendererPositionCallback()
{
    AUDIO_DEBUG_LOG("instance destroy");
}

void TaiheRendererPositionCallback::SaveCallbackReference(
    const std::string &callbackName, std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    // create function that will operate while save callback reference success.
    std::function<void(std::shared_ptr<AutoRef> generatedCallback)> successed =
        [this](std::shared_ptr<AutoRef> generatedCallback) {
        renderPositionCallback_ = generatedCallback;
    };
    TaiheAudioRendererCallbackInner::SaveCallbackReferenceInner(callbackName, callback, successed);
    if (!mainHandler_) {
        std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
        CHECK_AND_RETURN_LOG(runner != nullptr, "runner is null");
        mainHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    } else {
        AUDIO_DEBUG_LOG("mainHandler_ is not nullptr");
    }
}

std::shared_ptr<AutoRef> TaiheRendererPositionCallback::GetCallback(const std::string &callbackName)
{
    return renderPositionCallback_;
}

void TaiheRendererPositionCallback::RemoveCallbackReference(
    const std::string &callbackName, std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    //create function that will operate while save callback reference success.
    std::function<void()> successed = [this]() {
        renderPositionCallback_ = nullptr;
    };
    RemoveCallbackReferenceInner(callbackName, callback, successed);
}

bool TaiheRendererPositionCallback::CheckIfTargetCallbackName(const std::string &callbackName)
{
    return (callbackName == MARK_REACH_CALLBACK_NAME);
}

void TaiheRendererPositionCallback::OnMarkReached(const int64_t &framePosition)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_DEBUG_LOG("mark reached");
    CHECK_AND_RETURN_LOG(renderPositionCallback_ != nullptr, "Cannot find the reference of position callback");

    std::unique_ptr<RendererPositionJsCallback> cb = std::make_unique<RendererPositionJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
    cb->callback = renderPositionCallback_;
    cb->callbackName = MARK_REACH_CALLBACK_NAME;
    cb->position = framePosition;
    return OnJsRendererPositionCallback(cb);
}

void TaiheRendererPositionCallback::OnJsRendererPositionCallback(
    std::unique_ptr<RendererPositionJsCallback> &jsCb)
{
    if (jsCb == nullptr || jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsRendererPositionCallback: jsCb.get() is null");
        return;
    }
    CHECK_AND_RETURN_LOG(mainHandler_ != nullptr, "mainHandler_ is nullptr");
    RendererPositionJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");
    auto sharePtr = shared_from_this();
    auto task = [event, sharePtr]() {
        if (sharePtr != nullptr) {
            sharePtr->SafeJsCallbackPositionWork(event);
        }
    };
    mainHandler_->PostTask(task, "OnMarkReach", 0, OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE, {});
}

void TaiheRendererPositionCallback::SafeJsCallbackPositionWork(RendererPositionJsCallback *event)
{
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "SafeJsCallbackInterruptWork: no memory");
    std::shared_ptr<RendererPositionJsCallback> safeContext(
        static_cast<RendererPositionJsCallback*>(event),
        [](RendererPositionJsCallback *ptr) {
            if (ptr != nullptr) {
                delete ptr;
            }
    });
    std::string request = event->callbackName;
    do {
        std::shared_ptr<taihe::callback<void(int64_t)>> cacheCallback =
            std::reinterpret_pointer_cast<taihe::callback<void(int64_t)>>(event->callback->cb_);
        CHECK_AND_BREAK_LOG(cacheCallback != nullptr, "%{public}s get reference value fail", request.c_str());
        (*cacheCallback)(event->position);
    } while (0);
}
} // namespace ANI::Audio
