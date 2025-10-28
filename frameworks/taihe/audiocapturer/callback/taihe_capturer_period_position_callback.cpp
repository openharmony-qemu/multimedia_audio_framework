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
#define LOG_TAG "TaiheCapturerPeriodPositionCallback"
#endif

#include "taihe_capturer_period_position_callback.h"
#include <mutex>
#include <thread>
#include "audio_errors.h"
#include "audio_capturer_log.h"
#include "taihe_param_utils.h"
#include "taihe_audio_capturer_callbacks.h"

namespace ANI::Audio {
TaiheCapturerPeriodPositionCallback::TaiheCapturerPeriodPositionCallback()
{
    AUDIO_DEBUG_LOG("TaiheCapturerPeriodPositionCallback: instance create");
}

TaiheCapturerPeriodPositionCallback::~TaiheCapturerPeriodPositionCallback()
{
    AUDIO_DEBUG_LOG("TaiheCapturerPeriodPositionCallback: instance destroy");
}

void TaiheCapturerPeriodPositionCallback::SaveCallbackReference(const std::string &callbackName,
    std::shared_ptr<uintptr_t> &callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    // create function that will operate while save callback reference success.
    std::function<void(std::shared_ptr<AutoRef> generatedCallback)> successed =
        [this](std::shared_ptr<AutoRef> generatedCallback) {
        capturerPeriodPositionCallback_ = generatedCallback;
    };
    SaveCallbackReferenceInner(callbackName, callback, successed);
    if (!mainHandler_) {
        std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
        CHECK_AND_RETURN_LOG(runner != nullptr, "runner is null");
        mainHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    } else {
        AUDIO_DEBUG_LOG("mainHandler_ is not nullptr");
    }
}

void TaiheCapturerPeriodPositionCallback::RemoveCallbackReference(const std::string &callbackName,
    std::shared_ptr<uintptr_t> &callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    // create function that will operate while save callback reference success.
    std::function<void()> successed =
        [this]() {
        capturerPeriodPositionCallback_ = nullptr;
        };
    RemoveCallbackReferenceInner(callbackName, callback, successed);
}

void TaiheCapturerPeriodPositionCallback::OnPeriodReached(const int64_t &frameNumber)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_DEBUG_LOG("TaiheCapturerPeriodPositionCallback: period reached");
    CHECK_AND_RETURN_LOG(capturerPeriodPositionCallback_ != nullptr, "Cannot find the reference of position callback");

    std::unique_ptr<CapturerPeriodPositionJsCallback> cb = std::make_unique<CapturerPeriodPositionJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
    cb->callback = capturerPeriodPositionCallback_;
    cb->callbackName = PERIOD_REACH_CALLBACK_NAME;
    cb->position = frameNumber;
    return OnJsCapturerPeriodPositionCallback(cb);
}

void TaiheCapturerPeriodPositionCallback::SafeJsCallbackCapturerPeriodPositionWork(
    CapturerPeriodPositionJsCallback *event)
{
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackVolumeEvent: no memory");
    std::shared_ptr<CapturerPeriodPositionJsCallback> safeContext(
        static_cast<CapturerPeriodPositionJsCallback*>(event),
        [](CapturerPeriodPositionJsCallback *ptr) {
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

void TaiheCapturerPeriodPositionCallback::OnJsCapturerPeriodPositionCallback(
    std::unique_ptr<CapturerPeriodPositionJsCallback> &jsCb)
{
    if (jsCb == nullptr || jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCapturerPeriodPositionCallback: jsCb.get() is null");
        return;
    }
    CHECK_AND_RETURN_LOG(mainHandler_ != nullptr, "mainHandler_ is nullptr");
    CapturerPeriodPositionJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCapturerPeriodPositionCallback: event is nullptr.");

    auto sharePtr = shared_from_this();
    auto task = [event, sharePtr]() {
        if (sharePtr != nullptr) {
            sharePtr->SafeJsCallbackCapturerPeriodPositionWork(event);
        }
    };
    mainHandler_->PostTask(task, "OnPeriodReach", 0, OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE, {});
}

std::shared_ptr<AutoRef> TaiheCapturerPeriodPositionCallback::GetCallback(const std::string &callbackName)
{
    std::shared_ptr<AutoRef> cb = nullptr;
    if (callbackName == PERIOD_REACH_CALLBACK_NAME) {
        return capturerPeriodPositionCallback_;
    }
    return cb;
}

bool TaiheCapturerPeriodPositionCallback::CheckIfTargetCallbackName(const std::string &callbackName)
{
    if (callbackName == PERIOD_REACH_CALLBACK_NAME) {
        return true;
    }
    return false;
}
} // namespace ANI::Audio