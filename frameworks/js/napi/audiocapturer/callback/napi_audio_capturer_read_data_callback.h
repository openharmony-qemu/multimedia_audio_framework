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

#ifndef NAPI_AUDIO_CAPTURER_READ_DATA_CALLBACK_H
#define NAPI_AUDIO_CAPTURER_READ_DATA_CALLBACK_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_audio_capturer.h"
#include "napi_audio_capturer_callbacks.h"

namespace OHOS {
namespace AudioStandard {
class NapiCapturerReadDataCallback : public AudioCapturerReadCallback {
public:
    NapiCapturerReadDataCallback(napi_env env, NapiAudioCapturer *napiCapturer);
    virtual ~NapiCapturerReadDataCallback();
    void OnReadData(size_t length) override;

    void AddCallbackReference(const std::string &callbackName, napi_value args);
    void RemoveCallbackReference(napi_env env, napi_value callback);

private:
    struct CapturerReadDataJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        BufferDesc bufDesc {};
        NapiAudioCapturer *capturerNapiObj;
        NapiCapturerReadDataCallback *readDataCallbackPtr;
    };

    static void WorkCallbackCapturerReadData(uv_work_t *work, int status);
    static void WorkCallbackCapturerReadDataInner(uv_work_t *work, int status);
    void OnJsCapturerReadDataCallback(std::unique_ptr<CapturerReadDataJsCallback> &jsCb);

    std::mutex mutex_;
    napi_env env_ = nullptr;
    std::shared_ptr<AutoRef> capturerReadDataCallback_ = nullptr;
    NapiAudioCapturer *napiCapturer_;
    napi_ref callback_ = nullptr;
    bool isCallbackInited_ = false;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif // NAPI_AUDIO_CAPTURER_READ_DATA_CALLBACK_H