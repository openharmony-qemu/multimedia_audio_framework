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
#ifndef NAPI_TONEPLAYER_H
#define NAPI_TONEPLAYER_H
#include <iostream>
#include <map>
#include <queue>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "securec.h"
#include "audio_errors.h"
#include "tone_player.h"
#include "napi_async_work.h"

namespace OHOS {
namespace AudioStandard {
const std::string NAPI_TONE_PLAYER_CLASS_NAME = "TonePlayer";
const int32_t ARGS_LOAD_MAX = 28;
const int32_t TONE_TYPE_ARR[ARGS_LOAD_MAX] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    100, 101, 102, 103, 104, 106, 107, 108, 200, 201, 203, 204};

class NapiTonePlayer {
public:
    NapiTonePlayer();
    ~NapiTonePlayer();
    static napi_value Init(napi_env env, napi_value exports);
    std::shared_ptr<TonePlayer> tonePlayer_;

private:
    struct TonePlayerAsyncContext : public ContextBase {
        bool isTrue;
        int32_t intValue;
        AudioRendererInfo rendererInfo;
        int32_t toneType;
    };

    static void Destructor(napi_env env, void *nativeObject, void *finalizeHint);
    static napi_value Construct(napi_env env, napi_callback_info info);
    static napi_value CreateTonePlayer(napi_env env, napi_callback_info info);
    static napi_value CreateTonePlayerSync(napi_env env, napi_callback_info info);
    static napi_value Load(napi_env env, napi_callback_info info);
    static napi_value Start(napi_env env, napi_callback_info info);
    static napi_value Stop(napi_env env, napi_callback_info info);
    static napi_value Release(napi_env env, napi_callback_info info);
    static napi_value CreateTonePlayerWrapper(napi_env env, std::unique_ptr<AudioRendererInfo> &rendererInfo);
    static bool CheckTonePlayerStatus(NapiTonePlayer *napi, std::shared_ptr<TonePlayerAsyncContext> context);
    static bool ToneTypeCheck(napi_env env, int32_t type);

    static std::unique_ptr<AudioRendererInfo> sRendererInfo_;
    static std::mutex createMutex_;
    static int32_t isConstructSuccess_;
    napi_env env_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // NAPI_TONEPLAYER_H
