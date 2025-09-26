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
#define LOG_TAG "TaiheAudioCapturerCallbackInner"
#endif

#include "taihe_audio_capturer_callback_inner.h"
#include "audio_log.h"
#include "taihe_param_utils.h"

namespace ANI::Audio {
void TaiheAudioCapturerCallbackInner::SaveCallbackReferenceInner(const std::string &callbackName,
    std::shared_ptr<uintptr_t> &callback, std::function<void(std::shared_ptr<AutoRef> generatedCallback)> successed)
{
    CHECK_AND_RETURN_LOG(CheckIfTargetCallbackName(callbackName),
        "TaiheAudioCapturerCallbackInner-> SaveCallbackReferenceInner Unknown callback type: %{public}s",
        callbackName.c_str());
    CHECK_AND_RETURN_LOG(callback != nullptr, "Creating reference for callback fail");

    if (successed != nullptr) {
        std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(callback);
        CHECK_AND_RETURN_LOG(cb != nullptr, "Memory allocation failed!!");
        successed(cb);
    }
}

bool TaiheAudioCapturerCallbackInner::ContainSameJsCallbackInner(const std::string &callbackName,
    std::shared_ptr<uintptr_t> &callback)
{
    CHECK_AND_RETURN_RET_LOG(CheckIfTargetCallbackName(callbackName), false,
        "TaiheAudioCapturerCallbackInner->ContainSameJsCallbackInner Unknown callback type: %{public}s",
        callbackName.c_str());

    std::shared_ptr<AutoRef> callbackCur = GetCallback(callbackName);
    CHECK_AND_RETURN_RET_LOG(callbackCur != nullptr, false, "callbackCur is null");
    CHECK_AND_RETURN_RET_LOG(callbackCur->cb_ != nullptr, false, "callbackCur.cb_ is null");
    return TaiheParamUtils::IsSameRef(callback, callbackCur->cb_);
}

void TaiheAudioCapturerCallbackInner::RemoveCallbackReferenceInner(
    const std::string &callbackName, std::shared_ptr<uintptr_t> &callback, std::function<void()> successed)
{
    CHECK_AND_RETURN_LOG(CheckIfTargetCallbackName(callbackName),
        "TaiheAudioCapturerCallbackInner->RemoveCallbackReferenceInner Unknown callback type: %{public}s",
        callbackName.c_str());
    std::shared_ptr<AutoRef> callbackCur = GetCallback(callbackName);
    if (callback == nullptr) {
        callbackCur = nullptr;
        if (successed != nullptr) {
            successed();
        }
        return;
    }

    CHECK_AND_RETURN_LOG(ContainSameJsCallbackInner(callbackName, callback),
        "TaiheAudioCapturerCallbackInner->RemoveCallbackReferenceInner wrong callback to remove %{public}s",
        callbackName.c_str());
    callbackCur = nullptr;
    if (successed != nullptr) {
        successed();
    }
}
} // namespace ANI::Audio
