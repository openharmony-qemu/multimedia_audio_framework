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
#ifndef NAPI_AUDIO_RINGMODE_CALLBACK_H
#define NAPI_AUDIO_RINGMODE_CALLBACK_H

#include <uv.h>
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_async_work.h"
#include "napi_audio_manager.h"
#include "audio_system_manager.h"

namespace OHOS {
namespace AudioStandard {
const std::string RINGERMODE_CALLBACK_NAME = "ringerModeChange";

class NapiAudioRingerModeCallback : public AudioRingerModeCallback {
public:
    explicit NapiAudioRingerModeCallback(napi_env env);
    virtual ~NapiAudioRingerModeCallback();
    void SaveCallbackReference(const std::string &callbackName, napi_value args);
    bool IsSameCallback(const napi_value args);
    void RemoveCallbackReference(const napi_value args);
    void OnRingerModeUpdated(const AudioRingerMode &ringerMode) override;

private:
    struct AudioRingerModeJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        AudioRingerMode ringerMode = RINGER_MODE_NORMAL;
    };

    void OnJsCallbackRingerMode(std::unique_ptr<AudioRingerModeJsCallback> &jsCb);
    static void WorkCallbackInterruptDone(uv_work_t *work, int status);

    std::mutex mutex_;
    napi_env env_ = nullptr;
    std::shared_ptr<AutoRef> ringerModeCallback_ = nullptr;
};
} // namespace AudioStandard
} // namespace OHOS
#endif /* NAPI_AUDIO_RINGMODE_CALLBACK_H */