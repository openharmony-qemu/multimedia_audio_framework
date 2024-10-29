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

#ifndef NAPI_AUDIO_RENDERER_WRITE_DATA_CALLBACK_H
#define NAPI_AUDIO_RENDERER_WRITE_DATA_CALLBACK_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_audio_renderer.h"
#include "napi_audio_renderer_callback.h"

namespace OHOS {
namespace AudioStandard {
class NapiRendererWriteDataCallback : public AudioRendererWriteCallback {
public:
    NapiRendererWriteDataCallback(napi_env env, NapiAudioRenderer *napiRenderer);
    virtual ~NapiRendererWriteDataCallback();
    void OnWriteData(size_t length) override;

    void AddCallbackReference(const std::string &callbackName, napi_value args);
    void RemoveCallbackReference(napi_env env, napi_value callback);

private:
    struct RendererWriteDataJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        BufferDesc bufDesc {};
        NapiAudioRenderer *rendererNapiObj;
    };

    static void WorkCallbackRendererWriteDataInner(RendererWriteDataJsCallback *event);
    void OnJsRendererWriteDataCallback(std::unique_ptr<RendererWriteDataJsCallback> &jsCb);
    static void CheckWriteDataCallbackResult(napi_env env, BufferDesc &bufDesc, napi_value result);

    std::mutex mutex_;
    napi_env env_ = nullptr;
    std::shared_ptr<AutoRef> rendererWriteDataCallback_ = nullptr;
    NapiAudioRenderer *napiRenderer_;
    napi_ref callback_ = nullptr;

#if defined(ANDROID_PLATFORM) || defined(IOS_PLATFORM)
    static vector<NapiAudioRenderer*> activeRenderers_;
#endif
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif //NAPI_AUDIO_RENDERER_WRITE_DATA_CALLBACK_H
