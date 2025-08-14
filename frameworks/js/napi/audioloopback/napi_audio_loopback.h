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

#ifndef NAPI_AUDIO_LOOPBACK_H
#define NAPI_AUDIO_LOOPBACK_H
#include <iostream>
#include <map>
#include <queue>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "securec.h"
#include "audio_errors.h"
#include "napi_async_work.h"
#include "audio_loopback.h"

namespace OHOS {
namespace AudioStandard {
const std::string NAPI_AUDIO_LOOPBACK_CLASS_NAME = "AudioLoopback";

class NapiAudioLoopback {
public:
    NapiAudioLoopback();
    ~NapiAudioLoopback();
    static napi_value Init(napi_env env, napi_value exports);
    void DestroyCallbacks();
    void DestroyNAPICallbacks();
    std::shared_ptr<AudioLoopback> loopback_;

private:
    struct AudioLoopbackAsyncContext : public ContextBase {
        bool isTrue;
        int32_t intValue;
        int32_t loopbackMode;
        double volLevel;
        bool enable;
        AudioLoopbackStatus loopbackStatus;
    };

    static void Destructor(napi_env env, void *nativeObject, void *finalizeHint);
    static napi_value Construct(napi_env env, napi_callback_info info);
    static napi_value CreateAudioLoopbackWrapper(napi_env env, AudioLoopbackMode audioLoopbackMode);
    static napi_value CreateAudioLoopback(napi_env env, napi_callback_info info);
    static napi_value GetStatus(napi_env env, napi_callback_info info);
    static napi_value SetVolume(napi_env env, napi_callback_info info);
    static napi_value Enable(napi_env env, napi_callback_info info);
    static napi_value On(napi_env env, napi_callback_info info);
    static napi_value Off(napi_env env, napi_callback_info info);
    static napi_value GetCallback(size_t argc, napi_value *argv);

    static napi_value RegisterCallback(napi_env env, napi_value jsThis,
        napi_value *argv, const std::string &cbName);
    static napi_value RegisterLoopbackCallback(napi_env env, napi_value *argv,
        const std::string &cbName, NapiAudioLoopback *napiLoopback);
    static napi_value UnregisterCallback(napi_env env, napi_value jsThis, size_t argc, napi_value *argv,
        const std::string &cbName);
    static void UnregisterLoopbackCallback(napi_env env, size_t argc, const std::string &cbName,
        napi_value *argv, NapiAudioLoopback *napiLoopback);

    static bool CheckContextStatus(std::shared_ptr<AudioLoopbackAsyncContext> context);
    static bool CheckAudioLoopbackStatus(NapiAudioLoopback *napi, std::shared_ptr<AudioLoopbackAsyncContext> context);
    static NapiAudioLoopback* GetParamWithSync(const napi_env &env, napi_callback_info info,
        size_t &argc, napi_value *args);
    static napi_value SetReverbPreset(napi_env env, napi_callback_info info);
    static napi_value GetReverbPreset(napi_env env, napi_callback_info info);
    static napi_value SetEqualizerPreset(napi_env env, napi_callback_info info);
    static napi_value GetEqualizerPreset(napi_env env, napi_callback_info info);

    static std::mutex createMutex_;
    static int32_t isConstructSuccess_;
    static AudioLoopbackMode sLoopbackMode_;

    napi_env env_;
    std::shared_ptr<AudioLoopbackCallback> callbackNapi_ = nullptr;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // NAPI_AUDIO_LOOPBACK_H