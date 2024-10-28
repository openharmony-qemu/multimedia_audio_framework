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
#ifndef NAPI_AUDIO_CAPTURER_INFO_CHANGE_CALLBACK_H
#define NAPI_AUDIO_CAPTURER_INFO_CHANGE_CALLBACK_H

#include <uv.h>
#include <list>
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_async_work.h"
#include "audio_capturer.h"

namespace OHOS {
namespace AudioStandard {
class NapiAudioCapturerInfoChangeCallback : public AudioCapturerInfoChangeCallback {
public:
    explicit NapiAudioCapturerInfoChangeCallback(napi_env env);
    virtual ~NapiAudioCapturerInfoChangeCallback();
    void SaveCallbackReference(napi_value args);
    void OnStateChange(const AudioCapturerChangeInfo &capturerChangeInfo) override;
    bool ContainSameJsCallback(napi_value args);

private:
    struct AudioCapturerChangeInfoJsCallback {
        napi_ref callback_;
        napi_env env_;
        AudioCapturerChangeInfo capturerChangeInfo_;
    };

    void OnJsCallbackCapturerChangeInfo(napi_ref method, const AudioCapturerChangeInfo &capturerChangeInfo);

    std::mutex mutex_;
    napi_env env_ = nullptr;
    napi_ref callback_ = nullptr;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif /* NAPI_AUDIO_CAPTURER_INFO_CHANGE_CALLBACK_H */