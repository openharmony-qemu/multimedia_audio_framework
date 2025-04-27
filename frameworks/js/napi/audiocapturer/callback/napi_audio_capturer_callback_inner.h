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
#ifndef NAPI_AUDIO_CAPTURER_CALLBACK_INNER_H
#define NAPI_AUDIO_CAPTURER_CALLBACK_INNER_H
#include "napi_async_work.h"

namespace OHOS {
namespace AudioStandard {
class NapiAudioCapturerCallbackInner {
public:
    virtual void SaveCallbackReference(const std::string &callbackName, napi_value args) = 0;
    virtual void RemoveCallbackReference(const std::string &callbackName, napi_env env, napi_value callback) = 0;
    virtual bool CheckIfTargetCallbackName(const std::string &callbackName) = 0;
    void SaveCallbackReferenceInner(const std::string &callbackName, napi_value args,
        std::function<void(std::shared_ptr<AutoRef> generatedCallback)> successed);
    bool ContainSameJsCallbackInner(const std::string &callbackName, napi_value args);
    void RemoveCallbackReferenceInner(const std::string &callbackName, napi_env env, napi_value callback,
        std::function<void()> successed);
protected:
    virtual std::shared_ptr<AutoRef> GetCallback(const std::string &callbackName) = 0;
    virtual napi_env &GetEnv() = 0;
};

}  // namespace AudioStandard
}  // namespace OHOS
#endif /* NAPI_AUDIO_CAPTURER_CALLBACK_INNER_H */