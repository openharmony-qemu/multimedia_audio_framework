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
#define LOG_TAG "TaiheRendererPeriodPositionCallback"
#endif

#include "taihe_renderer_period_position_callback.h"
#include <thread>
#include "taihe_audio_renderer_callback.h"
#include "taihe_param_utils.h"

namespace ANI::Audio {
TaiheRendererPeriodPositionCallback::TaiheRendererPeriodPositionCallback()
{
    AUDIO_DEBUG_LOG("instance create");
}
TaiheRendererPeriodPositionCallback::~TaiheRendererPeriodPositionCallback()
{
    AUDIO_DEBUG_LOG("instance destroy");
}

void TaiheRendererPeriodPositionCallback::SaveCallbackReference(
    const std::string &callbackName, std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    //create function that will operate while save callback reference success.
    std::function<void(std::shared_ptr<AutoRef> generatedCallback)> successed =
        [this](std::shared_ptr<AutoRef> generatedCallback) {
        renderPeriodPositionCallback_ = generatedCallback;
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

std::shared_ptr<AutoRef> TaiheRendererPeriodPositionCallback::GetCallback(const std::string &callbackName)
{
    return renderPeriodPositionCallback_;
}

void TaiheRendererPeriodPositionCallback::RemoveCallbackReference(
    const std::string &callbackName, std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    //create function that will operate while save callback reference success.
    std::function<void()> successed = [this]() {
        renderPeriodPositionCallback_ = nullptr;
    };
    RemoveCallbackReferenceInner(callbackName, callback, successed);
}

bool TaiheRendererPeriodPositionCallback::CheckIfTargetCallbackName(const std::string &callbackName)
{
    return (callbackName == PERIOD_REACH_CALLBACK_NAME);
}

void TaiheRendererPeriodPositionCallback::OnPeriodReached(const int64_t &frameNumber)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_DEBUG_LOG("period reached");
    CHECK_AND_RETURN_LOG(renderPeriodPositionCallback_ != nullptr, "Cannot find the reference of position callback");

    std::unique_ptr<RendererPeriodPositionJsCallback> cb = std::make_unique<RendererPeriodPositionJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
    cb->callback = renderPeriodPositionCallback_;
    cb->callbackName = PERIOD_REACH_CALLBACK_NAME;
    cb->position = frameNumber;
    return OnJsRendererPeriodPositionCallback(cb);
}

void TaiheRendererPeriodPositionCallback::OnJsRendererPeriodPositionCallback(
    std::unique_ptr<RendererPeriodPositionJsCallback> &jsCb)
{
    if (jsCb == nullptr || jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsRendererPeriodPositionCallback: jsCb.get() is null");
        return;
    }
    CHECK_AND_RETURN_LOG(mainHandler_ != nullptr, "mainHandler_ is nullptr");
    RendererPeriodPositionJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");
    auto sharePtr = shared_from_this();
    auto task = [event, sharePtr]() {
        if (sharePtr != nullptr) {
            sharePtr->SafeJsCallbackPeriodPositionWork(event);
        }
    };
    mainHandler_->PostTask(task, "OnPeriodReach", 0, OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE, {});
}

void TaiheRendererPeriodPositionCallback::SafeJsCallbackPeriodPositionWork(RendererPeriodPositionJsCallback *event)
{
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "SafeJsCallbackInterruptWork: no memory");
    std::shared_ptr<RendererPeriodPositionJsCallback> safeContext(
        static_cast<RendererPeriodPositionJsCallback*>(event),
        [](RendererPeriodPositionJsCallback *ptr) {
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
