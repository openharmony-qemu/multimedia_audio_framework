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

namespace OHOS {
namespace AudioStandard {
class NapiAudioCapturerDeviceChangeCallback : public AudioCapturerDeviceChangeCallback {
public:
    explicit NapiAudioCapturerDeviceChangeCallback(napi_env env);
    virtual ~NapiAudioCapturerDeviceChangeCallback();
    void SaveCallbackReference(napi_value args);
    void OnStateChange(const DeviceInfo &deviceInfo) override;
    bool ContainSameJsCallback(napi_value args);

private:
    struct AudioCapturerDeviceChangeJsCallback {
        napi_ref callback_;
        napi_env env_;
        DeviceInfo deviceInfo_;
    };

    static void WorkCallbackCompleted(uv_work_t* work, int status);
    void OnJsCallbackCapturerDeviceInfo(napi_ref method, const DeviceInfo &deviceInfo);

    std::mutex mutex_;
    napi_env env_ = nullptr;
    napi_ref callback_ = nullptr;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif /* NAPI_AUDIO_CAPTURER_DEVICE_CHANGE_CALLBACK_H */