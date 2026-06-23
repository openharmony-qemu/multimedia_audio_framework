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

#ifndef NAPI_RECORD_CONTROLLER_CHANGE_CALLBACK_H
#define NAPI_RECORD_CONTROLLER_CHANGE_CALLBACK_H

#include <uv.h>
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_async_work.h"
#include "audio_policy_interface.h"

namespace OHOS {
namespace AudioStandard {
inline const std::string RECORD_CONTROLLER_ENABLED_CHANGE_CALLBACK_NAME = "recordControllerEnabledChange";

class NapiRecordControllerChangeCallback : public SystemRecordControllerEnabledChangeCallback {
public:
    explicit NapiRecordControllerChangeCallback(napi_env env);
    virtual ~NapiRecordControllerChangeCallback();
    void SaveCallbackReference(const std::string &callbackName, napi_value args);
    bool IsSameCallback(const napi_value args);
    void RemoveCallbackReference(const napi_value args);
    void OnRecordControllerEnabledChange(const SystemRecordControllerChangeInfo &changeInfo) override;
    void CreateRecordControllerChangeTsfn(napi_env env);
    bool GetRecordControllerChangeTsfnFlag();

private:
    struct RecordControllerChangeJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        SystemRecordControllerChangeInfo changeInfo;
    };

    void OnJsCallbackRecordControllerChange(std::unique_ptr<RecordControllerChangeJsCallback> &jsCb);
    static void RecordControllerChangeTsfnFinalize(napi_env env, void *data, void *hint);
    static void SafeJsCallbackRecordControllerChangeWork(
        napi_env env, napi_value js_cb, void *context, void *data);

    std::mutex mutex_;
    napi_env env_ = nullptr;
    std::shared_ptr<AutoRef> recordControllerChangeCallback_ = nullptr;
    bool regRecordControllerChgTsfn_ = false;
    napi_threadsafe_function recordControllerChgTsfn_ = nullptr;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // NAPI_RECORD_CONTROLLER_CHANGE_CALLBACK_H