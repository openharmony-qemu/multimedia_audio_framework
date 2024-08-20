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
#ifndef NAPI_AUDIO_SPATIALIZATION_MANAGER_H
#define NAPI_AUDIO_SPATIALIZATION_MANAGER_H

#include <iostream>
#include <map>
#include <vector>
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_async_work.h"
#include "audio_system_manager.h"
#include "audio_spatialization_manager.h"

namespace OHOS {
namespace AudioStandard {
const std::string AUDIO_SPATIALIZATION_MANAGER_NAPI_CLASS_NAME = "AudioSpatializationManager";
class NapiAudioSpatializationManager {
public:
    NapiAudioSpatializationManager();
    ~NapiAudioSpatializationManager();

    static napi_value Init(napi_env env, napi_value exports);
    static napi_value CreateSpatializationManagerWrapper(napi_env env);

private:
struct AudioSpatializationManagerAsyncContext : public ContextBase {
    sptr<AudioDeviceDescriptor> deviceDescriptor = new(std::nothrow) AudioDeviceDescriptor();
    bool spatializationEnable;
    bool headTrackingEnable;
    int32_t intValue;
    AudioSpatialDeviceState spatialDeviceState;
};
    static bool CheckContextStatus(std::shared_ptr<AudioSpatializationManagerAsyncContext> context);
    static bool CheckAudioSpatializationManagerStatus(NapiAudioSpatializationManager *napi,
    std::shared_ptr<AudioSpatializationManagerAsyncContext> context);
    static NapiAudioSpatializationManager* GetParamWithSync(const napi_env &env, napi_callback_info info,
        size_t &argc, napi_value *args);
    static void Destructor(napi_env env, void *nativeObject, void *finalizeHint);
    static napi_value Construct(napi_env env, napi_callback_info info);
    static napi_value IsSpatializationEnabled(napi_env env, napi_callback_info info);
    static napi_value SetSpatializationEnabled(napi_env env, napi_callback_info info);
    static napi_value IsHeadTrackingEnabled(napi_env env, napi_callback_info info);
    static napi_value SetHeadTrackingEnabled(napi_env env, napi_callback_info info);
    static napi_value IsSpatializationSupported(napi_env env, napi_callback_info info);
    static napi_value IsSpatializationSupportedForDevice(napi_env env, napi_callback_info info);
    static napi_value IsHeadTrackingSupported(napi_env env, napi_callback_info info);
    static napi_value IsHeadTrackingSupportedForDevice(napi_env env, napi_callback_info info);
    static napi_value UpdateSpatialDeviceState(napi_env env, napi_callback_info info);
    static napi_value GetSpatializationSceneType(napi_env env, napi_callback_info info);
    static napi_value SetSpatializationSceneType(napi_env env, napi_callback_info info);
    static napi_value On(napi_env env, napi_callback_info info);
    static napi_value Off(napi_env env, napi_callback_info info);
    static void RegisterCallback(napi_env env, napi_value jsThis,
        napi_value *args, const std::string &cbName);
    static void RegisterSpatializationEnabledChangeCallback(napi_env env, napi_value *args,
        const std::string &cbName, NapiAudioSpatializationManager *napiAudioSpatializationManager);
    static void RegisterHeadTrackingEnabledChangeCallback(napi_env env, napi_value *args,
        const std::string &cbName, NapiAudioSpatializationManager *napiAudioSpatializationManager);
    static void UnRegisterCallback(napi_env env, napi_value jsThis,
        napi_value *args, const std::string &cbName);
    static void UnregisterSpatializationEnabledChangeCallback(napi_env env, napi_value callback,
        const std::string &cbName, NapiAudioSpatializationManager *napiAudioSpatializationManager);
    static void UnregisterHeadTrackingEnabledChangeCallback(napi_env env, napi_value callback,
        const std::string &cbName, NapiAudioSpatializationManager *napiAudioSpatializationManager);
    static napi_value updateSpatializationEnabled(napi_env env, const std::size_t argc,
    std::shared_ptr<AudioSpatializationManagerAsyncContext> &context);
    static napi_value updateHeadTrackingEnabled(napi_env env, const std::size_t argc,
    std::shared_ptr<AudioSpatializationManagerAsyncContext> &context);

    AudioSpatializationManager *audioSpatializationMngr_;
    std::shared_ptr<AudioSpatializationEnabledChangeCallback> spatializationEnabledChangeCallbackNapi_ = nullptr;
    std::shared_ptr<AudioHeadTrackingEnabledChangeCallback> headTrackingEnabledChangeCallbackNapi_ = nullptr;

    napi_env env_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif /* NAPI_AUDIO_SPATIALIZATION_MANAGER_H */
