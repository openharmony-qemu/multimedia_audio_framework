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
#define LOG_TAG "TaiheAudioCapturerStateCallback"
#endif

#include <mutex>
#include <thread>
#include "taihe_audio_capturer_state_callback.h"
#include "taihe_param_utils.h"

namespace ANI::Audio {
TaiheAudioCapturerStateCallback::TaiheAudioCapturerStateCallback()
{
    AUDIO_DEBUG_LOG("TaiheAudioCapturerStateCallback: instance create");
}

TaiheAudioCapturerStateCallback::~TaiheAudioCapturerStateCallback()
{
    AUDIO_DEBUG_LOG("TaiheAudioCapturerStateCallback: instance destroy");
}

void TaiheAudioCapturerStateCallback::SaveCallbackReference(const std::string &callbackName,
    std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_LOG(callback != nullptr, "TaiheAudioCapturerStateCallback: creating reference for callback fail");
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(callback);
    CHECK_AND_RETURN_LOG(cb != nullptr, "Memory allocation failed!!");
    capturerStateCallback_ = cb;
    if (!mainHandler_) {
        std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
        CHECK_AND_RETURN_LOG(runner != nullptr, "runner is null");
        mainHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    } else {
        AUDIO_DEBUG_LOG("mainHandler_ is not nullptr");
    }
}

void TaiheAudioCapturerStateCallback::RemoveCallbackReference(std::shared_ptr<uintptr_t> callback)
{
    CHECK_AND_RETURN_LOG(capturerStateCallback_ != nullptr, "capturerStateCallback_ is nullptr");
    if (!IsSameCallback(callback)) {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    capturerStateCallback_->cb_ = nullptr;
    capturerStateCallback_.reset();
    AUDIO_DEBUG_LOG("Remove capturerStateCallback success");
}

bool TaiheAudioCapturerStateCallback::IsSameCallback(std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (capturerStateCallback_ == nullptr) {
        return false;
    }
    if (callback == nullptr) {
        return true;
    }
    return TaiheParamUtils::IsSameRef(callback, capturerStateCallback_->cb_);
}

void TaiheAudioCapturerStateCallback::OnCapturerStateChange(
    const std::vector<std::shared_ptr<OHOS::AudioStandard::AudioCapturerChangeInfo>> &audioCapturerChangeInfos)
{
    AUDIO_INFO_LOG("OnCapturerStateChange is called");

    std::lock_guard<std::mutex> lock(mutex_);
    std::unique_ptr<AudioCapturerStateJsCallback> cb = std::make_unique<AudioCapturerStateJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory!!");

    std::vector<std::shared_ptr<OHOS::AudioStandard::AudioCapturerChangeInfo>> capturerChangeInfos;
    for (const auto &changeInfo : audioCapturerChangeInfos) {
        capturerChangeInfos.push_back(std::make_shared<OHOS::AudioStandard::AudioCapturerChangeInfo>(*changeInfo));
    }

    cb->callback = capturerStateCallback_;
    cb->changeInfos = move(capturerChangeInfos);

    return OnJsCallbackCapturerState(cb);
}

void TaiheAudioCapturerStateCallback::SafeJsCallbackCapturerStateWork(AudioCapturerStateJsCallback *event)
{
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "SafeJsCallbackCapturerStateWork: no memory");
    std::shared_ptr<AudioCapturerStateJsCallback> safeContext(
        static_cast<AudioCapturerStateJsCallback*>(event),
        [](AudioCapturerStateJsCallback *ptr) {
            if (ptr != nullptr) {
                delete ptr;
            }
    });

    do {
        std::shared_ptr<taihe::callback<void(taihe::array_view<AudioCapturerChangeInfo>)>> cacheCallback =
            std::reinterpret_pointer_cast<taihe::callback<void(taihe::array_view<AudioCapturerChangeInfo>)>>(
                event->callback->cb_);
        CHECK_AND_BREAK_LOG(cacheCallback != nullptr, "get reference value fail");
        taihe::array<AudioCapturerChangeInfo> changeInfos = TaiheParamUtils::SetCapturerChangeInfos(event->changeInfos);
        (*cacheCallback)(changeInfos);
    } while (0);
}

void TaiheAudioCapturerStateCallback::OnJsCallbackCapturerState(std::unique_ptr<AudioCapturerStateJsCallback> &jsCb)
{
    if (jsCb == nullptr || jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackRendererState: jsCb.get() is null");
        return;
    }
    CHECK_AND_RETURN_LOG(mainHandler_ != nullptr, "mainHandler_ is nullptr");
    AudioCapturerStateJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");
    auto sharePtr = shared_from_this();
    auto task = [event, sharePtr]() {
        if (sharePtr != nullptr) {
            sharePtr->SafeJsCallbackCapturerStateWork(event);
        }
    };
    mainHandler_->PostTask(task, "OnAudioCapturerChange", 0, OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE, {});
}
} // namespace ANI::Audio