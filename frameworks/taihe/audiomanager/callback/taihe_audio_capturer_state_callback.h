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

#ifndef TAIHE_AUDIO_CAPTURER_STATE_CALLBACK_H
#define TAIHE_AUDIO_CAPTURER_STATE_CALLBACK_H

#include "event_handler.h"
#include "taihe_work.h"
#include "audio_policy_interface.h"

namespace ANI::Audio {
using namespace taihe;
using namespace ohos::multimedia::audio;

class TaiheAudioCapturerStateCallback : public OHOS::AudioStandard::AudioCapturerStateChangeCallback,
    public std::enable_shared_from_this<TaiheAudioCapturerStateCallback> {
public:
    explicit TaiheAudioCapturerStateCallback();
    virtual ~TaiheAudioCapturerStateCallback();
    void SaveCallbackReference(const std::string &callbackName, std::shared_ptr<uintptr_t> callback);
    bool IsSameCallback(std::shared_ptr<uintptr_t> callback);
    void RemoveCallbackReference(std::shared_ptr<uintptr_t> callback);
    void OnCapturerStateChange(const std::vector<std::shared_ptr<OHOS::AudioStandard::AudioCapturerChangeInfo>>
        &audioCapturerChangeInfos) override;

private:
    struct AudioCapturerStateJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::vector<std::shared_ptr<OHOS::AudioStandard::AudioCapturerChangeInfo>> changeInfos;
    };

    void OnJsCallbackCapturerState(std::unique_ptr<AudioCapturerStateJsCallback> &jsCb);
    static void SafeJsCallbackCapturerStateWork(AudioCapturerStateJsCallback *event);

    std::mutex mutex_;
    std::shared_ptr<AutoRef> capturerStateCallback_ = nullptr;
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> mainHandler_ = nullptr;
};
} // namespace ANI::Audio
#endif //TAIHE_AUDIO_CAPTURER_STATE_CALLBACK_H
