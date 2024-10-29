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

#ifndef NAPI_RENDERER_POSITION_CALLBACK_H
#define NAPI_RENDERER_POSITION_CALLBACK_H

#include <uv.h>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_async_work.h"
#include "audio_renderer.h"

namespace OHOS {
namespace AudioStandard {
class NapiRendererPositionCallback : public RendererPositionCallback {
public:
    explicit NapiRendererPositionCallback(napi_env env);
    virtual ~NapiRendererPositionCallback();
    void SaveCallbackReference(const std::string &callbackName, napi_value args);
    void OnMarkReached(const int64_t &framePosition) override;

private:
    struct RendererPositionJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        int64_t position = 0;
    };

    void OnJsRendererPositionCallback(std::unique_ptr<RendererPositionJsCallback> &jsCb);

    std::mutex mutex_;
    napi_env env_ = nullptr;
    std::shared_ptr<AutoRef> renderPositionCallback_ = nullptr;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif // NAPI_RENDERER_POSITION_CALLBACK_H
