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
#ifndef NAPI_AUDIO_CAPTURER_CALLBACK_H
#define NAPI_AUDIO_CAPTURER_CALLBACK_H

#include <uv.h>
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_async_work.h"
#include "audio_capturer.h"

namespace OHOS {
namespace AudioStandard {
const std::string INTERRUPT_CALLBACK_NAME = "interrupt";
const std::string AUDIO_INTERRUPT_CALLBACK_NAME = "audioInterrupt";
const std::string STATE_CHANGE_CALLBACK_NAME = "stateChange";
const std::string MARK_REACH_CALLBACK_NAME = "markReach";
const std::string PERIOD_REACH_CALLBACK_NAME = "periodReach";
const std::string INPUTDEVICE_CHANGE_CALLBACK_NAME = "inputDeviceChange";
const std::string AUDIO_CAPTURER_CHANGE_CALLBACK_NAME = "audioCapturerChange";
const std::string READ_DATA_CALLBACK_NAME = "readData";

class NapiAudioCapturerCallback : public AudioCapturerCallback {
public:
    explicit NapiAudioCapturerCallback(napi_env env);
    virtual ~NapiAudioCapturerCallback();
    void SaveCallbackReference(const std::string &callbackName, napi_value args);
    void RemoveCallbackReference(const std::string &callbackName);
    void OnInterrupt(const InterruptEvent &interruptEvent) override;
    void OnStateChange(const CapturerState state) override;

private:
    struct AudioCapturerJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        InterruptEvent interruptEvent;
        CapturerState state;
    };

    void OnJsCallbackInterrupt(std::unique_ptr<AudioCapturerJsCallback> &jsCb);
    void OnJsCallbackStateChange(std::unique_ptr<AudioCapturerJsCallback> &jsCb);

    std::mutex mutex_;
    napi_env env_ = nullptr;
    std::shared_ptr<AutoRef> interruptCallback_ = nullptr;
    std::shared_ptr<AutoRef> stateChangeCallback_ = nullptr;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif /* NAPI_AUDIO_CAPTURER_CALLBACK_H */
