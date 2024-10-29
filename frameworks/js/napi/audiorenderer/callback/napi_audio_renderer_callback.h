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
#ifndef NAPI_AUDIO_RENDERER_CALLBACK_H
#define NAPI_AUDIO_RENDERER_CALLBACK_H

#include <uv.h>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_async_work.h"
#include "audio_renderer.h"

namespace OHOS {
namespace AudioStandard {
const std::string INTERRUPT_CALLBACK_NAME = "interrupt";
const std::string AUDIO_INTERRUPT_CALLBACK_NAME = "audioInterrupt";
const std::string STATE_CHANGE_CALLBACK_NAME = "stateChange";
const std::string MARK_REACH_CALLBACK_NAME = "markReach";
const std::string PERIOD_REACH_CALLBACK_NAME = "periodReach";
const std::string DATA_REQUEST_CALLBACK_NAME = "dataRequest";
const std::string DEVICECHANGE_CALLBACK_NAME = "outputDeviceChange";
const std::string OUTPUT_DEVICECHANGE_WITH_INFO = "outputDeviceChangeWithInfo";
const std::string WRITE_DATA_CALLBACK_NAME = "writeData";

class NapiAudioRendererCallback : public AudioRendererCallback {
public:
    explicit NapiAudioRendererCallback(napi_env env);
    virtual ~NapiAudioRendererCallback();
    void OnInterrupt(const InterruptEvent &interruptEvent) override;
    void OnStateChange(const RendererState state, const StateChangeCmdType __attribute__((unused)) cmdType) override;
    void SaveCallbackReference(const std::string &callbackName, napi_value args);
    void RemoveCallbackReference(const std::string &callbackName);

private:
    struct AudioRendererJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        InterruptEvent interruptEvent;
        RendererState state;
    };

    void OnJsCallbackInterrupt(std::unique_ptr<AudioRendererJsCallback> &jsCb);
    void OnJsCallbackStateChange(std::unique_ptr<AudioRendererJsCallback> &jsCb);

    std::mutex mutex_;
    napi_env env_ = nullptr;
    std::shared_ptr<AutoRef> interruptCallback_ = nullptr;
    std::shared_ptr<AutoRef> stateChangeCallback_ = nullptr;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif /* NAPI_AUDIO_RENDERER_CALLBACK_H */
