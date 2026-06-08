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
#ifndef NAPI_AUDIO_SYSTEM_VOLUME_CHANGE_BY_FILTER_CALLBACK_H
#define NAPI_AUDIO_SYSTEM_VOLUME_CHANGE_BY_FILTER_CALLBACK_H

#include <uv.h>
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_async_work.h"
#include "audio_policy_interface.h"
#include "audio_system_manager.h"
#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {

inline const char *AUDIO_SYSTEM_VOLUME_CHANGE_BY_FILTER_CALLBACK_NAME = "systemVolumeChangeByFilter";

class NapiAudioSystemVolumeChangeByFilterCallback : public SystemVolumeChangeByFilterCallback {
public:
    explicit NapiAudioSystemVolumeChangeByFilterCallback(napi_env env);
    virtual ~NapiAudioSystemVolumeChangeByFilterCallback();
    void OnSystemVolumeChangeByFilter(const VolumeEvent &volumeEvent) override;
    void RemoveCallbackReference(napi_env env, napi_value args);
    void RemoveAllCallbackReference();
    void SaveCallbackReference(const std::string &callbackName, napi_value args, const SystemVolumeFilter &filter);
    bool ContainSameJsCallback(napi_value args);
    bool IsSameCallback(napi_env env, napi_value callback, napi_ref refCallback);
    void CreateSystemVolumeChangeByFilterTsfn(napi_env env);
    bool GetVolumeTsfnFlag();
    int32_t GetSystemVolumeByFilterCbListSize();
    napi_threadsafe_function GetTsfn();
    SystemVolumeFilter GetFilter() const { return filter_; }
    void SetFilter(const SystemVolumeFilter &filter) { filter_ = filter; }

private:
    struct AudioSystemVolumeChangeByFilterJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        VolumeEvent volumeEvent;
    };

    void OnJsCallbackSystemVolumeChangeByFilter(std::unique_ptr<AudioSystemVolumeChangeByFilterJsCallback> &jsCb);
    static void CleanUp(void *data);
    static void SafeJsCallbackSystemVolumeChangeByFilterWork(napi_env env, napi_value js_cb, void *context, void *data);
    static void SystemVolumeChangeByFilterTsfnFinalize(napi_env env, void *data, void *hint);

    std::list<std::shared_ptr<AutoRef>> audioSystemVolumeChangeByFilterCbList_;
    std::mutex mutex_;
    napi_env env_;
    static napi_ref sConstructor_;
    bool regVolumeTsfn_ = false;
    napi_threadsafe_function systemVolumeChangeByFilterTsfn_ = nullptr;
    SystemVolumeFilter filter_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif /* NAPI_AUDIO_SYSTEM_VOLUME_CHANGE_BY_FILTER_CALLBACK_H */