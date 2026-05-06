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
#ifndef NAPI_AUDIO_DEBUGGING_MANAGER_H
#define NAPI_AUDIO_DEBUGGING_MANAGER_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_async_work.h"
#include "audio_debug_manager.h"

namespace OHOS {
namespace AudioStandard {
inline const std::string AUDIO_DEBUGGING_MGR_NAPI_CLASS_NAME = "AudioDebuggingManager";

class NapiAudioDebuggingManager {
public:
    NapiAudioDebuggingManager();
    ~NapiAudioDebuggingManager();

    static napi_value Init(napi_env env, napi_value exports);
    static napi_value CreateDebuggingManagerWrapper(napi_env env);

private:
    static void Destructor(napi_env env, void *nativeObject, void *finalizeHint);
    static napi_value Construct(napi_env env, napi_callback_info info);
    static NapiAudioDebuggingManager *GetParamWithSync(const napi_env &env, napi_callback_info info,
        size_t &argc, napi_value *args);

    static napi_value PrintAppInfo(napi_env env, napi_callback_info info);
    static napi_value PrintRendererInfo(napi_env env, napi_callback_info info);
    static napi_value PrintCapturerInfo(napi_env env, napi_callback_info info);
    static napi_value PrintLoopbackInfo(napi_env env, napi_callback_info info);
    static napi_value PrintSessionInfo(napi_env env, napi_callback_info info);

    napi_env env_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // NAPI_AUDIO_DEBUGGING_MANAGER_H
