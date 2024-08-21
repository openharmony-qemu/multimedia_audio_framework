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
#ifndef NAPI_AUDIO_SESSION_CALLBACK_H
#define NAPI_AUDIO_SESSION_CALLBACK_H

#include "audio_session_manager.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_async_work.h"

namespace OHOS {
namespace AudioStandard {

const std::string AUDIO_SESSION_CALLBACK_NAME = "audioSessionDeactivated";

class NapiAudioSessionCallback : public AudioSessionCallback {
public:
    explicit NapiAudioSessionCallback(napi_env env);
    virtual ~NapiAudioSessionCallback();
    
    void OnAudioSessionDeactive(const AudioSessionDeactiveEvent &deactiveEvent);
    void SaveCallbackReference(napi_value args);
    
private:
    struct AudioSessionJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        AudioSessionDeactiveEvent audioSessionDeactiveEvent;
    };

    void OnJsCallbackAudioSession(std::unique_ptr<AudioSessionJsCallback> &jsCb);
    static void WorkCallbackAudioSessionChangeDone(uv_work_t *work, int status);

    std::shared_ptr<AutoRef> audioSessionJsCallback_ = nullptr;
    std::mutex mutex_;
    napi_env env_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif /* NAPI_AUDIO_INTERRUPT_MANAGER_H */