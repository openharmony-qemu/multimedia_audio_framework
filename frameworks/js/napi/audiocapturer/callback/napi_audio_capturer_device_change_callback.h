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
#ifndef NAPI_AUDIO_CAPTURER_DEVICE_CHANGE_CALLBACK_H
#define NAPI_AUDIO_CAPTURER_DEVICE_CHANGE_CALLBACK_H

#include <uv.h>
#include <list>
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_async_work.h"
#include "audio_capturer.h"
#include "napi_audio_capturer_callback_inner.h"
#include "napi_audio_capturer_callbacks.h"

namespace OHOS {
namespace AudioStandard {
class NapiAudioCapturerDeviceChangeCallback : public AudioCapturerDeviceChangeCallback,
    public NapiAudioCapturerCallbackInner {
public:
    explicit NapiAudioCapturerDeviceChangeCallback(napi_env env);
    ~NapiAudioCapturerDeviceChangeCallback() override;
    void SaveCallbackReference(const std::string &callbackName, napi_value args) override;
    void RemoveCallbackReference(const std::string &callbackName, napi_env env, napi_value callback) override;
    void OnStateChange(const AudioDeviceDescriptor &deviceInfo) override;
    bool ContainSameJsCallback(napi_value args);
    void CreateCaptureDeviceChangeTsfn(napi_env env);
    bool CheckIfTargetCallbackName(const std::string &callbackName) override;
protected:
    std::shared_ptr<AutoRef> GetCallback(const std::string &callbackName) override;
    napi_env &GetEnv() override;
private:
    struct AudioCapturerDeviceChangeJsCallback {
        napi_ref callback_;
        napi_env env_;
        AudioDeviceDescriptor deviceInfo_ = AudioDeviceDescriptor(AudioDeviceDescriptor::DEVICE_INFO);
    };

    void OnJsCallbackCapturerDeviceInfo(napi_ref method, const AudioDeviceDescriptor &deviceInfo);
    static void CaptureDeviceInfoTsfnFinalize(napi_env env, void *data, void *hint);
    static void SafeJsCallbackCapturerDeviceInfoWork(napi_env env, napi_value js_cb, void *context, void *data);

    std::mutex mutex_;
    napi_env env_ = nullptr;
    napi_ref callback_ = nullptr;
    bool regAcDevChgTsfn_ = false;
    napi_threadsafe_function acDevChgTsfn_ = nullptr;
    std::shared_ptr<AutoRef> callbackPtr_ = nullptr;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif /* NAPI_AUDIO_CAPTURER_DEVICE_CHANGE_CALLBACK_H */