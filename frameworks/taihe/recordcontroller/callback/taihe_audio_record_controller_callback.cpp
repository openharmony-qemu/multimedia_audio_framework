/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "TaiheAudioRecordControllerCallback"
#endif

#include "taihe_audio_record_controller_callback.h"
#include "taihe_param_utils.h"
#include "taihe_audio_error.h"
#include "taihe_audio_enum.h"

namespace ANI::Audio {
TaiheAudioRecordControllerCallback::TaiheAudioRecordControllerCallback()
{
    AUDIO_DEBUG_LOG("instance create");
}

TaiheAudioRecordControllerCallback::~TaiheAudioRecordControllerCallback()
{
    AUDIO_DEBUG_LOG("instance destroy");
}

void TaiheAudioRecordControllerCallback::OnRecordControllerEnabledChange(
    const OHOS::AudioStandard::SystemRecordControllerChangeInfo &changeInfo)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_LOG(enabledChangeCallback_ != nullptr, "callback not registered by JS client");

    std::unique_ptr<RecordControllerChangeJsCallback> cb = std::make_unique<RecordControllerChangeJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
    cb->callback = enabledChangeCallback_;
    cb->callbackName = RECORD_CONTROLLER_ENABLED_CHANGE_CALLBACK_NAME;
    cb->changeInfo = changeInfo;
    return OnJsCallbackRecordControllerChange(cb);
}

void TaiheAudioRecordControllerCallback::SaveCallbackReference(const std::string &callbackName,
    std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::function<void(std::shared_ptr<AutoRef> generatedCallback)> successed =
        [this, callbackName](std::shared_ptr<AutoRef> generatedCallback) {
        if (callbackName == RECORD_CONTROLLER_ENABLED_CHANGE_CALLBACK_NAME) {
            enabledChangeCallback_ = generatedCallback;
            return;
        }
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

void TaiheAudioRecordControllerCallback::RemoveCallbackReference(const std::string &callbackName,
    std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::function<void()> successed = [this, callbackName]() {
        if (callbackName == RECORD_CONTROLLER_ENABLED_CHANGE_CALLBACK_NAME) {
            enabledChangeCallback_ = nullptr;
            return;
        }
    };
    RemoveCallbackReferenceInner(callbackName, callback, successed);
}

std::shared_ptr<AutoRef> TaiheAudioRecordControllerCallback::GetCallback(const std::string &callbackName)
{
    std::shared_ptr<AutoRef> cb = nullptr;
    if (callbackName == RECORD_CONTROLLER_ENABLED_CHANGE_CALLBACK_NAME) {
        return enabledChangeCallback_;
    }
    AUDIO_ERR_LOG("TaiheAudioRecordControllerCallback->GetCallback Unknown callback type: %{public}s",
        callbackName.c_str());
    return cb;
}

bool TaiheAudioRecordControllerCallback::CheckIfTargetCallbackName(const std::string &callbackName)
{
    if (callbackName == RECORD_CONTROLLER_ENABLED_CHANGE_CALLBACK_NAME) {
        return true;
    }
    return false;
}

void TaiheAudioRecordControllerCallback::SafeJsCallbackRecordControllerChangeWork(
    RecordControllerChangeJsCallback *event)
{
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackRecordControllerChange: no memory");
    std::shared_ptr<RecordControllerChangeJsCallback> safeContext(
        static_cast<RecordControllerChangeJsCallback*>(event),
        [](RecordControllerChangeJsCallback *ptr) {
            if (ptr != nullptr) {
                delete ptr;
                ptr = nullptr;
            }
    });
    std::string request = event->callbackName;
    do {
        std::shared_ptr<taihe::callback<void(SystemRecordControllerChangeInfo const&)>> cacheCallback =
            std::reinterpret_pointer_cast<taihe::callback<void(SystemRecordControllerChangeInfo const&)>>(
                event->callback->cb_);
        CHECK_AND_BREAK_LOG(cacheCallback != nullptr, "%{public}s get reference value fail", request.c_str());
        SystemRecordControllerChangeInfo result = TaiheParamUtils::ToTaiheSystemRecordControllerChangeInfo(
            event->changeInfo);
        (*cacheCallback)(result);
    } while (0);
}

void TaiheAudioRecordControllerCallback::OnJsCallbackRecordControllerChange(
    std::unique_ptr<RecordControllerChangeJsCallback> &jsCb)
{
    CHECK_AND_RETURN_LOG(mainHandler_, "mainHandler_ is null");
    CHECK_AND_RETURN_LOG(jsCb && jsCb->callback, "callback is null");
    RecordControllerChangeJsCallback *event = jsCb.release();
    auto sharePtr = shared_from_this();
    auto task = [event, sharePtr]() {
        if (sharePtr != nullptr) {
            sharePtr->SafeJsCallbackRecordControllerChangeWork(event);
        }
    };
    mainHandler_->PostTask(task, "OnRecordControllerEnabledChange", 0,
        OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE, {});
}
} // namespace ANI::Audio
