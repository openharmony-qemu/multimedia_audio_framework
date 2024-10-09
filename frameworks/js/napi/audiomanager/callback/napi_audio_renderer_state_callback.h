/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef NAPI_AUDIO_RENDERER_STATE_CALLBACK_H
#define NAPI_AUDIO_RENDERER_STATE_CALLBACK_H

#include <uv.h>
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_async_work.h"
#include "audio_system_manager.h"
#include "audio_stream_manager.h"

namespace OHOS {
namespace AudioStandard {
class NapiAudioRendererStateCallback : public AudioRendererStateChangeCallback {
public:
    explicit NapiAudioRendererStateCallback(napi_env env);
    virtual ~NapiAudioRendererStateCallback();
    void SaveCallbackReference(napi_value args);
    void RemoveCallbackReference();
    void OnRendererStateChange(
        const std::vector<std::unique_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos) override;

private:
    struct AudioRendererStateJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::vector<std::unique_ptr<AudioRendererChangeInfo>> changeInfos;
    };

    static void WorkCallbackInterruptDone(uv_work_t *work, int status);
    void OnJsCallbackRendererState(std::unique_ptr<AudioRendererStateJsCallback> &jsCb);

    std::mutex mutex_;
    napi_env env_ = nullptr;
    std::shared_ptr<AutoRef> rendererStateCallback_ = nullptr;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif /* NAPI_AUDIO_RENDERER_STATE_CALLBACK_H */