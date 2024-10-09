/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifndef NAPI_ASR_PROCESSING_CONTROLLER_H
#define NAPI_ASR_PROCESSING_CONTROLLER_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "audio_system_manager.h"

namespace OHOS {
namespace AudioStandard {
const std::string NAPI_ASR_PROCESSING_CONTROLLER_CLASS_NAME = "AsrProcessingController";

class NapiAsrProcessingController {
public:
    NapiAsrProcessingController();
    ~NapiAsrProcessingController();

    static napi_value Init(napi_env env, napi_value exports);
    static napi_status InitNapiAsrProcessingController(napi_env env, napi_value& constructor);

private:
    static NapiAsrProcessingController* GetParamWithSync(const napi_env &env, napi_callback_info info,
        size_t& argc, napi_value* args);
    static void Destructor(napi_env env, void* nativeObject, void* finalizeHint);
    static napi_value Construct(napi_env env, napi_callback_info info);
    static napi_value CreateAsrProcessingController(napi_env env, napi_callback_info info);
    static napi_value CreateAsrProcessingControllerWrapper(napi_env env);
    static napi_value SetAsrAecMode(napi_env env, napi_callback_info info);
    static napi_value GetAsrAecMode(napi_env env, napi_callback_info info);
    static napi_value SetAsrNoiseSuppressionMode(napi_env env, napi_callback_info info);
    static napi_value GetAsrNoiseSuppressionMode(napi_env env, napi_callback_info info);
    static napi_value SetAsrWhisperDetectionMode(napi_env env, napi_callback_info info);
    static napi_value GetAsrWhisperDetectionMode(napi_env env, napi_callback_info info);
    static napi_value SetAsrVoiceControlMode(napi_env env, napi_callback_info info);
    static napi_value SetAsrVoiceMuteMode(napi_env env, napi_callback_info info);
    static napi_value IsWhispering(napi_env env, napi_callback_info info);

    AudioSystemManager* audioMngr_;
    napi_env env_;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif