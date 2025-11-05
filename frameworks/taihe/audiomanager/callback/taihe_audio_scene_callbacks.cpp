/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "TaiheAudioSceneChangedCallback"
#endif

#include "taihe_audio_scene_callbacks.h"
#include "audio_errors.h"
#include "audio_manager_log.h"
#include "taihe_param_utils.h"
#include "taihe_audio_error.h"
#include "taihe_audio_enum.h"

namespace ANI::Audio {
TaiheAudioSceneChangedCallback::TaiheAudioSceneChangedCallback()
{
    AUDIO_DEBUG_LOG("instance create");
}

TaiheAudioSceneChangedCallback::~TaiheAudioSceneChangedCallback()
{
    AUDIO_DEBUG_LOG("instance destroy");
}

void TaiheAudioSceneChangedCallback::SaveCallbackReference(const std::string &callbackName,
    std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_LOG(callbackName == AUDIO_SCENE_CHANGE_CALLBACK_NAME,
        "Unknown callback type: %{public}s", callbackName.c_str());
    for (auto &item : audioSceneChangeCbList_) {
        if (item == nullptr) {
            continue;
        }
        bool isSameCallback = TaiheParamUtils::IsSameRef(callback, item->cb_);
        CHECK_AND_RETURN_LOG(!isSameCallback, "has same callback, nothing to do");
    }

    CHECK_AND_RETURN_LOG(callback != nullptr, "TaiheAudioRendererStateCallback: creating reference for callback fail");
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(callback);
    CHECK_AND_RETURN_LOG(cb != nullptr, "Memory allocation failed!!");
    audioSceneChangeCbList_.push_back(cb);
    AUDIO_INFO_LOG("save callback ref success, list size [%{public}zu]", audioSceneChangeCbList_.size());

    if (!mainHandler_) {
        std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
        CHECK_AND_RETURN_LOG(runner != nullptr, "runner is null");
        mainHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    } else {
        AUDIO_DEBUG_LOG("mainHandler_ is not nullptr");
    }
}
void TaiheAudioSceneChangedCallback::RemoveCallbackReference(std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = audioSceneChangeCbList_.begin(); it != audioSceneChangeCbList_.end(); ++it) {
        std::shared_ptr<AutoRef> temp = (*it);
        if (temp == nullptr) {
            continue;
        }
        bool isSameCallback = TaiheParamUtils::IsSameRef(callback, temp->cb_);
        if (isSameCallback) {
            AUDIO_INFO_LOG("find audioSceneChanged callback, remove it");
            temp->cb_ = nullptr;
            audioSceneChangeCbList_.erase(it);
            return;
        }
    }
    AUDIO_INFO_LOG("remove audioSceneChanged callback no find");
}

void TaiheAudioSceneChangedCallback::RemoveAllCallbackReference()
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &item : audioSceneChangeCbList_) {
        if (item == nullptr) {
            continue;
        }
        item->cb_ = nullptr;
    }
    audioSceneChangeCbList_.clear();
    AUDIO_INFO_LOG("remove all js callback success");
}

int32_t TaiheAudioSceneChangedCallback::GetAudioSceneCbListSize()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return static_cast<int32_t>(audioSceneChangeCbList_.size());
}

void TaiheAudioSceneChangedCallback::OnAudioSceneChange(const OHOS::AudioStandard::AudioScene audioScene)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_INFO_LOG("audioScene status [%{public}d]", audioScene);

    for (auto &item : audioSceneChangeCbList_) {
        std::unique_ptr<AudioSceneJsCallback> cb = std::make_unique<AudioSceneJsCallback>();
        CHECK_AND_RETURN_LOG(cb != nullptr, "no memory");
        cb->callback = item;
        cb->callbackName = AUDIO_SCENE_CHANGE_CALLBACK_NAME;
        cb->audioScene = TaiheAudioEnum::GetJsAudioScene(audioScene);
        OnJsCallbackAudioSceneChange(cb);
    }
}

void TaiheAudioSceneChangedCallback::SafeJsCallbackAudioSceneChangeWork(AudioSceneJsCallback *event)
{
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event or event->callback is nullptr");
    std::shared_ptr<AudioSceneJsCallback> safeContext(
        static_cast<AudioSceneJsCallback*>(event),
        [](AudioSceneJsCallback *ptr) {
            if (ptr != nullptr) {
                delete ptr;
            }
    });
    std::string request = event->callbackName;
    AUDIO_INFO_LOG("safe js callback working.");
    do {
        std::shared_ptr<taihe::callback<void(AudioScene)>> cacheCallback =
            std::reinterpret_pointer_cast<taihe::callback<void(AudioScene)>>(event->callback->cb_);
        CHECK_AND_BREAK_LOG(cacheCallback != nullptr, "%{public}s get reference value fail", request.c_str());
        (*cacheCallback)(TaiheAudioEnum::ToTaiheAudioScene(event->audioScene));
    } while (0);
}

void TaiheAudioSceneChangedCallback::OnJsCallbackAudioSceneChange(std::unique_ptr<AudioSceneJsCallback> &jsCb)
{
    if (jsCb == nullptr || jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackAudioSceneChange: jsCb.get() is null");
        return;
    }
    CHECK_AND_RETURN_LOG(mainHandler_ != nullptr, "mainHandler_ is nullptr");
    AudioSceneJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event or event->callback is nullptr");
    auto sharePtr = shared_from_this();
    auto task = [event, sharePtr]() {
        if (sharePtr != nullptr) {
            sharePtr->SafeJsCallbackAudioSceneChangeWork(event);
        }
    };
    mainHandler_->PostTask(task, "OnAudioSceneChange", 0, OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE, {});
}
} // namespace ANI::Audio