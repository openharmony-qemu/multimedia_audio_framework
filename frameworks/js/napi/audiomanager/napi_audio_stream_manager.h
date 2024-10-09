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
#ifndef NAPI_AUDIO_STREAM_MANAGER_H
#define NAPI_AUDIO_STREAM_MANAGER_H

#include <iostream>
#include <map>
#include <vector>
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_async_work.h"
#include "audio_system_manager.h"
#include "audio_stream_manager.h"

namespace OHOS {
namespace AudioStandard {
const std::string AUDIO_STREAM_MGR_NAPI_CLASS_NAME = "AudioStreamManager";
const std::string RENDERERCHANGE_CALLBACK_NAME = "audioRendererChange";
const std::string CAPTURERCHANGE_CALLBACK_NAME = "audioCapturerChange";

class NapiAudioStreamMgr {
public:
    NapiAudioStreamMgr();
    ~NapiAudioStreamMgr();

    static napi_value Init(napi_env env, napi_value exports);
    static napi_value CreateStreamManagerWrapper(napi_env env);

private:
    struct AudioStreamMgrAsyncContext : public ContextBase {
        int32_t intValue;
        int32_t volType;
        int32_t contentType;
        int32_t streamUsage;
        bool isTrue;
        bool isLowLatencySupported;
        bool isActive;
        AudioStreamInfo audioStreamInfo;
        NapiAudioStreamMgr *objectInfo;
        std::vector<std::unique_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
        std::vector<std::unique_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
        AudioSceneEffectInfo audioSceneEffectInfo;
    };

    static napi_value Construct(napi_env env, napi_callback_info info);
    static void Destructor(napi_env env, void *nativeObject, void *finalizeHint);
    static bool CheckContextStatus(std::shared_ptr<AudioStreamMgrAsyncContext> context);
    static bool CheckAudioStreamManagerStatus(NapiAudioStreamMgr *napi,
        std::shared_ptr<AudioStreamMgrAsyncContext> context);
    static NapiAudioStreamMgr* GetParamWithSync(const napi_env &env, napi_callback_info info,
    size_t &argc, napi_value *args);
    static napi_value GetCurrentAudioRendererInfos(napi_env env, napi_callback_info info);
    static napi_value GetCurrentAudioRendererInfosSync(napi_env env, napi_callback_info info);
    static napi_value GetCurrentAudioCapturerInfos(napi_env env, napi_callback_info info);
    static napi_value GetCurrentAudioCapturerInfosSync(napi_env env, napi_callback_info info);
    static napi_value IsAudioRendererLowLatencySupported(napi_env env, napi_callback_info info);
    static napi_value IsStreamActive(napi_env env, napi_callback_info info);
    static napi_value IsStreamActiveSync(napi_env env, napi_callback_info info);
    static napi_value GetEffectInfoArray(napi_env env, napi_callback_info info);
    static napi_value GetEffectInfoArraySync(napi_env env, napi_callback_info info);
    static napi_value GetHardwareOutputSamplingRate(napi_env env, napi_callback_info info);
    static napi_value On(napi_env env, napi_callback_info info);
    static napi_value Off(napi_env env, napi_callback_info info);
    static void RegisterCallback(napi_env env, napi_value jsThis,
        napi_value *args, const std::string &cbName);
    static void RegisterCapturerStateChangeCallback(napi_env env, napi_value *args,
        const std::string &cbName, NapiAudioStreamMgr *napiStreamMgr);
    static void RegisterRendererStateChangeCallback(napi_env env, napi_value *args,
        const std::string &cbName, NapiAudioStreamMgr *napiStreamMgr);
    static void  UnregisterCallback(napi_env env, napi_value jsThis, const std::string &cbName);

    napi_env env_;
    AudioStreamManager *audioStreamMngr_;
    int32_t cachedClientId_ = -1;
    std::shared_ptr<AudioRendererStateChangeCallback> rendererStateChangeCallbackNapi_ = nullptr;
    std::shared_ptr<AudioCapturerStateChangeCallback> capturerStateChangeCallbackNapi_ = nullptr;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif /* NAPI_AUDIO_STREAM_MANAGER_H */
