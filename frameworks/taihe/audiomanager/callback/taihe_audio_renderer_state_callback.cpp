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
#define LOG_TAG "TaiheAudioRendererStateCallback"
#endif

#include "taihe_audio_renderer_state_callback.h"
#include <mutex>
#include <thread>
#include "taihe_param_utils.h"

using namespace ANI::Audio;

namespace ANI::Audio {
std::mutex TaiheAudioRendererStateCallback::sWorkerMutex_;
TaiheAudioRendererStateCallback::TaiheAudioRendererStateCallback(ani_env *env)
    : env_(env)
{
    AUDIO_DEBUG_LOG("TaiheAudioRendererStateCallback: instance create");
}

TaiheAudioRendererStateCallback::~TaiheAudioRendererStateCallback()
{
    AUDIO_DEBUG_LOG("TaiheAudioRendererStateCallback: instance destroy");
}

void TaiheAudioRendererStateCallback::RemoveCallbackReference(std::shared_ptr<uintptr_t> callback)
{
    if (!IsSameCallback(callback)) {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    callback_ = nullptr;
    rendererStateCallback_->cb_ = nullptr;
    rendererStateCallback_.reset();
    AUDIO_DEBUG_LOG("Remove rendererStateCallback success");
}

bool TaiheAudioRendererStateCallback::IsSameCallback(std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (callback == callback_) {
        return true;
    }
    return false;
}

void TaiheAudioRendererStateCallback::SaveCallbackReference(const std::string &callbackName,
    std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_LOG(callback != nullptr, "TaiheAudioRendererStateCallback: creating reference for callback fail");
    callback_ = callback;
    ani_env *env = get_env();
    CHECK_AND_RETURN_LOG(env != nullptr, "get_env() fail");
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env, callback);
    rendererStateCallback_ = cb;
    std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
    CHECK_AND_RETURN_LOG(runner != nullptr, "runner is null");
    mainHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
}

void TaiheAudioRendererStateCallback::OnRendererStateChange(
    const std::vector<std::shared_ptr<OHOS::AudioStandard::AudioRendererChangeInfo>> &audioRendererChangeInfos)
{
    AUDIO_INFO_LOG("OnRendererStateChange entered");

    std::lock_guard<std::mutex> lock(mutex_);

    CHECK_AND_RETURN_LOG(rendererStateCallback_ != nullptr, "rendererStateCallback_ is nullptr!");

    std::unique_ptr<AudioRendererStateJsCallback> cb = std::make_unique<AudioRendererStateJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory!!");

    std::vector<std::shared_ptr<OHOS::AudioStandard::AudioRendererChangeInfo>> rendererChangeInfos;
    for (const auto &changeInfo : audioRendererChangeInfos) {
        rendererChangeInfos.push_back(std::make_shared<OHOS::AudioStandard::AudioRendererChangeInfo>(*changeInfo));
    }

    cb->callback = rendererStateCallback_;
    cb->changeInfos = move(rendererChangeInfos);

    return OnJsCallbackRendererState(cb);
}

void TaiheAudioRendererStateCallback::SafeJsCallbackRendererStateWork(ani_env *env, AudioRendererStateJsCallback *event)
{
    std::lock_guard<std::mutex> lock(sWorkerMutex_);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "SafeJsCallbackRendererStateWork: no memory");
    std::shared_ptr<AudioRendererStateJsCallback> safeContext(
        static_cast<AudioRendererStateJsCallback*>(event),
        [](AudioRendererStateJsCallback *ptr) {
            delete ptr;
    });

    do {
        std::shared_ptr<taihe::callback<void(taihe::array<AudioRendererChangeInfo>)>> cacheCallback =
            std::reinterpret_pointer_cast<taihe::callback<void(taihe::array<AudioRendererChangeInfo>)>>(
            event->callback->cb_);
        CHECK_AND_BREAK_LOG(cacheCallback != nullptr, "get reference value fail");
        taihe::array<AudioRendererChangeInfo> changeInfos = TaiheParamUtils::SetRendererChangeInfos(event->changeInfos);
        (*cacheCallback)(changeInfos);
    } while (0);
}

void TaiheAudioRendererStateCallback::OnJsCallbackRendererState(std::unique_ptr<AudioRendererStateJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackRendererState: jsCb.get() is null");
        return;
    }
    CHECK_AND_RETURN_LOG(mainHandler_ != nullptr, "mainHandler_ is nullptr");
    AudioRendererStateJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");
    auto sharePtr = shared_from_this();
    auto task = [event, sharePtr, this]() {
        if (sharePtr != nullptr) {
            sharePtr->SafeJsCallbackRendererStateWork(this->env_, event);
        }
    };
    mainHandler_->PostTask(task, "OnAudioRendererChange", 0, OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE, {});
}
} // namespace ANI::Audio
