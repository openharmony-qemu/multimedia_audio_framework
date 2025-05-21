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

#ifndef NAPI_ACTIVE_VOLUME_TYPE_CHANGE_CALLBACK_H
#define NAPI_ACTIVE_VOLUME_TYPE_CHANGE_CALLBACK_H

#include <uv.h>
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_async_work.h"
#include "napi_audio_manager.h"
#include "audio_system_manager.h"

namespace OHOS {
namespace AudioStandard {
const std::string ACTIVE_VOLUME_TYPE_CHANGE_CALLBACK_NAME = "activeVolumeTypeChange";
class NapiAudioManagerActiveVolumeTypeChangeCallback : public AudioManagerActiveVolumeTypeChangeCallback {
public:
    explicit NapiAudioManagerActiveVolumeTypeChangeCallback(napi_env env);
    virtual ~NapiAudioManagerActiveVolumeTypeChangeCallback();
    void CreateManagerActiveVolumeTypeChangeTsfn(napi_env env);
    void OnActiveVolumeTypeChanged(const AudioVolumeType &event) override;
    void SaveActiveVolumeTypeChangeCallbackReference(const std::string &callbackName, napi_value args);
    bool IsSameCallback(napi_env env, napi_value callback, napi_ref refCallback);
    void RemoveSelfActiveVolumeTypeChangeCbRef(napi_env env, napi_value callback);
    void RemoveAllActiveVolumeTypeChangeCbRef();
    void RemoveCallbackReference(const napi_value args);
    int32_t GetActiveVolumeTypeChangeListSize();
    bool GetManagerActiveVolumeTypeChangeTsfnFlag();
private:
    struct AudioManagerActiveVolumeTypeChangeJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        AudioVolumeType activeVolumeTypeChangeEvent;
    };

    void OnJsCallbackActiveVolumeTypeChange(std::unique_ptr<AudioManagerActiveVolumeTypeChangeJsCallback> &jsCb);
    static void ActiveVolumeTypeChangeTsfnFinalize(napi_env env, void *data, void *hint);
    static void SafeJsCallbackActiveVolumeTypeChangeWork(napi_env env, napi_value js_cb, void *context, void *data);

    std::mutex mutex_;
    napi_env env_ = nullptr;
    std::shared_ptr<AutoRef> activeVolumeTypeChangeCallback_ = nullptr;
    std::list<std::shared_ptr<AutoRef>> activeVolumeTypeChangeList_;
    bool regAmActiveVolumeTypeChgTsfn_ = false;
    napi_threadsafe_function amActiveVolumeTypeChgTsfn_ = nullptr;
};
}
}

#endif