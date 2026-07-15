/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#ifndef TAIHE_AUDIO_RECORD_CONTROLLER_CALLBACK_H
#define TAIHE_AUDIO_RECORD_CONTROLLER_CALLBACK_H

#include "audio_policy_interface.h"
#include "event_handler.h"
#include "taihe_work.h"
#include "taihe_audio_record_controller_callback_inner.h"

namespace ANI::Audio {
using namespace taihe;
using namespace ohos::multimedia::audio;
inline const std::string RECORD_CONTROLLER_ENABLED_CHANGE_CALLBACK_NAME = "recordControllerEnabledChange";

class TaiheAudioRecordControllerCallback
    : public OHOS::AudioStandard::SystemRecordControllerEnabledChangeCallback,
    public TaiheAudioRecordControllerCallbackInner,
    public std::enable_shared_from_this<TaiheAudioRecordControllerCallback> {
public:
    explicit TaiheAudioRecordControllerCallback();
    ~TaiheAudioRecordControllerCallback() override;
    void OnRecordControllerEnabledChange(
        const OHOS::AudioStandard::SystemRecordControllerChangeInfo &changeInfo) override;
    void SaveCallbackReference(const std::string &callbackName, std::shared_ptr<uintptr_t> callback) override;
    void RemoveCallbackReference(const std::string &callbackName, std::shared_ptr<uintptr_t> callback) override;
    bool CheckIfTargetCallbackName(const std::string &callbackName) override;
protected:
    std::shared_ptr<AutoRef> GetCallback(const std::string &callbackName) override;

private:
    struct RecordControllerChangeJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        OHOS::AudioStandard::SystemRecordControllerChangeInfo changeInfo;
    };

    void OnJsCallbackRecordControllerChange(std::unique_ptr<RecordControllerChangeJsCallback> &jsCb);
    static void SafeJsCallbackRecordControllerChangeWork(RecordControllerChangeJsCallback *event);

    std::mutex mutex_;
    std::shared_ptr<AutoRef> enabledChangeCallback_ = nullptr;
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> mainHandler_ = nullptr;
};
} // namespace ANI::Audio
#endif // TAIHE_AUDIO_RECORD_CONTROLLER_CALLBACK_H
