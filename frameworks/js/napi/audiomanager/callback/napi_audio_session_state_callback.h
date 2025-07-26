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
#ifndef NAPI_AUDIO_SESSION_STATE_CALLBACK_H
#define NAPI_AUDIO_SESSION_STATE_CALLBACK_H

#include "audio_session_manager.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_async_work.h"

namespace OHOS {
namespace AudioStandard {

const std::string AUDIO_SESSION_STATE_CALLBACK_NAME = "audioSessionStateChanged";

class NapiAudioSessionStateCallback : public AudioSessionStateChangedCallback {
public:
    explicit NapiAudioSessionStateCallback(napi_env env);
    virtual ~NapiAudioSessionStateCallback();
    
    void OnAudioSessionStateChanged(const AudioSessionStateChangedEvent &stateEvent);
    void SaveCallbackReference(napi_value args);
    void CreateAudioSessionStateTsfn(napi_env env);
    bool GetAudioSessionStateTsfnFlag() const;
    bool ContainSameJsCallback(napi_value args);
    
private:
    struct AudioSessionStateJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        AudioSessionStateChangedEvent audioSessionStateEvent;
    };

    void OnJsCallbackAudioSessionState(std::unique_ptr<AudioSessionStateJsCallback> &jsCb);
    static void AudioSessionStateTsfnFinalize(napi_env env, void *data, void *hint);
    static void SafeJsCallbackAudioSessionStateWork(napi_env env, napi_value js_cb, void *context, void *data);

    std::shared_ptr<AutoRef> audioSessionStateJsCallback_ = nullptr;
    napi_ref callback_ = nullptr;
    std::mutex mutex_;
    napi_env env_;
    bool regAmSessionStateChgTsfn_ = false;
    napi_threadsafe_function amSessionStateChgTsfn_ = nullptr;
};
} // namespace AudioStandard
} // namespace OHOS
#endif /* NAPI_AUDIO_SESSION_STATE_CALLBACK_H */