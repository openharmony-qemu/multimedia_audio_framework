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
#ifndef NAPI_AUDIO_LOOPBACK_CALLBACK_H
#define NAPI_AUDIO_LOOPBACK_CALLBACK_H

#include <uv.h>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_async_work.h"
#include "audio_loopback.h"
#include "napi_audio_loopback_callback_inner.h"

namespace OHOS {
namespace AudioStandard {
const std::string STATUS_CHANGE_CALLBACK_NAME = "statusChange";

class NapiAudioLoopbackCallback : public AudioLoopbackCallback,
    public NapiAudioLoopbackCallbackInner {
public:
    explicit NapiAudioLoopbackCallback(napi_env env);
    ~NapiAudioLoopbackCallback() override;
    void OnStatusChange(const AudioLoopbackStatus status,
        const StateChangeCmdType __attribute__((unused)) cmdType) override;
    void CreateArStatusChange(napi_env env);
    bool GetArStatusChangeTsfnFlag();
    void SaveCallbackReference(const std::string &callbackName, napi_value args) override;
    void RemoveCallbackReference(const std::string &callbackName, napi_env env,
        napi_value callback, napi_value args = nullptr) override;
    bool CheckIfTargetCallbackName(const std::string &callbackName) override;
protected:
    std::shared_ptr<AutoRef> GetCallback(const std::string &callbackName) override;
    napi_env &GetEnv() override;

private:
    struct AudioLoopbackJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        AudioLoopbackStatus status;
    };

    void OnJsCallbackStatusChange(std::unique_ptr<AudioLoopbackJsCallback> &jsCb);
    static void SafeJsCallbackStatusChangeWork(napi_env env, napi_value js_cb, void *context, void *data);
    static void StatusChangeTsfnFinalize(napi_env env, void *data, void *hint);

    std::mutex mutex_;
    napi_env env_ = nullptr;
    std::shared_ptr<AutoRef> statusChangeCallback_ = nullptr;
    bool regArStatusChgTsfn_ = false;
    napi_threadsafe_function arStatusChgTsfn_ = nullptr;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif // NAPI_AUDIO_LOOPBACK_CALLBACK_H
