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
#ifndef TAIHE_AUDIO_LOOPBACK_CALLBACK_H
#define TAIHE_AUDIO_LOOPBACK_CALLBACK_H

#include "audio_loopback.h"
#include "event_handler.h"
#include "taihe_work.h"
#include "taihe_audio_loopback_callback_inner.h"

namespace ANI::Audio {
using namespace taihe;
using namespace ohos::multimedia::audio;
const std::string STATUS_CHANGE_CALLBACK_NAME = "statusChange";

class TaiheAudioLoopbackCallback : public OHOS::AudioStandard::AudioLoopbackCallback,
    public TaiheAudioLoopbackCallbackInner, public std::enable_shared_from_this<TaiheAudioLoopbackCallback> {
public:
    explicit TaiheAudioLoopbackCallback();
    ~TaiheAudioLoopbackCallback() override;
    void OnStatusChange(const OHOS::AudioStandard::AudioLoopbackStatus status,
        const OHOS::AudioStandard::StateChangeCmdType __attribute__((unused)) cmdType) override;
    void SaveCallbackReference(const std::string &callbackName, std::shared_ptr<uintptr_t> callback) override;
    void RemoveCallbackReference(const std::string &callbackName, std::shared_ptr<uintptr_t> callback) override;
    bool CheckIfTargetCallbackName(const std::string &callbackName) override;
protected:
    std::shared_ptr<AutoRef> GetCallback(const std::string &callbackName) override;

private:
    struct AudioLoopbackJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        OHOS::AudioStandard::AudioLoopbackStatus status;
    };

    void OnJsCallbackStatusChange(std::unique_ptr<AudioLoopbackJsCallback> &jsCb);
    static void SafeJsCallbackStatusChangeWork(AudioLoopbackJsCallback *event);

    std::mutex mutex_;
    std::shared_ptr<AutoRef> statusChangeCallback_ = nullptr;
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> mainHandler_ = nullptr;
};
} // namespace ANI::Audio
#endif // TAIHE_AUDIO_LOOPBACK_CALLBACK_H