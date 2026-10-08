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
#ifndef NAPI_AUDIO_VOLUME_LIMIT_EXCEEDED_CALLBACK_H
#define NAPI_AUDIO_VOLUME_LIMIT_EXCEEDED_CALLBACK_H

#include <cstdint>
#include <list>
#include <memory>
#include <mutex>
#include <string>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_async_work.h"
#include "audio_policy_interface.h"

namespace OHOS {
namespace AudioStandard {

inline const std::string VOLUME_LIMIT_EXCEEDED_CALLBACK_NAME = "volumeLimitExceeded";

class NapiAudioVolumeLimitExceededCallback : public VolumeLimitExceededCallback {
public:
    explicit NapiAudioVolumeLimitExceededCallback(napi_env env);
    ~NapiAudioVolumeLimitExceededCallback() override;

    void OnVolumeLimitExceeded(const VolumeLimitExceedEvent &event) override;
    napi_status SaveCallbackReference(const std::string &callbackName, napi_value callback);
    napi_status RemoveCallbackReference(napi_value callback);
    void RemoveAllCallbackReference();
    int32_t GetVolumeLimitExceededCbListSize();
    napi_status CreateVolumeLimitExceededTsfn();
    bool GetVolumeLimitExceededTsfnFlag() const;

private:
    struct AudioVolumeLimitExceededJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        VolumeLimitExceedEvent volumeLimitExceedEvent;
    };

    static bool IsSameCallback(napi_env env, napi_value callback, napi_ref callbackRef);
    static void SafeJsCallbackVolumeLimitExceededWork(
        napi_env env, napi_value jsCallback, void *context, void *data);
    void OnJsCallbackVolumeLimitExceeded(std::unique_ptr<AudioVolumeLimitExceededJsCallback> &jsCb);

    std::list<std::shared_ptr<AutoRef>> audioVolumeLimitExceededCbList_;
    std::mutex mutex_;
    napi_env env_ = nullptr;
    bool regVolumeLimitExceededTsfn_ = false;
    napi_threadsafe_function amVolLimitExceededTsfn_ = nullptr;
};

} // namespace AudioStandard
} // namespace OHOS
#endif // NAPI_AUDIO_VOLUME_LIMIT_EXCEEDED_CALLBACK_H
