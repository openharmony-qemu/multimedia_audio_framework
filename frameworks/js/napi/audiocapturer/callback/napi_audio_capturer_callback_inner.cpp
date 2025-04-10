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
#undef LOG_TAG
#define LOG_TAG "NapiAudioCapturerCallbackInner"

#include "napi_audio_capturer_callback_inner.h"

namespace OHOS {
namespace AudioStandard {

void NapiAudioCapturerCallbackInner::SaveCallbackReferenceInner(const std::string &callbackName, napi_value args,
    std::function<void(std::shared_ptr<AutoRef> generatedCallback)> successed)
{
    CHECK_AND_RETURN_LOG(CheckIfTargetCallbackName(callbackName),
        "NapiAudioCapturerCallbackInner-> SaveCallbackReferenceInner Unknown callback type: %{public}s",
        callbackName.c_str());
    napi_ref callback = nullptr;
    const int32_t refCount = 1;
    napi_env env = GetEnv();
    napi_status status = napi_create_reference(env, args, refCount, &callback);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr, "Creating reference for callback fail");

    if (successed != nullptr) {
        std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env, callback);
        successed(cb);
    }
};

bool NapiAudioCapturerCallbackInner::ContainSameJsCallbackInner(const std::string &callbackName, napi_value args)
{
    CHECK_AND_RETURN_RET_LOG(CheckIfTargetCallbackName(callbackName), false,
        "NapiAudioCapturerCallbackInner->ContainSameJsCallbackInner Unknown callback type: %{public}s",
        callbackName.c_str());
    CHECK_AND_RETURN_RET_LOG(args != nullptr, false, "args is nullptr");
    napi_env env = GetEnv();
    std::shared_ptr<AutoRef> callbackCur = GetCallback(callbackName);
    napi_value copyValue = nullptr;
    CHECK_AND_RETURN_RET_LOG(env != nullptr, false, "env is null");
    CHECK_AND_RETURN_RET_LOG(callbackCur != nullptr, false, "callbackCur is null");
    CHECK_AND_RETURN_RET_LOG(callbackCur->cb_ != nullptr, false, "callbackCur.cb_ is null");

    napi_get_reference_value(env, callbackCur->cb_, &copyValue);
    bool isEquals = false;
    CHECK_AND_RETURN_RET_LOG(
        napi_strict_equals(env, copyValue, args, &isEquals) == napi_ok, false, "Get napi_strict_equals failed");
    return isEquals;
};

void NapiAudioCapturerCallbackInner::RemoveCallbackReferenceInner(
    const std::string &callbackName, napi_env env, napi_value callback, std::function<void()> successed)
{
    CHECK_AND_RETURN_LOG(CheckIfTargetCallbackName(callbackName),
        "NapiAudioCapturerCallbackInner->RemoveCallbackReferenceInner Unknown callback type: %{public}s",
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
        "NapiAudioCapturerCallbackInner->RemoveCallbackReferenceInner wrong callback to remove %{public}s",
        callbackName.c_str());
    callbackCur = nullptr;
    if (successed != nullptr) {
        successed();
    }
};
}  // namespace AudioStandard
}  // namespace OHOS