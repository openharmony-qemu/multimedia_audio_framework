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

#ifndef TAIHE_AUDIO_MANAGER_INTERRUPT_CALLBACK_H
#define TAIHE_AUDIO_MANAGER_INTERRUPT_CALLBACK_H

#include "audio_system_manager.h"
#include "event_handler.h"
#include "taihe_work.h"

namespace ANI::Audio {
using namespace taihe;
using namespace ohos::multimedia::audio;

const std::string INTERRUPT_CALLBACK_NAME = "interrupt";

class TaiheAudioManagerInterruptCallback : public OHOS::AudioStandard::AudioManagerCallback,
    public std::enable_shared_from_this<TaiheAudioManagerInterruptCallback> {
public:
    explicit TaiheAudioManagerInterruptCallback(ani_env *env);
    virtual ~TaiheAudioManagerInterruptCallback();
    void SaveCallbackReference(const std::string &callbackName, std::shared_ptr<uintptr_t> &callback);
    void RemoveCallbackReference(const std::string &callbackName, std::shared_ptr<uintptr_t> &callback);
    void RemoveAllCallbackReferences(const std::string &callbackName);
    int32_t GetInterruptCallbackListSize();
    void OnInterrupt(const OHOS::AudioStandard::InterruptAction &interruptAction) override;
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> mainHandler_ = nullptr;

private:
    struct AudioManagerInterruptJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        OHOS::AudioStandard::InterruptAction interruptAction;
    };

    void OnJsCallbackAudioManagerInterrupt(std::unique_ptr<AudioManagerInterruptJsCallback> &jsCb);
    static void SafeJsCallbackAudioManagerInterruptWork(ani_env *env, AudioManagerInterruptJsCallback *event);

    std::mutex mutex_;
    ani_env *env_ = nullptr;
    std::list<std::shared_ptr<AutoRef>> audioManagerInterruptCallbackList_;
    static std::mutex sWorkerMutex_;
};
} // namespace ANI::Audio
#endif // TAIHE_AUDIO_MANAGER_INTERRUPT_CALLBACK_H