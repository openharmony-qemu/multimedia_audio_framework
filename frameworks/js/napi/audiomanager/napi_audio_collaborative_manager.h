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
#ifndef NAPI_AUDIO_COLLABORATIVE_MANAGER_H
#define NAPI_AUDIO_COLLABORATIVE_MANAGER_H
#include <iostream>
#include <map>
#include <vector>
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_async_work.h"
#include "audio_system_manager.h"
#include "audio_collaborative_manager.h"

namespace OHOS {
namespace AudioStandard {
const std::string AUDIO_COLLABORATIVE_MANAGER_NAPI_CLASS_NAME = "AudioCollaborativeManager";
class NapiAudioCollaborativeManager {
public:
    NapiAudioCollaborativeManager();
    ~NapiAudioCollaborativeManager();

    static napi_value Init(napi_env env, napi_value exports);
    static napi_value CreateCollaborativeManagerWrapper(napi_env env);

private:
    struct AudioCollaborativeManagerAsyncContext : public ContextBase {
        std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
        bool collaborativeEnable;
        int32_t intValue;
    };
    static bool CheckContextStatus(std::shared_ptr<AudioCollaborativeManagerAsyncContext> context);
    static bool CheckAudioCollaborativeManagerStatus(NapiAudioCollaborativeManager *napi,
    std::shared_ptr<AudioCollaborativeManagerAsyncContext> context);
    static NapiAudioCollaborativeManager* GetParamWithSync(const napi_env &env, napi_callback_info info,
        size_t &argc, napi_value *args);
    static void Destructor(napi_env env, void *nativeObject, void *finalizeHint);
    static napi_value Construct(napi_env env, napi_callback_info info);
    static napi_value IsCollaborativePlaybackSupported(napi_env env, napi_callback_info info);
    static napi_value IsCollaborativePlaybackEnabledForDevice(napi_env env, napi_callback_info info);
    static napi_value SetCollaborativePlaybackEnabledForDevice(napi_env env, napi_callback_info info);
    static napi_value UpdateCollaborativeEnabled(
        napi_env env, std::shared_ptr<AudioCollaborativeManagerAsyncContext> &context);

    AudioCollaborativeManager *audioCollaborativeMngr_;
    napi_env env_;
};
} // AudioStandard
} // OHOS
#endif