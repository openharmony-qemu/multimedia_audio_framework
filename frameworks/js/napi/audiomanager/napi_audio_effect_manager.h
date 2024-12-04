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
#ifndef NAPI_AUDIO_EFFECT_MANAGER_H
#define NAPI_AUDIO_EFFECT_MANAGER_H

#include <iostream>
#include <map>
#include <vector>
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_async_work.h"
#include "audio_system_manager.h"
#include "audio_effect_manager.h"

namespace OHOS {
namespace AudioStandard {
const std::string AUDIO_EFFECT_MGR_NAPI_CLASS_NAME = "AudioEffectManager";

class NapiAudioEffectMgr {
public:
    NapiAudioEffectMgr();
    ~NapiAudioEffectMgr();

    static napi_value Init(napi_env env, napi_value exports);
    static napi_value CreateEffectManagerWrapper(napi_env env);

private:
    static napi_value Construct(napi_env env, napi_callback_info info);
    static void Destructor(napi_env env, void *nativeObject, void *finalizeHint);
    static NapiAudioEffectMgr *GetParamWithSync(const napi_env &env, napi_callback_info info,
        size_t &argc, napi_value *args);

    static napi_value GetSupportedAudioEffectProperty(napi_env env, napi_callback_info info);
    static napi_value GetAudioEffectProperty(napi_env env, napi_callback_info info);
    static napi_value SetAudioEffectProperty(napi_env env, napi_callback_info info);

    napi_env env_;
    AudioEffectManager *audioEffectMngr_;
    int32_t cachedClientId_ = -1;
};
} // namespace AudioStandard
} // namespace OHOS
#endif /* NAPI_AUDIO_EFFECT_MANAGER_H */
