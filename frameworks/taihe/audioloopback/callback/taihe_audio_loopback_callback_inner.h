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
#ifndef TAIHE_AUDIO_LOOPBACK_CALLBACK_INNER_H
#define TAIHE_AUDIO_LOOPBACK_CALLBACK_INNER_H
#include "taihe_work.h"

namespace ANI::Audio {
using namespace taihe;
using namespace ohos::multimedia::audio;

class TaiheAudioLoopbackCallbackInner {
public:
    virtual void SaveCallbackReference(const std::string &callbackName, std::shared_ptr<uintptr_t> callback) = 0;
    virtual void RemoveCallbackReference(const std::string &callbackName, std::shared_ptr<uintptr_t> callback) = 0;
    virtual bool CheckIfTargetCallbackName(const std::string &callbackName) = 0;
    void SaveCallbackReferenceInner(const std::string &callbackName, std::shared_ptr<uintptr_t> callback,
        std::function<void(std::shared_ptr<AutoRef> generatedCallback)> successed);
    bool ContainSameJsCallbackInner(const std::string &callbackName, std::shared_ptr<uintptr_t> callback);
    void RemoveCallbackReferenceInner(const std::string &callbackName, std::shared_ptr<uintptr_t> callback,
        std::function<void()> successed = nullptr);
protected:
    virtual std::shared_ptr<AutoRef> GetCallback(const std::string &callbackName) = 0;
};
} // namespace ANI::Audio
#endif // TAIHE_AUDIO_LOOPBACK_CALLBACK_INNER_H