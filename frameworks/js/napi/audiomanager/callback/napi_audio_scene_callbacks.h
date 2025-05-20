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
#ifndef NAPI_AUDIO_SCENE_CALLBACK_H
#define NAPI_AUDIO_SCENE_CALLBACK_H

#include <uv.h>
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_async_work.h"
#include "napi_audio_manager.h"
#include "audio_system_manager.h"

namespace OHOS {
namespace AudioStandard {
const std::string AUDIO_SCENE_CHANGE_CALLBACK_NAME = "audioSceneChange";

class NapiAudioSceneChangedCallback : public AudioManagerAudioSceneChangedCallback {
public:
    static bool IsSameCallback(napi_env env, napi_value callback, napi_ref refCallback);

    explicit NapiAudioSceneChangedCallback(napi_env env);
    virtual ~NapiAudioSceneChangedCallback();
    void SaveCallbackReference(const std::string &callbackName, napi_value callback);
    void RemoveCallbackReference(napi_env env, napi_value callback);
    void RemoveAllCallbackReference();
    int32_t GetAudioSceneCbListSize();
    void OnAudioSceneChange(const AudioScene audioScene) override;
    void CreateSceneChgTsfn(napi_env env);
    bool GetSceneChgTsfnFlag() const;

private:
    struct AudioSceneJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        AudioScene audioScene = AudioScene::AUDIO_SCENE_INVALID;
    };

    void OnJsCallbackAudioSceneChange(std::unique_ptr<AudioSceneJsCallback> &jsCb);
    static void SafeJsCallbackAudioSceneChangeWork(napi_env env, napi_value js_cb, void *context, void *data);
    static void AudioSceneChangeTsfnFinalize(napi_env env, void *data, void *hint);

    std::mutex mutex_;
    napi_env env_ = nullptr;
    std::list<std::shared_ptr<AutoRef>> audioSceneChangeCbList_;
    bool regAmSceneChgTsfn_ = false;
    napi_threadsafe_function amSceneChgTsfn_ = nullptr;
};
} // namespace AudioStandard
} // namespace OHOS
#endif /* NAPI_AUDIO_SCENE_CALLBACK_H */