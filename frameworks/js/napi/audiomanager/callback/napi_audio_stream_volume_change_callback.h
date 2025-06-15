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
#ifndef NAPI_AUDIO_STREAM_VOLUME_CHANGE_CALLBACK_H
#define NAPI_AUDIO_STREAM_VOLUME_CHANGE_CALLBACK_H

#include <uv.h>
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_async_work.h"
#include "audio_policy_interface.h"
#include "audio_system_manager.h"

namespace OHOS {
namespace AudioStandard {

const std::string AUDIO_STREAM_VOLUME_CHANGE_CALLBACK_NAME = "streamVolumeChange";

class NapiAudioStreamVolumeChangeCallback : public StreamVolumeChangeCallback {
public:
    explicit NapiAudioStreamVolumeChangeCallback(napi_env env);
    virtual ~NapiAudioStreamVolumeChangeCallback();
    void OnStreamVolumeChange(StreamVolumeEvent volumeEvent) override;
    void SaveCallbackReference(const std::string &callbackName, napi_value args);
    bool ContainSameJsCallback(napi_value args);
    void CreateStreamVolumeChangeTsfn(napi_env env);
    bool GetVolumeTsfnFlag();
    napi_threadsafe_function GetTsfn();

private:
    struct AudioStreamVolumeChangeJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        StreamVolumeEvent volumeEvent;
    };

    void OnJsCallbackStreamVolumeChange(std::unique_ptr<AudioStreamVolumeChangeJsCallback> &jsCb);
    static void CleanUp(void *data);
    static void SafeJsCallbackStreamVolumeChangeWork(napi_env env, napi_value js_cb, void *context, void *data);
    static void StreamVolumeChangeTsfnFinalize(napi_env env, void *data, void *hint);

    std::shared_ptr<AutoRef> audioStreamVolumeChangeCallback_ = nullptr;
    std::mutex mutex_;
    napi_env env_;
    napi_ref callback_ = nullptr;
    static napi_ref sConstructor_;
    bool regVolumeTsfn_ = false;
    napi_threadsafe_function amVolEntTsfn_ = nullptr;
};
} // namespace AudioStandard
} // namespace OHOS
#endif /* NAPI_AUDIO_INTERRUPT_MANAGER_H */