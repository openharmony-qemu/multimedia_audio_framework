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

#ifndef NAPI_AUDIO_RECORD_CONTROLLER_H
#define NAPI_AUDIO_RECORD_CONTROLLER_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_async_work.h"
#include "audio_system_manager.h"
#include "napi_record_controller_change_callback.h"

namespace OHOS {
namespace AudioStandard {

class NapiAudioRecordController {
public:
    NapiAudioRecordController();
    ~NapiAudioRecordController();

    static napi_value Init(napi_env env, napi_value exports);
    static napi_status InitNapiAudioRecordController(napi_env env, napi_value &constructor);
    static napi_value CreateAudioRecordController(napi_env env, napi_callback_info info);

private:
    struct RecordControllerAsyncContext : public ContextBase {
        bool show = false;
        SystemRecordControllerConfig config;
    };

    static NapiAudioRecordController* GetParamWithSync(const napi_env &env, napi_callback_info info,
        size_t &argc, napi_value *args);
    static bool CheckContextStatus(std::shared_ptr<RecordControllerAsyncContext> context);
    static bool CheckAudioRecordControllerStatus(NapiAudioRecordController *napi,
        std::shared_ptr<RecordControllerAsyncContext> context);

    static void Destructor(napi_env env, void *nativeObject, void *finalizeHint);
    static napi_value Construct(napi_env env, napi_callback_info info);
    static napi_value CreateAudioRecordControllerWrapper(napi_env env);

    static napi_value EnableSystemRecordController(napi_env env, napi_callback_info info);
    static napi_value OnSystemRecordControllerEnabledChange(napi_env env, napi_callback_info info);
    static napi_value OffSystemRecordControllerEnabledChange(napi_env env, napi_callback_info info);

    static napi_value RegisterRecordControllerEnabledChangeCallback(napi_env env, napi_value *args,
        NapiAudioRecordController *napiRecordController);
    static void UnregisterRecordControllerEnabledChangeCallback(napi_env env,
        NapiAudioRecordController *napiRecordController, size_t argc, napi_value *args);

    AudioSystemManager *audioMngr_ = nullptr;
    std::shared_ptr<SystemRecordControllerEnabledChangeCallback> recordControllerChangeCallbackNapi_ = nullptr;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // NAPI_AUDIO_RECORD_CONTROLLER_H