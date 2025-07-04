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

#ifndef TAIHE_AUDIO_RENDERER_CALLBACK_H
#define TAIHE_AUDIO_RENDERER_CALLBACK_H

#include "event_handler.h"
#include "audio_renderer.h"
#include "taihe_audio_renderer_callback_inner.h"
#include "taihe_work.h"

namespace ANI::Audio {
using namespace taihe;
using namespace ohos::multimedia::audio;
const std::string INTERRUPT_CALLBACK_NAME = "interrupt";
const std::string AUDIO_INTERRUPT_CALLBACK_NAME = "audioInterrupt";
const std::string STATE_CHANGE_CALLBACK_NAME = "stateChange";
const std::string MARK_REACH_CALLBACK_NAME = "markReach";
const std::string PERIOD_REACH_CALLBACK_NAME = "periodReach";
const std::string DATA_REQUEST_CALLBACK_NAME = "dataRequest";
const std::string DEVICECHANGE_CALLBACK_NAME = "outputDeviceChange";
const std::string OUTPUT_DEVICECHANGE_WITH_INFO = "outputDeviceChangeWithInfo";
const std::string WRITE_DATA_CALLBACK_NAME = "writeData";

class TaiheAudioRendererCallback : public OHOS::AudioStandard::AudioRendererCallback,
    public TaiheAudioRendererCallbackInner, public std::enable_shared_from_this<TaiheAudioRendererCallback> {
public:
    explicit TaiheAudioRendererCallback();
    ~TaiheAudioRendererCallback() override;
    void SaveCallbackReference(const std::string &callbackName, std::shared_ptr<uintptr_t> callback) override;
    bool CheckIfTargetCallbackName(const std::string &callbackName) override;
    void OnInterrupt(const OHOS::AudioStandard::InterruptEvent &interruptEvent) override;
    void OnStateChange(const OHOS::AudioStandard::RendererState state,
        const OHOS::AudioStandard::StateChangeCmdType __attribute__((unused)) cmdType) override;
    void RemoveCallbackReference(const std::string &callbackName, std::shared_ptr<uintptr_t> callback) override;
protected:
    std::shared_ptr<AutoRef> GetCallback(const std::string &callbackName) override;

private:
    struct AudioRendererJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        OHOS::AudioStandard::InterruptEvent interruptEvent;
        OHOS::AudioStandard::RendererState state;
    };
    void OnJsCallbackInterrupt(std::unique_ptr<AudioRendererJsCallback> &jsCb);
    void OnJsCallbackStateChange(std::unique_ptr<AudioRendererJsCallback> &jsCb);
    static void SafeJsCallbackInterruptWork(AudioRendererJsCallback *event);
    static void SafeJsCallbackStateChangeWork(AudioRendererJsCallback *event);
    std::mutex mutex_;
    std::shared_ptr<AutoRef> interruptCallback_ = nullptr;
    std::shared_ptr<AutoRef> stateChangeCallback_ = nullptr;
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> mainHandler_ = nullptr;
};
} // namespace ANI::Audio
#endif // TAIHE_AUDIO_RENDERER_CALLBACK_H
